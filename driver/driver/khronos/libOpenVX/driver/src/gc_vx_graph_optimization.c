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
#include <gc_vx_nn_util.h>
#include <gc_vx_nn_wb.h>
#include <gc_vx_nn_encoder.h>

#define CONV_ENTROPY            (0.5f)
#define DEPTHWISE_CONV_ENTROPY  (0.6f)
#define _GC_OBJ_ZONE            gcdZONE_VX_GRAPH
#define QUANT_BIT_WIDTH         (8)

static vx_uint32 optPhase = 1;

extern vx_int16 Fp32toBF16(vx_float32 val);

VX_INTERNAL_API vx_uint32 vxoGraphOptimization_getMaxKernelSizeX(vx_context ctxt)
{
    return ctxt->nnConfig.fixedFeature.nnMaxKXSize;
}

VX_INTERNAL_API vx_uint32 vxoGraphOptimization_getMaxKernelSizeY(vx_context ctxt)
{
    return ctxt->nnConfig.fixedFeature.nnMaxKYSize;
}

VX_INTERNAL_API vx_uint32 vxoGraphOptimization_getMaxKernelSizeZ(vx_context ctxt)
{
    return ctxt->nnConfig.fixedFeature.nnMaxKZSize;
}

VX_INTERNAL_API void vxoGraphOptimization_getMaxKernelSize(vx_context ctxt, vx_uint32 *xKernelSize, vx_uint32 *yKernelSize, vx_uint32 *zKernelSize)
{
    vxmASSERT(ctxt);

    if(VX_NULL != xKernelSize)
        *xKernelSize = vxoGraphOptimization_getMaxKernelSizeX(ctxt);
    if(VX_NULL != yKernelSize)
        *yKernelSize = vxoGraphOptimization_getMaxKernelSizeY(ctxt);
    if(VX_NULL != zKernelSize)
        *yKernelSize = vxoGraphOptimization_getMaxKernelSizeZ(ctxt);
}

VX_INTERNAL_API void vxoGraphOptimization_printTensorData(vx_char *name, vx_tensor tensor)
{
    vx_uint32 index, elementCount;
    FILE *fp = NULL;

    vxmASSERT(name);

    fp = fopen(name, "w");
    if(!fp)
        return;
    vxoTensor_GetTensorElementCount(tensor, &elementCount);

    for(index = 0; index < elementCount; index++)
    {
        if (fp)
        {
            fprintf(fp, "%f\n", vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index,
                                    TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor)));
        }
    }

    fclose(fp);
}

/*get the max and miin of the tensor*/
VX_INTERNAL_API void vxoGraphOptimization_getTensorMaxOrMinValue(vx_tensor tensor, vx_float32* minData, vx_float32 *maxData)
{
    vx_uint32 index, elementCount;
    vx_float32 minD = 0.0f, maxD = 0.0f;

    vxmASSERT(tensor);

    vxoTensor_GetTensorElementCount(tensor, &elementCount);
    for(index = 0; index < elementCount; index++)
    {
        vx_float32 realData = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index,
                                    TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor));
        if(index == 1)
        {
            minD = realData;
            maxD = realData;
        }
        minD = gcmMIN(minD, realData);
        maxD = gcmMAX(maxD, realData);
    }

    if(minData) *minData = minD;
    if(maxData) *maxData = maxD;
}

VX_INTERNAL_API vx_uint32 vxoGraphOptimization_getOutputIndex(vx_node node)
{
    vx_uint32   i       = 0;

    gcmHEADER_ARG("node=%p", node);
    CHECK_NULL(node);

    for(i = 0; i < node->numParameters; i++)
    {
        if(VX_OUTPUT == node->kernel->signature.directionTable[i])
        {
            gcmFOOTER_NO();
            return  i;
        }
    }

    vxError("can not get node(%s) 's output index", node->kernel->name);
    vxmASSERT(0);
    return node->numParameters - 1;
}

VX_INTERNAL_API vx_reference vxoGraphOptimization_getOutputParameter(vx_node node)
{
    vx_uint32 index = 0;
    vx_reference output = NULL;

    gcmHEADER_ARG("node=%p", node);
    CHECK_NULL(node);

    index = vxoGraphOptimization_getOutputIndex(node);
    output = node->paramTable[index];

    gcmFOOTER_ARG("0x%x", output);
    return output;
}

void swap(vx_float32 *a, vx_float32 *b)
{
    float temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

VX_INTERNAL_API void vxoGraphOptimization_quickSort(vx_float32 *data, vx_uint32 size, vx_uint32 begin, vx_uint32 end)
{
    int i, j;
    if (begin < end) {
        i = begin + 1;
        j = end;
        while (i < j) {
            if(data[i] > data[begin]) {
                swap(&data[i], &data[j]);
                j--;
            } else {
                i++;
            }
        }
        if (data[i] >= data[begin]) {
            i--;
        }
        swap(&data[begin], &data[i]);
        vxoGraphOptimization_quickSort(data, size, begin, i);
        vxoGraphOptimization_quickSort(data, size, j, end);
    }
}
VX_INTERNAL_API vx_uint32 vxoGraphOptimization_computeFinalKernelSize(vx_uint32 kernelsize, vx_uint32 stride)
{
    vx_uint32 alignedWidth = ((kernelsize % stride == 0) ? kernelsize : (kernelsize + (stride - kernelsize % stride)));

    return alignedWidth / stride;
}

VX_INTERNAL_API vx_bool vxoGraphOptimization_isV8(vx_reference ref)
{
    vx_context context = vxGetContext(ref);

    return (vx_bool)(context->nnConfig.derivedFeature.nnXYDPX  == 0 || context->nnConfig.derivedFeature.nnXYDPY  == 0);
}

VX_INTERNAL_API vx_bool vxoGraphOptimization_dwConvHalSupport(vx_tensor inputTensor)
{
    if(!vxoGraphOptimization_isV8((vx_reference)inputTensor))
        return vx_false_e;
    if(GET_HW_FEATURE_DWCONV(inputTensor) == 0)
        return vx_false_e;
    if(TENSOR_DATA_TYPE(inputTensor) == VX_TYPE_INT16 || TENSOR_DATA_TYPE(inputTensor) == VX_TYPE_UINT16)
        return vx_false_e;
    if(TENSOR_DATA_SIZE(inputTensor) == 2 && gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcFEATURE_BIT_DEPTHWISE_16BIT_FORMAT))
        return vx_false_e;
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoGraphOptimization_nnHalSupport(vx_tensor inputTensor)
{
    vx_bool     nnSupportFormat  = vx_false_e;
    vx_context  context          = vxGetContext((vx_reference)inputTensor);
    vx_enum     dataType         = TENSOR_DATA_TYPE(inputTensor);

    gcmHEADER_ARG("inputTensor=%p", inputTensor);

    switch (dataType)
    {
    case VX_TYPE_FLOAT16:
        if (context->nnConfig.fixedFeature.nnCoreCountFloat16 > 0)
            nnSupportFormat = vx_true_e;
        break;
    case VX_TYPE_INT16:
        if (context->nnConfig.fixedFeature.nnCoreCountInt16 > 0)
            nnSupportFormat = vx_true_e;
        break;
    case VX_TYPE_INT8:
        if (TENSOR_QUANT_TYPE(inputTensor) == VX_QUANT_AFFINE_SCALE &&
            context->nnConfig.fixedFeature.nnCoreCountInt8 > 0 &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ASYMMETRIC_INT8))
            nnSupportFormat = vx_true_e;
        else if (TENSOR_QUANT_TYPE(inputTensor) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
            context->nnConfig.fixedFeature.nnCoreCountInt8 > 0)
            nnSupportFormat = vx_true_e;
        else if (TENSOR_QUANT_TYPE(inputTensor) == VX_QUANT_DYNAMIC_FIXED_POINT &&
            context->nnConfig.fixedFeature.nnCoreCountInt8 > 0)
            nnSupportFormat = vx_true_e;
        break;

    case VX_TYPE_UINT8:
        if (TENSOR_QUANT_TYPE(inputTensor) == VX_QUANT_AFFINE_SCALE &&
            context->nnConfig.fixedFeature.nnCoreCountInt8 > 0 &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT))
            nnSupportFormat = vx_true_e;
        else if (TENSOR_QUANT_TYPE(inputTensor) == VX_QUANT_DYNAMIC_FIXED_POINT &&
            context->nnConfig.fixedFeature.nnCoreCountInt8 > 0)
            nnSupportFormat = vx_true_e;
        break;
    case VX_TYPE_BFLOAT16:
        if (context->nnConfig.fixedFeature.nnCoreCountBFloat16 > 0)
            nnSupportFormat = vx_true_e;
        break;
    case VX_TYPE_FLOAT32:
        if (context->nnConfig.fixedFeature.nnCoreCountBFloat16 > 0 &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FLOAT32_IO))
        {
            nnSupportFormat = vx_true_e;
        }
        break;
    default:
        break;
    }

    gcmFOOTER_ARG("0x%x", nnSupportFormat);
    return nnSupportFormat;
}

VX_PRIVATE_API void vxoGraphOptimization_getAsymQuantizeAttribute(vx_enum dataType, float maxValue, float minValue, float *scale, vx_int32 *zeroPoint)
{
    vx_uint32 drange = 255;

    gcmASSERT(scale || zeroPoint);

    maxValue = gcmMAX(maxValue, 0);
    minValue = gcmMIN(minValue, 0);
    *scale = (maxValue - minValue)/drange;

    if(VX_TYPE_UINT8 == dataType)
    {
        *zeroPoint = gcmMIN(255, gcmMAX(0, (vx_int32)roundRTNE(0 - minValue/ *scale)));
    }
    else if(VX_TYPE_INT8 == dataType)
    {
        *zeroPoint = gcmMIN(127, gcmMAX(-128, (vx_int32)roundRTNE(-128 - minValue/ *scale)));
    }
    else
    {
        vxError("unsupported datatype for asym quantize\n");
        vxmASSERT(0);
    }
}

VX_INTERNAL_API vx_status vxoGraphOptimization_computeQuantAttribute(vx_enum quantType, vx_enum dataType,
                                                                     vx_float32 maxValue, vx_float32 minValue,
                                                                     vx_int8 *fixedPointPos, vx_int32 *zeroPoint, vx_float32 * scale)
{
    gcmHEADER_ARG("quantType=%d, maxValue=%f, minValue=%f, fixedPointPos=%p, zeroPoint=%p, scale=%p",
        quantType, maxValue, minValue, fixedPointPos, zeroPoint, scale);
    if(quantType == VX_QUANT_AFFINE_SCALE)
    {
        vxoGraphOptimization_getAsymQuantizeAttribute(dataType, maxValue, minValue, scale, zeroPoint);
    }
    else if(quantType == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        gcmASSERT(fixedPointPos);
        if(VX_TYPE_INT8 != dataType)
        {
            vxError("unsupported datatype for dfp quantization\n");
            vxmASSERT(0);
        }
        minValue = (vx_float32)fabs(minValue);
        maxValue = gcmMAX(maxValue, minValue);
        if(maxValue <= 0.0)
        {
            vxInfo("can not compute quant attribute");
            return VX_ERROR_INVALID_PARAMETERS;
        }
        *fixedPointPos = (vx_int8) gcmMIN(12, QUANT_BIT_WIDTH - ceilf((float)gcoMATH_Log2(maxValue) + 1));
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_tensor_create_params_t vxoGraphOptimization_createParamsForTensor(vx_uint32 dimNum, vx_uint32 *dims, vx_enum dataType,
                                                                               vx_enum quantType,vx_uint8 fixedPos, vx_int32 zp,
                                                                               vx_float32 scale)
{
    vx_tensor_create_params_t p;

    gcmHEADER_ARG("dimNum=%d, dims=%p, dataType=%d, quantType=%d, fixedPos=%d, zp=%d, scale=%f",
        dimNum, dims, dataType, quantType, fixedPos, zp, scale);
    INITIALIZE_STRUCT(p);

    p.num_of_dims= dimNum;
    p.sizes = dims;
    p.data_format = dataType;
    p.quant_format = quantType;
    if(quantType == VX_QUANT_AFFINE_SCALE)
    {
        p.quant_data.affine.scale = scale;
        p.quant_data.affine.zeroPoint = zp;
    }
    else
    {
        p.quant_data.dfp.fixed_point_pos = fixedPos;
    }
    gcmFOOTER_NO();
    return p;
}

VX_PRIVATE_API vx_tensor_create_params_t vxoGraphOptimization_cloneParamsFromTensor(vx_tensor tensor)
{
    vx_tensor_create_params_t p;

    gcmHEADER_ARG("tensor=%p", tensor);
    p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(tensor), TENSOR_SIZES(tensor), TENSOR_DATA_TYPE(tensor),
        TENSOR_QUANT_TYPE(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor));

    gcmFOOTER_NO();
    return p;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_cloneTensor(vx_tensor tensor, vx_graph graph, vx_bool isVirtual)
{
    vx_tensor cloneTensor = NULL;
    vx_tensor_create_params_t p;

    gcmHEADER_ARG("tensor=%p", tensor);

    p = vxoGraphOptimization_cloneParamsFromTensor(tensor);

    if(isVirtual)
        cloneTensor = vxCreateVirtualTensor2(graph, &p, sizeof(p));
    else
        cloneTensor = vxCreateTensor2(graph->base.context, &p, sizeof(p));
    CHECK_NULL(cloneTensor);

    TENSOR_VALUED(cloneTensor)          = TENSOR_VALUED(tensor);
    TENSOR_PRECISION(cloneTensor)       = TENSOR_PRECISION(tensor);
    TENSOR_DATA_LIFETIME(cloneTensor)   = TENSOR_DATA_LIFETIME(tensor);

    gcmFOOTER_ARG("clone tensor = %p", cloneTensor);
    return cloneTensor;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_copyConstData2tensor(void *data, vx_tensor *tensor, vx_enum readOrWrite)
{
    vx_uint32 i = 0;
    vx_size stride_size[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_size view_start[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_size view_end[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    gcmHEADER_ARG("data=%p, tensor=%p, readOrWrite=0x%x", data, tensor, readOrWrite);

    stride_size[0] = TENSOR_DATA_SIZE(*tensor);
    for(i = 1; i < TENSOR_DIM_NUM(*tensor); i++)
        stride_size[i] = TENSOR_STRIDE_INDEX(*tensor, i);

    memset(view_start, 0, sizeof(view_start));

    for(i = 0; i < TENSOR_DIM_NUM(*tensor); i++)
    {
        view_end[i] = TENSOR_SIZE_INDEX(*tensor, i);
    }
    vxCopyTensorPatch(*tensor,TENSOR_DIM_NUM(*tensor), view_start, view_end, stride_size, data, readOrWrite, VX_MEMORY_TYPE_HOST);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_bool vxoGraphOptimization_avgPool(vx_uint32 input, vx_uint32 output, vx_uint32 *pad, vx_uint32 kernel, vx_uint32 stride)
{
    vx_uint32 tmp = (input + pad[0] + pad[1] -kernel);

    if(tmp % stride != 0)
        return vx_false_e;

    return vx_true_e;
}

VX_INTERNAL_API vx_enum vxoGraphOptimization_getKernelType(vx_node node)
{
    vx_enum opType = 0;
    node_op_type_e nodeOpType = OP_INVALID;

    gcmHEADER_ARG("node=%p", node);

    vxmASSERT(node);
    vxmASSERT(node->kernel);

    opType = node->kernel->enumeration;

    switch (opType) {
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        {
            CONVERT_CONV_TYPE(PARAM_CONV_RELU_POOLING_2);
            break;
        }
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
        {
            CONVERT_CONV_TYPE(PARAM_CONV_RELU_POOLING_1);
            break;
        }

    /*case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
        {
            nodeOpType = OP_CONVOLUTION;
            if (SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_ENABLE_RELU_INDEX], b) == vx_true_e)
            {
                nodeOpType = (node_op_type_e)(nodeOpType | OP_RELU);
            }
            break;
        }*/
    case VX_KERNEL_CONVOLUTION_LAYER:
        {
            /*crl2 hal require that : pad < 7 dilation = 0*/
            vx_tensor weight = (vx_tensor)node->paramTable[1];
            vx_uint32 strideX = SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX], u32);
            vx_uint32 strideY = SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX+1], u32);
            vx_uint32 weightX = TENSOR_SIZE_INDEX(weight, 0);
            vx_uint32 weightY = TENSOR_SIZE_INDEX(weight, 1);

            vx_uint32 padl = SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX], u32);
            vx_uint32 padr = SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + 1], u32);
            vx_uint32 padt = SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + 2], u32);
            vx_uint32 padb = SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + 3], u32);

            vx_tensor inputTensor = (vx_tensor)node->paramTable[0];

            vx_uint32 maxKernelSizeX = 0;
            vx_uint32 maxKernelSizeY = 0;
            vx_uint32 elementCount  = 1;

            vxoTensor_GetTensorElementCount(inputTensor, &elementCount);
            if (elementCount == GET_TENSOR_BATCH(inputTensor) )
                break;

            vxoGraphOptimization_getMaxKernelSize(node->base.context, &maxKernelSizeX, &maxKernelSizeY, VX_NULL);

            weightX = vxoGraphOptimization_computeFinalKernelSize(weightX, strideX);
            weightY = vxoGraphOptimization_computeFinalKernelSize(weightY, strideY);

            if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(weight) &&
                gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT))
                break;

            {
                if(TENSOR_SIZE_INDEX(weight, 0) * TENSOR_SIZE_INDEX(weight, 1) == 1 &&
                    padl* padr * padt * padb != 0)
                    break;
            }

            if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1) &&
                (TENSOR_SIZE_INDEX(inputTensor,0) > NN_IMAGE_XSIZE_MAX || TENSOR_SIZE_INDEX(inputTensor, 1) > NN_IMAGE_YSIZE_MAX) )
                break;
            if(weightX > maxKernelSizeX || weightY > maxKernelSizeY )
                break;

            if(VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME(weight) &&
                 (node->paramTable[2] != VX_NULL ? VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[2]) : vx_true_e) &&
                SCALAR_VALUE(node->paramTable[PARAM_CONV_DILATION_INDEX],u32) == 0
                )
            {
                if((strideX == 1)  && (strideY == 1))
                {
                    vx_uint32 padSize = 0;
                    if(GET_HW_FEATURE_PAD_BIT_SIZE(node) > 0)
                        padSize = (vx_uint32)pow(2.0f, (vx_int32) GET_HW_FEATURE_PAD_BIT_SIZE(node) - 1); /*max pad size = 2 ^ (offset - 1)*/

                    if(vxoGraphOptimization_nnHalSupport(weight) &&
                        ((SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX], u32) > padSize) ||(SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + 2], u32) > padSize) )
                      )
                    {
                        nodeOpType = OP_CONVOLUTION_PAD;
                        break;
                    }
                }

                if(SCALAR_VALUE(node->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32) == 0)
                {
                    if(!(vxoGraphOptimization_isV8((vx_reference)weight)) &&
                        (weightX == weightY || weightX == 1 )/*tempically pad 1xN to NxN kernel because of terrible arch*/
                        )
                    {
                        nodeOpType = OP_CONVOLUTION;
                    }
                    else if((vxoGraphOptimization_isV8((vx_reference)weight)) && (weightX * weightY != 2) )
                    {
                        nodeOpType = OP_CONVOLUTION;
                    }
                    else if(vxoGraphOptimization_nnHalSupport(weight) /*&& (strideX == 1) && (strideY == 1) */)
                    {
                        nodeOpType = OP_CONVOLUTION_NxM;
                    }

                 }
                else
                {
                    if(SCALAR_VALUE(node->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32) == 1 &&
                        vxoGraphOptimization_dwConvHalSupport(weight) &&
                        (TENSOR_SIZE_INDEX(weight, 0) != 1 || TENSOR_SIZE_INDEX(weight, 1) != 1 ) &&
                        (strideX == strideY && (strideX == 1 || strideX == 2))
                        )
                    {
                        if(TENSOR_SIZE_INDEX(weight, 0) * TENSOR_SIZE_INDEX(weight, 1) != 2)
                        {
                           nodeOpType = OP_CONVOLUTION;
                        }
                        else
                        {
                            nodeOpType = OP_CONVOLUTION_NxM;
                        }
                    }
                    else
                    {
                        nodeOpType = OP_CONVOLUTION_DW;/*unroll dewiseconv*/
                    }
                }
            }
            break;
        }
    case VX_KERNEL_ACTIVATION_LAYER:
        {
            if(SCALAR_VALUE(node->paramTable[1], u32) == VX_NN_ACTIVATION_RELU)
                nodeOpType = OP_RELU;
            else if(SCALAR_VALUE(node->paramTable[1], u32) == VX_NN_ACTIVATION_RELU1)
                nodeOpType = OP_RELU1;
            else if(SCALAR_VALUE(node->paramTable[1], u32) == VX_NN_ACTIVATION_RELU6)
                nodeOpType = OP_RELU6;
            break;
        }
    case VX_KERNEL_NN_POOLING_LAYER2:
        {
            vx_enum poolType = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_TYPE_INDEX], u32);
            vx_tensor input = (vx_tensor)node->paramTable[0];
            vx_tensor output = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);
            vx_uint32 input_w = input->dims[0];
            vx_uint32 input_h = input->dims[1];
            vx_uint32 output_w = output->dims[0];
            vx_uint32 output_h = output->dims[1];

            vx_uint32   kernelx = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
            vx_uint32   kernely = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
            vx_uint32 pad[] = {
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_L_INDEX], u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_R_INDEX], u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_T_INDEX], u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_B_INDEX], u32)
            };

            vx_int32 stride_x = 1, stride_y = 1;
            if(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX] != NULL && node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX] != NULL)
            {
                stride_x = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX], u32);
                stride_y = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX], u32);
            }
            else
            {
                vxWarning("stride paramters is not passed, it will be computed");
                stride_x = (vx_int32)roundRTNE((vx_float32)(input_w + pad[0] + pad[1] - kernelx)/(output_w == 1? 1: output_w -1));
                stride_y = (vx_int32)roundRTNE((vx_float32)(input_h + pad[2] + pad[3]- kernely)/(output_h == 1? 1: output_h-1));
                stride_x = stride_x == 0 ? 1: stride_x;
                stride_y = stride_y == 0 ? 1: stride_y;
            }

            if(poolType == VX_NN_POOLING_MAX)
            {
                vx_uint32 poolx = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
                vx_uint32 pooly = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
                if(poolx == pooly && pad[0]== 0 && pad[2]== 0  && pad[1]== 0 && pad[3]== 0   &&
                    (poolx == 2 || (poolx == 3 &&  !vxoGraphOptimization_isV8((vx_reference)node)) )
                  )
                {
                    if((input_w - kernelx + pad[0] + pad[1]) % stride_x != 0 ||
                        (input_h - kernely + pad[2] + pad[3]) % stride_y != 0 )
                        break;
                    else if(((TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE && TENSOR_DATA_SIZE(input) == 1) || /*AFFINE support i8 and u8*/
                               (TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT && TENSOR_DATA_TYPE(input) == VX_TYPE_INT8)) &&
                        (input_w > 64 || input_h > 64) &&
                        poolx == 3)
                        break;
                    else if((TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT32 ||TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16) &&
                        (input_w > 32 || input_h > 32) &&
                        poolx == 3)
                        break;
                    else if((TENSOR_SIZE_INDEX(input, 0) > GET_HW_FEATURE_MAD_PER_CORE(input) ) && (poolx == 3) )
                        break;

                    if(stride_x == 2 && stride_y == 2)
                        nodeOpType = OP_POOLING;
                }
            }
            else if(poolType == VX_NN_POOLING_AVG || poolType == VX_NN_POOLING_AVG_ANDROID)
            {
                /*TODO: whether to convert VX_NN_POOLING_AVG_ANDROID to CONV*/
                vx_enum roundMode = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_ROUND_MODE_INDEX], u32);
                if(TENSOR_DATA_TYPE(input) != TENSOR_DATA_TYPE(output))
                {
                    if(TENSOR_DATA_TYPE(output) != VX_TYPE_BFLOAT16 &&
                        (TENSOR_DATA_TYPE(input) != VX_TYPE_FLOAT32 || TENSOR_DATA_TYPE(input) != VX_TYPE_BFLOAT16))
                    break;
                }
                /*hw only support i8 or i16 for dfp */
                if((!(TENSOR_DATA_TYPE(input) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(input) == VX_TYPE_INT16)) &&
                     TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT )
                    break;


                if((poolType == VX_NN_POOLING_AVG_ANDROID) &&
                    (pad[0] + pad[1] + pad[2] + pad[3] != 0) )
                    break;

                if(roundMode == VX_NN_DS_SIZE_ROUNDING_FLOOR)
                {
                    nodeOpType = OP_AVG_POOL;
                }
                else if(roundMode == VX_NN_DS_SIZE_ROUNDING_CEILING)
                {
                    /*AvgPool rounding mode is ceil, and output = input + 2 * pad - kernel) /stride & is interger.*/
                    if(vxoGraphOptimization_avgPool(input_w, output_w, pad, kernelx, stride_x) &&
                        vxoGraphOptimization_avgPool(input_h, output_h, pad + 2, kernely, stride_y)
                        )
                        nodeOpType = OP_AVG_POOL;
                }
            }
            break;
        }
    case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        {
            nodeOpType = OP_FULLYCONNECTED;
            {
                /* for batchfc, batch dims will be transposed to width in batchfc2conv feature,
                so it must be required by some condition
                */
                vx_tensor input = (vx_tensor)node->paramTable[0];
                vx_uint32 batch = GET_TENSOR_BATCH(input);
                vx_uint32 elementCount = 1;

                vxoTensor_GetTensorElementCount(input, &elementCount);
                if (elementCount == batch )
                    break;

                if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1) &&
                    batch > NN_IMAGE_XSIZE_MAX )
                    break;
            }

            if (SCALAR_VALUE(node->paramTable[PARAM_FULLYCONNECTED_RELU_ENABLE_RELU_INDEX], b))
            {
                nodeOpType = (node_op_type_e)(nodeOpType | OP_RELU);
            }
            break;
        }
    case VX_KERNEL_FULLY_CONNECTED_LAYER:
    case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        {
            vx_bool nnSupport = vxoGraphOptimization_nnHalSupport((vx_tensor)node->paramTable[0]);
            {
                /*TODO:for batch fc 2 conv feature, the input of conv can be (m x n) instead of (batch x 1)
                  which is more generous, and then the condition should be modified
                */
                /* for batchfc, batch dims will be transposed to width in batchfc2conv feature,
                so it must be required by some condition
                */
                vx_tensor input = (vx_tensor)node->paramTable[0];
                vx_uint32 batch = GET_TENSOR_BATCH(input);

                vx_uint32 elementCount = 1;
                vxoTensor_GetTensorElementCount((vx_tensor)node->paramTable[0], &elementCount);
                if (elementCount == batch )
                    break;

                if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1) &&
                    batch > NN_IMAGE_XSIZE_MAX )
                    break;
            }
            {
                vx_tensor weight = (vx_tensor)node->paramTable[1];
                if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(weight))
                    break;
            }
            if(VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[1]) &&
                VX_TENSOR_RANK_SN == TENSOR_RANK((vx_tensor)node->paramTable[1]) )
            {
                nodeOpType = OP_FC_ANDROID;
            }
            else if(VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[1]) &&
                (node->paramTable[2] != VX_NULL ? VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[2]) : vx_true_e) &&
                (GET_TENSOR_BATCH((vx_tensor)node->paramTable[1]) > 1 || nnSupport )) /*TP do not supoort output = 1, but NN do*/
            {
                /*if batch > 1, Converting BatchFC to NNConv will be done, so it must check NN support*/
                if(GET_TENSOR_BATCH((vx_tensor)node->paramTable[0]) == 1 ||
                    (GET_TENSOR_BATCH((vx_tensor)node->paramTable[0]) > 1 && nnSupport))
                    nodeOpType = OP_FULLYCONNECTED;
            }
            break;
        }

    case VX_KERNEL_NN_CONCATINDEFINITE_LAYER:
    case VX_KERNEL_NN_CONCAT2_LAYER:
        {
            nodeOpType = OP_CONCAT;
            break;
        }
    case VX_KERNEL_TENSOR_ADD:
    case VX_KERNEL_TENSOR_SUBTRACT:
        {
            vx_tensor input0 = (vx_tensor)node->paramTable[0];
            vx_tensor input1 = (vx_tensor)node->paramTable[1];
            vx_tensor output = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_uint32 i = 0;

            for(i = 0; i < TENSOR_DIM_NUM(output); i++)
            {
                if(TENSOR_SIZE_INDEX(input0, i) != TENSOR_SIZE_INDEX(input1, i))
                {
                    gcmFOOTER_ARG("%d", OP_ELTWISE_ASMD);
                    return OP_ELTWISE_ASMD;
                }
            }

            if((TENSOR_DATA_TYPE(input0) != TENSOR_DATA_TYPE(input1) ) ||
                (TENSOR_DIM_NUM(input0) != TENSOR_DIM_NUM(input1)     )
              )
            {
                gcmFOOTER_ARG("%d", OP_ELTWISE_ASMD);
                return OP_ELTWISE_ASMD;
            }

            switch (TENSOR_DATA_TYPE(input0))
            {
            case VX_TYPE_BFLOAT16:
                if(TENSOR_DATA_TYPE(output) == VX_TYPE_BFLOAT16)
                {
                    gcmFOOTER_ARG("%d", OP_ELTWISE_ASMD);
                    return OP_ADD_SUB;
                }
                break;
            case VX_TYPE_FLOAT16:
                if(TENSOR_DATA_TYPE(output) == VX_TYPE_FLOAT16 ||
                    TENSOR_DATA_TYPE(output) == VX_TYPE_UINT8 ||
                    TENSOR_DATA_TYPE(output) == VX_TYPE_INT8 )
                {
                    gcmFOOTER_ARG("%d", OP_ADD_SUB);
                    return OP_ADD_SUB;
                }
                break;
            case VX_TYPE_INT16:
            case VX_TYPE_UINT16:
                if(TENSOR_DATA_TYPE(output) == VX_TYPE_UINT16 ||
                    TENSOR_DATA_TYPE(output) == VX_TYPE_INT16  ||
                    TENSOR_DATA_TYPE(output) == VX_TYPE_UINT8  ||
                    TENSOR_DATA_TYPE(output) == VX_TYPE_INT8  )
                {
                    gcmFOOTER_ARG("%d", OP_ADD_SUB);
                    return OP_ADD_SUB;
                }
                break;
            case VX_TYPE_INT8:
            case VX_TYPE_UINT8:
                gcmFOOTER_ARG("%d", OP_ADD_SUB);
                return OP_ADD_SUB;
            default:
                break;
            }
            break;
        }
    case  VX_KERNEL_NN_TENSOR_RESHPE:
        {
            nodeOpType = OP_RESHAPE;
            break;
        }
    case VX_KERNEL_TENSOR_TRANSPOSE:
        {
            nodeOpType = OP_TRANSPOSE;
            break;
        }
    case VX_KERNEL_ROI_POOLING_LAYER:
        {
            nodeOpType = OP_ROIPOOL;
            break;
        }
    case VX_KERNEL_TENSOR_MULTIPLY:
    case VX_KERNEL_NN_TENSOR_DIV:
        {
            nodeOpType =  OP_ELTWISE_ASMD;
            break;
        }
    case VX_KERNEL_NN_PRELU:
        {
            nodeOpType = OP_PRELU;
            break;
        }
    case VX_KERNEL_NN_LEAKY:
        {
            nodeOpType = OP_LEAKYRELU;
            break;
        }
    default:
        break;
    }

    gcmFOOTER_ARG("%d", nodeOpType);
    return nodeOpType;
}

VX_INTERNAL_API vx_bool vxoGraphOptimization_isSameShapeTensor(vx_tensor tensor1, vx_tensor tensor2)
{
    vx_uint32 i = 0;
    vxmASSERT(tensor1 != NULL && tensor2 != NULL);

    if(TENSOR_DIM_NUM(tensor1) != TENSOR_DIM_NUM(tensor2))
        return vx_false_e;

    for(i = 0; i < TENSOR_DIM_NUM(tensor1); i++)
    {
        if(TENSOR_SIZE_INDEX(tensor1,i) != TENSOR_SIZE_INDEX(tensor2, i))
            return vx_false_e;
    }

    return vx_true_e;
}
VX_PRIVATE_API vx_tensor vxoGraphOptimization_CreateShareTensor(vx_tensor orginalTensor, vx_int32 *reshapeDims, vx_uint32 dimCount)
{
    vx_tensor reshapeTensor = vxReshapeTensor(orginalTensor, reshapeDims, dimCount);
    gcmHEADER_ARG("orginalTensor=%p, reshapeDims=%p, dimCount=0x%x", orginalTensor, reshapeDims, dimCount);

    TENSOR_DATA_LIFETIME(reshapeTensor)  = TENSOR_DATA_LIFETIME(orginalTensor);
    TENSOR_PRECISION(reshapeTensor)      = TENSOR_PRECISION(orginalTensor);
    TENSOR_VALUED(reshapeTensor)         = TENSOR_VALUED(orginalTensor);
    reshapeTensor->reshape = orginalTensor;

    /*vxoReference_Increment((vx_reference)orginalTensor, VX_REF_INTERNAL);*/

    gcmFOOTER_ARG("reshapeTensor=%p", reshapeTensor);
    return reshapeTensor;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_reshapeTensorAsOld(vx_tensor oldTensor, vx_tensor newTensor)
{
    vx_tensor tmpTensor = newTensor;
    if(!vxoGraphOptimization_isSameShapeTensor(oldTensor, newTensor))
    {
        tmpTensor = vxoGraphOptimization_CreateShareTensor(newTensor, (vx_int32 *)TENSOR_SIZES(oldTensor), TENSOR_DIM_NUM(oldTensor));
    }

    return tmpTensor;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_updateTensorInNodeWithIndex(vx_node *currentNode, vx_uint32 idex, vx_tensor newTensor)
{
    vx_tensor oldTensor = (vx_tensor)(*currentNode)->paramTable[idex];
    vx_tensor replaceTensor = newTensor;
    gcmHEADER_ARG("currentNode=%p, idex=0x%x, newTensor=%p", currentNode, idex, newTensor);

    replaceTensor = vxoGraphOptimization_reshapeTensorAsOld(oldTensor, newTensor);

    vxoNode_SetParameter(*currentNode, idex, (vx_reference)replaceTensor);

    if(replaceTensor != newTensor)
        vxReleaseTensor(&replaceTensor);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*
    find the related tensor's position in node's paramTable, and return it with index.
    if return false,
        1. it do not find the related tensor's index.
        2. or the tensor is viewed, which is not allowed.
*/
VX_PRIVATE_API vx_bool vxoGraphOptimization_matchTensorInNode(vx_node node, vx_tensor tensor, vx_uint32 *index)
{
    vx_uint32 k = 0;
    for(k = 0; k < node->numParameters; k++)
    {
        if(NULL == node->paramTable[k] || node->paramTable[k]->type != VX_TYPE_TENSOR)
            continue;
        {
            vx_tensor t = (vx_tensor)node->paramTable[k];
            if(t->isViewed || tensor->isViewed)
                continue;
            if(t->tensorBuffer != tensor->tensorBuffer)
                continue;

            /*with same memory size*/
            if(TENSOR_STRIDE_INDEX(t, TENSOR_DIM_NUM(t)) !=
                TENSOR_STRIDE_INDEX(tensor, TENSOR_DIM_NUM(tensor)))
                continue;

            if(index != VX_NULL)
                *index = k;
            return vx_true_e;
        }
    }

    return vx_false_e;
}

VX_PRIVATE_API void vxoGraphOptimization_updateTensorInNode(vx_node node, vx_tensor oldTensor, vx_tensor newTensor)
{
    vx_uint32 index = 0;
    if(vxoGraphOptimization_matchTensorInNode(node, oldTensor, &index))
    {
        vxoGraphOptimization_updateTensorInNodeWithIndex(&node, index, newTensor);
    }
}

/*
    traverse all of nodes that is related to currentNode in the whole graph,
and update their oldTensor with newTensor.
    first to update all of twins node and then parents node.
*/
VX_PRIVATE_API vx_status vxoGraphOptimization_updateTensorInGraph(vx_node currentNode, vx_tensor *oldTensor,
                                                                                vx_tensor *newTensor, vx_uint32 tensorSize)
{
    vx_uint32 i = 0, j = 0, k = 0;
    vx_graph graph = currentNode->graph;
    vx_node *nodeTable = graph->nodeTable;

    gcmHEADER_ARG("currentNode=%p, oldTensor=%p, subtensor=%p, subTensorSize=0x%x", currentNode, oldTensor, newTensor, tensorSize);

    for (i = 0; i < tensorSize; i++)
    {
        for (j = 0; j < currentNode->numParents; j++)
        {
            vx_uint32 index = 0;
            vx_node parent = nodeTable[currentNode->parentNodes[j]];

            if(!vxoGraphOptimization_matchTensorInNode(parent,oldTensor[i], VX_NULL))
                continue;

            for(k = 0; k <parent->numChildren; k++)
            {
                vx_node twinsNode = nodeTable[parent->childNodes[k]];
                if(twinsNode == currentNode)
                    continue;

                /*update twins node with new tensor*/
                vxoGraphOptimization_updateTensorInNode(twinsNode, oldTensor[i], newTensor[i]);
            }

            /*find valid parent and update parent node*/
            if(parent->merged)
            {
                parent = parent->replacedBy;
            }

            if(vxoGraphOptimization_matchTensorInNode(parent, oldTensor[i], &index))
            {
                vxoGraphOptimization_updateTensorInNodeWithIndex(&parent, index, newTensor[i]);
                break;
            }
        }

        /*update the current node*/
        vxoGraphOptimization_updateTensorInNode(currentNode, oldTensor[i], newTensor[i]);
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_uint32* vxoGraphOptimization_kernelSize(vx_node convNode)
{
    vx_uint32 *kernelSize = NULL;
    vx_uint32 convType = convNode->kernel->enumeration;
    gcmHEADER_ARG("convNode=%p", convNode);

    if(convType == VX_KERNEL_CONVOLUTION_LAYER)
        kernelSize = TENSOR_SIZES((vx_tensor)convNode->paramTable[1]);
    else if(convType == VX_KERNEL_NN_CONVOLUTION_RELU_LAYER ||
        convType == VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER ||
        convType == VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2 )
    {
        vx_weights_biases_parameter weight_bias = (vx_weights_biases_parameter )convNode->paramTable[1];
        kernelSize = WB_WEIGHT_DIMS_SIZES(weight_bias);
    }

    gcmFOOTER_ARG("%p", kernelSize);
    return kernelSize;
}

VX_INTERNAL_API void vxoGraphOptimization_fillDims2paramters(vx_char *buf, vx_uint32 bufLen, vx_uint32 dims[], vx_uint32 dimNum, vx_char *paramterName, vxcJSON *paramters)
{
    vx_uint32 i = 0, offset = 0;
    gcmHEADER_ARG("buf=%s, bufLen=0x%x, dims=%p, paramterName=%s, paramters=%p", buf, bufLen, dims, paramterName, paramters);

    memset(buf, 0, sizeof(vx_char)*bufLen);
    for(i = 0; i < dimNum; i++)
    {
        gcoOS_PrintStrSafe(buf, bufLen, &offset, "%d", dims[i]);
        if(i != dimNum -1)
        {
            gcoOS_PrintStrSafe(buf, bufLen, &offset, " x ");
        }
    }
    /*sprintf(buf, "%d x %d x %d x %d", dims[0], dims[1], dims[2], dims[3]);     */
    vxoJson_AddStringToObject(paramters, paramterName, buf);

    gcmFOOTER_NO();
    return;
}
VX_PRIVATE_API void vxoGraphOptimization_stroeNodeInOutInfo(vxcJSON *paramters, vx_node node)
{
    vx_char buf[100] = {0};
    vx_uint32 i = 0, inCnt = 0, outCnt = 0;
    vx_reference ref = NULL;
    for(i = 0; i < node->numParameters; i++)
    {
        ref = node->paramTable[i];
        if(ref == NULL)
            continue;

        if(ref->type == VX_TYPE_TENSOR && node->kernel->signature.directionTable[i] == VX_INPUT && inCnt == 0)
        {
            inCnt++;
            vxoGraphOptimization_fillDims2paramters(buf, 100, TENSOR_SIZES((vx_tensor)ref),
                TENSOR_DIM_NUM((vx_tensor)ref), "input dims:", paramters);
        }
        else if(ref->type == VX_TYPE_TENSOR && node->kernel->signature.directionTable[i] == VX_OUTPUT && outCnt == 0)
        {
            outCnt++;
            vxoGraphOptimization_fillDims2paramters(buf, 100, TENSOR_SIZES((vx_tensor)ref),
                        TENSOR_DIM_NUM((vx_tensor)ref), "output dims:", paramters);
        }
    }
}

VX_INTERNAL_API void vxoGraphOptimization_stroeNodeDims2paramter(vxcJSON *paramters, vx_node node)
{
    vx_enum kernelType = node->kernel->enumeration;

    vx_char buf[100] = {0};
    vx_uint32   *dims = NULL;
    vx_uint32 dimNum = 0;

    gcmHEADER_ARG("paramters=%p, node=%p", paramters, node);

    switch (kernelType)
    {
    case VX_KERNEL_CONVOLUTION_LAYER:
    case VX_KERNEL_FULLY_CONNECTED_LAYER:
    case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        {
            dims = TENSOR_SIZES((vx_tensor)node->paramTable[1]);
            dimNum = TENSOR_DIM_NUM((vx_tensor)node->paramTable[1]);
        }
        break;
    case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
    case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        {
            dims = TENSOR_SIZES(WB_WEIGHT_TENSOR((vx_weights_biases_parameter)node->paramTable[1]));
            dimNum = TENSOR_DIM_NUM(WB_WEIGHT_TENSOR((vx_weights_biases_parameter)node->paramTable[1]));
        }
        break;
    default:
        break;
    }

    vxoGraphOptimization_fillDims2paramters(buf, 100, dims, dimNum, "filter_dims", paramters);
    gcmFOOTER_NO();
    return;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_stroeNodeDetail2json(vx_node node, vxcJSON *layer, vxcJSON *paramters)
{
    vx_enum     nodeType = node->kernel->enumeration;
    vx_char     opName[100] = {0};
    vx_uint32   offset = 0;

    gcmHEADER_ARG("node=%p, layer=%p, paramters=%p", node, layer, paramters);

    vxoGraphOptimization_stroeNodeInOutInfo(paramters, node);
    switch(nodeType)
    {
    case VX_KERNEL_CONVOLUTION_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s","conv");

        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);

        vxoJson_AddNumberToObject(paramters, "pad_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "pad_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + 2], u32));
        vxoJson_AddNumberToObject(paramters, "dilation_w",SCALAR_VALUE(node->paramTable[PARAM_CONV_DILATION_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "dilation_h",SCALAR_VALUE(node->paramTable[PARAM_CONV_DILATION_INDEX + 1], u32));
        vxoJson_AddNumberToObject(paramters, "stride_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "stride_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX + 1], u32));
        vxoJson_AddNumberToObject(paramters, "depth_mult", SCALAR_VALUE(node->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32));

        break;
    case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s","convolutionrelu");

        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);

        vxoJson_AddBoolToObject(paramters,"has_relu", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_ENABLE_RELU_INDEX],b) == vx_true_e);
        vxoJson_AddNumberToObject(paramters, "pad_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_PAD_X_INDX], u32));
        vxoJson_AddNumberToObject(paramters, "pad_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_PAD_Y_INDX], u32));

        break;
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s","convolutionrelupooling");

        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);
        vxoJson_AddBoolToObject(paramters,"has_relu", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_1_ENABLE_RELU_INDEX], b) == vx_true_e);
        vxoJson_AddNumberToObject(paramters, "pad_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_X_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "pad_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_Y_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "pool_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_1_POOL_SIZE_X_INDEX], u32));
        vxoJson_AddNumberToObject(paramters, "pool_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_1_POOL_SIZE_Y_INDEX], u32));

        break;
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s","convolutionrelupooling2");

        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);

        vxoJson_AddBoolToObject(paramters,"has_relu", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_ENABLE_RELU_INDEX],b) == vx_true_e);
        vxoJson_AddNumberToObject(paramters, "pad_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX], u32) );
        vxoJson_AddNumberToObject(paramters, "pad_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX + 2], u32) );
        vxoJson_AddNumberToObject(paramters, "dilation_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_DILATION_INDEX ], u32) );
        vxoJson_AddNumberToObject(paramters, "dilation_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_DILATION_INDEX + 1], u32) );
        vxoJson_AddNumberToObject(paramters, "pool_w", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_POOL_SIZE_X_INDEX ], u32));
        vxoJson_AddNumberToObject(paramters, "pool_h", SCALAR_VALUE(node->paramTable[PARAM_CONV_RELU_POOLING_2_POOL_SIZE_Y_INDEX ], u32));
        break;
    case VX_KERNEL_NN_POOLING_LAYER2:
    {
        vx_enum poolingType = SCALAR_VALUE(node->paramTable[1], u32);
        vx_char poolType[10] = {0};

        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "pool");

        offset = 0;
        if(poolingType == VX_NN_POOLING_AVG || poolingType == VX_NN_POOLING_AVG_ANDROID)
            gcoOS_PrintStrSafe(poolType, sizeof(poolType), &offset, "%s", "AVG");
        else if(poolingType == VX_NN_POOLING_MAX)
            gcoOS_PrintStrSafe(poolType, sizeof(poolType), &offset, "%s", "MAX");
        else if(poolingType == VX_NN_POOLING_L2)
            gcoOS_PrintStrSafe(poolType, sizeof(poolType), &offset, "%s", "L2");
        else
            gcoOS_PrintStrSafe(poolType, sizeof(poolType), &offset, "%s", "unknown");

        vxoJson_AddStringToObject(paramters, "type", poolType);
        {
            vx_char buf[100] = {0};

            vxoGraphOptimization_fillDims2paramters(buf, 100, TENSOR_SIZES((vx_tensor)node->paramTable[0]),
                TENSOR_DIM_NUM((vx_tensor)node->paramTable[0]),
                "input_dims(whcn)", paramters);
            vxoGraphOptimization_fillDims2paramters(buf, 100, TENSOR_SIZES((vx_tensor)node->paramTable[node->numParameters - 1]),
                TENSOR_DIM_NUM((vx_tensor)node->paramTable[node->numParameters - 1]),
                "output_dims(whcn)", paramters);
        }
        vxoJson_AddNumberToObject(paramters, "ksize_w", SCALAR_VALUE(node->paramTable[2], u32));
        vxoJson_AddNumberToObject(paramters, "ksize_h", SCALAR_VALUE(node->paramTable[3], u32));
        vxoJson_AddNumberToObject(paramters, "pad_w", SCALAR_VALUE(node->paramTable[4], u32));
        vxoJson_AddNumberToObject(paramters, "pad_h", SCALAR_VALUE(node->paramTable[6], u32));
        if(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX] != NULL && node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX] != NULL)
        {
            vxoJson_AddNumberToObject(paramters, "stride_x", SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX], u32));
            vxoJson_AddNumberToObject(paramters, "stride_y", SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX], u32));
        }
        break;
    }
    case VX_KERNEL_ACTIVATION_LAYER:
    {
        vx_enum activationType = SCALAR_VALUE(node->paramTable[1], u32);
        if(activationType == VX_NN_ACTIVATION_RELU)
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "relu");
        else if(activationType == VX_NN_ACTIVATION_RELU1)
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "relu1");
        else if(activationType == VX_NN_ACTIVATION_RELU6)
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "relu6");
        else if(activationType == VX_NN_ACTIVATION_LOGISTIC)
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "logistic");
        else if(activationType == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "tanh");
        else
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "unknown");
        break;
    }
    case VX_KERNEL_FULLY_CONNECTED_LAYER:
    case VX_KERNEL_NN_FULLY_CONNECTED_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "fullyconnected");
        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);
        break;
    case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "fullyconnectedrelu");
        vxoGraphOptimization_stroeNodeDims2paramter(paramters, node);
        vxoJson_AddBoolToObject(paramters, "has_relu", SCALAR_VALUE(node->paramTable[7],b) == vx_true_e);
        break;
    case VX_KERNEL_NN_SOFTMAX2_LAYER:
        vxoJson_AddNumberToObject(paramters, "beta", SCALAR_VALUE(node->paramTable[1],f32));
    case VX_KERNEL_SOFTMAX_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "softmax");
        break;
    case VX_KERNEL_TENSOR_TRANSPOSE:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "transpose");
        break;

    case VX_KERNEL_TENSOR_ADD:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "add");
        break;
    case VX_KERNEL_NN_TENSOR_RESHPE:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "reshape");
        break;
    case VX_KERNEL_NORMALIZATION_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "normalization");
        break;
    case VX_KERNEL_NN_CONCATINDEFINITE_LAYER:
    case VX_KERNEL_NN_CONCAT2_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "concat");
        break;
    case VX_KERNEL_INTERNAL_ADAPTER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "adapter");
        break;
    case VX_KERNEL_NN_LEAKY:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "leakyrelu");
        break;
    case VX_KERNEL_NN_TENSOR_COPY:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "tensorcopy");
        break;
    case VX_KERNEL_INTERNAL_ROI_POOLING_RELU_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "ROIPoolingRelu");
        break;
    case VX_KERNEL_ROI_POOLING_LAYER:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "ROIPooling");
        break;
    case VX_KERNEL_NN_PRELU:
        {
            vx_tensor alpha = (vx_tensor)node->paramTable[1];
            float data = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(alpha), TENSOR_QUANT_TYPE(alpha), 0,
                TENSOR_LOGICAL_ADDR(alpha), TENSOR_POS(alpha), TENSOR_TF_ZEROPOINT(alpha), TENSOR_TF_SCALE(alpha));
            gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", "prelu");
            vxoJson_AddNumberToObject(paramters, "alpha", data);
            break;
        }
    default:
        gcoOS_PrintStrSafe(opName, sizeof(opName), &offset, "%s", node->kernel->name);
        break;
    }

    vxoJson_AddStringToObject(layer, "op", opName);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_storeNodeInfo(vx_node node, vxcJSON *layer)
{
    vxcJSON   *name       = NULL;
    vxcJSON   *paramters  = NULL;
    vxcJSON   *inputs     = NULL;
    vxcJSON   *outputs    = NULL;
    vxcJSON   *merged     = NULL;

    vx_uint32 i = 0;
    char buf[100] = {0};

    gcmHEADER_ARG("node=%p, layer=%p", node, layer);


    name = vxoJson_CreateString("null");
    CHECK_NULL(name);

    merged = vxoJson_CreateBool(node->merged == vx_true_e);

    paramters = vxoJson_CreateObject();
    CHECK_NULL(paramters);

    inputs = vxoJson_CreateArray();
    CHECK_NULL(inputs);

    outputs = vxoJson_CreateArray();
    CHECK_NULL(outputs );

    vxoJson_AddItemToObject(layer,"name",name);
    vxoGraphOptimization_stroeNodeDetail2json(node, layer, paramters);
    vxoJson_AddItemToObject(layer,"merged", merged);
    vxoJson_AddItemToObject(layer, "parameters", paramters);
    vxoJson_AddItemToObject(layer, "inputs", inputs);
    vxoJson_AddItemToObject(layer, "outputs", outputs);

    for(i = 0; i < node->numParents; i++)
    {
        vx_uint32 offset = 0;
        vx_node parentNode = node->graph->nodeTable[ node->parentNodes[i] ];
        vx_reference output = vxoGraphOptimization_getOutputParameter(parentNode);
        gcoOS_PrintStrSafe(buf, sizeof(buf), &offset, "@id_%d_uid_%d:out0", parentNode->nodeID, getUserIDFromOutputTensor(output));
        name = vxoJson_CreateString(buf);
        CHECK_NULL(name);
        vxoJson_AddItemToArray(inputs,name);
    }

    name = vxoJson_CreateString("out0");
    CHECK_NULL(name);
    vxoJson_AddItemToArray(outputs,name);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*store all of nodes into json*/
VX_INTERNAL_API char * vxoGraphOptimization_getJsonStyleInfo(vx_graph graph)
{
    vxcJSON   *topology = NULL;
    vxcJSON   *layers = NULL;
    vxcJSON   *layer = NULL;
    char    *string = NULL;
    vx_uint32 idx = 0;
    vx_node *nodeTable = graph->nodeTable;
    gcmHEADER_ARG("graph=%p", graph);
    topology = vxoJson_CreateObject();
    CHECK_NULL(topology);

    layers  = vxoJson_CreateObject();
    CHECK_NULL(layers);

    vxoJson_AddItemToObject(topology, "Layers", layers);

    for(idx = 0; idx < graph->nodeCount; idx ++)
    {
        char layerName[20] = {0};
        vx_uint32 offset = 0;
        vx_node currentNode = nodeTable[idx];
        vx_reference output = vxoGraphOptimization_getOutputParameter(currentNode);

        gcoOS_PrintStrSafe(layerName, sizeof(layerName), &offset, "id_%d_uid_%d", currentNode->nodeID, getUserIDFromOutputTensor(output));

        layer = vxoJson_CreateObject();
        vxoJson_AddItemToObject(layers,layerName,layer);

        vxoGraphOptimization_storeNodeInfo(currentNode, layer);
    }

    string = vxoJson_Print(topology);
    vxoJson_Delete(topology);

    gcmFOOTER_ARG("%s", string);
    return string;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_dumpTopology(vx_graph graph, char * filename)
{
    FILE *fid = NULL;
    vx_char *topologyInfo;

    gcmHEADER_ARG("graph=%p, filename=%s", graph, filename);

    vxmASSERT(graph);

    fid = fopen(filename, "wb");
    CHECK_NULL(fid);

    topologyInfo = vxoGraphOptimization_getJsonStyleInfo(graph);
    fprintf(fid, "%s\n", topologyInfo);
    vxFree(topologyInfo);

    fclose(fid);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/* swap the postion of leaky relu and maxpool in the gragh to reduce input size for leakyrelu.*/
VX_INTERNAL_API vx_status vxoGraphOptimization_LayerSwaping(vx_graph graph)
{
    vx_uint32   nodeIndex = 0;
    vx_node     leakyReluNode = VX_NULL, maxPoolNode = VX_NULL;
    vx_uint32   nodeCount = graph->nodeCount;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    /* traverse the node table to find the leakyrelu-maxpooling cascade relationship, */
    /* and swap their postion in the graph to the maxpooling-leakyrelu cascade relationship.    */
    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_tensor leakyInput    = NULL;
        vx_tensor leakyOutput   = VX_NULL;
        vx_tensor maxpoolOutput = NULL;

        leakyReluNode = graph->nodeTable[nodeIndex];
        if ((!vxoNode_IsLeakyReluNode(leakyReluNode)) ||
             (leakyReluNode->numChildren != 1) )
            continue;

        if(leakyReluNode->kernel->enumeration == VX_KERNEL_NN_LEAKY &&
            SCALAR_VALUE(leakyReluNode->paramTable[1], f32) < 0)
            continue;

        maxPoolNode = graph->nodeTable[ leakyReluNode->childNodes[0] ];
        if((!vxoNode_IsMaxPoolingNode(maxPoolNode)) ||
            (maxPoolNode->numParents != 1) )
            continue;

        leakyInput     = (vx_tensor)leakyReluNode->paramTable[0];
        leakyOutput    = (vx_tensor)vxoGraphOptimization_getOutputParameter(leakyReluNode);
        maxpoolOutput  = (vx_tensor)maxPoolNode->paramTable[maxPoolNode->numParameters - 1];

        if(!vxoTensor_IsVirtualTensor(leakyOutput))
            continue;
        {
            vx_tensor intermediaTensor = vxoGraphOptimization_cloneTensor(maxpoolOutput, graph, vxoTensor_IsVirtualTensor(maxpoolOutput));
            if(NULL == intermediaTensor)
                continue;

            intermediaTensor->base.isVirtual = vx_true_e; /* set intermedia Tensor to b be virtual. */
            intermediaTensor->base.scope = (vx_reference)graph; /* scop of virtual tensor should be the current graph */
            vxoNode_SetParameter(maxPoolNode, 0, (vx_reference)leakyInput);
            vxoNode_SetParameter(leakyReluNode, leakyReluNode->numParameters - 1, (vx_reference)maxpoolOutput);


            vxoNode_SetParameter(leakyReluNode, 0, (vx_reference)intermediaTensor);
            vxoNode_SetParameter(maxPoolNode, maxPoolNode->numParameters - 1, (vx_reference)intermediaTensor);

            vxReleaseTensor(&intermediaTensor);
        }
    }

    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_tensor vxoGraphOptimization_WAR7_getAlignedTensor(vx_context context, vx_tensor orginTensor, vx_bool flag_16x16)
{
    vx_uint32       i = 0;
    vx_uint32       *alignedTendorDims    = NULL;
    vx_tensor       alignedTensor         = NULL;
    vx_tensor_create_params_t param;

    gcmHEADER_ARG("context=%p, orginTensor=%p, flag_16x16=0x%x", context, orginTensor, flag_16x16);

    alignedTendorDims     = (vx_uint32 *)vxAllocateAndZeroMemory(TENSOR_DIM_NUM(orginTensor) * sizeof(vx_uint32));
    CHECK_NULL(alignedTendorDims);

    for(i = 0; i < TENSOR_DIM_NUM(orginTensor); i++)
        alignedTendorDims[i] = (orginTensor->viewRegion.viewEnds[i] - orginTensor->viewRegion.viewStarts[i] );

    alignedTendorDims[0] = 16;
    if(flag_16x16)
        alignedTendorDims[1] = 16;

    param = vxoGraphOptimization_cloneParamsFromTensor(orginTensor);
    param.sizes = alignedTendorDims;

    alignedTensor = vxCreateTensor2(context, &param, sizeof(param));
    CHECK_NULL(alignedTensor);
    vxFree(alignedTendorDims);

    gcmFOOTER_ARG("alignedTensor=%p", alignedTensor);
    return alignedTensor;
}

VX_INTERNAL_API vx_tensor vxoGraphOptimization_WAR7_getAlignedTensorview(vx_context context, vx_tensor orginTensor, vx_tensor alignedTensor)
{
    vx_uint32       *input_start        = NULL;
    vx_tensor_view  alignView           = NULL;
    vx_tensor       viewedTensor        = NULL;
    gcmHEADER_ARG("context=%p, orginTensor=%p, alignedTensor=%p", context, orginTensor, alignedTensor);

    input_start         = (vx_uint32 *)vxAllocateAndZeroMemory(TENSOR_DIM_NUM(orginTensor) * sizeof(vx_uint32));
    CHECK_NULL(input_start);

    alignView = vxCreateTensorView(context, input_start, TENSOR_SIZES(orginTensor), (vx_uint8)TENSOR_DIM_NUM(orginTensor));
    CHECK_NULL(alignView);

    viewedTensor = vxoTensor_CreateTensorFromView(alignedTensor, alignView);
    CHECK_NULL(viewedTensor);

    vxFree(input_start);

    gcmFOOTER_ARG("viewedTensor=%p", viewedTensor);
    return viewedTensor;
}


VX_INTERNAL_API vx_status vxoGraphOptimization_WAR7_singleCascadedNodes(vx_graph graph)
{
    vx_context context = vxGetContext((vx_reference)graph );
    vx_uint32 nodeIndex;
    vx_node *nodeTable = graph->nodeTable;
    vx_uint32   *currentNodeKernelSize = NULL, *parentNodeKernelSize = NULL;
    gcmHEADER_ARG("graph=%p", graph);
    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node currentNode = nodeTable[nodeIndex];

        if(currentNode->isTraversal)
            continue;

        if(vxoNode_IsConvNode(currentNode) && currentNode->numParents == 1)
        {
            vx_node     parentNode                  = nodeTable[currentNode->parentNodes[0]];
            vx_tensor   currNodeInputTensor         = (vx_tensor)currentNode->paramTable[0];
            vx_tensor   currNodeOutputTensor        = (vx_tensor)currentNode->paramTable[currentNode->numParameters - 1];
            vx_tensor   parentNodeInputTensor       = (vx_tensor)parentNode->paramTable[0];
            vx_tensor   parentNodeOutputTensor      = (vx_tensor)parentNode->paramTable[parentNode->numParameters - 1];

            if(TENSOR_SIZE_INDEX(currNodeInputTensor, 0) != 14 ||
                TENSOR_SIZE_INDEX(currNodeInputTensor, 1) != 14 ||
                TENSOR_SIZE_INDEX(currNodeOutputTensor, 0) != 14 ||
                TENSOR_SIZE_INDEX(currNodeOutputTensor, 1) != 14 )
                continue;

            if(!vxoNode_IsConvNode(parentNode) || parentNode->numChildren != 1)
                continue;

            if(TENSOR_SIZE_INDEX(parentNodeInputTensor, 0) != 14 ||
                TENSOR_SIZE_INDEX(parentNodeInputTensor, 1) != 14 ||
                TENSOR_SIZE_INDEX(parentNodeOutputTensor, 0) != 14 ||
                TENSOR_SIZE_INDEX(parentNodeOutputTensor, 1) != 14 )
                continue;

            currentNodeKernelSize       = vxoGraphOptimization_kernelSize(currentNode);
            parentNodeKernelSize        = vxoGraphOptimization_kernelSize(parentNode);

            if(currentNodeKernelSize[0] != 3 || currentNodeKernelSize[1] != 3 ||
                parentNodeKernelSize[0]  != 1 || parentNodeKernelSize[1]  != 1)
                continue;
            {
                vx_tensor   alignedTensor       = vxoGraphOptimization_WAR7_getAlignedTensor(context, currNodeInputTensor, ENABLE_GRAPH_WAR7_16x16);
                vx_tensor   alignedTensorView   = vxoGraphOptimization_WAR7_getAlignedTensorview(context, currNodeInputTensor, alignedTensor);
                vx_tensor   currNodeInput       = alignedTensorView;

                vx_tensor   parentNodeOutput    = alignedTensor;

                /* update input tensor of the current node*/
                vxoNode_SetParameter(currentNode, 0, (vx_reference)currNodeInput);

                {

                    vx_enum     kernelTYpe          = parentNode->kernel->enumeration;

                    /* update ouput tensor of parent nodes*/
                    vxoNode_SetParameter(parentNode, parentNode->numParameters - 1, (vx_reference)parentNodeOutput);

                    if(kernelTYpe == VX_KERNEL_NN_CONVOLUTION_RELU_LAYER ||
                        kernelTYpe == VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER ||
                        kernelTYpe == VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2 )
                    {
                        vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parentNode->paramTable[1];
                        vx_uint32 z_offset = alignedTensor->dims[0] * alignedTensor->dims[1] * TENSOR_DATA_SIZE(alignedTensor);
                        replaceKernelBufferZOffset(WB_ZOFFSET_HANDLE_INDEX(weights_biases, 0),
                                                   WB_NUM_OF_VZ_INDEX(weights_biases, 0),
                                                   WB_MEM_LOGICAL_BASE_ADDR(weights_biases),
                                                   z_offset);
                    }
                }

                currentNode->isTraversal = vx_true_e;
            }
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_WAR7(vx_graph graph)
{

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    /* firstly, process the muti-parent node using tensor_view,
       and then, process 1to1 nodes that replace old tensor with
       aligned tensor */
    /*vxoGraphOptimization_WAR7_mutiCascadedNodes(graph);*/

    vxoGraphOptimization_WAR7_singleCascadedNodes(graph);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API  void vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(vx_node convNode, vx_tensor *weight, vx_tensor *bias,
                                                                    vx_size dilation[2], vx_uint32 stride[2], vx_uint32 pad[4],
                                                                    vx_enum *overflow_policy, vx_enum *rounding_policy,
                                                                    vx_enum *down_scale_size_rounding,
                                                                    vx_uint32 *depth_multipler, vx_enum *padMode ,vx_uint32 *padConst)
{
    if(weight)
        *weight                      = (vx_tensor)convNode->paramTable[1];
    if(bias)
        *bias                        = (vx_tensor)convNode->paramTable[2];
    if(stride)
    {
        stride[0]                   = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_STRIDE_INDEX], u32);
        stride[1]                   = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_STRIDE_INDEX + 1], u32);
    }
    if(dilation)
    {
        dilation[0]                 = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_DILATION_INDEX], u32);
        dilation[1]                 = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_DILATION_INDEX + 1], u32);
    }
    if(pad)
    {
        pad[0]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_INDEX], u32);
        pad[1]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_INDEX + 1], u32);
        pad[2]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_INDEX + 2], u32);
        pad[3]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_INDEX + 3], u32);
    }
    if(padMode)
        *padMode                     = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_MODE_INDEX], u32);
    if(padConst)
        *padConst                    = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_PAD_CONST_INDEX], u32);
    if(overflow_policy)
        *overflow_policy             = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_OVERFLOW_INDEX], u32);
    if(rounding_policy)
        *rounding_policy             = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_ROUNDING_INDEX], u32);
    if(down_scale_size_rounding)
        *down_scale_size_rounding    = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_DOWN_SCALE_SIZEROUNDING_INDEX], u32);
    if(depth_multipler)
        *depth_multipler             = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32);

}

VX_PRIVATE_API  void vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConvReluPool(vx_node convNode, vx_tensor *weight, vx_tensor *bias,
                                                                    vx_uint32 pool_size[2], vx_uint32 pad[4],vx_uint32 stride[2],
                                                                    vx_uint8 *accumulator_bits, vx_enum *overflow_policy, vx_enum *rounding_policy,
                                                                    vx_enum *down_scale_size_rounding, vx_bool *enable_relu, vx_enum *pool_type)
{
    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)convNode->paramTable[PARAM_CONV_RELU_POOLING_1_WEIGHTED_BIAS_INDEX];
    if(weight)
        *weight                      = WB_WEIGHT_TENSOR(wb);
    if(bias)
        *bias                        = WB_BIAS_TENSOR(wb);
    if(stride)
    {
        stride[0]                   = WB_ORG_STRIDE_X(wb) != 0 ? WB_ORG_STRIDE_X(wb) : WB_STRIDE_X(wb) ;
        stride[1]                   = WB_ORG_STRIDE_Y(wb) != 0 ? WB_ORG_STRIDE_Y(wb) : WB_STRIDE_Y(wb) ;
    }

    if(pad)
    {
        pad[0]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_X_INDEX], u32);
        pad[1]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_X_INDEX], u32);
        pad[2]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_Y_INDEX], u32);
        pad[3]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_PAD_Y_INDEX], u32);
    }

    if(pool_size)
    {
        pool_size[0]                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_POOL_SIZE_X_INDEX], u32);
        pool_size[1]                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_POOL_SIZE_Y_INDEX], u32);
    }

    if(pool_type)
        *pool_type                  = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_POOL_TYPE_INDEX], u8);

    if(enable_relu)
        *enable_relu                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_ENABLE_RELU_INDEX], b);

    if(accumulator_bits)
        *accumulator_bits           = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_ACCUMULATOR_BITS_INDEX], u8);

    if(overflow_policy)
        *overflow_policy            = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_OVERFLOW_INDEX], u32);

    if(rounding_policy)
        *rounding_policy            = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_ROUNDING_INDEX], u32);

    if(down_scale_size_rounding)
        *down_scale_size_rounding   = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_1_DOWN_SCALE_SIZEROUNDING_INDEX], u32);
}

VX_PRIVATE_API  void vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConvReluPool2(vx_node convNode, vx_tensor *weight, vx_tensor *bias,
                                                                    vx_size dilation[2], vx_uint32 pool_size[2], vx_uint32 pad[4],vx_uint32 stride[2],
                                                                    vx_uint8 *accumulator_bits, vx_enum *overflow_policy, vx_enum *rounding_policy,
                                                                    vx_enum *down_scale_size_rounding, vx_bool *enable_relu, vx_enum *pool_type,
                                                                    vx_enum *pad_mode, vx_uint32 *pad_const)
{
    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)convNode->paramTable[PARAM_CONV_RELU_POOLING_1_WEIGHTED_BIAS_INDEX];
    if(weight)
        *weight                      = WB_WEIGHT_TENSOR(wb);

    if(bias)
        *bias                        = WB_BIAS_TENSOR(wb);

    if(stride)
    {
        stride[0]                   = WB_STRIDE_X(wb);
        stride[1]                   = WB_STRIDE_Y(wb);
    }

    if(dilation)
    {
        dilation[0]                 = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_DILATION_INDEX], u32);
        dilation[1]                 = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_DILATION_INDEX + 1], u32);
    }

    if(pad)
    {
        pad[0]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX], u32);
        pad[1]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX + 1], u32);
        pad[2]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX + 2], u32);
        pad[3]                      = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_INDEX + 3], u32);
    }

    if(pool_size)
    {
        pool_size[0]                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_POOL_SIZE_X_INDEX], u32);
        pool_size[1]                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_POOL_SIZE_Y_INDEX], u32);
    }

    if(pool_type)
        *pool_type                  = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_POOL_TYPE_INDEX], u8);

    if(enable_relu)
        *enable_relu                = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_ENABLE_RELU_INDEX], b);

    if(accumulator_bits)
        *accumulator_bits           = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_ACCUMULATOR_BITS_INDEX], u8);

    if(overflow_policy)
        *overflow_policy            = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_OVERFLOW_INDEX], u32);

    if(rounding_policy)
        *rounding_policy            = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_ROUNDING_INDEX], u32);

    if(down_scale_size_rounding)
        *down_scale_size_rounding   = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_DOWN_SCALE_SIZEROUNDING_INDEX], u32);

    if(pad_mode)
        *pad_mode                   = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_MODE_INDEX], u32);

    if(pad_const)
        *pad_const                  = SCALAR_VALUE(convNode->paramTable[PARAM_CONV_RELU_POOLING_2_PAD_CONST_INDEX], u32) - \
                                        TENSOR_TF_ZEROPOINT((vx_tensor)convNode->paramTable[0]);
}

VX_INTERNAL_API  vx_weights_biases_parameter vxoGraphOptimization_CreateWBParameter(vx_enum  layer_type,
                                                                                     vx_nn_convolution_relu_pooling_params_t *wb_params,
                                                                                     vx_uint32 sizeOfParms,
                                                                                     vx_tensor input, vx_tensor convOutput,vx_tensor finalOutput,
                                                                                     vx_tensor weight, vx_tensor bias, vx_tensor prelu_alpha,
                                                                                     vx_scalar leakyRelu_perm)

{
    vx_weights_biases_parameter wb = VX_NULL;
    vx_weights_biases_parameter_optimizations_ext2_t opt =
    {
        {
            -1, TENSOR_DATA_TYPE(finalOutput), TENSOR_TF_ZEROPOINT(input), TENSOR_DIM_NUM(input), TENSOR_DIM_NUM(finalOutput)
        },
        TENSOR_TF_SCALE(input), TENSOR_TF_SCALE(finalOutput), TENSOR_DATA_TYPE(input), TENSOR_TF_ZEROPOINT(convOutput), TENSOR_TF_SCALE(convOutput), 0
    };

    if(VX_NULL != prelu_alpha)
    {
        wb = vxoCreateWeightsBiasesParameterFromTensorsPRelu(layer_type,
            TENSOR_SIZES(input), TENSOR_SIZES(convOutput), TENSOR_SIZES(finalOutput),
            (vx_nn_convolution_relu_pooling_params_t *)wb_params, sizeOfParms,
        (vx_weights_biases_parameter_optimizations_t *)&opt, sizeof(opt),
        weight, bias, prelu_alpha);
    }
    else if(NULL != leakyRelu_perm)
    {
        wb = vxoCreateWeightsBiasesParameterFromTensorsLeakyRelu(layer_type,
            TENSOR_SIZES(input), TENSOR_SIZES(convOutput), TENSOR_SIZES(finalOutput),
            (vx_nn_convolution_relu_pooling_params_t *)wb_params, sizeOfParms,
        (vx_weights_biases_parameter_optimizations_t *)&opt, sizeof(opt),
        weight, bias, leakyRelu_perm);
    }
    else
    {
        wb = vxCreateWeightsBiasesParameterFromTensors3(
            layer_type,
            TENSOR_SIZES(input), TENSOR_SIZES(convOutput), TENSOR_SIZES(finalOutput),
            (vx_nn_convolution_relu_pooling_params_t *)wb_params, sizeOfParms,
            (vx_weights_biases_parameter_optimizations_t *)&opt, sizeof(opt),
            weight, bias);
    }

    CHECK_NULL(wb);

    return wb;
}

VX_INTERNAL_API vx_node vxoGraphOptimization_transformConv(vx_graph graph, vx_tensor input, vx_tensor convOut, vx_tensor finalOutput,
                                                               vx_tensor weight, vx_tensor bias, vx_bool enable_relu,
                                                               vx_size dilation[2], vx_uint32 stride[2], vx_uint32 pad[4],
                                                               vx_uint8 accumulator_bits, vx_uint32 pool_size[2], vx_uint32 pad_const,
                                                               vx_enum pool_type, vx_enum pad_mode,
                                                               vx_enum overflow_policy, vx_enum rounding_policy,
                                                               vx_enum down_scale_size_rounding, vx_uint32 depth_multiplier,
                                                               vx_tensor prelu_alpha, vx_scalar leakyRelu_perm,
                                                               vx_uint32 mergedNodeCount, vx_float32 interScale[MERGED_NODE_COUNT_MAX],
                                                               vx_int32 interZeroPoint[MERGED_NODE_COUNT_MAX], vx_enum interDataType[MERGED_NODE_COUNT_MAX])
{
    vx_node node;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar vxPadConst = vxCreateScalar(context, VX_TYPE_UINT32, &pad_const);

    vx_nn_convolution_relu_pooling_params_ext3_t wb_params =
    {
        {
            {
                {
                    dilation[0], dilation[1], pad[0],pad[1],pad[2],pad[3],
                    accumulator_bits, overflow_policy, rounding_policy,down_scale_size_rounding,
                    enable_relu, pool_type, pool_size[0], pool_size[1], pad_mode, vxPadConst
                },
                stride[0], stride[1]
            },
            depth_multiplier, VX_TENSOR_RANK_WHCN, TENSOR_DATA_TYPE(finalOutput)
        },
        mergedNodeCount, interScale, interZeroPoint, interDataType
    };

    vx_weights_biases_parameter wb = vxoGraphOptimization_CreateWBParameter(
        VX_NN_CONVOLUTION_LAYER,
        (vx_nn_convolution_relu_pooling_params_t *)&wb_params,
        sizeof(wb_params),
        input, convOut, finalOutput, weight, bias, prelu_alpha, leakyRelu_perm);
    CHECK_NULL(wb);

    node = vxConvolutionReluPoolingLayer2(graph, input, wb, (vx_nn_convolution_relu_pooling_params)&wb_params,
        sizeof(wb_params), finalOutput);

    vxReleaseWeightsBiasesParameter(&wb);
    vxReleaseScalar(&vxPadConst);

    return node;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_MergeConvolutionNodes(vx_node nodes[], vx_uint32 nodeCount)
{
    vx_node newNode = VX_NULL;
    vx_tensor inputTensor = VX_NULL;
    vx_tensor convOutputTensor = VX_NULL;
    vx_tensor reluOutputTensor = VX_NULL;
    vx_tensor finalOutTensor = VX_NULL;
    vx_tensor weight = VX_NULL;
    vx_tensor bias = VX_NULL;

    vx_uint32 lastNodeOutputIndex = nodes[0]->numParameters - 1;
    vx_int32  i = 0;
    vx_context context = vxGetContext((vx_reference)nodes[0]);

    vx_size     dilation[2]                 = {0,0};
    vx_uint32   pool_size[2]                = {0, 0};
    vx_uint32   stride[2]                   = {0, 0};
    vx_uint32   pad[4]                      = {0,0,0,0};
    vx_uint8    accumulator_bits            = 0;
    vx_enum     overflow_policy             = VX_CONVERT_POLICY_WRAP;
    vx_enum     rounding_policy             = VX_ROUND_POLICY_TO_ZERO;
    vx_enum     down_scale_size_rounding    = VX_NN_DS_SIZE_ROUNDING_FLOOR;
    vx_bool     enable_relu                 = vx_false_e;
    vx_enum     pool_type                   = 0;
    vx_enum     pad_mode                    = VX_PAD_CONSTANT;
    vx_uint32   pad_const                   = 0;
    vx_uint32   depth_multiplier            = 0;

    vx_uint32   lastNode                    = 0;
    vx_bool     int16_check                 = vx_false_e;

    vx_tensor   prelu_alpha                 = VX_NULL;
    vx_scalar   leayRelu_scalar             = VX_NULL;
    vx_bool     convert_leaky_to_prelu      = vx_false_e;

    gcmHEADER_ARG("nodes=%p, nodeCount=0x%x", nodes, nodeCount);
    inputTensor = (vx_tensor)nodes[0]->paramTable[0];
    convOutputTensor = (vx_tensor)nodes[0]->paramTable[nodes[0]->numParameters - 1];

    if(!vxoGraphOptimization_nnHalSupport(inputTensor))
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    if(TENSOR_DATA_TYPE(inputTensor) == VX_TYPE_UINT16 || TENSOR_DATA_TYPE(inputTensor) == VX_TYPE_INT16)
        int16_check = vx_true_e;
    if(int16_check &&  TENSOR_DATA_TYPE(convOutputTensor) == VX_TYPE_FLOAT16)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    for(i = 0; i < (vx_int32)nodeCount; i++)
    {
        switch (nodes[i]->kernel->enumeration){
            case VX_KERNEL_CONVOLUTION_LAYER:{
                vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(
                    nodes[i], &weight, &bias,
                    dilation, stride, pad,
                    &overflow_policy,
                    &rounding_policy, &down_scale_size_rounding,
                    &depth_multiplier, &pad_mode, &pad_const);
                    break;
                }
            case VX_KERNEL_ACTIVATION_LAYER:{
                    enable_relu = vx_true_e;
                    lastNodeOutputIndex = vxoGraphOptimization_getOutputIndex(nodes[i]);
                    lastNode = i;
                    reluOutputTensor = (vx_tensor)nodes[i]->paramTable[lastNodeOutputIndex];
                    break;
                }
            case VX_KERNEL_NN_PRELU:
                {
                    if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU))
                        goto merging; /*break merging node*/
                    if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(weight))
                        goto merging;

                    prelu_alpha = (vx_tensor) nodes[i]->paramTable[1];
                    lastNodeOutputIndex = nodes[i]->numParameters - 1;
                    reluOutputTensor = (vx_tensor)nodes[i]->paramTable[lastNodeOutputIndex];
                    lastNode = i;
                    break;
                }
            case VX_KERNEL_NN_LEAKY:
                {
                    if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_LEAKY_RELU))
                        goto merging;
                    if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(weight))
                        goto merging;

                    if(VX_QUANT_AFFINE_SCALE == TENSOR_QUANT_TYPE(weight))
                    {
                        leayRelu_scalar = (vx_scalar) nodes[i]->paramTable[1];
                        lastNodeOutputIndex = nodes[i]->numParameters - 1;
                        reluOutputTensor = (vx_tensor)nodes[i]->paramTable[lastNodeOutputIndex];
                        lastNode = i;
                    }
                    else if(gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU) && VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(weight))
                    {
                        vx_float32 alpha = SCALAR_VALUE((vx_scalar) nodes[i]->paramTable[1], f32);
                        vx_uint32 j = 0, dims[1] = {TENSOR_SIZE_INDEX((vx_tensor)nodes[i]->paramTable[0], 2)};
                        vx_float32 *data = NULL;

                        /*create prelu_alpha tensor*/
                        vx_tensor_create_params_t tt = vxoGraphOptimization_createParamsForTensor(1, dims, VX_TYPE_FLOAT32, 0, 0, 0, 0.0f);
                        prelu_alpha = vxCreateTensor2(context, &tt, sizeof(tt));/*it needs to be released later*/
                        if(!prelu_alpha)
                            goto merging;

                        CHECK_STATUS(vxoTensor_AllocateMemory(prelu_alpha) );
                        data = (vx_float32 *)TENSOR_LOGICAL_ADDR(prelu_alpha);
                        for(j = 0; j < dims[0]; j++)
                            data[j] = alpha;

                        convert_leaky_to_prelu = vx_true_e;
                        lastNodeOutputIndex = nodes[i]->numParameters - 1;
                        reluOutputTensor = (vx_tensor)nodes[i]->paramTable[lastNodeOutputIndex];
                        lastNode = i;
                    }
                    else
                        goto merging; /*stop merging node and start to transform*/

                    break;
                }
            case VX_KERNEL_NN_POOLING_LAYER2:{
                    vx_bool diff = vx_false_e;
                    vx_uint32 idx = 0;
                    vx_tensor maxpInput = (vx_tensor)nodes[i]->paramTable[0];
                    vx_uint32 poolOuputIndex = vxoGraphOptimization_getOutputIndex(nodes[i]);
                    vx_enum data_type = TENSOR_DATA_TYPE((vx_tensor)nodes[i]->paramTable[poolOuputIndex]);
                    if(data_type == VX_TYPE_UINT16 || data_type == VX_TYPE_INT16)
                        goto merging;
                    if(int16_check && data_type == VX_TYPE_FLOAT16)
                        goto merging;
                    if(depth_multiplier == 1 )
                        goto merging;
                    if(!vxoGraphOptimization_isSameShapeTensor(convOutputTensor, maxpInput))
                        goto merging;

                    for(idx = 0; idx < TENSOR_DIM_NUM(maxpInput); idx ++)
                        if(TENSOR_SIZE_INDEX(maxpInput, i) != TENSOR_SIZE_INDEX(convOutputTensor, i))
                            diff = vx_true_e;
                    if(diff) goto merging;

                    /*it is different, depending on the hal info*/
                    if(context->nnConfig.fixedFeature.nnInputBufferDepth)
                    {
                        if(context->nnConfig.fixedFeature.nnInputBufferDepth - ceilf((float)weight->dims[1]/ stride[1]) + 1 < SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32))
                            goto merging;
                    }

                    pool_type = VX_NN_POOLING_MAX;
                    pool_size[0] = SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
                    pool_size[1] = SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
                    lastNodeOutputIndex = poolOuputIndex;
                    lastNode = i;
                    break;
                }
            default:
                vxWarning("do not merge no unified conv node\n");
                gcmFOOTER_ARG("%d", VX_SUCCESS);
                return VX_SUCCESS;
        }
    }/*for(i = 0; i < nodeCount; i++)*/

 merging:
    finalOutTensor = (vx_tensor)nodes[lastNode]->paramTable[lastNodeOutputIndex];
    if(!vxoGraphOptimization_isSameShapeTensor(finalOutTensor, convOutputTensor) && pool_type != VX_NN_POOLING_MAX)
    {
        finalOutTensor = vxReshapeTensor(finalOutTensor, (vx_int32 *)TENSOR_SIZES(convOutputTensor), TENSOR_DIM_NUM(convOutputTensor));
        CHECK_NULL(finalOutTensor);
        finalOutTensor->reshape = (vx_tensor)nodes[lastNode]->paramTable[lastNodeOutputIndex];
    }

    TENSOR_QUANT_TYPE(finalOutTensor)   = reluOutputTensor ? TENSOR_QUANT_TYPE(reluOutputTensor)    : TENSOR_QUANT_TYPE(convOutputTensor);
    TENSOR_TF_SCALE(finalOutTensor)     = reluOutputTensor ? TENSOR_TF_SCALE(reluOutputTensor)      : TENSOR_TF_SCALE(convOutputTensor);
    TENSOR_TF_ZEROPOINT(finalOutTensor) = reluOutputTensor ? TENSOR_TF_ZEROPOINT(reluOutputTensor)  : TENSOR_TF_ZEROPOINT(convOutputTensor) ;
    TENSOR_POS(finalOutTensor)          = reluOutputTensor ? TENSOR_POS(reluOutputTensor)           : TENSOR_POS(convOutputTensor);

    nodes[0]->merged = vx_true_e;

    {
        vx_float32 interScale[MERGED_NODE_COUNT_MAX];
        vx_int32 interZeroPoint[MERGED_NODE_COUNT_MAX];
        vx_enum   interDataType[MERGED_NODE_COUNT_MAX];
        for(i = 0; i < (vx_int32)lastNode; i++)
        {
            vx_tensor output = (vx_tensor)vxoGraphOptimization_getOutputParameter(nodes[i]);
            interScale[i] = TENSOR_TF_SCALE(output);
            interZeroPoint[i] = TENSOR_TF_ZEROPOINT(output);
            interDataType[i] = TENSOR_DATA_TYPE(output);
        }
        newNode = vxoGraphOptimization_transformConv(nodes[0]->graph, inputTensor, convOutputTensor,
                                                    finalOutTensor, weight, bias, enable_relu,
                                                    dilation, stride, pad,accumulator_bits,
                                                    pool_size, pad_const, pool_type, pad_mode,
                                                    overflow_policy, rounding_policy,
                                                    down_scale_size_rounding, depth_multiplier, prelu_alpha, leayRelu_scalar,
                                                    lastNode, interScale, interZeroPoint, interDataType);
        CHECK_NULL(newNode);
        newNode->kernelAttributes.isSetKernelVIP = nodes[0]->kernelAttributes.isSetKernelVIP;
    }

    vxReleaseNode(&newNode);

    if(prelu_alpha && convert_leaky_to_prelu )
    {
        vxReleaseTensor(&prelu_alpha);
    }

    if(!vxoGraphOptimization_isSameShapeTensor(finalOutTensor, (vx_tensor)nodes[lastNode]->paramTable[lastNodeOutputIndex]) )
    {
        CHECK_STATUS(vxReleaseTensor(&finalOutTensor) );
    }

    for(i = lastNode; i >=0; i--)
        nodes[i]->merged = vx_true_e;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_node vxoGraphOptimization_TransferFC2FCRelu(vx_node FCnode, vx_tensor reluOut)
{
    vx_node     FCReluNode = VX_NULL;
    vx_uint32   pad;
    vx_uint8    acc;
    vx_uint32   rounding, overflow, down_scale_round;
    vx_tensor   input = (vx_tensor)FCnode->paramTable[0];
    vx_tensor   weight = (vx_tensor)FCnode->paramTable[1];
    vx_tensor   bias   = (vx_tensor)FCnode->paramTable[2];
    vx_tensor   output = (vx_tensor)FCnode->paramTable[FCnode->numParameters - 1];
    vx_bool     isRelu = vx_false_e;

    gcmHEADER_ARG("FCnode=%p", FCnode);

    if(VX_NULL != reluOut)
    {
        isRelu = vx_true_e;
        output = vxoGraphOptimization_reshapeTensorAsOld(output, reluOut);
    }
    vxoNode_getInfoFromFCNode(FCnode, &pad, &acc, &rounding, &overflow, &down_scale_round);
    {
        vx_uint32 padValue = 0;
        vx_scalar padConst = vxCreateScalar(((vx_reference)FCnode)->context, VX_TYPE_UINT32, &padValue);
        vx_nn_convolution_relu_pooling_params_ext2_t params = {
                {
                        { 0, 0,
                          pad, pad, pad, pad, acc, overflow,
                          rounding, down_scale_round,
                          isRelu, 0, 0, 0, VX_PAD_CONSTANT, padConst},
                        1, 1
                },
                0,
                TENSOR_RANK(input),
                TENSOR_DATA_TYPE(output)
        };


        vx_weights_biases_parameter wb = vxoGraphOptimization_CreateWBParameter(
                                            VX_NN_FULLYCONNECTED_LAYER,
                                            (vx_nn_convolution_relu_pooling_params_t *)&params,
                                            sizeof(params),
                                            input, output, output, weight, bias, VX_NULL, VX_NULL);
        CHECK_NULL(wb);
        FCReluNode = vxFullyConnectedReluLayer(FCnode->graph, input, wb,
            pad,
            acc,
            overflow,
            rounding,
            down_scale_round,
            isRelu,
            output
        );

        vxReleaseScalar(&padConst);
        vxReleaseWeightsBiasesParameter(&wb);
        FCReluNode->kernelAttributes.isSetKernelVIP = FCnode->kernelAttributes.isSetKernelVIP;
    }
    gcmFOOTER_ARG("FCReluNode=%p", FCReluNode);
    return FCReluNode;
}


VX_INTERNAL_API vx_status vxoGraphOptimization_MergeFullyConnectedNodes(vx_node nodes[], vx_uint32 nodeCount)
{
    vx_bool newNodeflag = vx_false_e;
    vx_tensor reluOut = VX_NULL;
    gcmHEADER_ARG("nodes=%p, nodeCount=0x%x", nodes, nodeCount);

    if(!vxnneIsTPSupportFormat(
                    nodes[0]->graph,
                    (vx_tensor)nodes[0]->paramTable[0],
                    VX_NULL,
                    (vx_tensor)nodes[0]->paramTable[nodes[0]->numParameters -1] )
        )
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    if(nodeCount == 1 && vxoGraphOptimization_getKernelType(nodes[0]) == OP_FULLYCONNECTED_RELU)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    if(nodeCount == 2 && vxoGraphOptimization_getKernelType(nodes[1]) != OP_RELU)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    if(nodeCount == 2)
    {
        nodes[1]->merged = vx_true_e;
        reluOut = (vx_tensor)vxoGraphOptimization_getOutputParameter(nodes[1]);
    }
    if(VX_KERNEL_NN_FULLY_CONNECTED_LAYER == nodes[0]->kernel->enumeration ||
        VX_KERNEL_FULLY_CONNECTED_LAYER == nodes[0]->kernel->enumeration)
    {
        vx_node newNode = vxoGraphOptimization_TransferFC2FCRelu(nodes[0], reluOut);
        nodes[0]->merged = vx_true_e;
        nodes[0] = newNode;
        newNodeflag = vx_true_e;
    }

    if(newNodeflag)
        vxReleaseNode(&nodes[0]);
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

extern VX_INTERNAL_API vx_node vxROIPoolingReluLayer(
    vx_graph  graph,
    vx_tensor input_data,
    vx_tensor input_rois,
    const vx_nn_roi_pool_params_t *roi_pool_params,
    vx_size size_of_roi_params,
    vx_tensor output_arr,
    vx_bool enable_relu
    );

VX_INTERNAL_API vx_status vxoGraphOptimization_MergeROIPoolmodes(vx_node nodes[], vx_uint32 nodeCount)
{
    gcmHEADER_ARG("nodes=%p, nodeCount=0x%x", nodes, nodeCount);

    if(nodeCount == 1)
        return VX_SUCCESS;
    {
        vx_tensor input_data        = (vx_tensor)nodes[0]->paramTable[0];
        vx_tensor input_rois        = (vx_tensor)nodes[0]->paramTable[1];
        vx_scalar pool_types        = (vx_scalar)nodes[0]->paramTable[2];
        vx_scalar spatial_scales    = (vx_scalar)nodes[0]->paramTable[3];
        vx_scalar pooled_heights    = (vx_scalar)nodes[0]->paramTable[4];
        vx_scalar pooled_widths     = (vx_scalar)nodes[0]->paramTable[5];
        vx_tensor outputarr         = (vx_tensor)nodes[1]->paramTable[nodes[1]->numParameters - 1];

        vx_nn_roi_pool_params_ext_t p = {
                                        {SCALAR_VALUE(pool_types, n32)},
                                        SCALAR_VALUE(spatial_scales, f32),
                                        SCALAR_VALUE(pooled_heights, n32),
                                        SCALAR_VALUE(pooled_widths, n32)
                                    };
        vx_node newnode = vxROIPoolingReluLayer(nodes[0]->graph, input_data, input_rois,
            (const vx_nn_roi_pool_params_t *)&p, sizeof(p), outputarr, vx_true_e);
        CHECK_NULL(newnode);

        nodes[0]->merged = vx_true_e;
        nodes[1]->merged = vx_true_e;
        newnode->kernelAttributes.isSetKernelVIP = nodes[0]->kernelAttributes.isSetKernelVIP;
        vxReleaseNode(&newnode);
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*merge Convlution Node or FullyConnected Nodes*/
VX_INTERNAL_API vx_status vxoGraphOptimization_MergeWithChildNodes(vx_node node)
{
    vx_node next = node;
    vx_graph graph = NULL;
    vx_reference ref = NULL;
    vx_node mergedNodes[3];
    vx_uint32 nodeCount;
    vx_uint32 nodeIndex = 0;
    vx_enum opType = OP_INVALID;
    vx_enum currNodeType;
    vx_enum features[][2] = {
        {OP_CONVOLUTION, OP_LEAKYRELU},
        {OP_CONVOLUTION, OP_PRELU},
        {OP_CONVOLUTION, OP_RELU | OP_POOLING}, /*conv + relu + pool = convrelupool*/
        {OP_FULLYCONNECTED, OP_RELU}, /*fc + relu = fcRelu*/
        {OP_ROIPOOL, OP_RELU}                /*roipool + relu = roipoolrelu*/
    };

    vx_enum allFeatures = 0;
    vx_uint32 featuresNum = sizeof(features)/(2 * sizeof(vx_enum));
    vx_uint32 i = 0;

    gcmHEADER_ARG("node=%p", node);
    vxmASSERT(node);

    for(i = 0; i < featuresNum; i++)
    {
        allFeatures |= features[i][0];
    }

    /* find the head node for merging, convolution or FC. */
    if(!vxoNode_IsConvNode(node) && !vxoNode_IsFCNode(node) && vxoGraphOptimization_getKernelType(node) != OP_ROIPOOL)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    graph = node->graph;
    vxmASSERT(graph);

    opType = vxoGraphOptimization_getKernelType(next);
    mergedNodes[0] = next;
    for (nodeCount = 1;
        (nodeCount < MAX_MERGE_OPS);
        nodeCount++)
    {
        /*check whether to finish*/
        for(i = 0; i < featuresNum; i++)
        {
            if(opType & features[i][0])
            {
                if(opType == (features[i][0] | features[i][1]))
                    goto merge;
            }
        }

        if((next->numChildren != 1))
            break;

        /* stop merging if the node's output is normal tensor,
           application maybe want to check the tensor's result */
        ref = vxoGraphOptimization_getOutputParameter(next);
        if(ref->type == VX_TYPE_TENSOR && !vxoTensor_IsVirtualTensor((vx_tensor)ref))
            break;

        nodeIndex = next->childNodes[0];
        next = graph->nodeTable[nodeIndex];

        if(next == NULL || next->numParents > 1 || (next->merged == vx_true_e))
            break;

        currNodeType = vxoGraphOptimization_getKernelType(next);

        if(opType & currNodeType)
            break;

        /*only do one feature per time, check the merge condition*/
        for(i = 0; i < featuresNum; i++)
        {
            if(opType & features[i][0])
            {
                if((currNodeType & (allFeatures ^ features[i][0])) != 0)
                    goto merge;

                /*if some node has been picked, just check the relative feature that include the node type*/
                if((opType ^ features[i][0]) && (opType & features[i][1]) == 0 )
                    continue;

                if((currNodeType & features[i][1]) != 0)
                    break;
            }
        }
        /*for the node that will be merged, the above for loop should break. or it must goto merge*/
        if(i == featuresNum)
            goto merge;

        opType |= currNodeType;
        mergedNodes[nodeCount] = next;
    }

merge:
    /* merging Convolution or FC */
    if(opType & OP_CONVOLUTION)
    {
        vxoGraphOptimization_MergeConvolutionNodes(mergedNodes, nodeCount);
    }
    else if(opType & OP_FULLYCONNECTED)
    {
        vxoGraphOptimization_MergeFullyConnectedNodes(mergedNodes, nodeCount);
    }
    else if(opType & OP_ROIPOOL)
    {
        vxoGraphOptimization_MergeROIPoolmodes(mergedNodes, nodeCount);
    }
    else
    {
        vxError("merging fail in graph optimization\n");
        assert(0);
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_LayerMerge(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];
        if (node->merged) continue;

        vxoGraphOptimization_MergeWithChildNodes(node);
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_ConcatTensors(vx_context context, vx_tensor* tensorsIn, vx_uint32 numTensors,
                                                 vx_uint32 concatAxis, vx_tensor* tensorsSub, vx_tensor tensorPool)
{
    vx_uint32 i = 0, j = 0;
    vx_size viewStart[4];
    vx_size viewEnd[4];
    vx_size prevEnd = 0;
    gcmHEADER_ARG("context=%p, tensorsIn=%p, numTensors=0x%x, concatAxis=0x%x, tensorsSub=%p, tensorPool=%p",
        context, tensorsIn, numTensors, concatAxis, tensorsSub, tensorPool);

    /*for (i = 0; i < numTensors; i++)
    {
        if (!vxoTensor_IsVirtualTensor(tensorsIn[i])) {
            return VX_ERROR_INVALID_TYPE;
        }
    }*/

    memset(viewStart, 0, sizeof(viewStart));
    memset(viewEnd, 0, sizeof(viewEnd));

    if (tensorPool->isViewed)
    {
        vx_view_region_s* view = &tensorPool->viewRegion;
        for(j = 0; j < tensorsIn[0]->dimCount; j++)
        {
            if(j != concatAxis)
                viewEnd[j] = (vx_size)(view->viewEnds[j] - view->viewStarts[j]);
        }
    }
    else
    {
        for(i = 0; i < TENSOR_DIM_NUM(tensorPool); i++)
            viewEnd[i] = TENSOR_SIZE_INDEX(tensorPool, i);
    }

    for (i = 0; i < numTensors; i++)
    {
        vx_uint8 dimCount = (vx_uint8) tensorsIn[i]->dimCount;

        vxmASSERT(tensorsIn[i]->dimCount > concatAxis);

        viewStart[concatAxis] = prevEnd;
        viewEnd[concatAxis] = prevEnd + (tensorsIn[i]->isViewed ? TENSOR_VIEW_END_INDEX(tensorsIn[i], concatAxis) - TENSOR_VIEW_START_INDEX(tensorsIn[i], concatAxis) : TENSOR_SIZE_INDEX(tensorsIn[i] ,concatAxis));

        tensorsSub[i] = vxCreateTensorFromView(tensorPool, (vx_size)dimCount, viewStart, viewEnd);   CHECK_NULL(tensorsSub[i]);
        prevEnd = viewEnd[concatAxis];
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_dispelConcatNode(vx_node node)
{
    vx_graph    graph = node->graph;

    vx_uint32   numTensor = 2;
    vx_uint32   concatAxis = 0;
    vx_tensor   *tensorsIn, *tensorsSub;
    vx_tensor   tensorOut = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);

    vx_enum nodeType = node->kernel->enumeration;
    gcmHEADER_ARG("node=%p", node);
    if(nodeType == VX_KERNEL_NN_CONCAT2_LAYER)
    {
        tensorsIn = (vx_tensor *)node->paramTable;
        numTensor = 2;
        concatAxis = tensorsIn[0]->dimCount - 1;
    }
    else if(nodeType == VX_KERNEL_NN_CONCATINDEFINITE_LAYER)
    {
        vx_object_array tensorArray = (vx_object_array)(node->paramTable[0]) ;
        tensorsIn = (vx_tensor *) (tensorArray)->itemsTable;
        numTensor = (vx_uint32)tensorArray->itemCount;
        concatAxis = SCALAR_VALUE(node->paramTable[1], b);
    }
    else
    {
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }

    tensorsSub = (vx_tensor *)vxAllocateAndZeroMemory(sizeof(vx_tensor*) * numTensor);
    CHECK_NULL(tensorsSub);

    vxoGraphOptimization_ConcatTensors(vxGetContext((vx_reference)graph), tensorsIn, numTensor, concatAxis, tensorsSub, tensorOut);
    vxoGraphOptimization_updateTensorInGraph(node,tensorsIn, tensorsSub, numTensor);

    {
        vx_uint32 i = 0;
        for(i = 0; i < numTensor; i++)
            vxReleaseTensor(&tensorsSub[i] );
    }
    vxFree(tensorsSub);

    node->merged = vx_true_e;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}


VX_INTERNAL_API vx_status vxoGraphOptimization_DispelConcat(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];
        vx_enum opType = vxoGraphOptimization_getKernelType(node);

        if (opType != OP_CONCAT) continue;
        if (node->numParents == 0) continue;

        vxoGraphOptimization_dispelConcatNode(node);
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*
repalce the output of reshape's previos node with reshape's output,
it is only valid for non-branch part.
*/
VX_INTERNAL_API vx_status vxoGraphOptimization_DeleteReshape(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_uint32 index = 0;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node *nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];
        vx_enum opType = vxoGraphOptimization_getKernelType(node);

        if (opType == OP_RESHAPE && node->numParents == 1 && nodeTable[node->parentNodes[0]]->numChildren == 1)
        {
            vx_tensor reshapeIn = (vx_tensor)node->paramTable[0];
            vx_tensor reshapeOut = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_node pNode = nodeTable[node->parentNodes[0]];

            if(pNode->merged && pNode->replacedBy != NULL)
                pNode = pNode->replacedBy;

            if(vxoGraphOptimization_matchTensorInNode(pNode, reshapeIn, &index) )
            {
                vxoGraphOptimization_updateTensorInNodeWithIndex(&pNode, index, reshapeOut);
                node->merged = vx_true_e;
                node->replacedBy = pNode;
            }
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();

    gcmFOOTER_ARG("%d", VX_SUCCESS);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_GetShareTensor(vx_tensor orginalTensor)
{
    vx_uint32 i = 0;
    vx_uint32 reshapeDims[3] = {1, 1, 1};
    gcmHEADER_ARG("orginalTensor=%p", orginalTensor);

    for(i = 0; i < TENSOR_DIM_NUM(orginalTensor) - 1; i++)
        reshapeDims[0] *= TENSOR_SIZE_INDEX(orginalTensor,i);
    reshapeDims[2] = TENSOR_SIZE_INDEX(orginalTensor,TENSOR_DIM_NUM(orginalTensor) - 1);

    gcmFOOTER_NO();
    return vxoGraphOptimization_CreateShareTensor(orginalTensor, (vx_int32 *)reshapeDims, 3);
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_GetPermuteTensor(vx_graph graph, vx_tensor orginalTensor)
{
    vx_uint32 permuterTensorDims[3] = {
        TENSOR_SIZE_INDEX(orginalTensor,2),
        TENSOR_SIZE_INDEX(orginalTensor,1),
        TENSOR_SIZE_INDEX(orginalTensor,0)};

    vx_tensor_create_params_t p = vxoGraphOptimization_createParamsForTensor(3, permuterTensorDims, TENSOR_DATA_TYPE(orginalTensor),
        TENSOR_QUANT_TYPE(orginalTensor), TENSOR_POS(orginalTensor), TENSOR_TF_ZEROPOINT(orginalTensor), TENSOR_TF_SCALE(orginalTensor));

    gcmHEADER_ARG("graph=%p, orginalTensor=%p", graph, orginalTensor);

    gcmFOOTER_NO();
    return vxCreateVirtualTensor2(graph, &p, sizeof(p));
}

/*
 *    pad orgTensor to padTensor,
 *    if rowOrCol is 0, pad the row of orgTensor,
 *    if rowOrCol is 1, pad the col of orgTensor,
 *    offset means the pad size for left or top
 */
VX_INTERNAL_API vx_status vxoGraphOptimization_transformConvNxM_padTensor(vx_tensor *orgTensor, vx_tensor *padTensor, vx_uint32 *offset)
{
    vx_uint32   i = 0;
    vx_uint32   orgTensorSize = 0, padTensorSize = 0;
    vx_uint32   padValue = 0;
    vx_ptr      padp = (vx_ptr)&padValue;
    vx_ptr      tensorData       = VX_NULL;
    vx_ptr      padTensorData    = VX_NULL;
    vx_size     start[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_size     end[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    gcmHEADER_ARG("org=%p, padTensor=%p, offset=%p", orgTensor, padTensor, offset);

    vxoTensor_GetTensorSize(*orgTensor, &orgTensorSize);
    tensorData      = vxAllocateAndZeroMemory(orgTensorSize);

    vxoTensor_GetTensorSize(*padTensor, &padTensorSize);
    padTensorData   = vxAllocateAndZeroMemory(padTensorSize);

    /* firstly, set pad value to all of the new memory, and then copy the valid data into it*/

    /*get the pad value if the quantization is assymetric, or use the default value 0*/
    if(TENSOR_QUANT_TYPE(*orgTensor) == VX_QUANT_AFFINE_SCALE)
    {
        switch (TENSOR_DATA_TYPE(*orgTensor))
        {
        case VX_TYPE_UINT16:
        case VX_TYPE_INT16:
            *(vx_int16*)padp = (vx_int8)TENSOR_TF_ZEROPOINT(*orgTensor);
            break;
        case VX_TYPE_UINT8:
        case VX_TYPE_INT8:
            *(vx_int8*)padp = (vx_int8)TENSOR_TF_ZEROPOINT(*orgTensor);
            break;
        default:
            vxError("invalid tensor\n");
            break;
        }
    }

    if(TENSOR_DATA_SIZE(*orgTensor) == 1)
    {
        memset(padTensorData, *(vx_uint8 *)padp, padTensorSize);
    }
    else if(TENSOR_DATA_SIZE(*orgTensor) == 2)
    {
        for(i = 0; i < padTensorSize / TENSOR_DATA_SIZE(*padTensor); i++)
            *((vx_uint16 *)padTensorData + i) = *(vx_uint16 *)padp;
    }
    else
    {
        vxError("do not process the tensor whose's data size > 2\n");
        assert(0);
        return VX_FAILURE;
    }
    vxoGraphOptimization_copyConstData2tensor(padTensorData, padTensor, VX_WRITE_ONLY);

    /*read raw data from tensor, and pad them and put them into new tensor*/

    vxoGraphOptimization_copyConstData2tensor(tensorData, orgTensor, VX_READ_ONLY);

    /*use the tensorview to copy data into the right position*/
    memset(start, 0, sizeof(start));
    for(i = 0; i < TENSOR_DIM_NUM(*orgTensor); i++){
        start[i] = offset[i];
        end[i] = TENSOR_SIZE_INDEX(*orgTensor, i) + offset[i];
    }


    {
        vx_size stride[6];
        for(i = 0; i < TENSOR_DIM_NUM(*orgTensor); i++){
            stride[i] = TENSOR_STRIDE_INDEX(*orgTensor, i);
        }
    CHECK_STATUS(vxCopyTensorPatch(*padTensor,TENSOR_DIM_NUM(*padTensor), start, end,
        stride, tensorData, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    }

    if(tensorData)      vxFree(tensorData);
    if(padTensorData)   vxFree(padTensorData);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_WAR_convert1x1x1weight(vx_tensor weight)
{
    vx_tensor bigWeight = weight;
    vx_uint32 dimsCount = TENSOR_DIM_NUM(weight);
    vx_uint32 *orgDims = TENSOR_SIZES(weight);
    vx_uint32 *dims = VX_NULL;


    gcmHEADER_ARG("weight=%p", weight);

    if(TENSOR_DIM_NUM(weight) >= 3 && (
        orgDims[0] == 1 && orgDims[1]== 1 && orgDims[2]== 1))
    {
        dims = (vx_uint32 *)vxAllocateAndZeroMemory(dimsCount * sizeof(vx_uint32));
        CHECK_NULL(dims);

        vxMemCopy(dims, orgDims, dimsCount * sizeof(vx_uint32));
        dims[0] = 2;
        dims[1] = 2;
        {
            vx_tensor_create_params_t param  = { dimsCount, dims, TENSOR_DATA_TYPE(weight), TENSOR_QUANT_TYPE(weight), { {0}}};
            if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE)
            {
                param.quant_data.affine.scale     = TENSOR_TF_SCALE(weight);
                param.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(weight);
            }
            else{
                param.quant_data.dfp.fixed_point_pos = TENSOR_POS(weight);
            }

            bigWeight = vxCreateTensor2(vxGetContext((vx_reference)weight), &param, sizeof(vx_tensor_create_params_t));
            vxoTensor_AllocateMemory(bigWeight);
        }

        if(dims)
            vxFree(dims);
        {
            vx_uint32 offsetView[VX_CONTEXT_TENSOR_MAX_DIMENSION] ;
            memset(offsetView, 0, sizeof(offsetView));
            vxoGraphOptimization_transformConvNxM_padTensor(&weight, &bigWeight, offsetView);
        }
    }
    gcmFOOTER_ARG("bigWeight=%p", bigWeight);
    return bigWeight;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_GetReshapeWeightTensor(vx_tensor orginalWeightTensor)
{
    vx_int32 reshapeDims[4] = {1,1,1,1};
    vx_tensor shareTensor = orginalWeightTensor;
    if(TENSOR_DIM_NUM(orginalWeightTensor) == 2)
    {
        reshapeDims[2] = TENSOR_SIZE_INDEX(orginalWeightTensor, 0);
        reshapeDims[3] = TENSOR_SIZE_INDEX(orginalWeightTensor, 1);
        shareTensor = vxoGraphOptimization_CreateShareTensor(orginalWeightTensor, reshapeDims, 4);
        if(reshapeDims[2] == 1)
        {
            vx_tensor tmpTensor = vxoGraphOptimization_WAR_convert1x1x1weight(shareTensor);
            vxReleaseTensor(&shareTensor);

            shareTensor = tmpTensor;
        }
    }

    return shareTensor;
}

/* get the orginal weight and bias from conv or fc-type node*/
VX_PRIVATE_API void vxoGraphOptimization_getOrignalWB(vx_node node, vx_tensor *orgWeight, vx_tensor *orgBias)
{
    vx_enum opType = node->kernel->enumeration;

    vxmASSERT(node != NULL && orgBias != NULL && orgWeight != NULL);

    switch (opType)
    {
    case VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER:
    case VX_KERNEL_NN_CONVOLUTION_RELU_LAYER:
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:
    case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:
        {
            vx_weights_biases_parameter weight_biases = (vx_weights_biases_parameter)node->paramTable[1];
            *orgWeight  = WB_WEIGHT_TENSOR(weight_biases);
            *orgBias    = WB_BIAS_TENSOR(weight_biases);
            break;
        }
    case VX_KERNEL_FULLY_CONNECTED_LAYER:
    case VX_NN_FULLYCONNECTED_LAYER:
    case VX_KERNEL_CONVOLUTION_LAYER:
        {
            *orgWeight  = (vx_tensor)node->paramTable[1];
            *orgBias    = (vx_tensor)node->paramTable[2];
            break;
        }
    default:
        {
            *orgWeight  = NULL;
            *orgBias    = NULL;
            vxError("unkown CONV or FC type Node");
        }
        break;
    }
}

VX_INTERNAL_API vx_status vxoGraphOptimization_ConvertBatchFCtoConv(vx_graph graph)
{
    /********************************************************************************
    * 1. convert the reshape of inpute tensor to 3dims (whcn: kin x 1 x batch)
    * 2. permute the input tesnor, to (whcn: batch x 1 x kin)
    * 3. convlution: weight(1 x 1 x kin x kout)
    * 4. permute output tensor, from(whcn: batch x 1 x kout) to (whcn: kout x 1 x batch)
    ********************************************************************************/

    /*TODO:
    for batch fc 2 conv feature, the input of conv can be (m x n) instead of (batch x 1)
        which is more generous, and then the condition that get fc op should be modified
    */
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    vx_uint32 permuteIdx[3] = {2, 1, 0};
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        vx_tensor inputTensor = (vx_tensor)node->paramTable[0];
        vx_tensor outputTensor = (vx_tensor)node->paramTable[node->numParameters - 1];

        vx_tensor shareInputTensor = VX_NULL, shareOutputTensor = VX_NULL;

        vx_tensor convInputTensor = VX_NULL;
        vx_tensor convOutputTensor = VX_NULL;

        vx_uint32   pad;
        vx_uint8    accumulator_bits;
        vx_uint32    overflow_policy;
        vx_uint32    rounding_policy;
        vx_uint32    down_scale_size_rounding;

        if(!vxoNode_IsFCNode(node))
            continue;

        if(!vxoGraphOptimization_nnHalSupport((vx_tensor)node->paramTable[0]))
        {
            gcmFOOTER_ARG("%d", VX_SUCCESS);
            return VX_SUCCESS;
        }

        if(TENSOR_RANK(inputTensor) == VX_TENSOR_RANK_WHCN &&
             (2 == TENSOR_DIM_NUM(inputTensor)? TENSOR_SIZE_INDEX(inputTensor, 1) : TENSOR_SIZE_INDEX(inputTensor, 3) ) <= 1)
            /*GET_TENSOR_BATCH(inputTensor) <= 1)*/
            continue;

        shareInputTensor = vxoGraphOptimization_GetShareTensor(inputTensor);
        shareOutputTensor = vxoGraphOptimization_GetShareTensor(outputTensor);
        CHECK_NULL(shareInputTensor);
        CHECK_NULL(shareOutputTensor);

        convInputTensor = vxoGraphOptimization_GetPermuteTensor(graph, shareInputTensor);
        convOutputTensor = vxoGraphOptimization_GetPermuteTensor(graph, shareOutputTensor);
        CHECK_NULL(convInputTensor);
        CHECK_NULL(convOutputTensor);

        {
            vx_node tmp = vxTensorPermuteNode(graph, shareInputTensor, convInputTensor, permuteIdx,3);
            CHECK_NULL(tmp);
            vxReleaseNode(&tmp);
        }

        /*if(vxoGraphOptimization_nnHalSupport(convInputTensor))*/

        vxoNode_getInfoFromFCNode(node, &pad, &accumulator_bits, &rounding_policy, &overflow_policy, &down_scale_size_rounding);

        {

            vx_uint32 pad_const = 0;
            vx_scalar padConst = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &pad_const);

            vx_tensor weight, orgBias, orgWeight;
            vx_nn_convolution_relu_pooling_params_ext2_t params = {
                    {
                            { 0, 0,
                              pad, pad, pad, pad, accumulator_bits, overflow_policy,
                              rounding_policy, down_scale_size_rounding,
                              node->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER ? SCALAR_VALUE(node->paramTable[7], b): vx_false_e,
                              0, 0, 0, VX_PAD_CONSTANT, padConst},
                            1, 1
                    },
                    0,
                    VX_TENSOR_RANK_WHCN,
                    TENSOR_DATA_TYPE((vx_tensor)node->paramTable[node->numParameters-1])
            };

            vxoGraphOptimization_getOrignalWB(node, &orgWeight, &orgBias);

            weight = vxoGraphOptimization_GetReshapeWeightTensor(orgWeight);
            TENSOR_DATA_LIFETIME(weight) = VX_TENSOR_LIFE_TIME_STATIC;

            {
                vx_weights_biases_parameter weights_biases = vxoGraphOptimization_CreateWBParameter(
                                    VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER,
                                    (vx_nn_convolution_relu_pooling_params_t *)&params,
                                    sizeof(params),
                                    convInputTensor, convOutputTensor, convOutputTensor, weight, orgBias, VX_NULL, VX_NULL);

                vx_node newNode = vxConvolutionReluPoolingLayer2(graph, convInputTensor, weights_biases,
                    (vx_nn_convolution_relu_pooling_params_t *)&params, sizeof(params),convOutputTensor);

                vxReleaseWeightsBiasesParameter(&weights_biases);
                vxReleaseNode(&newNode);

                if(weight != orgWeight)
                    vxReleaseTensor(&weight);
            }

            vxReleaseScalar(&padConst);

        }

        {
            vx_node tmp = vxTensorPermuteNode(graph, convOutputTensor, shareOutputTensor, permuteIdx,3);
            CHECK_NULL(tmp);
            vxReleaseNode(&tmp);
        }

        node->merged = vx_true_e;
        vxReleaseTensor(&shareInputTensor);
        vxReleaseTensor(&shareOutputTensor);
        vxReleaseTensor(&convInputTensor);
        vxReleaseTensor(&convOutputTensor);
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_tensor vxoGraphOptimization_ConvertAvgPool2Conv_createWeight(vx_tensor input, vx_uint32 weight_dims[4])
{
    vx_uint32   i = 0;
    vx_ptr      weight_memory = VX_NULL;
    vx_tensor   weight = VX_NULL;
    vx_uint32   weight_size = weight_dims[0] * weight_dims[1] * weight_dims[2];
    vx_int16    quantizedData = 0;
    vx_uint32   square = weight_dims[0] * weight_dims[1];
    vx_float32  fill_data = 1.0f / square;

    vx_context context = vxGetContext((vx_reference)input);

    vx_tensor_create_params_t parm;
    gcmHEADER_ARG("input=%p, weight_dims=%p", input, weight_dims);
    INITIALIZE_STRUCT(parm);
    parm.data_format = TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT32 ? VX_TYPE_BFLOAT16: TENSOR_DATA_TYPE(input);
    parm.num_of_dims = 4;
    parm.sizes = weight_dims;
    parm.quant_format = TENSOR_QUANT_TYPE(input);

    if((square & (square - 1) ) == 0 && VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(input))
    {
        parm.quant_data.dfp.fixed_point_pos = -1 * (vx_int8)gcoMATH_Log2(fill_data);
    }
    else
    {
        vxoGraphOptimization_computeQuantAttribute(TENSOR_QUANT_TYPE(input), TENSOR_DATA_TYPE(input),fill_data, 0,
            &parm.quant_data.dfp.fixed_point_pos, &parm.quant_data.affine.zeroPoint, &parm.quant_data.affine.scale);
    }

    weight = vxCreateTensor2(context, &parm, sizeof(parm));

    if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE)
    {
        quantizedData = (vx_int16) roundRTNE(fill_data / TENSOR_TF_SCALE(weight) + TENSOR_TF_ZEROPOINT(weight));
    }
    else
    {
        if(TENSOR_DATA_TYPE(weight) == VX_TYPE_FLOAT16)
            quantizedData = Fp32toFp16(fill_data);
        else if(TENSOR_DATA_TYPE(weight) == VX_TYPE_BFLOAT16)
            quantizedData = Fp32toBF16(fill_data);
        else
        {
            vx_int8 fl= TENSOR_POS(weight);

            if(fl > 0 )
            {
                quantizedData = (vx_int16)roundRTNE((vx_float64)fill_data * (1 << fl));
            }
            else
            {
                quantizedData = (vx_int16)roundRTNE((vx_float64)fill_data / (1 << (-fl)));
            }
        }
    }

    weight_memory = vxAllocateAndZeroMemory(weight_size * TENSOR_DATA_SIZE(input));
    if(TENSOR_DATA_SIZE(weight) == 1)
    {
        memset(weight_memory, (vx_int8)quantizedData, weight_size);
    }
    else if(TENSOR_DATA_SIZE(weight) == 2)
    {
        vx_int16 *ptr = (vx_int16 *)weight_memory;
        for(i = 0; i < weight_size; i++)
            ptr[i] = quantizedData;
    }

    vxoGraphOptimization_copyConstData2tensor(weight_memory, &weight, VX_WRITE_ONLY);

    vxFree(weight_memory);

    gcmFOOTER_ARG("%d", weight);
    return weight;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_ConvertAvgPool2Conv(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    vx_uint32 maxKernelSizeX = 0;
    vx_uint32 maxKernelSizeY = 0;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    vxoGraphOptimization_getMaxKernelSize(graph->base.context, &maxKernelSizeX, &maxKernelSizeY, VX_NULL);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_AVG_POOL)
        {
            vx_tensor weight = VX_NULL;
            vx_tensor input = (vx_tensor)node->paramTable[0];
            vx_tensor output = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);

            vx_uint32 kernel_x = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
            vx_uint32 kernel_y = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
            vx_uint32 pads[] = {
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_L_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_R_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_T_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_B_INDEX],u32)
            };

            vx_uint32 weight_dims[] = {kernel_x, kernel_y, TENSOR_SIZE_INDEX(input, 2),1};

            vx_int32 stride_x = 1, stride_y = 1;
            if(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX] != NULL && node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX] != NULL)
            {
                stride_x = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX], u32);
                stride_y = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX], u32);
            }
            else
            {
                stride_x = (vx_int32)roundRTNE((vx_float32)(TENSOR_SIZE_INDEX(input, 0) + pads[0] + pads[1] - kernel_x)/(TENSOR_SIZE_INDEX(output, 0) == 1? 1: TENSOR_SIZE_INDEX(output, 0) -1));
                stride_y = (vx_int32)roundRTNE((vx_float32)(TENSOR_SIZE_INDEX(input, 1) + pads[2] + pads[3] - kernel_y)/(TENSOR_SIZE_INDEX(output, 1) == 1? 1: TENSOR_SIZE_INDEX(output, 1) -1));
                stride_x = stride_x == 0 ? 1: stride_x;
                stride_y = stride_y == 0 ? 1: stride_y;
            }

            if(!vxoGraphOptimization_nnHalSupport(input))
                continue;

            /*V8 support depwiseConv hardware feature
            ** if input is fp32 tensor, weight could be bfloat16 for some hw*/
            if(vxoGraphOptimization_dwConvHalSupport(input) ||
                (TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT32 && gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcFEATURE_BIT_DEPTHWISE_16BIT_FORMAT)))
            {
                if(kernel_x > maxKernelSizeX || kernel_y>maxKernelSizeY)
                    continue;
                if(stride_x > 2 || stride_y > 2 || stride_x != stride_y)
                    continue;
            }
            else
            {
                if(kernel_y != kernel_x &&
                    ((kernel_x > 1 && kernel_y != 1) || (kernel_y > 1 && kernel_x != 1) )
                    )
                    continue;

                if(TENSOR_SIZE_INDEX(input, 2) * TENSOR_SIZE_INDEX(input, 2) > 100000)
                    continue;

                if(weight_dims[0]>3 || weight_dims[1] > 3)
                    continue;

                if((weight_dims[0] * weight_dims[1] * weight_dims[2] == 1))
                    continue;
            }
            weight = vxoGraphOptimization_ConvertAvgPool2Conv_createWeight(input, weight_dims);

            {
                vx_enum life_time = VX_TENSOR_LIFE_TIME_STATIC;
                vxSetTensorAttribute(weight,VX_TENSOR_LIFETIME, &life_time, sizeof(vx_enum));
            }
            {
                vx_int32 depth_multiplier = 1;
                vx_uint32 c = 0;
                vx_scalar padscalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &c);
                vx_nn_convolution_params_ext2_t params = {
                {
                    { (vx_size)pads[0], (vx_size)pads[2], VX_CONVERT_POLICY_SATURATE, VX_ROUND_POLICY_TO_ZERO, VX_NN_DS_SIZE_ROUNDING_FLOOR, 0, 0},
                    (vx_size)pads[1], (vx_size)pads[2], VX_PAD_CONSTANT, 0
                    },
                    (vx_uint32)stride_x, (vx_uint32)stride_y, depth_multiplier
                };

                vx_node convNode = vxConvolutionLayer(graph,
                    input,
                    weight,
                    VX_NULL,
                    (const vx_nn_convolution_params_t *)&params,
                    sizeof(vx_nn_convolution_params_ext2_t),
                    (vx_tensor)output);

                vxReleaseNode(&convNode);
                vxReleaseScalar(&padscalar);
            }

            vxReleaseTensor(&weight);
            node->merged = vx_true_e;
        }/*if(vxoGraphOptimization_getKernelType(node) == OP_AVG_POOL)*/
    }/*for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)*/

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int8(vx_tensor *weight, vx_uint32 coreNum, vx_int8 quantedData[2])
{
    vx_uint32 i = 0;
    vx_int8 *weightData = (vx_int8 *)vxAllocateAndZeroMemory(2*coreNum * coreNum);
    for(i = 0; i < coreNum; i++)
    {
        vx_int32 offset = i * 2 * coreNum;
        weightData[offset + i] = quantedData[0];
        weightData[offset + i + coreNum] = quantedData[1];
    }

    vxoGraphOptimization_copyConstData2tensor((void*)weightData, weight,VX_WRITE_ONLY);
    vxFree(weightData);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int16(vx_tensor *weight, vx_uint32 coreNum, vx_int16 quantedData[2])
{
    vx_uint32 i = 0;
    vx_int16 *weightData = (vx_int16 *)vxAllocateAndZeroMemory(2*coreNum * coreNum*sizeof(vx_int16));

    gcmHEADER_ARG("weight=%p, coreNum=0x%x, quantedData=%p", weight, coreNum, quantedData);

    for(i = 0; i < coreNum; i++)
    {
        vx_int32 offset = i * 2 * coreNum;
        weightData[offset + i] = quantedData[0];
        weightData[offset + i + coreNum] = quantedData[1];
    }

    vxoGraphOptimization_copyConstData2tensor((void*)weightData, weight,VX_WRITE_ONLY);
    vxFree(weightData);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_TensorAdd2Conv_createWeight_fp16(vx_tensor tensorIn[2], vx_uint32 coreNum, vx_tensor *weight, vx_int32 factor)
{
    vx_context context = vxGetContext((vx_reference)tensorIn[0]);
    vx_size wDims[4] = {1,1,2 * coreNum,coreNum};
    vx_int16 weightData[2] = {0, 0};

    gcmHEADER_ARG("tensorIn=%p, coreNum=0x%x, weight=%p, factor=0x%x", tensorIn, coreNum, weight, factor);

    *weight = vxCreateTensor(context, 4, wDims, TENSOR_DATA_TYPE(tensorIn[0]), 0);

    if(VX_TYPE_BFLOAT16 == TENSOR_DATA_TYPE(tensorIn[0]))
    {
        weightData[0] = Fp32toBF16(1.f);
        weightData[1] = Fp32toBF16(1.f* factor);
    }
    else if(VX_TYPE_FLOAT16 == TENSOR_DATA_TYPE(tensorIn[0]))
    {
        weightData[0] = Fp32toFp16(1.f);
        weightData[1] = Fp32toFp16(1.f* factor);
    }
    vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int16(weight, coreNum, weightData);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_createWeight(vx_context context, vx_tensor *weight, vx_uint32 coreNum, vx_enum dataTYpe,
                                                                           vx_enum quantType,float scale, vx_int32 zp, vx_int8 dfpos)
{
    vx_uint32 wDims[4] = {1,1,2 * coreNum,coreNum};
    vx_tensor_create_params_t weight_p;
    gcmHEADER_ARG("context=%p, weight=%p, coreNum=0x%x, dataTYpe=0x%x, quantType=0x%x, scale=%f, zp=0x%x, dfpos=0x%x", context, weight, coreNum, dataTYpe, quantType, scale, zp, dfpos);

    weight_p = vxoGraphOptimization_createParamsForTensor(4, wDims, dataTYpe, quantType, dfpos, zp, scale);

    *weight = vxCreateTensor2(context,&weight_p, sizeof(weight_p));
    CHECK_NULL(*weight);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_asymmetic(vx_tensor tensorIn[2], vx_uint32 coreNum,
                                                                               vx_tensor *weight, vx_int32 factor)
{
    vx_int32 zp;
    float scale;
    float fWeight[2] = {1, TENSOR_TF_SCALE(tensorIn[1]) / TENSOR_TF_SCALE(tensorIn[0])};

    vxoGraphOptimization_getAsymQuantizeAttribute(TENSOR_DATA_TYPE(*weight), gcmMAX(fWeight[0], fWeight[1]), gcmMIN(fWeight[0], fWeight[1]), &scale, &zp);

    if(TENSOR_DATA_TYPE(*weight) == VX_TYPE_UINT8 || TENSOR_DATA_TYPE(*weight) == VX_TYPE_INT8)
    {
        vx_int8 quantedData[2] = {
            (vx_int8) roundRTNE(fWeight[0] / scale+ zp),
            (vx_int8) (roundRTNE(fWeight[1]/ scale+ zp) * factor)
        };

        vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int8(weight, coreNum, quantedData);
    }
    else if(TENSOR_DATA_TYPE(*weight) == VX_TYPE_UINT16)
    {
        vx_int16 quantedData[2] = {
            (vx_int16) roundRTNE(fWeight[0] / scale+ zp),
            (vx_int16) (roundRTNE(fWeight[1]/ scale+ zp) * factor)
        };

        vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int16(weight, coreNum, quantedData);
    }
    else
    {
        vxError("unknwon data type for creating weight in func %s\n", __FUNCTION__);
        vxmASSERT(0);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_createBias_asymmetic(vx_tensor tensorIn[2], vx_uint32 coreNum,
                                                                               vx_tensor *weight, vx_tensor *bias)
{
    vx_context context = vxGetContext((vx_reference)tensorIn[0]);
    vx_uint32 bDims[1] = {coreNum};
    float biasScale = TENSOR_TF_SCALE(*weight) * TENSOR_TF_SCALE(tensorIn[0]);

    if(!(*bias))
    {
        vx_tensor_create_params_t bias_p = vxoGraphOptimization_createParamsForTensor(1, bDims, VX_TYPE_INT32, VX_QUANT_AFFINE_SCALE,
            0, 0, biasScale);
        *bias = vxCreateTensor2(context, &bias_p, sizeof(bias_p));
    }
    {
        vx_uint32 i = 0;
        float biasValue         = (TENSOR_TF_ZEROPOINT(tensorIn[0]) - TENSOR_TF_ZEROPOINT(tensorIn[1])) * TENSOR_TF_SCALE(tensorIn[1]);
        vx_int32 quantedDias    = (vx_int32)roundRTNE(biasValue / biasScale);
        vx_int32 *biasData      = (vx_int32 *)vxAllocateAndZeroMemory(coreNum* sizeof(vx_int32));

        for(i = 0; i < coreNum; i++)
            biasData[i] = quantedDias;

        vxoGraphOptimization_copyConstData2tensor(biasData, bias,VX_WRITE_ONLY);
        vxFree(biasData);
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_dfp(vx_tensor *weight, vx_int8 fl_delta, vx_uint32 coreNum,
                                                                               vx_int32 factor)
{
    if(TENSOR_DATA_TYPE(*weight) == VX_TYPE_INT8)
    {
        vx_int8 weightValue[2] = {0,0};
        vx_int8 fl = TENSOR_POS(*weight);

        weightValue[0] = (vx_int8)pow(2.0f, fl);
        weightValue[1] = (vx_int8)(pow(2.0f, fl - fl_delta ) * factor);

        vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int8(weight, coreNum, weightValue);
    }
    else if(TENSOR_DATA_TYPE(*weight) == VX_TYPE_INT16)
    {
        vx_int16 weightValue[2] = {0,0};
        vx_int16 fl = TENSOR_POS(*weight);

        weightValue[0] = (vx_int16)pow(2.0f, fl);
        weightValue[1] = (vx_int16)(pow(2.0f, fl - fl_delta ) * factor);

        vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_int16(weight, coreNum, weightValue);
    }
    else
    {
        vxError("unknwon data type for creating weight in tensorAdd\n");
        vxmASSERT(0);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_createQuantizedWeightAndBias(vx_tensor *weight, vx_tensor *bias,
                                                                                            vx_tensor tensorIn[2],
                                                                                            vx_uint32 coreNum)
{
    float scale = 1.0f;
    vx_int32 zp = 0;
    vx_int8 fl_delta = 0;

    vx_context context = vxGetContext((vx_reference)tensorIn[0]);
    gcmHEADER_ARG("weight=%p, bias=%p, tensorIn=%p, coreNum=0x%x", weight, bias, tensorIn, coreNum);

    if(TENSOR_QUANT_TYPE(tensorIn[0]) == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        fl_delta = gcmMAX(TENSOR_POS(tensorIn[1]) - TENSOR_POS(tensorIn[0]), 0);
    }
    else
    {
        float fWeight[2] = {1, TENSOR_TF_SCALE(tensorIn[1]) / TENSOR_TF_SCALE(tensorIn[0])};
        vxoGraphOptimization_getAsymQuantizeAttribute(TENSOR_DATA_TYPE(tensorIn[0]), gcmMAX(fWeight[0], fWeight[1]), gcmMIN(fWeight[0], fWeight[1]), &scale, &zp);
    }

    vxoGraphOptimization_TensorAdd2Conv_createWeight(context, weight, coreNum,
            TENSOR_DATA_TYPE(tensorIn[0]), TENSOR_QUANT_TYPE(tensorIn[0]), scale, zp, fl_delta);

    if(TENSOR_QUANT_TYPE(tensorIn[0]) == VX_QUANT_AFFINE_SCALE)
    {
        vx_uint32 bDims[1] = {coreNum};
        float biasScale = TENSOR_TF_SCALE(*weight) * TENSOR_TF_SCALE(tensorIn[0]);

        vx_tensor_create_params_t bias_p = vxoGraphOptimization_createParamsForTensor(1, bDims, VX_TYPE_INT32, VX_QUANT_AFFINE_SCALE, 0, 0, biasScale);

        *bias = vxCreateTensor2(context, &bias_p, sizeof(bias_p));
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2WeightAndBias_dfp(vx_tensor *weight, vx_tensor *bias, vx_tensor tensorIn[2],
                                                                                            vx_uint32 coreNum, vx_int32 factor)
{
    vx_int8 fl_delta = TENSOR_POS(tensorIn[1]) - TENSOR_POS(tensorIn[0]);
    return vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_dfp(weight, fl_delta, coreNum, factor);
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_copyData2WeightAndBias_asymmetic(vx_tensor *weight, vx_tensor *bias, vx_tensor tensorIn[2],
                                                                                            vx_uint32 coreNum, vx_int32 factor)
{
    vx_status status;

    status = vxoGraphOptimization_TensorAdd2Conv_copyData2Weight_asymmetic(tensorIn, coreNum, weight, factor);
    status |= vxoGraphOptimization_TensorAdd2Conv_createBias_asymmetic(tensorIn, coreNum, weight, bias);

    return status;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_computeCoreNum(vx_uint32 size, vx_uint32 *core)
{
    vx_uint32 i = 0, thresold = 1024 * 1024;
    vx_uint32 num = 1;
    gcmHEADER_ARG("size=0x%x, core=%p", size, core);
    if(size > thresold)
    {
        for(i = 1; i < 5; i++)
        {
            num = (1<<i);
            if((size & (num - 1)) == 0 && size / num < thresold)
                break;
        }

        if((size & (num - 1)) != 0 || size / num >= thresold)
        {
            gcmFOOTER_ARG("%d", -1);
            return -1;
        }
    }

    *core = num;
    gcmFOOTER_ARG("%d", 0);
    return 0;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_TensorAdd2Conv_computeConvDims(vx_uint32 size, vx_uint32 *convW)
{
    vx_uint32 triggerThresold   = 8192;
    vx_uint32 alginedValue      = 64;
    vx_uint32 initW             = alginedValue;
    gcmHEADER_ARG("size=0x%x, convW=%p", size, convW);
    while(1)
    {
        initW = alginedValue;
        while (1){
            if(size % (initW + alginedValue) != 0) break;
            initW += alginedValue;
            if(size / initW < triggerThresold) break;
        }

        if(size % initW == 0 && size / initW < triggerThresold)
            break;

        alginedValue >>= 1;
        if(alginedValue < 16)
            break;
    }

    if(size % initW != 0 || size / initW > triggerThresold)
    {
        initW = 63;
        while(initW)
        {
            if(size % initW == 0 && size / initW < triggerThresold)
                break;
            initW --;
        }

        if(initW != 0)
        {
            /*if(initW % 16 < 10)
                initW = size / initW;*/
             *convW = initW;
             gcmFOOTER_ARG("%d", VX_SUCCESS);
             return VX_SUCCESS;
        }else{
            initW = 65;
            while(1){
                if(size % initW == 0 && size / initW < triggerThresold) break;
                initW ++;
            }
            if(initW > triggerThresold)
            {
                gcmFOOTER_ARG("%d", -1);
                return -1;
            }
        }
    }

    *convW = initW;
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_TensorAdd2Conv(vx_graph graph)
{
    /*
    1. creat the big tensor (w, h, 2z), which is splited to tensor A and B.
    2. reshape it to (wxh, z, 2)
    3. create weight, (1,1,2,1). TODO:for more nncore, (1,1,2*nncore, nncore)
    */
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_ADD_SUB)
        {
            vx_tensor tensorIn[2]   = {(vx_tensor)node->paramTable[0], (vx_tensor)node->paramTable[1]};
            vx_tensor tensorSub[2]  = {VX_NULL, VX_NULL};
            vx_tensor output        = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_tensor bigTensor     = VX_NULL;
            vx_tensor convInTensor  = VX_NULL;
            vx_tensor convOutTensor = VX_NULL;
            vx_tensor weight        = VX_NULL;
            vx_tensor bias          = VX_NULL;
            vx_uint32 size          = 1;
            vx_uint32 core          = 1;
            vx_uint32 convW         = 64;
            vx_int32  convDims[3]   = {1,1,2};
            vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {1};
            vx_int32  factor        = 1;

            if(!vxoGraphOptimization_nnHalSupport((vx_tensor)node->paramTable[0]))
            {
                gcmFOOTER_ARG("%d", VX_SUCCESS);
                return VX_SUCCESS;
            }

            /*can not do it for viewed tensor*/
            if(vxoTensor_IsViewed(tensorIn[0]) || vxoTensor_IsViewed(tensorIn[1]))
                continue;

            /*do not process the head node*/
            if(node->numParents != 2 || tensorIn[0]->isViewed || tensorIn[1]->isViewed )
                continue;

            if(TENSOR_DATA_TYPE(tensorIn[0]) != TENSOR_DATA_TYPE(tensorIn[1]) ||
                TENSOR_DATA_TYPE(tensorIn[0]) != TENSOR_DATA_TYPE(output) )
                continue;

            /*normal tensor could not be replaced, because app maybe want its result*/
            if((!vxoTensor_IsVirtualTensor(tensorIn[0])) || (!vxoTensor_IsVirtualTensor(tensorIn[1])) )
                continue;

            if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NEGATIVE_POST_SHIFT_FIX) &&
                VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(tensorIn[0]))
            {
                    vx_int32 weight_fl = gcmMAX(TENSOR_POS(tensorIn[1]) - TENSOR_POS(tensorIn[0]), 0);
                    if(TENSOR_POS(tensorIn[0]) + weight_fl < TENSOR_POS(output) )
                        continue;
            }

            /* the formula is required:
                  abs(fl1 - fl2) < data's bit width

               such as:
                    input1's fl = 30,
                    input2's fl = 7,

                   if the conv's input's fl is 30, the weight should be [1, 2^-23], but 2^-23 is too small.
                   if 7, weight should be [2^23, 1], 2^23 is overflow for int8 or int16.
            */
            if(VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(tensorIn[0]) )
            {
                vx_uint8 delta = gcmABS(TENSOR_POS(tensorIn[0]) - TENSOR_POS(tensorIn[1]) );

                if((delta > 15 && TENSOR_DATA_TYPE(tensorIn[0]) == VX_TYPE_INT16) ||
                    (delta > 7 && TENSOR_DATA_TYPE(tensorIn[0]) == VX_TYPE_INT8)
                    )
                    continue;
            }

            if(node->kernel->enumeration == VX_KERNEL_TENSOR_SUBTRACT)
                factor = -1;
            {
                vx_uint32 i = 0;
                for(i = 0; i<TENSOR_DIM_NUM(tensorIn[0]); i++)
                {
                    dims[i] = TENSOR_SIZE_INDEX(tensorIn[0],i);
                    size *= dims[i];
                }
                dims[TENSOR_DIM_NUM(tensorIn[0])-1] *= 2;

                if (-1 == vxoGraphOptimization_TensorAdd2Conv_computeCoreNum(size, &core))
                    continue;

                size /= core;
                if(-1 == vxoGraphOptimization_TensorAdd2Conv_computeConvDims(size, &convW))
                    continue;
            }
            /*concat input tensor to tensorview and replace the older tensor with tensorview */
            {
                vx_tensor_create_params_t tensorParam;
                vx_uint32 i = 0;

                INITIALIZE_STRUCT(tensorParam);
                tensorParam.sizes   = dims;
                tensorParam.data_format                     = TENSOR_DATA_TYPE(tensorIn[0]);
                tensorParam.num_of_dims                     = TENSOR_DIM_NUM(tensorIn[0]);
                tensorParam.quant_format                    = TENSOR_QUANT_TYPE(tensorIn[0]);
                if(TENSOR_QUANT_TYPE(tensorIn[0]) == VX_QUANT_AFFINE_SCALE)
                {
                    tensorParam.quant_data.affine.scale         = TENSOR_TF_SCALE(tensorIn[0]);
                    tensorParam.quant_data.affine.zeroPoint     = TENSOR_TF_ZEROPOINT(tensorIn[0]);
                }
                else
                {
                    tensorParam.quant_data.dfp.fixed_point_pos  = TENSOR_POS(tensorIn[0]);
                }

                bigTensor = vxCreateVirtualTensor2(node->graph, &tensorParam, sizeof(tensorParam));
                CHECK_NULL(bigTensor);
                vxoGraphOptimization_ConcatTensors(vxGetContext((vx_reference)graph), tensorIn, 2, TENSOR_DIM_NUM(tensorIn[0])-1, tensorSub, bigTensor);
                for(i = 0; i <2; i++)
                {
                    TENSOR_QUANT_TYPE(tensorSub[i]) = TENSOR_QUANT_TYPE(tensorIn[i]);

                    TENSOR_POS(tensorSub[i]) = TENSOR_POS(tensorIn[i]);
                    TENSOR_TF_SCALE(tensorSub[i]) = TENSOR_TF_SCALE(tensorIn[i]);
                    TENSOR_TF_ZEROPOINT(tensorSub[i]) = TENSOR_TF_ZEROPOINT(tensorIn[i]);
                }
                vxoGraphOptimization_updateTensorInGraph(node,tensorIn, tensorSub, 2);
            }
            {
                /* get the convolution input and output*/

                convDims[0] = convW;
                convDims[1] = size / convW;
                convDims[2] = 2 * core;
                convInTensor =   vxoGraphOptimization_CreateShareTensor(bigTensor, convDims, 3);
                CHECK_NULL(convInTensor);
                vxReleaseTensor(&bigTensor);

                convDims[2] /= 2;
                convOutTensor = vxoGraphOptimization_CreateShareTensor(output, convDims, 3);
                CHECK_NULL(convOutTensor);
            }

            if(VX_TYPE_FLOAT16 == TENSOR_DATA_TYPE(tensorSub[0]) ||
                VX_TYPE_BFLOAT16 == TENSOR_DATA_TYPE(tensorSub[0]))
            {
                vxoGraphOptimization_TensorAdd2Conv_createWeight_fp16(tensorSub, core, &weight, factor);
            }
            else
            {
                vxoGraphOptimization_TensorAdd2Conv_createQuantizedWeightAndBias(&weight, &bias, tensorSub, core);
                switch (TENSOR_QUANT_TYPE(tensorSub[0]))
                {
                case VX_QUANT_AFFINE_SCALE:
                    vxoGraphOptimization_TensorAdd2Conv_copyData2WeightAndBias_asymmetic(&weight, &bias, tensorSub, core, factor);
                    break;
                case VX_QUANT_DYNAMIC_FIXED_POINT:
                    vxoGraphOptimization_TensorAdd2Conv_copyData2WeightAndBias_dfp(&weight, &bias, tensorSub, core, factor);
                    break;
                default:
                     vxWarning("unknwon quantization type when to create weight in tensorAdd\n");
                    break;
                }
            }

            if(bias) TENSOR_DATA_LIFETIME(bias) = VX_TENSOR_LIFE_TIME_STATIC;

            TENSOR_DATA_LIFETIME(weight) = VX_TENSOR_LIFE_TIME_STATIC;

            {
                vx_uint32 c = 0;
                vx_scalar padscalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &c);
                vx_nn_convolution_params_ext2_t params = {
                    {
                        {
                            0, 0, VX_CONVERT_POLICY_SATURATE, VX_ROUND_POLICY_TO_ZERO, VX_NN_DS_SIZE_ROUNDING_FLOOR, 0, 0
                        },
                        0, 0, VX_PAD_CONSTANT, padscalar
                    }, 1, 1, 0
                };

                vx_node convNode  =  vxConvolutionLayer(node->graph, convInTensor, weight, bias,
                    (vx_nn_convolution_params_t*)&params, sizeof(params), convOutTensor);

                CHECK_NULL(convNode );

                node->replacedBy = convNode;

                vxReleaseScalar(&padscalar);
                vxReleaseTensor(&weight);
                vxReleaseTensor(&bias);
                vxReleaseNode(&convNode);

                if(convInTensor)    vxReleaseTensor(&convInTensor);
                if(convOutTensor)   vxReleaseTensor(&convOutTensor);
                if(tensorSub[0])    vxReleaseTensor(&tensorSub[0]);
                if(tensorSub[1])    vxReleaseTensor(&tensorSub[1]);
            }

            node->merged = vx_true_e;
        }/*if(vxoGraphOptimization_getKernelType(node) == OP_ADD_SUB)*/
    }/*for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)*/

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_splitMaxpFromCRL2(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_CONVOLUTION_RELU_POOLING || vxoGraphOptimization_getKernelType(node) == OP_CONVOLUTION_POOLING)
        {
            vx_tensor   weight                      = VX_NULL;
            vx_tensor   bias                        = VX_NULL;
            vx_size     dilation[2]                 = {0,0};
            vx_uint32   pool_size[2]                = {0, 0};
            vx_uint32   stride[2]                   = {0, 0};
            vx_uint32   pad[4]                      = {0,0,0,0};
            vx_uint8    accumulator_bits            = 0;
            vx_enum     overflow_policy             = VX_CONVERT_POLICY_WRAP;
            vx_enum     rounding_policy             = VX_ROUND_POLICY_TO_ZERO;
            vx_enum     down_scale_size_rounding    = VX_NN_DS_SIZE_ROUNDING_FLOOR;
            vx_bool     enable_relu                 = vx_false_e;
            vx_enum     pool_type                   = 0;
            vx_enum     pad_mode                    = VX_PAD_CONSTANT;
            vx_uint32   pad_const                   = 0;

            switch (node->kernel->enumeration){
            case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER:{
                vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConvReluPool(
                        node, &weight, &bias,
                        pool_size, pad,stride,
                        &accumulator_bits, &overflow_policy, &rounding_policy,
                        &down_scale_size_rounding, &enable_relu, &pool_type
                        );
                break;
                }
            case VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2:{
                    vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConvReluPool2(
                        node, &weight, &bias,
                        dilation, pool_size, pad,stride,
                        &accumulator_bits, &overflow_policy, &rounding_policy,
                        &down_scale_size_rounding, &enable_relu, &pool_type,
                        &pad_mode, &pad_const);
                    break;
                }
             default:
                break;
            }

            if(pool_size[0] == 3 && pool_size[1] == 3)
            {
                vx_uint32 maxpStride = 2, i;
                vx_tensor convInTensor = (vx_tensor)node->paramTable[0];
                vx_tensor maxpOutTensor = (vx_tensor)node->paramTable[node->numParameters-1];
                vx_tensor convOutTensor =  VX_NULL;
                vx_uint32 convOutDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
                vx_context context = vxGetContext((vx_reference)node);
                vx_tensor_create_params_t convOuttensorP;

                for(i = 0; i < TENSOR_DIM_NUM(maxpOutTensor); i++)
                    convOutDims[i] = TENSOR_SIZE_INDEX(maxpOutTensor, i);
                convOutDims[0] = ((convOutDims[0] + maxpStride - 1) /  maxpStride * maxpStride - 1)* maxpStride + pool_size[0];
                convOutDims[1] = ((convOutDims[1] + maxpStride - 1) /  maxpStride * maxpStride - 1)* maxpStride + pool_size[1];
                INITIALIZE_STRUCT(convOuttensorP);
                convOuttensorP.data_format                     = TENSOR_DATA_TYPE(maxpOutTensor);
                convOuttensorP.num_of_dims                     = TENSOR_DIM_NUM(maxpOutTensor);
                convOuttensorP.sizes                           = convOutDims;
                convOuttensorP.quant_format                    = TENSOR_QUANT_TYPE(maxpOutTensor);
                if(TENSOR_QUANT_TYPE(maxpOutTensor) == VX_QUANT_AFFINE_SCALE)
                {
                    convOuttensorP.quant_data.affine.scale         = TENSOR_TF_SCALE(maxpOutTensor);
                    convOuttensorP.quant_data.affine.zeroPoint     = TENSOR_TF_ZEROPOINT(maxpOutTensor);
                }
                else
                {
                    convOuttensorP.quant_data.dfp.fixed_point_pos  = TENSOR_POS(maxpOutTensor);
                }

                convOutTensor = vxCreateVirtualTensor2(graph, &convOuttensorP, sizeof(convOuttensorP));
                {
                    vx_scalar vxPadConst = vxCreateScalar(context, VX_TYPE_UINT32, &pad_const);
                    vx_nn_convolution_relu_pooling_params_ext2_t wb_params =
                    {
                        {
                            {
                                dilation[0], dilation[1], pad[0],pad[1],pad[2],pad[3],
                                accumulator_bits,overflow_policy, rounding_policy,down_scale_size_rounding,
                                enable_relu, 0, 0, 0, pad_mode, vxPadConst
                            },
                            stride[0], stride[1]
                        },
                        0, VX_TENSOR_RANK_WHCN, TENSOR_DATA_TYPE(convOutTensor)
                    };

                     vx_weights_biases_parameter wb = vxoGraphOptimization_CreateWBParameter(
                                                        VX_NN_CONVOLUTION_LAYER,
                                                        (vx_nn_convolution_relu_pooling_params_t *)&wb_params,
                                                        sizeof(wb_params),
                                                        convInTensor, convOutTensor, convOutTensor, weight, bias, VX_NULL, VX_NULL);
                    CHECK_NULL(wb);

                    {
                        vx_node newConvNode = vxConvolutionReluPoolingLayer2(graph, convInTensor, wb,
                            (vx_nn_convolution_relu_pooling_params)&wb_params, sizeof(wb_params),
                            convOutTensor);
                        CHECK_NULL(newConvNode);
                        vxReleaseNode(&newConvNode);
                    }

                    vxReleaseScalar(&vxPadConst);
                    vxReleaseWeightsBiasesParameter(&wb);
                }
                {
                    vx_nn_pooling_params_t p = { VX_NN_POOLING_MAX,
                         pool_size[0], pool_size[1], 0, 0, 0, 0, VX_NN_DS_SIZE_ROUNDING_FLOOR };

                    vx_node maxpNode = vxPoolingLayer2(graph,
                                               convOutTensor,
                                               (const vx_nn_pooling_params_t*)&p,
                                               sizeof(p),
                                               maxpOutTensor);
                    vxReleaseNode(&maxpNode);
                }

                vxReleaseTensor(&convOutTensor);
                node->merged = vx_true_e;
            }
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoGraphOptimization_transformConvNxM_padData(vx_tensor *padTensor, vx_int8 *buffer, vx_uint32 starth, vx_uint32 startw, void* padValue)
{
    vx_uint32 outc, inc, h, w;
    gcmHEADER_ARG("padTensor=%p, buffer=%p, starth=0x%x, startw=0x%x, padValue=%p", padTensor, buffer, starth, startw, padValue);
    CHECK_STATUS(vxoGraphOptimization_copyConstData2tensor(buffer,padTensor,VX_READ_ONLY));

    for(outc = 0; outc < TENSOR_SIZE_INDEX(*padTensor, 3); outc++){
        for(inc = 0; inc < TENSOR_SIZE_INDEX(*padTensor, 2); inc++){
            for(h = starth; h < TENSOR_SIZE_INDEX(*padTensor, 1); h++)
            {
                vx_int8 *ptr = (vx_int8 *)buffer
                    + outc * TENSOR_STRIDE_INDEX(*padTensor, 3)
                    + inc * TENSOR_STRIDE_INDEX(*padTensor, 2)
                    + h * TENSOR_STRIDE_INDEX(*padTensor, 1);
                for(w = startw; w < TENSOR_SIZE_INDEX(*padTensor, 0); w++)
                {
                    if(TENSOR_DATA_SIZE(*padTensor) == 2)
                        *(vx_int16 *)(ptr + w * 2) = *(vx_int16 *)padValue;
                    else
                        *(ptr + w) = *(vx_int8*)padValue;
                }
            }
        }
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_status vxoGraphOptimization_transformConvNxM(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    vx_context context = graph->base.context;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];

        if(vxoGraphOptimization_getKernelType(node) == OP_CONVOLUTION_NxM)
        {
            vx_uint32 i = 0;
            vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
            vx_tensor weightNxM     = (vx_tensor)node->paramTable[1];
            vx_tensor padedWeight   = VX_NULL;

            vx_uint32 padIndex = 0;
            vx_uint32 totalPad = gcmABS((vx_int32)TENSOR_SIZE_INDEX(weightNxM, 0) - (vx_int32)TENSOR_SIZE_INDEX(weightNxM, 1));

            if(!vxoGraphOptimization_nnHalSupport(weightNxM) )
                break;

            if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcFEATURE_BIT_NN_HW_LIMITATION_NATIVE_KER_1x2_2x1) &&
                 TENSOR_SIZE_INDEX(weightNxM, 0) * TENSOR_SIZE_INDEX(weightNxM, 1) == 2)
            {
                continue;
            }

            for(i = 0; i < TENSOR_DIM_NUM(weightNxM); i++)
                dims[i] = TENSOR_SIZE_INDEX(weightNxM, i);

            if(dims[0] > dims[1])
            {
                padIndex = 1;
            }
            else
            {
                padIndex = 0;
            }

            dims[padIndex] = dims[1 - padIndex];

            {
                vx_tensor_create_params_t p = vxoGraphOptimization_cloneParamsFromTensor(weightNxM);
                p.sizes = dims;
                padedWeight = vxCreateTensor2(((vx_reference)graph)->context, &p, sizeof(p));
            }
            vxoTensor_AllocateMemory(padedWeight);

            {
                vx_uint32 offsetView[VX_CONTEXT_TENSOR_MAX_DIMENSION];
                for(i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
                {
                    offsetView[i] = 0;
                    if(i == padIndex)
                    {
                        offsetView[i] = totalPad/2;
                    }
                }
                vxoGraphOptimization_transformConvNxM_padTensor(&weightNxM, &padedWeight, offsetView);
                TENSOR_DATA_LIFETIME(padedWeight) = VX_TENSOR_LIFE_TIME_STATIC;
            }
            {
            vx_tensor weight, bias;
            vx_size dilation[2];
            vx_uint32 pad[4], stride[2], depth_multiplier, pad_const;
            vx_enum pad_mode, overflow_policy,rounding_policy,down_scale_size_rounding;
             vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(
                    node, &weight, &bias,
                    dilation, stride, pad,
                    &overflow_policy,
                    &rounding_policy, &down_scale_size_rounding,
                    &depth_multiplier, &pad_mode, &pad_const);

             {
                 /*update pad parameter*/
                 pad[padIndex * 2] += totalPad/2;
                 pad[padIndex * 2 + 1] += (totalPad + 1)/2;
             }
             {
            vx_scalar padconst = vxCreateScalar(context, VX_TYPE_UINT32, (void *)&pad_const);
            vx_nn_convolution_params_ext2_t params = {
            {
                {
                    (vx_size)pad[0], (vx_size)pad[2], overflow_policy, rounding_policy, down_scale_size_rounding, dilation[0], dilation[1]
                },
                (vx_size)pad[1], (vx_size)pad[3], pad_mode, padconst
                },
                (vx_uint32)stride[0], (vx_uint32)stride[1], depth_multiplier
            };
            vx_node newNode = vxConvolutionLayer(graph, (vx_tensor)node->paramTable[0],
                                    padedWeight, bias,
                                    (const vx_nn_convolution_params_t *)&params, sizeof(params),
                                    (vx_tensor)node->paramTable[node->numParameters - 1]);

            vxReleaseNode(&newNode);
            vxReleaseScalar(&padconst);
             }
            }

            node->merged = vx_true_e;
            vxReleaseTensor(&padedWeight);
        }
    }
    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_tensor vxoGraphOptimization_conv2fc_reshapeTensor(vx_tensor oldTensor)
{
    vx_uint32 i = 0;
    vx_uint32 reshapeDims[2]  = {1,TENSOR_SIZE_INDEX(oldTensor, 3)};
    gcmHEADER_ARG("oldTensor=%p", oldTensor);
    for(i = 0; i < 3; i++)
        reshapeDims[0] *= TENSOR_SIZE_INDEX(oldTensor, i);
    gcmFOOTER_NO();
    return vxoGraphOptimization_CreateShareTensor(oldTensor, (vx_int32 *)reshapeDims, 2);
}

VX_INTERNAL_API vx_status vxoGraphOptimization_conv2fc(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_uint32 i = 0;
        vx_bool diff = vx_false_e;
        vx_node node = nodeTable[nodeIndex];
        if(node->kernel->enumeration == VX_KERNEL_CONVOLUTION_LAYER)
        {
            vx_tensor convInput     = (vx_tensor)node->paramTable[0];
            vx_tensor convWeight    = VX_NULL;
            vx_tensor bias          = VX_NULL;
            vx_tensor convOutput    = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);
            vx_size   dilation[2]   = {0, 0};
            vx_uint32 stride[2]     = {1, 1};
            vx_uint32 pad[4]        = {0,0,0,0};
            vx_uint32 depth_multipler = 0;
            vx_enum   overflow_policy = VX_CONVERT_POLICY_SATURATE;
            vx_enum   rounding_policy = VX_ROUND_POLICY_TO_ZERO;

            vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(node, &convWeight, &bias, dilation, stride,
                        pad, &overflow_policy, &rounding_policy, VX_NULL, &depth_multipler, VX_NULL, VX_NULL);

            if(depth_multipler != 0)
                continue;

            if(TENSOR_QUANT_TYPE(convWeight) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
                continue;

            if(dilation[0] != 0 || dilation[1] != 0 || stride[0]!= 1 || stride[1] != 1)
                continue;

            if(pad[0] != 0 || pad[1] != 0 || pad[2] != 0 || pad[3] != 0)
                continue;

            for(i = 0; i < 3; i++)
                if(TENSOR_SIZE_INDEX(convWeight, i)  != TENSOR_SIZE_INDEX(convInput, i) )
                    diff = vx_true_e;

            if(diff) continue;

            {
                vx_tensor input     = vxoGraphOptimization_conv2fc_reshapeTensor(convInput);
                vx_tensor weight    = vxoGraphOptimization_conv2fc_reshapeTensor(convWeight);
                vx_tensor output    = vxoGraphOptimization_conv2fc_reshapeTensor(convOutput);
                convOutput->reshape = output;

                TENSOR_DATA_LIFETIME(weight) = VX_TENSOR_LIFE_TIME_STATIC;
                {
                vx_node fcNode = vxFullyConnectedLayer(graph,
                                                    input,
                                                    weight,
                                                    bias,
                                                    overflow_policy,
                                                    rounding_policy,
                                                    output);
                vxReleaseNode(&fcNode);
                }
                vxReleaseTensor(&input);
                vxReleaseTensor(&weight);
                vxReleaseTensor(&output);


                node->merged = vx_true_e;
            }
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}
VX_INTERNAL_API vx_tensor * vxoGraphOptimization_unrollDWConv_multC21_unrollBias(vx_tensor bias, vx_uint32 depth_mult)
{
    vx_uint32   i = 0;
    vx_tensor   tmpTensor;
    vx_tensor   reshapeBias;
    vx_tensor   *unrollBias = (vx_tensor *)vxAllocateAndZeroMemory(sizeof(vx_tensor) * depth_mult);
    vx_uint32   reshapeDims[2] = { depth_mult, TENSOR_SIZE_INDEX(bias, 0)/depth_mult};
    vx_uint32   rollDims[1] = {reshapeDims[1]};
    vx_size     view_start[2] = {0, 0};
    vx_size     view_end[2] = {reshapeDims[0], reshapeDims[1]};

    reshapeBias = vxoGraphOptimization_CreateShareTensor(bias, (vx_int32 *)reshapeDims, 2);
    CHECK_NULL(reshapeBias);
    for(i = 0; i < depth_mult; i++)
    {
        view_start[0] = i, view_end[0] = i+1;
        tmpTensor = vxCreateTensorFromView(reshapeBias, 2,view_start, view_end);
        CHECK_NULL(tmpTensor);

        unrollBias[i] = vxoGraphOptimization_CreateShareTensor(tmpTensor, (vx_int32 *)rollDims, 1);
        CHECK_NULL(unrollBias[i]);
    }

    return unrollBias;
}
VX_INTERNAL_API vx_tensor * vxoGraphOptimization_unrollDWConv_multC21_unroll4DTensor(vx_tensor tensor, vx_uint32 depth_mult)
{
    vx_uint32   i = 0;
    vx_tensor   tmpTensor;
    vx_tensor   *unrollTensor = (vx_tensor *)vxAllocateAndZeroMemory(sizeof(vx_tensor) * depth_mult);
    vx_uint32   reshapeDims[4] = {TENSOR_SIZE_INDEX(tensor, 0), TENSOR_SIZE_INDEX(tensor, 1), depth_mult, TENSOR_SIZE_INDEX(tensor, 2) / depth_mult};
    vx_uint32   rollDims[4] = {reshapeDims[0], reshapeDims[1], reshapeDims[3], 1};

    vx_size view_start[4] = {0,0,0,0};
    vx_size view_end[4] = {reshapeDims[0], reshapeDims[1], 0, reshapeDims[3]};
    vx_tensor tt;

    tmpTensor = vxoGraphOptimization_CreateShareTensor(tensor, (vx_int32 *)reshapeDims, 4);
    CHECK_NULL(tmpTensor);

    for(i = 0; i < depth_mult; i++)
    {
        view_start[2] = i;
        view_end[2] = i+1;
        tt = vxCreateTensorFromView(tmpTensor, 4, view_start, view_end);
        CHECK_NULL(tt);

        unrollTensor[i] = vxoGraphOptimization_CreateShareTensor(tt, (vx_int32 *)rollDims, 4);
        CHECK_NULL(unrollTensor[i]);

        vxReleaseTensor(&tt);
    }

    vxReleaseTensor(&tmpTensor);
    return unrollTensor;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_unrollDWConv_multC21(vx_node node)
{
    vx_uint32   i = 0;
    vx_size     dilation[2]                 = {0,0};
    vx_uint32   stride[2]                   = {0, 0};
    vx_uint32   pad[4]                      = {0,0,0,0};
    vx_enum     overflow_policy             = VX_CONVERT_POLICY_WRAP;
    vx_enum     rounding_policy             = VX_ROUND_POLICY_TO_ZERO;
    vx_enum     down_scale_size_rounding    = VX_NN_DS_SIZE_ROUNDING_FLOOR;
    vx_uint32   depth_mult                  = 0;
    vx_tensor   input                       = (vx_tensor)node->paramTable[0];
    vx_tensor   output                      = (vx_tensor)node->paramTable[node->numParameters - 1];
    vx_tensor   dwweight, dwbias;
    vx_tensor   *unrollWeight, *unrollBias, *unrollOut;
    vx_enum     pad_mode                    = VX_PAD_CONSTANT;
    vx_uint32   pad_const                   = 0;
    vx_scalar   sPad;

    vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(
            node, &dwweight, &dwbias,
            dilation, stride, pad,
            &overflow_policy,
            &rounding_policy, &down_scale_size_rounding,
            &depth_mult, &pad_mode, &pad_const);

    unrollWeight    =  vxoGraphOptimization_unrollDWConv_multC21_unroll4DTensor(dwweight, depth_mult);
    unrollOut       =  vxoGraphOptimization_unrollDWConv_multC21_unroll4DTensor(output, depth_mult);
    unrollBias      =  vxoGraphOptimization_unrollDWConv_multC21_unrollBias(dwbias, depth_mult);

    sPad = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &pad_const);

    {
        vx_nn_convolution_params_ext2_t params = {
            {
                {
                    (vx_size)pad[0], (vx_size)pad[2], overflow_policy, rounding_policy, down_scale_size_rounding,
                        dilation[0], dilation[1]
                },
                (vx_size)pad[1], (vx_size)pad[3], pad_mode, sPad
            }, (vx_uint32)stride[0], (vx_uint32)stride[1], (vx_int32)1
        };

        for(i = 0; i < depth_mult; i++)
        {
            vx_node tmpnode = vxConvolutionLayer(node->graph, input, unrollWeight[i], unrollBias[i],
                                                        (const vx_nn_convolution_params_t *)&params, sizeof(params), unrollOut[i]);
            vxReleaseNode(&tmpnode);
        }
    }
    for(i = 0; i < depth_mult; i++)
    {
        vxReleaseTensor(unrollWeight + i);
        vxReleaseTensor(unrollOut + i);
        vxReleaseTensor(unrollBias + i);
    }

    vxReleaseScalar(&sPad);
    if(unrollOut) vxFree(unrollOut);
    if(unrollBias) vxFree(unrollBias);
    if(unrollWeight) vxFree(unrollWeight);

    node->merged = vx_true_e;
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_unrollDWConv(vx_graph graph)
{

    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_uint32 i = 0;
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_CONVOLUTION_DW)
        {
            vx_size     dilation[2]                 = {0,0};
            vx_uint32   stride[2]                   = {0, 0};
            vx_uint32   pad[4]                      = {0,0,0,0};
            vx_enum     overflow_policy             = VX_CONVERT_POLICY_WRAP;
            vx_enum     rounding_policy             = VX_ROUND_POLICY_TO_ZERO;
            vx_enum     down_scale_size_rounding    = VX_NN_DS_SIZE_ROUNDING_FLOOR;
            vx_uint32   depth_mult                  = SCALAR_VALUE(node->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32);
            vx_enum     pad_mode                    = VX_PAD_CONSTANT;
            vx_uint32   pad_const                   = 0;
            vx_tensor   input                       = (vx_tensor)node->paramTable[0];
            vx_tensor   output                      = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_tensor   dwweight;
            vx_tensor   bias;
            vx_tensor   padWeight;

            vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(
                    node, &dwweight, &bias,
                    dilation, stride, pad,
                    &overflow_policy,
                    &rounding_policy, &down_scale_size_rounding,
                    &depth_mult, &pad_mode, &pad_const);

            if(vxoGraphOptimization_dwConvHalSupport(input) && depth_mult == 1 &&
                (TENSOR_SIZE_INDEX(dwweight, 0) != 1 || TENSOR_SIZE_INDEX(dwweight, 1) != 1) &&
                stride[0] == stride[1] &&
                (stride[0] == 1 || stride[0] == 2) )
                continue;

            if(!vxoGraphOptimization_nnHalSupport(dwweight))
                continue;

            {
                vx_uint32 newTensorDims[4] = { TENSOR_SIZE_INDEX(dwweight, 0),
                                            TENSOR_SIZE_INDEX(dwweight, 1),
                                            TENSOR_SIZE_INDEX(input, 2),
                                            TENSOR_SIZE_INDEX(dwweight, 2)};

                vx_tensor_create_params_t p; INITIALIZE_STRUCT(p);
                p.data_format                       = TENSOR_DATA_TYPE(dwweight);
                p.quant_format                      = TENSOR_QUANT_TYPE(dwweight);
                if(VX_QUANT_AFFINE_SCALE == TENSOR_QUANT_TYPE(dwweight))
                {
                    p.quant_data.affine.scale           = TENSOR_TF_SCALE(dwweight);
                    p.quant_data.affine.zeroPoint       = TENSOR_TF_ZEROPOINT(dwweight);
                }
                else if(VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(dwweight))
                {
                    p.quant_data.dfp.fixed_point_pos    = TENSOR_POS(dwweight);
                }
                else if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL == TENSOR_QUANT_TYPE(dwweight))
                {
                    p.quant_data.affinePerChannel.scales = TENSOR_TF_SCALE_POINTER(dwweight);
                    p.quant_data.affinePerChannel.scaleCount = TENSOR_TF_SCALE_COUNT(dwweight);
                    p.quant_data.affinePerChannel.channelDim = TENSOR_TF_CHANNEL_DIMS(dwweight)+1;
                    p.quant_data.affinePerChannel.zeroPoint = TENSOR_TF_ZEROPOINT_POINTER(dwweight);
                    p.quant_data.affinePerChannel.zeroPointCount = TENSOR_TF_SCALE_COUNT(dwweight); //TODO: how to set it.
                }

                p.sizes                             = newTensorDims;
                p.num_of_dims                       = 4;

                padWeight = vxCreateTensor2(graph->base.context, &p, sizeof(p));
                CHECK_NULL(padWeight);
                CHECK_STATUS(vxoTensor_AllocateMemory(padWeight) );
                TENSOR_DATA_LIFETIME(padWeight) = VX_TENSOR_LIFE_TIME_STATIC;
            }
            {
                /*fill data for new weight*/
                vx_uint32 sliceSize = TENSOR_STRIDE_INDEX(dwweight, 2);

                vx_uint8  *src = TENSOR_LOGICAL_ADDR(dwweight);
                vx_uint8  *dst = TENSOR_LOGICAL_ADDR(padWeight);

                if(VX_QUANT_AFFINE_SCALE_PER_CHANNEL != TENSOR_QUANT_TYPE(padWeight))
                {
                    vx_int32  padV = TENSOR_QUANT_TYPE(padWeight) == VX_QUANT_AFFINE_SCALE ? TENSOR_TF_ZEROPOINT(padWeight) : 0;
                    if(TENSOR_DATA_TYPE(padWeight) == VX_TYPE_INT8 ||
                        TENSOR_DATA_TYPE(padWeight) == VX_TYPE_UINT8 ||
                        TENSOR_QUANT_TYPE(padWeight) != VX_QUANT_AFFINE_SCALE
                        )
                    {
                        memset(dst, padV, TENSOR_STRIDE_INDEX(padWeight, 4));
                    }
                    else
                    {
                        for(i = 0; i < TENSOR_STRIDE_INDEX(padWeight, 4); i+=TENSOR_STRIDE_INDEX(padWeight, 0))
                        {
                            if(TENSOR_DATA_SIZE(padWeight) == 2)
                                *(vx_int16 *)(dst + i) = (vx_int16)padV;
                            else
                                *(vx_int32 *)(dst + i) = padV;
                        }
                    }
                }
                else
                {
                    // fill pad data for perchannel weight.
                    // fill its pad with the relative channel's pad
                    vx_int32 *padv = TENSOR_TF_ZEROPOINT_POINTER(padWeight);
                    for(i = 0; i < TENSOR_SIZE_INDEX(padWeight, 3); i++)
                    {
                        vx_uint8 *startAddress = dst + i * TENSOR_STRIDE_INDEX(padWeight,3) ;
                        vx_uint32 j = 0;
                        for(j = 0; j < TENSOR_STRIDE_INDEX(padWeight,3); j += TENSOR_DATA_SIZE(padWeight))
                        {
                            if(TENSOR_DATA_SIZE(padWeight) == 1)
                                startAddress[j] = (vx_int8)padv[i];
                            else if(TENSOR_DATA_SIZE(padWeight) == 2)
                                *(vx_int16*)(startAddress+j) = (vx_int16)padv[i];
                            else if(TENSOR_DATA_SIZE(padWeight) == 4)
                                *(vx_int32*)(startAddress+j) = padv[i];
                        }
                    }
                }

                for(i = 0; i < TENSOR_SIZE_INDEX(padWeight, 3); i++)
                {
                    vx_uint32 offsetC = i / depth_mult;
                    memcpy(dst + i * TENSOR_STRIDE_INDEX(padWeight,3) + offsetC * TENSOR_STRIDE_INDEX(padWeight, 2),
                        src + i * TENSOR_STRIDE_INDEX(dwweight, 2), sliceSize);
                }
            }

            {
                vx_scalar sPad = vxCreateScalar(graph->base.context, VX_TYPE_UINT32, &pad_const);
                vx_nn_convolution_params_ext2_t params = {
                    {
                        {
                            (vx_size)pad[0], (vx_size)pad[2], overflow_policy, rounding_policy, down_scale_size_rounding,
                                dilation[0], dilation[1]
                        },
                        (vx_size)pad[1], (vx_size)pad[3], pad_mode, sPad
                    }, (vx_uint32)stride[0], (vx_uint32)stride[1], (vx_int32)0
                };

                vx_node convNode = vxConvolutionLayer(graph, input, padWeight, bias,
                                                    (const vx_nn_convolution_params_t *)&params, sizeof(params), output);

                vxReleaseNode(&convNode);
                vxReleaseScalar(&sPad);
            }
            vxReleaseTensor(&padWeight);

            node->merged = vx_true_e;
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_multiTranspose_mergeTransposes(vx_node transposeNodes[], vx_int32 nodeCnt)
{
    vx_int32 i = 0;
    vx_uint32 *preParam = NULL;
    vx_uint32 *finalParam = NULL;
    vx_tensor finalout = (vx_tensor)transposeNodes[nodeCnt - 1]->paramTable[transposeNodes[nodeCnt - 1]->numParameters - 1];
    vx_tensor input = (vx_tensor)transposeNodes[0]->paramTable[0];
    vx_uint32 j = 0, dimNum = TENSOR_DIM_NUM(input);
    vx_bool   sync = vx_true_e;

    for(i = 1; i < nodeCnt; i++)
    {
        if(TENSOR_DIM_NUM((vx_tensor)transposeNodes[i]->paramTable[0]) != dimNum)
        {
            vxError("fail to merge multi-transpose due to the difference parameter\n");
            return VX_SUCCESS;
        }
    }

    finalParam = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dimNum);
    vxMemCopy(finalParam, ((vx_array)transposeNodes[nodeCnt - 1]->paramTable[1])->memory.logicals[0], sizeof(vx_uint32) * dimNum);

    for(i = nodeCnt-2; i >=0; i--)
    {
        preParam = (vx_uint32 *)((vx_array)transposeNodes[i]->paramTable[1])->memory.logicals[0];
        for(j  = 0; j < dimNum; j++)
            finalParam[j] = preParam[finalParam[j]];
    }

    /*check the correctness*/
    for(j  = 0; j < dimNum; j++)
    {
        if(TENSOR_SIZE_INDEX(input, finalParam[j]) != TENSOR_SIZE_INDEX(finalout, j))
            goto error;
    }
    for(j = 0; j < dimNum; j++)
    {
        if(finalParam[j] != j)
            sync = vx_false_e;
    }

    if(sync)
    {
        /*if input can not be repalced, replace the casted node's input*/
        if(input->isViewed || !input->base.isVirtual)
        {
            vx_node* nodeTable = transposeNodes[0]->graph->nodeTable;
            vx_node tailnode = transposeNodes[nodeCnt - 1];
            vx_node castednode = nodeTable[tailnode->childNodes[0]];

            if(VX_SUCCESS != vxoGraphOptimization_updateTensorInGraph(castednode, &finalout, &input, 1))
                goto error;
        }
        else if(VX_SUCCESS != vxoGraphOptimization_updateTensorInGraph(transposeNodes[0], &input, &finalout, 1))
            goto error;
        transposeNodes[0]->merged = vx_true_e;
    }
    else
    {
        vxMemCopy(((vx_array)transposeNodes[0]->paramTable[1])->memory.logicals[0], finalParam, sizeof(vx_uint32) * dimNum);
        vxoNode_SetParameter(transposeNodes[0],transposeNodes[0]->numParameters - 1, (vx_reference)finalout);
    }

    for(i = 1; i < nodeCnt; i++)
        transposeNodes[i]->merged = vx_true_e;

error:
    vxFree(finalParam);

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_multiTranspose(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_int32 transposeCnt = 0;
        vx_uint32 tnodeMax = 5;
        vx_node node = nodeTable[nodeIndex];
        vx_node *transposeNodes;

        if(node->merged)
            continue;

        if(vxoGraphOptimization_getKernelType(node) != OP_TRANSPOSE)
            continue;

        /*find the first Trospose Node*/
        while(node->numParents == 1)
        {
            if(vxoGraphOptimization_getKernelType(nodeTable[node->parentNodes[0]]) != OP_TRANSPOSE ||
                nodeTable[node->parentNodes[0]]->merged == vx_true_e)
                break;

            node = nodeTable[node->parentNodes[0]];
        }
        tnodeMax = gcmMAX(tnodeMax, node->numChildren + 1);
        transposeNodes = (vx_node*)vxAllocateAndZeroMemory(sizeof(vx_node) * tnodeMax);
        CHECK_NULL(transposeNodes);

        if(node->numChildren > 1)
        {
            vx_uint32 i = 0, childTcnt = 0, invalidChild = 0;
            vx_uint32 dimNum = TENSOR_DIM_NUM((vx_tensor)node->paramTable[0]);
            vx_uint32 transposeDims[6] = {0xff};
            vx_uint32 reverse_index = 0; /*record the reversed child index*/
            vx_uint32 *sameTranspose = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * node->numChildren);

            /* firstly, find all of child transpose nodes with same parameter, and merge them to one,
               second, if all of children can be merged to one, and then merge with current node*/

            for(i = 0; i < node->numChildren; i++)
            {
                vx_node child = nodeTable[node->childNodes[i]];
                vx_tensor input = (vx_tensor)child->paramTable[0];
                if(vxoGraphOptimization_getKernelType(child) == OP_TRANSPOSE &&
                    vxoTensor_IsVirtualTensor(input) &&
                    child->numParents == 1)
                {
                    if(transposeDims[0] == 0xff)
                    {
                        vxMemCopy(transposeDims, ((vx_array)child->paramTable[1])->memory.logicals[0],
                            sizeof(vx_uint32) * dimNum);

                        sameTranspose[childTcnt++] = node->childNodes[i];
                    }
                    else
                    {
                        vx_uint32 j = 0;
                        vx_uint32 *ptr = (vx_uint32 *)((vx_array)child->paramTable[1])->memory.logicals[0];
                        for(j = 0; j < dimNum; j++)
                        {
                            if(transposeDims[j] != ptr[j])
                                break;
                        }

                        if(j == dimNum)
                            sameTranspose[childTcnt++] = node->childNodes[i];
                    }
                }
            }

            /*reduce all of the same nodes to one, just reverse the first child node*/
            {
                vx_tensor finalTensor = (vx_tensor)vxoGraphOptimization_getOutputParameter(nodeTable[sameTranspose[0]]);
                for(i = 1; i < childTcnt; i++)
                {
                    vx_node child = nodeTable[sameTranspose[i]];
                    vx_tensor childOut = (vx_tensor)vxoGraphOptimization_getOutputParameter(child);

                    if(child->numChildren == 0 || !vxoTensor_IsVirtualTensor(childOut))
                    {
                        finalTensor = (vx_tensor)vxoGraphOptimization_getOutputParameter(child);
                        reverse_index = i;
                        break;
                    }
                }

                /*TODO: how to choice the reversed node and delete rest of nodes*/
                for(i = 0; i < childTcnt; i++)
                {
                    vx_uint32 index = 0;
                    vx_node child = nodeTable[sameTranspose[i]];
                    vx_tensor childOut = (vx_tensor)vxoGraphOptimization_getOutputParameter(child);
                    if(child->numChildren)
                    {
                        vx_node graphchild = nodeTable[child->childNodes[0]];
                        if(graphchild->numParents > 1 ||
                            (finalTensor != childOut && !vxoTensor_IsVirtualTensor(childOut))) /*keep normal tensor*/
                        {
                            invalidChild ++;
                            continue;
                        }

                        if(vxoGraphOptimization_matchTensorInNode(graphchild, (vx_tensor)childOut, &index))
                            vxoGraphOptimization_updateTensorInNodeWithIndex(&graphchild, index, finalTensor);

                        if(i != 0)
                            child->merged = vx_true_e;
                    }
                }
            }

            if(!invalidChild && node->numChildren == childTcnt)
            {
                transposeNodes[0] = node;
                transposeCnt++;
                transposeNodes[transposeCnt++] = nodeTable[sameTranspose[reverse_index]];
                /*for(i = 0; i < childTcnt; i++)
                    transposeNodes[transposeCnt++] = nodeTable[sameTranspose[i]];*/
                if(transposeCnt > 1)
                    vxoGraphOptimization_multiTranspose_mergeTransposes(transposeNodes, transposeCnt);
            }

            if(sameTranspose)
                vxFree(sameTranspose);
        }/*if(node->numChildren > 1)*/
        else
        {
            /*for one branch case*/
            while(vxoGraphOptimization_getKernelType(node) == OP_TRANSPOSE && transposeCnt < 6)
            {
                /*do not merge node whose output is normal tensor*/
                vx_tensor outputTensor = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);
                if (outputTensor == VX_NULL || !vxoTensor_IsVirtualTensor(outputTensor))
                    break;

                transposeNodes[transposeCnt++] = node;
                if(node->numChildren != 1)
                    break;
                node = nodeTable[node->childNodes[0]];
                if(node->numParents > 1 || node->merged)
                    break;
            }

            if(transposeCnt > 1)
            {
                vxoGraphOptimization_multiTranspose_mergeTransposes(transposeNodes, transposeCnt);
            }
        }

        vxFree(transposeNodes );
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*
    inset PAD node for convolution whose padsize is beyond HW limitation
    and then padsize of convlution is set to 0.
 */
VX_INTERNAL_API vx_status vxoGraphOptimization_padConv(vx_graph graph)
{
    vx_int32 nodeIndex, i;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];

        vx_uint32 padSize[4] = {0,0,0,0};
        vx_uint32 bigDims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
        vx_tensor orgInput, padTensor;

        if(vxoGraphOptimization_getKernelType(node) != OP_CONVOLUTION_PAD)
            continue;

        orgInput = (vx_tensor)node->paramTable[0];
        vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(node,NULL,NULL,NULL,NULL,padSize,NULL, NULL, NULL, NULL, NULL, NULL);

        for(i = 0; i < (vx_int32)TENSOR_DIM_NUM(orgInput); i++)
            bigDims[i] = TENSOR_SIZE_INDEX(orgInput, i);

        bigDims[0] += padSize[0] + padSize[1];
        bigDims[1] += padSize[2] + padSize[3];

        {
            vx_tensor_create_params_t p;
            p.data_format                       = TENSOR_DATA_TYPE(orgInput);
            p.sizes                             = bigDims;
            p.num_of_dims                       = TENSOR_DIM_NUM(orgInput);
            p.quant_format                      = TENSOR_QUANT_TYPE(orgInput);
            if(TENSOR_QUANT_TYPE(orgInput) ==  VX_QUANT_AFFINE_SCALE)
            {
                p.quant_data.affine.scale           = TENSOR_TF_SCALE(orgInput);
                p.quant_data.affine.zeroPoint       = TENSOR_TF_ZEROPOINT(orgInput);
            }
            else
            {
                p.quant_data.dfp.fixed_point_pos    = TENSOR_POS(orgInput);
            }


            padTensor = vxCreateVirtualTensor2(graph,&p, sizeof(p));
            CHECK_NULL(padTensor);
        }

        /*inset pad node*/
        {
            vx_uint32 data = VX_QUANT_AFFINE_SCALE == TENSOR_QUANT_TYPE(orgInput)? TENSOR_TF_ZEROPOINT(orgInput): 0;
            vx_scalar padv = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &data);
            vx_int32 padfornt[2]  = { padSize[0], padSize[2] };
            vx_int32 padback[2]   = { padSize[1], padSize[3] };
            vx_nn_pad_params_t p;
            p.pad_front_array = padfornt;
            p.pad_back_array  = padback;
            p.numViewDimensions = 2;
            p.pad_mode = VX_PAD_CONSTANT;
            p.pad_const = padv;

            {
                vx_node padnode = vxTensorPadNode(graph, orgInput, padTensor, &p, sizeof(p));
                CHECK_NULL(padnode);
                CHECK_STATUS(vxReleaseNode(&padnode));
            }
        }

        {
            CHECK_STATUS(vxoNode_SetParameter(node, 0, (vx_reference)padTensor) );
            for(i = 0; i < 4; i++)
                SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + i], u32) = 0;
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/* Greatest Common Divisor*/
vx_bool vxoGetDataDivisors(vx_uint32 input_value, vx_uint32 *divisors, vx_uint32 gcd)
{
    vx_uint32 i                 = 0;

    for (i = gcmMIN(input_value, VIV_TENSOR_MAX_WIDTH - 1); i > 0; i --)
    {
        if ((i % gcd == 0) && (input_value % i == 0))
        {
            *divisors = i;

            return vx_true_e;
        }
    }

    return vx_false_e;
}

VX_PRIVATE_API vx_bool vxoGraphOptimization_GetElementTensorShape(vx_tensor input, vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 * num_of_dims)
{
    vx_status status            = VX_SUCCESS;
    vx_uint32 element_count     = 0;
    vx_uint32 i                 = 0;

    status = vxoTensor_GetTensorElementCount(input, &element_count);
    if (status != VX_SUCCESS) return vx_false_e;

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        sizes[i] = 1;
    }

    if (element_count < VIV_TENSOR_MAX_WIDTH)
    {
        sizes[0] = element_count;

        *num_of_dims = 2;
    }
    else
    {
        vx_uint32 divisors = 1;
        for (i = 0; i < 2; i++)
        {
            divisors = 1;
            vxoGetDataDivisors(element_count, &divisors, 1);

            sizes[i] = divisors;
            element_count = element_count / divisors;
        }

        sizes[2] = element_count;
        *num_of_dims = 3;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_status vxoGraphOptimization_EltwiseTensorShapeOpt(vx_tensor input0, vx_tensor input1, vx_tensor output, vx_uint32 sizes0[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 sizes1[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 sizes2[VX_CONTEXT_TENSOR_MAX_DIMENSION], vx_uint32 *dim_num)
{
    vx_status status            = VX_SUCCESS;

    vx_uint32 i                 = 0;
    vx_uint32 cnt               = 0;
    vx_uint32 element_count0    = 0;
    vx_uint32 element_count1    = 0;
    vx_uint32 dims              = 0;
    vx_bool   enable_broadcast  = vx_false_e;
    vx_bool   enable_broadcast1 = vx_false_e;
    vx_uint32 broadcast_Bits    = 0;

    vxoTensor_GetTensorElementCount(input0, &element_count0);
    vxoTensor_GetTensorElementCount(input1, &element_count1);

    if (element_count0 == 1 || element_count1 == 1)
    {
        enable_broadcast1 = vx_true_e;
    }

    /*************step 1:init tensor shape*****************/
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        sizes0[i] = 1;
        sizes1[i] = 1;
        sizes2[i] = 1;
    }

    /*************step 2:squeeze tensor shape*****************/
    for (i = 0; i < TENSOR_DIM_NUM(output); i++)
    {
        vx_uint32 sz0 = TENSOR_DIM_NUM(input0) > i ? TENSOR_VIEW_SIZE_INDEX(input0, i) : 1;
        vx_uint32 sz1 = TENSOR_DIM_NUM(input1) > i ? TENSOR_VIEW_SIZE_INDEX(input1, i) : 1;
        vx_uint32 sz2 = TENSOR_DIM_NUM(output) > i ? TENSOR_VIEW_SIZE_INDEX(output, i) : 1;

        if (sz0 == sz1 && sz0 == 1)
        {
            continue;
        }
        else
        {
            sizes0[cnt] = sz0;
            sizes1[cnt] = sz1;
            sizes2[cnt] = sz2;

            cnt ++;
            dims ++;
        }
    }

    for (i = 0; i < dims; i++)
    {
        vx_uint32 sz0 = sizes0[i];
        vx_uint32 sz1 = sizes1[i];

        if (sz0 != sz1)
        {
            enable_broadcast = vx_true_e;
            broadcast_Bits |= (1 << i);
        }
    }

    /*************step 3:reshape tensor shape*****************/
    if (enable_broadcast == vx_false_e || enable_broadcast1)
    {
        vxoGraphOptimization_GetElementTensorShape(input0, sizes0, &dims);
        vxoGraphOptimization_GetElementTensorShape(input1, sizes1, &dims);
        vxoGraphOptimization_GetElementTensorShape(output, sizes2, &dims);
    }
    else
    {
        switch (broadcast_Bits)
        {
        case ELTWISE_BROAD_CAST_BITS_0:
            {
                vx_uint32 element_count = 1;
                vx_uint32 divisors = 1;

                for (i = 1; i < dims; i++)
                {
                    element_count *= sizes0[i];
                }

                divisors = 1;
                vxoGetDataDivisors(element_count, &divisors, 1);

                sizes0[1] = divisors;
                sizes1[1] = divisors;
                sizes2[1] = divisors;
                sizes0[2] = element_count / divisors;
                sizes1[2] = element_count / divisors;
                sizes2[2] = element_count / divisors;
                dims = 3;

                break;
            }
        case ELTWISE_BROAD_CAST_BITS_0 | ELTWISE_BROAD_CAST_BITS_1:
        case ELTWISE_BROAD_CAST_BITS_0 | ELTWISE_BROAD_CAST_BITS_1 | ELTWISE_BROAD_CAST_BITS_2:
            {
                vx_uint32 w0 = sizes0[0] * sizes0[1];
                vx_uint32 w1 = sizes1[0] * sizes1[1];
                vx_uint32 w  = sizes2[0] * sizes2[1];
                vx_uint32 h = sizes0[2];

                if (h < VIV_TENSOR_MAX_WIDTH && (w0 == 1 || w1 == 1)
                    && w < VIV_TENSOR_MAX_WIDTH)
                {
                    sizes0[0] = w0;
                    sizes1[0] = w1;
                    sizes2[0] = w;
                    sizes0[1] = sizes0[2];
                    sizes1[1] = sizes1[2];
                    sizes2[1] = sizes2[2];
                    sizes0[2] = 1;
                    sizes1[2] = 1;
                    sizes2[2] = 1;
                }

                break;
            }
        case ELTWISE_BROAD_CAST_BITS_2:
            {
                vx_uint32 w = sizes0[0] * sizes0[1];

                if (w < VIV_TENSOR_MAX_WIDTH)
                {
                    sizes0[0] = w;
                    sizes1[0] = w;
                    sizes2[0] = w;
                    sizes0[1] = sizes0[2];
                    sizes1[1] = sizes1[2];
                    sizes2[1] = sizes2[2];
                    sizes0[2] = 1;
                    sizes1[2] = 1;
                    sizes2[2] = 1;
                }

                break;
            }
        default:
            if (dims == TENSOR_DIM_NUM(output))
                status = VX_FAILURE;
            break;
        }
    }

    if (status == VX_SUCCESS)
        *dim_num = gcmMAX(dims, 2);

    return status;
}

/*optimization for element-wise op*/
VX_INTERNAL_API vx_status vxoGraphOptimization_eltwiseOp(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];

        if(vxoGraphOptimization_getKernelType(node) == OP_ELTWISE_ASMD || vxoGraphOptimization_getKernelType(node) == OP_ADD_SUB)
        {
            vx_status status = VX_SUCCESS;
            vx_tensor tensorIn[2]   = {(vx_tensor)node->paramTable[0], (vx_tensor)node->paramTable[1]};
            vx_tensor output        = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_uint32 inDims0[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 inDims1[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 outDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 dimcnt = 0;

            status = vxoGraphOptimization_EltwiseTensorShapeOpt(tensorIn[0], tensorIn[1], output, inDims0, inDims1, outDims, &dimcnt);

            if (status == VX_SUCCESS)
            {
                vx_tensor newinput0 = vxoTensor_ReshapeTensor(tensorIn[0], (vx_int32*)inDims0, dimcnt, VX_NULL);
                vx_tensor newinput1 = vxoTensor_ReshapeTensor(tensorIn[1], (vx_int32*)inDims1, dimcnt, VX_NULL);
                vx_tensor newoutput = vxoTensor_ReshapeTensor(output, (vx_int32*)outDims, dimcnt, VX_NULL);
                CHECK_NULL(newinput0);
                CHECK_NULL(newinput1);
                CHECK_NULL(newoutput);
                newinput0->reshape = tensorIn[0];
                newinput1->reshape = tensorIn[1];
                newoutput->reshape = output;

                CHECK_STATUS(vxoNode_SetParameter(node, 0, (vx_reference)newinput0) );
                CHECK_STATUS(vxoNode_SetParameter(node, 1, (vx_reference)newinput1) );
                CHECK_STATUS(vxoNode_SetParameter(node, node->numParameters - 1, (vx_reference)newoutput) );

                CHECK_STATUS(vxoTensor_ReleaseTensor(&newinput0) );
                CHECK_STATUS(vxoTensor_ReleaseTensor(&newinput1) );
                CHECK_STATUS(vxoTensor_ReleaseTensor(&newoutput) );
            }

        }/*if(vxoGraphOptimization_getKernelType(node) == OP_ADD_SUB)*/
    }/*for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)*/

    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*
the feautre: replace previous node's output with relu's output and
this feature is only valid for quantized data type and do not merge
for branch.
*/
VX_INTERNAL_API vx_status vxoGraphOptimization_deleteRelu(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    /* all of the nodes here, would not do this feature, because they do not accept the change for quantize attribute*/
    vx_enum blacklist[] = {
        VX_KERNEL_TENSOR_TRANSPOSE,
        VX_KERNEL_INTERNAL_ADAPTER,
        VX_KERNEL_NN_REORG2_LAYER
    };

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        vx_enum nodeType =  vxoGraphOptimization_getKernelType(node);

        if(node->merged)    continue;

        if((nodeType == OP_RELU || nodeType == OP_RELU1 || nodeType == OP_RELU6) &&
            node->numParents == 1 && node->numChildren == 1 &&
            nodeTable[node->parentNodes[0]]->numChildren == 1 &&
            nodeTable[node->childNodes[0]]->numParents == 1
            )
        {
            vx_uint32   i           = 0;
            vx_tensor   reluIn      = (vx_tensor)node->paramTable[0];
            vx_tensor   reluOut     = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_node     child       = nodeTable[node->childNodes[0]];
            vx_node     parent      = nodeTable[node->parentNodes[0]];
            vx_enum     dataType    = TENSOR_DATA_TYPE(reluIn);
            vx_int32    zp          = TENSOR_TF_ZEROPOINT(reluOut);    /*it is required to be in range [0, 255]*/
            vx_float32  scale       = TENSOR_TF_SCALE(reluOut);
            vx_float32  max         = scale * (255-zp);
            vx_float32  min         = scale * -zp;

            if(dataType != VX_TYPE_INT8 && dataType != VX_TYPE_UINT8 && dataType != VX_TYPE_UINT16)
                continue;
            if(TENSOR_QUANT_TYPE(reluOut) != VX_QUANT_AFFINE_SCALE)
                continue;

            if(TENSOR_TF_SCALE(reluIn) != TENSOR_TF_SCALE(reluOut) ||
                TENSOR_TF_ZEROPOINT(reluIn) != TENSOR_TF_ZEROPOINT(reluOut))
                continue;

            /*add blacklist, in which nodes do not requantize data*/
            for(i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); i++)
            {
                if(child->kernel->enumeration == blacklist[i] || parent->kernel->enumeration == blacklist[i])
                    break;
            }
            if(i != sizeof(blacklist)/sizeof(blacklist[0]))
                continue;

            /* if the frange of quantization is not beyond the reluN's frange,
            ** which means the op is unusefull and it can be deleted. otherwise keep it.
            */
            if(OP_RELU == nodeType)
            {
                if(min < 0)
                    continue;
            }
            else if(OP_RELU1 == nodeType)
            {
                if(min < -1 || max > 1.0f)
                    continue;
            }
            else if(OP_RELU6 == nodeType)
            {
                if(min < 0.0f || max > 6.0f)
                    continue;
            }

            vxoGraphOptimization_updateTensorInGraph(parent, &reluIn, &reluOut, 1);
            node->merged = vx_true_e;
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_deleteSqueeze(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(node->merged)    continue;

        if(node->kernel->enumeration == VX_KERNEL_NN_TENSOR_SQUEEZE && node->numChildren == 1)
        {
            vx_uint32 index = 0;
            vx_node child = nodeTable[node->childNodes[0]];
            vx_tensor in  = (vx_tensor)node->paramTable[0];
            vx_tensor out = (vx_tensor)node->paramTable[node->numParameters - 1];

            if(child->numParents > 1)
                continue;

            if(vxoGraphOptimization_matchTensorInNode(child, out, &index))
                vxoGraphOptimization_updateTensorInNodeWithIndex(&child, index, in);

            node->merged = vx_true_e;
        }
    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}


VX_INTERNAL_API vx_status vxoGraphOptimization_war1x1x1weight(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];

        if(node->merged)
            continue;
        if(node->kernel->enumeration == VX_KERNEL_CONVOLUTION_LAYER)
        {
            vx_tensor weight = (vx_tensor)node->paramTable[1];
            vx_tensor newWeight = NULL;

            if(weight == NULL || TENSOR_DATA_LIFETIME(weight) != VX_TENSOR_LIFE_TIME_STATIC)
                continue;

            if(TENSOR_SIZE_INDEX(weight, 0) != 1 ||
                TENSOR_SIZE_INDEX(weight, 1) != 1 ||
                TENSOR_SIZE_INDEX(weight, 2) != 1)
                continue;

            newWeight = vxoGraphOptimization_WAR_convert1x1x1weight(weight);
            if(NULL == newWeight)
                continue;
            TENSOR_DATA_LIFETIME(newWeight) = TENSOR_DATA_LIFETIME(weight);
            TENSOR_VALUED(newWeight) = TENSOR_VALUED(weight);

            vxoNode_SetParameter(node, 1, (vx_reference)newWeight);
            vxReleaseTensor(&newWeight);
        }
    }

    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_node vxoGraphOptimization_avgPoolAnd1x1Conv_createNewConv(vx_graph graph, vx_node avgPoolNode, vx_node pwConvNode,
                                                          vx_tensor newWeight, vx_tensor newBias)
{
    vx_node     newNode = VX_NULL;
    vx_size     dilation[2]                 = {0,0};
    vx_uint32   stride[2]                   = {0, 0};
    vx_uint32   pad[4]                      = {0,0,0,0};
    vx_enum     overflow_policy             = VX_CONVERT_POLICY_WRAP;
    vx_enum     rounding_policy             = VX_ROUND_POLICY_TO_ZERO;
    vx_enum     down_scale_size_rounding    = VX_NN_DS_SIZE_ROUNDING_FLOOR;
    vx_enum     pad_mode                    = VX_PAD_CONSTANT;
    vx_uint32   pad_const                   = 0;
    vx_uint32   depth_multiplier            = 0;

    gcmHEADER_ARG("graph=%p,avgPoolNode=%p,pwConvNode=%p,newWeight=%p,newBias=%p",
        graph, avgPoolNode, pwConvNode,newWeight, newBias);

    vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(
        pwConvNode, VX_NULL, VX_NULL,
        dilation, stride, pad,
        &overflow_policy,
        &rounding_policy, &down_scale_size_rounding,
        &depth_multiplier, &pad_mode, &pad_const);

    /*TODO: conv' pad should be 0, stride be 1, dilation be 0.*/
    if(pad[0] != 0 || pad[1] != 0 || pad[2] != 0 || pad[3] != 0)
        goto out;

    if(stride[0] != 1 || stride[1] != 1)
        goto out;

    if(dilation[0] != 0 || dilation[1] != 0)
        goto out;

    pad[0] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_PAD_X_L_INDEX], u32);
    pad[1] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_PAD_X_R_INDEX], u32);
    pad[2] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_PAD_Y_T_INDEX], u32);
    pad[3] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_PAD_Y_B_INDEX], u32);

    stride[0] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX], u32);
    stride[1] = SCALAR_VALUE(avgPoolNode->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX], u32);
    {
        vx_tensor   inputTensor     = (vx_tensor)avgPoolNode->paramTable[0];
        vx_tensor   outputTensor    = (vx_tensor)pwConvNode->paramTable[pwConvNode->numParameters - 1];

        vx_nn_convolution_params_ext2_t p =
        {
            {
                {
                    pad[0], pad[2], overflow_policy, rounding_policy, down_scale_size_rounding, dilation[0], dilation[1]
                },
                pad[1], pad[3], pad_mode, 0
            },
            stride[0], stride[1], depth_multiplier
        };

        newNode = vxConvolutionLayer(graph, inputTensor, newWeight, newBias, (vx_nn_convolution_params_t *)&p, sizeof(p), outputTensor);
        CHECK_NULL(newNode);
    }

out:
    gcmFOOTER_ARG("node=%p",newNode);
    return newNode;
}

VX_INTERNAL_API vx_tensor vxoGraphOptimization_avgPoolAnd1x1Conv_resetBiasQuantAttribute(vx_graph graph, vx_tensor input, vx_tensor weight, vx_tensor bias)
{
    vx_uint32   i           = 0;
    vx_uint32   biasSize    = 0;
    vx_float32  newScale    = 0;
    vx_float32  minData     =0.0f;
    vx_float32  maxData     =0.0f;
    vx_tensor   newBias     = bias;

    gcmHEADER_ARG("graph=%p, input=%p, weight=%p, bias=%p", graph, input, weight, bias);

    if(bias == NULL)
        goto out;

    if(TENSOR_QUANT_TYPE(weight) != VX_QUANT_AFFINE_SCALE)
        goto out;

    if(TENSOR_TF_SCALE(bias) == TENSOR_TF_SCALE(input) * TENSOR_TF_SCALE(weight))
        goto out;

    newBias     = vxoGraphOptimization_cloneTensor(bias, graph, vxoTensor_IsVirtualTensor(bias));
    CHECK_NULL(newBias);

    newScale    = TENSOR_TF_SCALE(input) * TENSOR_TF_SCALE(weight);
    TENSOR_TF_SCALE(newBias) = newScale;

    vxoGraphOptimization_getTensorMaxOrMinValue(bias, &minData, &maxData);
    TENSOR_TF_ZEROPOINT(newBias) = gcmMIN(255, gcmMAX(0, (vx_int32)roundRTNE(0 - minData/ newScale)));

    vxoTensor_AllocateMemory(newBias);
    vxoTensor_GetTensorElementCount(bias, &biasSize);
    for(i = 0; i < biasSize; i++)
    {
        float value = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(bias), TENSOR_QUANT_TYPE(bias), i,
                                    TENSOR_LOGICAL_ADDR(bias), TENSOR_POS(bias), TENSOR_TF_ZEROPOINT(bias), TENSOR_TF_SCALE(bias));
        vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(newBias), TENSOR_QUANT_TYPE(newBias), i, value,
                        TENSOR_LOGICAL_ADDR(newBias), TENSOR_POS(newBias), TENSOR_TF_ZEROPOINT(newBias), newScale, TENSOR_ROUNDING_MODE(newBias));
    }

out:
    gcmFOOTER_ARG("newBias = %p", newBias);
    return newBias;
}

/*check whether it is valid to merge avgPool + 1x1Conv*/
VX_INTERNAL_API vx_bool vxoGraphOptimization_avgPoolAnd1x1Conv_isValid(vx_node avgNode)
{
    vx_graph    graph       = avgNode->graph;
    vx_node *   nodeTable   = avgNode->graph->nodeTable;
    vx_node     pwConvNode  = NULL;
    vx_tensor   weight1x1   = NULL;
    vx_uint32   avgPoolSize[2] = {0, 0};
    vx_uint32   convPad[4]  = {0,0,0,0};
    vx_uint32   convStride[2] = {1, 1};
    vx_uint32   avgPoolStride[2] = {1, 1};
    vx_uint32   depth_multipler = 0;

    vx_bool     isValid     = vx_false_e;
    vx_enum     poolType;

    gcmHEADER_ARG("avgNode=%p", avgNode);
    vxmASSERT(avgNode);

    if(avgNode->merged || avgNode->numChildren != 1 ||
        avgNode->kernel->enumeration != VX_KERNEL_NN_POOLING_LAYER2)
        goto out;

    poolType = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_TYPE_INDEX], u32);
    if(poolType != VX_NN_POOLING_AVG && poolType != VX_NN_POOLING_AVG_ANDROID)
        goto out;

    avgPoolSize[0] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
    avgPoolSize[1] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
    avgPoolStride[0] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_STRIDE_X_INDEX], u32);
    avgPoolStride[1] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_STRIDE_Y_INDEX], u32);

    if(avgPoolSize[0] != 3 || avgPoolSize[1] != 3)
        goto out;

    pwConvNode = nodeTable[avgNode->childNodes[0]];
    if(pwConvNode == NULL || pwConvNode->numParents > 1 || pwConvNode->merged == vx_true_e)
        goto out;
    if(VX_KERNEL_CONVOLUTION_LAYER != pwConvNode->kernel->enumeration)
        goto out;

    vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(pwConvNode, VX_NULL, VX_NULL, VX_NULL, convStride, convPad,
        NULL, VX_NULL, VX_NULL, &depth_multipler, VX_NULL, VX_NULL);

    if(depth_multipler == 0)
    {
        vx_uint32 acutalKernelX = vxoGraphOptimization_computeFinalKernelSize(avgPoolSize[0], avgPoolStride[0]);
        vx_uint32 acutalKernelY = vxoGraphOptimization_computeFinalKernelSize(avgPoolSize[1], avgPoolStride[1]);
        if(acutalKernelX > vxoGraphOptimization_getMaxKernelSizeX(graph->base.context) ||
            acutalKernelY > vxoGraphOptimization_getMaxKernelSizeY(graph->base.context)
          )
            goto out;
    }
    else
    {
        /*for depthwise conv, reshuffle should not be taken into account*/
        if((avgPoolStride[0] > 2 || avgPoolStride[1] >2) ||
            (avgPoolSize[0] > vxoGraphOptimization_getMaxKernelSizeX(graph->base.context) ||
             avgPoolSize[1] > vxoGraphOptimization_getMaxKernelSizeY(graph->base.context) )
          )
            goto out;
    }

    weight1x1 = (vx_tensor)pwConvNode->paramTable[1];
    if(TENSOR_SIZE_INDEX(weight1x1, 0) != 1 || TENSOR_SIZE_INDEX(weight1x1, 1) != 1 )
        goto out;
    if(!vxnneIsNNSupportFormat(graph->base.context, graph, (vx_tensor)avgNode->paramTable[0],
                                NULL, (vx_tensor)pwConvNode->paramTable[pwConvNode->numParameters - 1]))
        goto out;
    if(!vxoGraphOptimization_nnHalSupport(weight1x1) )
        goto out;

    if(convPad[0] + convPad[1] + convPad[2] + convPad[3] != 0)
        goto out;
    if(convStride[0] != 1 || convStride[1] != 1 )
        goto out;

    isValid = vx_true_e;
out:
    gcmFOOTER_ARG("isValid = %d", isValid);
    return isValid;
}

/*creat a new weight [1/windowsize, 1/windowsize, c, n] from the old weight[1,1,c,n]*/
VX_INTERNAL_API vx_tensor vxoGraphOptimization_avgPoolAnd1x1Conv_prepareNewWeight(vx_tensor pwWeight, vx_uint32 avgPoolSize[2])
{
    vx_uint32   i           = 0;
    vx_uint32   sliceSize   = avgPoolSize[0] * avgPoolSize[1];
    vx_uint32   orgSize     = 0;
    vx_uint32   weightDims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {avgPoolSize[0], avgPoolSize[1], 0};

    vx_uint8_ptr orgWeightAddr = TENSOR_LOGICAL_ADDR(pwWeight);
    vx_tensor_create_params_t p;
    vx_tensor   newWeight = VX_NULL;

    gcmHEADER_ARG("pwWeight=%p, avgPoolSize=%p", pwWeight, avgPoolSize);

    vxmASSERT(pwWeight || avgPoolSize);
    vxmASSERT(TENSOR_SIZE_INDEX(pwWeight, 0) == 1 && TENSOR_SIZE_INDEX(pwWeight, 1) == 1 );

    for(i = 2; i < TENSOR_DIM_NUM(pwWeight); i++)
        weightDims[i] = TENSOR_SIZE_INDEX(pwWeight, i);

    {
        /*compute new quant attribute of new weight.*/
        vx_int8     fixedPointPos   = 0;
        vx_int32    zeroPoint       = 0;
        vx_float32  scale           = 0;
        vx_float32  maxdata = 0, mindata = 0;

        vxoGraphOptimization_getTensorMaxOrMinValue(pwWeight, &mindata, &maxdata);
        CHECK_STATUS(vxoGraphOptimization_computeQuantAttribute(TENSOR_QUANT_TYPE(pwWeight), TENSOR_DATA_TYPE(pwWeight),
            maxdata/sliceSize, mindata/sliceSize, &fixedPointPos, &zeroPoint, &scale) );
        p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(pwWeight), weightDims, TENSOR_DATA_TYPE(pwWeight),
            TENSOR_QUANT_TYPE(pwWeight),fixedPointPos, zeroPoint, scale);
    }

    newWeight = vxCreateTensor2(pwWeight->base.context, &p, sizeof(p));    CHECK_NULL(newWeight);
    CHECK_STATUS(vxoTensor_AllocateMemory(newWeight));

    /* set new value into weight */
    vxoTensor_GetTensorElementCount(pwWeight, &orgSize);
    for(i = 0; i < orgSize; i++)
    {
        vx_uint32 k = 0;
        vx_float32 orgData = vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(pwWeight), TENSOR_QUANT_TYPE(pwWeight), i,
                        orgWeightAddr, TENSOR_POS(pwWeight), TENSOR_TF_ZEROPOINT(pwWeight), TENSOR_TF_SCALE(pwWeight));
        orgData /= sliceSize;
        for(k = 0; k < sliceSize; k++)
        {
            vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(newWeight), TENSOR_QUANT_TYPE(newWeight), i* sliceSize + k, orgData,
                TENSOR_LOGICAL_ADDR(newWeight), TENSOR_POS(newWeight), TENSOR_TF_ZEROPOINT(newWeight),
                TENSOR_TF_SCALE(newWeight), TENSOR_ROUNDING_MODE(pwWeight));
        }
    }

    gcmFOOTER_ARG("newWeight = %p", newWeight);
    return newWeight;
}

/* merge (nxn)avgpool + (1x1)conv to nxn conv for NN or TP. */
VX_INTERNAL_API vx_status vxoGraphOptimization_avgPoolAnd1x1Conv(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node avgNode = nodeTable[nodeIndex];
        vx_node pwConvNode  = VX_NULL;
        vx_node newNode     = VX_NULL;

        vx_tensor pwWeight  = VX_NULL;
        vx_tensor newWeight = VX_NULL;
        vx_tensor pwBias    = VX_NULL;
        vx_tensor newBias   = VX_NULL;

        vx_uint32 avgPoolSize[2] = {0, 0};

        if(!vxoGraphOptimization_avgPoolAnd1x1Conv_isValid(avgNode))
            continue;

        avgPoolSize[0] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
        avgPoolSize[1] = SCALAR_VALUE(avgNode->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);

        pwConvNode  = nodeTable[avgNode->childNodes[0]];
        pwWeight    = (vx_tensor)pwConvNode->paramTable[1];
        pwBias      = (vx_tensor )pwConvNode->paramTable[2];

        /*  windowsize = avgPoolSize[0] * avgPoolSize[1];
            new weight's dim will be [1/windowSize, 1/windowSize, c, n)
            relactively, each of the weight's data will be diviede by windowsize;
        */
        newWeight = vxoGraphOptimization_avgPoolAnd1x1Conv_prepareNewWeight(pwWeight, avgPoolSize);
        CHECK_NULL(newWeight);

        newBias = vxoGraphOptimization_avgPoolAnd1x1Conv_resetBiasQuantAttribute(graph, (vx_tensor)avgNode->paramTable[0], newWeight, pwBias);
        if(VX_NULL == newBias)
            goto out;

        TENSOR_DATA_LIFETIME(newBias) = VX_TENSOR_LIFE_TIME_STATIC;
        TENSOR_DATA_LIFETIME(newWeight) = VX_TENSOR_LIFE_TIME_STATIC;
        newNode = vxoGraphOptimization_avgPoolAnd1x1Conv_createNewConv(graph, avgNode, pwConvNode, newWeight, newBias);

 out:
        if(newBias != pwBias)
            vxReleaseTensor(&newBias);

        if(newNode)
        {
            avgNode->merged = vx_true_e;
            pwConvNode->merged = vx_true_e;
            vxReleaseNode(&newNode);
        }

        if(newWeight)
            vxReleaseTensor(&newWeight);

    }

    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*convert quantized data to float data*/
vx_float32* vxoGraphOptimization_pcq_getFpWeightData(vx_tensor perchannelWeight)
{
    vx_uint32 i = 0, cnt = 0;
    vx_float32_ptr realData = VX_NULL;
    vx_uint32 sliceCnt = 0;
    vxmASSERT(perchannelWeight);

    vxoTensor_GetTensorElementCount(perchannelWeight, &cnt);
    realData = (vx_float32_ptr)vxAllocateAndZeroMemory(cnt * sizeof(vx_float32));
    if(realData == VX_NULL)
    {
        vxError("fail to malloc memory\n");
        return realData;
    }
    sliceCnt = cnt / TENSOR_TF_SCALE_COUNT(perchannelWeight);
    for(i = 0; i < TENSOR_TF_SCALE_COUNT(perchannelWeight); i++)
    {
        vx_float32  scale   = TENSOR_TF_SCALES_WITH_INDEX(perchannelWeight, i);
        vx_int32    zp      = TENSOR_TF_ZEROPOINTS_WITH_INDEX(perchannelWeight, i);
        vx_uint32   k       = 0;
        vx_uint8_ptr dataAddr = TENSOR_LOGICAL_ADDR(perchannelWeight) + i * sliceCnt * TENSOR_DATA_SIZE(perchannelWeight);

        for(k = 0; k < sliceCnt; k++)
        {
            realData[k + sliceCnt * i] = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(perchannelWeight), k, dataAddr, zp, scale);
        }
    }
    return realData;
}

/*convert quantized data to float data*/
void vxoGraphOptimization_pcq_quantizeData2Tensor(vx_float32* fp32data, vx_tensor tensor)
{
    vx_uint32 i = 0, cnt = 0;
    vx_uint8_ptr data = VX_NULL;
    vxmASSERT(tensor);

    vxoTensor_GetTensorElementCount(tensor, &cnt);
    if(!vxoTensor_IsAllocated(tensor))
        vxoTensor_AllocateMemory(tensor);

    data = TENSOR_LOGICAL_ADDR(tensor);
    for(i = 0; i < cnt; i++)
    {
        vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(tensor), i, fp32data[i], data,
        TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor), TENSOR_ROUNDING_MODE(tensor));
    }

    return;
}

vx_float32 vxoGraphOptimization_pcq_optimizeEntropy(vx_float32 * weightData, vx_uint32 size)
{
    vx_uint32  i = 0;
    vx_int32   j = 0, cnt = 0;
    vx_float32 *noZeroData = (vx_float32 *)vxAllocateAndZeroMemory(sizeof(vx_float32) * size);
    vx_float32 encoryp = 1.0f, sum = 0.0f;

    if(noZeroData == VX_NULL)
    {
        vxError("fail to alloc memory");
        goto exit;
    }
    for(i = 0; i < size; i++)
    {
        if(gcmABS(weightData[i]) < 1e-4f)
            continue;
        noZeroData[cnt++] = gcmABS(weightData[i]);
    }

    vxoGraphOptimization_quickSort(noZeroData, cnt, 0, cnt - 1);
    for(j = 1; j < cnt + 1; j++)
    {
        encoryp += (2 * j - cnt - 1) * noZeroData[j - 1];
        sum += noZeroData[j-1];
    }
    encoryp /=  (cnt * sum);

exit:
    if(noZeroData) vxFree(noZeroData);
    return encoryp;
}
/*find the invalid channel of weight and reset the channel to 0,
  and add the const value to bias*/
vx_status vxoGraphOptimization_pcq_resetDeadChannel(vx_node node)
{
    vx_status   status      = VX_SUCCESS;
    vx_uint32   i           = 0;
    vx_uint32   weightCnt   = 0;
    vx_uint32   sliceCnt    = 0;
    vx_uint32   sliceSize   = 0;
    vx_tensor   input       = (vx_tensor)node->paramTable[0];
    vx_tensor   weight      = (vx_tensor)node->paramTable[1];
    vx_tensor   bias        = (vx_tensor)node->paramTable[2];
    vx_tensor   output      = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);

    vx_float32  inFpmax     = gcmMAX(0.0f, (255 - TENSOR_TF_ZEROPOINT(input)) * TENSOR_TF_SCALE(input));
    vx_float32  inFpmin     = gcmMIN(0.0f, (0   - TENSOR_TF_ZEROPOINT(input)) * TENSOR_TF_SCALE(input));
    vx_float32  outFpmax    = (255 - TENSOR_TF_ZEROPOINT(output)) * TENSOR_TF_SCALE(output);
    vx_float32  outFpmin    = (0   - TENSOR_TF_ZEROPOINT(output)) * TENSOR_TF_SCALE(output);

    vx_float32 *weightData  = vxoGraphOptimization_pcq_getFpWeightData(weight);
    if(VX_NULL == weightData)
    {
        vxError("fail to alloc memory");
        status = VX_FAILURE;
        goto exit;
    }

    vxoTensor_GetTensorElementCount(weight, &weightCnt);
    sliceCnt = weightCnt / TENSOR_TF_SCALE_COUNT(weight);
    sliceSize = sliceCnt * TENSOR_DATA_SIZE(weight);

    /*  compute the theoretical max and min of output, if max == min,
        it can be considered that the channel's output is const value.
        and we can add the const to bias*/
    for(i = 0; i < TENSOR_TF_SCALE_COUNT(weight); i++)
    {
        vx_float32 cmax = 0.0f;
        vx_float32 cmin = 0.0f;
        vx_float32 plusSum = 0, minusSum = 0;
        vx_uint32  j = 0;
        vx_float32 *data = weightData + i * sliceSize;
        vx_float32 biasData = ((vx_int32 *)TENSOR_LOGICAL_ADDR(bias))[i] * TENSOR_TF_SCALES_WITH_INDEX(bias, i);
        for(j = 0; j < sliceCnt; j++)
        {
            if(data[j] > 0.f)
                plusSum += data[j];
            else
                minusSum += data[j];
        }
        cmax = inFpmax * plusSum + inFpmin * minusSum + biasData;
        cmin = inFpmin * plusSum + inFpmax * minusSum + biasData;

        cmax = gcmMAX(gcmMIN(cmax, outFpmax), outFpmin);
        cmin = gcmMAX(gcmMIN(cmin, outFpmax), outFpmin);
        if(gcmABS(cmin -cmax) <= 1e-4f)
        {
            memset(TENSOR_LOGICAL_ADDR(weight) + i * sliceSize, 0, sliceSize);
            vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(bias), i, cmax, TENSOR_LOGICAL_ADDR(bias),
                TENSOR_TF_ZEROPOINTS_WITH_INDEX(bias, i), TENSOR_TF_SCALES_WITH_INDEX(bias, i), TENSOR_ROUNDING_MODE(bias));
        }
    }
exit:
    if(weightData) vxFree(weightData);
    return status;
}

/*split per-channel conv to 2 parts:
    1. covoluiton: In(u8) * weight(u8) + bias(null) = Out(fp16)
    2. BatchNorm:  In(fp16) * scale(fp32) + offset(fp32: conv's bias) = Out(U8)
*/
vx_status vxoGraphOptimization_pcq_splitPerChannel(vx_node node)
{
    vx_status   status    = VX_SUCCESS;
    vx_uint32   i         = 0;
    vx_tensor   input     = (vx_tensor)node->paramTable[0];
    vx_tensor   weight    = (vx_tensor)node->paramTable[1];
    vx_tensor   bias      = (vx_tensor)node->paramTable[2];
    vx_tensor   output    = (vx_tensor)vxoGraphOptimization_getOutputParameter(node);
    vx_tensor   fpOutput  = VX_NULL;
    vx_tensor   newWeight = VX_NULL;
    vx_float32  newScale  = 1.0f / (TENSOR_TF_SCALE(input) *(vx_float32)pow(2.0f, 16));
    vx_tensor_create_params_t p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(weight), TENSOR_SIZES(weight), VX_TYPE_UINT8,
        VX_QUANT_AFFINE_SCALE, TENSOR_POS(weight), 128, newScale);

    /*create perTensor quantization weight,
        1. convert I8 data to U8 and do not chage the memory data.
        2. just change the new weight's scale to input's scale * pow(2,-16)
    */
    newWeight =  vxCreateTensor2(node->base.context, &p, sizeof(p));
    if(VX_NULL == newWeight)
    {
        vxError("%s: fail to create tensor", __FUNCTION__);
        status = VX_FAILURE;
        goto fail;
    }
    status = vxoTensor_AllocateMemory(newWeight);
    TENSOR_DATA_LIFETIME(newWeight) = TENSOR_DATA_LIFETIME(weight);
    {
        vx_uint8_ptr dstPtr = TENSOR_LOGICAL_ADDR(newWeight);
        vx_int8_ptr srcPtr = (vx_int8_ptr)TENSOR_LOGICAL_ADDR(weight);
        for(i =0; i < TENSOR_STRIDE_INDEX(newWeight, TENSOR_DIM_NUM(newWeight)); i++)
        {
            dstPtr[i] = srcPtr[i] + 128;
        }
    }

    p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(output), TENSOR_SIZES(output), VX_TYPE_FLOAT16, VX_QUANT_DYNAMIC_FIXED_POINT, 0, 0, 1.0f);
    fpOutput = vxCreateTensor2(node->base.context, &p, sizeof(p));

    /****************************************/
    /*      create BN node                  */
    /****************************************/
    {
        vx_tensor   scaleTensor     = VX_NULL;
        vx_tensor   offsetTensor    = VX_NULL;
        vx_tensor   meanTensor      = VX_NULL;
        vx_tensor   varianceTensor  = VX_NULL;

        vx_int32    *biasAddr   = VX_NULL;
        vx_float32  *scaleAddr  = VX_NULL;
        vx_float32  *offsetAddr = VX_NULL;
        vx_float32  *meanAddr =  VX_NULL;
        vx_float32  *varianceAddr =  VX_NULL;
        vx_float32  exp16 = (vx_float32)pow(2.0f, 16.0f);

        p = vxoGraphOptimization_createParamsForTensor(1, &(TENSOR_TF_SCALE_COUNT(weight)), VX_TYPE_FLOAT32, 0, 0, 0, 1.0f);
        scaleTensor     = vxCreateTensor2(node->base.context, &p, sizeof(p));
        offsetTensor    = vxCreateTensor2(node->base.context, &p, sizeof(p));
        meanTensor      = vxCreateTensor2(node->base.context, &p, sizeof(p));
        varianceTensor  = vxCreateTensor2(node->base.context, &p, sizeof(p));

        vxoTensor_AllocateMemory(scaleTensor);
        vxoTensor_AllocateMemory(offsetTensor);
        vxoTensor_AllocateMemory(meanTensor);
        vxoTensor_AllocateMemory(varianceTensor);

        biasAddr   = (vx_int32 *)TENSOR_LOGICAL_ADDR(bias);
        scaleAddr  = (vx_float32 *)TENSOR_LOGICAL_ADDR(scaleTensor);
        offsetAddr = (vx_float32 *)TENSOR_LOGICAL_ADDR(offsetTensor);
        meanAddr    = (vx_float32 *)TENSOR_LOGICAL_ADDR(meanTensor);
        varianceAddr = (vx_float32 *)TENSOR_LOGICAL_ADDR(varianceTensor);

        for(i = 0; i < TENSOR_TF_SCALE_COUNT(weight); i ++)
        {
            scaleAddr[i] = TENSOR_TF_SCALES_WITH_INDEX(weight, i) * exp16 * TENSOR_TF_SCALE(input);
            offsetAddr[i] = biasAddr[i] * TENSOR_TF_SCALES_WITH_INDEX(bias, i) ;
            meanAddr[i] = 0.0f;
            varianceAddr[i] = 1.0f;
        }
        {
            vx_node bnNode =  vxBatchNormalizationLayer(node->graph, 0, meanTensor, varianceTensor, scaleTensor, offsetTensor, fpOutput, output);
            CHECK_NULL(bnNode);
            vxReleaseNode(&bnNode);
        }
        if(scaleTensor) vxReleaseTensor(&scaleTensor);
        if(offsetTensor) vxReleaseTensor(&offsetTensor);
        if(meanTensor) vxReleaseTensor(&meanTensor);
        if(varianceTensor) vxReleaseTensor(&varianceTensor);
    }
    vxoNode_SetParameter(node, 1, (vx_reference)newWeight);
    vxoNode_SetParameter(node, 2, (vx_reference)VX_NULL);
    bias->base.internalCount --;
    node->paramTable[2] = VX_NULL;
    {
        vx_uint32 outputIndex = vxoGraphOptimization_getOutputIndex(node);
        vxoNode_SetParameter(node, outputIndex, (vx_reference)fpOutput);
    }

fail:
    if(fpOutput) vxReleaseTensor(&fpOutput);
    if(newWeight) vxReleaseTensor(&newWeight);

    return status;
}

/*requantize the weight to pertensor, according to its fpmax and fpmin*/
vx_tensor vxoGraphOptimization_pcq_perTensor(vx_tensor weight, vx_float32 *weightData)
{
    vx_tensor   newWeight = VX_NULL;
    vx_float32  fpMax = 0.0f, fpMin = 0.0f;
    vx_uint32   i         = 0;
    vx_uint32   size      = 0;

    vx_tensor_create_params_t p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(weight), TENSOR_SIZES(weight), VX_TYPE_UINT8,
                                        VX_QUANT_AFFINE_SCALE, 0, 128, 1.0f);

    vxoTensor_GetTensorElementCount(weight, &size);
    for(i = 0; i < size; i++)
    {
        if(fpMax < weightData[i])
            fpMax = weightData[i];
        if(fpMin > weightData[i])
            fpMin = weightData[i];
    }

    vxoGraphOptimization_getAsymQuantizeAttribute(VX_TYPE_UINT8, fpMax, fpMin, &p.quant_data.affine.scale, &p.quant_data.affine.zeroPoint);
    newWeight = vxCreateTensor2(weight->base.context, &p, sizeof(p));
    if(VX_NULL == newWeight){
        goto exit;
    }

    vxoTensor_AllocateMemory(newWeight);
    for(i = 0; i < size; i++)
    {
        vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(newWeight), i, weightData[i],
            TENSOR_LOGICAL_ADDR(newWeight), TENSOR_TF_ZEROPOINT(newWeight), TENSOR_TF_SCALE(newWeight), TENSOR_ROUNDING_MODE(newWeight));
    }

exit:
    return newWeight;
}

/*requantize per-channel weight to per-tensor weight*/
VX_INTERNAL_API vx_bool vxoGraphOptimization_isDepthWiseConv(vx_node node)
{
    vx_bool isDWConv = vx_false_e;
    vx_uint32 depth_multiplier = 0;

    if(node->kernel->enumeration != VX_KERNEL_CONVOLUTION_LAYER)
        return isDWConv;

    vxoGraphOptimization_MergeConvolutionNodes_GetParmFromConv(node, VX_NULL, VX_NULL, VX_NULL, VX_NULL,
        VX_NULL, VX_NULL, VX_NULL, VX_NULL, &depth_multiplier, VX_NULL, VX_NULL);

    if(depth_multiplier == 1)
        isDWConv = vx_true_e;

    return isDWConv;
}

/*requantize per-channel weight to per-tensor weight*/
VX_INTERNAL_API vx_status vxoGraphOptimization_pcq(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        vx_tensor weight = NULL;
        vx_tensor bias = VX_NULL;

        /*judge whehter to do this feature by the threshold*/
        vx_float32 quantizeEntropy = 0.0f;
        vx_float32 threshold = CONV_ENTROPY;
        vx_float32  *weightData = VX_NULL;
        vx_uint32   weightSize  = 0;

        if(node->kernel->enumeration != VX_KERNEL_CONVOLUTION_LAYER)
            continue;

        weight = (vx_tensor)node->paramTable[1];
        if(weight->quantFormat != VX_QUANT_AFFINE_SCALE_PER_CHANNEL ||
            gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT))
            continue;
        if(TENSOR_DATA_LIFETIME(weight) != VX_TENSOR_LIFE_TIME_STATIC)
            continue;
        if(vxoGraphOptimization_isDepthWiseConv(node))
            threshold = DEPTHWISE_CONV_ENTROPY;

#ifdef __ANDROID__
        if(!vxoGraphOptimization_isV8((vx_reference)node))
            goto exit;
        if(VX_SUCCESS != vxoGraphOptimization_pcq_splitPerChannel(node))
            goto exit;
#else
        vxoGraphOptimization_pcq_resetDeadChannel(node);
        weightData = vxoGraphOptimization_pcq_getFpWeightData(weight);
        vxoTensor_GetTensorElementCount(weight, &weightSize);

        /* check whether or not to requantize the weight to per tensor by the entropy*/
        quantizeEntropy = vxoGraphOptimization_pcq_optimizeEntropy(weightData, weightSize);
        if(quantizeEntropy > threshold)
        {
            if(!vxoGraphOptimization_isV8((vx_reference)node))
                goto exit;
            if(VX_SUCCESS != vxoGraphOptimization_pcq_splitPerChannel(node))
                goto exit;
        }
        else
        {
            /* requantized to per tensor */
            vx_tensor newBias = VX_NULL;
            vx_tensor newWeight = vxoGraphOptimization_pcq_perTensor(weight, weightData);
            if(newWeight == VX_NULL)
                goto exit;
            TENSOR_DATA_LIFETIME(newWeight) = TENSOR_DATA_LIFETIME(weight);

            /*driver require bias' scale = input's scale * weight's scale*/
            bias = (vx_tensor)node->paramTable[2];
            if(bias)
            {
                vx_float32 newScale = TENSOR_TF_SCALE(newWeight) * TENSOR_TF_SCALE((vx_tensor)node->paramTable[0]);
                vx_tensor_create_params_t p = vxoGraphOptimization_createParamsForTensor(TENSOR_DIM_NUM(bias), TENSOR_SIZES(bias), VX_TYPE_INT32,
                                    VX_QUANT_AFFINE_SCALE, 0, 0, newScale);
                vx_uint32 biasCnt = 0, i = 0;

                newBias = vxCreateTensor2(node->base.context, &p, sizeof(p));
                TENSOR_DATA_LIFETIME(newBias) = TENSOR_DATA_LIFETIME(bias);

                vxoTensor_GetTensorElementCount(newBias, &biasCnt);
                vxoTensor_AllocateMemory(newBias);
                for(i = 0; i < biasCnt; i++)
                {
                    vx_float32 biasValue = vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(bias), i,
                        TENSOR_LOGICAL_ADDR(bias), TENSOR_TF_ZEROPOINTS_WITH_INDEX(bias, i), TENSOR_TF_SCALES_WITH_INDEX(bias, i));
                    vxnneSaveDataQuant((vx_type_e)TENSOR_DATA_TYPE(newBias), i, biasValue,
                        TENSOR_LOGICAL_ADDR(newBias), TENSOR_TF_ZEROPOINT(newBias), TENSOR_TF_SCALE(newBias), TENSOR_ROUNDING_MODE(newBias));
                }
            }

            vxoNode_SetParameter(node, 1, (vx_reference)newWeight);
            vxoNode_SetParameter(node, 2, (vx_reference)newBias);

            if(newBias) vxReleaseTensor(&newBias);
            if(newWeight) vxReleaseTensor(&newWeight);
        }
#endif
exit:
        if(weightData) vxFree(weightData);
    }
    REMOVE_MERGED_NODE_FROM_GRAPH();
    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)graph );


    gcmHEADER_ARG("graph=%p", graph);

    REBUILD_TOPOLOGY_GRAPH();

    if(context->options.enableGraphTranform)
    {
        if(context->options.enableGraphDump)
            vxmONERROR(vxoGraphOptimization_dumpTopology(graph, "before_optimization_topology.json"));

        if(context->options.enableGraphPtc)
            vxoGraphOptimization_pcq(graph);

        if(context->options.enableGraphConvertTensorAdd)
            vxoGraphOptimization_TensorAdd2Conv(graph);

        if(context->options.enableGraphEltwiseOpShape)
            vxoGraphOptimization_eltwiseOp(graph);

        if(context->options.enableGraphAvgPoolandPWConv)
            vxoGraphOptimization_avgPoolAnd1x1Conv(graph);

        if(context->options.enableGraphConvertAvgPool2Conv)
            vxoGraphOptimization_ConvertAvgPool2Conv(graph);

        if(context->options.enableGraphUnrollDWConv)
            vxoGraphOptimization_unrollDWConv(graph);

        if(context->options.enableGraphConvertConv2Fc)
            vxoGraphOptimization_conv2fc(graph);

        if(context->options.enableTransformNMConv)
            vxoGraphOptimization_transformConvNxM(graph);

        if(context->options.enableGraphSwaplayer)
            vxmONERROR(vxoGraphOptimization_LayerSwaping(graph));

        if(context->options.enableGraphPadConv)
            vxoGraphOptimization_padConv(graph);

        if(context->options.enableGraphWar1x1x1weight)
            vxoGraphOptimization_war1x1x1weight(graph);

        if(context->options.enableGraphMerge)
            vxmONERROR(vxoGraphOptimization_LayerMerge(graph));

        if(context->options.enableGraphDeleteSqueeze)
            vxoGraphOptimization_deleteSqueeze(graph);

        if(context->options.enableGraphDeleteRelu)
            vxoGraphOptimization_deleteRelu(graph);

        if(context->options.enableGraphConvertBatchFC2NNConv)
            vxmONERROR(vxoGraphOptimization_ConvertBatchFCtoConv(graph) );

        if(context->options.enableGraphMergeTranspose)
            vxoGraphOptimization_multiTranspose(graph);

        if(context->options.enableGraphConcalayer)
            vxmONERROR(vxoGraphOptimization_DispelConcat(graph));

        if(context->options.enableGraphReshapelayer)
            vxmONERROR(vxoGraphOptimization_DeleteReshape(graph));

        if(context->options.enableGraphWAR7)
            vxmONERROR(vxoGraphOptimization_WAR7(graph));
    }

    if(vxoGraphOptimization_isV8((vx_reference)graph))
        vxoGraphOptimization_splitMaxpFromCRL2(graph);

    if(context->options.enableGraphDump)
        vxmONERROR(vxoGraphOptimization_dumpTopology(graph, "final_graph_topology.json"));
OnError:
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

