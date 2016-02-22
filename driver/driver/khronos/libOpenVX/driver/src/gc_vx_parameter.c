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
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

VX_INTERNAL_CALLBACK_API void vxoParameter_Destructor(vx_reference ref)
{
    vx_parameter parameter = (vx_parameter)ref;

    vxmASSERT(vxoReference_IsValidAndSpecific(&parameter->base, VX_TYPE_PARAMETER));

    if (parameter->node != VX_NULL)
    {
        vxmASSERT(vxoReference_IsValidAndSpecific(&parameter->node->base, VX_TYPE_NODE));

        if (vxoReference_IsValidAndSpecific(&parameter->node->base, VX_TYPE_NODE))
        {
            /* Release the ref count of the node from the parameter */
            vxoReference_Release((vx_reference_ptr)&parameter->node, VX_TYPE_NODE, VX_REF_INTERNAL);
        }
    }

    if (parameter->kernel != VX_NULL)
    {
        vxmASSERT(vxoReference_IsValidAndSpecific(&parameter->kernel->base, VX_TYPE_KERNEL));

        if (vxoReference_IsValidAndSpecific(&parameter->kernel->base, VX_TYPE_KERNEL))
        {
            /* Release the ref count of the kernel from the parameter */
            vxoReference_Release((vx_reference_ptr)&parameter->kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
        }
    }
}

VX_PUBLIC_API vx_parameter vxGetKernelParameterByIndex(vx_kernel kernel, vx_uint32 index)
{
    vx_parameter parameter;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_NULL;

    if (index >= VX_MAX_PARAMETERS || index >= kernel->signature.paramCount)
    {
        return (vx_parameter)vxoContext_GetErrorObject(kernel->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    parameter = (vx_parameter)vxoReference_Create(kernel->base.context, VX_TYPE_PARAMETER,
                                                    VX_REF_EXTERNAL, &kernel->base.context->base);

    if (vxoReference_GetStatus((vx_reference)parameter) != VX_SUCCESS) return parameter;

    parameter->index    = index;

    parameter->node     = VX_NULL;

    parameter->kernel   = kernel;
    /* Add the ref count of the kernel from the parameter */
    vxoReference_Increment(&parameter->kernel->base, VX_REF_INTERNAL);

    return parameter;
}

VX_PUBLIC_API vx_parameter vxGetParameterByIndex(vx_node node, vx_uint32 index)
{
    return vxoNode_GetParameter(node, index);
}

VX_PUBLIC_API vx_status vxReleaseParameter(vx_parameter *parameter)
{
    if((*parameter)->base.type == VX_TYPE_SCALAR)
    {
        vx_scalar scalar = (vx_scalar)(*parameter);
        if(scalar->node != NULL && ((scalar->base.externalCount == 1) && (scalar->base.internalCount == 0)))
        {
            gcoVX_FreeMemory((gcsSURF_NODE_PTR)scalar->node);
            scalar->node = NULL;
        }
    }

    return vxoReference_Release((vx_reference_ptr)parameter, VX_TYPE_PARAMETER, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_status vxSetParameterByIndex(vx_node node, vx_uint32 index, vx_reference value)
{
    return vxoNode_SetParameter(node, index, value);
}

VX_PUBLIC_API vx_status vxSetParameterByReference(vx_parameter parameter, vx_reference value)
{
    if (!vxoReference_IsValidAndSpecific((vx_reference_s *)parameter, VX_TYPE_PARAMETER))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (parameter->node == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    return vxoNode_SetParameter(parameter->node, parameter->index, value);
}

VX_PUBLIC_API vx_status vxQueryParameter(vx_parameter parameter, vx_enum attribute, void *ptr, vx_size size)
{
    vx_reference paramRef;

    if (!vxoReference_IsValidAndSpecific(&parameter->base, VX_TYPE_PARAMETER)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_PARAMETER_ATTRIBUTE_DIRECTION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = parameter->kernel->signature.directionTable[parameter->index];
            break;

        case VX_PARAMETER_ATTRIBUTE_INDEX:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = parameter->index;
            break;

        case VX_PARAMETER_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = parameter->kernel->signature.dataTypeTable[parameter->index];
            break;

        case VX_PARAMETER_ATTRIBUTE_STATE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = (vx_enum)parameter->kernel->signature.stateTable[parameter->index];
            break;

        case VX_PARAMETER_ATTRIBUTE_REF:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_reference, 0x3);

            if (parameter->node == VX_NULL) return VX_ERROR_NOT_SUPPORTED;

            paramRef = parameter->node->paramTable[parameter->index];

            if (paramRef != VX_NULL) vxoReference_Extract(paramRef);

            *(vx_reference_ptr)ptr = (vx_reference)paramRef;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}


