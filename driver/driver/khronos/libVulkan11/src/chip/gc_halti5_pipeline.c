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

#define __VK_SAVE_THEN_LOAD_SHADER_BIN  0

#define __VK_VSC_DRV_FORMAT_MAP_NUM 54

halti5_formatMapInfo mapTable[__VK_VSC_DRV_FORMAT_MAP_NUM] =
{
    {VSC_IMAGE_FORMAT_NONE, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED},
    /*F32.*/
    {VSC_IMAGE_FORMAT_RGBA32F, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG32F, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R32F, VK_FORMAT_R32_SFLOAT, VK_FORMAT_UNDEFINED},
    /*I32.*/
    {VSC_IMAGE_FORMAT_RGBA32I, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG32I, VK_FORMAT_R32G32_SINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R32I, VK_FORMAT_R32_SINT, VK_FORMAT_UNDEFINED},
    /*UI32.*/
    {VSC_IMAGE_FORMAT_RGBA32UI, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG32UI, VK_FORMAT_R32G32_UINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R32UI, VK_FORMAT_R32_UINT, VK_FORMAT_UNDEFINED},
    /*F16.*/
    {VSC_IMAGE_FORMAT_RGBA16F, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG16F, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R16F, VK_FORMAT_R16_SFLOAT, VK_FORMAT_UNDEFINED},
    /*I16.*/
    {VSC_IMAGE_FORMAT_RGBA16I, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG16I, VK_FORMAT_R16G16_SINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R16I, VK_FORMAT_R16_SINT, VK_FORMAT_UNDEFINED},
    /*UI16.*/
    {VSC_IMAGE_FORMAT_RGBA16UI, VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG16UI, VK_FORMAT_R16G16_UINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R16UI, VK_FORMAT_R16_UINT, VK_FORMAT_UNDEFINED},
    /*F16 SNORM/UNORM.*/
    {VSC_IMAGE_FORMAT_RGBA16, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RGBA16_SNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG16, VK_FORMAT_R16G16_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R16, VK_FORMAT_R16_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R16_SNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_UNDEFINED},
    /*F8 SNORM/UNORM.*/
    {VSC_IMAGE_FORMAT_BGRA8_UNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RGBA8, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_A8B8G8R8_UNORM_PACK32},
    {VSC_IMAGE_FORMAT_RGBA8_SNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_A8B8G8R8_SNORM_PACK32},
    {VSC_IMAGE_FORMAT_RG8, VK_FORMAT_R8G8_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_RG8_SNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R8, VK_FORMAT_R8_UNORM, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R8_SNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_UNDEFINED},
    /*I8.*/
    {VSC_IMAGE_FORMAT_RGBA8I, VK_FORMAT_R8G8B8A8_SINT, VK_FORMAT_A8B8G8R8_SINT_PACK32},
    {VSC_IMAGE_FORMAT_RG8I, VK_FORMAT_R8G8_SINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R8I, VK_FORMAT_R8_SINT, VK_FORMAT_UNDEFINED},
    /*UI8.*/
    {VSC_IMAGE_FORMAT_RGBA8UI, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_A8B8G8R8_UINT_PACK32},
    {VSC_IMAGE_FORMAT_RG8UI, VK_FORMAT_R8G8_UINT, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_R8UI, VK_FORMAT_R8_UINT, VK_FORMAT_UNDEFINED},
    /*F-PACK.*/
    {VSC_IMAGE_FORMAT_R5G6B5_UNORM_PACK16, VK_FORMAT_R5G6B5_UNORM_PACK16, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_ABGR8_UNORM_PACK32, VK_FORMAT_A8B8G8R8_UNORM_PACK32, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_ABGR8I_PACK32, VK_FORMAT_A8B8G8R8_SINT_PACK32, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_ABGR8UI_PACK32, VK_FORMAT_A8B8G8R8_UINT_PACK32, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_FORMAT_UNDEFINED},
    {VSC_IMAGE_FORMAT_A2B10G10R10UI_PACK32, VK_FORMAT_A2B10G10R10_UINT_PACK32, VK_FORMAT_UNDEFINED}
};

#if __VK_SAVE_THEN_LOAD_SHADER_BIN
static VkResult __vkSaveThenLoadShaderBin(
    VkAllocationCallbacks *pAllocator,
    SHADER_HANDLE *pVirShader
)
{
    gctUINT binBytes = 0;
    void *pBinBack = gcvNULL;
    void *pBinCopy = gcvNULL;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(pAllocator);

    do
    {
        __VK_ERR_BREAK(vscSaveShaderToBinary(*pVirShader, &pBinBack, &binBytes));
        __VK_ERR_BREAK(halti5_DestroyVkShader(*pVirShader));
        *pVirShader = gcvNULL;

        pBinCopy = __VK_ALLOC(__VK_ALIGN(binBytes, 8), 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
        __VK_ERR_BREAK(pBinCopy ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMCOPY(pBinCopy, pBinBack, binBytes);
        gcoOS_Free(gcvNULL, pBinBack);
        pBinBack = gcvNULL;

        __VK_ERR_BREAK(vscLoadShaderFromBinary(pBinCopy, binBytes, pVirShader, gcvFALSE));
    }
    while (VK_FALSE);

    if (pBinBack)
    {
        gcoOS_Free(gcvNULL, pBinBack);
    }

    if (pBinCopy)
    {
        __VK_FREE(pBinCopy);
    }

    return result;
}
#endif

static int cmpfunc(
    const void *a,
    const void *b
    )
{
    HwVertexAttribDesc *pA = (HwVertexAttribDesc *)a;
    HwVertexAttribDesc *pB = (HwVertexAttribDesc *)b;

    if (pA->sortedAttributeDescPtr->binding > pB->sortedAttributeDescPtr->binding)
    {
        return 1;
    }
    else if (pA->sortedAttributeDescPtr->binding == pB->sortedAttributeDescPtr->binding)
    {
        if (pA->sortedAttributeDescPtr->offset > pB->sortedAttributeDescPtr->offset)
        {
            return 1;
        }
    }

    return -1;
}

static VkBaseInStructure* get_object_by_type(VkBaseInStructure* pObj, VkStructureType types)
{
    while (pObj)
    {
        if (pObj->sType == types)
        {
            return pObj;
        }
        else
        {
            pObj = (VkBaseInStructure*)pObj->pNext;
        }
    }

    return gcvNULL;
}

VkBool32 halti5_isMismatch(VSC_IMAGE_FORMAT vscFormat, VkFormat descFormat)
{
    VkFormat drvFormat = mapTable[vscFormat].drvFormat;
    VkFormat cmpFormat = mapTable[vscFormat].cmpFormat;
    VkBool32 result = VK_FALSE;

    /*VIV: the vkcts1.1.3 cases: dEQP-VK.memory.pipeline_barrier.host_write_uniform_texel_buffer.*
    ** on 8qm_wld shouldn't trigger recompile
    */
    if (vscFormat == VSC_IMAGE_FORMAT_RGBA16UI &&
        descFormat == VK_FORMAT_R16_UINT)
    {
        result = VK_FALSE;
        return result;
    }

    if ((descFormat != drvFormat) &&
        (descFormat != cmpFormat))
    {
        result = VK_TRUE;
    }

    return result;
}

VkResult halti5_helper_convert_VertexAttribDesc(
    __vkDevContext *devCtx,
    uint32_t count,
    HwVertexAttribDesc *hwVertxAttribDesc
    )
{
    uint32_t i, j;
    uint32_t fetchSize = 0;
    VkBool32 fetchBreak;

    static const struct
    {
        VkFormat format;
        uint32_t hwDataType;
        uint32_t hwsize;
        uint32_t hwNormalized;
        VkBool32 integer;
        uint32_t sizeInByte;
    }
    s_vkFormatToHwVsInputInfos[] =
    {
        /* mandated formats */
        {VK_FORMAT_R8G8B8A8_UNORM, 0x1, 0x0, 0x2, VK_FALSE, 4},
        {VK_FORMAT_R8G8B8_UNORM, 0x1, 0x3, 0x2, VK_FALSE, 3},
        {VK_FORMAT_R8G8_UNORM, 0x1, 0x2, 0x2, VK_FALSE, 2},
        {VK_FORMAT_R8_UNORM, 0x1, 0x1, 0x2, VK_FALSE, 1},

        {VK_FORMAT_R8G8B8A8_USCALED, 0x1, 0x0, 0x0, VK_FALSE, 4},
        {VK_FORMAT_R8G8B8_USCALED, 0x1, 0x3, 0x0, VK_FALSE, 3},
        {VK_FORMAT_R8G8_USCALED, 0x1, 0x2, 0x0, VK_FALSE, 2},
        {VK_FORMAT_R8_USCALED, 0x1, 0x1, 0x0, VK_FALSE, 1},

        {VK_FORMAT_R8G8B8A8_UINT, 0xE, 0x0, 0x0, VK_TRUE, 4},
        {VK_FORMAT_R8G8B8_UINT, 0xE, 0x3, 0x0, VK_TRUE, 3},
        {VK_FORMAT_R8G8_UINT, 0xE, 0x2, 0x0, VK_TRUE, 2},
        {VK_FORMAT_R8_UINT, 0xE, 0x1, 0x0, VK_TRUE, 1},

        {VK_FORMAT_R8G8B8A8_SNORM, 0x0, 0x0, 0x2, VK_FALSE, 4},
        {VK_FORMAT_R8G8B8_SNORM, 0x0, 0x3, 0x2, VK_FALSE, 3},
        {VK_FORMAT_R8G8_SNORM, 0x0, 0x2, 0x2, VK_FALSE, 2},
        {VK_FORMAT_R8_SNORM, 0x0, 0x1, 0x2, VK_FALSE, 1},

        {VK_FORMAT_R8G8B8A8_SSCALED, 0x0, 0x0, 0x0, VK_FALSE, 4},
        {VK_FORMAT_R8G8B8_SSCALED, 0x0, 0x3, 0x0, VK_FALSE, 3},
        {VK_FORMAT_R8G8_SSCALED, 0x0, 0x2, 0x0, VK_FALSE, 2},
        {VK_FORMAT_R8_SSCALED, 0x0, 0x1, 0x0, VK_FALSE, 1},

        {VK_FORMAT_R8G8B8A8_SINT, 0xE, 0x0, 0x2, VK_TRUE, 4},
        {VK_FORMAT_R8G8B8_SINT, 0xE, 0x3, 0x2, VK_TRUE, 3},
        {VK_FORMAT_R8G8_SINT, 0xE, 0x2, 0x2, VK_TRUE, 2},
        {VK_FORMAT_R8_SINT, 0xE, 0x1, 0x2, VK_TRUE, 1},

        {VK_FORMAT_R16G16B16A16_UNORM, 0x3, 0x0, 0x2, VK_FALSE, 8},
        {VK_FORMAT_R16G16B16_UNORM, 0x3, 0x3, 0x2, VK_FALSE, 6},
        {VK_FORMAT_R16G16_UNORM, 0x3, 0x2, 0x2, VK_FALSE, 4},
        {VK_FORMAT_R16_UNORM, 0x3, 0x1, 0x2, VK_FALSE, 2},

        {VK_FORMAT_R16G16B16A16_USCALED, 0x3, 0x0, 0x0, VK_FALSE, 8},
        {VK_FORMAT_R16G16B16_USCALED, 0x3, 0x3, 0x0, VK_FALSE, 6},
        {VK_FORMAT_R16G16_USCALED, 0x3, 0x2, 0x0, VK_FALSE, 4},
        {VK_FORMAT_R16_USCALED, 0x3, 0x1, 0x0, VK_FALSE, 2},

        {VK_FORMAT_R16G16B16A16_UINT, 0xF, 0x0, 0x0, VK_TRUE, 8},
        {VK_FORMAT_R16G16B16_UINT, 0xF, 0x3, 0x0, VK_TRUE, 6},
        {VK_FORMAT_R16G16_UINT, 0xF, 0x2, 0x0, VK_TRUE, 4},
        {VK_FORMAT_R16_UINT, 0xF, 0x1, 0x0, VK_TRUE, 2},

        {VK_FORMAT_R16G16B16A16_SNORM, 0x2, 0x0, 0x2, VK_FALSE, 8},
        {VK_FORMAT_R16G16B16_SNORM, 0x2, 0x3, 0x2, VK_FALSE, 6},
        {VK_FORMAT_R16G16_SNORM, 0x2, 0x2, 0x2, VK_FALSE, 4},
        {VK_FORMAT_R16_SNORM, 0x2, 0x1, 0x2, VK_FALSE, 2},

        {VK_FORMAT_R16G16B16A16_SSCALED, 0x2, 0x0, 0x0, VK_FALSE, 8},
        {VK_FORMAT_R16G16B16_SSCALED, 0x2, 0x3, 0x0, VK_FALSE, 6},
        {VK_FORMAT_R16G16_SSCALED, 0x2, 0x2, 0x0, VK_FALSE, 4},
        {VK_FORMAT_R16_SSCALED, 0x2, 0x1, 0x0, VK_FALSE, 2},

        {VK_FORMAT_R16G16B16A16_SINT, 0xF, 0x0, 0x2, VK_TRUE, 8},
        {VK_FORMAT_R16G16B16_SINT, 0xF, 0x3, 0x2, VK_TRUE, 6},
        {VK_FORMAT_R16G16_SINT, 0xF, 0x2, 0x2, VK_TRUE, 4},
        {VK_FORMAT_R16_SINT, 0xF, 0x1, 0x2, VK_TRUE, 2},

        {VK_FORMAT_R32G32B32A32_SFLOAT, 0x8, 0x0, 0x0, VK_FALSE, 16},
        {VK_FORMAT_R32G32B32_SFLOAT, 0x8, 0x3, 0x0, VK_FALSE, 12},
        {VK_FORMAT_R32G32_SFLOAT, 0x8, 0x2, 0x0, VK_FALSE, 8},
        {VK_FORMAT_R32_SFLOAT, 0x8, 0x1, 0x0, VK_FALSE, 4},

        {VK_FORMAT_R32G32B32A32_SINT, 0x8, 0x0, 0x0, VK_TRUE, 16},
        {VK_FORMAT_R32G32B32_SINT, 0x8, 0x3, 0x0, VK_TRUE, 12},
        {VK_FORMAT_R32G32_SINT, 0x8, 0x2, 0x0, VK_TRUE, 8 },
        {VK_FORMAT_R32_SINT, 0x8, 0x1, 0x0, VK_TRUE, 4 },

        {VK_FORMAT_R32G32B32A32_UINT, 0x8, 0x0, 0x0, VK_TRUE, 16},
        {VK_FORMAT_R32G32B32_UINT, 0x8, 0x3, 0x0, VK_TRUE, 12},
        {VK_FORMAT_R32G32_UINT, 0x8, 0x2, 0x0, VK_TRUE, 8 },
        {VK_FORMAT_R32_UINT, 0x8, 0x1, 0x0, VK_TRUE, 4 },

        {VK_FORMAT_R16G16B16A16_SFLOAT, 0x9, 0x0, 0x0, VK_FALSE, 8},
        {VK_FORMAT_R16G16B16_SFLOAT, 0x9, 0x3, 0x0, VK_FALSE, 6},
        {VK_FORMAT_R16G16_SFLOAT, 0x9, 0x2, 0x0, VK_FALSE, 4},
        {VK_FORMAT_R16_SFLOAT, 0x9, 0x1, 0x0, VK_FALSE, 2},

        {VK_FORMAT_A8B8G8R8_UNORM_PACK32, 0x1, 0x0, 0x2, VK_FALSE, 4},
        {VK_FORMAT_A8B8G8R8_SNORM_PACK32, 0x0, 0x0, 0x2, VK_FALSE, 4},
        {VK_FORMAT_A8B8G8R8_UINT_PACK32, 0xE, 0x0, 0x0, VK_TRUE, 4},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32, 0xE, 0x0, 0x2, VK_TRUE, 4},


        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, 0x7, 0x0, 0x2, VK_FALSE,4},

        {VK_FORMAT_B8G8R8A8_UNORM, 0xA, 0x0, 0x2, VK_FALSE, 4},

        /* Not mandated formats */
        {VK_FORMAT_A2B10G10R10_USCALED_PACK32, 0x7, 0x0, 0x0, VK_FALSE, 4},
        {VK_FORMAT_A2B10G10R10_SNORM_PACK32, 0x6, 0x0, 0x2, VK_FALSE, 4},
        {VK_FORMAT_A2B10G10R10_SSCALED_PACK32, 0x6, 0x0, 0x0, VK_FALSE, 4},
    };

    /* sort attribute desc with binding and offset */
    qsort(hwVertxAttribDesc, count, sizeof(HwVertexAttribDesc), cmpfunc);

    for (j = 0; j  < count; j ++)
    {
        for (i = 0; i< __VK_COUNTOF(s_vkFormatToHwVsInputInfos); i++)
        {
            if (s_vkFormatToHwVsInputInfos[i].format == hwVertxAttribDesc[j].sortedAttributeDescPtr->format)
            {
                break;
            }
        }

        if (i >= __VK_COUNTOF(s_vkFormatToHwVsInputInfos))
        {
            __VK_ASSERT(0);
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }
        else
        {
            fetchSize += s_vkFormatToHwVsInputInfos[i].sizeInByte;

            if (j == (count -1))
            {
                /*last one */
                fetchBreak = VK_TRUE;
            }
            else
            {
                if (hwVertxAttribDesc[j+1].sortedAttributeDescPtr->binding != hwVertxAttribDesc[j].sortedAttributeDescPtr->binding)
                {
                    /* different stream */
                    fetchBreak = VK_TRUE;
                }
                else
                {
                    /* same stream */
                    if ((hwVertxAttribDesc[j].sortedAttributeDescPtr->offset + s_vkFormatToHwVsInputInfos[i].sizeInByte)
                        == hwVertxAttribDesc[j+1].sortedAttributeDescPtr->offset)
                    {
                        /* neighborhood is connected */
                        fetchBreak = VK_FALSE;
                    }
                    else
                    {
                        fetchBreak = VK_TRUE;
                    }
                }
            }

            hwVertxAttribDesc[j].hwDataType   = s_vkFormatToHwVsInputInfos[i].hwDataType;
            hwVertxAttribDesc[j].hwSize       = s_vkFormatToHwVsInputInfos[i].hwsize;
            hwVertxAttribDesc[j].hwNormalized = s_vkFormatToHwVsInputInfos[i].hwNormalized;
            hwVertxAttribDesc[j].integer      = s_vkFormatToHwVsInputInfos[i].integer;
            hwVertxAttribDesc[j].hwFetchSize  = fetchSize;
            hwVertxAttribDesc[j].hwFetchBreak = fetchBreak;

            if (hwVertxAttribDesc[j].is16Bit &&
                (s_vkFormatToHwVsInputInfos[i].format == VK_FORMAT_R16_SFLOAT ||
                 s_vkFormatToHwVsInputInfos[i].format == VK_FORMAT_R16G16_SFLOAT ||
                 s_vkFormatToHwVsInputInfos[i].format == VK_FORMAT_R16G16B16_SFLOAT ||
                 s_vkFormatToHwVsInputInfos[i].format == VK_FORMAT_R16G16B16A16_SFLOAT))
            {
                hwVertxAttribDesc[j].hwDataType = 0xF;
            }

            if (fetchBreak)
            {
                fetchSize = 0;
            }
        }
    }

    return VK_SUCCESS;
}

VkResult halti5_pip_emit_vsinput(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo * info
    )
{
    uint32_t bindingIdx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    const VkPipelineVertexInputStateCreateInfo *vsInputInfo = info->pVertexInputState;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    PROG_ATTRIBUTE_TABLE_ENTRY *shAttribTable = chipGfxPipeline->chipPipeline.masterInstance->pep.attribTable.pAttribEntries;
    uint32_t shAttribTableEntries = chipGfxPipeline->chipPipeline.masterInstance->pep.attribTable.countOfEntries;

    pCmdBuffer = pCmdBufferBegin = &chipGfxPipeline->chipPipeline.cmdBuffer[chipGfxPipeline->chipPipeline.curCmdIndex];

    /* Set up vertex stream */
    for (bindingIdx = 0; bindingIdx < vsInputInfo->vertexBindingDescriptionCount; bindingIdx++)
    {
        const VkVertexInputBindingDescription *vsInputBinding = &vsInputInfo->pVertexBindingDescriptions[bindingIdx];

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5190 + vsInputBinding->binding, VK_FALSE, vsInputBinding->stride);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x51A0 + vsInputBinding->binding, VK_FALSE, vsInputBinding->inputRate);

        if (vsInputBinding->inputRate == VK_VERTEX_INPUT_RATE_INSTANCE)
        {
            chipGfxPipeline->instancedVertexBindingMask |= 1 << vsInputBinding->binding;
            chipGfxPipeline->instancedVertexBindingStride[vsInputBinding->binding] = vsInputBinding->stride;
            chipGfxPipeline->instancedStrideDirty = 1;
        }
    }

    /* when vs has any input entries, it includes vertexID/instanceID*/
    if (shAttribTableEntries)
    {
        HwVertexAttribDesc hwVertexAttribDesc[__VK_MAX_VERTEX_ATTRIBS];
        uint32_t vsInputCount = 0;
        uint32_t vsInputState = 0;
        uint32_t vsInputHWRegAddr = 0x0230;
        uint32_t entryIdx, compositeIdx = 0, attribIdx;
        uint32_t usedAttribCount = 0;
        uint32_t hwRegForID = ~0U;
        uint32_t hwRegForVtxID = ~0U, hwRegForInstID = ~0U;

        /* collect hwReg for ID and filter out all unused vertex attributes */
        for (entryIdx = 0; entryIdx < shAttribTableEntries; entryIdx++)
        {
            uint32_t firstValidIoChannel = shAttribTable[entryIdx].pIoRegMapping[0].firstValidIoChannel;

            __VK_ASSERT(shAttribTable[entryIdx].arraySize == 1);

            if (shAttribTable[entryIdx].pIoRegMapping[0].ioChannelMapping[firstValidIoChannel].ioUsage == SHADER_IO_USAGE_VERTEXID)
            {
                __VK_ASSERT(shAttribTable[entryIdx].vec4BasedCount == 1);
                hwRegForVtxID = shAttribTable[entryIdx].pIoRegMapping[0].ioChannelMapping[firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo;
                hwRegForID = hwRegForVtxID;
                continue;
            }
            else if (shAttribTable[entryIdx].pIoRegMapping[0].ioChannelMapping[firstValidIoChannel].ioUsage == SHADER_IO_USAGE_INSTANCEID)
            {
                __VK_ASSERT(shAttribTable[entryIdx].vec4BasedCount == 1);
                hwRegForInstID = shAttribTable[entryIdx].pIoRegMapping[0].ioChannelMapping[firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo;
                hwRegForID = hwRegForInstID;
                continue;
            }

            if ((hwRegForVtxID != ~0U) && (hwRegForInstID != ~0U))
            {
                __VK_ASSERT(hwRegForInstID == hwRegForVtxID);
            }

            for (compositeIdx = 0; compositeIdx < shAttribTable[entryIdx].vec4BasedCount; compositeIdx++)
            {
                uint32_t location;
                __VK_ASSERT(compositeIdx < shAttribTable[entryIdx].locationCount);
                location = shAttribTable[entryIdx].pLocation[compositeIdx];

                for (attribIdx = 0; attribIdx < vsInputInfo->vertexAttributeDescriptionCount; attribIdx++)
                {
                    if (vsInputInfo->pVertexAttributeDescriptions[attribIdx].location == location)
                    {
                        hwVertexAttribDesc[usedAttribCount].is16Bit = (shAttribTable[entryIdx].resEntryBit & VSC_RES_ENTRY_BIT_16BIT) != 0;
                        hwVertexAttribDesc[usedAttribCount++].sortedAttributeDescPtr = &vsInputInfo->pVertexAttributeDescriptions[attribIdx];
                        break;
                    }
                }
            }
        }

        /* sort and convert all used vertex attributes to its HW desc */
        __VK_VERIFY_OK(halti5_helper_convert_VertexAttribDesc(devCtx, usedAttribCount, hwVertexAttribDesc));

        /* program vertex attributes and vs inputs for non-ID stuff */
        for (attribIdx = 0; attribIdx < usedAttribCount; attribIdx++)
        {
            VkBool32 found = VK_FALSE;
            uint32_t firstValidIoChannel;
            uint32_t hwRegNo;

            for (entryIdx = 0; entryIdx < shAttribTableEntries; entryIdx++)
            {
                for (compositeIdx = 0; compositeIdx < shAttribTable[entryIdx].vec4BasedCount; compositeIdx++)
                {
                    __VK_ASSERT(compositeIdx < shAttribTable[entryIdx].locationCount);

                    if (shAttribTable[entryIdx].pLocation[compositeIdx] ==
                        hwVertexAttribDesc[attribIdx].sortedAttributeDescPtr->location)
                    {
                        found = VK_TRUE;
                        break;
                    }
                }

                if (found)
                {
                    break;
                }
            }

            __VK_ASSERT(found);

            firstValidIoChannel = shAttribTable[entryIdx].pIoRegMapping[compositeIdx].firstValidIoChannel;
            hwRegNo = shAttribTable[entryIdx].pIoRegMapping[compositeIdx].ioChannelMapping[firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo;

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5E00 + attribIdx, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwSize) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwDataType) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwNormalized) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
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
 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].sortedAttributeDescPtr->binding) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].sortedAttributeDescPtr->offset) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16))));

            if (0)
            {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5EA0 + attribIdx, VK_FALSE,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:0) - (0 ?
 8:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:0) - (0 ?
 8:0) + 1))))))) << (0 ?
 8:0))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwFetchSize) & ((gctUINT32) ((((1 ?
 8:0) - (0 ?
 8:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:0) - (0 ? 8:0) + 1))))))) << (0 ? 8:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwFetchBreak) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))));
            }

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5E80 + attribIdx, VK_FALSE, hwVertexAttribDesc[attribIdx].integer ? 1 : 0x3F800000);

            /* Set vertex shader input linkage. */
            switch (vsInputCount & 3)
            {
            case 0:
            default:
                vsInputState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                break;
            case 1:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                break;
            case 2:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                break;
            case 3:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                /* Store the current shader input control value. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, vsInputHWRegAddr, VK_FALSE, vsInputState);
                vsInputHWRegAddr++;
                break;
            }
            vsInputCount++;
        }
        if (hwRegForID != ~0U)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F1, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:8) - (0 ?
 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (hwRegForID * 4) & ((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ? 14:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (hwRegForID * 4 + 1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16))));
        }
        else
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F1, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0))));
        }

        /* program vs input for ID */
        if ((vsInputCount & 3) || (hwRegForID != ~0U))
        {
            /* zero app-defined attributes */
            if (!vsInputCount)
            {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F2, VK_FALSE, 1);
            }
            if (hwRegForID != ~0U)
            {
                /* Set vertex shader input linkage. */
                switch (vsInputCount & 3)
                {
                case 0:
                default:
                    vsInputState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)));
                    break;
                case 1:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)));
                    break;
                case 2:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)));
                    break;
                case 3:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24)));
                    break;
                }
            }
            /* Store the current shader input control value. */
            __vkCmdLoadSingleHWState(&pCmdBuffer, vsInputHWRegAddr, VK_FALSE, vsInputState);
            vsInputHWRegAddr++;
        }
    }
    /* really zero input from FE side, no app-defined attributes and no ID, probably app use atomic counter in vs */
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F2, VK_FALSE, 1);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F1, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:8) - (0 ?
 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ? 14:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0230, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));

    }

    chipGfxPipeline->chipPipeline.curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);
    return VK_SUCCESS;
}


static VkResult halti5_pip_emit_viewport(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    float    wSmall;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;

    if(database->REG_BugFixes22 && database->REG_PAEnhancements3)
    {
        wSmall = 0.0f;
    }
    else
    {
        wSmall = 1.0f / 32768.0f;
    }

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x02A3, VK_FALSE, *(uint32_t*)&wSmall);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x02A1, VK_TRUE, 8192 << 16);

    if ((!(pip->dynamicStates & __VK_DYNAMIC_STATE_VIEWPORT_BIT))
        && info->pViewportState)
    {
        VkViewport  *pViewport = (VkViewport  *)&info->pViewportState->pViewports[0]; /* always pick first viewport */
        halti5_helper_set_viewport(devCtx, &pCmdBuffer, pViewport);
    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

static VkResult halti5_pip_emit_rs(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint32_t paState;
    VkBool32 hwEnablePrimitiveID;
    uint32_t hwCullMode = 0;
    VkBool32 hwRasterDiscard = VK_FALSE;
    const VkPipelineRasterizationStateCreateInfo *rsInfo = info->pRasterizationState;
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    VkBool32 isRenderPnt = VK_FALSE;
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;

    /* for tessellation also check out prim.*/
    if ((info->pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
    ||  (info->pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST &&
         chipPipeline->masterInstance->pep.seps[VSC_SHADER_STAGE_DS].exeHints.nativeHints.prvStates.ts.tessOutputPrim == SHADER_TESSELLATOR_OUTPUT_PRIM_POINT))
    {
        isRenderPnt = VK_TRUE;
    }

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    hwEnablePrimitiveID = (hints->primIdComponent != -1) && (!hints->prePaShaderHasPrimitiveId);

    switch (rsInfo->cullMode)
    {
    case VK_CULL_MODE_NONE:
        hwCullMode = 0x0;
        break;
    case VK_CULL_MODE_FRONT_BIT:
        switch (rsInfo->frontFace)
        {
        case VK_FRONT_FACE_COUNTER_CLOCKWISE:
            hwCullMode = 0x2;
            break;
        case VK_FRONT_FACE_CLOCKWISE:
            hwCullMode = 0x1;
            break;
        default:
            __VK_ASSERT(0);
        }
        break;
    case VK_CULL_MODE_BACK_BIT:
        switch (rsInfo->frontFace)
        {
        case VK_FRONT_FACE_COUNTER_CLOCKWISE:
            hwCullMode = 0x1;
            break;
        case VK_FRONT_FACE_CLOCKWISE:
            hwCullMode = 0x2;
            break;

        default:
            __VK_ASSERT(0);
        }
        break;
    case VK_CULL_MODE_FRONT_AND_BACK:
        hwRasterDiscard = VK_TRUE;
        break;

    default:
        __VK_ASSERT(0);
    }


    paState = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (hwEnablePrimitiveID) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))) &
              (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (hwCullMode) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))) &
              (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)))) &
              (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) ((hints->shaderMode == gcvSHADING_SMOOTH) ?
 1 : 0) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18)))) &
              (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))));

    /* Vulkan follows D3D rules for clipping/viewport xformation and provoking vertex,
    but pixel center is still (0.5,0.5) */
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x028A, VK_FALSE,
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
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x028D, VK_FALSE, paState);

    if (!(pip->dynamicStates & __VK_DYNAMIC_STATE_LINE_WIDTH_BIT))
    {
        halti5_helper_set_linewidth(devCtx, &pCmdBuffer, rsInfo->lineWidth);
    }

    if (database->HWTFB)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x7000, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) ((rsInfo->rasterizerDiscardEnable || hwRasterDiscard) ?
 0x1 : 0x0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4))));
    }

    if (!(pip->dynamicStates & __VK_DYNAMIC_STATE_DEPTH_BIAS_BIT)
        && (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
        && rsInfo->depthBiasEnable)
    {
        __vkAttachmentDesc *attachDesc;
        __vkDynamicDepthBiasState depthBiasState;
        attachDesc = &rdp->attachments[subPass->dsAttachIndex];
        depthBiasState.depthBiasConstantFactor = rsInfo->depthBiasConstantFactor;
        depthBiasState.depthBiasClamp = rsInfo->depthBiasClamp;
        depthBiasState.depthBiasSlopeFactor = rsInfo->depthBiasSlopeFactor;
        halti5_helper_set_depthBias(devCtx, attachDesc->formatInfo->residentImgFormat, &pCmdBuffer, VK_TRUE, &depthBiasState);
    }

    if (isRenderPnt)
    {
        VkBool32 hwEnablePtSize, hwEnablePtSprite;
        uint32_t pntPaState;
        hwEnablePtSize = hints->prePaShaderHasPointSize;
        hwEnablePtSprite = hints->prePaShaderHasPointSize && (hints->usePointCoord[0] || hints->usePointCoord[1]);

        pntPaState = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (hwEnablePtSize) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))) &
                     (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (hwEnablePtSprite) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x028D, VK_FALSE, pntPaState);
    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

static VkResult halti5_pip_emit_msaa(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    const VkPipelineMultisampleStateCreateInfo *msaaInfo = info->pMultisampleState;
    uint32_t msaaMode = 0, minSampleShadingValue = 0, sampleCount = 1;
    VkBool32 msaaEnable = VK_FALSE, sampleShading = VK_FALSE;
    VkBool32 sampleCenter = VK_FALSE;
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    gctUINT32_PTR sampleCoords = gcvNULL;
    gcsCENTROIDS_PTR centroids = {0};
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline*)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;

    /* no msaa info */
    if (msaaInfo == VK_NULL_HANDLE)
    {
        return VK_SUCCESS;
    }

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    switch (msaaInfo->rasterizationSamples)
    {
    case VK_SAMPLE_COUNT_4_BIT:
        msaaMode = 0x2;
        sampleCount = 4;
        msaaEnable = 0xF;
        sampleCoords = chipModule->sampleCoords4;
        centroids = chipModule->centroids4;
        break;
    case VK_SAMPLE_COUNT_2_BIT:
        msaaMode = 0x1;
        sampleCount = 2;
        msaaEnable = 0x3;
        sampleCoords = &chipModule->sampleCoords2;
        centroids = &chipModule->centroids2;
        break;
    case VK_SAMPLE_COUNT_1_BIT:
        msaaMode = 0x0;
        sampleCount = 1;
        msaaEnable = VK_FALSE;
        break;
    default:
        __VK_ASSERT(0);
        break;
    }

    if (database->MSAA_SHADING)
    {
        float minSampleShading = msaaInfo->minSampleShading;
        sampleShading = msaaInfo->sampleShadingEnable || hints->usedSampleIdOrSamplePosition || hints->psUsedSampleInput;

        if (hints->usedSampleIdOrSamplePosition)
        {
            minSampleShading = 1.0;
        }

        if (msaaEnable && sampleShading)
        {
            minSampleShadingValue = gcmMAX(gcmCEIL(minSampleShading * sampleCount), 1);
        }

        if ((!database->SH_PSO_MSAA1x_FIX) && (subPass->colorCount > 1))
        {
            sampleShading = VK_TRUE;
            minSampleShadingValue = 4;
        }

        if ((minSampleShadingValue == 1 && sampleShading) || !sampleShading)
        {
            sampleCenter = VK_TRUE;
        }
    }

    chipGfxPipeline->hwCacheMode = (sampleCount == 4) ? CHIP_CACHEMODE_256 : CHIP_CACHEMODE_64;

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E06, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (msaaMode) & ((gctUINT32) ((((1 ?
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
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (msaaEnable) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (sampleShading) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (minSampleShadingValue - 1) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) (sampleCenter) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28))));

    if (database->MSAA_SHADING || database->REG_GeometryShader)
    {
        if(sampleShading)
        {
            chipGfxPipeline->raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));
        }
        else
        {
            chipGfxPipeline->raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)));
        }
        chipGfxPipeline->raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (hints->useEarlyFragmentTest) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

        chipGfxPipeline->raControlEx |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((hints->rtArrayComponent != -1)  ?
 1 : 0) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    }

    if (msaaEnable)
    {
        VkBool32 sampleMaskEnable = (msaaInfo->pSampleMask != VK_NULL_HANDLE);
        uint32_t sampleMask = sampleMaskEnable ? msaaInfo->pSampleMask[0] : 0;

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0381, VK_FALSE, 0);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0384, VK_FALSE, *sampleCoords);
        __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0390, VK_FALSE, 4, centroids->value);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0415, VK_FALSE,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (msaaInfo->alphaToCoverageEnable) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)))) &
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (sampleMaskEnable) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))) &
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (sampleMask) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))));

        /* Program SAMPLE_DITHER_TABLE.*/
        if (msaaInfo->alphaToCoverageEnable)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418, VK_FALSE, 0x6E80E680);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 1, VK_FALSE, 0x2AC42A4C);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 2, VK_FALSE, 0x15FB5D3B);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 3, VK_FALSE, 0x9D7391F7);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 4, VK_FALSE, 0x08E691F7);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 5, VK_FALSE, 0x4CA25D3B);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 6, VK_FALSE, 0xBF512A4C);
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0418 + 7, VK_FALSE, 0x37D9E680);
        }
    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

static void halti5_helper_GetPsOutputSetting(
    __vkDevContext *devCtx,
    __vkPipeline * pip,
    gctUINT32 *outputMode,
    gctUINT32 *saturationMode,
    gctUINT32 *psOutCnl4to7)
{
    gctUINT i;
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    halti5_graphicsPipeline *chipGfxPipeline =(halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)chipGfxPipeline;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    PROG_FRAGOUT_TABLE_ENTRY *fragOutTable = chipGfxPipeline->chipPipeline.masterInstance->pep.fragOutTable.pFragOutEntries;

    gcmASSERT(outputMode);
    gcmASSERT(saturationMode);

    *outputMode =
    *saturationMode = 0;

    for (i = 0 ; fragOutTable != VK_NULL_HANDLE && i < subPass->colorCount; i++)
    {
        __vkAttachmentDesc *attachmentDesc;
        HwPEDesc hwPEDesc;
        uint32_t partIndex = 0;
        uint32_t hwRtIndex;
        VkBool32 is16BitStorage = VK_FALSE;

        if (subPass->color_attachment_index[i] == VK_ATTACHMENT_UNUSED)
        {
            continue;
        }

        attachmentDesc = &rdp->attachments[subPass->color_attachment_index[i]];
        is16BitStorage = (fragOutTable[i].resEntryBit & VSC_RES_ENTRY_BIT_16BIT) != 0;
        __VK_VERIFY_OK(halti5_helper_convertHwPEDesc(devCtx, attachmentDesc->formatInfo->residentImgFormat, is16BitStorage, &hwPEDesc));
        while (partIndex < chipGfxPipeline->patchOutput.outputs[i].partCount)
        {
            hwRtIndex = halti5_convertLocationToRenderIndex(hints, chipGfxPipeline->patchOutput.outputs[i].locations[partIndex]);

            if (hwRtIndex == 0xFFFFFFFF)
            {
                partIndex++;
                continue;
            }

            switch (hwRtIndex)
            {
            case 0:
                *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));
                break;

            case 1:
                *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
                break;

            case 2:
                *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
                break;

            case 3:
                *saturationMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:12) - (0 ?
 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)));
                break;

            case 4:
                *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));
                break;

            case 5:
                *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)));
                break;

            case 6:
                *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));
                break;

            case 7:
                *psOutCnl4to7 |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwSaturation) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)));

                *outputMode |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:28) - (0 ?
 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwOutputMode) & ((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ? 31:28)));
                break;

            default:
                gcmASSERT(0);

            }
            partIndex++;
        }
    }

    return;
}

static VkResult halti5_pip_emit_graphicsShaderInstance(
    __vkDevContext *devCtx,
    __vkPipeline *pip
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint32_t msaaExtraPsTemp = 0;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)chipGfxPipeline;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    const gcsFEATURE_DATABASE *database = devCtx->database;
    VkBool32 bypass = chipGfxPipeline->depthOnly;
    VkBool32 isRenderPnt = pip->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    uint32_t msaaExtraPsInput = 0;
    uint32_t vsOutputSequencerCount; /* output count of vs out sequencer */
    VkBool32 vsOutputSequencerForPA = VK_FALSE; /* if the output from sequence is for PA */
    uint32_t paOutputCount = 0;

    chipPipeline->curInstance->curInstanceCmdIndex = 0;

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->curInstance->instanceCmd[chipPipeline->curInstance->curInstanceCmdIndex];

    if (pip->msaaEnabled)
    {
        if (database->REG_RAWriteDepth)
        {
            if (chipGfxPipeline->subSampleZUsedInPS)
            {
                msaaExtraPsTemp = 1;
                msaaExtraPsInput = hints->fsIsDual16 ? 2 : 1;
            }
        }
        else
        {
            msaaExtraPsTemp = 1;
            msaaExtraPsInput = hints->fsIsDual16 ? 2 : 1;
        }
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0403, VK_FALSE,
        hints->fsMaxTemp + msaaExtraPsTemp);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E07, VK_FALSE,
        (bypass ? 0 : hints->componentCount));

    if (!hints->vsUseStoreAttr)
    {
        if (hints->stageBits & (gcvPROGRAM_STAGE_GEOMETRY_BIT | gcvPROGRAM_STAGE_TES_BIT))
        {
            vsOutputSequencerCount = hints->vsOutputCount;
        }
        else
        {
            if (bypass)
            {
                /* In bypass mode, we only need the position output and the point
                ** size if present and the primitive is a point list. */
                vsOutputSequencerCount = 1 + ((hints->vsHasPointSize && isRenderPnt) ? 1 : 0);
            }
            else
            {
                /* If the Vertex Shader generates a point size, and this point size will not be streamed out,
                   and the primitive is not a point, we don't need to send it down. */
                vsOutputSequencerCount
                    = (hints->vsHasPointSize && hints->vsPtSizeAtLastLinkLoc &&
                    !isRenderPnt && !hints->isPtSizeStreamedOut)
                        ? hints->vsOutputCount - 1
                        : hints->vsOutputCount;
            }

            vsOutputSequencerForPA = VK_TRUE;
        }

        /* SW WA for transform feedback, when VS has no output. */
        if (vsOutputSequencerCount == 0)
        {
            vsOutputSequencerCount = 1;
        }

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0201, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsOutputSequencerCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput16RegNo) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput17RegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hints->vsOutput18RegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ? 29:24))));
    }
    else
    {
        /* USC has been stored by shader, attr-sequencer does not need this info, so just
           set output count to 0 */
        vsOutputSequencerCount = 0;

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0201, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (vsOutputSequencerCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));
    }

    /* paOutputCount is dependent on vsOutputSequencerCount.*/
    if (bypass)
    {
        paOutputCount = 1;
    }
    else if (vsOutputSequencerForPA)
    {
        paOutputCount = gcmMIN((uint32_t)hints->shader2PaOutputCount, vsOutputSequencerCount);
    }
    else
    {
        paOutputCount = hints->shader2PaOutputCount;
    }
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x02AA, VK_FALSE, paOutputCount);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0402, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hints->fsInputCount + msaaExtraPsInput) & ((gctUINT32) ((((1 ?
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
 20:16))) | (((gctUINT32) ((gctUINT32) (hints->psHighPVaryingCount) & ((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ? 20:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) ((hints->psInputControlHighpPosition | chipGfxPipeline->sampleMaskInPos | (hints->rtArrayComponent != -1))) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))));

    chipPipeline->curInstance->curInstanceCmdIndex+= (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(chipPipeline->curInstance->curInstanceCmdIndex <= HALTI5_INSTANCE_CMD_BUFSIZE);

    return VK_SUCCESS;
}

static int32_t get_used_color_count(
    struct _gcsHINT *hints,
    __vkRenderSubPassInfo *subPass
    )
{
    uint32_t i;
    int32_t colorOutCount = 0;

    for (i = 0; i < subPass->colorCount; i++)
    {
        if (hints->psOutput2RtIndex[i] != -1 && subPass->color_attachment_index[i] != VK_ATTACHMENT_UNUSED)
        {
            colorOutCount++;
        }
        __VK_ASSERT(!(colorOutCount > 0 && hints->psOutput2RtIndex[i] != -1 && subPass->color_attachment_index[i] == VK_ATTACHMENT_UNUSED) );
    }

    return colorOutCount;
}

static VkResult halti5_pip_emit_graphicsProgram(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    const VkPipelineMultisampleStateCreateInfo *msaaInfo = info->pMultisampleState;
    VkBool32 msaaEnable = VK_FALSE, sampleMaskOutFromPS = VK_FALSE;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)chipGfxPipeline;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;
    uint32_t psOutCntl4to7 = hints->psOutCntl4to7;
    uint32_t outputMode0to7, saturationMode0to3;
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    uint32_t reservedPages;
    VkBool32 bypass = chipGfxPipeline->depthOnly;
    uint32_t i;
    uint32_t colorOutCount;
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;

    /* step 1: emit hardware states for shader instance */
    halti5_pip_emit_graphicsShaderInstance(devCtx, pip);

    /* step 2: emit hardware states for this program(pipeline), shared between instances */
    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    reservedPages = database->SH_SNAP2PAGE_MAXPAGES_FIX  ? 0 : 1;

    if (pip->msaaEnabled)
    {
        sampleMaskOutFromPS = hints->sampleMaskOutWritten;
    }

    halti5_helper_GetPsOutputSetting(devCtx, pip, &outputMode0to7, &saturationMode0to3, &psOutCntl4to7);

    chipGfxPipeline->psOutCntl4to7 = psOutCntl4to7;

    colorOutCount = get_used_color_count(hints, subPass);
    colorOutCount = ((colorOutCount + chipGfxPipeline->patchOutput.count) != 0) ?
        (colorOutCount + chipGfxPipeline->patchOutput.count - 1) : 0;

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0404, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (bypass) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | saturationMode0to3
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (colorOutCount) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (sampleMaskOutFromPS) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))));

    /* reprogram semantics registers */
    if (msaaEnable && (msaaInfo->sampleShadingEnable
                    || hints->usedSampleIdOrSamplePosition
                    || hints->psUsedSampleInput))
    {
        uint32_t i;
        for (i = 0; i < 16; i ++)
        {
            gctUINT state =
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 0]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 1]) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 2]) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 3]) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 4]) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 5]) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ? 21:20)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:24) - (0 ?
 25:24) + 1))))))) << (0 ?
 25:24))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 6]) & ((gctUINT32) ((((1 ?
 25:24) - (0 ?
 25:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:24) - (0 ? 25:24) + 1))))))) << (0 ? 25:24)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (hints->interpolationType[i * 8 + 7]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:2) - (0 ?
 3:2) + 1))))))) << (0 ?
 3:2))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 3:2) - (0 ?
 3:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:2) - (0 ? 3:2) + 1))))))) << (0 ? 3:2)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:6) - (0 ?
 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 7:6) - (0 ?
 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:22) - (0 ?
 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 23:22) - (0 ?
 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E30 + i, VK_FALSE, state);
        }
    }

    if (!(hints->stageBits & gcvPROGRAM_STAGE_VERTEX_BIT))
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0228, VK_FALSE, reservedPages);
    }

    if (!(hints->stageBits & gcvPROGRAM_STAGE_TES_BIT) && database->REG_TessellationShaders)
    {
        gcmASSERT(!(hints->stageBits & gcvPROGRAM_STAGE_TCS_BIT));
        gcmASSERT(!(hints->stageBits & gcvPROGRAM_STAGE_TES_BIT));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52C6, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52C7, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52CD, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52CD, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))));

        __vkCmdLoadSingleHWState(&pCmdBuffer,0x5286, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer,0x5286, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20))));
    }

    if (database->REG_GeometryShader &&
        (!(hints->stageBits & gcvPROGRAM_STAGE_GEOMETRY_BIT)))
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0440, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0450, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0450, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21))));
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x040C, VK_FALSE, outputMode0to7);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x020C, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (hints->balanceMax) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:8) - (0 ?
 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (hints->balanceMin) & ((gctUINT32) ((((1 ?
 15:8) - (0 ?
 15:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ? 15:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:16) - (0 ?
 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ?
 21:16) - (0 ?
 21:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ? 21:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0216, VK_FALSE, 0x00001005);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E22, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:0) - (0 ?
 6:0) + 1))))))) << (0 ?
 6:0))) | (((gctUINT32) ((gctUINT32) (hints->ptSzAttrIndex * 4) & ((gctUINT32) ((((1 ?
 6:0) - (0 ?
 6:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:0) - (0 ? 6:0) + 1))))))) << (0 ? 6:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:8) - (0 ?
 14:8) + 1))))))) << (0 ?
 14:8))) | (((gctUINT32) ((gctUINT32) (hints->pointCoordComponent) & ((gctUINT32) ((((1 ?
 14:8) - (0 ?
 14:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:8) - (0 ? 14:8) + 1))))))) << (0 ? 14:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:16) - (0 ?
 22:16) + 1))))))) << (0 ?
 22:16))) | (((gctUINT32) ((gctUINT32) (hints->rtArrayComponent) & ((gctUINT32) ((((1 ?
 22:16) - (0 ?
 22:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:16) - (0 ? 22:16) + 1))))))) << (0 ? 22:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:24) - (0 ?
 30:24) + 1))))))) << (0 ?
 30:24))) | (((gctUINT32) ((gctUINT32) (hints->primIdComponent) & ((gctUINT32) ((((1 ?
 30:24) - (0 ?
 30:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:24) - (0 ? 30:24) + 1))))))) << (0 ? 30:24))));

    for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
    {
        if (-1 != hints->vsICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0223, VK_FALSE, hints->vsICachePrefetch[i]);
        }

        if (-1 != hints->tcsICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5283, VK_FALSE, hints->tcsICachePrefetch[i]);
        }

        if (-1 != hints->tesICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52C4, VK_FALSE, hints->tesICachePrefetch[i]);
        }

        if (-1 != hints->gsICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0446, VK_FALSE, hints->gsICachePrefetch[i]);
        }

        if (-1 != hints->fsICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0412, VK_FALSE, hints->fsICachePrefetch[i]);
        }
    }

    if (info->pTessellationState)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F0, VK_FALSE, info->pTessellationState->patchControlPoints);
    }

    /* Flush private constant uniform. */
    if (chipGfxPipeline->sampleLocation.bUsed)
    {
        __VK_ASSERT(chipGfxPipeline->sampleLocation.hwRegCount != 0 && chipGfxPipeline->sampleLocation.hwRegCount <= 4);

        /* Compiler may not active the whole constant array, so we only need to flush the used part. */
        __vkCmdLoadBatchHWStates(&pCmdBuffer, chipGfxPipeline->sampleLocation.hwRegAddress, VK_FALSE,
            chipGfxPipeline->sampleLocation.hwRegCount * 4, &chipModule->sampleLocations[0][0]);
    }

    if (chipGfxPipeline->ehableMultiSampleBuffers.bUsed)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->ehableMultiSampleBuffers.hwRegAddress, VK_FALSE,
        ((info->pMultisampleState->rasterizationSamples > VK_SAMPLE_COUNT_1_BIT) ? 1 : 0));
    }

     /* program default UBO address */
    if (chipGfxPipeline->defaultBuffer.bUsed)
    {
        uint32_t physical;
        __vkBuffer *defaultBuf =  __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipGfxPipeline->defaultUbo);
        __VK_ASSERT(chipGfxPipeline->defaultBuffer.hwRegCount != 0 && chipGfxPipeline->defaultBuffer.hwRegCount <= 4);
        physical = defaultBuf->memory->devAddr;
        physical += (uint32_t)(defaultBuf->memOffset);

        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->defaultBuffer.hwRegAddress, VK_FALSE, physical);
    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}


static VkColorComponentFlags halti5_helper_GetColorMask(
    __vkDevContext *devCtx,
    VkColorComponentFlags oldValue,
    gctUINT32 hwFormat,
    uint32_t vkFormat,
    gctUINT32 partIndex
    )
{
    VkColorComponentFlags newColorMask = oldValue;
    const gcsFEATURE_DATABASE *database = devCtx->database;

    if (oldValue != 0xF)
    {
        gctUINT8 rMask = (oldValue & VK_COLOR_COMPONENT_R_BIT) >> 0;
        gctUINT8 gMask = (oldValue & VK_COLOR_COMPONENT_G_BIT) >> 1;
        gctUINT8 bMask = (oldValue & VK_COLOR_COMPONENT_B_BIT) >> 2;
        gctUINT8 aMask = (oldValue & VK_COLOR_COMPONENT_A_BIT) >> 3;

        if (!database->PE_32BPC_COLORMASK_FIX)
        {
            switch(vkFormat)
            {
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R32_SINT:
                newColorMask = (aMask << 3) | (bMask << 2) | (rMask << 1) | rMask;
                break;
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R32G32_SFLOAT:
                newColorMask =  (gMask << 3) | (gMask << 2) | (rMask << 1) | rMask;
                break;

            case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
            case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
            case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
                switch (partIndex)
                {
                case 0:
                    newColorMask =  (gMask << 3) | (gMask << 2) | (rMask << 1) | rMask;
                    break;
                case 1:
                    newColorMask =  (aMask << 3) | (aMask << 2) | (bMask << 1) | bMask;
                    break;
                default:
                    __VK_ASSERT(!"invalid part index value");
                    break;
                }
                break;
            }
        }
        else if ((hwFormat == 0x15)  && (partIndex == 1))
        {
            newColorMask = (aMask << 1) | bMask;
        }
    }

    return newColorMask;
}



static VkResult halti5_pip_emit_rt(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkBool32 depthOnly = VK_FALSE;
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    const VkPipelineDepthStencilStateCreateInfo *dsInfo = info->pDepthStencilState;
    const VkPipelineMultisampleStateCreateInfo  *msaaInfo = info->pMultisampleState;
    const VkPipelineRasterizationStateCreateInfo *rsInfo = info->pRasterizationState;
    const VkPipelineColorBlendStateCreateInfo *blendInfo = info->pColorBlendState;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)chipGfxPipeline;
    PROG_FRAGOUT_TABLE_ENTRY *fragOutTable = chipGfxPipeline->chipPipeline.masterInstance->pep.fragOutTable.pFragOutEntries;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;
    uint32_t depthMode, stencilMode;
    VkBool32 psHasMemoryAccess = (hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][gcvPROGRAM_STAGE_FRAGMENT] & (gceMA_FLAG_READ | gceMA_FLAG_WRITE));
    VkBool32 psEarlyFragmentTest = hints->useEarlyFragmentTest;
    VkBool32 msaaFragmentOp = (msaaInfo && (msaaInfo->alphaToCoverageEnable || msaaInfo->pSampleMask));
    VkBool32 msaaEnable = (msaaInfo && msaaInfo->rasterizationSamples > VK_SAMPLE_COUNT_1_BIT);
    const gcsFEATURE_DATABASE *database = devCtx->database;
    VkBool32 fullfuncZ;
    uint32_t regDepthConfig = 0, regRAControl = 0;
    VkBool32 hasDsSurface = (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED) ? VK_TRUE : VK_FALSE;
    uint32_t i;
    VkBool32 rtEnabled = gcvFALSE;

    static const gctINT32 s_xlateDepthCompare[] =
    {
        /* VK_COMPARE_OP_NEVER */
        0x0,
        /* VK_COMPARE_OP_LESS */
        0x1,
        /* VK_COMPARE_OP_EQUAL */
        0x2,
        /* VK_COMPARE_OP_LESS_OR_EQUAL */
        0x3,
        /* VK_COMPARE_OP_GREATER */
        0x4,
        /* VK_COMPARE_OP_NOT_EQUAL */
        0x5,
        /* VK_COMPARE_OP_GREATER_OR_EQUAL */
        0x6,
        /* VK_COMPARE_OP_ALWAYS */
        0x7,
    };

    static const gctINT8 s_xlateStencilCompare[] =
    {
        /* VK_COMPARE_OP_NEVER */
        0x0,
        /* VK_COMPARE_OP_LESS */
        0x1,
        /* VK_COMPARE_OP_EQUAL */
        0x2,
        /* VK_COMPARE_OP_LESS_OR_EQUAL */
        0x3,
        /* VK_COMPARE_OP_GREATER */
        0x4,
        /* VK_COMPARE_OP_NOT_EQUAL */
        0x5,
        /* VK_COMPARE_OP_GREATER_OR_EQUAL */
        0x6,
        /* VK_COMPARE_OP_ALWAYS */
        0x7
    };

    static const gctUINT8 s_xlateStencilOperation[] =
    {
        /* VK_STENCIL_OP_KEEP */
        0x0,
        /* VK_STENCIL_OP_ZERO */
        0x1,
        /* VK_STENCIL_OP_REPLACE */
        0x2,
        /* VK_STENCIL_OP_INCREMENT_AND_CLAMP */
        0x3,
        /* VK_STENCIL_OP_DECREMENT_AND_CLAMP */
        0x4,
        /* VK_STENCIL_OP_INVERT */
        0x5,
        /* VK_STENCIL_OP_INCREMENT_AND_WRAP */
        0x6,
        /* VK_STENCIL_OP_DECREMENT_AND_WRAP */
        0x7,
    };

    depthOnly = (subPass->colorCount == 0) || (!(hints->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT));
    /* ps shader is not necessary to be excuted */
    depthOnly &= !(hints->hasKill
        || hints->psHasFragDepthOut
        || psHasMemoryAccess
        || (hints->rtArrayComponent != -1)
        || (hints->sampleMaskLoc != -1)
        || msaaFragmentOp);

    chipGfxPipeline->depthOnly = depthOnly;

    if (rsInfo->rasterizerDiscardEnable)
    {
        return VK_SUCCESS;
    }

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    chipGfxPipeline->singlePEpipe = VK_FALSE;

    if (!((database->NumPixelPipes == 1) || database->PE_8bpp_DUALPIPE_FIX))
    {
        chipGfxPipeline->singlePEpipe = (subPass->colorCount > 1) && chipGfxPipeline->hasOne8bitCb;
    }
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x052F, VK_FALSE,
        chipGfxPipeline->singlePEpipe ? 0x1 : 0x0);

    for (i = 0; i < subPass->colorCount; i++)
    {
        rtEnabled |= subPass->color_attachment_index[i] != VK_ATTACHMENT_UNUSED;
    }
    if (!rtEnabled || fragOutTable == VK_NULL_HANDLE)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x050B, VK_FALSE,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (!VK_FALSE) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))));
    }

    chipGfxPipeline->raControlEx &=
        ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (!(chipGfxPipeline->depthOnly)) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x038D, VK_FALSE, chipGfxPipeline->raControlEx);

    /* step 2: rt programming */
    for (i = 0; fragOutTable != VK_NULL_HANDLE && blendInfo != NULL && i < subPass->colorCount; i++)
    {
        const VkPipelineColorBlendAttachmentState *blendAttach = blendInfo->pAttachments;
        VkColorComponentFlags colorMask;
        __vkAttachmentDesc *attachMent;
        VkBool32 sRGB = VK_FALSE;
        HwPEDesc hwPEDesc;
        uint32_t hwRtIndex;
        uint32_t partIndex = 0;
        VkBool32 is16BitStorage = VK_FALSE;

        __VK_ASSERT(subPass->colorCount == blendInfo->attachmentCount);

        attachMent = &rdp->attachments[subPass->color_attachment_index[i]];
        is16BitStorage = (fragOutTable[i].resEntryBit & VSC_RES_ENTRY_BIT_16BIT) != 0;
        __VK_VERIFY_OK(halti5_helper_convertHwPEDesc(devCtx, attachMent->formatInfo->residentImgFormat, is16BitStorage, &hwPEDesc));
        switch (attachMent->formatInfo->residentImgFormat)
        {
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            sRGB = VK_TRUE;
            break;
        default:
            break;
        }

        while (partIndex < chipGfxPipeline->patchOutput.outputs[i].partCount)
        {
            hwRtIndex = halti5_convertLocationToRenderIndex(hints, chipGfxPipeline->patchOutput.outputs[i].locations[partIndex]);

            if (hwRtIndex == 0xFFFFFFFF)
            {
                partIndex++;
                continue;
            }

            colorMask = halti5_helper_GetColorMask(devCtx, blendAttach[i].colorWriteMask,
                hwPEDesc.hwFormat, attachMent->formatInfo->residentImgFormat, partIndex);

            if (hwRtIndex == 0)
            {
                chipGfxPipeline->destinationRead = !chipGfxPipeline->depthOnly
                    && (msaaEnable || chipGfxPipeline->anyBlendEnabled || chipGfxPipeline->anyPartialColorWrite);

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x050B, VK_FALSE,
                      (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (hwPEDesc.hwFormat) & ((gctUINT32) ((((1 ?
 29:24) - (0 ?
 29:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:24) - (0 ?
 29:24) + 1))))))) << (0 ?
 29:24))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31))))
#if !__VK_ENABLETS
                    & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (!chipGfxPipeline->destinationRead) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))))
#endif
                    & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (colorMask) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12))))
                    & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (!chipGfxPipeline->colorPipeEnable) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))));

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, VK_FALSE, (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (sRGB) & ((gctUINT32) ((((1 ?
 31:31) - (0 ?
 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:31) - (0 ?
 31:31) + 1))))))) << (0 ?
 31:31))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))));
            }
            else
            {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5248 + (hwRtIndex-1), VK_FALSE,
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
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (colorMask) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))));
            }
            partIndex++;
        }
    }

    /* step 3: depth programming */
    if (dsInfo && subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        __vkAttachmentDesc *attachMent;
        chipGfxPipeline->earlyZ = VK_TRUE;
        attachMent = &rdp->attachments[subPass->dsAttachIndex];
        if ((dsInfo->depthCompareOp == VK_COMPARE_OP_NOT_EQUAL) ||
            (dsInfo->stencilTestEnable &&
             ((dsInfo->front.passOp != VK_STENCIL_OP_KEEP) ||
              (dsInfo->front.failOp != VK_STENCIL_OP_KEEP) ||
              (dsInfo->front.depthFailOp != VK_STENCIL_OP_KEEP))) ||
              (psEarlyFragmentTest ? VK_FALSE :((hints->psHasFragDepthOut)||psHasMemoryAccess)))
        {
            chipGfxPipeline->earlyZ = VK_FALSE;
        }

        depthMode = ((dsInfo->stencilTestEnable || dsInfo->depthTestEnable) && hasDsSurface) ? 0x1 : 0x0;

        if (hasDsSurface &&
            (attachMent->formatInfo->residentImgFormat != VK_FORMAT_D16_UNORM && attachMent->formatInfo->residentImgFormat != VK_FORMAT_D32_SFLOAT)
            && dsInfo->stencilTestEnable)
        {
            stencilMode = 0x2;
        }
        else
        {
            stencilMode = 0x0;
        }
        chipGfxPipeline->depthCompareOp = (dsInfo->depthTestEnable && hasDsSurface) ? dsInfo->depthCompareOp : VK_COMPARE_OP_ALWAYS;

        fullfuncZ = dsInfo->depthWriteEnable || !chipGfxPipeline->earlyZ || dsInfo->stencilTestEnable;
        fullfuncZ = fullfuncZ && hasDsSurface;
    }
    else
    {
        chipGfxPipeline->earlyZ = VK_FALSE;

        depthMode = 0x0;
        stencilMode = 0x0;
        chipGfxPipeline->depthCompareOp = VK_COMPARE_OP_ALWAYS;
        fullfuncZ = VK_FALSE;
    }
    chipGfxPipeline->peDepth = !chipGfxPipeline->colorPipeEnable || chipGfxPipeline->depthOnly;

    if (database->REG_RAWriteDepth)
    {
        gctBOOL useSubSampleZInPS = gcvFALSE;
        gctBOOL sampleMaskInPos = gcvFALSE;
        VkBool32 psReadW = hints->useFragCoord[3] || (hints->rtArrayComponent != -1);
        VkBool32 raDepthWrite = !(hints->psHasFragDepthOut || hints->hasKill || psHasMemoryAccess)
                             && !msaaFragmentOp
                             && !chipGfxPipeline->peDepth
                             && fullfuncZ;

        /* If need full function z but ra depth couldn't be on, we need peDepth help */
        if (fullfuncZ && !raDepthWrite)
        {
            chipGfxPipeline->peDepth = VK_TRUE;
        }

        if (!hasDsSurface)
        {
            regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
        }
        else if (raDepthWrite)
        {
            regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
        }
        else
        {
            regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)));
        }

        /* SHADER_SUB_SAMPLES control whether PE need sub sample depth or not. */
        if (chipGfxPipeline->peDepth && msaaEnable)
        {
            regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));

            useSubSampleZInPS = gcvTRUE;
        }
        else
        {
            regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)));
        }

        /* If compiler use sampleMask, compiler decide sample mask location firstly.*/
        if (hints->sampleMaskLoc != -1)
        {
            gcmASSERT(!chipGfxPipeline->depthOnly);

            if (hints->sampleMaskLoc == 0x0)
            {
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (hints->useFragCoord[2] || chipGfxPipeline->peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));

                useSubSampleZInPS = gcvTRUE;
            }
            else if (hints->sampleMaskLoc == 0x1)
            {
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));

                sampleMaskInPos = gcvTRUE;

            }
            /* 0x2 */
            else
            {
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (hints->useFragCoord[2] || chipGfxPipeline->peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));

                sampleMaskInPos = gcvTRUE;
            }
        }
        /* Else, driver decide sample mask location. */
        else
        {
            if (database->PSIO_SAMPLEMASK_IN_R0ZW_FIX)
            {
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) ((hints->useFragCoord[2] || chipGfxPipeline->peDepth)) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));

                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));

                if (useSubSampleZInPS)
                {
                    regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                }
                else if (!hints->useFragCoord[2] && !chipGfxPipeline->peDepth)
                {
                    regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                    sampleMaskInPos = gcvTRUE;
                }
                else
                {
                    regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));
                    useSubSampleZInPS = gcvTRUE;
                }
            }
            else
            {
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (hints->useFragCoord[2] || chipGfxPipeline->peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)));
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (psReadW) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));


                /* Always put in SubZ if compiler doesn't decide it. As in this case, compiler wont
                ** reserve r1 for highp under dual16 mode.
                */
                regRAControl = ((((gctUINT32) (regRAControl)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:30) - (0 ?
 31:30) + 1))))))) << (0 ?
 31:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:30) - (0 ?
 31:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:30) - (0 ? 31:30) + 1))))))) << (0 ? 31:30)));

                useSubSampleZInPS = gcvTRUE;
            }
        }

        chipGfxPipeline->subSampleZUsedInPS = useSubSampleZInPS;
        chipGfxPipeline->sampleMaskInPos = sampleMaskInPos;
    }
    else
    {
        chipGfxPipeline->peDepth = chipGfxPipeline->peDepth || fullfuncZ;
    }

    regDepthConfig = (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (depthMode) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3))))
                   & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (chipGfxPipeline->depthOnly) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21))))
                   & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (chipGfxPipeline->earlyZ) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))))
                   & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (!chipGfxPipeline->peDepth) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:25) - (0 ?
 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 25:25) - (0 ?
 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25))))
                   & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))));

    if (dsInfo && subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        gcuFLOAT_UINT32 depthNormalize = {0};
        uint32_t dsVkFormat = __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32;
        uint32_t stencilFormat = 0x0;

        regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (dsInfo->depthWriteEnable) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))));
        regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (s_xlateDepthCompare[chipGfxPipeline->depthCompareOp]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))));

        if (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
        {
            __vkAttachmentDesc *attachment = &rdp->attachments[subPass->dsAttachIndex];
            dsVkFormat = attachment->formatInfo->residentImgFormat;
        }

        switch (dsVkFormat)
        {
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
            regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));
            regRAControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (chipGfxPipeline->earlyZ) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
            depthNormalize.f = gcoMATH_UInt2Float(0xFFFFFF);
            break;
        case VK_FORMAT_D16_UNORM:
            regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));
            regRAControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (chipGfxPipeline->earlyZ) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
            depthNormalize.f = gcoMATH_UInt2Float(0xFFFF);
            break;
        case VK_FORMAT_S8_UINT:
            /*for s8, we always disable depthWriteEnable to avoid depth buffer has affect on stencil buffer in msaa*/
            regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))));
            regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));
            stencilFormat = 0x1;
            regRAControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
            break;
        default:
            regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) &  ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));
            regRAControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (chipGfxPipeline->earlyZ) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)));
            depthNormalize.f = gcoMATH_UInt2Float(0xFFFFFF);
            __VK_ASSERT(!"invalid depth attachment format\n");
            break;
        }

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0503, VK_FALSE, depthNormalize.u);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, VK_FALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (stencilFormat) & ((gctUINT32) ((((1 ?
 21:20) - (0 ?
 21:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:20) - (0 ?
 21:20) + 1))))))) << (0 ?
 21:20))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))));

        if (dsInfo->stencilTestEnable && hasDsSurface)
        {
            if (rsInfo->frontFace == VK_FRONT_FACE_CLOCKWISE)
            {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0506, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilCompare[dsInfo->front.compareOp]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.passOp]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.failOp]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.depthFailOp]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilCompare[dsInfo->back.compareOp]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.passOp]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.failOp]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.depthFailOp]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))));
            }
            else
            {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0506, VK_FALSE,
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilCompare[dsInfo->back.compareOp]) & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.passOp]) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.failOp]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->back.depthFailOp]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilCompare[dsInfo->front.compareOp]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.passOp]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.failOp]) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (s_xlateStencilOperation[dsInfo->front.depthFailOp]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))));
            }
        }
        else
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0506, VK_FALSE,
                  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ? 10:8)))
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
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x7 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
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
 30:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28))));
        }

        if (!(pip->dynamicStates & (__VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT
                                   | __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT
                                   | __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT)))
        {
            __vkDynamicStencilState stencilState;
            stencilState.compareMask[__VK_FACE_FRONT] = dsInfo->front.compareMask;
            stencilState.reference[__VK_FACE_FRONT] = dsInfo->front.reference;
            stencilState.writeMask[__VK_FACE_FRONT] = dsInfo->front.writeMask;
            stencilState.compareMask[__VK_FACE_BACK] = dsInfo->back.compareMask;
            stencilState.reference[__VK_FACE_BACK] = dsInfo->back.reference;
            stencilState.writeMask[__VK_FACE_BACK] = dsInfo->back.writeMask;

            halti5_helper_set_stencilStates(devCtx, rsInfo->frontFace, &pCmdBuffer, &stencilState, stencilMode);
        }
    }
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0507, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (stencilMode) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            );
        regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (s_xlateDepthCompare[chipGfxPipeline->depthCompareOp]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ?
 10:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:8) - (0 ?
 10:8) + 1))))))) << (0 ?
 10:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))));
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0500, VK_FALSE, regDepthConfig);
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0382, VK_FALSE, regRAControl);

    chipGfxPipeline->regDepthConfig = regDepthConfig;
    chipGfxPipeline->regRAControl = regRAControl;
    chipGfxPipeline->stencilMode = stencilMode;

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

#define gcvINVALID_BLEND_MODE   ~0U

static VkResult halti5_pip_emit_blend(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    uint32_t  *pCmdBuffer, *pCmdBufferBegin;
    const VkPipelineColorBlendStateCreateInfo *blendInfo = info->pColorBlendState;
    uint32_t i;
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;

    static const gctUINT32 s_xlateAlphaFuncSource[] =
    {
        /* VK_BLEND_FACTOR_ZERO */
        0x0,
        /* VK_BLEND_FACTOR_ONE */
        0x1,
        /* VK_BLEND_FACTOR_SRC_COLOR */
        0x2,
        /* VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR */
        0x3,
        /* VK_BLEND_FACTOR_DST_COLOR */
        0x8,
        /* VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR */
        0x9,
        /* VK_BLEND_FACTOR_SRC_ALPHA */
        0x4,
        /* VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA */
        0x5,
        /* VK_BLEND_FACTOR_DST_ALPHA */
        0x6,
        /* VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA */
        0x7,
        /* VK_BLEND_FACTOR_CONSTANT_COLOR */
        0xD,
        /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR */
        0xE,
        /* VK_BLEND_FACTOR_CONSTANT_ALPHA */
        0xB,
        /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA */
        0xC,
        /* VK_BLEND_FACTOR_SRC_ALPHA_SATURATE */
        0xA
    };

    static const gctUINT32 s_xlateAlphaFuncTarget[] =
    {
        /* VK_BLEND_FACTOR_ZERO */
        0x0,
        /* VK_BLEND_FACTOR_ONE */
        0x1,
        /* VK_BLEND_FACTOR_SRC_COLOR */
        0x2,
        /* VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR */
        0x3,
        /* VK_BLEND_FACTOR_DST_COLOR */
        0x8,
        /* VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR */
        0x9,
        /* VK_BLEND_FACTOR_SRC_ALPHA */
        0x4,
        /* VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA */
        0x5,
        /* VK_BLEND_FACTOR_DST_ALPHA */
        0x6,
        /* VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA */
        0x7,
        /* VK_BLEND_FACTOR_CONSTANT_COLOR */
        0xD,
        /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR */
        0xE,
        /* VK_BLEND_FACTOR_CONSTANT_ALPHA */
        0xB,
        /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA */
        0xC,
        /* VK_BLEND_FACTOR_SRC_ALPHA_SATURATE */
        0xA
    };

    static const gctUINT32 s_xlateAlphaMode[] =
    {
        0x0, /* VK_BLEND_OP_ADD */
        0x1, /* VK_BLEND_OP_SUBTRACT */
        0x2, /* VK_BLEND_OP_REVERSE_SUBTRACT */
        0x3, /* VK_BLEND_OP_MIN */
        0x4, /* VK_BLEND_OP_MAX */
    };

    static const gctUINT32 s_xlateAlphaModeAdvanced[] =
    {
        0x0, /* VK_BLEND_OP_ADD */
        0x0, /* VK_BLEND_OP_SUBTRACT */
        0x0, /* VK_BLEND_OP_REVERSE_SUBTRACT */
        0x0, /* VK_BLEND_OP_MIN */
        0x0, /* VK_BLEND_OP_MAX */
        0x1, /* ?? */
        0x2, /* ?? */
        0x3, /* ?? */
        0x4, /* ?? */
        0x5, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        0x6, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        0x7, /* ?? */
        0x8, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
        gcvINVALID_BLEND_MODE, /* ?? */
    };

    if (!blendInfo)
    {
        return VK_SUCCESS;
    }

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    for (i = 0; i < blendInfo->attachmentCount; i++)
    {
        const VkPipelineColorBlendAttachmentState * attachState = &blendInfo->pAttachments[i];
        uint32_t alphaConfig = 0, alphaControl = 0;

        if (attachState->blendEnable && database->ALPHA_BLENDING_OPT)
        {
            VkBool32 modeAdd, modeSub;
            VkBool32 srcAlphaZeroKill, srcAlphaOneKill, srcColorAlphaZeroKill, srcColorAlphaOneKill;

            modeAdd = (attachState->colorBlendOp == VK_BLEND_OP_ADD)&&
                (attachState->alphaBlendOp == VK_BLEND_OP_ADD);
            modeSub = (attachState->colorBlendOp == VK_BLEND_OP_SUBTRACT) &&
                (attachState->alphaBlendOp == VK_BLEND_OP_SUBTRACT);

            if (database->SH_SUPPORT_ALPHA_KILL && modeAdd)
            {
                srcAlphaZeroKill = (attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA) &&
                    (attachState->srcColorBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA) &&
                    (attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) &&
                    (attachState->dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);

                srcAlphaOneKill = (attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) &&
                    (attachState->srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA) &&
                    (attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA) &&
                    (attachState->dstColorBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA);

                srcColorAlphaZeroKill = ((attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA
                    || attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_SRC_COLOR) &&
                    (attachState->srcColorBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA
                    || attachState->srcColorBlendFactor == VK_BLEND_FACTOR_SRC_COLOR) &&
                    (attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                    || attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR) &&
                    (attachState->dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA))
                    || (attachState->dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);

                srcColorAlphaOneKill = (attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                    || attachState->srcAlphaBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR) &&
                    (attachState->srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
                    || attachState->srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR) &&
                    (attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA
                    || attachState->dstAlphaBlendFactor == VK_BLEND_FACTOR_SRC_COLOR) &&
                    (attachState->dstColorBlendFactor == VK_BLEND_FACTOR_SRC_ALPHA
                    || attachState->dstColorBlendFactor == VK_BLEND_FACTOR_SRC_COLOR);

                if (srcAlphaZeroKill)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)));
                }

                if (srcAlphaOneKill)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)));

                }

                if (srcColorAlphaZeroKill)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15)));

                }

                if (srcColorAlphaOneKill)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));
                }
            }

            if (modeAdd || modeSub)
            {
                VkBlendFactor srcFuncColor = attachState->srcColorBlendFactor;
                VkBlendFactor srcFuncAlpha = attachState->srcAlphaBlendFactor;
                VkBlendFactor trgFuncColor = attachState->dstColorBlendFactor;
                VkBlendFactor trgFuncAlpha = attachState->dstAlphaBlendFactor;

                static const VkBool32 s_xlateAlphaFuncOpt[] =
                {
                    /* VK_BLEND_FACTOR_ZERO */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_ONE */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_SRC_COLOR */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_DST_COLOR */
                    VK_FALSE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR */
                    VK_FALSE,
                    /* VK_BLEND_FACTOR_SRC_ALPHA */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_DST_ALPHA */
                    VK_FALSE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA */
                    VK_FALSE,
                    /* VK_BLEND_FACTOR_CONSTANT_COLOR */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_CONSTANT_ALPHA */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA */
                    VK_TRUE,
                    /* VK_BLEND_FACTOR_SRC_ALPHA_SATURATE */
                    VK_TRUE
                };


                VkBool32 srcAlphaOneNoRead = trgFuncAlpha == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA &&
                    trgFuncColor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA &&
                    s_xlateAlphaFuncOpt[srcFuncAlpha] && s_xlateAlphaFuncOpt[srcFuncColor] &&
                    s_xlateAlphaFuncOpt[trgFuncAlpha] && s_xlateAlphaFuncOpt[trgFuncColor];

                VkBool32 srcAlphaZeroNoRead = trgFuncAlpha == VK_BLEND_FACTOR_SRC_ALPHA &&
                    trgFuncColor == VK_BLEND_FACTOR_SRC_ALPHA &&
                    s_xlateAlphaFuncOpt[srcFuncAlpha] && s_xlateAlphaFuncOpt[srcFuncColor] &&
                    s_xlateAlphaFuncOpt[trgFuncAlpha] && s_xlateAlphaFuncOpt[trgFuncColor];

                VkBool32 srcColorAlphaOneNoRead = (trgFuncAlpha == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA || trgFuncAlpha == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR) &&
                    (trgFuncColor == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ||  trgFuncColor == VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR) &&
                    s_xlateAlphaFuncOpt[srcFuncAlpha] && s_xlateAlphaFuncOpt[srcFuncColor] &&
                    s_xlateAlphaFuncOpt[trgFuncAlpha] && s_xlateAlphaFuncOpt[trgFuncColor];

                VkBool32 srcColorAlphaZeroNoRead = (trgFuncAlpha == VK_BLEND_FACTOR_SRC_ALPHA || trgFuncAlpha == VK_BLEND_FACTOR_SRC_COLOR) &&
                    (trgFuncColor == VK_BLEND_FACTOR_SRC_ALPHA || trgFuncColor == VK_BLEND_FACTOR_SRC_COLOR) &&
                    s_xlateAlphaFuncOpt[srcFuncAlpha] && s_xlateAlphaFuncOpt[srcFuncColor] &&
                    s_xlateAlphaFuncOpt[trgFuncAlpha] && s_xlateAlphaFuncOpt[trgFuncColor];

                if (srcAlphaOneNoRead)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17)));
                }

                if (srcAlphaZeroNoRead)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
                }

                if (srcColorAlphaOneNoRead)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18)));
                }

                if (srcColorAlphaZeroNoRead)
                {
                    alphaConfig |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
                    alphaControl |=  ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));
                }
            }
        }

        __VK_ASSERT(attachState->srcColorBlendFactor < __VK_COUNTOF(s_xlateAlphaFuncSource));
        __VK_ASSERT(attachState->srcAlphaBlendFactor < __VK_COUNTOF(s_xlateAlphaFuncSource));
        __VK_ASSERT(attachState->dstColorBlendFactor < __VK_COUNTOF(s_xlateAlphaFuncSource));
        __VK_ASSERT(attachState->dstAlphaBlendFactor < __VK_COUNTOF(s_xlateAlphaFuncSource));

        if (i == 0)
        {
            alphaConfig = ((((gctUINT32) (alphaConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaModeAdvanced[attachState->colorBlendOp]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x050A, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (attachState->blendEnable) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (attachState->blendEnable) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncSource[attachState->srcColorBlendFactor]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncSource[attachState->srcAlphaBlendFactor]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncTarget[attachState->dstColorBlendFactor]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncTarget[attachState->dstAlphaBlendFactor]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaMode[attachState->colorBlendOp]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaMode[attachState->alphaBlendOp]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                );
        }
        else
        {
            alphaControl |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaMode[attachState->alphaBlendOp]) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncTarget[attachState->dstAlphaBlendFactor]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncSource[attachState->srcAlphaBlendFactor]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:12) - (0 ?
 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaMode[attachState->colorBlendOp]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ?
 14:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ? 14:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncTarget[attachState->dstColorBlendFactor]) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (s_xlateAlphaFuncSource[attachState->srcColorBlendFactor]) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (attachState->blendEnable) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5258 + (i-1), VK_FALSE, alphaControl);
        }

    }

    if (!(pip->dynamicStates & __VK_DYNAMIC_STATE_BLEND_CONSTANTS_BIT))
    {
        __vkDynamicColorBlendState blendConstants;
        __VK_MEMCOPY(&blendConstants.blendConstants[0], &blendInfo->blendConstants[0], 4 * sizeof(float));
        halti5_helper_set_blendConstants(devCtx, &pCmdBuffer, &blendConstants, blendInfo->attachmentCount);

    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;

}


static VkResult halti5_pip_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    uint32_t handleIdx;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(&pip->memCb);

    for (handleIdx = 0; handleIdx < chipModule->tweakHandleCount; handleIdx++)
    {
        halti5_tweak_handler *handler = chipModule->ppTweakHandlers[handleIdx];

        if ((*handler->match)(devCtx, pip, createInfo))
        {
            chipPipeline->tweakHandler = (halti5_tweak_handler *)__VK_ALLOC(sizeof(halti5_tweak_handler), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR(chipPipeline->tweakHandler  ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
            __VK_MEMCOPY(chipPipeline->tweakHandler, handler, sizeof(halti5_tweak_handler));
            __VK_ONERROR((*handler->tweak)(devCtx, pip, createInfo, chipPipeline->tweakHandler));
            break;
        }
    }

    return VK_SUCCESS;

OnError:
    if (chipPipeline->tweakHandler)
    {
        __VK_FREE(chipPipeline->tweakHandler);
    }
    return result;
}


static VkResult halti5_pip_build_prepare(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    __vkRenderPass *rdp = pip->renderPass;
    __vkRenderSubPassInfo *subPass = &rdp->subPassInfo[pip->subPass];
    VkBool32 has16orLessBppImage = VK_FALSE;
    VkBool32 has8bitCb = VK_FALSE, all8bitCb = VK_TRUE;
    VkBool32 allColorWriteOff = VK_TRUE, anyPartialColorWrite = VK_FALSE;
    VkBool32 anyBlendEnabled = VK_FALSE;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;

    if (info->pColorBlendState)
    {
        const VkPipelineColorBlendAttachmentState *blendAttachments = info->pColorBlendState->pAttachments;
        uint32_t i;

        for (i = 0; i < subPass->colorCount; i++)
        {
            const __vkFormatInfo *attachFmtInfo;
            if (subPass->color_attachment_index[i] == VK_ATTACHMENT_UNUSED)
            {
                continue;
            }

            attachFmtInfo = rdp->attachments[subPass->color_attachment_index[i]].formatInfo;
            if ((blendAttachments[i].colorWriteMask != 0x0) && allColorWriteOff)
            {
                allColorWriteOff = VK_FALSE;
            }

            if ((blendAttachments[i].colorWriteMask !=
                (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT))
                && (!anyPartialColorWrite))
            {
                anyPartialColorWrite = VK_TRUE;
            }

            if (blendAttachments[i].blendEnable && !anyBlendEnabled)
            {
                anyBlendEnabled = VK_TRUE;
            }

            if (attachFmtInfo->bitsPerBlock <= 16)
            {
                has16orLessBppImage = VK_TRUE;
            }

            if ((attachFmtInfo->bitsPerBlock == 8) &&
                (rdp->attachments[subPass->color_attachment_index[i]].sampleCount <= 1))
            {
                has8bitCb = VK_TRUE;
            }

            else
            {
                all8bitCb = VK_FALSE;
            }

            chipGfxPipeline->patchOutput.outputs[i].locations[0] = i;
            chipGfxPipeline->patchOutput.outputs[i].partCount = attachFmtInfo->partCount;
            chipGfxPipeline->patchOutput.outputs[i].format = attachFmtInfo->residentImgFormat;

            if (attachFmtInfo->partCount > 1)
            {
                chipGfxPipeline->patchOutput.mask |= (1 << i);
                chipGfxPipeline->patchOutput.count++;
            }
        }
    }

    chipGfxPipeline->hasOne8bitCb = has8bitCb && !all8bitCb;

    if (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        if (rdp->attachments[subPass->dsAttachIndex].formatInfo->bitsPerBlock <= 16)
        {
            has16orLessBppImage = VK_TRUE;
        }
    }
    chipGfxPipeline->has16orLessBppImage = has16orLessBppImage;
    chipGfxPipeline->allColorWriteOff = allColorWriteOff;
    chipGfxPipeline->anyPartialColorWrite = anyPartialColorWrite;
    chipGfxPipeline->anyBlendEnabled = anyBlendEnabled;
    chipGfxPipeline->colorPipeEnable = (subPass->colorCount > 0) && !allColorWriteOff;

    return VK_SUCCESS;
}

static VkResult halti5_pip_build_check(
    __vkDevContext *devCtx,
    __vkPipeline *pip
    )
{
     uint32_t i = 0;

    /*check the valid descriptorsetLayout*/
    if (pip->pipelineLayout)
    {
        uint32_t descSetLayoutCount = pip->pipelineLayout->descSetLayoutCount;
        uint32_t realCount = descSetLayoutCount;
        for (i = 0; i < descSetLayoutCount; i++)
        {
            __vkDescriptorSetLayout *descSetLayout = pip->pipelineLayout->descSetLayout[i];
            if (descSetLayout->validFlag != 0xff)
            {
                realCount--;
                if ((i+1) < descSetLayoutCount)
                {
                    pip->pipelineLayout->descSetLayout[i] = pip->pipelineLayout->descSetLayout[i+1];
                }
            }
        }
        pip->pipelineLayout->descSetLayoutCount = realCount;
    }

    return VK_SUCCESS;
}

static void halti5_helper_destroyVscResLayout(
    __vkPipeline *pip
    )
{
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;

    if (chipPipeline->vscResLayout)
    {
        uint32_t i;
        VSC_PROGRAM_RESOURCE_LAYOUT *pVscResLayout = chipPipeline->vscResLayout;
        __VK_SET_ALLOCATIONCB(&pip->memCb);

        if (pVscResLayout->pResourceSets)
        {
            for (i = 0; i < pVscResLayout->resourceSetCount; i++)
            {
                VSC_PROGRAM_RESOURCE_SET *pResourceSet = &pVscResLayout->pResourceSets[i];

                if (pResourceSet->pResouceBindings)
                {
                    __VK_ASSERT(pResourceSet->resourceBindingCount);
                    __VK_FREE(pResourceSet->pResouceBindings);
                }
            }
            __VK_FREE(pVscResLayout->pResourceSets);
        }

        if (pVscResLayout->pPushConstantRanges)
        {
            __VK_ASSERT(pVscResLayout->pushConstantRangeCount);
            __VK_FREE(pVscResLayout->pPushConstantRanges);
        }

        __VK_FREE(pVscResLayout);
        chipPipeline->vscResLayout = VK_NULL_HANDLE;
    }
    return;
}


static VkResult halti5_helper_createVscResLayout(
    __vkPipeline *pip
    )
{
    uint32_t i, j;
    VkResult result = VK_SUCCESS;
    __vkPipelineLayout *pipLayout = pip->pipelineLayout;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;

    if (!pipLayout)
    {
        return VK_SUCCESS;
    }

    if (!chipPipeline->vscResLayout)
    {
        VSC_PROGRAM_RESOURCE_LAYOUT *pVscResLayout = gcvNULL;
        __VK_SET_ALLOCATIONCB(&pip->memCb);

        pVscResLayout = (VSC_PROGRAM_RESOURCE_LAYOUT*)__VK_ALLOC(sizeof(VSC_PROGRAM_RESOURCE_LAYOUT), 8,
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        if (!pVscResLayout)
        {
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        __VK_MEMZERO(pVscResLayout, sizeof(VSC_PROGRAM_RESOURCE_LAYOUT));

        chipPipeline->vscResLayout = pVscResLayout;

        if (pipLayout->pushConstantRangeCount)
        {
            pVscResLayout->pushConstantRangeCount = pipLayout->pushConstantRangeCount;
            pVscResLayout->pPushConstantRanges = (VSC_PROGRAM_PUSH_CONSTANT_RANGE *)
                __VK_ALLOC(pVscResLayout->pushConstantRangeCount * sizeof(VSC_PROGRAM_PUSH_CONSTANT_RANGE),
                8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR(pVscResLayout->pPushConstantRanges? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
            __VK_MEMZERO(pVscResLayout->pPushConstantRanges,
                         pVscResLayout->pushConstantRangeCount * sizeof(VSC_PROGRAM_PUSH_CONSTANT_RANGE));
            for (i = 0; i < pipLayout->pushConstantRangeCount; i++)
            {
                VSC_PROGRAM_PUSH_CONSTANT_RANGE *pVscPushConstant = &pVscResLayout->pPushConstantRanges[i];
                VkPushConstantRange *pVkPushConstant = &pipLayout->pushConstantRanges[i];
                pVscPushConstant->shPushConstRange.offset = pVkPushConstant->offset;
                pVscPushConstant->shPushConstRange.size = pVkPushConstant->size;
                pVscPushConstant->stageBits = (VSC_SHADER_STAGE_BIT)pVkPushConstant->stageFlags;
            }
        }

        if (pipLayout->descSetLayoutCount)
        {
            pVscResLayout->resourceSetCount = pipLayout->descSetLayoutCount;
            pVscResLayout->pResourceSets = (VSC_PROGRAM_RESOURCE_SET *)__VK_ALLOC(
                pVscResLayout->resourceSetCount * sizeof(VSC_PROGRAM_RESOURCE_SET),
                8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR(pVscResLayout->pResourceSets ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
            __VK_MEMZERO(pVscResLayout->pResourceSets,
                         pVscResLayout->resourceSetCount * sizeof(VSC_PROGRAM_RESOURCE_SET));
            for (i = 0; i < pipLayout->descSetLayoutCount; i++)
            {
                __vkDescriptorSetLayout *pVkDescSetLayout = pipLayout->descSetLayout[i];
                VSC_PROGRAM_RESOURCE_SET *pVscResourceSet = &pVscResLayout->pResourceSets[i];

                if (pVkDescSetLayout->bindingCount)
                {
                    pVscResourceSet->resourceBindingCount = pVkDescSetLayout->bindingCount;
                    pVscResourceSet->pResouceBindings = (VSC_PROGRAM_RESOURCE_BINDING *)__VK_ALLOC(
                        pVscResourceSet->resourceBindingCount * sizeof(VSC_PROGRAM_RESOURCE_BINDING),
                        8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

                    __VK_ONERROR(pVscResourceSet->pResouceBindings ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(pVscResourceSet->pResouceBindings,
                                 pVscResourceSet->resourceBindingCount * sizeof(VSC_PROGRAM_RESOURCE_BINDING));
                    for (j = 0; j < pVkDescSetLayout->bindingCount; j++)
                    {
                        VSC_PROGRAM_RESOURCE_BINDING *pVscResourceBinding = &pVscResourceSet->pResouceBindings[j];
                        pVscResourceBinding->shResBinding.binding = pVkDescSetLayout->binding[j].std.binding;
                        pVscResourceBinding->shResBinding.arraySize = pVkDescSetLayout->binding[j].std.descriptorCount;
                        pVscResourceBinding->shResBinding.type = (VSC_SHADER_RESOURCE_TYPE)pVkDescSetLayout->binding[j].std.descriptorType;
                        pVscResourceBinding->shResBinding.set = i;
                        pVscResourceBinding->stageBits = (VSC_SHADER_STAGE_BIT)pVkDescSetLayout->binding[j].std.stageFlags;
                    }
                }
            }
        }
    }

    return VK_SUCCESS;

OnError:

    halti5_helper_destroyVscResLayout(pip);
    return result;
}

static void halti5_helper_destroyVscShaderResLayout(
    __vkPipeline *pip,
    VSC_SHADER_RESOURCE_LAYOUT* pShResourceLayout
    )
{
    if (pShResourceLayout)
    {
        __VK_SET_ALLOCATIONCB(&pip->memCb);

        if (pShResourceLayout->pPushConstantRanges)
        {
            __VK_FREE(pShResourceLayout->pPushConstantRanges);
            pShResourceLayout->pPushConstantRanges = gcvNULL;
        }

        if (pShResourceLayout->pResBindings)
        {
            __VK_FREE(pShResourceLayout->pResBindings);
            pShResourceLayout->pResBindings = gcvNULL;
        }
    }
}

static VkResult halti5_helper_createVscShaderResLayout(
    __vkPipeline *pip,
    VSC_PROGRAM_RESOURCE_LAYOUT* pPgResourceLayout,
    uint32_t shStage,
    VSC_SHADER_RESOURCE_LAYOUT* pOutShResourceLayout
    )
{
    VkResult result = VK_SUCCESS;
    gctUINT  i, j, resBindingCount = 0, pushCnstRangeCount = 0;

    __VK_SET_ALLOCATIONCB(&pip->memCb);

    /* Res binding */
    if (pPgResourceLayout->resourceSetCount)
    {
        for (i = 0; i < pPgResourceLayout->resourceSetCount; i++)
        {
            for (j = 0; j < pPgResourceLayout->pResourceSets[i].resourceBindingCount; j++)
            {
                if (pPgResourceLayout->pResourceSets[i].pResouceBindings[j].stageBits &
                    VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                {
                    resBindingCount++;
                }
            }
        }

        if (resBindingCount)
        {
            pOutShResourceLayout->resourceBindingCount = resBindingCount;
            pOutShResourceLayout->pResBindings =
                (VSC_SHADER_RESOURCE_BINDING*)__VK_ALLOC(resBindingCount * sizeof(VSC_SHADER_RESOURCE_BINDING),
                                                         8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR(pOutShResourceLayout->pResBindings ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

            resBindingCount = 0;
            for (i = 0; i < pPgResourceLayout->resourceSetCount; i++)
            {
                for (j = 0; j < pPgResourceLayout->pResourceSets[i].resourceBindingCount; j++)
                {
                    if (pPgResourceLayout->pResourceSets[i].pResouceBindings[j].stageBits &
                        VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                    {
                        __VK_MEMCOPY(&pOutShResourceLayout->pResBindings[resBindingCount],
                                     &pPgResourceLayout->pResourceSets[i].pResouceBindings[j].shResBinding,
                                     sizeof(VSC_SHADER_RESOURCE_BINDING));

                        resBindingCount++;
                    }
                }
            }
        }
    }

    /* Push-constant */
    if (pPgResourceLayout->pushConstantRangeCount)
    {
        for (i = 0; i < pPgResourceLayout->pushConstantRangeCount; i++)
        {
            if (pPgResourceLayout->pPushConstantRanges[i].stageBits &
                VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
            {
                pushCnstRangeCount++;
            }
        }

        if (pushCnstRangeCount)
        {
            pOutShResourceLayout->pushConstantRangeCount = pushCnstRangeCount;
            pOutShResourceLayout->pPushConstantRanges =
                (VSC_SHADER_PUSH_CONSTANT_RANGE*)__VK_ALLOC(pushCnstRangeCount * sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE),
                                                            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            __VK_ONERROR(pOutShResourceLayout->pPushConstantRanges ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

            pushCnstRangeCount = 0;
            for (i = 0; i < pPgResourceLayout->pushConstantRangeCount; i++)
            {
                if (pPgResourceLayout->pPushConstantRanges[i].stageBits &
                    VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                {
                    __VK_MEMCOPY(&pOutShResourceLayout->pPushConstantRanges[pushCnstRangeCount],
                                 &pPgResourceLayout->pPushConstantRanges[i],
                                 sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));

                    pushCnstRangeCount++;
                }
            }
        }
    }
OnError:
    return result;
}

#define VSC_PROG_INSTANCE_HASH_ENTRY_NUM   32
#define vSC_PROG_INSTANCE_HASH_ENTRY_SIZE  32

void halti5_free_vscprogram_instance(
    VkAllocationCallbacks *pAllocator,
    void *pUserData
    )
{
    halti5_vscprogram_instance * instance = (halti5_vscprogram_instance *)pUserData;
    __vkDevContext *devCtx = instance->devCtx;
    __VK_SET_ALLOCATIONCB(pAllocator);

    vscFinalizePEP(&instance->pep);
    vscFinalizeHwPipelineShadersStates(&devCtx->vscSysCtx,&instance->hwStates);

    __VK_FREE(instance);
    return;
}

static VSC_IMAGE_FORMAT halti5_pip_getImageFormat(
    PROG_VK_IMAGE_DERIVED_INFO* pImageDerivedInfo,
    uint32_t imageDerivedInfoCount
    )
{
    uint32_t i;
    VSC_IMAGE_FORMAT imageFormat = VSC_IMAGE_FORMAT_NONE;

    for (i = 0; i < imageDerivedInfoCount; i++)
    {
        if (pImageDerivedInfo[i].imageFormatInfo.imageFormat != VSC_IMAGE_FORMAT_NONE)
        {
            imageFormat = pImageDerivedInfo[i].imageFormatInfo.imageFormat;
            break;
        }
    }

    return imageFormat;
}

static void halti5_pip_build_patchKeyMask(
    __vkPipeline *pip
    )
{
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    uint32_t setIdx;

    for (setIdx = 0; setIdx < __VK_MAX_DESCRIPTOR_SETS; setIdx++)
    {
        if (chipPipeline->patchKeys[setIdx])
        {
            __VK_MEMSET(chipPipeline->patchKeys[setIdx], 0, sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[setIdx]);
        }
    }

    for (setIdx = 0; setIdx < __VK_MAX_DESCRIPTOR_SETS; setIdx++)
    {
        if (chipPipeline->patchKeys[setIdx])
        {
            uint32_t keyIndex = 0;
            uint32_t bindingIdx;
            PROG_VK_RESOURCE_SET *resSet;
            __vkDescriptorSetLayout *descSetLayout  = pip->pipelineLayout->descSetLayout[setIdx];
            __VK_ASSERT((descSetLayout->samplerDescriptorCount + descSetLayout->samplerBufferDescriptorCount
                        + descSetLayout->inputAttachmentDescriptorCount + descSetLayout->storageDescriptorCount)
                        == chipPipeline->patchKeyCount[setIdx]);

            if (chipPipeline->masterInstance->pep.u.vk.pResourceSets == VK_NULL_HANDLE)
            {
                continue;
            }
            resSet = &chipPipeline->masterInstance->pep.u.vk.pResourceSets[setIdx];
            for (bindingIdx = 0; bindingIdx < descSetLayout->bindingCount; bindingIdx++)
            {
                __vkDescriptorSetLayoutBinding  *binding = &descSetLayout->binding[bindingIdx];
                uint32_t arrayIdx;

                switch (binding->std.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    {
                        uint32_t entryIdx;
                        PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY *tableEntry = VK_NULL_HANDLE;
                        for (entryIdx = 0; entryIdx < resSet->separatedSamplerTable.countOfEntries; entryIdx++)
                        {
                            tableEntry = &resSet->separatedSamplerTable.pSamplerEntries[entryIdx];
                            if ((tableEntry->samplerBinding.set == setIdx) &&
                                (tableEntry->samplerBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }
                        __VK_ASSERT(entryIdx < resSet->separatedSamplerTable.countOfEntries);

                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &tableEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT | HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_GATHER)
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_GATHER_PCF | VSC_RES_OP_BIT_TEXLD_BIAS_PCF | VSC_RES_OP_BIT_TEXLD_LOD_PCF))
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_PCF_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD
                                    | VSC_RES_OP_BIT_TEXLD_BIAS
                                    | VSC_RES_OP_BIT_TEXLD_LOD
                                    | VSC_RES_OP_BIT_TEXLDP
                                    | VSC_RES_OP_BIT_TEXLDP_BIAS
                                    | VSC_RES_OP_BIT_TEXLDP_LOD))
                                {
                                    patchKey |= HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                }
                            }
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;

                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    {
                        uint32_t entryIdx, i;
                        uint32_t index = 0, imageIdx;
                        PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY *tableEntry = VK_NULL_HANDLE;
                        PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY *storageEntry = VK_NULL_HANDLE;
                        if (!resSet->combinedSampTexTable.countOfEntries)
                        {
                            chipPipeline->patchKeys[setIdx][keyIndex++] = 0;
                            continue;
                        }

                        for (entryIdx = 0; entryIdx < resSet->combinedSampTexTable.countOfEntries; entryIdx++)
                        {
                            tableEntry = &resSet->combinedSampTexTable.pCombTsEntries[entryIdx];
                            if ((tableEntry->combTsBinding.set == setIdx) &&
                                (tableEntry->combTsBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }

                        for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
                        {
                            if (tableEntry->activeStageMask & (1 << i))
                            {
                                index = i;
                                break;
                            }
                        }

                        imageIdx = tableEntry->sampledImageIndexInStorageTable[index];
                        storageEntry = &resSet->separatedTexTable.pTextureEntries[imageIdx];

                        __VK_ASSERT(entryIdx < resSet->combinedSampTexTable.countOfEntries);

                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &tableEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT | HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_GATHER)
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_GATHER_PCF | VSC_RES_OP_BIT_TEXLD_BIAS_PCF | VSC_RES_OP_BIT_TEXLD_LOD_PCF))
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_PCF_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD
                                    | VSC_RES_OP_BIT_TEXLD_BIAS
                                    | VSC_RES_OP_BIT_TEXLD_LOD
                                    | VSC_RES_OP_BIT_TEXLDP
                                    | VSC_RES_OP_BIT_TEXLDP_BIAS
                                    | VSC_RES_OP_BIT_TEXLDP_LOD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT | HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                    chipPipeline->patchResOpBit[setIdx][keyIndex] = *pResOp;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_IMAGE_OP)
                                {
                                    __VK_ASSERT((storageEntry->texBinding.set == setIdx) &&
                                    (storageEntry->texBinding.binding == binding->std.binding));
                                    patchKey |= HALTI5_PATCH_COMBINED_IMAGE_FORAMT_BIT;
                                    chipPipeline->patchCombinedImgFormat[setIdx][keyIndex] = halti5_pip_getImageFormat(&storageEntry->hwMappings[imageIdx].s.imageDerivedInfo, VSC_MAX_SHADER_STAGE_COUNT);
                                }
                            }
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    {
                        uint32_t entryIdx;
                        PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY *tableEntry = VK_NULL_HANDLE;
                        for (entryIdx = 0; entryIdx < resSet->separatedTexTable.countOfEntries; entryIdx++)
                        {
                            tableEntry = &resSet->separatedTexTable.pTextureEntries[entryIdx];
                            if ((tableEntry->texBinding.set == setIdx) &&
                                (tableEntry->texBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }
                        __VK_ASSERT(entryIdx < resSet->separatedTexTable.countOfEntries);

                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &tableEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;
                            uint32_t stageIndex = 0;
                            uint32_t i;

                            for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
                            {
                                if (tableEntry->activeStageMask & (1 << i))
                                {
                                    stageIndex = i;
                                    break;
                                }
                            }

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_GATHER)
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_GATHER_PCF | VSC_RES_OP_BIT_TEXLD_BIAS_PCF | VSC_RES_OP_BIT_TEXLD_LOD_PCF))
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_PCF_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD
                                    | VSC_RES_OP_BIT_TEXLD_BIAS
                                    | VSC_RES_OP_BIT_TEXLD_LOD
                                    | VSC_RES_OP_BIT_TEXLDP
                                    | VSC_RES_OP_BIT_TEXLDP_BIAS
                                    | VSC_RES_OP_BIT_TEXLDP_LOD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT | HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_IMAGE_OP)
                                {
                                    patchKey |= HALTI5_PATCH_SAMPLED_IMAGRE_FORMAT_BIT;
                                    chipPipeline->patchSampledImgFormat[setIdx][keyIndex] =
                                        halti5_pip_getImageFormat(&tableEntry->hwMappings[stageIndex].s.imageDerivedInfo, VSC_MAX_SHADER_STAGE_COUNT);
                                }
                            }
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    {
                        uint32_t entryIdx;
                        PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY *inputAttachmentEntry = VK_NULL_HANDLE;
                        for (entryIdx = 0; entryIdx < resSet->inputAttachmentTable.countOfEntries; entryIdx++)
                        {
                            inputAttachmentEntry = &resSet->inputAttachmentTable.pIaEntries[entryIdx];
                            if ((inputAttachmentEntry->iaBinding.set == setIdx) &&
                                (inputAttachmentEntry->iaBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }
                        __VK_ASSERT(entryIdx < resSet->inputAttachmentTable.countOfEntries);
                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &inputAttachmentEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD_GRAD | VSC_RES_OP_BIT_TEXLDP_GRAD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_GATHER)
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_GATHER_PCF | VSC_RES_OP_BIT_TEXLD_BIAS_PCF | VSC_RES_OP_BIT_TEXLD_LOD_PCF))
                                {
                                    patchKey |= HALTI5_PATCH_TX_GATHER_PCF_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_TEXLD
                                    | VSC_RES_OP_BIT_TEXLD_BIAS
                                    | VSC_RES_OP_BIT_TEXLD_LOD
                                    | VSC_RES_OP_BIT_TEXLDP
                                    | VSC_RES_OP_BIT_TEXLDP_BIAS
                                    | VSC_RES_OP_BIT_TEXLDP_LOD))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT | HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT;
                                }
                                if (*pResOp & (VSC_RES_OP_BIT_FETCH | VSC_RES_OP_BIT_FETCH_MS))
                                {
                                    patchKey |= HALTI5_PATCH_TX_EXTRA_INPUT_BIT;
                                }
                            }
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                    {
                        uint32_t entryIdx;
                        PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY *tableEntry = VK_NULL_HANDLE;
                        for (entryIdx = 0; entryIdx < resSet->uniformTexBufTable.countOfEntries; entryIdx++)
                        {
                            tableEntry = &resSet->uniformTexBufTable.pUtbEntries[entryIdx];
                            if ((tableEntry->utbBinding.set == setIdx) &&
                                (tableEntry->utbBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }

                        if (entryIdx == resSet->uniformTexBufTable.countOfEntries)
                            break;

                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &tableEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & VSC_RES_OP_BIT_FETCH)
                                {
                                    patchKey |= HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD_BIT;
                                }
                                if (*pResOp & VSC_RES_OP_BIT_LOAD_STORE)
                                {
                                    patchKey |= HALTI5_PATCH_FORMAT_TO_COMPILER_BIT;
                                }
                            }
                            chipPipeline->patchTexBufFormat[setIdx][keyIndex] = halti5_pip_getImageFormat(tableEntry->imageDerivedInfo, VSC_MAX_SHADER_STAGE_COUNT);
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    {
                        uint32_t entryIdx;
                        PROG_VK_STORAGE_TABLE_ENTRY *tableEntry = VK_NULL_HANDLE;

                        for (entryIdx = 0; entryIdx < resSet->storageTable.countOfEntries; entryIdx++)
                        {
                            tableEntry = &resSet->storageTable.pStorageEntries[entryIdx];
                            if ((tableEntry->storageBinding.set == setIdx) &&
                                (tableEntry->storageBinding.binding == binding->std.binding))
                            {
                                break;
                            }
                        }

                        if (entryIdx == resSet->storageTable.countOfEntries)
                            break;

                        for (arrayIdx = 0; arrayIdx < binding->std.descriptorCount; arrayIdx++)
                        {
                            VSC_RES_OP_BIT *pResOp = &tableEntry->pResOpBits[arrayIdx];
                            halti5_patch_key patchKey = 0;

                            if (pResOp != gcvNULL)
                            {
                                if (*pResOp & VSC_RES_OP_BIT_IMAGE_OP)
                                {
                                    patchKey |=  HALTI5_PATCH_SORAGE_IMAGE_FORMAT_BIT;
                                }
                            }

                            chipPipeline->patchStorageImgFormat[setIdx][keyIndex] = halti5_pip_getImageFormat(tableEntry->imageDerivedInfo, VSC_MAX_SHADER_STAGE_COUNT);
                            chipPipeline->patchKeys[setIdx][keyIndex++] = patchKey;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            __VK_ASSERT(keyIndex == chipPipeline->patchKeyCount[setIdx]);
        }
    }
}

static VkResult halti5_pip_process_priv_const(
    halti5_vscprogram_instance *masterInstance,
    void *chipPipeline,
    gctBOOL isGraphicsPipeline,
    const void *info
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t i;

    if (isGraphicsPipeline)
    {
        halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)chipPipeline;
        const VkGraphicsPipelineCreateInfo *gfxInfo = (VkGraphicsPipelineCreateInfo *) info;

        /* VS private constant mapping. */
        if (masterInstance->pep.seps[VSC_SHADER_STAGE_VS].staticPrivMapping.privConstantMapping.countOfEntries)
        {
            SHADER_PRIV_CONSTANT_MAPPING *privConstMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_VS].staticPrivMapping.privConstantMapping;
            for (i = 0; i < privConstMapping->countOfEntries; i++)
            {
                SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[i];

                if (privConstEntry->mode == SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO)
                {
                    continue;
                }

                switch (privConstEntry->commonPrivm.privmKind)
                {
                case SHS_PRIV_CONSTANT_KIND_BASE_INSTANCE:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipGfxPipeline->baseInstance.bUsed = VK_TRUE;
                    chipGfxPipeline->baseInstance.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipGfxPipeline->baseInstance.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_VERTEX] >> 2) + (chipGfxPipeline->baseInstance.hwRegNo * 4);
                    chipGfxPipeline->baseInstance.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;
                    /*HW not supoort the feature: multiview, so compiler treat the gl_ViewIndex as a private constant*/
                case SHS_PRIV_CONSTANT_KIND_VIEW_INDEX:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipGfxPipeline->useViewIndex.bUsed = VK_TRUE;
                    chipGfxPipeline->useViewIndex.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipGfxPipeline->useViewIndex.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_VERTEX] >> 2) + (chipGfxPipeline->useViewIndex.hwRegNo * 4);
                    chipGfxPipeline->useViewIndex.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;

                default:
                    break;
                }
            }
        }

        /* PS private constant mapping. */
        if (masterInstance->pep.seps[VSC_SHADER_STAGE_PS].staticPrivMapping.privConstantMapping.countOfEntries)
        {
            SHADER_PRIV_CONSTANT_MAPPING *privConstMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_PS].staticPrivMapping.privConstantMapping;
            for (i = 0; i < privConstMapping->countOfEntries; i++)
            {
                SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[i];

                if (privConstEntry->mode == SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO)
                {
                    continue;
                }

                switch (privConstEntry->commonPrivm.privmKind)
                {
                case SHS_PRIV_CONSTANT_KIND_SAMPLE_LOCATION:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipGfxPipeline->sampleLocation.bUsed = VK_TRUE;
                    chipGfxPipeline->sampleLocation.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipGfxPipeline->sampleLocation.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) +
                        (chipGfxPipeline->sampleLocation.hwRegNo * 4) +
                        privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                    chipGfxPipeline->sampleLocation.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;

                case SHS_PRIV_CONSTANT_KIND_ENABLE_MULTISAMPLE_BUFFERS:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipGfxPipeline->ehableMultiSampleBuffers.bUsed = VK_TRUE;
                    chipGfxPipeline->ehableMultiSampleBuffers.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipGfxPipeline->ehableMultiSampleBuffers.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) +
                        (chipGfxPipeline->ehableMultiSampleBuffers.hwRegNo * 4) +
                        privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                    chipGfxPipeline->ehableMultiSampleBuffers.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;
                    /*HW not support the feature: multiview, so compiler treat the gl_ViewIndex as a private constant*/
                case SHS_PRIV_CONSTANT_KIND_VIEW_INDEX:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipGfxPipeline->useViewIndex.bUsed = VK_TRUE;
                    chipGfxPipeline->useViewIndex.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipGfxPipeline->useViewIndex.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) + (chipGfxPipeline->useViewIndex.hwRegNo * 4);
                    chipGfxPipeline->useViewIndex.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;

                default:
                    break;
                }
            }

            if (chipGfxPipeline->defaultBuffer.bUsed)
            {
                halti5_module *chipModule = (halti5_module *)masterInstance->devCtx->chipPriv;
                SHADER_DEFAULT_UBO_MAPPING *defaultUboMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping;
                uint32_t index = defaultUboMapping->baseAddressIndexInPrivConstTable;
                uint32_t entryCount = defaultUboMapping->countOfEntries;
                SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[index];
                __vkBuffer *defaultBuf =  __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipGfxPipeline->defaultUbo);

                __VK_ASSERT(privConstEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_DEFAULT_UBO_ADDRESS);
                chipGfxPipeline->defaultBuffer.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                chipGfxPipeline->defaultBuffer.hwRegAddress =
                    (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) +
                    (chipGfxPipeline->defaultBuffer.hwRegNo * 4) +
                    privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                chipGfxPipeline->defaultBuffer.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;

                for (i = 0; i < entryCount; i++)
                {
                    SHADER_DEFAULT_UBO_MEMBER_ENTRY *uboMemberEntry = &defaultUboMapping->pDefaultUboMemberEntries[i];
                    uint32_t offset = uboMemberEntry->offsetInByte;
                    VkBool32 enableMultisample = (gfxInfo->pMultisampleState->rasterizationSamples > VK_SAMPLE_COUNT_1_BIT) ? 1 : 0;

                    if (uboMemberEntry->memberKind == SHS_DEFAULT_UBO_MEMBER_PRIV_CONST)
                    {
                        SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[uboMemberEntry->memberIndexInOtherEntryTable];
                        __VK_ASSERT(privConstEntry->mode == SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO);

                        switch (privConstEntry->commonPrivm.privmKind)
                        {
                        case SHS_PRIV_CONSTANT_KIND_SAMPLE_LOCATION:
                            gcoOS_MemCopy((gctPOINTER)((uint8_t *)defaultBuf->memory->hostAddr + offset), chipModule->sampleLocations, 16 * sizeof(gctFLOAT));
                            break;
                        case SHS_PRIV_CONSTANT_KIND_ENABLE_MULTISAMPLE_BUFFERS:
                            gcoOS_MemCopy((gctPOINTER)((uint8_t *)defaultBuf->memory->hostAddr + offset), &enableMultisample, sizeof(gctUINT32));
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        halti5_computePipeline *chipCmptPipeline = (halti5_computePipeline *)chipPipeline;

        if (masterInstance->pep.seps[VSC_SHADER_STAGE_CS].staticPrivMapping.privConstantMapping.countOfEntries)
        {
            SHADER_PRIV_CONSTANT_MAPPING *privConstMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_CS].staticPrivMapping.privConstantMapping;
            for (i = 0; i < privConstMapping->countOfEntries; i++)
            {
                SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[i];

                if (privConstEntry->mode == SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO)
                {
                    continue;
                }

                switch (privConstEntry->commonPrivm.privmKind)
                {
                case SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM:
                    __VK_ASSERT(privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel == 0);
                    chipCmptPipeline->numberOfWorkGroup.bUsed = VK_TRUE;
                    chipCmptPipeline->numberOfWorkGroup.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                    chipCmptPipeline->numberOfWorkGroup.hwRegAddress =
                        (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) + (chipCmptPipeline->numberOfWorkGroup.hwRegNo * 4);
                    chipCmptPipeline->numberOfWorkGroup.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    break;
                default:
                    break;
                }
            }
            if (chipCmptPipeline->defaultBuffer.bUsed)
            {
                SHADER_DEFAULT_UBO_MAPPING *defaultUboMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping;
                uint32_t index = defaultUboMapping->baseAddressIndexInPrivConstTable;
                uint32_t entryCount = defaultUboMapping->countOfEntries;
                SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[index];

                __VK_ASSERT(privConstEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_DEFAULT_UBO_ADDRESS);
                chipCmptPipeline->defaultBuffer.hwRegNo = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
                chipCmptPipeline->defaultBuffer.hwRegAddress =
                    (masterInstance->hwStates.hints.hwConstRegBases[gcvPROGRAM_STAGE_FRAGMENT] >> 2) +
                    (chipCmptPipeline->defaultBuffer.hwRegNo * 4) +
                    privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                chipCmptPipeline->defaultBuffer.hwRegCount = privConstEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;

                for (i = 0; i < entryCount; i++)
                {
                    SHADER_DEFAULT_UBO_MEMBER_ENTRY *uboMemberEntry = &defaultUboMapping->pDefaultUboMemberEntries[i];
                    uint32_t offset = uboMemberEntry->offsetInByte;

                    if (uboMemberEntry->memberKind == SHS_DEFAULT_UBO_MEMBER_PRIV_CONST)
                    {
                        SHADER_PRIV_CONSTANT_ENTRY *privConstEntry = &privConstMapping->pPrivmConstantEntries[uboMemberEntry->memberIndexInOtherEntryTable];
                        __VK_ASSERT(privConstEntry->mode == SHADER_PRIV_CONSTANT_MODE_VAL_2_DUBO);

                        switch (privConstEntry->commonPrivm.privmKind)
                        {
                        case SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM:
                            chipCmptPipeline->offset = offset;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    return result;
}

static VkResult halti5_pip_process_defaultUbo(
    halti5_vscprogram_instance *masterInstance,
    void *chipPipeline,
    gctBOOL isGraphicsPipeline
    )
{
    VkResult result = VK_SUCCESS;
    VkMemoryAllocateInfo mem_alloc;
    VkDeviceMemory bufferMemory;
    __vkDevContext *devCtx = masterInstance->devCtx;

    halti5_computePipeline *chipCmptPipeline = VK_NULL_HANDLE;
    halti5_graphicsPipeline *chipGfxPipeline = VK_NULL_HANDLE;

    if (isGraphicsPipeline)
    {
        chipGfxPipeline = (halti5_graphicsPipeline *)chipPipeline;
        if (masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping.sizeInByte)
        {
            VkBufferCreateInfo buf_info;
            __vkBuffer *buf = VK_NULL_HANDLE;
            uint32_t bufferSize = masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping.sizeInByte;

            /* Create a VkBuffer for default UBO */
            __VK_MEMZERO(&buf_info, sizeof(VkBufferCreateInfo));
            buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buf_info.size = bufferSize;

            chipGfxPipeline->defaultUbo = VK_NULL_HANDLE;
            __VK_ONERROR(__vk_CreateBuffer((VkDevice)devCtx, &buf_info, gcvNULL, &chipGfxPipeline->defaultUbo));

            /* Allocate device memory for default UBO */
            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipGfxPipeline->defaultUbo);
            __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.allocationSize = buf->memReq.size;
            mem_alloc.memoryTypeIndex = 0;
            __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &mem_alloc, gcvNULL, &bufferMemory));

            /* bind memory */
            __VK_ONERROR(__vk_BindBufferMemory((VkDevice)devCtx, chipGfxPipeline->defaultUbo, bufferMemory, 0));

            chipGfxPipeline->defaultBuffer.bUsed = VK_TRUE;
        }
    }
    else
    {
        chipCmptPipeline = (halti5_computePipeline *)chipPipeline;
        if (masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping.sizeInByte)
        {
            VkBufferCreateInfo buf_info;
            __vkBuffer *buf = VK_NULL_HANDLE;
            uint32_t bufferSize = masterInstance->pep.seps[VSC_SHADER_STAGE_PS].defaultUboMapping.sizeInByte;

            /* Create a VkBuffer for default UBO */
            __VK_MEMZERO(&buf_info, sizeof(VkBufferCreateInfo));
            buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buf_info.size = bufferSize;

            chipCmptPipeline->defaultUbo = VK_NULL_HANDLE;
            __VK_ONERROR(__vk_CreateBuffer((VkDevice)devCtx, &buf_info, gcvNULL, &chipCmptPipeline->defaultUbo));

            /* Allocate device memory for default UBO */
            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipCmptPipeline->defaultUbo);
            __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.allocationSize = buf->memReq.size;
            mem_alloc.memoryTypeIndex = 0;
            __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &mem_alloc, gcvNULL, &bufferMemory));

            /* bind memory */
            __VK_ONERROR(__vk_BindBufferMemory((VkDevice)devCtx, chipCmptPipeline->defaultUbo, bufferMemory, 0));

            chipCmptPipeline->defaultBuffer.bUsed = VK_TRUE;
        }
    }
    return result;

OnError:
    if (isGraphicsPipeline)
    {
        if (chipGfxPipeline->defaultUbo)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipGfxPipeline->defaultUbo);

            if (buf->memory)
            {
                __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);
            }

            __vk_DestroyBuffer((VkDevice)(uintptr_t)devCtx, chipGfxPipeline->defaultUbo, VK_NULL_HANDLE);
            chipGfxPipeline->defaultUbo = VK_NULL_HANDLE;
        }
    }
    else
    {
        if (chipCmptPipeline->defaultUbo)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipCmptPipeline->defaultUbo);

            if (buf->memory)
            {
                __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);
            }

            __vk_DestroyBuffer((VkDevice)(uintptr_t)devCtx, chipCmptPipeline->defaultUbo, VK_NULL_HANDLE);
            chipCmptPipeline->defaultUbo = VK_NULL_HANDLE;
        }
    }

    return result;
}


static VkResult halti5_pip_build_gfxshaders(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo *info
    )
{
    VSC_SHADER_COMPILER_PARAM vscCompileParams;
    VSC_PROGRAM_LINKER_PARAM vscLinkParams;
    VSC_PROG_LIB_LINK_TABLE vscLibLinkTable;
    uint32_t shaderType = VSC_SHADER_STAGE_UNKNOWN;
    __vkShaderModule *shaderModule;
    uint32_t i;
    VkResult result = VK_SUCCESS;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    halti5_vscprogram_instance *masterInstance = VK_NULL_HANDLE;
    VSC_PROG_LIB_LINK_ENTRY *vsclinkEntries = VK_NULL_HANDLE;
    uint32_t linkEntryNum = chipGfxPipeline->patchOutput.count;
    SpvDecodeInfo *decodeInfo = VK_NULL_HANDLE;
    void *stateKey[__VK_MAX_DESCRIPTOR_SETS];
    uint32_t stateKeySizeInBytes[__VK_MAX_DESCRIPTOR_SETS];
    vkShader_HANDLE hShaderArray[VSC_MAX_SHADER_STAGE_COUNT];
    vkShader_HANDLE hShaderArrayCopy[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_RESOURCE_LAYOUT vscShaderResLayout;
    struct _gcsHINT *hints = VK_NULL_HANDLE;
    VkBool32 bTriangle = (pip->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
                          pip->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
                          pip->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN ||
                          pip->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
                          pip->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY) ?
                         VK_TRUE : VK_FALSE;

    __VK_SET_ALLOCATIONCB(&pip->memCb);

    __VK_MEMZERO(hShaderArray, sizeof(hShaderArray));
    __VK_MEMZERO(hShaderArrayCopy, sizeof(hShaderArrayCopy));
    __VK_MEMZERO(&vscShaderResLayout, sizeof(vscShaderResLayout));

    if (pip->pipelineLayout)
    {
        uint32_t descSetLayoutCount = pip->pipelineLayout->descSetLayoutCount;
        uint32_t keyCount = 0;
        __VK_ASSERT(descSetLayoutCount <= __VK_MAX_DESCRIPTOR_SETS);
        for (i = 0; i < descSetLayoutCount; i++)
        {
            __vkDescriptorSetLayout *descSetLayout = pip->pipelineLayout->descSetLayout[i];
            chipPipeline->patchKeyCount[i] = descSetLayout->samplerDescriptorCount
                                           + descSetLayout->samplerBufferDescriptorCount
                                           + descSetLayout->inputAttachmentDescriptorCount
                                           + descSetLayout->storageDescriptorCount;
            if (chipPipeline->patchKeyCount[i])
            {
                chipPipeline->patchKeys[i] =
                    (halti5_patch_key *)__VK_ALLOC(sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                chipPipeline->patchResOpBit[i] =
                    (VSC_RES_OP_BIT *)__VK_ALLOC(sizeof(VSC_RES_OP_BIT) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                chipPipeline->patchTexBufFormat[i] =
                    (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                if (descSetLayout->storageDescriptorCount)
                {
                    chipPipeline->patchStorageImgFormat[i] =
                        (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    __VK_ONERROR(chipPipeline->patchStorageImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchStorageImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                }

                if (descSetLayout->samplerDescriptorCount)
                {
                    chipPipeline->patchSampledImgFormat[i] =
                        (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    chipPipeline->patchCombinedImgFormat[i] =
                    (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

                    __VK_ONERROR(chipPipeline->patchSampledImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchSampledImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                    __VK_ONERROR(chipPipeline->patchCombinedImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchCombinedImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                }

                __VK_ONERROR(chipPipeline->patchKeys[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_ONERROR(chipPipeline->patchTexBufFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_ONERROR(chipPipeline->patchResOpBit[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_MEMZERO(chipPipeline->patchKeys[i], sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i]);
                __VK_MEMZERO(chipPipeline->patchTexBufFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                __VK_MEMZERO(chipPipeline->patchResOpBit[i], sizeof(VSC_RES_OP_BIT) * chipPipeline->patchKeyCount[i]);

                stateKey[keyCount] = (void *)chipPipeline->patchKeys[i];
                stateKeySizeInBytes[keyCount] = sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i];
                keyCount++;
            }
        }
        chipPipeline->keyCount = keyCount;
    }

    chipPipeline->vanilla = (chipPipeline->keyCount == 0);

    masterInstance =(halti5_vscprogram_instance *)__VK_ALLOC(sizeof(halti5_vscprogram_instance), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(masterInstance ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(masterInstance, sizeof(halti5_vscprogram_instance));
    masterInstance->devCtx = devCtx;

    __VK_ONERROR(halti5_helper_createVscResLayout(pip));

    __VK_MEMZERO(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
    vscCompileParams.cfg.ctx.appNameId = devCtx->pPhyDevice->pInst->patchID;
    vscCompileParams.cfg.ctx.isPatchLib = gcvFALSE;
    vscCompileParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
    vscCompileParams.cfg.optFlags = (pip->flags & VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT)
                                  ? 0
                                  : VSC_COMPILER_OPT_FULL;

    decodeInfo =(SpvDecodeInfo *)__VK_ALLOC(sizeof(SpvDecodeInfo), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR((decodeInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

    __VK_MEMZERO(decodeInfo, sizeof(SpvDecodeInfo));

    for (i = 0; i < info->stageCount; i++)
    {
        uint8_t md5digest[16];
        switch (info->pStages[i].stage)
        {
        case VK_SHADER_STAGE_VERTEX_BIT:
            shaderType = VSC_SHADER_STAGE_VS;
            break;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            shaderType = VSC_SHADER_STAGE_HS;
            break;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            shaderType = VSC_SHADER_STAGE_DS;
            break;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            shaderType = VSC_SHADER_STAGE_GS;
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            shaderType = VSC_SHADER_STAGE_PS;
            break;
        default:
            __VK_ASSERT(!"Unknown shader type");
            __VK_ONERROR(VK_ERROR_INCOMPATIBLE_DRIVER);
            break;
        }

        shaderModule = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkShaderModule*, info->pStages[i].module);

        if (pip->cache)
        {
            gcsHASH_MD5CTX md5Ctx;
            __vkUtilsHashObject *hashObj = gcvNULL;
            const char *pName = info->pStages[i].pName;
            const VkSpecializationInfo *pSpecialInfo = info->pStages[i].pSpecializationInfo;

            gcsHASH_MD5Init(&md5Ctx);
            gcsHASH_MD5Update(&md5Ctx, &info->pStages[i].stage, sizeof(info->pStages[i].stage));
            gcsHASH_MD5Update(&md5Ctx, shaderModule->pCode, shaderModule->codeSize);
            if (pName)
            {
                gcsHASH_MD5Update(&md5Ctx, pName, gcoOS_StrLen(pName, gcvNULL));
            }
            if (pSpecialInfo && pSpecialInfo->pData)
            {
                gcsHASH_MD5Update(&md5Ctx, pSpecialInfo->pData, pSpecialInfo->dataSize);
            }
            gcsHASH_MD5Final(&md5Ctx, md5digest);

            gcoOS_AcquireMutex(gcvNULL, pip->cache->mutex, gcvINFINITE);
            hashObj = __vk_utils_hashFindObjByKey(pip->cache->moduleHash, md5digest);
            if (hashObj)
            {
                __vkModuleCacheEntry *pEntry = (__vkModuleCacheEntry*)hashObj->pUserData;
                hShaderArray[shaderType] = pEntry->handle;
                halti5_ReferenceVkShader(pEntry->handle);
            }
        }

        if (!hShaderArray[shaderType])
        {
            SHADER_HANDLE   vscHandle = VK_NULL_HANDLE;
            decodeInfo->binary = shaderModule->pCode;
            decodeInfo->sizeInByte = (gctUINT)shaderModule->codeSize;
            decodeInfo->stageInfo = (gctPOINTER)&(info->pStages[i]);
            decodeInfo->specFlag = SPV_SPECFLAG_NONE;
            decodeInfo->localSize[0] = decodeInfo->localSize[1] = decodeInfo->localSize[2] = 0;
            decodeInfo->tcsInputVertices = info->pTessellationState ? info->pTessellationState->patchControlPoints : 0;
            decodeInfo->funcCtx = shaderModule->funcTable;
            decodeInfo->subPass = pip->subPass;

            if (shaderType == VSC_SHADER_STAGE_HS || shaderType == VSC_SHADER_STAGE_DS)
            {
                VkPipelineTessellationDomainOriginStateCreateInfo *domain = (VkPipelineTessellationDomainOriginStateCreateInfo*)get_object_by_type(
                                                                                 (VkBaseInStructure*)info->pTessellationState,
                                                                                 VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO
                                                                                 );
                if (domain != gcvNULL && domain->domainOrigin == VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT)
                {
                    decodeInfo->specFlag = SPV_SPECFLAG_LOWER_LEFT_DOMAIN;
                }
            }

            if (shaderType == VSC_SHADER_STAGE_PS)
            {
                uint32_t j = 0;
                __vkRenderPass *rdp = pip->renderPass;
                decodeInfo->renderpassInfo = (SpvRenderPassInfo *)__VK_ALLOC(sizeof(SpvRenderPassInfo), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                __VK_ONERROR((decodeInfo->renderpassInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));
                __VK_MEMZERO(decodeInfo->renderpassInfo, sizeof(SpvRenderPassInfo));
                decodeInfo->renderpassInfo->attachmentCount = rdp->attachmentCount;

                if (rdp->attachmentCount > 0)
                {
                    decodeInfo->renderpassInfo->attachments =
                        (SpvAttachmentDesc *)__VK_ALLOC(sizeof(SpvAttachmentDesc) * rdp->attachmentCount, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    __VK_ONERROR((decodeInfo->renderpassInfo->attachments ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));
                    __VK_MEMZERO(decodeInfo->renderpassInfo->attachments, sizeof(SpvAttachmentDesc) * rdp->attachmentCount);
                }

                for (j = 0; j < rdp->attachmentCount; j++)
                {
                    decodeInfo->renderpassInfo->attachments[j].format = rdp->attachments[j].format;
                    if (rdp->attachments[j].sampleCount > VK_SAMPLE_COUNT_1_BIT)
                    {
                        decodeInfo->renderpassInfo->attachments[j].attachmentFlag =
                            (Spv_AttachmentFlag)(SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER | SPV_ATTACHMENTFLAG_MULTI_SAMPLE);
                    }
                    else
                    {
                        switch (rdp->attachments[j].format)
                        {
                        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
                        case VK_FORMAT_R8G8B8A8_SRGB:
                        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                        case VK_FORMAT_B8G8R8A8_SRGB:
                        case VK_FORMAT_D16_UNORM:
                        case VK_FORMAT_X8_D24_UNORM_PACK32:
                        case VK_FORMAT_D24_UNORM_S8_UINT:
                        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                            decodeInfo->renderpassInfo->attachments[j].attachmentFlag = SPV_ATTACHMENTFLAG_TREAT_AS_SAMPLER;
                            break;
                        default:
                            decodeInfo->renderpassInfo->attachments[j].attachmentFlag = SPV_ATTACHMENTFLAG_NONE;
                            break;
                        }
                    }
                }
                decodeInfo->renderpassInfo->subPassInfoCount = rdp->subPassInfoCount;
                decodeInfo->renderpassInfo->subPassInfo =
                    (SpvRenderSubPassInfo *)__VK_ALLOC(sizeof(SpvRenderSubPassInfo) * rdp->subPassInfoCount, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                __VK_ONERROR((decodeInfo->renderpassInfo->subPassInfo ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));
                __VK_MEMZERO(decodeInfo->renderpassInfo->subPassInfo, sizeof(SpvRenderSubPassInfo) * rdp->subPassInfoCount);

                for (j = 0; j < rdp->subPassInfoCount; j++)
                {
                    __VK_MEMCOPY(&decodeInfo->renderpassInfo->subPassInfo[j].input_attachment_index, &rdp->subPassInfo[j].input_attachment_index,
                        __VK_MAX_RENDER_TARGETS * sizeof(uint32_t));
                }
            }

            __VK_ONERROR((gcvSTATUS_OK == gcSPV_Decode(decodeInfo, &vscHandle))
                                        ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
            hShaderArray[shaderType] = halti5_CreateVkShader(vscHandle);
            __VK_ASSERT(hShaderArray[shaderType]);

            /*save the decoded shader info before compile*/
            __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&chipPipeline->vkShaderDecoded[shaderType], hShaderArray[shaderType]))
                                    ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

            __VK_ONERROR(halti5_helper_createVscShaderResLayout(pip, chipPipeline->vscResLayout, shaderType, &vscShaderResLayout));
            vscCompileParams.hShader = vscHandle;
            vscCompileParams.pShResourceLayout = &vscShaderResLayout;
            __VK_ONERROR((gcvSTATUS_OK == vscCompileShader(&vscCompileParams, gcvNULL))
                                        ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
            halti5_helper_destroyVscShaderResLayout(pip, &vscShaderResLayout);

            if (pip->cache)
            {
                __vkModuleCacheEntry *pEntry = gcvNULL;
                __VK_SET_ALLOCATIONCB(&pip->cache->memCb);

                pEntry = (__vkModuleCacheEntry*)__VK_ALLOC(sizeof(__vkModuleCacheEntry), 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);

                pEntry->head.headBytes = sizeof(__vkModuleCacheHead);
                pEntry->head.magic = 0x12345678;
                pEntry->head.stage = shaderType;
                pEntry->head.patchCase = 0;
                vscQueryShaderBinarySize(hShaderArray[shaderType]->vscHandle, &pEntry->head.binBytes);
                pEntry->head.alignBytes = __VK_ALIGN(pEntry->head.binBytes, 8);
                __VK_MEMCOPY(pEntry->head.hashKey, md5digest, sizeof(md5digest));

                pEntry->handle = hShaderArray[shaderType];
                __vk_utils_hashAddObj(pMemCb, pip->cache->moduleHash, pEntry, md5digest, VK_FALSE);
                halti5_ReferenceVkShader(pEntry->handle);

                pip->cache->numModules++;
                pip->cache->totalBytes += (pEntry->head.headBytes + pEntry->head.alignBytes);
            }
        }

        if (pip->cache)
        {
            gcoOS_ReleaseMutex(gcvNULL, pip->cache->mutex);
        }

#if __VK_SAVE_THEN_LOAD_SHADER_BIN
        __VK_ONERROR(__vkSaveThenLoadShaderBin(pMemCb, &hShaderArray[shaderType]));
#endif

        __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&hShaderArrayCopy[shaderType], hShaderArray[shaderType]))
                                    ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
    }

    __VK_MEMZERO(&vscLinkParams, sizeof(vscLinkParams));
    for (i = 0; i < sizeof(hShaderArrayCopy) / sizeof(hShaderArrayCopy[0]); i++)
    {
        vscLinkParams.hShaderArray[i] = hShaderArrayCopy[i] ? hShaderArrayCopy[i]->vscHandle : VK_NULL_HANDLE;
    }
    vscLinkParams.pGlApiCfg = &devCtx->pPhyDevice->shaderCaps;
    vscLinkParams.pPgResourceLayout = chipPipeline->vscResLayout;
    __VK_MEMCOPY(&vscLinkParams.cfg, &vscCompileParams.cfg, sizeof(VSC_COMPILER_CONFIG));
    vscLinkParams.cfg.cFlags |= (VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS | VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

    if (devCtx->enabledFeatures.robustBufferAccess)
    {
        vscLinkParams.cfg.cFlags |= VSC_COMPILER_FLAG_NEED_OOB_CHECK;
    }

    linkEntryNum += (bTriangle && pip->frontFace != VK_FRONT_FACE_CLOCKWISE) ? 1 : 0;
    linkEntryNum += (!bTriangle) ? 1 : 0;

    if (linkEntryNum)
    {
        uint32_t linkEntryIdx = 0;
        uint32_t j = 0;
        uint32_t patchMask = chipGfxPipeline->patchOutput.mask;
        halti5_module *chipModule = (halti5_module*)devCtx->chipPriv;

        __VK_MEMZERO(&vscLibLinkTable, sizeof(vscLibLinkTable));
        vscLinkParams.pProgLibLinkTable = &vscLibLinkTable;

        vsclinkEntries = (VSC_PROG_LIB_LINK_ENTRY *)__VK_ALLOC(sizeof(VSC_PROG_LIB_LINK_ENTRY) * linkEntryNum,
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR(vsclinkEntries ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(vsclinkEntries, sizeof(VSC_PROG_LIB_LINK_ENTRY) * linkEntryNum);

        vscLibLinkTable.progLinkEntryCount = linkEntryNum;
        vscLibLinkTable.pProgLibLinkEntries = vsclinkEntries;

        if (patchMask)
        {
            while (patchMask)
            {
                if (patchMask & (1 << j))
                {
                    vsclinkEntries[linkEntryIdx].mainShaderStageBits = VSC_SHADER_STAGE_BIT_PS;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.applyLevel = VSC_SHLEVEL_Post_Medium;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.hShaderLib = chipModule->patchLib->vscHandle;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.pTempHashTable = gcvNULL;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPointCount = 1;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_COLOR_OUTPUT;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].u.clrOutput.location = chipGfxPipeline->patchOutput.outputs[j].locations[0];
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].u.clrOutput.layers = chipGfxPipeline->patchOutput.outputs[j].partCount;
                    vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].strFunc =
                        halti5_helper_patchFuc(chipGfxPipeline->patchOutput.outputs[j].format,
                                               HALTI5_PATCH_PE_EXTRA_OUTPUT, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
                    linkEntryIdx++;
                    patchMask &= ~(1 << j);
                }
                j++;
            }
        }

        if (bTriangle && pip->frontFace != VK_FRONT_FACE_CLOCKWISE)
        {
            vsclinkEntries[linkEntryIdx].mainShaderStageBits = VSC_SHADER_STAGE_BIT_PS;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPointCount = 1;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.applyLevel = VSC_SHLEVEL_Post_Medium;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.hShaderLib = chipModule->patchLib->vscHandle;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.pTempHashTable = gcvNULL;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_FRONTFACING_CCW;
            linkEntryIdx++;
        }

        if (!bTriangle)
        {
            vsclinkEntries[linkEntryIdx].mainShaderStageBits = VSC_SHADER_STAGE_BIT_PS;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPointCount = 1;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.applyLevel = VSC_SHLEVEL_Post_Medium;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.hShaderLib = chipModule->patchLib->vscHandle;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.pTempHashTable = gcvNULL;
            vsclinkEntries[linkEntryIdx].shLibLinkEntry.linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_FRONTFACING_ALWAY_FRONT;
            linkEntryIdx++;
        }

        __VK_ASSERT(linkEntryIdx == linkEntryNum);
    }

    __VK_ONERROR((gcvSTATUS_OK == vscLinkProgram(&vscLinkParams, &masterInstance->pep, &masterInstance->hwStates))
                                ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

    hints = &masterInstance->hwStates.hints;

    if (linkEntryNum)
    {
        uint32_t j = 0;
        uint32_t patchMask = chipGfxPipeline->patchOutput.mask;
        if (patchMask)
        {
            SHADER_PRIV_OUTPUT_MAPPING *privOutputMapping = &masterInstance->pep.seps[VSC_SHADER_STAGE_PS].dynamicPrivMapping.privOutputMapping;
            uint32_t privOutputMappingEntries = privOutputMapping->countOfEntries;
            __VK_ASSERT(privOutputMappingEntries > 0);

            while (patchMask)
            {
                if (patchMask & (1 << j))
                {
                    uint32_t k;
                    for (k = 0; k < privOutputMappingEntries; k++)
                    {
                        if ((privOutputMapping->pPrivOutputEntries[k].commonPrivm.privmKind & VSC_LIB_LINK_TYPE_COLOR_OUTPUT)
                            && (*(gctUINT*)privOutputMapping->pPrivOutputEntries[k].commonPrivm.pPrivateData == chipGfxPipeline->patchOutput.outputs[j].locations[0]))
                        {
                            chipGfxPipeline->patchOutput.outputs[j].locations[1] = privOutputMapping->pPrivOutputEntries[k].pOutput->ioIndex;
                        }
                    }
                    patchMask &= ~(1 << j);
                }
                j++;
            }
        }
    }

    if (masterInstance->pep.u.vk.privateCombTsHwMappingPool.countOfArray)
    {
        chipPipeline->countOfseparateBinding =
            masterInstance->pep.u.vk.privateCombTsHwMappingPool.countOfArray * VSC_MAX_SHADER_STAGE_COUNT;
        chipPipeline->separateBindingProgramed = (VkBool32 *)__VK_ALLOC(
            chipPipeline->countOfseparateBinding *sizeof(VkBool32),
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR(chipPipeline->separateBindingProgramed ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(chipPipeline->separateBindingProgramed, chipPipeline->countOfseparateBinding * sizeof(VkBool32));
    }

    /*should create ubo if needed*/
    halti5_pip_process_defaultUbo(masterInstance, chipGfxPipeline, VK_TRUE);

    /* process private constants. */
    halti5_pip_process_priv_const(masterInstance, chipGfxPipeline, VK_TRUE, info);

    chipPipeline->curInstance = chipPipeline->masterInstance = masterInstance;

    if (!chipPipeline->vanilla)
    {
        uint32_t masterKey = __vk_utils_evalCrc32_masked_array(stateKey, stateKey, stateKeySizeInBytes, chipPipeline->keyCount);

        chipPipeline->vscProgInstanceHash = __vk_utils_hashCreate(pMemCb, sizeof(uint32_t),
            VSC_PROG_INSTANCE_HASH_ENTRY_NUM, vSC_PROG_INSTANCE_HASH_ENTRY_SIZE, halti5_free_vscprogram_instance);

        __VK_ONERROR(chipPipeline->vscProgInstanceHash ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        masterInstance->ownerCacheObj = __vk_utils_hashAddObj(pMemCb,
            chipPipeline->vscProgInstanceHash,
            masterInstance, &masterKey, VK_TRUE);

        __VK_ONERROR(masterInstance->ownerCacheObj ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        __vk_utils_objAddRef(masterInstance->ownerCacheObj);

        /* Set accurate patchMask according to information from PEP */
        halti5_pip_build_patchKeyMask(pip);
    }

    if (chipPipeline->tweakHandler)
    {
        (*chipPipeline->tweakHandler->collect)(devCtx, pip, chipPipeline->tweakHandler);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        if (hShaderArrayCopy[i])
        {
            halti5_DestroyVkShader(hShaderArrayCopy[i]);
        }
    }

    __VK_MEMCOPY(chipPipeline->vkShaderArray, hShaderArray, sizeof(hShaderArray));
    chipPipeline->linkEntries = vsclinkEntries;
    chipPipeline->linkEntryCount  = linkEntryNum;

    for (i = 0; i < gcvPROGRAM_STAGE_LAST; i++)
    {
        if (hints && (hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][i] & gceMA_FLAG_WRITE))
        {
            chipPipeline->curInstance->memoryWrite = gcvTRUE;
            break;
        }
    }

    if (decodeInfo)
    {
        if (decodeInfo->renderpassInfo)
        {
            if (decodeInfo->renderpassInfo->attachments)
            {
                __VK_FREE(decodeInfo->renderpassInfo->attachments);
                decodeInfo->renderpassInfo->attachments = VK_NULL_HANDLE;
            }
            __VK_FREE(decodeInfo->renderpassInfo->subPassInfo);
            decodeInfo->renderpassInfo->subPassInfo = VK_NULL_HANDLE;
            __VK_FREE(decodeInfo->renderpassInfo);
            decodeInfo->renderpassInfo = VK_NULL_HANDLE;
        }

        __VK_FREE(decodeInfo);
        decodeInfo = VK_NULL_HANDLE;
    }

    return VK_SUCCESS;

OnError:
    if (vsclinkEntries)
    {
        __VK_FREE(vsclinkEntries);
    }
    if (masterInstance)
    {
        /* if hash is already established, destroy hash would free master instance */
        if (chipPipeline->vscProgInstanceHash && masterInstance->ownerCacheObj)
        {
            __vk_utils_objReleaseRef(masterInstance->ownerCacheObj);
        }
        else
        {
            halti5_free_vscprogram_instance(pMemCb, (void *)masterInstance);
        }
    }
    if (chipPipeline->vscProgInstanceHash)
    {
        __vk_utils_hashDestory(pMemCb, chipPipeline->vscProgInstanceHash);
    }

    halti5_helper_destroyVscResLayout(pip);

    if (chipPipeline->separateBindingProgramed)
    {
        __VK_FREE(chipPipeline->separateBindingProgramed);
    }

    for (i = 0; i < __VK_MAX_DESCRIPTOR_SETS; i++)
    {
        if (chipPipeline->patchKeys[i])
        {
            __VK_FREE(chipPipeline->patchKeys[i]);
        }
        if (chipPipeline->patchTexBufFormat[i])
        {
            __VK_FREE(chipPipeline->patchTexBufFormat[i]);
        }
        if (chipPipeline->patchStorageImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchStorageImgFormat[i]);
        }
        if (chipPipeline->patchSampledImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchSampledImgFormat[i]);
        }
        if (chipPipeline->patchCombinedImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchCombinedImgFormat[i]);
        }
        if (chipPipeline->patchResOpBit[i])
        {
            __VK_FREE(chipPipeline->patchResOpBit[i]);
        }
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        if (hShaderArrayCopy[i])
        {
            halti5_DestroyVkShader(hShaderArrayCopy[i]);
        }
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        if (hShaderArray[i])
        {
            halti5_DestroyVkShader(hShaderArray[i]);
        }
        chipPipeline->vkShaderArray[i] = VK_NULL_HANDLE;
    }

    halti5_helper_destroyVscShaderResLayout(pip, &vscShaderResLayout);

    if (decodeInfo)
    {
        if (decodeInfo->renderpassInfo)
        {
            if (decodeInfo->renderpassInfo->attachments)
            {
                __VK_FREE(decodeInfo->renderpassInfo->attachments);
                decodeInfo->renderpassInfo->attachments = VK_NULL_HANDLE;
            }
            __VK_FREE(decodeInfo->renderpassInfo->subPassInfo);
            decodeInfo->renderpassInfo->subPassInfo = VK_NULL_HANDLE;
            __VK_FREE(decodeInfo->renderpassInfo);
            decodeInfo->renderpassInfo = VK_NULL_HANDLE;
        }

        __VK_FREE(decodeInfo);
        decodeInfo = VK_NULL_HANDLE;
    }

    return result;
}


VkResult halti5_createGraphicsPipeline(
    VkDevice device,
    const VkGraphicsPipelineCreateInfo *info,
    VkPipeline pipeline
    )
{
    __vkPipeline *pip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    __vkDevContext *devCtx = (__vkDevContext *) device;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    VkResult result = VK_SUCCESS;
    halti5_graphicsPipeline *chipGfxPipeline;
    __VK_SET_ALLOCATIONCB(&pip->memCb);

    chipGfxPipeline = (halti5_graphicsPipeline *)__VK_ALLOC(
        sizeof(halti5_graphicsPipeline), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR((chipGfxPipeline ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

    __VK_MEMZERO(chipGfxPipeline, sizeof(halti5_graphicsPipeline));

    pip->chipPriv = chipGfxPipeline;

    __VK_ONERROR(halti5_pip_build_check(devCtx, pip));

    __VK_ONERROR(halti5_pip_tweak(devCtx, pip, (void *)info));

    __VK_ONERROR(halti5_pip_build_prepare(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_build_gfxshaders(devCtx, pip, info));

    __VK_ONERROR((*chipModule->minorTable.pip_emit_vsinput)(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_viewport(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_rs(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_msaa(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_rt(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_blend(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_graphicsProgram(devCtx, pip, info));

    __VK_ASSERT(chipGfxPipeline->chipPipeline.curCmdIndex <= __VK_PIPELINE_CMDBUFFER_MAXSIZE);

    return VK_SUCCESS;
OnError:
    if (chipGfxPipeline)
    {
        __VK_FREE(chipGfxPipeline);
    }
    return result;
}

static VkResult halti5_pip_build_computeshader(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkComputePipelineCreateInfo *info
    )
{
    VSC_SHADER_RESOURCE_LAYOUT vscShaderResLayout;
    VSC_SHADER_COMPILER_PARAM vscCompileParams;
    VSC_PROGRAM_LINKER_PARAM vscLinkParams;
    vkShader_HANDLE virShader = VK_NULL_HANDLE;
    vkShader_HANDLE virShaderCopy = VK_NULL_HANDLE;
    __vkShaderModule *shaderModule;
    VkResult result = VK_SUCCESS;
    halti5_computePipeline *chipCmptPipeline = (halti5_computePipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    halti5_vscprogram_instance *masterInstance = VK_NULL_HANDLE;
    void *stateKey[__VK_MAX_DESCRIPTOR_SETS];
    uint32_t stateKeySizeInBytes[__VK_MAX_DESCRIPTOR_SETS];
    uint8_t md5digest[16];
    uint32_t i;

    __VK_SET_ALLOCATIONCB(&pip->memCb);
    __VK_MEMZERO(&vscShaderResLayout, sizeof(vscShaderResLayout));

    if (pip->pipelineLayout)
    {
        uint32_t descSetLayoutCount = pip->pipelineLayout->descSetLayoutCount;
        uint32_t keyCount = 0;
        __VK_ASSERT(descSetLayoutCount <= __VK_MAX_DESCRIPTOR_SETS);
        for (i = 0; i < descSetLayoutCount; i++)
        {
            __vkDescriptorSetLayout *descSetLayout = pip->pipelineLayout->descSetLayout[i];
            chipPipeline->patchKeyCount[i] = descSetLayout->samplerDescriptorCount
                                           + descSetLayout->samplerBufferDescriptorCount
                                           + descSetLayout->inputAttachmentDescriptorCount
                                           + descSetLayout->storageDescriptorCount;
            if (chipPipeline->patchKeyCount[i])
            {
                chipPipeline->patchKeys[i] =
                    (halti5_patch_key *)__VK_ALLOC(sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                chipPipeline->patchResOpBit[i] =
                    (VSC_RES_OP_BIT *)__VK_ALLOC(sizeof(VSC_RES_OP_BIT) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                chipPipeline->patchTexBufFormat[i] =
                    (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

                if (descSetLayout->storageDescriptorCount)
                {
                    chipPipeline->patchStorageImgFormat[i] =
                        (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    __VK_ONERROR(chipPipeline->patchStorageImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchStorageImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                }

                if (descSetLayout->samplerDescriptorCount)
                {
                    chipPipeline->patchSampledImgFormat[i] =
                        (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

                    chipPipeline->patchCombinedImgFormat[i] =
                    (VSC_IMAGE_FORMAT *)__VK_ALLOC(sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i], 8,
                                                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

                    __VK_ONERROR(chipPipeline->patchSampledImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchSampledImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                    __VK_ONERROR(chipPipeline->patchCombinedImgFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    __VK_MEMZERO(chipPipeline->patchCombinedImgFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                }

                __VK_ONERROR(chipPipeline->patchKeys[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_ONERROR(chipPipeline->patchTexBufFormat[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_ONERROR(chipPipeline->patchResOpBit[i] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                __VK_MEMZERO(chipPipeline->patchKeys[i], sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i]);
                __VK_MEMZERO(chipPipeline->patchTexBufFormat[i], sizeof(VSC_IMAGE_FORMAT) * chipPipeline->patchKeyCount[i]);
                __VK_MEMZERO(chipPipeline->patchResOpBit[i], sizeof(VSC_RES_OP_BIT) * chipPipeline->patchKeyCount[i]);

                stateKey[keyCount] = (void *)chipPipeline->patchKeys[i];
                stateKeySizeInBytes[keyCount] = sizeof(halti5_patch_key) * chipPipeline->patchKeyCount[i];
                keyCount++;
            }
        }

        chipPipeline->keyCount = keyCount;
    }

    chipPipeline->vanilla = (chipPipeline->keyCount == 0);

    masterInstance =(halti5_vscprogram_instance *)__VK_ALLOC(sizeof(halti5_vscprogram_instance), 8,
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR(masterInstance ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(masterInstance, sizeof(halti5_vscprogram_instance));
    masterInstance->devCtx = devCtx;

    __VK_ASSERT(info->stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

    __VK_ONERROR(halti5_helper_createVscResLayout(pip));

    shaderModule = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkShaderModule *, info->stage.module);

    __VK_MEMZERO(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
    vscCompileParams.cfg.ctx.appNameId = devCtx->pPhyDevice->pInst->patchID;
    vscCompileParams.cfg.ctx.isPatchLib = gcvFALSE;
    vscCompileParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
    vscCompileParams.cfg.optFlags = (pip->flags & VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT)
                                  ? 0
                                  : VSC_COMPILER_OPT_FULL;

    if (pip->cache)
    {
        gcsHASH_MD5CTX md5Ctx;
        __vkUtilsHashObject *hashObj = gcvNULL;
        const char *pName = info->stage.pName;
        const VkSpecializationInfo *pSpecialInfo = info->stage.pSpecializationInfo;

        gcsHASH_MD5Init(&md5Ctx);
        gcsHASH_MD5Update(&md5Ctx, &info->stage.stage, sizeof(info->stage.stage));
        gcsHASH_MD5Update(&md5Ctx, shaderModule->pCode, shaderModule->codeSize);
        if (pName)
        {
            gcsHASH_MD5Update(&md5Ctx, pName, gcoOS_StrLen(pName, gcvNULL));
        }
        if (pSpecialInfo && pSpecialInfo->pData)
        {
            gcsHASH_MD5Update(&md5Ctx, pSpecialInfo->pData, pSpecialInfo->dataSize);
        }
        gcsHASH_MD5Final(&md5Ctx, md5digest);

        gcoOS_AcquireMutex(gcvNULL, pip->cache->mutex, gcvINFINITE);
        hashObj = __vk_utils_hashFindObjByKey(pip->cache->moduleHash, md5digest);
        if (hashObj)
        {
            __vkModuleCacheEntry *pEntry = (__vkModuleCacheEntry*)hashObj->pUserData;
            virShader = pEntry->handle;
            halti5_ReferenceVkShader(pEntry->handle);
        }
    }

    if (!virShader)
    {
        SHADER_HANDLE vscShader = VK_NULL_HANDLE;
        SpvDecodeInfo decodeInfo;
        __VK_MEMZERO(&decodeInfo, sizeof(decodeInfo));
        decodeInfo.binary = shaderModule->pCode;
        decodeInfo.sizeInByte = (gctUINT)shaderModule->codeSize;
        decodeInfo.stageInfo = (gctPOINTER)&(info->stage);
        decodeInfo.specFlag = SPV_SPECFLAG_NONE;
        decodeInfo.localSize[0] = decodeInfo.localSize[1] = decodeInfo.localSize[2] = 0;
        decodeInfo.tcsInputVertices = 0;
        decodeInfo.funcCtx = gcvNULL;
        decodeInfo.renderpassInfo = gcvNULL;
        decodeInfo.subPass = ~0U;

        __VK_ONERROR((gcvSTATUS_OK == gcSPV_Decode(&decodeInfo, &vscShader))
                                    ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
        virShader = halti5_CreateVkShader(vscShader);
        __VK_ASSERT(virShader);

        /*save the decoded shader info before compile*/
        __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&chipPipeline->vkShaderDecoded[VSC_SHADER_STAGE_CS], virShader))
                                ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

        __VK_ONERROR(halti5_helper_createVscShaderResLayout(pip, chipPipeline->vscResLayout, VSC_SHADER_STAGE_CS, &vscShaderResLayout));
        vscCompileParams.hShader = vscShader;
        vscCompileParams.pShResourceLayout = &vscShaderResLayout;
        __VK_ONERROR((gcvSTATUS_OK == vscCompileShader(&vscCompileParams, gcvNULL))
                                    ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
        halti5_helper_destroyVscShaderResLayout(pip, &vscShaderResLayout);

        if (pip->cache)
        {
            __vkModuleCacheEntry *pEntry = gcvNULL;
            __VK_SET_ALLOCATIONCB(&pip->cache->memCb);

            pEntry = (__vkModuleCacheEntry*)__VK_ALLOC(sizeof(__vkModuleCacheEntry), 8, VK_SYSTEM_ALLOCATION_SCOPE_CACHE);
            pEntry->head.headBytes = sizeof(__vkModuleCacheHead);
            pEntry->head.magic = 0x12345678;
            pEntry->head.stage = VSC_SHADER_STAGE_CS;
            pEntry->head.patchCase = 0;
            vscQueryShaderBinarySize(virShader->vscHandle, &pEntry->head.binBytes);
            pEntry->head.alignBytes = __VK_ALIGN(pEntry->head.binBytes, 8);
            __VK_MEMCOPY(pEntry->head.hashKey, md5digest, sizeof(md5digest));

            pEntry->handle = virShader;
            __vk_utils_hashAddObj(pMemCb, pip->cache->moduleHash, pEntry, md5digest, VK_FALSE);
            halti5_ReferenceVkShader(pEntry->handle);

            pip->cache->numModules++;
            pip->cache->totalBytes += (pEntry->head.headBytes + pEntry->head.alignBytes);
        }
    }

    if (pip->cache)
    {
        gcoOS_ReleaseMutex(gcvNULL, pip->cache->mutex);
    }

#if __VK_SAVE_THEN_LOAD_SHADER_BIN
        __VK_ONERROR(__vkSaveThenLoadShaderBin(pMemCb, &virShader));
#endif

    __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&virShaderCopy, virShader))
                                ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(&vscLinkParams, sizeof(vscLinkParams));
    vscLinkParams.hShaderArray[VSC_SHADER_STAGE_CS] = virShaderCopy->vscHandle;
    vscLinkParams.pGlApiCfg = &devCtx->pPhyDevice->shaderCaps;
    vscLinkParams.pPgResourceLayout = chipPipeline->vscResLayout;
    __VK_MEMCOPY(&vscLinkParams.cfg, &vscCompileParams.cfg, sizeof(VSC_COMPILER_CONFIG));
    vscLinkParams.cfg.cFlags |= (VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS | VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

    if (devCtx->enabledFeatures.robustBufferAccess)
    {
        vscLinkParams.cfg.cFlags |= VSC_COMPILER_FLAG_NEED_OOB_CHECK;
    }

    __VK_ONERROR((gcvSTATUS_OK == vscLinkProgram(&vscLinkParams, &masterInstance->pep, &masterInstance->hwStates))
        ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

    chipPipeline->curInstance = chipPipeline->masterInstance = masterInstance;

     /*should create ubo if needed*/
    halti5_pip_process_defaultUbo(masterInstance, chipCmptPipeline, VK_FALSE);

    /* process private constants. */
    halti5_pip_process_priv_const(masterInstance, chipCmptPipeline, VK_FALSE, info);

    if (masterInstance->pep.u.vk.privateCombTsHwMappingPool.countOfArray)
    {
        chipPipeline->countOfseparateBinding =
            masterInstance->pep.u.vk.privateCombTsHwMappingPool.countOfArray * VSC_MAX_SHADER_STAGE_COUNT;
        chipPipeline->separateBindingProgramed = (VkBool32 *)__VK_ALLOC(chipPipeline->countOfseparateBinding * sizeof(VkBool32),
            8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

        __VK_ONERROR(chipPipeline->separateBindingProgramed ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(chipPipeline->separateBindingProgramed,
            chipPipeline->countOfseparateBinding * sizeof(VkBool32));
    }

    if (!chipPipeline->vanilla)
    {
        uint32_t masterKey = __vk_utils_evalCrc32_masked_array(stateKey, stateKey, stateKeySizeInBytes, chipPipeline->keyCount);

        chipPipeline->vscProgInstanceHash = __vk_utils_hashCreate(pMemCb, sizeof(uint32_t),
            VSC_PROG_INSTANCE_HASH_ENTRY_NUM, vSC_PROG_INSTANCE_HASH_ENTRY_SIZE, halti5_free_vscprogram_instance);

        __VK_ONERROR(chipPipeline->vscProgInstanceHash ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        masterInstance->ownerCacheObj = __vk_utils_hashAddObj(pMemCb,
            chipPipeline->vscProgInstanceHash,
            masterInstance, &masterKey, VK_TRUE);

        __VK_ONERROR(masterInstance->ownerCacheObj ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

        __vk_utils_objAddRef(masterInstance->ownerCacheObj);

        /* Set accurate patchMask according to information from PEP */
        halti5_pip_build_patchKeyMask(pip);
    }

    if (chipPipeline->tweakHandler)
    {
        (*chipPipeline->tweakHandler->collect)(devCtx, pip, chipPipeline->tweakHandler);
    }

    if (virShaderCopy)
    {
        halti5_DestroyVkShader(virShaderCopy);
    }
    chipPipeline->vkShaderArray[VSC_SHADER_STAGE_CS] = virShader;

    return VK_SUCCESS;

OnError:
    if (masterInstance)
    {
        /* if hash is realdy established, destroy hash would free master instance */
        if (chipPipeline->vscProgInstanceHash && masterInstance->ownerCacheObj)
        {
            __vk_utils_objReleaseRef(masterInstance->ownerCacheObj);
        }
        else
        {
            halti5_free_vscprogram_instance(pMemCb, (void *)masterInstance);
        }
    }

    if (chipPipeline->vscProgInstanceHash)
    {
        __vk_utils_hashDestory(pMemCb, chipPipeline->vscProgInstanceHash);
    }

    halti5_helper_destroyVscResLayout(pip);

    if (virShader)
    {
        halti5_DestroyVkShader(virShader);
    }
    chipPipeline->vkShaderArray[VSC_SHADER_STAGE_CS] = VK_NULL_HANDLE;

    if (chipPipeline->separateBindingProgramed)
    {
        __VK_FREE(chipPipeline->separateBindingProgramed);
    }

    for (i = 0; i < __VK_MAX_DESCRIPTOR_SETS; i++)
    {
        if (chipPipeline->patchKeys[i])
        {
            __VK_FREE(chipPipeline->patchKeys[i]);
        }
        if (chipPipeline->patchTexBufFormat[i])
        {
            __VK_FREE(chipPipeline->patchTexBufFormat[i]);
        }
        if (chipPipeline->patchStorageImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchStorageImgFormat[i]);
        }
        if (chipPipeline->patchCombinedImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchCombinedImgFormat[i]);
        }
        if (chipPipeline->patchResOpBit[i])
        {
            __VK_FREE(chipPipeline->patchResOpBit[i]);
        }
    }
    if (virShaderCopy)
    {
        halti5_DestroyVkShader(virShaderCopy);
    }

    halti5_helper_destroyVscShaderResLayout(pip, &vscShaderResLayout);

    return result;
}





static VkResult halti5_pip_emit_computeShaderInstance(
    __vkDevContext *devCtx,
    __vkPipeline *pip
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;

    chipPipeline->curInstance->curInstanceCmdIndex = 0;

    pCmdBuffer = pCmdBufferBegin = &chipPipeline->curInstance->instanceCmd[chipPipeline->curInstance->curInstanceCmdIndex];

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0403, VK_FALSE, hints->fsMaxTemp);

    chipPipeline->curInstance->curInstanceCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(chipPipeline->curInstance->curInstanceCmdIndex <= HALTI5_INSTANCE_CMD_BUFSIZE);

    return VK_SUCCESS;
}

static VkResult halti5_pip_emit_computeProgram(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkComputePipelineCreateInfo *info
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;
    uint32_t reservedPages;
    uint32_t i;
    const gcsFEATURE_DATABASE *database = devCtx->database;

    /* step 1: emit hardware states from master instance */
    halti5_pip_emit_computeShaderInstance(devCtx, pip);

    /* step 2: emit hardware states from program(pipeline) which shared between instances */
    pCmdBuffer = pCmdBufferBegin = &chipPipeline->cmdBuffer[chipPipeline->curCmdIndex];

    reservedPages = database->SH_SNAP2PAGE_MAXPAGES_FIX  ? 0 : 1;

    if (!database->PSIO_MSAA_CL_FIX)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E06, VK_FALSE,
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
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4))));
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0402, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hints->fsInputCount) & ((gctUINT32) ((((1 ?
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
 20:16))) | (((gctUINT32) ((gctUINT32) (hints->psHighPVaryingCount) & ((gctUINT32) ((((1 ?
 20:16) - (0 ?
 20:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:16) - (0 ? 20:16) + 1))))))) << (0 ? 20:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (hints->psInputControlHighpPosition) & ((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0404, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
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
 20:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0228, VK_FALSE, reservedPages);

    if (database->REG_TessellationShaders)
    {
        gcmASSERT(!(hints->stageBits & gcvPROGRAM_STAGE_TCS_BIT));
        gcmASSERT(!(hints->stageBits & gcvPROGRAM_STAGE_TES_BIT));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52C6, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52C7, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52CD, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:0) - (0 ?
 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 7:0) - (0 ?
 7:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ? 7:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x52CD, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:8) - (0 ?
 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 13:8) - (0 ?
 13:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ? 13:8))));

        __vkCmdLoadSingleHWState(&pCmdBuffer,0x5286, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));

        __vkCmdLoadSingleHWState(&pCmdBuffer,0x5286, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20))));
    }

    if (database->REG_GeometryShader)
    {
        gcmASSERT(!(hints->stageBits & gcvPROGRAM_STAGE_GEOMETRY_BIT));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0440, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0450, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:0) - (0 ?
 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (reservedPages) & ((gctUINT32) ((((1 ?
 5:0) - (0 ?
 5:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ? 5:0))));
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0450, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:21) - (0 ?
 26:21) + 1))))))) << (0 ?
 26:21))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 26:21) - (0 ?
 26:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:21) - (0 ? 26:21) + 1))))))) << (0 ? 26:21))));
    }

    for (i = 0; i < GC_ICACHE_PREFETCH_TABLE_SIZE; i++)
    {
        if (-1 != hints->fsICachePrefetch[i])
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0412, VK_FALSE, hints->fsICachePrefetch[i]);
        }
    }

    chipPipeline->curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}


VkResult halti5_createComputePipeline(
    VkDevice device,
    const VkComputePipelineCreateInfo *info,
    VkPipeline pipeline
    )
{
    __vkPipeline *pip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    __vkDevContext *devCtx = (__vkDevContext *) device;
    VkResult result = VK_SUCCESS;
    halti5_computePipeline *chipCmptPipeline;
    __VK_SET_ALLOCATIONCB(&pip->memCb);

    chipCmptPipeline = (halti5_computePipeline *)__VK_ALLOC(
        sizeof(halti5_computePipeline), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    __VK_ONERROR((chipCmptPipeline ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY));

    __VK_MEMZERO(chipCmptPipeline, sizeof(halti5_computePipeline));

    pip->chipPriv = chipCmptPipeline;

    __VK_ONERROR(halti5_pip_build_check(devCtx, pip));

    __VK_ONERROR(halti5_pip_tweak(devCtx, pip, (void *)info));

    __VK_ONERROR(halti5_pip_build_computeshader(devCtx, pip, info));

    __VK_ONERROR(halti5_pip_emit_computeProgram(devCtx, pip, info));

    __VK_ASSERT(chipCmptPipeline->chipPipeline.curCmdIndex <= __VK_PIPELINE_CMDBUFFER_MAXSIZE);

    return VK_SUCCESS;

OnError:
    if (chipCmptPipeline)
    {
        __VK_FREE(chipCmptPipeline);
    }
    return result;
}


VkResult halti5_destroyPipeline(
    VkDevice device,
    VkPipeline pipeline
    )
{
    __vkDevContext *devCtx = (__vkDevContext *) device;
    __vkPipeline *pip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    uint32_t i;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    halti5_graphicsPipeline *chipGfxPipeline = VK_NULL_HANDLE;
    halti5_computePipeline *chipCmptPipeline = VK_NULL_HANDLE;

    __VK_SET_ALLOCATIONCB(&pip->memCb);

    if (pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
        if (chipGfxPipeline->defaultUbo)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipGfxPipeline->defaultUbo);

            if (buf->memory)
            {
                __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);
            }

            __vk_DestroyBuffer((VkDevice)(uintptr_t)devCtx, chipGfxPipeline->defaultUbo, VK_NULL_HANDLE);
            chipGfxPipeline->defaultUbo = VK_NULL_HANDLE;
        }
    }
    else if (pip->type == __VK_PIPELINE_TYPE_COMPUTE)
    {
        chipCmptPipeline = (halti5_computePipeline *)pip->chipPriv;
        if (chipCmptPipeline->defaultUbo)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, chipCmptPipeline->defaultUbo);

            if (buf->memory)
            {
                __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);
            }

            __vk_DestroyBuffer((VkDevice)(uintptr_t)devCtx, chipCmptPipeline->defaultUbo, VK_NULL_HANDLE);
            chipCmptPipeline->defaultUbo = VK_NULL_HANDLE;
        }
    }

    if (chipPipeline->separateBindingProgramed)
    {
        __VK_FREE(chipPipeline->separateBindingProgramed);
    }

    if (chipPipeline->vanilla)
    {
        if (chipPipeline->masterInstance)
        {
            halti5_free_vscprogram_instance(pMemCb, (void*)chipPipeline->masterInstance);
        }
    }
    else
    {
        if (chipPipeline->curInstance)
        {
            __vk_utils_objReleaseRef(chipPipeline->curInstance->ownerCacheObj);
        }
        if (chipPipeline->vscProgInstanceHash)
        {
            __vk_utils_hashDestory(pMemCb, chipPipeline->vscProgInstanceHash);
        }
    }

    if (chipPipeline->tweakHandler)
    {
        (*chipPipeline->tweakHandler->cleanup)(devCtx, chipPipeline->tweakHandler);
        __VK_FREE(chipPipeline->tweakHandler);
    }

    halti5_helper_destroyVscResLayout(pip);

    if (chipPipeline->linkEntries)
    {
        __VK_FREE(chipPipeline->linkEntries);
    }

    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        if (chipPipeline->vkShaderArray[i])
        {
            halti5_DestroyVkShader(chipPipeline->vkShaderArray[i]);
            chipPipeline->vkShaderArray[i] = VK_NULL_HANDLE;
        }

        if (chipPipeline->vkShaderDecoded[i])
        {
            halti5_DestroyVkShader(chipPipeline->vkShaderDecoded[i]);
            chipPipeline->vkShaderDecoded[i] = VK_NULL_HANDLE;
        }
    }

    for (i = 0; i < __VK_MAX_DESCRIPTOR_SETS; i++)
    {
        if (chipPipeline->patchKeys[i])
        {
            __VK_FREE(chipPipeline->patchKeys[i]);
        }
        if (chipPipeline->patchTexBufFormat[i])
        {
            __VK_FREE(chipPipeline->patchTexBufFormat[i]);
        }
        if (chipPipeline->patchStorageImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchStorageImgFormat[i]);
        }
        if (chipPipeline->patchSampledImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchSampledImgFormat[i]);
        }
        if (chipPipeline->patchCombinedImgFormat[i])
        {
            __VK_FREE(chipPipeline->patchCombinedImgFormat[i]);
        }
        if (chipPipeline->patchResOpBit[i])
        {
            __VK_FREE(chipPipeline->patchResOpBit[i]);
        }
    }

    __VK_FREE(pip->chipPriv);

    return VK_SUCCESS;
}


VkResult halti5_patch_pipeline(
    __vkPipeline *pip,
    __vkCmdBindDescSetInfo *descSetInfo,
    VkBool32 *instanceSwitched
    )
{
    __vkDevContext *devCtx = pip->devCtx;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    uint32_t patchMask = descSetInfo->patchMask;
    uint32_t i = 0, j = 0, stageIdx = 0;
    void *stateKey[__VK_MAX_DESCRIPTOR_SETS] = {0};
    void *stateKeyMask[__VK_MAX_DESCRIPTOR_SETS] = {0};
    uint32_t stateKeySizeInBytes[__VK_MAX_DESCRIPTOR_SETS] = {0};
    uint32_t keyCount = 0;
    uint32_t instanceKey;
    uint32_t totalEntries = 0;
    __vkUtilsHashObject *instanceCacheObj;
    halti5_vscprogram_instance *vscProgInstance = VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;
    VSC_PROG_LIB_LINK_ENTRY *vscLinkEntries = VK_NULL_HANDLE;
    VSC_PROG_LIB_LINK_ENTRY *vscLinkEntriesCur = VK_NULL_HANDLE;
    VkBool32 needCleanInstance = VK_TRUE;
    vkShader_HANDLE hShaderArrayCopy[VSC_MAX_SHADER_STAGE_COUNT];
    vkShader_HANDLE shaderInfo[VSC_MAX_SHADER_STAGE_COUNT];
    VkBool32 newInstance = VK_FALSE;
    struct _gcsHINT *vscHints = VK_NULL_HANDLE;
    VkBool32 txClearPendingFix = devCtx->database->TX_CLEAR_PENDING_FIX;

    __VK_SET_ALLOCATIONCB(&pip->memCb);
    __VK_MEMZERO(hShaderArrayCopy, sizeof(hShaderArrayCopy));
    __VK_MEMZERO(shaderInfo, sizeof(shaderInfo));

    if (patchMask)
    {
        keyCount = chipPipeline->keyCount;
        j = 0;
        for (i = 0; i < keyCount; i++)
        {
            while (j < __VK_MAX_DESCRIPTOR_SETS)
            {
                __vkDescriptorSet *descSet = descSetInfo->descSets[j];

                if (descSet == VK_NULL_HANDLE)
                {
                    j++;
                }
                else
                {
                    halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;
                    stateKey[i] = (void*)chipDescSet->patchKeys;
                    stateKeyMask[i] = (void*)chipPipeline->patchKeys[j];
                    __VK_ASSERT(chipDescSet->numofEntries == chipPipeline->patchKeyCount[j]);
                    stateKeySizeInBytes[i] = chipDescSet->numofEntries * sizeof(halti5_patch_key);
                    j++;
                    break;
                }
            }
        }
    }

    if (keyCount)
    {
        instanceKey = __vk_utils_evalCrc32_masked_array(stateKey, stateKeyMask, stateKeySizeInBytes, keyCount);

        instanceCacheObj = __vk_utils_hashFindObjByKey(chipPipeline->vscProgInstanceHash, &instanceKey);

        if (!instanceCacheObj)
        {
            uint32_t entryIdx = 0, stageCount = 0;
            VSC_PROGRAM_LINKER_PARAM vscLinkParams;
            VSC_PROG_LIB_LINK_TABLE vscLibLinkTable;
            halti5_module *chipModule = (halti5_module*)devCtx->chipPriv;
            struct _gcsHINT *hints = VK_NULL_HANDLE;
            VkBool32 recompiled = VK_FALSE;
            VkBool32 preHighPatch = VK_TRUE;

            for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
            {
                if (chipPipeline->vkShaderArray[i])
                {
                    __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&hShaderArrayCopy[i], chipPipeline->vkShaderArray[i]))
                                                ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
                    stageCount++;
                }
            }

            vscProgInstance = (halti5_vscprogram_instance *)__VK_ALLOC(sizeof(halti5_vscprogram_instance), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

            __VK_ONERROR(vscProgInstance ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
            __VK_MEMZERO(vscProgInstance, sizeof(halti5_vscprogram_instance));
            vscProgInstance->devCtx = devCtx;
            vscProgInstance->ownerCacheObj =
            __vk_utils_hashAddObj(pMemCb, chipPipeline->vscProgInstanceHash, vscProgInstance, &instanceKey, VK_FALSE);
            __VK_ONERROR(vscProgInstance->ownerCacheObj ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
            needCleanInstance = VK_FALSE;
            __VK_MEMZERO(&vscLinkParams, sizeof(vscLinkParams));
            __VK_MEMZERO(&vscLibLinkTable, sizeof(vscLibLinkTable));
            for (i = 0; i < sizeof(hShaderArrayCopy) / sizeof(hShaderArrayCopy[0]); i++)
            {
                vscLinkParams.hShaderArray[i] = hShaderArrayCopy[i] ? hShaderArrayCopy[i]->vscHandle : VK_NULL_HANDLE;
            }
            vscLinkParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
            vscLinkParams.cfg.ctx.appNameId = devCtx->pPhyDevice->pInst->patchID;
            vscLinkParams.cfg.ctx.isPatchLib = gcvFALSE;
            vscLinkParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
            vscLinkParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS
                                     | VSC_COMPILER_FLAG_COMPILE_CODE_GEN
                                     | VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO
                                     | VSC_COMPILER_FLAG_RECOMPILER
                                     | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;

            if (devCtx->enabledFeatures.robustBufferAccess)
            {
                vscLinkParams.cfg.cFlags |= VSC_COMPILER_FLAG_NEED_OOB_CHECK;
            }

            vscLinkParams.cfg.optFlags =  (pip->flags & VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT) ? 0 : VSC_COMPILER_OPT_FULL;
            vscLinkParams.pGlApiCfg = &devCtx->pPhyDevice->shaderCaps;
            vscLinkParams.pPgResourceLayout = chipPipeline->vscResLayout;
            vscLinkParams.pInMasterPEP = &chipPipeline->masterInstance->pep;
            patchMask = descSetInfo->patchMask;

            i = 0;
            while (patchMask)
            {
                if (patchMask & (1 << i))
                {
                    __vkDescriptorSet *descSet = descSetInfo->descSets[i];
                    halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;
                    uint32_t j;
                    for (j = 0 ; j < chipDescSet->numofEntries; j++)
                    {
                        halti5_patch_key validPatchKey = chipDescSet->patchKeys[j] & chipPipeline->patchKeys[i][j];
                        uint32_t stageFlags = chipDescSet->patchInfos[j].patchStages;

                        uint32_t entryCount = 0;
                        uint32_t k = 0;

                        if (!stageFlags)
                        {
                           continue;
                        }

                        while (validPatchKey)
                        {
                            if (validPatchKey & (1 << k))
                            {
                                entryCount++;
                                validPatchKey &= ~(1 << k);
                            }
                            k++;
                        }

                        entryCount *= (stageCount > 0) ? stageCount : 1;
                        totalEntries += entryCount;
                    }
                }
                patchMask &= ~(1<< i);
                i++;
            }

            /*VIV: if totalEntries is zero, needn't do recompile,
            **but need free related resource to avoid memory leak.
            */
            if (totalEntries == 0)
            {
                preHighPatch = VK_FALSE;
            }

            /*handle all the Pre_High level patch per shader stage*/
            for (stageIdx = 0; (stageIdx < VSC_MAX_SHADER_STAGE_COUNT) && preHighPatch; stageIdx++)
            {
                if (chipPipeline->vkShaderDecoded[stageIdx])
                {
                     VSC_SHADER_COMPILER_PARAM vscCompileParams;
                     VSC_SHADER_LIB_LINK_TABLE *shaderLinkTable = VK_NULL_HANDLE;
                     VSC_SHADER_RESOURCE_LAYOUT vscShaderResLayout;
                     VkBool32 compileFlag = VK_FALSE;
                     uint32_t linkIdx = 0;
                     uint32_t defaultEntriesCnt = 16;

                     __VK_ONERROR((VK_SUCCESS == halti5_CopyVkShader(&shaderInfo[stageIdx], chipPipeline->vkShaderDecoded[stageIdx]))
                                                 ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
                    __VK_MEMZERO(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
                    __VK_MEMZERO(&vscShaderResLayout, sizeof(vscShaderResLayout));
                    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
                    vscCompileParams.cfg.ctx.appNameId = devCtx->pPhyDevice->pInst->patchID;
                    vscCompileParams.cfg.ctx.isPatchLib = gcvFALSE;
                    vscCompileParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
                    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
                    vscCompileParams.cfg.optFlags = (pip->flags & VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT)
                                                    ? 0
                                                    : VSC_COMPILER_OPT_FULL;
                    halti5_helper_createVscShaderResLayout(pip, chipPipeline->vscResLayout, stageIdx, &vscShaderResLayout);
                    __VK_ASSERT(chipPipeline->vkShaderDecoded[stageIdx]);
                    vscCompileParams.hShader = shaderInfo[stageIdx]->vscHandle;
                    vscCompileParams.pShResourceLayout = &vscShaderResLayout;

                    vscCompileParams.pShLibLinkTable = (VSC_SHADER_LIB_LINK_TABLE *)__VK_ALLOC(sizeof(VSC_SHADER_LIB_LINK_TABLE), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    shaderLinkTable = vscCompileParams.pShLibLinkTable;
                    __VK_ONERROR(shaderLinkTable ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                    shaderLinkTable->pShLibLinkEntries = (VSC_SHADER_LIB_LINK_ENTRY *)__VK_ALLOC(sizeof(VSC_SHADER_LIB_LINK_ENTRY) * defaultEntriesCnt, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                    __VK_ONERROR(shaderLinkTable->pShLibLinkEntries ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

                    __VK_MEMZERO(shaderLinkTable->pShLibLinkEntries, sizeof(VSC_SHADER_LIB_LINK_ENTRY) * 4);

                    patchMask = descSetInfo->patchMask;
                    i = 0;
                    while (patchMask)
                    {
                        if (patchMask & (1 << i))
                        {
                            __vkDescriptorSet *descSet = descSetInfo->descSets[i];
                            halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;
                            uint32_t j;
                            for (j = 0; j < chipDescSet->numofEntries; j++)
                            {
                                halti5_patch_key validPatchKey = chipDescSet->patchKeys[j] & chipPipeline->patchKeys[i][j];
                                halti5_patch_info *patchInfo = &chipDescSet->patchInfos[j];
                                uint32_t k = 0, m = 0;

                                while (validPatchKey)
                                {
                                    if (validPatchKey & (1 << k))
                                    {
                                        VSC_IMAGE_FORMAT compilerFormat = VSC_IMAGE_FORMAT_NONE;
                                        VSC_IMAGE_FORMAT mapedFormat = VSC_IMAGE_FORMAT_NONE;
                                        VkFormat imageFormat = (VkFormat)patchInfo->patchFormat;
                                        VkFormat shaderFormat = VK_FORMAT_UNDEFINED;
                                        VkFormat originalFormat = (VkFormat)patchInfo->originalFormat;
                                        VkBool32 needPatch = VK_TRUE;

                                        switch (k)
                                        {
                                        case HALTI5_PATCH_FORMAT_TO_COMPILER:
                                            compilerFormat = chipPipeline->patchTexBufFormat[i][j];

                                            if (halti5_isMismatch(compilerFormat,imageFormat))
                                            {
                                                for (m = 0; m < __VK_VSC_DRV_FORMAT_MAP_NUM; m++)
                                                {
                                                    if (imageFormat == mapTable[m].drvFormat)
                                                    {
                                                        mapedFormat = mapTable[m].vscFormat;
                                                        break;
                                                    }
                                                }

                                                if (patchInfo->patchStages & (1 << stageIdx))
                                                {
                                                    __VK_ASSERT(linkIdx < defaultEntriesCnt);

                                                    shaderLinkTable->shLinkEntryCount = linkIdx + 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].applyLevel = VSC_SHLEVEL_Pre_High;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPointCount = 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].hShaderLib = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].pTempHashTable = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_IMAGE_FORMAT;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].strFunc = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.binding = patchInfo->binding;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.imageFormat = mapedFormat;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.set = i;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.arrayIndex = patchInfo->arrayIndex;

                                                    compileFlag = VK_TRUE;
                                                }
                                            }
                                            totalEntries--;
                                            break;
                                        case HALTI5_PATCH_STORAGE_IMAGE_FORMAT:
                                            compilerFormat = chipPipeline->patchStorageImgFormat[i][j];
                                            shaderFormat = mapTable[compilerFormat].drvFormat;

                                            if (((shaderFormat <= VK_FORMAT_R32G32B32A32_SFLOAT &&
                                                shaderFormat >= VK_FORMAT_R32G32B32A32_UINT) ||
                                                (shaderFormat <= VK_FORMAT_R16G16B16A16_SFLOAT &&
                                                shaderFormat >= VK_FORMAT_R16G16B16A16_UNORM)) &&
                                                (originalFormat <= VK_FORMAT_R8G8B8A8_SINT &&
                                                originalFormat >= VK_FORMAT_R8G8B8A8_UNORM))
                                            {
                                                needPatch = VK_FALSE;
                                            }

                                            if (halti5_isMismatch(compilerFormat,originalFormat) && needPatch)
                                            {
                                                for (m = 0; m < __VK_VSC_DRV_FORMAT_MAP_NUM; m++)
                                                {
                                                    if (originalFormat == mapTable[m].drvFormat)
                                                    {
                                                        mapedFormat = mapTable[m].vscFormat;
                                                        break;
                                                    }
                                                }

                                                if (patchInfo->patchStages & (1 << stageIdx))
                                                {
                                                    __VK_ASSERT(linkIdx < defaultEntriesCnt);

                                                    shaderLinkTable->shLinkEntryCount = linkIdx + 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].applyLevel = VSC_SHLEVEL_Pre_High;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPointCount = 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].hShaderLib = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].pTempHashTable = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_IMAGE_FORMAT;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].strFunc = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.binding = patchInfo->binding;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.imageFormat = mapedFormat;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.set = i;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.arrayIndex = patchInfo->arrayIndex;

                                                    compileFlag = VK_TRUE;
                                                }
                                            }
                                            totalEntries--;
                                            break;
                                        case HALTI5_PATCH_SAMPLED_IMAGRE_FORMAT:
                                            compilerFormat = chipPipeline->patchSampledImgFormat[i][j];

                                            if (halti5_isMismatch(compilerFormat, originalFormat))
                                            {
                                                for (m = 0; m < __VK_VSC_DRV_FORMAT_MAP_NUM; m++)
                                                {
                                                    if (originalFormat == mapTable[m].drvFormat)
                                                    {
                                                        mapedFormat = mapTable[m].vscFormat;
                                                        break;
                                                    }
                                                }

                                                if (patchInfo->patchStages & (1 << stageIdx))
                                                {
                                                    __VK_ASSERT(linkIdx < defaultEntriesCnt);

                                                    shaderLinkTable->shLinkEntryCount = linkIdx + 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].applyLevel = VSC_SHLEVEL_Pre_High;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPointCount = 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].hShaderLib = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].pTempHashTable = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_IMAGE_FORMAT;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].strFunc = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.binding = patchInfo->binding;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.imageFormat = mapedFormat;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.set = i;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.arrayIndex = patchInfo->arrayIndex;

                                                    compileFlag = VK_TRUE;
                                                }
                                            }
                                            totalEntries--;
                                            break;
                                        case HALTI5_PATCH_COMBINED_IMAGE_FORMAT:
                                            compilerFormat = chipPipeline->patchCombinedImgFormat[i][j];

                                            if (halti5_isMismatch(compilerFormat,originalFormat))
                                            {
                                                for (m = 0; m < __VK_VSC_DRV_FORMAT_MAP_NUM; m++)
                                                {
                                                    if (originalFormat == mapTable[m].drvFormat)
                                                    {
                                                        mapedFormat = mapTable[m].vscFormat;
                                                        break;
                                                    }
                                                }

                                                if (patchInfo->patchStages & (1 << stageIdx))
                                                {
                                                    __VK_ASSERT(linkIdx < defaultEntriesCnt);

                                                    shaderLinkTable->shLinkEntryCount = linkIdx + 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].applyLevel = VSC_SHLEVEL_Pre_High;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPointCount = 1;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].hShaderLib = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].pTempHashTable = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_IMAGE_FORMAT;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].strFunc = gcvNULL;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.binding = patchInfo->binding;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.imageFormat = mapedFormat;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.set = i;
                                                    shaderLinkTable->pShLibLinkEntries[linkIdx].linkPoint[0].u.imageFormat.arrayIndex = patchInfo->arrayIndex;

                                                    compileFlag = VK_TRUE;
                                                }
                                            }
                                            totalEntries--;
                                            break;
                                        default:
                                            break;
                                        }
                                        validPatchKey &= ~(1 << k);
                                        linkIdx++;
                                    }
                                    k++;
                                }
                            }
                        }
                        patchMask &= ~(1<< i);
                        i++;
                    }

                    if (compileFlag)
                    {
                        __VK_ONERROR((gcvSTATUS_OK == vscCompileShader(&vscCompileParams, gcvNULL))
                                                      ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

                        vscLinkParams.hShaderArray[stageIdx] = shaderInfo[stageIdx]->vscHandle;
                        recompiled = VK_TRUE;
                    }

                    halti5_helper_destroyVscShaderResLayout(pip, &vscShaderResLayout);
                    __VK_FREE(shaderLinkTable->pShLibLinkEntries);
                    __VK_FREE(shaderLinkTable);

                }
            }

            totalEntries += chipPipeline->linkEntryCount;

            /*VIV: the follow two situations no need do reLink:
            **1.totalEntries is zero and didn't do recompile;
            **2.totalEntries is none zero, but didn't do recompile and needn't do other patch.
            */
            if ((!totalEntries && !recompiled) ||
                (totalEntries && !recompiled &&
                (totalEntries == chipPipeline->linkEntryCount)))
            {
                for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
                {
                    if (hShaderArrayCopy[i])
                    {
                        halti5_DestroyVkShader(hShaderArrayCopy[i]);
                    }
                    if (shaderInfo[i])
                    {
                        halti5_DestroyVkShader(shaderInfo[i]);
                    }
                }
                if (vscProgInstance)
                {
                    __vk_utils_hashDeleteObj(pMemCb, chipPipeline->vscProgInstanceHash, vscProgInstance->ownerCacheObj);
                    vscProgInstance = VK_NULL_HANDLE;
                }
                vscProgInstance = chipPipeline->masterInstance;
                goto PATCHEND;
            }else if (!totalEntries)
            {
                vscLinkEntries = VK_NULL_HANDLE;
            }
            else
            {
                vscLinkEntries = (VSC_PROG_LIB_LINK_ENTRY *)__VK_ALLOC(sizeof(VSC_PROG_LIB_LINK_ENTRY) * totalEntries, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                __VK_ONERROR(vscLinkEntries ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

                vscLinkEntriesCur = vscLinkEntries;
                if (chipPipeline->linkEntryCount)
                {
                    __VK_MEMCOPY(vscLinkEntriesCur, chipPipeline->linkEntries, sizeof(VSC_PROG_LIB_LINK_ENTRY) * chipPipeline->linkEntryCount);
                    vscLinkEntriesCur += chipPipeline->linkEntryCount;
                }

                patchMask = descSetInfo->patchMask;
                i = 0;
                while (patchMask)
                {
                    if (patchMask & (1 << i))
                    {
                        __vkDescriptorSet *descSet = descSetInfo->descSets[i];
                        halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;
                        uint32_t j;
                        for (j = 0; j < chipDescSet->numofEntries; j++)
                        {
                            halti5_patch_key validPatchKey = chipDescSet->patchKeys[j] & chipPipeline->patchKeys[i][j];
                            halti5_patch_info *patchInfo = &chipDescSet->patchInfos[j];
                            uint32_t k = 0;

                            while (validPatchKey)
                            {
                                if (validPatchKey & (1 << k))
                                {

                                    if (k == HALTI5_PATCH_FORMAT_TO_COMPILER ||
                                        k == HALTI5_PATCH_STORAGE_IMAGE_FORMAT ||
                                        k == HALTI5_PATCH_SAMPLED_IMAGRE_FORMAT ||
                                        k == HALTI5_PATCH_COMBINED_IMAGE_FORMAT)
                                    {
                                        validPatchKey &= ~(1 << k);
                                    }
                                    else
                                    {
                                        VSC_RES_OP_BIT opTypeBits = 0;
                                        VSC_RES_ACT_BIT actBits = 0;
                                        uint32_t subType;
                                        VSC_RES_OP_BIT resOpBits = chipPipeline->patchResOpBit[i][j];
                                        VkBool32 is128BppFormat =
                                            (patchInfo->patchFormat <= __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT) &&
                                            (patchInfo->patchFormat >= __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT);
                                        VkBool32 is2DViewType = (VK_IMAGE_VIEW_TYPE_2D == patchInfo->viewType);
                                        VkBool32 onlyTexLod = (resOpBits & VSC_RES_OP_BIT_TEXLD_LOD) &&
                                                              !(resOpBits & (VSC_RES_OP_BIT_TEXLD_BIAS |
                                                              VSC_RES_OP_BIT_TEXLD | VSC_RES_OP_BIT_TEXLDP |
                                                              VSC_RES_OP_BIT_TEXLDP_BIAS | VSC_RES_OP_BIT_TEXLDP_LOD));

                                        if (k == 1 && onlyTexLod && is128BppFormat && is2DViewType)
                                        {
                                            switch (patchInfo->patchFormat)
                                            {
                                            case __VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT:
                                                vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].strFunc =
                                                    "_inputcvt_R32G32B32A32SFLOAT_2_R32G32SFLOAT_TEXLD_LOD";
                                                break;
                                            case __VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT:
                                                vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].strFunc =
                                                    "_inputcvt_R32G32B32A32SINT_2_R32G32SINT_TEXLD_LOD";
                                                break;
                                            case __VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT:
                                                vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].strFunc =
                                                    "_inputcvt_R32G32B32A32UINT_2_R32G32UINT_TEXLD_LOD";
                                                break;
                                            default:
                                                break;
                                            }

                                            opTypeBits = VSC_RES_OP_BIT_TEXLD_LOD;
                                            actBits = VSC_RES_ACT_BIT_EXTRA_SAMPLER;
                                            subType = VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LAYER_SPECIFIED_OP;
                                        }
                                        else
                                        {
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].strFunc =
                                            halti5_helper_patchFuc(patchInfo->patchFormat, k, patchInfo->viewType, &opTypeBits, &actBits, &subType);
                                        }

                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.applyLevel = VSC_SHLEVEL_Post_Medium;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.hShaderLib = chipModule->patchLib->vscHandle;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.pTempHashTable = gcvNULL;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_RESOURCE;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.set = i;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.binding = patchInfo->binding;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.arrayIndex = patchInfo->arrayIndex;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.opTypeBits = opTypeBits;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.actBits = actBits;
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPoint[0].u.resource.subType = subType;
                                        switch (k)
                                        {
                                        case HALTI5_PATCH_TX_GATHER_PCF:
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.libSpecializationConstantCount = 2;
                                            break;
                                        default:
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.libSpecializationConstantCount = 1;
                                            break;
                                        }
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts =
                                                      (VSC_LIB_SPECIALIZATION_CONSTANT *)__VK_ALLOC(sizeof(VSC_LIB_SPECIALIZATION_CONSTANT) *
                                                                                                    vscLinkEntriesCur[entryIdx].shLibLinkEntry.libSpecializationConstantCount,
                                                                                                    8,
                                                                                                    VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
                                        __VK_ONERROR(vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[0].varName = "swizzles";
                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[0].type = VSC_SHADER_DATA_TYPE_INTEGER_X4;
                                        __VK_MEMCOPY(&vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[0].value, patchInfo->swizzles, sizeof(patchInfo->swizzles));
                                        if (k == HALTI5_PATCH_TX_GATHER_PCF)
                                        {
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[1].varName = "comparemode";
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[1].type = VSC_SHADER_DATA_TYPE_INTEGER_X4;
                                            vscLinkEntriesCur[entryIdx].shLibLinkEntry.pLibSpecializationConsts[1].value.uValue[0] = patchInfo->compareOp;
                                        }

                                        vscLinkEntriesCur[entryIdx].shLibLinkEntry.linkPointCount = 1;
                                        vscLinkEntriesCur[entryIdx].mainShaderStageBits = (VSC_SHADER_STAGE_BIT)chipDescSet->patchInfos[j].patchStages;
                                        entryIdx++;
                                        validPatchKey &= ~(1 << k);
                                    }
                                }
                                k++;
                            }
                        }
                    }
                    patchMask &= ~(1<< i);
                    i++;
                }
            }

            vscLibLinkTable.progLinkEntryCount = entryIdx + chipPipeline->linkEntryCount;
            vscLibLinkTable.pProgLibLinkEntries = vscLinkEntries;
            vscLinkParams.pProgLibLinkTable = &vscLibLinkTable;

            __VK_ONERROR((gcvSTATUS_OK == vscLinkProgram(&vscLinkParams, &vscProgInstance->pep, &vscProgInstance->hwStates))
                                        ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);
            newInstance = VK_TRUE;

            hints = &vscProgInstance->hwStates.hints;

            for (i = 0; i < gcvPROGRAM_STAGE_LAST; i++)
            {
                if (hints && (hints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][i] & gceMA_FLAG_WRITE))
                {
                    vscProgInstance->memoryWrite = gcvTRUE;
                    break;
                }
            }

            for (i = 0; i < entryIdx; i++)
            {
                if (vscLinkEntries[i].shLibLinkEntry.pLibSpecializationConsts)
                {
                    __VK_FREE(vscLinkEntries[i].shLibLinkEntry.pLibSpecializationConsts);
                }
            }

            if (vscLinkEntries)
            {
                __VK_FREE(vscLinkEntries);
            }

            for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
            {
                if (hShaderArrayCopy[i])
                {
                    halti5_DestroyVkShader(hShaderArrayCopy[i]);
                }
                if (shaderInfo[i])
                {
                    halti5_DestroyVkShader(shaderInfo[i]);
                }
            }
        }
        else
        {
            vscProgInstance = (halti5_vscprogram_instance *)instanceCacheObj->pUserData;
        }
    }
    else
    {
        vscProgInstance = chipPipeline->masterInstance;
    }

PATCHEND:
    vscHints = &vscProgInstance->hwStates.hints;

    if (vscHints && (!txClearPendingFix))
    {
        for (j = 0; j < gcvPROGRAM_STAGE_LAST; j++)
        {
            if ((vscHints->memoryAccessFlags[gcvSHADER_MACHINE_LEVEL][j] & (gceMA_FLAG_IMG_WRITE | gceMA_FLAG_IMG_READ)) &&
                (vscHints->texldFlags[gcvSHADER_MACHINE_LEVEL][j] & gceTEXLD_FLAG_TEXLD))
            {
                chipPipeline->fastFilterDisable = VK_TRUE;
                break;
            }
        }
    }

    if (j == gcvPROGRAM_STAGE_LAST)
    {
        chipPipeline->fastFilterDisable = VK_FALSE;
    }

    if (vscProgInstance != chipPipeline->curInstance)
    {
        __vk_utils_objReleaseRef(chipPipeline->curInstance->ownerCacheObj);
        __vk_utils_objAddRef(vscProgInstance->ownerCacheObj);
        chipPipeline->curInstance = vscProgInstance;
        *instanceSwitched = VK_TRUE;
        if (newInstance)
        {
            switch (pip->type)
            {
            case __VK_PIPELINE_TYPE_GRAPHICS:
                halti5_pip_emit_graphicsShaderInstance(devCtx, pip);
                break;
            case __VK_PIPELINE_TYPE_COMPUTE:
                halti5_pip_emit_computeShaderInstance(devCtx, pip);
                break;
            default:
                __VK_ASSERT(0);
                break;
            }
        }
    }

    return VK_SUCCESS;

OnError:
    for (i = 0; i < VSC_MAX_SHADER_STAGE_COUNT; i++)
    {
        if (hShaderArrayCopy[i])
        {
            halti5_DestroyVkShader(hShaderArrayCopy[i]);
        }
    }
    if (vscLinkEntries)
    {
        for (i = 0; i < totalEntries; i ++)
        {
            if (vscLinkEntries[i].shLibLinkEntry.pLibSpecializationConsts)
            {
                __VK_FREE(vscLinkEntries[i].shLibLinkEntry.pLibSpecializationConsts);
            }
        }

        __VK_FREE(vscLinkEntries);
    }
    if (vscProgInstance && needCleanInstance)
    {
        halti5_free_vscprogram_instance(pMemCb, (void *)vscProgInstance);
        __VK_FREE(vscProgInstance);
    }


    return result;
}


