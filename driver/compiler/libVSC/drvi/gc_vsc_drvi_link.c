/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/* This file includes all jobs that needs linkage, such as GL program linkage among
   shader stages, file modules linkage for kernel program, HW linkage for programming,
   etc */

#include "gc_vsc.h"
#include "chip/gpu/gc_vsc_chip_state_programming.h"
#include "vir/transform/gc_vsc_vir_uniform.h"
#include "vir/transform/gc_vsc_vir_vectorization.h"
#include "vir/codegen/gc_vsc_vir_ep_gen.h"

#define SUPPORT_ATTR_ALIAS               0

gceSTATUS vscInitializeShaderHWInfo(SHADER_HW_INFO* pShaderHwInfo, SHADER_EXECUTABLE_PROFILE* pSEP)
{
    gctUINT ioIdx;

    pShaderHwInfo->pSEP = pSEP;

    /* Input link */
    pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.totalLinkNoCount = 0;
    pShaderHwInfo->inputLinkageInfo.linkedShaderStage = SHADER_TYPE_UNKNOWN;
    for (ioIdx = 0; ioIdx < MAX_SHADER_IO_NUM; ioIdx ++)
    {
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo = NOT_ASSIGNED;
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO = gcvFALSE;
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bIsDummyLink = gcvFALSE;

        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageX =
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageY =
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageZ =
        pShaderHwInfo->inputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageW = gcvFALSE;
    }

    pShaderHwInfo->inputLinkageInfo.primLinkage.totalLinkNoCount = 0;
    pShaderHwInfo->inputLinkageInfo.linkedShaderStage = SHADER_TYPE_UNKNOWN;
    for (ioIdx = 0; ioIdx < MAX_SHADER_IO_NUM; ioIdx ++)
    {
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].linkNo = NOT_ASSIGNED;
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO = gcvFALSE;
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bIsDummyLink = gcvFALSE;

        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageX =
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageY =
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageZ =
        pShaderHwInfo->inputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageW = gcvFALSE;
    }

    /* Output link */
    pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.totalLinkNoCount = 0;
    pShaderHwInfo->outputLinkageInfo.linkedShaderStage = SHADER_TYPE_UNKNOWN;
    for (ioIdx = 0; ioIdx < MAX_SHADER_IO_NUM; ioIdx ++)
    {
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].linkNo = NOT_ASSIGNED;
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO = gcvFALSE;
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bIsDummyLink = gcvFALSE;

        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageX =
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageY =
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageZ =
        pShaderHwInfo->outputLinkageInfo.vtxPxlLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageW = gcvFALSE;
    }

    pShaderHwInfo->outputLinkageInfo.primLinkage.totalLinkNoCount = 0;
    pShaderHwInfo->outputLinkageInfo.linkedShaderStage = SHADER_TYPE_UNKNOWN;
    for (ioIdx = 0; ioIdx < MAX_SHADER_IO_NUM; ioIdx ++)
    {
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].linkNo = NOT_ASSIGNED;
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bOnlyLinkToSO = gcvFALSE;
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bIsDummyLink = gcvFALSE;

        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageX =
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageY =
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageZ =
        pShaderHwInfo->outputLinkageInfo.primLinkage.ioRegLinkage[ioIdx].bLinkedByOtherStageW = gcvFALSE;
    }

    pShaderHwInfo->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNUNIFIED_BUFFER;
    pShaderHwInfo->hwProgrammingHints.hwConstantFetchMode = HW_CONSTANT_FETCH_MODE_UNUNIFIED_REG_FILE;
    pShaderHwInfo->hwProgrammingHints.hwConstantRegAddrOffset = 0;
    pShaderHwInfo->hwProgrammingHints.hwSamplerFetchMode = HW_SAMPLER_FETCH_MODE_UNUNIFIED_REG_FILE;
    pShaderHwInfo->hwProgrammingHints.hwSamplerRegAddrOffset = 0;
    pShaderHwInfo->hwProgrammingHints.hwInstBufferAddrOffset = 0;
    pShaderHwInfo->hwProgrammingHints.maxUscSizeInKbyte = 0;
    pShaderHwInfo->hwProgrammingHints.minUscSizeInKbyte = 0;
    pShaderHwInfo->hwProgrammingHints.resultCacheWindowSize = 0;
    pShaderHwInfo->hwProgrammingHints.maxThreadsPerHwTG = 0;
    pShaderHwInfo->hwProgrammingHints.gsMetaDataSizePerHwTGInBtye = 0;
    pShaderHwInfo->hwProgrammingHints.maxParallelFactor = 0;

    return gcvSTATUS_OK;
}

gceSTATUS vscFinalizeShaderHWInfo(SHADER_HW_INFO* pShaderHwInfo)
{
    return vscInitializeShaderHWInfo(pShaderHwInfo, gcvNULL);
}

gceSTATUS vscInitializePEP(PROGRAM_EXECUTABLE_PROFILE* pPEP)
{
    gceSTATUS                  status = gcvSTATUS_OK;
    gctUINT                    stageIdx;

    gcoOS_ZeroMemory(pPEP, sizeof(PROGRAM_EXECUTABLE_PROFILE));

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        gcmONERROR(vscInitializeSEP(&pPEP->seps[stageIdx]));
    }

OnError:
    return status;
}

gceSTATUS vscFinalizePEP(PROGRAM_EXECUTABLE_PROFILE* pPEP)
{
    gceSTATUS                                    status = gcvSTATUS_OK;
    gctUINT                                      stageIdx, i, j;
    SHADER_CONSTANT_HW_LOCATION_MAPPING*         pCnstHwLocMapping;
    SHADER_UAV_SLOT_MAPPING*                     pUavSlotMapping;
    PROG_VK_COMBINED_TEXTURE_SAMPLER_HW_MAPPING* pCtsHwMapping;
    PROG_VK_SEPARATED_SAMPLER_HW_MAPPING*        pSeparatedSampMapping;
    PROG_VK_SEPARATED_TEXTURE_HW_MAPPING*        pSeparatedTextureMapping;
    PROG_VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING*     pUtbHwMapping;

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        gcmONERROR(vscFinalizeSEP(&pPEP->seps[stageIdx]));
    }

    if (pPEP->attribTable.countOfEntries)
    {
        gcmASSERT(pPEP->attribTable.pAttribEntries);

        for (i = 0; i < pPEP->attribTable.countOfEntries; i ++)
        {
            if (pPEP->attribTable.pAttribEntries[i].name)
            {
                gcoOS_Free(gcvNULL, (gctPOINTER)pPEP->attribTable.pAttribEntries[i].name);
                pPEP->attribTable.pAttribEntries[i].name = gcvNULL;
            }

            if (pPEP->attribTable.pAttribEntries[i].pLocation)
            {
                gcoOS_Free(gcvNULL, pPEP->attribTable.pAttribEntries[i].pLocation);
                pPEP->attribTable.pAttribEntries[i].pLocation = gcvNULL;
            }
        }

        gcoOS_Free(gcvNULL, pPEP->attribTable.pAttribEntries);
        pPEP->attribTable.pAttribEntries = gcvNULL;
        pPEP->attribTable.countOfEntries = 0;
    }

    if (pPEP->fragOutTable.countOfEntries)
    {
        gcmASSERT(pPEP->fragOutTable.pFragOutEntries);

        for (i = 0; i < pPEP->fragOutTable.countOfEntries; i ++)
        {
            if (pPEP->fragOutTable.pFragOutEntries[i].name)
            {
                gcoOS_Free(gcvNULL, (gctPOINTER)pPEP->fragOutTable.pFragOutEntries[i].name);
                pPEP->fragOutTable.pFragOutEntries[i].name = gcvNULL;
            }

            if (pPEP->fragOutTable.pFragOutEntries[i].pLocation)
            {
                gcoOS_Free(gcvNULL, pPEP->fragOutTable.pFragOutEntries[i].pLocation);
                pPEP->fragOutTable.pFragOutEntries[i].pLocation = gcvNULL;
            }
        }

        gcoOS_Free(gcvNULL, pPEP->fragOutTable.pFragOutEntries);
        pPEP->fragOutTable.pFragOutEntries = gcvNULL;
        pPEP->fragOutTable.countOfEntries = 0;
    }

    if (pPEP->pepClient == PEP_CLIENT_GL)
    {
    }
    else if (pPEP->pepClient == PEP_CLIENT_VK)
    {
        if (pPEP->u.vk.pResourceSets)
        {
            for (i = 0; i < pPEP->u.vk.resourceSetCount; i ++)
            {
                if (pPEP->u.vk.pResourceSets[i].combinedSampTexTable.countOfEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].combinedSampTexTable.countOfEntries; j ++)
                    {
                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            if (pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries[j].stageBits &
                                VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx))
                            {
                                pCtsHwMapping = &pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries[j].hwMappings[stageIdx];

                                if (pCtsHwMapping->ppExtraSamplerArray)
                                {
                                    gcoOS_Free(gcvNULL, pCtsHwMapping->ppExtraSamplerArray);
                                    pCtsHwMapping->ppExtraSamplerArray = gcvNULL;
                                }

                                if (pCtsHwMapping->samplerHwMappingList.arraySize)
                                {
                                    gcoOS_Free(gcvNULL, pCtsHwMapping->samplerHwMappingList.pPctsHmEntryIdxArray);
                                    pCtsHwMapping->samplerHwMappingList.pPctsHmEntryIdxArray = gcvNULL;
                                }

                                if (pCtsHwMapping->texHwMappingList.arraySize)
                                {
                                    gcoOS_Free(gcvNULL, pCtsHwMapping->texHwMappingList.pPctsHmEntryIdxArray);
                                    pCtsHwMapping->texHwMappingList.pPctsHmEntryIdxArray = gcvNULL;
                                }
                            }

                            if (pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries[j].pResOpBits)
                            {
                                gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries[j].pResOpBits);
                                pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries[j].pResOpBits = gcvNULL;
                            }
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries);
                    pPEP->u.vk.pResourceSets[i].combinedSampTexTable.pCombTsEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].combinedSampTexTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].separatedSamplerTable.countOfEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].separatedSamplerTable.countOfEntries; j ++)
                    {
                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            if (pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].bUsingHwMppingList &&
                                (pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].stageBits &
                                 VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx)))
                            {
                                pSeparatedSampMapping = &pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].hwMappings[stageIdx];

                                if (pSeparatedSampMapping->samplerHwMappingList.arraySize)
                                {
                                    gcoOS_Free(gcvNULL, pSeparatedSampMapping->samplerHwMappingList.pPctsHmEntryIdxArray);
                                    pSeparatedSampMapping->samplerHwMappingList.pPctsHmEntryIdxArray = gcvNULL;
                                }
                            }
                        }

                        if (pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].pResOpBits)
                        {
                            gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].pResOpBits);
                            pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries[j].pResOpBits = gcvNULL;
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries);
                    pPEP->u.vk.pResourceSets[i].separatedSamplerTable.pSamplerEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].separatedSamplerTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].separatedTexTable.countOfEntries; j ++)
                    {
                        PROG_VK_SEPARATED_TEXTURE_TABLE_ENTRY* pTexEntry = &pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries[j];

                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            if (pTexEntry->stageBits & VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx))
                            {
                                if (pTexEntry->bUsingHwMppingList)
                                {
                                    pSeparatedTextureMapping = &pTexEntry->hwMappings[stageIdx];

                                    if (pSeparatedTextureMapping->s.texHwMappingList.arraySize)
                                    {
                                        gcmOS_SAFE_FREE(gcvNULL, pSeparatedTextureMapping->s.texHwMappingList.pPctsHmEntryIdxArray);
                                    }
                                }

                                if (pTexEntry->hwMappings[stageIdx].s.hwMapping.hwLoc.pHwDirectAddrBase)
                                {
                                    gcmOS_SAFE_FREE(gcvNULL, pTexEntry->hwMappings[stageIdx].s.hwMapping.hwLoc.pHwDirectAddrBase);
                                }
                            }
                        }

                        if (pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries[j].pResOpBits)
                        {
                            gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries[j].pResOpBits);
                            pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries[j].pResOpBits = gcvNULL;
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries);
                    pPEP->u.vk.pResourceSets[i].separatedTexTable.pTextureEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].separatedTexTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].uniformTexBufTable.countOfEntries; j ++)
                    {
                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            pUtbHwMapping = &pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries[j].hwMappings[stageIdx];
                            if (pUtbHwMapping->hwMappingMode == VK_UNIFORM_TEXEL_BUFFER_HW_MAPPING_MODE_NOT_NATIVELY_SUPPORT)
                            {
                                if (pUtbHwMapping->u.s.ppExtraSamplerArray)
                                {
                                    gcoOS_Free(gcvNULL, pUtbHwMapping->u.s.ppExtraSamplerArray);
                                    pUtbHwMapping->u.s.ppExtraSamplerArray = gcvNULL;
                                }
                            }
                        }

                        if (pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries[j].pResOpBits)
                        {
                            gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries[j].pResOpBits);
                            pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries[j].pResOpBits = gcvNULL;
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries);
                    pPEP->u.vk.pResourceSets[i].uniformTexBufTable.pUtbEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].uniformTexBufTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].inputAttachmentTable.pIaEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].inputAttachmentTable.countOfEntries; j++)
                    {
                        PROG_VK_INPUT_ATTACHMENT_TABLE_ENTRY iaEntry = pPEP->u.vk.pResourceSets[i].inputAttachmentTable.pIaEntries[j];

                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx++)
                        {
                            if (iaEntry.hwMappings[stageIdx].uavMapping.hwLoc.pHwDirectAddrBase)
                            {
                                gcoOS_Free(gcvNULL, iaEntry.hwMappings[stageIdx].uavMapping.hwLoc.pHwDirectAddrBase);
                            }
                        }

                        if (iaEntry.pResOpBits)
                        {
                            gcoOS_Free(gcvNULL, iaEntry.pResOpBits);
                            iaEntry.pResOpBits = gcvNULL;
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].inputAttachmentTable.pIaEntries);
                    pPEP->u.vk.pResourceSets[i].inputAttachmentTable.pIaEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].inputAttachmentTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].storageTable.pStorageEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].storageTable.countOfEntries; j ++)
                    {
                        PROG_VK_STORAGE_TABLE_ENTRY storageEntry = pPEP->u.vk.pResourceSets[i].storageTable.pStorageEntries[j];

                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            if (storageEntry.stageBits & VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx))
                            {
                                pUavSlotMapping = &storageEntry.hwMappings[stageIdx];

                                if (pUavSlotMapping->hwMemAccessMode != SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER)
                                {
                                    gcmASSERT(pUavSlotMapping->hwLoc.pHwDirectAddrBase);

                                    gcoOS_Free(gcvNULL, pUavSlotMapping->hwLoc.pHwDirectAddrBase);
                                    pUavSlotMapping->hwLoc.pHwDirectAddrBase = gcvNULL;
                                }
                            }
                        }

                        if (storageEntry.pResOpBits)
                        {
                            gcoOS_Free(gcvNULL, storageEntry.pResOpBits);
                            storageEntry.pResOpBits = gcvNULL;
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].storageTable.pStorageEntries);
                    pPEP->u.vk.pResourceSets[i].storageTable.pStorageEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].storageTable.countOfEntries = 0;
                }

                if (pPEP->u.vk.pResourceSets[i].uniformBufferTable.pUniformBufferEntries)
                {
                    for (j = 0; j < pPEP->u.vk.pResourceSets[i].uniformBufferTable.countOfEntries; j ++)
                    {
                        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                        {
                            if (pPEP->u.vk.pResourceSets[i].uniformBufferTable.pUniformBufferEntries[j].stageBits &
                                VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx))
                            {
                                pCnstHwLocMapping = &pPEP->u.vk.pResourceSets[i].uniformBufferTable.pUniformBufferEntries[j].
                                                                                    hwMappings[stageIdx];

                                if (pCnstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_MEMORY &&
                                    pCnstHwLocMapping->hwLoc.memAddr.hwMemAccessMode != SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER)
                                {
                                    gcmASSERT(pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase);

                                    gcoOS_Free(gcvNULL, pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase);
                                    pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase = gcvNULL;
                                }
                            }
                        }
                    }

                    gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets[i].uniformBufferTable.pUniformBufferEntries);
                    pPEP->u.vk.pResourceSets[i].uniformBufferTable.pUniformBufferEntries = gcvNULL;
                    pPEP->u.vk.pResourceSets[i].uniformBufferTable.countOfEntries = 0;
                }
            }

            gcoOS_Free(gcvNULL, pPEP->u.vk.pResourceSets);
            pPEP->u.vk.pResourceSets = gcvNULL;
            pPEP->u.vk.resourceSetCount = 0;
        }

        if (pPEP->u.vk.pushConstantTable.pPushConstantEntries)
        {
            for (j = 0; j < pPEP->u.vk.pushConstantTable.countOfEntries; j ++)
            {
                for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
                {
                    if (pPEP->u.vk.pushConstantTable.pPushConstantEntries[j].stageBits &
                        VSC_SHADER_STAGE_2_STAGE_BIT(stageIdx))
                    {
                        pCnstHwLocMapping = &pPEP->u.vk.pushConstantTable.pPushConstantEntries[j].hwMappings[stageIdx];

                        if (pCnstHwLocMapping->hwAccessMode == SHADER_HW_ACCESS_MODE_MEMORY &&
                            pCnstHwLocMapping->hwLoc.memAddr.hwMemAccessMode != SHADER_HW_MEM_ACCESS_MODE_PLACE_HOLDER)
                        {
                            gcmASSERT(pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase);

                            gcoOS_Free(gcvNULL, pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase);
                            pCnstHwLocMapping->hwLoc.memAddr.memBase.pHwDirectAddrBase = gcvNULL;
                        }
                    }
                }
            }

            gcoOS_Free(gcvNULL, pPEP->u.vk.pushConstantTable.pPushConstantEntries);
            pPEP->u.vk.pushConstantTable.pPushConstantEntries = gcvNULL;
            pPEP->u.vk.pushConstantTable.countOfEntries = 0;
        }

        if (pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray)
        {
            for (i = 0; i < pPEP->u.vk.privateCombTsHwMappingPool.countOfArray; i ++)
            {
                if (pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[i].ppExtraSamplerArray)
                {
                    gcoOS_Free(gcvNULL, pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[i].ppExtraSamplerArray);
                    pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray[i].ppExtraSamplerArray = gcvNULL;
                }
            }

            gcoOS_Free(gcvNULL, pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray);
            pPEP->u.vk.privateCombTsHwMappingPool.pPrivCombTsHwMappingArray = gcvNULL;
            pPEP->u.vk.privateCombTsHwMappingPool.countOfArray = 0;
        }
    }

OnError:
    return status;
}

gceSTATUS vscInitializeKEP(KERNEL_EXECUTABLE_PROFILE* pKEP)
{
    gceSTATUS                  status = gcvSTATUS_OK;

    gcoOS_ZeroMemory(pKEP, sizeof(KERNEL_EXECUTABLE_PROFILE));

    gcmONERROR(vscInitializeSEP(&pKEP->sep));

OnError:
    return status;
}

gceSTATUS vscFinalizeKEP(KERNEL_EXECUTABLE_PROFILE* pKEP)
{
    gceSTATUS                  status = gcvSTATUS_OK;
    gctUINT                    i;

    gcmONERROR(vscFinalizeSEP(&pKEP->sep));

    if (pKEP->argTable.countOfEntries > 0)
    {
        for (i = 0; i < pKEP->argTable.countOfEntries; i++)
        {
            PROG_CL_ARG_ENTRY*  pClArgEntry = &pKEP->argTable.pArgsEntries[i];

            if (pClArgEntry && pClArgEntry->argName)
            {
                gcmOS_SAFE_FREE(gcvNULL, pClArgEntry->argName);
            }
            if (pClArgEntry && pClArgEntry->argTypeName)
            {
                gcmOS_SAFE_FREE(gcvNULL, pClArgEntry->argTypeName);
            }
        }
        gcoOS_Free(gcvNULL, pKEP->argTable.pArgsEntries);
        pKEP->argTable.pArgsEntries = gcvNULL;
        pKEP->argTable.countOfEntries = 0;
    }

    if (pKEP->resourceTable.combinedSampTexTable.countOfEntries > 0)
    {
        gcoOS_Free(gcvNULL,pKEP->resourceTable.combinedSampTexTable.pCombTsEntries);
        pKEP->resourceTable.combinedSampTexTable.pCombTsEntries = gcvNULL;
        pKEP->resourceTable.combinedSampTexTable.countOfEntries = 0;
    }

    if (pKEP->resourceTable.imageTable.countOfEntries > 0)
    {
        gcoOS_Free(gcvNULL,pKEP->resourceTable.imageTable.pImageEntries);
        pKEP->resourceTable.imageTable.pImageEntries = gcvNULL;
        pKEP->resourceTable.imageTable.countOfEntries = 0;
    }

    if (pKEP->resourceTable.uniformTable.countOfEntries > 0)
    {
        gcoOS_Free(gcvNULL,pKEP->resourceTable.uniformTable.pUniformEntries);
        pKEP->resourceTable.uniformTable.pUniformEntries = gcvNULL;
        pKEP->resourceTable.uniformTable.countOfEntries = 0;
    }

    if (pKEP->resourceTable.prvUniformTable.countOfEntries > 0)
    {
        gcoOS_Free(gcvNULL, pKEP->resourceTable.prvUniformTable.pUniformEntries);
        pKEP->resourceTable.prvUniformTable.countOfEntries = 0;
        pKEP->resourceTable.prvUniformTable.pUniformEntries = gcvNULL;
    }

    if(pKEP->kernelHints.constantMemBuffer)
    {
        gcmOS_SAFE_FREE(gcvNULL, pKEP->kernelHints.constantMemBuffer);
    }


OnError:
    return status;
}

typedef struct _VSC_BASE_LINKER_HELPER
{
    VSC_MM*                           pMM;
    PVSC_HW_CONFIG                    pHwCfg;
}VSC_BASE_LINKER_HELPER;

typedef struct _VSC_PROGRAM_LINKER_HELPER
{
    VSC_BASE_LINKER_HELPER            baseHelper;
    VSC_GPG_PASS_MANAGER              pgPassMnger;
}VSC_PROGRAM_LINKER_HELPER;

typedef struct _VSC_KERNEL_PROGRAM_LINKER_HELPER
{
    VSC_BASE_LINKER_HELPER            baseHelper;
    VSC_KPG_PASS_MANAGER              pgPassMnger;
}VSC_KERNEL_PROGRAM_LINKER_HELPER;

typedef enum FSL_STAGE
{
    /* In this first shader linkage stage, we dont calc ll slot for IO,
       just only do complete spec level check */
    FSL_STAGE_API_SPEC_CHECK,

    /* This is the intermedium stage, we dont calc ll slot, only do partial
       spec level check (actually, it is not necessary). This stage is for
       iterately deleting more outputs by checking IOs among shaders */
    FSL_STAGE_INTERMEDIUM,

    /* In this first shader linkage stage, we only do logic IO packing, i.e,
       io ll-slot level packing (io-vectorization) */
    FSL_STAGE_IO_VECTORIZATION,

    /* In this first shader linkage stage, we do both ll slot calc for IO
       and partial spec level check (actually, it is not necessary) */
    FSL_STAGE_LL_SLOT_CALC
}FSL_STAGE;

void vscSortIOsByHwLoc(SHADER_IO_MAPPING_PER_EXE_OBJ* pIoMappingPerExeObj, gctUINT* pSortedIoIdxArray)
{
    gctUINT                i, j, tempIdx;
    gctUINT                minHwRegNo, thisHwRegNo;
    gctUINT                firstValidChannel;

    /* Initialize default order */
    for (i = 0; i < pIoMappingPerExeObj->countOfIoRegMapping; i ++)
    {
        pSortedIoIdxArray[i] = i;
    }

    /* Use simple bubble sort */
    for (i = 0; i < pIoMappingPerExeObj->countOfIoRegMapping; i ++)
    {
        if (pIoMappingPerExeObj->ioIndexMask & (1LL << pSortedIoIdxArray[i]))
        {
            firstValidChannel = pIoMappingPerExeObj->pIoRegMapping[pSortedIoIdxArray[i]].firstValidIoChannel;
            minHwRegNo = pIoMappingPerExeObj->pIoRegMapping[pSortedIoIdxArray[i]].ioChannelMapping[firstValidChannel].
                         hwLoc.cmnHwLoc.u.hwRegNo;

            for (j = i + 1; j < pIoMappingPerExeObj->countOfIoRegMapping; j ++)
            {
                if (pIoMappingPerExeObj->ioIndexMask & (1LL << pSortedIoIdxArray[j]))
                {
                    firstValidChannel = pIoMappingPerExeObj->pIoRegMapping[pSortedIoIdxArray[j]].firstValidIoChannel;
                    thisHwRegNo = pIoMappingPerExeObj->pIoRegMapping[pSortedIoIdxArray[j]].ioChannelMapping[firstValidChannel].
                                  hwLoc.cmnHwLoc.u.hwRegNo;
                    if (thisHwRegNo < minHwRegNo)
                    {
                        minHwRegNo = thisHwRegNo;

                        tempIdx = pSortedIoIdxArray[i];
                        pSortedIoIdxArray[i] = pSortedIoIdxArray[j];
                        pSortedIoIdxArray[j] = tempIdx;
                    }
                }
            }
        }
    }
}

extern void _ConvertVirPerVtxPxlAndPerPrimIoList(VIR_Shader* pShader, VSC_MM* pMM, gctBOOL bInput,
                                                 VIR_IdList* pPerVtxPxlList, VIR_IdList* pPerPrimList)
{
    gctUINT                    idx;
    VIR_Symbol*                pSym;
    VIR_IdList*                pOrgPerVtxPxlList;
    VIR_IdList*                pOrgPerPrimCpList;

    VIR_IdList_Init(pMM, MAX_SHADER_IO_NUM, &pPerVtxPxlList);
    VIR_IdList_Init(pMM, MAX_SHADER_IO_NUM, &pPerPrimList);

    if (bInput)
    {
        pOrgPerVtxPxlList = VIR_Shader_GetAttributes(pShader);
        pOrgPerPrimCpList = VIR_Shader_GetPerpatchAttributes(pShader);
    }
    else
    {
        pOrgPerVtxPxlList = VIR_Shader_GetOutputs(pShader);
        pOrgPerPrimCpList = VIR_Shader_GetPerpatchOutputs(pShader);
    }

    for (idx = 0; idx < VIR_IdList_Count(pOrgPerVtxPxlList); idx ++)
    {
        pSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOrgPerVtxPxlList, idx));

        if (((VIR_Shader_GetKind(pShader) == VIR_SHADER_TESSELLATION_CONTROL ||
              VIR_Shader_GetKind(pShader) == VIR_SHADER_TESSELLATION_EVALUATION ||
              VIR_Shader_GetKind(pShader) == VIR_SHADER_GEOMETRY) && bInput) ||
             ((VIR_Shader_GetKind(pShader) == VIR_SHADER_TESSELLATION_CONTROL) && !bInput))
        {
            if (isSymArrayedPerVertex(pSym))
            {
                VIR_IdList_Add(pPerVtxPxlList, VIR_IdList_GetId(pOrgPerVtxPxlList, idx));
            }
            else
            {
                VIR_IdList_Add(pPerPrimList, VIR_IdList_GetId(pOrgPerVtxPxlList, idx));
            }
        }
        else
        {
            VIR_IdList_Add(pPerVtxPxlList, VIR_IdList_GetId(pOrgPerVtxPxlList, idx));
        }
    }

    for (idx = 0; idx < VIR_IdList_Count(pOrgPerPrimCpList); idx ++)
    {
        VIR_IdList_Add(pPerPrimList, VIR_IdList_GetId(pOrgPerPrimCpList, idx));
    }
}

static gctBOOL _CheckFakeSGVForPosAndPtSz(VIR_Shader* pUpperShader, VIR_Shader* pLowerShader,
                                          VIR_NameId attrBuiltinName, gctBOOL bCalcllSlotOrHwCompIdx)
{
    VIR_OutputIdList*          pOutputIdLsts;
    VIR_Symbol*                pOutputSym;
    gctUINT                    outputIdx;

    if ((pUpperShader->shaderKind == VIR_SHADER_VERTEX) &&
        (pLowerShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
         pLowerShader->shaderKind == VIR_SHADER_GEOMETRY))
    {
        /* !!! A controversial condition here because spec's vague description:
               spec says position/point-size can be undefined to rasterizer if VS does not
               write these variables, but it does not explicitly say undefined values for
               these two builtins are legal when communicating among the programmable shader
               stages. Now ES31 CTS test regard it is legal, so we have to follow this !!!
        */

        pOutputIdLsts = VIR_Shader_GetOutputs(pUpperShader);
        for (outputIdx = 0; outputIdx < VIR_IdList_Count(pOutputIdLsts); outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

            if (attrBuiltinName == VIR_NAME_POSITION ||
                attrBuiltinName == VIR_NAME_IN_POSITION)
            {
                if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_POSITION)
                {
                    return gcvTRUE;
                }
            }
            else
            {
                if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_POINT_SIZE)
                {
                    return gcvTRUE;
                }
            }
        }
    }
    else
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL _IsFakeSGV(VIR_Shader* pUpperShader, VIR_Shader* pLowerShader,
                          VIR_NameId attrBuiltinName, gctBOOL bCalcllSlotOrHwCompIdx)
{
    VIR_OutputIdList*          pOutputIdLsts;
    VIR_Symbol*                pOutputSym;
    gctUINT                    outputIdx;

    if (attrBuiltinName == VIR_NAME_POSITION || attrBuiltinName == VIR_NAME_POINT_SIZE)
    {
        /* gl_Position and gl_PointSize (gl_in) for inputs of TS/GS are faked builtins
           because they are actually passed down from upstream shaders (not FFU) */
        if (pUpperShader->shaderKind != VIR_SHADER_VERTEX &&
            pLowerShader->shaderKind != VIR_SHADER_FRAGMENT)
        {
            return _CheckFakeSGVForPosAndPtSz(pUpperShader, pLowerShader, attrBuiltinName, bCalcllSlotOrHwCompIdx);
        }
    }

    if (attrBuiltinName == VIR_NAME_IN_POSITION || attrBuiltinName == VIR_NAME_IN_POINT_SIZE)
    {
        return _CheckFakeSGVForPosAndPtSz(pUpperShader, pLowerShader, attrBuiltinName, bCalcllSlotOrHwCompIdx);
    }

    if ((attrBuiltinName == VIR_NAME_CLIP_DISTANCE || attrBuiltinName == VIR_NAME_IN_CLIP_DISTANCE)
        ||
        (attrBuiltinName == VIR_NAME_CULL_DISTANCE || attrBuiltinName == VIR_NAME_IN_CULL_DISTANCE)
        )
    {
        /* All OGL that before GL4.00 need ClipDistance/CullDistance have the same size for all stages. */
        if (VIR_Shader_IsDesktopGL(pLowerShader) && !VIR_Shader_IsGL40(pLowerShader))
        {
            return gcvTRUE;
        }

        if (pUpperShader->shaderKind != VIR_SHADER_VERTEX)
        {
            return gcvTRUE;
        }
    }

    if (attrBuiltinName == VIR_NAME_FRONT_COLOR || attrBuiltinName == VIR_NAME_FRONT_SECONDARY_COLOR ||
        attrBuiltinName == VIR_NAME_BACK_COLOR || attrBuiltinName == VIR_NAME_BACK_SECONDARY_COLOR)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);
        return gcvTRUE;
    }

    if (attrBuiltinName == VIR_NAME_LAYER)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);

        return bCalcllSlotOrHwCompIdx;
    }

    if (attrBuiltinName == VIR_NAME_PRIMITIVE_ID &&
        pLowerShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        /* If pre-RA shader has primitiveid output, then it can be regarded as normal IOs,
           otherwise, it will be regarded as true SGV */

        pOutputIdLsts = VIR_Shader_GetOutputs(pUpperShader);
        for (outputIdx = 0; outputIdx < VIR_IdList_Count(pOutputIdLsts); outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_PRIMITIVE_ID)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL _IsFakeSIV(VIR_Shader* pUpperShader, VIR_Shader* pLowerShader,
                           VIR_Symbol* pOutputSym, gctBOOL bCalcllSlotOrHwCompIdx)
{
    VIR_NameId outputBuiltinName = VIR_Symbol_GetName(pOutputSym);
    if (outputBuiltinName == VIR_NAME_POSITION || outputBuiltinName == VIR_NAME_POINT_SIZE)
    {
        /* gl_Position and gl_PointSize (gl_out) for outputs of shader that is not linked
           to PS are faked builtins because they are not actually needed by RA FFU */
        if (pLowerShader->shaderKind != VIR_SHADER_FRAGMENT)
        {
            return gcvTRUE;
        }
        /* if gl_PointSize is never defined in vertex shader, treat it as faked
         * TODO::another check is if driver is not drawing a point, gl_PointSize is faked (it need recompilation)
         */
        if (outputBuiltinName == VIR_NAME_POINT_SIZE &&
            pUpperShader->shaderKind == VIR_SHADER_VERTEX &&
            (!isSymStaticallyUsed(pOutputSym)) &&
            (isSymUnDef(pOutputSym)))
        {
            return gcvTRUE;
        }
    }

    if ((outputBuiltinName == VIR_NAME_CLIP_DISTANCE || outputBuiltinName == VIR_NAME_IN_CLIP_DISTANCE)
        ||
        (outputBuiltinName == VIR_NAME_CULL_DISTANCE || outputBuiltinName == VIR_NAME_IN_CULL_DISTANCE)
        )
    {
        /* All OGL that before GL4.00 need ClipDistance/CullDistance have the same size for all stages. */
        if (VIR_Shader_IsDesktopGL(pLowerShader) && !VIR_Shader_IsGL40(pLowerShader))
        {
            return gcvTRUE;
        }

        if (pLowerShader->shaderKind != VIR_SHADER_FRAGMENT)
        {
            return gcvTRUE;
        }
    }

    if (outputBuiltinName == VIR_NAME_LAYER)
    {
        gcmASSERT(pUpperShader->shaderKind == VIR_SHADER_GEOMETRY);

        return bCalcllSlotOrHwCompIdx;
    }

    if (outputBuiltinName == VIR_NAME_PRIMITIVE_ID)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);

        return gcvTRUE;
    }

    if (outputBuiltinName == VIR_NAME_FRONT_COLOR || outputBuiltinName == VIR_NAME_FRONT_SECONDARY_COLOR ||
        outputBuiltinName == VIR_NAME_BACK_COLOR || outputBuiltinName == VIR_NAME_BACK_SECONDARY_COLOR)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* check or set IoLocationMask according to 4th argument */
static VSC_ErrCode _CheckIoLocationMask(
    VIR_Symbol      *pVirIoSym,
    VSC_BIT_VECTOR  *locationMask,
    gctUINT         location,
    gctBOOL         setLocationMask,
    gctBOOL         *hasComponentQualifier
    )
{
    gctUINT   j, startComponent = 0, endComponent = 0;
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    *hasComponentQualifier = VIR_Symbol_GetStartAndEndComponentForIO(pVirIoSym, gcvTRUE, &startComponent, &endComponent);

    /* Check if there is any component overlap. */
    for (j = startComponent; j < endComponent; j++)
    {
        if (vscBV_TestBit(locationMask, location * VIR_CHANNEL_NUM + j))
        {
            errCode = VSC_ERR_LOCATION_ALIASED;
            ON_ERROR(errCode, "Check aliased location and component");
        }
        if (setLocationMask)
        {
            vscBV_SetBit(locationMask, location * VIR_CHANNEL_NUM + j);
        }
    }
OnError:
    return errCode;
}

static VSC_ErrCode _CheckIoAliasedLocationPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                    VIR_Shader* pShader,
                                                    VIR_IdList* pVirIoIdLsts,
                                                    gctBOOL bInput,
                                                    VIR_IdList* pAliasListArray,
                                                    VIR_IdList* pComponentMapListArray)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_BIT_VECTOR             locationMask;
    VIR_Symbol*                pVirIoSym;
    gctUINT                    i, virIo, thisVirIoRegCount, location;
    gctUINT                    virIoCount = VIR_IdList_Count(pVirIoIdLsts);
    VSC_BIT_VECTOR             visitedIoIdx;
    gctBOOL                    bCheckAliasedAttribute = (pAliasListArray != gcvNULL);
    gctBOOL                    bCheckComponentMapping = (pComponentMapListArray != gcvNULL);

    if (bCheckAliasedAttribute && !bInput)
    {
        /* We can only support the aliased attributes, we can't support the aliased outputs. */
        gcmASSERT(gcvFALSE);
    }

    if (bCheckAliasedAttribute && bCheckComponentMapping)
    {
        gcmASSERT(gcvFALSE);
    }

    /*
    ** Since in Vulkan, GL440 and above, IOs can consume the individual components within a location,
    ** so we need to make the locationMask per-component.
    */
    vscBV_Initialize(&locationMask, pBaseLinkHelper->pMM, MAX_SHADER_IO_NUM * VIR_CHANNEL_NUM);
    if (bCheckComponentMapping)
    {
        vscBV_Initialize(&visitedIoIdx, pBaseLinkHelper->pMM, virIoCount);
    }

    for (virIo = 0; virIo < virIoCount; virIo ++)
    {
        /* if virIo has been dealt because of previous output is array and use same location, skip it */
        if (!bCheckAliasedAttribute && bCheckComponentMapping && vscBV_TestBit(&visitedIoIdx, virIo))
        {
            continue;
        }

        if (!bCheckAliasedAttribute && bCheckComponentMapping)
        {
            vscBV_SetBit(&visitedIoIdx, virIo);
        }

        pVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirIoIdLsts, virIo));

        /* We don't need to check the builtin or unused attributes/outputs. */
        if (VIR_Shader_IsNameBuiltIn(pShader, VIR_Symbol_GetName(pVirIoSym)) ||
            isSymUnused(pVirIoSym))
        {
            continue;
        }

        thisVirIoRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pVirIoSym);
        location = VIR_Symbol_GetLocation(pVirIoSym);

        /* The location is not specified, skip it. */
        if (location == -1)
        {
            continue;
        }

        for (i = (gctUINT)location; i < (gctUINT)location + thisVirIoRegCount; i ++)
        {
            gctBOOL    hasComponentQualifier = gcvFALSE;

            if (bCheckAliasedAttribute)
            {
                gcmASSERT(i < MAX_SHADER_IO_NUM);
            }
            else
            {
                if (bInput && (i >= MAX_SHADER_IO_NUM) && (pShader->shaderKind == VIR_SHADER_VERTEX))
                {
                    errCode = VSC_ERR_NOT_SUPPORTED;
                    ON_ERROR(errCode, "Check aliased location");
                }
                else
                {
                    gcmASSERT(i < MAX_SHADER_IO_NUM);
                }
            }

            /* If the aliased list exist, just copy the symId. */
            if (bCheckAliasedAttribute)
            {
                VIR_IdList_Add(&pAliasListArray[i], VIR_Symbol_GetIndex(pVirIoSym));
            }
            else
            {
                errCode = _CheckIoLocationMask(pVirIoSym, &locationMask, i, gcvTRUE, &hasComponentQualifier);
                ON_ERROR(errCode, "Check aliased location");

                /* Save the component qualifier. */
                if (hasComponentQualifier && bCheckComponentMapping)
                {
                    VIR_IdList_Add(&pComponentMapListArray[i], VIR_Symbol_GetIndex(pVirIoSym));

                    /* if current output is array, the location covers a range[location, location+thisVirIoRegCount)
                        * check next outputs if has same location and add the symbol to this list
                        * an example from vulkan-1.1 glsl.440.linkage.varying.component.frag_out.two_vec4
                        * layout(location = 0, component = 0) out float dEQP_FragColor_0_0;
                        * layout(location = 0, component = 1) out vec3  dEQP_FragColor_0_1[2];
                        * layout(location = 1, component = 0) out float dEQP_FragColor_1_0;
                        * 3 output symbols are added to pComponentMapListArray[0] since second array output covers location [0,1]
                        */
                    if (thisVirIoRegCount > 1)
                    {
                        gctUINT next;
                        for (next = virIo + 1; next < virIoCount; next++)
                        {
                            VIR_Symbol* pNextVirIoSym;
                            gctUINT nextlocation;
                            gctBOOL nextHasComonentQualifer;
                            if (vscBV_TestBit(&visitedIoIdx, next))
                            {
                                continue;
                            }
                            pNextVirIoSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pVirIoIdLsts, next));
                            nextlocation = VIR_Symbol_GetLocation(pNextVirIoSym);
                            errCode = _CheckIoLocationMask(pNextVirIoSym, &locationMask, nextlocation, gcvFALSE, &nextHasComonentQualifer); /* check only */
                            ON_ERROR(errCode, "Check aliased location");
                            if (nextHasComonentQualifer && (nextlocation > i && (nextlocation < (location + thisVirIoRegCount))))
                            {
                                VIR_IdList_Add(&pComponentMapListArray[i], VIR_Symbol_GetIndex(pNextVirIoSym));
                                vscBV_SetBit(&visitedIoIdx, next);
                                errCode = _CheckIoLocationMask(pNextVirIoSym, &locationMask, nextlocation, gcvTRUE, &nextHasComonentQualifer); /* set locationMask */
                                ON_ERROR(errCode, "Check aliased location");
                            }
                        }
                    }
                }
            }
        }
    }

OnError:
    vscBV_Finalize(&locationMask);
    if (bCheckComponentMapping)
    {
        vscBV_Finalize(&visitedIoIdx);
    }

    return errCode;
}

static gctBOOL  _NeedCheckInterpolation(VIR_Shader* pShader)
{
    gctBOOL                    bCheckInterpolation = gcvTRUE;

    /* According to vulkan spec, interpolation decorations are not required to match. */
    if (VIR_Shader_IsVulkan(pShader))
    {
        bCheckInterpolation = gcvFALSE;
    }

    return bCheckInterpolation;
}

static VSC_ErrCode _LinkSVIoBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                   FSL_STAGE fslStage,
                                                   VIR_Shader* pUpperShader,
                                                   VIR_Shader* pLowerShader,
                                                   VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                   VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                                   gctUINT* pSvInputIdx,
                                                   gctUINT* pSvOutputIdx)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    svInputIdx = *pSvInputIdx, svOutputIdx = *pSvOutputIdx;
    gctUINT                    attrIdx, outputIdx;
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLstsOfLowerShader);
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLstsOfUpperShader);
    VIR_Symbol*                pOutputSym = gcvNULL;
    VIR_Symbol*                pAttrSym = gcvNULL;
    gctUINT                    thisOutputRegCount, thisAttrRegCount;
    VSC_BIT_VECTOR             outputMask, attrMask;

    vscBV_Initialize(&outputMask, pBaseLinkHelper->pMM, outputCount);
    vscBV_Initialize(&attrMask, pBaseLinkHelper->pMM, attrCount);

    /* Link the matched IO first cause we need to make sure the ioIdx is matched. */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

        if (VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym))
            &&
            !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym,
                        (fslStage == FSL_STAGE_LL_SLOT_CALC)))
        {
            /* Mark this output used */
            VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

            if (fslStage == FSL_STAGE_LL_SLOT_CALC)
            {
                for (attrIdx = 0; attrIdx < attrCount; attrIdx++)
                {
                    pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

                    /* Only consider used one */
                    if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
                    {
                        continue;
                    }

                    if (vscBV_TestBit(&attrMask, attrIdx))
                    {
                        continue;
                    }

                    if (VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym))
                        &&
                        !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym),
                                    (fslStage == FSL_STAGE_LL_SLOT_CALC))
                        &&
                        VIR_Symbol_isNameMatch(pLowerShader, pAttrSym, pUpperShader, pOutputSym))
                    {
                        /* Set output slot. */
                        vscBV_SetBit(&outputMask, outputIdx);
                        thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                        VIR_Symbol_SetFirstSlot(pOutputSym, svOutputIdx);
                        svOutputIdx += thisOutputRegCount;

                        /* Set attribute slot. */
                        vscBV_SetBit(&attrMask, attrIdx);
                        thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pLowerShader, pAttrSym);

                        VIR_Symbol_SetFirstSlot(pAttrSym, svInputIdx);
                        svInputIdx += thisAttrRegCount;

                        break;
                    }
                }
            }
        }
    }

    /* Set the rest output/attribute. */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

        if (vscBV_TestBit(&outputMask, outputIdx))
        {
            continue;
        }

        if (VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym))
            &&
            !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym,
                        (fslStage == FSL_STAGE_LL_SLOT_CALC)))
        {
            /* Mark this output used */
            VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

            if (fslStage == FSL_STAGE_LL_SLOT_CALC)
            {
                thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                VIR_Symbol_SetFirstSlot(pOutputSym, svOutputIdx);
                svOutputIdx += thisOutputRegCount;
            }
        }
    }

    if (fslStage == FSL_STAGE_LL_SLOT_CALC)
    {
        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

            if (vscBV_TestBit(&attrMask, attrIdx))
            {
                continue;
            }

            /* Only consider used one */
            if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
            {
                continue;
            }

            if (VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym))
                &&
                !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym),
                            (fslStage == FSL_STAGE_LL_SLOT_CALC)))
            {
                thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pLowerShader, pAttrSym);

                VIR_Symbol_SetFirstSlot(pAttrSym, svInputIdx);
                svInputIdx += thisAttrRegCount;
            }
        }
    }

    /* Update the sv io index. */
    if (pSvInputIdx)
    {
        *pSvInputIdx = svInputIdx;
    }

    if (pSvOutputIdx)
    {
        *pSvOutputIdx = svOutputIdx;
    }

    vscBV_Finalize(&outputMask);
    vscBV_Finalize(&attrMask);
    return errCode;
}


static gctBOOL
_CheckIOIdentical(
    IN VIR_Shader*              pUpperShader,
    IN VIR_Symbol*              pOutputSym,
    IN VIR_Type*                pOutputType,
    IN VIR_Shader*              pLowerShader,
    IN VIR_Symbol*              pAttrSym,
    IN VIR_Type*                pAttrType
    )
{
    if (!VIR_Type_Identical(pLowerShader, pAttrType, pUpperShader, pOutputType))
    {
        /*
        ** It is legal that the output is a vector with the same basic type and has at least as many components as the
        ** input, and the common component type of the input and output is 32-bit integer or
        ** floating-point (64-bitcomponent types are excluded).
        */
        if (VIR_Shader_IsVulkan(pUpperShader))
        {
            VIR_TypeId outputCompTypeId = VIR_GetTypeComponentType(VIR_Type_GetIndex(pOutputType));
            VIR_TypeId attrCompTypeId = VIR_GetTypeComponentType(VIR_Type_GetIndex(pAttrType));

            if ((VIR_Type_isVector(pOutputType))
                &&
                (VIR_Type_isVector(pAttrType) || VIR_Type_isScalar(pAttrType))
                &&
                (outputCompTypeId == attrCompTypeId)
                &&
                (VIR_TypeId_isInteger(attrCompTypeId) || VIR_TypeId_isFloat(attrCompTypeId))
                &&
                (VIR_GetTypeComponents(VIR_Type_GetIndex(pOutputType)) >= VIR_GetTypeComponents(VIR_Type_GetIndex(pAttrType))))
            {
                VIR_Symbol_SetFixedTypeId(pOutputSym, VIR_Type_GetIndex(pAttrType));
                return gcvTRUE;
            }
        }
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL _CompareIoByComponent(VIR_Symbol* pSym1,
                                     VIR_Symbol* pSym2)
{
    return VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pSym1)) > VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pSym2));
}

static VSC_ErrCode _LinkIoBetweenTwoShaderStagesPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                          FSL_STAGE fslStage,
                                                          VIR_Shader* pUpperShader,
                                                          VIR_Shader* pLowerShader,
                                                          VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                          VIR_OutputIdList* pOutputIdLstsOfUpperShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    attrIdx, outputIdx, ioIdx = 0;
    gctUINT                    svInputIdx, svOutputIdx, soOutputIdx, hsOutputIdx;
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLstsOfLowerShader);
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLstsOfUpperShader);
    gctUINT                    soOutputCount = 0;
    VIR_Symbol*                pAttrSym;
    VIR_Symbol*                pOutputSym;
    VIR_Symbol*                pSoSym;
    VSC_BIT_VECTOR             outputWorkingMask;
    gctUINT                    thisOutputRegCount, thisAttrRegCount;
    VIR_Type*                  pAttrType;
    VIR_Type*                  pOutputType;
    gctBOOL                    bLlSlotForLayerCalced = gcvFALSE;
    gctBOOL                    bCheckInterpolation = _NeedCheckInterpolation(pUpperShader);

    /*
       !!!! IO-index layout rule:
            general IOs                                    +
            SV IOs                                         +
            stream-out only outputs (if they are present)  +
            hs's per-cp outputs that are not used by next stage, but used in shader for other HS kickoff
       !!!!
    */

    /* No need to go on if there are obvious results */
    if (outputCount == 0 && attrCount == 0)
    {
        return VSC_ERR_NONE;
    }

    /* Check io aliased locations. NOTE that it looks spec needs this check before true IO linkage, but if
       aliased IOs are not used by shader, we actually can regard the shader as good shader because there
       will not be ambigous for counterpart from linked shaders. Do we need put this check to the end of
       this function??? */
    if (fslStage == FSL_STAGE_API_SPEC_CHECK)
    {
        VIR_IdList*                pOutputCompArrayList = gcvNULL;
        VIR_IdList*                pInputCompArrayList = gcvNULL;
        VIR_IdList*                pList = gcvNULL;
        gctUINT                    i;
        gctBOOL                    bCheckComponentMap = VIR_Shader_SupportIoCommponentMapping(pUpperShader);

        if (bCheckComponentMap)
        {
            VIR_Shader_CreateOutputComponentMapList(pUpperShader);
            pOutputCompArrayList = VIR_Shader_GetOutputComponentMapList(pUpperShader);
            gcmASSERT(pOutputCompArrayList);

            VIR_Shader_CreateAttributeComponentMapList(pLowerShader);
            pInputCompArrayList = VIR_Shader_GetAttributeComponentMapList(pLowerShader);
            gcmASSERT(pInputCompArrayList);
        }

        errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper,
                                                   pUpperShader,
                                                   pOutputIdLstsOfUpperShader,
                                                   gcvFALSE,
                                                   gcvNULL,
                                                   pOutputCompArrayList);
        CHECK_ERROR(errCode, "Check io aliased location");

        errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper,
                                                   pLowerShader,
                                                   pAttrIdLstsOfLowerShader,
                                                   gcvTRUE,
                                                   gcvNULL,
                                                   pInputCompArrayList);
        CHECK_ERROR(errCode, "Check io aliased location");

        if (bCheckComponentMap)
        {
            for (i = 0; i < MAX_SHADER_IO_NUM; i++)
            {
                pList = &pOutputCompArrayList[i];
                if (VIR_IdList_Count(pList) > 1)
                {
                    VIR_Shader_BubbleSortSymIdList(pUpperShader, pList, _CompareIoByComponent, VIR_IdList_Count(pList));
                    VIR_Shader_SetFlagExt1(pUpperShader, VIR_SHFLAG_EXT1_HAS_OUTPUT_COMP_MAP);
                }

                pList = &pInputCompArrayList[i];
                if (VIR_IdList_Count(pList) > 1)
                {
                    VIR_Shader_BubbleSortSymIdList(pLowerShader, pList, _CompareIoByComponent, VIR_IdList_Count(pList));
                    VIR_Shader_SetFlagExt1(pLowerShader, VIR_SHFLAG_EXT1_HAS_INPUT_COMP_MAP);
                }
            }
        }
    }

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM, outputCount);

    /* Mark all outputs of upper-shader as not used */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));
        VIR_Symbol_SetFlag(pOutputSym, VIR_SYMFLAG_UNUSED);
    }

    /* For each attribute of lower-shader, find whether upper-shader has corresponding output */
    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

        /* Only consider used one */
        if (fslStage == FSL_STAGE_API_SPEC_CHECK)
        {
            if (!VIR_Symbol_HasFlag(pAttrSym, VIR_SYMFLAG_STATICALLY_USED))
            {
                continue;
            }
        }
        else
        {
            if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
            {
                continue;
            }
        }

        /* Skip SGV now */
        if (VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym))
            &&
            !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym),
                        (fslStage == FSL_STAGE_LL_SLOT_CALC)))
        {
            continue;
        }

        pAttrType = VIR_Symbol_GetType(pAttrSym);
        thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pLowerShader, pAttrSym);

        /* For each output of upper-shader */
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            if (vscBV_TestBit(&outputWorkingMask, outputIdx))
            {
                continue;
            }

            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            if (isSymVectorizedOut(pOutputSym))
            {
                continue;
            }

            {
                /* If flag SKIP-NAME-CHECK is present, check location only. */
                if (isSymSkipNameCheck(pAttrSym))
                {
                    /* When a symbol has SKIP-NAME-CHECK flag, its location must be NOT -1. */
                    gcmASSERT(VIR_Symbol_GetLocation(pAttrSym) != -1);
                    if (VIR_Symbol_GetLocation(pAttrSym) != VIR_Symbol_GetLocation(pOutputSym))
                    {
                        continue;
                    }
                }
                /* Name or location of the output and attribute must be matched */
                else if (!(VIR_Symbol_isNameMatch(pLowerShader, pAttrSym, pUpperShader, pOutputSym) ||
                         (VIR_Symbol_GetLocation(pOutputSym) != -1 &&
                          VIR_Symbol_GetLocation(pOutputSym) == VIR_Symbol_GetLocation(pAttrSym))))
                {
                    continue;
                }
            }

            /* If any of them has the component qualifier, they must be matched. */
            if (VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pAttrSym)) != VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pOutputSym)))
            {
                continue;
            }

            pOutputType = VIR_Symbol_GetType(pOutputSym);
            thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

            /* Types must be matched */
            if (!_CheckIOIdentical(pUpperShader,
                                   pOutputSym,
                                   pOutputType,
                                   pLowerShader,
                                   pAttrSym,
                                   pAttrType))
            {
                errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                ON_ERROR(errCode, "Link Io between two shader stages");
            }

            if (bCheckInterpolation)
            {
                if ((isSymFlat(pAttrSym) != isSymFlat(pOutputSym)) || (isSymNoperspective(pAttrSym) != isSymNoperspective(pOutputSym)))
                {
                    errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Link Io between two shader stages");
                }

                if ((isSymSample(pAttrSym) != isSymSample(pOutputSym)) &&
                    !VIR_Shader_IsES31AndAboveCompiler(pUpperShader))
                {
                    errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Link Io between two shader stages");
                }
            }

            /* For a ES20 shader, invariant of varyings on VS and PS must match. */
            if (VIR_Shader_IsES20Compiler(pLowerShader))
            {
                if (isSymInvariant(pOutputSym) != isSymInvariant(pAttrSym))
                {
                    errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Link Io between two shader stages");
                }
            }

            /* Insure reg count of input and ouput are same */
            if (thisAttrRegCount == thisOutputRegCount)
            {
                /* Mark this output used */
                VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

                if (fslStage == FSL_STAGE_LL_SLOT_CALC)
                {
                    /* No need to set ll-slot as location specified by app here because it is an IO
                       among the shader stages which are linked together as non-seperated shaders */
                    VIR_Symbol_SetFirstSlot(pAttrSym, ioIdx);
                    VIR_Symbol_SetFirstSlot(pOutputSym, ioIdx);

                    ioIdx += thisOutputRegCount;

                    if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_LAYER)
                    {
                        bLlSlotForLayerCalced = gcvTRUE;
                    }
                }

                vscBV_SetBit(&outputWorkingMask, outputIdx);

                break;
            }
        }

        if (outputIdx == outputCount)
        {
            /* No matching vertex output found. */
            errCode = VSC_ERR_UNDECLARED_VARYING;
            ON_ERROR(errCode, "Link Io between two shader stages");
        }
    }

    if (fslStage == FSL_STAGE_LL_SLOT_CALC && !bLlSlotForLayerCalced && pLowerShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));
            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_LAYER)
            {
                /* Mark gl_layer to be used */
                VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

                /* Add layer to input of ps */
                pAttrSym = VIR_Shader_AddBuiltinAttribute(pLowerShader, VIR_TYPE_UINT32, gcvFALSE, VIR_NAME_LAYER);

                /* Per HW requirement, layer is highp*/
                VIR_Symbol_SetPrecision(pAttrSym, VIR_PRECISION_HIGH);

                VIR_Symbol_SetFirstSlot(pAttrSym, ioIdx);
                VIR_Symbol_SetFirstSlot(pOutputSym, ioIdx);
                ioIdx ++;

                break;
            }
        }
    }

    /* Then for SV IOs  */
    svInputIdx = svOutputIdx = ioIdx;
    _LinkSVIoBetweenTwoShaderStages(pBaseLinkHelper,
                                    fslStage,
                                    pUpperShader,
                                    pLowerShader,
                                    pAttrIdLstsOfLowerShader,
                                    pOutputIdLstsOfUpperShader,
                                    &svInputIdx,
                                    &svOutputIdx);

    /* Then for stream-out outputs */
    soOutputIdx = svOutputIdx;

    if (pUpperShader->transformFeedback.varyings)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);

        soOutputCount = VIR_IdList_Count(pUpperShader->transformFeedback.varyings);

        if (soOutputCount > 0)
        {
            for (outputIdx = 0; outputIdx < soOutputCount; outputIdx ++)
            {
                pSoSym = VIR_Shader_GetSymFromId(pUpperShader,
                                    VIR_IdList_GetId(pUpperShader->transformFeedback.varyings, outputIdx));

                pOutputSym = VIR_Symbol_GetIndexingInfo(pUpperShader, pSoSym).underlyingSym;

                if (VIR_Symbol_GetFirstSlot(pOutputSym) == NOT_ASSIGNED)
                {
                    /* Mark this output used */
                    VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

                    if (fslStage == FSL_STAGE_LL_SLOT_CALC)
                    {
                        thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                        VIR_Symbol_SetFirstSlot(pOutputSym, soOutputIdx);
                        soOutputIdx += thisOutputRegCount;
                    }
                }
            }
        }
    }

    /* Last for hs's per-cp outputs that are not used by next stage, but used
       in shader for other HS kickoff */
    hsOutputIdx = soOutputIdx;

    if (pUpperShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            if (!VIR_Symbol_HasFlag(pOutputSym, VIR_SYMFLAG_STATICALLY_USED))
            {
                continue;
            }

            if (isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym))
            {
                gcmASSERT(VIR_Symbol_GetFirstSlot(pOutputSym) == NOT_ASSIGNED);

                /* Mark this output used */
                VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

                if (fslStage == FSL_STAGE_LL_SLOT_CALC)
                {
                    thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                    VIR_Symbol_SetFirstSlot(pOutputSym, hsOutputIdx);
                    hsOutputIdx += thisOutputRegCount;
                }
            }
        }
    }

OnError:
    vscBV_Finalize(&outputWorkingMask);

    return errCode;
}

static VSC_ErrCode _LinkIoBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                 FSL_STAGE fslStage,
                                                 VIR_Shader* pUpperShader,
                                                 VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLstOfLowerShader, workingPerPrimAttrIdLstOfLowerShader;
    VIR_OutputIdList           workingPerVtxPxlAttrIdLstOfUpperShader, workingPerPrimAttrIdLstOfUpperShader;

    _ConvertVirPerVtxPxlAndPerPrimIoList(pLowerShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLstOfLowerShader,
                                         &workingPerPrimAttrIdLstOfLowerShader);

    _ConvertVirPerVtxPxlAndPerPrimIoList(pUpperShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLstOfUpperShader,
                                         &workingPerPrimAttrIdLstOfUpperShader);

    /* Per vtx/Pxl */
    errCode = _LinkIoBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, fslStage, pUpperShader, pLowerShader,
                                                     &workingPerVtxPxlAttrIdLstOfLowerShader,
                                                     &workingPerVtxPxlAttrIdLstOfUpperShader);
    ON_ERROR(errCode, "Link Io between two shader stages per vtx/pxl");

    /* Per prim */
    errCode = _LinkIoBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, fslStage, pUpperShader, pLowerShader,
                                                     &workingPerPrimAttrIdLstOfLowerShader,
                                                     &workingPerPrimAttrIdLstOfUpperShader);
    ON_ERROR(errCode, "Link Io between two shader stages per prim");

    /* Record linked shader stage type each other */
    pUpperShader->outLinkedShaderStage = pLowerShader->shaderKind;
    pLowerShader->inLinkedShaderStage = pUpperShader->shaderKind;

OnError:
    return errCode;
}

static VSC_ErrCode _CalcInputLowLevelSlotPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                   VIR_Shader* pShader,
                                                   VIR_AttributeIdList* pAttrIdLsts,
                                                   gctBOOL bSeperatedShaders)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    attrIdx, ioIdx = 0;
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLsts);
    gctINT                     location;
    VIR_IdList*                pAliasedAttrList = gcvNULL;
    VIR_Symbol*                pAttrSym;
    gctUINT                    thisAttrRegCount, i, j;
    VSC_BIT_VECTOR             inputWorkingMask;
    gctBOOL                    bHasNoAssignedLocation = gcvFALSE;
    gctBOOL                    bDirectlyUseLocation = (pShader->shaderKind != VIR_SHADER_VERTEX) && bSeperatedShaders;
    gctBOOL                    bHasAliasedAttribute = VIR_Shader_HasAliasedAttribute(pShader);

    vscBV_Initialize(&inputWorkingMask, pBaseLinkHelper->pMM, MAX_SHADER_IO_NUM);

    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrIdLsts, attrIdx));

        if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
        {
            thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);

            /* Get the location and the location list. */
            location = VIR_Symbol_GetLocation(pAttrSym);
            gcmASSERT(location < MAX_SHADER_IO_NUM);

            if (location != -1 && bHasAliasedAttribute)
            {
                pAliasedAttrList = &VIR_Shader_GetAttributeAliasList(pShader)[location];
            }
            else
            {
                pAliasedAttrList = gcvNULL;
            }

            /* For vertex shader, GL spec demands location of attribute specified
               by shader can not exceeds HW supported count even if total used
               attributes does not exceed this HW limitation. But such location
               can be changed by API glBindAttribLocation after program was linked,
               so why compiler needs take such check? This is a very weired behavior
               defined by spec !!! */
            if (pShader->shaderKind == VIR_SHADER_VERTEX && VIR_Symbol_GetLocation(pAttrSym) != -1)
            {
                if ((pAttrSym->layout.location + thisAttrRegCount) > pBaseLinkHelper->pHwCfg->maxAttributeCount)
                {
                    errCode = VSC_ERR_TOO_MANY_ATTRIBUTES;
                    ON_ERROR(errCode, "Calc input ll slot");
                }
            }

            /* It has been assigned by other aliased attribute. */
            if (VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED)
            {
                continue;
            }

            /* Check if any aliased attribute has been processed, if found, just use its info. */
            if (pAliasedAttrList && VIR_IdList_Count(pAliasedAttrList) > 1)
            {
                gctUINT firstSlot = NOT_ASSIGNED;
                VIR_Symbol* pCopySym = gcvNULL;

                for (j = 0; j < VIR_IdList_Count(pAliasedAttrList); j++)
                {
                    VIR_Symbol* pAliasedSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAliasedAttrList, j));

                    /* Find a valid symbol, use it to set all the other aliased symbol. */
                    if (VIR_Symbol_GetFirstSlot(pAliasedSym) != NOT_ASSIGNED)
                    {
                        firstSlot = VIR_Symbol_GetFirstSlot(pAliasedSym);
                        pCopySym = pAliasedSym;

                        for (j = 0; j < VIR_IdList_Count(pAliasedAttrList); j++)
                        {
                            pAliasedSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAliasedAttrList, j));

                            if (pCopySym == pAliasedSym)
                            {
                                continue;
                            }

                            /*
                            ** For those aliased attributes that without firstSlot, update it.
                            ** For those aliased attributes that with firstSlot, make sure that they are the same.
                            */
                            if (VIR_Symbol_GetFirstSlot(pAliasedSym) == NOT_ASSIGNED)
                            {
                                VIR_Symbol_SetFirstSlot(pAliasedSym, firstSlot);
                            }
                            else
                            {
                                gcmASSERT(VIR_Symbol_GetFirstSlot(pAliasedSym) == firstSlot);
                            }
                        }
                        break;
                    }
                }
            }

            /* It has been assigned by other aliased attribute. */
            if (VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED)
            {
                continue;
            }

            if (bDirectlyUseLocation)
            {
                if (location != -1)
                {
                    VIR_Symbol_SetFirstSlot(pAttrSym, (gctUINT)location);

                    for (i = (gctUINT)location;
                         i < (gctUINT)location + thisAttrRegCount;
                         i ++)
                    {
                        if (bHasAliasedAttribute)
                        {
                            gcmASSERT(i < MAX_SHADER_IO_NUM);
                        }
                        else
                        {
                            if (i >= MAX_SHADER_IO_NUM && pShader->shaderKind == VIR_SHADER_VERTEX)
                            {
                                errCode = VSC_ERR_TOO_MANY_ATTRIBUTES;
                                ON_ERROR(errCode, "Calc input ll slot");
                            }
                            else
                            {
                                gcmASSERT(i < MAX_SHADER_IO_NUM);
                            }
                        }
                        vscBV_SetBit(&inputWorkingMask, i);
                    }
                }
                else
                {
                    bHasNoAssignedLocation = gcvTRUE;
                }
            }
            else
            {
                VIR_Symbol_SetFirstSlot(pAttrSym, ioIdx);

                for (i = ioIdx;
                     i < ioIdx + thisAttrRegCount;
                     i ++)
                {
                    if (bHasAliasedAttribute)
                    {
                        gcmASSERT(i < MAX_SHADER_IO_NUM);
                    }
                    else
                    {
                        if (i >= MAX_SHADER_IO_NUM && pShader->shaderKind == VIR_SHADER_VERTEX)
                        {
                            errCode = VSC_ERR_TOO_MANY_ATTRIBUTES;
                            ON_ERROR(errCode, "Calc input ll slot");
                        }
                        else
                        {
                            gcmASSERT(i < MAX_SHADER_IO_NUM);
                        }
                    }

                    vscBV_SetBit(&inputWorkingMask, i);
                }

                ioIdx += thisAttrRegCount;
            }
        }
    }

    if (bDirectlyUseLocation && bHasNoAssignedLocation)
    {
        /* We hit here due to 2 reasons:
           1. App does not set locations for all app-level inputs
           2. For some internals or SVs, location can be missed
        */

        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrIdLsts, attrIdx));

            if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
            {
                if (VIR_Symbol_GetLocation(pAttrSym) == -1)
                {
                    thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);

                    ioIdx = vscBV_FindContinuousClearBitsForward(&inputWorkingMask, thisAttrRegCount, 0);

                    if (ioIdx == INVALID_BIT_LOC)
                    {
                        errCode = VSC_ERR_INVALID_DATA;
                        ON_ERROR(errCode, "Calc input ll slot");
                    }

                    VIR_Symbol_SetFirstSlot(pAttrSym, ioIdx);

                    for (i = ioIdx;
                         i < ioIdx + thisAttrRegCount;
                         i ++)
                    {
                        if (bHasAliasedAttribute)
                        {
                            gcmASSERT(i < MAX_SHADER_IO_NUM);
                        }
                        else
                        {
                            if (i >= MAX_SHADER_IO_NUM && pShader->shaderKind == VIR_SHADER_VERTEX)
                            {
                                errCode = VSC_ERR_TOO_MANY_ATTRIBUTES;
                                ON_ERROR(errCode, "Calc input ll slot");
                            }
                            else
                            {
                                gcmASSERT(i < MAX_SHADER_IO_NUM);
                            }
                        }

                        vscBV_SetBit(&inputWorkingMask, i);
                    }
                }
            }
        }
    }

OnError:
    vscBV_Finalize(&inputWorkingMask);

    return errCode;
}

static VSC_ErrCode _CalcInputLowLevelSlot(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                          VIR_Shader* pShader,
                                          gctBOOL bSeperatedShaders)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CalcInputLowLevelSlotPerExeObj(pBaseLinkHelper, pShader,
                                              &workingPerVtxPxlAttrIdLst, bSeperatedShaders);
    ON_ERROR(errCode, "Calc input ll slot per vtx/pxl");

    /* Per prim */
    errCode = _CalcInputLowLevelSlotPerExeObj(pBaseLinkHelper, pShader,
                                              &workingPerPrimAttrIdLst, bSeperatedShaders);
    ON_ERROR(errCode, "Calc input ll slot per prim");

OnError:
    return errCode;
}

static VSC_ErrCode _CalcOutputLowLevelSlotPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                    VIR_Shader* pShader,
                                                    VIR_OutputIdList* pOutputIdLsts,
                                                    gctBOOL bSeperatedShaders)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    outputIdx, ioIdx = 0;
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLsts);
    VIR_Symbol*                pOutputSym;
    gctUINT                    thisOutputRegCount, i;
    gctINT                     location;
    VSC_BIT_VECTOR             outputWorkingMask;
    VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper = gcvNULL;
    gctBOOL                    bHasNoAssignedLocation = gcvFALSE;

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM, MAX_SHADER_IO_NUM);

    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

        thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pOutputSym);

        /* For pixel shader, GL spec demands location of output specified by shader
           can not exceeds HW supported count even if total used outputs does not
           exceed this HW limitation.This is a very weired behavior defined by spec !!! */
        if (pShader->shaderKind == VIR_SHADER_FRAGMENT && VIR_Symbol_GetLocation(pOutputSym) != -1)
        {
            /* It is safe to cast base linker helper to program linker helper due to gfx shader */
            pPgLinkHelper = (VSC_PROGRAM_LINKER_HELPER*)pBaseLinkHelper;

            if ((VIR_Symbol_GetLocation(pOutputSym) + thisOutputRegCount - 1) >=
                ((pPgLinkHelper->pgPassMnger.pPgmLinkerParam->pInMasterPEP != gcvNULL) ?
                 pBaseLinkHelper->pHwCfg->maxRenderTargetCount : pPgLinkHelper->pgPassMnger.pPgmLinkerParam->pGlApiCfg->maxDrawBuffers))
            {
                errCode = VSC_ERR_TOO_MANY_OUTPUTS;
                ON_ERROR(errCode, "Calc output ll slot");
            }
        }

        /* Mark this output used */
        VIR_Symbol_ClrFlag(pOutputSym, VIR_SYMFLAG_UNUSED);

        if (bSeperatedShaders)
        {
            location = VIR_Symbol_GetLocation(pOutputSym);
            if (location != -1)
            {
                VIR_Symbol_SetFirstSlot(pOutputSym, (gctUINT)location);

                for (i = (gctUINT)location;
                     i < (gctUINT)location + thisOutputRegCount;
                     i ++)
                {
                    gcmASSERT(i < MAX_SHADER_IO_NUM);

                    vscBV_SetBit(&outputWorkingMask, i);
                }
            }
            else
            {
                bHasNoAssignedLocation = gcvTRUE;
            }
        }
        else
        {
            VIR_Symbol_SetFirstSlot(pOutputSym, ioIdx);

            for (i = ioIdx;
                 i < ioIdx + thisOutputRegCount;
                 i ++)
            {
                gcmASSERT(i < MAX_SHADER_IO_NUM);

                vscBV_SetBit(&outputWorkingMask, i);
            }

            ioIdx += thisOutputRegCount;
        }
    }

    if (bSeperatedShaders && bHasNoAssignedLocation)
    {
        /* We hit here due to 2 reasons:
           1. App does not set locations for all app-level outputs
           2. For some internals or SVs, location can be missed
        */

        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

            if (VIR_Symbol_GetLocation(pOutputSym) == -1)
            {
                thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pOutputSym);

                ioIdx = vscBV_FindContinuousClearBitsForward(&outputWorkingMask, thisOutputRegCount, 0);

                if (ioIdx == INVALID_BIT_LOC)
                {
                    errCode = VSC_ERR_INVALID_DATA;
                    ON_ERROR(errCode, "Calc output ll slot");
                }

                VIR_Symbol_SetFirstSlot(pOutputSym, ioIdx);

                for (i = ioIdx;
                     i < ioIdx + thisOutputRegCount;
                     i ++)
                {
                    gcmASSERT(i < MAX_SHADER_IO_NUM);

                    vscBV_SetBit(&outputWorkingMask, i);
                }
            }
        }
    }

OnError:
    vscBV_Finalize(&outputWorkingMask);

    return errCode;
}

static VSC_ErrCode _CalcOutputLowLevelSlot(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                           VIR_Shader* pShader,
                                           gctBOOL bSeperatedShaders)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_OutputIdList           workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CalcOutputLowLevelSlotPerExeObj(pBaseLinkHelper, pShader,
                                               &workingPerVtxPxlAttrIdLst, bSeperatedShaders);
    ON_ERROR(errCode, "Calc output ll slot per vtx/pxl");

    /* Per prim */
    errCode = _CalcOutputLowLevelSlotPerExeObj(pBaseLinkHelper, pShader,
                                               &workingPerPrimAttrIdLst, bSeperatedShaders);
    ON_ERROR(errCode, "Calc output ll slot per prim");

OnError:
    return errCode;
}

static VSC_ErrCode _CheckInputAliasedLocation(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                              VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;
    VIR_IdList*                pAliasedArrayList = gcvNULL;
    VIR_IdList*                pComponentArrayList = gcvNULL;
    VIR_IdList*                pList = gcvNULL;
    gctUINT                    i;
    gctBOOL                    bCheckComponentMap = VIR_Shader_SupportIoCommponentMapping(pShader);
    gctBOOL                    bSupportAliasedAttribute = VIR_Shader_SupportAliasedAttribute(pShader);

    /* Check if we need to allocate the aliased attribute list. */
    if (bSupportAliasedAttribute)
    {
        VIR_Shader_CreateAttributeAliasList(pShader);
        pAliasedArrayList = VIR_Shader_GetAttributeAliasList(pShader);
        gcmASSERT(pAliasedArrayList);
    }
    if (bCheckComponentMap)
    {
        VIR_Shader_CreateAttributeComponentMapList(pShader);
        pComponentArrayList = VIR_Shader_GetAttributeComponentMapList(pShader);
        gcmASSERT(pComponentArrayList);
    }

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper, pShader, &workingPerVtxPxlAttrIdLst, gcvTRUE, pAliasedArrayList, pComponentArrayList);
    ON_ERROR(errCode, "Check io aliased location");

    /* Per prim */
    errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper, pShader, &workingPerPrimAttrIdLst, gcvTRUE, gcvNULL, gcvNULL);
    ON_ERROR(errCode, "Check io aliased location");

    /* Check if there is any aliased attribute. */
    if (pAliasedArrayList)
    {
        for (i = 0; i < MAX_SHADER_IO_NUM; i++)
        {
            pList = &pAliasedArrayList[i];
            if (VIR_IdList_Count(pList) > 1)
            {
                VIR_Shader_SetFlag(pShader, VIR_SHFLAG_HAS_ALIAS_ATTRIBUTE);
            }
        }
    }

    /* Check if there is any attribute component mapping. */
    if (bCheckComponentMap)
    {
        for (i = 0; i < MAX_SHADER_IO_NUM; i++)
        {
            pList = &pComponentArrayList[i];
            if (VIR_IdList_Count(pList) > 1)
            {
                VIR_Shader_BubbleSortSymIdList(pShader, pList, _CompareIoByComponent, VIR_IdList_Count(pList));
                VIR_Shader_SetFlagExt1(pShader, VIR_SHFLAG_EXT1_HAS_INPUT_COMP_MAP);
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CheckOutputAliasedLocation(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                               VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_OutputIdList           workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;
    VIR_IdList*                pComponentArrayList = gcvNULL;
    VIR_IdList*                pList = gcvNULL;
    gctUINT                    i;
    gctBOOL                    bCheckComponentMap = VIR_Shader_SupportIoCommponentMapping(pShader);

    if (bCheckComponentMap)
    {
        VIR_Shader_CreateOutputComponentMapList(pShader);
        pComponentArrayList = VIR_Shader_GetOutputComponentMapList(pShader);
        gcmASSERT(pComponentArrayList);
    }

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper, pShader, &workingPerVtxPxlAttrIdLst, gcvFALSE, gcvNULL, pComponentArrayList);
    ON_ERROR(errCode, "Check io aliased location");

    /* Per prim */
    errCode = _CheckIoAliasedLocationPerExeObj(pBaseLinkHelper, pShader, &workingPerPrimAttrIdLst, gcvFALSE, gcvNULL, gcvNULL);
    ON_ERROR(errCode, "Check io aliased location");

    /* Check if there is any output component mapping. */
    if (bCheckComponentMap)
    {
        for (i = 0; i < MAX_SHADER_IO_NUM; i++)
        {
            pList = &pComponentArrayList[i];
            if (VIR_IdList_Count(pList) > 1)
            {
                VIR_Shader_BubbleSortSymIdList(pShader, pList, _CompareIoByComponent, VIR_IdList_Count(pList));
                VIR_Shader_SetFlagExt1(pShader, VIR_SHFLAG_EXT1_HAS_OUTPUT_COMP_MAP);
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CheckUniformAliasedLocation(VSC_BASE_LINKER_HELPER* pBaseLinkHelper, VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_BIT_VECTOR             locationMask;
    VIR_UniformIdList*         uniformList = VIR_Shader_GetUniforms(pShader);
    VIR_Symbol*                uniformSym;
    gctUINT                    uniformIdx, logicalCount;
    gctUINT                    uniformCount = VIR_IdList_Count(uniformList);
    gctINT                     i, location;

    if (uniformCount == 0)
    {
        return VSC_ERR_NONE;
    }

    vscBV_Initialize(&locationMask, pBaseLinkHelper->pMM, GetGLMaxUniformLocations());

    for (uniformIdx = 0; uniformIdx < uniformCount; uniformIdx++)
    {
        uniformSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(uniformList, uniformIdx));

        if (VIR_Symbol_GetUniformKind(uniformSym) != VIR_UNIFORM_STRUCT)
        {
            continue;
        }

        location = VIR_Symbol_GetLocation(uniformSym);
        logicalCount = VIR_Shader_GetLogicalCount(pShader, VIR_Symbol_GetType(uniformSym));

        if (location != -1)
        {
            for (i = location; i < location + (gctINT)logicalCount; i ++)
            {
                if (vscBV_TestBit(&locationMask, i))
                {
                    errCode = VSC_ERR_LOCATION_ALIASED;
                    ON_ERROR(errCode, "Check uniform aliased location");
                }

                vscBV_SetBit(&locationMask, i);
            }
        }
    }

OnError:
    vscBV_Finalize(&locationMask);
    return errCode;
}

static VSC_ErrCode _LinkUniformBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                      VIR_Shader* pUpperShader,
                                                      VIR_Shader* pLowerShader,
                                                      VIR_Symbol* pLowerUniformSym,
                                                      VSC_BIT_VECTOR* uniformWorkingMask)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_UniformIdList*         upperUniformList = VIR_Shader_GetUniforms(pUpperShader);
    gctUINT                    upperUniformCount = VIR_IdList_Count(upperUniformList);
    gctUINT                    uniformIdx;
    VIR_Symbol*                upperUniformSym;
    gctBOOL                    bCheckPrecision = gcvTRUE;
    VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper = (VSC_PROGRAM_LINKER_HELPER*)pBaseLinkHelper;

    /* Skip uniform precision check for some APPs to make link pass. */
    if (pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.cFlags & VSC_COMPILER_FLAG_API_UNIFORM_PRECISION_CHECK)
    {
        bCheckPrecision = gcvFALSE;
    }

    for (uniformIdx = 0; uniformIdx < upperUniformCount; uniformIdx++)
    {
        gctBOOL                matched = gcvFALSE;
        VIR_UniformKind        upperUniformKind;
        upperUniformSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(upperUniformList, uniformIdx));
        upperUniformKind = VIR_Symbol_GetUniformKind(upperUniformSym);
        if (upperUniformKind != VIR_UNIFORM_NORMAL &&
            upperUniformKind != VIR_UNIFORM_STRUCT &&
            upperUniformKind != VIR_UNIFORM_BLOCK_MEMBER)
        {
            continue;
        }

        /* a struct member should be checked when checking the struct. */
        if (VIR_Symbol_HasFlag(upperUniformSym, VIR_SYMFLAG_IS_FIELD))
            continue;

        if (vscBV_TestBit(uniformWorkingMask, uniformIdx))
        {
            continue;
        }

        errCode = VIR_Uniform_Identical(pLowerShader,
                                        pLowerUniformSym,
                                        pUpperShader,
                                        upperUniformSym,
                                        bCheckPrecision && (upperUniformKind != VIR_UNIFORM_BLOCK_MEMBER),
                                        &matched);

        ON_ERROR(errCode, "Check uniform error");

        if (matched)
        {
            vscBV_SetBit(uniformWorkingMask, uniformIdx);
            break;
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _LinkUniformsBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                       VIR_Shader* pUpperShader,
                                                       VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_UniformIdList*         upperUniformList = VIR_Shader_GetUniforms(pUpperShader);
    VIR_UniformIdList*         lowerUniformList = VIR_Shader_GetUniforms(pLowerShader);
    gctUINT                    upperUniformCount = VIR_IdList_Count(upperUniformList);
    gctUINT                    lowerUniformCount = VIR_IdList_Count(lowerUniformList);
    gctUINT                    uniformIdx;
    VIR_Symbol*                lowerUniformSym;
    VSC_BIT_VECTOR             uniformWorkingMask;

    if (upperUniformCount == 0 || lowerUniformCount == 0)
    {
        return VSC_ERR_NONE;
    }

    vscBV_Initialize(&uniformWorkingMask, pBaseLinkHelper->pMM, upperUniformCount);

    for (uniformIdx = 0; uniformIdx < lowerUniformCount; uniformIdx++)
    {
        VIR_UniformKind        lowerUniformKind;
        lowerUniformSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(lowerUniformList, uniformIdx));
        lowerUniformKind = VIR_Symbol_GetUniformKind(lowerUniformSym);
        if (lowerUniformKind != VIR_UNIFORM_NORMAL &&
            lowerUniformKind != VIR_UNIFORM_STRUCT &&
            lowerUniformKind != VIR_UNIFORM_BLOCK_MEMBER)
        {
            continue;
        }

        /* a struct member should be checked when checking the struct. */
        if (VIR_Symbol_HasFlag(lowerUniformSym, VIR_SYMFLAG_IS_FIELD))
            continue;

        errCode = _LinkUniformBetweenTwoShaderStages(pBaseLinkHelper,
                                                     pUpperShader,
                                                     pLowerShader,
                                                     lowerUniformSym,
                                                     &uniformWorkingMask);
        ON_ERROR(errCode, "Link uniform \"%s\" between two shader stages",
                          VIR_Shader_GetSymNameString(pLowerShader, lowerUniformSym));
    }

OnError:
    vscBV_Finalize(&uniformWorkingMask);
    return errCode;
}

static VSC_ErrCode _LinkUboBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                  VIR_Shader* pUpperShader,
                                                  VIR_Shader* pLowerShader,
                                                  VIR_Symbol* pLowerUboSym,
                                                  VSC_BIT_VECTOR* uboWorkingMask)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_UBOIdList*             upperUBOList = VIR_Shader_GetUniformBlocks(pUpperShader);
    gctUINT                    upperUboCount = VIR_IdList_Count(upperUBOList);
    gctUINT                    uboIdx;
    VIR_Symbol*                upperUboSym;
    VIR_UniformBlock*          upperUbo;

    for (uboIdx = 0; uboIdx < upperUboCount; uboIdx++)
    {
        gctBOOL                matched = gcvFALSE;

        upperUboSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(upperUBOList, uboIdx));
        upperUbo = VIR_Symbol_GetUBO(upperUboSym);

        /* Skip push-constant. */
        if (VIR_UBO_IsPushConst(upperUbo))
        {
            continue;
        }

        if (vscBV_TestBit(uboWorkingMask, uboIdx))
        {
            continue;
        }

        errCode = VIR_UBO_Identical(pLowerShader,
                                    pLowerUboSym,
                                    pUpperShader,
                                    upperUboSym,
                                    &matched);

        ON_ERROR(errCode, "Check ubo error");

        if (matched)
        {
            vscBV_SetBit(uboWorkingMask, uboIdx);
            break;
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _LinkUbosBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                  VIR_Shader* pUpperShader,
                                                  VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_UBOIdList*             upperUBOList = VIR_Shader_GetUniformBlocks(pUpperShader);
    VIR_UBOIdList*             lowerUBOList = VIR_Shader_GetUniformBlocks(pLowerShader);
    gctUINT                    upperUBOCount = VIR_IdList_Count(upperUBOList);
    gctUINT                    lowerUBOCount = VIR_IdList_Count(lowerUBOList);
    gctUINT                    uboIdx;
    VIR_Symbol*                lowerUBOSym;
    VIR_UniformBlock*          lowerUBO;
    VSC_BIT_VECTOR             uboWorkingMask;

    if (upperUBOCount == 0 || lowerUBOCount == 0)
    {
        return VSC_ERR_NONE;
    }

    vscBV_Initialize(&uboWorkingMask, pBaseLinkHelper->pMM, upperUBOCount);

    for (uboIdx = 0; uboIdx < lowerUBOCount; uboIdx++)
    {
        lowerUBOSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(lowerUBOList, uboIdx));
        lowerUBO = VIR_Symbol_GetUBO(lowerUBOSym);

        /* Skip push-constant. */
        if (VIR_UBO_IsPushConst(lowerUBO))
        {
            continue;
        }

        if (VIR_Symbol_GetUBO(lowerUBOSym)->blockSize > GetGLMaxUniformBLockSize())
        {
            errCode = VSC_ERR_UNIFORMS_TOO_MANY;
            ON_ERROR(errCode, "The block size of ubo \"%s\" is too big.",
                              VIR_Shader_GetSymNameString(pLowerShader, lowerUBOSym));
        }

        errCode = _LinkUboBetweenTwoShaderStages(pBaseLinkHelper,
                                                 pUpperShader,
                                                 pLowerShader,
                                                 lowerUBOSym,
                                                 &uboWorkingMask);
        ON_ERROR(errCode, "Link ubo \"%s\" between two shader stages",
                          VIR_Shader_GetSymNameString(pLowerShader, lowerUBOSym));
    }

OnError:
    vscBV_Finalize(&uboWorkingMask);
    return errCode;
}

static void _CountIOBlockList(VIR_Shader* pShader, VSC_MM* pMM, gctBOOL bInput,
                              VIR_IdList* pIOBlockList)
{
    gctUINT                    idx;
    VIR_IdList*                pOrgIOBlockList;
    VIR_Symbol*                pSym;
    VIR_StorageClass           storage;
    VIR_IOBlock*               ioBlock;

    pOrgIOBlockList = VIR_Shader_GetIOBlocks(pShader);
    VIR_IdList_Init(pMM, MAX_SHADER_IO_NUM, &pIOBlockList);

    for (idx = 0; idx < VIR_IdList_Count(pOrgIOBlockList); idx ++)
    {
        pSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOrgIOBlockList, idx));
        ioBlock = VIR_Symbol_GetIOB(pSym);
        gcmASSERT(ioBlock);

        storage =  VIR_IOBLOCK_GetStorage(ioBlock);

        if (((storage == VIR_STORAGE_INPUT || storage == VIR_STORAGE_PERPATCH_INPUT) && !bInput)
            ||
            ((storage == VIR_STORAGE_OUTPUT || storage == VIR_STORAGE_PERPATCH_OUTPUT) && bInput))
        {
            continue;
        }

        VIR_IdList_Add(pIOBlockList, VIR_IdList_GetId(pOrgIOBlockList, idx));
    }
}

static gctBOOL _IsBlockBuiltin(VIR_Shader* pShader,
                               VIR_Symbol* pBlock,
                               VIR_Type*   pType)
{
    if (VIR_Shader_IsNameBuiltIn(pShader, VIR_Symbol_GetName(pBlock)))
    {
        return gcvTRUE;
    }
    else
    {
        VIR_Type*   type = VIR_Symbol_GetType(pBlock);

        while (VIR_Type_isArray(type))
        {
            type = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(type));
        }

        if (VIR_Type_isStruct(type))
        {
            VIR_SymIdList*  fields = VIR_Type_GetFields(type);
            gctUINT         i;

            for (i = 0; i < VIR_IdList_Count(fields); i++)
            {
                VIR_Id      id = VIR_IdList_GetId(VIR_Type_GetFields(type), i);
                VIR_Symbol* fieldSym = VIR_Shader_GetSymFromId(pShader, id);

                /* Any field is builtin, then this block is builtin. */
                if (_IsBlockBuiltin(pShader, fieldSym, VIR_Symbol_GetType(fieldSym)))
                {
                    return gcvTRUE;
                }
            }
        }
    }

    return gcvFALSE;
}

static VSC_ErrCode _LinkIOBlockBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                      VIR_Shader* pUpperShader,
                                                      VIR_Shader* pLowerShader,
                                                      VIR_IOBIdList* pOutputIdListOfUpperShader,
                                                      VIR_IOBIdList* pInputIdListsOfLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    inputIdx, outputIdx;
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdListOfUpperShader);
    gctUINT                    inputCount = VIR_IdList_Count(pInputIdListsOfLowerShader);
    VIR_Symbol*                pInputSym;
    VIR_Symbol*                pOutputSym;
    VIR_Type*                  pInputType;
    VIR_Type*                  pOutputType;
    VSC_BIT_VECTOR             outputWorkingMask;
    gctBOOL                    bCheckInterpolation = _NeedCheckInterpolation(pUpperShader);

    if (inputCount == 0)
    {
        return VSC_ERR_NONE;
    }

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM, outputCount);

    for (inputIdx = 0; inputIdx < inputCount; inputIdx ++)
    {
        pInputSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pInputIdListsOfLowerShader, inputIdx));
        pInputType = VIR_Symbol_GetType(pInputSym);

        /* skip built-in IO blocks. */
        if (_IsBlockBuiltin(pLowerShader, pInputSym, pInputType))
        {
            continue;
        }

        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            if (vscBV_TestBit(&outputWorkingMask, outputIdx))
            {
                continue;
            }

            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdListOfUpperShader, outputIdx));
            pOutputType = VIR_Symbol_GetType(pOutputSym);

            /* If flag SKIP-NAME-CHECK is present, check location only. */
            if (isSymSkipNameCheck(pInputSym))
            {
                /* When a symbol has SKIP-NAME-CHECK flag, its location must be NOT -1. */
                gcmASSERT(VIR_Symbol_GetLocation(pInputSym) != -1);
                if (VIR_Symbol_GetLocation(pInputSym) != VIR_Symbol_GetLocation(pOutputSym))
                {
                    continue;
                }
            }
            /* Name or location of the output and attribute must be matched */
            else if (!(VIR_Symbol_isNameMatch(pLowerShader, pInputSym, pUpperShader, pOutputSym) ||
                     (VIR_Symbol_GetLocation(pOutputSym) != -1 &&
                     VIR_Symbol_GetLocation(pOutputSym) == VIR_Symbol_GetLocation(pInputSym))))
            {
                continue;
            }

            /* Check Location. */
            if (VIR_Symbol_GetLocation(pOutputSym) != VIR_Symbol_GetLocation(pInputSym))
            {
                errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                ON_ERROR(errCode, "Link IO blocks between two shader stages");
            }

            if (bCheckInterpolation)
            {
                if ((isSymFlat(pInputSym) != isSymFlat(pOutputSym)) || (isSymNoperspective(pInputSym) != isSymNoperspective(pOutputSym)))
                {
                    errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                    ON_ERROR(errCode, "Link IO blocks between two shader stages");
                }
            }

            /* Check its members. */
            if (!VIR_Type_Identical(pLowerShader, pInputType, pUpperShader, pOutputType))
            {
                errCode = VSC_ERR_VARYING_TYPE_MISMATCH;
                ON_ERROR(errCode, "Link IO blocks between two shader stages");
            }

            vscBV_SetBit(&outputWorkingMask, outputIdx);
            break;
        }

        if (outputIdx == outputCount)
        {
            /* No matching vertex output found. */
            errCode = VSC_ERR_UNDECLARED_VARYING;
            ON_ERROR(errCode, "Link IO blocks between two shader stages");
        }
    }

OnError:
    vscBV_Finalize(&outputWorkingMask);
    return errCode;
}

static VSC_ErrCode _LinkIOBBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                  VIR_Shader* pUpperShader,
                                                  VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_IOBIdList              outputBlockOfUpperShader;
    VIR_IOBIdList              intputBlockOfLowerShader;

    _CountIOBlockList(pUpperShader, pBaseLinkHelper->pMM,
                      gcvFALSE, &outputBlockOfUpperShader);

    _CountIOBlockList(pLowerShader, pBaseLinkHelper->pMM,
                      gcvTRUE, &intputBlockOfLowerShader);

    errCode = _LinkIOBlockBetweenTwoShaderStages(pBaseLinkHelper, pUpperShader, pLowerShader,
                                                 &outputBlockOfUpperShader, &intputBlockOfLowerShader);
    ON_ERROR(errCode, "Link IBO between two shader stages per vtx/pxl");

OnError:
    return errCode;
}

static VSC_ErrCode _LinkIoAmongShaderStages(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper, FSL_STAGE fslStage)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Shader*  pPreStage = gcvNULL;
    VIR_Shader*  pCurStage;
    gctUINT      stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            if (pPreStage)
            {
                /* Link two shader stages by I-O */
                errCode = _LinkIoBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                        fslStage,
                                                        pPreStage,
                                                        pCurStage);
                ON_ERROR(errCode, "Link Io between two shader stages");
            }
            else
            {
                /* Only consider input of first active stage */

                if (fslStage == FSL_STAGE_API_SPEC_CHECK)
                {
                    errCode = _CheckInputAliasedLocation(&pPgLinkHelper->baseHelper,
                                                         pCurStage);
                    ON_ERROR(errCode, "Check input aliased location");
                }
                else if (fslStage == FSL_STAGE_LL_SLOT_CALC)
                {
                    errCode = _CalcInputLowLevelSlot(&pPgLinkHelper->baseHelper,
                                                     pCurStage,
                                                     gcvFALSE);
                    ON_ERROR(errCode, "Calc LL slot of input");
                }
            }

            pPreStage = pCurStage;
        }
    }

    if (pPreStage)
    {
        /* Only consider output of last active stage */

        if (fslStage == FSL_STAGE_API_SPEC_CHECK)
        {
            errCode = _CheckOutputAliasedLocation(&pPgLinkHelper->baseHelper, pPreStage);
            ON_ERROR(errCode, "Check output aliased location");
        }
        else if (fslStage == FSL_STAGE_LL_SLOT_CALC)
        {
            if (pPreStage->shaderKind == VIR_SHADER_FRAGMENT)
            {
                /* Because 0x0401 is programed in VSC, so we need
                   force llFirstSlot same as location. But it is unsafe if spec allows location
                   of PS be changable. So it is better to move 0x0401
                   programming out of VSC at least. */
                errCode = _CalcOutputLowLevelSlot(&pPgLinkHelper->baseHelper, pPreStage, gcvTRUE);
            }
            else
            {
                errCode = _CalcOutputLowLevelSlot(&pPgLinkHelper->baseHelper, pPreStage, gcvFALSE);
            }
            ON_ERROR(errCode, "Calc LL slot of output");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _LinkUniformAmongShaderStages(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    gctUINT      stageIdx, stageIdx1;
    VIR_Shader*  pCurStage;
    VIR_Shader*  pTempStage;
    gctBOOL      bNeedLinkageValidate = gcvTRUE;

    /* We don't need to do the linkage validation for a vulkan program because spec doesn't require this. */
    if (pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENVK)
    {
        bNeedLinkageValidate = gcvFALSE;
    }

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            errCode = _CheckUniformAliasedLocation(&pPgLinkHelper->baseHelper,
                                                   pCurStage);
            ON_ERROR(errCode, "Link Uniforms between two shader stages");

            /* Skip the linkage validation. */
            if (!bNeedLinkageValidate)
            {
                continue;
            }

            for (stageIdx1 = 0; stageIdx1 < stageIdx; stageIdx1 ++)
            {
                pTempStage = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx1];

                if (pTempStage)
                {
                    errCode = _LinkUniformsBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                                  pTempStage,
                                                                  pCurStage);
                    ON_ERROR(errCode, "Link Uniforms between two shader stages");

                    errCode = _LinkUbosBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                             pTempStage,
                                                             pCurStage);
                    ON_ERROR(errCode, "Link Uniform blocks between two shader stages");
                }
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _LinkIobAmongShaderStages(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Shader*  pPreStage = gcvNULL;
    VIR_Shader*  pCurStage;
    gctUINT      stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage= (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            if (pPreStage)
            {
                errCode = _LinkIOBBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                         pPreStage,
                                                         pCurStage);
                ON_ERROR(errCode, "Link IOB between two shader stages");
            }

            pPreStage = pCurStage;
        }
    }

OnError:
    return errCode;
}

typedef struct ATTR_OUTPUT_PAIR
{
    /* For pure SO, pOutputSym is not NULL, but pAttrSym is NULL.
       For a break-pair, pAttrSym and pOutputSym are both NULL. */

    VIR_Symbol*        pAttrSym;
    VIR_Shader*        pAttrSymShader;

    VIR_Symbol*        pOutputSym;
    VIR_Shader*        pOutputSymShader;
}ATTR_OUTPUT_PAIR;

static gctBOOL _CanIoPairVectorizedToIoPackets(ATTR_OUTPUT_PAIR* pIoPair,
                                               VIR_IO_VECTORIZABLE_PACKET* pAVP,
                                               VIR_IO_VECTORIZABLE_PACKET* pOVP)
{
    gctUINT                     i, accumChannelCount = 0, compCount;
    gctBOOL                     bIsAVPValid = (pAVP && pAVP->realCount != 0);

    gcmASSERT(pOVP != gcvNULL);
    gcmASSERT(!bIsAVPValid || pAVP->realCount == pOVP->realCount);

    /* For a break-pair, just return FALSE */
    if (pIoPair->pAttrSym == gcvNULL && pIoPair->pOutputSym == gcvNULL)
    {
        return gcvFALSE;
    }

    gcmASSERT(pIoPair->pOutputSym);

    /* If io-packets have had syms, that attri-packets is valid but io-pair has no attri or inverse is not permitted */
    if ((pOVP->realCount > 0) && bIsAVPValid != (pIoPair->pAttrSym != gcvNULL))
    {
        return gcvFALSE;
    }

    /* Get component count io-packets already hold */
    for (i = 0; i < pOVP->realCount; i ++)
    {
        if (bIsAVPValid)
        {
            gcmASSERT(VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pAVP->pSymIo[i]))) ==
                      VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pOVP->pSymIo[i]))));
        }

        accumChannelCount += VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pOVP->pSymIo[i])));
    }

    /* Total component count can not be GT 4 */
    compCount = VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pIoPair->pOutputSym)));
    if ((accumChannelCount + compCount) > CHANNEL_NUM)
    {
        return gcvFALSE;
    }

    /* Check current output sym is matched to syms that are already in output packet */
    if (pOVP->realCount > 0)
    {
        if (!vscVIR_CheckTwoSymsVectorizability(pIoPair->pOutputSymShader, pIoPair->pOutputSym, pOVP->pSymIo[pOVP->realCount-1]))
        {
            return gcvFALSE;
        }
    }

    /* Check current attri sym is matched to syms that are already in attri packet */
    if (bIsAVPValid && pAVP->realCount)
    {
        if (!vscVIR_CheckTwoSymsVectorizability(pIoPair->pAttrSymShader, pIoPair->pAttrSym, pAVP->pSymIo[pAVP->realCount-1]))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static void _UpdateIoPacketWithOneIoSym(VIR_IO_VECTORIZABLE_PACKET* pIoPacket,
                                        VIR_Symbol* pIoSym)
{
    pIoPacket->pSymIo[pIoPacket->realCount] = pIoSym;
    pIoPacket->realCount ++;

    if (pIoPacket->vectorizedLocation == NOT_ASSIGNED &&
        VIR_Symbol_GetLocation(pIoSym) != -1)
    {
        pIoPacket->vectorizedLocation = VIR_Symbol_GetLocation(pIoSym);
    }
}

static void _CollectVectorizableIoPacketsFromSoPairs(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                     ATTR_OUTPUT_PAIR* pSoIoPairArray,
                                                     gctUINT soPairArraySize,
                                                     VIR_IO_VECTORIZABLE_PACKET* pAVPArray,
                                                     gctUINT* pAvpArraySize,
                                                     VIR_IO_VECTORIZABLE_PACKET* pOVPArray,
                                                     gctUINT* pOvpArraySize)
{
    gctUINT                     i;
    gctBOOL                     bHasAttrPaired = gcvFALSE;
    gctBOOL                     bCanVectorized;

    gcmASSERT(pOVPArray);

    for (i = 0; i < soPairArraySize; i ++)
    {
        bCanVectorized = _CanIoPairVectorizedToIoPackets(&pSoIoPairArray[i],
                                                         pAVPArray ? &pAVPArray[*pAvpArraySize] : gcvNULL,
                                                         &pOVPArray[*pOvpArraySize]);

        /* If the io-pair can not be added to current io-packets, then try to finish current io-packets */
        if (!bCanVectorized)
        {
            if (pOVPArray[*pOvpArraySize].realCount > 1)
            {
                if (bHasAttrPaired)
                {
                    gcmASSERT(pAVPArray[*pAvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);
                    gcmASSERT(pOVPArray[*pOvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);
                    gcmASSERT(pOVPArray[*pOvpArraySize].realCount == pAVPArray[*pAvpArraySize].realCount);

                    (*pAvpArraySize) ++;
                }

                (*pOvpArraySize) ++;
            }
            else if (pOVPArray[*pOvpArraySize].realCount == 1)
            {
                /* Reset io-packet */

                memset(&pOVPArray[*pOvpArraySize], 0, sizeof(VIR_IO_VECTORIZABLE_PACKET));
                pOVPArray[*pOvpArraySize].vectorizedLocation = NOT_ASSIGNED;

                if (bHasAttrPaired)
                {
                    gcmASSERT(pAVPArray[*pAvpArraySize].realCount == 1);

                    memset(&pAVPArray[*pAvpArraySize], 0, sizeof(VIR_IO_VECTORIZABLE_PACKET));
                    pAVPArray[*pAvpArraySize].vectorizedLocation = NOT_ASSIGNED;
                }
                else if (pAVPArray)
                {
                    gcmASSERT(pAVPArray[*pAvpArraySize].realCount == 0);
                }
            }
        }

        /* Add current valid io-pair to io-packets */
        if (pSoIoPairArray[i].pOutputSym)
        {
            if (pOVPArray[*pOvpArraySize].realCount == 0)
            {
                bHasAttrPaired = (pSoIoPairArray[i].pAttrSym != gcvNULL);

                pOVPArray[*pOvpArraySize].bOutput = gcvTRUE;
                pOVPArray[*pOvpArraySize].bTFB = gcvTRUE;
            }

            _UpdateIoPacketWithOneIoSym(&pOVPArray[*pOvpArraySize], pSoIoPairArray[i].pOutputSym);

            if (bHasAttrPaired)
            {
                _UpdateIoPacketWithOneIoSym(&pAVPArray[*pAvpArraySize], pSoIoPairArray[i].pAttrSym);
            }
        }
    }

    /* We need finish last io-packets */
    if (pOVPArray[*pOvpArraySize].realCount > 1)
    {
        if (bHasAttrPaired)
        {
            gcmASSERT(pAVPArray[*pAvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);
            gcmASSERT(pOVPArray[*pOvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);
            gcmASSERT(pOVPArray[*pOvpArraySize].realCount == pAVPArray[*pAvpArraySize].realCount);

            (*pAvpArraySize) ++;
        }

        (*pOvpArraySize) ++;
    }
}

static void _CollectVectorizableIoPacketsFromNormalPairs(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                         ATTR_OUTPUT_PAIR** ppNormalIoPairArray,
                                                         gctUINT* pNormalPairArraySize,
                                                         VIR_IO_VECTORIZABLE_PACKET* pAVPArray,
                                                         gctUINT* pAvpArraySize,
                                                         VIR_IO_VECTORIZABLE_PACKET* pOVPArray,
                                                         gctUINT* pOvpArraySize)
{
    gctINT                      i, j, k, l, m;
    gctUINT                     accumChannelCount;
    VSC_BIT_VECTOR              globalChannelsPairMask[3] = {{0}};
    VSC_BIT_VECTOR              localChannelsPairMask[3] = {{0}};
    gctBOOL                     bCanVectorized;
    gctBOOL                     bEarlyOut;

    gcmASSERT(pAVPArray && pOVPArray);

    for (i = 0; i < 3; i ++)
    {
        if (pNormalPairArraySize[i])
        {
            vscBV_Initialize(&globalChannelsPairMask[i], pBaseLinkHelper->pMM, pNormalPairArraySize[i]);
            vscBV_Initialize(&localChannelsPairMask[i], pBaseLinkHelper->pMM, pNormalPairArraySize[i]);
        }
    }

    /* Because more small channel count syms are more prone to be vectorizable easily,
       the seq picking syms is 'more bigger channel count, more earlier' */
    for (i = 2; i >= 0; i --)
    {
        for (j = 0; j < (gctINT)pNormalPairArraySize[i]; j ++)
        {
            /* If it has been put to io-packet, just skip */
            if (vscBV_TestBit(&globalChannelsPairMask[i], j))
            {
                continue;
            }

            /* Clear all local pair mask */
            for (m = 0; m < 3; m ++)
            {
                if (pNormalPairArraySize[m])
                {
                    vscBV_ClearAll(&localChannelsPairMask[m]);
                }
            }

            _UpdateIoPacketWithOneIoSym(&pAVPArray[*pAvpArraySize], ppNormalIoPairArray[i][j].pAttrSym);

            pOVPArray[*pOvpArraySize].bOutput = gcvTRUE;
            _UpdateIoPacketWithOneIoSym(&pOVPArray[*pOvpArraySize], ppNormalIoPairArray[i][j].pOutputSym);

            vscBV_SetBit(&localChannelsPairMask[i], j);

            /* Greedy searching to find candidates */
            for (l = i; l >= 0; l --)
            {
                /* Sym with 3 channels can only be vectorized with symbols with 1 channel */
                if (i == 2 && (l == 2 || l == 1))
                {
                    continue;
                }

                bEarlyOut = gcvFALSE;
                for (k = 0; k < (gctINT)pNormalPairArraySize[l]; k ++)
                {
                    /* Skip self */
                    if (l == i && k == j)
                    {
                        continue;
                    }

                    /* If it has been put to io-packet, just skip */
                    if (vscBV_TestBit(&globalChannelsPairMask[l], k))
                    {
                        continue;
                    }

                    bCanVectorized = _CanIoPairVectorizedToIoPackets(&ppNormalIoPairArray[l][k],
                                                                     &pAVPArray[*pAvpArraySize],
                                                                     &pOVPArray[*pOvpArraySize]);

                    if (bCanVectorized)
                    {
                        _UpdateIoPacketWithOneIoSym(&pAVPArray[*pAvpArraySize], ppNormalIoPairArray[l][k].pAttrSym);
                        _UpdateIoPacketWithOneIoSym(&pOVPArray[*pOvpArraySize], ppNormalIoPairArray[l][k].pOutputSym);

                        vscBV_SetBit(&localChannelsPairMask[l], k);

                        accumChannelCount = 0;
                        for (m = 0; m < (gctINT)pAVPArray[*pAvpArraySize].realCount; m ++)
                        {
                            accumChannelCount +=
                                VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pAVPArray[*pAvpArraySize].pSymIo[m])));
                        }

                        /* Has no more room, just early out */
                        if (accumChannelCount == CHANNEL_NUM)
                        {
                            bEarlyOut = gcvTRUE;
                            break;
                        }
                    }
                }

                if (bEarlyOut)
                {
                    break;
                }
            }

            gcmASSERT(pAVPArray[*pAvpArraySize].realCount == pOVPArray[*pOvpArraySize].realCount);
            gcmASSERT(pAVPArray[*pAvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);
            gcmASSERT(pOVPArray[*pOvpArraySize].realCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);

            /* If it is a valid io-packet, just receive it */
            if (pAVPArray[*pAvpArraySize].realCount > 1)
            {
                (*pAvpArraySize) ++;
                (*pOvpArraySize) ++;

                for (m = 0; m < 3; m ++)
                {
                    if (pNormalPairArraySize[m])
                    {
                        vscBV_Or1(&globalChannelsPairMask[m], &localChannelsPairMask[m]);
                    }
                }
            }
            else
            {
                /* Reset io-packet */

                memset(&pAVPArray[*pAvpArraySize], 0, sizeof(VIR_IO_VECTORIZABLE_PACKET));
                pAVPArray[*pAvpArraySize].vectorizedLocation = NOT_ASSIGNED;

                memset(&pOVPArray[*pOvpArraySize], 0, sizeof(VIR_IO_VECTORIZABLE_PACKET));
                pOVPArray[*pOvpArraySize].vectorizedLocation = NOT_ASSIGNED;
            }
        }
    }

    for (i = 0; i < 3; i ++)
    {
        if (pNormalPairArraySize[i])
        {
            vscBV_Finalize(&globalChannelsPairMask[i]);
            vscBV_Finalize(&localChannelsPairMask[i]);
        }
    }
}

static void _CollectVectorizableIoPairs(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                        VIR_Shader* pUpperShader,
                                        VIR_Shader* pLowerShader,
                                        VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                        VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                        ATTR_OUTPUT_PAIR* pSoIoPairArray,
                                        gctUINT* pSoPairArraySize,
                                        ATTR_OUTPUT_PAIR** ppNormalIoPairArray,
                                        gctUINT* pNormalPairArraySize)
{
    gctUINT                     attrIdx, outputIdx, i, j;
    gctUINT                     attrCount = VIR_IdList_Count(pAttrIdLstsOfLowerShader);
    gctUINT                     outputCount = VIR_IdList_Count(pOutputIdLstsOfUpperShader);
    gctUINT                     soOutputCount = 0, compCount;
    VIR_Symbol*                 pAttrSym;
    VIR_Symbol*                 pOutputSym;
    VIR_Symbol*                 pSoSym;
    VSC_BIT_VECTOR              outputWorkingMask;

    /* We only consider following IO-vectorization
       1. One-reg'ed fully SO'ed outputs and their couterpart inputs. For other SO outputs, dont consider them.
       2. Normal user defined IOs. For SVs, dont consider them.
    */

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM, outputCount);

    /* Collect SO related pairs */
    if (pUpperShader->transformFeedback.varyings)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);

        soOutputCount = VIR_IdList_Count(pUpperShader->transformFeedback.varyings);

        if (soOutputCount > 0)
        {
            for (outputIdx = 0; outputIdx < soOutputCount; outputIdx ++)
            {
                VIR_Type *pOutputSymType = gcvNULL;
                pSoSym = VIR_Shader_GetSymFromId(pUpperShader,
                                    VIR_IdList_GetId(pUpperShader->transformFeedback.varyings, outputIdx));

                pOutputSym = VIR_Symbol_GetIndexingInfo(pUpperShader, pSoSym).underlyingSym;
                pOutputSymType = VIR_Symbol_GetType(pOutputSym);

                gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

                pSoIoPairArray[*pSoPairArraySize].pAttrSym = gcvNULL;

                if ((VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym)) &&
                     !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym, gcvTRUE)) ||
                     pSoSym != pOutputSym ||
                     /*TODO: if output is matrix and used in lower shader, the layout is different */
                     (VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym) > 1 &&
                      VIR_Type_isMatrix(pOutputSymType) && VIR_IdList_Count(pAttrIdLstsOfLowerShader) > 0))
                {
                    /* For SIV/partial SO/multi-regs SO, add break-pair */
                    pSoIoPairArray[*pSoPairArraySize].pOutputSym = gcvNULL;
                }
                else
                {
                    /* For normal full output SO */
                    compCount = VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pOutputSym)));
                    gcmASSERT(compCount <= CHANNEL_NUM && compCount > 0);
                    pSoIoPairArray[*pSoPairArraySize].pOutputSym = compCount < CHANNEL_NUM ? pOutputSym : gcvNULL;
                    pSoIoPairArray[*pSoPairArraySize].pOutputSymShader = (pSoIoPairArray[*pSoPairArraySize].pOutputSym != gcvNULL) ?
                                                                         pUpperShader : gcvNULL;
                }

                (*pSoPairArraySize) ++;

                /* For separated SO mode, we always add a break-pair for each SO output, so later vectorizable-packet
                   collection can auto discard all SO outputs to vectorize */
                if (pUpperShader->transformFeedback.bufferMode == VIR_FEEDBACK_SEPARATE)
                {
                    pSoIoPairArray[*pSoPairArraySize].pAttrSym = gcvNULL;
                    pSoIoPairArray[*pSoPairArraySize].pOutputSym = gcvNULL;
                    (*pSoPairArraySize) ++;
                }
            }
        }
    }

    /* Collect normal pairs */
    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

        /* Only consider used one */
        if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
        {
            continue;
        }

        /* Don't consider SGV */
        if (VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym)) &&
            !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym), gcvTRUE))
        {
            continue;
        }

        /* Do not vectorize those inputs with the prefix "gl_in." */
        if (gcoOS_StrStr(VIR_Shader_GetSymNameString(pLowerShader, pAttrSym), "gl_in.", gcvNULL))
        {
            continue;
        }

        /* For each output of upper-shader */
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            if (vscBV_TestBit(&outputWorkingMask, outputIdx))
            {
                continue;
            }

            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            /* If flag SKIP-NAME-CHECK is present, check location only. */
            if (isSymSkipNameCheck(pAttrSym))
            {
                /* When a symbol has SKIP-NAME-CHECK flag, its location must be NOT -1. */
                gcmASSERT(VIR_Symbol_GetLocation(pAttrSym) != -1);
                if (VIR_Symbol_GetLocation(pAttrSym) != VIR_Symbol_GetLocation(pOutputSym))
                {
                    continue;
                }
            }
            /* Name or location of the output and attribute must be matched */
            else if (!(VIR_Symbol_isNameMatch(pLowerShader, pAttrSym, pUpperShader, pOutputSym) ||
                     (VIR_Symbol_GetLocation(pOutputSym) != -1 &&
                      VIR_Symbol_GetLocation(pOutputSym) == VIR_Symbol_GetLocation(pAttrSym))))
            {
                continue;
            }

            /* If any of them has the component qualifier, they must be matched. */
            if (VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pAttrSym)) != VIR_Layout_GetComponent(VIR_Symbol_GetLayout(pOutputSym)))
            {
                continue;
            }

            gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

            /* Check whether this output is in so-pair, if so, put the attr to that pair who has included the output */
            for (i = 0; i < *pSoPairArraySize; i ++)
            {
                if (pSoIoPairArray[i].pOutputSym == pOutputSym)
                {
                    pSoIoPairArray[i].pAttrSym = pAttrSym;
                    pSoIoPairArray[i].pAttrSymShader = pLowerShader;
                    break;
                }
            }

            /* If not in so-pair, check whether it is a SO output, if so, just skip it */
            j = soOutputCount = 0;
            if (i == *pSoPairArraySize && pUpperShader->transformFeedback.varyings
                && VIR_IdList_Count(pUpperShader->transformFeedback.varyings) > 0)
            {
                soOutputCount = VIR_IdList_Count(pUpperShader->transformFeedback.varyings);
                for (j = 0; j < soOutputCount; j ++)
                {
                    pSoSym = VIR_Shader_GetSymFromId(pUpperShader,
                                        VIR_IdList_GetId(pUpperShader->transformFeedback.varyings, j));

                    if (VIR_Symbol_GetIndexingInfo(pUpperShader, pSoSym).underlyingSym == pOutputSym)
                    {
                        break;
                    }
                }
            }

            /* Create new normal pair if it is not related to SO */
            if (i == *pSoPairArraySize && j == soOutputCount)
            {
                compCount = VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pOutputSym)));
                /* For Vulkan, the input and the output can have the different component count. */
                gcmASSERT((gctINT)compCount >= VIR_GetTypeComponents(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pAttrSym))));
                gcmASSERT(compCount <= CHANNEL_NUM && compCount > 0);

                if (compCount < CHANNEL_NUM)
                {
                    ppNormalIoPairArray[compCount - 1][pNormalPairArraySize[compCount - 1]].pAttrSym = pAttrSym;
                    ppNormalIoPairArray[compCount - 1][pNormalPairArraySize[compCount - 1]].pOutputSym = pOutputSym;
                    ppNormalIoPairArray[compCount - 1][pNormalPairArraySize[compCount - 1]].pAttrSymShader = pLowerShader;
                    ppNormalIoPairArray[compCount - 1][pNormalPairArraySize[compCount - 1]].pOutputSymShader = pUpperShader;
                    pNormalPairArraySize[compCount - 1] ++;
                }
            }

            vscBV_SetBit(&outputWorkingMask, outputIdx);

            break;
        }

        gcmASSERT(outputIdx < outputCount);
    }

    vscBV_Finalize(&outputWorkingMask);
}

static VSC_ErrCode _FindVectorizableIoPackets(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                              VIR_Shader* pUpperShader,
                                              VIR_Shader* pLowerShader,
                                              VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                              VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                              VIR_IO_VECTORIZABLE_PACKET** ppAttrVectorizablePackets,
                                              VIR_IO_VECTORIZABLE_PACKET** ppOutputVectorizablePackets,
                                              gctUINT* pNumOfInputPackets,
                                              gctUINT* pNumOfOutputPackets)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    ATTR_OUTPUT_PAIR*           pNormalIoPairArray[3]; /* Only IOs that hold 1/2/3 channels are considered, so size 3 is enough */
    ATTR_OUTPUT_PAIR*           pSoIoPairArray;
    gctUINT                     normalPairArraySize[3] = {0, 0, 0};
    gctUINT                     i, soPairArraySize = 0;
    VIR_IO_VECTORIZABLE_PACKET* pAVPArray = gcvNULL;
    VIR_IO_VECTORIZABLE_PACKET* pOVPArray = gcvNULL;
    gctUINT                     avpArraySize = 0, ovpArraySize = 0;

    /* Initialize */
    for (i = 0; i < 3; i ++)
    {
        pNormalIoPairArray[i] = (ATTR_OUTPUT_PAIR*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                               MAX_SHADER_IO_NUM * sizeof(ATTR_OUTPUT_PAIR));
    }
    pSoIoPairArray = (ATTR_OUTPUT_PAIR*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                    2 * MAX_SHADER_IO_NUM * sizeof(ATTR_OUTPUT_PAIR));

    /* Collect io-pairs */
    _CollectVectorizableIoPairs(pBaseLinkHelper,
                                pUpperShader,
                                pLowerShader,
                                pAttrIdLstsOfLowerShader,
                                pOutputIdLstsOfUpperShader,
                                pSoIoPairArray,
                                &soPairArraySize,
                                pNormalIoPairArray,
                                normalPairArraySize);

    /* Collect vectorizable-packet from pairs */
    if (normalPairArraySize[0] > 0 ||
        normalPairArraySize[1] > 0 ||
        normalPairArraySize[2] > 0 ||
        soPairArraySize > 0)
    {
        if (VIR_IdList_Count(pAttrIdLstsOfLowerShader) > 0)
        {
            pAVPArray = (VIR_IO_VECTORIZABLE_PACKET*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                 MAX_SHADER_IO_NUM * sizeof(VIR_IO_VECTORIZABLE_PACKET));
            memset(pAVPArray, 0, MAX_SHADER_IO_NUM * sizeof(VIR_IO_VECTORIZABLE_PACKET));
            for (i = 0; i < MAX_SHADER_IO_NUM; i ++)
            {
                pAVPArray[i].vectorizedLocation = NOT_ASSIGNED;
            }
        }

        if (VIR_IdList_Count(pOutputIdLstsOfUpperShader) > 0)
        {
            pOVPArray = (VIR_IO_VECTORIZABLE_PACKET*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                 MAX_SHADER_IO_NUM * sizeof(VIR_IO_VECTORIZABLE_PACKET));
            memset(pOVPArray, 0, MAX_SHADER_IO_NUM * sizeof(VIR_IO_VECTORIZABLE_PACKET));
            for (i = 0; i < MAX_SHADER_IO_NUM; i ++)
            {
                pOVPArray[i].vectorizedLocation = NOT_ASSIGNED;
            }
        }
    }

    if (soPairArraySize > 0)
    {
        /* Consider SO pairs. NOTE that all these pairs must be in order which is the requirement of xfb */
        _CollectVectorizableIoPacketsFromSoPairs(pBaseLinkHelper,
                                                 pSoIoPairArray,
                                                 soPairArraySize,
                                                 pAVPArray,
                                                 &avpArraySize,
                                                 pOVPArray,
                                                 &ovpArraySize);
    }

    if (normalPairArraySize[0] > 0 ||
        normalPairArraySize[1] > 0 ||
        normalPairArraySize[2] > 0)
    {
        /* Consider normal pairs. NOTE they can be out of order to get better vectorizability */
        _CollectVectorizableIoPacketsFromNormalPairs(pBaseLinkHelper,
                                                     pNormalIoPairArray,
                                                     normalPairArraySize,
                                                     pAVPArray,
                                                     &avpArraySize,
                                                     pOVPArray,
                                                     &ovpArraySize);
    }

    /* Finalization */
    for (i = 0; i < 3; i ++)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pNormalIoPairArray[i]);
    }
    vscMM_Free(pBaseLinkHelper->pMM, pSoIoPairArray);

    if (pAVPArray)
    {
        if (avpArraySize == 0)
        {
            vscMM_Free(pBaseLinkHelper->pMM, pAVPArray);
        }
        else if (avpArraySize > 0)
        {
            /* Client will free it */
            *ppAttrVectorizablePackets = pAVPArray;
            *pNumOfInputPackets = avpArraySize;
        }
    }

    if (pOVPArray)
    {
        if (ovpArraySize == 0)
        {
            vscMM_Free(pBaseLinkHelper->pMM, pOVPArray);
        }
        else if (ovpArraySize > 0)
        {
            /* Client will free it */
            *ppOutputVectorizablePackets = pOVPArray;
            *pNumOfOutputPackets = ovpArraySize;
        }
    }

    return errCode;
}

static VSC_ErrCode _DoIoVectorizationBetweenTwoShaderStagesPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                                     VIR_Shader* pUpperShader,
                                                                     VIR_Shader* pLowerShader,
                                                                     VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                                     VIR_OutputIdList* pOutputIdLstsOfUpperShader)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    gctUINT                     attrCount = VIR_IdList_Count(pAttrIdLstsOfLowerShader);
    gctUINT                     outputCount = VIR_IdList_Count(pOutputIdLstsOfUpperShader);
    VIR_IO_VECTORIZABLE_PACKET* pAttrVectorizablePackets = gcvNULL;
    VIR_IO_VECTORIZABLE_PACKET* pOutputVectorizablePackets = gcvNULL;
    gctUINT                     numOfInputPackets = 0, numOfOutputPackets = 0;
    VIR_IO_VECTORIZE_PARAM      ioVecParam;

    /* No need to go on if there are obvious results */
    if (outputCount == 0 && attrCount == 0)
    {
        return VSC_ERR_NONE;
    }

    /* Find all possible packets for io vectorization */
    errCode = _FindVectorizableIoPackets(pBaseLinkHelper,
                                         pUpperShader,
                                         pLowerShader,
                                         pAttrIdLstsOfLowerShader,
                                         pOutputIdLstsOfUpperShader,
                                         &pAttrVectorizablePackets,
                                         &pOutputVectorizablePackets,
                                         &numOfInputPackets,
                                         &numOfOutputPackets);
    ON_ERROR(errCode, "Find vectorizable IO-packets");

    ioVecParam.pMM = pBaseLinkHelper->pMM;

    /* If we have candidates (packets), yes, vectorize them now */
    if (numOfInputPackets > 0)
    {
        gcmASSERT(pAttrVectorizablePackets);

        ioVecParam.pShader = pLowerShader;
        ioVecParam.pIoVectorizablePackets = pAttrVectorizablePackets;
        ioVecParam.numOfPackets = numOfInputPackets;
        errCode = vscVIR_VectorizeIoPackets(&ioVecParam);
        ON_ERROR(errCode, "Vectorize attribute packets");
    }

    if (numOfOutputPackets)
    {
        gcmASSERT(pOutputVectorizablePackets);

        ioVecParam.pShader = pUpperShader;
        ioVecParam.pIoVectorizablePackets = pOutputVectorizablePackets;
        ioVecParam.numOfPackets = numOfOutputPackets;
        errCode = vscVIR_VectorizeIoPackets(&ioVecParam);
        ON_ERROR(errCode, "Vectorize outputs packets");
    }

OnError:
    if (pAttrVectorizablePackets)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pAttrVectorizablePackets);
    }

    if (pOutputVectorizablePackets)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pOutputVectorizablePackets);
    }

    return errCode;
}

static VSC_ErrCode _DoIoVectorizationBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                            VIR_Shader* pUpperShader,
                                                            VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLstOfLowerShader, workingPerPrimAttrIdLstOfLowerShader;
    VIR_OutputIdList           workingPerVtxPxlAttrIdLstOfUpperShader, workingPerPrimAttrIdLstOfUpperShader;

    _ConvertVirPerVtxPxlAndPerPrimIoList(pLowerShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLstOfLowerShader,
                                         &workingPerPrimAttrIdLstOfLowerShader);

    _ConvertVirPerVtxPxlAndPerPrimIoList(pUpperShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLstOfUpperShader,
                                         &workingPerPrimAttrIdLstOfUpperShader);

    /* Per vtx/Pxl */
    errCode = _DoIoVectorizationBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, pUpperShader, pLowerShader,
                                                                &workingPerVtxPxlAttrIdLstOfLowerShader,
                                                                &workingPerVtxPxlAttrIdLstOfUpperShader);
    ON_ERROR(errCode, "Do Io vectorization between two shader stages per vtx/pxl");

    /* Per prim */
    errCode = _DoIoVectorizationBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, pUpperShader, pLowerShader,
                                                                &workingPerPrimAttrIdLstOfLowerShader,
                                                                &workingPerPrimAttrIdLstOfUpperShader);
    ON_ERROR(errCode, "Do Io vectorization between two shader stages per prim");

OnError:
    return errCode;
}

static VSC_ErrCode _DoIoVectorizationAmongShaderStages(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Shader*  pPreStage = gcvNULL;
    VIR_Shader*  pCurStage;
    gctUINT      stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            if (pPreStage)
            {
                errCode = _DoIoVectorizationBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                                   pPreStage,
                                                                   pCurStage);
                ON_ERROR(errCode, "Do Io vectorization between two shader stages");
            }
            else
            {
                /* For the inputs of first stage, they are related to input data that driver needs
                   help flush (for example, VB data for VS), for now, we dont do io-vectorization
                   for it as it needs driver re-layout that data based on how we vectorized. If later
                   we hit issues on it, we can implement it */
            }

            pPreStage = pCurStage;
        }
    }

    if (pPreStage)
    {
        /* For the outputs of last stage, they are related to output data that driver needs
           help program to get (for example, RT for PS), for now, we dont do io-vectorization
           for it as it needs driver re-layout that data based on how we vectorized. If later
           we hit issues on it, we can implement it */
    }

OnError:
    return errCode;
}

static gctBOOL _IsRedundantIOSymList(VIR_Shader *pShader,
                                     VIR_IdList* pIdList,
                                     gctINT curr)
{
    gctINT prev = curr - 1;
    VIR_IdList* pList = &pIdList[prev];
    VIR_IdList* cList = &pIdList[curr];
    gctINT plen = VIR_IdList_Count(pList);
    gctINT clen = VIR_IdList_Count(cList);
    gctINT i;
    if (plen != clen)
    {
        return gcvFALSE;
    }
    for (i = 0; i < clen; i++)
    {
        /* check symbolId is same in two lists */
        if (VIR_IdList_GetId(pList, i) != VIR_IdList_GetId(cList, i))
        {
            return gcvFALSE;
        }
    }
    return gcvTRUE;
}

static VSC_ErrCode _FindIoVectorizablePacketsForSingleShader(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                             VIR_Shader* pCurrentShader,
                                                             VIR_IdList* pIdList,
                                                             gctBOOL bInput,
                                                             VIR_IO_VECTORIZABLE_PACKET** ppIoVecPackets,
                                                             gctUINT* pNumOfPackets)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_IO_VECTORIZABLE_PACKET* pIoVecPackets = gcvNULL;
    VIR_IdList*                 pList = gcvNULL;
    gctUINT                     i, j, packetIdx, numOfPackets = 0;
    gctUINT                     ioCount;

    /* Find the locations that have multiple IOs. */
    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pIdList[i];
        if (VIR_IdList_Count(pList) > 1 &&
            /* for the vectorized io which original are array with same lengths, redundant symbol list in
               outputComponentMapList[location] and outputComponentMapList[location+virioRegCount-1]
               float[2]  hp out  #spv_id21 ==> temp(279 - 280) common_flags:< enabled statically_used >;
               float[2]  hp out  #spv_id30 ==> temp(281 - 282) common_flags:< enabled statically_used >;
               float[2]  hp out  #spv_id35 ==> temp(283 - 284) common_flags:< enabled statically_used >;
               float[2]  hp out  #spv_id39 ==> temp(285 - 286) common_flags:< enabled statically_used >;
             */
            (i == 0 || !_IsRedundantIOSymList(pCurrentShader, pIdList, i)))
        {
            numOfPackets++;
        }
    }

    gcmASSERT(numOfPackets > 0 && numOfPackets <= MAX_SHADER_IO_NUM);

    /* Allocate packets. */
    pIoVecPackets = (VIR_IO_VECTORIZABLE_PACKET*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                             numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));
    memset(pIoVecPackets, 0, numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));

    /* Fill the data. */
    packetIdx = 0;
    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pIdList[i];
        ioCount = VIR_IdList_Count(pList);
        if (ioCount <= 1 ||
            (i > 0 && _IsRedundantIOSymList(pCurrentShader, pIdList, i)))
        {
            continue;
        }

        gcmASSERT(ioCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);

        for (j = 0; j < ioCount; j++)
        {
            pIoVecPackets[packetIdx].pSymIo[j] = VIR_Shader_GetSymFromId(pCurrentShader, VIR_IdList_GetId(pList, j));
        }

        pIoVecPackets[packetIdx].vectorizedLocation = i;
        pIoVecPackets[packetIdx].realCount = ioCount;
        pIoVecPackets[packetIdx].bOutput = !bInput;
        pIoVecPackets[packetIdx].bTFB = gcvFALSE;
        pIoVecPackets[packetIdx].bComponentPack = gcvTRUE;
        packetIdx++;
    }

    /* Store the return value. */
    if (ppIoVecPackets)
    {
        *ppIoVecPackets = pIoVecPackets;
    }
    if (pNumOfPackets)
    {
        *pNumOfPackets = numOfPackets;
    }

    return errCode;
}

static VSC_ErrCode _FindIoVectorizablePacketsForTwoShader(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                          VIR_Shader* pUpperShader,
                                                          VIR_Shader* pLowerShader,
                                                          VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                                          VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                          VIR_IO_VECTORIZABLE_PACKET** ppOutputVectorizablePackets,
                                                          VIR_IO_VECTORIZABLE_PACKET** ppAttrVectorizablePackets,
                                                          gctUINT* pNumOfOutputPackets,
                                                          gctUINT* pNumOfInputPackets)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_IO_VECTORIZABLE_PACKET* pOutputVecPackets = gcvNULL;
    VIR_IO_VECTORIZABLE_PACKET* pInputVecPackets = gcvNULL;
    VIR_IdList*                 pList = gcvNULL;
    gctUINT                     i, j, packetIdx, numOfPackets = 0;
    gctUINT                     ioCount;

    /* Find the locations that have multiple IOs. */
    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        pList = &pAttrIdLstsOfLowerShader[i];
        if (VIR_IdList_Count(pList) > 1)
        {
            numOfPackets++;
        }
    }

    gcmASSERT(numOfPackets > 0 && numOfPackets <= MAX_SHADER_IO_NUM);

    /* Allocate output packets. */
    pOutputVecPackets = (VIR_IO_VECTORIZABLE_PACKET*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                 numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));
    memset(pOutputVecPackets, 0, numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));

    /* Allocate input packets. */
    pInputVecPackets = (VIR_IO_VECTORIZABLE_PACKET*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));
    memset(pInputVecPackets, 0, numOfPackets * sizeof(VIR_IO_VECTORIZABLE_PACKET));

    /* Fill the data. */
    packetIdx = 0;
    for (i = 0; i < MAX_SHADER_IO_NUM; i++)
    {
        /* Get inputs. */
        pList = &pAttrIdLstsOfLowerShader[i];
        ioCount = VIR_IdList_Count(pList);
        if (ioCount <= 1)
        {
            continue;
        }

        gcmASSERT(ioCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);

        for (j = 0; j < ioCount; j++)
        {
            pInputVecPackets[packetIdx].pSymIo[j] = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pList, j));
        }

        pInputVecPackets[packetIdx].vectorizedLocation = i;
        pInputVecPackets[packetIdx].realCount = ioCount;
        pInputVecPackets[packetIdx].bOutput = gcvFALSE;
        pInputVecPackets[packetIdx].bTFB = gcvFALSE;
        pInputVecPackets[packetIdx].bComponentPack = gcvTRUE;

        /* Get outputs. */
        pList = &pOutputIdLstsOfUpperShader[i];
        ioCount = VIR_IdList_Count(pList);
        gcmASSERT(ioCount > 1 && ioCount <= MAX_VECTORIZABLE_IO_NUM_IN_PACKET);

        for (j = 0; j < ioCount; j++)
        {
            pOutputVecPackets[packetIdx].pSymIo[j] = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pList, j));
        }

        pOutputVecPackets[packetIdx].vectorizedLocation = i;
        pOutputVecPackets[packetIdx].realCount = ioCount;
        pOutputVecPackets[packetIdx].bOutput = gcvTRUE;
        pOutputVecPackets[packetIdx].bTFB = gcvFALSE;
        pOutputVecPackets[packetIdx].bComponentPack = gcvTRUE;
        packetIdx++;
    }

    /* Store the return value. */
    if (ppAttrVectorizablePackets)
    {
        *ppAttrVectorizablePackets = pInputVecPackets;
    }
    if (pNumOfInputPackets)
    {
        *pNumOfInputPackets = numOfPackets;
    }
    if (ppOutputVectorizablePackets)
    {
        *ppOutputVectorizablePackets = pOutputVecPackets;
    }
    if (pNumOfOutputPackets)
    {
        *pNumOfOutputPackets = numOfPackets;
    }

    return errCode;
}

static VSC_ErrCode _DoIoComponentPackSingleShaderStage(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                       VIR_Shader* pCurrentShader,
                                                       VIR_IdList* pIdList,
                                                       gctBOOL bInput)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_IO_VECTORIZABLE_PACKET* pIoVecPackets = gcvNULL;
    gctUINT                     numOfPackets = 0;
    VIR_IO_VECTORIZE_PARAM      ioVecParam;

    /* Find all IOs that consume the individual components within a location. */
    errCode = _FindIoVectorizablePacketsForSingleShader(pBaseLinkHelper,
                                                        pCurrentShader,
                                                        pIdList,
                                                        bInput,
                                                        &pIoVecPackets,
                                                        &numOfPackets);
    ON_ERROR(errCode, "Construct IO vectorizable packets. ");

    /* If we have candidates (packets), yes, vectorize them now */
    gcmASSERT(numOfPackets > 0 && numOfPackets <= MAX_SHADER_IO_NUM);
    gcmASSERT(pIoVecPackets);

    ioVecParam.pMM = pBaseLinkHelper->pMM;
    ioVecParam.pShader = pCurrentShader;
    ioVecParam.pIoVectorizablePackets = pIoVecPackets;
    ioVecParam.numOfPackets = numOfPackets;

    errCode = vscVIR_VectorizeIoPackets(&ioVecParam);
    ON_ERROR(errCode, "Vectorize packets");

OnError:
    return errCode;
}

static VSC_ErrCode _DoIoComponentPackBetweenTwoShaderStage(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                          VIR_Shader* pUpperShader,
                                                          VIR_Shader* pLowerShader,
                                                          VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                                          VIR_AttributeIdList* pAttrIdLstsOfLowerShader)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_IO_VECTORIZABLE_PACKET* pOutputVecPackets = gcvNULL;
    VIR_IO_VECTORIZABLE_PACKET* pInputVecPackets = gcvNULL;
    gctUINT                     numOfOutputPackets = 0, numOfInputPackets = 0;
    VIR_IO_VECTORIZE_PARAM      ioVecParam;

    /* Find all IOs that consume the individual components within a location. */
    errCode = _FindIoVectorizablePacketsForTwoShader(pBaseLinkHelper,
                                                     pUpperShader,
                                                     pLowerShader,
                                                     pOutputIdLstsOfUpperShader,
                                                     pAttrIdLstsOfLowerShader,
                                                     &pOutputVecPackets,
                                                     &pInputVecPackets,
                                                     &numOfOutputPackets,
                                                     &numOfInputPackets);
    ON_ERROR(errCode, "Construct IO vectorizable packets. ");

    /* If we have candidates (packets), yes, vectorize them now */
    gcmASSERT(numOfInputPackets > 0 && numOfInputPackets <= MAX_SHADER_IO_NUM);
    gcmASSERT(pInputVecPackets);

    /* vectorize output.  */
    ioVecParam.pMM = pBaseLinkHelper->pMM;
    ioVecParam.pShader = pUpperShader;
    ioVecParam.pIoVectorizablePackets = pOutputVecPackets;
    ioVecParam.numOfPackets = numOfOutputPackets;

    errCode = vscVIR_VectorizeIoPackets(&ioVecParam);
    ON_ERROR(errCode, "Vectorize packets");

    /* vectorize input.  */
    ioVecParam.pMM = pBaseLinkHelper->pMM;
    ioVecParam.pShader = pLowerShader;
    ioVecParam.pIoVectorizablePackets = pInputVecPackets;
    ioVecParam.numOfPackets = numOfInputPackets;

    errCode = vscVIR_VectorizeIoPackets(&ioVecParam);
    ON_ERROR(errCode, "Vectorize packets");

OnError:
    return errCode;
}

static VSC_ErrCode _DoIoComponentPackAmongShaderStages(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Shader*  pPreStage = gcvNULL;
    VIR_Shader*  pCurStage;
    gctUINT      stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            if (VIR_Shader_HAS_INPUT_COMP_MAP(pCurStage))
            {
                gcmASSERT(VIR_Shader_IsVulkan(pCurStage));

                if (pPreStage)
                {
                    errCode = _DoIoComponentPackBetweenTwoShaderStage(&pPgLinkHelper->baseHelper,
                                                                      pPreStage,
                                                                      pCurStage,
                                                                      VIR_Shader_GetOutputComponentMapList(pPreStage),
                                                                      VIR_Shader_GetAttributeComponentMapList(pCurStage));
                    ON_ERROR(errCode, "Do Io vectorization between two shader stages");
                }
                else
                {
                    if (VIR_Shader_IsVS(pCurStage))
                    {
                        errCode = _DoIoComponentPackSingleShaderStage(&pPgLinkHelper->baseHelper,
                                                                      pCurStage,
                                                                      VIR_Shader_GetAttributeComponentMapList(pCurStage),
                                                                      gcvTRUE);
                        ON_ERROR(errCode, "Do input component pack for single shader stage.");
                    }
                }
            }

            pPreStage = pCurStage;
        }
    }

    if (pPreStage && VIR_Shader_HAS_OUTPUT_COMP_MAP(pPreStage))
    {
        if (VIR_Shader_IsFS(pPreStage))
        {
            errCode = _DoIoComponentPackSingleShaderStage(&pPgLinkHelper->baseHelper,
                                                          pPreStage,
                                                          VIR_Shader_GetOutputComponentMapList(pPreStage),
                                                          gcvFALSE);
            ON_ERROR(errCode, "Do output component pack for single shader stage.");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoFirstStageOfLinkage(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper, FSL_STAGE fslStage)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;

    if (fslStage == FSL_STAGE_IO_VECTORIZATION)
    {
        /* Do Io vectorization among stages */
        errCode = _DoIoVectorizationAmongShaderStages(pPgLinkHelper);
        ON_ERROR(errCode, "Do Io vectorization among shader stages");
    }
    else
    {
        /* Link IO among stages */
        errCode = _LinkIoAmongShaderStages(pPgLinkHelper, fslStage);
        ON_ERROR(errCode, "Link Io among shader stages");
    }

    if (fslStage == FSL_STAGE_API_SPEC_CHECK)
    {
        /* Link uniforms among states. */
        errCode = _LinkUniformAmongShaderStages(pPgLinkHelper);
        ON_ERROR(errCode, "Link Uniform among shader stages");

        /* Link IOB among states. */
        errCode = _LinkIobAmongShaderStages(pPgLinkHelper);
        ON_ERROR(errCode, "Link IoB among shader stages");
    }

OnError:
    return errCode;
}

static void _SetHwCompIndexForSVs(VIR_ShaderKind shKind,
                                  VIR_Symbol* pSvSymbol,
                                  gctBOOL bNeedIoMemPacked,
                                  gctUINT hwChannelIdxForPos,
                                  gctUINT hwChannelIdxForPtSz,
                                  gctUINT hwChannelIdxForPrimId,
                                  gctUINT hwChannelIdxForPtCoord)
{
    gcmASSERT(VIR_Symbol_GetHwFirstCompIndex(pSvSymbol) == NOT_ASSIGNED);

    if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_TESS_LEVEL_OUTER)
    {
        if (!bNeedIoMemPacked)
        {
            /* Outer tess-factor has 4 32-bit values, hwCompIndex is from 0 to 3 */
            VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, 0);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_TESS_LEVEL_INNER)
    {
        if (!bNeedIoMemPacked)
        {
            /* Inner tess-factor has 2 32-bit values, hwCompIndex is from 4 to 5 */
            VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, CHANNEL_NUM);
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_POSITION ||
             VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_IN_POSITION)
    {
        if (hwChannelIdxForPos != NOT_ASSIGNED)
        {
            if (!bNeedIoMemPacked)
            {
                VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, hwChannelIdxForPos);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }
    else if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_POINT_SIZE ||
             VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_IN_POINT_SIZE)
    {
        if (hwChannelIdxForPtSz != NOT_ASSIGNED)
        {
            if (!bNeedIoMemPacked)
            {
                VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, hwChannelIdxForPtSz);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }
    else if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_PRIMITIVE_ID)
    {
        if (VIR_SHADER_TESSELLATION_EVALUATION == shKind)
        {
            if (!bNeedIoMemPacked)
            {
                /* hwCompIndex for primitiveId of DS is 6 */
                VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, CHANNEL_NUM + CHANNEL_Z);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else if (hwChannelIdxForPrimId != NOT_ASSIGNED)
        {
            if (!bNeedIoMemPacked)
            {
                VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, hwChannelIdxForPrimId);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }
    else if (VIR_Symbol_GetName(pSvSymbol) == VIR_NAME_POINT_COORD)
    {
        if (hwChannelIdxForPtCoord != NOT_ASSIGNED)
        {
            if (!bNeedIoMemPacked)
            {
                VIR_Symbol_SetHwFirstCompIndex(pSvSymbol, hwChannelIdxForPtCoord);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
    }
    else
    {
        /* Currently, do not need consider other SV because they are all allocated in gpr */
    }
}

static gctBOOL _NeedIoHwMemPackedBetweenTwoShaderStagesPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                                 VIR_Shader* pUpperShader,
                                                                 VIR_Shader* pLowerShader,
                                                                 VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                                 VIR_OutputIdList* pOutputIdLstsOfUpperShader)
{
    /* Currently, we don't support packed memory for IO */
    return gcvFALSE;
}

static VSC_ErrCode _CalcIoHwCompIndexBetweenTwoShaderStagesPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                                     VIR_Shader* pUpperShader,
                                                                     VIR_Shader* pLowerShader,
                                                                     VIR_AttributeIdList* pAttrIdLstsOfLowerShader,
                                                                     VIR_OutputIdList* pOutputIdLstsOfUpperShader,
                                                                     gctBOOL bPerPrim)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    attrIdx, outputIdx, hwChannelIdx = 0;
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLstsOfLowerShader);
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLstsOfUpperShader);
    gctUINT                    soOutputCount = 0, thisOutputRegCount;
    VIR_Symbol*                pAttrSym;
    VIR_Symbol*                pOutputSym;
    VIR_Symbol*                pSoSym;
    VSC_BIT_VECTOR             outputWorkingMask;
    gctUINT                    hwChannelIdxForPrimId = NOT_ASSIGNED, hwChannelIdxForPtCoord = NOT_ASSIGNED;
    gctUINT                    hwChannelIdxForPos = NOT_ASSIGNED, hwChannelIdxForPtSz = NOT_ASSIGNED;
    gctBOOL                    bNeedIoMemPacked;

    /* No need to go on if there are obvious results */
    if (outputCount == 0 && attrCount == 0)
    {
        return VSC_ERR_NONE;
    }

    /* !!!
          All hw-comp-index sequence is STRICTLY same as seq describled in
          _LinkVtxPxlIoBetweenTwoHwShaderStages  and _LinkPrimIoBetweenTwoHwShaderStages
       !!! */

    bNeedIoMemPacked = _NeedIoHwMemPackedBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper,
                                                                         pUpperShader,
                                                                         pLowerShader,
                                                                         pAttrIdLstsOfLowerShader,
                                                                         pOutputIdLstsOfUpperShader);

    /* HW reserve 8 packed components for TCS's per-patch data for tess-factors and other specials */
    if (pUpperShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL && bPerPrim)
    {
        hwChannelIdx = 2 * CHANNEL_NUM;
    }

    /* HW reserve 4 components to store vec4 position */
    if (pLowerShader->shaderKind == VIR_SHADER_FRAGMENT && !bPerPrim)
    {
        hwChannelIdxForPos = 0;
        hwChannelIdx = CHANNEL_NUM;
    }

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM, outputCount);

    /* For each attribute of lower-shader, find whether upper-shader has corresponding output */
    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

        /* Only consider used one */
        if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
        {
            continue;
        }

        /* Skip SGV now */
        if (VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym)) &&
            !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym), gcvTRUE))
        {
            continue;
        }

        /* For each output of upper-shader */
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            if (vscBV_TestBit(&outputWorkingMask, outputIdx))
            {
                continue;
            }

            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            if (isSymVectorizedOut(pOutputSym))
            {
                continue;
            }

            /* If flag SKIP-NAME-CHECK is present, check location only. */
            if (isSymSkipNameCheck(pAttrSym))
            {
                /* When a symbol has SKIP-NAME-CHECK flag, its location must be NOT -1. */
                gcmASSERT(VIR_Symbol_GetLocation(pAttrSym) != -1);

                if (VIR_Symbol_GetLocation(pAttrSym) != VIR_Symbol_GetLocation(pOutputSym))
                {
                    continue;
                }
            }
            /* Name or location of the output and attribute must be matched */
            else if (!(VIR_Symbol_isNameMatch(pLowerShader, pAttrSym, pUpperShader, pOutputSym) ||
                     (VIR_Symbol_GetLocation(pOutputSym) != -1 &&
                      VIR_Symbol_GetLocation(pOutputSym) == VIR_Symbol_GetLocation(pAttrSym))))
            {
                continue;
            }

            thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

            gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));
            gcmASSERT(VIR_Symbol_GetVirIoRegCount(pLowerShader, pAttrSym) == thisOutputRegCount);

            /* No need to set component-index as location specified by app here because it is an IO
               among the shader stages which are linked together as non-seperated shaders */
            if (!bNeedIoMemPacked)
            {
                pAttrSym->layout.hwFirstCompIndex =
                pOutputSym->layout.hwFirstCompIndex = hwChannelIdx;

                hwChannelIdx += thisOutputRegCount * CHANNEL_NUM;
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            vscBV_SetBit(&outputWorkingMask, outputIdx);

            break;
        }

        gcmASSERT(outputIdx < outputCount);
    }

    /* !!! For undefined position/point-size, just give them any hwChannelIdx (we now
           give 0 to it). See function _CheckFakeSGVForPosAndPtSz to check why we need
           this code !!! */
    if ((pUpperShader->shaderKind == VIR_SHADER_VERTEX) &&
        (pLowerShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL ||
         pLowerShader->shaderKind == VIR_SHADER_GEOMETRY))
    {
        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

            /* Only consider used one */
            if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
            {
                continue;
            }

            if (VIR_Symbol_GetHwFirstCompIndex(pAttrSym) == NOT_ASSIGNED)
            {
                if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_POSITION ||
                    VIR_Symbol_GetName(pAttrSym) == VIR_NAME_IN_POSITION)
                {
                    hwChannelIdxForPos = 0;
                }

                if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_POINT_SIZE ||
                    VIR_Symbol_GetName(pAttrSym) == VIR_NAME_IN_POINT_SIZE)
                {
                    hwChannelIdxForPtSz = 0;
                }
            }
        }
    }

    if (pLowerShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        /* Pre-reserve pt-coord's hwChannelIdx */
        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

            /* Only consider used one */
            if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
            {
                continue;
            }

            if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_POINT_COORD &&
                VIR_Symbol_GetHwFirstCompIndex(pAttrSym) == NOT_ASSIGNED)
            {
                hwChannelIdxForPtCoord = hwChannelIdx;
                if (!bNeedIoMemPacked)
                {
                    hwChannelIdx += CHANNEL_NUM;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

                VIR_Shader_SetFlag(pLowerShader, VIR_SHFLAG_PS_SPECIALLY_ALLOC_POINT_COORD);

                break;
            }
        }

        /* Pre-reserve primitive-id's hwChannelIdx if primitive-id is not regarded as normal IO (that means
           the hwChannelIndex is not assigned yet) */
        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

            /* Only consider used one */
            if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
            {
                continue;
            }

            if (VIR_Symbol_GetName(pAttrSym) == VIR_NAME_PRIMITIVE_ID &&
                VIR_Symbol_GetHwFirstCompIndex(pAttrSym) == NOT_ASSIGNED)
            {
                hwChannelIdxForPrimId = hwChannelIdx;
                if (!bNeedIoMemPacked)
                {
                    hwChannelIdx += CHANNEL_NUM;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

                VIR_Shader_SetFlag(pLowerShader, VIR_SHFLAG_PS_SPECIALLY_ALLOC_PRIMTIVE_ID);

                break;
            }
        }

        /* Pre-reserve point-size's hwChannelIdx */
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_POINT_SIZE)
            {
                hwChannelIdxForPtSz = hwChannelIdx;
                if (!bNeedIoMemPacked)
                {
                    hwChannelIdx += CHANNEL_NUM;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

                break;
            }
        }
    }

    /* Then for stream-out */
    if (pUpperShader->transformFeedback.varyings)
    {
        gcmASSERT(pLowerShader->shaderKind == VIR_SHADER_FRAGMENT);

        soOutputCount = VIR_IdList_Count(pUpperShader->transformFeedback.varyings);

        if (soOutputCount > 0)
        {
            for (outputIdx = 0; outputIdx < soOutputCount; outputIdx ++)
            {
                pSoSym = VIR_Shader_GetSymFromId(pUpperShader,
                                    VIR_IdList_GetId(pUpperShader->transformFeedback.varyings, outputIdx));

                pOutputSym = VIR_Symbol_GetIndexingInfo(pUpperShader, pSoSym).underlyingSym;

                /* Skip SIV now */
                if (VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym)) &&
                    !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym, gcvTRUE))
                {
                    continue;
                }

                if (VIR_Symbol_GetHwFirstCompIndex(pOutputSym) == NOT_ASSIGNED)
                {
                    thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                    gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

                    if (!bNeedIoMemPacked)
                    {
                        pOutputSym->layout.hwFirstCompIndex = hwChannelIdx;

                        hwChannelIdx += thisOutputRegCount * CHANNEL_NUM;
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }
                }
            }
        }
    }

    /* Then for hs's per-cp outputs that are not used by next stage, but used
       in shader for other HS kickoff */
    if (pUpperShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

            /* Skip SIV now */
            if (VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym)) &&
                !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym, gcvTRUE))
            {
                continue;
            }

            if (!VIR_Symbol_HasFlag(pOutputSym, VIR_SYMFLAG_STATICALLY_USED))
            {
                continue;
            }

            if (VIR_Symbol_GetHwFirstCompIndex(pOutputSym) == NOT_ASSIGNED &&
                !isSymVectorizedOut(pOutputSym))
            {
                thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pUpperShader, pOutputSym);

                gcmASSERT(!isSymUnused(pOutputSym));

                if (!bNeedIoMemPacked)
                {
                    pOutputSym->layout.hwFirstCompIndex = hwChannelIdx;

                    hwChannelIdx += thisOutputRegCount * CHANNEL_NUM;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }
        }
    }

    /* Last for SV IOs  */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pUpperShader, VIR_IdList_GetId(pOutputIdLstsOfUpperShader, outputIdx));

        if (isSymUnused(pOutputSym) || isSymVectorizedOut(pOutputSym))
        {
            continue;
        }

        /* Only consider SIVs */
        if (!(VIR_Shader_IsNameBuiltIn(pUpperShader, VIR_Symbol_GetName(pOutputSym)) &&
              !_IsFakeSIV(pUpperShader, pLowerShader, pOutputSym, gcvTRUE)))
        {
            continue;
        }

        _SetHwCompIndexForSVs(pUpperShader->shaderKind, pOutputSym, bNeedIoMemPacked,
                              hwChannelIdxForPos, hwChannelIdxForPtSz, hwChannelIdxForPrimId,
                              hwChannelIdxForPtCoord);
    }

    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pLowerShader, VIR_IdList_GetId(pAttrIdLstsOfLowerShader, attrIdx));

        /* Only consider used one */
        if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
        {
            continue;
        }

        /* Only consider SGVs */
        if (!(VIR_Shader_IsNameBuiltIn(pLowerShader, VIR_Symbol_GetName(pAttrSym)) &&
              !_IsFakeSGV(pUpperShader, pLowerShader, VIR_Symbol_GetName(pAttrSym), gcvTRUE)))
        {
            continue;
        }

        _SetHwCompIndexForSVs(pLowerShader->shaderKind, pAttrSym, bNeedIoMemPacked,
                              hwChannelIdxForPos, hwChannelIdxForPtSz, hwChannelIdxForPrimId,
                              hwChannelIdxForPtCoord);
    }

    vscBV_Finalize(&outputWorkingMask);

    return errCode;
}

static VSC_ErrCode _CalcIoHwCompIndexBetweenTwoShaderStages(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                            VIR_Shader* pUpperShader,
                                                            VIR_Shader* pLowerShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLstOfLowerShader, workingPerPrimAttrIdLstOfLowerShader;
    VIR_OutputIdList           workingPerVtxPxlAttrIdLstOfUpperShader, workingPerPrimAttrIdLstOfUpperShader;

    /* Currently, we only consider VS+HS or HS+DS or VS+GS or DS+GS or GS+PS (if GS does not support EMIT) */
    if (pLowerShader->shaderKind == VIR_SHADER_FRAGMENT &&
        (pUpperShader->shaderKind != VIR_SHADER_GEOMETRY ||
         pBaseLinkHelper->pHwCfg->hwFeatureFlags.gsSupportEmit))
    {
        return VSC_ERR_NONE;
    }

    _ConvertVirPerVtxPxlAndPerPrimIoList(pLowerShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLstOfLowerShader,
                                         &workingPerPrimAttrIdLstOfLowerShader);

    _ConvertVirPerVtxPxlAndPerPrimIoList(pUpperShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLstOfUpperShader,
                                         &workingPerPrimAttrIdLstOfUpperShader);

    /* Per vtx/Pxl */
    errCode = _CalcIoHwCompIndexBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, pUpperShader, pLowerShader,
                                                                &workingPerVtxPxlAttrIdLstOfLowerShader,
                                                                &workingPerVtxPxlAttrIdLstOfUpperShader,
                                                                gcvFALSE);
    ON_ERROR(errCode, "Calc io hw component index between two shader stages per vtx/pxl");

    /* Per prim */
    errCode = _CalcIoHwCompIndexBetweenTwoShaderStagesPerExeObj(pBaseLinkHelper, pUpperShader, pLowerShader,
                                                                &workingPerPrimAttrIdLstOfLowerShader,
                                                                &workingPerPrimAttrIdLstOfUpperShader,
                                                                gcvTRUE);
    ON_ERROR(errCode, "Calc io hw component index between two shader stages per prim");

OnError:
    return errCode;
}

static gctBOOL _NeedInputHwMemPacked(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                     VIR_Shader* pShader,
                                     VIR_AttributeIdList* pAttrIdLsts)
{
    /* Currently, we don't support packed memory for IO */
    return gcvFALSE;
}

static VSC_ErrCode _CalcInputHwCompIndexPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                  VIR_Shader* pShader,
                                                  VIR_AttributeIdList* pAttrIdLsts,
                                                  gctBOOL bPerPrim)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    attrIdx, hwChannelIdx = 0, thisHwChannelIdx;
    gctUINT                    attrCount = VIR_IdList_Count(pAttrIdLsts);
    gctUINT                    thisAttrRegCount;
    gctINT                     location, ioIdx = 0;
    VSC_BIT_VECTOR             inputWorkingMask;
    VIR_Symbol*                pAttrSym;
    gctBOOL                    bNeedIoMemPacked, bHasNoAssignedLocation = gcvFALSE;;

    vscBV_Initialize(&inputWorkingMask, pBaseLinkHelper->pMM,
                     pBaseLinkHelper->pHwCfg->maxAttributeCount * CHANNEL_NUM);

    bNeedIoMemPacked = _NeedInputHwMemPacked(pBaseLinkHelper, pShader, pAttrIdLsts);

    /* HW reserve 8 packed components for TCS's per-patch data for tess-factors and other specials */
    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION && bPerPrim)
    {
        hwChannelIdx = 2 * CHANNEL_NUM;
        vscBV_SetInRange(&inputWorkingMask, 0, (2 * CHANNEL_NUM));
    }

    /* Firstly for all inputs with 'location' */
    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrIdLsts, attrIdx));

        if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
        {
            thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);

            location = VIR_Symbol_GetLocation(pAttrSym);
            if (location != -1)
            {
                if (!bNeedIoMemPacked)
                {
                    thisHwChannelIdx = hwChannelIdx + location * CHANNEL_NUM;
                    VIR_Symbol_SetHwFirstCompIndex(pAttrSym, thisHwChannelIdx);

                    vscBV_SetInRange(&inputWorkingMask, thisHwChannelIdx, (thisAttrRegCount * CHANNEL_NUM));
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }
            else
            {
                bHasNoAssignedLocation = gcvTRUE;
            }
        }
    }

    /* Then for inputs that have no 'location', except those reserved SGVs */
    if (bHasNoAssignedLocation)
    {
        /* We hit here due to 2 reasons:
           1. App does not set locations for all app-level inputs
           2. For some internals or SVs, location can be missed
        */

        for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
        {
            pAttrSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrIdLsts, attrIdx));

            if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
            {
                if ((VIR_Symbol_GetName(pAttrSym) == VIR_NAME_PRIMITIVE_ID &&
                     pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION) ||
                    VIR_Symbol_GetName(pAttrSym) == VIR_NAME_TESS_LEVEL_OUTER ||
                    VIR_Symbol_GetName(pAttrSym) == VIR_NAME_TESS_LEVEL_INNER)
                {
                    continue;
                }

                if (VIR_Symbol_GetHwFirstCompIndex(pAttrSym) == NOT_ASSIGNED)
                {
                    thisAttrRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);

                    ioIdx = vscBV_FindContinuousClearBitsForward(&inputWorkingMask,
                                                                 thisAttrRegCount * CHANNEL_NUM, 0);

                    if (ioIdx == INVALID_BIT_LOC)
                    {
                        errCode = VSC_ERR_INVALID_DATA;
                        ON_ERROR(errCode, "Calc input hw-comp-index");
                    }

                    VIR_Symbol_SetHwFirstCompIndex(pAttrSym, ioIdx);

                    vscBV_SetInRange(&inputWorkingMask, ioIdx, (thisAttrRegCount * CHANNEL_NUM));
                }
            }
        }
    }

    /* Last for known reserved SGV IOs  */
    for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
    {
        pAttrSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrIdLsts, attrIdx));

        /* Only consider used one */
        if (isSymUnused(pAttrSym) || isSymVectorizedOut(pAttrSym))
        {
            continue;
        }

        if ((VIR_Symbol_GetName(pAttrSym) == VIR_NAME_PRIMITIVE_ID &&
             pShader->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION) ||
            VIR_Symbol_GetName(pAttrSym) == VIR_NAME_TESS_LEVEL_OUTER ||
            VIR_Symbol_GetName(pAttrSym) == VIR_NAME_TESS_LEVEL_INNER)
        {
            _SetHwCompIndexForSVs(pShader->shaderKind, pAttrSym, bNeedIoMemPacked,
                                  NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED);
        }
    }

OnError:
    vscBV_Finalize(&inputWorkingMask);

    return errCode;
}

static VSC_ErrCode _CalcInputHwCompIndex(VSC_BASE_LINKER_HELPER* pBaseLinkHelper, VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;

    /* Currently, we only consider HS/DS/GS */
    if (pShader->shaderKind == VIR_SHADER_VERTEX ||
        pShader->shaderKind == VIR_SHADER_FRAGMENT)
    {
        return VSC_ERR_NONE;
    }

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvTRUE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CalcInputHwCompIndexPerExeObj(pBaseLinkHelper, pShader, &workingPerVtxPxlAttrIdLst, gcvFALSE);
    ON_ERROR(errCode, "Calc input hw component index per vtx/pxl");

    /* Per prim */
    errCode = _CalcInputHwCompIndexPerExeObj(pBaseLinkHelper, pShader, &workingPerPrimAttrIdLst, gcvTRUE);
    ON_ERROR(errCode, "Calc input hw component index per prim");

OnError:
    return errCode;
}

static gctBOOL _NeedOutputHwMemPacked(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                      VIR_Shader* pShader,
                                      VIR_AttributeIdList* pAttrIdLsts)
{
    /* Currently, we don't support packed memory for IO */
    return gcvFALSE;
}

static VSC_ErrCode _CalcOutputHwCompIndexPerExeObj(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                                   VIR_Shader* pShader,
                                                   VIR_OutputIdList* pOutputIdLsts,
                                                   gctBOOL bPerPrim)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    outputIdx, hwChannelIdx = 0, thisHwChannelIdx;
    gctUINT                    outputCount = VIR_IdList_Count(pOutputIdLsts);
    gctUINT                    thisOutputRegCount;
    VIR_Symbol*                pOutputSym;
    VSC_BIT_VECTOR             outputWorkingMask;
    gctINT                     location, ioIdx = 0;
    gctBOOL                    bNeedIoMemPacked, bHasNoAssignedLocation = gcvFALSE;

    vscBV_Initialize(&outputWorkingMask, pBaseLinkHelper->pMM,
                     pBaseLinkHelper->pHwCfg->maxAttributeCount * CHANNEL_NUM);

    bNeedIoMemPacked = _NeedOutputHwMemPacked(pBaseLinkHelper, pShader, pOutputIdLsts);

    /* HW reserve 8 packed components for TCS's per-patch data for tess-factors and other specials */
    if (pShader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL && bPerPrim)
    {
        hwChannelIdx = 2 * CHANNEL_NUM;
        vscBV_SetInRange(&outputWorkingMask, 0, (2 * CHANNEL_NUM));
    }

    /* Firstly for all outputs with 'location' */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

        thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pOutputSym);

        gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

        location = VIR_Symbol_GetLocation(pOutputSym);
        if (location != -1)
        {
            if (!bNeedIoMemPacked)
            {
                thisHwChannelIdx = hwChannelIdx + location * CHANNEL_NUM;
                VIR_Symbol_SetHwFirstCompIndex(pOutputSym, thisHwChannelIdx);

                vscBV_SetInRange(&outputWorkingMask, thisHwChannelIdx, (thisOutputRegCount * CHANNEL_NUM));
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            bHasNoAssignedLocation = gcvTRUE;
        }
    }

    /* Then for outputs that have no 'location', except those reserved SIVs */
    if (bHasNoAssignedLocation)
    {
        /* We hit here due to 2 reasons:
           1. App does not set locations for all app-level outputs
           2. For some internals or SVs, location can be missed
        */

        for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
        {
            pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

            gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

            if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_TESS_LEVEL_OUTER ||
                VIR_Symbol_GetName(pOutputSym) == VIR_NAME_TESS_LEVEL_INNER)
            {
                continue;
            }

            if (VIR_Symbol_GetHwFirstCompIndex(pOutputSym) == NOT_ASSIGNED)
            {
                thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pOutputSym);

                ioIdx = vscBV_FindContinuousClearBitsForward(&outputWorkingMask,
                                                             thisOutputRegCount * CHANNEL_NUM, 0);

                if (ioIdx == INVALID_BIT_LOC)
                {
                    errCode = VSC_ERR_INVALID_DATA;
                    ON_ERROR(errCode, "Calc output hw-comp-index");
                }

                VIR_Symbol_SetHwFirstCompIndex(pOutputSym, ioIdx);

                vscBV_SetInRange(&outputWorkingMask, ioIdx, (thisOutputRegCount * CHANNEL_NUM));
            }
        }
    }

    /* Last for known reserved SIV IOs  */
    for (outputIdx = 0; outputIdx < outputCount; outputIdx ++)
    {
        pOutputSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputIdLsts, outputIdx));

        gcmASSERT(!isSymUnused(pOutputSym) && !isSymVectorizedOut(pOutputSym));

        if (VIR_Symbol_GetName(pOutputSym) == VIR_NAME_TESS_LEVEL_OUTER ||
            VIR_Symbol_GetName(pOutputSym) == VIR_NAME_TESS_LEVEL_INNER)
        {
            _SetHwCompIndexForSVs(pShader->shaderKind, pOutputSym, bNeedIoMemPacked,
                                  NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED, NOT_ASSIGNED);
        }
    }

OnError:
    vscBV_Finalize(&outputWorkingMask);

    return errCode;
}

static VSC_ErrCode _CalcOutputHwCompIndex(VSC_BASE_LINKER_HELPER* pBaseLinkHelper, VIR_Shader* pShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList        workingPerVtxPxlAttrIdLst, workingPerPrimAttrIdLst;

    /* Currently, we only consider HS or GS (if GS does not support EMIT) */
    if (pShader->shaderKind != VIR_SHADER_TESSELLATION_CONTROL &&
        (pShader->shaderKind != VIR_SHADER_GEOMETRY ||
         pBaseLinkHelper->pHwCfg->hwFeatureFlags.gsSupportEmit))
    {
        return VSC_ERR_NONE;
    }

    _ConvertVirPerVtxPxlAndPerPrimIoList(pShader, pBaseLinkHelper->pMM,
                                         gcvFALSE, &workingPerVtxPxlAttrIdLst, &workingPerPrimAttrIdLst);

    /* Per vtx/Pxl */
    errCode = _CalcOutputHwCompIndexPerExeObj(pBaseLinkHelper, pShader, &workingPerVtxPxlAttrIdLst, gcvFALSE);
    ON_ERROR(errCode, "Calc output hw component index per vtx/pxl");

    /* Per prim */
    errCode = _CalcOutputHwCompIndexPerExeObj(pBaseLinkHelper, pShader, &workingPerPrimAttrIdLst, gcvTRUE);
    ON_ERROR(errCode, "Calc output hw component index per prim");

OnError:
    return errCode;
}

static VSC_ErrCode _CalcHwCompIndexForIOs(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pPreStage = gcvNULL;
    VIR_Shader*                pCurStage;
    gctUINT                    stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage= (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            if (pPreStage)
            {
                /* Calc io hw component index between two shader stages */
                errCode = _CalcIoHwCompIndexBetweenTwoShaderStages(&pPgLinkHelper->baseHelper,
                                                                   pPreStage,
                                                                   pCurStage);
                ON_ERROR(errCode, "Calc io hw component index between two shader stages");
            }
            else
            {
                /* Only consider input of first active stage */
                errCode = _CalcInputHwCompIndex(&pPgLinkHelper->baseHelper,
                                                pCurStage);
                ON_ERROR(errCode, "Calc hw component index of input");
            }

            pPreStage = pCurStage;
        }
    }

    if (pPreStage)
    {
        /* Only consider output of last active stage */
        errCode = _CalcOutputHwCompIndex(&pPgLinkHelper->baseHelper, pPreStage);
        ON_ERROR(errCode, "Calc hw component index of output");
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CalcSamplerBaseOffset(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = gcvNULL;
    VSC_HW_CONFIG*             pHwCfg = pPgLinkHelper->baseHelper.pHwCfg;
    gctINT                     samplerBaseOffset, samplerCount, maxSamplerCount = 0;
    gctUINT                    stageIdx;

    if (!(pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.cFlags & VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC))
    {
        return errCode;
    }

    samplerBaseOffset = pHwCfg->maxHwNativeTotalSamplerCount;

    /* We pack all GPipe shaders one by one, and use the bottom sampler memory for GPipe. */
    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pShader = (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pShader)
        {
            /* If this shader use full unified uniforms, the base offset is zero. */
            if (VIR_Shader_isFullUnifiedUniforms(pShader))
            {
                return errCode;
            }

            /* Check the uniform usage first. */
            VSC_CheckUniformUsage(pShader);

            errCode = VIR_Shader_CalcSamplerCount(pShader,
                                                  &samplerCount);
            ON_ERROR(errCode, "Calc sampler Count");

            if (samplerCount > (gctINT)pHwCfg->maxSamplerCountPerShader)
            {
                errCode = VSC_ERR_OUT_OF_RESOURCE;
                ON_ERROR(errCode, "Calc sampler Count");
            }

            VIR_Shader_SetPackUnifiedSampler(pShader, gcvTRUE);

            /*
            ** For those chips can support full unified uniform allocation,
            ** pack all shaders one by one so we can get the minimum used sampler.
            ** For those chips can not support full unified uniform allocation,
            ** pack all GPipe shaders one by one, and use the bottom sampler memory for GPipe.
            */
            if (pHwCfg->hwFeatureFlags.supportUnifiedSampler)
            {
                VIR_Shader_SetSamplerBaseOffset(pShader, maxSamplerCount);
            }
            else
            {
                if (VIR_Shader_IsGPipe(pShader))
                {
                    samplerBaseOffset -= samplerCount;
                    VIR_Shader_SetSamplerBaseOffset(pShader, samplerBaseOffset);
                }
                else
                {
                    VIR_Shader_SetSamplerBaseOffset(pShader, 0);
                }
            }

            maxSamplerCount += samplerCount;
        }
    }

    if (maxSamplerCount > (gctINT)pHwCfg->maxHwNativeTotalSamplerCount)
    {
        errCode = VSC_ERR_OUT_OF_RESOURCE;
        ON_ERROR(errCode, "Calc sampler Count");
    }

OnError:
    return errCode;
}

/* API level check for shaders:
   Geometry shader:
   It is a link-time error if not all provided sizes (sized input arrays and layout size) match in the geometry
   shader of a program.
*/
static VSC_ErrCode _APICheckShader(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                   VIR_Shader* pShader)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;

    if (pShader->shaderKind == VIR_SHADER_GEOMETRY)
    {
        gctINT inputVtxCount = 0;
        if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_POINTS)
        {
            inputVtxCount = 1;
        }
        else if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_LINES)
        {
            inputVtxCount = 2;
        }
        else if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_LINES_ADJACENCY)
        {
            inputVtxCount = 4;
        }
        else if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES)
        {
            inputVtxCount = 3;
        }
        else if (pShader->shaderLayout.geo.geoInPrimitive == VIR_GEO_TRIANGLES_ADJACENCY)
        {
            inputVtxCount = 6;
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        if (pShader->geoMaxAccessVertices > inputVtxCount)
        {
            errCode = VSC_ERR_INVALID_INDEX;
        }
    }

    return errCode;
}

static VSC_ErrCode _APICheckShaders(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode  errCode = VSC_ERR_NONE;
    VIR_Shader*  pCurStage;
    gctUINT      stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage= (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage)
        {
            errCode = _APICheckShader(&pPgLinkHelper->baseHelper, pCurStage);
            ON_ERROR(errCode, "API check fail");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _DoSecondStageOfLinkage(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_GPG_PASS_MANAGER*      pPgPassMnger = &pPgLinkHelper->pgPassMnger;

    /* It must be called after lower */
    vscPM_SetCurPassLevel(&pPgPassMnger->basePgmPM.basePM, VSC_PASS_LEVEL_MC);

    /* 0. individual shader API level check */
    errCode = _APICheckShaders(pPgLinkHelper);
    ON_ERROR(errCode, "Error in API check");

    /* 1. Calc IO hw component index among stages. Note that this is actually doing hw
          shader linkage, so the real HW shader linkage called before state-programming
          just does verification for this calcing when ISB is using USC */
    errCode = _CalcHwCompIndexForIOs(pPgLinkHelper);
    ON_ERROR(errCode, "Calc hw component index for IOs");

    /* 2. Program-level constant reg spillage needs full program info, so entry of this
          kind of reg spillage is put in link time */
    CALL_GPG_PASS(VSC_UF_CreateAUBO, 0, gcvNULL);

    /* 3. Allocate unified uniform. */
    CALL_GPG_PASS(VSC_UF_UnifiedUniformAlloc, 0, gcvNULL);

    /* 4. Calc the sampler base offset for unified sampler mode. */
    errCode = _CalcSamplerBaseOffset(pPgLinkHelper);
    ON_ERROR(errCode, "Calc the sampler base offset for unified sampler mode.");

OnError:
    return errCode;
}

static VSC_ErrCode _GeneratePepHLMappingTables(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper,
                                               PROGRAM_EXECUTABLE_PROFILE* pOutPEP)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_GPG_PASS_MANAGER*      pPgPassMnger = &pPgLinkHelper->pgPassMnger;
    VSC_PEP_GEN_PRIV_DATA      pepGenPrvData;

    vscPM_SetCurPassLevel(&pPgPassMnger->basePgmPM.basePM, VSC_PASS_LEVEL_CG);

    pepGenPrvData.pOutPEP = pOutPEP;
    if (pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENVK)
    {
        pepGenPrvData.client = PEP_CLIENT_VK;
    }
    else
    {
        gcmASSERT(pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL_ES11 ||
                  pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL_ES20 ||
                  pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL_ES30 ||
                  pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL_ES31 ||
                  pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL_ES32 ||
                  pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.ctx.clientAPI == gcvAPI_OPENGL);

        pepGenPrvData.client = PEP_CLIENT_GL;
    }

    CALL_GPG_PASS(vscVIR_GeneratePEP, 0, &pepGenPrvData);

OnError:
    return errCode;
}

static VSC_ErrCode _GenerateKepHLMappingTables(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                               KERNEL_EXECUTABLE_PROFILE* pOutKEP)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;

    CALL_SH_PASS(vscVIR_GenerateKEP, 0, pOutKEP);

OnError:
    return errCode;
}

static VSC_ErrCode _ValidateProgram(VSC_PROGRAM_LINKER_PARAM* pPgLinkParam,
                                    gctBOOL* pGfxOnlyProgram,
                                    gctBOOL* pComputeOnlyProgram,
                                    gctBOOL* pHasTsOrGs,
                                    gctUINT* pFirstValidStage)
{
    gctUINT                           stageIdx;
    gctBOOL                           bHasValidShader = gcvFALSE;
    gcSHADER                          validShader = gcvNULL;

    *pHasTsOrGs = gcvFALSE;
    *pFirstValidStage = VSC_MAX_SHADER_STAGE_COUNT;

    /* Must have shaders included */
    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            if (stageIdx == VSC_SHADER_STAGE_HS ||
                stageIdx == VSC_SHADER_STAGE_DS ||
                stageIdx == VSC_SHADER_STAGE_GS)
            {
                *pHasTsOrGs = gcvTRUE;
            }

            bHasValidShader = gcvTRUE;
            validShader = pPgLinkParam->hShaderArray[stageIdx];

            if (*pFirstValidStage == VSC_MAX_SHADER_STAGE_COUNT)
            {
                *pFirstValidStage = stageIdx;
            }
        }
    }

    if (!bHasValidShader)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    /* Compute and graphics can not be mixed together for un-seperated program */
    if (pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_CS] &&
        !(pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_SEPERATED_SHADERS))
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pPgLinkParam->hShaderArray[stageIdx])
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }
        }
    }

    /* For un-seperated gfx program, combination of shader stages must be legal, these legal cases are
       1. VS + (PS)
       2. VS + GS + (PS)
       3. VS + HS + DS + (PS)
       4. VS + HS + DS + GS + (PS)

       PS is optional is because of SO
    */
    if (!VIR_Shader_IsDesktopGL(validShader) &&
        !(pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_SEPERATED_SHADERS) &&
        !(pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_CS]))
    {
        if (!pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_VS])
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        if ((pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_HS] && !pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_DS]) ||
            (pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_DS] && !pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_HS]))
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }
    }

    *pGfxOnlyProgram = (pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_CS] == gcvNULL);
    *pComputeOnlyProgram = (pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_CS] &&
                            pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_VS] == gcvNULL &&
                            pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_HS] == gcvNULL &&
                            pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_DS] == gcvNULL &&
                            pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_GS] == gcvNULL &&
                            pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_PS] == gcvNULL);

    return VSC_ERR_NONE;
}

static gctBOOL _InitializeShResLayout(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                      VSC_PROGRAM_RESOURCE_LAYOUT* pPgResourceLayout,
                                      gctUINT shStage,
                                      VSC_SHADER_RESOURCE_LAYOUT* pOutShResourceLayout)
{
    gctUINT  i, j, resBindingCount = 0, pushCnstRangeCount = 0;

    gcmASSERT (pPgResourceLayout);

    /* Res binding */
    if (pPgResourceLayout->resourceSetCount)
    {
        for (i = 0; i < pPgResourceLayout->resourceSetCount; i ++)
        {
            for (j = 0; j < pPgResourceLayout->pResourceSets[i].resourceBindingCount; j ++)
            {
                if (pPgResourceLayout->pResourceSets[i].pResouceBindings[j].stageBits &
                    VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                {
                    resBindingCount ++;
                }
            }
        }

        if (resBindingCount)
        {
            pOutShResourceLayout->resourceBindingCount = resBindingCount;
            pOutShResourceLayout->pResBindings = (VSC_SHADER_RESOURCE_BINDING*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                      resBindingCount*sizeof(VSC_SHADER_RESOURCE_BINDING));

            resBindingCount = 0;
            for (i = 0; i < pPgResourceLayout->resourceSetCount; i ++)
            {
                for (j = 0; j < pPgResourceLayout->pResourceSets[i].resourceBindingCount; j ++)
                {
                    if (pPgResourceLayout->pResourceSets[i].pResouceBindings[j].stageBits &
                        VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                    {
                        memcpy(&pOutShResourceLayout->pResBindings[resBindingCount],
                               &pPgResourceLayout->pResourceSets[i].pResouceBindings[j].shResBinding,
                               sizeof(VSC_SHADER_RESOURCE_BINDING));

                        resBindingCount ++;
                    }
                }
            }
        }
    }

    /* Push-constant */
    if (pPgResourceLayout->pushConstantRangeCount)
    {
        for (i = 0; i < pPgResourceLayout->pushConstantRangeCount; i ++)
        {
            if (pPgResourceLayout->pPushConstantRanges[i].stageBits &
                VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
            {
                pushCnstRangeCount ++;
            }
        }

        if (pushCnstRangeCount)
        {
            pOutShResourceLayout->pushConstantRangeCount = pushCnstRangeCount;
            pOutShResourceLayout->pPushConstantRanges = (VSC_SHADER_PUSH_CONSTANT_RANGE*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                    pushCnstRangeCount*sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));

            pushCnstRangeCount = 0;
            for (i = 0; i < pPgResourceLayout->pushConstantRangeCount; i ++)
            {
                if (pPgResourceLayout->pPushConstantRanges[i].stageBits &
                    VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                {
                    memcpy(&pOutShResourceLayout->pPushConstantRanges[pushCnstRangeCount],
                           &pPgResourceLayout->pPushConstantRanges[i],
                           sizeof(VSC_SHADER_PUSH_CONSTANT_RANGE));

                    pushCnstRangeCount ++;
                }
            }
        }
    }

    return ((resBindingCount > 0) || (pushCnstRangeCount > 0));
}

static void _FinalizeShResLayout(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                 VSC_SHADER_RESOURCE_LAYOUT* pShResourceLayout)
{
    if (pShResourceLayout == gcvNULL)
    {
        return;
    }

    if (pShResourceLayout->pPushConstantRanges)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pShResourceLayout->pPushConstantRanges);
    }

    if (pShResourceLayout->pResBindings)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pShResourceLayout->pResBindings);
    }
}

/* Defined in drvi_compile.c */
extern VSC_ErrCode _CompileShaderInternal(VSC_SHADER_PASS_MANAGER*   pShPassMnger,
                                          SHADER_EXECUTABLE_PROFILE* pOutSEP,
                                          gctBOOL                    bSkipPepGen);
extern gctUINT _GetCompLevelFromExpectedShaderLevel(VIR_ShLevel expectedShLevel);
extern VIR_ShLevel _GetExpectedLastLevel(VSC_SHADER_COMPILER_PARAM* pCompilerParam);

static gctBOOL _InitializeShLibLinkTable(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                         gctBOOL bGfxOnlyProgram,
                                         VSC_PROG_LIB_LINK_TABLE* pPgLibLinkTable,
                                         VIR_ShLevel maxShLevelAmongLinkLibs,
                                         gctUINT shStage,
                                         VIR_ShLevel* pTrueLibShLevel,
                                         VSC_SHADER_LIB_LINK_TABLE* pOutShLibLinkTable)
{
    gctUINT                     i, libLinkEntryCount = 0;
    VSC_SHADER_COMPILER_PARAM   compParam;
    VSC_PROGRAM_LINKER_HELPER*  pPgLinkHelper = (VSC_PROGRAM_LINKER_HELPER*)pBaseLinkHelper;
    VIR_ShLevel                 thisShLevel, expectedShLevel;
    gctBOOL                     bSeperatedShaders = (pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg.cFlags &
                                                     VSC_COMPILER_FLAG_SEPERATED_SHADERS);

    gcmASSERT (pPgLibLinkTable);

    memset(&compParam, 0, sizeof(VSC_SHADER_COMPILER_PARAM));

    if (pPgLibLinkTable->progLinkEntryCount)
    {
        for (i = 0; i < pPgLibLinkTable->progLinkEntryCount; i ++)
        {
            thisShLevel = (VIR_ShLevel)pPgLibLinkTable->pProgLibLinkEntries[i].shLibLinkEntry.applyLevel;

            expectedShLevel = vscMAX(thisShLevel, maxShLevelAmongLinkLibs);

            /* We need make sure lib is not at high-level because we will do lib
               link at ML or below level (this is then because main shader in program will be performed API
               spec check at high-level at first linkage stage, while lib linked main shader will break such
               check) */
             if (!bSeperatedShaders && bGfxOnlyProgram)
             {
                if (gcUseFullNewLinker(pBaseLinkHelper->pHwCfg->hwFeatureFlags.hasHalti2))
                {
                    expectedShLevel = vscMAX(expectedShLevel, VIR_SHLEVEL_Pre_Medium);
                }
             }

            /* We need make sure shader level of lib is not smaller than start shader level of main shader */
            expectedShLevel = vscMAX(expectedShLevel,
                              VIR_Shader_GetLevel(((VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[shStage])));

            if (thisShLevel < expectedShLevel
                &&
                pPgLibLinkTable->pProgLibLinkEntries[i].shLibLinkEntry.hShaderLib)
            {
                compParam.hShader = pPgLibLinkTable->pProgLibLinkEntries[i].shLibLinkEntry.hShaderLib;
                memcpy(&compParam.cfg, &pPgLinkHelper->pgPassMnger.pPgmLinkerParam->cfg, sizeof(VSC_COMPILER_CONFIG));
                compParam.cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_HL |
                                          VSC_COMPILER_FLAG_COMPILE_TO_ML |
                                          VSC_COMPILER_FLAG_COMPILE_TO_LL |
                                          VSC_COMPILER_FLAG_COMPILE_TO_MC);
                compParam.cfg.cFlags |= _GetCompLevelFromExpectedShaderLevel(expectedShLevel);

                if (vscCompileShader(&compParam, gcvNULL) != gcvSTATUS_OK)
                {
                    return gcvFALSE;
                }

                *pTrueLibShLevel = _GetExpectedLastLevel(&compParam);
            }
            else
            {
                *pTrueLibShLevel = thisShLevel;
            }

            if (pPgLibLinkTable->pProgLibLinkEntries[i].mainShaderStageBits & VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
            {
                libLinkEntryCount ++;
            }
        }

        if (libLinkEntryCount)
        {
            pOutShLibLinkTable->shLinkEntryCount = libLinkEntryCount;
            pOutShLibLinkTable->pShLibLinkEntries = (VSC_SHADER_LIB_LINK_ENTRY*)vscMM_Alloc(pBaseLinkHelper->pMM,
                                                                      libLinkEntryCount*sizeof(VSC_SHADER_LIB_LINK_ENTRY));

            libLinkEntryCount = 0;
            for (i = 0; i < pPgLibLinkTable->progLinkEntryCount; i ++)
            {
                if (pPgLibLinkTable->pProgLibLinkEntries[i].mainShaderStageBits & VSC_SHADER_STAGE_2_STAGE_BIT(shStage))
                {
                    memcpy(&pOutShLibLinkTable->pShLibLinkEntries[libLinkEntryCount],
                           &pPgLibLinkTable->pProgLibLinkEntries[i].shLibLinkEntry,
                           sizeof(VSC_SHADER_LIB_LINK_ENTRY));

                    libLinkEntryCount ++;
                }
            }
        }
    }

    return (libLinkEntryCount > 0);
}

static void _FinalizeShLibLinkTable(VSC_BASE_LINKER_HELPER* pBaseLinkHelper,
                                    VSC_SHADER_LIB_LINK_TABLE* pShLibLinkTable)
{
    if (pShLibLinkTable == gcvNULL)
    {
        return;
    }

    if (pShLibLinkTable->pShLibLinkEntries)
    {
        vscMM_Free(pBaseLinkHelper->pMM, pShLibLinkTable->pShLibLinkEntries);
    }
}

static VIR_ShLevel _GetMaximumShaderLevelAmongLinkLibs(VSC_PROG_LIB_LINK_TABLE* pPgLibLinkTable)
{
    gctUINT                     i;
    VIR_ShLevel                 thisShLevel, maxShLevel = VIR_SHLEVEL_Unknown;

    for (i = 0; i < pPgLibLinkTable->progLinkEntryCount; i ++)
    {
        thisShLevel = (VIR_ShLevel)pPgLibLinkTable->pProgLibLinkEntries[i].shLibLinkEntry.applyLevel;

        if (thisShLevel > maxShLevel)
        {
            maxShLevel = thisShLevel;
        }
    }

    return maxShLevel;
}

/* check execution mode value of tcs and tes and copy setting from tcs to tes if needed*/
static VSC_ErrCode
_CheckAndUnifiedTessExecutionMode(VSC_PROGRAM_LINKER_HELPER* pPgLinkHelper)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pPreStage = gcvNULL;
    VIR_Shader*                pCurStage;
    gctUINT                    stageIdx;

    for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        pCurStage= (VIR_Shader*)pPgLinkHelper->pgPassMnger.pPgmLinkerParam->hShaderArray[stageIdx];

        if (pCurStage && pCurStage->shaderKind == VIR_SHADER_TESSELLATION_EVALUATION)
        {
            if (pPreStage && pPreStage->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
            {
                /* check tessPrimitiveMode, if value in TES is undefined, copy value from TCS */
                gcmASSERT(pPreStage->shaderLayout.tcs.tessPrimitiveMode != VIR_TESS_PMODE_UNDEFINED ||
                          pCurStage->shaderLayout.tes.tessPrimitiveMode != VIR_TESS_PMODE_UNDEFINED);
                if ((pPreStage->shaderLayout.tcs.tessPrimitiveMode != pCurStage->shaderLayout.tes.tessPrimitiveMode) &&
                    (pCurStage->shaderLayout.tes.tessPrimitiveMode == VIR_TESS_PMODE_UNDEFINED))
                {
                    pCurStage->shaderLayout.tes.tessPrimitiveMode = pPreStage->shaderLayout.tcs.tessPrimitiveMode;
                }

                /* check tessOrdering, if value in TES is undefined, copy value from TCS except in ISOLINE mode */
                gcmASSERT((pPreStage->shaderLayout.tcs.tessPrimitiveMode == VIR_TESS_PMODE_ISOLINE) ||
                          (pPreStage->shaderLayout.tcs.tessOrdering != VIR_TESS_ORDER_UNDEFINED) ||
                          (pCurStage->shaderLayout.tes.tessOrdering != VIR_TESS_ORDER_UNDEFINED));
                if ((pPreStage->shaderLayout.tcs.tessOrdering != pCurStage->shaderLayout.tes.tessOrdering) &&
                    (pCurStage->shaderLayout.tes.tessOrdering == VIR_TESS_ORDER_UNDEFINED))
                {
                    /* copy setting of tcs to tes */
                    pCurStage->shaderLayout.tes.tessOrdering = pPreStage->shaderLayout.tcs.tessOrdering;
                }

                /* check tessVertexSpacing */
                gcmASSERT((pPreStage->shaderLayout.tcs.tessVertexSpacing != VIR_TESS_SPACING_UNDEFINED) ||
                          (pCurStage->shaderLayout.tes.tessVertexSpacing != VIR_TESS_SPACING_UNDEFINED));
                if ((pPreStage->shaderLayout.tcs.tessVertexSpacing != pCurStage->shaderLayout.tes.tessVertexSpacing) &&
                    (pCurStage->shaderLayout.tes.tessVertexSpacing == VIR_TESS_SPACING_UNDEFINED))
                {
                    pCurStage->shaderLayout.tes.tessVertexSpacing = pPreStage->shaderLayout.tcs.tessVertexSpacing;
                }

                /* check pointMode enabled in either tess stage */
                if (pPreStage->shaderLayout.tcs.tessPointMode)
                {
                    pCurStage->shaderLayout.tes.tessPointMode = pPreStage->shaderLayout.tcs.tessPointMode;
                }

            }
            break;
        }
        pPreStage = pCurStage;
    }
    return errCode;
}

gceSTATUS vscLinkProgram(VSC_PROGRAM_LINKER_PARAM* pPgLinkParam,
                         PROGRAM_EXECUTABLE_PROFILE* pOutPEP,
                         VSC_HW_PIPELINE_SHADERS_STATES* pOutPgStates)
{
    gceSTATUS                     status = gcvSTATUS_OK;
    VSC_ErrCode                   errCode = VSC_ERR_NONE;
    VSC_PROGRAM_LINKER_HELPER     pgLinkHelper;
    VSC_HW_PIPELINE_SHADERS_PARAM hwPipelineShsParam;
    PROGRAM_EXECUTABLE_PROFILE*   pPEP = gcvNULL;
    SHADER_EXECUTABLE_PROFILE*    pSEPToGen;
    VSC_SHADER_COMPILER_PARAM     shCompParamArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_RESOURCE_LAYOUT    shResLayoutArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_LIB_LINK_TABLE     shLibLinkTableArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_LIB_LINK_TABLE*    pShLibLinkTablePointerArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_PASS_MANAGER       shPassMngerArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_SHADER_PASS_RES*          pShPassResArray[VSC_MAX_SHADER_STAGE_COUNT];
    gctBOOL                       bNeedSSL = gcvFALSE, bSkipSepGen = gcvFALSE;
    gctBOOL                       bComputeOnlyProgram, bGfxOnlyProgram;
    gctBOOL                       bInternalPEP = gcvFALSE;
    gctUINT                       stageIdx, processedStageCount, firstValidStage = VSC_MAX_SHADER_STAGE_COUNT;
    gctINT                        sStageIdx;
    gctBOOL                       bSeperatedShaders = (pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_SEPERATED_SHADERS);
    gctBOOL                       bHasTsOrGs = gcvFALSE, bNeedActiveIo = gcvFALSE;
    VSC_OPTN_Options              options;
    char                          buffer[4096];
    VIR_Dumper                    dumper;
    VIR_Shader*                   pShader;
    VIR_ShLevel                   maxShLevelAmongLinkLibs = VIR_SHLEVEL_Unknown, finalTrueLibShLevel = VIR_SHLEVEL_Unknown;
    VIR_ShLevel                   trueLibShLevel = VIR_SHLEVEL_Unknown;

    gcmASSERT(pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_MC);

    if (pOutPEP || pOutPgStates)
    {
        gcmASSERT(pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_CODE_GEN);
    }

    memset(&hwPipelineShsParam, 0, sizeof(VSC_HW_PIPELINE_SHADERS_PARAM));
    memset(&shCompParamArray[0], 0, sizeof(VSC_SHADER_COMPILER_PARAM)*VSC_MAX_SHADER_STAGE_COUNT);
    memset(&shResLayoutArray[0], 0, sizeof(VSC_SHADER_RESOURCE_LAYOUT)*VSC_MAX_SHADER_STAGE_COUNT);
    memset(&shPassMngerArray[0], 0, sizeof(VSC_SHADER_PASS_MANAGER)*VSC_MAX_SHADER_STAGE_COUNT);
    memset(&pShPassResArray[0], 0, sizeof(VSC_SHADER_PASS_RES*)*VSC_MAX_SHADER_STAGE_COUNT);
    memset(&shLibLinkTableArray[0], 0, sizeof(VSC_SHADER_LIB_LINK_TABLE)*VSC_MAX_SHADER_STAGE_COUNT);
    memset(&pShLibLinkTablePointerArray[0], 0, sizeof(VSC_SHADER_LIB_LINK_TABLE*)*VSC_MAX_SHADER_STAGE_COUNT);

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    vscInitializeOptions(&options,
                         &pPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                         pPgLinkParam->cfg.cFlags,
                         pPgLinkParam->cfg.optFlags);

    vscGPPM_Initialize(&pgLinkHelper.pgPassMnger, pPgLinkParam, &dumper, &options, VSC_PM_MODE_SEMI_AUTO);

    pgLinkHelper.baseHelper.pHwCfg = &pPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    pgLinkHelper.baseHelper.pMM = &pgLinkHelper.pgPassMnger.basePgmPM.pgMmPool.sharedPMP.mmWrapper;

    /* Parameter legality check */
    pPEP = pOutPEP;
    if (pPEP == gcvNULL)
    {
        /* !!! We hit here ONLY this func is called by old gcLinkShaders, NORMALLY we CANNOT hit here !!! */

        if (pOutPgStates != gcvNULL)
        {
            pPEP = (PROGRAM_EXECUTABLE_PROFILE*)vscMM_Alloc(pgLinkHelper.baseHelper.pMM,
                                                            sizeof(PROGRAM_EXECUTABLE_PROFILE));
            bInternalPEP = gcvTRUE;
        }
        else
        {
            /* We are here when new codegen is not ready, so we still need old linker generate states.
               If so we dont need gen SEP as we dont generate states */
            bSkipSepGen = gcvTRUE;
        }
    }

    if (pPEP)
    {
        gcmONERROR(vscInitializePEP(pPEP));
    }

    errCode = _ValidateProgram(pPgLinkParam, &bGfxOnlyProgram, &bComputeOnlyProgram, &bHasTsOrGs, &firstValidStage);
    ON_ERROR(errCode, "Check program validation");

    if (pPgLinkParam->pProgLibLinkTable)
    {
        maxShLevelAmongLinkLibs = _GetMaximumShaderLevelAmongLinkLibs(pPgLinkParam->pProgLibLinkTable);
    }

    /* Initialize our shader pass manager who will take over all passes
       whether to trigger or not */
    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg = pPgLinkParam->cfg;
            shCompParamArray[stageIdx].hShader = pPgLinkParam->hShaderArray[stageIdx];

            if (pPgLinkParam->pPgResourceLayout)
            {
                if (_InitializeShResLayout(&pgLinkHelper.baseHelper, pPgLinkParam->pPgResourceLayout,
                                           stageIdx, &shResLayoutArray[stageIdx]))
                {
                    shCompParamArray[stageIdx].pShResourceLayout = &shResLayoutArray[stageIdx];
                }
            }

            if (pPgLinkParam->pProgLibLinkTable)
            {
                if (_InitializeShLibLinkTable(&pgLinkHelper.baseHelper, bGfxOnlyProgram, pPgLinkParam->pProgLibLinkTable,
                                              maxShLevelAmongLinkLibs, stageIdx, &trueLibShLevel,
                                              &shLibLinkTableArray[stageIdx]))
                {
                    pShLibLinkTablePointerArray[stageIdx] = &shLibLinkTableArray[stageIdx];

                    if (finalTrueLibShLevel < trueLibShLevel)
                    {
                        finalTrueLibShLevel = trueLibShLevel;
                    }
                }
            }

            pShader = (VIR_Shader*)shCompParamArray[stageIdx].hShader;

            if (bSeperatedShaders)
            {
                VIR_Shader_SetFlag(pShader, VIR_SHFLAG_SEPARATED);
            }
            else if (stageIdx == VSC_SHADER_STAGE_DS)
            {
                gcmASSERT(shCompParamArray[VSC_SHADER_STAGE_HS].hShader);
                pShader->shaderLayout.tes.tessPatchInputVertices =
                    ((VIR_Shader*)shCompParamArray[VSC_SHADER_STAGE_HS].hShader)->shaderLayout.tcs.tcsPatchOutputVertices;
            }

            vscSPM_Initialize(&shPassMngerArray[stageIdx],
                              &shCompParamArray[stageIdx],
                              &pgLinkHelper.pgPassMnger.basePgmPM.shMmPool,
                              gcvFALSE,
                              pShader->dumper,
                              &options,
                              VSC_PM_MODE_SEMI_AUTO);

            pShPassResArray[stageIdx] = &shPassMngerArray[stageIdx].passRes;
        }
    }

    /* Pass-resources of graphics-program pass manager are pointed to counterpart of each shader pass manager */
    vscGPPM_SetPassRes(&pgLinkHelper.pgPassMnger, pShPassResArray);

    bNeedActiveIo = (VSC_OPTN_FAIOOptions_GetSwitchOn(VSC_OPTN_Options_GetFAIOOptions(&options, 0)) || bHasTsOrGs);

    /* check Tessellation execution mode of tcs and tes, if execution mode is set on tcs, copy to tes */
    errCode = _CheckAndUnifiedTessExecutionMode(&pgLinkHelper);


    /* 1. At first fork link process, only do HL level compiling */

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg.cFlags = pPgLinkParam->cfg.cFlags;
            shCompParamArray[stageIdx].cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_ML |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_MC |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_LL |
                                                       VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

            if (pShLibLinkTablePointerArray[stageIdx] &&
                (finalTrueLibShLevel == VIR_SHLEVEL_Pre_High ||
                 finalTrueLibShLevel == VIR_SHLEVEL_Post_High))
            {
                shCompParamArray[stageIdx].pShLibLinkTable = pShLibLinkTablePointerArray[stageIdx];
            }
            else
            {
                shCompParamArray[stageIdx].pShLibLinkTable = gcvNULL;
            }

            errCode = _CompileShaderInternal(&shPassMngerArray[stageIdx], gcvNULL, gcvTRUE);
            ON_ERROR(errCode, "Compiler internal");
        }
    }

    /* 2. At 1st join stage (api-spec-check-stage), all I/Os will be checked, for redundant I/O,
          they will be marked, so later DCE can remove them; Note for this api-spec-check-stage,
          the reason why we separatedly put it aside just after HL because we hope we can check
          api level legality as early as possible.

          For seperated case, directly use app's location as io slot.

          Also others that need to be considered from perspective of GL API view are considered. */

    if (!bSeperatedShaders && bGfxOnlyProgram)
    {
        if (gcUseFullNewLinker(pPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
        {
            /* For recompilation, we dont need do api-spec check as it has been done when linking
               master program */
            if (pPgLinkParam->pInMasterPEP == gcvNULL)
            {
                errCode = _DoFirstStageOfLinkage(&pgLinkHelper, FSL_STAGE_API_SPEC_CHECK);
                ON_ERROR(errCode, "First stage of linkage");
            }
        }

        /*
           Conditions to need us do 2nd level shaders linkage
           a. Constant registers must be shared by program for unified constant register file, or
           b. Sampler registers must be shared by program for unified constant register file, or
           c. Constant register can be spillable to memory (for unseperated program, such memory
              is shared by whole program).
           d. Shaders need generate active IO (using explicit USC load_attr/store_attr), which needs
              calc hw component index for RA
        */
        bNeedSSL = (pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_UNI_UNIFORM_UNIFIED_ALLOC ||
                    pPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC ||
                    VSC_OPTN_UF_AUBOOptions_GetSwitchOn(VSC_OPTN_Options_GetAUBOOptions(&options, 0)) || bNeedActiveIo);
    }
    else
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            pShader = (VIR_Shader*)pPgLinkParam->hShaderArray[stageIdx];

            if (pShader)
            {
                /* Aliased location check */

                errCode = _CheckInputAliasedLocation(&pgLinkHelper.baseHelper, pShader);
                ON_ERROR(errCode, "Check input aliased location");

                errCode = _CheckOutputAliasedLocation(&pgLinkHelper.baseHelper, pShader);
                ON_ERROR(errCode, "Check output aliased location");

                /* LL-slot calculation */

                errCode = _CalcInputLowLevelSlot(&pgLinkHelper.baseHelper, pShader, gcvTRUE);
                ON_ERROR(errCode, "CalcInputLowLevelSlot of linkage");

                errCode = _CalcOutputLowLevelSlot(&pgLinkHelper.baseHelper, pShader, gcvTRUE);
                ON_ERROR(errCode, "CalcOutputLowLevelSlot of linkage");
            }
        }

        if (bComputeOnlyProgram)
        {
            /*
               Conditions to need us do 2nd level shaders linkage
               a. Constant register can be spillable to memory.
            */
            bNeedSSL = VSC_OPTN_UF_AUBOOptions_GetSwitchOn(VSC_OPTN_Options_GetAUBOOptions(&options, 0));
        }
    }

    /* 3. At second fork link process, only do ML level compiling */

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg.cFlags = pPgLinkParam->cfg.cFlags;
            shCompParamArray[stageIdx].cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_HL |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_MC |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_LL |
                                                       VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

            if (pShLibLinkTablePointerArray[stageIdx] &&
                (finalTrueLibShLevel == VIR_SHLEVEL_Pre_Medium ||
                 finalTrueLibShLevel == VIR_SHLEVEL_Post_Medium))
            {
                shCompParamArray[stageIdx].pShLibLinkTable = pShLibLinkTablePointerArray[stageIdx];
            }
            else
            {
                shCompParamArray[stageIdx].pShLibLinkTable = gcvNULL;
            }

            errCode = _CompileShaderInternal(&shPassMngerArray[stageIdx], gcvNULL, gcvTRUE);
            ON_ERROR(errCode, "Compiler internal");
        }
    }

    /* 4. At 1st join stage (ll-slot-calc-stage), after ML level compiling, more inputs of shader
          might be marked as redundant, so all I/Os will be checked again, for redundant I/O, they
          will be marked, so later DCE can remove them. Meanwhile, io slot is calc'ed by ourselves.
          Note that before ll-slot-calc-stage, several api-spec-check-stage might be called to insure
          only last stage of 1st stage of linkage is to calc ll slot (ll-slot-calc-stage).
    */

    if (!bSeperatedShaders && bGfxOnlyProgram)
    {
        if (gcUseFullNewLinker(pPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
        {
            /* At least do one pass of ll-slot-calc-stage of 1st join stage because previous api-spec-check-stage
               does not calc ll slot for IO. */
            processedStageCount = 0;
            for (sStageIdx = VSC_MAX_SHADER_STAGE_COUNT - 1; sStageIdx >= VSC_SHADER_STAGE_VS; sStageIdx --)
            {
                if (pPgLinkParam->hShaderArray[sStageIdx])
                {
                    errCode = _DoFirstStageOfLinkage(&pgLinkHelper, FSL_STAGE_INTERMEDIUM);
                    ON_ERROR(errCode, "First stage of linkage");

                    /* If DCE is not required, just bail out */
                    if (!(pPgLinkParam->cfg.optFlags | VSC_COMPILER_OPT_DCE))
                    {
                        break;
                    }

                    processedStageCount ++;

                    /* DCE for last 2 valid stages have been executed in previous fork processes, and their
                       inputs won't be changed anymore */
                    if (processedStageCount <= 2)
                    {
                        continue;
                    }

                    /* Note that we're still under ML level compiling because last fork process is on ML level,
                       so we revert post-ML to pre-ML level and do only DCE */
                    shCompParamArray[sStageIdx].cfg.optFlags = VSC_COMPILER_OPT_DCE;
                    VIR_Shader_SetLevel((VIR_Shader*)shCompParamArray[sStageIdx].hShader, VIR_SHLEVEL_Pre_Medium);
                    shCompParamArray[sStageIdx].pShLibLinkTable = gcvNULL;
                    errCode = _CompileShaderInternal(&shPassMngerArray[sStageIdx], gcvNULL, gcvTRUE);
                    ON_ERROR(errCode, "Compiler internal");

                    /* Restore the opt flags */
                    shCompParamArray[sStageIdx].cfg.optFlags = pPgLinkParam->cfg.optFlags;
                }
            }

            errCode = _DoIoComponentPackAmongShaderStages(&pgLinkHelper);
            ON_ERROR(errCode, "IO component packing. ");

            if (pPgLinkParam->cfg.optFlags | VSC_COMPILER_OPT_VEC)
            {
                errCode = _DoFirstStageOfLinkage(&pgLinkHelper, FSL_STAGE_IO_VECTORIZATION);
                ON_ERROR(errCode, "First stage of linkage");
            }

            errCode = _DoFirstStageOfLinkage(&pgLinkHelper, FSL_STAGE_LL_SLOT_CALC);
            ON_ERROR(errCode, "First stage of linkage");
        }
    }

    /* 5. At third fork link process, only do LL level compiling */

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg.cFlags = pPgLinkParam->cfg.cFlags;
            shCompParamArray[stageIdx].cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_HL |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_ML |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_MC |
                                                       VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

            if (pShLibLinkTablePointerArray[stageIdx] &&
                (finalTrueLibShLevel == VIR_SHLEVEL_Pre_Low ||
                 finalTrueLibShLevel == VIR_SHLEVEL_Post_Low))
            {
                shCompParamArray[stageIdx].pShLibLinkTable = pShLibLinkTablePointerArray[stageIdx];
            }
            else
            {
                shCompParamArray[stageIdx].pShLibLinkTable = gcvNULL;
            }

            errCode = _CompileShaderInternal(&shPassMngerArray[stageIdx], gcvNULL, gcvTRUE);
            ON_ERROR(errCode, "Compiler internal");
        }
    }

    /* 6. At fouth fork link process, only do MC level compiling, but exclude codegen pass */

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg.cFlags = pPgLinkParam->cfg.cFlags;
            shCompParamArray[stageIdx].cfg.cFlags &= ~(VSC_COMPILER_FLAG_COMPILE_TO_HL |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_ML |
                                                       VSC_COMPILER_FLAG_COMPILE_TO_LL |
                                                       VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

            if (pShLibLinkTablePointerArray[stageIdx] &&
                (finalTrueLibShLevel == VIR_SHLEVEL_Pre_Machine ||
                 finalTrueLibShLevel == VIR_SHLEVEL_Post_Machine))
            {
                shCompParamArray[stageIdx].pShLibLinkTable = pShLibLinkTablePointerArray[stageIdx];
            }
            else
            {
                shCompParamArray[stageIdx].pShLibLinkTable = gcvNULL;
            }

            errCode = _CompileShaderInternal(&shPassMngerArray[stageIdx], gcvNULL, gcvTRUE);
            ON_ERROR(errCode, "Compiler internal");
        }
    }

    /* 7. At 2nd join stage, HW level resources that are shared by all shader stages are
          considered. This must be just before forked codegen pass. Actually, this should
          be considered as part of codegen because it will allocate shared HW resources.
          Implementation should call common codegen functions to make code clear.
    */

    if (bNeedSSL)
    {
        errCode = _DoSecondStageOfLinkage(&pgLinkHelper);
        ON_ERROR(errCode, "Second stage of linkage");
    }
    else if (bNeedActiveIo)
    {
        /* For separated shaders, because of following 2 reasons, all separated shaders will
           call vscLinkProgram again from driver side when pipeline-program is deployed by
           regarding the pipeline program as non-separated. So some of hwcompindex calc'ed
           here might be not HW correct because we have no linked shader info !!!!

           1. hw arch is not friendly for some builtins, such point/pointsize/layer/...
           2. SSO can be linked by 'name' at draw time, not totally 'location', but meanwhile
              HW arch does not provide remap registers from 'name' to hw-comp-index
        */

        for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pPgLinkParam->hShaderArray[stageIdx])
            {
                _CalcInputHwCompIndex(&pgLinkHelper.baseHelper, pPgLinkParam->hShaderArray[stageIdx]);
                _CalcOutputHwCompIndex(&pgLinkHelper.baseHelper, pPgLinkParam->hShaderArray[stageIdx]);
            }
        }
    }

    /* 8. Fork again on post-MC to do codegen */

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            shCompParamArray[stageIdx].cfg.cFlags = pPgLinkParam->cfg.cFlags;
            shCompParamArray[stageIdx].cfg.cFlags &= ~VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS;

            pSEPToGen = bSkipSepGen ? gcvNULL : &pPEP->seps[stageIdx];

            shCompParamArray[stageIdx].pShLibLinkTable = gcvNULL;

            errCode = _CompileShaderInternal(&shPassMngerArray[stageIdx], pSEPToGen, (pOutPEP == gcvNULL));
            ON_ERROR(errCode, "Compiler internal");
        }
    }

    /* 9. Generate HL mapping tables */

    if (pOutPEP)
    {
        errCode = _GeneratePepHLMappingTables(&pgLinkHelper, pOutPEP);
        ON_ERROR(errCode, "Generate HL mapping tables");
    }

    /* 10. Now we can do HW level linkage and states programming if we have generated SEPs */

    if (
#if !PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
        !bSeperatedShaders &&
#endif
        pOutPgStates)
    {
        if (bGfxOnlyProgram)
        {
            for (stageIdx = 0; stageIdx < VSC_MAX_GFX_SHADER_STAGE_COUNT; stageIdx ++)
            {
                hwPipelineShsParam.pSEPArray[stageIdx] =
                    (pPgLinkParam->hShaderArray[stageIdx]) ? &pPEP->seps[stageIdx] : gcvNULL;
            }
        }
        else
        {
            hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_VS] = gcvNULL;
            hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_HS] = gcvNULL;
            hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_DS] = gcvNULL;
            hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_GS] = gcvNULL;
            hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_PS] = gcvNULL;
            hwPipelineShsParam.pSEPArray[VSC_CPT_SHADER_STAGE_CS] =
                (pPgLinkParam->hShaderArray[VSC_SHADER_STAGE_CS]) ? &pPEP->seps[VSC_SHADER_STAGE_CS] : gcvNULL;
        }

        hwPipelineShsParam.pSysCtx = pPgLinkParam->cfg.ctx.pSysCtx;

        gcmONERROR(vscProgramHwShaderStages(&hwPipelineShsParam, pOutPgStates, bSeperatedShaders));
    }

OnError:
    /* Finalize PEP only if it is an internal-generated PEP. */
    if (bInternalPEP)
    {
        gcmASSERT(pPEP);
        vscFinalizePEP(pPEP);
    }

    for (stageIdx = 0; stageIdx < VSC_MAX_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pPgLinkParam->hShaderArray[stageIdx])
        {
            vscSPM_Finalize(&shPassMngerArray[stageIdx], gcvFALSE);
            _FinalizeShResLayout(&pgLinkHelper.baseHelper, &shResLayoutArray[stageIdx]);
            _FinalizeShLibLinkTable(&pgLinkHelper.baseHelper, &shLibLinkTableArray[stageIdx]);
        }
    }

    vscFinalizeOptions(&options);
    vscGPPM_Finalize(&pgLinkHelper.pgPassMnger);

    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

static VSC_ErrCode _ConvertCompileFlags(VSC_SHADER_PASS_MANAGER*        pShPassMnger,
                                        VIR_Shader*                     pKernel
                                        )
{
    VSC_ErrCode                       errCode = VSC_ERR_NONE;

    /* Check if we can use VSC_ImageDesc. */
    if (pShPassMnger->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_USE_VSC_IMAGE_DESC)
    {
        VIR_Shader_SetFlag(pKernel, VIR_SHFLAG_USE_VSC_IMAGE_DESC);
    }

    return errCode;
}

VSC_ErrCode _CreateKernelInternal(VSC_SHADER_PASS_MANAGER*        pShPassMnger,
                                  KERNEL_EXECUTABLE_PROFILE*      pOutKEP,
                                  VSC_HW_PIPELINE_SHADERS_STATES* pOutKrnlStates
                                  )
{
    gceSTATUS                         status = gcvSTATUS_OK;
    VSC_ErrCode                       errCode = VSC_ERR_NONE;
    VSC_HW_PIPELINE_SHADERS_PARAM     hwPipelineShsParam;
    KERNEL_EXECUTABLE_PROFILE*        pKEP = gcvNULL;
    gctBOOL                           bSkipSepGen = gcvFALSE;
    VSC_BASE_LINKER_HELPER            baseHelper;
    VIR_Shader*                       pKernel = (VIR_Shader*)pShPassMnger->pCompilerParam->hShader;

    gcmASSERT(pKernel);
    gcmASSERT(pShPassMnger->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_MC);
    gcmASSERT(pShPassMnger->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

    memset(&hwPipelineShsParam, 0, sizeof(VSC_HW_PIPELINE_SHADERS_PARAM));

    baseHelper.pHwCfg = &pShPassMnger->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    baseHelper.pMM = &pShPassMnger->pMmPool->sharedPMP.mmWrapper;

    /* Parameter legality check */
    pKEP = pOutKEP;
    if (pKEP == gcvNULL)
    {
        /* !!! We hit here ONLY this func is called by old gcLinkShaders, NORMALLY we CANNOT hit here !!! */

        if (pOutKrnlStates != gcvNULL)
        {
            pKEP = (KERNEL_EXECUTABLE_PROFILE*)vscMM_Alloc(baseHelper.pMM,
                                                           sizeof(KERNEL_EXECUTABLE_PROFILE));
        }
        else
        {
            /* We are here when new codegen is not ready, so we still need old linker generate states.
               If so we dont need gen SEP as we dont generate states */
            bSkipSepGen = gcvTRUE;
        }
    }

    if (pKEP)
    {
        gcmONERROR(vscInitializeKEP(pKEP));
    }

    _CalcInputLowLevelSlot(&baseHelper, pKernel, gcvTRUE);
    _CalcOutputLowLevelSlot(&baseHelper, pKernel, gcvTRUE);

    /* 0. Set some flags. */
    errCode = _ConvertCompileFlags(pShPassMnger, pKernel);

    /* 1. Compile kernel */
    errCode = _CompileShaderInternal(pShPassMnger, bSkipSepGen ? gcvNULL : &pKEP->sep, (pOutKEP == gcvNULL));
    ON_ERROR(errCode, "Compiler internal");

    /* 2. Generate HL mapping tables */
    if (pOutKEP)
    {
        errCode = _GenerateKepHLMappingTables(pShPassMnger, pKEP);
        ON_ERROR(errCode, "Generate HL mapping tables");
    }

    if (pOutKrnlStates)
    {
        /* 3. Now we have generated SEP, we can do HW level linkage and states programming */
        hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_VS] = gcvNULL;
        hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_HS] = gcvNULL;
        hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_DS] = gcvNULL;
        hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_GS] = gcvNULL;
        hwPipelineShsParam.pSEPArray[VSC_GFX_SHADER_STAGE_PS] = gcvNULL;
        hwPipelineShsParam.pSEPArray[VSC_CPT_SHADER_STAGE_CS] = &pKEP->sep;

        hwPipelineShsParam.pSysCtx = pShPassMnger->pCompilerParam->cfg.ctx.pSysCtx;

        gcmONERROR(vscProgramHwShaderStages(&hwPipelineShsParam, pOutKrnlStates, gcvTRUE));
    }

OnError:
    if (status != gcvSTATUS_OK || errCode != VSC_ERR_NONE || pOutKEP == gcvNULL)
    {
        if (pKEP)
        {
            vscFinalizeKEP(pKEP);
        }
    }

    return (status != gcvSTATUS_OK) ? VSC_ERR_INVALID_ARGUMENT : errCode;
}

gceSTATUS vscCreateKernel(VSC_SHADER_COMPILER_PARAM*      pCompilerParam,
                          KERNEL_EXECUTABLE_PROFILE*      pOutKEP,
                          VSC_HW_PIPELINE_SHADERS_STATES* pOutKrnlStates
                         )
{
    gceSTATUS                         status = gcvSTATUS_OK;
    VSC_ErrCode                       errCode = VSC_ERR_NONE;
    VSC_SHADER_PASS_MANAGER           shPassMnger;
    VSC_PASS_MM_POOL                  passMemPool;
    VSC_OPTN_Options                  options;
    VIR_Shader*                       pKernel = (VIR_Shader*)pCompilerParam->hShader;

    vscInitializePassMMPool(&passMemPool);

    if (pKernel->optionsLen && VIR_Shader_UseOfflineCompiler(pKernel))
    {
        gctSTRING pos = gcvNULL;
        gcoOS_StrStr(pKernel->buildOptions, "-", &pos);

        while (pos)
        {
            pos++;
            if (!pos)
                break;

            if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "O0", 2))
            {
                pos += 2;
                if (*pos == ' ' || *pos == '\0')
                {
                    gcmOPT_SetDisableOPTforDebugger(gcvTRUE);
                }
            }
            gcoOS_StrStr(pos, "-", &pos);
        }
    }

    vscInitializeOptions(&options,
                         &pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                         pCompilerParam->cfg.cFlags,
                         pCompilerParam->cfg.optFlags);

    vscSPM_Initialize(&shPassMnger,
                      pCompilerParam,
                      &passMemPool,
                      gcvTRUE,
                      pKernel->dumper,
                      &options,
                      VSC_PM_MODE_SEMI_AUTO);

    errCode = _CreateKernelInternal(&shPassMnger, pOutKEP, pOutKrnlStates);
    gcmOPT_SetDisableOPTforDebugger(gcvFALSE);
    ON_ERROR(errCode, "Create kernel internal");

OnError:
    vscFinalizeOptions(&options);
    vscSPM_Finalize(&shPassMnger, gcvTRUE);
    vscFinalizePassMMPool(&passMemPool);

    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

static VSC_ErrCode _LinkKernelModules(VSC_KERNEL_PROGRAM_LINKER_HELPER* pKrnlPgLinkHelper,
                                      VIR_Shader* pOutLinkedProgram,
                                      gctUINT* pKernelCount,
                                      char*** pppKrnlNameList)
{
    return VSC_ERR_NONE;
}

gceSTATUS vscLinkKernelProgram(VSC_KERNEL_PROGRAM_LINKER_PARAM*  pKrnlPgLinkParam,
                               gctUINT*                          pKernelCount,
                               SHADER_HANDLE                     hOutLinkedProgram,
                               KERNEL_EXECUTABLE_PROFILE**       ppOutKEPArray,
                               VSC_HW_PIPELINE_SHADERS_STATES**  ppOutKrnlStates
                              )
{
    gceSTATUS                         status = gcvSTATUS_OK;
    VSC_ErrCode                       errCode = VSC_ERR_NONE;
    VIR_Shader*                       pOutLinkedProgram = (VIR_Shader*)hOutLinkedProgram;
    gctUINT                           kernelCount = 0, kernelIdx;
    VSC_KERNEL_PROGRAM_LINKER_HELPER  krnlPgLinkHelper;
    VIR_Shader                        linkedProgram;
    VIR_Shader                        kernel;
    VSC_SHADER_COMPILER_PARAM         compilerParam;
    VSC_SHADER_PASS_MANAGER           shPassMnger;
    char**                            ppKrnlNameList = gcvNULL;
    VSC_OPTN_Options                  options;
    char                              buffer[4096];
    VIR_Dumper                        dumper;

    gcmASSERT(pKrnlPgLinkParam->pKrnlModuleHandlesArray);
    gcmASSERT(pKrnlPgLinkParam->moduleCount > 0);
    gcmASSERT(pKrnlPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_TO_MC);
    gcmASSERT(pKrnlPgLinkParam->cfg.cFlags & VSC_COMPILER_FLAG_COMPILE_CODE_GEN);

    memset(&shPassMnger, 0, sizeof(VSC_SHADER_PASS_MANAGER));

    gcoOS_ZeroMemory(&dumper, sizeof(dumper));
    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

    vscInitializeOptions(&options,
                         &pKrnlPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                         pKrnlPgLinkParam->cfg.cFlags,
                         pKrnlPgLinkParam->cfg.optFlags);

    vscKPPM_Initialize(&krnlPgLinkHelper.pgPassMnger, pKrnlPgLinkParam, &dumper, &options, VSC_PM_MODE_SEMI_AUTO);

    krnlPgLinkHelper.baseHelper.pHwCfg = &pKrnlPgLinkParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    krnlPgLinkHelper.baseHelper.pMM = &krnlPgLinkHelper.pgPassMnger.basePgmPM.pgMmPool.sharedPMP.mmWrapper;

    /* 1. Link all modules together */

    errCode = _LinkKernelModules(&krnlPgLinkHelper, &linkedProgram, &kernelCount, &ppKrnlNameList);
    ON_ERROR(errCode, "Link kernel modules");

    *pKernelCount = kernelCount;

    if (pOutLinkedProgram)
    {
    }

    /* 2. For each kernel to do real compiling */

    if (kernelCount)
    {
        if (ppOutKEPArray)
        {
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      kernelCount*sizeof(KERNEL_EXECUTABLE_PROFILE),
                                      (gctPOINTER*)ppOutKEPArray));
        }

        if (ppOutKrnlStates)
        {
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      kernelCount*sizeof(VSC_HW_PIPELINE_SHADERS_STATES),
                                      (gctPOINTER*)ppOutKrnlStates));
        }
    }

    compilerParam.cfg = pKrnlPgLinkParam->cfg;
    compilerParam.pShResourceLayout = gcvNULL;
    for (kernelIdx = 0; kernelIdx < kernelCount; kernelIdx ++)
    {
        /* Extract kernel from linkedProgram */
        vscExtractSubShader(&linkedProgram, ppKrnlNameList[kernelIdx], &kernel);

        /* A kernel must be separated */
        VIR_Shader_SetFlag(&kernel, VIR_SHFLAG_SEPARATED);

        compilerParam.hShader = &kernel;

        vscSPM_Initialize(&shPassMnger,
                          &compilerParam,
                          &krnlPgLinkHelper.pgPassMnger.basePgmPM.shMmPool,
                          gcvTRUE,
                          kernel.dumper,
                          &options,
                          VSC_PM_MODE_SEMI_AUTO);

        errCode = _CreateKernelInternal(&shPassMnger,
                                        ppOutKEPArray ? ppOutKEPArray[kernelIdx] : gcvNULL,
                                        ppOutKrnlStates ? ppOutKrnlStates[kernelIdx] : gcvNULL);
        ON_ERROR(errCode, "Link kernel modules");

        vscSPM_Finalize(&shPassMnger, gcvTRUE);
    }

OnError:
    if (status != gcvSTATUS_OK || errCode != VSC_ERR_NONE)
    {
        if (ppOutKEPArray && *ppOutKEPArray)
        {
            gcoOS_Free(gcvNULL, *ppOutKEPArray);
            *ppOutKEPArray = gcvNULL;
        }

        if (ppOutKrnlStates && *ppOutKrnlStates)
        {
            gcoOS_Free(gcvNULL, *ppOutKrnlStates);
            *ppOutKrnlStates = gcvNULL;
        }
    }

    vscFinalizeOptions(&options);
    vscSPM_Finalize(&shPassMnger, gcvFALSE);
    vscKPPM_Finalize(&krnlPgLinkHelper.pgPassMnger);

    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

gceSTATUS vscInitializeHwPipelineShadersStates(VSC_SYS_CONTEXT* pSysCtx, VSC_HW_PIPELINE_SHADERS_STATES* pHwShdsStates)
{
    gceSTATUS                         status = gcvSTATUS_OK;
    gctUINT                           ftEntryIdx;

    /* Initialize output states */
    pHwShdsStates->stateBufferSize = 0;
    pHwShdsStates->pStateBuffer = gcvNULL;

    /* Initialize hints */
    gcoOS_ZeroMemory(&pHwShdsStates->hints, sizeof(struct _gcsHINT));

    for (ftEntryIdx = 0; ftEntryIdx < GC_ICACHE_PREFETCH_TABLE_SIZE; ftEntryIdx++)
    {
        pHwShdsStates->hints.vsICachePrefetch[ftEntryIdx] = -1;
        pHwShdsStates->hints.tcsICachePrefetch[ftEntryIdx] = -1;
        pHwShdsStates->hints.tesICachePrefetch[ftEntryIdx] = -1;
        pHwShdsStates->hints.gsICachePrefetch[ftEntryIdx] = -1;
        pHwShdsStates->hints.fsICachePrefetch[ftEntryIdx] = -1;
    }

    PROGRAM_UNIFIED_STATUS_Initialize(&pHwShdsStates->hints.unifiedStatus, pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalSamplerCount);

    pHwShdsStates->hints.sampleMaskLoc = -1;
    pHwShdsStates->hints.psOutCntl0to3 = -1;
    pHwShdsStates->hints.psOutCntl4to7 = -1;
    pHwShdsStates->hints.psOutCntl8to11 = -1;
    pHwShdsStates->hints.psOutCntl12to15 = -1;
    pHwShdsStates->hints.rtArrayComponent = -1;
    pHwShdsStates->hints.sampleMaskLoc = -1;

#if TEMP_SHADER_PATCH
    pHwShdsStates->hints.pachedShaderIdentifier = gcvMACHINECODE_COUNT;
#endif

    return status;
}

gceSTATUS vscFinalizeHwPipelineShadersStates(VSC_SYS_CONTEXT* pSysCtx, VSC_HW_PIPELINE_SHADERS_STATES* pHwShdsStates)
{
    gceSTATUS                         status = gcvSTATUS_OK;
    gctUINT                           i;

    if (pHwShdsStates->pStateBuffer)
    {
        gcoOS_Free(gcvNULL, pHwShdsStates->pStateBuffer);
        pHwShdsStates->pStateBuffer = gcvNULL;
    }

    if (pHwShdsStates->pStateDelta)
    {
        gcoOS_Free(gcvNULL, pHwShdsStates->pStateDelta);
        pHwShdsStates->pStateDelta = gcvNULL;
    }

    /* Destroy vid mems for i-caches */
    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (pHwShdsStates->hints.shaderVidNodes.instVidmemNode[i])
        {
            (*pSysCtx->drvCBs.pfnFreeVidMemCb)(pSysCtx->hDrv,
                                               gcvSURF_ICACHE,
                                               "instruction memory",
                                               pHwShdsStates->hints.shaderVidNodes.instVidmemNode[i]);

            pHwShdsStates->hints.shaderVidNodes.instVidmemNode[i] = gcvNULL;
        }

        if (pHwShdsStates->hints.shaderVidNodes.gprSpillVidmemNode[i])
        {
            (*pSysCtx->drvCBs.pfnFreeVidMemCb)(pSysCtx->hDrv,
                                               gcvSURF_VERTEX,
                                               "temp register spill memory",
                                               pHwShdsStates->hints.shaderVidNodes.gprSpillVidmemNode[i]);

            pHwShdsStates->hints.shaderVidNodes.gprSpillVidmemNode[i] = gcvNULL;
        }

        if (pHwShdsStates->hints.shaderVidNodes.crSpillVidmemNode[i])
        {
            (*pSysCtx->drvCBs.pfnFreeVidMemCb)(pSysCtx->hDrv,
                                               gcvSURF_VERTEX,
                                               "immediate constant spill memory",
                                               pHwShdsStates->hints.shaderVidNodes.crSpillVidmemNode[i]);

            pHwShdsStates->hints.shaderVidNodes.crSpillVidmemNode[i] = gcvNULL;
        }

        if (pHwShdsStates->hints.shaderVidNodes.sharedMemVidMemNode)
        {
            (*pSysCtx->drvCBs.pfnFreeVidMemCb)(pSysCtx->hDrv,
                                               gcvSURF_VERTEX,
                                               "share variable memory",
                                               pHwShdsStates->hints.shaderVidNodes.sharedMemVidMemNode);

            pHwShdsStates->hints.shaderVidNodes.sharedMemVidMemNode = gcvNULL;
        }

        if (pHwShdsStates->hints.shaderVidNodes.threadIdVidMemNode)
        {
            (*pSysCtx->drvCBs.pfnFreeVidMemCb)(pSysCtx->hDrv,
                                               gcvSURF_VERTEX,
                                               "thread id memory",
                                               pHwShdsStates->hints.shaderVidNodes.threadIdVidMemNode);

            pHwShdsStates->hints.shaderVidNodes.threadIdVidMemNode = gcvNULL;
        }
    }

    return status;
}

static VSC_ErrCode _ProgramHwShadersStates(VSC_SYS_CONTEXT*                pSysCtx,
                                           VSC_HW_SHADERS_LINK_INFO*       pHwShsLinkInfo,
                                           VSC_HW_PIPELINE_SHADERS_STATES* pOutHwShdsStates)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_CHIP_STATES_PROGRAMMER chipStatesPgmer;
    gctUINT                    stageIdx;

    gcmASSERT(pOutHwShdsStates);

    /* Initialize output states */
    vscInitializeHwPipelineShadersStates(pSysCtx, pOutHwShdsStates);

    /* Initialize chip states programer */
    errCode = vscInitializeChipStatesProgrammer(&chipStatesPgmer, pSysCtx, &pOutHwShdsStates->hints);
    ON_ERROR(errCode, "Intialize chip states-programmer");

    /* Do programming for each shader stage. Note that seq of calling vscProgramShaderStates must
       be vs->ds->gs because some hints fillage to driver dependends on this seq, like flags for
       pre-PA */
    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pHwShsLinkInfo->shHwInfoArray[stageIdx].pSEP)
        {
            errCode = vscProgramShaderStates(&pHwShsLinkInfo->shHwInfoArray[stageIdx], &chipStatesPgmer);
            ON_ERROR(errCode, "Shader states programming");
        }
    }

    gcmASSERT(chipStatesPgmer.pStartStateBuffer != gcvNULL && chipStatesPgmer.nextStateAddr > 0);

#if (_DEBUG || DEBUG)
    /* Verify correctness of states content */
    errCode = vscVerifyShaderStates(&chipStatesPgmer);
    ON_ERROR(errCode, "Verify shader states");
#endif

    /* Copy states out */
    pOutHwShdsStates->stateBufferSize = chipStatesPgmer.nextStateAddr * sizeof(gctUINT);
    if (gcoOS_Allocate(gcvNULL, pOutHwShdsStates->stateBufferSize,
                       (gctPOINTER*)&pOutHwShdsStates->pStateBuffer) == gcvSTATUS_OK)
    {
        gcoOS_MemCopy(pOutHwShdsStates->pStateBuffer,
                      chipStatesPgmer.pStartStateBuffer,
                      pOutHwShdsStates->stateBufferSize);
    }
    else
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "States allocation");
    }

    pOutHwShdsStates->stateDeltaSize = chipStatesPgmer.nextStateDeltaAddr * sizeof(gctUINT);
    if (gcoOS_Allocate(gcvNULL, pOutHwShdsStates->stateDeltaSize,
                       (gctPOINTER*)&pOutHwShdsStates->pStateDelta) == gcvSTATUS_OK)
    {
        gcoOS_MemCopy(pOutHwShdsStates->pStateDelta,
                      chipStatesPgmer.pStateDelta,
                      pOutHwShdsStates->stateDeltaSize);
    }
    else
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        ON_ERROR(errCode, "State Delta allocation");
    }

    gcoOS_MemCopy((gctPOINTER)&pOutHwShdsStates->patchOffsetsInDW, (gctPOINTER)&chipStatesPgmer.patchOffsetsInDW,
        sizeof(gcsPROGRAM_VidMemPatchOffset));


OnError:
    vscFinalizeChipStatesProgrammer(&chipStatesPgmer);
    return errCode;
}

static VSC_ErrCode _ValidateHwPipelineShaders(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam)
{
    gctUINT                           stageIdx;
    gctBOOL                           bHasValidShader = gcvFALSE;

    /* Must have shaders included */
    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pHwPipelineShsParam->pSEPArray[stageIdx])
        {
            bHasValidShader = gcvTRUE;
            break;
        }
    }

    if (!bHasValidShader)
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    /* Compute and graphics can not be mixed together for a single draw/dispatch kickoff */
    if (pHwPipelineShsParam->pSEPArray[VSC_CPT_SHADER_STAGE_CS] &&
        DECODE_SHADER_TYPE(pHwPipelineShsParam->pSEPArray[VSC_CPT_SHADER_STAGE_CS]->shVersionType) ==
        SHADER_TYPE_GENERAL)
    {
        for (stageIdx = 1; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pHwPipelineShsParam->pSEPArray[stageIdx])
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        /* For gfx kickoff, combination of shader stages must be legal, these legal cases are
           1. VS + (PS)
           2. VS + GS + (PS)
           3. VS + HS + DS + (PS)
           4. VS + HS + DS + GS + (PS)

           PS is optional is because of SO
        */

#if !PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
        if (!pHwPipelineShsParam->pSEPArray[VSC_GFX_SHADER_STAGE_VS])
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }

        if ((pHwPipelineShsParam->pSEPArray[VSC_GFX_SHADER_STAGE_HS]   &&
             !pHwPipelineShsParam->pSEPArray[VSC_GFX_SHADER_STAGE_DS]) ||
            (pHwPipelineShsParam->pSEPArray[VSC_GFX_SHADER_STAGE_DS]   &&
             !pHwPipelineShsParam->pSEPArray[VSC_GFX_SHADER_STAGE_HS]))
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }
#endif
    }

    return VSC_ERR_NONE;
}

gceSTATUS vscProgramHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM*       pHwPipelineShsParam,
                                   VSC_HW_PIPELINE_SHADERS_STATES*      pOutHwShdsStates,
                                   gctBOOL                              bSeperatedShaders)
{
    VSC_ErrCode                         errCode = VSC_ERR_NONE;
    gceSTATUS                           status = gcvSTATUS_OK;
    VSC_HW_SHADERS_LINK_INFO            hwShsLinkInfo;

    gcmASSERT(pOutHwShdsStates);

    errCode = _ValidateHwPipelineShaders(pHwPipelineShsParam);
    ON_ERROR(errCode, "Hw pipeline validation");

    /* Firstly, do HW level linkage */
    gcmONERROR(vscLinkHwShaders(pHwPipelineShsParam, &hwShsLinkInfo, bSeperatedShaders));

    /* Secondly, do states programming */
    errCode = _ProgramHwShadersStates(pHwPipelineShsParam->pSysCtx, &hwShsLinkInfo, pOutHwShdsStates);
    ON_ERROR(errCode, "Program shaders-states");

OnError:
    return (status == gcvSTATUS_OK) ? vscERR_CastErrCode2GcStatus(errCode) : status;
}

static VSC_ErrCode _FindAndLinkAnOuputForAnInput(SHADER_HW_INFO* pUpperHwShader,
                                                 SHADER_HW_INFO* pLowerHwShader,
                                                 SHADER_IO_MAPPING_PER_EXE_OBJ* pInputMapping,
                                                 SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMapping,
                                                 SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pInputLinkageInfo,
                                                 SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkageInfo,
                                                 gctUINT inputIdx,
                                                 gctUINT* pLinkNo
                                                 )
{
    SHADER_IO_REG_MAPPING*              pThisInputRegMapping;
    SHADER_IO_REG_MAPPING*              pThisOutputRegMapping = gcvNULL;
    SHADER_IO_REG_LINKAGE*              pThisInputRegLinkage;
    SHADER_IO_REG_LINKAGE*              pThisOutputRegLinkage;
    gctUINT                             outputIdx, channel;
    SHADER_CLIENT                       shClient = DECODE_SHADER_CLIENT(pUpperHwShader->pSEP->shVersionType);

    pThisInputRegMapping = &pInputMapping->pIoRegMapping[inputIdx];
    pThisInputRegLinkage = &pInputLinkageInfo->ioRegLinkage[inputIdx];

    if (shClient == SHADER_CLIENT_GL   ||
        shClient == SHADER_CLIENT_GLES ||
        shClient == SHADER_CLIENT_VK   ||
        /* Assume unknown client as GL for now */
        shClient == SHADER_CLIENT_UNKNOWN
       )
    {
        /* DX10+ and OGL use ioIdx matching */

        outputIdx = NOT_ASSIGNED;
        if (pOutputMapping->ioIndexMask & (1LL << inputIdx))
        {
            outputIdx = inputIdx;

            pThisOutputRegMapping = &pOutputMapping->pIoRegMapping[outputIdx];

            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                {
                    if (!pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }
                else if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                {
                    vscDumpMessage(gcvNULL, gcvNULL, "!!!Shader has redundant components, can be optimized!!!");
                }
            }
        }
    }
    else
    {
        /* DX9 uses usage and usageIdx matching */

        gcmASSERT(gcvFALSE);
        outputIdx = NOT_ASSIGNED;
    }

    if (outputIdx == pOutputMapping->countOfIoRegMapping || outputIdx == NOT_ASSIGNED)
    {
        /* Ops! Upper-shader has no corresponding output that lower-shader needs */
        return VSC_ERR_INVALID_ARGUMENT;
    }
    else
    {
        pThisOutputRegLinkage = &pOutputLinkageInfo->ioRegLinkage[outputIdx];
        pThisOutputRegLinkage->linkNo = (*pLinkNo) ++;

        if (pInputMapping->ioMode == SHADER_IO_MODE_ACTIVE)
        {
            if (pThisInputRegMapping->regIoMode == SHADER_IO_MODE_ACTIVE)
            {
                gcmASSERT(pThisOutputRegLinkage->linkNo ==
                    pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                    hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM);
            }
        }
        else
        {
            if (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_PIXEL &&
                pLowerHwShader->pSEP->exeHints.derivedHints.globalStates.bExecuteOnDual16)
            {
                /* How to do verification??? */
            }
            else
            {
                gcmASSERT(pThisOutputRegLinkage->linkNo ==
                          pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwRegNo);
            }
        }

        /* Mark both lower/upper shaders as be inter-used */
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
            {
                switch (channel) {
                case CHANNEL_X:
                    pThisInputRegLinkage->bLinkedByOtherStageX  =
                    pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                    break;
                case CHANNEL_Y:
                    pThisInputRegLinkage->bLinkedByOtherStageY  =
                    pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                    break;
                case CHANNEL_Z:
                    pThisInputRegLinkage->bLinkedByOtherStageZ  =
                    pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                    break;
                default:
                    pThisInputRegLinkage->bLinkedByOtherStageW  =
                    pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                    break;
                }
            }
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _AssignLinkNumToUnFFULinkedOutputs(SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMapping,
                                                      SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkageInfo,
                                                      gctUINT* pLinkNo)
{
    SHADER_IO_REG_MAPPING*              pThisOutputRegMapping;
    SHADER_IO_REG_LINKAGE*              pThisOutputRegLinkage;
    gctUINT                             outputIdx;

    for (outputIdx = 0; outputIdx < pOutputMapping->countOfIoRegMapping; outputIdx ++)
    {
        if (!(pOutputMapping->ioIndexMask & (1LL << outputIdx)))
        {
            continue;
        }

        pThisOutputRegMapping = &pOutputMapping->pIoRegMapping[outputIdx];
        pThisOutputRegLinkage = &pOutputLinkageInfo->ioRegLinkage[outputIdx];

        /* If it has been linked before, skip it */
        if (pThisOutputRegLinkage->linkNo != NOT_ASSIGNED)
        {
            continue;
        }

        /* Only consider active mode output */
        if (pThisOutputRegMapping->regIoMode != SHADER_IO_MODE_ACTIVE)
        {
            continue;
        }

        pThisOutputRegLinkage->linkNo = (*pLinkNo) ++;

        pThisOutputRegLinkage->bIsDummyLink = gcvTRUE;

        gcmASSERT(pThisOutputRegLinkage->linkNo ==
                  pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].
                  hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM);
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _LinkVtxPxlIoBetweenTwoHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam,
                                                         SHADER_HW_INFO* pUpperHwShader,
                                                         SHADER_HW_INFO* pLowerHwShader)
{
    SHADER_IO_MAPPING_PER_EXE_OBJ*      pVtxPxlInputMapping = &pLowerHwShader->pSEP->inputMapping.ioVtxPxl;
    SHADER_IO_MAPPING_PER_EXE_OBJ*      pVtxPxlOutputMapping = &pUpperHwShader->pSEP->outputMapping.ioVtxPxl;
    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pVtxPxlInputLinkageInfo = &pLowerHwShader->inputLinkageInfo.vtxPxlLinkage;
    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pVtxPxlOutputLinkageInfo = &pUpperHwShader->outputLinkageInfo.vtxPxlLinkage;
    SHADER_IO_REG_MAPPING*              pThisInputRegMapping;
    SHADER_IO_REG_MAPPING*              pThisOutputRegMapping = gcvNULL;
    SHADER_IO_REG_LINKAGE*              pThisOutputRegLinkage;
    gctUINT                             outputIdx, inputIdx, channel, linkNo = 0;
    gctUINT                             ptSzIdx = NOT_ASSIGNED, sortedInputIdx;
    gctBOOL                             bGLClient;
#if !IO_HW_LOC_NUMBER_IN_ORDER
    gctUINT                             sortedInputIdxArray[MAX_SHADER_IO_NUM];
#endif
#if (_DEBUG || DEBUG)
    gctUINT                             dummyPrimIdLinkNo = NOT_ASSIGNED, dummyPtCoordLinkNo = NOT_ASSIGNED;
    gctBOOL                             bProcessingHpPsInputOnDual16;
#endif

    gcmASSERT(pVtxPxlInputMapping->ioMemAlign == pVtxPxlOutputMapping->ioMemAlign ||
              DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_PIXEL);

    /*
       !!!! From macro view, the link seq is:
            normal outputs                                      +
            dummy point-coord (if lower shader is PS)           +
            dummy primId (if lower shader is PS, and
                             PS input primId, and
                             upper shader has no primId output) +
            ptSz (if lower shader is PS)                        +
            pure SO (if SO is deployed)                         +
            dummyOutputs
       !!!!
    */

    bGLClient = (DECODE_SHADER_CLIENT(pUpperHwShader->pSEP->shVersionType) == SHADER_CLIENT_GL ||
                 DECODE_SHADER_CLIENT(pUpperHwShader->pSEP->shVersionType) == SHADER_CLIENT_GLES ||
                 DECODE_SHADER_CLIENT(pUpperHwShader->pSEP->shVersionType) == SHADER_CLIENT_VK ||
                 /* Assume unknown client as GL for now */
                 DECODE_SHADER_CLIENT(pUpperHwShader->pSEP->shVersionType) == SHADER_CLIENT_UNKNOWN);

    if (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_PIXEL)
    {
        /* Set position of pre-RA stage at link 0 since it is reserved by HW */
        if (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask)
        {
            outputIdx = pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POSITION].mainIoIndex;

            pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
            pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

            /* Need assure there is no other usages packed with position */
            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POSITION ||
                        pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }

                    switch (channel) {
                    case CHANNEL_X:
                        pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                        break;
                    case CHANNEL_Y:
                        pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                        break;
                    case CHANNEL_Z:
                        pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                        break;
                    default:
                        pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                        break;
                    }
                }
            }

            pThisOutputRegLinkage->linkNo = 0;
        }

        /* Get pointsize output index */
        if (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask)
        {
            outputIdx = pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].mainIoIndex;

            pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
            pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

            /* Need assure there is no other usages packed with pointsize */
            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POINTSIZE ||
                        pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }

                    switch (channel) {
                    case CHANNEL_X:
                        pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                        break;
                    case CHANNEL_Y:
                        pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                        break;
                    case CHANNEL_Z:
                        pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                        break;
                    default:
                        pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                        break;
                    }
                }
            }

            ptSzIdx = outputIdx;
        }

        /* For linking to pixel shader, always start from 1 since 0 is reserved for position */
        linkNo = 1;
    }

#if !IO_HW_LOC_NUMBER_IN_ORDER
    vscSortIOsByHwLoc(pVtxPxlInputMapping, sortedInputIdxArray);
#endif

#if (_DEBUG || DEBUG)
    /* HP inputs of ps under dual16 mode start from the first one input or the 2nd one (in this case,
       the first one is position) */
    bProcessingHpPsInputOnDual16 = (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask == 0);
#endif

    /* By tranversing inputs of lower-shader, mark outputs of upper-shader as be used and get
       a link number based on lower-shader's input index (hole must be eliminated) */
    for (inputIdx = 0; inputIdx < pVtxPxlInputMapping->countOfIoRegMapping; inputIdx ++)
    {
#if !IO_HW_LOC_NUMBER_IN_ORDER
        sortedInputIdx = sortedInputIdxArray[inputIdx];
#else
        sortedInputIdx = inputIdx;
#endif

        if (!(pVtxPxlInputMapping->ioIndexMask & (1LL << sortedInputIdx)))
        {
            continue;
        }

        pThisInputRegMapping = &pVtxPxlInputMapping->pIoRegMapping[sortedInputIdx];

        if (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_PIXEL)
        {
            /* No need to link position since it is always at the first location (r0) */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask & (1LL << sortedInputIdx))
            {
                gcmASSERT(pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwRegNo == 0);

                /* Need assure there is no other usages packed with position */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POSITION ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

#if (_DEBUG || DEBUG)
                bProcessingHpPsInputOnDual16 = gcvTRUE;
#endif

                continue;
            }

            /* No need to link frontface since it is a special HW 'Face' register */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_ISFRONTFACE].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with frontface */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        gcmASSERT(pThisInputRegMapping->ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo ==
                                  SPECIAL_HW_IO_REG_NO);

                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_ISFRONTFACE ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

            /* No need to sample-id since it is a special HW register */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_SAMPLE_INDEX].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with sample-id */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        gcmASSERT(pThisInputRegMapping->ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo ==
                                  SPECIAL_HW_IO_REG_NO);

                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_SAMPLE_INDEX ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

            /* No need to sample-mask-in since it is a special HW register */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_SAMPLE_MASK].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with sample-mask-in */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        gcmASSERT(pThisInputRegMapping->ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo ==
                                  SPECIAL_HW_IO_REG_NO);

                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_SAMPLE_MASK ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

            /* No need to sample-pos since it is a special HW register */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_SAMPLE_POSITION].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with sample-pos */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        gcmASSERT(pThisInputRegMapping->ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo ==
                                  SPECIAL_HW_IO_REG_NO);

                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_SAMPLE_POSITION ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

            /* No need to link helper-pixel since it is passed into shader by pixel mask maintained by HW */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_HELPER_PIXEL].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with helper-pixel */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        gcmASSERT(pThisInputRegMapping->ioChannelMapping[channel].hwLoc.cmnHwLoc.u.hwRegNo ==
                                  SPECIAL_HW_IO_REG_NO);

                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_HELPER_PIXEL ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

            /* Need skip a link slot for point-coord, as HW always uses a dummy output of pre-PA shader and set
               correct point-coord in PA, so we need leave a room for such dummy output */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_POINT_COORD].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with point-coord */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POINT_COORD ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

#if (_DEBUG || DEBUG)
                if (pVtxPxlOutputMapping->ioMode == SHADER_IO_MODE_ACTIVE)
                {
                    dummyPtCoordLinkNo = linkNo;
                }
#endif

                /* Skip */
                linkNo ++;

                continue;
            }

            /* Need skip a link slot for primitive-id if pre-PA shader has no explicit primid output, as HW always
               uses a dummy output of pre-PA shader and set correct primid in PA, so we need leave a room for such
               dummy output. */
            if ((pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask & (1LL << sortedInputIdx)) &&
                (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask == 0))
            {
                /* Need assure there is no other usages packed with primid */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_PRIMITIVEID ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

#if (_DEBUG || DEBUG)
                if (pVtxPxlOutputMapping->ioMode == SHADER_IO_MODE_ACTIVE)
                {
                    dummyPrimIdLinkNo = linkNo;
                }
#endif

                /* Skip */
                linkNo ++;

                continue;
            }

            /* No need to link sample-depth since it is always passed into shader by the last temp register */
            if (pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_SAMPLE_DEPTH].ioIndexMask & (1LL << sortedInputIdx))
            {
                /* Need assure there is no other usages packed with sample-depth */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        if (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_SAMPLE_DEPTH ||
                            pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }
                    }
                }

                continue;
            }

#if (_DEBUG || DEBUG)
            /* All HP inputs of ps under dual16 mode must be aggregated together in the very begin (except position) */
            if (!bProcessingHpPsInputOnDual16 &&
                pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].flag.bHighPrecisionOnDual16)
            {
                gcmASSERT(gcvFALSE);
            }

            bProcessingHpPsInputOnDual16 &=
                pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].flag.bHighPrecisionOnDual16;
#endif
        }

        /* !!! For GL, for shaders combination VS + HS or VS + GS, position and point-size can be not
               flowed from VS to lower shader. See function _CheckFakeSGVForPosAndPtSz to check why we
               need this code !!! */
        if (bGLClient)
        {
            if ((DECODE_SHADER_TYPE(pUpperHwShader->pSEP->shVersionType) == SHADER_TYPE_VERTEX) &&
                (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_HULL ||
                 DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_GEOMETRY))
            {
                /* Case for undefined position */
                if ((pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask & (1LL << sortedInputIdx)) &&
                    (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask == 0))
                {
                    continue;
                }

                /* Case for undefined point-size */
                if ((pVtxPxlInputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask & (1LL << sortedInputIdx)) &&
                    (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask == 0))
                {
                    continue;
                }
            }
        }

        /* Try to find an output which is an counterpart of input, if found, link them then */
        if (_FindAndLinkAnOuputForAnInput(pUpperHwShader,
                                          pLowerHwShader,
                                          pVtxPxlInputMapping,
                                          pVtxPxlOutputMapping,
                                          pVtxPxlInputLinkageInfo,
                                          pVtxPxlOutputLinkageInfo,
                                          sortedInputIdx,
                                          &linkNo) != VSC_ERR_NONE)
        {
            return VSC_ERR_INVALID_ARGUMENT;
        }
    }

    /* Point-size must be at last link location (from perspective of PA) */
    if (ptSzIdx != NOT_ASSIGNED)
    {
        pVtxPxlOutputLinkageInfo->ioRegLinkage[ptSzIdx].linkNo = linkNo ++;
    }

#if (_DEBUG || DEBUG)
    /* Insure link slot for dummy pt-coord is at correct location */
    if (dummyPtCoordLinkNo != NOT_ASSIGNED)
    {
        if (dummyPrimIdLinkNo != NOT_ASSIGNED)
        {
            gcmASSERT(dummyPtCoordLinkNo == ((ptSzIdx != NOT_ASSIGNED) ? (linkNo - 3) : (linkNo - 2)));
        }
        else
        {
            gcmASSERT(dummyPtCoordLinkNo == ((ptSzIdx != NOT_ASSIGNED) ? (linkNo - 2) : (linkNo - 1)));
        }
    }

    /* Insure link slot for dummy primid is at correct location */
    if (dummyPrimIdLinkNo != NOT_ASSIGNED)
    {
        gcmASSERT(dummyPrimIdLinkNo == ((ptSzIdx != NOT_ASSIGNED) ? (linkNo - 2) : (linkNo - 1)));
    }
#endif

    /* Link stream-out */
    if (pVtxPxlOutputMapping->soIoIndexMask != 0 && pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut)
    {
        for (outputIdx = 0; outputIdx < pVtxPxlOutputMapping->countOfIoRegMapping; outputIdx ++)
        {
            if (!(pVtxPxlOutputMapping->ioIndexMask & (1LL << outputIdx)))
            {
                continue;
            }

            /* If is not streamed output, skip it */
            if (!(pVtxPxlOutputMapping->soIoIndexMask & (1LL << outputIdx)))
            {
                continue;
            }

            /* If it has been linked before, skip it */
            if (pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx].linkNo != NOT_ASSIGNED)
            {
                continue;
            }

            pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
            pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

            /* Mark channel of this output as be used by SO FFU */
            for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
            {
                if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                {
                    switch (channel) {
                    case CHANNEL_X:
                        pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                        break;
                    case CHANNEL_Y:
                        pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                        break;
                    case CHANNEL_Z:
                        pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                        break;
                    default:
                        pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                        break;
                    }
                }
            }

            pThisOutputRegLinkage->linkNo = linkNo ++;

            if (pVtxPxlOutputMapping->ioMode == SHADER_IO_MODE_ACTIVE)
            {
                gcmASSERT(pThisOutputRegLinkage->linkNo ==
                   pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].
                   hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM);
            }

            pThisOutputRegLinkage->bOnlyLinkToSO = gcvTRUE;
        }
    }

    /* For active-mode outputs that are not linked to other FFU, we also need give them
       a link number because USC-calc needs know about this */
    if (_AssignLinkNumToUnFFULinkedOutputs(pVtxPxlOutputMapping, pVtxPxlOutputLinkageInfo, &linkNo))
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    pVtxPxlOutputLinkageInfo->totalLinkNoCount = linkNo;

    return VSC_ERR_NONE;
}

static VSC_ErrCode _LinkPrimIoBetweenTwoHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam,
                                                       SHADER_HW_INFO* pUpperHwShader,
                                                       SHADER_HW_INFO* pLowerHwShader)
{
    SHADER_IO_MAPPING_PER_EXE_OBJ*      pPrimInputMapping = &pLowerHwShader->pSEP->inputMapping.ioPrim;
    SHADER_IO_MAPPING_PER_EXE_OBJ*      pPrimOutputMapping = &pUpperHwShader->pSEP->outputMapping.ioPrim;
    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pPrimInputLinkageInfo = &pLowerHwShader->inputLinkageInfo.primLinkage;
    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pPrimOutputLinkageInfo = &pUpperHwShader->outputLinkageInfo.primLinkage;
    SHADER_IO_REG_MAPPING*              pThisInputRegMapping;
    SHADER_IO_REG_MAPPING*              pThisOutputRegMapping = gcvNULL;
    SHADER_IO_REG_LINKAGE*              pThisOutputRegLinkage;
    gctUINT                             channel, inputIdx, outputIdx, linkNo = 0;
    gctUINT                             sortedInputIdx, thisUsageIndex;
#if !IO_HW_LOC_NUMBER_IN_ORDER
    gctUINT                             sortedInputIdxArray[MAX_SHADER_IO_NUM];
#endif

    if (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_DOMAIN)
    {
        /* Link number is reserved by HW for tessfactor */
        if (pPrimOutputMapping->usage2IO[SHADER_IO_USAGE_TESSFACTOR].ioIndexMask)
        {
            for (outputIdx = 0; outputIdx < pPrimOutputMapping->countOfIoRegMapping; outputIdx ++)
            {
                if (!(pPrimOutputMapping->usage2IO[SHADER_IO_USAGE_TESSFACTOR].ioIndexMask & (1LL << outputIdx)))
                {
                    continue;
                }

                pThisOutputRegMapping = &pPrimOutputMapping->pIoRegMapping[outputIdx];
                pThisOutputRegLinkage = &pPrimOutputLinkageInfo->ioRegLinkage[outputIdx];
                thisUsageIndex = pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].usageIndex;

                /* Need assure there is no other usages packed with tessfactor */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_TESSFACTOR ||
                            pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != thisUsageIndex)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }

                        switch (channel) {
                        case CHANNEL_X:
                            pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                            break;
                        case CHANNEL_Y:
                            pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                            break;
                        case CHANNEL_Z:
                            pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                            break;
                        default:
                            pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                            break;
                        }
                    }
                }

                pThisOutputRegLinkage->linkNo = 0;
                gcmASSERT(pThisOutputRegLinkage->linkNo ==
                    (pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].
                     hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM));
            }
        }

        /* Link number is reserved by HW for inside-tessfactor */
        if (pPrimOutputMapping->usage2IO[SHADER_IO_USAGE_INSIDETESSFACTOR].ioIndexMask)
        {
            for (outputIdx = 0; outputIdx < pPrimOutputMapping->countOfIoRegMapping; outputIdx ++)
            {
                if (!(pPrimOutputMapping->usage2IO[SHADER_IO_USAGE_INSIDETESSFACTOR].ioIndexMask & (1LL << outputIdx)))
                {
                    continue;
                }

                pThisOutputRegMapping = &pPrimOutputMapping->pIoRegMapping[outputIdx];
                pThisOutputRegLinkage = &pPrimOutputLinkageInfo->ioRegLinkage[outputIdx];
                thisUsageIndex = pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].usageIndex;

                /* Need assure there is no other usages packed with inside-tessfactor */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_INSIDETESSFACTOR ||
                            pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != thisUsageIndex)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }

                        switch (channel) {
                        case CHANNEL_X:
                            pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                            break;
                        case CHANNEL_Y:
                            pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                            break;
                        case CHANNEL_Z:
                            pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                            break;
                        default:
                            pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                            break;
                        }
                    }
                }

                pThisOutputRegLinkage->linkNo = 1;
                gcmASSERT(pThisOutputRegLinkage->linkNo ==
                    (pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].
                     hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM));
            }
        }

        linkNo = 2;
    }

#if !IO_HW_LOC_NUMBER_IN_ORDER
    vscSortIOsByHwLoc(pPrimInputMapping, sortedInputIdxArray);
#endif

    /* By tranversing inputs of lower-shader, mark outputs of upper-shader as be used and get
       a link number based on lower-shader's input index (hole must be eliminated) */
    for (inputIdx = 0; inputIdx < pPrimInputMapping->countOfIoRegMapping; inputIdx ++)
    {
#if !IO_HW_LOC_NUMBER_IN_ORDER
        sortedInputIdx = sortedInputIdxArray[inputIdx];
#else
        sortedInputIdx = inputIdx;
#endif

        if (!(pPrimInputMapping->ioIndexMask & (1LL << sortedInputIdx)))
        {
            continue;
        }

        pThisInputRegMapping = &pPrimInputMapping->pIoRegMapping[sortedInputIdx];

        if (DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_GEOMETRY ||
            DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_HULL)
        {
            /* Any of prim-inputs of GS/HS must be SVs */
            if (!IS_SHADER_IO_USAGE_SGV(
                    pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].ioUsage)
               )
            {
                gcmASSERT(gcvFALSE);
            }
        }
        else
        {
            gcmASSERT(DECODE_SHADER_TYPE(pUpperHwShader->pSEP->shVersionType) == SHADER_TYPE_HULL &&
                      DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType) == SHADER_TYPE_DOMAIN);

            gcmASSERT(pPrimInputMapping->ioMemAlign == pPrimOutputMapping->ioMemAlign);

            /* No need to link domain location since it is always at the first location (r0) */
            if (pPrimInputMapping->usage2IO[SHADER_IO_USAGE_DOMAIN_LOCATION].ioIndexMask & (1LL << sortedInputIdx))
            {
                gcmASSERT(pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwRegNo == 0);

                /* Need assure there is no other usages packed with domain location */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_DOMAIN_LOCATION ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

                continue;
            }

            /* No need to link primitiveID since it is reserved by HW */
            if (pPrimInputMapping->usage2IO[SHADER_IO_USAGE_PRIMITIVEID].ioIndexMask & (1LL << sortedInputIdx))
            {
                gcmASSERT(pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM == 1);

                /* Need assure there is no other usages packed with domain location */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_PRIMITIVEID ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != 0))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

                continue;
            }

            /* No need to link tessfactor since it is always at fixed hw channel index */
            if (pPrimInputMapping->usage2IO[SHADER_IO_USAGE_TESSFACTOR].ioIndexMask & (1LL << sortedInputIdx))
            {
                gcmASSERT(pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM == 0);
                thisUsageIndex = pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].usageIndex;

                /* Need assure there is no other usages packed with tessfactor */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_TESSFACTOR ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != thisUsageIndex))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

                continue;
            }

            /* No need to link inside-tessfactor since it is always at fixed hw channel index */
            if (pPrimInputMapping->usage2IO[SHADER_IO_USAGE_INSIDETESSFACTOR].ioIndexMask & (1LL << sortedInputIdx))
            {
                gcmASSERT(pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].
                          hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM == 1);
                thisUsageIndex = pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].usageIndex;

                /* Need assure there is no other usages packed with tessfactor */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader &&
                        (pThisInputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_INSIDETESSFACTOR ||
                        pThisInputRegMapping->ioChannelMapping[channel].usageIndex != thisUsageIndex))
                    {
                        return VSC_ERR_INVALID_ARGUMENT;
                    }
                }

                continue;
            }

            /* Try to find an output which is an counterpart of input, if found, link them then */
            if (_FindAndLinkAnOuputForAnInput(pUpperHwShader,
                                              pLowerHwShader,
                                              pPrimInputMapping,
                                              pPrimOutputMapping,
                                              pPrimInputLinkageInfo,
                                              pPrimOutputLinkageInfo,
                                              sortedInputIdx,
                                              &linkNo) != VSC_ERR_NONE)
            {
                return VSC_ERR_INVALID_ARGUMENT;
            }
        }
    }

    /* For active-mode outputs that are not linked to other FFU, we also need give them
       a link number because USC-calc needs know about this */
    if (_AssignLinkNumToUnFFULinkedOutputs(pPrimOutputMapping, pPrimOutputLinkageInfo, &linkNo))
    {
        return VSC_ERR_INVALID_ARGUMENT;
    }

    pPrimOutputLinkageInfo->totalLinkNoCount = linkNo;

    return VSC_ERR_NONE;
}

static VSC_ErrCode _LinkIoBetweenTwoHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam,
                                                   SHADER_HW_INFO* pUpperHwShader,
                                                   SHADER_HW_INFO* pLowerHwShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    SHADER_IO_LINKAGE_INFO*    pInputLinkageInfo = &pLowerHwShader->inputLinkageInfo;
    SHADER_IO_LINKAGE_INFO*    pOutputLinkageInfo = &pUpperHwShader->outputLinkageInfo;

    errCode = _LinkVtxPxlIoBetweenTwoHwShaderStages(pHwPipelineShsParam, pUpperHwShader, pLowerHwShader);
    ON_ERROR(errCode, "Link Io between two hw shader stages on vertex-pixel level");

    errCode = _LinkPrimIoBetweenTwoHwShaderStages(pHwPipelineShsParam, pUpperHwShader, pLowerHwShader);
    ON_ERROR(errCode, "Link Io between two hw shader stages on prim level");

    pInputLinkageInfo->linkedShaderStage = (SHADER_TYPE)DECODE_SHADER_TYPE(pUpperHwShader->pSEP->shVersionType);
    pOutputLinkageInfo->linkedShaderStage = (SHADER_TYPE)DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType);

OnError:
    return errCode;
}

static VSC_ErrCode _LinkInputOfFirstHwShaderStage(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam,
                                                  SHADER_HW_INFO* pHwShader)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    gctUINT                    linkNo = 0, ioIdx, sortedIoIdx, channel;
    SHADER_IO_MAPPING*         pInputMapping = &pHwShader->pSEP->inputMapping;
    SHADER_IO_LINKAGE_INFO*    pInputLinkageInfo = &pHwShader->inputLinkageInfo;
    SHADER_IO_REG_MAPPING*     pThisInputRegMapping;
    SHADER_IO_REG_LINKAGE*     pThisInputRegLinkage;
    gctINT                     maxLinkNo = -1;
#if !IO_HW_LOC_NUMBER_IN_ORDER
    gctUINT                    sortedIoIdxArray[MAX_SHADER_IO_NUM];
#endif

#if !PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
    gcmASSERT(pHwShader->pSEP->inputMapping.ioPrim.countOfIoRegMapping == 0);
#endif

#if !IO_HW_LOC_NUMBER_IN_ORDER
    vscSortIOsByHwLoc(&pHwShader->pSEP->inputMapping.ioVtxPxl, sortedIoIdxArray);
#endif

    /* Calc link number by simplily eliminating hole */
    for (ioIdx = 0; ioIdx < pInputMapping->ioVtxPxl.countOfIoRegMapping; ioIdx ++)
    {
#if !IO_HW_LOC_NUMBER_IN_ORDER
        sortedIoIdx = sortedIoIdxArray[ioIdx];
#else
        sortedIoIdx = ioIdx;
#endif

        if (!(pInputMapping->ioVtxPxl.ioIndexMask & (1LL << sortedIoIdx)))
        {
            continue;
        }

        pThisInputRegMapping = &pInputMapping->ioVtxPxl.pIoRegMapping[sortedIoIdx];
        pThisInputRegLinkage = &pInputLinkageInfo->vtxPxlLinkage.ioRegLinkage[sortedIoIdx];

        /* Mark channel of this input as be used by FFU */
        for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
        {
            if (pThisInputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
            {
                switch (channel) {
                case CHANNEL_X:
                    pThisInputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                    break;
                case CHANNEL_Y:
                    pThisInputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                    break;
                case CHANNEL_Z:
                    pThisInputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                    break;
                default:
                    pThisInputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                    break;
                }
            }
        }

        /* Note that now we only support IOs without usages mixed */
        if (IS_SHADER_IO_USAGE_SGV(
                       pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].ioUsage))
        {
            if (pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo !=
                SPECIAL_HW_IO_REG_NO)
            {
                pThisInputRegLinkage->linkNo =
                    pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo;
            }
        }
        else
        {
            pThisInputRegLinkage->linkNo = linkNo ++;

#if PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
            if (DECODE_SHADER_TYPE(pHwShader->pSEP->shVersionType) == SHADER_TYPE_VERTEX ||
                DECODE_SHADER_TYPE(pHwShader->pSEP->shVersionType) == SHADER_TYPE_GENERAL)
#endif
            {
                gcmASSERT(pThisInputRegLinkage->linkNo ==
                    pThisInputRegMapping->ioChannelMapping[pThisInputRegMapping->firstValidIoChannel].hwLoc.cmnHwLoc.u.hwRegNo);
            }
        }

        if (pThisInputRegLinkage->linkNo != NOT_ASSIGNED)
        {
            if ((gctINT)pThisInputRegLinkage->linkNo > maxLinkNo)
            {
                maxLinkNo = pThisInputRegLinkage->linkNo;
            }
        }
    }

    pInputLinkageInfo->vtxPxlLinkage.totalLinkNoCount = maxLinkNo + 1;
    pInputLinkageInfo->linkedShaderStage = SHADER_TYPE_FFU;

    return errCode;
}

static VSC_ErrCode _LinkOutputOfLastHwShaderStage(VSC_HW_PIPELINE_SHADERS_PARAM* pHwPipelineShsParam,
                                                  SHADER_HW_INFO* pHwShader)
{
    VSC_ErrCode                         errCode = VSC_ERR_NONE;
    SHADER_IO_LINKAGE_INFO*             pOutputLinkageInfo = &pHwShader->outputLinkageInfo;
    SHADER_IO_MAPPING_PER_EXE_OBJ*      pVtxPxlOutputMapping = &pHwShader->pSEP->outputMapping.ioVtxPxl;
    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pVtxPxlOutputLinkageInfo = &pOutputLinkageInfo->vtxPxlLinkage;
    SHADER_IO_REG_MAPPING*              pThisOutputRegMapping = gcvNULL;
    SHADER_IO_REG_LINKAGE*              pThisOutputRegLinkage;
    gctUINT                             outputIdx, channel, linkNo = 0;
    gctUINT                             ptSzIdx = NOT_ASSIGNED;

    pOutputLinkageInfo->linkedShaderStage = SHADER_TYPE_FFU;

    if (DECODE_SHADER_TYPE(pHwShader->pSEP->shVersionType) == SHADER_TYPE_VERTEX   ||
        DECODE_SHADER_TYPE(pHwShader->pSEP->shVersionType) == SHADER_TYPE_GEOMETRY ||
        DECODE_SHADER_TYPE(pHwShader->pSEP->shVersionType) == SHADER_TYPE_DOMAIN)
    {
        /* Before check the stream-out output, we need to check position and pointSize if present. */

        /* Get position output index */
        if (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POSITION].ioIndexMask)
        {
            outputIdx = pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POSITION].mainIoIndex;

            /* No need to check if it is stream outed. */
            if (!(pVtxPxlOutputMapping->soIoIndexMask & (1LL << outputIdx)))
            {
                pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
                pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

                /* Need assure there is no other usages packed with position */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POSITION ||
                            pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }

                        switch (channel) {
                        case CHANNEL_X:
                            pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                            break;
                        case CHANNEL_Y:
                            pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                            break;
                        case CHANNEL_Z:
                            pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                            break;
                        default:
                            pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                            break;
                        }
                    }
                }

                pThisOutputRegLinkage->linkNo = 0;

                /* Always start from 1 since 0 is reserved for position */
                linkNo = 1;
            }
        }

        /* Get pointsize output index */
        if (pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].ioIndexMask)
        {
            outputIdx = pVtxPxlOutputMapping->usage2IO[SHADER_IO_USAGE_POINTSIZE].mainIoIndex;

            /* No need to check if it is stream outed. */
            if (!(pVtxPxlOutputMapping->soIoIndexMask & (1LL << outputIdx)))
            {
                pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
                pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

                /* Need assure there is no other usages packed with pointsize */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        if (pThisOutputRegMapping->ioChannelMapping[channel].ioUsage != SHADER_IO_USAGE_POINTSIZE ||
                            pThisOutputRegMapping->ioChannelMapping[channel].usageIndex != 0)
                        {
                            return VSC_ERR_INVALID_ARGUMENT;
                        }

                        switch (channel) {
                        case CHANNEL_X:
                            pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                            break;
                        case CHANNEL_Y:
                            pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                            break;
                        case CHANNEL_Z:
                            pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                            break;
                        default:
                            pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                            break;
                        }
                    }
                }

                ptSzIdx = outputIdx;
            }
        }

        /* Link stream-out (only consider per-vertex IOs) */
        if (pVtxPxlOutputMapping->soIoIndexMask != 0 && pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportStreamOut)
        {
            for (outputIdx = 0; outputIdx < pVtxPxlOutputMapping->countOfIoRegMapping; outputIdx ++)
            {
                if (!(pVtxPxlOutputMapping->ioIndexMask & (1LL << outputIdx)))
                {
                    continue;
                }

                /* If it is not streamed output, skip it */
                if (!(pVtxPxlOutputMapping->soIoIndexMask & (1LL << outputIdx)))
                {
                    continue;
                }

                pThisOutputRegMapping = &pVtxPxlOutputMapping->pIoRegMapping[outputIdx];
                pThisOutputRegLinkage = &pVtxPxlOutputLinkageInfo->ioRegLinkage[outputIdx];

                /* Mark channel of this output as be used by SO FFU */
                for (channel = CHANNEL_X; channel < CHANNEL_NUM; channel ++)
                {
                    if (pThisOutputRegMapping->ioChannelMapping[channel].flag.bActiveWithinShader)
                    {
                        switch (channel) {
                        case CHANNEL_X:
                            pThisOutputRegLinkage->bLinkedByOtherStageX = gcvTRUE;
                            break;
                        case CHANNEL_Y:
                            pThisOutputRegLinkage->bLinkedByOtherStageY = gcvTRUE;
                            break;
                        case CHANNEL_Z:
                            pThisOutputRegLinkage->bLinkedByOtherStageZ = gcvTRUE;
                            break;
                        default:
                            pThisOutputRegLinkage->bLinkedByOtherStageW = gcvTRUE;
                            break;
                        }
                    }
                }

                if (pVtxPxlOutputMapping->ioMode == SHADER_IO_MODE_ACTIVE)
                {
                    /* It is not easy to verify relation between linkNo and hw-comp-index, so just
                       set linkNo as hw-comp-index/CHANNEL_NUM. */
                    pThisOutputRegLinkage->linkNo =
                       pThisOutputRegMapping->ioChannelMapping[pThisOutputRegMapping->firstValidIoChannel].
                       hwLoc.cmnHwLoc.u.hwChannelLoc/CHANNEL_NUM;
                }
                else
                {
                    pThisOutputRegLinkage->linkNo = linkNo ++;
                }

                pThisOutputRegLinkage->bOnlyLinkToSO = gcvTRUE;
            }
        }

        if (ptSzIdx != NOT_ASSIGNED)
        {
            pVtxPxlOutputLinkageInfo->ioRegLinkage[ptSzIdx].linkNo = linkNo ++;
        }
    }

    return errCode;
}

/* Note only valid for vs/ps/cs */
#define RESOLVE_UNIFIED_INST_BASE_ADDR_OFFSET(maxHwNativeTotalInstCount, pSEP)          \
           (DECODE_SHADER_TYPE((pSEP)->shVersionType) == SHADER_TYPE_VERTEX ||          \
            DECODE_SHADER_TYPE((pSEP)->shVersionType) == SHADER_TYPE_GENERAL)   ?       \
                                                                            0   :       \
                        ((maxHwNativeTotalInstCount) - (pSEP)->countOfMCInst)

#define RESOLVE_UNIFIED_CONST_RF_BASE_ADDR_OFFSET(maxHwNativeTotalConstRegCount, pSEP)  \
           (DECODE_SHADER_TYPE((pSEP)->shVersionType) == SHADER_TYPE_VERTEX ||          \
            DECODE_SHADER_TYPE((pSEP)->shVersionType) == SHADER_TYPE_GENERAL)   ?       \
                                                                            0   :       \
  ((maxHwNativeTotalConstRegCount) - (pSEP)->constantMapping.hwConstRegCount)

static VSC_ErrCode _AnalyzeHwInstProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                                  SHADER_HW_INFO**                ppActiveShHwInfoArray,
                                                  gctUINT                         activeHwShaderStageCount,
                                                  gctUINT                         totalMachineCodeSize,
                                                  VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo)
{
    gctBOOL                    bLoadInstToCache = gcvFALSE, bCanPutInstToIB = gcvFALSE;
    SHADER_TYPE                shType;
    gctUINT                    stageIdx;

    if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache &&
        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch)
    {
        bLoadInstToCache = gcvTRUE;
    }
    else
    {
        /* Old chips don't support HS/DS/GS. */
        gcmASSERT(pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP == gcvNULL);

        if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.instBufferUnified)
        {
            /* Case 1, each shader stage has float size of inst space in inst-buffer */
            bCanPutInstToIB = (totalMachineCodeSize <= pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount);

            /* Case 2, each shader stage has fixed size of inst space in inst-buffer */
        }
        else
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
            {
                shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                      pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->shVersionType);

                bCanPutInstToIB = (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->countOfMCInst <=
                                   ((shType == SHADER_TYPE_GENERAL) ? pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSInstCount :
                                                                      pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSInstCount));
            }

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP)
            {
                bCanPutInstToIB = ((pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP->countOfMCInst <=
                                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSInstCount) && bCanPutInstToIB);
            }
        }

        /* Inst-buffer is preferred against non-prefetchable cache */
        if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCache && !bCanPutInstToIB)
        {
            gcmASSERT(!pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasInstCachePrefetch);

            bLoadInstToCache = gcvTRUE;
        }
    }

    if (bLoadInstToCache)
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
            {
                pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.hwInstFetchMode =
                                                                                HW_INST_FETCH_MODE_CACHE;
            }
        }
    }
    else
    {
        /* Old chips don't support HS/DS/GS. */
        gcmASSERT(pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP == gcvNULL);

        if (!bCanPutInstToIB)
        {
            return VSC_ERR_OUT_OF_RESOURCE;
        }
        else
        {
            if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 1024)
            {
                gcmASSERT(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.instBufferUnified);

                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNIFIED_BUFFER_1;
                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstBufferAddrOffset =
                    RESOLVE_UNIFIED_INST_BASE_ADDR_OFFSET(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount,
                                                          ppActiveShHwInfoArray[0]->pSEP);

                if (activeHwShaderStageCount == 2)
                {
                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNIFIED_BUFFER_1;

                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstBufferAddrOffset =
                        RESOLVE_UNIFIED_INST_BASE_ADDR_OFFSET(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount,
                                                              ppActiveShHwInfoArray[1]->pSEP);
                }
            }
            else if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount > 256)
            {
                gcmASSERT(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.instBufferUnified);

                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNIFIED_BUFFER_0;
                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstBufferAddrOffset =
                    RESOLVE_UNIFIED_INST_BASE_ADDR_OFFSET(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount,
                                                          ppActiveShHwInfoArray[0]->pSEP);

                if (activeHwShaderStageCount == 2)
                {
                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNIFIED_BUFFER_0;

                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstBufferAddrOffset =
                        RESOLVE_UNIFIED_INST_BASE_ADDR_OFFSET(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount,
                                                              ppActiveShHwInfoArray[1]->pSEP);
                }
            }
            else
            {
                gcmASSERT(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalInstCount == 256);
                gcmASSERT(!pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.instBufferUnified);

                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNUNIFIED_BUFFER;
                ppActiveShHwInfoArray[0]->hwProgrammingHints.hwInstBufferAddrOffset = 0;

                if (activeHwShaderStageCount == 2)
                {
                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstFetchMode = HW_INST_FETCH_MODE_UNUNIFIED_BUFFER;
                    ppActiveShHwInfoArray[1]->hwProgrammingHints.hwInstBufferAddrOffset = 0;
                }
            }
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _AnalyzeHwConstantRFProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                                        SHADER_HW_INFO**                ppActiveShHwInfoArray,
                                                        gctUINT                         activeHwShaderStageCount,
                                                        gctUINT                         totalConstRegCount,
                                                        VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo)
{
    SHADER_TYPE                shType;
    gctUINT                    stageIdx;
    UNIFIED_RF_ALLOC_STRATEGY  unifiedAllocStrategy;

    if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.constRegFileUnified)
    {
        unifiedAllocStrategy = ppActiveShHwInfoArray[0]->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy;

        if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
        {
            /* If const register is allocated in full scope of unified register file, just set offset to zero */

            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                gcmASSERT(ppActiveShHwInfoArray[stageIdx]->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy ==
                          (gctUINT)unifiedAllocStrategy);

                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantFetchMode =
                                                                   HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE;
                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantRegAddrOffset = 0;
            }
        }
        else if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET ||
                 unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_GPIPE_TOP_PS_BOTTOM_FLOAT_ADDR_OFFSET)
        {
            if (totalConstRegCount > pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount)
            {
                return VSC_ERR_OUT_OF_RESOURCE;
            }

            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantFetchMode = HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE;

                if (stageIdx == 0)
                {
                    ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantRegAddrOffset = 0;
                }
                else if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET)
                {
                    ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantRegAddrOffset =
                        ppActiveShHwInfoArray[stageIdx - 1]->hwProgrammingHints.hwConstantRegAddrOffset +
                        ppActiveShHwInfoArray[stageIdx - 1]->pSEP->constantMapping.hwConstRegCount;
                }
                else
                {
                    gcmASSERT(unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_GPIPE_TOP_PS_BOTTOM_FLOAT_ADDR_OFFSET);

                    if (DECODE_SHADER_TYPE((ppActiveShHwInfoArray[stageIdx]->pSEP)->shVersionType) == SHADER_TYPE_PIXEL)
                    {
                        ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantRegAddrOffset =
                            RESOLVE_UNIFIED_CONST_RF_BASE_ADDR_OFFSET(pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalConstRegCount,
                                                                        ppActiveShHwInfoArray[stageIdx]->pSEP);
                    }
                    else
                    {
                        ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantRegAddrOffset =
                            ppActiveShHwInfoArray[stageIdx - 1]->hwProgrammingHints.hwConstantRegAddrOffset +
                            ppActiveShHwInfoArray[stageIdx - 1]->pSEP->constantMapping.hwConstRegCount;
                    }
                }
            }
        }
        else if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET)
        {
            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                gcmASSERT(ppActiveShHwInfoArray[stageIdx]->pSEP->exeHints.derivedHints.globalStates.unifiedConstRegAllocStrategy ==
                          (gctUINT)unifiedAllocStrategy);

                 ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwConstantFetchMode =
                                                                   HW_CONSTANT_FETCH_MODE_UNIFIED_REG_FILE;
            }

            /* Check VS/CS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
            {
                shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                      pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->shVersionType);

                if (shType == SHADER_TYPE_GENERAL)
                {
                    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].pSEP->constantMapping.hwConstRegCount >
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSConstRegCount)
                    {
                        return VSC_ERR_OUT_OF_RESOURCE;
                    }
                    pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].hwProgrammingHints.hwConstantRegAddrOffset =
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTESConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxGSConstRegCount;
                }
                else
                {
                    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->constantMapping.hwConstRegCount >
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount)
                    {
                        return VSC_ERR_OUT_OF_RESOURCE;
                    }
                    pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.hwConstantRegAddrOffset = 0;
                }
            }

            /* Check HS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP->constantMapping.hwConstRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSConstRegCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].hwProgrammingHints.hwConstantRegAddrOffset =
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount;
            }

            /* Check DS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP->constantMapping.hwConstRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTESConstRegCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].hwProgrammingHints.hwConstantRegAddrOffset =
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSConstRegCount;
            }

            /* Check GS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP->constantMapping.hwConstRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxGSConstRegCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].hwProgrammingHints.hwConstantRegAddrOffset =
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTESConstRegCount;
            }

            /* Check PS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP->constantMapping.hwConstRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSConstRegCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].hwProgrammingHints.hwConstantRegAddrOffset =
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTESConstRegCount +
                                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxGSConstRegCount;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else
    {
        /* Old chips don't support HS/DS/GS. */
        gcmASSERT(pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP == gcvNULL);

        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
        {
            shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->shVersionType);

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->constantMapping.hwConstRegCount >
                ((shType == SHADER_TYPE_GENERAL) ? pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSConstRegCount:
                                                   pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSConstRegCount))
            {
                return VSC_ERR_OUT_OF_RESOURCE;
            }

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.hwConstantRegAddrOffset = 0;
            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.hwConstantFetchMode =
                                                                            HW_CONSTANT_FETCH_MODE_UNUNIFIED_REG_FILE;
        }

        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP->constantMapping.hwConstRegCount >
                pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSConstRegCount)
            {
                return VSC_ERR_OUT_OF_RESOURCE;
            }

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].hwProgrammingHints.hwConstantRegAddrOffset = 0;
            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].hwProgrammingHints.hwConstantFetchMode =
                                                                            HW_CONSTANT_FETCH_MODE_UNUNIFIED_REG_FILE;
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _AnalyzeHwSamplerRFProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                                       SHADER_HW_INFO**                ppActiveShHwInfoArray,
                                                       gctUINT                         activeHwShaderStageCount,
                                                       gctUINT                         totalSamplerRegCount,
                                                       VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo)
{
    SHADER_TYPE                shType;
    gctUINT                    stageIdx;
    UNIFIED_RF_ALLOC_STRATEGY  unifiedAllocStrategy;

    if (totalSamplerRegCount > pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalSamplerCount)
    {
        return VSC_ERR_OUT_OF_RESOURCE;
    }

    if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.samplerRegFileUnified)
    {
        unifiedAllocStrategy = ppActiveShHwInfoArray[0]->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy;

        if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_UNIFIED)
        {
            /* If sampler register is allocated in full scope of unified register file, just set offset to zero */

            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                gcmASSERT(ppActiveShHwInfoArray[stageIdx]->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy ==
                          (gctUINT)unifiedAllocStrategy);

                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerFetchMode =
                                                                   HW_SAMPLER_FETCH_MODE_UNIFIED_REG_FILE;
                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset = 0;
            }
        }
        else if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET ||
                 unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PS_TOP_GPIPE_BOTTOM_FLOAT_ADDR_OFFSET)
        {
            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                if (ppActiveShHwInfoArray[stageIdx]->pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxSamplerCountPerShader)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }

                gcmASSERT(ppActiveShHwInfoArray[stageIdx]->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy ==
                          (gctUINT)unifiedAllocStrategy);

                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerFetchMode =
                                                                   HW_SAMPLER_FETCH_MODE_UNIFIED_REG_FILE;

                if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasSamplerBaseOffset)
                {
                    shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                          ppActiveShHwInfoArray[stageIdx]->pSEP->shVersionType);

                    /*
                    ** For those chips can support full unified uniform allocation,
                    ** pack all shaders one by one so we can get the minimum used sampler.
                    ** For those chips can not support full unified uniform allocation,
                    ** pack all GPipe shaders one by one, and use the bottom sampler memory for GPipe.
                    */
                    if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PACK_FLOAT_ADDR_OFFSET)
                    {
                        if (stageIdx == 0)
                        {
                            ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset = 0;
                        }
                        else
                        {
                            ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset =
                                    ppActiveShHwInfoArray[stageIdx - 1]->hwProgrammingHints.hwSamplerRegAddrOffset +
                                    ppActiveShHwInfoArray[stageIdx - 1]->pSEP->samplerMapping.hwSamplerRegCount;
                        }
                    }
                    else
                    {
                        gcmASSERT(unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_PS_TOP_GPIPE_BOTTOM_FLOAT_ADDR_OFFSET);

                        if (shType == SHADER_TYPE_GENERAL || shType == SHADER_TYPE_PIXEL)
                        {
                            ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset = 0;
                        }
                        else
                        {
                            if (stageIdx == 0)
                            {
                                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset =
                                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxHwNativeTotalSamplerCount -
                                        ppActiveShHwInfoArray[stageIdx]->pSEP->samplerMapping.hwSamplerRegCount;
                            }
                            else
                            {
                                ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset =
                                        ppActiveShHwInfoArray[stageIdx - 1]->hwProgrammingHints.hwSamplerRegAddrOffset -
                                        ppActiveShHwInfoArray[stageIdx]->pSEP->samplerMapping.hwSamplerRegCount;
                            }
                        }
                    }
                }
                else
                {
                    ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerRegAddrOffset = 0;
                }
            }
        }
        else if (unifiedAllocStrategy == UNIFIED_RF_ALLOC_STRATEGY_FIXED_ADDR_OFFSET)
        {
            for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
            {
                gcmASSERT(ppActiveShHwInfoArray[stageIdx]->pSEP->exeHints.derivedHints.globalStates.unifiedSamplerRegAllocStrategy ==
                          (gctUINT)unifiedAllocStrategy);

                 ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerFetchMode =
                                                                   HW_SAMPLER_FETCH_MODE_UNIFIED_REG_FILE;
            }

            /* Check VS/CS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
            {
                shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                      pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->shVersionType);

                if (shType == SHADER_TYPE_GENERAL)
                {
                    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].pSEP->samplerMapping.hwSamplerRegCount >
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxCSSamplerCount)
                    {
                        return VSC_ERR_OUT_OF_RESOURCE;
                    }
                    pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].hwProgrammingHints.hwSamplerRegAddrOffset =
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.csSamplerRegNoBase;
                }
                else
                {
                    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->samplerMapping.hwSamplerRegCount >
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSSamplerCount)
                    {
                        return VSC_ERR_OUT_OF_RESOURCE;
                    }
                    pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.hwSamplerRegAddrOffset =
                        pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.vsSamplerRegNoBase;
                }
            }

            /* Check HS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTCSSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.tcsSamplerRegNoBase;
            }

            /* Check DS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxTESSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.tesSamplerRegNoBase;
            }

            /* Check GS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxGSSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.gsSamplerRegNoBase;
            }

            /* Check PS. */
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.psSamplerRegNoBase;
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }
    }
    else
    {
        /* Old chips don't support HS/DS/GS. */
        gcmASSERT(pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP == gcvNULL &&
                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP == gcvNULL);

        for (stageIdx = 0; stageIdx < activeHwShaderStageCount; stageIdx ++)
        {
             ppActiveShHwInfoArray[stageIdx]->hwProgrammingHints.hwSamplerFetchMode =
                                                               HW_SAMPLER_FETCH_MODE_UNUNIFIED_REG_FILE;
        }

        /* Check VS/CS. */
        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
        {
            shType = (SHADER_TYPE)DECODE_SHADER_TYPE(
                                  pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->shVersionType);

            if (shType == SHADER_TYPE_GENERAL)
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxCSSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_CPT_SHADER_STAGE_CS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.csSamplerRegNoBase;
            }
            else
            {
                if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP->samplerMapping.hwSamplerRegCount >
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxVSSamplerCount)
                {
                    return VSC_ERR_OUT_OF_RESOURCE;
                }
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.hwSamplerRegAddrOffset =
                    pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.vsSamplerRegNoBase;
            }
        }

        /* Check PS. */
        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].pSEP->samplerMapping.hwSamplerRegCount >
                pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxPSSamplerCount)
            {
                return VSC_ERR_OUT_OF_RESOURCE;
            }
            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_PS].hwProgrammingHints.hwSamplerRegAddrOffset =
                pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.psSamplerRegNoBase;
        }
    }

    return VSC_ERR_NONE;
}

/* Defined in chip_state_programming.c */
extern gctUINT _GetValidHwRegChannelCount(SHADER_IO_REG_MAPPING* pIoRegMapping, SHADER_IO_MEM_ALIGN ioMemAlign);
extern gctUINT _GetGsValidMaxThreadsPerHwTG(SHADER_EXECUTABLE_PROFILE* pGsSEP, gctUINT maxThreadsPerHwTG, gctUINT maxHwTGThreadCount);
extern gctUINT _GetHsRemapMode(SHADER_EXECUTABLE_PROFILE* pHsSEP,
                               gctUINT hsPerCPInputCount,
                               gctUINT hsPerCPOutputCount,
                               gctBOOL* pIsInputRemapMode);
extern gctUINT _GetHsValidMaxPatchesPerHwTG(SHADER_EXECUTABLE_PROFILE* pHsSEP,
                                            gctUINT maxHwTGThreadCount,
                                            gctBOOL bIsInputRemap,
                                            gctUINT maxParallelFactor);

/* !!!! NOTE: The logic of this function MUST BE EQUAL to state programming of each stage
              in gc_vsc_chip_state_programmming.c !!!! */
static gctUINT _GetValidInputCount(gctUINT shStage,
                                   SHADER_IO_MAPPING_PER_EXE_OBJ* pInputMappingPerObj)
{
    gctUINT ioIdx, inputCount = 0;

    gcmASSERT(shStage == VSC_GFX_SHADER_STAGE_HS);

    for (ioIdx = 0; ioIdx < pInputMappingPerObj->countOfIoRegMapping; ioIdx ++)
    {
        if (pInputMappingPerObj->ioIndexMask & (1LL << ioIdx))
        {
            inputCount ++;
        }
    }

    return inputCount;
}

/* !!!! NOTE: The logic of this function MUST BE EQUAL to state programming of each stage
              in gc_vsc_chip_state_programmming.c !!!! */
static gctUINT _GetValidOutputCount(gctUINT shStage,
                                    SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMappingPerObj,
                                    SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkageInfoPerObj)
{
    gctUINT                    ioIdx, outputCount = 0;

    for (ioIdx = 0; ioIdx < pOutputMappingPerObj->countOfIoRegMapping; ioIdx ++)
    {
        if (pOutputMappingPerObj->ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pOutputLinkageInfoPerObj->ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                /* Ok, increase count */
                outputCount ++;
            }
        }
    }

    if (shStage != VSC_GFX_SHADER_STAGE_HS)
    {
        /* For the cases that need dummy outputs for non-HS (generally, these outputs should be generated by HW,
           but our HW does not), so outputs of shader and outputs to PA are different, we need pick the right
           one here */
        outputCount = vscMAX(outputCount, pOutputLinkageInfoPerObj->totalLinkNoCount);

        /* Output must include position (explicit or implicit) */
        if (outputCount == 0)
        {
            outputCount ++;
        }
    }
    else if (pOutputMappingPerObj->ioCategory == SHADER_IO_CATEGORY_PER_PRIM)
    {
        gcmASSERT(pOutputMappingPerObj->ioMode == SHADER_IO_MODE_ACTIVE ||
                  pOutputMappingPerObj->countOfIoRegMapping == 0);

        /* Need consider output link count is different with IO count because currently we only
           support IOs without usages mixed */
        outputCount = (outputCount == pOutputLinkageInfoPerObj->totalLinkNoCount) ?
                      outputCount : pOutputLinkageInfoPerObj->totalLinkNoCount;
        outputCount = (outputCount < 2) ? 2 : outputCount;
    }

    return outputCount;
}

/* !!!! NOTE: The logic of this function MUST BE EQUAL to state programming of each stage
              in gc_vsc_chip_state_programmming.c !!!! */
static gctUINT _GetValidOutputsChannelCount(gctUINT shStage,
                                            SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMappingPerObj,
                                            SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkageInfoPerObj)
{
    gctUINT                    ioIdx, outputCount = 0, totalChannelCount = 0;

    /* Now only GS support comp-granularity */
    gcmASSERT(shStage == VSC_GFX_SHADER_STAGE_GS);

    for (ioIdx = 0; ioIdx < pOutputMappingPerObj->countOfIoRegMapping; ioIdx ++)
    {
        if (pOutputMappingPerObj->ioIndexMask & (1LL << ioIdx))
        {
            /* Only consider linked one */
            if (pOutputLinkageInfoPerObj->ioRegLinkage[ioIdx].linkNo != NOT_ASSIGNED)
            {
                /* Ok, increase channel count */
                totalChannelCount += _GetValidHwRegChannelCount(&pOutputMappingPerObj->pIoRegMapping[ioIdx],
                                                                pOutputMappingPerObj->ioMemAlign);

                /* Ok, increase count */
                outputCount ++;
            }
        }
    }

    /* For the cases that need dummy outputs for non-HS (generally, these outputs should be generated by HW,
        but our HW does not), so outputs of shader and outputs to PA are different, we need pick the right
        one here */
    if (pOutputLinkageInfoPerObj->totalLinkNoCount > outputCount)
    {
        totalChannelCount += CHANNEL_NUM * (pOutputLinkageInfoPerObj->totalLinkNoCount - outputCount);
    }

    /* Output must include position (explicit or implicit) */
    if (totalChannelCount < CHANNEL_NUM)
    {
        totalChannelCount = CHANNEL_NUM;
    }

    return totalChannelCount;
}

static gctUINT _GetTotalOutputsSizeInBytesPerVtxCp(gctUINT shStage,
                                                   SHADER_IO_MAPPING_PER_EXE_OBJ* pOutputMappingPerObj,
                                                   SHADER_IO_LINKAGE_INFO_PER_EXE_OBJ* pOutputLinkageInfoPerObj,
                                                   gctBOOL bCompGranularity)
{
    gctUINT                    outputCount = 0, totalChannelCount = 0;

    gcmASSERT(shStage != VSC_GFX_SHADER_STAGE_PS);

    /* We will fully support packed mem mode for active io mode later */
    gcmASSERT(pOutputMappingPerObj->ioMode == SHADER_IO_MODE_PASSIVE ||
              pOutputMappingPerObj->ioMemAlign == SHADER_IO_MEM_ALIGN_4_CHANNELS);

    if (bCompGranularity)
    {
        totalChannelCount = _GetValidOutputsChannelCount(shStage, pOutputMappingPerObj, pOutputLinkageInfoPerObj);

        /* Each componenent has 4 bytes, but needs align to 16-bytes */
        return VSC_UTILS_ALIGN((totalChannelCount << 2), 16);
    }
    else
    {
        outputCount = _GetValidOutputCount(shStage, pOutputMappingPerObj, pOutputLinkageInfoPerObj);

        /* Each output has 16 bytes */
        return (outputCount << 4);
    }
}

static void _CalcHwThreadCount(gctUINT shStage,
                               gctUINT baseMaxThreadCount,
                               gctUINT maxHwTGThreadCount,
                               gctUINT iteIndex,
                               gctBOOL bUseRawThreadCount,
                               gctUINT* pMaxRawThreadCount,
                               gctUINT* pMaxAlignedThreadCount,
                               gctUINT* pRealUsedMaxThreadCount,
                               gctUINT* pExpectedMaxThreadsPerHwTG)
{
    *pMaxRawThreadCount = baseMaxThreadCount * iteIndex;
    *pMaxAlignedThreadCount = VSC_UTILS_ALIGN(*pMaxRawThreadCount, maxHwTGThreadCount);

    *pRealUsedMaxThreadCount = bUseRawThreadCount ? *pMaxRawThreadCount : *pMaxAlignedThreadCount;

    if ((shStage == VSC_GFX_SHADER_STAGE_VS || shStage == VSC_GFX_SHADER_STAGE_DS) &&
        (*pRealUsedMaxThreadCount < maxHwTGThreadCount))
    {
        *pRealUsedMaxThreadCount = VSC_UTILS_ALIGN(*pRealUsedMaxThreadCount, 4);
    }

    *pExpectedMaxThreadsPerHwTG = bUseRawThreadCount ?
                                  ((*pRealUsedMaxThreadCount > maxHwTGThreadCount) ?
                                   maxHwTGThreadCount : *pRealUsedMaxThreadCount) :
                                  maxHwTGThreadCount;
}

static gctUINT _AnalyzeHwUSCSizeForVs(gctUINT baseMaxThreadCount,
                                      gctUINT realUSCThreshold,
                                      gctUINT maxHwTGThreadCount,
                                      gctUINT maxResCashWinSize,
                                      gctUINT iteIndex,
                                      gctBOOL bUseRawThreadCount,
                                      gctUINT vsOutputSizePerThread,
                                      gctUINT downStreamVerticesCountPerHwTG,
                                      gctUINT* pMaxRawThreadCount,
                                      gctUINT* pMaxAlignedThreadCount,
                                      gctUINT* pRealUsedMaxThreadCount,
                                      gctUINT* pExpectedMaxThreadsPerHwTG,
                                      gctUINT* pExpectedResCashWinSize,
                                      gctBOOL* pBExceedThreshold)
{
    gctUINT    expectedUSCSize, totalThreadCount;

    *pBExceedThreshold = gcvFALSE;

    _CalcHwThreadCount(VSC_GFX_SHADER_STAGE_VS,
                       baseMaxThreadCount,
                       maxHwTGThreadCount,
                       iteIndex,
                       bUseRawThreadCount,
                       pMaxRawThreadCount,
                       pMaxAlignedThreadCount,
                       pRealUsedMaxThreadCount,
                       pExpectedMaxThreadsPerHwTG);

    *pExpectedResCashWinSize = (*pRealUsedMaxThreadCount < 128) ? (*pRealUsedMaxThreadCount / 4) :
                               (*pRealUsedMaxThreadCount / 2);
    *pExpectedResCashWinSize = (*pExpectedResCashWinSize > maxResCashWinSize) ?
                               maxResCashWinSize : *pExpectedResCashWinSize;
    *pExpectedResCashWinSize = vscMAX(3, *pExpectedResCashWinSize);

    totalThreadCount = VSC_UTILS_ALIGN(*pRealUsedMaxThreadCount, 4) + /* Output vertices */
                       *pExpectedResCashWinSize                     + /* Reslut window size */
                       3                                            + /* Fragmentation */
                       (downStreamVerticesCountPerHwTG - 1);          /* Input vertices that next down stream stage needs */
    totalThreadCount = VSC_UTILS_ALIGN(totalThreadCount, 4);
    expectedUSCSize = VSC_UTILS_ALIGN(vsOutputSizePerThread * totalThreadCount, 1024);

    /* USC for VS can not exceed max size that VS can hold */
    if (expectedUSCSize > (realUSCThreshold << 10))
    {
        expectedUSCSize = (realUSCThreshold << 10);
        *pBExceedThreshold = gcvTRUE;
    }

    return expectedUSCSize;
}

static gctUINT _AnalyzeHwUSCSizeForHs(gctUINT baseMaxThreadCount,
                                      gctUINT realUSCThreshold,
                                      gctUINT maxHwTGThreadCount,
                                      gctUINT outputCtrlPointCount,
                                      gctUINT iteIndex,
                                      gctBOOL bUseRawThreadCount,
                                      gctUINT hsPerCPOutputSizePerThread,
                                      gctUINT hsPerPatchOutputSize,
                                      gctUINT* pMaxRawThreadCount,
                                      gctUINT* pMaxAlignedThreadCount,
                                      gctUINT* pRealUsedMaxThreadCount,
                                      gctUINT* pExpectedMaxThreadsPerHwTG,
                                      gctBOOL* pBExceedThreshold)
{
    gctUINT    expectedUSCSize, patchCount;

    *pBExceedThreshold = gcvFALSE;

    _CalcHwThreadCount(VSC_GFX_SHADER_STAGE_HS,
                       baseMaxThreadCount,
                       maxHwTGThreadCount,
                       iteIndex,
                       bUseRawThreadCount,
                       pMaxRawThreadCount,
                       pMaxAlignedThreadCount,
                       pRealUsedMaxThreadCount,
                       pExpectedMaxThreadsPerHwTG);

    patchCount = (gctUINT)ceil((float)(*pRealUsedMaxThreadCount) / outputCtrlPointCount);
    expectedUSCSize = VSC_UTILS_ALIGN(hsPerCPOutputSizePerThread * (*pRealUsedMaxThreadCount) +
                                      patchCount * hsPerPatchOutputSize, 1024);

    /* USC for HS can not exceed max size that HS can hold */
    if (expectedUSCSize > (realUSCThreshold << 10))
    {
        expectedUSCSize = (realUSCThreshold << 10);
        *pBExceedThreshold = gcvTRUE;
    }

    return expectedUSCSize;
}

static gctUINT _AnalyzeHwUSCSizeForDs(gctUINT baseMaxThreadCount,
                                      gctUINT realUSCThreshold,
                                      gctUINT maxHwTGThreadCount,
                                      gctUINT maxResCashWinSize,
                                      gctUINT iteIndex,
                                      gctBOOL bUseRawThreadCount,
                                      gctUINT dsOutputSizePerThread,
                                      gctUINT downStreamVerticesCountPerHwTG,
                                      gctUINT* pMaxRawThreadCount,
                                      gctUINT* pMaxAlignedThreadCount,
                                      gctUINT* pRealUsedMaxThreadCount,
                                      gctUINT* pExpectedMaxThreadsPerHwTG,
                                      gctUINT* pExpectedResCashWinSize,
                                      gctBOOL* pBExceedThreshold)
{
    /* From view of output of DS, it is same as VS, so directly call VS's analyzer.
       DS also can not exceed max size that DS can hold */
    return _AnalyzeHwUSCSizeForVs(baseMaxThreadCount,
                                  realUSCThreshold,
                                  maxHwTGThreadCount,
                                  maxResCashWinSize,
                                  iteIndex,
                                  bUseRawThreadCount,
                                  dsOutputSizePerThread,
                                  downStreamVerticesCountPerHwTG,
                                  pMaxRawThreadCount,
                                  pMaxAlignedThreadCount,
                                  pRealUsedMaxThreadCount,
                                  pExpectedMaxThreadsPerHwTG,
                                  pExpectedResCashWinSize,
                                  pBExceedThreshold);
}

static gctUINT _AnalyzeHwUSCSizeForGs(gctUINT baseMaxThreadCount,
                                      gctUINT realUSCThreshold,
                                      gctUINT maxHwTGThreadCount,
                                      gctBOOL bNeedStreamIdx,
                                      gctBOOL bNeedRestart,
                                      gctUINT iteIndex,
                                      gctBOOL bUseRawThreadCount,
                                      gctUINT gsOutputSizePerThread,
                                      gctUINT gsOutputVertexCount,
                                      gctUINT* pMaxRawThreadCount,
                                      gctUINT* pMaxAlignedThreadCount,
                                      gctUINT* pRealUsedMaxThreadCount,
                                      gctUINT* pExpectedMaxThreadsPerHwTG,
                                      gctUINT* pExpectedGsMetaDataSizePerHwTG,
                                      gctBOOL* pBExceedThreshold)
{
    gctUINT    expectedUSCSize, hwTGCount;
    gctUINT    vertexBlocks = 0;
    gctUINT    i;

    *pBExceedThreshold = gcvFALSE;
    *pExpectedGsMetaDataSizePerHwTG = 0;

    _CalcHwThreadCount(VSC_GFX_SHADER_STAGE_GS,
                       baseMaxThreadCount,
                       maxHwTGThreadCount,
                       iteIndex,
                       bUseRawThreadCount,
                       pMaxRawThreadCount,
                       pMaxAlignedThreadCount,
                       pRealUsedMaxThreadCount,
                       pExpectedMaxThreadsPerHwTG);

    /* Counter. Note can not use *pExpectedMaxThreadsPerHwTG to calc! */
    *pExpectedGsMetaDataSizePerHwTG += (maxHwTGThreadCount * 2);

    /* SO stream index */
    if (bNeedStreamIdx)
    {
        vertexBlocks = (gctUINT)ceil((gctFLOAT)gsOutputVertexCount / 16.0f); /*We can fit in 16 vertices per block. */
    }
    /* Restart */
    else if (bNeedRestart)
    {
        vertexBlocks = (gctUINT)ceil((gctFLOAT)gsOutputVertexCount / 64.0f); /* We can fit in 64 vertices per block. */
    }

    for (i = 0; i < vertexBlocks;  i++)
    {
        if (i == (vertexBlocks - 1))
        {
             *pExpectedGsMetaDataSizePerHwTG += (8 * (*pExpectedMaxThreadsPerHwTG));
        }
        else
        {
             *pExpectedGsMetaDataSizePerHwTG += (8 * maxHwTGThreadCount);
        }
    }

    hwTGCount= (gctUINT)ceil((float)(*pRealUsedMaxThreadCount) / (*pExpectedMaxThreadsPerHwTG));
    expectedUSCSize = VSC_UTILS_ALIGN(gsOutputSizePerThread * (*pRealUsedMaxThreadCount) +
                                      hwTGCount * (*pExpectedGsMetaDataSizePerHwTG), /* Each hw thread group has a meta space */
                                      1024);

    /* USC for GS can not exceed max size that GS can hold */
    if (expectedUSCSize > (realUSCThreshold << 10))
    {
        expectedUSCSize = (realUSCThreshold << 10);
        *pBExceedThreshold = gcvTRUE;
    }

    return expectedUSCSize;
}

static gctUINT _GetInputVerticesCountPerHwTGForHs(SHADER_EXECUTABLE_PROFILE* pHsSEP,
                                                  gctUINT maxHwTGThreadCount,
                                                  SHADER_IO_LINKAGE_INFO* pLinkInfo,
                                                  gctUINT parallelFactor)
{
    gctUINT hsPerCPInputCount, hsPerCPOutputCount, maxPatchesPerHwTG;
    gctBOOL bIsInputRemap = gcvFALSE;

    hsPerCPInputCount = _GetValidInputCount(VSC_GFX_SHADER_STAGE_HS,
                                            &pHsSEP->inputMapping.ioVtxPxl);

    hsPerCPOutputCount = _GetValidOutputCount(VSC_GFX_SHADER_STAGE_HS,
                                              &pHsSEP->outputMapping.ioVtxPxl,
                                              &pLinkInfo->vtxPxlLinkage);

    _GetHsRemapMode(pHsSEP, hsPerCPInputCount, hsPerCPOutputCount, &bIsInputRemap);

    maxPatchesPerHwTG = _GetHsValidMaxPatchesPerHwTG(pHsSEP, maxHwTGThreadCount, bIsInputRemap, parallelFactor);

    return (maxPatchesPerHwTG * pHsSEP->exeHints.nativeHints.prvStates.ts.inputCtrlPointCount);
}

static gctUINT _GetInputVerticesCountPerHwTGForGs(SHADER_EXECUTABLE_PROFILE* pGsSEP, gctUINT expectedMaxThreadsPerHwTG, gctUINT maxHwTGThreadCount)
{
    return (_GetGsValidMaxThreadsPerHwTG(pGsSEP, expectedMaxThreadsPerHwTG, maxHwTGThreadCount) *
            pGsSEP->exeHints.nativeHints.prvStates.gs.inputVtxCount);
}

static gctBOOL _NeedAnalyzeHwUSCProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                                 VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo,
                                                 gctUINT                         maxHwTGThreadCount,
                                                 gctBOOL                         bSeperatedShaders)
{
    SHADER_EXECUTABLE_PROFILE*          pSEP;
    SHADER_IO_LINKAGE_INFO*             pLinkInfo;
    gctBOOL                             bNeedAnalyze = gcvTRUE, bHasPreRAShaderStages = gcvFALSE;
    gctUINT                             stageIdx;

    /* Check whether has pre-RA shaders */
    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
        {
            pSEP = pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP;

            if ((SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) != SHADER_TYPE_PIXEL &&
                (SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) != SHADER_TYPE_GENERAL)
            {
                bHasPreRAShaderStages = gcvTRUE;
                break;
            }
        }
    }

    /* If no USC armed on chip, just bail out */
    if (pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxUSCAttribBufInKbyte == 0)
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
            {
                pSEP = pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP;

                gcmASSERT(pSEP->inputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
                gcmASSERT(pSEP->inputMapping.ioPrim.ioMode == SHADER_IO_MODE_PASSIVE);
                gcmASSERT(pSEP->outputMapping.ioVtxPxl.ioMode == SHADER_IO_MODE_PASSIVE);
                gcmASSERT(pSEP->outputMapping.ioPrim.ioMode == SHADER_IO_MODE_PASSIVE);
            }
        }

        bNeedAnalyze = gcvFALSE;
    }

#if PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
    /* If any pre-RA shader is
        1. not linked to any SHADER unit
        2. or linked to FFU but no outputs of that shader will go to FFU memory
        3. or there are no pre-RA shaders
        then we dont need consider USC because they can not be ran on HW at all */

    if (!bHasPreRAShaderStages && bNeedAnalyze)
    {
        bNeedAnalyze = gcvFALSE;
    }

    if (bNeedAnalyze)
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
            {
                pSEP = pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP;
                pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].outputLinkageInfo;

                if ((SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_PIXEL ||
                    (SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) == SHADER_TYPE_GENERAL)
                {
                    continue;
                }

                if (bSeperatedShaders)
                {
                    if (pLinkInfo->linkedShaderStage == SHADER_TYPE_FFU &&
                        pLinkInfo->vtxPxlLinkage.totalLinkNoCount == 0)
                    {
                        bNeedAnalyze = gcvFALSE;
                        break;
                    }
                }
            }
        }
    }
#endif

    /* If we dont need do USC analysis, we also need set maxThreadsPerHwTG for each
       pre-RA shader because state-programming needs it */
    if (bHasPreRAShaderStages && !bNeedAnalyze)
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
            {
                pSEP = pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP;

                if ((SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) != SHADER_TYPE_PIXEL &&
                    (SHADER_TYPE)DECODE_SHADER_TYPE(pSEP->shVersionType) != SHADER_TYPE_GENERAL)
                {
                    pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxThreadsPerHwTG = maxHwTGThreadCount;
                }
            }
        }
    }

    return bNeedAnalyze;
}

static VSC_ErrCode _AnalyzeHwUSCProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                                 SHADER_HW_INFO**                ppActiveShHwInfoArray,
                                                 gctUINT                         activeHwShaderStageCount,
                                                 VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo,
                                                 gctBOOL                         bSeperatedShaders)
{
    typedef enum _USC_ANALYZE_TRIAL_TYPE
    {
        USC_ANALYZE_TRIAL_TYPE_MANY_HW_TGS = 0,
        USC_ANALYZE_TRIAL_TYPE_1_HW_TG,
        USC_ANALYZE_TRIAL_TYPE_COUNT,
    }USC_ANALYZE_TRIAL_TYPE;

    SHADER_EXECUTABLE_PROFILE*          pSEP;
    SHADER_IO_LINKAGE_INFO*             pLinkInfo;
    gctUINT                             vsOutputSizePerThread = 0, dsOutputSizePerThread = 0;
    gctUINT                             hsPerCPOutputSizePerThread = 0, hsPerPatchOutputSize = 0;
    gctUINT                             gsOutputSizePerThread = 0;
    gctUINT                             baseMaxThreadCount[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             maxRawThreadCount[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             maxAlignedThreadCount[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             realUsedMaxThreadCount[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             expectedMaxThreadsPerHwTG[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             expectedResCashWinSize[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             expectedUSCSize[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {0, 0, 0, 0, 0};
    gctUINT                             expectedGsMetaDataSizePerHwTG = 0;
    gctUINT                             realUSCThreshold[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT] = {31, 31, 31, 63, 0};
    gctBOOL                             bExceedThreshold[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];
    gctUINT                             totalExpectedUSCSize, stageIdx, i, perVtxOutputSize;
    gctINT                              totalAllocatedUSCSizeInKbyte = 0, unAllocatedUSCSizeInKbyte, deltaUSCSizeInKbyte;
    gctUINT                             vsDownStreamVerticesCountPerHwTG, dsDownStreamVerticesCountPerHwTG;
    gctUINT                             maxHwTGThreadCount = pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount * 4;
    gctUINT                             realMaxHwUSCSizeInKbyte = pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxUSCAttribBufInKbyte;
    gctUINT                             maxResCashWinSize = pHwPipelineShsParam->pSysCtx->pCoreSysCtx->hwCfg.maxResultCacheWinSize;
    USC_ANALYZE_TRIAL_TYPE              anaTrialType;
    gctBOOL                             bUSCAlloced;
    gctBOOL                             bVsUSCAllocedSuccess = gcvFALSE;

    /* Check whether we need go on to analyze USC programming hints */
    if (!_NeedAnalyzeHwUSCProgrammingHints(pHwPipelineShsParam, pOutHwShdsLinkInfo, maxHwTGThreadCount, bSeperatedShaders))
    {
        return VSC_ERR_NONE;
    }

    /* !!!
       NOTE that as when programming USC for each g-pipe stages, 1 page will be added into final USC page size,
       in analysis, that extra page will be excluded to ensure correctness. USC page size for each g-pipe stage
       in realUSCThreshold has been reduced by 1 page already. realMaxHwUSCSizeInKbyte will be already adjusted
       by -1 for each active g-pipe stage.
     */

    /* If VS is the only shader of pre-RA, just set it to max size that VS can hold */
    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
    {
        pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP;
        pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].outputLinkageInfo;

        if ((pLinkInfo->linkedShaderStage == SHADER_TYPE_PIXEL) ||
            (pLinkInfo->linkedShaderStage == SHADER_TYPE_FFU && pLinkInfo->vtxPxlLinkage.totalLinkNoCount > 0) ||
            (pLinkInfo->linkedShaderStage == SHADER_TYPE_UNKNOWN && pLinkInfo->vtxPxlLinkage.totalLinkNoCount == 0)) /* VS only */
        {
            realMaxHwUSCSizeInKbyte --;
            realUSCThreshold[VSC_GFX_SHADER_STAGE_VS] = vscMIN(realUSCThreshold[VSC_GFX_SHADER_STAGE_VS],
                                                               realMaxHwUSCSizeInKbyte);

            vsOutputSizePerThread = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_VS,
                                                                        &pSEP->outputMapping.ioVtxPxl,
                                                                        &pLinkInfo->vtxPxlLinkage,
                                                                        gcvFALSE);

            for (i = 1; ; i ++)
            {
                _AnalyzeHwUSCSizeForVs(1,
                                       realUSCThreshold[VSC_GFX_SHADER_STAGE_VS],
                                       maxHwTGThreadCount,
                                       maxResCashWinSize,
                                       i,
                                       gcvTRUE,
                                       vsOutputSizePerThread,
                                       3,
                                       &maxRawThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                       &maxAlignedThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                       &realUsedMaxThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                       &expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_VS],
                                       &expectedResCashWinSize[VSC_GFX_SHADER_STAGE_VS],
                                       &bExceedThreshold[VSC_GFX_SHADER_STAGE_VS]);

                if (bExceedThreshold[VSC_GFX_SHADER_STAGE_VS])
                {
                    break;
                }

                bVsUSCAllocedSuccess = gcvTRUE;

                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.maxUscSizeInKbyte =
                                                                         realUSCThreshold[VSC_GFX_SHADER_STAGE_VS];

                if (i == 1)
                {
                    pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.minUscSizeInKbyte =
                                               pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.maxUscSizeInKbyte;
                }

                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.resultCacheWindowSize =
                                                                             expectedResCashWinSize[VSC_GFX_SHADER_STAGE_VS];

                gcmASSERT(expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_VS] <= maxHwTGThreadCount);
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.maxThreadsPerHwTG =
                                                                             expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_VS];
            }

            if (!bVsUSCAllocedSuccess)
            {
                return VSC_ERR_TOO_MANY_VARYINGS;
            }
            else
            {
                return VSC_ERR_NONE;
            }
        }
    }

    /* We will firstly select a minimum parallelism combination, and then increase it
       to max USC limitation with following relation,
       1. VS + GS + (PS)           ----> thread count ratio = a : 1
       2. VS + HS + DS + (PS)      ----> thread count ratio = b : c : 1(see NOTE),
       3. VS + HS + DS + GS + (PS) ----> thread count ratio = b : c : a(see NOTE) : 1,

       a is input vertex count of GS
       b is input CP count of HS
       c is output CP count of HS

       NOTE: Due to FF tessellator (TPG) will generate lots of domain-location which will kick
             off a DS thread for each, and for the most of cases, we actully don't know exact
             number about how many domain-locations will be generated by FF, so ratio number for
             DS might be changed based on experience number or experimental number.
    */
    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
    {
        pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP;
        pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].outputLinkageInfo;

        vsOutputSizePerThread = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_VS,
                                                                    &pSEP->outputMapping.ioVtxPxl,
                                                                    &pLinkInfo->vtxPxlLinkage,
                                                                    gcvFALSE);

        if (pLinkInfo->linkedShaderStage == SHADER_TYPE_GEOMETRY)
        {
            baseMaxThreadCount[VSC_GFX_SHADER_STAGE_VS] =
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP->exeHints.
                                                                nativeHints.prvStates.gs.inputVtxCount;
        }
        else
        {
            gcmASSERT(pLinkInfo->linkedShaderStage == SHADER_TYPE_HULL);

            baseMaxThreadCount[VSC_GFX_SHADER_STAGE_VS] =
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP->exeHints.
                                                                nativeHints.prvStates.ts.inputCtrlPointCount;
        }

        realMaxHwUSCSizeInKbyte --;
    }

    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP)
    {
        pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP;
        pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].outputLinkageInfo;

        hsPerCPOutputSizePerThread = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_HS,
                                                                         &pSEP->outputMapping.ioVtxPxl,
                                                                         &pLinkInfo->vtxPxlLinkage,
                                                                         gcvFALSE);

        hsPerPatchOutputSize = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_HS,
                                                                   &pSEP->outputMapping.ioPrim,
                                                                   &pLinkInfo->primLinkage,
                                                                   gcvFALSE);

        baseMaxThreadCount[VSC_GFX_SHADER_STAGE_HS] = pSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount;

        realMaxHwUSCSizeInKbyte --;
    }

    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP)
    {
        pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP;
        pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].outputLinkageInfo;

        dsOutputSizePerThread = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_DS,
                                                                    &pSEP->outputMapping.ioVtxPxl,
                                                                    &pLinkInfo->vtxPxlLinkage,
                                                                    gcvFALSE);

        if (pLinkInfo->linkedShaderStage == SHADER_TYPE_GEOMETRY)
        {
            baseMaxThreadCount[VSC_GFX_SHADER_STAGE_DS] =
                pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP->exeHints.
                                                                nativeHints.prvStates.gs.inputVtxCount;
        }
        else
        {
            /* Change it from 1 to 6 for tuning 'Tess test' of GFX4.0 feature test */
            baseMaxThreadCount[VSC_GFX_SHADER_STAGE_DS] = 6;
        }

        realMaxHwUSCSizeInKbyte --;
    }

    if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP)
    {
        pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP;
        pLinkInfo = &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].outputLinkageInfo;

        perVtxOutputSize = _GetTotalOutputsSizeInBytesPerVtxCp(VSC_GFX_SHADER_STAGE_GS,
                                                               &pSEP->outputMapping.ioVtxPxl,
                                                               &pLinkInfo->vtxPxlLinkage,
#if PROGRAMING_OUTPUTS_ON_COMPONENT_GRANULARITY
                                                               gcvTRUE
#else
                                                               gcvFALSE
#endif
                                                               );

        gsOutputSizePerThread = perVtxOutputSize * pSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount;

        baseMaxThreadCount[VSC_GFX_SHADER_STAGE_GS] = 1;

        realMaxHwUSCSizeInKbyte --;
    }

    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        realUSCThreshold[stageIdx] = vscMIN(realUSCThreshold[stageIdx], realMaxHwUSCSizeInKbyte);
    }

    /* Now iterate to find proper usc size for each shader stage. We will do 2 trials, one is
       try to allocate USC that each shader stage can run at max thread count capability of HW
       thread-group; and the 2nd trial is to find whether each stage can run within 1 HW thread
       group. At each trial, we will do internal iteration based on info we have collected for
       minimum parallelism combination, each interal iteration will add one time of base maximum
       thread count for each shader stage. Note that in each internal iteration, the analysis
       seq is gs->ds->hs->vs because some stages (such as vs/ds) need know input vertices count
       of downstream stage */
    for (anaTrialType = 0; anaTrialType < USC_ANALYZE_TRIAL_TYPE_COUNT; anaTrialType ++)
    {
        bUSCAlloced = gcvFALSE;

        /* Iterate based on minimum parallelism combination */
        for (i = 1; ; i ++)
        {
            totalExpectedUSCSize = 0;
            memset(bExceedThreshold, 0, sizeof(gctBOOL)*VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT);
            vsDownStreamVerticesCountPerHwTG = dsDownStreamVerticesCountPerHwTG = 3;

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP)
            {
                pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP;

                expectedUSCSize[VSC_GFX_SHADER_STAGE_GS] =
                                     _AnalyzeHwUSCSizeForGs(baseMaxThreadCount[VSC_GFX_SHADER_STAGE_GS],
                                                            realUSCThreshold[VSC_GFX_SHADER_STAGE_GS],
                                                            maxHwTGThreadCount,
                                                            pSEP->exeHints.nativeHints.prvStates.gs.bHasStreamOut,
                                                            pSEP->exeHints.derivedHints.prvStates.gs.bHasPrimRestartOp,
                                                            i,
                                                            gcvTRUE,
                                                            gsOutputSizePerThread,
                                                            pSEP->exeHints.nativeHints.prvStates.gs.maxOutputVtxCount,
                                                            &maxRawThreadCount[VSC_GFX_SHADER_STAGE_GS],
                                                            &maxAlignedThreadCount[VSC_GFX_SHADER_STAGE_GS],
                                                            &realUsedMaxThreadCount[VSC_GFX_SHADER_STAGE_GS],
                                                            &expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_GS],
                                                            &expectedGsMetaDataSizePerHwTG,
                                                            &bExceedThreshold[VSC_GFX_SHADER_STAGE_GS]);

                totalExpectedUSCSize += expectedUSCSize[VSC_GFX_SHADER_STAGE_GS];

                vsDownStreamVerticesCountPerHwTG =
                dsDownStreamVerticesCountPerHwTG =
                         _GetInputVerticesCountPerHwTGForGs(pSEP,
                                                            expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_GS], maxHwTGThreadCount);
            }

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP)
            {
                expectedUSCSize[VSC_GFX_SHADER_STAGE_DS] =
                                     _AnalyzeHwUSCSizeForDs(baseMaxThreadCount[VSC_GFX_SHADER_STAGE_DS],
                                                            realUSCThreshold[VSC_GFX_SHADER_STAGE_DS],
                                                            maxHwTGThreadCount,
                                                            maxResCashWinSize,
                                                            i,
                                                            (anaTrialType != USC_ANALYZE_TRIAL_TYPE_MANY_HW_TGS),
                                                            dsOutputSizePerThread,
                                                            dsDownStreamVerticesCountPerHwTG,
                                                            &maxRawThreadCount[VSC_GFX_SHADER_STAGE_DS],
                                                            &maxAlignedThreadCount[VSC_GFX_SHADER_STAGE_DS],
                                                            &realUsedMaxThreadCount[VSC_GFX_SHADER_STAGE_DS],
                                                            &expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_DS],
                                                            &expectedResCashWinSize[VSC_GFX_SHADER_STAGE_DS],
                                                            &bExceedThreshold[VSC_GFX_SHADER_STAGE_DS]);

                totalExpectedUSCSize += expectedUSCSize[VSC_GFX_SHADER_STAGE_DS];
            }

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP)
            {
                pSEP = pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP;

                expectedUSCSize[VSC_GFX_SHADER_STAGE_HS] =
                                     _AnalyzeHwUSCSizeForHs(baseMaxThreadCount[VSC_GFX_SHADER_STAGE_HS],
                                                            realUSCThreshold[VSC_GFX_SHADER_STAGE_HS],
                                                            maxHwTGThreadCount,
                                                            pSEP->exeHints.nativeHints.prvStates.ts.outputCtrlPointCount,
                                                            i,
                                                            (anaTrialType != USC_ANALYZE_TRIAL_TYPE_MANY_HW_TGS),
                                                            hsPerCPOutputSizePerThread,
                                                            hsPerPatchOutputSize,
                                                            &maxRawThreadCount[VSC_GFX_SHADER_STAGE_HS],
                                                            &maxAlignedThreadCount[VSC_GFX_SHADER_STAGE_HS],
                                                            &realUsedMaxThreadCount[VSC_GFX_SHADER_STAGE_HS],
                                                            &expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_HS],
                                                            &bExceedThreshold[VSC_GFX_SHADER_STAGE_HS]);

                totalExpectedUSCSize += expectedUSCSize[VSC_GFX_SHADER_STAGE_HS];

                vsDownStreamVerticesCountPerHwTG =
                            _GetInputVerticesCountPerHwTGForHs(pSEP, maxHwTGThreadCount,
                                         &pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].outputLinkageInfo, i);
            }

            if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP)
            {
                expectedUSCSize[VSC_GFX_SHADER_STAGE_VS] =
                                     _AnalyzeHwUSCSizeForVs(baseMaxThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                                            realUSCThreshold[VSC_GFX_SHADER_STAGE_VS],
                                                            maxHwTGThreadCount,
                                                            maxResCashWinSize,
                                                            i,
                                                            (anaTrialType != USC_ANALYZE_TRIAL_TYPE_MANY_HW_TGS),
                                                            vsOutputSizePerThread,
                                                            vsDownStreamVerticesCountPerHwTG,
                                                            &maxRawThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                                            &maxAlignedThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                                            &realUsedMaxThreadCount[VSC_GFX_SHADER_STAGE_VS],
                                                            &expectedMaxThreadsPerHwTG[VSC_GFX_SHADER_STAGE_VS],
                                                            &expectedResCashWinSize[VSC_GFX_SHADER_STAGE_VS],
                                                            &bExceedThreshold[VSC_GFX_SHADER_STAGE_VS]);

                totalExpectedUSCSize += expectedUSCSize[VSC_GFX_SHADER_STAGE_VS];
            }

            gcmASSERT(totalExpectedUSCSize > 0);

            /* All USC pool is not enough to run requested shader stages */
            if (((totalExpectedUSCSize) >> 10) > realMaxHwUSCSizeInKbyte)
            {
                /* Can not fit even minimum requirement? */
                if (i == 1)
                {
                    if (anaTrialType == USC_ANALYZE_TRIAL_TYPE_1_HW_TG)
                    {
                        gcmASSERT(gcvFALSE);
                        return VSC_ERR_OUT_OF_RESOURCE;
                    }
                    else
                    {
                        /* Expect doing next trial */
                        break;
                    }
                }
                else
                {
                    bUSCAlloced = gcvTRUE;

                    break;
                }
            }

            /* Yes, we have found a possible USC combination, fill hw programming hints now */
            for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
            {
                /* If this round of calcing exceeds the threshold, do not consider it because the
                   data is invalid */
                if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP && !bExceedThreshold[stageIdx])
                {
                    pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxUscSizeInKbyte =
                                                                                (expectedUSCSize[stageIdx] >> 10);

                    if (i == 1)
                    {
                        pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.minUscSizeInKbyte =
                                                   pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxUscSizeInKbyte;
                    }

                    pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.resultCacheWindowSize =
                                                                                expectedResCashWinSize[stageIdx];

                    gcmASSERT(expectedMaxThreadsPerHwTG[stageIdx] <= maxHwTGThreadCount);
                    pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxThreadsPerHwTG =
                                                                                expectedMaxThreadsPerHwTG[stageIdx];

                    if (stageIdx == VSC_GFX_SHADER_STAGE_GS)
                    {
                        pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.gsMetaDataSizePerHwTGInBtye =
                                                                                expectedGsMetaDataSizePerHwTG;
                    }

                    pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxParallelFactor = i;
                }
            }

            /* All USC pool has been allocated, just bail out now */
            if (((totalExpectedUSCSize) >> 10) == realMaxHwUSCSizeInKbyte)
            {
                bUSCAlloced = gcvTRUE;

                break;
            }
        }

        /* Do we need do next trial analysis? */
        if (bUSCAlloced && anaTrialType == USC_ANALYZE_TRIAL_TYPE_MANY_HW_TGS)
        {
            break;
        }
    }

    /* Finally, let's assign unallocated pages to critical stage */
    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
        {
            totalAllocatedUSCSizeInKbyte += (gctINT)pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].hwProgrammingHints.maxUscSizeInKbyte;
        }
    }

    if (totalAllocatedUSCSizeInKbyte < (gctINT)realMaxHwUSCSizeInKbyte)
    {
        unAllocatedUSCSizeInKbyte = (gctINT)realMaxHwUSCSizeInKbyte - totalAllocatedUSCSizeInKbyte;

        /* Order of critical stage is DS, HS, GS, VS */
        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].pSEP && unAllocatedUSCSizeInKbyte > 0)
        {
            deltaUSCSizeInKbyte = realUSCThreshold[VSC_GFX_SHADER_STAGE_DS] -
                                        pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].hwProgrammingHints.maxUscSizeInKbyte;

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_DS].hwProgrammingHints.maxUscSizeInKbyte +=
                                        ((unAllocatedUSCSizeInKbyte > deltaUSCSizeInKbyte) ? deltaUSCSizeInKbyte : unAllocatedUSCSizeInKbyte);

            unAllocatedUSCSizeInKbyte -= deltaUSCSizeInKbyte;
        }

        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].pSEP && unAllocatedUSCSizeInKbyte > 0)
        {
            deltaUSCSizeInKbyte = realUSCThreshold[VSC_GFX_SHADER_STAGE_HS] -
                                        pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].hwProgrammingHints.maxUscSizeInKbyte;

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_HS].hwProgrammingHints.maxUscSizeInKbyte +=
                                        ((unAllocatedUSCSizeInKbyte > deltaUSCSizeInKbyte) ? deltaUSCSizeInKbyte : unAllocatedUSCSizeInKbyte);

            unAllocatedUSCSizeInKbyte -= deltaUSCSizeInKbyte;
        }

        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].pSEP && unAllocatedUSCSizeInKbyte > 0)
        {
            deltaUSCSizeInKbyte = realUSCThreshold[VSC_GFX_SHADER_STAGE_GS] -
                                        pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].hwProgrammingHints.maxUscSizeInKbyte;

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_GS].hwProgrammingHints.maxUscSizeInKbyte +=
                                        ((unAllocatedUSCSizeInKbyte > deltaUSCSizeInKbyte) ? deltaUSCSizeInKbyte : unAllocatedUSCSizeInKbyte);

            unAllocatedUSCSizeInKbyte -= deltaUSCSizeInKbyte;
        }

        if (pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].pSEP && unAllocatedUSCSizeInKbyte > 0)
        {
            deltaUSCSizeInKbyte = realUSCThreshold[VSC_GFX_SHADER_STAGE_VS] -
                                        pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.maxUscSizeInKbyte;

            pOutHwShdsLinkInfo->shHwInfoArray[VSC_GFX_SHADER_STAGE_VS].hwProgrammingHints.maxUscSizeInKbyte +=
                                        ((unAllocatedUSCSizeInKbyte > deltaUSCSizeInKbyte) ? deltaUSCSizeInKbyte : unAllocatedUSCSizeInKbyte);

            unAllocatedUSCSizeInKbyte -= deltaUSCSizeInKbyte;
        }
    }

    return VSC_ERR_NONE;
}

static VSC_ErrCode _AnalyzeHwProgrammingHints(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                                              VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo,
                                              gctBOOL                         bSeperatedShaders)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gctUINT             stageIdx, activeHwShaderStageCount = 0;
    gctUINT             totalMachineCodeSize = 0, totalConstRegCount = 0, totalSamplerRegCount = 0;
    SHADER_HW_INFO*     pActiveShHwInfoArray[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];

    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        if (pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP)
        {
            pActiveShHwInfoArray[activeHwShaderStageCount] = &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx];
            totalMachineCodeSize += pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP->countOfMCInst;
            totalConstRegCount += pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP->constantMapping.hwConstRegCount;
            totalSamplerRegCount += pOutHwShdsLinkInfo->shHwInfoArray[stageIdx].pSEP->samplerMapping.hwSamplerRegCount;
            activeHwShaderStageCount ++;
        }
    }

    /* 1. Inst programming hints */
    errCode = _AnalyzeHwInstProgrammingHints(pHwPipelineShsParam,
                                             pActiveShHwInfoArray,
                                             activeHwShaderStageCount,
                                             totalMachineCodeSize,
                                             pOutHwShdsLinkInfo);
    ON_ERROR(errCode, "Analyze HW inst progrmamming hints");

    /* 2. Constant reg programming hints */
    errCode = _AnalyzeHwConstantRFProgrammingHints(pHwPipelineShsParam,
                                                   pActiveShHwInfoArray,
                                                   activeHwShaderStageCount,
                                                   totalConstRegCount,
                                                   pOutHwShdsLinkInfo);
    ON_ERROR(errCode, "Analyze HW constant RF progrmamming hints");

    /* 3. Sampler reg programming hints */
    errCode = _AnalyzeHwSamplerRFProgrammingHints(pHwPipelineShsParam,
                                                  pActiveShHwInfoArray,
                                                  activeHwShaderStageCount,
                                                  totalSamplerRegCount,
                                                  pOutHwShdsLinkInfo);
    ON_ERROR(errCode, "Analyze HW sampler RF progrmamming hints");

    /* 4. Universal storage cache programming hints */
    errCode = _AnalyzeHwUSCProgrammingHints(pHwPipelineShsParam,
                                            pActiveShHwInfoArray,
                                            activeHwShaderStageCount,
                                            pOutHwShdsLinkInfo,
                                            bSeperatedShaders);
    ON_ERROR(errCode, "Analyze HW USC progrmamming hints");

OnError:
    return errCode;
}

static gctBOOL _NeedLinkWithUpperShader(gctBOOL         bSeperatedShaders,
                                        SHADER_HW_INFO* pUpperHwShader,
                                        SHADER_HW_INFO* pLowerHwShader)
{
    gctBOOL needLink = gcvTRUE;

    /* Enable only when we need to program states for seperated program. */
#if PROGRAMING_STATES_FOR_SEPERATED_PROGRAM
    SHADER_TYPE upperShaderType = (SHADER_TYPE)DECODE_SHADER_TYPE(pUpperHwShader->pSEP->shVersionType);
    SHADER_TYPE lowerShaderType = (SHADER_TYPE)DECODE_SHADER_TYPE(pLowerHwShader->pSEP->shVersionType);

    /* If the program is not separable, always need to link. */
    if (!bSeperatedShaders)
    {
        return needLink;
    }

    /* For TES, its upper shader must be TCS. */
    if (lowerShaderType == SHADER_TYPE_DOMAIN)
    {
        if (upperShaderType != SHADER_TYPE_HULL)
        {
            needLink = gcvFALSE;
        }
    }
    /* For PS, its upper shader must be VS, GS or TES. */
    else if (lowerShaderType == SHADER_TYPE_PIXEL)
    {
        if (upperShaderType != SHADER_TYPE_VERTEX   &&
            upperShaderType != SHADER_TYPE_GEOMETRY &&
            upperShaderType != SHADER_TYPE_DOMAIN)
        {
            needLink = gcvFALSE;
        }
    }
    else if (lowerShaderType == SHADER_TYPE_GEOMETRY)
    {
        if (upperShaderType != SHADER_TYPE_VERTEX &&
            upperShaderType != SHADER_TYPE_DOMAIN)
        {
            needLink = gcvFALSE;
        }
    }
#endif

    return needLink = gcvFALSE;
}

gceSTATUS vscLinkHwShaders(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                           VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo,
                           gctBOOL                         bSeperatedShaders)
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    SHADER_HW_INFO*  pPreShaderStageHwInfo = gcvNULL;
    gctUINT          stageIdx;

    errCode = _ValidateHwPipelineShaders(pHwPipelineShsParam);
    ON_ERROR(errCode, "Hw pipeline validation");

    /* 1. Do IO linkage */
    for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
    {
        vscInitializeShaderHWInfo(&pOutHwShdsLinkInfo->shHwInfoArray[stageIdx],
                                  pHwPipelineShsParam->pSEPArray[stageIdx]);

        if (pHwPipelineShsParam->pSEPArray[stageIdx])
        {
            if (pPreShaderStageHwInfo &&
                _NeedLinkWithUpperShader(bSeperatedShaders, pPreShaderStageHwInfo, &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx]))
            {
                /* Low-level link two hw shader stages by I-O */
                errCode = _LinkIoBetweenTwoHwShaderStages(pHwPipelineShsParam,
                                                          pPreShaderStageHwInfo,
                                                          &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx]);
                ON_ERROR(errCode, "Link Io between two hw shader stages");
            }
            else
            {
                /* Only consider input of first active stage */
                errCode = _LinkInputOfFirstHwShaderStage(pHwPipelineShsParam,
                                                         &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx]);
                ON_ERROR(errCode, "Link input of first shader stage");
            }

            pPreShaderStageHwInfo = &pOutHwShdsLinkInfo->shHwInfoArray[stageIdx];
        }
    }

    if (pPreShaderStageHwInfo)
    {
        /* Only consider output of last active stage */
        errCode = _LinkOutputOfLastHwShaderStage(pHwPipelineShsParam, pPreShaderStageHwInfo);
        ON_ERROR(errCode, "Link output of last shader stage");
    }

    /* 2. Determine some HW programming hints */
    errCode = _AnalyzeHwProgrammingHints(pHwPipelineShsParam, pOutHwShdsLinkInfo, bSeperatedShaders);
    ON_ERROR(errCode, "Analyze hw programming hints");

OnError:
    if (errCode != VSC_ERR_NONE)
    {
        for (stageIdx = 0; stageIdx < VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT; stageIdx ++)
        {
            vscFinalizeShaderHWInfo(&pOutHwShdsLinkInfo->shHwInfoArray[stageIdx]);
        }
    }

    return vscERR_CastErrCode2GcStatus(errCode);
}


