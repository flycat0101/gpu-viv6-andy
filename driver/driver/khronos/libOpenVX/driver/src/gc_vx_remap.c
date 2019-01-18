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

VX_INTERNAL_CALLBACK_API void vxoRemap_Destructor(vx_reference ref)
{
    vx_remap remap = (vx_remap)ref;

    vxoMemory_Free(remap->base.context, &remap->memory);
}

VX_API_ENTRY vx_remap VX_API_CALL vxCreateRemap(
        vx_context context, vx_uint32 src_width, vx_uint32 src_height, vx_uint32 dst_width, vx_uint32 dst_height)
{
    vx_remap remap;

    gcmDUMP_API("$VX vxCreateRemap: context=%p, src_width=0x%x, src_height=0x%x, dst_width=0x%x, dst_height=0x%x",
        context, src_width, src_height, dst_width, dst_height);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (src_width == 0 || src_height == 0 || dst_width == 0 || dst_height == 0)
    {
        return (vx_remap)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    remap = (vx_remap)vxoReference_Create(context, VX_TYPE_REMAP, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)remap) != VX_SUCCESS) return remap;

    remap->srcWidth     = src_width;
    remap->srcHeight    = src_height;
    remap->destWidth    = dst_width;
    remap->destHeight   = dst_height;

    remap->memory.planeCount                    = 1;
    remap->memory.dimCount                      = 3;
    remap->memory.dims[0][VX_DIM_CHANNEL]       = 2;
    remap->memory.dims[0][VX_DIM_X]             = dst_width;
    remap->memory.dims[0][VX_DIM_Y]             = dst_height;
    remap->memory.strides[0][VX_DIM_CHANNEL]    = sizeof(vx_float32);

    return remap;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseRemap(vx_remap *remap)
{

    gcmDUMP_API("$VX vxReleaseRemap: remap=%p", remap);
    return vxoReference_Release((vx_reference_ptr)remap, VX_TYPE_REMAP, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryRemap(vx_remap remap, vx_enum attribute, void *ptr, vx_size size)
{
    gcmDUMP_API("$VX vxQueryRemap: remap=%p, attribute=0x%x, ptr=%p, size=0x%lx", remap, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_REMAP_SOURCE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->srcWidth;
            break;

        case VX_REMAP_SOURCE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->srcHeight;
            break;

        case VX_REMAP_DESTINATION_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->destWidth;
            break;

        case VX_REMAP_DESTINATION_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->destHeight;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetRemapPoint(
        vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 src_x, vx_float32 src_y)
{
    vx_float32_ptr ptr;

    gcmDUMP_API("$VX vxSetRemapPoint: remap=%p, dst_x=0x%x, dst_y=0x%x, src_x=%f, src_y=%f", remap, dst_x, dst_y, src_x, src_y);

    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    if (dst_x >= remap->destWidth || dst_y >= remap->destHeight) return VX_ERROR_INVALID_VALUE;

    if (!vxoMemory_Allocate(remap->base.context, &remap->memory)) return VX_ERROR_NO_MEMORY;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0);
    *ptr = src_x;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0);
    *ptr = src_y;

    vxoReference_IncrementWriteCount(&remap->base);

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetRemapPoint(
        vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 *src_x, vx_float32 *src_y)
{
    vx_float32_ptr ptr;

    gcmDUMP_API("$VX vxGetRemapPoint: remap=%p, dst_x=0x%x, dst_y=0x%x, src_x=%p, src_y=%p", remap, dst_x, dst_y, src_x, src_y);
    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    if (dst_x >= remap->destWidth || dst_y >= remap->destHeight) return VX_ERROR_INVALID_VALUE;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0);
    *src_x = *ptr;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0);
    *src_y = *ptr;

    vxoReference_IncrementReadCount(&remap->base);

    return VX_SUCCESS;
}

static vx_status vxSetCoordValue(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 src_x, vx_float32 src_y)
{
    vx_status status = VX_FAILURE;
    if ((vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP) == vx_true_e) &&
         (vxoMemory_Allocate(remap->base.context, &remap->memory) == vx_true_e))
    {
        if ((dst_x < remap->destWidth) &&
            (dst_y < remap->destHeight))
        {
            vx_float32 *coords[] = {
                 (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0),
                 (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0),
            };
            *coords[0] = src_x;
            *coords[1] = src_y;
            vxoReference_IncrementWriteCount(&remap->base);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

static vx_bool ownIsValidRemap(vx_remap remap)
{
    if (vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP) == vx_true_e)
    {
        return vx_true_e;
    }
    else
    {
        return vx_false_e;
    }
}

static vx_status vxGetCoordValue(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 *src_x, vx_float32 *src_y)
{
    vx_status status = VX_FAILURE;
    if (vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP) == vx_true_e)
    {
        if ((dst_x < remap->destWidth) &&
            (dst_y < remap->destHeight))
        {
            vx_float32 *coords[] = {
                 (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0),
                 (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0),
            };
            *src_x = *coords[0];
            *src_y = *coords[1];
            remap->base.readCount++;
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_remap VX_API_CALL vxCreateVirtualRemap(
    vx_graph graph,
    vx_uint32 src_width,
    vx_uint32 src_height,
    vx_uint32 dst_width,
    vx_uint32 dst_height)
{
    vx_remap remap = NULL;
    vx_reference_s *gref = (vx_reference_s *)graph;

    gcmDUMP_API("$VX vxCreateVirtualRemap: graph=%p, src_width=0x%x, src_height=0x%x, dst_width=0x%x, dst_height=0x%x",
        graph, src_width, src_height, dst_width, dst_height);
    if (vxoReference_IsValidAndSpecific(gref, VX_TYPE_GRAPH) == vx_true_e)
    {
        remap = vxCreateRemap(gref->context, src_width, src_height, dst_width, dst_height);
        if (vxGetStatus((vx_reference)remap) == VX_SUCCESS && remap->base.type == VX_TYPE_REMAP)
        {
            remap->base.scope = (vx_reference_s *)graph;
            remap->base.isVirtual = vx_true_e;
        }
    }
    /* else, the graph is invalid, we can't get any context and then error object */
    return remap;
}

VX_API_ENTRY vx_status VX_API_CALL vxMapRemapPatch(
    vx_remap remap,
    const vx_rectangle_t *rect,
    vx_map_id *map_id,
    vx_size *stride_y,
    void **ptr,
    vx_enum coordinate_type,
    vx_enum usage,
    vx_enum mem_type)
{
    vx_memory_map_extra_s extra;
    vx_size stride;
    vx_size size;
    vx_size user_stride_y;
    vx_uint32 flags = 0;
    vx_uint8 *buf = NULL;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x   = rect ? rect->end_x : 0u;
    vx_uint32 end_y   = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);
    vx_status status = VX_FAILURE;

    gcmDUMP_API("$VX vxMapRemapPatch: remap=%p, rect=%p, map_id=%p, stride_y=%p, ptr=%p, coordinate_type=0x%x, usage=0x%x, mem_type=0x%x",
        remap, rect, map_id, stride_y, ptr, coordinate_type, usage, mem_type);

    /* bad parameters */
    if ((rect == NULL) || (map_id == NULL) || (remap == NULL) || (ptr == NULL) )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* more bad parameters */
    if(coordinate_type != VX_TYPE_COORDINATES2DF)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if (ownIsValidRemap(remap) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (remap->base.isVirtual == vx_true_e)
    {
        if (remap->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" remap. */
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual remap, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        ((0 >= remap->memory.nodePtrs) ||
        (start_x >= end_x) ||
        (start_y >= end_y)))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* MAP mode */
    extra.image_data.plane_index = 0;
    extra.image_data.rect = *rect;

    stride = (end_x - start_x);
    size = (stride * (end_y - start_y)) * sizeof(vx_coordinates2df_t);
    user_stride_y = stride * sizeof(vx_coordinates2df_t);

    if (vxoContext_MemoryMap(remap->base.context, (vx_reference)remap, size, usage, mem_type, flags, &extra, (void **)&buf, map_id) == vx_true_e)
    {
        if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
        {

            if (vxAcquireMutex(remap->base.lock) == vx_true_e)
            {
                vx_uint32 i;
                vx_uint32 j;
                vx_coordinates2df_t *buf_ptr = (vx_coordinates2df_t *)buf;
                *stride_y = user_stride_y;
                for (i = start_y; i < end_y; i++)
                {
                    for (j = start_x; j < end_x; j++)
                    {
                        vx_coordinates2df_t *coord_ptr = &(buf_ptr[i * stride + j]);
                        status = vxGetCoordValue(remap, j, i, &coord_ptr->x, &coord_ptr->y);
                        if(status != VX_SUCCESS)
                        {
                            goto exit;
                        }
                    }
                }

                *ptr = buf;
                vxoReference_Increment(&remap->base, VX_REF_EXTERNAL);
                vxReleaseMutex(remap->base.lock);

                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }
        else
        {
            /* write only mode */
            *stride_y = user_stride_y;
            *ptr = buf;
            vxoReference_Increment(&remap->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
    }
    else
    {
        status = VX_FAILURE;
        goto exit;
    }
exit:
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapRemapPatch(vx_remap remap, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;
    vx_context context;
    vx_memory_map_s* map;

    gcmDUMP_API("$VX vxUnmapRemapPatch: remap=%p, map_id=0x%x", remap, map_id);

    /* bad references */
    if (ownIsValidRemap(remap) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(remap->base.context, (vx_reference)remap, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    context = remap->base.context;
    map = &context->memoryMaps[map_id];
    if (map->used && map->ref == (vx_reference)remap)
    {
        vx_rectangle_t rect = map->extra.image_data.rect;
        if (VX_WRITE_ONLY == map->usage || VX_READ_AND_WRITE == map->usage)
        {
            vx_size stride = (rect.end_x - rect.start_x);
            vx_coordinates2df_t *ptr = (vx_coordinates2df_t *)map->logical;
            vx_uint32 i;
            vx_uint32 j;
            for (i = rect.start_y; i < rect.end_y; i++)
            {
                for (j = rect.start_x; j < rect.end_x; j++)
                {
                    vx_coordinates2df_t *coord_ptr = &(ptr[i * stride + j]);
                    status = vxSetCoordValue(remap, j, i, coord_ptr->x, coord_ptr->y);
                    if(status != VX_SUCCESS)
                    {
                        goto exit;
                    }
                }
            }

            vxoContext_MemoryUnmap(context, (vx_uint32)map_id);
            vxoReference_Decrement(&remap->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            /* rean only mode */
            vxoContext_MemoryUnmap(remap->base.context, (vx_uint32)map_id);
            vxoReference_Decrement(&remap->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
    }
    else
    {
        status = VX_FAILURE;
        goto exit;
    }
exit:
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyRemapPatch(
    vx_remap remap,
    const vx_rectangle_t *rect,
    vx_size user_stride_y,
    void * user_ptr,
    vx_enum user_coordinate_type,
    vx_enum usage,
    vx_enum user_mem_type)
{
    vx_size stride;
    vx_status status = VX_FAILURE;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);

    gcmDUMP_API("$VX vxCopyRemapPatch: remap=%p, rect=%p, user_stride_y=%p, user_ptr=%p, user_coordinate_type=0x%x, usage=0x%x, user_mem_type=0x%x",
        remap, rect, user_stride_y, user_ptr, user_coordinate_type, usage, user_mem_type);

    /* bad parameters */
    if (((VX_READ_ONLY != usage) && (VX_WRITE_ONLY != usage)) ||
         (rect == NULL) || (remap == NULL) || (user_ptr == NULL) )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* more bad parameters */
    if((user_stride_y < sizeof(vx_coordinates2df_t)*(rect->end_x - rect->start_x)) ||
        (user_coordinate_type != VX_TYPE_COORDINATES2DF))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* more bad parameters */
    if (user_mem_type != VX_MEMORY_TYPE_HOST && user_mem_type != VX_MEMORY_TYPE_NONE)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if (ownIsValidRemap(remap) == vx_false_e )
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (remap->base.isVirtual == vx_true_e)
    {
        if (remap->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" remap. */
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual remap, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        ((0 >= remap->memory.nodePtrs) ||
         (start_x >= end_x) ||
         (start_y >= end_y)))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    stride = user_stride_y / sizeof(vx_coordinates2df_t);

    if (usage == VX_READ_ONLY)
    {
        /* Copy from remap (READ) mode */
        vx_coordinates2df_t *ptr = (vx_coordinates2df_t*)user_ptr;
        vx_uint32 i;
        vx_uint32 j;
        for (i = start_y; i < end_y; i++)
        {
            for (j = start_x; j < end_x; j++)
            {
                vx_coordinates2df_t *coord_ptr = &(ptr[i * stride + j]);
                status = vxGetCoordValue(remap, j, i, &coord_ptr->x, &coord_ptr->y);
                if(status != VX_SUCCESS)
                {
                    goto exit;
                }

            }
        }
    }
    else
    {
        /* Copy to remap (WRITE) mode */
        vx_coordinates2df_t *ptr = (vx_coordinates2df_t*)user_ptr;
        vx_uint32 i;
        vx_uint32 j;
        for (i = start_y; i < end_y; i++)
        {
            for (j = start_x; j < end_x; j++)
            {
                vx_coordinates2df_t *coord_ptr = &(ptr[i * stride + j]);
                status = vxSetCoordValue(remap, j, i, coord_ptr->x, coord_ptr->y);
                if(status != VX_SUCCESS)
                {
                    goto exit;
                }

            }
        }
    }
    status = VX_SUCCESS;
exit:
    return status;
}

