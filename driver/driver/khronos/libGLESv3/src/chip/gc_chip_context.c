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


#include "gc_es_context.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    __GLES3_ZONE_CONTEXT

extern __GLchipGlobal dpGlobalInfo;
extern __GLformatInfo __glFormatInfoTable[];
extern GLint __glesApiProfileMode;

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/

static GLint __glCompressedTextureFormats_astc[] =
{
    GL_COMPRESSED_RGBA_ASTC_4x4_KHR,
    GL_COMPRESSED_RGBA_ASTC_5x4_KHR,
    GL_COMPRESSED_RGBA_ASTC_5x5_KHR,
    GL_COMPRESSED_RGBA_ASTC_6x5_KHR,
    GL_COMPRESSED_RGBA_ASTC_6x6_KHR,
    GL_COMPRESSED_RGBA_ASTC_8x5_KHR,
    GL_COMPRESSED_RGBA_ASTC_8x6_KHR,
    GL_COMPRESSED_RGBA_ASTC_8x8_KHR,
    GL_COMPRESSED_RGBA_ASTC_10x5_KHR,
    GL_COMPRESSED_RGBA_ASTC_10x6_KHR,
    GL_COMPRESSED_RGBA_ASTC_10x8_KHR,
    GL_COMPRESSED_RGBA_ASTC_10x10_KHR,
    GL_COMPRESSED_RGBA_ASTC_12x10_KHR,
    GL_COMPRESSED_RGBA_ASTC_12x12_KHR,

    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,
};

static GLint __glCompressedTextureFormats_etc1[] =
{
    GL_ETC1_RGB8_OES,
};


static GLint __glCompressedTextureFormats_palette[] =
{
    GL_PALETTE4_RGBA4_OES,
    GL_PALETTE4_RGB5_A1_OES,
    GL_PALETTE4_R5_G6_B5_OES,
    GL_PALETTE4_RGB8_OES,
    GL_PALETTE4_RGBA8_OES,
    GL_PALETTE8_RGBA4_OES,
    GL_PALETTE8_RGB5_A1_OES,
    GL_PALETTE8_R5_G6_B5_OES,
    GL_PALETTE8_RGB8_OES,
    GL_PALETTE8_RGBA8_OES,
};

static GLint __glCompressedTextureFormats[] =
{
    GL_COMPRESSED_R11_EAC,
    GL_COMPRESSED_SIGNED_R11_EAC,
    GL_COMPRESSED_RG11_EAC,
    GL_COMPRESSED_SIGNED_RG11_EAC,
    GL_COMPRESSED_RGB8_ETC2,
    GL_COMPRESSED_SRGB8_ETC2,
    GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    GL_COMPRESSED_RGBA8_ETC2_EAC,
    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,
};

static GLint __glShaderBinaryFormats_viv[] =
{
    GL_SHADER_BINARY_VIV,
};

static GLint __glProgramBinaryFormats_viv[] =
{
    GL_PROGRAM_BINARY_VIV,
};

/*******************************************************************************
**
**  gcChipConstructChipName
**
**  Construct chip name string.
**
**  INPUT:
**      __GLchipContext *chipCtx
**
**  OUTPUT:
**
**      Nothing.
*/

static GLvoid
gcChipConstructChipName(
    __GLchipContext *chipCtx
    )
{
    GLchar *ChipName = chipCtx->chipName;
    gctSTRING productName = gcvNULL;
    gceSTATUS status;

    /* Constant name postifx. */
    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    gcoOS_ZeroMemory(ChipName, __GL_CHIP_NAME_LEN);

    /* Append Vivante GC to the string. */
    *ChipName++ = 'V';
    *ChipName++ = 'i';
    *ChipName++ = 'v';
    *ChipName++ = 'a';
    *ChipName++ = 'n';
    *ChipName++ = 't';
    *ChipName++ = 'e';
    *ChipName++ = ' ';

    gcmONERROR(gcoHAL_GetProductName(chipCtx->hal, &productName));

    gcoOS_StrCatSafe(chipCtx->chipName, __GL_CHIP_NAME_LEN , productName);

    gcmOS_SAFE_FREE(chipCtx->os, productName);

OnError:
    gcmFOOTER();
    return;
}



__GL_INLINE gceSTATUS
gcChipGetSampleLocations(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status;
    GLint i, j;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 4; j++)
        {
            gcmONERROR(gco3D_GetSampleCoords(chipCtx->engine, j, (gctBOOL)i, &chipCtx->sampleLocations[i][j][0]));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

/* Init DP interface to chip specific function */
static GLvoid
gcChipInitDevicePipeline(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);
    gc->dp.makeCurrent = __glChipMakeCurrent;
    gc->dp.loseCurrent = __glChipLoseCurrent;
    gc->dp.destroyPrivateData = __glChipDestroyContext;
    gc->dp.queryFormatInfo = __glChipQueryFormatInfo;

    gc->dp.readPixelsBegin= __glChipReadPixelsBegin;
    gc->dp.readPixelsValidateState = __glChipReadPixelsValidateState;
    gc->dp.readPixelsEnd = __glChipReadPixelsEnd;
    gc->dp.readPixels = __glChipReadPixels;

    gc->dp.drawBegin = __glChipDrawBegin;
    gc->dp.drawValidateState = __glChipDrawValidateState;
    gc->dp.drawEnd = __glChipDrawEnd;

     /* Flush, Finish */
    gc->dp.flush = __glChipFlush;
    gc->dp.finish = __glChipFinish;

    /* Texture related functions */
    gc->dp.bindTexture = __glChipBindTexture;
    gc->dp.deleteTexture = __glChipDeleteTexture;
    gc->dp.detachTexture = __glChipDetachTexture;
    gc->dp.texImage2D = __glChipTexImage2D;
    gc->dp.texImage3D = __glChipTexImage3D;
    gc->dp.texSubImage2D = __glChipTexSubImage2D;
    gc->dp.texSubImage3D = __glChipTexSubImage3D;
    gc->dp.copyTexImage2D = __glChipCopyTexImage2D;
    gc->dp.copyTexSubImage2D = __glChipCopyTexSubImage2D;
    gc->dp.copyTexSubImage3D = __glChipCopyTexSubImage3D;
    gc->dp.compressedTexImage2D = __glChipCompressedTexImage2D;
    gc->dp.compressedTexSubImage2D = __glChipCompressedTexSubImage2D;
    gc->dp.compressedTexImage3D = __glChipCompressedTexImage3D;
    gc->dp.compressedTexSubImage3D = __glChipCompressedTexSubImage3D;
    gc->dp.generateMipmaps = __glChipGenerateMipMap;

    gc->dp.copyTexBegin = __glChipCopyTexBegin;
    gc->dp.copyTexValidateState = __glChipCopyTexValidateState;
    gc->dp.copyTexEnd = __glChipCopyTexEnd;

    gc->dp.copyImageSubData = __glChipCopyImageSubData;

    /* EGL image */
    gc->dp.bindTexImage = __glChipBindTexImage;
    gc->dp.freeTexImage = __glChipFreeTexImage;
    gc->dp.createEglImageTexture = __glChipCreateEglImageTexture;
    gc->dp.createEglImageRenderbuffer = __glChipCreateEglImageRenderbuffer;
    gc->dp.eglImageTargetTexture2DOES = __glChipEglImageTargetTexture2DOES;
    gc->dp.eglImageTargetRenderbufferStorageOES = __glChipEglImageTargetRenderbufferStorageOES;
    gc->dp.getTextureAttribFromImage = __glChipGetTextureAttribFromImage;

    /* VIV_texture_direct */
    gc->dp.texDirectVIV = __glChipTexDirectVIV;
    gc->dp.texDirectInvalidateVIV = __glChipTexDirectInvalidateVIV;
    gc->dp.texDirectVIVMap = __glChipTexDirectVIVMap;

    /* Toggle buffer change */
    gc->dp.changeDrawBuffers = __glChipChangeDrawBuffers;
    gc->dp.changeReadBuffers = __glChipChangeReadBuffers;
    gc->dp.detachDrawable = __glChipDetachDrawable;

    /* Clear buffer */
    gc->dp.clearBegin = __glChipClearBegin;
    gc->dp.clearValidateState = __glChipClearValidateState;
    gc->dp.clearEnd = __glChipClearEnd;
    gc->dp.clear = __glChipClear;
    gc->dp.clearBuffer = __glChipClearBuffer;
    gc->dp.clearBufferfi = __glChipClearBufferfi;


    /* GLSL */
    gc->dp.compileShader = __glChipCompileShader;
    gc->dp.deleteShader = __glChipDeleteShader;
    gc->dp.createProgram = __glChipCreateProgram;
    gc->dp.deleteProgram = __glChipDeleteProgram;
    gc->dp.linkProgram = __glChipLinkProgram;
    gc->dp.useProgram = __glChipUseProgram;
    gc->dp.validateProgram = __glChipValidateProgram;
    gc->dp.getProgramBinary = __glChipGetProgramBinary_V1;
    gc->dp.programBinary = __glChipProgramBinary_V1;
    gc->dp.shaderBinary = __glChipShaderBinary;
    gc->dp.bindAttributeLocation = __glChipBindAttributeLocation;
    gc->dp.getActiveAttribute = __glChipGetActiveAttribute;
    gc->dp.getAttributeLocation = __glChipGetAttributeLocation;
    gc->dp.getFragDataLocation = __glChipGetFragDataLocation;
    gc->dp.getUniformLocation = __glChipGetUniformLocation;
    gc->dp.getActiveUniform = __glChipGetActiveUniform;
    gc->dp.getActiveUniformsiv = __glChipGetActiveUniformsiv;
    gc->dp.getUniformIndices = __glChipGetUniformIndices;
    gc->dp.getUniformBlockIndex = __glChipGetUniformBlockIndex;
    gc->dp.getActiveUniformBlockiv = __glChipGetActiveUniformBlockiv;
    gc->dp.getActiveUniformBlockName = __glChipActiveUniformBlockName;
    gc->dp.uniformBlockBinding = __glChipUniformBlockBinding;
    gc->dp.setUniformData = __glChipSetUniformData;
    gc->dp.getUniformData = __glChipGetUniformData;
    gc->dp.getUniformSize = __glChipGetUniformSize;
    gc->dp.buildTexEnableDim = __glChipBuildTexEnableDim;
    gc->dp.getProgramResourceIndex = __glChipGetProgramResourceIndex;
    gc->dp.getProgramResourceName = __glChipGetProgramResourceName;
    gc->dp.getProgramResourceiv = __glChipGetProgramResourceiv;
    gc->dp.validateProgramPipeline = __glChipValidateProgramPipeline;

    /* Buffer object */
    gc->dp.bindBuffer = __glChipBindBufferObject;
    gc->dp.deleteBuffer = __glChipDeleteBufferObject;
    gc->dp.mapBufferRange = __glChipMapBufferRange;
    gc->dp.flushMappedBufferRange = __glChipFlushMappedBufferRange;
    gc->dp.unmapBuffer = __glChipUnMapBufferObject;
    gc->dp.bufferData = __glChipBufferData;
    gc->dp.bufferSubData = __glChipBufferSubData;
    gc->dp.copyBufferSubData = __glChipCopyBufferSubData;

    /* FBO */
    gc->dp.bindDrawFramebuffer = __glChipBindDrawFramebuffer;
    gc->dp.bindReadFramebuffer = __glChipBindReadFramebuffer;
    gc->dp.bindRenderbuffer = __glChipBindRenderbuffer;
    gc->dp.deleteRenderbuffer = __glChipDeleteRenderbuffer;
    gc->dp.detachRenderbuffer = __glChipDetachRenderbuffer;
    gc->dp.renderbufferStorage = __glChipRenderbufferStorage;
    gc->dp.blitFramebufferBegin = __glChipBlitFramebufferBegin;
    gc->dp.blitFramebufferValidateState = __glChipBlitFramebufferValidateState;
    gc->dp.blitFramebufferEnd = __glChipBlitFramebufferEnd;
    gc->dp.blitFramebuffer = __glChipBlitFramebuffer;
    gc->dp.frameBufferTexture = __glChipFramebufferTexture;
    gc->dp.framebufferRenderbuffer = __glChipFramebufferRenderbuffer;
    gc->dp.isFramebufferComplete = __glChipIsFramebufferComplete;
    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_FBO_PREFER_MEM))
    {
        gc->dp.cleanTextureShadow = __glChipCleanTextureShadow;
        gc->dp.cleanRenderbufferShadow = __glChipCleanRenderbufferShadow;
    }

    /*
    gc->dp.invalidateFramebuffer = NULL;
    gc->dp.invalidateDrawable = NULL;
    */

    /* Query */
    gc->dp.beginQuery = __glChipBeginQuery;
    gc->dp.endQuery = __glChipEndQuery;
    gc->dp.getQueryObject = __glChipGetQueryObject;
    gc->dp.deleteQuery = __glChipDeleteQuery;

    /* Sync */
    gc->dp.createSync = __glChipCreateSync;
    gc->dp.deleteSync = __glChipDeleteSync;
    gc->dp.waitSync = __glChipWaitSync;
    gc->dp.syncImage = __glChipSyncImage;

    /* XFB */
    gc->dp.bindXFB = __glChipBindXFB;
    gc->dp.deleteXFB = __glChipDeleteXFB;
    gc->dp.beginXFB  = __glChipBeginXFB;
    gc->dp.endXFB  = __glChipEndXFB;
    gc->dp.pauseXFB = __glChipPauseXFB;
    gc->dp.resumeXFB = __glChipResumeXFB;
    gc->dp.getXfbVarying = __glChipGetXFBVarying;
    gc->dp.checkXFBBufSizes = __glChipCheckXFBBufSizes;

    gc->dp.getGraphicsResetStatus = __glChipGetGraphicsResetStatus;

    /* Multisample */
    gc->dp.getSampleLocation = __glChipGetSampleLocation;
    gc->dp.drawArraysIndirect = __glChipDrawArraysIndirect;
    gc->dp.drawElementsIndirect = __glChipDrawElementsIndirect;

    /* MultiDrawIndirect */
    gc->dp.multiDrawArraysIndirectEXT = __glChipMultiDrawArraysIndirect;
    gc->dp.multiDrawElementsIndirectEXT = __glChipMultiDrawElementsIndirect;

    /* Compute shader */
    gc->dp.computeBegin = __glChipComputeBegin;
    gc->dp.computeValidateState = __glChipComputeValidateState;
    gc->dp.computeEnd = __glChipComputeEnd;
    gc->dp.dispatchCompute = __glChipDispatchCompute;

    gc->dp.memoryBarrier = __glChipMemoryBarrier;

    gc->dp.blendBarrier = __glChipBlendBarrier;

    /* profiler */
#if VIVANTE_PROFILER
    gc->dp.profiler = __glChipProfiler;
#else
    gc->dp.profiler = NULL;
#endif

    /* Patches. */
#if __GL_CHIP_PATCH_ENABLED
    gc->dp.patchBlend = __glChipPatchBlend;
#else
    gc->dp.patchBlend = NULL;
#endif

    gc->dp.getError = __glChipGetError;

#if gcdPATTERN_FAST_PATH
    gc->dp.drawPattern = __glChipDrawPattern;
#endif

    gcmFOOTER_NO();
}

static GLvoid
gcChipInitExtension(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gctSIZE_T extLen = 0;
    gctSIZE_T extGLSLLen = 0;
    GLuint numExts = 0;
    __GLextension *curExt;
    __GLdeviceConstants *constants = &gc->constants;
    GLubyte *pCurFmt = gcvNULL;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* OES Extensions for context 2.0 */
    __glExtension[__GL_EXTID_OES_compressed_ETC1_RGB8_texture].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_compressed_paletted_texture].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_EGL_image].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_depth24].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_depth32].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_element_index_uint].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_fbo_render_mipmap].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_fragment_precision_high].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_rgb8_rgba8].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_texture_npot].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_vertex_half_float].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_depth_texture].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_depth_texture_cube_map].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_packed_depth_stencil].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_standard_derivatives].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_get_program_binary].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_mapbuffer].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_EGL_sync].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_vertex_array_object].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_required_internalformat].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_OES_surfaceless_context].bEnabled = GL_TRUE;

#ifdef ANDROID
    __glExtension[__GL_EXTID_OES_EGL_image_external].bEnabled = GL_TRUE;
#endif

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TX_BORDER_CLAMP) == gcvSTATUS_TRUE)
    {
        __glExtension[__GL_EXTID_EXT_texture_border_clamp].bEnabled = gcvTRUE;
        __glExtension[__GL_EXTID_OES_texture_border_clamp].bEnabled = gcvTRUE;
    }

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_VERTEX_10_10_10_2) == gcvSTATUS_TRUE)
    {
        __glExtension[__GL_EXTID_OES_vertex_type_10_10_10_2].bEnabled = GL_TRUE;
    }

    __glExtension[__GL_EXTID_EXT_texture_format_BGRA8888].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_blend_minmax].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_read_format_bgra].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_multi_draw_arrays].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_frag_depth].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_discard_framebuffer].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_multisampled_render_to_texture].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_robustness].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_texture_type_2_10_10_10_REV].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_texture_sRGB_decode].bEnabled = GL_TRUE;
    __glExtension[__GL_EXTID_EXT_texture_rg].bEnabled = GL_TRUE;

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_ANISOTROPIC_FILTERING) == gcvSTATUS_TRUE)
    {
        __glExtension[__GL_EXTID_EXT_texture_filter_anisotropic].bEnabled = GL_TRUE;
    }

    if (!gcdPROC_IS_WEBGL(chipCtx->patchId))
    {
        __glExtension[__GL_EXTID_VIV_tex_direct].bEnabled = GL_TRUE;
    }


    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_HALF_FLOAT_PIPE) == gcvSTATUS_TRUE)
    {
        __glExtension[__GL_EXTID_EXT_color_buffer_half_float].bEnabled = GL_TRUE;
        if (chipCtx->maxDrawRTs >= 8)
        {
            __glExtension[__GL_EXTID_EXT_color_buffer_float].bEnabled = GL_TRUE;
        }
    }

    /* extension enabled only for context 3.0 and later */
    if (constants->majorVersion == 3 && constants->minorVersion >= 0)
    {
        __glExtension[__GL_EXTID_OES_texture_half_float].bEnabled = GL_TRUE;
        __glExtension[__GL_EXTID_OES_texture_float].bEnabled = GL_TRUE;

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_MSAA_SHADING) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_OES_sample_shading].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_sample_variables].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_sample_variables].bGLSL    = gcvTRUE;
            __glExtension[__GL_EXTID_OES_shader_multisample_interpolation].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_shader_multisample_interpolation].bGLSL    = gcvTRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_BLT_ENGINE) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_copy_image].bEnabled = gcvFALSE;
            __glExtension[__GL_EXTID_OES_copy_image].bEnabled = gcvFALSE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_STENCIL_TEXTURE) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_OES_texture_stencil8].bEnabled = GL_TRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_SEPARATE_RT_CTRL) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_draw_buffers_indexed].bEnabled = GL_TRUE;
            __glExtension[__GL_EXTID_OES_draw_buffers_indexed].bEnabled = GL_TRUE;
        }

        if (chipCtx->patchId == gcvPATCH_GTFES30 || chipCtx->patchId == gcvPATCH_DEQP)
        {
            if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_ASTC) &&
                gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_ASTC_DECODE_FIX) == gcvSTATUS_TRUE)
            {
                __glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled = GL_TRUE;
            }
        }
        else
        {
            if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_ASTC) == gcvSTATUS_TRUE)
            {
                __glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled = GL_TRUE;
            }
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_HALTI2) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_OES_shader_image_atomic].bEnabled = GL_TRUE;
        }
    }

    if(constants->majorVersion < 3)
    {
        __glFormatInfoTable[__GL_FMT_RGB10_A2].renderable = gcvFALSE;
    }

    /* extension enabled only for context 3.1 and later */
    if (constants->majorVersion == 3 && constants->minorVersion >= 1)
    {
        __glExtension[__GL_EXTID_KHR_blend_equation_advanced].bEnabled = GL_TRUE;
        __glExtension[__GL_EXTID_OES_texture_storage_multisample_2d_array].bEnabled = GL_TRUE;
        __glExtension[__GL_EXTID_OES_shader_image_atomic].bEnabled = GL_TRUE;
        __glExtension[__GL_EXTID_KHR_debug].bEnabled = GL_TRUE;
        __glExtension[__GL_EXTID_KHR_robustness].bEnabled = GL_TRUE;

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_GEOMETRY_SHADER) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_geometry_point_size].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_geometry_shader].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_geometry_point_size].bEnabled = gcvTRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_CUBEMAP_ARRAY) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_texture_cube_map_array].bEnabled = gcvTRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_ADVANCED_SH_INST) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_gpu_shader5].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_gpu_shader5].bGLSL = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_shader_io_blocks].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_shader_implicit_conversions].bEnabled = gcvTRUE;

            __glExtension[__GL_EXTID_OES_gpu_shader5].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_gpu_shader5].bGLSL = gcvTRUE;
            __glExtension[__GL_EXTID_OES_shader_io_blocks].bEnabled = gcvTRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_MULTIDRAW_INDIRECT) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_multi_draw_indirect].bEnabled = GL_TRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_BUFFER) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_texture_buffer].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_texture_buffer].bGLSL = gcvTRUE;
            __glExtension[__GL_EXTID_OES_texture_buffer].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_texture_buffer].bGLSL = gcvTRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TESSELLATION) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_EXT_tessellation_point_size].bEnabled = gcvTRUE;

            __glExtension[__GL_EXTID_OES_tessellation_shader].bEnabled = gcvTRUE;
            __glExtension[__GL_EXTID_OES_tessellation_point_size].bEnabled = gcvTRUE;

            __glExtension[__GL_EXTID_EXT_primitive_bounding_box].bEnabled = GL_TRUE;
            __glExtension[__GL_EXTID_EXT_primitive_bounding_box].bGLSL    = GL_TRUE;
            __glExtension[__GL_EXTID_OES_primitive_bounding_box].bEnabled = GL_TRUE;
            __glExtension[__GL_EXTID_OES_primitive_bounding_box].bGLSL    = GL_TRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_DRAW_ELEMENTS_BASE_VERTEX) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_EXT_draw_elements_base_vertex].bEnabled = GL_TRUE;
            __glExtension[__GL_EXTID_OES_draw_elements_base_vertex].bEnabled = GL_TRUE;
        }

        if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_ROBUSTNESS) == gcvSTATUS_TRUE)
        {
            __glExtension[__GL_EXTID_KHR_robust_buffer_access_behavior].bEnabled = GL_TRUE;
        }
    }

    /* Change renderable format base on extension enable status */
    if (__glExtension[__GL_EXTID_EXT_color_buffer_half_float].bEnabled)
    {
        __glFormatInfoTable[__GL_FMT_R16F].renderable =
        __glFormatInfoTable[__GL_FMT_RG16F].renderable =
        __glFormatInfoTable[__GL_FMT_RGB16F].renderable =
        __glFormatInfoTable[__GL_FMT_RGBA16F].renderable = GL_TRUE;
    }

    if (__glExtension[__GL_EXTID_EXT_color_buffer_float].bEnabled)
    {
        __glFormatInfoTable[__GL_FMT_R32F].renderable =
        __glFormatInfoTable[__GL_FMT_RG32F].renderable =
        __glFormatInfoTable[__GL_FMT_RGBA32F].renderable =
        __glFormatInfoTable[__GL_FMT_R11F_G11F_B10F].renderable = GL_TRUE;
    }

    /*Generate compressed texture format table base on extension status*/
    if (constants->majorVersion >= 3)
    {
        constants->numCompressedTextureFormats = __GL_TABLE_SIZE(__glCompressedTextureFormats);
    }

    if (__glExtension[__GL_EXTID_OES_compressed_ETC1_RGB8_texture].bEnabled)
    {
        constants->numCompressedTextureFormats += __GL_TABLE_SIZE(__glCompressedTextureFormats_etc1);
    }


    if (__glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled)
    {
        constants->numCompressedTextureFormats += __GL_TABLE_SIZE(__glCompressedTextureFormats_astc);
    }

    if (__glExtension[__GL_EXTID_OES_compressed_paletted_texture].bEnabled)
    {
        constants->numCompressedTextureFormats += __GL_TABLE_SIZE(__glCompressedTextureFormats_palette);
    }

    if (constants->numCompressedTextureFormats > 0)
    {
        constants->pCompressedTexturesFormats = (GLint*)(*gc->imports.calloc)(NULL, 1, sizeof(GLint) * constants->numCompressedTextureFormats);
        pCurFmt = (GLubyte*)constants->pCompressedTexturesFormats;
    }

    if (constants->majorVersion >= 3)
    {
        __GL_MEMCOPY(pCurFmt, __glCompressedTextureFormats, sizeof(__glCompressedTextureFormats));
        pCurFmt += sizeof(__glCompressedTextureFormats);
    }

    if (__glExtension[__GL_EXTID_OES_compressed_ETC1_RGB8_texture].bEnabled)
    {
        __GL_MEMCOPY(pCurFmt, __glCompressedTextureFormats_etc1, sizeof(__glCompressedTextureFormats_etc1));
        pCurFmt += sizeof(__glCompressedTextureFormats_etc1);
    }


    if (__glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled)
    {
        __GL_MEMCOPY(pCurFmt, __glCompressedTextureFormats_astc, sizeof(__glCompressedTextureFormats_astc));
        pCurFmt += sizeof(__glCompressedTextureFormats_astc);
    }

    if (__glExtension[__GL_EXTID_OES_compressed_paletted_texture].bEnabled)
    {
        __GL_MEMCOPY(pCurFmt, __glCompressedTextureFormats_palette, sizeof(__glCompressedTextureFormats_palette));
        pCurFmt += sizeof(__glCompressedTextureFormats_palette);
    }


    /* Go through the extension table to get the extension string length */
    curExt = __glExtension;
    while (curExt->index < __GL_EXTID_EXT_LAST)
    {
        if (curExt->bEnabled)
        {
            /* Add one more space to separate extension strings*/
            extLen += (gcoOS_StrLen(curExt->name, gcvNULL) + 1);
            if (curExt->bGLSL)
            {
                extGLSLLen += (gcoOS_StrLen(curExt->name, gcvNULL) + 1);
            }
        }
        curExt++;
    }
    extLen++; /* One more for the null character */
    extGLSLLen++;

    /* Allocate buffer to hold the extension string */
    constants->extensions = (GLchar*)(*gc->imports.malloc)(gc, extLen);
    GL_ASSERT(constants->extensions != gcvNULL);
    constants->extensions[0] = '\0';

     /* Allocate buffer to hold the glsl extension string */
    constants->shaderCaps.extensions = (GLchar*)(*gc->imports.malloc)(gc, extGLSLLen);
    GL_ASSERT(constants->shaderCaps.extensions != gcvNULL);
    constants->shaderCaps.extensions[0] = '\0';

    /* Go through the extension table again to construct the extension string */
    curExt = __glExtension;
    while (curExt->index < __GL_EXTID_EXT_LAST)
    {
        if (curExt->bEnabled)
        {
            gcoOS_StrCatSafe(constants->extensions, extLen, curExt->name);
            /* Add one more space to separate extension strings */
            gcoOS_StrCatSafe(constants->extensions, extLen, " ");
            numExts++;
            if (curExt->bGLSL)
            {
                gcoOS_StrCatSafe(constants->shaderCaps.extensions, extGLSLLen, curExt->name);
                /* Add one more space to separate extension strings */
                gcoOS_StrCatSafe(constants->shaderCaps.extensions, extGLSLLen, " ");
            }
        }
        curExt++;
    }
    GL_ASSERT(extLen == gcoOS_StrLen(constants->extensions, gcvNULL) + 1);
    GL_ASSERT(extGLSLLen == gcoOS_StrLen(constants->shaderCaps.extensions, gcvNULL) + 1);
    constants->numExtensions = numExts;

    gcmFOOTER_NO();
}

static GLvoid
gcChipInitChipFeature(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    __GLchipFeature *chipFeature = &chipCtx->chipFeature;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Check whether IP can use RT tile format as texture */
    chipFeature->indirectRTT = !gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_RTT);

    /* Check whether IP has correct stencil support in depth-only mode. */
    chipFeature->hasCorrectStencil = gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_CORRECT_STENCIL);

    /* Check whether IP has tile status support. */
    chipFeature->hasTileStatus = gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_FAST_CLEAR);

    /* Patch strip if the bug fix is not available. */
    chipFeature->patchTriangleStrip = !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP);
    chipFeature->lineLoop = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_LINE_LOOP);

    chipFeature->primitiveRestart = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_PRIMITIVE_RESTART);

    chipFeature->wideLine = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_WIDE_LINE);

#if gcdUSE_WCLIP_PATCH
    chipCtx->wLimitPatch = !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_FRUSTUM_CLIP_FIX);
#endif

#if gcdUSE_NPOT_PATCH
    chipFeature->patchNP2Texture = !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NON_POWER_OF_TWO);
#endif

    chipFeature->msaaFragmentOperation = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_MSAA_FRAGMENT_OPERATION);

    /* Texture features. */
    chipFeature->hasYuv420Tiler = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_YUV420_TILER);
    chipFeature->hasYuvAssembler = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_YUV_ASSEMBLER);
    chipFeature->hasLinearTx = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_LINEAR);
    chipFeature->hasTxSwizzle = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_SWIZZLE);
    chipFeature->hasSupertiledTx = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_SUPERTILED_TEXTURE);
    chipFeature->hasTxTileStatus = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_TILE_STATUS_READ);
    chipFeature->hasTxDecompressor = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TX_DECOMPRESSOR);

    chipFeature->attrib2101010Rev = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_HALTI2);
    chipFeature->extendIntSign = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_INTEGER_SIGNEXT_FIX);

    chipFeature->hasTxDescriptor = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TX_DESCRIPTOR);

    chipFeature->hasBlitEngine = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_BLT_ENGINE);

    chipFeature->hasHwTFB = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_HW_TFB);

    chipFeature->txDefaultValueFix = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TX_DEFAULT_VALUE_FIX);

    chipFeature->hasCommandPrefetch = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_COMMAND_PREFETCH);

    chipFeature->hasYuvAssembler10bit = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TX_YUV_ASSEMBLER_10BIT);

    chipFeature->supportMSAA2X = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_SUPPORT_MSAA2X);

    chipFeature->hasSecurity = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_SECURITY);

    chipFeature->hasRobustness = gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_ROBUSTNESS);

    /* Get Halti support level */
    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HALTI5))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_6;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI4))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_5;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI3))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_4;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI2))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_3;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI1))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_2;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0))
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_1;
    }
    else
    {
        chipFeature->haltiLevel = __GL_CHIP_HALTI_LEVEL_0;
    }

    gcmFOOTER_NO();
}

/**********************************************************************/
/* Implementation for EXPORTED FUNCTIONS                              */
/**********************************************************************/

/*
** Try to detach surface from chip context and HAL.
*/
GLvoid
gcChipDetachSurface(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    gcoSURF *surfList,
    GLuint count
    )
{
    GLuint i, j, k;
    gcsSURF_VIEW nullView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x surfList=0x%x count=%u",
                   gc, chipCtx, surfList, count);

    for (i = 0; i < count; ++i)
    {
        GL_ASSERT(surfList[i]);
        /* RT */
        for (j = 0; j < gc->constants.shaderCaps.maxDrawBuffers; ++j)
        {
            if (chipCtx->drawRtViews[j].surf == surfList[i])
            {
                for (k = 0; k < chipCtx->rtHalMapping[j].numOfSlots; ++k)
                {
                     gcmVERIFY_OK(gco3D_UnsetTarget(chipCtx->engine,
                                                    chipCtx->rtHalMapping[j].slots[k],
                                                    chipCtx->drawRtViews[j].surf));
                }

                chipCtx->drawRtViews[j] = nullView;
            }
        }

        if (chipCtx->readRtView.surf == surfList[i])
        {
            chipCtx->readRtView = nullView;
        }

        if (chipCtx->drawDepthView.surf == surfList[i] || chipCtx->drawStencilView.surf == surfList[i])
        {
            gcmVERIFY_OK(gco3D_UnsetDepth(chipCtx->engine, chipCtx->drawDepthView.surf
                                                         ? chipCtx->drawDepthView.surf
                                                         : chipCtx->drawStencilView.surf));
        }

        /* Depth */
        if (chipCtx->drawDepthView.surf == surfList[i])
        {
            chipCtx->drawDepthView = nullView;
        }

        if (chipCtx->readDepthView.surf == surfList[i])
        {
            chipCtx->readDepthView = nullView;
        }

        /* Stencil */
        if (chipCtx->drawStencilView.surf == surfList[i])
        {
            /*FIXME: detach stencil from HAL, as stencil is bound with depth, need more consideration */
            chipCtx->drawStencilView = nullView;
        }

        if (chipCtx->readStencilView.surf == surfList[i])
        {
            chipCtx->readStencilView = nullView;
        }
    }

    gcmFOOTER_NO();
}

static gceSTATUS
gcChipInitDeafultObjects(
    __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLuint i;
    /* Initialize default texture binding */
    for (i = 0; i < __GL_MAX_TEXTURE_BINDINGS; ++i)
    {
        gcmONERROR(gcChipCreateTexture(gc, &gc->texture.defaultTextures[i]));
    }

    __glChipBindXFB(gc, gc->xfb.boundXfbObj);

OnError:
    return status;

}


GLboolean
__glChipMakeCurrent(
    __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    gcmONERROR(gco3D_Set3DEngine(chipCtx->engine));
    gcmONERROR(chipCtx->pfInitCompiler(chipCtx->hal, &gc->constants.shaderCaps));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipLoseCurrent(
    __GLcontext *gc,
    GLboolean bkickoffcmd
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    gcmONERROR(chipCtx->pfFinalizeCompiler(chipCtx->hal));
    gcmONERROR(gco3D_UnSet3DEngine(chipCtx->engine));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLvoid
__glChipQueryFormatInfo(
    __GLcontext *gc,
    __GLformat drvformat,
    GLint *numSamples,
    GLint *samples,
    GLint bufsize
    )
{
    __GLchipFmtMapInfo *formatMapInfo;
    GLint numbSamples;
    GLint size;

    gcmHEADER_ARG("gc=0x%x drvformat=%d numSamples=0x%x samples=0x%x bufsize=%d",
                   gc, drvformat, numSamples, samples, bufsize);

    GL_ASSERT(drvformat < __GL_FMT_MAX);

    formatMapInfo = gcChipGetFormatMapInfo(gc, drvformat, __GL_CHIP_FMT_PATCH_NONE);

    numbSamples = formatMapInfo->numSamples;

    if (numSamples)
    {
        *numSamples = numbSamples;
    }

    if (samples)
    {
        size = __GL_MIN((numbSamples?4:1), bufsize);
        size = __GL_MIN(size, numbSamples);
        if (size == 0)
        {
            __GL_MEMZERO(samples, bufsize * sizeof(GLint));
        }
        else
        {
            __GL_MEMCOPY(samples, formatMapInfo->samples, size * sizeof(GLint));
        }
    }

    gcmFOOTER_NO();
    return;
}

GLboolean
__glChipGetDeviceConstants(
    __GLcontext *gc,
    __GLdeviceConstants *constants
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    __GLSLStage stage;
    gcsGLSLCaps *shaderCaps = &constants->shaderCaps;
    gctBOOL tsAvailable;
    gctBOOL gsAvailable = gcvFALSE;
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;
    gctBOOL isEs31;

    gcmHEADER_ARG("gc=0x%x constants=0x%x", gc, constants);

    constants->vendor = __GL_VENDOR;
    gcmVERIFY_OK(gcoHAL_GetPatchID(gcvNULL, &patchId));
    gcmVERIFY_OK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

    tsAvailable = (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TESSELLATION));
    gsAvailable = (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_GEOMETRY_SHADER));

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HALTI5) &&
        tsAvailable &&
        gsAvailable)
    {
        gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION32);
        constants->GLSLVersion =__GL_GLSL_VERSION32;
        constants->majorVersion = 3;
        constants->minorVersion = 2;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI2) ||
        (chipModel == gcv900 && chipRevision == 0x5250))
    {
        if (patchId == gcvPATCH_ANTUTU6X)
        {
            gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION20);
        }
        else
        {
            gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION31);
        }
        constants->GLSLVersion =__GL_GLSL_VERSION31;
        constants->majorVersion = 3;
        constants->minorVersion = 1;
    }
    else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0))
    {
#if defined(ANDROID)
#if (ANDROID_SDK_VERSION < 19)
        if (patchId == gcvPATCH_OESCTS)
#else
        if ((patchId == gcvPATCH_OESCTS) && !gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI2))
#endif
        {
            gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION20);
            constants->GLSLVersion = __GL_GLSL_VERSION20;
            constants->majorVersion = 2;
        }
        else
#endif
        {
            if (patchId == gcvPATCH_GANGSTAR)
            {
                gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION20);
            }
            else
            {
                gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION30);
            }
            /* Antutu332 (same process name as Antutu402) need a "1.0.0" GLSL string for correct shader source */
            constants->GLSLVersion = (patchId == gcvPATCH_ANTUTU)
                                   ? __GL_GLSL_VERSION20
                                   : __GL_GLSL_VERSION30;
            constants->majorVersion = 3;
        }
        constants->minorVersion = 0;
    }
    else
    {
        /* If app request client=3, while hw cannot support, it should be blocked in EGL. */
        GL_ASSERT(gc->apiVersion == __GL_API_VERSION_ES20);

        gcoOS_StrCopySafe(constants->version, __GL_MAX_VERSION_LEN, __GL_VERSION20);
        constants->GLSLVersion = __GL_GLSL_VERSION20;
        constants->majorVersion = 2;
        constants->minorVersion = 0;
    }
    gcoOS_StrCatSafe(constants->version, __GL_MAX_VERSION_LEN, gcvVERSION_STRING);

    isEs31 = (constants->majorVersion >= 3) && (constants->minorVersion >= 1);


    constants->maxTextureArraySize  = __GL_MAX_HW_ARRAY_SIZE;
    constants->maxTextureDepthSize  = __GL_MAX_HW_DEPTH_SIZE;

    constants->lineWidthMin = 1.0f;
    constants->lineWidthMax = gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_WIDE_LINE) ? 16.0f : 1.0f;

    constants->numberofQueryCounterBits = __GL_QUERY_COUNTER_BITS;

    constants->maxViewportWidth  = __GL_MAX_VIEWPORT_WIDTH ;
    constants->maxViewportHeight = __GL_MAX_VIEWPORT_HEIGHT;

    shaderCaps->minProgramTexelOffset = __GL_MIN_PROGRAM_TEXEL_OFFSET;
    shaderCaps->maxProgramTexelOffset = __GL_MAX_PROGRAM_TEXEL_OFFSET;

    shaderCaps->minProgramTexGatherOffset = __GL_MIN_PROGRAM_TEXGATHER_OFFSET;
    shaderCaps->maxProgramTexGatherOffset = __GL_MAX_PROGRAM_TEXGATHER_OFFSET;

    shaderCaps->uniformBufferOffsetAlignment = 4;
    shaderCaps->maxVertUniformBlocks = 16;
    shaderCaps->maxFragUniformBlocks = 16;
    shaderCaps->maxCmptUniformBlocks = 16;

    if (tsAvailable)
    {
        shaderCaps->maxTcsUniformBlocks = 16;
        shaderCaps->maxTesUniformBlocks = 16;
    }

    if (gsAvailable)
    {
        shaderCaps->maxGsUniformBlocks = 16;
    }

    shaderCaps->maxCombinedUniformBlocks = __GL_MAX((shaderCaps->maxVertUniformBlocks +
                                                     shaderCaps->maxFragUniformBlocks +
                                                     shaderCaps->maxTcsUniformBlocks  +
                                                     shaderCaps->maxTesUniformBlocks  +
                                                     shaderCaps->maxGsUniformBlocks),
                                                     shaderCaps->maxCmptUniformBlocks);

    shaderCaps->maxUniformBufferBindings = isEs31 ? __GL_MAX(shaderCaps->maxCombinedUniformBlocks, 36)
                                                  : shaderCaps->maxCombinedUniformBlocks;

    /* In basic machine unit */
    shaderCaps->maxUniformBlockSize = 65536;

    shaderCaps->maxVertAtomicCounters = 16;
    shaderCaps->maxFragAtomicCounters = 16;
    shaderCaps->maxCmptAtomicCounters = 16;
    if (tsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxTcsAtomicCounters = 16;
        shaderCaps->maxTesAtomicCounters = 16;
    }
    if (gsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxGsAtomicCounters = 16;
    }
    shaderCaps->maxCombinedAtomicCounters = __GL_MAX(shaderCaps->maxVertAtomicCounters +
                                                     shaderCaps->maxFragAtomicCounters +
                                                     shaderCaps->maxTcsAtomicCounters  +
                                                     shaderCaps->maxTesAtomicCounters  +
                                                     shaderCaps->maxGsAtomicCounters,
                                                     shaderCaps->maxCmptAtomicCounters);
    shaderCaps->maxAtomicCounterBufferBindings = shaderCaps->maxCombinedAtomicCounters;
    shaderCaps->maxAtomicCounterBufferSize = shaderCaps->maxCombinedAtomicCounters * sizeof(GLuint);

    shaderCaps->shaderStorageBufferOffsetAlignment = 4;
    shaderCaps->maxVertShaderStorageBlocks = 16;
    shaderCaps->maxFragShaderStorageBlocks = 16;
    shaderCaps->maxCmptShaderStorageBlocks = 16;
    if (tsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxTcsShaderStorageBlocks = 16;
        shaderCaps->maxTesShaderStorageBlocks = 16;
    }
    if (gsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxGsShaderStorageBlocks = 16;
    }
    shaderCaps->maxCombinedShaderStorageBlocks = __GL_MAX(shaderCaps->maxVertShaderStorageBlocks +
                                                          shaderCaps->maxFragShaderStorageBlocks +
                                                          shaderCaps->maxTcsShaderStorageBlocks  +
                                                          shaderCaps->maxTesShaderStorageBlocks  +
                                                          shaderCaps->maxGsShaderStorageBlocks,
                                                          shaderCaps->maxCmptShaderStorageBlocks);

    shaderCaps->maxShaderStorageBufferBindings = shaderCaps->maxCombinedShaderStorageBlocks;
    shaderCaps->maxShaderBlockSize = 1 << 27;

    shaderCaps->maxXfbInterleavedComponents = __GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS;
    shaderCaps->maxXfbSeparateComponents = __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS;
    shaderCaps->maxXfbSeparateAttribs = __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS;

    constants->numShaderBinaryFormats = __GL_TABLE_SIZE(__glShaderBinaryFormats_viv);
    constants->pShaderBinaryFormats = __glShaderBinaryFormats_viv;
    constants->numProgramBinaryFormats = __GL_TABLE_SIZE(__glProgramBinaryFormats_viv);
    constants->pProgramBinaryFormats = __glProgramBinaryFormats_viv;

    shaderCaps->maxVertexImageUniform    = 8;
    shaderCaps->maxFragImageUniform      = 8;
    shaderCaps->maxCmptImageUniform      = 8;
    if (tsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxTcsImageUniform = 8;
        shaderCaps->maxTesImageUniform = 8;
    }
    if (gsAvailable)
    {
        /* minimal is 0 in spec */
        shaderCaps->maxGsImageUniform = 8;
    }
    shaderCaps->maxCombinedImageUniform  = __GL_MAX(shaderCaps->maxVertexImageUniform +
                                                    shaderCaps->maxFragImageUniform   +
                                                    shaderCaps->maxTcsImageUniform    +
                                                    shaderCaps->maxTesImageUniform    +
                                                    shaderCaps->maxGsImageUniform,
                                                    shaderCaps->maxCmptImageUniform);

    shaderCaps->maxImageUnit = shaderCaps->maxCombinedImageUniform;

    if (gcvFALSE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HELPER_INVOCATION)
       )
    {
        shaderCaps->maxFragAtomicCounters = 0;
        shaderCaps->maxFragImageUniform = 0;
        shaderCaps->maxFragShaderStorageBlocks = 0;
    }

    shaderCaps->maxCombinedShaderOutputResource = shaderCaps->maxCombinedShaderStorageBlocks + shaderCaps->maxCombinedImageUniform;

    /* cs-only constants, others are set with vs/ps */
    shaderCaps->maxWorkGroupCount[0] = 65535;
    shaderCaps->maxWorkGroupCount[1] = 65535;
    shaderCaps->maxWorkGroupCount[2] = 65535;
    shaderCaps->maxWorkGroupSize[0] = 128;
    shaderCaps->maxWorkGroupSize[1] = 128;
    shaderCaps->maxWorkGroupSize[2] = 64;
    shaderCaps->maxWorkGroupInvocation = 128;
    shaderCaps->maxShareMemorySize = 32768;


    /*
    ** shader precision
    */
    for (stage = 0; stage < __GLSL_STAGE_LAST; ++stage)
    {
        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_FLOAT].rangeLow =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_FLOAT].rangeLow =
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_FLOAT].rangeLow = 127;

        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_FLOAT].rangeHigh =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_FLOAT].rangeHigh =
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_FLOAT].rangeHigh = 127;

        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_FLOAT].precision =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_FLOAT].precision =
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_FLOAT].precision = 23;

        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_INT].rangeLow =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_INT].rangeLow = 31;
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_INT].rangeLow = 32;

        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_INT].rangeHigh =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_INT].rangeHigh = 31;
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_INT].rangeHigh = 32;

        constants->shaderPrecision[stage][__GLSL_TYPE_LOW_INT].precision =
        constants->shaderPrecision[stage][__GLSL_TYPE_MEDIUM_INT].precision =
        constants->shaderPrecision[stage][__GLSL_TYPE_HIGH_INT].precision = 0;
    }

    do
    {
        gctUINT maxElementIndex;
        gctUINT unifiedUniforms = 0;
        gctUINT minVertUniforms = (constants->majorVersion < 3) ? 128 : 256;
        gctUINT minFragUniforms = (constants->majorVersion < 3) ?  16 : (isEs31 ? 256 : 224);
        gctUINT minCmptUniforms = (constants->majorVersion < 3 && constants->minorVersion < 1) ? 0 : 128;
        gctUINT minTcsUniforms  = tsAvailable ? 256 : 0;
        gctUINT minTesUniforms  = tsAvailable ? 256 : 0;
        gctUINT minGsUniforms   = gsAvailable ? 256 : 0;

        if (gc->apiVersion == __GL_API_VERSION_ES20 &&
            (gcvPATCH_OES20SFT == patchId ||
             gcvPATCH_GLBM27 == patchId   ||
             gcvPATCH_GFXBENCH == patchId ||
             gcdPROC_IS_WEBGL(patchId)))
        {
            minVertUniforms = 128;
            minFragUniforms = 16;
        }

        shaderCaps->maxBuildInVertAttributes = 2;

        gcmERR_BREAK(gcoHAL_QueryStreamCaps(gcvNULL,
                                            &shaderCaps->maxUserVertAttributes,
                                            &constants->maxVertexAttribStride,
                                            &constants->maxStreams,
                                            gcvNULL,
                                            &constants->maxVertexAttribRelativeOffset));

        constants->maxVertexAttribStride = __GL_MAX(constants->maxVertexAttribStride, 2048);
        shaderCaps->maxVertAttributes = shaderCaps->maxBuildInVertAttributes +
                                        shaderCaps->maxUserVertAttributes;
        /* vertex attribute limits */
        constants->maxVertexAttribBindings = shaderCaps->maxUserVertAttributes;
        /* The caps appears from ES3.1, the minimal value is 2047. */
        constants->maxVertexAttribRelativeOffset = __GL_MAX(2047, constants->maxVertexAttribRelativeOffset);

        /* Get the target maximum size. */
        gcmERR_BREAK(gcoHAL_QueryTargetCaps(gcvNULL,
                                            &constants->maxRenderBufferSize,
                                            gcvNULL,
                                            &shaderCaps->maxDrawBuffers,
                                            &constants->maxSamples));

        shaderCaps->maxDrawBuffers = __GL_MAX(shaderCaps->maxDrawBuffers / 2, 4);
        if (shaderCaps->maxDrawBuffers > __GL_MAX_DRAW_BUFFERS)
        {
            __GLES_PRINT("ERROR: please define __GL_MAX_DRAW_BUFFERS no less than %d\n", shaderCaps->maxDrawBuffers);
            gcmERR_BREAK(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        shaderCaps->maxSamples = constants->maxSamples;
        constants->maxSamplesInteger = constants->maxSamples;

        /* Get the texture max dimensions.
        ** Tex unit counts are designed to be same as sampler counts
        */
        gcmERR_BREAK(gcoHAL_QueryTextureCaps(gcvNULL,
                                             &constants->maxTextureSize,
                                             gcvNULL,
                                             gcvNULL,
                                             gcvNULL,
                                             gcvNULL,
                                             &shaderCaps->maxVertTextureImageUnits,
                                             &shaderCaps->maxFragTextureImageUnits));

        if (shaderCaps->maxFragTextureImageUnits < 8)
        {
            shaderCaps->maxFragTextureImageUnits = 8;
        }

        gcmERR_BREAK(gcoHAL_QueryTextureMaxAniso(gcvNULL,
                                                 &constants->maxAnistropic));


        shaderCaps->maxCmptTextureImageUnits  = shaderCaps->maxFragTextureImageUnits;

        if (tsAvailable)
        {
            shaderCaps->maxTcsTextureImageUnits = 16;
            shaderCaps->maxTesTextureImageUnits = 16;
        }
        if (gsAvailable)
        {
            shaderCaps->maxGsTextureImageUnits = 16;
        }

        /* Combined texture image units equals to CS+VS+PS+TCS+TES+GS texture image units */
        shaderCaps->maxCombinedTextureImageUnits = shaderCaps->maxCmptTextureImageUnits +
                                                   shaderCaps->maxVertTextureImageUnits +
                                                   shaderCaps->maxFragTextureImageUnits +
                                                   shaderCaps->maxTcsTextureImageUnits  +
                                                   shaderCaps->maxTesTextureImageUnits  +
                                                   shaderCaps->maxGsTextureImageUnits;

        /* Max texture samplers equals to VS+PS+TCS+TES+GS texture image units */
        shaderCaps->maxTextureSamplers = shaderCaps->maxVertTextureImageUnits +
                                         shaderCaps->maxFragTextureImageUnits +
                                         shaderCaps->maxTcsTextureImageUnits  +
                                         shaderCaps->maxTesTextureImageUnits  +
                                         shaderCaps->maxGsTextureImageUnits;

        /* HW's combined vs+ps samplers must be less than or equal to predefined GLcore
        ** macro __GL_MAX_GLSL_SAMPLERS, otherwise please enlarge the macro.
        */
        GL_ASSERT(shaderCaps->maxTextureSamplers <= __GL_MAX_GLSL_SAMPLERS);

        gcmERR_BREAK(gcoINDEX_QueryCaps(gcvNULL, gcvNULL, gcvNULL, &maxElementIndex));

        constants->maxElementIndex = (GLuint64)maxElementIndex;

        /* Get the shader variable maximum size. */
        gcmERR_BREAK(gcoHAL_QueryShaderCaps(gcvNULL,
                                            &unifiedUniforms,
                                            &shaderCaps->maxVertUniformVectors,
                                            &shaderCaps->maxFragUniformVectors,
                                            &shaderCaps->maxVaryingVectors,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL));

        /* maxVertOut and maxFragIn count gl_position in, while maxVarying does not.
        ** TODO: gl_position used 2 varyings on some chips, need to change interface of gcoHAL_QueryShaderCaps()
        **       for accurate maxVertOut and maxFragIn.
        */
        shaderCaps->maxVertOutVectors = shaderCaps->maxFragInVectors = shaderCaps->maxVaryingVectors + 1;

        if (tsAvailable)
        {
            shaderCaps->maxTcsOutVectors = 32;
            shaderCaps->maxTesOutVectors = 32;
            shaderCaps->maxTcsInVectors  = 32;
            shaderCaps->maxTesInVectors  = 32;
        }
        if (gsAvailable)
        {
            shaderCaps->maxGsOutVectors = 32;
            shaderCaps->maxGsInVectors  = 16;
        }

        /* Unified uniforms, maxUniformVectors will be calculated by number of unified uniforms. */
        if (unifiedUniforms > 0)
        {
            /* For enough unified uniforms, allocate vs/fs uniforms in proportion. */
            if (unifiedUniforms > minVertUniforms + minFragUniforms)
            {
                shaderCaps->maxVertUniformVectors = unifiedUniforms * minVertUniforms / (minVertUniforms + minFragUniforms);
                shaderCaps->maxFragUniformVectors = unifiedUniforms - shaderCaps->maxVertUniformVectors;
            }
            else
            {
                shaderCaps->maxVertUniformVectors = minVertUniforms;
                shaderCaps->maxFragUniformVectors = minFragUniforms;
            }

            if (tsAvailable)
            {
                shaderCaps->maxTcsUniformVectors = minTcsUniforms;
                shaderCaps->maxTesUniformVectors = minTesUniforms;
            }

            if (gsAvailable)
            {
                shaderCaps->maxGsUniformVectors = minGsUniforms;
            }

            shaderCaps->maxCmptUniformVectors = __GL_MAX(unifiedUniforms, minCmptUniforms);
        }
        else
        {
            /* The caps should meet spec minimum requirements. */
            shaderCaps->maxVertUniformVectors = __GL_MAX(minVertUniforms, shaderCaps->maxVertUniformVectors);
            shaderCaps->maxFragUniformVectors = __GL_MAX(minFragUniforms, shaderCaps->maxFragUniformVectors);
            shaderCaps->maxCmptUniformVectors = __GL_MAX(minCmptUniforms, shaderCaps->maxFragUniformVectors);

            if (tsAvailable)
            {
                shaderCaps->maxTcsUniformVectors = __GL_MAX(minTcsUniforms, shaderCaps->maxVertUniformVectors);
                shaderCaps->maxTesUniformVectors = __GL_MAX(minTesUniforms, shaderCaps->maxVertUniformVectors);
            }

            if (gsAvailable)
            {
                shaderCaps->maxGsUniformVectors = __GL_MAX(minGsUniforms, shaderCaps->maxVertUniformVectors);
            }

        }

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



        /* Determine the maximum uniform locations.
        ** Should work if all locatable uniforms are used as float/int/sampler/image.
        */
        shaderCaps->maxUniformLocations = shaderCaps->maxCombinedTextureImageUnits
                                        + shaderCaps->maxCombinedImageUniform
                                        + 4 * __GL_MAX(shaderCaps->maxCmptUniformVectors,
                                                       shaderCaps->maxVertUniformVectors + shaderCaps->maxFragUniformVectors +
                                                       shaderCaps->maxTcsUniformVectors + shaderCaps->maxTesUniformVectors);

        /* Should meet spec minimum 1024 requirements */
        shaderCaps->maxUniformLocations = __GL_MAX(shaderCaps->maxUniformLocations, 1024);

    } while (GL_FALSE);

    constants->maxNumTextureLevels = __glFloorLog2(constants->maxTextureSize) + 1;

    /* Assert reported caps are less than macro */
    GL_ASSERT(shaderCaps->maxUserVertAttributes <= __GL_MAX_VERTEX_ATTRIBUTES);
    GL_ASSERT(constants->maxVertexAttribBindings <= __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS);

    if (tsAvailable)
    {
        shaderCaps->maxTessPatchVertices = 32;
        shaderCaps->maxTessGenLevel = 64;
        shaderCaps->maxTcsOutPatchVectors = 30;
        shaderCaps->maxTcsOutTotalVectors = 1024;
        shaderCaps->tessPatchPR = gcvTRUE;
    }

    if (gsAvailable)
    {
        shaderCaps->maxGsOutTotalVectors = 256;
        shaderCaps->maxGsOutVertices = 256;
        shaderCaps->maxGsInvocationCount = 32;
        shaderCaps->provokingVertex = gcvPROVOKING_VERTEX_UNDEFINE;
        constants->gsLayerProvokingVertex = GL_UNDEFINED_VERTEX_EXT;
    }

    constants->minFragmentInterpolationOffset = -0.5f;
    constants->maxFragmentInterpolationOffset = 0.5f;
    constants->fragmentInterpolationOffsetBits = 4;

    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
}

GLboolean
__glChipDestroyContext(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    GL_ASSERT(gc->imports.config);

    if (gc->constants.extensions)
    {
        (*gc->imports.free)(gc, gc->constants.extensions);
        gc->constants.extensions = gcvNULL;
    }

    if (gc->constants.shaderCaps.extensions)
    {
        (*gc->imports.free)(gc, gc->constants.shaderCaps.extensions);
        gc->constants.shaderCaps.extensions = gcvNULL;
    }


    gcmVERIFY_OK(gcChipDeinitializeSampler(gc));
    gcmVERIFY_OK(gcChipDeinitializeDraw(gc, chipCtx));
    gcmVERIFY_OK(gcChipLTCReleaseResultArray(chipCtx, gcvNULL));
    gcmVERIFY_OK(gcChipReleaseCompiler(gc));
    (*gc->imports.free)(0, gc->constants.pCompressedTexturesFormats);
#if VIVANTE_PROFILER
    /* Destroy the profiler. */
    gcChipDestroyProfiler(gc);
#endif

    if (chipCtx->rtTexture)
    {
        gcmVERIFY_OK(gcoTEXTURE_Destroy(chipCtx->rtTexture));
    }
    /* Free index temporary buffer */
    if (chipCtx->tempIndexBuffer != gcvNULL)
    {
        (*gc->imports.free)(0, chipCtx->tempIndexBuffer);

    }

    /* Free tmp allocate attib shadow mem */
    if (chipCtx->anyAttibConverted)
    {
        gcChipPatchFreeTmpAttibMem(gc);
    }

    if (chipCtx->cmdInstaceCache)
    {
        gcChipUtilsHashDestory(gc, chipCtx->cmdInstaceCache);
    }

    if (chipCtx->pgKeyState)
    {
        gcChipPgStateKeyFree(gc, &chipCtx->pgKeyState);
    }

    gcmVERIFY_OK(gco3D_Destroy(chipCtx->engine));

    gcmVERIFY_OK(gcoHAL_Destroy(chipCtx->hal));

    gcmVERIFY_OK(gcoOS_Destroy(chipCtx->os));

    gcmVERIFY_OK(gcSHADER_FreeRecompilerLibrary());
    gcmVERIFY_OK(gcSHADER_FreeBlendLibrary());

    dpGlobalInfo.numContext--;

    if (chipCtx->patchId == gcvPATCH_GLBM25   ||
        chipCtx->patchId == gcvPATCH_GLBM27   ||
        chipCtx->patchId == gcvPATCH_GFXBENCH ||
        chipCtx->patchId == gcvPATCH_OES20SFT ||
        chipCtx->patchId == gcvPATCH_OES30SFT ||
        chipCtx->patchId == gcvPATCH_DEQP     ||
        gcdPROC_IS_WEBGL(chipCtx->patchId)
        )
    {
        gcmVERIFY_OK(gcoHAL_SetTimeOut(chipCtx->hal, gcdGPU_TIMEOUT));
    }

    (*gc->imports.free)(0, chipCtx);

    gc->dp.privateData = NULL;

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}


GLboolean
__glChipCreateContext(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    __GLdeviceConstants *constants = &gc->constants;

    gcmHEADER_ARG("gc=0x%x", gc);

    GL_ASSERT(gc->imports.config);

    chipCtx = (__GLchipContext*)(*gc->imports.calloc)(NULL, 1, sizeof(__GLchipContext));
    if (!chipCtx)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
    gc->dp.privateData = chipCtx;

    gcmONERROR(gcoOS_Construct(gcvNULL, &chipCtx->os));
    gcmONERROR(gcoHAL_Construct(gcvNULL, chipCtx->os, &chipCtx->hal));

    /* Get PatchID from HAL in the very beginning */
    gcmONERROR(gcoHAL_GetPatchID(chipCtx->hal, &chipCtx->patchId));

    /* Query chip identity. */
    gcmONERROR(gcoHAL_QueryChipIdentity(chipCtx->hal,
                                        &chipCtx->chipModel,
                                        &chipCtx->chipRevision,
                                        gcvNULL,
                                        gcvNULL));

    if (chipCtx->chipModel == 0)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Construct chip name. */
    gcChipConstructChipName(chipCtx);
    gc->constants.renderer = chipCtx->chipName;

    /* Get the target maximum size. */
    gcmONERROR(gcoHAL_QueryTargetCaps(gcvNULL,
                                      gcvNULL,
                                      gcvNULL,
                                      &chipCtx->maxDrawRTs,
                                      gcvNULL));

    if (chipCtx->maxDrawRTs > gcdMAX_DRAW_BUFFERS)
    {
        __GLES_PRINT("ERROR: please define gcdMAX_DRAW_BUFFERS no less than %d\n", chipCtx->maxDrawRTs);
        gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
    }

    gcChipInitChipFeature(gc, chipCtx);

    /* Only enable stencil opt for those conformance tests */
    chipCtx->needStencilOpt = chipCtx->patchId == gcvPATCH_GTFES30 ||
                              chipCtx->patchId == gcvPATCH_DEQP    ||
                              chipCtx->patchId == gcvPATCH_ANTUTUGL3;

#if VIVANTE_PROFILER
    if (__glesApiProfileMode >= 1)
    {
        gcChipInitProfileDevicePipeline(gc, chipCtx);
    }
    else
#endif
    {
        gcChipInitDevicePipeline(gc, chipCtx);
    }

    /* Get the 3D engine pointer. */
    gcmONERROR(gco3D_Construct(chipCtx->hal, gc->imports.robustAccess, &chipCtx->engine));

    /* Set API type to OpenGL. */
    gcmONERROR(gco3D_SetAPI(chipCtx->engine, (__GL_API_VERSION_ES20 == gc->apiVersion) ?
                                             gcvAPI_OPENGL_ES20 : gcvAPI_OPENGL_ES30));

    /*
    ** - For ES30 GTF, limit MAX_RENDERBUFFER_SIZE to spec minimum required value to speed up submission
    ** - ES30 conform requires at least 256 vs uniforms and 224 ps ones.
    */
    if (gc->apiVersion == __GL_API_VERSION_ES30 && chipCtx->patchId == gcvPATCH_GTFES30)
    {
        /* Reduce FBO size to shorten CTS time */
        constants->maxTextureSize = 2048;
        constants->maxRenderBufferSize = 2048;
        constants->maxNumTextureLevels = __glFloorLog2(constants->maxTextureSize) + 1;

        constants->shaderCaps.maxVertUniformVectors = constants->shaderCaps.maxVertUniformVectors > 256 ?
                                                      constants->shaderCaps.maxVertUniformVectors : 256;
        constants->shaderCaps.maxFragUniformVectors = constants->shaderCaps.maxFragUniformVectors > 224 ?
                                                      constants->shaderCaps.maxFragUniformVectors : 224;
        constants->shaderCaps.maxCmptUniformVectors = constants->shaderCaps.maxFragUniformVectors;    /* ES31 require minimum 128 */

        constants->shaderCaps.maxCombinedVertUniformComponents = constants->shaderCaps.maxVertUniformVectors * 4
                                                               + constants->shaderCaps.maxVertUniformBlocks * constants->shaderCaps.maxUniformBlockSize / 4;
        constants->shaderCaps.maxCombinedFragUniformComponents = constants->shaderCaps.maxFragUniformVectors * 4
                                                               + constants->shaderCaps.maxFragUniformBlocks * constants->shaderCaps.maxUniformBlockSize / 4;
        constants->shaderCaps.maxCombinedCmptUniformComponents = constants->shaderCaps.maxCmptUniformVectors * 4
                                                               + constants->shaderCaps.maxCmptUniformBlocks * constants->shaderCaps.maxUniformBlockSize / 4;
    }

    if (chipCtx->chipFeature.wideLine)
    {
        gcmONERROR(gco3D_SetAntiAliasLine(chipCtx->engine, gcvTRUE));
    }

    /* Set lastPixel Disable. */
    gcmONERROR(gco3D_SetLastPixelEnable(chipCtx->engine, gcvFALSE));

    gcmONERROR(gco3D_SetDepthOnly(chipCtx->engine, gcvFALSE));

    /* Turn on early-depth. */
    gcmONERROR(gco3D_SetEarlyDepth(chipCtx->engine, gcvTRUE));

    /* ES2 always uses SOLID fill mode */
    gcmONERROR(gco3D_SetFill(chipCtx->engine, gcvFILL_SOLID));

    gcmONERROR(gco3D_SetAlphaTest(chipCtx->engine, gcvFALSE));

    gcmONERROR(gco3D_SetShading(chipCtx->engine, gcvSHADING_SMOOTH));

#if gcdUSE_WCLIP_PATCH
    /* For WClipping. */
    chipCtx->wLimitRms = 0.0f;
    chipCtx->wLimitRmsDirty = gcvFALSE;
    chipCtx->clipWValue = 0.4f;
    chipCtx->wLimitZNear = 0.0f;
    chipCtx->wLimitSettled = gcvFALSE;
    chipCtx->computeWlimitByVertex = gcvFALSE;
    chipCtx->wLimitComputeLimit = 0x7FFFFFFF;
    chipCtx->wLimitSampleCount = 16;
    chipCtx->wLimitPSC = gcvFALSE;

    switch (chipCtx->patchId)
    {
    case gcvPATCH_GLOFTSXHM:
    case gcvPATCH_XRUNNER:
    case gcvPATCH_WISTONESG:
        chipCtx->clipW = gcvTRUE;
        break;
    case gcvPATCH_A8HP:
        chipCtx->computeWlimitByVertex = gcvTRUE;
        break;
    case gcvPATCH_AIRNAVY:
    case gcvPATCH_F18NEW:
        chipCtx->computeWlimitByVertex = gcvTRUE;
        chipCtx->wLimitComputeLimit = 44;
        break;
    case gcvPATCH_NAMESGAS:
    case gcvPATCH_TITANPACKING:
        chipCtx->clipW = gcvTRUE;
        chipCtx->clipWValue = 0.01f;
        break;
    case gcvPATCH_RIPTIDEGP2:
    case gcvPATCH_AFTERBURNER:
    case gcvPATCH_CHROME:
    case gcvPATCH_FIREFOX:
    case gcvPATCH_ANDROID_WEBGL:
    case gcvPATCH_BUSPARKING3D:
    case gcvPATCH_F18:
        chipCtx->computeWlimitByVertex = gcvTRUE;
        break;
    default:
        break;
    }

    if(chipCtx->patchId == gcvPATCH_INVALID)
    {
        chipCtx->computeWlimitByVertex = gcvTRUE;
    }
#endif

#if __GL_CHIP_PATCH_ENABLED
    chipCtx->patchInfo.bufBindDirty = GL_TRUE;
#endif

#if gcdUSE_NPOT_PATCH
    gcoHAL_SetBltNP2Texture(chipCtx->chipFeature.patchNP2Texture);
#endif


    /* LTC */
    chipCtx->curLTCResultArraySize = 0;
    chipCtx->cachedLTCResultArray  = gcvNULL;

    chipCtx->depthMode = gcvDEPTH_NONE;

    gcChipInitExtension(gc, chipCtx);
    gcmONERROR(gcChipPgStateKeyAlloc(gc, &chipCtx->pgKeyState));
    gcmONERROR(gcChipInitFormatMapInfo(gc));
    gcmONERROR(gcChipInitializeDraw(gc, chipCtx));
    gcmONERROR(gcChipInitializeSampler(gc));
    gcmONERROR(gcChipLoadCompiler(gc));
    gcmONERROR(gcChipInitDeafultObjects(gc));
#if VIVANTE_PROFILER
    gcChipInitializeProfiler(gc);
#endif

    dpGlobalInfo.numContext++;

    if (chipCtx->patchId == gcvPATCH_GLBM25   ||
        chipCtx->patchId == gcvPATCH_GLBM27   ||
        chipCtx->patchId == gcvPATCH_GFXBENCH ||
        chipCtx->patchId == gcvPATCH_OES20SFT ||
        chipCtx->patchId == gcvPATCH_OES30SFT ||
        chipCtx->patchId == gcvPATCH_DEQP
       )
    {
        /* Some cases need more time for GPU rendering.
        ** Set 10 times of gcdGPU_TIMEOUT.
        */
        gcoHAL_SetTimeOut(chipCtx->hal, 10 * gcdGPU_TIMEOUT);
    }

    if (gcdPROC_IS_WEBGL(chipCtx->patchId))
    {
        gcoHAL_SetTimeOut(chipCtx->hal, gcvINFINITE);
    }

#if __GL_CHIP_PATCH_ENABLED
    gcChipGetPatchConditionTb(gc);
#endif

    gcmONERROR(gcChipGetSampleLocations(gc, chipCtx));

    chipCtx->robust = (gc->imports.robustAccess && chipCtx->chipFeature.hasRobustness);

OnError:
    if (gcmIS_ERROR(status) && chipCtx)
    {
        if (chipCtx->pgKeyState)
        {
            gcmVERIFY_OK(gcChipPgStateKeyFree(gc, &chipCtx->pgKeyState));
        }
        if (chipCtx->engine)
        {
            gcmVERIFY_OK(gco3D_Destroy(chipCtx->engine));
        }
        if (chipCtx->hal)
        {
            gcmVERIFY_OK(gcoHAL_Destroy(chipCtx->hal));
        }

        if (chipCtx->os)
        {
            gcmVERIFY_OK(gcoOS_Destroy(chipCtx->os));
        }

        (gc->imports.free)(NULL, chipCtx);
        chipCtx = gcvNULL;

        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;

    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

}

GLenum
__glChipGetGraphicsResetStatus(
    __GLcontext *gc
    )
{
    GLenum retVal = GL_NO_ERROR;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (gc->imports.resetNotification == GL_LOSE_CONTEXT_ON_RESET_EXT)
    {
        __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
        gceSTATUS status;
        gctBOOL innocent = gcvTRUE;

        status = gco3D_QueryReset(chipCtx->engine, &innocent);

        if (gcvSTATUS_TRUE == status)
        {
            /* GPU was reseted */
            if (chipCtx->chipFeature.hasSecurity)
            {
                if (innocent)
                {
                    retVal = GL_INNOCENT_CONTEXT_RESET;
                }
                else
                {
                    retVal = GL_GUILTY_CONTEXT_RESET;
                }
            }
            else
            {
                retVal = GL_UNKNOWN_CONTEXT_RESET;
            }
        }
    }

    gcmFOOTER_ARG("return=0x%04x", retVal);
    return retVal;
}

gceSTATUS
gcChipSetError(
    __GLchipContext *chipCtx,
    gceSTATUS errorStatus
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("chipCtx=0x%x status=%d", chipCtx, errorStatus);

    if (errorStatus != gcvSTATUS_OK &&
        chipCtx->errorStatus == gcvSTATUS_OK)
    {
        chipCtx->errorStatus = errorStatus;
    }

    gcmFOOTER();
    return status;
}

GLenum
__glChipGetError(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLenum glError;
    gcmHEADER_ARG("gc=0x%x", gc);

    switch (chipCtx->errorStatus)
    {
    case gcvSTATUS_OUT_OF_MEMORY:
        glError = GL_OUT_OF_MEMORY;
        break;

    default:
    case gcvSTATUS_OK:
        glError = GL_NO_ERROR;
        break;

    }
    /* Reset error status */
    chipCtx->errorStatus = gcvSTATUS_OK;

    gcmFOOTER_ARG("return=%d", glError);
    return glError;
}


