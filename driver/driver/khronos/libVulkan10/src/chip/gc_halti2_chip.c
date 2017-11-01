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

VkResult halti5_computeClear(
    VkCommandBuffer cmdBuf,
    VkClearValue *clearValue,
    __vkBlitRes *dstRes
    );

VkResult halti2_pip_emit_vsinput(
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

    __VK_ASSERT(devCtx->database->REG_Generics);

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
        }
    }

    /* when vs has any input entries, it includes vertexID/instanceID*/
    if (shAttribTableEntries)
    {
        HwVertexAttribDesc hwVertexAttribDesc[__VK_MAX_VERTEX_ATTRIBS];
        uint32_t vsInputCount = 0;
        uint32_t vsInputState = 0;
        uint32_t vsInputHWRegAddr = 0x0208;
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
                uint32_t location = shAttribTable[entryIdx].pLocation[compositeIdx];
                for (attribIdx = 0; attribIdx < vsInputInfo->vertexAttributeDescriptionCount; attribIdx++)
                {
                    if (vsInputInfo->pVertexAttributeDescriptions[attribIdx].location == location)
                    {
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

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0180 + attribIdx, VK_FALSE,
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwSize) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwDataType) & ((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:14) - (0 ?
 15:14) + 1))))))) << (0 ? 15:14))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwNormalized) & ((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:14) - (0 ?
 15:14) + 1))))))) << (0 ? 15:14)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ? 6:6)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].sortedAttributeDescPtr->binding) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].sortedAttributeDescPtr->offset) & ((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwFetchSize) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (hwVertexAttribDesc[attribIdx].hwFetchBreak) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))));

            __vkCmdLoadSingleHWState(&pCmdBuffer, 0x5E80 + attribIdx, VK_FALSE, hwVertexAttribDesc[attribIdx].integer ? 1 : 0x3F800000);

            /* Set vertex shader input linkage. */
            switch (vsInputCount & 3)
            {
            case 0:
            default:
                vsInputState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
                break;
            case 1:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ? 13:8) - (0 ?
 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
                break;
            case 2:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ?
 21:16) + 1))))))) << (0 ? 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ?
 21:16) + 1))))))) << (0 ? 21:16)));
                break;
            case 3:
                vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ?
 29:24) + 1))))))) << (0 ? 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegNo) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ?
 29:24) + 1))))))) << (0 ? 29:24)));
                /* Store the current shader input control value. */
                __vkCmdLoadSingleHWState(&pCmdBuffer, vsInputHWRegAddr, VK_FALSE, vsInputState);
                vsInputHWRegAddr++;
                break;
            }
            vsInputCount++;
        }

        /* program vs input for ID */
        if ((vsInputCount & 3) || (hwRegForID != ~0U))
        {
            /* zero app-defined attributes */
            if (!vsInputCount)
            {
                if(devCtx->database->REG_Halti4) /*gcvFEATURE_ZERO_ATTRIB_SUPPORT*/
                {
                  __vkCmdLoadSingleHWState(&pCmdBuffer, 0x01F2, VK_FALSE, 1);
                }
                else
                {
                __vkCmdLoadSingleHWState(&pCmdBuffer, 0x0180, VK_FALSE,
                    ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:14) - (0 ?
 15:14) + 1))))))) << (0 ? 15:14))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 15:14) - (0 ? 15:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:14) - (0 ?
 15:14) + 1))))))) << (0 ? 15:14)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ? 5:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:4) - (0 ?
 5:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ?
 5:4)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:6) - (0 ? 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 6:6) - (0 ?
 6:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:6) - (0 ? 6:6) + 1))))))) << (0 ?
 6:6)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:16) - (0 ? 23:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:16) - (0 ?
 23:16) + 1))))))) << (0 ? 23:16)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24))) | (((gctUINT32) ((gctUINT32) (4) & ((gctUINT32) ((((1 ?
 31:24) - (0 ? 31:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:24) - (0 ?
 31:24) + 1))))))) << (0 ? 31:24)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))));

                vsInputState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
                vsInputCount++;
                }
            }
            if (hwRegForID != ~0U)
            {
                /* Set vertex shader input linkage. */
                switch (vsInputCount & 3)
                {
                case 0:
                default:
                    vsInputState = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)));
                    break;
                case 1:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 13:8) - (0 ? 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8)));
                    break;
                case 2:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ?
 21:16) + 1))))))) << (0 ? 21:16))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 21:16) - (0 ? 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ?
 21:16) + 1))))))) << (0 ? 21:16)));
                    break;
                case 3:
                    vsInputState = ((((gctUINT32) (vsInputState)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ?
 29:24) + 1))))))) << (0 ? 29:24))) | (((gctUINT32) ((gctUINT32) (hwRegForID) & ((gctUINT32) ((((1 ?
 29:24) - (0 ? 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ?
 29:24) + 1))))))) << (0 ? 29:24)));
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
        __VK_ASSERT(0);
    }

    chipGfxPipeline->chipPipeline.curCmdIndex += (uint32_t)(pCmdBuffer - pCmdBufferBegin);
    return VK_SUCCESS;
}


static VkResult halti2_helper_convertHwRsDesc(
    uint32_t vkFormat,
    HwRsDesc *hwRsDesc
    )
{
    uint32_t format = 0;
    VkBool32 bFakeFormat = VK_FALSE;
    VkBool32 flipRB = VK_FALSE;
    gceMSAA_DOWNSAMPLE_MODE downsampleMode = gcvMSAA_DOWNSAMPLE_AVERAGE;
    uint32_t i;
    gctUINT32 bitsPerPixel;
    static const struct
    {
        uint32_t vkFormat;
        uint32_t hwFormat;
        VkBool32 flipRB;
    }
    s_vkFormatToHwRsDescs[] =
    {
        {__VK_FORMAT_A4R4G4B4_UNFORM_PACK16, 0x01, VK_FALSE},
        {__VK_FORMAT_R8_1_X8R8G8B8, 0x05, VK_FALSE},
        {VK_FORMAT_R5G6B5_UNORM_PACK16, 0x04, VK_FALSE},
        {VK_FORMAT_B5G6R5_UNORM_PACK16, 0x04, VK_TRUE},
        {VK_FORMAT_A1R5G5B5_UNORM_PACK16, 0x03, VK_FALSE},
        {VK_FORMAT_B8G8R8A8_UNORM, 0x06, VK_FALSE},
        {VK_FORMAT_R8G8B8A8_UNORM, 0x06, VK_TRUE},
        {VK_FORMAT_A2R10G10B10_UNORM_PACK32, 0x16, VK_FALSE},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, 0x16, VK_TRUE},
        {VK_FORMAT_R8G8B8A8_SRGB, 0x06, VK_TRUE},
        {VK_FORMAT_A8B8G8R8_SRGB_PACK32, 0x06, VK_TRUE},
        {VK_FORMAT_A8B8G8R8_UNORM_PACK32, 0x06, VK_TRUE},
    };
    bitsPerPixel = g_vkFormatInfoTable[vkFormat].bitsPerBlock / g_vkFormatInfoTable[vkFormat].partCount;

    __VK_ASSERT(hwRsDesc);
    hwRsDesc->pixelSize = bitsPerPixel;

    for (i = 0; i < __VK_COUNTOF(s_vkFormatToHwRsDescs); i++)
    {
        if (vkFormat == s_vkFormatToHwRsDescs[i].vkFormat)
        {
            format = s_vkFormatToHwRsDescs[i].hwFormat;
            flipRB = s_vkFormatToHwRsDescs[i].flipRB;
            break;
        }
    }

    if (i >= __VK_COUNTOF(s_vkFormatToHwRsDescs))
    {
        /* Fake format.*/
        switch (bitsPerPixel)
        {
        case 8:
            format = 0x10;
            hwRsDesc->pixelSize = 8;
            break;
        case 16:
            format = 0x01;
            hwRsDesc->pixelSize = 16;
            break;
        case 32:
            format = 0x06;
            hwRsDesc->pixelSize = 32;
            break;
        default:
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        }

        /* set return value.*/
        bFakeFormat = gcvTRUE;
    }

    hwRsDesc->hwFormat       = format;
    hwRsDesc->downSampleMode = downsampleMode;
    hwRsDesc->flipRB         = flipRB;
    hwRsDesc->fakeFormat     = bFakeFormat;

    return VK_SUCCESS;
}

__VK_INLINE void rsConfigTiling(
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


VkResult halti2_clearImageWithRS(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageSubresource *subResource,
    VkClearValue *clearValue,
    VkRect2D *rect
    )
{
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, image);
    __vkImageLevel *pLevel = &img->pImgLevels[subResource->mipLevel];
    uint32_t address;
    uint32_t clearVals[2] = {0}, clearMasks[2] = {0};
    uint32_t hwConfig, hwDstStride;
    uint32_t dstStride, dstTiling, dstSuperTile;
    uint32_t *states = gcvNULL;
    VkResult result;
    uint32_t partIndex = 0;
    uint32_t hwFormat;
    uint32_t ditherTable[2] = { ~0U, ~0U };
    uint32_t hwSrcStride, hwControl;
    uint32_t hwWindowSize, hwOffset;

    __VK_ASSERT(((__vkCommandBuffer *)commandBuffer)->devCtx->option->affinityMode != __VK_MGPU_AFFINITY_COMBINE);

    if ((rect->offset.x & 0x3)     || (rect->offset.y & 0x3) ||
        (rect->extent.width & 0xF) || (rect->extent.height & 0x3))
    {
        __vkBlitRes dstRes;

        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = img;
        dstRes.u.img.subRes.aspectMask = subResource->aspectMask;
        dstRes.u.img.subRes.mipLevel = subResource->mipLevel;
        dstRes.u.img.offset.x = rect->offset.x;
        dstRes.u.img.offset.y = rect->offset.y;
        dstRes.u.img.offset.z = 1;
        dstRes.u.img.extent.width = rect->extent.width;
        dstRes.u.img.extent.height = rect->extent.height;
        dstRes.u.img.extent.depth = 1;
        dstRes.u.img.subRes.arrayLayer = subResource->arrayLayer;

        return (halti5_computeClear(commandBuffer, clearValue, &dstRes));
    }

    switch (img->formatInfo.bitsPerBlock / img->formatInfo.partCount)
    {
    case 8:
        __VK_ASSERT(((__vkCommandBuffer *)commandBuffer)->devCtx->database->REG_RSS8);
        hwFormat = 0x10;
        break;
    case 16:
        hwFormat = 0x01;
        break;
    case 32:
        hwFormat = 0x06;
        break;
    case 64:
        hwFormat = 0x15;
        break;
    default:
        __VK_ASSERT(0);
        hwFormat = 0x06;
        break;
    }

    rsConfigTiling(img, &dstTiling, &dstSuperTile);

    while (partIndex < pLevel->partCount)
    {
        __VK_ONERROR(__vkComputeClearVal(img,
            subResource->aspectMask,
            clearValue,
            partIndex,
            clearVals,
            VK_NULL_HANDLE,
            clearMasks));

        hwConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (hwFormat) & ((gctUINT32) ((((1 ? 4:0) - (0 ?
 4:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (hwFormat) & ((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:14) - (0 ?
 14:14) + 1))))))) << (0 ? 14:14))) | (((gctUINT32) ((gctUINT32) (dstTiling) & ((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:14) - (0 ?
 14:14) + 1))))))) << (0 ? 14:14)));

        hwSrcStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29)));

        hwControl = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((clearMasks[0] | (clearMasks[1] << 4))) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:16) - (0 ?
 17:16) + 1))))))) << (0 ? 17:16))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 17:16) - (0 ? 17:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:16) - (0 ?
 17:16) + 1))))))) << (0 ? 17:16)));

        dstStride = (uint32_t)pLevel->stride * (dstTiling == 0x0 ? 1 : 4);

        hwDstStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0))) | (((gctUINT32) ((gctUINT32) (dstStride) & ((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (dstSuperTile) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30)))
                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:27) - (0 ?
 28:27) + 1))))))) << (0 ? 28:27))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:27) - (0 ?
 28:27) + 1))))))) << (0 ? 28:27)));

        hwOffset = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (rect->offset.x * img->sampleInfo.x) & ((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16))) | (((gctUINT32) ((gctUINT32) (rect->offset.y * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16)));

        hwWindowSize = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (rect->extent.width * img->sampleInfo.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
                     | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (rect->extent.height * img->sampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)));

        address = img->memory->devAddr;
        address += (uint32_t)(img->memOffset
            + pLevel->partSize * partIndex
            + pLevel->offset
            + subResource->arrayLayer * pLevel->sliceSize);

        __vk_CmdAquireBuffer(commandBuffer, 10 * 2 + 8, &states);

        __vkCmdLoadSingleHWState(&states, 0x0581, VK_FALSE, hwConfig);

        __vkCmdLoadBatchHWStates(&states, 0x058C, VK_FALSE, 2, ditherTable);

        __vkCmdLoadSingleHWState(&states, 0x0585, VK_FALSE, hwDstStride);

        __vkCmdLoadSingleHWState(&states, 0x0583, VK_FALSE, hwSrcStride);

        __vkCmdLoadBatchHWStates(&states, 0x0590, VK_FALSE, 2, clearVals);

        __vkCmdLoadSingleHWState(&states, 0x058F, VK_FALSE, hwControl);

        __vkCmdLoadSingleHWState(&states, 0x05A8, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ?
 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 20:20) - (0 ?
 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))));

        __VK_ASSERT(((__vkCommandBuffer *)commandBuffer)->devCtx->database->RS_NEW_BASEADDR);

        __vkCmdLoadSingleHWState(&states, 0x05B8, VK_FALSE, address);

        __vkCmdLoadSingleHWState(&states, 0x0588, VK_FALSE, hwWindowSize);

        __vkCmdLoadSingleHWState(&states, 0x05C0, VK_FALSE, hwOffset);

        __vkCmdLoadSingleHWState(&states, 0x05AE, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (VK_TRUE) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))));

        __vkCmdLoadSingleHWState(&states, 0x0580, VK_FALSE, 0xBADABEEB);

        __vk_CmdReleaseBuffer(commandBuffer, 10 * 2 + 8);

        partIndex++;

    }
    return VK_SUCCESS;
OnError:
    return result;
}

VkResult halti2_copyImageWithRS(
    VkCommandBuffer commandBuffer,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy
    )
{
    HwRsDesc srcRsDesc = {0}, dstRsDesc = {0};
    uint32_t srcAddress, dstAddress;
    uint32_t srcSuperTile, srcTiling, dstSuperTile, dstTiling;
    uint32_t *states = gcvNULL;
    gcsSAMPLES srcSampleInfo = {0}, dstSampleInfo = {0};
    uint32_t srcStride = 0, dstStride = 0;
    uint32_t srcFormat, dstFormat;
    VkBool32 srcFmtFaked = VK_FALSE, dstFmtFaked = VK_FALSE;
    uint32_t srcParts, dstParts;
    VkOffset3D srcOffset, dstOffset;
    VkExtent3D srcExtent, dstExtent;
    VkBool32 useComputeBlit = VK_FALSE;
    VkBool32 srcMsaa, dstMsaa;
    VkResult result = VK_SUCCESS;
    uint32_t hwConfig, hwSrcStride, hwDstStride, hwOffset, hwWindowSize;
    uint32_t ditherTable[2] = { ~0U, ~0U };

    if (!srcRes->isImage && !dstRes->isImage)
    {
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    __VK_ASSERT(((__vkCommandBuffer *)commandBuffer)->devCtx->option->affinityMode != __VK_MGPU_AFFINITY_COMBINE);

    if (srcRes->isImage)
    {
        __vkImage *srcImg = srcRes->u.img.pImage;
        __vkImageLevel *pSrcLevel = &srcImg->pImgLevels[srcRes->u.img.subRes.mipLevel];

        srcOffset = srcRes->u.img.offset;
        srcExtent = srcRes->u.img.extent;

        srcStride = (uint32_t)pSrcLevel->stride;
        srcSampleInfo = srcImg->sampleInfo;
        srcFormat = srcImg->formatInfo.residentImgFormat;
        srcFmtFaked = (srcFormat != (uint32_t)srcImg->createInfo.format);
        srcParts = srcImg->formatInfo.partCount;
        srcAddress = srcImg->memory->devAddr;
        srcAddress += (uint32_t)(srcImg->memOffset + pSrcLevel->offset +
                                 srcRes->u.img.subRes.arrayLayer * pSrcLevel->sliceSize);
        srcMsaa = (srcImg->sampleInfo.product > 1);
        rsConfigTiling(srcImg, &srcTiling, &srcSuperTile);
    }
    else
    {
        uint32_t srcWidth;
        const __vkFormatInfo *fmtInfo;
        __vkBuffer *srcBuf = srcRes->u.buf.pBuffer;
        __vkImage *dstImg = dstRes->u.img.pImage;

        srcOffset.x = srcOffset.y = srcOffset.z = 0;
        srcExtent = dstRes->u.img.extent;

        srcAddress = srcBuf->memory->devAddr;
        srcAddress += (uint32_t)(srcBuf->memOffset + srcRes->u.buf.offset);
        srcTiling    = 0x0;
        srcSuperTile = 0x0;
        srcMsaa      = VK_FALSE;

        srcFormat = dstImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[srcFormat];
        srcParts = fmtInfo->partCount;
        srcWidth  = srcRes->u.buf.rowLength != 0 ? srcRes->u.buf.rowLength : dstRes->u.img.extent.width;
        srcStride = (srcWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        srcSampleInfo = dstImg->sampleInfo;
    }

    if (dstRes->isImage)
    {
        __vkImage *dstImg = dstRes->u.img.pImage;
        __vkImageLevel *pDstLevel = &dstImg->pImgLevels[dstRes->u.img.subRes.mipLevel];

        dstOffset = dstRes->u.img.offset;
        dstExtent = dstRes->u.img.extent;

        dstStride = (uint32_t)pDstLevel->stride;
        dstSampleInfo = dstImg->sampleInfo;
        dstFormat = dstImg->formatInfo.residentImgFormat;
        dstFmtFaked = (dstFormat != (uint32_t)dstImg->createInfo.format);
        dstParts = dstImg->formatInfo.partCount;
        dstAddress = dstImg->memory->devAddr;
        dstAddress += (uint32_t)(dstImg->memOffset + pDstLevel->offset +
                                 dstRes->u.img.subRes.arrayLayer * pDstLevel->sliceSize);
        dstMsaa = (dstImg->sampleInfo.product > 1);
        rsConfigTiling(dstImg, &dstTiling, &dstSuperTile);
    }
    else
    {
        uint32_t dstWidth;
        const __vkFormatInfo *fmtInfo;
        __vkBuffer *dstBuf = dstRes->u.buf.pBuffer;
        __vkImage *srcImg = srcRes->u.img.pImage;

        dstOffset.x = dstOffset.y = dstOffset.z = 0;
        dstExtent = srcRes->u.img.extent;

        dstAddress = dstBuf->memory->devAddr;
        dstAddress += (uint32_t)(dstBuf->memOffset + dstRes->u.buf.offset);
        dstTiling    = 0x0;
        dstSuperTile = 0x0;
        dstMsaa      = VK_FALSE;

        dstFormat = srcImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[dstFormat];
        dstParts = fmtInfo->partCount;
        dstWidth  = dstRes->u.buf.rowLength != 0 ? dstRes->u.buf.rowLength : srcRes->u.img.extent.width;
        dstStride = (dstWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        dstSampleInfo = srcImg->sampleInfo;
    }

    if (rawCopy && !srcFmtFaked && !dstFmtFaked)
    {
        /* Change srcFormat to be same as dstFormat for CmdCopyImage() */
        srcFormat = dstFormat;
    }

    if (srcParts != dstParts)
    {
        useComputeBlit = VK_TRUE;
    }
    else
    {
        if (!__VK_IS_SUCCESS(halti2_helper_convertHwRsDesc(srcFormat, &srcRsDesc)) ||
            !__VK_IS_SUCCESS(halti2_helper_convertHwRsDesc(dstFormat, &dstRsDesc)))
        {
            useComputeBlit = VK_TRUE;
        }

        /* Need special deal with if copying to block compressed texture. treat as linear to upload.
           One block is treated as one or two pixel to upload. It is required to handle compressed
           images created with dimensions that are not a multiple of the block dimensions.
        */
        if (dstFormat >= VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK &&
            dstFormat <= VK_FORMAT_EAC_R11G11_SNORM_BLOCK
            )
        {

            VkExtent2D rect;
            const __vkFormatInfo *fmtInfo;
            fmtInfo = &g_vkFormatInfoTable[dstFormat];
            rect = fmtInfo->blockSize;

            dstTiling = 0x0;
            dstSuperTile = 0x0;
            srcOffset.x = gcmALIGN(srcOffset.x - rect.width + 1, rect.width);
            srcOffset.y  = gcmALIGN(srcOffset.y - rect.height + 1, rect.height);
            dstOffset.x = gcmALIGN(dstOffset.x - rect.width + 1, rect.width);
            dstOffset.y = gcmALIGN(dstOffset.y - rect.height + 1, rect.height);
            srcExtent.width = gcmALIGN(srcExtent.width, rect.width) / rect.width;
            srcExtent.height = gcmALIGN(srcExtent.height, rect.width) / rect.height;
            dstExtent.width = gcmALIGN(dstExtent.width, rect.width) / rect.width;
            dstExtent.height = gcmALIGN(dstExtent.height ,rect.height) / rect.height;

            if(dstRsDesc.pixelSize == 128 )
            {
                dstRsDesc.pixelSize = 64;
                srcRsDesc.pixelSize = 64;
                srcOffset.x *=2;
                dstOffset.x *=2;
                srcExtent.width *=2;
                dstExtent.width *=2;
            }
        }

        if (!useComputeBlit)
        {
            /* Do NOT support stretch copy */
            if (gcoOS_MemCmp(&srcExtent, &dstExtent, gcmSIZEOF(VkExtent2D)))
            {
                useComputeBlit = VK_TRUE;
            }
            else if (gcoOS_MemCmp(&srcOffset, &dstOffset, gcmSIZEOF(VkExtent2D)))
            {
                useComputeBlit = VK_TRUE;
            }
            /* Fake format only works when same format */
            else if ((srcRsDesc.fakeFormat || dstRsDesc.fakeFormat) &&
                     (srcRsDesc.hwFormat != dstRsDesc.hwFormat))
            {
                useComputeBlit = VK_TRUE;
            }
            else if (srcFormat == VK_FORMAT_B4G4R4A4_UNORM_PACK16)
            {
                useComputeBlit = VK_TRUE;
            }
            else if (((srcOffset.x * srcSampleInfo.x) & 0x3) ||
                     ((srcOffset.y * srcSampleInfo.y) & 0x3) ||
                     ((dstOffset.x * dstSampleInfo.x) & 0x3) ||
                     ((dstOffset.y * dstSampleInfo.y) & 0x3))
            {
                useComputeBlit = VK_TRUE;
            }
            else if(((srcExtent.width * srcSampleInfo.x) & 0xF) ||
                    ((srcExtent.height *srcSampleInfo.y) & 0x3))
            {
                useComputeBlit = VK_TRUE;
            }
        }
    }

    if (useComputeBlit)
    {
        return (halti5_computeBlit(commandBuffer, srcRes, dstRes, gcvNULL, VK_FILTER_NEAREST));
    }

    __VK_ASSERT(srcSampleInfo.product >= dstSampleInfo.product);

    hwConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) ((gctUINT32) (srcRsDesc.hwFormat) & ((gctUINT32) ((((1 ?
 4:0) - (0 ? 4:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ?
 4:0)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) (srcTiling) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (dstRsDesc.hwFormat) & ((gctUINT32) ((((1 ?
 12:8) - (0 ? 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:14) - (0 ?
 14:14) + 1))))))) << (0 ? 14:14))) | (((gctUINT32) ((gctUINT32) (dstTiling) & ((gctUINT32) ((((1 ?
 14:14) - (0 ? 14:14) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:14) - (0 ?
 14:14) + 1))))))) << (0 ? 14:14)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29))) | (((gctUINT32) ((gctUINT32) ((srcRsDesc.flipRB ^ dstRsDesc.flipRB)) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) ((gctUINT32) (VK_FALSE) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) ((srcMsaa && !dstMsaa) ? 0x3 : 0x0) & ((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:13) - (0 ?
 13:13) + 1))))))) << (0 ? 13:13))) | (((gctUINT32) ((gctUINT32) (VK_FALSE) & ((gctUINT32) ((((1 ?
 13:13) - (0 ? 13:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:13) - (0 ?
 13:13) + 1))))))) << (0 ? 13:13)));

    srcStride = srcStride * (srcTiling == 0x0 ? 1 : 4);
    dstStride = dstStride * (dstTiling == 0x0 ? 1 : 4);

    /* Make sure the stride not exceed HW caps */
    gcmASSERT(srcStride <= ((gctUINT32) ((((1 ? 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))));
    gcmASSERT(dstStride <= ((gctUINT32) ((((1 ? 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))));

    hwSrcStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0))) | (((gctUINT32) ((gctUINT32) (srcStride) & ((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (srcSuperTile) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29))) | (((gctUINT32) ((gctUINT32) (srcMsaa) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29)));

    hwDstStride = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0))) | (((gctUINT32) ((gctUINT32) (dstStride) & ((gctUINT32) ((((1 ?
 19:0) - (0 ? 19:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:0) - (0 ? 19:0) + 1))))))) << (0 ?
 19:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (dstSuperTile) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30)));

    hwOffset = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0))) | (((gctUINT32) ((gctUINT32) (srcOffset.x * srcSampleInfo.x) & ((gctUINT32) ((((1 ?
 12:0) - (0 ? 12:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:0) - (0 ? 12:0) + 1))))))) << (0 ?
 12:0)))
             | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16))) | (((gctUINT32) ((gctUINT32) (srcOffset.y * srcSampleInfo.y) & ((gctUINT32) ((((1 ?
 28:16) - (0 ? 28:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:16) - (0 ?
 28:16) + 1))))))) << (0 ? 28:16)));

    hwWindowSize = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (srcExtent.width * srcSampleInfo.x) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
                 | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (srcExtent.height * srcSampleInfo.y) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)));

    if (dstTiling == 0x0)
    {
        __VK_ASSERT((dstAddress & 0x3F) == 0);
    }

    if (srcTiling == 0x0)
    {
        __VK_ASSERT((srcAddress & 0x3F) == 0);
    }

    __vk_CmdAquireBuffer(commandBuffer, 11*2 + 4, &states);

    __vkCmdLoadSingleHWState(&states, 0x0581, VK_FALSE, hwConfig);
    __vkCmdLoadSingleHWState(&states, 0x0583, VK_FALSE, hwSrcStride);
    __vkCmdLoadSingleHWState(&states, 0x0585, VK_FALSE, hwDstStride);
    __vkCmdLoadBatchHWStates(&states, 0x058C, VK_FALSE, 2, ditherTable);
    __vkCmdLoadSingleHWState(&states, 0x058F, VK_FALSE, 0);

    __vkCmdLoadSingleHWState(&states, 0x05A8, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 1:0) - (0 ? 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:20) - (0 ?
 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ? 20:20) + 1))))))) << (0 ?
 20:20))) | (((gctUINT32) ((gctUINT32) (VK_TRUE) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ?
 20:20) + 1))))))) << (0 ? 20:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:6) - (0 ?
 7:6) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ?
 7:6))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 7:6) - (0 ? 7:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 7:6) - (0 ? 7:6) + 1))))))) << (0 ? 7:6)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:8) - (0 ?
 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 9:8) - (0 ? 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8))));

    __VK_ASSERT(((__vkCommandBuffer *)commandBuffer)->devCtx->database->RS_NEW_BASEADDR);

    __vkCmdLoadSingleHWState(&states, 0x05B0, VK_FALSE, srcAddress);
    __vkCmdLoadSingleHWState(&states, 0x05B8, VK_FALSE, dstAddress);

    __vkCmdLoadSingleHWState(&states, 0x0588, VK_FALSE, hwWindowSize);

    __vkCmdLoadSingleHWState(&states, 0x05C0, VK_FALSE, hwOffset);

    __vkCmdLoadSingleHWState(&states, 0x05AE, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0))));

    __vkCmdLoadSingleHWState(&states, 0x0580, VK_FALSE, 0xBADABEEB);

    __vk_CmdReleaseBuffer(commandBuffer, 11 * 2 + 4);

    return result;
}


void halti2_helper_convertHwSampler(
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
    __VK_ASSERT(!devCtx->database->REG_TX6bitFrac);

    lodbias = _Float2SignedFixed(createInfo->mipLodBias, 5, 5);
    maxlod = _Float2SignedFixed(createInfo->maxLod, 5, 5);
    minlod = _Float2SignedFixed(__VK_MAX(0.0f, createInfo->minLod), 5, 5);

    if (createInfo->anisotropyEnable)
    {
        __VK_ASSERT(createInfo->maxAnisotropy <= devCtx->pPhyDevice->phyDevProp.limits.maxSamplerAnisotropy);
        anisoLog = gcoMATH_Log2in5dot5(gcoMATH_Float2UInt(createInfo->maxAnisotropy));
    }

    hwSamplerDesc->halti2.anisoLog = anisoLog;

    hwSamplerDesc->halti2.hwSamplerMode_p0 =
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:3) - (0 ?
 4:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ?
 4:3))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeU]) & ((gctUINT32) ((((1 ?
 4:3) - (0 ? 4:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ?
 4:3)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeV]) & ((gctUINT32) ((((1 ?
 6:5) - (0 ? 6:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:7) - (0 ?
 8:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:7) - (0 ? 8:7) + 1))))))) << (0 ?
 8:7))) | (((gctUINT32) ((gctUINT32) (s_minXlate[createInfo->minFilter]) & ((gctUINT32) ((((1 ?
 8:7) - (0 ? 8:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:7) - (0 ? 8:7) + 1))))))) << (0 ?
 8:7)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:11) - (0 ?
 12:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) ((gctUINT32) (s_magXlate[createInfo->magFilter]) & ((gctUINT32) ((((1 ?
 12:11) - (0 ? 12:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:11) - (0 ?
 12:11) + 1))))))) << (0 ? 12:11)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:9) - (0 ?
 10:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) ((gctUINT32) (s_mipXlate[createInfo->mipmapMode]) & ((gctUINT32) ((((1 ?
 10:9) - (0 ? 10:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ?
 10:9)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 18:18) - (0 ?
 18:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 18:18) - (0 ?
 18:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:18) - (0 ? 18:18) + 1))))))) << (0 ?
 18:18)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19)));

    hwSamplerDesc->halti2.hwSamplerLOD  =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) ((lodbias == 0 ? 0 : 1)) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:1) - (0 ?
 10:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:1) - (0 ? 10:1) + 1))))))) << (0 ?
 10:1))) | (((gctUINT32) ((gctUINT32) (maxlod) & ((gctUINT32) ((((1 ? 10:1) - (0 ?
 10:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:1) - (0 ? 10:1) + 1))))))) << (0 ?
 10:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:11) - (0 ?
 20:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:11) - (0 ? 20:11) + 1))))))) << (0 ?
 20:11))) | (((gctUINT32) ((gctUINT32) (minlod) & ((gctUINT32) ((((1 ? 20:11) - (0 ?
 20:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:11) - (0 ? 20:11) + 1))))))) << (0 ?
 20:11)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:21) - (0 ?
 30:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:21) - (0 ? 30:21) + 1))))))) << (0 ?
 30:21))) | (((gctUINT32) ((gctUINT32) (lodbias) & ((gctUINT32) ((((1 ?
 30:21) - (0 ? 30:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:21) - (0 ?
 30:21) + 1))))))) << (0 ? 30:21)));

    hwSamplerDesc->halti2.hwBaseLOD_p0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) ((gctUINT32) (createInfo->compareEnable ? 1 : 0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ?
 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (s_funcXlate[createInfo->compareOp]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ?
 22:20) + 1))))))) << (0 ? 22:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:23) - (0 ?
 24:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ?
 24:23))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 24:23) - (0 ? 24:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ? 24:23)));

    hwSamplerDesc->halti2.hwSampler3D_p0 =
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) ((gctUINT32) (s_addressXlate[createInfo->addressModeW]) & ((gctUINT32) ((((1 ?
 29:28) - (0 ? 29:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:28) - (0 ?
 29:28) + 1))))))) << (0 ? 29:28)));

    return;
}

VkResult halti2_helper_convertHwTxDesc(
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
#define FAKED_TEXTURE_MAX_WIDTH 8192
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
        if (((texelSize / FAKED_TEXTURE_MAX_WIDTH) < 1))
        {
            fakedImageLevel.requestW = fakedImageLevel.allocedW = texelSize;
            fakedImageLevel.requestH = fakedImageLevel.allocedH = 1;
        }
        else
        {
            fakedImageLevel.requestW = fakedImageLevel.allocedW = FAKED_TEXTURE_MAX_WIDTH;
            fakedImageLevel.requestH = fakedImageLevel.allocedH = (uint32_t)gcoMATH_Ceiling(((float)texelSize / FAKED_TEXTURE_MAX_WIDTH));
        }
        fakedImageLevel.stride = (uint32_t)(FAKED_TEXTURE_MAX_WIDTH * (residentFormatInfo->bitsPerBlock >> 3));
        fakedImageLevel.requestD = 1;
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
    __VK_ASSERT(!devCtx->database->REG_TX6bitFrac);

    swizzle_r = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->r, hwTxFmtInfo->hwSwizzles[0], hwTxFmtInfo->hwSwizzles);
    swizzle_g = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->g, hwTxFmtInfo->hwSwizzles[1], hwTxFmtInfo->hwSwizzles);
    swizzle_b = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->b, hwTxFmtInfo->hwSwizzles[2], hwTxFmtInfo->hwSwizzles);
    swizzle_a = halti5_helper_convertHwTxSwizzle(residentFormatInfo, componentMapping->a, hwTxFmtInfo->hwSwizzles[3], hwTxFmtInfo->hwSwizzles);

    logWidth  = gcoMATH_Log2in5dot5(baseLevel->allocedW);
    logHeight = gcoMATH_Log2in5dot5(baseLevel->allocedH);
    logDepth  = gcoMATH_Log2in5dot5(baseLevel->requestD);

    addressing = (tiling == gcvLINEAR) ? 0x3 : 0x0;
    astcImage = ((((((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT))) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) ) == 0x14) ?
        VK_TRUE : VK_FALSE;

    switch (createFormat)
    {
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) ((gctUINT32) (0x2) & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) ((gctUINT32) (0x1) & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16)));
        break;
    default:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
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

        hwTxDesc[partIdx].halti2.hwSamplerMode_p1 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (s_xlateType[viewType].hwType) & ((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 17:13) - (0 ? 17:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:13) - (0 ?
 17:13) + 1))))))) << (0 ? 17:13))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_OLD_SHIFT)) & ((gctUINT32) ((((1 ?
 17:13) - (0 ? 17:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:13) - (0 ?
 17:13) + 1))))))) << (0 ? 17:13)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:20) - (0 ?
 21:20) + 1))))))) << (0 ? 21:20))) | (((gctUINT32) ((gctUINT32) (addressing) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:20) - (0 ?
 21:20) + 1))))))) << (0 ? 21:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:22) - (0 ? 23:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:22) - (0 ?
 23:22) + 1))))))) << (0 ? 23:22))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 23:22) - (0 ? 23:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:22) - (0 ?
 23:22) + 1))))))) << (0 ? 23:22)));

        hwTxDesc[partIdx].halti2.hwSamplerModeEx =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT)) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ?
 10:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (swizzle_r) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ?
 14:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (swizzle_g) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:12) - (0 ?
 14:12) + 1))))))) << (0 ? 14:12)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 18:16) - (0 ?
 18:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (swizzle_b) & ((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:16) - (0 ?
 18:16) + 1))))))) << (0 ? 18:16)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ?
 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (swizzle_a) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ?
 22:20) + 1))))))) << (0 ? 22:20)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:15) - (0 ?
 15:15) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:15) - (0 ?
 15:15) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:15) - (0 ? 15:15) + 1))))))) << (0 ?
 15:15)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (s_xlateType[viewType].hwIsArray) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ?
 24:24) + 1))))))) << (0 ? 24:24)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:26) - (0 ?
 28:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:26) - (0 ? 28:26) + 1))))))) << (0 ?
 28:26))) | (((gctUINT32) ((gctUINT32) (hAlignment) & ((gctUINT32) ((((1 ?
 28:26) - (0 ? 28:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:26) - (0 ?
 28:26) + 1))))))) << (0 ? 28:26)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:25) - (0 ?
 25:25) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ?
 25:25))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 25:25) - (0 ? 25:25) + 1))))))) << (0 ? 25:25)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ?
 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 29:29) - (0 ? 29:29) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ? 29:29)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ? 31:31)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 23:23) - (0 ? 23:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:23) - (0 ?
 23:23) + 1))))))) << (0 ? 23:23)));

        hwTxDesc[partIdx].halti2.hwBaseLOD_p1 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (subResourceRange->levelCount - 1) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)));

        hwTxDesc[partIdx].halti2.hwSampleWH =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ?
 14:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->allocedW) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:16) - (0 ?
 30:16) + 1))))))) << (0 ? 30:16))) | (((gctUINT32) ((gctUINT32) (baseLevel->allocedH) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:16) - (0 ?
 30:16) + 1))))))) << (0 ? 30:16)));

        hwTxDesc[partIdx].halti2.hwSampleLogWH_p1 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (logWidth) & ((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:10) - (0 ? 19:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:10) - (0 ?
 19:10) + 1))))))) << (0 ? 19:10))) | (((gctUINT32) ((gctUINT32) (logHeight) & ((gctUINT32) ((((1 ?
 19:10) - (0 ? 19:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:10) - (0 ?
 19:10) + 1))))))) << (0 ? 19:10)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ?
 30:30) - (0 ? 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ?
 30:30) + 1))))))) << (0 ? 30:30)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:27) - (0 ? 28:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:27) - (0 ?
 28:27) + 1))))))) << (0 ? 28:27))) | (((gctUINT32) ((gctUINT32) ((astcImage ?
 2 : 0)) & ((gctUINT32) ((((1 ? 28:27) - (0 ? 28:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ?
 28:27) - (0 ? 28:27) + 1))))))) << (0 ? 28:27)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31))) | (((gctUINT32) ((gctUINT32) (hwTxDesc[partIdx].sRGB) & ((gctUINT32) ((((1 ?
 31:31) - (0 ? 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ?
 31:31) + 1))))))) << (0 ? 31:31)));

        hwTxDesc[partIdx].halti2.hwSliceSize = (uint32_t)baseLevel->sliceSize;

        hwTxDesc[partIdx].halti2.hwTxConfig3 =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:3) - (0 ?
 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) ((gctUINT32) (msaaImage) & ((gctUINT32) ((((1 ?
 3:3) - (0 ? 3:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ?
 3:3)))
          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (hwTxDesc[partIdx].sampleStencil) & ((gctUINT32) ((((1 ?
 0:0) - (0 ? 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)));

        hwTxDesc[partIdx].halti2.hwSamplerLinearStride =
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:0) - (0 ?
 17:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->stride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0)));

        hwTxDesc[partIdx].halti2.hwTxConfig2 = txConfig2;

        if (viewType >= VK_IMAGE_VIEW_TYPE_1D_ARRAY)
        {
            hwTxDesc[partIdx].halti2.hwSampler3D_p1 =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ? 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (subResourceRange->layerCount) & ((gctUINT32) ((((1 ?
 13:0) - (0 ? 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (logDepth) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)));
        }
        else
        {
            hwTxDesc[partIdx].halti2.hwSampler3D_p1 =
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:0) - (0 ? 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (baseLevel->requestD) & ((gctUINT32) ((((1 ?
 13:0) - (0 ? 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0)))
                | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (logDepth) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)));
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
 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ? 4:4) - (0 ?
 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 11:8) - (0 ? 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:12) - (0 ?
 12:12) + 1))))))) << (0 ? 12:12))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 12:12) - (0 ? 12:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:12) - (0 ?
 12:12) + 1))))))) << (0 ? 12:12)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ?
 20:20) + 1))))))) << (0 ? 20:20))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 20:20) - (0 ? 20:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:20) - (0 ?
 20:20) + 1))))))) << (0 ? 20:20)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ?
 27:24) + 1))))))) << (0 ? 27:24))) | (((gctUINT32) ((gctUINT32) (astcSize) & ((gctUINT32) ((((1 ?
 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ?
 27:24) + 1))))))) << (0 ? 27:24)))
              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:28) - (0 ?
 28:28) + 1))))))) << (0 ? 28:28))) | (((gctUINT32) ((gctUINT32) (astcSrgb) & ((gctUINT32) ((((1 ?
 28:28) - (0 ? 28:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:28) - (0 ?
 28:28) + 1))))))) << (0 ? 28:28)));
        }
    }

OnError:
    return result;
}

void halti2_helper_setSamplerStates(
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
    uint32_t hwSamplerMode_p0 = samplerDesc->halti2.hwSamplerMode_p0;

    if (!txHwRegisterIdx)
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x022D, VK_FALSE,
            ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))));
    }
    else
    {
        __vkCmdLoadSingleHWState(commandBuffer, 0x022D, VK_FALSE,
            ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))));
    }


    __vkCmdLoadSingleHWState(commandBuffer, 0x4000 + hwSamplerNo, VK_FALSE,
        hwSamplerMode_p0 | txDesc->halti2.hwSamplerMode_p1);

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
      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ?
 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (txDesc->fast_filter && ((samplerDesc->halti2.anisoLog == 0) || txDesc->isCubmap)) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29))));

    __vkCmdLoadSingleHWState(commandBuffer, 0x40C0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwSampler3D_p1 | samplerDesc->halti2.hwSampler3D_p0);

    __vkCmdLoadSingleHWState(commandBuffer, 0x4060 + hwSamplerNo, VK_FALSE,
        samplerDesc->halti2.hwSamplerLOD);

    __vkCmdLoadSingleHWState(commandBuffer, 0x41C0 + hwSamplerNo, VK_FALSE,
        txDesc->halti2.hwBaseLOD_p1 | samplerDesc->halti2.hwBaseLOD_p0);

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

VkResult halti2_program_blit_src_tex(
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
    uint32_t txStride, txSliceSize, txHAlign;
    uint32_t txType, txSRGB, txAddressing, txIntCtrl, txConfig2;
    uint32_t txFormat = VK_FORMAT_UNDEFINED;
    const __vkFormatToHwTxFmtInfo *hwTxFmtInfo = gcvNULL;
    SHADER_SAMPLER_SLOT_MAPPING *hwMapping = gcvNULL;
    uint32_t hwSamplerNo;

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

        txFormat = pDstImg->createInfo.format;
        fmtInfo = &g_vkFormatInfoTable[txFormat];
        txStride = (params->srcSize.width / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8;
        txSliceSize = (params->srcSize.height / fmtInfo->blockSize.height) * txStride;

        switch (txFormat)
        {
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            txFormat = VK_FORMAT_R32_SFLOAT;
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
            txFormat = VK_FORMAT_R32G32_SFLOAT;
            params->srcSize.width = gcmALIGN(params->srcSize.width, fmtInfo->blockSize.width) / fmtInfo->blockSize.width;
            params->srcSize.height = gcmALIGN(params->srcSize.height, fmtInfo->blockSize.height) / fmtInfo->blockSize.height;
            if (fmtInfo->bitsPerBlock == 128)
            {
                params->srcSize.width *= 2;
            }
            break;
        default:
            break;
        }

        txType = (pDstImg->createInfo.imageType == VK_IMAGE_TYPE_3D) ? 0x3 : 0x2;
        txAddressing = 0x3;
        txHAlign = 0x1;

        address = pSrcBuf->memory->devAddr;
        address += (uint32_t)(pSrcBuf->memOffset + srcRes->u.buf.offset);
    }

    switch (txFormat)
    {
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16B16A16_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8B8A8_SINT:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)))
                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16)));
        break;
    default:
        txConfig2 = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 19:18) - (0 ? 19:18) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:18) - (0 ?
 19:18) + 1))))))) << (0 ? 19:18)));
        break;
    }

    hwTxFmtInfo = halti5_helper_convertHwTxInfo(devCtx, txFormat);
    txSRGB = (hwTxFmtInfo->hwFormat >> TX_FORMAT_SRGB_SHIFT) & 0x1;
    if (txFormat == VK_FORMAT_R8G8B8A8_SRGB)
    {
        txSRGB = VK_FALSE;
    }

    txIntCtrl = ((hwTxFmtInfo->hwFormat >> TX_FORMAT_FAST_FILTER_SHIFT) & 0x1)
             && (txType != 0x3)
             && !txSRGB;

    hwMapping = &blitProg->srcTexEntry->hwMappings[VSC_SHADER_STAGE_CS].samplerMapping;
    hwSamplerNo = hwMapping->hwSamplerSlot + pHints->samplerBaseOffset[VSC_SHADER_STAGE_CS];

    __vkCmdLoadSingleHWState(states, 0x022D + hwSamplerNo, VK_FALSE,
        ((((gctUINT32) (pHints->shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))));

    __vkCmdLoadSingleHWState(states, 0x4000 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (txType) & ((gctUINT32) ((((1 ? 2:0) - (0 ?
 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:13) - (0 ?
 17:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:13) - (0 ? 17:13) + 1))))))) << (0 ?
 17:13))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_OLD_SHIFT)) & ((gctUINT32) ((((1 ?
 17:13) - (0 ? 17:13) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:13) - (0 ?
 17:13) + 1))))))) << (0 ? 17:13)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:20) - (0 ?
 21:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:20) - (0 ? 21:20) + 1))))))) << (0 ?
 21:20))) | (((gctUINT32) ((gctUINT32) (txAddressing) & ((gctUINT32) ((((1 ?
 21:20) - (0 ? 21:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:20) - (0 ?
 21:20) + 1))))))) << (0 ? 21:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:22) - (0 ?
 23:22) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ?
 23:22))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 23:22) - (0 ? 23:22) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:22) - (0 ? 23:22) + 1))))))) << (0 ? 23:22)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 4:3) - (0 ?
 4:3) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ?
 4:3))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 4:3) - (0 ? 4:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:3) - (0 ? 4:3) + 1))))))) << (0 ? 4:3)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:5) - (0 ?
 6:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ?
 6:5))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 6:5) - (0 ? 6:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 6:5) - (0 ? 6:5) + 1))))))) << (0 ? 6:5)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:7) - (0 ?
 8:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:7) - (0 ? 8:7) + 1))))))) << (0 ?
 8:7))) | (((gctUINT32) ((gctUINT32) (minXlate[filter]) & ((gctUINT32) ((((1 ?
 8:7) - (0 ? 8:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:7) - (0 ? 8:7) + 1))))))) << (0 ?
 8:7)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:11) - (0 ?
 12:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:11) - (0 ? 12:11) + 1))))))) << (0 ?
 12:11))) | (((gctUINT32) ((gctUINT32) (magXlate[filter]) & ((gctUINT32) ((((1 ?
 12:11) - (0 ? 12:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:11) - (0 ?
 12:11) + 1))))))) << (0 ? 12:11)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:9) - (0 ?
 10:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ?
 10:9))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 10:9) - (0 ? 10:9) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 10:9) - (0 ? 10:9) + 1))))))) << (0 ? 10:9)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:19) - (0 ?
 19:19) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ?
 19:19))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 19:19) - (0 ? 19:19) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 19:19) - (0 ? 19:19) + 1))))))) << (0 ? 19:19)))
        );

    __vkCmdLoadSingleHWState(states, 0x40E0 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_NEW_SHIFT)) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 7:7) - (0 ?
 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7))) | (((gctUINT32) ((gctUINT32) ((hwTxFmtInfo->hwFormat >> TX_FORMAT_COLOR_SWIZZLE_SHIFT)) & ((gctUINT32) ((((1 ?
 7:7) - (0 ? 7:7) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:7) - (0 ? 7:7) + 1))))))) << (0 ?
 7:7)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:8) - (0 ?
 10:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[0]) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:12) - (0 ?
 14:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:12) - (0 ? 14:12) + 1))))))) << (0 ?
 14:12))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[1]) & ((gctUINT32) ((((1 ?
 14:12) - (0 ? 14:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:12) - (0 ?
 14:12) + 1))))))) << (0 ? 14:12)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 18:16) - (0 ?
 18:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[2]) & ((gctUINT32) ((((1 ?
 18:16) - (0 ? 18:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 18:16) - (0 ?
 18:16) + 1))))))) << (0 ? 18:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 22:20) - (0 ?
 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (hwTxFmtInfo->hwSwizzles[3]) & ((gctUINT32) ((((1 ?
 22:20) - (0 ? 22:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 22:20) - (0 ?
 22:20) + 1))))))) << (0 ? 22:20)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 28:26) - (0 ?
 28:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:26) - (0 ? 28:26) + 1))))))) << (0 ?
 28:26))) | (((gctUINT32) ((gctUINT32) (txHAlign) & ((gctUINT32) ((((1 ?
 28:26) - (0 ? 28:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 28:26) - (0 ?
 28:26) + 1))))))) << (0 ? 28:26)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:30) - (0 ?
 30:30) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ?
 30:30))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:30) - (0 ? 30:30) + 1))))))) << (0 ? 30:30)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ? 24:24)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 23:23) - (0 ?
 23:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ?
 23:23))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 23:23) - (0 ? 23:23) + 1))))))) << (0 ? 23:23)))
        );

    __vkCmdLoadSingleHWState(states, 0x41E0 + hwSamplerNo, VK_FALSE, txConfig2);

    __vkCmdLoadSingleHWState(states, 0x44A0 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 0:0) - (0 ? 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        );

    __vkCmdLoadSingleHWState(states, 0x4480 + hwSamplerNo, VK_FALSE, txSliceSize);

    __vkCmdLoadSingleHWState(states, 0x4020 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 14:0) - (0 ?
 14:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0))) | (((gctUINT32) ((gctUINT32) (params->srcSize.width) & ((gctUINT32) ((((1 ?
 14:0) - (0 ? 14:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 14:0) - (0 ? 14:0) + 1))))))) << (0 ?
 14:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:16) - (0 ?
 30:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:16) - (0 ? 30:16) + 1))))))) << (0 ?
 30:16))) | (((gctUINT32) ((gctUINT32) (params->srcSize.height) & ((gctUINT32) ((((1 ?
 30:16) - (0 ? 30:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:16) - (0 ?
 30:16) + 1))))))) << (0 ? 30:16)))
        );

    __vkCmdLoadSingleHWState(states, 0x4040 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (gcoMATH_Log2in5dot5(params->srcSize.width)) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 19:10) - (0 ?
 19:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:10) - (0 ? 19:10) + 1))))))) << (0 ?
 19:10))) | (((gctUINT32) ((gctUINT32) (gcoMATH_Log2in5dot5(params->srcSize.height)) & ((gctUINT32) ((((1 ?
 19:10) - (0 ? 19:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:10) - (0 ?
 19:10) + 1))))))) << (0 ? 19:10)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31))) | (((gctUINT32) ((gctUINT32) (txSRGB) & ((gctUINT32) ((((1 ? 31:31) - (0 ?
 31:31) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:31) - (0 ? 31:31) + 1))))))) << (0 ?
 31:31)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:29) - (0 ?
 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ? 29:29) + 1))))))) << (0 ?
 29:29))) | (((gctUINT32) ((gctUINT32) (txIntCtrl) & ((gctUINT32) ((((1 ?
 29:29) - (0 ? 29:29) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:29) - (0 ?
 29:29) + 1))))))) << (0 ? 29:29)))
        );

    __vkCmdLoadSingleHWState(states, 0x40C0 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:28) - (0 ?
 29:28) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ?
 29:28))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ? 29:28) - (0 ? 29:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 29:28) - (0 ? 29:28) + 1))))))) << (0 ? 29:28)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:0) - (0 ?
 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0))) | (((gctUINT32) ((gctUINT32) (params->srcSize.depth) & ((gctUINT32) ((((1 ?
 13:0) - (0 ? 13:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:0) - (0 ? 13:0) + 1))))))) << (0 ?
 13:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 25:16) - (0 ?
 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ? 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) (gcoMATH_Log2in5dot5(params->srcSize.depth)) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))
        );

    __vkCmdLoadSingleHWState(states, 0x4060 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 0:0) - (0 ?
 0:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ?
 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:1) - (0 ?
 10:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:1) - (0 ? 10:1) + 1))))))) << (0 ?
 10:1))) | (((gctUINT32) ((gctUINT32) (0x3FF) & ((gctUINT32) ((((1 ? 10:1) - (0 ?
 10:1) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:1) - (0 ? 10:1) + 1))))))) << (0 ?
 10:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 20:11) - (0 ?
 20:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:11) - (0 ? 20:11) + 1))))))) << (0 ?
 20:11))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 20:11) - (0 ?
 20:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:11) - (0 ? 20:11) + 1))))))) << (0 ?
 20:11)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 30:21) - (0 ?
 30:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:21) - (0 ? 30:21) + 1))))))) << (0 ?
 30:21))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 30:21) - (0 ?
 30:21) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 30:21) - (0 ? 30:21) + 1))))))) << (0 ?
 30:21)))
        );

    __vkCmdLoadSingleHWState(states, 0x41C0 + hwSamplerNo, VK_FALSE,
          ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 3:0) - (0 ?
 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1))))))) << (0 ?
 3:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 11:8) - (0 ?
 11:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:8) - (0 ? 11:8) + 1))))))) << (0 ?
 11:8)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 16:16) - (0 ?
 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ?
 16:16))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:23) - (0 ?
 24:23) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ?
 24:23))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 24:23) - (0 ? 24:23) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 24:23) - (0 ? 24:23) + 1))))))) << (0 ? 24:23)))
        );

    __vkCmdLoadSingleHWState(states, 0x4200 + (hwSamplerNo << 4), VK_FALSE, address);

    __vkCmdLoadSingleHWState(states, 0x40A0 + hwSamplerNo, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 17:0) - (0 ?
 17:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0))) | (((gctUINT32) ((gctUINT32) (txStride) & ((gctUINT32) ((((1 ?
 17:0) - (0 ? 17:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 17:0) - (0 ? 17:0) + 1))))))) << (0 ?
 17:0)))
        );

    return result;
}




