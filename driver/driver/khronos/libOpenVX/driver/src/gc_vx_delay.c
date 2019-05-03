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

#define _GC_OBJ_ZONE            gcdZONE_VX_DELAY

VX_INTERNAL_API vx_delay vxoDelay_Create(vx_context context, vx_reference exemplar, vx_size count)
{
    vx_delay  delay;
    vx_uint32 index;

    gcmHEADER_ARG("context=%p, exemplar=%p, count=0x%lx", context, exemplar, count);
    vxmASSERT(context);

    delay = (vx_delay)vxoReference_Create(context, VX_TYPE_DELAY, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)delay) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%p", delay);
        return delay;
    }
    delay->type             = exemplar->type;
    delay->count            = count;
    delay->index            = 0;

    delay->paramListTable   = (vx_delay_parameter)vxAllocateAndZeroMemory(count * sizeof(vx_delay_parameter_s));

    delay->refTable         = (vx_reference_ptr)vxAllocateAndZeroMemory(count * sizeof(vx_reference));

    if (delay->paramListTable == VX_NULL || delay->refTable == VX_NULL)
    {
        gcmFOOTER_NO();
        return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_NO_MEMORY);
    }

    for (index = 0; index < count; index++)
    {
        vx_reference ref = VX_NULL;

        switch (exemplar->type)
        {
            case VX_TYPE_IMAGE:
                {
                    vx_image image = (vx_image)exemplar;
                    ref = (vx_reference)vxCreateImage(context, image->width, image->height, image->format);
                }
                break;

            case VX_TYPE_ARRAY:
                {
                    vx_array array = (vx_array)exemplar;
                    ref = (vx_reference)vxoArray_Create(
                        context, array->itemType, array->capacity, vx_false_e, VX_TYPE_ARRAY, vx_false_e);
                }
                break;

            case VX_TYPE_MATRIX:
                {
                    vx_matrix matrix = (vx_matrix)exemplar;
                    ref = (vx_reference)vxCreateMatrix(context, matrix->dataType, matrix->columns, matrix->rows);
                }
                break;

            case VX_TYPE_CONVOLUTION:
                {
                    vx_convolution conv = (vx_convolution)exemplar;
                    vx_convolution conv2 = vxCreateConvolution(context, conv->matrix.columns, conv->matrix.rows);
                    conv2->scale = conv->scale;
                    ref = (vx_reference)conv2;
                }
                break;

            case VX_TYPE_DISTRIBUTION:
                {
                    vx_distribution distribution = (vx_distribution)exemplar;
                    vx_uint32 range = distribution->memory.dims[0][VX_DIM_X] * distribution->windowX;

                    ref = (vx_reference)vxCreateDistribution(
                                context, distribution->memory.dims[0][VX_DIM_X], distribution->offsetX, range);
                }
                break;

            case VX_TYPE_REMAP:
                {
                    vx_remap remap = (vx_remap)exemplar;
                    ref = (vx_reference)vxCreateRemap(
                                context, remap->srcWidth, remap->srcHeight, remap->destWidth, remap->destHeight);
                }
                break;

            case VX_TYPE_LUT:
                {
                    vx_array lut = (vx_array)exemplar;
                    ref = (vx_reference)vxCreateLUT(context, lut->itemType, lut->capacity);
                }
                break;

            case VX_TYPE_PYRAMID:
                {
                    vx_pyramid pyramid = (vx_pyramid)exemplar;
                    ref = (vx_reference)vxCreatePyramid(
                                context, pyramid->levelCount, pyramid->scale, pyramid->width, pyramid->height, pyramid->format);
                }
                break;

            case VX_TYPE_THRESHOLD:
                {
                    vx_threshold threshold = (vx_threshold)exemplar;
                    ref = (vx_reference)vxCreateThreshold(context, threshold->dataType, VX_TYPE_UINT8);
                }
                break;

            case VX_TYPE_SCALAR:
                {
                    vx_scalar scalar = (vx_scalar )exemplar;
                    ref = (vx_reference)vxCreateScalar(context, scalar->dataType, VX_NULL);
                }
                break;

            default:
                vxmASSERT(0);
                gcmFOOTER_NO();
                return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }

        if (vxoReference_GetStatus(ref) != VX_SUCCESS)
        {
            gcmFOOTER_ARG("%p", ref);
            return (vx_delay)ref;
        }
        ref->scope = (vx_reference)delay;

        vxoReference_InitializeForDelay(ref, delay, index);

        vxoReference_Increment(ref, VX_REF_INTERNAL);

        vxoReference_Decrement(ref, VX_REF_EXTERNAL);

        delay->refTable[index] = ref;
    }

    if (exemplar->type == VX_TYPE_PYRAMID)
    {
        /* each pyramid level also need internal delays */
        vx_size j = 0;
        vx_delay pyramid_delay = NULL;

        if (delay->pyramidTable == VX_NULL)
            delay->pyramidTable = (vx_delay *)vxAllocateAndZeroMemory(((vx_pyramid)exemplar)->levelCount * sizeof(vx_delay));

        for (j = 0; j < ((vx_pyramid)exemplar)->levelCount; ++j)
        {
            pyramid_delay = (vx_delay)vxoReference_Create(context, VX_TYPE_DELAY, VX_REF_EXTERNAL, &context->base);
            delay->pyramidTable[j] = pyramid_delay;
            if (vxGetStatus((vx_reference)pyramid_delay) == VX_SUCCESS && pyramid_delay->base.type == VX_TYPE_DELAY)
            {
                pyramid_delay->paramListTable   = (vx_delay_parameter)vxAllocateAndZeroMemory(count * sizeof(vx_delay_parameter_s));
                pyramid_delay->refTable         = (vx_reference_ptr)vxAllocateAndZeroMemory(count * sizeof(vx_reference));
                pyramid_delay->type             = VX_TYPE_IMAGE;
                pyramid_delay->count            = count;

                for (index = 0; index < count; index++)
                {
                    pyramid_delay->refTable[index] = (vx_reference)vxGetPyramidLevel((vx_pyramid)delay->refTable[index], (vx_uint32)j);

                    vxoReference_InitializeForDelay(pyramid_delay->refTable[index], pyramid_delay, index);

                    vxoReference_Increment(pyramid_delay->refTable[index], VX_REF_INTERNAL);

                    vxoReference_Decrement(pyramid_delay->refTable[index], VX_REF_EXTERNAL);

                    ((vx_reference)pyramid_delay->refTable[index])->scope = (vx_reference)pyramid_delay;
                }
            }
        }
    }

    gcmFOOTER_ARG("%p", delay);
    return delay;
}

VX_INTERNAL_CALLBACK_API void vxoDelay_Destructor(vx_reference ref)
{
    vx_delay delay = (vx_delay)ref;
    vx_uint32 i;

    for (i = 0; i < delay->count; i++)
    {
        vxoReference_Release(&delay->refTable[i], delay->type, VX_REF_INTERNAL);
    }

    if (delay->refTable != VX_NULL) vxFree(delay->refTable);

    if (delay->paramListTable != VX_NULL)
    {
        for (i = 0; i < delay->count; i++)
        {
            vx_delay_parameter current = delay->paramListTable[i].next;

            while (current != VX_NULL)
            {
                vx_delay_parameter next = current->next;

                vxFree(current);

                current = next;
            }
        }

        vxFree(delay->paramListTable);
    }
}

VX_INTERNAL_API vx_bool vxoParameterValue_BindToDelay(vx_reference value, vx_node node, vx_uint32 index)
{
    vx_delay delay;
    vx_int32 paramListIndex;
    vx_delay_parameter param;

    gcmHEADER_ARG("value=%p, node=%p, index=0x%x", value, node, index);
    vxmASSERT(value);
    vxmASSERT(node);

    delay = value->delay;
    vxmASSERT(delay);

    paramListIndex  = (delay->index + delay->count - abs(value->delayIndex)) % (vx_int32)delay->count;
    param           = &delay->paramListTable[paramListIndex];

    if (param->node != VX_NULL)
    {
        while (param->next != VX_NULL) param = param->next;

        param->next = (vx_delay_parameter)vxAllocateAndZeroMemory(sizeof(vx_delay_parameter_s));

        if (param->next == VX_NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        param = param->next;
    }

    param->node     = node;
    param->index    = index;

    vxoReference_Increment(&delay->base, VX_REF_INTERNAL);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoParameterValue_UnbindFromDelay(vx_reference value, vx_node node, vx_uint32 index)
{
    vx_delay  delay;
    vx_uint32 paramListIndex;
    vx_delay_parameter param;

    gcmHEADER_ARG("value=%p, node=%p, index=0x%x", value, node, index);
    vxmASSERT(value);
    vxmASSERT(node);

    delay = value->delay;

    vxmASSERT(delay);

    paramListIndex = (delay->index + delay->count - abs(value->delayIndex)) % (vx_int32)delay->count;

    if (paramListIndex >= delay->count)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    param = &delay->paramListTable[paramListIndex];

    if (param->node == node && param->index == index)
    {
        param->node     = VX_NULL;
        param->index    = 0;
    }
    else
    {
        while (1)
        {
            vx_delay_parameter prevParam = param;

            param = param->next;

            if (param == VX_NULL)
            {
                gcmFOOTER_ARG("%d", vx_false_e);
                return vx_false_e;
            }
            if (param->node == node && param->index == index)
            {
                prevParam->next = param->next;
                vxFree(param);
                break;
            }
        }
    }

    vxoReference_Release((vx_reference_ptr)&delay, VX_TYPE_DELAY, VX_REF_INTERNAL);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_API_ENTRY vx_delay VX_API_CALL vxCreateDelay(vx_context context, vx_reference exemplar, vx_size count)
{
    gcmHEADER_ARG("context=%p, exemplar=%p, count=0x%lx", context, exemplar, count);
    gcmDUMP_API("$VX vxCreateDelay: context=%p, exemplar=%p, count=0x%lx", context, exemplar, count);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoReference_IsValidAndNoncontext(exemplar))
    {
        gcmFOOTER_NO();
        return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_REFERENCE);
    }

    if (exemplar->type == (vx_type_e)VX_TYPE_TARGET)
    {
        gcmFOOTER_NO();
        return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }
    switch (exemplar->type)
    {
        case VX_TYPE_IMAGE:
        case VX_TYPE_ARRAY:
        case VX_TYPE_MATRIX:
        case VX_TYPE_CONVOLUTION:
        case VX_TYPE_DISTRIBUTION:
        case VX_TYPE_REMAP:
        case VX_TYPE_LUT:
        case VX_TYPE_PYRAMID:
        case VX_TYPE_THRESHOLD:
        case VX_TYPE_SCALAR:
            break;

        case VX_TYPE_CONTEXT:
        case VX_TYPE_GRAPH:
        case VX_TYPE_NODE:
        case VX_TYPE_KERNEL:
        case VX_TYPE_TARGET:
        case VX_TYPE_PARAMETER:
        case VX_TYPE_REFERENCE:
        case VX_TYPE_DELAY:
            gcmFOOTER_NO();
            return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);

        default:
            vxmASSERT(0);
            gcmFOOTER_NO();
            return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    gcmFOOTER_NO();
    return vxoDelay_Create(context, exemplar, count);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDelay(vx_delay *d)
{
    gcmHEADER_ARG("d=%p", d);
    gcmDUMP_API("$VX vxReleaseDelay: d=%p", d);

    if (vxoReference_IsValidAndSpecific((vx_reference)(*d), VX_TYPE_DELAY) && ((*d)->type == VX_TYPE_PYRAMID) && ((*d)->pyramidTable != NULL))
    {
        vx_delay delay = *d;
        vx_size i;
        vx_size numLevels = ((vx_pyramid)delay->refTable[0])->levelCount;
        /* release pyramid delays */
        for (i = 0; i < numLevels; ++i)
            vxoReference_Release((vx_reference *)&(delay->pyramidTable[i]), VX_TYPE_DELAY, VX_REF_EXTERNAL);
        vxFree((*d)->pyramidTable);
    }

    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)d, VX_TYPE_DELAY, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceFromDelay(vx_delay delay, vx_int32 index)
{
    gcmHEADER_ARG("delay=%p, index=0x%x", delay, index);
    gcmDUMP_API("$VX vxGetReferenceFromDelay: delay=%p, index=0x%x", delay, index);

    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if ((vx_uint32)abs(index) >= delay->count)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcmFOOTER_NO();
    return delay->refTable[(delay->index + abs(index)) % (vx_int32)delay->count];
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDelay(vx_delay delay, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("delay=%p, attribute=0x%x, ptr=%p, size=0x%lx", delay, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryDelay: delay=%p, attribute=0x%x, ptr=%p, size=0x%lx", delay, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_DELAY_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_enum *)ptr = delay->type;
            break;

        case VX_DELAY_SLOTS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = (vx_size)delay->count;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAgeDelay(vx_delay delay)
{
    vx_uint32 index, refIndex;

    gcmHEADER_ARG("delay=%p", delay);
    gcmDUMP_API("$VX vxAgeDelay: delay=%p", delay);

    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    delay->index = (delay->index + 1) % (vx_uint32)delay->count;

    for (index = 0; index < delay->count; index++)
    {
        vx_delay_parameter param = VX_NULL;

        refIndex = (delay->index + index) % (vx_int32)delay->count;

        param = &delay->paramListTable[index];

        do
        {
            if (param->node != VX_NULL)
            {
                vx_node      node       = param->node;
                vx_uint32    paramIndex = param->index;
                vx_reference value      = delay->refTable[refIndex];

                /* change the parameter of node */
                if (node->paramTable[paramIndex])
                {
                    vxoReference_Release(&node->paramTable[paramIndex], node->paramTable[paramIndex]->type, VX_REF_INTERNAL);
                }

                vxoReference_Increment(value, VX_REF_INTERNAL);

                node->paramTable[paramIndex] = value;
            }

            param = param->next;
        }
        while (param != VX_NULL);
    }

    if (delay->type == VX_TYPE_PYRAMID && delay->pyramidTable != NULL)
    {
        /* age pyramid levels*/
        vx_size numLevels = ((vx_pyramid)delay->refTable[0])->levelCount;
        for (index = 0; index < numLevels; ++index)
            vxAgeDelay(delay->pyramidTable[index]);
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterAutoAging(vx_graph graph, vx_delay delay)
{
    unsigned int i;
    vx_status status = VX_SUCCESS;
    vx_bool isAlreadyRegistered = vx_false_e;
    vx_bool isRegisteredDelaysListFull = vx_true_e;

    gcmHEADER_ARG("graph=%p, delay=%p", graph, delay);
    gcmDUMP_API("$VX vxRegisterAutoAging: graph=%p, delay=%p", graph, delay);

    if (vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    /* check if this particular delay is already registered in the graph */
    for (i = 0; i < VX_MAX_REF_COUNT; i++)
    {
        if (vxoReference_IsValidAndSpecific((vx_reference)(graph->delays[i]), VX_TYPE_DELAY) && graph->delays[i] == delay)
        {
            isAlreadyRegistered = vx_true_e;
            break;
        }
    }

    /* if not regisered yet, find the first empty slot and register delay */
    if (isAlreadyRegistered == vx_false_e)
    {
        for (i = 0; i < VX_MAX_REF_COUNT; i++)
        {
            if (vxoReference_IsValidAndSpecific((vx_reference)graph->delays[i], VX_TYPE_DELAY) == vx_false_e)
            {
                isRegisteredDelaysListFull = vx_false_e;
                graph->delays[i] = delay;
                graph->hasAutoAgingDelay = vx_true_e;

                if (graph->dirty)
                    graph->dirty = vx_false_e;

                break;
            }
        }

        /* report error if there is no empty slots to register delay */
        if (isRegisteredDelaysListFull == vx_true_e)
            status = VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}


