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
#include <layers/gc_vx_layer_conv.h>

#define QUANT8_SUPPORT 0

extern vx_float32 vxnneActivation(vx_enum func_v, vx_float32 a_v, vx_float32 b_v, vx_float32 value);

extern vx_status VX_CALLBACK vxoNNDepthwiseConvolutionLayerInitializer(vx_node node,
        vx_tensor inputs,
        vx_weights_biases_parameter weights_biases,
        vx_tensor weights,
        vx_tensor biases,
        vx_scalar padXLeft,
        vx_scalar padXRight,
        vx_scalar padYTop,
        vx_scalar padYBottom,
        vx_enum   padMode,
        vx_scalar padConstant,
        vx_scalar dilationX,
        vx_scalar dilationY,
        vx_scalar strideX,
        vx_scalar strideY,
        vx_scalar depth_multiplier,
        vx_scalar relu_s,
        vx_scalar pooling_s,
        vx_scalar poolingX,
        vx_scalar poolingY,
        vx_scalar downScaleSizeRounding,
        vx_tensor outputs);
#if REGISTER_FRAME
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDilationConvolutionLayerInitializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
#else

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDilationConvolutionLayerInitializer(vx_node node,
        vx_tensor inputs,
        vx_weights_biases_parameter weights_biases,
        vx_tensor weights,
        vx_tensor biases,
        vx_scalar padXLeft,
        vx_scalar padXRight,
        vx_scalar padYTop,
        vx_scalar padYBottom,
        vx_enum   padMode,
        vx_scalar padConstant,
        vx_scalar dilationX,
        vx_scalar dilationY,
        vx_scalar stridesX,
        vx_scalar stridesY,
        vx_scalar relu_s,
        vx_scalar pooling_s,
        vx_scalar poolingX,
        vx_scalar poolingY,
        vx_scalar downScaleSizeRounding,
        vx_tensor outputs);
#endif

vx_status vxnneExecuteSWConvertFormat(struct _vxnne_operation_s *operation)
{
    vxnne_convert_format_operation           convertOperation   = (vxnne_convert_format_operation)operation;

    vx_tensor src  = (vx_tensor)convertOperation->inputs;
    vx_tensor dst = (vx_tensor)convertOperation->outputs;
    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_uint32 dstSize;
    vx_uint32 dstCount;
    vx_int8 srcFp = TENSOR_POS(src);
    vx_int8 dstFp = TENSOR_POS(dst);
    vx_enum   dstRoundingMode = TENSOR_ROUNDING_MODE(dst);
    vx_uint32 i;

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    vxoTensor_GetTensorSize(dst, &dstSize);

    memset(dstLogical,
           0,
           dstSize);

    vxoTensor_GetTensorElementCount(dst, &dstCount);
    for (i = 0; i < dstCount; i++)
    {
        vx_float32 src0 = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(src), TENSOR_QUANT_TYPE(src), i, (vx_uint8_ptr)srcLogical, srcFp, TENSOR_TF_ZEROPOINT(src), TENSOR_TF_SCALE(src));
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(dst), TENSOR_QUANT_TYPE(dst), i, src0, (vx_uint8_ptr)dstLogical, dstFp, TENSOR_TF_ZEROPOINT(dst), TENSOR_TF_SCALE(dst), dstRoundingMode);
    }


    return VX_SUCCESS;
}

vx_status vxnneOperation_ConvolutionReluPooling_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_operation op = (vxnne_convolution_relu_pooling_operation)operation;

    if (op->reshape_weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&op->reshape_weights_biases);
    }

    if (op->sub_wb != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&op->sub_wb);
    }

    if (op->reshape_inputs != VX_NULL)
    {
        vxoTensor_ReleaseTensor(&op->reshape_inputs);
    }

    if (op->reshape_outputs != VX_NULL)
    {
        vxoTensor_ReleaseTensor(&op->reshape_outputs);
    }

    if (op->swtWeightBiases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&op->swtWeightBiases);
    }

    vxnneOperation_Deinitialize(operation);
    return VX_SUCCESS;
}

vx_status vxnneOperation_DeConvoulutionNNE_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_operation op = (vxnne_convolution_relu_pooling_operation)operation;

    if (op->weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&op->weights_biases);
    }

    vxnneOperation_Deinitialize(operation);
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWReshuffle(struct _vxnne_operation_s *operation)
{
    vxnne_reshuffle_operation           reshuffleOperation   = (vxnne_reshuffle_operation)operation;

    vx_tensor inputs = (vx_tensor)reshuffleOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)reshuffleOperation->weights_biases;
    vx_enum   padMode = reshuffleOperation->pad_mode;
    vx_scalar padConst = reshuffleOperation->pad_const;
    vx_tensor outputs = (vx_tensor)reshuffleOperation->outputs;
    vx_uint32 stride_x, stride_y;
    vx_uint32 padXLeft;
    vx_uint32 padXRight;
    vx_uint32 padYTop;
    vx_uint32 padYBottom;
    void * padConstPtr = VX_NULL;
    vx_uint32 kx, ky;

    vx_status status = VX_SUCCESS;


    padXLeft   = reshuffleOperation->pad_x_left;
    padXRight  = reshuffleOperation->pad_x_right;
    padYTop    = reshuffleOperation->pad_y_top;
    padYBottom = reshuffleOperation->pad_y_bottom;

    stride_x = WB_STRIDE_X(weights_biases);
    stride_y = WB_STRIDE_Y(weights_biases);

    kx = WB_ORG_KERNEL_X(weights_biases);
    ky = WB_ORG_KERNEL_Y(weights_biases);

    padConstPtr = (void*)vxAllocateAndZeroMemory(sizeof(vx_int32));
    if (padConstPtr == NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    if (padConst != VX_NULL)
    {
        vxReadScalarValue(padConst, padConstPtr);
        vxWriteScalarValue(padConst, padConstPtr);
    }

    *(vx_int32*)padConstPtr += TENSOR_PAD_ZERO_VALUE(inputs);

    /* if stride > 1, need do reshuffle with input buffer */
    gcmASSERT ((WB_STRIDE_X(weights_biases) > 1) || (WB_STRIDE_Y(weights_biases) > 1));

    {
        vxoNNExternsionDoReshuffle(
            operation->currBatchIndex,
            inputs,
            outputs,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            padMode,
            padConstPtr,
            stride_x,
            stride_y,
            kx,
            ky);
    }

    if (padConstPtr != VX_NULL)
    {
        vxFree(padConstPtr);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API void vxnneConvolutionReluPoolingOperation_Initialize(
    vxnne_convolution_relu_pooling_operation operation,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_size dilationX,
    vx_size dilationY,
    vx_uint32 pad_x_left,
    vx_uint32 pad_x_right,
    vx_uint32 pad_y_top,
    vx_uint32 pad_y_bottom,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_bool enable_pooling,
    vx_enum pool_type,
    vx_uint32 pool_size_x,
    vx_uint32 pool_size_y,
    vx_enum padMode,
    vx_scalar padConst,
    vx_tensor outputs
    )
{
    operation->inputs           = inputs;
    operation->weights_biases   = weights_biases;
    operation->dilationX        = dilationX;
    operation->dilationY        = dilationY;
    operation->pad_x_left       = pad_x_left;
    operation->pad_x_right      = pad_x_right;
    operation->pad_y_top        = pad_y_top;
    operation->pad_y_bottom     = pad_y_bottom;
    operation->conv_rounding_type = conv_rounding_type;
    operation->enable_relu      = enable_relu;
    operation->enable_pooling   = enable_pooling;
    operation->pool_type        = pool_type;
    operation->pool_size_x      = pool_size_x;
    operation->pool_size_y      = pool_size_y;
    operation->padMode          = padMode;
    operation->padConst         = padConst;
    operation->outputs          = outputs;
}

vx_status vxnneConvolutionReluPoolingInitializer(
    vx_node node,
    char* name,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_size dilationX,
    vx_size dilationY,
    vx_uint32 pad_x_left,
    vx_uint32 pad_x_right,
    vx_uint32 pad_y_top,
    vx_uint32 pad_y_bottom,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_bool enable_pooling,
    vx_enum pool_type,
    vx_uint32 pool_size_x,
    vx_uint32 pool_size_y,
    vx_enum padMode,
    vx_scalar padConst,
    vx_tensor outputs
    )
{
    vx_status status = VX_SUCCESS;

    vx_context context = vxGetContext((vx_reference)node);
    vx_uint32 padConstValue = padConst != VX_NULL ? padConst->value->u32 : 0;
    vx_tensor convertTensor = VX_NULL;
    vx_uint32 batchCount = (TENSOR_SIZE_INDEX(inputs, 3) == 0) ? 1 : TENSOR_SIZE_INDEX(inputs, 3);
    vx_tensor interTensor = inputs;
    vx_uint32 tmpTensorIndex = 0;
    vx_bool enableBrickMode = vx_false_e;
    vx_bool needExtraBrickOp = vx_true_e;

    vx_weights_biases_parameter reshapeWb = VX_NULL;

    vxnne_convolution_relu_pooling_layer convolutionReluPoolingLayer = gcvNULL;

    vx_uint32 operationIndex = 0;

    if (!vxnneIsNNSupportFormat(context, inputs, weights_biases, outputs))
    {
        status = VX_ERROR_NOT_SUPPORTED;
        vxError("hw not support this format. function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_relu_pooling_layer_s), (gctPOINTER*)&convolutionReluPoolingLayer);
    if (!convolutionReluPoolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(convolutionReluPoolingLayer, sizeof(vxnne_convolution_relu_pooling_layer_s));

    vxnneLayer_Initialize(&convolutionReluPoolingLayer->base,
                            name,
                            node,
                            vxmOPERATION_COUNT(convolutionReluPoolingLayer),
                            convolutionReluPoolingLayer->operations,
                            VX_NULL);

    if (weights_biases->wb_base->hw_depth_wise)
    {
        status = vxnneOperation_Initialize(&convolutionReluPoolingLayer->convolution_operation.base,
                                           &convolutionReluPoolingLayer->base,
                                           VXNNE_OPERATION_TARGET_NN,
                                           VXNNE_OPERATOR_DEPTH_WISE_CONV,
                                           VX_NULL,
                                           vxnneOperation_ConvolutionReluPooling_Deinitialize,
                                           batchCount,
                                           NNE_COMMAND_SIZE);
    }
    else
    {
        status = vxnneOperation_Initialize(&convolutionReluPoolingLayer->convolution_operation.base,
                                           &convolutionReluPoolingLayer->base,
                                           VXNNE_OPERATION_TARGET_NN,
                                           VXNNE_OPERATOR_CONVOLUTION,
                                           VX_NULL,
                                           vxnneOperation_ConvolutionReluPooling_Deinitialize,
                                           batchCount,
                                           NNE_COMMAND_SIZE);
    }
    if (status != VX_SUCCESS) goto exit;


    if ((WB_STRIDE_X(weights_biases) > 1) || (WB_STRIDE_Y(weights_biases) > 1))
    {
        vx_uint32 sizes[4];
        vx_tensor reshuffleTensor = VX_NULL;
        vx_tensor specialTensorFor1x1 = VX_NULL;
        vx_tensor_create_params_t tensor_create_params;

        sizes[0] = ComputeInputSize(TENSOR_VIEW_SIZE_INDEX(outputs, 0), WB_KERNEL_X(weights_biases), 0, 0, pool_size_x, 2, VX_NULL, 1);
        sizes[1] = ComputeInputSize(TENSOR_VIEW_SIZE_INDEX(outputs, 1), WB_KERNEL_Y(weights_biases), 0, 0, pool_size_y, 2, VX_NULL, 1);
        sizes[2] = TENSOR_VIEW_SIZE_INDEX(inputs, 2) * WB_STRIDE_X(weights_biases) * WB_STRIDE_Y(weights_biases);
        sizes[3] = TENSOR_SIZE_INDEX(inputs, 3);

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = TENSOR_DIM_NUM(inputs);
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
        }

        reshuffleTensor = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
        if (reshuffleTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_RESHUFFLE))
        {
            vx_op_param_s conv = {0};

            reshuffleTensor->brickMode = enableBrickMode;
            status = vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_tp_operation.base,
                                               &convolutionReluPoolingLayer->base,
                                               VXNNE_OPERATION_TARGET_TP,
                                               VXNNE_OPERATOR_RESHUFFLE,
                                               VX_NULL,
                                               vxnneOperation_TP_Deinitialize,
                                               batchCount,
                                               0);
            if (status != VX_SUCCESS) goto exit;

            conv.pad_x_left = pad_x_left;
            conv.pad_y_top = pad_y_top;
            conv.pad_x_right = pad_x_right;
            conv.pad_y_bottom = pad_y_bottom;
            conv.pool_size_x = conv.pool_size_y = 0;
            conv.pool_stride = 1;
            conv.enable_relu = vx_false_e;
            conv.pad_mode = padMode;
            conv.pad_const = (vx_int32)padConstValue + TENSOR_PAD_ZERO_VALUE(inputs);
            conv.tpType = TP_RESHUFFLE;
            conv.other_ref = (vx_reference)weights_biases;
            conv.data_buff = gcvNULL;

            memcpy(&convolutionReluPoolingLayer->reshuffle_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));
            convolutionReluPoolingLayer->reshuffle_tp_operation.input          = inputs;
            convolutionReluPoolingLayer->reshuffle_tp_operation.weights_biases = weights_biases;
            convolutionReluPoolingLayer->reshuffle_tp_operation.output         = reshuffleTensor;

            vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_tp_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_tp_operation.base, (vx_reference)reshuffleTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &convolutionReluPoolingLayer->base,
                &convolutionReluPoolingLayer->reshuffle_tp_operation.base,
                operationIndex++);

            needExtraBrickOp = vx_false_e;
        }
        else
        {
            vx_uint32      kx                      = WB_ORG_KERNEL_X(weights_biases);
            vx_uint32      ky                      = WB_ORG_KERNEL_Y(weights_biases);
            vx_uint32      stride_x                = WB_STRIDE_X(weights_biases);
            vx_uint32      stride_y                = WB_STRIDE_Y(weights_biases);
            vx_uint32      dstWidth                = TENSOR_VIEW_SIZE_INDEX(reshuffleTensor, 0);
            vx_uint32      dstHeight               = TENSOR_VIEW_SIZE_INDEX(reshuffleTensor, 1);
            vx_uint32      dstDepth                = TENSOR_VIEW_SIZE_INDEX(reshuffleTensor, 2);
            vx_bool        shExe_flag              = vx_true_e;
            vx_enum        inputFormat             = TENSOR_DATA_TYPE(inputs);
            vx_enum        outputFormat            = TENSOR_DATA_TYPE(reshuffleTensor);
            vx_bool        padMode_flag            = (vx_bool)(padMode == VX_PAD_CONSTANT || padMode == VX_PAD_REPLICATE || (pad_x_left == 0 && pad_x_right == 0 && pad_y_top == 0 && pad_y_bottom == 0));

            shExe_flag    = (vx_bool)(((_IsSameType(inputs, reshuffleTensor) && stride_x == 2 && stride_y == 2)
                                    || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && stride_x == 4 && stride_y == 4)
                                    || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16 && stride_x == 4 && stride_y == 4)
                                    || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && stride_x == 3 && stride_y == 3)
                                    || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16 && stride_x == 3 && stride_y == 3))
                                    && (kx != 1 || ky != 1));
            if(shExe_flag && padMode_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
            {
                vxnne_shader_executable shaderExecutable = VX_NULL;
                vx_tensor                outTensor = NULL;
                if(outTensor == NULL)
                {
                    vx_int32   new_size[3] = {dstWidth, dstHeight, dstDepth};
                    vx_uint32  outputs_dims = 3;
                    outTensor = vxoTensor_ReshapeTensor(reshuffleTensor, new_size, outputs_dims);
                }

                shaderExecutable = vxnneReshuffleShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_RESHUFFLE, &node->kernelAttributes.borderMode, inputs, stride_x, stride_y, padMode, padConstValue, pad_x_left, pad_x_right, pad_y_top, pad_y_bottom, outTensor);


                vxoTensor_ReleaseTensor(&outTensor);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                status = vxnneShaderOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_shader_operation,
                    &convolutionReluPoolingLayer->base,
                    VXNNE_OPERATOR_RESHUFFLE,
                    batchCount,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_shader_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_shader_operation.base, (vx_reference)reshuffleTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &convolutionReluPoolingLayer->base,
                    &convolutionReluPoolingLayer->reshuffle_shader_operation.base,
                    operationIndex++);

            }
            else
            {
                vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_operation.base,
                    &convolutionReluPoolingLayer->base,
                    VXNNE_OPERATION_TARGET_SW,
                    VXNNE_OPERATOR_RESHUFFLE,
                    vxnneExecuteSWReshuffle,
                    VX_NULL,
                    batchCount,
                    0);

                convolutionReluPoolingLayer->reshuffle_operation.inputs         = inputs;
                convolutionReluPoolingLayer->reshuffle_operation.weights_biases = weights_biases;
                convolutionReluPoolingLayer->reshuffle_operation.pad_x_left     = pad_x_left;
                convolutionReluPoolingLayer->reshuffle_operation.pad_x_right    = pad_x_right;
                convolutionReluPoolingLayer->reshuffle_operation.pad_y_top      = pad_y_top;
                convolutionReluPoolingLayer->reshuffle_operation.pad_y_bottom   = pad_y_bottom;
                convolutionReluPoolingLayer->reshuffle_operation.pad_mode       = padMode;
                convolutionReluPoolingLayer->reshuffle_operation.pad_const      = padConst;
                convolutionReluPoolingLayer->reshuffle_operation.outputs        = reshuffleTensor;

                vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionReluPoolingLayer->reshuffle_operation.base, (vx_reference)reshuffleTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxnneLayer_SetOperation(
                    &convolutionReluPoolingLayer->base,
                    &convolutionReluPoolingLayer->reshuffle_operation.base,
                    operationIndex++);
            }
        }

        convolutionReluPoolingLayer->base.temp_tensors[tmpTensorIndex++] = reshuffleTensor;

        if (weights_biases->wb_base->org_weights_sizes[0] == 1 && weights_biases->wb_base->org_weights_sizes[1] == 1)
        {
            /*Since in kx=1, ky=1 situation, weight won't do reshuffle,
            TP has special opt for input, only first num of kz input channels are valid*/
            vx_uint32 smallSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_uint32 smallSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_tensor_view smallView = VX_NULL;

            smallSizeEnd[0] = reshuffleTensor->dims[0];
            smallSizeEnd[1] = reshuffleTensor->dims[1];
            smallSizeEnd[2] = weights_biases->weights_sizes[2];
            smallSizeEnd[3] = reshuffleTensor->dims[3];

            smallView = vxCreateTensorView(node->base.context, smallSizeStart, smallSizeEnd, (vx_uint8)reshuffleTensor->dimCount);
            specialTensorFor1x1 = vxoTensor_CreateTensorFromView(reshuffleTensor, smallView);
            if (smallView != VX_NULL) vxReleaseTensorView(&smallView);
            convolutionReluPoolingLayer->base.temp_tensors[tmpTensorIndex++] = specialTensorFor1x1;
            interTensor = specialTensorFor1x1;
        }
        else
            interTensor = reshuffleTensor;

        /* stride > 1, set pad value to 0 */
        pad_x_left   = 0;
        pad_x_right  = 0;
        pad_y_top    = 0;
        pad_y_bottom = 0;
    }
    else if (TENSOR_DATA_TYPE(interTensor) != WB_WEIGHT_DATA_FORMAT(weights_biases))
    {
        vx_enum        inputFormat             = TENSOR_DATA_TYPE(interTensor);
        vx_enum        weightFormat            = WB_WEIGHT_DATA_FORMAT(weights_biases);
        vx_bool        shExe_flag              = (vx_bool)(inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_INT8);
        vx_uint32 sizes[3] =
        {
            TENSOR_VIEW_SIZE_INDEX(inputs, 0),
            TENSOR_VIEW_SIZE_INDEX(inputs, 1),
            TENSOR_VIEW_SIZE_INDEX(inputs, 2)
        };

        vx_tensor_create_params_t tensor_create_params;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = TENSOR_DIM_NUM(interTensor);
        tensor_create_params.sizes = sizes;
        tensor_create_params.data_format = WB_WEIGHT_DATA_FORMAT(weights_biases);
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = WB_BIAS_FPP(weights_biases)- WB_WEIGHT_FPP(weights_biases);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
        }

        convertTensor = vxoTensor_CreateTensor(
                        context,
                        node->graph,
                        &tensor_create_params,
                        vx_true_e
                        );
        if (convertTensor == VX_NULL)
        {
            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;

            shaderExecutable = vxnneTensorConvFormatShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_CONVFORMAT, &node->kernelAttributes.borderMode, interTensor, convertTensor);

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&convolutionReluPoolingLayer->convert_format_shader_operation,
                &convolutionReluPoolingLayer->base,
                VXNNE_OPERATOR_CONVERT_FORMAT,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&convolutionReluPoolingLayer->convert_format_shader_operation.base, (vx_reference)interTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionReluPoolingLayer->convert_format_shader_operation.base, (vx_reference)convertTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &convolutionReluPoolingLayer->base,
                &convolutionReluPoolingLayer->convert_format_shader_operation.base,
                operationIndex++);
        }
        else
        {
            vxnneOperation_Initialize(&convolutionReluPoolingLayer->convert_format_operation.base,
                &convolutionReluPoolingLayer->base,
                VXNNE_OPERATION_TARGET_SW,
                VXNNE_OPERATOR_CONVERT_FORMAT,
                vxnneExecuteSWConvertFormat,
                VX_NULL,
                batchCount,
                0);
            convolutionReluPoolingLayer->convert_format_operation.inputs         = interTensor;
            convolutionReluPoolingLayer->convert_format_operation.outputs        = convertTensor;
            vxnneOperation_AddReference(&convolutionReluPoolingLayer->convert_format_operation.base, (vx_reference)interTensor, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionReluPoolingLayer->convert_format_operation.base, (vx_reference)convertTensor, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            vxnneLayer_SetOperation(
                &convolutionReluPoolingLayer->base,
                &convolutionReluPoolingLayer->convert_format_operation.base,
                operationIndex++);
        }

        convolutionReluPoolingLayer->base.temp_tensors[tmpTensorIndex++] = convertTensor;

        interTensor = convertTensor;
    }

    /* TODO. */
    needExtraBrickOp = vx_false_e;


    if (weights_biases->wb_base->do_zdp_opt &&
        !context->options.do1xnAfterSwtiling)
    {
        vx_uint32 fitN = 0;
        vx_uint32 fitOutN = 0;
        vx_uint32 i;
        vx_int32 reshapeInputSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
        vx_int32 reshapeOutputSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
        vx_uint32 stride = 1;

        calcFitZdp3N(context, TENSOR_VIEW_SIZE_INDEX(interTensor, 0), TENSOR_VIEW_SIZE_INDEX(interTensor, 1), &fitN, stride, pool_size_x);
        fitOutN = fitN / stride;

        if (fitN == 0)
        {
            vxmASSERT(0 && "fitN = 0");
            status = VX_FAILURE;
            goto exit;
        }
        reshapeInputSize[0] = TENSOR_VIEW_SIZE_INDEX(interTensor, 0) * TENSOR_VIEW_SIZE_INDEX(interTensor, 1) / fitN;
        reshapeInputSize[1] = fitN;

        reshapeOutputSize[0] = TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1) / fitOutN;
        reshapeOutputSize[1] = fitOutN;

        for (i = 2; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            reshapeInputSize[i] = TENSOR_VIEW_SIZE_INDEX(interTensor, i);
            reshapeOutputSize[i] = TENSOR_VIEW_SIZE_INDEX(outputs, i);
        }

        convolutionReluPoolingLayer->convolution_operation.reshape_inputs = vxoTensor_ReshapeTensor(interTensor, reshapeInputSize, interTensor->dimCount);
        convolutionReluPoolingLayer->convolution_operation.reshape_outputs = vxoTensor_ReshapeTensor(outputs, reshapeOutputSize, outputs->dimCount);

        vxmASSERT(convolutionReluPoolingLayer->convolution_operation.reshape_inputs != VX_NULL && convolutionReluPoolingLayer->convolution_operation.reshape_outputs != VX_NULL);
    }
    else if (weights_biases->wb_base->do_1xN &&
        !context->options.do1xnAfterSwtiling)
    {
        vx_uint32 fitN = calcFit1xN(context, TENSOR_VIEW_SIZE_INDEX(interTensor, 2), TENSOR_VIEW_SIZE_INDEX(interTensor, 0), TENSOR_VIEW_SIZE_INDEX(interTensor, 1));

        vx_uint32 i;
        vx_int32 reshapeInputSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
        vx_int32 reshapeOutputSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

        /* Need reshape input[x, y, kz] --> [x*y, fitN, kz/fitN] */
        /* Need reshape output[x, y, vz] --> [x*y, 1, vz] */
        reshapeInputSize[0] = TENSOR_VIEW_SIZE_INDEX(interTensor, 0) * TENSOR_VIEW_SIZE_INDEX(interTensor, 1);
        reshapeInputSize[1] = fitN;
        reshapeInputSize[2] = TENSOR_VIEW_SIZE_INDEX(interTensor, 2) / fitN;

        reshapeOutputSize[0] = TENSOR_VIEW_SIZE_INDEX(outputs, 0) * TENSOR_VIEW_SIZE_INDEX(outputs, 1);
        reshapeOutputSize[1] = 1;
        reshapeOutputSize[2] = TENSOR_VIEW_SIZE_INDEX(outputs, 2);

        for (i = 3; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            reshapeInputSize[i] = TENSOR_VIEW_SIZE_INDEX(interTensor, i);
            reshapeOutputSize[i] = TENSOR_VIEW_SIZE_INDEX(outputs, i);
        }

        convolutionReluPoolingLayer->convolution_operation.reshape_inputs = vxoTensor_ReshapeTensor(interTensor, reshapeInputSize, interTensor->dimCount);
        convolutionReluPoolingLayer->convolution_operation.reshape_outputs = vxoTensor_ReshapeTensor(outputs, reshapeOutputSize, outputs->dimCount);

        vxmASSERT(convolutionReluPoolingLayer->convolution_operation.reshape_inputs != VX_NULL && convolutionReluPoolingLayer->convolution_operation.reshape_outputs != VX_NULL);
    }

    /* Convolution operation. */
    if ((weights_biases->wb_base->do_zdp_opt || weights_biases->wb_base->do_1xN) &&
        convolutionReluPoolingLayer->convolution_operation.reshape_inputs != VX_NULL &&
        convolutionReluPoolingLayer->convolution_operation.reshape_outputs != VX_NULL &&
        !context->options.do1xnAfterSwtiling)
    {
        convolutionReluPoolingLayer->convolution_operation.inputs         = convolutionReluPoolingLayer->convolution_operation.reshape_inputs;
        convolutionReluPoolingLayer->convolution_operation.outputs        = convolutionReluPoolingLayer->convolution_operation.reshape_outputs;
    }
    else
    {
        convolutionReluPoolingLayer->convolution_operation.inputs         = interTensor;
        convolutionReluPoolingLayer->convolution_operation.outputs        = outputs;
    }
    convolutionReluPoolingLayer->convolution_operation.orig_inputs    = inputs;
    convolutionReluPoolingLayer->convolution_operation.weights_biases = weights_biases;
    convolutionReluPoolingLayer->convolution_operation.reshape_weights_biases = reshapeWb;


    vxnneLayer_SetOperation(
        &convolutionReluPoolingLayer->base,
        &convolutionReluPoolingLayer->convolution_operation.base,
        operationIndex++);

    convolutionReluPoolingLayer->base.num_temp_tensors        = tmpTensorIndex;

    inputs->brickMode = interTensor->brickMode;

    if (weights_biases->wb_base->do_fisrt_pixel_pool)
    {
        vxnneConvolutionReluPoolingOperation_Initialize(
                &convolutionReluPoolingLayer->convolution_operation,
                convolutionReluPoolingLayer->convolution_operation.inputs,
                weights_biases,
                dilationX,
                dilationY,
                pad_x_left,
                pad_x_right,
                pad_y_top,
                pad_y_bottom,
                conv_rounding_type,
                enable_relu,
                vx_true_e,
                VX_NN_POOLING_FFP,
                2,
                2,
                padMode,
                padConst,
                convolutionReluPoolingLayer->convolution_operation.outputs);
    }
    else
    {
        vxnneConvolutionReluPoolingOperation_Initialize(
                &convolutionReluPoolingLayer->convolution_operation,
                convolutionReluPoolingLayer->convolution_operation.inputs,
                weights_biases,
                dilationX,
                dilationY,
                pad_x_left,
                pad_x_right,
                pad_y_top,
                pad_y_bottom,
                conv_rounding_type,
                enable_relu,
                enable_pooling,
                pool_type,
                pool_size_x,
                pool_size_y,
                padMode,
                padConst,
                convolutionReluPoolingLayer->convolution_operation.outputs);
    }
    vxnneOperation_AddReference(&convolutionReluPoolingLayer->convolution_operation.base, (vx_reference)convolutionReluPoolingLayer->convolution_operation.inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
    vxnneOperation_AddReference(&convolutionReluPoolingLayer->convolution_operation.base, (vx_reference)convolutionReluPoolingLayer->convolution_operation.outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);



    {
        vx_op_param_s conv = {0};

        conv.pad_x_left = pad_x_left;
        conv.pad_x_right = pad_x_right;
        conv.pad_y_top = pad_y_top;
        conv.pad_y_bottom = pad_y_bottom;
        conv.pad_mode = padMode;
        conv.pad_const = padConst != VX_NULL ? padConst->value->n32 : 0;
        conv.pool_type = pool_type;
        conv.pool_size_x = pool_size_x;
        conv.pool_size_y = pool_size_y;
        conv.pool_stride = 2;
        conv.conv_rounding_type = conv_rounding_type;
        conv.enable_relu = enable_relu;

        if (weights_biases->wb_base->do_fisrt_pixel_pool)
        {
            conv.pool_type = VX_NN_POOLING_FFP;
            conv.pool_size_x = 2;
            conv.pool_size_y = 2;
        }
        else if (enable_pooling)
        {
            conv.pool_size_x = pool_size_x;
            conv.pool_size_y = pool_size_y;
        }
        else
        {
            conv.pool_size_x = conv.pool_size_y = 0;
        }
        memcpy(&convolutionReluPoolingLayer->convolution_operation.base.parameter, &conv, sizeof(vx_op_param_s));
    }

    node->layer = &convolutionReluPoolingLayer->base;
    return status;

exit:
    if (convolutionReluPoolingLayer) gcoOS_Free(gcvNULL, (gctPOINTER)convolutionReluPoolingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_scalar                   pool_type_s = (vx_scalar)parameters[9];
    vx_scalar                   pool_size_x_s = (vx_scalar)parameters[10];
    vx_scalar                   pool_size_y_s = (vx_scalar)parameters[11];
    vx_tensor                   outputs = (vx_tensor)parameters[12];

    vx_enum                     conv_rounding_type;
    vx_enum                     pool_type;
    vx_uint32                   pool_size_x;
    vx_uint32                   pool_size_y;
    vx_bool                     enable_relu;
    vx_uint32                   pad_x_left;
    vx_uint32                   pad_x_right;
    vx_uint32                   pad_y_top;
    vx_uint32                   pad_y_bottom;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    pool_type            = pool_type_s->value->e;
    pool_size_x          = pool_size_x_s->value->u32;
    pool_size_y          = pool_size_y_s->value->u32;
    enable_relu          = enable_relu_s->value->b;
    pad_x_left           = pad_x_s->value->u32;
    pad_x_right          = pad_x_left;
    pad_y_top            = pad_y_s->value->u32;
    pad_y_bottom         = pad_y_top;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingInitializer(node,
                                                     "ConvolutionReluPoolingLayer",
                                                      inputs,
                                                      weights_biases,
                                                      0,
                                                      0,
                                                      pad_x_left,
                                                      pad_x_right,
                                                      pad_y_top,
                                                      pad_y_bottom,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_true_e,
                                                      pool_type,
                                                      pool_size_x,
                                                      pool_size_y,
                                                      VX_PAD_CONSTANT,
                                                      VX_NULL,
                                                      outputs);

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer2(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status                   status = VX_SUCCESS;

    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   dilation_x_s = (vx_scalar)parameters[2];
    vx_scalar                   dilation_y_s = (vx_scalar)parameters[3];
    vx_scalar                   pad_x_left_s = (vx_scalar)parameters[4];
    vx_scalar                   pad_x_right_s = (vx_scalar)parameters[5];
    vx_scalar                   pad_y_top_s = (vx_scalar)parameters[6];
    vx_scalar                   pad_y_bottom_s = (vx_scalar)parameters[7];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[11];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[12];
    vx_scalar                   pool_type_s = (vx_scalar)parameters[13];
    vx_scalar                   pool_size_x_s = (vx_scalar)parameters[14];
    vx_scalar                   pool_size_y_s = (vx_scalar)parameters[15];
    vx_scalar                   pad_mode_s = (vx_scalar)parameters[16];
    vx_scalar                   pad_const_s = (vx_scalar)parameters[17];
    vx_tensor                   outputs = (vx_tensor)parameters[num - 1];

    if ((dilation_x_s && (dilation_x_s->value->n32) > 0) || (dilation_y_s && (dilation_y_s->value->n32) > 0))
    {
#if REGISTER_FRAME
        vx_reference params[] = {
            (vx_reference)inputs, (vx_reference)weights_biases, VX_NULL, VX_NULL, /*input, weights_bias, weights, biases*/
            (vx_reference)pad_x_left_s, (vx_reference)pad_x_right_s, (vx_reference)pad_y_top_s, (vx_reference)pad_y_bottom_s, /*pad x, x_right, y, y_bottom*/
            (vx_reference)pad_mode_s, (vx_reference)pad_const_s, /*padmode, padconstant*/
            (vx_reference)dilation_x_s, (vx_reference)dilation_y_s, parameters[8], parameters[9], /*dilation x/y, stride x/y */
            (vx_reference)enable_relu_s, (vx_reference)pool_type_s, (vx_reference)pool_size_x_s, (vx_reference)pool_size_y_s, /*relu, pool type, x, y */
            (vx_reference)down_scale_size_rounding_s, (vx_reference)outputs, /*rounding, output */
        };

        gcmASSERT(TENSOR_SIZE_INDEX(inputs, 3) == 1);/* not support batch while dilation enabled */
        status = vxoNNDilationConvolutionLayerInitializer(node, params, gcmCOUNTOF(params));
#else
        gcmASSERT(TENSOR_SIZE_INDEX(inputs, 3) == 1);/* not support batch while dilation enabled */

        status = vxoNNDilationConvolutionLayerInitializer(node,
            inputs,
            weights_biases, VX_NULL, VX_NULL,
            pad_x_left_s, pad_x_right_s, pad_y_top_s, pad_y_bottom_s, VX_PAD_CONSTANT, VX_NULL,
            dilation_x_s, dilation_y_s, VX_NULL, VX_NULL,
            enable_relu_s,
            pool_type_s, pool_size_x_s, pool_size_y_s,
            down_scale_size_rounding_s,
            outputs
            );
#endif
    }
    else
    {
        vx_enum                     conv_rounding_type;
        vx_enum                     pool_type;
        vx_uint32                   pool_size_x;
        vx_uint32                   pool_size_y;
        vx_bool                     enable_relu;

        conv_rounding_type   = down_scale_size_rounding_s->value->e;
        pool_type            = pool_type_s->value->e;
        pool_size_x          = pool_size_x_s->value->u32;
        pool_size_y          = pool_size_y_s->value->u32;
        enable_relu          = enable_relu_s->value->b;

        /* destroy the existing layer */
        if (node->layer)
        {
            vxnneLayer_Free(node->layer);
            node->layer = VX_NULL;
        }

        status = vxnneConvolutionReluPoolingInitializer(node,
                                                         "ConvolutionReluPoolingLayer2",
                                                          inputs,
                                                          weights_biases,
                                                          (dilation_x_s == NULL) ? 0 : dilation_x_s->value->s,
                                                          (dilation_y_s == NULL) ? 0 : dilation_y_s->value->s,
                                                          pad_x_left_s->value->u32,
                                                          pad_x_right_s->value->u32,
                                                          pad_y_top_s->value->u32,
                                                          pad_y_bottom_s->value->u32,
                                                          conv_rounding_type,
                                                          enable_relu,
                                                          vx_true_e,
                                                          pool_type,
                                                          pool_size_x,
                                                          pool_size_y,
                                                          pad_mode_s->value->e,
                                                          pad_const_s,
                                                          outputs);
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer2_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_ConvolutionReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_tensor                   outputs = (vx_tensor)parameters[9];
    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;
    vx_uint32                   pad_x_left;
    vx_uint32                   pad_x_right;
    vx_uint32                   pad_y_top;
    vx_uint32                   pad_y_bottom;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;
    pad_x_left           = pad_x_s->value->u32;
    pad_x_right          = pad_x_left;
    pad_y_top            = pad_y_s->value->u32;
    pad_y_bottom         = pad_y_top;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingInitializer(node,
                                                      "ConvolutionReluLayer",
                                                      inputs,
                                                      weights_biases,
                                                      0,
                                                      0,
                                                      pad_x_left,
                                                      pad_x_right,
                                                      pad_y_top,
                                                      pad_y_bottom,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_false_e,
                                                      VIV_NN_POOLING_NON,
                                                      0,
                                                      0,
                                                      VX_PAD_CONSTANT,
                                                      VX_NULL,
                                                      outputs);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

#define DUMP_ONE_PIXEL_CPU_CONV 0

#if QUANT8_SUPPORT
vx_int32 Add(vx_int32 a, vx_int32 b) {
    return a + b;
}
vx_int32 BitAnd(vx_int32 a, vx_int32 b) {
    return a & b;
}
vx_int64 ShiftRight(vx_int32 a, int offset) {
    return a >> offset;
}
vx_int32 BitNot(vx_int32 a) {
    return ~a;
}
vx_int32 MaskIfNonZero(vx_int32 a) {
    static vx_int32 zero = 0;
    return a ? BitNot(zero) : zero;
}
vx_int32 MaskIfLessThan(vx_int32 a, vx_int32 b) {
    return MaskIfNonZero(a < b);
}
vx_int64 MaskIfGreaterThan(vx_int32 a, vx_int32 b) {
    return MaskIfNonZero(a > b);
}
vx_int32 RoundingDivideByPOT(vx_int32 x, vx_int32 exponent) {
    assert(exponent >= 0);
    assert(exponent <= 31);
    const vx_int32 mask = ((1ll << exponent) - 1);
    const vx_int32 zero = 0;
    const vx_int32 one = 1;
    const vx_int32 remainder = BitAnd(x, mask);
    const vx_int32 threshold =
        Add(ShiftRight(mask, 1), BitAnd(MaskIfLessThan(x, zero), one));
    return Add(ShiftRight(x, exponent),
        BitAnd(MaskIfGreaterThan(remainder, threshold), one));
}
vx_int32 SaturatingRoundingDoublingHighMul(vx_int32 a, vx_int32 b) {
    vx_bool overflow = a == b && a == 0x80000000;
    vx_int64 ab_64 = (vx_int64)a * (vx_int64)b;
    vx_int32 nudge = ab_64 >= 0 ? (1 << 30) : (1 - (1 << 30));
    vx_int32 ab_x2_high32 =
        (vx_int32)((ab_64 + nudge) / (1ll << 31));
    return overflow ? 0x7fffffff : ab_x2_high32;
}
#endif

vx_status vxnneExecuteSWConvolution(vxnne_operation operation)
{
    vxnne_convolution_operation convolutionOperation   = (vxnne_convolution_operation)operation;

    vx_tensor inputs                = convolutionOperation->inputs;
    vx_tensor weights               = convolutionOperation->weights;
    vx_tensor biases                = convolutionOperation->biases;
    vx_scalar padX                  = convolutionOperation->padX;
    vx_scalar padY                  = convolutionOperation->padY;
    vx_scalar padX_Right            = convolutionOperation->padXRight;
    vx_scalar padY_Bottom           = convolutionOperation->padYBottom;
    vx_scalar dilationX             = convolutionOperation->dilationX;
    vx_scalar dilationY             = convolutionOperation->dilationY;
    vx_scalar strideX               = convolutionOperation->strideX;
    vx_scalar strideY               = convolutionOperation->strideY;
    vx_scalar relu                  = convolutionOperation->relu;
    vx_scalar downScaleSizeRounding = convolutionOperation->downScaleSizeRounding;
    vx_tensor outputs               = convolutionOperation->outputs;

    vx_int32 batch = 1;
    void * inputBaseLogical;
    void * outputBaseLogical;

    void *weightsBaseLogical = VX_NULL;
    void *biasesBaseLogical = VX_NULL;

    vx_int32 inputWidth, inputHeight, inputDepth, outputWidth, outputHeight, outputDepth;
    vx_int32 kernelXSize, kernelYSize, stride_x, stride_y;
    vx_int32 k, p, j, i;
    vx_uint8_ptr dataSrc;
    vx_uint8_ptr dataDst;
    vx_uint8_ptr dataWeight;
    vx_uint8_ptr dataBias;
    vx_type_e inputFormat;
    vx_type_e weightFormat;
    vx_type_e biasFormat;
    vx_type_e outputFormat;

    vx_enum downScaleSizeRoundingValue = downScaleSizeRounding->value->e;
    vx_uint32 padXLeft;
    vx_uint32 padXRight;
    vx_uint32 padYTop;
    vx_uint32 padYBottom;

    vx_int32 dilation_x = (dilationX)?dilationX->value->n32 + 1:1, dilation_y = (dilationY)?dilationY->value->n32 + 1:1;

    if ((padX_Right != VX_NULL) && (padY_Bottom != VX_NULL))
    {
        padXLeft    = padX->value->n32;
        padXRight   = padX_Right->value->n32;
        padYTop     = padY->value->n32;
        padYBottom  = padY_Bottom->value->n32;
    }
    else
    {
        padXLeft = padX->value->u32;
        padXRight = padXLeft;
        padYTop = padY->value->u32;
        padYBottom = padYTop;
    }

    batch = (TENSOR_VIEW_SIZE_INDEX(inputs, 3) == 0) ? 1 : TENSOR_VIEW_SIZE_INDEX(inputs, 3);

    vxoTensor_GetTensorViewMemory(inputs, &inputBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogical, VX_NULL);
    if (biases != VX_NULL)
        vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBaseLogical, VX_NULL);

    dataSrc      = (vx_uint8_ptr)inputBaseLogical;
    dataDst      = (vx_uint8_ptr)outputBaseLogical;
    dataWeight   = (vx_uint8_ptr)weightsBaseLogical;
    dataBias     = (vx_uint8_ptr)biasesBaseLogical;
    inputFormat  = (vx_type_e)(TENSOR_DATA_TYPE(inputs));
    weightFormat = (vx_type_e)(TENSOR_DATA_TYPE(weights));
    biasFormat   = (biases != VX_NULL) ? (vx_type_e)(TENSOR_DATA_TYPE(biases)) : VX_TYPE_FLOAT32;
    outputFormat = (vx_type_e)(TENSOR_DATA_TYPE(outputs));

    inputWidth   = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    inputHeight  = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    inputDepth   = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    outputWidth  = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    outputHeight = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    outputDepth  = TENSOR_VIEW_SIZE_INDEX(outputs, 2);

    kernelXSize = TENSOR_VIEW_SIZE_INDEX(weights, 0);
    kernelYSize = TENSOR_VIEW_SIZE_INDEX(weights, 1);

    if (strideX != VX_NULL && strideY != VX_NULL)
    {
        stride_x    = strideX->value->n32;
        stride_y    = strideY->value->n32;
    }
    else
    {

        if (inputWidth == 1 && inputHeight == 1)
        {
            stride_x = 1;
            stride_y = 1;
        }
        else
        {
            /* Calculate stride = (w + padXLeft + padXRight - weight)/(output_w - 1) */
            if (1 == outputWidth)
            {
                stride_x = 1;
            }
            else
            {
                stride_x = vxoNNExternsionConvlutionRound((vx_float32)(inputWidth + padXLeft + padXRight - kernelXSize) / (outputWidth - 1), downScaleSizeRoundingValue);
            }
            if (1 == outputHeight)
            {
                stride_y = 1;
            }
            else
            {
                stride_y = vxoNNExternsionConvlutionRound((vx_float32)(inputHeight + padYTop + padYBottom - kernelYSize) / (outputHeight - 1), downScaleSizeRoundingValue);
            }
        }
    }

    gcmASSERT(stride_x > 0 && stride_y > 0);

    gcoOS_MemFill(outputBaseLogical, 0, outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat));

    for (k = 0; k < batch; k++)
    {
#if DUMP_ONE_PIXEL_CPU_CONV
        vx_int32 my_count;
#endif
        dataSrc    = (vx_uint8_ptr)inputBaseLogical + k * inputWidth * inputHeight * inputDepth * vxnneGetTypeSize(inputFormat);
        dataWeight = (vx_uint8_ptr)weightsBaseLogical;
        dataDst    = (vx_uint8_ptr)outputBaseLogical + k * outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat);

        for (p = 0; p < outputDepth; p ++)
        {
            for (j = 0; j < outputHeight; j ++)
            {
                for (i = 0; i < outputWidth; i ++)
                {
#if DUMP_ONE_PIXEL_CPU_CONV
                    FILE * pfile = NULL;
                    // unit_test will print the index of mismatched pixel. set this to dump exact pixel
                    vx_int32 dump_index = 0x0;
#endif
                    vx_int32 hStart = j * stride_y - padYTop;
                    vx_int32 wStart = i * stride_x - padXLeft;
                    vx_int32 hEnd = gcmMIN(hStart + kernelYSize * dilation_y, inputHeight);
                    vx_int32 wEnd = gcmMIN(wStart + kernelXSize * dilation_x, inputWidth);
                    vx_int32 indexOut = 0;
                    vx_int32 indexBias = 0;
                    vx_int32 h, w = 0;
                    vx_int32 m, n = 0;
                    vx_int32 d;
#if QUANT8_SUPPORT
                    vx_int32 sum = 0;
#else
                    vx_float32 sum = 0;
#endif
                    vx_uint32 kernelXStart = 0, kernelYStart = 0;
#if DUMP_ONE_PIXEL_CPU_CONV
                    my_count = p*outputDepth*outputHeight*outputWidth + j*outputWidth + i;
#endif
                    kernelYStart = hStart < 0 ? (gcmALIGN_NP2_SAFE(-hStart, dilation_y)/dilation_y) : 0;
                    kernelXStart = wStart < 0 ? (gcmALIGN_NP2_SAFE(-wStart, dilation_x)/dilation_x) : 0;

                    if (hStart < 0 && dilation_y > 1)
                        hStart = gcmMAX(hStart, (j * stride_y)%dilation_y);
                    else
                        hStart = gcmMAX(hStart, 0);

                    if (wStart < 0 && dilation_x > 1)
                        wStart = gcmMAX(wStart, (i * stride_x)%dilation_x);
                    else
                        wStart = gcmMAX(wStart, 0);

                    indexOut = j * (outputWidth) + i;
#if DUMP_ONE_PIXEL_CPU_CONV
                    if(my_count == dump_index)
                    {
                        pfile = fopen("one_pixel_conv_dump.txt", "a");
                        if(pfile == NULL)
                        {
                            vxError("openfile error\n");
                        }
                    }
#endif

                    for (d = 0; d < inputDepth; d++)
                    {
                        for (h = hStart, n = kernelYStart; h < hEnd; h += dilation_y, n++)
                        {
                            for (w = wStart, m = kernelXStart; w < wEnd; w += dilation_x, m++)
                            {
                                const vx_int32 indexSrc = d * inputWidth * inputHeight + h * (inputWidth) + w;
                                const vx_int32 indexWeight = d * kernelXSize * kernelYSize + n * kernelXSize + m;
#if QUANT8_SUPPORT
                                if (TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(inputs) == VX_QUANT_AFFINE_SCALE)
                                {
                                    vx_int32 inImg_data = dataSrc[indexSrc] - TENSOR_TF_ZEROPOINT(inputs);
                                    vx_int32 weight_data = dataWeight[indexWeight] - TENSOR_TF_ZEROPOINT(weights);

                                    sum += inImg_data * weight_data;
                                }

#else
                                vx_float32 inImg_data, weight_data;

                                inImg_data = vxnneGetDataExt(inputFormat, TENSOR_QUANT_TYPE(inputs), indexSrc, (vx_uint8_ptr)dataSrc, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                                weight_data = vxnneGetDataExt(weightFormat, TENSOR_QUANT_TYPE(weights), indexWeight, (vx_uint8_ptr)dataWeight, TENSOR_POS(weights), TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights));
                                sum +=  inImg_data* weight_data;
#if DUMP_ONE_PIXEL_CPU_CONV
                                if(pfile != NULL && my_count == dump_index)
                                {
                                    fprintf(pfile, "indexSrc: %d, indexWeight:%d, X:%d, Y:%d, Z:%d\n",
                                        indexSrc, indexWeight, w, h, d);
                                    fprintf(pfile, "float in * float weight = %0.9f * %0.9f = %0.9f. sum:%0.9f\n", inImg_data, weight_data, inImg_data* weight_data, sum);
                                }
#endif
#endif
                            }
                        }
                    }

                    indexBias = p;
#if QUANT8_SUPPORT
                    if (biasFormat == VX_TYPE_FLOAT32 || biasFormat == VX_TYPE_INT32 || biasFormat == VX_TYPE_FLOAT16)
                    {
                        vxmASSERT(gcmABS(TENSOR_TF_SCALE(biases) - TENSOR_TF_SCALE(inputs) * TENSOR_TF_SCALE(weights)) < 0.000001);

                        vx_int32 bias = ((vx_int32_ptr)dataBias)[indexBias];

                        sum += bias;
                    }

                    if (TENSOR_DATA_TYPE(outputs) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(outputs) == VX_QUANT_AFFINE_SCALE)
                    {
                        vx_float32 input_product_scale = TENSOR_TF_SCALE(inputs) * TENSOR_TF_SCALE(weights);
                        vx_int32 shift = 0, multiplier = 0;
                        vx_float32 m = frexp(input_product_scale / TENSOR_TF_SCALE(outputs), &shift);


                        vx_int32 left_shift = shift > 0 ? shift : 0;
                        vx_int32 right_shift = shift > 0 ? 0 : -shift;

                        vx_int32 output = RoundingDivideByPOT(SaturatingRoundingDoublingHighMul(sum * (1 << left_shift), roundf(m * (1LL << 31))), right_shift) + TENSOR_TF_ZEROPOINT(outputs);

                        if (output > 0xff)
                            output = 0xff;
                        else if (output < 0)
                            output = 0;

                        dataDst[indexOut] = output;
                    }
#else

                    if (biasFormat == VX_TYPE_INT64 || biasFormat== VX_TYPE_FLOAT32 || biasFormat == VX_TYPE_INT32 || biasFormat == VX_TYPE_FLOAT16)
                    {
                        if (dataBias != VX_NULL)
                        {
                            if (biasFormat == VX_TYPE_INT32 && TENSOR_QUANT_TYPE(biases) == VX_QUANT_AFFINE_SCALE)
                                vxmASSERT(gcmABS(TENSOR_TF_SCALE(biases) - TENSOR_TF_SCALE(inputs) * TENSOR_TF_SCALE(weights)) < 0.000001);
                            sum += vxnneGetDataExt(biasFormat, TENSOR_QUANT_TYPE(biases), indexBias, (vx_uint8_ptr)dataBias, TENSOR_POS(biases), TENSOR_TF_ZEROPOINT(biases), TENSOR_TF_SCALE(biases));
                        }
                    }
                    else
                    {
                        vxError("can't support this bias data format\n");
                        gcmASSERT(0);
                    }

#if DUMP_ONE_PIXEL_CPU_CONV
                    if(pfile != NULL)
                    {
                        fclose(pfile);
                    }
#endif

                    if (relu)
                        sum = vxnneActivation(relu->value->e, 0, 0, sum);

                    vxnneSaveDataExt(outputFormat, TENSOR_QUANT_TYPE(outputs), indexOut, sum, dataDst, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
#endif
                }
            }

            dataWeight += kernelXSize * kernelYSize * inputDepth * vxnneGetTypeSize(weightFormat);
            dataDst += outputWidth * outputHeight * vxnneGetTypeSize(outputFormat);
        }
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status vxnneExecuteSWConv_UpSample(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation   = (vxnne_convolution_operation)operation;

    vx_tensor inputs        = convOperation->inputs;
    vx_tensor outputs       = convOperation->outputs;
    vx_scalar dilationX     = convOperation->dilationX;
    vx_scalar dilationY     = convOperation->dilationY;

    vx_type_e input_format = (vx_type_e)(TENSOR_DATA_TYPE(inputs));
    vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));

    vx_int32 in_h = TENSOR_SIZE_INDEX(inputs, 1);
    vx_int32 in_w = TENSOR_SIZE_INDEX(inputs, 0);
    vx_int32 in_n = TENSOR_SIZE_INDEX(inputs, 3);

    vx_int32 out_h = TENSOR_SIZE_INDEX(outputs, 1);
    vx_int32 out_w = TENSOR_SIZE_INDEX(outputs, 0);

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_int32 conv_out_channels = TENSOR_SIZE_INDEX(outputs, 2);

    vx_uint8_ptr input_ptr = inputs->tensorBuffer->memory.logicals[0];
    vx_uint8_ptr output_ptr = outputs->tensorBuffer->memory.logicals[0];

    vx_int32 i = 0, j = 0, b = 0;
    vx_int32 input_item_size = vxnneGetTypeSize(input_format);
    vx_int32 output_item_size = vxnneGetTypeSize(output_format);

    gcfVX_Flush(gcvTRUE);

    for (b = 0; b < conv_out_channels; b ++)/*1024*/
    {
        vx_uint8_ptr output_base = output_ptr + b * out_w * out_h * output_item_size;
        vx_uint8_ptr input_base = input_ptr;
        for (j = 0; j < out_h; j ++)/*19*/
        {
            for (i = 0; i < out_w; i ++)/*19*/
            {
                vx_int32 output_index = j * out_w + i;
                vx_int32 group_x = i % dilation_x, group_y = j % dilation_y;
                vx_int32 input_index = (j / dilation_y) * in_w + (i / dilation_x);
                vx_float32 input_value = 0;

                if (in_n == 1)
                    input_base = input_ptr + ((group_y * dilation_x + group_x) * in_w * in_h + b * in_w * in_h * dilation_x * dilation_y) * input_item_size;
                else
                    input_base = input_ptr + ((group_y * dilation_x + group_x) * in_w * in_h * conv_out_channels + b * in_w * in_h) * input_item_size;

                input_value = vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(inputs), input_index, (vx_uint8_ptr)input_base, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                vxnneSaveDataExt(output_format, TENSOR_QUANT_TYPE(outputs), output_index, input_value, output_base, TENSOR_POS(outputs), TENSOR_TF_ZEROPOINT(outputs), TENSOR_TF_SCALE(outputs), TENSOR_ROUNDING_MODE(outputs));
            }
        }
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneExecuteSWConv_Reshuffle(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_deconvolution_reshuffle_operation convOperation   = (vxnne_deconvolution_reshuffle_operation)operation;

    vx_tensor inputs                = convOperation->inputs;
    vx_tensor weights               = convOperation->weights;
    vx_int32 dilation_w             = convOperation->stride_x->value->n32 + 1;
    vx_int32 dilation_h             = convOperation->stride_y->value->n32 + 1;
    vx_int32 pad_x_left             = convOperation->padding_x_left->value->n32;
    vx_int32 pad_x_right            = convOperation->padding_x_right->value->n32;
    vx_int32 pad_y_top              = convOperation->padding_y_top->value->n32;
    vx_int32 pad_y_bottom           = convOperation->padding_y_bottom->value->n32;

    vx_int32 i = 0, j = 0, w = 0, h = 0, dx = 0, dy = 0, c = 0;

    vx_int32 padding_x_left   = pad_x_left/dilation_w;
    vx_int32 padding_x_right  = pad_x_right/dilation_w;
    vx_int32 padding_y_top    = pad_y_top/dilation_h;
    vx_int32 padding_y_bottom = pad_y_bottom/dilation_h;

    if (convOperation->weights_biaes)
        return status;

    if (inputs)
    {
        vx_type_e input_format = (vx_type_e)(TENSOR_DATA_TYPE(inputs));
        vx_int32 item_size = vxnneGetTypeSize(input_format);

        vx_uint8_ptr inputs_ptr         = inputs->tensorBuffer->memory.logicals[0];
        vx_tensor reshuffle_inputs      = convOperation->reshuffled_inputs;
        vx_uint8_ptr reshuffled_inputs  = reshuffle_inputs->tensorBuffer->memory.logicals[0];
        vx_uint8_ptr data = reshuffled_inputs, buffer = VX_NULL;

        vx_int32 input_w = TENSOR_SIZE_INDEX(inputs, 0);
        vx_int32 input_h = TENSOR_SIZE_INDEX(inputs, 1);
        vx_int32 input_c = TENSOR_SIZE_INDEX(inputs, 2);
        vx_int32 input_n = TENSOR_SIZE_INDEX(inputs, 3);

        vx_int32 reshuffle_width = gcmALIGN_NP2(input_w, dilation_w)/dilation_w, reshuffle_height = gcmALIGN_NP2(input_h, dilation_h)/dilation_h;
        vx_int32 slice_size = input_w * input_h, reshuffled_slice_size = reshuffle_width * reshuffle_height;
        vx_int32 batch = input_n;

        gcoOS_MemFill(reshuffled_inputs, 0, item_size * reshuffled_slice_size * input_c * dilation_w * dilation_h);

        if (convOperation->reshuffled)
            vxMemCopy(reshuffled_inputs, inputs_ptr, item_size * slice_size * input_c * batch);
        else
        {
            buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(item_size * slice_size * input_c * batch);
            data = reshuffled_inputs;


            for (dy = 0; dy < dilation_h; dy++)
            {
                for (dx = 0; dx < dilation_w; dx++)
                {
                    vx_int32 group_index = reshuffled_slice_size * input_c * (dy * dilation_w + dx);

                    for (c = 0; c < input_c; c++)
                    {
                        vx_int32 slice_index = reshuffled_slice_size * c;
                        vx_uint8_ptr reshuffed_data = reshuffled_inputs + (group_index + slice_index) * item_size;
                        vx_uint8_ptr input_data = inputs_ptr + slice_size * c * item_size;

                        for (h = dy - pad_y_top, j = - pad_y_top/dilation_h; h < input_h; h += dilation_h, j++)
                        {
                            for (w = dx - pad_x_left, i = -pad_x_left/dilation_w; w < input_w; w += dilation_w, i ++)
                            {
                                vx_float32 in_data = 0;
                                if (w >= 0 && h >= 0 && i >= 0 && j >= 0)
                                {
                                    //reshuffed_data[j * reshuffle_width + i] = input_data[h * input_w + w];
                                    in_data = vxnneGetDataExt(input_format, TENSOR_QUANT_TYPE(inputs), h * input_w + w, input_data, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs));
                                    vxnneSaveDataExt(input_format, TENSOR_QUANT_TYPE(inputs), j * reshuffle_width + i, in_data, reshuffed_data, TENSOR_POS(inputs), TENSOR_TF_ZEROPOINT(inputs), TENSOR_TF_SCALE(inputs), TENSOR_ROUNDING_MODE(inputs));
                                }


                            }
                        }
                    }
                }
            }

            vxFree(buffer);
        }


    }
    if (weights && convOperation->reshuffled_weights)
    {
        vx_tensor biases             = convOperation->bias;
        vx_tensor reshuffled_biases  = convOperation->reshuffled_biases;
        vx_tensor reshuffled_weights = convOperation->reshuffled_weights;
        vx_uint8_ptr weights_ptr     = weights->tensorBuffer->memory.logicals[0];
        vx_uint8_ptr reshuffled_weights_ptr = reshuffled_weights->tensorBuffer->memory.logicals[0];
        vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));
        vx_int32 item_size = vxnneGetTypeSize(weight_format);


        vx_int32 kernel_w = TENSOR_SIZE_INDEX(weights, 0);
        vx_int32 kernel_h = TENSOR_SIZE_INDEX(weights, 1);
        vx_int32 kernel_c = TENSOR_SIZE_INDEX(weights, 2);
        vx_int32 kernel_n = TENSOR_SIZE_INDEX(weights, 3);

        vx_int32 reshuffled_kernel_w = TENSOR_SIZE_INDEX(reshuffled_weights, 0);
        vx_int32 reshuffled_kernel_h = TENSOR_SIZE_INDEX(reshuffled_weights, 1);

        gcmASSERT(reshuffled_weights_ptr);

        for (j = 0; j < kernel_n; j ++)/*84*/
        {
            vx_int32 orginal_index = j * item_size * kernel_h * kernel_w * kernel_c;
            for (i = 0; i < dilation_w * dilation_h; i ++)/* 2x2=4 */
            {
                for (c = 0; c < dilation_w * dilation_h; c ++)/* 2x2=4 */
                {
                    vx_int32 reshuffled_index = (j * dilation_w * dilation_h * dilation_w * dilation_h + i * (dilation_w * dilation_h) + c) * item_size * reshuffled_kernel_h * reshuffled_kernel_w * kernel_c;
                    if (c == i)
                        memcpy(reshuffled_weights_ptr + reshuffled_index, weights_ptr + orginal_index, item_size * reshuffled_kernel_h * reshuffled_kernel_w * kernel_c);
                    else
                        memset(reshuffled_weights_ptr + reshuffled_index, 0, item_size * reshuffled_kernel_h * reshuffled_kernel_w * kernel_c);
                }
            }
        }

        if (biases && reshuffled_biases)
        {
            vx_type_e biases_format = (vx_type_e)(TENSOR_DATA_TYPE(biases));
            vx_int32 bias_c = TENSOR_SIZE_INDEX(biases, 3);
            vx_int32 r_bias_c = TENSOR_SIZE_INDEX(reshuffled_biases, 3);

            if (bias_c != r_bias_c)
            {
                vx_float32 bias = .0f;
                for (i = 0; i < r_bias_c; i ++)
                {
                    bias = vxnneGetDataExt(biases_format, TENSOR_QUANT_TYPE(biases), i/(dilation_w * dilation_h), biases->tensorBuffer->memory.logicals[0], TENSOR_POS(reshuffled_biases), TENSOR_TF_ZEROPOINT(biases), TENSOR_TF_SCALE(biases));
                    vxnneSaveDataExt(biases_format, TENSOR_QUANT_TYPE(reshuffled_biases), i, bias, reshuffled_biases->tensorBuffer->memory.logicals[0], TENSOR_POS(reshuffled_biases), TENSOR_TF_ZEROPOINT(reshuffled_biases), TENSOR_TF_SCALE(reshuffled_biases), TENSOR_ROUNDING_MODE(reshuffled_biases));

                }

            }
        }
    }

    if (weights && convOperation->create_wbp && (convOperation->weights_biaes == VX_NULL))
    {
        convOperation->weights_biaes = _createWeightsBiasesParameterFromTensors(
                            vxGetContext((vx_reference)convOperation->weights),
                            VX_NN_CONVOLUTION_LAYER,
                            convOperation->reshuffled_inputs->dims,/*inputs_dims,*/
                            convOperation->reshuffled_inputs->dimCount,
                            convOperation->reshuffled_inputs->dimCount,
                            padding_x_left, padding_x_right, padding_y_top, padding_y_bottom,
                            0,/*pooling_size_x,*/
                            0,/*pooling_size_y,*/
                            0,
                            0,
                            VX_NN_DS_SIZE_ROUNDING_FLOOR,
                            convOperation->outputs->dims,/*convolution_outputs_dims,*/
                            convOperation->outputs->dims,/*pool_outputs_dims,*/
                            convOperation->opt, /*optimizations,*/
                            TENSOR_DATA_TYPE(weights),
                            0,
                            VX_TENSOR_RANK_WHCN,
                            convOperation->reshuffled_weights?convOperation->reshuffled_weights:convOperation->weights,
                            convOperation->reshuffled_biases?convOperation->reshuffled_biases:convOperation->bias,
                            VX_NULL,
                            vx_false_e,
                            vx_false_e
                            );

        convOperation->create_wbp = vx_false_e;
    }

    return status;
}

VX_PRIVATE_API vx_tensor vxoNNTensor_ReorgWeights(vx_tensor weights, vx_graph graph, vx_uint32 input_size, vx_uint32 ofm)
{
    vx_tensor_create_params_t tensor_create_params;
    vx_tensor    weight_in      = NULL;
    vx_tensor    weight_out     = NULL;
    vx_tensor    weights_new_rs = NULL;
    vx_uint32    sizes[4]       = {1};
    vx_uint32    perm[4]        = {0, 2, 1, 3};
    vx_uint32    pnum           = 4;
    vx_uint8_ptr inaddr         = NULL;
    vx_uint8_ptr outaddr        = NULL;
    vx_uint32    dims           = 0;
    vx_uint32 _dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32  tensorSz = 0;
    vx_context context = vxGetContext((vx_reference)weights);


    /*permute input fc weight */
    sizes[0]        = 4;
    sizes[1]        = input_size / 4;
    sizes[2]        = 4;
    sizes[3]        = ofm / 4;
    dims            = 4;

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = TENSOR_DATA_TYPE(weights);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
    }

    weight_in       = vxoTensor_CreateTensor(context, graph, &tensor_create_params, vx_false_e);
    if (vxoTensor_AllocateMemory(weight_in) != VX_SUCCESS)
    {
        vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
        return VX_NULL;
    }

    sizes[0]        = 4;
    sizes[1]        = 4;
    sizes[2]        = input_size / 4;
    sizes[3]        = ofm / 4;
    dims            = 4;
    weight_out      = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);

    vxoTensor_GetTensorViewMemory(weight_in, (gctPOINTER*)&inaddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weight_out, (gctPOINTER*)&outaddr, VX_NULL);

    vxoTensor_GetTensorDimStride(weight_in, &pnum, _dims, strides);
    vxoTensor_GetTensorDimStride(weight_out, &pnum, VX_NULL, tstrides);

    vxoTensor_GetTensorSize(weight_in, &tensorSz);
    memcpy(inaddr, outaddr, tensorSz);

    _TransposeTensor(inaddr, outaddr, TENSOR_DATA_SIZE(weight_in), _dims, strides, tstrides, perm, pnum - 1);

    if (weight_in) vxoTensor_ReleaseTensor(&weight_in);
    if (weight_out) vxoTensor_ReleaseTensor(&weight_out);

    sizes[0]        = input_size * 4;
    sizes[1]        = 1;
    sizes[2]        = 1;
    sizes[3]        = ofm / 4;
    dims            = 4;
    weights_new_rs  = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);

    return weights_new_rs;
}

VX_PRIVATE_API vx_status vxnneTensorConstPad(vx_tensor src, vx_tensor dst, vx_uint32 left, vx_uint32 right, vx_uint32 top, vx_uint32 bottom, vx_uint32 constant)
{
    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vx_int8 srcFp = TENSOR_POS(src);
    vx_int8 dstFp = TENSOR_POS(dst);
    vx_enum  dstRoundingMode = TENSOR_ROUNDING_MODE(dst);

    vx_uint32 dstSize;

    vx_uint32 inWidth           = TENSOR_VIEW_SIZE_INDEX(src, 0);
    vx_uint32 inHeight          = TENSOR_VIEW_SIZE_INDEX(src, 1);
    vx_uint32 outWidth          = TENSOR_VIEW_SIZE_INDEX(dst, 0);
    vx_uint32 outHeight         = TENSOR_VIEW_SIZE_INDEX(dst, 1);
    vx_uint32 ofm               = TENSOR_VIEW_SIZE_INDEX(dst, 2);
    vx_uint32 batch             = TENSOR_VIEW_SIZE_INDEX(dst, 3);
    vx_uint32 x, y, z, w;

    if (inHeight + top + bottom != outHeight ||
        inWidth + left + right != outWidth)
    {
        vxmASSERT(gcvFALSE);
    }

    vxoTensor_GetTensorViewMemory(src, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dst, &dstLogical, VX_NULL);

    vxoTensor_GetTensorSize(dst,&dstSize);

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

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status vxnneExecuteSWConv_Convolution_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation   = (vxnne_convolution_operation)operation;

    if (convOperation->inputs)
        vxoTensor_ReleaseTensor(&convOperation->inputs);

    if (convOperation->padX)
        vxReleaseScalar(&convOperation->padX);

    if (convOperation->padY)
        vxReleaseScalar(&convOperation->padY);

    if (convOperation->downScaleSizeRounding)
        vxReleaseScalar(&convOperation->downScaleSizeRounding);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWConv_Convolution_DeInilition2(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation   = (vxnne_convolution_operation)operation;

    vxnneExecuteSWConv_Convolution_DeInilition(operation);

    if (convOperation->weights)
        vxoTensor_ReleaseTensor(&convOperation->weights);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWConv_UpSample_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_operation convOperation   = (vxnne_convolution_operation)operation;

    if (convOperation->inputs)
        vxoTensor_ReleaseTensor(&convOperation->inputs);

    return status;
}

VX_PRIVATE_API vx_status vxnneExecuteSWConv_Reshuffle_DeInilition(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;


    return status;
}

typedef enum _gcoNNConv_Mode
{
    gcoNNE_CONV_MODE_SW,
    gcoNNE_CONV_MODE_SW1,/*dilation_x x dilation_y convolution*/
    gcoNNE_CONV_MODE_SW2,/*1 convolution*/
    gcoNNE_CONV_MODE_NNE_TP,/*1 convolution*/
    gcoNNE_CONV_MODE_NNE_TP2,/*dilation_x x dilation_y convolution*/
    gcoNNE_CONV_MODE_SH,
}
gcoNNConv_Mode;

enum
{
    gcoNNE_CONV_RESHUFFLED_INPUTS = 0,
    gcoNNE_CONV_RESHUFFLED_OUTPUTS,
    gcoNNE_CONV_RESHUFFLED_WEIGHTS,
    gcoNNE_CONV_RESHUFFLED_BIASES,
};

#define DILATION_SELECT_ENV 1

#define QUANT8CHECK(tensor1, tensor2) \
     if (TENSOR_DATA_TYPE(tensor1) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(tensor1) == VX_QUANT_AFFINE_SCALE) \
     { \
         TENSOR_TF_ZEROPOINT(tensor2) = TENSOR_TF_ZEROPOINT(tensor1); \
         TENSOR_TF_SCALE(tensor2) = TENSOR_TF_SCALE(tensor1); \
         TENSOR_QUANT_TYPE(tensor2) = TENSOR_QUANT_TYPE(tensor1); \
         TENSOR_POS(tensor2) = TENSOR_POS(tensor1); \
         TENSOR_PAD_ZERO_VALUE(tensor2) = TENSOR_PAD_ZERO_VALUE(tensor1); \
     }

vx_status vxoNNDilationConvolutionLayer_Deinitialize(vxnne_layer layer)
{
    vxnne_convolution_layer   convolutionLayer = (vxnne_convolution_layer)layer;

    vxnneExecutionLayer_Deinitialize(layer);

    if (convolutionLayer->dynamic_operations)
        gcoOS_Free(VX_NULL, convolutionLayer->dynamic_operations);

    return VX_SUCCESS;
}
#if REGISTER_FRAME
VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayer_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[2];
    vx_tensor biases                = (vx_tensor)parameters[3];
    vx_scalar padX                  = (vx_scalar)parameters[4];
    vx_scalar padXRight             = (vx_scalar)parameters[5];
    vx_scalar padY                  = (vx_scalar)parameters[6];
    vx_scalar padYBottom            = (vx_scalar)parameters[7];
    vx_scalar dilationX             = (vx_scalar)parameters[10];
    vx_scalar dilationY             = (vx_scalar)parameters[11];

    vx_scalar relu_s                = (vx_scalar)parameters[14];
    vx_scalar pooling_s             = (vx_scalar)parameters[15];

    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs               = (vx_tensor)parameters[19];

    vxnne_convolution_layer  convolutionLayer = (vxnne_convolution_layer)ops_layer;

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vx_bool relu = relu_s?relu_s->value->b:vx_false_e;

    vx_enum pooling = pooling_s?pooling_s->value->e:(VX_NN_POOLING_MAX-1);

    vx_uint32 idx = 0;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    if(relu != vx_false_e)gcmASSERT("SW Conv Not support relu!");
    if((pooling == VX_NN_POOLING_MAX) || (pooling == VX_NN_POOLING_AVG))
        gcmASSERT("SW Conv Not support Pooling!");

    vxmONERROR(vxnneOperation_Initialize(&convolutionLayer->convolutionSW.base,
                            &convolutionLayer->base,
                            VXNNE_OPERATION_TARGET_SW,
                            VXNNE_OPERATOR_CONVOLUTION,
                            vxnneExecuteSWConvolution,
                            VX_NULL,
                            batchCount,
                            0));

    vxmONERROR(vxnneLayer_SetOperation(
        &convolutionLayer->base,
        &convolutionLayer->convolutionSW.base,
        idx++));

    convolutionLayer->convolutionSW.inputs                = inputs;
    convolutionLayer->convolutionSW.weights               = weights;
    convolutionLayer->convolutionSW.biases                = biases;
    convolutionLayer->convolutionSW.padX                  = padX;
    convolutionLayer->convolutionSW.padXRight             = padXRight;
    convolutionLayer->convolutionSW.padY                  = padY;
    convolutionLayer->convolutionSW.padYBottom            = padYBottom;
    convolutionLayer->convolutionSW.dilationX             = dilationX;
    convolutionLayer->convolutionSW.dilationY             = dilationY;
    convolutionLayer->convolutionSW.downScaleSizeRounding = downScaleSizeRounding;
    convolutionLayer->convolutionSW.outputs               = outputs;

    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNDilationConvolutionLayer_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[2];
    vx_tensor biases                = (vx_tensor)parameters[3];
    vx_scalar padXLeft              = (vx_scalar)parameters[4];
    vx_scalar padXRight             = (vx_scalar)parameters[5];
    vx_scalar dilationX             = (vx_scalar)parameters[10];
    vx_scalar dilationY             = (vx_scalar)parameters[11];

    vx_scalar pooling_s             = (vx_scalar)parameters[15];
    vx_scalar poolingx              = (vx_scalar)parameters[16];
    vx_tensor outputs               = (vx_tensor)parameters[19];

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_uint32  width             = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32  height            = TENSOR_VIEW_SIZE_INDEX(outputs, 1);

    vx_enum    inputFormat       = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat      = TENSOR_DATA_TYPE(outputs);
    //vx_bool    enable_shader     = vx_false_e;
    vx_bool    has_pool          = (pooling_s == NULL && poolingx == 0) ? vx_false_e : vx_true_e;
    vx_bool    enable_2dTensor   = vx_false_e;

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    if (weights != NULL)
    {
        vx_uint32 kernel_x            = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        vx_uint32 kernel_y            = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        vx_uint32 ifm                 = TENSOR_VIEW_SIZE_INDEX(weights, 2);
        vx_enum   weightFormat        = TENSOR_DATA_TYPE(weights);
        vx_enum   biasFormat          = biases ? TENSOR_DATA_TYPE(biases) : VX_TYPE_FLOAT64;
        vx_uint32 size                = width * height;

        if(evis)
        {
            vx_uint32 convsize              = kernel_x * kernel_y * ifm;
            vx_bool   support_type          = vx_false_e;
            vx_bool   is_cross_width_read   = vx_false_e;
            vx_int32  paddingLeft           = padXLeft->value->n32;
            vx_int32  paddingRight          = padXRight->value->n32;
            vx_int32  input_width           = TENSOR_VIEW_SIZE_INDEX(inputs, 0);

            is_cross_width_read = (vx_bool)(paddingLeft > 0 && paddingRight > 0
                                        && (input_width + paddingLeft) <= 8);

            if (biases)
            {
                support_type    = (vx_bool)(
                    (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_INT8  && weightFormat == VX_TYPE_INT8  && biasFormat == VX_TYPE_INT32 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT64 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_INT32 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_UINT8 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16));

                support_type = support_type && (is_cross_width_read == vx_false_e);
            }
            else
            {
                support_type    = (vx_bool)(
                    (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_INT8  && weightFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_INT8)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
            }

            support = (vx_bool)(convsize < IMG_MAX_WIDTH && support_type && has_pool == vx_false_e);
            enable_2dTensor = (vx_bool)(size < IMG_MAX_WIDTH && dilation_x == 1 && dilation_y == 1);
        }
        else
        {
            vx_bool   support_type    = vx_false_e;

            if (biases)
            {
                support_type    = (vx_bool)
                    ((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT64 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_UINT8)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16));
            }
            else
            {
                support_type    = (vx_bool)
                    ((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
            }

            /*enable_2dTensor = (vx_bool)(size < IMG_MAX_WIDTH && dilation_x == 1 && dilation_y == 1);*/
            support = (vx_bool)(support_type && has_pool == vx_false_e);
        }

        if (!support)return support;

        if (TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_DYNAMIC)
            support = vx_true_e;
    }

    if (support)
    {
        //reg_param->flag = gcoNNE_CONV_MODE_SH;
        SETBIT(reg_param->flag, enable_2dTensor, 0);
    }

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNDilationConvolutionLayer_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNDilationConvolutionLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayer_SH_EVIS_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;

    vxnne_convolution_layer  convolutionLayer = (vxnne_convolution_layer)ops_layer;

    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases= (vx_weights_biases_parameter)parameters[1];
    vx_tensor weights               = (vx_tensor)parameters[2];
    vx_tensor biases                = (vx_tensor)parameters[3];
    vx_scalar padXLeft              = (vx_scalar)parameters[4];
    vx_scalar padXRight             = (vx_scalar)parameters[5];
    vx_scalar padYTop               = (vx_scalar)parameters[6];
    vx_scalar padYBottom            = (vx_scalar)parameters[7];
    vx_scalar dilationX             = (vx_scalar)parameters[10];
    vx_scalar dilationY             = (vx_scalar)parameters[11];
    vx_scalar stridesX              = (vx_scalar)parameters[12];
    vx_scalar stridesY              = (vx_scalar)parameters[13];

    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[18];
    vx_tensor outputs               = (vx_tensor)parameters[19];

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_bool    enable_2dTensor   = GETBIT(reg_param->flag, 0);

    vx_context context = vxGetContext((vx_reference)inputs);
    vx_uint32 idx = 0;
    vx_uint32 numTmpTensor = 0;

#define CONV2D_ALIGN_SIZE4 (4)
#define CONV2D_ALIGN_SIZE16 (16)
    vx_tensor tensor2Row    = NULL;
    vx_uint32 sizes[]       = {1, 1, 1, 1};
    vx_uint32 dims          = TENSOR_DIM_NUM(inputs);
    vx_enum   downScaleSizeRoundingValue = downScaleSizeRounding->value->e;
    vx_int32  paddingLeft   = padXLeft->value->n32;
    vx_int32  paddingRight  = padXRight->value->n32;
    vx_int32  paddingTop    = padYTop->value->n32;
    vx_int32  paddingBottom = padYBottom->value->n32;
    vx_int32  k_w           = weights ? TENSOR_VIEW_SIZE_INDEX(weights, 0):WB_ORG_KERNEL_X(weights_biases);
    vx_int32  k_h           = weights ? TENSOR_VIEW_SIZE_INDEX(weights, 1):WB_ORG_KERNEL_Y(weights_biases);
    vx_int32  inputWidth    = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_int32  inputHeight   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_int32  inputDepth    = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    vx_int32  outputWidth   = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_int32  outputHeight  = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_int32  outputDepth   = TENSOR_VIEW_SIZE_INDEX(outputs, 2);
    vx_uint32 input_size    = k_w * k_h * inputDepth;
    /* Calculate stride     = (w + padXLeft + padXRight - weight)/(output_w - 1) */
    vx_int32  strideX       = (stridesX != VX_NULL) ? (stridesX->value->n32) : (vxoNNExternsionConvlutionRound((vx_float32)(inputWidth + paddingLeft + paddingRight - k_w) / (outputWidth - 1), downScaleSizeRoundingValue));
    vx_int32  strideY       = (stridesY != VX_NULL) ? (stridesY->value->n32) : (vxoNNExternsionConvlutionRound((vx_float32)(inputHeight + paddingTop + paddingBottom - k_h) / (outputHeight - 1), downScaleSizeRoundingValue));
    vx_bool   enableAlign4  = vx_false_e;
    vx_bool   enableAlign16  = vx_false_e;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_tensor_create_params_t tensor_create_params;
    vx_tensor    newBiases                      = NULL;
    vx_tensor    weights_new_rs                 = NULL;
    vx_tensor    input_rs                       = NULL;
    vx_tensor    outputs_rs                     = NULL;
    vx_bool      is_static_weights_biases       = vx_false_e;
    vx_bool      enable_adjust_biases           = vx_false_e;
    vx_bool      enable_conv2d_1x1              = vx_false_e;
    vx_bool      is_conv_3x3_s2                 = vx_false_e;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    // step 1, tensor to row
    if (enable_2dTensor)
    {
        sizes[0] = k_w * k_h * inputDepth;
        sizes[1] = outputWidth * outputHeight;
        sizes[2] = 1;
        sizes[3] = batchCount;
        dims = gcmMAX(2, TENSOR_DIM_NUM(outputs));
    }
    else
    {
        sizes[0] = k_w * k_h * inputDepth;
        sizes[1] = outputWidth;
        sizes[2] = outputHeight;
        sizes[3] = batchCount;
        dims = 4;
    }

    if (evis == vx_false_e
        && ((inputWidth * inputHeight < IMG_MAX_WIDTH) && inputDepth < IMG_MAX_WIDTH)
        && (k_w == k_h && k_w == 1)
        && (strideX == strideY && strideX == 1)
        && (paddingLeft == paddingRight && paddingLeft == 0)
        && (paddingTop == paddingBottom && paddingTop == 0)
        && biases != NULL
        && (CHECK_LIFETIME_IS_STATIC(weights) || TENSOR_QUANT_TYPE(inputs) != VX_QUANT_AFFINE_SCALE)
        /*&& ((inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 == 0) || (input_size % CONV2D_ALIGN_SIZE16 != 0) || TENSOR_QUANT_TYPE(inputs) != VX_QUANT_AFFINE_SCALE)*/
        )
    {
        enable_conv2d_1x1 = vx_true_e;
    }

    if (k_w == 3 && k_h == 3 && paddingLeft == 0 && strideX == 2 && dilation_x == 1 && dilation_y == 1)
    {
        is_conv_3x3_s2 = vx_true_e;
    }

    if (evis == vx_false_e
        && (sizes[0] % CONV2D_ALIGN_SIZE4) != 0 && CHECK_LIFETIME_IS_STATIC(weights))
    {
        if (is_conv_3x3_s2)
        {
            sizes[0] = gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16);
            enableAlign16 = vx_true_e;
        }
        else
        {
            sizes[0] = gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE4);
        }
        enableAlign4 = vx_true_e;
        input_size = sizes[0];
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
    }

    if (enable_conv2d_1x1 == vx_false_e)
    {
        if (enableAlign4)
        {
            tensor2Row = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
            if (tensor2Row == VX_NULL || vxoTensor_AllocateMemory(tensor2Row) != VX_SUCCESS)
            {
                vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto OnError;
            }
        }
        else
        {
            tensor2Row = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_true_e);
            if (tensor2Row == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto OnError;
            }
        }
    }

    if(evis)
    {
        shaderExecutable = vxnneTensor2RowShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR2ROW, &ops_layer->node->kernelAttributes.borderMode,
                           inputs, k_w, k_h, dilation_x, dilation_y, strideX, strideY, paddingLeft, paddingTop, outputWidth, outputHeight, tensor2Row);
    }
    else
    {
        if (enable_conv2d_1x1 == vx_false_e)
        {
            shaderExecutable = vxnneGPUTensor2RowShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR2ROW, &ops_layer->node->kernelAttributes.borderMode,
                           inputs, k_w, k_h, dilation_x, dilation_y, strideX, strideY, paddingLeft, paddingTop, outputWidth, outputHeight, tensor2Row);
        }
    }

    if (enable_conv2d_1x1 == vx_false_e)
    {
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            return status;
        }

        status = vxnneShaderOperation_Initialize(&convolutionLayer->convolutionTensor2Row_sh_operation,
            &convolutionLayer->base,
            VXNNE_OPERATOR_CONVOLUTION,
            batchCount,
            shaderExecutable);

        vxnneLayer_SetOperation(
                &convolutionLayer->base,
                &convolutionLayer->convolutionTensor2Row_sh_operation.base,
                idx++);

        vxnneOperation_AddReference(&convolutionLayer->convolutionTensor2Row_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolutionTensor2Row_sh_operation.base, (vx_reference)tensor2Row, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

    // step 2, gemm
    {
        is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights) && CHECK_LIFETIME_IS_STATIC(biases));
        enable_adjust_biases     = is_static_weights_biases && TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE && TENSOR_QUANT_TYPE(biases);
        enable_adjust_biases     = enable_adjust_biases && (TENSOR_DATA_TYPE(biases) != VX_TYPE_UINT8);

        if (enable_adjust_biases)
        {
            vx_tensor_create_params_t params;
            gctPOINTER weightsLogical   = VX_NULL;
            gctPOINTER biasesLogical    = VX_NULL;
            gctPOINTER newBiasesLogical = VX_NULL;
            vx_uint32  i                = 0;
            vx_uint32  j                = 0;
            vx_uint32  sizes[4]         = {1};
            vx_uint32  ofm              = TENSOR_VIEW_SIZE_INDEX(biases, 0);
            vx_uint32  ifm              = k_w * k_h * inputDepth;

            sizes[0] = TENSOR_VIEW_SIZE_INDEX(biases, 0);
            sizes[1] = 1;

            gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));
            params.num_of_dims = TENSOR_DIM_NUM(biases);
            params.sizes = sizes;
            params.data_format = TENSOR_DATA_TYPE(biases);
            params.quant_format = TENSOR_QUANT_TYPE(biases);
            if (params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                params.quant_data.dfp.fixed_point_pos = TENSOR_POS(biases);
            }
            else
            {
                params.quant_data.affine.scale = TENSOR_TF_SCALE(biases);
            }

            newBiases = vxoTensor_CreateTensor(context, NULL, &params, vx_false_e);
            convolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
            vxoTensor_AllocateMemory(newBiases);
            if (newBiases == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto OnError;
            }

            TENSOR_DATA_LIFETIME(newBiases) = VX_TENSOR_LIFE_TIME_STATIC;

            vxoTensor_GetTensorViewMemory(weights, &weightsLogical, VX_NULL);
            vxoTensor_GetTensorViewMemory(biases, &biasesLogical, VX_NULL);
            vxoTensor_GetTensorViewMemory(newBiases, &newBiasesLogical, VX_NULL);

            for (j = 0; j < ofm; j++)
            {
                vx_int32 offset = 0;
                vx_int32 dstVal = 0;

                for (i = 0; i < ifm; i++)
                {
                    vx_uint32 idx = j * ifm + i;
                    offset = offset - (((vx_uint8*)weightsLogical)[idx] - TENSOR_TF_ZEROPOINT(weights)) * TENSOR_TF_ZEROPOINT(inputs);
                }

                dstVal = ((vx_int32*)biasesLogical)[j] + offset;
                ((vx_int32*)newBiasesLogical)[j] = dstVal;
            }

            convolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
        }
        else
        {
            newBiases = biases;
        }
    }

    if(evis)
    {
        weights_new_rs = weights;
        input_rs = tensor2Row;
        outputs_rs = outputs;
        if (biases)
        {
            shaderExecutable = vxnneGemmShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GEMM, &ops_layer->node->kernelAttributes.borderMode, tensor2Row, weights_new_rs, newBiases, VX_NN_ACTIVATION_NONE, enable_2dTensor, outputs_rs);
        }
        else
        {
            shaderExecutable = vxnneGemm_noBiasShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GEMM_NOBIAS, &ops_layer->node->kernelAttributes.borderMode, tensor2Row, weights_new_rs, VX_NN_ACTIVATION_NONE, enable_2dTensor, outputs_rs);
        }
    }
    else
    {
        vx_bool   enable_tensor_cast  = vx_false_e;
        vx_bool   enable_packed_weights = vx_false_e;
        vx_bool   enable_ofm_gt_xy    = vx_false_e;
        vx_tensor weights_rs          = NULL;
        vx_tensor weights_new         = NULL;

        sizes[0] = enableAlign4 ? gcmALIGN(k_w * k_h * inputDepth, CONV2D_ALIGN_SIZE4) : (k_w * k_h * inputDepth);
        sizes[0] = enableAlign16 ? gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16) : sizes[0];
        sizes[1] = outputWidth;
        sizes[2] = outputHeight;
        sizes[3] = batchCount;
        dims = 4;

        if (enable_conv2d_1x1
         && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 == 0)
         && TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8
         && TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8
         )
        {
            if (/*inputWidth * inputHeight < inputDepth
              && */(inputWidth * inputHeight % CONV2D_ALIGN_SIZE16 != 0))
            {
                if ((outputDepth % CONV2D_ALIGN_SIZE16 == 0) && (input_size % CONV2D_ALIGN_SIZE4 == 0))
                {
                     enable_packed_weights = vx_true_e;
                }
                else
                {
                     enable_ofm_gt_xy = vx_true_e;
                }
            }
            else
            {
                enable_tensor_cast = vx_true_e;

                if ((outputDepth % CONV2D_ALIGN_SIZE16 == 0)
                    && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                    && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE16 == 0))
                {
                     enable_packed_weights = vx_true_e;
                }
            }
        }
        else if (enable_conv2d_1x1
                 /*&& (inputWidth * inputHeight < inputDepth)*/
                 && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 != 0)
                 && (outputDepth % CONV2D_ALIGN_SIZE16 == 0)
                 && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                 && TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8
                 && TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8
                 )
        {
            enable_packed_weights = vx_true_e;
        }
        else if (enable_conv2d_1x1
                 && (outputDepth % CONV2D_ALIGN_SIZE4 == 0)
                 && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                 && TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16
                 && TENSOR_DATA_TYPE(weights) == VX_TYPE_FLOAT16
                 )
        {
            enable_packed_weights = vx_true_e;
        }
        else if (!enable_conv2d_1x1 && biases != NULL
            && CHECK_LIFETIME_IS_STATIC(weights)
            && ((outputWidth * outputHeight < IMG_MAX_WIDTH) && outputDepth < IMG_MAX_WIDTH && input_size < IMG_MAX_WIDTH)
            && (outputDepth % CONV2D_ALIGN_SIZE4 == 0)
            && ((TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE && TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8)
            || (TENSOR_QUANT_TYPE(weights) == VX_QUANT_NONE && TENSOR_DATA_TYPE(weights) == VX_TYPE_FLOAT16)))
        {
            enable_packed_weights = vx_true_e;
        }

        if (enable_conv2d_1x1)
        {
            sizes[0] = inputWidth * inputHeight;
            sizes[1] = 1;
            sizes[2] = inputDepth;
            sizes[3] = batchCount;

            if (enable_tensor_cast)
            {
                vx_tensor t = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, TENSOR_DIM_NUM(inputs));

                input_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);

                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                convolutionLayer->base.temp_tensors[numTmpTensor++] = input_rs;
            }
            else
                input_rs = inputs;
        }
        else
        {
            input_rs = tensor2Row;
        }

        if (enableAlign4)
        {
            vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
            vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
            vx_uint32 ifm_rs = 0;

            sizes[0] = ifm;
            sizes[1] = ofm;
            dims     = 2;
            weights_rs = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);
            convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_rs;

            sizes[0] = gcmALIGN(ifm, CONV2D_ALIGN_SIZE4);
            sizes[0] = enableAlign16 ? gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16) : sizes[0];
            ifm_rs   = sizes[0];
            sizes[1] = ofm;
            dims     = 2;
            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = dims;
            tensor_create_params.sizes = sizes;
            tensor_create_params.data_format = TENSOR_DATA_TYPE(weights);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
            }

            weights_new = vxoTensor_CreateTensor(ops_layer->node->base.context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
            convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new;
            vxoTensor_AllocateMemory(weights_new);
            if (weights_new == VX_NULL)
            {
                vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto OnError;
            }

            TENSOR_DATA_LIFETIME(weights_new) = VX_TENSOR_LIFE_TIME_STATIC;

            vxnneTensorConstPad(weights_rs, weights_new, 0, ifm_rs - ifm, 0, 0, TENSOR_TF_ZEROPOINT(weights));

            if (enable_packed_weights)
            {
                vx_tensor t = NULL;

                t = vxoNNTensor_ReorgWeights(weights_new, ops_layer->node->graph, ifm_rs, ofm);

                if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE)
                {
                    weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                    convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                }
                else
                    weights_new_rs = t;

                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
            }
            else
            {
                sizes[0] = ifm_rs;
                sizes[1] = 1;
                sizes[2] = 1;
                sizes[3] = ofm;
                dims     = 4;
                 weights_new_rs = vxoTensor_ReshapeTensor(weights_new, (vx_int32*)sizes, dims);
                 convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
            }
        }
        else
        {
            if (enable_ofm_gt_xy)
            {
                vx_tensor t = NULL;

                sizes[0] = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                sizes[1] = 1;
                sizes[2] = 1;
                sizes[3] = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                dims     = 4;

                t = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);
                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;

                weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
            }

            if (enable_packed_weights && enable_conv2d_1x1)
            {
                vx_tensor t = NULL;
                vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                t = vxoNNTensor_ReorgWeights(weights, ops_layer->node->graph, ifm, ofm);
                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
            }
            else if (enable_packed_weights)
            {
                vx_tensor t = NULL;
                vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                t = vxoNNTensor_ReorgWeights(weights, ops_layer->node->graph, ifm, ofm);
                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE)
                {
                    weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                    convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                }
                else
                {
                    weights_new_rs = t;
                }
            }
            else if (!enable_ofm_gt_xy)
            {
                weights_new_rs = weights;
            }
        }

        if (biases)
        {
            if (enable_conv2d_1x1)
            {
                if (enable_tensor_cast || enable_ofm_gt_xy)
                {
                    vx_tensor t = NULL;

                    sizes[0] = outputWidth * outputHeight;
                    sizes[1] = 1;
                    sizes[2] = outputDepth;
                    sizes[3] = batchCount;

                    t = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, TENSOR_DIM_NUM(outputs));
                    convolutionLayer->base.temp_tensors[numTmpTensor++] = t;

                    outputs_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                    convolutionLayer->base.temp_tensors[numTmpTensor++] = outputs_rs;
                }
                else
                {
                    outputs_rs = outputs;
                }

                shaderExecutable = vxnneGPUConv2D_1x1ShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_CONVOLUTION_1X1,
                    &ops_layer->node->kernelAttributes.borderMode, enable_tensor_cast || enable_ofm_gt_xy, enable_packed_weights, input_rs, weights_new_rs, newBiases, outputs_rs);
            }
            else
            {
                outputs_rs = outputs;
                shaderExecutable = vxnneGPUGemmShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_GEMM,
                    &ops_layer->node->kernelAttributes.borderMode, enable_packed_weights, input_rs, weights_new_rs, newBiases, outputs_rs);
            }
        }
        else
        {
            outputs_rs = outputs;
            shaderExecutable = vxnneGPUGemm_noBiasShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_GEMM_NOBIAS, &ops_layer->node->kernelAttributes.borderMode, input_rs, weights_new_rs, outputs_rs);
        }
    }

    if (!shaderExecutable)
    {
        status = VX_FAILURE;
        return status;
    }

    status = vxnneShaderOperation_Initialize(&convolutionLayer->convolutionGemm_sh_operation,
        &convolutionLayer->base,
        VXNNE_OPERATOR_CONVOLUTION,
        batchCount,
        shaderExecutable);

    if (batchCount > 1)
    {
        vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
        vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
    }

    vxnneLayer_SetOperation(
        &convolutionLayer->base,
        &convolutionLayer->convolutionGemm_sh_operation.base,
        idx++);

    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)weights_new_rs, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)newBiases, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)outputs_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    convolutionLayer->base.temp_tensors[numTmpTensor++] = tensor2Row;

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayer_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNDilationConvolutionLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

OnError:
    return status;
}
VX_PRIVATE_API vx_bool vxoNNDilationConvolutionLayer_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && vxoNNDilationConvolutionLayer_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayer_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxoNNDilationConvolutionLayer_SH_EVIS_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNDilationConvolutionLayer_NN_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_NN, VX_TYPE_INVALID, VX_NULL);

    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[2];
    vx_scalar dilationX             = (vx_scalar)parameters[10];
    vx_scalar dilationY             = (vx_scalar)parameters[11];
    vx_tensor outputs               = (vx_tensor)parameters[19];

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_bool   nnSupportFormat    = vxnneIsNNSupportFormat(node->base.context, inputs, VX_NULL, outputs);
    vx_bool   tpSupportFormat    = vxnneIsTPSupportFormat(node->base.context, inputs, VX_NULL, outputs);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && nnSupportFormat;

    if (!support)return support;

    support =  (tpSupportFormat &&
        vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        ((dilation_x > 1) || (dilation_y > 1)));

    if (!support)return support;

    if (weights != VX_NULL && TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_DYNAMIC)
        support = vx_false_e;

    if (support)
        reg_param->flag = gcoNNE_CONV_MODE_NNE_TP2;

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayer_NN_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_convolution_layer  convolutionLayer = (vxnne_convolution_layer)ops_layer;

    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases= (vx_weights_biases_parameter)parameters[1];
    vx_tensor weights               = (vx_tensor)parameters[2];
    vx_tensor biases                = (vx_tensor)parameters[3];
    vx_scalar padXLeft              = (vx_scalar)parameters[4];
    vx_scalar padXRight             = (vx_scalar)parameters[5];
    vx_scalar padYTop               = (vx_scalar)parameters[6];
    vx_scalar padYBottom            = (vx_scalar)parameters[7];
    vx_scalar padModes              = (vx_scalar)parameters[8];
    vx_scalar padConstant           = (vx_scalar)parameters[9];
    vx_scalar dilationX             = (vx_scalar)parameters[10];
    vx_scalar dilationY             = (vx_scalar)parameters[11];

    vx_scalar relu_s                = (vx_scalar)parameters[14];
    vx_scalar pooling_s             = (vx_scalar)parameters[15];
    vx_scalar poolingX              = (vx_scalar)parameters[16];
    vx_scalar poolingY              = (vx_scalar)parameters[17];

    vx_tensor outputs               = (vx_tensor)parameters[19];

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    gcoNNConv_Mode mode = gcoNNE_CONV_MODE_NNE_TP2;

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_bool relu = relu_s?relu_s->value->b:vx_false_e;

    vx_enum pooling = pooling_s?pooling_s->value->e:(VX_NN_POOLING_MAX-1);
    vx_int32 poolingx = poolingX?poolingX->value->n32:0;
    vx_int32 poolingy = poolingY?poolingY->value->n32:0;

    vx_context context = vxGetContext((vx_reference)inputs);

    vx_uint32 idx = 0;
    vx_uint32 numTmpTensor = 0;
    vx_bool enable_batch = ((gcoNNE_CONV_MODE_SW2 == mode) || (gcoNNE_CONV_MODE_NNE_TP == mode))?vx_false_e:vx_true_e;
    vx_bool need_reshuffle = ((gcoNNE_CONV_MODE_SW2 == mode) || (gcoNNE_CONV_MODE_NNE_TP == mode))?vx_true_e:vx_false_e;

    vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));

    vx_int32 item_size = vxnneGetTypeSize(output_format);

    vx_int32 padding_x = padXLeft->value->n32;
    vx_int32 padding_y = padYTop->value->n32;

    vx_int32 in_h = TENSOR_SIZE_INDEX(inputs, 1);
    vx_int32 in_w = TENSOR_SIZE_INDEX(inputs, 0);
    vx_int32 in_c = TENSOR_SIZE_INDEX(inputs, 2);

    vx_int32 k_w = weights?TENSOR_SIZE_INDEX(weights, 0):WB_ORG_KERNEL_X(weights_biases);
    vx_int32 k_h = weights?TENSOR_SIZE_INDEX(weights, 1):WB_ORG_KERNEL_Y(weights_biases);
    vx_int32 out_c = TENSOR_SIZE_INDEX(outputs, 2);

    vx_int32 pad_x = padding_x/dilation_x, pad_y = padding_y/dilation_y;

    vx_int32 inputs_reshuffle_width = gcmALIGN_NP2(in_w, dilation_x)/dilation_x, inputs_reshuffle_height = gcmALIGN_NP2(in_h, dilation_y)/dilation_y;
    vx_int32 reshuffled_out_w = (inputs_reshuffle_width + 2 * padding_x/dilation_x - k_w) + 1;
    vx_int32 reshuffled_out_h = (inputs_reshuffle_height + 2 * padding_y/dilation_y - k_h) + 1;

    vx_scalar reshuffled_pad_x = vxCreateScalar(context, VX_TYPE_INT32, &pad_x);
    vx_scalar reshuffled_pad_y = vxCreateScalar(context, VX_TYPE_INT32, &pad_y);

    vx_uint32 size[][4] = {
         /*
          * {4, 4, 512, 6 * 6},reshuffled input
          * {4, 4, 1024, 6 * 6}, reshuffled output
          * {3, 3, 512 * 6 * 6, 1024 * 6 * 6}, reshuffled weights
          * {1, 1, 1, 512 * 6 * 6}, reshuffled biases
          */

         {inputs_reshuffle_width, inputs_reshuffle_height, in_c, dilation_x * dilation_y},
         {reshuffled_out_w, reshuffled_out_h, out_c, dilation_x * dilation_y},
         {k_w, k_h, in_c * dilation_x * dilation_y, dilation_x * dilation_y * out_c},
         {1, 1, 1, out_c * dilation_x * dilation_y},
     };

    vx_int32 up_sample_tp = 1;
    vx_tensor reshuffled_output = VX_NULL;
    vx_tensor reshuffled_input = VX_NULL;
    vx_tensor reshuffled_weights = VX_NULL;
    vx_op_param_s conv;
    vx_tensor_create_params_t tensor_create_params;
    vx_uint8_ptr reshuffled_input_base = VX_NULL;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    gcmASSERT((dilation_x > 1) || (dilation_y > 1));
    memset(&conv, 0, sizeof(vx_op_param_s));

    if ((mode == gcoNNE_CONV_MODE_SW2) || (mode == gcoNNE_CONV_MODE_NNE_TP))
        gcmASSERT((weights_biases == VX_NULL) && (weights != VX_NULL));

    if (!enable_batch)
    {
        size[gcoNNE_CONV_RESHUFFLED_INPUTS][2] *= size[gcoNNE_CONV_RESHUFFLED_INPUTS][3];
        size[gcoNNE_CONV_RESHUFFLED_INPUTS][3] = 1;

        size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][2] *= size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][3];
        size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][3] = 1;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 4;
    tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_INPUTS];
    tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
    }
    reshuffled_input = vxoTensor_CreateTensor(context, ops_layer->node->graph, &tensor_create_params, vx_false_e);

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = 4;
    tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_OUTPUTS];
    tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(reshuffled_input);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
    }
    reshuffled_output = vxoTensor_CreateTensor(context, ops_layer->node->graph, &tensor_create_params, vx_false_e);

    vxoTensor_AllocateMemory(reshuffled_input);
    vxoTensor_AllocateMemory(reshuffled_output);

    QUANT8CHECK(inputs, reshuffled_input);
    QUANT8CHECK(outputs, reshuffled_output);

    if (need_reshuffle)
    {
        vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = 4;
        tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_WEIGHTS];
        tensor_create_params.data_format = weight_format;
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
        }
        reshuffled_weights = vxoTensor_CreateTensor(context, ops_layer->node->graph, &tensor_create_params, vx_false_e);
        status = vxoTensor_AllocateMemory(reshuffled_weights);
        if (status)
        {
            vxError("Reshuffled Weights Out of Memeory!");
            goto OnError;
        }

        QUANT8CHECK(weights, reshuffled_weights);
    }

    vxoTensor_GetTensorViewMemory(reshuffled_output, (gctPOINTER*)&reshuffled_input_base, VX_NULL);
    memset(reshuffled_input_base, 0, size[1][0] * size[1][1] * size[1][2] * size[1][3] * item_size);

    if (need_reshuffle)
    {
        vx_type_e bias_format   = (vx_type_e)(TENSOR_DATA_TYPE(biases));

        vx_tensor reshuffled_biases;

        vx_weights_biases_parameter_optimizations_t opt = {0};
        opt.zrl = -1;

        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
        tensor_create_params.num_of_dims = 4;
        tensor_create_params.sizes = size[3];
        tensor_create_params.data_format = bias_format;
        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(biases);
        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(biases);
        }
        else
        {
            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(biases);
            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(biases);
        }
        reshuffled_biases = vxoTensor_CreateTensor(context, ops_layer->node->graph, &tensor_create_params, vx_false_e);

        if (TENSOR_DATA_TYPE(reshuffled_input) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffled_input) == VX_QUANT_AFFINE_SCALE)
        {
            opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(reshuffled_input);
            opt.outputFormat = TENSOR_DATA_TYPE(reshuffled_output);
        }

        vxoTensor_AllocateMemory(reshuffled_biases);

        QUANT8CHECK(biases, reshuffled_biases);

        convolutionLayer->convolution_sw1_reshuffle_operation.create_wbp                = vx_true_e;
        convolutionLayer->convolution_sw1_reshuffle_operation.weights                   = (need_reshuffle)?weights:VX_NULL;
        convolutionLayer->convolution_sw1_reshuffle_operation.stride_x                  = dilationX;/*for dilation x*/
        convolutionLayer->convolution_sw1_reshuffle_operation.stride_y                  = dilationY;/*for dilation y*/
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_left            = padXLeft;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_right           = padXRight;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_top             = padYTop;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_bottom          = padYBottom;
        convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_inputs         = reshuffled_input;
        convolutionLayer->convolution_sw1_reshuffle_operation.outputs                   = reshuffled_output;
        convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_weights        = reshuffled_weights;
        convolutionLayer->convolution_sw1_reshuffle_operation.bias                      = biases;
        convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_biases         = reshuffled_biases;
        convolutionLayer->convolution_sw1_reshuffle_operation.opt                       = &opt;

        vxnneExecuteSWConv_Reshuffle((struct _vxnne_operation_s *)&convolutionLayer->convolution_sw1_reshuffle_operation);
    }

    if ((mode == gcoNNE_CONV_MODE_NNE_TP) || (gcoNNE_CONV_MODE_NNE_TP2 == mode) || !(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP)))
    {
        status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_reshuffle_operation.base,
                                           &convolutionLayer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_DILATION_RESHUFFLE,
                                           VX_NULL,
                                           vxnneExecuteSWConv_Reshuffle_DeInilition,
                                           batchCount,
                                           0);

        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &convolutionLayer->convolution_tp_reshuffle_operation.base,
            idx++);

        convolutionLayer->convolution_tp_reshuffle_operation.input            = inputs;
        convolutionLayer->convolution_tp_reshuffle_operation.output           = reshuffled_input;

        vxnneOperation_AddReference(&convolutionLayer->convolution_tp_reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_tp_reshuffle_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pad_mode = padModes->value->e;
        conv.pad_const = padConstant ? padConstant->value->n32 : 0;
        conv.tpType = TP_DILATE_RESHUFFLE;
        conv.other_ref = (vx_reference)dilationX;
        conv.data_buff = gcvNULL;

        memcpy(&convolutionLayer->convolution_tp_reshuffle_operation.base.parameter, &conv, sizeof(vx_op_param_s));
    }
    else
    {
        /* Initialize reshuffle weights operation */
        vxnneOperation_Initialize(&convolutionLayer->convolution_sw1_reshuffle_operation.base,
                                  &convolutionLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_DILATION_RESHUFFLE,
                                  vxnneExecuteSWConv_Reshuffle,
                                  VX_NULL,
                                  batchCount,
                                  0);
        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &convolutionLayer->convolution_sw1_reshuffle_operation.base,
            idx++);

        convolutionLayer->convolution_sw1_reshuffle_operation.inputs            = inputs;
        convolutionLayer->convolution_sw1_reshuffle_operation.weights           = need_reshuffle?weights:VX_NULL;
        convolutionLayer->convolution_sw1_reshuffle_operation.stride_x          = dilationX;/*for dilation x*/
        convolutionLayer->convolution_sw1_reshuffle_operation.stride_y          = dilationY;/*for dilation y*/
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_left    = padXLeft;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_top     = padYTop;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_right   = padXRight;
        convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_bottom  = padYBottom;
        convolutionLayer->convolution_sw1_reshuffle_operation.outputs           = reshuffled_output;
        convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_inputs = reshuffled_input;
        convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_weights= reshuffled_weights;
        convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes     = weights_biases;

        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

        if (need_reshuffle)
        {
            vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_weights, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    if (gcoNNE_CONV_MODE_NNE_TP2 == mode)
    {
        vx_weights_biases_parameter_optimizations_t opt = {0};
        opt.zrl = -1;

        if (TENSOR_DATA_TYPE(reshuffled_input) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffled_input) == VX_QUANT_AFFINE_SCALE)
        {
            opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(reshuffled_input);
            opt.outputFormat = TENSOR_DATA_TYPE(reshuffled_output);
        }

        {
            if (weights_biases == VX_NULL)
                convolutionLayer->convolution_sw1_reshuffle_operation2.create_wbp            = vx_true_e;
            else
                convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes         = weights_biases;

            convolutionLayer->convolution_sw1_reshuffle_operation2.inputs                    = VX_NULL;
            convolutionLayer->convolution_sw1_reshuffle_operation2.weights                   = weights;
            convolutionLayer->convolution_sw1_reshuffle_operation2.stride_x                  = dilationX;/*for dilation x*/
            convolutionLayer->convolution_sw1_reshuffle_operation2.stride_y                  = dilationY;/*for dilation y*/
            convolutionLayer->convolution_sw1_reshuffle_operation2.padding_x_left            = padXLeft;
            convolutionLayer->convolution_sw1_reshuffle_operation2.padding_x_right           = padXRight;
            convolutionLayer->convolution_sw1_reshuffle_operation2.padding_y_top             = padYTop;
            convolutionLayer->convolution_sw1_reshuffle_operation2.padding_y_bottom          = padYBottom;
            convolutionLayer->convolution_sw1_reshuffle_operation2.reshuffled_inputs         = reshuffled_input;
            convolutionLayer->convolution_sw1_reshuffle_operation2.outputs                   = reshuffled_output;
            convolutionLayer->convolution_sw1_reshuffle_operation2.bias                      = biases;
            convolutionLayer->convolution_sw1_reshuffle_operation2.opt                       = &opt;

            vxnneExecuteSWConv_Reshuffle((struct _vxnne_operation_s *)&convolutionLayer->convolution_sw1_reshuffle_operation2);
        }

        if (convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes)
        {
            /* force enable batch for internal convolution, disabled while sub convolution finished */
            vx_uint32 loop = dilation_x * dilation_y;
            vx_uint32 i;
            vx_tensor *real_inputs = VX_NULL;
            vx_tensor *real_outputs = VX_NULL;
            vx_tensor_view input_view = VX_NULL;
            vx_tensor_view output_view = VX_NULL;
            vx_uint32 input_start[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_uint32 input_end[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_uint32 output_start[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_uint32 output_end[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

            gcoOS_Allocate(gcvNULL, sizeof(vx_tensor) * loop, (gctPOINTER*)&real_inputs);
            gcoOS_Allocate(gcvNULL, sizeof(vx_tensor) * loop, (gctPOINTER*)&real_outputs);
            gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_relu_pooling_operation_s) * (dilation_x * dilation_y), (gctPOINTER*)&convolutionLayer->convolution_nn_convolution_dynamic_operation);
            gcoOS_ZeroMemory(convolutionLayer->convolution_nn_convolution_dynamic_operation, sizeof(vxnne_convolution_relu_pooling_operation_s) * (dilation_x * dilation_y));


            for (i = 0; i < loop; i++)
            {
                /* Initialize covolution operation */
                status = vxnneOperation_Initialize(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base,
                                                   &convolutionLayer->base,
                                                   VXNNE_OPERATION_TARGET_NN,
                                                   VXNNE_OPERATOR_CONVOLUTION,
                                                   VX_NULL,
                                                   NULL,
                                                   batchCount,
                                                   NNE_COMMAND_SIZE);

                input_end[0] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 0);
                input_end[1] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 1);
                input_end[2] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 2);
                input_end[3] = i + 1;

                input_start[0] = 0;
                input_start[1] = 0;
                input_start[2] = 0;
                input_start[3] = i;

                input_view = vxCreateTensorView(context, input_start, input_end, (vx_uint8)reshuffled_input->dimCount);
                real_inputs[i] = vxoTensor_CreateTensorFromView(reshuffled_input, input_view);

                output_end[0] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 0);
                output_end[1] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 1);
                output_end[2] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 2);
                output_end[3] = i + 1;

                output_start[0] = 0;
                output_start[1] = 0;
                output_start[2] = 0;
                output_start[3] = i;

                output_view = vxCreateTensorView(context, output_start, output_end, (vx_uint8)reshuffled_output->dimCount);
                real_outputs[i] = vxoTensor_CreateTensorFromView(reshuffled_output, output_view);

                convolutionLayer->base.temp_tensors[numTmpTensor++] = real_inputs[i];
                convolutionLayer->base.temp_tensors[numTmpTensor++] = real_outputs[i];

                vxReleaseTensorView(&input_view);
                vxReleaseTensorView(&output_view);

                convolutionLayer->convolution_nn_convolution_dynamic_operation[i].inputs           = real_inputs[i];
                convolutionLayer->convolution_nn_convolution_dynamic_operation[i].orig_inputs      = inputs;
                convolutionLayer->convolution_nn_convolution_dynamic_operation[i].weights_biases   = convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes;
                convolutionLayer->convolution_nn_convolution_dynamic_operation[i].outputs          = real_outputs[i];

                vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base, (vx_reference)real_inputs[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base, (vx_reference)real_outputs[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

                {
                    conv.pad_x_left = pad_x;
                    conv.pad_x_right = pad_x;
                    conv.pad_y_top = pad_y;
                    conv.pad_y_bottom = pad_y;
                    conv.pad_mode = padModes->value->e;
                    conv.pad_const = padConstant ? padConstant->value->n32 : 0;
                    conv.pool_type = pooling;
                    conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                    conv.enable_relu = relu;
                    conv.pool_size_x = poolingx;
                    conv.pool_size_y = poolingy;
                    conv.pool_stride = (poolingx > 0) ? 2 : 1;
                    memcpy(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base.parameter, &conv, sizeof(vx_op_param_s));
                }

                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base,
                    idx++);
            }
        }
    }
    else if (gcoNNE_CONV_MODE_NNE_TP == mode)
    {
        if (convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes)
        {
            /* Initialize covolution operation */
            status = vxnneOperation_Initialize(&convolutionLayer->convolution_nn_convolution_operation.base,
                                               &convolutionLayer->base,
                                               VXNNE_OPERATION_TARGET_NN,
                                               VXNNE_OPERATOR_CONVOLUTION,
                                               VX_NULL,
                                               NULL,
                                               batchCount,
                                               NNE_COMMAND_SIZE);

            vxnneLayer_SetOperation(
                &convolutionLayer->base,
                &convolutionLayer->convolution_nn_convolution_operation.base,
                idx++);

            convolutionLayer->convolution_nn_convolution_operation.inputs           = reshuffled_input;
            convolutionLayer->convolution_nn_convolution_operation.orig_inputs      = inputs;
            convolutionLayer->convolution_nn_convolution_operation.weights_biases   = convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes;
            convolutionLayer->convolution_nn_convolution_operation.outputs          = reshuffled_output;

            vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            {
                conv.pad_x_left = pad_x;
                conv.pad_x_right = pad_x;
                conv.pad_y_top = pad_y;
                conv.pad_y_bottom = pad_y;
                conv.pad_mode = padModes->value->e;
                conv.pad_const = padConstant->value->n32;
                conv.pool_type = pooling;
                conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                conv.enable_relu = relu;
                conv.pool_size_x = poolingx;
                conv.pool_size_y = poolingy;
                conv.pool_stride = (poolingx > 0) ? 2 : 1;
                memcpy(&convolutionLayer->convolution_nn_convolution_operation.base.parameter, &conv, sizeof(vx_op_param_s));
            }
        }
    }
    else
    {
        vx_enum downScaleSizeRounding = VX_NN_DS_SIZE_ROUNDING_FLOOR;
        vx_scalar down_scale_s = vxCreateScalar(context, VX_TYPE_ENUM, &downScaleSizeRounding);

        vxnne_convolution_operation op = (vxnne_convolution_operation)vxAllocateAndZeroMemory(sizeof(vxnne_convolution_operation_s));

        /* Initialize covolution operation */
        vxnneOperation_Initialize(&op->base,
                                  &convolutionLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_CONVOLUTION,
                                  vxnneExecuteSWConvolution,
                                  (gcoNNE_CONV_MODE_SW2 == mode)?vxnneExecuteSWConv_Convolution_DeInilition2:vxnneExecuteSWConv_Convolution_DeInilition,
                                  batchCount,
                                  0);

        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &op->base,
            idx++);

        op->inputs                  = reshuffled_input;
        op->weights                 = (gcoNNE_CONV_MODE_SW2 == mode)?reshuffled_weights:weights;
        op->biases                  = (gcoNNE_CONV_MODE_SW2 == mode)?convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_biases:biases;
        op->padX                    = reshuffled_pad_x;
        op->padY                    = reshuffled_pad_y;

        op->downScaleSizeRounding   = down_scale_s;
        op->outputs                 = reshuffled_output;

        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

    }

    if ((mode == gcoNNE_CONV_MODE_NNE_TP) || (mode == gcoNNE_CONV_MODE_NNE_TP2) || !(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP)))
    {
        vx_bool need_correct = ((dilation_x * inputs_reshuffle_width != in_w) || (dilation_y * inputs_reshuffle_height != in_h))?vx_true_e:vx_false_e;
        vx_tensor reshuffled_output2 = VX_NULL;

        if (need_correct)
        {
            vx_uint32 size2[][4] = {
                 /*
                  * {4*6, 4*6, 1024, 1}, reshuffled output2
                  */

                 {reshuffled_out_w * dilation_x, reshuffled_out_h * dilation_x, out_c, 1},
             };

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 4;
            tensor_create_params.sizes = size2[0];
            tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(outputs);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
            }

            reshuffled_output2 = vxoTensor_CreateTensor(context, ops_layer->node->graph, &tensor_create_params, vx_false_e);

            vxoTensor_AllocateMemory(reshuffled_output2);

            QUANT8CHECK(outputs, reshuffled_output2);
        }


        status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_upsample_operation.base,
                                       &convolutionLayer->base,
                                       VXNNE_OPERATION_TARGET_TP,
                                       VXNNE_OPERATOR_DILATION_UPSAMPLE,
                                       VX_NULL,
                                       vxnneExecuteSWConv_UpSample_DeInilition,
                                       batchCount,
                                       0);

        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &convolutionLayer->convolution_tp_upsample_operation.base,
            idx++);

        convolutionLayer->convolution_tp_upsample_operation.input            = reshuffled_output;
        convolutionLayer->convolution_tp_upsample_operation.output           = need_correct?reshuffled_output2:outputs;

        vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation.base, (vx_reference)(need_correct?reshuffled_output2:outputs), VXNNE_OPERATION_REFENRENCE_OUTPUT);

        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.pad_mode = padModes->value->e;
        conv.pad_const = padConstant ? padConstant->value->n32 : 0;
        conv.tpType = TP_DILATE_UPSAMPLE;
        conv.other_ref = gcvNULL;
        conv.data_buff = gcvNULL;

        memcpy(&convolutionLayer->convolution_tp_upsample_operation.base.parameter, &conv, sizeof(vx_op_param_s));




        if (need_correct)
        {

            up_sample_tp ++;
            status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_upsample_operation2.base,
                                               &convolutionLayer->base,
                                               VXNNE_OPERATION_TARGET_TP,
                                               VXNNE_OPERATOR_DILATION_UPSAMPLE2,
                                               VX_NULL,
                                               vxnneExecuteSWConv_UpSample_DeInilition,
                                               batchCount,
                                               0);

            vxnneLayer_SetOperation(
                &convolutionLayer->base,
                &convolutionLayer->convolution_tp_upsample_operation2.base,
                idx++);

            convolutionLayer->convolution_tp_upsample_operation2.input            = reshuffled_output2;
            convolutionLayer->convolution_tp_upsample_operation2.output           = outputs;

            vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation2.base, (vx_reference)reshuffled_output2, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation2.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            conv.tpType = TP_DILATE_UPSAMPLE2;
            conv.other_ref = gcvNULL;
            conv.data_buff = gcvNULL;

            memcpy(&convolutionLayer->convolution_tp_upsample_operation2.base.parameter, &conv, sizeof(vx_op_param_s));

        }
    }
    else
    {

        /* Initialize upsample operation */
        vxnneOperation_Initialize(&convolutionLayer->convolution_sw1_upsample_operation.base,
                                  &convolutionLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_CONVOLUTION,
                                  vxnneExecuteSWConv_UpSample,
                                  vxnneExecuteSWConv_UpSample_DeInilition,
                                  batchCount,
                                  0);

        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &convolutionLayer->convolution_sw1_upsample_operation.base,
            idx++);

        convolutionLayer->convolution_sw1_upsample_operation.inputs           = reshuffled_output;
        convolutionLayer->convolution_sw1_upsample_operation.dilationX        = dilationX;
        convolutionLayer->convolution_sw1_upsample_operation.dilationY        = dilationY;

        convolutionLayer->convolution_sw1_upsample_operation.outputs          = outputs;

        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_upsample_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_upsample_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }

OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vx_scalar dilationX             = (vx_scalar)ops_layer->node->paramTable[9];
    vx_scalar dilationY             = (vx_scalar)ops_layer->node->paramTable[10];
    vxnne_convolution_layer  convolutionLayer = (vxnne_convolution_layer)ops_layer;

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_operation_s) * (dilation_x * dilation_y + vxmOPERATION_COUNT(convolutionLayer)), (gctPOINTER*)&convolutionLayer->dynamic_operations);
    if (!convolutionLayer->dynamic_operations)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }
    gcoOS_ZeroMemory(convolutionLayer->dynamic_operations, sizeof(vxnne_operation_s) * (dilation_x * dilation_y + vxmOPERATION_COUNT(convolutionLayer)));

    *max_num_operations = dilation_x * dilation_y + vxmOPERATION_COUNT(convolutionLayer);

    *operations = convolutionLayer->dynamic_operations;

    return status;
}

VX_PRIVATE_API vx_status vxoNNDilationConvolutionLayerInitializer_Ext(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vxnne_layer_imp_s registerTensorDilationConvLayers[] = {/* Please DON'T adjust the order, it's importent */
        { "RPNLAYER NN/TP", vxoNNDilationConvolutionLayer_NN_TP_Support, vxoNNDilationConvolutionLayer_NN_TP_Initialize, VX_NULL },
        { "RPNLAYER TP", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "RPNLAYER SH EVIS", vxoNNDilationConvolutionLayer_SH_EVIS_Support, vxoNNDilationConvolutionLayer_SH_EVIS_Initialize, VX_NULL },
        { "RPNLAYER SH F32", vxoNNDilationConvolutionLayer_SH_Support, vxoNNDilationConvolutionLayer_SH_Initialize, VX_NULL },
        { "RPNLAYER SW", vxoNNCommon_Support, vxoNNDilationConvolutionLayer_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorDilationConvLayers, vxnne_convolution_layer_s, "ConvolutionLayer", vxoNNLayer_GetOperations);

OnError:
    return status;
}
#endif

#if REGISTER_FRAME
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDilationConvolutionLayerInitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    return vxoNNDilationConvolutionLayerInitializer_Ext(node, parameters, num);
}

#else

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNDilationConvolutionLayerInitializer(vx_node node,
        vx_tensor inputs,
        vx_weights_biases_parameter weights_biases,
        vx_tensor weights,
        vx_tensor biases,
        vx_scalar padXLeft,
        vx_scalar padXRight,
        vx_scalar padYTop,
        vx_scalar padYBottom,
        vx_enum   padMode,
        vx_scalar padConstant,
        vx_scalar dilationX,
        vx_scalar dilationY,
        vx_scalar stridesX,
        vx_scalar stridesY,
        vx_scalar relu_s,
        vx_scalar pooling_s,
        vx_scalar poolingX,
        vx_scalar poolingY,
        vx_scalar downScaleSizeRounding,
        vx_tensor outputs)
{
    vx_status status = VX_SUCCESS;

    vxnne_convolution_layer convolutionLayer = VX_NULL;

    vx_uint32 batchCount = TENSOR_SIZE_INDEX(inputs, 3);

    gcoNNConv_Mode mode = gcoNNE_CONV_MODE_NNE_TP2;

    vx_int32 dilation_x = dilationX->value->n32 + 1;
    vx_int32 dilation_y = dilationY->value->n32 + 1;

    vx_bool relu = relu_s?relu_s->value->b:vx_false_e;

    vx_enum pooling = pooling_s?pooling_s->value->e:(VX_NN_POOLING_MAX-1);
    vx_int32 poolingx = poolingX?poolingX->value->n32:0;
    vx_int32 poolingy = poolingY?poolingY->value->n32:0;
    vx_uint32  width             = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32  height            = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_enum    inputFormat       = TENSOR_DATA_TYPE(inputs);
    vx_enum    outputFormat      = TENSOR_DATA_TYPE(outputs);
    vx_bool    enable_shader     = vx_false_e;
    vx_bool    has_pool          = (pooling_s == NULL && poolingx == 0) ? vx_false_e : vx_true_e;
    vx_bool    enable_2dTensor   = vx_false_e;

    vx_context context = vxGetContext((vx_reference)inputs);
    vx_bool   nnSupportFormat    = vx_false_e;
    vx_bool   tpSupportFormat    = vx_false_e;
    vx_uint32 idx = 0;
    vx_uint32 numTmpTensor = 0;

    if (weights != NULL)
    {
        vx_uint32 kernel_x            = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        vx_uint32 kernel_y            = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        vx_uint32 ifm                 = TENSOR_VIEW_SIZE_INDEX(weights, 2);
        vx_enum   weightFormat        = TENSOR_DATA_TYPE(weights);
        vx_enum   biasFormat          = biases ? TENSOR_DATA_TYPE(biases) : VX_TYPE_FLOAT64;
        vx_uint32 size                = width * height;

        if(node->base.context->evisNoInst.supportEVIS)
        {
            vx_uint32 convsize        = kernel_x * kernel_y * ifm;
            vx_bool   support_type    = vx_false_e;

            if (biases)
            {
                support_type    = (vx_bool)(
                    (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_INT8  && weightFormat == VX_TYPE_INT8  && biasFormat == VX_TYPE_INT32 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT64 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_INT32 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_UINT8 && outputFormat != VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16));
            }
            else
            {
                support_type    = (vx_bool)(
                    (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_INT8  && weightFormat == VX_TYPE_INT8  && outputFormat == VX_TYPE_INT8)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
            }

            enable_shader = (vx_bool)(convsize < IMG_MAX_WIDTH && support_type && has_pool == vx_false_e);
            enable_2dTensor = (vx_bool)(size < IMG_MAX_WIDTH && dilation_x == 1 && dilation_y == 1);
        }
        else
        {
            vx_bool   support_type    = vx_false_e;

            if (biases)
            {
                support_type    = (vx_bool)
                    ((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_INT16  && weightFormat == VX_TYPE_INT16  && biasFormat == VX_TYPE_INT64 && outputFormat == VX_TYPE_INT16)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && biasFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_UINT8)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && biasFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_BFLOAT16));
            }
            else
            {
                support_type    = (vx_bool)
                    ((inputFormat == VX_TYPE_FLOAT16 && weightFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                    || (inputFormat == VX_TYPE_FLOAT32 && weightFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                    || (inputFormat == VX_TYPE_UINT8 && weightFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                    || (inputFormat == VX_TYPE_BFLOAT16 && weightFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16));
            }

            /*enable_2dTensor = (vx_bool)(size < IMG_MAX_WIDTH && dilation_x == 1 && dilation_y == 1);*/
            enable_shader = (vx_bool)(support_type && has_pool == vx_false_e);
        }
    }

    nnSupportFormat = vxnneIsNNSupportFormat(context, inputs, VX_NULL, outputs);
    tpSupportFormat = vxnneIsTPSupportFormat(context, inputs, VX_NULL, outputs);

    if (tpSupportFormat &&
        nnSupportFormat &&
        vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_TP) &&
        ((dilation_x > 1) || (dilation_y > 1)))
    {
        mode = gcoNNE_CONV_MODE_NNE_TP2;
    }
    else if (enable_shader && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
    {
        mode = gcoNNE_CONV_MODE_SH;
    }
    else if (weights_biases == VX_NULL)
    {
        vxWarning("NN/TP/SH not support this format, goto cpu path. function %s line %d\n", __FUNCTION__, __LINE__);
        mode = gcoNNE_CONV_MODE_SW;
    }
    else
    {
        vxError("Not support this format. function %s line %d\n", __FUNCTION__, __LINE__);
        status = VX_ERROR_NOT_SUPPORTED;
        goto exit;
    }

    /* if weights is dynamic, nn/tp not support*/
    if (weights != VX_NULL && TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_DYNAMIC)
    {
        if (enable_shader && vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER))
            mode = gcoNNE_CONV_MODE_SH;
        else
            mode = gcoNNE_CONV_MODE_SW;
    }

#if DILATION_SELECT_ENV
    {
        gctSTRING MODE = VX_NULL;

        gcoOS_GetEnv(gcvNULL, "DILATION_MODE", &MODE);
        if (MODE != NULL)
        {
            if (gcoOS_StrStr(MODE, "VIP2", VX_NULL))
                mode = gcoNNE_CONV_MODE_NNE_TP2;
            else if (gcoOS_StrStr(MODE, "SW1", VX_NULL))
                mode = gcoNNE_CONV_MODE_SW1;
            else if (gcoOS_StrStr(MODE, "SW2", VX_NULL))
                mode = gcoNNE_CONV_MODE_SW2;
            else if (gcoOS_StrStr(MODE, "VIP", VX_NULL))
                mode = gcoNNE_CONV_MODE_NNE_TP;
            else if (gcoOS_StrStr(MODE, "SW", VX_NULL))
                mode = gcoNNE_CONV_MODE_SW;
            else
                vxError("Not support mode[%s]!", MODE);
        }
    }
#endif

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_layer_s), (gctPOINTER*)&convolutionLayer);
    if (!convolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(convolutionLayer, sizeof(vxnne_convolution_layer_s));


    gcoOS_Allocate(gcvNULL, sizeof(vxnne_operation_s) * (dilation_x * dilation_y + vxmOPERATION_COUNT(convolutionLayer)), (gctPOINTER*)&convolutionLayer->dynamic_operations);
    gcoOS_ZeroMemory(convolutionLayer->dynamic_operations, sizeof(vxnne_operation_s) * (dilation_x * dilation_y + vxmOPERATION_COUNT(convolutionLayer)));
    vxnneLayer_Initialize(&convolutionLayer->base,
                            "ConvolutionLayer",
                            node,
                            vxmOPERATION_COUNT(convolutionLayer) + dilation_x * dilation_y,
                            convolutionLayer->dynamic_operations,
                            vxoNNDilationConvolutionLayer_Deinitialize);

    switch(mode)
    {
    case gcoNNE_CONV_MODE_SW:

        gcmASSERT(relu == vx_false_e);
        gcmASSERT((pooling != VX_NN_POOLING_MAX) && (pooling != VX_NN_POOLING_AVG));

        vxnneOperation_Initialize(&convolutionLayer->convolutionSW.base,
                                &convolutionLayer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_CONVOLUTION,
                                vxnneExecuteSWConvolution,
                                VX_NULL,
                                batchCount,
                                0);

        vxnneLayer_SetOperation(
            &convolutionLayer->base,
            &convolutionLayer->convolutionSW.base,
            idx++);

        convolutionLayer->convolutionSW.inputs                = inputs;
        convolutionLayer->convolutionSW.weights               = weights;
        convolutionLayer->convolutionSW.biases                = biases;
        convolutionLayer->convolutionSW.padX                  = padXLeft;
        convolutionLayer->convolutionSW.padXRight             = padXRight;
        convolutionLayer->convolutionSW.padY                  = padYTop;
        convolutionLayer->convolutionSW.padYBottom            = padYBottom;
        convolutionLayer->convolutionSW.dilationX             = dilationX;
        convolutionLayer->convolutionSW.dilationY             = dilationY;
        convolutionLayer->convolutionSW.downScaleSizeRounding = downScaleSizeRounding;
        convolutionLayer->convolutionSW.outputs               = outputs;

        vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&convolutionLayer->convolutionSW.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        break;

    case gcoNNE_CONV_MODE_SW1:
    case gcoNNE_CONV_MODE_SW2:
    case gcoNNE_CONV_MODE_NNE_TP:
    case gcoNNE_CONV_MODE_NNE_TP2:
        {
            vx_bool enable_batch = ((gcoNNE_CONV_MODE_SW2 == mode) || (gcoNNE_CONV_MODE_NNE_TP == mode))?vx_false_e:vx_true_e;
            vx_bool need_reshuffle = ((gcoNNE_CONV_MODE_SW2 == mode) || (gcoNNE_CONV_MODE_NNE_TP == mode))?vx_true_e:vx_false_e;

            vx_type_e output_format = (vx_type_e)(TENSOR_DATA_TYPE(outputs));

            vx_int32 item_size = vxnneGetTypeSize(output_format);

            vx_int32 padding_x = padXLeft->value->n32;
            vx_int32 padding_y = padYTop->value->n32;

            vx_int32 in_h = TENSOR_SIZE_INDEX(inputs, 1);
            vx_int32 in_w = TENSOR_SIZE_INDEX(inputs, 0);
            vx_int32 in_c = TENSOR_SIZE_INDEX(inputs, 2);

            vx_int32 k_w = weights?TENSOR_SIZE_INDEX(weights, 0):WB_ORG_KERNEL_X(weights_biases);
            vx_int32 k_h = weights?TENSOR_SIZE_INDEX(weights, 1):WB_ORG_KERNEL_Y(weights_biases);
            vx_int32 out_c = TENSOR_SIZE_INDEX(outputs, 2);

            vx_int32 pad_x = padding_x/dilation_x, pad_y = padding_y/dilation_y;

            vx_int32 inputs_reshuffle_width = gcmALIGN_NP2(in_w, dilation_x)/dilation_x, inputs_reshuffle_height = gcmALIGN_NP2(in_h, dilation_y)/dilation_y;
            vx_int32 reshuffled_out_w = (inputs_reshuffle_width + 2 * padding_x/dilation_x - k_w) + 1;
            vx_int32 reshuffled_out_h = (inputs_reshuffle_height + 2 * padding_y/dilation_y - k_h) + 1;

            vx_context context = vxGetContext((vx_reference)inputs);
            vx_scalar reshuffled_pad_x = vxCreateScalar(context, VX_TYPE_INT32, &pad_x);
            vx_scalar reshuffled_pad_y = vxCreateScalar(context, VX_TYPE_INT32, &pad_y);

            vx_uint32 size[][4] = {
                 /*
                  * {4, 4, 512, 6 * 6},reshuffled input
                  * {4, 4, 1024, 6 * 6}, reshuffled output
                  * {3, 3, 512 * 6 * 6, 1024 * 6 * 6}, reshuffled weights
                  * {1, 1, 1, 512 * 6 * 6}, reshuffled biases
                  */

                 {inputs_reshuffle_width, inputs_reshuffle_height, in_c, dilation_x * dilation_y},
                 {reshuffled_out_w, reshuffled_out_h, out_c, dilation_x * dilation_y},
                 {k_w, k_h, in_c * dilation_x * dilation_y, dilation_x * dilation_y * out_c},
                 {1, 1, 1, out_c * dilation_x * dilation_y},
             };

            vx_int32 up_sample_tp = 1;
            vx_tensor reshuffled_output = VX_NULL;
            vx_tensor reshuffled_input = VX_NULL;
            vx_tensor reshuffled_weights = VX_NULL;
            vx_op_param_s conv;
            vx_tensor_create_params_t tensor_create_params;
            vx_uint8_ptr reshuffled_input_base = VX_NULL;

            gcmASSERT((dilation_x > 1) || (dilation_y > 1));
            memset(&conv, 0, sizeof(vx_op_param_s));

            if ((mode == gcoNNE_CONV_MODE_SW2) || (mode == gcoNNE_CONV_MODE_NNE_TP))
                gcmASSERT((weights_biases == VX_NULL) && (weights != VX_NULL));

            if (!enable_batch)
            {
                size[gcoNNE_CONV_RESHUFFLED_INPUTS][2] *= size[gcoNNE_CONV_RESHUFFLED_INPUTS][3];
                size[gcoNNE_CONV_RESHUFFLED_INPUTS][3] = 1;

                size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][2] *= size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][3];
                size[gcoNNE_CONV_RESHUFFLED_OUTPUTS][3] = 1;
            }

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 4;
            tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_INPUTS];
            tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
            }
            reshuffled_input = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

            gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
            tensor_create_params.num_of_dims = 4;
            tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_OUTPUTS];
            tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
            tensor_create_params.quant_format = TENSOR_QUANT_TYPE(reshuffled_input);
            if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
            {
                tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
            }
            else
            {
                tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
            }
            reshuffled_output = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

            vxoTensor_AllocateMemory(reshuffled_input);
            vxoTensor_AllocateMemory(reshuffled_output);

            QUANT8CHECK(inputs, reshuffled_input);
            QUANT8CHECK(outputs, reshuffled_output);

            if (need_reshuffle)
            {
                vx_type_e weight_format = (vx_type_e)(TENSOR_DATA_TYPE(weights));

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = 4;
                tensor_create_params.sizes = size[gcoNNE_CONV_RESHUFFLED_WEIGHTS];
                tensor_create_params.data_format = weight_format;
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
                }
                reshuffled_weights = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);
                status = vxoTensor_AllocateMemory(reshuffled_weights);
                if (status)
                {
                    vxError("Reshuffled Weights Out of Memeory!");
                    goto exit;
                }

                QUANT8CHECK(weights, reshuffled_weights);
            }

            vxoTensor_GetTensorViewMemory(reshuffled_output, (gctPOINTER*)&reshuffled_input_base, VX_NULL);
            memset(reshuffled_input_base, 0, size[1][0] * size[1][1] * size[1][2] * size[1][3] * item_size);

            if (need_reshuffle)
            {
                vx_type_e bias_format   = (vx_type_e)(TENSOR_DATA_TYPE(biases));

                vx_tensor reshuffled_biases;

                vx_weights_biases_parameter_optimizations_t opt = {0};
                opt.zrl = -1;

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = 4;
                tensor_create_params.sizes = size[3];
                tensor_create_params.data_format = bias_format;
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(biases);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(biases);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(biases);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(biases);
                }
                reshuffled_biases = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

                if (TENSOR_DATA_TYPE(reshuffled_input) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffled_input) == VX_QUANT_AFFINE_SCALE)
                {
                    opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(reshuffled_input);
                    opt.outputFormat = TENSOR_DATA_TYPE(reshuffled_output);
                }

                vxoTensor_AllocateMemory(reshuffled_biases);

                QUANT8CHECK(biases, reshuffled_biases);

                convolutionLayer->convolution_sw1_reshuffle_operation.create_wbp                = vx_true_e;
                convolutionLayer->convolution_sw1_reshuffle_operation.weights                   = (need_reshuffle)?weights:VX_NULL;
                convolutionLayer->convolution_sw1_reshuffle_operation.stride_x                  = dilationX;/*for dilation x*/
                convolutionLayer->convolution_sw1_reshuffle_operation.stride_y                  = dilationY;/*for dilation y*/
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_left            = padXLeft;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_right           = padXRight;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_top             = padYTop;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_bottom          = padYBottom;
                convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_inputs         = reshuffled_input;
                convolutionLayer->convolution_sw1_reshuffle_operation.outputs                   = reshuffled_output;
                convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_weights        = reshuffled_weights;
                convolutionLayer->convolution_sw1_reshuffle_operation.bias                      = biases;
                convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_biases         = reshuffled_biases;
                convolutionLayer->convolution_sw1_reshuffle_operation.opt                       = &opt;

                vxnneExecuteSWConv_Reshuffle((struct _vxnne_operation_s *)&convolutionLayer->convolution_sw1_reshuffle_operation);
            }

            if ((mode == gcoNNE_CONV_MODE_NNE_TP) || (gcoNNE_CONV_MODE_NNE_TP2 == mode) || !(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP)))
            {
                status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_reshuffle_operation.base,
                                                   &convolutionLayer->base,
                                                   VXNNE_OPERATION_TARGET_TP,
                                                   VXNNE_OPERATOR_DILATION_RESHUFFLE,
                                                   VX_NULL,
                                                   vxnneExecuteSWConv_Reshuffle_DeInilition,
                                                   batchCount,
                                                   0);
                if (status != VX_SUCCESS) goto exit;

                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &convolutionLayer->convolution_tp_reshuffle_operation.base,
                    idx++);

                convolutionLayer->convolution_tp_reshuffle_operation.input            = inputs;
                convolutionLayer->convolution_tp_reshuffle_operation.output           = reshuffled_input;

                vxnneOperation_AddReference(&convolutionLayer->convolution_tp_reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_tp_reshuffle_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                conv.pad_x_left = 0;
                conv.pad_y_top = 0;
                conv.pool_size_x = 0;
                conv.pool_size_y = 0;
                conv.pool_stride = 1;
                conv.enable_relu = vx_false_e;
                conv.pad_mode = padMode;
                conv.pad_const = padConstant ? padConstant->value->n32 : 0;
                conv.tpType = TP_DILATE_RESHUFFLE;
                conv.other_ref = (vx_reference)dilationX;
                conv.data_buff = gcvNULL;

                memcpy(&convolutionLayer->convolution_tp_reshuffle_operation.base.parameter, &conv, sizeof(vx_op_param_s));
            }
            else
            {
                /* Initialize reshuffle weights operation */
                vxnneOperation_Initialize(&convolutionLayer->convolution_sw1_reshuffle_operation.base,
                                          &convolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_DILATION_RESHUFFLE,
                                          vxnneExecuteSWConv_Reshuffle,
                                          VX_NULL,
                                          batchCount,
                                          0);
                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &convolutionLayer->convolution_sw1_reshuffle_operation.base,
                    idx++);

                convolutionLayer->convolution_sw1_reshuffle_operation.inputs            = inputs;
                convolutionLayer->convolution_sw1_reshuffle_operation.weights           = need_reshuffle?weights:VX_NULL;
                convolutionLayer->convolution_sw1_reshuffle_operation.stride_x          = dilationX;/*for dilation x*/
                convolutionLayer->convolution_sw1_reshuffle_operation.stride_y          = dilationY;/*for dilation y*/
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_left    = padXLeft;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_top     = padYTop;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_x_right   = padXRight;
                convolutionLayer->convolution_sw1_reshuffle_operation.padding_y_bottom  = padYBottom;
                convolutionLayer->convolution_sw1_reshuffle_operation.outputs           = reshuffled_output;
                convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_inputs = reshuffled_input;
                convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_weights= reshuffled_weights;
                convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes     = weights_biases;

                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                if (need_reshuffle)
                {
                    vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_weights, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                }
            }

            if (gcoNNE_CONV_MODE_NNE_TP2 == mode)
            {
                vx_weights_biases_parameter_optimizations_t opt = {0};
                opt.zrl = -1;

                if (TENSOR_DATA_TYPE(reshuffled_input) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(reshuffled_input) == VX_QUANT_AFFINE_SCALE)
                {
                    opt.inputZeroPoint = TENSOR_TF_ZEROPOINT(reshuffled_input);
                    opt.outputFormat = TENSOR_DATA_TYPE(reshuffled_output);
                }

                {
                    if (weights_biases == VX_NULL)
                        convolutionLayer->convolution_sw1_reshuffle_operation2.create_wbp            = vx_true_e;
                    else
                        convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes         = weights_biases;

                    convolutionLayer->convolution_sw1_reshuffle_operation2.inputs                    = VX_NULL;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.weights                   = weights;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.stride_x                  = dilationX;/*for dilation x*/
                    convolutionLayer->convolution_sw1_reshuffle_operation2.stride_y                  = dilationY;/*for dilation y*/
                    convolutionLayer->convolution_sw1_reshuffle_operation2.padding_x_left            = padXLeft;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.padding_x_right           = padXRight;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.padding_y_top             = padYTop;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.padding_y_bottom          = padYBottom;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.reshuffled_inputs         = reshuffled_input;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.outputs                   = reshuffled_output;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.bias                      = biases;
                    convolutionLayer->convolution_sw1_reshuffle_operation2.opt                       = &opt;

                    vxnneExecuteSWConv_Reshuffle((struct _vxnne_operation_s *)&convolutionLayer->convolution_sw1_reshuffle_operation2);
                }

                if (convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes)
                {
                    /* force enable batch for internal convolution, disabled while sub convolution finished */
                    vx_uint32 loop = dilation_x * dilation_y;
                    vx_uint32 i;
                    vx_tensor *real_inputs = VX_NULL;
                    vx_tensor *real_outputs = VX_NULL;
                    vx_tensor_view input_view = VX_NULL;
                    vx_tensor_view output_view = VX_NULL;
                    vx_uint32 input_start[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
                    vx_uint32 input_end[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
                    vx_uint32 output_start[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
                    vx_uint32 output_end[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};

                    gcoOS_Allocate(gcvNULL, sizeof(vx_tensor) * loop, (gctPOINTER*)&real_inputs);
                    gcoOS_Allocate(gcvNULL, sizeof(vx_tensor) * loop, (gctPOINTER*)&real_outputs);
                    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_relu_pooling_operation_s) * (dilation_x * dilation_y), (gctPOINTER*)&convolutionLayer->convolution_nn_convolution_dynamic_operation);
                    gcoOS_ZeroMemory(convolutionLayer->convolution_nn_convolution_dynamic_operation, sizeof(vxnne_convolution_relu_pooling_operation_s) * (dilation_x * dilation_y));

                    for (i = 0; i < loop; i++)
                    {
                        /* Initialize covolution operation */
                        status = vxnneOperation_Initialize(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base,
                                                           &convolutionLayer->base,
                                                           VXNNE_OPERATION_TARGET_NN,
                                                           VXNNE_OPERATOR_CONVOLUTION,
                                                           VX_NULL,
                                                           NULL,
                                                           batchCount,
                                                           NNE_COMMAND_SIZE);
                        if (status != VX_SUCCESS) goto exit;

                        input_end[0] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 0);
                        input_end[1] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 1);
                        input_end[2] = TENSOR_VIEW_SIZE_INDEX(reshuffled_input, 2);
                        input_end[3] = i + 1;

                        input_start[0] = 0;
                        input_start[1] = 0;
                        input_start[2] = 0;
                        input_start[3] = i;

                        input_view = vxCreateTensorView(context, input_start, input_end, (vx_uint8)reshuffled_input->dimCount);
                        real_inputs[i] = vxoTensor_CreateTensorFromView(reshuffled_input, input_view);

                        output_end[0] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 0);
                        output_end[1] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 1);
                        output_end[2] = TENSOR_VIEW_SIZE_INDEX(reshuffled_output, 2);
                        output_end[3] = i + 1;

                        output_start[0] = 0;
                        output_start[1] = 0;
                        output_start[2] = 0;
                        output_start[3] = i;

                        output_view = vxCreateTensorView(context, output_start, output_end, (vx_uint8)reshuffled_output->dimCount);
                        real_outputs[i] = vxoTensor_CreateTensorFromView(reshuffled_output, output_view);

                        convolutionLayer->base.temp_tensors[numTmpTensor++] = real_inputs[i];
                        convolutionLayer->base.temp_tensors[numTmpTensor++] = real_outputs[i];

                        vxReleaseTensorView(&input_view);
                        vxReleaseTensorView(&output_view);

                        convolutionLayer->convolution_nn_convolution_dynamic_operation[i].inputs           = real_inputs[i];
                        convolutionLayer->convolution_nn_convolution_dynamic_operation[i].orig_inputs      = inputs;
                        convolutionLayer->convolution_nn_convolution_dynamic_operation[i].weights_biases   = convolutionLayer->convolution_sw1_reshuffle_operation2.weights_biaes;
                        convolutionLayer->convolution_nn_convolution_dynamic_operation[i].outputs          = real_outputs[i];

                        vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base, (vx_reference)real_inputs[i], VXNNE_OPERATION_REFENRENCE_INPUT);
                        vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base, (vx_reference)real_outputs[i], VXNNE_OPERATION_REFENRENCE_OUTPUT);

                        {
                            conv.pad_x_left = pad_x;
                            conv.pad_x_right = pad_x;
                            conv.pad_y_top = pad_y;
                            conv.pad_y_bottom = pad_y;
                            conv.pad_mode = padMode;
                            conv.pad_const = padConstant ? padConstant->value->n32 : 0;
                            conv.pool_type = pooling;
                            conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                            conv.enable_relu = relu;
                            conv.pool_size_x = poolingx;
                            conv.pool_size_y = poolingy;
                            conv.pool_stride = (poolingx > 0) ? 2 : 1;
                            memcpy(&convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base.parameter, &conv, sizeof(vx_op_param_s));
                        }

                        vxnneLayer_SetOperation(
                            &convolutionLayer->base,
                            &convolutionLayer->convolution_nn_convolution_dynamic_operation[i].base,
                            idx++);
                    }
                }
            }
            else if (gcoNNE_CONV_MODE_NNE_TP == mode)
            {
                if (convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes)
                {
                    /* Initialize covolution operation */
                    status = vxnneOperation_Initialize(&convolutionLayer->convolution_nn_convolution_operation.base,
                                                       &convolutionLayer->base,
                                                       VXNNE_OPERATION_TARGET_NN,
                                                       VXNNE_OPERATOR_CONVOLUTION,
                                                       VX_NULL,
                                                       NULL,
                                                       batchCount,
                                                       NNE_COMMAND_SIZE);
                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &convolutionLayer->base,
                        &convolutionLayer->convolution_nn_convolution_operation.base,
                        idx++);

                    convolutionLayer->convolution_nn_convolution_operation.inputs           = reshuffled_input;
                    convolutionLayer->convolution_nn_convolution_operation.orig_inputs      = inputs;
                    convolutionLayer->convolution_nn_convolution_operation.weights_biases   = convolutionLayer->convolution_sw1_reshuffle_operation.weights_biaes;
                    convolutionLayer->convolution_nn_convolution_operation.outputs          = reshuffled_output;

                    vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&convolutionLayer->convolution_nn_convolution_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    {
                        conv.pad_x_left = pad_x;
                        conv.pad_x_right = pad_x;
                        conv.pad_y_top = pad_y;
                        conv.pad_y_bottom = pad_y;
                        conv.pad_mode = padMode;
                        conv.pad_const = padConstant->value->n32;
                        conv.pool_type = pooling;
                        conv.conv_rounding_type = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                        conv.enable_relu = relu;
                        conv.pool_size_x = poolingx;
                        conv.pool_size_y = poolingy;
                        conv.pool_stride = (poolingx > 0) ? 2 : 1;
                        memcpy(&convolutionLayer->convolution_nn_convolution_operation.base.parameter, &conv, sizeof(vx_op_param_s));
                    }
                }
            }
            else
            {
                vx_enum downScaleSizeRounding = VX_NN_DS_SIZE_ROUNDING_FLOOR;
                vx_scalar down_scale_s = vxCreateScalar(context, VX_TYPE_ENUM, &downScaleSizeRounding);

                vxnne_convolution_operation op = (vxnne_convolution_operation)vxAllocateAndZeroMemory(sizeof(vxnne_convolution_operation_s));

                /* Initialize covolution operation */
                vxnneOperation_Initialize(&op->base,
                                          &convolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_CONVOLUTION,
                                          vxnneExecuteSWConvolution,
                                          (gcoNNE_CONV_MODE_SW2 == mode)?vxnneExecuteSWConv_Convolution_DeInilition2:vxnneExecuteSWConv_Convolution_DeInilition,
                                          batchCount,
                                          0);

                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &op->base,
                    idx++);

                op->inputs                  = reshuffled_input;
                op->weights                 = (gcoNNE_CONV_MODE_SW2 == mode)?reshuffled_weights:weights;
                op->biases                  = (gcoNNE_CONV_MODE_SW2 == mode)?convolutionLayer->convolution_sw1_reshuffle_operation.reshuffled_biases:biases;
                op->padX                    = reshuffled_pad_x;
                op->padY                    = reshuffled_pad_y;

                op->downScaleSizeRounding   = down_scale_s;
                op->outputs                 = reshuffled_output;

                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_input, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)weights, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)biases, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_reshuffle_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            }

            if ((mode == gcoNNE_CONV_MODE_NNE_TP) || (mode == gcoNNE_CONV_MODE_NNE_TP2) || !(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP)))
            {
                vx_bool need_correct = ((dilation_x * inputs_reshuffle_width != in_w) || (dilation_y * inputs_reshuffle_height != in_h))?vx_true_e:vx_false_e;
                vx_tensor reshuffled_output2 = VX_NULL;

                if (need_correct)
                {
                    vx_uint32 size2[][4] = {
                         /*
                          * {4*6, 4*6, 1024, 1}, reshuffled output2
                          */

                         {reshuffled_out_w * dilation_x, reshuffled_out_h * dilation_x, out_c, 1},
                     };

                    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                    tensor_create_params.num_of_dims = 4;
                    tensor_create_params.sizes = size2[0];
                    tensor_create_params.data_format = TENSOR_DATA_TYPE(outputs);
                    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(outputs);
                    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                    {
                        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(outputs);
                    }
                    else
                    {
                        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(outputs);
                        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(outputs);
                    }

                    reshuffled_output2 = vxoTensor_CreateTensor(context, node->graph, &tensor_create_params, vx_false_e);

                    vxoTensor_AllocateMemory(reshuffled_output2);

                    QUANT8CHECK(outputs, reshuffled_output2);
                }

                {
                    status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_upsample_operation.base,
                                                   &convolutionLayer->base,
                                                   VXNNE_OPERATION_TARGET_TP,
                                                   VXNNE_OPERATOR_DILATION_UPSAMPLE,
                                                   VX_NULL,
                                                   vxnneExecuteSWConv_UpSample_DeInilition,
                                                   batchCount,
                                                   0);

                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &convolutionLayer->base,
                        &convolutionLayer->convolution_tp_upsample_operation.base,
                        idx++);

                    convolutionLayer->convolution_tp_upsample_operation.input            = reshuffled_output;
                    convolutionLayer->convolution_tp_upsample_operation.output           = need_correct?reshuffled_output2:outputs;

                    vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation.base, (vx_reference)(need_correct?reshuffled_output2:outputs), VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    conv.pad_x_left = 0;
                    conv.pad_y_top = 0;
                    conv.pool_size_x = 0;
                    conv.pool_size_y = 0;
                    conv.pool_stride = 1;
                    conv.enable_relu = vx_false_e;
                    conv.pad_mode = padMode;
                    conv.pad_const = padConstant ? padConstant->value->n32 : 0;
                    conv.tpType = TP_DILATE_UPSAMPLE;
                    conv.other_ref = gcvNULL;
                    conv.data_buff = gcvNULL;

                    memcpy(&convolutionLayer->convolution_tp_upsample_operation.base.parameter, &conv, sizeof(vx_op_param_s));

                }


                if (need_correct)
                {

                    up_sample_tp ++;
                    status = vxnneOperation_Initialize(&convolutionLayer->convolution_tp_upsample_operation2.base,
                                                       &convolutionLayer->base,
                                                       VXNNE_OPERATION_TARGET_TP,
                                                       VXNNE_OPERATOR_DILATION_UPSAMPLE2,
                                                       VX_NULL,
                                                       vxnneExecuteSWConv_UpSample_DeInilition,
                                                       batchCount,
                                                       0);
                    if (status != VX_SUCCESS) goto exit;

                    vxnneLayer_SetOperation(
                        &convolutionLayer->base,
                        &convolutionLayer->convolution_tp_upsample_operation2.base,
                        idx++);

                    convolutionLayer->convolution_tp_upsample_operation2.input            = reshuffled_output2;
                    convolutionLayer->convolution_tp_upsample_operation2.output           = outputs;

                    vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation2.base, (vx_reference)reshuffled_output2, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&convolutionLayer->convolution_tp_upsample_operation2.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                    conv.tpType = TP_DILATE_UPSAMPLE2;
                    conv.other_ref = gcvNULL;
                    conv.data_buff = gcvNULL;

                    memcpy(&convolutionLayer->convolution_tp_upsample_operation2.base.parameter, &conv, sizeof(vx_op_param_s));

                }
            }
            else
            {

                /* Initialize upsample operation */
                vxnneOperation_Initialize(&convolutionLayer->convolution_sw1_upsample_operation.base,
                                          &convolutionLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_CONVOLUTION,
                                          vxnneExecuteSWConv_UpSample,
                                          vxnneExecuteSWConv_UpSample_DeInilition,
                                          batchCount,
                                          0);

                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &convolutionLayer->convolution_sw1_upsample_operation.base,
                    idx++);

                convolutionLayer->convolution_sw1_upsample_operation.inputs           = reshuffled_output;
                convolutionLayer->convolution_sw1_upsample_operation.dilationX        = dilationX;
                convolutionLayer->convolution_sw1_upsample_operation.dilationY        = dilationY;

                convolutionLayer->convolution_sw1_upsample_operation.outputs          = outputs;

                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_upsample_operation.base, (vx_reference)reshuffled_output, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolution_sw1_upsample_operation.base, (vx_reference)outputs, VXNNE_OPERATION_REFENRENCE_OUTPUT);
            }

        }
        break;
   case gcoNNE_CONV_MODE_SH:
            {
#define CONV2D_ALIGN_SIZE4 (4)
#define CONV2D_ALIGN_SIZE16 (16)
                vx_tensor tensor2Row    = NULL;
                vx_uint32 sizes[]       = {1, 1, 1, 1};
                vx_uint32 dims          = TENSOR_DIM_NUM(inputs);
                vx_enum   downScaleSizeRoundingValue = downScaleSizeRounding->value->e;
                vx_int32  paddingLeft   = padXLeft->value->n32;
                vx_int32  paddingRight  = padXRight->value->n32;
                vx_int32  paddingTop    = padYTop->value->n32;
                vx_int32  paddingBottom = padYBottom->value->n32;
                vx_int32  k_w           = weights ? TENSOR_VIEW_SIZE_INDEX(weights, 0):WB_ORG_KERNEL_X(weights_biases);
                vx_int32  k_h           = weights ? TENSOR_VIEW_SIZE_INDEX(weights, 1):WB_ORG_KERNEL_Y(weights_biases);
                vx_int32  inputWidth    = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
                vx_int32  inputHeight   = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
                vx_int32  inputDepth    = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
                vx_int32  outputWidth   = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
                vx_int32  outputHeight  = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
                vx_int32  outputDepth   = TENSOR_VIEW_SIZE_INDEX(outputs, 2);
                vx_uint32 input_size    = k_w * k_h * inputDepth;
                /* Calculate stride     = (w + padXLeft + padXRight - weight)/(output_w - 1) */
                vx_int32  strideX       = (stridesX != VX_NULL) ? (stridesX->value->n32) : (vxoNNExternsionConvlutionRound((vx_float32)(inputWidth + paddingLeft + paddingRight - k_w) / (outputWidth - 1), downScaleSizeRoundingValue));
                vx_int32  strideY       = (stridesY != VX_NULL) ? (stridesY->value->n32) : (vxoNNExternsionConvlutionRound((vx_float32)(inputHeight + paddingTop + paddingBottom - k_h) / (outputHeight - 1), downScaleSizeRoundingValue));
                vx_bool   enableAlign4  = vx_false_e;
                vx_bool   enableAlign16  = vx_false_e;
                vxnne_shader_executable shaderExecutable = VX_NULL;
                vx_tensor_create_params_t tensor_create_params;
                vx_tensor    newBiases                      = NULL;
                vx_tensor    weights_new_rs                 = NULL;
                vx_tensor    input_rs                       = NULL;
                vx_tensor    outputs_rs                     = NULL;
                vx_bool      is_static_weights_biases       = vx_false_e;
                vx_bool      enable_adjust_biases           = vx_false_e;
                vx_bool      enable_conv2d_1x1              = vx_false_e;
                vx_bool      is_conv_3x3_s2                 = vx_false_e;
                // step 1, tensor to row
                if (enable_2dTensor)
                {
                    sizes[0] = k_w * k_h * inputDepth;
                    sizes[1] = outputWidth * outputHeight;
                    sizes[2] = 1;
                    sizes[3] = batchCount;
                    dims = gcmMAX(2, TENSOR_DIM_NUM(outputs));
                }
                else
                {
                    sizes[0] = k_w * k_h * inputDepth;
                    sizes[1] = outputWidth;
                    sizes[2] = outputHeight;
                    sizes[3] = batchCount;
                    dims = 4;
                }

                if (node->base.context->evisNoInst.supportEVIS == vx_false_e
                    && ((inputWidth * inputHeight < IMG_MAX_WIDTH) && inputDepth < IMG_MAX_WIDTH)
                    && (k_w == k_h && k_w == 1)
                    && (strideX == strideY && strideX == 1)
                    && (paddingLeft == paddingRight && paddingLeft == 0)
                    && (paddingTop == paddingBottom && paddingTop == 0)
                    && biases != NULL
                    && (CHECK_LIFETIME_IS_STATIC(weights) || TENSOR_QUANT_TYPE(inputs) != VX_QUANT_AFFINE_SCALE)
                    /*&& ((inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 == 0) || (input_size % CONV2D_ALIGN_SIZE16 != 0) || TENSOR_QUANT_TYPE(inputs) != VX_QUANT_AFFINE_SCALE)*/
                    )
                {
                    enable_conv2d_1x1 = vx_true_e;
                }

                if (k_w == 3 && k_h == 3 && paddingLeft == 0 && strideX == 2 && dilation_x == 1 && dilation_y == 1)
                {
                    is_conv_3x3_s2 = vx_true_e;
                }

                if (node->base.context->evisNoInst.supportEVIS == vx_false_e
                    && (sizes[0] % CONV2D_ALIGN_SIZE4) != 0 && CHECK_LIFETIME_IS_STATIC(weights))
                {
                    if (is_conv_3x3_s2)
                    {
                        sizes[0] = gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16);
                        enableAlign16 = vx_true_e;
                    }
                    else
                    {
                        sizes[0] = gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE4);
                    }
                    enableAlign4 = vx_true_e;
                    input_size = sizes[0];
                }

                gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                tensor_create_params.num_of_dims = dims;
                tensor_create_params.sizes = sizes;
                tensor_create_params.data_format = TENSOR_DATA_TYPE(inputs);
                tensor_create_params.quant_format = TENSOR_QUANT_TYPE(inputs);
                if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                {
                    tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(inputs);
                }
                else
                {
                    tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(inputs);
                    tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(inputs);
                }

                if (enable_conv2d_1x1 == vx_false_e)
                {
                    if (enableAlign4)
                    {
                        tensor2Row = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                        if (tensor2Row == VX_NULL || vxoTensor_AllocateMemory(tensor2Row) != VX_SUCCESS)
                        {
                            vxError("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }
                    }
                    else
                    {
                        tensor2Row = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_true_e);
                        if (tensor2Row == VX_NULL)
                        {
                            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }
                    }
                }

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneTensor2RowShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR2ROW, &node->kernelAttributes.borderMode,
                                       inputs, k_w, k_h, dilation_x, dilation_y, strideX, strideY, paddingLeft, paddingTop, outputWidth, outputHeight, tensor2Row);
                }
                else
                {
                    if (enable_conv2d_1x1 == vx_false_e)
                    {
                        shaderExecutable = vxnneGPUTensor2RowShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR2ROW, &node->kernelAttributes.borderMode,
                                       inputs, k_w, k_h, dilation_x, dilation_y, strideX, strideY, paddingLeft, paddingTop, outputWidth, outputHeight, tensor2Row);
                    }
                }

                if (enable_conv2d_1x1 == vx_false_e)
                {
                    if (!shaderExecutable)
                    {
                        status = VX_FAILURE;
                        return status;
                    }

                    status = vxnneShaderOperation_Initialize(&convolutionLayer->convolutionTensor2Row_sh_operation,
                        &convolutionLayer->base,
                        VXNNE_OPERATOR_CONVOLUTION,
                        batchCount,
                        shaderExecutable);

                    if (status != VX_SUCCESS)
                        return status;

                    vxnneLayer_SetOperation(
                            &convolutionLayer->base,
                            &convolutionLayer->convolutionTensor2Row_sh_operation.base,
                            idx++);

                    vxnneOperation_AddReference(&convolutionLayer->convolutionTensor2Row_sh_operation.base, (vx_reference)inputs, VXNNE_OPERATION_REFENRENCE_INPUT);
                    vxnneOperation_AddReference(&convolutionLayer->convolutionTensor2Row_sh_operation.base, (vx_reference)tensor2Row, VXNNE_OPERATION_REFENRENCE_OUTPUT);
                }

                // step 2, gemm
                {
                    is_static_weights_biases = (vx_bool)(CHECK_LIFETIME_IS_STATIC(weights) && CHECK_LIFETIME_IS_STATIC(biases));
                    enable_adjust_biases     = is_static_weights_biases && TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE && TENSOR_QUANT_TYPE(biases);
                    enable_adjust_biases     = enable_adjust_biases && (TENSOR_DATA_TYPE(biases) != VX_TYPE_UINT8);

                    if (enable_adjust_biases)
                    {
                        vx_tensor_create_params_t params;
                        gctPOINTER weightsLogical   = VX_NULL;
                        gctPOINTER biasesLogical    = VX_NULL;
                        gctPOINTER newBiasesLogical = VX_NULL;
                        vx_uint32  i                = 0;
                        vx_uint32  j                = 0;
                        vx_uint32  sizes[4]         = {1};
                        vx_uint32  ofm              = TENSOR_VIEW_SIZE_INDEX(biases, 0);
                        vx_uint32  ifm              = k_w * k_h * inputDepth;

                        sizes[0] = TENSOR_VIEW_SIZE_INDEX(biases, 0);
                        sizes[1] = 1;

                        gcoOS_MemFill(&params, 0, sizeof(vx_tensor_create_params_t));
                        params.num_of_dims = TENSOR_DIM_NUM(biases);
                        params.sizes = sizes;
                        params.data_format = TENSOR_DATA_TYPE(biases);
                        params.quant_format = TENSOR_QUANT_TYPE(biases);
                        if (params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                        {
                            params.quant_data.dfp.fixed_point_pos = TENSOR_POS(biases);
                        }
                        else
                        {
                            params.quant_data.affine.scale = TENSOR_TF_SCALE(biases);
                        }

                        newBiases = vxoTensor_CreateTensor(context, NULL, &params, vx_false_e);
                        convolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
                        vxoTensor_AllocateMemory(newBiases);
                        if (newBiases == VX_NULL)
                        {
                            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }

                        TENSOR_DATA_LIFETIME(newBiases) = VX_TENSOR_LIFE_TIME_STATIC;

                        vxoTensor_GetTensorViewMemory(weights, &weightsLogical, VX_NULL);
                        vxoTensor_GetTensorViewMemory(biases, &biasesLogical, VX_NULL);
                        vxoTensor_GetTensorViewMemory(newBiases, &newBiasesLogical, VX_NULL);

                        for (j = 0; j < ofm; j++)
                        {
                            vx_int32 offset = 0;
                            vx_int32 dstVal = 0;

                            for (i = 0; i < ifm; i++)
                            {
                                vx_uint32 idx = j * ifm + i;
                                offset = offset - (((vx_uint8*)weightsLogical)[idx] - TENSOR_TF_ZEROPOINT(weights)) * TENSOR_TF_ZEROPOINT(inputs);
                            }

                            dstVal = ((vx_int32*)biasesLogical)[j] + offset;
                            ((vx_int32*)newBiasesLogical)[j] = dstVal;
                        }

                        convolutionLayer->base.temp_tensors[numTmpTensor++] = newBiases;
                    }
                    else
                    {
                        newBiases = biases;
                    }
                }

                if(node->base.context->evisNoInst.supportEVIS)
                {
                    weights_new_rs = weights;
                    input_rs = tensor2Row;
                    outputs_rs = outputs;
                    if (biases)
                    {
                        shaderExecutable = vxnneGemmShaderExecutable(node->base.context, VXNNE_KERNEL_GEMM, &node->kernelAttributes.borderMode, tensor2Row, weights_new_rs, newBiases, VX_NN_ACTIVATION_NONE, enable_2dTensor, outputs_rs);
                    }
                    else
                    {
                        shaderExecutable = vxnneGemm_noBiasShaderExecutable(node->base.context, VXNNE_KERNEL_GEMM_NOBIAS, &node->kernelAttributes.borderMode, tensor2Row, weights_new_rs, VX_NN_ACTIVATION_NONE, enable_2dTensor, outputs_rs);
                    }
                }
                else
                {
                    vx_bool   enable_tensor_cast  = vx_false_e;
                    vx_bool   enable_packed_weights = vx_false_e;
                    vx_bool   enable_ofm_gt_xy    = vx_false_e;
                    vx_tensor weights_rs          = NULL;
                    vx_tensor weights_new         = NULL;

                    sizes[0] = enableAlign4 ? gcmALIGN(k_w * k_h * inputDepth, CONV2D_ALIGN_SIZE4) : (k_w * k_h * inputDepth);
                    sizes[0] = enableAlign16 ? gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16) : sizes[0];
                    sizes[1] = outputWidth;
                    sizes[2] = outputHeight;
                    sizes[3] = batchCount;
                    dims = 4;

                    if (enable_conv2d_1x1
                     && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 == 0)
                     && TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8
                     && TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8
                     )
                    {
                        if (/*inputWidth * inputHeight < inputDepth
                          && */(inputWidth * inputHeight % CONV2D_ALIGN_SIZE16 != 0))
                        {
                            if ((outputDepth % CONV2D_ALIGN_SIZE16 == 0) && (input_size % CONV2D_ALIGN_SIZE4 == 0))
                            {
                                 enable_packed_weights = vx_true_e;
                            }
                            else
                            {
                                 enable_ofm_gt_xy = vx_true_e;
                            }
                        }
                        else
                        {
                            enable_tensor_cast = vx_true_e;

                            if ((outputDepth % CONV2D_ALIGN_SIZE16 == 0)
                                && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                                && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE16 == 0))
                            {
                                 enable_packed_weights = vx_true_e;
                            }
                        }
                    }
                    else if (enable_conv2d_1x1
                             /*&& (inputWidth * inputHeight < inputDepth)*/
                             && (inputWidth * inputHeight % CONV2D_ALIGN_SIZE4 != 0)
                             && (outputDepth % CONV2D_ALIGN_SIZE16 == 0)
                             && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                             && TENSOR_DATA_TYPE(inputs) == VX_TYPE_UINT8
                             && TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8
                             )
                    {
                        enable_packed_weights = vx_true_e;
                    }
                    else if (enable_conv2d_1x1
                             && (outputDepth % CONV2D_ALIGN_SIZE4 == 0)
                             && (input_size % CONV2D_ALIGN_SIZE4 == 0)
                             && TENSOR_DATA_TYPE(inputs) == VX_TYPE_FLOAT16
                             && TENSOR_DATA_TYPE(weights) == VX_TYPE_FLOAT16
                             )
                    {
                        enable_packed_weights = vx_true_e;
                    }
                    else if (!enable_conv2d_1x1 && biases != NULL
                        && ((outputWidth * outputHeight < IMG_MAX_WIDTH) && outputDepth < IMG_MAX_WIDTH && input_size < IMG_MAX_WIDTH)
                        && (outputDepth % CONV2D_ALIGN_SIZE4 == 0)
                        && CHECK_LIFETIME_IS_STATIC(weights)
                        && (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE || TENSOR_QUANT_TYPE(weights) == VX_QUANT_NONE))
                    {
                        enable_packed_weights = vx_true_e;
                    }

                    if (enable_conv2d_1x1)
                    {
                        sizes[0] = inputWidth * inputHeight;
                        sizes[1] = 1;
                        sizes[2] = inputDepth;
                        sizes[3] = batchCount;

                        if (enable_tensor_cast)
                        {
                            vx_tensor t = vxoTensor_ReshapeTensor(inputs, (vx_int32*)sizes, TENSOR_DIM_NUM(inputs));

                            input_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);

                            convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = input_rs;
                        }
                        else
                            input_rs = inputs;
                    }
                    else
                    {
                        input_rs = tensor2Row;
                    }

                    if (enableAlign4)
                    {
                        vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                        vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                        vx_uint32 ifm_rs = 0;

                        sizes[0] = ifm;
                        sizes[1] = ofm;
                        dims     = 2;
                        weights_rs = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);
                        convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_rs;

                        sizes[0] = gcmALIGN(ifm, CONV2D_ALIGN_SIZE4);
                        sizes[0] = enableAlign16 ? gcmALIGN(sizes[0], CONV2D_ALIGN_SIZE16) : sizes[0];
                        ifm_rs   = sizes[0];
                        sizes[1] = ofm;
                        dims     = 2;
                        gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
                        tensor_create_params.num_of_dims = dims;
                        tensor_create_params.sizes = sizes;
                        tensor_create_params.data_format = TENSOR_DATA_TYPE(weights);
                        tensor_create_params.quant_format = TENSOR_QUANT_TYPE(weights);
                        if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
                        {
                            tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(weights);
                        }
                        else
                        {
                            tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(weights);
                            tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weights);
                        }

                        weights_new = vxoTensor_CreateTensor(node->base.context, node->graph, &tensor_create_params, vx_false_e);
                        convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new;
                        vxoTensor_AllocateMemory(weights_new);
                        if (weights_new == VX_NULL)
                        {
                            vxError("vxoTensor_CreateTensor fail at function %s, line %d", __FUNCTION__, __LINE__);
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }

                        TENSOR_DATA_LIFETIME(weights_new) = VX_TENSOR_LIFE_TIME_STATIC;

                        vxnneTensorConstPad(weights_rs, weights_new, 0, ifm_rs - ifm, 0, 0, TENSOR_TF_ZEROPOINT(weights));

                        if (enable_packed_weights)
                        {
                            vx_tensor t = NULL;

                            t = vxoNNTensor_ReorgWeights(weights_new, node->graph, ifm_rs, ofm);

                            if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE)
                            {
                                weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                                convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                            }
                            else
                                weights_new_rs = t;

                            convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                        }
                        else
                        {
                            sizes[0] = ifm_rs;
                            sizes[1] = 1;
                            sizes[2] = 1;
                            sizes[3] = ofm;
                            dims     = 4;
                             weights_new_rs = vxoTensor_ReshapeTensor(weights_new, (vx_int32*)sizes, dims);
                             convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                        }
                    }
                    else
                    {
                        if (enable_ofm_gt_xy)
                        {
                            vx_tensor t = NULL;

                            sizes[0] = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                            sizes[1] = 1;
                            sizes[2] = 1;
                            sizes[3] = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                            dims     = 4;

                            t = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, dims);
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = t;

                            weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                        }

                        if (enable_packed_weights && enable_conv2d_1x1)
                        {
                            vx_tensor t = NULL;
                            vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                            vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                            t = vxoNNTensor_ReorgWeights(weights, node->graph, ifm, ofm);
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                            weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                        }
                        else if (enable_packed_weights)
                        {
                            vx_tensor t = NULL;
                            vx_uint32 ifm = k_w * k_h * TENSOR_VIEW_SIZE_INDEX(weights, 2);
                            vx_uint32 ofm = TENSOR_VIEW_SIZE_INDEX(weights, 3);
                            t = vxoNNTensor_ReorgWeights(weights, node->graph, ifm, ofm);
                            convolutionLayer->base.temp_tensors[numTmpTensor++] = t;
                            if (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE)
                            {
                                weights_new_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                                convolutionLayer->base.temp_tensors[numTmpTensor++] = weights_new_rs;
                            }
                            else
                            {
                                weights_new_rs = t;
                            }
                        }
                        else if (!enable_ofm_gt_xy)
                        {
                            weights_new_rs = weights;
                        }
                    }

                    if (biases)
                    {
                        if (enable_conv2d_1x1)
                        {
                            if (enable_tensor_cast || enable_ofm_gt_xy)
                            {
                                vx_tensor t = NULL;

                                sizes[0] = outputWidth * outputHeight;
                                sizes[1] = 1;
                                sizes[2] = outputDepth;
                                sizes[3] = batchCount;

                                t = vxoTensor_ReshapeTensor(outputs, (vx_int32*)sizes, TENSOR_DIM_NUM(outputs));
                                convolutionLayer->base.temp_tensors[numTmpTensor++] = t;

                                outputs_rs = vxoTensor_ReformatTensor(t, VX_TYPE_UINT32);
                                convolutionLayer->base.temp_tensors[numTmpTensor++] = outputs_rs;
                            }
                            else
                            {
                                outputs_rs = outputs;
                            }

                            shaderExecutable = vxnneGPUConv2D_1x1ShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_CONVOLUTION_1X1,
                                &node->kernelAttributes.borderMode, enable_tensor_cast || enable_ofm_gt_xy, enable_packed_weights, input_rs, weights_new_rs, newBiases, outputs_rs);
                        }
                        else
                        {
                            outputs_rs = outputs;
                            shaderExecutable = vxnneGPUGemmShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_GEMM,
                                &node->kernelAttributes.borderMode, enable_packed_weights, input_rs, weights_new_rs, newBiases, outputs_rs);
                        }
                    }
                    else
                    {
                        outputs_rs = outputs;
                        shaderExecutable = vxnneGPUGemm_noBiasShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_GEMM_NOBIAS, &node->kernelAttributes.borderMode, input_rs, weights_new_rs, outputs_rs);
                    }
                }

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    return status;
                }

                status = vxnneShaderOperation_Initialize(&convolutionLayer->convolutionGemm_sh_operation,
                    &convolutionLayer->base,
                    VXNNE_OPERATOR_CONVOLUTION,
                    batchCount,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                    return status;

                if (batchCount > 1)
                {
                    vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
                    vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_NO_BATCH_BIT);
                }

                vxnneLayer_SetOperation(
                    &convolutionLayer->base,
                    &convolutionLayer->convolutionGemm_sh_operation.base,
                    idx++);

                vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)input_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)weights_new_rs, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)newBiases, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&convolutionLayer->convolutionGemm_sh_operation.base, (vx_reference)outputs_rs, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                convolutionLayer->base.temp_tensors[numTmpTensor++] = tensor2Row;

                break;
            }
        default:
            vxError("ERROR MODE: %d\n", mode);
            break;
    }

    convolutionLayer->base.num_temp_tensors = numTmpTensor;
    node->layer = &convolutionLayer->base;

exit:
    return status;
}
#endif

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[1];
    vx_tensor biases                = (vx_tensor)parameters[2];
    vx_scalar padX                  = (vx_scalar)parameters[3];
    vx_scalar padXRight             = (vx_scalar)parameters[4];
    vx_scalar padY                  = (vx_scalar)parameters[5];
    vx_scalar padYBottom            = (vx_scalar)parameters[6];
    vx_scalar dilationX             = (vx_scalar)parameters[9];
    vx_scalar dilationY             = (vx_scalar)parameters[10];
    vx_scalar strideX               = (vx_scalar)parameters[11];
    vx_scalar strideY               = (vx_scalar)parameters[12];
    vx_scalar depth_multiplier      = (vx_scalar)parameters[13];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[14];
    vx_tensor outputs               = (vx_tensor)parameters[17];

    if ((depth_multiplier != NULL) && (depth_multiplier->value->n32 > 0))
    {
        return vxoNNDepthwiseConvolutionLayerInitializer(node,
            inputs,
            VX_NULL, weights, biases,
            padX, padXRight, padY, padYBottom, VX_PAD_CONSTANT, VX_NULL,
            dilationX, dilationY, strideX, strideY, depth_multiplier,
            VX_NULL,
            VX_NULL, VX_NULL, VX_NULL,
            downScaleSizeRounding,
            outputs);

    }
    else
    {
#if REGISTER_FRAME
        vx_status status = VX_SUCCESS;
        vx_enum padmode = VX_PAD_CONSTANT;
        vx_reference params[] = {
            (vx_reference)inputs, VX_NULL, (vx_reference)weights, (vx_reference)biases, /*input, weights_bias, weights, biases*/
            (vx_reference)padX, (vx_reference)padXRight, (vx_reference)padY, (vx_reference)padYBottom, /*pad x, x_right, y, y_bottom, */
            (vx_reference)vxCreateScalar(node->base.context, VX_TYPE_ENUM, &padmode), VX_NULL, /*padmode, padconstant*/
            (vx_reference)dilationX, (vx_reference)dilationY, (vx_reference)strideX, (vx_reference)strideY, /*dilation x/y, stride x/y */
            VX_NULL, VX_NULL, VX_NULL, VX_NULL, /*relu, pool type, x, y */
            (vx_reference)downScaleSizeRounding, (vx_reference)outputs, /*rounding, output */
        };

        status = vxoNNDilationConvolutionLayerInitializer_Ext(node, params, gcmCOUNTOF(params));

        if (params[8])
            vxReleaseScalar((vx_scalar*)&params[8]);

        return status;
#else
        return vxoNNDilationConvolutionLayerInitializer(node,
            inputs,
            VX_NULL, weights, biases,
            padX, padXRight, padY, padYBottom, VX_PAD_CONSTANT, VX_NULL,
            dilationX, dilationY,
            strideX, strideY,
            VX_NULL,
            VX_NULL, VX_NULL, VX_NULL,
            downScaleSizeRounding,
            outputs);
#endif
    }
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

