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
    devCtx->chipFuncs = &halti5_chip;
    return ((*devCtx->chipFuncs->InitializeChipModule)((VkDevice)devCtx));
}


