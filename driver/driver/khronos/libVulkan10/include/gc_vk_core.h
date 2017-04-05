/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vk_core_h__
#define __gc_vk_core_h__

#define DEBUG_VULKAN_ALLOCATOR      0

#define __VK_COUNTOF(a) \
(\
    sizeof(a) / sizeof(a[0]) \
)

#define __VK_CLAMP(x, min, max) \
(\
    ((x) < (min)) \
        ? (min) \
        : ((x) > (max)) \
            ? (max) \
            : (x) \
)

#define __VK_MIN(x, y) \
(\
    ((x) <= (y)) \
        ? (x) \
        : (y) \
)

#define __VK_MAX(x, y) \
(\
    ((x) >= (y)) \
        ? (x) \
        : (y) \
)

#define __VK_NOTHING for (;;) break

#if DEBUG_VULKAN_ALLOCATOR
extern VkAllocationCallbacks __vkAllocatorWrapper;

#define __VK_ALLOC(__size__, __alignment__, __scope__) \
    __vkAllocatorWrapper.pfnAllocation(pMemCb, __size__, __alignment__, __scope__)

#define __VK_FREE(__pMemory__) \
    __vkAllocatorWrapper.pfnFree(pMemCb, __pMemory__)
#else
#define __VK_ALLOC(__size__, __alignment__, __scope__) \
    (*pMemCb->pfnAllocation)(pMemCb->pUserData, __size__, __alignment__, __scope__)

#define __VK_FREE(__pMemory__) \
    (*pMemCb->pfnFree)(pMemCb->pUserData, __pMemory__)

#endif

#define __VK_SET_API_ALLOCATIONCB(__default) \
    VkAllocationCallbacks *pMemCb; \
    if (pAllocator) \
    { \
        pMemCb = (VkAllocationCallbacks *)pAllocator; \
    } \
    else \
    { \
        pMemCb = __default; \
    }

#define __VK_SET_ALLOCATIONCB(__parent) \
    VkAllocationCallbacks *pMemCb; \
    pMemCb = (VkAllocationCallbacks *)__parent;

#define __VK_ALLOCATIONCB       (*pMemCb)

#define __VK_ONERROR(func) \
    do \
    { \
        result = (VkResult)(func); \
        if (result != VK_SUCCESS) \
        { \
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "ONERROR: result=%d\n", (uint32_t)result); \
            goto OnError; \
        } \
    } \
    while (VK_FALSE)

#define __VK_ERR_BREAK(func) \
    result = (VkResult)(func); \
    if (result != VK_SUCCESS) \
    { \
        __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "ERR_BREAK: result=%d\n", (uint32_t)result); \
        break; \
    } \
    do { } while (VK_FALSE)

#define __VK_VERIFY_OK(func) \
    do \
    { \
        VkResult verifyStatus = (VkResult)func; \
        if (verifyStatus != VK_SUCCESS) \
        { \
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "__VK_VERIFY_OK: function returned %d", verifyStatus); \
        } \
        __VK_ASSERT(verifyStatus == VK_SUCCESS); \
    } \
    while (VK_FALSE)

#define __VK_IS_SUCCESS(result)       ((VkResult)result == VK_SUCCESS)

#define __VK_RESOURCE_SAVE_TGA      0

#if __VK_RESOURCE_SAVE_TGA || gcdFRAMEINFO_STATISTIC
#define __VK_RESOURCE_INFO          1
#else
#define __VK_RESOURCE_INFO          0
#endif

#define __VK_NEW_DEVICE_QUEUE       1

VkResult __vk_DeviceControl(
    gcsHAL_INTERFACE * iface,
    uint32_t coreIdex
    );

#endif /* __gc_vk_core_h__ */


