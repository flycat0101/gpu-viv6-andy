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


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:     archModelInterface.c
* Author:       JinboHuang
* Data:         2019-05-27
* Version:      0.5.00
* Description:  Definition of the interface for driver calling Arch Model to support the original struction of the driver.
*               provide several API to transfer the parameters from driver for Standard Arch Model Lib
*
***************************************************************************************/

#include <gc_vx_common.h>
#include <gc_vx_nn_util.h>
#include <gc_vx_nn_wb.h>
#include "archModelInterface.h"


/*******************************************MARCO definition***************************************************/
#define USE_NNPERF_EXPORT           /* Use NNPerf export functions or SW version */

/*Solely for BRCM 2019July */
#define  DEFAULT_DDR_READ_BW_IN_BYTE_PER_CYCLE_64B              2
#define  DEFAULT_DDR_READ_BW_IN_BYTE_PER_CYCLE_128B             14.2
#define  DEFAULT_DDR_READ_BW_IN_BYTE_PER_CYCLE_256B             14.7
#define  DEFAULT_DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B         0.32
#define  DEFAULT_DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_128B        1.8
#define  DEFAULT_DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_256B        3.04
#define  DEFAULT_DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B      1.28
#define  DEFAULT_DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_128B     7.2
#define  DEFAULT_DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_256B     12.16

/****************************************** Local function declaration ***********************************************/
static vx_bool_e isValidNN(vxnne_operation operation);
static vx_bool_e isValidTP(vxnne_operation operation);
static vx_bool_e isValidFC(vxnne_operation operation);
static void updateSingleUpStreamLayer(vxnne_operation operation,archModelOpInfo **OpInfo,vx_uint32 index);
static void updateSingleDownStreamLayer(vxnne_operation operation,archModelOpInfo ** OpInfo,vx_uint32 index);
static void updateStreamLayer(vx_graph graph,archModelOpInfo **OpInfo);
static archModelOpInfo ** archTransferParam(vx_graph graph,archNN_DATABASE_FEATURE *pArchDataFeature,vx_uint32 *totalCount);
static archModelGraphInfo ** archTransferGraphInfo(vx_graph graph,vx_uint32 *totalCount);
archnne_operator_type archGetLayerType(vx_char * name);
static vx_status updateConfigration(archNN_DATABASE_FEATURE *pArchDataFeature,arch_nn_config *pArchNnConfig, vx_context context);
static vx_status initConfigration(arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions,vx_context context);
/***********************************************************************************
* Function:        archGetLayerType
* Description:    get layer type based on the layer name
*                Do not need to save the name string in opInfo, instead we save the type
* Input:        name:            layer name in string
* Ouput:        vx_uint32:    layer type
*            TBD to add more layer name mapping
***************************************************************************************/
archnne_operator_type archGetLayerType(vx_char * name)
{
    if(memcmp(name,"ConvolutionReluPoolingLayer2",strlen(name)) == 0)
        return ARCHNNE_LAYER_CONV_RELU_POOLING2;
    else if(memcmp(name,"TensorScale",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_SCALE;
    else if(memcmp(name,"ConvolutionReluLayer",strlen(name)) == 0)
        return ARCNNNE_LAYER_CONV_RELU;
    if(memcmp(name,"ConvolutionReluPoolingLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_CONV_RELU_POOLING;
    else if(memcmp(name,"SoftmaxLayer",strlen(name)) == 0)
        return ARCNNNE_LAYER_SOFT_MAX;
    else if(memcmp(name,"SoftMax2",strlen(name)) == 0)
        return ARCHNNE_LAYER_SOFT_MAX2;
    if(memcmp(name,"PoolingLayer",strlen(name)) == 0)
        return ARCNNNE_LAYER_POOLING;
    else if(memcmp(name,"PoolingLayer2",strlen(name)) == 0)
        return ARCNNNE_LAYER_POOLING2;
    else if(memcmp(name,"DepthwiseConvolutionLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_DW_CONV;
    if(memcmp(name,"ConvolutionLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_CONV;
    else if(memcmp(name,"FullyConnectedLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_FC;
    else if(memcmp(name,"FullyConnectedReluLayer",strlen(name)) == 0)
        return ARCNNNE_LAYER_FC_RELU;
    if(memcmp(name,"ActivationLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_ACTIVATE;
    else if(memcmp(name,"LeakyReluLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_LEAKY_RELU;
    else if(memcmp(name,"PReluLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_PRE_RELU;
    if(memcmp(name,"RpnLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_RPN;
    else if(memcmp(name,"ROIPoolLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_RIO_POOLING;
    else if(memcmp(name,"ROIPoolReluLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_RIO_POOLING_RELU;
    if(memcmp(name,"ConcatLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_CONCAT;
    else if(memcmp(name,"ReorgLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_REORG;
    else if(memcmp(name,"ReorgLayer2",strlen(name)) == 0)
        return ARCHNNE_LAYER_REORG2;
    if(memcmp(name,"DeConvolutionLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_DE_CONV;
    else if(memcmp(name,"NormalizationLayer",strlen(name)) == 0)
        return ARCNNNE_LAYER_NORMALIZE;
    else if(memcmp(name,"L2NormalizeLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_L2_NORMALIZE;
    if(memcmp(name,"BatchNormalizationLayer",strlen(name)) == 0)
        return ARCNNNE_LYAER_BATCH_NORMALIZE;
    else if(memcmp(name,"TensorAdd",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_ADD;
    else if(memcmp(name,"TensorSub",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_SUB;
    if(memcmp(name,"TensorMul",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_MUL;
    else if(memcmp(name,"TensorDiv",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_DIV;
    else if(memcmp(name,"TensorTranspose",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_TRANSPOSE;
    else if(memcmp(name,"TensorReduceSum",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_REDUCE_SUM;
    if(memcmp(name,"TensorPadOperation",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_PAD;
    else if(memcmp(name,"TensorPadOperation2",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_PAD2;
    else if(memcmp(name,"TensorCopy",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_COPY;
    if(memcmp(name,"TensorReverse",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_REVERSE;
    else if(memcmp(name,"TensorMean",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_MEAN;
    else if(memcmp(name,"TensorSqueeze",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_SQUEEZE;
    if(memcmp(name,"TensorStrideSlice",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_STRIDE_SLICE;
    else if(memcmp(name,"TensorRounding",strlen(name)) == 0)
        return ARCHNNE_LAYER_TENSOR_ROUNDING;
    else if(memcmp(name,"HashLUT",strlen(name)) == 0)
        return ARCHNNE_LAYER_HASHLUT;
    else if(memcmp(name,"LSHProjection",strlen(name)) == 0)
        return ARCHNNE_LAYER_LSH_PROJECT;
    if(memcmp(name,"Reshape",strlen(name)) == 0)
        return ARCHNNE_LAYER_RESHAPE;
    else if(memcmp(name,"LUT2",strlen(name)) == 0)
        return ARCHNNE_LAYER_LUT2;
    else if(memcmp(name,"NormalizationLayer2",strlen(name)) == 0)
        return ARCHNNE_LAYER_NORMALIZE2;
    if(memcmp(name,"AdapterLayer",strlen(name)) == 0)
        return ARCHNNE_LAYER_ADAPTER;
    else if(memcmp(name,"YUV2RGBScale",strlen(name)) == 0)
        return ARCHNNE_LAYER_YUV2RGB_SCALE;
    else if(memcmp(name,"_LSTM_LAYER",strlen(name)) == 0)
        return ARCHNNE_LAYER_LSTM;
    else
        return ARCHNNE_LAYER_ZERO;

}


/* Check if current operation is a valid NN */
static vx_bool_e isValidNN(vxnne_operation operation)
{
    vx_bool_e result = vx_false_e;
    if (   operation->target == VXNNE_OPERATION_TARGET_NN
        && (   operation->operatorType == VXNNE_OPERATOR_CONVOLUTION
            || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV
            /*|| operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED*/))
    {
        result = vx_true_e;
    }
    return result;
}

/* Check if current operation is a valid TP */
static vx_bool_e isValidTP(vxnne_operation operation)
{
    vx_bool_e result = vx_false_e;
    if (operation->target == VXNNE_OPERATION_TARGET_TP &&
                (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_RESHUFFLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_UPSAMPLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_UPSAMPLE2 ||
                operation->operatorType == VXNNE_OPERATOR_POOLING ||
                operation->operatorType == VXNNE_OPERATOR_ROIPOOL ||
                operation->operatorType == VXNNE_OPERATOR_NORMALIZATION ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_ADD ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_TRANS ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_COPY ||
                operation->operatorType == VXNNE_OPERATOR_REORG ||
                operation->operatorType == VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT ||
                operation->operatorType == VXNNE_OPERATOR_UPSAMPLE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_PAD ||
                operation->operatorType == VXNNE_OPERATOR_REORG2 ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_REVERSE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_SQUEEZE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_STRIDE_SLICE ||
                operation->operatorType == VXNNE_OPERATOR_CONCATINDEFINITE ||
                operation->operatorType == VXNNE_OPERATOR_SVDF_MAP ||
                operation->operatorType == VXNNE_OPERATOR_INTERLEAVE ||
                operation->operatorType == VXNNE_OPERATOR_ACTIVATION))
    {
        result = vx_true_e;
    }

    return result;
}

/* Check if current operation is a valid FC */
static vx_bool_e isValidFC(vxnne_operation operation)
{
    vx_bool_e result = vx_false_e;
    if (operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        result = vx_true_e;
    }

    return result;
}


/* Check if current operation is a valid NN */
static vx_bool_e isValidTensorAdd(vxnne_operation operation, arch_uint32 kz, arch_uint32 z)
{
    vx_bool_e result = vx_false_e;
    if (operation->target != VXNNE_OPERATION_TARGET_TP
        && kz <= 8 && kz == 2*z)
    {
        result = vx_true_e;
    }
    return result;
}


/***********************************************************************************
* Function:     updateSingleAllSilbling1X1
* Description:  update the silbling 1X1 info for a single layer
* Input:         operation:        The specific operation
*                OpInfo:            OpInfo struction
*                index:            Indicate the layer index
* Ouput:        void
***************************************************************************************/
static void updateSingleAllSilbling1X1(vxnne_operation operation,archModelOpInfo ** OpInfo,vx_uint32 index)
{
    vx_uint32 i=0, j = 0;
    OpInfo[index]->perf.allSibling1x1 = 1;
    OpInfo[index]->perf.SiblingHas1x1 = 0;
    for (i = 0; i < operation->parentOpNum; i++)
    {
        /* id type is uint32, it should be never become -1. mremove this if */
        {
            for(j = 0; j < operation->parentOps[i]->childOpNum; j++)
            {
                vxnne_operation tmpOperation = operation->parentOps[i]->childOps[j];
                if (isValidNN(tmpOperation) || isValidFC(tmpOperation))
                {
                    if (!isValidFC(tmpOperation))
                    {   /* NN none fc */
                        vx_weights_biases_parameter wb;
                        vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)tmpOperation;
                        wb = convOp->weights_biases;
                        if(WB_KERNEL_X(wb) != 1 || WB_KERNEL_Y(wb) != 1 || isValidTP(operation))
                        {
                            OpInfo[index]->perf.allSibling1x1 = 0;
                        }
                    }
                    /*  Compute SiblingHas1x1 array  */
                    if(operation->target != VXNNE_OPERATION_TARGET_TP
                        && OpInfo[index]->kx == 1 && OpInfo[index]->ky == 1)
                    {
                        OpInfo[index]->perf.SiblingHas1x1 = 1;
                    }
                }
                else    /* TP and FC are all kx==1 and ky == 1 */
                {
                    /* do nothing */
                }

            }
        }
    }
}

/***********************************************************************************
* Function:     updateSingleUpStreamLayer
* Description:  update the upstream relationship(parent information) for a single layer
* Input:         operation:        The specific operation
*                OpInfo:            OpInfo struction
*                index:            Indicate the layer index
* Ouput:        void
***************************************************************************************/
static void updateSingleUpStreamLayer(vxnne_operation operation,
    archModelOpInfo **OpInfo,
    vx_uint32 index)
{
    vx_uint32 j=0;
    /* update upstream based on the parent operation */
    {
        vx_uint32 k = 0;
        for (j = 0; j < operation->parentOpNum; j++)
        {
            if (operation->parentOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || operation->parentOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                OpInfo[index]->upStreamLayer[k++] = -1;
            }
            else
            {
                vx_int32 segIndex = operation->parentOps[j]->segIndex;
                if (k > 0)
                {
                    if (OpInfo[index]->upStreamLayer[k] < segIndex)
                    {
                        vx_int32 m;
                        for (m = k - 1; m >= 0; m--)
                        {
                            OpInfo[index]->upStreamLayer[m + 1] = OpInfo[index]->upStreamLayer[m];
                        }
                        OpInfo[index]->upStreamLayer[0] = segIndex;
                        k++;
                    }
                    else
                    {
                        OpInfo[index]->upStreamLayer[k++] = segIndex;
                    }
                }
                else {
                    OpInfo[index]->upStreamLayer[k++] = segIndex;
                }
            }
        }

        if (index == 0)
        {
            OpInfo[index]->upStreamLayer[k++] = -1;
        }
        OpInfo[index]->upStreamLayerCount = k;
    }
}


/***********************************************************************************
* Function:     updateSingleDownStreamLayer
* Description:  update the downstream relationship(child information) for a single layer
* Input:         operation:        The specific operation
*                OpInfo:            OpInfo struction
*                index:            Indicate the layer index
* Ouput:        void
***************************************************************************************/
static void updateSingleDownStreamLayer(vxnne_operation operation,
    archModelOpInfo ** OpInfo,
    vx_uint32 index)
{
    vx_uint32 j=0;

    /* update downstream based on the chile operation */
    {
        vx_uint32 k = 0;
        for (j = 0; j < operation->childOpNum; j++)
        {
            if (operation->childOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || operation->childOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (operation->childOpNum == 1) {
                    OpInfo[index]->downStreamLayer[k++] = -1;
                }
            }
            else
            {
                vx_int32 m;
                vx_int32 segIndex = operation->childOps[j]->segIndex;
                arch_bool hasDownLayer = arch_false_e;
                /*check childOperation's upStreamLayer to determin if segIndex is a real downStreamLayer*/
                for (m = 0; m < (vx_int32)OpInfo[segIndex]->upStreamLayerCount; m++)
                {
                    if (OpInfo[segIndex]->upStreamLayer[m] == (vx_int32)index)
                    {
                        hasDownLayer = arch_true_e;
                        break;
                    }
                }
                if (!hasDownLayer)
                {
                    continue;
                }
                if (k > 0)
                {
                    if (OpInfo[index]->downStreamLayer[k] < segIndex)
                    {
                        OpInfo[index]->downStreamLayer[k++] = segIndex;
                    }
                    else
                    {
                        for (m = k - 1; m >= 0; m--)
                        {
                            OpInfo[index]->downStreamLayer[m + 1] = OpInfo[index]->downStreamLayer[m];
                        }
                        OpInfo[index]->downStreamLayer[0] = segIndex;
                        k++;
                    }
                }
                else
                {
                    OpInfo[index]->downStreamLayer[k++] = segIndex;
                }
            }
        }
        OpInfo[index]->downStreamLayerCount = k;

        /* need to set the downstreamlayer to -1 if there is no downstream */
        if(OpInfo[index]->downStreamLayerCount == 0)
        {
            OpInfo[index]->downStreamLayer[0] = -1;
        }
    }
}

/***********************************************************************************
* Function:     updateStreamLayer
* Description:  update the up/down stream for the whole graph
* Input:         graph:        The whole graph for this case
*                OpInfo:        OpInfo struction
* Ouput:        void
***************************************************************************************/
static void updateStreamLayer(vx_graph graph,
    archModelOpInfo **OpInfo)
{
    vx_uint32 i, j, count=0;
    vx_uint32 k = 0;

    /* Loop every node */
    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];

        /* Loop all operation for one node */
        for (j = 0; j < node->layer->num_operations; j++)
        {
            vxnne_operation operation = node->layer->operations[j];

            if (isValidNN(operation) || isValidFC(operation) || isValidTP(operation))
            {
                updateSingleUpStreamLayer(operation,OpInfo,count);

                /* update orignal opinfo */
                for(k = 0; k < operation->parentOpNum; k++)
                {
                    OpInfo[count]->parentOpId[k] = operation->parentOps[k]->id;
                    OpInfo[count]->parentOpType[k] = operation->parentOps[k]->operatorType;
                    OpInfo[count]->parentLayer[k] = operation->parentOps[k]->layer->node->id;
                    /* Invalid parent operation will be ignored in current log, need to set the id to -1 */
                    if (isValidNN(operation->parentOps[k]) || isValidFC(operation->parentOps[k]) || isValidTP(operation->parentOps[k]))
                    {
                        OpInfo[count]->parentAbsId[k] = operation->parentOps[k]->segIndex;
                    }
                    else
                    {
                        OpInfo[count]->parentAbsId[k] = -1;
                    }
                    //OpInfo[count]->parentLayerName[k] = operation->parentOps[k]->layer->name;
                    OpInfo[count]->parentLayerType[k] = archGetLayerType(operation->parentOps[k]->layer->name);
                }
                /* add to set the invalid abs id to -1. VB can set to 0, but C need to set as -1 */
                if(operation->parentOpNum == 0)
                    OpInfo[count]->parentAbsId[0] = -1;

                for(k = 0; k < operation->childOpNum; k++)
                {
                    OpInfo[count]->childOpId[k] = operation->childOps[k]->id;
                    OpInfo[count]->childOpType[k] = operation->childOps[k]->operatorType;
                    OpInfo[count]->childLayer[k] = operation->childOps[k]->layer->node->id;
                    /* Invalid child operation will be ignored in current log, need to set the id to -1 */
                    if (isValidNN(operation->childOps[k]) || isValidFC(operation->childOps[k]) || isValidTP(operation->childOps[k]))
                    {
                        OpInfo[count]->childAbsId[k] = operation->childOps[k]->segIndex;
                    }
                    else
                    {
                        OpInfo[count]->childAbsId[k] = -1;
                    }
                    //OpInfo[count]->childLayerName[k] = operation->childOps[k]->layer->name;
                    OpInfo[count]->childLayerType[k] = archGetLayerType(operation->childOps[k]->layer->name);
                }
                /* add to set the invalid abs id to -1. VB can set to 0, but C need to set as -1 */

                if (operation->childOpNum == 0)
                {
                    OpInfo[count]->childAbsId[0] = -1;
                }

                count++;
            }
        }
    }
    count = 0;

    /* Merge from CL 213845 */
    OpInfo[0]->perf.allSibling1x1 = 0;
    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];

        for (j = 0; j < node->layer->num_operations; j++)
        {
            vxnne_operation operation = node->layer->operations[j];
            if (isValidNN(operation) || isValidFC(operation) || isValidTP(operation))
            {
                updateSingleDownStreamLayer(operation,OpInfo,count);

                /* Merge from CL213817 */
                updateSingleAllSilbling1X1(operation,OpInfo,count);
                /*vxInfo("index %d: allSibling1x1 is %d,  SiblingHas1x1 is %d.\n",count,OpInfo[count]->perf.allSibling1x1, OpInfo[count]->perf.SiblingHas1x1);*/
                count++;
            }
        }

    }

}


/***********************************************************************************
* Function:     archTransferGraphInfo
* Description:  Transfer the graph from driver to graph information for Arch Model
                After transfer, we do not need graph struction any more, all the information
                should be saved in graphInfo
                ****This function should be also align with the interface to Python
* Input:        graph:              The whole graph for this case
* Ouput:        GraphInfo:          archModelGraphInfo struction
***************************************************************************************/
archModelGraphInfo ** archTransferGraphInfo(vx_graph graph, vx_uint32 *totalCount)
{
    vx_uint32 i = 0, j = 0;
    vx_uint32 count = 0;
    archModelGraphInfo **graphInfo = NULL;

    arch_nn_config archNNConfig;
    arch_drv_option archDrvOption;
    vx_uint32 maxCount = graph->layer->base.num_operations;
    vx_context context = vxoContext_GetFromReference(&graph->base);
    arch_nn_config *pArchNnConfig =  &archNNConfig;
    arch_drv_option *pArchOptions = &archDrvOption;

    archNN_DATABASE_FEATURE archDataFeature = {0};

    memset(pArchNnConfig,0,sizeof(arch_nn_config));
    memset(pArchOptions,0,sizeof(arch_drv_option));
    initConfigration(pArchNnConfig,pArchOptions,context);

    /* Init data feature */
    updateConfigration(&archDataFeature,pArchNnConfig, context);

    /* Init graph info for arch model */
    graphInfo = initArchGraphInfo(maxCount);
    if(graphInfo == NULL)
        return NULL;

    /* Fill data for every node */
    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];

        for (j = 0; j < node->layer->num_operations; j++)
        {
            vxnne_operation operation = node->layer->operations[j];
            vx_weights_biases_parameter wb;
            //opInfo[count]->tpType = operation->parameter.tpType;   /* TBD */


            /* Basic information */
            graphInfo[count]->absId = operation->absoluteOperationID;
            graphInfo[count]->layerId = node->id;
            graphInfo[count]->layerType = archGetLayerType(node->layer->name);
            graphInfo[count]->operationId = operation->id;
            graphInfo[count]->opType = (archnne_operator_e)operation->operatorType;
            graphInfo[count]->opTarget = (archnne_operation_target_e)operation->target;

            /* parent/chind info */
            graphInfo[count]->upStreamLayerCount = operation->parentLayerNum;
            graphInfo[count]->upStreamOpCount = operation->parentOpNum;
            graphInfo[count]->downStreamLayerCount = operation->childLayerNum;
            graphInfo[count]->downStreamOpCount = operation->childOpNum;


            /* different for the target */
            if (isValidNN(operation))
            {
                vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;
                vx_uint32 outXSize, outYSize;
                wb = convOp->weights_biases;
                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0),
                     WB_KERNEL_X(wb),
                     operation->parameter.pad_x_left,
                     operation->parameter.pad_x_right,
                     operation->parameter.pool_size_x,
                     operation->parameter.pool_stride,
                     &outXSize,
                     1);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1),
                     WB_KERNEL_Y(wb),
                     operation->parameter.pad_y_top,
                     operation->parameter.pad_y_bottom,
                     operation->parameter.pool_size_y,
                     operation->parameter.pool_stride,
                     &outYSize,
                     1);

                /* set graph info for target NN */
                /* Pooling info */
                graphInfo[count]->pollingSize = gcmMAX(operation->parameter.pool_size_x, 1);
                graphInfo[count]->pollingStride = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;

                /* other */
                graphInfo[count]->xOffset = (-1) * convOp->pad_x_left;            /* for calculate xOffset*/
                graphInfo[count]->yOffset = (-1) * convOp->pad_y_top;            /* for calculate yOffset */

                /* data size */
                graphInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(convOp->inputs) * 8;
                graphInfo[count]->outputDataSize = TENSOR_DATA_SIZE(convOp->outputs) * 8;

                graphInfo[count]->isFp16 = (TENSOR_DATA_TYPE(convOp->inputs) == VX_TYPE_FLOAT16) ? 1:0;            /* is float 16 or not */

                /* kernel information */
                graphInfo[count]->kx = WB_KERNEL_X(wb);
                graphInfo[count]->ky = WB_KERNEL_Y(wb);
                graphInfo[count]->kz = (operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV && WB_IS_DEPTH_WISE(wb)) ? WB_OUTPUT_Z(wb) : WB_KERNEL_Z(wb);

                /* input */
                graphInfo[count]->origInX = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 0);
                graphInfo[count]->origInY = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 1);
                graphInfo[count]->origInZ = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 2);

                /* out */
                graphInfo[count]->finalOutX = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0);;
                graphInfo[count]->finalOutY = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1);
                graphInfo[count]->finalOutZ = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 2);

                graphInfo[count]->nnOutX = outXSize;
                graphInfo[count]->nnOutY = outYSize;

                /* Compress */
                graphInfo[count]->coefNonZeroRatio = WB_NON_ZERO_RATIO(wb);;
                graphInfo[count]->coefCompression = WB_COMPRESS_RATIO(wb);
                graphInfo[count]->imageCompression = archIsFeatureAvailable(pArchNnConfig,pArchOptions,&archDataFeature, ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
                graphInfo[count]->imageNonZeroRatio = 0.300000000000000;

                operation->segIndex = count;                /* for downstreamlayer setting */
                count++;
            }
            else if (isValidTP(operation))
            {

                vxnne_tp_operation tpOp  = (vxnne_tp_operation)operation;
                gcmASSERT(tpOp->input != NULL);
                gcmASSERT(tpOp->output != NULL);

                /* Pooling info */
                graphInfo[count]->pollingSize = operation->parameter.pool_size_x;
                graphInfo[count]->pollingStride = operation->parameter.pool_stride;

                /* other */
                graphInfo[count]->xOffset = (-1) * tpOp->base.parameter.pad_x_left;            /* for calculate xOffset*/
                graphInfo[count]->yOffset = (-1) * tpOp->base.parameter.pad_y_top;            /* for calculate yOffset */

                /* data size */
                graphInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(tpOp->input) * 8;
                graphInfo[count]->outputDataSize = TENSOR_DATA_SIZE(tpOp->output) * 8;
                graphInfo[count]->isFp16 = (TENSOR_DATA_TYPE(tpOp->input) == VX_TYPE_FLOAT16) ? 1:0;            /* is float 16 or not */


                /* kernel information */
                graphInfo[count]->kx = 1;
                graphInfo[count]->ky = 1;
                if (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE)
                {
                    wb = tpOp->weights_biases;
                    gcmASSERT(wb != NULL);
                    gcmASSERT(tpOp->weights_biases != NULL);
                    graphInfo[count]->kz = WB_KERNEL_Z(tpOp->weights_biases);
                }
                else
                {
                    graphInfo[count]->kz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);

                }

                /* input */
                graphInfo[count]->origInX = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                graphInfo[count]->origInY = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                graphInfo[count]->origInZ = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);

                /* out */
                graphInfo[count]->finalOutX = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 0);
                graphInfo[count]->finalOutY = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 1);
                graphInfo[count]->finalOutZ = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);

                graphInfo[count]->nnOutX = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                graphInfo[count]->nnOutY = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);

                /* Compress, TP do not need */
                operation->segIndex = count;            /* for downstreamlayer setting */
                count++;
            }
            else if (isValidFC(operation))
            {
               if (operation->target == VXNNE_OPERATION_TARGET_TP)
                {
                    vxnne_tp_operation fcOp = (vxnne_tp_operation)operation;
                    vx_uint32 inDims = fcOp->input->dimCount;
                    vx_uint32 outDims = fcOp->output->dimCount;

                    wb = fcOp->weights_biases;

                    /* Pooling info */
                    graphInfo[count]->pollingSize = operation->parameter.pool_size_x;
                    graphInfo[count]->pollingStride = operation->parameter.pool_stride;

                    /* data size */
                    graphInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(fcOp->input) * 8;
                    graphInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->output) * 8;
                    graphInfo[count]->isFp16 = (TENSOR_DATA_TYPE(fcOp->input) == VX_TYPE_FLOAT16) ? 1:0;            /* is float 16 or not */

                    /* input */
                    graphInfo[count]->origInX = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    graphInfo[count]->origInY = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);
                    graphInfo[count]->origInZ = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);

                    /* out */
                    graphInfo[count]->finalOutX = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 0);
                    graphInfo[count]->finalOutY = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 1);
                    graphInfo[count]->finalOutZ = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 2);

                    graphInfo[count]->nnOutX = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    graphInfo[count]->nnOutY = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);

                    {
                        /*convert TP FC input/output info for arch model analysis when dims<=2 */
                        if ((inDims == 2) || (inDims == 1))
                        {
                            graphInfo[count]->origInX = 1;
                            graphInfo[count]->origInY = 1;
                            graphInfo[count]->finalOutX = 1;
                            graphInfo[count]->finalOutY = 1;
                            graphInfo[count]->finalOutZ = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                        }

                        if ((outDims == 2) || (outDims == 1))
                        {
                            graphInfo[count]->finalOutX = 1;
                            graphInfo[count]->finalOutY = 1;
                            graphInfo[count]->finalOutZ = WB_OUTPUT_Z(wb);
                        }
                    }
                }
                else if (operation->target == VXNNE_OPERATION_TARGET_NN)
                {
                    vxnne_convolution_relu_pooling_operation fcOp = (vxnne_convolution_relu_pooling_operation)operation;
                    wb = fcOp->weights_biases;


                    /* Pooling info */
                    graphInfo[count]->pollingSize = gcmMAX(operation->parameter.pool_size_x, 1);
                    graphInfo[count]->pollingStride = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;

                    /* data size */
                    graphInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(fcOp->inputs) * 8;
                    graphInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->outputs) * 8;
                    graphInfo[count]->isFp16 = (TENSOR_DATA_TYPE(fcOp->inputs) == VX_TYPE_FLOAT16) ? 1:0;            /* is float 16 or not */

                    /* input */
                    graphInfo[count]->origInX = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    graphInfo[count]->origInY = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    graphInfo[count]->origInZ = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 2);

                    /* out */
                    graphInfo[count]->finalOutX = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 0);
                    graphInfo[count]->finalOutY = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 1);
                    graphInfo[count]->finalOutZ = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 2);

                    graphInfo[count]->nnOutX = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    graphInfo[count]->nnOutY = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                }
                else
                {
                    continue;
                }

                /* other */
                graphInfo[count]->xOffset = (-1) * WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * operation->parameter.pad_x_left);            /* for calculate xOffset*/
                graphInfo[count]->yOffset = (-1) * WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) * operation->parameter.pad_y_top);            /* for calculate yOffset */

                /* kernel information */
                graphInfo[count]->kx = 1;
                graphInfo[count]->ky = 1;
                graphInfo[count]->kz = WB_KERNEL_Z(wb);

                /* Compress */
                graphInfo[count]->coefNonZeroRatio = WB_NON_ZERO_RATIO(wb);;
                graphInfo[count]->coefCompression = WB_COMPRESS_RATIO(wb);
                graphInfo[count]->imageCompression = archIsFeatureAvailable(pArchNnConfig,pArchOptions,&archDataFeature, ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
                graphInfo[count]->imageNonZeroRatio = 0.300000000000000;

                operation->segIndex = count;            /* for downstreamlayer setting */
                count++;
            }

        }

    }

    *totalCount = count;
    return graphInfo;

}

/***********************************************************************************
* Function:     archTransferParam
* Description:  Transfer the graph from driver to OpInfo for Arch Model
                After transfer, we do not need graph struction any more, all the information
                should be saved in opInfo
                ****This function should be used by driver only
* Input:        graph:        The whole graph for this case
*                totalCount:    return the total OP count
* Ouput:        OpInfo:        OpInfo struction
***************************************************************************************/
static archModelOpInfo ** archTransferParam(vx_graph graph, archNN_DATABASE_FEATURE *pArchDataFeature,vx_uint32 *totalCount)
{
    vx_uint32 i, j, count=0;

    struct _archModelOpInfo ** opInfo;
    vx_context context = vxoContext_GetFromReference(&graph->base);
    arch_nn_config *pArchNnConfig = (arch_nn_config *)&(context->nnConfig);
    arch_drv_option *pArchOptions = (arch_drv_option *)&(context->options);
    vx_bool hasVXC = vx_false_e;
    /*vx_status vxStatus = VX_SUCCESS;*/
    vx_bool supportNNTPParallel = vx_false_e;

    if (!graph->layer) return NULL;

    opInfo = initArchOpInfo(graph->layer->base.num_operations);
    if (opInfo == NULL)
    {
        /*vxStatus = VX_FAILURE;*/
        goto error;
    }

    /* TBD:move to lib */
    //initSegmentCostResult(archModel);

    /* Fill data for every node */
    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
        if (node->layer == NULL)
        {
            hasVXC = vx_true_e;
            continue; /* skipped node without layer/operation */
        }

        for (j = 0; j < node->layer->num_operations; j++)
        {
            vxnne_operation operation = node->layer->operations[j];
            vx_weights_biases_parameter wb;
            opInfo[count]->swTilingType = -1;

            /*  init extra var */
            opInfo[count]->tpType = operation->parameter.tpType;
            //opInfo[count]->op = operation->operatorType;
            opInfo[count]->absoluteOperationID = operation->absoluteOperationID;
            opInfo[count]->uid = getUserIDFromOutputTensor(operation->outputs[0]);
            opInfo[count]->layerId = node->id;
            opInfo[count]->layerName = node->layer->name;
            opInfo[count]->operationId = operation->id;

            /* parent/chind info */
            opInfo[count]->upLayerCount = operation->parentLayerNum;
            opInfo[count]->upOpCount = operation->parentOpNum;
            opInfo[count]->downLayerCount = operation->childLayerNum;
            opInfo[count]->downOpCount = operation->childOpNum;
            /* init extra var done */

            if (isValidNN(operation))
            {
                /*vx_uint32 outXSize, outYSize;*/
                vx_uint32 calcInputXSize, calcoutputYSize;
                vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;

                wb = convOp->weights_biases;
                gcmASSERT(wb != NULL);
                gcmASSERT(convOp->orig_inputs != NULL);
                gcmASSERT(convOp->inputs != NULL);
                gcmASSERT(convOp->outputs != NULL);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0),
                     WB_KERNEL_X(wb),
                     operation->parameter.pad_x_left,
                     operation->parameter.pad_x_right,
                     operation->parameter.pool_size_x,
                     operation->parameter.pool_stride,
                     &calcInputXSize,
                     1);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1),
                     WB_KERNEL_Y(wb),
                     operation->parameter.pad_y_top,
                     operation->parameter.pad_y_bottom,
                     operation->parameter.pool_size_y,
                     operation->parameter.pool_stride,
                     &calcoutputYSize,
                     1);

                opInfo[count]->op      = (archnne_operator_e)operation->operatorType;
                opInfo[count]->target  = (archnne_operation_target_e)operation->target;
                opInfo[count]->psize   = gcmMAX(operation->parameter.pool_size_x, 1);
                opInfo[count]->pstride = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;
                opInfo[count]->xpad    = convOp->pad_x_left;
                opInfo[count]->ypad    = convOp->pad_y_top;
                opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(convOp->inputs);
                opInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(convOp->inputs) * 8;
                opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(convOp->outputs);
                opInfo[count]->outputDataSize   = TENSOR_DATA_SIZE(convOp->outputs) * 8;
                opInfo[count]->kx = WB_KERNEL_X(wb);
                opInfo[count]->ky = WB_KERNEL_Y(wb);
                opInfo[count]->kz = (operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV && WB_IS_DEPTH_WISE(wb)) ? WB_OUTPUT_Z(wb) : WB_KERNEL_Z(wb);
                opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 2);
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 2);
                opInfo[count]->calcinx = calcInputXSize;
                opInfo[count]->calciny = calcoutputYSize;
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1);
                opInfo[count]->p3 = operation->parameter.pool_size_x == 3 ? 1 : 0;
                opInfo[count]->xsize = opInfo[count]->calcinx;
                opInfo[count]->ysize = opInfo[count]->calciny;
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);

                /* init buf */
                opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;
                opInfo[count]->fcmd = vx_true_e;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                if (count > 0 && supportNNTPParallel)
                {
                    if (opInfo[count - 1]->target == ARCHNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }

                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCount;

                /* update compress ratio */
                opInfo[count]->perf.coefNonZeroRatio  = WB_NON_ZERO_RATIO(wb);
                opInfo[count]->perf.coefCompressRatio = WB_COMPRESS_RATIO(wb);
                opInfo[count]->perf.imageCompressRatio = archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
                opInfo[count]->perf.imageNonZeroRatio  = 0.300000000000000;

                /* check if it is Tensor Add: need to double check, TBD */
                if(isValidTensorAdd(operation, opInfo[count]->kz,opInfo[count]->oz))
                    opInfo[count]->op = ARCHNNE_OPERATOR_TENSOR_ADD;

                count++;
            }
            else if (isValidTP(operation))
            {
                vxnne_tp_operation tpOp  = (vxnne_tp_operation)operation;

                gcmASSERT(tpOp->input != NULL);
                gcmASSERT(tpOp->output != NULL);
                if (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE)
                {
                    wb = tpOp->weights_biases;
                    gcmASSERT(wb != NULL);
                    gcmASSERT(tpOp->weights_biases != NULL);
                    opInfo[count]->kz = WB_KERNEL_Z(tpOp->weights_biases);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                    opInfo[count]->stridex = WB_STRIDE_X(wb);
                    opInfo[count]->stridey = WB_STRIDE_Y(wb);
                }
                else
                {
                    opInfo[count]->kz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                }

                opInfo[count]->calcinx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->calciny = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 1);
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);

                opInfo[count]->op      = (archnne_operator_e)operation->operatorType;
                opInfo[count]->target  = (archnne_operation_target_e)operation->target;
                opInfo[count]->psize   = operation->parameter.pool_size_x;
                opInfo[count]->pstride = gcmMAX(1, operation->parameter.pool_stride);
                opInfo[count]->xpad    = tpOp->base.parameter.pad_x_left;
                opInfo[count]->ypad    = tpOp->base.parameter.pad_y_top;
                opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(tpOp->input);
                opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(tpOp->input) * 8;
                opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(tpOp->output);
                opInfo[count]->outputDataSize   = TENSOR_DATA_SIZE(tpOp->output) * 8;
                opInfo[count]->kx = 1;
                opInfo[count]->ky = 1;
                opInfo[count]->p3 = (operation->parameter.pool_size_x == 3) ? 1 : 0;
                if (operation->operatorType == VXNNE_OPERATOR_POOLING)
                {
                    opInfo[count]->xsize = opInfo[count]->inx;
                    opInfo[count]->ysize = opInfo[count]->iny;
                }
                else
                {
                    opInfo[count]->xsize = opInfo[count]->origoutx;
                    opInfo[count]->ysize = opInfo[count]->origouty;
                }
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;
                opInfo[count]->fcmd = vx_false_e;

                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                if (count > 0 && supportNNTPParallel)
                {
                    if (opInfo[count - 1]->target == ARCHNNE_OPERATION_TARGET_NN) /* NN -> TP*/
                    {
                        if (opInfo[count]->ky != 1)
                        {
                            opInfo[count - 1]->bfy = 2;
                        }
                        else
                        {
                            opInfo[count - 1]->bfz = 2;
                        }
                    }
                }
                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCount;

                /* Since the wb is NULL, do not need to update compress ratio */

                operation->segIndex = count;
                count++;
            }
            else if (isValidFC(operation))
            {
                if (operation->target == VXNNE_OPERATION_TARGET_TP)
                {
                    vxnne_tp_operation fcOp = (vxnne_tp_operation)operation;
                    vx_uint32 inDims = fcOp->input->dimCount;
                    vx_uint32 outDims = fcOp->output->dimCount;

                    wb = fcOp->weights_biases;
                    opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(fcOp->input) * 8;
                    opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(fcOp->input);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                    opInfo[count]->calcinx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->calciny = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);

                    opInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->output) * 8;
                    opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(fcOp->output);

                    opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 0);
                    opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 1);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 2);

                    opInfo[count]->psize   = operation->parameter.pool_size_x;
                    opInfo[count]->pstride = operation->parameter.pool_stride;

                    {
                        /*convert TP FC input/output info for arch model analysis when dims<=2 */
                        if ((inDims == 2) || (inDims == 1))
                        {
                            opInfo[count]->inx = 1;
                            opInfo[count]->iny = 1;
                            opInfo[count]->calcinx = 1;
                            opInfo[count]->calciny = 1;
                            opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                        }

                        if ((outDims == 2) || (outDims == 1))
                        {
                            opInfo[count]->origoutx = 1;
                            opInfo[count]->origouty = 1;
                            opInfo[count]->oz = WB_OUTPUT_Z(wb);
                        }
                    }
                    opInfo[count]->p3 = 0;
                    opInfo[count]->xsize = 1;
                    opInfo[count]->ysize = 1;
                    opInfo[count]->pix = 1;
                    opInfo[count]->piy = 1;
                }
                else if (operation->target == VXNNE_OPERATION_TARGET_NN)
                {
                    vxnne_convolution_relu_pooling_operation fcOp = (vxnne_convolution_relu_pooling_operation)operation;
                    wb = fcOp->weights_biases;
                    opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(fcOp->inputs) * 8;
                    opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(fcOp->inputs);
                    opInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->outputs) * 8;
                    opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(fcOp->outputs);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 2);
                    opInfo[count]->calcinx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->calciny = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 0);
                    opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 1);
                    opInfo[count]->oz = WB_OUTPUT_Z(wb);
                    opInfo[count]->psize   = gcmMAX(operation->parameter.pool_size_x, 1);
                    opInfo[count]->pstride = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;
                    opInfo[count]->p3 = 0;
                    opInfo[count]->xsize = opInfo[count]->calcinx;
                    opInfo[count]->ysize = opInfo[count]->calciny;
                    opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                    opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                }
                else
                {
                    continue;
                }

                opInfo[count]->op      = (archnne_operator_e)operation->operatorType;
                opInfo[count]->target  = (archnne_operation_target_e)operation->target;
                opInfo[count]->xpad    = WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * operation->parameter.pad_x_left);
                opInfo[count]->ypad    = WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) *  operation->parameter.pad_y_top);
                /*opInfo[count]->dsize = vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb)) * 8;*/
                opInfo[count]->kx = 1;
                opInfo[count]->ky = 1;
                opInfo[count]->kz = WB_KERNEL_Z(wb);


                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->sbuf = SW_TILING_FROM_DDR;
                opInfo[count]->dbuf = SW_TILING_FROM_DDR;
                opInfo[count]->kbuf = SW_TILING_FROM_DDR;
                opInfo[count]->fcmd = vx_false_e;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                if (count > 0 && supportNNTPParallel)
                {
                    if (operation->target == VXNNE_OPERATION_TARGET_TP)
                    {
                        if (opInfo[count - 1]->target != ARCHNNE_OPERATION_TARGET_TP)
                        {
                            if (opInfo[count - 1]->ky != 1)
                            {
                                opInfo[count - 1]->bfy = 2;
                            }
                            else
                            {
                                opInfo[count - 1]->bfz = 2;
                            }
                        }
                    }
                    else
                    {
                        if (opInfo[count - 1]->target == ARCHNNE_OPERATION_TARGET_TP)
                        {
                            opInfo[count - 1]->bfy = 2;
                        }
                    }
                }
                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCount;

                /* update compress ratio */
                opInfo[count]->perf.coefNonZeroRatio  = WB_NON_ZERO_RATIO(wb);
                opInfo[count]->perf.coefCompressRatio = WB_COMPRESS_RATIO(wb);
                opInfo[count]->perf.imageCompressRatio = archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
                opInfo[count]->perf.imageNonZeroRatio  = 0.300000000000000;

                count++;
            }
        }
    }
    *totalCount = count;
    return opInfo;
error:
    deInitArchOpInfo(opInfo, graph->layer->base.num_operations);
    return NULL;
}


/***********************************************************************************
* Function:     initConfigration
* Description:  init the configration from driver one by one
* Input:        pArchNnConfig:  extra data feature info, will be filled in this function
*               pArchOptions:    dependency
                context:            context from driver
* Ouput:        vx_status:       success or failed
***************************************************************************************/
static vx_status initConfigration(arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions,vx_context context)
{
    vx_nn_config *pContextNnConfig = &(context->nnConfig);
    vx_drv_option *pContextOptions = &(context->options);
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;
    gctUINT chipIDs[32] = {0};
    gctUINT32 coreCount = 0;

    /* set NN config */
    pArchNnConfig->isSet = pContextNnConfig->isSet;            /* NOT */

    /* archNN_FIXED_FEATURE */
    pArchNnConfig->fixedFeature.vipCoreCount = pContextNnConfig->fixedFeature.vipCoreCount;
    pArchNnConfig->fixedFeature.nnCoreCount = pContextNnConfig->fixedFeature.nnCoreCount;
    pArchNnConfig->fixedFeature.nnCoreCountInt8 = pContextNnConfig->fixedFeature.nnCoreCountInt8;
    pArchNnConfig->fixedFeature.nnCoreCountInt16 = pContextNnConfig->fixedFeature.nnCoreCountInt16;
    pArchNnConfig->fixedFeature.nnCoreCountFloat16 = pContextNnConfig->fixedFeature.nnCoreCountFloat16;
    pArchNnConfig->fixedFeature.nnCoreCountBFloat16 = pContextNnConfig->fixedFeature.nnCoreCountBFloat16;
    pArchNnConfig->fixedFeature.nnMadPerCore = pContextNnConfig->fixedFeature.nnMadPerCore;        /* NO */
    pArchNnConfig->fixedFeature.nnInputBufferDepth = pContextNnConfig->fixedFeature.nnInputBufferDepth;
    pArchNnConfig->fixedFeature.nnAccumBufferDepth = pContextNnConfig->fixedFeature.nnAccumBufferDepth;
    pArchNnConfig->fixedFeature.nnFCNonPrunAccel = pContextNnConfig->fixedFeature.nnFCNonPrunAccel;            /* May not needed */
    pArchNnConfig->fixedFeature.nnInImageOffsetBits = pContextNnConfig->fixedFeature.nnInImageOffsetBits;      /* NO */
    pArchNnConfig->fixedFeature.tpCoreCount = pContextNnConfig->fixedFeature.tpCoreCount;
    pArchNnConfig->fixedFeature.tpPwlLUTCount = pContextNnConfig->fixedFeature.tpPwlLUTCount;                  /* NO */
    pArchNnConfig->fixedFeature.tpPwlLUTSize = pContextNnConfig->fixedFeature.tpPwlLUTSize;                    /* NO */
    pArchNnConfig->fixedFeature.vip7Version = pContextNnConfig->fixedFeature.vip7Version;
    pArchNnConfig->fixedFeature.vipBrickMode = pContextNnConfig->fixedFeature.vipBrickMode;
    pArchNnConfig->fixedFeature.tpReorderInImageSize = pContextNnConfig->fixedFeature.tpReorderInImageSize;    /* NO */
    pArchNnConfig->fixedFeature.tpliteCoreCount = pContextNnConfig->fixedFeature.tpliteCoreCount;
    pArchNnConfig->fixedFeature.nnFP16XYDPX = pContextNnConfig->fixedFeature.nnFP16XYDPX;
    pArchNnConfig->fixedFeature.nnFP16XYDPY = pContextNnConfig->fixedFeature.nnFP16XYDPY;
    pArchNnConfig->fixedFeature.nnFP16ZDP = pContextNnConfig->fixedFeature.nnFP16ZDP;
    pArchNnConfig->fixedFeature.zrlBits = pContextNnConfig->fixedFeature.zrlBits;
    pArchNnConfig->fixedFeature.uscCacheControllers = pContextNnConfig->fixedFeature.uscCacheControllers;
    pArchNnConfig->fixedFeature.uscBanks = pContextNnConfig->fixedFeature.uscBanks;                            /* NO */
    pArchNnConfig->fixedFeature.nnLanesPerOutCycle = pContextNnConfig->fixedFeature.nnLanesPerOutCycle;
    pArchNnConfig->fixedFeature.maxOTNumber = pContextNnConfig->fixedFeature.maxOTNumber;
    pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte = pContextNnConfig->fixedFeature.equivalentVipsramWidthInByte;
    pArchNnConfig->fixedFeature.shaderCoreCount = pContextNnConfig->fixedFeature.shaderCoreCount;

    /* query multi core count */
    gcmGETCURRENTHARDWARE(hwType);
    gcoHAL_QueryCoreCount(gcvNULL, hwType, &coreCount, chipIDs);
    pArchNnConfig->fixedFeature.multiVIPnum = coreCount;

    /* archNN_CUSTOMIZED_FEATURE */
    pArchNnConfig->customizedFeature.vipSRAMSize = pContextNnConfig->customizedFeature.vipSRAMSize;
    pArchNnConfig->customizedFeature.axiSRAMSize = pContextNnConfig->customizedFeature.axiSRAMSize;
    pArchNnConfig->customizedFeature.ddrReadBWLimit = pContextNnConfig->customizedFeature.ddrReadBWLimit;
    pArchNnConfig->customizedFeature.ddrWriteBWLimit = pContextNnConfig->customizedFeature.ddrWriteBWLimit;
    pArchNnConfig->customizedFeature.ddrTotalBWLimit = pContextNnConfig->customizedFeature.ddrTotalBWLimit;
    pArchNnConfig->customizedFeature.axiSramReadBWLimit = pContextNnConfig->customizedFeature.axiSramReadBWLimit;
    pArchNnConfig->customizedFeature.axiSramWriteBWLimit = pContextNnConfig->customizedFeature.axiSramWriteBWLimit;
    pArchNnConfig->customizedFeature.axiSramTotalBWLimit = pContextNnConfig->customizedFeature.axiSramTotalBWLimit;
    pArchNnConfig->customizedFeature.axiBusReadBWLimit = pContextNnConfig->customizedFeature.axiBusReadBWLimit;
    pArchNnConfig->customizedFeature.axiBusWriteBWLimit = pContextNnConfig->customizedFeature.axiBusWriteBWLimit;
    pArchNnConfig->customizedFeature.axiBusTotalBWLimit = pContextNnConfig->customizedFeature.axiBusTotalBWLimit;
    pArchNnConfig->customizedFeature.vipSWTiling = pContextNnConfig->customizedFeature.vipSWTiling;
    pArchNnConfig->customizedFeature.ddrLatency = pContextNnConfig->customizedFeature.ddrLatency;
    pArchNnConfig->customizedFeature.freqInMHZ = pContextNnConfig->customizedFeature.freqInMHZ;
    pArchNnConfig->customizedFeature.axiClockFreqInMHZ = pContextNnConfig->customizedFeature.axiClockFreqInMHZ;
    pArchNnConfig->customizedFeature.maxSocOTNumber = pContextNnConfig->customizedFeature.maxSocOTNumber;
    pArchNnConfig->customizedFeature.nnWriteWithoutUSC = pContextNnConfig->customizedFeature.nnWriteWithoutUSC;
    pArchNnConfig->customizedFeature.depthWiseSupport = pContextNnConfig->customizedFeature.depthWiseSupport;
    pArchNnConfig->customizedFeature.vipVectorPrune = pContextNnConfig->customizedFeature.vipVectorPrune;
    /* archNN_UNIFIED_FEATURE */ /* part of pArchNnConfig->unifiedFeature use int while pContextNnConfig->unifiedFeature using bit filed */
    pArchNnConfig->unifiedFeature.nnUSCCacheSize = pContextNnConfig->unifiedFeature.nnUSCCacheSize;
    pArchNnConfig->unifiedFeature.nnCmdSizeInBytes = pContextNnConfig->unifiedFeature.nnCmdSizeInBytes;
    pArchNnConfig->unifiedFeature.tpCmdSizeInBytes = pContextNnConfig->unifiedFeature.tpCmdSizeInBytes;
    pArchNnConfig->unifiedFeature.vipCoefDecodePerf = pContextNnConfig->unifiedFeature.vipCoefDecodePerf;
    pArchNnConfig->unifiedFeature.vipCachedReadFromSram = pContextNnConfig->unifiedFeature.vipCachedReadFromSram;
    pArchNnConfig->unifiedFeature.vipImagePartialCache = pContextNnConfig->unifiedFeature.vipImagePartialCache;
    pArchNnConfig->unifiedFeature.lanesPerConv = pContextNnConfig->unifiedFeature.lanesPerConv;
    pArchNnConfig->unifiedFeature.maxTileSize = pContextNnConfig->unifiedFeature.maxTileSize;
    pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix = pContextNnConfig->unifiedFeature.fullCacheKernelHeadFix;
    pArchNnConfig->unifiedFeature.conv1x1HalfPerformance = pContextNnConfig->unifiedFeature.conv1x1HalfPerformance;
    pArchNnConfig->unifiedFeature.per3DTileBubbleFix = pContextNnConfig->unifiedFeature.per3DTileBubbleFix;
    pArchNnConfig->unifiedFeature.cacheLineModeDisabled = pContextNnConfig->unifiedFeature.cacheLineModeDisabled;
    pArchNnConfig->unifiedFeature.tpReOrderFix = pContextNnConfig->unifiedFeature.tpReOrderFix;
    pArchNnConfig->unifiedFeature.zdp3NoCompressFix = pContextNnConfig->unifiedFeature.zdp3NoCompressFix;
    pArchNnConfig->unifiedFeature.asyncCopyPerfFix = pContextNnConfig->unifiedFeature.asyncCopyPerfFix;
    pArchNnConfig->unifiedFeature.accurateTileBW = pContextNnConfig->unifiedFeature.accurateTileBW;
    pArchNnConfig->unifiedFeature.zxdp3KernelReadConflictFix = pContextNnConfig->unifiedFeature.zxdp3KernelReadConflictFix;
    pArchNnConfig->unifiedFeature.axiSramSlowedDownByAddr = pContextNnConfig->unifiedFeature.axiSramSlowedDownByAddr;
    pArchNnConfig->unifiedFeature.slowNNReqArbitrationFix = pContextNnConfig->unifiedFeature.slowNNReqArbitrationFix;
    pArchNnConfig->unifiedFeature.singlePortAccBuffer = pContextNnConfig->unifiedFeature.singlePortAccBuffer;
    pArchNnConfig->unifiedFeature.convOutFifoDepthFix = pContextNnConfig->unifiedFeature.convOutFifoDepthFix;
    pArchNnConfig->unifiedFeature.smallBatchEnable = pContextNnConfig->unifiedFeature.smallBatchEnable;
    pArchNnConfig->unifiedFeature.axiSramOnlySWTiling = pContextNnConfig->unifiedFeature.axiSramOnlySWTiling;
    pArchNnConfig->unifiedFeature.imageNotPackedInSram = pContextNnConfig->unifiedFeature.imageNotPackedInSram;
    pArchNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix = pContextNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix;
    pArchNnConfig->unifiedFeature.xyOffsetLimitationFix = pContextNnConfig->unifiedFeature.xyOffsetLimitationFix;
    pArchNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix = pContextNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix;
    pArchNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix = pContextNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix;

    /* archNN_DERIVED_FEATURE */
    pArchNnConfig->derivedFeature.nnDPAmount = pContextNnConfig->derivedFeature.nnDPAmount;
    pArchNnConfig->derivedFeature.nnXYDPX = pContextNnConfig->derivedFeature.nnXYDPX;
    pArchNnConfig->derivedFeature.nnXYDPY = pContextNnConfig->derivedFeature.nnXYDPY;
    pArchNnConfig->derivedFeature.nnZDP = pContextNnConfig->derivedFeature.nnZDP;
    pArchNnConfig->derivedFeature.totalLatency = pContextNnConfig->derivedFeature.totalLatency;
    pArchNnConfig->derivedFeature.internalLatency = pContextNnConfig->derivedFeature.internalLatency;
    pArchNnConfig->derivedFeature.ddrReadBWInBytePerCycle = pContextNnConfig->derivedFeature.ddrReadBWInBytePerCycle;
    pArchNnConfig->derivedFeature.ddrWriteBWInBytePerCycle = pContextNnConfig->derivedFeature.ddrWriteBWInBytePerCycle;
    /* set Drv Option */
    pArchOptions->enableTP = pContextOptions->enableTP;
    pArchOptions->enableMultiTP = pContextOptions->enableMultiTP;
    pArchOptions->flagTPFunc = pContextOptions->flagTPFunc;
    pArchOptions->typeTPFunc = pContextOptions->typeTPFunc;
    pArchOptions->enableSRAM = pContextOptions->enableSRAM;
    pArchOptions->enableSramStreamMode = pContextOptions->enableSramStreamMode;
    pArchOptions->enableCNNPerf = pContextOptions->enableCNNPerf;
    pArchOptions->enableBrickMode = pContextOptions->enableBrickMode;
    pArchOptions->enableNonZeroBalance = pContextOptions->enableNonZeroBalance;
    pArchOptions->enableBorderMode = pContextOptions->enableBorderMode;
    pArchOptions->enableTPReorder = pContextOptions->enableTPReorder;
    pArchOptions->enableTPInterleave8 = pContextOptions->enableTPInterleave8;
    pArchOptions->enableTPRTNE = pContextOptions->enableTPRTNE;
    pArchOptions->enableShader = pContextOptions->enableShader;
    pArchOptions->enableNNXYDP9 = pContextOptions->enableNNXYDP9;
    pArchOptions->enableNNXYDP6 = pContextOptions->enableNNXYDP6;
    pArchOptions->enableSwtilingPhase1 = pContextOptions->enableSwtilingPhase1;
    pArchOptions->enableSwtilingPhase2 = pContextOptions->enableSwtilingPhase2;
    pArchOptions->enableSwtilingPhase3 = pContextOptions->enableSwtilingPhase3;
    pArchOptions->enableHandleBranch = pContextOptions->enableHandleBranch;
    pArchOptions->enableNNFirstPixelPooling = pContextOptions->enableNNFirstPixelPooling;
    pArchOptions->enableNNDepthWiseSupport = pContextOptions->enableNNDepthWiseSupport;
    pArchOptions->enablePrintOperaTarget = pContextOptions->enablePrintOperaTarget;
    pArchOptions->enableSaveBinary = pContextOptions->enableSaveBinary;
    pArchOptions->enableGraphCommandBuffer = pContextOptions->enableGraphCommandBuffer;
    pArchOptions->nnFormulaOpt = pContextOptions->nnFormulaOpt;
    pArchOptions->ddrLatency = pContextOptions->ddrLatency;
    pArchOptions->ddrReadBWLimit = pContextOptions->ddrReadBWLimit;
    pArchOptions->ddrWriteBWLimit = pContextOptions->ddrWriteBWLimit;
    pArchOptions->ddrTotalBWLimit = pContextOptions->ddrTotalBWLimit;
    pArchOptions->axiSramReadBWLimit = pContextOptions->axiSramReadBWLimit;
    pArchOptions->axiSramWriteBWLimit = pContextOptions->axiSramWriteBWLimit;
    pArchOptions->axiSramTotalBWLimit = pContextOptions->axiSramTotalBWLimit;
    pArchOptions->axiBusReadBWLimit = pContextOptions->axiBusReadBWLimit;
    pArchOptions->axiBusWriteBWLimit = pContextOptions->axiBusWriteBWLimit;
    pArchOptions->axiBusTotalBWLimit = pContextOptions->axiBusTotalBWLimit;
    pArchOptions->vipSRAMSize = pContextOptions->vipSRAMSize;
    pArchOptions->axiSRAMSize = pContextOptions->axiSRAMSize;
    pArchOptions->graphPerfLogFile = pContextOptions->graphPerfLogFile;
    pArchOptions->nnZeroRunLen = pContextOptions->nnZeroRunLen;
    pArchOptions->tpZeroRunLen = pContextOptions->tpZeroRunLen;
    pArchOptions->enableNNArchPerfPrint = pContextOptions->enableNNArchPerfPrint;
    pArchOptions->enableNNLayerDump = pContextOptions->enableNNLayerDump;
    pArchOptions->enableNNLayerDump_Int = 0;    /* R6.4x has no member of enableNNLayerDump_Int */
    pArchOptions->enableInterleave8 = pContextOptions->enableInterleave8;
    pArchOptions->nnRoundingMode = pContextOptions->nnRoundingMode;
    pArchOptions->vxcShaderSourcePath = pContextOptions->vxcShaderSourcePath;
    pArchOptions->fcZMax = pContextOptions->fcZMax;
    pArchOptions->enableMemPool = pContextOptions->enableMemPool;
    pArchOptions->memPoolSize = pContextOptions->memPoolSize;
    pArchOptions->collectPerfType = pContextOptions->collectPerfType;
    pArchOptions->enableGraphAdapter = pContextOptions->enableGraphAdapter;
    pArchOptions->enableZdpOpt = pContextOptions->enableZdpOpt;
    pArchOptions->do1xnAfterSwtiling = pContextOptions->do1xnAfterSwtiling;
    pArchOptions->nn1x1To1xN = pContextOptions->nn1x1To1xN;
    pArchOptions->enableGraphTranform = pContextOptions->enableGraphTranform;
    pArchOptions->enableGraphWAR7 = pContextOptions->enableGraphWAR7;
    pArchOptions->enableGraphPadConv = pContextOptions->enableGraphPadConv;
    pArchOptions->enableGraphMerge = pContextOptions->enableGraphMerge;
    pArchOptions->enableGraphDump = pContextOptions->enableGraphDump;
    pArchOptions->enableTransformNMConv = pContextOptions->enableTransformNMConv;
    pArchOptions->enableGraphConvertAvgPool2Conv = pContextOptions->enableGraphConvertAvgPool2Conv;
    pArchOptions->enableGraphUnrollDWConv = pContextOptions->enableGraphUnrollDWConv;
    pArchOptions->enableGraphOptimizationToTest = pContextOptions->enableGraphOptimizationToTest;
    pArchOptions->enableGraphConvertBatchFC2NNConv = pContextOptions->enableGraphConvertBatchFC2NNConv;
    pArchOptions->enableGraphConvertTensorAdd = pContextOptions->enableGraphConvertTensorAdd;
    pArchOptions->enableGraphEltwiseOpShape = pContextOptions->enableGraphEltwiseOpShape;
    pArchOptions->enableGraphConvertConv2Fc = pContextOptions->enableGraphConvertConv2Fc;
    pArchOptions->enableGraphSwaplayer = pContextOptions->enableGraphSwaplayer;
    pArchOptions->enableGraphReshapelayer = pContextOptions->enableGraphReshapelayer;
    pArchOptions->enableGraphConcalayer = pContextOptions->enableGraphConcalayer;
    pArchOptions->enableGraphMergeTranspose = pContextOptions->enableGraphMergeTranspose;
    pArchOptions->enableGraphDeleteRelu = pContextOptions->enableGraphDeleteRelu;
    pArchOptions->enableGraphDeleteSqueeze = pContextOptions->enableGraphDeleteSqueeze;
    pArchOptions->freqInMHZ = pContextOptions->freqInMHZ;
    pArchOptions->axiClockFreqInMHZ = pContextOptions->axiClockFreqInMHZ;
    pArchOptions->maxSocOTNumber = pContextOptions->maxSocOTNumber;
    pArchOptions->enableHuffmanEnhancement = pContextOptions->enableHuffmanEnhancement;
    pArchOptions->enableTPHuffman = pContextOptions->enableTPHuffman;
    pArchOptions->enableMultiVIPCombined = pContextOptions->enableMultiVIPCombined;
    pArchOptions->enableVectorPrune = pContextOptions->enableVectorPrune;
    pArchOptions->enableYUV2RGBScaler = pContextOptions->enableYUV2RGBScaler;
    pArchOptions->enableVIPDEC400 = pContextOptions->enableVIPDEC400;
    pArchOptions->enableCacheBinaryGraph = pContextOptions->enableCacheBinaryGraph;
    pArchOptions->enableOpsDebugInfo = pContextOptions->enableOpsDebugInfo;
    //pArchOptions->enableMemOptimization = pContextOptions->enableMemOptimization;

    /* default set pArchOptions->enableSubnetworkSplitting to 1 */
    pArchOptions->enableSubnetworkSplitting = 1;
    return 0;
}

/***********************************************************************************
* Function:     updateConfigration
* Description:  Except pArchNnConfig and pArchOptions, there are still some configration defined
                and need to check during calculation.
                These configration will be set in here
* Input:        pArchDataFeature:  extra data feature info, will be filled in this function
*               pArchNnConfig:    dependency
* Ouput:        vx_status:       success or failed
***************************************************************************************/
static vx_status updateConfigration(archNN_DATABASE_FEATURE *pArchDataFeature,arch_nn_config *pArchNnConfig, vx_context context)
{
    vx_drv_option *pContextOptions = &(context->options);
    pArchDataFeature->swtilingPhase1Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1);
    pArchDataFeature->swtilingPhase2Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE2);
    pArchDataFeature->swtilingPhase3Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3);
    pArchDataFeature->zdp3Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3);
    pArchDataFeature->zdp6Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP6);
    pArchDataFeature->xydp9Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_XYDP9);
    pArchDataFeature->coefComEnhancement = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COEF_COMPRESSION_ENHANCEMENT);
    pArchDataFeature->tpComEnhancement = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_COEF_COMPRESSION_ENHANCEMENT);
    pArchDataFeature->vipDec400Enable = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_VIP_DEC400);

    pArchDataFeature->nnStrideEnable = gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_STRIDE);
    pArchDataFeature->nnZdp3Enable = gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_ZDP3);
    pArchDataFeature->nnZdp6Enable = gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_ZDP6);


    /* Check feature database and set the value */
    pArchDataFeature->depthWiseMergeSupport = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_DW_1x1_CONV_MERGE);

    //pArchDataFeature->nnSlowOutput = 1;                     /* hard code to 1 */
    //pArchDataFeature->partialKernelCacheInternalFix = 0;    /* hard code to 0, has not fix now */

    pArchDataFeature->noNarrowPostProcessPipe = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NO_NARROW_POST_PROCESS_PIPE);
    pArchDataFeature->nnSlowOutput = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_SLOW_OUTPUT);
    pArchDataFeature->prefetchNNCommandKernelHeader = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SMALL_BATCH);       /* small batch */

    pArchDataFeature->partialKernelCacheInternalFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_KERNEL_SIZE_WASTE_IN_PARTIAL_MODE_FIX);
    pArchDataFeature->internalKernelReadBottleneckFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_KERNEL_VIP_SRAM_READ_BW_LIMITATION_FIX);;
    pArchDataFeature->ImgPopPipelinePauseFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMG_POP_PIPELINE_PAUSE_FIX);;

    /* out put for log, fix value for now and my update in the furture */
    pArchDataFeature->ddrAlign = 0;
    pArchDataFeature->inlinesPerCycle = 3;              /* set to 3 as default */

    /* bug 2033 */
    pArchDataFeature->fullCacheIntervalFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FULLCACHE_KERNEL_INTERLEAVE_FIX);

    pArchDataFeature->readReturnArbiterBubbleFix = 0;                /* 2038 */
    pArchDataFeature->nerghborImageDataTransferNotEfficientFix = 0;  /* 2045 */
    pArchDataFeature->tpVipSramOt1Fix = 0;                           /* 2050 */

    /* bug 2046 */
    pArchDataFeature->drJdDiffConditionForCacheLineModePreFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_DR_JD_DIFF_CONDITION_FOR_CACHELINE_MODE_PRE_FIX);

    /* for verify, set the flag */
    pArchDataFeature->nnTranspose = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_TRANSPOSE);
    pArchDataFeature->specificDDRLimitByBurst = 0;

    /* Please NOTE that the value set here is just for log output, the actual value setting is in gc_vx_nn_util.c */
    /* DDR sustained BW burst, these data should be get from App in the furture, hard code to 16 for now */
    pArchDataFeature->specificDDRLimitByBurst = pContextOptions->specificDDRLimitByBurst;

    pArchDataFeature->ddrReadSustainedBw64BBurst = pContextOptions->ddrReadSustainedBw64BBurst;
    pArchDataFeature->ddrReadSustainedBw128BBurst = pContextOptions->ddrReadSustainedBw128BBurst;
    pArchDataFeature->ddrReadSustainedBw256BBurst = pContextOptions->ddrReadSustainedBw256BBurst;
    pArchDataFeature->ddrMaskWriteSustainedBw64BBurst = pContextOptions->ddrMaskWriteSustainedBw64BBurst;
    pArchDataFeature->ddrMaskWriteSustainedBw128BBurst = pContextOptions->ddrMaskWriteSustainedBw128BBurst;
    pArchDataFeature->ddrMaskWriteSustainedBw256BBurst = pContextOptions->ddrMaskWriteSustainedBw256BBurst;
    pArchDataFeature->ddrNonMaskWriteSustainedBw64BBurst = pContextOptions->ddrNonMaskWriteSustainedBw64BBurst;
    pArchDataFeature->ddrNonMaskWriteSustainedBw128BBurst = pContextOptions->ddrNonMaskWriteSustainedBw128BBurst;
    pArchDataFeature->ddrNonMaskWriteSustainedBw256BBurst =pContextOptions->ddrNonMaskWriteSustainedBw256BBurst;

    /* VIPSRAM ASYNC FIFO */
    pArchDataFeature->vipSramAsyncFifo = 0;             /* From Feature DB */

    /* Burst size */
    pArchDataFeature->nnDDRBurstSize = 64;              /* default set as 64, some of the config need to set as 256, will add into freature DB */
    pArchDataFeature->nnLargeBurstSize = 0;             /* There is no related setting in driver, so it should be always 0 in driver now */

    /* tp comp 2pixel per cycle */
    pArchDataFeature->tpComp2pixelPerCycle = 0;;

    /* graph batch count */
    pArchDataFeature->grachBatchCount = pContextOptions->graphBatchCount;
    return 0;
}



/*#define DUMP_PARAMETERS 1*/
#if DUMP_PARAMETERS
/* for test only */
void printConfig(arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions, archHAL_CHIPIDENTITY *pChipIdentity,archNN_DATABASE_FEATURE *pArchDataFeature)
{
    /* arch_nn_config */
    vxInfo("************************************  arch_nn_config  ********************************************\n");
    vxInfo("isSet is %d.\n",pArchNnConfig->isSet);
    /* archNN_FIXED_FEATURE */
    vxInfo("============================== archNN_FIXED_FEATURE ==============================\n");
    vxInfo("vipCoreCount is %d.\n",pArchNnConfig->fixedFeature.vipCoreCount);
    vxInfo("nnCoreCount is %d.\n",pArchNnConfig->fixedFeature.nnCoreCount);
    vxInfo("nnCoreCountInt8 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountInt8);
    vxInfo("nnCoreCountInt16 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountInt16);
    vxInfo("nnCoreCountFloat16 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountFloat16);
    vxInfo("nnMadPerCore is %d.\n",pArchNnConfig->fixedFeature.nnMadPerCore);
    vxInfo("nnInputBufferDepth is %d.\n",pArchNnConfig->fixedFeature.nnInputBufferDepth);
    vxInfo("nnAccumBufferDepth is %d.\n",pArchNnConfig->fixedFeature.nnAccumBufferDepth);
    vxInfo("nnFCNonPrunAccel is %d.\n",pArchNnConfig->fixedFeature.nnFCNonPrunAccel);
    vxInfo("nnInImageOffsetBits is %d.\n",pArchNnConfig->fixedFeature.nnInImageOffsetBits);
    vxInfo("tpCoreCount is %d.\n",pArchNnConfig->fixedFeature.tpCoreCount);
    vxInfo("tpPwlLUTCount is %d.\n",pArchNnConfig->fixedFeature.tpPwlLUTCount);
    vxInfo("tpPwlLUTSize is %d.\n",pArchNnConfig->fixedFeature.tpPwlLUTSize);
    vxInfo("vip7Version is %d.\n",pArchNnConfig->fixedFeature.vip7Version);
    vxInfo("vipBrickMode is %d.\n",pArchNnConfig->fixedFeature.vipBrickMode);
    vxInfo("tpReorderInImageSize is %d.\n",pArchNnConfig->fixedFeature.tpReorderInImageSize);
    vxInfo("tpliteCoreCount is %d.\n",pArchNnConfig->fixedFeature.tpliteCoreCount);
    vxInfo("nnFP16XYDPX is %d.\n",pArchNnConfig->fixedFeature.nnFP16XYDPX);
    vxInfo("nnFP16XYDPY is %d.\n",pArchNnConfig->fixedFeature.nnFP16XYDPY);
    vxInfo("nnFP16ZDP is %d.\n",pArchNnConfig->fixedFeature.nnFP16ZDP);
    vxInfo("zrlBits is %d.\n",pArchNnConfig->fixedFeature.zrlBits);
    vxInfo("uscCacheControllers is %d.\n",pArchNnConfig->fixedFeature.uscCacheControllers);
    vxInfo("uscBanks is %d.\n",pArchNnConfig->fixedFeature.uscBanks);
    vxInfo("nnLanesPerOutCycle is %d.\n",pArchNnConfig->fixedFeature.nnLanesPerOutCycle);
    vxInfo("maxOTNumber is %d.\n",pArchNnConfig->fixedFeature.maxOTNumber);
    vxInfo("equivalentVipsramWidthInByte is %d.\n",pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte);
    vxInfo("shaderCoreCount is %d.\n",pArchNnConfig->fixedFeature.shaderCoreCount);

    /* archNN_CUSTOMIZED_FEATURE */
    vxInfo("============================== archNN_CUSTOMIZED_FEATURE ==============================\n");
    vxInfo("vipSRAMSize is 0x%x.\n",pArchNnConfig->customizedFeature.vipSRAMSize);
    vxInfo("axiSRAMSize is 0x%x.\n",pArchNnConfig->customizedFeature.axiSRAMSize);
    vxInfo("ddrReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrReadBWLimit);
    vxInfo("ddrWriteBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrWriteBWLimit);
    vxInfo("ddrTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrTotalBWLimit);
    vxInfo("axiSramReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiSramReadBWLimit);
    vxInfo("axiSramWriteBWLimit is %f\n",pArchNnConfig->customizedFeature.axiSramWriteBWLimit);
    vxInfo("axiSramTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiSramTotalBWLimit);
    vxInfo("axiBusReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusReadBWLimit);
    vxInfo("axiBusWriteBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusWriteBWLimit);
    vxInfo("axiBusTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusTotalBWLimit);
    vxInfo("vipSWTiling is %d.\n",pArchNnConfig->customizedFeature.vipSWTiling);
    vxInfo("ddrLatency is %d.\n",pArchNnConfig->customizedFeature.ddrLatency);
    vxInfo("freqInMHZ is %d.\n",pArchNnConfig->customizedFeature.freqInMHZ);
    vxInfo("axiClockFreqInMHZ is %d.\n",pArchNnConfig->customizedFeature.axiClockFreqInMHZ);
    vxInfo("maxSocOTNumber is %d.\n",pArchNnConfig->customizedFeature.maxSocOTNumber);
    vxInfo("nnWriteWithoutUSC is %d.\n",pArchNnConfig->customizedFeature.nnWriteWithoutUSC);
    vxInfo("depthWiseSupport is %d.\n",pArchNnConfig->customizedFeature.depthWiseSupport);
    vxInfo("vipVectorPrune is %d.\n",pArchNnConfig->customizedFeature.vipVectorPrune);

    /* archNN_UNIFIED_FEATURE */
    vxInfo("============================== archNN_UNIFIED_FEATURE ==============================\n");
    vxInfo("nnUSCCacheSize is %d.\n",pArchNnConfig->unifiedFeature.nnUSCCacheSize);
    vxInfo("nnCmdSizeInBytes is %d.\n",pArchNnConfig->unifiedFeature.nnCmdSizeInBytes);
    vxInfo("tpCmdSizeInBytes is %d.\n",pArchNnConfig->unifiedFeature.tpCmdSizeInBytes);
    vxInfo("vipCoefDecodePerf is %d.\n",pArchNnConfig->unifiedFeature.vipCoefDecodePerf);
    vxInfo("vipCachedReadFromSram is %d.\n",pArchNnConfig->unifiedFeature.vipCachedReadFromSram);
    vxInfo("vipImagePartialCache is %d.\n",pArchNnConfig->unifiedFeature.vipImagePartialCache);
    vxInfo("lanesPerConv is %d.\n",pArchNnConfig->unifiedFeature.lanesPerConv);
    vxInfo("maxTileSize is %d.\n",pArchNnConfig->unifiedFeature.maxTileSize);
    vxInfo("fullCacheKernelHeadFix is %d.\n",pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix);
    vxInfo("conv1x1HalfPerformance is %d.\n",pArchNnConfig->unifiedFeature.conv1x1HalfPerformance);
    vxInfo("per3DTileBubbleFix is %d.\n",pArchNnConfig->unifiedFeature.per3DTileBubbleFix);
    vxInfo("cacheLineModeDisabled is %d.\n",pArchNnConfig->unifiedFeature.cacheLineModeDisabled);
    vxInfo("tpReOrderFix is %d.\n",pArchNnConfig->unifiedFeature.tpReOrderFix);
    vxInfo("zdp3NoCompressFix is %d.\n",pArchNnConfig->unifiedFeature.zdp3NoCompressFix);
    vxInfo("asyncCopyPerfFix is %d.\n",pArchNnConfig->unifiedFeature.asyncCopyPerfFix);
    vxInfo("accurateTileBW is %d.\n",pArchNnConfig->unifiedFeature.accurateTileBW);
    vxInfo("zxdp3KernelReadConflictFix is %d.\n",pArchNnConfig->unifiedFeature.zxdp3KernelReadConflictFix);
    vxInfo("axiSramSlowedDownByAddr is %d.\n",pArchNnConfig->unifiedFeature.axiSramSlowedDownByAddr);
    vxInfo("slowNNReqArbitrationFix is %d.\n",pArchNnConfig->unifiedFeature.slowNNReqArbitrationFix);
    vxInfo("singlePortAccBuffer is %d.\n",pArchNnConfig->unifiedFeature.singlePortAccBuffer);
    vxInfo("convOutFifoDepthFix is %d.\n",pArchNnConfig->unifiedFeature.convOutFifoDepthFix);
    vxInfo("smallBatchEnable is %d.\n",pArchNnConfig->unifiedFeature.smallBatchEnable);
    vxInfo("axiSramOnlySWTiling is %d.\n",pArchNnConfig->unifiedFeature.axiSramOnlySWTiling);
    vxInfo("imageNotPackedInSram is %d.\n",pArchNnConfig->unifiedFeature.imageNotPackedInSram);
    vxInfo("coefDeltaCordOverFlowZRL8BitFix is %d.\n",pArchNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix);
    vxInfo("lowEfficiencyOfIDWriteImgBufFix is %d.\n",pArchNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix);
    vxInfo("xyOffsetLimitationFix is %d.\n",pArchNnConfig->unifiedFeature.xyOffsetLimitationFix);
    vxInfo("kernelPerCoreLTOneThirdCoefFix is %d.\n",pArchNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix);


    /* archNN_DERIVED_FEATURE */
    vxInfo("============================== archNN_DERIVED_FEATURE ==============================\n");
    vxInfo("nnDPAmount is %d.\n",pArchNnConfig->derivedFeature.nnDPAmount);
    vxInfo("nnXYDPX is %d.\n",pArchNnConfig->derivedFeature.nnXYDPX);
    vxInfo("nnXYDPY is %d.\n",pArchNnConfig->derivedFeature.nnXYDPY);
    vxInfo("nnZDP is %d.\n",pArchNnConfig->derivedFeature.nnZDP);
    vxInfo("totalLatency is %d.\n",pArchNnConfig->derivedFeature.totalLatency);
    vxInfo("internalLatency is %d.\n",pArchNnConfig->derivedFeature.internalLatency);
    vxInfo("ddrReadBWInBytePerCycle is %f.\n",pArchNnConfig->derivedFeature.ddrReadBWInBytePerCycle);
    vxInfo("ddrWriteBWInBytePerCycle is %f.\n",pArchNnConfig->derivedFeature.ddrWriteBWInBytePerCycle);

    /* arch_drv_option */
    vxInfo("************************************  arch_drv_option  ********************************************\n");
    vxInfo("enableTP is %d.\n",pArchOptions->enableTP);
    vxInfo("enableMultiTP is %d.\n",pArchOptions->enableMultiTP);
    vxInfo("flagTPFunc is %s.\n",pArchOptions->flagTPFunc);
    vxInfo("typeTPFunc is %s.\n",pArchOptions->typeTPFunc);
    vxInfo("enableSRAM is %d.\n",pArchOptions->enableSRAM);
    vxInfo("enableSramStreamMode is %d.\n",pArchOptions->enableSramStreamMode);
    vxInfo("enableCNNPerf is %d.\n",pArchOptions->enableCNNPerf);
    vxInfo("enableBrickMode is %d.\n",pArchOptions->enableBrickMode);
    vxInfo("enableNonZeroBalance is %d.\n",pArchOptions->enableNonZeroBalance);
    vxInfo("enableBorderMode is %d.\n",pArchOptions->enableBorderMode);
    vxInfo("enableTPReorder is %d.\n",pArchOptions->enableTPReorder);
    vxInfo("enableTPInterleave8 is %d.\n",pArchOptions->enableTPInterleave8);
    vxInfo("enableTPRTNE is %d.\n",pArchOptions->enableTPRTNE);
    vxInfo("enableShader is %d.\n",pArchOptions->enableShader);
    vxInfo("enableNNXYDP9 is %d.\n",pArchOptions->enableNNXYDP9);
    vxInfo("enableNNXYDP6 is %d.\n",pArchOptions->enableNNXYDP6);
    vxInfo("enableSwtilingPhase1 is %d.\n",pArchOptions->enableSwtilingPhase1);
    vxInfo("enableSwtilingPhase2 is %d.\n",pArchOptions->enableSwtilingPhase2);
    vxInfo("enableSwtilingPhase3 is %d.\n",pArchOptions->enableSwtilingPhase3);
    vxInfo("enableHandleBranch is %d.\n",pArchOptions->enableHandleBranch);
    vxInfo("enableNNFirstPixelPooling is %d.\n",pArchOptions->enableNNFirstPixelPooling);
    vxInfo("enableNNDepthWiseSupport is %d.\n",pArchOptions->enableNNDepthWiseSupport);
    vxInfo("enablePrintOperaTarget is %d.\n",pArchOptions->enablePrintOperaTarget);
    vxInfo("enableSaveBinary is %d.\n",pArchOptions->enableSaveBinary);
    vxInfo("enableGraphCommandBuffer is %d.\n",pArchOptions->enableGraphCommandBuffer);
    vxInfo("nnFormulaOpt is %d.\n",pArchOptions->nnFormulaOpt);
    vxInfo("ddrLatency is %d.\n",pArchOptions->ddrLatency);
    vxInfo("ddrReadBWLimit is %f.\n",pArchOptions->ddrReadBWLimit);
    vxInfo("ddrWriteBWLimit is %f.\n",pArchOptions->ddrWriteBWLimit);
    vxInfo("ddrTotalBWLimit is %f.\n",pArchOptions->ddrTotalBWLimit);
    vxInfo("axiSramReadBWLimit is %f.\n",pArchOptions->axiSramReadBWLimit);
    vxInfo("axiSramWriteBWLimit is %f.\n",pArchOptions->axiSramWriteBWLimit);
    vxInfo("axiSramTotalBWLimit is %f.\n",pArchOptions->axiSramTotalBWLimit);
    vxInfo("axiBusReadBWLimit is %f.\n",pArchOptions->axiBusReadBWLimit);
    vxInfo("axiBusWriteBWLimit is %f.\n",pArchOptions->axiBusWriteBWLimit);
    vxInfo("axiBusTotalBWLimit is %f.\n",pArchOptions->axiBusTotalBWLimit);
    vxInfo("vipSRAMSize is 0x%x.\n",pArchOptions->vipSRAMSize);
    vxInfo("axiSRAMSize is 0x%x.\n",pArchOptions->axiSRAMSize);
    vxInfo("graphPerfLogFile is %s.\n",pArchOptions->graphPerfLogFile);
    vxInfo("nnZeroRunLen is %d.\n",pArchOptions->nnZeroRunLen);
    vxInfo("tpZeroRunLen is %d.\n",pArchOptions->tpZeroRunLen);
    vxInfo("enableNNArchPerfPrint is %d.\n",pArchOptions->enableNNArchPerfPrint);
    vxInfo("enableNNLayerDump is %d.\n",pArchOptions->enableNNLayerDump);
    vxInfo("enableInterleave8 is %d.\n",pArchOptions->enableInterleave8);
    vxInfo("nnRoundingMode is %s.\n",pArchOptions->nnRoundingMode);
    vxInfo("vxcShaderSourcePath is %s.\n",pArchOptions->vxcShaderSourcePath);
    vxInfo("fcZMax is %d.\n",pArchOptions->fcZMax);
    vxInfo("enableMemPool is %d.\n",pArchOptions->enableMemPool);
    vxInfo("memPoolSize is %d.\n",pArchOptions->memPoolSize);
    vxInfo("collectPerfType is %d.\n",pArchOptions->collectPerfType);
    vxInfo("enableGraphAdapter is %d.\n",pArchOptions->enableGraphAdapter);
    vxInfo("enableZdpOpt is %d.\n",pArchOptions->enableZdpOpt);
    vxInfo("do1xnAfterSwtiling is %d.\n",pArchOptions->do1xnAfterSwtiling);
    vxInfo("nn1x1To1xN is %d.\n",pArchOptions->nn1x1To1xN);
    vxInfo("enableGraphTranform is %d.\n",pArchOptions->enableGraphTranform);
    vxInfo("enableGraphWAR7 is %d.\n",pArchOptions->enableGraphWAR7);
    vxInfo("enableGraphPadConv is %d.\n",pArchOptions->enableGraphPadConv);
    vxInfo("enableGraphMerge is %d.\n",pArchOptions->enableGraphMerge);
    vxInfo("enableGraphDump is %d.\n",pArchOptions->enableGraphDump);
    vxInfo("enableTransformNMConv is %d.\n",pArchOptions->enableTransformNMConv);
    vxInfo("enableGraphConvertAvgPool2Conv is %d.\n",pArchOptions->enableGraphConvertAvgPool2Conv);
    vxInfo("enableGraphUnrollDWConv is %d.\n",pArchOptions->enableGraphUnrollDWConv);
    vxInfo("enableGraphOptimizationToTest is %d.\n",pArchOptions->enableGraphOptimizationToTest);
    vxInfo("enableGraphConvertBatchFC2NNConv is %d.\n",pArchOptions->enableGraphConvertBatchFC2NNConv);
    vxInfo("enableGraphConvertTensorAdd is %d.\n",pArchOptions->enableGraphConvertTensorAdd);
    vxInfo("enableGraphEltwiseOpShape is %d.\n",pArchOptions->enableGraphEltwiseOpShape);
    vxInfo("enableGraphConvertConv2Fc is %d.\n",pArchOptions->enableGraphConvertConv2Fc);
    vxInfo("enableGraphSwaplayer is %d.\n",pArchOptions->enableGraphSwaplayer);
    vxInfo("enableGraphReshapelayer is %d.\n",pArchOptions->enableGraphReshapelayer);
    vxInfo("enableGraphConcalayer is %d.\n",pArchOptions->enableGraphConcalayer);
    vxInfo("enableGraphMergeTranspose is %d.\n",pArchOptions->enableGraphMergeTranspose);
    vxInfo("enableGraphDeleteRelu is %d.\n",pArchOptions->enableGraphDeleteRelu);
    vxInfo("enableGraphDeleteSqueeze is %d.\n",pArchOptions->enableGraphDeleteSqueeze);
    vxInfo("enableGraphAvgPoolandPWConv %d.\n",pArchOptions->enableGraphAvgPoolandPWConv);
    vxInfo("freqInMHZ is %d.\n",pArchOptions->freqInMHZ);
    vxInfo("axiClockFreqInMHZ is %d.\n",pArchOptions->axiClockFreqInMHZ);
    vxInfo("maxSocOTNumber is %d.\n",pArchOptions->maxSocOTNumber);
    vxInfo("enableHuffmanEnhancement is %d.\n",pArchOptions->enableHuffmanEnhancement);
    vxInfo("enableTPHuffman is %d.\n",pArchOptions->enableTPHuffman);
    vxInfo("enableMultiVIPCombined is %d.\n",pArchOptions->enableMultiVIPCombined);
    //vxInfo("enableNNTPParallel is %d.\n",pArchOptions->enableNNTPParallel);
    vxInfo("enableVectorPrune is %d.\n",pArchOptions->enableVectorPrune);
    vxInfo("enableYUV2RGBScaler is %d.\n",pArchOptions->enableYUV2RGBScaler);
    vxInfo("enableVIPDEC400 is %d.\n",pArchOptions->enableVIPDEC400);
    vxInfo("enableCacheBinaryGraph is %d.\n",pArchOptions->enableCacheBinaryGraph);
    vxInfo("enableOpsDebugInfo is %s.\n",pArchOptions->enableOpsDebugInfo);
    //vxInfo("enableMemOptimization is %d.\n",pArchOptions->enableMemOptimization);
    //vxInfo("tpCoreCount is %d.\n",pArchOptions->tpCoreCount);

    /* archHAL_CHIPIDENTITY */
    vxInfo("************************************  archHAL_CHIPIDENTITY  ********************************************\n");
    vxInfo("chipModel is %d.\n",pChipIdentity->chipModel);
    vxInfo("chipRevision is %d.\n",pChipIdentity->chipRevision);
    vxInfo("productID is %d.\n",pChipIdentity->productID);
    vxInfo("customerID is %d.\n",pChipIdentity->customerID);
    vxInfo("ecoID is %d.\n",pChipIdentity->ecoID);
    vxInfo("chipFlags is %d.\n",pChipIdentity->chipFlags);
    vxInfo("platformFlagBits is %d.\n",pChipIdentity->platformFlagBits);

    /* archNN_DATABASE_FEATURE */
    vxInfo("************************************  archNN_DATABASE_FEATURE  ********************************************\n");
    vxInfo("swtilingPhase1Enable is %d.\n",pArchDataFeature->swtilingPhase1Enable);
    vxInfo("swtilingPhase2Enable is %d.\n",pArchDataFeature->swtilingPhase2Enable);
    vxInfo("swtilingPhase3Enable is %d.\n",pArchDataFeature->swtilingPhase3Enable);
    vxInfo("zdp3Enable is %d.\n",pArchDataFeature->zdp3Enable);
    vxInfo("zdp6Enable is %d.\n",pArchDataFeature->zdp6Enable);
    vxInfo("xydp9Enable is %d.\n",pArchDataFeature->xydp9Enable);
    vxInfo("coefComEnhancement is %d.\n",pArchDataFeature->coefComEnhancement);
    vxInfo("tpComEnhancement is %d.\n",pArchDataFeature->tpComEnhancement);
    vxInfo("vipDec400Enable is %d.\n",pArchDataFeature->vipDec400Enable);
    vxInfo("nnStrideEnable is %d.\n",pArchDataFeature->nnStrideEnable);
    vxInfo("nnZdp3Enable is %d.\n",pArchDataFeature->nnZdp3Enable);
    vxInfo("nnZdp6Enable is %d.\n",pArchDataFeature->nnZdp6Enable);
    vxInfo("depthWiseMergeSupport is %d.\n",pArchDataFeature->depthWiseMergeSupport);
    vxInfo("nnSlowOutput is %d.\n",pArchDataFeature->nnSlowOutput);
    vxInfo("noNarrowPostProcessPipe is %d.\n",pArchDataFeature->noNarrowPostProcessPipe);
    vxInfo("prefetchNNCommandKernelHeader is %d.\n",pArchDataFeature->prefetchNNCommandKernelHeader);
    vxInfo("partialKernelCacheInternalFix is %d.\n",pArchDataFeature->partialKernelCacheInternalFix);
    vxInfo("internalKernelReadBottleneckFix is %d.\n",pArchDataFeature->internalKernelReadBottleneckFix);
    vxInfo("depthWiseMergeSupport is %d.\n",pArchDataFeature->ImgPopPipelinePauseFix);
    vxInfo("nnSlowOutput is %d.\n",pArchDataFeature->ddrAlign);
    vxInfo("noNarrowPostProcessPipe is %d.\n",pArchDataFeature->inlinesPerCycle);
    vxInfo("nnTranspose is %d.\n",pArchDataFeature->nnTranspose);
    vxInfo("specificDDRLimitByBurst is %d.\n",pArchDataFeature->specificDDRLimitByBurst);
}

void printGraphInfo(archModelGraphInfo * GraphInfo, vx_uint32 index)
{
    vx_uint32 i = 0;
    vxInfo("Index is %d, Print Graph Info:==================================================\n",index);
    vxInfo("layerId is %d.\n",GraphInfo->layerId);
    vxInfo("layerType is %d.\n",GraphInfo->layerType);
    vxInfo("absId is %d.\n",GraphInfo->absId);
    vxInfo("operationId is %d.\n",GraphInfo->operationId);
    vxInfo("opType is %d.\n",GraphInfo->opType);
    vxInfo("opTarget is %d.\n",GraphInfo->opTarget);

    /* up and down */
    vxInfo("upStreamLayerCount is %d.\n",GraphInfo->upStreamLayerCount);
    vxInfo("upStreamLayer:    \n");
    for(i = 0; i < GraphInfo->upStreamLayerCount; i++)
        vxInfo("        %d: layer_id is %d, type is %d\n",i,GraphInfo->upStreamLayer[i],GraphInfo->upStreamLayerType[i]);

    vxInfo("upOpCount is %d.\n",GraphInfo->upStreamOpCount);
    vxInfo("upOp:    \n");
    for(i = 0; i < GraphInfo->upStreamOpCount; i++)
        vxInfo("        %d: upop_id is %d, type is %d, absId is %d.\n",i,GraphInfo->upStreamOp[i],GraphInfo->upStreamOpType[i],GraphInfo->parentAbsId[i]);

    vxInfo("downStreamLayerCount is %d.\n",GraphInfo->downStreamLayerCount);
    vxInfo("downStreamLayer:    \n");
    for(i = 0; i < GraphInfo->downStreamLayerCount; i++)
        vxInfo("        %d: down_id is %d, type is %d\n",i,GraphInfo->downStreamLayer[i],GraphInfo->downStreamLayerType[i]);

    vxInfo("downOpCount is %d.\n",GraphInfo->downStreamOpCount);
    vxInfo("downOp:    \n");
    for(i = 0; i < GraphInfo->downStreamOpCount; i++)
        vxInfo("        %d: downop_id is %d, type is %d, absId is %d.\n",i,GraphInfo->downStreamOp[i],GraphInfo->downStreamOpType[i],GraphInfo->childAbsId[i]);

    /* In/Out */
    vxInfo("origInX is %d\n",GraphInfo->origInX);
    vxInfo("origInY is %d\n",GraphInfo->origInY);
    vxInfo("origInZ is %d\n",GraphInfo->origInZ);
    vxInfo("nnOutX is %d\n",GraphInfo->nnOutX);
    vxInfo("nnOutY is %d\n",GraphInfo->nnOutY);
    vxInfo("nnOutZ is %d\n",GraphInfo->nnOutZ);
    vxInfo("subNNOutX is %d\n",GraphInfo->subNNOutX);
    vxInfo("subNNOutY is %d\n",GraphInfo->subNNOutY);
    vxInfo("subNNOutZ is %d\n",GraphInfo->subNNOutZ);
    vxInfo("finalOutX is %d\n",GraphInfo->finalOutX);
    vxInfo("finalOutY is %d\n",GraphInfo->finalOutY);
    vxInfo("finalOutZ is %d\n",GraphInfo->finalOutZ);
    vxInfo("kx is %d\n",GraphInfo->kx);
    vxInfo("ky is %d\n",GraphInfo->ky);
    vxInfo("kz is %d\n",GraphInfo->kz);
    vxInfo("pollingSize is %d\n",GraphInfo->pollingSize);
    vxInfo("pollingStride is %d\n",GraphInfo->pollingStride);
    vxInfo("inputDataSize is %d\n",GraphInfo->inputDataSize);
    vxInfo("outputDataSize is %d\n",GraphInfo->outputDataSize);
    vxInfo("isFp16 is %d\n",GraphInfo->isFp16);
    vxInfo("xOffset is %d\n",GraphInfo->xOffset);
    vxInfo("yOffset is %d\n",GraphInfo->yOffset);
    vxInfo("coefNonZeroRatio is %f\n",GraphInfo->coefNonZeroRatio);
    vxInfo("coefCompression is %f\n",GraphInfo->coefCompression);
    vxInfo("imageCompression is %f\n",GraphInfo->imageCompression);
    vxInfo("imageNonZeroRatio is %f\n",GraphInfo->imageNonZeroRatio);
}

void printOpInfo(archModelOpInfo ** archOp, vx_uint32 totalCount)
{
    vx_uint32 i = 0;
    vxInfo("Print ArchOp, totalCount is %d.\n",totalCount);
    for (i = 0; i< totalCount; i++)
    {
        vxInfo("\nArchOp index %d:\n",i);
        vxInfo("TP type is %d\n",archOp[i]->tpType);
        vxInfo("absoluteOperationID is %d\n",archOp[i]->absoluteOperationID);
        vxInfo("uid is %d\n",archOp[i]->uid);
        vxInfo("layerId is %d\n",archOp[i]->layerId);
        vxInfo("layerName is %s\n",archOp[i]->layerName);
        vxInfo("operationId is %d\n",archOp[i]->operationId);
        vxInfo("upLayerCount is %d\n",archOp[i]->upLayerCount);
        vxInfo("upOpCount is %d\n",archOp[i]->upOpCount);
        vxInfo("parentOpId is %d\n",archOp[i]->parentOpId[0],archOp[i]->parentOpId[1],archOp[i]->parentOpId[2]);
        vxInfo("parentOpType is %d\n",archOp[i]->parentOpType[0],archOp[i]->parentOpType[1],archOp[i]->parentOpType[2]);
        vxInfo("parentLayer is %d\n",archOp[i]->parentLayer[0],archOp[i]->parentLayer[1],archOp[i]->parentLayer[2]);
        vxInfo("parentAbsId is %d\n",archOp[i]->parentAbsId[0],archOp[i]->parentAbsId[1],archOp[i]->parentAbsId[2]);
        vxInfo("parentLayerType is %d\n",archOp[i]->parentLayerType[0],archOp[i]->parentLayerType[1],archOp[i]->parentLayerType[2]);
        vxInfo("downLayerCount is %d\n",archOp[i]->downLayerCount);
        vxInfo("downOpCount is %d\n",archOp[i]->downOpCount);
        vxInfo("childOpId is %d\n",archOp[i]->childOpId[0],archOp[i]->childOpId[1],archOp[i]->childOpId[2]);
        vxInfo("childOpType is %d\n",archOp[i]->childOpType[0],archOp[i]->childOpType[1],archOp[i]->childOpType[2]);
        vxInfo("childLayer is %d\n",archOp[i]->childLayer[0],archOp[i]->childLayer[1],archOp[i]->childLayer[2]);
        vxInfo("childAbsId is %d\n",archOp[i]->childAbsId[0],archOp[i]->childAbsId[1],archOp[i]->childAbsId[2]);
        vxInfo("childLayerType is %d\n",archOp[i]->childLayerType[0],archOp[i]->childLayerType[1],archOp[i]->childLayerType[2]);

        vxInfo("op is %d\n",archOp[i]->op);
        vxInfo("target is %d\n",archOp[i]->target);
        vxInfo("inx is %d\n",archOp[i]->inx);
        vxInfo("iny is %d\n",archOp[i]->iny);
        vxInfo("inz is %d\n",archOp[i]->inz);
        vxInfo("calcinx is %d\n",archOp[i]->calcinx);
        vxInfo("calciny is %d\n",archOp[i]->calciny);
        vxInfo("origoutx is %d\n",archOp[i]->origoutx);
        vxInfo("origouty is %d\n",archOp[i]->origouty);
        vxInfo("stridex is %d\n",archOp[i]->stridex);
        vxInfo("stridey is %d\n",archOp[i]->stridey);

        vxInfo("kx is %d\n",archOp[i]->kx);
        vxInfo("ky is %d\n",archOp[i]->ky);
        vxInfo("kz is %d\n",archOp[i]->kz);
        vxInfo("bfy is %d\n",archOp[i]->bfy);
        vxInfo("bfz is %d\n",archOp[i]->bfz);
        vxInfo("oz is %d\n",archOp[i]->oz);
        vxInfo("siz is %d\n",archOp[i]->siz);

        vxInfo("psize is %d\n",archOp[i]->psize);
        vxInfo("pstride is %d\n",archOp[i]->pstride);
        vxInfo("xpad is %d\n",archOp[i]->xpad);
        vxInfo("ypad is %d\n",archOp[i]->ypad);
        vxInfo("inputDataSize is %d\n",archOp[i]->inputDataSize);
        vxInfo("outputDataSize is %d\n",archOp[i]->outputDataSize);
        vxInfo("fcmd is %d\n",archOp[i]->fcmd);
        vxInfo("inputDataFormat is %d\n",archOp[i]->inputDataFormat);
        vxInfo("outputDataFormat is %d\n",archOp[i]->outputDataFormat);
        vxInfo("nnCores is %d\n",archOp[i]->nnCores);
        vxInfo("xsize is %d\n",archOp[i]->xsize);
        vxInfo("ysize is %d\n",archOp[i]->ysize);

        vxInfo("pix is %d\n",archOp[i]->pix);
        vxInfo("piy is %d\n",archOp[i]->piy);
        vxInfo("p3 is %d\n",archOp[i]->p3);
        vxInfo("psix is %d\n",archOp[i]->psix);
        vxInfo("psiy is %d\n",archOp[i]->psiy);

        vxInfo("sbuf is %d\n",archOp[i]->sbuf);
        vxInfo("dbuf is %d\n",archOp[i]->dbuf);
        vxInfo("kbuf is %d\n",archOp[i]->kbuf);
        vxInfo("swTilingSegKernelBufSizeInPixel is %d\n",archOp[i]->swTilingSegKernelBufSizeInPixel);
        vxInfo("swTilingSegOutBufSizeInPixel is %d\n",archOp[i]->swTilingSegOutBufSizeInPixel);
        vxInfo("segTotalBufferSizeInPixel is %d\n",archOp[i]->segTotalBufferSizeInPixel);
        vxInfo("swTilingType is %d\n",archOp[i]->swTilingType);

        vxInfo("upStreamLayerCount is %d\n",archOp[i]->upStreamLayerCount);
        vxInfo("downStreamLayerCount is %d\n",archOp[i]->downStreamLayerCount);
        vxInfo("upStreamLayer is %d\n",archOp[i]->upStreamLayer[0],archOp[i]->upStreamLayer[1],archOp[i]->upStreamLayer[2]);
        vxInfo("downStreamLayer is %d\n",archOp[i]->downStreamLayer[0],archOp[i]->downStreamLayer[1],archOp[i]->downStreamLayer[2]);
    }
}
#endif
/***********************************************************************************
* Function:     vxoGraph_PredictPerf
* Description:  Main function entry for Arch Model Predict to driver
* Input:        graph:        The whole graph for this case
* Ouput:        vx_status:    True for success while False if failed
***************************************************************************************/
#define USE_DRIVER_INTERFACE 1          /*  switch to use driver only interface or same with python*/
VX_INTERNAL_API vx_status archGraphPredictPerf(vx_graph graph)
{
    vx_status               vxStatus   = ARCH_SUCCESS;
    vx_uint32               totalCount = 0;
    vx_context              context    = vxoContext_GetFromReference(&graph->base);
    arch_nn_config          archNNConfig;             /* configration for specific case */
    arch_drv_option         archDrvOption;            /* options defined in env */
    archHAL_CHIPIDENTITY    chipIdentity;             /* Chip identify for HW info */
    archNN_DATABASE_FEATURE archDataFeature = {0};    /* case feature definition */
    archModelOpInfo         **opInfo   = NULL;        /* Base opInfo for Arch Model */

#if USE_DRIVER_INTERFACE

#else
    archSwLibContext *pArchSwLibContext = NULL;
    archModelGraphInfo **graphInfo = NULL;
    archSwLibHandle archSwHandle = NULL;
    vx_uint32 i = 0;
#endif

    memset(&archNNConfig,  0, sizeof(arch_nn_config));
    memset(&archDrvOption, 0, sizeof(arch_drv_option));
    initConfigration(&archNNConfig, &archDrvOption, context);

    /* update configuration */
    updateConfigration(&archDataFeature,&archNNConfig, context);

    /* get the chip information */
    gcoHAL_QueryChipIdentityEx(ARCH_NULL, sizeof(archHAL_CHIPIDENTITY),
                               (gcsHAL_CHIPIDENTITY *)&chipIdentity);

#if USE_DRIVER_INTERFACE
    /* Transfer the graph to Arch Model OpInfo */
    opInfo = archTransferParam(graph, &archDataFeature,&totalCount);
    if(opInfo == NULL)
    {
        return ARCH_FAILURE;
    }
#ifdef DUMP_PARAMETERS
    printConfig(&archNNConfig, &archDrvOption, &chipIdentity, &archDataFeature);
    printOpInfo(opInfo,totalCount);
#endif

    /* update up/down stream info (parent/child) */
    updateStreamLayer(graph,opInfo);

    /* Call predict function do the performance analysing */
    vxStatus = archPredictPerf(context->globalData->apm, opInfo, totalCount, &archNNConfig,
                               &archDrvOption, &archDataFeature,&chipIdentity);

    /* release OpInfo */
    deInitArchOpInfo(opInfo, totalCount);
#else

    /* Transfer the graph to Arch Model graph info */
    graphInfo = archTransferGraphInfo(graph, &totalCount);

    /* start to init Arch Sw Lib */
    archSwHandle = archSwLibInit(&archNNConfig, &archDrvOption, totalCount,
                                 &chipIdentity, &archDataFeature);
#ifdef DUMP_PARAMETERS
    printConfig(pArchNnConfig, pArchOptions, &chipIdentity, &archDataFeature);
#endif

    /* Loop to fill layer data */
    for(i = 0; i < totalCount; i++)
    {
        archPredictPerfFillLayer(archSwHandle,graphInfo[i],i);
#ifdef DUMP_PARAMETERS
        /*printGraphInfo(graphInfo[i],i);*/
#endif
    }

    /* start to analysing, since the updateStreamLayer need to do before analysing, expand them here now */
    //archPredictPerfAnalysing(archSwHandle);
    {

        pArchSwLibContext = (archSwLibContext *)archSwHandle;
        if(pArchSwLibContext == NULL)
        {
            /* handle invalid */
            vxInfo("Invalid Handle for Arch Sw Lib.\n");
            return archSTATUS_INVALID_ARGUMENT;
        }
        /* Init OpInfo */
        opInfo = initArchOpInfo(pArchSwLibContext->totalCount);
        if(opInfo == NULL)
        {
            /* Init Arch op Info failed */
            vxError("Init Arch op Info failed.\n");
            deInitArchOpInfo(opInfo,pArchSwLibContext->totalCount);
            return archSTATUS_INVALID_DATA;
        }

        /* Fill in opInfo based on graphInfo */
        vxStatus = archFillOpInfo(opInfo, pArchSwLibContext->graphInfo,
                                  pArchSwLibContext->pArchNnConfig,
                                  pArchSwLibContext->totalCount);
        //printOpInfo(opInfo,totalCount);
        /* extra steps for driver call Arch Sw Lib: update up/down stream info (parent/child) */
        updateStreamLayer(graph,opInfo);
        /*printOpInfo(opInfo,totalCount);*/
        vxStatus = archPredictPerf(opInfo, pArchSwLibContext->totalCount,
                                   pArchSwLibContext->pArchNnConfig,
                                   pArchSwLibContext->pArchOptions,
                                   pArchSwLibContext->pChipIdentity,
                                   pArchSwLibContext->pArchDataFeature);
    }

    /* Deinit the graphInfo struct */
    deInitArchGraphInfo(graphInfo, graph->layer->base.num_operations);

    /* Deinit the OpInfo */
    deInitArchOpInfo(opInfo, pArchSwLibContext->totalCount);

    /* done. deinit the Arch Sw lib */
    archSwLibDeInit(archSwHandle);

#endif

    return vxStatus;

}


/***********************************************************************************
* Function:     calculateArchPerfFromTiling
* Description:  Calculate the Arch Model performance when driver doing SWT
* Input:        Since the operation command will be passed from the called, the nntranspose
                Info will be set in this function
* Ouput:        void
***************************************************************************************/
VX_INTERNAL_API void archCalculateArchPerfFromTiling(
    vx_context context,
    vxnne_layer layer,
    arch_perf perf,
    vxnne_tensor_info input_tiling,
    vxnne_tensor_info output_tiling,
    vx_tensor input,
    vx_tensor output,
    vx_weights_biases_parameter wb,
    vxnne_operation_command  op_command,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type)
{

    /* addjust for C-Arch Sw Lib */
    archNN_DATABASE_FEATURE archDataFeature = {0};
    arch_nn_config archNNConfig;
    arch_drv_option archDrvOption;
    arch_nn_config *pArchNnConfig =  &archNNConfig;
    arch_drv_option *pArchOptions = &archDrvOption;
//    vx_uint32 coreIndex = 0,
    vx_uint32 numCores = 0;

    vx_uint32 kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride, strideX, strideY;
    vx_int32 inputDataSize, outputDataSize, xOffSet, yOffSet;
    vx_op_param conv_cmd = &op_command->parameter;
    vxnne_operation operation = op_command->operation;
    arch_bool axiSramOnlySWTiling = 0;

    /* Init configration */
    memset(pArchNnConfig, 0, sizeof(arch_nn_config));
    memset(pArchOptions, 0, sizeof(arch_drv_option));
    initConfigration(pArchNnConfig, pArchOptions,context);

    /* Init data feature */
    updateConfigration(&archDataFeature, pArchNnConfig, context);

    axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;

    kernelXSize = wb != ARCH_NULL && op_type != VXNNE_OPERATOR_RESHUFFLE ? WB_KERNEL_X(wb) : 1;
    kernelYSize = wb != ARCH_NULL && op_type != VXNNE_OPERATOR_RESHUFFLE ? WB_KERNEL_Y(wb) : 1;
    kernelZSize = wb != ARCH_NULL ? ((op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV && WB_IS_DEPTH_WISE(wb)) ? WB_OUTPUT_Z(wb) : WB_KERNEL_Z(wb))
                                : input_tiling->depth;

    xOffSet = (-1) * conv_cmd->pad_x_left;
    yOffSet = (-1) * conv_cmd->pad_y_top;

    poolingStride = !conv_cmd->pool_size_y ? 1 : gcmMAX(1, conv_cmd->pool_stride);
    poolingSize = gcmMAX(1, conv_cmd->pool_size_y);

    inputDataSize = 8 * TENSOR_DATA_SIZE(input);
    outputDataSize = 8 * TENSOR_DATA_SIZE(output);
    strideX = op_type == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_X(wb) : 1;
    strideY = op_type == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_Y(wb) : 1;

    /********** left perf structure configuration **********/
    perf->info.kx = kernelXSize;
    perf->info.ky = kernelYSize;
    perf->info.kz = kernelZSize;
    perf->info.oinx = TENSOR_VIEW_SIZE_INDEX(input, 0);
    perf->info.oiny = TENSOR_VIEW_SIZE_INDEX(input, 1);
    perf->info.oinz = TENSOR_VIEW_SIZE_INDEX(input, 2);
    perf->info.stridex = strideX;
    perf->info.stridey = strideY;
    perf->info.inputDataSize = inputDataSize;
    perf->info.outputDataSize = outputDataSize;
    perf->info.poolingSize = poolingSize;
    perf->info.poolingStride = poolingStride;
    perf->info.xOffSet = xOffSet;
    perf->info.yOffSet = yOffSet;
    perf->info.inputDataFormat = TENSOR_DATA_TYPE(input);
    perf->info.outputDataFormat = TENSOR_DATA_TYPE(output);
    perf->info.p3 = (poolingSize == 3) ? 1 : 0;

    perf->info.inx = op_type == VXNNE_OPERATOR_POOLING ? input_tiling->width : output_tiling->width;
    perf->info.iny = op_type == VXNNE_OPERATOR_POOLING ? input_tiling->height : output_tiling->height;
    perf->info.outz = output_tiling->depth;
    perf->info.pix = (vx_uint32)ceilf((arch_float32)(output_tiling->width - perf->info.p3) / poolingStride);
    perf->info.piy = (vx_uint32)ceilf((arch_float32)(output_tiling->height - perf->info.p3) / poolingStride);

    if ((op_target != VXNNE_OPERATION_TARGET_TP || op_type == VXNNE_OPERATOR_FULLYCONNECTED) && wb)
        perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(wb, 0), CACHE_ALIGNMENT_SIZE));
    perf->swTilingInfo.origOutX = TENSOR_VIEW_SIZE_INDEX(output, 0);
    perf->swTilingInfo.origOutY = TENSOR_VIEW_SIZE_INDEX(output, 1);
    perf->swTilingInfo.origOutZ = TENSOR_VIEW_SIZE_INDEX(output, 2);
    perf->swTilingInfo.cacheSpace = ARCH_VIP_SRAM_SIZE;
    if (op_type == VXNNE_OPERATOR_CONVOLUTION || op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        /* original output tensor might be reshaped, cannot use original output tensor size at here */
        /* use reshaped sized to calc imageStride and imageSlice */
        perf->swTilingInfo.outImageStride = output_tiling->width;
        perf->swTilingInfo.outImageSlice = output_tiling->width * output_tiling->height;

        perf->swTilingInfo.origInX = output_tiling->width;
        perf->swTilingInfo.origInY = output_tiling->height;
    }
    else
    {
        perf->swTilingInfo.origInX = perf->info.oinx;
        perf->swTilingInfo.origInY = perf->info.oiny;
    }

    if (op_type == VXNNE_OPERATOR_FULLYCONNECTED && op_target == VXNNE_OPERATION_TARGET_TP)
    {
        /*convert TP FC input/output info for arch model analysis when dims<=2 */
        vx_uint32 inDims = input->dimCount;
        vx_uint32 outDims = output->dimCount;
        if ((inDims == 2) || (inDims == 1))
        {
            perf->info.inx = 1;
            perf->info.iny = 1;
            perf->info.pix = 1;
            perf->info.piy = 1;
            perf->info.oinx = 1;
            perf->info.oinx = 1;
            perf->swTilingInfo.origInX = 1;
            perf->swTilingInfo.origInY = 1;
            perf->info.inz = TENSOR_VIEW_SIZE_INDEX(input, 0) * TENSOR_VIEW_SIZE_INDEX(input, 1) * TENSOR_VIEW_SIZE_INDEX(input, 2);
            perf->info.oinz = perf->info.inz;
        }

        if (((outDims == 2) || (outDims == 1)) && (wb != ARCH_NULL))
        {
            perf->info.inx = 1;
            perf->info.iny = 1;
            perf->info.pix = 1;
            perf->info.piy = 1;
            perf->swTilingInfo.origOutX = 1;
            perf->swTilingInfo.origOutY = 1;
            perf->swTilingInfo.origOutZ = WB_OUTPUT_Z(wb);
            perf->info.outz = WB_OUTPUT_Z(wb);
            perf->swTilingInfo.origInX = perf->info.oinx;
            perf->swTilingInfo.origInY = perf->info.oiny;
        }
    }

    perf->swTilingInfo.srcBuf    = input_tiling->sRAM ? (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM) : SW_TILING_FROM_DDR;
    perf->swTilingInfo.dstBuf    = output_tiling->sRAM ? (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM) : SW_TILING_FROM_DDR;
    perf->swTilingInfo.kernelBuf = (op_command->cmdInfo.kernelCacheMode == VXNNE_SRAM_CACHE_MODE_STREAM_CACHE) ? SW_TILING_FROM_VIP_SRAM : SW_TILING_FROM_DDR;

    perf->info.flush = op_command->cmdInfo.flush;

    /* update compress ratio */
    if (perf->info.inputDataFormat == VX_TYPE_FLOAT16)
    {
        numCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
    }
    else if (perf->info.inputDataFormat == VX_TYPE_INT16)
    {
        numCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
    }
    else
    {
        numCores  = pArchNnConfig->fixedFeature.nnCoreCount;
    }

    if(wb != NULL)
    {
        perf->coefNonZeroRatio  = WB_NON_ZERO_RATIO(wb);
        perf->coefCompressRatio = WB_COMPRESS_RATIO(wb);
        perf->imageCompressRatio = archIsFeatureAvailable(pArchNnConfig,pArchOptions,&archDataFeature, ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
        perf->imageNonZeroRatio  = 0.300000000000000;
    }
    else
    {
        perf->coefNonZeroRatio  = 0;
        perf->coefCompressRatio = 0;
        perf->imageCompressRatio = 0;
        perf->imageNonZeroRatio  = 0;
    }

    /* NN Transpose Info */
    perf->swTilingInfo.trspIvLayerChsIn = operation->transposeInChannel;
    perf->swTilingInfo.trspIvLayerChsOut = operation->transposeOutChannel;

    archCalculateArchPerf(context->globalData->apm, pArchNnConfig,pArchOptions, &archDataFeature,(arch_perf)perf, (archnne_operation_target_e)op_target, (archnne_operator_e)op_type);

}

/***********************************************************************************
* Function:     calculateArchPerfFromWB
* Description:  Calculate the Arch Model performance when driver doing AB Segment
* Input:        Since there is no operation command passed from the called, the nntranspose
                Info should be set by caller
* Ouput:        void
***************************************************************************************/
ARCH_INTERNAL_API void archCalculateArchPerfFromWB(
    vx_context context,
    vxnne_operation operation,
    arch_perf perf,
    vx_weights_biases_parameter wb,
    vx_uint32 orig_input_dims[],
    vx_uint32 output_dims[],
    vx_enum output_format,
    arch_uint32 pad_x_left,
    arch_uint32 pad_x_right,
    arch_uint32 pad_y_top,
    arch_uint32 pad_y_bottom,
    arch_uint32 pool_size,
    arch_uint32 pool_stride,
    vx_int32* offsets,
    vx_int32 flush,
    arch_uint8 src_buf,
    arch_uint8 dst_buf,
    arch_uint8 kernel_buf,
    vx_int32 cached_space,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type)
{

    vx_uint32 outXSize, outYSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride;
    vx_int32 inputDataSize, outputDataSize, xOffSet, yOffSet;
    /* vx_uint32 coreIndex = 0,*/
    /* vx_uint32 numCores = 0;*/

    /* addjust for C-Arch Sw Lib */
    archNN_DATABASE_FEATURE archDataFeature = {0};
    arch_nn_config archNNConfig;
    arch_drv_option archDrvOption;
    arch_nn_config *pArchNnConfig =  &archNNConfig;
    arch_drv_option *pArchOptions = &archDrvOption;

    /* Init configration */
    memset(pArchNnConfig,0,sizeof(arch_nn_config));
    memset(pArchOptions,0,sizeof(arch_drv_option));
    initConfigration(pArchNnConfig,pArchOptions,context);
    /* Init data feature */
    updateConfigration(&archDataFeature,pArchNnConfig, context);

    poolingSize = gcmMAX(1, pool_size);
    poolingStride = pool_size ? pool_stride : 1;

    ComputeInputSize(
        output_dims[0],
        WB_KERNEL_X(wb),
        pad_x_left,
        pad_x_right,
        poolingSize,
        poolingStride,
        &outXSize,
        1);

    ComputeInputSize(
        output_dims[1],
        WB_KERNEL_Y(wb),
        pad_y_top,
        pad_y_bottom,
        poolingSize,
        poolingStride,
        &outYSize,
        1);

    kernelXSize = WB_KERNEL_X(wb);
    kernelYSize = WB_KERNEL_Y(wb);
    kernelZSize = (op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV && WB_IS_DEPTH_WISE(wb)) ? WB_OUTPUT_Z(wb) : WB_KERNEL_Z(wb);
    xOffSet = offsets == VX_NULL ? WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * pad_x_left) : offsets[0];
    yOffSet = offsets == VX_NULL ? WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) * pad_y_top) : offsets[1];
    inputDataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb));
    outputDataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)output_format);

    /********** left perf structure configuration **********/
    perf->info.kx = kernelXSize;
    perf->info.ky = kernelYSize;
    perf->info.kz = kernelZSize;
    perf->info.oinx = orig_input_dims[0];
    perf->info.oiny = orig_input_dims[1];
    perf->info.oinz = orig_input_dims[2];
    if (op_type == VXNNE_OPERATOR_POOLING)
    {
        perf->info.inx = orig_input_dims[0];
        perf->info.iny = orig_input_dims[1];
        perf->info.inz = orig_input_dims[2];
    }
    else
    {
        perf->info.inx = outXSize;
        perf->info.iny = outYSize;
        perf->info.inz = kernelZSize;
    }
    perf->info.outx = output_dims[0];
    perf->info.outy = output_dims[1];
    perf->info.outz = output_dims[2];
    perf->info.stridex = WB_STRIDE_X(wb);
    perf->info.stridey = WB_STRIDE_Y(wb);
    perf->info.inputDataSize = inputDataSize;
    perf->info.outputDataSize = outputDataSize;
    perf->info.poolingSize = poolingSize;
    perf->info.poolingStride = poolingStride;
    perf->info.xOffSet = xOffSet;
    perf->info.yOffSet = yOffSet;

    if (op_type == VXNNE_OPERATOR_CONVOLUTION || op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        perf->swTilingInfo.origInX = perf->info.inx;
        perf->swTilingInfo.origInY = perf->info.iny;
    }
    else
    {
        perf->swTilingInfo.origInX = perf->info.oinx;
        perf->swTilingInfo.origInY = perf->info.oiny;
    }
    perf->swTilingInfo.cacheSpace = cached_space;
    perf->swTilingInfo.srcBuf = src_buf;
    perf->swTilingInfo.dstBuf = dst_buf;
    perf->swTilingInfo.kernelBuf = kernel_buf;

    perf->info.p3 = (poolingSize == 3) ? 1: 0;
    perf->info.pix = (vx_uint32)ceilf((arch_float32)(outXSize - perf->info.p3) / poolingStride);
    perf->info.piy = (vx_uint32)ceilf((arch_float32)(outYSize - perf->info.p3) / poolingStride);
    if ((op_target != VXNNE_OPERATION_TARGET_TP || op_type == VXNNE_OPERATOR_FULLYCONNECTED) && wb)
      perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(wb, 0), CACHE_ALIGNMENT_SIZE));
    /* use wb's format as input data format for arch model performance calculation */
    perf->info.inputDataFormat = WB_WEIGHT_DATA_FORMAT(wb);
    perf->info.flush = 1;

    /* update compress ratio */
    /*
    if (perf->info.inputDataFormat == VX_TYPE_FLOAT16)
        numCores       = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
    else if (perf->info.inputDataFormat == VX_TYPE_INT16)
        numCores       = pArchNnConfig->fixedFeature.nnCoreCountInt16;
    else
        numCores       = pArchNnConfig->fixedFeature.nnCoreCount;
    */

    if(wb != NULL)
    {
        perf->coefNonZeroRatio  = WB_NON_ZERO_RATIO(wb);
        perf->coefCompressRatio = WB_COMPRESS_RATIO(wb);
        perf->imageCompressRatio = archIsFeatureAvailable(pArchNnConfig, pArchOptions, &archDataFeature,ARCH_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
        perf->imageNonZeroRatio  = 0.300000000000000;
    }
    else
    {
        perf->coefNonZeroRatio  = 0;
        perf->coefCompressRatio = 0;
        perf->imageCompressRatio = 0;
        perf->imageNonZeroRatio  = 0;
    }
    /* NN Transpose Info, set by caller to archCalculateArchPerfFromWB */
    perf->swTilingInfo.trspIvLayerChsIn = operation->transposeInChannel;
    perf->swTilingInfo.trspIvLayerChsOut = operation->transposeOutChannel;

    archCalculateArchPerf(context->globalData->apm,pArchNnConfig,pArchOptions,&archDataFeature, (arch_perf)perf, (archnne_operation_target_e)op_target, (archnne_operator_e)op_type);
}


/******************************************************  function for Driver to showDriverPerformance, TBD optimize ************************/
/*******************************************************************************************************************************************/
#define AXI_BURST_SIZE 64
static vx_uint32 _kernel_size_in_pixel_by_arch_perf(
    vxnne_operation_target_e opTarget,
    vxnne_operator_e opType,
    arch_perf perf,
    vx_bool fullCacheIntervalFix)
{
    vx_float64 coefCompressionRatio = perf->coefCompressRatio;
    arch_float64 marginRatio = (1.25 - 1.05) * (1 - archMIN(1, coefCompressionRatio)) / (1 - 0.02) + 1.05;
    if (opTarget != VXNNE_OPERATION_TARGET_TP || opType == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        if (coefCompressionRatio)
        {
             if (fullCacheIntervalFix)        /* ID 2033 */
            {
                if(perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)
                {
                    return (arch_uint32)(perf->info.kx
                          * perf->info.ky
                          * perf->info.outz
                          * coefCompressionRatio * marginRatio);
                }
                else
                {
                    return (arch_uint32)(perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * perf->info.outz
                          * coefCompressionRatio * marginRatio);
                }
            }
            else
            {
               if(perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)
               {
                    return (arch_uint32)((arch_float32)perf->info.kx
                          * perf->info.ky
                          * ceilf((arch_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                          * coefCompressionRatio * marginRatio);
               }
               else
               {
                    return (arch_uint32)((arch_float32)perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * ceilf((arch_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                          * coefCompressionRatio * marginRatio);
               }
            }
        }
    }

    return 0;
}

/***********************************************************************************
* Function:     showDriverPerformance
* Description:  Show performance only for driver
* Input:        TBD
* Ouput:        TBD
***************************************************************************************/
static vx_int8 gOrigShowType = -1;
static const vx_char *archModelVersion = "ARCHCTS@235166";
static const vx_char *SWTilingVersion = "ARCHCTS@235166";
vx_status showDriverPerformance(
    vx_context context,
    vxnne_layer layer,
    vxnne_operation op,
    arch_perf perf
    )
{
    vx_uint32 i;
    vx_uint32 profileMode = 0;
    vx_float32 mutiGpuFactor = (context->nnConfig.fixedFeature.vipCoreCount > 1) ? (0.7f * context->nnConfig.fixedFeature.vipCoreCount) : 1.0f;

    /* for get all config */
    arch_nn_config          archNNConfig;             /* configration for specific case */
    arch_drv_option         archDrvOption;            /* options defined in env */
    archNN_DATABASE_FEATURE archDataFeature = {0};    /* case feature definition */
    gcsHAL_CHIPIDENTITY     chipIdentity = {0};

    if (gOrigShowType != (vx_int8)context->options.collectPerfType)
    {
        /* update configuration */
        memset(&archNNConfig,0,sizeof(arch_nn_config));
        memset(&archDrvOption,0,sizeof(arch_drv_option));
        initConfigration(&archNNConfig,&archDrvOption,context);
        updateConfigration(&archDataFeature,&archNNConfig, context);
        /* get the chip information */
        gcoHAL_QueryChipIdentityEx(ARCH_NULL, sizeof(archHAL_CHIPIDENTITY),
                               (gcsHAL_CHIPIDENTITY *)&chipIdentity);
        vxInfo("\nArchModelVersion: %s\nSWTilingVersion: %s\nProfileMode: %d\n"
               "chipModel: 0x%x\nchipRevision: 0x%x\nproductID: 0x%x\ncustomerID: 0x%x\necoID: 0x%x\n"
               "NumNNCores:%d\nNumNNCoresInt8: %d\nNumNNCoresInt16: %d\nNumNNCoresFloat16: %d\n"
               "NumTPCores: %d\nNumTPLiteCores: %d\nMadPerCore: %d\nVIP7Version: %d\n"
               "InBuffDepth: %d\nAccumBufferDepth: %d\nDPAmount: %d\n"
               "XYDPX: %d\nXYDPY: %d\nZDP: %d\n"
               "ZDP3Enable: %d\nZDP6Enable: %d\n"
               "AXISRAMSize: %d\nVIPSRAMSize: %d\nL2CacheWidth: %d\n"
               "USCCacheSize: %d\nBrickMode: %d\nSWTiling: %d\n"
               "SmallBatchEnable: %d\nSWTilingPhase1: %d\nTPWithFCLayer: %d\n"
               "TPCircularBufferSupport: %d\nKERNEL_HEADER_NOT_CACHED_FIX: %d\n"
               "NNFCNonPruneAccel: %d\nConv1x1HalfPerformance: %d\n"
               "DDRLatency: %d\nCacheLineModeDisabled: %d\n"
               "PER_3D_TILE_BUBBLE_FIX: %d\nSWConv1x1To1x2: %d\n"
               "TP_LOCALIZATION_REORDER_DISABLED_Fix: %d\nUSCCacheControllers: %d\n"
               "AsyncCopyPerfFix: %d\nZDP3NoCompressFix: %d\n"
               "ZXDP3KernelReadConflictFix: %d\nxyOffsetLimitationFix: %d\n",
               archModelVersion,
               SWTilingVersion,
               (context->options.collectPerfType == COLLECT_PERF_ESTIMATE) ? 1 : profileMode,
               chipIdentity.chipModel,
               chipIdentity.chipRevision,
               chipIdentity.productID,
               chipIdentity.customerID,
               chipIdentity.ecoID,
               context->nnConfig.fixedFeature.nnCoreCount,
               context->nnConfig.fixedFeature.nnCoreCountInt8,
               context->nnConfig.fixedFeature.nnCoreCountInt16,
               context->nnConfig.fixedFeature.nnCoreCountFloat16,
               context->nnConfig.fixedFeature.tpCoreCount,
               context->nnConfig.fixedFeature.tpliteCoreCount,
               context->nnConfig.fixedFeature.nnMadPerCore,
               context->nnConfig.fixedFeature.vip7Version,
               context->nnConfig.fixedFeature.nnInputBufferDepth,
               context->nnConfig.fixedFeature.nnAccumBufferDepth,
               context->nnConfig.derivedFeature.nnDPAmount,
               context->nnConfig.derivedFeature.nnXYDPX,
               context->nnConfig.derivedFeature.nnXYDPY,
               context->nnConfig.derivedFeature.nnZDP,
               gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3),
               gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP6),
               context->nnConfig.customizedFeature.axiSRAMSize,
               context->nnConfig.customizedFeature.vipSRAMSize,
               context->nnConfig.fixedFeature.equivalentVipsramWidthInByte,
               context->nnConfig.unifiedFeature.nnUSCCacheSize,
               context->nnConfig.fixedFeature.vipBrickMode,
               context->nnConfig.customizedFeature.vipSWTiling,
               context->nnConfig.unifiedFeature.smallBatchEnable, /*smallBatchEnable*/
               vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) ? 1 : 0,
               1, /*TPWithFCLayer*/
               vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE2) ? 1 : 0,
               context->nnConfig.unifiedFeature.fullCacheKernelHeadFix,/*KERNEL_HEADER_NOT_CACHED_BUG1968*/
               0,  /*NNFCNonPruneAccel*/
               context->nnConfig.unifiedFeature.conv1x1HalfPerformance,
               (vx_uint32)context->nnConfig.customizedFeature.ddrLatency,
               context->nnConfig.unifiedFeature.cacheLineModeDisabled,
               context->nnConfig.unifiedFeature.per3DTileBubbleFix,
               0, /*swConv1x1To1x2*/
               context->nnConfig.unifiedFeature.tpReOrderFix,
               context->nnConfig.fixedFeature.uscCacheControllers,
               context->nnConfig.unifiedFeature.asyncCopyPerfFix,
               context->nnConfig.unifiedFeature.zdp3NoCompressFix,
               context->nnConfig.unifiedFeature.zxdp3KernelReadConflictFix,
               context->nnConfig.unifiedFeature.xyOffsetLimitationFix
               );

        vxInfo("CoefDecodePerf: %d\nVectorPrune: %d\nEnableCacheDataFromSRAM: %d\n"
               "IMAGE_PARTIAL_CACHE_FIX: %d\nDDRReadBandWidthLimit: %.2f\n"
               "DDRWriteBandWidthLimit: %.2f\nDDRTotalBandWidthLimit: %.2f\n"
               "AXISRAMReadBandWidthLimit: %.2f\nAXISRAMWriteBandWidthLimit: %.2f\n"
               "AXISRAMTotalBandWidthLimit: %.2f\nAXIBusReadBandWidthLimit: %.2f\n"
               "AXIBusWriteBandWidthLimit: %.2f\nAXIBusTotalBandWidthLimit: %.2f\n\n",
               context->nnConfig.unifiedFeature.vipCoefDecodePerf,
               context->nnConfig.customizedFeature.vipVectorPrune,
               context->nnConfig.unifiedFeature.vipCachedReadFromSram,
               context->nnConfig.unifiedFeature.vipImagePartialCache,
               context->nnConfig.customizedFeature.ddrReadBWLimit,
               context->nnConfig.customizedFeature.ddrWriteBWLimit,
               context->nnConfig.customizedFeature.ddrTotalBWLimit,
               context->nnConfig.customizedFeature.axiSramReadBWLimit,
               context->nnConfig.customizedFeature.axiSramWriteBWLimit,
               context->nnConfig.customizedFeature.axiSramTotalBWLimit,
               context->nnConfig.customizedFeature.axiBusReadBWLimit,
               context->nnConfig.customizedFeature.axiBusWriteBWLimit,
               context->nnConfig.customizedFeature.axiBusTotalBWLimit);

        vxInfo("HANDLE_ABBUFFER: %d\nHANDLE_SUBIMAGE: %d\nHANDLE_BRANCH: %d\n\n",
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB) ? 1 : 0,
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_TILING) ? 1 : 0,
               (context->options.collectPerfType == 1) ? 1 : context->options.enableHandleBranch);

        vxInfo("FreqInMHZ: %u\nAxiClockFreqInMHZ: %u\nOutstandingTransfer: %d\nInternalWriteBWLimit: %.2f\n\n",
               context->nnConfig.customizedFeature.freqInMHZ,
               context->nnConfig.customizedFeature.axiClockFreqInMHZ,
               context->nnConfig.fixedFeature.maxOTNumber,
               (vx_float32)context->nnConfig.fixedFeature.nnLanesPerOutCycle);

        vxInfo("LanesPerConv: %u\nMaxTileSize: %u\nAxiSramSlowedDownByAddr: %d\nSLOW_NN_REQ_ARBITRATION_FIX: %d\n\n",
               context->nnConfig.unifiedFeature.lanesPerConv,
               context->nnConfig.unifiedFeature.maxTileSize,
               context->nnConfig.unifiedFeature.axiSramSlowedDownByAddr,
               context->nnConfig.unifiedFeature.slowNNReqArbitrationFix);

        vxInfo("FLOAT_XYDP_X: %u\nFLOAT_XYDP_Y: %u\nFLOAT_ZDP: %d\n",
               context->nnConfig.fixedFeature.nnFP16XYDPX,
               context->nnConfig.fixedFeature.nnFP16XYDPY,
               context->nnConfig.fixedFeature.nnFP16ZDP);

        vxInfo("SINGLE_PORT_ACC_BUFFER: %d\nMAX_ZRL_BIT_WIDTH: %d\nMAX_SOC_OUT_STANDING_NUMBER: %d\n\n",
            context->nnConfig.unifiedFeature.singlePortAccBuffer,
            context->nnConfig.fixedFeature.zrlBits,
            context->nnConfig.customizedFeature.maxSocOTNumber
            );

        vxInfo("SWTilingPhase3: %d\nAXI_SRAM_ONLY_SW_TILING: %d\n",
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE3) ? 1 : 0,
            context->nnConfig.unifiedFeature.axiSramOnlySWTiling
            );

        vxInfo("VIP_CORE_COUNT: %d\n",
            context->nnConfig.fixedFeature.vipCoreCount
            );

        vxInfo("DEPTH_WISE_SUPPORT: %d\nNN_WRITE_WITHOUT_USC: %d\n",
            context->nnConfig.customizedFeature.depthWiseSupport,
            context->nnConfig.customizedFeature.nnWriteWithoutUSC
            );

        /* Add log output */
        vxInfo("DDR_ALIGN: %d\nIN_LINES_PER_CYCLE: %d\n",
            archDataFeature.ddrAlign,
            archDataFeature.inlinesPerCycle
            );

        vxInfo("NN_SLOW_OUTPUT: %d\nNO_NARROW_POST_PROCESS_PIPE: %d\nNN_SMALLBATCH_PHASE1: %d\n",
            archDataFeature.nnSlowOutput,
            archDataFeature.noNarrowPostProcessPipe,
            archDataFeature.prefetchNNCommandKernelHeader
            );

        vxInfo("EQUIVALENT_VIP_SRAM_WIDTH_IN_BYTE: %d\n",
            context->nnConfig.fixedFeature.equivalentVipsramWidthInByte
            );

        vxInfo("IMAGE_NOT_PACKED_IN_SRAM: %d\n",
            context->nnConfig.unifiedFeature.imageNotPackedInSram
            );

        vxInfo("NN_COEF_COMPRESSION_ENHANCEMENT: %d\nTP_COMPRESSION_ENHANCEMENT: %d\n",
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT),
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT)
            );

        vxInfo("COEF_DELTA_CORD_OVER_FLOW_ZRL_8BIT_FIX: %d\n",
            context->nnConfig.unifiedFeature.coefDeltaCordOverFlowZRL8BitFix
            );

        vxInfo("NumShaderCores: %d\n",
            context->nnConfig.fixedFeature.shaderCoreCount
            );

        vxInfo("KERNEL_PER_CORE_LESS_THAN_THIRD_COEF_BUFF_DEPTH_FIX: %d\n",
            context->nnConfig.unifiedFeature.kernelPerCoreLTOneThirdCoefFix
            );

        vxInfo("LOW_EFFICIENCY_OF_ID_WRITE_IMGBUF_FIX: %d\n",
            context->nnConfig.unifiedFeature.lowEfficiencyOfIDWriteImgBufFix
            );

        /* Add log output for fix */
        vxInfo("NN_KERNEL_SIZE_WASTE_IN_PARTIAL_MODE_FIX: %d\n",
            archDataFeature.partialKernelCacheInternalFix
            );

        vxInfo("KERNEL_VIP_SRAM_READ_BW_LIMITATION_FIX: %d\n",
            archDataFeature.internalKernelReadBottleneckFix
            );

        vxInfo("IMG_POP_PIPELINE_PAUSE_FIX: %d\n",
            archDataFeature.ImgPopPipelinePauseFix
            );

        vxInfo("FULL_CACHE_INTERVAL_FIX: %d\n",
            archDataFeature.fullCacheIntervalFix
            );

        vxInfo("NN_Transpose: %d\nSPECIFIED_DDR_BW_LIMIT_BY_BURST: %d\n",
            archDataFeature.nnTranspose, archDataFeature.specificDDRLimitByBurst
            );

        vxInfo("DR_JD_Diff_For_Cacheline_Mode_Fix: %d\n",
            archDataFeature.drJdDiffConditionForCacheLineModePreFix
            );

        /* DDR Read Sustained BW Burst */
        vxInfo("DDR_READ_SUSTAINED_BW_64B_BURST: %f\nDDR_READ_SUSTAINED_BW_128B_BURST: %f\nDDR_READ_SUSTAINED_BW_256B_BURST: %f\n",
            archDataFeature.ddrReadSustainedBw64BBurst,
            archDataFeature.ddrReadSustainedBw128BBurst,
            archDataFeature.ddrReadSustainedBw256BBurst
            );
        vxInfo("DDR_WRITE_SUSTAINED_BW_64B_MASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_128B_MASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_256B_MASK_BURST: %f\n",
            archDataFeature.ddrMaskWriteSustainedBw64BBurst,
            archDataFeature.ddrMaskWriteSustainedBw128BBurst,
            archDataFeature.ddrMaskWriteSustainedBw256BBurst
            );
        vxInfo("DDR_WRITE_SUSTAINED_BW_64B_NONMASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_128B_NONMASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_256B_NONMASK_BURST: %f\n",
            archDataFeature.ddrNonMaskWriteSustainedBw64BBurst,
            archDataFeature.ddrNonMaskWriteSustainedBw128BBurst,
            archDataFeature.ddrNonMaskWriteSustainedBw256BBurst
            );

        /* new configration */
        vxInfo("VIPSRAM_ASYNC_FIFO: %d\nreadReturnArbiterBubbleFix: %d\nnerghborImageDataTransferNotEfficientFix: %d\ntpVipSramOt1Fix: %d\n",
            archDataFeature.vipSramAsyncFifo,
            archDataFeature.readReturnArbiterBubbleFix,          /* 2038 */
            archDataFeature.nerghborImageDataTransferNotEfficientFix,          /* 2045 */
            archDataFeature.tpVipSramOt1Fix           /* 2050 */
            );

        /* Burst size */
        vxInfo("NN_DDR_BURST_SIZE: %d\nNN_LARGE_BURST_SIZE: %d\nTP_COMP_2PIXEL_PER_CYCLE: %d\n",
            archDataFeature.nnDDRBurstSize,
            archDataFeature.nnLargeBurstSize,
            archDataFeature.tpComp2pixelPerCycle
            );

        vxInfo("\n");
        gOrigShowType = (vx_int8)context->options.collectPerfType;
    }

    vxInfo("\n");
    vxInfo("===========================\n");
    vxInfo("**********Show Perf********\n");
    vxInfo("===========================\n");

    vxInfo("layer_id:%d layer_name:%s\noperation_id:%d operation_name:%s operation_target:%s\n",
             layer->node->id, layer->name,
             op->id, vxnneGetOperatorTypeName(op->operatorType), vxnneGetOperatorTargetName(op->target));

    vxInfo("abs_op_id:%d\nuid:%d\n", op->absoluteOperationID, getUserIDFromOutputTensor(op->outputs[0]));

    vxInfo("upstream_layer_num:%d upstream_opertaion_num:%d\n",
             op->parentLayerNum, op->parentOpNum);

    for (i = 0; i < op->parentOpNum; i++)
    {
        vxInfo("%d) upstream_operation_id:%d uptream_operation_name:%s (upstream_layer_id:%d upstream_layer_name:%s)\n",
                 i, op->parentOps[i]->id,
                 vxnneGetOperatorTypeName(op->parentOps[i]->operatorType),
                 op->parentOps[i]->layer->node->id, op->parentOps[i]->layer->name);
    }

    vxInfo("downstream_layer_num:%d downstream_opertaion_num:%d\n",
             op->childLayerNum, op->childOpNum);

    for (i = 0; i < op->childOpNum; i++)
    {
        vxInfo("%d) downstream_operation_id:%d downstream_operation_name:%s (downstream_layer_id:%d downstream_layer_name:%s)\n",
                 i, op->childOps[i]->id,
                 vxnneGetOperatorTypeName(op->childOps[i]->operatorType),
                 op->childOps[i]->layer->node->id, op->childOps[i]->layer->name);
    }

    if (op->target == VXNNE_OPERATION_TARGET_SH ||
        op->target == VXNNE_OPERATION_TARGET_SW ||
        op->target == VXNNE_OPERATION_TARGET_SC)
    {
        return VX_SUCCESS;
    }

    if (!perf->swTilingInfo.origOutX || !perf->swTilingInfo.origOutY || !perf->swTilingInfo.origOutZ)
    {
        perf->swTilingInfo.origOutX = perf->info.poolingSize > 1 ?
                                     (perf->swTilingInfo.origInX - perf->info.poolingSize + perf->info.poolingStride - 1) / perf->info.poolingStride + 1 :
                                     perf->swTilingInfo.origInX;

        perf->swTilingInfo.origOutY = perf->info.poolingSize > 1 ?
                                     (perf->swTilingInfo.origInY - perf->info.poolingSize + perf->info.poolingStride - 1) / perf->info.poolingStride + 1 :
                                     perf->swTilingInfo.origInY;

        perf->swTilingInfo.origOutZ = perf->info.outz;
    }

    if (!perf->info.outx || !perf->info.outy)
    {
        perf->info.outx = perf->info.pix;
        perf->info.outy = perf->info.piy;
    }

    if (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        vxInfo("NumUsedNNCores: %d\nConvOutFIFODepth: %d\n\n", perf->info.nnCores, perf->info.convOutFifoDepth);
    }


    if (perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD
        || perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        vxInfo("OrigInImageX: %d\nOrigInImageY: %d\nOrigInImageZ: %d\nOutImageX: %d (sub: %d)\nOutImageY: %d (sub: %d)\nOutImageZ: %d (sub: %d)\nFinalOutImageX: %d\nFinalOutImageY: %d\nFinalOutImageZ: %d\n",
                    perf->info.oinx, perf->info.oiny, perf->info.oinz,
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->swTilingInfo.origOutZ, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else if (perf->opType == ARCHNNE_OPERATOR_POOLING)
    {
        vxInfo("OrigInImageX: %d (sub: %d)\nOrigInImageY: %d (sub: %d)\nOrigInImageZ: %d (sub: %d)\nOutImageX: %d\nOutImageY: %d\nOutImageZ: %d\n",
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->info.oinz, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else
    {
        vxInfo("OrigInImageX: %d\nOrigInImageY: %d\nOrigInImageZ: %d\nOutImageX: %d (sub: %d)\nOutImageY: %d (sub: %d)\nOutImageZ: %d (sub: %d)\n",
                    perf->swTilingInfo.origInX,
                    perf->swTilingInfo.origInY,
                    perf->info.oinz,
                    perf->swTilingInfo.origOutX, perf->info.outx,
                    perf->swTilingInfo.origOutY, perf->info.outy,
                    perf->swTilingInfo.origOutZ, perf->info.outz);
    }

    vxInfo("KernelX: %d\nKernelY: %d\nKernelZ: %d\nPoolingSize: %d\nPoolingStride: %d\ninputDataSize:%d\noutputDataSize: %d\nFP16: %d\nstridex: %d\nstridey: %d\n",
            perf->info.kx, perf->info.ky, perf->info.kz,
            perf->info.poolingSize, perf->info.poolingStride,
            perf->info.inputDataSize,perf->info.outputDataSize,perf->info.inputDataFormat == VX_TYPE_FLOAT16 ? 1 : 0,
            perf->info.stridex, perf->info.stridey);

    vxInfo("archModel_kernelSize: %u\nkernelSize: %u\n",
        _kernel_size_in_pixel_by_arch_perf((vxnne_operation_target_e)perf->opTarget, (vxnne_operator_e)perf->opType, perf, context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e),
        perf->info.kernelSize
        );

    vxInfo("SrcBuf: %s\nDstBuf: %s\nKernelBuf: %s\n",
            !perf->swTilingInfo.srcBuf ? "DDR" : (perf->swTilingInfo.srcBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.dstBuf ? "DDR" : (perf->swTilingInfo.dstBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.kernelBuf ? "DDR" : (perf->swTilingInfo.kernelBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM");

    vxInfo("NN_Transpose_Channel_In: %d\nNN_Transpose_Channel_out: %d\n",
            perf->swTilingInfo.trspIvLayerChsIn, perf->swTilingInfo.trspIvLayerChsOut
            );

    if ((context->options.collectPerfType == COLLECT_PERF_ESTIMATE) || profileMode)
    {
         /*_calcArchModelCacheMode(context, perf, &imageIdealCacheSizeInPixel);*/
        vxInfo("imageIdealCacheSizeInPixel: %d\n", perf->swTilingInfo.imageIdealCacheSizeInPixel);
    }

    vxInfo("KernelCacheMode=%s\nImageCacheMode=%s\n",
             vxnneGetCacheModeName(perf->swTilingInfo.kernelCacheMode),
             vxnneGetCacheModeName(perf->swTilingInfo.imageCacheMode));

    vxInfo("xOffset: %d, yOffset: %d\n", perf->info.xOffSet, perf->info.yOffSet);

    if (perf->opType == ARCHNNE_OPERATOR_FULLYCONNECTED ||
        perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD ||
        perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        //vxInfo("maxPerCoreCompressionRatio: %.15f\n", perf->maxPerCoreCompressionRatio);
        vxInfo("coefNonZeroRatio: %.15f\ncoefCompression: %.15f\nimageCompression: %.15f\nimageNonZeroRatio: %.15f\n\n",
                 perf->coefNonZeroRatio,
                 perf->coefCompressRatio,
                 perf->imageCompressRatio,
                 perf->imageNonZeroRatio
                );
        //vxInfo("maxPerCoreCompressionRatio__llu: %llu\n", *(vx_uint64 *)&perf->maxPerCoreCompressionRatio);
        vxInfo("coefNonZeroRatio__llu: %llu\ncoefCompression_llu: %llu\nimageCompression_llu: %llu\nimageNonZeroRatio_llu: %llu\n\n",
                 *(vx_uint64 *)&perf->coefNonZeroRatio,
                 *(vx_uint64 *)&perf->coefCompressRatio,
                 *(vx_uint64 *)&perf->imageCompressRatio,
                 *(vx_uint64 *)&perf->imageNonZeroRatio
                );

        if (perf->opType == ARCHNNE_OPERATOR_CONVOLUTION ||
            perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV ||
            (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN && perf->opType == ARCHNNE_OPERATOR_FULLYCONNECTED)
            || perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD)
        {
            vxInfo("OutImageTileXSize: %d\nOutImageTileYSize: %d\nKernelsPerCore: %d\n\n",
                     perf->resultInfo.outImageTileXSize,
                     perf->resultInfo.outImageTileYSize,
                     perf->resultInfo.kernelsPerCore
                  );
        }
    }
    else
    {
        vxInfo("\n");
    }

    vxInfo("kernelDDRReadBW: %llu\nInImageDDrReadBW: %llu\n",
             (vx_uint64)(perf->resultInfo.perfKernelReadBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfInImageReadBandWidth + 0.5f));

    vxInfo("ReadBW: %llu\nWriteBW: %llu\nCycleCount: %llu\n\n",
             (vx_uint64)(perf->resultInfo.perfReadBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfWriteBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfCycleCount / mutiGpuFactor + 0.5f));

    return VX_SUCCESS;
}
