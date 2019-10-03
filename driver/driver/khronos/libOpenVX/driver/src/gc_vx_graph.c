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
#include "gc_nn_arch_model.h"

#include "gc_vx_graph_optimization.h"
#include <ctype.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_GRAPH

#define PRE_SORT 1
#define OPTIMIZE_GenerateAllNodeIndexTable 1



VX_INTERNAL_API void vxoGraph_Dump(vx_graph graph)
{
    gcmHEADER_ARG("graph=%p", graph);
    if (graph == VX_NULL)
    {
        vxTrace(VX_TRACE_GRAPH, "<graph>null</graph>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)graph);

        vxTrace(VX_TRACE_GRAPH,
            "<graph>\n"
            "   <address>%p</address>\n"
            "</graph>",
            graph);
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoGraph_ClearAllVisitedFlags(vx_graph graph)
{
    vx_uint32 nodeIndex;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        graph->nodeTable[nodeIndex]->visited = vx_false_e;
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoGraph_ClearAllExecutedFlags(vx_graph graph)
{
    vx_uint32 nodeIndex;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        graph->nodeTable[nodeIndex]->executed = vx_false_e;
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_CALLBACK_API void vxoGraph_Destructor(vx_reference ref)
{
    vx_graph graph = (vx_graph)ref;
    vx_uint32    i;
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;

    gcmHEADER_ARG("ref=%p", ref);
    vxmASSERT(vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH));

    if(graph->parentGraph == VX_NULL)
    {
        if (gcvSTATUS_OK != gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex))
        {
            gcmFOOTER_NO();
            return;
        }
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

    /* Release the array. */
    if (graph->inputs)
    {
        vxFree(graph->inputs);
        graph->inputs = VX_NULL;
        graph->inputCount = 0;
    }

    if (graph->outputs)
    {
        vxFree(graph->outputs);
        graph->outputs = VX_NULL;
        graph->outputCount = 0;
    }

    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_uint32 vxoGraph_GetNextNodeIndex(vx_graph graph, vx_uint32 nodeIndex)
{
    gcmHEADER_ARG("graph=%p, nodeIndex=0x%x", graph, nodeIndex);
    vxmASSERT(graph);
    vxmASSERT(nodeIndex < graph->nodeCount);

    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, direction=0x%x, paramRef=%p, nodeIndexTable=%p, nodeCountPtr=%p", graph, direction, paramRef, nodeIndexTable, nodeCountPtr);
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

    if (nodeCount == 0)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_LINK);
        return VX_ERROR_INVALID_LINK;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

    gcmHEADER_ARG("graph=%p, parentNodeIndex=0x%x, childNodeIndex=0x%x, info=%p", graph, parentNodeIndex, childNodeIndex, info);
    vxmASSERT(graph);

    if (parentNodeIndex != VX_MAX_NODES_IN_GRAPH && parentNodeIndex == childNodeIndex)
    {
        vxError("Graph %p has a cycle", graph);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
        return VX_ERROR_INVALID_GRAPH;
    }

    if (info->depth > graph->nodeCount)
    {
        vxmASSERT(0);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
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
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

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
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
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
                        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}


#if PRE_SORT
#if OPTIMIZE_GenerateAllNodeIndexTable
VX_PRIVATE_API vx_bool_e vxoGraph_CheckParentsVisited(vx_node node, vx_graph graph)
{
    vx_uint32 index = 0;
    for (index = 0; index < node->numParents; index++)
    {
        if (!(graph->nodeTable[node->parentNodes[index]])->visited)
            return vx_false_e;
    }
    return vx_true_e;
}

VX_INTERNAL_API void vxoGraph_GenerateAllNodeIndexTable(vx_graph graph)
{
    /* the resolved node is put in leftNodeIndexTable */
    vx_uint32 leftNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 index = 0;

    /* allNodeIndexTable_Count is put in graph->allNodeIndexTable node count, nextNodeCount = graph->nodeCount - lastNodeCount */
    vx_uint32 nextNodeCount = graph->nodeCount;
    vx_uint32 allNodeIndexTableCount = 0;
    vx_uint32 leftNodeCount = 0;

    vx_uint32 nextIndex = 0;
    vx_node nextNode = VX_NULL;
    vx_uint32 childIndex = 0;
    vx_node childNode = VX_NULL;

    if (graph->headNodeCount == 0) return;

    for (index = 0; index < graph->headNodeCount; index++)
    {
        leftNodeIndexTable[leftNodeCount] = graph->headNodeIndexTable[graph->headNodeCount - index - 1];
        leftNodeCount++;
    }

    while (nextNodeCount > 0)
    {
        /* Pop the latest node form leftNodeIndexTable */
        nextIndex = leftNodeIndexTable[leftNodeCount - 1];
        nextNode = graph->nodeTable[nextIndex];
        graph->allNodeIndexTable[allNodeIndexTableCount] = nextIndex;
        nextNode->visited = vx_true_e;
        nextNodeCount--;
        allNodeIndexTableCount++;
        /* remove visited node */
        leftNodeCount--;
        /* Push the resolved children node to leftNodeIndexTable */
        for (index = 0; index < nextNode->numChildren; index++)
        {
            childIndex = nextNode->childNodes[nextNode->numChildren - index - 1];
            childNode = graph->nodeTable[childIndex];
            if (vxoGraph_CheckParentsVisited(childNode, graph))
            {
                leftNodeIndexTable[leftNodeCount] = childIndex;
                leftNodeCount++;
            }
        }
    }
}
#else
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

    gcmHEADER_ARG("graph=%p", graph);

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
    gcmFOOTER_NO();
}
#endif

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

    gcmHEADER_ARG("graph=%p", graph);

    if (!graph->base.context->allTensorNum)
    {
        gcmFOOTER_NO();
        return;
    }
    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(struct _paramNodeItemEx)*graph->base.context->allTensorNum, (gctPOINTER *)&paramNodeTable);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER_NO();
        return;
    }
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
                if (count == graph->base.context->allTensorNum)
                {
                    if (paramNodeTable)
                    {
                        gcoOS_FreeMemory(gcvNULL, paramNodeTable);
                    }

                    gcmFOOTER_NO();
                    return;
                }
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

    if (paramNodeTable)
    {
        gcoOS_FreeMemory(gcvNULL, paramNodeTable);
        paramNodeTable = VX_NULL;
    }
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("opType=0x%x, outputSize=0x%x, kernelSize=0x%x, padTop=0x%x, padBottom=0x%x, poolingSize=0x%x, poolingStride=0x%x, reshuffStride=0x%x, normStride=0x%x, convOut=%p, inputSize=%p",
        opType, outputSize, kernelSize, padTop, padBottom, poolingSize, poolingStride, reshuffStride, normStride, convOut, inputSize);

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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_uint32  GetEsitimateWBSize(
    vx_weights_biases_parameter weightsBiases
    )
{
    vx_enum weightFormat = weightsBiases->wb_base->weights_data_format;
    vx_enum biasFormat = weightsBiases->wb_base->biases_data_format;
    vx_float64 EstimateRatio = WB_COMPRESS_RATIO(weightsBiases) > 1.0f ? 1.05f : (1.25f-1.05f) * (1.0f - WB_COMPRESS_RATIO(weightsBiases)) / (1.0f - 0.02f) + 1.05f;

    vx_uint32 weightSize = weightsBiases->weights_sizes[0] * weightsBiases->weights_sizes[1] * weightsBiases->weights_sizes[2] *
                           weightsBiases->weights_sizes[3] * vxnneGetTypeSize((vx_type_e)weightFormat);

    vx_uint32 biasSize   = weightsBiases->weights_sizes[3] * vxnneGetTypeSize((vx_type_e)biasFormat);

    return gcmALIGN_NP2((vx_uint32)((weightSize + biasSize) * EstimateRatio + 0.5f), CACHE_ALIGNMENT_SIZE);
}

VX_INTERNAL_API vx_status ComputeMNEx(
    vxnne_execution_layer   layer,
    vx_int32                start,
    vx_uint32               count,
    vx_uint32*              outN,
    vx_uint32*              outM,
    vx_uint32*              axiSRAMUsed,
    vx_uint32*              vipSRAMUsed,
    vx_uint32               sRamIn,
    vx_uint32               sRamOut
    )
{
    vx_int32 i;
    vx_uint32  M = 0;
    vx_uint32 totalKernelbufferSize = 0, termA = 0, termB = 0, imageCacheSize = 0;
    vx_uint32 circleHeight = 0, accumPoolingStrideY = 1, inputSize = 0, outputSize = 0;
    vx_uint32 vipSRAMsize = layer->graph->base.context->vipSRAM.size;
    vx_uint32 axiSRAMsize = layer->graph->base.context->axiSRAM.size;
    vxnne_operation_info_s opInfo = {0}, opInfo2 = {0};
    vx_status status;
    vx_bool phase3 = vxoContext_IsFeatureAvailable(layer->graph->base.context, VX_NN_FEATURE_SWTILING_PHASE3);

    if (count < 2) return VX_FAILURE;

    vxnneOperation_GetInfo(layer->operations[start], &opInfo);
    vxnneOperation_GetInfo(layer->operations[start+ count - 1], &opInfo2);

    inputSize    = TENSOR_STRIDE_INDEX(opInfo.input, 2) * TENSOR_SIZE_INDEX(opInfo.input, 2);
    outputSize   = TENSOR_STRIDE_INDEX(opInfo2.output, 2) * TENSOR_SIZE_INDEX(opInfo2.output, 2);


    if (sRamIn == VXNNE_MEM_POOL_TYPE_VIP_SRAM)
    {
        vipSRAMsize -= inputSize;
    }

    if (sRamOut == VXNNE_MEM_POOL_TYPE_VIP_SRAM)
    {
        vipSRAMsize -= outputSize;
    }

    if (sRamIn == VXNNE_MEM_POOL_TYPE_AXI_SRAM)
    {
        axiSRAMsize -= inputSize;
    }

    if (sRamOut == VXNNE_MEM_POOL_TYPE_AXI_SRAM)
    {
        axiSRAMsize -= outputSize;
    }

    if ((((vx_int32)axiSRAMsize) <= 0 && !phase3) ||
        (((vx_int32)vipSRAMsize) <= 0 && phase3))
        return VX_FAILURE;


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

            circleHeight = 0;
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

            if (status != VX_SUCCESS) return status;

            termA += circleHeight * depth * stride;

            accumPoolingStrideY = poolingStrideY * reshuffStrideY * normStrideY;

            termB += depth * stride * accumPoolingStrideY;
        }

        if (i == start && phase3 && (opInfo.target == VXNNE_OPERATION_TARGET_NN) && !sRamIn)
        {
            if (layer->operations[i]->bTransposeIn)
            {
                termA += 32 * (vx_uint32)(ceilf(opInfo.kernelZ/16.f) * gcmMIN(TENSOR_SIZE_INDEX(opInfo.input, 0), 72) * (opInfo.poolSizeY +  opInfo.kernelY - 1) + 15);
                termB += 32 * (vx_uint32)ceilf(opInfo.kernelZ/16.f) * gcmMIN(TENSOR_SIZE_INDEX(opInfo.input, 0), 72) * opInfo.poolStrideY;
            }
            else
            {
                vx_uint32 outTileX = TENSOR_SIZE_INDEX(opInfo.output, 0), inTileX, inDataSize;
                vxmASSERT(opInfo.opType != VXNNE_OPERATOR_FULLYCONNECTED);

                status = ComputeInputSizeEx(
                                                opInfo.opType,
                                                outTileX,
                                                opInfo.kernelX,
                                                0,
                                                0,
                                                opInfo.poolSizeX,
                                                opInfo.poolStrideX,
                                                opInfo.reshuffStrideX,
                                                opInfo.normStrideX,
                                                &outTileX,
                                                VX_NULL
                                                );

                if (status != VX_SUCCESS) return status;

                outTileX = gcmMIN(outTileX, layer->graph->base.context->nnConfig.unifiedFeature.maxTileSize);
                inTileX = outTileX + opInfo.kernelX - 1;
                inDataSize = vxnneGetTypeSize((vx_type_e)TENSOR_DATA_TYPE(opInfo.input));
                /*termA += inTileX * (opInfo.poolSizeX + opInfo.kernelX - 1) * opInfo.kernelZ * inDataSize;*/
                termA += inTileX * (opInfo.poolSizeX + opInfo.kernelX - 1 - opInfo.poolStrideX) * opInfo.kernelZ * inDataSize;
                termB += inTileX * opInfo.kernelZ * inDataSize;
            }
        }

        if (opInfo.weightsBiases)
        {
            totalKernelbufferSize += GetEsitimateWBSize(opInfo.weightsBiases);
        }

    }

    vxmASSERT(vipSRAMsize > 0);

    if (phase3)
    {
        if (layer->operations[start]->bTransposeOut)
        {
            vipSRAMsize -= 32 * 1024;
        }

        if (vipSRAMsize - totalKernelbufferSize > 0 && (vx_int32)(vipSRAMsize - totalKernelbufferSize - termA) > 0)
        {
            M = (vipSRAMsize - totalKernelbufferSize - termA) / termB;
        }
    }
    else
    {
        vxmASSERT(axiSRAMsize > 0 && !phase3);

        if ((vx_int32)(axiSRAMsize - termA) > 0)
        {
            M = (axiSRAMsize - termA) / termB;
        }

        if (M >= 1)
        {
            vxnneOperation_GetInfo(layer->operations[start], &opInfo);

            if (opInfo.target == VXNNE_OPERATION_TARGET_NN && !sRamIn)
            {
                vx_uint32 outTileY, inputTileY;
                vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat;
                vx_arch_perf_s archPerfHandle;

                INITIALIZE_STRUCT(archPerfHandle);
                vxmASSERT(opInfo.opType != VXNNE_OPERATOR_FULLYCONNECTED);
                vxmASSERT(axiSRAMsize > 0 && !phase3);

                outTileY = gcmMIN(TENSOR_SIZE_INDEX(opInfo.output, 1), M);

                status = ComputeInputSizeEx(
                                                opInfo.opType,
                                                outTileY,
                                                opInfo.kernelY,
                                                0,
                                                0,
                                                opInfo.poolSizeY,
                                                opInfo.poolStrideY,
                                                opInfo.reshuffStrideY,
                                                opInfo.normStrideY,
                                                VX_NULL,
                                                &inputTileY
                                                );

                if (status != VX_SUCCESS) return status;

                {
                    vx_uint32 outputDims[3] = {TENSOR_SIZE_INDEX(opInfo.output, 0), outTileY, TENSOR_SIZE_INDEX(opInfo.output, 2)};
                    vx_uint32 inputDims[3]  = {TENSOR_SIZE_INDEX(opInfo.input, 0), inputTileY, TENSOR_SIZE_INDEX(opInfo.input, 2)};

                    calculateArchPerfFromWB(layer->graph->base.context,
                                        &archPerfHandle,
                                        opInfo.weightsBiases,
                                        inputDims,
                                        outputDims,
                                        TENSOR_DATA_TYPE(opInfo.output),
                                        VX_NULL,
                                        vx_true_e,
                                        SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR,
                                        layer->graph->base.context->vipSRAM.size,
                                        (vxnne_operation_target_e)opInfo.target,
                                        (vxnne_operator_e)opInfo.opType);
                }

                outImageTileX   = archPerfHandle.resultInfo.outImageTileXSize;
                outImageTileY   = archPerfHandle.resultInfo.outImageTileYSize;
                interleaveMode  = archPerfHandle.resultInfo.interleaveMode;
                kernelX         = opInfo.weightsBiases->weights_sizes[0];
                kernelY         = opInfo.weightsBiases->weights_sizes[1];
                inImageZ        = TENSOR_SIZE_INDEX(opInfo.input, 2);
                inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

                imageCacheSize =
                    caculate3DTileSize(layer->graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);

            }

            if (imageCacheSize + totalKernelbufferSize > vipSRAMsize)
            {
                return VX_FAILURE;
            }
        }
    }

    if (M < 1)
    {
        return VX_FAILURE;
    }
    else
    {
        if (phase3)
        {
            *axiSRAMUsed = layer->graph->base.context->axiSRAM.size - axiSRAMsize;
            *vipSRAMUsed = termA + M * termB + totalKernelbufferSize;
        }
        else
        {
            vxmASSERT(axiSRAMsize > 0);
            *axiSRAMUsed = termA + M * termB ;
            *vipSRAMUsed = imageCacheSize + totalKernelbufferSize;
        }

        *outN = TENSOR_SIZE_INDEX(opInfo2.output, 0);
        *outM = M;
        return VX_SUCCESS;
    }
}

VX_PRIVATE_API vx_status GenerateTilingOrderInfo(
    vx_context                  context,
    vxnne_segment_tiling_info   tilingSegmentInfo
    )
{
    vx_status         status     = VX_SUCCESS;
    vxnne_tiling_info tilingInfo = VX_NULL, prevTilingInfo = VX_NULL, tempTilingInfo = VX_NULL;
    vx_uint32         orderCount = 0, i, j, k, l, tilingYCount = 0;

    if (gcmIS_ERROR(gcoOS_Allocate(
        gcvNULL,
        sizeof(vxnne_tiling_order_info_s) * tilingSegmentInfo->tilingOrderCount,
        (gctPOINTER*)&tilingSegmentInfo->tilingOrderInfo)))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gcoOS_ZeroMemory(tilingSegmentInfo->tilingOrderInfo, sizeof(vxnne_tiling_order_info_s) * tilingSegmentInfo->tilingOrderCount);

    for (l = 0; l < tilingSegmentInfo->count; l++)
    {
        for(j = 0; j < tilingSegmentInfo->tileYCount; j++)
        {
            tilingInfo = tilingSegmentInfo->tilingInfo  + l * tilingSegmentInfo->tileYCount;

            if (!tilingInfo[j].walked && (tilingInfo[j].output.height > 0))
            {
                tilingSegmentInfo->tilingOrderInfo[orderCount].opID  = l;
                tilingSegmentInfo->tilingOrderInfo[orderCount].subID = j;
                tilingSegmentInfo->tilingOrderInfo[orderCount].tilingInfo = &tilingInfo[j];
                orderCount++;

                prevTilingInfo = &tilingInfo[j];

                for(i = l + 1; i < tilingSegmentInfo->count; i++)
                {
                    tilingInfo = tilingSegmentInfo->tilingInfo  + i * tilingSegmentInfo->tileYCount;
                    for(k = 0; k < tilingSegmentInfo->tileYCount; k++)
                    {
                        if (prevTilingInfo->output.end >= tilingInfo[k].input.end && !tilingInfo[k].walked && (tilingInfo[k].output.height > 0))
                        {
                            /* previous command need a flush*/
                            prevTilingInfo->flush = vx_true_e;

                            tilingSegmentInfo->tilingOrderInfo[orderCount].opID = i;
                            tilingSegmentInfo->tilingOrderInfo[orderCount].subID = k;
                            tilingSegmentInfo->tilingOrderInfo[orderCount].tilingInfo = &tilingInfo[k];
                            orderCount++;

                            tilingInfo[k].walked = vx_true_e;
                            prevTilingInfo =  &tilingInfo[k];
                            break;
                        }
                    }

                    if (k == tilingSegmentInfo->tileYCount) break;
                }
            }
        }
    }
    /* last command need a flush*/
    tilingSegmentInfo->tilingOrderInfo[orderCount - 1].tilingInfo->flush = vx_true_e;

    vxmASSERT(orderCount == tilingSegmentInfo->tilingOrderCount);

    /* rebuild the tiling info table for command initialize */
    tilingYCount = 1;
    for (i = 0; i < tilingSegmentInfo->tilingOrderCount; i++)
    {
        if (i != 0 && tilingSegmentInfo->tilingOrderInfo[i].opID <= tilingSegmentInfo->tilingOrderInfo[i-1].opID)
        {
            tilingYCount++;
        }
    }

    if (gcmIS_ERROR(gcoOS_Allocate(
        gcvNULL,
        sizeof(vxnne_tiling_info_s) * tilingYCount * tilingSegmentInfo->count,
        (gctPOINTER*)&tempTilingInfo)))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gcoOS_ZeroMemory(tempTilingInfo, sizeof(vxnne_tiling_info_s) * tilingYCount * tilingSegmentInfo->count);

    j = 0;
    for (i = 0; i < tilingSegmentInfo->tilingOrderCount; i++)
    {
        tilingInfo = tempTilingInfo + tilingSegmentInfo->tilingOrderInfo[i].opID * tilingYCount;
        if (i != 0 && tilingSegmentInfo->tilingOrderInfo[i].opID <= tilingSegmentInfo->tilingOrderInfo[i-1].opID)
        {
            j++;
        }

        tilingInfo[j] = *tilingSegmentInfo->tilingOrderInfo[i].tilingInfo;
    }

    vxmASSERT(j == tilingYCount - 1);

    gcoOS_Free(gcvNULL, tilingSegmentInfo->tilingInfo);
    tilingSegmentInfo->tilingInfo = tempTilingInfo;
    tilingSegmentInfo->tileYCount = tilingYCount;

OnError:
    return status;
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
    vx_uint32 initY = 0, lastTileHeight;
    vx_status status = VX_SUCCESS;
    vx_uint32  i, j;
    vxnne_tiling_info tilingInfo = VX_NULL;
    vxnne_operation_info_s opInfo, opInfo2;

    tilingBlockInfo->start  = start;
    tilingBlockInfo->count  = count;

    vxnneOperation_GetInfo(layer->operations[start], &opInfo);
    vxnneOperation_GetInfo(layer->operations[start + count - 1], &opInfo2);
    M = gcmMIN(TENSOR_SIZE_INDEX(opInfo2.output, 1), M);

    tilingBlockInfo->tileXCount = 1;
    tilingBlockInfo->tileYCount = gcmALIGN_NP2(TENSOR_SIZE_INDEX(opInfo.output, 1), M) / M + 1;

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
        vx_uint32 inputHeight, outputHeight, tileYCount;

        tilingInfo = tilingBlockInfo->tilingInfo  + i * tilingBlockInfo->tileYCount;

        vxnneOperation_GetInfo(layer->operations[start + i], &opInfo);

        outputHeight = TENSOR_SIZE_INDEX(opInfo.output, 1);
        inputHeight  = TENSOR_SIZE_INDEX(opInfo.input, 1);

        if (i == count - 1)
        {
            initY = outputHeight % M;
            initY = (initY == 0 ? M : initY);
        }

        tileYCount = gcmALIGN_NP2(outputHeight - initY, M) / M + 1;

        if (i > 0)
        {
            /* get circular buffer height */
            status = ComputeInputSizeEx(
                                        opInfo.opType,
                                        M,
                                        opInfo.kernelY,
                                        0,
                                        0,
                                        opInfo.poolSizeY,
                                        opInfo.poolStrideY,
                                        opInfo.reshuffStrideY,
                                        opInfo.normStrideY,
                                        VX_NULL,
                                        &layer->operations[start + i - 1]->circuleBufferHeight
                                        );

            if (status != VX_SUCCESS) goto OnError;
        }

        for(j = 0; j < tileYCount; j++)
        {
            if (j  == 0)
            {
                lastTileHeight = initY;
            }
            else if (j == tileYCount - 1)
            {
                lastTileHeight = (outputHeight - initY) % M;
                lastTileHeight = (lastTileHeight == 0 ? M : lastTileHeight);
            }
            else
            {
                lastTileHeight = M;
            }

            tilingInfo[j].output.start  = (j == 0 ? 0 : tilingInfo[j-1].output.end);
            tilingInfo[j].output.height = lastTileHeight;
            tilingInfo[j].output.width  = TENSOR_SIZE_INDEX(opInfo.output, 0);
            tilingInfo[j].output.end    = tilingInfo[j].output.start + tilingInfo[j].output.height;
            vxmASSERT(tilingInfo[j].output.height > 0);

            tilingInfo[j].padLeft         = opInfo.pad.left;
            tilingInfo[j].input.start     = tilingInfo[j].output.start * opInfo.poolStrideY * opInfo.reshuffStrideY - opInfo.pad.top;

            if ((vx_int32)tilingInfo[j].input.start < 0)
            {
                tilingInfo[j].padTop          = opInfo.pad.top - tilingInfo[j].output.start * opInfo.poolStrideY * opInfo.reshuffStrideY;
                tilingInfo[j].input.start     = 0;
            }

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

            if ((vx_int32)tilingInfo[j].input.height > 0)
            {
                if ((tilingInfo[j].input.start + tilingInfo[j].input.height) > inputHeight)
                {
                    tilingInfo[j].input.height -= (tilingInfo[j].input.start + tilingInfo[j].input.height) - inputHeight;
                }
            }

            if ((vx_int32)tilingInfo[j].input.height <= 0)
            {
                /* handle special case */
                if (opInfo.target == VXNNE_OPERATION_TARGET_TP)
                {
                    tilingInfo[j].input.height  = 1;/*opInfo.pad.top + tilingInfo[j].input.height;*/
                    tilingInfo[j].padTop        = tilingInfo[j].input.height * (-1);
                    tilingInfo[j].input.end     = tilingInfo[j].input.start;
                }
                else
                {
                    vxmASSERT(opInfo.target == VXNNE_OPERATION_TARGET_NN);
                    tilingInfo[j].input.height  = 1;
                    tilingInfo[j].padTop        = opInfo.pad.top;
                    tilingInfo[j].input.end     = tilingInfo[j].input.start;
                }
            }
            else
            {
                tilingInfo[j].input.end     = tilingInfo[j].input.height + tilingInfo[j].input.start;
            }

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

            tilingBlockInfo->tilingOrderCount++;

            if (tilingInfo[j].output.end == outputHeight)
            {
                break;
            }
        }

        /* get the next initY */
        status = ComputeInputSizeEx(
                                    opInfo.opType,
                                    initY,
                                    opInfo.kernelY,
                                    opInfo.pad.top,
                                    0,
                                    opInfo.poolSizeY,
                                    opInfo.poolStrideY,
                                    opInfo.reshuffStrideY,
                                    opInfo.normStrideY,
                                    VX_NULL,
                                    &initY
                                    );

        if (status != VX_SUCCESS) goto OnError;

        if ((vx_int32)initY < 0)
        {
            initY = 0;
        }

        initY = (initY % M == 0 ? M : initY % M);
    }

    status = GenerateTilingOrderInfo(context, tilingBlockInfo);

OnError:
    return status;
}

VX_PRIVATE_API vx_bool SupportAB(vx_context context, vxnne_operation operation)
{
    gcmHEADER_ARG("context=%p, operation=%p", context, operation);
    if ((operation->target == VXNNE_OPERATION_TARGET_NN ||
        (operation->target == VXNNE_OPERATION_TARGET_SH && context->axiSRAM.size > 0) ||
        (operation->target == VXNNE_OPERATION_TARGET_TP &&
          (operation->operatorType != VXNNE_OPERATOR_DILATION_RESHUFFLE &&
           operation->operatorType != VXNNE_OPERATOR_DILATION_UPSAMPLE &&
           operation->operatorType != VXNNE_OPERATOR_DILATION_UPSAMPLE2 &&
           operation->operatorType != VXNNE_OPERATOR_ROIPOOL
       ))) &&
       vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) &&
       !context->options.enableNNLayerDump &&
       (context->vipSRAM.size > VX_VIP_SRAM_IMAGE_STREAM_SIZE)
       )
    {
        /* AB buffer didn't support those TP 4d oprerations like dilation reshuffle & upsample*/
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

VX_PRIVATE_API vx_bool SupportSWTiling(vx_context context, vxnne_operation operation)
{
    gcmHEADER_ARG("context=%p, operation=%p", context, operation);
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
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    gcmFOOTER_ARG("%d", vx_false_e);
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

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, memRequest=%p", graph, start, count, memRequest);

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_REQUEST_INPUT
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_REQUEST_OUTPUT);

        memRequest[k].inputCount = graph->layer->operations[i]->inputsNum;

        for(j = 0; j < memRequest[k].inputCount; j++)
        {
            if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE)
            {
                memRequest[k].inputMemory[j] = &(((vx_image)graph->layer->operations[i]->inputs[j])->memory);
            }
            else
            {
                memRequest[k].inputMemory[j] = &(((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory);
            }
        }

        memRequest[k].outputCount = graph->layer->operations[i]->outputsNum;

        for(j = 0; j < memRequest[k].outputCount; j++)
        {
            if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE)
            {
                memRequest[k].outputMemory[j] = &(((vx_image)graph->layer->operations[i]->outputs[j])->memory);
            }
            else
            {
                memRequest[k].outputMemory[j] = &(((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory);
            }
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, memParamList=%p", graph, start, count, memParamList);

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        memParamList[k].inputCount = graph->layer->operations[i]->inputsNum;

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR)
            {
                memParamList[k].inputMemory[j] = ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory;
            }
            else if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE)
            {
                memParamList[k].inputMemory[j] = ((vx_image)graph->layer->operations[i]->inputs[j])->memory;
            }
        }

        memParamList[k].outputCount = graph->layer->operations[i]->outputsNum;

        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR)
            {
                memParamList[k].outputMemory[j] = ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory;
            }
            else if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE)
            {
                memParamList[k].outputMemory[j] = ((vx_image)graph->layer->operations[i]->outputs[j])->memory;
            }
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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
    vxnne_mem_request requestList;

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, memParamList=%p", graph, start, count, memParamList);

    for (i = start; i < start + count; i++)
    {
        requestList = graph->layer->memRequestList  + i;

        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        vxmASSERT(memParamList[k].inputCount <= VX_MAX_MEM_PARAM
                   && memParamList[k].outputCount <= VX_MAX_MEM_PARAM);

        requestList->inputCount = memParamList[k].inputCount;

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR)
            {
                ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory = memParamList[k].inputMemory[j];
            }
            else if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE)
            {
                ((vx_image)graph->layer->operations[i]->inputs[j])->memory = memParamList[k].inputMemory[j];
            }
        }

        requestList->outputCount = memParamList[k].outputCount;
        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR)
            {
                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory = memParamList[k].outputMemory[j];
            }
            else if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE)
            {
                ((vx_image)graph->layer->operations[i]->outputs[j])->memory = memParamList[k].outputMemory[j];
            }
        }

        gcoOS_ZeroMemory(&requestList->imageCache, sizeof(vx_memory_s));
        gcoOS_ZeroMemory(&requestList->kernelCache, sizeof(vx_memory_s));
        gcoOS_ZeroMemory(&requestList->transposeIn, sizeof(vx_memory_s));
        gcoOS_ZeroMemory(&requestList->transposeOut, sizeof(vx_memory_s));
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status RestorePartialMemoryParamList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vxnne_mem_param         memParamList
    )
{
    vx_uint32 i, j, k, dimIndex;
    vx_memory_s memory;

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, memParamList=%p", graph, start, count, memParamList);

    for (i = start; i < start + count; i++)
    {
        k = i - start;

        vxmASSERT(graph->layer->operations[i]->inputsNum <= VX_MAX_MEM_PARAM
                  && graph->layer->operations[i]->outputsNum <= VX_MAX_MEM_PARAM);

        vxmASSERT(memParamList[k].inputCount <= VX_MAX_MEM_PARAM
                   && memParamList[k].outputCount <= VX_MAX_MEM_PARAM);

        for(j = 0; j < memParamList[k].inputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_TENSOR)
            {
                memory = ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory;

                ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory = memParamList[k].inputMemory[j];

                ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.allocType = memory.allocType;
                ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.physicals[0] = memory.physicals[0];
                ((vx_tensor)graph->layer->operations[i]->inputs[j])->tensorBuffer->memory.logicals[0] = memory.logicals[0];
            }
            else if (graph->layer->operations[i]->inputs[j]->type == VX_TYPE_IMAGE)
            {
                memory = ((vx_image)graph->layer->operations[i]->inputs[j])->memory;

                ((vx_image)graph->layer->operations[i]->inputs[j])->memory = memParamList[k].inputMemory[j];
                ((vx_image)graph->layer->operations[i]->inputs[j])->memory.allocType = memory.allocType;
                ((vx_image)graph->layer->operations[i]->inputs[j])->memory.physicals[0] = memory.physicals[0];
                ((vx_image)graph->layer->operations[i]->inputs[j])->memory.logicals[0] = memory.logicals[0];
            }
        }

        for(j = 0; j < memParamList[k].outputCount; j++)
        {
            vxmASSERT(graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR || graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE);
            if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_TENSOR)
            {
                memory = ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory;

                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory = memParamList[k].outputMemory[j];
                for (dimIndex = 0; dimIndex < VX_CONTEXT_TENSOR_MAX_DIMENSION; dimIndex++)
                {
                    ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.dims[0][dimIndex] = memory.dims[0][dimIndex];
                    ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.strides[0][dimIndex] = memory.strides[0][dimIndex];
                }
                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.transposed = memory.transposed;
                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.allocType = memory.allocType;
                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.physicals[0] = memory.physicals[0];
                ((vx_tensor)graph->layer->operations[i]->outputs[j])->tensorBuffer->memory.logicals[0] = memory.logicals[0];
            }
            else if (graph->layer->operations[i]->outputs[j]->type == VX_TYPE_IMAGE)
            {
                memory = ((vx_image)graph->layer->operations[i]->outputs[j])->memory;

                ((vx_image)graph->layer->operations[i]->outputs[j])->memory = memParamList[k].outputMemory[j];
                ((vx_image)graph->layer->operations[i]->outputs[j])->memory.allocType = memory.allocType;
                ((vx_image)graph->layer->operations[i]->outputs[j])->memory.physicals[0] = memory.physicals[0];
                ((vx_image)graph->layer->operations[i]->outputs[j])->memory.logicals[0] = memory.logicals[0];
            }
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status SetMemoryRequestList(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vx_bool                 esitimateInAB,
    vx_bool                 sRamIn,
    vx_bool                 sRamOut)
{
    vx_uint32 i, j, dims;
    vxnne_mem_request   memRequest = graph->layer->memRequestList + start;
    vx_memory_s         outputMemory[VX_MAX_MEM_REQUEST_OUTPUT];
    vx_enum             allocType, allocType2;

    if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_FEATURE_SWTILING_PHASE3))
    {
        vxmASSERT(graph->base.context->vipSRAM.size != 0);

        allocType = graph->base.context->axiSRAM.size == 0 ? VXNNE_MEM_POOL_TYPE_VIP_SRAM : VXNNE_MEM_POOL_TYPE_SRAM;
    }
    else
    {
        vxmASSERT(graph->base.context->axiSRAM.size != 0);
        allocType = VXNNE_MEM_POOL_TYPE_AXI_SRAM;
    }

    for(j = 0; j < memRequest[count - 1].outputCount; j++)
    {
        outputMemory[j] = *memRequest[count - 1].outputMemory[j];
    }

    for (i = 0; i < count; i++)
    {
        if ((((i == 0 || i == count - 2) && esitimateInAB) ||
            ((i == count - 1) && sRamOut)) && (allocType == VXNNE_MEM_POOL_TYPE_SRAM))
        {
            vxmASSERT(!sRamOut || !esitimateInAB);
            allocType2 = allocType & ~VXNNE_MEM_POOL_TYPE_VIP_SRAM;
        }
        else
        {
            allocType2 = allocType;
        }

        for(j = 0; j < memRequest[i].outputCount; j++)
        {
            dims = memRequest[i].outputMemory[j]->dimCount - 1;
            memRequest[i].outputMemory[j]->allocType    = allocType2;
            memRequest[i].outputMemory[j]->sizes[0]     = gcmALIGN_NP2(memRequest[i].outputMemory[j]->strides[0][dims] * memRequest[i].outputMemory[j]->dims[0][dims], CACHE_ALIGNMENT_SIZE);
            memRequest[i].outputMemory[j]->allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_2);
            vxmASSERT(memRequest[i].outputMemory[j]->sizes[0] > 0);
        }

        if (graph->layer->operations[start + i]->target == VXNNE_OPERATION_TARGET_SH)
        {
            for(j = 0; j < memRequest[i].outputCount; j++)
            {
                memRequest[i].outputMemory[j]->allocType &= ~VXNNE_MEM_POOL_TYPE_VIP_SRAM;
            }

            for(j = 0; j < memRequest[i].inputCount; j++)
            {
                memRequest[i].inputMemory[j]->allocType &= ~VXNNE_MEM_POOL_TYPE_VIP_SRAM;
            }
        }
    }

    if (!sRamOut)
    {
        for(j = 0; j < memRequest[i-1].outputCount; j++)
        {
            *memRequest[i-1].outputMemory[j] = outputMemory[j];
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
    vx_uint32 i = 0, j = 0, k = 0, mergeInputType, mergeOutputType;
    vxnne_operation_info_s opInfo;
    vx_status vStatus = VX_SUCCESS, vStatus1 = VX_SUCCESS;
    vxnne_mem_param memParam = VX_NULL;
    gceSTATUS status = gcvSTATUS_OK;

     gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, collection=%p", graph, start, count, collection);

    status = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_param_s) * count, (gctPOINTER*)&memParam);
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
            vStatus = SetMemoryRequestList(
                graph, i, j, vx_true_e,
                vx_false_e, vx_false_e);

            if (vStatus != VX_SUCCESS) goto OnError;

            for (k = i; k < i + j; k++)
            {
                vxnneOperation_GetInfo(graph->layer->operations[k], &opInfo);

                if (opInfo.target == VXNNE_OPERATION_TARGET_NN)
                {
                    vxnne_mem_request requestList;
                    vx_uint32 transposeSize = 0;
                    vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)graph->layer->operations[k];

                    requestList = graph->layer->memRequestList  + k;
                    if (!(requestList->inputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_SRAM) || (requestList->inputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM))
                    {
                        if (graph->layer->operations[k]->bTransposeIn /*&& requestList->inputMemory[0]->transposed*/)
                        {
                            vx_tensor input = (vx_tensor)(graph->layer->operations[k]->inputs[0]);
                            vx_uint32 inputZ = 0;

                            alignTensorChannelToTransposeChannel(input, graph->layer->operations[k]->transposeInChannel);

                            inputZ = TENSOR_STRIDE_INDEX(input, 3) / TENSOR_STRIDE_INDEX(input, 2);

                            transposeSize = caculateInputTransposeBufferSize(VXNNE_SRAM_CACHE_MODE_FULL_CACHE,
                                                                             convOperation->resultInfo.outImageTileXSize,
                                                                             convOperation->resultInfo.outImageTileYSize,
                                                                             opInfo.weightsBiases->weights_sizes[0],
                                                                             opInfo.weightsBiases->weights_sizes[1],
                                                                             inputZ,
                                                                             convOperation->resultInfo.interleaveMode,
                                                                             graph->base.context->nnConfig.customizedFeature.ddrLatency,
                                                                             graph->layer->operations[k]->transposeInChannel,
                                                                             input->tensorBuffer->dataFormat);

                            gcoOS_ZeroMemory(&requestList->transposeIn, sizeof(vx_memory_s));
                            requestList->transposeIn.lastUseId = requestList->transposeIn.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                            requestList->transposeIn.sizes[0] = transposeSize;
                            requestList->transposeIn.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                            requestList->transposeIn.allocPartial = vx_false_e;
                            requestList->transposeIn.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                            requestList->inputMemory[requestList->inputCount] = &requestList->transposeIn;
                            requestList->inputCount++;
                        }
                        else
                        {
                            requestList->imageCache.lastUseId = requestList->imageCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                            requestList->imageCache.sizes[0]  = graph->layer->operations[k]->esitimateImageCacheSize;
                            requestList->imageCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                            requestList->imageCache.allocPartial = vx_false_e;

                            requestList->imageCache.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);

                            requestList->inputMemory[requestList->inputCount] = &requestList->imageCache;
                            requestList->inputCount++;
                        }
                    }

                    if (graph->layer->operations[k]->bTransposeOut && (!(requestList->outputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_SRAM) || (requestList->outputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)))
                    {
                        vx_tensor output = (vx_tensor)(graph->layer->operations[k]->outputs[0]);

                        alignTensorChannelToTransposeChannel(output, graph->layer->operations[k]->transposeOutChannel);

                        transposeSize = caculateOutTransposeBufferSize(convOperation->resultInfo.outImageTileXSize, convOperation->resultInfo.outImageTileYSize, output->tensorBuffer->dataFormat);
                        requestList->transposeOut.lastUseId = requestList->transposeOut.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                        requestList->transposeOut.sizes[0] = transposeSize;
                        requestList->transposeOut.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                        requestList->transposeOut.allocPartial = vx_false_e;
                        requestList->transposeOut.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                        requestList->inputMemory[requestList->inputCount] = &requestList->transposeOut;
                        requestList->inputCount++;
                    }

                    requestList->kernelCache.lastUseId = requestList->kernelCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                    requestList->kernelCache.sizes[0] = graph->layer->operations[k]->esitimateKernelCacheSize;;
                    requestList->kernelCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                    requestList->kernelCache.allocPriority = VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_3;
                    requestList->kernelCache.allocPartial = vx_true_e;
                    requestList->inputMemory[requestList->inputCount] = &requestList->kernelCache;
                    requestList->inputCount++;
                }
            }

            vStatus1 = vxoMemoryPool_RequestList(graph, graph->layer->memRequestList, graph->layer->base.num_operations, i, j, VX_NULL);

            vxmASSERT(j >= 2);

            mergeInputType  = (graph->layer->memRequestList + i)->outputMemory[0]->allocType;
            mergeOutputType = (graph->layer->memRequestList + i + j - 1)->inputMemory[0]->allocType;

            RestoreMemoryParamList(graph, start, count, memParam);

            if (vStatus1 == VX_SUCCESS)
            {
                vxmASSERT(collection->segmentNum < VX_MAX_SEGMENT_COUNT);
                collection->segments[collection->segmentNum].type = VXNNE_SEGMENT_TYPE_AB;
                collection->segments[collection->segmentNum].start = i;
                collection->segments[collection->segmentNum].count = j;
                collection->segments[collection->segmentNum].end = i + j - 1;
                collection->segments[collection->segmentNum].segmentInfo.ab.mergeInputType = mergeInputType;
                collection->segments[collection->segmentNum].segmentInfo.ab.mergeOutputType = mergeOutputType;
                collection->segmentNum++;

                i = i + j - 1;

                if (collection->segmentNum == VX_MAX_SEGMENT_COUNT)
                {
                    vStatus = VX_SUCCESS;
                    goto OnError;
                }
                break;
            }

        }
    }


OnError:
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

    if (memParam) gcoOS_Free(gcvNULL, memParam);

    gcmFOOTER_ARG("%d", vStatus);
    return vStatus;
}

VX_PRIVATE_API vx_status GetSWTilingCollection(
    vx_graph  graph,
    vx_uint32 start,
    vx_uint32 count,
    vxnne_segment_collection tilingCollection)
{
    vx_uint32 i = 0, num = 0;
    vx_bool terminator, subI;
    vxnne_operation_info_s opInfo1, opInfo2;

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, tilingCollection=%p", graph, start, count, tilingCollection);

    gcoOS_ZeroMemory(tilingCollection, sizeof(vxnne_segment_collection_s));

    for(i = start; i <= start + count; i++)
    {
        terminator = vx_false_e;
        subI       = vx_false_e;

        if ((i == start + count) ||
            !SupportSWTiling(graph->base.context, graph->layer->operations[i]) ||
            (graph->layer->operations[i]->parentOpNum > 1  && num != 0) ||
            (graph->layer->operations[i]->childOpNum > 1 && num == 0) )
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
                subI       = vx_true_e;
            }
            else if (graph->layer->operations[i]->childOpNum > 1)
            {
                i++;
                num++;
                terminator = vx_true_e;
                subI       = vx_true_e;
            }
        }

        if (terminator)
        {
            if (num > 1)
            {
                vxmASSERT(tilingCollection->segmentNum < VX_MAX_SEGMENT_COUNT);

                tilingCollection->segments[tilingCollection->segmentNum].start = i - num;
                tilingCollection->segments[tilingCollection->segmentNum].count = num;
                tilingCollection->segments[tilingCollection->segmentNum].end   = i - 1;

                tilingCollection->segmentNum++;

                if (tilingCollection->segmentNum == VX_MAX_SEGMENT_COUNT)
                {
                    return VX_SUCCESS;
                }

            }

            if (subI)
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status DetectTilingSegments(
    vx_graph                    graph,
    vx_uint32                   start,
    vx_uint32                   count,
    vxnne_segment_collection    abCollection,
    vxnne_segment_collection    tilingCollection)
{
    vx_uint32 i = 0, j = 0, k = 0, l = 0, tilingStart, tilingEnd, M, N, axiSRAMUsed = 0, vipSRAMUsed = 0;
    vx_status status = VX_SUCCESS;
    vx_bool sRamIn, sRamOut, sRamIn2, sRamOut2;
    vxnne_segment_collection_s    tempCollection;
    vx_uint32 mergeInputType, mergeOutputType;

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x, abCollection=%p, tilingCollection=%p", graph, start, count, abCollection, tilingCollection);

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

                        mergeInputType  = sRamIn2 ? (abCollection->segments[j-1].segmentInfo.ab.mergeOutputType) : VXNNE_MEM_POOL_TYPE_ORIG_DDR;
                        mergeOutputType = sRamOut2 ? (abCollection->segments[j].segmentInfo.ab.mergeInputType) : VXNNE_MEM_POOL_TYPE_ORIG_DDR;

                        status = ComputeMNEx(graph->layer, i, k, &N, &M, &axiSRAMUsed, &vipSRAMUsed, mergeInputType, mergeOutputType);
                        if (status == VX_SUCCESS)
                        {
                            vxmASSERT(tilingCollection->segmentNum < VX_MAX_SEGMENT_COUNT);
                            tilingCollection->segments[tilingCollection->segmentNum].type = VXNNE_SEGMENT_TYPE_TILING;
                            tilingCollection->segments[tilingCollection->segmentNum].start = i;
                            tilingCollection->segments[tilingCollection->segmentNum].count = k;
                            tilingCollection->segments[tilingCollection->segmentNum].end = i + k - 1;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.M = M;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.N = N;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.estimateAxiSRAMUsed = axiSRAMUsed;
                            tilingCollection->segments[tilingCollection->segmentNum].segmentInfo.tiling.estimateVipSRAMUsed = vipSRAMUsed;
                            tilingCollection->segmentNum++;

                            if (tilingCollection->segmentNum == VX_MAX_SEGMENT_COUNT)
                            {
                                goto OnError;
                            }

                            i = i + k - 1;
                            break;
                        }
                    }
                }
            }
        }
    }

OnError:
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

    gcmHEADER_ARG("graph=%p, block=%p, segment=%p, sRamIn=%p, sRamOut=%p", graph, block, segment, sRamIn, sRamOut);

    /*empty block*/
    if (segment->count == 0)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    status = SetMemoryRequestList(
        graph, segment->start, segment->count, vx_false_e,
        sRamIn, sRamOut);

    for (i = 0; i < segment->count; i++)
    {
        vxnneOperation_GetInfo(graph->layer->operations[segment->start + i], &opInfo);

        if (opInfo.target == VXNNE_OPERATION_TARGET_NN)
        {
            vxnne_operation operation = graph->layer->operations[segment->start + i];
            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operation;
            vxnne_mem_request requestList = graph->layer->memRequestList + segment->start + i;
            vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat, imageTileSize, kernelbufferSize;
            vx_uint32 outputDims[3] = {TENSOR_VIEW_SIZE_INDEX(opInfo.output, 0),
                                        TENSOR_VIEW_SIZE_INDEX(opInfo.output, 1),
                                        TENSOR_VIEW_SIZE_INDEX(opInfo.output, 2)};
            vx_uint32 transposeSize = 0;

            if (convOperation->resultInfo.kernelsPerCore == 0)
            {
                vxnne_tiling_info_s tilingsInfo[8];
                vxnne_operation op;
                vx_uint32 j;

                j = 0;
                op = operation;
                while (op != VX_NULL)
                {
                    vxnne_convolution_relu_pooling_operation convOperationT = (vxnne_convolution_relu_pooling_operation)op;
                    tilingsInfo[j].input.width   = TENSOR_VIEW_SIZE_INDEX(convOperationT->inputs, 0);
                    tilingsInfo[j].input.height  = TENSOR_VIEW_SIZE_INDEX(convOperationT->inputs, 1);
                    tilingsInfo[j].output.width  = TENSOR_VIEW_SIZE_INDEX(convOperationT->outputs, 0);
                    tilingsInfo[j].output.height = TENSOR_VIEW_SIZE_INDEX(convOperationT->outputs, 1);
                    tilingsInfo[j].padLeft = op->parameter.pad_x_left;
                    tilingsInfo[j].padTop  = op->parameter.pad_y_top;
                    j++;
                    op = op->mGpuNext;
                }
                vxmASSERT(j > 0);

                status = vxnneCalculateConvTilingParam(graph->base.context,
                                                        convOperation,
                                                        tilingsInfo,
                                                        requestList->inputMemory[0]->allocType == VXNNE_MEM_POOL_TYPE_SRAM  ? SW_TILING_FROM_AXI_SRAM : MemPoolTypeToPerfType(requestList->inputMemory[0]->allocType),
                                                        requestList->outputMemory[0]->allocType == VXNNE_MEM_POOL_TYPE_SRAM ? SW_TILING_FROM_AXI_SRAM : MemPoolTypeToPerfType(requestList->outputMemory[0]->allocType),
                                                        vx_false_e,
                                                        j,
                                                        graph->base.context->vipSRAM.size);

                j = 0;
                op = operation;
                while (op != VX_NULL)
                {
                    vxnne_convolution_relu_pooling_operation convOperationT = (vxnne_convolution_relu_pooling_operation)op;
                    convOperationT->resultInfo.outImageTileXSize = tilingsInfo[j].tilingParam.outImageTileXSize;
                    convOperationT->resultInfo.outImageTileYSize = tilingsInfo[j].tilingParam.outImageTileYSize;
                    convOperationT->resultInfo.kernelsPerCore    = tilingsInfo[j].tilingParam.kernelsPerCore;
                    convOperationT->resultInfo.interleaveMode    = tilingsInfo[j].tilingParam.interleaveMode;
                    convOperationT->resultInfo.nnCoreCount       = tilingsInfo[j].tilingParam.nnCoreCount;
                    j++;
                    op = op->mGpuNext;
                }
            }
            vxmASSERT(convOperation->resultInfo.kernelsPerCore != 0);

            vxmONERROR(vxoWeightsBiases_Compress(
                graph->base.context,
                convOperation->weights_biases,
                convOperation->resultInfo.kernelsPerCore,
                outputDims,
                TENSOR_DATA_TYPE(opInfo.output),
                TENSOR_STRIDE_INDEX(opInfo.output, 2)));

            outImageTileX  = convOperation->resultInfo.outImageTileXSize;
            outImageTileY  = convOperation->resultInfo.outImageTileYSize;
            interleaveMode = convOperation->resultInfo.interleaveMode;
            kernelX = opInfo.weightsBiases->weights_sizes[0];
            kernelY = opInfo.weightsBiases->weights_sizes[1];
            inImageZ = TENSOR_SIZE_INDEX(opInfo.input, 2);
            inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

            imageTileSize = caculate3DTileSize(graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);

            kernelbufferSize = (vx_uint32)gcmALIGN_NP2(opInfo.weightsBiases->slice_array[0].kernel_align_stream_size, CACHE_ALIGNMENT_SIZE);

            requestList->kernelCache.lastUseId = requestList->kernelCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
            requestList->kernelCache.sizes[0] = kernelbufferSize;
            requestList->kernelCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
            requestList->kernelCache.allocPriority = VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_3;
            requestList->kernelCache.allocPartial = vx_true_e;
            requestList->inputMemory[requestList->inputCount] = &requestList->kernelCache;
            requestList->inputCount++;

            if (!(requestList->inputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_VIP_SRAM))
            {
                if (operation->bTransposeIn /*&& requestList->inputMemory[0]->transposed*/)
                {
                    vx_tensor input = (vx_tensor)(operation->inputs[0]);

                    vx_uint32 inputZ = 0;

                    alignTensorChannelToTransposeChannel(input, operation->transposeInChannel);

                    inputZ = TENSOR_STRIDE_INDEX(input, 3) / TENSOR_STRIDE_INDEX(input, 2);

                    transposeSize = caculateInputTransposeBufferSize(VXNNE_SRAM_CACHE_MODE_FULL_CACHE,
                                                                     outImageTileX,
                                                                     outImageTileY,
                                                                     kernelX,
                                                                     kernelY,
                                                                     inputZ,
                                                                     interleaveMode,
                                                                     graph->base.context->nnConfig.customizedFeature.ddrLatency,
                                                                     operation->transposeInChannel,
                                                                     input->tensorBuffer->dataFormat);

                    gcoOS_ZeroMemory(&requestList->transposeIn, sizeof(vx_memory_s));
                    requestList->transposeIn.lastUseId = requestList->transposeIn.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                    requestList->transposeIn.sizes[0] = transposeSize;
                    requestList->transposeIn.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                    requestList->transposeIn.allocPartial = vx_false_e;
                    requestList->transposeIn.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                    requestList->inputMemory[requestList->inputCount] = &requestList->transposeIn;
                    requestList->inputCount++;
                }
                else
                {
                    /* From DDR, AXISRAM need to be cached */
                    requestList->imageCache.lastUseId = requestList->imageCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                    requestList->imageCache.sizes[0] = imageTileSize;
                    requestList->imageCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                    requestList->imageCache.allocPartial = vx_false_e;
                    requestList->imageCache.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                    requestList->inputMemory[requestList->inputCount] = &requestList->imageCache;
                    requestList->inputCount++;
                }
            }

            if (operation->bTransposeOut && (!(requestList->outputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_SRAM) || (requestList->outputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_AXI_SRAM)) )
            {
                vx_tensor output = (vx_tensor)(operation->outputs[0]);

                alignTensorChannelToTransposeChannel(output, operation->transposeOutChannel);

                transposeSize = caculateOutTransposeBufferSize(outImageTileX, outImageTileY, output->tensorBuffer->dataFormat);

                gcoOS_ZeroMemory(&requestList->transposeOut, sizeof(vx_memory_s));
                requestList->transposeOut.lastUseId = requestList->transposeOut.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                requestList->transposeOut.sizes[0] = transposeSize;
                requestList->transposeOut.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                requestList->transposeOut.allocPartial = vx_false_e;
                requestList->transposeOut.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                requestList->inputMemory[requestList->inputCount] = &requestList->transposeOut;
                requestList->inputCount++;
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
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
    vx_uint32 i, j, imageTileSize;
    vxnne_operation_info_s opInfo;
    vxnne_tiling_info tilingInfo = VX_NULL;
    vxnne_operation operation = VX_NULL;
    vxnne_mem_request requestList = VX_NULL;
    vx_uint32 inputCount = 0;
    vx_uint32 transposeSize;

    vx_status status = GenerateTilingInfo(
                    graph->base.context,
                    graph->layer,
                    segment->start,
                    segment->count,
                    segment->segmentInfo.tiling.N,
                    segment->segmentInfo.tiling.M,
                    &segment->segmentInfo.tiling);

    gcmHEADER_ARG("graph=%p, block=%p, segment=%p, sRamIn=%p, sRamOut=%p", graph, block, segment, sRamIn, sRamOut);

    if (status != VX_SUCCESS) goto OnError;

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
        requestList[i].outputMemory[0]->dims[0][1] = graph->layer->operations[i + segment->start]->circuleBufferHeight;
        requestList[i].outputMemory[0]->strides[0][2] = graph->layer->operations[i + segment->start]->circuleBufferHeight * requestList[i].outputMemory[0]->strides[0][1];
        for (j = 3; j < VX_CONTEXT_TENSOR_MAX_DIMENSION; j++)
        {
            requestList[i].outputMemory[0]->strides[0][j] = requestList[i].outputMemory[0]->strides[0][j-1] * requestList[i].outputMemory[0]->dims[0][j-1];
        }
        requestList[i].outputMemory[0]->circular = vx_true_e;
    }

    status = SetMemoryRequestList(graph,
        segment->start, segment->count, vx_false_e,
        sRamIn, sRamOut);

    if (status != VX_SUCCESS) goto OnError;

    for(i = 0; i < segment->count; i++)
    {
        tilingInfo = segment->segmentInfo.tiling.tilingInfo  + i * segment->segmentInfo.tiling.tileYCount;
        operation  = graph->layer->operations[i + segment->start];

        vxnneOperation_GetInfo(operation, &opInfo);

        if (operation->operatorType == VXNNE_OPERATOR_CONVOLUTION  || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
        {
            vxmASSERT(operation->target == VXNNE_OPERATION_TARGET_NN);

            status = vxnneCalculateConvTilingParam(
                    graph->base.context,
                    (vxnne_convolution_relu_pooling_operation)operation,
                    tilingInfo,
                    MemPoolTypeToPerfType(requestList[i].inputMemory[0]->allocType),
                    MemPoolTypeToPerfType(requestList[i].outputMemory[0]->allocType),
                    vx_true_e,
                    segment->segmentInfo.tiling.tileYCount,
                    graph->base.context->vipSRAM.size);


            if (status != VX_SUCCESS) goto OnError;

            for (j = 0; j < segment->segmentInfo.tiling.tileYCount; j++)
            {
                vx_uint32 outImageTileX = tilingInfo[j].tilingParam.outImageTileXSize;
                vx_uint32 outImageTileY = tilingInfo[j].tilingParam.outImageTileYSize;


                if (outImageTileY != 0)
                {
                    vx_uint32 interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat;

                    if (operation->bTransposeIn)
                    {
                        vx_uint32 inputZ = 0;
                        vx_tensor input = (vx_tensor)(operation->inputs[0]);

                        alignTensorChannelToTransposeChannel(input, operation->transposeInChannel);

                        inputZ = TENSOR_STRIDE_INDEX(input, 3) / TENSOR_STRIDE_INDEX(input, 2);

                        transposeSize = caculateInputTransposeBufferSize(VXNNE_SRAM_CACHE_MODE_FULL_CACHE,
                                                                        tilingInfo[j].tilingParam.outImageTileXSize,
                                                                        tilingInfo[j].tilingParam.outImageTileYSize,
                                                                        opInfo.weightsBiases->weights_sizes[0],
                                                                        opInfo.weightsBiases->weights_sizes[1],
                                                                        inputZ,
                                                                        tilingInfo[j].tilingParam.interleaveMode,
                                                                        graph->base.context->nnConfig.customizedFeature.ddrLatency,
                                                                        operation->transposeInChannel,
                                                                        input->tensorBuffer->dataFormat);
                        operation->transposeInSize = gcmMAX(operation->transposeInSize, transposeSize);
                    }
                    else
                    {
                        outImageTileX = tilingInfo[j].tilingParam.outImageTileXSize;
                        outImageTileY = tilingInfo[j].tilingParam.outImageTileYSize;
                        interleaveMode = tilingInfo[j].tilingParam.interleaveMode;
                        kernelX = opInfo.weightsBiases->weights_sizes[0];
                        kernelY = opInfo.weightsBiases->weights_sizes[1];
                        inImageZ = TENSOR_SIZE_INDEX(opInfo.input, 2);
                        inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

                        imageTileSize = caculate3DTileSize(graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);
                        operation->imageCacheSize = gcmMAX(operation->imageCacheSize, imageTileSize);
                    }
                }
            }
        }
    }

    vxnneSRAM_Reset(&graph->base.context->vipSRAM);

    for(i = 0; i < segment->count; i++)
    {
        operation  = graph->layer->operations[i + segment->start];
        requestList = graph->layer->memRequestList + segment->start + i;

        if (operation->operatorType == VXNNE_OPERATOR_CONVOLUTION || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
        {
            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operation;

            tilingInfo = segment->segmentInfo.tiling.tilingInfo  + i * segment->segmentInfo.tiling.tileYCount;

            vxnneOperation_GetInfo(operation, &opInfo);

            /* reshuffle weight data and save in wb_base->reshuffleWeightPtr if kernel stride > 1 */
            vxoWeightsBiases_Reshuffle(WB_BASE(opInfo.weightsBiases));

            vxmASSERT(convOperation->swtWeightBiases == VX_NULL);
            convOperation->swtWeightBiases = vxoWeightsBiases_Create(
                                                             graph->base.context,
                                                             WB_BASE(opInfo.weightsBiases),
                                                             WB_BASE_WEIGHT_DIMS(opInfo.weightsBiases),
                                                             VX_NN_CONVOLUTION_LAYER,
                                                             vx_false_e);

            if (convOperation->swtWeightBiases == VX_NULL)
            {
                status = VX_ERROR_NO_RESOURCES;
                goto OnError;
            }

            vxmASSERT(tilingInfo[0].tilingParam.kernelsPerCore != 0);

            status = vxoWeightsBiases_Compress(graph->base.context,
                    convOperation->swtWeightBiases,
                    tilingInfo[0].tilingParam.kernelsPerCore,
                    VX_NULL,
                    TENSOR_DATA_TYPE(opInfo.output),
                    requestList->outputMemory[0]->strides[0][2]);

            if (status != VX_SUCCESS) goto OnError;

            vxmASSERT(operation->kernelCacheSize == 0);
            operation->kernelCacheSize = (vx_uint32)gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(convOperation->swtWeightBiases, 0), CACHE_ALIGNMENT_SIZE);

            requestList->kernelCache.lastUseId = requestList->kernelCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
            requestList->kernelCache.sizes[0] = operation->kernelCacheSize;
            requestList->kernelCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
            requestList->kernelCache.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
            requestList->kernelCache.allocPartial = vx_false_e;

            if (operation->bTransposeIn == 0)
            {
                vxmASSERT(operation->imageCacheSize != 0);
            }
             {
                 vxnne_mem_request requestList1 = graph->layer->memRequestList + segment->start + segment->count - 1;

                 if (!(requestList1->inputMemory[i]->allocType & VXNNE_MEM_POOL_TYPE_SRAM))
                 {
                    if (operation->bTransposeIn /*&& requestList->inputMemory[0]->transposed*/)
                    {
                        gcoOS_ZeroMemory(&requestList->transposeIn, sizeof(vx_memory_s));
                        requestList->transposeIn.lastUseId = requestList->transposeIn.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                        requestList->transposeIn.sizes[0] = operation->transposeInSize;
                        requestList->transposeIn.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                        requestList->transposeIn.allocPartial = vx_false_e;
                        requestList->transposeIn.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                    }
                    else
                    {
                        requestList->imageCache.lastUseId = requestList->imageCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                        requestList->imageCache.sizes[0] = operation->imageCacheSize;
                        requestList->imageCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                        requestList->imageCache.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                        requestList->imageCache.allocPartial = vx_false_e;
                    }
                 }

                 if (operation->bTransposeOut &&
                    (i == segment->count - 1) &&
                    requestList1->outputMemory[0] &&
                    !(requestList1->outputMemory[0]->allocType & VXNNE_MEM_POOL_TYPE_SRAM))
                {
                    vx_tensor output = (vx_tensor)(operation->outputs[0]);

                    alignTensorChannelToTransposeChannel(output, operation->transposeOutChannel);

                    gcoOS_ZeroMemory(&requestList->transposeOut, sizeof(vx_memory_s));
                    requestList->transposeOut.lastUseId = requestList->transposeOut.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                    requestList->transposeOut.sizes[0] = 32 * 1024;
                    requestList->transposeOut.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                    requestList->transposeOut.allocPartial = vx_false_e;
                    requestList->transposeOut.allocPriority = VXNNE_MEM_ALLOC_TYPE_SET_MUST_HAVE(VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1);
                 }

                 requestList1->inputMemory[requestList1->inputCount] = &requestList->kernelCache;
                 requestList1->inputCount++;
                 if (requestList->imageCache.sizes[0] > 0)
                 {
                     requestList1->inputMemory[requestList1->inputCount] = &requestList->imageCache;
                     requestList1->inputCount++;
                 }

                 if (requestList->transposeIn.sizes[0] > 0)
                 {
                     requestList1->inputMemory[requestList1->inputCount] = &requestList->transposeIn;
                     requestList1->inputCount++;
                 }

                 if (requestList->transposeOut.sizes[0] > 0)
                 {
                     requestList1->inputMemory[requestList1->inputCount] = &requestList->transposeOut;
                     requestList1->inputCount++;
                 }
             }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API void  vxnneBlock_Free(
    vxnne_block             block
    )
{
    vx_uint32 j = 0;

    for(j = 0; j < block->segmentNum; j++)
    {
        if (block->segments[j].segmentInfo.tiling.tilingInfo)
        {
            gcoOS_Free(gcvNULL, block->segments[j].segmentInfo.tiling.tilingInfo);
            block->segments[j].segmentInfo.tiling.tilingInfo = gcvNULL;
        }

        if (block->segments[j].segmentInfo.tiling.tilingOrderInfo)
        {
            gcoOS_Free(gcvNULL, block->segments[j].segmentInfo.tiling.tilingOrderInfo);
            block->segments[j].segmentInfo.tiling.tilingOrderInfo = gcvNULL;
        }
    }

    if (block->memParam)
    {
        gcoOS_Free(gcvNULL, block->memParam);
        block->memParam = VX_NULL;
    }
    gcoOS_ZeroMemory(block, sizeof(vxnne_block_s));
}


VX_PRIVATE_API vx_status GenerateBlockInfo(
    vx_graph                graph,
    vxnne_block             block
    )
{
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;
    vx_uint32 i, peakMemSize[VXNNE_MEM_POOL_TYPE_END]={0};
    vxnne_mem_param tempMemParam = VX_NULL;
    vx_bool sRamIn, sRamOut;

    gcmHEADER_ARG("graph=%p, block=%p", graph, block);

    for (i = 0; i < block->segmentNum; i++)
    {
        block->count += block->segments[i].count;
    }

    block->start = block->segments[0].start;

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
            block->totalCommandCount += block->segments[i].segmentInfo.tiling.tilingOrderCount;
        }
        else
        {
            vxmASSERT(0);
            status =  VX_ERROR_NOT_SUPPORTED;
            goto OnError;
        }
    }

    {
        status = vxoMemoryPool_RequestList(graph, graph->layer->memRequestList, graph->layer->base.num_operations, block->start, block->count, peakMemSize);
        if (status != VX_SUCCESS)
        {
            vx_uint32 j = 0;
            vxnne_operation_info_s opInfo;
            vxInfo("WARINGING: Fail To GenerateBlockInfo with RequireMemoryList \n");
            for (i = 0; i < block->segmentNum; i++)
            {
                if (block->segments[i].type == VXNNE_SEGMENT_TYPE_TILING)
                {
                    for (j = 0; j < block->segments[i].count; j++)
                    {
                        vxnneOperation_GetInfo(graph->layer->operations[j + block->segments[i].start], &opInfo);
                        if (opInfo.target == VXNNE_OPERATION_TARGET_NN)
                        {
                            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)graph->layer->operations[j + block->segments[i].start];

                            vx_uint32 EstimateKernelSize = GetEsitimateWBSize(opInfo.weightsBiases);
                            vx_uint32 CompressedKernelSize = (vx_uint32)gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(convOperation->swtWeightBiases, 0), CACHE_ALIGNMENT_SIZE);

                            vxoReference_Release((vx_reference_ptr)&convOperation->swtWeightBiases, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL);

                            convOperation->swtWeightBiases = VX_NULL;

                            vxInfo("%d: %d, %d, IC %d EstimateKernelSize %d CompressedKernelSize %d  %f%% \n", j + block->segments[i].start,
                                opInfo.input->tensorBuffer->memory.allocType &  VXNNE_MEM_POOL_TYPE_SRAM ? opInfo.input->tensorBuffer->memory.sizes[0]: 0,
                                opInfo.output->tensorBuffer->memory.allocType &  VXNNE_MEM_POOL_TYPE_SRAM ? opInfo.output->tensorBuffer->memory.sizes[0]: 0,
                                opInfo.input->tensorBuffer->memory.allocType &  VXNNE_MEM_POOL_TYPE_SRAM ? 0 : graph->layer->operations[j + block->segments[i].start]->imageCacheSize,
                                EstimateKernelSize, CompressedKernelSize, (vx_float32)(CompressedKernelSize/EstimateKernelSize)*100);
                        }
                    }
                    vxInfo("=======================================================================\n");
                }
                else
                {
                    vxmASSERT(block->segments[i].type == VXNNE_SEGMENT_TYPE_AB);
                    for (j = 0; j < block->segments[i].count; j++)
                    {
                        if (graph->layer->operations[j + block->segments[i].start]->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)graph->layer->operations[j + block->segments[i].start];
                            vxoWeightsBiases_Decompress(graph->base.context, convOperation->weights_biases);
                        }
                    }
                }
            }

            goto OnError;
        }
        else
        {
            /* check memory allocation has overlap or not */
            vx_uint32 ii, jj, kk;
            vx_memory *checkArray;
            vx_uint32 *checkCount;
#define     MAX_INOUT_NUM  128
            status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vx_memory) * graph->layer->base.num_operations * MAX_INOUT_NUM, (gctPOINTER*)&checkArray);
            if (gcmIS_ERROR(status)) goto OnError;
            gcoOS_ZeroMemory(checkArray, gcmSIZEOF(vx_memory) * graph->layer->base.num_operations * MAX_INOUT_NUM);
            status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vx_uint32) * graph->layer->base.num_operations, (gctPOINTER*)&checkCount);
            if (gcmIS_ERROR(status)) goto OnError;
            gcoOS_ZeroMemory(checkCount, gcmSIZEOF(vx_uint32) * graph->layer->base.num_operations);

            for (ii = block->start; ii < block->start+block->count; ii++)
            {
                for (jj = 0; jj < graph->layer->memRequestList[ii].outputCount; jj++)
                {
                    if (graph->layer->memRequestList[ii].outputMemory[jj]->allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR ||
                        graph->layer->memRequestList[ii].outputMemory[jj]->allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
                    {
                        continue;
                    }
                    else
                    {
                        for (kk = graph->layer->memRequestList[ii].outputMemory[jj]->firstUseId; kk <= graph->layer->memRequestList[ii].outputMemory[jj]->lastUseId; kk++)
                        {
                            checkArray[kk * MAX_INOUT_NUM + checkCount[kk]] = graph->layer->memRequestList[ii].outputMemory[jj];
                            checkCount[kk]++;
                            vxmASSERT(checkCount[kk] < MAX_INOUT_NUM);
                        }
                    }
                }

                for (jj = 0; jj < graph->layer->memRequestList[ii].inputCount; jj++)
                {
                    if (graph->layer->memRequestList[ii].inputMemory[jj]->allocType == VXNNE_MEM_POOL_TYPE_ORIG_DDR ||
                        graph->layer->memRequestList[ii].inputMemory[jj]->allocType == VXNNE_MEM_POOL_TYPE_VIRTUAL_DDR)
                    {
                        continue;
                    }
                    else
                    {
                        for (kk = graph->layer->memRequestList[ii].inputMemory[jj]->firstUseId; kk <= graph->layer->memRequestList[ii].inputMemory[jj]->lastUseId; kk++)
                        {
                            checkArray[kk * MAX_INOUT_NUM + checkCount[kk]] = graph->layer->memRequestList[ii].inputMemory[jj];
                            checkCount[kk]++;
                            vxmASSERT(checkCount[kk] < MAX_INOUT_NUM);
                        }
                    }
                }
            }

            for (ii = 0; ii < graph->layer->base.num_operations; ii++)
            {
                if (checkCount[ii] == 0) continue;

                for (jj = 0; jj < checkCount[ii]; jj++)
                {
                    for (kk = jj+1; kk < checkCount[ii]; kk++)
                    {
                        vx_memory m1 = checkArray[ii * MAX_INOUT_NUM + jj];
                        vx_memory m2 = checkArray[ii * MAX_INOUT_NUM + kk];
                        if (!m1->allocated || !m2->allocated ||
                            VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(m1->allocType) != VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(m2->allocType) ||
                            m1 == m2)
                        {
                            continue;
                        }
                        else
                        {
                            vx_uint32 phys1 = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(m1->allocType) && VXNNE_MEM_POOL_TYPE_IS_CACHE(m1->allocType) ? graph->base.context->vipSRAM.physBase + m1->physicals[0] : m1->physicals[0];
                            vx_uint32 phys2 = VXNNE_MEM_POOL_TYPE_WITHOUT_CACHE(m2->allocType) && VXNNE_MEM_POOL_TYPE_IS_CACHE(m2->allocType) ? graph->base.context->vipSRAM.physBase + m2->physicals[0] : m2->physicals[0];
                            if ((phys1 > phys2 && phys1 < phys2 + m2->sizes[0]) || (phys2 > phys1 && phys2 < phys1 + m1->sizes[0]) || phys1 == phys2)
                            {
                                vxmASSERT(0);
                            }
                        }
                    }
                }
            }

            gcoOS_FreeMemory(gcvNULL, checkArray);
            gcoOS_FreeMemory(gcvNULL, checkCount);
        }

        /* temp solution for wb's reshuffleWeightPtr free, need remove it when solve reference count issue */
        {
            vx_uint32 j = 0;

            for (i = 0; i < block->segmentNum; i++)
            {
                if (block->segments[i].type == VXNNE_SEGMENT_TYPE_TILING)
                {
                    for (j = 0; j < block->segments[i].count; j++)
                    {
                        if (graph->layer->operations[j + block->segments[i].start]->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)graph->layer->operations[j + block->segments[i].start];
                            vxoWeightsBiases_Clear(convOperation->swtWeightBiases);
                        }
                    }
                }
                else if (block->segments[i].type == VXNNE_SEGMENT_TYPE_AB)
                {
                    for (j = 0; j < block->segments[i].count; j++)
                    {
                        if (graph->layer->operations[j + block->segments[i].start]->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)graph->layer->operations[j + block->segments[i].start];
                            vxoWeightsBiases_Clear(convOperation->weights_biases);
                        }
                    }
                }
            }
        }

        for (i = 0; i < block->segmentNum; i++)
        {
            vx_uint32 j;
            vxnne_mem_request memReqList;

            for (j = 0; j < block->segments[i].count; j++)
            {
                vxnne_operation operation = graph->layer->operations[j + block->segments[i].start];

                memReqList = graph->layer->memRequestList  + j + block->segments[i].start;
                if (memReqList->imageCache.physicals[0] != 0)
                {
                    operation->imageCacheSize = (vx_uint32)memReqList->imageCache.sizes[0];
                    operation->imageCacheStart = memReqList->imageCache.physicals[0];
                    vxmASSERT(memReqList->imageCache.allocPartial == vx_false_e);
                    operation->imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                }
                else
                {
                    operation->imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    operation->imageCacheSize = 0;
                }

                if (memReqList->kernelCache.physicals[0] != 0)
                {
                    operation->kernelCacheSize = (vx_uint32)memReqList->kernelCache.sizes[0];
                    operation->kernelCacheStart = memReqList->kernelCache.physicals[0];
                    operation->kernelCacheMode = memReqList->kernelCache.allocPartial ? VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE : VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                }
                else
                {
                    operation->kernelCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    operation->kernelCacheSize = 0;
                }

                if (memReqList->transposeIn.physicals[0] != 0)
                {
                    operation->transposeInSize  = (vx_uint32)memReqList->transposeIn.sizes[0];
                    operation->transposeInStart = memReqList->transposeIn.physicals[0];
                    operation->transposeInMode  = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                }
                else
                {
                    operation->transposeInMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    operation->transposeInSize = 0;
                }

                if (memReqList->transposeOut.physicals[0] != 0)
                {
                    operation->transposeOutSize  = (vx_uint32)memReqList->transposeOut.sizes[0];
                    operation->transposeOutStart = memReqList->transposeOut.physicals[0];
                    operation->transposeOutMode  = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;

                    memReqList->outputMemory[0]->transposed = vx_true_e;
                }
                else
                {
                    operation->transposeOutMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    operation->transposeOutSize = 0;
                }
            }
        }

        status = GetMemoryParamList(graph, block->start, block->count, block->memParam);
        if (status != VX_SUCCESS) goto OnError;

        status = RestorePartialMemoryParamList(graph, block->start, block->count, tempMemParam);
        if (status != VX_SUCCESS) goto OnError;
    }



#if SW_TILING_DEBUG

    {
        vxnne_tiling_info_s* tilingInfo;
        vx_uint32  j, k;
        vxnne_operation_info_s opInfo;
        char* opTarget[5] = {"NONE", "SH", "NN", "TP", "SW"};
        char* memType[VXNNE_MEM_POOL_TYPE_END] = {"DD", "DD", "VS", "", "AS"};

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

                for(j = block->segments[i].start; j < block->segments[i].start + block->segments[i].count; j++)
                {
                    vxInfo("[%s -> %s]  ", memType[block->memParam[j - block->start].inputMemory[0].allocType],
                                           memType[block->memParam[j - block->start].outputMemory[0].allocType]);
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

                        if (tilingInfo[j].output.height > 0)
                        {
                            vxInfo("[%s%3d(%3d,%3d)(%3d) ->%s%3d(%3d,%3d)(%3d) P(%2d) F(%d)]    ",
                                memType[block->memParam[k - block->start].inputMemory[0].allocType], tilingInfo[j].input.height, tilingInfo[j].input.start, tilingInfo[j].input.end,
                                block->memParam[k - block->start].inputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM ? block->memParam[k - block->start].inputMemory[0].dims[0][1] : 0,
                                memType[block->memParam[k - block->start].outputMemory[0].allocType], tilingInfo[j].output.height, tilingInfo[j].output.start, tilingInfo[j].output.end,
                                block->memParam[k - block->start].outputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM ? block->memParam[k - block->start].outputMemory[0].dims[0][1]:0,
                                tilingInfo[j].padTop,
                                tilingInfo[j].flush
                                );
                        }
                        else
                        {
                            vxInfo("                                                          ");
                        }

                    }

                    vxInfo("\n");
                }

                vxInfo("\nAXISRAM: Estimate used %d  %f%%  VIPSRAM: Estimate used %d  %f%% M = %d\n",
                        block->segments[i].segmentInfo.tiling.estimateAxiSRAMUsed, graph->base.context->axiSRAM.size == 0 ? 0 : ((vx_float32)block->segments[i].segmentInfo.tiling.estimateAxiSRAMUsed / graph->base.context->axiSRAM.size) * 100,
                        block->segments[i].segmentInfo.tiling.estimateVipSRAMUsed, graph->base.context->vipSRAM.size == 0 ? 0 : ((vx_float32)block->segments[i].segmentInfo.tiling.estimateVipSRAMUsed / graph->base.context->vipSRAM.size) * 100,
                        block->segments[i].segmentInfo.tiling.M
                        );
            }

            vxInfo("\nCacheState = [IC KC][ ");
            for(j = 0; j < block->segments[i].count; j++)
            {
                vxInfo("%c %c", graph->layer->operations[j + block->segments[i].start]->imageCacheSize == 0 ? '0' : graph->layer->operations[j + block->segments[i].start]->imageCacheMode == VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE ? 'P' : 'F',
                                graph->layer->operations[j + block->segments[i].start]->kernelCacheSize == 0 ? '0' : graph->layer->operations[j + block->segments[i].start]->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE ? 'P' : 'F');
                if (j !=  block->segments[i].count - 1)
                {
                    vxInfo(",");
                }
            }
            vxInfo("]");

            vxInfo("\nAXISRAM: Peak used %d  %f%% VIPSRAM: Peak used %d  %f%%\n",
                   peakMemSize[VXNNE_MEM_POOL_TYPE_AXI_SRAM], graph->base.context->axiSRAM.size == 0 ? 0 : ((vx_float32)peakMemSize[VXNNE_MEM_POOL_TYPE_AXI_SRAM] / graph->base.context->axiSRAM.size) * 100,
                   peakMemSize[VXNNE_MEM_POOL_TYPE_VIP_SRAM], graph->base.context->vipSRAM.size == 0 ? 0 : ((vx_float32)peakMemSize[VXNNE_MEM_POOL_TYPE_VIP_SRAM] / graph->base.context->vipSRAM.size) * 100);

        }

        vxInfo("======================= End block  ==============================\n");

    }
#endif

OnError:

    if (status != VX_SUCCESS)
    {
        /* roll back */
        if (tempMemParam)
            RestoreMemoryParamList(graph, block->start, block->count, tempMemParam);

        vxnneBlock_Free(block);
    }

    GetMemoryRequestList(graph, 0, graph->layer->base.num_operations, graph->layer->memRequestList);

    if (tempMemParam) gcoOS_Free(gcvNULL, tempMemParam);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vxnne_segment GetNextSegment(
        vx_uint32                   *abCurrent,
        vxnne_segment_collection    abCollection,
        vx_uint32                   *tilingCurrent,
        vxnne_segment_collection    tilingCollection)
{
    vxnne_segment next;

    gcmHEADER_ARG("abCurrent=%p, abCollection=%p, tilingCurrent=%p, tilingCollection=%p", abCurrent, abCollection, tilingCurrent, tilingCollection);

    if (*abCurrent >= abCollection->segmentNum &&
        *tilingCurrent >= tilingCollection->segmentNum)
    {
        gcmFOOTER_NO();
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

    gcmFOOTER_ARG("next=%p", next);
    return next;

}

VX_PRIVATE_API vx_status GenerateBlocks(
    vx_graph                    graph,
    vxnne_segment_collection    abCollection,
    vxnne_segment_collection    tilingCollection
    )
{
    vx_status     status = VX_SUCCESS;
    vx_uint32     i, abCurrent = 0, tilingCurrent = 0, maxSegmentNum = 0;
    vxnne_segment next;
    vxnne_block   block = VX_NULL;

    gcmHEADER_ARG("graph=%p, abCollection=%p, tilingCollection=%p", graph, abCollection, tilingCollection);

    vxmASSERT(graph->layer->blockNum < VX_MAX_BLOCK_COUNT);

    block = &graph->layer->blocks[graph->layer->blockNum];

    if (abCollection->segmentNum + tilingCollection->segmentNum == 0)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    maxSegmentNum = abCollection->segmentNum + tilingCollection->segmentNum;

    for(i = 0; i < maxSegmentNum; i++)
    {
        next = GetNextSegment(&abCurrent, abCollection, &tilingCurrent, tilingCollection);
        vxmASSERT(next);

        if (block->segmentNum == 0)
        {
            block->segments[block->segmentNum] = *next;
            block->segmentNum++;
            vxmASSERT(block->segmentNum <= VX_MAX_BLOCK_SEGMENT_COUNT);
        }
        else
        {
            if (block->segments[block->segmentNum - 1].end == next->start)
            {
                vxmASSERT(block->segmentNum < VX_MAX_BLOCK_SEGMENT_COUNT);
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
                vxmASSERT(block->segmentNum <= VX_MAX_BLOCK_SEGMENT_COUNT);
            }
            else
            {
                /* block prepared */
                status = GenerateBlockInfo(graph, block);

                if (status == VX_SUCCESS)
                {
                    graph->layer->blockNum++;
                    if (graph->layer->blockNum == VX_MAX_BLOCK_COUNT)
                    {
                        status = VX_SUCCESS;
                        goto OnError;
                    }
                }

                block = &graph->layer->blocks[graph->layer->blockNum];
                block->segments[block->segmentNum] = *next;
                block->segmentNum++;
                vxmASSERT(block->segmentNum <= VX_MAX_BLOCK_SEGMENT_COUNT);
            }
        }
    }

    /*last block prepared */
    status = GenerateBlockInfo(graph, block);
    if (status == VX_SUCCESS)
    {
        /* pointer to next block */
        graph->layer->blockNum++;
    }

    status = VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return status;
}

VX_PRIVATE_API vx_status AnalyzeBlock(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vx_uint32               SWTilingOption
    )
{
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus = gcvSTATUS_OK;

    vxnne_segment_collection abCollection = VX_NULL, tilingCollection = VX_NULL;

    gcmHEADER_ARG("graph=%p, start=0x%x, count=0x%x", graph, start, count);

    if (graph->layer->blockNum >= VX_MAX_BLOCK_COUNT)
    {
        goto OnError;
    }

    vxInfo("Analyze block (%d - %d)\n", start, start + count - 1);

    gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_segment_collection_s), (gctPOINTER*)&abCollection);
    if (gcmIS_ERROR(gStatus))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_segment_collection_s), (gctPOINTER*)&tilingCollection);
    if (gcmIS_ERROR(gStatus))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gcoOS_ZeroMemory(abCollection, sizeof(vxnne_segment_collection_s));
    gcoOS_ZeroMemory(tilingCollection, sizeof(vxnne_segment_collection_s));

    if ((SWTilingOption == VX_SWTILING_OPTION_ALL) ||
        (SWTilingOption == VX_SWTILING_OPTION_AB))
    {
        status  = DetectABSegments(graph, start, count, abCollection);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }

    if ((SWTilingOption == VX_SWTILING_OPTION_ALL) ||
        (SWTilingOption == VX_SWTILING_OPTION_TILING))
    {
        status  = DetectTilingSegments(graph, start, count, abCollection, tilingCollection);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }

    status = GenerateBlocks(graph, abCollection, tilingCollection);
    if (status != VX_SUCCESS)
    {
        goto OnError;
    }

OnError:
    if (abCollection)
    {
        gcoOS_Free(gcvNULL, abCollection);
    }

    if (tilingCollection)
    {
        gcoOS_Free(gcvNULL, tilingCollection);
    }

    gcmFOOTER_ARG("%d", status);
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
    gcmHEADER_ARG("graph=%p, block=%p, segment=%p", graph, block, segment);

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

        opCommand->inputTile.sRAM                    = block->memParam[bufferID].inputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM;

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

        opCommand->outputTile.x         = 0;
        opCommand->outputTile.y         = 0;
        opCommand->outputTile.width     = TENSOR_SIZE_INDEX(opInfo.output, 0);
        opCommand->outputTile.height    = TENSOR_SIZE_INDEX(opInfo.output, 1);
        opCommand->outputTile.xStride   = TENSOR_STRIDE_INDEX(opInfo.output, 0);
        opCommand->outputTile.yStride   = TENSOR_STRIDE_INDEX(opInfo.output, 1);
        opCommand->outputTile.zStride   = TENSOR_STRIDE_INDEX(opInfo.output, 2);

        opCommand->outputTile.sRAM                    = block->memParam[bufferID].outputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM;
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

        opCommand->cmdInfo.wb           = opInfo.weightsBiases;
        opCommand->cmdInfo.flush        = vx_true_e;
        opCommand->dump                 = vxOpCommandDump;

        opCommand->cmdInfo.imageCacheSize  = graph->layer->operations[i]->imageCacheSize;
        opCommand->cmdInfo.imageCacheStart = graph->layer->operations[i]->imageCacheStart;
        opCommand->cmdInfo.imageCacheMode  = graph->layer->operations[i]->imageCacheMode;
        opCommand->cmdInfo.kernelCacheSize  = graph->layer->operations[i]->kernelCacheSize;
        opCommand->cmdInfo.kernelCacheStart = graph->layer->operations[i]->kernelCacheStart;
        opCommand->cmdInfo.kernelCacheMode  = graph->layer->operations[i]->kernelCacheMode;

        opCommand->cmdInfo.transposeInSize = graph->layer->operations[i]->transposeInSize;
        opCommand->cmdInfo.transposeInStart = graph->layer->operations[i]->transposeInStart;
        opCommand->cmdInfo.transposeInMode = graph->layer->operations[i]->transposeInMode;
        opCommand->cmdInfo.transposeInChannel = graph->layer->operations[i]->transposeInChannel;
        opCommand->cmdInfo.transposeOutSize = graph->layer->operations[i]->transposeOutSize;
        opCommand->cmdInfo.transposeOutStart = graph->layer->operations[i]->transposeOutStart;
        opCommand->cmdInfo.transposeOutMode = graph->layer->operations[i]->transposeOutMode;
        opCommand->cmdInfo.transposeOutChannel = graph->layer->operations[i]->transposeOutChannel;


        graph->layer->opIndicesNum++;
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status InitializeTilingSegmentCommands(
    vx_graph                graph,
    vxnne_block             block,
    vxnne_segment           segment)
{
    vx_status status = VX_SUCCESS;

    vxnne_tiling_info tilingInfo = VX_NULL;
    vx_uint32 l, k, i, bufferID, viewOffset = 0;
    vx_bool   flush = vx_false_e;

    vxnne_operation_command         opCommand = VX_NULL, prevOpCommand = VX_NULL;
    vxnne_segment_tiling_info_s*    segmentTiling = &segment->segmentInfo.tiling;

    gcmHEADER_ARG("graph=%p, block=%p, segment=%p", graph, block, segment);

    for (l = 0; l < segmentTiling->tileXCount; l++)
    {
        vx_uint32 kStart, iStart, offset = 0;

        for (kStart = 0; kStart < segmentTiling->count; kStart++)
        {
            for (iStart = 0; iStart < (kStart == segmentTiling->count - 1 ? segmentTiling->tileYCount : 1); iStart++)
            {
                for (i = iStart, k = kStart; ((vx_int32)k >= 0) && i < segmentTiling->tileYCount; k--, i++)
                {
                    tilingInfo = segmentTiling->tilingInfo + k * segmentTiling->tileYCount;

                    if (tilingInfo[i].output.height != 0)
                    {
                        prevOpCommand = opCommand;
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

                        opCommand->inputTile.sRAM        = block->memParam[bufferID].inputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM;

                        if (opCommand->inputTile.sRAM)
                        {
                            viewOffset = 0;
                            if (opCommand->operationID == segment->start)
                            {
                                vxoTensor_GetTensorViewOffset((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], &viewOffset);

                                opCommand->inputTile.xStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 0);
                                opCommand->inputTile.yStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 1);
                                opCommand->inputTile.zStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->inputs[0], 2);
                            }
                            else
                            {
                                opCommand->inputTile.xStride     = block->memParam[bufferID].inputMemory[0].strides[0][0];
                                opCommand->inputTile.yStride     = block->memParam[bufferID].inputMemory[0].strides[0][1];
                                opCommand->inputTile.zStride     = block->memParam[bufferID].inputMemory[0].strides[0][2];
                            }

                            opCommand->inputTile.logicalBase = block->memParam[bufferID].inputMemory[0].logicals[0] + viewOffset;
                            opCommand->inputTile.physical  = block->memParam[bufferID].inputMemory[0].physicals[0] + viewOffset;
                            opCommand->inputTile.logical   = block->memParam[bufferID].inputMemory[0].logicals[0] + viewOffset;

                            opCommand->inputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].inputMemory[0].sizes[0];
                            opCommand->inputTile.circularBufEndAddrPlus1 = block->memParam[bufferID].inputMemory[0].physicals[0] + opCommand->inputTile.circleBufferSize;

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

                        opCommand->operation   = graph->layer->operations[opCommand->operationID];

                        opCommand->cmdInfo.transposeInSize = opCommand->operation->transposeInSize;
                        opCommand->cmdInfo.transposeInStart = opCommand->operation->transposeInStart;
                        opCommand->cmdInfo.transposeInMode = opCommand->operation->transposeInMode;
                        opCommand->cmdInfo.transposeInChannel = opCommand->operation->transposeInChannel;
                        opCommand->cmdInfo.transposeOutSize = opCommand->operation->transposeOutSize;
                        opCommand->cmdInfo.transposeOutStart = opCommand->operation->transposeOutStart;
                        opCommand->cmdInfo.transposeOutMode = opCommand->operation->transposeOutMode;
                        opCommand->cmdInfo.transposeOutChannel = opCommand->operation->transposeOutChannel;

                        if (opCommand->cmdInfo.transposeInMode == VXNNE_SRAM_CACHE_MODE_FULL_CACHE)
                        {
                            offset = (opCommand->inputTile.x * opCommand->inputTile.xStride
                                         + opCommand->inputTile.y * opCommand->inputTile.yStride) * opCommand->cmdInfo.transposeInChannel;
                        }
                        else
                        {
                            offset = opCommand->inputTile.x * opCommand->inputTile.xStride
                                        + opCommand->inputTile.y * opCommand->inputTile.yStride;
                        }

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

                        opCommand->outputTile.sRAM        = block->memParam[bufferID].outputMemory[0].allocType & VXNNE_MEM_POOL_TYPE_SRAM;

                        if (opCommand->outputTile.sRAM)
                        {
                            viewOffset = 0;
                            if (opCommand->operationID == (segment->start + segment->count - 1))
                            {
                                vxoTensor_GetTensorViewOffset((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], &viewOffset);

                                opCommand->outputTile.xStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 0);
                                opCommand->outputTile.yStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 1);
                                opCommand->outputTile.zStride     = TENSOR_STRIDE_INDEX((vx_tensor)graph->layer->operations[opCommand->operationID]->outputs[0], 2);
                            }
                            else
                            {
                                opCommand->outputTile.xStride     = block->memParam[bufferID].outputMemory[0].strides[0][0];
                                opCommand->outputTile.yStride     = block->memParam[bufferID].outputMemory[0].strides[0][1];
                                opCommand->outputTile.zStride     = block->memParam[bufferID].outputMemory[0].strides[0][2];
                            }

                            opCommand->outputTile.logicalBase = block->memParam[bufferID].outputMemory[0].logicals[0] + viewOffset;
                            opCommand->outputTile.physical  = block->memParam[bufferID].outputMemory[0].physicals[0] + viewOffset;
                            opCommand->outputTile.logical   = block->memParam[bufferID].outputMemory[0].logicals[0] + viewOffset;

                            opCommand->outputTile.circleBufferSize        = (vx_uint32)block->memParam[bufferID].outputMemory[0].sizes[0];
                            opCommand->outputTile.circularBufEndAddrPlus1 = block->memParam[bufferID].outputMemory[0].physicals[0] + opCommand->outputTile.circleBufferSize;

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

                        if (opCommand->cmdInfo.transposeOutMode == VXNNE_SRAM_CACHE_MODE_FULL_CACHE)
                        {
                            offset = (opCommand->outputTile.x * opCommand->outputTile.xStride
                                         + opCommand->outputTile.y * opCommand->outputTile.yStride) * opCommand->cmdInfo.transposeOutChannel;
                        }
                        else
                        {
                            offset = opCommand->outputTile.x * opCommand->outputTile.xStride
                                        + opCommand->outputTile.y * opCommand->outputTile.yStride;
                        }

                        if (opCommand->outputTile.sRAM && opCommand->outputTile.circleBufferSize != 0)
                        {
                            offset = offset % opCommand->outputTile.circleBufferSize;
                        }

                        opCommand->outputTile.physical  = opCommand->outputTile.physical + offset;
                        opCommand->outputTile.logical   = (vx_uint8*)opCommand->outputTile.logical + offset;

                        vxmASSERT(!opCommand->outputTile.sRAM || opCommand->outputTile.physical < opCommand->outputTile.circularBufEndAddrPlus1);

                        opCommand->cmdInfo.tilingParam = tilingInfo[i].tilingParam;
                        opCommand->cmdInfo.imageCacheSize  = opCommand->operation->imageCacheSize;
                        opCommand->cmdInfo.imageCacheStart = opCommand->operation->imageCacheStart;
                        opCommand->cmdInfo.imageCacheMode  = opCommand->operation->imageCacheSize == 0 ? VXNNE_SRAM_CACHE_MODE_NONE : VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                        opCommand->cmdInfo.kernelCacheSize  = opCommand->operation->kernelCacheSize;
                        opCommand->cmdInfo.kernelCacheStart = opCommand->operation->kernelCacheStart;
                        opCommand->cmdInfo.kernelCacheMode  = opCommand->inputTile.y == 0 ? VXNNE_SRAM_CACHE_MODE_FULL_CACHE : VXNNE_SRAM_CACHE_MODE_STREAM_CACHE;

                        if (opCommand->operation->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            opCommand->cmdInfo.wb  = ((vxnne_convolution_relu_pooling_operation)opCommand->operation)->swtWeightBiases;
                        }

                        opCommand->dump        = vxOpCommandDump;
                        graph->layer->opIndicesNum++;
                    }

                    flush |= tilingInfo[i].flush;

                    if ((i == segmentTiling->tileYCount - 1) || k == 0)
                    {
                        vxmASSERT(opCommand->operation);
                        opCommand->cmdInfo.flush = flush;
                        flush = vx_false_e;
                    }


                    if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_USC_INVALIDATE_CACHE_LINE_FIX) &&
                         prevOpCommand && prevOpCommand->operation->target != opCommand->operation->target)
                    {
                        prevOpCommand->cmdInfo.flush = vx_true_e;
                    }

#if SW_TILING_DEBUG
                    /* update the flush state for debug*/
                    tilingInfo[i].flush = opCommand->cmdInfo.flush;
#endif
                }
            }
        }
    }

#if SW_TILING_DEBUG
    {
        for(l = 0; l < segmentTiling->tileYCount; l++)
        {
            for(k = 0; k < segmentTiling->count; k++)
            {
                tilingInfo = segmentTiling->tilingInfo + k * segmentTiling->tileYCount;

                if (tilingInfo[l].output.height > 0)
                {
                    vxInfo("F(%d) ", tilingInfo[l].flush);
                }
                else
                {
                    vxInfo("     ");
                }
            }

            vxInfo("\n");
        }
    }
#endif
    gcmFOOTER_ARG("%d", status);
    return status;

}

VX_PRIVATE_API vx_status InitializeBlock(
    vx_graph                graph,
    vxnne_block             block)
{
    vx_uint32 i;
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p, block=%p", graph, block);

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

    gcmFOOTER_ARG("%d", status);
    return status;
}


VX_PRIVATE_API vx_status DetectBlocks(
    vx_graph                graph,
    vx_uint32               start,
    vx_uint32               count,
    vx_uint32               swTilingOption)
{
    vx_status status = VX_SUCCESS;
    vx_bool   terminator = vx_false_e;
    vx_uint32 i = 0, begin = start, num = 0;
    vxnne_operation_info_s opInfo1 = {0}, opInfo2 = {0};

    for (i = start; i <= start + count; i++)
    {
        terminator = vx_false_e;

        if ((i == start + count) ||
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
                else if (num >= 1)
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
                        num++;
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
                num++;
                terminator = vx_true_e;
            }
        }

        if (terminator)
        {
            if (num > 1)
            {
                status = AnalyzeBlock(graph, begin, num, swTilingOption);
                if (status != VX_SUCCESS) goto OnError;
            }

            begin = i + 1;
            num = 0;
        }
        else
        {
            num++;
        }
    }

OnError:
    return status;
}


enum {
    BLOCK_ARGS_INIT_FLAG = 0,
    BLOCK_ARGS_START_FLAG,
    BLOCK_ARGS_LAST_FLAG,
    BLOCK_ARGS_TYPE_FLAG
};

VX_PRIVATE_API vx_status DetectBlocksFromConfig(
    vx_graph                    graph,
    gctSTRING                   config
    )
{
    vx_status status = VX_SUCCESS;
    char *s = config;
    char buf[32];
    int stage = BLOCK_ARGS_INIT_FLAG, len = 0, start = 0, last = 0, type = 0;

    while (s && *s != '\0')
    {
        if (*s == '[')
        {
            len = 0;
            stage = BLOCK_ARGS_START_FLAG;
        }
        else if (*s == ']')
        {
            buf[len] = '\0';
            len = 0;
            if (stage == BLOCK_ARGS_TYPE_FLAG)
            {
                if (!strcmp(buf, "AB"))
                {
                    type = VX_SWTILING_OPTION_AB;
                }
                else if (!strcmp(buf, "SWT"))
                {
                    type = VX_SWTILING_OPTION_TILING;
                }
                else if (!strcmp(buf, "AUTO"))
                {
                    type = VX_SWTILING_OPTION_ALL;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    vxInfo("ERROR: invalid block type: %s\n", buf);
                    goto OnError;
                }
                stage = BLOCK_ARGS_INIT_FLAG;
                vxInfo("block[%d]: {start: %d, last: %d, type: %s}\n", graph->layer->blockNum, start, last, buf);

                last = gcmMIN(last, (vx_int32)(graph->layer->base.num_operations - 1));

                status = DetectBlocks(graph, start, last - start + 1, type);
                if (status != VX_SUCCESS) goto OnError;
            }
        }
        else if (*s == ',')
        {
            if (stage == BLOCK_ARGS_START_FLAG)
            {
                buf[len] = '\0';
                len = 0;
                start = atoi(buf);
                stage = BLOCK_ARGS_LAST_FLAG;
            }
            else if (stage == BLOCK_ARGS_LAST_FLAG)
            {
                buf[len] = '\0';
                len = 0;
                last = atoi(buf);
                stage = BLOCK_ARGS_TYPE_FLAG;
            }
        }
        else if (isdigit(*s))
        {
            if ((stage == BLOCK_ARGS_START_FLAG) || (stage == BLOCK_ARGS_LAST_FLAG))
            {
                buf[len++] = *s;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                vxInfo("ERROR: invalid star/last segment argument: %c\n", *s);
                goto OnError;
            }
        }
        else if (isalpha(*s))
        {
            if (stage == BLOCK_ARGS_TYPE_FLAG)
            {
                buf[len++] = *s;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                vxInfo("ERROR: invalid block type argument: %c\n", *s);
                goto OnError;
            }
        }
        s++;
    }

    return VX_SUCCESS;

OnError:
    graph->layer->blockNum = 0;
    memset(graph->layer->blocks, 0, sizeof(vxnne_block_s) * VX_MAX_BLOCK_COUNT);
    return status;
}

vx_status DetectInImageNonZeroRatioFromConfig(
    vx_graph graph,
    char * config
    )
{
    char *s = config;
    char buf[32];
    int index = 0, len = 0, abs_op_id = 0;

    s = config;
    while (s && *s != '\0')
    {
        if (*s == '[')
        {
            len = 0;
            index = 0;
        }
        else if (*s == ']')
        {
            buf[len] = '\0';
            if ((1 == index) && (len > 0))
            {
                graph->layer->operations[abs_op_id]->imgNonZeroRatio = atof(buf);
            }
            else
            {
                vxInfo("ERROR: invalid input: %s\n", s);
                goto OnError;
            }
            len = 0;
            index = 0;
        }
        else if (*s == ',')
        {
            buf[len] = '\0';
            if (len > 0)
            {
                if (index == 0)
                {
                    abs_op_id = atoi(buf);
                    index++;
                }
                else
                {
                    graph->layer->operations[abs_op_id]->imgNonZeroRatio = atof(buf);
                    index = 0;
                }
            }
            else
            {
                vxInfo("ERROR: invalid input: %s\n", s);
                goto OnError;
            }
            len = 0;
         }
         else if (isdigit(*s) || *s == '.')
         {
             buf[len++] = *s;
         }
         else if (*s == ' ' || *s == '\t')
         {
         }
         else
         {
             vxInfo("ERROR: invalid input: %s\n", s);
             goto OnError;
         }
         s++;
    }

    return VX_SUCCESS;

OnError:
    return VX_FAILURE;
}

VX_INTERNAL_API vx_status vxoGraph_VerifyTiling(vx_graph graph)
{
    vx_uint32 i = 0, j = 0, k = 0;

    vxnne_execution_layer layer = VX_NULL;
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;
    vx_uint32 s = 0, e = 0, maxOpCommandCount = 0;
    vxnne_operation_info_s opInfo;
    gctSTRING blocksConfig = gcvNULL;

    gcmHEADER_ARG("graph=%p", graph);

    if (!graph->layer)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    layer = (vxnne_execution_layer)graph->layer;

#if SW_TILING_DEBUG
    {
        char* opTarget[6] = {"NONE", "SH", "NN", "TP", "SW", "SC"};
        vx_uint32 offsetIn = 0, offsetOut = 0, inputSize, outputSize;
        vxInfo("---------------------------Begin VerifyTiling -------------------------\n");
        vxInfo("AXI-SRAM = %d Bytes VIP-SRAM = %d Bytes\n", graph->base.context->axiSRAM.size, graph->base.context->vipSRAM.size);
        for(i = 0; i < graph->layer->base.num_operations; i++)
        {
            offsetIn = 0; offsetOut = 0;
            inputSize = 0; outputSize = 0;
            vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);
            if (opInfo.input)
            {
                vxoTensor_GetTensorViewOffset(opInfo.input, &offsetIn);
                inputSize = TENSOR_SIZE_INDEX(opInfo.input, 3) * TENSOR_STRIDE_INDEX(opInfo.input, 3);
            }
            if (opInfo.output)
            {
                vxoTensor_GetTensorViewOffset(opInfo.output, &offsetOut);
                outputSize = TENSOR_SIZE_INDEX(opInfo.output, 3) * TENSOR_STRIDE_INDEX(opInfo.output, 3);
            }

            vxInfo("%3d %s [%3d %3d %3d %3d 0x%p(0x%p, %u) -> %3d %3d %3d %3d 0x%p(0x%p, %u)] (k=%d i=%d O=%d)",
                        i,
                        opTarget[graph->layer->operations[i]->target],
                        opInfo.input  ? TENSOR_SIZE_INDEX(opInfo.input, 0) : 0,
                        opInfo.input  ? TENSOR_SIZE_INDEX(opInfo.input, 1) : 0,
                        opInfo.input  ? TENSOR_SIZE_INDEX(opInfo.input, 2) : 0,
                        opInfo.input  ? TENSOR_SIZE_INDEX(opInfo.input, 3) : 0,
                        opInfo.input  ? opInfo.input->tensorBuffer + offsetIn : 0,
                        opInfo.input  ? opInfo.input->tensorBuffer : 0,
                        opInfo.input  ? offsetIn : 0,
                        opInfo.output ? TENSOR_SIZE_INDEX(opInfo.output, 0) : 0,
                        opInfo.output ? TENSOR_SIZE_INDEX(opInfo.output, 1) : 0,
                        opInfo.output ? TENSOR_SIZE_INDEX(opInfo.output, 2) : 0,
                        opInfo.output ? TENSOR_SIZE_INDEX(opInfo.output, 3) : 0,
                        opInfo.output ? opInfo.output->tensorBuffer+ offsetOut : 0,
                        opInfo.output ? opInfo.output->tensorBuffer : 0,
                        opInfo.output ? offsetOut : 0,
                        opInfo.weightsBiases ?  GetEsitimateWBSize(opInfo.weightsBiases) : 0,
                        inputSize,
                        outputSize);

            if (graph->layer->operations[i]->childOpNum >= 1  || graph->layer->operations[i]->parentOpNum >= 1 )
            {
                if (graph->layer->operations[i]->parentOpNum > 0) vxInfo(" P[");
                for(j = 0; j < graph->layer->operations[i]->parentOpNum; j++)
                {
                    vxInfo("%3d%s", graph->layer->operations[i]->parentOps[j]->absoluteOperationID, j == graph->layer->operations[i]->parentOpNum - 1 ? "]":",");
                }
                if (graph->layer->operations[i]->childOpNum > 0)vxInfo(" C[");
                for(j = 0; j < graph->layer->operations[i]->childOpNum; j++)
                {
                    vxInfo("%3d%s", graph->layer->operations[i]->childOps[j]->absoluteOperationID, j == graph->layer->operations[i]->childOpNum - 1 ? "]":",");
                }
            }

            vxInfo("\n");
        }
    }
#endif

    /* colloect the input/output param */
    gStatus = gcoOS_Allocate(gcvNULL, sizeof(vxnne_mem_request_s) * graph->layer->base.num_operations, (gctPOINTER*)&graph->layer->memRequestList);
    if (gcmIS_ERROR(gStatus))
    {
        status = VX_ERROR_NO_MEMORY;
        goto OnError;
    }

    gcoOS_ZeroMemory(graph->layer->memRequestList, sizeof(vxnne_mem_request_s) * graph->layer->base.num_operations);

    status = GetMemoryRequestList(graph, 0, graph->layer->base.num_operations, graph->layer->memRequestList);
    if (status != VX_SUCCESS) goto OnError;

    gStatus = gcoOS_Allocate(gcvNULL,
                            sizeof(vxnne_block_s) * VX_MAX_BLOCK_COUNT,
                            (gctPOINTER*)&graph->layer->blocks);

    if (gcmIS_ERROR(gStatus))
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    gcoOS_ZeroMemory(graph->layer->blocks, sizeof(vxnne_block_s) * VX_MAX_BLOCK_COUNT);

    /* initialize operations perfHandle with DDR->DDR kerelPerCore, tx, ty */
    for (i = 0; i < graph->layer->base.num_operations; i++)
    {
        vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);

        if (opInfo.target == VXNNE_OPERATION_TARGET_NN)
        {
            vx_uint32 outputDims[3] = {TENSOR_SIZE_INDEX(opInfo.output, 0), TENSOR_SIZE_INDEX(opInfo.output, 1), TENSOR_SIZE_INDEX(opInfo.output, 2)};
            vx_uint32 inputDims[3]  = {TENSOR_SIZE_INDEX(opInfo.input, 0), TENSOR_SIZE_INDEX(opInfo.input, 1), TENSOR_SIZE_INDEX(opInfo.input, 2)};
            vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat;
            vx_arch_perf_s archPerfHandle;

            INITIALIZE_STRUCT(archPerfHandle);
            calculateArchPerfFromWB(graph->base.context,
                                &archPerfHandle,
                                opInfo.weightsBiases,
                                inputDims,
                                outputDims,
                                TENSOR_DATA_TYPE(opInfo.output),
                                VX_NULL,
                                vx_true_e,
                                SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR,
                                graph->base.context->vipSRAM.size,
                                (vxnne_operation_target_e)opInfo.target,
                                (vxnne_operator_e)opInfo.opType);

            outImageTileX   = archPerfHandle.resultInfo.outImageTileXSize;
            outImageTileY   = archPerfHandle.resultInfo.outImageTileYSize;
            interleaveMode  = archPerfHandle.resultInfo.interleaveMode;
            kernelX         = opInfo.weightsBiases->weights_sizes[0];
            kernelY         = opInfo.weightsBiases->weights_sizes[1];
            inImageZ        = TENSOR_SIZE_INDEX(opInfo.input, 2);
            inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

            graph->layer->operations[i]->esitimateImageCacheSize =
                caculate3DTileSize(graph->base.context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);
            graph->layer->operations[i]->esitimateKernelCacheSize = GetEsitimateWBSize(opInfo.weightsBiases);
        }
    }

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_BLOCKS_CONFIG", &blocksConfig)) && blocksConfig)
    {
        status = DetectBlocksFromConfig(graph, blocksConfig);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vx_uint32 swtilingOption = graph->base.context->options.enableSwtilingPhase1;
        status = DetectBlocks(graph, 0, layer->base.num_operations, swtilingOption);
        if (status != VX_SUCCESS) goto OnError;
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
            vxnne_operation operation = layer->operations[i];

            if (operation->target == VXNNE_OPERATION_TARGET_NN)
            {
                vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operation;
                vx_uint32 outputDims[3] = {TENSOR_VIEW_SIZE_INDEX(convOperation->outputs, 0),
                                           TENSOR_VIEW_SIZE_INDEX(convOperation->outputs, 1),
                                           TENSOR_VIEW_SIZE_INDEX(convOperation->outputs, 2)};

                if (convOperation->resultInfo.kernelsPerCore == 0)
                {
                    vxnne_tiling_info_s tilingsInfo[8];
                    vxnne_operation op;
                    vx_uint32 j;

                    j = 0;
                    op = operation;
                    while (op != VX_NULL)
                    {
                        vxnne_convolution_relu_pooling_operation convOperationT = (vxnne_convolution_relu_pooling_operation)op;
                        tilingsInfo[j].input.width   = TENSOR_VIEW_SIZE_INDEX(convOperationT->inputs, 0);
                        tilingsInfo[j].input.height  = TENSOR_VIEW_SIZE_INDEX(convOperationT->inputs, 1);
                        tilingsInfo[j].output.width  = TENSOR_VIEW_SIZE_INDEX(convOperationT->outputs, 0);
                        tilingsInfo[j].output.height = TENSOR_VIEW_SIZE_INDEX(convOperationT->outputs, 1);
                        tilingsInfo[j].padLeft = op->parameter.pad_x_left;
                        tilingsInfo[j].padTop  = op->parameter.pad_y_top;
                        j++;
                        op = op->mGpuNext;
                    }
                    vxmASSERT(j > 0);

                    status = vxnneCalculateConvTilingParam(graph->base.context,
                                                           convOperation,
                                                           tilingsInfo,
                                                           SW_TILING_FROM_DDR,
                                                           SW_TILING_FROM_DDR,
                                                           vx_false_e,
                                                           j,
                                                           graph->base.context->vipSRAM.size);

                    j = 0;
                    op = operation;
                    while (op != VX_NULL)
                    {
                        vxnne_convolution_relu_pooling_operation convOperationT = (vxnne_convolution_relu_pooling_operation)op;
                        convOperationT->resultInfo.outImageTileXSize = tilingsInfo[j].tilingParam.outImageTileXSize;
                        convOperationT->resultInfo.outImageTileYSize = tilingsInfo[j].tilingParam.outImageTileYSize;
                        convOperationT->resultInfo.kernelsPerCore    = tilingsInfo[j].tilingParam.kernelsPerCore;
                        convOperationT->resultInfo.interleaveMode    = tilingsInfo[j].tilingParam.interleaveMode;
                        convOperationT->resultInfo.nnCoreCount       = tilingsInfo[j].tilingParam.nnCoreCount;
                        j++;
                        op = op->mGpuNext;
                    }
                }
                vxmASSERT(convOperation->resultInfo.kernelsPerCore != 0);

                vxmONERROR(vxoWeightsBiases_Compress(
                    graph->base.context,
                    convOperation->weights_biases,
                    convOperation->resultInfo.kernelsPerCore,
                    outputDims,
                    TENSOR_DATA_TYPE(convOperation->outputs),
                    TENSOR_STRIDE_INDEX(convOperation->outputs, 2)));

                vxoWeightsBiases_Clear(convOperation->weights_biases);
            }

            for (k = 0; k < layer->operations[i]->batchCount; k++)
            {
                layer->opIndices[layer->opIndicesNum].blockFlag = VXNNE_BLOCK_FLAG_NONE;
                layer->opIndices[layer->opIndicesNum].dump = vxOpCommandDump;
                layer->opIndices[layer->opIndicesNum].batchID = k;
                layer->opIndices[layer->opIndicesNum].operationID = i;
                layer->opIndices[layer->opIndicesNum].operation  = operation;
                vxnneOperation_InitializeCommand(graph->base.context, graph, operation, &layer->opIndices[layer->opIndicesNum]);

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
            vxnneBlock_Free(&layer->blocks[i]);
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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoGraph_AllocateContiguousMemory(vx_graph graph)
{
    vxnne_execution_layer layer = VX_NULL;
    vxnne_operation_info_s opInfo;
    vx_uint32 i, SumTotalKernelBufferSize;
    vx_uint64 CpuPhysicalAddr, GpuPhysicalAddr;
    vx_context context = vxGetContext((vx_reference)graph);

    gcmHEADER_ARG("graph=%p", graph);

    if (!graph->layer)
    {
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    layer = (vxnne_execution_layer)graph->layer;

    for(i = 0; i < graph->layer->base.num_operations; i++)
    {
        vxnneOperation_GetInfo(graph->layer->operations[i], &opInfo);
        context->SumTotalKernelBufferSize += (opInfo.weightsBiases ?  GetEsitimateWBSize(opInfo.weightsBiases) : 0);
    }

    context->Node = (gcsSURF_NODE_PTR *)vxAllocate(sizeof(gcsSURF_NODE_PTR));
    context->Physical = (gctUINT32 *)vxAllocate(sizeof(gctUINT32));
    context->Logical = (vx_uint8_ptr *)vxAllocate(sizeof(vx_uint8_ptr));
    SumTotalKernelBufferSize = context->SumTotalKernelBufferSize;
    for (i = 0; i < graph->layer->base.num_operations; i++)
    {
        if (gcmIS_SUCCESS((gcoVX_AllocateMemoryExAddAllocflag(&SumTotalKernelBufferSize, gcvSURF_VERTEX, 64, gcvALLOC_FLAG_CONTIGUOUS, context->Physical, (vx_ptr_ptr)context->Logical, (vx_uint32 *)&CpuPhysicalAddr, context->Node))))
        {
            gcoOS_ZeroMemory(*context->Logical, SumTotalKernelBufferSize);
            context->CurrentContigousSize = SumTotalKernelBufferSize;
            gcoOS_CPUPhysicalToGPUPhysical((gctPHYS_ADDR_T)CpuPhysicalAddr, (gctPHYS_ADDR_T *)&GpuPhysicalAddr);
            gcmVERIFY_OK(gcoVX_SetRemapAddress((vx_uint32)GpuPhysicalAddr, (vx_uint32)GpuPhysicalAddr + SumTotalKernelBufferSize, gcvVX_OCB_REMAP));

            break;
        }
        vxnneOperation_GetInfo(graph->layer->operations[graph->layer->base.num_operations - i - 1], &opInfo);
        SumTotalKernelBufferSize -= (opInfo.weightsBiases ?  GetEsitimateWBSize(opInfo.weightsBiases) : 0);
    }

    return VX_SUCCESS;
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
    vx_int32 diff = 0, sum = 1;
    vx_uint32 count = gpuCount;
    vx_uint32 splitCount = 0;
    vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;
    vxnne_operation_info_s opInfo;
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("operation=%p, gpuCount=0x%x", operation, gpuCount);
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
        diff = (vx_int32)inputY - (vx_int32)operation->parameter.pad_y_bottom * 2;
        if (opInfo.poolStrideY > 0)
        {
            diff = diff / opInfo.poolStrideY;
            sum = opInfo.poolStrideY;
        }
        if (opInfo.poolSizeY > 0)
        {
            diff = diff / opInfo.poolSizeY;
            sum = opInfo.poolSizeY * sum;
        }

        if (((outputY / count) == 0) || (diff <= (vx_int32)opInfo.kernelY) ||
        (inputY <= (opInfo.kernelY + sum)))
        {
            /* this CONV operation doesn't support split on Y-axis */
            count -= 2;
        }
        else
        {
            splitCount = count;
            break;
        }
    }

OnError:
    gcmFOOTER_ARG("splitCount=0x%x", splitCount);
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

    gcmHEADER_ARG("operation=%p, gpuCount=0x%x, splitCount=%p", operation, gpuCount, splitCount);

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
    else
    {
        gcmFOOTER_ARG("%d", vx_false_e);
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

    gcmFOOTER_ARG("0x%x", OpFlag && splitFlag);
    return OpFlag && splitFlag;

OnError:
    gcmFOOTER_ARG("%d", vx_false_e);
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

    gcmHEADER_ARG("node=%p, gpuCount=0x%x", node, gpuCount);

    vxoMultiGpu_FreeMemory(node);

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
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoMultiGpu_FreeMemory(vx_node node)
{
    vx_uint32 i = 0, j = 0;
    vx_status status = VX_SUCCESS;
    vxnne_operation operation = VX_NULL;
    vx_reference reference = VX_NULL;

    gcmHEADER_ARG("node=%p", node);

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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoMultiGPU_CopyOperationBase(
    vxnne_operation srcOperation,
    vxnne_operation dstOperation
    )
{
    vx_uint32 i = 0;
    gcmHEADER_ARG("srcOperation=%p, dstOperation=%p", srcOperation, dstOperation);

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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoMultiGPU_SplitOperation(
    vx_node node,
    vxnne_operation srcOperation
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("node=%p, srcOperation=%p", node, srcOperation);

    if (VXNNE_OPERATION_TARGET_TP == srcOperation->target)
    {
        vxnne_tp_operation dstTpOP = &node->mGpuTpOperation[node->mGpuTpOpCnt];
        vxnne_tp_operation srcTpOp = (vxnne_tp_operation)srcOperation;

        vxoMultiGPU_CopyOperationBase(srcOperation, &dstTpOP->base);

        dstTpOP->input = VX_NULL;
        dstTpOP->input_ex = srcTpOp->input_ex;
        dstTpOP->weights_biases = srcTpOp->weights_biases;
        dstTpOP->output = VX_NULL;
        dstTpOP->base.parameter = srcTpOp->base.parameter;
    }
    else if ((VXNNE_OPERATION_TARGET_NN == srcOperation->target) &&
        (VXNNE_OPERATOR_CONVOLUTION == srcOperation->operatorType))
    {
        vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)srcOperation;
        vxnne_convolution_relu_pooling_operation dstNNOP = &node->mGpuNNOperation[node->mGpuNNOpCnt];
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
    else
    {
        vxmASSERT(0);
        vxError("mutliple GPU can't support this operation type: %d\n", srcOperation->target);
    }
    gcmFOOTER_ARG("%d", status);
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

    gcmHEADER_ARG("node=%p, dstOperation=%p, srcOperation=%p, input=%p, output=%p, splitCount=0x%x, gpuIndex=0x%x",
        node, dstOperation, srcOperation, input, output, splitCount, gpuIndex);

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

    if ((origInputTensor == VX_NULL) || (origOutputTensor == VX_NULL))
    {
        vxError("%s[%d]: origInputTensor or origOutputTensor is NULL\n", __FUNCTION__, __LINE__);
        vxmONERROR(VX_ERROR_NO_MEMORY);
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
    gcmFOOTER_ARG("%d", status);
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
    vx_uint32 kzGroup = srcTpOp->base.parameter.tp_value->u32[0];
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

    gcmHEADER_ARG("node=%p, dstOperation=%p, srcOperation=%p, splitCount=0x%x, gpuIndex=0x%x", node, dstOperation, srcOperation, splitCount, gpuIndex);

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
        dstOperation->base.parameter.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(newWeightBias->slice_num * sizeof(vx_tp_value_cmd_s));
        if (dstOperation->base.parameter.tp_value != VX_NULL)
        {
            for (i = 0; i < newWeightBias->slice_num; i++)
            {
                values.u32[0] = kzgroup;
                values.u32[1] = zoffset;
                vxMemCopy(&dstOperation->base.parameter.tp_value[i], &values, sizeof(vx_tp_value_cmd_s));
                zoffset += newWeightBias->slice_array[i].z_count;
            }
        }
    }
    else
    {
        vx_tp_value_cmd_s values ;
        vx_uint32 kzoffset = 0, kzoffset2 = 0, zoffset = 0;
        memset(&values,0,sizeof(vx_tp_value_cmd_s));
        if (0 == srcTpOp->base.parameter.tp_value->e32[0])
        {
            dstOperation->base.parameter.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(newWeightBias->slice_num * sizeof(vx_tp_value_cmd_s));
            if (dstOperation->base.parameter.tp_value != VX_NULL)
            {
                vx_uint32 kzgroup = newWeightBias->weights_sizes[2] / newWeightBias->slice_array[0].kz_count;
                for (i = 0; i < newWeightBias->slice_num; i++)
                {
                    values.e32[0] = 0;
                    values.u32[0] = kzgroup;
                    values.u32[1] = zoffset;
                    values.u32[2] = kzoffset;
                    values.u32[3] = kzoffset2;

                    vxMemCopy(&dstOperation->base.parameter.tp_value[i], &values, sizeof(vx_tp_value_cmd_s));
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
        else if (1 == srcTpOp->base.parameter.tp_value->e32[0])
        {
             dstOperation->base.parameter.tp_value = (vx_tp_value_cmd_s*)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
             if (dstOperation->base.parameter.tp_value != VX_NULL)
             {
                values.e32[0] = 1;
                values.u32[1] = newWeightBias->weights_sizes[3];
                vxMemCopy(dstOperation->base.parameter.tp_value, &values, sizeof(vx_tp_value_cmd_s));
             }
        }
        else
        {
            vxmASSERT(0);
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
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

    gcmHEADER_ARG("layer=%p, node=%p, operation=%p, splitCount=0x%x", layer, node, operation, splitCount);
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
                if (gpuIndex > 0)
                {
                    layer->operations[layer->base.num_operations-1]->mGpuNext = layer->operations[layer->base.num_operations];
                }
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
            layer->operations[layer->base.num_operations-1]->mGpuNext = VX_NULL;
        }
    }
    else
    {
        vxError("multiGPU can't support this target: %d\n", operation->target);
        vxmASSERT(0);
    }
    gcmFOOTER_ARG("%d", status);
    return status;

OnError:
    /* use the original operation if fail to split */
    layer->operations[layer->base.num_operations] = operation;
    layer->operations[layer->base.num_operations]->gpuId = 0;
    layer->operations[layer->base.num_operations]->mGpuSync = vx_true_e;
    layer->base.num_operations++;
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoGraphParallel_AnalyzeOperationsBefore(vx_graph graph)
{
#define MAX_PARALLEL_OUTPUT_NUMBER 36
    vxnne_execution_layer layer = graph->layer;
    vxnne_operation list[VX_MAX_NODE_COUNT] = {VX_NULL};
    vx_uint32 e, p = 0, i, j, k, count = 0, ecount = 0, wDepth = 0, head, tail = 0, start;
    vx_uint32 ecounts[MAX_PARALLEL_OUTPUT_NUMBER];
    vxnne_operation operation, entrys[MAX_PARALLEL_OUTPUT_NUMBER];

    for (i = 0; i < layer->base.num_operations; i++)
    {
        operation = layer->operations[i];

        if (operation->childOpNum == 0)
        {
            for (j = ecount-1; (vx_int32)j >= 0; j--)
            {
                if (operation->absoluteOperationID > entrys[j]->absoluteOperationID)
                {
                    break;
                }
                else
                {
                    if (j == MAX_PARALLEL_OUTPUT_NUMBER - 1) return VX_FAILURE;
                    entrys[j+1] = entrys[j];
                }
            }
            if (j == MAX_PARALLEL_OUTPUT_NUMBER - 1) return VX_FAILURE;
            entrys[j+1] = operation;
            ecount++;
            operation->opDepth = 1;
            count++;
        }
    }

    vxmASSERT(ecount <= MAX_PARALLEL_OUTPUT_NUMBER);

    for (e = 0; e < ecount; e++)
    {
        if (list[p] == VX_NULL)
        {
            list[p] = entrys[e];
        }

        head = 0;
        tail = 0;

        for (;;)
        {
            if (head > tail) break;

            operation = list[p+head];
            head++;

            for (i = 0; i < operation->parentOpNum; i++)
            {
                vxnne_operation parent = operation->parentOps[i];

                if (parent->opDepth != 0)
                {
                    if (parent->opDepth < operation->opDepth + 1)
                    {
                        vxnne_operation op;
                        vxnne_operation list2[VX_MAX_NODE_COUNT] = {0};
                        vx_uint32 head2 = 0, tail2 = 0;

                        /* update op depth to max and adjust its list position */
                        parent->opDepth = operation->opDepth + 1;
                        wDepth = gcmMAX(wDepth, parent->opDepth);
                        list2[tail2] = parent;
                        for (;;)
                        {
                            vx_uint32 chDepth = 0;
                            if (head2 > tail2) break;
                            op = list2[head2];
                            for (j = 0; j < op->childOpNum; j++)
                            {
                                if (op->childOps[j]->opDepth > chDepth)
                                    chDepth = op->childOps[j]->opDepth;
                            }
                            op->opDepth = chDepth + 1;
                            wDepth = gcmMAX(wDepth, op->opDepth);
                            head2++;
                            for (j = 0; j <= tail; j++)
                            {
                                if (list[p+j] == op)
                                    break;
                            }
                            vxmASSERT(j <= tail);

                            for (k = j; k < tail; k++)
                            {
                                if (list[p+k+1]->opDepth > op->opDepth)
                                    break;
                                list[p+k] = list[p+k+1];
                            }
                            list[p+k] = op;

                            if (j < head && k > head)
                                head--;

                            for (j = 0; j < op->parentOpNum; j++)
                            {
                                vxnne_operation pa = op->parentOps[j];
                                if (!pa->opDepth)
                                    continue;
                                list2[++tail2] = pa;
                            }
                        }
                    }
                    else if (parent->opDepth > operation->opDepth + 1)
                    {
                        vx_uint32 minChildDepth = 0xFFFF;
                        for (j = 0; j < operation->parentOpNum; j++)
                        {
                            if (operation->parentOps[j]->opDepth < minChildDepth)
                                minChildDepth = operation->parentOps[j]->opDepth;
                        }
                        if (minChildDepth == operation->opDepth + 1)
                            continue;

                        for (j = 0; j <= tail; j++)
                        {
                            if (list[p+j] == operation)
                                break;
                        }
                        vxmASSERT(j <= tail);

                        operation->opDepth = parent->opDepth - 1;
                        for (k = j; k < tail; k++)
                        {
                            if (list[p+k+1]->opDepth >= operation->opDepth)
                                break;
                            list[p+k] = list[p+k+1];
                        }
                        list[p+k] = operation;
                    }
                    continue;
                }
                else
                {
                    parent->opDepth = operation->opDepth + 1;
                    wDepth = gcmMAX(wDepth, parent->opDepth);

                    for (j = tail; (vx_int32)j >= 0; j--)
                    {
                        if (parent->opDepth >= list[p+j]->opDepth)
                            break;
                    }

                    /* put same depth ops together */
                    if (j < tail)
                    {
                        for (k = tail; k > j; k--)
                        {
                            list[p+k+1] = list[p+k];
                        }
                        tail++;
                        list[p+k+1] = parent;
                    }
                    else
                    {
                        tail++;
                        list[p+tail] = parent;
                    }
                    count++;
                }
            }
        }

        ecounts[e] = tail + 1;
        p += ecounts[e];
    }

    vxmASSERT(count == layer->base.num_operations);

    p = 0;
    start = count - 1;
    for (e = 0; e < ecount; e++)
    {
        /* now sequence is from bottom to top */
        head = 0;
        for (;;)
        {
            vx_uint32 cDepth = list[p+head]->opDepth;
            vx_uint32 nextOpSeq = start == count - 1 ? count : layer->operations[start+1]->opSequence;

            for (i = head; i < ecounts[e]; i++)
            {
                vx_uint32 tDepth = list[p+i]->opDepth;
                if (tDepth != cDepth)
                    break;
            }

            /* adjust same depth ops sequence */
            if (i - head > 1)
            {
#define MAX_OP_NUMBER_ONE_LAYER 128
                vx_uint32 swCount = 0, shCount = 0, nntpCount = 0, l = 0, r = MAX_OP_NUMBER_ONE_LAYER;
                vxnne_operation tmpNNTPOps[MAX_OP_NUMBER_ONE_LAYER] = {VX_NULL};
                vxnne_operation tmpSHOps[MAX_OP_NUMBER_ONE_LAYER] = {VX_NULL};
                vxnne_operation tmpSWOps[MAX_OP_NUMBER_ONE_LAYER/2] = {VX_NULL};

                for (j = head; j < i; j++)
                {
                    vx_uint32 paDepth = 0xFFFF, chOpDelta = 0xFFFF;
                    operation = list[p+j];

                    for (k = 0; k < operation->parentOpNum; k++)
                    {
                        if (operation->parentOps[k]->opDepth < paDepth)
                            paDepth = operation->parentOps[k]->opDepth;
                    }
                    for (k = 0; k < operation->childOpNum; k++)
                    {
                        vx_uint32 delta = abs((vx_int32)(operation->childOps[k]->opSequence - nextOpSeq));
                        chOpDelta = gcmMIN(delta, chOpDelta);
                    }

                    if (operation->target == VXNNE_OPERATION_TARGET_SH)
                    {
                        for (k = shCount - 1; (vx_int32)k >= 0; k--)
                        {
                            vx_uint32 m, tpaDepth = 0xFFFF, tchOpDelta = 0xFFFF;
                            for (m = 0; m < tmpSHOps[k]->parentOpNum; m++)
                            {
                                if (tmpSHOps[k]->parentOps[m]->opDepth < tpaDepth)
                                    tpaDepth = tmpSHOps[k]->parentOps[m]->opDepth;
                            }
                            for (m = 0; m < tmpSHOps[k]->childOpNum; m++)
                            {
                                vx_uint32 delta = abs((vx_int32)(tmpSHOps[k]->childOps[m]->opSequence - nextOpSeq));
                                tchOpDelta = gcmMIN(delta, tchOpDelta);
                            }
                            if ((operation->parentOpNum != 0 && (!tmpSHOps[k]->parentOpNum || tpaDepth < paDepth)) || chOpDelta > tchOpDelta)
                            {
                                tmpSHOps[k+1] = tmpSHOps[k];
                            }
                            else
                            {
                                break;
                            }
                        }
                        tmpSHOps[k+1] = operation;
                        shCount++;
                        vxmASSERT(shCount <= MAX_OP_NUMBER_ONE_LAYER);
                    }
                    else if (operation->target == VXNNE_OPERATION_TARGET_SW)
                    {
                        for (k = swCount - 1; (vx_int32)k >= 0; k--)
                        {
                            vx_uint32 m, tpaDepth = 0xFFFF, tchOpDelta = 0xFFFF;
                            for (m = 0; m < tmpSWOps[k]->parentOpNum; m++)
                            {
                                if (tmpSWOps[k]->parentOps[m]->opDepth < tpaDepth)
                                    tpaDepth = tmpSWOps[k]->parentOps[m]->opDepth;
                            }
                            for (m = 0; m < tmpSWOps[k]->childOpNum; m++)
                            {
                                vx_uint32 delta = abs((vx_int32)(tmpSWOps[k]->childOps[m]->opSequence - nextOpSeq));
                                tchOpDelta = gcmMIN(delta, tchOpDelta);
                            }
                            if ((operation->parentOpNum != 0 && (!tmpSWOps[k]->parentOpNum || tpaDepth < paDepth)) || chOpDelta > tchOpDelta)
                            {
                                tmpSWOps[k+1] = tmpSWOps[k];
                            }
                            else
                            {
                                break;
                            }
                        }
                        tmpSWOps[k+1] = operation;
                        swCount++;
                        vxmASSERT(swCount <= MAX_OP_NUMBER_ONE_LAYER / 2);
                    }
                    else
                    {
                        if (operation->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            for (k = l - 1; (vx_int32)k >= 0; k--)
                            {
                                vx_uint32 m, tpaDepth = 0xFFFF, tchOpDelta = 0xFFFF;
                                for (m = 0; m < tmpNNTPOps[k]->parentOpNum; m++)
                                {
                                    if (tmpNNTPOps[k]->parentOps[m]->opDepth < tpaDepth)
                                        tpaDepth = tmpNNTPOps[k]->parentOps[m]->opDepth;
                                }
                                for (m = 0; m < tmpNNTPOps[k]->childOpNum; m++)
                                {
                                    vx_uint32 delta = abs((vx_int32)(tmpNNTPOps[k]->childOps[m]->opSequence - nextOpSeq));
                                    tchOpDelta = gcmMIN(delta, tchOpDelta);
                                }
                                if ((operation->parentOpNum != 0 && (!tmpNNTPOps[k]->parentOpNum || tpaDepth < paDepth)) || chOpDelta > tchOpDelta)
                                {
                                    tmpNNTPOps[k+1] = tmpNNTPOps[k];
                                }
                                else
                                {
                                    break;
                                }
                            }
                            tmpNNTPOps[k+1] = operation;
                            l++;
                        }
                        else /* TP */
                        {
                            for (k = r; k < MAX_OP_NUMBER_ONE_LAYER; k++)
                            {
                                vx_uint32 m, tpaDepth = 0xFFFF, tchOpDelta = 0xFFFF;
                                for (m = 0; m < tmpNNTPOps[k]->parentOpNum; m++)
                                {
                                    if (tmpNNTPOps[k]->parentOps[m]->opDepth < tpaDepth)
                                        tpaDepth = tmpNNTPOps[k]->parentOps[m]->opDepth;
                                }
                                for (m = 0; m < tmpNNTPOps[k]->childOpNum; m++)
                                {
                                    vx_uint32 delta = abs((vx_int32)(tmpNNTPOps[k]->childOps[m]->opSequence - nextOpSeq));
                                    tchOpDelta = gcmMIN(delta, tchOpDelta);
                                }
                                if ((operation->parentOpNum != 0 && (!tmpNNTPOps[k]->parentOpNum || tpaDepth < paDepth)) || chOpDelta > tchOpDelta)
                                {
                                    tmpNNTPOps[k-1] = tmpNNTPOps[k];
                                }
                                else
                                {
                                    break;
                                }
                            }
                            tmpNNTPOps[k-1] = operation;
                            r--;
                        }
                        nntpCount++;
                        vxmASSERT(nntpCount <= MAX_OP_NUMBER_ONE_LAYER);
                    }
                }

                /* put SW op with reverse sequence */
                for (j = 0; j < swCount; j++)
                {
                    if (p == 0)
                    {
                        tmpSWOps[j]->opSequence = start;
                        layer->operations[start--] = tmpSWOps[j];
                    }
                    else
                    {
                        for (k = start; k < count - 1; k++)
                        {
                            if (tmpSWOps[j]->opDepth > layer->operations[k+1]->opDepth)
                            {
                                break;
                            }
                            else
                            {
                                layer->operations[k]->opSequence = layer->operations[k+1]->opSequence;
                                layer->operations[k] = layer->operations[k+1];
                            }
                        }
                        tmpSWOps[j]->opSequence = k;
                        layer->operations[k] = tmpSWOps[j];
                        start--;
                    }
                }

                /* put SH op with reverse sequence */
                for (j = shCount - 1; (vx_int32)j >= 0; j--)
                {
                    if (p == 0)
                    {
                        tmpSHOps[j]->opSequence = start;
                        layer->operations[start--] = tmpSHOps[j];
                    }
                    else
                    {
                        for (k = start; k < count - 1; k++)
                        {
                            if (tmpSHOps[j]->opDepth > layer->operations[k+1]->opDepth)
                            {
                                break;
                            }
                            else
                            {
                                layer->operations[k]->opSequence = layer->operations[k+1]->opSequence;
                                layer->operations[k] = layer->operations[k+1];
                            }
                        }
                        tmpSHOps[j]->opSequence = k;
                        layer->operations[k] = tmpSHOps[j];
                        start--;
                    }
                }

                /* put NN/TP op with reverse sequence */
                l--;
                if (p == 0)
                {
                    start -= nntpCount - 1;

                    j = 0;
                    k = MAX_OP_NUMBER_ONE_LAYER - 1;
                    for (; ((vx_int32)l >= 0 && j <= l) || k >= r; )
                    {
                        /* TP */
                        if (k >= r)
                        {
                            tmpNNTPOps[k]->opSequence = start;
                            layer->operations[start++] = tmpNNTPOps[k--];
                        }
                        /* NN */
                        if ((vx_int32)l >= 0 && j <= l)
                        {
                            tmpNNTPOps[j]->opSequence = start;
                            layer->operations[start++] = tmpNNTPOps[j++];
                        }
                    }

                    start -= nntpCount + 1;
                }
                else
                {
                    for (; nntpCount > 0;)
                    {
                        if ((vx_int32)l >= 0)
                        {
                            for (k = start; k < count - 1; k++)
                            {
                                if ((tmpNNTPOps[l]->opDepth > layer->operations[k+1]->opDepth) ||
                                    (tmpNNTPOps[l]->target != VXNNE_OPERATION_TARGET_SH &&
                                     tmpNNTPOps[l]->opDepth == layer->operations[k+1]->opDepth &&
                                     layer->operations[k+1]->target == VXNNE_OPERATION_TARGET_SH))
                                {
                                    break;
                                }
                                else
                                {
                                    layer->operations[k]->opSequence = layer->operations[k+1]->opSequence;
                                    layer->operations[k] = layer->operations[k+1];
                                }
                            }
                            tmpNNTPOps[l]->opSequence = k;
                            layer->operations[k] = tmpNNTPOps[l];
                            start--;
                            nntpCount--;
                        }
                        if (r < MAX_OP_NUMBER_ONE_LAYER)
                        {
                            for (k = start; k < count - 1; k++)
                            {
                                if ((tmpNNTPOps[r]->opDepth > layer->operations[k+1]->opDepth) ||
                                    (tmpNNTPOps[r]->target != VXNNE_OPERATION_TARGET_SH &&
                                     tmpNNTPOps[r]->opDepth == layer->operations[k+1]->opDepth &&
                                     layer->operations[k+1]->target == VXNNE_OPERATION_TARGET_SH))
                                {
                                    break;
                                }
                                else
                                {
                                    layer->operations[k]->opSequence = layer->operations[k+1]->opSequence;
                                    layer->operations[k] = layer->operations[k+1];
                                }
                            }
                            tmpNNTPOps[r]->opSequence = k;
                            layer->operations[k] = tmpNNTPOps[r];
                            start--;
                            nntpCount--;
                        }
                    }
                }
            }
            else
            {
                if (p == 0)
                {
                    list[p+head]->opSequence = start;
                    layer->operations[start--] = list[p+head];
                }
                else
                {
                    for (j = start; j < count - 1; j++)
                    {
                        if ((list[p+head]->opDepth > layer->operations[j+1]->opDepth ||
                            (list[p+head]->target != VXNNE_OPERATION_TARGET_SH &&
                             list[p+head]->opDepth == layer->operations[j+1]->opDepth &&
                             layer->operations[j+1]->target == VXNNE_OPERATION_TARGET_SH)))
                        {
                            break;
                        }
                        else
                        {
                            layer->operations[j]->opSequence = layer->operations[j+1]->opSequence;
                            layer->operations[j] = layer->operations[j+1];
                        }
                    }
                    list[p+head]->opSequence = j;
                    layer->operations[j] = list[p+head];
                    start--;
                }
            }

            head = i;

            if (head >= ecounts[e])
                break;
        }

        p += ecounts[e];
    }

    return VX_SUCCESS;
}

#define APPEND_NEW_OP_EVENT(oldOP, newOP) \
do \
{ \
    vx_uint32 jj; \
    vx_uint32 eventIDT = newOP->engineSync.eventId[newOP->engineSync.eventCnt-1]; \
    if (newOP->target == VXNNE_OPERATION_TARGET_SH || newOP->target == VXNNE_OPERATION_TARGET_SW) break; \
    for (jj = oldOP->engineSync.waitCnt - 1; (vx_int32)jj >= 0; jj--) \
    { \
        if ((eventIDT < oldOP->engineSync.waitId[jj] && abs((vx_int32)(eventIDT - oldOP->engineSync.waitId[jj])) < 20) || \
            (eventIDT > oldOP->engineSync.waitId[jj] && abs((vx_int32)(eventIDT - oldOP->engineSync.waitId[jj])) > 20)) \
        { \
            oldOP->engineSync.waitId[jj+1] = oldOP->engineSync.waitId[jj]; \
            oldOP->engineSync.waitTarget[jj+1] = oldOP->engineSync.waitTarget[jj]; \
        } \
        else \
            break; \
    } \
    if ((vx_int32)jj < 0 || oldOP->engineSync.waitId[jj] != eventIDT) \
    { \
        oldOP->engineSync.waitId[jj+1] = eventIDT; \
        oldOP->engineSync.waitTarget[jj+1] = newOP->target; \
        oldOP->engineSync.waitCnt++; \
    } \
    else \
    { \
        for (jj++; jj < oldOP->engineSync.waitCnt; jj++) \
        { \
            oldOP->engineSync.waitId[jj] = oldOP->engineSync.waitId[jj+1]; \
            oldOP->engineSync.waitTarget[jj] = oldOP->engineSync.waitTarget[jj+1]; \
        } \
    } \
} while(0)

VX_INTERNAL_API vx_status vxoGraphParallel_AnalyzeOperationsAfter(vx_graph graph)
{
    vx_uint32 i = 0, j = 0, k, start = 0;
    vxnne_execution_layer layer = graph->layer;
    vx_uint32 eventId = 1;
    vx_uint32 flushOpFlag[VX_MAX_NODE_COUNT] = {0}, flushNNPos = 0xFFFF, flushTPPos = 0xFFFF;
    vxnne_operation prevNNOp = VX_NULL, flushedNNOp = VX_NULL, flushedTPOp = VX_NULL;
    vx_bool nnTPFlushed = vx_false_e, tpTPFlushed = vx_false_e, tpNNFlushed = vx_false_e;

    vxmASSERT(layer->base.num_operations == layer->opIndicesNum);

    for (i = 0; i < layer->opIndicesNum; i++)
    {
        vxnne_operation operation = layer->operations[layer->opIndices[i].operationID];

        if (layer->opIndices[i].commandBuffer.commandCount > 0)
        {
            operation->engineSync.eventCnt = layer->opIndices[i].commandBuffer.commandCount;

            for (j = 0; j < layer->opIndices[i].commandBuffer.commandCount - 1; j++)
            {
                operation->engineSync.eventId[j] = 31;
            }
            operation->engineSync.eventId[operation->engineSync.eventCnt - 1] = eventId;

            eventId = eventId >= 30 ? 1 : eventId + 1;
        }
    }

    for (i = layer->opIndicesNum - 1; (vx_int32)i >= 0; i--)
    {
        vxnne_operation operation = layer->operations[layer->opIndices[i].operationID];

        if (operation->target == VXNNE_OPERATION_TARGET_NN)
        {
            for (j = 0; j < operation->parentOpNum; j++)
            {
                vxnne_operation operationPA = operation->parentOps[j];
                flushOpFlag[operationPA->absoluteOperationID] = i+1;
            }
        }
        else if (flushOpFlag[operation->absoluteOperationID] && operation->target != VXNNE_OPERATION_TARGET_TP && operation->target != VXNNE_OPERATION_TARGET_NN)
        {
            for (j = 0; j < operation->parentOpNum; j++)
            {
                vxnne_operation operationPA = operation->parentOps[j];
                flushOpFlag[operationPA->absoluteOperationID] = flushOpFlag[operation->absoluteOperationID];
            }
        }

        if (operation->target == VXNNE_OPERATION_TARGET_NN)
        {
            vxnneModifyNNLastNoflushBit(graph->base.context, &layer->opIndices[i].commandBuffer, operation, i != layer->opIndicesNum - 1 ? 1 : 0);
        }
        else if (operation->target == VXNNE_OPERATION_TARGET_TP)
        {
            vxnneModifyTPLastNoflushBit(graph->base.context, &layer->opIndices[i].commandBuffer, operation, i != layer->opIndicesNum - 1 ? 1 : 0);
        }
    }

    for (;;)
    {
        vx_uint32 cDepth = layer->operations[layer->opIndices[start].operationID]->opDepth;
        vx_bool hasSH = vx_false_e;

        for (i = start; i < layer->opIndicesNum; i++)
        {
            vxnne_operation operation = layer->operations[layer->opIndices[i].operationID];

            if (operation->opDepth != cDepth)
                break;

            if (operation->target != VXNNE_OPERATION_TARGET_SH)
            {
                vx_uint32 lastParentNN = 0, lastParentTP = 0;

                for (j = 0; j < operation->parentOpNum; j++)
                {
                    vxnne_operation operationPa = operation->parentOps[j];
                    if (operationPa->engineSync.eventCnt > 0)
                    {
                        vx_uint32 parentID = operationPa->engineSync.eventId[operationPa->engineSync.eventCnt-1];
                        APPEND_NEW_OP_EVENT(operation, operationPa);
                        if (operationPa->target == VXNNE_OPERATION_TARGET_NN && parentID > lastParentNN)
                            lastParentNN = parentID;
                        else if (operationPa->target == VXNNE_OPERATION_TARGET_TP && parentID > lastParentTP)
                            lastParentTP = parentID;
                    }
                }

                if (operation->target == VXNNE_OPERATION_TARGET_NN)
                {
                    /* NN op must wait for previous NN op */
                    if (prevNNOp != VX_NULL && prevNNOp->engineSync.eventId[prevNNOp->engineSync.eventCnt-1] > lastParentNN)
                    {
                        APPEND_NEW_OP_EVENT(operation, prevNNOp);
                    }
                    prevNNOp = operation;

                    /* NN op must wait for previous layer flushed TP op */
                    if (!nnTPFlushed && flushedTPOp != VX_NULL)
                    {
                        if (flushedTPOp->engineSync.eventId[flushedTPOp->engineSync.eventCnt-1] > lastParentTP)
                            APPEND_NEW_OP_EVENT(operation, flushedTPOp);
                        nnTPFlushed = vx_true_e;
                    }

                    if (flushOpFlag[operation->absoluteOperationID])
                    {
                        flushNNPos = i;
                    }
                }
                else if (operation->target == VXNNE_OPERATION_TARGET_TP)
                {
                    /* TP op must wait for previous layer flushed NN op */
                    if (!tpNNFlushed && flushedNNOp != VX_NULL)
                    {
                        if (flushedNNOp->engineSync.eventId[flushedNNOp->engineSync.eventCnt-1] > lastParentTP)
                            APPEND_NEW_OP_EVENT(operation, flushedNNOp);
                        tpNNFlushed = vx_true_e;
                    }
                    /* TP op must wait for previous layer flushed TP op */
                    if (!tpTPFlushed && flushedTPOp != VX_NULL)
                    {
                        if (flushedTPOp->engineSync.eventId[flushedTPOp->engineSync.eventCnt-1] > lastParentTP)
                            APPEND_NEW_OP_EVENT(operation, flushedTPOp);
                        tpTPFlushed = vx_true_e;
                    }

                    if (flushOpFlag[operation->absoluteOperationID])
                    {
                        flushTPPos = i;
                    }
                }
            }
            else /* SH operation */
            {
                if (hasSH || i == 0)
                {
                    operation->engineSync.waitCnt = 0;
                }
                else
                {
                    vx_uint32 pDepth = layer->operations[layer->opIndices[start-1].operationID]->opDepth;

                    for (j = start - 1; (vx_int32)j >= 0; j--)
                    {
                        if (layer->operations[layer->opIndices[j].operationID]->opDepth != pDepth)
                            break;
                        if (layer->operations[layer->opIndices[j].operationID]->target == VXNNE_OPERATION_TARGET_SH)
                            break;
                    }

                    /* wait for all operations in previous layer if previous layer has no SH ops */
                    for (k = j + 1; k < start; k++)
                    {
                        vxnne_operation prevOp = layer->operations[layer->opIndices[k].operationID];
                        if (prevOp->target != VXNNE_OPERATION_TARGET_SW)
                        {
                            APPEND_NEW_OP_EVENT(operation, prevOp);
                        }
                    }

                    /* wait for all previous non-sh ops in same layer */
                    for (j = start; j < i; j++)
                    {
                        vxnne_operation prevOp = layer->operations[layer->opIndices[j].operationID];
                        if (prevOp->target != VXNNE_OPERATION_TARGET_SW)
                        {
                            APPEND_NEW_OP_EVENT(operation, prevOp);
                        }
                    }

                    nnTPFlushed = tpTPFlushed = tpNNFlushed = vx_true_e;
                    hasSH = vx_true_e;
                }

                if (i == layer->opIndicesNum - 1)
                {
                    operation->engineSync.eventId[0] = 0xFF;
                    vxInfo("SH flush %2d\n", operation->absoluteOperationID);
                }
                else
                {
                    for (j = 0; j < operation->childOpNum; j++)
                    {
                        vxnne_operation operationCh = operation->childOps[j];
                        if (operationCh->target == VXNNE_OPERATION_TARGET_NN)
                        {
                            operation->engineSync.eventId[0] = 0xFF;
                            vxInfo("SH flush %2d\n", operation->absoluteOperationID);
                            break;
                        }
                    }
                }
            }
        }

        if (i >= layer->opIndicesNum)
            break;

        if (flushTPPos != 0xFFFF)
        {
            vx_bool needFlush = vx_true_e;
            vx_uint32 lastWaitNN = 0;
            vxnne_operation operation = layer->operations[layer->opIndices[flushTPPos].operationID];
            vxnne_operation prevOp = VX_NULL;

            if (flushOpFlag[operation->absoluteOperationID] != 0xFFFF)
            {
                /* check if this TP op can no-flush if next layer last TP flush op is before NN op */
                vx_uint32 nDepth = layer->operations[layer->opIndices[i].operationID]->opDepth;
                vxnne_operation firstNNOp = layer->operations[layer->opIndices[flushOpFlag[operation->absoluteOperationID]-1].operationID];
                vxnne_operation lastTPOp = VX_NULL;
                for (j = i; j < layer->opIndicesNum; j++)
                {
                    vxnne_operation op = layer->operations[layer->opIndices[j].operationID];
                    if (op->opDepth != nDepth)
                        break;
                    if (op->target == VXNNE_OPERATION_TARGET_TP && flushOpFlag[op->absoluteOperationID])
                    {
                        lastTPOp = op;
                    }
                    else if (op->target == VXNNE_OPERATION_TARGET_NN &&
                             op->engineSync.eventId[op->engineSync.eventCnt-1] >= firstNNOp->engineSync.eventId[firstNNOp->engineSync.eventCnt-1])
                    {
                        break;
                    }
                }
                if (lastTPOp != VX_NULL &&
                    lastTPOp->engineSync.eventId[lastTPOp->engineSync.eventCnt-1] < firstNNOp->engineSync.eventId[firstNNOp->engineSync.eventCnt-1])
                {
                    /* must flush next time */
                    flushOpFlag[lastTPOp->absoluteOperationID] = 0xFFFF;
                    needFlush = vx_false_e;
                }
            }

            if (needFlush)
            {
                /* this flushed TP op must wait for previous NN op */
                for (j = flushTPPos - 1; (vx_int32)j >= 0; j--)
                {
                    prevOp = layer->operations[layer->opIndices[j].operationID];
                    if (prevOp->target == VXNNE_OPERATION_TARGET_NN)
                    {
                        break;
                    }
                    else if (prevOp->target == VXNNE_OPERATION_TARGET_TP)
                    {
                        for (k = 0; k < prevOp->engineSync.waitCnt; k++)
                        {
                            if (prevOp->engineSync.waitTarget[k] == VXNNE_OPERATION_TARGET_NN && prevOp->engineSync.waitId[k] > lastWaitNN)
                                lastWaitNN = prevOp->engineSync.waitId[k];
                        }
                    }
                    else if (prevOp->target == VXNNE_OPERATION_TARGET_SH)
                        break;
                }
                if (prevOp != VX_NULL && prevOp->target == VXNNE_OPERATION_TARGET_NN &&
                    prevOp->engineSync.eventId[prevOp->engineSync.eventCnt-1] > lastWaitNN)
                {
                    APPEND_NEW_OP_EVENT(operation, prevOp);
                }

                /* first NN op in same layer must wait for this flush TP op */
                for (j = flushTPPos + 1; j < layer->opIndicesNum; j++)
                {
                    vxnne_operation nextOp = layer->operations[layer->opIndices[j].operationID];
                    if (nextOp->opDepth != cDepth)
                        break;
                    if (nextOp->target == VXNNE_OPERATION_TARGET_NN)
                    {
                        APPEND_NEW_OP_EVENT(nextOp, operation);
                        break;
                    }
                    else if (nextOp->target == VXNNE_OPERATION_TARGET_SH)
                        break;
                }

                vxnneModifyTPLastNoflushBit(graph->base.context, &layer->opIndices[flushTPPos].commandBuffer, operation, 0);
                vxInfo("TP flush %2d\n", operation->absoluteOperationID);
                nnTPFlushed = tpTPFlushed = vx_false_e;
                flushedTPOp = operation;
            }
            else
            {
                flushedTPOp = VX_NULL;
            }
            flushTPPos = 0xFFFF;
        }

        if (flushNNPos != 0xFFFF)
        {
            vx_uint32 lastWaitNN = 0;
            vxnne_operation operation = layer->operations[layer->opIndices[flushNNPos].operationID];
            vxnne_operation prevOp = VX_NULL;

            /* this flushed NN op must wait for previous TP op */
            for (j = flushNNPos - 1; (vx_int32)j >= 0; j--)
            {
                prevOp = layer->operations[layer->opIndices[j].operationID];
                if (prevOp->target == VXNNE_OPERATION_TARGET_TP)
                {
                    break;
                }
                else if (prevOp->target == VXNNE_OPERATION_TARGET_NN)
                {
                    for (k = 0; k < prevOp->engineSync.waitCnt; k++)
                    {
                        if (prevOp->engineSync.waitTarget[k] == VXNNE_OPERATION_TARGET_NN && prevOp->engineSync.waitId[k] > lastWaitNN)
                            lastWaitNN = prevOp->engineSync.waitId[k];
                    }
                }
                else if (prevOp->target == VXNNE_OPERATION_TARGET_SH)
                    break;
            }
            if (prevOp != VX_NULL && prevOp->target == VXNNE_OPERATION_TARGET_TP &&
                prevOp->engineSync.eventId[prevOp->engineSync.eventCnt-1] > lastWaitNN)
            {
                APPEND_NEW_OP_EVENT(operation, prevOp);
            }

            /* first TP op in same layer must wait for this flush NN op */
            for (j = flushNNPos + 1; j < layer->opIndicesNum; j++)
            {
                vxnne_operation nextOp = layer->operations[layer->opIndices[j].operationID];
                if (nextOp->opDepth != cDepth)
                    break;
                if (nextOp->target == VXNNE_OPERATION_TARGET_TP)
                {
                    APPEND_NEW_OP_EVENT(nextOp, operation);
                    break;
                }
                else if (nextOp->target == VXNNE_OPERATION_TARGET_SH)
                    break;
            }

            vxnneModifyNNLastNoflushBit(graph->base.context, &layer->opIndices[flushNNPos].commandBuffer, operation, 0);
            vxInfo("NN flush %2d\n", operation->absoluteOperationID);
            tpNNFlushed = vx_false_e;
            flushedNNOp = operation;
        }
        else
        {
            flushedNNOp = VX_NULL;
        }

        flushNNPos = 0xFFFF;

        start = i;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoGraph_GenerateOperationTable(vx_graph graph)
{
    vxnne_execution_layer layer = VX_NULL;
    vx_uint32 i, j, operationCount = 0;
    gctUINT32 gpuCount = 1;
    vx_uint32 enableMultiVIPCombined = graph->base.context->options.enableMultiVIPCombined;
    vx_status status = VX_SUCCESS;
    gceSTATUS gStatus;
    gcmHEADER_ARG("graph=%p", graph);

    if ((graph->nodeCount == 0) || !graph->nodeTable[graph->allNodeIndexTable[0]]->layer)
    {
        gcmFOOTER_NO();
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
                layer->operations[layer->base.num_operations]->absoluteOperationID = layer->base.num_operations;
                layer->operations[layer->base.num_operations]->mGpuNext = VX_NULL;
                layer->base.num_operations++;
            }
        }
    }

    vxmASSERT(layer->base.num_operations <= operationCount);
    graph->layer = layer;

    gcmFOOTER_NO();
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

VX_INTERNAL_API vx_bool vxoGraph_IsInTailNodesTable(vx_graph graph, vx_uint32 nodeIndex)
{
    vx_uint32 i;
    vx_bool result = vx_false_e;

    gcmHEADER_ARG("graph=%p, nodeIndex=0x%x", graph, nodeIndex);

    for (i = 0; i < graph->tailNodeCount; i++)
    {
        result = (nodeIndex == graph->tailNodeIndexTable[i]) ? vx_true_e : vx_false_e;

        if (result == vx_true_e)
            break;
    }

    gcmFOOTER_ARG("0x%x", result);
    return result;
}

VX_INTERNAL_API vx_status vxoGraph_PrepareParamMemory(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, i;

    gcmHEADER_ARG("graph=%p", graph);

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
                        node, node->kernel->name, i);
                    status = VX_ERROR_NO_MEMORY;
                }
            }
        }
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
VX_INTERNAL_API vx_status vxoGraph_VerifyVirtualBuffer(vx_graph graph)
{
    vx_uint32 i, j, count = 0;
    vxnne_mem_request rlist = VX_NULL;
    vx_context context = graph->base.context;
    vx_status status = VX_SUCCESS;
    vx_bool enablePool = context->options.enableMemPool ? vx_true_e : vx_false_e;

    gcmHEADER_ARG("graph=%p", graph);

    if (enablePool)
    {
        if (graph->memoryPool != VX_NULL)
        {
            vxoMemoryPool_Deinitialize(graph);
        }

        if (graph->virtTensorNum != 0)
        {
            if (!vxoMemoryPool_Initialize(graph, context->options.memPoolSize))
            {
                vxError("Can't allocate memory for virtual memory pool");
                vxmONERROR(VX_FAILURE);
            }
        }

        rlist = (vxnne_mem_request)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_mem_request_s) * graph->layer->opIndicesNum);
        vxmONERROR_NULLPTR(rlist);
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
                        vxmONERROR(vxoTensor_AllocateMemory(input));
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
                        vxmONERROR(vxoTensor_AllocateMemory(output));
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
        status = vxoMemoryPool_RequestList(graph, rlist, count, 0, count, VX_NULL);
        gcmASSERT(status == VX_SUCCESS);
    }

OnError:
    if (rlist != VX_NULL)
    {
        vxFree(rlist);
        rlist = VX_NULL;
    }
    gcmFOOTER_ARG("%d", status);
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

    gcmHEADER_ARG("graph=%p", graph);

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
    gcmFOOTER_NO();
}
VX_INTERNAL_API void vxoGraph_PolluteIfInput(vx_graph graph, vx_reference targetRef)
{
    vx_uint32 nodeIndex;
    vx_context context = targetRef->context;
    gcmHEADER_ARG("graph=%p, targetRef=%p", graph, targetRef);
    vxmASSERT(graph);
    vxmASSERT(targetRef);

    vxAcquireMutex(context->base.lock);

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
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_NO();
                return;
            }
        }
    }
    vxReleaseMutex(context->base.lock);
}

VX_API_ENTRY vx_graph VX_API_CALL vxCreateGraph(vx_context context)
{
    vx_graph graph;

    gcmHEADER_ARG("context=%p", context);
    gcmDUMP_API("$VX vxCreateGraph: context=%p", context);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    graph = (vx_graph)vxoReference_Create(context, VX_TYPE_GRAPH, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)graph) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get graph reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&context->base, vxoReference_GetStatus((vx_reference)graph), "%s[%d]: Get graph reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%p", graph);
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

    gcmFOOTER_ARG("%p", graph);
    return graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetGraphAttribute: graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryGraph: graph=%p, attribute=0x%x, ptr=%p, size=0x%lx", graph, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
        return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

    gcmHEADER_ARG("graph=%p, path=%s, only_top=0x%x, top=0x%x, buffer=%s", graph, path, only_top, top, buffer);
    gcmDUMP_API("$VX vx_vivPeferGraph: graph=%p, path=%s, only_top=0x%x, top=0x%x, buffer=%s", graph, path, only_top, top, buffer);

    if(!graph)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
        return VX_ERROR_INVALID_GRAPH;
    }
    if (only_top && graph->isChildGraph)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
        return VX_ERROR_INVALID_GRAPH;
    }
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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseGraph(vx_graph *graph)
{
    vx_status status;
    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxReleaseGraph: graph=%p", graph);

    if (graph == NULL || *graph == NULL)
    {
        return VX_ERROR_INVALID_GRAPH;
    }

    {
        vx_char * path = (vx_char *)((*graph)->base.context->options.graphPerfLogFile);
        if (path)
            vx_vivPeferGraph(*graph, path, vx_true_e, vx_true_e, NULL);
    }

    vxoBinaryGraph_ReleaseCache(*graph);

    status = vxoReference_Release((vx_reference_ptr)graph, VX_TYPE_GRAPH, VX_REF_EXTERNAL);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_bool vxoGraph_IsParentGraphScope(vx_reference scope, vx_graph graph)
{
    vx_bool flag = vx_false_e;
    vx_graph nextGpraph = graph;

    gcmHEADER_ARG("scope=%p, graph=%p", scope, graph);
    vxmASSERT(graph);
    vxmASSERT(scope);

    if (vxoReference_GetType(scope) != VX_TYPE_GRAPH)
    {
        gcmFOOTER_ARG("%d", flag);
        return flag;
    }

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

    gcmFOOTER_ARG("%d", flag);
    return flag;
}

VX_PRIVATE_API vx_status vxoGraph_InitMetaFormatData(vx_graph graph, vx_node node, vx_uint32 paramIndex, vx_reference_s **vRef, vx_meta_format* metaFormat, vx_status* status)
{
    gcmHEADER_ARG("graph=%p, node=%p, paramIndex=0x%x, vRef=%p, metaFormat=%p, status=%p", graph, node, paramIndex, vRef, metaFormat, status);

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
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
    }
    (*metaFormat)->type = node->kernel->signature.dataTypeTable[paramIndex];
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* calculate image valid rectangle through callback */
VX_PRIVATE_API vx_bool vxoGraph_SetImagValidRect(vx_node node, vx_reference paramRef, vx_uint32 paramIndex, vx_meta_format metaFormat)
{
    vx_image img = (vx_image)paramRef;
    gcmHEADER_ARG("node=%p", node);

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
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
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
            gcmFOOTER_ARG("%d", vx_false_e);
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
        gcmFOOTER_ARG("0x%x", res);
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
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
    }

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_PRIVATE_API vx_status vxoGraph_PostMetaFormatData(vx_node node, vx_reference paramRef, vx_uint32 paramIndex, vx_reference_s **vRef, vx_meta_format metaFormat)
{
    gcmHEADER_ARG("node=%p", node);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_FORMAT);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                return VX_ERROR_INVALID_VALUE;
            }

            if (pyramid->scale != metaFormat->u.pyramidInfo.scale)
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter has invalid scale, %f (expected: %f)",
                    node, node->kernel->name, paramIndex,
                    pyramid->scale, metaFormat->u.pyramidInfo.scale);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                return VX_ERROR_INVALID_VALUE;
            }

            if (pyramid->format != VX_DF_IMAGE_VIRT && pyramid->format != metaFormat->u.pyramidInfo.format)
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter's format, %d, is invalid (expected: %d)",
                    node, node->kernel->name, paramIndex,
                    pyramid->format, metaFormat->u.pyramidInfo.format);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_FORMAT);
                return VX_ERROR_INVALID_FORMAT;
            }

            if ((pyramid->width != 0 && pyramid->width != metaFormat->u.pyramidInfo.width)
                || (pyramid->height != 0 && pyramid->height != metaFormat->u.pyramidInfo.height))
            {
                vxError("Node %p(\"%s\"): No.%d pyramid parameter's width and height, %dx%d"
                    ", is invalid (expected: %dx%d)",
                    node, node->kernel->name, paramIndex, pyramid->width, pyramid->height,
                    metaFormat->u.pyramidInfo.width, metaFormat->u.pyramidInfo.height);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_TYPE);
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
                    gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                    gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_TYPE);
                return VX_ERROR_INVALID_TYPE;
            }

            if (matrix->columns != metaFormat->u.matrixInfo.cols || matrix->rows != metaFormat->u.matrixInfo.rows)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid matrix dimention %ux%u\n",
                    node->kernel->name, paramIndex, matrix->rows, matrix->columns);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                return VX_ERROR_INVALID_VALUE;
            }
        }
        break;

    case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)paramRef;
            if (remap->srcWidth != metaFormat->u.remapInfo.src_width || remap->srcHeight != metaFormat->u.remapInfo.src_height)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid source dimention %ux%u\n",
                    node->kernel->name, paramIndex, remap->srcWidth, remap->srcHeight);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
                return VX_ERROR_INVALID_DIMENSION;
            }

            if (remap->destWidth != metaFormat->u.remapInfo.dst_width || remap->destHeight != metaFormat->u.remapInfo.dst_height)
            {
                vxAddLogEntry(&node->base, VX_ERROR_INVALID_DIMENSION,
                    "Node: %s: parameter[%u] has an invalid dest dimention %ux%u\n",
                    node->kernel->name, paramIndex, remap->destWidth, remap->destHeight);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_TYPE);
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
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_DIMENSION);
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
                        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
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
                        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
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
            "The format of meta is invalid objects %d for node: %s\n", metaFormat->type, node->kernel->name);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_FORMAT);
        return VX_ERROR_INVALID_FORMAT;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}


VX_INTERNAL_API vx_status vxoGraph_UserKernelPreprocess(vx_graph graph, vx_bool first)
{
    vx_uint32 nodeIndex;
    gcmHEADER_ARG("graph=%p", graph);
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
                        gcmFOOTER_ARG("%d", status);
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
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodeParameters(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++)
    {
        vx_status status = VX_SUCCESS;
        vx_meta_format metaFormat = VX_NULL;
        vx_node node = graph->nodeTable[nodeIndex];

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

        if (status != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%d", status);
            return status;
        }
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


            if (status != VX_SUCCESS)
            {
                gcmFOOTER_ARG("%d", status);
                return status;
            }
            /* Validate all output/bidirectional parameters */
            for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
            {
                vx_status validationStatus;
                vx_reference paramRef = node->paramTable[paramIndex];
                vx_reference vRef = node->paramTable[paramIndex];

                if (node->kernel->enumeration == VX_KERNEL_IMPORT_FROM_FILE)
                {
                    break;
                }

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
        if (status != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%d", status);
            return status;
        }
    } /* for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++) */

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodeWriteDependencies(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, paramIndex;
    gcmHEADER_ARG("graph=%p", graph);
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

        if (status != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%d", status);
            return status;
        }
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_AllocateAllMemoryObjects(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 nodeIndex, paramIndex;
    gcmHEADER_ARG("graph=%p", graph);
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

        if (status != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%d", status);
            return status;
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

    gcmHEADER_ARG("graph=%p", graph);
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
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_DetectAllHeadNodes(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;
    gcmHEADER_ARG("graph=%p", graph);
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
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
        return VX_ERROR_INVALID_GRAPH;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_DetectAllTailNodes(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;

    gcmHEADER_ARG("graph=%p", graph);
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
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
        return VX_ERROR_INVALID_GRAPH;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool vxoGraph_OptCheckDependency(vx_graph graph, vx_node node, vx_reference param, vx_enum type)
{
    vx_bool in_output = vx_true_e;

    vx_uint32 i = 0;

    gcmHEADER_ARG("graph=%p, node=%p, param=%p, type=0x%x", graph, node, param, type);

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

    gcmFOOTER_ARG("0x%x", in_output);
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
    gcmHEADER_ARG("graph=%p", graph);

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

    gcmFOOTER_ARG("0x%x", count);
    return count;
}

VX_PRIVATE_API vx_status vxAddHeadTensorToGraph(vx_graph graph, vx_parameter param)
{
    gcmHEADER_ARG("graph=%p, param=%p", graph, param);

    if (!vxoReference_IsValidAndNoncontext(&graph->base))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoGraph_AdapterFindParams(vx_graph graph, vx_internal_convert * internal_convert_lists, vx_uint32_ptr internal_convert_count)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;

    gcmHEADER_ARG("graph=%p", graph);

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
                if (vxoGraph_OptCheckDependency(graph, node, (vx_reference)tensor, node->kernel->signature.directionTable[j]))
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
    gcmFOOTER_ARG("%d", status);
    return status;
}

extern vx_status vxoNode_ConvertDims(vx_int32_ptr dims, vx_uint32_ptr org_dims, vx_uint32 count, vx_bool dimsto4);

VX_INTERNAL_API vx_status vxoGraph_Adapter(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_bool opt = vx_false_e;
    vx_uint32_ptr count = &graph->headTensorCount;/*&graph->paramCount;*/
    vx_uint32 i = 0;
    vx_uint32 internal_convert_count = 0;
    vx_internal_convert * internal_convert_lists = (vx_internal_convert *)vxAllocateAndZeroMemory(sizeof(vx_internal_convert) * VX_MAX_PARAMETERS * graph->nodeCount);

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        status = VX_SUCCESS;
        goto exit;
    }

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

            if((node->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_LAYER ||
                  node->kernel->enumeration == VX_KERNEL_FULLY_CONNECTED_LAYER )  &&
                TENSOR_RANK((vx_tensor)node->paramTable[1]) != VX_TENSOR_RANK_WHCN &&
                TENSOR_DATA_LIFETIME((vx_tensor)node->paramTable[1]) == VX_TENSOR_LIFE_TIME_STATIC &&
                index == 1)
                continue;

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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoGraph_DetectCycle(vx_graph graph)
{
    vx_uint32                   nodeIndex;
    vx_graph_traverse_info_s    traverseInfo;

    gcmHEADER_ARG("graph=%p", graph);
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
            gcmFOOTER_ARG("%d", status);
            return status;
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_DetectUnvisitedNodes(vx_graph graph)
{
    vx_uint32 nodeIndex;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        if (!node->visited)
        {
            vxError("Node %p (\"%s\") is unvisited in Graph %p", node, node->kernel->name, graph);
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_GRAPH);
            return VX_ERROR_INVALID_GRAPH;
        }
    }

    vxoGraph_ClearAllVisitedFlags(graph);
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_VerifyAllNodesByTarget(vx_graph graph)
{
    vx_uint32 nodeIndex;
    gcmHEADER_ARG("graph=%p", graph);

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
            gcmFOOTER_ARG("%d", status);
            return status;
        }

        node->id = nodeIndex;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
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

#if defined(__QNX__)
static vx_int32 nnVxcHandleCnt = 0;
static vx_int32 ovx12VxcHandleCnt = 0;
static vx_int32 nnGpuHandleCnt = 0;
#endif

VX_INTERNAL_API vx_status vxoGraph_InitializeAllNodeKernels(vx_graph graph)
{
    vx_uint32 nodeIndex;
    gcmHEADER_ARG("graph=%p", graph);

    vxmASSERT(graph);

#if gcdUSE_VXC_BINARY
#if defined(__QNX__)
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);
    if (vx_false_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        gceSTATUS status = gcvSTATUS_OK;
        if (context->libNNVXCKernelHandle == NULL)
        {
            status = gcoOS_LoadLibrary(gcvNULL, NNVXC_LIB_NAME, &context->libNNVXCKernelHandle);
            if(status != gcvSTATUS_OK) vxError("Can't open libNNVXCBinary!\n");
            nnVxcHandleCnt++;
        }
        else
        {
            nnVxcHandleCnt++;
        }

        if (context->libOvx12VXCBinaryHandle == NULL)
        {
            status = gcoOS_LoadLibrary(gcvNULL, OVX12_VXC_LIB_NAME, &context->libOvx12VXCBinaryHandle);
            if(status != gcvSTATUS_OK) vxError("Can't open libOvx12VXCBinary!\n");
            ovx12VxcHandleCnt++;
        }
        else
        {
            ovx12VxcHandleCnt++;
        }

        if (context->libNNGPUKernelHandle == NULL)
        {
            status = gcoOS_LoadLibrary(gcvNULL, NNGPU_LIB_NAME, &context->libNNGPUKernelHandle);
            if(status != gcvSTATUS_OK) vxError("Can't open libNNGPUBinary!\n");
            nnGpuHandleCnt++;
        }
        else
        {
            nnGpuHandleCnt++;
        }
    }
#else
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);
    if (vx_false_e == vxoBinaryGraph_HasBinaryInGraph(graph))
    {
        gceSTATUS status = gcvSTATUS_OK;
        status = gcoOS_LoadLibrary(gcvNULL, NNVXC_LIB_NAME, &context->libNNVXCKernelHandle);
        if(status != gcvSTATUS_OK) vxError("Can't open libNNVXCBinary!\n");

        status = gcoOS_LoadLibrary(gcvNULL, OVX12_VXC_LIB_NAME, &context->libOvx12VXCBinaryHandle);
        if(status != gcvSTATUS_OK) vxError("Can't open libOvx12VXCBinary!\n");

        status = gcoOS_LoadLibrary(gcvNULL, NNGPU_LIB_NAME, &context->libNNGPUKernelHandle);
        if(status != gcvSTATUS_OK) vxError("Can't open libNNGPUBinary!\n");
    }
#endif
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
                gcmFOOTER_ARG("%d", status);
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

    }
    graph->Initilized = vx_true_e;

#if gcdUSE_VXC_BINARY
#if defined(__QNX__)
    if(context->libNNVXCKernelHandle)
    {
        nnVxcHandleCnt--;
        if (nnVxcHandleCnt == 0)
        {
            gcoOS_FreeLibrary(gcvNULL, context->libNNVXCKernelHandle);
            context->libNNVXCKernelHandle = NULL;
        }
    }

    if(context->libOvx12VXCBinaryHandle)
    {
        ovx12VxcHandleCnt--;
        if (ovx12VxcHandleCnt == 0)
        {
            gcoOS_FreeLibrary(gcvNULL, context->libOvx12VXCBinaryHandle);
            context->libOvx12VXCBinaryHandle = NULL;
        }
    }

    if(context->libNNGPUKernelHandle)
    {
        nnGpuHandleCnt--;
        if (nnGpuHandleCnt == 0)
        {
            gcoOS_FreeLibrary(gcvNULL, context->libNNGPUKernelHandle);
            context->libNNGPUKernelHandle = NULL;
        }
    }
#else
    if(context->libNNVXCKernelHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libNNVXCKernelHandle);

    if(context->libOvx12VXCBinaryHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libOvx12VXCBinaryHandle);

    if(context->libNNGPUKernelHandle)
        gcoOS_FreeLibrary(gcvNULL, context->libNNGPUKernelHandle);
#endif
#endif
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_CaculateCostFactors(vx_graph graph)
{
    vx_uint32 nodeIndex, paramIndex;
    gcmHEADER_ARG("graph=%p", graph);

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
    gcmFOOTER_ARG("%d", VX_SUCCESS);
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
    gcmHEADER_ARG("operation=%p, isNode=0x%x, syncMode=0x%x, semaHandle=0x%x", operation, isNode, syncMode, semaHandle);

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
    gcmFOOTER_NO();
}

VX_PRIVATE_API vx_bool vxoGraph_GetMCFESemphore(vx_graph graph, gctUINT32 *semaHandle)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("graph=%p, semaHandle=%p", graph, semaHandle);

    if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MCFE))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (graph->semaNum == VX_MAX_MCFE_SEMAPHORE)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    status = gcoHAL_AllocateMCFESemaphore(semaHandle);

    if (gcmIS_SUCCESS(status))
    {
        graph->mcfeSema[graph->semaNum] = *semaHandle;
        graph->semaNum++;
    }

    gcmFOOTER_NO();
    return (vx_bool)(status == gcvSTATUS_OK ? vx_true_e : vx_false_e);
}

VX_PRIVATE_API void CheckOperationSyncPattern(vx_graph graph, void* operation1, vx_bool isNode1, void* operation2, vx_bool isNode2)
{
    vx_uint32           syncPattern;
    vxnne_operation_target_e opTarget1, opTarget2;

    gcmHEADER_ARG("graph=%p, operation1=%p, isNode1=0x%x, operation2=%p, isNode2=0x%x", graph, operation1, isNode1, operation2, isNode2);
    if (!operation1 || !operation2)
    {
        gcmFOOTER_NO();
        return;
    }
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
    gcmFOOTER_NO();
}

VX_PRIVATE_API vx_status vxoGraph_VerifyNodeSync(vx_graph graph)
{
    vx_uint32 i;

    void*   currentOperation = VX_NULL;
    vx_bool isCurrentNode = vx_false_e;

    gcmHEADER_ARG("graph=%p", graph);

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
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            CheckOperationSyncPattern(graph, currentOperation, isCurrentNode, (void*)node, vx_true_e);
            currentOperation = (void*)node;
            isCurrentNode = vx_true_e;
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoGraph_VerifyOperationSync(vx_graph graph)
{
    vx_uint32 i;
    vx_uint32 syncPattern;
    vxnne_operation op1, op2;

    gcmHEADER_ARG("graph=%p", graph);

    if (!graph->layer || (graph->layer->opIndicesNum == 0))
    {
        gcmFOOTER_NO();
        return;
    }
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
    gcmFOOTER_NO();
}

VX_PRIVATE_API vx_status vxoGraph_ProcessInternal(vx_graph graph);

VX_PRIVATE_API void vxoGraph_GenerateCommandBuffer(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 *commandBuffer = VX_NULL;
    vx_uint32 outCommandBufferSize = 0;
    gceSTATUS mcfeEnabled = (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MCFE));
    vx_bool   bCaptureOn = vx_false_e;

    gcmHEADER_ARG("graph=%p", graph);

#if gcdDUMP_PER_OPERATION || VXM_FORCE_PER_OPERATION_IDLE
    vxmONERROR(VX_ERROR_NOT_SUPPORTED);
#endif

    if (graph->commandBuffer)
    {
        vxFree(graph->commandBuffer);
        graph->commandBufferSizeInByte = 0;
        graph->commandBuffer = VX_NULL;
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

    if (vx_true_e == vxoBinaryGraph_HasBinaryInGraph(graph))
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
    gcmFOOTER_NO();
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
    gcmFOOTER_NO();
    return;
}

VX_PRIVATE_API void vxoGraph_GeneratePatchLocForInputs(vx_graph graph)
{
    vx_uint32 i;
    vx_uint32 j;

    gcmHEADER_ARG("graph=%p", graph);

    if (graph->commandBuffer == VX_NULL)
    {
        gcmFOOTER_NO();
        return;
    }

    for (i = 0; i < graph->headNodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->headNodeIndexTable[i]];
        for (j = 0; j < node->kernel->signature.paramCount; j++)
        {
            if (node->kernel->signature.stateTable[j] == VX_PARAMETER_STATE_OPTIONAL)
                continue;

            switch (node->paramTable[j]->type)
            {

            case VX_TYPE_TENSOR:
                {
                    vx_tensor tensor = (vx_tensor)node->paramTable[j];
                    vx_uint32 commandSizeInUint = graph->commandBufferSizeInByte / 4;
                    vx_uint32 physical = tensor->tensorBuffer->memory.physicals[0];
                    vx_uint32 location = 0;
                    for (location = 0; location < commandSizeInUint; location++)
                    {
                        if (physical == graph->commandBuffer[location])
                            break;
                    }
                    if (location == commandSizeInUint)
                        location = 0;
                    node->patchLocation[j][0] = location;

                    break;
                }

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
    gcmFOOTER_NO();
    return;
}

VX_INTERNAL_API vx_status vxoGraph_VerifyNNTranspose(vx_graph graph)
{
#define ESTIMATE_KERNEL_CACHE_COEFFICIENT 0.5
    vx_uint32 index = 0, j = 0;
    vxnne_operation operation;
    vx_uint8 transposeOutChannel = 0;
    vx_uint32 outputZ = 0;
    vx_tensor input, output;
    vx_context context = graph->base.context;
    vx_status status = VX_SUCCESS;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_TRANSPOSE))
    {
        for (index = 0; index < graph->layer->base.num_operations; index++)
        {
            vxnne_operation_info_s opInfo;
            operation = graph->layer->operations[index];
            input = (vx_tensor)(operation->inputs[0]);
            output = (vx_tensor)(operation->outputs[0]);

            vxnneOperation_GetInfo(operation, &opInfo);
            if (opInfo.target == VXNNE_OPERATION_TARGET_NN)
            {
                vx_uint32 outputDims[3] = {TENSOR_SIZE_INDEX(opInfo.output, 0), TENSOR_SIZE_INDEX(opInfo.output, 1), TENSOR_SIZE_INDEX(opInfo.output, 2)};
                vx_uint32 inputDims[3]  = {TENSOR_SIZE_INDEX(opInfo.input, 0), TENSOR_SIZE_INDEX(opInfo.input, 1), TENSOR_SIZE_INDEX(opInfo.input, 2)};
                vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat;
                vx_arch_perf_s archPerfHandle;
                vx_uint8 transposeInChannel = 0;

                outputZ = TENSOR_VIEW_SIZE_INDEX(output, 2);
                if ((WB_KERNEL_X(((vxnne_convolution_relu_pooling_operation)operation)->weights_biases) == 1) &&
                    (WB_KERNEL_Y(((vxnne_convolution_relu_pooling_operation)operation)->weights_biases) == 1))
                {
                    transposeInChannel = VX_TRANSPOSE_MAX_INTERLEAVE_1MULTI1_CH;
                }
                else
                    transposeInChannel = (vx_uint8)gcmMIN(outputZ, VX_TRANSPOSE_MAX_INTERLEAVE_CH);

                INITIALIZE_STRUCT(archPerfHandle);
                calculateArchPerfFromWB(context,
                                    &archPerfHandle,
                                    opInfo.weightsBiases,
                                    inputDims,
                                    outputDims,
                                    TENSOR_DATA_TYPE(output),
                                    VX_NULL,
                                    vx_true_e,
                                    SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR,
                                    context->vipSRAM.size,
                                    (vxnne_operation_target_e)opInfo.target,
                                    (vxnne_operator_e)opInfo.opType);

                outImageTileX   = archPerfHandle.resultInfo.outImageTileXSize;
                outImageTileY   = archPerfHandle.resultInfo.outImageTileYSize;
                interleaveMode  = archPerfHandle.resultInfo.interleaveMode;
                kernelX         = opInfo.weightsBiases->weights_sizes[0];
                kernelY         = opInfo.weightsBiases->weights_sizes[1];
                inImageZ        = TENSOR_SIZE_INDEX(opInfo.input, 2);
                inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

                operation->transposeInSize = caculateInputTransposeBufferSize(VXNNE_SRAM_CACHE_MODE_FULL_CACHE,
                                                outImageTileX,
                                                outImageTileY,
                                                kernelX,
                                                kernelY,
                                                inputDims[2],
                                                interleaveMode,
                                                context->nnConfig.customizedFeature.ddrLatency,
                                                transposeInChannel,
                                                input->tensorBuffer->dataFormat);

                operation->transposeOutSize = caculateOutTransposeBufferSize(outImageTileX, outImageTileY, output->tensorBuffer->dataFormat);
                operation->transposeKernelSize = GetEsitimateWBSize(opInfo.weightsBiases);
            }
        }

        for (index = 0; index < graph->layer->base.num_operations; index++)
        {
            operation = graph->layer->operations[index];

            input = (vx_tensor)(operation->inputs[0]);
            output = (vx_tensor)(operation->outputs[0]);

            /* skip TP, shader operation */
            if (operation->target != VXNNE_OPERATION_TARGET_NN)
            {
                operation->bTransposeIn = vx_false_e;
                operation->bTransposeOut = vx_false_e;

                vxmASSERT((operation->parentOpNum == 0 )|| operation->bTransposeIn  == operation->parentOps[0]->bTransposeOut);
                continue;
            }

            if (operation->parentOpNum == 1)
            {
                operation->bTransposeIn = operation->parentOps[0]->bTransposeOut;
                operation->transposeInChannel = input->tensorBuffer->memory.transposeChannel;
            }
            else
            {
                operation->bTransposeIn = vx_false_e;
                operation->transposeInChannel = 0;
            }

            vxmASSERT((operation->parentOpNum == 0 )|| operation->bTransposeIn  == operation->parentOps[0]->bTransposeOut);

            /* skip last layer's output */
            if (vxoTensor_IsVirtualTensor(output) == 0 ||
                output->isViewed ||
                (operation->childOpNum == 0) ||
                (((operation->bTransposeIn ? operation->transposeInSize : 0) + operation->transposeOutSize + operation->transposeKernelSize * ESTIMATE_KERNEL_CACHE_COEFFICIENT) >= context->vipSRAM.size))
            {
                operation->bTransposeOut = vx_false_e;
                continue;
            }

            outputZ = TENSOR_VIEW_SIZE_INDEX(output, 2);
            transposeOutChannel = (vx_uint8)gcmMIN(outputZ, VX_TRANSPOSE_MAX_INTERLEAVE_CH);

            for (j = 0; j < operation->childOpNum; j++)
            {
                vxnne_operation childOp = operation->childOps[j];

                if ((childOp->target != VXNNE_OPERATION_TARGET_NN) ||
                    (((vx_tensor)childOp->inputs[0])->isViewed) ||
                    (childOp->inputs[0] != operation->outputs[0]) ||
                    ((childOp->transposeInSize + childOp->transposeKernelSize * ESTIMATE_KERNEL_CACHE_COEFFICIENT) >= context->vipSRAM.size))
                {
                    transposeOutChannel = 0;
                    break;
                }

                if ((childOp->target == VXNNE_OPERATION_TARGET_NN) &&
                    (WB_KERNEL_X(((vxnne_convolution_relu_pooling_operation)childOp)->weights_biases) == 1) &&
                    (WB_KERNEL_Y(((vxnne_convolution_relu_pooling_operation)childOp)->weights_biases) == 1))
                {
                    transposeOutChannel = VX_TRANSPOSE_MAX_INTERLEAVE_1MULTI1_CH;
                }
            }

            operation->bTransposeOut = (transposeOutChannel > 0) ? vx_true_e : vx_false_e;
            operation->transposeOutChannel = transposeOutChannel;
            output->tensorBuffer->memory.transposeChannel = operation->transposeOutChannel;
        }
    }

    return status;
}


VX_PRIVATE_API vx_status vxoGraph_VerifyGraph(vx_graph graph)
{
    vx_status status;
    gcePATCH_ID patchID = gcvPATCH_INVALID;

    vx_bool first = ((graph->verified == vx_false_e) && (graph->reverify == vx_false_e)) ? vx_true_e : vx_false_e;

    vx_context context = vxGetContext((vx_reference)graph);
    gcmHEADER_ARG("graph=%p", graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    vxAcquireMutex(graph->base.lock);
    gcoHAL_GetPatchID(gcvNULL,&patchID);

    /* CTS: UserNode.UserKernel/42/_FROM_REF/ UserNode.UserKernel/42/_FROM_ATTR/ w/a*/
    if (graph->nodeTable[0]->kernel->isUserkernel && patchID == gcvPATCH_OVX_CTS)
        graph->verified = vx_false_e;
    else if (graph->verified)
    {
        vxReleaseMutex(graph->base.lock);
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }

    vxmONERROR(vxoGraph_UserKernelPreprocess(graph, first));

    vxmONERROR(vxoGraph_VerifyAllNodeParameters(graph));

    vxmONERROR(vxoGraph_VerifyAllNodeWriteDependencies(graph));

    vxmONERROR(vxoGraph_RetrieveTopology(graph));

    vxoBinaryGraph_CacheOrImport(graph);

    if (graph->base.context->options.enableGraphAdapter)
    {
        vxmONERROR(vxoGraph_Adapter(graph));
    }

    vxmONERROR(vxoGraph_AllocateAllMemoryObjects(graph));

    vxmONERROR(vxoGraph_DetectAllHeadNodes(graph));

    vxmONERROR(vxoGraph_DetectAllTailNodes(graph));

    vxmONERROR(vxoBinaryGraph_GetGraphInputOutput(graph));

    vxmONERROR(vxoGraphOptimization(graph));

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
        char *envConfig = NULL;
        vxoGraph_GenerateOpParentChild(graph);

        if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_TP_PARALLEL))
        {
            vxoGraphParallel_AnalyzeOperationsBefore(graph);
        }

        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_IN_IMAGE_NON_ZERO_RATIO_CONFIG", &envConfig)) && envConfig)
        {
            vxmONERROR(DetectInImageNonZeroRatioFromConfig(graph, envConfig));
        }

        if (graph->base.context->options.collectPerfType == COLLECT_PERF_ESTIMATE)
        {
            vxoGraph_PredictPerf(graph);
        }

        if (context->options.enableAllocateContigousMemForKernel)
        {
            vxmONERROR(vxoGraph_AllocateContiguousMemory(graph));
        }

        vxmONERROR(vxoGraph_VerifyNNTranspose(graph));

        vxmONERROR(vxoGraph_VerifyTiling(graph));

        vxmONERROR(vxoGraph_VerifyVirtualBuffer(graph));

        vxmONERROR(vxoBinaryGraph_SaveBinaryEntrance(graph));

        vxmONERROR(vxnneExecutionLayer_GenerateCommands(graph->base.context, &graph->layer->base));

        if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_TP_PARALLEL))
        {
            vxoGraphParallel_AnalyzeOperationsAfter(graph);
        }

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
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:

    graph->reverify = vx_false_e;

    graph->verified = vx_false_e;

    graph->status   = VX_GRAPH_STATE_UNVERIFIED;

    vxReleaseMutex(graph->base.lock);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxVerifyGraph: graph=%p", graph);

    vxmASSERT(graph);
    if(graph->parentGraph == gcvNULL)
    {
        gcmONERROR(gcoVX_SwitchContext(graph->deviceID, &savedHardware, &savedHardwareType, &savedCoreIndex));
    }
    status = vxoGraph_VerifyGraph(graph);

    if(graph->parentGraph == gcvNULL)
    {
        gcoVX_RestoreContext(savedHardware, savedHardwareType, savedCoreIndex);
    }
    gcmFOOTER_ARG("%d", status);
    return status;

OnError:
    vxError("%s[%d]: Verify Graph fail!\n", __FUNCTION__, __LINE__);
    vxAddLogEntry(&graph->base, VX_FAILURE, "%s[%d]: Verify Graph fail!\n", __FUNCTION__, __LINE__);
    gcmFOOTER_ARG("%d", VX_FAILURE);
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
    gcoVX_FlushCache(vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e);
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
            gcoVX_FlushCache(vx_false_e, vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e);
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
                gcoVX_FlushCache(vx_false_e, vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e);
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
        gcoVX_FlushCache(vx_false_e, vx_true_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e);
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
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    vxoGraph_ClearAllVisitedFlags(graph);

    vxoGraph_ClearAllExecutedFlags(graph);

    if (graph->base.context->options.enableCNNPerf)
        vxoPerf_Begin(&graph->perf);
    gcmFOOTER_NO();
}

VX_PRIVATE_API vx_status vxoGraph_ProcessKernelPrint(vx_graph graph)
{
    vx_uint32 i;
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    if (!graph->layer)
    {
        return VX_SUCCESS;
    }

    /* this path is for openvx1.2 build kernel coding with vxc, will refine them to integrate to the path "layer!= NULL" */
    if (graph->layer == VX_NULL)
    {
        for (i = 0; i < graph->nodeCount; i++)
        {
            vx_node node = graph->nodeTable[i];
            if (node->kernel->kernelShader &&
                node->kernel->kernelShader[node->kernel->currShaderID]->hasPrintf)
            {
                vx_shader shader;
                shader = node->kernel->kernelShader[node->kernel->currShaderID];
                vxmONERROR(vxoKernel_ProcessKernelShaderPrint(shader,
                    &node->kernelAttributes.shaderParameter));
            }
        }
    }
    else /* layer != NULL */
    {
        for (i = 0; i < graph->layer->base.num_operations; i++)
        {
            if (graph->layer->operations[i]->target == VXNNE_OPERATION_TARGET_SH)
            {
                vxnne_shader_operation shaderOperation  = (vxnne_shader_operation)graph->layer->operations[i];
                vx_shader shader = shaderOperation->shaderExecutable->kernelShader;

                if (shader->hasPrintf)
                {
                    vxmONERROR(vxoKernel_ProcessKernelShaderPrint(shader,
                        &shaderOperation->shaderExecutable->shaderParam));
                }
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API void vxoGraph_EndProcess(vx_graph graph)
{
    vx_uint32 i, j;

    gcmHEADER_ARG("graph=%p", graph);
    vxmASSERT(graph);

    if (!graph->isChildGraph)
    {
        gcfVX_Flush(gcvTRUE);
    }

    if ((graph->binarySave) && (!graph->isChildGraph))
    {
        vx_status status = VX_SUCCESS;
        status = gcfVX_CaptureState(gcvNULL, 0, &graph->binarySave->endCommandsSize, gcvFALSE, gcvFALSE);
        if (status != VX_SUCCESS)
        {
            vxError("fail to capture end operation for generating binary graph\n");
        }
        if (graph->binarySave->endCommandsSize > 0)
        {
            vxoBinaryGraph_SaveEndOperation(graph,
                                            (vx_uint8_ptr)graph->binarySave->endCommands,
                                            (vx_uint32)graph->binarySave->endCommandsSize);
        }
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

                if (!image ) continue;
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

                if (!tensor ) continue;
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

    if (vxoContext_IsFeatureAvailable(graph->base.context, VX_NN_TP_PARALLEL))
    {
        gcfVX_Flush(gcvTRUE);
    }

    vxoGraph_ProcessKernelPrint(graph);

    graph->dirty = vx_false_e;

    if (graph->base.context->options.enableCNNPerf)
    {
        vxoPerf_End(&graph->perf);
        vxoPerf_Dump(&graph->perf);
    }

    if (graph->binarySave)
    {
        /* close binary graph file */
        vxoBinaryGraph_ReSaveInputAndPatchTable(graph);
    }

    vxoGraph_ClearAllVisitedFlags(graph);
    gcmFOOTER_NO();
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
                    -1, 0, 0, NULL, NULL, 0, -1, -1, (vx_int32)node->id, (vx_int32)graph->graphID, gcvTRUE);
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

    gcmHEADER_ARG("graph=%p", graph);

#if PRE_SORT
    if (graph->layer)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[0]];
        vx_target target = &graph->base.context->targetTable[node->targetIndex];

        vxoNode_EnableVirtualAccessible(node);
        action = target->funcs.processLayer(target, &graph->layer->base);
        vxoNode_DisableVirtualAccessible(node);
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

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status vxoGraph_Submit(vx_graph graph)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("graph=%p", graph);

    if (!graph->commandBuffer)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NOT_IMPLEMENTED);
        return VX_ERROR_NOT_IMPLEMENTED;
    }

    gcmONERROR(gcoVX_Replay((gctPOINTER)graph->commandBuffer, (gctUINT32)graph->commandBufferSizeInByte));

OnError:
    gcmFOOTER_NO();
    return gcmIS_SUCCESS(status) ? VX_SUCCESS : VX_FAILURE;
}


VX_PRIVATE_API vx_status vxoGraph_Process(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    gcmHEADER_ARG("graph=%p", graph);

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
            gcmFOOTER_ARG("%d", status);
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

        vxo_updateSwapHandle(graph);

        if (graph->commandBuffer)
        {
            if (graph->scalerHead)
            {
                status |= vxoGraph_ProcessInternal(graph);
            }
            if (graph->commandBufferSizeInByte > 0)
            {
                status |= vxoGraph_Submit(graph);
            }
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
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxScheduleGraph: graph=%p", graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (!vxTryAcquireMutex(graph->scheduleLock, 0))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_GRAPH_SCHEDULED);
        return VX_ERROR_GRAPH_SCHEDULED;
    }
    vxZeroMemory(&graph->data, sizeof(vx_value_set_s));
    graph->data.v1 = (vx_return_value)graph;

    if (!vxoQueue_WriteData(&graph->base.context->processor.input, &graph->data))
    {
        vxReleaseMutex(graph->scheduleLock);
        return VX_ERROR_NO_RESOURCES;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    vx_status       status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxWaitGraph: graph=%p", graph);

    if (!vxoReference_IsValidAndNoncontext(&graph->base))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    while (1)
    {
        vx_value_set    data = VX_NULL;

        if (!vxoQueue_ReadData(&graph->base.context->processor.output, OUT &data))
        {
            vxReleaseMutex(graph->scheduleLock);
            gcmFOOTER_ARG("%d", VX_FAILURE);
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

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    gcoHARDWARE savedHardware = gcvNULL;
    gctUINT32 savedCoreIndex = 0;
    gceHARDWARE_TYPE savedHardwareType = gcvHARDWARE_INVALID;
    vx_status status = VX_SUCCESS;
    gctBOOL switched = gcvFALSE;

    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxProcessGraph: graph=%p", graph);

    vxmASSERT(graph);

    if (!vxoReference_IsValidAndNoncontext(&graph->base))
    {
        vxError("%s[%d]: Graph's reference is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Graph's reference is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
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
    if (status != VX_SUCCESS) goto OnError;

    if(graph->parentGraph == gcvNULL)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
OnError:
    if(switched)
    {
        gcoVX_RestoreContext(savedHardware,savedHardwareType,savedCoreIndex);
    }

    vxError("%s[%d]: Process Graph fail!\n", __FUNCTION__, __LINE__);
    vxAddLogEntry(&graph->base, status, "%s[%d]: Process Graph fail!\n", __FUNCTION__, __LINE__);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter param)
{
    gcmHEADER_ARG("graph=%p, param=%p", graph, param);
    gcmDUMP_API("$VX vxAddParameterToGraph: graph=%p, param=%p", graph, param);

    if (!vxoReference_IsValidAndNoncontext(&graph->base))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoGraph_SetParameter(vx_graph graph, vx_uint32 index, vx_reference value)
{
    vx_status status;

    gcmHEADER_ARG("graph=%p, index=0x%x, value=%p", graph, index, value);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (index >= VX_MAX_PARAMETERS)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
        return VX_ERROR_INVALID_VALUE;
    }
    status = vxoNode_SetParameter(graph->paramTable[index].node, graph->paramTable[index].index, value);

    if (status != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    graph->dirty = vx_true_e;

    graph->reverify = vx_false_e;

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoGraph_IdentifyInputsAndOutputs(
    vx_graph graph,
    vx_uint32 num_of_inputs,
    vx_reference *inputs,
    vx_uint32 num_of_outputs,
    vx_reference *outputs
    )
{
    vx_status status = VX_SUCCESS;

    vx_reference *input_refs = VX_NULL;
    vx_reference *output_refs = VX_NULL;
    vx_uint32 i;

    if (!num_of_inputs || !inputs || !num_of_outputs || !outputs)
    {
        vxmONERROR(VX_ERROR_INVALID_PARAMETERS);
    }

    input_refs = (vx_reference *)vxAllocateAndZeroMemory(num_of_inputs * gcmSIZEOF(vx_reference));
    vxmONERROR_NULLPTR(input_refs);

    for (i = 0; i < num_of_inputs; i++)
    {
        input_refs[i] = inputs[i];
    }

    output_refs = (vx_reference *)vxAllocateAndZeroMemory(num_of_outputs * gcmSIZEOF(vx_reference));
    vxmONERROR_NULLPTR(output_refs);

    for (i = 0; i < num_of_outputs; i++)
    {
        output_refs[i] = outputs[i];
    }

    if (graph->inputs)
    {
        vxFree(graph->inputs);
        graph->inputs = VX_NULL;
    }

    if (graph->outputs)
    {
        vxFree(graph->outputs);
        graph->outputs = VX_NULL;
    }

    graph->inputCount = num_of_inputs;
    graph->inputs = input_refs;
    graph->outputCount = num_of_outputs;
    graph->outputs = output_refs;

    return status;

OnError:
    if (input_refs)
    {
        vxFree(input_refs);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphParameterByIndex(vx_graph graph, vx_uint32 index, vx_reference value)
{
    gcmHEADER_ARG("graph=%p, index=0x%x, value=%p", graph, index, value);
    gcmDUMP_API("$VX vxSetGraphParameterByIndex: graph=%p, index=0x%x, value=%p", graph, index, value);
    gcmFOOTER_NO();
    return vxoGraph_SetParameter(graph, index, value);
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    gcmHEADER_ARG("graph=%p, index=0x%x", graph, index);
    gcmDUMP_API("$VX vxGetGraphParameterByIndex: graph=%p, index=0x%x", graph, index);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    if (index >= graph->paramCount)
    {
        gcmFOOTER_NO();
        return (vx_parameter)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_REFERENCE);
    }

    gcmFOOTER_NO();
    return vxoNode_GetParameter(graph->paramTable[index].node, graph->paramTable[index].index);
}

VX_API_ENTRY vx_bool VX_API_CALL vxIsGraphVerified(vx_graph graph)
{
    gcmHEADER_ARG("graph=%p", graph);
    gcmDUMP_API("$VX vxIsGraphVerified: graph=%p", graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return vx_false_e;

    gcmFOOTER_NO();
    return graph->verified;
}

VX_API_ENTRY vx_status VX_API_CALL
vxIdentifyGraphInputsAndOutputs(
    vx_graph graph,
    vx_uint32 num_of_inputs,
    vx_reference *inputs,
    vx_uint32 num_of_outputs,
    vx_reference *outputs
    )
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("graph=%p, num_of_inputs=%u, inputs=%p, num_of_outputs=%u, outputs=%p",
                  graph, num_of_inputs, inputs, outputs, outputs);
    gcmDUMP_API("$VX vxIdentifyGraphInputsAndOutputs: graph=%p, num_of_inputs=%u, inputs=%p, num_of_outputs=%u, outputs=%p",
                graph, num_of_inputs, inputs, outputs, outputs);

    status =  vxoGraph_IdentifyInputsAndOutputs(graph,
                                                num_of_inputs,
                                                inputs,
                                                num_of_outputs,
                                                outputs);
    gcmFOOTER_NO();

    return status;
}


