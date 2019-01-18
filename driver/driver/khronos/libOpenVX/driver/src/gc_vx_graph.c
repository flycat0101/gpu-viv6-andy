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

#include "gc_vx_graph_optimization.h"


#define PRE_SORT 1
#define ENABLE_ARCH_MODEL_DUMP 0


VX_INTERNAL_API void vxoGraph_Dump(vx_graph graph)
{
    if (graph == VX_NULL)
    {
        vxTrace(VX_TRACE_GRAPH, "<graph>null</graph>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)graph);

        vxTrace(VX_TRACE_GRAPH,
            "<graph>\n"
            "   <address>"VX_FORMAT_HEX"</address>\n"
            "</graph>",
            graph);
    }
}

VX_INTERNAL_API void vxoGraph_ClearAllVisitedFlags(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        graph->nodeTable[nodeIndex]->visited = vx_false_e;
    }
}

VX_INTERNAL_API void vxoGraph_ClearAllExecutedFlags(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        graph->nodeTable[nodeIndex]->executed = vx_false_e;
    }
}

VX_INTERNAL_CALLBACK_API void vxoGraph_Destructor(vx_reference ref)
{
    vx_graph graph = (vx_graph)ref;
    vx_uint32    i;
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    vxmASSERT(vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH));

    if(graph->parentGraph == VX_NULL)
    {
        gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex);
    }

    while (graph->nodeCount > 0)
    {
        vx_node node = (vx_node)graph->nodeTable[0];

        if (vxoReference_GetExternalCount(&node->base) > 0)
        {
            vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_EXTERNAL);
        }

        vxoNode_RemoveFromGraph(&graph->nodeTable[0]);
    }

    if (graph->commandBuffer)
    {
        vxFree(graph->commandBuffer);
        graph->commandBuffer = VX_NULL;
        graph->commandBufferSizeInByte = 0;
    }

    vxDestroyMutex(graph->scheduleLock);

    for(i = 0; i < graph->semaNum; i++)
    {
        gcoHAL_FreeMCFESemaphore(graph->mcfeSema[i]);
    }

    if (graph->layer) vxnneLayer_Free(&graph->layer->base);

    if (graph->memoryPool != VX_NULL)
    {
        vxoMemoryPool_Deinitialize(graph);
    }

    if(graph->parentGraph == VX_NULL)
    {
       gcoVX_RestoreContext(savedHardware, savedHardwareType, savedCoreIndex);
    }
}

VX_INTERNAL_API vx_uint32 vxoGraph_GetNextNodeIndex(vx_graph graph, vx_uint32 nodeIndex)
{
    vxmASSERT(graph);
    vxmASSERT(nodeIndex < graph->nodeCount);

    return (nodeIndex + 1) % graph->nodeCount;
}

VX_INTERNAL_API vx_bool vxoReference_HasWriteDependency(vx_reference ref1, vx_reference ref2)
{
    if (ref1 == VX_NULL || ref2 == VX_NULL) return vx_false_e;
    if (ref1 == ref2) return (((vx_array)ref1)->base.isStage)?vx_false_e:vx_true_e;/*if is staging reference, skip dependency checking*/

    if (ref1->type == VX_TYPE_PYRAMID && ref2->type == VX_TYPE_IMAGE)
    {
        vx_image image = (vx_image)ref2;

        while (image->parent != VX_NULL && image->parent != image) image = image->parent;

        if (image->base.scope == ref1) return vx_true_e;
    }
    else if (ref1->type == VX_TYPE_IMAGE && ref2->type == VX_TYPE_PYRAMID)
    {
        vx_image image = (vx_image)ref1;

        while (image->parent != VX_NULL && image->parent != image) image = image->parent;

        if (image->base.scope == ref2) return vx_true_e;
    }
    else if (ref1->type == VX_TYPE_IMAGE && ref2->type == VX_TYPE_IMAGE)
    {
        vx_rectangle_t rect1, rect2;
        vx_image image1, image2;

        image1 = vxoImage_LocateROI((vx_image)ref1, &rect1);
        image2 = vxoImage_LocateROI((vx_image)ref2, &rect2);

        if (image1 == image2)
        {
            if (rect1.start_x       < rect2.end_x
                && rect1.end_x      > rect2.start_x
                && rect1.start_y    < rect2.end_y
                && rect1.end_y      > rect2.start_y)
            {
                return vx_true_e;
            }
        }
    }
    else if (ref1->type == VX_TYPE_TENSOR && ref2->type == VX_TYPE_TENSOR)
    {
        return vxoTensor_IsOverlap((vx_tensor)ref1, (vx_tensor)ref2);
    }
    else if (ref1->type == VX_TYPE_OBJECT_ARRAY)
    {
        vx_object_array oa = (vx_object_array)ref1;
        vx_bool dep = vx_false_e;
        vx_uint32 i;

        for (i = 0; i < oa->itemCount; i++)
        {
            dep = vxoReference_HasWriteDependency(oa->itemsTable[i], ref2);

            if (dep == vx_true_e)
                return vx_true_e;
        }
    }
    else if (ref2->type == VX_TYPE_OBJECT_ARRAY)
    {
        vx_object_array oa = (vx_object_array)ref2;
        vx_bool dep = vx_false_e;
        vx_uint32 i;

        for (i = 0; i < oa->itemCount; i++)
        {
            dep = vxoReference_HasWriteDependency(ref1, oa->itemsTable[i]);

            if (dep == vx_true_e)
                return vx_true_e;
        }
    }
    return vx_false_e;
}

VX_INTERNAL_API vx_status vxoGraph_FindAllRelatedNodes(
    vx_graph graph, vx_direction_e direction, vx_reference paramRef,
    OUT vx_uint32 nodeIndexTable[], INOUT vx_uint32 *nodeCountPtr)
{
    vx_uint32 nodeCount = 0;
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);
    vxmASSERT(paramRef);
    vxmASSERT(nodeCountPtr);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        vxmASSERT(node->kernel);

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            if (node->kernel->signature.directionTable[paramIndex] != direction) continue;

            if (!vxoReference_HasWriteDependency(node->paramTable[paramIndex], paramRef)) continue;

            if (nodeCount >= *nodeCountPtr)
            {
                vxError("Too many nodes");
                break;
            }

            if (nodeIndexTable) nodeIndexTable[nodeCount] = nodeIndex;

            nodeCount++;
        }
    }

    *nodeCountPtr = nodeCount;

    if (nodeCount == 0) return VX_ERROR_INVALID_LINK;

    return VX_SUCCESS;
}

typedef struct _vx_graph_traverse_info_s
{
    vx_uint32                               nodeIndexTable[VX_MAX_NODE_COUNT];

    vx_uint32                               beginNodeIndex;

    vx_uint32                               depth;
}
vx_graph_traverse_info_s;

typedef vx_graph_traverse_info_s *          vx_graph_traverse_info;

VX_PRIVATE_API void vxoGraph_InitializeTraverseInfo(OUT vx_graph_traverse_info info)
{
    vxmASSERT(info);

    info->beginNodeIndex    = 0;
    info->depth             = 0;
}

VX_PRIVATE_API vx_status vxoGraph_Traverse(
    vx_graph graph, vx_uint32 parentNodeIndex, vx_uint32 childNodeIndex, INOUT vx_graph_traverse_info info)
{
    vx_uint32   nodeCount;
    vx_uint32   currentNodeIndex;
    vx_status   status;
    vx_uint32   paramIndex;
    vx_node     currentNode;

    vxmASSERT(graph);

    if (parentNodeIndex != VX_MAX_NODES_IN_GRAPH && parentNodeIndex == childNodeIndex)
    {
        vxError("Graph %p has a cycle", graph);
        return VX_ERROR_INVALID_GRAPH;
    }

    if (info->depth > graph->nodeCount)
    {
        vxmASSERT(0);
        return VX_ERROR_INVALID_GRAPH;
    }

    /*if (parentNodeIndex == VX_MAX_NODES_IN_GRAPH)
    {
    parentNodeIndex = childNodeIndex;
    }*/

    currentNodeIndex = childNodeIndex;

    currentNode = graph->nodeTable[currentNodeIndex];
    vxmASSERT(currentNode);

    if (currentNode->visited)
        return VX_SUCCESS;

    for (paramIndex = 0; paramIndex < currentNode->kernel->signature.paramCount; paramIndex++)
    {
        vx_enum direction       = currentNode->kernel->signature.directionTable[paramIndex];
        vx_reference paramRef   = currentNode->paramTable[paramIndex];

        if (direction == VX_INPUT || paramRef == VX_NULL) continue;

        nodeCount = vxmLENGTH_OF(info->nodeIndexTable) - info->beginNodeIndex;

        status = vxoGraph_FindAllRelatedNodes(graph, VX_INPUT, paramRef,
            &info->nodeIndexTable[info->beginNodeIndex], INOUT &nodeCount);

        switch (status)
        {
        case VX_ERROR_INVALID_GRAPH:
            return VX_ERROR_INVALID_GRAPH;

        case VX_ERROR_INVALID_LINK:
            break;

        case VX_SUCCESS:
            {
                vx_uint32 endNodeIndex = info->beginNodeIndex + nodeCount;
                vx_uint32 nodeIndex;

                for (nodeIndex = info->beginNodeIndex; nodeIndex < endNodeIndex; nodeIndex++)
                {
                    info->beginNodeIndex += nodeCount;
                    info->depth++;

                    status = vxoGraph_Traverse(graph, currentNodeIndex, info->nodeIndexTable[nodeIndex], INOUT info);

                    info->depth--;
                    info->beginNodeIndex -= nodeCount;

                    switch (status)
                    {
                    case VX_ERROR_INVALID_GRAPH:
                        return VX_ERROR_INVALID_GRAPH;

                    case VX_ERROR_INVALID_LINK:
                        break;

                    case VX_SUCCESS:
                        break;

                    default:
                        vxmASSERT(0);
                        break;
                    }
                }
            }
            break;

        default:
            vxmASSERT(0);
            break;
        }
    }

    currentNode->visited = vx_true_e;

    return VX_SUCCESS;
}


#if PRE_SORT
VX_INTERNAL_API void vxoGraph_GenerateAllNodeIndexTable(vx_graph graph)
{
    vx_uint32 possibleNextNodeIndexTable[VX_MAX_REF_COUNT];
    vx_uint32 leftNodeIndexTable[VX_MAX_REF_COUNT];
    vx_uint32 lastNodeIndexTable[VX_MAX_REF_COUNT];

    vx_uint32 possibleNextNodeCount = 0;
    vx_uint32 index;
    vx_uint32 paramIndex;

    vx_uint32 nextNodeCount = graph->nodeCount - graph->headNodeCount;
    vx_uint32 lastNodeCount = graph->headNodeCount;
    vx_uint32 leftNodeCount = 0;

    vx_uint32 nextIndex = 0;
    vx_uint32 lastIndex = 0;

    for (index = 0; index < graph->headNodeCount; index++)
    {
        vx_node headNode = graph->nodeTable[graph->headNodeIndexTable[index]];
        headNode->visited = vx_true_e;
        graph->allNodeIndexTable[index] = graph->headNodeIndexTable[index];
        lastNodeIndexTable[index] = graph->headNodeIndexTable[index];
    }
    nextIndex = graph->headNodeCount;

    while (nextNodeCount > 0)
    {
        /* Build up the possible next index table from the last node index table */
        for (index = 0; index < lastNodeCount; index++)
        {
            vx_node lastNode = graph->nodeTable[lastNodeIndexTable[index]];

            vxmASSERT(lastNode);

            for (paramIndex = 0; paramIndex < lastNode->kernel->signature.paramCount; paramIndex++)
            {
                vx_reference    paramRef;
                vx_uint32       newPossibleNextNodeCount;

                if (lastNode->kernel->signature.directionTable[paramIndex] == VX_INPUT) continue;

                paramRef = lastNode->paramTable[paramIndex];

                if (paramRef == VX_NULL) continue;

                newPossibleNextNodeCount = vxmLENGTH_OF(possibleNextNodeIndexTable) - possibleNextNodeCount;

                if (vxoGraph_FindAllRelatedNodes(graph, VX_INPUT, paramRef,
                    &possibleNextNodeIndexTable[possibleNextNodeCount],
                    INOUT &newPossibleNextNodeCount) == VX_SUCCESS)
                {
                    possibleNextNodeCount += newPossibleNextNodeCount;
                }
            }
        }

        /* Add all left nodes to the possible next node index table */
        for (index = 0; index < leftNodeCount; index++)
        {
            vx_uint32   index2;
            vx_bool     found = vx_false_e;

            for (index2 = 0; index2 < possibleNextNodeCount; index2++)
            {
                if (leftNodeIndexTable[index] == possibleNextNodeIndexTable[index2])
                {
                    found = vx_true_e;
                    break;
                }
            }

            if (!found)
            {
                possibleNextNodeIndexTable[possibleNextNodeCount] = leftNodeIndexTable[index];
                possibleNextNodeCount++;
                vxmASSERT(possibleNextNodeCount < VX_MAX_REF_COUNT);
            }
        }

        leftNodeCount = 0;
        lastIndex = 0;

        /* now check all possible next nodeTable to see if the parent nodeTable are visited. */
        for (index = 0; index < possibleNextNodeCount; index++)
        {
            vx_uint32   possibleNextNodeIndex = possibleNextNodeIndexTable[index];
            vx_node     possibleNextNode = graph->nodeTable[possibleNextNodeIndex];

            vx_uint32   possibleParamIndexTable[VX_MAX_PARAMETERS];
            vx_uint32   possibleParamCount = 0;
            vx_uint32   index2;
            vx_bool     areAllParamsReady = vx_true_e;

            /* Build up the possible parameter index table from the possible next node */
            for (paramIndex = 0; paramIndex < possibleNextNode->kernel->signature.paramCount; paramIndex++)
            {
                if (possibleNextNode->kernel->signature.directionTable[paramIndex] == VX_INPUT)
                {
                    possibleParamIndexTable[possibleParamCount] = paramIndex;
                    possibleParamCount++;
                }
            }

            /* Check if all possible parameters are ready */
            for (index2 = 0; index2 < possibleParamCount; index2++)
            {
                vx_uint32       possibleParamIndex  = possibleParamIndexTable[index2];
                vx_reference    possibleParamRef    = possibleNextNode->paramTable[possibleParamIndex];
                vx_direction_e  paramDirTable[]     = {VX_OUTPUT, VX_BIDIRECTIONAL};
                vx_uint32       index3;

                if (possibleNextNode->kernel->signature.stateTable[possibleParamIndex] == VX_PARAMETER_STATE_OPTIONAL && possibleParamRef == VX_NULL)
                    continue;

                for (index3 = 0; index3 < vxmLENGTH_OF(paramDirTable); index3++)
                {
                    vx_direction_e  direction = paramDirTable[index3];
                    vx_uint32       predicateNodeIndexTable[VX_MAX_REF_COUNT];
                    vx_uint32       predicateNodeCount = vxmLENGTH_OF(predicateNodeIndexTable);
                    vx_uint32       index4;

                    if (vxoGraph_FindAllRelatedNodes(graph, direction, possibleParamRef,
                        predicateNodeIndexTable, INOUT &predicateNodeCount) != VX_SUCCESS) continue;

                    /* Check if all predicate nodes executed */
                    for (index4 = 0; index4 < predicateNodeCount; index4++)
                    {
                        vx_node predicateNode = graph->nodeTable[predicateNodeIndexTable[index4]];

                        vxmASSERT(predicateNode);

                        if (!predicateNode->visited)
                        {
                            areAllParamsReady = vx_false_e;
                            break;
                        }
                    }

                    if (!areAllParamsReady) break;
                }
            }

            /* Add the possible node index to the next node index table or the left node index table */
            if (areAllParamsReady)
            {
                if (!possibleNextNode->visited)
                {
                    graph->allNodeIndexTable[nextIndex] = possibleNextNodeIndex;
                    nextIndex++;
                    vxmASSERT(nextIndex <= graph->nodeCount);
                    lastNodeIndexTable[lastIndex] = possibleNextNodeIndex;
                    lastIndex++;
                    possibleNextNode->visited = vx_true_e;
                    nextNodeCount--;
                }
            }
            else
            {
                leftNodeIndexTable[leftNodeCount] = possibleNextNodeIndex;
                leftNodeCount++;
            }
            lastNodeCount = lastIndex;
        }
    }
}

VX_INTERNAL_API void vxoGraph_GenerateOpParentChild(vx_graph graph)
{
    struct _paramNodeItemEx
    {
        vxnne_operation  ops[MAX_PARENT_CHILD_OP_NUM];
        vx_uint32        opn;
        vx_reference     output;
    };

    vx_uint32 i, j, k, l, m, count = 0;
    struct _paramNodeItemEx * paramNodeTable;
    vxnne_operation op = VX_NULL;
    gceSTATUS status;

    if (!graph->base.context->allTensorNum) return;

    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _paramNodeItemEx)*graph->base.context->allTensorNum, (gctPOINTER *)&paramNodeTable);
    if (gcmIS_ERROR(status)) return;

    for (i = 0; i < graph->layer->base.num_operations; i++)
    {
        op = graph->layer->operations[i];

        for (j = 0; j < op->outputsNum; j++)
        {
            if (op->outputs[j]->type != VX_TYPE_TENSOR)
                continue;

            for (k = 0; k < count; k++)
            {
                if (op->outputs[j] == paramNodeTable[k].output) break;
            }

            if (k < count)
            {
                paramNodeTable[k].ops[paramNodeTable[k].opn] = op;
                paramNodeTable[k].opn++;
            }
            else
            {
                if (count == graph->base.context->allTensorNum) return;
                paramNodeTable[count].ops[0] = op;
                paramNodeTable[count].opn = 1;
                paramNodeTable[count].output = op->outputs[j];
                count++;
            }
        }
    }

    for (i = 0; i < graph->layer->base.num_operations; i++)
    {
        op = graph->layer->operations[i];

        for (j = 0; j < op->inputsNum; j++)
        {
            if (op->inputs[j]->type != VX_TYPE_TENSOR)
                continue;

            for (k = 0; k < count; k++)
            {
                if (vxoTensor_IsOverlap((vx_tensor)op->inputs[j], (vx_tensor)paramNodeTable[k].output))
                {
                    for (l = 0; l < paramNodeTable[k].opn; l++)
                    {
                        vxnne_operation fop = paramNodeTable[k].ops[l];

                        /* parent */
                        for (m = 0; m < op->parentOpNum; m++)
                        {
                            if (fop->layer == op->parentOps[m]->layer) break;
                        }
                        if (m == op->parentOpNum) op->parentLayerNum++;
                        if (op->parentOpNum < MAX_PARENT_CHILD_OP_NUM)
                        {
                            op->parentOps[op->parentOpNum] = fop;
                            op->parentOpNum++;
                        }

                        /* child */
                        for (m = 0; m < fop->childOpNum; m++)
                        {
                            if (fop->childOps[m]->layer == op->layer) break;
                        }
                        if (m == fop->childOpNum) fop->childLayerNum++;
                        if (fop->childOpNum < MAX_PARENT_CHILD_OP_NUM)
                        {
                            fop->childOps[fop->childOpNum] = op;
                            fop->childOpNum++;
                        }
                    }
                }
            }
        }
    }

    gcoOS_FreeMemory(gcvNULL, paramNodeTable);
}

vx_uint32 ComputeInputSize(
    vx_uint32  outputSize,
    vx_uint32  kernelSize,
    vx_uint32  padTop,
    vx_uint32  padBottom,
    vx_uint32  poolingSize,
    vx_uint32  poolingStride,
    vx_uint32* convOut,
    vx_uint32  type)
{
    vx_uint32  inputSize = 0;

    if (type == 1)
    {
        if (poolingSize != 0)
        {
            outputSize = (outputSize - 1) * poolingStride + poolingSize;
        }

        if (convOut) *convOut = outputSize;


        inputSize = outputSize - 1 + kernelSize - padTop - padBottom;
    }

    return inputSize;
}

VX_INTERNAL_API vx_status ComputeInputSizeEx(
    vx_enum    opType,
    vx_uint32  outputSize,
    vx_uint32  kernelSize,
    vx_uint32  padTop,
    vx_uint32  padBottom,
    vx_uint32  poolingSize,
    vx_uint32  poolingStride,
    vx_uint32  reshuffStride,
    vx_uint32  normStride,
    vx_uint32* convOut,
    vx_uint32* inputSize
   )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size = 0;

    switch (opType)
    {
        case VXNNE_OPERATOR_CONVOLUTION:
        case VXNNE_OPERATOR_DEPTH_WISE_CONV:
        {
            size =  ComputeInputSize(outputSize, kernelSize, padTop, padBottom, poolingSize, poolingStride, convOut, 1);
            break;
        }
        case VXNNE_OPERATOR_RESHUFFLE:
        {
            size =  outputSize * reshuffStride - padTop - padBottom;
            break;
        }
        case VXNNE_OPERATOR_POOLING:
        {
            size =  (outputSize - 1) * poolingStride + poolingSize - padTop - padBottom;
            break;
        }
        case VXNNE_OPERATOR_NORMALIZATION:
        {
            size = normStride * outputSize;
            break;
        }
        case VXNNE_OPERATOR_ACTIVATION:
        case VXNNE_OPERATOR_TENSOR_ADD:
        case VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT:
        {
            size = outputSize;
            break;
        }
        default:
            vxmASSERT(0);
            status = VX_FAILURE;
            break;
    }

    if (inputSize) *inputSize = size;

    return status;
}



VX_INTERNAL_API vx_status ComputeMNEx(
    vxnne_execution_layer   layer,
    vx_int32                start,
    vx_uint32               count,
    vx_uint32*              outN,
    vx_uint32*              outM,
    vx_bool                 sRamIn,
    vx_bool                 sRamOut
    )
{
    vx_int32 i;
    vx_uint32  M = 0, maxPad = 0, tileYCount = 0, tileHeight = 0;
    vx_uint32 totalKernelbufferSize = 0, termA = 0, termB = 0;
    vx_uint32 circleHeight = 0, accumPoolingStrideY = 1, totalImageTileSize = 0, inputSize = 0, outputSize = 0, outputHeight = 0;
    vx_uint32 vipSRAMsize = layer->graph->base.context->vipSRAM.size;
    vx_uint32 axiSRAMsize = layer->graph->base.context->axiSRAM.size;
    vxnne_operation_info_s opInfo = {0}, opInfo2 = {0};
    vx_status status;
    vx_bool phase3Hack = vxoContext_IsFeatureAvailable(layer->graph->base.context, VX_NN_FEATURE_SWTILING_PHASE3);

    if (count < 2) return VX_FAILURE;

    vxnneOperation_GetInfo(layer->operations[start], &opInfo);
    vxnneOperation_GetInfo(layer->operations[start+ count - 1], &opInfo2);

    inputSize    = TENSOR_STRIDE_INDEX(opInfo.input, 2) * TENSOR_SIZE_INDEX(opInfo.input, 2);
    outputSize   = TENSOR_STRIDE_INDEX(opInfo2.output, 2) * TENSOR_SIZE_INDEX(opInfo2.output, 2);
    outputHeight = TENSOR_SIZE_INDEX(opInfo2.output, 1);

    if (sRamIn)
    {
        axiSRAMsize -= inputSize;
    }

    if (sRamOut)
    {
        axiSRAMsize -= outputSize;
    }

    if (((vx_int32)axiSRAMsize) < 0) return VX_FAILURE;


    for (i = start + count - 1; i >= start; i--)
    {
        vxnneOperation_GetInfo(layer->operations[i], &opInfo);

        if (i >= start + 1)
        {
            vx_uint32 kernelSizeY = opInfo.kernelY;

            vx_uint32 poolingSizeY   = opInfo.poolSizeY;
            vx_uint32 poolingStrideY = opInfo.poolStrideY;
            vx_uint32 depth  = TENSOR_SIZE_INDEX(opInfo.input, 2);
            vx_uint32 stride = TENSOR_STRIDE_INDEX(opInfo.input, 1);
            vx_uint32 reshuffStrideY = opInfo.reshuffStrideY;
            vx_uint32 normStrideY    = opInfo.normStrideY;

            status = ComputeInputSizeEx(
                                            opInfo.opType,
                                            circleHeight,
                                            kernelSizeY,
                                            0,
                                            0,
                                            poolingSizeY,
                                            poolingStrideY,
                                            reshuffStrideY,
                                            normStrideY,
                                            VX_NULL,
                                            &circleHeight
                                            );

            if (status != VX_SUCCESS) return 0;

            termA += gcmALIGN_NP2(circleHeight * depth * stride, CACHE_ALIGNMENT_SIZE);

            accumPoolingStrideY *= poolingStrideY * reshuffStrideY * normStrideY;

            termB +=  gcmALIGN_NP2(depth * stride * accumPoolingStrideY, CACHE_ALIGNMENT_SIZE);
        }

        if (opInfo.weightsBiases)
        {
            vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, kernelZ, inputDataFormat, imageTileSize;

            outImageTileX = opInfo.weightsBiases->archPerfHandle->resultInfo.outImageTileXSize;
            outImageTileY = opInfo.weightsBiases->archPerfHandle->resultInfo.outImageTileYSize;
            interleaveMode = opInfo.weightsBiases->archPerfHandle->resultInfo.interleaveMode;
            kernelX = opInfo.weightsBiases->weights_sizes[0];
            kernelY = opInfo.weightsBiases->weights_sizes[1];
            kernelZ = opInfo.weightsBiases->weights_sizes[2];
            inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

            imageTileSize = caculate3DTileSize(layer->graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, kernelZ, inputDataFormat, interleaveMode);

            totalKernelbufferSize += (vx_uint32)gcmALIGN_NP2(opInfo.weightsBiases->slice_array[0].kernel_align_stream_size, CACHE_ALIGNMENT_SIZE);

            if (!phase3Hack) totalImageTileSize += imageTileSize;
        }

        maxPad = gcmMAX(opInfo.pad.top, maxPad);
    }
    /* since the size can not predict presice, so add a margin currently, need to refine */
    totalKernelbufferSize = (vx_uint32)(totalKernelbufferSize*1.05f);

    if ((vx_int32)(axiSRAMsize - termA) > 0 && ((totalKernelbufferSize + totalImageTileSize) <= vipSRAMsize))
    {
        M = (axiSRAMsize - termA) / termB;
    }

    if (M > 0)
    {
        tileHeight = gcmMIN(M, outputHeight);
        tileYCount = gcmALIGN_NP2(outputHeight, tileHeight) / tileHeight;

        if (tileYCount == 2)
        {
            vxmASSERT(M >= outputHeight - (outputHeight/2));
            M = outputHeight - (outputHeight/2);
        }
    }

    if ((M <= 1) || M < maxPad + 1)
    {
        return VX_FAILURE;
    }
    else
    {
#if SW_TILING_DEBUG
        vxInfo("\nM = %d  predict ON CHIP SRAM need %d VIP SRAM need %d\n", M, termA + M * termB, totalKernelbufferSize + totalImageTileSize);
#endif
        *outN = TENSOR_SIZE_INDEX(opInfo2.output, 0);
        *outM = M;
        return VX_SUCCESS;
    }
}

VX_PRIVATE_API vx_status GenerateTilingInfo(
    vx_context              context,
    vxnne_execution_layer   layer,
    vx_uint32               start,
    vx_uint32               count,
    vx_uint32               N,
    vx_uint32               M,
    vxnne_segment_tiling_info tilingBlockInfo
    )
{
    vx_uint32 height = 0, width = 0, lastTileHeight;
    vx_uint32  tileHeight = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32  i, j;
    vxnne_tiling_info tilingInfo = VX_NULL, nextTilingInfo = VX_NULL;
    vxnne_operation_info_s opInfo = {0};

    tilingBlockInfo->start      = start;
    tilingBlockInfo->count      = count;

    vxnneOperation_GetInfo(layer->operations[start + count -1], &opInfo);

    width  = TENSOR_SIZE_INDEX(opInfo.output, 0);
    height = TENSOR_SIZE_INDEX(opInfo.output, 1);

    tileHeight = gcmMIN(height, M);

    tilingBlockInfo->tileXCount = gcmALIGN_NP2(width, N) / N;
    tilingBlockInfo->tileYCount = gcmALIGN_NP2(height, tileHeight) / tileHeight;

    if (gcmIS_ERROR(gcoOS_Allocate(
        gcvNULL,
        tilingBlockInfo->tileYCount * sizeof(vxnne_tiling_info_s) * count,
        (gctPOINTER*)&tilingBlockInfo->tilingInfo)))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gcoOS_ZeroMemory(tilingBlockInfo->tilingInfo, tilingBlockInfo->tileYCount * sizeof(vxnne_tiling_info_s) * count);

    for(i = count - 1; (vx_int32)i >= 0; i--)
    {
        vx_uint32 inputHeight, outputHeight;

        tilingInfo = tilingBlockInfo->tilingInfo  + i * tilingBlockInfo->tileYCount;

        vxnneOperation_GetInfo(layer->operations[start + i], &opInfo);

        outputHeight = TENSOR_SIZE_INDEX(opInfo.output, 1);
        inputHeight  = TENSOR_SIZE_INDEX(opInfo.input, 1);

        for(j = 0; j < tilingBlockInfo->tileYCount; j++)
        {
            if (i == count - 1)
            {
                if (tilingBlockInfo->tileYCount > 1 && j == tilingBlockInfo->tileYCount - 2)
                {
                    lastTileHeight = outputHeight % tileHeight;
                    if (lastTileHeight == 0) lastTileHeight = tileHeight;
                }
                else
                {
                    lastTileHeight = tileHeight;
                }
            }
            else
            {
                if (j == 0)
                {
                    lastTileHeight = nextTilingInfo[j].input.height;
                }
                else
                {
                    lastTileHeight = nextTilingInfo[j].input.end - nextTilingInfo[j-1].input.end;
                }
            }

            tilingInfo[j].output.start  = (j == 0 ? 0 : tilingInfo[j-1].output.end);
            tilingInfo[j].output.height = lastTileHeight;
            tilingInfo[j].output.width  = TENSOR_SIZE_INDEX(opInfo.output, 0);
            tilingInfo[j].output.end    = tilingInfo[j].output.start + tilingInfo[j].output.height;
            vxmASSERT(tilingInfo[j].output.height > 0);

            tilingInfo[j].padLeft         = opInfo.pad.left;
            tilingInfo[j].padTop          = (j == 0 ? opInfo.pad.top : 0);
            tilingInfo[j].input.start     = (j == 0 ? 0 : tilingInfo[j].output.start * opInfo.poolStrideY * opInfo.reshuffStrideY - opInfo.pad.top);
            vxmASSERT((vx_int32)tilingInfo[j].input.start >= 0);

            /* in this case, last Y size is 0 */
            tilingInfo[j].input.start = gcmMIN(tilingInfo[j].input.start, inputHeight);

            status = ComputeInputSizeEx(
                                        opInfo.opType,
                                        tilingInfo[j].output.height,
                                        opInfo.kernelY,
                                        tilingInfo[j].padTop,
                                        0,
                                        opInfo.poolSizeY,
                                        opInfo.poolStrideY,
                                        opInfo.reshuffStrideY,
                                        opInfo.normStrideY,
                                        &tilingInfo[j].convHeight,
                                        &tilingInfo[j].input.height
                                        );

            if (status != VX_SUCCESS) goto OnError;

            if ((tilingInfo[j].input.start + tilingInfo[j].input.height) > inputHeight)
            {
                tilingInfo[j].input.height -= (tilingInfo[j].input.start + tilingInfo[j].input.height) - inputHeight;
            }

            vxmASSERT(tilingInfo[j].input.height > 0);

            status = ComputeInputSizeEx(
                                        opInfo.opType,
                                        tilingInfo[j].output.width,
                                        0,
                                        0,
                                        0,
                                        opInfo.poolSizeX,
                                        opInfo.poolStrideX,
                                        opInfo.reshuffStrideX,
                                        opInfo.normStrideX,
                                        &tilingInfo[j].convWidth,
                                        VX_NULL
                                        );

            if (status != VX_SUCCESS) goto OnError;

            tilingInfo[j].input.width  = TENSOR_SIZE_INDEX(opInfo.input, 0);
            tilingInfo[j].input.end    = tilingInfo[j].input.start + tilingInfo[j].input.height;

            tilingBlockInfo->fixedTileHeight[i] = ((i == count - 1 ) ? 0 : gcmMAX(tilingBlockInfo->fixedTileHeight[i], nextTilingInfo[j].input.height));

            if (tilingInfo[j].output.end == outputHeight)
            {
                break;
            }

        }

        nextTilingInfo = tilingInfo;
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_bool SupportAB(vx_context context, vxnne_operation operation)
{
    if ((operation->target == VXNNE_OPERATION_TARGET_NN ||
        (operation->target == VXNNE_OPERATION_TARGET_SH && !vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE3)) ||
        (operation->target == VXNNE_OPERATION_TARGET_TP &&
          (operation->operatorType != VXNNE_OPERATOR_DILATION_RESHUFFLE &&
           operation->operatorType != VXNNE_OPERATOR_DILATION_UPSAMPLE &&
           operation->operatorType != VXNNE_OPERATOR_DILATION_UPSAMPLE2 &&
           operation->operatorType != VXNNE_OPERATOR_ROIPOOL
       ))) &&
       vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) &&
       !context->options.enableNNLayerDump
       )
    {
        /* AB buffer didn't support those TP 4d oprerations like dilation reshuffle & upsample*/
        return vx_true_e;
    }

    return vx_false_e;
}

VX_PRIVATE_API vx_bool SupportSWTiling(vx_context context, vxnne_operation operation)
{

    if ((operation->target == VXNNE_OPERATION_TARGET_NN &&
         operation->operatorType != VXNNE_OPERATOR_FULLYCONNECTED)
        ||(operation->target == VXNNE_OPERATION_TARGET_TP &&
           vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE2) &&
          (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE ||
           operation->operatorType == VXNNE_OPERATOR_POOLING   ||
           operation->operatorType == VXNNE_OPERATOR_NORMALIZATION ||
           operation->operatorType == VXNNE_OPERATOR_ACTIVATION ||
           operation->operatorType == VXNNE_OPERATOR_TENSOR_ADD
          ))
       )
    {
        return vx_true_e;
    }

    return vx_false_e;
}


VX_PRIVATE_API vx_status GetMemoryRequestList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vxnne_mem_request       memRequest
    )
{
    vx_uint32 i, j, k;

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_REQUEST_INPUT
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_REQUEST_OUTPUT);

        memRequest[k].inputCount = graph->layer->operations[i]->inputsNum;

        for(j = 0; j < memRequest[k].inputCount; j++)
        {
            memRequest[k].inputMemory[j] = &(((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory);
        }

        memRequest[k].outputCount = graph->layer->operations[i]->outputsNum;

        for(j = 0; j < memRequest[k].outputCount; j++)
        {
            memRequest[k].outputMemory[j] = &(((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory);
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status GetMemoryParamList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vxnne_mem_param         memParamList
    )
{
    vx_uint32 i, j, k;

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        memParamList[k].inputCount = graph->layer->operations[i]->inputsNum;

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR);
            memParamList[k].inputMemory[j] = ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory;
        }

        memParamList[k].outputCount = graph->layer->operations[i]->outputsNum;

        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR);
            memParamList[k].outputMemory[j] = ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status RestoreMemoryParamList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vxnne_mem_param         memParamList
    )
{
    vx_uint32 i, j, k;

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        vxmASSERT(memParamList[k].inputCount <= VX_MAX_MEM_PARAM
                   && memParamList[k].outputCount <= VX_MAX_MEM_PARAM);

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR);
            ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory = memParamList[k].inputMemory[j];
        }

        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR);
            ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory = memParamList[k].outputMemory[j];
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status RestorePartialMemoryParamList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vxnne_mem_param         memParamList
    )
{
    vx_uint32 i, j, k;
    vx_memory_s memory;

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        vxmASSERT(memParamList[k].inputCount <= VX_MAX_MEM_PARAM
                   && memParamList[k].outputCount <= VX_MAX_MEM_PARAM);

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR);
            memory = ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory;
            ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory = memParamList[k].inputMemory[j];

            ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.allocType = memory.allocType;
            ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.physicals[0] = memory.physicals[0];
            ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.logicals[0] = memory.logicals[0];
        }

        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR);
            memory = ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory;

            ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory = memParamList[k].outputMemory[j];
            ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.allocType = memory.allocType;
            ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.physicals[0] = memory.physicals[0];
            ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.logicals[0] = memory.logicals[0];
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status SetMemoryRequestList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vx_bool                 sRamIn,
    vx_bool                 sRamOut)
{
    vx_uint32 i, j, dims;
    vxnne_mem_request memRequest = graph->layer->memRequestList + start;
    vx_memory_s   outputMemory[VX_MAX_MEM_REQUEST_OUTPUT];

    for(j = 0; j < memRequest[count - 1].outputCount; j++)
    {
        outputMemory[j] = *memRequest[count - 1].outputMemory[j];
    }

    for (i = 0; i < count - 1; i++)
    {
        for(j = 0; j < memRequest[i].outputCount; j++)
        {
            dims = memRequest[i].outputMemory[j]->dimCount - 1;
            memRequest[i].outputMemory[j]->allocType    = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
            memRequest[i].outputMemory[j]->sizes[0]     = gcmALIGN_NP2(memRequest[i].outputMemory[j]->strides[0][dims] * memRequest[i].outputMemory[j]->dims[0][dims], CACHE_ALIGNMENT_SIZE);
            vxmASSERT(memRequest[i].outputMemory[j]->sizes[0] > 0);
        }
    }

    if (sRamOut)
    {
        for(j = 0; j < memRequest[i].outputCount; j++)
        {
            dims = memRequest[i].outputMemory[j]->dimCount - 1;
            memRequest[i].outputMemory[j]->allocType    = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
            memRequest[i].outputMemory[j]->sizes[0]     = gcmALIGN_NP2(memRequest[i].outputMemory[j]->strides[0][dims] * memRequest[i].outputMemory[j]->dims[0][dims], CACHE_ALIGNMENT_SIZE);
            vxmASSERT(memRequest[i].outputMemory[j]->sizes[0] > 0);
        }
    }
    else
    {
        for(j = 0; j < memRequest[i].outputCount; j++)
        {
            *memRequest[i].outputMemory[j] = outputMemory[j];
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status DetectABSegments(
    vx_graph                   graph,
    vx_uint32                  start,
    vx_uint32                  count,
    vxnne_segment_collection   collection)
{
    vx_uint32 i = 0, j = 0, end = start + count - 1, inputSize = 0, outputSize = 0;
    vxnne_operation_info_s opInfo;
    vx_status vStatus = VX_SUCCESS, vStatus1 = VX_SUCCESS;
    vxnne_mem_param memParam = VX_NULL;

    if (graph->base.context->options.enableHandleBranch)
    {
        gceSTATUS status = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_param_s) * count, (gctPOINTER*)&memParam);
        if (gcmIS_ERROR(status))
        {
            vStatus =  VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        vStatus = GetMemoryParamList(graph, start, count, memParam);
        if (vStatus != VX_SUCCESS) goto OnError;

        for(i = start; i < start + count - 1; i++)
        {
            for(j = start + count - i; j > 1; j--)
            {
                vxmASSERT((vx_int32) j > 0);
                vStatus = SetMemoryRequestList(graph, i, j, vx_false_e, vx_false_e);
                if (vStatus != VX_SUCCESS) goto OnError;

                vStatus1 = vxoMemoryPool_RequestList(graph, graph->layer->memRequestList, graph->layer->base.num_operations, i, j, VX_NULL);
                RestoreMemoryParamList(graph, start, count, memParam);
                if (vStatus1 == VX_SUCCESS)
                {
                    collection->segments[collection->segmentNum].type = VXNNE_SEGMENT_TYPE_AB;
                    collection->segments[collection->segmentNum].start = i;
                    collection->segments[collection->segmentNum].count = j;
                    collection->segments[collection->segmentNum].end = i + j - 1;
                    collection->segmentNum++;

                    i = i + j - 1;

                    if (collection->segmentNum >= VX_MAX_SEGMENT_COUNT)
                    {
                        goto OnError;
                    }
                    break;
                }

            }
        }
    }
    else
    {
        for(i = start; i <= end; i++)
        {
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);
            inputSize  = (i == start) ? 0 : TENSOR_STRIDE_INDEX(opInfo.input, 2) * TENSOR_SIZE_INDEX(opInfo.input, 2);
            outputSize = (i == end) ? 0 : TENSOR_STRIDE_INDEX(opInfo.output, 2) * TENSOR_SIZE_INDEX(opInfo.output, 2);

            if ((((inputSize + outputSize) > graph->base.context->axiSRAM.size) || i == end))
            {
                if (i > start)
                {
                    collection->segments[collection->segmentNum].type = VXNNE_SEGMENT_TYPE_AB;
                    collection->segments[collection->segmentNum].start = start;
                    collection->segments[collection->segmentNum].count = i - start + 1;
                    collection->segments[collection->segmentNum].end = i;
                    collection->segmentNum++;

                    if (collection->segmentNum >= VX_MAX_SEGMENT_COUNT)
                    {
                        goto OnError;
                    }
                }

                start = i + 1;
            }
        }
    }

#if SW_TILING_DEBUG
    if (collection->segmentNum > 0)
    {
        vxInfo("Detected AB Segments\n");
        for(i = 0; i <  collection->segmentNum; i++)
        {
            vxInfo("%3d (%d - %d)\n", i, collection->segments[i].start, collection->segments[i].end);
        }
    }
#endif

OnError:
    if (memParam) gcoOS_Free(gcvNULL, memParam);

    return vStatus;
}

VX_PRIVATE_API vx_status GetSWTilingCollection(
    vx_graph  graph,
    vx_uint32 start,
    vx_uint32 count,
    vxnne_segment_collection tilingCollection)
{
    vx_uint32 i = 0, num = 0;
    vx_bool terminator;
    vxnne_operation_info_s opInfo1, opInfo2;

    gcoOS_ZeroMemory(tilingCollection, sizeof(vxnne_segment_collection_s));

    for(i = start; i <= start + count; i++)
    {
        terminator = vx_false_e;

        if ((i == start + count) ||
            !SupportSWTiling(graph->base.context, graph->layer->operations[i]) ||
            graph->layer->operations[i]->parentOpNum > 1 ||
            graph->layer->operations[i]->childOpNum > 1)
        {
            terminator = vx_true_e;
        }
        else if (num >= 1)
        {
            vxmASSERT(graph->layer->operations[i-1]->target == VXNNE_OPERATION_TARGET_NN
                   || graph->layer->operations[i-1]->target == VXNNE_OPERATION_TARGET_TP);
            vxmASSERT(graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_NN
                   || graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_TP);

            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo1);
            vxnneOperation_GetInfo(graph->layer->operations[i - 1], &opInfo2);

            if (opInfo1.input != opInfo2.output)
            {
                terminator = vx_true_e;
            }
        }

        if (terminator)
        {
            if (num > 1)
            {
                if (tilingCollection->segmentNum >= VX_MAX_SEGMENT_COUNT)
                {
                    return VX_SUCCESS;
                }

                tilingCollection->segments[tilingCollection->segmentNum].start = i - num;
                tilingCollection->segments[tilingCollection->segmentNum].count = num;
                tilingCollection->segments[tilingCollection->segmentNum].end   = i - 1;

                tilingCollection->segmentNum++;
            }
            else if (num == 1)
            {
                i--;
            }

            num = 0;
        }
        else
        {
            num++;
        }
    }


    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status DetectTilingSegments(
    vx_graph                    graph,
    vx_uint32                   start,
    vx_uint32                   count,
    vxnne_segment_collection    abCollection,
    vxnne_segment_collection    tilingCollection)
{
    vx_uint32 i = 0, j = 0, k = 0, l = 0, tilingStart, tilingEnd, M, N;
    vx_status status = VX_SUCCESS;
    vx_bool sRamIn, sRamOut, sRamIn2, sRamOut2;
    vxnne_segment_collection_s    tempCollection;

    for(j = 0; j <= abCollection->segmentNum; j++)
    {
        tilingStart = (j == 0) ? start : abCollection->segments[j - 1].end;
        tilingEnd = (j == abCollection->segmentNum) ? (start + count - 1) : abCollection->segments[j].start;

        if (tilingEnd - tilingStart > 0)
        {
            sRamIn  = (tilingStart  == start ? vx_false_e : vx_true_e);
            sRamOut = (tilingEnd    == start + count - 1 ? vx_false_e : vx_true_e);

            GetSWTilingCollection(graph, tilingStart, tilingEnd - tilingStart + 1, &tempCollection);
            if (tempCollection.segmentNum == 0) continue;

            for(l = 0; l < tempCollection.segmentNum; l++)
            {
                for (i = tempCollection.segments[l].start; i < tempCollection.segments[l].end; i++)
                {
                    sRamIn2 = (i == tilingStart ? sRamIn : vx_false_e);

                    for (k = tempCollection.segments[l].end - i + 1; (vx_int32)k > 1; k--)
                    {
                        sRamOut2 = (tilingEnd == i + k -1 ? sRamOut : vx_false_e);

                        status = ComputeMNEx(graph->layer, i, k, &N, &M, sRamIn2, sRamOut2);
                        if (status == VX_SUCCESS)
                        {
                            if (tilingCollection->segmentNum >= VX_MAX_SEGMENT_COUNT)
                            {
                                return VX_SUCCESS;
                            }

                            tilingCollection->segments[tilingCollection->segmentNum].type = VXNNE_SEGMENT_TYPE_TILING;
                            tilingCollection->segments[tilingCollection->segmentNum].start = i;
                            tilingCollection->segments[tilingCollection->segmentNum].count = k;
                            tilingCollection->segments[tilingCollection->segmentNum].end = i + k - 1;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.M = M;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.N = N;
                            tilingCollection->segmentNum++;

                            i = i + k - 1;
                            break;
                        }
                    }
                }
            }
        }
    }

#if SW_TILING_DEBUG
    if (tilingCollection->segmentNum > 0)
    {
        vxInfo("Detected Tiling Segments\n");
        for(j = 0; j <tilingCollection->segmentNum; j++)
        {
            vxInfo("%3d (%d - %d)\n", j, tilingCollection->segments[j].start, tilingCollection->segments[j].end);
        }
    }
#endif

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status GenerateABSegmentInfo(
    vx_graph                graph,
    vxnne_block             block,
    vxnne_segment           segment,
    vx_bool                 sRamIn,
    vx_bool                 sRamOut)
{
    vx_uint32 i;
    vxnne_operation_info_s opInfo;
    vx_status status = VX_SUCCESS;

    /*empty block*/
    if (segment->count == 0) return VX_SUCCESS;

    if (graph->base.context->options.enableHandleBranch)
    {
        status = SetMemoryRequestList(graph, segment->start, segment->count, sRamIn, sRamOut);
    }
    else
    {
        if (!sRamIn)
        {
            vxnneOperation_GetInfo(graph->layer->operations[segment->start], &opInfo);

            block->bufferDesc[block->bufferCount].xStride  = TENSOR_STRIDE_INDEX(opInfo.input, 0);
            block->bufferDesc[block->bufferCount].yStride  = TENSOR_STRIDE_INDEX(opInfo.input, 1);
            block->bufferDesc[block->bufferCount].zStride  = TENSOR_STRIDE_INDEX(opInfo.input, 2);
            block->bufferDesc[block->bufferCount].sRAM     = vx_false_e;
            block->bufferDesc[block->bufferCount].bufferSize = 0;
            block->bufferDesc[block->bufferCount].circle  = vx_false_e;
            block->bufferCount++;
        }

        for (i = segment->start; i < segment->start + segment->count - 1; i++)
        {
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

            block->bufferDesc[block->bufferCount].xStride = TENSOR_STRIDE_INDEX(opInfo.output, 0);
            block->bufferDesc[block->bufferCount].yStride = TENSOR_STRIDE_INDEX(opInfo.output, 1);
            block->bufferDesc[block->bufferCount].zStride = TENSOR_STRIDE_INDEX(opInfo.output, 2);
            block->bufferDesc[block->bufferCount].sRAM    = vx_true_e;
            block->bufferDesc[block->bufferCount].bufferSize = gcmALIGN_NP2(TENSOR_STRIDE_INDEX(opInfo.output, 2) * TENSOR_SIZE_INDEX(opInfo.output, 2), CACHE_ALIGNMENT_SIZE);
            block->bufferDesc[block->bufferCount].circle  = vx_true_e;

            block->currentAllocateFlag = (block->currentAllocateFlag & VXNNE_BUFFER_ALLOCATE_FLAG_HEAD ? VXNNE_BUFFER_ALLOCATE_FLAG_TAIL: VXNNE_BUFFER_ALLOCATE_FLAG_HEAD);
            block->bufferDesc[block->bufferCount].allocateFlag = block->currentAllocateFlag;

            vxmASSERT(block->bufferDesc[block->bufferCount].bufferSize > 0);

            block->bufferCount++;
        }

        vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

        block->bufferDesc[block->bufferCount].xStride    = TENSOR_STRIDE_INDEX(opInfo.output, 0);
        block->bufferDesc[block->bufferCount].yStride    = TENSOR_STRIDE_INDEX(opInfo.output, 1);
        block->bufferDesc[block->bufferCount].zStride    = TENSOR_STRIDE_INDEX(opInfo.output, 2);
        if (!sRamOut)
        {
            block->bufferDesc[block->bufferCount].sRAM       = vx_false_e;
            block->bufferDesc[block->bufferCount].bufferSize = 0;
            block->bufferDesc[block->bufferCount].circle     = vx_false_e;
        }
        else
        {
            block->bufferDesc[block->bufferCount].sRAM       = vx_true_e;
            block->bufferDesc[block->bufferCount].bufferSize = gcmALIGN_NP2(TENSOR_STRIDE_INDEX(opInfo.output, 2) * TENSOR_SIZE_INDEX(opInfo.output, 2), CACHE_ALIGNMENT_SIZE);
            block->bufferDesc[block->bufferCount].circle     = vx_true_e;

            block->currentAllocateFlag = (block->currentAllocateFlag & VXNNE_BUFFER_ALLOCATE_FLAG_HEAD ? VXNNE_BUFFER_ALLOCATE_FLAG_TAIL: VXNNE_BUFFER_ALLOCATE_FLAG_HEAD);
            block->bufferDesc[block->bufferCount].allocateFlag = block->currentAllocateFlag;

            vxmASSERT(block->bufferDesc[block->bufferCount].bufferSize > 0);
        }

        block->bufferCount++;
    }

    return status;
}

VX_PRIVATE_API vx_status GenerateTilingSegmentInfo(
    vx_graph                graph,
    vxnne_block             block,
    vxnne_segment           segment,
    vx_bool                 sRamIn,
    vx_bool                 sRamOut
    )
{
    vx_uint32 i, j, circleBufferHeight, depth, stride, totalKernelbufferSize = 0, totalImageCacheSize = 0, imageTileSize;
    vxnne_operation_info_s opInfo;
    vxnne_tiling_info tilingInfo = VX_NULL;
    vxnne_operation operation = VX_NULL;
    vxnne_mem_request requestList = VX_NULL;
    vx_bool phase3Hack = vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_FEATURE_SWTILING_PHASE3);
    vx_sub_wb_vdata sub_wb_vdata = VX_NULL;

    vx_status status = GenerateTilingInfo(
                    graph->base.context,
                    graph->layer,
                    segment->start,
                    segment->count,
                    segment->segmentInfo.tiling.N,
                    segment->segmentInfo.tiling.M,
                    &segment->segmentInfo.tiling);

    if (status != VX_SUCCESS) goto OnError;

    if (graph->base.context->options.enableHandleBranch)
    {
        vx_uint32 inputCount = 0;

        requestList = graph->layer->memRequestList + segment->start;

        for(i = 1; i < segment->count - 1; i++)
        {
            requestList[i].inputCount = 0;
        }

        requestList[segment->count - 1].inputCount =  segment->count;

        vxmASSERT(requestList[segment->count - 1].inputCount <= VX_MAX_MEM_REQUEST_INPUT);

        requestList[segment->count - 1].inputMemory[inputCount++] =  requestList[0].inputMemory[0];

        for(i = 0; i < segment->count - 1; i++)
        {
            vxmASSERT(requestList[i].outputCount == 1);
            requestList[segment->count - 1].inputMemory[inputCount++] = requestList[i].outputMemory[0];
        }

        vxmASSERT(inputCount == segment->count);

        /* change strides to calculate size */
        for(i = 0; i < segment->count - 1; i++)
        {
            vxmASSERT(requestList[i].outputMemory[0]->dims[0][3] == 1);
            requestList[i].outputMemory[0]->dims[0][1] = segment->segmentInfo.tiling.fixedTileHeight[i];
            requestList[i].outputMemory[0]->strides[0][2] = segment->segmentInfo.tiling.fixedTileHeight[i] * requestList[i].outputMemory[0]->strides[0][1];
            for (j = 3; j < VX_CONTEXT_TENSOR_MAX_DIMENSION; j++)
            {
                requestList[i].outputMemory[0]->strides[0][j] = requestList[i].outputMemory[0]->strides[0][j-1] * requestList[i].outputMemory[0]->dims[0][j-1];
            }
            requestList[i].outputMemory[0]->circular = vx_true_e;
        }

        status = SetMemoryRequestList(graph, segment->start, segment->count, sRamIn, sRamOut);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        if (!sRamIn)
        {
            vxnneOperation_GetInfo(graph->layer->operations[segment->start], &opInfo);

            block->bufferDesc[block->bufferCount].xStride    = TENSOR_STRIDE_INDEX(opInfo.input, 0);
            block->bufferDesc[block->bufferCount].yStride    = TENSOR_STRIDE_INDEX(opInfo.input, 1);
            block->bufferDesc[block->bufferCount].zStride    = TENSOR_STRIDE_INDEX(opInfo.input, 2);
            block->bufferDesc[block->bufferCount].sRAM       = vx_false_e;
            block->bufferDesc[block->bufferCount].bufferSize = 0;
            block->bufferDesc[block->bufferCount].circle     = vx_false_e;
            block->bufferCount++;
        }
        else
        {
            vxnneOperation_GetInfo(graph->layer->operations[segment->start], &opInfo);

            block->bufferDesc[block->bufferCount-1].xStride    = TENSOR_STRIDE_INDEX(opInfo.input, 0);
            block->bufferDesc[block->bufferCount-1].yStride    = TENSOR_STRIDE_INDEX(opInfo.input, 1);
            block->bufferDesc[block->bufferCount-1].zStride    = TENSOR_STRIDE_INDEX(opInfo.input, 2);
        }

        for (i = segment->start; i < segment->start + segment->count - 1; i++)
        {
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

            circleBufferHeight = segment->segmentInfo.tiling.fixedTileHeight[i - segment->start];

            depth  = TENSOR_SIZE_INDEX(opInfo.output, 2);
            stride = TENSOR_STRIDE_INDEX(opInfo.output, 1);

            block->bufferDesc[block->bufferCount].xStride    = TENSOR_STRIDE_INDEX(opInfo.output, 0);
            block->bufferDesc[block->bufferCount].yStride    = stride;
            block->bufferDesc[block->bufferCount].zStride    = circleBufferHeight * stride;
            block->bufferDesc[block->bufferCount].bufferSize = gcmALIGN_NP2(depth * circleBufferHeight * stride, CACHE_ALIGNMENT_SIZE);
            block->bufferDesc[block->bufferCount].sRAM       = vx_true_e;
            block->bufferDesc[block->bufferCount].circle     = vx_true_e;

            block->currentAllocateFlag = (block->currentAllocateFlag == VXNNE_BUFFER_ALLOCATE_FLAG_NONE ? VXNNE_BUFFER_ALLOCATE_FLAG_HEAD: block->currentAllocateFlag | VXNNE_BUFFER_ALLOCATE_FLAG_APPEND);
            block->bufferDesc[block->bufferCount].allocateFlag = block->currentAllocateFlag;
            vxmASSERT(block->bufferDesc[block->bufferCount].bufferSize > 0);
            block->bufferCount++;
        }

        vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

        block->bufferDesc[block->bufferCount].xStride    = TENSOR_STRIDE_INDEX(opInfo.output, 0);
        block->bufferDesc[block->bufferCount].yStride    = TENSOR_STRIDE_INDEX(opInfo.output, 1);
        block->bufferDesc[block->bufferCount].zStride    = TENSOR_STRIDE_INDEX(opInfo.output, 2);
        if (!sRamOut)
        {
            block->bufferDesc[block->bufferCount].sRAM       = vx_false_e;
            block->bufferDesc[block->bufferCount].bufferSize = 0;
            block->bufferDesc[block->bufferCount].circle     = vx_false_e;
        }
        else
        {
            block->bufferDesc[block->bufferCount].sRAM       = vx_true_e;
            block->bufferDesc[block->bufferCount].bufferSize = gcmALIGN_NP2(TENSOR_STRIDE_INDEX(opInfo.output, 2) * TENSOR_SIZE_INDEX(opInfo.output, 2), CACHE_ALIGNMENT_SIZE);
            block->bufferDesc[block->bufferCount].circle     = vx_true_e;

            block->currentAllocateFlag = (block->currentAllocateFlag & VXNNE_BUFFER_ALLOCATE_FLAG_HEAD ? VXNNE_BUFFER_ALLOCATE_FLAG_TAIL: VXNNE_BUFFER_ALLOCATE_FLAG_HEAD);
            block->bufferDesc[block->bufferCount].allocateFlag = block->currentAllocateFlag;
            vxmASSERT(block->bufferDesc[block->bufferCount].bufferSize > 0);
        }

        block->bufferCount++;
    }

    for(i = 0; i < segment->count; i++)
    {
        tilingInfo = segment->segmentInfo.tiling.tilingInfo  + i * segment->segmentInfo.tiling.tileYCount;
        operation  = graph->layer->operations[i + segment->start];

        vxnneOperation_GetInfo(operation, &opInfo);

        if (operation->operatorType == VXNNE_OPERATOR_CONVOLUTION  || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
        {
            vxmASSERT(operation->target == VXNNE_OPERATION_TARGET_NN);

            if (graph->base.context->options.enableHandleBranch)
            {
                status = vxnneCalculateConvTilingParam(
                    graph->base.context,
                    (vxnne_convolution_relu_pooling_operation)operation,
                    tilingInfo,
                    requestList[i].inputMemory[0]->allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e,
                    requestList[i].outputMemory[0]->allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e,
                    segment->segmentInfo.tiling.tileYCount);
            }
            else
            {
                status = vxnneCalculateConvTilingParam(
                    graph->base.context,
                    (vxnne_convolution_relu_pooling_operation)operation,
                    tilingInfo,
                    block->bufferDesc[i + block->start].sRAM,
                    block->bufferDesc[i + block->start + 1].sRAM,
                    segment->segmentInfo.tiling.tileYCount);
            }

            if (status != VX_SUCCESS) goto OnError;

            if (phase3Hack && i == 0)
            {
                vx_uint32 outImageTileX, outImageTileY, kernelX, kernelY, kernelZ, inputDataFormat;

                kernelX = opInfo.kernelX;
                kernelY = opInfo.kernelY;
                kernelZ = opInfo.kernelZ;
                inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

                for (j = 0; j < segment->segmentInfo.tiling.tileYCount; j++)
                {
                    outImageTileX = tilingInfo[j].tilingParam.outImageTileXSize;
                    outImageTileY = tilingInfo[j].tilingParam.outImageTileYSize;


                    if (outImageTileY != 0)
                    {
                        vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, kernelZ, inputDataFormat;

                        outImageTileX = tilingInfo[j].tilingParam.outImageTileXSize;
                        outImageTileY = tilingInfo[j].tilingParam.outImageTileYSize;
                        interleaveMode = tilingInfo[j].tilingParam.interleaveMode;
                        kernelX = opInfo.weightsBiases->weights_sizes[0];
                        kernelY = opInfo.weightsBiases->weights_sizes[1];
                        kernelZ = opInfo.weightsBiases->weights_sizes[2];
                        inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

                        imageTileSize = caculate3DTileSize(graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, kernelZ, inputDataFormat, interleaveMode);
                        segment->imageCacheSize[i] = gcmMAX(segment->imageCacheSize[i], imageTileSize);
                    }
                }
            }

            totalKernelbufferSize += (vx_uint32)gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(opInfo.weightsBiases, 0), CACHE_ALIGNMENT_SIZE);
            totalImageCacheSize   += segment->imageCacheSize[i];
        }
    }

    if (totalImageCacheSize != 0)
    {
        totalKernelbufferSize = (vx_uint32)(totalKernelbufferSize*1.05f);

        if ((totalKernelbufferSize + totalImageCacheSize) > graph->base.context->vipSRAM.size)
        {
            imageTileSize = totalKernelbufferSize;

            for(i = 0; i < segment->count; i++)
            {
                 imageTileSize += segment->imageCacheSize[i];
                 if (imageTileSize > graph->base.context->vipSRAM.size)
                 {
                     /* disable image cache from here */
                     segment->imageCacheSize[i] = 0;
                 }
            }
        }
    }

    vxnneSRAM_Reset(&graph->base.context->vipSRAM);

    for(i = 0; i < segment->count; i++)
    {
        operation  = graph->layer->operations[i + segment->start];
        if (operation->operatorType == VXNNE_OPERATOR_CONVOLUTION || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
        {
            tilingInfo = segment->segmentInfo.tiling.tilingInfo  + i * segment->segmentInfo.tiling.tileYCount;

            vxnneOperation_GetInfo(operation, &opInfo);

            vxmASSERT(segment->segmentInfo.tiling.weightsBias[i] == VX_NULL);
            segment->segmentInfo.tiling.weightsBias[i] = vxoWeightsBiases_Create(
                                                             graph->base.context,
                                                             WB_BASE(opInfo.weightsBiases),
                                                             WB_BASE_WEIGHT_DIMS(opInfo.weightsBiases),
                                                             VX_NN_CONVOLUTION_LAYER,
                                                             vx_false_e);

            if (segment->segmentInfo.tiling.weightsBias[i] == VX_NULL)
            {
                status = VX_ERROR_NO_RESOURCES;
                goto OnError;
            }

            vxmASSERT(tilingInfo[0].tilingParam.kernelsPerCore != 0);

            if (opInfo.weightsBiases->sub_wb_vdata != VX_NULL)
            {
                /* Load sub WB kernel stream data from vdata*/
                if (opInfo.weightsBiases->wb_base->weightPtr == NULL)
                {
                    segment->segmentInfo.tiling.weightsBias[i]->slice_num = opInfo.weightsBiases->sub_wb_vdata->slice_num;
                    if (opInfo.weightsBiases->slice_num > 0)
                    {
                        segment->segmentInfo.tiling.weightsBias[i]->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * opInfo.weightsBiases->sub_wb_vdata->slice_num);
                        if (segment->segmentInfo.tiling.weightsBias[i]->slice_array == VX_NULL)
                        {
                            status = VX_ERROR_NO_MEMORY;
                            goto OnError;
                        }

                        for (j = 0; j  < opInfo.weightsBiases->sub_wb_vdata->slice_num; j++)
                        {
                            memcpy(&segment->segmentInfo.tiling.weightsBias[i]->slice_array[j],
                                &opInfo.weightsBiases->sub_wb_vdata->slice_array[j], sizeof(vx_weights_biases_slice_s));
                        }
                    }
                }

                if (!WeightBiasBufferAllocate(graph->base.context, segment->segmentInfo.tiling.weightsBias[i], opInfo.weightsBiases->sub_wb_vdata->wb_memory_size))
                {
                    vxError("vxnneOperationCommand_GenerateNNCommands: OUT OF MEMORY");
                    return VX_ERROR_NO_MEMORY;
                }
                vxmASSERT(opInfo.weightsBiases->sub_wb_vdata->kernel_per_core == tilingInfo[0].tilingParam.kernelsPerCore);
                memcpy(WB_MEM_LOGICAL_BASE_ADDR(segment->segmentInfo.tiling.weightsBias[i]), (vx_uint8_ptr)opInfo.weightsBiases->sub_wb_vdata->wb_memory_ptr, opInfo.weightsBiases->sub_wb_vdata->wb_memory_size);
                segment->segmentInfo.tiling.weightsBias[i]->general_compression_ratio = opInfo.weightsBiases->general_compression_ratio;
                segment->segmentInfo.tiling.weightsBias[i]->non_zero_ratio = opInfo.weightsBiases->non_zero_ratio;
#if gcdDUMP
                gcmDUMP(gcvNULL, "#[sub weights and biases]\n");
                gcmDUMP_BUFFER(gcvNULL,
                               gcvDUMP_BUFFER_MEMORY,
                               WB_MEM_PHYSICAL_BASE_ADDR(segment->segmentInfo.tiling.weightsBias[i]),
                               (gctPOINTER)WB_MEM_LOGICAL_BASE_ADDR(segment->segmentInfo.tiling.weightsBias[i]),
                               0,
                               WB_RAW_DATA_SIZE(segment->segmentInfo.tiling.weightsBias[i]));
#endif
            }
            else
            {
                if (graph->base.context->options.enableHandleBranch)
                {
                    status = vxoWeightsBiases_Compress(graph->base.context,
                        segment->segmentInfo.tiling.weightsBias[i],
                        tilingInfo[0].tilingParam.kernelsPerCore,
                        VX_NULL,
                        TENSOR_DATA_TYPE(opInfo.output),
                        requestList[i].outputMemory[0]->strides[0][2]);
                }
                else
                {
                    status = vxoWeightsBiases_Compress(graph->base.context,
                        segment->segmentInfo.tiling.weightsBias[i],
                        tilingInfo[0].tilingParam.kernelsPerCore,
                        VX_NULL,
                        TENSOR_DATA_TYPE(opInfo.output),
                        block->bufferDesc[i + 1 + segment->start -  block->segments[0].start].zStride);
                }

                if (status != VX_SUCCESS) goto OnError;

                vxoWeightsBiases_Clear(segment->segmentInfo.tiling.weightsBias[i]);

                /* Save sub WB kernel stream to original WB for SW tiling vdata*/
                sub_wb_vdata = (vx_sub_wb_vdata)vxAllocateAndZeroMemory(sizeof(vx_sub_wb_vdata_s));
                if (sub_wb_vdata == VX_NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    goto OnError;
                }

                sub_wb_vdata->slice_num = segment->segmentInfo.tiling.weightsBias[i]->slice_num;
                sub_wb_vdata->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * sub_wb_vdata->slice_num);
                if (sub_wb_vdata->slice_array == VX_NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    if (sub_wb_vdata)
                    {
                        vxFree(sub_wb_vdata);
                        sub_wb_vdata = VX_NULL;
                    }
                    goto OnError;
                }
                for(j = 0; j < sub_wb_vdata->slice_num; j++)
                {
                    memcpy(&sub_wb_vdata->slice_array[j], &segment->segmentInfo.tiling.weightsBias[i]->slice_array[j], sizeof(vx_weights_biases_slice_s));
                }
                sub_wb_vdata->wb_memory_size = WB_RAW_DATA_SIZE(segment->segmentInfo.tiling.weightsBias[i]);
                sub_wb_vdata->wb_memory_ptr = (vx_uint8_ptr)vxAllocateAndZeroMemory(sub_wb_vdata->wb_memory_size);
                if (sub_wb_vdata->wb_memory_ptr == VX_NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    if (sub_wb_vdata)
                    {
                        if (sub_wb_vdata->slice_array) {
                                vxFree(sub_wb_vdata->slice_array);
                                sub_wb_vdata->slice_array = VX_NULL;
                        }

                        vxFree(sub_wb_vdata);
                        sub_wb_vdata = VX_NULL;
                    }
                    goto OnError;
                }
                memcpy(sub_wb_vdata->wb_memory_ptr, WB_MEM_LOGICAL_BASE_ADDR(segment->segmentInfo.tiling.weightsBias[i]), sub_wb_vdata->wb_memory_size);
                sub_wb_vdata->kernel_per_core = tilingInfo[0].tilingParam.kernelsPerCore;
                opInfo.weightsBiases->sub_wb_vdata = sub_wb_vdata;
            }

             vxmASSERT(segment->kernelCacheSize[i] == 0);
             segment->kernelCacheSize[i] = (vx_uint32)gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(segment->segmentInfo.tiling.weightsBias[i], 0), CACHE_ALIGNMENT_SIZE);

             status = vxnneSRAM_Allocate(
                                &graph->base.context->vipSRAM,
                                segment->kernelCacheSize[i],
                                gcvNULL,
                                &segment->kernelCacheStart[i]);

             if (status != VX_SUCCESS) goto OnError;

             vxmASSERT(segment->imageCacheStart[i] == 0);

             if (segment->imageCacheSize[i] != 0)
             {
                 status = vxnneSRAM_Allocate(
                                    &graph->base.context->vipSRAM,
                                    segment->imageCacheSize[i],
                                    gcvNULL,
                                    &segment->imageCacheStart[i]);

                 if (status != VX_SUCCESS) goto OnError;
             }
        }
    }
    return status;

OnError:

    return status;
}

VX_PRIVATE_API vx_status GenerateBlockInfo(
    vx_graph                graph,
    vxnne_block             block
    )
{
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;
    vx_uint32 i;
    vxnne_mem_param tempMemParam = VX_NULL;
    vx_bool sRamIn, sRamOut;

    for (i = 0; i < block->segmentNum; i++)
    {
        block->count += block->segments[i].count;
    }

    block->start = block->segments[0].start;

    if (graph->base.context->options.enableHandleBranch)
    {
        gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_param_s) * block->count, (gctPOINTER*)&block->memParam);
        if (gcmIS_ERROR(gStatus))
        {
            status =  VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_param_s) * block->count, (gctPOINTER*)&tempMemParam);
        if (gcmIS_ERROR(gStatus))
        {
            status =  VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        /* backup mem param to restore */
        status = GetMemoryParamList(graph, block->start, block->count, tempMemParam);
        if (status != VX_SUCCESS) goto OnError;
    }

    for (i = 0; i < block->segmentNum; i++)
    {
        sRamIn = (i == 0 ? vx_false_e : vx_true_e);
        sRamOut = (i == block->segmentNum - 1 ? vx_false_e : vx_true_e);

        if (block->segments[i].type == VXNNE_SEGMENT_TYPE_AB)
        {
            status = GenerateABSegmentInfo(graph, block, &block->segments[i], sRamIn, sRamOut);
            if (status != VX_SUCCESS) goto OnError;
            block->totalCommandCount += block->segments[i].count;
        }
        else if (block->segments[i].type == VXNNE_SEGMENT_TYPE_TILING)
        {
            status = GenerateTilingSegmentInfo(graph, block, &block->segments[i], sRamIn, sRamOut);
            if (status != VX_SUCCESS) goto OnError;
            block->totalCommandCount += block->segments[i].count * block->segments[i].segmentInfo.tiling.tileYCount;
        }
        else
        {
            vxmASSERT(0);
            status =  VX_ERROR_NOT_SUPPORTED;
            goto OnError;
        }
    }

    if (graph->base.context->options.enableHandleBranch)
    {
        status = vxoMemoryPool_RequestList(graph, graph->layer->memRequestList, graph->layer->base.num_operations, block->start, block->count, VX_NULL);
        if (status != VX_SUCCESS)
        {
            vxmASSERT(0);
            goto OnError;
        }

        status = GetMemoryParamList(graph, block->start, block->count, block->memParam);
        if (status != VX_SUCCESS) goto OnError;

        status = RestorePartialMemoryParamList(graph, block->start, block->count, tempMemParam);
        if (status != VX_SUCCESS) goto OnError;

#if SW_TILING_DEBUG

        for (i = 0; i < block->count; i++)
        {
            if (block->memParam[i].inputMemory[0].physicals[0] != block->memParam[i].outputMemory[0].physicals[0] &&
                block->memParam[i].inputMemory[0].allocType == block->memParam[i].outputMemory[0].allocType)
            {
                if (block->memParam[i].inputMemory[0].physicals[0] < block->memParam[i].outputMemory[0].physicals[0])
                {
                    vxmASSERT(block->memParam[i].inputMemory[0].physicals[0] + block->memParam[i].inputMemory[0].sizes[0] <=
                        block->memParam[i].outputMemory[0].physicals[0]);
                }
                else
                {
                    vxmASSERT(block->memParam[i].inputMemory[0].physicals[0] >=
                        block->memParam[i].outputMemory[0].physicals[0]  + block->memParam[i].outputMemory[0].sizes[0]);
                }
            }
        }
#endif

    }
    else
    {
        vxnneSRAM_Reset(&graph->base.context->axiSRAM);

        for(i = 0; i < block->bufferCount; i++)
        {
            if (block->bufferDesc[i].bufferSize > 0)
            {
                vxmASSERT(block->bufferDesc[i].sRAM);

                status = vxnneSRAM_AllocateEx(&graph->base.context->axiSRAM,
                                    block->bufferDesc[i].bufferSize,
                                    &block->bufferDesc[i].logical,
                                    &block->bufferDesc[i].physical,
                                    block->bufferDesc[i].allocateFlag);

                if (status != VX_SUCCESS)  goto OnError;

                if (block->bufferDesc[i].circle)
                {
                    block->bufferDesc[i].circularBufEndAddrPlus1 = block->bufferDesc[i].bufferSize + block->bufferDesc[i].physical;
                    /*need to 256 byte alignment */
                    vxmASSERT(!(block->bufferDesc[i].circularBufEndAddrPlus1 & (CACHE_ALIGNMENT_SIZE - 1)));
                }
                else
                {
                    block->bufferDesc[i].circularBufEndAddrPlus1 = 0xFFFFFFFF;
                }

            }
            else
            {
                block->bufferDesc[i].circularBufEndAddrPlus1 = 0xFFFFFFFF;
                vxmASSERT(!block->bufferDesc[i].sRAM);
            }
        }
    }


#if SW_TILING_DEBUG

    {
        vxnne_tiling_info_s* tilingInfo;
        vx_uint32  j, k, sRamPeak = 0;
        vxnne_operation_info_s opInfo;
        char* opTarget[5] = {"NONE", "SH", "NN", "TP", "SW"};

        vxInfo("======================= start block  ==============================\n");
        vxInfo("            input          output    kernelSize  pad   pooling\n");
        for(i = block->start; i < block->start + block->count; i++)
        {
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);
            vxInfo("%3d %s [(%3d %3d %4d, %3d %3d %4d) (%d, %d) (%d, %d) (%d, %d, %d, %d)] \n",
                    i, opTarget[opInfo.target],
                    TENSOR_SIZE_INDEX(opInfo.input, 0), TENSOR_SIZE_INDEX(opInfo.input, 1), TENSOR_SIZE_INDEX(opInfo.input, 2),
                    TENSOR_SIZE_INDEX(opInfo.output, 0), TENSOR_SIZE_INDEX(opInfo.output, 1), TENSOR_SIZE_INDEX(opInfo.output, 2),
                    opInfo.kernelX, opInfo.kernelY,
                    opInfo.pad.left, opInfo.pad.top,
                    opInfo.poolSizeX, opInfo.poolSizeY, opInfo.poolStrideX, opInfo.poolStrideY);
        }


        for (i = 0; i < block->segmentNum; i++)
        {
            vxInfo("--------------------------------------------------------\n");
            if (block->segments[i].type == VXNNE_SEGMENT_TYPE_AB)
            {
                vxInfo("Segment AB (%d - %d)\n", block->segments[i].start, block->segments[i].start + block->segments[i].count - 1);
                sRamPeak = 0;
                if (graph->base.context->options.enableHandleBranch)
                {
                    for(j = block->segments[i].start; j < block->segments[i].start + block->segments[i].count; j++)
                    {
                        vxInfo("[%s -> %s]  ", block->memParam[j - block->start].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? "S" : "D",
                                                    block->memParam[j - block->start].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM? "S" : "D");

                        sRamPeak = gcmMAX(sRamPeak,
                            (block->memParam[j - block->start].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? (vx_uint32)block->memParam[j - block->start].inputMemory[0].sizes[0] : 0)
                          + (block->memParam[j - block->start].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? (vx_uint32)block->memParam[j - block->start].outputMemory[0].sizes[0] : 0));
                    }
                }
                else
                {
                    for(j = block->segments[i].start; j < block->segments[i].start + block->segments[i].count; j++)
                    {
                        vxInfo("[%s -> %s]  ", block->bufferDesc[j - block->start].sRAM ? "S" : "D",
                                                    block->bufferDesc[j - block->start+1].sRAM ? "S" : "D");
                        sRamPeak = gcmMAX(sRamPeak, block->bufferDesc[j - block->start].bufferSize + block->bufferDesc[j-block->start+1].bufferSize);
                    }
                }
            }
            else
            {
                vxInfo("Segment Tiling (%d - %d)\n", block->segments[i].start, block->segments[i].start +  block->segments[i].count - 1);

                for(j = 0; j <block->segments[i].segmentInfo.tiling.tileYCount; j++)
                {
                    for(k = block->segments[i].start; k < block->segments[i].start + block->segments[i].count; k++)
                    {
                        tilingInfo = block->segments[i].segmentInfo.tiling.tilingInfo + (k - block->segments[i].start) * block->segments[i].segmentInfo.tiling.tileYCount;

                        if (graph->base.context->options.enableHandleBranch)
                        {
                            vxInfo("[%s%3d(%3d,%3d)(%3d) ->%s%3d(%3d,%3d)(%3d)]    ",
                                block->memParam[k - block->start].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? "S" : "D", tilingInfo[j].input.height, tilingInfo[j].input.start, tilingInfo[j].input.end,
                                block->memParam[k - block->start].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? block->memParam[k - block->start].inputMemory[0].dims[0][1] : 0,
                                block->memParam[k - block->start].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? "S" : "D", tilingInfo[j].output.height, tilingInfo[j].output.start, tilingInfo[j].output.end,
                                block->memParam[k - block->start].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? block->memParam[k - block->start].outputMemory[0].dims[0][1]:0
                                );
                        }
                        else
                        {
                            vxInfo("[%s%3d(%3d,%3d)(%3d) ->%s%3d(%3d,%3d)(%3d) %d]    ",
                                block->bufferDesc[k - block->start].sRAM ? "S" : "D", tilingInfo[j].input.height, tilingInfo[j].input.start, tilingInfo[j].input.end,
                                block->bufferDesc[k - block->start].sRAM ? (block->bufferDesc[k - block->start].zStride / block->bufferDesc[k - block->start].yStride) : 0,
                                block->bufferDesc[k - block->start+1].sRAM ? "S" : "D", tilingInfo[j].output.height, tilingInfo[j].output.start, tilingInfo[j].output.end,
                                block->bufferDesc[k - block->start+1].sRAM ? (block->bufferDesc[k - block->start+1].zStride / block->bufferDesc[k - block->start+1].yStride):0, tilingInfo[j].kernelMode);
                        }
                    }

                    vxInfo("\n");
                }

                sRamPeak = 0;

                if (graph->base.context->options.enableHandleBranch)
                {
                    for(j = block->segments[i].start; j < block->segments[i].start + block->segments[i].count; j++)
                    {
                        if (block->memParam[j - block->start].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM)
                        {

                            sRamPeak += (vx_uint32)block->memParam[j - block->start].inputMemory[0].sizes[0];
                        }
                    }

                    if (block->memParam[j - block->start - 1].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM)
                    {
                        sRamPeak += (vx_uint32)block->memParam[j - block->start - 1].outputMemory[0].sizes[0];
                    }
                }
                else
                {
                    for(j = block->segments[i].start; j <= block->segments[i].start + block->segments[i].count; j++)
                    {
                        sRamPeak += block->bufferDesc[j - block->start].bufferSize;
                    }
                }
            }

            vxInfo("\nCacheState = [IC KC][ ");
            for(j = 0; j < block->segments[i].count; j++)
            {
                vxInfo("%d %d", block->segments[i].imageCacheSize[j] == 0 ? 0 : 1, block->segments[i].kernelCacheSize[j] == 0 ? 0 : 1);
                if (j !=  block->segments[i].count - 1)
                {
                    vxInfo(",");
                }
            }
            vxInfo("]");

            vxInfo("\nAXI SRAM peak used %d Bytes %f%%\n",
                sRamPeak,
                ((vx_float32)sRamPeak / graph->base.context->axiSRAM.size) * 100);

        }

        vxInfo("======================= End block  ==============================\n");

    }
#endif

OnError:
    if (tempMemParam) gcoOS_Free(gcvNULL, tempMemParam);
    return status;
}

VX_PRIVATE_API vxnne_segment GetNextSegment(
        vx_uint32                   *abCurrent,
        vxnne_segment_collection    abCollection,
        vx_uint32                   *tilingCurrent,
        vxnne_segment_collection    tilingCollection)
{
    vxnne_segment next;

    if (*abCurrent >= abCollection->segmentNum &&
        *tilingCurrent >= tilingCollection->segmentNum)
    {
        return VX_NULL;
    }


    if (*abCurrent >= abCollection->segmentNum ||
        (*tilingCurrent < tilingCollection->segmentNum && abCollection->segments[*abCurrent].start > tilingCollection->segments[*tilingCurrent].start))
    {
        next = &tilingCollection->segments[*tilingCurrent];
        (*tilingCurrent)++;
    }
    else
    {
        next = &abCollection->segments[*abCurrent];
        (*abCurrent)++;
    }

    return next;

}

VX_PRIVATE_API vx_status GenerateBlocks(
    vx_graph                    graph,
    vxnne_segment_collection    abCollection,
    vxnne_segment_collection    tilingCollection
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i, abCurrent = 0, tilingCurrent = 0;
    vxnne_segment next;
    vxnne_block   block = &graph->layer->blocks[graph->layer->blockNum];

    if (abCollection->segmentNum + tilingCollection->segmentNum == 0)
    {
        return VX_SUCCESS;
    }

    for(i = 0; i < abCollection->segmentNum + tilingCollection->segmentNum; i++)
    {
        next = GetNextSegment(&abCurrent, abCollection, &tilingCurrent, tilingCollection);
        vxmASSERT(next);

        if (block->segmentNum == 0)
        {
            block->segments[block->segmentNum] = *next;
            block->segmentNum++;
            vxmASSERT(block->segmentNum <= 2 * VX_MAX_SEGMENT_COUNT);
        }
        else
        {
            if (block->segments[block->segmentNum - 1].end == next->start)
            {
                /* remove the overlap operation */
                if (block->segments[block->segmentNum - 1].type == VXNNE_SEGMENT_TYPE_AB)
                {
                    vxmASSERT(next->type == VXNNE_SEGMENT_TYPE_TILING);
                    block->segments[block->segmentNum - 1].count -= 1;
                    block->segments[block->segmentNum - 1].end -= 1;
                }
                else
                {
                    vxmASSERT(next->type == VXNNE_SEGMENT_TYPE_AB);
                    next->start += 1;
                    next->count -= 1;
                }

                /* merge segments */
                block->segments[block->segmentNum] = *next;
                block->segmentNum++;
                vxmASSERT(block->segmentNum <= 2 * VX_MAX_SEGMENT_COUNT);
            }
            else
            {
                /* block prepared */
                status = GenerateBlockInfo(graph, block);
                if (status != VX_SUCCESS) goto OnError;

                graph->layer->blockNum++;
                if (graph->layer->blockNum > VX_MAX_BLOCK_COUNT)
                {
                    status = VX_ERROR_NO_RESOURCES;
                    goto OnError;
                }

                block = &graph->layer->blocks[graph->layer->blockNum];
                block->segments[block->segmentNum] = *next;
                block->segmentNum++;
                vxmASSERT(block->segmentNum <= 2 * VX_MAX_SEGMENT_COUNT);
            }
        }
    }

    /*last block prepared */
    status = GenerateBlockInfo(graph, block);

    /* pointer to next block */
    graph->layer->blockNum++;
    if (graph->layer->blockNum > VX_MAX_BLOCK_COUNT)
    {
        status = VX_ERROR_NO_RESOURCES;
        goto OnError;
    }

OnError:
    return status;
}


VX_PRIVATE_API vx_status AnalyzeBlock(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 SWTilingOption = graph->base.context->options.enableSwtilingPhase1;

    vxnne_segment_collection_s abCollection={0}, tilingCollection={0};

#if SW_TILING_DEBUG
    vxInfo("Analyze block (%d - %d)\n", start, start + count - 1);
#endif

    if ((SWTilingOption == VX_SWTILING_OPTION_ALL) ||
        (SWTilingOption == VX_SWTILING_OPTION_AB))
    {
        status  = DetectABSegments(graph, start, count, &abCollection);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }

    if ((SWTilingOption == VX_SWTILING_OPTION_ALL) ||
        (SWTilingOption == VX_SWTILING_OPTION_TILING))
    {
        status  = DetectTilingSegments(graph, start, count, &abCollection, &tilingCollection);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }

    status = GenerateBlocks(graph, &abCollection, &tilingCollection);
    if (status != VX_SUCCESS)
    {
        goto OnError;
    }

OnError:
    return status;
}


VX_PRIVATE_API vx_status InitializeABSegmentCommands(
    vx_graph                graph,
    vxnne_block             block,
    vxnne_segment           segment)
{
    vx_uint32 i, bufferID;
    vxnne_operation_info_s opInfo;
    vxnne_operation_command opCommand;
    vx_status status = VX_SUCCESS;
    vx_uint32 offset = 0;

    for(i = segment->start; i < segment->start + segment->count; i++)
    {
        opCommand = &graph->layer->opIndices[graph->layer->opIndicesNum];
        bufferID  = i - block->start;
        vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

        if (i == segment->start)
        {
            opCommand->segmentFlag = VXNNE_SEGMENT_FLAG_START;

            if (&block->segments[0] == segment)
            {
                opCommand->blockFlag = VXNNE_BLOCK_FLAG_START;
            }
            else
            {
                opCommand->blockFlag = VXNNE_BLOCK_FLAG_INTERNAL;
            }
        }
        else if (i == segment->start + segment->count - 1)
        {
            opCommand->segmentFlag = VXNNE_SEGMENT_FLAG_END;

            if (&block->segments[block->segmentNum - 1] == segment)
            {
                opCommand->blockFlag = VXNNE_BLOCK_FLAG_END;
            }
            else
            {
                opCommand->blockFlag = VXNNE_BLOCK_FLAG_INTERNAL;
            }
        }
        else
        {
            opCommand->segmentFlag = VXNNE_SEGMENT_FLAG_INTERNAL;
            opCommand->blockFlag   = VXNNE_BLOCK_FLAG_INTERNAL;
        }

        opCommand->operationID = i;
        opCommand->operation   = graph->layer->operations[i];

        opCommand->inputTile.x        = 0;
        opCommand->inputTile.y        = 0;
        opCommand->inputTile.width    = TENSOR_SIZE_INDEX(opInfo.input, 0);
        opCommand->inputTile.height   = TENSOR_SIZE_INDEX(opInfo.input, 1);
        opCommand->inputTile.xStride  = TENSOR_STRIDE_INDEX(opInfo.input, 0);
        opCommand->inputTile.yStride  = TENSOR_STRIDE_INDEX(opInfo.input, 1);
        opCommand->inputTile.zStride  = TENSOR_STRIDE_INDEX(opInfo.input, 2);

        if (graph->base.context->options.enableHandleBranch)
        {
            opCommand->inputTile.sRAM                    = block->memParam[bufferID].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e;

            if (opCommand->inputTile.sRAM)
            {
                vxoTensor_GetTensorViewOffset(opInfo.input, &offset);
                opCommand->inputTile.logical                 = block->memParam[bufferID].inputMemory[0].logicals[0] + offset;
                opCommand->inputTile.logicalBase             = block->memParam[bufferID].inputMemory[0].logicals[0] + offset;
                opCommand->inputTile.physical                = block->memParam[bufferID].inputMemory[0].physicals[0] + offset;

                opCommand->inputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].inputMemory[0].sizes[0];
                opCommand->inputTile.circularBufEndAddrPlus1 = block->memParam[bufferID].inputMemory[0].physicals[0] + opCommand->inputTile.circleBufferSize;
                vxmASSERT(!(opCommand->inputTile.circularBufEndAddrPlus1 & (CACHE_ALIGNMENT_SIZE - 1)));
                vxmASSERT(!(opCommand->inputTile.circleBufferSize & (CACHE_ALIGNMENT_SIZE - 1)));
                vxmASSERT(opCommand->inputTile.circleBufferSize > 0 );
            }
            else
            {
                opCommand->inputTile.circleBufferSize        = 0;
                opCommand->inputTile.circularBufEndAddrPlus1 = 0xFFFFFFFF;
            }

            vxmASSERT(!opCommand->inputTile.sRAM || opCommand->inputTile.physical);
        }
        else
        {
            opCommand->inputTile.sRAM     = block->bufferDesc[bufferID].sRAM;
            opCommand->inputTile.logical  = block->bufferDesc[bufferID].logical;
            opCommand->inputTile.logicalBase  = block->bufferDesc[bufferID].logical;
            opCommand->inputTile.physical = block->bufferDesc[bufferID].physical;
            opCommand->inputTile.circleBufferSize        = block->bufferDesc[bufferID].bufferSize;
            opCommand->inputTile.circularBufEndAddrPlus1 = block->bufferDesc[bufferID].circularBufEndAddrPlus1;
            bufferID++;
        }



        opCommand->outputTile.x         = 0;
        opCommand->outputTile.y         = 0;
        opCommand->outputTile.width     = TENSOR_SIZE_INDEX(opInfo.output, 0);
        opCommand->outputTile.height    = TENSOR_SIZE_INDEX(opInfo.output, 1);
        opCommand->outputTile.xStride   = TENSOR_STRIDE_INDEX(opInfo.output, 0);
        opCommand->outputTile.yStride   = TENSOR_STRIDE_INDEX(opInfo.output, 1);
        opCommand->outputTile.zStride   = TENSOR_STRIDE_INDEX(opInfo.output, 2);
        if (graph->base.context->options.enableHandleBranch)
        {

            opCommand->outputTile.sRAM                    = block->memParam[bufferID].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e;
            if (opCommand->outputTile.sRAM)
            {
                vxoTensor_GetTensorViewOffset(opInfo.output, &offset);
                opCommand->outputTile.logical                 = block->memParam[bufferID].outputMemory[0].logicals[0] + offset;
                opCommand->outputTile.logicalBase             = block->memParam[bufferID].outputMemory[0].logicals[0] + offset;
                opCommand->outputTile.physical                = block->memParam[bufferID].outputMemory[0].physicals[0] + offset;

                opCommand->outputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].outputMemory[0].sizes[0];
                opCommand->outputTile.circularBufEndAddrPlus1 = block->memParam[bufferID].outputMemory[0].physicals[0] + opCommand->outputTile.circleBufferSize;
                vxmASSERT(!(opCommand->outputTile.circularBufEndAddrPlus1 & (CACHE_ALIGNMENT_SIZE - 1)));
                vxmASSERT(!(opCommand->outputTile.circleBufferSize & (CACHE_ALIGNMENT_SIZE - 1)));
                vxmASSERT(opCommand->outputTile.circleBufferSize > 0 );
            }
            else
            {
                opCommand->outputTile.circleBufferSize        = 0;
                opCommand->outputTile.circularBufEndAddrPlus1 = 0xFFFFFFFF;
            }

            vxmASSERT(!opCommand->outputTile.sRAM || opCommand->outputTile.physical);
        }
        else
        {
            opCommand->outputTile.sRAM      = block->bufferDesc[bufferID].sRAM;
            opCommand->outputTile.logical   = block->bufferDesc[bufferID].logical;
            opCommand->outputTile.logicalBase   = block->bufferDesc[bufferID].logical;
            opCommand->outputTile.physical  = block->bufferDesc[bufferID].physical;
            opCommand->outputTile.circleBufferSize        = block->bufferDesc[bufferID].bufferSize;
            opCommand->outputTile.circularBufEndAddrPlus1 = block->bufferDesc[bufferID].circularBufEndAddrPlus1;
        }


        opCommand->cmdInfo.padLeft      = opInfo.pad.left;
        opCommand->cmdInfo.padTop       = opInfo.pad.top;
        opCommand->cmdInfo.convWidth    = opCommand->outputTile.width;
        opCommand->cmdInfo.convHeight   = opCommand->outputTile.height;
        if (opInfo.enablePooling)
        {
            vxmONERROR(ComputeInputSizeEx(
                            opInfo.opType,
                            opCommand->outputTile.width,
                            0,
                            0,
                            0,
                            opInfo.poolSizeX,
                            opInfo.poolStrideX,
                            opInfo.reshuffStrideX,
                            opInfo.normStrideX,
                            &opCommand->cmdInfo.convWidth,
                            VX_NULL
                            ));

            vxmONERROR(ComputeInputSizeEx(
                            opInfo.opType,
                            opCommand->outputTile.height,
                            0,
                            0,
                            0,
                            opInfo.poolSizeY,
                            opInfo.poolStrideY,
                            opInfo.reshuffStrideY,
                            opInfo.normStrideY,
                            &opCommand->cmdInfo.convHeight,
                            VX_NULL
                            ));
        }
        opCommand->cmdInfo.wb           = VX_NULL;
        opCommand->cmdInfo.flush        = vx_true_e;
        opCommand->dump                 = vxOpCommandDump;

        graph->layer->opIndicesNum++;
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status InitializeTilingSegmentCommands(
    vx_graph                graph,
    vxnne_block             block,
    vxnne_segment           segment)
{
    vx_status status = VX_SUCCESS;

    vxnne_tiling_info tilingInfo = VX_NULL;
    vx_uint32 l, k, i, bufferID;

    vxnne_operation_command         opCommand;
    vxnne_segment_tiling_info_s*    segmentTiling = &segment->segmentInfo.tiling;

    for (l = 0; l < segmentTiling->tileXCount; l++)
    {
        vx_uint32 kStart, iStart, offset = 0;
        vxnne_tiling_info prevTilingInfo = VX_NULL;

        for (kStart = 0; kStart < segmentTiling->count; kStart++)
        {
            for (iStart = 0; iStart < (kStart == segmentTiling->count - 1 ? segmentTiling->tileYCount : 1); iStart++)
            {
                for (i = iStart, k = kStart; ((vx_int32)k >= 0) && i < segmentTiling->tileYCount; k--, i++)
                {
                    tilingInfo = segmentTiling->tilingInfo + k * segmentTiling->tileYCount;

                    if (tilingInfo[i].output.height != 0)
                    {
                        opCommand = &graph->layer->opIndices[graph->layer->opIndicesNum];
                        bufferID  = segmentTiling->start + k - block->start;

                        if (i == 0 && k == 0)
                        {
                            opCommand->segmentFlag = VXNNE_SEGMENT_FLAG_START;

                            if (&block->segments[0] == segment)
                            {
                                opCommand->blockFlag = VXNNE_BLOCK_FLAG_START;
                            }
                            else
                            {
                                opCommand->blockFlag = VXNNE_BLOCK_FLAG_INTERNAL;
                            }
                        }
                        else if ((i == segmentTiling->tileYCount - 1) && (k == segmentTiling->count - 1))
                        {
                            opCommand->segmentFlag          = VXNNE_SEGMENT_FLAG_END;
                            if (&block->segments[block->segmentNum - 1] == segment)
                            {
                                opCommand->blockFlag = VXNNE_BLOCK_FLAG_END;
                            }
                            else
                            {
                                opCommand->blockFlag = VXNNE_BLOCK_FLAG_INTERNAL;
                            }
                        }
                        else
                        {
                            opCommand->segmentFlag          = VXNNE_SEGMENT_FLAG_INTERNAL;
                            opCommand->blockFlag            = VXNNE_BLOCK_FLAG_INTERNAL;
                        }

                        opCommand->operationID          = segmentTiling->start + k;
                        opCommand->inputTile.x          = 0;
                        opCommand->inputTile.y          = tilingInfo[i].input.start;
                        opCommand->inputTile.width      = tilingInfo[i].input.width;
                        opCommand->inputTile.height     = tilingInfo[i].input.height;
                        opCommand->cmdInfo.padLeft      = tilingInfo[i].padLeft;
                        opCommand->cmdInfo.padTop       = tilingInfo[i].padTop;

                        if (graph->base.context->options.enableHandleBranch)
                        {
                            opCommand->inputTile.sRAM        = block->memParam[bufferID].inputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e;

                            if (opCommand->inputTile.sRAM)
                            {
                                opCommand->inputTile.xStride     = block->memParam[bufferID].inputMemory[0].strides[0][0];
                                opCommand->inputTile.yStride     = block->memParam[bufferID].inputMemory[0].strides[0][1];
                                opCommand->inputTile.zStride     = block->memParam[bufferID].inputMemory[0].strides[0][2];

                                opCommand->inputTile.logicalBase = block->memParam[bufferID].inputMemory[0].logicals[0];
                                opCommand->inputTile.physical  = block->memParam[bufferID].inputMemory[0].physicals[0];
                                opCommand->inputTile.logical   = block->memParam[bufferID].inputMemory[0].logicals[0];

                                opCommand->inputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].inputMemory[0].sizes[0];
                                opCommand->inputTile.circularBufEndAddrPlus1 = opCommand->inputTile.physical + opCommand->inputTile.circleBufferSize;
                                vxmASSERT(!(opCommand->inputTile.circularBufEndAddrPlus1 & (CACHE_ALIGNMENT_SIZE - 1)));
                                vxmASSERT(!(opCommand->inputTile.circleBufferSize & (CACHE_ALIGNMENT_SIZE - 1)));
                                vxmASSERT(opCommand->inputTile.circleBufferSize > 0 );
                            }
                            else
                            {
                                opCommand->inputTile.xStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 0);
                                opCommand->inputTile.yStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 1);
                                opCommand->inputTile.zStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 2);

                                opCommand->inputTile.circleBufferSize = 0;
                                opCommand->inputTile.circularBufEndAddrPlus1 = 0xFFFFFFFF;
                            }

                            vxmASSERT(!opCommand->inputTile.sRAM || opCommand->inputTile.physical);
                        }
                        else
                        {
                            opCommand->inputTile.logicalBase = block->bufferDesc[bufferID].logical;
                            opCommand->inputTile.xStride     = block->bufferDesc[bufferID].xStride;
                            opCommand->inputTile.yStride     = block->bufferDesc[bufferID].yStride;
                            opCommand->inputTile.zStride     = block->bufferDesc[bufferID].zStride;
                            opCommand->inputTile.sRAM        = block->bufferDesc[bufferID].sRAM;
                            opCommand->inputTile.circleBufferSize        = block->bufferDesc[bufferID].circle ? block->bufferDesc[bufferID].bufferSize : 0;
                            opCommand->inputTile.circularBufEndAddrPlus1 = block->bufferDesc[bufferID].circularBufEndAddrPlus1;

                            opCommand->inputTile.physical  = block->bufferDesc[bufferID].physical;
                            opCommand->inputTile.logical   = (vx_uint8*)block->bufferDesc[bufferID].logical;

                            bufferID++;
                        }

                        offset = opCommand->inputTile.x * opCommand->inputTile.xStride
                                    + opCommand->inputTile.y * opCommand->inputTile.yStride;

                        if (opCommand->inputTile.sRAM && opCommand->inputTile.circleBufferSize != 0)
                        {
                            offset = offset % opCommand->inputTile.circleBufferSize;
                        }

                        opCommand->inputTile.physical = opCommand->inputTile.physical + offset;
                        opCommand->inputTile.logical  = (vx_uint8*)opCommand->inputTile.logical + offset;

                        vxmASSERT(!opCommand->inputTile.sRAM || opCommand->inputTile.physical < opCommand->inputTile.circularBufEndAddrPlus1);

                        opCommand->outputTile.x             = 0;
                        opCommand->outputTile.y             = tilingInfo[i].output.start;
                        opCommand->outputTile.width         = tilingInfo[i].output.width;
                        opCommand->outputTile.height        = tilingInfo[i].output.height;
                        opCommand->cmdInfo.convWidth        = tilingInfo[i].convWidth;
                        opCommand->cmdInfo.convHeight       = tilingInfo[i].convHeight;
                        if (graph->base.context->options.enableHandleBranch)
                        {
                            opCommand->outputTile.sRAM        = block->memParam[bufferID].outputMemory[0].allocType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ? vx_true_e : vx_false_e;

                            if (opCommand->outputTile.sRAM)
                            {
                                opCommand->outputTile.xStride     = block->memParam[bufferID].outputMemory[0].strides[0][0];
                                opCommand->outputTile.yStride     = block->memParam[bufferID].outputMemory[0].strides[0][1];
                                opCommand->outputTile.zStride     = block->memParam[bufferID].outputMemory[0].strides[0][2];

                                opCommand->outputTile.logicalBase = block->memParam[bufferID].outputMemory[0].logicals[0];
                                opCommand->outputTile.physical  = block->memParam[bufferID].outputMemory[0].physicals[0];
                                opCommand->outputTile.logical   = block->memParam[bufferID].outputMemory[0].logicals[0];

                                opCommand->outputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].outputMemory[0].sizes[0];
                                opCommand->outputTile.circularBufEndAddrPlus1 = opCommand->outputTile.physical + opCommand->outputTile.circleBufferSize;
                                vxmASSERT(!(opCommand->outputTile.circularBufEndAddrPlus1 & (CACHE_ALIGNMENT_SIZE - 1)));
                                vxmASSERT(!(opCommand->outputTile.circleBufferSize & (CACHE_ALIGNMENT_SIZE - 1)));
                                vxmASSERT(opCommand->outputTile.circleBufferSize > 0 );
                            }
                            else
                            {
                                opCommand->outputTile.xStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 0);
                                opCommand->outputTile.yStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 1);
                                opCommand->outputTile.zStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 2);

                                opCommand->outputTile.circleBufferSize = 0;
                                opCommand->outputTile.circularBufEndAddrPlus1 = 0xFFFFFFFF;
                            }

                            vxmASSERT(!opCommand->outputTile.sRAM || opCommand->outputTile.physical);
                        }
                        else
                        {
                            opCommand->outputTile.logicalBase   =  block->bufferDesc[bufferID].logical;
                            opCommand->outputTile.xStride       = block->bufferDesc[bufferID].xStride;
                            opCommand->outputTile.yStride       = block->bufferDesc[bufferID].yStride;
                            opCommand->outputTile.zStride       = block->bufferDesc[bufferID].zStride;
                            opCommand->outputTile.sRAM          = block->bufferDesc[bufferID].sRAM;
                            opCommand->outputTile.circleBufferSize        = block->bufferDesc[bufferID].circle ? block->bufferDesc[bufferID].bufferSize : 0;
                            opCommand->outputTile.circularBufEndAddrPlus1 = block->bufferDesc[bufferID].circularBufEndAddrPlus1;

                            opCommand->outputTile.physical  = block->bufferDesc[bufferID].physical;
                            opCommand->outputTile.logical   = block->bufferDesc[bufferID].logical;
                        }

                        offset = opCommand->outputTile.x * opCommand->outputTile.xStride
                                    + opCommand->outputTile.y * opCommand->outputTile.yStride;

                        if (opCommand->outputTile.sRAM && opCommand->outputTile.circleBufferSize != 0)
                        {
                            offset = offset % opCommand->outputTile.circleBufferSize;
                        }

                        opCommand->outputTile.physical  = opCommand->outputTile.physical + offset;
                        opCommand->outputTile.logical   = (vx_uint8*)opCommand->outputTile.logical + offset;

                        vxmASSERT(!opCommand->outputTile.sRAM || opCommand->outputTile.physical < opCommand->outputTile.circularBufEndAddrPlus1);

                        if ((i + 1) <  segmentTiling->tileYCount && k >= 1)
                        {
                            prevTilingInfo = segmentTiling->tilingInfo + (k - 1) * segmentTiling->count;

                            if (prevTilingInfo[i+1].output.height == 0)
                            {
                                opCommand->cmdInfo.flush = vx_true_e;
                            }
                        }
                        else
                        {
                            opCommand->cmdInfo.flush = vx_true_e;
                        }

                        opCommand->cmdInfo.tilingParam = tilingInfo[i].tilingParam;
#if SW_TILING_DEBUG
                        vxInfo("[(%3d, %3d) %d] ", k, i, opCommand->cmdInfo.flush);
                        if (opCommand->cmdInfo.flush)
                        {
                            vxInfo("\n");
                        }
#endif

                        opCommand->cmdInfo.imageCacheSize  = segment->imageCacheSize[k];
                        opCommand->cmdInfo.imageCacheStart = segment->imageCacheStart[k];
                        opCommand->cmdInfo.imageCacheMode  = segment->imageCacheSize[k] == 0 ? VXNNE_SRAM_CACHE_MODE_NONE : VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                        opCommand->cmdInfo.kernelCacheSize  = segment->kernelCacheSize[k];
                        opCommand->cmdInfo.kernelCacheStart = segment->kernelCacheStart[k];
                        opCommand->cmdInfo.kernelCacheMode  = opCommand->inputTile.y == 0 ? VXNNE_SRAM_CACHE_MODE_FULL_CACHE : VXNNE_SRAM_CACHE_MODE_STREAM_CACHE;

                        opCommand->cmdInfo.wb  = segmentTiling->weightsBias[k];
                        opCommand->operation   = graph->layer->operations[opCommand->operationID];
                        opCommand->dump        = vxOpCommandDump;


                        graph->layer->opIndicesNum++;
                    }

                }
            }
        }
    }

    return status;

}

VX_PRIVATE_API vx_status InitializeBlock(
    vx_graph                graph,
    vxnne_block             block)
{
    vx_uint32 i;
    vx_status status = VX_SUCCESS;

    for (i = 0; i < block->segmentNum; i++)
    {
        if (block->segments[i].type == VXNNE_SEGMENT_TYPE_AB)
        {
            status = InitializeABSegmentCommands(graph, block, &block->segments[i]);
        }
        else if (block->segments[i].type == VXNNE_SEGMENT_TYPE_TILING)
        {
            status = InitializeTilingSegmentCommands(graph, block, &block->segments[i]);
        }
        else
        {
            vxmASSERT(0);
            status =  VX_ERROR_NOT_SUPPORTED;
        }
    }

    return status;
}


VX_INTERNAL_API vx_status vxoGraph_VerifyTiling(vx_graph graph)
{
    vx_uint32 i = 0, j = 0, k = 0, count = 0, start = 0;

    vxnne_execution_layer layer = VX_NULL;
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;
    vx_uint32 s = 0, e = 0, maxOpCommandCount = 0;
    vx_bool   terminator = vx_false_e;
    vxnne_operation_info_s opInfo1 = {0}, opInfo2 = {0};

    if (!graph->layer) return VX_SUCCESS;

    layer = (vxnne_execution_layer)graph->layer;

#if SW_TILING_DEBUG
    {
        char* opTarget[6] = {"NONE", "SH", "NN", "TP", "SW", "SC"};
        vx_uint32 offsetIn = 0, offsetOut = 0;
        vxInfo("---------------------------Begin VerifyTiling -------------------------\n");
        vxInfo("AXI-SRAM = %d Bytes VIP-SRAM = %d Bytes\n", graph->base.context->axiSRAM.size, graph->base.context->vipSRAM.size);
        for(i = 0; i < graph->layer->base.num_operations; i++)
        {
            offsetIn = 0; offsetOut = 0;
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo1);
            if (opInfo1.input)
            {
                vxoTensor_GetTensorViewOffset(opInfo1.input, &offsetIn);

            }
            if (opInfo1.output)
            {
                vxoTensor_GetTensorViewOffset(opInfo1.output, &offsetOut);
            }

            vxInfo("%3d %s (branch %d) kernelSize = %9d, [%3d %3d %3d %3d 0x%p, %3d %3d %3d %3d 0x%p]  IN+OUT= %f%%\n", i,
                        opTarget[graph->layer->operations[i]->target],
                        (graph->layer->operations[i]->childOpNum > 1  || graph->layer->operations[i]->parentOpNum > 1 )? 1 : 0,
                        opInfo1.weightsBiases ?  (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(opInfo1.weightsBiases, 0), 32)*1.05): 0,
                        opInfo1.input  ? TENSOR_SIZE_INDEX(opInfo1.input, 0) : 0,
                        opInfo1.input  ? TENSOR_SIZE_INDEX(opInfo1.input, 1) : 0,
                        opInfo1.input  ? TENSOR_SIZE_INDEX(opInfo1.input, 2) : 0,
                        opInfo1.input  ? TENSOR_SIZE_INDEX(opInfo1.input, 3) : 0,
                        opInfo1.input  ? opInfo1.input->tensorBuffer + offsetIn : 0,
                        opInfo1.output ? TENSOR_SIZE_INDEX(opInfo1.output, 0) : 0,
                        opInfo1.output ? TENSOR_SIZE_INDEX(opInfo1.output, 1) : 0,
                        opInfo1.output ? TENSOR_SIZE_INDEX(opInfo1.output, 2) : 0,
                        opInfo1.output ? TENSOR_SIZE_INDEX(opInfo1.output, 3) : 0,
                        opInfo1.output ? opInfo1.output->tensorBuffer+ offsetOut : 0,
                        (opInfo1.input && opInfo1.output)  ?
 (vx_float32)(TENSOR_SIZE_INDEX(opInfo1.input, 2) * TENSOR_STRIDE_INDEX(opInfo1.input, 2) + TENSOR_SIZE_INDEX(opInfo1.output, 2) * TENSOR_STRIDE_INDEX(opInfo1.output, 2)) / graph->base.context->axiSRAM.size * 100 :
 0
                        );
        }
    }
#endif

    if (graph->base.context->options.enableHandleBranch)
    {
        /* colloect the input/output param */
        gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_request_s) * graph->layer->base.num_operations, (gctPOINTER*)&graph->layer->memRequestList);
        if (gcmIS_ERROR(gStatus))
        {
            status = VX_ERROR_NO_MEMORY;
            goto OnError;
        }

        status = GetMemoryRequestList(graph, 0, graph->layer->base.num_operations, graph->layer->memRequestList);
        if (status != VX_SUCCESS) goto OnError;
    }

    gStatus = gcoOS_Allocate(gcvNULL,
                            sizeof(vxnne_block_s) * VX_MAX_BLOCK_COUNT,
                            (gctPOINTER*)&graph->layer->blocks);

    if (gcmIS_ERROR(gStatus))
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    gcoOS_ZeroMemory(graph->layer->blocks, sizeof(vxnne_block_s) * VX_MAX_BLOCK_COUNT);

    for (i = 0; i <= graph->layer->base.num_operations; i++)
    {
        terminator = vx_false_e;

        if (i == graph->layer->base.num_operations ||
            !SupportAB(graph->base.context, graph->layer->operations[i]) ||
            graph->layer->operations[i]->inputsNum == 0 || graph->layer->operations[i]->outputsNum == 0 ||
            graph->layer->operations[i]->batchCount > 1)
        {
            terminator = vx_true_e;
        }
        else
        {
            vxmASSERT(graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_NN
                   || graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_TP
                   || graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_SH);

            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo1);

            if (TENSOR_SIZE_INDEX(opInfo1.input, 3) > 1 || TENSOR_SIZE_INDEX(opInfo1.output, 3) > 1)
            {
                terminator = vx_true_e;
            }
            else if (!graph->base.context->options.enableHandleBranch)
            {
                if (graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_SH)
                {
                    terminator = vx_true_e;
                }
                else if (graph->layer->operations[i]->parentOpNum > 1)
                {
                    terminator = vx_true_e;
                }
                else if (count >= 1)
                {
                    vxmASSERT(graph->layer->operations[i-1]->target == VXNNE_OPERATION_TARGET_NN
                            || graph->layer->operations[i-1]->target == VXNNE_OPERATION_TARGET_TP);

                    vxnneOperation_GetInfo(graph->layer->operations[i - 1], &opInfo2);

                    if (!vxoTensor_IsOverlap(opInfo1.input, opInfo2.output))
                    {
                        terminator = vx_true_e;
                    }
                    else if (graph->layer->operations[i]->childOpNum > 1)
                    {
                        count++;
                        terminator = vx_true_e;
                    }
                }
                else if (graph->layer->operations[i]->childOpNum > 1)
                {
                    terminator = vx_true_e;
                }
            }
            else if (graph->layer->operations[i]->inputsNum > 1 || graph->layer->operations[i]->outputsNum > 1)
            {
                terminator = vx_true_e;
            }
            else if (!opInfo1.output->isVirtual)
            {
                count++;
                terminator = vx_true_e;
            }
        }

        if (terminator)
        {
            if (count > 1)
            {
                status = AnalyzeBlock(graph, start, count);
                if (status != VX_SUCCESS) goto OnError;
            }

            start = i + 1;
            count = 0;
        }
        else
        {
            count++;
        }
    }

    for (i = 0; i < layer->base.num_operations; i++)
    {
        maxOpCommandCount += graph->layer->operations[i]->batchCount;
    }

    for (i = 0; i < layer->blockNum; i++)
    {
        maxOpCommandCount += layer->blocks[i].totalCommandCount -  layer->blocks[i].count;
    }

    gStatus = gcoOS_Allocate(gcvNULL,
                            sizeof(vxnne_operation_command_s) * maxOpCommandCount,
                            (gctPOINTER*)&layer->opIndices);

    if (gcmIS_ERROR(gStatus))
    {
        layer->opIndicesNum = 0;
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    gcoOS_ZeroMemory(layer->opIndices, sizeof(vxnne_operation_command_s) * maxOpCommandCount);

    for(j = 0; j <= layer->blockNum; j++)
    {
        if (j == layer->blockNum)
        {
            e = layer->base.num_operations;
        }
        else
        {
            e = layer->blocks[j].start;
        }

        for (i = s; i < e; i++)
        {
            for (k = 0; k < layer->operations[i]->batchCount; k++)
            {
                layer->opIndices[layer->opIndicesNum].blockFlag = VXNNE_BLOCK_FLAG_NONE;
                layer->opIndices[layer->opIndicesNum].dump = vxOpCommandDump;
                layer->opIndices[layer->opIndicesNum].batchID = k;
                layer->opIndices[layer->opIndicesNum].operationID = i;
                layer->opIndices[layer->opIndicesNum].operation  = layer->operations[i];
                vxnneOperation_InitializeCommand(graph->base.context, layer->operations[i], &layer->opIndices[layer->opIndicesNum]);

                layer->opIndicesNum++;
            }
        }

        if (j == layer->blockNum) break;

        status = InitializeBlock(graph, &layer->blocks[j]);
        if (status != VX_SUCCESS) goto OnError;

        s = layer->blocks[j].start + layer->blocks[j].count;
    }

    gcmASSERT(maxOpCommandCount >= layer->opIndicesNum);

#if SW_TILING_DEBUG
    if (layer->blockNum > 0)
    {
        vxInfo("\n id IN [ x  y  w   h ]   OUT  [ x  y  w  h ] \n");
        for (i = 0; i < layer->opIndicesNum; i++)
        {
            vxInfo(" %3d %s %8x [%3d %3d %3d %3d] -> %s %8x [%3d %3d %3d %3d] \n",
                layer->opIndices[i].operationID,
                layer->opIndices[i].inputTile.sRAM ? "SRAM" : "GDDR",
                layer->opIndices[i].inputTile.physical,
                layer->opIndices[i].inputTile.x,
                layer->opIndices[i].inputTile.y,
                layer->opIndices[i].inputTile.width,
                layer->opIndices[i].inputTile.height,
                layer->opIndices[i].outputTile.sRAM ? "SRAM" : "GDDR",
                layer->opIndices[i].outputTile.physical,
                layer->opIndices[i].outputTile.x,
                layer->opIndices[i].outputTile.y,
                layer->opIndices[i].outputTile.width,
                layer->opIndices[i].outputTile.height);
        }
    }

    vxInfo("---------------------------End VerifyTiling -------------------------\n");
#endif

OnError:
    if (graph->layer->blocks)
    {
        /* free tiling block */
        for (i = 0; i < layer->blockNum; i++)
        {
            for(j = 0; j < layer->blocks[i].segmentNum; j++)
            {
                if (layer->blocks[i].segments[j].segmentInfo.tiling.tilingInfo)
                {
                    gcoOS_Free(gcvNULL, layer->blocks[i].segments[j].segmentInfo.tiling.tilingInfo);
                    layer->blocks[i].segments[j].segmentInfo.tiling.tilingInfo = gcvNULL;
                }
            }

            if (layer->blocks[i].memParam)
            {
                gcoOS_Free(gcvNULL, layer->blocks[i].memParam);
                layer->blocks[i].memParam = VX_NULL;
            }
        }

        gcoOS_Free(gcvNULL, layer->blocks);

        layer->blockNum = 0;
        layer->blocks = VX_NULL;
    }


    if (layer->memRequestList)
    {
        gcoOS_Free(gcvNULL, layer->memRequestList);
        layer->memRequestList = VX_NULL;
    }

    return status;
}

VX_INTERNAL_API vx_uint32 BatchPipelined(vxnne_operation* operations, vx_int32 num)
{
    if (num >= 3 &&
        (operations[0]->target != operations[1]->target) &&
        (operations[1]->target != operations[2]->target) &&
        (operations[0]->target != operations[2]->target) &&
        (operations[0]->target != VXNNE_OPERATION_TARGET_SW) &&
        (operations[1]->target != VXNNE_OPERATION_TARGET_SW) &&
        (operations[2]->target != VXNNE_OPERATION_TARGET_SW))
    {
        return 3;
    }
    else if (num >= 2 &&
        (operations[0]->target != operations[1]->target) &&
        (operations[0]->target != VXNNE_OPERATION_TARGET_SW) &&
        (operations[1]->target != VXNNE_OPERATION_TARGET_SW))
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

VX_PRIVATE_API vx_status vxoMultiGPU_ComputeInputSize(
    vx_enum    opType,
    vx_uint32  outputSize,
    vx_uint32  kernelSize,
    vx_uint32  poolingSize,
    vx_uint32  poolingStride,
    vx_uint32  reshuffStride,
    vx_uint32* inputSize
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 size = 0;

    switch (opType)
    {
        case VXNNE_OPERATOR_CONVOLUTION:
        {
            if (poolingSize != 0)
            {
                outputSize = (outputSize - 1) * poolingStride + poolingSize;
            }
            size = outputSize - 1 + kernelSize;
            break;
        }
        case VXNNE_OPERATOR_POOLING:
        {
            size =  (outputSize - 1) * poolingStride + poolingSize;
            break;
        }
        case VXNNE_OPERATOR_RESHUFFLE:
        {
            size =  outputSize * reshuffStride;
            break;
        }
        case VXNNE_OPERATOR_ACTIVATION:
        case VXNNE_OPERATOR_TENSOR_COPY:
        {
            size = outputSize;
            break;
        }
        default:
            vxmASSERT(0);
            status = VX_FAILURE;
            break;
    }

    if (inputSize != VX_NULL)
    {
        *inputSize = size;
    }

    return status;
}

 VX_PRIVATE_API vx_uint32 vxoMultiGPU_ConvSplitCount(
    vxnne_operation operation,
    vx_uint32 gpuCount
    )
 {
     vx_uint32 inputY = 0, outputY = 0;
     vx_int32 diff = 0;
     vx_uint32 count = gpuCount;
     vx_uint32 splitCount = 0;
     vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;
     vxnne_operation_info_s opInfo;
     vx_status status = VX_SUCCESS;

     vxmASSERT(VXNNE_OPERATOR_CONVOLUTION == operation->operatorType);
     vxmASSERT(gpuCount > 1);

     INITIALIZE_STRUCT(opInfo);
     outputY = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1); /* Y Axis*/
     vxnneOperation_GetInfo(operation, &opInfo);

     while (count >= 2)
     {
         vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                             outputY / count,
                                             opInfo.kernelY,
                                             opInfo.poolSizeY,
                                             opInfo.poolStrideY,
                                             opInfo.reshuffStrideY,
                                             &inputY
                                             ));
         diff = (vx_int32)inputY - (vx_int32)operation->parameter.pad_y_bottom;
         if (((outputY / count) == 0) || (diff <= 1) ||
             (inputY <= (opInfo.kernelY + opInfo.poolSizeY)))
         {
            /* this CONV operation doesn't support split on Y-axis */
            count--;
         }
         else
         {
            splitCount = count;
            break;
         }
     }

 OnError:
    return splitCount;
 }

VX_PRIVATE_API vx_bool vxoMultiGPU_IsSupport(
    vxnne_operation operation,
    vx_uint32 gpuCount,
    vx_uint32 *splitCount
)
{
    vx_status status = VX_SUCCESS;
    vx_bool OpFlag =vx_false_e;
    vx_bool splitFlag = vx_false_e;
    vx_uint32 outputHeight = 0;
    vx_tensor output = VX_NULL;

    *splitCount = 0;
    /*1. support operations */
    if ((VXNNE_OPERATION_TARGET_TP == operation->target) &&
        ((VXNNE_OPERATOR_POOLING == operation->operatorType) ||
        (VXNNE_OPERATOR_RESHUFFLE == operation->operatorType) ||
        (VXNNE_OPERATOR_ACTIVATION == operation->operatorType) ||
        (VXNNE_OPERATOR_TENSOR_COPY == operation->operatorType) ||
        (VXNNE_OPERATOR_FULLYCONNECTED == operation->operatorType)))
    {
        OpFlag = vx_true_e;
    }
    else if ((VXNNE_OPERATION_TARGET_NN == operation->target) &&
        (VXNNE_OPERATOR_CONVOLUTION == operation->operatorType))
    {
         OpFlag = vx_true_e;
    }
    else if ((VXNNE_OPERATION_TARGET_SC == operation->target) &&
        (VXNNE_OPERATOR_YUV2RGB_SCALE == operation->operatorType))
    {
        OpFlag = vx_true_e;
    }
    else
    {
        return vx_false_e;
    }

    /*2. get TP/NP output tensor to determine whether it can be splited */
    if (VXNNE_OPERATION_TARGET_TP == operation->target)
    {
        vxnne_tp_operation srcTpOp = (vxnne_tp_operation)operation;
        output = srcTpOp->output;
    }
    else if ((VXNNE_OPERATION_TARGET_NN == operation->target) &&
        (VXNNE_OPERATOR_CONVOLUTION == operation->operatorType))
    {
        vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;
        output = convOp->outputs;
    }
    else if (VXNNE_OPERATOR_YUV2RGB_SCALE == operation->operatorType)
    {
        vxnne_yuv2rgb_scale_operation scaleOp = (vxnne_yuv2rgb_scale_operation)operation;
        output = scaleOp->outputs;
    }

    /*3. operation can be splited to multi core? */
    if (VXNNE_OPERATOR_FULLYCONNECTED == operation->operatorType)
    {
        vxnne_tp_operation srcTpOp = (vxnne_tp_operation)operation;
        vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)srcTpOp->base.parameter.other_ref;
        vx_uint32 outputDim = 0;
        vx_uint32 count = gpuCount;
        vxmONERROR(vxQueryTensor(output, VX_TENSOR_NUMBER_OF_DIMS, &outputDim, sizeof(outputDim)));
        outputHeight = TENSOR_STRIDE_INDEX(output, 3) / TENSOR_DATA_SIZE(output);

        /* weight/bias tensor is NULL for vData. create weight/bias parameter form stream, the origWeight tensor is NULL */
        if (((weights_biases->wb_base->origWeight == VX_NULL) ||
            ((TENSOR_LOGICAL_ADDR(weights_biases->wb_base->origWeight)) == VX_NULL)) &&
            (weights_biases->mGpuWBCount <= 1))
        {
            splitFlag = vx_false_e;
        }
        else
        {
            while (count >= 2)
            {
                /* TP can't support 1x1x1 output */
                if ((outputHeight / count) <= 1)
                {
                    splitFlag = vx_false_e;
                    count--;
                }
                else
                {
                    splitFlag = vx_true_e;
                    *splitCount = count;
                    break;
                }
            }
        }
    }
    else if (VXNNE_OPERATOR_CONVOLUTION == operation->operatorType)
    {
        vx_uint32 count = vxoMultiGPU_ConvSplitCount(operation, gpuCount);
        if (count > 1)
        {
            splitFlag = vx_true_e;
            *splitCount = count;
        }
    }
    else if (VXNNE_OPERATOR_YUV2RGB_SCALE == operation->operatorType)
    {
        vx_uint32 count = gpuCount;

        outputHeight = TENSOR_VIEW_SIZE_INDEX(output, 1);
         while (count >= 2)
         {
             if ((outputHeight / count) == 0)
             {
                 splitFlag = vx_false_e;
                 count--;
             }
             else
             {
                 splitFlag = vx_true_e;
                 *splitCount = count;
                 break;
             }
         }
    }
    else
    {
        /* split input/output tensor on Y-axis for multiVIP */
        vxnne_operation_info_s opInfo;
        vx_uint32 inputHeight = 0;
        vx_int32 diff = 0;
        vx_uint32 count = gpuCount;

        outputHeight = TENSOR_VIEW_SIZE_INDEX(output, 1);
        INITIALIZE_STRUCT(opInfo);
        vxnneOperation_GetInfo(operation, &opInfo);

        while (count >= 2)
        {
            vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                                outputHeight / count,
                                                opInfo.kernelY,
                                                opInfo.poolSizeY,
                                                opInfo.poolStrideY,
                                                opInfo.reshuffStrideY,
                                                &inputHeight
                                                ));
            diff = (vx_int32)inputHeight - (vx_int32)operation->parameter.pad_y_bottom;
            if (((outputHeight / count) == 0) || (diff <= 1))
            {
                splitFlag = vx_false_e;
                count--;
            }
            else
            {
                splitFlag = vx_true_e;
                *splitCount = count;
                break;
            }
        }
    }

    return OpFlag && splitFlag;

OnError:
    return vx_false_e;
}

VX_PRIVATE_API vx_status vxoMultiGPU_AllocateMemoryForOperations(
    vx_node node,
    vx_uint32 gpuCount
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0, size = 0;
    vx_uint32 NNOpCount = 0, TPOpCount = 0, SCOpCount = 0;
    vx_uint32 splitCount = 0;
    vxnne_operation operation = VX_NULL;

    for (i = 0; i < node->layer->num_operations; i++)
    {
        operation = node->layer->operations[i];
        if (vxoMultiGPU_IsSupport(operation, gpuCount, &splitCount))
        {
            if (VXNNE_OPERATION_TARGET_TP == operation->target)
            {
                TPOpCount += splitCount;
            }
            else if (VXNNE_OPERATION_TARGET_NN == operation->target)
            {
                NNOpCount += splitCount;
            }
            else if (VXNNE_OPERATION_TARGET_SC == operation->target)
            {
                SCOpCount += splitCount;
            }
        }
    }

    /* for splitting operations allocate memory */
    /* these memory will be released in vxoNode_Release function */
    if (TPOpCount > 0)
    {
        size = sizeof(vxnne_tp_operation_s) * TPOpCount;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&node->mGpuTpOperation)))
        {
            vxError("fail to allocate memory for mGpu TPOperation\n");
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        else
        {
            gcoOS_MemFill((gctPOINTER)node->mGpuTpOperation, 0, size);
        }
    }
    else
    {
        node->mGpuTpOperation = VX_NULL;
    }

    if (NNOpCount > 0)
    {
        size = sizeof(vxnne_convolution_relu_pooling_operation_s) * NNOpCount;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&node->mGpuNNOperation)))
        {
            vxError("fail to allocate memory for mGpu NNOperation\n");
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        else
        {
            gcoOS_MemFill((gctPOINTER)node->mGpuNNOperation, 0, size);
        }
    }
    else
    {
        node->mGpuNNOperation = VX_NULL;
    }

    if (SCOpCount > 0)
    {
        size = sizeof(vxnne_yuv2rgb_scale_operation_s) * SCOpCount;
        node->mGpuSCOperation = (vxnne_yuv2rgb_scale_operation)vxAllocate((vx_size)size);
    }
    else
    {
        node->mGpuSCOperation = VX_NULL;
    }

OnError:
    return status;
}

VX_INTERNAL_API vx_status vxoMultiGpu_FreeMemory(vx_node node)
{
    vx_uint32 i = 0, j = 0;
    vx_status status = VX_SUCCESS;
    vxnne_operation operation = VX_NULL;
    vx_reference reference = VX_NULL;

    for (i = 0; i < node->mGpuTpOpCnt; i++)
    {
        operation = &node->mGpuTpOperation[i].base;
        for (j = VX_MULTIVIP_REFERENCE_START; j < VX_MULTIVIP_REFERENCE_END; j++)
        {
            reference = operation->references[j];
            if ((reference != VX_NULL) && (reference->type == VX_TYPE_TENSOR))
            {
                vx_tensor tensor = (vx_tensor)reference;
                status |= vxoTensor_ReleaseTensor(&tensor);
            }
            else if ((reference != VX_NULL) && (reference->type == VX_TYPE_WEIGHTS_BIASES_PARAMETER))
            {
                vx_weights_biases_parameter param = (vx_weights_biases_parameter)reference;
                status |= vxReleaseWeightsBiasesParameter(&param);
            }
        }
    }
    for (i = 0; i < node->mGpuNNOpCnt; i++)
    {
        operation = &node->mGpuNNOperation[i].base;
        for (j = VX_MULTIVIP_REFERENCE_START; j < VX_MULTIVIP_REFERENCE_END; j++)
        {
            reference = operation->references[j];
            if ((reference != VX_NULL) && (reference->type == VX_TYPE_TENSOR))
            {
                vx_tensor tensor = (vx_tensor)reference;
                status |= vxoTensor_ReleaseTensor(&tensor);
            }
            else if ((reference != VX_NULL) && (reference->type == VX_TYPE_WEIGHTS_BIASES_PARAMETER))
            {
                vx_weights_biases_parameter param = (vx_weights_biases_parameter)reference;
                status |= vxReleaseWeightsBiasesParameter(&param);
            }
        }
    }
    for (i = 0; i < node->mGpuSCOpCnt; i++)
    {
        operation = &node->mGpuSCOperation[i].base;
        for (j = VX_MULTIVIP_REFERENCE_START; j < VX_MULTIVIP_REFERENCE_END; j++)
        {
            reference = operation->references[j];
            if ((reference != VX_NULL) && (reference->type == VX_TYPE_TENSOR))
            {
                vx_tensor tensor = (vx_tensor)reference;
                status |= vxoTensor_ReleaseTensor(&tensor);
            }
        }
    }

    if (node->mGpuNNOperation != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)(node->mGpuNNOperation)));
        node->mGpuNNOperation = VX_NULL;
        node->mGpuNNOpCnt = 0;
    }
    if (node->mGpuTpOperation != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)(node->mGpuTpOperation)));
        node->mGpuTpOperation = VX_NULL;
        node->mGpuTpOpCnt = 0;
    }
    if (node->mGpuSCOperation != VX_NULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)(node->mGpuSCOperation)));
        node->mGpuSCOperation = VX_NULL;
        node->mGpuSCOpCnt = 0;
    }

    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_CopyOperationBase(
    vxnne_operation srcOperation,
    vxnne_operation dstOperation
    )
{
    vx_uint32 i = 0;

    dstOperation->layer = srcOperation->layer;
    dstOperation->id = srcOperation->id;
    dstOperation->target = srcOperation->target;
    dstOperation->operatorType = srcOperation->operatorType;
    dstOperation->execute = srcOperation->execute;
    dstOperation->dump = srcOperation->dump;
    dstOperation->initialize = srcOperation->initialize;
    dstOperation->deinitialize = srcOperation->deinitialize;
    dstOperation->generateCommands = srcOperation->generateCommands;
    dstOperation->caculateDimSize = srcOperation->caculateDimSize;
    dstOperation->waitMode = srcOperation->waitMode;
    dstOperation->wakeMode = srcOperation->wakeMode;
    dstOperation->semaWaitHandle = srcOperation->semaWaitHandle;
    dstOperation->semaWakeHandle = srcOperation->semaWakeHandle;
    dstOperation->perCmdSize = srcOperation->perCmdSize;
    dstOperation->refNum = srcOperation->refNum;
    dstOperation->inputsNum = srcOperation->inputsNum;
    dstOperation->outputsNum = srcOperation->outputsNum;
    dstOperation->genericNum = srcOperation->genericNum;
    dstOperation->onetimeRefsNum = srcOperation->onetimeRefsNum;
    dstOperation->currBatchIndex = srcOperation->currBatchIndex;
    dstOperation->batchCount = srcOperation->batchCount;
    vxMemCopy(&dstOperation->parameter, &srcOperation->parameter, sizeof(vx_op_param_s));
    dstOperation->parentOpNum = srcOperation->parentOpNum;
    dstOperation->parentLayerNum = srcOperation->parentLayerNum;
    dstOperation->childOpNum = srcOperation->childOpNum;
    dstOperation->childLayerNum = srcOperation->childLayerNum;
    dstOperation->segIndex = srcOperation->segIndex;
    dstOperation->gpuId = srcOperation->gpuId;

    dstOperation->inputs        = &dstOperation->references[0];
    dstOperation->outputs       = &dstOperation->references[VX_MAX_OPERTAION_INPUTS_OUTPUTS];

    for (i = 0; i < srcOperation->genericNum; i++)
    {
        dstOperation->generics[i] = srcOperation->generics[i];
    }
    for (i = 0; i < VX_MAX_OPERTAION_GENERICS; i++)
    {
        dstOperation->onetimeRefs[i] = srcOperation->onetimeRefs[i];
    }
    dstOperation->parameter.other_ref = srcOperation->parameter.other_ref;
    dstOperation->parameter.data_buff = srcOperation->parameter.data_buff;
    for (i = 0; i < MAX_PARENT_CHILD_OP_NUM; i++)
    {
        dstOperation->parentOps[i] = srcOperation->parentOps[i];
        dstOperation->childOps[i] = srcOperation->childOps[i];
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoMultiGPU_SplitOperation(
    vx_node node,
    vxnne_operation srcOperation
    )
{
    vx_status status = VX_SUCCESS;

    if (VXNNE_OPERATION_TARGET_TP == srcOperation->target)
    {
        vxnne_tp_operation dstTpOP = &node->mGpuTpOperation[node->mGpuTpOpCnt];
        vxnne_tp_operation srcTpOp = (vxnne_tp_operation)srcOperation;
        if (VX_NULL == dstTpOP)
        {
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        vxoMultiGPU_CopyOperationBase(srcOperation, &dstTpOP->base);

        dstTpOP->input = VX_NULL;
        dstTpOP->input_ex = srcTpOp->input_ex;
        dstTpOP->weights_biases = srcTpOp->weights_biases;
        dstTpOP->output = VX_NULL;
        dstTpOP->buffer = srcTpOp->buffer;
        dstTpOP->separate_value = srcTpOp->separate_value;
        dstTpOP->tp_value = srcTpOp->tp_value;
    }
    else if ((VXNNE_OPERATION_TARGET_NN == srcOperation->target) &&
        (VXNNE_OPERATOR_CONVOLUTION == srcOperation->operatorType))
    {
        vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)srcOperation;
        vxnne_convolution_relu_pooling_operation dstNNOP = &node->mGpuNNOperation[node->mGpuNNOpCnt];
        if (VX_NULL == dstNNOP)
        {
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }

        vxoMultiGPU_CopyOperationBase(srcOperation, &dstNNOP->base);

        dstNNOP->inputs = VX_NULL;
        dstNNOP->orig_inputs = convOp->orig_inputs;
        dstNNOP->reshape_inputs = convOp->reshape_inputs;
        dstNNOP->reshape_outputs = convOp->reshape_outputs;
        dstNNOP->weights_biases = convOp->weights_biases;
        dstNNOP->reshape_weights_biases = convOp->reshape_weights_biases;
        dstNNOP->dilationX = convOp->dilationX;
        dstNNOP->dilationY = convOp->dilationY;
        dstNNOP->pad_x_left = convOp->pad_x_left;
        dstNNOP->pad_x_right = convOp->pad_x_right;
        dstNNOP->pad_y_top = convOp->pad_y_top;
        dstNNOP->pad_y_bottom = convOp->pad_y_bottom;
        dstNNOP->conv_rounding_type = convOp->conv_rounding_type;
        dstNNOP->enable_relu = convOp->enable_relu;
        dstNNOP->enable_pooling = convOp->enable_pooling;
        dstNNOP->pool_type = convOp->pool_type;
        dstNNOP->pool_size_x = convOp->pool_size_x;
        dstNNOP->pool_size_y = convOp->pool_size_y;
        dstNNOP->padMode = convOp->padMode;
        dstNNOP->padConst = convOp->padConst;
        dstNNOP->down_scale_size_rounding = convOp->down_scale_size_rounding;
        dstNNOP->outputs = VX_NULL;
        dstNNOP->sub_wb = convOp->sub_wb;
    }
    else if (VXNNE_OPERATOR_YUV2RGB_SCALE == srcOperation->operatorType)
    {
        vxnne_yuv2rgb_scale_operation scaleOp = (vxnne_yuv2rgb_scale_operation)srcOperation;
        vxnne_yuv2rgb_scale_operation dstSCOP = &node->mGpuSCOperation[node->mGpuSCOpCnt];
        if (VX_NULL == dstSCOP)
        {
            vxmASSERT(0);
            vxmONERROR(VX_ERROR_NO_MEMORY);
        }
        vxoMultiGPU_CopyOperationBase(srcOperation, &dstSCOP->base);

        dstSCOP->inputs = scaleOp->inputs;
        dstSCOP->r_mean = scaleOp->r_mean;
        dstSCOP->g_mean = scaleOp->g_mean;
        dstSCOP->b_mean = scaleOp->b_mean;
        dstSCOP->rgb_scale = scaleOp->rgb_scale;
        dstSCOP->y_only = scaleOp->y_only;
        vxMemCopy(&dstSCOP->rect, &scaleOp->rect, sizeof(vx_rectangle_t));
        dstSCOP->x_scale = scaleOp->x_scale;
        dstSCOP->y_scale = scaleOp->y_scale;
        dstSCOP->x_init_error = scaleOp->x_init_error;
        dstSCOP->y_init_error = scaleOp->y_init_error;
        dstSCOP->outputs = VX_NULL;
    }
    else
    {
        vxmASSERT(0);
        vxError("mutliple GPU can't support this operation type: %d\n", srcOperation->target);
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_SplitInputOutput(
    vx_node node,
    vxnne_operation dstOperation,
    vxnne_operation srcOperation,
    vx_tensor *input,
    vx_tensor *output,
    vx_uint32 splitCount,
    vx_uint32 gpuIndex
    )
{
    vxnne_operation_info_s opInfo;
    vx_tensor_view inputView = VX_NULL, outputView = 0;
    vx_tensor inputTensor = VX_NULL, outputTensor = VX_NULL;
    vx_tensor origInputTensor = VX_NULL, origOutputTensor = VX_NULL;
    vx_uint32 outputHeightPreOp = 0, outputResidue = 0;
    vx_uint32 inputHeight = 0, outputHeight = 0;
    vx_uint32 inputHeightStart = 0, inputHeightEnd = 0;
    vx_uint32 outputHeightStart = 0, outputHeightEnd = 0;
    vx_uint32 origInputHeight = 0, origOutputHeight = 0;
    vx_uint32 inputSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0}, outputSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 inputSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0}, outputSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 offsetConv = 0, offsetPool = 0, offsetResuffle = 0;
    vx_uint32 inputDim = 0, outputDim = 0;
    vx_uint32 pad = 0;
    vx_status status = VX_SUCCESS;

    INITIALIZE_STRUCT(opInfo);

    if (VXNNE_OPERATION_TARGET_TP == srcOperation->target)
    {
        vxnne_tp_operation srcTpOp = (vxnne_tp_operation)srcOperation;
        origInputTensor = srcTpOp->input;
        origOutputTensor = srcTpOp->output;
    }
    else if ((VXNNE_OPERATION_TARGET_NN == srcOperation->target) &&
        (VXNNE_OPERATOR_CONVOLUTION == srcOperation->operatorType))
    {
        vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)srcOperation;
        origInputTensor = convOp->inputs;
        origOutputTensor = convOp->outputs;
    }

    origInputHeight = TENSOR_VIEW_SIZE_INDEX(origInputTensor, 1);
    origOutputHeight = TENSOR_VIEW_SIZE_INDEX(origOutputTensor, 1);

    vxnneOperation_GetInfo(srcOperation, &opInfo);
    opInfo.pad.bottom = srcOperation->parameter.pad_y_bottom;
    opInfo.pad.right =  srcOperation->parameter.pad_x_right;

    vxmASSERT((srcOperation->outputsNum == 1) && (origOutputHeight > 0) && (splitCount > 0));

    vxmONERROR(vxQueryTensor(origInputTensor, VX_TENSOR_DIMS, inputSizeEnd, sizeof(inputSizeEnd)));
    vxmONERROR(vxQueryTensor(origOutputTensor, VX_TENSOR_DIMS, outputSizeEnd, sizeof(outputSizeEnd)));
    vxmONERROR(vxQueryTensor(origInputTensor, VX_TENSOR_NUMBER_OF_DIMS, &inputDim, sizeof(inputDim)));
    vxmONERROR(vxQueryTensor(origOutputTensor, VX_TENSOR_NUMBER_OF_DIMS, &outputDim, sizeof(outputDim)));

    outputHeightPreOp = origOutputHeight / splitCount;
    outputResidue = origOutputHeight % splitCount;
    if (outputResidue > gpuIndex)
    {
        outputHeight = outputHeightPreOp + 1;
        outputHeightStart = gpuIndex * outputHeight;
    }
    else
    {
        outputHeight = outputHeightPreOp;
        outputHeightStart = gpuIndex * outputHeightPreOp + outputResidue;
    }

    outputHeightEnd =  ((outputHeightStart + outputHeight) > origOutputHeight) ?
                        origOutputHeight : (outputHeightStart + outputHeight);
    vxmASSERT(outputHeight != 0);

    /* split output y-axis */
    outputSizeStart[1] = outputHeightStart;
    outputSizeEnd[1] = outputHeightEnd;

    if (gpuIndex == 0)
    {
        vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                            outputHeight,
                                            opInfo.kernelY,
                                            opInfo.poolSizeY,
                                            opInfo.poolStrideY,
                                            opInfo.reshuffStrideY,
                                            &inputHeight
                                            ));
        inputHeightStart = 0;
        inputHeightEnd = inputHeight - opInfo.pad.top;

        dstOperation->parameter.pad_y_bottom = 0;
        if (srcOperation->operatorType == VXNNE_OPERATOR_CONVOLUTION)
        {
            vxnne_convolution_relu_pooling_operation dstOp = (vxnne_convolution_relu_pooling_operation)dstOperation;
            dstOp->pad_y_bottom = 0;
        }
    }
    else if (gpuIndex == (splitCount - 1))
    {
        vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                            outputHeightStart + 1,
                                            opInfo.kernelY,
                                            opInfo.poolSizeY,
                                            opInfo.poolStrideY,
                                            opInfo.reshuffStrideY,
                                            &inputHeight
                                            ));
        /* 1. -offsetConv for getting the position of (outputHeightStart + 1) CONV's begin index
           2. -offsetPool for getting the position of (outputHeightStart + 1) POOL begin index
           3. -offsetResuffle for resuffle operation
           4. -1 for creating tensorView index */
        offsetConv = opInfo.kernelY ? (opInfo.kernelY - 1) : 0;
        offsetPool = opInfo.poolSizeY ? (opInfo.poolSizeY - 1) : 0;
        offsetResuffle = opInfo.reshuffStrideY ? (opInfo.reshuffStrideY - 1) : 0;
        inputHeightStart = inputHeight - opInfo.pad.top - offsetConv - offsetPool - offsetResuffle - 1;

        vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                            outputHeightEnd,
                                            opInfo.kernelY,
                                            opInfo.poolSizeY,
                                            opInfo.poolStrideY,
                                            opInfo.reshuffStrideY,
                                            &inputHeight
                                            ));
        inputHeightEnd = inputHeight - opInfo.pad.top - opInfo.pad.bottom + 1;
        if (inputHeightEnd > origInputHeight)
            inputHeightEnd = origInputHeight;

        dstOperation->parameter.pad_y_top = 0;
        if (srcOperation->operatorType == VXNNE_OPERATOR_CONVOLUTION)
        {
            vxnne_convolution_relu_pooling_operation dstOp = (vxnne_convolution_relu_pooling_operation)dstOperation;
            dstOp->pad_y_top = 0;
        }
    }
    else
    {
        vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                            outputHeightStart + 1,
                                            opInfo.kernelY,
                                            opInfo.poolSizeY,
                                            opInfo.poolStrideY,
                                            opInfo.reshuffStrideY,
                                            &inputHeight
                                            ));
        /* 1. -offsetConv for getting the position of (outputHeightStart + 1) CONV's begin index
           2. -offsetPool for getting the position of (outputHeightStart + 1) POOL begin index
           3. -offsetResuffle for resuffle operation
           4. -1 for creating tensorView index */
        offsetConv = opInfo.kernelY ? (opInfo.kernelY - 1) : 0;
        offsetPool = opInfo.poolSizeY ? (opInfo.poolSizeY - 1) : 0;
        offsetResuffle = opInfo.reshuffStrideY ? (opInfo.reshuffStrideY - 1) : 0;
        inputHeightStart = inputHeight - opInfo.pad.top - offsetConv - offsetPool - offsetResuffle - 1;

        vxmONERROR(vxoMultiGPU_ComputeInputSize(opInfo.opType,
                                            outputHeight,
                                            opInfo.kernelY,
                                            opInfo.poolSizeY,
                                            opInfo.poolStrideY,
                                            opInfo.reshuffStrideY,
                                            &inputHeight
                                            ));
        inputHeightEnd = inputHeightStart + inputHeight;
        if (inputHeightEnd > origInputHeight)
        {
            pad = inputHeightEnd - origInputHeight;
            inputHeightEnd = origInputHeight;
        }
        else
        {
            pad = 0;
        }

        dstOperation->parameter.pad_y_bottom = pad;
        dstOperation->parameter.pad_y_top = 0;
        if (srcOperation->operatorType == VXNNE_OPERATOR_CONVOLUTION)
        {
            vxnne_convolution_relu_pooling_operation dstOp = (vxnne_convolution_relu_pooling_operation)dstOperation;
            dstOp->pad_y_top = 0;
            dstOp->pad_y_bottom = pad;
        }
    }

    /* split input y-axis*/
    inputSizeStart[1] = inputHeightStart;
    inputSizeEnd[1] = inputHeightEnd;
    vxmASSERT((inputHeightEnd - inputHeightStart) != 0);

    inputView = vxCreateTensorView(node->base.context, inputSizeStart, inputSizeEnd, (vx_uint8)inputDim);
    inputTensor  = vxoTensor_CreateTensorFromView(origInputTensor, inputView);
    if (inputView != VX_NULL) vxReleaseTensorView(&inputView);
    dstOperation->references[VX_MULTIVIP_INPUT_TENSOR_REFERENCE] = (vx_reference)inputTensor;

    outputView = vxCreateTensorView(node->base.context, outputSizeStart, outputSizeEnd, (vx_uint8)outputDim);
    outputTensor  = vxoTensor_CreateTensorFromView(origOutputTensor, outputView);
    if (outputView != VX_NULL) vxReleaseTensorView(&outputView);
    dstOperation->references[VX_MULTIVIP_OUTPUT_TENSOR_REFERENCE] = (vx_reference)outputTensor;

    dstOperation->inputs[0] = (vx_reference)inputTensor;
    dstOperation->outputs[0] = (vx_reference)outputTensor;
    input[0] = inputTensor;
    output[0] = outputTensor;

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_SplitResourceForFC(
    vx_node node,
    vxnne_tp_operation dstOperation,
    vxnne_operation srcOperation,
    vx_uint32 splitCount,
    vx_uint32 gpuIndex
    )
{
    vx_status status = VX_SUCCESS;
    vx_tensor input = (vx_tensor)srcOperation->inputs[0];
    vx_tensor output = (vx_tensor)srcOperation->outputs[0];
    vxnne_tp_operation srcTpOp = (vxnne_tp_operation)srcOperation;
    vx_tensor_view outputView = VX_NULL, weightView = VX_NULL, biasView = VX_NULL;
    vx_tensor outputTensor = VX_NULL, weightTensor = VX_NULL, biasTensor = VX_NULL;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)srcTpOp->base.parameter.other_ref;
    vx_uint32 kzGroup = srcTpOp->tp_value->u32[0];
    vx_weights_biases_parameter newWeightBias = VX_NULL;
    vx_weights_biases_parameter_optimizations_t optimizations;
    vx_uint32 outputDim = 0, inputDim = 0, weightDim = 0, biasDim = 0;
    vx_uint32 outputSize = 0, newOutputSize = 0, outputResidue = 0;
    vx_uint32 outputPreOp = 0, outputStart = 0, outputEnd = 0;
    vx_uint32 outputSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 outputSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 inputSize[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 outputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 weightSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 weightSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 biasSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 biasSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_tensor weight = VX_NULL, bias = VX_NULL;
    vx_uint32 outputSplitAxis = 0, weightSplitAxis = 0, biasSplitAxis = 0;
    vx_uint32 i = 0;

    vx_nn_convolution_relu_pooling_params_ext2_t params = {
            {
                    { 0, 0, 0, 0, 0, 0, 0,
                      VX_CONVERT_POLICY_SATURATE,
                      VX_ROUND_POLICY_TO_ZERO,
                      VX_NN_DS_SIZE_ROUNDING_FLOOR,
                      vx_false_e,
                      0, 0, 0, VX_PAD_CONSTANT, 0
                    },
                    1, 1
            },
            0,
            VX_TENSOR_RANK_WHCN,
            0
    };

    vxmONERROR(vxQueryTensor(input, VX_TENSOR_DIMS, inputSize, sizeof(inputSize)));
    vxmONERROR(vxQueryTensor(input, VX_TENSOR_NUMBER_OF_DIMS, &inputDim, sizeof(inputDim)));

    /*1. split output tensor */
    vxmONERROR(vxQueryTensor(output, VX_TENSOR_DIMS, outputSizeEnd, sizeof(outputSizeEnd)));
    vxmONERROR(vxQueryTensor(output, VX_TENSOR_NUMBER_OF_DIMS, &outputDim, sizeof(outputDim)));
    if (1 != TENSOR_VIEW_SIZE_INDEX(output, 0))
    {
        outputSplitAxis = 0;
    }
    else if (1 != TENSOR_VIEW_SIZE_INDEX(output, 1))
    {
        outputSplitAxis = 1;
    }
    else if (1 != TENSOR_VIEW_SIZE_INDEX(output, 2))
    {
        outputSplitAxis = 2;
    }
    else
    {
        vxmASSERT(0);
    }
    outputSize = TENSOR_VIEW_SIZE_INDEX(output, outputSplitAxis);
    vxmASSERT(outputSize > splitCount);

    outputPreOp = outputSize / splitCount;
    outputResidue = outputSize % splitCount;
    if (outputResidue > gpuIndex)
    {
        newOutputSize = outputPreOp + 1;
        outputStart = gpuIndex * newOutputSize;
    }
    else
    {
        newOutputSize = outputPreOp;
        outputStart = gpuIndex * outputPreOp + outputResidue;
    }

    outputEnd =  ((outputStart + newOutputSize) > outputSize) ? outputSize : (outputStart + newOutputSize);
    outputSizeStart[outputSplitAxis] = outputStart;
    outputSizeEnd[outputSplitAxis] = outputEnd;
    vxmASSERT((outputEnd - outputStart) != 0);
    outputView = vxCreateTensorView(node->base.context, outputSizeStart, outputSizeEnd, (vx_uint8)outputDim);
    outputTensor  = vxoTensor_CreateTensorFromView((vx_tensor)srcOperation->outputs[0], outputView);
    if (outputView != VX_NULL) vxReleaseTensorView(&outputView);
    dstOperation->base.references[VX_MULTIVIP_OUTPUT_TENSOR_REFERENCE] = (vx_reference)outputTensor;

    if (weights_biases->mGpuWBCount > 0)
    {
        /*2. for vData weight/bias */
        newWeightBias = weights_biases->mGpuWBTable[gpuIndex];
        vxmASSERT(newWeightBias != VX_NULL);
    }
    else
    {
        /*2. split weight tensor*/
        weight = weights_biases->wb_base->origWeight;
        bias   = weights_biases->wb_base->origBias;
        vxmONERROR(vxQueryTensor(weight, VX_TENSOR_DIMS, weightSizeEnd, sizeof(weightSizeEnd)));
        vxmONERROR(vxQueryTensor(weight, VX_TENSOR_NUMBER_OF_DIMS, &weightDim, sizeof(weightDim)));
        weightSplitAxis = outputSplitAxis + 1;
        if (TENSOR_VIEW_SIZE_INDEX(weight, weightSplitAxis) != TENSOR_VIEW_SIZE_INDEX(output, outputSplitAxis))
        {
            for (i = 0 ; i < weightDim; i++)
            {
                if (TENSOR_VIEW_SIZE_INDEX(weight, weightSplitAxis) == TENSOR_VIEW_SIZE_INDEX(output, outputSplitAxis))
                {
                    weightSplitAxis = i;
                }
            }
        }
        vxmASSERT(TENSOR_VIEW_SIZE_INDEX(weight, weightSplitAxis) == TENSOR_VIEW_SIZE_INDEX(output, outputSplitAxis));
        weightSizeStart[weightSplitAxis] = outputStart;
        weightSizeEnd[weightSplitAxis] = outputEnd;
        weightView = vxCreateTensorView(node->base.context, weightSizeStart, weightSizeEnd, (vx_uint8)weightDim);
        weightTensor  = vxoTensor_CreateTensorFromView(weight, weightView);
        if (weightView != VX_NULL) vxReleaseTensorView(&weightView);
        dstOperation->base.references[VX_MULTIVIP_WEIGHT_TENSOR_REFERENCE] = (vx_reference)weightTensor;

        /*3. split bias tensor*/
        if (1 != TENSOR_VIEW_SIZE_INDEX(bias, 0))
        {
            biasSplitAxis = 0;
        }
        else if (1 != TENSOR_VIEW_SIZE_INDEX(bias, 1))
        {
            biasSplitAxis = 1;
        }
        else if (1 != TENSOR_VIEW_SIZE_INDEX(bias, 2))
        {
            biasSplitAxis = 2;
        }
        else if (1 != TENSOR_VIEW_SIZE_INDEX(bias, 3))
        {
            biasSplitAxis = 3;
        }
        else
        {
            vxmASSERT(0);
        }
        vxmASSERT(TENSOR_VIEW_SIZE_INDEX(bias, biasSplitAxis) == TENSOR_VIEW_SIZE_INDEX(output, outputSplitAxis));
        vxmONERROR(vxQueryTensor(bias, VX_TENSOR_DIMS, biasSizeEnd, sizeof(biasSizeEnd)));
        vxmONERROR(vxQueryTensor(bias, VX_TENSOR_NUMBER_OF_DIMS, &biasDim, sizeof(biasDim)));
        biasSizeStart[biasSplitAxis] = outputStart;
        biasSizeEnd[biasSplitAxis] = outputEnd;
        biasView = vxCreateTensorView(node->base.context, biasSizeStart, biasSizeEnd, (vx_uint8)biasDim);
        biasTensor  = vxoTensor_CreateTensorFromView(bias, biasView);
        if (biasView != VX_NULL) vxReleaseTensorView(&biasView);
        dstOperation->base.references[VX_MULTIVIP_BIAS_TENSOR_REFERENCE] = (vx_reference)biasTensor;

        /*4. create new vx weights biases parameter */
        params.ext.base.pad_x_left = weights_biases->wb_base->pad_x_left;
        params.ext.base.pad_x_right = weights_biases->wb_base->pad_x_right;
        params.ext.base.pad_y_top = weights_biases->wb_base->pad_y_top;
        params.ext.base.pad_y_bottom = weights_biases->wb_base->pad_y_bottom;
        params.ext.base.down_scale_size_rounding = weights_biases->wb_base->down_scale_size_rounding;
        params.ext.stride_x = weights_biases->wb_base->strideX;
        params.ext.stride_y = weights_biases->wb_base->strideY;
        params.convert_dst_format = TENSOR_DATA_TYPE(outputTensor);
        optimizations.inputZeroPoint = weights_biases->wb_base->inputZP;
        optimizations.zrl = weights_biases->wb_base->setZeroLength;
        optimizations.outputFormat = TENSOR_DATA_TYPE(outputTensor);

        vxmONERROR(vxQueryTensor(outputTensor, VX_TENSOR_DIMS, outputDims, sizeof(outputDims)));
        newWeightBias = vxCreateWeightsBiasesParameterFromTensors2(VX_NN_FULLYCONNECTED_LAYER,
                                                                    weightDim,
                                                                    inputSize,
                                                                    outputDims,
                                                                    outputDims,
                                                                    TENSOR_DATA_TYPE(outputTensor),
                                                                    (vx_nn_convolution_relu_pooling_params)&params,
                                                                    sizeof(vx_nn_convolution_relu_pooling_params_ext2_t),
                                                                    (vx_weights_biases_parameter_optimizations_t*)&optimizations,
                                                                    weightTensor,
                                                                    biasTensor);
        /* save newWeightBias to mGpuWBTable[] for supporting vData */
        if (gpuIndex == (splitCount - 1))
        {
            weights_biases->mGpuWBCount = splitCount;
        }
        weights_biases->mGpuWBTable[gpuIndex] = newWeightBias;
    }
    dstOperation->base.references[VX_MULTIVIP_WEIGHT_BIAS_PARAM_REFERENCE] = (vx_reference)newWeightBias;

    /*5. construction new TP FullyConnect operation */
    dstOperation->output = outputTensor;
    dstOperation->base.outputs[0] = (vx_reference)outputTensor;
    dstOperation->input = srcTpOp->input;
    dstOperation->base.inputs[0] = srcOperation->inputs[0];
    dstOperation->weights_biases = newWeightBias;
    dstOperation->base.parameter.other_ref = (vx_reference)newWeightBias;

    if (1 == kzGroup)
    {
        vx_tp_value_cmd_s values ;
        vx_uint32 kzgroup = newWeightBias->weights_sizes[2] / newWeightBias->slice_array[0].kz_count;
        vx_uint32 zoffset = 0;
        vxmASSERT(1 == kzGroup);
        memset(&values,0,sizeof(vx_tp_value_cmd_s));
        dstOperation->tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(newWeightBias->slice_num * sizeof(vx_tp_value_cmd_s));
        if (dstOperation->tp_value != VX_NULL)
        {
            for (i = 0; i < newWeightBias->slice_num; i++)
            {
                values.u32[0] = kzgroup;
                values.u32[1] = zoffset;
                vxMemCopy(&dstOperation->tp_value[i], &values, sizeof(vx_tp_value_cmd_s));
                zoffset += newWeightBias->slice_array[i].z_count;
            }
        }
    }
    else
    {
        vx_tp_value_cmd_s values ;
        vx_uint32 kzoffset = 0, kzoffset2 = 0, zoffset = 0;
        memset(&values,0,sizeof(vx_tp_value_cmd_s));
        if (0 == srcTpOp->tp_value->e32[0])
        {
            dstOperation->tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(newWeightBias->slice_num * sizeof(vx_tp_value_cmd_s));
            if (dstOperation->tp_value != VX_NULL)
            {
                vx_uint32 kzgroup = newWeightBias->weights_sizes[2] / newWeightBias->slice_array[0].kz_count;
                for (i = 0; i < newWeightBias->slice_num; i++)
                {
                    values.e32[0] = 0;
                    values.u32[0] = kzgroup;
                    values.u32[1] = zoffset;
                    values.u32[2] = kzoffset;
                    values.u32[3] = kzoffset2;

                    vxMemCopy(&dstOperation->tp_value[i], &values, sizeof(vx_tp_value_cmd_s));
                    if (i % kzgroup == kzgroup - 1)
                    {
                        kzoffset = kzoffset2 = 0;
                        zoffset += newWeightBias->slice_array[i].z_count;
                    }
                    else
                    {
                        kzoffset += newWeightBias->slice_array[i].kz_count;
                        kzoffset2 += newWeightBias->weights_sizes[3];
                    }
                }
            }
        }
        else if (1 == srcTpOp->tp_value->e32[0])
        {
             dstOperation->tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
             if (dstOperation->tp_value != VX_NULL)
             {
                values.e32[0] = 1;
                values.u32[1] = newWeightBias->weights_sizes[3];
                vxMemCopy(dstOperation->tp_value, &values, sizeof(vx_tp_value_cmd_s));
             }
        }
        else
        {
            vxmASSERT(0);
        }
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_SplitResourceForScaler(
    vx_node node,
    vxnne_yuv2rgb_scale_operation dstOperation,
    vxnne_operation srcOperation,
    vx_uint32 splitCount,
    vx_uint32 *outputStart,
    vx_uint32 *outputSize,
    vx_uint32 *inputStart,
    vx_uint32 *inputSize,
    vx_uint32 *inputInitError,
    vx_uint32 gpuIndex
    )
{
    vxnne_operation_info_s opInfo;
    vx_tensor_view outputView = 0;
    vx_tensor origOutputTensor = VX_NULL, outputTensor = VX_NULL;
    vx_status status = VX_SUCCESS;
    vx_uint32 outputSizeEnd[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 outputSizeStart[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint32 outputDim = 0;
    vxnne_yuv2rgb_scale_operation scaleOp = (vxnne_yuv2rgb_scale_operation)srcOperation;

    INITIALIZE_STRUCT(opInfo);

    origOutputTensor = scaleOp->outputs;

    vxmONERROR(vxQueryTensor(origOutputTensor, VX_TENSOR_DIMS, outputSizeEnd, sizeof(outputSizeEnd)));
    vxmONERROR(vxQueryTensor(origOutputTensor, VX_TENSOR_NUMBER_OF_DIMS, &outputDim, sizeof(outputDim)));

    /* split output y-axis */
    outputSizeStart[1] = outputStart[gpuIndex];
    outputSizeEnd[1] = outputStart[gpuIndex] + outputSize[gpuIndex];

    outputView = vxCreateTensorView(node->base.context, outputSizeStart, outputSizeEnd, (vx_uint8)outputDim);
    outputTensor  = vxoTensor_CreateTensorFromView(origOutputTensor, outputView);
    if (outputView != VX_NULL) vxReleaseTensorView(&outputView);
    dstOperation->base.references[VX_MULTIVIP_OUTPUT_TENSOR_REFERENCE] = (vx_reference)outputTensor;

    dstOperation->outputs = outputTensor;
    dstOperation->base.outputs[0] = (vx_reference)outputTensor;
    dstOperation->base.inputs[0] = srcOperation->inputs[0];

    /* split rectangle */
    dstOperation->rect.start_y += inputStart[gpuIndex];
    dstOperation->rect.end_y = dstOperation->rect.start_y + inputSize[gpuIndex];
    dstOperation->y_init_error = inputInitError[gpuIndex];

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_Handle(
    vxnne_execution_layer layer,
    vx_node node,
    vxnne_operation operation,
    vx_uint32 splitCount
    )
{
    vx_status status = VX_SUCCESS;
    gctUINT32 gpuIndex = 0;
    vxmASSERT(layer != VX_NULL);
    vxmASSERT(node != VX_NULL);
    vxmASSERT(operation != VX_NULL);

    if (VXNNE_OPERATION_TARGET_TP == operation->target)
    {
        for (gpuIndex = 0; gpuIndex < splitCount; gpuIndex++)
        {
            vxnne_tp_operation tpOperation = VX_NULL;
            vxmONERROR(vxoMultiGPU_SplitOperation(node, operation));

            tpOperation = &node->mGpuTpOperation[node->mGpuTpOpCnt];
            if ((VXNNE_OPERATOR_FULLYCONNECTED == operation->operatorType))
            {
                vxmONERROR(vxoMultiGPU_SplitResourceForFC(node, tpOperation,
                                                           operation,
                                                           splitCount, gpuIndex));
            }
            else
            {
                vxmONERROR(vxoMultiGPU_SplitInputOutput(node, &tpOperation->base,
                                                         operation,
                                                         &tpOperation->input,
                                                         &tpOperation->output,
                                                         splitCount, gpuIndex));
            }
            layer->operations[layer->base.num_operations] = &tpOperation->base;
            layer->operations[layer->base.num_operations]->gpuId = gpuIndex;
            if (gpuIndex == splitCount - 1)
            {
                layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
            }
            else
            {
                layer->operations[layer->base.num_operations]->mGpuSync = vx_false_e;
            }
            layer->base.num_operations++;
            node->mGpuTpOpCnt++;
        }
    }
    else if (VXNNE_OPERATION_TARGET_NN == operation->target)
    {
        for (gpuIndex = 0; gpuIndex < splitCount; gpuIndex++)
        {
            vxmONERROR(vxoMultiGPU_SplitOperation(node, operation));
            if (VXNNE_OPERATOR_CONVOLUTION == operation->operatorType)
            {
                vxnne_convolution_relu_pooling_operation nnOperation =  &node->mGpuNNOperation[node->mGpuNNOpCnt];

                vxmONERROR(vxoMultiGPU_SplitInputOutput(node, &nnOperation->base,
                                                     operation,
                                                     &nnOperation->inputs,
                                                     &nnOperation->outputs,
                                                     splitCount, gpuIndex));
                layer->operations[layer->base.num_operations] = &nnOperation->base;
                layer->operations[layer->base.num_operations]->gpuId = gpuIndex;
                if (gpuIndex == splitCount - 1)
                {
                    layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
                }
                else
                {
                    layer->operations[layer->base.num_operations]->mGpuSync = vx_false_e;
                }
                layer->base.num_operations++;
                node->mGpuNNOpCnt++;
            }
            else
            {
                vxmASSERT(0);
            }
        }
    }
    else if (VXNNE_OPERATION_TARGET_SC == operation->target)
    {
        vxnne_yuv2rgb_scale_operation scaleOp = (vxnne_yuv2rgb_scale_operation)operation;
        vx_rectangle_t rect = scaleOp->rect;
        vx_uint32 imageInputHeight = rect.end_y - rect.start_y;
        vx_uint32 outputHeight = TENSOR_VIEW_SIZE_INDEX(scaleOp->outputs, 1); /* y-axis*/
        vx_uint32 scale = scaleOp->y_scale;
        vx_uint32 inputStarts[gcdMAX_3DGPU_COUNT], inputSizes[gcdMAX_3DGPU_COUNT], inputInitErrors[gcdMAX_3DGPU_COUNT];
        vx_uint32 outputStarts[gcdMAX_3DGPU_COUNT], outputHeights[gcdMAX_3DGPU_COUNT];

        vxmASSERT(VXNNE_OPERATOR_YUV2RGB_SCALE == operation->operatorType);

        vxmONERROR(vxnneComputeYUV2RGBInputParameter(outputHeight, scale, &splitCount,
                                                     outputStarts, outputHeights,
                                                     inputStarts, inputSizes, inputInitErrors));
        if (splitCount == 1)
        {
            vxmONERROR(VX_FAILURE);
        }

        if (imageInputHeight < (inputStarts[splitCount - 1] + inputSizes[splitCount - 1]))
        {
            vxError("ERROR: invalid input image size.\n");
            vxmASSERT(0);
        }

        for (gpuIndex = 0; gpuIndex < splitCount; gpuIndex++)
        {
            vxnne_yuv2rgb_scale_operation SCOperation = VX_NULL;

            vxmONERROR(vxoMultiGPU_SplitOperation(node, operation));
            SCOperation = &node->mGpuSCOperation[node->mGpuSCOpCnt];

            vxmONERROR(vxoMultiGPU_SplitResourceForScaler(node, SCOperation,
                                                          operation, splitCount,
                                                          outputStarts, outputHeights,
                                                          inputStarts, inputSizes,
                                                          inputInitErrors, gpuIndex));

            layer->operations[layer->base.num_operations] = &SCOperation->base;
            layer->operations[layer->base.num_operations]->gpuId = gpuIndex;
            if (gpuIndex == splitCount - 1)
            {
                layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
            }
            else
            {
                layer->operations[layer->base.num_operations]->mGpuSync = vx_false_e;
            }
            layer->base.num_operations++;
            node->mGpuSCOpCnt++;
        }
    }
    else
    {
        vxError("multiGPU can't support this target: %d\n", operation->target);
        vxmASSERT(0);
    }

    return status;

OnError:
    /* use the original operation if fail to split */
    layer->operations[layer->base.num_operations] = operation;
    layer->operations[layer->base.num_operations]->gpuId = 0;
    layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
    layer->base.num_operations++;
    return status;
}

VX_INTERNAL_API void vxoGraph_GenerateOperationTable(vx_graph graph)
{
    vxnne_execution_layer layer = VX_NULL;
    vx_uint32 i, j, operationCount = 0;
    gctUINT32 gpuCount = 1;
    vx_uint32 enableMultiVIPCombined = graph->base.context->options.enableMultiVIPCombined;
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;

    if ((graph->nodeCount == 0) || !graph->nodeTable[graph->allNodeIndexTable[0]]->layer)
    {
        return;
    }

    if (graph->layer)
    {
        vxnneLayer_Free(&graph->layer->base);
        graph->layer = VX_NULL;
    }

    /* TODO: use accurate batchCount. */
    vxmONERROR(vxnneExecutionLayer_Create(graph, &layer));

    gcmONERROR(gcoVX_GetHWConfigGpuCount(&gpuCount));

    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
        vx_uint32 splitCount = 0;

        if (!node->layer)
        {
            vxmONERROR(VX_ERROR_INVALID_NODE);
        }

        for (j = 0; j < node->layer->num_operations; j++)
        {
            if ((gpuCount > 1) && enableMultiVIPCombined &&
                (vxoMultiGPU_IsSupport(node->layer->operations[j], gpuCount, &splitCount)))
            {
                operationCount += splitCount;
            }
            else
            {
                operationCount++;
            }
        }
    }

    gStatus = gcoOS_Allocate(gcvNULL,
                             sizeof(vxnne_operation) * operationCount,
                            (gctPOINTER*)&layer->operations);
    if (gcmIS_ERROR(gStatus))
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    gcoOS_ZeroMemory(layer->operations, sizeof(vxnne_operation) * operationCount);

    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
        vx_uint32 splitCount = 0;

        if ((gpuCount > 1) && enableMultiVIPCombined)
        {
            vxmONERROR(vxoMultiGPU_AllocateMemoryForOperations(node, gpuCount));
        }

        for (j = 0; j < node->layer->num_operations; j++)
        {
            if ((gpuCount > 1) && enableMultiVIPCombined &&
                (vxoMultiGPU_IsSupport(node->layer->operations[j], gpuCount, &splitCount)))
            {
                vxoMultiGPU_Handle(layer, node, node->layer->operations[j], splitCount);
            }
            else
            {
                layer->operations[layer->base.num_operations] = node->layer->operations[j];
                layer->operations[layer->base.num_operations]->gpuId = 0;
                layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
                layer->base.num_operations++;
            }
        }
    }

    vxmASSERT(layer->base.num_operations <= operationCount);
    graph->layer = layer;

    return;

OnError:
    if (layer)
    {
        vxnneLayer_Free(&layer->base);

        if (layer->operations)
        {
            gcoOS_Free(gcvNULL, layer->operations);
        }
    }
}

/*******************************************Predict arch performance***************************************************/
#define DISABLE_TP_RESHUFFLE_SPLIT 0
#define MAX_COST 2147483647
struct _archModelOpInfo
{
    /* fixed */
    vx_node     node;
    vxnne_operation opt;
    vx_enum     op;
    vx_enum     target;
    vx_uint32   inx;
    vx_uint32   iny;
    vx_uint32   inz;
    vx_uint32   origx;
    vx_uint32   origy;
    vx_uint32   origoutx;
    vx_uint32   origouty;
    vx_uint32   stridex;
    vx_uint32   stridey;
    vx_uint32   kx;
    vx_uint32   ky;
    vx_uint32   kz;
    vx_uint32   bfy;
    vx_uint32   bfz;
    vx_uint32   oz;
    vx_uint32   siz;
    vx_uint32   psize;
    vx_uint32   pstride;
    vx_uint32   xpad;
    vx_uint32   ypad;
    vx_uint32   dsize;
    vx_uint8    fcmd;
    vx_enum     input_data_format;
    vx_uint32   nnCores;
    vx_weights_biases_parameter weight_bias;
    vx_arch_perf_s perf;
    vx_uint32   xsize;
    vx_uint32   ysize;
    /* tmp */
    vx_uint32   pix;
    vx_uint32   piy;
    vx_uint32   p3;
    vx_uint32   psix;
    vx_uint32   psiy;
    /* calculate */
    vx_uint8    sbuf;
    vx_uint8    dbuf;
    vx_uint8    kbuf;
    vx_int32    swTilingType;/*-1: none, 1: AB, 0: Sub-IMAGE*/
    vx_uint32   upStreamLayerCount;
    vx_uint32   downStreamLayerCount;
    vx_int32    upStreamLayer[20];
    vx_int32    downStreamLayer[20];
};

struct _archModelCost
{
    vx_float64 cycle;
    vx_float64 bw;
};

struct _archModelSplitInfo
{
    struct _archModelCost *savedSegmentCost;
    vx_uint32   **savedSIX;
    vx_uint32   **savedSIY;
    vx_uint32   **savedSIZ;
    struct _archModelCost   **savedCost;
    vx_uint8    **split_array;
    vx_int32    *bestCostSWTilingType;
};

struct _archModelInfo
{
    struct _archModelOpInfo **opInfoArray;
    struct _archModelSplitInfo **splitInfoArray;
    vx_uint32 totalOpCount;
};

#define CYCLE_WEIGHT 20
#define BW_WEIGHT 1
vx_bool _cur_cost_is_more_better(struct _archModelCost *cost, struct _archModelCost *cur, vx_uint32 cycle_weight, vx_uint32 bw_weight)
{
    vx_float64 f;
    f = -(1.0f * (cur->cycle - cost->cycle) / gcmMAX(cur->cycle, cost->cycle) * cycle_weight + 1.0f * (cur->bw - cost->bw) / gcmMAX(cur->bw, cost->bw) * bw_weight);
    if (f > 0) return vx_true_e;
    return vx_false_e;
}

void deInitArchModelSplitInfo(struct _archModelSplitInfo * splitInfo, vx_uint32 operationCount)
{
    vx_uint32 i;
    if (splitInfo->savedSIX)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIX[i] != NULL) vxFree(splitInfo->savedSIX[i]);
        }
        vxFree(splitInfo->savedSIX);
    }
    if (splitInfo->savedSIY)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIY[i] != NULL) vxFree(splitInfo->savedSIY[i]);
        }
        vxFree(splitInfo->savedSIY);
    }
    if (splitInfo->savedSIZ)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIZ[i] != NULL) vxFree(splitInfo->savedSIZ[i]);
        }
        vxFree(splitInfo->savedSIZ);
    }

    if (splitInfo->savedCost)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedCost[i] != NULL) vxFree(splitInfo->savedCost[i]);
        }
        vxFree(splitInfo->savedCost);
    }

    if (splitInfo->savedSegmentCost) vxFree(splitInfo->savedSegmentCost);
    if (splitInfo->bestCostSWTilingType) vxFree(splitInfo->bestCostSWTilingType);
    if (splitInfo->split_array)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->split_array[i] != NULL) vxFree(splitInfo->split_array[i]);
        }
        vxFree(splitInfo->split_array);
    }
    if (splitInfo) vxFree(splitInfo);
}

struct _archModelSplitInfo * initArchModelSplitInfo(vx_uint32 operationCount)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i;
    struct _archModelSplitInfo *splitInfo;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelSplitInfo),
        (gctPOINTER *)&splitInfo
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo, 0, gcmSIZEOF(struct _archModelSplitInfo));

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIX
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIX, 0, sizeof(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIX[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIX[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIY
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIY, 0, gcmSIZEOF(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIY[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIY[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIZ
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIZ, 0, sizeof(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIZ[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIZ[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelCost *) * operationCount,
        (gctPOINTER *)&splitInfo->savedCost
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedCost, 0, sizeof(struct _archModelCost *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelCost) * operationCount,
            (gctPOINTER *)&splitInfo->savedCost[i]
            );
        if (gcmIS_ERROR(status)) goto error;
        memset(splitInfo->savedCost[i], 0, gcmSIZEOF(struct _archModelCost) * operationCount);
    }

    status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelCost) * operationCount,
            (gctPOINTER *)&splitInfo->savedSegmentCost
            );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSegmentCost, 0, gcmSIZEOF(struct _archModelCost) * operationCount);

    status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_bool) * operationCount,
            (gctPOINTER *)&splitInfo->bestCostSWTilingType
            );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->bestCostSWTilingType, 0, gcmSIZEOF(vx_bool) * operationCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint8 *) * operationCount,
        (gctPOINTER *)&splitInfo->split_array
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->split_array, 0, gcmSIZEOF(vx_uint8 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint8) * operationCount,
            (gctPOINTER *)&splitInfo->split_array[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->split_array[i], 0, sizeof(vx_uint8) * operationCount);
    }

    return splitInfo;

error:
    if (splitInfo) deInitArchModelSplitInfo(splitInfo, operationCount);
    vxInfo("ERROR: initArchModelSplitInfo() return out-of-memory\n");
    return NULL;
}

void deInitArchModelInfo(struct _archModelInfo *archModel, vx_uint32 operationCount)
{
    vx_uint32 i;
    for (i = 0; i < operationCount; i++)
    {
        if (archModel->opInfoArray && archModel->opInfoArray[i] != NULL)
        {
            vxFree(archModel->opInfoArray[i]);
        }
        if (archModel->splitInfoArray && archModel->splitInfoArray[i] != NULL)
        {
            deInitArchModelSplitInfo(archModel->splitInfoArray[i], operationCount);
        }
    }

    if (archModel->opInfoArray != NULL) vxFree(archModel->opInfoArray);
    if (archModel->splitInfoArray != NULL) vxFree(archModel->splitInfoArray);

    if (archModel != NULL) vxFree(archModel);
}

struct _archModelInfo * initArchModelInfo(vx_uint32 operationCount)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i;
    struct _archModelInfo *archModel;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelInfo),
        (gctPOINTER *)&archModel
        );
    if (gcmIS_ERROR(status)) goto error;
    archModel->totalOpCount = operationCount;

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelOpInfo *) * operationCount,
        (gctPOINTER *)&archModel->opInfoArray
        );
    if (gcmIS_ERROR(status)) goto error;

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelSplitInfo *) * operationCount,
        (gctPOINTER *)&archModel->splitInfoArray
        );
    if (gcmIS_ERROR(status)) goto error;

    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelOpInfo),
            (gctPOINTER *)&archModel->opInfoArray[i]
            );
        if (gcmIS_ERROR(status)) goto error;
        memset(archModel->opInfoArray[i], 0, gcmSIZEOF(struct _archModelOpInfo));

        archModel->splitInfoArray[i]= initArchModelSplitInfo(operationCount);
        if (archModel->splitInfoArray[i] == NULL) goto error;
    }

    return archModel;

error:
    if (archModel != NULL) {
        deInitArchModelInfo(archModel, operationCount);
    }
    vxInfo("ERROR: initArchModelInfo() return out-of-memory\n");
    return NULL;
}

static void initSegmentCostResult(struct _archModelInfo * archModel, vx_uint32 count)
{
    vx_uint32 i, j;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < count; j++)
        {
            archModel->splitInfoArray[i]->savedSegmentCost[j].bw = -1;
            archModel->splitInfoArray[i]->savedSegmentCost[j].cycle = -1;
        }
    }
}

static void getSegmentCostResult(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    struct _archModelCost *cost)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    cost->cycle = archModel->splitInfoArray[segment_first]->savedSegmentCost[segment_last].cycle;
    cost->bw = archModel->splitInfoArray[segment_first]->savedSegmentCost[segment_last].bw;
}

static void setSegmentCostResult(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    struct _archModelCost *cost)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first]->savedSegmentCost[segment_last].cycle = cost->cycle;
    archModel->splitInfoArray[segment_first]->savedSegmentCost[segment_last].bw = cost->bw;
}

static vx_int32 getBestCostSWTilingTypeInfo(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    return archModel->splitInfoArray[segment_first]->bestCostSWTilingType[segment_last];
}

static void setBestCostSWTilingTypeInfo(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 bestCostSWTilingType)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first]->bestCostSWTilingType[segment_last] = bestCostSWTilingType;
}
static void setSplitArrayInfo(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 pos,
    vx_int8 split)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first]->split_array[segment_last][pos] = split;
}


static void saveCalculationArgs(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 pos,
    struct _archModelCost *cost,
    vx_uint32 six,
    vx_uint32 siy,
    vx_uint32 siz,
    vx_uint8 split)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first]->savedSIX[segment_last][pos] = six;
    archModel->splitInfoArray[segment_first]->savedSIY[segment_last][pos] = siy;
    archModel->splitInfoArray[segment_first]->savedSIZ[segment_last][pos] = siz;
    archModel->splitInfoArray[segment_first]->split_array[segment_last][pos] = split;
    if (cost != NULL)
    {
         archModel->splitInfoArray[segment_first]->savedCost[segment_last][pos].cycle = cost->cycle;
         archModel->splitInfoArray[segment_first]->savedCost[segment_last][pos].bw = cost->bw;
    }
}

static vx_float64 _calc_cost(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 index,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint8 src_buf,
    vx_uint8 dst_buf,
    vx_uint8 kenerl_buf,
    vx_int32 cache_space_in_kb)
{
    vx_status status;
    vx_arch_perf perf = &archModel->opInfoArray[index]->perf;
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];

    gcmASSERT(index >= 0);
    perf->calculated = vx_false_e;
    perf->info.kx   = opInfo->kx;
    perf->info.ky   = opInfo->ky;
    perf->info.kz   = opInfo->kz;
    perf->info.inputDataFormat = opInfo->input_data_format;

    /*save original input x/y/z*/
    perf->info.oinx = opInfo->inx;
    perf->info.oiny = opInfo->iny;
    perf->info.oinz = opInfo->inz;
    perf->info.inx = x;
    perf->info.iny = y;
    perf->info.inz = opInfo->kz;
    perf->info.outx = x;
    perf->info.outy = y;
    perf->info.outz = z;
    perf->info.stridex = opInfo->stridex ? opInfo->stridex : 1;
    perf->info.stridex = opInfo->stridey ? opInfo->stridey : 1;
    perf->info.poolingSize   = opInfo->psize;
    perf->info.poolingStride = opInfo->pstride;
    perf->info.xOffSet = (-1) * opInfo->xpad;
    perf->info.yOffSet = (-1) * opInfo->ypad;
    perf->info.dataSize = opInfo->dsize;
    perf->info.nnCores = opInfo->nnCores;
    if (opInfo->target == VXNNE_OPERATION_TARGET_TP && opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        perf->info.nnCores += context->nnConfig.fixedFeature.tpliteCoreCount;
    }
    perf->swTilingInfo.origInX = opInfo->origx;
    perf->swTilingInfo.origInY = opInfo->origy;
    perf->swTilingInfo.origOutX = opInfo->origoutx;
    perf->swTilingInfo.origOutY = opInfo->origouty;
    perf->swTilingInfo.origOutZ = opInfo->oz;
    perf->swTilingInfo.srcBuf    = src_buf;
    perf->swTilingInfo.dstBuf    = dst_buf;
    perf->swTilingInfo.kernelBuf = kenerl_buf;
    perf->swTilingInfo.calcNonFirstCmd = opInfo->fcmd ? vx_true_e : vx_false_e;
    perf->swTilingInfo.cacheSpaceInKB = cache_space_in_kb;
    perf->info.pix = opInfo->pix;
    perf->info.piy = opInfo->piy;
    perf->info.p3 = opInfo->p3;
    perf->info.nextKY = ((vx_uint32)(index + 1) < archModel->totalOpCount) ? archModel->opInfoArray[index + 1]->ky : 0;
    if ((opInfo->target != VXNNE_OPERATION_TARGET_TP || opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED) && opInfo->weight_bias)
        perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(opInfo->weight_bias, 0), CACHE_ALIGNMENT_SIZE));
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1))
    {
        vx_uint32 repeatePerFrame = 1;
        vx_uint32 cnum = (vx_uint32)(ceilf((vx_float32)opInfo->xsize / x)
                            * ceilf((vx_float32)opInfo->ysize / y)
                            * ceilf((vx_float32)opInfo->oz / z) * repeatePerFrame);

        memset(&perf->resultInfo, 0, sizeof(vx_performance_info_s));
        status = calculateArchPerf(context,
            opInfo->opt->layer,
            VX_NULL,
            perf,
            opInfo->weight_bias,
            opInfo->target,
            (opInfo->op == VXNNE_OPERATOR_ROIPOOL
             && opInfo->opt->parameter.tpType == TP_ROI_POOLING_STEP_1) ? VXNNE_OPERATOR_POOLING :
            opInfo->op);
        if (status != VX_SUCCESS)
        {
            return (vx_float64)MAX_COST;
        }
        if (perf->opTarget == VXNNE_OPERATION_TARGET_NN)
        {
            perf->resultInfo.perfCycleCount     += (cnum - 1) * perf->swTilingInfo.perfNonFirstCycleCount;
            perf->resultInfo.perfReadBandWidth  += (cnum - 1) * perf->swTilingInfo.perfNonFirstReadBandWidth;
            perf->resultInfo.perfWriteBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstWriteBandWidth;
            perf->resultInfo.perfAXIReadBandWidth  += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIReadBandWidth;
            perf->resultInfo.perfAXIWriteBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;
        }
        else
        {
            perf->resultInfo.perfCycleCount     = cnum * perf->resultInfo.perfCycleCount;
            perf->resultInfo.perfReadBandWidth  = cnum * perf->resultInfo.perfReadBandWidth;
            perf->resultInfo.perfWriteBandWidth = cnum * perf->resultInfo.perfWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth += cnum * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += cnum * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;
        }
    }
    else
    {
        memset(&perf->resultInfo, 0, sizeof(vx_performance_info_s));
        status = calculateArchPerf(context,
            opInfo->node->layer,
            opInfo->opt,
            perf,
            opInfo->weight_bias,
            opInfo->target,
            (opInfo->op == VXNNE_OPERATOR_ROIPOOL
            && opInfo->opt->parameter.tpType == TP_ROI_POOLING_STEP_1) ? VXNNE_OPERATOR_POOLING :
            opInfo->op);
        if (status != VX_SUCCESS)
        {
            return (vx_float64)MAX_COST;
        }
    }
    return perf->resultInfo.perfCycleCount;
}

static vx_uint32 _kernel_size_in_pixel(struct _archModelInfo *archModel, vx_int32 index, vx_uint32 cores, vx_bool full_chache_kernel_head_fix)
{
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];
    vx_float64 coefCompressionRatio = 0;
    if (opInfo->weight_bias != NULL)
    {
        coefCompressionRatio = WB_COMPRESS_RATIO(opInfo->weight_bias);
    }

    if (opInfo->target != VXNNE_OPERATION_TARGET_TP || opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        if (full_chache_kernel_head_fix)
        {
            return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * opInfo->oz
                      * coefCompressionRatio * 1.05f + 0.5f);
        }
        else
        {
           return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * ceilf((vx_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * 1.05f + 0.5f);
        }
    }
    return 0;
}

static vx_uint32 _outbuf_needed_ex(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 psixArray[],
    vx_uint32 psiyArray[],
    vx_uint32 psizArray[])
{
    vx_int32 outBufNeeded = 0, i;

    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    for (i = segment_first; i < segment_last; i++)
    {
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP || (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED)) {
            if (archModel->opInfoArray[i + 1]->kx == 1 && archModel->opInfoArray[i + 1]->ky == 1)
            {
                outBufNeeded += archModel->opInfoArray[i]->bfy
                    * ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                    * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i])
                    * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
            }
            else
            {
                outBufNeeded += ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                    * gcmMIN((gcmMAX(
                        archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i]),
                        archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i + 1]->piy : psiyArray[i + 1])
                        * archModel->opInfoArray[i + 1]->pstride)
                        + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1),
                        archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->piy)
                    * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            }
        }
        else
        {
            outBufNeeded += ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                * gcmMIN((gcmMAX(
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i]),
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i + 1]->piy : psiyArray[i + 1])
                    * archModel->opInfoArray[i + 1]->pstride)
                    + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1),
                    archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->piy)
                * (archModel->opInfoArray[i]->bfz * ((psizArray == NULL) ? archModel->opInfoArray[i]->siz : psizArray[i]) + 1 /*tpkz*/ - 1);
        }
    }
    return outBufNeeded;
}
static vx_int32 _calc_gcd(vx_int32 a, vx_int32 b)
{
    vx_int32 t;

    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}

static vx_int32 _calc_lcm(vx_int32 a, vx_int32 b)
{
    vx_int32 x = _calc_gcd(a, b);
    return (vx_int32)(a * b / x);
}

vx_uint32 _calc_full_cached_space_needed(struct _archModelInfo *archModel, vx_uint32 segment_index, vx_uint32 psixArray[], vx_uint32 psiyArray[], vx_uint32 max_tile_size)
{
    if (archModel->opInfoArray[segment_index]->target != VXNNE_OPERATION_TARGET_TP
       || (archModel->opInfoArray[segment_index]->opt->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
       )
    {
        return (gcmMIN(psixArray[segment_index] * archModel->opInfoArray[segment_index]->pstride, max_tile_size) + archModel->opInfoArray[segment_index]->kx - 1)
               * (psiyArray[segment_index] * archModel->opInfoArray[segment_index]->pstride + archModel->opInfoArray[segment_index]->ky - 1) * archModel->opInfoArray[segment_index]->kz;
    }
    return 0;
}

static vx_bool _calc_y_subimage(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 sram_left,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint32 max_tile_size)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 *sixArray, *siyArray;
    vx_int32 i, termA = 0, termB = 0;
    vx_int32 m, lcm = 1;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_y_subimage() return out-of-memory\n");
        return vx_false_e;
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_y_subimage() return out-of-memory\n");
        goto OnError;
    }

    for (i = segment_first; i <= segment_last - 1; i++)
    {
        vx_uint32 termAIn, termBIn;
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
        {
            termAIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            termBIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
        }
        else
        {
            termAIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
            termBIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
        }

        termA += termAIn;
        termB += termBIn;
    }

    /* Calculate largest M that fits */
    if (archModel->opInfoArray[segment_first]->target != VXNNE_OPERATION_TARGET_TP ||
        (archModel->opInfoArray[segment_first]->op == VXNNE_OPERATOR_FULLYCONNECTED))
    {
        vx_int32 termC = (gcmMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);
        vx_int32 termD = (gcmMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);
        termC = sram_left - (termB + termC * (archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz);
        termD = termA + termD * archModel->opInfoArray[segment_first]->pstride * archModel->opInfoArray[segment_first]->kz;
        m = (vx_int32)(termC / termD);
    }
    else
    {
        m = (vx_int32)((sram_left - termB) / termA);
    }
#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("M=%d\n", m);
#endif

    if (m > 0 && !(m % lcm))
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            sixArray[i] = archModel->opInfoArray[i]->xsize;
            siyArray[i] = gcmMIN((vx_uint32)m, archModel->opInfoArray[i]->piy);
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            vx_int32 outbufNeeded;
            siyArray[i] = gcmMIN((vx_uint32)(m * 2), archModel->opInfoArray[i]->piy);
            outbufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array)
                + _calc_full_cached_space_needed(archModel, segment_first, sixArray, siyArray, max_tile_size);
            if (sram_left < outbufNeeded)
            {
                siyArray[i] = gcmMIN((vx_int32)m, (vx_int32)archModel->opInfoArray[i]->piy); /* put M back */
                break;
            }
        }
    }
    /* generate SIY */
    for (i = segment_first; i <= segment_last; i++)
    {
        x_array[i] = archModel->opInfoArray[i]->xsize;
        if (m > 0)
        {
            y_array[i] = gcmMIN(siyArray[i] * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3, archModel->opInfoArray[i]->ysize);
        }
        else
        {
            y_array[i] = 0;
        }
    }

    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return vx_true_e;

OnError:
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return vx_false_e;
}


static vx_bool _calc_x_subimage(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 sram_left,
    vx_uint32 ppsiy,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint32 max_tile_size)
{
    /* This routine compute the SubImageYSize assuming SubImageXSize = ImageXSize */
    vx_int32 termC = 0, termD = 0, termE = 0, termF = 0;
    vx_uint32 sumProdPSKX = archModel->opInfoArray[segment_last]->p3 + archModel->opInfoArray[segment_last]->kx - 1, prevProdPS = 0, prevSumProdPSKX = 0;
    vx_uint32 prodPoolStride = archModel->opInfoArray[segment_last]->pstride;
    vx_int32 i, n, /*ds = archModel->opInfoArray[segment_first]->dsize / 8, lcm = 1,*/ psix = 0;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    for (i = (vx_int32)segment_last - 1; i >= (vx_int32)segment_first; i--)
    {
        vx_int32 termCIn, termDIn;
        /*vx_uint32 d = archModel->opInfoArray[i]->target == archModel->opInfoArray[i - 1]->target ? 1 : 2;*/
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
        {
            termCIn = prodPoolStride * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;

            termDIn = sumProdPSKX * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;

        }
        else
        {
            termCIn = prodPoolStride * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);

            termDIn = sumProdPSKX * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);

        }

        termC += termCIn;
        termD += termDIn;
        prevProdPS = prodPoolStride;
        prevSumProdPSKX = sumProdPSKX;
        prodPoolStride = prodPoolStride * archModel->opInfoArray[i]->pstride;
        sumProdPSKX = sumProdPSKX * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3 + archModel->opInfoArray[i]->kx - 1 + 2 * (1 - 1);
    }

    termE = prodPoolStride * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz + termC;
    termF = sumProdPSKX * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz + termD;

    /* Calculate largest M that fits */
    if (archModel->opInfoArray[segment_first]->target != VXNNE_OPERATION_TARGET_TP ||
        (archModel->opInfoArray[segment_first]->op == VXNNE_OPERATOR_FULLYCONNECTED))
    {
        n = gcmMAX((vx_int32)((sram_left - termF)/termE),
            (vx_int32)((sram_left - (vx_int32)(termD + (64 + archModel->opInfoArray[segment_first]->kx - 1) * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz)) / termC));

    }
    else
    {
        n = (vx_int32)((sram_left - termD)/termC);
    }

    psix = n * prevProdPS + prevSumProdPSKX;
#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("N=%d, PSIX=%d, segment_first=%d\n", n, psix, segment_first);
#endif
    x_array[segment_first] = gcmMIN(psix * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3, archModel->opInfoArray[segment_first]->xsize);
    x_array[segment_first] = (vx_uint32)(x_array[segment_first] / 64) * 64;
    y_array[segment_first] = ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3;
    for (i = (vx_int32)(segment_first + 1); i <= (vx_int32)segment_last; i++)
    {
        psix = (vx_uint32)ceilf((vx_float32)(x_array[i-1] - archModel->opInfoArray[i - 1]->p3) / archModel->opInfoArray[i - 1]->pstride);
        if (n > 0)
        {
            x_array[i] = gcmMIN(psix - (archModel->opInfoArray[i]->kx - 1), archModel->opInfoArray[i]->xsize);
        }
        else
        {
            x_array[i] = 0;
        }

        y_array[i]  = ppsiy * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3;
    }

    return vx_true_e;
}


/* return sram space left size*/
static vx_int32 _calc_ab_buffer(
    struct _archModelInfo *archModel,
    vx_int32 segment_first, vx_int32 segment_last,
    vx_int32 sram_space,
    vx_uint32 x_array[], vx_uint32 y_array[])
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 abBufSize[2] = {0, 0}, outBufNeeded = 0, abBufPairSize = 0;
    vx_uint32 *sixArray, *siyArray;
    vx_int32 i, leftSramSize;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_ab_buffer() return out-of-memory\n");
        return 0;
    }
    gcmASSERT(sixArray != NULL);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_ab_buffer() return out-of-memory\n");
        goto OnError;
    }

    for (i = segment_first; i <= segment_last; i++)
    {
        sixArray[i] = (vx_uint32)(ceilf((vx_float32)(archModel->opInfoArray[i]->xsize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (vx_uint32)(ceilf((vx_float32)(archModel->opInfoArray[i]->ysize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
    }

    for (i = segment_first; i <= segment_last - 1; i++)
    {
        outBufNeeded = _outbuf_needed_ex(archModel, i, i + 1, sixArray, siyArray, NULL);
        abBufSize[i % 2] = outBufNeeded;
        abBufPairSize = gcmMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
    }
    leftSramSize = (vx_int32)(sram_space - abBufPairSize);
    if (leftSramSize < 0)
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            x_array[i] = 0;
            y_array[i] = 0;
        }
    }
    else
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            x_array[i] = archModel->opInfoArray[i]->xsize;
            y_array[i] = archModel->opInfoArray[i]->ysize;
        }
    }

    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return leftSramSize;

OnError:
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return 0;
}
static void _subimage_segment_cost(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_int32 *best_cost_swtiling_type,
    struct _archModelCost *cost)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_int32 i, j;
    struct _archModelCost bestCost = {MAX_COST, MAX_COST};
    struct _archModelCost cur_cost = {MAX_COST, MAX_COST};
    vx_uint32 *xArray = NULL, *yArray = NULL, *zArray = NULL;
    vx_int32 kernelBufNeeded = 0, sramLeft/*, ds = archModel->opInfoArray[segment_first]->dsize / 8*/;
    vx_int32 vipSramSpaceSize = context->nnConfig.customizedFeature.vipSRAMSizeInKB * 1024 - VX_VIP_SRAM_IMAGE_STREAM_SIZE;
    vx_int32 axiSramSpaceSize = context->nnConfig.customizedFeature.axiSRAMSizeInKB * 1024;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
    vx_int32 sramSpaceSize = axiSramOnlySWTiling ? (vx_int32)axiSramSpaceSize : (vx_int32)vipSramSpaceSize;
    vx_int32 abBufferSpaceLeft = 0;
    vx_bool fullCacheKernelHeadFix = context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&xArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(1) return out-of-memory\n");
        return;
    }
    memset(xArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&yArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(2) return out-of-memory\n");
        goto exit;
    }
    memset(yArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&zArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(3) return out-of-memory\n");
        goto exit;
    }
    memset(zArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    if (segment_first == segment_last)
    {
        x_array[segment_first] = archModel->opInfoArray[segment_first]->xsize;
        y_array[segment_first] = archModel->opInfoArray[segment_first]->ysize;
        z_array[segment_first] = archModel->opInfoArray[segment_first]->oz;
        archModel->opInfoArray[segment_first]->perf.info.flush = 1;
        cost->cycle = _calc_cost(
            context,
            archModel,
            segment_first,
            x_array[segment_first],
            y_array[segment_first],
            z_array[segment_first],
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
            (vx_int32)(vipSramSpaceSize / 1024));
        cost->bw = archModel->opInfoArray[segment_first]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[segment_first]->perf.resultInfo.perfWriteBandWidth;
        goto exit;
    }
    else
    {
        vx_bool hasUnsupportedTPLayer = vx_false_e;
        vx_uint32 tpCircularBuf = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE2) ? 1 : 0;
        for (i = segment_first; i <= segment_last; i++)
        {
            zArray[i] = archModel->opInfoArray[i]->oz;
            if (i == segment_last)
            {
                if (i > 0 && archModel->opInfoArray[i]->target == VXNNE_OPERATION_TARGET_TP)
                {
                    zArray[i] = zArray[i - 1] * archModel->opInfoArray[i]->oz / archModel->opInfoArray[i]->kz;
                }
            }
            else {
                if ((i + 1 <= segment_last)
                    && (archModel->opInfoArray[i + 1]->target == VXNNE_OPERATION_TARGET_TP)
                    && (archModel->opInfoArray[i + 1]->ky == 1)
                    )
                {
                    if (archModel->opInfoArray[i]->target != VXNNE_OPERATION_TARGET_TP)
                    {
                        if (context->nnConfig.derivedFeature.nnXYDPX == 0)
                        {
                            zArray[i] = (vx_uint32)gcmMIN(archModel->opInfoArray[i]->oz,
                                archModel->opInfoArray[i]->nnCores
                                * ceilf((vx_float32)(context->nnConfig.unifiedFeature.maxTileSize + 8)
                                / (vx_float32)(WB_NON_ZERO_RATIO(archModel->opInfoArray[i]->weight_bias) * context->nnConfig.fixedFeature.equivalentVipsramWidthInByte)));
                        }
                        else
                        {
                            zArray[i] = (vx_uint32)gcmMIN(archModel->opInfoArray[i]->oz,
                                archModel->opInfoArray[i]->nnCores
                                * ceilf((vx_float32)(context->nnConfig.unifiedFeature.maxTileSize + 8)
                                / (ceilf((vx_float32)archModel->opInfoArray[i]->kx / context->nnConfig.derivedFeature.nnXYDPX)
                                  * ceilf((vx_float32)archModel->opInfoArray[i]->ky / context->nnConfig.derivedFeature.nnXYDPY)
                                  * (vx_float32)WB_NON_ZERO_RATIO(archModel->opInfoArray[i]->weight_bias)
                                  * context->nnConfig.fixedFeature.equivalentVipsramWidthInByte)));
                        }
                    }
                    else
                    {
                        zArray[i] = gcmMIN(archModel->opInfoArray[i]->oz, context->nnConfig.fixedFeature.tpCoreCount * 1 /*SIZSmallUnit(layer)*/);
                        if (i > 0 && (i != segment_first))
                        {
                            zArray[i] = _calc_lcm(zArray[i], zArray[i - 1] * archModel->opInfoArray[i]->oz / archModel->opInfoArray[i]->kz);
                        }
                    }
                }
            }
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            kernelBufNeeded += _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, fullCacheKernelHeadFix);
            if ((archModel->opInfoArray[i]->target == VXNNE_OPERATION_TARGET_TP) && (tpCircularBuf == 0))
            {
                hasUnsupportedTPLayer = vx_true_e;
            }
        }

        if (kernelBufNeeded <= sramSpaceSize || 1)
        {
            vx_int32 starti = 0, endi = 8;
            sramLeft = sramSpaceSize - kernelBufNeeded;
            if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL) /*AB + SUBIMAGE for SWTiling*/
            {
                starti = 0;
                endi = 8;
            }
            else if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB)/*AB for SWTiling*/
            {
                starti = 0;
                endi = 0;
            }
            else if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_TILING) /*SUBIMAGE for SWTiling*/
            {
                starti = 1;
                endi = 8;
            }

            for (i = starti; i <= endi; i++)
            {
                vx_float64 cost_cycle = 0;
                vx_float64 cost_bw = 0;
                if ((i == 0)
                  || ((i == 1) && (kernelBufNeeded < vipSramSpaceSize) && !hasUnsupportedTPLayer)
                  || ((i >= 3) && ((i % 1 /*LcmPSIYSmallUnit*/) == 0) && (kernelBufNeeded < vipSramSpaceSize) && !hasUnsupportedTPLayer))
                {
                    if (!i)
                    {
                        abBufferSpaceLeft = _calc_ab_buffer(archModel, segment_first, segment_last, sramSpaceSize, xArray, yArray);
                    }
                    else if (i == 1)
                    {
                        _calc_y_subimage(archModel, segment_first, segment_last, sramLeft, xArray, yArray, zArray, context->nnConfig.unifiedFeature.maxTileSize);
                    }
                    else
                    {
                        _calc_x_subimage(archModel, segment_first, segment_last, sramLeft, i, xArray, yArray, zArray, context->nnConfig.unifiedFeature.maxTileSize);
                    }

                    for (j = segment_first; j <= segment_last; j++)
                    {
                        vx_float64 c = 0;
                        vx_bool flush_and_wait = vx_false_e;
                        if ((segment_first + 1) == segment_last)
                        {
                            flush_and_wait = vx_true_e;
                        }
                        archModel->opInfoArray[j]->perf.info.flush = 0;
                        if (xArray[j] > 0 && yArray[j] > 0)
                        {
                            if (i == 0)
                            {
                                if (axiSramOnlySWTiling)
                                {
                                    abBufferSpaceLeft = vipSramSpaceSize;
                                }
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait ? 1 : 0;
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        (vx_int32)(abBufferSpaceLeft/1024));
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(
                                        context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_DDR,
                                        (vx_int32)(abBufferSpaceLeft/1024));
                                }
                                else
                                {
                                    c = _calc_cost(
                                        context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        (vx_int32)(abBufferSpaceLeft/1024));
                                }
                            }
                            else
                            {
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait ? 1 : 0;
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
                                        (vx_int32)(vipSramSpaceSize/1024));
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_VIP_SRAM,
                                        (vx_int32)(vipSramSpaceSize/1024));
                                }
                                else
                                {
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
                                        (vx_int32)(vipSramSpaceSize/1024));
                                }
                            }

                            if (c == MAX_COST)
                            {
                                cost_cycle = MAX_COST;
                                cost_bw = MAX_COST;
                                break;
                            }

                            cost_cycle += c;
                            cost_bw += archModel->opInfoArray[j]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[j]->perf.resultInfo.perfWriteBandWidth;
                        }
                        else
                        {
                            cost_cycle = MAX_COST;
                            cost_bw = MAX_COST;
                            break;
                        }
                    }
                    cur_cost.cycle = cost_cycle;
                    cur_cost.bw = cost_bw;
#if ENABLE_ARCH_MODEL_DUMP
                    vxInfo("bestCost: %llu, bestBW: %llu, cost(%d, %d): %llu, bw: %llu\n", (vx_uint64)bestCost.cycle, (vx_uint64)bestCost.bw, segment_first + 1, segment_last + 1, (vx_uint64)cost_cycle, (vx_uint64)cost_bw);
#endif
                    if (_cur_cost_is_more_better(&bestCost, &cur_cost, CYCLE_WEIGHT, BW_WEIGHT))
                    {
                        bestCost.cycle = cur_cost.cycle;
                        bestCost.bw = cur_cost.bw;
                        for (j = segment_first; j <= segment_last; j++)
                        {
                            if (i == 0)
                            {
                                x_array[j] = archModel->opInfoArray[j]->xsize;
                                y_array[j] = archModel->opInfoArray[j]->ysize;
                                z_array[j] = archModel->opInfoArray[j]->oz;
                            }
                            else
                            {
                                x_array[j] = xArray[j];
                                y_array[j] = yArray[j];
                                z_array[j] = zArray[j];
                            }
                        }
                        *best_cost_swtiling_type = (i == 0) ? ((segment_first == segment_last) ?  -1 : 1) : 0;
                    }
                }

                if ((i <= 1) && (bestCost.cycle < MAX_COST))
                {
                    break;
                }
            }
        }
    }
    cost->cycle = bestCost.cycle;
    cost->bw = bestCost.bw;
exit:
    if (xArray)
        vxFree(xArray);

    if (yArray)
        vxFree(yArray);

    if (zArray)
        vxFree(zArray);
    return;
}

void getUpstreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 index,
    vx_uint32 id,
    vx_int32 *upstream)
{
    gcmASSERT(upstream != NULL);
    if (id < archModel->opInfoArray[index]->upStreamLayerCount)
    {
        *upstream = archModel->opInfoArray[index]->upStreamLayer[id];
    }
    else
    {
        *upstream = -1;
    }
}

void getDownstreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 index,
    vx_uint32 id,
    vx_int32 *downstream)
{
    gcmASSERT(downstream != NULL);
    if (id < archModel->opInfoArray[index]->downStreamLayerCount)
    {
        *downstream = archModel->opInfoArray[index]->downStreamLayer[id];
    }
    else
    {
        *downstream = -1;
    }
}


void updateStreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 count)
{
    vx_uint32 i, j;
    for (i = 0; i < count; i++)
    {
        vx_uint32 k = 0;
        for (j = 0; j < archModel->opInfoArray[i]->opt->parentOpNum; j++)
        {
            if (archModel->opInfoArray[i]->opt->parentOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || archModel->opInfoArray[i]->opt->parentOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                archModel->opInfoArray[i]->upStreamLayer[k++] = -1;
            }
            else
            {
                vx_int32 segIndex = archModel->opInfoArray[i]->opt->parentOps[j]->segIndex;
                if (k > 0)
                {
                    if (archModel->opInfoArray[i]->upStreamLayer[k] < segIndex)
                    {
                        vx_int32 m;
                        for (m = k - 1; m >= 0; m--)
                        {
                            archModel->opInfoArray[i]->upStreamLayer[m + 1] = archModel->opInfoArray[i]->upStreamLayer[m];
                        }
                        archModel->opInfoArray[i]->upStreamLayer[0] = segIndex;
                        k++;
                    }
                    else
                    {
                        archModel->opInfoArray[i]->upStreamLayer[k++] = segIndex;
                    }
                }
                else {
                    archModel->opInfoArray[i]->upStreamLayer[k++] = segIndex;
                }
            }
        }

        if (i == 0)
        {
            archModel->opInfoArray[i]->upStreamLayer[k++] = -1;
        }
        archModel->opInfoArray[i]->upStreamLayerCount = k;
    }

    for (i = 0; i < count; i++)
    {
        vx_uint32 k = 0;
        for (j = 0; j < archModel->opInfoArray[i]->opt->childOpNum; j++)
        {
            if (archModel->opInfoArray[i]->opt->childOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || archModel->opInfoArray[i]->opt->childOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (archModel->opInfoArray[i]->opt->childOpNum == 1) {
                    archModel->opInfoArray[i]->downStreamLayer[k++] = -1;
                }
            }
            else
            {
                vx_int32 m;
                vx_int32 segIndex = archModel->opInfoArray[i]->opt->childOps[j]->segIndex;
                vx_bool hasDownLayer = vx_false_e;
                /*check childOperation's upStreamLayer to determin if segIndex is a real downStreamLayer*/
                for (m = 0; m < (vx_int32)archModel->opInfoArray[segIndex]->upStreamLayerCount; m++)
                {
                    if (archModel->opInfoArray[segIndex]->upStreamLayer[m] == (vx_int32)i)
                    {
                        hasDownLayer = vx_true_e;
                        break;
                    }
                }
                if (!hasDownLayer)
                {
                    continue;
                }
                if (k > 0)
                {
                    if (archModel->opInfoArray[i]->downStreamLayer[k] < segIndex)
                    {
                        archModel->opInfoArray[i]->downStreamLayer[k++] = segIndex;
                    }
                    else
                    {
                        for (m = k - 1; m >= 0; m--)
                        {
                            archModel->opInfoArray[i]->downStreamLayer[m + 1] = archModel->opInfoArray[i]->downStreamLayer[m];
                        }
                        archModel->opInfoArray[i]->downStreamLayer[0] = segIndex;
                        k++;
                    }
                }
                else
                {
                    archModel->opInfoArray[i]->downStreamLayer[k++] = segIndex;
                }
            }
        }
        archModel->opInfoArray[i]->downStreamLayerCount = k;
    }
}



static void _split_segment_loop(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[])
{
    struct _archModelCost cost, upcost, downcost;
    struct _archModelCost cur_segment_cost = {MAX_COST, MAX_COST};
    vx_int32 len_segment, cur_pos, temp_pos;
#if ENABLE_ARCH_MODEL_DUMP
    vx_int32 debug = 0;
    if (segment_first == 0 && segment_last == 22)
    {
        debug = 1;
    }
#endif
    for (len_segment = 0; len_segment <= (segment_last - segment_first); len_segment++)
    {
        for (cur_pos = segment_first; cur_pos <= (segment_last - len_segment); cur_pos++)
        {
            getSegmentCostResult(archModel, cur_pos, cur_pos + len_segment, &cost);
            setSplitArrayInfo(archModel, cur_pos, cur_pos + len_segment, cur_pos, 1);
            if (cost.cycle != -1)
            {
                vx_int32 split_pos;
                for (split_pos = cur_pos; split_pos <= (cur_pos + len_segment); split_pos++)
                {
                    if ((archModel->opInfoArray[split_pos]->target != VXNNE_OPERATION_TARGET_TP ||
                        archModel->opInfoArray[split_pos]->opt->operatorType != VXNNE_OPERATOR_FULLYCONNECTED)
                        && split_pos != cur_pos)
                    {
                        getSegmentCostResult(archModel, cur_pos, split_pos - 1, &upcost);
                        if (upcost.cycle == MAX_COST)
                        {
                            break;
                        }
                        getSegmentCostResult(archModel, split_pos, cur_pos + len_segment, &downcost);
                        if (downcost.cycle == MAX_COST || upcost.cycle < 0 || downcost.cycle < 0)
                        {
                            cur_segment_cost.cycle = MAX_COST;
                            cur_segment_cost.bw = MAX_COST;
                        }
                        else
                        {
                            cur_segment_cost.cycle = upcost.cycle + downcost.cycle;
                            cur_segment_cost.bw = upcost.bw + downcost.bw;
                        }
#if ENABLE_ARCH_MODEL_DUMP
                        if (debug)
                        {
                            vxInfo("split_pos: %d, old_cost: %llu, old_bw: %llu cur_cost: %llu, cur_bw: %llu\n", split_pos + 1, (vx_uint64)cost.cycle, (vx_uint64)cost.bw, (vx_uint64)cur_segment_cost.cycle, (vx_uint64)cur_segment_cost.bw);
                        }
#endif
                        if (_cur_cost_is_more_better(&cost, &cur_segment_cost, CYCLE_WEIGHT, BW_WEIGHT))
                        {
                            cost.cycle = cur_segment_cost.cycle;
                            cost.bw = cur_segment_cost.bw;
                            setBestCostSWTilingTypeInfo(archModel, cur_pos, cur_pos + len_segment, -1);
                            for (temp_pos = cur_pos; temp_pos <= cur_pos + len_segment; temp_pos++)
                            {
                                if (temp_pos < split_pos)
                                {
                                    saveCalculationArgs(
                                            archModel,
                                            cur_pos,
                                            cur_pos + len_segment,
                                            temp_pos,
                                            &archModel->splitInfoArray[cur_pos]->savedCost[split_pos - 1][temp_pos],
                                            archModel->splitInfoArray[cur_pos]->savedSIX[split_pos - 1][temp_pos],
                                            archModel->splitInfoArray[cur_pos]->savedSIY[split_pos - 1][temp_pos],
                                            archModel->splitInfoArray[cur_pos]->savedSIZ[split_pos - 1][temp_pos],
                                            archModel->splitInfoArray[cur_pos]->split_array[split_pos - 1][temp_pos]);
                                }
                                else
                                {
                                    saveCalculationArgs(
                                            archModel,
                                            cur_pos,
                                            cur_pos + len_segment,
                                            temp_pos,
                                            &archModel->splitInfoArray[split_pos]->savedCost[cur_pos + len_segment][temp_pos],
                                            archModel->splitInfoArray[split_pos]->savedSIX[cur_pos + len_segment][temp_pos],
                                            archModel->splitInfoArray[split_pos]->savedSIY[cur_pos + len_segment][temp_pos],
                                            archModel->splitInfoArray[split_pos]->savedSIZ[cur_pos + len_segment][temp_pos],
                                            archModel->splitInfoArray[split_pos]->split_array[cur_pos + len_segment][temp_pos]);
                                }
                            }
                        }
                        setSegmentCostResult(archModel, cur_pos, cur_pos + len_segment, &cost);
                    }
                }
            }
        }
    }

    for (temp_pos = segment_first; temp_pos <= segment_last; temp_pos++)
    {
        x_array[temp_pos] = archModel->splitInfoArray[segment_first]->savedSIX[segment_last][temp_pos];
        y_array[temp_pos] = archModel->splitInfoArray[segment_first]->savedSIY[segment_last][temp_pos];
        z_array[temp_pos] = archModel->splitInfoArray[segment_first]->savedSIZ[segment_last][temp_pos];
        /* to save best cost */
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfCycleCount = archModel->splitInfoArray[segment_first]->savedCost[segment_last][temp_pos].cycle;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfWriteBandWidth = archModel->splitInfoArray[segment_first]->savedCost[segment_last][temp_pos].bw;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfReadBandWidth = 0;
    }
}

static void _split_segment(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[])
{
    vx_int32 i, j;
    struct _archModelCost cost = {MAX_COST, MAX_COST};
    vx_int32 split_pos = 0;
    vx_bool fullCacheKernelHeadFix = context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
    vx_int32 first, original_first = segment_first, prev_split = segment_first;
    vx_int32 temp_pos, totalSegments = segment_last -  segment_first + 1;
    vx_uint32 * cur_xArray = NULL;
    vx_uint32 * cur_yArray = NULL;
    vx_uint32 * cur_zArray = NULL;


    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    if (segment_first != segment_last)
    {
        for (split_pos = segment_first + 1; split_pos <= segment_last; split_pos++)
        {
            vx_bool connected = vx_false_e;
            vx_int32 downstream_pos;
            for (downstream_pos = split_pos; downstream_pos <= segment_last; downstream_pos++)
            {
                for (i = 0; i < (vx_int32)archModel->opInfoArray[downstream_pos]->upStreamLayerCount; i++)
                {
                    vx_int32 upStreamLayer;
                    getUpstreamLayer(archModel, downstream_pos, i, &upStreamLayer);
                    if ((upStreamLayer >= segment_first) && (upStreamLayer < split_pos))
                    {
                        connected = vx_true_e;
                        break;
                    }
                }
                if (connected)
                {
                    break;
                }
            }

            if (!connected)
            {
                split_array[split_pos] = 1;
            }
        }
    }

    if (segment_first != segment_last)
    {
        vx_int32 upStreamLayer;
        vx_int32 sramForSWTiling = axiSramOnlySWTiling
            ? context->nnConfig.customizedFeature.axiSRAMSizeInKB
            : (context->nnConfig.customizedFeature.vipSRAMSizeInKB - (VX_VIP_SRAM_IMAGE_STREAM_SIZE / 1024));
        for (split_pos = segment_first + 1; split_pos <= segment_last; split_pos++)
        {
            vx_uint32 size1, size2, outbufNeeded;

            if (archModel->opInfoArray[split_pos]->upStreamLayerCount > 1)
            {
                split_array[split_pos] = 1;
                continue;
            }

            if (archModel->opInfoArray[split_pos]->upStreamLayerCount > 0)
            {
                getUpstreamLayer(archModel, split_pos, 0, &upStreamLayer);
                if (upStreamLayer == -1)
                {
                    split_array[split_pos] = 1;
                    continue;
                }
                if (upStreamLayer != 0)
                {
                    if (archModel->opInfoArray[upStreamLayer]->downStreamLayerCount > 1)
                    {
                        split_array[split_pos] = 1;
                        continue;
                    }
                    else
                    {
                        if (upStreamLayer != (split_pos - 1))
                        {
                            split_array[split_pos] = 1;
                            continue;
                        }
                    }
                }
            }

            if ((archModel->opInfoArray[split_pos]->target == VXNNE_OPERATION_TARGET_TP && archModel->opInfoArray[split_pos]->op == VXNNE_OPERATOR_FULLYCONNECTED) ||
                (archModel->opInfoArray[split_pos - 1]->target == VXNNE_OPERATION_TARGET_TP && archModel->opInfoArray[split_pos - 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
            {
                split_array[split_pos] = 1;
                continue;
            }
            size1 = _kernel_size_in_pixel(archModel, split_pos, archModel->opInfoArray[split_pos]->nnCores, fullCacheKernelHeadFix);
            size2 = _kernel_size_in_pixel(archModel, split_pos - 1, archModel->opInfoArray[split_pos - 1]->nnCores, fullCacheKernelHeadFix);
            outbufNeeded = _outbuf_needed_ex(archModel, split_pos - 1, split_pos, NULL, NULL, NULL);
            if (((size1 + size2) > (context->nnConfig.customizedFeature.vipSRAMSizeInKB * 1024 - VX_VIP_SRAM_IMAGE_STREAM_SIZE))
                && ((vx_int32)outbufNeeded > sramForSWTiling * 1024))
            {
                split_array[split_pos] = 1;
                continue;
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("\n++init is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        vxInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    vxInfo("--init is_split==\n");
#endif

    for (i = 0; i <= totalSegments; i++)
    {
        if (cur_xArray == NULL)
            cur_xArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * totalSegments);
        if (cur_yArray == NULL)
            cur_yArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * totalSegments);
        if (cur_zArray == NULL)
            cur_zArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * totalSegments);
        gcmASSERT(cur_xArray != NULL && cur_yArray != NULL && cur_zArray != NULL);

        if (split_array[i] || (i == totalSegments))
        {
            vx_int32 last = i - 1;
            first = original_first;
            if (last >= first) {

                while (last >= first)
                {
                    while (first <= last)
                    {
                        vx_int32 bestCostSWTilingType = -1;
                        _subimage_segment_cost(context, archModel, first, last, cur_xArray, cur_yArray, cur_zArray, &bestCostSWTilingType, &cost);
#if ENABLE_ARCH_MODEL_DUMP
                        vxInfo("++_subimage_segment_cost(%d, %d)=%.7f, bw: %.7f\n", first + 1, last + 1, cost.cycle, cost.bw);
#endif
                        setSegmentCostResult(archModel, first, last, &cost);
                        setBestCostSWTilingTypeInfo(archModel, first, last, bestCostSWTilingType);
                        for (temp_pos = first; temp_pos <= last; temp_pos++)
                        {
                            struct _archModelCost cur_cost;
                            cur_cost.cycle = archModel->opInfoArray[temp_pos]->perf.resultInfo.perfCycleCount;
                            cur_cost.bw = archModel->opInfoArray[temp_pos]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[temp_pos]->perf.resultInfo.perfWriteBandWidth;
                            saveCalculationArgs(
                                archModel,
                                first,
                                last,
                                temp_pos,
                                &cur_cost,
                                cur_xArray[temp_pos],
                                cur_yArray[temp_pos],
                                cur_zArray[temp_pos],
                                split_array[temp_pos]);
                        }
                        first++;
                    }

                    last--;
                    first = original_first;
                }


            }
            original_first = i;
        }
    }

    if (cur_xArray) vxFree(cur_xArray);
    if (cur_yArray) vxFree(cur_yArray);
    if (cur_zArray) vxFree(cur_zArray);

    for (i = 0; i <= totalSegments; i++)
    {
        if ((split_array[i] && (i > 0)) || (i == totalSegments))
        {
            vx_int32 bestCostSWTilingType = getBestCostSWTilingTypeInfo(archModel, prev_split, i - 1);
            vx_bool alwaysSplit = vx_true_e;
            if (bestCostSWTilingType != 1 || alwaysSplit)
            {
                vx_int32 segStart, segEnd;
                _split_segment_loop(context, archModel, prev_split, i - 1, x_array, y_array, z_array);
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("++_split_segment_loop(%d, %d)\n", prev_split + 1, i - 1 + 1);
#endif
                for (temp_pos = prev_split; temp_pos <= (i - 1); temp_pos++)
                {
                    if (archModel->splitInfoArray[prev_split]->split_array[i - 1][temp_pos] && (archModel->splitInfoArray[prev_split]->bestCostSWTilingType[i - 1] != 1) && temp_pos != prev_split)
                    {
                        split_array[temp_pos] = archModel->splitInfoArray[prev_split]->split_array[i - 1][temp_pos];
#if ENABLE_ARCH_MODEL_DUMP
                        vxInfo("update _split_array[%d]: %d\n", temp_pos + 1, split_array[temp_pos]);
#endif
                    }
                }
                segStart = prev_split;
                for (temp_pos = prev_split + 1; temp_pos <= i; temp_pos++)
                {
                    if (temp_pos == totalSegments || split_array[temp_pos])
                    {
                        segEnd = temp_pos - 1;
                        if (segEnd > segStart)
                        {
                            for (j = segStart; j <= segEnd; j++)
                            {
                                archModel->opInfoArray[j]->swTilingType = archModel->splitInfoArray[segStart]->bestCostSWTilingType[segEnd];
                            }
                        }
                        else
                        {
                            archModel->opInfoArray[segEnd]->swTilingType = -1;
                        }
                        segStart = temp_pos;
                    }
                }
            }
            else
            {
                for (temp_pos = prev_split; temp_pos <= (i - 1); temp_pos++)
                {
                    x_array[temp_pos] = archModel->splitInfoArray[prev_split]->savedSIX[i - 1][temp_pos];
                    y_array[temp_pos] = archModel->splitInfoArray[prev_split]->savedSIY[i - 1][temp_pos];
                    z_array[temp_pos] = archModel->splitInfoArray[prev_split]->savedSIZ[i - 1][temp_pos];
                    archModel->opInfoArray[temp_pos]->swTilingType = archModel->splitInfoArray[prev_split]->bestCostSWTilingType[i - 1];
                }
            }
            prev_split = i;
        }
    }
#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("++split_end is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        vxInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    vxInfo("--split_end is_split==\n");
#endif
}

vx_uint32 _seg_buf_needed(
    struct _archModelInfo *archModel,
    vx_uint32 segment_first,
    vx_uint32 segment_last,
    vx_uint32 sixArray[],
    vx_uint32 siyArray[],
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[])
{
    vx_uint32 segBufNeeded = 0;
    if (segment_first == segment_last)
    {
        segBufNeeded = 0;
    }
    else
    {
        vx_bool allTypeABBuf = vx_true_e;
        vx_uint32 i;
        for (i = segment_first; i <= (segment_last - 1); i++)
        {
            if (archModel->opInfoArray[i]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                allTypeABBuf = vx_false_e;
            }
        }
        if (allTypeABBuf)
        {
            vx_uint32 abBufSize[2] = {0, 0};
            vx_uint32 abBufPairSize = 0;
            for (i = segment_first; i <= (segment_last - 1); i++)
            {
                abBufSize[i % 2] = _outbuf_needed_ex(archModel, i, i+1, sixArray, siyArray, z_array);
                abBufPairSize = gcmMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
            }
            segBufNeeded = abBufPairSize;
        }
        else
        {
            segBufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array);
        }
    }
    return segBufNeeded;
}

typedef struct _GIBIO {
    vx_int32 gib_input[99];
    vx_int32 gib_output[99];
    vx_uint32 gib_input_count;
    vx_uint32 gib_output_count;
} GIBIO;

typedef struct _GIBObj
{
    vx_uint32 gid;
    vx_uint32 totalBufferNeeded;
    vx_uint32 layerInBufferSize;
    vx_uint32 layerOutBufferSize;
} GIBObj;

vx_bool _gib_io_overlap(
    GIBIO *gibIO,
    vx_uint32 gib,
    struct _archModelInfo *archModel,
    vx_uint32 layer)
{
    vx_uint32 i, j;
    for (i = 0; i < gibIO[gib].gib_input_count; i++)
    {
        for (j = 0; j < archModel->opInfoArray[layer]->upStreamLayerCount; j++)
        {
            vx_int32 upStreamLayer;
            getUpstreamLayer(archModel, layer, j, &upStreamLayer);
            if (upStreamLayer > 0)
            {
                if (gibIO[gib].gib_input[i] == upStreamLayer)
                {
                    return vx_true_e;
                }
            }
        }
    }

    return vx_false_e;
}

void _append_gib_layer(
    GIBIO *gibIO,
    vx_uint32 gib,
    vx_int32 layer,
    vx_bool input)
{
    vx_bool  present = vx_false_e;
    vx_uint32 i;
    vx_uint32 count = input ? gibIO[gib].gib_input_count : gibIO[gib].gib_output_count;
    vx_int32 *target = input ? gibIO[gib].gib_input : gibIO[gib].gib_output;
    for (i = 0; i < count; i++)
    {
        if (target[i] == layer)
        {
            present = vx_true_e;
            break;
        }
    }

    if (!present)
    {
        if (input)
        {
            gibIO[gib].gib_input[gibIO[gib].gib_input_count] = layer;
            gibIO[gib].gib_input_count++;
        }
        else
        {
            gibIO[gib].gib_output[gibIO[gib].gib_output_count] = layer;
            gibIO[gib].gib_output_count++;
        }
    }
}

void _merge_gib_io(
    GIBIO *gibIO,
    vx_uint32 gib,
    struct _archModelInfo *archModel,
    vx_uint32 layer)
{
    vx_uint32 i;
    vx_int32 upStreamLayer;
    for (i = 0; i < archModel->opInfoArray[layer]->upStreamLayerCount; i++)
    {
        getUpstreamLayer(archModel, layer, i, &upStreamLayer);
        if (upStreamLayer > 0)
        {
            _append_gib_layer(gibIO, gib, upStreamLayer, vx_true_e);
        }
    }
}

vx_uint32 _create_gib(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_uint32 count,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[],
    GIBIO *gib_io,
    GIBObj *gib_obj)
{
    vx_int32 i, j, end_index;
    vx_uint32 gib, gib_last = 0;
    vx_bool *layerChecked = NULL;
    vx_uint32 segmentBufferNeeded, id;
    vx_uint32 *sixArray = NULL, *siyArray = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(1) return out-of-memory\n");
        return 0;
    }
    memset(sixArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(2) return out-of-memory\n");
        goto OnError;
    }
    memset(siyArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_bool) * archModel->totalOpCount,
        (gctPOINTER *)&layerChecked
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(3) return out-of-memory\n");
        goto OnError;
    }
    memset(layerChecked, 0, gcmSIZEOF(vx_bool) * archModel->totalOpCount);

    for (i = 0; i < (vx_int32)count; i++)
    {
        sixArray[i] = (vx_uint32)(ceilf((vx_float32)(x_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (vx_uint32)(ceilf((vx_float32)(y_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        layerChecked[i] = vx_false_e;
    }

    id = 0;
    end_index = count - 1;
    if (archModel->opInfoArray[0]->target == VXNNE_OPERATION_TARGET_NN || archModel->opInfoArray[0]->target  == VXNNE_OPERATION_TARGET_TP)
    {
        split_array[0] = 1;
    }
    for (i = count - 1; i >= 0; i--)
    {
        if (split_array[i]) {
            if (i == end_index)
            {
                segmentBufferNeeded = 0;
            }
            else
            {
                segmentBufferNeeded = _seg_buf_needed(archModel, i, end_index, sixArray, siyArray, x_array, y_array, z_array);
            }

            for (j = i; j <= end_index; j++)
            {
                gib_obj[j].gid = id;
                gib_obj[j].totalBufferNeeded = segmentBufferNeeded;
                gib_obj[j].layerOutBufferSize = archModel->opInfoArray[j]->bfy * archModel->opInfoArray[j]->pix * archModel->opInfoArray[j]->piy * archModel->opInfoArray[j]->bfz * archModel->opInfoArray[j]->oz;

                if (j == i)
                {
                    gib_obj[j].layerInBufferSize = 0;
                }
                else
                {
                    gib_obj[j].layerInBufferSize = gib_obj[j - 1].layerOutBufferSize;
                }
            }
            id++;
            end_index = i - 1;
        }
    }


    gib = 0;
    gib_last = 0;
    if (id == 1)
    {
        if (split_array[0] && !layerChecked[0])
        {
            gib_io[0].gib_input_count = archModel->opInfoArray[0]->upStreamLayerCount;
            gib_io[0].gib_output_count = archModel->opInfoArray[0]->downStreamLayerCount;
            gib_io[0].gib_output[0] = -1;
            gib_io[0].gib_input[0] = -1;
            layerChecked[0] = vx_true_e;
        }
        gib_last = gib;
        if (sixArray) vxFree(sixArray);
        if (siyArray) vxFree(siyArray);
        if (layerChecked) vxFree(layerChecked);
        return gib_last;
    }
    for (i = count - 1; i >= 1; i--)
    {
        if (split_array[i] && !layerChecked[i])
        {
            gib_io[gib].gib_input_count = archModel->opInfoArray[i]->upStreamLayerCount;
            if (gib_io[gib].gib_input_count != 0)
            {
                gib_last = gib;
                for (j = 0; j < (vx_int32)archModel->opInfoArray[i]->upStreamLayerCount; j++)
                {
                    vx_int32 upStreamLayer;
                    getUpstreamLayer(archModel, i, j, &upStreamLayer);
                    gib_io[gib].gib_input[j] = upStreamLayer;
                }
                gib_io[gib].gib_output_count = 1;
                gib_io[gib].gib_output[0] = i;

                for (j = (i - 1); j >= 0; j--)
                {
                    if (split_array[j])
                    {
                        if (archModel->opInfoArray[j]->target != VXNNE_OPERATION_TARGET_SH &&
                           archModel->opInfoArray[j]->target != VXNNE_OPERATION_TARGET_SW)
                        {
                            if (_gib_io_overlap(gib_io, gib, archModel, j))
                            {
                                _merge_gib_io(gib_io, gib, archModel, j);
                                _append_gib_layer(gib_io, gib, j, vx_false_e);
                                layerChecked[j] = vx_true_e;
                            }
                        }
                    }
                }
                gib++;
            }
        }
    }
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    if (layerChecked) vxFree(layerChecked);
    return gib_last;

OnError:
    if (sixArray)
        vxFree(sixArray);

    if (siyArray)
        vxFree(siyArray);

    return 0;
}

void _merge_sub_graph(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_uint32 count,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[],
    GIBIO *gib_io,
    GIBObj *gib_obj,
    vx_uint32 gib_last)
{
    vx_uint32 gib, j, k;
    vx_bool *gib_input_checked = NULL;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling;
    gceSTATUS status = gcvSTATUS_OK;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_bool) * archModel->totalOpCount,
        (gctPOINTER *)&gib_input_checked
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _merge_sub_graph() return out-of-memory\n");
        return;
    }
    memset(gib_input_checked, 0, gcmSIZEOF(vx_bool) * archModel->totalOpCount);

    for (gib = 0; gib <= gib_last; gib++)
    {
        vx_bool anyLayerExternal = vx_false_e;
        vx_uint32 outputSubimageSegmentCount = 0, inputSubimageSegmentCount = 0, outputABSegmentCount = 0, inputABSegmentCount = 0;
        struct _archModelCost baseCost = {0,0}, curCost = {0,0};

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            vx_int32 layer = gib_io[gib].gib_input[j];
            if (layer == -1)
            {
                anyLayerExternal = vx_true_e;
                break;
            }
        }
        if (anyLayerExternal)
        {
            continue;
        }

        for (j = 0; j < gib_io[gib].gib_output_count; j++)
        {
            vx_uint32 layer = gib_io[gib].gib_output[j];
            if (archModel->opInfoArray[layer]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                outputSubimageSegmentCount++;
            }
            else
            {
                outputABSegmentCount++;
            }

            baseCost.cycle += archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth;
#if ENABLE_ARCH_MODEL_DUMP
            vxInfo("gib_io[%d, %d].output, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));

#endif
        }

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            vx_uint32 layer = gib_io[gib].gib_input[j];
            if (archModel->opInfoArray[layer]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                inputSubimageSegmentCount++;
            }
            else
            {
                inputABSegmentCount++;
            }
            baseCost.cycle += archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth;
#if ENABLE_ARCH_MODEL_DUMP
            vxInfo("gib_io[%d, %d].input, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));

#endif
        }

        if (inputSubimageSegmentCount < 1 && outputSubimageSegmentCount < 1)
        {
            vx_uint32 gibBufferSize = 0;
            vx_uint32 bufferNeeded = 0;
            vx_int32 sramSize, abBufferSpaceLeftInKByte;
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                gib_input_checked[j] = vx_false_e;
            }
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                vx_int32 largest_GraphTotalBufferNeeded = -1;
                vx_uint32 largest_input = 0;
                for (k = 0; k < gib_io[gib].gib_input_count; k++)
                {
                    if (!gib_input_checked[k]
                    && largest_GraphTotalBufferNeeded < (vx_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded)
                    {
                        largest_GraphTotalBufferNeeded = (vx_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded;
                        largest_input = k;
                    }
                }
                gib_input_checked[largest_input] = vx_true_e;
                gibBufferSize += gib_obj[gib_io[gib].gib_input[largest_input]].layerOutBufferSize;
                bufferNeeded = gcmMAX(bufferNeeded,
                    gcmMAX(gib_obj[gib_io[gib].gib_input[largest_input]].totalBufferNeeded,
                    gib_obj[gib_io[gib].gib_input[largest_input]].layerInBufferSize + gibBufferSize));
            }

            for (j = 0; j < gib_io[gib].gib_output_count; j++)
            {
                bufferNeeded = gcmMAX(bufferNeeded,
                    gcmMAX(gib_obj[gib_io[gib].gib_output[j]].totalBufferNeeded,
                    gib_obj[gib_io[gib].gib_output[j]].layerOutBufferSize + gibBufferSize));
            }

            sramSize = axiSramOnlySWTiling ? context->nnConfig.customizedFeature.axiSRAMSizeInKB * 1024
                                           : (context->nnConfig.customizedFeature.vipSRAMSizeInKB * 1024 - VX_VIP_SRAM_IMAGE_STREAM_SIZE);

            if ((vx_int32)bufferNeeded < sramSize)
            {
                vx_uint8 kbuf = SW_TILING_FROM_DDR, sbuf, dbuf;
                vx_bool allOutputSegmentsAreShort = vx_true_e;
                vx_uint32 dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                abBufferSpaceLeftInKByte = axiSramOnlySWTiling ? (vx_int32)(context->nnConfig.customizedFeature.vipSRAMSizeInKB - VX_VIP_SRAM_IMAGE_STREAM_SIZE / 1024)
                                                               : (vx_int32)((sramSize - bufferNeeded) / 1024);
                for (j = 0; j < gib_io[gib].gib_output_count; j++)
                {
                    vx_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                    vx_bool my_gib_output_layer_is_last_in_segment = vx_false_e;
                    vx_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    vx_arch_perf_s perf;

                    if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                        (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                        )
                    {
                        allOutputSegmentsAreShort = vx_false_e;
                        my_gib_output_layer_is_last_in_segment = vx_true_e;
                    }
                    else
                    {
                        vx_int32 downStreamLayer;
                        getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                        if (downStreamLayer > 0 && split_array[downStreamLayer])
                        {
                            allOutputSegmentsAreShort = vx_false_e;
                            my_gib_output_layer_is_last_in_segment = vx_true_e;
                        }
                    }


                    my_sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    my_dbuf = my_gib_output_layer_is_last_in_segment ? SW_TILING_FROM_DDR
                                                                     : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);

                    archModel->opInfoArray[my_gib_output_layer]->perf.info.flush = 0;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_output_layer]->perf, sizeof(vx_arch_perf_s));
                    _calc_cost(context,
                            archModel,
                            my_gib_output_layer,
                            x_array[my_gib_output_layer],
                            y_array[my_gib_output_layer],
                            z_array[my_gib_output_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            abBufferSpaceLeftInKByte);

                    curCost.cycle += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfWriteBandWidth;
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_output_layer]->perf, &perf, sizeof(vx_arch_perf_s));
                }

                for (j = 0; j < gib_io[gib].gib_input_count; j++)
                {
                    vx_uint32 my_gib_input_layer = gib_io[gib].gib_input[j];
                    vx_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    vx_arch_perf_s perf;

                    my_sbuf = split_array[my_gib_input_layer] ? SW_TILING_FROM_DDR
                                                                    : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                    my_dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    archModel->opInfoArray[my_gib_input_layer]->perf.info.flush = allOutputSegmentsAreShort;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_input_layer]->perf, sizeof(vx_arch_perf_s));
                    _calc_cost(context,
                            archModel,
                            my_gib_input_layer,
                            x_array[my_gib_input_layer],
                            y_array[my_gib_input_layer],
                            z_array[my_gib_input_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            abBufferSpaceLeftInKByte);
                    curCost.cycle += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfWriteBandWidth;
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_input_layer]->perf, &perf, sizeof(vx_arch_perf_s));
                }
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("gib: %d, base_cycle: %llu, base_bw: %llu\n", gib+1, (vx_uint64)(baseCost.cycle + 0.5), (vx_uint64)(baseCost.bw + 0.5));
                vxInfo("gib: %d, cur_cycle: %llu, cur_bw: %llu\n", gib+1, (vx_uint64)(curCost.cycle + 0.5), (vx_uint64)(curCost.bw + 0.5));
#endif
                if (_cur_cost_is_more_better(&baseCost, &curCost, 1, 0))
                {
                    kbuf = SW_TILING_FROM_DDR;
                    abBufferSpaceLeftInKByte = axiSramOnlySWTiling ? ((vx_int32)context->nnConfig.customizedFeature.vipSRAMSizeInKB)
                                                                   : ((vx_int32)((sramSize - bufferNeeded) / 1024));
                    allOutputSegmentsAreShort = vx_true_e;
                    dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                    for (j = 0; j < gib_io[gib].gib_output_count; j++)
                    {
                        vx_uint32 src_graph_id;
                        vx_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                        vx_bool my_gib_output_layer_is_last_in_segment = vx_false_e;
                        if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                            (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                           )
                        {
                            allOutputSegmentsAreShort = vx_false_e;
                            my_gib_output_layer_is_last_in_segment = vx_true_e;
                        }
                        else
                        {
                            vx_int32 downStreamLayer;
                            getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                            if (downStreamLayer > 0 && split_array[downStreamLayer])
                            {
                                allOutputSegmentsAreShort = vx_false_e;
                                my_gib_output_layer_is_last_in_segment = vx_true_e;
                            }
                        }

                        src_graph_id = gib_obj[my_gib_output_layer].gid;
                        for (k = 0; k < count; k++)
                        {
                            if (src_graph_id == gib_obj[k].gid)
                            {
                                gib_obj[k].gid = dst_graph_id;
                                gib_obj[k].totalBufferNeeded = bufferNeeded;
                            }
                        }

                        {
                            sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                            dbuf = my_gib_output_layer_is_last_in_segment ? SW_TILING_FROM_DDR :
                                (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                            archModel->opInfoArray[my_gib_output_layer]->sbuf = sbuf;
                            archModel->opInfoArray[my_gib_output_layer]->dbuf = dbuf;
                            archModel->opInfoArray[my_gib_output_layer]->kbuf = kbuf;
                            archModel->opInfoArray[my_gib_output_layer]->perf.info.flush = 0;
                            _calc_cost(context,
                                archModel,
                                my_gib_output_layer,
                                x_array[my_gib_output_layer],
                                y_array[my_gib_output_layer],
                                z_array[my_gib_output_layer],
                                sbuf,
                                dbuf,
                                kbuf,
                                abBufferSpaceLeftInKByte);
                            split_array[my_gib_output_layer] = vx_false_e;
#if ENABLE_ARCH_MODEL_DUMP
                            vxInfo("== merged: %d\n", my_gib_output_layer + 1);
#endif
                        }
                    }

                    for (j = 0; j < gib_io[gib].gib_input_count; j++)
                    {
                        vx_uint32 src_graph_id = gib_obj[gib_io[gib].gib_input[j]].gid;
                        for (k = 0; k < count; k++)
                        {
                            if (src_graph_id == gib_obj[k].gid)
                            {
                                gib_obj[k].gid = dst_graph_id;
                                gib_obj[k].totalBufferNeeded = bufferNeeded;
                            }
                        }
                        {
                            if (split_array[gib_io[gib].gib_input[j]])
                            {
                                sbuf = SW_TILING_FROM_DDR;
                            }
                            else if (axiSramOnlySWTiling)
                            {
                                sbuf = SW_TILING_FROM_AXI_SRAM;
                            }
                            else
                            {
                                sbuf = SW_TILING_FROM_VIP_SRAM;
                            }
                            dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;


                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->sbuf = sbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->dbuf = dbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->kbuf = kbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->perf.info.flush = allOutputSegmentsAreShort ? 1 : 0;
                            _calc_cost(context,
                                archModel,
                                gib_io[gib].gib_input[j],
                                x_array[gib_io[gib].gib_input[j]],
                                y_array[gib_io[gib].gib_input[j]],
                                z_array[gib_io[gib].gib_input[j]],
                                sbuf,
                                dbuf,
                                kbuf,
                                abBufferSpaceLeftInKByte);
                        }
                    }
                }
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("++split_status after merged==\n");
    for (j = 0; j < count; j++)
    {
        vxInfo("layer[%d]: %d\n", j+1, split_array[j]);
    }
    vxInfo("--split_status after merged==\n");
#endif
    if (gib_input_checked) vxFree(gib_input_checked);
}

VX_INTERNAL_API vx_status vxoGraph_PredictPerf(vx_graph graph)
{
    vx_uint32 i, j, count=0;
    struct _archModelInfo * archModel;
    struct _archModelOpInfo ** opInfo;
    vx_context context = vxoContext_GetFromReference(&graph->base);
    vx_uint32 *xArray = NULL, *yArray = NULL, *zArray = NULL;
    vx_uint8 *sArray = NULL;
    vx_bool hasVXC = vx_false_e;
    gceSTATUS status = gcvSTATUS_OK;
    vx_status vxStatus = VX_SUCCESS;
    vx_bool supportNNTPParallel = vx_false_e;

    if (!graph->layer) return status;

    archModel = initArchModelInfo(graph->layer->base.num_operations);
    if (archModel == NULL)
    {
        vxStatus = VX_FAILURE;
        goto error;
    }
    opInfo = archModel->opInfoArray;
    initSegmentCostResult(archModel, graph->layer->base.num_operations);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&xArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(1) return out-of-memory\n");
        goto error;
    }
    memset(xArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&yArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(2) return out-of-memory\n");
        goto error;
    }
    memset(yArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&zArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(3) return out-of-memory\n");
        goto error;
    }
    memset(zArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint8) * archModel->totalOpCount,
        (gctPOINTER *)&sArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(4) return out-of-memory\n");
        goto error;
    }
    memset(sArray, 0, gcmSIZEOF(vx_uint8) * archModel->totalOpCount);

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
            if ((operation->operatorType == VXNNE_OPERATOR_CONVOLUTION || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV) &&
                operation->target == VXNNE_OPERATION_TARGET_NN)
            {
                vx_uint32 outXSize, outYSize;
                vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;

                wb = convOp->weights_biases;
                gcmASSERT(wb != NULL);
                gcmASSERT(convOp->orig_inputs != NULL);
                gcmASSERT(convOp->inputs != NULL);
                gcmASSERT(convOp->outputs != NULL);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0),
                     wb->weights_sizes[0],
                     WB_PAD_LEFT(wb),
                     WB_PAD_RIGHT(wb),
                     WB_POOLING_SIZE_X(wb),
                     WB_POOLING_STRIDE(wb),
                     &outXSize,
                     1);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1),
                     wb->weights_sizes[1],
                     WB_PAD_TOP(wb),
                     WB_PAD_BOTTOM(wb),
                     WB_POOLING_SIZE_Y(wb),
                     WB_POOLING_STRIDE(wb),
                     &outYSize,
                     1);

                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->psize   = gcmMAX(WB_POOLING_SIZE_X(wb), 1);
                opInfo[count]->pstride = WB_POOLING_SIZE_X(wb) ? 2 : 1;
                opInfo[count]->xpad    = convOp->pad_x_left;
                opInfo[count]->ypad    = convOp->pad_y_top;
                opInfo[count]->input_data_format = TENSOR_DATA_TYPE(convOp->inputs);
                opInfo[count]->dsize   = TENSOR_DATA_SIZE(convOp->inputs) * 8;
                opInfo[count]->weight_bias = wb;
                opInfo[count]->kx = wb->weights_sizes[0];
                opInfo[count]->ky = wb->weights_sizes[1];
                opInfo[count]->kz = wb->weights_sizes[2];
                opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 2);
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 2);
                opInfo[count]->origx = outXSize;
                opInfo[count]->origy = outYSize;
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1);
                opInfo[count]->p3 = convOp->pool_size_x == 3 ? 1 : 0;
                opInfo[count]->xsize = opInfo[count]->origx;
                opInfo[count]->ysize = opInfo[count]->origy;
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
                /* init buf */
                opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;
                opInfo[count]->fcmd = vx_true_e;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                if (count > 0 && supportNNTPParallel)
                {
                    if (opInfo[count - 1]->target == VXNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }

                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->input_data_format == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->input_data_format == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                count++;
            }
            else if (operation->target == VXNNE_OPERATION_TARGET_TP &&
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
                operation->operatorType == VXNNE_OPERATOR_LRN2 ||
                operation->operatorType == VXNNE_OPERATOR_SVDF_MAP ||
                operation->operatorType == VXNNE_OPERATOR_INTERLEAVE ||
                operation->operatorType == VXNNE_OPERATOR_ACTIVATION))
            {
                vxnne_tp_operation tpOp  = (vxnne_tp_operation)operation;

                gcmASSERT(tpOp->input != NULL);
                gcmASSERT(tpOp->output != NULL);
                if (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE)
                {
                    wb = tpOp->weights_biases;
                    gcmASSERT(wb != NULL);
                    gcmASSERT(tpOp->weights_biases != NULL);
                    opInfo[count]->kz = tpOp->weights_biases->weights_sizes[2];
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                    opInfo[count]->stridex = WB_STRIDE_X(wb);
                    opInfo[count]->stridey = WB_STRIDE_Y(wb);
                }
                else
                {
                    opInfo[count]->kz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                }

                opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 1);
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);
                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->psize   = operation->parameter.pool_size_x;
                opInfo[count]->pstride = operation->parameter.pool_stride;
                opInfo[count]->input_data_format = TENSOR_DATA_TYPE(tpOp->input);
                opInfo[count]->xpad    = tpOp->base.parameter.pad_x_left;
                opInfo[count]->ypad    = tpOp->base.parameter.pad_y_top;
                opInfo[count]->dsize = 8 * gcmMIN(TENSOR_DATA_SIZE(tpOp->input), TENSOR_DATA_SIZE(tpOp->output));
                opInfo[count]->weight_bias = VX_NULL;
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

                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                if (count > 0 && supportNNTPParallel)
                {
                    if (opInfo[count - 1]->target == VXNNE_OPERATION_TARGET_NN) /* NN -> TP*/
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
                if (opInfo[count]->input_data_format == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->input_data_format == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                operation->segIndex = count;
                count++;
            }
            else if (operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
            {
                if (operation->target == VXNNE_OPERATION_TARGET_TP)
                {
                    vxnne_tp_operation fcOp = (vxnne_tp_operation)operation;
                    vx_uint32 inDims = fcOp->input->dimCount;
                    vx_uint32 outDims = fcOp->output->dimCount;

                    wb = fcOp->weights_biases;
                    opInfo[count]->dsize = TENSOR_DATA_SIZE(fcOp->input);
                    opInfo[count]->input_data_format = TENSOR_DATA_TYPE(fcOp->input);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                    opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);

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
                            opInfo[count]->origx = 1;
                            opInfo[count]->origy = 1;
                            opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                        }

                        if ((outDims == 2) || (outDims == 1))
                        {
                            opInfo[count]->origoutx = 1;
                            opInfo[count]->origouty = 1;
                            opInfo[count]->oz = wb->weights_sizes[3];
                        }
                    }

                }
                else if (operation->target == VXNNE_OPERATION_TARGET_NN)
                {
                    vxnne_convolution_relu_pooling_operation fcOp = (vxnne_convolution_relu_pooling_operation)operation;
                    wb = fcOp->weights_biases;
                    opInfo[count]->dsize = TENSOR_DATA_SIZE(fcOp->inputs);
                    opInfo[count]->input_data_format = TENSOR_DATA_TYPE(fcOp->inputs);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 2);
                    opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 0);
                    opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 1);
                    opInfo[count]->oz = wb->weights_sizes[3];
                    opInfo[count]->psize   = gcmMAX(WB_POOLING_SIZE_X(wb), 1);
                    opInfo[count]->pstride = WB_POOLING_SIZE_X(wb) ? 2 : 1;
                }
                else
                {
                    continue;
                }

                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->xpad    = WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * WB_PAD_LEFT(wb));
                opInfo[count]->ypad    = WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) *  WB_PAD_TOP(wb));
                opInfo[count]->dsize = vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb)) * 8;
                opInfo[count]->weight_bias = wb;
                opInfo[count]->kx = 1;
                opInfo[count]->ky = 1;
                opInfo[count]->kz = wb->weights_sizes[2];
                opInfo[count]->p3 = 0;
                opInfo[count]->xsize = opInfo[count]->origoutx;
                opInfo[count]->ysize = opInfo[count]->origouty;
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
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
                        if (opInfo[count - 1]->target != VXNNE_OPERATION_TARGET_TP)
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
                        if (opInfo[count - 1]->target == VXNNE_OPERATION_TARGET_TP)
                        {
                            opInfo[count - 1]->bfy = 2;
                        }
                    }
                }
                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->input_data_format == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->input_data_format == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                count++;
            }
        }
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) && !hasVXC && (count > 1))
    {
        vx_uint32 first = 0;
        vx_uint32 last = count > 0 ? count - 1 : 0;
        GIBIO *gibIO = NULL;
        GIBObj *gibObj = NULL;
        vx_uint32 gibLast = 0;
        vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(GIBIO) * count,
            (gctPOINTER *)&gibIO
            );
        if (gcmIS_ERROR(status))
        {
            vxStatus = VX_FAILURE;
            vxInfo("ERROR: vxoGraph_PredictPerf(5) return out-of-memory\n");
            goto error;
        }
        memset(gibIO, 0, sizeof(GIBIO) * count);
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(GIBObj) * count,
            (gctPOINTER *)&gibObj
            );
        if (gcmIS_ERROR(status))
        {
            vxStatus = VX_FAILURE;
            vxInfo("ERROR: vxoGraph_PredictPerf(6) return out-of-memory\n");
            vxFree(gibIO);
            goto error;
        }
        memset(gibObj, 0, sizeof(GIBObj) * count);
        updateStreamLayer(archModel, count);

        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_NN ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_TP)
            {
                first = i;
                sArray[i] = 1;
                break;
            }
        }

        if (last < count)
        {
            _split_segment(context, archModel, first, last, xArray, yArray, zArray, sArray);

            if (/*context->options.enableHandleBranch*/ vx_true_e && /*always turn on branch merge feature for arch model algorithm*/
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB))
            {
                gibLast = _create_gib(context, archModel, count, xArray, yArray, zArray, sArray, gibIO, gibObj);
                _merge_sub_graph(context, archModel, count, xArray, yArray, zArray, sArray, gibIO, gibObj, gibLast);
            }

            for (i = first; i < count; i++)
            {
                if (opInfo[i]->upStreamLayerCount == 0)
                {
                    opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                }
                else
                {
                    opInfo[i]->sbuf = (sArray[i]) ? SW_TILING_FROM_DDR : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                }
                if (opInfo[i]->downStreamLayerCount == 0)
                {
                    opInfo[i]->dbuf = SW_TILING_FROM_DDR;
                }
                else
                {
                    opInfo[i]->dbuf = (sArray[i]) ? SW_TILING_FROM_DDR : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                }
            }
            /* first sbuf and last dbuf is DDR */
            opInfo[first]->sbuf = SW_TILING_FROM_DDR;
            opInfo[last]->dbuf = SW_TILING_FROM_DDR;

            for (i = first; i < count; i++)
            {
                opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                if (opInfo[i]->upStreamLayerCount > 0)
                {
                    vx_int32 uplayer = -1;
                    getUpstreamLayer(archModel, i, opInfo[i]->upStreamLayerCount - 1, &uplayer);
                    if (uplayer >= 0) {
                        opInfo[i]->sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    }
                }

                opInfo[i]->dbuf = SW_TILING_FROM_DDR;
                if (opInfo[i]->upStreamLayerCount > 0)
                {
                    vx_int32 downlayer = -1;
                    getDownstreamLayer(archModel, i, 0, &downlayer);
                    if (downlayer > 0)
                    {
                        opInfo[i]->dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    }
                }

                if (opInfo[i]->swTilingType == 0)
                {
                    opInfo[i]->kbuf = SW_TILING_FROM_VIP_SRAM;
                }
                else
                {
                    opInfo[i]->kbuf = SW_TILING_FROM_DDR;
                }

                if (sArray[i])
                {
                    opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                    if (opInfo[i]->upStreamLayerCount > 0)
                    {
                        for (j = 0; j < opInfo[i]->upStreamLayerCount; j++)
                        {
                            vx_int32 uplayer;
                            getUpstreamLayer(archModel, i, j, &uplayer);
                            if (uplayer >= 0) {
                                opInfo[uplayer]->dbuf = SW_TILING_FROM_DDR;
                            }
                        }
                    }
                }
            }
        }
        for (i = first; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (context->options.enableNNArchPerfPrint)
                {
                    showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, VX_NULL);
                }
                continue;
            }

            {
                vx_int32 spaceLeftInKByte = axiSramOnlySWTiling ? (vx_int32)(context->nnConfig.customizedFeature.vipSRAMSizeInKB - (VX_VIP_SRAM_IMAGE_STREAM_SIZE / 1024))
                                                           : (vx_int32)((vx_float32)context->nnConfig.customizedFeature.vipSRAMSizeInKB - (VX_VIP_SRAM_IMAGE_STREAM_SIZE / 1024) - (vx_float32)gibObj[i].totalBufferNeeded / 1024);
                vx_int32 flush_and_wait = 0;
                if (opInfo[i]->sbuf == SW_TILING_FROM_DDR)
                {
                    if (opInfo[i]->downStreamLayerCount == 0)
                    {
                        flush_and_wait = 1;
                    }
                    else
                    {
                        for (j = 0; j < opInfo[i]->downStreamLayerCount; j++)
                        {
                            vx_int32 downlayer = -1;
                            getDownstreamLayer(archModel, i, j, &downlayer);
                            if (downlayer == -1)
                            {
                                flush_and_wait = 1;
                                break;
                            }
                            else
                            {
                                if (opInfo[downlayer]->dbuf == SW_TILING_FROM_DDR)
                                {
                                    flush_and_wait = 1;
                                    break;
                                }
                            }
                        }
                    }
                    opInfo[i]->perf.info.flush = flush_and_wait;
                }
                else
                {
                    opInfo[i]->perf.info.flush = flush_and_wait;
                }
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeftInKByte: %u, flush_and_waith: %d\n", i + 1,
                opInfo[i]->sbuf == 0 ? "DDR" : (opInfo[i]->sbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->dbuf == 0 ? "DDR" : (opInfo[i]->dbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->kbuf == 0 ? "DDR" : (opInfo[i]->kbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                spaceLeftInKByte,
                opInfo[i]->perf.info.flush);
#endif
                _calc_cost(context, archModel, i,
                    xArray[i], yArray[i], zArray[i],
                    opInfo[i]->sbuf,
                    opInfo[i]->dbuf,
                    opInfo[i]->kbuf,
                    spaceLeftInKByte);
            }

            if (context->options.enableNNArchPerfPrint)
            {
                showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, &opInfo[i]->perf);
            }

        }
        vxFree(gibIO);
        vxFree(gibObj);
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (context->options.enableNNArchPerfPrint)
                {
                    showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, VX_NULL);
                }
                continue;
            }
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeftInKByte: %u, flush_and_waith: %d\n", i + 1,
                "DDR",
                "DDR",
                "DDR",
                context->nnConfig.customizedFeature.vipSRAMSizeInKB,
                1);
#endif
            opInfo[i]->perf.info.flush = 1;
            _calc_cost(context, archModel, i,
                opInfo[i]->xsize, opInfo[i]->ysize, opInfo[i]->oz,
                SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, (vx_int32)context->nnConfig.customizedFeature.vipSRAMSizeInKB - (VX_VIP_SRAM_IMAGE_STREAM_SIZE / 1024));

            if (context->options.enableNNArchPerfPrint)
            {
                showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, &opInfo[i]->perf);
            }

        }
    }

error:
    if (xArray) vxFree(xArray);
    if (yArray) vxFree(yArray);
    if (zArray) vxFree(zArray);
    if (sArray) vxFree(sArray);
    deInitArchModelInfo(archModel, count);
    return vxStatus;
}
/*******************************************Predict arch performance***************************************************/

VX_INTERNAL_API vx_bool vxoGraph_IsInTailNodesTable(vx_graph graph, vx_uint32 nodeIndex)
{
    vx_uint32 i;
    vx_bool result = vx_false_e;

    for (i = 0; i < graph->tailNodeCount; i++)
    {
        result = (nodeIndex == graph->tailNodeIndexTable[i]) ? vx_true_e : vx_false_e;

        if (result == vx_true_e)
            break;
    }

    return result;
}

VX_INTERNAL_API vx_status vxoGraph_PrepareParamMemory(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, i;

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        for (i = 0; i < node->kernel->signature.paramCount; i++)
        {
            vx_reference paramRef = node->paramTable[i];

            if (paramRef == VX_NULL) continue;
            if (paramRef->type == VX_TYPE_TENSOR)
            {
                if (vxoTensor_AllocateMemory((vx_tensor)paramRef) != VX_SUCCESS)
                {
                    vxError("Node %p(\"%s\"): Can't allocate memory for No.%d tensor parameter",
                        node, node->kernel->name, i);
                    status = VX_ERROR_NO_MEMORY;
                }
            }

#if gcdDUMP
            if ((vxoGraph_IsInTailNodesTable(graph, nodeIndex) == vx_true_e) &&
                (paramRef->type == VX_TYPE_TENSOR) &&
                vxmIS_OUTPUT_OR_BIDIRECTION(node->kernel->signature.directionTable[i]))
            {
                vx_uint32 size;
                vxoTensor_GetTensorSize((vx_tensor)paramRef, &size);
                gcmDUMP(gcvNULL, "#[output]\n");
                gcmDUMP_BUFFER(gcvNULL,
                    gcvDUMP_BUFFER_MEMORY,
                    TENSOR_PHYSICAL_ADDR((vx_tensor)paramRef),
                    TENSOR_LOGICAL_ADDR((vx_tensor)paramRef),
                    0,
                    size);
            }
#endif
        }

        if (node->layer != VX_NULL)
        {
            for (i = 0; i < node->layer->num_temp_tensors; i++)
            {
                if (vxoTensor_AllocateMemory(node->layer->temp_tensors[i]) != VX_SUCCESS)
                {
                    vxError("Node %p(\"%s\"): Can't allocate memory for No.%d tensor",
                        node, node->kernel->name, node->layer->temp_tensors[i]);
                    status = VX_ERROR_NO_MEMORY;
                }
            }
        }
    }

    return status;
}
VX_INTERNAL_API vx_status vxoGraph_VerifyVirtualBuffer(vx_graph graph)
{
    vx_uint32 i, j, count = 0;
    vxnne_mem_request rlist = VX_NULL;
    vx_context context = graph->base.context;
    vx_status status = VX_SUCCESS;
    vx_bool enablePool = context->options.enableMemPool ? vx_true_e : vx_false_e;

    if (enablePool)
    {
        if (graph->memoryPool != VX_NULL)
        {
            vxoMemoryPool_Deinitialize(graph);
        }
        if ((graph->virtTensorNum != 0) && vxoMemoryPool_Initialize(graph, context->options.memPoolSize))
        {
            status = VX_SUCCESS;
        }
        else if (graph->virtTensorNum != 0)
        {
            vxError("Can't allocate memory for virtual memory pool");
            return VX_FAILURE;
        }

        status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vxnne_mem_request_s) * graph->layer->opIndicesNum, (gctPOINTER*)&rlist);
        if (gcmIS_ERROR(status)) goto exit;

        gcoOS_ZeroMemory(rlist, gcmSIZEOF(vxnne_mem_request_s) * graph->layer->opIndicesNum);
    }

    /* allocate non-virtual tensor first */
    for (i = 0; i < graph->layer->opIndicesNum; i++)
    {
        vxnne_operation op = graph->layer->operations[graph->layer->opIndices[i].operationID];

        if (!graph->layer->opIndices[i].inputTile.sRAM)
        {
            for (j = 0; j < op->inputsNum; j++)
            {
                if (op->inputs[j] != VX_NULL && op->inputs[j]->type == VX_TYPE_TENSOR)
                {
                    vx_tensor input = (vx_tensor)op->inputs[j];
                    vx_memory inmem = &input->tensorBuffer->memory;

                    if (!vxoTensor_IsVirtualTensor(input) || !enablePool)
                    {
                        status = vxoTensor_AllocateMemory(input);
                        if (status != VX_SUCCESS) goto exit;
                        vxoMemory_SetType(inmem, VXNNE_MEM_POOL_TYPE_ORIG_DDR);
                    }
                    else
                    {
                        vx_size size;
                        vxoTensor_GetTensorWholeSize(input, &size);
                        vxoMemory_ResetOffset(inmem);
                        vxoMemory_SetSize(inmem, gcmALIGN(size, 64));
                        vxoMemory_SetType(inmem, VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR);
                    }

                    if (enablePool)
                    {
                        rlist[count].inputMemory[rlist[count].inputCount] = inmem;
                        rlist[count].inputCount++;
                        vxmASSERT(rlist[count].inputCount <= VX_MAX_MEM_REQUEST_INPUT);
                    }
                }
            }
        }

        if (!graph->layer->opIndices[i].outputTile.sRAM)
        {
            for (j = 0; j < op->outputsNum; j++)
            {
                if (op->outputs[j] != VX_NULL && op->outputs[j]->type == VX_TYPE_TENSOR)
                {
                    vx_tensor output = (vx_tensor)op->outputs[j];
                    vx_memory outmem = &output->tensorBuffer->memory;

                    if (!vxoTensor_IsVirtualTensor(output) || !enablePool)
                    {
                        status = vxoTensor_AllocateMemory(output);
                        if (status != VX_SUCCESS) goto exit;
                        vxoMemory_SetType(outmem, VXNNE_MEM_POOL_TYPE_ORIG_DDR);
                    }
                    else
                    {
                        vx_size size;
                        vxoTensor_GetTensorWholeSize(output, &size);
                        vxoMemory_ResetOffset(outmem);
                        vxoMemory_SetSize(outmem, gcmALIGN(size, 64));
                        vxoMemory_SetType(outmem, VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR);
                    }

                    if (enablePool)
                    {
                        rlist[count].outputMemory[rlist[count].outputCount] = outmem;
                        rlist[count].outputCount++;
                        vxmASSERT(rlist[count].outputCount <= VX_MAX_MEM_REQUEST_OUTPUT);
                    }
                }
            }
        }

        if (enablePool && (rlist[count].inputCount > 0 || rlist[count].outputCount > 0)) count++;
    }

    if (enablePool && count > 0)
    {
        vx_memory emem = VX_NULL;

        status = vxoMemoryPool_RequestList(graph, rlist, count, 0, count, &emem);

        /* for debug */
        if (status != VX_SUCCESS && emem != VX_NULL)
        {
            for (i = 0; i < graph->layer->opIndicesNum; i++)
            {
                vxnne_operation op = graph->layer->operations[graph->layer->opIndices[i].operationID];

                if (!graph->layer->opIndices[i].inputTile.sRAM)
                {
                    for (j = 0; j < op->inputsNum; j++)
                    {
                        if (op->inputs[j] != VX_NULL && op->inputs[j]->type == VX_TYPE_TENSOR)
                        {
                            vx_tensor input = (vx_tensor)op->inputs[j];
                            vx_memory inmem = &input->tensorBuffer->memory;
                            if (emem  == inmem)
                            {
                                gcmPRINT("[layer id: %d layer name: %s op type: %s op target: %s] the %dth input tensor %p is not balance",
                                          op->layer->node->id, op->layer->name,
                                          vxnneGetOperatorTypeName(op->operatorType),
                                          vxnneGetOperatorTargetName(op->target),
                                          j, input);
                            }
                        }
                    }
                }

                if (!graph->layer->opIndices[i].outputTile.sRAM)
                {
                    for (j = 0; j < op->outputsNum; j++)
                    {
                        if (op->outputs[j] != VX_NULL && op->outputs[j]->type == VX_TYPE_TENSOR)
                        {
                            vx_tensor output = (vx_tensor)op->outputs[j];
                            vx_memory outmem = &output->tensorBuffer->memory;
                            if (emem == outmem)
                            {
                                gcmPRINT("[layer id: %d layer name: %s op type: %s op target: %s] the %dth output tensor %p is not balance",
                                          op->layer->node->id, op->layer->name,
                                          vxnneGetOperatorTypeName(op->operatorType),
                                          vxnneGetOperatorTargetName(op->target),
                                          j, output);
                            }
                        }
                    }
                }
            }
        }

        gcmASSERT(status == VX_SUCCESS);
    }

exit:

    if (rlist != VX_NULL)
    {
        gcoOS_FreeMemory(gcvNULL, rlist);
        rlist = VX_NULL;
    }

    return status;
}

#endif

VX_INTERNAL_API void vxoGraph_GenerateNextNodeTable(vx_graph graph,
    vx_uint32 lastNodeIndexTable[VX_MAX_REF_COUNT], vx_uint32 lastNodeCount,
    OUT vx_uint32 nextNodeIndexTable[VX_MAX_REF_COUNT], OUT vx_uint32_ptr nextNodeCountPtr,
    INOUT vx_uint32 leftNodeIndexTable[VX_MAX_REF_COUNT], INOUT vx_uint32_ptr leftNodeCountPtr)
{
    vx_uint32 possibleNextNodeIndexTable[VX_MAX_REF_COUNT];
    vx_uint32 possibleNextNodeCount = 0;
    vx_uint32 index;
    vx_uint32 paramIndex;

    vxmASSERT(lastNodeIndexTable);
    vxmASSERT(lastNodeCount > 0);
    vxmASSERT(nextNodeIndexTable);
    vxmASSERT(nextNodeCountPtr);
    vxmASSERT(leftNodeIndexTable);
    vxmASSERT(leftNodeCountPtr);

    *nextNodeCountPtr = 0;

    /* Build up the possible next index table from the last node index table */
    for (index = 0; index < lastNodeCount; index++)
    {
        vx_node lastNode = graph->nodeTable[lastNodeIndexTable[index]];

        vxmASSERT(lastNode);

        for (paramIndex = 0; paramIndex < lastNode->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference    paramRef;
            vx_uint32       newPossibleNextNodeCount;

            if (lastNode->kernel->signature.directionTable[paramIndex] == VX_INPUT) continue;

            paramRef = lastNode->paramTable[paramIndex];

            if (paramRef == VX_NULL) continue;

            newPossibleNextNodeCount = vxmLENGTH_OF(possibleNextNodeIndexTable) - possibleNextNodeCount;

            if (vxoGraph_FindAllRelatedNodes(graph, VX_INPUT, paramRef,
                &possibleNextNodeIndexTable[possibleNextNodeCount],
                INOUT &newPossibleNextNodeCount) == VX_SUCCESS)
            {
                possibleNextNodeCount += newPossibleNextNodeCount;
            }
        }
    }

    /* Add all left nodes to the possible next node index table */
    for (index = 0; index < *leftNodeCountPtr; index++)
    {
        vx_uint32   index2;
        vx_bool     found = vx_false_e;

        for (index2 = 0; index2 < possibleNextNodeCount; index2++)
        {
            if (leftNodeIndexTable[index] == possibleNextNodeIndexTable[index2])
            {
                found = vx_true_e;
                break;
            }
        }

        if (!found)
        {
            possibleNextNodeIndexTable[possibleNextNodeCount] = leftNodeIndexTable[index];
            possibleNextNodeCount++;
            vxmASSERT(possibleNextNodeCount < VX_MAX_REF_COUNT);
        }
    }

    *leftNodeCountPtr = 0;

    /* now check all possible next nodeTable to see if the parent nodeTable are visited. */
    for (index = 0; index < possibleNextNodeCount; index++)
    {
        vx_uint32   possibleNextNodeIndex = possibleNextNodeIndexTable[index];
        vx_node     possibleNextNode = graph->nodeTable[possibleNextNodeIndex];

        vx_uint32   possibleParamIndexTable[VX_MAX_PARAMETERS];
        vx_uint32   possibleParamCount = 0;
        vx_uint32   index2;
        vx_bool     areAllParamsReady = vx_true_e;

        /* Build up the possible parameter index table from the possible next node */
        for (paramIndex = 0; paramIndex < possibleNextNode->kernel->signature.paramCount; paramIndex++)
        {
            if (possibleNextNode->kernel->signature.directionTable[paramIndex] == VX_INPUT)
            {
                possibleParamIndexTable[possibleParamCount] = paramIndex;
                possibleParamCount++;
            }
        }

        /* Check if all possible parameters are ready */
        for (index2 = 0; index2 < possibleParamCount; index2++)
        {
            vx_uint32       possibleParamIndex  = possibleParamIndexTable[index2];
            vx_reference    possibleParamRef    = possibleNextNode->paramTable[possibleParamIndex];
            vx_direction_e  paramDirTable[]     = {VX_OUTPUT, VX_BIDIRECTIONAL};
            vx_uint32       index3;

            for (index3 = 0; index3 < vxmLENGTH_OF(paramDirTable); index3++)
            {
                vx_direction_e  direction = paramDirTable[index3];
                vx_uint32       predicateNodeIndexTable[VX_MAX_REF_COUNT];
                vx_uint32       predicateNodeCount = vxmLENGTH_OF(predicateNodeIndexTable);
                vx_uint32       index4;

                if (vxoGraph_FindAllRelatedNodes(graph, direction, possibleParamRef,
                    predicateNodeIndexTable, INOUT &predicateNodeCount) != VX_SUCCESS) continue;

                /* Check if all predicate nodes executed */
                for (index4 = 0; index4 < predicateNodeCount; index4++)
                {
                    vx_node predicateNode = graph->nodeTable[predicateNodeIndexTable[index4]];

                    vxmASSERT(predicateNode);

                    if (!predicateNode->executed)
                    {
                        areAllParamsReady = vx_false_e;
                        break;
                    }
                }

                if (!areAllParamsReady) break;
            }
        }

        /* Add the possible node index to the next node index table or the left node index table */
        if (areAllParamsReady)
        {
            if (!possibleNextNode->visited)
            {
                nextNodeIndexTable[*nextNodeCountPtr] = possibleNextNodeIndex;
                (*nextNodeCountPtr)++;
                possibleNextNode->visited = vx_true_e;
            }
        }
        else
        {
            leftNodeIndexTable[*leftNodeCountPtr] = possibleNextNodeIndex;
            (*leftNodeCountPtr)++;
        }
    }
}
VX_INTERNAL_API void vxoGraph_PolluteIfInput(vx_graph graph, vx_reference targetRef)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);
    vxmASSERT(targetRef);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_uint32 paramIndex;
        vx_node node = graph->nodeTable[nodeIndex];

        for (paramIndex = 0u; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            if (node->kernel->signature.directionTable[paramIndex] == VX_OUTPUT) continue;

            if (node->paramTable[paramIndex] == targetRef)
            {
                graph->reverify = graph->verified;

                graph->verified = vx_false_e;

                graph->status   = VX_GRAPH_STATE_UNVERIFIED;
                return;
            }
        }
    }
}

VX_API_ENTRY vx_graph VX_API_CALL vxCreateGraph(vx_context context)
{
    vx_graph graph;

    gcmDUMP_API("$VX vxCreateGraph: context=%p", context);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    graph = (vx_graph)vxoReference_Create(context, VX_TYPE_GRAPH, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)graph) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get graph reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&context->base, vxoReference_GetStatus((vx_reference)graph), "%s[%d]: Get graph reference failed!\n", __FUNCTION__, __LINE__);
        return graph;
    }

    graph->dirty = vx_true_e;

    graph->reverify = graph->verified;

    graph->verified = vx_false_e;

    graph->hasAutoAgingDelay = vx_false_e;

    graph->status = VX_GRAPH_STATE_UNVERIFIED;
    vxCreateMutex(OUT &graph->scheduleLock);

    vxoPerf_Initialize(&graph->perf);

    vxoGraph_Dump(graph);

    graph->headTensorCount = 0;

    graph->headTensorCountTable = VX_NULL;

    graph->graphID = context->graphCount++;

    graph->binarySave = VX_NULL;

    return graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    gcmDUMP_API("$VX vxSetGraphAttribute: graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_GRAPH_DEVICE_INDEX_VIV:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            if(*(vx_uint32*) ptr >= graph->base.context->deviceCount)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            else
            {
                graph->deviceID = *(vx_uint32*) ptr;
            }
            break;

        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
    gcmDUMP_API("$VX vxQueryGraph: graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
    case VX_GRAPH_PERFORMANCE:
        vxmVALIDATE_PARAMETERS(ptr, size, vx_perf_t, 0x3);

        *(vx_perf)ptr = graph->perf;

        vxoPerf_Dump(&graph->perf);
        break;

    case VX_GRAPH_STATE:
        vxmVALIDATE_PARAMETERS(ptr, size, vx_status, 0x3);

        *(vx_status *)ptr = graph->status;
        break;

    case VX_GRAPH_NUMNODES:
        vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

        *(vx_uint32 *)ptr = graph->nodeCount;
        break;

    case VX_GRAPH_NUMPARAMETERS:
        vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

        *(vx_uint32 *)ptr = graph->paramCount;
        break;

     case VX_GRAPH_DEVICE_INDEX_VIV:
        vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

        *(vx_uint32*) ptr=  graph->deviceID ;
        break;

    default:
        vxError("The attribute parameter, %d, is not supported", attribute);
        return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vx_vivPeferGraph(vx_graph graph, vx_char* path, vx_bool only_top, vx_bool top, vx_char* buffer)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex = 0;
    vx_uint32 offset = 0;

#define _OUTPUT_BUFFER_SZ   1024 * 100
#define _TEMP_BUFFER_SZ     1024
    vx_char log[_TEMP_BUFFER_SZ] = {0};
    vx_char outputBuffer[_OUTPUT_BUFFER_SZ] = {'\0'};
    vx_char tempBuffer[_OUTPUT_BUFFER_SZ] = {'\0'};


    FILE* fp = NULL;
    vx_char * output = (buffer != NULL)? buffer:outputBuffer;

    gcmDUMP_API("$VX vx_vivPeferGraph: graph=%p, path=%s, only_top=0x%x, top=0x%x, buffer=%s", graph, path, only_top, top, buffer);

    if(!graph)
        return VX_ERROR_INVALID_GRAPH;

    if (only_top && graph->isChildGraph)
        return VX_ERROR_INVALID_GRAPH;

    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, "    <graph>\n");
    gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "        <name>%p</name>\n", graph);
    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);
    gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "        <start>%llu</start>\n", (unsigned long long)graph->perf.beg);
    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);
    gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "        <end>%llu</end>\n", (unsigned long long)graph->perf.end);
    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);
    gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "        <interval>%llu</interval>\n\n", (unsigned long long)graph->perf.tmp);
    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);

    for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, "        <node>\n");

        gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "            <name>%s</name>\n", node->kernel->name);
        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);

        gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "            <start>%llu</start>\n", (unsigned long long)node->perf.beg);
        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);

        gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "            <end>%llu</end>\n", (unsigned long long)node->perf.end);
        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);

        gcoOS_PrintStrSafe(log, _TEMP_BUFFER_SZ, &offset, "            <interval>%llu</interval>\n\n", (unsigned long long)node->perf.tmp);
        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, log);

        gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, "        </node>\n");

        if (node->childGraph != NULL)
        {
            memset(tempBuffer, 0, sizeof(tempBuffer));
            vx_vivPeferGraph(node->childGraph, NULL, vx_false_e, vx_false_e, tempBuffer);
            gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, tempBuffer);
        }
    }

    gcoOS_StrCatSafe(output, _OUTPUT_BUFFER_SZ, "    </graph>\n");

    if(top)
    {
        fp = fopen(path, "w+");
        fwrite(output, sizeof(vx_char), strlen(output), fp);
        fclose(fp);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseGraph(vx_graph *graph)
{
    vx_status status;
    if (graph && *graph)
    {
        vx_char * path = (vx_char *)((*graph)->base.context->options.graphPerfLogFile);
        if (path)
            vx_vivPeferGraph(*graph, path, vx_true_e, vx_true_e, NULL);
    }

    gcmDUMP_API("$VX vxReleaseGraph: graph=%p", graph);

    status = vxoReference_Release((vx_reference_ptr)graph, VX_TYPE_GRAPH, VX_REF_EXTERNAL);


    return status;
}

VX_PRIVATE_API vx_bool vxoGraph_IsParentGraphScope(vx_reference scope, vx_graph graph)
{
    vx_bool flag = vx_false_e;
    vx_graph nextGpraph = graph;

    vxmASSERT(graph);
    vxmASSERT(scope);

    if (vxoReference_GetType(scope) != VX_TYPE_GRAPH)
        return flag;

    do
    {
        if(scope == (vx_reference)nextGpraph)
        {
            flag = vx_true_e;
            break;
        }
        else
        {
            nextGpraph = nextGpraph->parentGraph;
        }

    }while (nextGpraph != NULL);

    return flag;
}

VX_PRIVATE_API vx_status vxoGraph_InitMetaFormatData(vx_graph graph, vx_node node, vx_uint32 paramIndex, vx_reference_s **vRef, vx_meta_format* metaFormat, vx_status* status)
{
    *vRef = node->paramTable[paramIndex];
    *metaFormat = vxoMetaFormat_Create(graph->base.context);

    if ((*vRef)->isVirtual == vx_false_e)
    {
        *vRef = NULL;
    }
    else
    {
        if (!vxoGraph_IsParentGraphScope((*vRef)->scope, graph))
        {
            *status = VX_ERROR_INVALID_SCOPE;
            vxAddLogEntry((vx_reference)(*vRef), *status, "Virtual Reference is in the wrong scope, created from another graph!\n");

            return vx_false_e;
        }
    }
    (*metaFormat)->type = node->kernel->signature.dataTypeTable[paramIndex];
    return vx_true_e;
}

/* calculate image valid rectangle through callback */
VX_PRIVATE_API vx_bool vxoGraph_SetImagValidRect(vx_node node, vx_reference paramRef, vx_uint32 paramIndex, vx_meta_format metaFormat)
{
    vx_image img = (vx_image)paramRef;

    if (NULL != metaFormat->setValidRectangleCallback)
        node->kernelAttributes.validRectReset = vx_false_e;

    if (vx_false_e == node->kernelAttributes.validRectReset &&
        NULL != metaFormat->setValidRectangleCallback)
    {
        vx_uint32 i;
        vx_uint32 nparams = 0;
        vx_uint32 num_in_images = 0;
        vx_rectangle_t** in_rect = NULL;
        vx_rectangle_t* out_rect[1] = { NULL };
        vx_bool res = vx_true_e;

        if (VX_SUCCESS != vxQueryNode(node, VX_NODE_PARAMETERS, &nparams, sizeof(nparams)))
            return vx_false_e;

        /* get the num of input images */
        for (i = 0; i < nparams; i++)
        {
            if (VX_INPUT == node->kernel->signature.directionTable[i] &&
                VX_TYPE_IMAGE == node->paramTable[i]->type)
                num_in_images++;
        }
        in_rect = (vx_rectangle_t**)vxAllocateAndZeroMemory(num_in_images * sizeof(vx_rectangle_t*));
        if (NULL == in_rect)
        {
            return vx_false_e;
        }

        for (i = 0; i < nparams; i++)
        {
            if (VX_INPUT == node->kernel->signature.directionTable[i] &&
                VX_TYPE_IMAGE == node->paramTable[i]->type)
            {
                in_rect[i] = (vx_rectangle_t*)vxAllocateAndZeroMemory(sizeof(vx_rectangle_t));
                if (NULL == in_rect[i])
                {
                    res = vx_false_e;
                    break;
                }
                /* collect input images valid rectagles in array */
                if (VX_SUCCESS != vxGetValidRegionImage((vx_image)node->paramTable[i], in_rect[i]))
                {
                    res = vx_false_e;
                    break;
                }

            }
        }

        if (vx_false_e != res)
        {
            out_rect[0] = (vx_rectangle_t*)vxAllocateAndZeroMemory(sizeof(vx_rectangle_t));
            if (NULL == out_rect[0])
                res = vx_false_e;

            if (VX_SUCCESS == metaFormat->setValidRectangleCallback(node, paramIndex, (const vx_rectangle_t* const*)in_rect,
                (vx_rectangle_t* const*)out_rect))
            {
                /* set output image valid rectangle */
                if (VX_SUCCESS != vxSetImageValidRectangle(img, (const vx_rectangle_t*)out_rect[0]))
                    res = vx_false_e;
            }

            else
                res = vx_false_e;

            vxFree(out_rect[0]);
        }

        /* deallocate arrays memory */
        for (i = 0; i < num_in_images; i++)
        {
            if (NULL != in_rect[i])
                vxFree(in_rect[i]);
        }

        if (NULL != in_rect)
            vxFree(in_rect);

        return res;
    }

    if (vx_true_e == node->kernelAttributes.validRectReset)
    {
        /* reset image valid rectangle */
        vx_rectangle_t out_rect;
        out_rect.start_x = 0;
        out_rect.start_y = 0;
        out_rect.end_x   = img->width;
        out_rect.end_y   = img->height;

        if (VX_SUCCESS != vxSetImageValidRectangle(img, &out_rect))
            return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_status vxoGraph_PostMetaFormatData(vx_node node, vx_reference paramRef, vx_uint32 paramIndex, vx_reference_s **vRef, vx_meta_format metaFormat)
{

    switch (metaFormat->type)
    {
    case VX_TYPE_IMAGE:
        if ((vRef != NULL) && (paramRef == *vRef))
        {
            vx_image image = (vx_image)paramRef;

            if (image->format != metaFormat->u.imageInfo.format && image->format != VX_DF_IMAGE_VIRT)
            {
                vxError("Node %p(\"%s\"): No.%d image parameter's format, %d, is invalid (expected: %d)",
                    node, node->kernel->name, paramIndex, image->format, metaFormat->u.imageInfo.format);
                return VX_ERROR_INVALID_FORMAT;
            }

            image->format   = metaFormat->u.imageInfo.format;
            image->width    = metaFormat->u.imageInfo.width;
            image->height   = metaFormat->u.imageInfo.height;

            vxoImage_Initialize(image, image->width, image->height, image->format);

            vxoImage_Dump(image);
        }
        else
            vxoGraph_SetImagValidRect(node, paramRef, paramIndex, metaFormat);
        break;

    case VX_TYPE_PYRAMID:
        {
            vx_pyramid pyramid = (vx_pyramid)paramRef;

            if (pyramid->levelCount != metaFormat->u.pyramidInfo.levelCount)
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter has invalid levels, %d (expected: %d)",
                    node, node->kernel->name, paramIndex,
                    pyramid->levelCount, metaFormat->u.pyramidInfo.levelCount);
                return VX_ERROR_INVALID_VALUE;
            }

            if (pyramid->scale != metaFormat->u.pyramidInfo.scale)
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter has invalid scale, %f (expected: %f)",
                    node, node->kernel->name, paramIndex,
                    pyramid->scale, metaFormat->u.pyramidInfo.scale);
                return VX_ERROR_INVALID_VALUE;
            }

            if (pyramid->format != VX_DF_IMAGE_VIRT && pyramid->format != metaFormat->u.pyramidInfo.format)
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter's format, %d, is invalid (expected: %d)",
                    node, node->kernel->name, paramIndex,
                    pyramid->format, metaFormat->u.pyramidInfo.format);
                return VX_ERROR_INVALID_FORMAT;
            }

            if ((pyramid->width != 0 && pyramid->width != metaFormat->u.pyramidInfo.width)
                || (pyramid->height != 0 && pyramid->height != metaFormat->u.pyramidInfo.height))
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter's width and height, %dx%d"
                    ", is invalid (expected: %dx%d)",
                    node, node->kernel->name, paramIndex, pyramid->width, pyramid->height,
                    metaFormat->u.pyramidInfo.width, metaFormat->u.pyramidInfo.height);
                return VX_ERROR_INVALID_DIMENSION;
            }

            if (pyramid->base.isVirtual)
            {
                vxoPyramid_Initialize(pyramid, pyramid->levelCount, pyramid->scale, metaFormat->u.pyramidInfo.width,
                    metaFormat->u.pyramidInfo.height, metaFormat->u.pyramidInfo.format);
            }
        }
        break;

    case VX_TYPE_SCALAR:
        {
            vx_scalar scalar = (vx_scalar)paramRef;

            if (scalar->dataType != metaFormat->u.scalarInfo.type)
            {
                vxError("Node %p(\"%s\"): No.%d scalar parameter's type, %d, is invalid (expected: %d)",
                    node, node->kernel->name, paramIndex,
                    scalar->dataType, metaFormat->u.scalarInfo.type);
                return VX_ERROR_INVALID_TYPE;
            }
        }
        break;

    case VX_TYPE_ARRAY:
        {
            vx_array array = (vx_array)paramRef;

            if (array->base.isVirtual)
            {
                if (!vxoArray_InitializeAsVirtual(array, metaFormat->u.arrayInfo.itemType,
                    metaFormat->u.arrayInfo.capacity))
                {
                    vxError("Node %p(\"%s\"): No.%d virtual array parameter: "
                        "the corresponding metaFormat format's item type and/or capacity, %d/%d, is invalid",
                        node, node->kernel->name, paramIndex,
                        metaFormat->u.arrayInfo.itemType, metaFormat->u.arrayInfo.capacity);
                    return VX_ERROR_INVALID_DIMENSION;
                }
            }
            else
            {
                if (!vxoArray_CheckItemTypeAndCapacity(array, metaFormat->u.arrayInfo.itemType,
                    metaFormat->u.arrayInfo.capacity))
                {
                    vxError("Node %p(\"%s\"): No.%d array parameter's item type and/or capacity"
                        ", %d/%d, is invalid (expected: %d/%d)",
                        node, node->kernel->name, paramIndex,
                        array->itemType, array->capacity,
                        metaFormat->u.arrayInfo.itemType, metaFormat->u.arrayInfo.capacity);
                    return VX_ERROR_INVALID_DIMENSION;
                }
            }
        }
        break;

    case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = (vx_matrix)paramRef;
            if (matrix->dataType != metaFormat->u.matrixInfo.type)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_TYPE,
                    "Node: %s: parameter[%u] has an invalid data type 0x%08x\n",
                    node->kernel->name, paramIndex, matrix->dataType);

                return VX_ERROR_INVALID_TYPE;
            }

            if (matrix->columns != metaFormat->u.matrixInfo.cols || matrix->rows != metaFormat->u.matrixInfo.rows)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid matrix dimention %ux%u\n",
                    node->kernel->name, paramIndex, matrix->dataType, matrix->rows, matrix->columns);

                return VX_ERROR_INVALID_DIMENSION;
            }
        }
        break;

    case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = (vx_distribution)paramRef;

            if (distribution->offsetX != metaFormat->u.distributionInfo.offset ||
                distribution->rangeX != metaFormat->u.distributionInfo.range ||
                (vx_size)distribution->memory.dims[0][VX_DIM_X] != metaFormat->u.distributionInfo.bins)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_VALUE,
                    "Node: %s: parameter[%u] has an invalid offset %u, number of bins %u or range %u\n",
                    node->kernel->name, paramIndex, distribution->offsetX,
                    distribution->memory.dims[0][VX_DIM_X], distribution->rangeX);

                return VX_ERROR_INVALID_VALUE;
            }
        }
        break;

    case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)paramRef;
            if (remap->srcWidth != metaFormat->u.remapInfo.src_width || remap->srcWidth != metaFormat->u.remapInfo.src_height)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid source dimention %ux%u\n",
                    node->kernel->name, paramIndex);

                return VX_ERROR_INVALID_DIMENSION;
            }

            if (remap->destWidth != metaFormat->u.remapInfo.dst_width || remap->destHeight != metaFormat->u.remapInfo.dst_height)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid dest dimention %ux%u\n",
                    node->kernel->name, paramIndex);

                return VX_ERROR_INVALID_DIMENSION;
            }
        }
        break;

    case VX_TYPE_LUT:
        {
            vx_lut_s *lut = (vx_lut_s *)paramRef;
            if (lut->itemType != metaFormat->u.lutInfo.type || lut->itemCount != metaFormat->u.lutInfo.count)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid item type 0x%08x or count "VX_FMT_SIZE"\n",
                    node->kernel->name, paramIndex, lut->itemType, lut->itemCount);

                return VX_ERROR_INVALID_DIMENSION;
            }
        }
        break;

    case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = (vx_threshold)paramRef;
            if (threshold->thresholdType != metaFormat->u.thresholdInfo.type)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_TYPE,
                    "Threshold contains invalid typed objects for node %s\n", node->kernel->name);

                return VX_ERROR_INVALID_TYPE;
            }
        }
        break;

    case VX_TYPE_OBJECT_ARRAY:
        {
            vx_object_array_s *objarr = (vx_object_array_s *)paramRef;
            vxInfo("meta: type 0x%08x, 0x%08x "VX_FMT_SIZE"\n", metaFormat->type, metaFormat->u.objectArrayInfo.item_type, metaFormat->u.objectArrayInfo.item_count);

            if (vxoOA_ValidateObjectArray(objarr, metaFormat->u.objectArrayInfo.item_type, metaFormat->u.objectArrayInfo.item_type) != vx_true_e)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid item type 0x%08x or num_items "VX_FMT_SIZE"\n",
                    node->kernel->name, paramIndex, objarr->itemType, objarr->itemCount);

                return VX_ERROR_INVALID_DIMENSION;
            }

            if (*vRef == (vx_reference_s *)objarr)
            {
                vx_int32 i = 0;

                for (i = 0; i < metaFormat->u.objectArrayInfo.item_type; i++)
                {
                    vx_reference item = vxGetObjectArrayItem(objarr, i);

                    if (!vxoGraph_PostMetaFormatData(node, item, paramIndex, vRef, metaFormat))
                    {
                        vxReleaseReference(&item);
                        vxAddLogEntry(&node->base, VX_ERROR_INVALID_PARAMETERS,
                            "Node: %s: meta[%u] has an invalid meta of exemplar\n",
                            node->kernel->name, paramIndex);

                        return VX_ERROR_INVALID_PARAMETERS;
                    }

                    vxReleaseReference(&item);
                }
            }
            else
            {
                vx_int32 i = 0;
                /* check the data that came back from the output validator against the object */
                for (i = 0; i < metaFormat->u.objectArrayInfo.item_type; i++)
                {
                    vx_reference item = vxGetObjectArrayItem(objarr, i);
                    vx_reference itemref = vxGetObjectArrayItem((vx_object_array)*vRef, i);

                    if (!vxoGraph_PostMetaFormatData(node, item, paramIndex, &itemref, metaFormat))
                    {
                        vxReleaseReference(&item);
                        vxAddLogEntry(&node->base, VX_ERROR_INVALID_PARAMETERS,
                            "Node: %s: meta[%u] has an invalid meta of exemplar\n",
                            node->kernel->name, paramIndex);

                        return VX_ERROR_INVALID_PARAMETERS;
                    }

                    vxReleaseReference(&item);
                }
            }
        }
        break;

    case VX_TYPE_TENSOR:
        break;

    case VX_TYPE_REFERENCE:
        break;

    default:
        vxAddLogEntry(&node->base, VX_ERROR_INVALID_TYPE,
            "The format of meta is invalid objects %s for node: %s\n", metaFormat->type, node->kernel->name);
        return VX_ERROR_INVALID_FORMAT;
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status vxoGraph_UserKernelPreprocess(vx_graph graph, vx_bool first)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        if (node->kernel->isUserkernel)
        {
            if (!first)
            {
                if (node->kernel->deinitializeFunction != VX_NULL)
                {
                    vx_status status = VX_SUCCESS;

                    if (node->localDataSetByImplementation == vx_false_e)
                        node->localDataChangeIsEnabled = vx_true_e;

                    status = node->kernel->deinitializeFunction(
                        node, node->paramTable, node->kernel->signature.paramCount);

                    node->localDataChangeIsEnabled = vx_false_e;

                    if (status != VX_SUCCESS)
                    {
                        vxError("Failed to deinitialize Kernel \"%s\" of Node %p (status = %d)",
                            node->kernel->name, node, status);
                        return status;
                    }
                }

                if (node->kernelAttributes.localDataSize > 0 && node->kernelAttributes.localDataPtr == VX_NULL)
                {
                    if (node->kernelAttributes.localDataPtr)
                    {
                        vxFree(node->kernelAttributes.localDataPtr);
                        node->kernelAttributes.localDataSize = 0;
                        node->kernelAttributes.localDataPtr = VX_NULL;
                    }

                }

                node->localDataSetByImplementation = vx_false_e;

                if (node->layer)
                {
                    vxnneLayer_Free(node->layer);
                    node->layer = VX_NULL;
                }
            }
        }
    }

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status vxoGraph_VerifyAllNodeParameters(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++)
    {
        vx_status status = VX_SUCCESS;
        vx_meta_format metaFormat = VX_NULL;
        vx_node node = graph->nodeTable[nodeIndex];

        /* graph binary kernel sanity check*/
        if ((node->kernel->enumeration == VX_KERNEL_IMPORT_FROM_FILE) && (nodeIndex > 0))
        {
            vxError("binary kernel only one node in graph, nodeCount: %d\n", graph->nodeCount);
            return VX_ERROR_NOT_SUPPORTED;
        }

        /* Check if all required parametes are supplied */
        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            switch (node->kernel->signature.stateTable[paramIndex])
            {
            case VX_PARAMETER_STATE_REQUIRED:
                if (paramRef == VX_NULL)
                {
                    vxError("Node %p(\"%s\"): No.%d parameter is required but not supplied",
                        node, node->kernel->name, paramIndex);
                    status = VX_ERROR_NOT_SUFFICIENT;
                    continue;
                }

                vxmASSERT(vxoReference_GetInternalCount(paramRef) != 0);
                break;

            case VX_PARAMETER_STATE_OPTIONAL:
                break;

            default:
                vxmASSERT(0);
                break;
            }
        }

        if (status != VX_SUCCESS) return status;
        /*if user kernel, use validateFunction to check input and output*/
        if (node->kernel->validateFunction != VX_NULL)
        {
            vx_status validationStatus = VX_SUCCESS;
            vx_meta_format metaFormat[VX_MAX_PARAMETERS];
            vx_uint32 index;
            vx_uint32 paramIndex;
            vx_reference_s *vRef = NULL;

            for (index = 0; index < vxmLENGTH_OF(metaFormat); index++)
            {
                metaFormat[index] = NULL;
            }

            for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
            {
                if ((node->paramTable[paramIndex] != NULL) &&
                    (node->kernel->signature.directionTable[paramIndex] == VX_OUTPUT))
                {
                    vxoGraph_InitMetaFormatData(graph, node, paramIndex, &vRef, &metaFormat[paramIndex], &status);
                }
            }

            if (status == VX_SUCCESS)
            {
                validationStatus = node->kernel->validateFunction(node,
                    node->paramTable,
                    node->kernel->signature.paramCount,
                    metaFormat);
                if (validationStatus != VX_SUCCESS)
                {
                    status = validationStatus;
                    vxError("Node %p(\"%s\"): No.%d parameter output validation failure (status = %d)",
                        node, node->kernel->name, paramIndex, validationStatus);
                }
            }

            if (status == VX_SUCCESS)
            {
                for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
                {
                    if ((node->paramTable[paramIndex] != NULL) &&
                        (node->kernel->signature.directionTable[paramIndex] == VX_OUTPUT))
                    {
                        if (vxoGraph_PostMetaFormatData(node, node->paramTable[paramIndex], paramIndex, &vRef, metaFormat[paramIndex]) != VX_SUCCESS)
                            break;
                    }
                }
            }

            for (index = 0; index < vxmLENGTH_OF(metaFormat); index++)
            {
                if (metaFormat[index])  vxoMetaFormat_Release(&metaFormat[index]);
            }
        }
        else
        {

            /* Validate all input/bidirectional parameters */
            for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
            {
                vx_status validationStatus;
                vx_reference paramRef = node->paramTable[paramIndex];

                if (paramRef == VX_NULL) continue;

                if (!vxmIS_INPUT_OR_BIDIRECTION(node->kernel->signature.directionTable[paramIndex])) continue;

                if (node->kernel->validateFunction == VX_NULL)
                {
                    vxmASSERT(node->kernel->inputValidateFunction);

                    validationStatus = node->kernel->inputValidateFunction(node, paramIndex);

                    if (validationStatus != VX_SUCCESS)
                    {
                        vxError("Node %p(\"%s\"): No.%d parameter input validation failure (status = %d)",
                                node, node->kernel->name, paramIndex, validationStatus);
                        status = validationStatus;
                        continue;
                    }
                }
            }


            if (status != VX_SUCCESS) return status;

            /* Validate all output/bidirectional parameters */
            for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
            {
                vx_status validationStatus;
                vx_reference paramRef = node->paramTable[paramIndex];
                    vx_reference vRef = node->paramTable[paramIndex];


                if (paramRef == VX_NULL) continue;

                if (node->kernel->signature.directionTable[paramIndex] != VX_OUTPUT) continue;

                if (paramRef->isVirtual)
                {
                    if (!vxoGraph_IsParentGraphScope(paramRef->scope, graph))
                    {
                        vxError("Node %p(\"%s\") in Graph %p: No.%d virtual parameter has an invalid scope, %p",
                                node, node->kernel->name, graph, paramIndex, paramRef->scope);
                        status = VX_ERROR_INVALID_SCOPE;
                        continue;
                    }
                }
                else
                {
                    vRef = NULL;
                }

                if (metaFormat != VX_NULL) vxoMetaFormat_Release(&metaFormat);

                metaFormat = vxoMetaFormat_Create(graph->base.context);

                if (vxoReference_GetStatus((vx_reference)metaFormat) != VX_SUCCESS)
                {
                    status = vxoReference_GetStatus((vx_reference)metaFormat);
                    continue;
                }

                metaFormat->type = node->kernel->signature.dataTypeTable[paramIndex];

                vxmASSERT(node->kernel->outputValidateFunction);

                validationStatus = node->kernel->outputValidateFunction(node, paramIndex, metaFormat);

                if (validationStatus != VX_SUCCESS)
                {
                    vxError("Node %p(\"%s\"): No.%d parameter output validation failure (status = %d)",
                                node, node->kernel->name, paramIndex, validationStatus);
                    status = validationStatus;
                    continue;
                }

                if (!vxDataType_IsValid(metaFormat->type))
                {
                    vxError("Node %p(\"%s\"): No.%d parameter's type, %d, is invalid",
                                node, node->kernel->name, paramIndex, metaFormat->type);
                    status = VX_ERROR_INVALID_TYPE;
                    continue;
                }


                    /*set metaformat data to paramRef*/
                    status = vxoGraph_PostMetaFormatData(node, paramRef, paramIndex, &vRef, metaFormat);
            }  /* for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++) */

            if (metaFormat != VX_NULL) vxoMetaFormat_Release(&metaFormat);
        }

        if (node->kernel->enabled)
        {
            node->numParameters = node->kernel->signature.paramCount;
        }
        if (status != VX_SUCCESS) return status;
    } /* for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++) */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_VerifyAllNodeWriteDependencies(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];
            vx_uint32 nodeIndex2, paramIndex2;

            if (paramRef == VX_NULL) continue;

            if (!vxmIS_OUTPUT_OR_BIDIRECTION(node->kernel->signature.directionTable[paramIndex])) continue;

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (!vxmIS_OUTPUT_OR_BIDIRECTION(node2->kernel->signature.directionTable[paramIndex2])) continue;

                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        vxError("Node %p and Node %p have the same output reference, %p",
                            node, node2, paramRef);
                        status = VX_ERROR_MULTIPLE_WRITERS;
                    }
                }
            }
        }

        if (status != VX_SUCCESS) return status;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_AllocateAllMemoryObjects(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            if (paramRef == VX_NULL) continue;

            switch (paramRef->type)
            {
            case VX_TYPE_ARRAY:
                if (!vxoArray_AllocateMemory((vx_array)paramRef))
                {
                    vxError("Node %p(\"%s\"): Can't allocate memory for No.%d array parameter",
                        node, node->kernel->name, paramIndex);
                    status = VX_ERROR_NO_MEMORY;
                }
                break;

            case VX_TYPE_LUT:
                {
                    vx_lut_s * lut = (vx_lut_s *)paramRef;

                    if (!vxoMemory_Allocate(graph->base.context, &lut->memory))
                    {
                        vxError("Node %p(\"%s\"): Can't allocate memory for No.%d LUT parameter",
                            node, node->kernel->name, paramIndex);
                        status = VX_ERROR_NO_MEMORY;
                    }
                }
                break;

            case VX_TYPE_DISTRIBUTION:
                {
                    vx_distribution_s * distribution = (vx_distribution_s *)paramRef;
                    if (!vxoMemory_Allocate(graph->base.context, &distribution->memory))
                    {
                        vxError("Node %p(\"%s\"): Can't allocate memory for No.%d distribution parameter",
                            node, node->kernel->name, paramIndex);
                        status = VX_ERROR_NO_MEMORY;
                    }
                }
                break;

            case VX_TYPE_MATRIX:
            case VX_TYPE_CONVOLUTION:
                {
                    vx_matrix matrix = (vx_matrix)paramRef;

                    if (!vxoMemory_Allocate(graph->base.context, &matrix->memory))
                    {
                        vxError("Node %p(\"%s\"): Can't allocate memory for No.%d %s parameter",
                            node, node->kernel->name, paramIndex,
                            (paramRef->type == VX_TYPE_CONVOLUTION)? "convolution" : "matrix");
                        status = VX_ERROR_NO_MEMORY;
                    }
                }
                break;

            case VX_TYPE_IMAGE:
                if (!vxoImage_AllocateMemory((vx_image)paramRef))
                {
                    vxInfo("Node %p(\"%s\"): Don't need to allocate memory for No.%d image parameter",
                        node, node->kernel->name, paramIndex);
                    /*status = VX_ERROR_NO_MEMORY;*/
                }
                break;

            case VX_TYPE_PYRAMID:
                {
                    vx_uint32 i;
                    vx_pyramid pyramid = (vx_pyramid)paramRef;

                    for (i = 0; i < pyramid->levelCount; i++)
                    {
                        if (!vxoImage_AllocateMemory((vx_image)pyramid->levels[i]))
                        {
                            vxError("Node %p(\"%s\"): Can't allocate memory for No.%d pyramid parameter",
                                node, node->kernel->name, paramIndex);
                            status = VX_ERROR_NO_MEMORY;
                        }
                    }
                }
                break;

            default:
                /* Do nothing */
                break;
            }

        } /* for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++) */

        if (status != VX_SUCCESS) return status;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_getParentNodes(vx_node node, vx_uint32 *num_parents, vx_uint32 **parents)
{
    vxmASSERT(node);
    *num_parents = node->numParents;
    *parents = node->parentNodes;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_getChildNodes(vx_node node, vx_uint32 *num_children, vx_uint32 **children)
{
    vxmASSERT(node);
    *num_children = node->numChildren;
    *children = node->childNodes;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_RetrieveTopology(vx_graph graph)
{
    vx_uint32 nodeIndex1 = 0;
    vx_uint32 nodeIndex2 = 0;
    vx_uint32 paramIndex1 = 0;
    vx_uint32 paramIndex2 = 0;
    vx_reference paramRef1 = VX_NULL;
    vx_reference paramRef2 = VX_NULL;

    vxmASSERT(graph);

    for (nodeIndex1 = 0; nodeIndex1 < graph->nodeCount; nodeIndex1++)
    {
        vx_node node = graph->nodeTable[nodeIndex1];
        node->numParents = 0;
        node->numChildren = 0;
        memset(node->childNodes, 0, sizeof(node->childNodes));
        memset(node->parentNodes, 0, sizeof(node->parentNodes));
    }

    for (nodeIndex1 = 0; nodeIndex1 < graph->nodeCount; nodeIndex1++)
    {
        vx_node node1 = graph->nodeTable[nodeIndex1];
        node1->numChildren = 0;

        for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex1);
             nodeIndex2 != nodeIndex1;
             nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
        {
            vx_node node2 = graph->nodeTable[nodeIndex2];

            vx_bool hasRelationship = vx_false_e;

            for (paramIndex1 = 0; paramIndex1 < node1->kernel->signature.paramCount && !hasRelationship; paramIndex1++)
            {
                if (!vxmIS_OUTPUT_OR_BIDIRECTION(node1->kernel->signature.directionTable[paramIndex1])) continue;

                paramRef1 = node1->paramTable[paramIndex1];

                if (paramRef1 == VX_NULL) continue;

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount && !hasRelationship; paramIndex2++)
                {
                    if (vxmIS_OUTPUT_OR_BIDIRECTION(node2->kernel->signature.directionTable[paramIndex2])) continue;

                    paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (vxoReference_HasWriteDependency(paramRef1, paramRef2))
                    {
                        hasRelationship = vx_true_e;

                        node1->childNodes[node1->numChildren] = nodeIndex2;
                        node1->numChildren++;
                        vxmASSERT(node1->numChildren <= VX_MAX_NODE_CHILDREN);

                        node2->parentNodes[node2->numParents] = nodeIndex1;
                        node2->numParents++;
                        vxmASSERT(node2->numParents <= VX_MAX_NODE_PARENTS);
                    }
                }
            }
        }
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_DetectAllHeadNodes(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    graph->headNodeCount = 0;
    vxZeroMemory(graph->headNodeIndexTable, sizeof(graph->headNodeIndexTable));

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool isHeadNode = vx_true_e;
        vx_node node = graph->nodeTable[nodeIndex];
        uint32_t nodeIndex2, paramIndex2;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            if (paramRef == VX_NULL) continue;

            if (node->kernel->signature.directionTable[paramIndex] != VX_INPUT) continue;

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (node2->kernel->signature.directionTable[paramIndex2] == VX_INPUT) continue;

                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        isHeadNode = vx_false_e;
                        break;
                    }
                }

                if (!isHeadNode) break;
            }

            if (!isHeadNode) break;
        }

        if (isHeadNode)
        {
            vxTrace(VX_TRACE_GRAPH, "Node %p (\"%s\") is a head node of Graph %p", node, node->kernel->name, graph);

            graph->headNodeIndexTable[graph->headNodeCount] = nodeIndex;
            graph->headNodeCount++;
        }
    }

    if (graph->nodeCount != 0 && graph->headNodeCount == 0)
    {
        vxError("Graph %p has no head node", graph);
        return VX_ERROR_INVALID_GRAPH;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_DetectAllTailNodes(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    graph->tailNodeCount = 0;
    vxZeroMemory(graph->tailNodeIndexTable, sizeof(graph->tailNodeIndexTable));

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_bool istailNode = vx_true_e;
        vx_node node = graph->nodeTable[nodeIndex];
        uint32_t nodeIndex2, paramIndex2;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            if (paramRef == VX_NULL) continue;

            if (!vxmIS_OUTPUT_OR_BIDIRECTION(node->kernel->signature.directionTable[paramIndex])) continue;

            for (nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex);
                nodeIndex2 != nodeIndex;
                nodeIndex2 = vxoGraph_GetNextNodeIndex(graph, nodeIndex2))
            {
                vx_node node2 = graph->nodeTable[nodeIndex2];

                for (paramIndex2 = 0; paramIndex2 < node2->kernel->signature.paramCount; paramIndex2++)
                {
                    vx_reference paramRef2 = node2->paramTable[paramIndex2];

                    if (paramRef2 == VX_NULL) continue;

                    if (vxmIS_OUTPUT_OR_BIDIRECTION(node2->kernel->signature.directionTable[paramIndex2])) continue;

                    if (vxoReference_HasWriteDependency(paramRef, paramRef2))
                    {
                        istailNode = vx_false_e;
                        break;
                    }
                }

                if (!istailNode) break;
            }

            if (!istailNode) break;
        }

        if (istailNode)
        {
            vxTrace(VX_TRACE_GRAPH, "Node %p (\"%s\") is a tail node of Graph %p", node, node->kernel->name, graph);

            graph->tailNodeIndexTable[graph->tailNodeCount] = nodeIndex;
            graph->tailNodeCount++;
        }
    }

    if (graph->nodeCount != 0 && graph->tailNodeCount == 0)
    {
        vxError("Graph %p has no tail node", graph);
        return VX_ERROR_INVALID_GRAPH;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoGraph_OptCheckDependency(vx_graph graph, vx_node node, vx_reference param, vx_enum type)
{
    vx_bool in_output = vx_true_e;

    vx_uint32 i = 0;

    for (i = 0; i < graph->nodeCount; i ++)
    {
        vx_uint32 j = 0;
        vx_node n = graph->nodeTable[i];

        if (n == node)
            continue;

        for (j = 0; j < n->kernel->signature.paramCount; j ++)
        {
            vx_uint32 k = 0, count = 1;
            vx_reference p = n->paramTable[j], pc;

            if (!p)
            {
                continue;
            }

            pc = p;

            if (p->type == VX_TYPE_OBJECT_ARRAY)
            {
                count = (vx_uint32)((vx_object_array)p)->itemCount;
            }

            for (k = 0; k < count; k++)
            {
                if (p->type == VX_TYPE_OBJECT_ARRAY)
                {
                    if (in_output == vx_false_e)
                        break;

                    pc = ((vx_object_array)p)->itemsTable[k];
                }
                if (pc == param)
                {
                    /*If param is output os other node, we make sure it's not input paramenter */
                    if ((type == VX_INPUT) && ((n->kernel->signature.directionTable[j] == VX_OUTPUT)
                        || (n->kernel->signature.directionTable[j] == VX_BIDIRECTIONAL))
                        )
                    {
                        in_output = vx_false_e;
                    }
                    else if ((type == VX_OUTPUT) && (n->kernel->signature.directionTable[j] == VX_INPUT))
                    {
                        in_output = vx_false_e;
                    }

                }

            }
        }
    }
    return in_output;
}


typedef struct _vx_internal_convert
{
    vx_int32 index;
    vx_node node;
}

vx_internal_convert;


VX_PRIVATE_API vx_int32 vxoGraph_CountTensor(vx_graph graph)
{
    vx_int32 count = 0;
    vx_uint32 i = 0;

    for (i = 0; i < graph->nodeCount; i ++)
    {
        vx_uint32 j = 0;
        vx_node node = graph->nodeTable[i];

        for (j = 0; j < node->kernel->signature.paramCount; j ++)
        {
            vx_reference param = node->paramTable[j];

            if (param != VX_NULL)
            {
                if (param->type == VX_TYPE_TENSOR)
                    count++;
                else if (param->type == VX_TYPE_OBJECT_ARRAY && ((vx_object_array)param)->itemsTable[0]->type == VX_TYPE_TENSOR)
                    count ++;
            }
        }
    }

    return count;
}

VX_PRIVATE_API vx_status vxAddHeadTensorToGraph(vx_graph graph, vx_parameter param)
{
    if (!vxoReference_IsValidAndNoncontext(&graph->base)) return VX_ERROR_INVALID_REFERENCE;

    if (graph->headTensorCount == 0)
    {
        gcoOS_Allocate(VX_NULL, vxoGraph_CountTensor(graph) * sizeof(vx_graph_parameter_s), (vx_ptr_ptr)&graph->headTensorCountTable);
    }

    if (vxoReference_IsValidAndSpecific(&param->base, VX_TYPE_PARAMETER))
    {
        graph->headTensorCountTable[graph->headTensorCount].node = param->node;
        graph->headTensorCountTable[graph->headTensorCount].index = param->index;
    }
    else
    {
        graph->headTensorCountTable[graph->headTensorCount].node = VX_NULL;
        graph->headTensorCountTable[graph->headTensorCount].index = 0;
    }

    graph->headTensorCount++;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_AdapterFindParams(vx_graph graph, vx_internal_convert * internal_convert_lists, vx_uint32_ptr internal_convert_count)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;

    for (i = 0; i < graph->nodeCount; i ++)
    {
        vx_uint32 j = 0;
        vx_node node = graph->nodeTable[i];

        if (node->kernel->isUserkernel)
            continue;

        for (j = 0; j < node->kernel->signature.paramCount; j ++)
        {
            vx_tensor tensor = VX_NULL;
            vx_reference param = node->paramTable[j];
            if (param == VX_NULL)
                continue;

            if (param->type == VX_TYPE_TENSOR)
            {
                tensor = (vx_tensor)param;
            }
            else if (param->type == VX_TYPE_OBJECT_ARRAY)
            {
                vx_object_array arrays = (vx_object_array)param;
                if (arrays->itemType == VX_TYPE_TENSOR)
                    tensor = (vx_tensor)arrays->itemsTable[0];
            }

            if (tensor != VX_NULL)
            {
                if (vxoGraph_OptCheckDependency(graph, node, param, node->kernel->signature.directionTable[j]))
                {
                     vx_parameter p;
                    if (TENSOR_RANK(tensor) == VX_TENSOR_RANK_SN || (TENSOR_RANK(tensor) == VX_TENSOR_RANK_CWHN && tensor->dimCount != 4))
                    {
                        vx_internal_convert convert = {j, node};

                        internal_convert_lists[(*internal_convert_count) ++] = convert;
                    }

                    p = vxoNode_GetParameter(node, j);
                    status |= vxAddHeadTensorToGraph(graph, p);
                    if(p) vxReleaseParameter(&p);
                }
                else if (TENSOR_RANK(tensor) == VX_TENSOR_RANK_CWHN || (TENSOR_RANK(tensor) == VX_TENSOR_RANK_SN && TENSOR_PRECISION(tensor) == VX_TENSOR_PRECISION_AUTO))
                {
                    vx_internal_convert convert = {j, node};
                    internal_convert_lists[(*internal_convert_count) ++] = convert;
                }

            }
        }
    }

    return status;
}

extern vx_status vxoNode_ConvertDims(vx_int32_ptr dims, vx_uint32_ptr org_dims, vx_uint32 count, vx_bool dimsto4);

VX_PRIVATE_API vx_status vxoGraph_Adapter(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_bool opt = vx_false_e;
    vx_uint32_ptr count = &graph->headTensorCount;/*&graph->paramCount;*/
    vx_uint32 i = 0;
    vx_uint32 internal_convert_count = 0;
    vx_internal_convert * internal_convert_lists = (vx_internal_convert *)vxAllocateAndZeroMemory(sizeof(vx_internal_convert) * VX_MAX_PARAMETERS * graph->nodeCount);

    vxmASSERT(graph);

    if (*count == 0)
    {
        vxoGraph_AdapterFindParams(graph, internal_convert_lists, &internal_convert_count);
    }

    for (i = 0; i < *count; i ++)
    {
        vx_graph_parameter_s param = graph->headTensorCountTable[i];

        if (vxoNode_Adapter(graph, param.node, param.index))
            opt = vx_true_e;
    }

    if (graph->headTensorCountTable != VX_NULL)
    {
        gcoOS_Free(VX_NULL, graph->headTensorCountTable);
        graph->headTensorCount = 0;
        graph->headTensorCountTable = VX_NULL;
    }

    if (internal_convert_count > 0)
    {
        for (i = 0; i < internal_convert_count; i++)
        {
            vx_int32 dims[4] = {0}, subcount = 1;
            vx_node node = internal_convert_lists[i].node;
            vx_int32 index = internal_convert_lists[i].index;
            vx_reference param = node->paramTable[index];
            vx_tensor tensor = VX_NULL;

            if (param->type == VX_TYPE_TENSOR)
                tensor = (vx_tensor)param;
            else if (param->type == VX_TYPE_OBJECT_ARRAY)
            {
                vx_object_array arrays = (vx_object_array)param;
                subcount = (vx_int32)arrays->itemCount;
                tensor = (vx_tensor)(arrays->itemsTable[subcount - 1]);
            }

            do
            {

                if (tensor->reshape == VX_NULL)
                {

                    vx_bool dimsto4 = ((node->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER
                        || node->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_LAYER)
                        && (TENSOR_DIM_NUM(tensor) == 2));

                    status = vxoNode_ConvertDims(dims, TENSOR_SIZES(tensor), TENSOR_DIM_NUM(tensor), dimsto4);

                    tensor->reshape = vxoTensor_ReshapeTensor(tensor, dims, dimsto4?4:TENSOR_DIM_NUM(tensor));
                    tensor->reshape->reshape = tensor;
                    TENSOR_DATA_LIFETIME(tensor->reshape) = TENSOR_DATA_LIFETIME(tensor);
                    TENSOR_PRECISION(tensor->reshape) = TENSOR_PRECISION(tensor);


                    status = vxoReference_GetStatus((vx_reference)tensor->reshape);

                }

                if (param->type == VX_TYPE_TENSOR)
                {

                    status = vxoNode_SetParameter(node, index, (vx_reference)tensor->reshape);
                    vxoTensor_ReleaseTensor(&tensor->reshape);
                }
                else if (param->type == VX_TYPE_OBJECT_ARRAY)
                {
                    vxoReference_Increment((vx_reference)tensor->reshape, VX_REF_EXTERNAL);

                    ((vx_object_array)param)->itemsTable[subcount - 1] = (vx_reference)tensor->reshape;
                    if ((subcount - 2) >= 0)tensor = (vx_tensor)(((vx_object_array)param)->itemsTable[subcount - 2]);
                }
            } while (--subcount);
        }

        opt = vx_true_e;
    }

    if (opt)
    {
        status = vxoGraph_VerifyAllNodeParameters(graph);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        status = vxoGraph_VerifyAllNodeWriteDependencies(graph);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        status = vxoGraph_AllocateAllMemoryObjects(graph);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        status = vxoGraph_DetectAllHeadNodes(graph);

        if (status != VX_SUCCESS) {
            goto exit;
        }

        status = vxoGraph_DetectAllTailNodes(graph);
    }

exit:
    if (internal_convert_lists) {
        vxFree(internal_convert_lists);
        internal_convert_lists = VX_NULL;
    }

    return status;
}

VX_PRIVATE_API vx_status vxoGraph_DetectCycle(vx_graph graph)
{
    vx_uint32                   nodeIndex;
    vx_graph_traverse_info_s    traverseInfo;

    vxmASSERT(graph);

    vxoGraph_InitializeTraverseInfo(&traverseInfo);

    vxoGraph_ClearAllVisitedFlags(graph);

    for (nodeIndex = 0; nodeIndex < graph->headNodeCount; nodeIndex++)
    {
        vx_status status = vxoGraph_Traverse(graph, VX_MAX_NODES_IN_GRAPH,
            graph->headNodeIndexTable[nodeIndex], INOUT &traverseInfo);

        if (status != VX_SUCCESS)
        {
            vxError("Graph %p has a cycle", graph);
            return status;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_DetectUnvisitedNodes(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        if (!node->visited)
        {
            vxError("Node %p (\"%s\") is unvisited in Graph %p", node, node->kernel->name, graph);
            return VX_ERROR_INVALID_GRAPH;
        }
    }

    vxoGraph_ClearAllVisitedFlags(graph);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_VerifyAllNodesByTarget(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];
        vx_target target = &graph->base.context->targetTable[node->targetIndex];
        vx_status status;

        if (target == VX_NULL) continue;

        vxmASSERT(target->funcs.verifynode != VX_NULL);

        status = target->funcs.verifynode(target, node);

        if (status != VX_SUCCESS)
        {
            vxError("Failed to verify Node %p (\"%s\") by Target %p (\"%s\"), status = %d",
                node, node->kernel->name, target, target->name, status);
            return status;
        }

        node->id = nodeIndex;
    }

    return VX_SUCCESS;
}

#if defined(_WINDOWS)
#define NNVXC_LIB_NAME "libNNVXCBinary.dll"
#define OVX12_VXC_LIB_NAME "libOvx12VXCBinary.dll"
#define NNGPU_LIB_NAME "libNNGPUBinary.dll"
#else
#define NNVXC_LIB_NAME "libNNVXCBinary.so"
#define OVX12_VXC_LIB_NAME "libOvx12VXCBinary.so"
#define NNGPU_LIB_NAME "libNNGPUBinary.so"
#endif
VX_PRIVATE_API vx_status vxoGraph_InitializeAllNodeKernels(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);

#if gcdUSE_VXC_BINARY
    gceSTATUS status = gcvSTATUS_OK;
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);
    status = gcoOS_LoadLibrary(gcvNULL, NNVXC_LIB_NAME, &context->libNNVXCKernelHandle);
    if(status != gcvSTATUS_OK) vxError("Can't open libNNVXCBinary!\n");

    status = gcoOS_LoadLibrary(gcvNULL, OVX12_VXC_LIB_NAME, &context->libOvx12VXCBinaryHandle);
    if(status != gcvSTATUS_OK) vxError("Can't open libOvx12VXCBinary!\n");

    status = gcoOS_LoadLibrary(gcvNULL, NNGPU_LIB_NAME, &context->libNNGPUKernelHandle);
    if(status != gcvSTATUS_OK) vxError("Can't open libNNGPUBinary!\n");
#endif

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        if (node->kernel->initializeFunction != VX_NULL)
        {
            vx_status status = VX_SUCCESS;

            if (node->kernel->isUserkernel && node->kernelAttributes.localDataSize == 0)
                node->localDataChangeIsEnabled = vx_true_e;

            status = node->kernel->initializeFunction(
                node, node->paramTable, node->kernel->signature.paramCount);

            node->localDataChangeIsEnabled = vx_false_e;

            if (status != VX_SUCCESS)
            {
                vxError("Failed to initialize Kernel \"%s\" of Node %p (status = %d)",
                    node->kernel->name, node, status);
                return status;
            }

            /* Wrap the user node (VXC or CPU node) to operation based one. */
            if (!node->layer)
            {
                if (node->kernel->isUserkernel)
                {
                    if (node->kernel->kernelShaderCount > 0 &&
                         node->kernel->kernelShader)
                    {
                        /* VXC node. */
                        vxnneWrapUserNode(graph->base.context, node, VXNNE_USER_NODE_TYPE_VXC);
                    }
                    else
                    {
                        /* CPU node. */
                        vxnneWrapUserNode(graph->base.context, node, VXNNE_USER_NODE_TYPE_CPU);
                    }
                }
            }
        }

        if (node->kernelAttributes.localDataSize > 0 && node->kernelAttributes.localDataPtr == VX_NULL)
        {
            node->kernelAttributes.localDataPtr = vxAllocate(node->kernelAttributes.localDataSize);

            if (node->kernel->isUserkernel)
                node->localDataSetByImplementation = vx_true_e;
        }

#ifdef OPENVX_KHR_TILING
        if (node->kernelAttributes.tileMemorySize > 0 && node->kernelAttributes.tileMemoryPtr == VX_NULL)
        {
            node->kernelAttributes.tileMemoryPtr = vxAllocate(node->kernelAttributes.tileMemorySize);
        }
#endif

        if (!graph->hasCPUFunction && vxoNode_HasCPUfunction(node))
            graph->hasCPUFunction = vx_true_e;

        if (!graph->hasPrintf &&
            node->kernel->kernelShader &&
            node->kernel->kernelShader[node->kernel->currShaderID]->hasPrintf)
        {
            graph->hasPrintf = vx_true_e;
        }
    }
    graph->Initilized = vx_true_e;

#if gcdUSE_VXC_BINARY
    if(context->libNNVXCKernelHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libNNVXCKernelHandle);

    if(context->libOvx12VXCBinaryHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libOvx12VXCBinaryHandle);

    if(context->libNNGPUKernelHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libNNGPUKernelHandle);
#endif

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_CaculateCostFactors(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;

    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        node->costFactors.bandwidth = 0;

        for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
        {
            vx_reference paramRef = node->paramTable[paramIndex];

            if (paramRef == VX_NULL) continue;

            switch (paramRef->type)
            {
            case VX_TYPE_ARRAY:
                {
                    vx_array array = (vx_array)paramRef;
                    node->costFactors.bandwidth += vxoMemory_ComputeSize(&array->memory, 0);
                }
                break;

            case VX_TYPE_IMAGE:
                {
                    vx_uint32 i;
                    vx_image image = (vx_image)paramRef;

                    for (i = 0; i < image->memory.planeCount; i++)
                    {
                        node->costFactors.bandwidth += vxoMemory_ComputeSize(&image->memory, i);
                    }
                }
                break;

            case VX_TYPE_PYRAMID:
                {
                    vx_uint32   l;
                    vx_uint32   i;
                    vx_pyramid  pyramid = (vx_pyramid)paramRef;

                    for (l = 0; l < pyramid->levelCount; l++)
                    {
                        vx_image image = pyramid->levels[l];

                        for (i = 0; i < image->memory.planeCount; i++)
                        {
                            node->costFactors.bandwidth += vxoMemory_ComputeSize(&image->memory, i);
                        }
                    }
                }
                break;

            default:
                /* Do nothing */
                break;
            }
        }
    }

    return VX_SUCCESS;
}

#if gcdVX_OPTIMIZER
VX_PRIVATE_API void vxCopyNodeIndexTable(
    vx_uint32 srcNodeIndexTable[VX_MAX_NODE_COUNT], vx_uint32 srcNodeCount,
    OUT vx_uint32 destNodeIndexTable[VX_MAX_NODE_COUNT], OUT vx_uint32_ptr destNodeCountPtr)
{
    vx_uint32 i;

    for (i = 0; i < srcNodeCount; i++)
    {
        destNodeIndexTable[i] = srcNodeIndexTable[i];
    }

    *destNodeCountPtr = srcNodeCount;
}

VX_PRIVATE_API vx_status vxoGraph_Flatten(vx_graph graph)
{
    vx_uint32 lastNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 nextNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 leftNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 lastNodeCount, nextNodeCount, leftNodeCount = 0;
    vx_uint32 i;

    vxmASSERT(graph && graph->verified);

    /* Flatten and sequentialize nodes in the graph. */
    vxmASSERT(graph->optimizedNodeCount == 0);

    vxoGraph_ClearAllVisitedFlags(graph);
    vxoGraph_ClearAllExecutedFlags(graph);

    vxCopyNodeIndexTable(graph->headNodeIndexTable, graph->headNodeCount,
        OUT nextNodeIndexTable, OUT &nextNodeCount);

    while (nextNodeCount > 0)
    {
        for (i = 0; i < nextNodeCount; i++)
        {
            vx_node node = graph->nodeTable[nextNodeIndexTable[i]];

            if (node->executed)
            {
                vxError("Node %p in Graph %p was flattened", node, graph);
                continue;
            }

            if (node->childGraph)
            {
                /* Get nodes from childGraph. */
                vx_node_ptr nodes = node->childGraph->optimizedNodeTable;
                vx_uint32 j;

                for (j = 0; j < node->childGraph->optimizedNodeCount; j++)
                {
                    graph->optimizedNodeTable[graph->optimizedNodeCount] = nodes[j];
                    graph->optimizedNodeCount++;
                }
            }
            else
            {
                graph->optimizedNodeTable[graph->optimizedNodeCount] = node;
                graph->optimizedNodeCount++;
            }
            node->visited  = vx_true_e;
            node->executed = vx_true_e;
        }

        vxCopyNodeIndexTable(nextNodeIndexTable, nextNodeCount, OUT lastNodeIndexTable, OUT &lastNodeCount);

        vxoGraph_GenerateNextNodeTable(graph, lastNodeIndexTable, lastNodeCount,
            OUT nextNodeIndexTable, OUT &nextNodeCount,
            INOUT leftNodeIndexTable, INOUT &leftNodeCount);
    }

    vxoGraph_ClearAllVisitedFlags(graph);
    vxoGraph_ClearAllExecutedFlags(graph);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_AddNodeBatch(
    vx_graph graph,
    vx_uint32 startIndex,
    vx_uint32 endIndex,
    vx_bool cpuInvolved,
    vx_bool endHasCallback
    )
{
    vx_status   status = VX_SUCCESS;
    vx_node_batch batch;
    vx_uint32 index;
#if gcdVX_OPTIMIZER > 2
    vx_bool tiled = vx_true_e;
    vx_coordinates2d_t imageSize = { 0, 0 };
    vx_coordinates2d_t tileSize = { 0, 0 };
    vx_uint32 imageCount = 0;
#endif

    vxmASSERT(graph);

    batch = &graph->nodeBatch[graph->nodeBatchCount];
    batch->startIndex = startIndex;
    batch->endIndex = endIndex;
    batch->cpuInvolved = cpuInvolved;
    batch->endHasCallback = endHasCallback;
    graph->nodeBatchCount++;

    /* No optimization if cpu is involved. */
    if (cpuInvolved) return VX_SUCCESS;


    /* Generate code and states. */
    /* Check image size. */
    for (index = startIndex; index <= endIndex; index++)
    {
        vx_node     node = graph->optimizedNodeTable[index];
        gcoVX_Kernel_Context * kernelContext;

        /* Allocal a local kernelContext for execution. */
        kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        if (kernelContext == gcvNULL) return VX_ERROR_NO_MEMORY;
        node->kernelContext = kernelContext;

#if gcdVX_OPTIMIZER > 1
        kernelContext->codeGenOnly = vx_true_e;
        status = node->kernel->function(node, (vx_reference *)node->paramTable, node->kernel->signature.paramCount);
#if gcdVX_OPTIMIZER > 2
        if (tiled)
        {
            /* Get max input/output count and access pattern. */
            vx_kernel_optimization_attribute_s * kernelAttr = &node->kernel->attributes.optAttributes;
            vx_node_optimization_attribute_s * nodeAttr = &node->optAttributes;
            nodeAttr->inputImageCount = kernelAttr->inputImageCount;
            nodeAttr->outputImageCount = kernelAttr->outputImageCount;
            if (kernelAttr->inputImageCount + kernelAttr->outputImageCount > imageCount)
            {
                imageCount = kernelAttr->inputImageCount + kernelAttr->outputImageCount;
            }

            /* Check if image size is the same. */
            if (imageSize.x == 0)
            {
                imageSize.x = kernelContext->params.xmax;
                imageSize.y = kernelContext->params.ymax;
            }
            else if ((imageSize.x != kernelContext->params.xmax)
                || (imageSize.y != kernelContext->params.ymax))
            {
                tiled = vx_false_e;
            }
        }
#endif
#endif

        if (status != VX_SUCCESS) return status;
    }

#if gcdVX_OPTIMIZER > 2
    batch->tiled = tiled;
    batch->globalSize = imageSize;

    if (tiled)
    {
        /* TODO - Get cache size. */
        vx_uint32 cacheSize = 65536;

        /* Calculate tile size. */
        vx_uint32 tileArea = gcmALIGN(cacheSize / imageCount, 256);
        tileSize.x = gcmALIGN((int) sqrt((float) tileArea), 16);
        tileSize.y = tileArea / tileSize.x;
        batch->tileSize.x = tileSize.x;
        batch->tileSize.y = tileSize.y;
        batch->tileCount.x = (imageSize.x + tileSize.x - 1) / tileSize.x;
        batch->tileCount.y = (imageSize.y + tileSize.y - 1) / tileSize.y;
    }
#endif

    return status;
}

VX_PRIVATE_API vx_status vxoGraph_Optimize(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 start;
    vx_uint32 i;

    vxmASSERT(graph && graph->verified);

    /* TODO - Copy nodes for kernel fusion. */

    /* Split nodes into batches at nodes that are process by CPU or have callback function. */
    graph->nodeBatchCount = 0;
    start = 0;
    for (i = 0; i < graph->optimizedNodeCount; i++)
    {
        vx_node node = graph->optimizedNodeTable[i];

        if (!node->kernelAttributes.optAttributes.oneKernelModule)
        {
            /* Check if there is an unended batch. */
            if (i > start)
            {
                /* End the previous batch. */
                gcmONERROR(vxoGraph_AddNodeBatch(graph, start, i - 1, vx_false_e, vx_false_e));
            }

            /* Add a new batch that contains this node only. */
            gcmONERROR(vxoGraph_AddNodeBatch(graph, i, i, vx_true_e, vx_false_e));

            start = i + 1;
            continue;
        }
        else if (node->completeCallback)
        {
            /* End the current batch. */
            gcmONERROR(vxoGraph_AddNodeBatch(graph, start, i, vx_false_e, vx_true_e));
            /* Mark batch with callback function. */

            start = i + 1;
            continue;
        }
    }

    /* Check if there is an unended batch. */
    if (i > start)
    {
        /* End the previous batch. */
        gcmONERROR(vxoGraph_AddNodeBatch(graph, start, i - 1, vx_false_e, vx_false_e));
    }

    graph->optimized = vx_true_e;

    return VX_SUCCESS;

OnError:
    /* TODO - Release optimzed nodes. */

    /* Always return VX_SUCCESS. */
    return VX_SUCCESS;
}
#endif

VX_PRIVATE_API vxnne_operation_target_e GetOperationTarget(void* operation, vx_bool isNode)
{
    if (isNode)
    {
        vx_node node = (vx_node)operation;
        if (node->kernelAttributes.isGPUKernel)
        {
            return VXNNE_OPERATION_TARGET_SH;
        }
        else
        {
            return VXNNE_OPERATION_TARGET_SW;
        }
    }
    else
    {
        return ((vxnne_operation)operation)->target;
    }
}

VX_PRIVATE_API void SetOperationSyncMode(void* operation, vx_bool isNode, vxnne_sync_mode_e syncMode, gctUINT32 semaHandle)
{
    if (isNode)
    {
        vx_node node = (vx_node)operation;
        if (syncMode == VXNNE_SYNC_MODE_HW_WAKE || syncMode == VXNNE_SYNC_MODE_SW_WAKE)
        {
            node->wakeMode = syncMode;
            node->semaWakeHandle = semaHandle;
        }
        else
        {
            node->waitMode = syncMode;
            node->semaWaitHandle = semaHandle;
        }
    }
    else
    {
        if (syncMode == VXNNE_SYNC_MODE_HW_WAKE || syncMode == VXNNE_SYNC_MODE_SW_WAKE)
        {
            ((vxnne_operation)operation)->wakeMode = syncMode;
            ((vxnne_operation)operation)->semaWakeHandle = semaHandle;
        }
        else
        {
            ((vxnne_operation)operation)->waitMode = syncMode;
            ((vxnne_operation)operation)->semaWaitHandle = semaHandle;
        }
    }
}

VX_PRIVATE_API vx_bool vxoGraph_GetMCFESemphore(vx_graph graph, gctUINT32 *semaHandle)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MCFE)) return vx_false_e;

    if (graph->semaNum == VX_MAX_MCFE_SEMAPHORE) return vx_false_e;

    status = gcoHAL_AllocateMCFESemaphore(semaHandle);

    if (gcmIS_SUCCESS(status))
    {
        graph->mcfeSema[graph->semaNum] = *semaHandle;
        graph->semaNum++;
    }

    return (vx_bool)(status == gcvSTATUS_OK ? vx_true_e : vx_false_e);
}

VX_PRIVATE_API void CheckOperationSyncPattern(vx_graph graph, void* operation1, vx_bool isNode1, void* operation2, vx_bool isNode2)
{
    vx_uint32           syncPattern;
    vxnne_operation_target_e opTarget1, opTarget2;

    if (!operation1 || !operation2) return;

    opTarget1 = GetOperationTarget(operation1, isNode1);
    opTarget2 = GetOperationTarget(operation2, isNode2);

    syncPattern = VXNNE_MAKE_SYNC_PATTERN(opTarget1, opTarget2);

    switch (syncPattern)
    {
    case VXNNE_OPERATION_SYNC_PATTERN_NN_SW :
    case VXNNE_OPERATION_SYNC_PATTERN_SH_SW :
    case VXNNE_OPERATION_SYNC_PATTERN_TP_SW :
        SetOperationSyncMode(operation2, isNode2, VXNNE_SYNC_MODE_SW_WAIT, 0);
        break;
    case VXNNE_OPERATION_SYNC_PATTERN_NN_SH :
    case VXNNE_OPERATION_SYNC_PATTERN_NN_TP :
    case VXNNE_OPERATION_SYNC_PATTERN_SH_NN :
    case VXNNE_OPERATION_SYNC_PATTERN_SH_TP :
    case VXNNE_OPERATION_SYNC_PATTERN_TP_SH :
    case VXNNE_OPERATION_SYNC_PATTERN_TP_NN :
        {
        }

    default: break;
    }
}

VX_PRIVATE_API vx_status vxoGraph_VerifyNodeSync(vx_graph graph)
{
    vx_uint32 i;

    void*   currentOperation = VX_NULL;
    vx_bool isCurrentNode = vx_false_e;

    for(i = 0; i < graph->semaNum; i++)
    {
        gcoHAL_FreeMCFESemaphore(graph->mcfeSema[i]);
    }

    graph->semaNum = 0;

    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];

        if (node->layer)
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            CheckOperationSyncPattern(graph, currentOperation, isCurrentNode, (void*)node, vx_true_e);
            currentOperation = (void*)node;
            isCurrentNode = vx_true_e;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API void vxoGraph_VerifyOperationSync(vx_graph graph)
{
    vx_uint32 i;
    vx_uint32 syncPattern;
    vxnne_operation op1, op2;

    if (!graph->layer || (graph->layer->opIndicesNum == 0)) return;

    for(i = 0; i < graph->semaNum; i++)
    {
        gcoHAL_FreeMCFESemaphore(graph->mcfeSema[i]);
    }

    graph->semaNum = 0;

    for (i = 0; i < graph->layer->opIndicesNum - 1; i++)
    {
        op1 = graph->layer->operations[graph->layer->opIndices[i].operationID];
        op2 = graph->layer->operations[graph->layer->opIndices[i + 1].operationID];

        syncPattern = VXNNE_MAKE_SYNC_PATTERN(op1->target, op2->target);

        switch (syncPattern)
        {
        case VXNNE_OPERATION_SYNC_PATTERN_NN_SW :
        case VXNNE_OPERATION_SYNC_PATTERN_SH_SW :
        case VXNNE_OPERATION_SYNC_PATTERN_TP_SW :
        case VXNNE_OPERATION_SYNC_PATTERN_SC_SW :
            graph->layer->opIndices[i+1].waitMode           = VXNNE_SYNC_MODE_SW_WAIT;
            graph->layer->opIndices[i+1].semaWaitHandle     = 0;
            break;
        case VXNNE_OPERATION_SYNC_PATTERN_NN_SH :
        case VXNNE_OPERATION_SYNC_PATTERN_NN_TP :
        case VXNNE_OPERATION_SYNC_PATTERN_NN_SC :
        case VXNNE_OPERATION_SYNC_PATTERN_SH_NN :
        case VXNNE_OPERATION_SYNC_PATTERN_SH_TP :
        case VXNNE_OPERATION_SYNC_PATTERN_SH_SC :
        case VXNNE_OPERATION_SYNC_PATTERN_TP_SH :
        case VXNNE_OPERATION_SYNC_PATTERN_TP_NN :
        case VXNNE_OPERATION_SYNC_PATTERN_TP_SC :
        case VXNNE_OPERATION_SYNC_PATTERN_SC_SH :
        case VXNNE_OPERATION_SYNC_PATTERN_SC_NN :
        case VXNNE_OPERATION_SYNC_PATTERN_SC_TP :
            {
                gctUINT32 semaHandle;

                if ((graph->layer->opIndices[i].batchID == graph->layer->opIndices[i+1].batchID)
                    && vxoGraph_GetMCFESemphore(graph, &semaHandle))
                {
                    graph->layer->opIndices[i].wakeMode         = VXNNE_SYNC_MODE_HW_WAKE;
                    graph->layer->opIndices[i].semaWakeHandle   = semaHandle;

                    graph->layer->opIndices[i+1].waitMode       = VXNNE_SYNC_MODE_HW_WAIT;
                    graph->layer->opIndices[i+1].semaWaitHandle = semaHandle;
                }

                break;
            }

        default: break;
        }
    }
}

VX_PRIVATE_API vx_status vxoGraph_ProcessInternal(vx_graph graph);

VX_PRIVATE_API void vxoGraph_GenerateCommandBuffer(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 *commandBuffer = VX_NULL;
    vx_uint32 outCommandBufferSize = 0;
    gceSTATUS mcfeEnabled = (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MCFE));
    vx_bool   bCaptureOn = vx_false_e;

#if gcdDUMP_PER_OPERATION || VXM_FORCE_PER_OPERATION_IDLE
    vxmONERROR(VX_ERROR_NOT_SUPPORTED);
#endif

    if (graph->commandBuffer)
    {
        vxFree(graph->commandBuffer);
        graph->commandBufferSizeInByte = 0;
    }

    if (graph->hasCPUFunction                                  ||
        graph->base.context->options.enableCNNPerf             ||
        graph->base.context->options.enableNNLayerDump         ||
        graph->base.context->options.enableSaveBinary          ||
        graph->binarySave                                      ||
        !graph->base.context->options.enableGraphCommandBuffer ||
        mcfeEnabled)
    {
        vxmONERROR(VX_ERROR_NOT_SUPPORTED);
    }

    commandBuffer = (vx_uint8 *)vxAllocateAndZeroMemory(gcdCMD_BUFFER_SIZE);

    vxmONERROR((commandBuffer != VX_NULL ? VX_SUCCESS : VX_ERROR_NO_MEMORY));

    vxmONERROR(gcfVX_CaptureState(commandBuffer,
        gcdCMD_BUFFER_SIZE, gcvNULL, gcvTRUE, gcvTRUE));

    bCaptureOn = vx_true_e;

    vxmONERROR(vxoGraph_ProcessInternal(graph));

    vxmONERROR(gcfVX_CaptureState(gcvNULL, 0, &outCommandBufferSize, gcvFALSE, gcvFALSE));

    bCaptureOn = vx_false_e;

    graph->commandBuffer = (vx_uint32 *)commandBuffer;
    graph->commandBufferSizeInByte = outCommandBufferSize;

    return;

OnError:

    if (bCaptureOn)
    {
        gcfVX_CaptureState(gcvNULL, 0, &outCommandBufferSize, gcvFALSE, gcvFALSE);
    }

#if gcdDEBUG
    vxInfo("Graph=%p doesn't support graph command: "
             "hasCPUFunction=%d CNNPERF=%d NNLayerDump=%d"
             "MCFE=%d networkBinary=%d enableGraphCommandBuffer=%d\n",
             graph, graph->hasCPUFunction,
             graph->base.context->options.enableCNNPerf,
             graph->base.context->options.enableNNLayerDump,
             mcfeEnabled, graph->base.context->options.enableSaveBinary,
             graph->base.context->options.enableGraphCommandBuffer);
#endif
    if (commandBuffer)
        vxFree(commandBuffer);

    return;
}

VX_PRIVATE_API void vxoGraph_GeneratePatchLocForInputs(vx_graph graph)
{
    vx_uint32 i;
    vx_uint32 j;

    if (graph->commandBuffer == VX_NULL)
        return;

    for (i = 0; i < graph->headNodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->headNodeIndexTable[i]];
        for (j = 0; j < node->kernel->signature.paramCount; j++)
        {
            if (node->kernel->signature.stateTable[j] == VX_PARAMETER_STATE_OPTIONAL)
                continue;

            switch (node->paramTable[j]->type)
            {
            case VX_TYPE_IMAGE:
                {
                    vx_uint32 planeIndx = 0;
                    vx_image image = (vx_image)node->paramTable[j];
                    vx_uint32 commandSizeInUint = graph->commandBufferSizeInByte / 4;
                    for (planeIndx = 0; planeIndx < image->planeCount; planeIndx++)
                    {
                        vx_uint32 physical = image->memory.physicals[planeIndx];
                        vx_uint32 location = 0;
                        for (location = 0; location < commandSizeInUint; location++)
                        {
                            if (physical == graph->commandBuffer[location])
                                break;
                        }
                        if (location == commandSizeInUint)
                            location = 0;
                        node->patchLocation[j][planeIndx] = location;

                    }
                    break;
                }
            case VX_TYPE_SCALAR:
                {
                    vx_uint32 location = 0;
                    vx_scalar scalar = (vx_scalar)node->paramTable[j];
                    vx_uint32 physical = scalar->physical;
                    vx_uint32 commandSizeInUint = graph->commandBufferSizeInByte / 4;
                    for (location = 0; location < commandSizeInUint; location++)
                    {
                        if (physical == graph->commandBuffer[location])
                            break;
                    }
                    if (location == commandSizeInUint)
                        location = 0;
                    node->patchLocation[j][0] = location;
                }
                break;
            default:
                /* vxmASSERT(0); */
                break;
            }
        }
    }
    return;
}



VX_PRIVATE_API vx_status vxoGraph_VerifyGraph(vx_graph graph)
{
    vx_status status;
    vx_bool first = ((graph->verified == vx_false_e) && (graph->reverify == vx_false_e)) ? vx_true_e : vx_false_e;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    vxAcquireMutex(graph->base.lock);

    /* CTS UserNode.UserKernel/42/_FROM_REF/ UserNode.UserKernel/42/_FROM_ATTR/ w/a*/
    if (graph->nodeTable[0]->kernel->isUserkernel)
        graph->verified = vx_false_e;
    else if (graph->verified)
    {
        vxReleaseMutex(graph->base.lock);
        return VX_SUCCESS;
    }

    vxmONERROR(vxoGraph_UserKernelPreprocess(graph, first));

    vxmONERROR(vxoGraph_VerifyAllNodeParameters(graph));

    vxmONERROR(vxoGraph_VerifyAllNodeWriteDependencies(graph));

    vxmONERROR(vxoGraph_RetrieveTopology(graph));

    vxoGraphBinary_CacheOrImport(graph);

    if (graph->base.context->options.enableGraphAdapter)
    {
        vxmONERROR(vxoGraph_Adapter(graph));
    }

    vxmONERROR(vxoGraph_AllocateAllMemoryObjects(graph));

    vxmONERROR(vxoGraph_DetectAllHeadNodes(graph));

    vxmONERROR(vxoGraph_DetectAllTailNodes(graph));

    vxmONERROR(vxoGraph_Optimization(graph));

    vxmONERROR(vxoGraph_DetectCycle(graph));;

    vxmONERROR(vxoGraph_DetectUnvisitedNodes(graph));

    vxmONERROR(vxoGraph_VerifyAllNodesByTarget(graph));

#if PRE_SORT
    vxoGraph_GenerateAllNodeIndexTable(graph);
#endif

    vxmONERROR(vxoGraph_InitializeAllNodeKernels(graph));

    vxmONERROR(vxoGraph_CaculateCostFactors(graph));

#if gcdVX_OPTIMIZER
    vxoGraph_Flatten(graph);

    /* Only top-level graph needs to be optimized. */
    if (!graph->isSubGraph)
    {
        vxoGraph_Optimize(graph);
    }
#endif

#if PRE_SORT
    vxoGraph_GenerateOperationTable(graph);

    if (graph->layer != NULL)
    {
        vxoGraph_GenerateOpParentChild(graph);

        if (graph->base.context->options.collectPerfType == COLLECT_PERF_ESTIMATE)
        {
            vxoGraph_PredictPerf(graph);
        }

        vxmONERROR(vxoGraph_VerifyTiling(graph));

        vxmONERROR(vxoGraph_VerifyVirtualBuffer(graph));

        vxmONERROR(vxoGraphBinary_SaveBinaryEntrance(graph));

        vxmONERROR(vxnneExecutionLayer_GenerateCommands(graph->base.context, &graph->layer->base));

        vxoGraph_VerifyOperationSync(graph);
    }
    else
    {
        vxmONERROR(vxoGraph_PrepareParamMemory(graph));

        vxoGraph_VerifyNodeSync(graph);
    }
#endif

    vxoGraph_GenerateCommandBuffer(graph);

    vxoGraph_GeneratePatchLocForInputs(graph);

    graph->reverify = vx_false_e;

    graph->verified = vx_true_e;

    graph->status   = VX_GRAPH_STATE_VERIFIED;

    vxReleaseMutex(graph->base.lock);

    return VX_SUCCESS;

OnError:

    graph->reverify = vx_false_e;

    graph->verified = vx_false_e;

    graph->status   = VX_GRAPH_STATE_UNVERIFIED;

    vxReleaseMutex(graph->base.lock);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    vx_status status = VX_SUCCESS;
    gctBOOL  switched = gcvFALSE;

    gcmDUMP_API("$VX vxVerifyGraph: graph=%p", graph);

    vxmASSERT(graph);
    if(graph->parentGraph == gcvNULL)
    {
        gcmONERROR(gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex));
        switched = gcvTRUE;
    }
    status = vxoGraph_VerifyGraph(graph);

    if(graph->parentGraph == gcvNULL)
    {
        gcoVX_RestoreContext(savedHardware, savedHardwareType, savedCoreIndex);
    }

    return status;
OnError:
    if(switched)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }

    vxError("%s[%d]: Verify Graph fail!\n", __FUNCTION__, __LINE__);
    vxAddLogEntry(&graph->base, VX_FAILURE, "%s[%d]: Verify Graph fail!\n", __FUNCTION__, __LINE__);
    return VX_FAILURE;
}

#if gcdVX_OPTIMIZER
VX_PRIVATE_API vx_status vxoGraph_ProcessOptimized(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_action action;
    vx_uint32 i;

    vxmASSERT(graph && graph->verified && graph->optimized);

#if gcdVX_OPTIMIZER > 2
    /* Flush ICache and PSSHL1 cache without stall. */
    gcoVX_FlushCache(vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e);
#endif
    vxoPerf_Begin(&graph->perf);

    /*Restart:*/

    action = VX_ACTION_CONTINUE;

    /* Execute all node batches. */
    for (i = 0; i < graph->nodeBatchCount; i++)
    {
        vx_node_batch batch = &graph->nodeBatch[i];
        vx_node node = gcvNULL;
        vx_target target;
        vx_uint32 j;

        if (batch->cpuInvolved)
        {
#if gcdVX_OPTIMIZER > 2
            /* Flush PSSHL1 cache with stall for cpu to work on the data. */
            gcoVX_FlushCache(vx_false_e, vx_true_e, vx_true_e, vx_false_e, vx_false_e);
#endif
            node = graph->optimizedNodeTable[batch->startIndex];
            target = &graph->base.context->targetTable[node->targetIndex];

            vxoNode_EnableVirtualAccessible(node);

            action = target->funcs.processnodes(target, &node, 0, 1);

            vxoNode_DisableVirtualAccessible(node);
        }
        else
        {
            vx_uint32 ii, jj;
            vx_uint32 tileCountI = 1, tileCountJ = 1;
            vx_uint32 tileOffsetI, tileOffsetJ;

#if gcdVX_OPTIMIZER > 1
            gcoVX_Kernel_Context * kernelContext;

            for (j = batch->startIndex; j <= batch->endIndex; j++)
            {
                node = graph->optimizedNodeTable[j];
                kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
                kernelContext->codeGenOnly = vx_false_e;

                /* Lock kernel instruction nodes. */
                status = gcoVX_LockKernel(&kernelContext->params);
                if (status != VX_SUCCESS)
                {
                    action = VX_ACTION_ABANDON;
                    break;
                }
#if gcdVX_OPTIMIZER > 2
                if (batch->tiled)
                {
                    kernelContext->params.tiled   = batch->tiled;
                    kernelContext->params.xoffset = kernelContext->params.xmin;
                    kernelContext->params.yoffset = kernelContext->params.ymin;
                    kernelContext->params.xcount  = batch->tileSize.x;
                    kernelContext->params.ycount  = batch->tileSize.y;
                }
#endif
            }
#endif

#if gcdVX_OPTIMIZER > 2
            if (batch->tiled)
            {
                tileCountI = batch->tileCount.x;
                tileCountJ = batch->tileCount.y;
            }
#endif

            /* Loop through each tile. */
            for (jj = 0, tileOffsetJ = 0; jj < tileCountJ; jj++, tileOffsetJ += batch->tileSize.y)
            {
                for (ii = 0, tileOffsetI = 0; ii < tileCountI; ii++, tileOffsetI += batch->tileSize.x)
                {
                    /* Execute each nodes. */
                    for (j = batch->startIndex; j <= batch->endIndex; j++)
                    {
                        node = graph->optimizedNodeTable[j];

                        vxoNode_EnableVirtualAccessible(node);

                        vxoPerf_Begin(&node->perf);

#if gcdVX_OPTIMIZER > 1
                        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;

                        /* Bind objects. */
                        status = gcfVX_BindObjects(kernelContext);
                        if (status != VX_SUCCESS)
                        {
                            action = VX_ACTION_ABANDON;
                            break;
                        }

                        /* Bind kernel. */
                        status = gcoVX_BindKernel(&kernelContext->params);
                        if (status != VX_SUCCESS)
                        {
                            action = VX_ACTION_ABANDON;
                            break;
                        }

                        /* Bind uniforms. */
                        if (kernelContext->uniform_num > 0)
                        {
                            vx_uint32 kk;

                            for(kk = 0; kk < kernelContext->uniform_num; kk++)
                            {
                                gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, kernelContext->uniforms[kk].index, (gctUINT32*)&kernelContext->uniforms[kk].uniform, kernelContext->uniforms[kk].num);
                            }
                        }

                        /*status = gcfVX_Kernel(kernelContext);*/
#if gcdVX_OPTIMIZER > 2
                        kernelContext->params.xoffset = tileOffsetI;
                        kernelContext->params.yoffset = tileOffsetJ;
                        /* Light-weight InvokeKernel that does not include flush nor semaphore-stall. */
                        kernelContext->params.tileMode = vx_true_e;
#endif
                        status = gcoVX_InvokeKernel(&kernelContext->params);
#else
                        status = node->kernel->function(node, (vx_reference *)node->paramTable, node->kernel->signature.paramCount);
#endif

                        vxoPerf_End(&node->perf);

                        vxoNode_DisableVirtualAccessible(node);

                        if (status != VX_SUCCESS)
                        {
                            action = VX_ACTION_ABANDON;
                            break;
                        }
                    }
                }
            }

            /* TODO - Unlock kernels. */
            /*        Currently unlocked when freeMemory. */

            if (batch->endHasCallback)
            {
#if gcdVX_OPTIMIZER > 2
                /* Flush PSSHL1 cache with stall for callback. */
                gcoVX_FlushCache(vx_false_e, vx_true_e, vx_true_e, vx_false_e, vx_false_e);
#endif
                action = node->completeCallback(node);
            }
        }

        if (action != VX_ACTION_CONTINUE) break;
    }

    switch (action)
    {
        /*case VX_ACTION_RESTART:
        goto Restart;*/

    case VX_ACTION_ABANDON:
        status = VX_ERROR_GRAPH_ABANDONED;
        break;
    }

    if (status == VX_SUCCESS)
    {
#if gcdVX_OPTIMIZER > 2
        /* Flush PSSHL1 cache without stall. */
        gcoVX_FlushCache(vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e);
#endif
        gcfVX_Flush(gcvTRUE);
        graph->dirty = vx_false_e;
    }

    vxoPerf_End(&graph->perf);

    vxoPerf_Dump(&graph->perf);

    return status;
}
#endif

VX_PRIVATE_API void vxoGraph_BeginProcess(vx_graph graph)
{
    vxmASSERT(graph);

    vxoGraph_ClearAllVisitedFlags(graph);

    vxoGraph_ClearAllExecutedFlags(graph);

    if (graph->base.context->options.enableCNNPerf)
        vxoPerf_Begin(&graph->perf);
}

VX_PRIVATE_API vx_status vxoGraph_ProcessKernelPrint(vx_graph graph)
{
    vx_uint32 i;
    vx_status status = VX_SUCCESS;
    vxmASSERT(graph);

    for (i = 0; i < graph->layer->base.num_operations; i++)
    {
        if (graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_SH)
        {
            vxnne_shader_operation shaderOperation  = VX_NULL;
            vx_shader shader = VX_NULL;

            if (!graph->layer->operations[i]->layer->node->kernel->kernelShader)
            {
                shaderOperation  = (vxnne_shader_operation)graph->layer->operations[i];
                shader = shaderOperation->shaderExecutable->kernelShader;
                if (shader->hasPrintf)
                {
                    vxmONERROR(vxoKernel_ProcessKernelShaderPrint(shader,
                        &shaderOperation->shaderExecutable->shaderParam));
                }
            }
            else
            {
                shader = graph->layer->operations[i]->layer->node->kernel->kernelShader[graph->layer->operations[i]->layer->node->kernel->currShaderID];
                if (shader->hasPrintf)
                {
                    vxmONERROR(vxoKernel_ProcessKernelShaderPrint(shader,
                        &graph->layer->operations[i]->layer->node->kernelAttributes.shaderParameter));
                }
            }
        }
    }

OnError:
    return status;
}

VX_PRIVATE_API void vxoGraph_EndProcess(vx_graph graph)
{
    vx_uint32 i, j;
    vxmASSERT(graph);
    if (!graph->isChildGraph)
    {
        gcfVX_Flush(gcvTRUE);
    }

    /*copy data from gpu buffer to cpu buffer in the end*/
    for (j = 0; j < graph->tailNodeCount; j++)
    {
        vx_node node = graph->nodeTable[graph->tailNodeIndexTable[j]];
        for (i = 0; i < node->kernel->signature.paramCount; i++)
        {
            vx_enum direction, type;
            direction = node->kernel->signature.directionTable[i];
            type = node->kernel->signature.dataTypeTable[i];
            if(type == VX_TYPE_IMAGE && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
            {
                vx_rectangle_t rect;
                vx_image image = (vx_image)node->paramTable[i];
                vx_uint32 plane = 0;

                if(image->importType != VX_MEMORY_TYPE_HOST || image->useInternalMem == vx_false_e)
                {
                    continue;
                }

                gcoVX_Flush(gcvTRUE);

                vxGetValidRegionImage(image, &rect);

                for (plane = 0; plane < image->memory.planeCount; plane++)
                {
                    if (image->memory.nodePtrs[plane] != VX_NULL && image->memory.logicals[plane] != image->memory.nodePtrs[plane]->logical)
                    {
                        vx_size size = 0;
                        size = vxComputeImagePatchSize(image, &rect, plane);
                        /*Only copy different memory. For CTS GraphROI.Simple */
                        if (size > 0 && (abs((vx_int32)(gcmALL_TO_UINT32(image->memory.logicals[plane]) - gcmALL_TO_UINT32(image->memory.nodePtrs[plane]->logical))) > (vx_int32)size))
                            gcoOS_MemCopy(image->memory.logicals[plane], image->memory.nodePtrs[plane]->logical, size);
                    }
                }
            }
            else if(type == VX_TYPE_TENSOR && (direction == VX_OUTPUT || direction == VX_BIDIRECTIONAL))
            {
                vx_tensor tensor = (vx_tensor)node->paramTable[i];

                if(tensor->useInternalMem == vx_true_e && tensor->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
                {
                    gcoVX_Flush(gcvTRUE);
                    gcoOS_MemCopy(tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.nodePtrs[0]->logical, tensor->tensorBuffer->memory.nodePtrs[0]->size);
                }
            }
        }
    }

#if gcdDUMP
    {
        vx_uint32 i, j;
        vx_node node;

        gcfVX_Flush(gcvTRUE);
        for (j = 0; j < graph->tailNodeCount; j++)
        {
            node = graph->nodeTable[graph->tailNodeIndexTable[j]];

            for (i = 0; i < node->kernel->signature.paramCount; i++)
            {
                vx_reference paramRef = node->paramTable[i];
                if (paramRef == VX_NULL) continue;

                if ((paramRef->type == VX_TYPE_TENSOR) && vxmIS_OUTPUT_OR_BIDIRECTION(node->kernel->signature.directionTable[i]))
                {
                    vx_uint32 size;

                    vxoTensor_GetTensorSize((vx_tensor)paramRef, &size);

                    gcmDUMP(gcvNULL, "#[verify]\n");

                    gcmDUMP_BUFFER(gcvNULL,
                        gcvDUMP_BUFFER_VERIFY,
                        TENSOR_PHYSICAL_ADDR((vx_tensor)paramRef),
                        TENSOR_LOGICAL_ADDR((vx_tensor)paramRef),
                        0,
                        size);
                }
            }
        }
    }
#endif

    if (graph->hasPrintf)
    {
        vxoGraph_ProcessKernelPrint(graph);
    }

    graph->dirty = vx_false_e;

    if (graph->base.context->options.enableCNNPerf)
    {
        vxoPerf_End(&graph->perf);
        vxoPerf_Dump(&graph->perf);
    }

    vxoGraph_ClearAllVisitedFlags(graph);
}

#if (0 && !PRE_SORT)
VX_PRIVATE_API vx_status vxoGraph_Sort(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_action action;
    vx_uint32 lastNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 nextNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 leftNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 lastNodeCount, nextNodeCount, leftNodeCount = 0;
    vx_uint32 i;

    vx_uint32 debugShaderIndex;
    vx_context context = vxoContext_GetFromReference(&graph->base);

    vxmASSERT(graph);
    if (!graph->verified)
    {
        status = vxVerifyGraph(graph);

        if (status != VX_SUCCESS) return status;
    }
    action = VX_ACTION_CONTINUE;
    while (nextNodeCount > 0)
    {
        for (i = 0; i < nextNodeCount; i++)
        {
            vx_node node = graph->nodeTable[nextNodeIndexTable[i]];
            vx_target target = &graph->base.context->targetTable[node->targetIndex];

            if (node->sorted)
            {
                vxError("Node %p in Graph %p was executed", node, graph);
                continue;
            }

            vxoNode_EnableVirtualAccessible(node);

            if (context->callback.enable && node != NULL && node->kernel->name != NULL)
            {
                gcSHADER shader = NULL;
                gctPOINTER debugInfo = NULL;

                if (node->kernel->kernelShader != NULL &&
                    node->kernel->kernelShader[0] != NULL)
                {
                    shader = (gcSHADER)node->kernel->kernelShader[0]->states.binary;
                    debugInfo = gcSHADER_GetDebugInfo(shader);
                }

                gcoVX_addDebugShader(&debugShaderIndex,
                    node->kernel->name, graph, node, NULL,
                    node->kernel->program, NULL,
                    (node->kernel->program != NULL)?node->kernel->program->source:NULL,
                    debugInfo,
                    node->kernelAttributes.borderMode.mode,
                    node->kernel->type,
                    -1, 0, 0, NULL, NULL, 0, -1, -1, (vx_int32)node->id, gcvTRUE);
                node->debugShaderIndex = debugShaderIndex;
            }

            /*node->sorted = vx_true_e;*/
            if (graph->dirty)
            {
                action = target->funcs.processnodesForSort(target, &node, 0, 1);
                vxoNode_RecordForSort(node);
            }
            else
            {
                action = VX_ACTION_CONTINUE;
                if (vxoNode_ReplayForSort(node) != VX_SUCCESS)
                {
                    action = target->funcs.processnodesForSort(target, &node, 0, 1);
                }
            }

            vxoNode_DisableVirtualAccessible(node);

            if (action != VX_ACTION_CONTINUE) break;

        }

        vxCopyNodeIndexTable(nextNodeIndexTable, nextNodeCount, OUT lastNodeIndexTable, OUT &lastNodeCount);

        vxoGraph_GenerateNextNodeTableForSort(graph, lastNodeIndexTable, lastNodeCount,
            OUT nextNodeIndexTable, OUT &nextNodeCount,
            INOUT leftNodeIndexTable, INOUT &leftNodeCount);
    }

    if (status != VX_SUCCESS) return status;

    return VX_SUCCESS;
}
#endif



VX_PRIVATE_API vx_status vxoGraph_ProcessInternal(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_action action = VX_ACTION_CONTINUE;
#if !PRE_SORT
    vx_uint32 lastNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 nextNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 leftNodeIndexTable[VX_MAX_NODE_COUNT] = {0};
    vx_uint32 lastNodeCount, nextNodeCount, leftNodeCount = 0;
#endif
    vx_uint32 i;

#if PRE_SORT
    if (graph->layer)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[0]];
        vx_target target = &graph->base.context->targetTable[node->targetIndex];

        action = target->funcs.processLayer(target, &graph->layer->base);
    }
    else
    {
        for (i = 0; i < graph->nodeCount; i++)
        {
            vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
            vx_target target = &graph->base.context->targetTable[node->targetIndex];

            if (node->executed)
            {
                vxError("Node %p in Graph %p was executed", node, graph);
                continue;
            }

            vxoNode_EnableVirtualAccessible(node);

            action = target->funcs.processnodes(target, &node, 0, 1);

            vxoNode_DisableVirtualAccessible(node);

            if (action != VX_ACTION_CONTINUE) vxmONERROR(VX_ERROR_GRAPH_ABANDONED);
        }
    }
#else
    vxCopyNodeIndexTable(graph->headNodeIndexTable, graph->headNodeCount,
        OUT nextNodeIndexTable, OUT &nextNodeCount);

    while (nextNodeCount > 0)
    {
        for (i = 0; i < nextNodeCount; i++)
        {
            vx_node node = graph->nodeTable[nextNodeIndexTable[i]];
            vx_target target = &graph->base.context->targetTable[node->targetIndex];

            if (node->executed)
            {
                vxError("Node %p in Graph %p was executed", node, graph);
                continue;
            }

            vxoNode_EnableVirtualAccessible(node);


            if (graph->dirty)
            {
                action = target->funcs.processnodes(target, &node, 0, 1);
                vxoNode_Record(node);
            }
            else
            {
                action = VX_ACTION_CONTINUE;
                if (vxoNode_Replay(node) != VX_SUCCESS)
                {
                    action = target->funcs.processnodes(target, &node, 0, 1);
                }
                else
                {
                    if (node->completeCallback != VX_NULL)
                    {
                        action = node->completeCallback(node);
                    }
                }
            }

            vxoNode_DisableVirtualAccessible(node);

            if (action != VX_ACTION_CONTINUE) vxmONERROR(VX_ERROR_GRAPH_ABANDONED);
        }

        if (action != VX_ACTION_CONTINUE) vxmONERROR(VX_ERROR_GRAPH_ABANDONED);

        vxCopyNodeIndexTable(nextNodeIndexTable, nextNodeCount, OUT lastNodeIndexTable, OUT &lastNodeCount);

        vxoGraph_GenerateNextNodeTable(graph, lastNodeIndexTable, lastNodeCount,
            OUT nextNodeIndexTable, OUT &nextNodeCount,
            INOUT leftNodeIndexTable, INOUT &leftNodeCount);
    }
#endif

    if (action != VX_ACTION_CONTINUE)
    {
        vxmONERROR(VX_ERROR_GRAPH_ABANDONED);
    }

    if (graph->binarySave != NULL)
    {
        vxmONERROR(vxoGraphBinary_ReSaveInputAndPatchTable(graph));
    }

OnError:
    return status;
}

VX_INTERNAL_API vx_status vxoGraph_Submit(vx_graph graph)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (!graph->commandBuffer)
        return VX_ERROR_NOT_IMPLEMENTED;

    gcmONERROR(gcoVX_Replay((gctPOINTER)graph->commandBuffer, (gctUINT32)graph->commandBufferSizeInByte));

OnError:
    return gcmIS_SUCCESS(status) ? VX_SUCCESS : VX_FAILURE;
}


VX_PRIVATE_API vx_status vxoGraph_Process(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    vxmASSERT(graph);

#if VIVANTE_PROFILER
    /* Only top-level graph needs to profile. */
    if (!graph->isChildGraph)
    {
        vxoProfiler_Begin((vx_reference)graph);
    }
#endif

    if (!graph->verified)
    {
        status = vxoGraph_VerifyGraph(graph);

        if (status != VX_SUCCESS)
        {
            return status;
        };
    }

#if gcdVX_OPTIMIZER
    if (graph->optimized)
    {
        status = vxoGraph_ProcessOptimized(graph);
    }
    else
#endif
    {
/*Restart:*/
        graph->status = VX_GRAPH_STATE_RUNNING;

        vxoGraph_BeginProcess(graph);

        if (graph->commandBuffer)
        {
            status = vxoGraph_Submit(graph);
        }
        else
        {
            status = vxoGraph_ProcessInternal(graph);
        }

        vxoGraph_EndProcess(graph);

        if (status != VX_SUCCESS)
            graph->dirty = vx_true_e;

        if (graph->hasAutoAgingDelay)
        {
            for (i = 0; i < VX_MAX_REF_COUNT; i++)
            {
                if (vxoReference_IsValidAndSpecific(&graph->delays[i]->base, VX_TYPE_DELAY) == vx_true_e)
                    vxAgeDelay(graph->delays[i]);
            }
        }
    }


#if VIVANTE_PROFILER
    if (!graph->isChildGraph)
    {
        vxoProfiler_End((vx_reference)graph);
    }
#endif

    if (status == VX_SUCCESS)
    {
        graph->status = VX_GRAPH_STATE_COMPLETED;
    }
    else
    {
        graph->status = VX_GRAPH_STATE_ABANDONED;
        return status;
    }
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    gcmDUMP_API("$VX vxScheduleGraph: graph=%p", graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxTryAcquireMutex(graph->scheduleLock, 0)) return VX_ERROR_GRAPH_SCHEDULED;

    vxZeroMemory(&graph->data, sizeof(vx_value_set_s));
    graph->data.v1 = (vx_return_value)graph;

    if (!vxoQueue_WriteData(&graph->base.context->processor.input, &graph->data))
    {
        vxReleaseMutex(graph->scheduleLock);
        return VX_ERROR_NO_RESOURCES;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    vx_status       status = VX_SUCCESS;

    gcmDUMP_API("$VX vxWaitGraph: graph=%p", graph);

    if (!vxoReference_IsValidAndNoncontext(&graph->base)) return VX_ERROR_INVALID_REFERENCE;

    while (1)
    {
        vx_value_set    data = VX_NULL;

        if (!vxoQueue_ReadData(&graph->base.context->processor.output, OUT &data))
        {
            vxReleaseMutex(graph->scheduleLock);
            return VX_FAILURE;
        }

        vxmASSERT(data);

        if ((vx_graph)data->v1 != graph)
        {
            vxoQueue_WriteData(&graph->base.context->processor.output, data);
            continue;
        }

        vxmASSERT(data == &graph->data);

        status = (vx_status)data->v2;

        break;
    }

    vxReleaseMutex(graph->scheduleLock);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    vx_status status = VX_SUCCESS;
    gctBOOL switched = gcvFALSE;

    gcmDUMP_API("$VX vxProcessGraph: graph=%p", graph);

    vxmASSERT(graph);

    if (!vxoReference_IsValidAndNoncontext(&graph->base))
    {
        vxError("%s[%d]: Graph's reference is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Graph's reference is invalid!\n", __FUNCTION__, __LINE__);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if(graph->parentGraph == gcvNULL)
    {
        gcmONERROR(gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex));
        switched = gcvTRUE;
    }

#if (0 && !PRE_SORT)
    vxoGraph_Sort(graph);
#endif

    status = vxoGraph_Process(graph);

    if(graph->parentGraph == gcvNULL)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }
    return status;
OnError:
    if(switched)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }

    vxError("%s[%d]: Process Graph fail!\n", __FUNCTION__, __LINE__);
    vxAddLogEntry(&graph->base, VX_FAILURE, "%s[%d]: Process Graph fail!\n", __FUNCTION__, __LINE__);
    return VX_FAILURE;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter param)
{
    gcmDUMP_API("$VX vxAddParameterToGraph: graph=%p, param=%p", graph, param);

    if (!vxoReference_IsValidAndNoncontext(&graph->base)) return VX_ERROR_INVALID_REFERENCE;

    if (vxoReference_IsValidAndSpecific(&param->base, VX_TYPE_PARAMETER))
    {
        graph->paramTable[graph->paramCount].node = param->node;
        graph->paramTable[graph->paramCount].index = param->index;
    }
    else
    {
        graph->paramTable[graph->paramCount].node = VX_NULL;
        graph->paramTable[graph->paramCount].index = 0;
    }

    graph->paramCount++;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_SetParameter(vx_graph graph, vx_uint32 index, vx_reference value)
{
    vx_status status;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    if (index >= VX_MAX_PARAMETERS) return VX_ERROR_INVALID_VALUE;

    status = vxoNode_SetParameter(graph->paramTable[index].node, graph->paramTable[index].index, value);

    if (status != VX_SUCCESS) return status;

    graph->dirty = vx_true_e;

    graph->reverify = vx_false_e;

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphParameterByIndex(vx_graph graph, vx_uint32 index, vx_reference value)
{
    gcmDUMP_API("$VX vxSetGraphParameterByIndex: graph=%p, index=0x%x, value=%p", graph, index, value);

    return vxoGraph_SetParameter(graph, index, value);
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    gcmDUMP_API("$VX vxGetGraphParameterByIndex: graph=%p, index=0x%x", graph, index);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    if (index >= graph->paramCount)
    {
        return (vx_parameter)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_REFERENCE);
    }

    return vxoNode_GetParameter(graph->paramTable[index].node, graph->paramTable[index].index);
}

VX_API_ENTRY vx_bool VX_API_CALL vxIsGraphVerified(vx_graph graph)
{
    gcmDUMP_API("$VX vxIsGraphVerified: graph=%p", graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return vx_false_e;

    return graph->verified;
}


