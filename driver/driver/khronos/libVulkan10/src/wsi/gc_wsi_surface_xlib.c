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


/*
 * Implement VK_KHR_xlib_surface
 * Implement support functions for VK_KHR_swapchain, VK_KHR_surface.
 */
#include "gc_vk_precomp.h"

#ifdef VK_USE_PLATFORM_XLIB_KHR
VkResult VKAPI_CALL __vk_CreateXlibSurfaceKHR(
    VkInstance instance,
    Display* dpy,
    Window window,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    return VK_SUCCESS;
}

VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    Display* dpy,
    VisualID visualID
    )
{
    return VK_TRUE;
}
#endif



