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
    const __vkFormatInfo *fmtInfo = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    uint32_t partIdx, partCount = 1;
    uint32_t partSize = 0;
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

    static const struct
    {
        uint32_t samplerCtrl0Reg;
        uint32_t samplerCtrl1Reg;
        uint32_t samplerLodMaxMinReg;
        uint32_t samplerLodBiasReg;
        uint32_t samplerAnisCtrlReg;
        uint32_t texDescAddrReg;
        uint32_t textureControlAddrReg;
        uint32_t txCommandReg;
    }
    s_TxHwRegisters[] =
    {
        {
            0x5800, 0x5880,
            0x5900, 0x5980,
            0x5A00, 0x5600,
            0x5680, 0x5311
        },
        {
            0x5B00, 0x5B80,
            0x5C00, 0x5C80,
            0x5D00, 0x5700,
            0x5780, 0x5312
        },
    };
    uint32_t txHwRegisterIdx = (devCtx->database->SMALLBATCH
                             && devCtx->pPhyDevice->phyDevConfig.options.smallBatch) ? 0 : 1;
    __vkScratchMem *pScratchMem[2] = {gcvNULL, gcvNULL};
    gcsTEXTUREDESCRIPTORREGS *pDesc[2] = {gcvNULL, gcvNULL};
    uint32_t txStride, txSliceSize, txHAlign, txMsaa;
    uint32_t txType, txSRGB, txSignExt, txAddressing, txIntCtrl;
    uint32_t swizzle_r, swizzle_g, swizzle_b, swizzle_a;
    const __vkFormatToHwTxFmtInfo *hwTxFmtInfo = gcvNULL;
    SHADER_SAMPLER_SLOT_MAPPING *hwMapping = gcvNULL;
    uint32_t hwSamplerNo;
    uint32_t tmpFormat;
    uint32_t address;
    int32_t  planeIdx = 0;

    if (srcRes->isImage)
    {
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        __vkImageLevel *pSrcLevel = &pSrcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];
        planeIdx = __vk_GetPlaneIndex(srcRes->u.img.subRes.aspectMask);
        planeIdx = planeIdx < 0 ? 0 : planeIdx;

        params->srcOffset = srcRes->u.img.offset;
        params->srcExtent = srcRes->u.img.extent;

        params->srcSize.width  = pSrcLevel->planeAllocedW[planeIdx];
        params->srcSize.height = pSrcLevel->planeAllocedH[planeIdx];
        params->srcSize.depth  = pSrcLevel->requestD;

        fmtInfo = __vk_GetPlaneFormatInfo(pSrcImg, srcRes->u.img.subRes.aspectMask);
        fmtInfo = __vk_GetVkFormatInfo((VkFormat)(fmtInfo == VK_NULL_HANDLE ? pSrcImg->formatInfo.residentImgFormat : fmtInfo->residentImgFormat));
        txStride = (uint32_t)pSrcLevel->planeStride[planeIdx];
        txSliceSize = (uint32_t)pSrcLevel->sliceSize;

        txType = (pSrcImg->createInfo.imageType == VK_IMAGE_TYPE_3D) ? 0x3 : 0x2;
        txAddressing = (pSrcImg->halTiling == gcvLINEAR) ? 0x3 : 0x0;
        txHAlign = pSrcImg->hAlignment;
        txMsaa = (pSrcImg->sampleInfo.product > 1) ? 1 : 0;

        address = pSrcImg->memory->devAddr;
        address += (uint32_t)(pSrcImg->memOffset + pSrcLevel->offset +
                              srcRes->u.img.subRes.arrayLayer * pSrcLevel->sliceSize);
        address += (uint32_t)__vk_GetPlaneOffset(pSrcImg, srcRes->u.img.subRes.aspectMask, srcRes->u.img.subRes.mipLevel);

        partCount = pSrcImg->formatInfo.partCount;
        partSize = (uint32_t)pSrcLevel->partSize;
    }
    else
    {
        uint32_t txFormat = VK_FORMAT_UNDEFINED;
        __vkBuffer *pSrcBuf = srcRes->u.buf.pBuffer;
        __vkImage  *pDstImg = dstRes->u.img.pImage;
        uint32_t alignWidth;

        params->srcOffset.x = params->srcOffset.y = params->srcOffset.z = 0;
        params->srcExtent = dstRes->u.img.extent;
        params->srcSize.width  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        params->srcSize.height = srcRes->u.buf.imgHeight != 0 ? srcRes->u.buf.imgHeight : dstRes->u.img.extent.height;
        params->srcSize.depth  = dstRes->u.img.extent.depth;

        params->flushTex = VK_TRUE;

        /* Special treatment for VK_IMAGE_ASPECT_STENCIL_BIT */
        txFormat = (dstRes->u.img.subRes.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
                 ? params->srcFormat
                 : pDstImg->createInfo.format;

        fmtInfo = __vk_GetVkFormatInfo((VkFormat) txFormat);
        alignWidth = gcmALIGN_NP2(params->srcSize.width, fmtInfo->blockSize.width);
        txStride = (alignWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        txSliceSize = (params->srcSize.height / fmtInfo->blockSize.height) * txStride;

        txType = (pDstImg->createInfo.imageType == VK_IMAGE_TYPE_3D) ? 0x3 : 0x2;
        txAddressing = 0x3;
        txHAlign = 0x1;
        txMsaa = 0;

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
        params->dstSRGB = __vk_GetVkFormatInfo(pSrcImg->createInfo.format)->category == __VK_FMT_CATEGORY_SRGB;
    }

    if (params->rawCopy)
    {
        /* raw copy was always handled slice by slice as 2D texture */
        txType = 0x2;
        params->srcOffset.z = 0;
        params->srcExtent.depth = 1;
        params->srcSize.depth = 1;

        /* Special handle for compressed formats */
        if (fmtInfo->compressed)
        {
            VkExtent2D rect = fmtInfo->blockSize;

            /* GBGR ycbcr fromat is 2X1 compressed, but when do copy and plane view, treat as R8G8_unorm */
            __VK_ASSERT(fmtInfo->blockSize.width != 2);
            params->srcOffset.x = gcmALIGN_NP2(params->srcOffset.x - rect.width  + 1, rect.width );
            params->srcOffset.y = gcmALIGN_NP2(params->srcOffset.y - rect.height + 1, rect.height);
            params->srcExtent.width  = gcmALIGN_NP2(params->srcExtent.width, rect.width ) / rect.width;
            params->srcExtent.height = gcmALIGN_NP2(params->srcExtent.height, rect.height) / rect.height;
            params->srcSize.width    = gcmALIGN_NP2(params->srcSize.width, rect.width ) / rect.width;
            params->srcSize.height   = gcmALIGN_NP2(params->srcSize.height, rect.height) / rect.height;

            if (fmtInfo->bitsPerBlock == 128)
            {
                params->srcOffset.x *= 2;
                params->srcExtent.width *= 2;
                params->srcSize.width *= 2;
            }
        }
    }

    switch (params->srcFormat)
    {
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case __VK_FORMAT_R16G16B16A16_SINT_2_R16G16_SINT:
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

    tmpFormat = params->srcFormat;
    switch (params->srcFormat)
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

    if (params->txSwizzles)
    {
        swizzle_r = halti5_helper_convertHwTxSwizzle(fmtInfo, params->txSwizzles->r, hwTxFmtInfo->hwSwizzles[0], hwTxFmtInfo->hwSwizzles);
        swizzle_g = halti5_helper_convertHwTxSwizzle(fmtInfo, params->txSwizzles->g, hwTxFmtInfo->hwSwizzles[1], hwTxFmtInfo->hwSwizzles);
        swizzle_b = halti5_helper_convertHwTxSwizzle(fmtInfo, params->txSwizzles->b, hwTxFmtInfo->hwSwizzles[2], hwTxFmtInfo->hwSwizzles);
        swizzle_a = halti5_helper_convertHwTxSwizzle(fmtInfo, params->txSwizzles->a, hwTxFmtInfo->hwSwizzles[3], hwTxFmtInfo->hwSwizzles);
    }
    else
    {
        swizzle_r = hwTxFmtInfo->hwSwizzles[0];
        swizzle_g = hwTxFmtInfo->hwSwizzles[1];
        swizzle_b = hwTxFmtInfo->hwSwizzles[2];
        swizzle_a = hwTxFmtInfo->hwSwizzles[3];
    }

    txIntCtrl = ((hwTxFmtInfo->hwFormat >> TX_FORMAT_FAST_FILTER_SHIFT) & 0x1)
             && (txType != 0x3)
             && !txSRGB
             && !devCtx->database->TX_NO_FIXED_FILTER;

    if (params->flushTex)
    {
        __vkCmdLoadSingleHWState(states, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))));
        __vkCmdLoadSingleHWState(states, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))));
    }

    for (partIdx = 0; partIdx < partCount; partIdx++)
    {
        uint32_t descAddress;
        pScratchMem[partIdx] = __vkGetScratchMem(cmdBuf, TX_HW_DESCRIPTOR_MEM_SIZE);
        __VK_ONERROR(__vk_MapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem[partIdx]->memory,
                                    0, TX_HW_DESCRIPTOR_MEM_SIZE, 0, (void**)&pDesc[partIdx]));
        __VK_MEMZERO(pDesc[partIdx], TX_HW_DESCRIPTOR_MEM_SIZE);

        pDesc[partIdx]->gcregTXAddress[0] = address + partIdx * partSize;

        pDesc[partIdx]->gcregTXConfig =
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

        pDesc[partIdx]->gcregTXSize =
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

        pDesc[partIdx]->gcregTX3D = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (params->srcSize.depth) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)));

        pDesc[partIdx]->gcregTXLinearStride =
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

        pDesc[partIdx]->gcregTXExtConfig =
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
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_COLOR_SWIZZLE_SHIFT)) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))
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

        pDesc[partIdx]->gcregTXConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (txSignExt) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)));
        pDesc[partIdx]->gcregTXConfig3 = ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (txMsaa) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

        pDesc[partIdx]->gcregTXSizeExt =
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

        pDesc[partIdx]->gcregTXVolumeExt =
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

        pDesc[partIdx]->gcregTXSlice = txSliceSize;

        hwMapping = &blitProg->srcTexEntry[partIdx]->hwMappings[VSC_SHADER_STAGE_CS].samplerMapping;
        hwSamplerNo = hwMapping->hwSamplerSlot + pHints->samplerBaseOffset[VSC_SHADER_STAGE_CS];

        descAddress = pScratchMem[partIdx]->memory->devAddr;
        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].texDescAddrReg + hwSamplerNo, VK_FALSE, descAddress);

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].samplerCtrl0Reg + hwSamplerNo, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].samplerCtrl1Reg + hwSamplerNo, VK_FALSE,
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
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
            );

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].samplerLodMaxMinReg + hwSamplerNo, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].samplerLodBiasReg + hwSamplerNo, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].samplerAnisCtrlReg + hwSamplerNo, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].textureControlAddrReg + hwSamplerNo, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, s_TxHwRegisters[txHwRegisterIdx].txCommandReg, VK_FALSE,
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
    }

OnError:
    for (partIdx = 0; partIdx < partCount; partIdx++)
    {
        if (pDesc[partIdx])
        {
            __vk_UnmapMemory((VkDevice)devCtx, (VkDeviceMemory)(uintptr_t)pScratchMem[partIdx]->memory);
        }
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
    HwImgDesc hwImgDesc[__VK_MAX_PARTS * __VK_MAX_PLANE];
    uint32_t hwConstRegAddr;
    uint32_t partCount = 1;
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;
    int32_t planeIdx = 0;
    __vkFormatInfo *fmtInfo = VK_NULL_HANDLE;
    VkBool32 bYCbCr = VK_FALSE;

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
        planeIdx = __vk_GetPlaneIndex(dstRes->u.img.subRes.aspectMask);
        fmtInfo  = planeIdx > -1 ? __vk_GetPlaneFormatInfo(pDstImg, dstRes->u.img.subRes.aspectMask) : &pDstImg->formatInfo;
        planeIdx = planeIdx < 0 ? 0 : planeIdx;
        bYCbCr   = pDstImg->ycbcrFormatInfo.bYUVFormat;

        __VK_MEMCOPY(&tmpFormatInfo, fmtInfo, sizeof(tmpFormatInfo));

        tmpFormatInfo.residentImgFormat = params->dstFormat;
        switch (tmpFormatInfo.residentImgFormat)
        {
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

        default:
            break;
        }

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        partCount = tmpImgView.formatInfo->partCount;

        params->dstOffset = dstRes->u.img.offset;
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

        switch (tmpBufView.formatInfo.residentImgFormat)
        {
        case VK_FORMAT_B8G8R8A8_SRGB:
            /* Disable dst SRGB if src also require SRGB */
            if (params->srcSRGB)
            {
                tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_B8G8R8A8_UNORM;
            }
        default:
            break;
        }

        bufView = &tmpBufView;

        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;
        params->dstExtent = srcRes->u.img.extent;
        dstSize.width  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstSize.height = dstRes->u.buf.imgHeight != 0 ? dstRes->u.buf.imgHeight : srcRes->u.img.extent.height;
        dstSize.depth  = srcRes->u.img.extent.depth;

        if (params->rawCopy && bufView->formatInfo.compressed)
        {
            VkExtent2D rect = bufView->formatInfo.blockSize;

            params->dstOffset.x = gcmALIGN_NP2(params->dstOffset.x - rect.width  + 1, rect.width );
            params->dstOffset.y = gcmALIGN_NP2(params->dstOffset.y - rect.height + 1, rect.height);
            params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width ) / rect.width;
            params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
            dstSize.width  = gcmALIGN_NP2(dstSize.width, rect.width ) / rect.width;
            dstSize.height = gcmALIGN_NP2(dstSize.height, rect.height) / rect.height;

            if (bufView->formatInfo.bitsPerBlock == 128)
            {
                params->dstOffset.x *= 2;
                params->dstExtent.width *= 2;
                dstSize.width *= 2;
            }
        }

        pUserSize = &dstSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    if (params->rawCopy && imgView && imgView->formatInfo->compressed && !bYCbCr)
    {
        VkBool32 srcCompressed = srcRes->isImage && srcRes->u.img.pImage->formatInfo.compressed;
        VkExtent2D rect = imgView->formatInfo->blockSize;
        __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgView->createInfo.image);
        __vkImageLevel *baseLevel = &img->pImgLevels[imgView->createInfo.subresourceRange.baseMipLevel];

        uint32_t width  = gcmALIGN_NP2(baseLevel->requestW, rect.width ) / rect.width;
        uint32_t height = gcmALIGN_NP2(baseLevel->requestH, rect.height) / rect.height;

        params->dstOffset.x = gcmALIGN_NP2(params->dstOffset.x - rect.width  + 1, rect.width ) / rect.width;
        params->dstOffset.y = gcmALIGN_NP2(params->dstOffset.y - rect.height + 1, rect.height) / rect.height;
        if (srcCompressed)
        {
            params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width ) / rect.width;
            params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
        }

        if (imgView->formatInfo->bitsPerBlock == 128)
        {
            width *= 2;
            params->dstOffset.x *= 2;
            params->dstExtent.width *= 2;
        }
        hwImgDesc[planeIdx * __VK_MAX_PARTS + 0].imageInfo[2] = width | (height << 16);
    }

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 0].imageInfo);

    if (partCount == 2)
    {
        hwMapping = &blitProg->dstImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                       + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                       + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 1].imageInfo);
    }

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
    HwImgDesc hwImgDesc[__VK_MAX_PARTS * __VK_MAX_PLANE];
    uint32_t hwConstRegAddr;
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;
    int32_t planeIdx = 0;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (dstRes->isImage)
    {
        __vkFormatInfo *fmtInfo = VK_NULL_HANDLE;
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pDstImg = dstRes->u.img.pImage;
        uint32_t bitsPerPixel = pDstImg->formatInfo.bitsPerBlock / pDstImg->formatInfo.partCount;

        __VK_ASSERT(!dstRes->u.img.pImage->ycbcrFormatInfo.bYUVFormat);

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

        planeIdx = __vk_GetPlaneIndex(dstRes->u.img.subRes.aspectMask);
        fmtInfo  = planeIdx > -1 ? __vk_GetPlaneFormatInfo(pDstImg, dstRes->u.img.subRes.aspectMask) : &pDstImg->formatInfo;
        planeIdx = planeIdx < 0 ? 0 : planeIdx;

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        __VK_MEMCOPY(&tmpFormatInfo, fmtInfo, sizeof(tmpFormatInfo));

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

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R32_UINT);
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
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 0].imageInfo);

    if (blitProg->kind == HALTI3_CLEAR_TO_2LAYERS_IMG)
    {
        hwMapping = &blitProg->dstImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
            + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
            + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 1].imageInfo);
    }

OnError:
    return result;
}

VkResult halti5_program_copy_src_oq_query_pool(
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

    __VK_MEMZERO(&hwImgDesc, sizeof(hwImgDesc));

    __VK_ASSERT(!srcRes->isImage);

    /* program buffer.*/
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
        tmpBufView.createInfo.format = VK_FORMAT_R32_UINT;
        tmpBufView.createInfo.offset = srcRes->u.buf.offset;
        tmpBufView.createInfo.range = VK_WHOLE_SIZE;

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R32_UINT);
        tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
        bufView = &tmpBufView;

        srcSize.width  = srcRes->u.buf.rowLength;
        srcSize.height = srcRes->u.buf.imgHeight;
        srcSize.depth  = 1;
        params->srcExtent = srcSize;
        pUserSize = &srcSize;
    }

    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

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

VkResult halti5_program_copy_dst_oq_query_pool(
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

    __VK_MEMZERO(&hwImgDesc, sizeof(hwImgDesc));

    __VK_ASSERT(!dstRes->isImage);

    /* program buffer.*/
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

        tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R32_UINT);
        tmpBufView.formatInfo.residentImgFormat = VK_FORMAT_R32_UINT;
        bufView = &tmpBufView;

        dstSize.width  = dstRes->u.buf.rowLength;
        dstSize.height = dstRes->u.buf.imgHeight;
        dstSize.depth  = 1;
        params->dstExtent = dstSize;
        pUserSize = &dstSize;
    }

    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

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
    gctBOOL_PTR  pB = gcvNULL;
    uint32_t addresses[3];
    uint32_t addrCount = 0;
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
    pU = (gctUINT_PTR)pF;
    pF[0] = params->srcExtent.width  / (gctFLOAT)params->dstExtent.width;
    pF[1] = params->srcExtent.height / (gctFLOAT)params->dstExtent.height;
    pF[2] = params->srcExtent.depth  / (gctFLOAT)params->dstExtent.depth;
    /* writeMask */
    pU[3] = params->channelWriteMask;

    /* invert */
    pI = (gctINT_PTR)(pF + 4);
    pU += 4;
    pI[0] = (gctINT)params->reverse.x;
    pI[1] = (gctINT)params->reverse.y;
    pI[2] = (gctINT)params->reverse.z;
    /* dstFormat */
    pU[3] = params->packFormat;

    /* dstOffset */
    pI += 4;
    pU += 4;
    pI[0] = params->dstOffset.x;
    pI[1] = params->dstOffset.y;
    pI[2] = params->dstOffset.z;
    pU[3] = params->srcParts;

    /* dstExtent */
    pU = (gctUINT_PTR)(pI + 4);
    pU[0] = params->dstExtent.width;
    pU[1] = params->dstExtent.height;
    pU[2] = params->dstExtent.depth;
    pU[3] = params->dstParts;

    /* srcOffset */
    pF = (gctFLOAT_PTR)(pU + 4);
    pB = (gctBOOL_PTR)pF;
    pF[0] = (gctFLOAT)params->srcOffset.x;
    pF[1] = (gctFLOAT)params->srcOffset.y;
    pF[2] = (gctFLOAT)params->srcOffset.z;
    pB[3] = (gctBOOL)params->dstSRGB;

    /* srcExtent */
    pF += 4;
    pF[0] = (gctFLOAT)params->srcExtent.width;
    pF[1] = (gctFLOAT)params->srcExtent.height;
    pF[2] = (gctFLOAT)params->srcExtent.depth;

    /* srcSize */
    pF += 4;
    pB = (gctBOOL_PTR)pF;
    pF[0] = (gctFLOAT)params->srcSize.width;
    pF[1] = (gctFLOAT)params->srcSize.height;
    pF[2] = (gctFLOAT)params->srcSize.depth;
    pB[3] = (gctBOOL)params->fmtConvert;

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
                   + (hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   +  hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->firstValidHwChannel;

    addresses[addrCount++] = pScratchMem->memory->devAddr;
    if (devCtx->enabledFeatures.robustBufferAccess)
    {
        addresses[addrCount++] = pScratchMem->memory->devAddr;
        addresses[addrCount++] = pScratchMem->memory->devAddr + 144 - 1;
    }
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, addrCount, addresses);


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
    HwImgDesc hwImgDesc[__VK_MAX_PARTS * __VK_MAX_PLANE];
    uint32_t hwConstRegAddr;
    VkExtent3D *pUserSize = gcvNULL;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;
    int32_t planeIdx = 0;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (srcRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pSrcImg = srcRes->u.img.pImage;
        int32_t bpp;

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
        tmpImgView.createInfo.subresourceRange.aspectMask = srcRes->u.img.subRes.aspectMask;

        planeIdx = __vk_GetPlaneIndex(srcRes->u.img.subRes.aspectMask);
        bpp  = planeIdx > -1 ? __vk_GetPlaneFormatInfo(pSrcImg, srcRes->u.img.subRes.aspectMask)->bitsPerBlock : pSrcImg->formatInfo.bitsPerBlock;
        planeIdx = planeIdx < 0 ? 0 : planeIdx;

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        switch (bpp)
        {
        case 128:
            __VK_MEMCOPY(&tmpFormatInfo, __vk_GetVkFormatInfo(VK_FORMAT_R16G16B16A16_UINT), sizeof(tmpFormatInfo));
            break;
        case 64:
            if (pSrcImg->formatInfo.partCount == 2)
                __VK_MEMCOPY(&tmpFormatInfo, __vk_GetVkFormatInfo(VK_FORMAT_R8G8B8A8_UINT), sizeof(tmpFormatInfo));
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
                tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R16G16B16A16_UINT);
            }
            break;
        default:
            break;
        }

        bufView = &tmpBufView;

        srcSize.width  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        srcSize.height = srcRes->u.buf.imgHeight != 0 ? srcRes->u.buf.imgHeight : dstRes->u.img.extent.height;
        srcSize.depth  = dstRes->u.img.extent.depth;

        if ((pDstImg->createInfo.flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) &&
            pDstImg->formatInfo.bitsPerBlock == 128)
        {
            __vkImageLevel *pImgLevel = &pDstImg->pImgLevels[dstRes->u.img.subRes.mipLevel];

            params->srcExtent.width = pImgLevel->allocedW;
            params->srcExtent.height = pImgLevel->allocedH;
            srcSize.width = pImgLevel->allocedW;
            srcSize.height = pImgLevel->allocedH;
            srcSize.depth = srcRes->u.img.extent.depth;
        }

        /* Double width while halve bpp */
        srcSize.width *= pDstImg->formatInfo.partCount;

        pUserSize = &srcSize;
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    hwMapping = &blitProg->srcImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 0].imageInfo);

    if (blitProg->kind == HALTI5_BLIT_2LAYERS_IMG_TO_BUF)
    {
        hwMapping = &blitProg->srcImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                       + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                       + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 1].imageInfo);
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
    HwImgDesc hwImgDesc[__VK_MAX_PARTS * __VK_MAX_PLANE];
    uint32_t hwConstRegAddr;
    VkExtent3D *pUserSize = gcvNULL;
    __vkImage *pSrcImg = srcRes->u.img.pImage;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    VkResult result = VK_SUCCESS;
    int32_t  planeIdx = 0;

    __VK_MEMZERO(hwImgDesc, sizeof(hwImgDesc));

    if (dstRes->isImage)
    {
        static __vkImageView tmpImgView;
        static __vkFormatInfo tmpFormatInfo;
        __vkImage *pDstImg = dstRes->u.img.pImage;
        __vkImageLevel *pImgLevel = &pDstImg->pImgLevels[dstRes->u.img.subRes.mipLevel];
        static VkExtent3D userSize;
        int32_t bpp;

        params->dstOffset = dstRes->u.img.offset;
        params->dstExtent = dstRes->u.img.extent;

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

        planeIdx = __vk_GetPlaneIndex(dstRes->u.img.subRes.aspectMask);
        bpp  = planeIdx > -1 ? __vk_GetPlaneFormatInfo(pDstImg, dstRes->u.img.subRes.aspectMask)->bitsPerBlock : pDstImg->formatInfo.bitsPerBlock;
        planeIdx = planeIdx < 0 ? 0 : planeIdx;

        __VK_MEMZERO(&tmpFormatInfo, sizeof(tmpFormatInfo));
        switch (bpp)
        {
        case 128:
            __VK_MEMCOPY(&tmpFormatInfo, __vk_GetVkFormatInfo(VK_FORMAT_R16G16B16A16_UINT), sizeof(tmpFormatInfo));
            break;
        case 64:
            if (pSrcImg->formatInfo.partCount == 2)
            {
                __VK_MEMCOPY(&tmpFormatInfo, __vk_GetVkFormatInfo(VK_FORMAT_R8G8B8A8_UINT), sizeof(tmpFormatInfo));
            }
            break;
        default:
            /* Not support other bpp currently */
            __VK_ASSERT(0);
            break;
        }
        tmpFormatInfo.partCount = pDstImg->formatInfo.partCount;

        tmpImgView.formatInfo = &tmpFormatInfo;
        imgView = &tmpImgView;

        if (imgView && pDstImg->formatInfo.compressed)
        {
            VkExtent2D rect = pDstImg->formatInfo.blockSize;
            VkBool32 srcCompressed = srcRes->isImage && srcRes->u.img.pImage->formatInfo.compressed;
            uint32_t width = gcmALIGN_NP2(pImgLevel->requestW, rect.width) / rect.width;
            uint32_t height = gcmALIGN_NP2(pImgLevel->requestH, rect.height) / rect.height;

            /*ditOffset dstExtent is the area to copy to, width/height is the image Size*/
            params->dstOffset.x = gcmALIGN_NP2(params->dstOffset.x - rect.width + 1, rect.width) / rect.width;
            params->dstOffset.y = gcmALIGN_NP2(params->dstOffset.y - rect.height + 1, rect.height) / rect.height;
            if (srcCompressed)
            {
                params->dstExtent.width = gcmALIGN_NP2(params->dstExtent.width, rect.width)     / rect.width;
                params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
            }

            if (pDstImg->formatInfo.bitsPerBlock == 128)
            {
                width *= 2;
            }

            userSize.width = width;
            userSize.height = height;
            userSize.depth = dstRes->u.img.extent.depth;

            pUserSize = &userSize;
        }
        else
        {
            /* For 1 part images, faked it as double widthed with half bpp */
            if (pDstImg->formatInfo.partCount == 1)
            {
                userSize.width = pImgLevel->requestW * pSrcImg->formatInfo.partCount;
                userSize.height = pImgLevel->requestH;
                userSize.depth = dstRes->u.img.extent.depth;

                pUserSize = &userSize;
            }
            if (srcRes->isImage && srcRes->u.img.pImage->formatInfo.compressed)
            {
                VkExtent2D rect = srcRes->u.img.pImage->formatInfo.blockSize;

                params->dstExtent.width  = gcmALIGN_NP2(params->dstExtent.width, rect.width ) / rect.width;
                params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
            }
        }
    }
    else
    {
        static __vkBufferView tmpBufView;
        static VkExtent3D dstSize;

        params->dstOffset.x = params->dstOffset.y = params->dstOffset.z = 0;
        params->dstExtent = srcRes->u.img.extent;
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
                tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R16G16B16A16_UINT);
            }
            break;
        case 64:
            /* Dst must be 2 part faked format here. */
            if (pSrcImg->formatInfo.partCount == 2)
            {
                tmpBufView.formatInfo = *__vk_GetVkFormatInfo(VK_FORMAT_R8G8B8A8_UINT);
            }
            break;
        default:
            break;
        }

        bufView = &tmpBufView;

        if (pSrcImg->formatInfo.compressed && !pSrcImg->ycbcrFormatInfo.bYUVFormat)
        {
            VkExtent2D rect = pSrcImg->formatInfo.blockSize;
            __vkImageLevel *pImgLevel = &pSrcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];

            uint32_t width = gcmALIGN_NP2(params->dstExtent.width, rect.width) / rect.width;
            uint32_t height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;

            params->dstOffset.x = gcmALIGN_NP2(params->dstOffset.x - rect.width + 1, rect.width) / rect.width;
            params->dstOffset.y = gcmALIGN_NP2(params->dstOffset.y - rect.height + 1, rect.height)/ rect.height;
            params->dstExtent.width = gcmALIGN_NP2(params->dstExtent.width, rect.width) / rect.width;
            params->dstExtent.height = gcmALIGN_NP2(params->dstExtent.height, rect.height) / rect.height;
            params->dstExtent.depth = pImgLevel->requestD;

            if (pSrcImg->formatInfo.bitsPerBlock == 128)
            {
                params->dstOffset.x *= 2;
                params->dstExtent.width *= 2;
                width *= 2;
            }

            dstSize.width = width;
            dstSize.height = height;
            dstSize.depth = srcRes->u.img.extent.depth;

            pUserSize = &dstSize;
        }
        else
        {
            dstSize.width = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
            dstSize.height = dstRes->u.buf.imgHeight != 0 ? dstRes->u.buf.imgHeight : srcRes->u.img.extent.height;
            dstSize.depth = srcRes->u.img.extent.depth;

            if ((pSrcImg->createInfo.flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) &&
                pSrcImg->formatInfo.bitsPerBlock == 128)
            {
                __vkImageLevel *pImgLevel = &pSrcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];

                params->dstExtent.width = pImgLevel->allocedW;
                params->dstExtent.height = pImgLevel->allocedH;
                dstSize.width = pImgLevel->allocedW;
                dstSize.height = pImgLevel->allocedH;
                dstSize.depth = srcRes->u.img.extent.depth;
            }
            /* Double width while halve bpp */
            dstSize.width *= pSrcImg->formatInfo.partCount;

            pUserSize = &dstSize;
        }
    }
    __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgView, bufView, pUserSize, hwImgDesc));

    hwMapping = &blitProg->dstImgEntry[0]->hwMappings[VSC_SHADER_STAGE_CS];
    __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
    __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);
    hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                   + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                   + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
    __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 0].imageInfo);

    if (blitProg->kind == HALTI5_BLIT_BUF_TO_2LAYERS_IMG)
    {
        hwMapping = &blitProg->dstImgEntry[1]->hwMappings[VSC_SHADER_STAGE_CS];
        __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
        __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

        hwConstRegAddr = (pHints->hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2)
                       + (hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.constReg.hwRegNo * 4)
                       + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
        __vkCmdLoadBatchHWStates(states, hwConstRegAddr, VK_FALSE, 4, hwImgDesc[planeIdx * __VK_MAX_PARTS + 1].imageInfo);
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
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_F0 */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_F1 */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 2, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_I0 */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 3, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_I1 */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 4, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_U0 */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 5, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex2D_U1 */

            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 6, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_F */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 7, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_I */
            {{VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 0, 8, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcTex3D_U */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 9, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_F0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 10, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_F1 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 11, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_I0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 12, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_I1 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 13, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_U0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 14, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg2D_U1 */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 15, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_F */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 16, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_I */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 17, 1}, VSC_SHADER_STAGE_BIT_CS}, /* dstImg3D_U */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 18, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_U0 */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 19, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_U1 */

            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 20, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_F  */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 21, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_I  */
            {{VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE, 0, 22, 1}, VSC_SHADER_STAGE_BIT_CS}, /* srcImg2D_U  */

            {{VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER, 0, 24, 1}, VSC_SHADER_STAGE_BIT_CS}, /* uniform PARAMS */
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
            {"blit2D_unorm_float", { 0, 1}, { 9, 10}, 24},
            {"blit2D_unorm_float_hwdoublerounding", { 0, 1}, { 9, 10}, 24},
            {"blit2D_unorm_to_pack", { 0, 1}, {13, ~0u}, 24},
            {"blit2D_sint", { 2, 3}, {11, 12}, 24},
            {"blit2D_uint", { 4, 5}, {13, 14}, 24},
            {"blit2D_uint_to_a2b10g10r10_pack", { 4, 5}, {13, ~0u}, 24},

            {"blit2D_sfloat_downsample", { 0, 1}, { 9, 10}, 24},
            {"blit2D_sint_downsample", { 2, 3}, {11, 12}, 24},
            {"blit2D_uint_downsample", { 4, 5}, {13, 14}, 24},

            {"blit3D_unorm_float", { 6, ~0u}, {15, ~0u}, 24},
            {"blit3D_unorm_float_hwdoublerounding", { 6, ~0u}, {15, ~0u}, 24},
            {"blit3D_unorm_to_pack", { 6, ~0u}, {17, ~0u}, 24},
            {"blit3D_sint", { 7, ~0u}, {16, ~0u}, 24},
            {"blit3D_uint", { 8, ~0u}, {17, ~0u}, 24},
            {"blit3D_uint_to_a2b10g10r10_pack", { 8, ~0u}, {17, ~0u}, 24},

            {"copy_2layers_img_to_buf", {18, 19}, {13, 14}, 24},
            {"copy_buf_to_2layers_img", {18, 19}, {13, 14}, 24},
            {"copy_2D_unorm_float", {20, ~0u}, {9, ~0u}, 24},
            {"copy_2D_unorm_to_a4r4g4b4", {22, ~0u}, {13, ~0u}, 24},
            {"copy_2D_sint", {21, ~0u}, {11, ~0u}, 24},
            {"copy_2D_uint", {22, ~0u}, {13, ~0u}, 24},
            {"copy_oq_query_pool", {22, ~0u}, {13, ~0u}, 24},

            {"clear_2D_uint", {~0u,~0u}, {13, ~0u}, 24},
            {"clear_to_2layers_img", {~0u,~0u}, {13, 14}, 24},
            {"blit_2D_buffer", {21, ~0u}, {11, ~0u}, 24},
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
        decodeInfo.specFlag = SPV_SPECFLAG_ENTRYPOINT | SPV_SPECFLAG_SPECIFIED_LOCAL_SIZE | SPV_SPECFLAG_INTERNAL_LIBRARY;
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
        vscLinkParams.cfg.ctx.appNameId = devCtx->pPhyDevice->pInst->patchID;
        vscLinkParams.cfg.ctx.isPatchLib = gcvFALSE;
        vscLinkParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS
                                 | VSC_COMPILER_FLAG_COMPILE_CODE_GEN
                                 | VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO
                                 | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
        if (devCtx->enabledFeatures.robustBufferAccess)
        {
            vscLinkParams.cfg.cFlags |= VSC_COMPILER_FLAG_NEED_OOB_CHECK;
        }
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
        case HALTI5_BLIT_COPY_2D_UNORM_TO_A4R4G4B4:
        case HALTI5_BLIT_COPY_2D_SINT:
        case HALTI5_BLIT_COPY_2D_UINT:
            blitProg->srcImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_src   = halti3_program_copy_src_img;
            blitProg->program_dst   = halti3_program_copy_dst_img;
            blitProg->program_const = halti5_program_blit_const;
            break;

        case HALTI5_BLIT_COPY_OQ_QUERY_POOL:
            blitProg->srcImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);

            blitProg->program_src   = halti5_program_copy_src_oq_query_pool;
            blitProg->program_dst   = halti5_program_copy_dst_oq_query_pool;
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
            blitProg->srcTexEntry[0] = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[0]);
            if (entryInfos[blitKind].srcBindings[1] != ~0u)
            {
                blitProg->srcTexEntry[1] = halti5_getCombinedTexSamplerEntry(progResSet, entryInfos[blitKind].srcBindings[1]);
            }

            blitProg->dstImgEntry[0] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[0]);
            if (entryInfos[blitKind].dstBindings[1] != ~0u)
            {
                blitProg->dstImgEntry[1] = halti5_getImageEntry(progResSet, entryInfos[blitKind].dstBindings[1]);
            }

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
    __vkCommandBuffer *cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    __vkComputeBlitParams *params,
    VkFilter filter
    )
{
    static const VkComponentMapping sComponents_gbar =
    {
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
        VK_COMPONENT_SWIZZLE_R
    };

    static const VkComponentMapping sComponents_argb =
    {
        VK_COMPONENT_SWIZZLE_A,
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B
    };

    uint32_t kind = HALTI5_BLIT_NUM;
    uint32_t srcParts, dstParts;
    uint32_t srcFormat, dstFormat;
    uint32_t srcCategory, dstCategory;
    uint32_t srcBitsPerPixel = 0, dstBitsPerPixel = 0;
    VkBool32 srcMsaa, dstMsaa;
    VkImageType srcType, dstType;
    VkImageAspectFlags srcAspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    __vkImage *pSrcImg, *pDstImg;
    const __vkFormatInfo *fmtInfo = gcvNULL;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    VkBool32 bAlign = VK_TRUE;

    if (!srcRes->isImage && !dstRes->isImage)
    {
        return HALTI3_BLIT_BUFFER_2D;
    }

    if (srcRes->isImage)
    {
        pSrcImg = srcRes->u.img.pImage;
        fmtInfo = __vk_GetPlaneFormatInfo(pSrcImg, srcRes->u.img.subRes.aspectMask);
        fmtInfo = fmtInfo == VK_NULL_HANDLE ? &pSrcImg->formatInfo : fmtInfo;

        srcFormat = fmtInfo->residentImgFormat;
        srcParts = fmtInfo->partCount;
        srcCategory = fmtInfo->category;
        srcType = pSrcImg->createInfo.imageType;
        srcBitsPerPixel = fmtInfo->bitsPerBlock;
        srcMsaa = (pSrcImg->sampleInfo.product > 1);
        srcAspect = srcRes->u.img.subRes.aspectMask;

        if ((pSrcImg->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
            ((pSrcImg->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM) ||
            (pSrcImg->createInfo.format == VK_FORMAT_R8G8B8A8_SRGB) ||
            (pSrcImg->createInfo.format == VK_FORMAT_A8B8G8R8_UNORM_PACK32) ||
            (pSrcImg->createInfo.format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)))
        {
            srcFormat = pSrcImg->createInfo.format;
        }

        /* for compressed image, when do copy to buffer, it is R16G16B16A16 format, it must be tile align */
        if (pSrcImg->formatInfo.compressed && !pSrcImg->ycbcrFormatInfo.bYUVFormat)
        {
            VkExtent2D rect = pSrcImg->formatInfo.blockSize;
            uint32_t   width  = pSrcImg->createInfo.extent.width;
            uint32_t   height = pSrcImg->createInfo.extent.height;


            width  = gcmALIGN_NP2(width, rect.width ) / rect.width;
            height = gcmALIGN_NP2(height, rect.height) / rect.height;

            bAlign = ((height | width) & 0x3) == 0; /* 4 * 4 tile align */
        }
    }
    else
    {
        pDstImg = dstRes->u.img.pImage;

        fmtInfo = __vk_GetPlaneFormatInfo(pDstImg, dstRes->u.img.subRes.aspectMask);
        srcFormat = fmtInfo == VK_NULL_HANDLE ? pDstImg->createInfo.format : fmtInfo->residentImgFormat;
        fmtInfo = __vk_GetVkFormatInfo((VkFormat) srcFormat);
        srcParts = fmtInfo->partCount;
        srcCategory = fmtInfo->category;
        srcType = pDstImg->createInfo.imageType;
        srcMsaa = VK_FALSE;
        srcAspect = dstRes->u.img.subRes.aspectMask;
    }

    /* Change float/unorm/snorm to be same categroy. */
    switch (srcCategory)
    {
    case __VK_FMT_CATEGORY_SNORM:
    case __VK_FMT_CATEGORY_UFLOAT:
    case __VK_FMT_CATEGORY_SFLOAT:
    case __VK_FMT_CATEGORY_SRGB:
        srcCategory = __VK_FMT_CATEGORY_UNORM;
        break;
    default:
        break;
    }

    if (dstRes->isImage)
    {
        pDstImg = dstRes->u.img.pImage;
        fmtInfo = __vk_GetPlaneFormatInfo(pDstImg, dstRes->u.img.subRes.aspectMask);
        dstFormat = fmtInfo == VK_NULL_HANDLE ? pDstImg->formatInfo.residentImgFormat : fmtInfo->residentImgFormat;
        dstParts = pDstImg->formatInfo.partCount;
        dstCategory = pDstImg->formatInfo.category;
        dstBitsPerPixel = pDstImg->formatInfo.bitsPerBlock;
        dstMsaa = (pDstImg->sampleInfo.product > 1);
        dstType = pDstImg->createInfo.imageType;

        if ((pDstImg->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
            ((pDstImg->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM) ||
            (pDstImg->createInfo.format == VK_FORMAT_R8G8B8A8_SRGB) ||
            (pDstImg->createInfo.format == VK_FORMAT_A8B8G8R8_UNORM_PACK32) ||
            (pDstImg->createInfo.format == VK_FORMAT_A8B8G8R8_SRGB_PACK32)))
        {
            dstFormat = pDstImg->createInfo.format;
        }
    }
    else
    {
        pSrcImg = srcRes->u.img.pImage;
        fmtInfo = __vk_GetPlaneFormatInfo(pSrcImg, srcRes->u.img.subRes.aspectMask);

        dstFormat = fmtInfo == VK_NULL_HANDLE ? pSrcImg->createInfo.format : fmtInfo->residentImgFormat;
        fmtInfo = __vk_GetVkFormatInfo((VkFormat) dstFormat);
        dstParts = fmtInfo->partCount;
        dstCategory = fmtInfo->category;
        dstMsaa = VK_FALSE;
        dstType = pSrcImg->createInfo.imageType;
    }
    params->dstSRGB = (dstCategory == __VK_FMT_CATEGORY_SRGB);

    /* Change float/unorm/snorm to be same categroy. */
    switch (dstCategory)
    {
    case __VK_FMT_CATEGORY_SNORM:
    case __VK_FMT_CATEGORY_UFLOAT:
    case __VK_FMT_CATEGORY_SFLOAT:
    case __VK_FMT_CATEGORY_SRGB:
        dstCategory = __VK_FMT_CATEGORY_UNORM;
        break;
    default:
        break;
    }

    __VK_ASSERT(srcAspect != VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM);
    if ((srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT || srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT))
    {
        /* For depth or stencil only aspect: cannot support combined formats now. */
        switch (dstFormat)
        {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit does NOT support the combination now: aspect=0x%x dstFormat=%d\n",
                             srcAspect, dstFormat);
            __VK_ASSERT(VK_FALSE);
            return kind;
            break;
        default:
            break;
        }
    }

    /* If rawCopy or src/dst are of same format when NEAREST filtering,
    ** fake it for better tex/image supported.
    */
    if ((filter == VK_FILTER_NEAREST) &&
        ((srcFormat == dstFormat && srcParts == 1 && dstParts == 1 && !srcMsaa && !dstMsaa) ||
         (params->rawCopy)
        )
       )
    {
        fmtInfo = __vk_GetVkFormatInfo((VkFormat) dstFormat);

        /* Int type do not have data conversion */
        switch (fmtInfo->bitsPerBlock)
        {
        case 8:
            srcFormat = dstFormat = VK_FORMAT_R8_UINT;
            break;
        case 16:
            srcFormat = dstFormat = VK_FORMAT_R16_UINT;
            break;
        case 32:
            if (srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT)
            {
                if (dstFormat == __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32)
                {
                    params->channelWriteMask = __CHANNEL_MASK_R;
                }
                else if (dstFormat == VK_FORMAT_D24_UNORM_S8_UINT)
                {
                    params->channelWriteMask = __CHANNEL_MASK_A;
                }
            }
            else if (srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT)
            {
                if (dstFormat == __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32)
                {
                    params->channelWriteMask = __CHANNEL_MASK_GBA;
                }
                else if (dstFormat == VK_FORMAT_D24_UNORM_S8_UINT)
                {
                    params->channelWriteMask = __CHANNEL_MASK_RGB;
                }
            }
            srcFormat = dstFormat = VK_FORMAT_R16G16_UINT;
            break;
        case 64:
        case 128:
            srcFormat = dstFormat = VK_FORMAT_R16G16B16A16_UINT;
            break;
        default:
            __VK_ASSERT(0);
        }

        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        params->dstSRGB = VK_FALSE;
        params->rawCopy = VK_TRUE;
        if (fmtInfo->bitsPerBlock == 128)
        {
            params->fmtConvert = VK_TRUE;
        }
    }

    switch (dstFormat)
    {
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        dstFormat = VK_FORMAT_R8G8B8A8_UINT;
        switch (srcFormat)
        {
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = &sComponents_gbar;
            params->channelWriteMask = __CHANNEL_MASK_RGB;
            break;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = gcvNULL;
            params->channelWriteMask = __CHANNEL_MASK_RGB;
        default:
            __VK_ASSERT(0);
            break;
        }
        break;

    case VK_FORMAT_D24_UNORM_S8_UINT:
        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        dstFormat = VK_FORMAT_R8G8B8A8_UINT;
        switch (srcFormat)
        {
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = &sComponents_gbar;
            params->channelWriteMask = __CHANNEL_MASK_GBA;
            break;

        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = &sComponents_gbar;
            if (srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT)
            {
                dstFormat = dstRes->isImage ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8_UINT;
                params->txSwizzles = gcvNULL;
                params->channelWriteMask = dstRes->isImage ? __CHANNEL_MASK_A : __CHANNEL_MASK_RGBA;
            }
            else if (srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT)
            {
                params->channelWriteMask = __CHANNEL_MASK_RGB;
            }
            else
            {
                __VK_ASSERT(srcAspect == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
                params->channelWriteMask = __CHANNEL_MASK_RGBA;
            }
            break;

        case VK_FORMAT_X8_D24_UNORM_PACK32:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = gcvNULL;
            params->channelWriteMask = __CHANNEL_MASK_RGB;
            break;

        case VK_FORMAT_S8_UINT:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT);
            srcFormat = VK_FORMAT_R8_UINT;
            params->channelWriteMask = __CHANNEL_MASK_A;
            params->txSwizzles = &sComponents_gbar;

        default:
            __VK_ASSERT(0);
            break;
        }
        break;

    case __VK_FORMAT_D24_UNORM_X8_PACKED32:
        __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        dstFormat = VK_FORMAT_R8G8B8A8_UINT;
        switch (srcFormat)
        {
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = &sComponents_argb;
            params->channelWriteMask = __CHANNEL_MASK_GBA;
            break;
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = gcvNULL;
            params->channelWriteMask = __CHANNEL_MASK_GBA;
            break;
        default:
            __VK_ASSERT(0);
            break;
        }
        break;

    case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        dstFormat = VK_FORMAT_R8G8B8A8_UINT;
        switch (srcFormat)
        {
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = gcvNULL;
            params->channelWriteMask = __CHANNEL_MASK_GBA;
            break;

        case VK_FORMAT_D24_UNORM_S8_UINT:
            if (srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT)
            {
                srcFormat = srcRes->isImage ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8_UINT;
                params->txSwizzles = srcRes->isImage ? &sComponents_argb : gcvNULL;
                params->channelWriteMask = __CHANNEL_MASK_R;
            }
            else if (srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT)
            {
                srcFormat = VK_FORMAT_R8G8B8A8_UINT;
                params->txSwizzles = &sComponents_argb;
                params->channelWriteMask = __CHANNEL_MASK_GBA;
            }
            else
            {
                __VK_ASSERT(srcAspect == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
                srcFormat = VK_FORMAT_R8G8B8A8_UINT;
                params->txSwizzles = &sComponents_argb;
                params->channelWriteMask = __CHANNEL_MASK_RGBA;
            }
            break;

        case VK_FORMAT_X8_D24_UNORM_PACK32:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_DEPTH_BIT);
            srcFormat = VK_FORMAT_R8G8B8A8_UINT;
            params->txSwizzles = &sComponents_argb;
            params->channelWriteMask = __CHANNEL_MASK_GBA;
            break;

        case VK_FORMAT_S8_UINT:
            __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT);
            srcFormat = VK_FORMAT_R8_UINT;
            params->txSwizzles = gcvNULL;
            params->channelWriteMask = __CHANNEL_MASK_R;

        default:
            __VK_ASSERT(0);
            break;
        }
        break;

    case VK_FORMAT_S8_UINT:
        __VK_ASSERT(srcAspect == VK_IMAGE_ASPECT_STENCIL_BIT);
        srcCategory = dstCategory = __VK_FMT_CATEGORY_UINT;
        dstFormat = VK_FORMAT_R8_UINT;
        params->channelWriteMask = __CHANNEL_MASK_RGBA;
        switch (srcFormat)
        {
        case VK_FORMAT_D24_UNORM_S8_UINT:
            srcFormat = srcRes->isImage ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8_UINT;
            params->txSwizzles = srcRes->isImage ? &sComponents_argb : gcvNULL;
            break;

        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            dstFormat = dstRes->isImage ? VK_FORMAT_R8G8B8A8_UINT : VK_FORMAT_R8_UINT;
            params->txSwizzles = gcvNULL;
            break;
        default:
            __VK_ASSERT(0);
            break;
        }
        break;

    default:
        break;
    }

    params->srcFormat = srcFormat;
    params->dstFormat = dstFormat;
    params->srcParts = srcParts;
    params->dstParts = dstParts;

    if (srcCategory != dstCategory)
    {
        __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit unsupported categories: srcCategory=%u, dstCategory=%u\n",
                         srcCategory, dstCategory);
        __VK_ASSERT(VK_FALSE);
    }
    else if (srcRes->isImage && dstRes->isImage &&
             srcMsaa != dstMsaa)
    {
        __VK_ASSERT(srcBitsPerPixel == dstBitsPerPixel);

        switch (dstCategory)
        {
        case __VK_FMT_CATEGORY_SINT:
            kind = HALTI5_BLIT_2D_SINT_DOWNSAMPLE;
            break;
        case __VK_FMT_CATEGORY_UINT:
            kind = HALTI5_BLIT_2D_UINT_DOWNSAMPLE;
            break;
        default:
            kind = HALTI5_BLIT_2D_SFLOAT_DOWNSAMPLE;
            break;
        }

        if (srcParts == 2)
        {
            switch (srcFormat)
            {
                case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
                case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
                case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
                    params->packFormat = __PACK_FORMAT_R32G32B32A32;
                    break;
                case __VK_FORMAT_R16G16B16A16_SFLOAT_2_R16G16_SFLOAT:
                case __VK_FORMAT_R16G16B16A16_SINT_2_R16G16_SINT:
                case __VK_FORMAT_R16G16B16A16_UINT_2_R16G16_UINT:
                    params->packFormat = __PACK_FORMAT_R16G16B16A16;
                    break;
                case __VK_FORMAT_R32G32_SFLOAT_2_R32_SFLOAT:
                case __VK_FORMAT_R32G32_SINT_2_R32_SINT:
                case __VK_FORMAT_R32G32_UINT_2_R32_UINT:
                    params->packFormat = __PACK_FORMAT_R32G32;
                    break;
                default:
                    __VK_ASSERT(!"invalid downsample format!");
                    break;
            }
        }
    }
    else if (srcParts != dstParts)
    {
        if (srcParts == 2 && dstParts == 1)
        {
            /*if dst resource is buffer or image with 128 bpp native format which is a temp image created in __vk_CmdCopyImage,
            then go blit buffer to 2layer iamge path*/
            if (!dstRes->isImage || (srcBitsPerPixel == dstBitsPerPixel))
            {
                kind = HALTI5_BLIT_2LAYERS_IMG_TO_BUF;
            }
        }
        else if (srcParts == 1 && dstParts == 2)
        {
            /*if src resource is buffer or image with 128 bpp native format which is a temp image created in __vk_CmdCopyImage,
            then go blit buffer to 2layer iamge path*/
            if ((srcBitsPerPixel == dstBitsPerPixel) || !srcRes->isImage)
            {
                kind = HALTI5_BLIT_BUF_TO_2LAYERS_IMG;
            }
        }
        else
        {
            __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "Compute blit unsupported fake formats: srcFormat=%u, dstFormat=%u\n",
                             srcFormat, dstFormat);
            __VK_ASSERT(VK_FALSE);
        }
    }
    else if (!srcRes->isImage)
    {
        switch (dstFormat)
        {
        case __VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            if (srcFormat == VK_FORMAT_B4G4R4A4_UNORM_PACK16)
            {
                kind = HALTI5_BLIT_COPY_2D_UNORM_TO_A4R4G4B4;
                params->srcFormat = VK_FORMAT_R16_UINT;
                params->dstFormat = VK_FORMAT_R16_UINT;
            }
            break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            kind = HALTI5_BLIT_COPY_2D_UINT;
            params->srcFormat = VK_FORMAT_R16_UINT;
            params->dstFormat = VK_FORMAT_R16_UINT;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            kind = HALTI5_BLIT_COPY_2D_UINT;
            params->srcFormat = VK_FORMAT_R32_UINT;
            params->dstFormat = VK_FORMAT_R32_UINT;
            break;
        default:
            switch (dstCategory)
            {
            case __VK_FMT_CATEGORY_SINT:
                kind = HALTI5_BLIT_COPY_2D_SINT;
                break;
            case __VK_FMT_CATEGORY_UINT:
                kind = HALTI5_BLIT_COPY_2D_UINT;
                break;
            default:
                kind = HALTI5_BLIT_COPY_2D_UNORM_FLOAT;
                break;
            }
            break;
        }
    }

    if (kind == HALTI5_BLIT_NUM)
    {
        switch (dstFormat)
        {
        case __VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_TO_PACK16;
            params->dstFormat = VK_FORMAT_R16_UINT;
            params->packFormat = __PACK_FORMAT_A4R4G4B4;
            break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_TO_PACK16;
            params->dstFormat = VK_FORMAT_R16_UINT;
            params->packFormat = __PACK_FORMAT_B4G4R4A4;
            break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_TO_PACK16;
            params->dstFormat = VK_FORMAT_R16_UINT;
            params->packFormat = __PACK_FORMAT_R5G6B5;
            break;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            kind = HALTI5_BLIT_2D_UNORM_TO_PACK16;
            params->dstFormat = VK_FORMAT_R16_UINT;
            params->packFormat = __PACK_FORMAT_A1R5G5B5;
            break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            kind = HALTI5_BLIT_2D_UNORM_TO_PACK16;
            params->dstFormat = VK_FORMAT_R32_UINT;
            params->packFormat = __PACK_FORMAT_A2B10G10R10;
            break;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            kind = HALTI5_BLIT_2D_UINT_TO_A2B10G10R10_PACK;
            params->dstFormat = VK_FORMAT_R32_UINT;
            break;
        default:
            switch (dstCategory)
            {
            case __VK_FMT_CATEGORY_SINT:
                if (bAlign)
                {
                    kind = HALTI5_BLIT_2D_SINT;
                }
                else
                {
                    kind = HALTI5_BLIT_COPY_2D_SINT;
                }
                break;
            case __VK_FMT_CATEGORY_UINT:
                if (bAlign)
                {
                    kind = HALTI5_BLIT_2D_UINT;
                }
                else
                {
                    kind = HALTI5_BLIT_COPY_2D_UINT;
                }
                break;
            default:
                {
                    if (devCtx->database->TX_8bit_UVFrac && !devCtx->database->TX_8bit_UVFrac_ROUNDING_FIX)
                    {
                        /* Tune textureCoord for the chip which will triger HW double rounding issue. */
                        kind = HALTI5_BLIT_2D_UNORM_FLOAT_HWDOUBLEROUNDING;
                    }
                    else
                    {
                        kind = HALTI5_BLIT_2D_UNORM_FLOAT;
                    }
                }
                break;
            }
            break;
        }

        /* If either type was of 3D, choose corresponding 3D variant except rawCopy. */
        if (!params->rawCopy && (srcType == VK_IMAGE_TYPE_3D || dstType == VK_IMAGE_TYPE_3D))
        {
            kind += (HALTI5_BLIT_3D_UNORM_FLOAT - HALTI5_BLIT_2D_UNORM_FLOAT);
        }
    }
    return kind;
}

__VK_INLINE VkResult halti5_triggerComputeShader(
    __vkDevContext *devCtx,
    uint32_t **states,
    halti5_vscprogram_blit *blitProg,
    __vkComputeBlitParams *params
    )
{
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;
    /*
    ** Program the shader
    */
    {
        const gcsFEATURE_DATABASE *database = devCtx->database;
        uint32_t reservedPages = database->SH_SNAP2PAGE_MAXPAGES_FIX ? 0 : 1;
        uint32_t i;

        __VK_MEMCOPY(*states, blitProg->hwStates.pStateBuffer, blitProg->hwStates.stateBufferSize);
        *states += (blitProg->hwStates.stateBufferSize / sizeof(uint32_t));

        __vkCmdLoadSingleHWState(states, 0x0402, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, 0x0403, VK_FALSE, pHints->fsMaxTemp);

        __vkCmdLoadSingleHWState(states, 0x0404, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, 0x0228, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x52C6, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x52C7, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x52CD, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
            );
        __vkCmdLoadSingleHWState(states, 0x52CD, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)))
            );
        __vkCmdLoadSingleHWState(states,0x5286, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states,0x5286, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20)))
            );
        __vkCmdLoadSingleHWState(states, 0x0440, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x0450, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x0450, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21)))
            );
        __vkCmdLoadSingleHWState(states, 0x02AA, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x028C, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, 0x0E07, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x040C, VK_FALSE, 0);

        __vkCmdLoadSingleHWState(states, 0x0E22, VK_FALSE,
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

        __vkCmdLoadSingleHWState(states, 0x0E06, VK_FALSE,
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
                __vkCmdLoadSingleHWState(states, 0x0412, VK_FALSE, pHints->fsICachePrefetch[i]);
            }
        }
    }

    /*
    ** Program the compute dispatch
    */
    {
        halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
        uint32_t threadAllocation = gcmCEIL((gctFLOAT)(pHints->workGrpSize.x * pHints->workGrpSize.y * pHints->workGrpSize.z)
                                  / (devCtx->database->NumShaderCores * 4));
        uint32_t localMemSizeInByte = 0;
        uint32_t groupNumberPerCluster = 0;
        uint32_t workGroupCountX, workGroupCountY, workGroupCountZ;

        workGroupCountX = gcmCEIL((float)params->dstExtent.width  / VIV_LOCAL_X);
        workGroupCountY = gcmCEIL((float)params->dstExtent.height / VIV_LOCAL_Y);
        workGroupCountZ = gcmCEIL((float)params->dstExtent.depth  / VIV_LOCAL_Z);

        __vkCmdLoadSingleHWState(states, 0x0240, VK_FALSE,
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
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))));

        if (devCtx->database->PSCS_THROTTLE)
        {
            localMemSizeInByte = gcmCEIL((gctFLOAT)pHints->localMemSizeInByte  / 16.0);
        }
        if (chipModule->clusterInfo.clusterAliveMask > 0)
        {
            uint32_t allocationSize = pHints->workGrpSize.x * pHints->workGrpSize.y * pHints->workGrpSize.z;

            groupNumberPerCluster = (devCtx->database->NumShaderCores * 4
                * (pHints->fsIsDual16 ? 2 : 1)) / allocationSize;
            groupNumberPerCluster = groupNumberPerCluster > 1 ? groupNumberPerCluster -1 : 0;
            groupNumberPerCluster = __VK_MIN(groupNumberPerCluster, 63);
        }

        __vkCmdLoadSingleHWState(states, 0x0249, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (localMemSizeInByte) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
             | (devCtx->database->SH_MULTI_WG_PACK ?
                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) ((pHints->threadGroupSync ?
 0x0 : 0x1)) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) : 0)
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (groupNumberPerCluster) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20))));

        __vkCmdLoadSingleHWState(states, 0x0247, VK_FALSE, threadAllocation);

        __vkCmdLoadSingleHWState(states, 0x024B, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x024D, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x024F, VK_FALSE,
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
        __vkCmdLoadSingleHWState(states, 0x0250, VK_FALSE,
                workGroupCountX - 1
            );
        __vkCmdLoadSingleHWState(states, 0x0251, VK_FALSE,
                workGroupCountY - 1
            );
        __vkCmdLoadSingleHWState(states, 0x0252, VK_FALSE,
                workGroupCountZ - 1
            );
        __vkCmdLoadSingleHWState(states, 0x0253, VK_FALSE,
                pHints->workGrpSize.x - 1
            );
        __vkCmdLoadSingleHWState(states, 0x0254, VK_FALSE,
                pHints->workGrpSize.y - 1
            );
        __vkCmdLoadSingleHWState(states, 0x0255, VK_FALSE,
                pHints->workGrpSize.z - 1
            );

        __vkCmdLoadSingleHWState(states, 0x0248, VK_FALSE, 0xBADABEEB);
    }

    return VK_SUCCESS;
}

static VkResult halti5_addAllocationForCompute(
    uint32_t **states,
    halti5_vscprogram_blit *blitProg
    )
{
    VkResult result = VK_SUCCESS;
    gcsHINT_PTR pHints = &blitProg->hwStates.hints;

    if (pHints->unifiedStatus.constCount)
    {
        /* now, always copy.*/
        __vkCmdLoadSingleHWState(states, 0x042B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:0) - (0 ?
 8:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:0) - (0 ?
 8:0) + 1))))))) << (0 ?
 8:0))) | (((gctUINT32) ((gctUINT32) (pHints->unifiedStatus.constCount) & ((gctUINT32) ((((1 ?
 8:0) - (0 ?
 8:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:0) - (0 ? 8:0) + 1))))))) << (0 ? 8:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            );
    }

    if (pHints->unifiedStatus.samplerCount)
    {
        __vkCmdLoadSingleHWState(states, 0x042C, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->unifiedStatus.samplerCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            );

        __vkCmdLoadSingleHWState(states, 0x042D, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (pHints->unifiedStatus.samplerCount) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (gcvTRUE) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
            );
    }

    return result;
}

VkResult halti5_computeBlit(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy,
    VkBool3D *reverse,
    VkFilter filter
    )
{
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    uint32_t *states;
    uint32_t *scatch = VK_NULL_HANDLE;
    uint32_t *scatchBegin = VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;
    __vkComputeBlitParams params;
    halti5_vscprogram_blit *blitProg = gcvNULL;
    uint32_t blitKind = HALTI5_BLIT_NUM;

    __VK_MEMZERO(&params, sizeof(params));

    params.rawCopy = rawCopy;
    if (reverse)
    {
        params.reverse = *reverse;
    }
    params.txSwizzles = gcvNULL;
    params.packFormat = __PACK_FORMAT_INVALID;
    params.channelWriteMask = __CHANNEL_MASK_RGBA;

    blitKind = halti5_detect_blit_kind(pCmdBuf, srcRes, dstRes, &params, filter);
    if (blitKind >= HALTI5_BLIT_NUM)
    {
        __VK_ONERROR(VK_NOT_READY);
    }

    /* TODO: implement blit3D_unorm_float_hwdoublerounding in gc_halti5_blit.comp */
    __VK_ASSERT(blitKind != HALTI5_BLIT_3D_UNORM_FLOAT_HWDOUBLEROUNDING);
    blitProg = halti5_GetComputeBlitProg(devCtx, blitKind);

    scatch = scatchBegin = &pCmdBuf->scratchCmdBuffer[pCmdBuf->curScrachBufIndex];

    if (blitKind >= HALTI5_BLIT_2D_SFLOAT_DOWNSAMPLE &&
        blitKind <= HALTI5_BLIT_2D_UINT_DOWNSAMPLE)
    {
        uint32_t stall = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

        __vkCmdLoadSingleHWState(&scatch, 0x0E02, VK_FALSE, stall);
        *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&scatch)++ = (stall);
;

    }

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


        __vkCmdLoadSingleHWState(&scatch, 0x0E80, VK_FALSE, 1);
    }

    /*when treat the src image as the texture, need flush the texture data cache*/
    if (blitKind <= HALTI5_BLIT_2LAYERS_IMG_TO_BUF)
    {
        halti5_flushCache((VkDevice)devCtx, &scatch, VK_NULL_HANDLE, HW_CACHE_TEXTURE_DATA);
    }

    if (devCtx->database->SMALLBATCH && devCtx->pPhyDevice->phyDevConfig.options.smallBatch)
    {
        __VK_ONERROR(halti5_addAllocationForCompute(&scatch, blitProg));
    }

    __VK_ONERROR(blitProg->program_src(pCmdBuf, blitProg, &scatch, srcRes, dstRes, filter, &params));
    __VK_ONERROR(blitProg->program_dst(pCmdBuf, blitProg, &scatch, srcRes, dstRes, &params));
    __VK_ONERROR(blitProg->program_const(pCmdBuf, blitProg, &scatch, &params));

    /* program compute shader.*/
    __VK_ONERROR(halti5_triggerComputeShader(devCtx, &scatch, blitProg, &params));

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

    if (params.flushTex)
    {
        __vkCmdLoadSingleHWState(&scatch, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))));
        __vkCmdLoadSingleHWState(&scatch, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))));

        if (!devCtx->database->REG_Halti5)
        {
            uint32_t stall = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                           | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

            __VK_ASSERT(!devCtx->database->REG_BltEngine);
            __vkCmdLoadSingleHWState(&scatch, 0x0E02, VK_FALSE, stall);
            *(*&scatch)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&scatch)++ = (stall);
;

        }
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
    uint32_t partIndex = 0;
    uint32_t blitKind = HALTI5_BLIT_NUM;
    __vkImage *pDstImg;
    uint32_t bitsPerPixel;
    uint32_t tmpClearValue[4] = { 0 };
    uint32_t bytemask[4] = { 0 };


    if (dstRes->isImage)
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


        __vkCmdLoadSingleHWState(&scatch, 0x0E80, VK_FALSE, 1);
    }

     __VK_MEMZERO(&params, sizeof(params));
     params.channelWriteMask = 0xf;

    if (dstRes->isImage)
    {
        pDstImg = dstRes->u.img.pImage;

        for (partIndex = 0; partIndex < pDstImg->pImgLevels[dstRes->u.img.subRes.mipLevel].partCount; partIndex++)
        {
            __VK_ONERROR(__vkComputeClearVal(
                pDstImg->formatInfo.residentImgFormat,
                dstRes->u.img.subRes.aspectMask,
                clearValue,
                partIndex,
                &tmpClearValue[2 * partIndex],
                VK_NULL_HANDLE,
                &bytemask[2 * partIndex]));
        }

        bitsPerPixel = pDstImg->formatInfo.bitsPerBlock;

         switch (bitsPerPixel)
         {
         case 64:
             params.uClearValue0[0] =  tmpClearValue[0] & 0x0000FFFF;
             params.uClearValue0[1] = (tmpClearValue[0] & 0xFFFF0000) >> 16;
             params.uClearValue0[2] =  tmpClearValue[1] & 0x0000FFFF;
             params.uClearValue0[3] = (tmpClearValue[1] & 0xFFFF0000) >> 16;
             break;

         case 128:
             params.uClearValue0[0] =  tmpClearValue[0] & 0x0000FFFF;
             params.uClearValue0[1] = (tmpClearValue[0] & 0xFFFF0000) >> 16;
             params.uClearValue0[2] =  tmpClearValue[1] & 0x0000FFFF;
             params.uClearValue0[3] = (tmpClearValue[1] & 0xFFFF0000) >> 16;

             params.uClearValue1[0] =  tmpClearValue[2] & 0x0000FFFF;
             params.uClearValue1[1] = (tmpClearValue[2] & 0xFFFF0000) >> 16;
             params.uClearValue1[2] =  tmpClearValue[3] & 0x0000FFFF;
             params.uClearValue1[3] = (tmpClearValue[3] & 0xFFFF0000) >> 16;
             break;

         default:
             params.uClearValue0[0] =  tmpClearValue[0] & 0x000000FF;
             params.uClearValue0[1] = (tmpClearValue[0] & 0x0000FF00) >> 8;
             params.uClearValue0[2] = (tmpClearValue[0] & 0x00FF0000) >> 16;
             params.uClearValue0[3] = (tmpClearValue[0] & 0xFF000000) >> 24;
         }

         if (dstRes->u.img.subRes.aspectMask &
             (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
             )
         {
             params.channelWriteMask = bytemask[0];
         }
    }
    else
    {
        params.uClearValue0[0] = clearValue->color.int32[0];
    }

    if (devCtx->database->SMALLBATCH && devCtx->pPhyDevice->phyDevConfig.options.smallBatch)
    {
        __VK_ONERROR(halti5_addAllocationForCompute(&scatch, blitProg));
    }

    __VK_ONERROR(halti5_flushCache((VkDevice)devCtx, &scatch, VK_NULL_HANDLE, HW_CACHE_ALL));

    __VK_ONERROR(blitProg->program_dst(pCmdBuf, blitProg, &scatch, VK_NULL_HANDLE, dstRes, &params));
    __VK_ONERROR(blitProg->program_const(pCmdBuf, blitProg, &scatch, &params));

    /* program compute shader.*/
    __VK_ONERROR(halti5_triggerComputeShader(devCtx, &scatch, blitProg, &params));

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

VkResult halti5_computeCopyOQQueryPool(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes
    )
{
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    uint32_t *scatch, *scatchBegin, *states;
    VkResult result = VK_SUCCESS;
    __vkComputeBlitParams params;
    halti5_vscprogram_blit *blitProg = gcvNULL;
    uint32_t blitKind = HALTI5_BLIT_NUM;

    /* set kind.*/
    blitKind = HALTI5_BLIT_COPY_OQ_QUERY_POOL;

    blitProg = halti5_GetComputeBlitProg(devCtx, blitKind);

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


        __vkCmdLoadSingleHWState(&scatch, 0x0E80, VK_FALSE, 1);
    }

    __VK_MEMZERO(&params, sizeof(params));

    if (devCtx->database->SMALLBATCH && devCtx->pPhyDevice->phyDevConfig.options.smallBatch)
    {
        __VK_ONERROR(halti5_addAllocationForCompute(&scatch, blitProg));
    }
    __VK_ONERROR(blitProg->program_src(pCmdBuf, blitProg, &scatch, srcRes, VK_NULL_HANDLE, VK_FILTER_NEAREST, &params));
    __VK_ONERROR(blitProg->program_dst(pCmdBuf, blitProg, &scatch, VK_NULL_HANDLE, dstRes, &params));
    __VK_ONERROR(blitProg->program_const(pCmdBuf, blitProg, &scatch, &params));

    /* program compute shader.*/
    __VK_ONERROR(halti5_triggerComputeShader(devCtx, &scatch, blitProg, &params));

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


