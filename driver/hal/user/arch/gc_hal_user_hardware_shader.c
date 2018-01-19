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


/*
**  Shader programming..
*/

#include "gc_hal_user_hardware_precomp.h"

#if gcdENABLE_3D

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

#define IS_HW_SUPPORT(feature) (Hardware->features[feature])

gceSTATUS
gcoHARDWARE_QueryShaderCompilerHwCfg(
    IN gcoHARDWARE Hardware,
    OUT PVSC_HW_CONFIG pVscHwCfg
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 maxVaryingCount, maxAttribs;
    gctUINT32 samplerCount[gcvPROGRAM_STAGE_LAST] = {0};
    gctUINT32 samplerBase[gcvPROGRAM_STAGE_LAST] = {0};
    gctUINT32 totalCount = 0;
    gctUINT32 fragmentSizeInKbyte = 0;
    gctUINT32 attribBufSizeInKbyte = 0;
    gctUINT32 threadCount = 0;

    gcmHEADER_ARG("Hardware=0x%x pVscHwCfg=%d", Hardware, pVscHwCfg);

    gcmGETHARDWARE(Hardware );

    gcoOS_ZeroMemory(pVscHwCfg, sizeof(VSC_HW_CONFIG));

    gcmONERROR(gcoHARDWARE_QueryShaderCaps(gcvNULL,
                                           gcvNULL,
                                           gcvNULL,
                                           gcvNULL,
                                           &maxVaryingCount,
                                           gcvNULL,
                                           &threadCount,
                                           gcvNULL,
                                           gcvNULL));

    gcmONERROR(gcoHARDWARE_QuerySamplerBase(gcvNULL,
                                            samplerCount,
                                            samplerBase,
                                            &totalCount));

    gcmONERROR(gcoHARDWARE_QueryStreamCaps(gcvNULL,
                                           &maxAttribs,
                                           gcvNULL,
                                           gcvNULL,
                                           gcvNULL,
                                           gcvNULL));

    pVscHwCfg->chipModel                             = Hardware->config->chipModel;
    pVscHwCfg->chipRevision                          = Hardware->config->chipRevision;
    pVscHwCfg->maxCoreCount                          = Hardware->config->shaderCoreCount;
    pVscHwCfg->maxThreadCountPerCore                 = Hardware->config->threadCount/Hardware->config->shaderCoreCount;
    pVscHwCfg->maxVaryingCount                       = maxVaryingCount;
    pVscHwCfg->maxAttributeCount                     = maxAttribs;
    pVscHwCfg->maxRenderTargetCount                  = Hardware->config->renderTargets;
    pVscHwCfg->maxGPRCountPerCore                    = 512;
    pVscHwCfg->maxGPRCountPerThread                  = Hardware->config->registerMax;
    pVscHwCfg->maxHwNativeTotalInstCount             = Hardware->config->instructionCount;
    pVscHwCfg->maxTotalInstCount                     = Hardware->config->instMax;
    pVscHwCfg->maxVSInstCount                        = Hardware->config->vsInstMax;
    pVscHwCfg->maxPSInstCount                        = Hardware->config->psInstMax;
    pVscHwCfg->vsInstBufferAddrBase                  = Hardware->config->vsInstBase;
    pVscHwCfg->psInstBufferAddrBase                  = Hardware->config->psInstBase;
    pVscHwCfg->maxHwNativeTotalConstRegCount         = Hardware->config->numConstants;
    pVscHwCfg->maxTotalConstRegCount                 = Hardware->config->constMax;
    pVscHwCfg->unifiedConst                          = Hardware->config->unifiedConst;
    pVscHwCfg->maxVSConstRegCount                    = Hardware->config->vsConstMax;
    /* use VS count for now */
    pVscHwCfg->maxTCSConstRegCount                   = Hardware->config->vsConstMax;
    pVscHwCfg->maxTESConstRegCount                   = Hardware->config->vsConstMax;
    pVscHwCfg->maxGSConstRegCount                    = Hardware->config->vsConstMax;
    pVscHwCfg->maxPSConstRegCount                    = Hardware->config->psConstMax;
    pVscHwCfg->vsConstRegAddrBase                    = Hardware->config->vsConstBase;
    /* use VS count for now */
    pVscHwCfg->tcsConstRegAddrBase                   = Hardware->config->vsConstBase;
    pVscHwCfg->tesConstRegAddrBase                   = Hardware->config->vsConstBase;
    pVscHwCfg->gsConstRegAddrBase                    = Hardware->config->vsConstBase;
    pVscHwCfg->psConstRegAddrBase                    = Hardware->config->psConstBase;
    /*
    ** Set sample base and count. Here is the sampler order:
    ** PS: 0~15
    ** CS/CL: 0~31
    ** VS: 16~31
    ** TCS: 32~47
    ** TES: 48~63
    ** GS: 64~79
    */
    pVscHwCfg->maxVSSamplerCount                     = samplerCount[gcvPROGRAM_STAGE_VERTEX];
    pVscHwCfg->maxTCSSamplerCount                    = samplerCount[gcvPROGRAM_STAGE_TCS];
    pVscHwCfg->maxTESSamplerCount                    = samplerCount[gcvPROGRAM_STAGE_TES];
    pVscHwCfg->maxGSSamplerCount                     = samplerCount[gcvPROGRAM_STAGE_GEOMETRY];
    pVscHwCfg->maxPSSamplerCount                     = samplerCount[gcvPROGRAM_STAGE_FRAGMENT];
    pVscHwCfg->maxCSSamplerCount                     = samplerCount[gcvPROGRAM_STAGE_COMPUTE];
    /* Right now there are 5 bits for sampler index, so the max sampler count is 32. */
    pVscHwCfg->maxSamplerCountPerShader              = 32;

    pVscHwCfg->maxHwNativeTotalSamplerCount          = totalCount;

    /* samplerNoBase for state programming */
    if (IS_HW_SUPPORT(gcvFEATURE_SAMPLER_BASE_OFFSET))
    {
        pVscHwCfg->vsSamplerRegNoBase                = samplerBase[gcvPROGRAM_STAGE_VERTEX];
        pVscHwCfg->psSamplerRegNoBase                = samplerBase[gcvPROGRAM_STAGE_FRAGMENT];
        pVscHwCfg->csSamplerRegNoBase                = samplerBase[gcvPROGRAM_STAGE_COMPUTE];
        pVscHwCfg->tcsSamplerRegNoBase               = samplerBase[gcvPROGRAM_STAGE_TCS];
        pVscHwCfg->tesSamplerRegNoBase               = samplerBase[gcvPROGRAM_STAGE_TES];
        pVscHwCfg->gsSamplerRegNoBase                = samplerBase[gcvPROGRAM_STAGE_GEOMETRY];
        /* samplerNoBase in instruction words */
        pVscHwCfg->vsSamplerNoBaseInInstruction      = 0;
        pVscHwCfg->psSamplerNoBaseInInstruction      = samplerBase[gcvPROGRAM_STAGE_FRAGMENT];
    }
    else
    {
        pVscHwCfg->vsSamplerRegNoBase                = 0;
        pVscHwCfg->psSamplerRegNoBase                = 0;
        pVscHwCfg->csSamplerRegNoBase                = 0;
        pVscHwCfg->tcsSamplerRegNoBase               = 0;
        pVscHwCfg->tesSamplerRegNoBase               = 0;
        pVscHwCfg->gsSamplerRegNoBase                = 0;
        /* samplerNoBase in instruction words */
        pVscHwCfg->vsSamplerNoBaseInInstruction      = samplerBase[gcvPROGRAM_STAGE_VERTEX];
        pVscHwCfg->psSamplerNoBaseInInstruction      = samplerBase[gcvPROGRAM_STAGE_FRAGMENT];
    }
    pVscHwCfg->vertexOutputBufferSize                = Hardware->config->vertexOutputBufferSize;
    pVscHwCfg->vertexCacheSize                       = Hardware->config->vertexCacheSize;
    pVscHwCfg->ctxStateCount                         = Hardware->maxState;

    if ((!IS_HW_SUPPORT(gcvFEATURE_SNAPPAGE_CMD_FIX) ||
         !IS_HW_SUPPORT(gcvFEATURE_SH_SNAP2PAGE_MAXPAGES_FIX)) &&
        IS_HW_SUPPORT(gcvFEATURE_GEOMETRY_SHADER)              &&
        IS_HW_SUPPORT(gcvFEATURE_TESSELLATION))
    {
        fragmentSizeInKbyte = 5;
    }
    if (IS_HW_SUPPORT(gcvFEATURE_USC))
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

        attribBufSizeInKbyte = (gctUINT32)(Hardware->config->uscPagesMaxInKbyte
              - (Hardware->config->l1CacheSizeInKbyte * s_uscCacheRatio[Hardware->options.uscL1CacheRatio]));

        attribBufSizeInKbyte -= fragmentSizeInKbyte;
    }
    pVscHwCfg->maxUSCAttribBufInKbyte                = IS_HW_SUPPORT(gcvFEATURE_USC) ?  attribBufSizeInKbyte : 0;
    pVscHwCfg->maxLocalMemSizeInByte                 = attribBufSizeInKbyte * 1024;
    pVscHwCfg->maxResultCacheWinSize                 = Hardware->config->resultWindowMaxSize;

    /*
    ** We use a 128 as workgroupsize for computation.
    ** At runtime, if the real workgroupsize is smaller than 128,
    ** we need to adjust hwRegCount to make sure that the number of workgroup
    ** fit the local memory size requirement.
    */
    pVscHwCfg->defaultWorkGroupSize                  = 128;
    pVscHwCfg->maxWorkGroupSize                      = gcmMIN(threadCount, 1024);
    pVscHwCfg->minWorkGroupSize                      = 1;

    pVscHwCfg->hwFeatureFlags.hasHalti0              = IS_HW_SUPPORT(gcvFEATURE_HALTI0);
    pVscHwCfg->hwFeatureFlags.hasHalti1              = IS_HW_SUPPORT(gcvFEATURE_HALTI1);
    pVscHwCfg->hwFeatureFlags.hasHalti2              = IS_HW_SUPPORT(gcvFEATURE_HALTI2);
    pVscHwCfg->hwFeatureFlags.hasHalti3              = IS_HW_SUPPORT(gcvFEATURE_HALTI3);
    pVscHwCfg->hwFeatureFlags.hasHalti4              = IS_HW_SUPPORT(gcvFEATURE_HALTI4);
    pVscHwCfg->hwFeatureFlags.hasHalti5              = IS_HW_SUPPORT(gcvFEATURE_HALTI5);
    pVscHwCfg->hwFeatureFlags.supportGS              = IS_HW_SUPPORT(gcvFEATURE_GEOMETRY_SHADER);
    pVscHwCfg->hwFeatureFlags.supportTS              = IS_HW_SUPPORT(gcvFEATURE_TESSELLATION);
    pVscHwCfg->hwFeatureFlags.supportInteger         = IS_HW_SUPPORT(gcvFEATURE_SUPPORT_INTEGER);
    pVscHwCfg->hwFeatureFlags.hasSignFloorCeil       = IS_HW_SUPPORT(gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS0);
    pVscHwCfg->hwFeatureFlags.hasSqrtTrig            = IS_HW_SUPPORT(gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS1);
    pVscHwCfg->hwFeatureFlags.hasNewSinCosLogDiv     = IS_HW_SUPPORT(gcvFEATURE_EXTRA_SHADER_INSTRUCTIONS2);
    pVscHwCfg->hwFeatureFlags.hasMediumPrecision     = IS_HW_SUPPORT(gcvFEATURE_MEDIUM_PRECISION);
    pVscHwCfg->hwFeatureFlags.canBranchOnImm         = IS_HW_SUPPORT(gcvFEATURE_BRANCH_ON_IMMEDIATE_REG);
    pVscHwCfg->hwFeatureFlags.supportDual16          = IS_HW_SUPPORT(gcvFEATURE_DUAL_16);
    pVscHwCfg->hwFeatureFlags.hasBugFix8             = IS_HW_SUPPORT(gcvFEATURE_BUG_FIXES8);
    pVscHwCfg->hwFeatureFlags.hasBugFix10            = IS_HW_SUPPORT(gcvFEATURE_BUG_FIXES10);
    pVscHwCfg->hwFeatureFlags.hasBugFix11            = IS_HW_SUPPORT(gcvFEATURE_BUG_FIXES11);
    pVscHwCfg->hwFeatureFlags.hasSelectMapSwizzleFix = IS_HW_SUPPORT(gcvFEATURE_SELECTMAP_SRC0_SWIZZLE_FIX);
    pVscHwCfg->hwFeatureFlags.hasSamplePosSwizzleFix = IS_HW_SUPPORT(gcvFEATURE_SAMPLEPOS_SWIZZLE_FIX);
    pVscHwCfg->hwFeatureFlags.hasLoadAttrOOBFix      = IS_HW_SUPPORT(gcvFEATURE_LOADATTR_OOB_FIX);
    pVscHwCfg->hwFeatureFlags.hasSHEnhance2          = IS_HW_SUPPORT(gcvFEATURE_SHADER_ENHANCEMENTS2);
    pVscHwCfg->hwFeatureFlags.hasInstCache           = IS_HW_SUPPORT(gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE);
    pVscHwCfg->hwFeatureFlags.instBufferUnified      = Hardware->config->unifiedInst;
    pVscHwCfg->hwFeatureFlags.constRegFileUnified    = Hardware->config->unifiedConst;
    pVscHwCfg->hwFeatureFlags.samplerRegFileUnified  = IS_HW_SUPPORT(gcvFEATURE_UNIFIED_SAMPLERS);
    pVscHwCfg->hwFeatureFlags.bigEndianMI            = Hardware->bigEndian;
    pVscHwCfg->hwFeatureFlags.raPushPosW             = IS_HW_SUPPORT(gcvFEATURE_SHADER_HAS_W);
    pVscHwCfg->hwFeatureFlags.vtxInstanceIdAsAttr    = IS_HW_SUPPORT(gcvFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE);
    pVscHwCfg->hwFeatureFlags.vtxInstanceIdAsInteger = IS_HW_SUPPORT(gcvFEATURE_VERTEX_INST_ID_AS_INTEGER);
    pVscHwCfg->hwFeatureFlags.hasSHEnhance3          = IS_HW_SUPPORT(gcvFEATURE_SHADER_ENHANCEMENTS3);
    pVscHwCfg->hwFeatureFlags.rtneRoundingEnabled    = IS_HW_SUPPORT(gcvFEATURE_SHADER_HAS_RTNE);
    pVscHwCfg->hwFeatureFlags.hasInstCachePrefetch   = IS_HW_SUPPORT(gcvFEATURE_SH_INSTRUCTION_PREFETCH);
    pVscHwCfg->hwFeatureFlags.hasThreadWalkerInPS    = Hardware->threadWalkerInPS;
    pVscHwCfg->hwFeatureFlags.has32Attributes        = IS_HW_SUPPORT(gcvFEATURE_PIPELINE_32_ATTRIBUTES);
    pVscHwCfg->hwFeatureFlags.newSteeringICacheFlush = IS_HW_SUPPORT(gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH);
    pVscHwCfg->hwFeatureFlags.gsSupportEmit          = IS_HW_SUPPORT(gcvFEATURE_GS_SUPPORT_EMIT);
    pVscHwCfg->hwFeatureFlags.hasSamplerBaseOffset   = IS_HW_SUPPORT(gcvFEATURE_SAMPLER_BASE_OFFSET);
    pVscHwCfg->hwFeatureFlags.supportStreamOut       = IS_HW_SUPPORT(gcvFEATURE_HW_TFB);
    pVscHwCfg->hwFeatureFlags.supportZeroAttrsInFE   = IS_HW_SUPPORT(gcvFEATURE_ZERO_ATTRIB_SUPPORT);
    pVscHwCfg->hwFeatureFlags.outputCountFix         = IS_HW_SUPPORT(gcvFEATURE_HAS_OUTPUT_COUNT_FIX);
    pVscHwCfg->hwFeatureFlags.varyingPackingLimited  = IS_HW_SUPPORT(gcvFEATURE_VARYING_PACKING_LIMITATION);
    pVscHwCfg->hwFeatureFlags.highpVaryingShift      = IS_HW_SUPPORT(gcvFEATURE_HIGHP_VARYING_SHIFT);
    pVscHwCfg->hwFeatureFlags.needCLXFixes           = IS_HW_SUPPORT(gcvFEATURE_NEED_FIX_FOR_CL_X);
    pVscHwCfg->hwFeatureFlags.needCLXEFixes          = IS_HW_SUPPORT(gcvFEATURE_NEED_FIX_FOR_CL_XE);
    pVscHwCfg->hwFeatureFlags.robustAtomic           = IS_HW_SUPPORT(gcvFEATURE_ROBUST_ATOMIC);
    pVscHwCfg->hwFeatureFlags.newGPIPE               = IS_HW_SUPPORT(gcvFEATURE_NEW_GPIPE);
    pVscHwCfg->hwFeatureFlags.flatDual16Fix          = IS_HW_SUPPORT(gcvFEATURE_SH_FLAT_INTERPOLATION_DUAL16_FIX);
    pVscHwCfg->hwFeatureFlags.supportEVIS            = IS_HW_SUPPORT(gcvFEATURE_EVIS);
    pVscHwCfg->hwFeatureFlags.supportImgAtomic       = IS_HW_SUPPORT(gcvFEATURE_SH_SUPPORT_V4);
    pVscHwCfg->hwFeatureFlags.supportAdvancedInsts   = IS_HW_SUPPORT(gcvFEATURE_ADVANCED_SH_INST);
    pVscHwCfg->hwFeatureFlags.noOneConstLimit        = IS_HW_SUPPORT(gcvFEATURE_SH_NO_ONECONST_LIMIT);
    pVscHwCfg->hwFeatureFlags.hasUniformB0            = IS_HW_SUPPORT(gcvFEATURE_INDEX_CONST_ON_B0);
    pVscHwCfg->hwFeatureFlags.hasSampleMaskInR0ZWFix = IS_HW_SUPPORT(gcvFEATURE_PSIO_SAMPLEMASK_IN_R0ZW_FIX);
    pVscHwCfg->hwFeatureFlags.supportOOBCheck        = IS_HW_SUPPORT(gcvFEATURE_ROBUSTNESS);
    pVscHwCfg->hwFeatureFlags.hasUniversalTexld      = IS_HW_SUPPORT(gcvFEATURE_TX_INTEGER_COORDINATE);
    pVscHwCfg->hwFeatureFlags.hasUniversalTexldV2    = IS_HW_SUPPORT(gcvFEATURE_TX_INTEGER_COORDINATE_V2);
    pVscHwCfg->hwFeatureFlags.hasTexldUFix           = IS_HW_SUPPORT(gcvFEATURE_SH_TEXLD_U_FIX);
    pVscHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp = IS_HW_SUPPORT(gcvFEATURE_SH_IMG_LDST_ON_TEMP);
    pVscHwCfg->hwFeatureFlags.hasICacheAllocCountFix = IS_HW_SUPPORT(gcvFEATURE_SH_ICACHE_ALLOC_COUNT_FIX);
    pVscHwCfg->hwFeatureFlags.hasPSIOInterlock       = IS_HW_SUPPORT(gcvFEATURE_PSIO_INTERLOCK);
    pVscHwCfg->hwFeatureFlags.support128BppImage     = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.supportMSAATexture     = IS_HW_SUPPORT(gcvFEATURE_MSAA_TEXTURE);
    pVscHwCfg->hwFeatureFlags.supportPerCompDepForLS = IS_HW_SUPPORT(gcvFEATURE_LS_SUPPORT_PER_COMP_DEPENDENCY);
    pVscHwCfg->hwFeatureFlags.supportImgAddr         = IS_HW_SUPPORT(gcvFEATURE_IMG_INSTRUCTION);
    pVscHwCfg->hwFeatureFlags.hasUscGosAddrFix       = IS_HW_SUPPORT(gcvFEATURE_USC_GOS_ADDR_FIX);
    pVscHwCfg->hwFeatureFlags.multiCluster           = IS_HW_SUPPORT(gcvFEATURE_MULTI_CLUSTER);
    pVscHwCfg->hwFeatureFlags.hasImageOutBoundaryFix = IS_HW_SUPPORT(gcvFEATURE_IMAGE_OUT_BOUNDARY_FIX);
    pVscHwCfg->hwFeatureFlags.supportTexldOffset     = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.supportLSAtom          = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.supportUnOrdBranch     = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.supportPatchVerticesIn = gcvFALSE;
    pVscHwCfg->hwFeatureFlags.hasHalfDepFix          = IS_HW_SUPPORT(gcvFEATURE_SH_HALF_DEPENDENCY_FIX);
    pVscHwCfg->hwFeatureFlags.supportUSC             = IS_HW_SUPPORT(gcvFEATURE_USC);
    pVscHwCfg->hwFeatureFlags.supportPartIntBranch   = IS_HW_SUPPORT(gcvFEATURE_PARTLY_SUPPORT_INTEGER_BRANCH);
    pVscHwCfg->hwFeatureFlags.supportIntAttrib       = IS_HW_SUPPORT(gcvFEATURE_SUPPORT_INTEGER_ATTRIBUTE);
    pVscHwCfg->hwFeatureFlags.hasTxBiasLodFix        = IS_HW_SUPPORT(gcvFEATURE_TEXTURE_BIAS_LOD_FIX);
    pVscHwCfg->hwFeatureFlags.supportmovai           = IS_HW_SUPPORT(gcvFEATURE_SUPPORT_MOVAI);
    pVscHwCfg->hwFeatureFlags.useGLZ                 = IS_HW_SUPPORT(gcvFEATURE_USE_GL_Z);
    pVscHwCfg->hwFeatureFlags.supportHelperInv       = IS_HW_SUPPORT(gcvFEATURE_HELPER_INVOCATION);
    pVscHwCfg->hwFeatureFlags.supportAdvBlendPart0   = IS_HW_SUPPORT(gcvFEATURE_ADVANCED_BLEND_MODE_PART0);
    pVscHwCfg->hwFeatureFlags.supportStartVertexFE   = IS_HW_SUPPORT(gcvFEATURE_FE_START_VERTEX_SUPPORT);
    pVscHwCfg->hwFeatureFlags.supportTxGather        = IS_HW_SUPPORT(gcvFEATURE_TEXTURE_GATHER);
    pVscHwCfg->hwFeatureFlags.singlePipeHalti1       = IS_HW_SUPPORT(gcvFEATURE_SINGLE_PIPE_HALTI1);
    pVscHwCfg->hwFeatureFlags.supportEVISVX2         = IS_HW_SUPPORT(gcvFEATURE_EVIS_VX2);
    pVscHwCfg->hwFeatureFlags.computeOnly            = IS_HW_SUPPORT(gcvFEATURE_COMPUTE_ONLY);
    pVscHwCfg->hwFeatureFlags.hasBugFix7             = IS_HW_SUPPORT(gcvFEATURE_BUG_FIXES7);
    pVscHwCfg->hwFeatureFlags.hasExtraInst2          = IS_HW_SUPPORT(gcvFEATURE_SHADER_HAS_EXTRA_INSTRUCTIONS2);
    pVscHwCfg->hwFeatureFlags.hasAtomic              = IS_HW_SUPPORT(gcvFEATURE_SHADER_HAS_ATOMIC);
    pVscHwCfg->hwFeatureFlags.supportFullIntBranch   = IS_HW_SUPPORT(gcvFEATURE_FULLLY_SUPPORT_INTEGER_BRANCH);
    pVscHwCfg->hwFeatureFlags.hasDynamicIdxDepFix    = IS_HW_SUPPORT(gcvFEATURE_HALTI5);
    pVscHwCfg->hwFeatureFlags.supportImgLDSTCLamp    = IS_HW_SUPPORT(gcvFEATURE_SH_IMG_LDST_CLAMP);
    /* LODQ doesn't return the correct raw LOD value, which can match the spec requirement. */
    pVscHwCfg->hwFeatureFlags.hasLODQFix             = gcvFALSE;

OnError:
    gcmFOOTER();
    return status;
}

static void
_StallHw(
    gcoHARDWARE Hardware
    )
{
    gctBOOL stallStates = gcvFALSE;
    gctBOOL stallDraw = gcvFALSE;
    gcsHINT_PTR hints = Hardware->SHStates->programState.hints;
    gcsPROGRAM_UNIFIED_STATUS *prevProgUnifiedStatus = &Hardware->prevProgramUnfiedStatus;
    gctBOOL reconfigUSC = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;

    do
    {
        if (Hardware->features[gcvFEATURE_USC])
        {
            if ((Hardware->prevProgramStageBits & gcvPROGRAM_STAGE_COMPUTE_BIT) !=
                (hints->stageBits & gcvPROGRAM_STAGE_COMPUTE_BIT))
            {
                reconfigUSC = gcvTRUE;
            }
        }

        if (((Hardware->config->instructionCount > 256) &&
             (Hardware->config->instructionCount <= 1024) &&
             (!Hardware->features[gcvFEATURE_BUG_FIXES7]))
            )
        {
            stallStates = gcvTRUE;
            break;
        }

        if ((Hardware->prevProgramStageBits & gcvPROGRAM_STAGE_COMPUTE_BIT) &&
             Hardware->prevProgramBarrierUsed &&
             !(hints->stageBits & gcvPROGRAM_STAGE_COMPUTE_BIT))
        {
            stallDraw = gcvTRUE;
        }

        if (prevProgUnifiedStatus->useIcache != hints->unifiedStatus.useIcache)
        {
            stallStates = gcvTRUE;
            break;
        }

        if (prevProgUnifiedStatus->instruction != hints->unifiedStatus.instruction)
        {
            gcmASSERT(prevProgUnifiedStatus->useIcache != hints->unifiedStatus.useIcache);
        }

        if (prevProgUnifiedStatus->constant != hints->unifiedStatus.constant)
        {
            gcmASSERT(gcvFALSE);
        }

        if ((hints->unifiedStatus.instruction) && (!hints->unifiedStatus.useIcache))
        {
            if ((hints->unifiedStatus.instVSEnd >= prevProgUnifiedStatus->instPSStart) ||
                (hints->unifiedStatus.instPSStart <= prevProgUnifiedStatus->instVSEnd))
            {
                stallStates = gcvTRUE;
                break;
            }
        }

        if (hints->unifiedStatus.constant)
        {
            if ((hints->unifiedStatus.constGPipeEnd >= prevProgUnifiedStatus->constPSStart) ||
                (hints->unifiedStatus.constPSStart <= prevProgUnifiedStatus->constGPipeEnd))
            {
                stallStates = gcvTRUE;
                break;
            }
        }

        if (hints->unifiedStatus.sampler)
        {
            if ((hints->unifiedStatus.samplerPSEnd >= prevProgUnifiedStatus->samplerGPipeStart) ||
                (hints->unifiedStatus.samplerGPipeStart <= prevProgUnifiedStatus->samplerPSEnd))
            {
                stallStates = gcvTRUE;
                break;
            }
        }

    }while (gcvFALSE);

    if (stallStates)
    {
        gcmVERIFY_OK(
            gcoHARDWARE_Semaphore(Hardware, gcvWHERE_COMMAND, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL, gcvNULL));
    }
    else if (stallDraw)
    {
        gcmVERIFY_OK(
            gcoHARDWARE_Semaphore(Hardware, gcvWHERE_COMMAND, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE, gcvNULL));
    }


    if (Hardware->features[gcvFEATURE_SNAPPAGE_CMD] &&
        Hardware->features[gcvFEATURE_SNAPPAGE_CMD_FIX])
    {
        gcePROGRAM_STAGE_BIT snapStags = ((Hardware->prevProgramStageBits & (~hints->stageBits)) &
                                          gcvPORGRAM_STAGE_GPIPE);
        if (snapStags)
        {
            gcmVERIFY_OK(
                gcoHARDWARE_SnapPages(
                    Hardware,
                    snapStags,
                    gcvNULL));
        }
    }

    if (reconfigUSC && gcoHAL_GetOption(gcvNULL, gcvOPTION_PREFER_USC_RECONFIG))
    {
        gctUINT32 uscConfig;
        gcmASSERT(Hardware->features[gcvFEATURE_USC]);

        if (hints->stageBits & (gcvPROGRAM_STAGE_COMPUTE_BIT | gcvPROGRAM_STAGE_OPENCL_BIT))
        {
            if (Hardware->features[gcvFEATURE_USC_FULLCACHE_FIX])
            {
                uscConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16)));
            }
            else
            {
                uscConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (0x6 & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))
                          | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16)));
            }
        }
        else
        {
            uscConfig = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) ((gctUINT32) (Hardware->options.uscL1CacheRatio) & ((gctUINT32) ((((1 ?
 2:0) - (0 ? 2:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ?
 2:0)))
                      | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ?
 20:16) - (0 ? 20:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 20:16) - (0 ?
 20:16) + 1))))))) << (0 ? 20:16)));
        }

        gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                                                0x0380C,
                                                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
                                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10)))
                                              | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ?
 11:11) + 1))))))) << (0 ? 11:11))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 11:11) - (0 ? 11:11) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 11:11) - (0 ?
 11:11) + 1))))))) << (0 ? 11:11))),
                                                gcvNULL));

        if (Hardware->features[gcvFEATURE_HW_TFB])
        {
            gcmONERROR(gcoHARDWARE_LoadCtrlStateNEW(Hardware,
                                                    0x1C00C,
                                                    0x12345678,
                                                    gcvNULL));
        }

        gcmONERROR(
            gcoHARDWARE_Semaphore(Hardware,
                                  gcvWHERE_COMMAND,
                                  gcvWHERE_PIXEL,
                                  gcvHOW_SEMAPHORE_STALL,
                                  gcvNULL));

        gcmONERROR(
            gcoHARDWARE_SnapPages(
                    Hardware,
                    0x1F,
                    gcvNULL));

        gcmONERROR(gcoHARDWARE_LoadState32NEW(Hardware,
                                                0x03884,
                                                uscConfig,
                                                gcvNULL));
    }

    /* overwrite to previous one */
    gcoOS_MemCopy(&Hardware->prevProgramUnfiedStatus,
                  &hints->unifiedStatus,
                  gcmSIZEOF(gcsPROGRAM_UNIFIED_STATUS));

    Hardware->prevProgramStageBits = hints->stageBits;
    Hardware->prevProgramBarrierUsed |= (hints->threadGroupSync);

OnError:

    return;
}


/*******************************************************************************
**
**  gcoHARDWARE_LoadProgram
**
**  Load multiple program state.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**      gcsPROGRAM_STATE_PTR ProgramState,
**          Pointer to program state.
**
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoHARDWARE_LoadProgram(
    IN gcoHARDWARE Hardware,
    IN gcePROGRAM_STAGE_BIT StageBits,
    IN gctPOINTER ProgramState
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Hardware=0x%x ProgramState=0x%x",
                   Hardware, ProgramState);

    gcmGETHARDWARE(Hardware);

    gcmVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (ProgramState == gcvNULL)
    {
        Hardware->SHStates->programState.stateBufferSize = 0;
        Hardware->SHStates->programState.stateBuffer = gcvNULL;
        Hardware->SHStates->programState.hints = gcvNULL;
    }
    else
    {
        Hardware->SHStates->programState = *(gcsPROGRAM_STATE_PTR)ProgramState;
    }

    Hardware->SHDirty->shaderDirty = gcvTRUE;

    if ((Hardware->prevProgramStageBits & gcvPROGRAM_STAGE_COMPUTE_BIT) !=
        (Hardware->SHStates->programState.hints->stageBits & gcvPROGRAM_STAGE_COMPUTE_BIT) &&
        !Hardware->features[gcvFEATURE_PSIO_MSAA_CL_FIX])
    {
        Hardware->MsaaDirty->msaaConfigDirty = gcvTRUE;
    }

    /* Need to reprogram centroid table for some DEQP/CTS cases. */
    if ((!Hardware->features[gcvFEATURE_HALTI5]) &&
        (Hardware->patchID == gcvPATCH_DEQP || Hardware->patchID == gcvPATCH_GTFES30))
    {
        Hardware->MsaaDirty->centroidsDirty = gcvTRUE;
    }

    /* Multiple core rendering mode flush relies on these flags,
    ** if hints changes, they need to be dirty to make sure rendering
    ** mode is flushed. */

    Hardware->multiGPURenderingModeDirty = gcvTRUE;

    _StallHw(Hardware);

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_LoadKernel(
    IN gcoHARDWARE Hardware,
    IN gctSIZE_T StateBufferSize,
    IN gctPOINTER StateBuffer,
    IN gcsHINT_PTR Hints
    )
{
    gceSTATUS status;
    gcsPROGRAM_STATE programState;

    gcmHEADER_ARG("Hardware=0x%x StateBufferSize=%zu StateBuffer=0x%x Hints=0x%x",
        Hardware, StateBufferSize, StateBuffer, Hints);

    gcmGETHARDWARE(Hardware);

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(Hardware, gcvPIPE_3D, gcvNULL));

    programState.stateBuffer = StateBuffer;
    programState.stateBufferSize = (gctUINT32)StateBufferSize;
    programState.hints = Hints;

    /* Load shaders. */
    if (StateBufferSize > 0)
    {
        gcmONERROR(gcoHARDWARE_LoadProgram(Hardware,
            Hints->stageBits,
            &programState));
    }

    if (Hardware->threadWalkerInPS)
    {
        /* Set input count. */
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
            0x01008,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (Hints->fsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 24:24) - (0 ?
 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ? 24:24) + 1))))))) << (0 ?
 24:24))) | (((gctUINT32) ((gctUINT32) (Hints->psInputControlHighpPosition) & ((gctUINT32) ((((1 ?
 24:24) - (0 ? 24:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 24:24) - (0 ?
 24:24) + 1))))))) << (0 ? 24:24)))));
#if TEMP_SHADER_PATCH
        switch (Hints->pachedShaderIdentifier)
        {
        case gcvMACHINECODE_GLB27_RELEASE_0:
            Hints->fsMaxTemp = 0x00000004;
            break;

        case gcvMACHINECODE_GLB25_RELEASE_0:
            Hints->fsMaxTemp = 0x00000008;
            break;

        case gcvMACHINECODE_GLB25_RELEASE_1:
            Hints->fsMaxTemp = 0x00000008;
            break;

        default:
            break;
        }
#endif

        /* Set temporary register control. */
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
            0x0100C,
            Hints->fsMaxTemp));
    }
    else
    {
        /* Set input count. */
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
            0x00808,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (Hints->fsInputCount) & ((gctUINT32) ((((1 ?
 5:0) - (0 ? 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) ((gctUINT32) (~0) & ((gctUINT32) ((((1 ? 12:8) - (0 ?
 12:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ?
 12:8)))));

        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
            0x00820,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 13:8) - (0 ?
 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ? 13:8) - (0 ?
 13:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:8) - (0 ? 13:8) + 1))))))) << (0 ?
 13:8))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) | (((gctUINT32) ((gctUINT32) (2) & ((gctUINT32) ((((1 ? 21:16) - (0 ?
 21:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 21:16) - (0 ? 21:16) + 1))))))) << (0 ?
 21:16))) |
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 29:24) - (0 ?
 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24))) | (((gctUINT32) ((gctUINT32) (3) & ((gctUINT32) ((((1 ? 29:24) - (0 ?
 29:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 29:24) - (0 ? 29:24) + 1))))))) << (0 ?
 29:24)))));
        gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
            0x0080C,
            Hints->fsMaxTemp));
    }

    /* Set output count. */
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
        0x00804,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 5:0) - (0 ?
 5:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:0) - (0 ? 5:0) + 1))))))) << (0 ?
 5:0)))));

    /* Set local storage register count. */
    /* Set it to 0 since it is not used. */
    gcmONERROR(gcoHARDWARE_LoadState32(Hardware,
        0x00924,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))));

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoHARDWARE_InvokeThreadWalkerCL
**
**  Start OCL thread walker.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gcsTHREAD_WALKER_INFO_PTR Info
**          Pointer to the informational structure.
*/

gceSTATUS
gcoHARDWARE_InvokeThreadWalkerCL(
                   IN gcoHARDWARE Hardware,
                   IN gcsTHREAD_WALKER_INFO_PTR Info
                   )
{
    gceSTATUS status;
    gctPOINTER *cmdBuffer = gcvNULL;
    gctUINT allocation;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Info=0x%x", Hardware, Info);

    gcmGETHARDWARE(Hardware);

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, cmdBuffer);

    cmdBuffer = (gctPOINTER *)&memory;

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, cmdBuffer));

    if (Hardware->SHDirty->shaderDirty)
    {
        /* Flush shader states. */
        gcmONERROR(gcoHARDWARE_FlushShaders(Hardware, gcvPRIMITIVE_TRIANGLE_LIST, cmdBuffer));
    }

    if (Hardware->features[gcvFEATURE_SH_INSTRUCTION_PREFETCH])
    {
        gcmONERROR(gcoHARDWARE_FlushPrefetchInst(Hardware, cmdBuffer));
    }

    cmdBuffer = gcvNULL;

    /* Calculate thread allocation. */

    allocation           = Info->workGroupSizeX;
    if (Info->dimensions > 1)
    {
        allocation          *= Info->workGroupSizeY;
    }

    if (Info->dimensions > 2)
    {
        allocation          *= Info->workGroupSizeZ;
    }

    Info->threadAllocation = gcmCEIL((gctFLOAT)allocation / (Hardware->config->shaderCoreCount * 4));

    if (Hardware->features[gcvFEATURE_SHADER_ENHANCEMENTS2])
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0240) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0240, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (Info->dimensions) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:4) - (0 ?
 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (Info->traverseOrder) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathX) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ?
 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathY) & ((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathZ) & ((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeX) & ((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeY) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeZ) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24))) | (((gctUINT32) ((gctUINT32) (Info->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0249) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0249, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (Hardware->features[gcvFEATURE_SH_MULTI_WG_PACK] ? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) ((Info->barrierUsed ?
 0x0 : 0x1)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ?
 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) : 0));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0247) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0247, Info->threadAllocation );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024B, Info->globalOffsetX );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024D, Info->globalOffsetY );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024F, Info->globalOffsetZ );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0256) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0256, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Info->globalScaleX) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0257) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0257, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Info->globalScaleY) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0258) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0258, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) | (((gctUINT32) ((gctUINT32) (Info->globalScaleZ) & ((gctUINT32) ((((1 ?
 7:0) - (0 ? 7:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 7:0) - (0 ? 7:0) + 1))))))) << (0 ?
 7:0))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)6  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (6 ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0250) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0250,
            Info->workGroupCountX - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0251,
            Info->workGroupCountY - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0252,
            Info->workGroupCountZ - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0253,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeX - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0254,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeY - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0255,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeZ - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }
    else
    {
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)8  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (8 ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0240) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0240,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (Info->dimensions) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (Info->traverseOrder) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathX) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathY) & ((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathZ) & ((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeX) & ((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeY) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeZ) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24))) | (((gctUINT32) ((gctUINT32) (Info->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0241,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Info->globalSizeX) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetX) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0242,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Info->globalSizeY) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0243,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (Info->globalSizeZ) & ((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetZ) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0244,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeX - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->workGroupCountX - 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0245,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeY - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->workGroupCountY - 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0246,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeZ - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->workGroupCountZ - 1) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0247,
            Info->threadAllocation
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0248) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0248, 0xBADABEEB );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, cmdBuffer);

    gcoBUFFER_OnIssueFence(Hardware->engine[gcvENGINE_RENDER].buffer);

    /* Flush the Shader L1 cache. */
    gcmONERROR(gcoHARDWARE_LoadCtrlState(
        Hardware,
        0x0380C,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 5:5) - (0 ?
 5:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 5:5) - (0 ? 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)))
           |((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 2:2) - (0 ?
 2:2) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ? 2:2) - (0 ? 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
        ));

    /* Add FE - PE samaphore stall to prevent unwanted SHL1_CACHE flush. */
    gcmONERROR(gcoHARDWARE_Semaphore(
        Hardware, gcvWHERE_COMMAND, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL, gcvNULL));

    gcoHARDWARE_SendFence(Hardware, gcvFALSE, gcvENGINE_RENDER, gcvNULL);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoHARDWARE_InvokeThreadWalkerGL
**
**  Start OGL thread walker.
**
**  INPUT:
**
**      gcoHARDWARE Hardware
**          Pointer to an gcoHARDWARE object.
**
**
**      gcsTHREAD_WALKER_INFO_PTR Info
**          Pointer to the informational structure.
*/

gceSTATUS
gcoHARDWARE_InvokeThreadWalkerGL(
    IN gcoHARDWARE Hardware,
    IN gcsTHREAD_WALKER_INFO_PTR Info
    )
{
    gceSTATUS status;
    gctPOINTER  *outSide = gcvNULL;
    gctUINT allocation;
    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Info=0x%x", Hardware, Info);

    gcmDEBUG_VERIFY_ARGUMENT(Info != gcvNULL);

    gcmGETHARDWARE(Hardware);
    /* Calculate thread allocation. */
    allocation = Info->workGroupSizeX *Info->workGroupSizeY * Info->workGroupSizeZ;

    Info->threadAllocation = gcmCEIL((gctFLOAT)allocation / (Hardware->config->shaderCoreCount * 4));
    Info->valueOrder = Hardware->SHStates->programState.hints->valueOrder;

    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outSide);

    /* Switch to the 3D pipe. */
    gcmONERROR(gcoHARDWARE_SelectPipe(gcvNULL, gcvPIPE_3D, (gctPOINTER*)&memory));

    if (Hardware->TXDirty->textureDirty)
    {
        gcmONERROR((*Hardware->funcPtr->programTexture)(Hardware, (gctPOINTER*)&memory));
    }

    if (Hardware->SHDirty->shaderDirty)
    {
        gcmONERROR(gcoHARDWARE_FlushDepthOnly(Hardware));
        /* Flush shader states. */
        gcmONERROR(gcoHARDWARE_FlushShaders(Hardware, gcvPRIMITIVE_TRIANGLE_LIST, (gctPOINTER*)&memory));
    }

    if (Hardware->MsaaDirty->msaaConfigDirty)
    {
        /* Flush anti-alias states. */
        gcmONERROR(gcoHARDWARE_FlushSampling(Hardware, (gctPOINTER*)&memory));
    }

    if (Hardware->features[gcvFEATURE_SH_INSTRUCTION_PREFETCH])
    {
        gcmONERROR(gcoHARDWARE_FlushPrefetchInst(Hardware, (gctPOINTER*)&memory));
    }

    if (Hardware->features[gcvFEATURE_DRAW_ID])
    {
        gcmONERROR(gcoHARDWARE_FlushDrawID(Hardware, (gctPOINTER*)&memory));
    }

    if (Hardware->config->gpuCoreCount > 1)
    {
        gcmONERROR(gcoHARDWARE_FlushMultiGPURenderingMode(Hardware, (gctPOINTER*)&memory));
    }


    if (Hardware->stallSource < Hardware->stallDestination)
    {
        /* Stall if we have to. */
        gcmONERROR(gcoHARDWARE_Semaphore(Hardware,
                                         Hardware->stallSource,
                                         Hardware->stallDestination,
                                         gcvHOW_STALL,
                                         (gctPOINTER*)&memory));
    }

    gcmASSERT(Hardware->features[gcvFEATURE_SHADER_ENHANCEMENTS2]);

    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0240) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0240, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (Info->dimensions) & ((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 6:4) - (0 ?
 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | (((gctUINT32) ((gctUINT32) (Info->traverseOrder) & ((gctUINT32) ((((1 ?
 6:4) - (0 ? 6:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 6:4) - (0 ? 6:4) + 1))))))) << (0 ?
 6:4))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 8:8) - (0 ?
 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathX) & ((gctUINT32) ((((1 ?
 8:8) - (0 ? 8:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 8:8) - (0 ? 8:8) + 1))))))) << (0 ?
 8:8))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:9) - (0 ?
 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathY) & ((gctUINT32) ((((1 ?
 9:9) - (0 ? 9:9) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:9) - (0 ? 9:9) + 1))))))) << (0 ?
 9:9))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 10:10) - (0 ?
 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ? 10:10) + 1))))))) << (0 ?
 10:10))) | (((gctUINT32) ((gctUINT32) (Info->enableSwathZ) & ((gctUINT32) ((((1 ?
 10:10) - (0 ? 10:10) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 10:10) - (0 ?
 10:10) + 1))))))) << (0 ? 10:10))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeX) & ((gctUINT32) ((((1 ?
 15:12) - (0 ? 15:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:12) - (0 ?
 15:12) + 1))))))) << (0 ? 15:12))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeY) & ((gctUINT32) ((((1 ?
 19:16) - (0 ? 19:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 19:16) - (0 ?
 19:16) + 1))))))) << (0 ? 19:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | (((gctUINT32) ((gctUINT32) (Info->swathSizeZ) & ((gctUINT32) ((((1 ?
 23:20) - (0 ? 23:20) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 23:20) - (0 ?
 23:20) + 1))))))) << (0 ? 23:20))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24))) | (((gctUINT32) ((gctUINT32) (Info->valueOrder) & ((gctUINT32) ((((1 ?
 26:24) - (0 ? 26:24) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:24) - (0 ?
 26:24) + 1))))))) << (0 ? 26:24))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0249) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0249, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (Hardware->features[gcvFEATURE_SH_MULTI_WG_PACK] ? ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) ((Info->barrierUsed ?
 0x0 : 0x1)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ?
 16:16) - (0 ? 16:16) + 1))))))) << (0 ? 16:16))) : 0));    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0247) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0247, Info->threadAllocation );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024B) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024B, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetX) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024D) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024D, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetY) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x024F) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x024F, ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) | (((gctUINT32) ((gctUINT32) (Info->globalOffsetZ) & ((gctUINT32) ((((1 ?
 31:16) - (0 ? 31:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:16) - (0 ?
 31:16) + 1))))))) << (0 ? 31:16))) );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    if (Hardware->features[gcvFEATURE_COMPUTE_INDIRECT] && Info->indirect)
    {
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)3  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (3 ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0253) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0253,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeX - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0254,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeY - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0255,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeZ - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }
    else
    {
        {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)6  <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (6 ) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0250) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0250,
            Info->workGroupCountX - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0251,
            Info->workGroupCountY - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0252,
            Info->workGroupCountZ - 1
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0253,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeX - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0254,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeY - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETSTATEDATA_NEW(
            stateDelta, reserve, memory, gcvFALSE, 0x0255,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ? 9:0) - (0 ?
 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0))) | (((gctUINT32) ((gctUINT32) (Info->workGroupSizeZ - 1) & ((gctUINT32) ((((1 ?
 9:0) - (0 ? 9:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:0) - (0 ? 9:0) + 1))))))) << (0 ?
 9:0)))
            );

        gcmSETFILLER_NEW(
            reserve, memory
            );

        gcmENDSTATEBATCH_NEW(
            reserve, memory
            );
    }

    gcoHARDWARE_MultiGPUSync(Hardware, &memory);
    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0); memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_0_MASK << gcmTO_CHIP_ID(0));
 } };


    if (Hardware->features[gcvFEATURE_COMPUTE_INDIRECT] && Info->indirect)
    {
        gctUINT32 constantBase = 0;
        gctBOOL unifiedConst = Hardware->SHStates->programState.hints->unifiedStatus.constant;

        if (!unifiedConst)
        {
            constantBase = 0x00000100;
        }

        gcmASSERT(Hardware->threadWalkerInPS);
        if (unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
        {
            /* ComputeIndirect will program unified c0, set correct mode */
            {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0218, ((((gctUINT32) (Hardware->SHStates->programState.hints->shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))) | (((gctUINT32) ((gctUINT32) (0x0) & ((gctUINT32) ((((1 ?
 16:16) - (0 ? 16:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 16:16) - (0 ?
 16:16) + 1))))))) << (0 ? 16:16))));    gcmENDSTATEBATCH_NEW(reserve, memory);
};

        }

        /* set uniform to store work group count */
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x01F3) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x01F3, constantBase + Info->groupNumberUniformIdx);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        /* Program the GCCMD_DRAW_INDIRECT_COMMAND.Command data. */
        memory[0] =
                      ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x11 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)));

        memory[1] = Info->baseAddress;

        memory += 2;


        /* Dump the indirect compute command. */
        gcmDUMP(gcvNULL,
                "#[compute.indirectindex (%d,%d,%d) 0x%08X]",
                Info->workGroupSizeX, Info->workGroupSizeY, Info->workGroupSizeZ, Info->baseAddress);

    }
    else
    {
        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0248) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETCTRLSTATE_NEW(stateDelta, reserve, memory, 0x0248, 0xBADABEEB );
    gcmENDSTATEBATCH_NEW(reserve, memory);
};


        gcmDUMP(gcvNULL,
                "#[compute (%d,%d,%d), workGroupNum(%d,%d,%d)",
                 Info->workGroupSizeX, Info->workGroupSizeY, Info->workGroupSizeZ,
                 Info->workGroupCountX, Info->workGroupCountY, Info->workGroupCountZ);
    }

    { if (Hardware->config->gpuCoreCount > 1) { *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x0D & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | gcvCORE_3D_ALL_MASK; memory++;
  gcmDUMP(gcvNULL, "#[chip.enable 0x%04X]", gcvCORE_3D_ALL_MASK);
 } };

    gcoHARDWARE_MultiGPUSync(Hardware, &memory);

#if gcdFRAMEINFO_STATISTIC
    gcmONERROR(gcoHARDWARE_FlushDrawID(Hardware, (gctPOINTER *)&memory));
#endif

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outSide);


OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHARDWARE_ProgramUniform(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT Columns,
    IN gctUINT Rows,
    IN gctCONST_POINTER Values,
    IN gctBOOL FixedPoint,
    IN gctBOOL ConvertToFloat,
    IN gcSHADER_KIND Type
    )
{
    gceSTATUS status;
    gctUINT i, j;
    gctUINT32_PTR src = (gctUINT32_PTR) Values;
    gctUINT32 address = Address >> 2;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER(reserve, stateDelta, memory, reserveSize);

    gcmHEADER_ARG("Hardware=0x%x, Address=%u Columns=%u Rows=%u Values=%p FixedPoint=%d Type=%d",
                  Hardware, Address, Columns, Rows, Values, FixedPoint, Type);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Values != gcvNULL);

    /* Get the current hardware object. */
    gcmGETHARDWARE(Hardware);

    if (address >= Hardware->maxState)
    {
        /* If the uniform (sampler) was not assigned address, cannot call into here */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    /* Determine the size of the buffer to reserve. */
    reserveSize = Rows * gcmALIGN((1 + Columns) * gcmSIZEOF(gctUINT32), 8);

    if (Hardware->unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
    {
        reserveSize += gcmALIGN((1 + 1) * gcmSIZEOF(gctUINT32), 8);
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER(Hardware, reserve, stateDelta, memory, reserveSize);

    if (Hardware->unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
    {
        gctUINT32 shaderConfigData = Hardware->SHStates->programState.hints ?
                                     Hardware->SHStates->programState.hints->shaderConfigData : 0;

        shaderConfigData = ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((Type != gcSHADER_TYPE_VERTEX)) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));

        {    {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)1 <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, 0x0218, 1);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA(stateDelta, reserve, memory, gcvFALSE, 0x0218, shaderConfigData);
     gcmENDSTATEBATCH(reserve, memory);
};

    }

    /* Walk all rows. */
    for (i = 0; i < Rows; i++)
    {
        /* Begin the state batch. */
        {    gcmASSERT(((memory - gcmUINT64_TO_TYPE(reserve->lastReserve,
 gctUINT32_PTR)) & 1) == 0);    gcmASSERT((gctUINT32)Columns <= 1024);
    gcmVERIFYLOADSTATEDONE(reserve);
    gcmSTORELOADSTATE(reserve, memory, address, Columns);
    *memory++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (FixedPoint) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Columns) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));gcmSKIPSECUREUSER();
};


        /* Walk all columns. */
        for (j = 0; j < Columns; j++)
        {
            gctINT32 data = *src++;
            if (ConvertToFloat)
            {
                gctFLOAT fData = (gctFLOAT)data;
                data = *((gctUINT32_PTR)&fData);
            }

            /* Set the state value. */
            gcmSETSTATEDATA(stateDelta,
                            reserve,
                            memory,
                            FixedPoint,
                            address + j,
                            data);
        }

        if ((j & 1) == 0)
        {
            /* Align. */
            gcmSETFILLER(reserve, memory);
        }

        /* End the state batch. */
        gcmENDSTATEBATCH(reserve, memory);

        /* Next row. */
        address += 4;
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER(Hardware, reserve, memory, reserveSize);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_ProgramUniformEx(
    IN gcoHARDWARE Hardware,
    IN gctUINT32 Address,
    IN gctUINT Columns,
    IN gctUINT Rows,
    IN gctUINT Arrays,
    IN gctBOOL IsRowMajor,
    IN gctUINT MatrixStride,
    IN gctUINT ArrayStride,
    IN gctCONST_POINTER Values,
    IN gceUNIFORMCVT Convert,
    IN gcSHADER_KIND Type
    )
{
    gceSTATUS status;
    gctUINT arr, row, col;
    gctUINT32 address = Address >> 2;
    gctPOINTER *outSide = gcvNULL;
    gctUINT8_PTR pArray = (gctUINT8_PTR)Values;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x, Address=%u Columns=%u Rows=%u Arrays=%u IsRowMajor=%d "
                  "MatrixStride=%d ArrayStride=%d Values=%p Convert=%d Type=%d",
                  Hardware, Address, Columns, Rows, Arrays, IsRowMajor,
                  MatrixStride, ArrayStride, Values, Convert, Type);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Values != gcvNULL);

    /* Get the current hardware object. */
    gcmGETHARDWARE(Hardware);

    if (address >= Hardware->maxState)
    {
        /* If the uniform (sampler) was not assigned address, cannot call into here */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outSide);

    if (Hardware->unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
    {
        gctUINT32 shaderConfigData = Hardware->SHStates->programState.hints ?
                                     Hardware->SHStates->programState.hints->shaderConfigData : 0;

        shaderConfigData = ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((Type != gcSHADER_TYPE_VERTEX)) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0218, shaderConfigData);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

    }

    for (arr = 0; arr < Arrays; ++arr)
    {
        /* Walk all rows. */
        for (row = 0; row < Rows; ++row)
        {
            /* Begin the state batch. */
            {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)Columns <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (Columns) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};


            /* Walk all columns. */
            for (col = 0; col < Columns; ++col)
            {
                gctUINT8_PTR pData = IsRowMajor
                                   ? pArray + col * MatrixStride + (row << 2)
                                   : pArray + row * MatrixStride + (col << 2);

                gctUINT data = *(gctUINT_PTR)pData;

                if (Convert == gcvUNIFORMCVT_TO_BOOL)
                {
                    data = (data == 0) ? 0 : 1;
                }
                else if (Convert == gcvUNIFORMCVT_TO_FLOAT)
                {
                    gctFLOAT fData = (gctFLOAT)(gctINT)data;
                    data = *((gctUINT32_PTR)&fData);
                }

                /* Set the state value. */
                gcmSETSTATEDATA_NEW(stateDelta,
                                    reserve,
                                    memory,
                                    gcvFALSE,
                                    address + col,
                                    data);
            }

            if ((col & 1) == 0)
            {
                /* Align. */
                gcmSETFILLER_NEW(reserve, memory);
            }

            /* End the state batch. */
            gcmENDSTATEBATCH_NEW(reserve, memory);

            /* Next row. */
            address += 4;
        }

        pArray += ArrayStride;
    }

    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outSide);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHARDWARE_BindBufferBlock(
        IN gcoHARDWARE Hardware,
        IN gctUINT32 Address,
        IN gctUINT32 Base,
        IN gctSIZE_T Offset,
        IN gctSIZE_T Size,
        IN gcSHADER_KIND Type
        )
{
    gceSTATUS status;
    gctUINT32 address = Address >> 2;
    gctPOINTER *outSide = gcvNULL;

    /* Define state buffer variables. */
    gcmDEFINESTATEBUFFER_NEW(reserve, stateDelta, memory);

    gcmHEADER_ARG("Hardware=0x%x Address=%u Base=%u Offset=%lu Size=%lu Type=%d",
                  Hardware, Address, Base, Offset, Size, Type);

    /* Get the current hardware object. */
    gcmGETHARDWARE(Hardware);

    if (address >= Hardware->maxState)
    {
        /* If the uniform (sampler) was not assigned address, cannot call into here */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    /* Reserve space in the command buffer. */
    gcmBEGINSTATEBUFFER_NEW(Hardware, reserve, stateDelta, memory, outSide);

    if (Hardware->unifiedConst && !Hardware->features[gcvFEATURE_NEW_STEERING_AND_ICACHE_FLUSH])
    {
        gctUINT32 shaderConfigData = Hardware->SHStates->programState.hints ?
                                        Hardware->SHStates->programState.hints->shaderConfigData : 0;

        shaderConfigData = ((((gctUINT32) (shaderConfigData)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) ((Type != gcSHADER_TYPE_VERTEX)) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)));

        {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (0x0218) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, 0x0218, shaderConfigData);
    gcmENDSTATEBATCH_NEW(reserve, memory);
};

     }

    /* Set buffer location. */
    {    {    gcmVERIFYLOADSTATEALIGNED(reserve, memory);
    gcmASSERT((gctUINT32)1 <= 1024);
    *memory++        = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 31:27) - (0 ?
 31:27) + 1))))))) << (0 ? 31:27)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26))) | (((gctUINT32) ((gctUINT32) (gcvFALSE) & ((gctUINT32) ((((1 ?
 26:26) - (0 ? 26:26) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 26:26) - (0 ?
 26:26) + 1))))))) << (0 ? 26:26)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16))) | (((gctUINT32) ((gctUINT32) (1) & ((gctUINT32) ((((1 ?
 25:16) - (0 ? 25:16) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 25:16) - (0 ?
 25:16) + 1))))))) << (0 ? 25:16)))        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ? 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) (address) & ((gctUINT32) ((((1 ? 15:0) - (0 ?
 15:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 15:0) - (0 ? 15:0) + 1))))))) << (0 ?
 15:0)));    gcmSKIPSECUREUSER();
};
    gcmSETSTATEDATA_NEW(stateDelta, reserve, memory, gcvFALSE, address,
 Base + Offset );    gcmENDSTATEBATCH_NEW(reserve, memory);
};


    /* Validate the state buffer. */
    gcmENDSTATEBUFFER_NEW(Hardware, reserve, memory, outSide);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif


