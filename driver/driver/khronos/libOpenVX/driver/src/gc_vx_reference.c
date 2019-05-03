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

#define _GC_OBJ_ZONE            gcdZONE_VX_REF

VX_INTERNAL_API vx_status vxoReference_GetStatus(vx_reference ref)
{
    if (ref == VX_NULL)
    {
        /* Default error status */
        return VX_ERROR_NO_RESOURCES;
    }

    if (vxoReference_IsValidAndNoncontext(ref))
    {
        if (ref->type == VX_TYPE_ERROR)
        {
            vx_error error = (vx_error)ref;
            return error->status;
        }
        else
        {
            return VX_SUCCESS;
        }
    }
    else
    {
        if (vxoContext_IsValid((vx_context)ref))
        {
            return VX_SUCCESS;
        }
        else
        {
            /* Unknown error status */
            return VX_FAILURE;
        }
    }
}

VX_INTERNAL_API vx_reference vxoReference_Create(
        vx_context context, vx_type_e type, vx_reference_kind_e kind, vx_reference scope)
{
    vx_reference ref = VX_NULL;

    gcmHEADER_ARG("context=%p, type=0x%x, kind=0x%x, scope=%p", context, type, kind, scope);

    ref = (vx_reference)vxAllocateAndZeroMemory(vxDataType_GetSize(type));

    if (ref == VX_NULL) goto ErrorExit;

    vxoReference_Initialize(ref, context, type, scope);

    vxoReference_Increment(ref, kind);

    if (!vxoContext_AddObject(context, ref)) goto ErrorExit;

    gcmFOOTER_NO();
    return ref;

ErrorExit:
    if (ref != VX_NULL)
    {
        if (ref->lock != VX_NULL)
        {
            vxDestroyMutex(ref->lock);
        }
        vxFree(ref);
    }

    gcmFOOTER_NO();
    return (vx_reference)vxoContext_GetErrorObject(context, VX_ERROR_NO_RESOURCES);;
}

VX_INTERNAL_API void vxoReference_Initialize(
        vx_reference ref, vx_context context, vx_type_e type, vx_reference scope)
{
    vxmASSERT(ref);

    ref->signature          = VX_REF_SIGNATURE_ALIVE;
    ref->context            = context;
    ref->type               = type;
    ref->scope              = scope;

    vxCreateMutex(OUT &ref->lock);

    ref->internalCount      = 0;
    ref->externalCount      = 0;

    ref->writeCount         = 0;
    ref->readCount          = 0;

    ref->delay              = VX_NULL;
    ref->delayIndex         = 0;

    ref->extracted          = vx_false_e;
    ref->isVirtual          = vx_false_e;
    ref->accessible         = vx_false_e;

    ref->reserved           = VX_NULL;
}

VX_INTERNAL_API void vxoReference_InitializeForDelay(
        vx_reference ref, vx_delay delay, vx_int32 delayIndex)
{
    vxmASSERT(ref);

    ref->delay      = delay;
    ref->delayIndex = delayIndex;
}

VX_INTERNAL_API vx_type_e vxoReference_GetType(vx_reference ref)
{
    vxmASSERT(ref);

    return ref->type;
}

VX_INTERNAL_API void vxoReference_Dump(vx_reference ref)
{
    if (ref == VX_NULL)
    {
        vxTrace(VX_TRACE_REF, "<reference>null</reference>\n");
    }
    else
    {
        vxTrace(VX_TRACE_REF,
                "<reference>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <signature>"VX_FORMAT_HEX"</signature>\n"
                "   <context>"VX_FORMAT_HEX"</context>\n"
                "   <type>"VX_FORMAT_HEX"</type>\n"
                "   <count>"VX_FORMAT_HEX" + "VX_FORMAT_HEX"</count>\n"
                "</reference>",
                ref, ref->signature, ref->context, ref->type, ref->externalCount, ref->internalCount);
    }
}

VX_INTERNAL_API vx_bool vxoReference_IsValid(vx_reference ref)
{
    if (ref == VX_NULL) return vx_false_e;

    vxoReference_Dump(ref);

    if (ref->signature == VX_REF_SIGNATURE_RELEASED)
    {
        vxError("The reference object, %p, has already been released", ref);
        return vx_false_e;
    }
    else if (ref->signature != VX_REF_SIGNATURE_ALIVE)
    {
        vxError("The signature of the reference object is unexpected: "VX_FORMAT_HEX,
                ref->signature);
        return vx_false_e;
    }

    if (!vxDataType_IsValid(ref->type)) return vx_false_e;

    if (ref->type == VX_TYPE_CONTEXT && (ref->context != NULL)) return vx_false_e;

    if ((ref->type != VX_TYPE_CONTEXT) && !vxoContext_IsValid(ref->context)) return vx_false_e;

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoReference_IsValidAndNoncontext(vx_reference ref)
{
    if (ref == VX_NULL) return vx_false_e;

    vxoReference_Dump(ref);

    if (ref->signature == VX_REF_SIGNATURE_RELEASED)
    {
        vxError("The reference object, %p, has already been released", ref);
        return vx_false_e;
    }
    else if (ref->signature != VX_REF_SIGNATURE_ALIVE)
    {
        vxError("The signature of the reference object is unexpected: "VX_FORMAT_HEX,
                ref->signature);
        return vx_false_e;
    }

    if (!vxDataType_IsValid(ref->type)) return vx_false_e;

    if (ref->type == VX_TYPE_CONTEXT) return vx_false_e;

    if (!vxoContext_IsValid(ref->context)) return vx_false_e;

    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoReference_IsValidAndSpecific(vx_reference ref, vx_type_e type)
{
    if (ref == VX_NULL) return vx_false_e;

    if (ref->signature == VX_REF_SIGNATURE_RELEASED)
    {
        vxError("The reference object, %p, has already been released", ref);
        return vx_false_e;
    }
    else if (ref->signature != VX_REF_SIGNATURE_ALIVE)
    {
        vxError("The signature of the reference object is unexpected: "VX_FORMAT_HEX,
                ref->signature);
        return vx_false_e;
    }

    if (ref->type != type) return vx_false_e;

    if (ref->type != VX_TYPE_CONTEXT)
    {
        if (!vxoContext_IsValid(ref->context)) return vx_false_e;
    }

    return vx_true_e;
}

VX_INTERNAL_API vx_uint32 vxoReference_Increment(vx_reference ref, vx_reference_kind_e kind)
{
    vx_uint32 count = 0;
    gcmHEADER_ARG("ref=%p, kind=0x%x", ref, kind);
    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    switch (kind)
    {
        case VX_REF_EXTERNAL:
            ref->externalCount++;
            break;

        case VX_REF_INTERNAL:
            ref->internalCount++;
            break;

        default:
            vxmASSERT(0);
    }

    count = ref->externalCount + ref->internalCount;

    vxReleaseMutex(ref->lock);

    vxTrace(VX_TRACE_REF, "vxoReference_Increment(%p, %d): count => %u", ref, kind, count);

    gcmFOOTER_NO();
    return count;
}

VX_INTERNAL_API vx_uint32 vxoReference_Decrement(vx_reference ref, vx_reference_kind_e kind)
{
    vx_uint32 count = 0;
    gcmHEADER_ARG("ref=%p, kind=0x%x", ref, kind);
    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    switch (kind)
    {
        case VX_REF_EXTERNAL:
            if (ref->externalCount == 0)
            {
                vxError("The external count of the reference object, %p, is already 0", ref);
                vxmASSERT(0);
            }
            else
            {
                ref->externalCount--;

                if ((ref->externalCount == 0) && ref->extracted)
                {
                    ref->extracted = vx_false_e;
                }
            }
            break;

        case VX_REF_INTERNAL:
            if (ref->internalCount == 0)
            {
                vxError("The internal count of the reference object, %p, is already 0", ref);
                vxmASSERT(0);
            }
            else
            {
                ref->internalCount--;
            }
            break;

        default:
            vxmASSERT(0);
    }

    count = ref->externalCount + ref->internalCount;

    vxReleaseMutex(ref->lock);

    vxTrace(VX_TRACE_REF, "vxoReference_Decrement(%p, %d): count => %u", ref, kind, count);

    gcmFOOTER_NO();
    return count;
}

VX_INTERNAL_API void vxoReference_Extract(vx_reference ref)
{
    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    if (ref->externalCount == 0) ref->extracted = vx_true_e;

    vxReleaseMutex(ref->lock);

    vxoReference_Increment(ref, VX_REF_EXTERNAL);
}

VX_INTERNAL_API vx_uint32 vxoReference_GetExternalCount(vx_reference ref)
{
    vx_uint32 count = 0;

    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    count = ref->externalCount;

    vxReleaseMutex(ref->lock);

    return count;
}

VX_INTERNAL_API vx_uint32 vxoReference_GetInternalCount(vx_reference ref)
{
    vx_uint32 count = 0;

    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    count = ref->internalCount;

    vxReleaseMutex(ref->lock);

    return count;
}

VX_INTERNAL_API vx_uint32 vxoReference_GetTotalCount(vx_reference ref)
{
    vx_uint32 count = 0;

    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    count = ref->externalCount + ref->internalCount;

    vxReleaseMutex(ref->lock);

    return count;
}

VX_PRIVATE_API void vxoReference_Destroy(vx_reference ref, vx_bool order)
{
    vx_object_destructor_f destructor;

    vxmASSERT(ref);

    vxoContext_RemoveObject(ref->context, ref, order);

    destructor = vxDataType_GetDestructor(ref->type);

    if (destructor != VX_NULL) destructor(ref);

    vxmASSERT(ref->lock);
    vxDestroyMutex(ref->lock);

    ref->signature = VX_REF_SIGNATURE_RELEASED;

    if (!(vxDataType_IsStatic(ref->type) || ref->type == (vx_type_e)VX_TYPE_TARGET)) vxFree(ref);
}

VX_PRIVATE_API vx_status _Reference_Release(
        INOUT vx_reference_ptr refPtr, vx_type_e type, vx_reference_kind_e kind, vx_bool order)
{
    vx_reference ref;

    gcmHEADER_ARG("refPtr=%p, type=0x%x, kind=0x%x", refPtr, type, kind);

    if (refPtr == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    ref = *refPtr;

    if (!vxoReference_IsValidAndSpecific(ref, type))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    vxoReference_Dump(ref);

    if (vxoReference_Decrement(ref, kind) == 0)
    {
        vxTrace(VX_TRACE_REF, "vxoReference_Release():"
                " the reference count of the referece obejct, %p, is released to 0", ref);


        vxoReference_Destroy(ref, order);
    }

    *refPtr = VX_NULL;

    gcmFOOTER_NO();

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoReference_Release(
        INOUT vx_reference_ptr refPtr, vx_type_e type, vx_reference_kind_e kind)
{
    return _Reference_Release(refPtr, type, kind, vx_false_e);
}

VX_INTERNAL_API vx_status vxoReference_ReleaseEx(
        INOUT vx_reference_ptr refPtr, vx_type_e type, vx_reference_kind_e kind, vx_bool order)
{
    return _Reference_Release(refPtr, type, kind, order);
}

VX_PRIVATE_API void vxoReference_PolluteAllInputGraphs(vx_reference targetRef)
{
    vx_reference_item current;
    vx_context context;

    gcmHEADER_ARG("targetRef=%p", targetRef);
    vxmASSERT(targetRef);

    context = targetRef->context;
    current = context->refListHead;

    while (current != VX_NULL)
    {
        vx_reference ref = current->ref;

        if (ref != VX_NULL && ref != targetRef && ref->type == VX_TYPE_GRAPH)
        {
            vxoGraph_PolluteIfInput((vx_graph)ref, targetRef);
        }

        current = current->next;
    }
    gcmFOOTER_NO();

}

VX_INTERNAL_API void vxoReference_IncrementWriteCount(vx_reference ref)
{
    gcmHEADER_ARG("ref=%p", ref);
    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    ref->writeCount++;

    if (ref->extracted) vxoReference_PolluteAllInputGraphs(ref);

    vxReleaseMutex(ref->lock);
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoReference_IncrementReadCount(vx_reference ref)
{
    gcmHEADER_ARG("ref=%p", ref);
    vxmASSERT(ref);

    vxAcquireMutex(ref->lock);

    ref->readCount++;

    vxReleaseMutex(ref->lock);
    gcmFOOTER_NO();
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("ref=%p, attribute=0x%x, ptr=%p, size=0x%lx", ref, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryReference: ref=%p, attribute=0x%x, ptr=%p, size=0x%lx", ref, attribute, ptr, size);

    if (!vxoReference_IsValidAndNoncontext(ref)
        && !vxoContext_IsValid((vx_context_s *)ref))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_REF_ATTRIBUTE_COUNT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = vxoReference_GetExternalCount(ref);
            break;

        case VX_REF_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_enum *)ptr = vxoReference_GetType(ref);
            break;

        case VX_REF_ATTRIBUTE_NAME:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_char*, 0x3);

            *(vx_char**)ptr = &ref->name[0];
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetStatus(vx_reference reference)
{
    vx_status status = vxoReference_GetStatus(reference);

    gcmDUMP_API("$VX vxGetStatus: reference=%p", reference);

    vxTrace(VX_TRACE_REF, "Got the status, %d, from the reference, %p", status, reference);

    return status;
}
#define GC_GET_NODE(type, obj, p)   \
    if(((type)(obj))->memory.nodePtrs == NULL)  \
        status = VX_ERROR_NO_MEMORY;    \
    else    \
    {    \
        *ptr = ((type)(obj))->memory.nodePtrs[p];   \
        status = VX_SUCCESS;    \
    }

vx_status vxQuerySurfaceNode(vx_reference reference,
                             vx_uint32 plane,
                             void **ptr)
{
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("reference=%p, plane=0x%x, ptr=%p", reference, plane, ptr);
    switch(reference->type)
    {
    case VX_TYPE_IMAGE:
        GC_GET_NODE(vx_image, reference, plane);
        if(*ptr == NULL)
        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t patch_addr;
            void * pointer = NULL;

            status |= vxGetValidRegionImage((vx_image)reference, &rect);
            status |= vxAccessImagePatch((vx_image)reference, &rect, 0, &patch_addr, &pointer, VX_READ_ONLY);
            status |= vxCommitImagePatch((vx_image)reference, NULL, 0, &patch_addr, pointer);

            GC_GET_NODE(vx_image, reference, plane);
        }
        break;
    case VX_TYPE_REMAP:
        GC_GET_NODE(vx_remap, reference, plane);
        break;
    case VX_TYPE_DISTRIBUTION:
        GC_GET_NODE(vx_distribution, reference, plane);
        if(*ptr == NULL)
        {
            vx_uint32 size = 0;
            void * pointer = NULL;
            status |= vxAccessDistribution((vx_distribution)reference, &pointer, VX_WRITE_ONLY);
            status |= vxQueryDistribution((vx_distribution)reference, VX_DISTRIBUTION_SIZE, &size, sizeof(size));
            memset(pointer, 0, size);
            status |= vxCommitDistribution((vx_distribution)reference, pointer);

            GC_GET_NODE(vx_distribution, reference, plane);
        }
        break;
    case VX_TYPE_LUT:
        GC_GET_NODE(vx_lut_s*, reference, plane);
        break;
    case VX_TYPE_ARRAY:
        GC_GET_NODE(vx_array, reference, plane);
        if(*ptr == NULL)
        {
            if (!vxoArray_AllocateMemory((vx_array)reference))
                status = VX_ERROR_NO_MEMORY;
            else
            {
                GC_GET_NODE(vx_array, reference, plane);
            }
        }
        break;
    case VX_TYPE_SCALAR:
        {
            vx_scalar scalar = (vx_scalar)reference;
            if(scalar->node != NULL && scalar->value != NULL)
            {
                *ptr = scalar->node;
                status = VX_SUCCESS;
            }
            else
                status = VX_ERROR_NO_MEMORY;
        }
        break;
    default:
        GC_GET_NODE(vx_image, reference, plane);
        break;
    }

    /*if(plane == 0) vxAcquireMutex(reference->lock);*/ /*disable this because vxRelaseMutes has problem in test case related with array*/
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference* ref_ptr)
{
    //return vxoReference_Release(ref_ptr, (*ref_ptr)->type, VX_REF_INTERNAL);

    vx_status status = VX_SUCCESS;

    vx_reference ref = (ref_ptr ? *ref_ptr : NULL);
    gcmHEADER_ARG("reference=%p", ref_ptr);
    gcmDUMP_API("$VX vxReleaseReference: reference=%p", ref_ptr);

    if ((ref && ref->type == VX_TYPE_CONTEXT && vxoContext_IsValid((vx_context)ref)) ||
        (vxoReference_IsValidAndNoncontext(ref) == vx_true_e))
    {
        status = vxoReference_Release(ref_ptr, ref->type, VX_REF_EXTERNAL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_context context;
    gcmHEADER_ARG("ref=%p, name=%s", ref, name);
    gcmDUMP_API("$VX vxSetReferenceName: ref=%p, name=%s", ref, name);

    if (vxoReference_IsValidAndNoncontext(ref))
    {
        context= vxGetContext(ref);

        strncpy(ref->name, name, strnlen(name, VX_MAX_REFERENCE_NAME));
        status = VX_SUCCESS;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}


vx_status vxCommitSurfaceNode(vx_reference reference)
{
    vx_status status = VX_FAILURE;

    /*status = vxReleaseMutex(reference->lock);*/ /*disable this because vxRelaseMutes has problem in test case related with array*/
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRetainReference(vx_reference ref)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("ref=%p", ref);
    gcmDUMP_API("$VX vxRetainReference: ref=%p", ref);

    if ((ref->type == VX_TYPE_CONTEXT && vxoContext_IsValid((vx_context)ref)) ||
        (vxoReference_IsValidAndNoncontext(ref) == vx_true_e))
    {
        vxoReference_Increment(ref, VX_REF_EXTERNAL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}


