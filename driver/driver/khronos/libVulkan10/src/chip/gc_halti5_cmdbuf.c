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

#define __VK_SCRATCH_BUFFER_SIZE        32

#if (defined(DEBUG)||defined(_DEBUG))
static VkBool32 g_dbgForceCacheFlushAndStallPerDraw = VK_FALSE;
static VkBool32 g_dbgSkipDraw = VK_FALSE;
#endif

/*
** Handle pipeline switch between compute and graphics
*/
static void halti5_pipeline_switch(
    __vkCommandBuffer *cmdBuf
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;

    /* switch to compute shader */
    if (cmdBuf->bindInfo.pipeline.activePipeline ==  VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        /*(Todo) reconfig USC */
    }
    /* switch to graphics shader */
    else if (cmdBuf->bindInfo.pipeline.activePipeline == VK_PIPELINE_BIND_POINT_COMPUTE)
    {

    }
    /* if previous compute pipeline has barrier, need stall draw at FE point */
    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

    *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&pCmdBuffer)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;


    cmdBuf->curScrachBufIndex += (uint32_t)(uintptr_t)(pCmdBuffer - pCmdBufferBegin);

    return;
}


/*
** Handle switch between graphics pipelines.
*/
static void halti5_gfxpipeline_switch(
    __vkCommandBuffer *cmdBuf
    )
{
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    VkBool32 flushColorcache = VK_FALSE, flushZcache = VK_FALSE;
    VkBool32 stallRAatDraw = VK_FALSE;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkBool32 switchCacheMode = VK_FALSE;
    VkBool32 feStallState = VK_FALSE, raStallState = VK_FALSE;

    if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_CACHEMODE_DIRTY)
    {
        flushColorcache = VK_TRUE;
        flushZcache = VK_TRUE;
        switchCacheMode = VK_TRUE;
        raStallState = VK_TRUE;
    }
    else if ((chipCommand->gfxPipelineSwitchDirtyMask & (HALTI5_GFXPIPELINE_SWITCH_EARLYZ_DIRTY
                                                       | HALTI5_GFXPIPELINE_SWITCH_DEPTH_COMPAREOP_DIRTY))
            && chipGfxPipeline->earlyZ)
    {
        flushZcache = VK_TRUE;
        stallRAatDraw = VK_TRUE;
    }
    else if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_PE_DEPTH_DIRTY)
    {
        flushZcache = VK_TRUE;
        stallRAatDraw = VK_TRUE;
    }
    else if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_STENCIL_MODE_DIRTY)
    {
        flushZcache = VK_TRUE;
    }

    if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_SINGLE_PE_DIRTY)
    {
        flushZcache = VK_TRUE;
        flushColorcache = VK_TRUE;
    }
    else if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_DESTINATION_READ_DIRTY)
    {
        flushColorcache = VK_TRUE;
    }

    if (chipCommand->gfxPipelineSwitchDirtyMask & HALTI5_GFXPIPELINE_SWITCH_UNIFIED_RESOURCE_DIRTY)
    {
        feStallState = VK_TRUE;
    }

    if (flushColorcache || flushZcache || stallRAatDraw || feStallState || raStallState)
    {
        pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

        if (flushZcache || flushColorcache)
        {
            uint32_t flushState = flushColorcache ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) : 0;
            flushState |= flushZcache ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) : 0;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E03, VK_FALSE, flushState);
        }

        if (feStallState)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));
            *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&pCmdBuffer)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;

        }
        else if (raStallState || stallRAatDraw)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));
            __VK_STALL_RA(&pCmdBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));
        }

        if (switchCacheMode)
        {
            uint32_t cacheMode = (chipGfxPipeline->hwCacheMode == CHIP_CACHEMODE_256) ? 1 : 0;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, gcvFALSE,
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 24:24) - (0 ?
 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 24:24) - (0 ?
 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (cacheMode) & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))) &
                (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) (cacheMode) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27)))));
        }
        cmdBuf->curScrachBufIndex += (uint32_t)(uintptr_t)(pCmdBuffer - pCmdBufferBegin);
    }

    chipCommand->gfxPipelineSwitchDirtyMask = 0;

#if (defined(DEBUG)||defined(_DEBUG))
    if (g_dbgForceCacheFlushAndStallPerDraw)
    {
        chipCommand->gfxPipelineSwitchDirtyMask = HALTI5_GFXPIPELINE_SWITCH_ALL_DIRTY;
    }
#endif

    return;
}

static VkResult halti5_flushRsViewFirstUse(
    __vkCommandBuffer *cmdBuf
    )
{
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer*)cmdBuf->chipPriv;
    HwResourceViewUsage newRsViewMask = chipCommandBuffer->newResourceViewUsageMask;
    uint32_t index = 0;
    HwCacheMask cacheMask = HW_CACHE_NONE;
    static const HwCacheMask s_rsViewUsageToCacheMask[] =
        {
            HW_CACHE_TEXTURE_DATA | HW_CACHE_VST_DATA, /* HW_RESOURCEVIEW_USAGE_TX */
            HW_CACHE_SH_L1, /* HW_RESOURCEVIEW_USAGE_SH */
            HW_CACHE_DEPTH, /* HW_RESOURCEVIEW_USAGE_DEPTH */
            HW_CACHE_COLOR                             /* HW_RESOURCEVIEW_USAGE_COLOR */
        };
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result = VK_SUCCESS;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    __VK_ASSERT(newRsViewMask);

    while (newRsViewMask)
    {
        if (newRsViewMask & (1 << index))
        {
            newRsViewMask &= ~(1 << index);
            cacheMask |= s_rsViewUsageToCacheMask[index];
        }
        index++;
    }

    __VK_ASSERT(cacheMask);

    result = halti5_flushCache((VkDevice)cmdBuf->devCtx, &pCmdBuffer, VK_NULL_HANDLE, cacheMask);

    cmdBuf->curScrachBufIndex += (uint32_t)(uintptr_t)(pCmdBuffer - pCmdBufferBegin);

    chipCommandBuffer->newResourceViewUsageMask = 0;

    return result;

}

static VkResult halti5_draw_validate(
    __vkCommandBuffer *cmdBuf,
    __vkDrawComputeCmdParams *cmdParams
    )
{
    __vkCmdBindInfo *bindInfo = &cmdBuf->bindInfo;

    VkResult result = VK_SUCCESS;

#if __VK_RESOURCE_INFO
    static int32_t tgtCmdBufferID = 0;
    static uint32_t tgtSequenceID = 0;

    if ((tgtCmdBufferID == cmdBuf->obj.id)
    && (tgtSequenceID == cmdBuf->sequenceID))
    {
       /* make compiler happy */
       uint32_t tmpID = tgtSequenceID;
       tgtSequenceID = tmpID;
    }
#endif

    do
    {
        VkBool32 gfxPipelineDirty = (bindInfo->pipeline.dirty & __VK_CMDBUF_BINDNIGINFO_PIPELINE_GRAPHICS_DIRTY);
        __vkPipeline *pip = bindInfo->pipeline.graphics;
        __vkCmdBindDescSetInfo *descSetInfo = &bindInfo->bindDescSet.graphics;
        VkBool32 vscprogramInstanceSwitched = VK_FALSE;
        halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
        halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;
        uint32_t dynamicStateDirty = (bindInfo->dynamicStates.dirtyMask & pip->dynamicStates);
        __vkDevContext *devCtx = cmdBuf->devCtx;

        if (chipPipeline->tweakHandler)
        {
            (*chipPipeline->tweakHandler->set)(cmdBuf, cmdParams, chipPipeline->tweakHandler);
        }

        /* patch pipeline */
        if ((gfxPipelineDirty || descSetInfo->dirtyMask) && (!chipPipeline->vanilla))
        {
            __VK_ERR_BREAK(halti5_patch_pipeline(pip, descSetInfo, &vscprogramInstanceSwitched));
        }

        if (cmdBuf->bindInfo.pipeline.activePipeline != VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            halti5_pipeline_switch(cmdBuf);
            cmdBuf->bindInfo.pipeline.activePipeline = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }

        if (chipCommandBuffer->gfxPipelineSwitchDirtyMask)
        {
            halti5_gfxpipeline_switch(cmdBuf);
        }

        if (gfxPipelineDirty || bindInfo->renderPass.dirty)
        {
            dynamicStateDirty |= __VK_DYNAMIC_STATE_SCISSOR_BIT;
        }

        /* dirty descSet when pipeline switch, we don't have pipeline compatibility now !!! */
        if (gfxPipelineDirty || vscprogramInstanceSwitched)
        {
            descSetInfo->dirtyMask = ~(~0 << chipPipeline->masterInstance->pep.u.vk.resourceSetCount);
        }

        if (vscprogramInstanceSwitched || gfxPipelineDirty)
        {
            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                chipPipeline->curInstance->hwStates.pStateBuffer, chipPipeline->curInstance->hwStates.stateBufferSize);
            cmdBuf->curScrachBufIndex += chipPipeline->curInstance->hwStates.stateBufferSize / sizeof(uint32_t);

            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                chipPipeline->curInstance->instanceCmd, chipPipeline->curInstance->curInstanceCmdIndex * sizeof(uint32_t));
            cmdBuf->curScrachBufIndex += chipPipeline->curInstance->curInstanceCmdIndex;
        }

        if (gfxPipelineDirty)
        {
            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                chipPipeline->cmdBuffer, chipPipeline->curCmdIndex * sizeof(uint32_t));
            cmdBuf->curScrachBufIndex += chipPipeline->curCmdIndex;
            bindInfo->pipeline.dirty &= ~__VK_CMDBUF_BINDNIGINFO_PIPELINE_GRAPHICS_DIRTY;

            __VK_ERR_BREAK(halti5_setPsOutputMode(cmdBuf, pip));
        }

        if (bindInfo->indexBuffer.dirty)
        {
            __VK_ERR_BREAK(halti5_setIndexBuffer(cmdBuf));
            bindInfo->indexBuffer.dirty = VK_FALSE;
        }

        if (bindInfo->vertexBuffers.dirtyBits)
        {
            __VK_ERR_BREAK(halti5_setVertexBuffers(cmdBuf));
            bindInfo->vertexBuffers.dirtyBits = 0;
        }

        if (dynamicStateDirty)
        {
            if (dynamicStateDirty & __VK_DYNAMIC_STATE_VIEWPORT_BIT)
            {
                __VK_ERR_BREAK(halti5_setViewport(cmdBuf));
            }

            if (dynamicStateDirty & __VK_DYNAMIC_STATE_SCISSOR_BIT)
            {
                __VK_ERR_BREAK(halti5_setScissor(cmdBuf));
            }

            if (dynamicStateDirty & __VK_DYNAMIC_STATE_STENCIL_BITS)
            {
                __VK_ERR_BREAK(halti5_setStencilStates(cmdBuf));
            }

            if (dynamicStateDirty & __VK_DYNAMIC_STATE_DEPTH_BIAS_BIT)
            {
                __VK_ERR_BREAK(halti5_setDepthBias(cmdBuf));
            }

            if (dynamicStateDirty & __VK_DYNAMIC_STATE_BLEND_CONSTANTS_BIT)
            {
               __VK_ERR_BREAK(halti5_setBlendConstants(cmdBuf));
            }

            if (dynamicStateDirty & __VK_DYNAMIC_STATE_LINE_WIDTH_BIT)
            {
                __VK_ERR_BREAK(halti5_setLineWidth(cmdBuf));
            }

            bindInfo->dynamicStates.dirtyMask = 0;
        }

        if (bindInfo->renderPass.dirty)
        {
            __VK_ERR_BREAK(halti5_setRenderTargets(cmdBuf));
            bindInfo->renderPass.dirty = VK_FALSE;
        }

        if (descSetInfo->dirtyMask)
        {
            __VK_ERR_BREAK(halti5_setDesriptorSets(cmdBuf, pip, descSetInfo));
#if __VK_ENABLETS
            __VK_ERR_BREAK(halti5_setTxTileStatus(cmdBuf, descSetInfo));
#endif
            descSetInfo->dirtyMask = 0;
        }

        if (bindInfo->pushConstants.dirtyMask)
        {
            __VK_ERR_BREAK(halti5_setPushConstants(cmdBuf, pip));
            bindInfo->pushConstants.dirtyMask = 0;
        }

        if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        {
            __VK_ERR_BREAK(halti5_setMultiGPURenderingMode(cmdBuf,pip));
        }

        if (devCtx->database->DRAWID)
        {
            __VK_ERR_BREAK(halti5_setDrawID(cmdBuf, pip));
        }

        if (chipCommandBuffer->newResourceViewUsageMask)
        {
            __VK_ERR_BREAK(halti5_flushRsViewFirstUse(cmdBuf));
        }

    } while(VK_FALSE);

    cmdBuf->sequenceID++;

    return result;
}

VkResult halti5_draw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t drawCommand, drawCount;
    uint32_t *states;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    VkBool32 useOneCore = VK_FALSE;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    static const uint32_t s_xlatePrimitiveTopology[] =
    {
        /* VK_PRIMITIVE_TOPOLOGY_POINT_LIST */
        0x1,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST */
        0x2,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP */
        0x3,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST */
        0x4,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP */
        0x5,
        /*  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN */
        0x6,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY */
        0x9,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY */
        0xA,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY */
        0xB,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY */
        0xC,
        /* VK_PRIMITIVE_TOPOLOGY_PATCH_LIST */
        0xD,
    };

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (firstInstance != cmdBuf->bindInfo.vertexBuffers.firstInstance)
    {
        cmdBuf->bindInfo.vertexBuffers.firstInstance = firstInstance;
        cmdBuf->bindInfo.vertexBuffers.dirtyBits |= chipGfxPipeline->instancedVertexBindingMask;
    }

    if (vertexCount == 0)
    {
        return VK_SUCCESS;
    }

    if (chipGfxPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.draw.indexDraw = VK_FALSE;
        cmdParams.draw.indirectDraw = VK_FALSE;
        cmdParams.draw.firstVertex = firstVertex;
        cmdParams.draw.vertexCount = vertexCount;
        cmdParams.draw.firstInstance = firstInstance;
        cmdParams.draw.instanceCount = instanceCount;
    }

    __VK_ONERROR(halti5_draw_validate(cmdBuf, &cmdParams));

    if (cmdBuf->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = VK_TRUE;
    }

    /* Determine draw command. */
    drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (instanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (s_xlatePrimitiveTopology[pip->topology]) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));

    drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:0) - (0 ?
 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) (vertexCount) & ((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ? 23:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((instanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
        {
            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&pCmdBuffer)++ = 0;
;

        }
    }

    if (chipGfxPipeline->baseInstance.bUsed)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->baseInstance.hwRegAddress, VK_FALSE, firstInstance);
    }

    __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
    *pCmdBuffer++ = drawCommand;
    *pCmdBuffer++ = drawCount;
    *pCmdBuffer++ = firstVertex;
    *pCmdBuffer++ = 0;
    __VK_DEBUG_ONLY(});

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
        }
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}

VkResult halti5_drawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t drawCommand, drawCount;
    uint32_t *states;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    VkBool32 useOneCore = VK_FALSE;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    static const uint32_t s_xlatePrimitiveTopology[] =
    {
        /* VK_PRIMITIVE_TOPOLOGY_POINT_LIST */
        0x1,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST */
        0x2,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP */
        0x3,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST */
        0x4,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP */
        0x5,
        /*  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN */
        0x6,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY */
        0x9,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY */
        0xA,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY */
        0xB,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY */
        0xC,
        /* VK_PRIMITIVE_TOPOLOGY_PATCH_LIST */
        0xD,
    };

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (firstIndex != cmdBuf->bindInfo.indexBuffer.firstIndex)
    {
        cmdBuf->bindInfo.indexBuffer.firstIndex = firstIndex;
        cmdBuf->bindInfo.indexBuffer.dirty = VK_TRUE;
    }

    if (firstInstance != cmdBuf->bindInfo.vertexBuffers.firstInstance)
    {
        cmdBuf->bindInfo.vertexBuffers.firstInstance = firstInstance;
        cmdBuf->bindInfo.vertexBuffers.dirtyBits |= chipGfxPipeline->instancedVertexBindingMask;
    }

    if (chipGfxPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.draw.indexDraw = VK_TRUE;
        cmdParams.draw.indirectDraw = VK_FALSE;
        cmdParams.draw.firstIndex = firstIndex;
        cmdParams.draw.indexCount = indexCount;
        cmdParams.draw.firstInstance = firstInstance;
        cmdParams.draw.instanceCount = instanceCount;
        cmdParams.draw.firstVertex = vertexOffset;
    }

    __VK_ONERROR(halti5_draw_validate(cmdBuf, &cmdParams));

    if (cmdBuf->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = VK_TRUE;
    }

    /* Determine draw command. */
    drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (instanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (s_xlatePrimitiveTopology[pip->topology]) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));

    drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:0) - (0 ?
 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) (indexCount) & ((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ? 23:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((instanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
        {
            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&pCmdBuffer)++ = 0;
;

        }
    }

    if (chipGfxPipeline->baseInstance.bUsed)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->baseInstance.hwRegAddress, VK_FALSE, firstInstance);
    }
    __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
    *pCmdBuffer++ = drawCommand;
    *pCmdBuffer++ = drawCount;
    *pCmdBuffer++ = vertexOffset;
    *pCmdBuffer++ = 0;
    __VK_DEBUG_ONLY(});

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
        }
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}

__VK_INLINE VkResult halti5_setIndexBufferCmd(
    __vkCommandBuffer *cmdBuf,
    uint32_t **pCmdBuffer
    )
{
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;

    __vkDevContext *devCtx = cmdBuf->devCtx;
    const gcsFEATURE_DATABASE *database = devCtx->pPhyDevice->phyDevConfig.database;

    static const uint32_t xlateIndexType[] =
    {
        /* VK_INDEX_TYPE_UINT16*/
        0x1,
        /* VK_INDEX_TYPE_UINT32 */
        0x2,
    };

    static const uint32_t xlatePRindex[] =
    {
        /* VK_INDEX_TYPE_UINT16*/
        0xFFFF,
         /* VK_INDEX_TYPE_UINT32 */
        0xFFFFFFFF,
    };

    uint32_t srcAddr;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->bindInfo.indexBuffer.buffer);

    if (pip->patchControlPoints != 0 && buf->splitMemory)
    {
        srcAddr = buf->splitMemory->devAddr;
        srcAddr += (uint32_t)buf->splitMemOffset;
    }
    else
    {
        srcAddr = buf->memory->devAddr;
        srcAddr += (uint32_t)(buf->memOffset + cmdBuf->bindInfo.indexBuffer.offset);
    }

    if (cmdBuf->bindInfo.indexBuffer.firstIndex)
    {
        srcAddr += cmdBuf->bindInfo.indexBuffer.firstIndex
            * ((cmdBuf->bindInfo.indexBuffer.indexType == VK_INDEX_TYPE_UINT16) ? 2 : 4);
    }

    __vkCmdLoadSingleHWState(pCmdBuffer, 0x0191, VK_FALSE, srcAddr);
    __vkCmdLoadSingleHWState(pCmdBuffer, 0x0192, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (xlateIndexType[cmdBuf->bindInfo.indexBuffer.indexType]) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
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
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (pip->primitiveRestartEnable) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))));

    if (pip->primitiveRestartEnable)
    {
        __vkCmdLoadSingleHWState(pCmdBuffer, 0x019D, VK_FALSE,
            xlatePRindex[cmdBuf->bindInfo.indexBuffer.indexType]);
    }

    if (database->ROBUSTNESS)
    {
        uint32_t endAddress = srcAddr + (uint32_t)buf->memReq.size - 1;
        __vkCmdLoadSingleHWState(pCmdBuffer, 0x01FE, VK_FALSE, endAddress);
    }

    return VK_SUCCESS;
}

/*********************************************************
**              split draw func
**********************************************************/
#define __VK_COMPUTE_SPLIT_PARAMTER(cmdBuf, pip, indexCount) \
    splitParamter.indexSize = ((cmdBuf->bindInfo.indexBuffer.indexType == VK_INDEX_TYPE_UINT16) ? 2 : 4); \
    splitParamter.indexBufferSize = (indexCount) * splitParamter.indexSize; \
    splitParamter.bytesPerPatch = pip->patchControlPoints * splitParamter.indexSize; \
    splitParamter.alignBytes = 64; \
    splitParamter.splitBytesPerPatch = splitParamter.bytesPerPatch - splitParamter.indexSize

__VK_INLINE VkResult halti5_allocSplitMemory(
    __vkCommandBuffer *cmdBuf,
    __vkBuffer * buf,
    __vkSplitPathListParams* pParameter
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t copyOffset = 0;
    uint32_t copyLen = 0;
    uint32_t splitMemSize = 0;
    uint32_t i = 0;
    VkMemoryAllocateInfo mem_alloc;
    __vkDevContext *devCtx = cmdBuf->devCtx;

    if (buf->splitMemory)
    {
        /* already exist, just update offset.*/
        buf->splitMemOffset = __VK_ALIGN(buf->splitMemory->devAddr, pParameter->alignBytes) - buf->splitMemory->devAddr;
    }
    else
    {
        /* allocate split memory.*/
        /* compute split memory size.*/
        do
        {
            VkBool32 doCopy = VK_FALSE;

            if (copyOffset >= pParameter->indexBufferSize)
            {
                /* done.*/
                break;
            }

            /* compute copyLen.*/
            for (i = 1; (pParameter->alignBytes * i) < pParameter->indexBufferSize - copyOffset; ++i)
            {
                /* only one index beyond 64 bytes.*/
                if (((pParameter->alignBytes * i) % pParameter->bytesPerPatch) == pParameter->splitBytesPerPatch)
                {
                    doCopy = gcvTRUE;
                    break;
                }
            }

            if (doCopy)
            {
                copyLen = pParameter->alignBytes * i - pParameter->splitBytesPerPatch;
            }
            else
            {
                copyLen = pParameter->indexBufferSize - copyOffset;
            }

            /* No split draw for this time.*/
            if (copyLen == 0)
            {
                continue;
            }

            /* Compute split Memory size.*/
            splitMemSize += __VK_ALIGN(copyLen, pParameter->alignBytes);

            /* Update offset.*/
            copyOffset += copyLen;
        }
        while(copyOffset < pParameter->indexBufferSize);

        /* allocate split memory.*/
        __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.allocationSize = splitMemSize;
        mem_alloc.memoryTypeIndex = 0;
        __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &mem_alloc, gcvNULL, (VkDeviceMemory *)&buf->splitMemory));
        buf->splitMemOffset = __VK_ALIGN(buf->splitMemory->devAddr, pParameter->alignBytes) - buf->splitMemory->devAddr;
    }

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    return result;
}

__VK_INLINE VkResult halti5_setSplitDrawCmd(
    __vkCommandBuffer *cmdBuf,
    uint32_t **pCmdBuffer,
    __vkBuffer * buf,
    __vkSplitPathListParams* pParameter,
    uint32_t instanceCount,
    int32_t vertexOffset
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t copyOffset = 0;
    uint32_t copyLen = 0;
    uint32_t i = 0;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    uint32_t drawCommand, drawCount;

    static const uint32_t s_xlatePrimitiveTopology[] =
    {
        /* VK_PRIMITIVE_TOPOLOGY_POINT_LIST */
        0x1,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST */
        0x2,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP */
        0x3,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST */
        0x4,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP */
        0x5,
        /*  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN */
        0x6,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY */
        0x9,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY */
        0xA,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY */
        0xB,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY */
        0xC,
        /* VK_PRIMITIVE_TOPOLOGY_PATCH_LIST */
        0xD,
    };

    do
    {
        VkBool32 doCopy = VK_FALSE;

        if (copyOffset >= pParameter->indexBufferSize)
        {
            /* done.*/
            break;
        }

        /* compute copyLen.*/
        for (i = 1; (pParameter->alignBytes * i) < pParameter->indexBufferSize - copyOffset; ++i)
        {
            /* only one index beyond 64 bytes.*/
            if (((pParameter->alignBytes * i) % pParameter->bytesPerPatch) == pParameter->splitBytesPerPatch)
            {
                doCopy = VK_TRUE;
                break;
            }
        }

        if (doCopy)
        {
            copyLen = pParameter->alignBytes * i - pParameter->splitBytesPerPatch;
        }
        else
        {
            copyLen = pParameter->indexBufferSize - copyOffset;
        }

        /* No split draw for this time.*/
        if (copyLen == 0)
        {
            continue;
        }

        __VK_MEMCOPY(__VK_PTR2SIZE(buf->splitMemory->hostAddr) + (gctSIZE_T)buf->splitMemOffset,
                     __VK_PTR2SIZE(buf->memory->hostAddr) + (gctSIZE_T)(buf->memOffset + cmdBuf->bindInfo.indexBuffer.offset + copyOffset),
                     copyLen);

        __VK_ONERROR(halti5_setIndexBufferCmd(cmdBuf, pCmdBuffer));

        /* Update offset.*/
        copyOffset += copyLen;
        buf->splitMemOffset += __VK_ALIGN(copyLen, pParameter->alignBytes);

        /* draw command.*/
        /* Determine draw command. */
        drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0C & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (instanceCount) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (s_xlatePrimitiveTopology[pip->topology]) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)));

        drawCount = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:0) - (0 ?
 23:0) + 1))))))) << (0 ?
 23:0))) | (((gctUINT32) ((gctUINT32) ((copyLen / pParameter->indexSize)) & ((gctUINT32) ((((1 ?
 23:0) - (0 ?
 23:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:0) - (0 ? 23:0) + 1))))))) << (0 ? 23:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:24) - (0 ?
 31:24) + 1))))))) << (0 ?
 31:24))) | (((gctUINT32) ((gctUINT32) ((instanceCount >> 16)) & ((gctUINT32) ((((1 ?
 31:24) - (0 ?
 31:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:24) - (0 ? 31:24) + 1))))))) << (0 ? 31:24)));

        __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
        *((*pCmdBuffer)++) = drawCommand;
        *((*pCmdBuffer)++) = drawCount;
        *((*pCmdBuffer)++) = vertexOffset;
        *((*pCmdBuffer)++) = 0;
        __VK_DEBUG_ONLY(});
    }
    while(copyOffset < pParameter->indexBufferSize);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    return result;
}

VkResult halti5_splitDrawIndexedPatchList(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t *states;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    VkBool32 useOneCore = VK_FALSE;
    __vkDevContext *devCtx = cmdBuf->devCtx;

    /* Split draw variable.*/
    __vkSplitPathListParams splitParamter;
    __vkBuffer *buf = VK_NULL_HANDLE;

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (firstIndex != cmdBuf->bindInfo.indexBuffer.firstIndex)
    {
        cmdBuf->bindInfo.indexBuffer.firstIndex = firstIndex;
        cmdBuf->bindInfo.indexBuffer.dirty = VK_TRUE;
    }

    if (firstInstance != cmdBuf->bindInfo.vertexBuffers.firstInstance)
    {
        cmdBuf->bindInfo.vertexBuffers.firstInstance = firstInstance;
        cmdBuf->bindInfo.vertexBuffers.dirtyBits |= chipGfxPipeline->instancedVertexBindingMask;
    }

    if (chipGfxPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.draw.indexDraw = VK_TRUE;
        cmdParams.draw.indirectDraw = VK_FALSE;
        cmdParams.draw.firstIndex = firstIndex;
        cmdParams.draw.indexCount = indexCount;
        cmdParams.draw.firstInstance = firstInstance;
        cmdParams.draw.instanceCount = instanceCount;
        cmdParams.draw.firstVertex = vertexOffset;
    }

    __VK_ONERROR(halti5_draw_validate(cmdBuf, &cmdParams));

    if (cmdBuf->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = VK_TRUE;
    }

    buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->bindInfo.indexBuffer.buffer);

    /* split draw command.*/
    __VK_COMPUTE_SPLIT_PARAMTER(cmdBuf, pip, indexCount);
    __VK_ONERROR(halti5_allocSplitMemory(cmdBuf, buf, &splitParamter));

    /* prepare cmd buf.*/
    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
        {
            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&pCmdBuffer)++ = 0;
;

        }
    }

    if (chipGfxPipeline->baseInstance.bUsed)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->baseInstance.hwRegAddress, VK_FALSE, firstInstance);
    }

    /* set split draw cmd.*/
    __VK_ONERROR(halti5_setSplitDrawCmd(cmdBuf, &pCmdBuffer, buf, &splitParamter, instanceCount, vertexOffset));

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
        }
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}

/* Whether has invalid index when the bug exist.*/
static VkBool32 halti5_needSplitPatchList(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount
    )
{
    __vkSplitPathListParams splitParamter;
    uint32_t i = 0;
    uint32_t baseOffset = 0;

    uint32_t srcAddr;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->bindInfo.indexBuffer.buffer);
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;

    /* Get physical address.*/
    __VK_COMPUTE_SPLIT_PARAMTER(cmdBuf, pip, indexCount);
    srcAddr = buf->memory->devAddr;
    srcAddr += (uint32_t)(buf->memOffset + cmdBuf->bindInfo.indexBuffer.offset);

    baseOffset = __VK_ALIGN(srcAddr, splitParamter.alignBytes) - srcAddr;

    if (splitParamter.splitBytesPerPatch == splitParamter.alignBytes || (baseOffset == 0 && pip->patchControlPoints % 2 == 0))
    {
        /* splitBytesPerPatch == alignBytes is corner case, can not handle.*/
        return VK_FALSE;
    }

    /* check invalid index.*/
    for (i = 0; (splitParamter.alignBytes * i + baseOffset) < splitParamter.indexBufferSize; ++i)
    {
        /* only one index beyond 64 bytes.*/
        if (((splitParamter.alignBytes * i + baseOffset) % splitParamter.bytesPerPatch) == splitParamter.splitBytesPerPatch)
        {
            return VK_TRUE;
        }
    }

    return VK_FALSE;
}

static VkResult halti5_pickSplitDrawIndexedFunc(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance,
    VK_DRAW_INDEXED_FUNC* pFunc
    )
{
    VkResult result = VK_SUCCESS;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    const gcsFEATURE_DATABASE *database = devCtx->pPhyDevice->phyDevConfig.database;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;

    char tempBuf[__VK_MAX_NAME_LENGTH];
    char *pos;
    const char* caseName = "\x9b\x9a\x8e\x8f"; /* "deqp".*/

    __vk_utils_reverseBytes(caseName, tempBuf, __VK_MAX_NAME_LENGTH);
    gcoOS_StrStr(devCtx->pPhyDevice->pInst->applicationName, tempBuf, &pos);

    /* Default func.*/
    *pFunc = halti5_drawIndexed;

    /* Pick split draw func.*/
    if (pip->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
    &&  pos
    &&  firstIndex == 0
    &&  !database->FE_PATCHLIST_FETCH_FIX
    &&  halti5_needSplitPatchList(commandBuffer, indexCount)
    )
    {
        *pFunc = halti5_splitDrawIndexedPatchList;
    }


    return result;
}

VkResult halti5_splitDrawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    )
{
    VkResult result = VK_SUCCESS;
    VK_DRAW_INDEXED_FUNC pFunc = VK_NULL_HANDLE;

    /* Pick match split draw.*/
    __VK_ONERROR(halti5_pickSplitDrawIndexedFunc(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, &pFunc));
    __VK_ONERROR((*pFunc)(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance));

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    return result;
}

static VkResult halti5_drawIndirect_common(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride,
    VkBool32 indexMode
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t drawCommand;
    uint32_t *states;
    uint32_t srcAddr;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    VkBool32 useOneCore = VK_FALSE;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    static const uint32_t s_xlatePrimitiveTopology[] =
    {
        /* VK_PRIMITIVE_TOPOLOGY_POINT_LIST */
        0x1,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST */
        0x2,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP */
        0x3,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST */
        0x4,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP */
        0x5,
        /*  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN */
        0x6,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY */
        0x9,
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY */
        0xA,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY */
        0xB,
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY */
        0xC,
        /* VK_PRIMITIVE_TOPOLOGY_PATCH_LIST */
        0xD,
    };

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (0 != cmdBuf->bindInfo.vertexBuffers.firstInstance)
    {
        cmdBuf->bindInfo.vertexBuffers.firstInstance = 0;
        cmdBuf->bindInfo.vertexBuffers.dirtyBits |= chipGfxPipeline->instancedVertexBindingMask;
    }

    if (0 != cmdBuf->bindInfo.indexBuffer.firstIndex)
    {
        cmdBuf->bindInfo.indexBuffer.firstIndex = 0;
        cmdBuf->bindInfo.indexBuffer.dirty = VK_TRUE;
    }

    if (chipGfxPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.draw.indexDraw = indexMode;
        cmdParams.draw.indirectDraw = VK_TRUE;
        cmdParams.draw.buffer = buffer;
        cmdParams.draw.offset = offset;
        cmdParams.draw.count = count;
        cmdParams.draw.stride = stride;
    }
    __VK_ONERROR(halti5_draw_validate(cmdBuf, &cmdParams));

    if (cmdBuf->gpuRenderingMode == gcvMULTI_GPU_RENDERING_MODE_OFF)
    {
        useOneCore = VK_TRUE;
    }

    /* Determine draw command. */
    drawCommand = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (indexMode) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:0) - (0 ?
 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (s_xlatePrimitiveTopology[pip->topology]) & ((gctUINT32) ((((1 ?
 3:0) - (0 ?
 3:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ? 3:0)));

    srcAddr = buf->memory->devAddr;
    srcAddr += (uint32_t)(buf->memOffset + offset);

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
        {
            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&pCmdBuffer)++ = 0;
;

        }
    }

    if (chipGfxPipeline->baseInstance.bUsed)
    {
        __VK_ASSERT(!cmdBuf->devCtx->pPhyDevice->phyDevFeatures.drawIndirectFirstInstance);
        __vkCmdLoadSingleHWState(&pCmdBuffer, chipGfxPipeline->baseInstance.hwRegAddress, VK_FALSE, 0);
    }
    __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
    *pCmdBuffer++ = drawCommand;
    *pCmdBuffer++ = srcAddr;
    __VK_DEBUG_ONLY(})


    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        if (useOneCore)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;

            halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
        }
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}


VkResult halti5_drawIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    )
{
    return halti5_drawIndirect_common(commandBuffer, buffer, offset, count, stride, VK_FALSE);
}

VkResult halti5_drawIndexedIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    )
{
    return halti5_drawIndirect_common(commandBuffer, buffer, offset, count, stride, VK_TRUE);
}

static VkResult halti5_compute_validate(
    __vkCommandBuffer *cmdBuf,
    __vkDrawComputeCmdParams *cmdParams
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t threadAllocation;
    uint32_t data[3];
    __vkCmdBindInfo *bindInfo = &cmdBuf->bindInfo;
    __vkPipeline *pip = bindInfo->pipeline.compute;
    __vkCmdBindDescSetInfo *descSetInfo = &bindInfo->bindDescSet.compute;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;
    VkBool32 computePipelineDirty = (bindInfo->pipeline.dirty & __VK_CMDBUF_BINDINGINFO_PIPELINE_COMPUTE_DIRTY);
    VkBool32 vscprogramInstanceSwitched = VK_FALSE;
    VkResult result = VK_SUCCESS;
#if __VK_RESOURCE_INFO
    static int32_t tgtCmdBufferID = 0;
    static uint32_t tgtSequenceID = 0;

    if ((tgtCmdBufferID == cmdBuf->obj.id)
    && (tgtSequenceID == cmdBuf->sequenceID))
    {
       /* make compiler happy */
       uint32_t tmpID = tgtSequenceID;
       tgtSequenceID = tmpID;
    }
#endif

    do
    {
        if (chipPipeline->tweakHandler)
        {
            (*chipPipeline->tweakHandler->set)(cmdBuf, cmdParams, chipPipeline->tweakHandler);
        }

        /* patch pipeline */
        if ((computePipelineDirty || descSetInfo->dirtyMask) && (!chipPipeline->vanilla))
        {
            __VK_ERR_BREAK(halti5_patch_pipeline(pip, descSetInfo, &vscprogramInstanceSwitched));
        }

        if (bindInfo->pipeline.activePipeline != VK_PIPELINE_BIND_POINT_COMPUTE)
        {
            halti5_pipeline_switch(cmdBuf);
            bindInfo->pipeline.activePipeline = VK_PIPELINE_BIND_POINT_COMPUTE;
        }

        /* dirty descSet when pipeline switch, we don't have pipeline compatibility now !!! */
        if (computePipelineDirty || vscprogramInstanceSwitched)
        {
            descSetInfo->dirtyMask = ~(~0 << chipPipeline->masterInstance->pep.u.vk.resourceSetCount);
        }

        if (vscprogramInstanceSwitched || computePipelineDirty)
        {
            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                chipPipeline->curInstance->hwStates.pStateBuffer, chipPipeline->curInstance->hwStates.stateBufferSize);
            cmdBuf->curScrachBufIndex += chipPipeline->curInstance->hwStates.stateBufferSize / sizeof(uint32_t);

            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                 chipPipeline->curInstance->instanceCmd, chipPipeline->curInstance->curInstanceCmdIndex * sizeof(uint32_t));
             cmdBuf->curScrachBufIndex += chipPipeline->curInstance->curInstanceCmdIndex;
        }

        if (computePipelineDirty)
        {
            __VK_MEMCOPY(&cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex],
                chipPipeline->cmdBuffer, chipPipeline->curCmdIndex * sizeof(uint32_t));
            cmdBuf->curScrachBufIndex += chipPipeline->curCmdIndex;
            bindInfo->pipeline.dirty &= ~__VK_CMDBUF_BINDINGINFO_PIPELINE_COMPUTE_DIRTY;
        }

        if (bindInfo->bindDescSet.compute.dirtyMask)
        {
            __VK_ERR_BREAK(halti5_setDesriptorSets(cmdBuf, pip, descSetInfo));
            bindInfo->bindDescSet.compute.dirtyMask = 0;
        }

        if (cmdBuf->bindInfo.pushConstants.dirtyMask)
        {
            __VK_ERR_BREAK(halti5_setPushConstants(cmdBuf, pip));
            bindInfo->pushConstants.dirtyMask = 0;
        }

        threadAllocation = gcmCEIL((gctFLOAT)(hints->workGrpSize.x * hints->workGrpSize.y * hints->workGrpSize.z)
            / (devCtx->database->NumShaderCores * 4));

        pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0240, VK_FALSE,
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
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:12) - (0 ?
 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:12) - (0 ?
 15:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ? 15:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (hints->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24))));

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0247, VK_FALSE, threadAllocation);

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x024B, VK_FALSE,
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

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x024D, VK_FALSE,
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

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x024F, VK_FALSE,
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

        data[0] = hints->workGrpSize.x - 1;
        data[1] = hints->workGrpSize.y - 1;
        data[2] = hints->workGrpSize.z - 1;

        __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0253, VK_FALSE, 3, data);

        cmdBuf->curScrachBufIndex += (uint32_t)(uintptr_t)(pCmdBuffer - pCmdBufferBegin);

        if (devCtx->database->DRAWID)
        {
            __VK_ERR_BREAK(halti5_setDrawID(cmdBuf, pip));
        }

        if (chipCommandBuffer->newResourceViewUsageMask)
        {
            __VK_ERR_BREAK(halti5_flushRsViewFirstUse(cmdBuf));
        }

    } while (VK_FALSE);

    cmdBuf->sequenceID++;

    return result;
}

VkResult halti5_dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t x,
    uint32_t y,
    uint32_t z
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    uint32_t *states;
    uint32_t data[3];
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_computePipeline *chipCmptPipeline = (halti5_computePipeline *)cmdBuf->bindInfo.pipeline.compute->chipPriv;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    __vkDevContext *devCtx = cmdBuf->devCtx;

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (chipCmptPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.compute.indirectCompute = VK_FALSE;
        cmdParams.compute.x = x;
        cmdParams.compute.y = y;
        cmdParams.compute.z = z;
    }

    __VK_ONERROR(halti5_compute_validate(cmdBuf, &cmdParams));

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);

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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&pCmdBuffer)++ = 0;
;

    }

    data[0] = x - 1;
    data[1] = y - 1;
    data[2] = z - 1;

    __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0250, VK_FALSE, 3, data);

    if (chipCmptPipeline->numberOfWorkGroup.bUsed)
    {
        data[0] = x; data[1] = y; data[2] = z;
        __vkCmdLoadBatchHWStates(&pCmdBuffer, chipCmptPipeline->numberOfWorkGroup.hwRegAddress, VK_FALSE, 3, data);
    }
    __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0248, VK_FALSE, 0xBADABEEB);
    __VK_DEBUG_ONLY(})

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;


        halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
    }
    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}

VkResult halti5_dispatchIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    uint32_t *states;
    uint32_t srcAddr;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_computePipeline *chipCmptPipeline = (halti5_computePipeline *)cmdBuf->bindInfo.pipeline.compute->chipPriv;
    VkResult result;
    __vkDrawComputeCmdParams cmdParams;
    __vkDevContext *devCtx = cmdBuf->devCtx;

    __VK_ASSERT(cmdBuf->curScrachBufIndex == 0);

    if (chipCmptPipeline->chipPipeline.tweakHandler)
    {
        __VK_MEMZERO(&cmdParams, sizeof(cmdParams));
        cmdParams.compute.indirectCompute = VK_TRUE;
        cmdParams.compute.buffer = buffer;
        cmdParams.compute.offset = offset;
    }

    __VK_ONERROR(halti5_compute_validate(cmdBuf, &cmdParams));

    srcAddr = buf->memory->devAddr;
    srcAddr += (uint32_t)(buf->memOffset + offset);

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);

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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK << (0));*(*&pCmdBuffer)++ = 0;
;

    }

    if (chipCmptPipeline->numberOfWorkGroup.bUsed)
    {
        struct _gcsHINT *hints = &chipCmptPipeline->chipPipeline.masterInstance->hwStates.hints;
        __vkCmdLoadSingleHWState(&pCmdBuffer,
                                 0x01F3,
                                 VK_FALSE,
                                 chipCmptPipeline->numberOfWorkGroup.hwRegNo + hints->constRegNoBase[gcvPROGRAM_STAGE_FRAGMENT]);
    }

    __VK_DEBUG_ONLY(if (!g_dbgSkipDraw) {)
    *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x11 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
    *pCmdBuffer++ = srcAddr;
    __VK_DEBUG_ONLY(})


    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_ALL_MASK);*(*&pCmdBuffer)++ = 0;
;


        halti5_setMultiGpuSync((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE);
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    __vk_CmdAquireBuffer(commandBuffer, cmdBuf->curScrachBufIndex, &states);

    __VK_MEMCOPY(states, cmdBuf->scratchCmdBuffer, cmdBuf->curScrachBufIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, cmdBuf->curScrachBufIndex);

OnError:
    __VK_ASSERT(result == VK_SUCCESS);
    cmdBuf->curScrachBufIndex = 0;
    return result;
}
#if __VK_ENABLETS
uint32_t halti5_computeTileStatusAddr(
    __vkDevContext *devCtx,
    __vkImage *img,
    uint32_t offset
    )
{
    VkBool32 is2BitPerTile = devCtx->database->REG_TileStatus2Bits;
    __vkTileStatus *tsResource = img->memory->ts;
    uint32_t tileStatusAddress = tsResource->devAddr;
    uint32_t tileStatusOffset = 0;
    tileStatusOffset = is2BitPerTile ? (offset >> 8) : (offset >> 7);

    if (img->sampleInfo.product > 1)
    {
        tileStatusOffset >>= 2;
    }

    tileStatusAddress += tileStatusOffset;

    return tileStatusAddress;
}

VkResult halti5_disableRtTileStatus(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImage *image,
    uint32_t hwRtIndex
    )
{
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;

    if (image->createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        /*
        ** 1. 0x0690 slot 0 is uesless on RTL impelmetation.
        ** 2. we need keep memoryConfig always has correct information for RT0,
        **    as it's combined with depth information.
        */
        if (hwRtIndex == 0)
        {
            /* Disable color tile status. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

            /* Make sure auto-disable is turned off as well. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

            /* Make sure compression is turned off as well. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));

            /* Program memory configuration register. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0595, VK_FALSE, chipCommand->memoryConfig);
        }
        else
        {
            /* Disable color tile status. */
            chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            /* Make sure auto-disable is turned off as well. */
            chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

            /* Make sure compression is turned off as well. */
            chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));

            /* Program memory configuration register. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x05E8 + hwRtIndex, VK_FALSE, chipCommand->memoryConfigMRT[hwRtIndex]);
        }
    }
    else if (image->createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        /* Disable depth tile status. */
        chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

        /* Make sure auto-disable is turned off as well. */
        chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

        /* Make sure compression is turned off as well. */
        chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

        /* Program memory configuration register. */
        __vkCmdLoadSingleHWState(commandBuffer, 0x0595, VK_FALSE, chipCommand->memoryConfig);
    }

    __VK_STALL_RA(commandBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

    return VK_SUCCESS;
}

VkResult halti5_setRtTileStatus(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImage *img,
    VkImageSubresourceRange* pRanges,
    uint32_t hwRtIndex
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkTileStatus *tsResource = img->memory->ts;
    uint32_t tileStatusAddress = tsResource ? tsResource->devAddr : VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;
    uint32_t tileCount = 0;
    VkBool32 autoDisable = VK_FALSE;
    VkBool32 canEnableTs = VK_TRUE;
    VkBool32 anyTSEnable = VK_FALSE;
    uint32_t size = 0;
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
    __vkImageLevel *pLevel = &img->pImgLevels[pRanges->baseMipLevel];

    do
    {
        if (tsResource == 0)
        {
            canEnableTs = VK_FALSE;
        }
        else
        {
            __VK_AnyTSEnable(tsResource, pRanges, &anyTSEnable);
            __VK_CanTSEnable(tsResource, pRanges, &canEnableTs);

            if (anyTSEnable)
            {
                if (!canEnableTs)
                {
                    halti5_decompressTileStatus(cmdBuf, commandBuffer, img, pRanges);

                    if (devCtx->database->REG_BltEngine)
                    {
                        __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

                        __vkCmdLoadSingleHWState(commandBuffer, 0x0E02, VK_FALSE,
                            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

                        __VK_STALL_RA(commandBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
                                                   | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

                        __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
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
                    }
                }
            }
        }

        if (!canEnableTs)
        {
            if (img->createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT && hwRtIndex == 0)
            {
                chipCommand->rt0TSEnable = VK_FALSE;
            }

            result = halti5_disableRtTileStatus(cmdBuf, commandBuffer, img, hwRtIndex);
            return result;
        }

        __VK_ASSERT(pRanges->levelCount == 1);

        size = (uint32_t)pLevel->sliceSize * pRanges->layerCount;

        if (devCtx->database->CACHE128B256BPERLINE)
        {
            /*Todo*/
        }
        else
        {
            /* 64-types per tile.*/
            tileCount = size / ((img->sampleInfo.product > 1) ? 256 : 64);
        }

        __VK_ASSERT((devCtx->database->REG_CorrectAutoDisableCountWidth) && (devCtx->database->REG_CorrectAutoDisable1));

        autoDisable = (tsResource->compressed == VK_FALSE);

        if (img->createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            uint32_t bitsPerPixel = img->formatInfo.bitsPerBlock / img->formatInfo.partCount;
            /* Flush the color cache. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));

            /*
            ** 1. 0x0690 slot 0 is uesless on RTL impelmetation.
            ** 2. we need keep memoryConfig always has correct information for RT0,
            **    as it's combined with depth information.
            */
            if (hwRtIndex == 0)
            {
                chipCommand->rt0TSEnable = VK_TRUE;

                /* Enable fast clear or not. */
                chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

                /* Turn on color compression when in MSAA mode. */
                chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (tsResource->compressed) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));

                if (tsResource->compressed)
                {
                    chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:8) - (0 ?
 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (tsResource->compressedFormat) & ((gctUINT32) ((((1 ?
 11:8) - (0 ?
 11:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ? 11:8)));
                }

                if (devCtx->database->V4Compression)
                {
                    /* Todo */
                }

                if (devCtx->database->CACHE128B256BPERLINE)
                {
                    /* Todo. */
                }

                /* Automatically disable fast clear or not. */
                chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

                chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) ((bitsPerPixel == 64)) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)));

                if (autoDisable)
                {
                    __vkCmdLoadSingleHWState(commandBuffer, 0x059D, VK_FALSE, tileCount);
                }

                /* Program tile status base address register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x0596, VK_FALSE,
                    tileStatusAddress);

                /* Program surface base address register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x0597, VK_FALSE,
                    img->memory->devAddr);

                /* Program clear value register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x0598, VK_FALSE,
                    tsResource->fcValue[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

                __vkCmdLoadSingleHWState(commandBuffer, 0x05AF, VK_FALSE,
                    tsResource->fcValueUpper[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

                gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", tileStatusAddress, tsResource->tsNode.size);

                /* Program memory configuration register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x0595, VK_FALSE, chipCommand->memoryConfig);
            }
            else
            {
                /* Enable fast clear or not. */
                chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

                /* Turn on color compression when in MSAA mode. */
                chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (tsResource->compressed) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));

                if (tsResource->compressed)
                {
                    chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:3) - (0 ?
 6:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:3) - (0 ?
 6:3) + 1))))))) << (0 ?
 6:3))) | (((gctUINT32) ((gctUINT32) (tsResource->compressedFormat) & ((gctUINT32) ((((1 ?
 6:3) - (0 ?
 6:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:3) - (0 ? 6:3) + 1))))))) << (0 ? 6:3)));
                }

                if (devCtx->database->V4Compression)
                {
                    /* Todo */
                }

                if (devCtx->database->CACHE128B256BPERLINE)
                {
                    /* Todo. */
                }

                /* Automatically disable fast clear or not. */
                chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)));

                chipCommand->memoryConfigMRT[hwRtIndex] = ((((gctUINT32) (chipCommand->memoryConfigMRT[hwRtIndex])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) ((bitsPerPixel == 64)) & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)));

                if (autoDisable)
                {
                    __vkCmdLoadSingleHWState(commandBuffer, 0x0690 + hwRtIndex, VK_FALSE, tileCount);
                }

                /* Program tile status base address register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x05F0 + hwRtIndex, VK_FALSE,
                    tileStatusAddress);

                /* Program surface base address register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x05F8 + hwRtIndex, VK_FALSE,
                    img->memory->devAddr);

                /* Program clear value register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x0680 + hwRtIndex, VK_FALSE,
                    tsResource->fcValue[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

                __vkCmdLoadSingleHWState(commandBuffer, 0x0688 + hwRtIndex, VK_FALSE,
                    tsResource->fcValueUpper[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

                gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", tileStatusAddress, tsResource->tsNode.size);

                /* Program memory configuration register. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x05E8 + hwRtIndex, VK_FALSE, chipCommand->memoryConfigMRT[hwRtIndex]);
            }
        }
        else if (img->createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            uint32_t bitsPerPixel = img->formatInfo.bitsPerBlock / img->formatInfo.partCount;
            VkBool32 hasStencil = VK_FALSE;
            /* Flush the depth cache. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0E03, VK_FALSE, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

            /* Enable fast clear or not. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            /* Set depth format. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (bitsPerPixel == 16) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)));

            /* Enable compression or not. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (tsResource->compressed) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)));

            if (devCtx->database->V4Compression)
            {
                /* Todo */
            }

            /* Automatically disable fast clear or not. */
            chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (autoDisable) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

            if (devCtx->database->REG_Halti5)
            {
                hasStencil = ((img->formatInfo.residentImgFormat == __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32)
                    || (img->formatInfo.residentImgFormat == VK_FORMAT_S8_UINT));

                chipCommand->memoryConfig = ((((gctUINT32) (chipCommand->memoryConfig)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (hasStencil) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14)));
            }

            if (autoDisable)
            {
                __vkCmdLoadSingleHWState(commandBuffer, 0x059C, VK_FALSE, tileCount);
            }

            /* Program fast clear registers. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0599, VK_FALSE, 3);

            /* Program tile status base address register. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0599, VK_FALSE,
                tileStatusAddress);

            __vkCmdLoadSingleHWState(commandBuffer, 0x059A, VK_FALSE,
                img->memory->devAddr);

            /* Program clear value register. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x059B, VK_FALSE,
                tsResource->fcValue[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

            /* Program memory configuration register. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x0595, VK_FALSE, chipCommand->memoryConfig);

            gcmDUMP(gcvNULL, "#[surface 0x%x 0x%x]", tileStatusAddress, tsResource->tsNode.size);
        }

        __VK_STALL_RA(commandBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

        /* Any time tile status enable, we need set cache mode */
        if (devCtx->database->CACHE128B256BPERLINE && tileStatusAddress)
        {
            /* Todo*/
            /* When enable TS, we don't always trigger flushTarget, but cachemode could change */
        }

    }while (VK_FALSE);

    return result;
}

static VkResult halti5_3DBlitTileFill(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkImage *img,
    VkImageSubresourceRange* pRanges
    )
{
    __vkTileStatus *tsResource = img->memory->ts;
    uint32_t tileCount = 0;
    uint32_t fillerTileSize = 0x0;
    uint32_t bitsPerPixel = img->formatInfo.bitsPerBlock / img->formatInfo.partCount;
    uint32_t fillerBpp = 0;
    uint32_t fillerFormat = 0;
    uint32_t generalConfig = 0;
    uint32_t tileStatusAddress = tsResource->devAddr;
    VkResult result = VK_SUCCESS;
    uint32_t size = 0;
    uint32_t level;
    uint32_t dstAddress = img->memory->devAddr;
    uint32_t offset = 0;
    __vkImageLevel *baseLevel = &img->pImgLevels[pRanges->baseMipLevel];

    do
    {
        /* compute size. */
        for (level = pRanges->baseMipLevel; level < pRanges->baseMipLevel + pRanges->levelCount; ++level)
        {
            __vkImageLevel *pLevel = &img->pImgLevels[level];
            size += (uint32_t)pLevel->sliceSize * pRanges->layerCount;
        }

        offset = (uint32_t)(img->memOffset + baseLevel->offset + pRanges->baseArrayLayer * baseLevel->sliceSize);
        dstAddress += offset;
        tileStatusAddress = halti5_computeTileStatusAddr(devCtx, img, offset);

        if (devCtx->database->CACHE128B256BPERLINE)
        {
            /* Todo. */
            switch (bitsPerPixel)
            {
            case 8:
                fillerBpp = 0;
                break;
            case 16:
                fillerBpp = 1;
                break;
            case 32:
                fillerBpp = 2;
                break;
            case 64:
                fillerBpp = 3;
                break;
            case 128:
                fillerBpp = 4;
                break;
            default:
                __VK_ASSERT(0);
                fillerBpp = 2;
                break;
            }
        }
        else
        {
            /* Calculate tile count. */
            if (img->sampleInfo.product > 1)
            {
                tileCount = size / 256;
                fillerTileSize = 0x1;
            }
            else
            {
                tileCount = size / 128;
                fillerTileSize = 0x0;
            }
        }

        fillerFormat = 0x1;

        generalConfig
            = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (fillerTileSize) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (fillerFormat) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:15) - (0 ?
 17:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:15) - (0 ?
 17:15) + 1))))))) << (0 ?
 17:15))) | (((gctUINT32) ((gctUINT32) (fillerBpp) & ((gctUINT32) ((((1 ?
 17:15) - (0 ?
 17:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:15) - (0 ? 17:15) + 1))))))) << (0 ? 17:15)));

        /************************************************
        * Program states.
        */
        /* Flush tile status buffer */
        __vkCmdLoadSingleHWState(commandBuffer, 0x0594, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

        __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

        /* GeneralConfig. */
        __vkCmdLoadSingleHWState(commandBuffer, 0x5019, VK_FALSE, generalConfig);

        if (devCtx->chipInfo->gpuCoreCount > 1)
        {
            uint32_t i;
            uint32_t tsBitsPerTile = devCtx->database->REG_TileStatus2Bits ? 2 : 4;
            uint32_t averateTiles = __VK_ALIGN(tileCount / devCtx->chipInfo->gpuCoreCount, 8 / tsBitsPerTile);
            uint32_t tileFillCount = tileCount - (devCtx->chipInfo->gpuCoreCount - 1) * averateTiles;
            uint32_t dstAddr = dstAddress;
            uint32_t dstTsAddr = tileStatusAddress;

            for (i = 0; i < devCtx->chipInfo->gpuCoreCount; i++)
            {
                *(*commandBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*commandBuffer)++ = 0;
;


                /* DestAddress. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x5006, VK_FALSE, dstAddr);

                /* DestTileStatusAddress. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x5008, VK_FALSE, dstTsAddr);

                /* DstTileCount. */
                __vkCmdLoadSingleHWState(commandBuffer, 0x501A, VK_FALSE, tileFillCount);

                dstAddr += tileFillCount * ((img->sampleInfo.product > 1) ? 256 : 64);
                dstTsAddr += tileFillCount * tsBitsPerTile / 8;
                tileFillCount = averateTiles;
            }
        }
        else
        {
            /* DestAddress. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x5006, VK_FALSE, dstAddress);

            /* DestTileStatusAddress. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x5008, VK_FALSE, tileStatusAddress);

            /* DstTileCount. */
            __vkCmdLoadSingleHWState(commandBuffer, 0x501A, VK_FALSE, tileCount);
        }

        /* DstClearValue. */
        __vkCmdLoadSingleHWState(commandBuffer, 0x500F, VK_FALSE,
            tsResource->fcValue[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

        /* DstClearValue64. */
        __vkCmdLoadSingleHWState(commandBuffer, 0x5010, VK_FALSE,
            tsResource->fcValueUpper[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

        __vkCmdLoadSingleHWState(commandBuffer, 0x5018, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
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
        __vkCmdLoadSingleHWState(commandBuffer, 0x502B, VK_FALSE,
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

        __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
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
    }while (VK_FALSE);

    return result;
}

static VkResult halti5_3DBlitToSelf(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkBlitRes *dstRes,
    VkRect2D *rect
    )
{
    __vkImage *img = dstRes->u.img.pImage;
    __vkImageLevel *pLevel = &img->pImgLevels[dstRes->u.img.subRes.mipLevel];
    VkResult result = VK_SUCCESS;
    __vkTileStatus *tsResource = img->memory->ts;
    uint32_t tileStatusAddress = tsResource ? tsResource->devAddr : VK_NULL_HANDLE;
    uint32_t config, dstConfigEx, srcConfigEx;
    HwBLTDesc dstBltDesc;
    uint32_t msaa, superTile, tiling, cacheMode, address;
    VkBool32 fastClear = VK_FALSE;
    int32_t srcCompressionFormat = -1;
    VkBool32 srcCompression = VK_FALSE;
    uint32_t color64 = 0;
    uint32_t offset = 0;

    __VK_ONERROR(halti5_helper_convertHwBltDesc(VK_FALSE, img->formatInfo.residentImgFormat, &dstBltDesc));
    halti5_helper_configMSAA(img, &msaa, &cacheMode);
    halti5_helper_configTiling(img, &tiling, &superTile);

    if (tileStatusAddress)
    {
        fastClear = !tsResource->tileStatusDisable[dstRes->u.img.subRes.mipLevel][dstRes->u.img.subRes.arrayLayer];
    }

    if (fastClear)
    {
        srcCompressionFormat = tsResource->compressedFormat;
        srcCompression = tsResource->compressed;
    }

    color64 = (tsResource->fcValue[dstRes->u.img.subRes.mipLevel][dstRes->u.img.subRes.arrayLayer]
        != tsResource->fcValueUpper[dstRes->u.img.subRes.mipLevel][dstRes->u.img.subRes.arrayLayer]);

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

    dstConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 20:20))) | (((gctUINT32) ((gctUINT32) (VK_FALSE) & ((gctUINT32) ((((1 ?
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
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22)));

    srcConfigEx = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 19:19))) | (((gctUINT32) ((gctUINT32) (VK_FALSE) & ((gctUINT32) ((((1 ?
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
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (color64) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)));

    address = img->memory->devAddr;
    offset = (uint32_t)(img->memOffset + pLevel->offset
        + dstRes->u.img.subRes.arrayLayer * pLevel->sliceSize);
    address += offset;

    tileStatusAddress = halti5_computeTileStatusAddr(devCtx, img, offset);

    if (devCtx->database->CACHE128B256BPERLINE)
    {
        /* Todo. */
    }

    __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

    __vkCmdLoadSingleHWState(commandBuffer, 0x5019, VK_FALSE,
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

    __vkCmdLoadSingleHWState(commandBuffer, 0x502F, VK_FALSE,
        (dstBltDesc.bltSwizzleEx | (dstBltDesc.bltSwizzleEx << 12))
        );

    __vkCmdLoadSingleHWState(commandBuffer, 0x5009, VK_FALSE, config);
    __vkCmdLoadSingleHWState(commandBuffer, 0x500A, VK_FALSE, dstConfigEx);
    __vkCmdLoadSingleHWState(commandBuffer, 0x5006, VK_FALSE, address);

    __vkCmdLoadSingleHWState(commandBuffer, 0x5002, VK_FALSE, config);
    __vkCmdLoadSingleHWState(commandBuffer, 0x5003, VK_FALSE, srcConfigEx);
    __vkCmdLoadSingleHWState(commandBuffer, 0x5000, VK_FALSE, address);
    /* Tilesize for customer config */
    __vkCmdLoadSingleHWState(commandBuffer, 0x5028, VK_FALSE,
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
    __vkCmdLoadSingleHWState(commandBuffer, 0x5027, VK_FALSE,
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

    if (fastClear)
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x5004, VK_FALSE, tileStatusAddress);
        __vkCmdLoadSingleHWState(commandBuffer, 0x500D, VK_FALSE,
            tsResource->fcValue[dstRes->u.img.subRes.mipLevel][dstRes->u.img.subRes.arrayLayer]);
        __vkCmdLoadSingleHWState(commandBuffer, 0x500E, VK_FALSE,
            tsResource->fcValueUpper[dstRes->u.img.subRes.mipLevel][dstRes->u.img.subRes.arrayLayer]);
    }

    if (devCtx->chipInfo->gpuCoreCount > 1)
    {
        uint32_t i;
        uint32_t averageW = __VK_ALIGN((rect->extent.width * img->sampleInfo.x) / devCtx->chipInfo->gpuCoreCount,
                                        img->sampleInfo.x);
        uint32_t splitWidth = (rect->extent.width * img->sampleInfo.x) - (devCtx->chipInfo->gpuCoreCount - 1) * averageW;
        uint32_t srcOriginX = rect->offset.x * img->sampleInfo.x;
        uint32_t dstOriginX = rect->offset.x * img->sampleInfo.x;
        uint32_t leftWidth = rect->extent.width;

        for (i = 0; i < devCtx->chipInfo->gpuCoreCount; i++)
        {
            /* Align srcEndX to 64 boundary */
            uint32_t srcEndX = __VK_ALIGN(srcOriginX + splitWidth, 64);
            splitWidth = __VK_ALIGN(srcEndX - srcOriginX, leftWidth);

            *(*commandBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | ((1 << i));*(*commandBuffer)++ = 0;
;


            __vkCmdLoadSingleHWState(commandBuffer, 0x500B, VK_FALSE,
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
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->offset.y * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );
            __vkCmdLoadSingleHWState(commandBuffer, 0x500B, VK_FALSE,
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
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->offset.y * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );
            __vkCmdLoadSingleHWState(commandBuffer, 0x500C, VK_FALSE,
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
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->extent.height * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
                );

            leftWidth -= splitWidth;
            srcOriginX += splitWidth;
            dstOriginX += splitWidth;
            splitWidth = averageW;
        }
    }
    else
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x500B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (rect->offset.x * img->sampleInfo.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->offset.y * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(commandBuffer, 0x500B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (rect->offset.x * img->sampleInfo.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->offset.y * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
        __vkCmdLoadSingleHWState(commandBuffer, 0x500C, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (rect->extent.width  * img->sampleInfo.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:16) - (0 ?
 31:16) + 1))))))) << (0 ?
 31:16))) | (((gctUINT32) ((gctUINT32) (rect->extent.height * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ?
 31:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:16) - (0 ? 31:16) + 1))))))) << (0 ? 31:16)))
            );
    }

    __vkCmdLoadSingleHWState(commandBuffer, 0x502B, VK_FALSE,
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
    __vkCmdLoadSingleHWState(commandBuffer, 0x5018, VK_FALSE,
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
    __vkCmdLoadSingleHWState(commandBuffer, 0x502B, VK_FALSE,
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

    __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
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

OnError:
    return result;
}

VkResult halti5_decompressTileStatus(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImage *img,
    VkImageSubresourceRange* pRanges
    )
{
    __vkTileStatus *tsResource = img->memory->ts;
    VkResult result = VK_SUCCESS;
    VkBool32 anyTsEnable = VK_FALSE;
    uint32_t i = 0;
    uint32_t j = 0;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    /* tileFiller 32tile aligned. */
    uint32_t sizeAlignment = 0x7ff;
    VkBool32 canFillFullSlice = VK_TRUE;

    /* If tilestatus was already disabled */
    __VK_AnyTSEnable(tsResource, pRanges, &anyTsEnable);

    if (!anyTsEnable)
    {
        goto OnError;
    }

    /* Flush the pipe. */
    __VK_ONERROR(halti5_flushCache((VkDevice)devCtx, commandBuffer, VK_NULL_HANDLE, HW_CACHE_ALL));

    __VK_CanTSEnable(tsResource, pRanges, &canFillFullSlice);

    __VK_ASSERT((devCtx->database->REG_TileFiller) && (devCtx->database->REG_FcFlushStall));

    if ((!tsResource->compressed) && ((img->memory->node.size & sizeAlignment) == 0))
    {
        if (!devCtx->database->PE_TILE_CACHE_FLUSH_FIX &&
            devCtx->database->REG_BltEngine)
        {
            __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

            __vkCmdLoadSingleHWState(commandBuffer, 0x502B, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

            __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
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

            /* Semaphore-stall before next primitive. */
            __VK_STALL_RA(commandBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));
        }
        else
        {
            __vkCmdLoadSingleHWState(commandBuffer, 0x0594, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

            /* Semaphore-stall before next primitive. */
            __VK_STALL_RA(commandBuffer, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));
        }

        /* Fast clear fill. */
        if (!canFillFullSlice)
        {
            uint32_t level, layer;
            VkImageSubresourceRange fillRanges;

            for (level = pRanges->baseMipLevel; level < pRanges->baseMipLevel + pRanges->levelCount; ++level)
            {
                fillRanges.aspectMask = pRanges->aspectMask;
                fillRanges.baseMipLevel = level;
                fillRanges.levelCount = 1;
                for (layer = pRanges->baseArrayLayer; layer < pRanges->baseArrayLayer + pRanges->layerCount; ++layer)
                {
                    fillRanges.baseArrayLayer = layer;
                    fillRanges.layerCount = 1;
                    __VK_ONERROR(halti5_3DBlitTileFill(devCtx, commandBuffer, img, &fillRanges));
                    tsResource->tileStatusDisable[level][layer] = VK_TRUE;
                }
            }
        }
        else
        {
            __VK_ONERROR(halti5_3DBlitTileFill(devCtx, commandBuffer, img, pRanges));

            /* Disable the tile status for this surface. */
            for (i = pRanges->baseMipLevel; i < (pRanges->baseMipLevel + pRanges->levelCount); i++)
            {
                for (j = pRanges->baseArrayLayer; j < (pRanges->baseArrayLayer + pRanges->layerCount); j++)
                {
                    tsResource->tileStatusDisable[i][j] = VK_TRUE;
                }
            }
        }
    }
    else
    {
        VkRect2D rect;
        uint32_t level, layer;
        __vkBlitRes dstRes;

        __VK_MEMZERO(&dstRes, sizeof(dstRes));
        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = img;

        for (level = pRanges->baseMipLevel; level < pRanges->baseMipLevel + pRanges->levelCount; ++level)
        {
            __vkImageLevel *pLevel = &dstRes.u.img.pImage->pImgLevels[level];
            rect.offset.x = rect.offset.y = 0;
            rect.extent.width = pLevel->requestW;
            rect.extent.height = pLevel->requestH;
            dstRes.u.img.subRes.mipLevel = level;

            for (layer = pRanges->baseArrayLayer; layer < pRanges->baseArrayLayer + pRanges->layerCount; ++layer)
            {
                dstRes.u.img.subRes.arrayLayer = layer;
                __VK_ONERROR(halti5_3DBlitToSelf(devCtx, commandBuffer, &dstRes, &rect));
                tsResource->tileStatusDisable[level][layer] = VK_TRUE;
            }
        }
    }


OnError:
    return result;
}
#endif
VkResult halti5_setIndexBuffer(
    __vkCommandBuffer *cmdBuf
    )
{
    uint32_t *pCmdBuffer, *pCmdBufferBegin;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    halti5_setIndexBufferCmd(cmdBuf, &pCmdBuffer);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

VkResult halti5_setVertexBuffers(
    __vkCommandBuffer *cmdBuf
    )
{
    uint32_t i = 0;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint32_t dirtyMask = cmdBuf->bindInfo.vertexBuffers.dirtyBits;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)
        cmdBuf->bindInfo.pipeline.graphics->chipPriv;
    const gcsFEATURE_DATABASE *database = cmdBuf->devCtx->database;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];
    while(dirtyMask)
    {
        __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->bindInfo.vertexBuffers.buffers[i]);

        if ((dirtyMask & (1 << i)) && buf)
        {
            uint32_t srcAddr;
            srcAddr = buf->memory->devAddr;
            srcAddr += (uint32_t)(buf->memOffset + cmdBuf->bindInfo.vertexBuffers.offsets[i]);
            if ((cmdBuf->bindInfo.vertexBuffers.firstInstance) && (chipGfxPipeline->instancedVertexBindingMask & (1 << i)))
            {
                srcAddr += cmdBuf->bindInfo.vertexBuffers.firstInstance * chipGfxPipeline->instancedVertexBindingStride[i];
            }
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5180 + i, VK_FALSE, srcAddr);

            if (database->ROBUSTNESS)
            {
                uint32_t endAddress = srcAddr + (uint32_t)buf->memReq.size - 1;
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x51B0 + i, VK_FALSE, endAddress);
            }
        }
        dirtyMask &= ~(1 << i);
        i++;
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}

VkResult halti5_setViewport(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkDynamicViewportState *viewportState = &((__vkCommandBuffer *)cmdBuf)->bindInfo.dynamicStates.viewport;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    halti5_helper_set_viewport(devCtx, &pCmdBuffer, &viewportState->viewports[0]);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setScissor(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDynamicScissorState *scissorState;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    __vkRenderSubPassInfo *subPass = cmdBuf->bindInfo.renderPass.subPass;
    __vkFramebuffer *fb = cmdBuf->bindInfo.renderPass.fb;
    int32_t rtWidth, rtHeight;
    __vkImageView *rtImageView = VK_NULL_HANDLE;
    __vkImage *rtImage;
    __vkImageLevel *rtImageLevel;
    uint32_t scLeft, scTop, scRight, scBottom;
    gctINT fixRightClip;
    gctINT fixBottomClip;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    const gcsFEATURE_DATABASE *database = cmdBuf->devCtx->database;
    __vkDynamicViewportState *viewportState;
    uint32_t vpLeft, vpTop, vpRight, vpBottom;

    if (pip->dynamicStates & __VK_DYNAMIC_STATE_SCISSOR_BIT)
    {
        scissorState = &cmdBuf->bindInfo.dynamicStates.scissor;
    }
    else
    {
        scissorState = &pip->scissorState;
    }

    if (pip->dynamicStates & __VK_DYNAMIC_STATE_VIEWPORT_BIT)
    {
        viewportState = &((__vkCommandBuffer *)cmdBuf)->bindInfo.dynamicStates.viewport;
    }
    else
    {
        viewportState = &pip->viewportState;
    }

    if (pip->rasterDiscard && !database->HWTFB)
    {
        static __vkDynamicScissorState zeroScissor = {0};

        zeroScissor.scissorCount = scissorState->scissorCount;
        scissorState = &zeroScissor;
    }

    if ((subPass->colorCount > 0) &&
       (subPass->color_attachment_index[0] != VK_ATTACHMENT_UNUSED))
    {
        rtImageView = fb->imageViews[subPass->color_attachment_index[0]];
    }
    else if (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        rtImageView = fb->imageViews[subPass->dsAttachIndex];
    }

    if (rtImageView != VK_NULL_HANDLE)
    {
        rtImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, rtImageView->createInfo.image);
        rtImageLevel = &rtImage->pImgLevels[rtImageView->createInfo.subresourceRange.baseMipLevel];
        rtWidth = rtImageLevel->requestW;
        rtHeight = rtImageLevel->requestH;
    }
    else
    {
        rtWidth = 0;
        rtHeight = 0;
    }

    /* Intersect scissor with RT size, HW cannot handle out of range scissor */
    scLeft    = gcmMIN(gcmMAX(0, scissorState->scissors[0].offset.x), rtWidth);
    scTop     = gcmMIN(gcmMAX(0, scissorState->scissors[0].offset.y), rtHeight);
    scRight   = gcmMIN(gcmMAX(0, scissorState->scissors[0].offset.x + (int32_t)scissorState->scissors[0].extent.width), rtWidth);
    scBottom  = gcmMIN(gcmMAX(0, scissorState->scissors[0].offset.y + (int32_t)scissorState->scissors[0].extent.height), rtHeight);

    /* The application can specify a negative term for height, which has the effect of negating the y coordinate in clip space
    ** before performing the transform. When using a negative height, the application should also adjust the y value to point to
    ** the lower left corner of the viewport instead of the upper left corner.
    ** So, befor intersect scissor with viewport, adjust the top/bottom. */
    vpLeft   = (uint32_t)viewportState->viewports[0].x;
    vpTop = (viewportState->viewports[0].height > 0) ?
            (uint32_t)viewportState->viewports[0].y : (uint32_t)(viewportState->viewports[0].y + viewportState->viewports[0].height);
    vpRight  = (uint32_t)(viewportState->viewports[0].x + viewportState->viewports[0].width);
    vpBottom = (viewportState->viewports[0].height > 0) ?
        ((uint32_t)(viewportState->viewports[0].y + viewportState->viewports[0].height)) : (uint32_t)viewportState->viewports[0].y;

    /* Intersect scissor with viewport. */
    scLeft   = gcmMAX(scLeft, vpLeft);
    scTop    = gcmMAX(scTop, vpTop);
    scRight  = gcmMIN(scRight, vpRight);
    scBottom = gcmMIN(scBottom, vpBottom);

    /* Trivial case. */
    if ((scLeft >= scRight) || (scTop >= scBottom))
    {
        scLeft   = 1;
        scTop    = 1;
        scRight  = 1;
        scBottom = 1;
    }
    fixRightClip    = 0;
    fixBottomClip   = 0x1111;
    /* If right or bottom equal 8192, hw will overflow.
    We should decrease the size of Scissor.*/
    if (scRight == 8192)
    {
        fixRightClip = -0x119;
    }
    if (scBottom == 8192)
    {
        fixBottomClip = -0x111;
    }

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0300, VK_TRUE, scLeft << 16);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0301, VK_TRUE, scTop << 16);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0302, VK_TRUE, ((scRight << 16) + fixRightClip));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0303, VK_TRUE, ((scBottom << 16) + fixBottomClip));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0308, VK_TRUE, ((scRight << 16) + 0xFFFF));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0309, VK_TRUE, ((scBottom << 16) + 0xFFFFF));

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setDepthBias(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    __vkRenderPass *rdp = cmdBuf->bindInfo.renderPass.rdp;
    __vkRenderSubPassInfo *subPass = cmdBuf->bindInfo.renderPass.subPass;
    __vkAttachmentDesc *attachDesc;
    __vkDynamicDepthBiasState *depthBiasState = &cmdBuf->bindInfo.dynamicStates.depthBias;

    if (VK_ATTACHMENT_UNUSED != subPass->dsAttachIndex)
    {
        attachDesc = &rdp->attachments[subPass->dsAttachIndex];

        pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

        halti5_helper_set_depthBias(devCtx, attachDesc->formatInfo->residentImgFormat, &pCmdBuffer, pip->depthBiasEnable, depthBiasState);

        cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

        __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);
    }

    return VK_SUCCESS;
}

VkResult halti5_setStencilStates(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t stencilMode = chipGfxPipeline->stencilMode;
    __vkDynamicStencilState *dynamicStencilStates = & cmdBuf->bindInfo.dynamicStates.stencil;
    __vkDynamicStencilState  stenciltates;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (pip->dynamicStates & __VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT)
    {
        __VK_MEMCOPY(&stenciltates.compareMask[0], &dynamicStencilStates->compareMask[0], __VK_FACE_NUMBER * sizeof(uint32_t));
    }
    else
    {
        stenciltates.compareMask[__VK_FACE_FRONT] = pip->dsInfo.front.compareMask;
        stenciltates.compareMask[__VK_FACE_BACK] = pip->dsInfo.back.compareMask;
    }

    if (pip->dynamicStates & __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT)
    {
        __VK_MEMCOPY(&stenciltates.writeMask[0], &dynamicStencilStates->writeMask[0], __VK_FACE_NUMBER * sizeof(uint32_t));
    }
    else
    {
        stenciltates.writeMask[__VK_FACE_FRONT] = pip->dsInfo.front.writeMask;
        stenciltates.writeMask[__VK_FACE_BACK] = pip->dsInfo.back.writeMask;
    }

    if (pip->dynamicStates & __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT)
    {
        __VK_MEMCOPY(&stenciltates.reference[0], &dynamicStencilStates->reference[0], __VK_FACE_NUMBER * sizeof(uint32_t));
    }
    else
    {
        stenciltates.reference[__VK_FACE_FRONT] = pip->dsInfo.front.reference;
        stenciltates.reference[__VK_FACE_BACK] = pip->dsInfo.back.reference;
    }

    halti5_helper_set_stencilStates(devCtx, pip->frontFace, &pCmdBuffer, &stenciltates, stencilMode);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setBlendConstants(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx =  cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    __vkDynamicColorBlendState *blendState = &cmdBuf->bindInfo.dynamicStates.blend;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    halti5_helper_set_blendConstants(devCtx, &pCmdBuffer, blendState, pip->blendAttachmentCount);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setLineWidth(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkDynamicLineWidthState *dynamiclineWidthStates = &cmdBuf->bindInfo.dynamicStates.lineWidth;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    halti5_helper_set_linewidth(devCtx, &pCmdBuffer, dynamiclineWidthStates->width);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setPsOutputMode(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline* pip
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    uint32_t psOutCntl4to7 = hints->psOutCntl4to7;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    halti5_helper_set_psOutputMode(devCtx, &pCmdBuffer, chipGfxPipeline->psOutCntl4to7, psOutCntl4to7);

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

VkResult halti5_setRenderTargets(
    __vkCommandBuffer *cmdBuf
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkRenderSubPassInfo *subPass = cmdBuf->bindInfo.renderPass.subPass;
    __vkFramebuffer *fb = cmdBuf->bindInfo.renderPass.fb;
    __vkPipeline *pip = cmdBuf->bindInfo.pipeline.graphics;
    uint32_t i;
    __vkImage *rtImage = VK_NULL_HANDLE, *dsImage = VK_NULL_HANDLE;
    __vkImageView *rtImageView, *dsImageView;
    uint32_t colorAddressMode = ~0U;
    gceTILING tiling = gcvINVALIDTILED;
    uint32_t rtBaseAddr;
    __vkImageLevel *pLevel;
    uint32_t baseAddresses[2];
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
#if __VK_ENABLETS
    VkImageSubresourceRange* pRanges;
#endif
    const gcsFEATURE_DATABASE *database = devCtx->database;
    halti5_graphicsPipeline *chipGfxPipeline = (halti5_graphicsPipeline *)pip->chipPriv;
    uint32_t tileMode = 0;
#if __VK_ENABLETS
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
#endif
    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    for (i = 0; i < subPass->colorCount; i++)
    {
        halti5_imageView *chipImgv;
        uint32_t hwRtIndex ;
        uint32_t partIndex = 0;
        __VK_ASSERT(subPass->colorCount == pip->blendAttachmentCount);

        rtImageView = fb->imageViews[subPass->color_attachment_index[i]];
        __VK_ASSERT(rtImageView->createInfo.subresourceRange.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT);
        chipImgv = (halti5_imageView *)rtImageView->chipPriv;
        rtImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, rtImageView->createInfo.image);
        pLevel = &rtImage->pImgLevels[rtImageView->createInfo.subresourceRange.baseMipLevel];
#if __VK_ENABLETS
        pRanges = &rtImageView->createInfo.subresourceRange;
#endif

        if (gcvINVALIDTILED == tiling)
        {
            tiling = rtImage->halTiling;
        }
        else
        {
            __VK_ASSERT(tiling == rtImage->halTiling);
        }

        if (devCtx->database->CACHE128B256BPERLINE)
        {
            if (tiling == gcvSUPERTILED)
            {
                tileMode = 0x1;
            }
            else if (tiling== gcvYMAJOR_SUPERTILED)
            {
                tileMode = 0x2;
            }
        }

        while (partIndex < chipGfxPipeline->patchOutput.outputs[i].partCount)
        {
            hwRtIndex = chipGfxPipeline->patchOutput.outputs[i].locations[partIndex];

            rtBaseAddr = rtImage->memory->devAddr;
            rtBaseAddr += (uint32_t)(rtImage->memOffset
                + pLevel->partSize * partIndex
                + pLevel->offset
                + rtImageView->createInfo.subresourceRange.baseArrayLayer * pLevel->sliceSize);

            baseAddresses[0] = baseAddresses[1] = rtBaseAddr;
#if __VK_ENABLETS
            /* Enable rt tile status.*/
            halti5_setRtTileStatus(cmdBuf, &pCmdBuffer, rtImage, pRanges, hwRtIndex);
#endif
            if (hwRtIndex == 0)
            {
#if __VK_ENABLETS
                VkBool32 destinationRead = VK_FALSE;

                destinationRead = chipGfxPipeline->destinationRead || (rtImage->memory->ts && !chipCommand->rt0TSEnable && rtImage->memory->ts->compressed);
#endif
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x050B, VK_FALSE,
                    (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) ((rtImage->halTiling == gcvSUPERTILED)) & ((gctUINT32) ((((1 ?
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
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:13) - (0 ?
 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:13) - (0 ?
 14:13) + 1))))))) << (0 ?
 14:13))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))))
#if __VK_ENABLETS
                    & (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (!destinationRead) & ((gctUINT32) ((((1 ?
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
                    );

                if (database->NumPixelPipes == 1)
                {
                    __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0518, VK_FALSE, 1, baseAddresses);
                }
                else
                {
                    __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0518, VK_FALSE, 2, baseAddresses);
                }

                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x050D, VK_FALSE, (uint32_t)pLevel->stride);

                if (database->REG_GeometryShader)
                {
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E28, VK_FALSE,
                        (rtImageView->createInfo.subresourceRange.layerCount > 0) ? (uint32_t)pLevel->sliceSize : 0);
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0426, VK_FALSE, (rtImageView->createInfo.subresourceRange.layerCount - 1));
                }

                /* For mutable image, image can be used to create a VkImageView with a different format from the image.
                ** eg. image format is RGBA8_UINT, but imageView is RGBA8_UNORM.
                ** 1)draw to this image, 2)copy this image to buffer.
                ** PE not support RGBA8_UNORM, so treat the image as BGRA8_UNORM, but when copy, APP treat it as RGBA8_UINT. The R/B channel is opposite.
                ** So, not use faked format BGRA8_UNORM for RGBA8_UNORM, and use PE swizzle to deal with.
                ** But, For PE swizzle, not support MRT and Blend. So just a simple workround for this mutable group.
                */
                if ((rtImage->createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) &&
                    (rtImageView->createInfo.format == VK_FORMAT_R8G8B8A8_UNORM))
                {
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, VK_FALSE,
                        ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x3) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))
                        );
                }
                else
                {
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, VK_FALSE,
                        ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:16) - (0 ?
 17:16) + 1))))))) << (0 ?
 17:16))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 17:16) - (0 ?
 17:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:16) - (0 ? 17:16) + 1))))))) << (0 ? 17:16)))
                        & ((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:18) - (0 ?
 19:18) + 1))))))) << (0 ?
 19:18))) | (((gctUINT32) ((gctUINT32) (0x3) & ((gctUINT32) ((((1 ?
 19:18) - (0 ?
 19:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:18) - (0 ? 19:18) + 1))))))) << (0 ? 19:18)))
                        );
                }

                if (database->ROBUSTNESS)
                {
                    uint32_t endAddress = rtBaseAddr + (uint32_t)pLevel->sliceSize - 1;
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5270, VK_FALSE, endAddress);
                }
            }
            else
            {
                VkPipelineColorBlendAttachmentState *blendAttach = pip->blendAttachments;
                VkBool32 sRGB = VK_FALSE;
                HwPEDesc *hwPEDesc = &chipImgv->peDesc;
                switch (rtImageView->formatInfo->residentImgFormat)
                {
                case VK_FORMAT_R8G8B8A8_SRGB:
                case VK_FORMAT_B8G8R8A8_SRGB:
                case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                    sRGB = VK_TRUE;
                    break;
                default:
                    break;
                }

                if (database->NumPixelPipes == 1)
                {
                    __vkCmdLoadBatchHWStates(&pCmdBuffer, (0x5200 + (hwRtIndex - 1) * 8), VK_FALSE,
                        1, baseAddresses);
                }
                else
                {
                    __vkCmdLoadBatchHWStates(&pCmdBuffer, (0x5200 + (hwRtIndex - 1) * 8), VK_FALSE,
                        2, baseAddresses);
                }

                __vkCmdLoadSingleHWState(&pCmdBuffer, (0x5240 + (hwRtIndex-1)), VK_FALSE,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:30) - (0 ?
 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) ((gctUINT32) (sRGB) & ((gctUINT32) ((((1 ?
 30:30) - (0 ?
 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:0) - (0 ?
 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (pLevel->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ?
 17:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ? 17:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:20) - (0 ?
 25:20) + 1))))))) << (0 ?
 25:20))) | (((gctUINT32) ((gctUINT32) (hwPEDesc->hwFormat) & ((gctUINT32) ((((1 ?
 25:20) - (0 ?
 25:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:20) - (0 ? 25:20) + 1))))))) << (0 ? 25:20)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 28:28) - (0 ?
 28:28) + 1))))))) << (0 ?
 28:28))) | (((gctUINT32) ((gctUINT32) ((rtImage->halTiling == gcvSUPERTILED)) & ((gctUINT32) ((((1 ?
 28:28) - (0 ?
 28:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 28:28) - (0 ? 28:28) + 1))))))) << (0 ? 28:28)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:29) - (0 ?
 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (blendAttach[i].blendEnable) & ((gctUINT32) ((((1 ?
 29:29) - (0 ?
 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:26) - (0 ?
 27:26) + 1))))))) << (0 ?
 27:26))) | (((gctUINT32) ((gctUINT32) (tileMode) & ((gctUINT32) ((((1 ?
 27:26) - (0 ?
 27:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:26) - (0 ? 27:26) + 1))))))) << (0 ? 27:26))));

                if (database->REG_GeometryShader)
                {
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E28 + hwRtIndex, VK_FALSE,
                        (rtImageView->createInfo.subresourceRange.layerCount > 0) ? (uint32_t)pLevel->sliceSize : 0);
                }

                if (database->ROBUSTNESS)
                {
                    uint32_t endAddress = rtBaseAddr + (uint32_t)pLevel->sliceSize - 1;
                    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5270 + hwRtIndex, VK_FALSE, endAddress);
                }
            }
            partIndex++;
        }
    }

    if (subPass->dsAttachIndex != VK_ATTACHMENT_UNUSED)
    {
        dsImageView = fb->imageViews[subPass->dsAttachIndex];
        __VK_ASSERT(dsImageView->createInfo.subresourceRange.aspectMask
            & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
        dsImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dsImageView->createInfo.image);
        pLevel = &dsImage->pImgLevels[dsImageView->createInfo.subresourceRange.baseMipLevel];
#if __VK_ENABLETS
        pRanges = &dsImageView->createInfo.subresourceRange;
#endif
        if (gcvINVALIDTILED == tiling)
        {
            tiling = dsImage->halTiling;
        }
        else
        {
            __VK_ASSERT(tiling == dsImage->halTiling);
        }
#if __VK_ENABLETS
        /* Enable ds tile status. */
        halti5_setRtTileStatus(cmdBuf, &pCmdBuffer, dsImage, pRanges, 0);
#endif
        rtBaseAddr = dsImage->memory->devAddr;
        rtBaseAddr += (uint32_t)(dsImage->memOffset + pLevel->offset +
                                 dsImageView->createInfo.subresourceRange.baseArrayLayer * pLevel->sliceSize);

        chipGfxPipeline->regDepthConfig &= (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) ((dsImage->halTiling == gcvSUPERTILED)) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:27) - (0 ?
 27:27) + 1))))))) << (0 ?
 27:27))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 27:27) - (0 ?
 27:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:27) - (0 ? 27:27) + 1))))))) << (0 ? 27:27))));

        baseAddresses[0] = baseAddresses[1] = rtBaseAddr;

        if (database->NumPixelPipes == 1)
        {
            __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0520, VK_FALSE, 1, baseAddresses);
        }
        else
        {
            __vkCmdLoadBatchHWStates(&pCmdBuffer, 0x0520, VK_FALSE, 2, baseAddresses);
        }
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0505, VK_FALSE, (uint32_t)pLevel->stride);
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0500, VK_FALSE, chipGfxPipeline->regDepthConfig);

        if (database->REG_GeometryShader)
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E23, VK_FALSE,
                (dsImageView->createInfo.subresourceRange.layerCount > 0) ? (uint32_t)pLevel->sliceSize : 0);
        }

        if (database->ROBUSTNESS)
        {
            uint32_t endAddress = rtBaseAddr + (uint32_t)pLevel->sliceSize - 1 ;
            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0531, VK_FALSE, endAddress);
        }
    }

    colorAddressMode = (tiling == gcvLINEAR) ? 0x1 :
        (chipGfxPipeline->has16orLessBppImage ? 0x3 :
                                                0x2);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0529, VK_FALSE, (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (colorAddressMode) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:7) - (0 ?
 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 7:7) - (0 ?
 7:7) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ? 7:7)))));

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return VK_SUCCESS;
}

static HwResourceViewUsage halti5_check_resView_firstUse(
    HwResourceViewUsage *usedUsageMask,
    HwResourceViewUsage  usage
    )
{
    if (!(*usedUsageMask & usage))
    {
        *usedUsageMask |= usage;
        return usage;
    }

    return (HwResourceViewUsage)0;
}

void halti5_helper_setSamplerStates(
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
    static const struct
    {
        uint32_t samplerCtrl0Reg;
        uint32_t samplerCtrl1Reg;
        uint32_t samplerLodMaxMinReg;
        uint32_t samplerLodBiasReg;
        uint32_t samplerAnisCtrlReg;
        uint32_t texDescAddrReg;
        uint32_t textureControlAddrReg;
    }
    s_TxHwRegisters[] =
    {
        {
            0x5800, 0x5880,
            0x5900, 0x5980,
            0x5A00, 0x5600,
            0x5680
        },
        {
            0x5B00, 0x5B80,
            0x5C00, 0x5C80,
            0x5D00, 0x5700,
            0x5780
        },
    };

    __vkDeviceMemory *txDescriptorNode;
    uint32_t physical;
#if __VK_ENABLETS
    int32_t tsSlotIndex = -1;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
#endif
    uint32_t hwSamplerCtrl0 = samplerDesc->halti5.hwSamplerCtrl0;

    txDescriptorNode = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, txDesc->descriptor);

    physical = txDescriptorNode->devAddr;

    physical += TX_HW_DESCRIPTOR_MEM_SIZE * borderColorIdx;

    if (imgv)
    {
        VkFormatProperties* pFormatProperties = &g_vkFormatInfoTable[imgv->formatInfo->residentImgFormat].formatProperties;
        __vkImage* img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgv->createInfo.image);
        VkFormatFeatureFlags formatFeatureFlags = (img->createInfo.tiling == VK_IMAGE_TILING_LINEAR)
                                                ? pFormatProperties->linearTilingFeatures
                                                : pFormatProperties->optimalTilingFeatures;

        if (!(formatFeatureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            hwSamplerCtrl0 |= ((((gctUINT32) (hwSamplerCtrl0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:9) - (0 ?
 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:9) - (0 ?
 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ? 10:9)));
            hwSamplerCtrl0 |= ((((gctUINT32) (hwSamplerCtrl0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:13) - (0 ?
 14:13) + 1))))))) << (0 ?
 14:13))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 14:13) - (0 ?
 14:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:13) - (0 ? 14:13) + 1))))))) << (0 ? 14:13)));
            hwSamplerCtrl0 |= ((((gctUINT32) (hwSamplerCtrl0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:11) - (0 ?
 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:11) - (0 ?
 12:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ? 12:11)));
#if (defined(DEBUG)||defined(_DEBUG))
            if (hwSamplerCtrl0 != samplerDesc->halti5.hwSamplerCtrl0)
            {
                __VK_PRINT("texture format is not filterable, invalid input from app\n");
            }
#endif
        }
    }


    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].samplerCtrl0Reg + hwSamplerNo, VK_FALSE,
        (hwSamplerCtrl0
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (txDesc->sampleStencil) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (txDesc->fast_filter && ((samplerDesc->halti5.hwSamplerAnisVal == 0) || txDesc->isCubmap)) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))));

    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].samplerCtrl1Reg + hwSamplerNo, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (txDesc->sRGB) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))));

    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].samplerLodMaxMinReg + hwSamplerNo, VK_FALSE,
        samplerDesc->halti5.hwSamplerLodMinMax);

    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].samplerLodBiasReg + hwSamplerNo, VK_FALSE,
        samplerDesc->halti5.hwSamplerLodBias);

    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].samplerAnisCtrlReg + hwSamplerNo, VK_FALSE,
        txDesc->isCubmap ? 0 : samplerDesc->halti5.hwSamplerAnisVal);

    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].texDescAddrReg + hwSamplerNo, VK_FALSE,
        physical);

    /* Only need to enable TX256_BYTE_REQUEST when msaa and ts both exist.
       For vulkan, ts is not ready, this bit should always be disable.
    */
    __vkCmdLoadSingleHWState(commandBuffer,
        s_TxHwRegisters[txHwRegisterIdx].textureControlAddrReg + hwSamplerNo, VK_FALSE,
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
 1:1))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:2) - (0 ?
 4:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:2) - (0 ?
 4:2) + 1))))))) << (0 ?
 4:2))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 4:2) - (0 ?
 4:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:2) - (0 ? 4:2) + 1))))))) << (0 ? 4:2)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))));
#if __VK_ENABLETS
    if (imgv)
    {
        __vkImage * img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
        __vkTileStatus * tsResource = img->memory->ts;
        VkImageSubresourceRange *pRange = &imgv->createInfo.subresourceRange;
        VkBool32 anyTsEnableForMultiSlice = VK_FALSE;
        VkBool32 canTsEnable = VK_TRUE;

        uint32_t levelCount = pRange->levelCount;
        uint32_t layerCount = pRange->layerCount;
        if (pRange->layerCount == VK_REMAINING_MIP_LEVELS)
        {
            levelCount = img->createInfo.mipLevels - pRange->baseMipLevel;
        }

        if (pRange->layerCount == VK_REMAINING_ARRAY_LAYERS)
        {
            layerCount = img->createInfo.arrayLayers - pRange->baseArrayLayer;
        }

        __VK_AnyTSEnable(tsResource, pRange, &anyTsEnableForMultiSlice);
        __VK_CanTSEnable(tsResource, pRange, &canTsEnable);
        if (tsResource && anyTsEnableForMultiSlice)
        {
            /*
            ** 1, ES3.1 MSAA texture.
            ** 2, multisampled_render_to_texture has been put into shadow rendering path.
            */
            uint32_t bitsPerPixel = img->formatInfo.bitsPerBlock / img->formatInfo.partCount;
            if (((bitsPerPixel < 16) && !devCtx->database->REG_Halti5) ||
                !canTsEnable)
            {
                __VK_ASSERT(devCtx->database->REG_TextureTileStatus);

                halti5_decompressTileStatus(cmdBuf, commandBuffer, img, pRange);
                txDesc->hasTileStatus = VK_FALSE;

                if (devCtx->database->REG_BltEngine)
                {
                    __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

                    __vkCmdLoadSingleHWState(commandBuffer, 0x0E02, VK_FALSE,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

                    *(*commandBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*commandBuffer)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;


                    __vkCmdLoadSingleHWState(commandBuffer, 0x502E, VK_FALSE,
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
                }
            }
            else
            {
                txDesc->hasTileStatus = VK_TRUE;
            }
        }
        else
        {
            txDesc->hasTileStatus = VK_FALSE;
        }
    }


    tsSlotIndex = chipCommand->texTileStatusSlotIndex[hwSamplerNo];
    chipCommand->texHasTileStatus[hwSamplerNo] = txDesc->hasTileStatus;
    chipCommand->imgvWithTS[hwSamplerNo] = imgv;
    chipCommand->textureControlAddrReg[hwSamplerNo] = s_TxHwRegisters[txHwRegisterIdx].textureControlAddrReg + hwSamplerNo;

    if (txDesc->hasTileStatus)
    {
        if (tsSlotIndex != -1)
        {
            chipCommand->texTileStatusSlotDirty |= 1 << tsSlotIndex;
        }
        else
        {
            /* Mark sampler to let later BindTextureTS() assign new tsSlotIndex */
            chipCommand->txDescDirty |= 1 << hwSamplerNo;
        }
    }
    else
    {
        if (tsSlotIndex != -1)
        {
            __VK_ASSERT(tsSlotIndex < gcdSAMPLER_TS);
            chipCommand->texTileStatusSlotUser[tsSlotIndex] = -1;
            chipCommand->texTileStatusSlotIndex[hwSamplerNo] = -1;
            chipCommand->texTileStatusSlotDirty |= 1 << tsSlotIndex;
        }
    }
#endif
    return;

}

static VkResult halti5_helper_setDescSetSeperateImage(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    __vkCmdBindDescSetInfo *descSetInfo,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    PROGRAM_EXECUTABLE_PROFILE *pep = &chipPipeline->curInstance->pep;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    VkBool32 *separateBindingProgramed = chipPipeline->separateBindingProgramed;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;

    uint32_t entryIdx, j;
    uint32_t stageIdx = 0, activeStageMask;
    PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY *texEntry = VK_NULL_HANDLE;

    for (entryIdx = 0; entryIdx < progResourceSet->separatedTexTable.countOfEntries; entryIdx++)
    {
        texEntry = &progResourceSet->separatedTexTable.pTextureEntries[entryIdx];
        if (descriptorBinding->std.binding == texEntry->texBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->separatedTexTable.countOfEntries);
    __VK_ASSERT(texEntry->texBinding.set == descSetIndex);
    __VK_ASSERT(texEntry->texBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = texEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            uint32_t TxHwRegisterIdx = (stageIdx < VSC_SHADER_STAGE_PS) ? 0 : 1;

            PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING *privCombinedMapping;
            uint32_t* pctsHmEntryIdxArray = texEntry->hwMappings[stageIdx].texHwMappingList.pPctsHmEntryIdxArray;
            uint32_t privCombindMappingCount = texEntry->hwMappings[stageIdx].texHwMappingList.arraySize;

            for (j = 0; j < privCombindMappingCount; j++)
            {
                privCombinedMapping = &pep->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[pctsHmEntryIdxArray[j]];

                if (!separateBindingProgramed[(privCombinedMapping->pctsHmEntryIndex * VSC_MAX_SHADER_STAGE_COUNT) + stageIdx])
                {
                    uint32_t samplerArrayIdx;
                    uint32_t hwSamplerSlotIdx = 0;
                    uint32_t samplerEnd, texEnd;
                    VSC_SHADER_RESOURCE_BINDING *vscSamplerBinding = privCombinedMapping->samplerSubBinding.pResBinding;
                    __vkDescriptorSet *samplerDescSet = descSetInfo->descSets[vscSamplerBinding->set];
                    __vkDescriptorSetLayoutBinding *samplerBinding = &samplerDescSet->descSetLayout->binding[vscSamplerBinding->binding];

                    samplerEnd = privCombinedMapping->samplerSubBinding.startIdxOfSubArray +
                                 privCombinedMapping->samplerSubBinding.subArraySize;

                    for (samplerArrayIdx = privCombinedMapping->samplerSubBinding.startIdxOfSubArray;
                         samplerArrayIdx < samplerEnd;
                         samplerArrayIdx++)
                    {
                        uint32_t texArrayIdx;
                        __vkDescriptorResourceRegion curSamplerRegion;
                        __vkSampler **samplers;
                        __vkSampler *sampler;
                        halti5_sampler *chipSampler;

                        __vk_utils_region_mad(&curSamplerRegion, &samplerBinding->perElementSize, samplerArrayIdx, &samplerBinding->offset);
                        samplers = (__vkSampler **)((uint8_t*)samplerDescSet->samplers + curSamplerRegion.sampler);
                        sampler = samplers[0];
                        chipSampler = (halti5_sampler *)sampler->chipPriv;

                        texEnd = privCombinedMapping->texSubBinding.startIdxOfSubArray +
                                 privCombinedMapping->texSubBinding.subArraySize;

                        for (texArrayIdx = privCombinedMapping->texSubBinding.startIdxOfSubArray;
                             texArrayIdx < texEnd;
                             texArrayIdx++)
                        {
                            __vkDescriptorResourceRegion curTexRegion;
                            __vkDescriptorResourceInfo *resInfo;
                            __vkImageView *imgv;
                            halti5_imageView *chipImgv;

                            SHADER_SAMPLER_SLOT_MAPPING *hwMapping = &privCombinedMapping->pSamplerSlotMapping[hwSamplerSlotIdx];
                            uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

                            __vk_utils_region_mad(&curTexRegion, &descriptorBinding->perElementSize, texArrayIdx, &descriptorBinding->offset);
                            resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curTexRegion.resource);

                            if(resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                break;
                            }

                            imgv = resInfo->u.imageInfo.imageView;
                            chipImgv = (halti5_imageView *)imgv->chipPriv;

                            chipCommandBuffer->newResourceViewUsageMask |=
                                halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_TX);

                            /* if board color will be used, must be valid combination */
                            __VK_ASSERT(((sampler->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                                (sampler->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                                (sampler->createInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
                                ((chipImgv->txDesc[0].invalidBorderColorMask & (1 << sampler->createInfo.borderColor)) == 0));

                             (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                               commandBuffer,
                                                                               imgv,
                                                                               TxHwRegisterIdx,
                                                                               &chipImgv->txDesc[0],
                                                                               &chipSampler->samplerDesc,
                                                                               sampler->createInfo.borderColor,
                                                                               hwSamplerNo,
                                                                               hints->shaderConfigData);

                            if (privCombinedMapping->ppExtraSamplerArray && privCombinedMapping->ppExtraSamplerArray[hwSamplerSlotIdx])
                            {
                                SHADER_PRIV_SAMPLER_ENTRY *privEntry = privCombinedMapping->ppExtraSamplerArray[hwSamplerSlotIdx];
                                uint32_t extraSamplerNo = privEntry->pSampler->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];
                                __VK_ASSERT(privEntry->commonPrivm.privmFlag ==  VSC_LIB_LINK_TYPE_RESOURCE);


                                (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                                  commandBuffer,
                                                                                  imgv,
                                                                                  TxHwRegisterIdx,
                                                                                  &chipImgv->txDesc[1],
                                                                                  &chipSampler->samplerDesc,
                                                                                  sampler->createInfo.borderColor,
                                                                                  extraSamplerNo,
                                                                                  hints->shaderConfigData);
                            }
                            hwSamplerSlotIdx++;

                        }
                    }

                    separateBindingProgramed[(privCombinedMapping->pctsHmEntryIndex * VSC_MAX_SHADER_STAGE_COUNT) + stageIdx] = VK_TRUE;
                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    return VK_SUCCESS;
}

static VkResult halti5_helper_setDescSetSeperateSampler(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    __vkCmdBindDescSetInfo *descSetInfo,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    PROGRAM_EXECUTABLE_PROFILE *pep = &chipPipeline->curInstance->pep;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    VkBool32 *separateBindingProgramed = chipPipeline->separateBindingProgramed;

    uint32_t entryIdx, k;
    uint32_t stageIdx = 0, activeStageMask;
    PROG_VK_SEPARATED_SAMPLER_TABLE_ENTRY *samplerEntry = VK_NULL_HANDLE;

    for (entryIdx = 0; entryIdx < progResourceSet->separatedSamplerTable.countOfEntries; entryIdx++)
    {
        samplerEntry = &progResourceSet->separatedSamplerTable.pSamplerEntries[entryIdx];
        if (descriptorBinding->std.binding == samplerEntry->samplerBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->separatedSamplerTable.countOfEntries);
    __VK_ASSERT(samplerEntry->samplerBinding.set == descSetIndex);
    __VK_ASSERT(samplerEntry->samplerBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = samplerEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            uint32_t TxHwRegisterIdx = (stageIdx < VSC_SHADER_STAGE_PS) ? 0 : 1;

            PROG_VK_PRIV_COMB_TEX_SAMP_HW_MAPPING *privCombinedMapping;
            uint32_t* pctsHmEntryIdxArray = samplerEntry->hwMappings[stageIdx].samplerHwMappingList.pPctsHmEntryIdxArray;
            uint32_t privCombindMappingCount = samplerEntry->hwMappings[stageIdx].samplerHwMappingList.arraySize;

            for (k = 0; k < privCombindMappingCount; k++)
            {
                privCombinedMapping = &pep->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[pctsHmEntryIdxArray[k]];

                if (!separateBindingProgramed[(privCombinedMapping->pctsHmEntryIndex * VSC_MAX_SHADER_STAGE_COUNT) + stageIdx])
                {
                    uint32_t samplerArrayIdx;
                    uint32_t hwSamplerSlotIdx = 0;
                    uint32_t samplerEnd, texEnd;
                    VSC_SHADER_RESOURCE_BINDING *vscTexBinding = privCombinedMapping->texSubBinding.pResBinding;
                    __vkDescriptorSet *texDescSet = descSetInfo->descSets[vscTexBinding->set];
                    __vkDescriptorSetLayoutBinding *texBinding = &texDescSet->descSetLayout->binding[vscTexBinding->binding];

                    samplerEnd = privCombinedMapping->samplerSubBinding.startIdxOfSubArray +
                                 privCombinedMapping->samplerSubBinding.subArraySize;

                    for (samplerArrayIdx = privCombinedMapping->samplerSubBinding.startIdxOfSubArray;
                         samplerArrayIdx < samplerEnd;
                         samplerArrayIdx++)
                    {
                        uint32_t texArrayIdx;
                        __vkDescriptorResourceRegion curSamplerRegion;
                        __vkSampler **samplers;
                        __vkSampler *sampler;
                        halti5_sampler *chipSampler;
                        __vk_utils_region_mad(&curSamplerRegion, &descriptorBinding->perElementSize, samplerArrayIdx, &descriptorBinding->offset);
                        samplers = (__vkSampler **)((uint8_t*)descSet->samplers + curSamplerRegion.sampler);
                        sampler = samplers[0];
                        chipSampler = (halti5_sampler *)sampler->chipPriv;

                        texEnd = privCombinedMapping->texSubBinding.startIdxOfSubArray +
                                 privCombinedMapping->texSubBinding.subArraySize;

                        for (texArrayIdx = privCombinedMapping->texSubBinding.startIdxOfSubArray;
                             texArrayIdx < texEnd;
                             texArrayIdx++)
                        {
                            __vkDescriptorResourceRegion curTexRegion;
                            __vkDescriptorResourceInfo *resInfo;
                            __vkImageView *imgv;
                            halti5_imageView *chipImgv;

                            SHADER_SAMPLER_SLOT_MAPPING *hwMapping = &privCombinedMapping->pSamplerSlotMapping[hwSamplerSlotIdx];
                            uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

                            __vk_utils_region_mad(&curTexRegion, &texBinding->perElementSize, texArrayIdx, &texBinding->offset);
                            resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)texDescSet->resInfos + curTexRegion.resource);

                            if (resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                            {
                                break;
                            }

                            imgv = resInfo->u.imageInfo.imageView;
                            chipImgv = (halti5_imageView *)imgv->chipPriv;
                            /* if board color will be used, must be valid combination */
                            __VK_ASSERT(((sampler->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                                (sampler->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                                (sampler->createInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
                                ((chipImgv->txDesc[0].invalidBorderColorMask & (1 << sampler->createInfo.borderColor)) == 0));

                            (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                               commandBuffer,
                                                                               imgv,
                                                                               TxHwRegisterIdx,
                                                                               &chipImgv->txDesc[0],
                                                                               &chipSampler->samplerDesc,
                                                                               sampler->createInfo.borderColor,
                                                                               hwSamplerNo,
                                                                               hints->shaderConfigData);

                            if (privCombinedMapping->ppExtraSamplerArray && privCombinedMapping->ppExtraSamplerArray[hwSamplerSlotIdx])
                            {
                                SHADER_PRIV_SAMPLER_ENTRY *privEntry = privCombinedMapping->ppExtraSamplerArray[hwSamplerSlotIdx];
                                uint32_t extraSamplerNo = privEntry->pSampler->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];
                                __VK_ASSERT(privEntry->commonPrivm.privmFlag ==  VSC_LIB_LINK_TYPE_RESOURCE);

                                (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                                  commandBuffer,
                                                                                  imgv,
                                                                                  TxHwRegisterIdx,
                                                                                  &chipImgv->txDesc[1],
                                                                                  &chipSampler->samplerDesc,
                                                                                  sampler->createInfo.borderColor,
                                                                                  extraSamplerNo,
                                                                                  hints->shaderConfigData);
                            }
                            hwSamplerSlotIdx++;
                        }
                    }
                    separateBindingProgramed[(privCombinedMapping->pctsHmEntryIndex * VSC_MAX_SHADER_STAGE_COUNT) + stageIdx] = VK_TRUE;
                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    return VK_SUCCESS;
}

static VkResult halti5_helper_setDescSetCombinedImageSampler(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    uint32_t entryIdx, arrayIdx;
    uint32_t stageIdx = 0, activeStageMask;
    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY *samplerEntry = VK_NULL_HANDLE;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;

    for (entryIdx = 0; entryIdx < progResourceSet->combinedSampTexTable.countOfEntries; entryIdx++)
    {
        samplerEntry = &progResourceSet->combinedSampTexTable.pCombTsEntries[entryIdx];
        if (descriptorBinding->std.binding == samplerEntry->combTsBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->combinedSampTexTable.countOfEntries);
    __VK_ASSERT(samplerEntry->combTsBinding.set == descSetIndex);
    __VK_ASSERT(samplerEntry->combTsBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = samplerEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            uint32_t TxHwRegisterIdx = (stageIdx < VSC_SHADER_STAGE_PS) ? 0 : 1;
            SHADER_SAMPLER_SLOT_MAPPING *hwMapping = &samplerEntry->hwMappings[stageIdx].samplerMapping;
            uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

            for (arrayIdx = 0; arrayIdx < descriptorBinding->std.descriptorCount; arrayIdx++)
            {
                __vkDescriptorResourceRegion curDescRegion;
                __vkDescriptorResourceInfo *resInfo;
                __vkSampler **samplers;
                __vkImageView *imgv;
                __vkImage *img;
                __vkSampler *sampler;
                halti5_sampler *chipSampler;
                halti5_imageView *chipImgv;

                __vk_utils_region_mad(&curDescRegion, &descriptorBinding->perElementSize, arrayIdx, &descriptorBinding->offset);
                resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curDescRegion.resource);
                samplers = (__vkSampler **)((uint8_t*)descSet->samplers + curDescRegion.sampler);

                if (resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    break;
                }

                imgv = resInfo->u.imageInfo.imageView;
                img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgv->createInfo.image);
                sampler = samplers[0];
                chipImgv = (halti5_imageView *)imgv->chipPriv;
                chipSampler = (halti5_sampler *)sampler->chipPriv;
                /* if board color will be used, must be valid combination */
                __VK_ASSERT(((sampler->createInfo.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                    (sampler->createInfo.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) &&
                    (sampler->createInfo.addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
                    ((chipImgv->txDesc[0].invalidBorderColorMask & (1 << sampler->createInfo.borderColor)) == 0));

                chipCommandBuffer->newResourceViewUsageMask |=
                    halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_TX);

                (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                  commandBuffer,
                                                                  imgv,
                                                                  TxHwRegisterIdx,
                                                                  &chipImgv->txDesc[0],
                                                                  &chipSampler->samplerDesc,
                                                                  sampler->createInfo.borderColor,
                                                                  (hwSamplerNo + arrayIdx),
                                                                  hints->shaderConfigData);

                if (samplerEntry->hwMappings[stageIdx].ppExtraSamplerArray && samplerEntry->hwMappings[stageIdx].ppExtraSamplerArray[arrayIdx])
                {
                    SHADER_PRIV_SAMPLER_ENTRY *privEntry = samplerEntry->hwMappings[stageIdx].ppExtraSamplerArray[arrayIdx];
                    uint32_t extraSamplerNo = privEntry->pSampler->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];
                    __VK_ASSERT(privEntry->commonPrivm.privmFlag ==  VSC_LIB_LINK_TYPE_RESOURCE);

                    (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                      commandBuffer,
                                                                      imgv,
                                                                      TxHwRegisterIdx,
                                                                      &chipImgv->txDesc[1],
                                                                      &chipSampler->samplerDesc,
                                                                      sampler->createInfo.borderColor,
                                                                      extraSamplerNo,
                                                                      hints->shaderConfigData);
                }

                if (samplerEntry->pTextureSize[stageIdx])
                {
                    SHADER_PRIV_CONSTANT_ENTRY *privEntry = samplerEntry->pTextureSize[stageIdx];

                    if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                    {
                        uint32_t data[4] = {0};
                        uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                        uint32_t hwConstRegAddr = (hints->hwConstRegBases[stageIdx] >> 2) + (hwConstRegNoForSize * 4)
                                                + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;

                        __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_TEXTURE_SIZE);

                        data[0] = chipImgv->txDesc[0].baseWidth;
                        data[1] = chipImgv->txDesc[0].baseHeight;
                        data[2] = chipImgv->txDesc[0].baseDepth;
                        data[3] = chipImgv->txDesc[0].baseSlice;
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4, data);
                    }
                }

                if (samplerEntry->pLodMinMax[stageIdx])
                {
                    SHADER_PRIV_CONSTANT_ENTRY *privEntry = samplerEntry->pLodMinMax[stageIdx];

                    if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                    {
                        uint32_t data[4] = {0};
                        uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                        uint32_t hwConstRegAddr = (hints->hwConstRegBases[stageIdx] >> 2) + (hwConstRegNoForSize * 4)
                                                + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;

                        __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_LOD_MIN_MAX);

                        data[0] = (gctINT)sampler->createInfo.minLod;
                        data[1] = (gctINT)sampler->createInfo.maxLod;
                        data[2] = (sampler->createInfo.minFilter == VK_FILTER_NEAREST) ? 0 : 1;
                        data[3] = 0;
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4, data);
                    }
                }

                if (samplerEntry->pLevelsSamples[stageIdx])
                {
                    SHADER_PRIV_CONSTANT_ENTRY *privEntry = samplerEntry->pLevelsSamples[stageIdx];

                    if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                    {
                        uint32_t data[4] = {0};
                        uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                        uint32_t hwConstRegAddr = (hints->hwConstRegBases[stageIdx] >> 2) + (hwConstRegNoForSize * 4)
                                                + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;

                        __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_LEVELS_SAMPLES);

                        data[0] = (gctINT)imgv->createInfo.subresourceRange.levelCount;
                        data[1] = (gctINT)img->sampleInfo.product;
                        data[2] = 0;
                        data[3] = 0;
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4, data);
                    }
                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    return VK_SUCCESS;
}

static VkResult halti5_helper_setDescSetUniformTexelBuffer(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    uint32_t entryIdx, arrayIdx;
    uint32_t stageIdx = 0, activeStageMask;
    PROG_VK_UNIFORM_TEXEL_BUFFER_TABLE_ENTRY *samplerBufEntry = VK_NULL_HANDLE;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;

    for (entryIdx = 0; entryIdx < progResourceSet->uniformTexBufTable.countOfEntries; entryIdx++)
    {
        samplerBufEntry = &progResourceSet->uniformTexBufTable.pUtbEntries[entryIdx];
        if (descriptorBinding->std.binding == samplerBufEntry->utbBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->uniformTexBufTable.countOfEntries);
    __VK_ASSERT(samplerBufEntry->utbBinding.set == descSetIndex);
    __VK_ASSERT(samplerBufEntry->utbBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = samplerBufEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            uint32_t TxHwRegisterIdx = (stageIdx < VSC_SHADER_STAGE_PS) ? 0 : 1;
            SHADER_SAMPLER_SLOT_MAPPING *hwMapping = &samplerBufEntry->hwMappings[stageIdx].s.samplerMapping;
            uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

            __VK_ASSERT(descriptorBinding->std.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
            for (arrayIdx = 0; arrayIdx < descriptorBinding->std.descriptorCount; arrayIdx++)
            {
                __vkDescriptorResourceRegion curDescRegion;
                __vkDescriptorResourceInfo *resInfo;
                __vkBufferView *bufv;
                halti5_bufferView *chipBufv;

                __vk_utils_region_mad(&curDescRegion, &descriptorBinding->perElementSize, arrayIdx, &descriptorBinding->offset);
                resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curDescRegion.resource);

                if(resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    break;
                }

                bufv = resInfo->u.bufferView;
                chipBufv = (halti5_bufferView *)bufv->chipPriv;

                (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                  commandBuffer,
                                                                  VK_NULL_HANDLE,
                                                                  TxHwRegisterIdx,
                                                                  chipBufv->txDesc,
                                                                  &chipBufv->samplerDesc,
                                                                  0,
                                                                  (hwSamplerNo + arrayIdx),
                                                                  hints->shaderConfigData);
                chipCommandBuffer->newResourceViewUsageMask |=
                    halti5_check_resView_firstUse(&chipBufv->usedUsageMask, HW_RESOURCEVIEW_USAGE_TX);

                if (samplerBufEntry->pTextureSize[stageIdx])
                {
                    SHADER_PRIV_CONSTANT_ENTRY *privEntry = samplerBufEntry->pTextureSize[stageIdx];

                    if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                    {
                        uint32_t data[4] = {0};
                        uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                        uint32_t hwConstRegAddr = (hints->hwConstRegBases[stageIdx] >> 2) + (hwConstRegNoForSize * 4)
                                                + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;

                        __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_TEXTURE_SIZE);

                        data[0] = chipBufv->txDesc[0].baseWidth;
                        data[1] = chipBufv->txDesc[0].baseHeight;
                        data[2] = chipBufv->txDesc[0].baseDepth;
                        data[3] = chipBufv->txDesc[0].baseSlice;
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4, data);
                    }
                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    return VK_SUCCESS;
}

static VkResult halti5_helper_setDescSetInputAttach(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet
    )
{
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    uint32_t entryIdx, arrayIdx;
    uint32_t arraySize;
    uint32_t stageIdx = 0, activeStageMask;

    PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY *inputAttachEntry = VK_NULL_HANDLE;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;

    for (entryIdx = 0; entryIdx < progResourceSet->inputAttachmentTable.countOfEntries; entryIdx++)
    {
        inputAttachEntry = &progResourceSet->inputAttachmentTable.pIaEntries[entryIdx];

        if (descriptorBinding->std.binding == inputAttachEntry->iaBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->inputAttachmentTable.countOfEntries);
    __VK_ASSERT(inputAttachEntry->iaBinding.set == descSetIndex);
    __VK_ASSERT(inputAttachEntry->iaBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = inputAttachEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            PROG_VK_INPUT_ATTACHMENT_HW_MAPPING *iaHwMapping = &inputAttachEntry->hwMappings[stageIdx];
            SHADER_UAV_SLOT_MAPPING *hwMapping = &iaHwMapping->uavMapping;
            uint32_t hwConstRegAddrBase = hints->hwConstRegBases[stageIdx];
            uint32_t TxHwRegisterIdx = (stageIdx < VSC_SHADER_STAGE_PS) ? 0 : 1;

            __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR ||
                        hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_SAMPLER);
            __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

            if (hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR)
            {
                arraySize = descriptorBinding->std.descriptorCount;
            }
            else
            {
                arraySize = descriptorBinding->std.descriptorCount;
            }
            __VK_ASSERT(arraySize);

            for (arrayIdx = 0; arrayIdx < arraySize; arrayIdx++)
            {
                __vkDescriptorResourceRegion curRegion;
                __vkDescriptorResourceInfo *resInfo;
                __vkImageView *imgv;
                halti5_imageView *chipImgv;
                __vkImage *img;

                __vk_utils_region_mad(&curRegion, &descriptorBinding->perElementSize, arrayIdx, &descriptorBinding->offset);
                resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curRegion.resource);

                if (resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    break;
                }

                imgv = resInfo->u.imageInfo.imageView;
                img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
                chipImgv = (halti5_imageView *)imgv->chipPriv;

                if (img->createInfo.samples > VK_SAMPLE_COUNT_1_BIT)
                {
                    uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

                    (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                      commandBuffer,
                                                                      VK_NULL_HANDLE,
                                                                      TxHwRegisterIdx,
                                                                      &chipImgv->txDesc[0],
                                                                      &chipImgv->samplerDesc,
                                                                      0,
                                                                      (hwSamplerNo + arrayIdx),
                                                                      hints->shaderConfigData);
                    chipCommandBuffer->newResourceViewUsageMask |=
                        halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_TX);

                    if (iaHwMapping->ppExtraSamplerArray && iaHwMapping->ppExtraSamplerArray[arrayIdx])
                    {
                        hwSamplerNo = iaHwMapping->ppExtraSamplerArray[arrayIdx]->pSampler->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

                        (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                          commandBuffer,
                                                                          VK_NULL_HANDLE,
                                                                          TxHwRegisterIdx,
                                                                          &chipImgv->txDesc[1],
                                                                          &chipImgv->samplerDesc,
                                                                          0,
                                                                          (hwSamplerNo + arrayIdx),
                                                                          hints->shaderConfigData);
                    }
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
                    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                        {
                            uint32_t hwSamplerNo = hwMapping->hwSamplerSlot + hints->samplerBaseOffset[stageIdx];

                            (*chipModule->minorTable.helper_setSamplerStates)(cmdBuf,
                                                                              commandBuffer,
                                                                              VK_NULL_HANDLE,
                                                                              TxHwRegisterIdx,
                                                                              chipImgv->txDesc,
                                                                              &chipImgv->samplerDesc,
                                                                              0,
                                                                              (hwSamplerNo + arrayIdx),
                                                                              hints->shaderConfigData);
                            chipCommandBuffer->newResourceViewUsageMask |=
                                halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_TX);
                        }
                        break;
                    default:
                        {
                            uint32_t hwConstRegNo;
                            uint32_t hwConstRegAddr;

                            hwConstRegNo = hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo;
                            hwConstRegAddr = (hwConstRegAddrBase >> 2) + (hwConstRegNo * 4)
                                + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;

                            chipCommandBuffer->newResourceViewUsageMask |=
                                halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_SH);

                            __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4,
                                chipImgv->imgDesc[0].imageInfo);

                            if (inputAttachEntry->pImageSize[stageIdx])
                            {
                                SHADER_PRIV_CONSTANT_ENTRY *privEntry = inputAttachEntry->pImageSize[stageIdx];

                                if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                                {
                                    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
                                    __vkImageLevel *pLevel = &img->pImgLevels[imgv->createInfo.subresourceRange.baseMipLevel];
                                    uint32_t data[4];
                                    uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                                    uint32_t hwConstRegAddrForSize = (hwConstRegAddrBase >> 2) + (hwConstRegNoForSize * 4)
                                        + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                                    __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_IMAGE_SIZE);
                                    data[0] = pLevel->requestW;
                                    data[1] = pLevel->requestH;
                                    data[2] = pLevel->requestD;
                                    data[3] = (uint32_t)pLevel->sliceSize;
                                    __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddrForSize + (arrayIdx * 4), VK_FALSE, 4,
                                        data);
                                }
                            }

                            if (inputAttachEntry->pExtraLayer[stageIdx])
                            {
                                SHADER_PRIV_UAV_ENTRY* privEntry = inputAttachEntry->pExtraLayer[stageIdx];
                                uint32_t hwConstRegNoForExtraLayer = privEntry->pBuffer->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo;
                                uint32_t hwConstRegAddrForExtraLayer = (hwConstRegAddrBase >> 2) + (hwConstRegNoForExtraLayer * 4)
                                    + privEntry->pBuffer->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
                                __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_EXTRA_UAV_LAYER);
                                __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddrForExtraLayer + (arrayIdx * 4), VK_FALSE, 4,
                                    chipImgv->imgDesc[1].imageInfo);
                            }
                        }
                        break;
                    }
                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    return VK_SUCCESS;
}



static VkResult halti5_helper_setDescSetStorage(
    __vkCommandBuffer *cmdBuf,
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet,
    uint32_t *dynamicOffsets,
    uint32_t *dynamicOffsetIndex
    )
{
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;
    uint32_t entryIdx, arrayIdx;
    uint32_t stageIdx = 0, activeStageMask;

    PROG_VK_STORAGE_TABLE_ENTRY *storageEntry = VK_NULL_HANDLE;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;

    for (entryIdx = 0; entryIdx < progResourceSet->storageTable.countOfEntries; entryIdx++)
    {
        storageEntry = &progResourceSet->storageTable.pStorageEntries[entryIdx];

        if (descriptorBinding->std.binding == storageEntry->storageBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->storageTable.countOfEntries);
    __VK_ASSERT(storageEntry->storageBinding.set == descSetIndex);
    __VK_ASSERT(storageEntry->storageBinding.arraySize == descriptorBinding->std.descriptorCount);
    activeStageMask = storageEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            SHADER_UAV_SLOT_MAPPING *hwMapping = &storageEntry->hwMappings[stageIdx];
            uint32_t hwConstRegNo;
            uint32_t hwConstRegAddrBase = hints->hwConstRegBases[stageIdx];
            uint32_t hwConstRegAddr;

            __VK_ASSERT(hwMapping->hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
            __VK_ASSERT(hwMapping->hwLoc.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

            hwConstRegNo = hwMapping->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo;
            hwConstRegAddr = (hwConstRegAddrBase >> 2) + (hwConstRegNo * 4)
                + hwMapping->hwLoc.pHwDirectAddrBase->firstValidHwChannel;

            for (arrayIdx = 0; arrayIdx < descriptorBinding->std.descriptorCount; arrayIdx++)
            {
                __vkDescriptorResourceRegion curRegion;
                __vkDescriptorResourceInfo *resInfo;

                __vk_utils_region_mad(&curRegion, &descriptorBinding->perElementSize, arrayIdx, &descriptorBinding->offset);
                resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curRegion.resource);

                if (resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    break;
                }

                switch (descriptorBinding->std.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    {
                        __vkImageView *imgv;
                        halti5_imageView *chipImgv;
                        imgv = resInfo->u.imageInfo.imageView;
                        chipImgv = (halti5_imageView *)imgv->chipPriv;

                        chipCommandBuffer->newResourceViewUsageMask |=
                            halti5_check_resView_firstUse(&chipImgv->usedUsageMask, HW_RESOURCEVIEW_USAGE_SH);
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4,
                            chipImgv->imgDesc[0].imageInfo);

                        if (storageEntry->pImageSize[stageIdx])
                        {
                            SHADER_PRIV_CONSTANT_ENTRY *privEntry = storageEntry->pImageSize[stageIdx];

                            if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                            {
                                __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
                                __vkImageLevel *pLevel = &img->pImgLevels[imgv->createInfo.subresourceRange.baseMipLevel];
                                uint32_t data[4];
                                uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                                uint32_t hwConstRegAddrForSize = (hwConstRegAddrBase >> 2) + (hwConstRegNoForSize * 4)
                                                        + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                                __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_IMAGE_SIZE);
                                data[0] = pLevel->requestW;
                                data[1] = pLevel->requestH;
                                data[2] = pLevel->requestD;
                                data[3] = (uint32_t)pLevel->sliceSize;
                                __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddrForSize + (arrayIdx * 4), VK_FALSE, 4,
                                                         data);
                            }
                        }

                        if (storageEntry->pExtraLayer[stageIdx])
                        {
                            SHADER_PRIV_UAV_ENTRY* privEntry = storageEntry->pExtraLayer[stageIdx];
                            uint32_t hwConstRegNoForExtraLayer = privEntry->pBuffer->hwLoc.pHwDirectAddrBase->hwLoc.hwRegNo;
                            uint32_t hwConstRegAddrForExtraLayer = (hwConstRegAddrBase >> 2) + (hwConstRegNoForExtraLayer * 4)
                                                    + privEntry->pBuffer->hwLoc.pHwDirectAddrBase->firstValidHwChannel;
                            __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_EXTRA_UAV_LAYER);
                            __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddrForExtraLayer + (arrayIdx * 4), VK_FALSE, 4,
                                chipImgv->imgDesc[1].imageInfo);
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    {
                        __vkBufferView *bufv;
                        halti5_bufferView *chipBufv;
                        bufv = resInfo->u.bufferView;
                        chipBufv = (halti5_bufferView *)bufv->chipPriv;

                        chipCommandBuffer->newResourceViewUsageMask |=
                            halti5_check_resView_firstUse(&chipBufv->usedUsageMask, HW_RESOURCEVIEW_USAGE_SH);
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 4,
                            chipBufv->imgDesc[0].imageInfo);

                        if (storageEntry->pImageSize[stageIdx])
                        {
                            SHADER_PRIV_CONSTANT_ENTRY *privEntry = storageEntry->pImageSize[stageIdx];

                            if (arrayIdx < privEntry->u.pSubCBMapping->subArrayRange)
                            {
                                uint32_t size;
                                __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, bufv->createInfo.buffer);
                                uint32_t hwConstRegNoForSize = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.hwRegNo;
                                uint32_t hwConstRegAddr = (hwConstRegAddrBase >> 2) + (hwConstRegNoForSize * 4)
                                                        + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel;
                                __VK_ASSERT(privEntry->commonPrivm.privmFlag == SHS_PRIV_CONSTANT_FLAG_IMAGE_SIZE);

                                size = (bufv->createInfo.range == VK_WHOLE_SIZE)
                                    ? (uint32_t)(buf->createInfo.size - bufv->createInfo.offset)
                                    : (uint32_t)bufv->createInfo.range;
                                size /= (bufv->formatInfo.bitsPerBlock >> 3);
                                __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, 1, &size);
                            }
                        }
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    {
                        uint32_t physical;
                        uint32_t data[4];
                        uint32_t dataCount = 0;
                        __vkBuffer *buf = resInfo->u.bufferInfo.buffer;
                        physical = buf->memory->devAddr;
                        physical += (uint32_t)resInfo->u.bufferInfo.offset;
                        data[dataCount++] = physical;
                        if (devCtx->database->ROBUSTNESS)
                        {
                            data[dataCount++] = physical;
                            data[dataCount++] = physical + (uint32_t)buf->memReq.size - 1;
                        }
                        if (hwMapping->accessMode == SHADER_UAV_ACCESS_MODE_RESIZABLE)
                        {
                            VkDeviceSize ssboSize = (resInfo->u.bufferInfo.range == VK_WHOLE_SIZE) ?
                                (buf->createInfo.size - resInfo->u.bufferInfo.offset) : resInfo->u.bufferInfo.range;
                            data[dataCount++] = ((uint32_t)ssboSize- hwMapping->sizeInByte)/hwMapping->u.sizableEleSize;
                        }
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, dataCount, data);
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    {
                        uint32_t data[4];
                        uint32_t dataCount = 0;
                        uint32_t physical;
                        uint32_t offset = (uint32_t)resInfo->u.bufferInfo.offset + dynamicOffsets[(*dynamicOffsetIndex) + arrayIdx];
                        __vkBuffer *buf = resInfo->u.bufferInfo.buffer;
                        physical = buf->memory->devAddr;
                        physical += offset;
                        data[dataCount++] = physical;
                        if (devCtx->database->ROBUSTNESS)
                        {
                            data[dataCount++] = physical;
                            data[dataCount++] = physical + (uint32_t)buf->memReq.size - 1;
                        }
                       if (hwMapping->accessMode == SHADER_UAV_ACCESS_MODE_RESIZABLE)
                        {
                            VkDeviceSize ssboSize = (resInfo->u.bufferInfo.range == VK_WHOLE_SIZE) ?
                                (buf->createInfo.size - (VkDeviceSize)offset) : resInfo->u.bufferInfo.range;
                            data[dataCount++] = ((uint32_t)ssboSize- hwMapping->sizeInByte)/hwMapping->u.sizableEleSize;
                        }
                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, dataCount, data);
                    }
                    break;
                default:
                    __VK_ASSERT(!"Wrong descriptor type, which should not fall in storage set function");
                    break;

                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    if (descriptorBinding->std.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
    {
        (*dynamicOffsetIndex) += descriptorBinding->std.descriptorCount;
    }

    return VK_SUCCESS;
}

static VkResult halti5_helper_setDescSetUniformBuffer(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkPipeline *pip,
    uint32_t descSetIndex,
    __vkDescriptorSet *descSet,
    __vkDescriptorSetLayoutBinding *descriptorBinding,
    PROG_VK_RESOURCE_SET *progResourceSet,
    uint32_t *dynamicOffsets,
    uint32_t *dynamicOffsetIndex
    )
{
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    struct _gcsHINT *hints = &chipPipeline->curInstance->hwStates.hints;

    uint32_t  entryIdx, arrayIdx;
    uint32_t stageIdx = 0, activeStageMask;
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY *uniformBufferEntry = VK_NULL_HANDLE;

    for (entryIdx = 0; entryIdx < progResourceSet->uniformBufferTable.countOfEntries; entryIdx++)
    {
        uniformBufferEntry = &progResourceSet->uniformBufferTable.pUniformBufferEntries[entryIdx];
        if (descriptorBinding->std.binding == uniformBufferEntry->ubBinding.binding)
        {
            break;
        }
    }

    __VK_ASSERT(entryIdx < progResourceSet->uniformBufferTable.countOfEntries);
    __VK_ASSERT(uniformBufferEntry->ubBinding.set == descSetIndex);
    __VK_ASSERT(uniformBufferEntry->ubBinding.arraySize == descriptorBinding->std.descriptorCount);

    activeStageMask = uniformBufferEntry->activeStageMask;
    while (activeStageMask)
    {
        if (activeStageMask & (1 << stageIdx))
        {
            SHADER_CONSTANT_HW_LOCATION_MAPPING *hwMapping = &uniformBufferEntry->hwMappings[stageIdx];
            uint32_t hwConstRegNo;
            uint32_t hwConstRegAddrBase = hints->hwConstRegBases[stageIdx];
            uint32_t hwConstRegAddr;

            __VK_ASSERT(hwMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_MEMORY);
            __VK_ASSERT(hwMapping->hwLoc.memAddr.hwMemAccessMode == SHADER_HW_MEM_ACCESS_MODE_DIRECT_MEM_ADDR);
            __VK_ASSERT(hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->hwAccessMode == SHADER_HW_ACCESS_MODE_REGISTER);

            hwConstRegNo = hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->hwLoc.hwRegNo;
            hwConstRegAddr = (hwConstRegAddrBase >> 2) + (hwConstRegNo * 4) +
                             hwMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase->firstValidHwChannel;

            for (arrayIdx = 0; arrayIdx < descriptorBinding->std.descriptorCount; arrayIdx++)
            {
                __vkDescriptorResourceRegion curRegion;
                __vkDescriptorResourceInfo *resInfo;

                __vk_utils_region_mad(&curRegion, &descriptorBinding->perElementSize, arrayIdx, &descriptorBinding->offset);
                resInfo = (__vkDescriptorResourceInfo *)((uint8_t*)descSet->resInfos + curRegion.resource);

                if (resInfo->type == __VK_DESC_RESOURCE_INVALID_INFO)
                {
                    break;
                }

                switch (descriptorBinding->std.descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    {
                        uint32_t physical;
                        uint32_t data[3];
                        uint32_t dataCount = 0;
                        __vkBuffer *buf = resInfo->u.bufferInfo.buffer;
                        physical = buf->memory->devAddr;
                        physical += (uint32_t)resInfo->u.bufferInfo.offset;
                        data[dataCount++] = physical;
                        if (devCtx->database->ROBUSTNESS)
                        {
                            data[dataCount++] = physical;
                            data[dataCount++] = physical + (uint32_t)buf->memReq.size - 1;
                        }

                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, dataCount, data);
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    {
                        uint32_t physical;
                        uint32_t data[3];
                        uint32_t dataCount = 0;
                        __vkBuffer *buf = resInfo->u.bufferInfo.buffer;
                        physical = buf->memory->devAddr;
                        physical += (uint32_t)resInfo->u.bufferInfo.offset + dynamicOffsets[(*dynamicOffsetIndex) + arrayIdx];
                        data[dataCount++] = physical;
                        if (devCtx->database->ROBUSTNESS)
                        {
                            data[dataCount++] = physical;
                            data[dataCount++] = physical + (uint32_t)buf->memReq.size - 1;
                        }

                        __vkCmdLoadBatchHWStates(commandBuffer, hwConstRegAddr + (arrayIdx * 4), VK_FALSE, dataCount, data);
                    }
                    break;

                default:
                    __VK_ASSERT(!"Wrong descriptor type which should not fall in unfiorm buffer set function");
                    break;

                }
            }
            activeStageMask &= ~(1 << stageIdx);
        }
        stageIdx++;
    }

    if (descriptorBinding->std.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
    {
        (*dynamicOffsetIndex) += descriptorBinding->std.descriptorCount;
    }

    return VK_SUCCESS;
}

#if __VK_ENABLETS
VkResult halti5_setTxTileStatus(
    __vkCommandBuffer *cmdBuf,
    __vkCmdBindDescSetInfo *descSetInfo
    )
{
    VkResult result = VK_SUCCESS;
    int32_t hwSamplerNo;
    int32_t samplerTSSlotDirty = 0;
    uint32_t samplerTSIndex = 0;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    __vkImageView *imgv;
    __vkImage *img;
    VkImageSubresourceRange* pRanges;
    __vkDeviceMemory *imgMem;
    __vkTileStatus * tsResource;
    VkBool32 msaaImage;
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
    int32_t txDescDirty = chipCommand->txDescDirty;
    VkBool32 syncBetweenBLTAndRenderEngine = VK_FALSE;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    for (hwSamplerNo = 0; txDescDirty; hwSamplerNo++, txDescDirty >>= 1)
    {
        int32_t texTSSlot = -1;
        uint32_t j = 0;
        imgv = chipCommand->imgvWithTS[hwSamplerNo];
        img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, imgv->createInfo.image);
        pRanges = &imgv->createInfo.subresourceRange;

        if (chipCommand->texHasTileStatus[hwSamplerNo])
        {
            if (chipCommand->texTileStatusSlotIndex[hwSamplerNo] == -1)
            {
                for (j = 0; j < gcdSAMPLER_TS; j++)
                {
                    if (chipCommand->texTileStatusSlotUser[j] == -1)
                    {
                        texTSSlot = j;
                        break;
                    }
                }

                /* Can't find the slot. */
                if (texTSSlot == -1)
                {
                    __VK_PRINT("texture TS has to be disabled as out of slots");
                    __VK_ONERROR(halti5_decompressTileStatus(cmdBuf, &pCmdBuffer, img, pRanges));
                    chipCommand->texHasTileStatus[hwSamplerNo] = VK_FALSE;

                    if (cmdBuf->devCtx->database->REG_BltEngine)
                    {
                        syncBetweenBLTAndRenderEngine = VK_TRUE;
                    }
                }
                else
                {
                    chipCommand->texTileStatusSlotUser[texTSSlot] = hwSamplerNo;
                    chipCommand->texTileStatusSlotIndex[hwSamplerNo] = texTSSlot;
                    chipCommand->texTileStatusSlotDirty |= 1 << texTSSlot;
                }
            }
        }
    }

    if (syncBetweenBLTAndRenderEngine)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

        /* Semaphore-stall before next primitive. */
        *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&pCmdBuffer)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;


        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502E, VK_FALSE,
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
    }

    samplerTSSlotDirty = chipCommand->texTileStatusSlotDirty;

    for (samplerTSIndex = 0; samplerTSSlotDirty; samplerTSIndex++, samplerTSSlotDirty >>= 1)
    {
        if (!(samplerTSSlotDirty & 0x1))
        {
            continue;
        }

        hwSamplerNo = chipCommand->texTileStatusSlotUser[samplerTSIndex];

        /* Program tile status information. */
        if (hwSamplerNo != -1)
        {
            uint32_t control;
            uint32_t bitsPerPixel = 0;
            uint32_t tileStatusAddress = 0;

            imgv = chipCommand->imgvWithTS[hwSamplerNo];
            img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imgv->createInfo.image);
            imgMem = img->memory;
            tsResource = imgMem->ts;
            msaaImage = (img->sampleInfo.product > 1);
            pRanges = &imgv->createInfo.subresourceRange;
            bitsPerPixel = img->formatInfo.bitsPerBlock / img->formatInfo.partCount;
            tileStatusAddress = tsResource ? tsResource->devAddr : VK_NULL_HANDLE;

            if (!tsResource->compressed)
            {
                control = 1; /* Enable. */
            }
            else
            {
                /* Enabled and compressed */
                control = 3;
            }

            __vkCmdLoadSingleHWState(&pCmdBuffer,
                chipCommand->textureControlAddrReg[hwSamplerNo], VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:2) - (0 ?
 4:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:2) - (0 ?
 4:2) + 1))))))) << (0 ?
 4:2))) | (((gctUINT32) ((gctUINT32) (samplerTSIndex) & ((gctUINT32) ((((1 ?
 4:2) - (0 ?
 4:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:2) - (0 ? 4:2) + 1))))))) << (0 ? 4:2)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5))));

            __vkCmdLoadSingleHWState(&pCmdBuffer,
                0x05C8 + samplerTSIndex, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (control) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 7:4) - (0 ?
 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (tsResource->compressedFormat) & ((gctUINT32) ((((1 ?
 7:4) - (0 ?
 7:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (bitsPerPixel) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:11) - (0 ?
 13:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:11) - (0 ?
 13:11) + 1))))))) << (0 ?
 13:11))) | (((gctUINT32) ((gctUINT32) (-1) & ((gctUINT32) ((((1 ?
 13:11) - (0 ?
 13:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:11) - (0 ? 13:11) + 1))))))) << (0 ? 13:11)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05D0 + samplerTSIndex, tileStatusAddress);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05D8 + samplerTSIndex,
                tsResource->fcValue[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05E0 + samplerTSIndex,
                tsResource->fcValueUpper[pRanges->baseMipLevel][pRanges->baseArrayLayer]);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x06A0 + samplerTSIndex, imgMem->devAddr);
        }
        else
        {
            __vkCmdLoadSingleHWState(&pCmdBuffer,
                0x05C8 + samplerTSIndex, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ? 7:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:11) - (0 ?
 13:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:11) - (0 ?
 13:11) + 1))))))) << (0 ?
 13:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 13:11) - (0 ?
 13:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:11) - (0 ? 13:11) + 1))))))) << (0 ? 13:11)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05D0 + samplerTSIndex, 0);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05D8 + samplerTSIndex, 0);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x05E0 + samplerTSIndex, 0x00C0FFEE);

            __vkCmdLoadSingleHWState(&pCmdBuffer, VK_FALSE, 0x06A0 + samplerTSIndex, 0x00C0FFEF);
        }
    }

    /* clear dirty bit now */
    chipCommand->texTileStatusSlotDirty = 0;
    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);
OnError:
    return result;
}
#endif

VkResult halti5_setDesriptorSets(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip,
    __vkCmdBindDescSetInfo *descSetInfo
    )
{
    __vkDevContext *devCtx = cmdBuf->devCtx;
    uint32_t dirtyMask;
    uint32_t setIdx = 0;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    PROGRAM_EXECUTABLE_PROFILE *pep = &chipPipeline->curInstance->pep;

    if (!pep->u.vk.resourceSetCount)
    {
        return VK_SUCCESS;
    }
    dirtyMask = descSetInfo->dirtyMask;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    while (dirtyMask)
    {
        if (dirtyMask & (1 << setIdx))
        {
            __vkDescriptorSet *descSet = descSetInfo->descSets[setIdx];

            /* The pipeline layout can include entries that are not used by a particular pipeline, or
               that are dead-code eliminated from any of the shaders, so, we get mask from Pipeline layout, but not means we have the descSet */
            if (descSet)
            {
                uint32_t *dynamicOffsets = descSetInfo->dynamicOffsets[setIdx];
                uint32_t dynamicOffsetIndex = 0;
                __vkDescriptorSetLayout *descSetLayout = descSet->descSetLayout;
                PROG_VK_RESOURCE_SET *progResourceSet;
                uint32_t j;

                __VK_ASSERT(setIdx < pep->u.vk.resourceSetCount);
                progResourceSet = &pep->u.vk.pResourceSets[setIdx];

                for (j = 0; j < descSetLayout->bindingCount; j++)
                {
                    __vkDescriptorSetLayoutBinding *binding = &descSetLayout->binding[j];
                    switch ((int) binding->std.descriptorType)
                    {
                    case VK_DESCRIPTOR_TYPE_SAMPLER:
                        __VK_VERIFY_OK(halti5_helper_setDescSetSeperateSampler(
                            cmdBuf,
                            devCtx,
                            descSetInfo,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet));
                        break;

                    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        __VK_VERIFY_OK(halti5_helper_setDescSetSeperateImage(
                            cmdBuf,
                            devCtx,
                            descSetInfo,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet));
                        break;

                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        __VK_VERIFY_OK(halti5_helper_setDescSetCombinedImageSampler(
                            cmdBuf,
                            devCtx,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet));
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        __VK_VERIFY_OK(halti5_helper_setDescSetUniformTexelBuffer(
                            cmdBuf,
                            devCtx,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet));
                        break;

                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        __VK_VERIFY_OK(halti5_helper_setDescSetUniformBuffer(
                            devCtx,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet,
                            dynamicOffsets,
                            &dynamicOffsetIndex));
                        break;

                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        __VK_VERIFY_OK(halti5_helper_setDescSetStorage(
                            cmdBuf,
                            devCtx,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet,
                            dynamicOffsets,
                            &dynamicOffsetIndex));
                        break;

                    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                        __VK_VERIFY_OK(halti5_helper_setDescSetInputAttach(
                            cmdBuf,
                            devCtx,
                            &pCmdBuffer,
                            pip,
                            setIdx,
                            descSet,
                            binding,
                            progResourceSet));
                        break;
                    }
                }
            }
            dirtyMask &= ~(1 << setIdx);
        }
        setIdx++;
    }
    __VK_MEMZERO(chipPipeline->separateBindingProgramed, sizeof(VkBool32) * chipPipeline->countOfseparateBinding);
    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;

}

VkResult halti5_beginCommandBuffer(
    VkCommandBuffer commandBuffer
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    uint32_t *states;
    uint32_t hwFlushState = 0;
    uint32_t hwFlushVST = 0;
    uint32_t requestSize = 0;
#if __VK_ENABLETS
    uint32_t i;
    halti5_commandBuffer *chipCommand = (halti5_commandBuffer *)cmdBuf->chipPriv;
#endif
    hwFlushState |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
    requestSize += 2;
    hwFlushVST |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    requestSize += 2;

    if (!devCtx->database->REG_Halti5)
    {
        requestSize += 4;
    }

    __vk_CmdAquireBuffer(commandBuffer, chipModule->curInitCmdIndex, &states);

    __VK_MEMCOPY(states, chipModule->initialCmds, chipModule->curInitCmdIndex * sizeof(uint32_t));

    __vk_CmdReleaseBuffer(commandBuffer, chipModule->curInitCmdIndex);

    /* Force flush cache.
    ** if propertyFlags has the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT bit set,
    ** host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges are not needed to
    ** make host writes visible to the device or device writes visible to the host, respectively.
    */
    __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);

    if (!devCtx->database->REG_Halti5)
    {
        uint32_t stallDestination = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

        __vkCmdLoadSingleHWState(&states, 0x0E02, VK_FALSE, stallDestination);
        *(*&states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&states)++ = (stallDestination);
;

    }

    __vkCmdLoadSingleHWState(&states, 0x0E03, VK_FALSE, hwFlushState);

    __vkCmdLoadSingleHWState(&states, 0x0E03, VK_FALSE, hwFlushVST);

    __vk_CmdReleaseBuffer(commandBuffer, requestSize);
#if __VK_ENABLETS
    /* Initialize some TS information. */
    for (i = 0; i < gcdTXDESCRIPTORS; i++)
    {
        chipCommand->texHasTileStatus[i] = VK_FALSE;
        chipCommand->imgvWithTS[i] = VK_NULL_HANDLE;
        chipCommand->texTileStatusSlotIndex[i] = -1;
        chipCommand->textureControlAddrReg[i] = 0;
    }

    for (i = 0; i < gcdSAMPLER_TS; i++)
    {
        chipCommand->texTileStatusSlotUser[i] = -1;
    }
#endif

    return VK_SUCCESS;
}

VkResult halti5_endCommandBuffer(
    VkCommandBuffer commandBuffer
    )
{
#if __VK_RESOURCE_SAVE_TGA || gcdDUMP
    uint32_t *states;

    __vk_CmdAquireBuffer(commandBuffer, 2, &states);

    __vkCmdLoadSingleHWState(&states, 0x0E03, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
        );

    __vk_CmdReleaseBuffer(commandBuffer, 2);
#endif

    return VK_SUCCESS;
}


VkResult halti5_setPushConstants(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    )
{
    uint32_t dirtyMask;
    uint32_t entryIdx;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    PROGRAM_EXECUTABLE_PROFILE *pep = &chipPipeline->masterInstance->pep;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    struct _gcsHINT *hints = &chipPipeline->masterInstance->hwStates.hints;

    if (!pep->u.vk.pushConstantTable.countOfEntries)
    {
        return VK_SUCCESS;
    }

    dirtyMask = cmdBuf->bindInfo.pushConstants.dirtyMask;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    for (entryIdx = 0; entryIdx < pep->u.vk.pushConstantTable.countOfEntries; entryIdx++)
    {
        PROG_VK_PUSH_CONSTANT_TABLE_ENTRY *progPushConstEntry= &pep->u.vk.pushConstantTable.pPushConstantEntries[entryIdx];
        VSC_SHADER_PUSH_CONSTANT_RANGE *progPushConstRange = &progPushConstEntry->pushConstRange;
        uint32_t usageMask = (progPushConstRange->size >> 2) == (uint32_t)__VK_MAX_PUSHCONST_SIZE_IN_UINT
                           ? (uint32_t) ~0 : (~((uint32_t)~0 << (progPushConstRange->size >> 2))) << (progPushConstRange->offset >> 2);

        if (usageMask & dirtyMask)
        {
            uint32_t stageIdx = 0;
            uint32_t activeStageMask = progPushConstEntry->activeStageMask;
            while (activeStageMask)
            {
                if (activeStageMask & (1 << stageIdx))
                {
                    uint32_t hwConstRegNo = progPushConstEntry->hwMappings[stageIdx].hwLoc.hwRegNo;
                    uint32_t hwConstRegAddrBase = hints->hwConstRegBases[stageIdx];
                    uint32_t hwConstRegAddr;

                    hwConstRegAddr = (hwConstRegAddrBase >> 2) + (hwConstRegNo << 2) + progPushConstEntry->hwMappings[stageIdx].firstValidHwChannel;

                    __vkCmdLoadBatchHWStates(&pCmdBuffer, hwConstRegAddr, VK_FALSE, (progPushConstRange->size >> 2),
                        &cmdBuf->bindInfo.pushConstants.values[progPushConstRange->offset >> 2]);
                    activeStageMask &= ~(1 << stageIdx);
                }
                stageIdx++;
            }
        }
    }

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;

}

VkResult halti5_setMultiGPURenderingMode(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    )
{
    uint32_t i;

    uint32_t gpuCoreCount;
    uint32_t control = 0;
    gceMULTI_GPU_RENDERING_MODE mode;
    VkBool32 singleCore = gcvFALSE;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    const gcsFEATURE_DATABASE *database = devCtx->pPhyDevice->phyDevConfig.database;
    halti5_pipeline *chipPipeline = (halti5_pipeline *)pip->chipPriv;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result = VK_SUCCESS;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    gpuCoreCount = devCtx->chipInfo->gpuCoreCount;

    __VK_ASSERT(devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE);

    if (cmdBuf->bindInfo.oqEnable || chipPipeline->curInstance->memoryWrite)
    {
        singleCore = VK_TRUE;
    }

    if (singleCore)
    {
        mode = gcvMULTI_GPU_RENDERING_MODE_OFF;
    }
    else
    {
        mode = gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x64;
    }

    if (mode == cmdBuf->gpuRenderingMode)
    {
        return VK_SUCCESS;
    }

    /* Flush pipe.*/
    result = halti5_flushCache((VkDevice)devCtx, &pCmdBuffer, VK_NULL_HANDLE, HW_CACHE_ALL);

    switch (mode)
    {
    case gcvMULTI_GPU_RENDERING_MODE_OFF:
        control = 0x0;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_64x64:
        control = 0x4;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x64:
        control = 0x5;
        break;

    case gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x128:
        control = 0x6;
        break;
    case gcvMULTI_GPU_RENDERING_MODE_SPLIT_WIDTH:
    case gcvMULTI_GPU_RENDERING_MODE_SPLIT_HEIGHT:
    default:
        {
            __VK_ASSERT(0);
            break;
        }
    }

    if (control != 0x1)
    {
        gctBOOL blockSetConfig = database->MULTI_CORE_BLOCK_SET_CONFIG;
        if (!blockSetConfig || (blockSetConfig && gpuCoreCount == 4) )
        {
            /* Flat map set[i] to core[i]. */
            control |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (gpuCoreCount - 1) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)))
                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13)))
                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18)))
                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))
                     ;
        }
        else if (gpuCoreCount == 2)
        {
            gctUINT32 mapping[4][2][2] =
            {
                {
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))),
                    },
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))),
                    },
                },
                {
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))),
                    },
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))),
                    },
                },
                {
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))),
                    },
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))),
                    },
                },
                {
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21))),
                    },
                    {
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22))),
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))),
                    },
                }
            };

            gcmASSERT(gpuCoreCount == 2);

            control |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)));

            for (i = 0; i < gpuCoreCount; i++)
            {
                gctUINT32 chipID = i;
                control |= mapping[chipID][i][0];
                control |= mapping[chipID][i][1];
            }
        }
        else
        {
            gctUINT32 chipID;

            gctUINT32 mappingForFirstCore[4] =
            {
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 8:8) - (0 ?
 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 8:8) - (0 ?
 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:9) - (0 ?
 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 9:9) - (0 ?
 9:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ? 9:9))),

                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:13) - (0 ?
 13:13) + 1))))))) << (0 ?
 13:13))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 13:13) - (0 ?
 13:13) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:13) - (0 ? 13:13) + 1))))))) << (0 ? 13:13))),

                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 16:16) - (0 ?
 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 16:16) - (0 ?
 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 17:17) - (0 ?
 17:17) + 1))))))) << (0 ?
 17:17))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 17:17) - (0 ?
 17:17) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 17:17) - (0 ? 17:17) + 1))))))) << (0 ? 17:17))),

                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 20:20) - (0 ?
 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 20:20) - (0 ?
 20:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ? 20:20)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 21:21) - (0 ?
 21:21) + 1))))))) << (0 ?
 21:21))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 21:21) - (0 ?
 21:21) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 21:21) - (0 ? 21:21) + 1))))))) << (0 ? 21:21))),
            };

            gctUINT32 mappingForNextTwoCores[4][2] =
            {
                {
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))),
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11))),
                },
                {
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 14:14) - (0 ?
 14:14) + 1))))))) << (0 ?
 14:14))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 14:14) - (0 ?
 14:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 14:14) - (0 ? 14:14) + 1))))))) << (0 ? 14:14))),
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:15) - (0 ?
 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 15:15) - (0 ?
 15:15) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ? 15:15))),
                },
                {
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:18) - (0 ?
 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 18:18) - (0 ?
 18:18) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ? 18:18))),
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:19) - (0 ?
 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 19:19) - (0 ?
 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19))),
                },
                {
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:22) - (0 ?
 22:22) + 1))))))) << (0 ?
 22:22))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 22:22) - (0 ?
 22:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:22) - (0 ? 22:22) + 1))))))) << (0 ? 22:22))),
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:23) - (0 ?
 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 23:23) - (0 ?
 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23))),
                }
            };

            control |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:4) - (0 ?
 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ?
 6:4) - (0 ?
 6:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ? 6:4)));

            chipID = 0;

            control |= mappingForFirstCore[chipID];

            for (i = 1; i < gpuCoreCount; i++)
            {
                chipID = i;

                control |= mappingForNextTwoCores[chipID][i - 1];
            }

            gcmASSERT(gpuCoreCount == 3);
        }
    }

    __vkCmdLoadSingleHWState(&pCmdBuffer,0x0E80, VK_FALSE, control);

    cmdBuf->gpuRenderingMode = mode;

    cmdBuf->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(cmdBuf->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

    return result;
}

VkResult halti5_processQueryRequest(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    VkBool32 beginOQ
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    uint32_t *states;
    __vkBuffer *queryBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, qyp->queryBuffer);
    __vkDeviceMemory *memory = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, queryBuffer->memory);
    uint32_t queryAddress;

    queryAddress = memory->devAddr;
    queryAddress += query * sizeof(uint64_t);

    if (qyp->pQueries[query].type == VK_QUERY_TYPE_OCCLUSION)
    {
        if (beginOQ)
        {
            const gcsFEATURE_DATABASE *database = cmd->devCtx->database;
            VkBool32 peDepth = VK_TRUE;

            uint32_t *pCmdBuffer, *pCmdBufferBegin;

            if (cmd->devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
            {
                pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];
                halti5_setMultiGpuSync((VkDevice)cmd->devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&pCmdBuffer)++ = 0;
;

                cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

                __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

                __vk_CmdAquireBuffer(commandBuffer, cmd->curScrachBufIndex, &states);

                __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

                __vk_CmdReleaseBuffer(commandBuffer, cmd->curScrachBufIndex);

                cmd->curScrachBufIndex = 0;
            }

            if (cmd->bindInfo.pipeline.graphics)
            {
                halti5_graphicsPipeline *chipPriv = (halti5_graphicsPipeline *)cmd->bindInfo.pipeline.graphics->chipPriv;
                peDepth = chipPriv->peDepth;
            }

            __vk_CmdAquireBuffer(commandBuffer, 4, &states);
            if (database->REG_RAWriteDepth && !peDepth)
            {
                __vkCmdLoadSingleHWState(&states, 0x0E18, VK_FALSE,
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
                    );
            }
            else
            {
                __vkCmdLoadSingleHWState(&states, 0x0E18, VK_FALSE,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                    );
            }
            __vkCmdLoadSingleHWState(&states, 0x0E09, VK_FALSE, queryAddress);
            __vk_CmdReleaseBuffer(commandBuffer, 4);

            if (cmd->devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
            {
                pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];
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

                halti5_setMultiGpuSync((VkDevice)cmd->devCtx, &pCmdBuffer, VK_NULL_HANDLE);

                cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

                __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

                __vk_CmdAquireBuffer(commandBuffer, cmd->curScrachBufIndex, &states);

                __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

                __vk_CmdReleaseBuffer(commandBuffer, cmd->curScrachBufIndex);

                cmd->curScrachBufIndex = 0;
            }

            qyp->pQueries[query].isBegin = VK_TRUE;
            cmd->bindInfo.oqEnable = VK_TRUE;
        }
        else
        {
            uint32_t data = 31415926;
            uint32_t *pCmdBuffer, *pCmdBufferBegin;

            if (cmd->devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
            {
                pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];
                halti5_setMultiGpuSync((VkDevice)cmd->devCtx, &pCmdBuffer, VK_NULL_HANDLE);
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
 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27))) | (gcvCORE_3D_0_MASK);*(*&pCmdBuffer)++ = 0;
;

                cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

                __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

                __vk_CmdAquireBuffer(commandBuffer, cmd->curScrachBufIndex, &states);

                __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

                __vk_CmdReleaseBuffer(commandBuffer, cmd->curScrachBufIndex);

                cmd->curScrachBufIndex = 0;
            }

            __vk_CmdAquireBufferAndLoadHWStates(commandBuffer, 0x0E0C, VK_FALSE, 1, &data);

            __vk_CmdSetEvent(commandBuffer, qyp->pQueries[query].event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

            if (cmd->devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
            {
                pCmdBuffer = pCmdBufferBegin = &cmd->scratchCmdBuffer[cmd->curScrachBufIndex];
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

                halti5_setMultiGpuSync((VkDevice)cmd->devCtx, &pCmdBuffer, VK_NULL_HANDLE);

                cmd->curScrachBufIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);

                __VK_ASSERT(cmd->curScrachBufIndex <= __VK_CMDBUF_SCRATCH_BUFFER_SIZE);

                __vk_CmdAquireBuffer(commandBuffer, cmd->curScrachBufIndex, &states);

                __VK_MEMCOPY(states, cmd->scratchCmdBuffer, cmd->curScrachBufIndex * sizeof(uint32_t));

                __vk_CmdReleaseBuffer(commandBuffer, cmd->curScrachBufIndex);

                cmd->curScrachBufIndex = 0;
            }

            cmd->bindInfo.oqEnable = VK_FALSE;
        }
    }

    return VK_SUCCESS;
}

VkResult halti5_bindDescriptors(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    uint32_t firstSet,
    uint32_t setCount
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    uint32_t i;
    __vkCmdBindDescSetInfo *bindDescSet = VK_NULL_HANDLE;
    switch (pipelineBindPoint)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        bindDescSet = &cmdBuf->bindInfo.bindDescSet.graphics;
        break;
    case VK_PIPELINE_BIND_POINT_COMPUTE:
        bindDescSet = &cmdBuf->bindInfo.bindDescSet.compute;
        break;
    default:
        __VK_ASSERT(!"unknown pipeline binding point");
        break;
    }
    __VK_ASSERT(firstSet + setCount < __VK_MAX_DESCRIPTOR_SETS);
    for (i = 0; i < setCount; i++)
    {
        uint32_t setIdx = i + firstSet;
        __vkDescriptorSet *descSet = bindDescSet->descSets[setIdx];
        halti5_descriptorSet *chipDescSet = (halti5_descriptorSet *)descSet->chipPriv;

        if (chipDescSet->numofKeys)
        {
            bindDescSet->patchMask |= 1 << setIdx;
        }
        else
        {
            bindDescSet->patchMask &= ~(1 << setIdx);
        }
    }

    return VK_SUCCESS;
}

 void halti5_pipelineBarrier(
     VkCommandBuffer commandBuffer,
     VkPipelineStageFlags srcStageMask,
     VkPipelineStageFlags dstStageMask,
     VkDependencyFlags dependencyFlags,
     uint32_t memoryBarrierCount,
     const VkMemoryBarrier* pMemoryBarriers,
     uint32_t bufferMemoryBarrierCount,
     const VkBufferMemoryBarrier* pBufferMemoryBarriers,
     uint32_t imageMemoryBarrierCount,
     const VkImageMemoryBarrier* pImageMemoryBarriers
     )
 {
     __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
     uint32_t i;
     VkFlags srcAccessMask = 0, dstAccessMask = 0;
     VkBool32 flushColorCache = VK_FALSE;
     VkBool32 flushZCache = VK_FALSE;
     VkBool32 invalidateTxCache = VK_FALSE;
     VkBool32 flushSHL1Cache = VK_FALSE;
     VkBool32 stallRA = VK_FALSE;
     VkBool32 stallFE = VK_FALSE;
     uint32_t requestSize = 0;
     uint32_t *states;
     uint32_t hwFlushState = 0;
     uint32_t hwFlushVST = 0;

     for (i = 0; i < memoryBarrierCount; i++)
     {
         srcAccessMask |= pMemoryBarriers[i].srcAccessMask;
         dstAccessMask |= pMemoryBarriers[i].dstAccessMask;
     }
     for (i = 0; i < bufferMemoryBarrierCount; i++)
     {
         srcAccessMask |= pBufferMemoryBarriers[i].srcAccessMask;
         dstAccessMask |= pBufferMemoryBarriers[i].dstAccessMask;
     }
     for (i = 0; i < imageMemoryBarrierCount; i++)
     {
         srcAccessMask |= pImageMemoryBarriers[i].srcAccessMask;
         dstAccessMask |= pImageMemoryBarriers[i].dstAccessMask;
     }

     if (srcStageMask & (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT
                       | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                       | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
                       | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
                       | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
                       | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
                       | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                       | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                       | VK_PIPELINE_STAGE_TRANSFER_BIT
                       | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
                       | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT))
     {
         if (srcAccessMask & (VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT))
         {
             flushSHL1Cache = VK_TRUE;
             if (dstAccessMask & (VK_ACCESS_INDIRECT_COMMAND_READ_BIT
                                | VK_ACCESS_INDEX_READ_BIT
                                | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                                | VK_ACCESS_UNIFORM_READ_BIT
                                | VK_ACCESS_SHADER_READ_BIT
                                | VK_ACCESS_TRANSFER_READ_BIT))
             {
                 stallFE = VK_TRUE;
             }
         }
         if (srcAccessMask & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
         {
             flushColorCache = VK_TRUE;
         }
         if (srcAccessMask & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
         {
             flushZCache = VK_TRUE;
         }
     }

     if ((srcStageMask & VK_PIPELINE_STAGE_HOST_BIT) && (srcAccessMask & VK_ACCESS_HOST_WRITE_BIT))
     {
         if (dstAccessMask & VK_ACCESS_UNIFORM_READ_BIT)
         {
             flushSHL1Cache = VK_TRUE;
         }

         if (dstAccessMask & (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT))
         {
             flushSHL1Cache = VK_TRUE;
             invalidateTxCache = VK_TRUE;
         }
     }
    /* bewtee copy images, sometime we will use computeblit to do copyImage */
    if ((srcStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT) && (dstStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT))
    {
        flushSHL1Cache = VK_TRUE;
        stallFE = VK_TRUE;
    }

    if ((srcStageMask & VK_PIPELINE_STAGE_TRANSFER_BIT) && (srcAccessMask & VK_ACCESS_TRANSFER_WRITE_BIT))
     {
         if (dstAccessMask & (VK_ACCESS_INDIRECT_COMMAND_READ_BIT
                            | VK_ACCESS_INDEX_READ_BIT
                            | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                            | VK_ACCESS_UNIFORM_READ_BIT
                            | VK_ACCESS_SHADER_READ_BIT
                            | VK_ACCESS_SHADER_WRITE_BIT
                            | VK_ACCESS_TRANSFER_READ_BIT
                            | VK_ACCESS_TRANSFER_WRITE_BIT))
         {
             stallFE = VK_TRUE;
         }

         if (dstAccessMask & (VK_ACCESS_INPUT_ATTACHMENT_READ_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                            | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT))
         {
             stallRA = VK_TRUE;
         }
     }

     if (flushColorCache || flushZCache || flushSHL1Cache)
     {
         hwFlushState |= flushColorCache ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) : 0;
         hwFlushState |= flushZCache ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) : 0;
         hwFlushState |= flushSHL1Cache ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10))) : 0;
         requestSize += 2;
     }

    if (invalidateTxCache)
    {
        requestSize += !hwFlushState ? 4 : 2;
        hwFlushState |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)));
        hwFlushVST   |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));

        if (!devCtx->database->REG_Halti5)
        {
            stallFE = VK_TRUE;
        }
    }

    if (stallFE || stallRA)
    {
        requestSize += 4;
        if (devCtx->database->REG_BltEngine)
        {
           requestSize += 4;
        }
    }

    if (requestSize)
    {
        __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);

        if (hwFlushState != 0)
        {
            __vkCmdLoadSingleHWState(&states, 0x0E03, VK_FALSE, hwFlushState);
        }

        if (hwFlushVST != 0)
        {
            __vkCmdLoadSingleHWState(&states, 0x0E03, VK_FALSE, hwFlushVST);
        }

        if (stallFE || stallRA)
        {
            uint32_t stallDestination;
            VkBool32 needLockBlt = VK_FALSE;
            if (devCtx->database->REG_BltEngine)
            {
                stallDestination = stallFE ?
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) :
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
                needLockBlt = VK_TRUE;
            }
            else
            {
                stallDestination = stallFE ?
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))) :
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x05 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0))) |
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
            }

            if (needLockBlt)
            {
                __vkCmdLoadSingleHWState(&states, 0x502E, VK_FALSE,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));
            }

            __vkCmdLoadSingleHWState(&states, 0x0E02, VK_FALSE, stallDestination);

            if (stallFE)
            {
                *(*&states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&states)++ = (stallDestination);
;

            }
            else
            {
                __VK_STALL_RA(&states, stallDestination);
            }

            if (needLockBlt)
            {
                __vkCmdLoadSingleHWState(&states, 0x502E, VK_FALSE,
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
            }
        }

        __vk_CmdReleaseBuffer(commandBuffer, requestSize);
    }

     return;
 }

 void halti5_setEvent(
     VkCommandBuffer commandBuffer,
     VkEvent event,
     VkPipelineStageFlags stageMask,
     VkBool32 signal
     )
 {
     __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
     __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
     __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->devCtx->fenceBuffer);
     __vkDeviceMemory *memory = fenceBuffer->memory;
     uint32_t fenceAddress;
     uint32_t *states;
     uint32_t requestSize = 6;
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
     fenceAddress = memory->devAddr;

     if (stageMask &
         (VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
         | VK_PIPELINE_STAGE_TRANSFER_BIT
         | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT))
     {
         requestSize += 4;
          if (devCtx->database->REG_BltEngine)
          {
              requestSize += 4;
          }
     }

     __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);

     if (stageMask &
         (VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
         | VK_PIPELINE_STAGE_TRANSFER_BIT
         | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT))
     {

         if(devCtx->database->REG_BltEngine)
         {
             __vkCmdLoadSingleHWState(&states, 0x502E, VK_FALSE,
                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

             __vkCmdLoadSingleHWState(&states, 0x0E02, VK_FALSE,
                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

             *(*&states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&states)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;


             __vkCmdLoadSingleHWState(&states, 0x502E, VK_FALSE,
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
         }
         else
         {
             __vkCmdLoadSingleHWState(&states, 0x0E02, VK_FALSE,
                 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

             *(*&states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&states)++ = (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));;

         }
     }

     /* always send fence from PE */
     __vkCmdLoadSingleHWState(&states, 0x0E1A, VK_FALSE, (fenceAddress + evt->fenceIndex * sizeof(__vkHwFenceData)));
     __vkCmdLoadSingleHWState(&states, 0x0E26, VK_FALSE, 0);
     __vkCmdLoadSingleHWState(&states, 0x0E1B, VK_FALSE, signal ? 1 : 0);

     __vk_CmdReleaseBuffer(commandBuffer, requestSize);

     return;

 }

 void halti5_waitEvents(
    VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags destStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, cmdBuf->devCtx->fenceBuffer);
    __vkDeviceMemory *memory = fenceBuffer->memory;
    uint32_t ievt;
    uint32_t *states;
    uint32_t fenceAddress;

    fenceAddress = memory->devAddr;

    __vk_CmdAquireBuffer(commandBuffer, (2 * eventCount), &states);

    /* always wait at FE */
    for (ievt = 0; ievt < eventCount; ievt++)
    {
        __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, pEvents[ievt]);

        *states++ = __VK_CMD_HW_FENCE_WAIT(10);
        *states++ = fenceAddress + evt->fenceIndex * sizeof(__vkHwFenceData);
    }

    __vk_CmdReleaseBuffer(commandBuffer, (2 * eventCount));

    halti5_pipelineBarrier(commandBuffer, srcStageMask, destStageMask, 0, memoryBarrierCount,pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
        imageMemoryBarrierCount, pImageMemoryBarriers);

}

VkResult halti5_allocateCommandBuffer(
    VkDevice device,
    VkCommandBuffer commandBuffer
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmdBuf->commandPool);
    halti5_commandBuffer *chipCommand = VK_NULL_HANDLE;
    VkResult result;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    chipCommand = (halti5_commandBuffer *)__VK_ALLOC(sizeof(halti5_commandBuffer), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    __VK_ONERROR(chipCommand ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
    __VK_MEMZERO(chipCommand, sizeof(halti5_commandBuffer));

    cmdBuf->chipPriv = chipCommand;

    return VK_SUCCESS;
OnError:
    if (chipCommand)
    {
        __VK_FREE(chipCommand);
    }
    return result;

}

VkResult halti5_freeCommandBuffer(
    VkDevice device,
    VkCommandBuffer commandBuffer
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmdBuf->commandPool);

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    if (cmdBuf->chipPriv)
    {
        __VK_FREE(cmdBuf->chipPriv);
    }

    return VK_SUCCESS;
}

void halti5_bindPipeline(
    VkCommandBuffer commandBuffer,
    VkPipeline oldPipeline,
    VkPipeline newPipeline
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    halti5_commandBuffer *chipCommandBuffer = (halti5_commandBuffer *)cmdBuf->chipPriv;
    __vkPipeline *oldpip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, oldPipeline);
    __vkPipeline *newpip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, newPipeline);
    uint32_t pipType = oldpip ? oldpip->type : (newpip ? newpip->type : __VK_PIPELINE_TYPE_INVALID);

    __VK_ASSERT(!oldpip || !newpip || (oldpip->type == newpip->type));

    if (pipType == __VK_PIPELINE_TYPE_GRAPHICS)
    {
        if (!oldpip || !newpip)
        {
            chipCommandBuffer->gfxPipelineSwitchDirtyMask = HALTI5_GFXPIPELINE_SWITCH_ALL_DIRTY;
        }
        else
        {
            halti5_graphicsPipeline *oldgfxPipeline = (halti5_graphicsPipeline *)oldpip->chipPriv;
            halti5_graphicsPipeline *newgfxPipeline = (halti5_graphicsPipeline *)newpip->chipPriv;
            halti5_pipeline *oldPipeline = (halti5_pipeline *)oldgfxPipeline;
            halti5_pipeline *newPipeline = (halti5_pipeline *)newgfxPipeline;

            __VK_ASSERT(newgfxPipeline && oldgfxPipeline);
            if (newgfxPipeline->singlePEpipe != oldgfxPipeline->singlePEpipe)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_SINGLE_PE_DIRTY;
            }

            if (newgfxPipeline->hwCacheMode != oldgfxPipeline->hwCacheMode)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_CACHEMODE_DIRTY;
            }

            if (newgfxPipeline->stencilMode != oldgfxPipeline->stencilMode)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_STENCIL_MODE_DIRTY;
            }

            if (newgfxPipeline->earlyZ != oldgfxPipeline->earlyZ)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_EARLYZ_DIRTY;
            }

            if (newgfxPipeline->depthCompareOp != oldgfxPipeline->depthCompareOp)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_DEPTH_COMPAREOP_DIRTY;
            }

            if (newgfxPipeline->peDepth != oldgfxPipeline->peDepth)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_PE_DEPTH_DIRTY;
            }

            if (newgfxPipeline->destinationRead != oldgfxPipeline->destinationRead)
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_DESTINATION_READ_DIRTY;
            }

            if ((oldPipeline->curInstance->hwStates.hints.unifiedStatus.constGPipeEnd <=
                 newPipeline->curInstance->hwStates.hints.unifiedStatus.constPSStart)
              ||(oldPipeline->curInstance->hwStates.hints.unifiedStatus.samplerPSEnd <=
                 newPipeline->curInstance->hwStates.hints.unifiedStatus.samplerGPipeStart))
            {
                chipCommandBuffer->gfxPipelineSwitchDirtyMask |= HALTI5_GFXPIPELINE_SWITCH_UNIFIED_RESOURCE_DIRTY;
            }
        }
    }
}

VkResult halti5_setMultiGpuSync(
    VkDevice device,
    uint32_t** commandBuffer,
    uint32_t * sizeInUint
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    uint32_t stallDestination;
    VkBool32 hasBltEngine = devCtx->database->REG_BltEngine;
    static uint32_t scratchBuffer[__VK_SCRATCH_BUFFER_SIZE];

    if (commandBuffer)
    {
        pCmdBuffer = *commandBuffer;
    }
    else
    {
        pCmdBuffer = scratchBuffer;
    }

    pCmdBufferBegin = pCmdBuffer;

    __VK_ASSERT(devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE);

    /* Drain the whole pipe */
    if (hasBltEngine)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));
    }

    stallDestination = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 29:28) - (0 ?
 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 29:28) - (0 ?
 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))
                     | (hasBltEngine ?
                       ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                     : ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE, stallDestination);

    *(*&pCmdBuffer)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));*(*&pCmdBuffer)++ = (stallDestination);
;


    if (hasBltEngine)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x502E, VK_FALSE,
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
    }

    if (devCtx->database->MultiCoreSemaphoreStallV2)
    {
        uint32_t coreIdx;
        uint32_t coreCount = devCtx->chipInfo->gpuCoreCount;
        const uint32_t *coreIDs = devCtx->chipInfo->gpuCoreID;
        for (coreIdx = 0; coreIdx < coreCount; coreIdx++)
        {
            /* Select ith core 3D. */
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                          | (gcvCORE_3D_0_MASK << coreIDs[coreIdx]);
            pCmdBuffer++;
            /* For 1st Core 3D */
            if (coreIdx == 0)
            {
                /* Send a semaphore token from FE to next Core 3D. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx + 1]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

                 gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", coreIDs[coreIdx + 1]);

                /* Await a semaphore token from next core 3D. */
                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx + 1]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

                gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", coreIDs[coreIdx]);
            }
            /* For last Core 3D */
            else if (coreIdx == (coreCount - 1))
            {
                /* Await a semaphore token from previous core 3D. */
                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx - 1]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

                gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", coreIDs[coreIdx]);

                /* Send a semaphore token from FE to previous Core 3D. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx - 1]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

                 gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", coreIDs[coreIdx - 1]);
            }
            /* For all other Core 3D sitting in the middle */
            else
            {
                /* Await a semaphore token from previous core 3D. */
                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx - 1]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

                gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", coreIDs[coreIdx]);

                /* Send a semaphore token from FE to next Core 3D. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx + 1]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

                 gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", coreIDs[coreIdx + 1]);

                /* Await a semaphore token from next core 3D. */
                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

                *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx + 1]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

                gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", coreIDs[coreIdx]);

                /* Send a semaphore token from FE to previous Core 3D. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx]) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (coreIDs[coreIdx - 1]) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

                 gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", coreIDs[coreIdx - 1]);
            }
        }

        /* Enable all 3D GPUs. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | gcvCORE_3D_ALL_MASK;

        pCmdBuffer++;

    }
    else
    {
        /* Select core 3D 0. */

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | gcvCORE_3D_0_MASK;

        pCmdBuffer++;

        gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK);

        /* Send a semaphore token from FE to core 3D 1. */
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24))));

        gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcvCORE_3D_1_ID);

        /* Await a semaphore token from core 3D 1. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

        gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcvCORE_3D_0_ID);

        /* Select core 3D 1. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | gcvCORE_3D_1_MASK;

        pCmdBuffer++;

        gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_1_MASK);

        /* Send a semaphore token from FE to core 3D 0. */
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E02, VK_FALSE,
                        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 23:20) - (0 ?
 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 23:20) - (0 ?
 23:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ? 23:20)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)))
                        );

        gcmDUMP(gcvNULL, "#[chip.semaphore 0x%04X]", gcvCORE_3D_0_ID);

        /* Await a semaphore token from GPU 3D_0. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x0F & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 19:16) - (0 ?
 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 19:16) - (0 ?
 19:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ? 19:16)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 27:24) - (0 ?
 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 27:24) - (0 ?
 27:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ? 27:24)));

        gcmDUMP(gcvNULL, "#[chip.stall 0x%04X]", gcvCORE_3D_1_ID);

        /* Enable all 3D GPUs. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | gcvCORE_3D_ALL_MASK;
        pCmdBuffer++;
    }

    if (commandBuffer)
    {
        *commandBuffer = pCmdBuffer;
    }

    if (sizeInUint)
    {
        size_t size = pCmdBuffer - pCmdBufferBegin;
        __VK_ASSERT(size <= __VK_SCRATCH_BUFFER_SIZE);
        *sizeInUint = (uint32_t)size;
    }

    return VK_SUCCESS;
}

/*
** 1) For multiGPU combine mode, we need flush more GPU cache for every commit as
** only GPU0 would receive event, where it triggers to flush GPU internal cache
** for only GPU0. so we need put all cache flush command in user command buffer
** to make it broadcasted.
** 2) For new submission path, Vulkan directly unlock and free memory w/o through
** Event mechanmis, which mean GPU cache is not get flush out for these freed memory.
** we need flush cache at the end of every commit.
*/
VkResult halti5_flushCache(
    VkDevice device,
    uint32_t **commandBuffer,
    uint32_t *sizeInUint,
    int32_t  cacheMask
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    static uint32_t scratchBuffer[__VK_SCRATCH_BUFFER_SIZE];
    HwCacheMask hwCacheMask = (HwCacheMask)cacheMask;
    VkBool32 needSemaphoreStall = VK_FALSE;
    uint32_t aqFlushState = 0;

    if (commandBuffer)
    {
        pCmdBuffer = *commandBuffer;
    }
    else
    {
        pCmdBuffer = scratchBuffer;
    }

    __VK_ASSERT(hwCacheMask);

    pCmdBufferBegin = pCmdBuffer;

    if (hwCacheMask & (HW_CACHE_TEXTURE_ALL | HW_CACHE_VST_ALL | HW_CACHE_INSTRUCTION))
    {
        needSemaphoreStall = VK_TRUE;
    }

    if (needSemaphoreStall)
    {
        /* Semaphore. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

        /* Stall. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
    }

    aqFlushState |= (hwCacheMask & HW_CACHE_DEPTH) ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))) : 0;
    aqFlushState |= (hwCacheMask & HW_CACHE_COLOR) ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))) : 0;
    aqFlushState |= (hwCacheMask & HW_CACHE_SH_L1) ?
        (((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 10:10) - (0 ?
 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ?
 10:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ? 10:10)))
       | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:11) - (0 ?
 11:11) + 1))))))) << (0 ?
 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ?
 11:11) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:11) - (0 ? 11:11) + 1))))))) << (0 ? 11:11)))) : 0;
    aqFlushState |= (hwCacheMask & HW_CACHE_L2) ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 6:6) - (0 ?
 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 6:6) - (0 ?
 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6))) : 0;
    aqFlushState |= (hwCacheMask & HW_CACHE_TEXTURE_DATA) ?
 ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2))) : 0;

    if (aqFlushState)
    {
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

        *pCmdBuffer++ = aqFlushState;
    }

    if (hwCacheMask & HW_CACHE_VST_DATA)
    {
         /* Append LOAD_STATE to AQFlush. */
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E03) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    }
#if __VK_ENABLETS
    if (hwCacheMask & HW_CACHE_TS)
    {
        if (!devCtx->database->PE_TILE_CACHE_FLUSH_FIX &&
            devCtx->database->REG_BltEngine)
        {
            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x502E) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x502B) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));

            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x502E) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

            *pCmdBuffer++
                = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
        }
        else
        {
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0594) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)));
        }
    }
#endif
    if ((devCtx->database->REG_Halti5) && (hwCacheMask & HW_CACHE_INSTRUCTION))
    {
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x022C) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)));
    }

    if ((devCtx->database->REG_Halti5) && (hwCacheMask & HW_CACHE_TX_DESC))
    {
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x5311) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));

        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:28) - (0 ?
 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 31:28) - (0 ?
 31:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ? 31:28)));

    }

    if ((devCtx->database->HWTFB) && (hwCacheMask & HW_CACHE_TFB))
    {
        *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x7003) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)));
        *pCmdBuffer++ = 0x12345678;

    }

    if (needSemaphoreStall)
    {
#if __VK_ENABLETS
        if (!devCtx->database->PE_TILE_CACHE_FLUSH_FIX &&
            devCtx->database->REG_BltEngine)
        {
            /* Semaphore. */
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));

            /* Stall. */
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
        }
        else
#endif
        {
            /* Semaphore. */
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ? 25:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0E02) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ? 15:0)));

            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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

            /* Stall. */
            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));

            *pCmdBuffer++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
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
        }
    }

    if (commandBuffer)
    {
        *commandBuffer = pCmdBuffer;
    }

    if (sizeInUint)
    {
        size_t size = pCmdBuffer - pCmdBufferBegin;
        __VK_ASSERT(size <= __VK_SCRATCH_BUFFER_SIZE);
        *sizeInUint = (uint32_t)size;
    }

    return VK_SUCCESS;
}

VkResult halti5_setDrawID(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    )
{
    uint32_t drawID;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    static VkBool32 s_IDcontrol = VK_TRUE;

    pCmdBuffer = pCmdBufferBegin = &cmdBuf->scratchCmdBuffer[cmdBuf->curScrachBufIndex];

    if (s_IDcontrol)
    {
        drawID = (uint32_t)pip->obj.id;
    }
    else
    {
        __VK_ASSERT(!(cmdBuf->obj.id & 0xFFFF0000));
        __VK_ASSERT(!(cmdBuf->sequenceID & 0xFFFF0000));
        drawID = (cmdBuf->obj.id << 16) | cmdBuf->sequenceID;
    }
    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E27, VK_FALSE, drawID);

    cmdBuf->curScrachBufIndex += (uint32_t)(uintptr_t)(pCmdBuffer - pCmdBufferBegin);

    return VK_SUCCESS;
}



