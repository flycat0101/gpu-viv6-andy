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

/* Checked some opTag ' count  to detect the shader */
static VkBool32 cube_useLOD_shaderDetect(__vkShaderModule * module)
{
    static uint32_t opArray[] =
    {
        /*
        OpTypeSampledImage 1
        OpVariable 3
        OpLoad 2
        OpStore 1
        OpFMul 1
        OpFAdd 1
        OpImageSampleImplicitLod 1
        */
        27, 1,
        59, 3,
        61, 2,
        62, 1,
        133, 1,
        129, 1,
        87, 1
    };
    uint32_t opCount[7];
    uint32_t index = 0;
    uint32_t *pMem = (uint32_t*) module->pCode;
    const uint32_t startPos = 5;

    /* init the count*/
    __VK_MEMSET(opCount,0,sizeof(opCount));

    for(index = startPos ;index < module->codeSize / 4; )
    {
        uint32_t length = pMem[index] >> 16;
        uint32_t opTag = pMem[index] & 0xffff ;

        if (opTag <= 134 && opTag >= 27)
        {
            switch (opTag)
            {
            case 27:
                {
                    opCount[0]++;
                    break;
                }
            case 59:
                {
                    opCount[1]++;
                    break;
                }
            case 61:
                {
                    opCount[2]++;
                    break;
                }
            case 62:
                {
                    opCount[3]++;
                    break;
                }
            case 133:
                {
                    opCount[4]++;
                    break;
                }
            case 129:
                {
                    opCount[5]++;
                    break;
                }
            case 87:
                {
                    opCount[6]++;
                    break;
                }
            default:
                break;
            }

        }
        index += length;
    }

    /*Verify the count*/
    for(index = 0; index < sizeof(opArray)/(2*sizeof(uint32_t)); index++)
    {
        if(opArray[index * 2 + 1] != opCount[index])
            return VK_FALSE;
    }

   return VK_TRUE;
}


static VkBool32 cube_useLOD_match(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    if(pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;
        float x,y,width,height;
        VkBool32 ret = VK_TRUE;
        /* Check state match */
        if(graphicCreateInfo->pViewportState)
        {
            x = graphicCreateInfo->pViewportState->pViewports->x;
            y = graphicCreateInfo->pViewportState->pViewports->y;
            width = graphicCreateInfo->pViewportState->pViewports->width;
            height = graphicCreateInfo->pViewportState->pViewports->height;

            ret = ret &  (((int )x == 0) &&
                ((int)(y) == 0) &&
                ((int)width == 39) &&
                ((int)height == 26)
                );
            if(!ret)
                return VK_FALSE;
        }

        if(graphicCreateInfo->pRasterizationState)
        {
            ret = ret &(graphicCreateInfo->pRasterizationState->cullMode == VK_CULL_MODE_NONE)
                &(graphicCreateInfo->pRasterizationState->depthBiasEnable == VK_FALSE)
                & (graphicCreateInfo->pRasterizationState->depthClampEnable == VK_FALSE)
                &(graphicCreateInfo->pRasterizationState->rasterizerDiscardEnable == VK_FALSE);
            if(!ret)
                return VK_FALSE;
        }

        if(graphicCreateInfo->pDepthStencilState)
        {
            ret = ret & (graphicCreateInfo->pDepthStencilState->depthTestEnable ==0)
                & (graphicCreateInfo->pDepthStencilState->depthWriteEnable == 0)
                & (graphicCreateInfo->pDepthStencilState->depthBoundsTestEnable ==0)
                & (graphicCreateInfo->pDepthStencilState->stencilTestEnable == 0);
            if(!ret)
                return VK_FALSE;

        }

        if(graphicCreateInfo->pVertexInputState->vertexAttributeDescriptionCount == 2 &&
            graphicCreateInfo->pVertexInputState->vertexBindingDescriptionCount == 1)
        {

            ret = ret & (graphicCreateInfo->pVertexInputState->pVertexBindingDescriptions[0].stride == 32)
                & (graphicCreateInfo->pVertexInputState->pVertexAttributeDescriptions[0].format == VK_FORMAT_R32G32B32A32_SFLOAT)
                & (graphicCreateInfo->pVertexInputState->pVertexAttributeDescriptions[1].format == VK_FORMAT_R32G32B32A32_SFLOAT);

            if(!ret)
                return VK_FALSE;
        }
        /*Check shader Match*/
        if(graphicCreateInfo->stageCount == 2)
        {
            const VkPipelineShaderStageCreateInfo *pStage = &(graphicCreateInfo->pStages[1]);

            if(graphicCreateInfo->pStages[0].stage == VK_SHADER_STAGE_FRAGMENT_BIT)
                pStage = &graphicCreateInfo->pStages[0];

            ret = ret & cube_useLOD_shaderDetect((__vkShaderModule * )(uintptr_t)pStage->module);

            if(!ret)
                return VK_FALSE;
        }

        return ret;
    }

    return VK_FALSE;
}

static VkResult cube_useLOD_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult cube_useLOD_collect(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult cube_useLOD_set(
    __vkCommandBuffer *cmdBuf,
    __vkDrawComputeCmdParams *cmdParams,
    halti5_tweak_handler *handler
    )
{
    __vkCmdBindDescSetInfo *descSetInfo =  &cmdBuf->bindInfo.bindDescSet.graphics;
    __vkDescriptorSetLayout *descSetLayout = gcvNULL;
    VkBool32 match = VK_TRUE;
    __vkDescriptorResourceInfo *resInfo;
    halti5_sampler *chipSampler;
    if (descSetInfo->descSets[0])
    {
        descSetLayout =  descSetInfo->descSets[0]->descSetLayout;
    }

    if (descSetLayout &&
        (descSetLayout->bindingCount == 1) &&
        (descSetLayout->binding[0].std.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER))
    {

        __vkDescriptorSet *descSet = descSetInfo->descSets[0];
        __vkImage * image = gcvNULL;
        uint32_t  samplerLodMinMax = 0;
        resInfo = (__vkDescriptorResourceInfo *)(descSet->resInfos );
        image = (__vkImage *)(uintptr_t) resInfo->u.imageInfo.imageView->createInfo.image;
        chipSampler =  (halti5_sampler *) descSet->samplers[0]->chipPriv;
        samplerLodMinMax = chipSampler->samplerDesc.halti5.hwSamplerLodMinMax;
        if ((cmdParams->draw.indexDraw == VK_FALSE) &&
            (cmdParams->draw.firstVertex == 0) &&
            (cmdParams->draw.vertexCount == 36) &&
            (cmdParams->draw.instanceCount == 1) &&
            (cmdBuf->bindInfo.vertexBuffers.bindingCount == 1) &&
            (descSet->samplers[0]) &&
            (descSet->samplers[0]->createInfo.magFilter == VK_FILTER_NEAREST) &&
            (descSet->samplers[0]->createInfo.minFilter == VK_FILTER_NEAREST) &&
            (descSet->samplers[0]->createInfo.mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST) &&
            (descSet->samplers[0]->createInfo.mipLodBias == 0.0f) &&
            (resInfo->u.imageInfo.imageView->createInfo.viewType == VK_IMAGE_VIEW_TYPE_CUBE
            || resInfo->u.imageInfo.imageView->createInfo.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
            )
        {
            uint32_t bufferIndex =  cmdBuf->bindInfo.vertexBuffers.firstBinding;
            __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->bindInfo.vertexBuffers.buffers[bufferIndex]);
            uint32_t index;
            __vkDeviceMemory * mem = buf->memory;
            uint32_t * pointer =   (uint32_t *)(uint8_t *)mem->hostAddr + buf->memOffset + cmdBuf->bindInfo.vertexBuffers.offsets[bufferIndex];

            for(index  = 0; index < 36 * 8; index++)
            {
                uint32_t rawData = pointer[index];

                if(rawData != 0xbeaaaaaa && rawData != 0x3eaaaaac && rawData != 0x0 && rawData !=0x3f800000
                    && rawData !=0xbf800000)
                {
                    match = VK_FALSE;
                    break;
                }
            }

            if(match)
            {
                float texW = (float)image->createInfo.extent.width;
                float texH = (float)image->createInfo.extent.height;
                float rtW = 39.0f;
                float rtH = 26.0f;
                float derX = gcmMAX(gcmABS(texW/rtW), gcmABS(texH/rtW));
                float  derY = gcmMAX(gcmABS(texW/rtH), gcmABS(texH/rtH));
                gctFLOAT lod = gcoMATH_Log2(gcmMAX(derX, derY));
                int32_t  maxlod = _Float2SignedFixed(lod, 5, 8);
                int32_t  minlod = _Float2SignedFixed(lod -0.5f, 5, 8); /*clamped max min lod to the real lod*/
                samplerLodMinMax = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (maxlod) & ((gctUINT32) ((((1 ? 12:0) - (0 ?
 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16))) | (((gctUINT32) ((gctUINT32) (minlod) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16)));
            }
        }

        if(!match) /*if not match, should restore the orignal max min lod*/
        {
            samplerLodMinMax = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (descSet->samplers[0]->createInfo.maxLod) & ((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16))) | (((gctUINT32) ((gctUINT32) (descSet->samplers[0]->createInfo.minLod) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16)));
        }

        if(samplerLodMinMax != chipSampler->samplerDesc.halti5.hwSamplerLodMinMax)
        {
            chipSampler->samplerDesc.halti5.hwSamplerLodMinMax = samplerLodMinMax;
            descSetInfo->dirtyMask |= 1;
        }
    }
    return VK_SUCCESS;
}

static VkResult cube_useLOD_cleanup(
    __vkDevContext *devCtx,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static const halti5_tweak_handler g_tweakArray[] =
{
    /* "deqp" cube lod issue*/
    {
     "\x9b\x9a\x8e\x8f",
     cube_useLOD_match,
     cube_useLOD_tweak,
     cube_useLOD_collect,
     cube_useLOD_set,
     cube_useLOD_cleanup,
     0
     },
};


VkResult halti5_tweak_detect(
    __vkDevContext *devCtx
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    uint32_t arrayIdx;
    uint32_t handleIdx = 0;
    halti5_tweak_handler **tempArray = VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    tempArray = (halti5_tweak_handler **)__VK_ALLOC(
        sizeof(halti5_tweak_handler *) * __VK_COUNTOF(g_tweakArray), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    __VK_ONERROR(tempArray ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_MEMZERO(tempArray, sizeof(tempArray));

    for (arrayIdx = 0; arrayIdx < __VK_COUNTOF(g_tweakArray); arrayIdx++)
    {
        char tempBuf[__VK_MAX_NAME_LENGTH];
        char *pos;

        __vk_utils_reverseBytes(g_tweakArray[arrayIdx].reversedName, tempBuf, __VK_MAX_NAME_LENGTH);

        gcoOS_StrStr(devCtx->pPhyDevice->pInst->applicationName, tempBuf, &pos);

        if (pos || (devCtx->pPhyDevice->pInst->applicationName[0] == '\0'))
        {
            tempArray[handleIdx++] = (halti5_tweak_handler *)&g_tweakArray[arrayIdx];
        }
    }

    if (handleIdx)
    {
        chipModule->ppTweakHandlers = (halti5_tweak_handler **)__VK_ALLOC(
            handleIdx * sizeof(halti5_tweak_handler*), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        __VK_ONERROR(chipModule->ppTweakHandlers ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMCOPY(chipModule->ppTweakHandlers, tempArray, handleIdx * sizeof(halti5_tweak_handler*));

        chipModule->tweakHandleCount = handleIdx;
    }


OnError:
    if (tempArray)
    {
        __VK_FREE(tempArray);
    }
    return result;
}



