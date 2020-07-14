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


VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateFramebuffer(
    VkDevice device,
    const VkFramebufferCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFramebuffer* pFramebuffer
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFramebuffer *fb;
    VkResult result;
    uint32_t i;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_FRAMEBUFFER,
        sizeof(__vkFramebuffer), (__vkObject**)&fb));

    fb->renderPass = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, pCreateInfo->renderPass);
    fb->width = pCreateInfo->width;
    fb->height = pCreateInfo->height;
    fb->layers = pCreateInfo->layers; /* clamp to max array size of image views */
    fb->attachmentCount = pCreateInfo->attachmentCount;

    if (fb->attachmentCount > 0)
    {
        fb->imageViews = (__vkImageView **)__VK_ALLOC(
            sizeof(__vkImageView *) * fb->attachmentCount, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR(fb->imageViews ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        for (i = 0; i < fb->attachmentCount; i++)
        {
            fb->imageViews[i] = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, pCreateInfo->pAttachments[i]);
        }
    }

    *pFramebuffer = (VkFramebuffer)(uintptr_t)fb;

    return VK_SUCCESS;

OnError:
    if (fb)
    {
        if (fb->imageViews)
        {
            __VK_FREE(fb->imageViews);
        }
        __vk_DestroyObject(devCtx, __VK_OBJECT_FRAMEBUFFER, (__vkObject *)fb);
    }

    return result;

}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyFramebuffer(
    VkDevice device,
    VkFramebuffer framebuffer,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;

    if (framebuffer)
    {
        __vkFramebuffer *fb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFramebuffer *, framebuffer);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (fb->imageViews)
        {
            __VK_FREE(fb->imageViews);
        }
        __vk_DestroyObject(devCtx, __VK_OBJECT_FRAMEBUFFER, (__vkObject *)fb);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateRenderPass(
    VkDevice device,
    const VkRenderPassCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkRenderPass* pRenderPass
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkRenderPass *rdp;
    VkResult result;
    uint32_t i;
    uint32_t allocSize;
    VkRenderPassMultiviewCreateInfo *multiViewCreateInfo = VK_NULL_HANDLE;
    VkRenderPassMultiviewCreateInfo *tempCreateInfo = VK_NULL_HANDLE;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    __VK_ONERROR(__vk_CreateObject(devCtx,
        __VK_OBJECT_RENDER_PASS, sizeof(__vkRenderPass), (__vkObject**)&rdp));

    tempCreateInfo = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkRenderPassMultiviewCreateInfo *, pCreateInfo->pNext);
    /*judge the pNext whether point to a VkRenderPassMultiviewCreateInfo structure */
    if (tempCreateInfo && tempCreateInfo->sType == VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO)
    {
        multiViewCreateInfo = tempCreateInfo;
    }

    rdp->attachmentCount = pCreateInfo->attachmentCount;
    rdp->subPassInfoCount = pCreateInfo->subpassCount;
    rdp->dependencyCount =  pCreateInfo->dependencyCount;
    allocSize = sizeof(__vkAttachmentDesc) * rdp->attachmentCount +
                sizeof(__vkRenderSubPassInfo) * rdp->subPassInfoCount +
                sizeof(VkSubpassDependency) * rdp->dependencyCount +
                sizeof(__vkRenderPassMultiViewInfo);
    rdp->attachments = (__vkAttachmentDesc *)__VK_ALLOC(allocSize, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR(rdp->attachments ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(rdp->attachments, allocSize);

    rdp->subPassInfo = (__vkRenderSubPassInfo *) (rdp->attachments + rdp->attachmentCount);

    if (rdp->dependencyCount)
    {
        rdp->pDependencies = (VkSubpassDependency *) (rdp->subPassInfo + rdp->subPassInfoCount);
        rdp->multiViewInfo = (__vkRenderPassMultiViewInfo *) (rdp->pDependencies + rdp->dependencyCount);
    }
    else
    {
        rdp->pDependencies = VK_NULL_HANDLE;
        rdp->multiViewInfo = (__vkRenderPassMultiViewInfo *) (rdp->subPassInfo + rdp->subPassInfoCount);
    }

    for (i = 0; i < rdp->attachmentCount; i++)
    {
        __vkAttachmentDesc *desc = &rdp->attachments[i];
        uint32_t residentFormat;
        desc->format = pCreateInfo->pAttachments[i].format;
        desc->finalLayout = pCreateInfo->pAttachments[i].finalLayout;
        desc->initialLayout = pCreateInfo->pAttachments[i].initialLayout;
        desc->sampleCount = pCreateInfo->pAttachments[i].samples;
        residentFormat = __vk_GetVkFormatInfo(desc->format)->residentImgFormat;
        desc->formatInfo = __vk_GetVkFormatInfo((VkFormat) residentFormat);

        if (desc->sampleCount == 4 && desc->formatInfo->bitsPerBlock == 64 &&
            !devCtx->msaa_64bpp)
        {
            switch (desc->formatInfo->residentImgFormat)
            {
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                residentFormat = __VK_FORMAT_R16G16B16A16_SFLOAT_2_R16G16_SFLOAT;
                break;
            case VK_FORMAT_R16G16B16A16_SINT:
                residentFormat = __VK_FORMAT_R16G16B16A16_SINT_2_R16G16_SINT;
                break;
            case VK_FORMAT_R16G16B16A16_UINT:
                residentFormat = __VK_FORMAT_R16G16B16A16_UINT_2_R16G16_UINT;
                break;
            case VK_FORMAT_R32G32_SFLOAT:
                residentFormat = __VK_FORMAT_R32G32_SFLOAT_2_R32_SFLOAT;
                break;
            case VK_FORMAT_R32G32_SINT:
                residentFormat = __VK_FORMAT_R32G32_SINT_2_R32_SINT;
                break;
            case VK_FORMAT_R32G32_UINT:
                residentFormat = __VK_FORMAT_R32G32_UINT_2_R32_UINT;
                break;
            default:
                __VK_ASSERT(!"invalid format!");
                break;
            }
            desc->formatInfo = __vk_GetVkFormatInfo((VkFormat) residentFormat);
        }

        switch (desc->format)
        {
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            desc->stencil_ignoreStore =
                (pCreateInfo->pAttachments[i].stencilStoreOp == VK_ATTACHMENT_STORE_OP_DONT_CARE);
            desc->stencil_loadClear =
                (pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR);

        default:
            desc->ignoreStore =
                (pCreateInfo->pAttachments[i].storeOp == VK_ATTACHMENT_STORE_OP_DONT_CARE);
            desc->loadClear =
                (pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR);
            break;
        }
    }

    for (i = 0; i < rdp->subPassInfoCount; i++)
    {
        __vkRenderSubPassInfo *subPassInfo = &rdp->subPassInfo[i];
        const VkSubpassDescription *createInfo = &pCreateInfo->pSubpasses[i];
        uint32_t j;
        const VkAttachmentReference *ds_ref = createInfo->pDepthStencilAttachment;

        subPassInfo->colorCount = createInfo->colorAttachmentCount;
        subPassInfo->inputCount = createInfo->inputAttachmentCount;

        /* Initialize attachment index to UNUSED */
        for (j = 0; j < __VK_MAX_RENDER_TARGETS; j++)
        {
            subPassInfo->color_attachment_index[j] = VK_ATTACHMENT_UNUSED;
            subPassInfo->resolve_attachment_index[j] = VK_ATTACHMENT_UNUSED;
            subPassInfo->input_attachment_index[j] = VK_ATTACHMENT_UNUSED;
        }

        for (j = 0; j < subPassInfo->colorCount; j++)
        {
            const VkAttachmentReference *color_ref = &createInfo->pColorAttachments[j];
            const VkAttachmentReference *resolve_ref =
                createInfo->pResolveAttachments ? &createInfo->pResolveAttachments[j] : NULL;

            if (color_ref->attachment != VK_ATTACHMENT_UNUSED)
            {
                __vkAttachmentDesc *desc = &rdp->attachments[color_ref->attachment];
                desc->usedInRenderPass = VK_TRUE;
            }

            subPassInfo->color_attachment_index[j] = color_ref->attachment;
            subPassInfo->color_attachment_imageLayout[j] = color_ref->layout;

            if (resolve_ref)
            {
                if (resolve_ref->attachment != VK_ATTACHMENT_UNUSED)
                {
                    __vkAttachmentDesc *desc = &rdp->attachments[resolve_ref->attachment];
                    desc->usedInRenderPass = VK_TRUE;
                }

                subPassInfo->resolve_attachment_index[j] = resolve_ref->attachment;
                subPassInfo->resolve_attachment_imageLayout[j] = resolve_ref->layout;
            }
            else
            {
                subPassInfo->resolve_attachment_index[j] = VK_ATTACHMENT_UNUSED;
                subPassInfo->resolve_attachment_imageLayout[j] = VK_IMAGE_LAYOUT_UNDEFINED;
            }

        }

        for (j = 0;  j < subPassInfo->inputCount; j++)
        {
            const VkAttachmentReference *input_ref =
                createInfo->pInputAttachments ? &createInfo->pInputAttachments[j] : NULL;

            if (input_ref)
            {
                if (input_ref->attachment != VK_ATTACHMENT_UNUSED)
                {
                    __vkAttachmentDesc *desc = &rdp->attachments[input_ref->attachment];
                    desc->usedInRenderPass = VK_TRUE;
                }

                subPassInfo->input_attachment_index[j] = input_ref->attachment;
                subPassInfo->input_attachment_imageLayout[j] = input_ref->layout;
            }
            else
            {
                subPassInfo->input_attachment_index[j] = VK_ATTACHMENT_UNUSED;
                subPassInfo->input_attachment_imageLayout[j] = VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }

        if (ds_ref)
        {
            if (ds_ref->attachment != VK_ATTACHMENT_UNUSED)
            {
                __vkAttachmentDesc *desc = &rdp->attachments[ds_ref->attachment];
                desc->usedInRenderPass = VK_TRUE;
            }

            subPassInfo->dsAttachIndex = ds_ref->attachment;
            subPassInfo->dsImageLayout = ds_ref->layout;
        }
        else
        {
            subPassInfo->dsAttachIndex = VK_ATTACHMENT_UNUSED;
            subPassInfo->dsImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    for (i = 0; i < pCreateInfo->dependencyCount; i++)
    {
        rdp->pDependencies[i] = pCreateInfo->pDependencies[i];
    }

    /*record the multiView info if enable the multiView*/
    if (multiViewCreateInfo)
    {
        __vkSubpassViewInfo * pViewInfo = VK_NULL_HANDLE;
        uint32_t size = sizeof(__vkSubpassViewInfo) * rdp->subPassInfoCount;
        __VK_ASSERT(multiViewCreateInfo->subpassCount == rdp->subPassInfoCount);

        rdp->multiViewInfo = (__vkRenderPassMultiViewInfo *)__VK_ALLOC(
            sizeof(__vkRenderPassMultiViewInfo), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        __VK_ONERROR(rdp->multiViewInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(rdp->multiViewInfo, sizeof(__vkRenderPassMultiViewInfo));

        rdp->multiViewInfo->subPassViewInfo = (__vkSubpassViewInfo *)__VK_ALLOC(
            size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        __VK_ONERROR(rdp->multiViewInfo->subPassViewInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        pViewInfo= rdp->multiViewInfo->subPassViewInfo;
        __VK_MEMZERO(pViewInfo, size);

        rdp->multiViewInfo->enabledMultiView = VK_TRUE;
        rdp->multiViewInfo->dependencyCount = multiViewCreateInfo->dependencyCount;
        rdp->multiViewInfo->correlationMaskCount = multiViewCreateInfo->correlationMaskCount;

        for (i = 0; i < multiViewCreateInfo->subpassCount; i++)
        {
            uint32_t viewMask = multiViewCreateInfo->pViewMasks[i];
            uint32_t j;

            if (viewMask)
            {
                uint32_t count = 0;
                for (j = 0; j < __VK_MAX_MULTIVIEW_VIEW; j++)
                {
                    if (viewMask & (1<<j))
                    {
                        pViewInfo[i].enabledViewIdx[count++] = j;
                    }
                }
                pViewInfo[i].validViewCount = count;
                pViewInfo[i].curLayer = 0;

                if (count > 0)
                {
                    pViewInfo[i].validSubpassView = VK_TRUE;
                }
                else
                {
                    pViewInfo[i].validSubpassView = VK_FALSE;
                }
            }
        }

        for (i = 0; i <multiViewCreateInfo->dependencyCount; i++)
        {
            rdp->multiViewInfo->pViewOffsets[i] = multiViewCreateInfo->pViewOffsets[i];
        }

        for (i = 0; i < multiViewCreateInfo->correlationMaskCount; i++)
        {
            rdp->multiViewInfo->pCorrelationMasks[i] = multiViewCreateInfo->pCorrelationMasks[i];
        }
    }

    /* Return the object pointer as a 64-bit handle */
    *pRenderPass = (VkRenderPass)(uintptr_t)rdp;

    return VK_SUCCESS;
OnError:
    if (rdp)
    {
        if (rdp->attachments)
        {
            __VK_FREE(rdp->attachments);
        }
        __vk_DestroyObject(devCtx, __VK_OBJECT_RENDER_PASS, (__vkObject *)rdp);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyRenderPass(
    VkDevice device,
    VkRenderPass renderPass,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (renderPass)
    {
        __vkRenderPass *rdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, renderPass);

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        if (rdp->attachments)
        {
            __VK_FREE(rdp->attachments);
            rdp->attachments = NULL;
            rdp->subPassInfo = NULL;
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_RENDER_PASS, (__vkObject *)rdp);
    }
}


