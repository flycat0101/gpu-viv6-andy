/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vk_precomp.h"

#include "gc_halti5_blit.comp.h"

#define VIV_LOCAL_X 16
#define VIV_LOCAL_Y 16
#define VIV_LOCAL_Z  1

VkResult halti2_program_blit_src_tex(
    IN  __vkCommandBuffer *pCmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    IN  VkFilter filter,
    OUT __vkComputeBlitParams *params
    );

VkResult halti3_program_copy_src_img(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    IN  VkFilter filter,
    OUT __vkComputeBlitParams *params
    );

VkResult halti3_program_copy_dst_img(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    OUT __vkComputeBlitParams *params
    );

VkResult halti3_program_blit_buffer_src(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    IN  VkFilter filter,
    OUT __vkComputeBlitParams *params
    );

VkResult halti3_program_blit_buffer_dst(
    IN  __vkCommandBuffer *cmdBuf,
    IN  halti5_vscprogram_blit *blitProg,
    IN  uint32_t **states,
    IN  __vkBlitRes *srcRes,
    IN  __vkBlitRes *dstRes,
    OUT __vkComputeBlitParams *params
    );

static PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY* halti5_getCombinedTexSamplerEntry(
    PROG_VK_RESOURCE_SET *resSet,
    uint32_t binding
)
{
    gctUINT entryIdx;
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY *texEntry = gcvNULL;

    for (entryIdx = 0; entryIdx < resSet->combinedSampTexTable.countOfEntries; ++entryIdx)
    {
        PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY *pEntry = &resSet->combinedSampTexTable.pCombTsEntries[entryIdx];

        if (binding == pEntry->combTsBinding.binding)
        {
            texEntry = pEntry;
            break;
        }
    }

    __VK_ASSERT(texEntry);
    return texEntry;
}

static PROG_VK_STORAGE_TABLE_ENTRY* halti5_getImageEntry(
    PROG_VK_RESOURCE_SET *resSet,
    uint32_t binding
)
{
    gctUINT entryIdx;
    PROG_VK_STORAGE_TABLE_ENTRY *imgEntry = gcvNULL;

    for (entryIdx = 0; entryIdx < resSet->storageTable.countOfEntries; ++entryIdx)
    {
        PROG_VK_STORAGE_TABLE_ENTRY *pEntry = &resSet->storageTable.pStorageEntries[entryIdx];

        if (binding == pEntry->storageBinding.binding)
        {
            imgEntry = pEntry;
            break;
        }
    }

    __VK_ASSERT(imgEntry);
    return imgEntry;
}

static PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY* halti5_getUboEntry(
    PROG_VK_RESOURCE_SET *resSet,
    uint32_t binding
)
{
    gctUINT entryIdx;
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY *uboEntry = gcvNULL;

    for (entryIdx = 0; entryIdx < resSet->uniformBufferTable.countOfEntries; ++entryIdx)
    {
        PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY *pEntry = &resSet->uniformBufferTable.pUniformBufferEntries[entryIdx];

        if (binding == pEntry->ubBinding.binding)
        {
            uboEntry = pEntry;
            break;
        }
    }

    __VK_ASSERT(uboEntry);
    return uboEntry;
}


static VkResult halti5_program_blit_src_tex(
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
    uint32_t address;
    __vkScratchMem *pScratchMem = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    /*
    ** Prepare and program textures
    */
    static const gctINT32 magXlate[] =
    {
        0x1,
        0x2,
    };
    static const gctINT32 minXlate[] =
    {
        0x1,
        0x2,
    };
    gcsTEXTUREDESCRIPTORREGS *pDesc = gcvNULL;
    uint32_t txStride, txSliceSize, txHAlign;
    uint32_t txType, txSRGB, txSignExt, txAddressing, txIntCtrl;
    uint32_t txFormat = VK_FORMAT_UNDEFINED;
    const __vkFormatToHwTxFmtInfo *hwTxFmtInfo = gcvNULL;
    SHADER_SAMPLER_SLOT_MAPPING *hwMapping = gcvNULL;
    uint32_t hwSamplerNo;
    uint32_t tmpFormat;

    if (srcRes->isImage)
    {
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        __vkImageLevel *pSrcLevel = &pSrcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];

        params->srcOffset = srcRes->u.img.offset;
        params->srcExtent = srcRes->u.img.extent;
        params->srcSize.width  = pSrcLevel->requestW;
        params->srcSize.height = pSrcLevel->requestH;
        params->srcSize.depth  = pSrcLevel->requestD;

        txFormat = pSrcImg->formatInfo.residentImgFormat;
        txStride = (uint32_t)pSrcLevel->stride;
        txSliceSize = (uint32_t)pSrcLevel->sliceSize;

        txType = (pSrcImg->createInfo.imageType == VK_IMAGE_TYPE_3D) ? 0x3 : 0x2;
        txAddressing = (pSrcImg->halTiling == gcvLINEAR) ? 0x3 : 0x0;
        txHAlign = pSrcImg->hAlignment;

        address = pSrcImg->memory->devAddr;
        address += (uint32_t)(pSrcImg->memOffset + pSrcLevel->offset +
                              srcRes->u.img.subRes.arrayLayer * pSrcLevel->sliceSize);
    }
    else
    {
        const __vkFormatInfo *fmtInfo;
        __vkBuffer *pSrcBuf = srcRes->u.buf.pBuffer;
        __vkImage  *pDstImg = dstRes->u.img.pImage;

        params->srcOffset.x = params->srcOffset.y = params->srcOffset.z = 0;
        params->srcExtent = dstRes->u.img.extent;
        params->srcSize.width  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        params->srcSize.height = srcRes->u.buf.imgHeight != 0 ? srcRes->u.buf.imgHeight : dstRes->u.img.extent.height;
        params->srcSize.depth  = dstRes->u.img.extent.depth;

        if (dstRes->u.img.subRes.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            txFormat = VK_FORMAT_R8_UNORM;
        }
        else
        {
            txFormat = pDstImg->createInfo.format;
        }
        fmtInfo = &g_vkFormatInfoTable[txFormat];
        txStride = (params->srcSize.width / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        txSliceSize = (params->srcSize.height / fmtInfo->blockSize.height) * txStride;

        txType = (pDstImg->createInfo.imageType == VK_IMAGE_TYPE_3D) ? 0x3 : 0x2;
        txAddressing = 0x3;
        txHAlign = 0x1;

        address = pSrcBuf->memory->devAddr;
        address += (uint32_t)(pSrcBuf->memOffset + srcRes->u.buf.offset);
    }

    if (dstRes->isImage)
    {
        __vkImage *pDstImg = dstRes->u.img.pImage;
        params->dstSRGB = pDstImg->formatInfo.category == __VK_FMT_CATEGORY_SRGB;
    }
    else
    {
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        params->dstSRGB = g_vkFormatInfoTable[pSrcImg->createInfo.format].category == __VK_FMT_CATEGORY_SRGB;
    }

    switch (txFormat)
    {
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
        txSignExt = 0x2;
        break;
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
        txSignExt = 0x1;
        break;
    default:
        txSignExt = 0x0;
        break;
    }

    tmpFormat = txFormat;

    switch (txFormat)
    {
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
        tmpFormat = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        break;
    }

    hwTxFmtInfo = halti5_helper_convertHwTxInfo(devCtx, tmpFormat);
    txSRGB = (hwTxFmtInfo->hwFormat >> TX_FORMAT_SRGB_SHIFT) & 0x1;
    params->srcSRGB = txSRGB ? VK_TRUE : VK_FALSE;
    /* Disable src texture SRGB if dst also require SRGB */
    if (params->dstSRGB && txSRGB)
    {
        txSRGB = 0;
    }

    txIntCtrl = ((hwTxFmtInfo->hwFormat >> TX_FORMAT_FAST_FILTER_SHIFT) & 0x1)
             && (txType != 0x3)
             && !txSRGB;

    pScratchMem = __vkGetScratchMem(cmdBuf, TX_HW_DESCRIPTOR_MEM_SIZE);
    __VK_ONERROR(__vk_MapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem->memory,
                                0, TX_HW_DESCRIPTOR_MEM_SIZE, 0, (void**)&pDesc));
    __VK_MEMZERO(pDesc, TX_HW_DESCRIPTOR_MEM_SIZE);
    pDesc->gcregTXAddress[0] = address;

    pDesc->gcregTXConfig =
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (txType) & ((gctUINT32) ((((1 ?
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
 21:20))) | (((gctUINT32) ((gctUINT32) (txAddressing) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)));

    pDesc->gcregTXSize =
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:0) - (0 ?
 14:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:0) - (0 ?
 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (params->srcSize.width) & ((gctUINT32) ((((1 ?
 14:0) - (0 ?
 14:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ? 14:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:16) - (0 ?
 30:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:16) - (0 ?
 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (params->srcSize.height) & ((gctUINT32) ((((1 ?
 30:16) - (0 ?
 30:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ? 30:16)));

    pDesc->gcregTX3D = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (params->srcSize.depth) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)));

    pDesc->gcregTXLinearStride =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:0) - (0 ?
 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (txStride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0)));

    pDesc->gcregTXExtConfig =
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
 10:8))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[0]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[1]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[2]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[3]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:26) - (0 ?
 28:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:26) - (0 ?
 28:26) + 1))))))) << (0 ?
 28:26))) | (((gctUINT32) ((gctUINT32) (txHAlign) & ((gctUINT32) ((((1 ?
 28:26) - (0 ?
 28:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:26) - (0 ? 28:26) + 1))))))) << (0 ? 28:26)))
        ;

    pDesc->gcregTXConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (txSignExt) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)));

    pDesc->gcregTXSizeExt =
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (__vk_UtilLog2inXdot8(params->srcSize.width)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (__vk_UtilLog2inXdot8(params->srcSize.height)) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)));

    pDesc->gcregTXVolumeExt =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (__vk_UtilLog2inXdot8(params->srcSize.depth)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

    pDesc->gcregTXSlice = txSliceSize;

    hwMapping = &blitProg->srcTexEntry->hwMappings[VSC_SHADER_STAGE_CS].samplerMapping;
    hwSamplerNo = hwMapping->hwSamplerSlot + pHints->samplerBaseOffset[VSC_SHADER_STAGE_CS];

    address = pScratchMem->memory->devAddr;
    __vkCmdLoadSingleHWState(states, 0x5700 + hwSamplerNo, VK_FALSE, address);

    __vkCmdLoadSingleHWState(states, 0x5B00 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:6) - (0 ? 8:6) + 1))))))) << (0 ? 8:6)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:9) - (0 ?
 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) ((gctUINT32) (minXlate[filter]) & ((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ? 10:9)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:13) - (0 ?
 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) ((gctUINT32) (magXlate[filter]) & ((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ? 14:13)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:11) - (0 ?
 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ? 12:11)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (txIntCtrl) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))
        );

    __vkCmdLoadSingleHWState(states, 0x5B80 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (txSRGB) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
        );

    __vkCmdLoadSingleHWState(states, 0x5C00 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:0) - (0 ?
 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (0xFFF) & ((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ? 12:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:16) - (0 ?
 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ? 28:16)))
        );

    __vkCmdLoadSingleHWState(states, 0x5C80 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
        );

    __vkCmdLoadSingleHWState(states, 0x5D00 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:0) - (0 ?
 10:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:0) - (0 ?
 10:0) + 1))))))) << (0 ?
 10:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 10:0) - (0 ?
 10:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:0) - (0 ? 10:0) + 1))))))) << (0 ? 10:0)))
        );

    __vkCmdLoadSingleHWState(states, 0x5780 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
        );

    __vkCmdLoadSingleHWState(states, 0x5312, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:28) - (0 ?
 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ? 31:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (hwSamplerNo) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
        );

OnError:
    if (pDesc)
    {
        __vk_UnmapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem->memory);
    }

    return result;
}

static VkResult halti5_program_blit_dst_img(
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
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (dstRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pDstImg = dstRes->u.img.pImage;

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
        tmpImgView.createInfo.subresourceRange.aspectMask = dstRes->u.img.subRes.aspectMask;

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        __VK_MEMCOPY(&tmpFormatInfo, &pDstImg->formatInfo, sizeof(tmpFormatInfo));

        switch (tmpFormatInfo.residentImgFormat)
        {
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case __VK_FORMAT_A4R4G4B4_UNFORM_PACK16:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R16_UINT;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
            break;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R32_SFLOAT;
            break;

        case VK_FORMAT_R8G8B8A8_SRGB:
            /* Disable dst SRGB if src also require SRGB */
            if (params->srcSRGB)
            {
                tmpFormatInfo.residentImgFormat = VK_FORMAT_R8G8B8A8_UNORM;
            }
            break;
        case VK_FORMAT_B8G8R8A8_SRGB:
            /* Disable dst SRGB if src also require SRGB */
            if (params->srcSRGB)
            {
                tmpFormatInfo.residentImgFormat = VK_FORMAT_B8G8R8A8_UNORM;
            }
            break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            /* Disable dst SRGB if src also require SRGB */
            if (params->srcSRGB)
            {
                tmpFormatInfo.residentImgFormat = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
            }
            break;

        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R32G32_SFLOAT;
            break;
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
            break;
        default:
            break;
        }

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        params->dstOffset = dstRes->u.img.offset;
        params->dstExtent = dstRes->u.img.extent;
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D dstSize;
        __vkImage *pSrcImg = srcRes->u.img.pImage;

        __VK_MEMZERO(&tmpBufView, sizeof(tmpBufView));
        tmpBufView.obj.sType = __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW);
        tmpBufView.obj.pDevContext = devCtx;
        tmpBufView.devCtx = devCtx;
        tmpBufView.createInfo.buffer = (VkBuffer)(uintptr_t)dstRes->u.buf.pBuffer;
        tmpBufView.createInfo.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        tmpBufView.createInfo.format = pSrcImg->createInfo.format;
        tmpBufView.createInfo.offset = dstRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;
        tmpBufView.formatInfo = g_vkFormatInfoTable[pSrcImg->createInfo.format];
        tmpBufView.formatInfo.residentImgFormat = tmpBufView.createInfo.format;

        switch (tmpBufView.formatInfo.residentImgFormat)
        {
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case __VK_FORMAT_A4R4G4B4_UNFORM_PACK16:
            tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R16_UINT;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
            break;
        case VK_FORMAT_B8G8R8A8_SRGB:
            tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        default:
            tmpBufView.formatInfo.residentImgFormat = tmpBufView.createInfo.format;
            break;
        }

        bufView = &tmpBufView;

        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;
        params->dstExtent = srcRes->u.img.extent;
        dstSize.width  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstSize.height = dstRes->u.buf.imgHeight != 0 ? dstRes->u.buf.imgHeight : srcRes->u.img.extent.height;
        dstSize.depth  = srcRes->u.img.extent.depth;

        pUserSize = &dstSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    if (imgView && imgView->formatInfo->compressed)
    {
        __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgView->createInfo.image);
        __vkImageLevel *baseLevel = &img->pImgLevels[imgView->createInfo.subresourceRange.baseMipLevel];

        uint32_t width = gcmALIGN(baseLevel->requestW, imgView->formatInfo->blockSize.width) / imgView->formatInfo->blockSize.width;
        uint32_t height = gcmALIGN(baseLevel->requestH, imgView->formatInfo->blockSize.height) / imgView->formatInfo->blockSize.height;

        if (imgView->formatInfo->bitsPerBlock == 128)
        {
            width *= 2;
        }
        hwImgDesc[0].imageInfo[2] = width | (height << 16);
    }

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

OnError:
    return result;
}

VkResult halti5_program_clear_dst_img(
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
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (dstRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pDstImg = dstRes->u.img.pImage;
        uint32_t bitsPerPixel = pDstImg->formatInfo.bitsPerBlock / pDstImg->formatInfo.partCount;

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

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        __VK_MEMCOPY(&tmpFormatInfo, &pDstImg->formatInfo, sizeof(tmpFormatInfo));

        switch (bitsPerPixel)
        {
        case 8:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R8_UINT;
            break;
        case 16:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R8G8_UINT;
            break;
        case 32:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R8G8B8A8_UINT;
            break;
        case 64:
            tmpFormatInfo.residentImgFormat = VK_FORMAT_R16G16B16A16_UINT;
            break;
        default:
            __VK_ASSERT(0);
            break;
        }

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        params->dstOffset = dstRes->u.img.offset;
        params->dstExtent = dstRes->u.img.extent;
    }
    else
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
        tmpBufView.createInfo.format = VK_FORMAT_R32_UINT;
        tmpBufView.createInfo.offset = dstRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = g_vkFormatInfoTable[VK_FORMAT_R32_UINT];
        tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
        bufView = &tmpBufView;

        dstSize.width  = dstRes->u.buf.rowLength ;
        dstSize.height = dstRes->u.buf.imgHeight ;
        dstSize.depth  = 1;
        params->dstExtent = dstSize;
        pUserSize = &dstSize;
    }

    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

    if (blitProg->kind == HALTI3_CLEAR_TO_2LAYERS_IMG)
    {
        hwMapping = &blitProg->dstImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
            + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
            + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[1].imageInfo);
    }

OnError:
    return result;
}

static VkResult halti5_program_blit_const(
    IN __vkCommandBuffer *cmdBuf,
    IN halti5_vscprogram_blit *blitProg,
    IN uint32_t **states,
    IN __vkComputeBlitParams *params
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    gctINT_PTR   pI = gcvNULL;
    gctUINT_PTR  pU = gcvNULL;
    gctFLOAT_PTR pF = gcvNULL;
    uint32_t address;
    uint32_t hwConstRegAddr;
    SHADER_CONSTANT_HW_LOCATION_MAPPING *hwMapping = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    __vkScratchMem *pScratchMem = __vkGetScratchMem(cmdBuf, 144);
    VkResult result = VK_SUCCESS;

    __VK_ONERROR(__vk_MapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem->memory,
                                0, 128, 0, (void**)&pF));
    __VK_MEMZERO(pF, 128);

    /* > srcOffset, srcExtent, dstOffset, dstExtent, invert: same as parameter
       > scale: srcExtent / (float)dstExtent
       > srcSize: image size of the src sub image
    */
    /* scale */
    pF[0] = params->srcExtent.width  / (gctFLOAT)params->dstExtent.width;
    pF[1] = params->srcExtent.height / (gctFLOAT)params->dstExtent.height;
    pF[3] = params->srcExtent.depth  / (gctFLOAT)params->dstExtent.depth;

    /* invert */
    pI = (gctINT_PTR)(pF + 4);
    pI[0] = (gctINT)params->reverse.x;
    pI[1] = (gctINT)params->reverse.y;
    pI[3] = (gctINT)params->reverse.z;

    /* dstOffset */
    pI += 4;
    pI[0] = params->dstOffset.x;
    pI[1] = params->dstOffset.y;
    pI[3] = params->dstOffset.z;

    /* dstExtent */
    pU = (gctUINT_PTR)(pI + 4);
    pU[0] = params->dstExtent.width;
    pU[1] = params->dstExtent.height;
    pU[3] = params->dstExtent.depth;

    /* srcOffset */
    pF = (gctFLOAT_PTR)(pU + 4);
    pF[0] = (gctFLOAT)params->srcOffset.x;
    pF[1] = (gctFLOAT)params->srcOffset.y;
    pF[3] = (gctFLOAT)params->srcOffset.z;

    /* srcExtent */
    pF += 4;
    pF[0] = (gctFLOAT)params->srcExtent.width;
    pF[1] = (gctFLOAT)params->srcExtent.height;
    pF[3] = (gctFLOAT)params->srcExtent.depth;

    /* srcSize */
    pF += 4;
    pF[0] = (gctFLOAT)params->srcSize.width;
    pF[1] = (gctFLOAT)params->srcSize.height;
    pF[3] = (gctFLOAT)params->srcSize.depth;

    /* uint clear value */
    pU = (gctUINT_PTR)(pF + 4);
    pU[0] = params->uClearValue0[0];
    pU[1] = params->uClearValue0[1];
    pU[2] = params->uClearValue0[2];
    pU[3] = params->uClearValue0[3];

    pU += 4;
    pU[0] = params->uClearValue1[0];
    pU[1] = params->uClearValue1[1];
    pU[2] = params->uClearValue1[2];
    pU[3] = params->uClearValue1[3];

    hwMapping = &blitProg->constEntry->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_MEMORY);
    __VK_ASSERT(hwMapping->hwLoc.memAddr.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                   +  hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->firstValidHwChannel;

    address = pScratchMem->memory->devAddr;
    __vkCmdLoadSingleHWState(states, hwConstRegAddr, VK_FALSE, address);

OnError:
    if (pI)
    {
        __vk_UnmapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem->memory);
    }
    return result;
}

static VkResult halti5_program_copy_src_img(
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
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (srcRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pSrcImg = srcRes->u.img.pImage;

        params->srcOffset = srcRes->u.img.offset;

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

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        switch (pSrcImg->formatInfo.bitsPerBlock)
        {
        case 128:
            __VK_MEMCOPY(&tmpFormatInfo, &g_vkFormatInfoTable[VK_FORMAT_R16G16B16A16_UINT], sizeof(tmpFormatInfo));
            break;
        default:
            /* Not support other bpp currently */
            __VK_ASSERT(0);
            break;
        }
        tmpFormatInfo.partCount = pSrcImg->formatInfo.partCount;

        /* For 1 part images, faked it as double widthed with half bpp */
        if (pSrcImg->formatInfo.partCount == 1)
        {
            static VkExtent3D userSize;
            __vkImage *pDstImg = dstRes->u.img.pImage;
            __vkImageLevel *pImgLevel = &pSrcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];

            userSize.width  = pImgLevel->requestW * pDstImg->formatInfo.partCount;
            userSize.height = pImgLevel->requestH;
            userSize.depth  = srcRes->u.img.extent.depth;

            pUserSize = &userSize;
        }
        tmpImgView.formatInfo = &tmpFormatInfo;

        imgView = &tmpImgView;
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D srcSize;
        __vkImage *pDstImg = dstRes->u.img.pImage;

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

        switch (pDstImg->formatInfo.bitsPerBlock)
        {
        case 128:
            /* Dst must be 2 part faked format here. */
            if (pDstImg->formatInfo.partCount == 2)
            {
                tmpBufView.formatInfo = g_vkFormatInfoTable[VK_FORMAT_R16G16B16A16_UINT];
            }
            break;
        default:
            break;
        }

        bufView = &tmpBufView;

        srcSize.width  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        srcSize.height = srcRes->u.buf.imgHeight != 0 ? srcRes->u.buf.imgHeight : dstRes->u.img.extent.height;
        srcSize.depth  = dstRes->u.img.extent.depth;

        /* Double width while halve bpp */
        srcSize.width *= pDstImg->formatInfo.partCount;

        pUserSize = &srcSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    hwMapping = &blitProg->srcImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

    if (blitProg->kind == HALTI5_BLIT_2LAYERS_IMG_TO_BUF)
    {
        hwMapping = &blitProg->srcImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                       + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                       + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[1].imageInfo);
    }

OnError:
    return result;
}

static VkResult halti5_program_copy_dst_img(
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
    VkExtent3D *pUserSize = gcvNULL;
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

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        switch (pDstImg->formatInfo.bitsPerBlock)
        {
        case 128:
            __VK_MEMCOPY(&tmpFormatInfo, &g_vkFormatInfoTable[VK_FORMAT_R16G16B16A16_UINT], sizeof(tmpFormatInfo));
            break;
        default:
            /* Not support other bpp currently */
            __VK_ASSERT(0);
            break;
        }
        tmpFormatInfo.partCount = pDstImg->formatInfo.partCount;

        /* For 1 part images, faked it as double widthed with half bpp */
        if (pDstImg->formatInfo.partCount == 1)
        {
            static VkExtent3D userSize;
            __vkImage *pSrcImg = srcRes->u.img.pImage;
            __vkImageLevel *pImgLevel = &pDstImg->pImgLevels[dstRes->u.img.subRes.mipLevel];

            userSize.width  = pImgLevel->requestW * pSrcImg->formatInfo.partCount;
            userSize.height = pImgLevel->requestH;
            userSize.depth  = dstRes->u.img.extent.depth;

            pUserSize = &userSize;
        }
        tmpImgView.formatInfo = &tmpFormatInfo;

        imgView = &tmpImgView;

        params->dstExtent = dstRes->u.img.extent;
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D dstSize;
        __vkImage *pSrcImg = srcRes->u.img.pImage;

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

        switch (pSrcImg->formatInfo.bitsPerBlock)
        {
        case 128:
            /* Dst must be 2 part faked format here. */
            if (pSrcImg->formatInfo.partCount == 2)
            {
                tmpBufView.formatInfo = g_vkFormatInfoTable[VK_FORMAT_R16G16B16A16_UINT];
            }
            break;
        default:
            break;
        }

        bufView = &tmpBufView;

        //params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;
        params->dstExtent = srcRes->u.img.extent;
        dstSize.width  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstSize.height = dstRes->u.buf.imgHeight != 0 ? dstRes->u.buf.imgHeight : srcRes->u.img.extent.height;
        dstSize.depth  = srcRes->u.img.extent.depth;

        /* Double width while halve bpp */
        dstSize.width *= pSrcImg->formatInfo.partCount;

        pUserSize = &dstSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[0].imageInfo);

    if (blitProg->kind == HALTI5_BLIT_BUF_TO_2LAYERS_IMG)
    {
        hwMapping = &blitProg->dstImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                       + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo * 4)
                       + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[1].imageInfo);
    }

OnError:
    return result;
}

static halti5_vscprogram_blit* halti5_GetComputeBlitProg(
    __vkDevContext *devCtx,
    uint32_t blitKind
    )
{
    SHADER_HANDLE blitVIR = VK_NULL_HANDLE;
    halti5_vscprogram_blit *blitProg = gcvNULL;
    halti5_vscprogram_blit *retProg = gcvNULL;
    halti5_module *chipModule = (halti5_module*)devCtx->chipPriv;
    VkResult result = VK_SUCCESS;

    if (blitKind >= HALTI5_BLIT_NUM)
    {
        __VK_ONERROR(VK_ERROR_INITIALIZATION_FAILED);
    }

    blitProg = &chipModule->blitProgs[blitKind];
    if (!blitProg->inited)
    {
        static VSC_PROGRAM_RESOURCE_BINDING vscResBinding[] =
        {
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_F */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_I */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 2, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_U */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 3, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_F */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 4, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_I */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 5, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_U */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 6, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_F */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 7, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_I */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 8, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_U */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 9, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_F */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 10, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_I */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 11, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_U */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 12, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_U0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 13, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_U1 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 14, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_U0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 15, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_U1 */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 16, 1}, VSC_SHADER_STAGE_BIT_CS }, /* srcImg2D_F */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 17, 1 }, VSC_SHADER_STAGE_BIT_CS }, /*srcImg2D_I */

            {{VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER, 0, 20, 1}, VSC_SHADER_STAGE_BIT_CS}, /* uniform PARAMS */
        };
        VSC_PROGRAM_LINKER_PARAM vscLinkParams;
        VSC_PROGRAM_RESOURCE_LAYOUT vscResLayout;
        VSC_PROGRAM_RESOURCE_SET vscResSet;
        SpvDecodeInfo decodeInfo;
        VkPipelineShaderStageCreateInfo stageInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            gcvNULL,
            0,
            VK_SHADER_STAGE_COMPUTE_BIT,
            VK_NULL_HANDLE,
            gcvNULL,
            gcvNULL
        };

        static const struct
        {
            const char* name;
            uint32_t    srcBindings[__VK_MAX_PARTS];
            uint32_t    dstBindings[__VK_MAX_PARTS];
            uint32_t    uboBinding;
        }
        entryInfos[] =
        {
            {"blit2D_unorm_float", { 0, ~0u}, { 6, ~0u}, 20},
            {"blit2D_unorm_a1r5g5b5_packed", { 0, ~0u}, { 8, ~0u}, 20},
            {"blit2D_unorm_a2b10g10r10_packed", { 0, ~0u}, { 8, ~0u}, 20},
            {"blit2D_sint", { 1, ~0u}, { 7, ~0u}, 20},
            {"blit2D_uint", { 2, ~0u}, { 8, ~0u}, 20},
            {"blit2D_uint_a2b10g10r10_packed", { 2, ~0u}, { 8, ~0u}, 20},
            {"blit2D_unorm_r5g6b5_packed", { 0, ~0u}, { 8, ~0u}, 20},
            {"blit2D_unorm_a4r4g4b4_packed", { 0, ~0u}, { 8, ~0u}, 20},

            {"blit3D_unorm_float", { 3, ~0u}, { 9, ~0u}, 20},
            {"blit3D_unorm_a1r5g5b5_packed", { 3, ~0u}, {11, ~0u}, 20},
            {"blit3D_unorm_a2b10g10r10_packed", { 3, ~0u}, {11, ~0u}, 20},
            {"blit3D_sint", { 4, ~0u}, {10, ~0u}, 20},
            {"blit3D_uint", { 5, ~0u}, {11, ~0u}, 20},
            {"blit3D_uint_a2b10g10r10_packed", { 5, ~0u}, {11, ~0u}, 20},
            {"blit3D_unorm_r5g6b5_packed", { 3, ~0u}, {11, ~0u}, 20},
            {"blit3D_unorm_a4r4g4b4_packed", { 3, ~0u}, {11, ~0u}, 20},

            {"copy_2layers_img_to_buf", {12, 13}, {14, 15}, 20},
            {"copy_buf_to_2layers_img", {12, 13}, {14, 15}, 20},
            {"copy_2D_unorm_float", {16, ~0u}, {6, ~0u}, 20},

            {"copy_buf_to_x8d24_img", { 0, ~0u}, {6, ~0u}, 20},
            {"copy_buf_to_d24s8_img_stencil", { 0, ~0u}, {8, ~0u}, 20},
            {"copy_buf_to_d24s8_img_depth", { 0, ~0u}, {8, ~0u}, 20},
            {"copy_img_to_d24s8_img_stencil", { 0, ~0u}, { 8, ~0u}, 20},
            {"copy_img_to_d24s8_img_depth", { 0, ~0u}, { 8, ~0u}, 20},

            {"clear_2D_uint", {~0u,~0u}, { 8, ~0u}, 20},
            {"clear_to_2layers_img", {~0u,~0u}, {14, 15}, 20},
            {"blit_2D_buffer", {17, ~0u}, {7, ~0u}, 20},
        };

        PROG_VK_RESOURCE_SET *progResSet = gcvNULL;

        __VK_MEMZERO(&vscResLayout, sizeof(vscResLayout));
        vscResLayout.resourceSetCount = 1;
        vscResLayout.pResourceSets = &vscResSet;
        vscResSet.resourceBindingCount = __VK_COUNTOF(vscResBinding);
        vscResSet.pResouceBindings = vscResBinding;

        __VK_MEMZERO(&decodeInfo, sizeof(decodeInfo));
        stageInfo.pName = entryInfos[blitKind].name;
        decodeInfo.binary = (gctUINT*)gc_halti5_blit_comp;
        decodeInfo.sizeInByte = (gctUINT)sizeof(gc_halti5_blit_comp);
        decodeInfo.stageInfo = (gctPOINTER)(&stageInfo);
        decodeInfo.specFlag = SPV_SPECFLAG_ENTRYPOINT | SPV_SPECFLAG_SPECIFIED_LOCAL_SIZE;
        decodeInfo.localSize[0] = VIV_LOCAL_X;
        decodeInfo.localSize[1] = VIV_LOCAL_Y;
        decodeInfo.localSize[2] = VIV_LOCAL_Z;
        decodeInfo.tcsInputVertices = 0;
        decodeInfo.funcCtx = gcvNULL;
        decodeInfo.renderpassInfo = gcvNULL;
        decodeInfo.subPass = ~0U;
        __VK_ONERROR((gcvSTATUS_OK == gcSPV_Decode(&decodeInfo, &blitVIR)) ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

        __VK_MEMZERO(&vscLinkParams, sizeof(vscLinkParams));
        vscLinkParams.hShaderArray[VSC_SHADER_STAGE_CS] = blitVIR;
        vscLinkParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
        vscLinkParams.pGlApiCfg = &devCtx->pPhyDevice->shaderCaps;
        vscLinkParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
        vscLinkParams.cfg.ctx.appNameId = gcvPATCH_INVALID;
        vscLinkParams.cfg.ctx.isPatchLib = gcvFALSE;
        vscLinkParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS
                                 | VSC_COMPILER_FLAG_COMPILE_CODE_GEN
                                 | VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO
                                 | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
        vscLinkParams.cfg.optFlags = VSC_COMPILER_OPT_FULL;
        vscLinkParams.pPgResourceLayout = &vscResLayout;
        __VK_ONERROR(vscLinkProgram(&vscLinkParams, &blitProg->pep, &blitProg->hwStates));

        progResSet = &blitProg->pep.u.vk.pResourceSets[0];
        switch (blitKind)
        {
        case HALTI5_BLIT_2LAYERS_IMG_TO_BUF:
        case HALTI5_BLIT_BUF_TO_2LAYERS_IMG:
            blitProg->srcImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->srcImgEntry[1] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[1]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->dstImgEntry[1] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[1]);

            blitProg->program_src   = halti5_program_copy_src_img;
            blitProg->program_dst   = halti5_program_copy_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_2D_UNORM_FLOAT:
            blitProg->srcImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_src   = halti3_program_copy_src_img;
            blitProg->program_dst   = halti3_program_copy_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_BUF_TO_X8D24_IMG:
            blitProg->srcTexEntry = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_src = halti5_program_blit_src_tex;
            blitProg->program_dst = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_BUF_TO_D24S8IMG_STENCIL:
            blitProg->srcTexEntry = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->program_src = halti5_program_blit_src_tex;
            blitProg->program_dst = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;
        case HALTI5_BLIT_COPY_BUF_TO_D24S8IMG_DEPTH:
            blitProg->srcTexEntry = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->program_src = halti5_program_blit_src_tex;
            blitProg->program_dst = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_IMG_TO_D24S8IMG_STENCIL:
            blitProg->srcTexEntry = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->program_src = halti5_program_blit_src_tex;
            blitProg->program_dst = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_IMG_TO_D24S8IMG_DEPTH:
            blitProg->srcTexEntry = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->program_src = halti5_program_blit_src_tex;
            blitProg->program_dst = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI3_CLEAR_2D_UINT:
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_dst   = halti5_program_clear_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI3_CLEAR_TO_2LAYERS_IMG:
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->dstImgEntry[1] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[1]);

            blitProg->program_dst   = halti5_program_clear_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;
        case HALTI3_BLIT_BUFFER_2D:
            blitProg->srcImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            blitProg->program_dst   = halti3_program_blit_buffer_dst;
            blitProg->program_src   = halti3_program_blit_buffer_src;
            blitProg->program_const = halti5_program_blit_const;
            break;

        default:
            blitProg->srcTexEntry    = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_src   = (devCtx->database->REG_Halti5)
                                    ? halti5_program_blit_src_tex
                                    : halti2_program_blit_src_tex;
            blitProg->program_dst   = halti5_program_blit_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;
        }
        blitProg->constEntry = halti5_getUboEntry(progResSet, entryInfos[blitKind].uboBinding);

        blitProg->kind = blitKind;
        blitProg->inited = VK_TRUE;
    }

    retProg = blitProg;

OnError:
    if (blitVIR)
    {
        vscDestroyShader(blitVIR);
    }
    return retProg;
}

static uint32_t halti5_detect_blit_kind(
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes
    )
{
    uint32_t kind = HALTI5_BLIT_NUM;
    uint32_t srcParts, dstParts;
    uint32_t srcFormat, dstFormat;
    uint32_t srcCategory, dstCategory;
    VkImageType srcType, dstType;
    VkImageAspectFlags srcAspect;
    __VK_DEBUG_ONLY(VkImageAspectFlags dstAspect;)
    __vkImage *pSrcImg, *pDstImg;
    const __vkFormatInfo *fmtInfo;

    if (!srcRes->isImage && !dstRes->isImage)
    {
        return HALTI3_BLIT_BUFFER_2D;
    }

    if (srcRes->isImage)
    {
        pSrcImg = srcRes->u.img.pImage;

        srcFormat = pSrcImg->formatInfo.residentImgFormat;
        srcParts = pSrcImg->formatInfo.partCount;
        srcCategory = pSrcImg->formatInfo.category;
        srcType = pSrcImg->createInfo.imageType;
        srcAspect = srcRes->u.img.subRes.aspectMask;
    }
    else
    {
        pDstImg = dstRes->u.img.pImage;

        srcFormat = pDstImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[srcFormat];
        srcParts = fmtInfo->partCount;
        srcCategory = fmtInfo->category;
        srcType = pDstImg->createInfo.imageType;
        srcAspect = dstRes->u.img.subRes.aspectMask;
    }

    /* Change float/unorm/snorm to be same categroy. */
    switch (srcCategory)
    {
    case __VK_FMT_CATEGORY_SNORM:
    case __VK_FMT_CATEGORY_UFLOAT:
    case __VK_FMT_CATEGORY_SFLOAT:
        srcCategory = __VK_FMT_CATEGORY_UNORM;
        break;
    default:
        break;
    }

    if (dstRes->isImage)
    {
        pDstImg = dstRes->u.img.pImage;

        dstFormat = pDstImg->formatInfo.residentImgFormat;
        dstParts = pDstImg->formatInfo.partCount;
        dstCategory = pDstImg->formatInfo.category;
        dstType = pDstImg->createInfo.imageType;
        __VK_DEBUG_ONLY(dstAspect = dstRes->u.img.subRes.aspectMask);
    }
    else
    {
        pSrcImg = srcRes->u.img.pImage;

        dstFormat = pSrcImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[dstFormat];
        dstParts = fmtInfo->partCount;
        dstCategory = fmtInfo->category;
        dstType = pSrcImg->createInfo.imageType;
        __VK_DEBUG_ONLY(dstAspect = srcRes->u.img.subRes.aspectMask);
    }

    /* Change float/unorm/snorm to be same categroy. */
    switch (dstCategory)
    {
    case __VK_FMT_CATEGORY_SNORM:
    case __VK_FMT_CATEGORY_UFLOAT:
    case __VK_FMT_CATEGORY_SFLOAT:
        dstCategory = __VK_FMT_CATEGORY_UNORM;
        break;
    default:
        break;
    }

    __VK_ASSERT(srcAspect == dstAspect);
    /* For depth or stencil only aspect: cannot support combined formats now. */
    if (((srcAspect & VK_IMAGE_ASPECT_DEPTH_BIT  ) && !(srcAspect & ~VK_IMAGE_ASPECT_DEPTH_BIT  )) ||
        ((srcAspect & VK_IMAGE_ASPECT_STENCIL_BIT) && !(srcAspect & ~VK_IMAGE_ASPECT_STENCIL_BIT))
       )
    {
        /* For depth or stencil only aspect: cannot support combined formats now. */
        switch (dstFormat)
        {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit does NOT support the combination now: dstAspect=0x%x dstFormat=%d\n",
                             dstAspect, dstFormat);
            __VK_ASSERT(VK_FALSE);
            return kind;
            break;
        default:
            break;
        }
    }

    if (srcCategory != dstCategory)
    {
        __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit unsupported categories: srcCategory=%u, dstCategory=%u\n",
                         srcCategory, dstCategory);
        __VK_ASSERT(VK_FALSE);
    }
    else if (srcParts != dstParts)
    {
        if (srcParts == 2 && dstParts == 1)
        {
            kind = HALTI5_BLIT_2LAYERS_IMG_TO_BUF;
        }
        else if (srcParts == 1 && dstParts == 2)
        {
            kind = HALTI5_BLIT_BUF_TO_2LAYERS_IMG;
        }
        else
        {
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit unsupported fake formats: srcFormat=%u, dstFormat=%u\n",
                             srcFormat, dstFormat);
            __VK_ASSERT(VK_FALSE);
        }
    }
    else if ((srcRes->isImage && !dstRes->isImage && srcFormat == __VK_FORMAT_R8_1_X8R8G8B8) ||
             (!srcRes->isImage && dstRes->isImage && dstFormat == __VK_FORMAT_R8_1_X8R8G8B8))
    {
        kind = HALTI5_BLIT_COPY_2D_UNORM_FLOAT;
    }
    else
    {
        switch (dstFormat)
        {
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_A1R5G5B5_PACKED;
            break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_R5G6B5_PACKED;
            break;
        case __VK_FORMAT_A4R4G4B4_UNFORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_A4R4G4B4_PACKED;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            kind = HALTI5_BLIT_2D_UNORM_A2B10G10R10_PACKED;
            break;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            kind = HALTI5_BLIT_2D_UINT_A2B10G10R10_PACKED;
            break;
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
            kind = HALTI5_BLIT_COPY_BUF_TO_X8D24_IMG;
            break;
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            if ((dstRes->u.img.subRes.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) && !srcRes->isImage)
            {
                kind = HALTI5_BLIT_COPY_BUF_TO_D24S8IMG_STENCIL;
            }
            else if (dstRes->u.img.subRes.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT && !srcRes->isImage)
            {
                kind = HALTI5_BLIT_COPY_BUF_TO_D24S8IMG_DEPTH;
            }
            else if ((dstRes->u.img.subRes.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) &&
                (srcRes->isImage) && (dstRes->isImage))
            {
                kind = HALTI5_BLIT_COPY_IMG_TO_D24S8IMG_STENCIL;
            }
            else if ((dstRes->u.img.subRes.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) &&
                (srcRes->isImage) && (dstRes->isImage))
            {
                kind = HALTI5_BLIT_COPY_IMG_TO_D24S8IMG_DEPTH;
            }
            break;
        default:
            switch (dstCategory)
            {
            case __VK_FMT_CATEGORY_SINT:
                kind = HALTI5_BLIT_2D_SINT;
                break;
            case __VK_FMT_CATEGORY_UINT:
                kind = HALTI5_BLIT_2D_UINT;
                break;
            default:
                kind = HALTI5_BLIT_2D_UNORM_FLOAT;
                break;
            }
            break;
        }

        /* If either type was of 3D, choose corresponding 3D variant. */
        if (srcType == VK_IMAGE_TYPE_3D || dstType == VK_IMAGE_TYPE_3D)
        {
            kind += (HALTI5_BLIT_3D_UNORM_FLOAT - HALTI5_BLIT_2D_UNORM_FLOAT);
        }
    }

    return kind;
}

VkResult halti5_computeBlit(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool3D *reverse,
    VkFilter filter
    )
{
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    uint32_t *scatch, *scatchBegin, *states;
    VkResult result = VK_SUCCESS;
    __vkComputeBlitParams params;
    halti5_vscprogram_blit *blitProg = gcvNULL;
    gcsHINT_PTR pHints = gcvNULL;
    uint32_t blitKind = halti5_detect_blit_kind(srcRes, dstRes);

    __VK_MEMZERO(&params, sizeof(params));

    if (blitKind >= HALTI5_BLIT_NUM)
    {
        __VK_ONERROR(VK_NOT_READY);
    }

    blitProg = halti5_GetComputeBlitProg(devCtx, blitKind);
    pHints = &blitProg->hwStates.hints;

    scatch = scatchBegin = &pCmdBuf->scratchCmdBuffer[pCmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        halti5_setMultiGpuSync((VkDevice)devCtx, &scatch, VK_NULL_HANDLE);

        *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&scatch)++ = 0;
;

    }

    if (reverse)
    {
        params.reverse = *reverse;
    }
    __VK_ONERROR(blitProg->program_src(pCmdBuf, blitProg, &scatch, srcRes, dstRes, filter, &params));
    __VK_ONERROR(blitProg->program_dst(pCmdBuf, blitProg, &scatch, srcRes, dstRes, &params));
    __VK_ONERROR(blitProg->program_const(pCmdBuf, blitProg, &scatch, &params));

    /*
    ** Program the shader
    */
    {
        const gcsFEATURE_DATABASE *database = devCtx->database;
        uint32_t reservedPages = database->SH_SNAP2PAGE_MAXPAGES_FIX ? 0 : 1;
        uint32_t i;

        __VK_MEMCOPY(scatch, blitProg->hwStates.pStateBuffer, blitProg->hwStates.stateBufferSize);
        scatch += (blitProg->hwStates.stateBufferSize / sizeof(uint32_t));

        __vkCmdLoadSingleHWState(&scatch, 0x0402, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (pHints->fsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:16) - (0 ?
 20:16) + 1))))))) << (0 ?
 20:16))) | (((gctUINT32) ((gctUINT32) (pHints->psHighPVaryingCount) & ((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ? 20:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (pHints->psInputControlHighpPosition) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0403, VK_FALSE, pHints->fsMaxTemp);

        __vkCmdLoadSingleHWState(&scatch, 0x0404, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0228, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x52C6, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x52C7, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x5286, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0440, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0450, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x02AA, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (pHints->shader2PaOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x028C, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (pHints->elementCount) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0E07, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->componentCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x040C, VK_FALSE, 0);

        __vkCmdLoadSingleHWState(&scatch, 0x0E22, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->ptSzAttrIndex * 4) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:8) - (0 ?
 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (pHints->pointCoordComponent) & ((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ? 14:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (pHints->rtArrayComponent) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:24) - (0 ?
 30:24) + 1))))))) << (0 ?
 30:24))) | (((gctUINT32) ((gctUINT32) (pHints->primIdComponent) & ((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:24) - (0 ? 30:24) + 1))))))) << (0 ? 30:24)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0E06, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
            );

        for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; ++i)
        {
            if (-1 != pHints->fsICachePrefetch[i])
            {
                __vkCmdLoadSingleHWState(&scatch, 0x0412, VK_FALSE, pHints->fsICachePrefetch[i]);
            }
        }
    }

    /*
    ** Program the compute dispatch
    */
    {
        uint32_t threadAllocation = gcmCEIL((gctFLOAT)(pHints->workGrpSize.x * pHints->workGrpSize.y * pHints->workGrpSize.z)
                                  / (devCtx->database->NumShaderCores * 4));

        __vkCmdLoadSingleHWState(&scatch, 0x0240, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (pHints->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0247, VK_FALSE, threadAllocation);

        __vkCmdLoadSingleHWState(&scatch, 0x024B, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x024D, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x024F, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0250, VK_FALSE,
                gcmCEIL((float)params.dstExtent.width  / VIV_LOCAL_X) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0251, VK_FALSE,
                gcmCEIL((float)params.dstExtent.height / VIV_LOCAL_Y) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0252, VK_FALSE,
                gcmCEIL((float)params.dstExtent.depth  / VIV_LOCAL_Z) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0253, VK_FALSE,
                pHints->workGrpSize.x - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0254, VK_FALSE,
                pHints->workGrpSize.y - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0255, VK_FALSE,
                pHints->workGrpSize.z - 1
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0248, VK_FALSE, 0xBADABEEB);
    }

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&scatch)++ = 0;
;


        halti5_setMultiGpuSync((VkDevice)devCtx, &scatch, VK_NULL_HANDLE);
    }

    pCmdBuf->curScrachBufIndex += (uint32_t)(scatch - scatchBegin);
    __VK_ASSERT(pCmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(cmdBuf, pCmdBuf->curScrachBufIndex, &states);
    __VK_MEMCOPY(states, pCmdBuf->scratchCmdBuffer, pCmdBuf->curScrachBufIndex * sizeof(uint32_t));
    __vk_CmdReleaseBuffer(cmdBuf, pCmdBuf->curScrachBufIndex);

OnError:
    pCmdBuf->curScrachBufIndex = 0;
    return result;
}

VkResult halti5_computeClear(
    VkCommandBuffer cmdBuf,
    VkClearValue *clearValue,
    __vkBlitRes *dstRes
    )
{
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    uint32_t *scatch, *scatchBegin, *states;
    VkResult result = VK_SUCCESS;
    __vkComputeBlitParams params;
    halti5_vscprogram_blit *blitProg = gcvNULL;
    gcsHINT_PTR pHints = gcvNULL;
    uint32_t partIndex = 0;
    uint32_t blitKind = HALTI5_BLIT_NUM;
    __vkImage *pDstImg;
    uint32_t bitsPerPixel;
    uint32_t tmpClearValue[4];

    if(dstRes->isImage)
    {
        if (dstRes->u.img.pImage->formatInfo.partCount == 2)
        {
            blitKind = HALTI3_CLEAR_TO_2LAYERS_IMG;
        }
        else
        {
            blitKind = HALTI3_CLEAR_2D_UINT;
        }
    }
    else
    {
        blitKind = HALTI3_CLEAR_2D_UINT;
    }


    if (blitKind >= HALTI5_BLIT_NUM)
    {
        __VK_ONERROR(VK_NOT_READY);
    }

    blitProg = halti5_GetComputeBlitProg(devCtx, blitKind);
    pHints = &blitProg->hwStates.hints;

    scatch = scatchBegin = &pCmdBuf->scratchCmdBuffer[pCmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        halti5_setMultiGpuSync((VkDevice)devCtx, &scatch, VK_NULL_HANDLE);

        *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&scatch)++ = 0;
;

    }

     __VK_MEMZERO(&params, sizeof(params));

    if(dstRes->isImage)
    {
        pDstImg = dstRes->u.img.pImage;

        for (partIndex = 0; partIndex < pDstImg->pImgLevels[dstRes->u.img.subRes.mipLevel].partCount; partIndex++)
        {
            __VK_ONERROR(__vkComputeClearVal(pDstImg,
                dstRes->u.img.subRes.aspectMask,
                clearValue,
                partIndex,
                &tmpClearValue[2 * partIndex],
                VK_NULL_HANDLE,
                VK_NULL_HANDLE));
        }

        bitsPerPixel = pDstImg->formatInfo.bitsPerBlock;

         switch (bitsPerPixel)
         {
         case 64:
             {
                 params.uClearValue0[0] =  tmpClearValue[0] & 0x0000FFFF;
                 params.uClearValue0[1] = (tmpClearValue[0] & 0xFFFF0000) >> 16;
                 params.uClearValue0[2] =  tmpClearValue[1] & 0x0000FFFF;
                 params.uClearValue0[3] = (tmpClearValue[1] & 0xFFFF0000) >> 16;
             }break;
         case 128:
             {
                 params.uClearValue0[0] =  tmpClearValue[0] & 0x0000FFFF;
                 params.uClearValue0[1] = (tmpClearValue[0] & 0xFFFF0000) >> 16;
                 params.uClearValue0[2] =  tmpClearValue[1] & 0x0000FFFF;
                 params.uClearValue0[3] = (tmpClearValue[1] & 0xFFFF0000) >> 16;

                 params.uClearValue1[0] =  tmpClearValue[2] & 0x0000FFFF;
                 params.uClearValue1[1] = (tmpClearValue[2] & 0xFFFF0000) >> 16;
                 params.uClearValue1[2] =  tmpClearValue[3] & 0x0000FFFF;
                 params.uClearValue1[3] = (tmpClearValue[3] & 0xFFFF0000) >> 16;
             }break;
         default:
             {
                 params.uClearValue0[0] =  tmpClearValue[0] & 0x000000FF;
                 params.uClearValue0[1] = (tmpClearValue[0] & 0x0000FF00) >> 8;
                 params.uClearValue0[2] = (tmpClearValue[0] & 0x00FF0000) >> 16;
                 params.uClearValue0[3] = (tmpClearValue[0] & 0xFF000000) >> 24;
             }break;
         }
    }
    else
    {

        params.uClearValue0[0] = clearValue->color.int32[0];
    }

    __VK_ONERROR(blitProg->program_dst(pCmdBuf, blitProg, &scatch, VK_NULL_HANDLE, dstRes, &params));
    __VK_ONERROR(blitProg->program_const(pCmdBuf, blitProg, &scatch, &params));

    /*
    ** Program the shader
    */
    {
        const gcsFEATURE_DATABASE *database = devCtx->database;
        uint32_t reservedPages = database->SH_SNAP2PAGE_MAXPAGES_FIX ? 0 : 1;
        uint32_t i;

        __VK_MEMCOPY(scatch, blitProg->hwStates.pStateBuffer, blitProg->hwStates.stateBufferSize);
        scatch += (blitProg->hwStates.stateBufferSize / sizeof(uint32_t));

        __vkCmdLoadSingleHWState(&scatch, 0x0402, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (pHints->fsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:16) - (0 ?
 20:16) + 1))))))) << (0 ?
 20:16))) | (((gctUINT32) ((gctUINT32) (pHints->psHighPVaryingCount) & ((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ? 20:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (pHints->psInputControlHighpPosition) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0403, VK_FALSE, pHints->fsMaxTemp);

        __vkCmdLoadSingleHWState(&scatch, 0x0404, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0228, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x52C6, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x52C7, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x5286, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0440, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0450, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x02AA, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (pHints->shader2PaOutputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x028C, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (pHints->elementCount) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0E07, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->componentCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x040C, VK_FALSE, 0);

        __vkCmdLoadSingleHWState(&scatch, 0x0E22, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->ptSzAttrIndex * 4) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:8) - (0 ?
 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (pHints->pointCoordComponent) & ((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ? 14:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (pHints->rtArrayComponent) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:24) - (0 ?
 30:24) + 1))))))) << (0 ?
 30:24))) | (((gctUINT32) ((gctUINT32) (pHints->primIdComponent) & ((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:24) - (0 ? 30:24) + 1))))))) << (0 ? 30:24)))
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0E06, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
            );

        for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; ++i)
        {
            if (-1 != pHints->fsICachePrefetch[i])
            {
                __vkCmdLoadSingleHWState(&scatch, 0x0412, VK_FALSE, pHints->fsICachePrefetch[i]);
            }
        }
    }

    /*
    ** Program the compute dispatch
    */
    {
        uint32_t threadAllocation = gcmCEIL((gctFLOAT)(pHints->workGrpSize.x * pHints->workGrpSize.y * pHints->workGrpSize.z)
                                  / (devCtx->database->NumShaderCores * 4));

        __vkCmdLoadSingleHWState(&scatch, 0x0240, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (pHints->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0247, VK_FALSE, threadAllocation);

        __vkCmdLoadSingleHWState(&scatch, 0x024B, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x024D, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x024F, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0250, VK_FALSE,
                gcmCEIL((float)params.dstExtent.width  / VIV_LOCAL_X) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0251, VK_FALSE,
                gcmCEIL((float)params.dstExtent.height / VIV_LOCAL_Y) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0252, VK_FALSE,
                gcmCEIL((float)params.dstExtent.depth  / VIV_LOCAL_Z) - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0253, VK_FALSE,
                pHints->workGrpSize.x - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0254, VK_FALSE,
                pHints->workGrpSize.y - 1
            );
        __vkCmdLoadSingleHWState(&scatch, 0x0255, VK_FALSE,
                pHints->workGrpSize.z - 1
            );

        __vkCmdLoadSingleHWState(&scatch, 0x0248, VK_FALSE, 0xBADABEEB);
    }

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&scatch)++ = 0;
;


        halti5_setMultiGpuSync((VkDevice)devCtx, &scatch, VK_NULL_HANDLE);
    }

    pCmdBuf->curScrachBufIndex += (uint32_t)(scatch - scatchBegin);
    __VK_ASSERT(pCmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(cmdBuf, pCmdBuf->curScrachBufIndex, &states);
    __VK_MEMCOPY(states, pCmdBuf->scratchCmdBuffer, pCmdBuf->curScrachBufIndex * sizeof(uint32_t));
    __vk_CmdReleaseBuffer(cmdBuf, pCmdBuf->curScrachBufIndex);

OnError:
    pCmdBuf->curScrachBufIndex = 0;
    return result;
}


