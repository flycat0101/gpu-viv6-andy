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



#ifndef __GC_VX_GRAPH_OPTIMIZATION_H__
#define __GC_VX_GRAPH_OPTIMIZATION_H__

EXTERN_C_BEGIN

#define VIV_TENSOR_MAX_WIDTH 65536

#define ENABLE_GRAPH_WAR7_16x16 1

#define CONVERT_CONV_TYPE(type) {\
            nodeOpType = OP_CONVOLUTION;\
            if (SCALAR_VALUE(node->paramTable[type##_ENABLE_RELU_INDEX], b) == vx_true_e )\
            {\
                nodeOpType = (node_op_type_e)(nodeOpType | OP_RELU);\
            }\
                \
            if ((SCALAR_VALUE(node->paramTable[type##_POOL_TYPE_INDEX], u32) == VX_NN_POOLING_MAX ) &&\
                 (SCALAR_VALUE(node->paramTable[type##_POOL_SIZE_X_INDEX], u32) != 0) )\
            {\
               nodeOpType = (node_op_type_e)(nodeOpType | OP_POOLING);\
            }\
}

    #define MAX_MERGE_OPS 3

#define SCALAR_VALUE(scalar, Type) (((vx_scalar)(scalar))->value->Type)

#define CHECK_NULL(v) {\
    if(v == NULL){\
        vxError("create fail\n");\
        vxmASSERT(0);\
    }\
}

#define OPTIMIZATION_RESLUT()     {\
    vx_context context = vxGetContext((vx_reference)graph);\
    if(context->options.enableGraphDump)\
    {\
        char logName[100] = {0};\
        const char *ptr = __FUNCTION__ + sizeof("vxoGraph_Optimization_") - 1;\
        sprintf(logName,"%s_%s_%s", "after",ptr,"graph.json");    \
        vxoGraph_Optimization_dumpTopology(graph, logName);\
    }\
}

#define CHECK_STATUS(status) {\
    if((status) != VX_SUCCESS )\
    {\
        fprintf(stderr, "status error, line: %d, file:%s\n", __LINE__, __FILE__);\
        assert(0);\
    }\
}

#define REBUILD_TOPOLOGY_GRAPH() {\
    CHECK_STATUS(vxoGraph_DetectAllHeadNodes(graph));\
    CHECK_STATUS(vxoGraph_RetrieveTopology(graph) );\
    CHECK_STATUS(vxoGraph_DetectAllTailNodes(graph) );\
}

#define REMOVE_MERGED_NODE_FROM_GRAPH() {\
    for (nodeIndex = nodeCount - 1; nodeIndex >= 0; nodeIndex--){\
        vx_node node = graph->nodeTable[nodeIndex];\
        if (node->merged) {\
            vxoNode_RemoveFromGraph(&node);\
        }\
    }\
}

#define GET_TENSOR_BATCH(tensor) (TENSOR_DIM_NUM(tensor) == 2 ? TENSOR_SIZE_INDEX(tensor, 1) : TENSOR_SIZE_INDEX(tensor,3))

#define GET_HW_FEATURE_ACCUM_BUf_SIZE(ref)      (((vx_reference)(ref) )->context->nnConfig.fixedFeature.nnAccumBufferDepth)
#define GET_HW_FEATURE_INPUT_BUF_SIZE(ref)    (((vx_reference)(ref) )->context->nnConfig.fixedFeature.nnInputBufferDepth)
#define GET_HW_FEATURE_PAD_BIT_SIZE(ref)    (((vx_reference)(ref) )->context->nnConfig.fixedFeature.nnInImageOffsetBits)


typedef enum _node_op_type_e
{
    OP_INVALID                  = 0,
    OP_CONVOLUTION              = 0x1,
    OP_FULLYCONNECTED           = 0x1<<1,
    OP_RELU                     = 0x1<<2,
    OP_POOLING                  = 0x1<<3,
    OP_CONCAT                   = 0x1<<4,
    OP_RESHAPE                  = 0x1<<5,
    OP_AVG_POOL                 = 0x1<<6,
    OP_ADD_SUB                  = 0x1<<7,
    OP_FC_ANDROID               = 0x1<<8,
    OP_CONVOLUTION_NxM          = 0x1<<9,
    OP_CONVOLUTION_DW           = 0x1<<10,
    OP_TRANSPOSE                = 0x1<<11,
    OP_ROIPOOL                  = 0x1<<12,
    OP_CONVOLUTION_PAD          = 0x1<<13,
    OP_ELTWISE_ASMD             = 0x1<<14, /*broadcast add/sub/mul/div*/
    OP_CONVOLUTION_RELU         = OP_CONVOLUTION | OP_RELU,
    OP_FULLYCONNECTED_RELU      = OP_FULLYCONNECTED | OP_RELU,
    OP_CONVOLUTION_POOLING      = OP_CONVOLUTION | OP_POOLING,
    OP_CONVOLUTION_RELU_POOLING = OP_CONVOLUTION | OP_RELU | OP_POOLING,
    OP_ROIPOOL_RELU             = OP_ROIPOOL | OP_RELU,
    OP_MAX,
} node_op_type_e;

typedef enum _param_conv_relu_pooling_2_index_e
{
    PARAM_CONV_RELU_POOLING_2_WEIGHTED_BIAS_INDEX = 1,
    PARAM_CONV_RELU_POOLING_2_DILATION_INDEX = 2,
    PARAM_CONV_RELU_POOLING_2_PAD_INDEX = 4,
    PARAM_CONV_RELU_POOLING_2_ACCUMULATOR_BITS_INDEX = 8,
    PARAM_CONV_RELU_POOLING_2_OVERFLOW_INDEX = 9,
    PARAM_CONV_RELU_POOLING_2_ROUNDING_INDEX = 10,
    PARAM_CONV_RELU_POOLING_2_DOWN_SCALE_SIZEROUNDING_INDEX = 11,
    PARAM_CONV_RELU_POOLING_2_ENABLE_RELU_INDEX = 12,
    PARAM_CONV_RELU_POOLING_2_POOL_TYPE_INDEX = 13,
    PARAM_CONV_RELU_POOLING_2_POOL_SIZE_X_INDEX = 14,
    PARAM_CONV_RELU_POOLING_2_POOL_SIZE_Y_INDEX = 15,
    PARAM_CONV_RELU_POOLING_2_PAD_MODE_INDEX = 16,
    PARAM_CONV_RELU_POOLING_2_PAD_CONST_INDEX = 17,
    PARAM_CONV_RELU_POOLING_2_OUTPUT_INDEX = 23,
    PARAM_CONV_RELU_POOLING_2_INDEX_MAX
} param_conv_relu_pooling_2_index_e;

typedef enum _param_conv_relu_pooling_1_index_e
{
    PARAM_CONV_RELU_POOLING_1_WEIGHTED_BIAS_INDEX = 1,
    PARAM_CONV_RELU_POOLING_1_PAD_X_INDEX = 2,
    PARAM_CONV_RELU_POOLING_1_PAD_Y_INDEX = 3,
    PARAM_CONV_RELU_POOLING_1_ACCUMULATOR_BITS_INDEX = 4,
    PARAM_CONV_RELU_POOLING_1_OVERFLOW_INDEX = 5,
    PARAM_CONV_RELU_POOLING_1_ROUNDING_INDEX = 6,
    PARAM_CONV_RELU_POOLING_1_DOWN_SCALE_SIZEROUNDING_INDEX = 7,
    PARAM_CONV_RELU_POOLING_1_ENABLE_RELU_INDEX = 8,
    PARAM_CONV_RELU_POOLING_1_POOL_TYPE_INDEX = 9,
    PARAM_CONV_RELU_POOLING_1_POOL_SIZE_X_INDEX = 10,
    PARAM_CONV_RELU_POOLING_1_POOL_SIZE_Y_INDEX = 11,
    PARAM_CONV_RELU_POOLING_1_OUTPUT_INDEX = 12,
    PARAM_CONV_RELU_POOLING_1_INDEX_MAX
} param_conv_relu_pooling_1_index_e;

typedef enum _param_conv_relu_index_e
{
    PARAM_CONV_RELU_WEIGHTED_BIAS_INDEX = 1,
    PARAM_CONV_RELU_PAD_X_INDX = 2,
    PARAM_CONV_RELU_PAD_Y_INDX = 3,
    PARAM_CONV_RELU_ACCUMULATOR_BITS_INDEX = 4,
    PARAM_CONV_RELU_OVERFLOW_INDEX = 5,
    PARAM_CONV_RELU_ROUNDING_INDEX = 6,
    PARAM_CONV_RELU_DOWN_SCALE_SIZEROUNDING_INDEX = 7,
    PARAM_CONV_RELU_ENABLE_RELU_INDEX = 8,
    PARAM_CONV_RELU_OUTPUT_INDEX = 9,
    PARAM_CONV_RELU_INDEX_MAX
} param_conv_relu_index_e;

typedef enum _param_conv_index_e
{
    PARAM_CONV_WEIGHT_INDEX = 1,
    PARAM_CONV_BIAS_INDEX = 2,
    PARAM_CONV_PAD_INDEX = 3,
    PARAM_CONV_PAD_MODE_INDEX = 7,
    PARAM_CONV_PAD_CONST_INDEX = 8,
    PARAM_CONV_DILATION_INDEX = 9,
    PARAM_CONV_STRIDE_INDEX = 11,
    PARAM_CONV_DEPTH_MULTIPLIER_INDEX = 13,
    PARAM_CONV_DOWN_SCALE_SIZEROUNDING_INDEX = 14,
    PARAM_CONV_OVERFLOW_INDEX = 15,
    PARAM_CONV_ROUNDING_INDEX = 16,
    PARAM_CONV_OUTPUT_INDEX = 17,
    PARAM_CONV_INDEX_MAX
} param_conv_index_e;

typedef enum _param_relu_index_e
{
    PARAM_RELU_OUTPUT_INDEX = 4,
    PARAM_RELU_INDEX_MAX
} param_relu_index_e;

typedef enum _param_pooling_index_e
{
    PARAM_POOLING_POOL_TYPE_INDEX = 1,
    PARAM_POOLING_POOL_SIZE_X_INDEX = 2,
    PARAM_POOLING_POOL_SIZE_Y_INDEX = 3,
    PARAM_POOLING_POOL_PAD_X_L_INDEX = 4,
    PARAM_POOLING_POOL_PAD_X_R_INDEX = 5,
    PARAM_POOLING_POOL_PAD_Y_T_INDEX = 6,
    PARAM_POOLING_POOL_PAD_Y_B_INDEX = 7,
    PARAM_POOLING_POOL_ROUND_MODE_INDEX = 8,
    PARAM_POOLING_OUTPUT_INDEX = 9,
    PARAM_POOLING_INDEX_MAX
} param_pooling_index_e;

typedef enum _param_fullyconnected_relu_index_e
{
    PARAM_FULLYCONNECTED_RELU_ENABLE_RELU_INDEX = 7,
    PARAM_FULLYCONNECTED_RELU_OUTPUT_INDEX = 8,
    PARAM_FULLYCONNECTED_INDEX_MAX
} param_fullyconnected_relu_index_e;

typedef enum _param_concat2_index_e
{
    PARAM_CONCAT2_INPUT1_INDEX = 0,
    PARAM_CONCAT2_INPUT2_INDEX = 1,
    PARAM_CONCAT2_OUTPUT_INDEX = 2,
    PARAM_CONCAT2_INDEX_MAX
} param_concat2_index_e;

typedef vx_status (*perpareQuantizedWeightAndBiasFunc) (vx_tensor *weight, vx_tensor *bias, vx_tensor tensorIn[2], vx_uint32 coreNum, vx_int32 factor);

extern vx_status vxnneAdapter_Tensor_FormatConvert(vx_tensor inputs, vx_tensor outputs);

VX_INTERNAL_API vx_enum vxoGraph_getKernelType(vx_node node);

VX_INTERNAL_API vx_status vxoGraph_Optimization(vx_graph graph);

EXTERN_C_END

#endif /* __GC_VX_GRAPH_H__ */

