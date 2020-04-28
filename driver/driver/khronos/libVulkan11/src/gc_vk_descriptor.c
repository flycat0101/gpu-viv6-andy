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


#include "gc_vk_precomp.h"

__VK_INLINE VkResult __vki_AllocateDescriptorSet(
    __vkDevContext *devCtx,
    __vkDescriptorPool *dsp,
    __vkDescriptorSetLayout *setLayout,
    __vkDescriptorSet *descSet
    )
{

    __vkDescriptorResourceRegion remainSize;
    VkResult result = VK_SUCCESS;

    __vk_utils_region_minus(&remainSize, &dsp->size, &dsp->cur);

    if (__vk_utils_region_gequal(&remainSize, &setLayout->size))
    {
        __vk_utils_region_copy(&descSet->begin, &dsp->cur);
        __vk_utils_region_copy(&descSet->size, &setLayout->size);
        __vk_utils_region_add(&dsp->cur, &dsp->cur, &descSet->size);

        descSet->resInfos = (__vkDescriptorResourceInfo *)((uint8_t *)dsp->resourceInfo + descSet->begin.resource);
        __VK_MEMZERO(descSet->resInfos, descSet->size.resource);
        descSet->samplers = (__vkSampler **)((uint8_t *)dsp->sampler + descSet->begin.sampler);
        __VK_MEMZERO(descSet->samplers, descSet->size.sampler);

        dsp->allocatedSets++;

        __VK_ONERROR((*devCtx->chipFuncs->AllocDescriptorSet)((VkDevice)(uintptr_t)devCtx, (VkDescriptorSet)(uintptr_t)descSet));
    }
    else
    {
        __VK_ONERROR(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
    }

    return VK_SUCCESS;

OnError:
    return result;
}

__VK_INLINE void __vki_get_region_size(
    __vkDescriptorResourceRegion *region,
    VkDescriptorType type
    )
{
    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        /* vkSampler only, don't need resource */
        __vk_utils_region_set(region, 0, sizeof(__vkSampler *));
        break;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        /* Need both sampler and resource */
        __vk_utils_region_set(region, sizeof(__vkDescriptorResourceInfo), sizeof(__vkSampler *));
        break;

    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        /* Don't need vkSampler */
        __vk_utils_region_set(region, sizeof(__vkDescriptorResourceInfo), 0);
        break;

    default:
        __VK_ASSERT(0);
        break;
    }
}

static void __vki_FreeDescriptorSet(
    __vkDevContext *devCtx,
    __vkDescriptorSet *descSet
    )
{
    __vkDescriptorPool *dsp = descSet->descriptorPool;
    __vkDescriptorResourceRegion freeRegionEnd;

    __vk_utils_region_add(&freeRegionEnd, &descSet->begin, &descSet->size);
    if (__vk_utils_region_equal(&freeRegionEnd, &dsp->cur))
    {
        dsp->cur = descSet->begin;
    }
    __vk_utils_region_set(&descSet->size, 0, 0);

    __VK_VERIFY_OK((*devCtx->chipFuncs->FreeDescriptorSet)((VkDevice)(uintptr_t)devCtx, (VkDescriptorSet)(uintptr_t)descSet));
    if (dsp->allocatedSets)
    {
        dsp->allocatedSets--;
    }
    return;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorPool(
    VkDevice device,
    const VkDescriptorPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorPool* pDescriptorPool
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t count = pCreateInfo->poolSizeCount;
    __vkDescriptorPool *dsp;
    __vkDescriptorResourceRegion size;
    VkResult result;
    uint32_t i;
    __vkDescriptorSet *des;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    result = __vk_CreateObject(devCtx, __VK_OBJECT_DESCRIPTOR_POOL, sizeof(__vkDescriptorPool), (__vkObject**)&dsp);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    /* Initialize __vkDescriptorPool specific data fields here */
    dsp->memCb = __VK_ALLOCATIONCB;
    dsp->flags = pCreateInfo->flags;
    dsp->maxSets = pCreateInfo->maxSets;
    dsp->allocatedSets = 0;
    __vk_utils_region_set(&size, 0, 0);
    for (i = 0; i < count; ++i)
    {
        __vkDescriptorResourceRegion entrySize;
        __vki_get_region_size(&entrySize, pCreateInfo->pPoolSizes[i].type);
        __vk_utils_region_mul(&entrySize, &entrySize, pCreateInfo->pPoolSizes[i].descriptorCount);
        __vk_utils_region_add(&size, &size, &entrySize);
    }

    if (size.resource)
    {
        dsp->resourceInfo = (__vkDescriptorResourceInfo *)__VK_ALLOC(
            size.resource, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR((dsp->resourceInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

        __VK_MEMZERO(dsp->resourceInfo, size.resource);
    }

    if (size.sampler)
    {
        dsp->sampler = (__vkSampler **)__VK_ALLOC(
            size.sampler, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR((dsp->sampler ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

        __VK_MEMZERO(dsp->sampler, size.sampler);
    }

    __vk_utils_region_set(&dsp->cur, 0, 0);
    dsp->size = size;

    dsp->pDescSets = (__vkDescriptorSetEntry *)__VK_ALLOC(dsp->maxSets * sizeof(__vkDescriptorSetEntry), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    if (!dsp->pDescSets)
    {
        goto OnError;
    }

    __VK_MEMZERO(dsp->pDescSets, dsp->maxSets * sizeof(__vkDescriptorSetEntry));

    for (i = 0; i < dsp->maxSets; i++)
    {
        __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_DESCRIPTORSET, sizeof(__vkDescriptorSet), (__vkObject**)&des));

        /* Initialize __vkDescriptorSet specific data fields here */
        des->descriptorPool = dsp;
        dsp->pDescSets[i].descSet = (VkDescriptorSet)(uintptr_t)des;
    }

    /* Return the object pointer as a 64-bit handle */
    *pDescriptorPool = (VkDescriptorPool)(uintptr_t)dsp;

    return VK_SUCCESS;

OnError:
    if (dsp->resourceInfo)
    {
        __VK_FREE(dsp->resourceInfo);
        dsp->resourceInfo = VK_NULL_HANDLE;
    }

    if (dsp->sampler)
    {
        __VK_FREE(dsp->sampler);
        dsp->sampler = VK_NULL_HANDLE;
    }

    if (dsp->pDescSets)
    {
        uint32_t i = 0;

        for (i = 0; i < dsp->maxSets; i++)
        {
            __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, dsp->pDescSets[i].descSet);
            if (descSet)
            {
                __VK_VERIFY_OK(__vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject *)descSet));
            }
        }

        __VK_FREE(dsp->pDescSets);
        dsp->pDescSets = VK_NULL_HANDLE;
    }

    if (dsp)
        __vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTOR_POOL, (__vkObject *)dsp);

    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyDescriptorPool(
    VkDevice device,
    VkDescriptorPool descriptorPool,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, descriptorPool);
    uint32_t i = 0;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (dsp)
    {
        __vk_ResetDescriptorPool(device, descriptorPool, 0);
        for (i = 0; i < dsp->maxSets; i++)
        {
            __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, dsp->pDescSets[i].descSet);
            __VK_VERIFY_OK(__vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject *)descSet));
        }

        /* Free __vkDescriptorSetEntry */
        if (dsp->pDescSets)
        {
            __VK_FREE(dsp->pDescSets);
        }
        /* Free __vkDescriptorPool specific data fields here */
        if (dsp->resourceInfo)
            __VK_FREE(dsp->resourceInfo);

        if (dsp->sampler)
            __VK_FREE(dsp->sampler);

        __vk_utils_region_set(&dsp->size, 0, 0);
    }

    __vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTOR_POOL, (__vkObject *)dsp);
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetDescriptorPool(
    VkDevice device,
    VkDescriptorPool descriptorPool,
    VkDescriptorPoolResetFlags flags
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, descriptorPool);

    uint32_t i = 0;
    for (i = 0; i < dsp->maxSets; i++)
    {
        if (dsp->pDescSets[i].isUsed)
        {
            __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, dsp->pDescSets[i].descSet);
            __vki_FreeDescriptorSet(devCtx, descSet);
            dsp->pDescSets[i].isUsed = VK_FALSE;
        }
    }

    __vk_utils_region_set(&dsp->cur, 0, 0);
    __VK_ASSERT(dsp->allocatedSets == 0);

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_AllocateDescriptorSets(
    VkDevice device,
    const VkDescriptorSetAllocateInfo* pAllocateInfo,
    VkDescriptorSet* pDescriptorSets
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, pAllocateInfo->descriptorPool);
    __vkDescriptorSet *des;
    VkResult result;
    uint32_t i;
    __VK_MEMSET(pDescriptorSets, VK_NULL_HANDLE, sizeof(VkDescriptorSet) * pAllocateInfo->descriptorSetCount);

    for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
    {
        uint32_t j;
        uint32_t k;
        __vkDescriptorSetLayout *pDescriptorSetLayout = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetLayout *, pAllocateInfo->pSetLayouts[i]);

        if (dsp->allocatedSets == dsp->maxSets)
        {
            return VK_ERROR_OUT_OF_POOL_MEMORY_KHR;
        }

        for (k = i; k < dsp->maxSets; k++)
        {
            if (!dsp->pDescSets[k].isUsed)
            {
                break;
            }
        }
        /* Initialize __vkDescriptorSet specific data fields here */
        des = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, dsp->pDescSets[k].descSet);
        dsp->pDescSets[k].isUsed = VK_TRUE;
        des->descSetLayout = pDescriptorSetLayout;

        /* Return the object pointer as a 64-bit handle */
        pDescriptorSets[i] = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorSet, &dsp->pDescSets[k]);

        __VK_ONERROR(__vki_AllocateDescriptorSet(devCtx, dsp, pDescriptorSetLayout, des));

        for (j = 0; j < pDescriptorSetLayout->bindingCount; j++)
        {
            __vkDescriptorSetLayoutBinding *binding = &pDescriptorSetLayout->binding[j];

            if (binding->std.pImmutableSamplers)
            {
                __vkSampler **samplers = (__vkSampler **)((uint8_t *)des->samplers + binding->offset.sampler);
                uint32_t j;
                for (j = 0; j < binding->std.descriptorCount; j++)
                {
                    samplers[j] = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, binding->std.pImmutableSamplers[j]);
                }
            }
        }
    }

    return VK_SUCCESS;

OnError:

    for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
    {
        if (pDescriptorSets[i])
        {
            __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pDescriptorSets[i]);
            __vkDescriptorSet *des = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
            __vki_FreeDescriptorSet(devCtx, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, des));
            pDescriptorSets[i] = VK_NULL_HANDLE;
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_FreeDescriptorSets(
    VkDevice device,
    VkDescriptorPool descriptorPool,
    uint32_t count,
    const VkDescriptorSet* pDescriptorSets
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t i;

    for (i = 0; i < count; i++)
    {
        if (pDescriptorSets[i])
        {
            __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pDescriptorSets[i]);
            __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
            __vki_FreeDescriptorSet(devCtx, descSet);
            pDescSets->isUsed = VK_FALSE;
        }
    }

    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __vk_UpdateDescriptorSets(
    VkDevice device,
    uint32_t writeCount,
    const VkWriteDescriptorSet* pDescriptorWrites,
    uint32_t copyCount,
    const VkCopyDescriptorSet* pDescriptorCopies
    )
{
    uint32_t i;

    /* Write descriptor */
    for (i = 0; i < writeCount; i++)
    {
        __vkDescriptorResourceRegion dstBegin, dstEnd, dstDelta;
        __vkDescriptorSetLayoutBinding *dstBinding = VK_NULL_HANDLE;
        __vkDescriptorResourceInfo *dstResource;
        __vkSampler **dstSampler;
        uint32_t index;
        uint32_t writeIndex = 0;
        VkWriteDescriptorSet *pWrite = (VkWriteDescriptorSet*)&pDescriptorWrites[i];
        __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pWrite->dstSet);
        __vkDescriptorSet *dstDesc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
        __vkDescriptorSetLayout *dstDescLayout = dstDesc->descSetLayout;
        __vkDescriptorPool *dstDescPool = dstDesc->descriptorPool;

        for (index = 0; index < dstDescLayout->bindingCount; index++)
        {
            dstBinding = &dstDescLayout->binding[index];
            if (dstBinding->std.binding == pWrite->dstBinding)
                break;
        }
        __VK_ASSERT(index < dstDescLayout->bindingCount);
        __vk_utils_region_mad(&dstBegin, &dstBinding->perElementSize, pWrite->dstArrayElement, &dstBinding->offset);
        __vk_utils_region_mad(&dstEnd, &dstBinding->perElementSize, pWrite->descriptorCount, &dstBegin);
        __VK_ASSERT(__vk_utils_region_gequal(&dstDescLayout->size, &dstEnd));
        __vk_utils_region_add(&dstBegin, &dstBegin, &dstDesc->begin);
        __vk_utils_region_add(&dstEnd, &dstEnd, &dstDesc->begin);
        __vk_utils_region_copy(&dstDelta, &dstBinding->perElementSize);
        dstResource = (__vkDescriptorResourceInfo*)((uint8_t *)dstDescPool->resourceInfo + dstBegin.resource);
        dstSampler =  (__vkSampler **)((uint8_t*)dstDescPool->sampler + dstBegin.sampler);
        __VK_ASSERT(dstBinding->std.descriptorType == pWrite->descriptorType);
        while (writeIndex < pWrite->descriptorCount)
        {
            switch (pWrite->descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                __VK_ASSERT(dstBinding->std.pImmutableSamplers == VK_NULL_HANDLE);
                *dstSampler = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, pWrite->pImageInfo[writeIndex].sampler);
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                if (!dstBinding->std.pImmutableSamplers)
                {
                    *dstSampler = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, pWrite->pImageInfo[writeIndex].sampler);
                }
                dstResource->u.imageInfo.imageView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, pWrite->pImageInfo[writeIndex].imageView);
                dstResource->u.imageInfo.layout = pWrite->pImageInfo[writeIndex].imageLayout;
                dstResource->type = __VK_DESC_RESOURCE_IMAGEVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                dstResource->u.imageInfo.imageView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, pWrite->pImageInfo[writeIndex].imageView);
                dstResource->u.imageInfo.layout = pWrite->pImageInfo[writeIndex].imageLayout;
                dstResource->type = __VK_DESC_RESOURCE_IMAGEVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                dstResource->u.bufferView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBufferView*, pWrite->pTexelBufferView[writeIndex]);
                dstResource->type = __VK_DESC_RESOURCE_BUFFERVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                dstResource->u.bufferInfo.buffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, pWrite->pBufferInfo[writeIndex].buffer);
                dstResource->u.bufferInfo.offset = pWrite->pBufferInfo[writeIndex].offset;
                dstResource->u.bufferInfo.range = pWrite->pBufferInfo[writeIndex].range;
                dstResource->type = __VK_DESC_RESOURCE_BUFFER_INFO;
                break;
            default:
                break;
            }
            dstResource = (__vkDescriptorResourceInfo *)((uint8_t *)dstResource + dstDelta.resource);
            dstSampler = (__vkSampler **)((uint8_t *)dstSampler + dstDelta.sampler);
            writeIndex++;
        }
        __VK_ASSERT((uint8_t *)dstResource <= ((uint8_t *)dstDescPool->resourceInfo + dstEnd.resource));
        __VK_ASSERT((uint8_t *)dstSampler <= ((uint8_t *)dstDescPool->sampler + dstEnd.sampler));
    }

    /* Copy descriptor */
    for (i = 0; i < copyCount; i++)
    {
        __vkDescriptorResourceRegion srcBegin, srcEnd, srcDelta;
        __vkDescriptorResourceRegion dstBegin, dstEnd, dstDelta;
        __vkDescriptorSetLayoutBinding *srcBinding = VK_NULL_HANDLE, *dstBinding = VK_NULL_HANDLE;
        __vkDescriptorResourceInfo *srcResource, *dstResource;
        __vkSampler **srcSampler, **dstSampler;
        uint32_t index;
        VkCopyDescriptorSet *pCopy = (VkCopyDescriptorSet*)&pDescriptorCopies[i];
        __vkDescriptorSetEntry *srcpDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pCopy->srcSet);
        __vkDescriptorSet *srcDesc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, srcpDescSets->descSet);
        __vkDescriptorSetEntry *dstpDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pCopy->dstSet);
        __vkDescriptorSet *dstDesc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, dstpDescSets->descSet);
        __vkDescriptorSetLayout *srcDescLayout = srcDesc->descSetLayout;
        __vkDescriptorSetLayout *dstDescLayout = dstDesc->descSetLayout;
        __vkDescriptorPool *srcDescPool = srcDesc->descriptorPool;
        __vkDescriptorPool *dstDescPool = dstDesc->descriptorPool;

        for (index = 0; index < srcDescLayout->bindingCount; index++)
        {
            srcBinding = &srcDescLayout->binding[index];
            if (srcBinding->std.binding == pCopy->srcBinding)
                break;
        }
        __VK_ASSERT(index < srcDescLayout->bindingCount);
        for (index = 0; index < dstDescLayout->bindingCount; index++)
        {
            dstBinding = &dstDescLayout->binding[index];
            if (dstBinding->std.binding == pCopy->dstBinding)
                break;
        }
        __VK_ASSERT(index < dstDescLayout->bindingCount);

        __vk_utils_region_mad(&srcBegin, &srcBinding->perElementSize, pCopy->dstArrayElement, &srcBinding->offset);
        __vk_utils_region_mad(&srcEnd, &srcBinding->perElementSize, pCopy->descriptorCount, &srcBegin);
        __VK_ASSERT(__vk_utils_region_gequal(&srcDescLayout->size, &srcEnd));
        __vk_utils_region_add(&srcBegin, &srcBegin, &srcDesc->begin);
        __vk_utils_region_add(&srcEnd, &srcEnd, &srcDesc->begin);
        __vk_utils_region_copy(&srcDelta, &srcBinding->perElementSize);
        srcResource = (__vkDescriptorResourceInfo*)((uint8_t *)srcDescPool->resourceInfo + srcBegin.resource);
        srcSampler = (__vkSampler **)((uint8_t*)srcDescPool->sampler + srcBegin.sampler);

        __VK_ASSERT(index < dstDescLayout->bindingCount);
        __vk_utils_region_mad(&dstBegin, &dstBinding->perElementSize, pCopy->dstArrayElement, &dstBinding->offset);
        __vk_utils_region_mad(&dstEnd, &dstBinding->perElementSize, pCopy->descriptorCount, &dstBegin);
        __VK_ASSERT(__vk_utils_region_gequal(&dstDescLayout->size, &dstEnd));
        __vk_utils_region_add(&dstBegin, &dstBegin, &dstDesc->begin);
        __vk_utils_region_add(&dstEnd, &dstEnd, &dstDesc->begin);
        __vk_utils_region_copy(&dstDelta, &dstBinding->perElementSize);
        dstResource = (__vkDescriptorResourceInfo*)((uint8_t *)dstDescPool->resourceInfo + dstBegin.resource);
        dstSampler = (__vkSampler **)((uint8_t*)dstDescPool->sampler + dstBegin.sampler);

        __VK_ASSERT(srcBinding->std.descriptorType == dstBinding->std.descriptorType);
        __VK_ASSERT(__vk_utils_region_equal(&srcDelta, &dstDelta));

        switch (srcBinding->std.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            __VK_ASSERT(dstBinding->std.pImmutableSamplers == VK_NULL_HANDLE);
            __VK_MEMCOPY(dstSampler, srcSampler, sizeof(__vkSampler *) * pCopy->descriptorCount);
            break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if (!dstBinding->std.pImmutableSamplers)
            {
                __VK_MEMCOPY(dstSampler, srcSampler, sizeof(__vkSampler *) * pCopy->descriptorCount);
            }
            __VK_MEMCOPY(dstResource, srcResource, sizeof(__vkDescriptorResourceInfo) * pCopy->descriptorCount);
            break;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            __VK_MEMCOPY(dstResource, srcResource, sizeof(__vkDescriptorResourceInfo) * pCopy->descriptorCount);
            break;
        default:
            break;
        }
    }

    /* update halti5 descriptor in __vk_CmdBindDescriptorSets to ensure all be updated */
    return;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorUpdateTemplate(
    VkDevice device,
    const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    __vkDescriptorUpdateTemplate *desUpdateTemplate = VK_NULL_HANDLE;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_DESCRIPTOR_UPDATE_TEMPLATE, sizeof(__vkDescriptorUpdateTemplate), (__vkObject**)&desUpdateTemplate));

    desUpdateTemplate->desUpdateTemplateEntry = (VkDescriptorUpdateTemplateEntry  *)__VK_ALLOC(
        pCreateInfo->descriptorUpdateEntryCount * sizeof(VkDescriptorUpdateTemplateEntry),
        8,
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR(desUpdateTemplate->desUpdateTemplateEntry ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(desUpdateTemplate->desUpdateTemplateEntry, pCreateInfo->descriptorUpdateEntryCount * sizeof(VkDescriptorUpdateTemplateEntry));

    desUpdateTemplate->desUpdateEntryCount = pCreateInfo->descriptorUpdateEntryCount;

    __VK_MEMCOPY(desUpdateTemplate->desUpdateTemplateEntry, pCreateInfo->pDescriptorUpdateEntries, pCreateInfo->descriptorUpdateEntryCount * sizeof(VkDescriptorUpdateTemplateEntry));


    *pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate)(uintptr_t)desUpdateTemplate;
    return result;

OnError:
    return VK_ERROR_OUT_OF_HOST_MEMORY;
}

VKAPI_ATTR void VKAPI_CALL __vk_UpdateDescriptorSetWithTemplate(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const void* pData)
{
    uint32_t index, i;
    __vkDescriptorResourceRegion dstBegin, dstEnd, dstDelta;
    __vkDescriptorResourceInfo *dstResource = VK_NULL_HANDLE;
    VkDescriptorImageInfo* pImageInfo = VK_NULL_HANDLE;
    VkBufferView* pTexelBufferView = VK_NULL_HANDLE;
    VkDescriptorBufferInfo* pBufferInfo = VK_NULL_HANDLE;
    __vkDescriptorUpdateTemplate * desUpdateTemplate = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorUpdateTemplate *, descriptorUpdateTemplate);
    __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, descriptorSet);
    __vkDescriptorSet *dstDesc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
    __vkDescriptorSetLayout *dstDescLayout = dstDesc->descSetLayout;
    __vkDescriptorSetLayoutBinding *dstBinding = VK_NULL_HANDLE;
    __vkSampler **dstSampler = VK_NULL_HANDLE;
    __vkDescriptorPool *dstDescPool = dstDesc->descriptorPool;

    for (i = 0; i < desUpdateTemplate->desUpdateEntryCount; i++)
    {
        uint32_t updateIndex = 0;
        VkDescriptorUpdateTemplateEntry * desUpdateEntries = (VkDescriptorUpdateTemplateEntry *)&desUpdateTemplate->desUpdateTemplateEntry[i];
        for (index = 0; index < dstDescLayout->bindingCount; index++)
        {
            dstBinding = &dstDescLayout->binding[index];
            if (dstBinding->std.binding == desUpdateEntries->dstBinding)
                break;
        }

        __VK_ASSERT(index < dstDescLayout->bindingCount);
        __VK_ASSERT(dstBinding->std.descriptorType == desUpdateEntries->descriptorType);

        __vk_utils_region_mad(&dstBegin, &dstBinding->perElementSize, desUpdateEntries->dstArrayElement, &dstBinding->offset);
        __vk_utils_region_mad(&dstEnd, &dstBinding->perElementSize, desUpdateEntries->descriptorCount, &dstBegin);
        __VK_ASSERT(__vk_utils_region_gequal(&dstDescLayout->size, &dstEnd));
        __vk_utils_region_add(&dstBegin, &dstBegin, &dstDesc->begin);
        __vk_utils_region_add(&dstEnd, &dstEnd, &dstDesc->begin);
        __vk_utils_region_copy(&dstDelta, &dstBinding->perElementSize);
        dstResource = (__vkDescriptorResourceInfo*)((uint8_t *)dstDescPool->resourceInfo + dstBegin.resource);
        dstSampler = (__vkSampler **)((uint8_t*)dstDescPool->sampler + dstBegin.sampler);
        __VK_ASSERT(dstBinding->std.descriptorType == desUpdateEntries->descriptorType);

        while (updateIndex < desUpdateEntries->descriptorCount)
        {
            switch (desUpdateEntries->descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                __VK_ASSERT(dstBinding->std.pImmutableSamplers == VK_NULL_HANDLE);
                pImageInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorImageInfo *,
                            ((size_t *)pData +(desUpdateEntries->offset / sizeof(size_t *))));
                *dstSampler = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, pImageInfo[updateIndex].sampler);
                break;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                if (!dstBinding->std.pImmutableSamplers)
                {
                    VkDescriptorImageInfo* pImageInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorImageInfo *,
                                                        ((size_t *)pData + (desUpdateEntries->offset / sizeof(size_t *))));
                    *dstSampler = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, pImageInfo[updateIndex].sampler);
                }
                pImageInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorImageInfo *,
                            ((size_t *)pData + (desUpdateEntries->offset / sizeof(size_t *))));
                dstResource->u.imageInfo.imageView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, pImageInfo[updateIndex].imageView);
                dstResource->u.imageInfo.layout = pImageInfo[updateIndex].imageLayout;
                dstResource->type = __VK_DESC_RESOURCE_IMAGEVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                pImageInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorImageInfo *,
                            ((size_t *)pData + (desUpdateEntries->offset / sizeof(size_t *))));
                dstResource->u.imageInfo.imageView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, pImageInfo[updateIndex].imageView);
                dstResource->u.imageInfo.layout = pImageInfo[updateIndex].imageLayout;
                dstResource->type = __VK_DESC_RESOURCE_IMAGEVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                pTexelBufferView = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkBufferView *,
                                  ((size_t *)pData + (desUpdateEntries->offset / sizeof(size_t *))));
                dstResource->u.bufferView = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBufferView*, pTexelBufferView[updateIndex]);
                dstResource->type = __VK_DESC_RESOURCE_BUFFERVIEW_INFO;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                pBufferInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkDescriptorBufferInfo *,
                             ((size_t *)pData + (desUpdateEntries->offset / sizeof(size_t *))));
                dstResource->u.bufferInfo.buffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, pBufferInfo[updateIndex].buffer);
                dstResource->u.bufferInfo.offset = pBufferInfo[updateIndex].offset;
                dstResource->u.bufferInfo.range = pBufferInfo[updateIndex].range;
                dstResource->type = __VK_DESC_RESOURCE_BUFFER_INFO;
                break;
            default:
                break;
            }
            dstResource = (__vkDescriptorResourceInfo *)((uint8_t *)dstResource + dstDelta.resource);
            dstSampler = (__vkSampler **)((uint8_t *)dstSampler + dstDelta.sampler);
            updateIndex++;
        }

        __VK_ASSERT((uint8_t *)dstResource <= ((uint8_t *)dstDescPool->resourceInfo + dstEnd.resource));
        __VK_ASSERT((uint8_t *)dstSampler <= ((uint8_t *)dstDescPool->sampler + dstEnd.sampler));
    }

    /* update halti5 descriptor in __vk_CmdBindDescriptorSets to ensure all be updated */
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyDescriptorUpdateTemplate(
    VkDevice device,
    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;

    if (descriptorUpdateTemplate)
    {
        __vkDescriptorUpdateTemplate * desUpdateTemplate = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorUpdateTemplate *, descriptorUpdateTemplate);

        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (desUpdateTemplate->desUpdateTemplateEntry)
        {
            __VK_FREE((void*)desUpdateTemplate->desUpdateTemplateEntry);
            desUpdateTemplate->desUpdateTemplateEntry = VK_NULL_HANDLE;
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTOR_UPDATE_TEMPLATE, (__vkObject *)desUpdateTemplate);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorSetLayout(
    VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDescriptorSetLayout* pSetLayout
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorSetLayout *dsl;
    VkResult result;
    __vkDescriptorResourceRegion offset;
    uint32_t totalEntries = 0;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, sizeof(__vkDescriptorSetLayout), (__vkObject**)&dsl));

    /* Initialize __vkDescriptorSetLayout specific data fields here */
    dsl->bindingCount = pCreateInfo->bindingCount;
    dsl->dynamicDescriptorCount = 0;

    if (dsl->bindingCount)
    {
        uint32_t i;
        dsl->binding = (__vkDescriptorSetLayoutBinding  *)__VK_ALLOC(
            dsl->bindingCount * sizeof(__vkDescriptorSetLayoutBinding),
            8,
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR(dsl->binding ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        __VK_MEMZERO(dsl->binding, dsl->bindingCount * sizeof(__vkDescriptorSetLayoutBinding));

        __vk_utils_region_set(&offset, 0, 0);

        for (i = 0; i < dsl->bindingCount; i++)
        {
            __vkDescriptorSetLayoutBinding *pBinding = &dsl->binding[i];
            uint32_t descriptorCount = pCreateInfo->pBindings[i].descriptorCount;

            if (pCreateInfo->pBindings[i].pImmutableSamplers)
            {
                pBinding->std.pImmutableSamplers = (VkSampler *)__VK_ALLOC(sizeof(VkSampler) * descriptorCount, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                __VK_ONERROR(pBinding->std.pImmutableSamplers ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_MEMCOPY(pBinding->std.pImmutableSamplers, pCreateInfo->pBindings[i].pImmutableSamplers, sizeof(VkSampler) *descriptorCount);
            }
            pBinding->std.binding = pCreateInfo->pBindings[i].binding;
            pBinding->std.descriptorCount = descriptorCount;
            pBinding->std.descriptorType =  pCreateInfo->pBindings[i].descriptorType;
            pBinding->std.stageFlags = pCreateInfo->pBindings[i].stageFlags;

            totalEntries += descriptorCount;

            switch (pBinding->std.descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                dsl->dynamicDescriptorCount += pBinding->std.descriptorCount;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                dsl->samplerDescriptorCount += pBinding->std.descriptorCount;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                dsl->samplerBufferDescriptorCount += pBinding->std.descriptorCount;
                break;
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                dsl->inputAttachmentDescriptorCount += pBinding->std.descriptorCount;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                dsl->storageDescriptorCount += pBinding->std.descriptorCount;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                break;
            default:
                break;
            }
            __vk_utils_region_copy(&pBinding->offset, &offset);
            __vki_get_region_size(&pBinding->perElementSize, pBinding->std.descriptorType);
            __vk_utils_region_mad(&offset, &pBinding->perElementSize, descriptorCount, &offset);

        }
        __vk_utils_region_copy(&dsl->size, &offset);
    }

    dsl->totalEntries = totalEntries;

    /* Return the object pointer as a 64-bit handle */
    *pSetLayout = (VkDescriptorSetLayout)(uintptr_t)dsl;

    __vk_InsertObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pSetLayout));

    return VK_SUCCESS;

OnError:
    if (dsl)
    {
        if (dsl->binding)
        {
            uint32_t i;
            for (i = 0; i < dsl->bindingCount; i++)
            {
                if (dsl->binding[i].std.pImmutableSamplers)
                {
                    __VK_FREE((void*)dsl->binding[i].std.pImmutableSamplers);
                }
            }

            __VK_FREE(dsl->binding);
        }
        __vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, (__vkObject *)dsl);
    }
    return result;

}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyDescriptorSetLayout(
    VkDevice device,
    VkDescriptorSetLayout descriptorSetLayout,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;

    if (descriptorSetLayout)
    {
        __vkDescriptorSetLayout *dsl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetLayout *, descriptorSetLayout);
        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (dsl->binding)
        {
            uint32_t i;
            for (i = 0; i < dsl->bindingCount; i++)
            {
                if (dsl->binding[i].std.pImmutableSamplers)
                {
                    __VK_FREE((void*)dsl->binding[i].std.pImmutableSamplers);
                }
            }
            __VK_FREE(dsl->binding);
            dsl->binding = NULL;
        }

        __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, (__vkObject*)dsl);
        __vk_DestroyObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, (__vkObject *)dsl);
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_GetDescriptorSetLayoutSupport(
    VkDevice device,
    const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
    VkDescriptorSetLayoutSupport* pSupport
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t desCount = 0;
    uint32_t i = 0;
    __vkPhysicalDevice * phyDev = devCtx->pPhyDevice;
    for (i = 0; i < pCreateInfo->bindingCount; i++)
    {
        desCount += pCreateInfo->pBindings[i].descriptorCount;
    }

    if (desCount > phyDev->phyDevMain3Prop.maxPerSetDescriptors)
    {
        pSupport->supported = VK_FALSE;
    }
    else
    {
        pSupport->supported = VK_TRUE;
    }
}



