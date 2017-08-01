/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

#define PRE_SORT 1

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

    vxmASSERT(vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH));

    while (graph->nodeCount > 0)
    {
        vx_node node = (vx_node)graph->nodeTable[0];

        if (vxoReference_GetExternalCount(&node->base) > 0)
        {
            vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_EXTERNAL);
        }

        vxoNode_RemoveFromGraph(&graph->nodeTable[0]);
    }

    vxDestroyMutex(graph->scheduleLock);
}

VX_PRIVATE_API vx_uint32 vxoGraph_GetNextNodeIndex(vx_graph graph, vx_uint32 nodeIndex)
{
    vxmASSERT(graph);
    vxmASSERT(nodeIndex < graph->nodeCount);

    return (nodeIndex + 1) % graph->nodeCount;
}

VX_PRIVATE_API vx_bool vxoReference_HasWriteDependency(vx_reference ref1, vx_reference ref2)
{
    if (ref1 == VX_NULL || ref2 == VX_NULL) return vx_false_e;
    if ((ref1->type != VX_TYPE_TENSOR && ref2->type != VX_TYPE_TENSOR) && (ref1 == ref2)) return (((vx_array)ref1)->base.isStage)?vx_false_e:vx_true_e;/*if is staging reference, skip dependency checking*/

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

        vx_view_region_s viewRegion1, viewRegion2;
        vx_tensor_buffer_s* tensorBuf1, *tensorBuf2;
        tensorBuf1 = vxoTensor_LocateView((vx_tensor)ref1, &viewRegion1);
        tensorBuf2 = vxoTensor_LocateView((vx_tensor)ref2, &viewRegion2);

        if (tensorBuf1 == tensorBuf2)
        {
            if (viewRegion1.dimCount == viewRegion2.dimCount)
            {
                vx_uint32 dimIndex = viewRegion1.dimCount - 1;
                vx_bool isInterCross = vx_false_e;

                do
                {
                    vx_bool result = vx_false_e;
                    if ((viewRegion1.viewStarts[dimIndex] < viewRegion2.viewEnds[dimIndex])
                        && (viewRegion1.viewEnds[dimIndex] > viewRegion2.viewStarts[dimIndex]))
                    {
                        result = vx_true_e;
                    }

                    isInterCross = (vx_bool)(isInterCross && result);

                    dimIndex--;
                }while (dimIndex > 0);

                if (isInterCross)
                {
                    return vx_true_e;
                }

            }
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

    if (!vxoContext_IsValid(context)) return VX_NULL;

    graph = (vx_graph)vxoReference_Create(context, VX_TYPE_GRAPH, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)graph) != VX_SUCCESS) return graph;

    graph->dirty = vx_true_e;

    graph->reverify = graph->verified;

    graph->verified = vx_false_e;

    graph->status = VX_GRAPH_STATE_UNVERIFIED;

    vxCreateMutex(OUT &graph->scheduleLock);

    vxoPerf_Initialize(&graph->perf);

    vxoGraph_Dump(graph);

    return graph;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    return VX_ERROR_NOT_SUPPORTED;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
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

    vx_char log[1024] = {0};
    vx_char outputBuffer[1024 * 100] = {'\0'};
    vx_char tempBuffer[1024 * 100] = {'\0'};

    FILE* fp = NULL;
    vx_char * output = (buffer != NULL)? buffer:outputBuffer;

    if(!graph)
        return VX_ERROR_INVALID_GRAPH;

    if (only_top && graph->isChildGraph)
        return VX_ERROR_INVALID_GRAPH;

    strcat(output, "    <graph>\n");
    sprintf(log, "        <name>%p</name>\n", graph);
    strcat(output, log);
    sprintf(log, "        <start>%llu</start>\n", (unsigned long long)graph->perf.beg);
    strcat(output, log);
    sprintf(log, "        <end>%llu</end>\n", (unsigned long long)graph->perf.end);
    strcat(output, log);
    sprintf(log, "        <interval>%llu</interval>\n\n", (unsigned long long)graph->perf.tmp);
    strcat(output, log);

    for (nodeIndex = 0; nodeIndex <graph->nodeCount; nodeIndex++)
    {
        vx_node node = graph->nodeTable[nodeIndex];

        strcat(output, "        <node>\n");

                sprintf(log, "            <name>%s</name>\n", node->kernel->name);
                strcat(output, log);

                sprintf(log, "            <start>%llu</start>\n", (unsigned long long)node->perf.beg);
                strcat(output, log);

                sprintf(log, "            <end>%llu</end>\n", (unsigned long long)node->perf.end);
                strcat(output, log);

                sprintf(log, "            <interval>%llu</interval>\n\n", (unsigned long long)node->perf.tmp);
                strcat(output, log);

        strcat(output, "        </node>\n");

        if (node->childGraph != NULL)
        {
            memset(tempBuffer, 0, sizeof(tempBuffer));
            vx_vivPeferGraph(node->childGraph, NULL, vx_false_e, vx_false_e, tempBuffer);
            strcat(output, tempBuffer);
        }
    }

        strcat(output, "    </graph>\n");

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
    vx_char* path;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_GRAPH_PERF", &path);
    if (path)
        vx_vivPeferGraph(*graph, path, vx_true_e, vx_true_e, NULL);


    return vxoReference_Release((vx_reference_ptr)graph, VX_TYPE_GRAPH, VX_REF_EXTERNAL);
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
        if ((*vRef)->scope->type == VX_TYPE_GRAPH &&
            (*vRef)->scope != (vx_reference_s *)graph &&
            (*vRef)->scope != (vx_reference_s *)graph->parentGraph)
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
        in_rect = (vx_rectangle_t**)malloc(num_in_images * sizeof(vx_rectangle_t*));
        if (NULL == in_rect)
        {
            return vx_false_e;
        }

        for (i = 0; i < nparams; i++)
        {
            if (VX_INPUT == node->kernel->signature.directionTable[i] &&
                VX_TYPE_IMAGE == node->paramTable[i]->type)
            {
                in_rect[i] = (vx_rectangle_t*)malloc(sizeof(vx_rectangle_t));
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
            out_rect[0] = (vx_rectangle_t*)malloc(sizeof(vx_rectangle_t));
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

            free(out_rect[0]);
        }

        /* deallocate arrays memory */
        for (i = 0; i < num_in_images; i++)
        {
            if (NULL != in_rect[i])
                free(in_rect[i]);
        }

        if (NULL != in_rect)
            free(in_rect);

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
                vxError("meta: type 0x%08x, 0x%08x "VX_FMT_SIZE"\n", metaFormat->type, metaFormat->u.objectArrayInfo.item_type, metaFormat->u.objectArrayInfo.item_count);

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
                        if (vxoReference_GetType(paramRef->scope) == VX_TYPE_GRAPH
                            && (vx_graph)paramRef->scope != graph
                            && paramRef->scope != (vx_reference)graph->parentGraph
                            )
                    {
                        vxError("Node %p(\"%s\") in Graph %p: No.%d virtual parameter has an invalid scope, %p",
                                node, node->kernel->name, graph, paramIndex, paramRef->scope);
                        status = VX_ERROR_INVALID_SCOPE;
                        continue;
                    }
                }
                    else
                        vRef = NULL;


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
                        vxError("Node %p(\"%s\"): Can't allocate memory for No.%d image parameter",
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

                case VX_TYPE_TENSOR:
                    {
                        if (vxoTensor_AllocateMemory((vx_tensor)paramRef) != VX_SUCCESS)
                        {
                            vxError("Node %p(\"%s\"): Can't allocate memory for No.%d tensor parameter",
                                node, node->kernel->name, paramIndex);
                            status = VX_ERROR_NO_MEMORY;
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

VX_PRIVATE_API vx_status vxoGraph_DetectAllHeadNodes(vx_graph graph)
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
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status vxoGraph_InitializeAllNodeKernels(vx_graph graph)
{
    vx_uint32 nodeIndex;

    vxmASSERT(graph);
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
    }

    graph->Initilized = vx_true_e;


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

#if gcdVX_OPTIMIZER
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


VX_API_ENTRY vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    vx_status status;
    vx_bool first = ((graph->verified == vx_false_e) && (graph->reverify == vx_false_e)) ? vx_true_e : vx_false_e;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_REFERENCE;

    vxAcquireMutex(graph->base.lock);

    /* CTS UserNode.UserKernel/42/_FROM_REF/ UserNode.UserKernel/42/_FROM_ATTR/ w/a*/
    if (graph->nodeTable[0]->kernel->isUserkernel)
        graph->verified = vx_false_e;
    else if (graph->verified)
        return VX_SUCCESS;

    status = vxoGraph_UserKernelPreprocess(graph, first);

    if (status != VX_SUCCESS) goto ErrorExit;


    status = vxoGraph_VerifyAllNodeParameters(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_VerifyAllNodeWriteDependencies(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_AllocateAllMemoryObjects(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_DetectAllHeadNodes(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_DetectCycle(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_DetectUnvisitedNodes(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_VerifyAllNodesByTarget(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_InitializeAllNodeKernels(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    status = vxoGraph_CaculateCostFactors(graph);

    if (status != VX_SUCCESS) goto ErrorExit;

    graph->reverify = vx_false_e;

    graph->verified = vx_true_e;

    graph->status   = VX_GRAPH_STATE_VERIFIED;

#if gcdVX_OPTIMIZER
    vxoGraph_Flatten(graph);

    /* Only top-level graph needs to be optimized. */
    if (!graph->isSubGraph)
    {
        vxoGraph_Optimize(graph);
    }
#endif

#if PRE_SORT
    vxoGraph_GenerateAllNodeIndexTable(graph);
#endif


    vxReleaseMutex(graph->base.lock);

    if (status != VX_SUCCESS) goto ErrorExit;

    return VX_SUCCESS;

ErrorExit:

    graph->reverify = vx_false_e;

    graph->verified = vx_false_e;

    graph->status   = VX_GRAPH_STATE_UNVERIFIED;

    vxReleaseMutex(graph->base.lock);

    return status;
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

VX_PRIVATE_API void vxoGraph_BeginProcess(
        vx_graph graph, OUT vx_uint32 nextNodeIndexTable[VX_MAX_NODE_COUNT], OUT vx_uint32 * nextNodeCountPtr)
{
    vxmASSERT(graph);
#if !PRE_SORT
    vxmASSERT(nextNodeIndexTable);
    vxmASSERT(nextNodeCountPtr);
#endif

    vxoGraph_ClearAllVisitedFlags(graph);

    vxoGraph_ClearAllExecutedFlags(graph);
#if !PRE_SORT
    vxCopyNodeIndexTable(graph->headNodeIndexTable, graph->headNodeCount,
                        OUT nextNodeIndexTable, OUT nextNodeCountPtr);
#endif

    if (graph->base.context->perfEnable)
        vxoPerf_Begin(&graph->perf);
}

VX_PRIVATE_API vx_status vxoGraph_ProcessKernelPrint(vx_graph graph)
{
    vx_uint32 nodeIndex;
    vx_status status = VX_SUCCESS;
    vxmASSERT(graph);

    for (nodeIndex = 0; nodeIndex < graph->nodeCount; nodeIndex++)
    {
        if (graph->nodeTable[nodeIndex]->executed)
        {
            vx_kernel kernel = graph->nodeTable[nodeIndex]->kernel;

            status = vxoKernel_ProcessKernelShaderPrint(kernel, &graph->nodeTable[nodeIndex]->kernelAttributes.shaderParameter);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

OnError:
    return status;
}

VX_PRIVATE_API void vxoGraph_EndProcess(vx_graph graph)
{
    vxmASSERT(graph);


    gcfVX_Flush(gcvTRUE);

    vxoGraph_ProcessKernelPrint(graph);

    graph->dirty = vx_false_e;

    vxoPerf_End(&graph->perf);

    vxoPerf_Dump(&graph->perf);

    vxoGraph_ClearAllVisitedFlags(graph);
}


VX_PRIVATE_API vx_status vxoGraph_Process(vx_graph graph)
{
    vx_status status = VX_SUCCESS;
    vx_action action;
#if !PRE_SORT
    vx_uint32 lastNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 nextNodeIndexTable[VX_MAX_NODE_COUNT];
    vx_uint32 leftNodeIndexTable[VX_MAX_NODE_COUNT] = {0};
    vx_uint32 lastNodeCount, nextNodeCount, leftNodeCount = 0;
#endif
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
        status = vxVerifyGraph(graph);

        if (status != VX_SUCCESS) return status;
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

        action = VX_ACTION_CONTINUE;
        graph->status = VX_GRAPH_STATE_RUNNING;

#if PRE_SORT
        vxoGraph_BeginProcess(graph, OUT gcvNULL, OUT gcvNULL);
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

            if (action != VX_ACTION_CONTINUE) break;
        }
#else
        vxoGraph_BeginProcess(graph, OUT nextNodeIndexTable, OUT &nextNodeCount);
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

                if (action != VX_ACTION_CONTINUE) break;
            }

            if (action != VX_ACTION_CONTINUE) break;

            vxCopyNodeIndexTable(nextNodeIndexTable, nextNodeCount, OUT lastNodeIndexTable, OUT &lastNodeCount);

            vxoGraph_GenerateNextNodeTable(graph, lastNodeIndexTable, lastNodeCount,
                                            OUT nextNodeIndexTable, OUT &nextNodeCount,
                                            INOUT leftNodeIndexTable, INOUT &leftNodeCount);
        }
#endif
        switch (action)
        {
            /*case VX_ACTION_RESTART:
                goto Restart;*/

            case VX_ACTION_ABANDON:
                status = VX_ERROR_GRAPH_ABANDONED;
                break;
        }

        vxoGraph_EndProcess(graph);

        if (action == VX_ACTION_ABANDON)
            graph->dirty = vx_true_e;

        for (i = 0; i < VX_MAX_REF_COUNT; i++)
        {
            if (vxoReference_IsValidAndSpecific(&graph->delays[i]->base, VX_TYPE_DELAY) == vx_true_e)
                vxAgeDelay(graph->delays[i]);
        }
    }


#if VIVANTE_PROFILER
    if (!graph->isChildGraph)
    {
        vxoProfiler_End((vx_reference)graph);
    }
#endif

    if (status == VX_SUCCESS)
        graph->status = VX_GRAPH_STATE_COMPLETED;
    else
    {
        graph->status = VX_GRAPH_STATE_ABANDONED;

        return status;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
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
    if (!vxoReference_IsValidAndNoncontext(&graph->base)) return VX_ERROR_INVALID_REFERENCE;
#if (0 && !PRE_SORT)
    vxoGraph_Sort(graph);
#endif

    return vxoGraph_Process(graph);
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter param)
{
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
    return vxoGraph_SetParameter(graph, index, value);
}

VX_API_ENTRY vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    if (index >= graph->paramCount)
    {
        return (vx_parameter)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_REFERENCE);
    }

    return vxoNode_GetParameter(graph->paramTable[index].node, graph->paramTable[index].index);
}

VX_API_ENTRY vx_bool VX_API_CALL vxIsGraphVerified(vx_graph graph)
{
    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return vx_false_e;

    return graph->verified;
}

