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

VkResult halti3_helper_convertHwTxDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    HwTxDesc *hwTxDesc
    )
{
    static const struct
    {
        uint32_t hwType;
        VkBool32 hwIsArray;
    }
    s_xlateType[] =
    {
        /* VK_IMAGE_VIEW_TYPE_1D */
        {0x2, VK_FALSE},
        /* VK_IMAGE_VIEW_TYPE_2D */
        {0x2, VK_FALSE},
        /* VK_IMAGE_VIEW_TYPE_3D */
        {0x3, VK_FALSE},
        /* VK_IMAGE_VIEW_TYPE_CUBE */
        {0x5, VK_FALSE},
        /* VK_IMAGE_VIEW_TYPE_1D_ARRAY */
        {0x3, VK_TRUE},
        /* VK_IMAGE_VIEW_TYPE_2D_ARRAY */
        {0x3, VK_TRUE},
        /* VK_IMAGE_VIEW_TYPE_CUBE_ARRAY */
        {0x5, VK_TRUE},
    };

    VkResult result = VK_SUCCESS;
    VkImageSubresourceRange *subResourceRange = VK_NULL_HANDLE;
    __vkImageLevel *baseLevel = VK_NULL_HANDLE;
    const __vkFormatToHwTxFmtInfo *hwTxFmtInfo = gcvNULL;
    uint32_t logWidth, logHeight, logDepth;
    uint32_t addressing;
    uint32_t txConfig2;
    VkBool32 astcImage;
    uint32_t swizzle_r, swizzle_g, swizzle_b, swizzle_a;
    __vkFormatInfo *residentFormatInfo = VK_NULL_HANDLE;
    __vkDeviceMemory *resourceMemory = VK_NULL_HANDLE;
    VkDeviceSize offsetInResourceMemory = 0;
    gceTILING tiling = gcvLINEAR;
    VkComponentMapping *componentMapping = VK_NULL_HANDLE;
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    VkBool32 msaaImage = VK_FALSE;
    VkFormat createFormat = VK_FORMAT_UNDEFINED;
    gctUINT32 hAlignment = 0x1;
    __vkImage *img = VK_NULL_HANDLE;
    uint32_t hwDescriptorSize = TX_HW_DESCRIPTOR_MEM_SIZE;
    uint32_t partIdx, partCount = 1;

    __VK_ASSERT(hwTxDesc);

    if (imgv)
    {
        img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
        subResourceRange = &imgv->createInfo.subresourceRange;
        baseLevel = &img->pImgLevels[subResourceRange->baseMipLevel];
        residentFormatInfo = (__vkFormatInfo *)imgv->formatInfo;
        __VK_ASSERT(!bufv);

        resourceMemory = img->memory;
        offsetInResourceMemory = img->memOffset;
        tiling = img->halTiling;
        componentMapping = &imgv->createInfo.components;
        viewType = imgv->createInfo.viewType;
        aspectFlag= imgv->createInfo.subresourceRange.aspectMask;
        msaaImage = (img->sampleInfo.product > 1);
        createFormat = imgv->createInfo.format;
        hAlignment = img->hAlignment;
        hwDescriptorSize *= VK_BORDER_COLOR_RANGE_SIZE;
    }
    else if (bufv)
    {
#define __VK_FAKED_TEX_MAX_WIDTH 8192

        static __vkImageLevel fakedImageLevel = {0};
        static VkImageSubresourceRange fakedSubResourceRange;
        static const VkComponentMapping identityComponentMapping =
        {
            VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
        };
        __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bufv->createInfo.buffer);
        uint32_t sizeInByte, texelSize;

        __VK_ASSERT(!imgv);
        residentFormatInfo = &bufv->formatInfo;
        sizeInByte = (bufv->createInfo.range == VK_WHOLE_SIZE)
                   ? (uint32_t)(buf->createInfo.size - bufv->createInfo.offset)
                   : (uint32_t)bufv->createInfo.range;

        texelSize = sizeInByte / (residentFormatInfo->bitsPerBlock >> 3);
        tiling = gcvLINEAR;
        if (((texelSize / __VK_FAKED_TEX_MAX_WIDTH) < 1))
        {
            fakedImageLevel.requestW = fakedImageLevel.allocedW = texelSize;
            fakedImageLevel.requestH = fakedImageLevel.allocedH = 1;
        }
        else
        {
            fakedImageLevel.requestW = fakedImageLevel.allocedW = __VK_FAKED_TEX_MAX_WIDTH;
            fakedImageLevel.requestH = fakedImageLevel.allocedH = (uint32_t)gcoMATH_Ceiling(((float)texelSize / __VK_FAKED_TEX_MAX_WIDTH));
        }
        fakedImageLevel.stride = (uint32_t)(fakedImageLevel.allocedW * (residentFormatInfo->bitsPerBlock >> 3));
        fakedImageLevel.requestD = 1;
        fakedImageLevel.sliceSize = (VkDeviceSize)sizeInByte;
        resourceMemory = buf->memory;
        offsetInResourceMemory = buf->memOffset;
        fakedImageLevel.offset = bufv->createInfo.offset;
        baseLevel = &fakedImageLevel;
        componentMapping = (VkComponentMapping *)&identityComponentMapping;
        createFormat = bufv->createInfo.format;
        fakedSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        fakedSubResourceRange.baseArrayLayer = 0;
        fakedSubResourceRange.baseMipLevel = 0;
        fakedSubResourceRange.layerCount = 1;
        fakedSubResourceRange.levelCount = 1;
        subResourceRange = &fakedSubResourceRange;
    }
    else
    {
        __VK_ASSERT(!"Must have one resource view to generate tx descriptor");
    }

    hwTxFmtInfo = halti5_helper_convertHwTxInfo(devCtx, residentFormatInfo->residentImgFormat);

    if (!hwTxFmtInfo)
    {
        __VK_ONERROR(VK_ERROR_FORMAT_NOT_SUPPORTED);
    }

    __VK_ASSERT((baseLevel->allocedW * baseLevel->allocedH) >= 1);

    swizzle_r = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->r, hwTxFmtInfo->hwSwizzles[0], hwTxFmtInfo->hwSwizzles);
    swizzle_g = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->g, hwTxFmtInfo->hwSwizzles[1], hwTxFmtInfo->hwSwizzles);
    swizzle_b = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->b, hwTxFmtInfo->hwSwizzles[2], hwTxFmtInfo->hwSwizzles);
    swizzle_a = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->a, hwTxFmtInfo->hwSwizzles[3], hwTxFmtInfo->hwSwizzles);

    logWidth  = __vk_UtilLog2inXdot8(baseLevel->allocedW);
    logHeight = __vk_UtilLog2inXdot8(baseLevel->allocedH);
    logDepth  = __vk_UtilLog2inXdot8(baseLevel->requestD);

    addressing = (tiling == gcvLINEAR) ? 0x3 : 0x0;
    astcImage = ((((((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT))) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) ) == 0x14)
              ? VK_TRUE : VK_FALSE;

    switch (createFormat)
    {
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
        break;
    default:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    }
    partCount = residentFormatInfo->partCount;

    for (partIdx = 0; partIdx < partCount; partIdx++)
    {
        hwTxDesc[partIdx].sRGB = (hwTxFmtInfo->hwFormat >> TX_FORMAT_SRGB_SHIFT) & 0x1;
        hwTxDesc[partIdx].fast_filter = ((hwTxFmtInfo->hwFormat >> TX_FORMAT_FAST_FILTER_SHIFT) & 0x1)
                                      && (viewType != VK_IMAGE_VIEW_TYPE_3D) && (viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY);
        hwTxDesc[partIdx].sampleStencil = (aspectFlag & VK_IMAGE_ASPECT_STENCIL_BIT) ? VK_TRUE : VK_FALSE;
        hwTxDesc[partIdx].msaaImage =  msaaImage;
        hwTxDesc[partIdx].isCubmap = (viewType == VK_IMAGE_VIEW_TYPE_CUBE) || (viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);

        hwTxDesc[partIdx].baseWidth  = baseLevel->requestW;
        hwTxDesc[partIdx].baseHeight = baseLevel->requestH;
        hwTxDesc[partIdx].baseDepth  = baseLevel->requestD;
        hwTxDesc[partIdx].baseSlice  = (uint32_t) (baseLevel->sliceSize) / (residentFormatInfo->bitsPerBlock >> 3);

        hwTxDesc[partIdx].halti2.hwSamplerMode_p1 =
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (s_xlateType[viewType].hwType) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:13) - (0 ?
 17:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:13) - (0 ?
 17:13) + 1))))))) << (0 ?
 17:13))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_OLD_SHIFT)) & ((gctUINT32) ((((1 ?
 17:13) - (0 ?
 17:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:13) - (0 ? 17:13) + 1))))))) << (0 ? 17:13)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (addressing) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)));

        hwTxDesc[partIdx].halti2.hwSamplerModeEx =
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT)) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (swizzle_r) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (swizzle_g) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (swizzle_b) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (swizzle_a) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (s_xlateType[viewType].hwIsArray) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:26) - (0 ?
 28:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:26) - (0 ?
 28:26) + 1))))))) << (0 ?
 28:26))) | (((gctUINT32) ((gctUINT32) (hAlignment) & ((gctUINT32) ((((1 ?
 28:26) - (0 ?
 28:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:26) - (0 ? 28:26) + 1))))))) << (0 ? 28:26)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));

        hwTxDesc[partIdx].halti2.hwBaseLOD_p1 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (subResourceRange->levelCount - 1) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));

        hwTxDesc[partIdx].halti2.hwSampleWH =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ?
 14:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:0) - (0 ?
 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->allocedW) & ((gctUINT32) ((((1 ?
 14:0) - (0 ?
 14:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:16) - (0 ?
 30:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:16) - (0 ?
 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (baseLevel->allocedH) & ((gctUINT32) ((((1 ?
 30:16) - (0 ?
 30:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

        hwTxDesc[partIdx].halti3.hwTxSizeExt =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (logWidth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (logHeight) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

        hwTxDesc[partIdx].halti2.hwSampleLogWH_p1 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) ((astcImage ?
 2 : 0)) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (hwTxDesc[partIdx].sRGB) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

        hwTxDesc[partIdx].halti2.hwSliceSize = (uint32_t)baseLevel->sliceSize;

        hwTxDesc[partIdx].halti2.hwTxConfig3 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (hwTxDesc[partIdx].sampleStencil) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        hwTxDesc[partIdx].halti2.hwSamplerLinearStride =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:0) - (0 ?
 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0)));

        hwTxDesc[partIdx].halti2.hwTxConfig2 = txConfig2;

        if (viewType >= VK_IMAGE_VIEW_TYPE_1D_ARRAY)
        {
            hwTxDesc[partIdx].halti3.hwSamplerVolumeExt =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (logDepth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
            hwTxDesc[partIdx].halti2.hwSampler3D_p1 =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (subResourceRange->layerCount) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)));
        }
        else
        {
            hwTxDesc[partIdx].halti3.hwSamplerVolumeExt =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (logDepth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));
            hwTxDesc[partIdx].halti2.hwSampler3D_p1 =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->requestD) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)));
        }


        if (imgv)
        {
            uint32_t levelIdx;
            __VK_ASSERT(img);
            for (levelIdx = 0; levelIdx < subResourceRange->levelCount; ++levelIdx)
            {
                __vkImageLevel *level = &img->pImgLevels[subResourceRange->baseMipLevel + levelIdx];
                uint32_t physical = 0;

                physical = img->memory->devAddr;
                physical += (uint32_t)(img->memOffset
                            + partIdx * level->partSize
                            + level->offset
                            + subResourceRange->baseArrayLayer * level->sliceSize);
                __VK_ASSERT(levelIdx < 14);
                hwTxDesc[partIdx].halti2.hwSamplerLodAddresses[levelIdx] = physical;
            }
        }
        else
        {
            uint32_t physical = 0;

            __VK_ASSERT(bufv);
            __VK_ASSERT(partIdx == 0);

            physical = resourceMemory->devAddr;
            physical += (uint32_t)(offsetInResourceMemory + baseLevel->offset);
            hwTxDesc[partIdx].halti2.hwSamplerLodAddresses[0] = physical;
        }

        if (astcImage)
        {
            uint32_t astcSize = (imgv->createInfo.format - VK_FORMAT_ASTC_4x4_UNORM_BLOCK) / 2;
            uint32_t astcSrgb = hwTxDesc[partIdx].sRGB;

            hwTxDesc[partIdx].halti2.hwTxASTCEx =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)));
        }
    }

OnError:
    return result;
}

void halti3_helper_convertHwSampler(
    __vkDevContext *devCtx,
    const VkSamplerCreateInfo *createInfo,
    HwSamplerDesc *hwSamplerDesc
    )
{
    static const int32_t s_addressXlate[] =
    {
        /* VK_SAMPLER_ADDRESS_MODE_REPEAT */
        0x0,
        /* VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT */
        0x1,
        /* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE */
        0x2,
        /* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER */
        0x3,
        /* VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE */
        -1,
    };

    static const uint32_t s_magXlate[] =
    {
        /* VK_FILTER_NEAREST */
        0x1,
        /* VK_FILTER_LINEAR */
        0x2,
    };

    static const uint32_t s_minXlate[] =
    {
        /* VK_FILTER_NEAREST */
        0x1,
        /* VK_FILTER_LINEAR */
        0x2,
    };

    static const uint32_t s_mipXlate[] =
    {
        /* VK_SAMPLER_MIPMAP_MODE_NEAREST */
        0x1,
        /* VK_SAMPLER_MIPMAP_MODE_LINEAR */
        0x2,
        /* __VK_SAMPLER_MIPMAP_MODE_NONE */
        0x0,
    };

    static const uint32_t s_funcXlate[] =
    {
        0x7, /* VK_COMPARE_OP_NEVER */
        0x2, /* VK_COMPARE_OP_LESS */
        0x4, /* VK_COMPARE_OP_EQUAL */
        0x0, /* VK_COMPARE_OP_LESS_OR_EQUAL */
        0x3, /* VK_COMPARE_OP_GREATER */
        0x5, /* VK_COMPARE_OP_NOT_EQUAL */
        0x1, /* VK_COMPARE_OP_GREATER_OR_EQUAL */
        0x6, /* VK_COMPARE_OP_ALWAYS */
    };
    int32_t  lodbias, minlod, maxlod, anisoLog = 0;

    /* Convert to 5.8 format  */
    lodbias = _Float2SignedFixed(createInfo->mipLodBias, 8, 8);
    maxlod  = _Float2SignedFixed(createInfo->maxLod, 5, 8);
    minlod  = _Float2SignedFixed(__VK_MAX(0.0f, createInfo->minLod), 5, 8);

    if (createInfo->anisotropyEnable)
    {
        __VK_ASSERT(createInfo->maxAnisotropy <= devCtx->pPhyDevice->phyDevProp.limits.maxSamplerAnisotropy);
        anisoLog = __vk_UtilLog2inXdot8(gcoMATH_Float2UInt(createInfo->maxAnisotropy));
    }

    hwSamplerDesc->halti3.anisoLog = anisoLog;

    hwSamplerDesc->halti3.hwSamplerMode_p0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:3) - (0 ?
 4:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:3) - (0 ?
 4:3) + 1))))))) << (0 ?
 4:3))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeU]) & ((gctUINT32) ((((1 ?
 4:3) - (0 ?
 4:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ? 4:3)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ?
 6:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:5) - (0 ?
 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeV]) & ((gctUINT32) ((((1 ?
 6:5) - (0 ?
 6:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:7) - (0 ?
 8:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:7) - (0 ?
 8:7) + 1))))))) << (0 ?
 8:7))) | (((gctUINT32) ((gctUINT32) (s_minXlate[createInfo->minFilter]) & ((gctUINT32) ((((1 ?
 8:7) - (0 ?
 8:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:7) - (0 ? 8:7) + 1))))))) << (0 ? 8:7)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:11) - (0 ?
 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) ((gctUINT32) (s_magXlate[createInfo->magFilter]) & ((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ? 12:11)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:9) - (0 ?
 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) ((gctUINT32) (s_mipXlate[createInfo->mipmapMode]) & ((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ? 10:9)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)));

    hwSamplerDesc->halti3.hwSamplerLOD  =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:0) - (0 ?
 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (maxlod) & ((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ? 12:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:16) - (0 ?
 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (minlod) & ((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ? 28:16)));

    hwSamplerDesc->halti3.hwSamplerLODBias  =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) ((lodbias == 0 ?
 0 : 1)) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (lodbias) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    hwSamplerDesc->halti3.hwBaseLOD_p0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (createInfo->compareEnable ?
 1 : 0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (s_funcXlate[createInfo->compareOp]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:23) - (0 ?
 24:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:23) - (0 ?
 24:23) + 1))))))) << (0 ?
 24:23))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 24:23) - (0 ?
 24:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ? 24:23)));

    hwSamplerDesc->halti3.hwSampler3D_p0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeW]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));

    return;
}

void halti3_helper_setSamplerStates(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImageView *imgv,
    uint32_t txHwRegisterIdx,
    HwTxDesc *txDesc,
    HwSamplerDesc *samplerDesc,
    uint32_t borderColorIdx,
    uint32_t hwSamplerNo,
    uint32_t shaderConfigData
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    if (!txHwRegisterIdx)
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x022D, VK_FALSE,
            ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));
    }
    else
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x022D, VK_FALSE,
            ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));
    }

    __vkCmdLoadSingleHWState(commandBuffer, 0x4000 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSamplerMode_p1 | samplerDesc->halti3.hwSamplerMode_p0);

    __vkCmdLoadSingleHWState(commandBuffer, 0x40E0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSamplerModeEx);

    __vkCmdLoadSingleHWState(commandBuffer, 0x41E0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxConfig2);

    __vkCmdLoadSingleHWState(commandBuffer, 0x44A0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxConfig3);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4480 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSliceSize);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4020 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSampleWH);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4040 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSampleLogWH_p1
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (txDesc->fast_filter && ((samplerDesc->halti3.anisoLog == 0) || txDesc->isCubmap) && !devCtx->database->TX_NO_FIXED_FILTER) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29))));

    __vkCmdLoadSingleHWState(commandBuffer, 0x4400 + hwSamplerNo, VK_FALSE,
        txDesc->halti3.hwTxSizeExt);

    __vkCmdLoadSingleHWState(commandBuffer, 0x44C0 + hwSamplerNo, VK_FALSE,
        samplerDesc->halti3.anisoLog);

    __vkCmdLoadSingleHWState(commandBuffer, 0x40C0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSampler3D_p1 | samplerDesc->halti3.hwSampler3D_p0);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4420 + hwSamplerNo, VK_FALSE,
        txDesc->halti3.hwSamplerVolumeExt);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4440 + hwSamplerNo, VK_FALSE,
        samplerDesc->halti3.hwSamplerLOD);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4460 + hwSamplerNo, VK_FALSE,
        samplerDesc->halti3.hwSamplerLODBias);

    __vkCmdLoadSingleHWState(commandBuffer, 0x41C0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwBaseLOD_p1 | samplerDesc->halti3.hwBaseLOD_p0);

    __vkCmdLoadBatchHWStates(commandBuffer, 0x4200 + (hwSamplerNo << 4), VK_FALSE,
        14, txDesc->halti2.hwSamplerLodAddresses);

    __vkCmdLoadSingleHWState(commandBuffer, 0x40A0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSamplerLinearStride);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4140 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxASTCEx);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4160 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxASTCEx);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4180 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxASTCEx);

    __vkCmdLoadSingleHWState(commandBuffer, 0x41A0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwTxASTCEx);

    return;
}

VkResult halti3_program_copy_src_img(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    IN  VkFilter filter,
    OUT __vkComputeBlitParams *params
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkImageView  *imgView = gcvNULL;
    __vkBufferView *bufView = gcvNULL;
    SHADER_UAV_SLOT_MAPPING *hwMapping = gcvNULL;
    HwImgDesc hwImgDesc[__VK_MAX_PARTS];
    uint32_t hwConstRegAddr;
    VkExtent3D *pBufSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    uint32_t imageDesc = 0;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (srcRes->isImage)
    {
        static __vkImageView tmpImgView;
        static VkExtent3D srcSize;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        const __vkFormatInfo *fmtInfo = gcvNULL;

        params->srcOffset = srcRes->u.img.offset;
        params->srcExtent = srcRes->u.img.extent;

        __VK_MEMZERO(&tmpImgView, sizeof(tmpImgView));
        tmpImgView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpImgView.obj.pDevContext = devCtx;
        tmpImgView.devCtx = devCtx;
        tmpImgView.createInfo.image = (VkImage)(uintptr_t)pSrcImg;
        tmpImgView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpImgView.createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        tmpImgView.createInfo.format = pSrcImg->createInfo.format;
        tmpImgView.createInfo.subresourceRange.baseMipLevel = srcRes->u.img.subRes.mipLevel;
        tmpImgView.createInfo.subresourceRange.levelCount = 1;
        tmpImgView.createInfo.subresourceRange.baseArrayLayer = srcRes->u.img.subRes.arrayLayer;
        tmpImgView.createInfo.subresourceRange.layerCount = 1;

        __VK_MEMCOPY(&tmpFormatInfo, &pSrcImg->formatInfo, sizeof(tmpFormatInfo));

        tmpFormatInfo.residentImgFormat = params->srcFormat;

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        fmtInfo = &srcRes->u.img.pImage->formatInfo;


        if (params->rawCopy && fmtInfo->compressed)
        {
            VkExtent2D rect = fmtInfo->blockSize;

            params->srcOffset.x      = params->srcOffset.x / rect.width;
            params->srcOffset.y      = params->srcOffset.y / rect.height;
            params->srcExtent.width  = gcmALIGN_NP2(params->srcExtent.width, rect.width)  / rect.width;
            params->srcExtent.height = gcmALIGN_NP2(params->srcExtent.height, rect.height) / rect.height;

            srcSize.width  = gcmALIGN_NP2(srcRes->u.img.extent.width, rect.width ) / rect.width;
            srcSize.height = gcmALIGN_NP2(srcRes->u.img.extent.height, rect.height) / rect.height;
            srcSize.depth  = srcRes->u.img.extent.depth;

            if (fmtInfo->bitsPerBlock == 128)
            {
                params->srcOffset.x *= 2;
                params->srcExtent.width *= 2;
                srcSize.width *= 2;
            }
            pBufSize = &srcSize;
        }
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D srcSize;
        __vkImage *pDstImg = dstRes->u.img.pImage;
        const __vkFormatInfo *fmtInfo = gcvNULL;
        uint32_t imgFormat = VK_FORMAT_UNDEFINED;

        params->srcOffset.x = params->srcOffset.y = params->srcOffset.z = 0;

        __VK_MEMZERO(&tmpBufView, sizeof(tmpBufView));
        tmpBufView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpBufView.obj.pDevContext = devCtx;
        tmpBufView.devCtx = devCtx;
        tmpBufView.createInfo.buffer = (VkBuffer)(uintptr_t)srcRes->u.buf.pBuffer;
        tmpBufView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpBufView.createInfo.format = pDstImg->createInfo.format;
        tmpBufView.createInfo.offset = srcRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo((VkFormat) params->srcFormat);

        bufView = &tmpBufView;

        srcSize.width  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        srcSize.height = srcRes->u.buf.imgHeight != 0 ? srcRes->u.buf.imgHeight : dstRes->u.img.extent.height;
        srcSize.depth  = dstRes->u.img.extent.depth;

        pBufSize = &srcSize;

        imgFormat = (dstRes->u.img.subRes.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
                 ? params->srcFormat
                 : pDstImg->createInfo.format;

        fmtInfo = __vk_GetVkFormatInfo((VkFormat) imgFormat);

        if (params->rawCopy && fmtInfo->compressed)
        {
            VkExtent2D rect = fmtInfo->blockSize;

            srcSize.width  = gcmALIGN_NP2(dstRes->u.img.extent.width, rect.width ) / rect.width;
            srcSize.height = gcmALIGN_NP2(dstRes->u.img.extent.height, rect.height) / rect.height;

            if (fmtInfo->bitsPerBlock == 128)
            {
                srcSize.width *= 2;
            }
        }
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pBufSize, hwImgDesc));

    imageDesc = hwImgDesc[0].imageInfo[3];

    if (params->txSwizzles)
    {
        uint32_t swizzle_r, swizzle_g, swizzle_b, swizzle_a;
        swizzle_r = halti5_helper_convertHwImgSwizzle(params->txSwizzles->r);
        swizzle_g = halti5_helper_convertHwImgSwizzle(params->txSwizzles->g);
        swizzle_b = halti5_helper_convertHwImgSwizzle(params->txSwizzles->b);
        swizzle_a = halti5_helper_convertHwImgSwizzle(params->txSwizzles->a);

        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (imageDesc)) >> (0 ?
 9:6)) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1)))))) )) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (imageDesc)) >> (0 ?
 2:0)) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1)))))) )) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (imageDesc)) >> (0 ?
 15:14)) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1)))))) )) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) ((((((gctUINT32) (imageDesc)) >> (0 ?
 5:4)) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1)))))) )) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (swizzle_r) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (swizzle_g) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (swizzle_b) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (swizzle_a) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));

        hwImgDesc[0].imageInfo[3] = imageDesc;
    }

    hwMapping = &blitProg->srcImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

OnError:
    return result;
}

VkResult halti3_program_copy_dst_img(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    OUT __vkComputeBlitParams *params
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkImageView  *imgView = gcvNULL;
    __vkBufferView *bufView = gcvNULL;
    SHADER_UAV_SLOT_MAPPING *hwMapping = gcvNULL;
    HwImgDesc hwImgDesc[__VK_MAX_PARTS];
    uint32_t hwConstRegAddr;
    VkExtent3D *pBufSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (dstRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pDstImg = dstRes->u.img.pImage;

        params->dstOffset = dstRes->u.img.offset;

        __VK_MEMZERO(&tmpImgView, sizeof(tmpImgView));
        tmpImgView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpImgView.obj.pDevContext = devCtx;
        tmpImgView.devCtx = devCtx;
        tmpImgView.createInfo.image = (VkImage)(uintptr_t)pDstImg;
        tmpImgView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpImgView.createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        tmpImgView.createInfo.format = pDstImg->createInfo.format;
        tmpImgView.createInfo.subresourceRange.baseMipLevel = dstRes->u.img.subRes.mipLevel;
        tmpImgView.createInfo.subresourceRange.levelCount = 1;
        tmpImgView.createInfo.subresourceRange.baseArrayLayer = dstRes->u.img.subRes.arrayLayer;
        tmpImgView.createInfo.subresourceRange.layerCount = 1;

        __VK_MEMCOPY(&tmpFormatInfo, &pDstImg->formatInfo, sizeof(tmpFormatInfo));

        tmpFormatInfo.residentImgFormat = params->dstFormat;

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        params->dstExtent = dstRes->u.img.extent;

        if (!imgView->formatInfo->compressed && srcRes->isImage && srcRes->u.img.pImage->formatInfo.compressed)
        {
            VkExtent2D rect = srcRes->u.img.pImage->formatInfo.blockSize;

            params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width ) / rect.width;
            params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
        }
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D dstSize;
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        const __vkFormatInfo *fmtInfo = gcvNULL;

        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;

        __VK_MEMZERO(&tmpBufView, sizeof(tmpBufView));
        tmpBufView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpBufView.obj.pDevContext = devCtx;
        tmpBufView.devCtx = devCtx;
        tmpBufView.createInfo.buffer = (VkBuffer)(uintptr_t)dstRes->u.buf.pBuffer;
        tmpBufView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpBufView.createInfo.format = pSrcImg->createInfo.format;
        tmpBufView.createInfo.offset = dstRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo((VkFormat) params->dstFormat);

        bufView = &tmpBufView;

        params->dstExtent = srcRes->u.img.extent;
        fmtInfo = __vk_GetVkFormatInfo(pSrcImg->createInfo.format);

        dstSize.width  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstSize.height = dstRes->u.buf.imgHeight != 0 ? dstRes->u.buf.imgHeight : srcRes->u.img.extent.height;
        dstSize.depth  = srcRes->u.img.extent.depth;

        if (params->rawCopy && pSrcImg->formatInfo.compressed)
        {
            VkExtent2D rect = fmtInfo->blockSize;

            params->dstOffset.x      = params->dstOffset.x / rect.width;
            params->dstOffset.y      = params->dstOffset.y / rect.height;
            params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width)  / rect.width;
            params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;

            dstSize.width  = gcmALIGN_NP2(dstSize.width, rect.width ) / rect.width;
            dstSize.height = gcmALIGN_NP2(dstSize.height, rect.height) / rect.height;

            if (fmtInfo->bitsPerBlock == 128)
            {
                dstSize.width *= 2;
                params->dstExtent.width *= 2;
                params->dstOffset.x *= 2;
            }
        }

        pBufSize = &dstSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pBufSize, hwImgDesc));

    if (params->rawCopy && imgView && imgView->formatInfo->compressed)
    {
        VkBool32 srcCompressed = srcRes->isImage && srcRes->u.img.pImage->formatInfo.compressed;
        VkExtent2D rect = imgView->formatInfo->blockSize;
        __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgView->createInfo.image);
        __vkImageLevel *baseLevel = &img->pImgLevels[imgView->createInfo.subresourceRange.baseMipLevel];

        uint32_t width  = gcmALIGN_NP2(baseLevel->requestW, rect.width ) / rect.width;
        uint32_t height = gcmALIGN_NP2(baseLevel->requestH, rect.height) / rect.height;

        params->dstOffset.x = gcmALIGN_NP2(params->dstOffset.x - rect.width  + 1, rect.width ) / rect.width;
        params->dstOffset.y = gcmALIGN_NP2(params->dstOffset.y - rect.height + 1, rect.height) / rect.width;
        params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width ) / (srcCompressed ? rect.width  : 1);
        params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / (srcCompressed ? rect.width  : 1);

        if (imgView->formatInfo->bitsPerBlock == 128)
        {
            width *= 2;
            params->dstOffset.x *= 2;
            params->dstExtent.width *= 2;
        }
        hwImgDesc[0].imageInfo[2] = width | (height << 16);
    }

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

OnError:
    return result;
}

/* buffer copy will fake the format as R8 */
VkResult halti3_program_blit_buffer_src(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    IN  VkFilter filter,
    OUT __vkComputeBlitParams *params
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkImageView  *imgView = gcvNULL;
    __vkBufferView *bufView = gcvNULL;
    SHADER_UAV_SLOT_MAPPING *hwMapping = gcvNULL;
    HwImgDesc hwImgDesc[__VK_MAX_PARTS];
    uint32_t hwConstRegAddr;
    VkExtent3D *pBufSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));
    __VK_ASSERT(!srcRes->isImage);

    {
        static __vkBufferView tmpBufView;
        static VkExtent3D srcSize;

        params->srcOffset.x = params->srcOffset.y = params->srcOffset.z = 0;

        __VK_MEMZERO(&tmpBufView, sizeof(tmpBufView));
        tmpBufView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpBufView.obj.pDevContext = devCtx;
        tmpBufView.devCtx = devCtx;
        tmpBufView.createInfo.buffer = (VkBuffer)(uintptr_t)srcRes->u.buf.pBuffer;
        tmpBufView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpBufView.createInfo.format =  VK_FORMAT_R8_SINT;
        tmpBufView.createInfo.offset = srcRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R8_SINT);
        tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R8_SINT;
        bufView = &tmpBufView;

        srcSize.width  = srcRes->u.buf.rowLength ;
        srcSize.height = srcRes->u.buf.imgHeight ;
        srcSize.depth  = 1;
        params->srcExtent = srcSize;
        params->srcOffset.x = params->srcOffset.y = params->srcOffset.z = 0;
        pBufSize = &srcSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pBufSize, hwImgDesc));

    hwMapping = &blitProg->srcImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

OnError:
    return result;
}

VkResult halti3_program_blit_buffer_dst(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    OUT __vkComputeBlitParams *params
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkImageView  *imgView = gcvNULL;
    __vkBufferView *bufView = gcvNULL;
    SHADER_UAV_SLOT_MAPPING *hwMapping = gcvNULL;
    HwImgDesc hwImgDesc[__VK_MAX_PARTS];
    uint32_t hwConstRegAddr;
    VkExtent3D *pBufSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));
    __VK_ASSERT(!dstRes->isImage);

    {
        static __vkBufferView tmpBufView;
        static VkExtent3D dstSize;
        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;

        __VK_MEMZERO(&tmpBufView, sizeof(tmpBufView));
        tmpBufView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpBufView.obj.pDevContext = devCtx;
        tmpBufView.devCtx = devCtx;
        tmpBufView.createInfo.buffer = (VkBuffer)(uintptr_t)dstRes->u.buf.pBuffer;
        tmpBufView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpBufView.createInfo.format = VK_FORMAT_R8_SINT;
        tmpBufView.createInfo.offset = dstRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R8_SINT);
        tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R8_SINT;
        bufView = &tmpBufView;


        dstSize.width  = dstRes->u.buf.rowLength ;
        dstSize.height = dstRes->u.buf.imgHeight ;
        dstSize.depth  = 1;
        params->dstExtent = dstSize;
        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;

        pBufSize = &dstSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pBufSize, hwImgDesc));

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

OnError:
    return result;
}


VkResult halti5_computeClear(
    VkCommandBuffer cmdBuf,
    VkClearValue *clearValue,
    __vkBlitRes *dstRes
    );

VkResult halti3_fillBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    uint32_t data
    )
{
    /* To do */
    __vkBuffer *pDstBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkBlitRes tmpRes;
    const uint32_t maxTexSize = 8192;
    VkClearValue  clearValue;
    tmpRes.isImage = VK_FALSE;
    tmpRes.u.buf.pBuffer = pDstBuffer;
    tmpRes.u.buf.offset = offset;
    if (size == VK_WHOLE_SIZE)
    {
        size = pDstBuffer->memReq.size - offset;
    }

    size = size / sizeof(uint32_t);
    /* Split the clear size to two rect */
    clearValue.color.int32[0] = data;
    if(size < maxTexSize)
    {
        if(size > 0)
        {
        tmpRes.u.buf.rowLength = (uint32_t)size;
        tmpRes.u.buf.imgHeight = 1;
        halti5_computeClear(cmdBuf,&clearValue,&tmpRes);
        }
    }
    else
    {
        uint32_t height = (uint32_t)size / maxTexSize;
        tmpRes.u.buf.rowLength = maxTexSize;
        tmpRes.u.buf.imgHeight = height;
        halti5_computeClear(cmdBuf,&clearValue,&tmpRes);
        if(size % maxTexSize)
        {
             tmpRes.u.buf.rowLength = size % maxTexSize;
             tmpRes.u.buf.imgHeight = 1;
             tmpRes.u.buf.offset += maxTexSize * height * sizeof(uint32_t);
             halti5_computeClear(cmdBuf,&clearValue,&tmpRes);
        }
    }

    if (pCmdBuf->bindInfo.pipeline.graphics)
    {
        pCmdBuf->bindInfo.pipeline.dirty |= __VK_CMDBUF_BINDNIGINFO_PIPELINE_GRAPHICS_DIRTY;
    }

    if(pCmdBuf->bindInfo.pipeline.compute)
    {
        pCmdBuf->bindInfo.pipeline.dirty |= __VK_CMDBUF_BINDINGINFO_PIPELINE_COMPUTE_DIRTY;
    }

    return VK_SUCCESS;
}


VkResult halti3_copyBuffer(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    uint64_t copySize
    )
{
    const uint32_t maxTexSize = 8192;

    if (copySize < maxTexSize)
    {
         /* use rowLength and imgHeight to fake a 2d image*/
        if (copySize > 0)
        {
            srcRes->u.buf.rowLength =(uint32_t)  copySize;
            srcRes->u.buf.imgHeight = 1;

            dstRes->u.buf.rowLength = (uint32_t) copySize;
            dstRes->u.buf.imgHeight = 1;

            halti5_computeBlit(cmdBuf, srcRes, dstRes, VK_FALSE, gcvNULL,VK_FILTER_NEAREST);
        }
    }
    else
    {
        uint32_t height =(uint32_t)  copySize / maxTexSize;

        srcRes->u.buf.rowLength = maxTexSize;
        srcRes->u.buf.imgHeight = height;
        dstRes->u.buf.rowLength = maxTexSize;
        dstRes->u.buf.imgHeight = height;

        halti5_computeBlit(cmdBuf, srcRes, dstRes, VK_FALSE, gcvNULL, VK_FILTER_NEAREST);

        if (copySize % maxTexSize)
        {
            VkDeviceSize addOffset = maxTexSize * height;
            srcRes->u.buf.rowLength = dstRes->u.buf.rowLength = copySize % maxTexSize;
            srcRes->u.buf.imgHeight = dstRes->u.buf.imgHeight = 1;
            srcRes->u.buf.offset += addOffset;
            dstRes->u.buf.offset += addOffset;

            halti5_computeBlit(cmdBuf, srcRes, dstRes, VK_FALSE, gcvNULL, VK_FILTER_NEAREST);

            /*restore to orignal offset*/
            srcRes->u.buf.offset -= addOffset;
            dstRes->u.buf.offset -= addOffset;
        }
    }
    return VK_SUCCESS;
}

VkResult halti3_updateBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    const uint32_t* pData
    )
{
   __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkBuffer *dstbuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, buffer);
    __vkScratchMem *pScratch = gcvNULL;
    void *pSrcHost = gcvNULL;
    __vkBlitRes srcRes, dstRes;
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    __vkBuffer srcFakeBuf;
    /* Allocate scratch gpu memory and copy host data to it first */
    pScratch = __vkGetScratchMem(pCmdBuf, size);
    __VK_ONERROR(__vk_MapMemory((VkDevice)pCmdBuf->devCtx, (VkDeviceMemory)(uintptr_t)pScratch->memory,
                                0, size, 0, (void**)&pSrcHost));
    gcoOS_MemCopy(pSrcHost, pData, (gctSIZE_T)size);

    srcFakeBuf.devCtx = devCtx;
    srcFakeBuf.memory = pScratch->memory;
    srcFakeBuf.memOffset = 0;
    srcRes.isImage = dstRes.isImage = VK_FALSE;
    srcRes.u.buf.pBuffer = &srcFakeBuf;
    srcRes.u.buf.offset = 0;
    dstRes.u.buf.offset = offset;
    dstRes.u.buf.pBuffer = dstbuf;

    result = halti3_copyBuffer(cmdBuf,&srcRes,&dstRes,size);
OnError:
    if (pSrcHost)
    {
        __vk_UnmapMemory((VkDevice)pCmdBuf->devCtx, (VkDeviceMemory)(uintptr_t)pScratch->memory);
    }
    pCmdBuf->curScrachBufIndex = 0;
    return result;
}




