/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

VX_INTERNAL_API vx_delay vxoDelay_Create(vx_context context, vx_reference exemplar, vx_size count)
{
    vx_delay  delay;
    vx_uint32 index;

    vxmASSERT(context);

    delay = (vx_delay)vxoReference_Create(context, VX_TYPE_DELAY, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)delay) != VX_SUCCESS) return delay;

    delay->type             = exemplar->type;
    delay->count            = count;
    delay->index            = 0;

    delay->paramListTable   = (vx_delay_parameter)vxAllocateAndZeroMemory(count * sizeof(vx_delay_parameter_s));

    delay->refTable         = (vx_reference_ptr)vxAllocateAndZeroMemory(count * sizeof(vx_reference));

    if (delay->paramListTable == VX_NULL || delay->refTable == VX_NULL)
    {
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
                                context, array->itemType, array->capacity, vx_false_e, VX_TYPE_ARRAY);
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
                    ref = (vx_reference)vxCreateThreshold(context, threshold->type, VX_TYPE_UINT8);
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
                return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }

        if (vxoReference_GetStatus(ref) != VX_SUCCESS) return (vx_delay)ref;

        ref->scope = (vx_reference)delay;

        vxoReference_InitializeForDelay(ref, delay, index);

        vxoReference_Increment(ref, VX_REF_INTERNAL);

        vxoReference_Decrement(ref, VX_REF_EXTERNAL);

        delay->refTable[index] = ref;
    }

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

        if (param->next == VX_NULL) return vx_false_e;

        param = param->next;
    }

    param->node     = node;
    param->index    = index;

    vxoReference_Increment(&delay->base, VX_REF_INTERNAL);

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoParameterValue_UnbindFromDelay(vx_reference value, vx_node node, vx_uint32 index)
{
    vx_delay  delay;
    vx_uint32 paramListIndex;
    vx_delay_parameter param;

    vxmASSERT(value);
    vxmASSERT(node);

    delay = value->delay;

    vxmASSERT(delay);

    paramListIndex = (delay->index + delay->count - abs(value->delayIndex)) % (vx_int32)delay->count;

    if (paramListIndex >= delay->count) return vx_false_e;

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

            if (param == VX_NULL) return vx_false_e;

            if (param->node == node && param->index == index)
            {
                prevParam->next = param->next;
                vxFree(param);
                break;
            }
        }
    }

    vxoReference_Release((vx_reference_ptr)&delay, VX_TYPE_DELAY, VX_REF_INTERNAL);

    return vx_true_e;
}

VX_PUBLIC_API vx_delay vxCreateDelay(vx_context context, vx_reference exemplar, vx_size count)
{
    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxoReference_IsValidAndNoncontext(exemplar))
    {
        return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_REFERENCE);
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
            return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);

        default:
            vxmASSERT(0);
            return (vx_delay)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    return vxoDelay_Create(context, exemplar, count);
}

VX_PUBLIC_API vx_status vxReleaseDelay(vx_delay *delay)
{
    return vxoReference_Release((vx_reference_ptr)delay, VX_TYPE_DELAY, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_reference vxGetReferenceFromDelay(vx_delay delay, vx_int32 index)
{
    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY)) return VX_NULL;

    if ((vx_uint32)abs(index) >= delay->count) return VX_NULL;

    return delay->refTable[(delay->index + abs(index)) % (vx_int32)delay->count];
}

VX_PUBLIC_API vx_status vxQueryDelay(vx_delay delay, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_DELAY_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_enum *)ptr = delay->type;
            break;

        case VX_DELAY_ATTRIBUTE_COUNT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = (vx_uint32)delay->count;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxAgeDelay(vx_delay delay)
{
    vx_uint32 index, refIndex;

    if (!vxoReference_IsValidAndSpecific((vx_reference)delay, VX_TYPE_DELAY)) return VX_ERROR_INVALID_REFERENCE;

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

    return VX_SUCCESS;
}

