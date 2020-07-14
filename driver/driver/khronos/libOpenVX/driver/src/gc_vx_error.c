/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

VX_INTERNAL_API vx_error vxoError_Create(vx_context context, vx_status status)
{
    vx_error error = (vx_error)vxoReference_Create(context, VX_TYPE_ERROR, VX_REF_INTERNAL, &context->base);

    gcmHEADER_ARG("context=%p, status=0x%x", context, status);

    if (error == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    error->status = status;

    gcmFOOTER_ARG("%p", error);
    return error;
}

VX_INTERNAL_API vx_status vxError_Release(vx_error_ptr errorPtr)
{
    return vxoReference_Release((vx_reference_ptr)errorPtr, VX_TYPE_ERROR, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_error_s *vxoError_GetErrorObject(vx_context_s *context, vx_status status)
{
    vx_reference_item current;
    gcmHEADER_ARG("context=%p, status=0x%x", context, status);

    vxAcquireMutex(context->base.lock);

    current = context->refListHead;

    while (current != VX_NULL)
    {
        if (current->ref != VX_NULL && current->ref->type == VX_TYPE_ERROR)
        {
            vx_error error = (vx_error_s *)current->ref;

            if (error->status == status) {
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_ARG("%p", error);
                return error;
            }
        }
        current = current->next;
    }

    vxReleaseMutex(context->base.lock);
    gcmFOOTER_NO();
    return VX_NULL;
}

