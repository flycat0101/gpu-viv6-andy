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

__vkChipFuncTable halti5_chip =
{
    halti5_initializeChipModule,
    halti5_finalizeChipModule,
    halti5_createGraphicsPipeline,
    halti5_createComputePipeline,
    halti5_destroyPipeline,
    halti5_clearImage,
    halti5_copyImage,
    halti5_fillBuffer,
    halti5_copyBuffer,
    halti5_updateBuffer,
    halti5_computeBlit,
    halti5_draw,
    halti5_drawIndexed,
    halti5_drawIndirect,
    halti5_drawIndexedIndirect,
    halti5_dispatch,
    halti5_dispatchIndirect,
    halti5_createSampler,
    halti5_createImageView,
    halti5_destroyImageView,
    halti5_createBufferView,
    halti5_destroyBufferView,
    halti5_beginCommandBuffer,
    halti5_endCommandBuffer,
    halti5_allocDescriptorSet,
    halti5_freeDescriptorSet,
    halti5_updateDescriptorSet,
    halti5_processQueryRequest,
    halti5_bindDescriptors,
    halti5_pipelineBarrier,
    halti5_setEvent,
    halti5_waitEvents,
    halti5_allocateCommandBuffer,
    halti5_freeCommandBuffer,
    halti5_bindPipeline,
    halti5_setMultiGpuSync,
    halti5_flushCache,
};

static void halti5_helper_computeCentroids(
    IN  __vkDevContext *devCtx,
    IN  gctUINT32 Count,
    IN  gctUINT32_PTR SampleCoords,
    OUT gcsCENTROIDS_PTR Centroids
    )
{
    gctUINT i, j, count, sumX, sumY;

    for (i = 0; i < Count; i += 1)
    {
        /* Zero out the centroid table. */
        gcoOS_ZeroMemory(&Centroids[i], sizeof(gcsCENTROIDS));

        /* Set the value for invalid pixels. */
        Centroids[i].value[0]
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (8) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (8) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)));

        /* Compute all centroids. */
        for (j = 1; j < 16; j += 1)
        {
            /* Initializ sums and count. */
            sumX = sumY = 0;
            count = 0;

            if ((j == 0x7) || (j == 0xB)
                ||  (j == 0xD) || (j == 0xE)
                )
            {
                sumX = sumY = 0x8;
            }
            else
            {
                if (j & 0x1)
                {
                    /* Add sample 1. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 7:4)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1)))))) );
                    ++count;
                }

                if (j & 0x2)
                {
                    /* Add sample 2. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 11:8)) & ((gctUINT32) ((((1 ? 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 15:12)) & ((gctUINT32) ((((1 ? 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1)))))) );
                    ++count;
                }

                if (j & 0x4)
                {
                    /* Add sample 3. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 19:16)) & ((gctUINT32) ((((1 ? 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 23:20)) & ((gctUINT32) ((((1 ? 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1)))))) );
                    ++count;
                }

                if (j & 0x8)
                {
                    /* Add sample 4. */
                    sumX += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 27:24)) & ((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1)))))) );
                    sumY += (((((gctUINT32) (SampleCoords[i])) >> (0 ? 31:28)) & ((gctUINT32) ((((1 ? 31:28) - (0 ? 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1)))))) );
                    ++count;
                }

                /* Compute average. */
                if (count > 0)
                {
                    sumX /= count;
                    sumY /= count;
                }
            }

            switch (j & 3)
            {
            case 0:
                /* Set for centroid 0, 4, 8, or 12. */
                Centroids[i].value[j / 4]
                = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:4) - (0 ? 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)));
                break;

            case 1:
                /* Set for centroid 1, 5, 9, or 13. */
                Centroids[i].value[j / 4]
                = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12)));
                break;

            case 2:
                /* Set for centroid 2, 6, 10, or 14. */
                Centroids[i].value[j / 4]
                = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20)));
                break;

            case 3:
                /* Set for centroid 3, 7, 11, or 15. */
                Centroids[i].value[j / 4]
                = ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ?
 27:24) + 1))))))) << (0 ? 27:24))) | (((gctUINT32) ((gctUINT32) (sumX) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ?
 27:24) + 1))))))) << (0 ? 27:24)))
                    | ((((gctUINT32) (Centroids[i].value[j / 4])) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:28) - (0 ? 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ?
 31:28) + 1))))))) << (0 ? 31:28))) | (((gctUINT32) ((gctUINT32) (sumY) & ((gctUINT32) ((((1 ?
 31:28) - (0 ? 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ?
 31:28) + 1))))))) << (0 ? 31:28)));
                break;
            }
        }
    }

    return;
}

#include "gc_halti5_patchlib.frag.h"

static VkResult halti5_finalizeComputeBlit(
    __vkDevContext *devCtx
    )
{
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    uint32_t kind;

    for (kind = 0; kind < HALTI5_BLIT_NUM; ++kind)
    {
        halti5_vscprogram_blit *blitProg = &chipModule->blitProgs[kind];

        if (blitProg->inited)
        {
            vscFinalizePEP(&blitProg->pep);
            vscFinalizeHwPipelineShadersStates(&devCtx->vscSysCtx, &blitProg->hwStates);

            blitProg->inited = VK_FALSE;
        }
    }

    return VK_SUCCESS;
}

VkResult halti5_initializeChipModule(
    VkDevice device
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    halti5_module *chipModule;
    SpvDecodeInfo decodeInfo;
    VSC_SHADER_COMPILER_PARAM vscCompileParams;
    float limit;
    uint32_t *pCmdBuffer, *pCmdBufferBegin;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    chipModule = (halti5_module *)__VK_ALLOC(
        sizeof(halti5_module), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

    if (!chipModule)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    __VK_MEMZERO(chipModule, sizeof(halti5_module));

    /* 2x MSAA sample coordinates. */
    chipModule->sampleCoords2
        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* 4x MSAA sample coordinates.
    **
    **                        1 1 1 1 1 1                        1 1 1 1 1 1
    **    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  0| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  1| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  2| | | | | | |X| | | | | | | | | |  | | | | | | | | | | |X| | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  3| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  4| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  5| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  6| | | | | | | | | | | | | | |X| |  | | |X| | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  7| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  8| | | | | | | | |o| | | | | | | |  | | | | | | | | |o| | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    **  9| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 10| | |X| | | | | | | | | | | | | |  | | | | | | | | | | | | | | |X| |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 11| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 12| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 13| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 14| | | | | | | | | | |X| | | | | |  | | | | | | |X| | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ** 15| | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |
    **   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    /* Diamond. */
    chipModule->sampleCoords4[0]
    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Mirrored diamond. */
    chipModule->sampleCoords4[1]
    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (6) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (14) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));

    /* Square. */
    chipModule->sampleCoords4[2]
    = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 7:4) - (0 ?
 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1))))))) << (0 ?
 7:4)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 15:12) - (0 ?
 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ? 15:12) + 1))))))) << (0 ?
 15:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 19:16) - (0 ?
 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ? 19:16) + 1))))))) << (0 ?
 19:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 23:20) - (0 ?
 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ? 23:20) + 1))))))) << (0 ?
 23:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 27:24) - (0 ?
 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1))))))) << (0 ?
 27:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28))) | (((gctUINT32) ((gctUINT32) (10) & ((gctUINT32) ((((1 ? 31:28) - (0 ?
 31:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:28) - (0 ? 31:28) + 1))))))) << (0 ?
 31:28)));


    /* Compute centroids. */
    halti5_helper_computeCentroids(
        devCtx,
        1, &chipModule->sampleCoords2, &chipModule->centroids2
        );

    halti5_helper_computeCentroids(
        devCtx,
        3, chipModule->sampleCoords4, chipModule->centroids4
        );

    /* Compute sampleLocations. */
    for (i = 0; i < 4; i++)
    {
        uint32_t sampleCoords = chipModule->sampleCoords4[0];
        sampleCoords = sampleCoords >> (8 * i);
        chipModule->sampleLocations[i][0] = (((((gctUINT32) (sampleCoords)) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) ) / 16.0f;
        chipModule->sampleLocations[i][1] = (((((gctUINT32) (sampleCoords)) >> (0 ? 7:4)) & ((gctUINT32) ((((1 ? 7:4) - (0 ? 7:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:4) - (0 ? 7:4) + 1)))))) ) / 16.0f;
    }


    chipModule->curInitCmdIndex = 0;

    pCmdBuffer = pCmdBufferBegin = &chipModule->initialCmds[chipModule->curInitCmdIndex];

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F6, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:1) - (0 ?
 1:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:1) - (0 ? 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1))));

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0380, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

    limit = 1.0f / 8388607.0f;

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x028B, VK_FALSE, *(uint32_t *)&limit);

    __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0221, VK_FALSE, 0x0808);

    if (devCtx->database->REG_Halti5)
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5310, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));
    }
    else
    {
        __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0E05, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 4:4) - (0 ? 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4))));
    }

    chipModule->curInitCmdIndex = (uint32_t)(pCmdBuffer - pCmdBufferBegin);

    __VK_ASSERT(chipModule->curInitCmdIndex <= HALTI5_INIT_CMD_SIZE_INUINT);

    if (devCtx->database->REG_Halti5)
    {
        chipModule->minorTable.helper_convertHwTxDesc = halti5_helper_convertHwTxDesc;
        chipModule->minorTable.helper_convertHwSampler = halti5_helper_convertHwSampler;
        chipModule->minorTable.helper_setSamplerStates = halti5_helper_setSamplerStates;
        chipModule->minorTable.pip_emit_vsinput = halti5_pip_emit_vsinput;
    }
    else if (devCtx->database->REG_Halti3 && devCtx->database->REG_TX6bitFrac)
    {

        chipModule->minorTable.helper_convertHwTxDesc = halti3_helper_convertHwTxDesc;
        chipModule->minorTable.helper_convertHwSampler = halti3_helper_convertHwSampler;
        chipModule->minorTable.helper_setSamplerStates = halti3_helper_setSamplerStates;
        chipModule->minorTable.pip_emit_vsinput = halti2_pip_emit_vsinput;
    }
    else
    {
        chipModule->minorTable.helper_convertHwTxDesc = halti2_helper_convertHwTxDesc;
        chipModule->minorTable.helper_convertHwSampler = halti2_helper_convertHwSampler;
        chipModule->minorTable.helper_setSamplerStates = halti2_helper_setSamplerStates;
        chipModule->minorTable.pip_emit_vsinput = halti2_pip_emit_vsinput;
    }

    __VK_MEMZERO(&decodeInfo, sizeof(decodeInfo));
    decodeInfo.binary = (gctUINT *)gc_halti5_patchlib_frag;
    decodeInfo.sizeInByte = (gctUINT)sizeof(gc_halti5_patchlib_frag);
    decodeInfo.stageInfo = gcvNULL;
    decodeInfo.specFlag = SPV_SPECFLAG_NONE;
    decodeInfo.tcsInputVertices = 0;

    __VK_ONERROR((gcvSTATUS_OK == gcSPV_Decode(&decodeInfo, &chipModule->patchLib)) ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

    __VK_MEMZERO(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENVK;
    vscCompileParams.cfg.ctx.appNameId = gcvPATCH_INVALID;
    vscCompileParams.cfg.ctx.isPatchLib = gcvTRUE;
    vscCompileParams.cfg.ctx.pSysCtx = &devCtx->vscSysCtx;
    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                | VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO
                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
    vscCompileParams.cfg.optFlags = (VSC_COMPILER_OPT_FULL & (~VSC_COMPILER_OPT_ILF_LINK)) | VSC_COMPILER_OPT_NO_ILF_LINK;
    vscCompileParams.hShader = chipModule->patchLib;

    __VK_ONERROR((gcvSTATUS_OK == vscCompileShader(&vscCompileParams, gcvNULL))
                                ? VK_SUCCESS : VK_ERROR_INCOMPATIBLE_DRIVER);

#if __VK_SAVE_THEN_LOAD_SHADER_BIN
    __VK_ONERROR(__vkSaveThenLoadShaderBin(&devCtx->memCb, &chipModule->patchLib));
#endif

    devCtx->chipPriv = chipModule;

    __VK_ONERROR(halti5_tweak_detect(devCtx));

    return VK_SUCCESS;

OnError:
    if (chipModule->patchLib)
    {
        vscDestroyShader(chipModule->patchLib);
        chipModule->patchLib = VK_NULL_HANDLE;
    }

    if (chipModule)
    {
        __VK_FREE(chipModule);
        devCtx->chipPriv = VK_NULL_HANDLE;
    }
    return result;
}

VkResult halti5_finalizeChipModule(
    VkDevice device
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    halti5_module *chipModule = (halti5_module *)devCtx->chipPriv;
    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    halti5_finalizeComputeBlit(devCtx);

    if (chipModule && (chipModule->patchLib))
    {
        vscDestroyShader(chipModule->patchLib);
    }

    if (chipModule && chipModule->ppTweakHandlers)
    {
        __VK_FREE(chipModule->ppTweakHandlers);
    }

    __VK_FREE(devCtx->chipPriv);

    return VK_SUCCESS;

}

VkResult halti5_helper_set_viewport(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    VkViewport *viewport
    )
{
    gcuFLOAT_UINT32 xScale, yScale, xOffset, yOffset, zOffset, zScale;
    float    wClip;
    /* Compute viewport states. */
    xScale.f  = (viewport->width) / 2.0f;
    xOffset.f = viewport->x + xScale.f;

    yScale.f = (viewport->height) / 2.0f;
    yOffset.f = viewport->y + yScale.f;

    zOffset.f = viewport->minDepth;
    zScale.f  = (viewport->maxDepth - viewport->minDepth);

    wClip = gcmMAX(viewport->width, viewport->height) / (2.0f * 8388607.0f - 8192.0f);

    __vkCmdLoadSingleHWState(commandBuffer, 0x0280, VK_FALSE, xScale.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0281, VK_FALSE, yScale.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0282, VK_FALSE, zScale.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0283, VK_FALSE, xOffset.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0284, VK_FALSE, yOffset.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0285, VK_FALSE, zOffset.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0501, VK_FALSE, *(uint32_t *)&viewport->minDepth);
    __vkCmdLoadSingleHWState(commandBuffer, 0x0502, VK_FALSE, *(uint32_t *)&viewport->maxDepth);

    __vkCmdLoadSingleHWState(commandBuffer, 0x02A0, VK_FALSE, *(uint32_t*)&wClip);

    return VK_SUCCESS;
}

VkResult halti5_helper_set_depthBias(
    __vkDevContext *devCtx,
    uint32_t depthFormat,
    uint32_t **commandBuffer,
    VkBool32 depthBiasEnable,
    __vkDynamicDepthBiasState *depthBiasState
    )
{
    gcuFLOAT_UINT32 scale, bias;

    if (depthBiasEnable)
    {
        switch ((int) depthFormat)
        {
        case VK_FORMAT_D16_UNORM:
            bias.f = (depthBiasState->depthBiasConstantFactor * 2) / 65535.0f;
            break;
        case __VK_FORMAT_D24_UNORM_S8_UINT_PACKED32:
        case __VK_FORMAT_D24_UNORM_X8_PACKED32:
            bias.f = (depthBiasState->depthBiasConstantFactor * 2) / 16777215.0f;
            break;
        default:
            __VK_ASSERT(0);
            bias.f = 0.0f;
            break;
        }
        scale.f = depthBiasState->depthBiasSlopeFactor;

    }
    else
    {
        scale.u = bias.u = 0;
    }

    __vkCmdLoadSingleHWState(commandBuffer, 0x0304, VK_FALSE, scale.u);

    __vkCmdLoadSingleHWState(commandBuffer, 0x0305, VK_FALSE, bias.u);

    return VK_SUCCESS;
}

VkResult halti5_helper_set_linewidth(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    float lineWidth
    )
{
    gcuFLOAT_UINT32 adjustSub;
    gcuFLOAT_UINT32 adjustAdd;
    adjustSub.f = lineWidth / 2.0f;
    adjustAdd.f = -adjustSub.f + lineWidth;

    __vkCmdLoadSingleHWState(commandBuffer, 0x028E, VK_FALSE, adjustSub.u);
    __vkCmdLoadSingleHWState(commandBuffer, 0x028F, VK_FALSE, adjustAdd.u);
    return VK_SUCCESS;
}

VkResult halti5_helper_set_stencilStates(
    __vkDevContext *devCtx,
    VkFrontFace frontFace,
    uint32_t **commandBuffer,
    __vkDynamicStencilState *stencilState,
    uint32_t stencilMode
    )
{
    if (frontFace == VK_FRONT_FACE_CLOCKWISE)
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x0507, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (stencilMode) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (stencilState->reference[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16))) | (((gctUINT32) ((gctUINT32) (stencilState->compareMask[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24))) | (((gctUINT32) ((gctUINT32) (stencilState->compareMask[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24)))
            );

        __vkCmdLoadSingleHWState(commandBuffer, 0x0528, VK_FALSE,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (stencilState->reference[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))));

        __vkCmdLoadSingleHWState(commandBuffer, 0x052E, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (stencilState->writeMask[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (stencilState->compareMask[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))));
    }
    else
    {
       __vkCmdLoadSingleHWState(commandBuffer, 0x0507, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (stencilMode) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (stencilState->reference[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16))) | (((gctUINT32) ((gctUINT32) (stencilState->compareMask[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24))) | (((gctUINT32) ((gctUINT32) (stencilState->writeMask[__VK_FACE_BACK]) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24)))
            );

        __vkCmdLoadSingleHWState(commandBuffer, 0x0528, VK_FALSE,
            (((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (stencilState->reference[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) &((((gctUINT32) (~0U)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 8:8) - (0 ? 8:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ? 8:8)))));

        __vkCmdLoadSingleHWState(commandBuffer, 0x052E, VK_FALSE,
              ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8))) | (((gctUINT32) ((gctUINT32) (stencilState->writeMask[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 15:8) - (0 ? 15:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:8) - (0 ? 15:8) + 1))))))) << (0 ?
 15:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (stencilState->compareMask[__VK_FACE_FRONT]) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))));
    }
    return VK_SUCCESS;
}

VkResult halti5_helper_set_blendConstants(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkDynamicColorBlendState *blendState,
    uint32_t attachmentCount
    )
{
    uint32_t i;
    gctUINT32 colorLow;
    gctUINT32 colorHigh;
    gctUINT16 colorR;
    gctUINT16 colorG;
    gctUINT16 colorB;
    gctUINT16 colorA;

    colorR = gcoMATH_FloatToFloat16(*(gctUINT *)&blendState->blendConstants[0]);
    colorG = gcoMATH_FloatToFloat16(*(gctUINT *)&blendState->blendConstants[1]);
    colorB = gcoMATH_FloatToFloat16(*(gctUINT *)&blendState->blendConstants[2]);
    colorA = gcoMATH_FloatToFloat16(*(gctUINT *)&blendState->blendConstants[3]);

    colorLow = colorR | (colorG << 16);
    colorHigh = colorB | (colorA << 16);

    for (i = 0; i < attachmentCount; i++)
    {
        if (0 == i)
        {
            __vkCmdLoadSingleHWState(commandBuffer, 0x052C, VK_FALSE, colorLow);
            __vkCmdLoadSingleHWState(commandBuffer, 0x052D, VK_FALSE, colorHigh);
        }
        else
        {
            __vkCmdLoadSingleHWState(commandBuffer, 0x5260 + (i-1), VK_FALSE, colorLow);
            __vkCmdLoadSingleHWState(commandBuffer, 0x5268 + (i-1), VK_FALSE, colorHigh);
        }
    }

    return VK_SUCCESS;
}


VkResult halti5_helper_convertHwPEDesc(
    uint32_t vkFormat,
    HwPEDesc *hwPEDesc
    )
{
    uint32_t i;
    static const struct
    {
        uint32_t vkFormat;
        HwPEDesc hwDesc;
    }
    s_vkFormatToHwPeDescs[] =
    {
        {__VK_FORMAT_A4R4G4B4_UNFORM_PACK16, {0x01, VK_FALSE, VK_TRUE, 0x0}},
        {__VK_FORMAT_R8_1_X8R8G8B8, {0x05, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_A1R5G5B5_UNORM_PACK16, {0x03, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_R5G6B5_UNORM_PACK16, {0x04, VK_FALSE, VK_TRUE, 0x0}},

        {VK_FORMAT_R8_UNORM, {0x23, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_R8_UINT, {0x17, VK_FALSE, VK_FALSE, 0x3}},
        {VK_FORMAT_R8_SINT, {0x17, VK_FALSE, VK_FALSE, 0x5}},
        {VK_FORMAT_R8_SRGB, {0x23, VK_TRUE, VK_TRUE, 0x0}},
        {VK_FORMAT_R8G8_UNORM, {0x1F, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_R8G8_UINT, {0x18, VK_FALSE, VK_FALSE, 0x3}},
        {VK_FORMAT_R8G8_SINT, {0x18, VK_FALSE, VK_FALSE, 0x5}},
        {VK_FORMAT_R8G8_SRGB, {0x1F, VK_TRUE, VK_TRUE, 0x0}},

        {VK_FORMAT_R8G8B8A8_UNORM, {0x06, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_B8G8R8A8_UNORM, {0x06, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_R8G8B8A8_UINT, {0x19, VK_FALSE, VK_FALSE, 0x3}},
        {VK_FORMAT_R8G8B8A8_SINT, {0x19, VK_FALSE, VK_FALSE, 0x5}},
        {VK_FORMAT_B8G8R8A8_SRGB, {0x06, VK_TRUE, VK_TRUE, 0x0}},

        {VK_FORMAT_A8B8G8R8_UINT_PACK32, {0x19, VK_FALSE, VK_FALSE, 0x3}},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32, {0x19, VK_FALSE, VK_FALSE, 0x5}},

        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, {0x16, VK_FALSE, VK_TRUE, 0x0}},
        {VK_FORMAT_A2B10G10R10_UINT_PACK32, {0x1E, VK_FALSE, VK_FALSE, 0x4}},
        {VK_FORMAT_A2B10G10R10_SINT_PACK32, {0x1E, VK_FALSE, VK_FALSE, 0x6}},

        {VK_FORMAT_R16_UINT, {0x1A, VK_FALSE, VK_FALSE, 0x4}},
        {VK_FORMAT_R16_SINT, {0x1A, VK_FALSE, VK_FALSE, 0x6}},
        {VK_FORMAT_R16_SFLOAT, {0x11, VK_FALSE, VK_FALSE, 0x0}},

        {VK_FORMAT_R16G16_UINT, {0x1B, VK_FALSE, VK_FALSE, 0x4}},
        {VK_FORMAT_R16G16_SINT, {0x1B, VK_FALSE, VK_FALSE, 0x6}},
        {VK_FORMAT_R16G16_SFLOAT, {0x12, VK_FALSE, VK_FALSE, 0x0}},

        {VK_FORMAT_R16G16B16A16_UINT, {0x1C, VK_FALSE, VK_FALSE, 0x4}},
        {VK_FORMAT_R16G16B16A16_SINT, {0x1C, VK_FALSE, VK_FALSE, 0x6}},
        {VK_FORMAT_R16G16B16A16_SFLOAT, {0x13, VK_FALSE, VK_FALSE, 0x0}},

        {VK_FORMAT_R32_UINT, {0x14, VK_FALSE, VK_FALSE, 0x2}},
        {VK_FORMAT_R32_SINT, {0x14, VK_FALSE, VK_FALSE, 0x2}},
        {VK_FORMAT_R32_SFLOAT, {0x14, VK_FALSE, VK_FALSE, 0x2}},

        {VK_FORMAT_R32G32_UINT, {0x15, VK_FALSE, VK_FALSE, 0x2}},
        {VK_FORMAT_R32G32_SINT, {0x15, VK_FALSE, VK_FALSE, 0x2}},
        {VK_FORMAT_R32G32_SFLOAT, {0x15, VK_FALSE, VK_FALSE, 0x2}},

        {VK_FORMAT_B10G11R11_UFLOAT_PACK32, {0x1D, VK_FALSE, VK_FALSE, 0x0}},

        {__VK_FORMAT_R32G32B32A32_UINT_2_R32G32_UINT, {0x15, VK_FALSE, VK_FALSE, 0x2}},
        {__VK_FORMAT_R32G32B32A32_SINT_2_R32G32_SINT, {0x15, VK_FALSE, VK_FALSE, 0x2}},
        {__VK_FORMAT_R32G32B32A32_SFLOAT_2_R32G32_SFLOAT, {0x15, VK_FALSE, VK_FALSE, 0x2}},

    };

    for (i = 0; i < __VK_COUNTOF(s_vkFormatToHwPeDescs); i++)
    {
        if (s_vkFormatToHwPeDescs[i].vkFormat == vkFormat)
        {
            break;
        }
    }

    if (i < __VK_COUNTOF(s_vkFormatToHwPeDescs))
    {
        __VK_ASSERT(hwPEDesc);
        *hwPEDesc = s_vkFormatToHwPeDescs[i].hwDesc;

        return VK_SUCCESS;
    }

    return VK_ERROR_FORMAT_NOT_SUPPORTED;
}



