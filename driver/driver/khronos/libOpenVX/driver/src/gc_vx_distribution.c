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

VX_INTERNAL_CALLBACK_API void vxoDistribution_Destructor(vx_reference ref)
{
    vx_distribution distribution = (vx_distribution)ref;

    vxoMemory_Free(distribution->base.context, &distribution->memory);
}

VX_API_ENTRY vx_distribution VX_API_CALL vxCreateDistribution(vx_context context, vx_size numBins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution distribution;

    gcmDUMP_API("$VX vxCreateDistribution: context=%p, numBins=0x%lx, offset=0x%x, range=0x%x", context, numBins, offset, range);

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

    distribution->rangeX    = range;
    distribution->rangeY    = 1;

    return distribution;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDistribution(vx_distribution *distribution)
{
    gcmDUMP_API("$VX vxReleaseDistribution: distribution=%p", distribution);

    return vxoReference_Release((vx_reference_ptr)distribution, VX_TYPE_DISTRIBUTION, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDistribution(vx_distribution distribution, vx_enum attribute, void *ptr, vx_size size)
{
    vx_int32 dims;

    gcmDUMP_API("$VX vxQueryDistribution: distribution=%p, attribute=0x%x, ptr=%p, size=0x%lx", distribution, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_DISTRIBUTION_DIMENSIONS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)(distribution->memory.dimCount - 1);
            break;

        case VX_DISTRIBUTION_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            *(vx_int32*)ptr = (vx_int32)(distribution->rangeX);
            break;

        case VX_DISTRIBUTION_BINS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size*)ptr = (vx_size)distribution->memory.dims[0][VX_DIM_X];
            break;

        case VX_DISTRIBUTION_WINDOW:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            {
                vx_size nbins = (vx_size)distribution->memory.dims[0][VX_DIM_X];
                vx_uint32 range = (vx_uint32)(distribution->rangeX);
                vx_size window = range / nbins;
                if (window*nbins == range)
                    *(vx_uint32*)ptr = (vx_uint32)window;
                else
                    *(vx_uint32*)ptr = 0;
            }
            break;

        case VX_DISTRIBUTION_OFFSET:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            *(vx_int32*)ptr = (vx_int32)distribution->offsetX;
            break;

        case VX_DISTRIBUTION_SIZE:
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

VX_API_ENTRY vx_status VX_API_CALL vxAccessDistribution(vx_distribution distribution, void **ptr, vx_enum usage)
{
    gcmDUMP_API("$VX vxAccessDistribution: distribution=%p, ptr=%p, usage=0x%x", distribution, ptr, usage);

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

VX_API_ENTRY vx_status VX_API_CALL vxCommitDistribution(vx_distribution distribution, const void *ptr)
{
    gcmDUMP_API("$VX vxCommitDistribution: distribution=%p, ptr=%p", distribution, ptr);

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

VX_API_ENTRY vx_status VX_API_CALL vxCopyDistribution(vx_distribution distribution, void *user_ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    gcmDUMP_API("$VX vxCopyDistribution: distribution=%p, user_ptr=%p, usage=0x%x, mem_type=0x%x", distribution, user_ptr, usage, mem_type);

    /* bad references */
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION) ||
        (vxoMemory_Allocate(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        vxError("Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (usage != VX_WRITE_ONLY)) ||
        (user_ptr == NULL) || (mem_type != VX_MEMORY_TYPE_HOST))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to copy distribution\n");
        return status;
    }

    /* copy data */
    size = vxoMemory_ComputeSize(&distribution->memory, 0);

    switch (usage)
    {
    case VX_READ_ONLY:
        if (vxAcquireMutex(distribution->base.lock) == vx_true_e)
        {
            memcpy(user_ptr, distribution->memory.logicals[0], size);
            vxReleaseMutex(distribution->base.lock);

            vxoReference_IncrementReadCount(&distribution->base);
            status = VX_SUCCESS;
        }
        break;
    case VX_WRITE_ONLY:
        if (vxAcquireMutex(distribution->base.lock) == vx_true_e)
        {
            memcpy(distribution->memory.logicals[0], user_ptr, size);
            vxReleaseMutex(distribution->base.lock);

            vxoReference_IncrementWriteCount(&distribution->base);
            status = VX_SUCCESS;
        }
        break;
    }

    return status;
}

#if MAP_UNMAP_REFERENCE
VX_API_ENTRY vx_status VX_API_CALL vxMapDistribution(vx_distribution distribution, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    gcmDUMP_API("$VX vxMapDistribution: distribution=%p, map_id=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", distribution, map_id, ptr, user_ptr, usage, mem_type, flags);

    /* bad references */
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION) ||
        (vxoMemory_Allocate(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        vxError("Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (usage != VX_READ_AND_WRITE) && (usage != VX_WRITE_ONLY)) ||
        (mem_type != VX_MEMORY_TYPE_HOST))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to map distribution\n");
        return status;
    }

    /* map data */
    size = vxoMemory_ComputeSize(&distribution->memory, 0);

    if (vxoContext_MemoryMap(distribution->base.context, (vx_reference)distribution, size, usage, mem_type, flags, NULL, ptr, map_id) == vx_true_e)
    {
        switch (usage)
        {
        case VX_READ_ONLY:
        case VX_READ_AND_WRITE:
            if (vxAcquireMutex(distribution->base.lock) == vx_true_e)
            {
                memcpy(*ptr, distribution->memory.logicals[0], size);
                vxReleaseMutex(distribution->base.lock);

                vxoReference_IncrementReadCount(&distribution->base);
                status = VX_SUCCESS;
            }
            break;
        case VX_WRITE_ONLY:
            status = VX_SUCCESS;
            break;
        }

        if (status == VX_SUCCESS)
            vxoReference_Increment(&distribution->base, VX_REF_EXTERNAL);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapDistribution(vx_distribution distribution, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    gcmDUMP_API("$VX vxUnmapDistribution: vx_distribution=%p, map_id=%p", vx_distribution, map_id);

    /* bad references */
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION) ||
        (vxoMemory_Allocate(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        vxError("Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(distribution->base.context, (vx_reference)distribution, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to unmap distribution\n");
        return status;
    }

    /* unmap data */
    size = vxoMemory_ComputeSize(&distribution->memory, 0);

    {
        vx_uint32 id = (vx_uint32)map_id;
        vx_memory_map_s* map = &distribution->base.context->memoryMaps[id];

        switch (map->usage)
        {
        case VX_READ_ONLY:
            status = VX_SUCCESS;
            break;
        case VX_READ_AND_WRITE:
        case VX_WRITE_ONLY:
            if (vxAcquireMutex(distribution->base.lock) == vx_true_e)
            {
                memcpy(distribution->memory.logicals[0], map->logical, size);
                vxReleaseMutex(distribution->base.lock);

                vxoReference_IncrementWriteCount(&distribution->base);
                status = VX_SUCCESS;
            }
            break;
        }

        vxoContext_MemoryUnmap(distribution->base.context, map_id);

        /* regardless of the current status, if we're here, so previous call to vxMapDistribution()
         * was successful and thus ref was locked once by a call to vxoReference_Increment() */
        vxoReference_Decrement(&distribution->base, VX_REF_EXTERNAL);
    }

    return status;
}
#else
VX_API_ENTRY vx_status VX_API_CALL vxMapDistribution(vx_distribution distribution, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    gcmDUMP_API("$VX vxMapDistribution: distribution=%p, map_id=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", distribution, map_id, ptr, usage, mem_type, flags);

    /* bad references */
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION) ||
        (vxoMemory_Allocate(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        vxError("Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (usage != VX_READ_AND_WRITE) && (usage != VX_WRITE_ONLY)) ||
        (mem_type != VX_MEMORY_TYPE_HOST))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to map distribution\n");
        return status;
    }

    /* map data */
    size = vxoMemory_ComputeSize(&distribution->memory, 0);

    if (vxoContext_MemoryMap(distribution->base.context, (vx_reference)distribution, size, usage, mem_type, flags, NULL, ptr, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("failed to map distribution\n");
        return status;
    }

    vxoReference_Increment(&distribution->base, VX_REF_EXTERNAL);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapDistribution(vx_distribution distribution, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;

    gcmDUMP_API("$VX vxUnmapDistribution: distribution=%p, map_id=%p", distribution, map_id);

    /* bad references */
    if (!vxoReference_IsValidAndSpecific(&distribution->base, VX_TYPE_DISTRIBUTION) ||
        (vxoMemory_Allocate(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        vxError("Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(distribution->base.context, (vx_reference)distribution, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("not found mapped distribution\n");
        return status;
    }

    vxoContext_MemoryUnmap(distribution->base.context, map_id);

    /* regardless of the current status, if we're here, so previous call to vxMapDistribution()
     * was successful and thus ref was locked once by a call to vxoReference_Increment() */
    vxoReference_Decrement(&distribution->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}
#endif

VX_API_ENTRY vx_distribution VX_API_CALL vxCreateVirtualDistribution(vx_graph graph, vx_size numBins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution distribution;
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);

    gcmDUMP_API("$VX vxCreateVirtualDistribution: graph=%p, numBins=0x%lx, offset=0x%x, range=0x%x", graph, numBins, offset, range);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    distribution = vxCreateDistribution(context, numBins, offset, range);

    if (vxoReference_GetStatus((vx_reference)distribution) != VX_SUCCESS) return distribution;

    distribution->base.scope        = (vx_reference)graph;

    distribution->base.isVirtual    = vx_true_e;

    return distribution;
}

