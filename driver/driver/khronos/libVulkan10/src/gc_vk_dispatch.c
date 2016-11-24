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


#include "gc_vk_precomp.h"

VKAPI_ATTR void VKAPI_CALL __vk_GetImageSparseMemoryRequirements(
    VkDevice device,
    VkImage image,
    uint32_t* pNumRequirements,
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements
    )
{
    /* TODO */
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usage,
    VkImageTiling tiling,
    uint32_t* pPropertyCount,
    VkSparseImageFormatProperties* pProperties
    )
{
    /* TODO */
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueBindSparse(
    VkQueue queue,
    uint32_t bindInfoCount,
    const VkBindSparseInfo* pBindInfo,
    VkFence fence
    )
{
    /* TODO */
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetRenderAreaGranularity(
    VkDevice device,
    VkRenderPass renderPass,
    VkExtent2D* pGranularity
    )
{
    /* TODO */
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugReportCallbackEXT* pCallback
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    __vkDebugCallbackEXT *dcb;

    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    dcb = (__vkDebugCallbackEXT *)__VK_ALLOC(sizeof(__vkDebugCallbackEXT), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!dcb)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    __VK_MEMZERO(dcb, sizeof(__vkDebugCallbackEXT));

    dcb->sType = __VK_OBJECT_TYPE_DEBUG_CB_EXT;
    dcb->createInfo = *pCreateInfo;

    *pCallback = (VkDebugReportCallbackEXT)(uintptr_t)dcb;

    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyDebugReportCallbackEXT(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* pAllocator
    )
{
    //__vkInstance *inst = (__vkInstance *)instance;
    __vkDebugCallbackEXT *dcb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDebugCallbackEXT *, callback);

    __VK_SET_ALLOCATIONCB(pAllocator);

    __VK_FREE(dcb);
}

VKAPI_ATTR void VKAPI_CALL __vk_DebugReportMessageEXT(
    VkInstance instance,
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage
    )
{
    /* TODO */
}


#define __vkEntry_(func)    __vk_##func,
#define __vkICDEntry_(func) __vk_icd##func,

__vkDispatchTable __vkDrvEntryFuncTable = {
    __VK_API_ENTRIES(__vkEntry_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkEntry_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkEntry_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkEntry_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkEntry_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkEntry_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkEntry_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkEntry_)
#endif
    __VK_ICD_API_ENTRIES(__vkICDEntry_)
};


