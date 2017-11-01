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


#include "gc_vk_precomp.h"

#define __VK_ICD_LOADABLE_API_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateInstance) \
    vkApiMacro(DestroyInstance) \
    vkApiMacro(EnumeratePhysicalDevices) \
    vkApiMacro(GetPhysicalDeviceFeatures) \
    vkApiMacro(GetPhysicalDeviceFormatProperties) \
    vkApiMacro(GetPhysicalDeviceImageFormatProperties) \
    vkApiMacro(GetPhysicalDeviceProperties) \
    vkApiMacro(GetPhysicalDeviceQueueFamilyProperties) \
    vkApiMacro(GetPhysicalDeviceMemoryProperties) \
    vkApiMacro(GetInstanceProcAddr) \
    vkApiMacro(GetDeviceProcAddr) \
    vkApiMacro(CreateDevice) \
    vkApiMacro(EnumerateInstanceExtensionProperties) \
    vkApiMacro(EnumerateDeviceExtensionProperties) \
    vkApiMacro(EnumerateInstanceLayerProperties) \
    vkApiMacro(EnumerateDeviceLayerProperties) \
    vkApiMacro(GetPhysicalDeviceSparseImageFormatProperties) \
    vkApiMacro(DestroySurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceSupportKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceCapabilitiesKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceFormatsKHR) \
    vkApiMacro(GetPhysicalDeviceSurfacePresentModesKHR) \
    vkApiMacro(GetPhysicalDeviceDisplayPropertiesKHR) \
    vkApiMacro(GetPhysicalDeviceDisplayPlanePropertiesKHR) \
    vkApiMacro(GetDisplayPlaneSupportedDisplaysKHR) \
    vkApiMacro(GetDisplayModePropertiesKHR) \
    vkApiMacro(CreateDisplayModeKHR) \
    vkApiMacro(GetDisplayPlaneCapabilitiesKHR) \
    vkApiMacro(CreateDisplayPlaneSurfaceKHR) \
    vkApiMacro(CreateSwapchainKHR) \
    vkApiMacro(GetPhysicalDeviceFeatures2KHR) \
    vkApiMacro(GetPhysicalDeviceProperties2KHR) \
    vkApiMacro(GetPhysicalDeviceFormatProperties2KHR) \
    vkApiMacro(GetPhysicalDeviceImageFormatProperties2KHR) \
    vkApiMacro(GetPhysicalDeviceQueueFamilyProperties2KHR) \
    vkApiMacro(GetPhysicalDeviceMemoryProperties2KHR) \
    vkApiMacro(GetPhysicalDeviceSparseImageFormatProperties2KHR) \
    vkApiMacro(TrimCommandPoolKHR) \
    vkApiMacro(CreateDebugReportCallbackEXT) \
    vkApiMacro(DestroyDebugReportCallbackEXT) \
    vkApiMacro(DebugReportMessageEXT) \
    vkApiMacro(_icdGetInstanceProcAddr) \
    vkApiMacro(_icdNegotiateLoaderICDInterfaceVersion)

/* Define APIs can be obtained by vk_icdGetInstanceProcAddr. */
#define __vkProc(func) #func,
static const char *__vkICDLoadableProcTable[] =
{
    __VK_ICD_LOADABLE_API_ENTRIES(__vkProc)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkProc)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkProc)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkProc)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkProc)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkProc)
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkProc)
#endif
};

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_icdGetInstanceProcAddr(
    VkInstance instance,
    const char* pName
    )
{
    const char *apiName;
    size_t i;

    /* Skip invalid names first */
    if (!pName || pName[0] != 'v' || pName[1] != 'k' || pName[2] == '\0')
    {
        return VK_NULL_HANDLE;
    }

    /* Skip the first two characters "vk" of pName. */
    apiName = pName + 2;

    /* Find API function's offset in __vkICDLoadableProcTable[] table */
    for (i = 0; i < __VK_TABLE_SIZE(__vkICDLoadableProcTable); ++i)
    {
        if (strcmp(__vkICDLoadableProcTable[i], apiName) == 0)
        {
            return __vk_GetApiProcAddr(pName);
        }
    }

    return VK_NULL_HANDLE;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_icdNegotiateLoaderICDInterfaceVersion(
    uint32_t *pVersion)
{
    if (*pVersion < 2)
        return VK_ERROR_INCOMPATIBLE_DRIVER;

    *pVersion = 2;
    return VK_SUCCESS;
}



