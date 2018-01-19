/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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

    node->cnnTriggerEventID = 0;
    node->cnnWaitEventID0    = 0xffffffff;
    node->cnnWaitEventID1    = 0xffffffff;

    /* Add the node ref from the graph */
    vxoReference_Increment(&node->base, VX_REF_INTERNAL);

    graph->nodeCount++;

    vxoPerf_Initialize(&graph->nodeTable[i]->perf);

    graph->reverify = graph->verified;

    graph->verified = vx_false_e;

    graph->status = VX_GRAPH_STATE_UNVERIFIED;

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
        vx_status status;

        if (node->localDataSetByImplementation == vx_false_e)
           node->localDataChangeIsEnabled = vx_true_e;

        status = node->kernel->deinitializeFunction(
                                node, node->paramTable, node->kernel->signature.paramCount);

        node->localDataChangeIsEnabled = vx_false_e;

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

#if !gcdVX_OPTIMIZER
    if (node->kernelContext)
    {
        gcoVX_Kernel_Context * kernelContext = (gcoVX_Kernel_Context *)(node->kernelContext);

        for (i = 0; i < GC_VX_MAX_HARDWARE_CONTEXT; i++)
        {
            if (kernelContext->hwContext[i])
            {
                if (kernelContext->hwContext[i]->node && kernelContext->hwContext[i]->node->pool != gcvPOOL_UNKNOWN)
                {
                    gcoVX_DestroyInstruction(kernelContext->hwContext[i]->node);
                    kernelContext->hwContext[i]->node = gcvNULL;
                }
                vxFree(kernelContext->hwContext[i]);
                kernelContext->hwContext[i] = gcvNULL;
            }
        }

        vxFree(node->kernelContext);
        node->kernelContext = gcvNULL;
    }
#endif

    if (node->cmdBuffer)
    {
#if VX_C_MEMORY_MANAGE
        vxoMemory_CFree(node->base.context, (void**)&(node->cmdBuffer));
#else
        free(node->cmdBuffer);
#endif
    }

    /* Remove the ref count of the kernel from the node */
    vxoReference_Release((vx_reference *)&node->kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);

    if (node->uniforms)
    {
        for(i = 0 ; i < node->uniformCount; i++)
        {
            gcoOS_Free(gcvNULL, node->uniforms[i].data);
        }

        gcoOS_Free(gcvNULL, node->uniforms);
    }
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

                graph->reverify = graph->verified;

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

VX_PRIVATE_API vx_status vxoNode_Remove(vx_node *nodePtr)
{
    vx_status status = VX_FAILURE;
    vx_node node;

    if (nodePtr == VX_NULL) return status;

    node = *nodePtr;

    *nodePtr = VX_NULL;

    if (node == VX_NULL) return status;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return status;

    vxoNode_RemoveFromGraph(&node);

    status = vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_EXTERNAL);

    return status;
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
        if (value == VX_NULL) return VX_ERROR_INVALID_REFERENCE;
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
#if gcdVX_OPTIMIZER
            node->childGraph->isSubGraph = vx_false_e;
#endif
            node->childGraph->isChildGraph = vx_false_e;
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

#if gcdVX_OPTIMIZER
        vxmASSERT(!graph->isSubGraph);
        graph->isSubGraph = vx_true_e;
#endif
        /*the flag isChildGraph is the same with isSubGraph. add it because isSubGraph only use in optimize mode.*/
        vxmASSERT(!graph->isChildGraph);
        graph->isChildGraph = vx_true_e;

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
    gcoHARDWARE savedHardware;

    if (!node->kernelAttributes.isAllGPU)
        return VX_ERROR_NOT_IMPLEMENTED;

    gcoVX_SaveContext(&savedHardware);

    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, &CmdBuffer, &CmdSizeBytes);

    gcoVX_RestoreContext(savedHardware);

    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;

    if (CmdSizeBytes > 0 && CmdBuffer)
    {
        if (node->cmdBuffer && (node->cmdSizeBytes < CmdSizeBytes))
        {
#if VX_C_MEMORY_MANAGE
            vxoMemory_CFree(node->base.context, (void**)&node->cmdBuffer);
#else
            free(node->cmdBuffer);
#endif
            node->cmdBuffer = NULL;
        }

        if (node->cmdBuffer == NULL)
        {
            node->cmdSizeBytes = (size_t) CmdSizeBytes;
#if VX_C_MEMORY_MANAGE
            vxoMemory_CAllocate(node->base.context, (void**)&node->cmdBuffer, (vx_uint32)node->cmdSizeBytes);
#else
            node->cmdBuffer = malloc(node->cmdSizeBytes);
#endif
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
    gcoHARDWARE savedHardware;

    if (!node->kernelAttributes.isAllGPU)

        return VX_ERROR_NOT_IMPLEMENTED;

    if ((node->cmdBuffer == NULL) || (node->cmdSizeBytes == 0))
        return VX_FAILURE;

    vxoPerf_Begin(&node->perf);

    gcoVX_SaveContext(&savedHardware);

    gcStatus = gcoVX_Replay((gctPOINTER)node->cmdBuffer, (gctUINT32)node->cmdSizeBytes);

    if (gcStatus != gcvSTATUS_OK)
    {
        gcoVX_RestoreContext(savedHardware);
        return VX_FAILURE;
    }

    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, NULL, NULL);

    gcoVX_RestoreContext(savedHardware);

    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;

#if gcdDUMP
    vxoDumpOutput(node, node->paramTable, node->kernel->signature.paramCount);
#endif

    node->executed = vx_true_e;
    node->status = VX_SUCCESS;
    vxoPerf_End(&node->perf);

    return VX_SUCCESS;
}



VX_INTERNAL_API vx_status vxoNode_Release(vx_node_ptr nodePtr)
{
    if ((*nodePtr)->cmdBuffer)
    {
#if VX_C_MEMORY_MANAGE
        vxoMemory_CFree((*nodePtr)->base.context, (void**)&(*nodePtr)->cmdBuffer);
#else
        free((*nodePtr)->cmdBuffer);
#endif
    }

#if gcdVX_OPTIMIZER
    if ((*nodePtr)->kernelContext)
    {
        vxFree((*nodePtr)->kernelContext);
    }
#else
    if ((*nodePtr)->kernelContext)
    {
        vx_int32 i = 0;
        gcoVX_Kernel_Context * kernelContext = (gcoVX_Kernel_Context *)((*nodePtr)->kernelContext);

        for (i = 0; i < GC_VX_MAX_HARDWARE_CONTEXT; i++)
        {
            if (kernelContext->hwContext[i])
            {
                if (kernelContext->hwContext[i]->node && kernelContext->hwContext[i]->node->pool != gcvPOOL_UNKNOWN)
                {
                    gcoVX_DestroyInstruction(kernelContext->hwContext[i]->node);
                    kernelContext->hwContext[i]->node = gcvNULL;
                }
                vxFree(kernelContext->hwContext[i]);
                kernelContext->hwContext[i] = gcvNULL;
            }
        }

        vxFree((*nodePtr)->kernelContext);
        (*nodePtr)->kernelContext = gcvNULL;
    }
#endif

    return vxoReference_Release((vx_reference_ptr)nodePtr, VX_TYPE_NODE, VX_REF_EXTERNAL);
}


VX_INTERNAL_API vx_status vxoNode_GetTriggerCNNEventID(vx_node node, vx_uint32 * eventID)
{
    /*TODO: need to manage the eventID  */
    if (node->base.context->cnnAvailableEventID == 32)
    {
        node->base.context->cnnAvailableEventID = 1;
    }

    *eventID = node->base.context->cnnAvailableEventID++;

    node->cnnTriggerEventID = *eventID;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoNode_SetWaitCNNEventID0(vx_node node, vx_uint32 eventID)
{
    node->cnnWaitEventID0 = eventID;
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoNode_SetWaitCNNEventID1(vx_node node, vx_uint32 eventID)
{
    node->cnnWaitEventID1 = eventID;
    return VX_SUCCESS;
}

VX_API_ENTRY vx_node VX_API_CALL vxCreateGenericNode(vx_graph graph, vx_kernel kernel)
{
    return vxoNode_CreateGeneric(graph, kernel);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryNode(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_NODE_PERFORMANCE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_perf_t, 0x3);

            vxoPerf_Dump(&node->perf);

            *(vx_perf_t *)ptr = node->perf;
            break;

        case VX_NODE_STATUS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_status, 0x3);

            *(vx_status *)ptr = node->status;
            break;

        case VX_NODE_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = node->kernelAttributes.localDataSize;
            break;

        case VX_NODE_LOCAL_DATA_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            *(vx_ptr *)ptr = node->kernelAttributes.localDataPtr;
            break;


        case VX_NODE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            *(vx_border_t *)ptr = node->kernelAttributes.borderMode;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_NODE_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            *(vx_tile_block_size_t *)ptr = node->kernel->attributes.tileBlockSize;
            break;

        case VX_NODE_TILE_MEMORY_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = node->kernelAttributes.tileMemorySize;
            break;

        case VX_NODE_TILE_MEMORY_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            *(vx_ptr *)ptr = node->kernelAttributes.tileMemoryPtr;
            break;
#endif
        case VX_NODE_PARAMETERS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            memcpy((vx_uint32*)ptr, &node->kernel->signature.paramCount, sizeof(vx_uint32));

            break;

        case VX_NODE_IS_REPLICATED:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_bool, 0x3);
            {
                vx_bool isReplicated = node->isReplicated;
                if (vx_true_e == isReplicated)
                    vxError("Node is replicated\n");
                else
                    vxError("Number is not replicated\n");

                memcpy((vx_bool*)ptr, &isReplicated, sizeof(isReplicated));
            }
            break;

        case VX_NODE_REPLICATE_FLAGS:
            {
                vx_size sz = sizeof(vx_bool)*node->kernel->signature.paramCount;
                if (size == sz && ((vx_size)ptr & 0x3) == 0)
                {
                    vx_uint32 i = 0;
                    vx_uint32 numParams = node->kernel->signature.paramCount;
                    for (i = 0; i < numParams; i++)
                        ((vx_bool*)ptr)[i] = node->replicated_flags[i];
                }
            }
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeAttribute(vx_node node, vx_enum attribute, const void *ptr, vx_size size)
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
        case VX_NODE_LOCAL_DATA_SIZE:

            if (!node->localDataChangeIsEnabled)
                return VX_ERROR_NOT_SUPPORTED;

            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            node->kernelAttributes.localDataSize = *(vx_size *)ptr;

            node->localDataSetByImplementation = vx_false_e;
            break;

        case VX_NODE_LOCAL_DATA_PTR:

            if (!node->localDataChangeIsEnabled)
                return VX_ERROR_NOT_SUPPORTED;

            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            node->kernelAttributes.localDataPtr = *(vx_ptr *)ptr;

            node->localDataSetByImplementation = vx_false_e;

            break;

        case VX_NODE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

#ifdef OPENVX_KHR_TILING
            if (node->kernelAttributes.borderMode.mode == VX_BORDER_SELF)
            {
                return VX_ERROR_INVALID_VALUE;
            }
#endif

            node->kernelAttributes.borderMode = *(vx_border_t *)ptr;
            break;

        case VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_kernel_execution_parameters_t, 0x3);

            node->kernelAttributes.shaderParameter = *(vx_kernel_execution_parameters_t *)ptr;
            break;


        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxReplicateNode(vx_graph graph, vx_node first_node, vx_bool replicate[], vx_uint32 number_of_parameters)
{
    vx_uint32 n;
    vx_uint32 p;
    vx_uint32 numParams = 0;
    vx_size   num_of_replicas = 0;
    vx_status status = VX_SUCCESS;

    if (vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH) != vx_true_e)
    {
        vxError("Graph %p was invalid!\n", graph);
        vxAddLogEntry((vx_reference)graph, VX_ERROR_INVALID_REFERENCE, "Graph %p as invalid!\n", graph);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (vxoReference_IsValidAndSpecific(&first_node->base, VX_TYPE_NODE) != vx_true_e)
    {
        vxError("Node %p was invalid!\n", first_node);
        vxAddLogEntry((vx_reference)first_node, VX_ERROR_INVALID_REFERENCE, "Node %p as invalid!\n", first_node);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (first_node->graph != graph)
        return VX_FAILURE;

    if (replicate == NULL)
        return VX_ERROR_INVALID_PARAMETERS;

    /* validate replicated params */
    status = vxQueryNode(first_node, VX_NODE_PARAMETERS, &numParams, sizeof(numParams));
    if (VX_SUCCESS == status)
    {
        if (numParams != number_of_parameters)
            return VX_ERROR_INVALID_PARAMETERS;
    }
    else
        return status;

    for (p = 0; p < number_of_parameters; p++)
    {
        vx_parameter param = 0;
        vx_reference ref = 0;
        vx_enum type = 0;
        vx_enum state = 0;
        vx_enum dir = 0;

        param = vxGetParameterByIndex(first_node, p);

        vxQueryParameter(param, VX_PARAMETER_TYPE, &type, sizeof(vx_enum));
        vxQueryParameter(param, VX_PARAMETER_REF, &ref, sizeof(vx_reference));
        vxQueryParameter(param, VX_PARAMETER_STATE, &state, sizeof(vx_enum));
        vxQueryParameter(param, VX_PARAMETER_DIRECTION, &dir, sizeof(vx_enum));


        if (replicate[p] == vx_false_e && (dir == VX_OUTPUT || dir == VX_BIDIRECTIONAL))
            return VX_FAILURE;

        if (replicate[p] == vx_true_e)
        {
            if (vxoReference_IsValidAndSpecific(ref, (vx_type_e)type) == vx_true_e)
            {
                vx_size levels = 0;

                if (vxoReference_IsValidAndSpecific(ref->scope, VX_TYPE_PYRAMID) == vx_true_e)
                {
                    vx_pyramid pyramid = (vx_pyramid)ref->scope;
                    vxQueryPyramid(pyramid, VX_PYRAMID_LEVELS, &levels, sizeof(vx_size));
                }
                else if (vxoReference_IsValidAndSpecific(ref->scope, VX_TYPE_OBJECT_ARRAY) == vx_true_e)
                {
                    vx_object_array object_array = (vx_object_array)ref->scope;
                    vxQueryObjectArray(object_array, VX_OBJECT_ARRAY_NUMITEMS, &levels, sizeof(vx_size));
                }
                else
                    return VX_FAILURE;

                if (num_of_replicas == 0)
                    num_of_replicas = levels;

                if (num_of_replicas != 0 && levels != num_of_replicas)
                    return VX_FAILURE;
            }
            else
                return VX_FAILURE;
        }

        vxReleaseReference(&ref);
        vxReleaseParameter(&param);
    }

    /* set replicate flag for node */
    first_node->isReplicated = vx_true_e;

    for (n = 0; n < number_of_parameters; n++)
    {
        first_node->replicated_flags[n] = replicate[n];
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseNode(vx_node *node)
{
    return vxoNode_Release(node);
}

VX_API_ENTRY vx_status  VX_API_CALL vxRemoveNode(vx_node *node)
{
    return vxoNode_Remove(node);
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeCallback(vx_node node, vx_nodecomplete_f callback)
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

VX_API_ENTRY vx_nodecomplete_f VX_API_CALL vxRetrieveNodeCallback(vx_node node)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_NULL;

    return node->completeCallback;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetChildGraphOfNode(vx_node node, vx_graph graph)
{
    return vxoNode_SetChildGraph(node, graph);
}

VX_API_ENTRY vx_graph VX_API_CALL vxGetChildGraphOfNode(vx_node node)
{
    return vxoNode_GetChildGraph(node);
}

#if defined(OPENVX_USE_VARIANTS)
VX_API_ENTRY vx_status VX_API_CALL vxChooseKernelVariant(vx_node node, vx_char variantName[VX_MAX_VARIANT_NAME])
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

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeTarget(vx_node node, vx_enum target_enum, const char* target_string)
{
    vx_context context;
    vx_kernel_s *kernel = VX_NULL;
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_uint32 index, targetIndex = 0;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_NODE;

    context = vxGetContext((vx_reference)node);

    for (index = 0; index < context->targetCount; index++)
    {
        if (target_enum == VX_TARGET_ANY)
        {
            targetIndex = index;
            kernel = vxoKernel_GetByEnumFromTarget(context, &context->targetTable[index], index, node->kernel->enumeration);
        }
        else if (target_enum == VX_TARGET_STRING)
        {
            targetIndex = context->targetPriorityTable[index];
            if (vxoTarget_MatchTargetNameWithString(context->targetTable[targetIndex].name, target_string) == vx_true_e)
            {
                kernel = vxoKernel_GetByEnumFromTarget(context, &context->targetTable[targetIndex], targetIndex, node->kernel->enumeration);
            }
        }
        else
        {
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        }

        if (kernel == VX_NULL) continue;
    }

    if (kernel != NULL)
    {
        vxoReference_Decrement(&node->kernel->base, VX_REF_INTERNAL);
        node->kernel = kernel;
        vxoReference_Increment(&node->kernel->base, VX_REF_INTERNAL);

        node->targetIndex = targetIndex;

        node->graph->reverify = node->graph->verified;

        node->graph->verified = vx_false_e;
        status = VX_SUCCESS;
    }
    else
    {
        status = VX_ERROR_NOT_SUPPORTED;
    }

    return status;
}

