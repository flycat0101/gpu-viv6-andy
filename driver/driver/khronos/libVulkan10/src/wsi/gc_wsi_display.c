/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
 * Implement VK_KHR_display for 'fbdev' hardware.
 */
#include "gc_vk_precomp.h"

#include <stdio.h>
#include <string.h>

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties)
{
    VkResult result = VK_SUCCESS;
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t i = 0;

    if (phyDev->numberOfDisplays == 0)
    {
        result = __vkInitializePhysicalDeviceDisplays(phyDev);

        if (result != VK_SUCCESS)
            return result;
    }

    if (pProperties)
    {
        if (*pPropertyCount > phyDev->numberOfDisplays)
            *pPropertyCount = phyDev->numberOfDisplays;

        for (i = 0; i < *pPropertyCount; i++)
        {
            __vkDisplayKHR *disp = phyDev->displays[i];
            pProperties[i].display              = (VkDisplayKHR)(uintptr_t)disp;
            pProperties[i].displayName          = disp->displayName;
            pProperties[i].physicalDimensions   = disp->physicalDimensions;
            pProperties[i].physicalResolution   = disp->physicalResolution;
            pProperties[i].supportedTransforms  = disp->supportedTransforms;
            pProperties[i].planeReorderPossible = disp->planeReorderPossible;
            pProperties[i].persistentContent    = disp->persistentContent;

        }

        if (*pPropertyCount < phyDev->numberOfDisplays)
            return VK_INCOMPLETE;
    }
    else
    {
        *pPropertyCount = phyDev->numberOfDisplays;;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkDisplayPlane *plane    = phyDev->displayPlanes[planeIndex];
    uint32_t i;

    if (pDisplays)
    {
        if (*pDisplayCount > plane->supportedDisplayCount)
            *pDisplayCount = plane->supportedDisplayCount;

        for (i = 0; i < *pDisplayCount; i++)
        {
            pDisplays[i] = (VkDisplayKHR)(uintptr_t)plane->supportedDisplays[i];
        }

        if (*pDisplayCount < plane->supportedDisplayCount)
            return VK_INCOMPLETE;
    }
    else
    {
        *pDisplayCount = plane->supportedDisplayCount;
    }

    return VK_SUCCESS;
}


VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    if (pProperties)
    {
        uint32_t i;

        if (*pPropertyCount > phyDev->numberOfDisplayPlanes)
            *pPropertyCount = phyDev->numberOfDisplayPlanes;

        for (i = 0; i < *pPropertyCount; i++)
        {
            __vkDisplayPlane *plane = phyDev->displayPlanes[i];
            pProperties[i].currentDisplay    = plane->currentDisplay;
            pProperties[i].currentStackIndex = plane->currentStackIndex;
        }

        if (*pPropertyCount < phyDev->numberOfDisplayPlanes)
            return VK_INCOMPLETE;
    }
    else
    {
        *pPropertyCount = phyDev->numberOfDisplayPlanes;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties)
{
    __vkDisplayKHR *dpy = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayKHR *, display);

    if (pProperties)
    {
        uint32_t i;

        if (*pPropertyCount > dpy->displayModeCount)
            *pPropertyCount = dpy->displayModeCount;

        for (i = 0; i < *pPropertyCount; i++)
        {
            __vkDisplayModeKHR *mode   = dpy->displayModes[i];
            pProperties[i].displayMode = (VkDisplayModeKHR)(uintptr_t) mode;
            pProperties[i].parameters  = mode->parameters;
        }

        if (*pPropertyCount < dpy->displayModeCount)
            return VK_INCOMPLETE;
    }
    else
    {
        *pPropertyCount = dpy->displayModeCount;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode)
{
    __vkDisplayKHR *dpy = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayKHR *, display);
    if (dpy->CreateDisplayMode)
    {
        return dpy->CreateDisplayMode(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}


VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities)
{
    __vkDisplayModeKHR * displayMode = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayModeKHR *, mode);
    __vkDisplayKHR *dpy = displayMode->display;

    if (dpy->GetDisplayPlaneCapabilities)
    {
        return dpy->GetDisplayPlaneCapabilities(physicalDevice, mode, planeIndex, pCapabilities);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceDisplay *surf;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    surf = (VkIcdSurfaceDisplay *)__VK_ALLOC(sizeof(VkIcdSurfaceDisplay), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    if (!surf)
    {
        *pSurface = VK_NULL_HANDLE;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    /* Assign function poionters. */
    surf->base.platform   = VK_ICD_WSI_PLATFORM_DISPLAY;
    surf->displayMode     = pCreateInfo->displayMode;
    surf->planeIndex      = pCreateInfo->planeIndex;
    surf->planeStackIndex = pCreateInfo->planeStackIndex;
    surf->transform       = pCreateInfo->transform;
    surf->globalAlpha     = pCreateInfo->globalAlpha;
    surf->alphaMode       = pCreateInfo->alphaMode;
    surf->imageExtent     = pCreateInfo->imageExtent;

    *pSurface = (VkSurfaceKHR)(uintptr_t)surf;
    return VK_SUCCESS;
}


static void dpyDestroySurface(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceDisplay *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceDisplay *, surface);

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    __VK_FREE(surf);
}

static __vkDisplayKHR * GetDisplayFromSurface(VkSurfaceKHR surface)
{
    VkIcdSurfaceDisplay *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceDisplay *, surface);
    __vkDisplayModeKHR *mode  = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayModeKHR *, surf->displayMode);
    return mode->display;
}

static VkResult dpyGetPhysicalDeviceSurfaceSupport(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported)
{
    __vkDisplayKHR *dpy = GetDisplayFromSurface(surface);

    if (dpy->GetPhysicalDeviceSurfaceSupport)
    {
        return dpy->GetPhysicalDeviceSurfaceSupport(physicalDevice, queueFamilyIndex, surface, pSupported);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}

static VkResult dpyGetPhysicalDeviceSurfaceCapabilities(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
    __vkDisplayKHR *dpy = GetDisplayFromSurface(surface);

    if (dpy->GetPhysicalDeviceSurfaceCapabilities)
    {
        return dpy->GetPhysicalDeviceSurfaceCapabilities(physicalDevice, surface, pSurfaceCapabilities);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}

static VkResult dpyGetPhysicalDeviceSurfaceFormats(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats)
{
    __vkDisplayKHR *dpy = GetDisplayFromSurface(surface);

    if (dpy->GetPhysicalDeviceSurfaceFormats)
    {
        return dpy->GetPhysicalDeviceSurfaceFormats(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}

static VkResult dpyGetPhysicalDeviceSurfacePresentModes(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
    __vkDisplayKHR *dpy = GetDisplayFromSurface(surface);

    if (dpy->GetPhysicalDeviceSurfacePresentModes)
    {
        return dpy->GetPhysicalDeviceSurfacePresentModes(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}

static VkResult dpyCreateSwapchain(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain)
{
    __vkDisplayKHR *dpy = GetDisplayFromSurface(pCreateInfo->surface);

    if (dpy->CreateSwapchain)
    {
        return dpy->CreateSwapchain(device, pCreateInfo, pAllocator, pSwapchain);
    }

    return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
}

__vkSurfaceOperation __vkDisplaySurfaceOperation =
{
    dpyDestroySurface,
    dpyGetPhysicalDeviceSurfaceSupport,
    dpyGetPhysicalDeviceSurfaceCapabilities,
    dpyGetPhysicalDeviceSurfaceFormats,
    dpyGetPhysicalDeviceSurfacePresentModes,
    dpyCreateSwapchain,
};



