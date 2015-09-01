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

VX_INTERNAL_CALLBACK_API void vxoDistribution_Destructor(vx_reference ref)
{
    vx_distribution distribution = (vx_distribution)ref;

    vxoMemory_Free(distribution->base.context, &distribution->memory);
}

VX_PUBLIC_API vx_distribution vxCreateDistribution(vx_context context, vx_size numBins, vx_size offset, vx_size range)
{
    vx_distribution distribution;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (numBins == 0 || range == 0)
    {
        return (vx_distribution)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    distribution = (vx_distribution)vxoReference_Create(context, VX_TYPE_DISTRIBUTION, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)distribution) != VX_SUCCESS) return distribution;

    distribution->memory.planeCount                 = 1;
    distribution->memory.dimCount                   = 2;
    distribution->memory.strides[0][VX_DIM_CHANNEL] = sizeof(vx_int32);
    distribution->memory.dims[0][VX_DIM_CHANNEL]    = 1;
    distribution->memory.dims[0][VX_DIM_X]          = (vx_int32)numBins;
    distribution->memory.dims[0][VX_DIM_Y]          = 1;

    distribution->windowX   = (vx_uint32)range / (vx_uint32)numBins;
    distribution->windowY   = 1;
    distribution->offsetX   = (vx_uint32)offset;
    distribution->offsetY   = 0;

    return distribution;
}

VX_PUBLIC_API vx_status vxReleaseDistribution(vx_distribution *distribution)
{
    return vxoReference_Release((vx_reference_ptr)distribution, VX_TYPE_DISTRIBUTION, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_status vxQueryDistribution(vx_distribution distribution, vx_enum attribute, void *ptr, vx_size size)
{
    vx_int32 dims;

    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_DISTRIBUTION_ATTRIBUTE_DIMENSIONS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)(distribution->memory.dimCount - 1);
            break;

        case VX_DISTRIBUTION_ATTRIBUTE_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)(distribution->memory.dims[0][VX_DIM_X] * distribution->windowX);
            break;

        case VX_DISTRIBUTION_ATTRIBUTE_BINS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)distribution->memory.dims[0][VX_DIM_X];
            break;

        case VX_DISTRIBUTION_ATTRIBUTE_WINDOW:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32*)ptr = distribution->windowX;
            break;

        case VX_DISTRIBUTION_ATTRIBUTE_OFFSET:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)distribution->offsetX;
            break;

        case VX_DISTRIBUTION_ATTRIBUTE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            dims = distribution->memory.dimCount - 1;

            *(vx_size*)ptr = distribution->memory.strides[0][dims] * distribution->memory.dims[0][dims];
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxAccessDistribution(vx_distribution distribution, void **ptr, vx_enum usage)
{
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoMemory_Allocate(distribution->base.context, &distribution->memory)) return VX_ERROR_NO_MEMORY;

    if (ptr != VX_NULL)
    {
        vx_size size;

        vxAcquireMutex(distribution->base.lock);

        size = vxoMemory_ComputeSize(&distribution->memory, 0);

        vxoMemory_Dump(&distribution->memory);

        if (*ptr == VX_NULL)
        {
            *ptr = distribution->memory.logicals[0];
        }
        else
        {
            vxMemCopy(*ptr, distribution->memory.logicals[0], size);
        }

        vxReleaseMutex(distribution->base.lock);

        vxoReference_IncrementReadCount(&distribution->base);
    }

    vxoReference_Increment(&distribution->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxCommitDistribution(vx_distribution distribution, void *ptr)
{
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoMemory_Allocate(distribution->base.context, &distribution->memory)) return VX_ERROR_NO_MEMORY;

    if (ptr != VX_NULL)
    {
        vxAcquireMutex(distribution->base.lock);

        if (ptr != distribution->memory.logicals[0])
        {
            vx_size size = vxoMemory_ComputeSize(&distribution->memory, 0);

            vxMemCopy(distribution->memory.logicals[0], ptr, size);
        }

        vxReleaseMutex(distribution->base.lock);

        vxoReference_IncrementWriteCount(&distribution->base);
    }

    vxoReference_Decrement(&distribution->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}


