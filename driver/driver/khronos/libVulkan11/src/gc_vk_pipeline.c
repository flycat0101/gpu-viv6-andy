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

#define __VK_PIPELINE_CACHE_HASH_ENTRY_NUM  256
#define __VK_PIPELINE_CACHE_HASH_ENTRY_SIZE 0xFFFFFFFF

void __vkPipelineCacheFreeModule(
    VkAllocationCallbacks *pAllocator,
    void *pUserData
    )
{
    __vkModuleCacheEntry *moduleCache = (__vkModuleCacheEntry*)pUserData;
    __VK_SET_ALLOCATIONCB(pAllocator);

    halti5_DestroyVkShader(moduleCache->handle);

    __VK_FREE(moduleCache);
    return;
}

VkResult __vkInitPipelineCacheData(
    __vkDevContext *devCtx,
    __vkPipelineCache *pch,
    const void *pInitialData,
    size_t dataSize
    )
{
    VkResult result = VK_SUCCESS;
    const uint8_t *pData = (const uint8_t*)pInitialData;
    const uint8_t *pEnd = pData + dataSize;
    const __vkPipelineCachePublicHead  *publicHead  = (const __vkPipelineCachePublicHead*)pData;
    const __vkPipelineCachePrivateHead *privateHead = (const __vkPipelineCachePrivateHead*)(pData + publicHead->headBytes);
    __VK_SET_ALLOCATIONCB(&pch->memCb);

    /* Public head mismatch */
    if (__VK_MEMCMP(publicHead, pch->publicHead, publicHead->headBytes) != 0)
    {
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    /* Private head mismatch */
    if (privateHead->totalBytes != dataSize ||
        privateHead->headBytes != pch->privateHead->headBytes ||
        privateHead->bigEndian != pch->privateHead->bigEndian ||
        privateHead->osInfo    != pch->privateHead->osInfo    ||
        privateHead->patchID   != pch->privateHead->patchID   ||
        privateHead->version   != pch->privateHead->version   ||
        __VK_MEMCMP(privateHead->drvVersion, pch->privateHead->drvVersion, 32) != 0
       )
    {
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    pData += (publicHead->headBytes + privateHead->headBytes);
    while (pData < pEnd)
    {
        const __vkModuleCacheHead *moduleHead = (const __vkModuleCacheHead*)pData;
        __vkModuleCacheEntry *pEntry = gcvNULL;

        __VK_ASSERT(moduleHead->binBytes <= moduleHead->alignBytes);
        do
        {
            SHADER_HANDLE vscHandle = VK_NULL_HANDLE;
            pEntry = (__vkModuleCacheEntry*)__VK_ALLOC(sizeof(__vkModuleCacheEntry), 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
            __VK_ERR_BREAK(pEntry ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

            __VK_MEMCOPY(&pEntry->head, moduleHead, __VK_MIN(sizeof(__vkModuleCacheHead), moduleHead->headBytes));
            pEntry->head.headBytes = sizeof(__vkModuleCacheHead);
            __VK_ERR_BREAK(vscLoadShaderFromBinary((void*)(pData + moduleHead->headBytes),
                                                   moduleHead->binBytes, &vscHandle, VK_FALSE));
            pEntry->handle = halti5_CreateVkShader(vscHandle);
            __VK_ASSERT(pEntry->handle);

            if (!__vk_utils_hashAddObj(pMemCb, pch->moduleHash, pEntry, moduleHead->hashKey, VK_FALSE))
            {
                __VK_ERR_BREAK(VK_ERROR_OUT_OF_HOST_MEMORY);
            }

            pData += (pEntry->head.headBytes + pEntry->head.alignBytes);

            pch->numModules++;
            pch->totalBytes += (pEntry->head.headBytes + pEntry->head.alignBytes);
        } while (VK_FALSE);

        if (result != VK_SUCCESS)
        {
            if (pEntry)
            {
                __VK_VERIFY_OK(halti5_DestroyVkShader(pEntry->handle));
                __VK_FREE(pEntry);
            }
            __VK_ERR_BREAK(result);
        }
    }

    if (pch->numModules != privateHead->numModules ||
        pch->totalBytes != privateHead->totalBytes)
    {
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    if (result != VK_SUCCESS)
    {
        pch->numModules = 0;
        pch->totalBytes = sizeof(__vkPipelineCachePublicHead) + sizeof(__vkPipelineCachePrivateHead);
        __vk_utils_hashDestory(pMemCb, pch->moduleHash);
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreatePipelineCache(
    VkDevice device,
    const VkPipelineCacheCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPipelineCache* pPipelineCache
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineCache *pch = gcvNULL;
    gcePATCH_ID patchID = gcvPATCH_INVALID;
    VkResult result = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_PIPELINE_CACHE, sizeof(__vkPipelineCache), (__vkObject**)&pch));
    __VK_ONERROR(gcoOS_CreateMutex(gcvNULL, &pch->mutex));

    /* Initialize __vkPipelineCache specific data fields here */
    pch->memCb = __VK_ALLOCATIONCB;

    pch->totalBytes = sizeof(__vkPipelineCachePublicHead) + sizeof(__vkPipelineCachePrivateHead);

    pch->publicHead = (__vkPipelineCachePublicHead*)__VK_ALLOC(pch->totalBytes, 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
    if (!pch->publicHead)
    {
        __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
    }
    __VK_MEMZERO(pch->publicHead, pch->totalBytes);

    pch->publicHead->headBytes = (uint32_t)sizeof(__vkPipelineCachePublicHead);
    pch->publicHead->version   = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
    pch->publicHead->vendorID  = devCtx->pPhyDevice->phyDevProp.vendorID;
    pch->publicHead->deviceID  = devCtx->pPhyDevice->phyDevProp.deviceID;
    __VK_MEMCOPY(pch->publicHead->UUID, devCtx->pPhyDevice->phyDevProp.pipelineCacheUUID, VK_UUID_SIZE);

    pch->privateHead = (__vkPipelineCachePrivateHead*)(pch->publicHead + 1);
    pch->privateHead->headBytes = (uint32_t)sizeof(__vkPipelineCachePrivateHead);
    pch->privateHead->bigEndian = 0;
    pch->privateHead->osInfo    = 0x00000000;
    pch->privateHead->patchID   = (uint32_t)patchID;
    pch->privateHead->version   = 0x00000000;
    gcoOS_StrCopySafe((gctSTRING)pch->privateHead->drvVersion, 32, gcvVERSION_STRING);

    pch->moduleHash = __vk_utils_hashCreate(pMemCb, 16, __VK_PIPELINE_CACHE_HASH_ENTRY_NUM,
                                            __VK_PIPELINE_CACHE_HASH_ENTRY_SIZE, __vkPipelineCacheFreeModule);
    __VK_ONERROR(pch->moduleHash ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    if (pCreateInfo->pInitialData && pCreateInfo->initialDataSize > pch->totalBytes)
    {
        __vkInitPipelineCacheData(devCtx, pch, pCreateInfo->pInitialData, pCreateInfo->initialDataSize);
    }

    /* Return the object pointer as a 64-bit handle */
    *pPipelineCache = (VkPipelineCache)(uintptr_t)pch;

OnError:
    if (result != VK_SUCCESS)
    {
        __vk_DestroyPipelineCache(device, (VkPipelineCache)(uintptr_t)pch, pAllocator);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyPipelineCache(
    VkDevice device,
    VkPipelineCache pipelineCache,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineCache *pch = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache *, pipelineCache);

    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (pch)
    {
        if (pch->moduleHash)
        {
            __vk_utils_hashDestory(pMemCb, pch->moduleHash);
        }

        if (pch->publicHead)
        {
            __VK_FREE(pch->publicHead);
            pch->publicHead = gcvNULL;
            pch->privateHead = gcvNULL;
        }

        if (pch->mutex)
        {
            gcoOS_DeleteMutex(gcvNULL, pch->mutex);
        }

        pch->numModules  = 0;
        pch->totalBytes  = 0;

        __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE_CACHE, (__vkObject *)pch);
    }
}

typedef struct __vkPlcGetDataCtxRec
{
    const VkAllocationCallbacks* pAllocator;
    uint32_t numModules;
    size_t maxBytes;
    size_t accumBytes;
    uint8_t *pData;
} __vkPlcGetDataCtx;

static VkResult __vkPlcGetData(
    void *pContext,
    void *pUserData
    )
{
    const size_t headBytes = sizeof(__vkModuleCacheHead);
    __vkPlcGetDataCtx* pCtx = (__vkPlcGetDataCtx*)pContext;
    __vkModuleCacheEntry *pEntry = (__vkModuleCacheEntry*)pUserData;
    uint32_t binBytes = pEntry->head.binBytes;
    VkResult result = VK_SUCCESS;

    if (pCtx->accumBytes + headBytes + (size_t)pEntry->head.alignBytes > pCtx->maxBytes)
    {
        __VK_ONERROR(VK_INCOMPLETE);
    }

    __VK_MEMCOPY(pCtx->pData, &pEntry->head, headBytes);
    pCtx->pData      += headBytes;
    pCtx->accumBytes += headBytes;

    __VK_ONERROR(vscSaveShaderToBinary(pEntry->handle->vscHandle, (void**)&pCtx->pData, &binBytes));
    /* binBytes might be overwritten when save binary */
    __VK_ASSERT(binBytes <= pEntry->head.binBytes);
    pCtx->pData      += pEntry->head.alignBytes;
    pCtx->accumBytes += pEntry->head.alignBytes;
    pCtx->numModules++;

OnError:
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPipelineCacheData(
    VkDevice device,
    VkPipelineCache pipelineCache,
    size_t* pDataSize,
    void* pData
    )
{
    __vkPipelineCache *pch = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache*, pipelineCache);
    size_t bytes = pch->totalBytes;
    VkResult result = VK_SUCCESS;

    if (pData)
    {
        const size_t pubHeadBytes = sizeof(__vkPipelineCachePublicHead);
        const size_t headBytes = pubHeadBytes + sizeof(__vkPipelineCachePrivateHead);
        uint8_t *pCur = (uint8_t*)pData;
        size_t maxBytes = *pDataSize;
        __vkPipelineCachePrivateHead *pPrivateHead = gcvNULL;
        __vkPlcGetDataCtx context;

        bytes = 0;

        /* Need enough room for pub header, or return empty */
        if (pubHeadBytes <= maxBytes)
        {
            if (bytes + headBytes > maxBytes)
            {
                __VK_ONERROR(VK_INCOMPLETE);
            }
            pPrivateHead = (__vkPipelineCachePrivateHead*)(pCur + pubHeadBytes);

            gcoOS_AcquireMutex(gcvNULL, pch->mutex, gcvINFINITE);
            __VK_MEMCOPY(pCur, pch->publicHead, headBytes);
            pCur  += headBytes;
            bytes += headBytes;

            context.pAllocator = &pch->memCb;
            context.numModules = 0;
            context.maxBytes   = maxBytes;
            context.accumBytes = bytes;
            context.pData      = pCur;
            result = __vk_utils_hashTraverse(pch->moduleHash, (void*)&context, __vkPlcGetData);
            pCur  = context.pData;
            bytes = context.accumBytes;

            pPrivateHead->numModules = context.numModules;
            pPrivateHead->totalBytes = (uint64_t)bytes;
            gcoOS_ReleaseMutex(gcvNULL, pch->mutex);
        }
        else
        {
            __VK_ONERROR(VK_INCOMPLETE);
        }
    }

OnError:
    *pDataSize = bytes;

    return result;
}

static VkResult
__vkPlcMergeData(
    void *pContext,
    void *pUserData
    )
{
    __vkPipelineCache *pDstCache = (__vkPipelineCache*)pContext;
    __vkModuleCacheEntry *pEntry = (__vkModuleCacheEntry*)pUserData;
    VkResult result = VK_SUCCESS;

    if (__vk_utils_hashFindObjByKey(pDstCache->moduleHash, pEntry->head.hashKey))
    {
        __vkModuleCacheEntry *dstEntry = gcvNULL;
        __VK_SET_ALLOCATIONCB(&pDstCache->memCb);

        dstEntry = (__vkModuleCacheEntry*)__VK_ALLOC(sizeof(__vkModuleCacheEntry), 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
        __VK_ONERROR(dstEntry ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        __VK_MEMCOPY(dstEntry, pEntry, sizeof(__vkModuleCacheEntry));
        halti5_ReferenceVkShader(dstEntry->handle);

        if (__vk_utils_hashAddObj(pMemCb, pDstCache->moduleHash, dstEntry, pEntry->head.hashKey, VK_FALSE))
        {
            __VK_VERIFY_OK(halti5_DestroyVkShader(dstEntry->handle));
            __VK_FREE(dstEntry);
            __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        pDstCache->numModules++;
        pDstCache->totalBytes += (pEntry->head.headBytes + pEntry->head.alignBytes);
    }

OnError:
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_MergePipelineCaches(
    VkDevice device,
    VkPipelineCache dstCache,
    uint32_t srcCacheCount,
    const VkPipelineCache* pSrcCaches
    )
{
    uint32_t i;
    __vkPipelineCache *pDstCache = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache*, dstCache);
    VkResult result = VK_SUCCESS;

    gcoOS_AcquireMutex(gcvNULL, pDstCache->mutex, gcvINFINITE);
    for (i = 0; i < srcCacheCount; ++i)
    {
        __vkPipelineCache *pSrcCache = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache*, pSrcCaches[i]);

        gcoOS_AcquireMutex(gcvNULL, pSrcCache->mutex, gcvINFINITE);
        result = __vk_utils_hashTraverse(pSrcCache->moduleHash, (void*)pDstCache, __vkPlcMergeData);
        gcoOS_ReleaseMutex(gcvNULL, pSrcCache->mutex);

        __VK_ERR_BREAK(result);
    }
    gcoOS_ReleaseMutex(gcvNULL, pDstCache->mutex);

    return result;
}


static VkResult __vki_CreateGraphicsPipeline(
    VkDevice device,
    VkPipelineCache pipelineCache,
    const VkGraphicsPipelineCreateInfo* info,
    __vkPipeline *pip
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result;
    __VK_SET_ALLOCATIONCB(&pip->memCb);

    /* cache */
    pip->cache = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache*, pipelineCache);

    /* Pipeline layout */
    pip->pipelineLayout = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout*, info->layout);
    pip->flags = info->flags;
    /* type */
    pip->type = __VK_PIPELINE_TYPE_GRAPHICS;

    /* render pass */
    pip->renderPass = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, info->renderPass);
    pip->subPass = info->subpass;

    /* IA states */
    pip->topology = info->pInputAssemblyState->topology;
    pip->primitiveRestartEnable = info->pInputAssemblyState->primitiveRestartEnable;

    /* RS states */
    pip->rasterDiscard = (info->pRasterizationState->rasterizerDiscardEnable || (info->pRasterizationState->cullMode == VK_CULL_MODE_FRONT_AND_BACK));
    pip->depthBiasEnable = info->pRasterizationState->depthBiasEnable;
    pip->frontFace = info->pRasterizationState->frontFace;

    /* MSAA state */
    pip->msaaEnabled = info->pMultisampleState ?
        (info->pMultisampleState->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT) : VK_FALSE;

    /* dynamic states */
    pip->dynamicStates = 0;
    if (info->pDynamicState != NULL)
    {
        uint32_t i;
        for (i = 0; i < info->pDynamicState->dynamicStateCount; i++)
        {
            switch (info->pDynamicState->pDynamicStates[i])
            {
            case VK_DYNAMIC_STATE_VIEWPORT:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_VIEWPORT_BIT;
                break;
            case VK_DYNAMIC_STATE_SCISSOR:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_SCISSOR_BIT;
                break;
            case VK_DYNAMIC_STATE_LINE_WIDTH:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_LINE_WIDTH_BIT;
                break;
            case VK_DYNAMIC_STATE_DEPTH_BIAS:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_DEPTH_BIAS_BIT;
                break;
            case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_BLEND_CONSTANTS_BIT;
                break;
            case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_DEPTH_BOUNDS_BIT;
                break;
            case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT;
                break;
            case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT;
                break;
            case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
                pip->dynamicStates |= __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT;
                break;
            default:
                __VK_ASSERT(0);
                break;
            }
        }
    }

    if ((!(pip->dynamicStates & __VK_DYNAMIC_STATE_SCISSOR_BIT))
        && info->pViewportState)
    {
        pip->scissorState.scissorCount = info->pViewportState->scissorCount;
        __VK_MEMCOPY(&pip->scissorState.scissors[0], info->pViewportState->pScissors,
            info->pViewportState->scissorCount * sizeof(VkRect2D));
    }

    if ((!(pip->dynamicStates & __VK_DYNAMIC_STATE_VIEWPORT_BIT))
        && info->pViewportState)
    {
        pip->viewportState.viewportCount = info->pViewportState->viewportCount;
        __VK_MEMCOPY(&pip->viewportState.viewports[0], info->pViewportState->pViewports,
            info->pViewportState->viewportCount * sizeof(VkRect2D));
    }

    if (info->pDepthStencilState)
    {
        __VK_MEMCOPY(&pip->dsInfo, info->pDepthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo));
    }

    if (info->pTessellationState)
    {
        pip->patchControlPoints = info->pTessellationState->patchControlPoints;
    }
    else
    {
        pip->patchControlPoints = 0;
    }

    /* Color blend attachment states */
    if (info->pColorBlendState)
    {
        pip->blendAttachmentCount  = info->pColorBlendState->attachmentCount;
        if (pip->blendAttachmentCount)
        {
            pip->blendAttachments = (VkPipelineColorBlendAttachmentState *)__VK_ALLOC(
                pip->blendAttachmentCount * sizeof(VkPipelineColorBlendAttachmentState), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR((pip->blendAttachments != VK_NULL_HANDLE) ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

            __VK_MEMCOPY(pip->blendAttachments, info->pColorBlendState->pAttachments,
                pip->blendAttachmentCount * sizeof(VkPipelineColorBlendAttachmentState));
        }
    }

    __VK_ONERROR((*devCtx->chipFuncs->CreateGraphicsPipeline)(device, info, (VkPipeline)(uintptr_t)pip));

    return VK_SUCCESS;

OnError:

    if (pip->blendAttachments)
    {
        __VK_FREE(pip->blendAttachments);
    }

    return result;

}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateGraphicsPipelines(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t i;
    VkResult retVal = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    for (i = 0; i < createInfoCount; i++)
    {
        __vkPipeline *pip =  VK_NULL_HANDLE;
        VkResult result;

        do
        {
            __VK_ERR_BREAK(__vk_CreateObject(devCtx, __VK_OBJECT_PIPELINE, sizeof(__vkPipeline), (__vkObject**)&pip));
#if __VK_RESOURCE_INFO
            {
                static int32_t tgtPipelineID = 0;
                if (tgtPipelineID == pip->obj.id)
                {
                   /* make compiler happy */
                   int32_t tmpID = tgtPipelineID;
                   tgtPipelineID = tmpID;
                }
            }
#endif

            pip->memCb = __VK_ALLOCATIONCB;

            pip->devCtx = devCtx;

            __VK_ERR_BREAK(__vki_CreateGraphicsPipeline(device, pipelineCache, &pCreateInfos[i], pip));

        } while (VK_FALSE);

        if (pip)
        {
            if (result != VK_SUCCESS)
            {
                __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE, (__vkObject *)pip);
                pip = VK_NULL_HANDLE;
                retVal = result;
            }
        }

        pPipelines[i] = (VkPipeline)(uintptr_t)pip;
    }

    return retVal;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateComputePipelines(
    VkDevice device,
    VkPipelineCache pipelineCache,
    uint32_t createInfoCount,
    const VkComputePipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator,
    VkPipeline* pPipelines
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t i;
    VkResult retVal = VK_SUCCESS;
    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    for (i = 0; i < createInfoCount; i++)
    {
        VkResult result;
        __vkPipeline *pip = VK_NULL_HANDLE;
        const VkComputePipelineCreateInfo *info = &pCreateInfos[i];

        do
        {
            __VK_ERR_BREAK(__vk_CreateObject(devCtx, __VK_OBJECT_PIPELINE, sizeof(__vkPipeline), (__vkObject**)&pip));
#if __VK_RESOURCE_INFO
            {
                static int32_t tgtPipelineID = 0;
                if (tgtPipelineID == pip->obj.id)
                {
                   /* make compiler happy */
                   int32_t tmpID = tgtPipelineID;
                   tgtPipelineID = tmpID;
                }
            }
#endif

            pip->memCb = __VK_ALLOCATIONCB;
            pip->devCtx = devCtx;
            pip->flags = pCreateInfos->flags;
            /* cache */
            pip->cache = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache*, pipelineCache);
            /* Pipeline layout */
            pip->pipelineLayout = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout*, info->layout);
            /* type */
            pip->type = __VK_PIPELINE_TYPE_COMPUTE;

            __VK_ERR_BREAK((*devCtx->chipFuncs->CreateComputePipeline)(device, info, (VkPipeline)(uintptr_t)pip));

        } while (VK_FALSE);

        if (pip)
        {
            if (result != VK_SUCCESS)
            {
                __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE, (__vkObject *)pip);
                pip = VK_NULL_HANDLE;
                retVal = result;
            }
        }
        pPipelines[i] = (VkPipeline)(uintptr_t)pip;
    }

    return retVal;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyPipeline(
    VkDevice device,
    VkPipeline pipeline,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;

    if (pipeline)
    {
        __vkPipeline *pip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (pip->blendAttachments)
            __VK_FREE(pip->blendAttachments);

        __VK_VERIFY_OK((*devCtx->chipFuncs->DestroyPipeline)(device, pipeline));

        __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE, (__vkObject *)pip);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreatePipelineLayout(
    VkDevice device,
    const VkPipelineLayoutCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkPipelineLayout* pPipelineLayout
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineLayout *plt;
    VkResult result;
    uint32_t i;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    result = __vk_CreateObject(devCtx, __VK_OBJECT_PIPELINE_LAYOUT, sizeof(__vkPipelineLayout), (__vkObject**)&plt);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    /* Initialize __vkPipelineLayout specific data fields here */
    plt->descSetLayoutCount = pCreateInfo->setLayoutCount;

    if (plt->descSetLayoutCount > 0)
    {
        plt->descSetLayout = (__vkDescriptorSetLayout **) __VK_ALLOC(
            plt->descSetLayoutCount * sizeof(__vkDescriptorSetLayout *),
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR((plt->descSetLayout ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

        plt->dynamic_index = (uint32_t *)__VK_ALLOC(
            plt->descSetLayoutCount * sizeof(uint32_t),
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR((plt->dynamic_index ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

        for (i = 0; i < plt->descSetLayoutCount; i++)
        {
            plt->descSetLayout[i] = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetLayout *, pCreateInfo->pSetLayouts[i]);
            plt->dynamic_index[i] = plt->total_dynamic_index;
            plt->total_dynamic_index += plt->descSetLayout[i]->dynamicDescriptorCount;
        }
    }

    __VK_ASSERT(plt->total_dynamic_index <= (__VK_MAX_UNIFORM_BUFFER_DYNAMIC + __VK_MAX_STORAGE_BUFFER_DYNAMIC));

    plt->pushConstantRangeCount = pCreateInfo->pushConstantRangeCount;

    if (plt->pushConstantRangeCount)
    {
        plt->pushConstantRanges = (VkPushConstantRange *)__VK_ALLOC(
            plt->pushConstantRangeCount * sizeof(VkPushConstantRange),
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR((plt->pushConstantRanges ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

        __VK_MEMCOPY(plt->pushConstantRanges,
            pCreateInfo->pPushConstantRanges,
            plt->pushConstantRangeCount * sizeof(VkPushConstantRange));

    }

    /* Return the object pointer as a 64-bit handle */
    *pPipelineLayout = (VkPipelineLayout)(uintptr_t)plt;

    return VK_SUCCESS;

OnError:

    if (plt->descSetLayout)
    {
        __VK_FREE(plt->descSetLayout);
    }

    if (plt->dynamic_index)
    {
        __VK_FREE(plt->dynamic_index);
    }

    if (plt->pushConstantRanges)
    {
        __VK_FREE(plt->pushConstantRanges);
    }

    if (plt)
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE_LAYOUT, (__vkObject *)plt);
    }

    return result;

}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyPipelineLayout(
    VkDevice device,
    VkPipelineLayout pipelineLayout,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (pipelineLayout)
    {
        __vkPipelineLayout *plt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout *, pipelineLayout);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (plt->descSetLayout)
        {
            __VK_FREE(plt->descSetLayout);
        }
        if (plt->dynamic_index)
        {
            __VK_FREE(plt->dynamic_index);
        }
        if (plt->pushConstantRanges)
        {
            __VK_FREE(plt->pushConstantRanges);
        }
        __vk_DestroyObject(devCtx, __VK_OBJECT_PIPELINE_LAYOUT, (__vkObject *)plt);
    }
}


