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

#define _GC_OBJ_ZONE            gcdZONE_VX_ARRAY

VX_PRIVATE_API vx_size vxoArray_GetItemSize(vx_context context, vx_enum itemType)
{
    vx_uint32 i;
    vx_size size;

    gcmHEADER_ARG("context=%p, itemType=0x%x", context, itemType);
    vxmASSERT(context);

    size = vxDataType_GetSize(itemType);

    if (size != 0)
    {
        gcmFOOTER_ARG("0x%lx", size);
        return size;
    }
    /* The data type should be user structs */
    for (i = 0; i < VX_MAX_USER_STRUCT_COUNT; i++)
    {
        if (context->userStructTable[i].type == itemType)
        {
            gcmFOOTER_NO();
            return context->userStructTable[i].size;
        }
    }

    gcmFOOTER_NO();
    return 0;
}

VX_PRIVATE_API vx_bool vxoArray_IsValidItemType(vx_context context, vx_enum itemType)
{
    return (vx_bool)(vxoArray_GetItemSize(context, itemType) != 0);
}

VX_PRIVATE_API vx_bool vxoArray_IsValid(vx_array array)
{
    gcmHEADER_ARG("array=%p", array);
    if (!vxoReference_IsValidAndSpecific(&array->base, VX_TYPE_ARRAY))
    {
        vxError("%s[%d]: Array reference is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Array reference is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    gcmFOOTER_NO();
    return vxoArray_IsValidItemType(array->base.context, array->itemType);
}

VX_PRIVATE_API void vxoArray_InitializeMemory(vx_array array)
{
    vx_uint32 i;
    gcmHEADER_ARG("array=%p", array);
    array->memory.planeCount    = 1;
    array->memory.dimCount      = 2;
    array->memory.dims[0][0]    = (int32_t)array->itemSize;
    array->memory.dims[0][1]    = (int32_t)array->capacity;

    for (i = 0; i < array->memory.planeCount; i++)
    {
        array->memory.sizes[i] = 0;
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoArray_Dump(vx_array array)
{
    gcmHEADER_ARG("array=%p", array);
    if (array == VX_NULL)
    {
        vxTrace(VX_TRACE_ARRAY, "<array>null</array>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)array);

        vxTrace(VX_TRACE_ARRAY,
                "<array>\n"
                "   <address>%p</address>\n"
                "   <itemType>%d</itemType>\n"
                "   <itemSize>%ul</itemSize>\n"
                "   <itemCount>%ul</itemCount>\n"
                "   <capacity>%ul</capacity>\n"
                "</array>",
                array, array->itemType, array->itemSize, array->itemCount, array->capacity);
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_array vxoArray_Create(
    vx_context context, vx_enum itemType, vx_size capacity, vx_bool isVirtual, vx_enum type, vx_bool isVivArray)
{
    vx_array array;
    gcmHEADER_ARG("context=%p, itemType=0x%x, capacity=0x%lx, isVirtual=0x%x, type=0x%x, isVivArray=0x%x",
        context, itemType, capacity, isVirtual, type, isVivArray);
    vxmASSERT(context);
    vxmASSERT(type == VX_TYPE_ARRAY || type == VX_TYPE_LUT);

    array = (vx_array)vxoReference_Create(context, type, VX_REF_EXTERNAL, &context->base);;

    if (vxoReference_GetStatus((vx_reference)array) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("array=%p", array);
        return array;
    }
    array->itemType         = itemType;
    array->itemSize         = vxoArray_GetItemSize(context, itemType);
    array->capacity         = capacity;
    array->base.isVirtual   = isVirtual;
    array->isVivArray       = isVivArray;

    vxoArray_InitializeMemory(array);

    gcmFOOTER_ARG("array=%p", array);
    return array;
}

VX_INTERNAL_API void vxoArray_Destroy(vx_array* array)
{
    vxoMemory_Free((*array)->base.context, &(*array)->memory);

    *array = VX_NULL;
}

VX_INTERNAL_CALLBACK_API void vxoArray_Destructor(vx_reference ref)
{
    vx_array array = (vx_array)ref;
    gcmHEADER_ARG("ref=%p", ref);

    vxoMemory_Free(array->base.context, &array->memory);

    if (array->memAllocInfo.node)
    {
        gcoVX_FreeMemory(array->memAllocInfo.node);
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoArray_InitializeAsVirtual(vx_array array, vx_enum itemType, vx_size capacity)
{
    gcmHEADER_ARG("array=%p, itemType=0x%x, capacity=0x%lx", array, itemType, capacity);
    vxmASSERT(array);

    if (!vxoArray_IsValidItemType(array->base.context, itemType))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (array->itemType != VX_TYPE_INVALID && array->itemType != itemType)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    array->itemType = itemType;
    array->itemSize = vxoArray_GetItemSize(array->base.context, itemType);

    if (array->capacity == 0) array->capacity = capacity;

    vxoArray_InitializeMemory(array);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoArray_CheckItemTypeAndCapacity(vx_array array, vx_enum itemType, vx_size capacity)
{
    gcmHEADER_ARG("array=%p, itemType=0x%x, capacity=0x%lx", array, itemType, capacity);
    vxmASSERT(array);

    if (!vxoArray_IsValidItemType(array->base.context, itemType))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (array->itemType != itemType)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (capacity > 0)
    {
        if (capacity > array->capacity)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
    }
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoArray_AllocateMemory(vx_array array)
{
    if (array->capacity == 0) return vx_false_e;

    return vxoMemory_Allocate(array->base.context, &array->memory);
}

VX_INTERNAL_API vx_bool vxoArray_FreeMemory(vx_array array)
{
    return vxoMemory_Free(array->base.context, &array->memory);
}

VX_INTERNAL_API vx_bool vxoArray_AllocateMemoryEx(vx_array array)
{
    if (array->capacity == 0) return vx_false_e;

    return vxoMemory_AllocateEx(array->base.context, &array->memory);
}

VX_INTERNAL_API vx_bool vxoArray_FreeMemoryEx(vx_array array)
{
    return vxoMemory_FreeEx(array->base.context, &array->memory);
}

VX_INTERNAL_API vx_status vxoArray_AccessRange(
        vx_array array, vx_size start, vx_size end, vx_size* pStride, vx_ptr_ptr ptrPtr, vx_enum usage)
{
    gcmHEADER_ARG("array=%p, start=0x%lx, end=0x%lx, pStride=%p, ptrPtr=%p, usage=0x%x", array, start, end, pStride, ptrPtr, usage);
    vxmASSERT(array);

    if (usage < VX_READ_ONLY || VX_READ_AND_WRITE < usage)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (ptrPtr == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (start >= end || end > array->itemCount)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (array->base.isVirtual && !array->base.accessible)
    {
        vxError("The virtual array, %p, is unaccessible", array);
        gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
        return VX_ERROR_OPTIMIZED_AWAY;
    }

    if (!vxoArray_AllocateMemory(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
        return VX_ERROR_NO_MEMORY;
    }
    if (*ptrPtr == VX_NULL && (usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE))
    {
        if (!vxAcquireMutex(array->memory.writeLocks[0]))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
            return VX_ERROR_NO_RESOURCES;
        }
        *ptrPtr = &array->memory.logicals[0][start * array->itemSize];
    }
    else
    {
        vx_size *strideSave;
        vx_size size = (end - start) * array->itemSize;
        vx_uint32 index;
        strideSave = vxAllocate(sizeof(vx_size));

        if (pStride == NULL) {
            *strideSave = array->itemSize;
            pStride = strideSave;
        }
        else {
            *strideSave = *pStride;
        }

        if (!vxoContext_AddAccessor(array->base.context, size, usage, *ptrPtr, &array->base, OUT &index, strideSave))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
            return VX_ERROR_NO_MEMORY;
        }

        *ptrPtr = array->base.context->accessorTable[index].ptr;

        if (usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE)
        {
            if (!vxAcquireMutex(array->memory.writeLocks[0]))
            {
                gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
                return VX_ERROR_NO_RESOURCES;
            }
        }

        if (usage != VX_WRITE_ONLY)
        {
            vx_size i;
            vx_uint8 *pSrc, *pDest;
            vx_size stride = *pStride;
            if (stride == array->itemSize)
            {
                vxMemCopy(*ptrPtr, &array->memory.logicals[0][start * array->itemSize], size);
            }
            else
            {
                for (i = start, pDest = (vx_uint8*)(*ptrPtr), pSrc = &array->memory.logicals[0][start * array->itemSize];
                     i < end;
                     i++, pDest += stride, pSrc += array->itemSize)
                {
                    vxMemCopy(pDest, pSrc, array->itemSize);
                }
            }
        }
    }

    vxoReference_IncrementReadCount(&array->base);

    vxoReference_Increment(&array->base, VX_REF_EXTERNAL);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoArray_CommitRange(vx_array array, vx_size start, vx_size end, vx_ptr ptr)
{
    vx_uint32 index;
    vx_bool foundAccessor;

    gcmHEADER_ARG("array=%p, start=0x%lx, end=0x%lx, ptr=%p", array, start, end, ptr);
    vxmASSERT(array);

    if (ptr == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (start > end || end > array->itemCount)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (array->base.isVirtual && !array->base.accessible)
    {
        vxError("The virtual array, %p, is unaccessible", array);
        gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
        return VX_ERROR_OPTIMIZED_AWAY;
    }

    foundAccessor = vxoContext_SearchAccessor(array->base.context, ptr, OUT &index);

    if (end == 0)
    {
        if (foundAccessor)
        {
            vxoContext_RemoveAccessor(array->base.context, index);
        }
        else
        {
            vxReleaseMutex(array->memory.writeLocks[0]);
        }
    }
    else
    {
        if (foundAccessor && array->base.context->accessorTable[index].usage == VX_READ_ONLY)
        {
            vxoContext_RemoveAccessor(array->base.context, index);
        }
        else
        {
            vx_uint8_ptr memoryPtr      = array->memory.logicals[0];
            vx_uint8_ptr memoryEndPtr   = &memoryPtr[array->itemSize * array->itemCount];
            vx_bool isExternal          = vx_true_e;

            if (memoryPtr <= (vx_uint8_ptr)ptr && (vx_uint8_ptr)ptr < memoryEndPtr)
            {
                isExternal = vx_false_e;
            }

            if (isExternal || foundAccessor)
            {
                vx_size stride = *(vx_size *)array->base.context->accessorTable[index].extraDataPtr;

                if (stride == array->itemSize)
                {
                    vxMemCopy(&memoryPtr[start * array->itemSize], ptr, (end - start) * array->itemSize);
                }
                else
                {
                    vx_size i;
                    const vx_uint8 *pSrc; vx_uint8 *pDest;

                    for (i = start, pSrc = ptr, pDest= &memoryPtr[start * array->itemSize];
                         i < end;
                         i++, pSrc += stride, pDest += array->itemSize)
                    {
                        vxMemCopy(pDest, pSrc, array->itemSize);
                    }
                }

                if (foundAccessor) vxoContext_RemoveAccessor(array->base.context, index);
            }

            vxoReference_IncrementWriteCount(&array->base);

            vxReleaseMutex(array->memory.writeLocks[0]);
        }
    }

    vxoReference_Decrement(&array->base, VX_REF_EXTERNAL);
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoArray_CopyArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_size stride, void *ptr, vx_enum usage, vx_enum mem_type)
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
VX_INTERNAL_API vx_status vxoArray_MapArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_map_id *map_id, vx_size *stride,
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

VX_INTERNAL_API vx_status vxoArray_UnmapArrayRangeInt(vx_array arr, vx_map_id map_id)
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
VX_INTERNAL_API vx_status vxoArray_MapArrayRangeInt(vx_array arr, vx_size start, vx_size end, vx_map_id *map_id, vx_size *stride,
                             void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    vx_status status = VX_FAILURE;
    gcmHEADER_ARG("arr=%p, start=0x%lx, end=0x%lx, map_id=%p, stride=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", arr, start, end, map_id, stride, ptr, usage, mem_type, flags);

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

VX_INTERNAL_API vx_status vxoArray_UnmapArrayRangeInt(vx_array arr, vx_map_id map_id)
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

VX_API_ENTRY vx_array VX_API_CALL vxCreateArray(vx_context context, vx_enum itemType, vx_size capacity)
{
    gcmHEADER_ARG("context=%p, itemType=0x%x, capacity=0x%lx", context, itemType, capacity);
    gcmDUMP_API("$VX vxCreateArray: context=%p, itemType=0x%x, capacity=0x%lx", context, itemType, capacity);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoArray_IsValidItemType(context, itemType) || (capacity <= 0))
    {
        gcmFOOTER_NO();
        return (vx_array)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    gcmFOOTER_NO();
    return (vx_array)vxoArray_Create(context, itemType, capacity, vx_false_e, VX_TYPE_ARRAY, vx_false_e);
}

VX_API_ENTRY vx_array VX_API_CALL vxCreateVirtualArray(vx_graph graph, vx_enum itemType, vx_size capacity)
{
    vx_array array;

    gcmHEADER_ARG("graph=%p, itemType=0x%x, capacity=0x%lx", graph, itemType, capacity);
    gcmDUMP_API("$VX vxCreateVirtualArray: graph=%p, itemType=0x%x, capacity=0x%lx", graph, itemType, capacity);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        vxError("%s[%d]: Reference is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Reference is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (itemType != VX_TYPE_INVALID)
    {
        if (!vxoArray_IsValidItemType(graph->base.context, itemType))
        {
            gcmFOOTER_NO();
            return (vx_array)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    array = vxoArray_Create(graph->base.context, itemType, capacity, vx_true_e, VX_TYPE_ARRAY, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)array) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("array=%p", array);
        return array;
    }

    array->base.scope = (vx_reference)graph;

    gcmFOOTER_ARG("array=%p", array);
    return array;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseArray(vx_array *array)
{
    gcmHEADER_ARG("array=%p", array);
    gcmDUMP_API("$VX vxReleaseArray: array=%p", array);

    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)array, VX_TYPE_ARRAY, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryArray(vx_array array, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("array=%p, attribute=0x%lx, ptr=%p, size=0x%lx", array, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryArray: array=%p, attribute=0x%lx, ptr=%p, size=0x%lx", array, attribute, ptr, size);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_ARRAY_ITEMTYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = array->itemType;
            break;

        case VX_ARRAY_NUMITEMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = array->itemCount;
            break;

        case VX_ARRAY_CAPACITY:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = array->capacity;
            break;

        case VX_ARRAY_ITEMSIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = array->itemSize;
            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported", __FUNCTION__, __LINE__, attribute);
            vxAddLogEntry(&array->base, VX_ERROR_NOT_SUPPORTED, "%s[%d]: The attribute parameter, %d, is not supported!\n",
                __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetArrayAttribute(vx_array array, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("array=%p, attribute=0x%x, ptr=%p, size=0x%lx", array, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetArrayAttribute: array=%p, attribute=0x%x, ptr=%p, size=0x%lx", array, attribute, ptr, size);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_ARRAY_ITEMTYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            array->itemType = *(vx_enum *)ptr;
            break;

        case VX_ARRAY_NUMITEMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            array->itemCount = *(vx_size *)ptr;
            break;

        case VX_ARRAY_CAPACITY:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            array->capacity = *(vx_size *)ptr;
            break;

        case VX_ARRAY_ITEMSIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            array->itemSize = *(vx_size *)ptr;
            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported", __FUNCTION__, __LINE__, attribute);
            vxAddLogEntry(&array->base, VX_ERROR_NOT_SUPPORTED, "%s[%d]: The attribute parameter, %d, is not supported!\n",
                __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddArrayItems(vx_array array, vx_size count, const void *ptr, vx_size stride)
{
    vx_size         destOffset;
    vx_uint8_ptr    destPtr;

    gcmHEADER_ARG("array=%p, count=0x%lx, ptr=%p, stride=0x%lx", array, count, ptr, stride);
    gcmDUMP_API("$VX vxAddArrayItems: array=%p, count=0x%lx, ptr=%p, stride=0x%lx", array, count, ptr, stride);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (count <= 0)
    {
        vxError("%s[%d]: Array count <= 0 !\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Array count <= 0 !\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (ptr == VX_NULL)
    {
        vxError("%s[%d]: Parameter ptr is NULL!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Parameter ptr is NULL!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (stride != 0 && stride < array->itemSize)
    {
        vxError("%s[%d]: Parameter stride is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Parameter stride is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (array->itemCount + count > array->capacity)
    {
        vxError("%s[%d]: Parameter itemCount is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Parameter itemCount is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (!vxoArray_AllocateMemory(array))
    {
        vxError("%s[%d]: Allocate array memory failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Allocate array memory failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
        return VX_ERROR_NO_MEMORY;
    }

    destOffset  = array->itemCount * array->itemSize;
    destPtr     = &array->memory.logicals[0][destOffset];

    if (stride == 0 || stride == array->itemSize)
    {
        memcpy(destPtr, ptr, count * array->itemSize);
    }
    else
    {
        vx_size i;
        vx_uint8_ptr srcPtr = (vx_uint8_ptr)ptr;

        for (i = 0; i < count; ++i)
        {
            memcpy(&destPtr[i * array->itemSize], &srcPtr[i * stride], array->itemSize);
        }
    }

    array->itemCount += count;

    vxoReference_IncrementWriteCount(&array->base);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxTruncateArray(vx_array array, vx_size new_num_items)
{
    gcmHEADER_ARG("array=%p, new_num_items=0x%lx", array, new_num_items);
    gcmDUMP_API("$VX vxTruncateArray: array=%p, new_num_items=0x%lx", array, new_num_items);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (new_num_items > array->itemCount)
    {
        vxError("%s[%d]: Parameter itemCount is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Parameter itemCount is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    array->itemCount = new_num_items;

    vxoReference_IncrementWriteCount(&array->base);

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessArrayRange(
        vx_array array, vx_size start, vx_size end, vx_size *stride, void **ptr, vx_enum usage)
{
    vx_status status;

    gcmHEADER_ARG("array=%p, start=0x%lx, end=0x%lx, stride=%p, ptr=%p, usage=0x%x", array, start, end, stride, ptr, usage);
    gcmDUMP_API("$VX vxAccessArrayRange: array=%p, start=0x%lx, end=0x%lx, stride=%p, ptr=%p, usage=0x%x",
        array, start, end, stride, ptr, usage);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (stride == VX_NULL)
    {
        vxError("%s[%d]: Parameter stride is invalid!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&array->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Parameter stride is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (*ptr ==  VX_NULL)
    {
        *stride = array->itemSize;
    }

    status = vxoArray_AccessRange(array, start, end, stride, ptr, usage);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitArrayRange(vx_array array, vx_size start, vx_size end, const void *ptr)
{
    gcmHEADER_ARG("array=%p, start=0x%lx, end=0x%lx, ptr=%p", array, start, end, ptr);
    gcmDUMP_API("$VX vxCommitArrayRange: array=%p, start=0x%lx, end=0x%lx, ptr=%p", array, start, end, ptr);

    if (!vxoArray_IsValid(array))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_NO();
    return vxoArray_CommitRange(array, start, end, (vx_ptr)ptr);
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyArrayRange(vx_array arr, vx_size start, vx_size end, vx_size stride,
                                                    void *ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_FAILURE;
    gcmHEADER_ARG("arr=%p, start=0x%lx, end=0x%lx, stride=0x%lx, ptr=%p, usage=0x%x, mem_type=0x%x",
        arr, start, end, stride, ptr, usage, mem_type);
    gcmDUMP_API("$VX vxCopyArrayRange: arr=%p, start=0x%lx, end=0x%lx, stride=0x%lx, ptr=%p, usage=0x%x, mem_type=0x%x",
        arr, start, end, stride, ptr, usage, mem_type);

    if (!vxoArray_IsValid(arr))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    status = vxoArray_CopyArrayRangeInt(arr, start, end, stride, ptr, usage, mem_type);
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxMapArrayRange(vx_array arr, vx_size start, vx_size end, vx_map_id *map_id, vx_size *stride,
                                                   void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    vx_status status = VX_FAILURE;
    gcmHEADER_ARG("arr=%p, start=0x%lx, end=0x%lx, map_id=%p, stride=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        arr, start, end, map_id, stride, ptr, usage, mem_type, flags);
    gcmDUMP_API("$VX vxMapArrayRange: arr=%p, start=0x%lx, end=0x%lx, map_id=%p, stride=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        arr, start, end, map_id, stride, ptr, usage, mem_type, flags);

    if (!vxoArray_IsValid(arr))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    status = vxoArray_MapArrayRangeInt(arr, start, end, map_id, stride, ptr, usage, mem_type, flags);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapArrayRange(vx_array arr, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;
    gcmHEADER_ARG("arr=%p, map_id=%p", arr, map_id);
    gcmDUMP_API("$VX vxUnmapArrayRange: arr=%p, map_id=%p", arr, map_id);

    if (!vxoArray_IsValid(arr))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    status = vxoArray_UnmapArrayRangeInt(arr, map_id);

    gcmFOOTER_ARG("%d", status);
    return status;
}

