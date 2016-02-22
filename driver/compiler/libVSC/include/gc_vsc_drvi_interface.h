/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

#define gcdSUPPORT_COMPUTE_SHADER   1
#define gcdSUPPORT_TESS_GS_SHADER   1
#define gcdSUPPORT_OCL_1_2          1
#define ENABLE_FULL_NEW_LINKER      (gcmOPT_UseVIRCodeGen() == VIRCG_FULL)
#define TREAT_ES20_INTEGER_AS_FLOAT 0

BEGIN_EXTERN_C()

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
        gctUINT          noUniformA0            : 1;
        gctUINT          supportOOBCheck        : 1;
        gctUINT          hasUniversalTexld      : 1;
        gctUINT          hasUniversalTexldV2    : 1;
        gctUINT          canSrc0OfImgLdStBeTemp : 1;

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

        gctUINT          reserved               : 6;
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
    gctUINT              maxUSCSizeInKbyte;
    gctUINT              maxLocalMemSizeInByte;
    gctUINT              maxResultCacheWinSize;

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
#define VSC_COMPILER_OPT_ALGE_SIMP                     0x0000000000000001
#define VSC_COMPILER_OPT_GCP                           0x0000000000000002
#define VSC_COMPILER_OPT_LCP                           0x0000000000000004
#define VSC_COMPILER_OPT_LCSE                          0x0000000000000008
#define VSC_COMPILER_OPT_DCE                           0x0000000000000010
#define VSC_COMPILER_OPT_PRE                           0x0000000000000020
#define VSC_COMPILER_OPT_PEEPHOLE                      0x0000000000000040
#define VSC_COMPILER_OPT_CONSTANT_PROPOGATION          0x0000000000000080
#define VSC_COMPILER_OPT_CONSTANT_FOLDING              0x0000000000000100
#define VSC_COMPILER_OPT_FUNC_INLINE                   0x0000000000000200
#define VSC_COMPILER_OPT_INST_SKED                     0x0000000000000400
#define VSC_COMPILER_OPT_GPR_SPILLABLE                 0x0000000000000800
#define VSC_COMPILER_OPT_CONSTANT_REG_SPILLABLE        0x0000000000001000
#define VSC_COMPILER_OPT_VECTORIZATION                 0x0000000000002000 /* Including logic io packing */
#define VSC_COMPILER_OPT_IO_PACKING                    0x0000000000004000 /* Physical io packing */
#define VSC_COMPILER_OPT_FULL_ACTIVE_IO                0x0000000000008000
#define VSC_COMPILER_OPT_DUAL16                        0x0000000000010000

#define VSC_COMPILER_OPT_FULL                          0x0000000000013FFF

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
#define VSC_COMPILER_FLAG_RECOMPILING                  0x00000400
#define VSC_COMPILER_FLAG_NEED_OOB_CHECK               0x00000800
#define VSC_COMPILER_FLAG_FLUSH_DENORM_TO_ZERO         0x00001000

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

/* For indexing VSC_MAX_GFX_SHADER_STAGE_COUNT, VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT
   and VSC_MAX_LINKABLE_SHADER_STAGE_COUNT */
#define VSC_GFX_SHADER_STAGE_VS                        VSC_SHADER_STAGE_VS
#define VSC_GFX_SHADER_STAGE_HS                        VSC_SHADER_STAGE_HS
#define VSC_GFX_SHADER_STAGE_DS                        VSC_SHADER_STAGE_DS
#define VSC_GFX_SHADER_STAGE_GS                        VSC_SHADER_STAGE_GS
#define VSC_GFX_SHADER_STAGE_PS                        VSC_SHADER_STAGE_PS
#define VSC_CPT_SHADER_STAGE_CS                        0

#include "drvi/gc_vsc_drvi_shader_profile.h"
#include "drvi/gc_vsc_drvi_program_profile.h"
#include "drvi/gc_vsc_drvi_kernel_profile.h"

typedef struct _VIR_SHADER VIR_Shader;

/* VSC compiler configuration */
typedef struct _VSC_COMPILER_CONFIG
{
    gceAPI                     clientAPI;
    PVSC_HW_CONFIG             pHwCfg;
    gctUINT                    cFlags;
    gctUINT64                  optFlags;
}VSC_COMPILER_CONFIG, *PVSC_COMPILER_CONFIG;

/* Shader compiler parameter for VSC compiler entrance. */
typedef struct _VSC_SHADER_COMPILER_PARAM
{
    VSC_COMPILER_CONFIG        cfg;

    VIR_Shader*                pShader;
}VSC_SHADER_COMPILER_PARAM, *PVSC_SHADER_COMPILER_PARAM;

/* Program linker parameter for VSC linker entrance. */
typedef struct _VSC_PROGRAM_LINKER_PARAM
{
    VSC_COMPILER_CONFIG        cfg;
    PVSC_GL_API_CONFIG         pGlApiCfg;

    VIR_Shader*                pShaderArray[VSC_MAX_SHADER_STAGE_COUNT];
}VSC_PROGRAM_LINKER_PARAM, *PVSC_PROGRAM_LINKER_PARAM;

/* Kernel program linker parameter for VSC linker entrance. */
typedef struct VSC_KERNEL_PROGRAM_LINKER_PARAM
{
    VSC_COMPILER_CONFIG        cfg;

    VIR_Shader**               ppKrnlModulesArray;
    gctUINT                    moduleCount;
}VSC_KERNEL_PROGRAM_LINKER_PARAM, *PVSC_KERNEL_PROGRAM_LINKER_PARAM;

/* HW pipeline shaders linker parameter */
typedef struct _VSC_HW_PIPELINE_SHADERS_PARAM
{
    PVSC_HW_CONFIG             pHwCfg;

    SHADER_EXECUTABLE_PROFILE* pSEPArray[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];
}VSC_HW_PIPELINE_SHADERS_PARAM, *PVSC_HW_PIPELINE_SHADERS_PARAM;

/* Returned HW linker info which will be used for states programming */
typedef struct _VSC_HW_SHADERS_LINK_INFO
{
    SHADER_HW_INFO             shHwInfoArray[VSC_MAX_HW_PIPELINE_SHADER_STAGE_COUNT];
}VSC_HW_SHADERS_LINK_INFO, *PVSC_HW_SHADERS_LINK_INFO;

/* A state buffer that hold all shaders programming for GPU kickoff */
typedef struct _VSC_HW_PIPELINE_SHADERS_STATES
{
    gctUINT32                  stateBufferSize;
    void*                      pStateBuffer;
    struct _gcsHINT            hints;
}VSC_HW_PIPELINE_SHADERS_STATES, *PVSC_HW_PIPELINE_SHADERS_STATES;

/* DX/CL driver will call this directly, GL will call this directly after FE
   or inside of vscLinkProgram. After successfully shader compilation, a SEP
   may be generated based on cFlags */
gceSTATUS vscCompileShader(VSC_SHADER_COMPILER_PARAM* pCompilerParam,
                           SHADER_EXECUTABLE_PROFILE* pOutSEP /* Caller should destory the mem,
                                                                 so dont use VSC MM to allocate
                                                                 inside of VSC. */
                          );

/* GL driver ONLY interface, this is HL interface to match glLinkProgram API.
   It may call vscCompileShader for each shader inside. After successfully
   program linking, a PEP will be generated. Also program states will auto be
   generated at this time if driver request. If not, driver may want program
   states by itself.  */
gceSTATUS vscLinkProgram(VSC_PROGRAM_LINKER_PARAM*       pPgLinkParam,
                         PROGRAM_EXECUTABLE_PROFILE*     pOutPEP, /* Caller should destory the mem, */
                         VSC_HW_PIPELINE_SHADERS_STATES* pOutPgStates /* for these two, so dont use VSC
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
                          VSC_HW_PIPELINE_SHADERS_STATES* pOutKrnlStates /* for these two, so dont use VSC
                                                                            MM to allocate inside of VSC. */
                         );

/* CL driver ONLY interface, this is HL interface to match clLinkProgram or
   clBuildProgram API. It will call vscCreateKernel inside. After successfully
   creating, a KEP list will be generated to match each kernel. Also kernel
   states array matched each kernel will auto be generated at this time
   if driver request. If not, driver may want program states by itself. */
gceSTATUS vscLinkKernelProgram(VSC_KERNEL_PROGRAM_LINKER_PARAM*  pKrnlPgLinkParam,
                               gctUINT*                          pKernelCount,
                               VIR_Shader*                       pOutLinkedProgram,
                               KERNEL_EXECUTABLE_PROFILE**       ppOutKEPArray, /* Caller should destory the mem, */
                               VSC_HW_PIPELINE_SHADERS_STATES**  ppOutKrnlStates      /* for these two, so dont use VSC
                                                                                         MM to allocate inside of VSC. */
                              );

/* This is hw level link that every client should call before programming */
gceSTATUS vscLinkHwShaders(VSC_HW_PIPELINE_SHADERS_PARAM*  pHwPipelineShsParam,
                           VSC_HW_SHADERS_LINK_INFO*       pOutHwShdsLinkInfo);

/* All clients can call this function when drawing/dispatching/kickoffing to program
   shaders stages that current pipeline uses to launch GPU task in cores. For example,
   driver may send program-pipeline which holds all shaders that pipeline needs to ask
   for programming. After success, hw pipeline shaders states will be generated. If
   driver want do program by itself, then it wont be called. */
gceSTATUS vscProgramHwShaderStages(VSC_HW_PIPELINE_SHADERS_PARAM*   pHwPipelineShsParam,
                                   VSC_HW_PIPELINE_SHADERS_STATES*  pOutHwShdsStates  /* Caller should destory the mem,
                                                                                         so dont use VSC MM to allocate
                                                                                         inside of VSC. */
                                  );


/* CL driver ONLY interface. It will extract a specific kernel from kernel program */
gceSTATUS vscExtractKernel(VIR_Shader*     pKernelProgram,
                           gctCONST_STRING pKernelName,
                           VIR_Shader*     pOutKernel);

/****************************************************************************
   Following are for future MC level recompiling. Right now, we are using HL
   recompiling, so these are not used now.
*****************************************************************************/

/* VSC recompiler configuration */
typedef struct _VSC_RECOMPILER_CONFIG
{
    gceAPI                clientAPI;
    PVSC_HW_CONFIG        pHwCfg;
    gctUINT               rcFlags;
}VSC_RECOMPILER_CONFIG, *PVSC_RECOMPILER_CONFIG;

/* Shader recompiler parameter for VSC recompiler entrance. */
typedef struct _VSC_SHADER_RECOMPILER_PARAM
{
    VSC_RECOMPILER_CONFIG cfg;
}VSC_SHADER_RECOMPILER_PARAM, *PVSC_SHADER_RECOMPILER_PARAM;

END_EXTERN_C();

#endif /*__gc_vsc_drvi_interface_h_ */


