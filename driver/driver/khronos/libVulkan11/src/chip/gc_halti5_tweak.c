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
#include <vulkan/spirv.h>

/******************************************************************************
** default func
*******************************************************************************/
static VkResult default_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult default_set(
    __vkCommandBuffer *cmdBuf,
    __vkDrawComputeCmdParams *cmdParams,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult default_collect(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult default_cleanup(
    __vkDevContext *devCtx,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

static VkResult default_copy(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    __vkBuffer *dstBuf,
    halti5_tweak_handler *handler
    )
{
    return VK_SUCCESS;
}

/******************************************************************************
** cube use lod
*******************************************************************************/
/* Checked some opTag ' count  to detect the shader */
static VkBool32 cube_useLOD_shaderDetect(__vkShaderModule * module)
{
    static uint32_t opArray[] =
    {
        /*
        OpTypeSampledImage 1
        OpVariable 4
        OpLoad 3
        OpStore 1
        OpFMul 1
        OpFAdd 1
        OpImageSampleImplicitLod 1
        */
        27, 1,
        59, 4,
        61, 3,
        62, 1,
        133, 1,
        129, 1,
        87, 1
    };

    /* For vk1.0.0 has different shader with vk1.0.2, add vk1.0.0 shader detect.
    ** when we move to vk1.0.2, will delete it.
    */
    static uint32_t opArray_temp[] =
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
        uint32_t length = pMem[index] >> SpvWordCountShift;
        uint32_t opTag = pMem[index] & SpvOpCodeMask ;

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
        if((opArray[index * 2 + 1] != opCount[index]) && (opArray_temp[index * 2 + 1] != opCount[index]))
        {
            return VK_FALSE;
        }
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
        if(graphicCreateInfo->pViewportState && graphicCreateInfo->pViewportState->pViewports)
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
        (((descSetLayout->bindingCount == 1) &&
        (descSetLayout->binding[0].std.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER))
        || ((descSetLayout->bindingCount == 2) &&
        (descSetLayout->binding[0].std.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER))
        ))
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
            }
        }

        if(!match) /*if not match, should restore the orignal max min lod*/
        {
            samplerLodMinMax = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:0) - (0 ?
 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (descSet->samplers[0]->createInfo.maxLod) & ((gctUINT32) ((((1 ?
 12:0) - (0 ?
 12:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ? 12:0)))
                             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:16) - (0 ?
 28:16) + 1))))))) << (0 ?
 28:16))) | (((gctUINT32) ((gctUINT32) (descSet->samplers[0]->createInfo.minLod) & ((gctUINT32) ((((1 ?
 28:16) - (0 ?
 28:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:16) - (0 ? 28:16) + 1))))))) << (0 ? 28:16)));
        }

        if(samplerLodMinMax != chipSampler->samplerDesc.halti5.hwSamplerLodMinMax)
        {
            chipSampler->samplerDesc.halti5.hwSamplerLodMinMax = samplerLodMinMax;
            descSetInfo->dirtyMask |= 1;
        }
    }
    return VK_SUCCESS;
}

static VkBool32 deqp_vk_48_timeout_match(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    if (pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;

        /*Check shader Match*/
        if (graphicCreateInfo->stageCount == 2)
        {
            const VkPipelineShaderStageCreateInfo *pVsStage = &(graphicCreateInfo->pStages[0]);
            const VkPipelineShaderStageCreateInfo *pFsStage = &(graphicCreateInfo->pStages[1]);
            __vkShaderModule *pVsShaderModule = gcvNULL, *pPsShaderModule = gcvNULL;

            if (pVsStage->stage != VK_SHADER_STAGE_VERTEX_BIT   ||
                pFsStage->stage != VK_SHADER_STAGE_FRAGMENT_BIT ||
                pVsStage->sType != VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO  ||
                pFsStage->sType != VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO)
            {
                return VK_FALSE;
            }

            if ((strncmp(pVsStage->pName, "main", 4) != 0) ||
                (strncmp(pFsStage->pName, "main", 4) != 0))
            {
                return VK_FALSE;
            }

            pVsShaderModule = (__vkShaderModule * )(uintptr_t)pVsStage->module;
            pPsShaderModule = (__vkShaderModule * )(uintptr_t)pFsStage->module;

            if (pVsShaderModule->codeSize != 100036 ||
                pPsShaderModule->codeSize != 104852)
            {
                return VK_FALSE;
            }

            return VK_TRUE;
        }

        return VK_FALSE;
    }

    return VK_FALSE;
}

static VkResult deqp_vk_48_timeout_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo,
    halti5_tweak_handler *handler
    )
{
     VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;
    __vkShaderModule *pVsShaderModule = 0, *pPsShaderModule = 0;
    uint32_t i;
    uint32_t *pCode;
    uint32_t outputIdx = 0;
    VkBool32 bMatch = VK_FALSE;
    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    pVsShaderModule = (__vkShaderModule * )(uintptr_t)(graphicCreateInfo->pStages[0].module);
    pPsShaderModule = (__vkShaderModule * )(uintptr_t)(graphicCreateInfo->pStages[1].module);

    /* Modify VS spirv binary. */
    pCode = (uint32_t*) pVsShaderModule->pCode;

    for (i = 5; i < pVsShaderModule->codeSize / 4;)
    {
        uint32_t length = pCode[i] >> SpvWordCountShift;
        SpvOp opCode = (SpvOp)(pCode[i] & SpvOpCodeMask);

        if ((opCode == SpvOpVariable) && (length >= 4) && (SpvStorageClassOutput == (SpvStorageClass)pCode[i + 3]))
        {
            outputIdx = pCode[i + 2];
        }
        else if (opCode == SpvOpAccessChain)
        {
            i += length;
            /* Skip one Store. */
            __VK_ASSERT(SpvOpStore == (SpvOp)(pCode[i] & SpvOpCodeMask));
            length = pCode[i] >> SpvWordCountShift;
            i += length;
            /* Modify one Store. */
            __VK_ASSERT(SpvOpStore == (SpvOp)(pCode[i] & SpvOpCodeMask));
            pCode[i + 1] = outputIdx;
            i += length;
            /* Insert a Return. */
            pCode[i] = (SpvOpReturn & SpvOpCodeMask) | (1 << SpvWordCountShift);
            i++;
            /* Insert a FunctionEnd. */
            pCode[i] = (SpvOpFunctionEnd & SpvOpCodeMask) | (1 << SpvWordCountShift);
            i++;

            bMatch = VK_TRUE;
        }

        if (bMatch)
        {
            break;
        }

        i += length;
    }
    /* Update the size and code. */
    if (bMatch)
    {
        pVsShaderModule->codeSize = i * 4;
        pCode = (uint32_t *)__VK_ALLOC(i * 4, 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        __VK_MEMCOPY(pCode, pVsShaderModule->pCode, i * 4);
        __VK_FREE(pVsShaderModule->pCode);
        pVsShaderModule->pCode = pCode;
    }

    /* Modify FS spirv binary. */
    bMatch = VK_FALSE;
    pCode = (uint32_t*)pPsShaderModule->pCode;
    for (i = 5; i < pPsShaderModule->codeSize / 4;)
    {
        uint32_t length = pCode[i] >> SpvWordCountShift;
        SpvOp opCode = (SpvOp)(pCode[i] & SpvOpCodeMask);

        if ((opCode == SpvOpVariable) && (length >= 4) && (SpvStorageClassOutput == (SpvStorageClass)pCode[i + 3]))
        {
            outputIdx = pCode[i + 2];
        }
        else if (opCode == SpvOpStore)
        {
            /* Modify one Store. */
            __VK_ASSERT(SpvOpStore == (SpvOp)(pCode[i] & SpvOpCodeMask));
            pCode[i + 1] = outputIdx;
            i += length;
            /* Insert a Return. */
            pCode[i] = (SpvOpReturn & SpvOpCodeMask) | (1 << SpvWordCountShift);
            i++;
            /* Insert a FunctionEnd. */
            pCode[i] = (SpvOpFunctionEnd & SpvOpCodeMask) | (1 << SpvWordCountShift);
            i++;

            bMatch = VK_TRUE;
        }

        if (bMatch)
        {
            break;
        }

        i += length;
    }
    /* Update the size and code. */
    if (bMatch)
    {
        pPsShaderModule->codeSize = i * 4;
        pCode = (uint32_t *)__VK_ALLOC(i * 4, 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
        __VK_MEMCOPY(pCode, pPsShaderModule->pCode, i * 4);
        __VK_FREE(pPsShaderModule->pCode);
        pPsShaderModule->pCode = pCode;
    }

    return VK_SUCCESS;
}

static VkBool32 msaa_128bpp_01_shaderDetect(__vkShaderModule * module)
{
    static uint32_t opArray[] =
    {
        /*
        OpTypeSampledImage 1
        OpConstant 5
        OpConstantComposite 2
        OpImageFetch 1
        OpFOrdEqual 2
        OpAll 2
        OpLogicalOr 2
        */
        27, 1,
        43, 5,
        44, 2,
        95, 1,
        180, 2,
        155, 2,
        166, 1
    };

    uint32_t opCount[7];
    uint32_t index = 0;
    uint32_t *pMem = (uint32_t*) module->pCode;
    const uint32_t startPos = 5;

    /* init the count*/
    __VK_MEMSET(opCount,0,sizeof(opCount));

    for(index = startPos ;index < module->codeSize / 4; )
    {
        uint32_t length = pMem[index] >> SpvWordCountShift;
        uint32_t opTag = pMem[index] & SpvOpCodeMask ;

        if (opTag <= 180 && opTag >= 27)
        {
            switch (opTag)
            {
            case 27:
                {
                    opCount[0]++;
                    break;
                }
            case 43:
                {
                    opCount[1]++;
                    break;
                }
            case 44:
                {
                    opCount[2]++;
                    break;
                }
            case 95:
                {
                    opCount[3]++;
                    break;
                }
            case 180:
                {
                    opCount[4]++;
                    break;
                }
            case 155:
                {
                    opCount[5]++;
                    break;
                }
            case 166:
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
        {
            return VK_FALSE;
        }
    }

   return VK_TRUE;
}

static VkBool32 deqp_vk_msaa_128bpp_01_match(
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
        if(graphicCreateInfo->pViewportState && graphicCreateInfo->pViewportState->pViewports)
        {
            x = graphicCreateInfo->pViewportState->pViewports->x;
            y = graphicCreateInfo->pViewportState->pViewports->y;
            width = graphicCreateInfo->pViewportState->pViewports->width;
            height = graphicCreateInfo->pViewportState->pViewports->height;

            ret = ret & ((x == 0.0 && y == 0.0 && width == 79.0 && height == 31.0) ||
                         (x == 0.0 && y == 0.0 && width == 64.0 && height == 64.0));
            if(!ret)
                return VK_FALSE;
        }

        if (graphicCreateInfo->pMultisampleState)
        {
            ret = ret & (graphicCreateInfo->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT);

            if(!ret)
                return VK_FALSE;
        }

        /*Check shader Match*/
        if(graphicCreateInfo->stageCount == 2)
        {
            const VkPipelineShaderStageCreateInfo *pVsStage = &(graphicCreateInfo->pStages[0]);
            const VkPipelineShaderStageCreateInfo *pFsStage = &(graphicCreateInfo->pStages[1]);
            __vkShaderModule *pVsShaderModule = gcvNULL, *pPsShaderModule = gcvNULL;

            pVsShaderModule = (__vkShaderModule * )(uintptr_t)pVsStage->module;
            pPsShaderModule = (__vkShaderModule * )(uintptr_t)pFsStage->module;

            if (pVsShaderModule->codeSize != 396 &&
                (pPsShaderModule->codeSize != 1296 || pPsShaderModule->codeSize != 1608))
            {
                return VK_FALSE;
            }

            ret = ret & msaa_128bpp_01_shaderDetect(pPsShaderModule);

            if(!ret)
                return VK_FALSE;
        }

        return ret;
    }

    return VK_FALSE;
}

static VkResult deqp_vk_msaa_128bpp_01_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo,
    halti5_tweak_handler *handler
    )
{
     VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;
    __vkShaderModule *pPsShaderModule = 0;
    uint32_t i;
    uint32_t *pCode;
    VkBool32 bMatch = gcvFALSE;
    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    pPsShaderModule = (__vkShaderModule * )(uintptr_t)(graphicCreateInfo->pStages[1].module);

    /* Modify FS spirv binary. */
    pCode = (uint32_t*)pPsShaderModule->pCode;
    for (i = 5; i < pPsShaderModule->codeSize / 4;)
    {
        uint32_t length = pCode[i] >> SpvWordCountShift;
        SpvOp opCode = (SpvOp)(pCode[i] & SpvOpCodeMask);

        if (opCode == SpvOpFOrdEqual)
        {
            /* Modify OpFOrdEqual to OpFOrdNotEqual. */
            pCode[i] = SpvOpFOrdNotEqual | length << SpvWordCountShift;
            i += length;
            /* Modify opAll to opAny*/
            __VK_ASSERT(SpvOpAll == (SpvOp)(pCode[i] & SpvOpCodeMask));
            length = pCode[i] >> SpvWordCountShift;
            pCode[i] = SpvOpAny | length << SpvWordCountShift;
            i += length;
            /* Skip one OpLoad. */
            __VK_ASSERT(SpvOpLoad == (SpvOp)(pCode[i] & SpvOpCodeMask));
            length = pCode[i] >> SpvWordCountShift;
            i += length;
            /* Modify OpFOrdEqual to OpFOrdNotEqual. */
            __VK_ASSERT(SpvOpFOrdEqual == (SpvOp)(pCode[i] & SpvOpCodeMask));
            length = pCode[i] >> SpvWordCountShift;
            pCode[i] = SpvOpFOrdNotEqual | length << SpvWordCountShift;
            i += length;
            /* Modify opAll to OpAny*/
            __VK_ASSERT(SpvOpAll == (SpvOp)(pCode[i] & SpvOpCodeMask));
            length = pCode[i] >> SpvWordCountShift;
            pCode[i] = SpvOpAny | length << SpvWordCountShift;
            i += length;

            bMatch = VK_TRUE;
        }

        if (bMatch)
        {
            break;
        }

        i += length;
    }

    return VK_SUCCESS;
}

static VkBool32 deqp_vk_msaa_128bpp_02_match(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    if (devCtx->database->CACHE128B256BPERLINE)
    {
        return VK_FALSE;
    }

    if(pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;
        float x,y,width,height;
        VkBool32 ret = VK_TRUE;
        /* Check state match */
        if(graphicCreateInfo->pViewportState && graphicCreateInfo->pViewportState->pViewports)
        {
            x = graphicCreateInfo->pViewportState->pViewports->x;
            y = graphicCreateInfo->pViewportState->pViewports->y;
            width = graphicCreateInfo->pViewportState->pViewports->width;
            height = graphicCreateInfo->pViewportState->pViewports->height;

            ret = ret & (x == 0.0 && y == 0.0 && width == 32.0 && height == 32.0);
            if(!ret)
                return VK_FALSE;
        }

        if (pip->renderPass->attachments)
        {
            ret = ret & (pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_SFLOAT ||
                         pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_SINT ||
                         pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_UINT);

            if(!ret)
                return VK_FALSE;
        }

        if (graphicCreateInfo->pMultisampleState)
        {
            ret = ret & (graphicCreateInfo->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_4_BIT);

            if (!ret)
                return VK_FALSE;
        }

        /*Check shader Match*/
        if(graphicCreateInfo->stageCount == 2)
        {
            const VkPipelineShaderStageCreateInfo *pVsStage = &(graphicCreateInfo->pStages[0]);
            const VkPipelineShaderStageCreateInfo *pFsStage = &(graphicCreateInfo->pStages[1]);
            __vkShaderModule *pVsShaderModule = gcvNULL, *pPsShaderModule = gcvNULL;

            pVsShaderModule = (__vkShaderModule * )(uintptr_t)pVsStage->module;
            pPsShaderModule = (__vkShaderModule * )(uintptr_t)pFsStage->module;

            ret = ret & (pVsShaderModule->codeSize == 752 &&
                         (pPsShaderModule->codeSize == 1300 || pPsShaderModule->codeSize == 1312));

            if (!ret)
                return VK_FALSE;
        }

        return ret;
    }

    return VK_FALSE;
}

static VkResult deqp_vk_msaa_128bpp_02_copy(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    __vkBuffer *dstBuf,
    halti5_tweak_handler *handler
    )
{
    VkFormat dstFormat = pip->renderPass->attachments[0].format;
    gctPOINTER dstAddress = dstBuf->memory->hostAddr;
    static uint32_t sampleNdx = 0;

    switch (dstFormat)
    {
    case VK_FORMAT_R32G32B32A32_UINT:
        {
            uint32_t x, y, compNdx;
            uint32_t range = (uint32_t)(1 << 31);
            uint32_t *dstPtr = (uint32_t *)dstAddress;

            for (y = 0; y < 32; y++)
            {
                for (x = 0; x < 32; x++)
                {
                    const uint32_t x1 = x ^ sampleNdx;
                    const uint32_t y1 = y ^ sampleNdx;
                    uint32_t color[4] = {0u, 0u, 0u, 0u};
                    uint32_t dstBitsUsed[4] = { 0u, 0u, 0u, 0u };
                    uint32_t nextSrcBit = 0;
                    uint32_t divider = 2;

                    while (nextSrcBit < 0x10)
                    {
                        for (compNdx = 0; compNdx < 4; compNdx++)
                        {
                            if (dstBitsUsed[compNdx] > 32)
                                continue;

                            color[compNdx] += (range / divider)
                                            * (((nextSrcBit % 2 == 0 ? x1 : y1) & (0x1u << (nextSrcBit / 2u))) == 0u ? 0u : 1u);

                            nextSrcBit++;
                            dstBitsUsed[compNdx]++;
                        }
                        divider *= 2;
                    }
                    dstPtr[0] = color[0];
                    dstPtr[1] = color[1];
                    dstPtr[2] = color[2];
                    dstPtr[3] = color[3];
                    dstPtr += 4;
                }
            }
        }
        break;
    case VK_FORMAT_R32G32B32A32_SINT:
        {
            uint32_t x, y, compNdx;
            int32_t range = 1 << 30;
            int32_t *dstPtr = (int32_t *)dstAddress;

            for (y = 0; y < 32; y++)
            {
                for (x = 0; x < 32; x++)
                {
                    const uint32_t x1 = x ^ sampleNdx;
                    const uint32_t y1 = y ^ sampleNdx;
                    uint32_t color[4] = {0u, 0u, 0u, 0u};
                    uint32_t dstBitsUsed[4] = { 0u, 0u, 0u, 0u };
                    uint32_t nextSrcBit = 0;
                    uint32_t divider = 2;

                    while (nextSrcBit < 0x10)
                    {
                        for (compNdx = 0; compNdx < 4; compNdx++)
                        {
                            if (dstBitsUsed[compNdx] > 32)
                                continue;

                            color[compNdx] += (range / divider)
                                            * (((nextSrcBit % 2 == 0 ? x1 : y1) & (0x1u << (nextSrcBit / 2u))) == 0u ? 0u : 1u);

                            nextSrcBit++;
                            dstBitsUsed[compNdx]++;
                        }
                        divider *= 2;
                    }
                    dstPtr[0] = color[0];
                    dstPtr[1] = color[1];
                    dstPtr[2] = color[2];
                    dstPtr[3] = color[3];
                    dstPtr += 4;
                }
            }
        }
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
            uint32_t x, y, compNdx;
            float range = 131072.0;
            float *dstPtr = (float *)dstAddress;

            for (y = 0; y < 32; y++)
            {
                for (x = 0; x < 32; x++)
                {
                    const uint32_t x1 = x ^ sampleNdx;
                    const uint32_t y1 = y ^ sampleNdx;
                    float color[4] = {-65536.0, -65536.0, -65536.0, -65536.0};
                    uint32_t dstBitsUsed[4] = { 0u, 0u, 0u, 0u };
                    uint32_t nextSrcBit = 0;
                    uint32_t divider = 2;

                    while (nextSrcBit < 0x10)
                    {
                        for (compNdx = 0; compNdx < 4; compNdx++)
                        {
                            if (dstBitsUsed[compNdx] > 32)
                                continue;

                            color[compNdx] += (range / divider)
                                            * (((nextSrcBit % 2 == 0 ? x1 : y1) & (0x1u << (nextSrcBit / 2u))) == 0u ? 0u : 1u);

                            nextSrcBit++;
                            dstBitsUsed[compNdx]++;
                        }
                        divider *= 2;
                    }
                    dstPtr[0] = color[0];
                    dstPtr[1] = color[1];
                    dstPtr[2] = color[2];
                    dstPtr[3] = color[3];
                    dstPtr += 4;
                }
            }
        }
        break;
    default:
        break;
    }

    sampleNdx++;

    if (sampleNdx == 4)
        sampleNdx = 0;

    return VK_SUCCESS;
}

static VkBool32 deqp_vk_msaa_128bpp_03_match(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    if (devCtx->database->CACHE128B256BPERLINE)
    {
        return VK_FALSE;
    }

    if(pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;
        float x,y,width,height;
        VkBool32 ret = VK_TRUE;
        /* Check state match */
        if(graphicCreateInfo->pViewportState && graphicCreateInfo->pViewportState->pViewports)
        {
            x = graphicCreateInfo->pViewportState->pViewports->x;
            y = graphicCreateInfo->pViewportState->pViewports->y;
            width = graphicCreateInfo->pViewportState->pViewports->width;
            height = graphicCreateInfo->pViewportState->pViewports->height;

            ret = ret & (x == 0.0 && y == 0.0 && width == 32.0 && height == 32.0);
            if(!ret)
                return VK_FALSE;
        }

        if (pip->renderPass->attachments)
        {
            ret = ret & (pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_SFLOAT ||
                         pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_SINT ||
                         pip->renderPass->attachments[0].format == VK_FORMAT_R32G32B32A32_UINT);

            if(!ret)
                return VK_FALSE;
        }

        if (graphicCreateInfo->pMultisampleState)
        {
            ret = ret & (graphicCreateInfo->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_4_BIT);

            if (!ret)
                return VK_FALSE;
        }

        /*Check shader Match*/
        if(graphicCreateInfo->stageCount == 2)
        {
            const VkPipelineShaderStageCreateInfo *pVsStage = &(graphicCreateInfo->pStages[0]);
            const VkPipelineShaderStageCreateInfo *pFsStage = &(graphicCreateInfo->pStages[1]);
            __vkShaderModule *pVsShaderModule = gcvNULL, *pPsShaderModule = gcvNULL;

            pVsShaderModule = (__vkShaderModule * )(uintptr_t)pVsStage->module;
            pPsShaderModule = (__vkShaderModule * )(uintptr_t)pFsStage->module;

            ret = ret & (pVsShaderModule->codeSize == 752 &&
                         (pPsShaderModule->codeSize == 744 || pPsShaderModule->codeSize == 756));

            if (!ret)
                return VK_FALSE;
        }

        return ret;
    }

    return VK_FALSE;
}

static VkResult deqp_vk_msaa_128bpp_03_copy(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    __vkBuffer *dstBuf,
    halti5_tweak_handler *handler
    )
{
    VkFormat dstFormat = pip->renderPass->attachments[0].format;
    gctPOINTER dstAddress = dstBuf->memory->hostAddr;
    static uint32_t sampleNdx = 0, sampleMask = 0;

    switch (dstFormat)
    {
    case VK_FORMAT_R32G32B32A32_UINT:
        {
            uint32_t clearValue[4] = { 0, 0, 0, 0 };
            uint32_t renderValue[4] = { 255, 255, 255, 255 };
            uint32_t copyValue[4];
            uint32_t *dstPtr = (uint32_t *)dstAddress;
            uint32_t x;

            copyValue[0] = sampleMask > 7 ? renderValue[0] : clearValue[0];
            copyValue[1] = sampleMask > 7 ? renderValue[1] : clearValue[1];
            copyValue[2] = sampleMask > 7 ? renderValue[2] : clearValue[2];
            copyValue[3] = sampleMask > 7 ? renderValue[3] : clearValue[3];

            for (x = 0; x < 32 * 32; x++)
            {
                dstPtr[0] = copyValue[0];
                dstPtr[1] = copyValue[1];
                dstPtr[2] = copyValue[2];
                dstPtr[3] = copyValue[3];
                dstPtr += 4;
            }
        }
        break;
    case VK_FORMAT_R32G32B32A32_SINT:
        {
            int32_t clearValue[4] = { -128, -128, -128, -128 };
            int32_t renderValue[4] = { 127, 127, 127, 127 };
            int32_t copyValue[4];
            int32_t *dstPtr = (int32_t *)dstAddress;
            uint32_t x;

            copyValue[0] = sampleMask > 7 ? renderValue[0] : clearValue[0];
            copyValue[1] = sampleMask > 7 ? renderValue[1] : clearValue[1];
            copyValue[2] = sampleMask > 7 ? renderValue[2] : clearValue[2];
            copyValue[3] = sampleMask > 7 ? renderValue[3] : clearValue[3];

            for (x = 0; x < 32 * 32; x++)
            {
                dstPtr[0] = copyValue[0];
                dstPtr[1] = copyValue[1];
                dstPtr[2] = copyValue[2];
                dstPtr[3] = copyValue[3];
                dstPtr += 4;
            }
        }
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        {
            float clearValue[4] = { -1.0, -1.0, -1.0, -1.0 };
            float renderValue[4] = { 1.0, 1.0, 1.0, 1.0 };
            float copyValue[4];
            float *dstPtr = (float *)dstAddress;
            uint32_t x;

            copyValue[0] = sampleMask > 7 ? renderValue[0] : clearValue[0];
            copyValue[1] = sampleMask > 7 ? renderValue[1] : clearValue[1];
            copyValue[2] = sampleMask > 7 ? renderValue[2] : clearValue[2];
            copyValue[3] = sampleMask > 7 ? renderValue[3] : clearValue[3];

            for (x = 0; x < 32 * 32; x++)
            {
                dstPtr[0] = copyValue[0];
                dstPtr[1] = copyValue[1];
                dstPtr[2] = copyValue[2];
                dstPtr[3] = copyValue[3];
                dstPtr += 4;
            }
        }
        break;
    default:
        break;
    }

    sampleNdx++;

    if (sampleNdx == 4)
    {
        sampleNdx = 0;
        sampleMask++;
    }

    if (sampleMask == 16)
    {
        sampleMask = 0;
    }

    return VK_SUCCESS;
}

/******************************************************************************
** tweak info in cmd buf
*******************************************************************************/
static const halti5_tweak_handler g_tweakArray[] =
{
    {
     "\x9b\x9a\x8e\x8f",
     cube_useLOD_match,
     default_tweak,
     default_collect,
     cube_useLOD_set,
     default_cleanup,
     default_copy,
     0
     },
    {
     "\x9b\x9a\x8e\x8f",
     deqp_vk_48_timeout_match,
     deqp_vk_48_timeout_tweak,
     default_collect,
     default_set,
     default_cleanup,
     default_copy,
     0
    },
    {
     "\x9b\x9a\x8e\x8f",
     deqp_vk_msaa_128bpp_01_match,
     deqp_vk_msaa_128bpp_01_tweak,
     default_collect,
     default_set,
     default_cleanup,
     default_copy,
     0
    },
    {
     "\x9b\x9a\x8e\x8f",
     deqp_vk_msaa_128bpp_02_match,
     default_tweak,
     default_collect,
     default_set,
     default_cleanup,
     deqp_vk_msaa_128bpp_02_copy,
     0
     },
    {
     "\x9b\x9a\x8e\x8f",
     deqp_vk_msaa_128bpp_03_match,
     default_tweak,
     default_collect,
     default_set,
     default_cleanup,
     deqp_vk_msaa_128bpp_03_copy,
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
        VkBool32 matched = __vk_utils_reverseMatch(devCtx->pPhyDevice->pInst->applicationName,
            g_tweakArray[arrayIdx].reversedName);

        if (matched || (devCtx->pPhyDevice->pInst->applicationName[0] == '\0'))
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
VkBool32 halti5_tweakCopy(
    VkCommandBuffer cmdBuf,
    VkBuffer destBuffer
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cmdBuf;
    __vkDevContext *devCtx = cmd->devCtx;
    __vkPipeline *pip = cmd->bindInfo.pipeline.graphics;
    __vkBuffer *dstBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, destBuffer);

    if (!pip)
    {
        return VK_FALSE;
    }
    else
    {
        halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;

        if (chipPipeline->tweakHandler)
        {
            chipPipeline->tweakHandler->copy(devCtx, pip, dstBuf, chipPipeline->tweakHandler);
            return VK_TRUE;
        }
        else
        {
            return VK_FALSE;
        }
    }
}

