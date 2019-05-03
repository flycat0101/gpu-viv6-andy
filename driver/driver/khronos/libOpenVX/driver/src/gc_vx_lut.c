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

#define _GC_OBJ_ZONE            gcdZONE_VX_LUT

VX_INTERNAL_CALLBACK_API void vxoLUT_Destructor(vx_reference ref)
{
    vxoArray_Destructor(ref);
}

VX_PRIVATE_API vx_status gcoLUT_CopyArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_size stride, void *ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("arr=%p, start=0x%lx, end=0x%lx, stride=0x%lx, ptr=%p, usage=0x%x, mem_type=0x%x", arr, start, end, stride, ptr, usage, mem_type);

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (VX_WRITE_ONLY != usage)) ||
         (ptr == NULL) || (stride < arr->itemSize) ||
         (start >= end) || (end > arr->itemCount))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* determine if virtual before checking for memory */
    if (arr->base.isVirtual == vx_true_e)
    {
        if (arr->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            vxError("Can not access a virtual array\n");
            gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual array, this is ok. */
    }

    /* verify has not run or will not run yet. this allows this API to "touch"
     * the array to create it.
     */
    if (vxoArray_AllocateMemory(arr) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
        return VX_ERROR_NO_MEMORY;
    }
    {
    vx_size i = 0;
    vx_size offset = start * arr->itemSize;
    if (usage == VX_READ_ONLY)
    {
        vxInfo("CopyArrayRange from "VX_FMT_REF" to ptr %p from %u to %u\n", arr, ptr, start, end);
        {
            vx_uint8 *pSrc = (vx_uint8 *)&arr->memory.logicals[0][offset];
        vx_uint8 *pDst = (vx_uint8 *)ptr;
        if (stride == arr->itemSize)
        {
            vx_size size = (end - start) * arr->itemSize;
            memcpy(pDst, pSrc, size);
        }
        else
        {
            /* The source is not compact, we need to copy per element */
            for (i = start; i < end; i++)
            {
                memcpy(pDst, pSrc, arr->itemSize);
                pDst += stride;
                pSrc += arr->itemSize;
            }
        }

        vxoReference_IncrementReadCount(&arr->base);

        status = VX_SUCCESS;
        }
    }
    else
    {
        vxInfo("CopyArrayRange from ptr %p to "VX_FMT_REF" from %u to %u\n", arr, ptr, start, end);

        if (vxAcquireMutex(arr->memory.writeLocks[0]) == vx_true_e)
        {
            vx_uint8 *pSrc = (vx_uint8 *)ptr;
            vx_uint8 *pDst = (vx_uint8 *)&arr->memory.logicals[0][offset];
            if (stride == arr->itemSize)
            {
                vx_size size = (end - start) * arr->itemSize;
                memcpy(pDst, pSrc, size);
            }
            else
            {
                /* The source is not compact, we need to copy per element */
                for (i = start; i < end; i++)
                {
                    memcpy(pDst, pSrc, arr->itemSize);
                    pDst += arr->itemSize;
                    pSrc += stride;
                }
            }

            vxoReference_IncrementWriteCount(&arr->base);
            vxReleaseMutex(arr->memory.writeLocks[0]);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

#if MAP_UNMAP_REFERENCE
VX_PRIVATE_API vx_status gcoLUT_MapArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_map_id *map_id, vx_size *stride,
                             void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL) ||
        (start >= end) || (end > arr->itemCount))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* determine if virtual before checking for memory */
    if (arr->base.isVirtual == vx_true_e)
    {
        if (arr->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            vxError("Can not access a virtual array\n");
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual array, this is ok. */
    }

    /* verify has not run or will not run yet. this allows this API to "touch"
     * the array to create it.
     */
    if (vxoArray_AllocateMemory(arr) == vx_false_e)
    {
        return VX_ERROR_NO_MEMORY;
    }

    vxInfo("MapArrayRange from "VX_FMT_REF" to ptr %p from %u to %u\n", arr, *ptr, start, end);
    {
    vx_memory_map_extra_s extra;
    vx_uint8 *buf = NULL;
    vx_size size = (end - start) * arr->itemSize;
    extra.array_data.start = start;
    extra.array_data.end = end;
    if (vxoContext_MemoryMap(arr->base.context, (vx_reference)arr, size, usage, mem_type, flags, &extra, (void **)&buf, map_id) == vx_true_e)
    {
        if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
        {
            if (vxAcquireMutex(arr->memory.writeLocks[0]) == vx_true_e)
            {
                vx_size offset = start * arr->itemSize;
                vx_uint8 *pSrc = (vx_uint8 *)&arr->memory.logicals[0][offset];
                vx_uint8 *pDst = (vx_uint8 *)buf;
                *stride = arr->itemSize;

                memcpy(pDst, pSrc, size);

                *ptr = buf;
                vxoReference_Increment(&arr->base, VX_REF_EXTERNAL);
                vxReleaseMutex(arr->memory.writeLocks[0]);

                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            /* write only mode */
            *stride = arr->itemSize;
            *ptr = buf;
            vxoReference_Increment(&arr->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
    }
    else
    {
        status = VX_FAILURE;
    }
    }
    return status;
}

VX_PRIVATE_API vx_status gcoLUT_UnmapArrayRangeInt(vx_array arr, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;

    /* determine if virtual before checking for memory */
    if (arr->base.isVirtual == vx_true_e)
    {
        if (arr->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            vxError("Can not access a virtual array\n");
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual array, this is ok. */
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(arr->base.context, (vx_reference)arr, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap array range\n");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("UnmapArrayRange from "VX_FMT_REF"\n", arr);
    {
    vx_context context = arr->base.context;
    vx_memory_map_s* map = &context->memoryMaps[map_id];
    if (map->used && map->ref == (vx_reference)arr)
    {
        vx_size start = map->extra.array_data.start;
        vx_size end = map->extra.array_data.end;
        if (VX_WRITE_ONLY == map->usage || VX_READ_AND_WRITE == map->usage)
        {
            if (vxAcquireMutex(arr->memory.writeLocks[0]) == vx_true_e)
            {
                vx_size offset = start * arr->itemSize;
                vx_uint8 *pSrc = (vx_uint8 *)map->logical;
                vx_uint8 *pDst = (vx_uint8 *)&arr->memory.logicals[0][offset];
                vx_size size = (end - start) * arr->itemSize;
                memcpy(pDst, pSrc, size);

                vxoContext_MemoryUnmap(context, map_id);
                vxoReference_Decrement(&arr->base, VX_REF_EXTERNAL);
                vxReleaseMutex(arr->memory.writeLocks[0]);
                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            /* rean only mode */
            vxoContext_MemoryUnmap(arr->base.context, map_id);
            vxoReference_Decrement(&arr->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
    }
    else
    {
        status = VX_FAILURE;
    }

    return status;
    }
}
#else
VX_PRIVATE_API vx_status gcoLUT_MapArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_map_id *map_id, vx_size *stride,
                             void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("arr=%p, start=0x%lx, end=0x%lx, map_id=%p, stride=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        arr, start, end, map_id, stride, ptr, usage, mem_type, flags);

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL) ||
        (start >= end) || (end > arr->itemCount))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* determine if virtual before checking for memory */
    if (arr->base.isVirtual == vx_true_e)
    {
        if (arr->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            vxError("Can not access a virtual array\n");
            gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual array, this is ok. */
    }

    /* verify has not run or will not run yet. this allows this API to "touch"
     * the array to create it.
     */
    if (vxoArray_AllocateMemory(arr) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
        return VX_ERROR_NO_MEMORY;
    }

    vxInfo("MapArrayRange from "VX_FMT_REF" to ptr %p from %u to %u\n", arr, *ptr, start, end);
    {
        vx_memory_map_extra_s extra;
        vx_uint8 *buf = NULL;
        vx_size size = (end - start) * arr->itemSize;
        extra.array_data.start = start;
        extra.array_data.end = end;
        if (vxoContext_MemoryMap(arr->base.context, (vx_reference)arr, size, usage, mem_type, flags, &extra, (void **)&buf, map_id) == vx_true_e)
        {
            /* write only mode */
            *stride = arr->itemSize;
            *ptr = buf;
            vxoReference_Increment(&arr->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status gcoLUT_UnmapArrayRangeInt(vx_array arr, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("arr=%p, map_id=0x%x", arr, map_id);

    /* determine if virtual before checking for memory */
    if (arr->base.isVirtual == vx_true_e)
    {
        if (arr->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            vxError("Can not access a virtual array\n");
            gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual array, this is ok. */
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(arr->base.context, (vx_reference)arr, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap array range\n");
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("UnmapArrayRange from "VX_FMT_REF"\n", arr);
    {
        vx_context context = arr->base.context;
        vx_memory_map_s* map = &context->memoryMaps[map_id];
        if (map->used && map->ref == (vx_reference)arr)
        {
            /* rean only mode */
            vxoContext_MemoryUnmap(arr->base.context, map_id);
            vxoReference_Decrement(&arr->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

VX_API_ENTRY vx_lut VX_API_CALL vxCreateLUT(vx_context context, vx_enum data_type, vx_size count)
{
    vx_array lut;

    gcmHEADER_ARG("context=%p, data_type=0x%x, count=0x%lx", context, data_type, count);
    gcmDUMP_API("$VX vxCreateLUT: context=%p, data_type=0x%x, count=0x%lx", context, data_type, count);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    switch (data_type)
    {
        case VX_TYPE_UINT8:
        case VX_TYPE_UINT16:
        case VX_TYPE_INT16:
            break;

        default:
            gcmFOOTER_NO();
            return (vx_lut)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    lut = vxoArray_Create(context, data_type, count, vx_false_e, VX_TYPE_LUT, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)lut) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("lut=%p", lut);
        return (vx_lut)lut;
    }
    lut->itemCount = count;

    lut->offset       = (data_type == VX_TYPE_INT16)?((vx_uint32)(count/2)):0;

    gcmFOOTER_ARG("lut=%p", lut);
    return (vx_lut)lut;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseLUT(vx_lut *lut)
{
    gcmHEADER_ARG("lut=%p", lut);
    gcmDUMP_API("$VX vxReleaseLUT: lut=%p", lut);

    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)lut, VX_TYPE_LUT, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryLUT(vx_lut lut, vx_enum attribute, void *ptr, vx_size size)
{
    vx_array lutArray = (vx_array)lut;

    gcmHEADER_ARG("lut=%p, attribute=0x%x, ptr=%p, size=0x%lx", lut, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryLUT: lut=%p, attribute=0x%x, ptr=%p, size=0x%lx", lut, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_LUT_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = lutArray->itemType;
            break;

        case VX_LUT_COUNT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = lutArray->itemCount;
            break;

        case VX_LUT_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = lutArray->itemCount * lutArray->itemSize;
            break;

        case VX_LUT_OFFSET:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = lutArray->offset;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessLUT(vx_lut lut, void **ptr, vx_enum usage)
{
    vx_array lutArray = (vx_array)lut;

    gcmHEADER_ARG("lut=%p, ptr=%p, usage=0x%x", lut, ptr, usage);
    gcmDUMP_API("$VX vxAccessLUT: lut=%p, ptr=%p, usage=0x%x", lut, ptr, usage);

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    gcmFOOTER_NO();
    return vxoArray_AccessRange(lutArray, 0, lutArray->itemCount, VX_NULL, ptr, usage);
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitLUT(vx_lut lut, const void *ptr)
{
    vx_array lutArray = (vx_array)lut;

    gcmHEADER_ARG("lut=%p, ptr=%p", lut, ptr);
    gcmDUMP_API("$VX vxCommitLUT: lut=%p, ptr=%p", lut, ptr);

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_NO();
    return vxoArray_CommitRange(lutArray, 0, lutArray->itemCount, (vx_ptr)ptr);
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyLUT(vx_lut l, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_FAILURE;
    vx_lut_s *lut = (vx_lut_s *)l;

    gcmHEADER_ARG("l=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x", l, user_ptr, usage, user_mem_type);
    gcmDUMP_API("$VX vxCopyLUT: l=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x", l, user_ptr, usage, user_mem_type);
    if (vxoReference_IsValidAndSpecific(&lut->base, VX_TYPE_LUT))
    {
        vx_size stride = lut->itemSize;
        status = gcoLUT_CopyArrayRangeInt((vx_array_s *)l, 0, lut->itemCount, stride, user_ptr, usage, user_mem_type);
    }
    else
    {
        vxError("Not a valid object!\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxMapLUT(vx_lut l, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    vx_status status = VX_FAILURE;
    vx_lut_s *lut = (vx_lut_s *)l;

    gcmHEADER_ARG("l=%p, map_id=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", l, map_id, ptr, usage, mem_type, flags);
    gcmDUMP_API("$VX vxMapLUT: l=%p, map_id=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", l, map_id, ptr, usage, mem_type, flags);
    if (vxoReference_IsValidAndSpecific(&lut->base, VX_TYPE_LUT))
    {
        vx_size stride = lut->itemSize;
        status = gcoLUT_MapArrayRangeInt((vx_array_s *)lut, 0, lut->itemCount, map_id, &stride, ptr, usage, mem_type, flags);
    }
    else
    {
        vxError("Not a valid object!\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapLUT(vx_lut l, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;
    vx_lut_s *lut = (vx_lut_s *)l;

    gcmHEADER_ARG("l=%p, map_id=0x%x", l, map_id);
    gcmDUMP_API("$VX vxUnmapLUT: l=%p, map_id=0x%x", l, map_id);

    if (vxoReference_IsValidAndSpecific(&lut->base, VX_TYPE_LUT))
    {
        status = gcoLUT_UnmapArrayRangeInt((vx_array_s *)l, map_id);
    }
    else
    {
        vxError("Not a valid object!\n");
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_lut VX_API_CALL vxCreateVirtualLUT(vx_graph graph, vx_enum data_type, vx_size count)
{
    vx_array lut;
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);

    gcmHEADER_ARG("graph=%p, data_type=0x%x, count=0x%lx", graph, data_type, count);
    gcmDUMP_API("$VX vxCreateVirtualLUT: graph=%p, data_type=0x%x, count=0x%lx", graph, data_type, count);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    lut = (vx_array)vxCreateLUT(context, data_type, count);

    if (vxoReference_GetStatus((vx_reference)lut) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("lut=%p", lut);
        return (vx_lut)lut;
    }
    lut->base.scope        = (vx_reference)graph;

    lut->base.isVirtual    = vx_true_e;

    gcmFOOTER_ARG("lut=%p", lut);
    return (vx_lut)lut;
}

