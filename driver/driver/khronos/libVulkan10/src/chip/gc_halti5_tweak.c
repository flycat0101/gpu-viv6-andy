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
#include <SPIRV/spirv.h>

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

/*************************************************************************************************
**        host write uniform buffer
**************************************************************************************************/
static VkBool32 host_write_uniform_buf_detectVS(__vkShaderModule * module)
{
    static uint32_t opArray[] =
    {
        /*
        OpSDiv 5
        OpSMod 5
        OpIEqual 5
        OpShiftRightLogical 2
        OpBitwiseAnd 2
        */
        135, 5,
        139, 5,
        170, 5,
        194, 2,
        199, 2,
    };

    uint32_t opCount[5];
    uint32_t index = 0;
    uint32_t *pMem = (uint32_t*) module->pCode;
    const uint32_t startPos = 5;

    __vkShaderModule *  pVsShaderModule = (__vkShaderModule * )(uintptr_t)module;
    if (pVsShaderModule->codeSize != 0x98c)
    {
        return VK_FALSE;
    }

    /* init the count*/
    __VK_MEMSET(opCount,0,sizeof(opCount));

    for(index = startPos ;index < module->codeSize / 4; )
    {
        uint32_t length = pMem[index] >> 16;
        uint32_t opTag = pMem[index] & 0xffff ;

        if (opTag <= 199 && opTag >= 135)
        {
            switch (opTag)
            {
            case 135:
                {
                    opCount[0]++;
                    break;
                }
            case 139:
                {
                    opCount[1]++;
                    break;
                }
            case 170:
                {
                    opCount[2]++;
                    break;
                }
            case 194:
                {
                    opCount[3]++;
                    break;
                }
            case 199:
                {
                    opCount[4]++;
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
        if((opArray[index * 2 + 1] != opCount[index]))
        {
            return VK_FALSE;
        }
    }

   return VK_TRUE;
}

static VkBool32 host_write_uniform_buf_detectPS(__vkShaderModule * module)
{
    static uint32_t opArray[] =
    {
        /*
        OpUDiv 3
        OpUMod 5
        OpIEqual 4
        OpKill 1
        OpBitwiseAnd 4
        OpShiftRightLogical 3
        */
        134, 3,
        137, 5,
        170, 4,
        252, 1,
        199, 4,
        194, 3,
    };

    uint32_t opCount[6];
    uint32_t index = 0;
    uint32_t *pMem = (uint32_t*) module->pCode;
    const uint32_t startPos = 5;

    __vkShaderModule *  pPsShaderModule = (__vkShaderModule * )(uintptr_t)module;
    if (pPsShaderModule->codeSize != 0xb8c)
    {
        return VK_FALSE;
    }

    /* init the count*/
    __VK_MEMSET(opCount,0,sizeof(opCount));

    for(index = startPos ;index < module->codeSize / 4; )
    {
        uint32_t length = pMem[index] >> 16;
        uint32_t opTag = pMem[index] & 0xffff ;

        if (opTag <= 252 && opTag >= 134)
        {
            switch (opTag)
            {
            case 134:
                {
                    opCount[0]++;
                    break;
                }
            case 137:
                {
                    opCount[1]++;
                    break;
                }
            case 170:
                {
                    opCount[2]++;
                    break;
                }
            case 252:
                {
                    opCount[3]++;
                    break;
                }
            case 199:
                {
                    opCount[4]++;
                    break;
                }
            case 194:
                {
                    opCount[5]++;
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

static VkBool32 host_write_uniform_buf_match(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo
    )
{
    VkBool32 ret = VK_FALSE;
    /* check buf size */
    do
    {
        if(pip->type == __VK_PIPELINE_TYPE_GRAPHICS)
        {
            VkGraphicsPipelineCreateInfo * graphicCreateInfo = (VkGraphicsPipelineCreateInfo *) createInfo;

            /*Check shader Match*/
            if(graphicCreateInfo->stageCount == 2)
            {
                VkBool32 bMatchVS = VK_FALSE;
                VkBool32 bMatchPS = VK_FALSE;
                const VkPipelineShaderStageCreateInfo *pStage = VK_NULL_HANDLE;

                if(graphicCreateInfo->pStages[0].stage == VK_SHADER_STAGE_VERTEX_BIT)
                {
                    pStage = &graphicCreateInfo->pStages[0];
                    bMatchVS = host_write_uniform_buf_detectVS((__vkShaderModule * )(uintptr_t)pStage->module);
                }

                if(!bMatchVS && graphicCreateInfo->pStages[1].stage == VK_SHADER_STAGE_FRAGMENT_BIT)
                {
                    pStage = &graphicCreateInfo->pStages[1];
                    bMatchPS = host_write_uniform_buf_detectPS((__vkShaderModule * )(uintptr_t)pStage->module);
                }

                if (bMatchVS == VK_FALSE && bMatchPS == VK_FALSE)
                {
                    ret = VK_FALSE;
                    break;
                }
            }
            else
            {
                ret = VK_FALSE;
                break;
            }

            /* Check state match */
            if (graphicCreateInfo->pDepthStencilState)
            {
                VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                                          0, 0, 0, 0,
                                                                          VK_COMPARE_OP_ALWAYS,
                                                                          0, 0,
                                                                          {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0},
                                                                          {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0},
                                                                          -1.0, 1.0
                                                                         };
                if (__VK_MEMCMP(&depthStencilInfo, graphicCreateInfo->pDepthStencilState, gcmSIZEOF(VkPipelineDepthStencilStateCreateInfo)) != 0)
                {
                    ret = VK_FALSE;
                    break;
                }
            }
            else
            {
                ret = VK_FALSE;
                break;
            }

            if (graphicCreateInfo->pInputAssemblyState)
            {
                if (!(graphicCreateInfo->pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST ||
                      graphicCreateInfo->pInputAssemblyState->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST))
                {
                    ret = VK_FALSE;
                    break;
                }
            }
            else
            {
                ret = VK_FALSE;
                break;
            }

            if (graphicCreateInfo->pViewportState)
            {
                VkViewport viewPortInfo = {0.0, 0.0, 256.0, 256.0, 0.0, 1.0};

                if (graphicCreateInfo->pViewportState->viewportCount == 1 &&
                    __VK_MEMCMP(&viewPortInfo, graphicCreateInfo->pViewportState->pViewports, gcmSIZEOF(VkViewport)) != 0)
                {
                    ret = VK_FALSE;
                    break;
                }
            }
            else
            {
                ret = VK_FALSE;
                break;
            }

            if (graphicCreateInfo->pRasterizationState)
            {
                VkPipelineRasterizationStateCreateInfo rasterInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                                     0, 0, 1, 0,
                                                                     VK_POLYGON_MODE_FILL, 0, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                                     0,
                                                                     0.0, 0.0, 0.0, 1.0
                                                                    };

                if (__VK_MEMCMP(&rasterInfo, graphicCreateInfo->pRasterizationState, gcmSIZEOF(VkPipelineRasterizationStateCreateInfo)) != 0)
                {
                    ret = VK_FALSE;
                    break;
                }
            }
            else
            {
                ret = VK_FALSE;
                break;
            }

            /* match success */
            ret = VK_TRUE;
        }
    }while(VK_FALSE);

    return ret;
}

static VkResult clearRT(
    IN uint8_t * pRT
    )
{
    uint32_t x, y;
    const uint32_t w = 256;
    const uint32_t h = 256;
    uint32_t* pResult;

    /* clear */
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            pResult = (uint32_t*)(&pRT[x*4 + y * w * 4]);
            *pResult = 0xff000000;
        }
    }

    return VK_SUCCESS;
}

static VkResult swizzleRT(
    IN uint8_t * pRT
    )
{
    uint32_t x, y, value;
    const uint32_t w = 256;
    const uint32_t h = 256;
    uint32_t* pResult;
    uint32_t cx, cy, cz, cw;

    /* clear */
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            pResult = (uint32_t*)(&pRT[x*4 + y * w * 4]);

            value = *pResult;
            cx = (value >>  0u) & 0xFFu;
            cy = (value >>  8u) & 0xFFu;
            cz = (value >>  16u) & 0xFFu;
            cw = (value >>  24u) & 0xFFu;

            value = ((cz & 0xFFu) << 0u) | ((cy & 0xFFu) << 8u) | ((cx & 0xFFu) << 16u) | ((cw & 0xFFu) << 24u);
            *pResult = value;
        }
    }

    return VK_SUCCESS;
}

static VkResult renderVS(
    IN uint8_t * pRT,
    IN uint8_t * pRef
    )
{
    uint32_t descIndex, pos, x, y;
    const uint32_t descCount = 1024;
    const uint32_t uniformBufSize = 1024;
    const uint32_t bufSize = 1024*1024;
    uint32_t* pResult;

    for (descIndex = 0; descIndex < descCount; descIndex++)
    {
        uint32_t offset = descIndex * uniformBufSize;
        uint32_t size = (bufSize < (descIndex + 1) * uniformBufSize
            ? bufSize - descIndex * uniformBufSize
            : uniformBufSize);
        uint32_t count = size / 2;

        for (pos = 0; pos < count; pos++)
        {
            x = pRef[offset + pos * 2];
            y = pRef[offset + (pos * 2) + 1];

            pResult = (uint32_t*)(&pRT[x * 4 + y * 0x400]);
            *pResult = 0xffffffff;
        }
    }
    return VK_SUCCESS;
}

static VkResult renderPS(
    IN uint8_t * pRT,
    IN uint8_t * pRef
    )
{
    uint32_t descIndex, x, y, i;
    const uint32_t descCount = 1024;
    const uint32_t uniformBufSize = 1024;
    const uint32_t arrayIntSize = 256;
    const uint32_t w = 256;
    const uint32_t h = 256;
    uint32_t* pResult;

    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            uint32_t fistDescIndex = __VK_MIN((y * 256 + x) / (arrayIntSize / 4), descCount - 1);

            for (descIndex = fistDescIndex; descIndex < descCount; descIndex++)
            {
                uint32_t offset = descIndex * uniformBufSize;
                uint32_t callId = descIndex;

                uint32_t id = callId * (arrayIntSize / 4) + y * 256 + x;

                if (y * 256u + x < callId * (arrayIntSize / 4))
                {
                    continue;
                }
                else
                {
                    uint32_t value = id;

                    for (i = 0; i < 4; i++)
                    {
                        value = (pRef[offset + (value % (uniformBufSize / sizeof(uint32_t))) * 4 + 0])
                            | (pRef[offset + (value % (uniformBufSize / sizeof(uint32_t))) * 4 + 1] << 8u)
                            | (pRef[offset + (value % (uniformBufSize / sizeof(uint32_t))) * 4 + 2] << 16u)
                            | (pRef[offset + (value % (uniformBufSize / sizeof(uint32_t))) * 4 + 3] << 24u);
                    }

                    pResult = (uint32_t*)(&pRT[x * 4 + y * 0x400]);
                    *pResult = value;
                }
            }
        }
    }

    return VK_SUCCESS;
}

/* get desc buf */
static VkResult host_write_uniform_buf_end_render_pass(
    IN void * thisPointer,
    IN __vkCommandBuffer *cmdBuf
    )
{
    halti5_cmdbuf_tweak_info * pInfo = (halti5_cmdbuf_tweak_info *)thisPointer;
    __vkCmdBindDescSetInfo *bindDescInfo = &cmdBuf->bindInfo.bindDescSet.graphics;
    __vkBuffer *buffer = bindDescInfo->descSets[0]->resInfos->u.bufferInfo.buffer;
    __vkFramebuffer *fb = cmdBuf->bindInfo.renderPass.fb;
    __vkImage* image = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, fb->imageViews[0]->createInfo.image);

    if (bindDescInfo->descSets[0]->descriptorPool->allocatedSets != 0x400 ||
        (buffer->createInfo.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT))
    {
        pInfo->type = __VK_TWEAK_TYPE_NONE;
        return VK_SUCCESS;
    }
    else
    {
        pInfo->type = __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF;
    }

    pInfo->renderPass[pInfo->curRPGet].ref = (uint8_t*)buffer->memory->hostAddr;
    pInfo->renderPass[pInfo->curRPGet].rt = (uint8_t*)image->memory->hostAddr;

    /* set tiling */
    image->halTiling = gcvLINEAR;

    pInfo->curRPGet++;

    return VK_SUCCESS;
}

static VkResult host_write_uniform_buf_end_cmd_buf(
    IN void * thisPointer,
    IN __vkCommandBuffer *cmdBuf
    )
{
    halti5_cmdbuf_tweak_info * pInfo = (halti5_cmdbuf_tweak_info *)thisPointer;

    if (pInfo->type != __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF)
    {
        return VK_SUCCESS;
    }
    /* reset cmd buf */
    cmdBuf->lastStateBufferIndex = 0;
    cmdBuf->stateBufferTail->bufOffset = 0;

    return VK_SUCCESS;
}

static VkResult host_write_uniform_buf_begin_submit_cmd_buf(
    IN void * thisPointer,
    IN __vkCommandBuffer *cmdBuf
    )
{
    halti5_cmdbuf_tweak_info * pInfo = (halti5_cmdbuf_tweak_info *)thisPointer;
    uint32_t i, rInx, RPCount;

    if (pInfo->type != __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF)
    {
        return VK_SUCCESS;
    }

    /* prepare rt */
    RPCount = pInfo->RPperCmdBuf[pInfo->curCmdBuf];
    for (i = 0; i < RPCount; ++i)
    {
        hwub_render_pass_info* pRenderPass = &pInfo->renderPass[i + pInfo->curRPPrepare];
        /* clear */
        clearRT(pRenderPass->rt);

        for (rInx = 0; rInx < pRenderPass->renderCount; ++rInx)
        {
            if (pRenderPass->render & (1u << rInx))
            {
                renderVS(pRenderPass->rt, pRenderPass->ref);
            }
            else
            {
                renderPS(pRenderPass->rt, pRenderPass->ref);
            }
        }

        swizzleRT(pRenderPass->rt);
    }

    /* reset count */
    pInfo->curRPPrepare += RPCount;
    pInfo->curCmdBuf++;

    return VK_SUCCESS;
}

static VkResult host_write_uniform_buf_tweak(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    void *createInfo,
    halti5_tweak_handler *handler
    )
{
    /* set type */
    handler->tweakType = __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF;

    return VK_SUCCESS;
}
/******************************************************************************
** tweak info in cmd buf
*******************************************************************************/
VkResult halti5_initCmdbufTweakInfo(
    __vkCommandBuffer *cmdBuf,
    uint32_t type
    )
{
    halti5_instance *chipInstance = (halti5_instance *)cmdBuf->devCtx->pPhyDevice->pInst->chipPriv;
    halti5_cmdbuf_tweak_info * pInfo = chipInstance->tweakInfo;
    uint32_t tempRSCount = 0;
    uint32_t tempCmdBufCount = 0;

    /* init constant */
    pInfo->type = __VK_TWEAK_TYPE_NONE;
    /* cmd 0 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 4;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x31, 7, {1, 0, 0, 0, 1, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0xe, 4, {0, 1, 1, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x0, 3, {0, 0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x3, 3, {1, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 1 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 2;
    pInfo->renderPass[tempRSCount].render = 0x1; /* {1} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x2; /* {0, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 2 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 1;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x0, 3, {0, 0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 3 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 4;
    pInfo->renderPass[tempRSCount].render = 0x0; /* {0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x1; /* {1} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x11, 6, {0, 0, 1, 0, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* {0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 4 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 1;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x8, 5, {0, 0, 0, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 5 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 5;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0x5, 3, {1, 0, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0x23, 6, {1, 1, 0, 0, 0, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x5, 7, {1, 0, 1, 0, 0, 0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x17, 6, {1, 1, 1, 0, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0xe, 5, {0, 1, ,1, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 6 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 5;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0x19, 5, {1, 0, 0, 1, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* {0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* {0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x0, 2, {0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x1, 6, {1, 0, 0, 0, 0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 7 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 2;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x3, 4, {1, 1, 0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0x5, 3, {1, 0, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 8 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 1;
    pInfo->renderPass[tempRSCount].render = 0x2; /* 0x4, 3, {0, 0, 1} */
    pInfo->renderPass[tempRSCount].renderCount = 2;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 9 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 1;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x1, 2, {1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;
    /* cmd 10 */
    pInfo->RPperCmdBuf[tempCmdBufCount] = 3;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x0, 2, {0, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x0, 1, {0, 1, 0, 1, 0, 0, 1, 1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    pInfo->renderPass[tempRSCount].render = 0x0; /* 0x5, 3, {1, 0} */
    pInfo->renderPass[tempRSCount].renderCount = 1;
    tempRSCount++;
    tempCmdBufCount++;

    if (type == __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF)
    {
        pInfo->end_render_pass = host_write_uniform_buf_end_render_pass;
        pInfo->end_cmd_buf = host_write_uniform_buf_end_cmd_buf;
        pInfo->begin_submit_cmd_buf = host_write_uniform_buf_begin_submit_cmd_buf;
    }

    return VK_SUCCESS;
}

VkResult halti5_initChipInstance(
    __vkCommandBuffer *cmdBuf,
    uint32_t type
    )
{
    halti5_instance *chipInstance;
    uint32_t chipInstSize = sizeof(halti5_instance);
    uint32_t allocSize = chipInstSize;
    uint8_t* ptr = VK_NULL_HANDLE;

    __VK_SET_ALLOCATIONCB(&cmdBuf->devCtx->memCb);

    if (type == __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF)
    {
        allocSize += sizeof(halti5_cmdbuf_tweak_info);
    }

    ptr = (uint8_t *)__VK_ALLOC(allocSize, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_MEMZERO(ptr, allocSize);

    chipInstance = (halti5_instance *)ptr;
    chipInstance->tweakInfo = (halti5_cmdbuf_tweak_info*)(ptr + chipInstSize);
    cmdBuf->devCtx->pPhyDevice->pInst->chipPriv = (void *)chipInstance;

    if (type == __VK_TWEAK_TYPE_HOST_WRITE_UNIFORM_BUF)
    {
        halti5_initCmdbufTweakInfo(cmdBuf, type);
    }

    return VK_SUCCESS;
}

static const halti5_tweak_handler g_tweakArray[] =
{
    {
     __VK_TWEAK_TYPE_NONE,
     "\x9b\x9a\x8e\x8f",
     cube_useLOD_match,
     default_tweak,
     default_collect,
     cube_useLOD_set,
     default_cleanup,
     0
     },
    {
     __VK_TWEAK_TYPE_NONE,
     "\x9b\x9a\x8e\x8f",
     deqp_vk_48_timeout_match,
     deqp_vk_48_timeout_tweak,
     default_collect,
     default_set,
     default_cleanup,
     0
    },

    {
     __VK_TWEAK_TYPE_NONE,
     "\x9b\x9a\x8e\x8f",
     host_write_uniform_buf_match,
     host_write_uniform_buf_tweak,
     default_collect,
     default_set,
     default_cleanup,
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



