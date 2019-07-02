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


VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateShaderModule(
    VkDevice device,
    const VkShaderModuleCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkShaderModule* pShaderModule
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkShaderModule *shm;
    VkResult result;
    SpvDecodeInfo decodeInfo;
    void *funcTable = VK_NULL_HANDLE;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    result = __vk_CreateObject(devCtx, __VK_OBJECT_SHADER_MODULE, sizeof(__vkShaderModule), (__vkObject**)&shm);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    /* Initialize __vkShaderModule specific data fields here */
    shm->codeSize = pCreateInfo->codeSize;
    shm->pCode = (uint32_t *)__VK_ALLOC(shm->codeSize, 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!shm->pCode)
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_SHADER_MODULE, (__vkObject *)shm);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    __VK_MEMCOPY(shm->pCode, pCreateInfo->pCode, shm->codeSize);

    __VK_MEMZERO(&decodeInfo, sizeof(decodeInfo));
    decodeInfo.binary = shm->pCode;
    decodeInfo.sizeInByte = (gctUINT)shm->codeSize;
    decodeInfo.localSize[0] = decodeInfo.localSize[1] = decodeInfo.localSize[2] = 0;
    if (gcvSTATUS_OK == gcSPV_PreDecode(&decodeInfo, &funcTable))
    {
        shm->funcTable = funcTable;
    }

    /* Return the object pointer as a 64-bit handle */
    *pShaderModule = (VkShaderModule)(uintptr_t)shm;

    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyShaderModule(
    VkDevice device,
    VkShaderModule shaderModule,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (shaderModule)
    {
        __vkShaderModule *shm = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkShaderModule *, shaderModule);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        /* Free the function dependency */
        if (shm->funcTable)
        {
            gcSPV_PostDecode(shm->funcTable);
        }

        __VK_FREE(shm->pCode);

        __vk_DestroyObject(devCtx, __VK_OBJECT_SHADER_MODULE, (__vkObject *)shm);
    }
}



