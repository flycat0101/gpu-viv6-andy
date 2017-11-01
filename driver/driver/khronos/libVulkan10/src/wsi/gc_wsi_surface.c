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


/* Implement VK_KHR_surface. */
#include "gc_vk_precomp.h"

static __vkSurfaceOperation *
__GetSurfaceOperation(
    VkSurfaceKHR surface
    )
{
    VkIcdSurfaceBase *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceBase *, surface);
    if (surf)
    {
        switch ((int)surf->platform)
        {
#ifdef VK_USE_PLATFORM_MIR_KHR
        case VK_ICD_WSI_PLATFORM_MIR:
            return &__vkMirSurfaceOperation;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        case VK_ICD_WSI_PLATFORM_WAYLAND:
            return &__vkWaylandSurfaceOperation;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_ICD_WSI_PLATFORM_WIN32:
            return &__vkWin32SurfaceOperation;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
        case VK_ICD_WSI_PLATFORM_XCB:
            return &__vkXcbSurfaceOperation;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
        case VK_ICD_WSI_PLATFORM_XLIB:
            return &__vkXlibSurfaceOperation;
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case __VK_ICD_WSI_PLATFORM_ANDROID:
            return &__vkAndroidSurfaceOperation;
#endif

        case VK_ICD_WSI_PLATFORM_DISPLAY:
        default:
            return &__vkDisplaySurfaceOperation;
        }
    }
    else
    {
        return gcvNULL;
    }
}

void VKAPI_CALL __vk_DestroySurfaceKHR(
    VkInstance  instance,
    VkSurfaceKHR  surface,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(surface);
    if (operation)
    {
        operation->DestroySurface(instance, surface, pAllocator);
    }
}

VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(surface);
    return operation->GetPhysicalDeviceSurfaceSupport(physicalDevice, queueFamilyIndex, surface, pSupported);
}

VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(surface);
    return operation->GetPhysicalDeviceSurfaceCapabilities(physicalDevice, surface, pSurfaceCapabilities);
}

VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(surface);
    return operation->GetPhysicalDeviceSurfaceFormats(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}


VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(surface);
    return operation->GetPhysicalDeviceSurfacePresentModes(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

VkResult VKAPI_CALL __vk_CreateSwapchainKHR(
    VkDevice device,
    const VkSwapchainCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchain
    )
{
    __vkSurfaceOperation *operation = __GetSurfaceOperation(pCreateInfo->surface);
    return operation->CreateSwapchain(device, pCreateInfo, pAllocator, pSwapchain);
}


