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


#include "gc_vsc.h"
#include "vir/linker/gc_vsc_vir_linker.h"
#include "vir/lower/gc_vsc_vir_hl_2_hl.h"

#define __LL_LIB_LENGTH__       (65535 * 3)
#define __LIB_NAME_LENGTH__     256
#define __LIB_SHADER_PMP     512

extern gctGLSLCompiler gcGLSLCompiler;
extern gctCLCompiler gcCLCompiler;
extern gctUINT vscHFUNC_Label(const char *);
extern gctBOOL vcsHKCMP_Label(const char *, const char *);

#define     CompileInstrisicLibfromSrc                1
#define     CompileInstrisicLibList                   0

#if CompileInstrisicLibfromSrc

/* library for gl built-in functions that are written in high level shader */
#include "lib/gc_vsc_lib_gl_builtin.h"

/* library for cl built-in functions that are written in high level shader */
#include "lib/gc_vsc_lib_cl_builtin.h"

#if CompileInstrisicLibList
static VSC_ErrCode
VIR_intrinsic_LibSource(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN VIR_Intrinsic_LibList    *pIntrinsicLibList,
    IN VIR_Intrinsic_LibSource  *pLibSource,
    IN gctBOOL                   ForOCL,
    IN gctBOOL                   DumpShader
    )
{
    VSC_ErrCode                  errCode = VSC_ERR_NONE;
    VIR_Shader                  *virIntrinsicLibrary = gcvNULL;
    gceSTATUS                    status = gcvSTATUS_OK;
    gcSHADER                     binary = gcvNULL;
    gctSTRING                    log = gcvNULL;
    gctSTRING                    source = gcvNULL;
    gctBOOL                      catString = gcvFALSE;
    gctUINT                      i, stringNum;

    gcmASSERT(pIntrinsicLibList);

    /* Allocate the string. */
    source = (gctSTRING) vscMM_Alloc(pMM, __LL_LIB_LENGTH__ * sizeof(char));

    /* Add header. */
    if (pLibSource->header)
    {
        gcoOS_StrCopySafe(source, gcoOS_StrLen(pLibSource->header, gcvNULL) + 1, pLibSource->header);
        catString = gcvTRUE;
    }

    /* Add extension. */
    if (pLibSource->extension)
    {
        if (catString)
        {
            gcoOS_StrCatSafe(source, __LL_LIB_LENGTH__, pLibSource->extension);
        }
        else
        {
            gcoOS_StrCopySafe(source, gcoOS_StrLen(pLibSource->extension, gcvNULL) + 1, pLibSource->extension);
            catString = gcvTRUE;
        }
    }

    /* Add source. */
    stringNum = sizeof(pLibSource->body) / sizeof(gctSTRING);
    for (i = 0; i < stringNum; i++)
    {
        if (catString)
        {
            gcoOS_StrCatSafe(source, __LL_LIB_LENGTH__, pLibSource->body[i]);
        }
        else
        {
            gcoOS_StrCopySafe(source, gcoOS_StrLen(pLibSource->body[i], gcvNULL) + 1, pLibSource->body[i]);
            catString = gcvTRUE;
        }
    }

    /* Compile the source. */
    if (ForOCL)
    {
        gcmONERROR((*gcCLCompiler)(gcvNULL,
                                   gcoOS_StrLen(source, gcvNULL),
                                   source,
                                   gcmOPT_oclPackedBasicType() ? "-cl-viv-vx-extension" : "",
                                   &binary,
                                   &log));
    }
    else
    {
        gcmONERROR((*gcGLSLCompiler)(gcSHADER_TYPE_LIBRARY,
                                     gcoOS_StrLen(source, gcvNULL),
                                     source,
                                     &binary,
                                     &log));
    }

    /* Create the VIR shader. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(VIR_Shader),
                              (gctPOINTER*)&virIntrinsicLibrary));
    gcmASSERT(virIntrinsicLibrary != gcvNULL);

    errCode = VIR_Shader_Construct(gcvNULL,
                                   VIR_SHADER_LIBRARY,
                                   virIntrinsicLibrary);
    ON_ERROR(errCode, "VIR_CompileIntrinsicLib");

    gcSHADER_Conv2VIR(binary, pHwCfg, virIntrinsicLibrary);

    if (DumpShader)
    {
        VIR_Shader_Dump(gcvNULL, "VIR library shader IR.", virIntrinsicLibrary, gcvTRUE);
    }

    /* Insert the shader into the lib list. */
    VIR_Intrinsic_LibList_AppendNode(pIntrinsicLibList,
                                     virIntrinsicLibrary,
                                     pLibSource->libKind);
OnError:
    if (source)
    {
        vscMM_Free(pMM, source);
    }

    if (binary)
    {
        gcSHADER_Destroy(binary);
    }

    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }

    return errCode;
}
#endif /* CompileInstrisicLibList */

VIR_Shader* virGLIntrinsicLibrary = gcvNULL;
VIR_Shader* virGLUseImgInstIntrinsicLibrary = gcvNULL;

/* this is for test only, compile the library from the FE and GCSL->VIR converter */
static VSC_ErrCode
_CreateIntrinsicLib(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN gctBOOL                   forGraphics,
    IN gctBOOL                   DumpShader,
    OUT VIR_Shader              **pOutLib
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gceSTATUS   status  = gcvSTATUS_OK;
    gctSTRING   sloBuiltinSource = gcvNULL;
    gcSHADER    Binary = gcvNULL;
    gctSIZE_T   length;
    gctINT      i, stringNum = 0;
    gctBOOL     locked = gcvFALSE;
    gctSTRING   log    = gcvNULL;
    gctBOOL     supportTexldU = pHwCfg->hwFeatureFlags.hasUniversalTexldV2 && pHwCfg->hwFeatureFlags.hasTexldUFix;
    gctBOOL     supportTexelFetchForMSAA = pHwCfg->hwFeatureFlags.supportMSAATexture;
    gctBOOL     supportTexMSAA2DArray = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_texture_storage_multisample_2d_array", gcvNULL);
    gctBOOL     supportMSShading = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_shader_multisample_interpolation", gcvNULL);
    gctBOOL     supportSamplerBuffer = gcoOS_StrStr(GetGLExtensionString(), "GL_EXT_texture_buffer", gcvNULL);
    gctBOOL     supportImgAddr = pHwCfg->hwFeatureFlags.supportImgAddr;
    gctBOOL     supportImgInst = supportImgAddr;
    VIR_Shader* virIntrinsicLibrary = gcvNULL;
    VIR_Shader** virIntrinsicLibraryPtr = gcvNULL;
    gctBOOL isReadVirLib = gcvFALSE;
    gctSTRING virLibName;
    const gctSTRING virImgInstLibName   = "viv_vir_intrinsic.lib";
    const gctSTRING virNoImgInstLibName = "viv_vir_noimg_intrinsic.lib" ;

    /* built-in function library */
    gctSTRING   BuiltinLib_common[] =
    {
        gcLibMODF_Func,
        gcLibCommon_Func,
        gcLibBitfieldExtract_Func,
        gcLibBitfieldInsert_Func,
        gcLibFREXP_Func,
        gcLibFREXPSTRUCT_Func,
        gcLibLDEXP_Func,
        gcLibPack_Func,
        gcLibUnpack_Func,
        gcLibQuantizeToF16_Funcs,
        gcLibUaddCarry_Func_VK_hati4,
        gcLibUsubBorrow_Func_VK,
        gcLibUmulExtended_Func_VK,
        gcLibImulExtended_Func_VK,
        gcLibMatrixTranspose_Func,
        gcLibNMin_Func,
        gcLibNMax_Func,
        gcLibNClamp_Func,
        gcLibTextureCommon_Func,
    };

    gctSTRING   BuiltinLib_Reflect[] =
    {
        gcLibREFLECT_Func_float,
        gcLibREFLECT_Func_vec2,
        gcLibREFLECT_Func_vec3,
        gcLibREFLECT_Func_vec4,
    };

    gctSTRING   BuiltinLib_Reflect_fmaSupported[] =
    {
        gcLibREFLECT_Func_float_fmaSupported,
        gcLibREFLECT_Func_vec2_fmaSupported,
        gcLibREFLECT_Func_vec3_fmaSupported,
        gcLibREFLECT_Func_vec4_fmaSupported,
    };

    gctSTRING ImageLib_common[] =
    {
        gcLibImageSwizzle,
        /* image size. */
        gcLibImageSize_halti4,
    };

    /* image query. */
    gctSTRING ImageQuery_halti4[] =
    {
        gcLibImageQuery_halti4,
    };

    /* image_load, image_store gc3000/5000 implementation */
    gctSTRING ImageLib[] =
    {
        /* gcLibImageAddr must be the first element. */
        gcLibImageAddr,
        gcLibImageAddr_intrinsic,
        gcLibImageStoreSwizzle,

        gcLibImageLoad_2D_int, /* 16i */
        gcLibImageLoad_2D_int_rgba32i,
        gcLibImageLoad_2D_int_rg32i,
        gcLibImageLoad_2D_int_r32i,
        gcLibImageLoad_2D_int_rg16i,
        gcLibImageLoad_2D_int_r16i,
        gcLibImageLoad_2D_int_rgba8i,
        gcLibImageLoad_2D_int_rg8i,
        gcLibImageLoad_2D_int_r8i,
        gcLibImageLoad_2D_uint, /* 16ui */
        gcLibImageLoad_2D_uint_rgba32ui,
        gcLibImageLoad_2D_uint_rgba8ui,
        gcLibImageLoad_2D_uint_rg8ui,
        gcLibImageLoad_2D_uint_r8ui,
        gcLibImageLoad_2D_uint_rg16ui,
        gcLibImageLoad_2D_uint_r16ui,
        gcLibImageLoad_2D_uint_rg32ui,
        gcLibImageLoad_2D_uint_r32ui,
        gcLibImageLoad_2D_float, /* 16f */
        gcLibImageLoad_2D_float_rgba8,
        gcLibImageLoad_2D_float_rgba8_snorm,
        gcLibImageLoad_2D_float_rg8,
        gcLibImageLoad_2D_float_r8,
        gcLibImageLoad_2D_float_rgba32f,
        gcLibImageLoad_2D_float_rg32f,
        gcLibImageLoad_2D_float_rg16f,
        gcLibImageLoad_2D_float_r16f,
        gcLibImageLoad_2D_float_r32f,
        gcLibImageLoad_2D_float_r5g6b5_unorm_pack16,
        gcLibImageLoad_2D_float_abgr8_unorm_pack32,
        gcLibImageLoad_2D_float_a2r10g10b10_unorm_pack32,
        gcLibImageLoad_2D_uint_abgr8ui_pack32,
        gcLibImageLoad_2D_int_abgr8i_pack32,

        gcLibImageLoad_1D_int, /* 16i */
        gcLibImageLoad_1D_int_rgba32i,
        gcLibImageLoad_1D_int_rgba8i,
        gcLibImageLoad_1D_int_r32i,
        gcLibImageLoad_1D_uint, /* 16ui */
        gcLibImageLoad_1D_uint_rgba32ui,
        gcLibImageLoad_1D_uint_rgba8ui,
        gcLibImageLoad_1D_uint_r32ui,
        gcLibImageLoad_1D_float, /* 16f */
        gcLibImageLoad_1D_float_rgba8,
        gcLibImageLoad_1D_float_rgba8_snorm,
        gcLibImageLoad_1D_float_rgba32f,
        gcLibImageLoad_1D_float_r32f,
        gcLibImageLoad_1D_float_r32i,
        gcLibImageLoad_1D_float_r32ui,

        gcLibImageLoad_Buffer_int, /* 16i */
        gcLibImageLoad_Buffer_int_rgba32i,
        gcLibImageLoad_Buffer_int_rgba8i,
        gcLibImageLoad_Buffer_int_r32i,
        gcLibImageLoad_Buffer_uint, /* 16ui */
        gcLibImageLoad_Buffer_uint_rgba32ui,
        gcLibImageLoad_Buffer_uint_rgba8ui,
        gcLibImageLoad_Buffer_uint_r32ui,
        gcLibImageLoad_Buffer_float, /* 16f */
        gcLibImageLoad_Buffer_float_rgba8,
        gcLibImageLoad_Buffer_float_rgba8_snorm,
        gcLibImageLoad_Buffer_float_rgba32f,
        gcLibImageLoad_Buffer_float_r32f,

        gcLibImageLoad_3Dcommon,
        gcLibImageLoad_3D,
        gcLibImageLoad_cube,
        gcLibImageLoad_1DArray,
        gcLibImageLoad_2DArray,
        gcLibImageLoad_CubeArray,

        gcLibImageStore_2D_float, /* 16f */
        gcLibImageStore_2D_float_rgba32f,
        gcLibImageStore_2D_float_rg32f,
        gcLibImageStore_2D_float_rg16f,
        gcLibImageStore_2D_float_r16f,
        gcLibImageStore_2D_float_r32f,
        gcLibImageStore_2D_float_r5g6b5_unorm_pack16,
        gcLibImageStore_2D_float_abgr8_unorm_pack32,
        gcLibImageStore_2D_float_a2r10g10b10_unorm_pack32,
        gcLibImageStore_2D_uint_abgr8ui_pack32,
        gcLibImageStore_2D_int_abgr8i_pack32,
        gcLibImageStore_2D_float_rgba8,
        gcLibImageStore_2D_float_rgba8_snorm,
        gcLibImageStore_2D_float_rg8,
        gcLibImageStore_2D_float_r8,
        gcLibImageStore_2D_int, /* 16i */
        gcLibImageStore_2D_int_rgba32i,
        gcLibImageStore_2D_int_rg16i,
        gcLibImageStore_2D_int_r16i,
        gcLibImageStore_2D_int_r32i,
        gcLibImageStore_2D_int_rg32i,
        gcLibImageStore_2D_int_rgba8i,
        gcLibImageStore_2D_int_rg8i,
        gcLibImageStore_2D_int_r8i,
        gcLibImageStore_2D_uint, /* 16ui */
        gcLibImageStore_2D_uint_rgba32ui,
        gcLibImageStore_2D_uint_rg32ui,
        gcLibImageStore_2D_uint_r32ui,
        gcLibImageStore_2D_uint_rgba8ui,
        gcLibImageStore_2D_uint_rg8ui,
        gcLibImageStore_2D_uint_r8ui,
        gcLibImageStore_2D_uint_rg16ui,
        gcLibImageStore_2D_uint_r16ui,

        gcLibImageStore_1D_float, /* 16f */
        gcLibImageStore_1D_float_rgba32f,
        gcLibImageStore_1D_float_r32f,
        gcLibImageStore_1D_float_r32i,
        gcLibImageStore_1D_float_r32ui,
        gcLibImageStore_1D_float_rgba8,
        gcLibImageStore_1D_float_rgba8_snorm,
        gcLibImageStore_1D_int, /* 16i */
        gcLibImageStore_1D_int_rgba32i,
        gcLibImageStore_1D_int_r32i,
        gcLibImageStore_1D_int_rgba8i,
        gcLibImageStore_1D_uint, /* 16ui */
        gcLibImageStore_1D_uint_rgba32ui,
        gcLibImageStore_1D_uint_r32ui,
        gcLibImageStore_1D_uint_rgba8ui,

        gcLibImageStore_Buffer_float, /* 16f */
        gcLibImageStore_Buffer_float_rgba32f,
        gcLibImageStore_Buffer_float_r32f,
        gcLibImageStore_Buffer_float_rgba8,
        gcLibImageStore_Buffer_float_rgba8_snorm,
        gcLibImageStore_Buffer_int, /* 16i */
        gcLibImageStore_Buffer_int_rgba32i,
        gcLibImageStore_Buffer_int_r32i,
        gcLibImageStore_Buffer_int_rgba8i,
        gcLibImageStore_Buffer_uint, /* 16ui */
        gcLibImageStore_Buffer_uint_rgba32ui,
        gcLibImageStore_Buffer_uint_r32ui,
        gcLibImageStore_Buffer_uint_rgba8ui,

        gcLibImageStore_3Dcommon,
        gcLibImageStore_3D,
        gcLibImageStore_cube,
        gcLibImageStore_1DArray,
        gcLibImageStore_2DArray,
        gcLibImageStore_CubeArray,
    };

    /* image load/store. */
    gctSTRING ImageLib_hati4[] =
    {
        gcLibImageAddr_halti4,
        gcLibImageAddr_intrinsic,
        gcLibImageLoad_1D_float_hati4,
        gcLibImageLoad_1D_float_1_hati4,
        gcLibImageLoad_1D_int_hati4,
        gcLibImageLoad_1D_int_1_hati4,
        gcLibImageLoad_1D_uint_hati4,
        gcLibImageLoad_1D_uint_1_hati4,

        gcLibImageLoad_1D_array_float_hati4,
        gcLibImageLoad_1D_array_float_1_hati4,
        gcLibImageLoad_1D_array_int_hati4,
        gcLibImageLoad_1D_array_int_1_hati4,
        gcLibImageLoad_1D_array_uint_hati4,
        gcLibImageLoad_1D_array_uint_1_hati4,

        gcLibImageLoad_2D_float_hati4,
        gcLibImageLoad_2D_float_1_hati4,
        gcLibImageLoad_2D_int_hati4,
        gcLibImageLoad_2D_int_1_hati4,
        gcLibImageLoad_2D_uint_hati4,
        gcLibImageLoad_2D_uint_1_hati4,

        gcLibImageLoad_2DArray_float_hati4,
        gcLibImageLoad_2DArray_float_1_hati4,
        gcLibImageLoad_2DArray_int_hati4,
        gcLibImageLoad_2DArray_int_1_hati4,
        gcLibImageLoad_2DArray_uint_hati4,
        gcLibImageLoad_2DArray_uint_1_hati4,

        gcLibImageLoad_3D_float_hati4,
        gcLibImageLoad_3D_float_1_hati4,
        gcLibImageLoad_3D_int_hati4,
        gcLibImageLoad_3D_int_1_hati4,
        gcLibImageLoad_3D_uint_hati4,
        gcLibImageLoad_3D_uint_1_hati4,

        gcLibImageLoad_cube_float_hati4,
        gcLibImageLoad_cube_float_1_hati4,
        gcLibImageLoad_cube_int_hati4,
        gcLibImageLoad_cube_int_1_hati4,
        gcLibImageLoad_cube_uint_hati4,
        gcLibImageLoad_cube_uint_1_hati4,

        gcLibImageLoad_CubeArray_float_img_access,
        gcLibImageLoad_CubeArray_float_1_img_access,
        gcLibImageLoad_CubeArray_int_img_access,
        gcLibImageLoad_CubeArray_int_1_img_access,
        gcLibImageLoad_CubeArray_uint_img_access,
        gcLibImageLoad_CubeArray_uint_1_img_access,

        gcLibImageLoad_Buffer_float_img_access,
        gcLibImageLoad_Buffer_int_img_access,
        gcLibImageLoad_Buffer_uint_img_access,

        gcLibImageStore_1D_float_hati4,
        gcLibImageStore_1D_float_1_hati4,
        gcLibImageStore_1D_int_hati4,
        gcLibImageStore_1D_int_1_hati4,
        gcLibImageStore_1D_uint_hati4,
        gcLibImageStore_1D_uint_1_hati4,

        gcLibImageStore_1D_array_float_hati4,
        gcLibImageStore_1D_array_float_1_hati4,
        gcLibImageStore_1D_array_int_hati4,
        gcLibImageStore_1D_array_int_1_hati4,
        gcLibImageStore_1D_array_uint_hati4,
        gcLibImageStore_1D_array_uint_1_hati4,

        gcLibImageStore_2D_float_hati4,
        gcLibImageStore_2D_float_1_hati4,
        gcLibImageStore_2D_int_hati4,
        gcLibImageStore_2D_int_1_hati4,
        gcLibImageStore_2D_uint_hati4,
        gcLibImageStore_2D_uint_1_hati4,

        gcLibImageStore_2DArray_float_hati4,
        gcLibImageStore_2DArray_float_1_hati4,
        gcLibImageStore_2DArray_int_hati4,
        gcLibImageStore_2DArray_int_1_hati4,
        gcLibImageStore_2DArray_uint_hati4,
        gcLibImageStore_2DArray_uint_1_hati4,

        gcLibImageStore_3D_float_hati4,
        gcLibImageStore_3D_float_1_hati4,
        gcLibImageStore_3D_int_hati4,
        gcLibImageStore_3D_int_1_hati4,
        gcLibImageStore_3D_uint_hati4,
        gcLibImageStore_3D_uint_1_hati4,

        gcLibImageStore_cube_float_hati4,
        gcLibImageStore_cube_float_1_hati4,
        gcLibImageStore_cube_int_hati4,
        gcLibImageStore_cube_int_1_hati4,
        gcLibImageStore_cube_uint_hati4,
        gcLibImageStore_cube_uint_1_hati4,

        gcLibImageStore_CubeArray_float_img_access,
        gcLibImageStore_CubeArray_float_1_img_access,
        gcLibImageStore_CubeArray_int_img_access,
        gcLibImageStore_CubeArray_int_1_img_access,
        gcLibImageStore_CubeArray_uint_img_access,
        gcLibImageStore_CubeArray_uint_1_img_access,

        gcLibImageStore_Buffer_float_img_access,
        gcLibImageStore_Buffer_int_img_access,
        gcLibImageStore_Buffer_uint_img_access,
    };

    /* texel fetch. */
    gctSTRING TexelFetchLib[] =
    {
        gcLibTexelFetch_Sampler2D,
        gcLibTexelFetch_Sampler2DArray,
        gcLibTexelFetch_Sampler3D,
    };

    gctSTRING TexelFetchLib_halti4[] =
    {
        gcLibTexelFetch_Sampler2D_halti4,
        gcLibTexelFetch_Sampler2DArray_halti4,
        gcLibTexelFetch_Sampler3D_halti4,
    };

    gctSTRING TexelFetchSamplerBuffer[] =
    {
        gcLibTexelFetch_SamplerBuffer,
    };

    gctSTRING TexelFetchSamplerBuffer_halti4[] =
    {
        gcLibTexelFetch_SamplerBuffer_halti4,
    };

    gctSTRING texelFetchMSLib_halti4[] =
    {
        gcLibTexelFetch_Sampler2DMS_halti4,
        gcLibTexelFetch_Sampler2DMSArray_halti4,
    };

    /* MS shading related built-in functions. */
    gctSTRING MSShadingLib[] =
    {
        gcLibInterpolateCommon,

        gcLibInterpolateAtCentroid_float,
        gcLibInterpolateAtCentroid_vec2,
        gcLibInterpolateAtCentroid_vec3,
        gcLibInterpolateAtCentroid_vec4,

        gcLibInterpolateAtSample_float,
        gcLibInterpolateAtSample_vec2,
        gcLibInterpolateAtSample_vec3,
        gcLibInterpolateAtSample_vec4,

        gcLibInterpolateAtOffset_float,
        gcLibInterpolateAtOffset_vec2,
        gcLibInterpolateAtOffset_vec3,
        gcLibInterpolateAtOffset_vec4,
    };

    /* texld-related. */
    gctSTRING TexLdLib_hati4[] =
    {
        /* texld for sampler1DArray */
        gcLibTexLd_sampler_1d_array,
        gcLibTexLd_sampler_1d_array_lod,
        gcLibTexLd_sampler_1d_array_bias,
        /* texld for sampler2DArray */
        gcLibTexLd_sampler_2d_array,
        gcLibTexLd_sampler_2d_array_lod,
        gcLibTexLd_sampler_2d_array_bias,

        gcLibGetLod,
    };

    /* For halti5 chips, if they don't have USC_GOS_ADDR_FIX feature, then they can't use IMG_LOAD/IMG_STORE for vs/ts/gs/ps. */
    if (supportImgInst &&
        pHwCfg->hwFeatureFlags.hasHalti5 &&
        !pHwCfg->hwFeatureFlags.hasUscGosAddrFix &&
        forGraphics)
    {
        supportImgInst = gcvFALSE;
    }

    if (supportImgAddr && !supportImgInst)
    {
        ImageLib[0] = gcLibImageAddr_halti4;
    }

    if (pHwCfg->hwFeatureFlags.hasHalti4)
    {
        BuiltinLib_common[2] = gcLibBitfieldExtract_Func_halti4;
        BuiltinLib_common[3] = gcLibBitfieldInsert_Func_halti4;
    }
    /* select intrinsic library and lib file name based on whether support ImgInst */
    if (supportImgInst)
    {
        virIntrinsicLibraryPtr = &virGLUseImgInstIntrinsicLibrary;
        virLibName = virImgInstLibName;
    }
    else
    {
        virIntrinsicLibraryPtr = &virGLIntrinsicLibrary;
        virLibName = virNoImgInstLibName;
    }

    gcmONERROR(gcLockLoadLibrary());
    locked = gcvTRUE;

    if (*virIntrinsicLibraryPtr != gcvNULL)
    {
        /* use cached intrinsic library */
        *pOutLib = *virIntrinsicLibraryPtr;
        locked = gcvFALSE;
        gcmVERIFY_OK(gcUnLockLoadLibrary());
        return VSC_ERR_NONE;
    }

    if (gcmOPT_LibShaderFile())
    {
        gcmONERROR(gcInitializeLibFile());

        status = gcSHADER_ReadVirLibFromFile(virLibName, (SHADER_HANDLE *)&virIntrinsicLibrary);

        if ((status != gcvSTATUS_OK) || (virIntrinsicLibrary == gcvNULL))
        {
        }
        else
        {
            isReadVirLib = gcvTRUE;
        }
    }

    /* create shader if it is empty or load from file failed */
    if ((isReadVirLib == gcvFALSE) &&((status != gcvSTATUS_OK) || (Binary == gcvNULL)))
    {
        sloBuiltinSource = (gctSTRING) vscMM_Alloc(pMM, __LL_LIB_LENGTH__ * sizeof(char));

        /* add the extension source */
        length = gcoOS_StrLen(gcLibFunc_Extension, gcvNULL);
        gcoOS_StrCopySafe(sloBuiltinSource, length + 1, gcLibFunc_Extension);

        /* add the extension source */
        if (supportTexMSAA2DArray)
        {
            gcoOS_StrCatSafe(sloBuiltinSource, __LL_LIB_LENGTH__, gcLibFunc_Extension_For_TexMS2DArray);
        }

        if (supportMSShading)
        {
            gcoOS_StrCatSafe(sloBuiltinSource, __LL_LIB_LENGTH__, gcLibFunc_Extension_For_MSShading);
        }

        /* add the header source */
        gcoOS_StrCatSafe(sloBuiltinSource, __LL_LIB_LENGTH__, gcLibFunc_TextureBufferSize_For_VK);
        gcoOS_StrCatSafe(sloBuiltinSource, __LL_LIB_LENGTH__, gcLibFunc_BuiltinHeader);

        stringNum = sizeof(BuiltinLib_common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __LL_LIB_LENGTH__, BuiltinLib_common[i]);
        }

        /* fma supported */
        if (pHwCfg->hwFeatureFlags.supportAdvancedInsts &&
            pHwCfg->hwFeatureFlags.hasHalti5)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibFMA_Func_fmaSupported);

            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibASIN_ACOS_Funcs_halti5_fmaSupported);

            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibATAN2_Funcs_halti5_fmaSupported);

            /* Use FMA to implement reflect. */
            stringNum = sizeof(BuiltinLib_Reflect_fmaSupported) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, BuiltinLib_Reflect_fmaSupported[i]);
            }
        }
        else
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibFMA_Func_fmaNotSupported);

            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibATAN_Funcs_halti2);

            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibASIN_ACOS_Funcs_halti2);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, gcLibATAN2_Funcs_halti2);

            /* Use normal MAD to implement reflect. */
            stringNum = sizeof(BuiltinLib_Reflect) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, BuiltinLib_Reflect[i]);
            }
        }

        stringNum = sizeof(ImageLib_common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __LL_LIB_LENGTH__, ImageLib_common[i]);
        }

        /* imageQuery. */
        stringNum = sizeof(ImageQuery_halti4) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __LL_LIB_LENGTH__, ImageQuery_halti4[i]);
        }

        /* imageLoad/imageStore. */
        if (supportImgInst)
        {
            stringNum = sizeof(ImageLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, ImageLib_hati4[i]);
            }
        }
        else
        {
            stringNum = sizeof(ImageLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, ImageLib[i]);
            }
        }

        /* texelFetch. */
        if (supportTexldU)
        {
            stringNum = sizeof(TexelFetchLib_halti4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, TexelFetchLib_halti4[i]);
            }

            if (supportSamplerBuffer)
            {
                stringNum = sizeof(TexelFetchSamplerBuffer_halti4) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __LL_LIB_LENGTH__, TexelFetchSamplerBuffer_halti4[i]);
                }
            }
        }
        else
        {
            stringNum = sizeof(TexelFetchLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, TexelFetchLib[i]);
            }

            if (supportSamplerBuffer)
            {
                stringNum = sizeof(TexelFetchSamplerBuffer) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __LL_LIB_LENGTH__, TexelFetchSamplerBuffer[i]);
                }
            }
        }

        /* texelFetch for MSAA. */
        if (supportTexMSAA2DArray)
        {
            if (supportTexelFetchForMSAA)
            {
                stringNum = sizeof(texelFetchMSLib_halti4) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __LL_LIB_LENGTH__, texelFetchMSLib_halti4[i]);
                }
            }
        }

        if (supportMSShading)
        {
            stringNum = sizeof(MSShadingLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __LL_LIB_LENGTH__, MSShadingLib[i]);
            }
        }

        /* texld. */
        stringNum = sizeof(TexLdLib_hati4) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __LL_LIB_LENGTH__, TexLdLib_hati4[i]);
        }

        (*gcGLSLCompiler)(gcSHADER_TYPE_LIBRARY,
                            gcoOS_StrLen(sloBuiltinSource, gcvNULL),
                            sloBuiltinSource,
                            &Binary,
                            &log);
    }

    if((isReadVirLib == gcvFALSE) && (Binary != gcvNULL))
    {
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    sizeof(VIR_Shader),
                                    (gctPOINTER*)&virIntrinsicLibrary));
        gcmASSERT(virIntrinsicLibrary != gcvNULL);

        errCode = VIR_Shader_Construct(gcvNULL,
                                       VIR_SHADER_LIBRARY,
                                       virIntrinsicLibrary);
        ON_ERROR(errCode, "VIR_CompileIntrinsicLib");

        gcSHADER_Conv2VIR(Binary, pHwCfg, virIntrinsicLibrary);
    }

    if (gcmOPT_LibShaderFile())
    {
        if((isReadVirLib == gcvFALSE) && (virIntrinsicLibrary != gcvNULL))
        {
             status = gcSHADER_WriteVirLibToFile(virLibName,virIntrinsicLibrary);

            if (status != gcvSTATUS_OK)
            {
                if (DumpShader)
                        gcoOS_Print("gcSHADER_WriteVirLibToFile Error:%d\n", status);
                    status = gcvSTATUS_OK;
            }
        }

        gcmONERROR(gcFinalizeLibFile());
    }

#if _DEBUG_VIR_IO_COPY
    {
        VIR_Shader *copiedShader;

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    sizeof(VIR_Shader),
                                    (gctPOINTER*)&copiedShader));

        gcmASSERT(copiedShader != gcvNULL);

        VIR_Shader_Copy(copiedShader, virIntrinsicLibrary);
        VIR_Shader_Destroy(virIntrinsicLibrary);
        gcoOS_Free(gcvNULL, virIntrinsicLibrary);

        if (DumpShader)
        {
            VIR_Shader_Dump(gcvNULL, "Converted and Copied VIR library Shader", copiedShader, gcvTRUE);
        }
        {
            VIR_Shader_IOBuffer buf;
            VIR_Shader * readShader;

            VIR_Shader_Save(copiedShader, &buf);

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                        sizeof(VIR_Shader),
                                        (gctPOINTER*)&readShader));

            errCode = VIR_Shader_Construct(gcvNULL,
                                            VIR_Shader_GetKind(copiedShader),
                                            readShader);
            buf.shader = readShader;
            buf.curPos = 0;

            VIR_Shader_Read(readShader, &buf);

            VIR_Shader_Destroy(copiedShader);
            gcoOS_Free(gcvNULL, copiedShader);
            VIR_IO_Finalize(&buf);

            virIntrinsicLibrary = readShader;
        }
    }
#endif

    if (DumpShader)
    {
        VIR_Shader_Dump(gcvNULL, "VIR library shader IR.", virIntrinsicLibrary, gcvTRUE);
    }

    *virIntrinsicLibraryPtr = virIntrinsicLibrary;
    *pOutLib = virIntrinsicLibrary;

OnError:
    if (sloBuiltinSource)
    {
        vscMM_Free(pMM, sloBuiltinSource);
    }

    if (Binary)
    {
        gcSHADER_Destroy(Binary);
        Binary = gcvNULL;
    }

    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }

    if (locked)
    {
        gcmVERIFY_OK(gcUnLockLoadLibrary());
    }

    if (status  != gcvSTATUS_OK)
    {
        errCode = vscERR_CastGcStatus2ErrCode(status);
    }
    return errCode;
}

/* this is for test only, compile the library from the OCL FE and GCSL->VIR converter */
static VSC_ErrCode
_CreateCLIntrinsicLib(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN gctBOOL                   DumpShader,
    OUT VIR_Shader              **pOutLib
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gceSTATUS   status  = gcvSTATUS_OK;
    gctSTRING   builtinSource = gcvNULL;
    gcSHADER    Binary = gcvNULL;
    gctSIZE_T   length;
    gctINT      i, stringNum = 0;
    gctBOOL     locked = gcvFALSE;
    gctBOOL     needAtomicPatch = (pHwCfg->hwFeatureFlags.supportUSC) && (!pHwCfg->hwFeatureFlags.hasUSCAtomicFix2);
    gctSTRING   log    = gcvNULL;
    VIR_Shader* virCLIntrinsicLibrary = gcvNULL;
    VIR_Shader** virCLIntrinsicLibraryPtr = gcvNULL;
    gctBOOL     isReadVirLib = gcvFALSE;
    gctSTRING   virLibName = "viv_vir_cl_intrinsic_LS-v624.lib";

    /* built-in function library */
    gctSTRING   builtinLib_common_packed[] =
    {
        gcCLLibRelational_Funcs_packed,
    };

    virCLIntrinsicLibraryPtr = &virCLIntrinsicLibrary;
    gcmONERROR(gcLockLoadLibrary());
    locked = gcvTRUE;

    if (*virCLIntrinsicLibraryPtr != gcvNULL)
    {
        *pOutLib = *virCLIntrinsicLibraryPtr;
        locked = gcvFALSE;
        gcmVERIFY_OK(gcUnLockLoadLibrary());
        return VSC_ERR_NONE;
    }

    if (gcmOPT_LibShaderFile())
    {

        gcmONERROR(gcInitializeLibFile());

        status = gcSHADER_ReadVirLibFromFile(virLibName, (SHADER_HANDLE *)&virCLIntrinsicLibrary);

        if ((status != gcvSTATUS_OK) || (virCLIntrinsicLibrary == gcvNULL))
        {
        }
        else
        {
            isReadVirLib = gcvTRUE;
        }
    }

    /* create shader if it is empty or load from file failed */
    if ((isReadVirLib == gcvFALSE) &&((status != gcvSTATUS_OK) || (Binary == gcvNULL)))
    {
        builtinSource = (gctSTRING) vscMM_Alloc(pMM, __LL_LIB_LENGTH__ * sizeof(char));

        /* add the header source */
        length = gcoOS_StrLen(gcCLLibHeader, gcvNULL);
        gcoOS_StrCopySafe(builtinSource, length + 1, gcCLLibHeader);

        /* add the extension source */
        gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcCLLibFunc_Extension);

        if(gcmOPT_oclPackedBasicType())
        {
            stringNum = sizeof(builtinLib_common_packed) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(builtinSource,
                                 __LL_LIB_LENGTH__, builtinLib_common_packed[i]);
            }
        }

        /* add atomic patch according to pHwCfg */
        if (needAtomicPatch)
        {
            gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcCLLibGetLocalID);
            if (pHwCfg->maxCoreCount == 1)
            {
                gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcLib_AtomicPatch_Common_Func_core1_Str);
            }
            else if (pHwCfg->maxCoreCount == 2)
            {
                gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcLib_AtomicPatch_Common_Func_core2_Str);
            }
            else if (pHwCfg->maxCoreCount == 4)
            {
                gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcLib_AtomicPatch_Common_Func_core4_Str);
            }
            else if (pHwCfg->maxCoreCount == 8)
            {
                gcoOS_StrCatSafe(builtinSource, __LL_LIB_LENGTH__, gcLib_AtomicPatch_Common_Func_core8_Str);
            }
        }

        gcmONERROR((*gcCLCompiler)(gcvNULL,
                                    gcoOS_StrLen(builtinSource, gcvNULL),
                                    builtinSource,
                                    gcmOPT_oclPackedBasicType() ? "-cl-viv-vx-extension" : "",
                                    &Binary,
                                    &log));
    }
        /* convert the gcSL lib shader to VIR shader */
    if((isReadVirLib == gcvFALSE) && (Binary != gcvNULL))
    {
        /* do not dump verbose info when convert lib to vir shader, it is too big */
        gcOPTIMIZER_OPTION * option = gcmGetOptimizerOption();
        gctBOOL savedDumpBEVerbose = option->dumpBEVerbose;

        option->dumpBEVerbose = gcvFALSE;

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    sizeof(VIR_Shader),
                                    (gctPOINTER*)&virCLIntrinsicLibrary));
        gcmASSERT(virCLIntrinsicLibrary != gcvNULL);

        errCode = VIR_Shader_Construct(gcvNULL,
                                        VIR_SHADER_LIBRARY,
                                        virCLIntrinsicLibrary);
        ON_ERROR(errCode, "VIR_CompileCLIntrinsicLib");

        gcSHADER_Conv2VIR(Binary, pHwCfg, virCLIntrinsicLibrary);
        /* restore saved option */
        option->dumpBEVerbose = savedDumpBEVerbose;
    }

    if (gcmOPT_LibShaderFile())
    {
        /* save just created lib shader to file system */
        if((isReadVirLib == gcvFALSE) && (virCLIntrinsicLibrary != gcvNULL))
        {
             status = gcSHADER_WriteVirLibToFile(virLibName,virCLIntrinsicLibrary);

            if (status != gcvSTATUS_OK)
            {
                if (DumpShader)
                        gcoOS_Print("gcSHADER_WriteVirLibToFile Error:%d\n", status);
                    status = gcvSTATUS_OK;
            }
        }

        gcmONERROR(gcFinalizeLibFile());
    }

#if _DEBUG_VIR_IO_COPY
    {
        VIR_Shader *copiedShader;

        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                    sizeof(VIR_Shader),
                                    (gctPOINTER*)&copiedShader));

        gcmASSERT(copiedShader != gcvNULL);

        VIR_Shader_Copy(copiedShader, virCLIntrinsicLibrary);
        VIR_Shader_Destroy(virCLIntrinsicLibrary);
        gcoOS_Free(gcvNULL, virCLIntrinsicLibrary);

        if (DumpShader)
        {
            VIR_Shader_Dump(gcvNULL, "Converted and Copied VIR library Shader", copiedShader, gcvTRUE);
        }
        {
            VIR_Shader_IOBuffer buf;
            VIR_Shader * readShader;

            VIR_Shader_Save(copiedShader, &buf);

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                        sizeof(VIR_Shader),
                                        (gctPOINTER*)&readShader));

            errCode = VIR_Shader_Construct(gcvNULL,
                                            VIR_Shader_GetKind(copiedShader),
                                            readShader);
            buf.shader = readShader;
            buf.curPos = 0;

            VIR_Shader_Read(readShader, &buf);

            VIR_Shader_Destroy(copiedShader);
            gcoOS_Free(gcvNULL, copiedShader);
            VIR_IO_Finalize(&buf);

            virCLIntrinsicLibrary = readShader;
        }
    }
#endif

    if (DumpShader)
    {
        VIR_Shader_Dump(gcvNULL, "VIR library shader IR.", virCLIntrinsicLibrary, gcvTRUE);
    }

    *virCLIntrinsicLibraryPtr = virCLIntrinsicLibrary;
    *pOutLib = virCLIntrinsicLibrary;

OnError:
    if (builtinSource)
    {
        vscMM_Free(pMM, builtinSource);
    }

    if (Binary)
    {
        gcSHADER_Destroy(Binary);
        Binary = gcvNULL;
    }

    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }

    if (locked)
    {
        gcmVERIFY_OK(gcUnLockLoadLibrary());
    }

    if (status  != gcvSTATUS_OK)
    {
        errCode = vscERR_CastGcStatus2ErrCode(status);
    }
    return errCode;
}

#else
VSC_ErrCode
VIR_ReadIntrinsicLib(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    return errCode;
}
#endif  /* CompileInstrisicLibfromSrc */

VSC_ErrCode
VIR_GetIntrinsicLib(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN gctBOOL                  forOCL,
    IN gctBOOL                  forGraphics,
    IN gctBOOL                   DumpShader,
    IN gctBOOL                   hasExtcallAtomic,
    OUT VIR_Shader              **pOutLib
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;

#if CompileInstrisicLibList
    gctSTRING                   builtinLib_common_packed[] =
    {
        gcCLLibRelational_Funcs_packed,
    };
    VIR_Intrinsic_LibSource     libSource[] =
    {
        {
            VIR_INTRINSIC_LIB_CL,
            gcCLLibHeader,
            gcCLLibFunc_Extension,
            builtinLib_common_packed,
        }
    };
#else
#if CompileInstrisicLibfromSrc
    if (forOCL)
    {
        if(gcmOPT_oclPackedBasicType()||
           _OCL_USE_INTRINSIC_FOR_IMAGE || hasExtcallAtomic)
        {
            errCode = _CreateCLIntrinsicLib(pHwCfg, pMM, DumpShader, pOutLib);
        }
    }
    else
    {
        errCode = _CreateIntrinsicLib(pHwCfg, pMM, forGraphics, DumpShader, pOutLib);
    }

#else
#endif /* CompileInstrisicLibfromSrc */
#endif /* CompileInstrisicLibList */

    return errCode;
}

VSC_ErrCode
VIR_DestroyIntrinsicLib(
    IN VIR_Shader              *pLib
    )
{
    VIR_Shader_Destroy(pLib);
    gcoOS_Free(gcvNULL, pLib);

    return VSC_ERR_NONE;
}

/* queue for lib functions that are needed to be linked in */
void
VIR_LIB_WorkListQueue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_WORKLIST         *WorkList,
    IN VIR_Function             *Func
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(pMM,
        sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(worklistNode, Func);
    QUEUE_PUT_ENTRY(WorkList, worklistNode);
}

void
VIR_LIB_WorkListDequeue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_WORKLIST         *WorkList,
    OUT VIR_Function            **Func
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(WorkList);

    *Func = (VIR_Function *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(pMM, worklistNode);
}

/* queue for call instruction that are needed to be updated */
void
VIR_LIB_CallSitesQueue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_CALLSITES        *pCallSites,
    IN VIR_LINKER_CALL_INST_NODE*InstNode
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(pMM,
        sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(worklistNode, InstNode);
    QUEUE_PUT_ENTRY(pCallSites, worklistNode);
}

void
VIR_LIB_CallSitesDequeue(
    IN VSC_MM                   *pMM,
    IN VIR_LIB_CALLSITES        *pCallSites,
    OUT VIR_LINKER_CALL_INST_NODE**InstNode
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(pCallSites);

    *InstNode = (VIR_LINKER_CALL_INST_NODE *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(pMM, worklistNode);
}

/* convert the type, for the primitive type, they should be the same,
   for the non-primitive type, we should add the type into the master shader */
VIR_TypeId VIR_LinkLib_TypeConv(
    IN VIR_Shader   *pShader,
    IN VIR_Type     *inType,
    IN gctBOOL       ConvertSampler)
{
    VIR_TypeId inTyId = VIR_Type_GetIndex(inType);
    VIR_TypeId outTyId = VIR_TYPE_VOID;

    /* primitive type should be the same for both shader */
    if (VIR_TypeId_isPrimitive(inTyId))
    {
        if (ConvertSampler && VIR_TypeId_isSampler(inTyId))
        {
            outTyId = VIR_TYPE_UINT_X4;
        }
        else
        {
            outTyId = inTyId;
        }
    }
    else
    {
        switch (VIR_Type_GetKind(inType)) {
        case VIR_TY_ARRAY:
            VIR_Shader_AddArrayType(pShader,
                                    VIR_Type_GetBaseTypeId(inType),
                                    VIR_Type_GetArrayLength(inType),
                                    0,
                                    &outTyId);
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    return outTyId;
};

static VIR_TypeId
_ConvImageTypeId(
    VIR_TypeId       ImageTypeId,
    VIR_ImageFormat  ImageFormat
    )
{
    VIR_TypeId       fixedTypeId = ImageTypeId;
    VIR_TypeId       imageFormatTypeId = VIR_TYPE_VOID;

    if (VIR_TypeId_isImageSubPassData(fixedTypeId))
    {
        switch (fixedTypeId)
        {
        case VIR_TYPE_SUBPASSINPUT:
            fixedTypeId = VIR_TYPE_IMAGE_2D;
            break;

        case VIR_TYPE_SUBPASSINPUTMS:
            fixedTypeId = VIR_TYPE_IMAGE_2D_MSSA;
            break;

        case VIR_TYPE_ISUBPASSINPUT:
            fixedTypeId = VIR_TYPE_IIMAGE_2D;
            break;

        case VIR_TYPE_ISUBPASSINPUTMS:
            fixedTypeId = VIR_TYPE_IIMAGE_2D_MSSA;
            break;

        case VIR_TYPE_USUBPASSINPUT:
            fixedTypeId = VIR_TYPE_UIMAGE_2D;
            break;

        case VIR_TYPE_USUBPASSINPUTMS:
            fixedTypeId = VIR_TYPE_UIMAGE_2D_MSSA;
            break;

        default:
            fixedTypeId = VIR_TYPE_IMAGE_2D;
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    switch (ImageFormat)
    {
    /* Floating format. */
    case VIR_IMAGE_FORMAT_RGBA32F:
    case VIR_IMAGE_FORMAT_RG32F:
    case VIR_IMAGE_FORMAT_R32F:
    case VIR_IMAGE_FORMAT_RGBA16F:
    case VIR_IMAGE_FORMAT_RG16F:
    case VIR_IMAGE_FORMAT_R16F:
    case VIR_IMAGE_FORMAT_BGRA8_UNORM:
    case VIR_IMAGE_FORMAT_RGBA8:
    case VIR_IMAGE_FORMAT_RGBA8_SNORM:
    case VIR_IMAGE_FORMAT_RGBA8_UNORM:
    case VIR_IMAGE_FORMAT_RG8:
    case VIR_IMAGE_FORMAT_RG8_SNORM:
    case VIR_IMAGE_FORMAT_RG8_UNORM:
    case VIR_IMAGE_FORMAT_R8:
    case VIR_IMAGE_FORMAT_R8_SNORM:
    case VIR_IMAGE_FORMAT_R8_UNORM:
        imageFormatTypeId = VIR_TYPE_FLOAT32;
        break;

    /* Signed integer format. */
    case VIR_IMAGE_FORMAT_RGBA32I:
    case VIR_IMAGE_FORMAT_RG32I:
    case VIR_IMAGE_FORMAT_R32I:
    case VIR_IMAGE_FORMAT_RGBA16I:
    case VIR_IMAGE_FORMAT_RG16I:
    case VIR_IMAGE_FORMAT_R16I:
    case VIR_IMAGE_FORMAT_RGBA8I:
    case VIR_IMAGE_FORMAT_RG8I:
    case VIR_IMAGE_FORMAT_R8I:
        imageFormatTypeId = VIR_TYPE_INT32;
        break;

    /* Unsigned integer format. */
    case VIR_IMAGE_FORMAT_RGBA32UI:
    case VIR_IMAGE_FORMAT_RG32UI:
    case VIR_IMAGE_FORMAT_R32UI:
    case VIR_IMAGE_FORMAT_RGBA16UI:
    case VIR_IMAGE_FORMAT_RG16UI:
    case VIR_IMAGE_FORMAT_R16UI:
    case VIR_IMAGE_FORMAT_RGBA8UI:
    case VIR_IMAGE_FORMAT_RG8UI:
    case VIR_IMAGE_FORMAT_R8UI:
    case VIR_IMAGE_FORMAT_ABGR8UI_PACK32:
    case VIR_IMAGE_FORMAT_ABGR8I_PACK32:
    case VIR_IMAGE_FORMAT_A2B10G10R10UI_PACK32:
        imageFormatTypeId = VIR_TYPE_UINT32;
        break;

    default:
        break;
    }

    if (imageFormatTypeId == VIR_TYPE_INT32)
    {
        if (VIR_TypeId_isImageDataFloat(fixedTypeId))
        {
            switch (fixedTypeId)
            {
            /* 1D */
            case VIR_TYPE_IMAGE_1D:
                fixedTypeId = VIR_TYPE_IIMAGE_1D;
                break;
            case VIR_TYPE_IMAGE_1D_ARRAY:
                fixedTypeId = VIR_TYPE_IIMAGE_1D_ARRAY;
                break;
            /* 2D */
            case VIR_TYPE_IMAGE_2D:
                fixedTypeId = VIR_TYPE_IIMAGE_2D;
                break;
            case VIR_TYPE_IMAGE_2D_ARRAY:
                fixedTypeId = VIR_TYPE_IIMAGE_2D_ARRAY;
                break;
            /* 3D */
            case VIR_TYPE_IMAGE_3D:
                fixedTypeId = VIR_TYPE_IIMAGE_3D;
                break;
            /* CUBE */
            case VIR_TYPE_IMAGE_CUBE:
                fixedTypeId = VIR_TYPE_IIMAGE_CUBE;
                break;
            case VIR_TYPE_IMAGE_CUBE_DEPTH:
                fixedTypeId = VIR_TYPE_IIMAGE_CUBE_DEPTH;
                break;
            case VIR_TYPE_IMAGE_CUBE_ARRAY:
                fixedTypeId = VIR_TYPE_IIMAGE_CUBE_ARRAY;
                break;
            /* BUFFER */
            case VIR_TYPE_IMAGE_BUFFER:
                fixedTypeId = VIR_TYPE_IIMAGE_BUFFER;
                break;
            default:
                break;
            }
        }
    }
    else if (imageFormatTypeId == VIR_TYPE_UINT32)
    {
        if (VIR_TypeId_isImageDataFloat(fixedTypeId))
        {
            switch (fixedTypeId)
            {
            /* 1D */
            case VIR_TYPE_IMAGE_1D:
                fixedTypeId = VIR_TYPE_UIMAGE_1D;
                break;
            case VIR_TYPE_IMAGE_1D_ARRAY:
                fixedTypeId = VIR_TYPE_UIMAGE_1D_ARRAY;
                break;
            /* 2D */
            case VIR_TYPE_IMAGE_2D:
                fixedTypeId = VIR_TYPE_UIMAGE_2D;
                break;
            case VIR_TYPE_IMAGE_2D_ARRAY:
                fixedTypeId = VIR_TYPE_UIMAGE_2D_ARRAY;
                break;
            /* 3D */
            case VIR_TYPE_IMAGE_3D:
                fixedTypeId = VIR_TYPE_UIMAGE_3D;
                break;
            /* CUBE */
            case VIR_TYPE_IMAGE_CUBE:
                fixedTypeId = VIR_TYPE_UIMAGE_CUBE;
                break;
            case VIR_TYPE_IMAGE_CUBE_DEPTH:
                fixedTypeId = VIR_TYPE_UIMAGE_CUBE_DEPTH;
                break;
            case VIR_TYPE_IMAGE_CUBE_ARRAY:
                fixedTypeId = VIR_TYPE_UIMAGE_CUBE_ARRAY;
                break;
            /* BUFFER */
            case VIR_TYPE_IMAGE_BUFFER:
                fixedTypeId = VIR_TYPE_UIMAGE_BUFFER;
                break;
            default:
                break;
            }
        }
    }
    else if (imageFormatTypeId == VIR_TYPE_FLOAT32)
    {
    }

    return fixedTypeId;
}

static void
_IntrisicImageRelatedFuncName(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    VIR_Instruction *pInst,
    VIR_Symbol      *pImageSym,
    VIR_TypeId       TypeId,
    gctSTRING       *pLibName
    )
{
    VIR_TypeId       fixedTypeId = TypeId;
    VIR_TypeId       imageTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pImageSym));
    VIR_ImageFormat  imageFormat = VIR_Symbol_GetImageFormat(pImageSym);
    gctBOOL          useImgInst = gcvFALSE;

    /* Check if chip can support IMG_LOAD/IMG_STORE. */
    useImgInst = pHwCfg->hwFeatureFlags.supportImgAddr;

    /* For halti5 chips, if they don't have USC_GOS_ADDR_FIX feature, then they can't use IMG_LOAD/IMG_STORE for vs/ts/gs/ps. */
    if (useImgInst &&
        pHwCfg->hwFeatureFlags.hasHalti5 &&
        !pHwCfg->hwFeatureFlags.hasUscGosAddrFix &&
        VIR_Shader_IsGraphics(pShader))
    {
        useImgInst = gcvFALSE;
    }

    /* Convert the image type if needed. */
    if (!useImgInst || VIR_TypeId_isImageSubPassData(fixedTypeId))
    {
        fixedTypeId = _ConvImageTypeId(fixedTypeId, imageFormat);
    }

    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__,
        VIR_Shader_GetTypeNameString(pShader, VIR_Shader_GetTypeFromId(pShader, fixedTypeId)));

    if (useImgInst)
    {
        /* whether can support 128 bpp image. */
        if (!pHwCfg->hwFeatureFlags.support128BppImage)
        {
            if (VIR_Symbol_Is128Bpp(pImageSym) && !VIR_TypeId_isImageBuffer(imageTypeId))
            {
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_1");
            }
        }
    }
    else
    {
        switch (imageFormat)
        {
        /* 32bit. */
        case VIR_IMAGE_FORMAT_RGBA32F:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba32f");
            break;

        case VIR_IMAGE_FORMAT_RG32F:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg32f");
            break;

        case VIR_IMAGE_FORMAT_R32F:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r32f");
            break;

        case VIR_IMAGE_FORMAT_RGBA32I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba32i");
            break;

        case VIR_IMAGE_FORMAT_RG32I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg32i");
            break;

        case VIR_IMAGE_FORMAT_R32I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r32i");
            break;

        case VIR_IMAGE_FORMAT_RGBA32UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba32ui");
            break;

        case VIR_IMAGE_FORMAT_RG32UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg32ui");
            break;

        case VIR_IMAGE_FORMAT_R32UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r32ui");
            break;

        /* 16bit. */
        case VIR_IMAGE_FORMAT_RGBA16F:
            break;

        case VIR_IMAGE_FORMAT_RG16F:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg16f");
            break;

        case VIR_IMAGE_FORMAT_R16F:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r16f");
            break;

        case VIR_IMAGE_FORMAT_RGBA16I:
            break;

        case VIR_IMAGE_FORMAT_RG16I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg16i");
            break;

        case VIR_IMAGE_FORMAT_R16I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r16i");
            break;

        case VIR_IMAGE_FORMAT_RGBA16UI:
            break;

        case VIR_IMAGE_FORMAT_RG16UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg16ui");
            break;

        case VIR_IMAGE_FORMAT_R16UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r16ui");
            break;

        /* 8bit. */
        case VIR_IMAGE_FORMAT_BGRA8_UNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_bgra8");
            break;

        case VIR_IMAGE_FORMAT_RGBA8:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba8");
            break;

        case VIR_IMAGE_FORMAT_RGBA8_SNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba8_snorm");
            break;

        case VIR_IMAGE_FORMAT_RGBA8_UNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba8_unorm");
            break;

        case VIR_IMAGE_FORMAT_RG8:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg8");
            break;

        case VIR_IMAGE_FORMAT_RG8_SNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg8_snorm");
            break;

        case VIR_IMAGE_FORMAT_RG8_UNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg8_unorm");
            break;

        case VIR_IMAGE_FORMAT_R8:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r8");
            break;

        case VIR_IMAGE_FORMAT_R8_SNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r8_snorm");
            break;

        case VIR_IMAGE_FORMAT_R8_UNORM:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r8_unorm");
            break;

        case VIR_IMAGE_FORMAT_RGBA8I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba8i");
            break;

        case VIR_IMAGE_FORMAT_RG8I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg8i");
            break;

        case VIR_IMAGE_FORMAT_R8I:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r8i");
            break;

        case VIR_IMAGE_FORMAT_RGBA8UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rgba8ui");
            break;

        case VIR_IMAGE_FORMAT_RG8UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_rg8ui");
            break;

        case VIR_IMAGE_FORMAT_R8UI:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r8ui");
            break;

        /* pack bit. */
        case VIR_IMAGE_FORMAT_R5G6B5_UNORM_PACK16:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_r5g6b5_unorm_pack16");
            break;

        case VIR_IMAGE_FORMAT_ABGR8_UNORM_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_abgr8_unorm_pack32");
            break;

        case VIR_IMAGE_FORMAT_ABGR8I_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_abgr8i_pack32");
            break;

        case VIR_IMAGE_FORMAT_ABGR8UI_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_abgr8ui_pack32");
            break;

        case VIR_IMAGE_FORMAT_A2R10G10B10_UNORM_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_a2r10g10b10_unorm_pack32");
            break;

        case VIR_IMAGE_FORMAT_A2B10G10R10_UNORM_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_a2b10g10r10_unorm_pack32");
            break;

        case VIR_IMAGE_FORMAT_A2B10G10R10UI_PACK32:
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_a2b10g10r10ui_pack32");
            break;

        case VIR_IMAGE_FORMAT_NONE:
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
}

static void
_IntrisicTexldFuncName(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    VIR_Instruction *pInst,
    VIR_TypeId       typeId,
    gctSTRING       *pLibName
    )
{
    VIR_ParmPassing *parmOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pInst, 1));
    VIR_Operand     *pOpnd;
    VIR_Operand  *texldOperand;

    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__,
        VIR_Shader_GetTypeNameString(pShader, VIR_Shader_GetTypeFromId(pShader, typeId)));

    if (parmOpnd->argNum < 3)
    {
        return;
    }
    pOpnd = parmOpnd->args[2];

    /* Not optional bias, lod or gradient exist. */
    if (VIR_Operand_GetOpKind(pOpnd) != VIR_OPND_TEXLDPARM)
    {
        return;
    }

    texldOperand = (VIR_Operand*)pOpnd;

    /* Check bias. */
    if (VIR_Operand_GetTexldBias(texldOperand) != gcvNULL)
    {
        gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_bias");
    }
    /* Check lod. */
    if (VIR_Operand_GetTexldLod(texldOperand) != gcvNULL)
    {
        gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_lod");
    }
}

static void
_IntrisicImageAddrFuncName(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    VIR_Instruction *pInst,
    VIR_Symbol      *pImageSym,
    VIR_TypeId       typeId,
    gctSTRING       *pLibName
    )
{
    VIR_TypeId       imageTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pImageSym));

    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__,
        VIR_Shader_GetTypeNameString(pShader, VIR_Shader_GetTypeFromId(pShader, typeId)));

    if (VIR_TypeId_isImage3D(imageTypeId) ||
        VIR_TypeId_isImageCube(imageTypeId) ||
        VIR_TypeId_isImageArray(imageTypeId))
    {
        if (VIR_TypeId_isImage1D(imageTypeId))
        {
            gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, "_viv_image_addr_image_1d_array");
        }
        else if (VIR_TypeId_isImage2D(imageTypeId))
        {
            gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, "_viv_image_addr_image_2d_array");
        }
        else
        {
            gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, "_viv_image_addr_image_3d");
        }
    }
    else if (VIR_TypeId_isImage1D(imageTypeId))
    {
        gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, "_viv_image_addr_image_1d");
    }
    else
    {
        gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, "_viv_image_addr_image_2d");
    }
}

static void
_IntrisicImageFetchFuncName(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    VIR_Instruction *pInst,
    VIR_TypeId      imageTypeId,
    gctSTRING       *pLibName
    )
{
    VIR_ParmPassing *parmOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pInst, 1));
    VIR_Operand     *pOpnd;
    VIR_TypeId      typeId = imageTypeId;

    /* We treat sampler1D as a sampler2D. */
    if (VIR_TypeId_isSampler1D(typeId))
    {
        switch (typeId)
        {
        case VIR_TYPE_SAMPLER_1D:
            typeId = VIR_TYPE_SAMPLER_2D;
            break;
        case VIR_TYPE_SAMPLER_1D_ARRAY:
            typeId = VIR_TYPE_SAMPLER_2D_ARRAY;
            break;
        case VIR_TYPE_SAMPLER_1D_SHADOW:
            typeId = VIR_TYPE_SAMPLER_2D_SHADOW;
            break;
        case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
            typeId = VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW;
            break;
        case VIR_TYPE_ISAMPLER_1D:
            typeId = VIR_TYPE_ISAMPLER_2D;
            break;
        case VIR_TYPE_ISAMPLER_1D_ARRAY:
            typeId = VIR_TYPE_ISAMPLER_2D_ARRAY;
            break;
        case VIR_TYPE_USAMPLER_1D:
            typeId = VIR_TYPE_USAMPLER_2D;
            break;
        case VIR_TYPE_USAMPLER_1D_ARRAY:
            typeId = VIR_TYPE_USAMPLER_2D_ARRAY;
            break;

        default:
            gcmASSERT(gcvFALSE);
            typeId = VIR_TYPE_SAMPLER_2D;
            break;
        }

        gcmASSERT(VIR_TypeId_isSampler2D(typeId));
    }

    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__,
        VIR_Shader_GetTypeNameString(pShader, VIR_Shader_GetTypeFromId(pShader, typeId)));

    if (parmOpnd->argNum < 3)
    {
        return;
    }
    pOpnd = parmOpnd->args[2];

    /* Not optional bias, lod or gradient exist. */
    if (VIR_Operand_GetOpKind(pOpnd) != VIR_OPND_TEXLDPARM)
    {
        return;
    }

    /* Check offset. */
    if (VIR_Operand_GetTexldOffset(pOpnd) != gcvNULL)
    {
        gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_offset");
    }
}

static void
_IntrisicOrExtFuncName(
    VIR_Shader      *pShader,
    VSC_HW_CONFIG   *pHwCfg,
    VIR_Instruction *pInst,
    gctSTRING       *pLibName
    )
{
    VIR_Operand *src0Opnd = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand *src1Opnd = VIR_Inst_GetSource(pInst, 1);
    VIR_ParmPassing *parmOpnd;
    VIR_TypeId   opndTypeId;
    VIR_IntrinsicsKind intrinsicKind = VIR_Operand_GetIntrinsicKind(src0Opnd);
    VIR_Operand *imageOpnd = gcvNULL;
    VIR_Symbol *imageSym = gcvNULL;
    VIR_TypeId imageTypeId = VIR_INVALID_ID;

    /* handle EXTCALL first which already has the function name id in instruction */
    if (VIR_Inst_GetOpcode(pInst) == VIR_OP_EXTCALL)
    {
        VIR_NameId nameId;
        gcmASSERT(VIR_Operand_GetOpKind(src0Opnd) == VIR_OPND_NAME);
        nameId = VIR_Operand_GetNameId(src0Opnd);
        gcoOS_StrCopySafe(*pLibName, __LIB_NAME_LENGTH__, VIR_Shader_GetStringFromId(pShader, nameId));
        return;
    }

    /* Get parameters */
    parmOpnd = VIR_Operand_GetParameters(src1Opnd);

    /* If it is image-related function, get image symbol. */
    if (VIR_Intrinsics_isImageRelated(intrinsicKind) || VIR_Intrinsics_isImageAddr(intrinsicKind) || VIR_Intrinsics_isImageFetchForSampler(intrinsicKind))
    {
        imageOpnd = parmOpnd->args[0];
        imageSym = VIR_Operand_GetSymbol(imageOpnd);
        imageTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(imageSym));
    }

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_INTRINSIC);

    gcoOS_StrCopySafe(*pLibName, 6, "_viv_");
    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, VIR_Intrinsic_GetName(intrinsicKind));

    /* since transpose lib function's input is already lowered to vec
       its src0 could not distinguish the function */
    if (intrinsicKind == VIR_IK_transpose)
    {
        opndTypeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst));
    }
    else
    {
        opndTypeId = VIR_Operand_GetTypeId(parmOpnd->args[0]);
    }

    /* We don't need to get data type for image query. */
    if (VIR_Intrinsics_isImageQuery(intrinsicKind))
    {
        gctCHAR buffer[128];
        gctUINT offset = 0;

        if (VIR_Intrinsics_isImageQueryDimRelated(intrinsicKind))
        {
            if (VIR_TypeId_isImage1D(imageTypeId) || VIR_TypeId_isImageBuffer(imageTypeId) ||
                VIR_TypeId_isSampler1D(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_1d");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else if (VIR_TypeId_isImage2D(imageTypeId) ||
                     VIR_TypeId_isSampler2D(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_2d");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else if (VIR_TypeId_isImageCube(imageTypeId) ||
                     VIR_TypeId_isSamplerCube(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_cube");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else if (VIR_TypeId_isImage3D(imageTypeId) ||
                     VIR_TypeId_isSampler3D(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_3d");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else if (VIR_TypeId_isSamplerBuffer(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_samplerBuffer");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }

            /* Check if it is an array. */
            if (VIR_TypeId_isImageArray(imageTypeId) ||
                VIR_TypeId_isSamplerArray(imageTypeId))
            {
                offset = 0;
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_array");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
        }
        else if (VIR_Intrinsics_isImageQueryLod(intrinsicKind))
        {
            if (VIR_TypeId_isImage3D(imageTypeId) || VIR_TypeId_isSampler3D(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_3d");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else if (VIR_TypeId_isImageCube(imageTypeId) || VIR_TypeId_isSamplerCube(imageTypeId))
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_cube");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
            else
            {
                gcoOS_PrintStrSafe(buffer, 128, &offset, "_2d");
                gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, buffer);
            }
        }
        return;
    }

    gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__, "_");

    /* Check for imageLoad/imageStore. */
    if (VIR_Intrinsics_isImageLoad(intrinsicKind) || VIR_Intrinsics_isImageStore(intrinsicKind))
    {
        _IntrisicImageRelatedFuncName(pShader, pHwCfg, pInst, imageSym, opndTypeId, pLibName);
    }
    else
    {
        /* Check for texld related. */
        if (VIR_Intrinsics_isTexLdRelated(intrinsicKind))
        {
            _IntrisicTexldFuncName(pShader, pHwCfg, pInst, opndTypeId, pLibName);
        }
        /* Check for image addr. */
        else if (VIR_Intrinsics_isImageAddr(intrinsicKind))
        {
            _IntrisicImageAddrFuncName(pShader, pHwCfg, pInst, imageSym, opndTypeId, pLibName);
        }
        /* Check for image fetch. */
        else if (VIR_Intrinsics_isImageFetchForSampler(intrinsicKind))
        {
            _IntrisicImageFetchFuncName(pShader, pHwCfg, pInst, opndTypeId, pLibName);
        }
        else
        {
            gcoOS_StrCatSafe(*pLibName, __LIB_NAME_LENGTH__,
                VIR_Shader_GetTypeNameString(pShader, VIR_Shader_GetTypeFromId(pShader, opndTypeId)));
        }
    }
}

VSC_ErrCode
_VIR_LinkIntrinsicLib_AddVregSymbol(
    IN VIR_Shader               *pShader,
    IN VIR_Shader               *pLibShader,
    IN VIR_Function             *pFunc,
    IN VIR_Function             *pLibFunc,
    IN VIR_Symbol               *pLibVirRegSym,
    IN VIR_TypeId                virRegTypeId,
    IN gctUINT                  *pRegId,
    OUT VIR_SymId               *pVirRegSymId,
    OUT VSC_HASH_TABLE          *pTempSet
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE, preErrCode = VSC_ERR_NONE;
    VIR_SymId           virRegSymId = VIR_INVALID_ID;
    VIR_Symbol          *pVirRegSym = gcvNULL;
    VIR_Symbol          *pLibRegSym = gcvNULL;
    gctUINT             regCount = 0;
    gctBOOL             realAdd = gcvTRUE;

    gcmASSERT(pRegId);

    /* If this vir reg symbol is a parameters symbol, we need to add all regs of this parameter. */
    if (VIR_Symbol_isParamVirReg(pLibVirRegSym))
    {
        VIR_Symbol      *pLibParamSym = VIR_Symbol_GetVregVariable(pLibVirRegSym);
        VIR_VirRegId     libParamStartVirRegIndex = VIR_Symbol_GetVregIndex(pLibParamSym);
        VIR_VirRegId     libCurrentVirRegIndex = VIR_Symbol_GetVregIndex(pLibVirRegSym);
        VIR_TypeId       tyId = VIR_TYPE_VOID, baseTyId = VIR_TYPE_VOID;
        gctUINT          i;

        gcmASSERT(libCurrentVirRegIndex >= libParamStartVirRegIndex);

        tyId = VIR_LinkLib_TypeConv(pShader, VIR_Symbol_GetType(pLibParamSym), gcvTRUE);
        baseTyId = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(pShader, tyId));

        regCount = VIR_Type_GetRegOrOpaqueCount(pShader,
                                                VIR_Shader_GetTypeFromId(pShader, tyId),
                                                VIR_TypeId_isSampler(baseTyId),
                                                VIR_TypeId_isImage(baseTyId),
                                                VIR_TypeId_isAtomicCounters(baseTyId),
                                                gcvFALSE);

        /*
        ** VIV:TODO: we may use the index range to get the regCount for an array
        ** because when convert gcSHADER to VIR, we create more than one parameter for a parameter array.
        */
        if (VIR_Symbol_GetIndexRange(pLibParamSym) > (gctINT)libParamStartVirRegIndex)
        {
            regCount = vscMAX(regCount, VIR_Symbol_GetIndexRange(pLibParamSym) - libParamStartVirRegIndex);
        }

        for (i = 0; i < regCount; i++)
        {
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           *pRegId + i,
                                           VIR_Shader_GetTypeFromId(pShader, virRegTypeId),
                                           VIR_STORAGE_UNKNOWN,
                                           &virRegSymId);
            if (errCode == VSC_ERR_REDEFINITION)
            {
                realAdd = gcvFALSE;
            }
            if (errCode != VSC_ERR_REDEFINITION && errCode != VSC_ERR_NONE)
            {
                ON_ERROR(errCode, "Add vir reg symbol.");
            }

            pVirRegSym = VIR_Function_GetSymFromId(pFunc, virRegSymId);
            pLibRegSym = VIR_Shader_FindSymbolByTempIndex(pLibShader, libParamStartVirRegIndex + i);
            vscHTBL_DirectSet(pTempSet, (void*) pLibRegSym, (void*) pVirRegSym);

            if (i != 0 && errCode != preErrCode)
            {
                /* The reg of a parameter must be added at the same time. */
                gcmASSERT(gcvFALSE);
            }
            preErrCode = errCode;

            if (libCurrentVirRegIndex - libParamStartVirRegIndex == i)
            {
                if (pVirRegSymId)
                {
                    *pVirRegSymId = virRegSymId;
                }
            }
        }
    }
    else
    {
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       *pRegId,
                                       VIR_Shader_GetTypeFromId(pShader, virRegTypeId),
                                       VIR_STORAGE_UNKNOWN,
                                       &virRegSymId);
        if (errCode == VSC_ERR_REDEFINITION)
        {
            realAdd = gcvFALSE;
        }
        if (errCode != VSC_ERR_REDEFINITION && errCode != VSC_ERR_NONE)
        {
            ON_ERROR(errCode, "Add vir reg symbol.");
        }

        pVirRegSym = VIR_Function_GetSymFromId(pFunc, virRegSymId);
        regCount = VIR_Type_GetRegCount(pShader, VIR_Symbol_GetType(pVirRegSym), gcvFALSE);

        vscHTBL_DirectSet(pTempSet, (void*) pLibVirRegSym, (void*) pVirRegSym);

        if (pVirRegSymId)
        {
            *pVirRegSymId = virRegSymId;
        }
    }

    /* Update reg index. */
    if (realAdd)
    {
        *pRegId = *pRegId + regCount;
    }

OnError:
    return errCode;
}

VSC_ErrCode
_VIR_LinkIntrinsicLib_CopyOpnd(
    IN VIR_Shader               *pShader,
    IN VIR_Shader               *pLibShader,
    IN VIR_Function             *pFunc,
    IN VIR_Function             *libFunc,
    IN VIR_Instruction          *libInst,
    IN VIR_Operand              *libOpnd,
    IN VIR_Instruction          *pInst,
    IN VIR_Operand              *pOpnd,
    OUT VSC_HASH_TABLE          *pTempSet,
    OUT gctUINT                  *tempIndexStart)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(libOpnd);
    VIR_Symbol          *libSym = gcvNULL;
    VIR_Symbol          *newVirRegSym = gcvNULL;
    VIR_Symbol          *newVarSym = gcvNULL;
    VIR_TypeId          pTyId = VIR_TYPE_UNKNOWN;
    VIR_TypeId          origTypeId = VIR_Operand_GetTypeId(libOpnd);
    VIR_SymId           newVirRegId = VIR_INVALID_ID;
    VIR_SymId           newVarId = VIR_INVALID_ID;
    VIR_NameId          nameId;
    gctSTRING           libName;

    if (VIR_Operand_isLvalue(libOpnd))
    {
        VIR_Operand_SetEnable(pOpnd, VIR_Operand_GetEnable(libOpnd));
    }
    else
    {
        VIR_Operand_SetSwizzle(pOpnd, VIR_Operand_GetSwizzle(libOpnd));
    }

    if (opndKind == VIR_OPND_SYMBOL || opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        libSym = VIR_Operand_GetSymbol(libOpnd);
        pTyId = VIR_LinkLib_TypeConv(pShader, VIR_Symbol_GetType(libSym), gcvFALSE);

        if (VIR_Symbol_GetKind(libSym) == VIR_SYM_VIRREG)
        {
            if (vscHTBL_DirectTestAndGet(pTempSet, (void*) libSym, (void **)&newVirRegSym))
            {
                newVirRegId = VIR_Symbol_GetIndex(newVirRegSym);
            }
            else
            {
                _VIR_LinkIntrinsicLib_AddVregSymbol(pShader,
                                                    pLibShader,
                                                    pFunc,
                                                    libFunc,
                                                    libSym,
                                                    pTyId,
                                                    tempIndexStart,
                                                    &newVirRegId,
                                                    pTempSet);

                newVirRegSym = VIR_Function_GetSymFromId(pFunc, newVirRegId);
            }

            VIR_Operand_SetSymbol(pOpnd, pFunc, newVirRegId);

            libSym = VIR_Symbol_GetVregVariable(libSym); /* set the sym to corresponding variable */
        }
        else
        {
            libName = VIR_Shader_GetSymNameString(pLibShader, libSym);
            newVarSym = VIR_Shader_FindSymbolByName(pShader, libSym->_kind, libName);

           /* create attribute localInvocationID if need */
           if (gcmIS_SUCCESS(gcoOS_StrCmp(libName, "gl_LocalInvocationID")))
           {
               VIR_Symbol   *pLocIdSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_LOCAL_INVOCATION_ID);
               if (pLocIdSym == gcvNULL)
               {
                   VIR_SymId     outRegId;
                   VIR_AttributeIdList *pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
                   gctUINT       attrCount = VIR_IdList_Count(pAttrIdLsts);
                   gctUINT       attrIdx;
                   gctUINT       nextAttrLlSlot = 0;
                   for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
                   {
                       VIR_SymId       attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
                       VIR_Symbol      *pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);
                       gctUINT         thisOutputRegCount;

                       if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
                       {
                           gcmASSERT(VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED);

                           thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);
                           if (nextAttrLlSlot < (VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount))
                           {
                               nextAttrLlSlot = VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount;
                           }
                       }
                   }

                   pLocIdSym = VIR_Shader_AddBuiltinAttribute(pShader, VIR_TYPE_UINT_X4, gcvFALSE, VIR_NAME_LOCAL_INVOCATION_ID);
                   outRegId = VIR_Shader_NewVirRegId(pShader, 1);
                   VIR_Shader_AddSymbol(pShader,
                                        VIR_SYM_VIRREG,
                                        outRegId,
                                        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4),
                                        VIR_STORAGE_UNKNOWN,
                                        &newVarId);
                   VIR_Symbol_SetVariableVregIndex(pLocIdSym, outRegId);
                   VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(pShader, newVarId), pLocIdSym);
                   VIR_Symbol_SetIndexRange(pLocIdSym, outRegId + 1);
                   VIR_Symbol_SetIndexRange(VIR_Shader_GetSymFromId(pShader, newVarId), outRegId + 1);
                   VIR_Symbol_SetFirstSlot(pLocIdSym, nextAttrLlSlot);

                   newVarSym = VIR_Function_GetSymFromId(pFunc, newVarId);
               }
               else
               {
                   VIR_SymId vregindex = VIR_Symbol_GetVariableVregIndex(pLocIdSym);
                   newVarSym = VIR_Shader_FindSymbolByTempIndex(pShader, vregindex);
               }
           }
           else if (newVarSym == gcvNULL)
            {
                errCode = VIR_Shader_AddString(pShader,
                                               libName,
                                               &nameId);

                VIR_Shader_AddSymbol(pShader,
                                     VIR_Symbol_GetKind(libSym),
                                     nameId,
                                     VIR_Shader_GetTypeFromId(pShader, pTyId),
                                     VIR_STORAGE_UNKNOWN,
                                     &newVarId);

                newVarSym = VIR_Function_GetSymFromId(pFunc, newVarId);

                if (VIR_Symbol_GetIndex(libSym) == VIR_Shader_GetBaseSamplerId(pLibShader))
                {
                    VIR_Shader_SetBaseSamplerId(pShader, newVarId);
                }
            }
            VIR_Operand_SetSymbol(pOpnd, pFunc, VIR_Symbol_GetIndex(newVarSym));
        }
        /* Copy the operand type. */
        VIR_Operand_SetTypeId(pOpnd, origTypeId);
    }
    else if (opndKind == VIR_OPND_IMMEDIATE)
    {
        switch(VIR_Operand_GetTypeId(libOpnd))
        {
        case VIR_TYPE_FLOAT32:
        case VIR_TYPE_FLOAT16:
            VIR_Operand_SetImmediateFloat(pOpnd, VIR_Operand_GetImmediateFloat(libOpnd));
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_INT16:
            VIR_Operand_SetImmediateInt(pOpnd, VIR_Operand_GetImmediateInt(libOpnd));
            break;

        case VIR_TYPE_UINT32:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_BOOLEAN:
            VIR_Operand_SetImmediateUint(pOpnd, VIR_Operand_GetImmediateUint(libOpnd));
            break;

        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else if (opndKind == VIR_OPND_CONST)
    {
        VIR_ConstVal    new_const_val;
        VIR_ConstId     new_const_id, lib_const_id;
        VIR_Const       *libConst;

        lib_const_id = VIR_Operand_GetConstId(libOpnd);
        libConst = VIR_Shader_GetConstFromId(pLibShader, lib_const_id);

        new_const_val.vecVal.u32Value[0] = libConst->value.vecVal.u32Value[0];
        new_const_val.vecVal.u32Value[1] = libConst->value.vecVal.u32Value[1];
        new_const_val.vecVal.u32Value[2] = libConst->value.vecVal.u32Value[2];
        new_const_val.vecVal.u32Value[3] = libConst->value.vecVal.u32Value[3];

        VIR_Shader_AddConstant(pShader,
            libConst->type, &new_const_val, &new_const_id);

        VIR_Operand_SetConstId(pOpnd, new_const_id);
        VIR_Operand_SetOpKind(pOpnd, VIR_OPND_CONST);
        VIR_Operand_SetTypeId(pOpnd, libConst->type);
        VIR_Operand_SetSwizzle(pOpnd, VIR_Operand_GetSwizzle(libOpnd));
    }
    else if (opndKind == VIR_OPND_TEXLDPARM)
    {
        VIR_Operand *libTexldOperand;
        VIR_Operand *newOperand = gcvNULL;

        VIR_Operand_SetOpKind(pOpnd, VIR_OPND_TEXLDPARM);
        VIR_Operand_SetRoundMode(pOpnd, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(pOpnd, VIR_MOD_NONE);

        libTexldOperand = (VIR_Operand*)libOpnd;

        /* Check bias. */
        if (VIR_Operand_hasBiasFlag(libTexldOperand))
        {
            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldBias(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldBias(pOpnd, newOperand);
        }
        /* Check lod. */
        if (VIR_Operand_hasLodFlag(libTexldOperand))
        {
            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldLod(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldLod(pOpnd, newOperand);
        }
        /* Check offset. */
        if (VIR_Operand_hasOffsetFlag(libTexldOperand))
        {
            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldOffset(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldOffset(pOpnd, newOperand);
        }
        /* Check grad. */
        if (VIR_Operand_hasGradFlag(libTexldOperand))
        {
            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldGrad_dx(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldGradientDx(pOpnd, newOperand);

            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldGrad_dy(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldGradientDy(pOpnd, newOperand);
        }
        /* Check gather. */
        if (VIR_Operand_hasGatherFlag(libTexldOperand))
        {
            if (VIR_Operand_GetTexldGather_comp(libTexldOperand))
            {
                VIR_Function_NewOperand(pFunc, &newOperand);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                    pLibShader,
                    pFunc,
                    libFunc,
                    libInst,
                    VIR_Operand_GetTexldGather_comp(libTexldOperand),
                    pInst,
                    newOperand,
                    pTempSet,
                    tempIndexStart);
                VIR_Operand_SetTexldGatherComp(pOpnd, newOperand);
            }

            if (VIR_Operand_GetTexldGather_refz(libTexldOperand))
            {
                VIR_Function_NewOperand(pFunc, &newOperand);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                    pLibShader,
                    pFunc,
                    libFunc,
                    libInst,
                    VIR_Operand_GetTexldGather_refz(libTexldOperand),
                    pInst,
                    newOperand,
                    pTempSet,
                    tempIndexStart);
                VIR_Operand_SetTexldGatherRefZ(pOpnd, newOperand);
            }
        }
        /* Check fetch ms. */
        if (VIR_Operand_hasFetchMSFlag(libTexldOperand))
        {
            VIR_Function_NewOperand(pFunc, &newOperand);
            errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                     pLibShader,
                                                     pFunc,
                                                     libFunc,
                                                     libInst,
                                                     VIR_Operand_GetTexldFetchMS_sample(libTexldOperand),
                                                     pInst,
                                                     newOperand,
                                                     pTempSet,
                                                     tempIndexStart);
            VIR_Operand_SetTexldFetchMS(pOpnd, newOperand);
        }
    }
    else if (opndKind == VIR_OPND_NAME)
    {
        /* copy name string from lib shader to host shader */
        gctSTRING nameStr = VIR_Shader_GetStringFromId(pLibShader, VIR_Operand_GetNameId(libOpnd));
        VIR_NameId nameId;
        errCode = VIR_Shader_AddString(pShader, nameStr, &nameId);

        VIR_Operand_SetName(pOpnd, nameId);
    }
    else if (opndKind == VIR_OPND_INTRINSIC)
    {
        VIR_Operand_SetIntrinsic(pOpnd, VIR_Operand_GetIntrinsicKind(libOpnd));
    }
    else if (opndKind == VIR_OPND_PARAMETERS)
    {
        VIR_ParmPassing *libParm = VIR_Operand_GetParameters(libOpnd);
        VIR_ParmPassing *parmOpnd = gcvNULL;
        gctUINT i;
        VIR_Operand     *newOperand = gcvNULL;

        VIR_Function_NewParameters(pFunc, libParm->argNum, &parmOpnd);
        if (libParm->argNum > 0)
        {
            for (i = 0; i < libParm->argNum; i++)
            {
                VIR_Function_NewOperand(pFunc, &newOperand);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader,
                                                         pLibShader,
                                                         pFunc,
                                                         libFunc,
                                                         libInst,
                                                         libParm->args[i],
                                                         pInst,
                                                         newOperand,
                                                         pTempSet,
                                                         tempIndexStart);
                parmOpnd->args[i] = newOperand;
            }
        }
        VIR_Operand_SetParameters(pOpnd, parmOpnd);
    }
    else
    {
        /* We may hit VIR_OPND_UNDEF for some opcodes, e.g, IMG_LOAD. */
    }

    if (opndKind != VIR_OPND_TEXLDPARM)
    {
        /* Copy the index from orig operand to new operand. */
        VIR_Operand_SetIsConstIndexing(pOpnd, VIR_Operand_GetIsConstIndexing(libOpnd));
        VIR_Operand_SetRelAddrMode(pOpnd, VIR_Operand_GetRelAddrMode(libOpnd));
        VIR_Operand_SetMatrixConstIndex(pOpnd, VIR_Operand_GetMatrixConstIndex(libOpnd));
        VIR_Operand_SetRelAddrLevel(pOpnd, VIR_Operand_GetRelAddrLevel(libOpnd));
        VIR_Operand_SetRoundMode(pOpnd, VIR_Operand_GetRoundMode(libOpnd));

        /* If it is constant index, just copy; if it is reg indexed, need to map to the new reg. */
        if (VIR_Operand_GetIsConstIndexing(libOpnd))
        {
            VIR_Operand_SetRelIndex(pOpnd, VIR_Operand_GetRelIndexing(libOpnd));
        }
        else if (VIR_Operand_GetRelAddrMode(libOpnd) != VIR_INDEXED_NONE)
        {
            libSym = VIR_Function_GetSymFromId(libFunc, VIR_Operand_GetRelIndexing(libOpnd));
            /* This must be a vreg symbol. */
            gcmASSERT(VIR_Symbol_isVreg(libSym));

            if (vscHTBL_DirectTestAndGet(pTempSet, (void*)libSym, (void **)&newVirRegSym))
            {
                newVirRegId = VIR_Symbol_GetIndex(newVirRegSym);
            }
            else
            {
                if (VIR_TypeId_isSampler(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(libSym))))
                {
                    pTyId = VIR_TYPE_UINT_X4;
                }
                else
                {
                    pTyId = VIR_Type_GetIndex(VIR_Symbol_GetType(libSym));
                }

                VIR_Shader_AddSymbol(pShader,
                    VIR_SYM_VIRREG,
                    *tempIndexStart,
                    VIR_Shader_GetTypeFromId(pShader, pTyId),
                    VIR_STORAGE_UNKNOWN,
                    &newVirRegId);

                newVirRegSym = VIR_Function_GetSymFromId(pFunc, newVirRegId);

                *tempIndexStart = *tempIndexStart +
                    VIR_Type_GetRegCount(pShader, VIR_Symbol_GetType(newVirRegSym), gcvFALSE);

                vscHTBL_DirectSet(pTempSet, (void*)libSym, (void*)newVirRegSym);
            }
            VIR_Operand_SetRelIndex(pOpnd, newVirRegId);
        }
    }

    return errCode;
}

VSC_ErrCode
_VIR_LinkIntrinsicLib_CopyInst(
    IN  VIR_Shader              *pShader,
    IN  VIR_Shader              *pLibShader,
    IN  VIR_Function            *libFunc,
    IN  VIR_Function            *pFunc,
    IN  VIR_Instruction         *libInst,
    IN  VSC_MM                  *pMM,
    OUT VSC_HASH_TABLE          *pAddLibFuncSet,
    OUT VSC_HASH_TABLE          *pLabelSet,
    OUT VSC_HASH_TABLE          *pJmpSet,
    OUT VSC_HASH_TABLE          *pTempSet,
    OUT VIR_LIB_WORKLIST        *pWorkList,
    OUT VIR_LIB_CALLSITES       *pCallSites,
    OUT gctUINT                 *tempIndexStart
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_OpCode      libOpcode = VIR_Inst_GetOpcode(libInst);
    gctUINT         srcNum = VIR_OPCODE_GetSrcOperandNum(libOpcode);
    VIR_Operand     *origDest = gcvNULL, *origSrc = gcvNULL;
    gctUINT i;
    VIR_Instruction *newInst = gcvNULL;
    VIR_TypeId  newTyId = VIR_TYPE_UNKNOWN;

    switch (libOpcode)
    {
    case VIR_OP_CALL:
        {
            /* if function is in the pShader, it is OK,
                otherwise, add it to the queue */
            VIR_Function *libCallee = gcvNULL, *pCallee = gcvNULL;
            VIR_LINKER_CALL_INST_NODE *callInstNode;

            origDest = VIR_Inst_GetDest(libInst);
            libCallee = VIR_Operand_GetFunction(origDest);
            newTyId = VIR_LinkLib_TypeConv(pShader, VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(origDest)), gcvFALSE);

            VIR_Shader_GetFunctionByName(pShader, VIR_Function_GetNameString(libCallee), &pCallee);
            if (pCallee == gcvNULL)
            {
                /* add pCallee to the pShader */
                errCode = VIR_Shader_AddFunction(pShader,
                                                 VIR_Function_GetFlags(libCallee),
                                                 VIR_Function_GetNameString(libCallee),
                                                 newTyId,
                                                 &pCallee);
                ON_ERROR(errCode, "_VIR_LinkIntrinsicLib_CopyInst");

                vscHTBL_DirectSet(pAddLibFuncSet, (void*) pCallee, gcvNULL);
                VIR_LIB_WorkListQueue(pMM, pWorkList, pCallee);
            }

            errCode = VIR_Function_AddInstruction(pFunc, VIR_OP_CALL, newTyId, &newInst);
            VIR_Operand_SetFunction(VIR_Inst_GetDest(newInst), pCallee);
            ON_ERROR(errCode, "_VIR_LinkIntrinsicLib_CopyInst");

            callInstNode = (VIR_LINKER_CALL_INST_NODE *) vscMM_Alloc(pMM, sizeof(VIR_LINKER_CALL_INST_NODE));
            callInstNode->inst = newInst;
            callInstNode->u.libIntrinsicKind = VIR_IK_NONE;

            VIR_LIB_CallSitesQueue(pMM, pCallSites, callInstNode);

            break;
        }
    case VIR_OP_LABEL:
        {
            VIR_LabelId newLabelId;
            VIR_Label   *newLabel = gcvNULL, *libLabel;
            VIR_Symbol  *libSym;
            gctSTRING   labelName = gcvNULL;

            /* the label should have new name to avoid the same name */
            labelName = (gctSTRING) vscMM_Alloc(pMM, __LIB_NAME_LENGTH__ * sizeof(char));

            libLabel = VIR_Operand_GetLabel(libInst->dest);
            libSym = VIR_Function_GetSymFromId(libFunc, libLabel->sym);

            gcoOS_StrCopySafe(labelName, __LIB_NAME_LENGTH__, "_viv_");
            gcoOS_StrCatSafe(labelName, __LIB_NAME_LENGTH__, VIR_Shader_GetSymNameString(pLibShader, libSym));

            errCode = VIR_Function_AddLabel(pFunc,
                                             labelName,
                                             &newLabelId);
            ON_ERROR(errCode, "_VIR_LinkIntrinsicLib_CopyInst");

            errCode = VIR_Function_AddInstruction(pFunc, VIR_OP_LABEL, newTyId, &newInst);
            ON_ERROR(errCode, "_VIR_LinkIntrinsicLib_CopyInst");

            newLabel = VIR_GetLabelFromId(pFunc, newLabelId);
            newLabel->defined = newInst;
            VIR_Operand_SetLabel(newInst->dest, newLabel);
            vscHTBL_DirectSet(pLabelSet, (void*) libLabel, (void*) newLabel);

            vscMM_Free(pMM, labelName);

            break;
        }
    case VIR_OP_JMP:
    case VIR_OP_JMPC:
    case VIR_OP_JMP_ANY:
        {
            VIR_Label       *label = VIR_Operand_GetLabel(VIR_Inst_GetDest(libInst));
            VIR_Label       *pNewLabel = gcvNULL;
            VIR_Link        *pNewLink     = gcvNULL;

            errCode = VIR_Function_AddInstruction(pFunc, libOpcode, newTyId, &newInst);
            ON_ERROR(errCode, "_VIR_LinkIntrinsicLib_CopyInst");

            if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) label, (void **)&pNewLabel))
            {
                VIR_Operand_SetLabel(VIR_Inst_GetDest(newInst), pNewLabel);
                VIR_Function_NewLink(pFunc, &pNewLink);
                VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)newInst);
                VIR_Link_AddLink(&(pNewLabel->referenced), pNewLink);
            }
            else
            {
                /* we need to save the unchanged jmp into a list, its label willl be changed
                    at the end */
                vscHTBL_DirectSet(pJmpSet, (void*) libInst, (void*) newInst);
            }

            VIR_Inst_SetConditionOp(newInst, VIR_Inst_GetConditionOp(libInst));

            /* handle source operand */
            for (i = 0; i < srcNum; i++)
            {
                origSrc = VIR_Inst_GetSource(libInst, i);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader, pLibShader, pFunc, libFunc, libInst,
                                                          origSrc, newInst, newInst->src[i], pTempSet, tempIndexStart);
            }

            break;
        }
    default:
        {
            errCode = VIR_Function_AddInstruction(pFunc, VIR_Inst_GetOpcode(libInst), newTyId, &newInst);

            VIR_Inst_SetConditionOp(newInst, VIR_Inst_GetConditionOp(libInst));

            VIR_Inst_SetResOpType(newInst, VIR_Inst_GetResOpType(libInst));

            /* handle dest operand */
            if (VIR_OPCODE_hasDest(libOpcode))
            {
                origDest = VIR_Inst_GetDest(libInst);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader, pLibShader, pFunc, libFunc, libInst,
                                                          origDest, newInst, VIR_Inst_GetDest(newInst), pTempSet, tempIndexStart);

                VIR_Inst_SetInstType(newInst, VIR_Operand_GetTypeId(VIR_Inst_GetDest(newInst)));
            }

            /* handle source operand */
            for (i = 0; i < srcNum; i++)
            {
                origSrc = VIR_Inst_GetSource(libInst, i);
                errCode = _VIR_LinkIntrinsicLib_CopyOpnd(pShader, pLibShader, pFunc, libFunc, libInst,
                                                          origSrc, newInst, newInst->src[i], pTempSet, tempIndexStart);
            }
        }
        break;
    }

OnError:
    return errCode;
}

/* link the functions in the worklist from library to the shader */
VSC_ErrCode
VIR_Lib_LinkFunctions(
    IN  VIR_LinkLibContext      *Context,
    IN  VIR_Shader              *pShader,
    IN  VIR_Shader              *pLibShader,
    IN  VSC_MM                  *pMM,
    OUT VSC_HASH_TABLE          *pAddLibFuncSet,
    OUT VIR_LIB_WORKLIST        *pWorkList,
    OUT VIR_LIB_CALLSITES       *pCallSites)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Function    *pFunc = gcvNULL;
    VIR_Function    *libFunc = gcvNULL;
    gctSTRING       libName = gcvNULL;
    gctUINT         tempIndexStart;

    VIR_InstIterator inst_iter;
    VIR_Instruction *inst;

    VSC_HASH_TABLE       *pLabelSet;
    VSC_HASH_TABLE       *pJmpSet;
    VSC_HASH_TABLE       *pTempSet = Context->pTempHashTable;

    /* label map to update the jmp target (forward jmp could not find label when processing) */
    pLabelSet = vscHTBL_Create(pMM,
                (PFN_VSC_HASH_FUNC)vscHFUNC_Label, (PFN_VSC_KEY_CMP)vcsHKCMP_Label, 64);

    /* jmp to be updated at the end of the shader processing */
    pJmpSet = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 64);

    while(!QUEUE_CHECK_EMPTY(pWorkList))
    {
        VIR_LIB_WorkListDequeue(pMM, pWorkList, &pFunc);

        /* clean the label/jmp hash table */
        vscHTBL_Reset(pLabelSet);
        vscHTBL_Reset(pJmpSet);

        libName = VIR_Function_GetNameString(pFunc);

        VIR_Shader_GetFunctionByName(pLibShader, libName, &libFunc);

        gcmASSERT(libFunc != gcvNULL);

        /* create temp registers in Shader */
        tempIndexStart = VIR_Shader_NewVirRegId(pShader, libFunc->tempIndexCount);

        /* copy arguments. */
        if (VIR_IdList_Count(&libFunc->paramters) > 0)
        {
            VIR_Symbol  *libParam, *newParam, *newVirReg = gcvNULL;
            VIR_SymId   libParamId, libVirRegId, newParamId, newVirRegId;
            gctINT      libIndexRange = 0;
            gctUINT     i, j, regCount;
            gctUINT     paramTempIndexStart = 0;
            VIR_TypeId  pTyId = VIR_TYPE_VOID;
            VIR_TypeId  pBaseTyId = VIR_TYPE_VOID;
            gctBOOL     isVregExist = gcvFALSE;
            gctBOOL     isCreateNewVreg = gcvFALSE;

            for (i = 0; i < VIR_IdList_Count(&libFunc->paramters); i++)
            {
                isVregExist = gcvFALSE;
                libParamId = VIR_IdList_GetId(&libFunc->paramters, i);
                libParam = VIR_Function_GetSymFromId(libFunc, libParamId);
                libIndexRange = VIR_Symbol_GetIndexRange(libParam) - VIR_Symbol_GetVregIndex(libParam);

                libVirRegId = VIR_Symbol_GetVariableVregIndex(libParam);

                pTyId = VIR_LinkLib_TypeConv(pShader, VIR_Symbol_GetType(libParam), gcvTRUE);

                pBaseTyId = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(pShader, pTyId));

                /* add parameter symbol */
                errCode = VIR_Function_AddParameter(pFunc,
                                                    VIR_Shader_GetSymNameString(pLibShader, libParam),
                                                    pTyId,
                                                    VIR_Symbol_GetStorageClass(libParam),
                                                    &newParamId);

                newParam = VIR_Function_GetSymFromId(pFunc, newParamId);

                /* add parameter virreg */
                regCount = VIR_Type_GetRegOrOpaqueCount(pShader,
                                                        VIR_Shader_GetTypeFromId(pShader, pTyId),
                                                        VIR_TypeId_isSampler(pBaseTyId),
                                                        VIR_TypeId_isImage(pBaseTyId),
                                                        VIR_TypeId_isAtomicCounters(pBaseTyId),
                                                        gcvFALSE);

                for (j = 0; j < regCount; j++)
                {
                    VIR_VirRegId    tmpVregId;
                    VIR_Symbol      *tmpVirReg = gcvNULL;

                    tmpVregId = libVirRegId + j;
                    tmpVirReg = VIR_Shader_FindSymbolByTempIndex(pLibShader, tmpVregId);

                    /* The reg of the parameter may be added before, we need to check it first. */
                    /* And if it is been added before, then all of regs must be added. */
                    if (vscHTBL_DirectTestAndGet(pTempSet, (void*) tmpVirReg, (void **)&newVirReg))
                    {
                        if (j == 0)
                        {
                            paramTempIndexStart = VIR_Symbol_GetVregIndex(newVirReg);
                        }
                        isVregExist = gcvTRUE;
                    }
                    else
                    {
                        VIR_Shader_AddSymbol(pShader,
                                            VIR_SYM_VIRREG,
                                            tempIndexStart + j,
                                            VIR_Shader_GetTypeFromId(pShader, pBaseTyId),
                                            VIR_STORAGE_UNKNOWN,
                                            &newVirRegId);
                        newVirReg = VIR_Function_GetSymFromId(pFunc, newVirRegId);

                        /* save temp to the temp hash table */
                        vscHTBL_DirectSet(pTempSet, (void*) tmpVirReg, (void*) newVirReg);

                        isCreateNewVreg = gcvTRUE;
                    }

                    VIR_Symbol_SetVregVariable(newVirReg, newParam);
                    VIR_Symbol_SetStorageClass(newVirReg, VIR_Symbol_GetStorageClass(libParam));
                    VIR_Symbol_SetParamFuncSymId(newVirReg, VIR_Function_GetSymId(pFunc));
                    VIR_Symbol_SetIndexRange(newVirReg, VIR_Symbol_GetVregIndex(newVirReg) + libIndexRange - j);
                }

                if (isVregExist && isCreateNewVreg)
                {
                    gcmASSERT(gcvFALSE);
                }

                if (isVregExist)
                {
                    newParam->u2.tempIndex = paramTempIndexStart;
                }
                else
                {
                    newParam->u2.tempIndex = tempIndexStart;
                    tempIndexStart = tempIndexStart + regCount;
                }
                VIR_Symbol_SetIndexRange(newParam, VIR_Symbol_GetVregIndex(newParam) + libIndexRange);
            }
        }

        /* Copy the local variable */
        if (VIR_IdList_Count(&libFunc->localVariables) > 0)
        {
            VIR_Symbol  *libVar, *libVirReg, *newVar, *newVirReg;
            VIR_SymId   libVarId, libVirRegId, newVarId, newVirRegId;
            gctUINT     i, regCount;
            VIR_TypeId  pTyId = VIR_TYPE_VOID;

            for (i = 0; i < VIR_IdList_Count(&libFunc->localVariables); i++)
            {
                libVarId = VIR_IdList_GetId(&libFunc->localVariables, i);
                libVar = VIR_Function_GetSymFromId(libFunc, libVarId);

                libVirRegId = VIR_Symbol_GetVariableVregIndex(libVar);
                libVirReg = VIR_Shader_FindSymbolByTempIndex(pLibShader, libVirRegId);

                pTyId = VIR_LinkLib_TypeConv(pShader, VIR_Symbol_GetType(libVar), gcvFALSE);

                /* add local variable symbol */
                errCode = VIR_Function_AddLocalVar(pFunc,
                                                    VIR_Shader_GetSymNameString(pLibShader, libVar),
                                                    pTyId,
                                                    &newVarId);
                ON_ERROR(errCode, "VIR_Lib_LinkFunctions");

                newVar = VIR_Function_GetSymFromId(pFunc, newVarId);

                /* add local variable virreg */
                errCode = VIR_Shader_AddSymbol(pShader,
                                                VIR_SYM_VIRREG,
                                                tempIndexStart,
                                                VIR_Shader_GetTypeFromId(pShader, pTyId),
                                                VIR_STORAGE_UNKNOWN,
                                                &newVirRegId);
                ON_ERROR(errCode, "VIR_Lib_LinkFunctions");

                newVirReg = VIR_Function_GetSymFromId(pFunc, newVirRegId);

                VIR_Symbol_SetVregVariable(newVirReg, newVar);

                newVar->u2.tempIndex = tempIndexStart;
                regCount = VIR_Type_GetRegCount(pShader, VIR_Symbol_GetType(libVar), gcvFALSE);
                tempIndexStart = tempIndexStart + regCount;

                /* save temp to the temp hash table */
                vscHTBL_DirectSet(pTempSet, (void*) libVirReg, (void*) newVirReg);
            }
        }

        /* merge the global variable:
           for all the global variable in libFunc,
           if it is also in pShader, combine them, otherwise */

        /* copy the instructions
            1) change its tempIndex
            2) chnage its input/out to the mapped temp
        */
        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(libFunc));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            _VIR_LinkIntrinsicLib_CopyInst(pShader,
                                           pLibShader,
                                           libFunc,
                                           pFunc,
                                           inst,
                                           pMM,
                                           pAddLibFuncSet,
                                           pLabelSet,
                                           pJmpSet,
                                           pTempSet,
                                           pWorkList,
                                           pCallSites,
                                           &tempIndexStart);
        }

        /* fixup the jmp instruction's label */
        {
            VSC_HASH_ITERATOR jmpSetIter;
            VSC_DIRECT_HNODE_PAIR jmpSetPair;
            vscHTBLIterator_Init(&jmpSetIter, pJmpSet);
            for(jmpSetPair = vscHTBLIterator_DirectFirst(&jmpSetIter);
                IS_VALID_DIRECT_HNODE_PAIR(&jmpSetPair); jmpSetPair = vscHTBLIterator_DirectNext(&jmpSetIter))
            {
                VIR_Instruction* libInst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&jmpSetPair);
                VIR_Instruction* pInst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_SECOND(&jmpSetPair);
                VIR_Label       *libLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(libInst));
                VIR_Label       *newLabel = gcvNULL;
                VIR_Link        *pNewLink = gcvNULL;

                if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) libLabel, (void **)&newLabel) != gcvTRUE)
                {
                    gcmASSERT(gcvFALSE);
                }
                VIR_Operand_SetLabel(VIR_Inst_GetDest(pInst), newLabel);
                VIR_Function_NewLink(pFunc, &pNewLink);
                VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)pInst);
                VIR_Link_AddLink(&(newLabel->referenced), pNewLink);
            }
        }
   }

OnError:
   if (pLabelSet)
    {
        vscHTBL_Destroy(pLabelSet);
    }

    if (pJmpSet)
    {
        vscHTBL_Destroy(pJmpSet);
    }

    /* Return the status. */
    return errCode;

}

static gctBOOL
_NeedAddExtraImageLayer(
    IN  VSC_HW_CONFIG           *pHwCfg,
    IN  VIR_IntrinsicsKind       IntrinsicsKind,
    IN  VIR_Symbol              *ImageSym,
    IN  VIR_Type                *ImageType
    )
{
    gctBOOL result = gcvFALSE;

    if ((VIR_Intrinsics_isImageLoad(IntrinsicsKind)  || VIR_Intrinsics_isImageStore(IntrinsicsKind)) &&
        !pHwCfg->hwFeatureFlags.support128BppImage &&
        VIR_Symbol_Is128Bpp(ImageSym) &&
        !VIR_TypeId_isImageBuffer(VIR_Type_GetIndex(ImageType)))
    {
        result = gcvTRUE;
    }

    return result;
}

VSC_ErrCode
_UpdateOperandParameterForIntrinsicCall(
    IN  VIR_Shader              *pShader,
    IN  VSC_HW_CONFIG           *pHwCfg,
    IN  VIR_Instruction         *pCallInst,
    IN  VIR_IntrinsicsKind       IntrinsicsKind
    )
{
    VSC_ErrCode                  errCode = VSC_ERR_NONE;
    VIR_Function                *func = VIR_Inst_GetFunction(pCallInst);
    VIR_Operand                 *paramOperand = VIR_Inst_GetSource(pCallInst, 1);
    VIR_ParmPassing             *opndParm = gcvNULL;
    VIR_ParmPassing             *newOpndParam = gcvNULL;
    VIR_Operand                 *newOperand = gcvNULL;
    VIR_SymId                    newSymId = VIR_INVALID_ID;
    VIR_Symbol                  *newSymbol = gcvNULL;
    VIR_TypeId                   newTypeId = VIR_INVALID_ID;
    gctUINT                      argNum, newArgNum;
    gctUINT                      i;

    opndParm = VIR_Operand_GetParameters(paramOperand);
    argNum = opndParm->argNum;

    /* Check image-related. */
    if (VIR_Intrinsics_isImageRelated(IntrinsicsKind))
    {
        VIR_Operand             *imageOpnd = opndParm->args[0];
        VIR_Symbol              *imageSym = VIR_Operand_GetSymbol(imageOpnd);
        VIR_Uniform             *image = VIR_Symbol_GetImage(imageSym);
        VIR_Type                *imageType = VIR_Symbol_GetType(imageSym);
        VIR_SymId                extraLayerSymId;

        /* Check if we need to add extra layer image for this image, and put it to the second parameter. */
        if (_NeedAddExtraImageLayer(pHwCfg, IntrinsicsKind, imageSym, imageType))
        {
            extraLayerSymId = image->u.samplerOrImageAttr.extraImageLayer;
            if (extraLayerSymId == VIR_INVALID_ID)
            {
                VIR_Symbol          *extraLayerSym;
                VIR_Uniform         *extraLayer;
                VIR_NameId           nameId;
                gctCHAR              name[128] = "#";

                gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, imageSym));
                gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$ExtraLayer");
                errCode = VIR_Shader_AddString(pShader,
                                               name,
                                               &nameId);
                ON_ERROR(errCode, "VIR_Shader_AddString");

                errCode = VIR_Shader_AddSymbol(pShader,
                                               VIR_SYM_IMAGE,
                                               nameId,
                                               imageType,
                                               VIR_STORAGE_UNKNOWN,
                                               &extraLayerSymId);
                ON_ERROR(errCode, "VIR_Shader_AddSymbol");

                extraLayerSym = VIR_Shader_GetSymFromId(pShader, extraLayerSymId);
                image->u.samplerOrImageAttr.extraImageLayer = extraLayerSymId;
                VIR_Symbol_SetFlag(extraLayerSym, VIR_SYMFLAG_COMPILER_GEN);
                VIR_Symbol_SetPrecision(extraLayerSym, VIR_Symbol_GetPrecision(imageSym));
                VIR_Symbol_SetUniformKind(extraLayerSym, VIR_UNIFORM_EXTRA_LAYER);
                VIR_Symbol_SetAddrSpace(extraLayerSym, VIR_AS_CONSTANT);
                VIR_Symbol_SetTyQualifier(extraLayerSym, VIR_Symbol_GetTyQualifier(imageSym));
                extraLayerSym->layout = imageSym->layout;

                extraLayer = VIR_Symbol_GetImage(extraLayerSym);
                extraLayer->u.samplerOrImageAttr.parentSamplerSymId = image->sym;
                extraLayer->u.samplerOrImageAttr.arrayIdxInParent = NOT_ASSIGNED;
                extraLayer->u.samplerOrImageAttr.texelBufferToImageSymId = NOT_ASSIGNED;
            }

            /* New a operand with this extra image layer.  */
            errCode = VIR_Function_NewOperand(func, &newOperand);
            ON_ERROR(errCode, "VIR_Function_NewOperand");

            VIR_Operand_SetSymbol(newOperand, func, extraLayerSymId);
            VIR_Operand_SetTypeId(newOperand, VIR_Operand_GetTypeId(imageOpnd));
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XYZW);
            VIR_Operand_SetRoundMode(newOperand, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(newOperand, VIR_MOD_NONE);
            /* Copy the index from image operand. */
            VIR_Operand_SetIsConstIndexing(newOperand, VIR_Operand_GetIsConstIndexing(imageOpnd));
            VIR_Operand_SetRelIndex(newOperand, VIR_Operand_GetRelIndexing(imageOpnd));
            VIR_Operand_SetRelAddrMode(newOperand, VIR_Operand_GetRelAddrMode(imageOpnd));
            VIR_Operand_SetMatrixConstIndex(newOperand, VIR_Operand_GetMatrixConstIndex(imageOpnd));
            VIR_Operand_SetRelAddrLevel(newOperand, VIR_Operand_GetRelAddrLevel(imageOpnd));

            /* Put the new operand it into the para list. */
            newArgNum = argNum + 1;
            VIR_Function_NewParameters(func, newArgNum, &newOpndParam);

            newOpndParam->args[0] = opndParm->args[0];
            newOpndParam->args[1] = newOperand;
            for (i = 2; i < newArgNum; i++)
            {
                newOpndParam->args[i] = opndParm->args[i - 1];
            }

            /* Update the parameter operand. */
            VIR_Operand_SetParameters(paramOperand, newOpndParam);
            opndParm = newOpndParam;
            argNum = newArgNum;
        }
    }
    /* Check interpolated-related:
        vec4 _viv_interpolateAtCentroid_vec4(vec4 interpolant,
                                             int interpolateType,
                                             vec4 position,
                                             bool multiSampleBuffer,
                                             bool underSampleShading,
                                             int sampleId,
                                             int sampleMaskIn,
                                             vec4 sampleLocation[4])
        vec4 _viv_interpolateAtSample_vec2(vec4 interpolant,
                                           int sampleIndex,
                                           int interpolateType,
                                           vec4 position,
                                           bool multiSampleBuffer,
                                           bool underSampleShading,
                                           int sampleId,
                                           vec4 sampleLocation[4])
        vec4 _viv_interpolateAtOffset_float(vec4 interpolant,
                                            vec2 offset,
                                            int interpolateType,
                                            vec4 position,
                                            bool multiSampleBuffer,
                                            bool underSampleShading,
                                            int sampleId,
                                            vec4 sampleLocation[4])

    */
    else if (VIR_intrinsics_isInterpolateRelated(IntrinsicsKind))
    {
        gctINT interpolateType = 0;
        gctBOOL underSampleShading = VIR_Shader_PS_RunOnSampleShading(pShader);
        gctUINT argIdx = 0, i = 0;
        VIR_VirRegId newVirRegId;
        VIR_SymId newVirRegSymId;
        VIR_Symbol *newVirRegSym = gcvNULL;
        VIR_Symbol *interpolantVar = gcvNULL;

        gcmASSERT(VIR_Operand_isSymbol(opndParm->args[0]));

        interpolantVar = VIR_Operand_GetSymbol(opndParm->args[0]);
        if (isSymCentroid(interpolantVar))
        {
            interpolateType = 0;
        }
        else if (isSymSample(interpolantVar))
        {
            interpolateType = 1;
        }
        else
        {
            interpolateType = 2;
        }

        /* Adjust the argment count. */
        if (VIR_Intrinsics_isInterpolateAtCentroid(IntrinsicsKind))
        {
            newArgNum = argNum + 10;
        }
        else if (VIR_Intrinsics_isInterpolateAtSample(IntrinsicsKind))
        {
            newArgNum = argNum + 9;
        }
        else
        {
            gcmASSERT(VIR_Intrinsics_isInterpolateAtOffset(IntrinsicsKind));

            newArgNum = argNum + 9;
        }

        /* Allocate a new parameter list. */
        VIR_Function_NewParameters(func, newArgNum, &newOpndParam);

        /* Copy the original parameters. */
        for (argIdx = 0; argIdx < argNum; argIdx++)
        {
            newOpndParam->args[argIdx] = opndParm->args[argIdx];
        }

        /* Update the parameter operand. */
        VIR_Operand_SetParameters(paramOperand, newOpndParam);

        /* Now insert the new parameters. */
        /* 1. Insert interpolate type. */
        errCode = VIR_Function_NewOperand(func, &newOperand);
        ON_ERROR(errCode, "new operand");
        VIR_Operand_SetImmediateInt(newOperand, interpolateType);
        newOpndParam->args[argIdx++] = newOperand;

        /* 2. Insert position. */
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_POSITION,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X4),
                                       VIR_STORAGE_INPUT,
                                       &newSymId);
        gcmASSERT(newSymId != VIR_INVALID_ID);
        newSymbol = VIR_Shader_GetSymFromId(pShader, newSymId);
        VIR_Symbol_SetPrecision(newSymbol, VIR_PRECISION_HIGH);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_STATICALLY_USED);
        VIR_Symbol_ClrFlag(newSymbol, VIR_SYMFLAG_UNUSED);

        if (errCode == VSC_ERR_NONE)
        {
            newVirRegId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           newVirRegId,
                                           VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X4),
                                           VIR_STORAGE_UNKNOWN,
                                           &newVirRegSymId);
            gcmASSERT(newVirRegSymId != VIR_INVALID_ID);

            newVirRegSym = VIR_Shader_GetSymFromId(pShader, newVirRegSymId);
            VIR_Symbol_SetVregVarSymId(newVirRegSym, newSymId);
            VIR_Symbol_SetVariableVregIndex(newSymbol, newVirRegId);
        }

        errCode = VIR_Function_NewOperand(func, &newOperand);
        ON_ERROR(errCode, "new operand");
        VIR_Operand_SetSymbol(newOperand, VIR_Shader_GetMainFunction(pShader), newSymId);
        VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XYZW);
        newOpndParam->args[argIdx++] = newOperand;

        /* 3. Insert multiSampleBuffer. */
        newSymId = VIR_INVALID_ID;
        errCode = VIR_Shader_AddSymbolWithName(pShader,
                                               VIR_SYM_UNIFORM,
                                               "#EnableMultiSampleBuffer",
                                               VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_BOOLEAN),
                                               VIR_STORAGE_UNKNOWN,
                                               &newSymId);
        gcmASSERT(newSymId != VIR_INVALID_ID);

        newSymbol = VIR_Shader_GetSymFromId(pShader, newSymId);
        VIR_Symbol_SetPrecision(newSymbol, VIR_PRECISION_HIGH);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_STATICALLY_USED);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetUniformKind(newSymbol, VIR_UNIFORM_ENABLE_MULTISAMPLE_BUFFERS);

        errCode = VIR_Function_NewOperand(func, &newOperand);
        ON_ERROR(errCode, "new operand");
        VIR_Operand_SetSymbol(newOperand, VIR_Shader_GetMainFunction(pShader), newSymId);
        VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
        newOpndParam->args[argIdx++] = newOperand;

        /* 4. Insert underSampleShading. */
        errCode = VIR_Function_NewOperand(func, &newOperand);
        ON_ERROR(errCode, "new operand");
        VIR_Operand_SetImmediateBoolean(newOperand, underSampleShading);
        newOpndParam->args[argIdx++] = newOperand;

        /* 5. Insert sampleId. */
        newSymId = VIR_INVALID_ID;
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VARIABLE,
                                       VIR_NAME_SAMPLE_ID,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                       VIR_STORAGE_INPUT,
                                       &newSymId);
        gcmASSERT(newSymId != VIR_INVALID_ID);
        newSymbol = VIR_Shader_GetSymFromId(pShader, newSymId);
        VIR_Symbol_SetPrecision(newSymbol, VIR_PRECISION_HIGH);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_STATICALLY_USED);
        VIR_Symbol_ClrFlag(newSymbol, VIR_SYMFLAG_UNUSED);

        if (errCode == VSC_ERR_NONE)
        {
            newVirRegId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           newVirRegId,
                                           VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                           VIR_STORAGE_UNKNOWN,
                                           &newVirRegSymId);
            gcmASSERT(newVirRegSymId != VIR_INVALID_ID);

            newVirRegSym = VIR_Shader_GetSymFromId(pShader, newVirRegSymId);
            VIR_Symbol_SetVregVarSymId(newVirRegSym, newSymId);
            VIR_Symbol_SetVariableVregIndex(newSymbol, newVirRegId);
        }

        errCode = VIR_Function_NewOperand(func, &newOperand);
        ON_ERROR(errCode, "new operand");
        VIR_Operand_SetSymbol(newOperand, VIR_Shader_GetMainFunction(pShader), newSymId);
        VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
        newOpndParam->args[argIdx++] = newOperand;

        /* 6. Insert sampleMaskIn for InterpolateAtCentroid. */
        if (VIR_Intrinsics_isInterpolateAtCentroid(IntrinsicsKind))
        {
            gctUINT32 arrayLength = (GetGLMaxSamples() + 31) / 32;
            errCode = VIR_Shader_AddArrayType(pShader,
                                              VIR_TYPE_INT32,
                                              arrayLength,
                                              4,
                                              &newTypeId);
            ON_ERROR(errCode, "new array type");

            newSymId = VIR_INVALID_ID;
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VARIABLE,
                                           VIR_NAME_SAMPLE_MASK_IN,
                                           VIR_Shader_GetTypeFromId(pShader, newTypeId),
                                           VIR_STORAGE_INPUT,
                                           &newSymId);
            gcmASSERT(newSymId != VIR_INVALID_ID);

            newSymbol = VIR_Shader_GetSymFromId(pShader, newSymId);
            VIR_Symbol_SetPrecision(newSymbol, VIR_PRECISION_HIGH);
            VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_STATICALLY_USED);
            VIR_Symbol_ClrFlag(newSymbol, VIR_SYMFLAG_UNUSED);

            if (errCode == VSC_ERR_NONE)
            {
                newVirRegId = VIR_Shader_NewVirRegId(pShader, arrayLength);
                for (i = 0; i < arrayLength; i++)
                {
                    errCode = VIR_Shader_AddSymbol(pShader,
                                                   VIR_SYM_VIRREG,
                                                   newVirRegId + i,
                                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &newVirRegSymId);
                    gcmASSERT(newVirRegSymId != VIR_INVALID_ID);

                    newVirRegSym = VIR_Shader_GetSymFromId(pShader, newVirRegSymId);
                    VIR_Symbol_SetVregVarSymId(newVirRegSym, newSymId);
                    VIR_Symbol_SetVariableVregIndex(newSymbol, newVirRegId);
                }
            }

            errCode = VIR_Function_NewOperand(func, &newOperand);
            ON_ERROR(errCode, "new operand");
            VIR_Operand_SetSymbol(newOperand, VIR_Shader_GetMainFunction(pShader), newSymId);
            VIR_Operand_SetTypeId(newOperand, VIR_TYPE_INT32);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XXXX);
            VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
            VIR_Operand_SetRelIndexingImmed(newOperand, 0);
            newOpndParam->args[argIdx++] = newOperand;
        }

        /* 7. Insert sampleLocation. */
        errCode = VIR_Shader_AddArrayType(pShader,
                                          VIR_TYPE_FLOAT_X4,
                                          4,
                                          16,
                                          &newTypeId);
        ON_ERROR(errCode, "new array type");

        newSymId = VIR_INVALID_ID;
        errCode = VIR_Shader_AddSymbolWithName(pShader,
                                               VIR_SYM_UNIFORM,
                                               "#SampleLocation",
                                               VIR_Shader_GetTypeFromId(pShader, newTypeId),
                                               VIR_STORAGE_UNKNOWN,
                                               &newSymId);
        gcmASSERT(newSymId != VIR_INVALID_ID);

        newSymbol = VIR_Shader_GetSymFromId(pShader, newSymId);
        VIR_Symbol_SetPrecision(newSymbol, VIR_PRECISION_HIGH);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_STATICALLY_USED);
        VIR_Symbol_SetFlag(newSymbol, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetUniformKind(newSymbol, VIR_UNIFORM_SAMPLE_LOCATION);

        for (i = 0; i < 4; i++)
        {
            errCode = VIR_Function_NewOperand(func, &newOperand);
            ON_ERROR(errCode, "new operand");
            VIR_Operand_SetSymbol(newOperand, VIR_Shader_GetMainFunction(pShader), newSymId);
            VIR_Operand_SetTypeId(newOperand, VIR_TYPE_FLOAT_X4);
            VIR_Operand_SetSwizzle(newOperand, VIR_SWIZZLE_XYZW);
            VIR_Operand_SetIsConstIndexing(newOperand, gcvTRUE);
            VIR_Operand_SetRelIndexingImmed(newOperand, i);
            newOpndParam->args[argIdx++] = newOperand;
        }

        gcmASSERT(argIdx == newArgNum);
    }
    else if (VIR_Intrinsics_isImageFetchForSampler(IntrinsicsKind))
    {
        VIR_TypeId      opndTypeId = VIR_Operand_GetTypeId(opndParm->args[0]);
        VIR_Operand*    pTexldOperand = gcvNULL;

        /* No texldParm for samplerBuffer. */
        if (VIR_TypeId_isSamplerBuffer(opndTypeId))
        {
            return errCode;
        }

        /* If there is no texldParm operand, create one first.*/
        if (argNum == 2)
        {
            VIR_Operand*        pNewOpnd = gcvNULL;

            /* Create the texld parameter operand. */
            errCode = VIR_Function_NewOperand(func, &pNewOpnd);
            ON_ERROR(errCode, "new operand");
            VIR_Operand_SetOpKind(pNewOpnd, VIR_OPND_TEXLDPARM);

            /* Update the parameter operand. */
            VIR_Function_NewParameters(func, 3, &newOpndParam);
            newOpndParam->args[0] = opndParm->args[0];
            newOpndParam->args[1] = opndParm->args[1];
            newOpndParam->args[2] = pNewOpnd;
            newOpndParam->argNum = 3;

            VIR_Operand_SetParameters(paramOperand, newOpndParam);
            opndParm = newOpndParam;
            argNum = 3;
        }

        pTexldOperand = opndParm->args[2];

        /* If there is no LOD parameter, add one whose value is zero. */
        if (!VIR_Operand_GetTexldLod(pTexldOperand))
        {
            VIR_Operand*        pNewOpnd = gcvNULL;

            /* Update the offset parameter. */
            errCode = VIR_Function_NewOperand(func, &pNewOpnd);
            ON_ERROR(errCode, "new operand");
            VIR_Operand_SetImmediateInt(pNewOpnd, 0);
            VIR_Operand_SetTexldLod(pTexldOperand, pNewOpnd);
        }

        /* We treat a sampler1D as a sampler2D, so we need to convert the coordinate, and the offset if needed,*/
        if (VIR_TypeId_isSampler1D(opndTypeId))
        {
            VIR_Operand*        pCoordOpnd = opndParm->args[1];
            VIR_Operand*        pTexldOperand = opndParm->args[2];
            VIR_Operand*        pOffsetOpnd = VIR_Operand_GetTexldOffset(pTexldOperand);
            VIR_TypeId          coordTypeId = VIR_Operand_GetTypeId(pCoordOpnd);
            VIR_TypeId          coordCompTypeId = VIR_GetTypeComponentType(coordTypeId);
            gctUINT             coordCount = VIR_GetTypeComponents(coordTypeId);
            VIR_TypeId          newCoordTypeId = VIR_INVALID_ID;
            VIR_SymId           newCoordSymId = VIR_INVALID_ID;
            VIR_VirRegId        newVirRegId = VIR_INVALID_ID;
            VIR_Instruction*    pNewInst = gcvNULL;
            VIR_Operand*        pNewOpnd = gcvNULL;

            /* Construct a new reg to hold the new coordiante, the Y channel is always zero. */
            gcmASSERT(coordCount <= 2);
            newCoordTypeId = VIR_TypeId_ComposeNonOpaqueType(coordCompTypeId, coordCount + 1, 1);

            newVirRegId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           newVirRegId,
                                           VIR_Shader_GetTypeFromId(pShader, newCoordTypeId),
                                           VIR_STORAGE_UNKNOWN,
                                           &newCoordSymId);
            ON_ERROR(errCode, "add new symbol");

            /* Construct the X coordinate. */
            errCode = VIR_Function_AddInstructionBefore(func,
                                                        VIR_OP_MOV,
                                                        coordCompTypeId,
                                                        pCallInst,
                                                        gcvTRUE,
                                                        &pNewInst);
            ON_ERROR(errCode, "add new instruction");

            /* Set dest. */
            pNewOpnd = VIR_Inst_GetDest(pNewInst);
            VIR_Operand_SetTempRegister(pNewOpnd,
                                        func,
                                        newCoordSymId,
                                        coordCompTypeId);
            VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

            /* Set SRC0. */
            pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
            VIR_Operand_Copy(pNewOpnd, pCoordOpnd);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

            /* Construct the Y coordinate. */
            errCode = VIR_Function_AddInstructionBefore(func,
                                                        VIR_OP_MOV,
                                                        coordCompTypeId,
                                                        pCallInst,
                                                        gcvTRUE,
                                                        &pNewInst);
            ON_ERROR(errCode, "add new instruction");

            /* Set dest. */
            pNewOpnd = VIR_Inst_GetDest(pNewInst);
            VIR_Operand_SetTempRegister(pNewOpnd,
                                        func,
                                        newCoordSymId,
                                        coordCompTypeId);
            VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_Y);

            /* Set SRC0. */
            pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
            VIR_Operand_SetImmediateInt(pNewOpnd, 0);

            /* Construct the array coordinate if needed. */
            if (coordCount > 1)
            {
                errCode = VIR_Function_AddInstructionBefore(func,
                                                            VIR_OP_MOV,
                                                            coordCompTypeId,
                                                            pCallInst,
                                                            gcvTRUE,
                                                            &pNewInst);
                ON_ERROR(errCode, "add new instruction");

                /* Set dest. */
                pNewOpnd = VIR_Inst_GetDest(pNewInst);
                VIR_Operand_SetTempRegister(pNewOpnd,
                                            func,
                                            newCoordSymId,
                                            coordCompTypeId);
                VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_Z);

                /* Set SRC0. */
                pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                VIR_Operand_Copy(pNewOpnd, pCoordOpnd);
                VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_YYYY);
            }

            /* Update the parameter. */
            errCode = VIR_Function_NewOperand(func, &pNewOpnd);
            ON_ERROR(errCode, "new operand");
            VIR_Operand_SetSymbol(pNewOpnd, func, newCoordSymId);
            VIR_Operand_SetTypeId(pNewOpnd, newCoordTypeId);
            VIR_Operand_SetSwizzle(pNewOpnd, VIR_TypeId_Conv2Swizzle(newCoordTypeId));
            opndParm->args[1] = pNewOpnd;

            /* convert the offset. */
            if (pOffsetOpnd != gcvNULL)
            {
                VIR_SymId       newOffsetSymId = VIR_INVALID_ID;

                newVirRegId = VIR_Shader_NewVirRegId(pShader, 1);
                errCode = VIR_Shader_AddSymbol(pShader,
                                               VIR_SYM_VIRREG,
                                               newVirRegId,
                                               VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X2),
                                               VIR_STORAGE_UNKNOWN,
                                               &newOffsetSymId);
                ON_ERROR(errCode, "add new symbol");

                /* Construct the X channel. */
                errCode = VIR_Function_AddInstructionBefore(func,
                                                            VIR_OP_MOV,
                                                            VIR_TYPE_INT32,
                                                            pCallInst,
                                                            gcvTRUE,
                                                            &pNewInst);
                ON_ERROR(errCode, "add new instruction");

                /* Set dest. */
                pNewOpnd = VIR_Inst_GetDest(pNewInst);
                VIR_Operand_SetTempRegister(pNewOpnd,
                                            func,
                                            newOffsetSymId,
                                            VIR_TYPE_INT32);
                VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

                /* Set SRC0. */
                pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                VIR_Operand_Copy(pNewOpnd, pOffsetOpnd);
                VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

                /* Construct the Y coordinate. */
                errCode = VIR_Function_AddInstructionBefore(func,
                                                            VIR_OP_MOV,
                                                            VIR_TYPE_INT32,
                                                            pCallInst,
                                                            gcvTRUE,
                                                            &pNewInst);
                ON_ERROR(errCode, "add new instruction");

                /* Set dest. */
                pNewOpnd = VIR_Inst_GetDest(pNewInst);
                VIR_Operand_SetTempRegister(pNewOpnd,
                                            func,
                                            newOffsetSymId,
                                            VIR_TYPE_INT32);
                VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_Y);

                /* Set SRC0. */
                pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                VIR_Operand_SetImmediateInt(pNewOpnd, 0);

                /* Update the offset parameter. */
                errCode = VIR_Function_NewOperand(func, &pNewOpnd);
                ON_ERROR(errCode, "new operand");
                VIR_Operand_SetSymbol(pNewOpnd, func, newOffsetSymId);
                VIR_Operand_SetTypeId(pNewOpnd, VIR_TYPE_INTEGER_X2);
                VIR_Operand_SetSwizzle(pNewOpnd, VIR_TypeId_Conv2Swizzle(VIR_TYPE_INTEGER_X2));
                VIR_Operand_SetTexldOffset(pTexldOperand, pNewOpnd);
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_UpdateResOpType(
    IN  VIR_RES_OP_TYPE         ResOpType,
    IN  VIR_Function           *pCalleeFunc
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_InstIterator            inst_iter;
    VIR_Instruction            *inst;

    if (ResOpType == VIR_RES_OP_TYPE_UNKNOWN)
    {
        return errCode;
    }

    VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pCalleeFunc));
    for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
         inst != gcvNULL;
         inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
    {
        /* Skip none texld-related instructions. */
        if (!VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)))
        {
            continue;
        }

        VIR_Inst_SetResOpType(inst, ResOpType);
    }
    return errCode;
}

/* update each call site in pCallSites
   INTRINSIC: insert MOV for input/output and change the instruction to CALL
   CALL:  change the MOV for input/output to the new renamed vreg */
VSC_ErrCode
VIR_Lib_UpdateCallSites(
    IN  VIR_LinkLibContext      *Context,
    IN  VIR_Shader              *pShader,
    IN  VIR_Shader              *pLibShader,
    IN  VSC_HW_CONFIG           *pHwCfg,
    IN  VSC_MM                  *pMM,
    IN  VSC_HASH_TABLE          *pAddLibFuncSet,
    OUT VIR_LIB_CALLSITES       *pCallSites)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;

    VIR_LINKER_CALL_INST_NODE *pCallInstNode = gcvNULL;
    VIR_IntrinsicsKind  intrinsicsKind;
    VIR_Function        *pCallerFunc = gcvNULL, *pCalleeFunc = gcvNULL;
    VIR_Instruction     *pCallerInst = gcvNULL, *newInst = gcvNULL;
    VIR_RES_OP_TYPE     callerInstResOpType;
    VIR_ParmPassing     *opndParm = gcvNULL;
    VIR_SymId           parmSymId, parmVregId;
    VIR_Symbol          *parmSym, *parmVregSym;
    VIR_Operand         *destOpnd = gcvNULL;
    VIR_Operand         *opnd = gcvNULL;
    VIR_Operand         *newOpnd = gcvNULL;
    VIR_Operand *texldOperand = gcvNULL;
    VIR_Function        *func;
    VIR_OpCode          opcode;
    VIR_Enable          movEnable = VIR_ENABLE_NONE;
    VIR_Type            *parmType = gcvNULL, *dstType = gcvNULL;
    VIR_TypeId          typeId;
    gctUINT             i, argIndex, texldModifierIndex = 0;
    gctUINT             argCount = 0;

    while(!QUEUE_CHECK_EMPTY(pCallSites))
    {
        VIR_LIB_CallSitesDequeue(pMM, pCallSites, &pCallInstNode);
        pCallerInst = pCallInstNode->inst;
        intrinsicsKind = pCallInstNode->u.libIntrinsicKind;

        callerInstResOpType = VIR_Inst_GetResOpType(pCallerInst);
        destOpnd = VIR_Inst_GetDest(pCallerInst);

        pCallerFunc = VIR_Inst_GetFunction(pCallerInst);

        if (VIR_Inst_GetOpcode(pCallerInst) == VIR_OP_INTRINSIC ||
            VIR_Inst_GetOpcode(pCallerInst) == VIR_OP_EXTCALL)
        {
            /* Update operand parameter list if needed. */
            _UpdateOperandParameterForIntrinsicCall(pShader, pHwCfg, pCallerInst, intrinsicsKind);

            func = VIR_Inst_GetFunction(pCallerInst);
            opndParm = VIR_Operand_GetParameters(VIR_Inst_GetSource(pCallerInst, 1));

            pCalleeFunc = VIR_Operand_GetFunction(VIR_Inst_GetSource(pCallerInst, 0));

            /* Update the instructions of callee function. */
            _UpdateResOpType(callerInstResOpType, pCalleeFunc);

            /* Init index. */
            argIndex = 0;
            texldModifierIndex = 0;
            texldOperand = gcvNULL;
            argCount = opndParm->argNum;
            /* input and output parameter */
            for (i = 0; i < VIR_IdList_Count(&pCalleeFunc->paramters); i++)
            {
                parmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, i);
                parmSym = VIR_Function_GetSymFromId(pCalleeFunc, parmSymId);

                parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);
                parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);

                parmType = VIR_Symbol_GetType(parmVregSym);
                movEnable = VIR_Type_Conv2Enable(parmType);

                /* If the argument index is bigger than argNum, it must be the return value. */
                if (argIndex >= argCount)
                {
                    opnd = gcvNULL;
                }
                /* If this operand is a texld modifier, check all modifiers first. */
                else if (texldOperand)
                {
                    while (texldModifierIndex < VIR_TEXLDMODIFIER_COUNT && VIR_Operand_GetTexldModifier(texldOperand, texldModifierIndex) == gcvNULL)
                    {
                        texldModifierIndex++;
                    }
                    /* If all modifiers are been check, then move back to arg list. */
                    if (texldModifierIndex == VIR_TEXLDMODIFIER_COUNT)
                    {
                        texldOperand = gcvNULL;
                        opnd = opndParm->args[argIndex];
                        if (VIR_Operand_isUndef(opnd))
                        {
                            argCount--;
                        }
                        argIndex++;
                    }
                    else
                    {
                        opnd = VIR_Operand_GetTexldModifier(texldOperand, texldModifierIndex);
                        texldModifierIndex++;
                    }
                }
                else
                {
                    opnd = opndParm->args[argIndex];
                    if (VIR_Operand_isUndef(opnd))
                    {
                        argCount--;
                    }
                    argIndex++;
                    if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_TEXLDPARM)
                    {
                        texldOperand = (VIR_Operand*)opnd;
                        while (texldModifierIndex < VIR_TEXLDMODIFIER_COUNT &&
                               VIR_Operand_GetTexldModifier(texldOperand, texldModifierIndex) == gcvNULL)
                        {
                            texldModifierIndex++;
                        }
                        gcmASSERT(texldModifierIndex < VIR_TEXLDMODIFIER_COUNT);
                        opnd = VIR_Operand_GetTexldModifier(texldOperand, texldModifierIndex);
                        texldModifierIndex++;
                    }
                }

                if (VIR_Symbol_isInParam(parmSym))
                {
                    opcode = VIR_OP_MOV;
                    typeId = VIR_Operand_GetTypeId(opnd);
                    if (VIR_TypeId_isSampler(VIR_Operand_GetTypeId(opnd)))
                    {
                        opcode = VIR_OP_GET_SAMPLER_IDX;
                        typeId = VIR_TYPE_UINT_X4;
                    }
                    /* matrix and struct's input is already lowered */
                    VIR_Function_AddInstructionBefore(pCallerFunc,
                                                      opcode,
                                                      VIR_TYPE_UNKNOWN,
                                                      pCallerInst,
                                                      gcvTRUE,
                                                      &newInst);

                    VIR_Operand_SetTempRegister(newInst->dest,
                                                pCallerFunc,
                                                VIR_Symbol_GetIndex(parmVregSym),
                                                typeId);
                    VIR_Operand_SetEnable(newInst->dest, movEnable);

                    newOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
                    VIR_Operand_Copy(newOpnd, opnd);
                    VIR_Operand_SetSwizzle(newOpnd, VIR_Enable_2_Swizzle(movEnable));

                    /* Set the offset.*/
                    if (VIR_TypeId_isSampler(VIR_Operand_GetTypeId(opnd)))
                    {
                        VIR_Operand    *offsetOpnd = gcvNULL;

                        newOpnd = VIR_Inst_GetSource(newInst, 0);
                        offsetOpnd = VIR_Inst_GetSource(newInst, 1);

                        if (VIR_Operand_GetIsConstIndexing(newOpnd))
                        {
                            VIR_Operand_SetImmediateUint(offsetOpnd, VIR_Operand_GetConstIndexingImmed(newOpnd));
                        }
                        else if (VIR_Operand_GetRelAddrMode(newOpnd) != VIR_INDEXED_NONE)
                        {
                            VIR_Operand_SetSymbol(offsetOpnd, func, VIR_Operand_GetRelIndexing(newOpnd));
                            VIR_Operand_SetSwizzle(offsetOpnd, VIR_Enable_2_Swizzle_WShift((VIR_Enable)VIR_Operand_GetRelAddrMode(newOpnd)));
                        }
                        else if (VIR_Operand_GetRelAddrMode(newOpnd) == VIR_INDEXED_NONE)
                        {
                            VIR_Operand_SetImmediateUint(offsetOpnd, 0);
                        }
                        VIR_Operand_SetIsConstIndexing(newOpnd, 0);
                        VIR_Operand_SetRelAddrMode(newOpnd, 0);
                        VIR_Operand_SetMatrixConstIndex(newOpnd, 0);
                        VIR_Operand_SetRelAddrLevel(newOpnd, 0);
                        VIR_Operand_SetRelIndex(newOpnd, 0);
                    }
                }
                if (VIR_Symbol_isOutParam(parmSym))
                {
                    dstType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(destOpnd));

                    VIR_Function_AddInstructionAfter(pCallerFunc,
                                                     VIR_OP_MOV,
                                                     VIR_TYPE_UNKNOWN,
                                                     pCallerInst,
                                                     gcvTRUE,
                                                     &newInst);

                    VIR_Operand_SetTempRegister(newInst->src[0],
                                                pCallerFunc,
                                                VIR_Symbol_GetIndex(parmVregSym),
                                                VIR_Type_GetIndex(parmType));

                    VIR_Operand_SetEnable(newInst->dest, movEnable);

                    if (i < argCount)
                    {
                        gcmASSERT(opnd && !VIR_Operand_isUndef(opnd));
                        /* matrix and struct's output in parameter list is already lowered */
                        VIR_Operand_SetTempRegister(newInst->dest,
                                                    pCallerFunc,
                                                    VIR_Operand_GetSymbolId_(opnd),
                                                    VIR_Operand_GetTypeId(opnd));
                    }
                    else
                    {
                        /* matrix and struct's output is not lowered */
                        if (VIR_TypeId_isPrimitive(VIR_Operand_GetTypeId(destOpnd)) &&
                            VIR_GetTypeRows(VIR_Operand_GetTypeId(destOpnd)) == 1)
                        {
                            VIR_Operand_SetTempRegister(newInst->dest,
                                                        pCallerFunc,
                                                        VIR_Operand_GetSymbolId_(destOpnd),
                                                        VIR_Operand_GetTypeId(destOpnd));
                        }
                        else if ((VIR_TypeId_isPrimitive(VIR_Operand_GetTypeId(destOpnd)) &&
                                  VIR_GetTypeRows(VIR_Operand_GetTypeId(destOpnd)) > 1) ||
                                  VIR_Type_GetKind(dstType) == VIR_TY_STRUCT)
                        {
                            VIR_SymId           subVregId;
                            VIR_Symbol          *subVregSym;

                            subVregId = VIR_Symbol_GetVariableVregIndex(VIR_Operand_GetUnderlyingSymbol(destOpnd));
                            subVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, subVregId + i - opndParm->argNum);

                            /* make sure the order of parameters is the same as struct fields */
                            gcmASSERT(VIR_Symbol_GetType(subVregSym) == parmType);

                            VIR_Operand_SetTempRegister(newInst->dest,
                                                        pCallerFunc,
                                                        VIR_Symbol_GetIndex(subVregSym),
                                                        VIR_Type_GetIndex(parmType));
                        }
                        else
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }

                    VIR_Operand_SetSwizzle(newInst->src[0], VIR_Enable_2_Swizzle(movEnable));
                }
            }

            /* change the instruction to call */
            VIR_Inst_SetOpcode(pCallerInst, VIR_OP_CALL);
            VIR_Inst_SetConditionOp(pCallerInst, VIR_COP_ALWAYS);
            for (i=0; i < VIR_Inst_GetSrcNum(pCallerInst); i++)
            {
                if (VIR_Inst_GetSource(pCallerInst, i) != gcvNULL)
                {
                    VIR_Function_FreeOperand(pCallerFunc, VIR_Inst_GetSource(pCallerInst, i));
                    VIR_Inst_SetSource(pCallerInst, i, gcvNULL);
                }
            }
            VIR_Inst_SetSrcNum(pCallerInst, 0);
            VIR_Operand_SetFunction(pCallerInst->dest, pCalleeFunc);
        }
        else
        {
            VIR_Function*   pLibCalleeFunc = gcvNULL;

            pCalleeFunc = VIR_Operand_GetFunction(VIR_Inst_GetDest(pCallerInst));

            /* Update the instructions of callee function. */
            _UpdateResOpType(callerInstResOpType, pCalleeFunc);

            /*
            ** If this function is already existed in the main shader, we need to use the parameters of
            ** the lib function to search the argument assignments(in the temp hash table),
            ** and replace them with the parameters of the function in the main shader.
            */
            if (!vscHTBL_DirectTestAndGet(pAddLibFuncSet, (void *)pCalleeFunc, gcvNULL))
            {
                VIR_Shader_GetFunctionByName(pLibShader, VIR_Function_GetNameString(pCalleeFunc), &pLibCalleeFunc);
                gcmASSERT(pLibCalleeFunc != gcvNULL && pLibCalleeFunc != pCalleeFunc);
            }
            else
            {
                pLibCalleeFunc = pCalleeFunc;
            }

            /* Update call parameter assignments. */
            errCode = VIR_Shader_UpdateCallParmAssignment(pShader,
                                                          pCalleeFunc,
                                                          pLibCalleeFunc,
                                                          pCallerFunc,
                                                          pCallerInst,
                                                          gcvFALSE,
                                                          (pLibCalleeFunc != pCalleeFunc) ? Context->pTempHashTable : gcvNULL);
        }

        /* Free the call inst node. */
        vscMM_Free(pMM, pCallInstNode);
    }

    return errCode;
}

/* Intrinsic library list. */
void
VIR_Intrinsic_LibList_Initialize(
    IN  VSC_MM                   *pMM,
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList
    )
{
    VIR_Intrinsic_LibList_SetMM(pIntrinsicLibList, pMM);
    vscUNILST_Initialize(VIR_Intrinsic_LibList_GetList(pIntrinsicLibList), gcvFALSE);
}

void
VIR_Intrinsic_LibList_Finalize(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList
    )
{
    VSC_UNI_LIST                 *list = VIR_Intrinsic_LibList_GetList(pIntrinsicLibList);

    while (!vscUNILST_IsEmpty(list))
    {
        VSC_UNI_LIST_NODE *node = list->pHead;

        vscUNILST_RemoveHead(list);
        vscMM_Free(VIR_Intrinsic_LibList_GetMM(pIntrinsicLibList), node);
    }

    VIR_Intrinsic_LibList_SetMM(pIntrinsicLibList, gcvNULL);
    vscUNILST_Finalize(&pIntrinsicLibList->intrinsicLibList);
}

void
VIR_Intrinsic_LibList_AppendNode(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList,
    IN  VIR_Shader               *pIntrinsicLib,
    IN  VIR_Intrinsic_LibKind    libKind
    )
{
    VIR_Intrinsic_LibNode        *lib_node = (VIR_Intrinsic_LibNode *)vscMM_Alloc(VIR_Intrinsic_LibList_GetMM(pIntrinsicLibList),
                                                                                  sizeof(VIR_Intrinsic_LibNode));

    VIR_Intrinsic_LibNode_SetLib(lib_node, pIntrinsicLib);
    VIR_Intrinsic_LibNode_SetLibKind(lib_node, libKind);

    vscUNILST_Append(VIR_Intrinsic_LibList_GetList(pIntrinsicLibList), VIR_Intrinsic_LibNode_GetNode(lib_node));
}

VIR_Intrinsic_LibNode*
VIR_Intrinsic_LibList_GetNodeByLibKind(
    IN  VIR_Intrinsic_LibList    *pIntrinsicLibList,
    IN  VIR_Intrinsic_LibKind    libKind
    )
{
    VIR_Intrinsic_LibNode        *lib_node = gcvNULL;
    VSC_UL_ITERATOR              lib_iter;

    vscULIterator_Init(&lib_iter, VIR_Intrinsic_LibList_GetList(pIntrinsicLibList));

    for (lib_node = (VIR_Intrinsic_LibNode*)vscULIterator_First(&lib_iter);
         lib_node != gcvNULL;
         lib_node = (VIR_Intrinsic_LibNode*)vscULIterator_Next(&lib_iter))
    {
        if (VIR_Intrinsic_LibNode_GetLibKind(lib_node) == libKind)
        {
            return lib_node;
        }
    }

    return gcvNULL;
}

/******************************************************************************
 Functions for LinkLib and link lib functions
******************************************************************************/
static void
_TranspointsQueue(
    IN VSC_MM                   *pMM,
    IN VIR_TRANS_WORKLIST       *TranList,
    IN void                     *TranPoint
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(pMM,
        sizeof(VSC_UNI_LIST_NODE_EXT));

    vscULNDEXT_Initialize(worklistNode, TranPoint);
    QUEUE_PUT_ENTRY(TranList, worklistNode);
}

static void
_TranspointsDequeue(
    IN VSC_MM                   *pMM,
    IN VIR_TRANS_WORKLIST       *TranList,
    OUT void                    **TranPoint
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(TranList);

    *TranPoint = vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(pMM, worklistNode);
}

/*********** functions for intrinsic/extCall function name LinkLib *************/
static void
_GetIntrinsicOrExtFunc(
    IN VIR_LinkLibContext         *Context,
    OUT VIR_TRANS_WORKLIST        *Worklist
    )
{
    VIR_Shader              *pShader = Context->shader;
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode        *func_node;
    VIR_Function            *func;
    VSC_MM                  *pMM = Context->pMM;
    VSC_LIB_LINK_POINT      *linkPoint = Context->linkPoint;
    gctBOOL                  notLinkImageIntrinsic = (linkPoint->libLinkType == VSC_LIB_LINK_TYPE_FUNC_NAME) &&
                                                      VIR_Shader_IsPatchLib(pShader);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator            inst_iter;
        VIR_Instruction             *inst;
        VIR_LINKER_CALL_INST_NODE   *callInstNode;

        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_INTRINSIC)
            {
                VIR_Operand *   src0 = VIR_Inst_GetSource(inst, 0);
                VIR_Operand *   src1 = VIR_Inst_GetSource(inst, 1);
                VIR_ParmPassing * parmOpnd = VIR_Operand_GetParameters(src1);
                VIR_IntrinsicsKind ik = VIR_Operand_GetIntrinsicKind(src0);

                /* For some cases, we need to update the intrinsic kind. */
                if (VIR_Intrinsics_isImageRelated(ik) || VIR_Intrinsics_isImageAddr(ik))
                {
                    if (notLinkImageIntrinsic)
                    {
                        continue;
                    }
                    else
                    {
                        VIR_TypeId opndTypeId = VIR_Operand_GetTypeId(parmOpnd->args[0]);
                        VIR_TypeId symTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(VIR_Operand_GetSymbol(parmOpnd->args[0])));
                        /*
                        ** The source of image_fetch can be a image or a sampler:
                        ** 1) If it is a image, we convert it to a image_load.
                        ** 2) If it is a sampler, we convert it to a image_fetch_for_sampler.
                        */
                        if (VIR_Intrinsics_isImageFetch(ik))
                        {
                            if (VIR_TypeId_isSampler(opndTypeId) || VIR_TypeId_isSampler(symTypeId))
                            {
                                ik = VIR_IK_image_fetch_for_sampler;
                            }
                            else
                            {
                                ik = VIR_IK_image_load;
                            }
                            VIR_Operand_SetIntrinsicKind(src0, ik);
                        }
                    }
                }

                callInstNode = (VIR_LINKER_CALL_INST_NODE *) vscMM_Alloc(pMM, sizeof(VIR_LINKER_CALL_INST_NODE));
                callInstNode->inst = inst;
                callInstNode->u.libIntrinsicKind = ik;

                _TranspointsQueue(Context->pMM, Worklist, (void *) callInstNode);
            }
            else if (VIR_Inst_GetOpcode(inst) == VIR_OP_EXTCALL)
            {
                VIR_Operand *   src0 = VIR_Inst_GetSource(inst, 0);
                VIR_NameId nameId = VIR_Operand_GetNameId(src0);

                callInstNode = (VIR_LINKER_CALL_INST_NODE *) vscMM_Alloc(pMM, sizeof(VIR_LINKER_CALL_INST_NODE));
                callInstNode->inst = inst;
                callInstNode->u.extFuncName = nameId;

                _TranspointsQueue(Context->pMM, Worklist, (void *) callInstNode);
            }
        }
    }
}

static VSC_ErrCode
_GetIntrinsicOrextFuncName(
    IN  VIR_LinkLibContext          *Context,
    IN void                         *TransPoint,
    IN  gctSTRING                   *LibFuncName
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader                  *pShader = Context->shader;
    VSC_HW_CONFIG               *pHwCfg = Context->pHwCfg;
    VIR_LINKER_CALL_INST_NODE   *callInstNode = (VIR_LINKER_CALL_INST_NODE*) TransPoint;

    /* Get the intrisic function name. */
    _IntrisicOrExtFuncName(pShader,
                           pHwCfg,
                           callInstNode->inst,
                           LibFuncName);

    return errCode;
}

static VSC_ErrCode
_InsertIntrinsicFunc(
    IN VIR_LinkLibContext     *Context,
    IN void                   *TransPoint,
    IN VIR_Function           *pFunc
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_LINKER_CALL_INST_NODE   *callInstNode = (VIR_LINKER_CALL_INST_NODE*) TransPoint;
    VIR_Instruction             *pInst = callInstNode->inst;

    /* Set the function into this instruction. */
    VIR_Operand_SetFunction(VIR_Inst_GetSource(pInst, 0), pFunc);

    return errCode;
}

/*********** functions for output format LinkLib *************/
static void
_GetTranspointOutputFmt(
    IN VIR_LinkLibContext         *Context,
    OUT VIR_TRANS_WORKLIST        *Worklist
    )
{
    VIR_Shader              *pShader = Context->shader;
    VSC_LIB_LINK_POINT      *pLinkPoint = Context->linkPoint;
    VIR_OutputIdList        *pOutputs = VIR_Shader_GetOutputs(pShader);
    gctUINT                 curOutput;

    for (curOutput = 0; curOutput < VIR_IdList_Count(pOutputs); curOutput ++)
    {
        VIR_Symbol  *output = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputs, curOutput));

        if (output->layout.location == pLinkPoint->u.clrOutput.location)
        {
            _TranspointsQueue(Context->pMM, Worklist, (void *) output);
            break;
        }
    }
}

static VSC_ErrCode _InsertInstAtEoMF(IN  VIR_Function *  Function,
                                     IN  VIR_OpCode      Opcode,
                                     OUT VIR_Instruction **Inst)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Instruction         *endInst = VIR_Function_GetInstEnd(Function);

    if (endInst->_opcode == VIR_OP_RET)
    {
        errCode = VIR_Function_AddInstructionBefore(Function, Opcode, VIR_TYPE_UNKNOWN, endInst, gcvTRUE, Inst);
    }
    else
    {
        errCode = VIR_Function_AddInstruction(Function, Opcode, VIR_TYPE_UNKNOWN, Inst);
    }

    return errCode;
}

/* insert the argument passing and call to the libfunc,
   and mov back the return value
   at the end of the main function */
static VSC_ErrCode
_InsertCallOutputFmt(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader              *pShader = Context->shader;
    VIR_Function            *pFunc = VIR_Shader_GetMainFunction(pShader);
    VSC_LIB_LINK_POINT      *pLinkPoint = Context->linkPoint;
    VIR_Symbol              *output = (VIR_Symbol *) Transpoint;
    VIR_Instruction         *newInst = gcvNULL;
    VIR_SymId               outputVregId, parmSymId, parmVregId;
    VIR_Symbol              *outputVreg, *parmSym, *parmVregSym;
    gctUINT                 i;
    VIR_TypeId              symTy;
    VIR_Enable              movEnable = VIR_ENABLE_NONE;

    gcmASSERT(VIR_Symbol_isOutput(output));

    symTy = VIR_Type_GetIndex(VIR_Symbol_GetType(output));

    /* insert the MOV to pass arguement
       MOV arg1, output */
    errCode =_InsertInstAtEoMF(pFunc, VIR_OP_MOV, &newInst);
    ON_ERROR(errCode, "_InsertCallOutputFmt");

    outputVregId = VIR_Symbol_GetVariableVregIndex(output);
    outputVreg = VIR_Shader_FindSymbolByTempIndex(pShader, outputVregId);

    /* assume input is the first parameter */
    parmSymId = VIR_IdList_GetId(&LibFunc->paramters, 0);
    parmSym = VIR_Function_GetSymFromId(LibFunc, parmSymId);
    gcmASSERT(VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INPARM ||
              VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM);
    parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);
    parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);

    VIR_Operand_SetTempRegister(newInst->dest,
                                pFunc,
                                VIR_Symbol_GetIndex(parmVregSym),
                                symTy);
    movEnable = VIR_Type_Conv2Enable(VIR_Symbol_GetType(outputVreg));
    VIR_Operand_SetEnable(newInst->dest, movEnable);

    VIR_Operand_SetTempRegister(newInst->src[0],
                                pFunc,
                                VIR_Symbol_GetIndex(outputVreg),
                                symTy);
    VIR_Operand_SetSwizzle(newInst->src[0], VIR_Enable_2_Swizzle(movEnable));

    if (VIR_Symbol_GetComponents(output) < 4)
    {
        VIR_TypeId  newTy = symTy;
        VIR_Enable  newEnable = VIR_ENABLE_NONE;
        switch (VIR_Symbol_GetComponents(output))
        {
        case 1:
            newEnable = VIR_ENABLE_YZW;
            newTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(symTy), 3, 1);
            break;
        case 2:
            newEnable = VIR_ENABLE_ZW;
            newTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(symTy), 2, 1);
            break;
        case 3:
            newEnable = VIR_ENABLE_W;
            newTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(symTy), 1, 1);
            break;
        default:
            gcmASSERT(gcvFALSE);
            newEnable = VIR_ENABLE_XYZW;
            break;
        }

        /* since we assume the color output is a vec4, to avoid undefined other channel
           we generate MOV arg1, 0 */
        errCode = _InsertInstAtEoMF(pFunc, VIR_OP_MOV, &newInst);
        ON_ERROR(errCode, "_InsertCallOutputFmt");

        VIR_Operand_SetTempRegister(newInst->dest,
                                    pFunc,
                                    VIR_Symbol_GetIndex(parmVregSym),
                                    newTy);
        VIR_Operand_SetEnable(newInst->dest, newEnable);
        VIR_Operand_SetImmediateInt(newInst->src[0], 0);
    }

    /* insert the call instruction */
    errCode = _InsertInstAtEoMF(pFunc, VIR_OP_CALL, &newInst);
    ON_ERROR(errCode, "_InsertCallOutputFmt");

    VIR_Inst_SetConditionOp(newInst, VIR_COP_ALWAYS);
    VIR_Operand_SetFunction(newInst->dest, LibFunc);

    /* insert the MOV to get the return value
       MOV output, arg[] */
    for (i = 0; i < (gctUINT) pLinkPoint->u.clrOutput.layers; i++)
    {
        errCode = _InsertInstAtEoMF(pFunc, VIR_OP_MOV, &newInst);
        ON_ERROR(errCode, "_InsertCallOutputFmt");

        /* add an output for layer > 1 */
        if (i >= 1)
        {
            VIR_NameId      nameId;
            VIR_SymId       symId;
            VIR_VirRegId    regId;
            VIR_Symbol      *outputSym;

            gctCONST_STRING    outputName    = VIR_Shader_GetSymNameString(pShader, output);
            gctCHAR            name[256];
            gctUINT            offset   = 0;

            gcoOS_PrintStrSafe(name, sizeof(name), &offset,
                               "%s_layer%d", outputName, i+1);

            errCode = VIR_Shader_AddString(pShader,
                                 name,
                                 &nameId);
            ON_ERROR(errCode, "_InsertCallOutputFmt");

            errCode = VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VARIABLE,
                                 nameId,
                                 VIR_Symbol_GetType(output),
                                 VIR_STORAGE_OUTPUT,
                                 &symId);
            ON_ERROR(errCode, "_InsertCallOutputFmt");

            outputSym = VIR_Shader_GetSymFromId(pShader, symId);

            VIR_Symbol_SetTyQualifier(outputSym, VIR_TYQUAL_NONE);
            VIR_Symbol_SetPrecision(outputSym, VIR_PRECISION_HIGH);

            VIR_Symbol_SetFlag(outputSym, VIR_Symbol_GetFlag(output));
            VIR_Symbol_SetFlag(outputSym, VIR_SYMFLAG_COMPILER_GEN);

            /* set layout info */
            VIR_Symbol_SetLayoutQualifier(outputSym, VIR_LAYQUAL_NONE);
            VIR_Symbol_SetLocation(outputSym, NOT_ASSIGNED);
            VIR_Symbol_SetMasterLocation(outputSym, output->layout.location);

            /* generate the vreg */
            regId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VIRREG,
                                 regId,
                                 VIR_Symbol_GetType(output),
                                 VIR_STORAGE_UNKNOWN,
                                 &outputVregId);
            ON_ERROR(errCode, "_InsertCallOutputFmt");

            outputVreg = VIR_Shader_GetSymFromId(pShader, outputVregId);

            VIR_Symbol_SetVregVariable(outputVreg, outputSym);

            VIR_Symbol_SetVariableVregIndex(outputSym, regId);
        }

        VIR_Operand_SetTempRegister(newInst->dest,
                                    pFunc,
                                    VIR_Symbol_GetIndex(outputVreg),
                                    symTy);

        VIR_Operand_SetEnable(newInst->dest, movEnable);

        /* we assume output is the second parameter */
        parmSymId = VIR_IdList_GetId(&LibFunc->paramters, 1);
        parmSym = VIR_Function_GetSymFromId(LibFunc, parmSymId);
        gcmASSERT(VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_OUTPARM ||
              VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM);
        parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym) + i;
        parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);

        gcmASSERT(parmVregSym != gcvNULL);

        VIR_Operand_SetTempRegister(newInst->src[0],
                                    pFunc,
                                    VIR_Symbol_GetIndex(parmVregSym),
                                    symTy);

        VIR_Operand_SetSwizzle(newInst->src[0], VIR_Enable_2_Swizzle(movEnable));
    }

OnError:
    return errCode;
}

/*********** functions for texld format LinkLib *************/
extern VSC_RES_OP_BIT _VirResOpType2DrviResOpBit(gctUINT resOpType);

static VIR_Symbol*
_FindSamplerAssignmentSymbol(
    IN VSC_LIB_LINK_POINT         *pLinkPoint,
    IN VIR_Shader                 *pShader,
    IN VIR_Instruction            *pInst,
    IN VIR_VirRegId               virRegId
    )
{
    VIR_Symbol                    *pSamplerUniform = gcvNULL;
    VIR_Instruction               *pInstIter = gcvNULL;
    VIR_Operand                   *pDest = gcvNULL;
    VIR_OpCode                    opCode;

    for (pInstIter = VIR_Inst_GetPrev(pInst);
         pInstIter && (VIR_Inst_GetFunction(pInstIter) == VIR_Inst_GetFunction(pInst));
         pInstIter = VIR_Inst_GetPrev(pInstIter))
    {
        opCode = VIR_Inst_GetOpcode(pInstIter);
        if (VIR_OPCODE_hasDest(opCode))
        {
            pDest = VIR_Inst_GetDest(pInstIter);

            if (VIR_Operand_isSymbol(pDest) &&
                VIR_Symbol_GetVregIndex(VIR_Operand_GetSymbol(pDest)) == virRegId)
            {
                gcmASSERT(opCode == VIR_OP_GET_SAMPLER_IDX);

                pSamplerUniform = VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInstIter, 0));
                break;
            }
        }
    }

    return pSamplerUniform;
}

static gctBOOL
_CheckTexldSymbolFmt(
    IN VSC_LIB_LINK_POINT         *pLinkPoint,
    IN VIR_Shader                 *pShader,
    IN VIR_Instruction            *pInst,
    IN VIR_Operand                *pSrcOpnd,
    IN VIR_Symbol                 *pSym,
    IN VSC_RES_OP_BIT              resOpBit
    )
{
    gctBOOL matched = gcvFALSE;

    /* If this is a baseSampler+offset, we need to check the offset. */
    if (VIR_Symbol_GetIndex(pSym) == VIR_Shader_GetBaseSamplerId(pShader) &&
        VIR_Operand_GetRelAddrMode(pSrcOpnd) != VIR_INDEXED_NONE)
    {
        pSym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(pSrcOpnd));

        pSym = _FindSamplerAssignmentSymbol(pLinkPoint, pShader, pInst, VIR_Symbol_GetVregIndex(pSym));
        if (!pSym)
        {
            return matched;
        }
    }

    if (pLinkPoint->u.resource.set == VIR_Symbol_GetDescriptorSet(pSym) &&
        pLinkPoint->u.resource.binding == VIR_Symbol_GetBinding(pSym)   &&
        ((pLinkPoint->u.resource.opTypeBits & resOpBit) != 0))
    {
        matched = gcvTRUE;
    }
    /* If this symbol is a sampled image, check the SeparateSampler/SeparateImage. */
    else if (VIR_Symbol_isSampler(pSym) &&
             VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_SAMPLED_IMAGE)
    {
        VIR_Symbol*   separateSamplerSym = gcvNULL;
        VIR_Symbol*   separateImageSym = gcvNULL;

        if (VIR_Symbol_GetSeparateSampler(pSym) != VIR_INVALID_ID)
        {
            separateSamplerSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetSeparateSampler(pSym));

            matched = _CheckTexldSymbolFmt(pLinkPoint, pShader, pInst, pSrcOpnd, separateSamplerSym, resOpBit);
        }

        if (!matched && VIR_Symbol_GetSeparateImage(pSym) != VIR_INVALID_ID)
        {
            separateImageSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetSeparateImage(pSym));
            matched = _CheckTexldSymbolFmt(pLinkPoint, pShader, pInst, pSrcOpnd, separateImageSym, resOpBit);
        }
    }
    /*
    ** VIV:TODO: if this resource is an array, do we need to check the array index(static index or dynamic index)?
    ** If the array elements have different link type, we can't handle it right now.
    */

    return matched;
}

static void
_GetTranspointTexldFmt(
    IN VIR_LinkLibContext         *Context,
    OUT VIR_TRANS_WORKLIST        *Worklist
    )
{
    VIR_Shader              *pShader = Context->shader;
    VSC_LIB_LINK_POINT      *pLinkPoint = Context->linkPoint;

    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;
    VIR_Function        *func;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator    inst_iter;
        VIR_Instruction     *inst;
        VIR_Operand         *srcOpnd;
        VIR_Symbol          *srcSym;
        VSC_RES_OP_BIT      resOpBit;

        func = func_node->function;

        /* we don't need to go through the library functions */
        if ((func->flags & VIR_FUNCFLAG_LINKED_LIB) == 0)
        {
            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                 inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                /* find the texld to patch */
                if (VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)))
                {
                    srcOpnd = VIR_Inst_GetSource(inst, 0);

                    if (VIR_Operand_isSymbol(srcOpnd))
                    {
                        srcSym = VIR_Operand_GetSymbol(srcOpnd);
                        resOpBit = _VirResOpType2DrviResOpBit(VIR_Inst_GetResOpType(inst));

                        if (_CheckTexldSymbolFmt(pLinkPoint, pShader, inst, srcOpnd, srcSym, resOpBit))
                        {
                            _TranspointsQueue(Context->pMM, Worklist, (void *) inst);
                        }
                    }
                }
            }
        }
    }
}

static VSC_ErrCode
_InsertMovToArgs(
    IN VIR_Shader               *pShader,
    IN VIR_Function             *pFunc,
    IN VIR_Function             *pCalleeFunc,
    IN gctUINT                  ParmIdx,
    IN VIR_Instruction          *MeInst,
    OUT VIR_Instruction         **newInst
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_SymId               parmSymId, parmVregId;
    VIR_Symbol              *parmSym, *parmVregSym;
    VIR_TypeId              symTy;

    errCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_MOV,
        VIR_TYPE_UNKNOWN,
        MeInst,
        gcvTRUE,
        newInst);
    ON_ERROR(errCode, "_InsertMovToArgs");

    parmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, ParmIdx);
    parmSym = VIR_Function_GetSymFromId(pCalleeFunc, parmSymId);
    gcmASSERT(VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INPARM ||
                VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM);
    parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);
    parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);
    symTy = VIR_Symbol_GetTypeId(parmSym);

    VIR_Operand_SetTempRegister((*newInst)->dest,
                                pFunc,
                                VIR_Symbol_GetIndex(parmVregSym),
                                symTy);

    VIR_Operand_SetEnable((*newInst)->dest, VIR_TypeId_Conv2Enable(symTy));

OnError:
    return errCode;
}

static VSC_ErrCode
_InsertMovFromArgs(
    IN VIR_Shader               *pShader,
    IN VIR_Function             *pFunc,
    IN VIR_Function             *pCalleeFunc,
    IN gctUINT                  ParmIdx,
    IN VIR_Instruction          *MeInst,
    OUT VIR_Instruction         **newInst
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_SymId               parmSymId, parmVregId;
    VIR_Symbol              *parmSym, *parmVregSym;
    VIR_TypeId              symTy;

    errCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_MOV,
        VIR_TYPE_UNKNOWN,
        MeInst,
        gcvTRUE,
        newInst);
    ON_ERROR(errCode, "_InsertMovFromArgs");

    parmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, ParmIdx);
    parmSym = VIR_Function_GetSymFromId(pCalleeFunc, parmSymId);
    gcmASSERT(VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_OUTPARM ||
                VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM);
    parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);
    parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);
    symTy = VIR_Symbol_GetTypeId(parmSym);

    VIR_Operand_SetTempRegister((*newInst)->src[0],
                                pFunc,
                                VIR_Symbol_GetIndex(parmVregSym),
                                symTy);

    VIR_Operand_SetSwizzle((*newInst)->src[0],
        VIR_Enable_2_Swizzle_WShift(VIR_TypeId_Conv2Enable(symTy)));

OnError:
    return errCode;
}

static VSC_ErrCode
_AddExtraSampler(
    IN VIR_Shader       *pShader,
    IN VIR_Function     *pFunc,
    IN VIR_Operand      *samplerOpnd,
    IN gctUINT          arrayIndex,
    OUT VIR_Operand     **newOpnd
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Symbol          *samplerSym = VIR_Operand_GetSymbol(samplerOpnd);
    VIR_Uniform         *sampler = VIR_Symbol_GetSampler(samplerSym);
    VIR_Symbol          *extraLayerSym;
    VIR_Uniform         *extraLayer;
    VIR_SymId           extraLayerSymId = sampler->u.samplerOrImageAttr.extraImageLayer;

    gcmASSERT(sampler);

    if (extraLayerSymId == VIR_INVALID_ID)
    {
        VIR_NameId      nameId;
        gctCHAR         name[128] = "#";

        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerSym));
        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$ExtraLayer");
        errCode = VIR_Shader_AddString(pShader,
                                        name,
                                        &nameId);
        ON_ERROR(errCode, "VIR_Shader_AddString");

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_SAMPLER,
                                       nameId,
                                       VIR_Symbol_GetType(samplerSym),
                                       VIR_STORAGE_UNKNOWN,
                                       &extraLayerSymId);

        ON_ERROR(errCode, "VIR_Shader_AddSymbol");

        extraLayerSym = VIR_Shader_GetSymFromId(pShader, extraLayerSymId);
        sampler->u.samplerOrImageAttr.extraImageLayer = extraLayerSymId;
        VIR_Symbol_SetFlag(extraLayerSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(extraLayerSym, VIR_Symbol_GetPrecision(samplerSym));
        VIR_Symbol_SetUniformKind(extraLayerSym, VIR_UNIFORM_EXTRA_LAYER);
        VIR_Symbol_SetAddrSpace(extraLayerSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(extraLayerSym, VIR_Symbol_GetTyQualifier(samplerSym));
        extraLayerSym->layout = samplerSym->layout;

        extraLayer = VIR_Symbol_GetSampler(extraLayerSym);
        extraLayer->u.samplerOrImageAttr.parentSamplerSymId = sampler->sym;
        extraLayer->u.samplerOrImageAttr.arrayIdxInParent = arrayIndex;
        extraLayer->u.samplerOrImageAttr.texelBufferToImageSymId = NOT_ASSIGNED;
    }

    /* New a operand with this extra image layer.  */
    errCode = VIR_Function_NewOperand(pFunc, newOpnd);
    ON_ERROR(errCode, "VIR_Function_NewOperand");

    VIR_Operand_SetSymbol(*newOpnd, pFunc, extraLayerSymId);
    VIR_Operand_SetTypeId(*newOpnd, VIR_Operand_GetTypeId(samplerOpnd));
    VIR_Operand_SetSwizzle(*newOpnd, VIR_SWIZZLE_XYZW);
    VIR_Operand_SetRoundMode(*newOpnd, VIR_ROUND_DEFAULT);
    VIR_Operand_SetModifier(*newOpnd, VIR_MOD_NONE);

    /* Copy the index from image operand. */
    VIR_Operand_SetIsConstIndexing(*newOpnd, VIR_Operand_GetIsConstIndexing(samplerOpnd));
    VIR_Operand_SetRelIndex(*newOpnd, VIR_Operand_GetRelIndexing(samplerOpnd));
    VIR_Operand_SetRelAddrMode(*newOpnd, VIR_Operand_GetRelAddrMode(samplerOpnd));
    VIR_Operand_SetMatrixConstIndex(*newOpnd, VIR_Operand_GetMatrixConstIndex(samplerOpnd));
    VIR_Operand_SetRelAddrLevel(*newOpnd, VIR_Operand_GetRelAddrLevel(samplerOpnd));

OnError:
    return errCode;
}

static gctUINT
_texldInstMod(
    IN VIR_Instruction      *pInst)
{
    gctUINT retValue = TEXLDMOD_NONE;

    switch (VIR_Inst_GetResOpType(pInst))
    {
    case VIR_RES_OP_TYPE_TEXLD_BIAS:
    case VIR_RES_OP_TYPE_TEXLDP_BIAS:
        retValue = TEXLDMOD_BIAS;
        break;

    case VIR_RES_OP_TYPE_TEXLD_LOD:
    case VIR_RES_OP_TYPE_TEXLDP_LOD:
        retValue = TEXLDMOD_LOD;
        break;

    case VIR_RES_OP_TYPE_GATHER:
    case VIR_RES_OP_TYPE_GATHER_PCF:
        retValue = TEXLDMOD_GATHER;
        break;

    /* Share with LOD. */
    case VIR_RES_OP_TYPE_FETCH:
    case VIR_RES_OP_TYPE_FETCH_MS:
        retValue = TEXLDMOD_LOD;
        break;

    default:
        retValue = TEXLDMOD_NONE;
        break;
    }

    return retValue;
}

static gctUINT
_texldInstType(
    IN VIR_LinkLibContext   *Context,
    IN VIR_Instruction      *pInst
    )
{
    gctBOOL supportTexldU = Context->pHwCfg->hwFeatureFlags.hasUniversalTexldV2 && Context->pHwCfg->hwFeatureFlags.hasTexldUFix;
    gctUINT retValue = TEXLDTYPE_NORMAL;

    switch (VIR_Inst_GetResOpType(pInst))
    {
    case VIR_RES_OP_TYPE_TEXLD:
    case VIR_RES_OP_TYPE_TEXLD_GRAD:
    case VIR_RES_OP_TYPE_TEXLD_BIAS:
    case VIR_RES_OP_TYPE_TEXLD_LOD:
        retValue = TEXLDTYPE_NORMAL;
        break;
    case VIR_RES_OP_TYPE_TEXLDP:
    case VIR_RES_OP_TYPE_TEXLDP_GRAD:
    case VIR_RES_OP_TYPE_TEXLDP_BIAS:
    case VIR_RES_OP_TYPE_TEXLDP_LOD:
        retValue = TEXLDTYPE_PROJ;
        break;
    case VIR_RES_OP_TYPE_GATHER:
        retValue = TEXLDTYPE_GATHER;
        break;
    case VIR_RES_OP_TYPE_GATHER_PCF:
        retValue = TEXLDTYPE_GATHERPCF;
        break;
    case VIR_RES_OP_TYPE_FETCH:
        /*
        ** If this chip can't support TEXLDU, then the coordinate has been changed to the floating point,
        ** we just need to treat it as a normal TEXLD.
        */
        if (supportTexldU)
        {
            retValue = TEXLDTYPE_U;
        }
        break;
    case VIR_RES_OP_TYPE_FETCH_MS:
        retValue = TEXLDTYPE_FETCHMS;
        break;
    default:
        retValue = TEXLDTYPE_U;
        break;
    }

    return retValue;
}

static Vir_TexldModifier_Name instMod2TexldMod(gctUINT instMod)
{
    switch (instMod)
    {
    case TEXLDMOD_BIAS:
        return VIR_TEXLDMODIFIER_BIAS;
    case TEXLDMOD_LOD:
        return VIR_TEXLDMODIFIER_LOD;
    default:
        break;
    }

    return VIR_TEXLDMODIFIER_COUNT;
}

static gctSTRING _GetLibFuncParam(IN VIR_Function             *pCalleeFunc,
                                  IN gctUINT                   ParmIdx)
{
    VIR_SymId               parmSymId;
    VIR_Symbol              *parmSym;

    parmSymId = VIR_IdList_GetId(&pCalleeFunc->paramters, ParmIdx);
    parmSym = VIR_Function_GetSymFromId(pCalleeFunc, parmSymId);

    if (VIR_Symbol_GetName(parmSym) > VIR_NAME_BUILTIN_LAST)
    {
        return VIR_Shader_GetSymNameString(pCalleeFunc->hostShader, parmSym);
    }
    else
    {
        return "";
    }
}

/* change texldInst to the call instruction */
static void _ChangeTexldToCall(
    VIR_Instruction     *texldInst,
    VIR_Function        *libFunc )
{
    gctUINT i;

    VIR_Inst_SetOpcode(texldInst, VIR_OP_CALL);
    VIR_Inst_SetConditionOp(texldInst, VIR_COP_ALWAYS);
    VIR_Operand_SetFunction(texldInst->dest, libFunc);

    for (i = 0; i < VIR_Inst_GetSrcNum(texldInst); i++)
    {
        if (VIR_Inst_GetSource(texldInst, i) != gcvNULL)
        {
            VIR_Inst_FreeSource(texldInst, i);
        }
    }

    VIR_Inst_SetSrcNum(texldInst, 0);
}

/* insert the argument passing and call to the libfunc, and mov back the return value */
/* _inputcvt_R32G32B32A32SINT_2_R32G32SINT(isampler3D origSampler,
                                           vec4 coord,
                                           int mod,
                                           float lod_bias,
                                           int type,
                                           isampler3D extraSampler,
                                           ivec4 swizzles)
*/
static VSC_ErrCode
_InsertCallTexld(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode                      errCode = VSC_ERR_NONE;
    VIR_Shader                       *pShader = Context->shader;
    VIR_Instruction                  *texldInst = (VIR_Instruction *) Transpoint;
    VIR_Function                     *pFunc = VIR_Inst_GetFunction(texldInst);
    VIR_Instruction                  *newInst = gcvNULL;
    VSC_LIB_SPECIALIZATION_CONSTANT  *specializationConst;

    VIR_Operand                      *texldSrc = gcvNULL, *newOpnd = gcvNULL;
    gctUINT                          argIdx = 0, i;
    gctUINT                          instMod = _texldInstMod(texldInst);
    Vir_TexldModifier_Name           texldMod = instMod2TexldMod(instMod);
    gctSTRING                        paramName;
    gctBOOL                          paraFound = gcvFALSE;

    gcmASSERT(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(texldInst)));

    /* insert the MOV to pass arguement
       MOV arg1, sampler
       MOV arg2, coord */
    for (argIdx = 0; argIdx < 2; argIdx++)
    {
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallTexld");

        texldSrc = VIR_Inst_GetSource(texldInst, argIdx);

        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), texldSrc);
    }

    /* mod */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    VIR_Operand_SetImmediateInt(newInst->src[0], instMod);

    /* lod_bias */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    if (texldMod < VIR_TEXLDMODIFIER_COUNT)
    {
        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0),
                         VIR_Operand_GetTexldModifier((VIR_Operand *)VIR_Inst_GetSource(texldInst, 2), texldMod));
    }
    else
    {
        if (VIR_Operand_isUndef(VIR_Inst_GetSource(texldInst, 2)))
        {
            VIR_Operand_SetImmediateFloat(VIR_Inst_GetSource(newInst, 0), 0.0);
        }
        else
        {
            VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Inst_GetSource(texldInst, 2));
        }
    }

    /* type */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    VIR_Operand_SetImmediateInt(newInst->src[0], _texldInstType(Context, texldInst));

    /* create extra sampler */
    if (Context->linkPoint->u.resource.actBits & VSC_RES_ACT_BIT_EXTRA_SAMPLER)
    {
        errCode = _AddExtraSampler(pShader, pFunc, VIR_Inst_GetSource(texldInst, 0),
            Context->linkPoint->u.resource.arrayIndex, &newOpnd);
        ON_ERROR(errCode, "_InsertCallTexld");
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallTexld");
        newInst->src[0] = newOpnd;
    }
    else
    {
        argIdx++;
    }

    /* swizzle */
    paramName = _GetLibFuncParam(LibFunc, argIdx);
    for (i = 0; i < Context->libSpecializationConstantCount; i ++)
    {
        specializationConst = &Context->libSpecializationConsts[i];

        if (gcmIS_SUCCESS(gcoOS_StrCmp(specializationConst->varName, paramName)) &&
            specializationConst->type == VSC_SHADER_DATA_TYPE_IVEC4)
        {
            VIR_ConstVal    new_const_val;
            VIR_ConstId     new_const_id;
            VIR_Const       *new_const;

            errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
            ON_ERROR(errCode, "_InsertCallTexld");

            new_const_val.vecVal.u32Value[0] = specializationConst->value.iValue[0];
            new_const_val.vecVal.u32Value[1] = specializationConst->value.iValue[1];
            new_const_val.vecVal.u32Value[2] = specializationConst->value.iValue[2];
            new_const_val.vecVal.u32Value[3] = specializationConst->value.iValue[3];

            VIR_Shader_AddConstant(pShader,
                                   VIR_TYPE_UINT_X4, &new_const_val, &new_const_id);
            new_const = VIR_Shader_GetConstFromId(pShader, new_const_id);
            new_const->type = VIR_TYPE_UINT_X4;
            VIR_Operand_SetConstId(newInst->src[0], new_const_id);
            VIR_Operand_SetOpKind(newInst->src[0], VIR_OPND_CONST);
            VIR_Operand_SetTypeId(newInst->src[0], VIR_TYPE_UINT_X4);
            VIR_Operand_SetSwizzle(newInst->src[0], VIR_SWIZZLE_XYZW);

            paraFound = gcvTRUE;
            break;
        }
    }
    if (!paraFound)
    {
        gcmASSERT(gcvFALSE);
    }

    /* insert the MOV to get the return value
       MOV destination, arg[7] */
    errCode = _InsertMovFromArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), texldInst->dest);

    /* change texldInst to the call instruction */
    _ChangeTexldToCall(texldInst, LibFunc);

OnError:
    return errCode;
};

/* vec4 _inputgather_R32SINT(isamplerCubeArray origSampler,
                             vec4 coord,
                             int mod,
                             isamplerCubeArray extraSampler
                             ivec4 swizzles)
*/
static VSC_ErrCode
_InsertCallTexldGather(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader              *pShader = Context->shader;
    VIR_Instruction         *texldInst = (VIR_Instruction *) Transpoint;
    VIR_Function            *pFunc = VIR_Inst_GetFunction(texldInst);
    VIR_Instruction         *newInst = gcvNULL;

    VIR_Operand             *texldSrc = gcvNULL;
    gctUINT                 argIdx, i;
    gctSTRING               paramName;
    VSC_LIB_SPECIALIZATION_CONSTANT  *specializationConst;
    gctBOOL                 paraFound = gcvFALSE;

    /* insert the MOV to pass arguement
    MOV arg1, sampler
    MOV arg2, coord */
    for (argIdx = 0; argIdx < 2; argIdx++)
    {
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallTexldGather");

        texldSrc = VIR_Inst_GetSource(texldInst, argIdx);

        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), texldSrc);
    }

    /* mod */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexldGather");
    texldSrc = VIR_Inst_GetSource(texldInst, 2);
    gcmASSERT(VIR_Operand_GetTexldGather_comp(texldSrc));
    VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Operand_GetTexldGather_comp(texldSrc));

    /* extraSampler (may not be needed)*/
    if (Context->linkPoint->u.resource.actBits & VSC_RES_ACT_BIT_EXTRA_SAMPLER)
    {
        /* create extra sampler */
        errCode = _AddExtraSampler(pShader, pFunc, VIR_Inst_GetSource(texldInst, 0),
            Context->linkPoint->u.resource.arrayIndex, &texldSrc);
        ON_ERROR(errCode, "_InsertCallTexldGather");
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallTexldGather");
        newInst->src[0] = texldSrc;
    }
    else
    {
        argIdx++;
    }

    /* swizzle */
    paramName = _GetLibFuncParam(LibFunc, argIdx);
    for (i = 0; i < Context->libSpecializationConstantCount; i++)
    {
        specializationConst = &Context->libSpecializationConsts[i];

        if (gcmIS_SUCCESS(gcoOS_StrCmp(specializationConst->varName, paramName)) &&
            specializationConst->type == VSC_SHADER_DATA_TYPE_IVEC4)
        {
            VIR_ConstVal    new_const_val;
            VIR_ConstId     new_const_id;
            VIR_Const       *new_const;

            errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
            ON_ERROR(errCode, "_InsertCallTexldGather");

            new_const_val.vecVal.u32Value[0] = specializationConst->value.iValue[0];
            new_const_val.vecVal.u32Value[1] = specializationConst->value.iValue[1];
            new_const_val.vecVal.u32Value[2] = specializationConst->value.iValue[2];
            new_const_val.vecVal.u32Value[3] = specializationConst->value.iValue[3];

            VIR_Shader_AddConstant(pShader,
                VIR_TYPE_UINT_X4, &new_const_val, &new_const_id);
            new_const = VIR_Shader_GetConstFromId(pShader, new_const_id);
            new_const->type = VIR_TYPE_UINT_X4;
            VIR_Operand_SetConstId(newInst->src[0], new_const_id);
            VIR_Operand_SetOpKind(newInst->src[0], VIR_OPND_CONST);
            VIR_Operand_SetTypeId(newInst->src[0], VIR_TYPE_UINT_X4);
            VIR_Operand_SetSwizzle(newInst->src[0], VIR_SWIZZLE_XYZW);

            paraFound = gcvTRUE;

            break;
        }
    }

    if (!paraFound)
    {
        gcmASSERT(gcvFALSE);
    }

    /* insert the MOV to get the return value
       MOV destination, arg[5] */
    errCode = _InsertMovFromArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), texldInst->dest);

     /* change texldInst to the call instruction */
    _ChangeTexldToCall(texldInst, LibFunc);

OnError:
    return errCode;
};

/* insert the argument passing and call to the libfunc, and mov back the return value */
static VSC_ErrCode
_InsertCallTexldGatherPCF(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode                      errCode = VSC_ERR_NONE;
    VIR_Shader                       *pShader = Context->shader;
    VIR_Instruction                  *texldInst = (VIR_Instruction *)Transpoint;
    VIR_Function                     *pFunc = VIR_Inst_GetFunction(texldInst);
    VIR_Instruction                  *newInst = gcvNULL;
    VSC_LIB_SPECIALIZATION_CONSTANT  *specializationConst;

    VIR_Operand                      *texldSrc = gcvNULL;
    gctUINT                          argIdx = 0, i;
    gctSTRING                        paramName;
    gctBOOL                          paraFound = gcvFALSE;

    gcmASSERT(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(texldInst)));

    /* vec4 _inputgather_pcf_D32SFLOAT(samplerCubeArray origSampler,
                                  vec4 coord,
                                  int type,
                                  int mod,
                                  int comparemode,
                                  float refZ,
                                  ivec4 swizzles) */

    /* insert the MOV to pass arguement
    MOV arg1, sampler
    MOV arg2, coord */
    for (argIdx = 0; argIdx < 2; argIdx++)
    {
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallTexld");

        texldSrc = VIR_Inst_GetSource(texldInst, argIdx);

        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), texldSrc);
    }

    /* type */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    texldSrc = VIR_Inst_GetSource(texldInst, 2);

    if (VIR_Operand_GetTexldGather_comp(texldSrc))
    {
        VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(newInst, 0), TEXLDTYPE_GATHER);
    }
    else
    {
        VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(newInst, 0), TEXLDTYPE_NORMAL);
    }

    /* mod */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    texldSrc = VIR_Inst_GetSource(texldInst, 2);

    if (VIR_Operand_GetTexldGather_comp(texldSrc))
    {
        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Operand_GetTexldGather_comp(texldSrc));
    }
    else
    {
        VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(newInst, 0), 0);
    }

    /* comparemode */
    paramName = _GetLibFuncParam(LibFunc, argIdx);
    for (i = 0; i < Context->libSpecializationConstantCount; i++)
    {
        specializationConst = &Context->libSpecializationConsts[i];

        if (gcmIS_SUCCESS(gcoOS_StrCmp(specializationConst->varName, paramName)) &&
            specializationConst->type == VSC_SHADER_DATA_TYPE_IVEC4)
        {
            errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
            ON_ERROR(errCode, "_InsertCallTexld");

            VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(newInst, 0), specializationConst->value.iValue[0]);

            paraFound = gcvTRUE;
            break;
        }
    }
    if (!paraFound)
    {
        gcmASSERT(gcvFALSE);
    }

    /* refZ */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexld");
    texldSrc = VIR_Inst_GetSource(texldInst, 2);
    gcmASSERT(VIR_Operand_GetTexldGather_refz(texldSrc));
    VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Operand_GetTexldGather_refz(texldSrc));

    /* swizzle */
    paraFound = gcvFALSE;
    paramName = _GetLibFuncParam(LibFunc, argIdx);
    for (i = 0; i < Context->libSpecializationConstantCount; i++)
    {
        specializationConst = &Context->libSpecializationConsts[i];

        if (gcmIS_SUCCESS(gcoOS_StrCmp(specializationConst->varName, paramName)) &&
            specializationConst->type == VSC_SHADER_DATA_TYPE_IVEC4)
        {
            VIR_ConstVal    new_const_val;
            VIR_ConstId     new_const_id;
            VIR_Const       *new_const;

            errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
            ON_ERROR(errCode, "_InsertCallTexld");

            new_const_val.vecVal.u32Value[0] = specializationConst->value.iValue[0];
            new_const_val.vecVal.u32Value[1] = specializationConst->value.iValue[1];
            new_const_val.vecVal.u32Value[2] = specializationConst->value.iValue[2];
            new_const_val.vecVal.u32Value[3] = specializationConst->value.iValue[3];

            VIR_Shader_AddConstant(pShader,
                VIR_TYPE_UINT_X4, &new_const_val, &new_const_id);
            new_const = VIR_Shader_GetConstFromId(pShader, new_const_id);
            new_const->type = VIR_TYPE_UINT_X4;
            VIR_Operand_SetConstId(newInst->src[0], new_const_id);
            VIR_Operand_SetOpKind(newInst->src[0], VIR_OPND_CONST);
            VIR_Operand_SetTypeId(newInst->src[0], VIR_TYPE_UINT_X4);
            VIR_Operand_SetSwizzle(newInst->src[0], VIR_SWIZZLE_XYZW);

            paraFound = gcvTRUE;
            break;
        }
    }
    if (!paraFound)
    {
        gcmASSERT(gcvFALSE);
    }

    /* insert the MOV to get the return value
    MOV destination, arg[6] */
    errCode = _InsertMovFromArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), texldInst->dest);

    /* change texldInst to the call instruction */
    _ChangeTexldToCall(texldInst, LibFunc);

OnError:
    return errCode;
};

/* vec3 _unormalizeCoord(sampler3D origSampler, vec3 coord) */
static VSC_ErrCode
_InsertCallUnnormalizeCoord(
IN VIR_LinkLibContext     *Context,
IN void                   *Transpoint,
IN VIR_Function           *LibFunc
)
{
    VSC_ErrCode                      errCode = VSC_ERR_NONE;
    VIR_Shader                       *pShader = Context->shader;
    VIR_Instruction                  *texldInst = (VIR_Instruction *)Transpoint;
    VIR_Function                     *pFunc = VIR_Inst_GetFunction(texldInst);
    VIR_Instruction                  *newInst = gcvNULL, *callInst = gcvNULL;

    VIR_Operand                      *texldSrc = gcvNULL;
    gctUINT                          argIdx = 0;

    VIR_SymId               parmSymId, parmVregId;
    VIR_Symbol              *parmSym, *parmVregSym;
    VIR_TypeId              symTy;

    gcmASSERT(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(texldInst)));

    /* insert the MOV to pass arguement
    MOV arg1, sampler
    MOV arg2, coord */
    for (argIdx = 0; argIdx < 2; argIdx++)
    {
        errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx, texldInst, &newInst);
        ON_ERROR(errCode, "_InsertCallUnnormalizeCoord");

        texldSrc = VIR_Inst_GetSource(texldInst, argIdx);

        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), texldSrc);
    }

    /* insert a call instruction */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
        VIR_OP_CALL,
        VIR_TYPE_UNKNOWN,
        texldInst,
        gcvTRUE,
        &callInst);
    ON_ERROR(errCode, "_InsertCallUnnormalizeCoord");
    VIR_Operand_SetFunction(callInst->dest, LibFunc);

    /* insert the MOV to get the return value to set texld coordinate
    MOV coordinate, arg[2] */
    errCode = VIR_Function_AddInstructionAfter(pFunc,
        VIR_OP_MOV,
        VIR_TYPE_UNKNOWN,
        callInst,
        gcvTRUE,
        &newInst);
    ON_ERROR(errCode, "_InsertCallUnnormalizeCoord");

    parmSymId = VIR_IdList_GetId(&LibFunc->paramters, argIdx);
    parmSym = VIR_Function_GetSymFromId(LibFunc, parmSymId);
    gcmASSERT(VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_OUTPARM ||
        VIR_Symbol_GetStorageClass(parmSym) == VIR_STORAGE_INOUTPARM);
    parmVregId = VIR_Symbol_GetVariableVregIndex(parmSym);
    parmVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, parmVregId);
    symTy = VIR_Symbol_GetTypeId(parmSym);

    VIR_Operand_SetTempRegister(newInst->src[0],
        pFunc,
        VIR_Symbol_GetIndex(parmVregSym),
        symTy);

    VIR_Operand_SetSwizzle(newInst->src[0],
        VIR_Enable_2_Swizzle_WShift(VIR_TypeId_Conv2Enable(symTy)));

    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), (VIR_Inst_GetSource(texldInst, 1)));

    VIR_Operand_Change2Dest(VIR_Inst_GetDest(newInst));


OnError:
    return errCode;
};

static VIR_ImageFormat
_FixImageFormatByImageType(
    IN VIR_TypeId       imageType,
    IN VIR_ImageFormat  imageFormat
    )
{
    VIR_ImageFormat     fixedImageFormat = imageFormat;

    if (VIR_TypeId_isImageDataFloat(imageType))
    {
        switch (fixedImageFormat)
        {
        /* Format RGBA. */
        case VIR_IMAGE_FORMAT_RGBA32I:
        case VIR_IMAGE_FORMAT_RGBA32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA32F;
            break;

        case VIR_IMAGE_FORMAT_RGBA16I:
        case VIR_IMAGE_FORMAT_RGBA16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA16F;
            break;

        case VIR_IMAGE_FORMAT_RGBA8I:
        case VIR_IMAGE_FORMAT_RGBA8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA8;
            break;

        /* Format RG. */
        case VIR_IMAGE_FORMAT_RG32I:
        case VIR_IMAGE_FORMAT_RG32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG32F;
            break;

        case VIR_IMAGE_FORMAT_RG16I:
        case VIR_IMAGE_FORMAT_RG16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG16F;
            break;

        case VIR_IMAGE_FORMAT_RG8I:
        case VIR_IMAGE_FORMAT_RG8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG8;
            break;

        /* Format R. */
        case VIR_IMAGE_FORMAT_R32I:
        case VIR_IMAGE_FORMAT_R32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R32F;
            break;

        case VIR_IMAGE_FORMAT_R16I:
        case VIR_IMAGE_FORMAT_R16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R16F;
            break;

        case VIR_IMAGE_FORMAT_R8I:
        case VIR_IMAGE_FORMAT_R8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R8;
            break;

        default:
            if (!VIR_TypeId_isImageDataFloat(imageType))
            {
                gcmASSERT(gcvFALSE);
            }
            break;
        }
    }
    else if (VIR_TypeId_isImageDataSignedInteger(imageType))
    {
        switch (fixedImageFormat)
        {
        /* Format RGBA. */
        case VIR_IMAGE_FORMAT_RGBA32F:
        case VIR_IMAGE_FORMAT_RGBA32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA32I;
            break;

        case VIR_IMAGE_FORMAT_RGBA16F:
        case VIR_IMAGE_FORMAT_RGBA16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA16I;
            break;

        case VIR_IMAGE_FORMAT_RGBA8:
        case VIR_IMAGE_FORMAT_RGBA8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA8I;
            break;

        /* Format RG. */
        case VIR_IMAGE_FORMAT_RG32F:
        case VIR_IMAGE_FORMAT_RG32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG32I;
            break;

        case VIR_IMAGE_FORMAT_RG16F:
        case VIR_IMAGE_FORMAT_RG16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG16I;
            break;

        case VIR_IMAGE_FORMAT_RG8:
        case VIR_IMAGE_FORMAT_RG8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG8I;
            break;

        /* Format R. */
        case VIR_IMAGE_FORMAT_R32F:
        case VIR_IMAGE_FORMAT_R32UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R32I;
            break;

        case VIR_IMAGE_FORMAT_R16F:
        case VIR_IMAGE_FORMAT_R16UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R16I;
            break;

        case VIR_IMAGE_FORMAT_R8:
        case VIR_IMAGE_FORMAT_R8UI:
            fixedImageFormat = VIR_IMAGE_FORMAT_R8I;
            break;

        default:
            if (!VIR_TypeId_isImageDataSignedInteger(imageType))
            {
                gcmASSERT(gcvFALSE);
            }
            break;
        }
    }
    else if (VIR_TypeId_isImageDataUnSignedInteger(imageType))
    {
        switch (fixedImageFormat)
        {
        /* Format RGBA. */
        case VIR_IMAGE_FORMAT_RGBA32I:
        case VIR_IMAGE_FORMAT_RGBA32F:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA32UI;
            break;

        case VIR_IMAGE_FORMAT_RGBA16I:
        case VIR_IMAGE_FORMAT_RGBA16F:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA16UI;
            break;

        case VIR_IMAGE_FORMAT_RGBA8I:
        case VIR_IMAGE_FORMAT_RGBA8:
            fixedImageFormat = VIR_IMAGE_FORMAT_RGBA8UI;
            break;

        /* Format RG. */
        case VIR_IMAGE_FORMAT_RG32I:
        case VIR_IMAGE_FORMAT_RG32F:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG32UI;
            break;

        case VIR_IMAGE_FORMAT_RG16I:
        case VIR_IMAGE_FORMAT_RG16F:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG16UI;
            break;

        case VIR_IMAGE_FORMAT_RG8I:
        case VIR_IMAGE_FORMAT_RG8:
            fixedImageFormat = VIR_IMAGE_FORMAT_RG8UI;
            break;

        /* Format R. */
        case VIR_IMAGE_FORMAT_R32I:
        case VIR_IMAGE_FORMAT_R32F:
            fixedImageFormat = VIR_IMAGE_FORMAT_R32UI;
            break;

        case VIR_IMAGE_FORMAT_R16I:
        case VIR_IMAGE_FORMAT_R16F:
            fixedImageFormat = VIR_IMAGE_FORMAT_R16UI;
            break;

        case VIR_IMAGE_FORMAT_R8I:
        case VIR_IMAGE_FORMAT_R8:
            fixedImageFormat = VIR_IMAGE_FORMAT_R8UI;
            break;

        default:
            if (!VIR_TypeId_isImageDataUnSignedInteger(imageType))
            {
                gcmASSERT(gcvFALSE);
            }
            break;
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return fixedImageFormat;
}

static VSC_ErrCode
_AddTexelBufferToImage(
    IN VIR_Shader       *pShader,
    IN VIR_Function     *pFunc,
    IN VIR_Symbol       *pSamplerSym,
    IN VIR_Uniform      *pSamplerUniform,
    IN gctUINT          arrayIndex,
    IN VIR_ImageFormat  imageFormat
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_SymId           texelBufferToImageSymId = pSamplerUniform->u.samplerOrImageAttr.texelBufferToImageSymId;
    VIR_Symbol          *pTexelBufferToImageSym = gcvNULL;
    VIR_Uniform         *pTexelBufferToImage = gcvNULL;

    if (texelBufferToImageSymId == NOT_ASSIGNED)
    {
        VIR_NameId      nameId;
        gctCHAR         name[128] = "#";
        VIR_TypeId      imageType = VIR_TypeId_ConvertSamplerTypeToImageType(pShader, VIR_Symbol_GetTypeId(pSamplerSym));
        VIR_ImageFormat fixedImageFormat = _FixImageFormatByImageType(imageType, imageFormat);

        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, pSamplerSym));
        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$TexelBufferToImage");
        errCode = VIR_Shader_AddString(pShader,
                                       name,
                                       &nameId);
        ON_ERROR(errCode, "VIR_Shader_AddString");

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_IMAGE,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, imageType),
                                       VIR_STORAGE_UNKNOWN,
                                       &texelBufferToImageSymId);

        ON_ERROR(errCode, "VIR_Shader_AddSymbol");

        pTexelBufferToImageSym = VIR_Shader_GetSymFromId(pShader, texelBufferToImageSymId);
        pSamplerUniform->u.samplerOrImageAttr.texelBufferToImageSymId = texelBufferToImageSymId;
        VIR_Symbol_SetFlag(pTexelBufferToImageSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(pTexelBufferToImageSym, VIR_Symbol_GetPrecision(pSamplerSym));
        VIR_Symbol_SetUniformKind(pTexelBufferToImageSym, VIR_UNIFORM_TEXELBUFFER_TO_IMAGE);
        VIR_Symbol_SetAddrSpace(pTexelBufferToImageSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(pTexelBufferToImageSym, VIR_Symbol_GetTyQualifier(pSamplerSym));
        pTexelBufferToImageSym->layout = pSamplerSym->layout;
        /* Set the corresponding image format. */
        VIR_Symbol_SetImageFormat(pTexelBufferToImageSym, fixedImageFormat);

        pTexelBufferToImage = VIR_Symbol_GetImage(pTexelBufferToImageSym);
        pTexelBufferToImage->u.samplerOrImageAttr.parentSamplerSymId = VIR_Symbol_GetIndex(pSamplerSym);
        pTexelBufferToImage->u.samplerOrImageAttr.arrayIdxInParent = arrayIndex;
    }

    VIR_Uniform_SetFlag(pSamplerUniform, VIR_UNIFORMFLAG_TREAT_TEXELBUFFE_AS_IMG);

OnError:
    return errCode;
}

/* vec4 _texld_with_imgld_R32G32B32A32SFLOAT(int coord, samplerBuffer sampledTexelBuffer) */
static VSC_ErrCode
_InsertCallTexldImg(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode                      errCode = VSC_ERR_NONE;
    VIR_Shader                       *pShader = Context->shader;
    VIR_Instruction                  *texldInst = (VIR_Instruction *)Transpoint;
    VIR_Function                     *pFunc = VIR_Inst_GetFunction(texldInst);
    VIR_Instruction                  *newInst = gcvNULL;

    VIR_Symbol                       *pSamplerSym = gcvNULL;
    VIR_Uniform                      *pSamplerUniform = gcvNULL;
    gctUINT                          argIdx = 0;

    gcmASSERT(VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(texldInst)));

    /* Get the sampler uniform. */
    gcmASSERT(VIR_Operand_isSymbol(VIR_Inst_GetSource(texldInst, 0)));
    pSamplerSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(texldInst, 0));

    gcmASSERT(VIR_Symbol_isSampler(pSamplerSym));
    pSamplerUniform = VIR_Symbol_GetSampler(pSamplerSym);

    /* Add the texelBuffer to image uniform. */
    errCode = _AddTexelBufferToImage(pShader,
                                     pFunc,
                                     pSamplerSym,
                                     pSamplerUniform,
                                     Context->linkPoint->u.resource.arrayIndex,
                                     /* Need to fix it by the sampler type. */
                                     VIR_IMAGE_FORMAT_RGBA32F);
    ON_ERROR(errCode, "_InsertCallTexldImg");

    /* insert the MOV to pass arguement */
    /* MOV arg1, sampler */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexldImg");
    VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Inst_GetSource(texldInst, 0));

    /* MOV arg2, TexelBufferToImage2D */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexldImg");
    gcmASSERT(pSamplerUniform->u.samplerOrImageAttr.texelBufferToImageSymId != NOT_ASSIGNED);
    VIR_Operand_SetSymbol(VIR_Inst_GetSource(newInst, 0),
                          pFunc,
                          pSamplerUniform->u.samplerOrImageAttr.texelBufferToImageSymId);
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(newInst, 0), VIR_SWIZZLE_XYZW);

    /* MOV arg3, coord */
    errCode = _InsertMovToArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    ON_ERROR(errCode, "_InsertCallTexldImg");
    VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Inst_GetSource(texldInst, 1));

    /* insert the MOV to get the return value
       MOV destination, arg4 */
    errCode = _InsertMovFromArgs(pShader, pFunc, LibFunc, argIdx++, texldInst, &newInst);
    VIR_Operand_Copy(VIR_Inst_GetDest(newInst), texldInst->dest);

     /* change texldInst to the call instruction */
    _ChangeTexldToCall(texldInst, LibFunc);

OnError:
    return errCode;
};


static VSC_ErrCode
_InsertCallTexldFmt(
    IN VIR_LinkLibContext     *Context,
    IN void                   *Transpoint,
    IN VIR_Function           *LibFunc
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Instruction         *texldInst = (VIR_Instruction *) Transpoint;
    VIR_OpCode              texldOp = VIR_Inst_GetOpcode(texldInst);

    if (VIR_OPCODE_isTexLd(texldOp))
    {
        switch (Context->linkPoint->u.resource.subType)
        {
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXLD_EXTRA_LATYER:
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGRAD_EXTRA_LATYER:
            errCode = _InsertCallTexld(Context, Transpoint, LibFunc);
            break;
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHER_EXTRA_LAYTER:
            errCode = _InsertCallTexldGather(Context, Transpoint, LibFunc);
            break;
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXGATHERPCF_D32F:
            errCode = _InsertCallTexldGatherPCF(Context, Transpoint, LibFunc);
            break;
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_NORMALIZE_TEXCOORD:
            errCode = _InsertCallUnnormalizeCoord(Context, Transpoint, LibFunc);
            break;
        case VSC_LINK_POINT_RESOURCE_SUBTYPE_TEXFETCH_REPLACE_WITH_IMGLD:
            errCode = _InsertCallTexldImg(Context, Transpoint, LibFunc);
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return errCode;
};

static void
_LinkLibContext_Initialize(
    IN OUT VIR_LinkLibContext               *Context,
    IN VSC_HW_CONFIG                        *pHwCfg,
    IN VSC_MM                               *pMM,
    IN VIR_Shader                           *Shader,
    IN VIR_Shader                           *LibShader,
    IN VSC_HASH_TABLE                       *pTempHashTable,
    IN VIR_ShaderKind                       shaderKind,
    IN VSC_LIB_LINK_POINT                   *pLinkPoint,
    IN gctUINT                              libSpecializationConstantCount,
    IN VSC_LIB_SPECIALIZATION_CONSTANT      *libSpecializationConsts,
    IN VIR_LinkLib_GET_TRANSPOINT_PTR       GetTranspoint,
    IN VIR_LinkLib_GET_LIB_FUNC_NAME_PTR    GetLibFuncName,
    IN VIR_LinkLib_INSERT_CALL_PTR          InsertCallPtr
    )
{
    Context->pMM = pMM;
    Context->pHwCfg = pHwCfg;
    Context->shader     = Shader;
    Context->libShader = LibShader;
    Context->pTempHashTable = pTempHashTable;
    Context->shaderKind = shaderKind;
    Context->linkPoint  = pLinkPoint;
    Context->libSpecializationConstantCount = libSpecializationConstantCount;
    Context->libSpecializationConsts = libSpecializationConsts;

    Context->changed    = gcvFALSE;

    Context->getTranspoint  = GetTranspoint;
    Context->getLibFuncName = GetLibFuncName;
    Context->insertCall     = InsertCallPtr;
}

void
_LinkLibContext_Finalize(
    IN VIR_LinkLibContext     *Context
    )
{
}

/* main function for lib-link */
VSC_ErrCode
_LinkLib_Transform(
    IN VIR_LinkLibContext *Context
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_HW_CONFIG       *pHwCfg = Context->pHwCfg;
    VSC_MM              *pMM = Context->pMM;
    VIR_Shader          *pShader = Context->shader;
    gctSTRING           str = gcvNULL;

    /* Use it to save the lib function that generates during this transform. */
    VSC_HASH_TABLE      *pAddLibFuncSet = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 64);

    VIR_TRANS_WORKLIST  vTranspointslist;
    VIR_LIB_WORKLIST    vFuncList;
    VIR_LIB_CALLSITES   vCallSites;

    QUEUE_INITIALIZE(&vTranspointslist);
    QUEUE_INITIALIZE(&vFuncList);
    QUEUE_INITIALIZE(&vCallSites);

    /* transform the shader based on lib-link shaderKind */
    if (Context->shaderKind != VIR_SHADER_UNKNOWN &&
        Context->shaderKind != VIR_SHADER_LIBRARY &&
        pShader->shaderKind != Context->shaderKind)
    {
        return errCode;
    }

    /* step1: get the transform points (where lib-link is needed) into a worklist */
    Context->getTranspoint(Context, &vTranspointslist);

    /* for each transpoint in the worklist  */
    while(!QUEUE_CHECK_EMPTY(&vTranspointslist))
    {
        void            *transPoint;
        VIR_TypeId      tyId = VIR_TYPE_VOID;
        VIR_Function    *pFunc = gcvNULL;
        VIR_Function    *libFunc = gcvNULL;
        gctSTRING       libFuncName = gcvNULL;

        _TranspointsDequeue(pMM, &vTranspointslist, &transPoint);

        /* Get the lib function name. */
        if (Context->getLibFuncName)
        {
            if (str == gcvNULL)
            {
                str = (gctSTRING) vscMM_Alloc(pMM, __LIB_NAME_LENGTH__ * sizeof(char));
            }
            Context->getLibFuncName(Context, transPoint, &str);
            libFuncName = str;

            VIR_LIB_CallSitesQueue(pMM, &vCallSites, (VIR_LINKER_CALL_INST_NODE*)transPoint);
        }
        else
        {
            libFuncName = (gctSTRING) Context->linkPoint->strFunc;
        }

        VIR_Shader_GetFunctionByName(pShader, libFuncName, &pFunc);

        if (pFunc == gcvNULL)
        {
            VIR_NameId nameId;

            /* if we could not find the function in the library, linkError */
            VIR_Shader_GetFunctionByName(Context->libShader, libFuncName, &libFunc);
            {
                if (libFunc == gcvNULL)
                {
                    errCode = VSC_ERR_UNSAT_LIB_SYMBOL;
                    ON_ERROR(errCode, "_LinkLib_Transform: cannot find lib function \"%s\"", libFuncName);
                }
            }

            /* add the libFunc to the pShader */
            tyId = VIR_LinkLib_TypeConv(pShader, VIR_Function_GetType(libFunc), gcvFALSE);
            errCode = VIR_Shader_AddString(pShader,
                                           libFuncName,
                                           &nameId);
            ON_ERROR(errCode, "_LinkLib_Transform");

            errCode = VIR_Shader_AddFunction(pShader,
                                             gcvFALSE,
                                             VIR_Shader_GetStringFromId(pShader, nameId),
                                             tyId,
                                             &pFunc);
            ON_ERROR(errCode, "_LinkLib_Transform");

            VIR_LIB_WorkListQueue(pMM, &vFuncList, pFunc);

            VIR_Function_SetFlag(pFunc, VIR_FUNCFLAG_LINKED_LIB);

            /* step2.2 linkin the lib-link function */
            errCode = VIR_Lib_LinkFunctions(Context, pShader, Context->libShader, pMM, pAddLibFuncSet, &vFuncList, &vCallSites);
            ON_ERROR(errCode, "_LinkLib_Transform");
        }

        /* step2.3 insert the argument passing, call and return value passing instructions */
        ON_ERROR0(Context->insertCall(Context, transPoint, pFunc));

        /* step2.4 update the call sites, this is particular for the calls inside the lib function,
           since the lib call site itself already has the right parameters/return passing*/
        errCode = VIR_Lib_UpdateCallSites(Context, pShader, Context->libShader, pHwCfg, pMM, pAddLibFuncSet, &vCallSites);
        ON_ERROR(errCode, "_LinkLib_Transform");

        Context->changed = gcvTRUE;
    }

    /* clear the flag: extcall atomic should be link the lib and replace by call */
    if (VIR_Shader_HasExtcallAtomic(pShader))
    {
        VIR_Shader_ClrFlag(pShader, VIR_SHFLAG_HAS_EXTCALL_ATOM);
    }

OnError:
    if (str)
    {
        vscMM_Free(pMM, str);
    }

    if (pAddLibFuncSet)
    {
        vscHTBL_Destroy(pAddLibFuncSet);
    }

    QUEUE_FINALIZE(&vTranspointslist);
    QUEUE_FINALIZE(&vFuncList);
    QUEUE_FINALIZE(&vCallSites);

    return errCode;
}

VSC_ErrCode
VIR_Shader_ReverseFacingValue(
    IN OUT VIR_Shader               *pShader
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;
    VIR_Function        *func;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator    inst_iter;
        VIR_Instruction     *inst;

        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SrcOperand_Iterator srcOpndIter;
            VIR_Operand             *pOpnd;
            VIR_Operand             *reversedFacingOpnd = gcvNULL;

            VIR_SrcOperand_Iterator_Init(inst, &srcOpndIter);
            pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                /* find if any operand using gl_FrontFace */
                if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL &&
                     VIR_Symbol_GetName(VIR_Operand_GetSymbol(pOpnd)) == VIR_NAME_FRONT_FACING)
                {
                    /* reuse reversedFacingOpnd if it is already created */
                    if (reversedFacingOpnd != gcvNULL)
                    {
                        VIR_Operand_Copy(pOpnd, reversedFacingOpnd);
                        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);
                    }
                    else
                    {
                        /* add instruction to reverse facing value */
                        VIR_Instruction     *select_inst;
                        VIR_Operand         *select_dest;
                        VIR_VirRegId        select_regid;
                        VIR_SymId           select_symid;
                        VIR_Symbol          *select_sym;
                        errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_SELECT, VIR_TYPE_BOOLEAN, inst, gcvTRUE, &select_inst);

                        VIR_Inst_SetConditionOp(select_inst, VIR_COP_NOT_ZERO);

                        select_regid = VIR_Shader_NewVirRegId(pShader, 1);
                        errCode = VIR_Shader_AddSymbol(pShader,
                                                          VIR_SYM_VIRREG,
                                                          select_regid,
                                                          VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_BOOLEAN),
                                                          VIR_STORAGE_UNKNOWN,
                                                          &select_symid);
                        if(errCode != VSC_ERR_NONE) return errCode;

                        select_sym = VIR_Shader_GetSymFromId(pShader, select_symid);
                        VIR_Symbol_SetPrecision(select_sym, VIR_PRECISION_MEDIUM);
                        select_dest = VIR_Inst_GetDest(select_inst);
                        VIR_Operand_SetTempRegister(select_dest, func, select_symid, VIR_TYPE_BOOLEAN);
                        VIR_Operand_SetEnable(select_dest, VIR_ENABLE_X);

                        VIR_Operand_Copy(VIR_Inst_GetSource(select_inst, 0), pOpnd);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(select_inst, 0), VIR_SWIZZLE_XXXX);

                        /* src1 = 0 */
                        VIR_Operand_SetImmediateBoolean(VIR_Inst_GetSource(select_inst, 1), 0);

                        /* src2 = 1 */
                        VIR_Operand_SetImmediateBoolean(VIR_Inst_GetSource(select_inst, 2), 1);

                        reversedFacingOpnd = select_dest;

                        /* change the operand to use reverse value */
                        VIR_Operand_SetSym(pOpnd, select_sym);
                    }
                }
            }
        }
    }

    return errCode;
}

VSC_ErrCode
VIR_Shader_FacingValueAlwaysFront(
    IN OUT VIR_Shader               *pShader
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode    *func_node;
    VIR_Function        *func;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator    inst_iter;
        VIR_Instruction     *inst;

        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SrcOperand_Iterator srcOpndIter;
            VIR_Operand             *pOpnd;

            VIR_SrcOperand_Iterator_Init(inst, &srcOpndIter);
            pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                /* Find if any operand using gl_FrontFace and use TRUE to replace it. */
                if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL &&
                    VIR_Symbol_GetName(VIR_Operand_GetSymbol(pOpnd)) == VIR_NAME_FRONT_FACING)
                {
                    VIR_Symbol_SetFlag(VIR_Operand_GetSymbol(pOpnd), VIR_SYMFLAG_UNUSED);
                    VIR_Operand_SetImmediateBoolean(pOpnd, gcvTRUE);
                }
            }
        }
    }

    return errCode;
}

static void
_InitializeLibLinkEntryData(
    IN VSC_MM                   *pMM,
    VSC_SHADER_LIB_LINK_ENTRY   *pLibLinkEntry
    )
{
    if (pLibLinkEntry->pTempHashTable == gcvNULL)
    {
        pLibLinkEntry->pTempHashTable = (VSC_HASH_TABLE*)vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 64);
    }
}

static void
_FinalizeLibLinkEntryData(
    IN VSC_MM                   *pMM,
    VSC_SHADER_LIB_LINK_ENTRY   *pLibLinkEntry
    )
{
    if (pLibLinkEntry->pTempHashTable != gcvNULL)
    {
        vscHTBL_Destroy((VSC_HASH_TABLE*)pLibLinkEntry->pTempHashTable);
    }
}

VSC_ErrCode
VIR_LinkLibLibrary(
    IN VSC_HW_CONFIG            *pHwCfg,
    IN VSC_MM                   *pMM,
    IN VIR_Shader               *pShader,
    IN VSC_SHADER_LIB_LINK_TABLE*pLibLinkTable,
    INOUT gctBOOL               *pChanged
    )
{
    VSC_ErrCode                errCode  = VSC_ERR_NONE;
    VSC_SHADER_LIB_LINK_ENTRY* libEntry;
    VIR_LinkLibContext         vContext;
    gctUINT                    i, j;

    /* If no lib need to be linked, just return */
    if (pLibLinkTable == gcvNULL)
    {
        return errCode;
    }

    vContext.changed = gcvFALSE;

    for (i = 0; i < pLibLinkTable->shLinkEntryCount; i ++)
    {
        libEntry = &pLibLinkTable->pShLibLinkEntries[i];
        _InitializeLibLinkEntryData(pMM, libEntry);

        /* for each link point */
        for (j = 0; j < libEntry->linkPointCount; j++)
        {
            VSC_LIB_LINK_POINT *linkPoint = &libEntry->linkPoint[j];

            switch(linkPoint->libLinkType)
            {
            case VSC_LIB_LINK_TYPE_FUNC_NAME:
                _LinkLibContext_Initialize(
                        &vContext,
                        pHwCfg,
                        pMM,
                        pShader,
                        (VIR_Shader *)libEntry->hShaderLib,
                        (VSC_HASH_TABLE*)libEntry->pTempHashTable,
                        VIR_SHADER_LIBRARY,
                        linkPoint,
                        0,
                        gcvNULL,
                        _GetIntrinsicOrExtFunc,
                        _GetIntrinsicOrextFuncName,
                        _InsertIntrinsicFunc);
                break;

            case VSC_LIB_LINK_TYPE_COLOR_OUTPUT:
                _LinkLibContext_Initialize(
                        &vContext,
                        pHwCfg,
                        pMM,
                        pShader,
                        (VIR_Shader *)libEntry->hShaderLib,
                        (VSC_HASH_TABLE*)libEntry->pTempHashTable,
                        VIR_SHADER_FRAGMENT,
                        linkPoint,
                        libEntry->libSpecializationConstantCount,
                        libEntry->pLibSpecializationConsts,
                        _GetTranspointOutputFmt,
                        gcvNULL,
                        _InsertCallOutputFmt);
                break;

            case VSC_LIB_LINK_TYPE_RESOURCE:
                _LinkLibContext_Initialize(
                        &vContext,
                        pHwCfg,
                        pMM,
                        pShader,
                        (VIR_Shader *)libEntry->hShaderLib,
                        (VSC_HASH_TABLE*)libEntry->pTempHashTable,
                        VIR_SHADER_UNKNOWN,
                        linkPoint,
                        libEntry->libSpecializationConstantCount,
                        libEntry->pLibSpecializationConsts,
                        _GetTranspointTexldFmt,
                        gcvNULL,
                        _InsertCallTexldFmt);
                break;

            case VSC_LIB_LINK_TYPE_FRONTFACING_CCW:
                errCode = VIR_Shader_ReverseFacingValue(pShader);
                ON_ERROR(errCode, "VIR_LinkLibLibrary: FRONTFACING_CCW");
                continue;

            case VSC_LIB_LINK_TYPE_FRONTFACING_ALWAY_FRONT:
                errCode = VIR_Shader_FacingValueAlwaysFront(pShader);
                ON_ERROR(errCode, "VIR_LinkLibLibrary: FRONTFACING_CCW");
                continue;
                break;

            default:
                gcmASSERT(gcvFALSE);
                break;
            }

            errCode = _LinkLib_Transform(&vContext);
            ON_ERROR(errCode, "VIR_LinkLibLibrary");
        }

        _FinalizeLibLinkEntryData(pMM, libEntry);
    }

    if (pChanged)
    {
        *pChanged = vContext.changed;
    }

    _LinkLibContext_Finalize(&vContext);

    if (pChanged && VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after linking library", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_LinkExternalLibFunc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_HL |
                                 VSC_PASS_LEVEL_ML |
                                 VSC_PASS_LEVEL_LL |
                                 VSC_PASS_LEVEL_MC |
                                 VSC_PASS_LEVEL_CG;

    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

VSC_ErrCode
VIR_LinkExternalLibFunc(IN VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    gctBOOL                     bChanged = gcvFALSE;
    VSC_EXTERNAL_LINK_PASS_DATA passData = { gcvFALSE, gcvFALSE};

    if (pPassWorker->basePassWorker.pPrvData != gcvNULL)
    {
        passData = *(VSC_EXTERNAL_LINK_PASS_DATA*)pPassWorker->basePassWorker.pPrvData;
    }

    errCode = VIR_LinkLibLibrary(&pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                                 pPassWorker->basePassWorker.pMM,
                                 (VIR_Shader *)pPassWorker->pCompilerParam->hShader,
                                 pPassWorker->pCompilerParam->pShLibLinkTable,
                                 &bChanged);
    ON_ERROR(errCode, "VIR_LinkExternalLibFunc");

    /* Check if we need to invalid CFG/CG. */
    if (bChanged && passData.bNeedToInvalidCFG)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCg = gcvTRUE;
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = gcvTRUE;
        pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;

        gcmASSERT(pPassWorker->basePassWorker.pPrvData);
        ((VSC_EXTERNAL_LINK_PASS_DATA*)pPassWorker->basePassWorker.pPrvData)->bChanged = gcvTRUE;
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_LinkInternalLibFunc)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_HL |
                                 VSC_PASS_LEVEL_ML |
                                 VSC_PASS_LEVEL_LL |
                                 VSC_PASS_LEVEL_MC |
                                 VSC_PASS_LEVEL_CG;

    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_ILF_LINK;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
}

VSC_ErrCode
VIR_LinkInternalLibFunc(IN VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VIR_Shader *                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    PVSC_SYS_CONTEXT            pSysCtx = pPassWorker->pCompilerParam->cfg.ctx.pSysCtx;
    VSC_PRIV_DATA*              pPrivData = (VSC_PRIV_DATA*)pSysCtx->pCoreSysCtx->hPrivData;
    VSC_HW_CONFIG*              pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader *                pIntrinsicLib = gcvNULL;
    VSC_LIB_LINK_POINT          libLinkPoint;
    VSC_SHADER_LIB_LINK_ENTRY   libLinkEntry;
    VSC_SHADER_LIB_LINK_TABLE   libLinkTable;

    /* Create intrinsic lib shader. */
    if (pPrivData)
    {
        /* Initialize pmp first. */
        if (!vscPMP_IsInitialized(&pPrivData->pmp))
        {
            vscPMP_Intialize(&pPrivData->pmp, gcvNULL, 512, sizeof(void*), gcvTRUE);
        }

        {
            errCode = VIR_GetIntrinsicLib(pHwCfg,
                &pPrivData->pmp.mmWrapper,
                (pPassWorker->pCompilerParam->cfg.ctx.clientAPI == gcvAPI_OPENCL),
                VIR_Shader_IsGraphics(pShader),
                gcvFALSE,
                VIR_Shader_HasExtcallAtomic(pShader),
                &pIntrinsicLib);
            CHECK_ERROR(errCode, "VIR_GetIntrinsicLib failed.");
        }
    }

    if (pIntrinsicLib && gcUseFullNewLinker(pHwCfg->hwFeatureFlags.hasHalti2))
    {
        /* Construct the lib link point. */
        gcoOS_ZeroMemory(&libLinkPoint, sizeof(VSC_LIB_LINK_POINT));
        libLinkPoint.libLinkType = VSC_LIB_LINK_TYPE_FUNC_NAME;
        libLinkPoint.strFunc = gcvNULL;

        /* Construct the lib link entry. */
        gcoOS_ZeroMemory(&libLinkEntry, sizeof(VSC_SHADER_LIB_LINK_ENTRY));
        libLinkEntry.hShaderLib = pIntrinsicLib;
        libLinkEntry.pTempHashTable = gcvNULL;
        libLinkEntry.libSpecializationConstantCount = 0;
        libLinkEntry.pLibSpecializationConsts = gcvNULL;
        libLinkEntry.linkPointCount = 1;
        libLinkEntry.linkPoint[0] = libLinkPoint;

        /* Construct the lib link table. */
        gcoOS_ZeroMemory(&libLinkTable, sizeof(VSC_SHADER_LIB_LINK_TABLE));
        libLinkTable.shLinkEntryCount = 1;
        libLinkTable.pShLibLinkEntries = &libLinkEntry;

        errCode = VIR_LinkLibLibrary(&pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                                     pPassWorker->basePassWorker.pMM,
                                     (VIR_Shader *)pPassWorker->pCompilerParam->hShader,
                                     &libLinkTable,
                                     gcvNULL);
        ON_ERROR(errCode, "VIR_LinkExternalLibFunc");
    }

OnError:
    return errCode;
}

