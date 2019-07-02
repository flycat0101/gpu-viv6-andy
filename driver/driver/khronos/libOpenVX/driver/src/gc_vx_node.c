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
#include <gc_hal_vx.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_NODE

VX_PRIVATE_API vx_node vxoNode_CreateGeneric(vx_graph graph, vx_kernel kernel)
{
    vx_uint32 i;
    vx_node node;
    static vx_uint32 nodeID = 0;

    gcmHEADER_ARG("graph=%p, kernel=%p", graph, kernel);
    vxmASSERT(graph);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        vxError("The graph, %p, is invalid", graph);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL))
    {
        vxError("The kernel, %p, is invalid", kernel);
        gcmFOOTER_NO();
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
        gcmFOOTER_NO();
        return (vx_node)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_NO_RESOURCES);
    }

    node = (vx_node)vxoReference_Create(graph->base.context, VX_TYPE_NODE, VX_REF_EXTERNAL, &graph->base);

    if (vxoReference_GetStatus((vx_reference)node) != VX_SUCCESS)
    {
        vxReleaseMutex(graph->base.lock);
        gcmFOOTER_NO();
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

    node->tensorVxcOptimize = vx_false_e;

    /* Add the node ref from the graph */
    vxoReference_Increment(&node->base, VX_REF_INTERNAL);

    node->nodeID = nodeID++;

    graph->nodeCount++;

    vxoPerf_Initialize(&graph->nodeTable[i]->perf);

    graph->reverify = graph->verified;

    graph->verified = vx_false_e;

    graph->status = VX_GRAPH_STATE_UNVERIFIED;

    vxReleaseMutex(graph->base.lock);

    vxoNode_Dump(node);
    gcmFOOTER_NO();
    return node;
}

VX_INTERNAL_API vx_node vxoNode_CreateSpecific(
        vx_graph graph, vx_enum kernelEnum, vx_reference parameters[], vx_uint32 paramCount)
{
    vx_kernel   kernel;
    vx_node     node;
    vx_uint32   index;

    gcmHEADER_ARG("graph=%p, kernelEnum=0x%x, parameters=%p, paramCount=0x%x", graph, kernelEnum, parameters, paramCount);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        vxError("The graph, %p, is invalid", graph);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    kernel = vxoKernel_GetByEnum(graph->base.context, kernelEnum);

    if (vxoReference_GetStatus((vx_reference)kernel) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)kernel;
    }
    node = vxoNode_CreateGeneric(graph, kernel);

    node->numParameters = paramCount;

    if (vxoReference_GetStatus((vx_reference)node) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return node;
    }
    for (index = 0; index < paramCount; index++)
    {
        vx_status status = vxoNode_SetParameter(node, index, parameters[index]);

        if (status != VX_SUCCESS)
        {
            vxoNode_Release(&node);
            gcmFOOTER_NO();
            return (vx_node)vxoContext_GetErrorObject(graph->base.context, status);
        }
    }

    vxoKernel_ExternalRelease(&kernel);
    gcmFOOTER_NO();
    return node;
}

VX_INTERNAL_API vx_bool vxoNode_CheckF32Support(vx_node node)
{
    return
        !(node->base.context->nnConfig.fixedFeature.nnCoreCount > 0 ||
            node->base.context->nnConfig.fixedFeature.tpCoreCount > 0 ||
            node->base.context->evisNoInst.supportEVIS);
}

VX_INTERNAL_API vx_status vxoNode_ConvertDims(vx_uint32_ptr dims, vx_uint32_ptr org_dims, vx_uint32 count, vx_bool dimsto4)
{
     /* Convert dims from CWHN(NHWC) => WHCN */
    switch (count)
    {
    case 4:
        dims[0] = org_dims[2]; /* W : */
        dims[1] = org_dims[1]; /* H : */
        dims[2] = org_dims[3]; /* C : */
        dims[3] = org_dims[0]; /* N : */
        break;
    case 3:
        dims[0] = org_dims[2]; /* W : */
        dims[1] = org_dims[1]; /* H : */
        dims[2] = org_dims[0]; /* C : */
        dims[3] = 1;           /* N : */
        break;
    case 2:
        if (dimsto4)
        {
            dims[0] = 1;            /* S : */
            dims[1] = 1;            /* N : */
            dims[2] = org_dims[1];  /* C : */
            dims[3] = org_dims[0];  /* N : */
        }
        else
        {
            dims[0] = org_dims[1]; /* S : */
            dims[1] = org_dims[0]; /* N : */
            dims[2] = 1;           /* C : */
            dims[3] = 1;           /* N : */
        }
        break;
    default:
        break;
    }

    return VX_SUCCESS;
}

extern vx_status vxnneAdapter_Tensor_CWHN2WHCN(vx_tensor inputs, vx_tensor outputs);
extern vx_status vxnneAdapter_Tensor_FormatConvert(vx_tensor inputs, vx_tensor outputs);

VX_INTERNAL_API vx_bool vxoNode_Adapter(vx_graph graph, vx_node node, vx_uint32 index)
{
    vx_bool opt = vx_false_e;
    vx_int32 i = 0, max = 1;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_reference ref = node->paramTable[index];
    vx_tensor tensor = VX_NULL;
    gcmHEADER_ARG("graph=%p, node=%p, index=0x%x", graph, node, index);

    if (ref == VX_NULL || node->kernel->enumeration == VX_KERNEL_INTERNAL_ADAPTER)
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }

    if (ref->type == VX_TYPE_OBJECT_ARRAY)
    {
        max = (vx_int32)((vx_object_array)ref)->itemCount;
    }
    else if (ref->type == VX_TYPE_TENSOR)
    {
        tensor = (vx_tensor)ref;
    }

    for (i = 0; i < max; i++)
    {
        if (ref->type == VX_TYPE_OBJECT_ARRAY)
            tensor = (vx_tensor)((vx_object_array)ref)->itemsTable[i];

        if (tensor != VX_NULL)
        {
            vx_enum format = TENSOR_DATA_TYPE(tensor);

           vx_bool only_support_f32 = vxoNode_CheckF32Support(node);

            if ((TENSOR_RANK(tensor) == VX_TENSOR_RANK_CWHN) || ((format == VX_TYPE_FLOAT32) && !only_support_f32))
            {
                vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION] = { 0 };
                vx_enum rank = VX_TENSOR_RANK_WHCN;

                vx_tensor_create_params_t param = { tensor->dimCount, tensor->dims, VX_TYPE_FLOAT32};
                vx_tensor virt_tensor = VX_NULL;
                vx_enum type = VX_ADAPTER_F32_TO_F16;
                vx_node convert = VX_NULL;
                vx_reference parameters[] = {
                    VX_NULL,
                    VX_NULL,
                    VX_NULL
                };
                vx_bool instatic = node->kernel->signature.isStaticTable[index] ||
                                   TENSOR_DATA_LIFETIME(tensor) == VX_TENSOR_LIFE_TIME_STATIC ? vx_true_e : vx_false_e;

                if(!vxoNode_CheckF32Support(node))
                    param.data_format = VX_TYPE_FLOAT16;

                if (format == VX_TYPE_UINT8)
                {
                    param.quant_format = TENSOR_QUANT_TYPE(tensor);
                    param.quant_data.affine.scale = TENSOR_TF_SCALE(tensor);
                    param.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(tensor);

                    param.data_format = VX_TYPE_UINT8;
                }
                /*param.quant_data.dfp.fixed_point_pos = TENSOR_POS(tensor);*/

                if (node->kernel->signature.directionTable[index] == VX_INPUT)
                {
                    if ((TENSOR_RANK(tensor) == VX_TENSOR_RANK_CWHN) && (tensor->dimCount == 4))
                    {
                        vxoNode_ConvertDims(dims, tensor->dims, 4/*tensor->dimCount*/, vx_false_e);

                        param.sizes = dims;
                        param.num_of_dims = 4;

                        if (TENSOR_PRECISION(tensor) == VX_TENSOR_PRECISION_HIGH)
                            param.data_format = TENSOR_DATA_TYPE(tensor);

                        type = VX_ADAPTER_CWHN_TO_WHCN;

                        virt_tensor = instatic ? vxoTensor_CreateTensor2(context, &param, sizeof(vx_tensor_create_params_t)) :
                                                 vxoTensor_CreateVirtualTensor2(graph, &param, sizeof(vx_tensor_create_params_t));

                        vxSetTensorAttribute(virt_tensor, VX_TENSOR_RANK, &rank, sizeof(vx_enum));
                    }
                    else if ((format == VX_TYPE_FLOAT32) && (TENSOR_PRECISION(tensor) == VX_TENSOR_PRECISION_AUTO) && !only_support_f32)
                    {
                        type = VX_ADAPTER_F32_TO_F16;

                        virt_tensor = instatic ? vxoTensor_CreateTensor2(context, &param, sizeof(vx_tensor_create_params_t)) :
                                                 vxoTensor_CreateVirtualTensor2(graph, &param, sizeof(vx_tensor_create_params_t));

                        vxSetTensorAttribute(virt_tensor, VX_TENSOR_RANK, &TENSOR_RANK(tensor), sizeof(vx_enum));
                    }
                    else
                    {
                        gcmFOOTER_NO();
                        return vx_false_e;
                    }
                    vxSetTensorAttribute(virt_tensor, VX_TENSOR_LIFETIME, &TENSOR_DATA_LIFETIME(tensor), sizeof(vx_enum));


                    parameters[0] = (vx_reference)tensor;
                    parameters[2] = (vx_reference)virt_tensor;
                }
                else if (node->kernel->signature.directionTable[index] == VX_OUTPUT)
                {
                    if ((TENSOR_RANK(tensor) == VX_TENSOR_RANK_CWHN) && (tensor->dimCount == 4))
                    {
                        vxoNode_ConvertDims(dims, tensor->dims, 4, vx_false_e);

                        param.sizes = dims;
                        param.num_of_dims = 4;

                        if (TENSOR_PRECISION(tensor) == VX_TENSOR_PRECISION_HIGH)
                            param.data_format = TENSOR_DATA_TYPE(tensor);

                        type = VX_ADAPTER_WHCN_TO_CWHN;

                        virt_tensor = vxoTensor_CreateVirtualTensor2(graph, &param, sizeof(vx_tensor_create_params_t));

                        vxSetTensorAttribute(virt_tensor, VX_TENSOR_RANK, &rank, sizeof(vx_enum));
                    }
                    else if (format == VX_TYPE_FLOAT32 && (TENSOR_PRECISION(tensor) == VX_TENSOR_PRECISION_AUTO) && !only_support_f32)
                    {
                        type = VX_ADAPTER_F16_TO_F32;
                        virt_tensor = vxoTensor_CreateVirtualTensor2(graph, &param, sizeof(vx_tensor_create_params_t));

                        vxSetTensorAttribute(virt_tensor, VX_TENSOR_RANK, &TENSOR_RANK(tensor), sizeof(vx_enum));
                    }
                    else{
                        gcmFOOTER_NO();
                        return vx_false_e;
                    }
                    vxSetTensorAttribute(virt_tensor, VX_TENSOR_LIFETIME, &TENSOR_DATA_LIFETIME(tensor), sizeof(vx_enum));

                    parameters[0] = (vx_reference)virt_tensor;
                    parameters[2] = (vx_reference)tensor;
                }
                else
                    vxError("Not support VX_BIDIRECTIONAL !");

                parameters[1] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &type);

                if (node->kernel->signature.directionTable[index] == VX_INPUT && instatic)
                {
                    vxoTensor_AllocateMemory(virt_tensor);

                    if (type == VX_ADAPTER_CWHN_TO_WHCN)
                    {
                        vxnneAdapter_Tensor_CWHN2WHCN((vx_tensor)parameters[0], (vx_tensor)parameters[2]);
                    }
                    else if (type == VX_ADAPTER_F32_TO_F16)
                    {
                        vxnneAdapter_Tensor_FormatConvert((vx_tensor)parameters[0], (vx_tensor)parameters[2]);
                    }
                    else
                        vxError("[%d] Static Tensor but not operation! ", __LINE__);
                }
                else
                {
                    convert = vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ADAPTER, parameters, vxmLENGTH_OF(parameters));
                    if (vxoReference_GetStatus((vx_reference)convert) != VX_SUCCESS)
                    {
                        gcmFOOTER_NO();
                        return vx_false_e;
                    }
                }

                if(virt_tensor != VX_NULL)
                    vxSetTensorAttribute(virt_tensor, VX_TENSOR_VALUE, &TENSOR_VALUED(tensor), sizeof(vx_bool));

                if (ref->type == VX_TYPE_OBJECT_ARRAY)
                {
                    ((vx_object_array)ref)->itemsTable[i] = (vx_reference)virt_tensor;
                    vxoReference_Increment(&virt_tensor->base, VX_REF_INTERNAL);
                }
                else
                    vxoNode_SetParameter(node, index, (vx_reference)virt_tensor);

                if(virt_tensor != VX_NULL)
                    vxoTensor_ReleaseTensor(&virt_tensor);

                vxReleaseScalar((vx_scalar*)&parameters[1]);
                opt = vx_true_e;
            }
        }
    }
    gcmFOOTER_NO();
    return opt;
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
    vx_status status;
    gcmHEADER_ARG("ref=%p", ref);
    vxmASSERT(vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE));
    vxmASSERT(node->kernel);

    /* Wrapped user node need to deinitialize layer/op first */
    if (node->kernel->deinitializeWrapFunction != VX_NULL)
    {
        status = node->kernel->deinitializeWrapFunction(
                                node, node->paramTable, node->kernel->signature.paramCount);

        if (status != VX_SUCCESS)
        {
            vxError("Deinitializing the kernel, \"%s\"(%p->%p), failed", node->kernel->name, node, node->kernel);
        }
    }

    if (node->kernel->deinitializeFunction != VX_NULL)
    {
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
                    gcoVX_FreeMemoryEx(kernelContext->hwContext[i]->node, gcvSURF_ICACHE);
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
        vxFree(node->cmdBuffer);
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
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoNode_RemoveFromGraph(vx_node_ptr nodePtr)
{
    vx_node node;
    vx_graph graph;
    gcmHEADER_ARG("nodePtr=%p", nodePtr);

    vxmASSERT(nodePtr);

    node = *nodePtr;

    if (node == VX_NULL)
    {
        gcmFOOTER_NO();
        return;
    }
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
                vx_uint32 j;
                for (j = i; j < graph->nodeCount - 1; j++)
                {
                    graph->nodeTable[j] = graph->nodeTable[j + 1];
                }
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
        gcmFOOTER_NO();
        return;
    }

Exit:
    *nodePtr = VX_NULL;
}

VX_PRIVATE_API vx_status vxoNode_Remove(vx_node *nodePtr)
{
    vx_status status = VX_FAILURE;
    vx_node node;

    gcmHEADER_ARG("nodePtr=%p", nodePtr);

    if (nodePtr == VX_NULL) {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    node = *nodePtr;

    *nodePtr = VX_NULL;

    if (node == VX_NULL)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    vxoNode_RemoveFromGraph(&node);

    status = vxoReference_Release((vx_reference_ptr)&node, VX_TYPE_NODE, VX_REF_EXTERNAL);

    gcmFOOTER_ARG("%d", status);
    return status;
}

#define VX_TYPE_IS_DATA_OBJECT(type) ((((type) >= VX_TYPE_LUT) && ((type) <= VX_TYPE_REMAP)) || \
                                        (((type) >= VX_TYPE_OBJECT_ARRAY) && ((type) <= VX_TYPE_TENSOR)) )

VX_INTERNAL_API vx_status vxoNode_SetParameter(vx_node node, vx_uint32 index, vx_reference value)
{
    vx_type_e type;
    gcmHEADER_ARG("node=%p, index=0x%x, value=%p", node, index, value);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (node->kernel == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_NODE;
    }
    if (index >= node->kernel->signature.paramCount || index >= VX_MAX_PARAMETERS)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (node->kernel->signature.stateTable[index] == VX_PARAMETER_STATE_OPTIONAL)
    {
        if (value == VX_NULL)
        {
            gcmFOOTER_NO();
            return VX_SUCCESS;
        }
    }
    else
    {
        if (value == VX_NULL)
        {

            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;

        }
    }
    if (!vxoReference_IsValidAndNoncontext((vx_reference_s *)value))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_VALUE;
    }
    type = vxoReference_GetType(value);

    if (node->kernel->signature.dataTypeTable[index] != type)
    {
        if (node->kernel->signature.dataTypeTable[index] == VX_TYPE_REFERENCE)
        {
            if (!VX_TYPE_IS_DATA_OBJECT(type))
            {
                gcmFOOTER_NO();
                return VX_ERROR_INVALID_TYPE;
            }
        }
        else if (type == VX_TYPE_SCALAR)
        {
            vx_enum dataType = vxoScalar_GetDataType((vx_scalar)value);

            if (node->kernel->signature.dataTypeTable[index] != dataType)
            {
                gcmFOOTER_NO();
                return VX_ERROR_INVALID_TYPE;
            }
        }
        else
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_TYPE;
        }
    }

    if (node->graph->verified &&
        (node->kernel->signature.directionTable[index] != VX_INPUT || !node->kernel->kernelShader) &&
        (node->kernel->enumeration != VX_KERNEL_IMPORT_FROM_FILE))
    {
        /* Only VXC header node support change INPUT parameters without re-verify */
        node->graph->reverify = vx_true_e;
        node->graph->verified = vx_false_e;
    }

    if (node->paramTable[index] != VX_NULL && node->paramTable[index]->delay != VX_NULL)
    {
        if (!vxoParameterValue_UnbindFromDelay(node->paramTable[index], node, index))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;

        }
    }
    if (value->delay != VX_NULL)
    {
        if (!vxoParameterValue_BindToDelay(value, node, index))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;

        }
    }
    if (node->paramTable[index] != VX_NULL)
    {
        vxoReference_Release(&node->paramTable[index], node->paramTable[index]->type, VX_REF_INTERNAL);
    }

    /* Add the internal ref count of value from the node */
    vxoReference_Increment(value, VX_REF_INTERNAL);

    node->paramTable[index] = value;

    /* Note that we don't need to do anything special for parameters to child graphs. */
    /*if (node->childGraph != VX_NULL)
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
    }*/

    if (node->graph->commandBuffer)
    {
        switch (node->paramTable[index]->type)
        {
        case VX_TYPE_IMAGE:
            {
                vx_image image = (vx_image)node->paramTable[index];
                vx_uint32 planeIndx;
                for (planeIndx = 0; planeIndx < image->planeCount; planeIndx++)
                {
                    if (node->patchLocation[index][planeIndx] != 0)
                    {
                        if((image->importType == VX_MEMORY_TYPE_HOST) && (image->useInternalMem == vx_true_e))
                        {
                            gceSTATUS status;

                            gcmONERROR(gcfVX_AllocateMemForImageFromHandle(image, planeIndx));
                        }

                        node->graph->commandBuffer[node->patchLocation[index][planeIndx]]
                                = image->memory.physicals[planeIndx];
                    }
                }
            }
            break;

        case VX_TYPE_SCALAR:
            {
                vx_scalar scalar = (vx_scalar)node->paramTable[index];
                if (node->patchLocation[index][0] != 0)
                {
                    node->graph->commandBuffer[node->patchLocation[index][0]]
                        = scalar->physical;
                }
            }
            break;

        default:
            vxError("error:The %d parameter type=%d is not handled for graph command",
                index, node->paramTable[index]->type);
            break;
        }
    }

    /* update binary graph node parameters to support multiple input/output buffers
       this is for importing binary graph */
    if (node->kernel->enumeration == VX_KERNEL_IMPORT_FROM_FILE)
    {
        vxoGraphBinary_SetParameter(node, index);
    }

    /* update binary graph input/output table if user set parameter again
      this is for generating binary graph */
    if (1 == node->base.context->options.enableSaveBinary)
    {
        vxoGraphBinary_UpdataIOPhsicalTable(node, index);
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
    return VX_ERROR_NO_RESOURCES;
}

VX_INTERNAL_API vx_parameter vxoNode_GetParameter(vx_node node, vx_uint32 index)
{
    vx_parameter parameter;
    gcmHEADER_ARG("node=%p, index=0x%x", node, index);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (node->kernel == VX_NULL)
    {
        gcmFOOTER_NO();
        return (vx_parameter)vxoContext_GetErrorObject(node->base.context, VX_ERROR_INVALID_NODE);
    }

    if (index >= VX_MAX_PARAMETERS || index >= node->kernel->signature.paramCount)
    {
        gcmFOOTER_NO();
        return (vx_parameter)vxoContext_GetErrorObject(node->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    parameter = (vx_parameter)vxoReference_Create(node->base.context, VX_TYPE_PARAMETER, VX_REF_EXTERNAL, &node->base);

    if (vxoReference_GetStatus((vx_reference)parameter) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return parameter;
    }
    parameter->index    = index;

    parameter->node     = node;
     /* Add the ref count of the node from the parameter */
    vxoReference_Increment(&parameter->node->base, VX_REF_INTERNAL);

    parameter->kernel   = node->kernel;
    /* Add the ref count of the kernel from the parameter */
    vxoReference_Increment(&parameter->kernel->base, VX_REF_INTERNAL);

    gcmFOOTER_NO();
    return parameter;
}

VX_INTERNAL_API vx_status vxoNode_SetChildGraph(vx_node node, vx_graph graph)
{
    gcmHEADER_ARG("node=%p, graph=%p", node, graph);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_NODE;
    }
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

        if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_GRAPH;
        }
        if (graph->paramCount == 0)
        {
            vxError("The child graph %p has no parameter", graph);
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_GRAPH;
        }

        /* Check signatures */
        for (paramIndex = 0; paramIndex < signature1->paramCount; paramIndex++)
        {
            vx_signature signature2;
            vx_uint32 graphParamIndex = graph->paramTable[paramIndex].index;

            if (graph->paramTable[paramIndex].node == VX_NULL)
            {
                vxInfo("No.%d parameter of the child graph %p refer to NULL node", paramIndex, graph);
                continue;
            }

            signature2 = &graph->paramTable[paramIndex].node->kernel->signature;

            if (signature2->paramCount > signature1->paramCount)continue;
            if(paramIndex < signature2->paramCount){
                if (signature1->directionTable[paramIndex]      != signature2->directionTable[graphParamIndex]
                    || signature1->stateTable[paramIndex]       != signature2->stateTable[graphParamIndex]
                    || signature1->dataTypeTable[paramIndex]    != signature2->dataTypeTable[graphParamIndex])
                {
                    vxError("No.%d parameter of the child graph %p does not match the parameter of node %p",
                            paramIndex, graph, node);
                    gcmFOOTER_NO();
                    return VX_ERROR_INVALID_GRAPH;
                }
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
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("node=%p, accessible=0x%x", node, accessible);
    vxmASSERT(node);

    for (paramIndex = 0; paramIndex < node->kernel->signature.paramCount; paramIndex++)
    {
        vx_reference paramRef = node->paramTable[paramIndex];

        if (paramRef == VX_NULL) continue;

        if (paramRef->isVirtual) paramRef->accessible = accessible;
    }
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("node=%p", node);

    if (!node->kernelAttributes.isAllGPU)
    {
        gcmFOOTER_NO();
        return VX_ERROR_NOT_IMPLEMENTED;
    }
    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, &CmdBuffer, &CmdSizeBytes);

    if (gcStatus != gcvSTATUS_OK)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }
    if (CmdSizeBytes > 0 && CmdBuffer)
    {
        if (node->cmdBuffer && (node->cmdSizeBytes < CmdSizeBytes))
        {
            vxFree(node->cmdBuffer);

            node->cmdBuffer = NULL;
        }

        if (node->cmdBuffer == NULL)
        {
            node->cmdSizeBytes = (size_t) CmdSizeBytes;

            node->cmdBuffer = vxAllocateAndZeroMemory((vx_uint32)node->cmdSizeBytes);
        }

        memcpy(node->cmdBuffer, CmdBuffer, node->cmdSizeBytes);
    }

    /*gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, NULL, NULL);
    if (gcStatus != gcvSTATUS_OK)
        return VX_FAILURE;*/
    gcmFOOTER_NO();
    return VX_SUCCESS;
}


VX_INTERNAL_API vx_status vxoNode_Replay(vx_node node)
{
    gceSTATUS gcStatus = gcvSTATUS_OK;
    gcmHEADER_ARG("node=%p", node);

    if (!node->kernelAttributes.isAllGPU){

        gcmFOOTER_NO();
        return VX_ERROR_NOT_IMPLEMENTED;
    }

    if ((node->cmdBuffer == NULL) || (node->cmdSizeBytes == 0))
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }

    vxoPerf_Begin(&node->perf);

    gcStatus = gcoVX_Replay((gctPOINTER)node->cmdBuffer, (gctUINT32)node->cmdSizeBytes);

    if (gcStatus != gcvSTATUS_OK)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }

    gcStatus = gcoVX_Commit(gcvFALSE, gcvFALSE, NULL, NULL);


    if (gcStatus != gcvSTATUS_OK)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }

#if gcdDUMP
    vxoDumpOutput(node, node->paramTable, node->kernel->signature.paramCount);
#endif

    node->executed = vx_true_e;
    node->status = VX_SUCCESS;
    vxoPerf_End(&node->perf);
    gcmFOOTER_NO();
    return VX_SUCCESS;
}



VX_INTERNAL_API vx_status vxoNode_Release(vx_node_ptr nodePtr)
{
    gctUINT32 gpuCount = 1;
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("nodePtr=%p", nodePtr);

    gcoVX_GetHWConfigGpuCount(&gpuCount);
    if (gpuCount > 1)
    {
        /* release multiGPU operations memory*/
        status |= vxoMultiGpu_FreeMemory(*nodePtr);
    }
    status |= vxoReference_Release((vx_reference_ptr)nodePtr, VX_TYPE_NODE, VX_REF_EXTERNAL);

    gcmFOOTER_ARG("%d", status);
    return status;
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
    gcmDUMP_API("$VX vxCreateGenericNode: graph=%p, kernel=%p", graph, kernel);

    return vxoNode_CreateGeneric(graph, kernel);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryNode(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("node=%p, attribute=0x%x, size=0x%lx", node, attribute, size);
    gcmDUMP_API("$VX vxQueryNode: node=%p, attribute=0x%x, size=0x%lx", node, attribute, size);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
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
                    vxInfo("Node is replicated\n");
                else
                    vxInfo("Node is not replicated\n");

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
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeAttribute(vx_node node, vx_enum attribute, const void *ptr, vx_size size)
{
    gcmHEADER_ARG("node=%p, attribute=0x%x, ptr=%p, size=0x%lx", node, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetNodeAttribute: node=%p, attribute=0x%x, ptr=%p, size=0x%lx", node, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (node->graph->verified == vx_true_e)
    {
        gcmFOOTER_NO();
        return VX_ERROR_NOT_SUPPORTED;
    }

    switch (attribute)
    {
        case VX_NODE_LOCAL_DATA_SIZE:

            if (!node->localDataChangeIsEnabled)
            {
                gcmFOOTER_NO();
                return VX_ERROR_NOT_SUPPORTED;
            }

            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            node->kernelAttributes.localDataSize = *(vx_size *)ptr;

            node->localDataSetByImplementation = vx_false_e;
            break;

        case VX_NODE_LOCAL_DATA_PTR:

            if (!node->localDataChangeIsEnabled)
            {
                gcmFOOTER_NO();
                return VX_ERROR_NOT_SUPPORTED;
            }
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            node->kernelAttributes.localDataPtr = *(vx_ptr *)ptr;

            node->localDataSetByImplementation = vx_false_e;

            break;

        case VX_NODE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

#ifdef OPENVX_KHR_TILING
            if (node->kernelAttributes.borderMode.mode == VX_BORDER_MODE_SELF)
            {
                gcmFOOTER_NO();
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
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxReplicateNode(vx_graph graph, vx_node first_node, vx_bool replicate[], vx_uint32 number_of_parameters)
{
    vx_uint32 n;
    vx_uint32 p;
    vx_uint32 numParams = 0;
    vx_size   num_of_replicas = 0;
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("graph=%p, first_node=%p, replicate=%p, number_of_parameters=0x%x", graph, first_node, replicate, number_of_parameters);
    gcmDUMP_API("$VX vxReplicateNode: graph=%p, first_node=%p, replicate=%p, number_of_parameters=0x%x", graph, first_node, replicate, number_of_parameters);

    if (vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH) != vx_true_e)
    {
        vxError("Graph %p was invalid!\n", graph);
        vxAddLogEntry((vx_reference)graph, VX_ERROR_INVALID_REFERENCE, "Graph %p as invalid!\n", graph);
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (vxoReference_IsValidAndSpecific(&first_node->base, VX_TYPE_NODE) != vx_true_e)
    {
        vxError("Node %p was invalid!\n", first_node);
        vxAddLogEntry((vx_reference)first_node, VX_ERROR_INVALID_REFERENCE, "Node %p as invalid!\n", first_node);
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (first_node->graph != graph)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }

    if (replicate == NULL)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }
    /* validate replicated params */
    status = vxQueryNode(first_node, VX_NODE_PARAMETERS, &numParams, sizeof(numParams));
    if (VX_SUCCESS == status)
    {
        if (numParams != number_of_parameters)
        {
            gcmFOOTER_ARG("%d", status);
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
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
        {
            gcmFOOTER_NO();
            return VX_FAILURE;
        }
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
                {
                    gcmFOOTER_NO();
                    return VX_FAILURE;
                }
                if (num_of_replicas == 0)
                    num_of_replicas = levels;

                if (num_of_replicas != 0 && levels != num_of_replicas)
                {
                    gcmFOOTER_NO();
                    return VX_FAILURE;
                }
            }
            else
            {
                gcmFOOTER_NO();
                return VX_FAILURE;
            }
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
    gcmFOOTER_NO();
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseNode(vx_node *node)
{
    gcmDUMP_API("$VX vxReleaseNode: node=%p", node);

    return vxoNode_Release(node);
}

VX_API_ENTRY vx_status  VX_API_CALL vxRemoveNode(vx_node *node)
{
    gcmDUMP_API("$VX vxRemoveNode: node=%p", node);
    return vxoNode_Remove(node);
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeCallback(vx_node node, vx_nodecomplete_f callback)
{
    gcmHEADER_ARG("node=%p, callback=0x%x", node, callback);
    gcmDUMP_API("$VX vxAssignNodeCallback: node=%p, callback=0x%x", node, callback);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    if (callback != VX_NULL && node->completeCallback != VX_NULL)
    {
        vxError("Can't re-assign the existing callback, %p, of the node, %p, to the new one, %p",
                node->completeCallback, node, callback);
        gcmFOOTER_NO();
        return VX_ERROR_NOT_SUPPORTED;
    }

    node->completeCallback = callback;
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_API_ENTRY vx_nodecomplete_f VX_API_CALL vxRetrieveNodeCallback(vx_node node)
{
    gcmDUMP_API("$VX vxRetrieveNodeCallback: node=%p", node);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_NULL;

    return node->completeCallback;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetChildGraphOfNode(vx_node node, vx_graph graph)
{
    gcmDUMP_API("$VX vxSetChildGraphOfNode: node=%p, graph=%p", node, graph);

    return vxoNode_SetChildGraph(node, graph);
}

VX_API_ENTRY vx_graph VX_API_CALL vxGetChildGraphOfNode(vx_node node)
{
    gcmDUMP_API("$VX vxGetChildGraphOfNode: node=%p", node);

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

    gcmHEADER_ARG("node=%p, variantName=%s", node, variantName);
    gcmDUMP_API("$VX vxChooseKernelVariant: node=%p, variantName=%s", node, variantName);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    target = &node->base.context->targetTable[node->targetIndex];

    gcoOS_StrDup(gcvNULL, node->kernel->name, &kernelName);
    colonCharPtr    = strchr(kernelName, ':');

    if (colonCharPtr != VX_NULL) *colonCharPtr = '\0';

    status = target->funcs.iskernelsupported(target, target->name, kernelName, variantName, OUT &kernelIndex);

    gcoOS_Free(gcvNULL, kernelName);

    if (status != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    vxoReference_Decrement(&node->kernel->base, VX_REF_INTERNAL);

    node->kernel = &target->kernelTable[kernelIndex];

    vxoReference_Increment(&node->kernel->base, VX_REF_INTERNAL);
    gcmFOOTER_NO();
    return VX_SUCCESS;
}
#endif

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeTarget(vx_node node, vx_enum target_enum, const char* target_string)
{
    vx_context context;
    vx_kernel_s *kernel = VX_NULL;
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_uint32 index, targetIndex = 0;

    gcmHEADER_ARG("node=%p, target_enum=0x%x, target_string=%s", node, target_enum, target_string);
    gcmDUMP_API("$VX vxSetNodeTarget: node=%p, target_enum=0x%x, target_string=%s", node, target_enum, target_string);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE))
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_NODE;
    }
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
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_bool vxoNode_IsGPUNode(vx_node node)
{
    if ((node->layer && node->layer->operations[0]->target != VXNNE_OPERATION_TARGET_SW)  ||
        (!node->layer && node->kernelAttributes.isGPUKernel))
    {
        return vx_true_e;
    }
    else
    {
        return vx_false_e;
    }
}

VX_INTERNAL_API vx_bool vxoNode_HasCPUfunction(vx_node node)
{
    if ((node->layer && node->layer->hasCPUFunction) ||
        (!node->layer && !node->kernelAttributes.isAllGPU))
    {
#if gcdDEBUG
        vxInfo("node name =%s is not full GPU node", node->kernel->name);
#endif
        return vx_true_e;
    }
    else
    {
        return vx_false_e;
    }
}

VX_INTERNAL_API vx_bool vxoNode_IsConvNode(vx_node node)
{
    vx_bool isConv = vx_false_e;
    vx_enum nodeType = vxoGraphOptimization_getKernelType(node);
    if(nodeType & OP_CONVOLUTION )
    {
        isConv = vx_true_e;
    }

    return isConv;
}

VX_INTERNAL_API vx_bool vxoNode_IsFCNode(vx_node node)
{
    vx_bool isFC = vx_false_e;
    vx_enum nodeType = vxoGraphOptimization_getKernelType(node);
    if(nodeType & OP_FULLYCONNECTED)
    {
        isFC = vx_true_e;
    }

    return isFC;
}

VX_INTERNAL_API vx_bool vxoNode_IsLeakyReluNode(vx_node node)
{
    vx_bool isLeakyRelu = vx_false_e;
    vx_enum kernelType = node->kernel->enumeration;

    if(VX_KERNEL_ACTIVATION_LAYER == kernelType)
    {
        vx_enum reluType = SCALAR_VALUE(node->paramTable[1], u32);
        if(reluType == VX_NN_ACTIVATION_RELU1 ||
            reluType == VX_NN_ACTIVATION_RELU6 )
        {
            isLeakyRelu = vx_true_e;
        }
    }else if(VX_KERNEL_NN_LEAKY == kernelType)
        isLeakyRelu = vx_true_e;

    return isLeakyRelu;
}

VX_INTERNAL_API vx_bool vxoNode_IsMaxPoolingNode(vx_node node)
{
    vx_bool isMP = vx_false_e;
    vx_enum kernelType = node->kernel->enumeration;
    if(kernelType == VX_KERNEL_NN_POOLING_LAYER2  || kernelType == VX_KERNEL_POOLING_LAYER)
    {
        if(SCALAR_VALUE(node->paramTable[PARAM_POOLING_POOL_TYPE_INDEX], u32) == VX_NN_POOLING_MAX )
        {
            isMP = vx_true_e;
        }
    }

    return isMP;
}

VX_INTERNAL_API void vxoNode_getInfoFromFCNode(vx_node FCnode, vx_uint32 *pad, vx_uint8 *acc,
                                               vx_uint32 *rounding, vx_uint32 *overflow, vx_uint32 *down_scale_round)
{
    if(FCnode->kernel->enumeration == VX_KERNEL_FULLY_CONNECTED_LAYER)
    {
        *overflow = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[3], u32);
        *rounding = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[4], u32);
        *pad = 0;
        *acc = 0;
        *down_scale_round = VX_NN_DS_SIZE_ROUNDING_FLOOR;
    }
    else if(FCnode->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_LAYER)
    {
        *pad = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[3], u32);
        *acc = (vx_uint8)SCALAR_VALUE(FCnode->paramTable[4], u8);
        *overflow = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[5], u32);
        *rounding = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[6], u32);
        *down_scale_round = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[7], u32);
    }
    else if(FCnode->kernel->enumeration == VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER)
    {
        *pad = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[2], u32);
        *acc = (vx_uint8)SCALAR_VALUE(FCnode->paramTable[3], u8);
        *overflow = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[4], u32);
        *rounding = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[5], u32);
        *down_scale_round = (vx_uint32)SCALAR_VALUE(FCnode->paramTable[6], u32);
    }
    else{
        vxError("it is not fc node");
        assert(0);
    }
}

VX_INTERNAL_API vx_status vxoNode_setTensorVxcOptimize(vx_node node)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_NODE;

    node->tensorVxcOptimize = vx_true_e;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoNode_resetTensorVxcOptimize(vx_node node)
{
    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_NODE;

    node->tensorVxcOptimize = vx_false_e;

    return VX_SUCCESS;
}

