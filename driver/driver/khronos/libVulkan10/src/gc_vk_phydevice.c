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

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 24)
#  include <vulkan/vk_android_native_buffer.h>
#endif

#define _GC_OBJ_ZONE    gcvZONE_HAL

const VkExtensionProperties g_DeviceExtensions[] =
{
    {VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_maintenance1},
#if defined(VK_USE_PLATFORM_ANDROID_KHR) && (ANDROID_SDK_VERSION >= 24)
    {VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME, VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION},
#else
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SWAPCHAIN_SPEC_VERSION}
#endif
};

const uint32_t g_DeviceExtensionsCount =
    sizeof(g_DeviceExtensions) / sizeof(g_DeviceExtensions[0]);

#if __VK_NEW_DEVICE_QUEUE
static void __vki_InitializeHWcapsForVSC(
    __vkPhysicalDevice *phyDev
    )
{
    gctUINT32 maxVaryingCount, maxAttribs;
    gctUINT32 samplerCount[gcvPROGRAM_STAGE_LAST] = { 0 };
    gctUINT32 samplerBase[gcvPROGRAM_STAGE_LAST] = { 0 };
    gctUINT32 totalCount = 0;
    gctUINT32 fragmentSizeInKbyte = 0;
    gctUINT32 attribBufSizeInKbyte = 0;
    VSC_HW_CONFIG *pVscHwCfg = &phyDev->vscCoreSysCtx.hwCfg;
    const gcsFEATURE_DATABASE *database = phyDev->phyDevConfig.database;
    uint32_t chipModel = phyDev->phyDevConfig.chipModel;
    uint32_t chipRevision = phyDev->phyDevConfig.chipRevision;
    gctBOOL unifiedConst;
    uint32_t vsConstBase, psConstBase, vsConstMax, psConstMax, constMax;
    gctBOOL supportInteger, needFixForCLX, needFixForCLXE;
    uint32_t maxWorkGroupSize = 0;

    /* Need special handle for CL_X. */
    needFixForCLX =  ((chipModel == gcv2100) ||
        (chipModel == gcv2000 && chipRevision == 0x5108)    ||
        (chipModel == gcv880 && chipRevision == 0x5106)     ||
        (chipModel == gcv880 && chipRevision == 0x5121)     ||
        (chipModel == gcv880 && chipRevision == 0x5122)     ||
        (chipModel == gcv880 && chipRevision == 0x5124));

    needFixForCLXE = (chipModel > gcv2100 || chipModel == gcv1500 ||
                      (chipModel == gcv900 && chipRevision == 0x5250));

    supportInteger = (chipModel > gcv2000 || chipModel == gcv880 || chipModel == gcv1500 ||
                     (chipModel == gcv900 && chipRevision == 0x5250) ||
                     needFixForCLX);

    maxVaryingCount = database->VaryingCount;

    if (database->REG_Halti5)
    {
        gctUINT base = 0;
        samplerCount[gcvPROGRAM_STAGE_FRAGMENT] = 16;
        samplerBase[gcvPROGRAM_STAGE_FRAGMENT] = base;
        base += 16;
        if (database->REG_GeometryShader)
        {
            samplerCount[gcvPROGRAM_STAGE_GEOMETRY] = 16;
            samplerBase[gcvPROGRAM_STAGE_GEOMETRY] = base;
            base += 16;
        }
        if (database->REG_TessellationShaders)
        {
            samplerCount[gcvPROGRAM_STAGE_TES] = 16;
            samplerBase[gcvPROGRAM_STAGE_TES] = base;
            base += 16;
            samplerCount[gcvPROGRAM_STAGE_TCS] = 16;
            samplerBase[gcvPROGRAM_STAGE_TCS] = base;
            base += 16;
        }
        samplerCount[gcvPROGRAM_STAGE_VERTEX] = 16;
        samplerBase[gcvPROGRAM_STAGE_VERTEX] = base;

        samplerBase[gcvPROGRAM_STAGE_COMPUTE] =
        samplerBase[gcvPROGRAM_STAGE_OPENCL] = 0;
    }
    else
    {
        __VK_ASSERT(database->REG_Halti1);
        samplerCount[gcvPROGRAM_STAGE_VERTEX] = 16;
        samplerCount[gcvPROGRAM_STAGE_FRAGMENT] = 16;
        samplerBase[gcvPROGRAM_STAGE_FRAGMENT] = 0;
        samplerBase[gcvPROGRAM_STAGE_VERTEX] = 16;
    }

    totalCount = samplerCount[gcvPROGRAM_STAGE_VERTEX]
               + samplerCount[gcvPROGRAM_STAGE_TCS]
               + samplerCount[gcvPROGRAM_STAGE_TES]
               + samplerCount[gcvPROGRAM_STAGE_GEOMETRY]
               + samplerCount[gcvPROGRAM_STAGE_FRAGMENT];

    samplerCount[gcvPROGRAM_STAGE_COMPUTE] =
    samplerCount[gcvPROGRAM_STAGE_OPENCL]  = totalCount;

    __VK_ASSERT(database->REG_Halti0);
    maxAttribs = database->PIPELINE_32_ATTRIBUTES ? 32 : 16;
    /* Determine constant parameters. */
    {if (database->NumberOfConstants > 256){    unifiedConst = gcvTRUE;
if (database->REG_Halti5){    vsConstBase  = 0xD000;
    psConstBase  = 0xD800;
}else{    vsConstBase  = 0xC000;
    psConstBase  = 0xC000;
}if ((chipModel == gcv880) && ((chipRevision & 0xfff0) == 0x5120)){    vsConstMax   = 512;
    psConstMax   = 64;
    constMax     = 576;
}else{    vsConstMax   = gcmMIN(512, database->NumberOfConstants - 64);
    psConstMax   = gcmMIN(512, database->NumberOfConstants - 64);
    constMax     = database->NumberOfConstants;
}}else if (database->NumberOfConstants == 256){    if (chipModel == gcv2000 && (chipRevision == 0x5118 || chipRevision == 0x5140))    {        unifiedConst = gcvFALSE;
        vsConstBase  = 0x1400;
        psConstBase  = 0x1C00;
        vsConstMax   = 256;
        psConstMax   = 64;
        constMax     = 320;
    }    else    {        unifiedConst = gcvFALSE;
        vsConstBase  = 0x1400;
        psConstBase  = 0x1C00;
        vsConstMax   = 256;
        psConstMax   = 256;
        constMax     = 512;
    }}else{    unifiedConst = gcvFALSE;
    vsConstBase  = 0x1400;
    psConstBase  = 0x1C00;
    vsConstMax   = 168;
    psConstMax   = 64;
    constMax     = 232;
}};


    vsConstMax = vsConstMax;
    psConstMax = psConstMax;
    constMax   = constMax;

    pVscHwCfg->chipModel = chipModel;
    pVscHwCfg->chipRevision = chipRevision;
    pVscHwCfg->maxCoreCount = database->NumShaderCores;
    pVscHwCfg->maxThreadCountPerCore = database->ThreadCount / database->NumShaderCores;
    pVscHwCfg->maxVaryingCount = maxVaryingCount;
    pVscHwCfg->maxAttributeCount = maxAttribs;
    __VK_ASSERT(database->REG_Halti2);
    pVscHwCfg->maxRenderTargetCount = 8;
    pVscHwCfg->maxGPRCountPerCore = 512;
    pVscHwCfg->maxGPRCountPerThread = database->TempRegisters;
    pVscHwCfg->maxHwNativeTotalInstCount = database->InstructionCount;
    pVscHwCfg->maxTotalInstCount = database->InstructionCount;
    pVscHwCfg->maxVSInstCount = database->InstructionCount;
    pVscHwCfg->maxPSInstCount = database->InstructionCount;
    pVscHwCfg->vsInstBufferAddrBase = 0;
    pVscHwCfg->psInstBufferAddrBase = 0;
    pVscHwCfg->maxHwNativeTotalConstRegCount = database->NumberOfConstants;
    pVscHwCfg->maxTotalConstRegCount = database->NumberOfConstants;
    /* Vulkan needs constant reg count for each shader are meanly divided, which means
       constant reg space for each shader can be fixed */
    pVscHwCfg->maxVSConstRegCount = pVscHwCfg->maxTotalConstRegCount/5;
    pVscHwCfg->maxTCSConstRegCount = pVscHwCfg->maxTotalConstRegCount/5;
    pVscHwCfg->maxTESConstRegCount = pVscHwCfg->maxTotalConstRegCount/5;
    pVscHwCfg->maxGSConstRegCount = pVscHwCfg->maxTotalConstRegCount/5;
    pVscHwCfg->maxPSConstRegCount = pVscHwCfg->maxTotalConstRegCount/5;
    pVscHwCfg->vsConstRegAddrBase = vsConstBase;
    /* use VS count for now */
    pVscHwCfg->tcsConstRegAddrBase = vsConstBase;
    pVscHwCfg->tesConstRegAddrBase = vsConstBase;
    pVscHwCfg->gsConstRegAddrBase = vsConstBase;
    pVscHwCfg->psConstRegAddrBase = psConstBase;
    /*
    ** Set sample base and count. Here is the sampler order:
    ** PS: 0~15
    ** CS/CL: 0~31
    ** VS: 16~31
    ** TCS: 32~47
    ** TES: 48~63
    ** GS: 64~79
    */
    pVscHwCfg->maxVSSamplerCount = samplerCount[gcvPROGRAM_STAGE_VERTEX];
    pVscHwCfg->maxTCSSamplerCount = samplerCount[gcvPROGRAM_STAGE_TCS];
    pVscHwCfg->maxTESSamplerCount = samplerCount[gcvPROGRAM_STAGE_TES];
    pVscHwCfg->maxGSSamplerCount = samplerCount[gcvPROGRAM_STAGE_GEOMETRY];
    pVscHwCfg->maxPSSamplerCount = samplerCount[gcvPROGRAM_STAGE_FRAGMENT];
    pVscHwCfg->maxCSSamplerCount = samplerCount[gcvPROGRAM_STAGE_COMPUTE];
    /* Right now there are 5 bits for sampler index, so the max sampler count is 32. */
    pVscHwCfg->maxSamplerCountPerShader = 32;

    pVscHwCfg->maxHwNativeTotalSamplerCount = totalCount;

    if (database->REG_Halti5)
    {
        pVscHwCfg->vsSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_VERTEX];
        pVscHwCfg->psSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_FRAGMENT];
        pVscHwCfg->csSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_COMPUTE];
        pVscHwCfg->tcsSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_TCS];
        pVscHwCfg->tesSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_TES];
        pVscHwCfg->gsSamplerRegNoBase = samplerBase[gcvPROGRAM_STAGE_GEOMETRY];
    }
    else
    {
        pVscHwCfg->vsSamplerRegNoBase = 0;
        pVscHwCfg->psSamplerRegNoBase = 0;
        pVscHwCfg->csSamplerRegNoBase = 0;
        pVscHwCfg->tcsSamplerRegNoBase = 0;
        pVscHwCfg->tesSamplerRegNoBase = 0;
        pVscHwCfg->gsSamplerRegNoBase = 0;
    }
    pVscHwCfg->vertexOutputBufferSize = database->VertexOutputBufferSize;
    pVscHwCfg->vertexCacheSize = database->VertexCacheSize;
    pVscHwCfg->ctxStateCount = 0;

    if ((!database->SH_SNAP2PAGE_FIX || !database->SH_SNAP2PAGE_MAXPAGES_FIX) &&
        (database->REG_GeometryShader && database->REG_TessellationShaders))
    {
        fragmentSizeInKbyte = 5;
    }
    if (database->REG_Halti5)
    {

        static const gctFLOAT s_uscCacheRatio[] =
        {
            1.0f,
            0.5f,
            0.25f,
            0.125f,
            0.0625f,
            0.03125f,
            0.75f,
            0.0f,
        };

        attribBufSizeInKbyte = (uint32_t)
           (database->USC_MAX_PAGES - (database->L1CacheSize * s_uscCacheRatio[phyDev->phyDevConfig.options.uscL1CacheRatio]));
        attribBufSizeInKbyte -= fragmentSizeInKbyte;
    }

    pVscHwCfg->maxUSCAttribBufInKbyte = database->REG_Halti5 ? attribBufSizeInKbyte : 0;
    pVscHwCfg->maxLocalMemSizeInByte = attribBufSizeInKbyte * 1024;
    pVscHwCfg->maxResultCacheWinSize = database->RESULT_WINDOW_MAX_SIZE;

    /*
    ** 1) Use the DEVICE_MAX_WORK_GROUP_SIZE as the default workGroupSize for a shader.
    ** 2) When we need to use the workGroupSize to calculate the maxRegCount(e.g., use BARRIER in shader),
    **    use initWorkGroupSizeToCalcRegCount as the workGroupSize. And we may also reduce it to use more HW registers.
    */
    if (pVscHwCfg->maxCoreCount >= 4)
    {
        maxWorkGroupSize = 256;
    }
    else if (pVscHwCfg->maxCoreCount == 2)
    {
        maxWorkGroupSize = 64;
    }
    else
    {
        gcmASSERT(pVscHwCfg->maxCoreCount == 1);
        maxWorkGroupSize = 32;
    }
    maxWorkGroupSize = gcmMIN(database->ThreadCount, 128);
    pVscHwCfg->initWorkGroupSizeToCalcRegCount       = 128;
    pVscHwCfg->maxWorkGroupSize                      = maxWorkGroupSize;
    pVscHwCfg->minWorkGroupSize                      = 1;

    pVscHwCfg->hwFeatureFlags.hasHalti0 = database->REG_Halti0;
    pVscHwCfg->hwFeatureFlags.hasHalti1 = database->REG_Halti1;
    pVscHwCfg->hwFeatureFlags.hasHalti2 = database->REG_Halti2;
    pVscHwCfg->hwFeatureFlags.hasHalti3 = database->REG_Halti3;
    pVscHwCfg->hwFeatureFlags.hasHalti4 = database->REG_Halti4;
    pVscHwCfg->hwFeatureFlags.hasHalti5 = database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.supportGS = database->REG_GeometryShader;
    pVscHwCfg->hwFeatureFlags.supportTS = database->REG_TessellationShaders;
    pVscHwCfg->hwFeatureFlags.supportUSC = database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.supportInteger = supportInteger;
    pVscHwCfg->hwFeatureFlags.hasSignFloorCeil = database->REG_ExtraShaderInstructions0;
    pVscHwCfg->hwFeatureFlags.hasSqrtTrig = database->REG_ExtraShaderInstructions1;
    pVscHwCfg->hwFeatureFlags.hasNewSinCosLogDiv = database->REG_ExtraShaderInstructions2;
    pVscHwCfg->hwFeatureFlags.hasMediumPrecision = database->REG_MediumPrecision;
    pVscHwCfg->hwFeatureFlags.canBranchOnImm = database->REG_Halti2;
    pVscHwCfg->hwFeatureFlags.supportDual16 = database->REG_Halti2;
    pVscHwCfg->hwFeatureFlags.hasBugFix8 = database->REG_BugFixes8;
    pVscHwCfg->hwFeatureFlags.hasBugFix10 = database->REG_BugFixes10;
    pVscHwCfg->hwFeatureFlags.hasBugFix11 = database->REG_BugFixes11;
    pVscHwCfg->hwFeatureFlags.hasSelectMapSwizzleFix = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasSamplePosSwizzleFix = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasLoadAttrOOBFix = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasSHEnhance2 = database->REG_SHEnhancements2;
    pVscHwCfg->hwFeatureFlags.hasInstCache = database->REG_InstructionCache;
    pVscHwCfg->hwFeatureFlags.instBufferUnified = gcvTRUE;
    pVscHwCfg->hwFeatureFlags.constRegFileUnified = unifiedConst;
    pVscHwCfg->hwFeatureFlags.samplerRegFileUnified = database->REG_UnifiedSamplers;
    pVscHwCfg->hwFeatureFlags.bigEndianMI = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.raPushPosW = database->REG_ShaderGetsW;
    pVscHwCfg->hwFeatureFlags.vtxInstanceIdAsAttr = database->REG_Halti2;
    pVscHwCfg->hwFeatureFlags.vtxInstanceIdAsInteger = database->REG_Halti2;
    pVscHwCfg->hwFeatureFlags.hasSHEnhance3 = database->REG_SHEnhancements3;
    pVscHwCfg->hwFeatureFlags.rtneRoundingEnabled = database->REG_SHEnhancements2;
    pVscHwCfg->hwFeatureFlags.hasInstCachePrefetch = database->SH_ICACHE_PREFETCH;
    pVscHwCfg->hwFeatureFlags.hasThreadWalkerInPS = database->REG_ThreadWalkerInPS;
    pVscHwCfg->hwFeatureFlags.has32Attributes = database->PIPELINE_32_ATTRIBUTES;
    pVscHwCfg->hwFeatureFlags.newSteeringICacheFlush = database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.gsSupportEmit = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasSamplerBaseOffset = database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.supportStreamOut = database->HWTFB;
    pVscHwCfg->hwFeatureFlags.supportZeroAttrsInFE = database->REG_Halti4;
    pVscHwCfg->hwFeatureFlags.outputCountFix = (chipRevision >= 0x5240);
    pVscHwCfg->hwFeatureFlags.varyingPackingLimited = (chipModel < gcv1000);
    pVscHwCfg->hwFeatureFlags.highpVaryingShift = database->REG_Halti3;
    pVscHwCfg->hwFeatureFlags.needCLXFixes = needFixForCLX;
    pVscHwCfg->hwFeatureFlags.needCLXEFixes = needFixForCLXE;
    pVscHwCfg->hwFeatureFlags.robustAtomic = ((chipRevision >= 0x5400) && (chipRevision < 0x6000));
    pVscHwCfg->hwFeatureFlags.newGPIPE = database->NEW_GPIPE;
    pVscHwCfg->hwFeatureFlags.flatDual16Fix = database->SH_FLAT_INTERPOLATION_DUAL16_FIX || !database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.supportEVIS = database->REG_Evis;
    pVscHwCfg->hwFeatureFlags.supportImgAtomic = database->SH_SUPPORT_V4;
    pVscHwCfg->hwFeatureFlags.supportAdvancedInsts = database->SH_ADVANCED_INSTR;
    pVscHwCfg->hwFeatureFlags.noOneConstLimit = database->SH_NO_ONECONST_LIMIT;
    pVscHwCfg->hwFeatureFlags.hasUniformB0 = database->SH_NO_INDEX_CONST_ON_A0;
    pVscHwCfg->hwFeatureFlags.hasSampleMaskInR0ZWFix = database->PSIO_SAMPLEMASK_IN_R0ZW_FIX;
    pVscHwCfg->hwFeatureFlags.supportOOBCheck = database->ROBUSTNESS;
    pVscHwCfg->hwFeatureFlags.hasUniversalTexld = database->TX_INTEGER_COORDINATE;
    pVscHwCfg->hwFeatureFlags.hasUniversalTexldV2 = database->TX_INTEGER_COORDINATE_V2;
    pVscHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp = database->SH_IMG_LDST_ON_TEMP;
    pVscHwCfg->hwFeatureFlags.hasICacheAllocCountFix = database->SH_ICACHE_ALLOC_COUNT_FIX;
    pVscHwCfg->hwFeatureFlags.hasPSIOInterlock = database->PSIO_INTERLOCK;
    pVscHwCfg->hwFeatureFlags.support128BppImage = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.supportMSAATexture = database->REG_Halti4;
    pVscHwCfg->hwFeatureFlags.supportPerCompDepForLS = !database->REG_Halti5;
    pVscHwCfg->hwFeatureFlags.supportImgAddr = database->REG_Halti3;
    pVscHwCfg->hwFeatureFlags.hasUscGosAddrFix = database->USC_GOS_ADDR_FIX;
    pVscHwCfg->hwFeatureFlags.multiCluster = database->MULTI_CLUSTER;
    /* Now LODQ can't return the correct raw LOD value as spec require. */
    pVscHwCfg->hwFeatureFlags.hasLODQFix = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasImageLoadEnableFix = database->SH_IMAGE_ENABLE_FIX;

    return;
}

#endif

static void __vki_InitializeShaderCaps(
    __vkPhysicalDevice *phyDev
    )
{
    gcsGLSLCaps *shaderCaps = &phyDev->shaderCaps;
    VkPhysicalDeviceProperties *phyDevProp = &phyDev->phyDevProp;
    const gcsFEATURE_DATABASE *database = phyDev->phyDevConfig.database;
    uint32_t i;

    /* GLSL extension string. */
    shaderCaps->extensions = gcvNULL;
    shaderCaps->maxDrawBuffers = phyDevProp->limits.maxColorAttachments;
    shaderCaps->maxSamples = database->REG_MSAA ? 4 : 1;
    shaderCaps->maxVertTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxCmptTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxFragTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxTcsTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxTesTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxGsTextureImageUnits = phyDevProp->limits.maxPerStageDescriptorSampledImages;
    shaderCaps->maxCombinedTextureImageUnits = phyDevProp->limits.maxDescriptorSetSampledImages;
    shaderCaps->maxTextureSamplers = phyDevProp->limits.maxDescriptorSetSamplers;
    shaderCaps->minProgramTexelOffset = phyDevProp->limits.minTexelOffset;
    shaderCaps->maxProgramTexelOffset = phyDevProp->limits.maxTexelOffset;
    shaderCaps->minProgramTexGatherOffset = phyDevProp->limits.minTexelGatherOffset;
    shaderCaps->maxProgramTexGatherOffset = phyDevProp->limits.maxTexelGatherOffset;

    shaderCaps->maxUserVertAttributes = phyDevProp->limits.maxVertexInputAttributes;
    shaderCaps->maxBuildInVertAttributes = 2;
    shaderCaps->maxVertAttributes = shaderCaps->maxUserVertAttributes + shaderCaps->maxBuildInVertAttributes;
    shaderCaps->maxVertOutVectors = (phyDevProp->limits.maxVertexOutputComponents / 4);
    shaderCaps->maxVaryingVectors = shaderCaps->maxVertOutVectors - 1;
    shaderCaps->maxFragInVectors = (phyDevProp->limits.maxFragmentInputComponents / 4);
    shaderCaps->maxTcsOutVectors = (phyDevProp->limits.maxTessellationControlPerVertexOutputComponents / 4);
    shaderCaps->maxTcsOutPatchVectors = (phyDevProp->limits.maxTessellationControlPerPatchOutputComponents / 4);
    shaderCaps->maxTcsOutTotalVectors = (phyDevProp->limits.maxTessellationControlTotalOutputComponents / 4);
    shaderCaps->maxTesOutVectors = (phyDevProp->limits.maxTessellationEvaluationOutputComponents / 4);
    shaderCaps->maxGsOutVectors = (phyDevProp->limits.maxGeometryOutputComponents / 4);
    shaderCaps->maxTcsInVectors = (phyDevProp->limits.maxTessellationControlPerVertexInputComponents / 4);
    shaderCaps->maxTesInVectors = (phyDevProp->limits.maxTessellationEvaluationInputComponents / 4);
    shaderCaps->maxGsInVectors = (phyDevProp->limits.maxGeometryInputComponents / 4);
    shaderCaps->maxGsOutTotalVectors = (phyDevProp->limits.maxGeometryTotalOutputComponents / 4);

    shaderCaps->maxVertUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxFragUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxCmptUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxTcsUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxTesUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxGsUniformVectors = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));
    shaderCaps->maxUniformLocations = (phyDevProp->limits.maxPushConstantsSize / (4 * 4));

    /* buffer bindings */
    shaderCaps->uniformBufferOffsetAlignment = (gctUINT)phyDevProp->limits.minUniformBufferOffsetAlignment;
    shaderCaps->maxUniformBufferBindings = phyDevProp->limits.maxDescriptorSetUniformBuffers;
    shaderCaps->maxVertUniformBlocks = phyDevProp->limits.maxPerStageDescriptorUniformBuffers;
    shaderCaps->maxFragUniformBlocks = phyDevProp->limits.maxPerStageDescriptorUniformBuffers;
    shaderCaps->maxCmptUniformBlocks = phyDevProp->limits.maxPerStageDescriptorUniformBuffers;
    shaderCaps->maxTcsUniformBlocks = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorUniformBuffers : 0;
    shaderCaps->maxTesUniformBlocks = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorUniformBuffers : 0;
    shaderCaps->maxGsUniformBlocks = phyDev->phyDevFeatures.geometryShader ?
        phyDevProp->limits.maxPerStageDescriptorUniformBuffers : 0;
    shaderCaps->maxCombinedUniformBlocks = phyDevProp->limits.maxDescriptorSetUniformBuffers;
    shaderCaps->maxUniformBlockSize = phyDevProp->limits.maxUniformBufferRange;

    shaderCaps->maxCombinedVertUniformComponents = shaderCaps->maxVertUniformVectors * 4
                                                 + shaderCaps->maxVertUniformBlocks * shaderCaps->maxUniformBlockSize / 4;

    shaderCaps->maxCombinedFragUniformComponents = shaderCaps->maxFragUniformVectors * 4
                                                 + shaderCaps->maxFragUniformBlocks * shaderCaps->maxUniformBlockSize / 4;

    shaderCaps->maxCombinedCmptUniformComponents = shaderCaps->maxCmptUniformVectors * 4
                                                 + shaderCaps->maxCmptUniformBlocks * shaderCaps->maxUniformBlockSize / 4;

    shaderCaps->maxCombinedTcsUniformComponents = shaderCaps->maxTcsUniformVectors * 4
                                                + shaderCaps->maxTcsUniformBlocks * shaderCaps->maxUniformBlockSize / 4;

    shaderCaps->maxCombinedTesUniformComponents = shaderCaps->maxTesUniformVectors * 4
                                                + shaderCaps->maxTesUniformBlocks * shaderCaps->maxUniformBlockSize / 4;

    shaderCaps->maxCombinedGsUniformComponents = shaderCaps->maxGsUniformVectors * 4
                                                + shaderCaps->maxGsUniformBlocks * shaderCaps->maxUniformBlockSize / 4;


    shaderCaps->maxVertAtomicCounterBuffers = 0;
    shaderCaps->maxFragAtomicCounterBuffers = 0;
    shaderCaps->maxCmptAtomicCounterBuffers = 0;
    shaderCaps->maxTcsAtomicCounterBuffers = 0;
    shaderCaps->maxTesAtomicCounterBuffers = 0;
    shaderCaps->maxGsAtomicCounterBuffers = 0;
    shaderCaps->maxVertAtomicCounters = shaderCaps->maxVertAtomicCounterBuffers * 8;
    shaderCaps->maxFragAtomicCounters = shaderCaps->maxFragAtomicCounterBuffers * 8;
    shaderCaps->maxCmptAtomicCounters = shaderCaps->maxCmptAtomicCounterBuffers * 8;
    shaderCaps->maxTcsAtomicCounters = shaderCaps->maxTcsAtomicCounterBuffers * 8;
    shaderCaps->maxTesAtomicCounters = shaderCaps->maxTesAtomicCounterBuffers * 8;
    shaderCaps->maxGsAtomicCounters = shaderCaps->maxGsAtomicCounterBuffers * 8;
    shaderCaps->maxCombinedAtomicCounterBuffers = __VK_MAX(shaderCaps->maxVertAtomicCounterBuffers +
                                                           shaderCaps->maxFragAtomicCounterBuffers +
                                                           shaderCaps->maxTcsAtomicCounterBuffers +
                                                           shaderCaps->maxTesAtomicCounterBuffers +
                                                           shaderCaps->maxGsAtomicCounterBuffers,
                                                           shaderCaps->maxCmptAtomicCounterBuffers);
    shaderCaps->maxCombinedAtomicCounters = shaderCaps->maxCombinedAtomicCounterBuffers * 8;
    shaderCaps->maxAtomicCounterBufferBindings = 0;
    shaderCaps->maxAtomicCounterBufferSize = 0;

    shaderCaps->shaderStorageBufferOffsetAlignment = (gctUINT)phyDevProp->limits.minStorageBufferOffsetAlignment;
    shaderCaps->maxVertShaderStorageBlocks = phyDevProp->limits.maxPerStageDescriptorStorageBuffers;
    shaderCaps->maxFragShaderStorageBlocks = phyDevProp->limits.maxPerStageDescriptorStorageBuffers;
    shaderCaps->maxCmptShaderStorageBlocks = phyDevProp->limits.maxPerStageDescriptorStorageBuffers;
    shaderCaps->maxTcsShaderStorageBlocks = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageBuffers : 0;
    shaderCaps->maxTesShaderStorageBlocks = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageBuffers : 0;
    shaderCaps->maxGsShaderStorageBlocks = phyDev->phyDevFeatures.geometryShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageBuffers : 0;
    shaderCaps->maxCombinedShaderStorageBlocks = phyDevProp->limits.maxDescriptorSetStorageBuffers;
    shaderCaps->maxShaderStorageBufferBindings = shaderCaps->maxCombinedShaderStorageBlocks;
    shaderCaps->maxShaderBlockSize = phyDevProp->limits.maxStorageBufferRange;

    shaderCaps->maxXfbInterleavedComponents = 0;
    shaderCaps->maxXfbSeparateComponents = 0;
    shaderCaps->maxXfbSeparateAttribs = 0;

    shaderCaps->maxProgErrStrLen = 256;

    /* Image limits  */
    shaderCaps->maxVertexImageUniform = phyDevProp->limits.maxPerStageDescriptorStorageImages;
    shaderCaps->maxFragImageUniform = phyDevProp->limits.maxPerStageDescriptorStorageImages;
    shaderCaps->maxCmptImageUniform = phyDevProp->limits.maxPerStageDescriptorStorageImages;
    shaderCaps->maxTcsImageUniform = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageImages : 0;
    shaderCaps->maxTesImageUniform = phyDev->phyDevFeatures.tessellationShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageImages : 0;
    shaderCaps->maxGsImageUniform = phyDev->phyDevFeatures.geometryShader ?
        phyDevProp->limits.maxPerStageDescriptorStorageImages : 0;
    shaderCaps->maxImageUnit = phyDevProp->limits.maxDescriptorSetStorageImages;
    shaderCaps->maxCombinedImageUniform = phyDevProp->limits.maxDescriptorSetStorageImages;

    shaderCaps->maxCombinedShaderOutputResource = shaderCaps->maxCombinedImageUniform + shaderCaps->maxCombinedShaderStorageBlocks;

    /* Compute limits */
    for (i = 0; i < 3; i++)
    {
        shaderCaps->maxWorkGroupSize[i] = phyDevProp->limits.maxComputeWorkGroupSize[i];
        shaderCaps->maxWorkGroupCount[i] = phyDevProp->limits.maxComputeWorkGroupCount[i];
    }
    shaderCaps->maxWorkGroupInvocation = phyDevProp->limits.maxComputeWorkGroupInvocations;
    shaderCaps->maxShareMemorySize = phyDevProp->limits.maxComputeSharedMemorySize;

    /* TS-only limits */
    shaderCaps->maxTessPatchVertices = phyDevProp->limits.maxTessellationPatchSize;
    shaderCaps->maxTessGenLevel = phyDevProp->limits.maxTessellationGenerationLevel;
    shaderCaps->tessPatchPR = VK_TRUE;

    /* GS-only limits */
    shaderCaps->maxGsOutVertices = phyDevProp->limits.maxGeometryOutputVertices;
    shaderCaps->provokingVertex = gcvPROVOKING_VERTEX_UNDEFINE;
    shaderCaps->maxGsInvocationCount = phyDevProp->limits.maxGeometryShaderInvocations;

    gcInitializeCompiler(gcvPATCH_INVALID, &phyDev->vscCoreSysCtx.hwCfg, &phyDev->shaderCaps);

#if __VK_NEW_DEVICE_QUEUE
    __vki_InitializeHWcapsForVSC(phyDev);
#else
    __VK_VERIFY_OK(gcQueryShaderCompilerHwCfg(gcvNULL, &phyDev->vscCoreSysCtx.hwCfg));

    /* Vulkan needs constant reg count for each shader are meanly divided, which means
       constant reg space for each shader can be fixed */
    phyDev->vscCoreSysCtx.hwCfg.maxVSConstRegCount =
    phyDev->vscCoreSysCtx.hwCfg.maxTCSConstRegCount =
    phyDev->vscCoreSysCtx.hwCfg.maxTESConstRegCount =
    phyDev->vscCoreSysCtx.hwCfg.maxGSConstRegCount =
    phyDev->vscCoreSysCtx.hwCfg.maxPSConstRegCount = phyDev->vscCoreSysCtx.hwCfg.maxTotalConstRegCount/5;
#endif
    /* VSC private data */
    vscCreatePrivateData(&phyDev->vscCoreSysCtx, &phyDev->vscCoreSysCtx.hPrivData, gcvFALSE);
    return;
}

static void __vki_InitializePhysicalDeviceFeatures(
    VkPhysicalDevice physicalDevice
    )
{
    __vkPhysicalDevice * phyDev = (__vkPhysicalDevice *)physicalDevice;
    gcsHAL_INTERFACE iface;
    __vkPhysicalDeviceConfig *phyDevConfig = &phyDev->phyDevConfig;

    iface.command = gcvHAL_QUERY_CHIP_IDENTITY;

    __VK_VERIFY_OK(__vk_DeviceControl(&iface, 0));

    phyDevConfig->chipModel = iface.u.QueryChipIdentity.chipModel;
    phyDevConfig->chipRevision = iface.u.QueryChipIdentity.chipRevision;
    phyDevConfig->productID = iface.u.QueryChipIdentity.productID;
    phyDevConfig->customerID = iface.u.QueryChipIdentity.customerID;
    phyDevConfig->ecoID = iface.u.QueryChipIdentity.ecoID;
    phyDevConfig->chipFlags = iface.u.QueryChipIdentity.chipFlags;

    phyDevConfig->database = gcQueryFeatureDB(
        phyDevConfig->chipModel,
        phyDevConfig->chipRevision,
        phyDevConfig->productID,
        phyDevConfig->ecoID,
        phyDevConfig->customerID);

    iface.command = gcvHAL_QUERY_CHIP_OPTION;
    __VK_VERIFY_OK(__vk_DeviceControl(&iface, 0));
    phyDevConfig->options = iface.u.QueryChipOptions;

    phyDev->phyDevFeatures.robustBufferAccess                      = VK_TRUE; /* phyDevConfig->database->ROBUSTNESS */
    phyDev->phyDevFeatures.fullDrawIndexUint32                     = VK_FALSE;
    phyDev->phyDevFeatures.imageCubeArray                          = phyDevConfig->database->REG_Halti5;
    phyDev->phyDevFeatures.independentBlend                        = phyDevConfig->database->REG_Halti5;
    phyDev->phyDevFeatures.geometryShader                          = phyDevConfig->database->REG_GeometryShader;
    phyDev->phyDevFeatures.tessellationShader                      = phyDevConfig->database->REG_TessellationShaders;
    phyDev->phyDevFeatures.sampleRateShading                       = phyDevConfig->database->MSAA_SHADING;
    phyDev->phyDevFeatures.dualSrcBlend                            = VK_FALSE;
    phyDev->phyDevFeatures.logicOp                                 = VK_FALSE;
    phyDev->phyDevFeatures.multiDrawIndirect                       = VK_FALSE;
    phyDev->phyDevFeatures.depthClamp                              = VK_FALSE;
    phyDev->phyDevFeatures.depthBiasClamp                          = VK_FALSE;
    phyDev->phyDevFeatures.fillModeNonSolid                        = VK_FALSE;
    phyDev->phyDevFeatures.depthBounds                             = VK_FALSE;
    phyDev->phyDevFeatures.wideLines                               = phyDevConfig->database->REG_WideLine;
    phyDev->phyDevFeatures.largePoints                             = VK_TRUE;
    phyDev->phyDevFeatures.alphaToOne                              = VK_FALSE;
    phyDev->phyDevFeatures.multiViewport                           = VK_FALSE;
    phyDev->phyDevFeatures.samplerAnisotropy                       = phyDevConfig->database->REG_Halti0 && !phyDevConfig->database->NO_ANISTRO_FILTER;
    phyDev->phyDevFeatures.textureCompressionETC2                  = phyDevConfig->database->REG_Halti2;
    phyDev->phyDevFeatures.textureCompressionASTC_LDR              = phyDevConfig->database->REG_TXEnhancements4 &&
                                                                     phyDevConfig->database->TX_ASTC_MULTISLICE_FIX &&
                                                                     !phyDevConfig->database->NO_ASTC;
    phyDev->phyDevFeatures.textureCompressionBC                    = VK_FALSE;
    phyDev->phyDevFeatures.occlusionQueryPrecise                   = VK_TRUE;
    phyDev->phyDevFeatures.pipelineStatisticsQuery                 = VK_FALSE;
    phyDev->phyDevFeatures.vertexPipelineStoresAndAtomics          = VK_FALSE;
    phyDev->phyDevFeatures.fragmentStoresAndAtomics                = phyDevConfig->database->REG_Halti2;
    phyDev->phyDevFeatures.shaderTessellationAndGeometryPointSize  = phyDevConfig->database->REG_TessellationShaders;
    phyDev->phyDevFeatures.shaderImageGatherExtended               = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageImageExtendedFormats       = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageImageMultisample           = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageImageReadWithoutFormat     = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageImageWriteWithoutFormat    = VK_FALSE;
    phyDev->phyDevFeatures.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
    phyDev->phyDevFeatures.shaderSampledImageArrayDynamicIndexing  = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageBufferArrayDynamicIndexing = VK_FALSE;
    phyDev->phyDevFeatures.shaderStorageImageArrayDynamicIndexing  = VK_FALSE;
    phyDev->phyDevFeatures.shaderClipDistance                      = VK_FALSE;
    phyDev->phyDevFeatures.shaderCullDistance                      = VK_FALSE;
    phyDev->phyDevFeatures.shaderFloat64                           = VK_FALSE;
    phyDev->phyDevFeatures.shaderInt64                             = VK_FALSE;
    phyDev->phyDevFeatures.shaderInt16                             = VK_FALSE;
    phyDev->phyDevFeatures.shaderResourceResidency                 = VK_FALSE;
    phyDev->phyDevFeatures.shaderResourceMinLod                    = VK_FALSE;
    phyDev->phyDevFeatures.sparseBinding                           = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidencyBuffer                   = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidencyImage2D                  = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidencyImage3D                  = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidency2Samples                 = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidency4Samples                 = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidency8Samples                 = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidency16Samples                = VK_FALSE;
    phyDev->phyDevFeatures.sparseResidencyAliased                  = VK_FALSE;
    phyDev->phyDevFeatures.variableMultisampleRate                 = VK_FALSE;
}

static void __vki_InitializePhysicalDeviceName(
    __vkPhysicalDevice *phyDev
    )
{
    gctUINT i;
    uint32_t chipID;
    char *  chipNameBase;
    char * chipName;
    const char companyName[] = "VeriSilicon ";
    VkBool32 foundID = VK_FALSE;

    __VK_MEMZERO(&phyDev->phyDevProp.deviceName[0], sizeof(char) * VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
    gcoOS_StrCopySafe(&phyDev->phyDevProp.deviceName[0], VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, companyName);

    chipName = chipNameBase = &phyDev->phyDevProp.deviceName[0] + sizeof(companyName);
    if (phyDev->phyDevConfig.database->REG_HasChipProductReg)
    {
        uint32_t productID = phyDev->phyDevConfig.productID;
        uint32_t type = (((((gctUINT32) (productID)) >> (0 ? 27:24)) & ((gctUINT32) ((((1 ? 27:24) - (0 ? 27:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 27:24) - (0 ? 27:24) + 1)))))) );
        uint32_t grade = (((((gctUINT32) (productID)) >> (0 ? 3:0)) & ((gctUINT32) ((((1 ? 3:0) - (0 ? 3:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 3:0) - (0 ? 3:0) + 1)))))) );

        chipID = (((((gctUINT32) (productID)) >> (0 ? 23:4)) & ((gctUINT32) ((((1 ? 23:4) - (0 ? 23:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:4) - (0 ? 23:4) + 1)))))) );

        switch (type)
        {
        case 0:
            *chipName++ = 'G';
            *chipName++ = 'C';
            break;
        case 1:
            *chipName++ = 'D';
            *chipName++ = 'E';
            *chipName++ = 'C';
            break;
        case 2:
            *chipName++ = 'D';
            *chipName++ = 'C';
            break;
        case 3:
            *chipName++ = 'V';
            *chipName++ = 'G';
            break;
        case 4:
            *chipName++ = 'S';
            *chipName++ = 'C';
            break;
        case 5:
            *chipName++ = 'V';
            *chipName++ = 'P';
            break;
        default:
            *chipName++ = '?';
            *chipName++ = '?';
            gcmPRINT("VK: Invalid product type");
            break;
        }

        /* Translate the ID. */
        for (i = 0; i < 8; i++)
        {
            /* Get the current digit. */
            gctUINT8 digit = (gctUINT8) ((chipID >> 28) & 0xFF);

            /* Append the digit to the string. */
            if (foundID || digit)
            {
                *chipName++ = '0' + digit;
                foundID = VK_TRUE;
            }

            /* Advance to the next digit. */
            chipID <<= 4;
        }

        switch (grade)
        {
        case 0:             /* Normal id */
        default:
            break;

        case 1:
            gcoOS_StrCatSafe(chipNameBase, 32, "Nano");
            break;

        case 2:
            gcoOS_StrCatSafe(chipNameBase, 32, "L");
            break;

        case 3:
            gcoOS_StrCatSafe(chipNameBase, 32, "UL");
            break;

        case 4:
            gcoOS_StrCatSafe(chipNameBase, 32, "XS");
            break;

        case 5:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra");
            break;

        case 6:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoLite");
            break;

        case 7:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra3");
            break;

        case 8:
            gcoOS_StrCatSafe(chipNameBase, 32, "XSVX");
            break;

        case 9:
            gcoOS_StrCatSafe(chipNameBase, 32, "NanoUltra2");
            break;

        case 10:
            gcoOS_StrCatSafe(chipNameBase, 32, "LXS");
            break;

        case 11:
            gcoOS_StrCatSafe(chipNameBase, 32, "LXSVX");
            break;

        case 12:
            gcoOS_StrCatSafe(chipNameBase, 32, "ULXS");
            break;

        case 13:
            gcoOS_StrCatSafe(chipNameBase, 32, "VX");
            break;

        case 14:
            gcoOS_StrCatSafe(chipNameBase, 32, "LVX");
            break;

        case 15:
            gcoOS_StrCatSafe(chipNameBase, 32, "ULVX");
            break;
        }
    }
    else
    {
        chipID = phyDev->phyDevConfig.chipModel;

        if (phyDev->phyDevConfig.chipFlags & gcvCHIP_FLAG_GC2000_R2)
        {
            chipID = gcv2000;
        }

        *chipName++ = 'G';
        *chipName++ = 'C';

        /* Translate the ID. */
        for (i = 0; i < 8; i++)
        {
            /* Get the current digit. */
            uint8_t digit = (uint8_t) ((chipID >> 28) & 0xFF);

            /* Append the digit to the string. */
            if (foundID || digit)
            {
                *chipName++ = '0' + digit;
                foundID = VK_TRUE;
            }

            /* Advance to the next digit. */
            chipID <<= 4;
        }

        if (phyDev->phyDevConfig.chipFlags & gcvCHIP_FLAG_GC2000_R2)
        {
            *chipName++ ='+';
        }
    }
    return;
}


static void __vki_InitializePhysicalDevicePorperties(
    VkPhysicalDevice physicalDevice
    )
{
    __vkPhysicalDevice *    phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t                maxTexDim;
    uint32_t                maxRtDim;
    uint32_t                maxAniso;
    uint32_t                maxAttributes;
    uint32_t                maxStreamStride;
    uint32_t                numberOfStreams;
    uint32_t                maxAttribOffset;
    uint32_t                maxVaryings;
    uint32_t                maxSamples, sampleMask = 0;
    uint32_t                shaderStages = 3;
    const gcsFEATURE_DATABASE *database = phyDev->phyDevConfig.database;

    phyDev->phyDevProp.apiVersion    = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);
    phyDev->phyDevProp.driverVersion = __VK_DRIVER_VERSION;
    phyDev->phyDevProp.vendorID      = __VK_DEVICE_VENDOR_ID;
    phyDev->phyDevProp.deviceID      = (database->chipID << 16)|(database->chipVersion & 0xffff);
    phyDev->phyDevProp.deviceType    = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    __vki_InitializePhysicalDeviceName(phyDev);

    /* VSI Vulkan pipeline cache UUID: 66ca5e8c-7d09-47bb-b5d1-0ac5c5b13c85, DeviceID in [0..3] bytes */
    phyDev->phyDevProp.pipelineCacheUUID[0] = (uint8_t)((phyDev->phyDevProp.deviceID & 0xff));
    phyDev->phyDevProp.pipelineCacheUUID[1] = (uint8_t)((phyDev->phyDevProp.deviceID & 0xff00) >> 8);
    phyDev->phyDevProp.pipelineCacheUUID[2] = (uint8_t)((phyDev->phyDevProp.deviceID & 0xff0000) >> 16);
    phyDev->phyDevProp.pipelineCacheUUID[3] = (uint8_t)((phyDev->phyDevProp.deviceID & 0xff000000) >> 24);
    phyDev->phyDevProp.pipelineCacheUUID[4] = 0x7d;
    phyDev->phyDevProp.pipelineCacheUUID[5] = 0x09;
    phyDev->phyDevProp.pipelineCacheUUID[6] = 0x47;
    phyDev->phyDevProp.pipelineCacheUUID[7] = 0xbb;
    phyDev->phyDevProp.pipelineCacheUUID[8] = 0xb5;
    phyDev->phyDevProp.pipelineCacheUUID[9] = 0xd1;
    phyDev->phyDevProp.pipelineCacheUUID[10] = 0x0a;
    phyDev->phyDevProp.pipelineCacheUUID[11] = 0xc5;
    phyDev->phyDevProp.pipelineCacheUUID[12] = 0xc5;
    phyDev->phyDevProp.pipelineCacheUUID[13] = 0xb1;
    phyDev->phyDevProp.pipelineCacheUUID[14] = 0x3c;
    phyDev->phyDevProp.pipelineCacheUUID[15] = 0x85;

    phyDev->phyDevProp.sparseProperties.residencyAlignedMipSize                  = VK_FALSE;
    phyDev->phyDevProp.sparseProperties.residencyNonResidentStrict               = VK_FALSE;
    phyDev->phyDevProp.sparseProperties.residencyStandard2DBlockShape            = VK_FALSE;
    phyDev->phyDevProp.sparseProperties.residencyStandard2DMultisampleBlockShape = VK_FALSE;
    phyDev->phyDevProp.sparseProperties.residencyStandard3DBlockShape            = VK_FALSE;

    maxAniso = (database->REG_Halti0 && (!database->NO_ANISTRO_FILTER)) ? 16: 1;
    maxVaryings = database->VaryingCount;
    maxAttributes = database->PIPELINE_32_ATTRIBUTES ? 32 : 16;
    numberOfStreams = database->Streams;
    maxAttribOffset = database->NEW_GPIPE ? 2047 : 255;
    maxStreamStride = database->REG_Halti4 ? 2048 : 256;
    maxSamples = database->REG_MSAA ? 4 : 1;

    if (phyDev->phyDevFeatures.tessellationShader)
    {
        shaderStages += 2;
    }
    if (phyDev->phyDevFeatures.geometryShader)
    {
        shaderStages++;
    }

    if (database->REG_Texture8K)
        maxTexDim = 8192;
    else
        maxTexDim = 2048;

    phyDev->phyDevProp.limits.maxImageDimension1D                             = maxTexDim;
    phyDev->phyDevProp.limits.maxImageDimension2D                             = maxTexDim;
    phyDev->phyDevProp.limits.maxImageDimension3D                             = __VK_MAX_IMAGE_LAYERS;
    phyDev->phyDevProp.limits.maxImageDimensionCube                           = maxTexDim;
    phyDev->phyDevProp.limits.maxImageArrayLayers                             = __VK_MAX_IMAGE_LAYERS;
    phyDev->phyDevProp.limits.maxTexelBufferElements                          = 65536;
    phyDev->phyDevProp.limits.maxUniformBufferRange                           = 16384;
    phyDev->phyDevProp.limits.maxStorageBufferRange                           = 1 << 27;
    phyDev->phyDevProp.limits.maxPushConstantsSize                            = __VK_MAX_PUSHCONST_SIZE_IN_UINT * sizeof(uint32_t);
    phyDev->phyDevProp.limits.maxMemoryAllocationCount                        = 4096;
    phyDev->phyDevProp.limits.maxSamplerAllocationCount                       = 4000;
    phyDev->phyDevProp.limits.bufferImageGranularity                          = 4096;
    phyDev->phyDevProp.limits.sparseAddressSpaceSize                          = 0;
    phyDev->phyDevProp.limits.maxBoundDescriptorSets                          = __VK_MAX_DESCRIPTOR_SETS;
    phyDev->phyDevProp.limits.maxPerStageDescriptorSamplers                   = 16;
    phyDev->phyDevProp.limits.maxPerStageDescriptorUniformBuffers             = 12;
    phyDev->phyDevProp.limits.maxPerStageDescriptorStorageBuffers             = 4;
    phyDev->phyDevProp.limits.maxPerStageDescriptorSampledImages              = 16;
    phyDev->phyDevProp.limits.maxPerStageDescriptorStorageImages              = 4;
    phyDev->phyDevProp.limits.maxPerStageDescriptorInputAttachments           = 4;
    phyDev->phyDevProp.limits.maxPerStageResources                            = 128;
    phyDev->phyDevProp.limits.maxDescriptorSetSamplers                        =
        shaderStages * phyDev->phyDevProp.limits.maxPerStageDescriptorSamplers;
    phyDev->phyDevProp.limits.maxDescriptorSetUniformBuffers                  =
        shaderStages * phyDev->phyDevProp.limits.maxPerStageDescriptorUniformBuffers;
    phyDev->phyDevProp.limits.maxDescriptorSetUniformBuffersDynamic           = __VK_MAX_UNIFORM_BUFFER_DYNAMIC;
    phyDev->phyDevProp.limits.maxDescriptorSetStorageBuffers                  =
        shaderStages * phyDev->phyDevProp.limits.maxPerStageDescriptorStorageImages;
    phyDev->phyDevProp.limits.maxDescriptorSetStorageBuffersDynamic           = __VK_MAX_STORAGE_BUFFER_DYNAMIC;
    phyDev->phyDevProp.limits.maxDescriptorSetSampledImages                   =
        shaderStages * phyDev->phyDevProp.limits.maxPerStageDescriptorSampledImages;
    phyDev->phyDevProp.limits.maxDescriptorSetStorageImages                   =
        shaderStages * phyDev->phyDevProp.limits.maxPerStageDescriptorStorageImages;
    phyDev->phyDevProp.limits.maxDescriptorSetInputAttachments                = phyDev->phyDevProp.limits.maxPerStageDescriptorInputAttachments;
    /* VS */
    phyDev->phyDevProp.limits.maxVertexInputAttributes                        = maxAttributes;
    phyDev->phyDevProp.limits.maxVertexInputBindings                          = numberOfStreams;
    phyDev->phyDevProp.limits.maxVertexInputAttributeOffset                   = maxAttribOffset;
    phyDev->phyDevProp.limits.maxVertexInputBindingStride                     = maxStreamStride;
    phyDev->phyDevProp.limits.maxVertexOutputComponents                       = (maxVaryings + 1) * 4;

    /* TS */
    if (phyDev->phyDevFeatures.tessellationShader)
    {
        phyDev->phyDevProp.limits.maxTessellationGenerationLevel                  = 64;
        phyDev->phyDevProp.limits.maxTessellationPatchSize                        = 32;
        phyDev->phyDevProp.limits.maxTessellationControlPerVertexInputComponents  = 128;
        phyDev->phyDevProp.limits.maxTessellationControlPerVertexOutputComponents = 128;
        phyDev->phyDevProp.limits.maxTessellationControlPerPatchOutputComponents  = 120;
        phyDev->phyDevProp.limits.maxTessellationControlTotalOutputComponents     = 2048;
        phyDev->phyDevProp.limits.maxTessellationEvaluationInputComponents        = 128;
        phyDev->phyDevProp.limits.maxTessellationEvaluationOutputComponents       = 128;
    }

    /* GS */
    if (phyDev->phyDevFeatures.geometryShader)
    {
        phyDev->phyDevProp.limits.maxGeometryShaderInvocations                    = 32;
        phyDev->phyDevProp.limits.maxGeometryInputComponents                      = 64;
        phyDev->phyDevProp.limits.maxGeometryOutputComponents                     = 128;
        phyDev->phyDevProp.limits.maxGeometryOutputVertices                       = 256;
        phyDev->phyDevProp.limits.maxGeometryTotalOutputComponents                = 1024;
    }

    /* FS */
    phyDev->phyDevProp.limits.maxFragmentInputComponents                      = (maxVaryings + 1) * 4;
    phyDev->phyDevProp.limits.maxFragmentOutputAttachments                    = __VK_MAX_RENDER_TARGETS;
    phyDev->phyDevProp.limits.maxFragmentDualSrcAttachments                   = 0;
    phyDev->phyDevProp.limits.maxFragmentCombinedOutputResources              = __VK_MAX_RENDER_TARGETS;

    /* CS */
    phyDev->phyDevProp.limits.maxComputeSharedMemorySize                      = 32768;
    phyDev->phyDevProp.limits.maxComputeWorkGroupCount[0]                     = 65535;
    phyDev->phyDevProp.limits.maxComputeWorkGroupCount[1]                     = 65535;
    phyDev->phyDevProp.limits.maxComputeWorkGroupCount[2]                     = 65535;
    /*modify the max WorkGroupInvocation from 128 to 2048 when do barrier in cs, the max value should be 128*/
    phyDev->phyDevProp.limits.maxComputeWorkGroupInvocations                  = 2048;
    phyDev->phyDevProp.limits.maxComputeWorkGroupSize[0]                      = 128;
    phyDev->phyDevProp.limits.maxComputeWorkGroupSize[1]                      = 128;
    phyDev->phyDevProp.limits.maxComputeWorkGroupSize[2]                      = 64;

    phyDev->phyDevProp.limits.subPixelPrecisionBits                           = 4;
    phyDev->phyDevProp.limits.subTexelPrecisionBits                           =
        database->TX_8bit_UVFrac ? 8 : (database->REG_TX6bitFrac ? 6 : 5);
    phyDev->phyDevProp.limits.mipmapPrecisionBits                             =
        database->TX_8bit_UVFrac ? 8 : (database->REG_TX6bitFrac ? 6 : 5);
    phyDev->phyDevProp.limits.maxDrawIndexedIndexValue                        =
        phyDev->phyDevFeatures.fullDrawIndexUint32 ? 0xFFFFFFFF : ((1 << 24) - 1);

    phyDev->phyDevProp.limits.maxDrawIndirectCount                            = 1;
    phyDev->phyDevProp.limits.maxSamplerLodBias                               = 2.f;
    phyDev->phyDevProp.limits.maxSamplerAnisotropy                            = (float)maxAniso;

    switch (maxSamples)
    {
    case VK_SAMPLE_COUNT_64_BIT:
        sampleMask |= VK_SAMPLE_COUNT_64_BIT;
    case VK_SAMPLE_COUNT_32_BIT:
        sampleMask |= VK_SAMPLE_COUNT_32_BIT;
    case VK_SAMPLE_COUNT_16_BIT:
        sampleMask |= VK_SAMPLE_COUNT_16_BIT;
    case VK_SAMPLE_COUNT_8_BIT:
        sampleMask |= VK_SAMPLE_COUNT_8_BIT;
    case VK_SAMPLE_COUNT_4_BIT:
        sampleMask |= VK_SAMPLE_COUNT_4_BIT;
    case VK_SAMPLE_COUNT_1_BIT:
        sampleMask |= VK_SAMPLE_COUNT_1_BIT;
        break;

    default:
        sampleMask |= VK_SAMPLE_COUNT_1_BIT;
    }

    if (database->REG_Halti3)
        maxRtDim = 8192;
    else if (database->REG_Render8K)
        maxRtDim = (8192 - 128);
    else
        maxRtDim = 2048;

    /* Viewport */
    phyDev->phyDevProp.limits.maxViewports                                    = 1;
    phyDev->phyDevProp.limits.maxViewportDimensions[0]                        = maxRtDim;
    phyDev->phyDevProp.limits.maxViewportDimensions[1]                        = maxRtDim;
    phyDev->phyDevProp.limits.viewportBoundsRange[0]                          = -(float)(maxRtDim * 2);
    phyDev->phyDevProp.limits.viewportBoundsRange[1]                          = (float)(maxRtDim * 2);
    phyDev->phyDevProp.limits.viewportSubPixelBits                            = 2;
    phyDev->phyDevProp.limits.minMemoryMapAlignment                           = 64;
    phyDev->phyDevProp.limits.minTexelBufferOffsetAlignment                   = 256;
    phyDev->phyDevProp.limits.minUniformBufferOffsetAlignment                 = 256;
    phyDev->phyDevProp.limits.minStorageBufferOffsetAlignment                 = 256;
    phyDev->phyDevProp.limits.minTexelOffset                                  = -8;
    phyDev->phyDevProp.limits.maxTexelOffset                                  = 7;
    phyDev->phyDevProp.limits.minTexelGatherOffset                            = -8;
    phyDev->phyDevProp.limits.maxTexelGatherOffset                            = 7;
    phyDev->phyDevProp.limits.minInterpolationOffset                          = -0.5f;
    phyDev->phyDevProp.limits.maxInterpolationOffset                          = 0.5f;
    phyDev->phyDevProp.limits.subPixelInterpolationOffsetBits                 = 4;
    phyDev->phyDevProp.limits.maxFramebufferWidth                             = maxRtDim;
    phyDev->phyDevProp.limits.maxFramebufferHeight                            = maxRtDim;
    phyDev->phyDevProp.limits.maxFramebufferLayers                            = __VK_MAX_IMAGE_LAYERS;
    phyDev->phyDevProp.limits.framebufferColorSampleCounts                    = sampleMask;
    phyDev->phyDevProp.limits.framebufferDepthSampleCounts                    = sampleMask;
    phyDev->phyDevProp.limits.framebufferStencilSampleCounts                  = sampleMask;
    phyDev->phyDevProp.limits.framebufferNoAttachmentsSampleCounts            = sampleMask;
    phyDev->phyDevProp.limits.maxColorAttachments                             = __VK_MAX_RENDER_TARGETS;
    phyDev->phyDevProp.limits.sampledImageColorSampleCounts                   = sampleMask;
    phyDev->phyDevProp.limits.sampledImageIntegerSampleCounts                 = VK_SAMPLE_COUNT_1_BIT;
    phyDev->phyDevProp.limits.sampledImageDepthSampleCounts                   = sampleMask;
    phyDev->phyDevProp.limits.sampledImageStencilSampleCounts                 = sampleMask;
    phyDev->phyDevProp.limits.storageImageSampleCounts                        = phyDev->phyDevFeatures.shaderStorageImageMultisample
                                                                              ? sampleMask
                                                                              : VK_SAMPLE_COUNT_1_BIT;
    phyDev->phyDevProp.limits.maxSampleMaskWords                              = 1;
    phyDev->phyDevProp.limits.timestampPeriod                                 = 0.0f;
    phyDev->phyDevProp.limits.maxClipDistances                                = 0;
    phyDev->phyDevProp.limits.maxCullDistances                                = 0;
    phyDev->phyDevProp.limits.maxCombinedClipAndCullDistances                 = 0;
    phyDev->phyDevProp.limits.discreteQueuePriorities                         = 1;
    phyDev->phyDevProp.limits.pointSizeGranularity                            = 0.125f;
    phyDev->phyDevProp.limits.pointSizeRange[0]                               = 1.0f;
    phyDev->phyDevProp.limits.pointSizeRange[1]                               = 64.0f;
    phyDev->phyDevProp.limits.lineWidthRange[0]                               = 1.0f;
    phyDev->phyDevProp.limits.lineWidthRange[1]                               = phyDev->phyDevConfig.database->REG_WideLine? 8.0f : 1.0f;
    phyDev->phyDevProp.limits.lineWidthGranularity                            = 0.125;
    phyDev->phyDevProp.limits.strictLines                                     = VK_FALSE;
    phyDev->phyDevProp.limits.standardSampleLocations                         = VK_FALSE;
    phyDev->phyDevProp.limits.optimalBufferCopyOffsetAlignment                = 256;
    phyDev->phyDevProp.limits.optimalBufferCopyRowPitchAlignment              = 256;
    phyDev->phyDevProp.limits.nonCoherentAtomSize                             = 128;

    __vki_InitializeShaderCaps(phyDev);
}

static VkResult __vki_InitializePhysicalDevice(
    VkPhysicalDevice physicalDevice
    )
{
    __vkPhysicalDevice *    phyDev = (__vkPhysicalDevice *)physicalDevice;
    gceSTATUS               status;

#if !__VK_NEW_DEVICE_QUEUE
    status = gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    status = gcoHAL_GetHardware(gcvNULL, &phyDev->hardware);
    if (gcmIS_ERROR(status))
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
#endif

    status = gcoOS_CreateMutex(gcvNULL, &phyDev->mutex);
    if (gcmIS_ERROR(status))
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    set_loader_magic_value(phyDev);
    phyDev->sType = __VK_OBJECT_TYPE_PHYSICAL_DEVICE;

    /* Initialize VkPhysicalDeviceFeatures here */
    __vki_InitializePhysicalDeviceFeatures(physicalDevice);

    /* Initialize VkPhysicalDeviceProperties here */
    __vki_InitializePhysicalDevicePorperties(physicalDevice);

    /* Modify formatInfo table based on queried caps */
    if (!phyDev->phyDevFeatures.textureCompressionETC2)
    {
        uint32_t fmt;

        for (fmt = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK; fmt < VK_FORMAT_EAC_R11G11_SNORM_BLOCK; ++fmt)
        {
            __VK_MEMZERO(&g_vkFormatInfoTable[fmt].formatProperties, sizeof(VkFormatProperties));
        }
    }

    if (!phyDev->phyDevFeatures.textureCompressionASTC_LDR)
    {
        uint32_t fmt;

        for (fmt = VK_FORMAT_ASTC_4x4_UNORM_BLOCK; fmt < VK_FORMAT_ASTC_12x12_SRGB_BLOCK; ++fmt)
        {
            __VK_MEMZERO(&g_vkFormatInfoTable[fmt].formatProperties, sizeof(VkFormatProperties));
        }
    }

    if (!phyDev->phyDevConfig.database->REG_Halti5 && phyDev->phyDevConfig.database->REG_Halti3)
    {
       g_vkFormatInfoTable[VK_FORMAT_R8_UNORM].residentImgFormat = __VK_FORMAT_R8_1_X8R8G8B8;
    }

    /* Initialize VkPhysicalDeviceMemoryProperties here */
    phyDev->phyDevMemProp.memoryHeapCount = 1;
    phyDev->phyDevMemProp.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    phyDev->phyDevMemProp.memoryHeaps[0].size = 0x30000000;
    phyDev->phyDevMemProp.memoryTypeCount = 1;
    phyDev->phyDevMemProp.memoryTypes[0].heapIndex = 0;
    phyDev->phyDevMemProp.memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    /* Initialize VkPhysicalDeviceQueueProperties here */

    /* Only support general queue at this moment */
    phyDev->queueFamilyCount = __VK_PDEV_QUEUEFAMILY_GENERAL + 1;

    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].queueCount = 1;
    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT;
    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].timestampValidBits = 0;
    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].minImageTransferGranularity.width  = 1;
    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].minImageTransferGranularity.height = 1;
    phyDev->queueProp[__VK_PDEV_QUEUEFAMILY_GENERAL].minImageTransferGranularity.depth  = 1;
    phyDev->queuePresentSupported[__VK_PDEV_QUEUEFAMILY_GENERAL] = VK_TRUE;

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumeratePhysicalDevices(
    VkInstance instance,
    uint32_t* pPhysicalDeviceCount,
    VkPhysicalDevice* pPhysicalDevices
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    /* Set physicalDeviceCount to 1 for the time being */
    inst->physicalDeviceCount = 1;

    if (pPhysicalDevices)
    {
        uint32_t    i;
        uint32_t    devCount = gcmMIN(inst->physicalDeviceCount, *pPhysicalDeviceCount);

        for (i = 0; i < __VK_MAX_PDEV_COUNT; i++)
        {
            /* Initialize physical devices in not yet initialized. */
            if (inst->physicalDevice[i].mutex == gcvNULL)
            {
                __vkPhysicalDevice *    phyDev;

                phyDev = &inst->physicalDevice[i];
                phyDev->pInst = inst;

                __VK_ONERROR(__vki_InitializePhysicalDevice((VkPhysicalDevice)phyDev));
            }
        }

        for (i = 0; i < devCount; i++)
        {
            *pPhysicalDevices = (VkPhysicalDevice)&inst->physicalDevice[i];
        }

        *pPhysicalDeviceCount = devCount;

        /* If pPhysicalDeviceCount is smaller than the number of physical devices available,
        ** VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the available physical devices were returned.
        */
        if (*pPhysicalDeviceCount < inst->physicalDeviceCount)
        {
            __VK_ONERROR(VK_INCOMPLETE);
        }
    }
    else
    {
        *pPhysicalDeviceCount = inst->physicalDeviceCount;
    }

    return VK_SUCCESS;

OnError:
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateDeviceLayerProperties(
    VkPhysicalDevice physicalDevice,
    uint32_t* pCount,
    VkLayerProperties* pProperties
    )
{
    if (!pProperties)
    {
        *pCount = 0;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateDeviceExtensionProperties(
    VkPhysicalDevice physicalDevice,
    const char* pLayerName,
    uint32_t* pCount,
    VkExtensionProperties* pProperties
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t devExtCount = __VK_COUNTOF(g_DeviceExtensions);

    if (!pProperties)
    {
        *pCount = devExtCount;
    }
    else
    {
        uint32_t i;
        uint32_t extCount = gcmMIN(*pCount, devExtCount);

        for (i = 0; i < extCount; i++)
        {
            __VK_MEMCOPY(&pProperties[i], &g_DeviceExtensions[i], sizeof(VkExtensionProperties));
        }

        /* If pPropertyCount is smaller than the number of extensions available, VK_INCOMPLETE will be returned instead of VK_SUCCESS,
        ** to indicate that not all the available properties were returned.
        */
        if (extCount < devExtCount)
        {
            result = VK_INCOMPLETE;
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceFeatures(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceFeatures* pFeatures
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    *pFeatures = phyDev->phyDevFeatures;

}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceFormatProperties(
    VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkFormatProperties* pFormatProperties
    )
{
    *pFormatProperties = g_vkFormatInfoTable[format].formatProperties;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkImageCreateFlags flags,
    VkImageFormatProperties* pImageFormatProperties
    )
{
    VkResult ret = VK_SUCCESS;
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t maxDim = phyDev->phyDevConfig.database->REG_Texture8K ? 8192 : 2048;
    __vkFormatInfo *formatInfo = &g_vkFormatInfoTable[format];
    VkFormatFeatureFlags formatFeatures = (tiling == VK_IMAGE_TILING_LINEAR)
                                        ? formatInfo->formatProperties.linearTilingFeatures
                                        : formatInfo->formatProperties.optimalTilingFeatures;

    if (formatFeatures == 0)
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    if (ret != VK_SUCCESS)
    {
        /* For unsupported combinations, then all members will be filled with zero*/
        __VK_MEMZERO(pImageFormatProperties, sizeof(VkImageFormatProperties));
    }
    else
    {
        VkSampleCountFlags sampleCounts = VK_SAMPLE_COUNT_1_BIT;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && type == VK_IMAGE_TYPE_2D &&
            !(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
            (formatFeatures & (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)))
        {
            VkPhysicalDeviceLimits *pLimits = &phyDev->phyDevProp.limits;

            sampleCounts = ~(VkSampleCountFlags)0;

            if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            {
                sampleCounts &= pLimits->storageImageSampleCounts;
            }
            if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            {
                sampleCounts &= pLimits->framebufferColorSampleCounts;
            }
            if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                switch (format)
                {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D32_SFLOAT:
                    sampleCounts &= pLimits->framebufferDepthSampleCounts;
                    break;
                case VK_FORMAT_S8_UINT:
                    sampleCounts &= pLimits->framebufferStencilSampleCounts;
                    break;
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    sampleCounts &= (pLimits->framebufferDepthSampleCounts & pLimits->framebufferStencilSampleCounts);
                    break;
                default:
                    __VK_ASSERT(!"Must have depth or stencil aspect");
                    break;
                }
            }
            if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            {
                switch (format)
                {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D32_SFLOAT:
                    sampleCounts &= pLimits->sampledImageDepthSampleCounts;
                    break;
                case VK_FORMAT_S8_UINT:
                    sampleCounts &= pLimits->sampledImageStencilSampleCounts;
                    break;
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    sampleCounts &= (pLimits->sampledImageDepthSampleCounts & pLimits->sampledImageStencilSampleCounts);
                    break;
                default:
                    if (formatInfo->category == __VK_FMT_CATEGORY_UINT || formatInfo->category == __VK_FMT_CATEGORY_SINT)
                    {
                        sampleCounts &= pLimits->sampledImageIntegerSampleCounts;
                    }
                    else
                    {
                        sampleCounts &= pLimits->sampledImageColorSampleCounts;
                    }
                    break;
                }
            }

            if (sampleCounts == ~(VkSampleCountFlags)0)
            {
                sampleCounts &= VK_SAMPLE_COUNT_1_BIT;
            }
        }

        pImageFormatProperties->maxExtent.width       = maxDim;
        pImageFormatProperties->maxExtent.height      = 1;
        pImageFormatProperties->maxExtent.depth       = 1;
        if (type > VK_IMAGE_TYPE_1D)
            pImageFormatProperties->maxExtent.height = maxDim;
        if (type == VK_IMAGE_TYPE_3D)
            pImageFormatProperties->maxExtent.depth  = maxDim;
        pImageFormatProperties->maxArrayLayers       = (type == VK_IMAGE_TYPE_3D) ? 1 : maxDim;
        pImageFormatProperties->maxMipLevels         = __vkFloorLog2(maxDim) + 1;
        pImageFormatProperties->sampleCounts         = sampleCounts;
        pImageFormatProperties->maxResourceSize      = 1u << 31;
    }

    return ret;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceProperties(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceProperties* pProperties
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    *pProperties = phyDev->phyDevProp;

}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties* pQueueFamilyProperties
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t i, count;

    if (!pQueueFamilyProperties)
    {
        /* Just return the number of queue families available */
        *pQueueFamilyPropertyCount = phyDev->queueFamilyCount;
    }
    else
    {
        count = gcmMIN(*pQueueFamilyPropertyCount, phyDev->queueFamilyCount);
        for (i = 0; i < count; i++)
        {
            pQueueFamilyProperties[i] = phyDev->queueProp[i];
        }
        /* Return the actual queue family count */
        *pQueueFamilyPropertyCount = count;
    }

}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDeviceMemoryProperties* pMemoryProperties
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkInstance *inst = phyDev->pInst;
    uint32_t i;

    for (i = 0; i < inst->physicalDeviceCount; i++)
    {
        pMemoryProperties[i] = inst->physicalDevice[i].phyDevMemProp;
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice,
    VkFormat format,
    VkImageType type,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usage,
    VkImageTiling tiling,
    uint32_t* pPropertyCount,
    VkSparseImageFormatProperties* pProperties
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    if (!pProperties)
    {
        *pPropertyCount = phyDev->phyDevFeatures.sparseBinding;
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2KHR*               pFeatures)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    pFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    pFeatures->pNext = VK_NULL_HANDLE;
    pFeatures->features = phyDev->phyDevFeatures;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2KHR*             pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    pProperties->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    pProperties->pNext = VK_NULL_HANDLE;
    pProperties->properties = phyDev->phyDevProp;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2KHR*                     pFormatProperties)
{
    pFormatProperties->sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR;
    pFormatProperties->pNext = VK_NULL_HANDLE;
    pFormatProperties->formatProperties = g_vkFormatInfoTable[format].formatProperties;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2KHR*  pImageFormatInfo,
    VkImageFormatProperties2KHR*                pImageFormatProperties)
{
    VkResult ret = VK_SUCCESS;
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t maxDim = phyDev->phyDevConfig.database->REG_Texture8K ? 8192 : 2048;
    VkFormat format = pImageFormatInfo->format;
    VkImageTiling tiling = pImageFormatInfo->tiling;
    VkImageUsageFlags usage = pImageFormatInfo->usage;
    VkImageType type = pImageFormatInfo->type;
    VkImageCreateFlags flags = pImageFormatInfo->flags;
    __vkFormatInfo *formatInfo = &g_vkFormatInfoTable[format];
    VkFormatFeatureFlags formatFeatures = (tiling == VK_IMAGE_TILING_LINEAR)
                                        ? formatInfo->formatProperties.linearTilingFeatures
                                        : formatInfo->formatProperties.optimalTilingFeatures;

    if (formatFeatures == 0)
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    else if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(formatFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        ret = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    if (ret != VK_SUCCESS)
    {
        /* For unsupported combinations, then all members will be filled with zero*/
        __VK_MEMZERO(&pImageFormatProperties->imageFormatProperties, sizeof(VkImageFormatProperties));
    }
    else
    {
        VkSampleCountFlags sampleCounts = VK_SAMPLE_COUNT_1_BIT;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && type == VK_IMAGE_TYPE_2D &&
            !(flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) &&
            (formatFeatures & (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)))
        {
            VkPhysicalDeviceLimits *pLimits = &phyDev->phyDevProp.limits;

            sampleCounts = ~(VkSampleCountFlags)0;

            if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            {
                sampleCounts &= pLimits->storageImageSampleCounts;
            }
            if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            {
                sampleCounts &= pLimits->framebufferColorSampleCounts;
            }
            if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                switch (format)
                {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D32_SFLOAT:
                    sampleCounts &= pLimits->framebufferDepthSampleCounts;
                    break;
                case VK_FORMAT_S8_UINT:
                    sampleCounts &= pLimits->framebufferStencilSampleCounts;
                    break;
                    case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    sampleCounts &= (pLimits->framebufferDepthSampleCounts & pLimits->framebufferStencilSampleCounts);
                    break;
                default:
                    __VK_ASSERT(!"Must have depth or stencil aspect");
                    break;
                }
            }
            if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            {
                switch (format)
                {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D32_SFLOAT:
                    sampleCounts &= pLimits->sampledImageDepthSampleCounts;
                    break;
                case VK_FORMAT_S8_UINT:
                    sampleCounts &= pLimits->sampledImageStencilSampleCounts;
                    break;
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    sampleCounts &= (pLimits->sampledImageDepthSampleCounts & pLimits->sampledImageStencilSampleCounts);
                    break;
                default:
                    if (formatInfo->category == __VK_FMT_CATEGORY_UINT || formatInfo->category == __VK_FMT_CATEGORY_SINT)
                    {
                        sampleCounts &= pLimits->sampledImageIntegerSampleCounts;
                    }
                    else
                    {
                        sampleCounts &= pLimits->sampledImageColorSampleCounts;
                    }
                    break;
                }
            }

            if (sampleCounts == ~(VkSampleCountFlags)0)
            {
                sampleCounts &= VK_SAMPLE_COUNT_1_BIT;
            }
        }

        pImageFormatProperties->imageFormatProperties.maxExtent.width       = maxDim;
        pImageFormatProperties->imageFormatProperties.maxExtent.height      = 1;
        pImageFormatProperties->imageFormatProperties.maxExtent.depth       = 1;
        if (type > VK_IMAGE_TYPE_1D)
            pImageFormatProperties->imageFormatProperties.maxExtent.height = maxDim;
        if (type == VK_IMAGE_TYPE_3D)
            pImageFormatProperties->imageFormatProperties.maxExtent.depth  = maxDim;
        pImageFormatProperties->imageFormatProperties.maxArrayLayers       = (type == VK_IMAGE_TYPE_3D) ? 1 : maxDim;
        pImageFormatProperties->imageFormatProperties.maxMipLevels         = __vkFloorLog2(maxDim) + 1;
        pImageFormatProperties->imageFormatProperties.sampleCounts         = sampleCounts;
        pImageFormatProperties->imageFormatProperties.maxResourceSize      = 1u << 31;
    }

    return ret;
}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR*                pQueueFamilyProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    uint32_t i, count;

    if (!pQueueFamilyProperties)
    {
        /* Just return the number of queue families available */
        *pQueueFamilyPropertyCount = phyDev->queueFamilyCount;
    }
    else
    {
        count = gcmMIN(*pQueueFamilyPropertyCount, phyDev->queueFamilyCount);
        for (i = 0; i < count; i++)
        {
            pQueueFamilyProperties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR;
            pQueueFamilyProperties[i].pNext = VK_NULL_HANDLE;
            pQueueFamilyProperties[i].queueFamilyProperties = phyDev->queueProp[i];
        }
        /* Return the actual queue family count */
        *pQueueFamilyPropertyCount = count;
    }

}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2KHR*       pMemoryProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkInstance *inst = phyDev->pInst;
    uint32_t i;

    pMemoryProperties->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
    pMemoryProperties->pNext = VK_NULL_HANDLE;

    for (i = 0; i < inst->physicalDeviceCount; i++)
    {
        pMemoryProperties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR;
        pMemoryProperties[i].pNext = VK_NULL_HANDLE;
        pMemoryProperties[i].memoryProperties = inst->physicalDevice[i].phyDevMemProp;
    }

}

VKAPI_ATTR void VKAPI_CALL __vk_GetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2KHR* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2KHR*          pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    if (!pProperties)
    {
        *pPropertyCount = phyDev->phyDevFeatures.sparseBinding;
    }
}



