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


/*
** Main interface header that outside world uses
*/

#ifndef __gc_vsc_drvi_interface_h_
#define __gc_vsc_drvi_interface_h_

#include "gc_vsc_precomp.h"

/* It will be fully removed after VIR totally replaces of gcSL */
#include "old_impl/gc_vsc_old_drvi_interface.h"

/* 0.0.1.1 add chipModel and ChipRevision, Nov. 30, 2017 */
/* 0.0.1.2 change VIR_Operand size, April. 9, 2017 */
/* 1.3 for 6.2.4 release */
/* 0.0.1.4 add atomic patch library function */
/* 0.0.1.5 implement lib function nmin, nmax and nclamp, Oct. 24, 2018 */
#define gcdVIR_SHADER_BINARY_FILE_VERSION gcmCC(SHADER_64BITMODE, 0, 1, 5)

#define gcdVIR_PROGRAM_BINARY_FILE_VERSION gcmCC(SHADER_64BITMODE, 0, 1, 5)

#define gcdSUPPORT_COMPUTE_SHADER   1
#define gcdSUPPORT_TESS_GS_SHADER   1
#define gcdSUPPORT_OCL_1_2          1
#define TREAT_ES20_INTEGER_AS_FLOAT 0
#define __USE_IMAGE_LOAD_TO_ACCESS_SAMPLER_BUFFER__ 1

BEGIN_EXTERN_C()

typedef void* DRIVER_HANDLE;

/* Callback defintions. Note that first param of all drv-callbacks must be hDrv which
   designates the specific client driver who needs insure param hDrv of callbacks and
   VSC_SYS_CONTEXT::hDrv point to the same underlying true driver object (driver-contex/
   driver device/...) */
typedef void* (*PFN_ALLOC_VIDMEM_CB)(DRIVER_HANDLE hDrv,
                                     gceSURF_TYPE type,
                                     gctSTRING tag,
                                     gctSIZE_T size,
                                     gctUINT32 align,
                                     gctPOINTER* ppOpaqueNode,
                                     gctPOINTER* ppVirtualAddr,
                                     gctUINT32* pPhysicalAddr,
                                     gctPOINTER pInitialData,
                                     gctBOOL bZeroMemory);
typedef void* (*PFN_FREE_VIDMEM_CB)(DRIVER_HANDLE hDrv,
                                    gceSURF_TYPE type,
                                    gctSTRING tag,
                                    gctPOINTER pOpaqueNode);

typedef struct _VSC_DRV_CALLBACKS
{
    PFN_ALLOC_VIDMEM_CB pfnAllocVidMemCb;
    PFN_FREE_VIDMEM_CB  pfnFreeVidMemCb;
}VSC_DRV_CALLBACKS, *PVSC_DRV_CALLBACKS;

/* VSC hardware (chip) configuration that poses on (re)-compiling */
typedef struct _VSC_HW_CONFIG
{
    struct
    {
        gctUINT          hasHalti0              : 1;
        gctUINT          hasHalti1              : 1;
        gctUINT          hasHalti2              : 1;
        gctUINT          hasHalti3              : 1;
        gctUINT          hasHalti4              : 1;
        gctUINT          hasHalti5              : 1;
        gctUINT          supportGS              : 1;
        gctUINT          supportTS              : 1;
        gctUINT          supportInteger         : 1;
        gctUINT          hasSignFloorCeil       : 1;
        gctUINT          hasSqrtTrig            : 1;
        gctUINT          hasNewSinCosLogDiv     : 1;
        gctUINT          canBranchOnImm         : 1;
        gctUINT          supportDual16          : 1;
        gctUINT          hasBugFix8             : 1;
        gctUINT          hasBugFix10            : 1;
        gctUINT          hasBugFix11            : 1;
        gctUINT          hasSelectMapSwizzleFix : 1;
        gctUINT          hasSamplePosSwizzleFix : 1;
        gctUINT          hasLoadAttrOOBFix      : 1;
        gctUINT          hasSampleMaskInR0ZWFix : 1;
        gctUINT          hasICacheAllocCountFix : 1;
        gctUINT          hasSHEnhance2          : 1;
        gctUINT          hasMediumPrecision     : 1;
        gctUINT          hasInstCache           : 1;
        gctUINT          hasInstCachePrefetch   : 1;
        gctUINT          instBufferUnified      : 1;
        gctUINT          constRegFileUnified    : 1;
        gctUINT          samplerRegFileUnified  : 1;
        gctUINT          bigEndianMI            : 1;
        gctUINT          raPushPosW             : 1;
        gctUINT          vtxInstanceIdAsAttr    : 1;
        gctUINT          vtxInstanceIdAsInteger : 1;
        gctUINT          gsSupportEmit          : 1;
        gctUINT          highpVaryingShift      : 1;
        gctUINT          needCLXFixes           : 1;
        gctUINT          needCLXEFixes          : 1;
        gctUINT          flatDual16Fix          : 1;
        gctUINT          supportEVIS            : 1;
        gctUINT          supportImgAtomic       : 1;
        gctUINT          supportAdvancedInsts   : 1;
        gctUINT          noOneConstLimit        : 1;
        gctUINT          hasUniformB0           : 1;
        gctUINT          supportOOBCheck        : 1;
        gctUINT          hasUniversalTexld      : 1;
        gctUINT          hasUniversalTexldV2    : 1;
        gctUINT          hasTexldUFix           : 1;
        gctUINT          canSrc0OfImgLdStBeTemp : 1;
        gctUINT          hasPSIOInterlock       : 1;
        gctUINT          support128BppImage     : 1;
        gctUINT          supportMSAATexture     : 1;
        gctUINT          supportPerCompDepForLS : 1;
        gctUINT          supportImgAddr         : 1;
        gctUINT          hasUscGosAddrFix       : 1;
        gctUINT          multiCluster           : 1;
        gctUINT          hasImageOutBoundaryFix : 1;
        gctUINT          supportTexldOffset     : 1;
        gctUINT          supportLSAtom          : 1;
        gctUINT          supportUnOrdBranch     : 1;
        gctUINT          supportPatchVerticesIn : 1;
        gctUINT          hasHalfDepFix          : 1;
        gctUINT          supportUSC             : 1;
        gctUINT          supportPartIntBranch   : 1;
        gctUINT          supportIntAttrib       : 1;
        gctUINT          hasTxBiasLodFix        : 1;
        gctUINT          supportmovai           : 1;
        gctUINT          useGLZ                 : 1;
        gctUINT          supportHelperInv       : 1;
        gctUINT          supportAdvBlendPart0   : 1;
        gctUINT          supportStartVertexFE   : 1;
        gctUINT          supportTxGather        : 1;
        gctUINT          singlePipeHalti1       : 1;
        gctUINT          supportEVISVX2         : 1;
        gctUINT          computeOnly            : 1;
        gctUINT          hasBugFix7             : 1;
        gctUINT          hasExtraInst2          : 1;
        gctUINT          hasAtomic              : 1;
        gctUINT          supportFullIntBranch   : 1;
        gctUINT          hasDynamicIdxDepFix    : 1;
        gctUINT          hasLODQFix             : 1;
        gctUINT          hasImageLoadEnableFix  : 1;

        /* Followings will be removed after shader programming is removed out of VSC */
        gctUINT          hasSHEnhance3          : 1;
        gctUINT          rtneRoundingEnabled    : 1;
        gctUINT          hasThreadWalkerInPS    : 1;
        gctUINT          newSteeringICacheFlush : 1;
        gctUINT          has32Attributes        : 1;
        gctUINT          hasSamplerBaseOffset   : 1;
        gctUINT          supportStreamOut       : 1;
        gctUINT          supportZeroAttrsInFE   : 1;
        gctUINT          outputCountFix         : 1;
        gctUINT          varyingPackingLimited  : 1;
        gctUINT          robustAtomic           : 1;
        gctUINT          newGPIPE               : 1;
        gctUINT          supportImgLDSTCLamp    : 1;

        gctUINT          hasUSCAtomicFix2       : 1;
        gctUINT          reserved               : 1;

    } hwFeatureFlags;

    gctUINT              chipModel;
    gctUINT              chipRevision;
    gctUINT              maxCoreCount;
    gctUINT              maxThreadCountPerCore;
    gctUINT              maxVaryingCount;
    gctUINT              maxAttributeCount;
    gctUINT              maxRenderTargetCount;
    gctUINT              maxGPRCountPerCore;
    gctUINT              maxGPRCountPerThread;
    gctUINT              maxHwNativeTotalInstCount;
    gctUINT              maxTotalInstCount;
    gctUINT              maxVSInstCount;
    gctUINT              maxPSInstCount;
    gctUINT              maxHwNativeTotalConstRegCount;
    gctUINT              maxTotalConstRegCount;
    gctUINT              unifiedConst;
    gctUINT              maxVSConstRegCount;
    gctUINT              maxTCSConstRegCount;  /* HS */
    gctUINT              maxTESConstRegCount;  /* DS */
    gctUINT              maxGSConstRegCount;
    gctUINT              maxPSConstRegCount;
    gctUINT              vsSamplerRegNoBase;
    gctUINT              tcsSamplerRegNoBase;  /* HS */
    gctUINT              tesSamplerRegNoBase;  /* DS */
    gctUINT              gsSamplerRegNoBase;
    gctUINT              psSamplerRegNoBase;
    gctUINT              csSamplerRegNoBase;
    gctUINT              maxVSSamplerCount;
    gctUINT              maxTCSSamplerCount;   /* HS */
    gctUINT              maxTESSamplerCount;   /* DS */
    gctUINT              maxGSSamplerCount;
    gctUINT              maxPSSamplerCount;
    gctUINT              maxCSSamplerCount;
    gctUINT              maxHwNativeTotalSamplerCount;
    gctUINT              maxSamplerCountPerShader;
    gctUINT              maxUSCAttribBufInKbyte;
    gctUINT              maxLocalMemSizeInByte;
    gctUINT              maxResultCacheWinSize;
    gctUINT              vsSamplerNoBaseInInstruction;
    gctUINT              psSamplerNoBaseInInstruction;

    /* Caps for workGroupSize. */
    gctUINT              initWorkGroupSizeToCalcRegCount;
    gctUINT              maxWorkGroupSize;
    gctUINT              minWorkGroupSize;

    /* Followings will be removed after shader programming is removed out of VSC */
    gctUINT              vsInstBufferAddrBase;
    gctUINT              psInstBufferAddrBase;
    gctUINT              vsConstRegAddrBase;
    gctUINT              tcsConstRegAddrBase;
    gctUINT              tesConstRegAddrBase;
    gctUINT              gsConstRegAddrBase;
    gctUINT              psConstRegAddrBase;
    gctUINT              vertexOutputBufferSize;
    gctUINT              vertexCacheSize;
    gctUINT              ctxStateCount;
}VSC_HW_CONFIG, *PVSC_HW_CONFIG;

typedef gcsGLSLCaps VSC_GL_API_CONFIG, *PVSC_GL_API_CONFIG;

/* VSC supported optional opts */
#define VSC_COMPILER_OPT_NONE                           0x0000000000000000ULL

#define VSC_COMPILER_OPT_ALGE_SIMP                      0x0000000000000001ULL
#define VSC_COMPILER_OPT_GCP                            0x0000000000000002ULL
#define VSC_COMPILER_OPT_LCP                            0x0000000000000004ULL
#define VSC_COMPILER_OPT_LCSE                           0x0000000000000008ULL
#define VSC_COMPILER_OPT_DCE                            0x0000000000000010ULL
#define VSC_COMPILER_OPT_PRE                            0x0000000000000020ULL
#define VSC_COMPILER_OPT_PEEPHOLE                       0x0000000000000040ULL
#define VSC_COMPILER_OPT_CONSTANT_PROPOGATION           0x0000000000000080ULL
#define VSC_COMPILER_OPT_CONSTANT_FOLDING               0x0000000000000100ULL
#define VSC_COMPILER_OPT_FUNC_INLINE                    0x0000000000000200ULL
#define VSC_COMPILER_OPT_INST_SKED                      0x0000000000000400ULL
#define VSC_COMPILER_OPT_GPR_SPILLABLE                  0x0000000000000800ULL
#define VSC_COMPILER_OPT_CONSTANT_REG_SPILLABLE         0x0000000000001000ULL
#define VSC_COMPILER_OPT_VEC                            0x0000000000002000ULL /* Including logic io packing */
#define VSC_COMPILER_OPT_IO_PACKING                     0x0000000000004000ULL /* Physical io packing */
#define VSC_COMPILER_OPT_FULL_ACTIVE_IO                 0x0000000000008000ULL
#define VSC_COMPILER_OPT_DUAL16                         0x0000000000010000ULL
#define VSC_COMPILER_OPT_ILF_LINK                       0x0000000000020000ULL

#define VSC_COMPILER_OPT_FULL                           0x000000000003FFFFULL

#define VSC_COMPILER_OPT_NO_ALGE_SIMP                   0x0000000100000000ULL
#define VSC_COMPILER_OPT_NO_GCP                         0x0000000200000000ULL
#define VSC_COMPILER_OPT_NO_LCP                         0x0000000400000000ULL
#define VSC_COMPILER_OPT_NO_LCSE                        0x0000000800000000ULL
#define VSC_COMPILER_OPT_NO_DCE                         0x0000001000000000ULL
#define VSC_COMPILER_OPT_NO_PRE                         0x0000002000000000ULL
#define VSC_COMPILER_OPT_NO_PEEPHOLE                    0x0000004000000000ULL
#define VSC_COMPILER_OPT_NO_CONSTANT_PROPOGATION        0x0000008000000000ULL
#define VSC_COMPILER_OPT_NO_CONSTANT_FOLDING            0x0000010000000000ULL
#define VSC_COMPILER_OPT_NO_FUNC_INLINE                 0x0000020000000000ULL
#define VSC_COMPILER_OPT_NO_INST_SKED                   0x0000040000000000ULL
#define VSC_COMPILER_OPT_NO_GPR_SPILLABLE               0x0000080000000000ULL
#define VSC_COMPILER_OPT_NO_CONSTANT_REG_SPILLABLE      0x0000100000000000ULL
#define VSC_COMPILER_OPT_NO_VEC                         0x0000200000000000ULL /* Including logic io packing */
#define VSC_COMPILER_OPT_NO_IO_PACKING                  0x0000400000000000ULL /* Physical io packing */
#define VSC_COMPILER_OPT_NO_FULL_ACTIVE_IO              0x0000800000000000ULL
#define VSC_COMPILER_OPT_NO_DUAL16                      0x0001000000000000ULL
#define VSC_COMPILER_OPT_NO_ILF_LINK                    0x0002000000000000ULL

#define VSC_COMPILER_OPT_NO_OPT                         0x0003FFFF00000000ULL

/* Compiler flag for special purpose */
#define VSC_COMPILER_FLAG_COMPILE_TO_HL                0x00000001   /* Compile IR to HL, including doing all opts in HL */
#define VSC_COMPILER_FLAG_COMPILE_TO_ML                0x00000002   /* Compile IR to ML, including doing all opts in HL&ML */
#define VSC_COMPILER_FLAG_COMPILE_TO_LL                0x00000004   /* Compile IR to LL, including doing all opts in HL&ML&LL */
#define VSC_COMPILER_FLAG_COMPILE_TO_MC                0x00000008   /* Compile IR to MC, including doing all opts in all levels */
#define VSC_COMPILER_FLAG_COMPILE_CODE_GEN             0x00000010
#define VSC_COMPILER_FLAG_SEPERATED_SHADERS            0x00000020
#define VSC_COMPILER_FLAG_UNI_UNIFORM_UNIFIED_ALLOC    0x00000040
#define VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC    0x00000080
#define VSC_COMPILER_FLAG_UNI_UNIFORM_FIXED_BASE_ALLOC 0x00000100
#define VSC_COMPILER_FLAG_UNI_SAMPLER_FIXED_BASE_ALLOC 0x00000200
#define VSC_COMPILER_FLAG_NEED_OOB_CHECK               0x00000400
#define VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO         0x00000800
#define VSC_COMPILER_FLAG_WAVIER_RESLAYOUT_COMPATIBLE  0x00001000   /* Vulkan only for resource layout is provided */
#define VSC_COMPILER_FLAG_NEED_RTNE_ROUNDING           0x00002000
#define VSC_COMPILER_FLAG_API_UNIFORM_PRECISION_CHECK  0x00004000
#define VSC_COMPILER_FLAG_RECOMPILER                   0x00008000

#define VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS          0x0000000F

#define VSC_MAX_GFX_SHADER_STAGE_COUNT                 5
#define VSC_MAX_CPT_SHADER_STAGE_COUNT                 1
#define VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT         VSC_MAX_GFX_SHADER_STAGE_COUNT
#define VSC_MAX_LINKABLE_SHADER_STAGE_COUNT            VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT
#define VSC_MAX_SHADER_STAGE_COUNT                     (VSC_MAX_GFX_SHADER_STAGE_COUNT + VSC_MAX_CPT_SHADER_STAGE_COUNT)

/* For indexing VSC_MAX_SHADER_STAGE_COUNT */
#define VSC_SHADER_STAGE_VS                            0
#define VSC_SHADER_STAGE_HS                            1
#define VSC_SHADER_STAGE_DS                            2
#define VSC_SHADER_STAGE_GS                            3
#define VSC_SHADER_STAGE_PS                            4
#define VSC_SHADER_STAGE_CS                            5

/* This means stage (type) of shader is not known, and can not be directly flushed down
   to HW. Normally, this shader might be a combination of several shader stage, or a
   portion of a shader stage, or functional lib that might be linked to another shader.
   So VSC_SHADER_STAGE_UNKNOWN is not counted by VSC_MAX_SHADER_STAGE_COUNT */
#define VSC_SHADER_STAGE_UNKNOWN                       0xFF

#define VSC_SHADER_STAGE_2_STAGE_BIT(shStage)          (1 << (shStage))

typedef enum _VSC_SHADER_STAGE_BIT
{
    VSC_SHADER_STAGE_BIT_VS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_VS),
    VSC_SHADER_STAGE_BIT_HS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_HS),
    VSC_SHADER_STAGE_BIT_DS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_DS),
    VSC_SHADER_STAGE_BIT_GS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_GS),
    VSC_SHADER_STAGE_BIT_PS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_PS),
    VSC_SHADER_STAGE_BIT_CS = VSC_SHADER_STAGE_2_STAGE_BIT(VSC_SHADER_STAGE_CS),
}
VSC_SHADER_STAGE_BIT;

/* For indexing VSC_MAX_GFX_SHADER_STAGE_COUNT, VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT
   and VSC_MAX_LINKABLE_SHADER_STAGE_COUNT */
#define VSC_GFX_SHADER_STAGE_VS                        VSC_SHADER_STAGE_VS
#define VSC_GFX_SHADER_STAGE_HS                        VSC_SHADER_STAGE_HS
#define VSC_GFX_SHADER_STAGE_DS                        VSC_SHADER_STAGE_DS
#define VSC_GFX_SHADER_STAGE_GS                        VSC_SHADER_STAGE_GS
#define VSC_GFX_SHADER_STAGE_PS                        VSC_SHADER_STAGE_PS
#define VSC_CPT_SHADER_STAGE_CS                        0

typedef void* SHADER_HANDLE;
typedef void* VSC_PRIV_DATA_HANDLE;

typedef enum _VSC_SHADER_RESOURCE_TYPE
{
    VSC_SHADER_RESOURCE_TYPE_SAMPLER                = 0, /* s#, sampler         */
    VSC_SHADER_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = 1, /* s#/t#, sampler2D       */
    VSC_SHADER_RESOURCE_TYPE_SAMPLED_IMAGE          = 2, /* t#, texture2D       */
    VSC_SHADER_RESOURCE_TYPE_STORAGE_IMAGE          = 3, /* u#, image2D         */
    VSC_SHADER_RESOURCE_TYPE_UNIFORM_TEXEL_BUFFER   = 4, /* t#, samplerBuffer   */
    VSC_SHADER_RESOURCE_TYPE_STORAGE_TEXEL_BUFFER   = 5, /* u#, imageBuffer     */
    VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER         = 6, /* cb#, uniform 'block' */
    VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER         = 7, /* u#, buffer 'block'  */
    VSC_SHADER_RESOURCE_TYPE_UNIFORM_BUFFER_DYNAMIC = 8, /* cb#, uniform 'block' */
    VSC_SHADER_RESOURCE_TYPE_STORAGE_BUFFER_DYNAMIC = 9, /* u#, buffer 'block'  */
    VSC_SHADER_RESOURCE_TYPE_INPUT_ATTACHMENT       = 10,/* t#, subpass         */
} VSC_SHADER_RESOURCE_TYPE;

typedef struct _VSC_SHADER_RESOURCE_BINDING
{
    VSC_SHADER_RESOURCE_TYPE          type;
    gctUINT32                         set;        /* Set-No in pResourceSets array */
    gctUINT32                         binding;
    gctUINT32                         arraySize;  /* 1 means not an array */
}VSC_SHADER_RESOURCE_BINDING;

typedef struct _VSC_PROGRAM_RESOURCE_BINDING
{
    VSC_SHADER_RESOURCE_BINDING       shResBinding;
    VSC_SHADER_STAGE_BIT              stageBits;  /* Which shader stages could access this resource */
}VSC_PROGRAM_RESOURCE_BINDING;

typedef struct _VSC_PROGRAM_RESOURCE_SET
{
    gctUINT32                         resourceBindingCount;
    VSC_PROGRAM_RESOURCE_BINDING*     pResouceBindings;
}VSC_PROGRAM_RESOURCE_SET;

typedef struct _VSC_SHADER_PUSH_CONSTANT_RANGE
{
    gctUINT32                         offset;
    gctUINT32                         size;
} VSC_SHADER_PUSH_CONSTANT_RANGE;

typedef struct _VSC_PROGRAM_PUSH_CONSTANT_RANGE
{
    VSC_SHADER_PUSH_CONSTANT_RANGE    shPushConstRange;
    VSC_SHADER_STAGE_BIT              stageBits; /* Which shader stages could access this push-constant */
} VSC_PROGRAM_PUSH_CONSTANT_RANGE;

/* Resource layout */
typedef struct _VSC_PROGRAM_RESOURCE_LAYOUT
{
    gctUINT32                         resourceSetCount; /* How many descritor set? */
    VSC_PROGRAM_RESOURCE_SET*         pResourceSets;

    gctUINT32                         pushConstantRangeCount;
    VSC_PROGRAM_PUSH_CONSTANT_RANGE*  pPushConstantRanges;
} VSC_PROGRAM_RESOURCE_LAYOUT;

typedef struct _VSC_SHADER_RESOURCE_LAYOUT
{
    gctUINT32                         resourceBindingCount;
    VSC_SHADER_RESOURCE_BINDING*      pResBindings;

    gctUINT32                         pushConstantRangeCount;
    VSC_SHADER_PUSH_CONSTANT_RANGE*   pPushConstantRanges;
}VSC_SHADER_RESOURCE_LAYOUT;

/* In general, a core system contex is maintained by driver's adapter/device who can
   designate a GPU chip, which means core system contex is GPU wide global context. */
typedef struct _VSC_CORE_SYS_CONTEXT
{
    /* Designates a target HW */
    VSC_HW_CONFIG                     hwCfg;

    /* VSC private data, maintained by vscCreatePrivateData and vscDestroyPrivateData */
    VSC_PRIV_DATA_HANDLE              hPrivData;
}VSC_CORE_SYS_CONTEXT, *PVSC_CORE_SYS_CONTEXT;

/* A system context is combination of core system contex and driver callbacks which can
   be driver context/device/... wide based on each driver's architecture */
typedef struct _VSC_SYS_CONTEXT
{
    PVSC_CORE_SYS_CONTEXT             pCoreSysCtx;

    /* Driver system callbacks. When VSC is fully decoupled with driver later,
       no need this anymore. Note hDrv designates driver where callbacks are
       called to and all callbacks' first param must be this hDrv */
    DRIVER_HANDLE                     hDrv;
    VSC_DRV_CALLBACKS                 drvCBs;
}VSC_SYS_CONTEXT, *PVSC_SYS_CONTEXT;

/* Lib-link inerface definitions */
#include "gc_vsc_drvi_lib_link.h"

/* Output profiles definitions */
#include "gc_vsc_drvi_shader_priv_mapping.h"
#include "gc_vsc_drvi_shader_profile.h"
#include "gc_vsc_drvi_program_profile.h"
#include "gc_vsc_drvi_kernel_profile.h"

/* A context designate an enviroment where a shader is going to be compiled */
typedef struct _VSC_CONTEXT
{
    /* Designate invoking client driver */
    gceAPI                            clientAPI;
    gcePATCH_ID                       appNameId;
    gctBOOL                           isPatchLib;

    /* System wide context */
    PVSC_SYS_CONTEXT                  pSysCtx;
}VSC_CONTEXT, *PVSC_CONTEXT;

/* VSC compiler configuration */
typedef struct _VSC_COMPILER_CONFIG
{
    VSC_CONTEXT                       ctx;

    /* Compiler controls */
    gctUINT                           cFlags;
    gctUINT64                         optFlags;
}VSC_COMPILER_CONFIG, *PVSC_COMPILER_CONFIG;

/* Shader compiler parameter for VSC compiler entrance. */
typedef struct _VSC_SHADER_COMPILER_PARAM
{
    VSC_COMPILER_CONFIG               cfg;

    SHADER_HANDLE                     hShader;
    VSC_SHADER_RESOURCE_LAYOUT*       pShResourceLayout; /* Vulkan ONLY */

    /* For static compilation, when it is set (as static lib), pInMasterSEP must
       be set to NULL and hShader should be the original shader.

       For recompilaton, when it is set (as dynamic lib), pInMasterSEP must NOT be
       set to NULL and hShader should be the combination of original shader and static
       lib passed in when static compilation */
    VSC_SHADER_LIB_LINK_TABLE*        pShLibLinkTable;

    SHADER_EXECUTABLE_PROFILE*        pInMasterSEP; /* For recompilation ONLY */
}VSC_SHADER_COMPILER_PARAM, *PVSC_SHADER_COMPILER_PARAM;

/* Program linker parameter for VSC linker entrance. */
typedef struct _VSC_PROGRAM_LINKER_PARAM
{
    VSC_COMPILER_CONFIG               cfg;
    PVSC_GL_API_CONFIG                pGlApiCfg;

    SHADER_HANDLE                     hShaderArray[VSC_MAX_SHADER_STAGE_COUNT];
    VSC_PROGRAM_RESOURCE_LAYOUT*      pPgResourceLayout; /* Vulkan ONLY */

    /* See comment of pShLibLinkTable in VSC_SHADER_COMPILER_PARAM */
    VSC_PROG_LIB_LINK_TABLE*          pProgLibLinkTable;

    PROGRAM_EXECUTABLE_PROFILE*       pInMasterPEP; /* For recompilation ONLY */
}VSC_PROGRAM_LINKER_PARAM, *PVSC_PROGRAM_LINKER_PARAM;

/* Kernel program linker parameter for VSC linker entrance. */
typedef struct VSC_KERNEL_PROGRAM_LINKER_PARAM
{
    VSC_COMPILER_CONFIG               cfg;

    SHADER_HANDLE*                    pKrnlModuleHandlesArray;
    gctUINT                           moduleCount;

    /* See comment of pShLibLinkTable in VSC_SHADER_COMPILER_PARAM */
    VSC_SHADER_LIB_LINK_TABLE*        pShLibLinkTable;

    KERNEL_EXECUTABLE_PROFILE**       ppInMasterKEPArray; /* For recompilation ONLY */
}VSC_KERNEL_PROGRAM_LINKER_PARAM, *PVSC_KERNEL_PROGRAM_LINKER_PARAM;

/* HW pipeline shaders linker parameter */
typedef struct _VSC_HW_PIPELINE_SHADERS_PARAM
{
    PVSC_SYS_CONTEXT                  pSysCtx;

    SHADER_EXECUTABLE_PROFILE*        pSEPArray[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];
}VSC_HW_PIPELINE_SHADERS_PARAM, *PVSC_HW_PIPELINE_SHADERS_PARAM;

/* Returned HW linker info which will be used for states programming */
typedef struct _VSC_HW_SHADERS_LINK_INFO
{
    SHADER_HW_INFO                    shHwInfoArray[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];
}VSC_HW_SHADERS_LINK_INFO, *PVSC_HW_SHADERS_LINK_INFO;

/* A state buffer that hold all shaders programming for GPU kickoff */
typedef struct _VSC_HW_PIPELINE_SHADERS_STATES
{
    gctUINT32                         stateBufferSize;
    void*                             pStateBuffer;

    /* It is DEPRECATED if driver directly uses EPs (PEP/KEP/SEP) as all
       the info stored in hints can be retrieved by SEP */
    struct _gcsHINT                   hints;
}VSC_HW_PIPELINE_SHADERS_STATES, *PVSC_HW_PIPELINE_SHADERS_STATES;

gceSTATUS vscInitializeHwPipelineShadersStates(VSC_SYS_CONTEXT* pSysCtx, VSC_HW_PIPELINE_SHADERS_STATES* pHwShdsStates);
gceSTATUS vscFinalizeHwPipelineShadersStates(VSC_SYS_CONTEXT* pSysCtx, VSC_HW_PIPELINE_SHADERS_STATES* pHwShdsStates);

/* VSC private data creater/destroyer, every client should call it at least at
   driver device granularity  */
gceSTATUS vscCreatePrivateData(VSC_CORE_SYS_CONTEXT* pCoreSysCtx, VSC_PRIV_DATA_HANDLE* phOutPrivData, gctBOOL bForOCL);
gceSTATUS vscDestroyPrivateData(VSC_CORE_SYS_CONTEXT* pCoreSysCtx, VSC_PRIV_DATA_HANDLE hPrivData);

/* Create a shader with content unfilled. In general, driver calls this
   function to create a shader, and then pass this shader handle to shader
   generator. Or alternatively, shader generator can directly call this
   function to create a shader in mem inside and return it to driver, then
   driver dont need call it. To all these two cases, driver must call
   vscDestroyShader when driver does not need it. */
gceSTATUS vscCreateShader(SHADER_HANDLE* pShaderHandle,
                          gctUINT shStage);
gceSTATUS vscDestroyShader(SHADER_HANDLE hShader);
gceSTATUS vscReferenceShader(SHADER_HANDLE hShader);

/* Print (dump) shader */
gceSTATUS vscPrintShader(SHADER_HANDLE hShader,
                         gctFILE hFile,
                         gctCONST_STRING strHeaderMsg,
                         gctBOOL bPrintHeaderFooter);

/* Shader binary saver and loader */

/* vscSaveShaderToBinary()
 * Two ways to save shader binary:
 * 1) compiler allocates memory and return the memory in *pBinary and size in pSizeInByte
 *    if (* pBinary) is NULL when calling the function
 * 2) user query the shader binary size with vscQueryShaderBinarySize() and allocate
 *    memory in (*pBinary), size in pSizeInByte
 */
gceSTATUS vscSaveShaderToBinary(SHADER_HANDLE hShader,
                                void**        pBinary,
                                gctUINT*      pSizeInByte);
/* vscLoadShaderFromBinary()
 * restore shader from pBinary which should be serialized by vscSaveShaderToBinary()
 * if bFreeBinary is true, pBinary is deallocated by this function
 */
gceSTATUS vscLoadShaderFromBinary(void*          pBinary,
                                  gctUINT        sizeInByte,
                                  SHADER_HANDLE* pShaderHandle,
                                  gctBOOL        bFreeBinary);

gceSTATUS vscQueryShaderBinarySize(SHADER_HANDLE hShader, gctUINT* pSizeInByte);

/* Shader copy */
gceSTATUS vscCopyShader(SHADER_HANDLE * hToShader, SHADER_HANDLE hFromShader);

/* It will extract a specific sub-shader from main shader. It will be mainly used by CL to pick specific kernel
   from kernel program. */
gceSTATUS vscExtractSubShader(SHADER_HANDLE   hMainShader,
                              gctCONST_STRING pSubShaderEntryName,
                              SHADER_HANDLE   hSubShader);

/* Link a lib shader to main shader. */
gceSTATUS vscLinkLibShaderToShader(SHADER_HANDLE              hMainShader,
                                   VSC_SHADER_LIB_LINK_TABLE* pShLibLinkTable);

/* DX/CL driver will call this directly, GL will call this directly after FE
   or inside of vscLinkProgram. After successfully shader compilation, a SEP
   may be generated based on cFlags */
gceSTATUS vscCompileShader(VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                           SHADER_EXECUTABLE_PROFILE* pOutSEP /* Caller should destory the mem,
                                                                 so dont use VSC MM to allocate
                                                                 inside of VSC. */
                          );

gcSHADER_KIND vscGetShaderKindFromShaderHandle(SHADER_HANDLE hShader);

gceSTATUS vscGetTemporaryDir(OUT gctSTRING gcTmpDir);

/* GL/Vulkan driver ONLY interface, this is HL interface to match glLinkProgram
   API. It may call vscCompileShader for each shader inside. After successfully
   program linking, a PEP will be generated. Also program states will auto be
   generated at this time if driver request. If not, driver may want program
   states by itself.  */
gceSTATUS vscLinkProgram(VSC_PROGRAM_LINKER_PARAM*       pPgLinkParam,
                         PROGRAM_EXECUTABLE_PROFILE*     pOutPEP, /* Caller should destory the mem, */
                         VSC_HW_PIPELINE_SHADERS_STATES* pOutPgStates /* so for these two, dont use VSC
                                                                         MM to allocate inside of VSC. */
                        );

/* CL driver ONLY interface, this is HL interface to match clCreateKernel API
   if clBuildProgram does not directly generate MC, or to be called inside of
   vscLinkKernelProgram matched with clLinkProgram/clBuildProgram if it want
   to directly generate MC. It will call vscCompileShader inside. After succ-
   essfully creating, a KEP will be generated. Also kernel states will auto be
   generated at this time if driver request. If not, driver may want program
   states by itself. */
gceSTATUS vscCreateKernel(VSC_SHADER_COMPILER_PARAM*      pCompilerParam,
                          KERNEL_EXECUTABLE_PROFILE*      pOutKEP, /* Caller should destory the mem, */
                          VSC_HW_PIPELINE_SHADERS_STATES* pOutKrnlStates /* so for these two, dont use VSC
                                                                            MM to allocate inside of VSC. */
                         );

/* CL driver ONLY interface, this is HL interface to match clLinkProgram or
   clBuildProgram API. It will call vscCreateKernel inside. After successfully
   creating, a KEP list will be generated to match each kernel. Also kernel
   states array matched each kernel will auto be generated at this time
   if driver request. If not, driver may want program states by itself. */
gceSTATUS vscLinkKernelProgram(VSC_KERNEL_PROGRAM_LINKER_PARAM*  pKrnlPgLinkParam,
                               gctUINT*                          pKernelCount,
                               SHADER_HANDLE                     hOutLinkedProgram,
                               KERNEL_EXECUTABLE_PROFILE**       ppOutKEPArray, /* Caller should destory the mem, */
                               VSC_HW_PIPELINE_SHADERS_STATES**  ppOutKrnlStates      /* so for these two, dont use VSC
                                                                                         MM to allocate inside of VSC. */
                              );

/* This is hw level link that every client should call before programming */
gceSTATUS vscLinkHwShaders(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                           VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo,
                           gctBOOL                         bSeperatedShaders);

/* All clients can call this function when drawing/dispatching/kickoffing to program
   shaders stages that current pipeline uses to launch GPU task in cores. For example,
   driver may send program-pipeline which holds all shaders that pipeline needs to ask
   for programming. After success, hw pipeline shaders states will be generated. If
   driver want do program by itself, then it wont be called. */
gceSTATUS vscProgramHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM*   pHwPipelineShsParam,
                                   VSC_HW_PIPELINE_SHADERS_STATES*  pOutHwShdsStates, /* Caller should destory the mem,
                                                                                           so dont use VSC MM to allocate
                                                                                           inside of VSC. */
                                   gctBOOL                          bSeperatedShaders   /* It is a temporary fix,
                                                                                           we need to remove it later. */
                                  );



/****************************************************************************
   Following are for future MC level recompiling. Right now, we are using HL
   recompiling, so these are not used now.
*****************************************************************************/

/* VSC recompiler configuration */
typedef struct _VSC_RECOMPILER_CONFIG
{
    VSC_CONTEXT                       ctx;

    /* Recompiler controls */
    gctUINT                           rcFlags;
}VSC_RECOMPILER_CONFIG, *PVSC_RECOMPILER_CONFIG;

/* Shader recompiler parameter for VSC recompiler entrance. */
typedef struct _VSC_SHADER_RECOMPILER_PARAM
{
    VSC_RECOMPILER_CONFIG             cfg;
}VSC_SHADER_RECOMPILER_PARAM, *PVSC_SHADER_RECOMPILER_PARAM;

gceSTATUS
gcSHADER_WriteBufferToFile(
    IN gctSTRING buffer,
    IN gctUINT32 bufferSize,
    IN gctSTRING ShaderName
    );

gceSTATUS
gcSHADER_ReadBufferFromFile(
    IN gctSTRING    ShaderName,
    OUT gctSTRING    *buf,
    OUT gctUINT *bufSize
    );

gceSTATUS vscGetTemporaryDir(OUT gctSTRING gcTmpDir);

END_EXTERN_C();

#endif /*__gc_vsc_drvi_interface_h_ */


