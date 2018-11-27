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

#define R 0
#define G 1
#define B 2
#define A 3

#define __VK_3DBLIT_LOCK(devCtx, pCmdBuffer, forceSGPU) \
    if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) \
    { \
        halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE); \
        if (forceSGPU) \
        { \
            __VK_ENABLE3DCORE(&(pCmdBuffer), gcvCORE_3D_0_MASK); \
        } \
    } \
    __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, \
        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK) \
        );

#define __VK_3DBLIT_UNLOCK(devCtx, pCmdBuffer, forceSGPU) \
    __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, \
        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK) \
        ); \
    if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) \
    { \
        if (forceSGPU) \
        { \
            __VK_ENABLE3DCORE(&(pCmdBuffer), gcvCORE_3D_ALL_MASK); \
        } \
        halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE); \
    }

const __vkFormatToHwTxFmtInfo* halti5_helper_convertHwTxInfo(
    __vkDevContext *devCtx,
    uint32_t vkFormat
    )
{
    uint32_t i;
    const __vkFormatToHwTxFmtInfo *hwTxFmtInfo = gcvNULL;

    static const __vkFormatToHwTxFmtInfo s_vkFormatToHwTxInfos[] =
    {
        {__VK_FORMAT_A4R4G4B4_UNFORM_PACK16, TX_FORMAT(0x05, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {__VK_FORMAT_R8_1_X8R8G8B8, TX_FORMAT(0x08, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},
        {__VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT, TX_FORMAT(0, 0x24, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {__VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT, TX_FORMAT(0, 0x24, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {__VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT, TX_FORMAT(0, 0x0B, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_S8_UINT, TX_FORMAT(0, 0x15, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},
        {__VK_FORMAT_D24_UNORM_S8_UINT_PACKED32, TX_FORMAT(0, 0x22, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},
        {__VK_FORMAT_D24_UNORM_X8_PACKED32, TX_FORMAT(0x11, 0, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},

        {VK_FORMAT_B4G4R4A4_UNORM_PACK16, TX_FORMAT(0x05, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_GREEN, SWZL_USE_RED, SWZL_USE_ALPHA, SWZL_USE_BLUE)},
        {VK_FORMAT_R5G6B5_UNORM_PACK16, TX_FORMAT(0x0B, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A1R5G5B5_UNORM_PACK16, TX_FORMAT(0x0C, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R5G5B5A1_UNORM_PACK16, TX_FORMAT(0x0C, 0, VK_TRUE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8_UNORM, TX_FORMAT(0, 0x21, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8_SNORM, TX_FORMAT(0, 0x0E, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8_UINT,TX_FORMAT(0, 0x15, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8_SINT, TX_FORMAT(0, 0x15, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8_UNORM, TX_FORMAT(0, 0x06, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8_SNORM, TX_FORMAT(0, 0x0F, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8_UINT, TX_FORMAT(0, 0x16, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8_SINT, TX_FORMAT(0, 0x16, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8B8A8_UNORM, TX_FORMAT(0x09, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8B8A8_SNORM, TX_FORMAT(0, 0x11, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8B8A8_UINT, TX_FORMAT(0, 0x17, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8B8A8_SINT, TX_FORMAT(0, 0x17, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R8G8B8A8_SRGB, TX_FORMAT(0x09, 0, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_B8G8R8A8_UNORM, TX_FORMAT(0x07, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_B8G8R8A8_SRGB, TX_FORMAT(0x07, 0, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A8B8G8R8_UNORM_PACK32, TX_FORMAT(0x09, 0, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A8B8G8R8_SNORM_PACK32, TX_FORMAT(0, 0x11, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A8B8G8R8_UINT_PACK32, TX_FORMAT(0, 0x17, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32, TX_FORMAT(0, 0x17, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A8B8G8R8_SRGB_PACK32, TX_FORMAT(0x09, 0, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, TX_FORMAT(0, 0x0C, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_A2R10G10B10_UNORM_PACK32, TX_FORMAT(0, 0x0C, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_BLUE, SWZL_USE_GREEN, SWZL_USE_RED, SWZL_USE_ALPHA)},
        {VK_FORMAT_A2B10G10R10_UINT_PACK32, TX_FORMAT(0, 0x1C, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_R16_UINT, TX_FORMAT(0, 0x18, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16_SINT, TX_FORMAT(0, 0x18, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16_SFLOAT, TX_FORMAT(0, 0x07, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16_UINT, TX_FORMAT(0, 0x19, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16_SINT, TX_FORMAT(0, 0x19, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16_SFLOAT, TX_FORMAT(0, 0x08, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16B16A16_UINT, TX_FORMAT(0, 0x1A, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16B16A16_SINT, TX_FORMAT(0, 0x1A, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R16G16B16A16_SFLOAT, TX_FORMAT(0, 0x09, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_R32_UINT, TX_FORMAT(0, 0x23, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R32_SINT, TX_FORMAT(0, 0x23, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R32_SFLOAT, TX_FORMAT(0, 0x0A, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R32G32_UINT, TX_FORMAT(0, 0x24, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R32G32_SINT, TX_FORMAT(0, 0x24, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_R32G32_SFLOAT, TX_FORMAT(0, 0x0B, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_B10G11R11_UFLOAT_PACK32, TX_FORMAT(0, 0x1B, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, TX_FORMAT(0x1D, 0, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_D16_UNORM, TX_FORMAT(0x10, 0, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},
        {VK_FORMAT_D32_SFLOAT, TX_FORMAT(0, 0x0A, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)},

        {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, TX_FORMAT(0, 0x00, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, TX_FORMAT(0, 0x00, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, TX_FORMAT(0, 0x01, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, TX_FORMAT(0, 0x01, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, TX_FORMAT(0, 0x02, VK_FALSE, VK_FALSE, VK_TRUE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, TX_FORMAT(0, 0x02, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_EAC_R11_UNORM_BLOCK, TX_FORMAT(0, 0x03, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_EAC_R11_SNORM_BLOCK, TX_FORMAT(0, 0x0D, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_EAC_R11G11_UNORM_BLOCK, TX_FORMAT(0, 0x04, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_EAC_R11G11_SNORM_BLOCK, TX_FORMAT(0, 0x05, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},

        {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_5x4_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_5x4_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_5x5_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_5x5_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_6x5_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_6x5_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_6x6_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_6x6_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x5_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x5_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x6_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x6_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x8_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_8x8_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x5_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x5_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x6_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x6_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x8_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x8_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x10_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_10x10_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_12x10_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_12x10_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_12x12_UNORM_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_FALSE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
        {VK_FORMAT_ASTC_12x12_SRGB_BLOCK, TX_FORMAT(0, 0x14, VK_FALSE, VK_TRUE, VK_FALSE),
            TX_COMP_SWIZZLE(SWZL_USE_RED, SWZL_USE_GREEN, SWZL_USE_BLUE, SWZL_USE_ALPHA)},
    };

    static const __vkFormatToHwTxFmtInfo s_vkR8HwTxInfo_halti2 =
    {
        VK_FORMAT_R8_UNORM,
        TX_FORMAT(0x01, 0, VK_FALSE, VK_FALSE, VK_TRUE),
        TX_COMP_SWIZZLE(SWZL_USE_ALPHA, SWZL_USE_ZERO, SWZL_USE_ZERO, SWZL_USE_ONE)
    };

    if (!devCtx->database->REG_Halti5 && vkFormat == VK_FORMAT_R8_UNORM)
    {
        hwTxFmtInfo = &s_vkR8HwTxInfo_halti2;
    }
    else
    {
        for (i = 0; i < __VK_COUNTOF(s_vkFormatToHwTxInfos); i++)
        {
            if (s_vkFormatToHwTxInfos[i].vkFormat == vkFormat)
            {
                hwTxFmtInfo = &s_vkFormatToHwTxInfos[i];
                break;
            }
        }
    }

    return hwTxFmtInfo;
}

float __vkLinearToNonLinear(
    float lFloat
    )
{
    float sFloat;

    if (lFloat <= 0.0f)
    {
        sFloat = 0.0f;
    }
    else if (lFloat < 0.0031308f)
    {
        sFloat = 12.92f * lFloat;
    }
    else if (lFloat < 1.0)
    {
        sFloat = 1.055f * gcoMATH_Power(lFloat, 0.41666f) - 0.055f;
    }
    else
    {
        sFloat = 1.0f;
    }

    return sFloat;
}

static uint32_t
__vkConvertSINT(
    IN int32_t value,
    IN uint32_t bits
    )
{
    uint32_t mask = (bits == 32) ? ~0 : ((1 << bits) - 1);
    int32_t iMinValue = (bits == 32)? (1 << (bits-1))   :((~(1 << (bits -1))) + 1);
    int32_t iMaxValue = (bits == 32)? (~(1 << (bits-1))): ((1 << (bits - 1)) - 1);

    return gcmCLAMP(value, iMinValue, iMaxValue) & mask;
}

static uint32_t
__vkConvertUINT(
    IN uint32_t value,
    IN uint32_t bits
    )
{
    uint32_t uMaxValue = (bits == 32) ? ~0 : ((1 << bits) - 1);
    return (value > uMaxValue ? uMaxValue : value);
}

static uint32_t
__vkConvertSFLOAT(
    IN gceVALUE_TYPE type,
    IN float value,
    IN uint32_t bits
    )
{
    /* Setup maximum value. */
    uint32_t tmpRet = 0;
    uint32_t uMaxValue = (bits == 32) ? ~0 : ((1 << bits) - 1);
    float sFloat = value;

    if (type & gcvVALUE_FLAG_GAMMAR)
    {
        gcmASSERT ((type & gcvVALUE_FLAG_FLOAT_TO_FLOAT16) == 0);
        sFloat = __vkLinearToNonLinear(sFloat);
    }

    if (type & gcvVALUE_FLAG_FLOAT_TO_FLOAT16)
    {
        gcmASSERT ((type & gcvVALUE_FLAG_GAMMAR) == 0);
        tmpRet = (uint32_t)gcoMATH_FloatToFloat16(*(uint32_t*)&sFloat);
        return tmpRet;
    }
    else if (type & gcvVALUE_FLAG_UNSIGNED_DENORM)
    {
        sFloat = gcmFLOATCLAMP_0_TO_1(sFloat);
        /* Convert floating point (0.0 - 1.0) into color value. */
        sFloat = gcoMATH_Multiply(gcoMATH_UInt2Float(uMaxValue), sFloat);
        tmpRet = gcoMath_Float2UINT_STICKROUNDING(sFloat);
        return tmpRet > uMaxValue ? uMaxValue : tmpRet;
    }
    else if (type & gcvVALUE_FLAG_SIGNED_DENORM)
    {
        int32_t sTmpRet = 0;
        int32_t sMaxValue = ((1 << (bits - 1)) - 1);

        __VK_ASSERT(bits < 32);

        sFloat = gcmFLOATCLAMP_NEG1_TO_1(sFloat);
        /* Convert floating point (-1.0 - 1.0) into color value. */
        sTmpRet = gcoMATH_Float2Int(gcoMATH_Multiply(gcoMATH_UInt2Float(sMaxValue), sFloat));
        sTmpRet = gcmCLAMP(sTmpRet, -sMaxValue, sMaxValue);
        return *(uint32_t*)&sTmpRet;
    }
    else
    {
        tmpRet = *(uint32_t*)&sFloat;
        return tmpRet > uMaxValue ? uMaxValue : tmpRet;
    }
}

/* (gctINT32)gcoMATH_Log2() can achieve same func, but has precision issue.
** e.g. gcoMATH_Log2(32768) = 14.999999... and become 14 after floor.
** Actually it should be 15.
*/
static int32_t __vkFloorLog2F(float val)
{
    int32_t exp = 0;
    uint64_t base = 1;

    if (val > 1.0f)
    {
        while (val >= (float)base)
        {
            base = (uint64_t)1 << (++exp);
        }
        exp--;
    }
    else
    {
        while (val < (1.0f / (float)base))
        {
            base = (uint64_t)1 << (++exp);
        }
        exp = -exp;
    }

    return exp;
}

VkResult __vkComputeClearVal(
    IN __vkImage *img,
    IN VkImageAspectFlags aspectMask,
    IN VkClearValue *vkClearValue,
    IN uint32_t partIndex,
    OUT uint32_t *pClearVals,
    OUT uint32_t *pClearBitMasks,
    OUT uint32_t *pClearByteMasks
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t bitMasks[2] = {0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t byteMasks[2] = {0xF, 0xF};

    switch (img->formatInfo.residentImgFormat)
    {
    case __VK_FORMAT_A4R4G4B4_UNFORM_PACK16:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 4) <<  8)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 4) <<  4)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 4)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[A], 4) << 12);

        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case __VK_FORMAT_R8_1_X8R8G8B8:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 8) <<  16)
            | 0xFF000000;

        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 5) << 10)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 5) <<  5)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 5)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[A], 1) << 15);

        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 5) << 11)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 6) <<  5)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 5)      );

        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8G8B8A8_UNORM:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 8))
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 8) <<  8)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 8) << 16)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[A], 8) << 24);

        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_B8G8R8A8_UNORM:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 8) << 16)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 8) <<  8)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 8)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[A], 8) << 24);

        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_B8G8R8A8_SRGB:
        pClearVals[0]
            = (__vkConvertSFLOAT((gceVALUE_TYPE)(gcvVALUE_FLAG_GAMMAR | gcvVALUE_FLAG_UNSIGNED_DENORM), vkClearValue->color.float32[R], 8) << 16)
            | (__vkConvertSFLOAT((gceVALUE_TYPE)(gcvVALUE_FLAG_GAMMAR | gcvVALUE_FLAG_UNSIGNED_DENORM), vkClearValue->color.float32[G], 8) <<  8)
            | (__vkConvertSFLOAT((gceVALUE_TYPE)(gcvVALUE_FLAG_GAMMAR | gcvVALUE_FLAG_UNSIGNED_DENORM), vkClearValue->color.float32[B], 8)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[3], 8) << 24);

        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8_SNORM:
        pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[R], 8);
        pClearVals[0] |= (pClearVals[0] << 8);
        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;
        break;

    case VK_FORMAT_R8G8_SNORM:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[R], 8)     )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[G], 8) << 8);

        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        pClearVals[0] = pClearVals[1]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[R], 8)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[G], 8) << 8 )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[B], 8) << 16)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_SIGNED_DENORM, vkClearValue->color.float32[A], 8) << 24);
        break;

    case VK_FORMAT_R8G8_UNORM:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 8)     )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 8) << 8);

        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        pClearVals[1] =
            pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 10)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[G], 10) << 10)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[B], 10) << 20)
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[A], 2) << 30);
        break;

    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertUINT(vkClearValue->color.uint32[R], 10)      )
            | (__vkConvertUINT(vkClearValue->color.uint32[G], 10) << 10)
            | (__vkConvertUINT(vkClearValue->color.uint32[B], 10) << 20)
            | (__vkConvertUINT(vkClearValue->color.uint32[A], 2) << 30);
        break;

    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertSINT(vkClearValue->color.uint32[R], 10)      )
            | (__vkConvertSINT(vkClearValue->color.uint32[G], 10) << 10)
            | (__vkConvertSINT(vkClearValue->color.uint32[B], 10) << 20)
            | (__vkConvertSINT(vkClearValue->color.uint32[A], 2) << 30);
        break;


    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertSINT(vkClearValue->color.int32[R], 8)      )
            | (__vkConvertSINT(vkClearValue->color.int32[G], 8) <<  8)
            | (__vkConvertSINT(vkClearValue->color.int32[B], 8) << 16)
            | (__vkConvertSINT(vkClearValue->color.int32[A], 8) << 24);
        break;

    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertUINT(vkClearValue->color.uint32[R], 8)      )
            | (__vkConvertUINT(vkClearValue->color.uint32[G], 8) <<  8)
            | (__vkConvertUINT(vkClearValue->color.uint32[B], 8) << 16)
            | (__vkConvertUINT(vkClearValue->color.uint32[A], 8) << 24);
        break;

    case VK_FORMAT_R8_SINT:
        pClearVals[0] = __vkConvertSINT(vkClearValue->color.int32[R], 8);
        pClearVals[0] |= (pClearVals[0] << 8);
        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8_UINT:
        pClearVals[0] = __vkConvertUINT(vkClearValue->color.uint32[R], 8);
        pClearVals[0] |= (pClearVals[0] << 8);
        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8_UNORM:
        pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->color.float32[R], 8);
        pClearVals[0] |= (pClearVals[0] << 8);
        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8G8_SINT:
        pClearVals[0]
            = (__vkConvertSINT(vkClearValue->color.int32[R], 8)     )
            | (__vkConvertSINT(vkClearValue->color.int32[G], 8) << 8);

        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R8G8_UINT:
        pClearVals[0]
            = (__vkConvertUINT(vkClearValue->color.uint32[R], 8)     )
            | (__vkConvertUINT(vkClearValue->color.uint32[G], 8) << 8);

        pClearVals[0] |= (pClearVals[0] << 16);
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R16_SFLOAT:
        pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[R], 16);
        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R16_SINT:
        pClearVals[0] = __vkConvertSINT(vkClearValue->color.int32[R], 16);
        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R16_UINT:
        pClearVals[0] = __vkConvertUINT(vkClearValue->color.uint32[R], 16);
        pClearVals[0] |= pClearVals[0] << 16;
        pClearVals[1] = pClearVals[0];
        break;

    case VK_FORMAT_R16G16_SFLOAT:
        pClearVals[1] =
            pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[R], 16)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[G], 16) << 16);
        break;

    case VK_FORMAT_R16G16_SINT:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertSINT(vkClearValue->color.int32[R], 16)
            | (__vkConvertSINT(vkClearValue->color.int32[G], 16) << 16));
        break;

    case VK_FORMAT_R16G16_UINT:
        pClearVals[0] =
            pClearVals[1]
            = (__vkConvertUINT(vkClearValue->color.uint32[R], 16)
            | (__vkConvertUINT(vkClearValue->color.uint32[G], 16) << 16));
        break;

    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        pClearVals[1] =
            pClearVals[0]
            = (gcoMATH_FloatToFloat11(vkClearValue->color.uint32[R])      )
            | (gcoMATH_FloatToFloat11(vkClearValue->color.uint32[G]) << 11)
            | (gcoMATH_FloatToFloat10(vkClearValue->color.uint32[B]) << 22);
        break;

    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        {
            const int32_t mBits = 9;
            const int32_t eBits = 5;
            const int32_t eBias = 15;
            const int32_t eMax  = (1 << eBits) - 1;
            const float   sharedExpMax = ((1 << mBits) - 1) * (1 << (eMax - eBias)) / (float)(1 << mBits);

            float rc   = gcmCLAMP(vkClearValue->color.float32[R], 0.0f, sharedExpMax);
            float gc   = gcmCLAMP(vkClearValue->color.float32[G], 0.0f, sharedExpMax);
            float bc   = gcmCLAMP(vkClearValue->color.float32[B], 0.0f, sharedExpMax);
            float maxc = gcoMATH_MAX(rc, gcoMATH_MAX(gc, bc));

            int32_t expp  = gcoMATH_MAX(-eBias - 1, __vkFloorLog2F(maxc)) + 1 + eBias;
            float   scale = (float)gcoMATH_Power(2.0f, (float)(expp - eBias - mBits));
            int32_t maxs  = (int32_t)(maxc / scale + 0.5f);

            uint32_t exps = (maxs == (1 << mBits)) ? (uint32_t)(expp + 1) : (uint32_t)expp;
            uint32_t rs = gcmMIN((uint32_t)(rc / scale + 0.5f), 511);
            uint32_t gs = gcmMIN((uint32_t)(gc / scale + 0.5f), 511);
            uint32_t bs = gcmMIN((uint32_t)(bc / scale + 0.5f), 511);

            pClearVals[1] = pClearVals[0] = rs | (gs << 9) | (bs << 18) | (exps << 27);
        }
        break;

    case VK_FORMAT_R16G16B16A16_SFLOAT:
        pClearVals[0]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[R], 16)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[G], 16) << 16);
        pClearVals[1]
            = (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[B], 16)      )
            | (__vkConvertSFLOAT(gcvVALUE_FLAG_FLOAT_TO_FLOAT16, vkClearValue->color.float32[A], 16) << 16);
        break;

    case VK_FORMAT_R16G16B16A16_SINT:
        pClearVals[0]
            = (__vkConvertSINT(vkClearValue->color.int32[R], 16)      )
            | (__vkConvertSINT(vkClearValue->color.int32[G], 16) << 16);
        pClearVals[1]
            = (__vkConvertSINT(vkClearValue->color.int32[B], 16)      )
            | (__vkConvertSINT(vkClearValue->color.int32[A], 16) << 16);
        break;

    case VK_FORMAT_R16G16B16A16_UINT:
        pClearVals[0]
            = (__vkConvertUINT(vkClearValue->color.uint32[R], 16)      )
            | (__vkConvertUINT(vkClearValue->color.uint32[G], 16) << 16);
        pClearVals[1]
            = (__vkConvertUINT(vkClearValue->color.uint32[B], 16)      )
            | (__vkConvertUINT(vkClearValue->color.uint32[A], 16) << 16);
        break;


    case VK_FORMAT_R32_SFLOAT:
        pClearVals[0] =
            pClearVals[1] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[R], 32);
        break;

    case VK_FORMAT_R32_SINT:
        pClearVals[0] =
            pClearVals[1] = __vkConvertSINT(vkClearValue->color.int32[R], 32);
        break;

    case VK_FORMAT_R32_UINT:
        pClearVals[0] =
            pClearVals[1] = __vkConvertUINT(vkClearValue->color.uint32[R], 32);
        break;

    case VK_FORMAT_R32G32_SFLOAT:
        pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[R], 32);
        pClearVals[1] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[G], 32);
        break;

    case VK_FORMAT_R32G32_SINT:
        pClearVals[0] = __vkConvertSINT(vkClearValue->color.int32[R], 32);
        pClearVals[1] = __vkConvertSINT(vkClearValue->color.int32[G], 32);
        break;

    case VK_FORMAT_R32G32_UINT:
        pClearVals[0] = __vkConvertUINT(vkClearValue->color.uint32[R], 32);
        pClearVals[1] = __vkConvertUINT(vkClearValue->color.uint32[G], 32);
        break;

    case VK_FORMAT_D16_UNORM:
        if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            pClearVals[0] = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->depthStencil.depth, 16));
            pClearVals[0] |= (pClearVals[0] << 16);
            pClearVals[1] = pClearVals[0];
        }
        break;

    case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
        {
            uint32_t tmpBitMask = 0;
            uint32_t tmpByteMask = 0;
            if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
            {
                pClearVals[0] = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->depthStencil.depth, 24) << 8);
                tmpBitMask |= 0xFFFFFF00;
                tmpByteMask |= 0xE;
            }

            if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
            {
                pClearVals[0] &= ~0xFF;
                pClearVals[0] |= vkClearValue->depthStencil.stencil;
                tmpBitMask |= 0x000000FF;
                tmpByteMask |= 0x1;
            }

            pClearVals[1] = pClearVals[0];
            bitMasks[0] = bitMasks[1] = tmpBitMask;
            byteMasks[0] = byteMasks[1] = tmpByteMask;
        }
        break;

    case __VK_FORMAT_D24_UNORM_X8_PACKED32:
        if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            pClearVals[0] =
                pClearVals[1] = (__vkConvertSFLOAT(gcvVALUE_FLAG_UNSIGNED_DENORM, vkClearValue->depthStencil.depth, 24) << 8);
            bitMasks[0] = bitMasks[1] = 0xFFFFFF00;
            byteMasks[0] = byteMasks[1] = 0xF;
        }
        break;

    case VK_FORMAT_D32_SFLOAT:
        if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
        {
            pClearVals[0] = pClearVals[1] = *(uint32_t*)(&vkClearValue->depthStencil.depth);
            bitMasks[0] = bitMasks[1] = 0xFFFFFFFF;
            byteMasks[0] = byteMasks[1] = 0xF;
        }
        break;

    case VK_FORMAT_S8_UINT:
        if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            uint32_t tmpBitMask = 0xFF;
            pClearVals[0] = vkClearValue->depthStencil.stencil;
            pClearVals[0]  |= (pClearVals[0] << 8);
            pClearVals[0]  |= (pClearVals[0] << 16);
            pClearVals[1] = pClearVals[0];
            tmpBitMask |= (tmpBitMask << 8);
            tmpBitMask |= (tmpBitMask << 16);
            bitMasks[0] = bitMasks[1] = tmpBitMask;
            byteMasks[0] = byteMasks[1] = 0xF;
        }
        break;

    case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
        switch(partIndex)
        {
        case 0:
            pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[R], 32);
            pClearVals[1] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[G], 32);
            break;
        case 1:
            pClearVals[0] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[B], 32);
            pClearVals[1] = __vkConvertSFLOAT(gcvVALUE_FLOAT, vkClearValue->color.float32[A], 32);
            break;
        default:
            __VK_ASSERT(!"invalid part index value");
            break;
        }
        break;

    case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
        switch(partIndex)
        {
        case 0:
            pClearVals[0] = __vkConvertSINT(vkClearValue->color.int32[R], 32);
            pClearVals[1] = __vkConvertSINT(vkClearValue->color.int32[G], 32);
            break;
        case 1:
            pClearVals[0] = __vkConvertSINT(vkClearValue->color.int32[B], 32);
            pClearVals[1] = __vkConvertSINT(vkClearValue->color.int32[A], 32);
            break;
    default:
            __VK_ASSERT(!"invalid part index value");
            break;
        }
        break;

    case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
        switch(partIndex)
        {
        case 0:
            pClearVals[0] = __vkConvertUINT(vkClearValue->color.uint32[R], 32);
            pClearVals[1] = __vkConvertUINT(vkClearValue->color.uint32[G], 32);
            break;
        case 1:
            pClearVals[0] = __vkConvertUINT(vkClearValue->color.uint32[B], 32);
            pClearVals[1] = __vkConvertUINT(vkClearValue->color.uint32[A], 32);
            break;
        default:
            __VK_ASSERT(!"invalid part index value");
            break;
        }
        break;

    default:
        result = VK_ERROR_FORMAT_NOT_SUPPORTED;
        __VK_DEBUG_PRINT(0, "Invalid clear format %p\n", img->formatInfo.residentImgFormat);
        break;
    }

    if(pClearBitMasks)
    {
        __VK_MEMCOPY(pClearBitMasks, bitMasks, sizeof(bitMasks));
    }

    if (pClearByteMasks)
    {
        __VK_MEMCOPY(pClearByteMasks, byteMasks, sizeof(byteMasks));
    }
    return result;
}


VkResult halti5_helper_convertHwBltDesc(
    VkBool32 isSource,
    uint32_t vkFormat,
    HwBLTDesc *hwBltDesc
    )
{
    uint32_t format = 0;
    uint32_t swizzle = 0;
    VkBool32 srgb = VK_FALSE;
    VkBool32 bFakeFormat = VK_FALSE;
    gceMSAA_DOWNSAMPLE_MODE downsampleMode = gcvMSAA_DOWNSAMPLE_AVERAGE;
    uint32_t i;
    gctUINT32 bitsPerPixel;
    static const struct
    {
        uint32_t vkFormat;
        uint32_t hwFormat;
        uint32_t hwSwizzle;
        VkBool32 hwSrgb;
    }
    s_vkFormatToHwblitDescs[] =
    {
        {VK_FORMAT_R4G4B4A4_UNORM_PACK16, 0x01, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_B4G4R4A4_UNORM_PACK16, 0x01, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {__VK_FORMAT_A4R4G4B4_UNFORM_PACK16, 0x01, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},

        {VK_FORMAT_R5G6B5_UNORM_PACK16, 0x04, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_B5G6R5_UNORM_PACK16, 0x04, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_A1R5G5B5_UNORM_PACK16, 0x03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_B5G5R5A1_UNORM_PACK16, 0x03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_R5G5B5A1_UNORM_PACK16, 0x03, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},

        {VK_FORMAT_R8_UNORM, 0x23, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_R8_SRGB, 0x23, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_TRUE},

        {VK_FORMAT_R8G8_UNORM, 0x24, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_R8G8_SRGB, 0x24, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_TRUE},

        {VK_FORMAT_R8G8B8A8_UNORM, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_R8G8B8A8_SRGB, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_TRUE},
        {VK_FORMAT_B8G8R8A8_UNORM, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_B8G8R8A8_SRGB, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_TRUE},
        {VK_FORMAT_A8B8G8R8_UINT_PACK32, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_A8B8G8R8_UNORM_PACK32, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_A8B8G8R8_SRGB_PACK32, 0x06, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_TRUE},

        {VK_FORMAT_A2R10G10B10_UNORM_PACK32, 0x16, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, 0x16, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},

        {VK_FORMAT_R16G16B16A16_SINT, 0x1C, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},

        {VK_FORMAT_D16_UNORM, 0x18, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {__VK_FORMAT_D24_UNORM_S8_UINT_PACKED32, 0x17, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
        {__VK_FORMAT_D24_UNORM_X8_PACKED32, 0x17, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9))), VK_FALSE},
    };
    bitsPerPixel = g_vkFormatInfoTable[vkFormat].bitsPerBlock / g_vkFormatInfoTable[vkFormat].partCount;
    hwBltDesc->pixelSize = bitsPerPixel;

    __VK_ASSERT(hwBltDesc);

    for (i = 0; i < __VK_COUNTOF(s_vkFormatToHwblitDescs); i++)
    {
        if (vkFormat == s_vkFormatToHwblitDescs[i].vkFormat)
        {
            format = s_vkFormatToHwblitDescs[i].hwFormat;
            swizzle = s_vkFormatToHwblitDescs[i].hwSwizzle;
            srgb = s_vkFormatToHwblitDescs[i].hwSrgb;
            break;
        }
    }

    if (i >= __VK_COUNTOF(s_vkFormatToHwblitDescs))
    {
        /* Fake format.*/
        switch (bitsPerPixel)
        {
        case 8:
            format = 0x23;
            hwBltDesc->pixelSize = 8;
            break;
        case 16:
            format = 0x01;
            hwBltDesc->pixelSize = 16;
            break;
        case 24:
            format = 0x22;
            hwBltDesc->pixelSize = 24;
            break;
        case 32:
            format = 0x06;
            hwBltDesc->pixelSize = 32;
            break;
        case 64:
            format = 0x1C;
            hwBltDesc->pixelSize = 64;
            downsampleMode = gcvMSAA_DOWNSAMPLE_SAMPLE;
            break;
        case 128:
            if (vkFormat >= VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK &&
                vkFormat <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
            {
                format = 0x1C;
                hwBltDesc->pixelSize = 128;
                downsampleMode = gcvMSAA_DOWNSAMPLE_SAMPLE;
            }
            else
            {
                return VK_ERROR_FORMAT_NOT_SUPPORTED;
            }
            break;
        default:
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        /* set return value.*/
        swizzle = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9)));
        bFakeFormat = gcvTRUE;
    }
    else if (!isSource)
    {
        /* dstSwizzle is different than srcSwizzle when faking RGBA format as ARGB. */
        switch (vkFormat)
        {
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            swizzle = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9)));
            break;
        }
    }

    if (g_vkFormatInfoTable[vkFormat].category == __VK_FMT_CATEGORY_UINT ||
        g_vkFormatInfoTable[vkFormat].category == __VK_FMT_CATEGORY_SINT)
    {
        downsampleMode = gcvMSAA_DOWNSAMPLE_SAMPLE;
    }

    hwBltDesc->hwFormat       = format;
    hwBltDesc->bltSwizzleEx   = swizzle;
    hwBltDesc->downSampleMode = downsampleMode;
    hwBltDesc->sRGB           = srgb;
    hwBltDesc->fakeFormat     = bFakeFormat;

    return VK_SUCCESS;
}

void halti5_helper_configMSAA(
    __vkImage *img,
    uint32_t *msaa,
    uint32_t *cacheMode
    )
{
    switch (img->sampleInfo.product)
    {
    case 4:
        *msaa = 0x3;
        *cacheMode = 0x1;
        break;
    case 2:
        *msaa = 0x1;
        *cacheMode = 0x0;
        break;
    default:
        *msaa = 0x0;
        *cacheMode = 0x0;
        break;
    }
}

void halti5_helper_configTiling(
    __vkImage *img,
    uint32_t *tiling,
    uint32_t *superTile
    )
{
    switch (img->halTiling)
    {
    case gcvTILED:
        *tiling    = 0x1;
        *superTile = 0x0;
        break;
    case gcvSUPERTILED:
        *tiling    = 0x1;
        *superTile = 0x1;
        break;
    default:
        *tiling    = 0x0;
        *superTile = 0x0;
        break;
    }
}

VkResult halti5_clearImage(
    VkCommandBuffer cmdBuf,
    VkImage image,
    VkImageSubresource *subResource,
    VkClearValue *clearValue,
    VkRect2D *rect
    )
{
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, image);
    __vkImageLevel *pLevel = &img->pImgLevels[subResource->mipLevel];
    uint32_t msaa, superTile, tiling, cacheMode, address;
    uint32_t clearVals[2] = {0}, clearMasks[2] = {0};
    uint32_t config, dstConfigEx, srcConfigEx;
    uint32_t *states = gcvNULL;
    VkResult result = VK_SUCCESS;
    uint32_t offset = 0;
    uint32_t partIndex = 0;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cmdBuf;
    __vkDevContext *devCtx = cmd->devCtx;
    uint32_t srcTileConfigEx = 0, dstTileConfigEx = 0;
#if __VK_ENABLETS
    uint32_t fcClearValue[2] = {0};
    __vkTileStatus *tsResource = img->memory->ts;
    VkBool32 fastClear = VK_TRUE;
    VkBool32 compression = VK_FALSE;
    int32_t compressionFormat = -1;
    uint32_t tileStatusAddress = tsResource ? tsResource->devAddr : VK_NULL_HANDLE;
    VkImageSubresourceRange imgvRange = {subResource->aspectMask, subResource->mipLevel, 1, subResource->arrayLayer, 1};
#endif
    VkBool32 forceSGPU = VK_FALSE;

    halti5_helper_configMSAA(img, &msaa, &cacheMode);
    halti5_helper_configTiling(img, &tiling, &superTile);

    pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];

    /* 128BTILE feature, we need program the tilemode,the supertile is same with xmajor */
    if (devCtx->database->CACHE128B256BPERLINE)
    {
        if(img->halTiling == gcvSUPERTILED)
        {
            srcTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)));
            dstTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)));
        }
        else if (img->halTiling == gcvYMAJOR_SUPERTILED)
        {
            srcTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
            srcTileConfigEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)));
            dstTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
            dstTileConfigEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)));
        }
    }

    if ((img->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (img->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
    {
        img->formatInfo.residentImgFormat = VK_FORMAT_R8G8B8A8_UNORM;
    }

    while (partIndex < pLevel->partCount)
    {
        uint32_t originX, originY;
        uint32_t width, height;

        __VK_ONERROR(__vkComputeClearVal(img,
            subResource->aspectMask,
            clearValue,
            partIndex,
            clearVals,
            clearMasks,
            VK_NULL_HANDLE));
#if __VK_ENABLETS
        /* Test for entire surface clear. */
        if ((rect->offset.x == 0) && (rect->offset.y == 0)
            && (rect->extent.width == pLevel->requestW)
            && (rect->extent.height == pLevel->requestH)
            && (clearMasks[0] == 0xFFFFFFFF)
            && (clearMasks[1] == 0xFFFFFFFF))
        {
            fcClearValue[0] = clearVals[0];
            fcClearValue[1] = clearVals[1];
        }
        else
        {
            if (tsResource)
            {
                fcClearValue[0] = tsResource->fcValue[imgvRange.baseMipLevel][imgvRange.baseArrayLayer];
                fcClearValue[1] = tsResource->fcValueUpper[imgvRange.baseMipLevel][imgvRange.baseArrayLayer];
            }
        }

        if (tsResource)
        {
            fastClear &= !tsResource->tileStatusDisable[subResource->mipLevel][subResource->arrayLayer];
        }
        else
        {
            fastClear = VK_FALSE;
        }

        if (!fastClear)
        {
            halti5_decompressTileStatus(cmd, &pCmdBuffer, img, &imgvRange);
        }
        else
        {
            compression = tsResource->compressed;
            compressionFormat = tsResource->compressedFormat;
        }
#endif

        /* Flush the pipe. */
        __VK_ONERROR(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

        config = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:0) - (0 ?
 20:0) + 1))))))) << (0 ?
 20:0))) | (((gctUINT32) ((gctUINT32) (pLevel->stride) & ((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:0) - (0 ? 20:0) + 1))))))) << (0 ? 20:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (msaa) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (tiling) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (superTile) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

        dstConfigEx =
#if __VK_ENABLETS
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (fastClear) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (compression) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (compressionFormat) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
#else
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
#endif
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (clearVals[0] != clearVals[1]) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (cacheMode) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
            | dstTileConfigEx;

        srcConfigEx =
#if __VK_ENABLETS
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (fastClear) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (compression) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (compressionFormat) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
#else
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
#endif
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (clearVals[0] != clearVals[1]) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (cacheMode) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
            | srcTileConfigEx;


        address = img->memory->devAddr;
        offset = (uint32_t)(img->memOffset
            + pLevel->partSize * partIndex
            + pLevel->offset
            + subResource->arrayLayer * pLevel->sliceSize);
        address += offset;
#if __VK_ENABLETS
        if (fastClear)
        {
            tileStatusAddress = halti5_computeTileStatusAddr(devCtx, img, offset);
        }
#endif
        if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&(pCmdBuffer))++ = 0;
;
 } }__vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );;


        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5019, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:7) - (0 ?
 9:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:7) - (0 ?
 9:7) + 1))))))) << (0 ?
 9:7))) | (((gctUINT32) ((gctUINT32) ((img->formatInfo.bitsPerBlock >> 3) - 1) & ((gctUINT32) ((((1 ?
 9:7) - (0 ?
 9:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:7) - (0 ? 9:7) + 1))))))) << (0 ? 9:7)))
            );

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5009, VK_FALSE, config);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500A, VK_FALSE, dstConfigEx);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, address);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5002, VK_FALSE, config);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5003, VK_FALSE, srcConfigEx);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, address);

        if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE,
                (address + (uint32_t)pLevel->sliceSize - 1));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE,
                (address + (uint32_t)pLevel->sliceSize - 1));
        }

        __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x5011, VK_FALSE, 2, clearVals);
        __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x5013, VK_FALSE, 2, clearMasks);
#if __VK_ENABLETS
        if (fastClear)
        {
            /* DestTileStatusAddress. */
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5008, VK_FALSE, tileStatusAddress);
            /* Set SrcTileStatusAddress. */
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5004, VK_FALSE, tileStatusAddress);
            /* DstClearValue. */
            __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x500F, VK_FALSE, 2, fcClearValue);
            /* SrcClearValue. */
            __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x500D, VK_FALSE, 2, fcClearValue);
        }
#endif
        originX = rect->offset.x * img->sampleInfo.x;
        originY = rect->offset.y * img->sampleInfo.y;
        width   = rect->extent.width  * img->sampleInfo.x;
        height  = rect->extent.height * img->sampleInfo.y;
        if (!forceSGPU && devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        {
            uint32_t i;
            uint32_t gpuCount = devCtx->chipInfo->gpuCoreCount;
            uint32_t averageW = __VK_ALIGN(width / gpuCount, img->sampleInfo.x);
            uint32_t splitWidth = width - (gpuCount - 1) * averageW;
            uint32_t dstStartX = originX;
            uint32_t leftWidth = width;

            for (i = 0; i < gpuCount; ++i)
            {
                /* Align dstEndX to 64 boundary */
                uint32_t dstEndX = __VK_ALIGN(dstStartX + splitWidth, 64);
                splitWidth = __VK_MIN(dstEndX - dstStartX, leftWidth);

                *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*&pCmdBuffer)++ = 0;
;


                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (dstStartX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (originY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    );
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (splitWidth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    );

                leftWidth -= splitWidth;
                dstStartX += splitWidth;
                splitWidth = averageW;
            }

            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

        }
        else
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (originX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (originY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );
        }

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&(pCmdBuffer))++ = 0;
;
 }halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 };

#if __VK_ENABLETS
        if (fastClear)
        {
            /* Record FC value. */
            tsResource->fcValue[subResource->mipLevel][subResource->arrayLayer] = fcClearValue[0];
            tsResource->fcValueUpper[subResource->mipLevel][subResource->arrayLayer] = fcClearValue[1];

            /* Turn the tile status on again. */
            tsResource->tileStatusDisable[subResource->mipLevel][subResource->arrayLayer] = gcvFALSE;
        }
#endif
        partIndex++;
    }

    cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    if (cmd->curScrachBufIndex > 0)
    {
        __vk_CmdAquireBuffer(cmdBuf, cmd->curScrachBufIndex, &states);
        __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));
        __vk_CmdReleaseBuffer(cmdBuf, cmd->curScrachBufIndex);
    }

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmd->curScrachBufIndex = 0;
    return result;
}

VkResult halti5_copyImage(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy
    )
{
    HwBLTDesc srcBltDesc = {0}, dstBltDesc = {0};
    uint32_t srcAddress, dstAddress;
    uint32_t srcConfig, srcConfigEx, dstConfig, dstConfigEx;
    uint32_t srcMsaa, srcCacheMode, dstMsaa, dstCacheMode;
    uint32_t srcSuperTile, srcTiling, dstSuperTile, dstTiling;
    uint32_t *states = gcvNULL;
    gcsSAMPLES srcSampleInfo = {0}, dstSampleInfo = {0};
    uint32_t srcStride = 0, dstStride = 0;
    uint32_t srcFormat, dstFormat;
    VkImageAspectFlags srcAspect;
    __VK_DEBUG_ONLY(VkImageAspectFlags dstAspect;)
    uint32_t srcParts, dstParts;
    uint32_t srcPartSize, dstPartSize;
    VkOffset3D srcOffset, dstOffset;
    VkExtent3D srcExtent, dstExtent;
    VkBool32 useComputeBlit = VK_FALSE;
    VkResult result = VK_SUCCESS;
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cmdBuf;
    __vkDevContext *devCtx = cmd->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint32_t srcTileConfigEx = 0, dstTileConfigEx = 0;
    uint32_t partIdx;
#if __VK_ENABLETS
    uint32_t srcTileStatusAddress = 0;
    VkBool32 srcFastClear = VK_FALSE;
    int32_t srcCompressionFormat = -1;
    VkBool32 srcCompression = VK_FALSE;
    __vkTileStatus * srcTsResource = VK_NULL_HANDLE;
    uint32_t color64 = 0;
#endif
    uint32_t offset = 0;
    __vkImage *srcImg = VK_NULL_HANDLE;

    VkBool32 forceSGPU = VK_FALSE;

    if (!srcRes->isImage && !dstRes->isImage)
    {
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];

    if (srcRes->isImage)
    {
        __vkImageLevel *pSrcLevel;
        srcImg = srcRes->u.img.pImage;
#if __VK_ENABLETS
        srcTsResource = srcImg->memory->ts;
#endif
        pSrcLevel = &srcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];
        srcOffset = srcRes->u.img.offset;
        srcExtent = srcRes->u.img.extent;
        srcAspect = srcRes->u.img.subRes.aspectMask;

        srcStride = (uint32_t)pSrcLevel->stride;
        srcSampleInfo = srcImg->sampleInfo;
        srcFormat = srcImg->formatInfo.residentImgFormat;

        if ((srcImg->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (srcImg->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
        {
            srcFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }

        srcParts = pSrcLevel->partCount;
        srcPartSize = (uint32_t)pSrcLevel->partSize;
        srcAddress = srcImg->memory->devAddr;
        offset = (uint32_t)(srcImg->memOffset + pSrcLevel->offset + srcRes->u.img.subRes.arrayLayer * pSrcLevel->sliceSize);
        srcAddress += offset;
        halti5_helper_configMSAA(srcImg, &srcMsaa, &srcCacheMode);
        halti5_helper_configTiling(srcImg, &srcTiling, &srcSuperTile);
#if __VK_ENABLETS
        if (srcTsResource)
        {
            /* Flush the tile status cache. */
            __VK_ONERROR(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));
            srcTileStatusAddress = halti5_computeTileStatusAddr(devCtx, srcImg, offset);;

            color64 = (srcTsResource->fcValue[srcRes->u.img.subRes.mipLevel][srcRes->u.img.subRes.arrayLayer] !=
                srcTsResource->fcValueUpper[srcRes->u.img.subRes.mipLevel][srcRes->u.img.subRes.arrayLayer]);

            srcFastClear = !srcTsResource->tileStatusDisable[srcRes->u.img.subRes.mipLevel][srcRes->u.img.subRes.arrayLayer];
            srcCompressionFormat = srcTsResource->compressedFormat;
            srcCompression = srcTsResource->compressed;
        }
#endif
        if (devCtx->database->CACHE128B256BPERLINE)
        {
            if(srcImg->halTiling == gcvSUPERTILED)
            {
                srcTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)));
            }
            else if (srcImg->halTiling == gcvYMAJOR_SUPERTILED)
            {
                srcTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
                srcTileConfigEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)));
            }
        }
    }
    else
    {
        uint32_t srcWidth;
        const __vkFormatInfo *fmtInfo;
        __vkBuffer *srcBuf = srcRes->u.buf.pBuffer;
        __vkImage *dstImg = dstRes->u.img.pImage;

        srcOffset.x = srcOffset.y = srcOffset.z = 0;
        srcExtent = dstRes->u.img.extent;
        srcAspect = dstRes->u.img.subRes.aspectMask;

        srcAddress = srcBuf->memory->devAddr;
        srcAddress += (uint32_t)(srcBuf->memOffset + srcRes->u.buf.offset);
        srcTiling    = 0x0;
        srcSuperTile = 0x0;
        srcMsaa      = 0x0;
        srcCacheMode = 0x0;

        srcFormat = dstImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[srcFormat];
        srcParts = fmtInfo->partCount;
        srcPartSize = (uint32_t)(srcBuf->memReq.size);
        srcWidth  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        srcStride = (srcWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        srcSampleInfo = dstImg->sampleInfo;
    }

    if (dstRes->isImage)
    {
        __vkImage *dstImg = dstRes->u.img.pImage;
        __vkImageLevel *pDstLevel = &dstImg->pImgLevels[dstRes->u.img.subRes.mipLevel];
#if __VK_ENABLETS
        VkImageSubresourceRange *imgvRange;
        VkBool32 dstAnyTsEnable = VK_FALSE;
        __vkTileStatus * dstTsResource = dstImg->memory->ts;

        __VK_SET_ALLOCATIONCB(&dstImg->memCb);

        imgvRange = (VkImageSubresourceRange*)__VK_ALLOC(sizeof(VkImageSubresourceRange), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        imgvRange->aspectMask = dstRes->u.img.subRes.aspectMask;
        imgvRange->baseArrayLayer = dstRes->u.img.subRes.arrayLayer;
        imgvRange->baseMipLevel = dstRes->u.img.subRes.mipLevel;
        imgvRange->layerCount = imgvRange->levelCount = 1;
#endif
        dstOffset = dstRes->u.img.offset;
        dstExtent = dstRes->u.img.extent;
        __VK_DEBUG_ONLY(dstAspect = dstRes->u.img.subRes.aspectMask);

        dstStride = (uint32_t)pDstLevel->stride;
        dstSampleInfo = dstImg->sampleInfo;
        dstFormat = dstImg->formatInfo.residentImgFormat;

        if ((dstImg->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) && (dstImg->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
        {
            dstFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }

        dstParts = pDstLevel->partCount;
        dstPartSize = (uint32_t)pDstLevel->partSize;
        dstAddress = dstImg->memory->devAddr;
        dstAddress += (uint32_t)(dstImg->memOffset + pDstLevel->offset +
                                 dstRes->u.img.subRes.arrayLayer * pDstLevel->sliceSize);
        halti5_helper_configMSAA(dstImg, &dstMsaa, &dstCacheMode);
        halti5_helper_configTiling(dstImg, &dstTiling, &dstSuperTile);
#if __VK_ENABLETS
        __VK_AnyTSEnable(dstTsResource, imgvRange, &dstAnyTsEnable);
#endif
        if (devCtx->database->CACHE128B256BPERLINE)
        {
            if (dstImg->halTiling == gcvSUPERTILED)
            {
                dstTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)));
            }
            else if (dstImg->halTiling == gcvYMAJOR_SUPERTILED)
            {
                dstTileConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)));
                dstTileConfigEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)));
            }
        }
#if __VK_ENABLETS
        if (dstAnyTsEnable)
        {
            result = halti5_decompressTileStatus(cmd, &pCmdBuffer, dstImg, imgvRange);
        }

        __VK_FREE(imgvRange);
#endif
        __VK_ONERROR(result);
    }
    else
    {
        uint32_t dstWidth;
        const __vkFormatInfo *fmtInfo;
        __vkBuffer *dstBuf = dstRes->u.buf.pBuffer;
        __vkImage *srcImg = srcRes->u.img.pImage;

        dstOffset.x = dstOffset.y = dstOffset.z = 0;
        dstExtent = srcRes->u.img.extent;
        __VK_DEBUG_ONLY(dstAspect = srcRes->u.img.subRes.aspectMask);

        dstAddress = dstBuf->memory->devAddr;
        dstAddress += (uint32_t)(dstBuf->memOffset + dstRes->u.buf.offset);
        dstTiling    = 0x0;
        dstSuperTile = 0x0;
        dstMsaa      = 0x0;
        dstCacheMode = 0x0;

        dstFormat = srcImg->createInfo.format;
        if (srcRes->u.img.subRes.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            switch (srcImg->createInfo.format)
            {
            case VK_FORMAT_D24_UNORM_S8_UINT:
                dstFormat = VK_FORMAT_R8_UNORM;
                break;
            case VK_FORMAT_S8_UINT:
                break;
            default:
                /* ASPECT_STENCIL_BIT didn't support other formats */
                __VK_ASSERT(0);
            }
        }

        fmtInfo = &g_vkFormatInfoTable[dstFormat];
        dstParts = fmtInfo->partCount;
        dstPartSize = (uint32_t)(dstBuf->memReq.size);
        dstWidth  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstStride = (dstWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        dstSampleInfo = srcImg->sampleInfo;
    }

    if (rawCopy)
    {
        /* Change srcFormat to be same as dstFormat for CmdCopyImage() */
        srcFormat = dstFormat;

        /* Fake 1 layer 128 bpp format as double widthed 64bpp */
        if (g_vkFormatInfoTable[dstFormat].bitsPerBlock == 128 && g_vkFormatInfoTable[dstFormat].partCount == 1)
        {
            srcFormat = dstFormat = VK_FORMAT_R16G16B16A16_UINT;
            srcOffset.x *= 2;
            dstOffset.x *= 2;
            srcExtent.width  *= 2;
            dstExtent.width  *= 2;
        }

        if (g_vkFormatInfoTable[dstFormat].bitsPerBlock >= 64 &&
            srcMsaa != dstMsaa)
        {
            useComputeBlit = VK_TRUE;
            rawCopy = VK_FALSE;
        }
    }

    if (srcParts != dstParts)
    {
        useComputeBlit = VK_TRUE;
    }
    else
    {
        __VK_ONERROR(halti5_helper_convertHwBltDesc(VK_TRUE, srcFormat, &srcBltDesc));
        __VK_ONERROR(halti5_helper_convertHwBltDesc(VK_FALSE, dstFormat, &dstBltDesc));

        /* Disable srgb if both srgb source and destination (Multiple conversion is not needed and can cause errors */
        if (srcBltDesc.sRGB && dstBltDesc.sRGB)
        {
            srcBltDesc.sRGB = dstBltDesc.sRGB = VK_FALSE;
        }

        /* Need special deal with if copying to block compressed texture. treat as linear to upload.
           One block is treated as one or two pixel to upload. It is required to handle compressed
           images created with dimensions that are not a multiple of the block dimensions.
        */
        if (dstFormat >= VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK &&
            dstFormat <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK
            )
        {
            VkExtent2D rect;
            const __vkFormatInfo *fmtInfo;
            fmtInfo = &g_vkFormatInfoTable[dstFormat];
            rect = fmtInfo->blockSize;

            dstTiling = 0x0;
            dstSuperTile = 0x0;
            dstTileConfigEx = 0;
            srcOffset.x = gcmALIGN_NP2(srcOffset.x - rect.width  + 1, rect.width);
            srcOffset.y = gcmALIGN_NP2(srcOffset.y - rect.height + 1, rect.height);
            dstOffset.x = gcmALIGN_NP2(dstOffset.x - rect.width  + 1, rect.width);
            dstOffset.y = gcmALIGN_NP2(dstOffset.y - rect.height + 1, rect.height);
            srcExtent.width  = gcmALIGN_NP2(srcExtent.width, rect.width)  / rect.width;
            srcExtent.height = gcmALIGN_NP2(srcExtent.height, rect.height) / rect.height;
            dstExtent.width  = gcmALIGN_NP2(dstExtent.width, rect.width)  / rect.width;
            dstExtent.height = gcmALIGN_NP2(dstExtent.height, rect.height) / rect.height;

            if (dstBltDesc.pixelSize == 128)
            {
                dstBltDesc.pixelSize = 64;
                srcBltDesc.pixelSize = 64;
                srcOffset.x *= 2;
                dstOffset.x *= 2;
                srcExtent.width *= 2;
                dstExtent.width *= 2;
            }
        }
        /* Need special swizzle when copying from D24S8 / D24X8 to buffer */
        if (!dstRes->isImage && srcBltDesc.hwFormat == 0x17)
        {
            /* Change both formats to ARGB8888 since swizzle is ignored in 3D blt for depth formats */
            srcBltDesc.hwFormat = 0x06;
            srcBltDesc.bltSwizzleEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9)));
            srcBltDesc.fakeFormat = VK_FALSE;

            switch (dstBltDesc.pixelSize)
            {
            case 8:
                dstBltDesc.hwFormat = 0x23;
                dstBltDesc.bltSwizzleEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9)));
                dstBltDesc.fakeFormat = VK_FALSE;
                break;
            case 32:
                dstBltDesc.hwFormat = 0x06;
                dstBltDesc.bltSwizzleEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:9) - (0 ?
 11:9) + 1))))))) << (0 ?
 11:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:9) - (0 ?
 11:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:9) - (0 ? 11:9) + 1))))))) << (0 ? 11:9)));
                dstBltDesc.fakeFormat = VK_FALSE;
                break;
            default:
                __VK_ASSERT(0);
            }
        }
        /* For depth or stencil only aspect: cannot support combined formats now. */
        else if (((srcAspect & VK_IMAGE_ASPECT_DEPTH_BIT  ) && !(srcAspect & ~VK_IMAGE_ASPECT_DEPTH_BIT  )) ||
                 ((srcAspect & VK_IMAGE_ASPECT_STENCIL_BIT) && !(srcAspect & ~VK_IMAGE_ASPECT_STENCIL_BIT)))
        {
            /* For depth or stencil only aspect: cannot support combined formats now. */
            switch (dstFormat)
            {
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                __VK_DEBUG_PRINT(__VK_DBG_LEVEL_ERROR, "copyImage does NOT support the combination now: dstAspect=0x%x dstFormat=%d\n",
                                 dstAspect, dstFormat);
                useComputeBlit = VK_TRUE;
                break;
            default:
                break;
            }

            if (dstFormat == __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32)
            {
                useComputeBlit = VK_TRUE;
            }
        }

        if (!useComputeBlit)
        {
            /* Do NOT support stretch copy */
            if (gcoOS_MemCmp(&srcExtent, &dstExtent, gcmSIZEOF(VkExtent2D)))
            {
                useComputeBlit = VK_TRUE;
            }
            /* Fake format only works when same format */
            else if ((srcBltDesc.fakeFormat || dstBltDesc.fakeFormat) && (srcFormat != dstFormat))
            {
                useComputeBlit = VK_TRUE;
            }
        }
    }

    if (useComputeBlit)
    {
        return (halti5_computeBlit(cmdBuf, srcRes, dstRes, rawCopy, gcvNULL, VK_FILTER_NEAREST));
    }

    /* 128BTILE feature, we need program the tilemode,the supertile is same with xmajor */

    srcConfig
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:0) - (0 ?
 20:0) + 1))))))) << (0 ?
 20:0))) | (((gctUINT32) ((gctUINT32) (srcStride) & ((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:0) - (0 ? 20:0) + 1))))))) << (0 ? 20:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (srcBltDesc.hwFormat) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (srcMsaa) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (srcTiling) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (srcSuperTile) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
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
        ;

    srcConfigEx
        =
#if __VK_ENABLETS
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (srcFastClear) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (srcCompression) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (srcCompressionFormat) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (color64) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)))
#else
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
#endif
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (srcBltDesc.sRGB) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
        | srcTileConfigEx
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (srcCacheMode) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
        ;

    dstConfig
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:0) - (0 ?
 20:0) + 1))))))) << (0 ?
 20:0))) | (((gctUINT32) ((gctUINT32) (dstStride) & ((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:0) - (0 ? 20:0) + 1))))))) << (0 ? 20:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (dstBltDesc.hwFormat) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (dstMsaa) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (dstTiling) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (dstSuperTile) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
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
        ;

    dstConfigEx
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (dstBltDesc.sRGB) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)))
        | dstTileConfigEx;

    /* Flush the pipe. */
    __VK_ONERROR(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

    __VK_ASSERT(srcParts == dstParts);
    for (partIdx = 0; partIdx < srcParts; ++partIdx)
    {
        uint32_t srcOriginX = srcOffset.x * srcSampleInfo.x;
        uint32_t srcOriginY = srcOffset.y * srcSampleInfo.y;
        uint32_t dstOriginX = dstOffset.x * dstSampleInfo.x;
        uint32_t dstOriginY = dstOffset.y * dstSampleInfo.y;
        uint32_t width      = srcExtent.width  * srcSampleInfo.x;
        uint32_t height     = srcExtent.height * srcSampleInfo.y;

        if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&(pCmdBuffer))++ = 0;
;
 } }__vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );;


        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5019, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (srcBltDesc.downSampleMode) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
            );

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5016, VK_FALSE, ~0U);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5017, VK_FALSE, ~0U);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5002, VK_FALSE, srcConfig);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5003, VK_FALSE, srcConfigEx);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, srcAddress);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5009, VK_FALSE, dstConfig);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500A, VK_FALSE, dstConfigEx);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, dstAddress);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502F, VK_FALSE,
            (srcBltDesc.bltSwizzleEx | (dstBltDesc.bltSwizzleEx << 12))
            );

        if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE,
                (srcAddress + srcPartSize - 1));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE,
                (dstAddress + dstPartSize - 1));
        }

        if (!forceSGPU && devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        {
            uint32_t i;
            uint32_t gpuCount = devCtx->chipInfo->gpuCoreCount;
            uint32_t averageW = __VK_ALIGN(width / gpuCount, srcSampleInfo.x);
            uint32_t splitWidth = width - (gpuCount - 1) * averageW;
            uint32_t srcStartX = srcOriginX;
            uint32_t dstStartX = dstOriginX;
            uint32_t leftWidth = width;

            for (i = 0; i < gpuCount; ++i)
            {
                /* Align srcEndX to 64 boundary */
                uint32_t srcEndX = __VK_ALIGN(srcStartX + splitWidth, 64);
                splitWidth = __VK_MIN(srcEndX - srcStartX, leftWidth);

                *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*&pCmdBuffer)++ = 0;
;



                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5005, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (srcStartX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (srcOriginY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    );

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (dstStartX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (dstOriginY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    );

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (splitWidth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                    );

                leftWidth -= splitWidth;
                srcStartX += splitWidth;
                dstStartX += (splitWidth / srcSampleInfo.x) * dstSampleInfo.x;
                splitWidth = averageW;
            }

            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

        }
        else
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5005, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (srcOriginX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (srcOriginY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (dstOriginX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (dstOriginY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );
        }
        /* Tilesize for customer config */
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5028, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))));

         /* block size for customer config*/
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5027, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x40) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (0x40) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))));
#if __VK_ENABLETS
        /* Set SrcTileStatusAddress. */
        if (srcFastClear)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5004, VK_FALSE, srcTileStatusAddress);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500D, VK_FALSE,
                srcTsResource->fcValue[srcRes->u.img.subRes.mipLevel][srcRes->u.img.subRes.arrayLayer]);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500E, VK_FALSE,
                srcTsResource->fcValueUpper[srcRes->u.img.subRes.mipLevel][srcRes->u.img.subRes.arrayLayer]);
        }
#endif
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
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
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&(pCmdBuffer))++ = 0;
;
 }halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 };

#if __VK_ENABLETS
        if (srcFastClear)
        {
            __VK_ASSERT(srcImg != VK_NULL_HANDLE);
            offset += srcPartSize;
            srcTileStatusAddress = halti5_computeTileStatusAddr(devCtx, srcImg, offset);;
        }
#endif
        srcAddress += srcPartSize;
        dstAddress += dstPartSize;
    }

    cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    if (cmd->curScrachBufIndex > 0)
    {
        __vk_CmdAquireBuffer(cmdBuf, cmd->curScrachBufIndex, &states);

        __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

        __vk_CmdReleaseBuffer(cmdBuf, cmd->curScrachBufIndex);
    }

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmd->curScrachBufIndex = 0;
    return result;
}

VkResult halti5_fillBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    uint32_t data
    )
{
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, buffer);
    const uint32_t stride = 4096;
    uint32_t config, dstConfigEx, srcConfigEx, address;
    uint32_t width, height, extraSize;
    uint32_t *states = gcvNULL;
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cmdBuf;
    __vkDevContext *devCtx = cmd->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkBool32 forceSGPU = VK_FALSE;

    if (size == VK_WHOLE_SIZE)
    {
        size = buf->memReq.size - offset;
    }

    if (size <= stride)
    {
        width  = (uint32_t)(size / sizeof(uint32_t));
        height = 1;
        extraSize = 0;
    }
    else
    {
        width  = stride / sizeof(uint32_t);
        height = (uint32_t)(size / stride);
        extraSize = (uint32_t)(size % stride);
    }

    __VK_ASSERT(((__vkCommandBuffer *)cmdBuf)->devCtx->database->REG_BltEngine);

    config
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:0) - (0 ?
 20:0) + 1))))))) << (0 ?
 20:0))) | (((gctUINT32) ((gctUINT32) (stride) & ((gctUINT32) ((((1 ?
 20:0) - (0 ?
 20:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:0) - (0 ? 20:0) + 1))))))) << (0 ? 20:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) (0x06 & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
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
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
        ;

    dstConfigEx
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
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
        ;

    srcConfigEx
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)))
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
        ;

    address = buf->memory->devAddr;
    address += (uint32_t)(buf->memOffset + offset);

    pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];

    /* Flush the pipe. */
    __VK_VERIFY_OK(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

    if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&(pCmdBuffer))++ = 0;
;
 } }__vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );;


    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5019, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:7) - (0 ?
 9:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:7) - (0 ?
 9:7) + 1))))))) << (0 ?
 9:7))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 9:7) - (0 ?
 9:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:7) - (0 ? 9:7) + 1))))))) << (0 ? 9:7)))
        );

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5009, VK_FALSE, config);
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500A, VK_FALSE, dstConfigEx);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5002, VK_FALSE, config);
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5003, VK_FALSE, srcConfigEx);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, address);
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, address);


    if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE,
            (address + (uint32_t)size - 1));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE,
            (address + (uint32_t)size - 1));
    }

    if (!forceSGPU && devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        uint32_t i;
        uint32_t gpuCount = devCtx->chipInfo->gpuCoreCount;
        uint32_t averageW = width / gpuCount;
        uint32_t splitWidth = width - (gpuCount - 1) * averageW;
        uint32_t dstStartX = 0;
        uint32_t leftWidth = width;

        for (i = 0; i < gpuCount; ++i)
        {
            /* Align dstEndX to 64 boundary */
            uint32_t dstEndX = __VK_ALIGN(dstStartX + splitWidth, 64);
            splitWidth = __VK_MIN(dstEndX - dstStartX, leftWidth);

            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*&pCmdBuffer)++ = 0;
;


            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (dstStartX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (splitWidth) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );

            leftWidth -= splitWidth;
            dstStartX += splitWidth;
            splitWidth = averageW;
        }

        *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

    }
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500B, VK_FALSE,
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
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (width) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (height) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5011, VK_FALSE, data);
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5013, VK_FALSE, 0xFFFFFFFF);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );

    if (extraSize > 0)
    {
        /* Move address to the extra size */
        address += (height * stride);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, address);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, address);

        if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE,
                (address + extraSize - 1));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE,
                (address + extraSize - 1));
        }

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x500C, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (extraSize / sizeof(uint32_t)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
            );
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
    }

    __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&(pCmdBuffer))++ = 0;
;
 }halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 };


    cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    if (cmd->curScrachBufIndex > 0)
    {
        __vk_CmdAquireBuffer(cmdBuf, cmd->curScrachBufIndex, &states);

        __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

        __vk_CmdReleaseBuffer(cmdBuf, cmd->curScrachBufIndex);
    }

    cmd->curScrachBufIndex = 0;

    return VK_SUCCESS;
}

VkResult halti5_copyBuffer(
    VkCommandBuffer cmdBuf,
     __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    uint64_t copySize
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cmdBuf;
    __vkDevContext *devCtx = cmd->devCtx;
    uint32_t *states = gcvNULL;
    VkResult result = VK_SUCCESS;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint64_t srcAddress;
    uint64_t dstAddress;
    VkBool32 forceSGPU = VK_FALSE;

    __VK_ASSERT(devCtx->database->REG_BltEngine);
    __VK_ASSERT(!srcRes->isImage && ! dstRes->isImage);

    srcAddress = srcRes->u.buf.pBuffer->memory->devAddr + srcRes->u.buf.pBuffer->memOffset
               + srcRes->u.buf.offset;
    dstAddress = dstRes->u.buf.pBuffer->memory->devAddr + dstRes->u.buf.pBuffer->memOffset
               + dstRes->u.buf.offset;

    pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];

    /* Flush the pipe. */
    __VK_VERIFY_OK(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

    if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&(pCmdBuffer))++ = 0;
;
 } }__vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );;


    if (!forceSGPU && devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        uint32_t i;
        uint32_t gpuCount = devCtx->chipInfo->gpuCoreCount;
        uint64_t averageSize = copySize / gpuCount;
        uint64_t copyBytes = copySize - (gpuCount - 1) * averageSize;
        uint64_t srcAddr = srcAddress;
        uint64_t dstAddr = dstAddress;

        for (i = 0; i < gpuCount; ++i)
        {
            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*&pCmdBuffer)++ = 0;
;


            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, (uint32_t)srcAddr);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, (uint32_t)dstAddr);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5015, VK_FALSE, (uint32_t)copyBytes);

            if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
            {
                uint64_t endAddress;

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

                endAddress = srcAddr + copyBytes - 1;
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE, (uint32_t)endAddress);

                endAddress = dstAddr + copyBytes - 1;
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE, (uint32_t)endAddress);
            }

            srcAddr += copyBytes;
            dstAddr += copyBytes;
            copyBytes = averageSize;
        }

        *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

    }
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, (uint32_t)srcAddress);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, (uint32_t)dstAddress);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5015, VK_FALSE, (uint32_t)copySize);

        if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
        {
            uint64_t endAddress;

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

            endAddress = srcAddress + copySize - 1;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE, (uint32_t)endAddress);

            endAddress = dstAddress + copySize - 1;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE, (uint32_t)endAddress);
        }
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );

    __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&(pCmdBuffer))++ = 0;
;
 }halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 };


    cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    if (cmd->curScrachBufIndex > 0)
    {
        __vk_CmdAquireBuffer(cmdBuf, cmd->curScrachBufIndex, &states);

        __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

        __vk_CmdReleaseBuffer(cmdBuf, cmd->curScrachBufIndex);
    }

    cmd->curScrachBufIndex = 0;

    return result;
}

VkResult halti5_updateBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    const uint32_t* pData
    )
{
    __vkCommandBuffer *pCmdBuf = (__vkCommandBuffer*)cmdBuf;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, buffer);
    __vkScratchMem *pScratch = gcvNULL;
    void *pSrcHost = gcvNULL;
    uint64_t srcAddress, dstAddress;
    uint32_t *states = gcvNULL;
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = pCmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkBool32 forceSGPU = VK_FALSE;

    /* Allocate scratch gpu memory and copy host data to it first */
    pScratch = __vkGetScratchMem(pCmdBuf, size);
    __VK_ONERROR(__vk_MapMemory((VkDevice)pCmdBuf->devCtx, (VkDeviceMemory)(uintptr_t)pScratch->memory,
                                0, size, 0, (void**)&pSrcHost));
    gcoOS_MemCopy(pSrcHost, pData, (gctSIZE_T)size);
    srcAddress = (uint64_t)pScratch->memory->devAddr;
    dstAddress = buf->memory->devAddr + buf->memOffset + offset;

    __VK_ASSERT(pCmdBuf->devCtx->database->REG_BltEngine);

    pCmdBuffer = pCmdBufferBegin = &pCmdBuf->scratchCmdBuffer[pCmdBuf->curScrachBufIndex];

    /* Flush the pipe. */
    __VK_VERIFY_OK(halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

    if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&(pCmdBuffer))++ = 0;
;
 } }__vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );;


    if (!forceSGPU && devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        uint32_t i;
        uint32_t gpuCount = devCtx->chipInfo->gpuCoreCount;
        uint64_t averageSize = size / gpuCount;
        uint64_t copyBytes = size - (gpuCount - 1) * averageSize;
        uint64_t srcAddr = srcAddress;
        uint64_t dstAddr = dstAddress;

        for (i = 0; i < gpuCount; ++i)
        {
            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*&pCmdBuffer)++ = 0;
;


            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, (uint32_t)srcAddr);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, (uint32_t)dstAddr);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5015, VK_FALSE, (uint32_t)copyBytes);

            if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
            {
                uint64_t endAddress;

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

                endAddress = srcAddr + copyBytes - 1;
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE, (uint32_t)endAddress);

                endAddress = dstAddr + copyBytes - 1;
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE, (uint32_t)endAddress);
            }

            srcAddr += copyBytes;
            dstAddr += copyBytes;
            copyBytes = averageSize;
        }
        *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

    }
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5000, VK_FALSE, (uint32_t)srcAddress);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5006, VK_FALSE, (uint32_t)dstAddress);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5015, VK_FALSE, (uint32_t)size);

        if (devCtx->database->ROBUSTNESS && devCtx->database->SH_ROBUSTNESS_FIX)
        {
            uint64_t endAddress;

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x006B, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))));

            endAddress = srcAddress + size - 1;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x503D, VK_FALSE, (uint32_t)endAddress);

            endAddress = dstAddress + size - 1;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x50CD, VK_FALSE, (uint32_t)endAddress);
        }
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5018, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
        );
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502B, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );

    __vkCmdLoadSingleHWState(&(pCmdBuffer), 0x502E, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) );if ((devCtx)->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE) {if (forceSGPU) {*(*&(pCmdBuffer))++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&(pCmdBuffer))++ = 0;
;
 }halti5_setMultiGpuSync((VkDevice)(devCtx), &(pCmdBuffer), VK_NULL_HANDLE);
 };


    pCmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(pCmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    if (pCmdBuf->curScrachBufIndex > 0)
    {
        __vk_CmdAquireBuffer(cmdBuf, pCmdBuf->curScrachBufIndex, &states);

        __VK_MEMCOPY(states, pCmdBuf->scratchCmdBuffer, pCmdBuf->curScrachBufIndex * sizeof(uint32_t));

        __vk_CmdReleaseBuffer(cmdBuf, pCmdBuf->curScrachBufIndex);
    }

OnError:
    if (pSrcHost)
    {
        __vk_UnmapMemory((VkDevice)pCmdBuf->devCtx, (VkDeviceMemory)(uintptr_t)pScratch->memory);
    }
    pCmdBuf->curScrachBufIndex = 0;
    return result;
}

uint32_t halti5_helper_convertHwTxSwizzle(
    const __vkFormatInfo * residentFormatInfo,
    VkComponentSwizzle swizzle,
    uint32_t currentSwizzle,
    const uint32_t hwComponentSwizzle[]
    )
{
    if (residentFormatInfo->partCount > 1)
    {
        swizzle = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

    switch (swizzle) {
    case VK_COMPONENT_SWIZZLE_IDENTITY:
        return currentSwizzle;
    case VK_COMPONENT_SWIZZLE_ZERO:
        return 0x4;
    case VK_COMPONENT_SWIZZLE_ONE:
        return 0x5;
    case VK_COMPONENT_SWIZZLE_R:
        return hwComponentSwizzle[0];
    case VK_COMPONENT_SWIZZLE_G:
        return hwComponentSwizzle[1];
    case VK_COMPONENT_SWIZZLE_B:
        return hwComponentSwizzle[2];
    case VK_COMPONENT_SWIZZLE_A:
        return hwComponentSwizzle[3];
    default:
        __VK_ASSERT(!"WRONG SWIZZLE");
        return 0x4;
    }
}

#define BGRA_PACKED_B 0
#define BGRA_PACKED_G 1
#define BGRA_PACKED_R 2
#define BGRA_PACKED_A 3

#define FLOAT_ONE    0x3F800000

static void halti5_helper_convertHwBorderColor(
    VkBorderColor borderColor,
    uint32_t HwFormat,
    VkBool32 integerFormat,
    VkBool32 sampleStencil,
    uint32_t partIndex,
    uint32_t gcregTXBorderColorBGRA32bit[],
    uint32_t *gcregTXBorderColorBGRA,
    VkFlags *invalidMask
    )
{
    switch (borderColor)
    {
    case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x0B:
                if (partIndex == 1)
                {
                    gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = FLOAT_ONE;
                }
                break;
            case 0x22:
                if (sampleStencil)
                {
                    *invalidMask |= (1 << VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
                }
            }
            break;
        default:
            break;
        }

        *gcregTXBorderColorBGRA = 0xFF000000;
        gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] = 0;
        gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = 0;
        gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = 0;
        gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
        break;

    case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x0A:
            case 0x07:
            case 0x0E:
            case 0x21:
            case 0x03:
            case 0x0D:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFFFF0000;
                break;
            case 0x0B:
            case 0x08:
            case 0x06:
            case 0x0F:
            case 0x04:
            case 0x05:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFFFFFF00;
                break;
            case 0x22:
                if (sampleStencil)
                {
                    *invalidMask |= (1 << VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
                }
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = 0x00FFFFFF;
                *gcregTXBorderColorBGRA = 0xFFFFFFFF;
                break;
            default:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = FLOAT_ONE;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFFFFFFFF;
                break;
            }
            break;

        case 0x10:
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = 0x0000FFFF;
            *gcregTXBorderColorBGRA = 0xFFFFFFFF;
            break;
        case 0x11:
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = 0x00FFFFFF;
            *gcregTXBorderColorBGRA = 0xFFFFFFFF;
            break;
        default:
            /* not legal combination */
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] = FLOAT_ONE;
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = FLOAT_ONE;
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = FLOAT_ONE;
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
            *gcregTXBorderColorBGRA = 0xFFFFFFFF;
            break;

        }
        break;

    case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x23:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 0;
                break;
            case 0x24:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 0;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = 0;
                break;
            case 0x22:
                if (!sampleStencil)
                {
                    *invalidMask |= (1 << VK_BORDER_COLOR_INT_OPAQUE_BLACK);
                }
                /* fall through */
            default:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] = 0;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] = 0;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] = 0;
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                break;
            }
            break;

        default:
            /* not legal combination */
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 0;
            break;

        }
        /* not useful for hw */
        *gcregTXBorderColorBGRA = 0xFF000000;
        if (!integerFormat)
        {
            *invalidMask |= (1 << VK_BORDER_COLOR_INT_OPAQUE_BLACK);
        }
        break;

    case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x23:
            case 0x15:
            case 0x18:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                *gcregTXBorderColorBGRA = 0xFFFF0000;
                break;
            case 0x24:
            case 0x16:
            case 0x19:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                *gcregTXBorderColorBGRA = 0xFFFFFF00;
                break;
            case 0x22:
                if (!sampleStencil)
                {
                    *invalidMask |= (1 << VK_BORDER_COLOR_INT_OPAQUE_WHITE);
                }
                /* fall through */
            default:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] =
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                *gcregTXBorderColorBGRA = 0xFFFFFFFF;
                break;
            }
            break;

        default:
            /* not legal combination */
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_B] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_G] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_R] =
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 0;
            *gcregTXBorderColorBGRA = 0xFFFFFFFF;
            break;

        }

        if (!integerFormat)
        {
            *invalidMask |= (1 << VK_BORDER_COLOR_INT_OPAQUE_WHITE);
        }
        break;

    case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x0A:
            case 0x07:
            case 0x0E:
            case 0x21:
            case 0x03:
            case 0x0D:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFF000000;
                break;
            case 0x0B:
            case 0x08:
            case 0x06:
            case 0x0F:
            case 0x04:
            case 0x05:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFF000000;
                break;
            case 0x1B:
            case 0x00:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
                *gcregTXBorderColorBGRA = 0xFF000000;
                break;
            }
            break;
        case 0x0B:
        case 0x1D:
            gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = FLOAT_ONE;
            *gcregTXBorderColorBGRA |= 0xFF000000;
            break;
        default:
            break;
        }
        break;

    case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
        switch ((HwFormat  >> TX_FORMAT_OLD_SHIFT) & 0xFF)
        {
        case 0:
            switch ((HwFormat >> TX_FORMAT_NEW_SHIFT) & 0xFF)
            {
            case 0x23:
            case 0x15:
            case 0x18:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                *gcregTXBorderColorBGRA32bit = 0xFF000000;
                break;
            case 0x24:
            case 0x16:
            case 0x19:
                gcregTXBorderColorBGRA32bit[BGRA_PACKED_A] = 1;
                *gcregTXBorderColorBGRA = 0xFF000000;
                break;
            }
        default:
            break;
        }
        break;

    default:
        __VK_ASSERT(!"invalid vk border color");
        break;
    }

    return;
}

VkResult halti5_helper_convertHwTxDesc(
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
    uint32_t tmpResidentImgFormat = 0;

    __VK_ASSERT(hwTxDesc);

    if (imgv)
    {
        img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
        subResourceRange = &imgv->createInfo.subresourceRange;
        baseLevel = &img->pImgLevels[subResourceRange->baseMipLevel];
        residentFormatInfo = (__vkFormatInfo *)imgv->formatInfo;
        tmpResidentImgFormat = residentFormatInfo->residentImgFormat;
        __VK_ASSERT(!bufv);

        if ((img->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
            (imgv->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
        {
            tmpResidentImgFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }

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

        uint32_t sizeInByte, texelSize;
        __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, bufv->createInfo.buffer);

        static __vkImageLevel fakedImageLevel = {0};
        static VkImageSubresourceRange fakedSubResourceRange;
        static const VkComponentMapping identityComponentMapping =
        {
            VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
        };

        __VK_ASSERT(!imgv);
        residentFormatInfo = &bufv->formatInfo;
        tmpResidentImgFormat = residentFormatInfo->residentImgFormat;

        sizeInByte = (bufv->createInfo.range == VK_WHOLE_SIZE)
                   ? (uint32_t)(buf->createInfo.size - bufv->createInfo.offset)
                   : (uint32_t)bufv->createInfo.range;

        texelSize = sizeInByte / (residentFormatInfo->bitsPerBlock >> 3);
        tiling = gcvLINEAR;
        if (texelSize <= __VK_FAKED_TEX_MAX_WIDTH)
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
        fakedImageLevel.requestD = texelSize;
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

    hwTxFmtInfo = halti5_helper_convertHwTxInfo(devCtx, tmpResidentImgFormat);
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
    astcImage = ((((((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT))) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) ) == 0x14) ?
        VK_TRUE : VK_FALSE;

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
        VkMemoryAllocateInfo mem_alloc;
        uint32_t *texDesc;
        gcsTEXTUREDESCRIPTORREGS *hwDescriptor;
        uint32_t levelCount = subResourceRange->levelCount;
        uint32_t layerCount = subResourceRange->layerCount;

        /* Allocate device memory for texture descriptor */
        __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.allocationSize = hwDescriptorSize;
        mem_alloc.memoryTypeIndex = 0;
        __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &mem_alloc, VK_NULL_HANDLE, &hwTxDesc[partIdx].descriptor));
        __VK_ONERROR(__vk_MapMemory((VkDevice)devCtx, hwTxDesc[partIdx].descriptor, 0, hwDescriptorSize, 0, (void **)&texDesc));
        __VK_MEMZERO(texDesc, hwDescriptorSize);

        hwDescriptor = (gcsTEXTUREDESCRIPTORREGS *)texDesc;

        hwTxDesc[partIdx].sRGB = (hwTxFmtInfo->hwFormat >> TX_FORMAT_SRGB_SHIFT) & 0x1;
        hwTxDesc[partIdx].fast_filter = ((hwTxFmtInfo->hwFormat >> TX_FORMAT_FAST_FILTER_SHIFT) & 0x1)
                                      && (viewType != VK_IMAGE_VIEW_TYPE_3D) && (viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY);
        hwTxDesc[partIdx].sampleStencil = (aspectFlag & VK_IMAGE_ASPECT_STENCIL_BIT) ? VK_TRUE : VK_FALSE;
        hwTxDesc[partIdx].msaaImage =  msaaImage;
        hwTxDesc[partIdx].isCubmap = (viewType == VK_IMAGE_VIEW_TYPE_CUBE) || (viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);

        hwTxDesc[partIdx].baseWidth  = baseLevel->requestW;
        hwTxDesc[partIdx].baseHeight = baseLevel->requestH;
        hwTxDesc[partIdx].baseDepth  = baseLevel->requestD;
        hwTxDesc[partIdx].baseSlice  = (uint32_t)(baseLevel->sliceSize) / (residentFormatInfo->bitsPerBlock >> 3);

        if (subResourceRange->levelCount == VK_REMAINING_MIP_LEVELS)
        {
            levelCount = img->createInfo.mipLevels - subResourceRange->baseMipLevel;
        }

        if (subResourceRange->layerCount == VK_REMAINING_ARRAY_LAYERS)
        {
            layerCount = img->createInfo.arrayLayers - subResourceRange->baseArrayLayer;
        }

        if (imgv)
        {
            uint32_t levelIdx;
            __VK_ASSERT(img);
            for (levelIdx = 0; levelIdx < levelCount; ++levelIdx)
            {
                __vkImageLevel *level = &img->pImgLevels[subResourceRange->baseMipLevel + levelIdx];
                uint32_t physical = 0;

                physical = img->memory->devAddr;
                physical += (uint32_t)(img->memOffset
                                     + partIdx * level->partSize
                                     + level->offset
                                     + subResourceRange->baseArrayLayer * level->sliceSize);
                hwDescriptor->gcregTXAddress[levelIdx] = physical;
            }
        }
        else
        {
            uint32_t physical = 0;

            __VK_ASSERT(bufv);
            __VK_ASSERT(partIdx == 0);

            physical = resourceMemory->devAddr;
            physical += (uint32_t)(offsetInResourceMemory + baseLevel->offset);
            hwDescriptor->gcregTXAddress[0] = physical;
        }

        hwDescriptor->gcregTXConfig =
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

        hwDescriptor->gcregTXSize =
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

        if (viewType >= VK_IMAGE_VIEW_TYPE_1D_ARRAY)
        {
            hwDescriptor->gcregTX3D = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:0) - (0 ?
 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (layerCount) & ((gctUINT32) ((((1 ?
 13:0) - (0 ?
 13:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ? 13:0)));
        }
        else
        {
            hwDescriptor->gcregTX3D = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

        hwDescriptor->gcregTXLinearStride =
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
        hwDescriptor->gcregTXExtConfig =
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

        hwDescriptor->gcregTXBaseLOD =
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

        hwDescriptor->gcregTXConfig2 = txConfig2;

        hwDescriptor->gcregTXConfig3 = ((((gctUINT32) (0x00000000)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

        hwDescriptor->gcregTXSizeExt =
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

        hwDescriptor->gcregTXVolumeExt =
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

        hwDescriptor->gcregTXSlice = (uint32_t)baseLevel->sliceSize;

        hwDescriptor->gcregTXLogSize = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:27) - (0 ?
 28:27) + 1))))))) << (0 ?
 28:27))) | (((gctUINT32) ((gctUINT32) (astcImage ?
 2 : 0) & ((gctUINT32) ((((1 ?
 28:27) - (0 ?
 28:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)));

        if (astcImage)
        {
            uint32_t astcSize = (imgv->createInfo.format - VK_FORMAT_ASTC_4x4_UNORM_BLOCK) / 2;
            uint32_t astcSrgb = hwTxDesc[partIdx].sRGB;

            hwDescriptor->gcregTXASTC0Ex =
            hwDescriptor->gcregTXASTC1Ex =
            hwDescriptor->gcregTXASTC2Ex =
            hwDescriptor->gcregTXASTC3Ex = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

        if (imgv)
        {
            VkBorderColor j;
            gcsTEXTUREDESCRIPTORREGS *nextHwDescriptor;
            VkBool32 integer = (residentFormatInfo->category == __VK_FMT_CATEGORY_UINT || residentFormatInfo->category == __VK_FMT_CATEGORY_SINT) ? VK_TRUE : VK_FALSE;

            for (j = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK; j < VK_BORDER_COLOR_RANGE_SIZE; j = (VkBorderColor)(j+1))
            {
                nextHwDescriptor = (gcsTEXTUREDESCRIPTORREGS*)((uint8_t *)texDesc + TX_HW_DESCRIPTOR_MEM_SIZE * j);
                __VK_MEMCOPY(nextHwDescriptor, hwDescriptor, sizeof(gcsTEXTUREDESCRIPTORREGS));

                halti5_helper_convertHwBorderColor(j,
                    hwTxFmtInfo->hwFormat,
                    integer,
                    hwTxDesc[partIdx].sampleStencil,
                    partIdx,
                    &nextHwDescriptor->gcregTXBorderColorBlue32,
                    &nextHwDescriptor->gcregTXBorderColor,
                    &hwTxDesc[partIdx].invalidBorderColorMask);
            }
        }

        __vk_UnmapMemory((VkDevice)devCtx, hwTxDesc[partIdx].descriptor);
    }

    return VK_SUCCESS;

OnError:
    for (partIdx = 0; partIdx < partCount; partIdx++)
    {
        if (hwTxDesc[partIdx].descriptor)
        {
            __vk_FreeMemory((VkDevice)devCtx, hwTxDesc[partIdx].descriptor, VK_NULL_HANDLE);
            hwTxDesc[partIdx].descriptor = VK_NULL_HANDLE;
        }
    }
    return result;

}

VkResult halti5_helper_convertHwImgDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    VkExtent3D *userSize,
    HwImgDesc *hwImgDesc
    )
{
    __vkFormatInfo *residentFormatInfo = VK_NULL_HANDLE;
    uint32_t imageDesc = 0;
    uint32_t physical = 0;
    gceTILING tiling = gcvLINEAR;
    uint32_t stride = 0;
    uint32_t width = 0, height = 0;
    VkBool32 extraPart = VK_FALSE;
    uint32_t partSize = 0;
    uint32_t tmpResidentImgFormat = 0;

    __VK_ASSERT(hwImgDesc);

    if (imgv)
    {
        __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
        VkImageSubresourceRange *subResourceRange = &imgv->createInfo.subresourceRange;
        __vkImageLevel *baseLevel = &img->pImgLevels[subResourceRange->baseMipLevel];

        __VK_ASSERT(!bufv);
        physical = img->memory->devAddr;
        physical += (uint32_t)(img->memOffset + baseLevel->offset +
                               subResourceRange->baseArrayLayer * baseLevel->sliceSize);

        residentFormatInfo = (__vkFormatInfo*)imgv->formatInfo;
        tmpResidentImgFormat = residentFormatInfo->residentImgFormat;

        if (imgv->formatInfo->compressed)
        {
            tiling = gcvLINEAR;
        }
        else
        {
            tiling = img->halTiling;
        }

        if ((img->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
            (imgv->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
        {
            tmpResidentImgFormat = VK_FORMAT_R8G8B8A8_UNORM;
        }

        width = userSize ? userSize->width : baseLevel->requestW;
        height = userSize ? userSize->height : baseLevel->requestH;
        stride = (uint32_t)baseLevel->stride;
        extraPart = (residentFormatInfo->partCount > 1);
        partSize = (uint32_t)baseLevel->partSize;
    }
    else if (bufv)
    {
#define __VK_FAKED_IMG_MAX_WIDTH 8192
        __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, bufv->createInfo.buffer);

        __VK_ASSERT(!imgv);

        physical = buf->memory->devAddr;
        physical += (uint32_t)(buf->memOffset + bufv->createInfo.offset);

        residentFormatInfo = &bufv->formatInfo;
        tmpResidentImgFormat = residentFormatInfo->residentImgFormat;
        tiling = gcvLINEAR;
        if (userSize)
        {
            width  = userSize->width;
            height = userSize->height;
        }
        else
        {
            uint32_t sizeInByte = (bufv->createInfo.range == VK_WHOLE_SIZE)
                                ? (uint32_t)(buf->createInfo.size - bufv->createInfo.offset)
                                : (uint32_t)bufv->createInfo.range;

            uint32_t texelSize = sizeInByte / (residentFormatInfo->bitsPerBlock >> 3);

            if (texelSize <= __VK_FAKED_IMG_MAX_WIDTH)
            {
                width = texelSize;
                height = 1;
            }
            else
            {
                width = __VK_FAKED_IMG_MAX_WIDTH;
                height = (uint32_t)gcoMATH_Ceiling(((float)texelSize / __VK_FAKED_IMG_MAX_WIDTH));
            }
        }
        stride = (uint32_t)(width / residentFormatInfo->blockSize.width) * residentFormatInfo->bitsPerBlock / 8;

        hwImgDesc[0].baseWidth = width;
        hwImgDesc[0].baseHeight = height;
        hwImgDesc[0].baseDepth = 1;
        hwImgDesc[0].baseSlice = stride * height;
    }
    else
    {
        __VK_ASSERT(!"Must have one view to generate image descriptor");
    }

    switch (tmpResidentImgFormat)
    {
    case VK_FORMAT_R8_UNORM:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R8_SNORM:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xC & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R8_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R8G8_UNORM:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R8G8_SNORM:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xC & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R8G8_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8G8_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case __VK_FORMAT_R8_1_X8R8G8B8:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xC & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
   case VK_FORMAT_R16_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16_UNORM:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xE & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16B16A16_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16B16A16_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R16G16B16A16_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32_UINT:
    case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32_SINT:
    case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32_SFLOAT:
    case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32B32A32_UINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32B32A32_SINT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    case VK_FORMAT_R32G32B32A32_SFLOAT:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x9 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x5 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0x8 & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xA & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;
    case __VK_FORMAT_D24_UNORM_X8_PACKED32:
        imageDesc = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) (0xF & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)));
        break;

    default:
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    switch (tiling)
    {
    case gcvLINEAR:
        imageDesc |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)));
        break;
    case gcvSUPERTILED:
        imageDesc |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)));
        break;
    default:
        __VK_ASSERT(!"Wrong tiling");
    }

    imageDesc |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));

    hwImgDesc[0].imageInfo[0] = physical;
    hwImgDesc[0].imageInfo[1] = stride;
    hwImgDesc[0].imageInfo[2] = width | (height << 16);
    hwImgDesc[0].imageInfo[3] = imageDesc;

    if (extraPart)
    {
        hwImgDesc[1].imageInfo[0] = physical + partSize;
        hwImgDesc[1].imageInfo[1] = stride;
        hwImgDesc[1].imageInfo[2] = width | (height << 16);
        hwImgDesc[1].imageInfo[3] = imageDesc;
    }

    return VK_SUCCESS;
}

void halti5_helper_convertHwSampler(
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
    int32_t  lodbias = 0, minlod = 0, maxlod = 0;

    __VK_ASSERT(devCtx->database->REG_TX6bitFrac);

    /* Convert to 5.8 format  */
    lodbias = _Float2SignedFixed(createInfo->mipLodBias, 8, 8);
    maxlod = _Float2SignedFixed(createInfo->maxLod, 5, 8);
    minlod = _Float2SignedFixed(__VK_MAX(0.0f, createInfo->minLod), 5, 8);

    hwSamplerDesc->halti5.hwSamplerCtrl0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeU]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:3) - (0 ?
 5:3) + 1))))))) << (0 ?
 5:3))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeV]) & ((gctUINT32) ((((1 ?
 5:3) - (0 ?
 5:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:3) - (0 ? 5:3) + 1))))))) << (0 ? 5:3)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:6) - (0 ?
 8:6) + 1))))))) << (0 ?
 8:6))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeW]) & ((gctUINT32) ((((1 ?
 8:6) - (0 ?
 8:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:6) - (0 ? 8:6) + 1))))))) << (0 ? 8:6)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:9) - (0 ?
 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) ((gctUINT32) (s_minXlate[createInfo->minFilter]) & ((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ? 10:9)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:11) - (0 ?
 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) ((gctUINT32) (s_mipXlate[createInfo->mipmapMode]) & ((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ? 12:11)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:13) - (0 ?
 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) ((gctUINT32) (s_magXlate[createInfo->magFilter]) & ((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ? 14:13)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (createInfo->compareEnable ?
 1 : 0) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:18) - (0 ?
 20:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:18) - (0 ?
 20:18) + 1))))))) << (0 ?
 20:18))) | (((gctUINT32) ((gctUINT32) (s_funcXlate[createInfo->compareOp]) & ((gctUINT32) ((((1 ?
 20:18) - (0 ?
 20:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:18) - (0 ? 20:18) + 1))))))) << (0 ? 20:18)))
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:21) - (0 ?
 22:21) + 1))))))) << (0 ?
 22:21))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:21) - (0 ?
 22:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:21) - (0 ? 22:21) + 1))))))) << (0 ? 22:21)));

    hwSamplerDesc->halti5.hwSamplerLodMinMax = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

    hwSamplerDesc->halti5.hwSamplerLodBias = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (lodbias) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
        |  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (lodbias != 0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));

    if (createInfo->anisotropyEnable)
    {
        __VK_ASSERT(createInfo->maxAnisotropy <= devCtx->pPhyDevice->phyDevProp.limits.maxSamplerAnisotropy);
        hwSamplerDesc->halti5.hwSamplerAnisVal = __vk_UtilLog2inXdot8(gcoMATH_Float2UInt(createInfo->maxAnisotropy));
    }

    return;
}


VkResult halti5_createSampler(
    VkDevice device,
    VkSampler sampler
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    __vkSampler *spl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, sampler);
    VkSamplerCreateInfo *createInfo = &spl->createInfo;
    halti5_sampler *chipSampler;
    VkResult result = VK_SUCCESS;
    halti5_patch_key patchKey = 0;
    __VK_SET_ALLOCATIONCB(&spl->memCb);

    chipSampler = (halti5_sampler *)__VK_ALLOC(
        sizeof(halti5_sampler), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR(chipSampler ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(chipSampler, sizeof(halti5_sampler));

    (*chipModule->minorTable.helper_convertHwSampler)(devCtx, createInfo, &chipSampler->samplerDesc);

    if (createInfo->unnormalizedCoordinates)
    {
        patchKey |= HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
    }

    chipSampler->patchKey = patchKey;

    spl->chipPriv = chipSampler;

    return VK_SUCCESS;

OnError:
    return result;
}


VkResult halti5_createImageView(
    VkDevice device,
    VkImageView imageView
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    __vkImageView *imgv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, imageView);
    VkResult result = VK_SUCCESS;
    halti5_imageView *chipImgv;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
    const VkFormatProperties *formatProps = &imgv->formatInfo->formatProperties;
    VkFormatFeatureFlags formatFeatureFlags = (img->createInfo.tiling == VK_IMAGE_TILING_LINEAR) ?
        formatProps->linearTilingFeatures : formatProps->optimalTilingFeatures;
    uint32_t residentImgFormat = imgv->formatInfo->residentImgFormat;

    __VK_SET_ALLOCATIONCB(&imgv->memCb);

    chipImgv = (halti5_imageView *)__VK_ALLOC(
        sizeof(halti5_imageView), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    if (!chipImgv)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    __VK_MEMZERO(chipImgv, sizeof(halti5_imageView));

    if ((img->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
        (imgv->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
    {
        residentImgFormat = VK_FORMAT_R8G8B8A8_UNORM;
    }

    if ((img->createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        || ((img->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            && (formatFeatureFlags & VK_FORMAT_FEATURE_BLIT_SRC_BIT)))
    {
        (*chipModule->minorTable.helper_convertHwTxDesc)(devCtx, imgv, VK_NULL_HANDLE, chipImgv->txDesc);
    }

    if ((img->createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        || ((img->createInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
           && (formatFeatureFlags & VK_FORMAT_FEATURE_BLIT_DST_BIT)))
    {
        __VK_ONERROR(halti5_helper_convertHwPEDesc(residentImgFormat, &chipImgv->peDesc));
    }

    if (img->createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT)
    {
        __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgv, VK_NULL_HANDLE, gcvNULL, chipImgv->imgDesc));
    }

    if (img->createInfo.usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
    {
        if (img->createInfo.samples > VK_SAMPLE_COUNT_1_BIT)
        {
            (*chipModule->minorTable.helper_convertHwTxDesc)(devCtx, imgv, VK_NULL_HANDLE, chipImgv->txDesc);
        }
        else
        {
            switch (img->createInfo.format)
            {
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                /* Create Hw texture descriptor for all unsupported formats */
                {
                    static const VkSamplerCreateInfo s_SamplerCreateInfo =
                    {
                        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                        VK_NULL_HANDLE,
                        0,
                        VK_FILTER_NEAREST, VK_FILTER_NEAREST,
                        (VkSamplerMipmapMode)__VK_SAMPLER_MIPMAP_MODE_NONE,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                        0.0,
                        VK_FALSE,
                        1.0,
                        VK_FALSE,
                        VK_COMPARE_OP_ALWAYS,
                        -0.1f,
                        0.1f,
                        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                        VK_FALSE
                    };
                    (*chipModule->minorTable.helper_convertHwSampler)(devCtx, &s_SamplerCreateInfo, &chipImgv->samplerDesc);
                    if (!chipImgv->txDesc[0].descriptor)
                    {
                        (*chipModule->minorTable.helper_convertHwTxDesc)(devCtx, imgv, VK_NULL_HANDLE, chipImgv->txDesc);
                    }
                }
                break;
            default:
                __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, imgv, VK_NULL_HANDLE, gcvNULL, chipImgv->imgDesc));
                break;
            }
        }
    }

    switch (residentImgFormat)
    {
    case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
    case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
    case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
        chipImgv->patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT | HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT;
        /* fall through */
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
        chipImgv->patchFormat = residentImgFormat;
        chipImgv->patchKey |= HALTI5_PATCH_TX_GATHER_BIT;
        break;
    case VK_FORMAT_D32_SFLOAT:
        chipImgv->patchFormat = residentImgFormat;
        chipImgv->patchKey |= HALTI5_PATCH_TX_GATHER_PCF_BIT | HALTI5_PATCH_TX_GATHER_BIT;
        break;
    default:
        break;
    }

    imgv->chipPriv = chipImgv;

    return VK_SUCCESS;
OnError:
    if (chipImgv)
    {
        __VK_FREE(chipImgv);
    }
    return result;
}

VkResult halti5_destroyImageView(
    VkDevice device,
    VkImageView imageView
    )
{
    __vkImageView *imgv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, imageView);
    halti5_imageView *chipImgv;

    __VK_SET_ALLOCATIONCB(&imgv->memCb);

    if (imgv->chipPriv)
    {
        uint32_t partIdx;
        chipImgv = (halti5_imageView *)imgv->chipPriv;
        for (partIdx = 0; partIdx < __VK_MAX_PARTS; partIdx++)
        {
            if (chipImgv->txDesc[partIdx].descriptor)
            {
                __vk_FreeMemory(device, chipImgv->txDesc[partIdx].descriptor, VK_NULL_HANDLE);
            }
        }
        __VK_FREE(chipImgv);
    }

    return VK_SUCCESS;
}

VkResult halti5_createBufferView(
    VkDevice device,
    VkBufferView bufferView
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    __vkBufferView *bufv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBufferView *, bufferView);
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, bufv->createInfo.buffer);
    halti5_bufferView *chipBufv;
    VkResult result;
    __VK_SET_ALLOCATIONCB(&bufv->memCb);

    chipBufv = (halti5_bufferView *)__VK_ALLOC(
        sizeof(halti5_bufferView), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    if (!chipBufv)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    __VK_MEMZERO(chipBufv, sizeof(halti5_bufferView));

    if (buf->createInfo.usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
    {
        __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, VK_NULL_HANDLE, bufv, gcvNULL, chipBufv->imgDesc));
    }

    if (buf->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
    {
        switch (bufv->createInfo.format)
        {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
            chipBufv->patchFormat = bufv->createInfo.format;
            __VK_ONERROR(halti5_helper_convertHwImgDesc(devCtx, VK_NULL_HANDLE, bufv, gcvNULL, chipBufv->imgDesc));
            chipBufv->patchKey |= HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD_BIT;
            break;
        default:
            /* Create Hx texture descriptor for all supported formats */
            {
                static const VkSamplerCreateInfo s_SamplerCreateInfo =
                {
                    VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    VK_NULL_HANDLE,
                    0,
                    VK_FILTER_NEAREST, VK_FILTER_NEAREST,
                    (VkSamplerMipmapMode)__VK_SAMPLER_MIPMAP_MODE_NONE,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                    0.0,
                    VK_FALSE,
                    1.0,
                    VK_FALSE,
                    VK_COMPARE_OP_ALWAYS,
                    -0.1f,
                     0.1f,
                    VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                    VK_FALSE
                };
                (*chipModule->minorTable.helper_convertHwSampler)(devCtx, &s_SamplerCreateInfo, &chipBufv->samplerDesc);
                (*chipModule->minorTable.helper_convertHwTxDesc)(devCtx, VK_NULL_HANDLE, bufv, chipBufv->txDesc);
            }
            break;
        }
    }

    bufv->chipPriv = chipBufv;

    return VK_SUCCESS;

OnError:
    if (chipBufv)
    {
        __VK_FREE(chipBufv);
    }
    return result;
}

VkResult halti5_destroyBufferView(
    VkDevice device,
    VkBufferView bufferView
    )
{
    __vkBufferView *bufv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBufferView *, bufferView);

    __VK_SET_ALLOCATIONCB(&bufv->memCb);

    if (bufv->chipPriv)
    {
        halti5_bufferView *chipBufv;
        chipBufv = (halti5_bufferView *)bufv->chipPriv;

        if (chipBufv->txDesc[0].descriptor)
        {
            __vk_FreeMemory(device, chipBufv->txDesc[0].descriptor, VK_NULL_HANDLE);
        }

        __VK_FREE(chipBufv);
    }

    return VK_SUCCESS;
}

VkResult halti5_allocDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    )
{
    VkResult result = VK_SUCCESS;
    halti5_descriptorSet *chipDescSet;
    __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet*, descriptorSet);
    uint32_t numofEntries = descSet->descSetLayout->samplerDescriptorCount
                          + descSet->descSetLayout->samplerBufferDescriptorCount
                          + descSet->descSetLayout->inputAttachmentDescriptorCount;
    size_t bytes = __VK_ALIGN(sizeof(halti5_descriptorSet), 8)
                 + __VK_ALIGN(sizeof(halti5_patch_key)  * numofEntries, 8)
                 + __VK_ALIGN(sizeof(halti5_patch_info) * numofEntries, 8);

    __VK_SET_ALLOCATIONCB(&descSet->descriptorPool->memCb);

    chipDescSet = (halti5_descriptorSet*)__VK_ALLOC(bytes, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR(chipDescSet ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(chipDescSet, bytes);

    if (numofEntries)
    {
        chipDescSet->patchKeys  = (halti5_patch_key*)((uint8_t*)chipDescSet +
                                                       __VK_ALIGN(sizeof(halti5_descriptorSet), 8));
        chipDescSet->patchInfos = (halti5_patch_info*)((uint8_t*)chipDescSet->patchKeys +
                                                       __VK_ALIGN(sizeof(halti5_patch_key) * numofEntries, 8));
    }
    chipDescSet->numofEntries = numofEntries;

    descSet->chipPriv = chipDescSet;

    return VK_SUCCESS;

OnError:
    if (chipDescSet)
    {
        __VK_FREE(chipDescSet);
    }
    return result;
}

VkResult halti5_freeDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    )
{
    __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet*, descriptorSet);
    halti5_descriptorSet *chipDescSet = (halti5_descriptorSet*)descSet->chipPriv;
    __VK_SET_ALLOCATIONCB(&descSet->descriptorPool->memCb);

    if (chipDescSet)
    {
        __VK_FREE(chipDescSet);
        descSet->chipPriv = VK_NULL_HANDLE;
    }
    return VK_SUCCESS;
}

static SwizzleComponent __vkConvertSwizzle(
    VkComponentSwizzle vkComponentSwizzle,
    SwizzleComponent defaultSwizzle
    )
{
    switch (vkComponentSwizzle)
    {
    case VK_COMPONENT_SWIZZLE_IDENTITY:
        return defaultSwizzle;
    case VK_COMPONENT_SWIZZLE_ZERO:
        return SWIZZLE_ZERO;
    case VK_COMPONENT_SWIZZLE_ONE:
        return SWIZZLE_ONE;
    case VK_COMPONENT_SWIZZLE_R:
        return SWIZZLE_RED;
    case VK_COMPONENT_SWIZZLE_G:
        return SWIZZLE_GREEN;
    case VK_COMPONENT_SWIZZLE_B:
        return SWIZZLE_BLUE;
    case VK_COMPONENT_SWIZZLE_A:
        return SWIZZLE_ALPHA;
    default:
        return defaultSwizzle;
    }

}

const char * halti5_helper_patchFuc(
    uint32_t patchFormat,
    uint32_t patchType,
    VkImageViewType viewType,
    VSC_RES_OP_BIT *pOpTypeBits,
    VSC_RES_ACT_BIT *pActBits,
    uint32_t *pSubType
    )
{
    static const struct
    {
        uint32_t patchFormat;
        uint32_t patchType;
        uint32_t viewTypeBits;
        const char *patchFunc;
        VSC_RES_OP_BIT  opTypeBits; /* What resource operation we need apply this function link */
        VSC_RES_ACT_BIT actBits;    /* For this link, what extra action we need compiler to prepare before that */
        uint32_t subType;
    }
    s_patchEntries[] =
    {
        {
            __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT,
            HALTI5_PATCH_PE_EXTRA_OUTPUT,
            0,
            "_outputcvt_R32G32B32A32SFLOAT_2_R32G32SFLOAT",
            0,
            0,
            0
        },
        {
            __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
            HALTI5_PATCH_PE_EXTRA_OUTPUT,
            0,
            "_outputcvt_R32G32B32A32SINT_2_R32G32SINT",
            0,
            0,
            0
        },
        {
            __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
            HALTI5_PATCH_PE_EXTRA_OUTPUT,
            0,
            "_outputcvt_R32G32B32A32UINT_2_R32G32UINT",
            0,
            0,
            0
        },

        {
            __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_1D_BIT | __VK_IMAGE_VIEW_TYPE_2D_BIT | __VK_IMAGE_VIEW_TYPE_3D_BIT | __VK_IMAGE_VIEW_TYPE_CUBE_BIT
          | __VK_IMAGE_VIEW_TYPE_1D_ARRAY_BIT | __VK_IMAGE_VIEW_TYPE_2D_ARRAY_BIT,
            "_inputcvt_R32G32B32A32SFLOAT_2_R32G32SFLOAT",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD
          | VSC_RES_OP_BIT_TEXLDP | VSC_RES_OP_BIT_TEXLDP_BIAS | VSC_RES_OP_BIT_TEXLDP_LOD | VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS,
          VSC_RES_ACT_BIT_EXTRA_SAMPLER,
          VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER

        },
        {
            __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_1D_BIT | __VK_IMAGE_VIEW_TYPE_2D_BIT | __VK_IMAGE_VIEW_TYPE_3D_BIT | __VK_IMAGE_VIEW_TYPE_CUBE_BIT
          | __VK_IMAGE_VIEW_TYPE_1D_ARRAY_BIT | __VK_IMAGE_VIEW_TYPE_2D_ARRAY_BIT,
            "_inputcvt_R32G32B32A32SINT_2_R32G32SINT",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD
          | VSC_RES_OP_BIT_TEXLDP | VSC_RES_OP_BIT_TEXLDP_BIAS | VSC_RES_OP_BIT_TEXLDP_LOD | VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS,
          VSC_RES_ACT_BIT_EXTRA_SAMPLER,
          VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER
        },
        {
            __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_1D_BIT | __VK_IMAGE_VIEW_TYPE_2D_BIT | __VK_IMAGE_VIEW_TYPE_3D_BIT | __VK_IMAGE_VIEW_TYPE_CUBE_BIT
          | __VK_IMAGE_VIEW_TYPE_1D_ARRAY_BIT | __VK_IMAGE_VIEW_TYPE_2D_ARRAY_BIT,
            "_inputcvt_R32G32B32A32UINT_2_R32G32UINT",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD
          | VSC_RES_OP_BIT_TEXLDP | VSC_RES_OP_BIT_TEXLDP_BIAS | VSC_RES_OP_BIT_TEXLDP_LOD | VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS,
          VSC_RES_ACT_BIT_EXTRA_SAMPLER,
          VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER
        },

        {
            __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_CUBE_ARRAY_BIT,
            "_inputcvt_R32G32B32A32SFLOAT_2_R32G32SFLOAT_cubeArray",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER
        },
        {
            __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_CUBE_ARRAY_BIT,
            "_inputcvt_R32G32B32A32SINT_2_R32G32SINT_cubeArray",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER
        },
        {
            __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
            HALTI5_PATCH_TX_EXTRA_INPUT,
            __VK_IMAGE_VIEW_TYPE_CUBE_ARRAY_BIT,
            "_inputcvt_R32G32B32A32UINT_2_R32G32UINT_cubeArray",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER
        },


        {
            VK_FORMAT_R32G32B32A32_SFLOAT,
            HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD,
            0,
            "_texld_with_imgld_R32G32B32A32SFLOAT",
            VSC_RES_OP_BIT_FETCH,
            VSC_RES_ACT_BIT_REPLACE_SAMPLER_WITH_IMAGE_UNIFORM,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXFETCH_REPLACE_WITH_IMGLD

        },
        {
            VK_FORMAT_R32G32B32A32_SINT,
            HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD,
            0,
            "_texld_with_imgld_R32G32B32A32SINT",
            VSC_RES_OP_BIT_FETCH,
            VSC_RES_ACT_BIT_REPLACE_SAMPLER_WITH_IMAGE_UNIFORM,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXFETCH_REPLACE_WITH_IMGLD
        },
        {
            VK_FORMAT_R32G32B32A32_UINT,
            HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD,
            0,
            "_texld_with_imgld_R32G32B32A32UINT",
            VSC_RES_OP_BIT_FETCH,
            VSC_RES_ACT_BIT_REPLACE_SAMPLER_WITH_IMAGE_UNIFORM,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXFETCH_REPLACE_WITH_IMGLD
        },

        {
            __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32B32A32SFLOAT_2_R32G32SFLOAT",
            VSC_RES_OP_BIT_GATHER,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER

        },
        {
            __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32B32A32SINT_2_R32G32SINT",
            VSC_RES_OP_BIT_GATHER,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32B32A32UINT_2_R32G32UINT",
            VSC_RES_OP_BIT_GATHER,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },

        {
            VK_FORMAT_R32G32_SFLOAT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32SFLOAT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            VK_FORMAT_R32G32_SINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32SINT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
         },
        {
            VK_FORMAT_R32G32_UINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32G32UINT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },

        {
            VK_FORMAT_R32_SFLOAT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32SFLOAT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            VK_FORMAT_R32_SINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32SINT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            VK_FORMAT_R32_UINT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_R32UINT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            VK_FORMAT_D32_SFLOAT,
            HALTI5_PATCH_TX_GATHER,
            0,
            "_inputgather_D32SFLOAT",
            VSC_RES_OP_BIT_GATHER,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER
        },
        {
            VK_FORMAT_D32_SFLOAT,
            HALTI5_PATCH_TX_GATHER_PCF,
            0,
            "_inputgather_pcf_D32SFLOAT",
            VSC_RES_OP_BIT_GATHER_PCF,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHERPCF_D32F
        },

        {
            VK_FORMAT_UNDEFINED,
            HALTI5_PATCH_UNORMALIZED_SAMPLER,
            0,
            "_unormalizeCoord",
            VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLD_BIAS | VSC_RES_OP_BIT_TEXLD_LOD
          | VSC_RES_OP_BIT_TEXLDP | VSC_RES_OP_BIT_TEXLDP_BIAS | VSC_RES_OP_BIT_TEXLDP_LOD,
            0,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_NORMALIZE_TEXCOORD
        },

        {
            __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT,
            HALTI5_PATCH_TX_EXTRA_INPUT_GRAD,
            0,
            "_inputTexGrad_R32G32B32A32SFLOAT_2_R32G32SFLOAT",
            VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGRAD_EXTRA_LATYER
        },
        {
            __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT,
            HALTI5_PATCH_TX_EXTRA_INPUT_GRAD,
            0,
            "_inputTexGrad_R32G32B32A32SINT_2_R32G32UINT",
            VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGRAD_EXTRA_LATYER
        },
        {
            __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT,
            HALTI5_PATCH_TX_EXTRA_INPUT_GRAD,
            0,
            "_inputTexGrad_R32G32B32A32UINT_2_R32G32UINT",
            VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD,
            VSC_RES_ACT_BIT_EXTRA_SAMPLER,
            VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGRAD_EXTRA_LATYER
        },
    };

    uint32_t i;

    for (i = 0; i < __VK_COUNTOF(s_patchEntries); i++)
    {
        if (((patchFormat == s_patchEntries[i].patchFormat) || (patchFormat == VK_FORMAT_UNDEFINED))
          && (patchType == s_patchEntries[i].patchType)
          && ((s_patchEntries[i].viewTypeBits == 0) || (s_patchEntries[i].viewTypeBits & (1 << viewType))))
        {
            if (pOpTypeBits)
            {
                *pOpTypeBits = s_patchEntries[i].opTypeBits;
            }
            if (pActBits)
            {
                *pActBits = s_patchEntries[i].actBits;
            }
            if (pSubType)
            {
                *pSubType = s_patchEntries[i].subType;
            }
            return s_patchEntries[i].patchFunc;
        }
    }

    return VK_NULL_HANDLE;
}

VkResult halti5_updateDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    )
{
    __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, descriptorSet);
    halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;
    __vkDescriptorSetLayout *descSetLayout = descSet->descSetLayout;
    uint32_t i;
    uint32_t patchIdx = 0;
    uint32_t entryIdx = 0;
    halti5_patch_key *patchKeys = chipDescSet->patchKeys;
    halti5_patch_info *patchInfos = chipDescSet->patchInfos;

    __VK_MEMZERO(patchKeys, sizeof(halti5_patch_key) * chipDescSet->numofEntries);
    __VK_MEMZERO(patchInfos, sizeof(halti5_patch_info) * chipDescSet->numofEntries);

    for (i = 0; i < descSetLayout->bindingCount; i++)
    {
        __vkDescriptorSetLayoutBinding  *binding = &descSetLayout->binding[i];
        uint32_t j;
        for (j = 0; j < binding->std.descriptorCount; j++)
        {
            __vkDescriptorResourceRegion curRegion;
            __vkDescriptorResourceInfo *resInfo;
            __vkSampler **samplers;

            __vk_utils_region_mad(&curRegion, &binding->perElementSize, j, &binding->offset);
            resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curRegion.resource);
            samplers = (__vkSampler **)((uint8_t*)descSet->samplers + curRegion.sampler);

            switch (binding->std.descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                if (samplers[0])
                {
                    __vkSampler *sampler = samplers[0];
                    halti5_sampler *chipSampler = (halti5_sampler *)sampler->chipPriv;
                    if (chipSampler->patchKey)
                    {
                        patchKeys[entryIdx] = chipSampler->patchKey;
                        patchInfos[entryIdx].patchStages = binding->std.stageFlags;
                        patchInfos[entryIdx].binding = i;
                        patchInfos[entryIdx].arrayIndex = j;
                        patchInfos[entryIdx].viewType = VK_IMAGE_VIEW_TYPE_1D;
                        patchInfos[entryIdx].compareOp = sampler->createInfo.compareOp;
                        patchIdx++;
                    }
                }
                entryIdx++;
                break;

            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                {
                    halti5_patch_key patchKey = 0;
                    uint32_t patchFormat = VK_FORMAT_UNDEFINED;
                    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
                    SwizzleComponent swizzles[4] = {SWIZZLE_RED,
                                                       SWIZZLE_GREEN,
                                                       SWIZZLE_BLUE,
                                                       SWIZZLE_ALPHA};
                    VkCompareOp compareOp = VK_COMPARE_OP_MAX_ENUM;
                    if (samplers[0])
                    {
                        __vkSampler *sampler = samplers[0];
                        halti5_sampler *chipSampler = (halti5_sampler *)sampler->chipPriv;
                        patchKey |= chipSampler->patchKey;
                        compareOp = sampler->createInfo.compareOp;
                    }

                    if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                    {
                        __vkImageView *imgv = resInfo->u.imageInfo.imageView;
                        halti5_imageView *chipImgv = (halti5_imageView *)imgv->chipPriv;
                        __VK_ASSERT(resInfo->type == __VK_DESC_RESOURCE_IMAGEVIEW_INFO);
                        patchKey |= chipImgv->patchKey;
                        patchFormat = chipImgv->patchFormat;
                        viewType = imgv->createInfo.viewType;
                        swizzles[0] = __vkConvertSwizzle(imgv->createInfo.components.r, SWIZZLE_RED);
                        swizzles[1] = __vkConvertSwizzle(imgv->createInfo.components.g, SWIZZLE_GREEN);
                        swizzles[2] = __vkConvertSwizzle(imgv->createInfo.components.b, SWIZZLE_BLUE);
                        swizzles[3] = __vkConvertSwizzle(imgv->createInfo.components.a, SWIZZLE_ALPHA);
                    }

                    if (patchKey)
                    {
                        patchKeys[entryIdx] = patchKey;
                        patchInfos[entryIdx].patchStages = binding->std.stageFlags;
                        patchInfos[entryIdx].patchFormat = patchFormat;
                        patchInfos[entryIdx].binding = i;
                        patchInfos[entryIdx].arrayIndex = j;
                        patchInfos[entryIdx].viewType = viewType;
                        __VK_ASSERT(compareOp != VK_COMPARE_OP_MAX_ENUM);
                        patchInfos[entryIdx].compareOp = compareOp;
                        __VK_MEMCOPY(patchInfos[entryIdx].swizzles, swizzles, sizeof(swizzles));
                        patchIdx++;
                    }
                }
                entryIdx++;
                break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    __vkImageView *imgv = resInfo->u.imageInfo.imageView;
                    halti5_imageView *chipImgv = (halti5_imageView *)imgv->chipPriv;
                    __VK_ASSERT(resInfo->type == __VK_DESC_RESOURCE_IMAGEVIEW_INFO);
                    if (chipImgv->patchKey)
                    {
                        patchKeys[entryIdx] = chipImgv->patchKey;
                        patchInfos[entryIdx].binding = i;
                        patchInfos[entryIdx].arrayIndex = j;
                        patchInfos[entryIdx].patchStages = binding->std.stageFlags;
                        patchInfos[entryIdx].patchFormat = chipImgv->patchFormat;
                        patchInfos[entryIdx].viewType = imgv->createInfo.viewType;
                        patchInfos[entryIdx].swizzles[0] = __vkConvertSwizzle(imgv->createInfo.components.r, SWIZZLE_RED);
                        patchInfos[entryIdx].swizzles[1] = __vkConvertSwizzle(imgv->createInfo.components.g, SWIZZLE_GREEN);
                        patchInfos[entryIdx].swizzles[2] = __vkConvertSwizzle(imgv->createInfo.components.b, SWIZZLE_BLUE);
                        patchInfos[entryIdx].swizzles[3] = __vkConvertSwizzle(imgv->createInfo.components.a, SWIZZLE_ALPHA);
                        patchIdx++;
                    }
                }
                entryIdx++;
                break;

            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                if (resInfo->type != __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    __vkBufferView *bufv = resInfo->u.bufferView;
                    halti5_bufferView *chipBufv = (halti5_bufferView *)bufv->chipPriv;
                    __VK_ASSERT(resInfo->type == __VK_DESC_RESOURCE_BUFFERVIEW_INFO);
                    if (chipBufv->patchKey)
                    {
                        patchKeys[entryIdx] = chipBufv->patchKey;
                        patchInfos[entryIdx].patchFormat = chipBufv->patchFormat;
                        patchInfos[entryIdx].patchStages = binding->std.stageFlags;
                        patchInfos[entryIdx].binding = i;
                        patchInfos[entryIdx].arrayIndex = j;
                        patchInfos[entryIdx].viewType = VK_IMAGE_VIEW_TYPE_1D;
                        patchIdx++;
                    }
                }
                entryIdx++;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            default:
                break;
            }

        }
    }

    __VK_ASSERT(entryIdx == chipDescSet->numofEntries);

    chipDescSet->numofKeys = patchIdx;

    return VK_SUCCESS;
}

