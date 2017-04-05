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


/*
 * Implement VK_KHR_xcb_surface
 * Implement support functions for VK_KHR_swapchain, VK_KHR_surface.
 */
#include "gc_vk_precomp.h"

#ifdef VK_USE_PLATFORM_XCB_KHR
VkResult VKAPI_CALL __vk_CreateXcbSurfaceKHR(
    VkInstance instance,
    xcb_connection_t* connection,
    xcb_window_t window,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    return VK_SUCCESS;
}

VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    xcb_connection_t* connection,
    xcb_visualid_t visual_id
    )
{
    return VK_TRUE;
}
#endif



