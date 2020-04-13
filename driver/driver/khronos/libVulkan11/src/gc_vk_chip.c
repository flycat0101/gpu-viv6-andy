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

extern __vkChipFuncTable halti5_chip;

VkResult __vkInitilizeChipModule(
    __vkPhysicalDevice *phyDev,
    __vkDevContext *devCtx
    )
{
    if (!phyDev->phyDevConfig.database->REG_BltEngine)
    {
        halti5_chip.ClearImage = halti2_clearImageWithRS;
        halti5_chip.CopyImage = halti2_copyImageWithRS;
        halti5_chip.FillBuffer = halti3_fillBuffer;
        halti5_chip.CopyBuffer = halti3_copyBuffer;
        halti5_chip.UpdateBuffer = halti3_updateBuffer;
    }

    /* Split draw path.*/
    if (!phyDev->phyDevConfig.database->FE_PATCHLIST_FETCH_FIX)
    {
        halti5_chip.DrawIndexed = halti5_splitDrawIndexed;
    }

    if (phyDev->phyDevConfig.database->FE_DRAW_DIRECT)
    {
        halti5_chip.Draw = halti5_drawDirect;
        halti5_chip.DrawIndexed = halti5_drawIndexedDirect;
    }

    devCtx->chipFuncs = &halti5_chip;
    return ((*devCtx->chipFuncs->InitializeChipModule)((VkDevice)devCtx));
}

VKAPI_ATTR void VKAPI_CALL __vk_GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue)
{
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSamplerYcbcrConversion* pConversion = VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (pCreateInfo->ycbcrModel == VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020 ||
        pCreateInfo->ycbcrModel == VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY ||
        (pCreateInfo->ycbcrRange == VK_SAMPLER_YCBCR_RANGE_ITU_FULL &&
         pCreateInfo->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY))
    {
        result = VK_ERROR_FEATURE_NOT_PRESENT;
        goto OnError;
    }

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_YCBCR_CONVERSION, sizeof(__vkSampler), (__vkObject**)&pConversion));

    /* Initialize __vkSamplerYcbcrConversion specific data fields here */
    __VK_MEMCOPY(&pConversion->createInfo, pCreateInfo, sizeof(VkSamplerYcbcrConversionCreateInfo));

    *pYcbcrConversion = (VkSamplerYcbcrConversion)(uintptr_t)pConversion;

    return VK_SUCCESS;
OnError:
    if (pConversion)
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_YCBCR_CONVERSION, (__vkObject *)pConversion);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (ycbcrConversion)
    {
        __vkSamplerYcbcrConversion *pConversion = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSamplerYcbcrConversion *, ycbcrConversion);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);


        __vk_DestroyObject(devCtx, __VK_OBJECT_YCBCR_CONVERSION, (__vkObject *)pConversion);
    }
}




