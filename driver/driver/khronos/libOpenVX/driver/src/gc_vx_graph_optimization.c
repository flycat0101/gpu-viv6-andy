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
#include <gc_vx_nn_util.h>
#include <gc_vx_nn_wb.h>
#include <gc_vx_nn_encoder.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_GRAPH
#define QUANT_BIT_WIDTH         (8)

static vx_uint32 optPhase = 1;

extern vx_int16 Fp32toBF16(vx_float32 val);

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
        if (context->nnConfig.fixedFeature.nnCoreCountInt8 > 0)
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
    default:
        break;
    }

    gcmFOOTER_ARG("0x%x", nnSupportFormat);
    return nnSupportFormat;
}

VX_PRIVATE_API void vxoGraphOptimization_getQuantizeParam(float dMax, float dMin, float *scale, vx_int32 *zp)
{
    *scale = dMin> 0 ? dMax/ 255 :  (dMax - dMin)/ 255;
    *zp = (vx_int32)gcmMIN(255, gcmMAX(0, roundRTNE(0 - dMin/ *scale)));
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

VX_PRIVATE_API vx_tensor vxoGraphOptimization_cloneTensor(vx_tensor tensor, vx_graph graph)
{
    vx_tensor cloneTensor = NULL;
    vx_tensor_create_params_t p;

    gcmHEADER_ARG("tensor=%p", tensor);

    p = vxoGraphOptimization_cloneParamsFromTensor(tensor);

    if(vxoTensor_IsVirtualTensor(tensor))
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

            vx_tensor inputTensor = (vx_tensor)node->paramTable[0];

            weightX = vxoGraphOptimization_computeFinalKernelSize(weightX, strideX);
            weightY = vxoGraphOptimization_computeFinalKernelSize(weightY, strideY);

            if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1) &&
                (TENSOR_SIZE_INDEX(inputTensor,0) > NN_IMAGE_XSIZE_MAX || TENSOR_SIZE_INDEX(inputTensor, 1) > NN_IMAGE_YSIZE_MAX) )
                break;

            if(VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME(weight) &&
                 (node->paramTable[2] != VX_NULL ? VX_TENSOR_LIFE_TIME_STATIC == TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[2]) : vx_true_e) &&
                 weightX <= 15 && weightY <= 15 &&
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
                        (TENSOR_SIZE_INDEX(weight, 0) != 1 || TENSOR_SIZE_INDEX(weight, 1) != 1 )
                        )
                    {
                        nodeOpType = OP_CONVOLUTION;
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
            vx_tensor output = (vx_tensor)node->paramTable[PARAM_POOLING_OUTPUT_INDEX];
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


            vx_int32 stride_x = (vx_int32)roundRTNE((vx_float32)(input_w + pad[0] + pad[1] - kernelx)/(output_w == 1? 1: output_w -1));
            vx_int32 stride_y = (vx_int32)roundRTNE((vx_float32)(input_h + pad[2] + pad[3]- kernely)/(output_h == 1? 1: output_h-1));
            stride_x = stride_x == 0 ? 1: stride_x;
            stride_y = stride_y == 0 ? 1: stride_y;
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
                    else if(((TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE && TENSOR_DATA_TYPE(input) == VX_TYPE_UINT8) ||
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
            else if(poolType == VX_NN_POOLING_AVG)
            {
                /*TODO: whether to convert VX_NN_POOLING_AVG_ANDROID to CONV*/
                vx_enum roundMode = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_ROUND_MODE_INDEX], u32);
                if(TENSOR_DATA_TYPE(input) != TENSOR_DATA_TYPE(output))
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
                vx_uint32 batch = 1;
                if(2 == TENSOR_DIM_NUM(input) )
                    batch = TENSOR_SIZE_INDEX(input, 1);
                else
                    batch = TENSOR_SIZE_INDEX(input, 3);

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
                vx_uint32 batch = 1;
                if(2 == TENSOR_DIM_NUM(input) )
                    batch = TENSOR_SIZE_INDEX(input, 1);
                else
                    batch = TENSOR_SIZE_INDEX(input, 3);

                if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1) &&
                    batch > NN_IMAGE_XSIZE_MAX )
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

VX_PRIVATE_API vx_tensor vxoGraphOptimization_reshapeTensorAsOld(vx_tensor oldTensor, vx_tensor newTensor)
{
    vx_tensor tmpTensor = newTensor;
    if(!vxoGraphOptimization_isSameShapeTensor(oldTensor, newTensor))
    {
        tmpTensor = vxReshapeTensor(newTensor, (vx_int32 *)TENSOR_SIZES(oldTensor), TENSOR_DIM_NUM(oldTensor));
        tmpTensor->reshape = oldTensor;
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
    if return false, it do not find the related tensor's index.
*/
VX_PRIVATE_API vx_bool vxoGraphOptimization_matchTensorInNode(vx_node node, vx_tensor tensor, vx_uint32 *index)
{
    vx_uint32 k = 0;
    for(k = 0; k < node->numParameters; k++)
    {
        if(vxoReference_HasWriteDependency(node->paramTable[k], (vx_reference)tensor))
        {
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
        gcoOS_PrintStrSafe(buf, sizeof(buf), &offset, "@id_%d:out0", node->graph->nodeTable[ node->parentNodes[i] ]->nodeID);
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

        gcoOS_PrintStrSafe(layerName, sizeof(layerName), &offset, "id_%d", currentNode->nodeID);

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
        maxpoolOutput  = (vx_tensor)maxPoolNode->paramTable[maxPoolNode->numParameters - 1];

        {
            vx_tensor intermediaTensor = vxoGraphOptimization_cloneTensor(maxpoolOutput, graph);
            if(NULL == intermediaTensor)
                continue;

            intermediaTensor->base.isVirtual = vx_true_e; // set intermedia Tensor to b be virtual.

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
        stride[0]                   = WB_STRIDE_X(wb);
        stride[1]                   = WB_STRIDE_Y(wb);
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
                                                                                     vx_tensor weight, vx_tensor bias, vx_tensor prelu_alpha)

{
    vx_weights_biases_parameter wb = VX_NULL;
    vx_weights_biases_parameter_optimizations_ext_t opt = {-1, TENSOR_DATA_TYPE(finalOutput),
        TENSOR_TF_ZEROPOINT(input), TENSOR_DIM_NUM(input), TENSOR_DIM_NUM(finalOutput) };
    if(VX_NULL != prelu_alpha)
    {
        wb = vxoCreateWeightsBiasesParameterFromTensorsPRelu(layer_type,
            TENSOR_SIZES(input), TENSOR_SIZES(convOutput), TENSOR_SIZES(finalOutput),
            (vx_nn_convolution_relu_pooling_params_t *)wb_params, sizeOfParms,
        (vx_weights_biases_parameter_optimizations_t *)&opt, sizeof(opt),
        weight, bias, prelu_alpha);
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
                                                               vx_tensor prelu_alpha)
{
    vx_node node;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar vxPadConst = vxCreateScalar(context, VX_TYPE_UINT32, &pad_const);

    vx_nn_convolution_relu_pooling_params_ext2_t wb_params =
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
    };

    vx_weights_biases_parameter wb = vxoGraphOptimization_CreateWBParameter(
        VX_NN_CONVOLUTION_LAYER,
        (vx_nn_convolution_relu_pooling_params_t *)&wb_params,
        sizeof(wb_params),
        input, convOut, finalOutput, weight, bias, prelu_alpha);
    CHECK_NULL(wb);

    if(depth_multiplier == 1)
        node = vxConvolutionReluLayer(graph, input, wb, pad[0], pad[2], 0, overflow_policy, rounding_policy, down_scale_size_rounding, enable_relu, finalOutput);
    else
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
                    lastNodeOutputIndex = PARAM_RELU_OUTPUT_INDEX;
                    lastNode = i;
                    reluOutputTensor = (vx_tensor)nodes[i]->paramTable[PARAM_RELU_OUTPUT_INDEX];
                    break;
                }
            case VX_KERNEL_NN_PRELU:
                {
                    if(gcvSTATUS_TRUE != gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU))
                        break;

                    prelu_alpha = (vx_tensor) nodes[i]->paramTable[1];
                    lastNodeOutputIndex = nodes[i]->numParameters - 1;
                    reluOutputTensor = (vx_tensor)nodes[i]->paramTable[lastNodeOutputIndex];
                    lastNode = i;
                    break;
                }
            case VX_KERNEL_NN_POOLING_LAYER2:{
                vx_bool diff = vx_false_e;
                vx_uint32 idx = 0;
                vx_tensor maxpInput = (vx_tensor)nodes[i]->paramTable[0];
                vx_enum data_type = TENSOR_DATA_TYPE((vx_tensor)nodes[i]->paramTable[PARAM_POOLING_OUTPUT_INDEX]);
                if(data_type == VX_TYPE_UINT16 || data_type == VX_TYPE_INT16)
                    break;
                if(int16_check && data_type == VX_TYPE_FLOAT16)
                    break;
                if(depth_multiplier == 1 )
                    break;
                if(!vxoGraphOptimization_isSameShapeTensor(convOutputTensor, maxpInput))
                    break;

                for(idx = 0; idx < TENSOR_DIM_NUM(maxpInput); idx ++)
                    if(TENSOR_SIZE_INDEX(maxpInput, i) != TENSOR_SIZE_INDEX(convOutputTensor, i))
                        diff = vx_true_e;
                if(diff) break;

                /*it is different, depending on the hal info*/
                if(context->nnConfig.fixedFeature.nnInputBufferDepth)
                    if(context->nnConfig.fixedFeature.nnInputBufferDepth - ceilf((float)weight->dims[1]/ stride[1]) + 1 < SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32))
                        break;

                    pool_type = VX_NN_POOLING_MAX;
                    pool_size[0] = SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
                    pool_size[1] = SCALAR_VALUE(nodes[i]->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
                    lastNodeOutputIndex = PARAM_POOLING_OUTPUT_INDEX;
                    lastNode = i;
                    break;
                }
            default:
                vxError("do not merge no unified conv node\n");
                return VX_SUCCESS;
        }
    }/*for(i = 0; i < nodeCount; i++)*/


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
    newNode = vxoGraphOptimization_transformConv(nodes[0]->graph, inputTensor, convOutputTensor,
                                                    finalOutTensor, weight, bias, enable_relu,
                                                    dilation, stride, pad,accumulator_bits,
                                                    pool_size, pad_const, pool_type, pad_mode,
                                                    overflow_policy, rounding_policy,
                                                    down_scale_size_rounding, depth_multiplier, prelu_alpha);
    CHECK_NULL(newNode);
    vxReleaseNode(&newNode);

    if(!vxoGraphOptimization_isSameShapeTensor(finalOutTensor, (vx_tensor)nodes[lastNode]->paramTable[lastNodeOutputIndex]) )
    {
        CHECK_STATUS(vxReleaseTensor(&finalOutTensor) );
    }

    for(i = lastNode; i >=0; i--)
        nodes[i]->merged = vx_true_e;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_node vxoGraphOptimization_TransferFC2FCRelu(vx_node FCnode)
{
    vx_node     FCReluNode = VX_NULL;
    vx_uint32   pad;
    vx_uint8    acc;
    vx_uint32   rounding, overflow, down_scale_round;
    vx_tensor   input = (vx_tensor)FCnode->paramTable[0];
    vx_tensor   weight = (vx_tensor)FCnode->paramTable[1];
    vx_tensor   bias   = (vx_tensor)FCnode->paramTable[2];
    vx_tensor   output = (vx_tensor)FCnode->paramTable[FCnode->numParameters - 1];

    gcmHEADER_ARG("FCnode=%p", FCnode);

    vxoNode_getInfoFromFCNode(FCnode, &pad, &acc, &rounding, &overflow, &down_scale_round);
    {
        vx_uint32 padValue = 0;
        vx_scalar padConst = vxCreateScalar(((vx_reference)FCnode)->context, VX_TYPE_UINT32, &padValue);
        vx_nn_convolution_relu_pooling_params_ext2_t params = {
                {
                        { 0, 0,
                          pad, pad, pad, pad, acc, overflow,
                          rounding, down_scale_round,
                          vx_false_e, 0, 0, 0, VX_PAD_CONSTANT, padConst},
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
                                            input, output, output, weight, bias, VX_NULL);
        CHECK_NULL(wb);
        FCReluNode = vxFullyConnectedReluLayer(FCnode->graph, input, wb,
            pad,
            acc,
            overflow,
            rounding,
            down_scale_round,
            vx_false_e,
            output
        );

        vxReleaseScalar(&padConst);
        vxReleaseWeightsBiasesParameter(&wb);
    }
    gcmFOOTER_ARG("FCReluNode=%p", FCReluNode);
    return FCReluNode;
}


VX_INTERNAL_API vx_status vxoGraphOptimization_MergeFullyConnectedNodes(vx_node nodes[], vx_uint32 nodeCount)
{
    vx_bool newNodeflag = vx_false_e;
    gcmHEADER_ARG("nodes=%p, nodeCount=0x%x", nodes, nodeCount);

    if(!vxnneIsTPSupportFormat(
                    nodes[0]->base.context,
                    (vx_tensor)nodes[0]->paramTable[0],
                    VX_NULL,
                    (vx_tensor)nodes[0]->paramTable[nodes[0]->numParameters -1] )
        )
        return VX_SUCCESS;
    if(nodeCount == 1 && vxoGraphOptimization_getKernelType(nodes[0]) == OP_FULLYCONNECTED_RELU)
        return VX_SUCCESS;
    if(nodeCount == 2 && vxoGraphOptimization_getKernelType(nodes[1]) != OP_RELU)
        return VX_SUCCESS;

    if(VX_KERNEL_NN_FULLY_CONNECTED_LAYER == nodes[0]->kernel->enumeration ||
        VX_KERNEL_FULLY_CONNECTED_LAYER == nodes[0]->kernel->enumeration)
    {
        vx_node newNode = vxoGraphOptimization_TransferFC2FCRelu(nodes[0]);
        nodes[0]->merged = vx_true_e;
        nodes[0] = newNode;
        newNodeflag = vx_true_e;
    }

    /*replace fc's output with relu's output, but reshape it as fc's output*/
    if(nodeCount >1)
    {
        vx_tensor reluOut = (vx_tensor)nodes[1]->paramTable[PARAM_RELU_OUTPUT_INDEX];
        vxoGraphOptimization_updateTensorInNodeWithIndex(nodes, PARAM_FULLYCONNECTED_RELU_OUTPUT_INDEX, reluOut);

        SCALAR_VALUE(nodes[0]->paramTable[PARAM_FULLYCONNECTED_RELU_ENABLE_RELU_INDEX], b) = vx_true_e;
        nodes[1]->merged = vx_true_e;
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

    vx_node mergedNodes[3];
    vx_uint32 nodeCount;
    vx_uint32 nodeIndex = 0;
    vx_enum opType = OP_INVALID;
    vx_enum currNodeType;
    vx_enum features[][2] = {
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
    vx_tensor   tensorOut = (vx_tensor) node->paramTable[PARAM_CONCAT2_OUTPUT_INDEX];

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

VX_PRIVATE_API vx_tensor vxoGraphOptimization_CreateShareTensor(vx_tensor orginalTensor, vx_int32 *reshapeDims, vx_uint32 dimCount)
{
    vx_tensor reshapeTensor = vxReshapeTensor(orginalTensor, reshapeDims, dimCount);
    gcmHEADER_ARG("orginalTensor=%p, reshapeDims=%p, dimCount=0x%x", orginalTensor, reshapeDims, dimCount);
    orginalTensor->reshape = reshapeTensor;
    TENSOR_DATA_LIFETIME(reshapeTensor)  = TENSOR_DATA_LIFETIME(orginalTensor);
    TENSOR_PRECISION(reshapeTensor)      = TENSOR_PRECISION(orginalTensor);
    TENSOR_VALUED(reshapeTensor)         = TENSOR_VALUED(orginalTensor);
    reshapeTensor->reshape = orginalTensor;

    /*vxoReference_Increment((vx_reference)orginalTensor, VX_REF_INTERNAL);*/

    gcmFOOTER_ARG("reshapeTensor=%p", reshapeTensor);
    return reshapeTensor;
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
                                    convInputTensor, convOutputTensor, convOutputTensor, weight, orgBias, VX_NULL);

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

VX_INTERNAL_API vx_status vxoGraphOptimization_computeQuantAttribute(vx_enum quantType, vx_float32 maxValue, vx_float32 minValue,
                                                                 vx_int8 *fixedPointPos, vx_int32 *zeroPoint, vx_float32 * scale)
{
    gcmHEADER_ARG("quantType=%d, maxValue=%f, minValue=%f, fixedPointPos=%p, zeroPoint=%p, scale=%p",
        quantType, maxValue, minValue, fixedPointPos, zeroPoint, scale);
    if(quantType == VX_QUANT_AFFINE_SCALE)
    {
        vx_uint32 drange = 255;

        gcmASSERT(fixedPointPos);
        gcmASSERT(scale);

        maxValue = gcmMAX(maxValue, 0);
        minValue = gcmMIN(minValue, 0);
        *scale = (maxValue - minValue)/drange;
        *zeroPoint = gcmMIN(255, gcmMAX(0, (vx_int32)roundRTNE(0 - minValue/ *scale)));
    }
    else if(quantType == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        gcmASSERT(fixedPointPos);

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
    parm.data_format = TENSOR_DATA_TYPE(input);
    parm.num_of_dims = 4;
    parm.sizes = weight_dims;
    parm.quant_format = TENSOR_QUANT_TYPE(input);

    if((square & (square - 1) ) == 0 && VX_QUANT_DYNAMIC_FIXED_POINT == TENSOR_QUANT_TYPE(input))
    {
        parm.quant_data.dfp.fixed_point_pos = -1 * (vx_int8)gcoMATH_Log2(fill_data);
    }
    else
    {
        vxoGraphOptimization_computeQuantAttribute(TENSOR_QUANT_TYPE(input), fill_data, 0,
            &parm.quant_data.dfp.fixed_point_pos, &parm.quant_data.affine.zeroPoint, &parm.quant_data.affine.scale);
    }

    weight = vxCreateTensor2(context, &parm, sizeof(parm));

    if(TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE)
    {
        quantizedData = (vx_int16) roundRTNE(fill_data / TENSOR_TF_SCALE(weight) + TENSOR_TF_ZEROPOINT(weight));
    }
    else
    {
        if(TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16)
            quantizedData = Fp32toFp16(fill_data);
        else if(TENSOR_DATA_TYPE(input) == VX_TYPE_BFLOAT16)
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
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_AVG_POOL)
        {
            vx_tensor weight = VX_NULL;
            vx_tensor input = (vx_tensor)node->paramTable[0];
            vx_tensor output = (vx_tensor)node->paramTable[PARAM_POOLING_OUTPUT_INDEX];

            vx_uint32 kernel_x = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_X_INDEX], u32);
            vx_uint32 kernel_y = SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_SIZE_Y_INDEX], u32);
            vx_uint32 pads[] = {
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_L_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_X_R_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_T_INDEX],u32),
                SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_PAD_Y_B_INDEX],u32)
            };

            vx_uint32 weight_dims[] = {kernel_x, kernel_y, TENSOR_SIZE_INDEX(input, 2),1};

            vx_int32 stride_x = (TENSOR_SIZE_INDEX(input, 0) + pads[0] + pads[1] - kernel_x)/(TENSOR_SIZE_INDEX(output, 0)== 1? 1: TENSOR_SIZE_INDEX(output, 0) -1);
            vx_int32 stride_y = (TENSOR_SIZE_INDEX(input, 1) + pads[2] + pads[3] - kernel_y)/(TENSOR_SIZE_INDEX(output, 1)== 1? 1: TENSOR_SIZE_INDEX(output, 1)-1);

            stride_x = stride_x == 0? 1: stride_x;
            stride_y = stride_y == 0? 1: stride_y;

            if(!vxoGraphOptimization_nnHalSupport(input))
                continue;

            /*V8 support depwiseConv hardware feature*/
            if(vxoGraphOptimization_dwConvHalSupport(input))
            {
                vx_uint32 actual_x = stride_x > 1? vxoGraphOptimization_computeFinalKernelSize(kernel_x, stride_x) : kernel_x;
                vx_uint32 actual_y = stride_y > 1? vxoGraphOptimization_computeFinalKernelSize(kernel_y, stride_y): kernel_y;
                if(actual_x > 15 || actual_y>15)
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

                CHECK_NULL(vxConvolutionLayer(graph,
                    input,
                    weight,
                    VX_NULL,
                    (const vx_nn_convolution_params_t *)&params,
                    sizeof(vx_nn_convolution_params_ext2_t),
                    (vx_tensor)output));

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

    vxoGraphOptimization_getQuantizeParam(gcmMAX(fWeight[0], fWeight[1]), gcmMIN(fWeight[0], fWeight[1]), &scale, &zp);

    if(TENSOR_DATA_TYPE(*weight) == VX_TYPE_UINT8)
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
        vxoGraphOptimization_getQuantizeParam(gcmMAX(fWeight[0], fWeight[1]), gcmMIN(fWeight[0], fWeight[1]), &scale, &zp);
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

            /*do not process the head node*/
            if(node->numParents != 2 || tensorIn[0]->isViewed || tensorIn[1]->isViewed )
                continue;

            if(TENSOR_DATA_TYPE(tensorIn[0]) != TENSOR_DATA_TYPE(tensorIn[1]) ||
                TENSOR_DATA_TYPE(tensorIn[0]) != TENSOR_DATA_TYPE(output) )
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
                convInTensor = vxReshapeTensor(bigTensor, convDims, 3);
                CHECK_NULL(convInTensor);
                vxReleaseTensor(&bigTensor);

                convDims[2] /= 2;
                convOutTensor = vxReshapeTensor(output, convDims, 3);
                convOutTensor->reshape = output;
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
                                                        convInTensor, convOutTensor, convOutTensor, weight, bias, VX_NULL);
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

VX_PRIVATE_API vx_status vxoGraphOptimization_adjustFC_getChannelInfo(vx_node FCnode, vx_uint32 *c)
{
    vx_uint32 channel = 1;
    vx_tensor input = (vx_tensor)FCnode->paramTable[0];

    gcmHEADER_ARG("FCnode=%p, c=%p", FCnode, c);

    if(TENSOR_DIM_NUM(input) > 2)
        channel = TENSOR_SIZE_INDEX(input, 2);
    else if(TENSOR_DIM_NUM(input) == 1 || TENSOR_DIM_NUM(input) == 2)
    {
        if(FCnode->numParents == 1)
        {
            vx_node preNode = FCnode->graph->nodeTable[FCnode->parentNodes[0]];
            if(vxoGraphOptimization_getKernelType(preNode) == OP_RESHAPE)
            {
                channel = TENSOR_SIZE_INDEX((vx_tensor)FCnode->paramTable[0], 2);
            }
        }
    }
    *c = channel;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

/*adjust weight of FC, change the memory layout from NHWC to NCHW*/
VX_PRIVATE_API vx_status vxoGraphOptimization_adjustFC_reshuffleWeightAndUpdateNode(vx_node *FCnode, vx_uint32 channel)
{
    vx_tensor weight        = (vx_tensor)(*FCnode)->paramTable[1];
    vx_uint32 outChannel    = TENSOR_SIZE_INDEX(weight,0);
    vx_uint32 size          = TENSOR_SIZE_INDEX(weight,1);

    gcmHEADER_ARG("FCnode=%p, channel=0x%x", FCnode, channel);
    if(channel > 1)
    {
        vx_uint32 i, j, n;
        vx_ptr orgData          = (vx_ptr)vxAllocateAndZeroMemory(size * outChannel *TENSOR_DATA_SIZE(weight));
        vx_ptr reshuffledData   = (vx_ptr)vxAllocateAndZeroMemory(size * outChannel *TENSOR_DATA_SIZE(weight));

        vxoGraphOptimization_copyConstData2tensor(orgData, &weight, VX_READ_ONLY);


        for(n = 0; n < outChannel; n++)
        for(i = 0; i < channel; i++)
            for(j = 0; j < size/channel; j++)
            {
                switch (TENSOR_DATA_SIZE(weight))
                {
                case 1:
                    ((vx_uint8 *)reshuffledData)[n * size + i * size/channel + j] = ((vx_uint8 *)orgData)[n * size + j * channel + i];
                    break;
                case 2:
                    ((vx_uint16 *)reshuffledData)[n * size + i * size/channel + j] = ((vx_uint16 *)orgData)[n * size + j * channel + i];
                    break;
                case 4:
                    ((vx_uint32 *)reshuffledData)[n * size + i * size/channel + j] = ((vx_uint32 *)orgData)[n * size + j * channel + i];
                    break;
                default:
                    break;
                }
            }

        vxoGraphOptimization_copyConstData2tensor(reshuffledData, &weight, VX_WRITE_ONLY);

        if(orgData)         vxFree(orgData);
        if(reshuffledData)  vxFree(reshuffledData);
    }
    {
        vx_uint32 whcnWeightDims[2] = {size, outChannel};
        vx_tensor whcnWight = vxReshapeTensor(weight, (vx_int32 *)whcnWeightDims, 2);
        vxoNode_SetParameter(*FCnode, 1, (vx_reference)whcnWight);
        TENSOR_RANK(whcnWight) = VX_TENSOR_RANK_WHCN;
        vxReleaseTensor(&whcnWight);
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API void vxoGraphOptimization_adjustFC_adjustBias(vx_node *node)
{
    vx_tensor bias = (vx_tensor)(*node)->paramTable[2];
    if(bias)
    {
        TENSOR_RANK(bias) = VX_TENSOR_RANK_WHCN;
    }
}

VX_PRIVATE_API vx_status vxoGraphOptimization_adjustFC_adjustInput(vx_node *node)
{
    vx_tensor input     = (vx_tensor)(*node)->paramTable[0];
    vx_tensor weight    = (vx_tensor)(*node)->paramTable[1];
    vx_uint32 dotLen    = TENSOR_SIZE_INDEX(weight, 0);
    vx_uint32 dims[2]   = {dotLen,1};
    vx_uint32 dimNum    = 1;

    vx_uint32 size = 1, i = 0;

    gcmHEADER_ARG("node=%p", node);

    for(i = 0; i < TENSOR_DIM_NUM(input); i++)
        size *= TENSOR_SIZE_INDEX(input, i);

    dims[1] = size / dotLen;

    if(dims[1] > 1)
        dimNum  = 2;

    {
        vx_tensor newInput = vxReshapeTensor(input, (vx_int32*)dims, dimNum);
        TENSOR_RANK(newInput) = VX_TENSOR_RANK_WHCN;

        vxoNode_SetParameter(*node, 0, (vx_reference)newInput);
        vxReleaseTensor(&newInput);
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraphOptimization_adjustFC(vx_graph graph)
{
    vx_int32 nodeIndex;
    vx_int32 nodeCount = graph->nodeCount;
    vx_node* nodeTable = graph->nodeTable;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
        vx_node node = nodeTable[nodeIndex];
        if(vxoGraphOptimization_getKernelType(node) == OP_FC_ANDROID)
        {
            vx_uint32 channel = 1;

            /*fetch the channel info, reshuffle memory layout of weight and set new weight to node*/
            vxoGraphOptimization_adjustFC_getChannelInfo(node, &channel);
            vxoGraphOptimization_adjustFC_reshuffleWeightAndUpdateNode(&node, channel);

            /*adjust input dims*/
            vxoGraphOptimization_adjustFC_adjustInput(&node);

            vxoGraphOptimization_adjustFC_adjustBias(&node);
        }
    }

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
                (vx_uint32)stride[0], (vx_uint32)stride[1], 0
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
    return vxReshapeTensor(oldTensor, (vx_int32 *)reshapeDims, 2);
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
            vx_tensor convWeight    = (vx_tensor)node->paramTable[1];
            vx_tensor bias          = (vx_tensor)node->paramTable[2];
            vx_tensor convOutput    = (vx_tensor)node->paramTable[node->numParameters - 1];

            if(SCALAR_VALUE(node->paramTable[PARAM_CONV_DEPTH_MULTIPLIER_INDEX], u32) != 0)
                continue;

            if(SCALAR_VALUE(node->paramTable[PARAM_CONV_DILATION_INDEX], u32) != 0 ||
                SCALAR_VALUE(node->paramTable[PARAM_CONV_DILATION_INDEX + 1], u32) != 0 ||
                SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX], u32) != 1 ||
                SCALAR_VALUE(node->paramTable[PARAM_CONV_STRIDE_INDEX + 1], u32) != 1)
                continue;

            for(i = 0; i < 4; i++)
                if(SCALAR_VALUE(node->paramTable[PARAM_CONV_PAD_INDEX + i], u32) != 0)
                    diff = vx_true_e;

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
                                                    VX_CONVERT_POLICY_SATURATE,
                                                    VX_ROUND_POLICY_TO_ZERO,
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

    reshapeBias = vxReshapeTensor(bias, (vx_int32 *)reshapeDims, 2);
    CHECK_NULL(reshapeBias);
    for(i = 0; i < depth_mult; i++)
    {
        view_start[0] = i, view_end[0] = i+1;
        tmpTensor = vxCreateTensorFromView(reshapeBias, 2,view_start, view_end);
        CHECK_NULL(tmpTensor);

        unrollBias[i] = vxReshapeTensor(tmpTensor, (vx_int32 *)rollDims, 1);
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

    tmpTensor = vxReshapeTensor(tensor, (vx_int32 *)reshapeDims, 4);
    CHECK_NULL(tmpTensor);

    for(i = 0; i < depth_mult; i++)
    {
        view_start[2] = i;
        view_end[2] = i+1;
        tt = vxCreateTensorFromView(tmpTensor, 4, view_start, view_end);
        CHECK_NULL(tt);

        unrollTensor[i] = vxReshapeTensor(tt, (vx_int32 *)rollDims, 4);
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
                (TENSOR_SIZE_INDEX(dwweight, 0) != 1 || TENSOR_SIZE_INDEX(dwweight, 1) != 1) )
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
                if(TENSOR_QUANT_TYPE(dwweight) == VX_QUANT_AFFINE_SCALE)
                {
                    p.quant_data.affine.scale           = TENSOR_TF_SCALE(dwweight);
                    p.quant_data.affine.zeroPoint       = TENSOR_TF_ZEROPOINT(dwweight);
                }
                else
                {
                    p.quant_data.dfp.fixed_point_pos    = TENSOR_POS(dwweight);
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
                vx_int32  padV = TENSOR_QUANT_TYPE(padWeight) == VX_QUANT_AFFINE_SCALE ? TENSOR_TF_ZEROPOINT(padWeight) : 0;
                if(TENSOR_DATA_TYPE(padWeight) == VX_TYPE_INT8 ||
                    TENSOR_DATA_TYPE(padWeight) == VX_TYPE_UINT8 ||
                    TENSOR_QUANT_TYPE(padWeight) != VX_QUANT_AFFINE_SCALE
                    )
                    memset(dst, padV, TENSOR_STRIDE_INDEX(padWeight, 4));
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
            vx_uint32 *sameTranspose = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * node->numChildren);

            /*find the same transpose node*/
            for(i = 0; i < node->numChildren; i++)
            {
                if(vxoGraphOptimization_getKernelType(nodeTable[node->childNodes[i]]) == OP_TRANSPOSE)
                {
                    if(transposeDims[0] == 0xff)
                    {
                        vxMemCopy(transposeDims, ((vx_array)nodeTable[node->childNodes[i]]->paramTable[1])->memory.logicals[0],
                            sizeof(vx_uint32) * dimNum);

                        sameTranspose[childTcnt++] = node->childNodes[i];
                    }
                    else
                    {
                        vx_uint32 j = 0;
                        vx_uint32 *ptr = (vx_uint32 *)((vx_array)nodeTable[node->childNodes[i]]->paramTable[1])->memory.logicals[0];
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
                vx_tensor finalTensor = VX_NULL;
                for(i = 0; i < childTcnt; i++)
                {
                    vx_node child = nodeTable[sameTranspose[i]];
                    if(finalTensor == VX_NULL)
                        finalTensor = (vx_tensor)child->paramTable[child->numParameters- 1];

                    if(child->numChildren== 0)
                        finalTensor = (vx_tensor)child->paramTable[child->numParameters- 1];
                }

                /*TODO: how to choice the reversed node and delete rest of nodes*/
                for(i = 0; i < childTcnt; i++)
                {
                    vx_uint32 index = 0;
                    vx_node child = nodeTable[sameTranspose[i]];
                    if(child->numChildren)
                    {
                        vx_node graphchild = nodeTable[child->childNodes[0]];
                        if(graphchild->numParents > 1)
                        {
                            invalidChild ++;
                            continue;
                        }

                        if(vxoGraphOptimization_matchTensorInNode(graphchild, (vx_tensor)child->paramTable[child->numParameters - 1], &index))
                            vxoGraphOptimization_updateTensorInNodeWithIndex(&graphchild, index, finalTensor);

                        if(i != 0)
                            child->merged = vx_true_e;
                    }
                }
            }

            if(!invalidChild)
            {
                transposeNodes[0] = node;
                transposeCnt++;
                for(i = 0; i < childTcnt; i++)
                    transposeNodes[transposeCnt++] = nodeTable[sameTranspose[i]];
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
    clon a tensor for the fp32 tensor, the only difference is the data type.
    data type of the cloned tensor is fp16.
*/
VX_INTERNAL_API vx_tensor vxoGraphOptimization_convertFp32Tensor_clonFp16Tensor(vx_graph graph,vx_tensor fp32Tensor)
{
    vx_uint32   i = 0;
    vx_size     dims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_tensor   fp16Tensor = NULL;

    vxmASSERT(TENSOR_DATA_TYPE(fp32Tensor) == VX_TYPE_FLOAT32);

    for(i = 0; i < TENSOR_DIM_NUM(fp32Tensor); i++)
        dims[i] = TENSOR_SIZE_INDEX(fp32Tensor, i);

    if(vxoTensor_IsVirtualTensor(fp32Tensor))
    {
        fp16Tensor = vxCreateVirtualTensor(graph, TENSOR_DIM_NUM(fp32Tensor), dims, VX_TYPE_FLOAT16, 0);
    }
    else
    {
        fp16Tensor = vxCreateTensor(fp32Tensor->base.context, TENSOR_DIM_NUM(fp32Tensor), dims, VX_TYPE_FLOAT16, 0);
    }

    TENSOR_DATA_LIFETIME(fp16Tensor)    = TENSOR_DATA_LIFETIME(fp32Tensor);
    TENSOR_PRECISION(fp16Tensor)        = TENSOR_PRECISION(fp32Tensor);
    TENSOR_VALUED(fp16Tensor)           = TENSOR_VALUED(fp32Tensor);
    /*TENSOR_RANK(fp16Tensor)             = TENSOR_RANK(fp32Tensor);*/

    return fp16Tensor;
}

/*
    item 1. static tensor: create fp16 tensor and convert data from fp32 to fp16.
    item 2. virtual in/out tensor : replace with fp16 virtual tensor
*/
VX_INTERNAL_API vx_status vxoGraphOptimization_convertFp32Tensor(vx_graph graph)
{
    vx_uint32 i = 0, j = 0;

    if(vxoNode_CheckF32Support(graph->nodeTable[0]))
        return VX_SUCCESS;

    for(i = 0; i < graph->nodeCount; i++)
    {
        vx_node     node        = graph->nodeTable[i];
        vx_tensor   tensor32    = NULL;
        vx_uint32   paramIndex  = 0;

        for(paramIndex = 0; paramIndex < node->numParameters; paramIndex++)
        {
            vx_uint32 tensorCnt = 1;

            if(node->paramTable[paramIndex] == NULL)
                continue;

            if (node->paramTable[paramIndex]->type != VX_TYPE_TENSOR  && node->paramTable[paramIndex]->type != VX_TYPE_OBJECT_ARRAY)
                continue;

            /*maybe it is a image array*/
            if(node->paramTable[paramIndex]->type == VX_TYPE_OBJECT_ARRAY )
            {
                if(((vx_object_array)node->paramTable[paramIndex])->itemsTable[0]->type != VX_TYPE_TENSOR )
                    continue;
                else
                    tensorCnt = (vx_int32)((vx_object_array)node->paramTable[paramIndex])->itemCount;
            }

            for(j = 0; j < tensorCnt; j++)
            {
                vx_tensor   tensor16    = NULL;

                if(node->paramTable[paramIndex]->type == VX_TYPE_OBJECT_ARRAY)
                {
                    tensor32 = (vx_tensor)((vx_object_array)node->paramTable[paramIndex])->itemsTable[j];
                }
                else
                {
                    tensor32 = (vx_tensor)node->paramTable[paramIndex];
                }
                if(TENSOR_DATA_TYPE(tensor32) != VX_TYPE_FLOAT32 || TENSOR_PRECISION(tensor32) == VX_TENSOR_PRECISION_HIGH)
                    continue;

                /*item 1*/
                if(TENSOR_VALUED(tensor32) == vx_true_e)
                {
                    tensor16 = vxoGraphOptimization_convertFp32Tensor_clonFp16Tensor(graph, tensor32);
                    vxoTensor_AllocateMemory(tensor16);
                    vxnneAdapter_Tensor_FormatConvert(tensor32, tensor16);
                }

                /*do item2.*/
                if(node->kernel->signature.directionTable[paramIndex] == VX_INPUT &&
                    vxoTensor_IsVirtualTensor(tensor32)
                    )
                {
                    tensor16 = vxoGraphOptimization_convertFp32Tensor_clonFp16Tensor(graph, tensor32);
                }

                if(tensor16 != NULL)
                {
                    /*traverse all of child nodes for itme2*/
                    if(node->kernel->signature.directionTable[paramIndex] == VX_INPUT && vxoTensor_IsVirtualTensor(tensor32))
                    {
                        vxoGraphOptimization_updateTensorInGraph(node, &tensor32, &tensor16,1);
                    }
                    /*replace fp32 tensor with fp16 tensor*/
                    if(node->paramTable[paramIndex]->type == VX_TYPE_OBJECT_ARRAY)
                    {
                        ((vx_object_array)node->paramTable[paramIndex])->itemsTable[j] = (vx_reference)tensor16;
                    }
                    else
                    {
                        vxoNode_SetParameter(node, paramIndex, (vx_reference)tensor16);
                    }

                    vxReleaseTensor(&tensor16);
                }/*if(tensor16 != NULL)*/
            }/*for(j = 0; j < tensorCnt; j++)*/
        }/*for(paramIndex = 0; paramIndex < node->numParameters; paramIndex++)*/
    }/*for(i = 0; i < graph->nodeCount; i++)*/


    REBUILD_TOPOLOGY_GRAPH();
    OPTIMIZATION_RESLUT();
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

VX_PRIVATE_API vx_uint32 vxoEltwise_fill_dim
    (
    vx_uint32* shape_x, vx_uint32* shape_y,
    vx_uint32* shape_output, vx_uint32 rank,
    vx_uint32 max_rank, vx_uint32 size_x, vx_uint32 size_y,
    vx_uint32 size_output
    )
{
    vx_uint32 cost_size = 1;
    gcmASSERT(rank >= max_rank );
    if(size_output < VIV_TENSOR_MAX_WIDTH )
    {
        shape_x[rank] = size_x;
        shape_y[rank] = size_y;
        shape_output[rank] = size_output;
    }
    else
    {
        vx_uint32 divisor = 0;
        vx_uint32 remainder = 0;

        vxoGetDataDivisors(size_output, &divisor, 1 );
        remainder = size_output / divisor;
        if(remainder > VIV_TENSOR_MAX_WIDTH || rank >= max_rank )
        {
            // Cannot optimize.
            shape_x[rank] = size_x;
            shape_y[rank] = size_y;
            shape_output[rank] = size_output;
        }
        else
        {
            /*
             * We've limit the max size to 2^32 -1(Almost 4G * sizeof(data type)),
             * so it should be always 2.
             */
            cost_size = 2;
            if(size_x > 1 )
            {
                shape_x[rank]  = divisor;
                shape_x[rank + 1] = remainder;
            }
            else
            {
                shape_x[rank] = 1;
                shape_x[rank + 1] = 1;
            }
            if(size_y > 1 )
            {
                shape_y[rank]  = divisor;
                shape_y[rank + 1] = remainder;
            }
            else
            {
                shape_y[rank] = 1;
                shape_y[rank + 1] = 1;
            }
            shape_output[rank] = divisor;
            shape_output[rank + 1] = remainder;
        }
    }
    return cost_size;
}

VX_PRIVATE_API vx_bool vxoGraphOptimization_EltwiseTensorShapeOpt
    (
    vx_tensor input0,
    vx_tensor input1,
    vx_tensor output,
    vx_uint32 *out_shape_x,
    vx_uint32 *out_shape_y,
    vx_uint32 *out_shape_output,
    vx_uint32 *out_rank_output
    )
{
    vx_bool   ret                           = vx_true_e;
    vx_bool   append_dim                    = vx_false_e;
    vx_uint32 i                             = 0;
    vx_uint32 dims                          = 0;
    vx_uint32 effective_size_x              = 1;
    vx_uint32 effective_size_y              = 1;
    vx_uint32 tmp_sz                        = 0;
    vx_uint32 sx                            = 0;
    vx_uint32 sy                            = 0;
    vx_uint32 rank_x                        = TENSOR_DIM_NUM(input0);
    vx_uint32 rank_y                        = TENSOR_DIM_NUM(input1);
    vx_uint32 rank_output                   = TENSOR_DIM_NUM(output);
    vx_uint32 shape_x[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 shape_y[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 shape_output[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_eltwise_broadcast_state_e state      = VX_ELTWISE_BROADCAST_STATE_EMPTY;
    vx_eltwise_broadcast_state_e prv_state  = VX_ELTWISE_BROADCAST_STATE_EMPTY;


    for (i = 0; i < rank_x; i++)
    {
        shape_x[i] = TENSOR_VIEW_SIZE_INDEX(input0, i);
    }

    for (i = 0; i < rank_y; i++)
    {
        shape_y[i] = TENSOR_VIEW_SIZE_INDEX(input1, i);
    }

    for (i = 0; i < rank_output; i++)
    {
        shape_output[i] = TENSOR_VIEW_SIZE_INDEX(output, i);
    }
#define _swap_size(a, b, tmp)  \
    do { \
        tmp = a; \
        a = b; \
        b = tmp; \
    } while(0)
    for(i = 0; i < rank_output; i++ )
    {
        sx = i < rank_x ? shape_x[i] : 1;
        sy = i < rank_y ? shape_y[i] : 1;

        /*
         * Skip dim if the size is equal to 1
         * Also skip if(sx == 1 && sy == 1 )
         */
        if(shape_output[i] == 1 )
        {
            continue;
        }
        // Invalid shape for broadcasting
        if(sx != sy && sx > 1 && sy > 1 )
        {
            ret = vx_false_e;
            break;
        }
        // Update state
        state = VX_ELTWISE_BROADCAST_STATE_EMPTY;
        if(sx == sy )
        {
            state = VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST;
        }
        else if(sx == 1 )
        {
            state = VX_ELTWISE_BROADCAST_STATE_BROADCAST_X;
        }
        else if(sy == 1 )
        {
            state = VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y;
        }
        else
        {
            gcmASSERT(vx_false_e );
        }
        if(prv_state == VX_ELTWISE_BROADCAST_STATE_EMPTY )
        {
            effective_size_x *= sx;
            effective_size_y *= sy;
            prv_state = state;
            continue;
        }
        append_dim = vx_false_e;
#define _pack_state(prev_state, cur_state )    (prev_state << 16 | cur_state)
        switch(_pack_state(prv_state, state ) )
        {
            /*
             * ...,x1,x2,...
             * ...,y1,y2,...
             */
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST, VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST ):
                effective_size_x *= sx;
                effective_size_y *= sy;
                break;
            /*
             * ..., 1, 1,...
             * ...,y1,y2,...
             */
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_X, VX_ELTWISE_BROADCAST_STATE_BROADCAST_X ):
                effective_size_y *= sy;
                break;
            /*
             * ...,x1,x2,...
             * ..., 1, 1,...
             */
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y, VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y ):
                effective_size_x *= sx;
                break;

            /*
             * ...,x1, 1,...
             * ...,y1,y2,...
             *
             * ...,x1,x2,...
             * ...,y1, 1,...
             *
             * ..., 1,x2,...
             * ...,y1, 1,...
             *
             * ..., 1,x2,...
             * ...,y1,y2,...
             *
             * ...,x1, 1,...
             * ..., 1,y2,...
             *
             * ...,x1,x2,...
             * ..., 1,y2,...
             */
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST, VX_ELTWISE_BROADCAST_STATE_BROADCAST_X ):
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST, VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y ):
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_X, VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y ):
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_X, VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST ):
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y, VX_ELTWISE_BROADCAST_STATE_BROADCAST_X ):
            case _pack_state(VX_ELTWISE_BROADCAST_STATE_BROADCAST_Y, VX_ELTWISE_BROADCAST_STATE_NO_BROADCAST ):
                _swap_size(sx, effective_size_x, tmp_sz);
                _swap_size(sy, effective_size_y, tmp_sz);
                append_dim = vx_true_e;
                break;
            default:
                vxError("Get error state (%d -> %d) while computing broadcast shape.",
                        prv_state, state);
                break;
        }
#undef _pack_state
        prv_state = state;
        if(append_dim )
        {
            dims += vxoEltwise_fill_dim(out_shape_x, out_shape_y, out_shape_output,
                    dims, VX_CONTEXT_TENSOR_MAX_DIMENSION, sx, sy, gcmMAX(sx, sy ) );
        }
    }
    if(ret )
    {
        /* Append the last dim */
        if(i == rank_output )
        {
            sx = effective_size_x;
            sy = effective_size_y;
            dims += vxoEltwise_fill_dim(out_shape_x, out_shape_y, out_shape_output,
                    dims, VX_CONTEXT_TENSOR_MAX_DIMENSION, sx, sy, gcmMAX(sx, sy ) );
        }
        /* Avoid 1D shape*/
        if(1 == dims )
        {
            out_shape_x[1] = 1;
            out_shape_y[1] = 1;
            out_shape_output[1] = 1;
            dims = 2;
        }

        *out_rank_output = dims;
    }
#undef _swap_size
    return ret;
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
            vx_bool ret = vx_true_e;
            vx_tensor tensorIn[2]   = {(vx_tensor)node->paramTable[0], (vx_tensor)node->paramTable[1]};
            vx_tensor output        = (vx_tensor)node->paramTable[node->numParameters - 1];
            vx_uint32 inDims0[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 inDims1[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 outDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 dimcnt = 0;

            ret = vxoGraphOptimization_EltwiseTensorShapeOpt(tensorIn[0], tensorIn[1], output, inDims0, inDims1, outDims, &dimcnt);

            if (ret)
            {
                vx_tensor newinput0 = vxoTensor_ReshapeTensor(tensorIn[0], (vx_int32*)inDims0, dimcnt);
                vx_tensor newinput1 = vxoTensor_ReshapeTensor(tensorIn[1], (vx_int32*)inDims1, dimcnt);
                vx_tensor newoutput = vxoTensor_ReshapeTensor(output, (vx_int32*)outDims, dimcnt);
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
            nodeTable[node->parentNodes[0]]->numChildren == 1
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

            if(dataType != VX_TYPE_UINT8 && dataType != VX_TYPE_UINT16)
                continue;
            if(TENSOR_QUANT_TYPE(reluOut) != VX_QUANT_AFFINE_SCALE)
                continue;

            /*add blacklist, in which nodes do not requantize data*/
            for(i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); i++)
            {
                if(child->kernel->enumeration == blacklist[i] || parent->kernel->enumeration == blacklist[i])
                    break;
            }
            if(i != sizeof(blacklist)/sizeof(blacklist[0]))
                continue;

            /*update the quantized teensor is saturated as relu-type*/
            switch (nodeType)
            {
            case OP_RELU:
             {
                 min = 0;
                 zp = 0;
                break;
             }
            case OP_RELU1:
             {
                max = max > 1.0f ? 1.0f : max;
                min = min < -1.0f ? -1.0f : gcmMIN(0, min);

                scale = (max - min)/255;
                zp = (vx_int32) ((0 -min)/ scale );
                break;
            }
            case OP_RELU6:
                {
                    max = max > 6.0f ? 6.0f : max;
                    min = 0;
                    zp = 0;
                    scale = (max - min) / 255;
                    break;
                }
            }

            /*if quantization attribute is changed, and then the child's input has to be update.
                what's more, if the child node is conv node, the bias's scale has to be updated so that
                bais's scale = input's scale * weight's scale, which is required*/
            if(TENSOR_TF_SCALE(reluOut) != scale || TENSOR_TF_ZEROPOINT(reluOut) != zp)
            {
                vx_tensor weight = NULL, bias = NULL;
                TENSOR_TF_SCALE(reluOut) = scale;
                TENSOR_TF_ZEROPOINT(reluOut) = zp;

                vxoGraphOptimization_updateTensorInNodeWithIndex(&child, 0, reluOut);
                if(child->kernel->enumeration == VX_KERNEL_CONVOLUTION_LAYER)
                {
                    weight  = (vx_tensor)node->paramTable[1];
                    bias    = (vx_tensor)node->paramTable[1];
                }else if(child->kernel->enumeration == VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2 ||
                    child->kernel->enumeration == VX_KERNEL_NN_CONVOLUTION_RELU_LAYER)
                {
                    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)child->paramTable[1];

                    weight  = WB_WEIGHT_TENSOR(wb);
                    bias    = WB_BIAS_TENSOR(wb);

                    WB_BIAS_SCALE(wb) = scale * TENSOR_TF_SCALE(weight);
                    WB_INPUT_ZP(wb) = TENSOR_TF_ZEROPOINT(reluOut);
                }

                if(bias)
                {
                    TENSOR_TF_SCALE(bias) = TENSOR_TF_SCALE(reluOut) * TENSOR_TF_SCALE(weight);
                }
            }

            vxoGraphOptimization_updateTensorInGraph(node, &reluIn, &reluOut, 1);
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

VX_INTERNAL_API vx_status vxoGraphOptimization(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxGetContext((vx_reference)graph );


    gcmHEADER_ARG("graph=%p", graph);

    REBUILD_TOPOLOGY_GRAPH();
    vxoGraphOptimization_convertFp32Tensor(graph);
    vxoGraphOptimization_adjustFC(graph);

    if(context->options.enableGraphTranform)
    {
        if(context->options.enableGraphDump)
            vxmONERROR(vxoGraphOptimization_dumpTopology(graph, "before_optimization_topology.json"));

        if(context->options.enableGraphConvertTensorAdd)
            vxoGraphOptimization_TensorAdd2Conv(graph);

        if(context->options.enableGraphEltwiseOpShape)
            vxoGraphOptimization_eltwiseOp(graph);

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

