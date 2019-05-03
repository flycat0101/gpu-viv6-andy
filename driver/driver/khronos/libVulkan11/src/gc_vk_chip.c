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
    return VK_SUCCESS;
}
VKAPI_ATTR void VKAPI_CALL __vk_DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
{
}



