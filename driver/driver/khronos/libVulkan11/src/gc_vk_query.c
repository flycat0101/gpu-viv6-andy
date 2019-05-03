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


#include "gc_vk_precomp.h"

/* suppose maximum count is 8 under multi-GPU/multi-Cluster architecture.*/
#define MAX_UNIT_COUNT 8

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateQueryPool(
    VkDevice device,
    const VkQueryPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkQueryPool* pQueryPool
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkQueryPool *qyp;
    VkMemoryAllocateInfo mem_alloc;
    VkBufferCreateInfo buf_info;
    VkDeviceMemory memory;
    uint64_t *queryMemory;
    uint32_t iq;
    VkResult result;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    result = __vk_CreateObject(devCtx, __VK_OBJECT_QUERY_POOL, sizeof(__vkQueryPool), (__vkObject**)&qyp);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    /* maybe overflow maximum size of query pool.*/
    __VK_ASSERT(pCreateInfo->queryCount * MAX_UNIT_COUNT * sizeof(uint64_t)< __VK_MAX_QUERY_BUF_SIZE);

    /* Initialize __vkQueryPool specific data fields here */
    qyp->queryCount = pCreateInfo->queryCount;

    qyp->pQueries = (__vkQuery *)__VK_ALLOC((qyp->queryCount * sizeof(__vkQuery)), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!qyp->pQueries)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }
    __VK_MEMZERO(qyp->pQueries, (qyp->queryCount * sizeof(__vkQuery)));

    /* Create a VkBuffer for query memory */
    __VK_MEMZERO(&buf_info, sizeof(VkBufferCreateInfo));
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.size = __VK_MAX_QUERY_BUF_SIZE;

    qyp->queryBuffer = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateBuffer(device, &buf_info, gcvNULL, &qyp->queryBuffer));

    /* Allocate device memory for fences */
    __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.allocationSize = __VK_MAX_QUERY_BUF_SIZE;
    mem_alloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory(device, &mem_alloc, gcvNULL, &memory));

    /* bind memory */
    __VK_ONERROR(__vk_BindBufferMemory(device, qyp->queryBuffer, memory, 0));

    /* Lock surface for memory */
    __VK_ONERROR(__vk_MapMemory(device, memory, 0,
        __VK_MAX_QUERY_BUF_SIZE, 0, (void **)&queryMemory));

    __VK_MEMZERO(queryMemory, __VK_MAX_QUERY_BUF_SIZE);

    /* Unlock the memory */
    __vk_UnmapMemory(device, memory);

    for (iq = 0; iq < qyp->queryCount; iq++)
    {
        VkEventCreateInfo ci;

        qyp->pQueries[iq].type           = pCreateInfo->queryType;
        qyp->pQueries[iq].queryPool      = (VkQueryPool)(uintptr_t)qyp;
        qyp->pQueries[iq].queryPoolIndex = iq;

        __VK_MEMZERO(&ci, sizeof(VkEventCreateInfo));
        ci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

        /* Associate a VkEvent with every query */
        __VK_ONERROR(__vk_CreateEvent(device, &ci, VK_NULL_HANDLE, &qyp->pQueries[iq].event));
    }

    /* Return the object pointer as a 64-bit handle */
    *pQueryPool = (VkQueryPool)(uintptr_t)qyp;

    return VK_SUCCESS;

OnError:

    if (qyp)
    {
        if (qyp->queryBuffer)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, qyp->queryBuffer);

            if (buf->memory)
                __vk_FreeMemory(device, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);

            __vk_DestroyBuffer(device, qyp->queryBuffer, gcvNULL);
        }

        if (qyp->pQueries)
        {
            for (iq = 0; iq < qyp->queryCount; iq++)
            {
                if (qyp->pQueries[iq].event)
                    __vk_DestroyEvent(device, qyp->pQueries[iq].event, VK_NULL_HANDLE);
            }

            __VK_FREE(qyp->pQueries);
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_QUERY_POOL, (__vkObject *)qyp);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyQueryPool(
    VkDevice device,
    VkQueryPool queryPool,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (queryPool)
    {
        __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
        uint32_t iq;

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (qyp->queryBuffer)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, qyp->queryBuffer);

            if (buf->memory)
                __vk_FreeMemory(device, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);

            __vk_DestroyBuffer(device, qyp->queryBuffer, gcvNULL);
        }

        if (qyp->pQueries)
        {
            for (iq = 0; iq < qyp->queryCount; iq++)
            {
                if (qyp->pQueries[iq].event)
                    __vk_DestroyEvent(device, qyp->pQueries[iq].event, VK_NULL_HANDLE);
            }

            __VK_FREE(qyp->pQueries);
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_QUERY_POOL, (__vkObject *)qyp);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetQueryPoolResults(
    VkDevice device,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount,
    size_t dataSize,
    void* pData,
    VkDeviceSize stride,
    VkQueryResultFlags flags
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    uint32_t iq;
    VkResult result = VK_SUCCESS;
    uint64_t value64 = 0;

    for (iq = firstQuery; iq < (firstQuery + queryCount); iq++)
    {
        uint8_t *pData8 = (uint8_t *)pData + ((iq - firstQuery) * stride);

        result = __vk_GetEventStatus(device, qyp->pQueries[iq].event);

        if (flags & VK_QUERY_RESULT_WAIT_BIT)
        {
            if (result != VK_EVENT_SET)
            {
                do
                {
                    gcoOS_Delay(gcvNULL, 30);
                    result = __vk_GetEventStatus(device, qyp->pQueries[iq].event);
                    if (result == VK_EVENT_SET)
                        break;
                } while (VK_TRUE);
            }
        }
        else if (!(flags & VK_QUERY_RESULT_PARTIAL_BIT)           &&
                 !(flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) &&
                 (result != VK_EVENT_SET))
        {
            result = VK_NOT_READY;
            continue;
        }

        if ((flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) && (result != VK_EVENT_SET))
        {
            result = VK_NOT_READY;
            value64 = 0;
        }
        else
        {
            __VK_ONERROR((*devCtx->chipFuncs->getQueryResult)(device, queryPool, iq, &value64));
        }

        if (flags & VK_QUERY_RESULT_64_BIT)
        {
            uint64_t *pData64 = (uint64_t *)pData8;

            *pData64 = value64;

            if ((flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) && (stride >= (2 * sizeof(uint64_t))))
            {
                if (result == VK_SUCCESS)
                    pData64[1] = VK_TRUE;
                else
                    pData64[1] = VK_FALSE;
            }
        }
        else
        {
            uint32_t value32 = (value64 > 0xffffffff) ? 0xffffffff : (uint32_t)value64;
            uint32_t *pData32 = (uint32_t *)pData8;

            *pData32 = value32;

            if ((flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) && (stride >= (2 * sizeof(uint32_t))))
            {
                if (result == VK_SUCCESS)
                    pData32[1] = VK_TRUE;
                else
                    pData32[1] = VK_FALSE;
            }
        }
    }

    return result;

OnError:

    return result;
}

