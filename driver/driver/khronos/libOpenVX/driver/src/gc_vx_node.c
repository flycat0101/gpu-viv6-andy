/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>
#include <gc_hal_vx.h>

VX_PRIVATE_API vx_node vxoNode_CreateGeneric(vx_graph graph, vx_kernel kernel)
{
    vx_uint32 i;
    vx_node node;

    vxmASSERT(graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        vxError("The graph, %p, is invalid", graph);
        return VX_NULL;
    }

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        vxError("The kernel, %p, is invalid", kernel);
        return (vx_node)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_REFERENCE);
    }

    vxAcquireMutex(graph->base.lock);

    for (i = 0; i < VX_MAX_NODE_COUNT; i++)
    {
        if (graph->nodeTable[i] == VX_NULL) break;
    }

    if (i == VX_MAX_NODE_COUNT)
    {
        vxReleaseMutex(graph->base.lock);

        vxError("Too many nodes");

        return (vx_node)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_NO_RESOURCES);
    }

    node = (vx_node)vxoReference_Create(graph->base.context, VX_TYPE_NODE, VX_REF_EXTERNAL, &graph->base);

    if (vxoReference_GetStatus((vx_reference)node) != VX_SUCCESS)
    {
        vxReleaseMutex(graph->base.lock);

        return node;
    }

    node->kernel            = kernel;
    node->targetIndex       = kernel->targetIndex;

    /* Add the kernel ref from the node */
    vxoReference_Increment(&kernel->base, VX_REF_INTERNAL);

    node->kernelAttributes = kernel->attributes;

    graph->nodeTable[i]     = node;
    node->graph             = graph;

    /* Add the node ref from the graph */
    vxoReference_Increment(&node->base, VX_REF_INTERNAL);

    graph->nodeCount++;

    vxoPerf_Initialize(&graph->nodeTable[i]->perf);

    graph->verified = vx_false_e;

    vxReleaseMutex(graph->base.lock);

    vxoNode_Dump(node);

    return node;
}

VX_INTERNAL_API vx_node vxoNode_CreateSpecific(
        vx_graph graph, vx_enum kernelEnum, vx_reference parameters[], vx_uint32 paramCount)
{
    vx_kernel   kernel;
    vx_node     node;
    vx_uint32   index;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        vxError("The graph, %p, is invalid", graph);
        return VX_NULL;
    }

    kernel = vxoKernel_GetByEnum(graph->base.context, kernelEnum);

    if (vxoReference_GetStatus((vx_reference)kernel) != VX_SUCCESS) return (vx_node)kernel;

    node = vxoNode_CreateGeneric(graph, kernel);

    if (vxoReference_GetStatus((vx_reference)node) != VX_SUCCESS) return node;

    for (index = 0; index < paramCount; index++)
    {
        vx_status status = vxoNode_SetParameter(node, index, parameters[index]);

        if (status != VX_SUCCESS)
        {
            vxoNode_Release(&node);
            return (vx_node)vxoContext_GetErrorObject(graph->base.context, status);
        }
    }

    vxoKernel_ExternalRelease(&kernel);

    return node;
}

VX_INTERNAL_API void vxoNode_Dump(vx_node node)
{
    if (node == VX_NULL)
    {
        vxTrace(VX_TRACE_NODE, "<node>null</node>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)node);

        vxTrace(VX_TRACE_NODE,
                "<node>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <kernel>"VX_FORMAT_HEX"</kernel>\n"
                "</node>",
                node, node->kernel);
    }
}

VX_INTERNAL_CALLBACK_API void vxoNode_Destructor(vx_reference ref)
{
    vx_uint32 i;
    vx_node node = (vx_node)ref;

    vxmASSERT(vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE));
    vxmASSERT(node->kernel);

    if (node->kernel->deinitializeFunction != VX_NULL)
    {
        vx_status status = node->kernel->deinitializeFunction(
                                node, node->paramTable, node->kernel->signature.paramCount);

        if (status != VX_SUCCESS)
        {
            vxError("Deinitializing the kernel, \"%s\"(%p->%p), failed", node->kernel->name, node, node->kernel);
        }
    }

    for (i = 0; i < node->kernel->signature.paramCount; i++)
    {
        vx_reference param = node->paramTable[i];

        if (param == VX_NULL) continue;

        if (param->delay != VX_NULL)
        {
            if (!vxoParameterValue_UnbindFromDelay(param, node, i))
            {
                vxError("Fail to remove association to delay");
                vxmASSERT(0);
            }
        }

        /* Remove the ref count of the param from the node */
        vxoReference_Release(&param, param->type, VX_REF_INTERNAL);

        node->paramTable[i] = VX_NULL;
    }

    if (node->kernelAttributes.localDataPtr != VX_NULL)
    {
        vxFree(node->kernelAttributes.localDataPtr);
        node->kernelAttributes.localDataPtr = VX_NULL;
    }

    /* Remove the ref count of the kernel from the node */
    vxoReference_Release((vx_reference *)&node->kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
}

VX_INTERNAL_API void vxoNode_RemoveFromGraph(vx_node_ptr nodePtr)
{
    vx_node node;
    vx_graph graph;

    vxmASSERT(nodePtr);

    node = *nodePtr;

    if (node == VX_NULL) return;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) goto Exit;

    graph = node->graph;

    if (graph != VX_NULL)
    {
        vx_uint32 i;
        vx_bool removedFromGraph = vx_false_e;

        vxAcquireMutex(graph->base.lock);

        for (i = 0; i < graph->nodeCount; i++)
        {
            if (graph->nodeTable[i] == node)
            {
                graph->nodeTable[i] = graph->nodeTable[graph->nodeCount - 1];
                graph->nodeTable[graph->nodeCount -1] = VX_NULL;
                graph->nodeCount--;

                removedFromGraph = vx_true_e;

                graph->verified = vx_false_e;
                break;
            }
        }

        vxReleaseMutex(graph->base.lock);

        vxmASSERT(removedFromGraph);

        if (removedFromGraph)
        {
            /* Release the internal ref count of the node from the graph */
            vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_INTERNAL);
        }

        return;
    }

Exit:
    *nodePtr = VX_NULL;
}

VX_PRIVATE_API void vxoNode_Remove(vx_node *nodePtr)
{
    vx_node node;

    if (nodePtr == VX_NULL) return;

    node = *nodePtr;

    *nodePtr = VX_NULL;

    if (node == VX_NULL) return;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return;

    vxoNode_RemoveFromGraph(&node);

    vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_EXTERNAL);
}


VX_INTERNAL_API vx_status vxoNode_SetParameter(vx_node node, vx_uint32 index, vx_reference value)
{
    vx_type_e type;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    if (node->kernel == VX_NULL) return VX_ERROR_INVALID_NODE;

    if (index >= node->kernel->signature.paramCount || index >= VX_MAX_PARAMETERS)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (node->kernel->signature.stateTable[index] == VX_PARAMETER_STATE_OPTIONAL)
    {
        if (value == VX_NULL) return VX_SUCCESS;
    }
    else
    {
        if (value == VX_NULL) return VX_ERROR_INVALID_VALUE;
    }

    if (!vxoReference_IsValidAndNoncontext((vx_reference_s *)value)) return VX_ERROR_INVALID_VALUE;

    type = vxoReference_GetType(value);

    if (node->kernel->signature.dataTypeTable[index] != type)
    {
        if (type == VX_TYPE_SCALAR)
        {
            vx_enum dataType = vxoScalar_GetDataType((vx_scalar)value);

            if (node->kernel->signature.dataTypeTable[index] != dataType)
            {
                return VX_ERROR_INVALID_TYPE;
            }
        }
        else
        {
            return VX_ERROR_INVALID_TYPE;
        }
    }

    if (node->paramTable[index] != VX_NULL && node->paramTable[index]->delay != VX_NULL)
    {
        if (!vxoParameterValue_UnbindFromDelay(node->paramTable[index], node, index)) return VX_ERROR_INVALID_REFERENCE;
    }

    if (value->delay != VX_NULL)
    {
        if (!vxoParameterValue_BindToDelay(value, node, index)) return VX_ERROR_INVALID_REFERENCE;
    }

    if (node->paramTable[index] != VX_NULL)
    {
        vxoReference_Release(&node->paramTable[index], node->paramTable[index]->type, VX_REF_INTERNAL);
    }

    /* Add the internal ref count of value from the node */
    vxoReference_Increment(value, VX_REF_INTERNAL);

    node->paramTable[index] = value;

    if (node->childGraph != VX_NULL)
    {
        vx_uint32 i;

        for (i = 0; i < node->childGraph->paramCount; i++)
        {
            if (node->childGraph->paramTable[i].node == node
                && node->childGraph->paramTable[i].index == index)
            {
                vx_status status = vxoGraph_SetParameter(node->childGraph, i, value);

                if (status != VX_SUCCESS) return status;

                break;
            }
        }
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_parameter vxoNode_GetParameter(vx_node node, vx_uint32 index)
{
    vx_parameter parameter;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_NULL;

    if (node->kernel == VX_NULL)
    {
        return (vx_parameter)vxoContext_GetErrorObject(node->base.context, VX_ERROR_INVALID_NODE);
    }

    if (index >= VX_MAX_PARAMETERS || index >= node->kernel->signature.paramCount)
    {
        return (vx_parameter)vxoContext_GetErrorObject(node->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    parameter = (vx_parameter)vxoReference_Create(node->base.context, VX_TYPE_PARAMETER, VX_REF_EXTERNAL, &node->base);

    if (vxoReference_GetStatus((vx_reference)parameter) != VX_SUCCESS) return parameter;

    parameter->index    = index;

    parameter->node     = node;
     /* Add the ref count of the node from the parameter */
    vxoReference_Increment(&parameter->node->base, VX_REF_INTERNAL);

    parameter->kernel   = node->kernel;
    /* Add the ref count of the kernel from the parameter */
    vxoReference_Increment(&parameter->kernel->base, VX_REF_INTERNAL);

    return parameter;
}

VX_INTERNAL_API vx_status vxoNode_SetChildGraph(vx_node node, vx_graph graph)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_NODE;

    if (graph == VX_NULL)
    {
        if (node->childGraph != VX_NULL)
        {
            vxoReference_Release((vx_reference *)&node->childGraph, VX_TYPE_GRAPH, VX_REF_INTERNAL);
        }
    }
    else
    {
        vx_uint32 paramIndex;
        vx_signature signature1 = &node->kernel->signature;

        if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_ERROR_INVALID_GRAPH;

        if (graph->paramCount == 0)
        {
            vxError("The child graph %p has no parameter", graph);
            return VX_ERROR_INVALID_GRAPH;
        }

        /* Check signatures */
        for (paramIndex = 0; paramIndex < signature1->paramCount; paramIndex++)
        {
            vx_signature signature2;
            vx_uint32 graphParamIndex = graph->paramTable[paramIndex].index;

            if (graph->paramTable[paramIndex].node == VX_NULL)
            {
                vxError("No.%d parameter of the child graph %p refer to NULL node", paramIndex, graph);
                continue;
            }

            signature2 = &graph->paramTable[paramIndex].node->kernel->signature;

            if (signature1->directionTable[paramIndex]      != signature2->directionTable[graphParamIndex]
                || signature1->stateTable[paramIndex]       != signature2->stateTable[graphParamIndex]
                || signature1->dataTypeTable[paramIndex]    != signature2->dataTypeTable[graphParamIndex])
            {
                vxError("No.%d parameter of the child graph %p does not match the parameter of node %p",
                        paramIndex, graph, node);
                return VX_ERROR_INVALID_GRAPH;
            }
        }

        node->childGraph = graph;

        vxoReference_Increment(&graph->base, VX_REF_INTERNAL);
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_graph vxoNode_GetChildGraph(vx_node node)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_NULL;

    return node->childGraph;
}

VX_PRIVATE_API void vxoNode_SetVirtualAccessible(vx_node node, vx_bool accessible)
{
    vx_uint32 paramIndex;

    vxmASSERT(node);

    for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
    {
        vx_reference paramRef = node->paramTable[paramIndex];

        if (paramRef == VX_NULL) continue;

        if (paramRef->isVirtual) paramRef->accessible = accessible;
    }
}

VX_INTERNAL_API void vxoNode_EnableVirtualAccessible(vx_node node)
{
    vxoNode_SetVirtualAccessible(node, vx_true_e);
}

VX_INTERNAL_API void vxoNode_DisableVirtualAccessible(vx_node node)
{
    vxoNode_SetVirtualAccessible(node, vx_false_e);
}

VX_INTERNAL_API vx_status vxoNode_Record(vx_node node)
{
    gctPOINTER CmdBuffer = NULL;
    gctUINT32  CmdSizeBytes = 0;
    gceSTATUS gcStatus = gcvSTATUS_OK;

    if (!node->kernelAttributes.isAllGPU)
        return VX_ERROR_NOT_IMPLEMENTED;

    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, &CmdBuffer, &CmdSizeBytes);
    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;

    if (CmdSizeBytes > 0 && CmdBuffer)
    {
        if (node->cmdBuffer && (node->cmdSizeBytes < CmdSizeBytes))
        {
            free(node->cmdBuffer);
            node->cmdBuffer = NULL;
        }

        if (node->cmdBuffer == NULL)
        {
            node->cmdSizeBytes = (size_t) CmdSizeBytes;
            node->cmdBuffer = malloc(node->cmdSizeBytes);
        }

        memcpy(node->cmdBuffer, CmdBuffer, node->cmdSizeBytes);
    }

    /*gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, NULL, NULL);
    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;*/

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoNode_Replay(vx_node node)
{
    gceSTATUS gcStatus = gcvSTATUS_OK;

    if (!node->kernelAttributes.isAllGPU)
        return VX_ERROR_NOT_IMPLEMENTED;

    if ((node->cmdBuffer == NULL) || (node->cmdSizeBytes == 0))
        return VX_FAILURE;

    gcStatus = gcoVX_Replay((gctPOINTER)node->cmdBuffer, (gctUINT32)node->cmdSizeBytes);
    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;

    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, NULL, NULL);
    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;

    vxoPerf_Begin(&node->perf);
    node->executed = vx_true_e;
    node->status = VX_SUCCESS;
    vxoPerf_End(&node->perf);

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoNode_Release(vx_node_ptr nodePtr)
{
    if ((*nodePtr)->cmdBuffer)
    {
        free((*nodePtr)->cmdBuffer);
    }

    return vxoReference_Release((vx_reference_ptr)nodePtr, VX_TYPE_NODE, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_node vxCreateGenericNode(vx_graph graph, vx_kernel kernel)
{
    return vxoNode_CreateGeneric(graph, kernel);
}

VX_PUBLIC_API vx_status vxQueryNode(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_NODE_ATTRIBUTE_PERFORMANCE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_perf_t, 0x3);

            vxoPerf_Dump(&node->perf);

            *(vx_perf_t *)ptr = node->perf;
            break;

        case VX_NODE_ATTRIBUTE_STATUS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_status, 0x3);

            *(vx_status *)ptr = node->status;
            break;

        case VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = node->kernelAttributes.localDataSize;
            break;

        case VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            *(vx_ptr *)ptr = node->kernelAttributes.localDataPtr;
            break;

#ifdef OPENVX_KHR_NODE_MEMORY
        case VX_NODE_ATTRIBUTE_GLOBAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = node->kernelAttributes.globalDataSize;
            break;

        case VX_NODE_ATTRIBUTE_GLOBAL_DATA_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            *(vx_ptr *)ptr = node->kernelAttributes.globalDataPtr;
            break;
#endif

        case VX_NODE_ATTRIBUTE_BORDER_MODE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_mode_t, 0x3);

            *(vx_border_mode_t *)ptr = node->kernelAttributes.borderMode;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_NODE_ATTRIBUTE_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            *(vx_neighborhood_size_t *)ptr = node->kernel->attributes.inputNeighborhoodSize;
            break;

        case VX_NODE_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            *(vx_tile_block_size_t *)ptr = node->kernel->attributes.tileBlockSize;
            break;

        case VX_NODE_ATTRIBUTE_TILE_MEMORY_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = node->kernelAttributes.tileMemorySize;
            break;

        case VX_NODE_ATTRIBUTE_TILE_MEMORY_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            *(vx_ptr *)ptr = node->kernelAttributes.tileMemoryPtr;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxSetNodeAttribute(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (node->graph->verified == vx_true_e)
    {
        return VX_ERROR_NOT_SUPPORTED;
    }

    switch (attribute)
    {
        case VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            node->kernelAttributes.localDataSize = *(vx_size *)ptr;
            break;

        case VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            node->kernelAttributes.localDataPtr = *(vx_ptr *)ptr;
            break;

        case VX_NODE_ATTRIBUTE_BORDER_MODE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_mode_t, 0x3);

#ifdef OPENVX_KHR_TILING
            if (node->kernelAttributes.borderMode.mode == VX_BORDER_MODE_SELF)
            {
                return VX_ERROR_INVALID_VALUE;
            }
#endif

            node->kernelAttributes.borderMode = *(vx_border_mode_t *)ptr;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxReleaseNode(vx_node *node)
{
    return vxoNode_Release(node);
}

VX_PUBLIC_API void vxRemoveNode(vx_node *node)
{
    vxoNode_Remove(node);
}

VX_PUBLIC_API vx_status vxAssignNodeCallback(vx_node node, vx_nodecomplete_f callback)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    if (callback != VX_NULL && node->completeCallback != VX_NULL)
    {
        vxError("Can't re-assign the existing callback, %p, of the node, %p, to the new one, %p",
                node->completeCallback, node, callback);
        return VX_ERROR_NOT_SUPPORTED;
    }

    node->completeCallback = callback;
    return VX_SUCCESS;
}

VX_PUBLIC_API vx_nodecomplete_f vxRetrieveNodeCallback(vx_node node)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_NULL;

    return node->completeCallback;
}

#if defined(OPENVX_USE_VARIANTS)
VX_PUBLIC_API vx_status vxChooseKernelVariant(vx_node node, vx_char variantName[VX_MAX_VARIANT_NAME])
{
    vx_target   target;
    vx_string   kernelName;
    vx_char_ptr colonCharPtr;
    vx_status   status;
    vx_uint32   kernelIndex;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    target = &node->base.context->targetTable[node->targetIndex];

    gcoOS_StrDup(gcvNULL, node->kernel->name, &kernelName);

    colonCharPtr    = strchr(kernelName, ':');

    if (colonCharPtr != VX_NULL) *colonCharPtr = '\0';

    status = target->funcs.iskernelsupported(target, target->name, kernelName, variantName, OUT &kernelIndex);

    free(kernelName);

    if (status != VX_SUCCESS) return status;

    vxoReference_Decrement(&node->kernel->base, VX_REF_INTERNAL);

    node->kernel = &target->kernelTable[kernelIndex];

    vxoReference_Increment(&node->kernel->base, VX_REF_INTERNAL);

    return VX_SUCCESS;
}
#endif
