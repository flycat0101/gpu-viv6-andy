/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vk_wsi_h__
#define __gc_vk_wsi_h__

#define __VK_WSI_MAX_PHYSICAL_DISPLAYS       4
#define __VK_WSI_MAX_DISPLAY_PLANES          32
#define __VK_WSI_MAX_DISPLAY_MODES           128


typedef enum __VkDirectRenderModeRec
{
    __VK_WSI_INDIRECT_RENDERING = 0,
    __VK_WSI_DIRECT_RENDERING_FCFILL,
    __VK_WSI_DIRECT_RENDERING_NOFC,
    __VK_WSI_DIRECT_RENDERING_FC_NOCC,
    __VK_WSI_DIRECT_RENDERING
}
__VkDirectRenderMode;

typedef struct __vkSurfaceKHRRec        __vkSurfaceKHR;
typedef struct __vkSwapchainKHRRec      __vkSwapchainKHR;
typedef struct __vkDisplayKHRRec        __vkDisplayKHR;
typedef struct __vkDisplayPlaneRec      __vkDisplayPlane;
typedef struct __vkDisplayModeKHRRec    __vkDisplayModeKHR;
typedef struct __vkSurfaceOperationRec  __vkSurfaceOperation;

struct __vkSurfaceOperationRec
{
    void     (* DestroySurface)(VkInstance                    instance,
                                VkSurfaceKHR                  surface,
                                const VkAllocationCallbacks * pAllocator);

    VkResult (* GetPhysicalDeviceSurfaceSupport)(VkPhysicalDevice physicalDevice,
                                                 uint32_t         queueFamilyIndex,
                                                 VkSurfaceKHR     surface,
                                                 VkBool32*        pSupported);

    VkResult (* GetPhysicalDeviceSurfaceCapabilities)(VkPhysicalDevice          physicalDevice,
                                                      VkSurfaceKHR              surface,
                                                      VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);

    VkResult (* GetPhysicalDeviceSurfaceFormats)(VkPhysicalDevice    physicalDevice,
                                                 VkSurfaceKHR        surface,
                                                 uint32_t*           pSurfaceFormatCount,
                                                 VkSurfaceFormatKHR* pSurfaceFormats);

    VkResult (* GetPhysicalDeviceSurfacePresentModes)(VkPhysicalDevice  physicalDevice,
                                                      VkSurfaceKHR      surface,
                                                      uint32_t*         pPresentModeCount,
                                                      VkPresentModeKHR* pPresentModes);

    VkResult (* GetDeviceGroupSurfacePresentModesKHR)(VkDevice                          device,
                                                      VkSurfaceKHR                      surface,
                                                      VkDeviceGroupPresentModeFlagsKHR* pModes);

    VkResult (* GetPhysicalDevicePresentRectanglesKHR)(VkPhysicalDevice physicalDevice,
                                                       VkSurfaceKHR     surface,
                                                       uint32_t*        pRectCount,
                                                       VkRect2D*        pRects);

    VkResult (* CreateSwapchain)(VkDevice                        device,
                                 const VkSwapchainCreateInfoKHR* pCreateInfo,
                                 const VkAllocationCallbacks*    pAllocator,
                                 VkSwapchainKHR*                 pSwapchain);
};

struct __vkSwapchainKHRRec
{
    __vkObject                      obj;
    const char *                    concreteType;

    /* Functions. */
    void     (* DestroySwapchain)(VkDevice                     device,
                                  VkSwapchainKHR               swapchain,
                                  const VkAllocationCallbacks* pAllocator);

    VkResult (* GetSwapchainImages)(VkDevice       device,
                                    VkSwapchainKHR swapchain,
                                    uint32_t*      pSwapchainImageCount,
                                    VkImage*       pSwapchainImages);

    VkResult (* AcquireNextImage)(VkDevice       device,
                                  VkSwapchainKHR swapchain,
                                  uint64_t       timeout,
                                  VkSemaphore    semaphore,
                                  VkFence        fence,
                                  uint32_t*      pImageIndex);

    VkResult (* QueuePresentSingle)(VkQueue                        queue,
                                    const VkDisplayPresentInfoKHR* pDisplayPresentInfo,
                                    VkSwapchainKHR                 swapchain,
                                    uint32_t                       imageIndex);
};

/* VK_KHR_display. */
struct __vkDisplayKHRRec
{
    __vkObjectType                  sType;

    /* Display properties. */
    char                            displayName[32];
    VkExtent2D                      physicalDimensions;
    VkExtent2D                      physicalResolution;
    VkSurfaceTransformFlagsKHR      supportedTransforms;
    VkBool32                        planeReorderPossible;
    VkBool32                        persistentContent;

    /* Display modes. */
    __vkDisplayModeKHR *            displayModes[__VK_WSI_MAX_DISPLAY_MODES];
    uint32_t                        displayModeCount;

    __vkDisplayPlane *              planeStack[__VK_WSI_MAX_DISPLAY_PLANES];
    uint32_t                        planeStackSize;

    /* Create DisplayMode. */
    VkResult (* CreateDisplayMode)(VkPhysicalDevice                  physicalDevice,
                                   VkDisplayKHR                      display,
                                   const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                   const VkAllocationCallbacks*      pAllocator,
                                   VkDisplayModeKHR*                 pMode);

    /* Get DisplayPlane capabilities with display and mode. */
    /* Capabilities may vary according to different display and mode combination. */
    VkResult (* GetDisplayPlaneCapabilities)(VkPhysicalDevice               physicalDevice,
                                             VkDisplayModeKHR               mode,
                                             uint32_t                       planeIndex,
                                             VkDisplayPlaneCapabilitiesKHR* pCapabilities);

    /* Surface functions. */
    VkResult (* GetPhysicalDeviceSurfaceSupport)(VkPhysicalDevice physicalDevice,
                                                 uint32_t         queueFamilyIndex,
                                                 VkSurfaceKHR     surface,
                                                 VkBool32*        pSupported);

    VkResult (* GetPhysicalDeviceSurfaceCapabilities)(VkPhysicalDevice          physicalDevice,
                                                      VkSurfaceKHR              surface,
                                                      VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);

    VkResult (* GetPhysicalDeviceSurfaceFormats)(VkPhysicalDevice    physicalDevice,
                                                 VkSurfaceKHR        surface,
                                                 uint32_t*           pSurfaceFormatCount,
                                                 VkSurfaceFormatKHR* pSurfaceFormats);

    VkResult (* GetPhysicalDeviceSurfacePresentModes)(VkPhysicalDevice  physicalDevice,
                                                      VkSurfaceKHR      surface,
                                                      uint32_t*         pPresentModeCount,
                                                      VkPresentModeKHR* pPresentModes);

    VkResult (* GetDeviceGroupSurfacePresentModesKHR)(VkDevice                          device,
                                                      VkSurfaceKHR                      surface,
                                                      VkDeviceGroupPresentModeFlagsKHR* pModes);

    VkResult (* GetPhysicalDevicePresentRectanglesKHR)(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR      surface,
                                                      uint32_t*         pRectCount,
                                                      VkRect2D*         pRects);

    /* Swapchain functions. */
    VkResult (* CreateSwapchain)(VkDevice                        device,
                                 const VkSwapchainCreateInfoKHR* pCreateInfo,
                                 const VkAllocationCallbacks*    pAllocator,
                                 VkSwapchainKHR*                 pSwapchain);
};

struct __vkDisplayPlaneRec
{
    uint32_t                        planeIndex;

    /* DisplayPlane properties. */
    VkDisplayKHR                    currentDisplay;
    uint32_t                        currentStackIndex;

    /* DisplayPlane supported displays. */
    __vkDisplayKHR *                supportedDisplays[__VK_WSI_MAX_PHYSICAL_DISPLAYS];
    uint32_t                        supportedDisplayCount;
};

struct __vkDisplayModeKHRRec
{
    __vkObjectType                  sType;

    /* DisplayMode implies the display it belongs to. */
    __vkDisplayKHR *                display;

    /* DisplayMode specific fields */
    VkDisplayModeParametersKHR      parameters;

    uint32_t                        bitsPerPixel;
    VkBool32                        interlaced;
};

VkResult __vkInitializePhysicalDeviceDisplays(__vkPhysicalDevice *phyDevice);

extern __vkSurfaceOperation __vkMirSurfaceOperation;
extern __vkSurfaceOperation __vkWaylandSurfaceOperation;
extern __vkSurfaceOperation __vkWin32SurfaceOperation;
extern __vkSurfaceOperation __vkXcbSurfaceOperation;
extern __vkSurfaceOperation __vkXlibSurfaceOperation;
extern __vkSurfaceOperation __vkAndroidSurfaceOperation;
extern __vkSurfaceOperation __vkDisplaySurfaceOperation;

#define __VK_ICD_WSI_PLATFORM_ANDROID   (VK_ICD_WSI_PLATFORM_DISPLAY + 1)

#endif /* __gc_vk_wsi_h__ */


