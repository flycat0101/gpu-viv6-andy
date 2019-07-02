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


/*
 * Implement VK_KHR_swapchain
 * Require support functions of concrete surface type.
 */
#include "gc_vk_precomp.h"

void VKAPI_CALL __vk_DestroySwapchainKHR(
    VkDevice device,
    VkSwapchainKHR swapchain,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    if (sc)
    {
        sc->DestroySwapchain(device, swapchain, pAllocator);
    }
}

VkResult VKAPI_CALL __vk_GetSwapchainImagesKHR(
    VkDevice device,
    VkSwapchainKHR swapchain,
    uint32_t* pSwapchainImageCount,
    VkImage* pSwapchainImages
    )
{
    __vkSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    return sc->GetSwapchainImages(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VkResult VKAPI_CALL __vk_AcquireNextImageKHR(
    VkDevice device,
    VkSwapchainKHR swapchain,
    uint64_t timeout,
    VkSemaphore semaphore,
    VkFence fence,
    uint32_t* pImageIndex
    )
{
    __vkSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    return sc->AcquireNextImage(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VkResult VKAPI_CALL __vk_QueuePresentKHR(
    VkQueue queue,
    const VkPresentInfoKHR* pPresentInfo
    )
{
    VkResult result = VK_SUCCESS;
    const VkDisplayPresentInfoKHR *dispPresentInfo = pPresentInfo->pNext;
    uint32_t i;

    if (pPresentInfo->waitSemaphoreCount)
    {
        __vk_InsertSemaphoreWaits(
            queue, pPresentInfo->pWaitSemaphores, pPresentInfo->waitSemaphoreCount);
    }

    for (i = 0; i < pPresentInfo->swapchainCount; i++)
    {
        VkResult r;
        __vkSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, pPresentInfo->pSwapchains[i]);
        uint32_t ii = pPresentInfo->pImageIndices[i];

        r = sc->QueuePresentSingle(queue, dispPresentInfo, pPresentInfo->pSwapchains[i], ii);

        if (pPresentInfo->pResults)
            pPresentInfo->pResults[i] = r;

        if (r != VK_SUCCESS)
            result = VK_ERROR_SURFACE_LOST_KHR;
    }

    return result;
}

VkResult VKAPI_CALL __vk_GetDeviceGroupPresentCapabilitiesKHR(
    VkDevice device,
    VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities
    )
{
    uint32_t i = 0;
    for (i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; i++)
    {
        pDeviceGroupPresentCapabilities->presentMask[i] = 0;
    }

    /* Modes always has VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR set. */
    pDeviceGroupPresentCapabilities->modes = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;
    return VK_SUCCESS;
}

VkResult VKAPI_CALL __vk_AcquireNextImage2KHR(
    VkDevice device,
    const VkAcquireNextImageInfoKHR* pAcquireInfo,
    uint32_t* pImageIndex)
{
    __vkSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, pAcquireInfo->swapchain);
    return sc->AcquireNextImage(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore, pAcquireInfo->fence, pImageIndex);
}

/* VK_KHR_display_swapchain. */
VkResult VKAPI_CALL __vk_CreateSharedSwapchainsKHR(
    VkDevice device,
    uint32_t swapchainCount,
    const VkSwapchainCreateInfoKHR* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchains
    )
{
    return VK_SUCCESS;
}



