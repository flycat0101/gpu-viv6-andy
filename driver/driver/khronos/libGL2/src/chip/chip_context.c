/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"
#include "icdver.h"
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_CONTEXT

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _OPENGL2_VERSION = "\n\0$VERSION$"
                               gcmTXT2STR(gcvVERSION_MAJOR) "."
                               gcmTXT2STR(gcvVERSION_MINOR) "."
                               gcmTXT2STR(gcvVERSION_PATCH) ":"
                               gcmTXT2STR(gcvVERSION_BUILD)
                               "$\n";

extern glsDEVICEPIPELINEGLOBAL dpGlobalInfo;

GLvoid __glChipNotifyChangeExclusiveMode(__GLcontext * gc);
GLboolean __glChipPresentBuffers(__GLcontext* gc,
            __GLdrawablePrivate* draw,
            GLvoid* hSurface,
            GLboolean bSwapFront,
            GLboolean DWMEnabled,
            ULONGLONG presentToken);

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/

/*******************************************************************************
**
**  constructChipName
**
**  Construct chip name string.
**
**  INPUT:
**
**      ChipID
**          Chip ID.
**
**      ChipName
**          Pointer to the chip name string.
**
**  OUTPUT:
**
**      Nothing.
*/

static void constructChipName(
    glsCHIPCONTEXT_PTR chipCtx
    )
{
    gctSTRING ChipName = (gctSTRING)chipCtx->chipName;
    gctSTRING productName = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    gcoOS_ZeroMemory(ChipName, gldCHIP_NAME_LEN);

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

    gcoOS_StrCatSafe((gctSTRING)chipCtx->chipName, gldCHIP_NAME_LEN , productName);

    gcmOS_SAFE_FREE(chipCtx->os, productName);

OnError:

    gcmFOOTER_NO();
    return;

}



/* Init DP interface to chip specific function */
GLvoid initDPInterface(__GLcontext *gc)
{
    /* Initialize dp.ctx function pointers */
    gc->dp.ctx.makeCurrent = __glChipMakeCurrent;
    gc->dp.ctx.loseCurrent = __glChipLoseCurrent;
    gc->dp.ctx.destroyPrivateData = __glChipDestroyContext;

    /* Initialize dp.ctx drawable notification functions*/
    gc->dp.ctx.notifyChangeBufferSize = __glChipNotifyChangeBufferSize;
    gc->dp.ctx.notifyDrawableSwitch = __glChipNotifyDrawableSwitch;
    gc->dp.ctx.notifyDestroyBuffers = __glChipNotifyDestroyBuffers;
    gc->dp.ctx.deviceConfigurationChanged = __glChipDeviceConfigurationChanged;

    /* Used to notify DP of GL attribute changes */
    gc->dp.attributeChanged = __glChipAttributeChange;

    gc->dp.begin = __glChipBegin;
    gc->dp.end = __glChipFlush;

    /* Flush, Finish */
    gc->dp.flush = __glChipFlush;
    gc->dp.finish = __glChipFinish;

    /* Texture related functions */
    gc->dp.bindTexture = __glChipBindTexture;
    gc->dp.deleteTexture = __glChipDeleteTexture;
    gc->dp.texImage1D = __glChipTexImage1D;
    gc->dp.texImage2D = __glChipTexImage2D;
    gc->dp.texImage3D = __glChipTexImage3D;
    gc->dp.texSubImage1D = __glChipTexSubImage1D;
    gc->dp.texSubImage2D = __glChipTexSubImage2D;
    gc->dp.texSubImage3D = __glChipTexSubImage3D;
    gc->dp.copyTexImage1D = __glChipCopyTexImage1D;
    gc->dp.copyTexImage2D = __glChipCopyTexImage2D;
    gc->dp.copyTexSubImage1D = __glChipCopyTexSubImage1D;
    gc->dp.copyTexSubImage2D = __glChipCopyTexSubImage2D;
    gc->dp.copyTexSubImage3D = __glChipCopyTexSubImage3D;
    gc->dp.generateMipmaps = __glChipGenerateMipMap;
    gc->dp.compressedTexImage2D = __glChipCompressedTexImage2D;
    gc->dp.compressedTexSubImage2D = __glChipCompressedTexSubImage2D;

    gc->dp.drawBuffer  = __glChipDrawBuffers;
    gc->dp.drawBuffers = __glChipDrawBuffers;
    gc->dp.readBuffer  = __glChipReadBuffer;
    /* Clear buffer */
    gc->dp.clear = __glChipClear;

    /* GLSL */
    gc->dp.compileShader = __glChipCompileShader;
    gc->dp.deleteShaderProgram =  __glChipDeleteShaderProgram;
    gc->dp.deleteShader = __glChipDeleteShader;
    gc->dp.linkProgram = __glChipLinkProgram;
    gc->dp.useProgram = __glChipUseProgram;
    gc->dp.validateShaderProgram = __glChipValidateProgram;
    gc->dp.getActiveUniform = __glChipGetActiveUniform;
    gc->dp.getUniformLocation = __glChipGetUniformLocation;
    gc->dp.bindAttributeLocation = __glChipBindAttributeLocation;
    gc->dp.getAttributeLocation = __glChipGetAttributeLocation;
    gc->dp.uniforms = __glChipUniforms;
    gc->dp.getUniforms = __glChipGetUniforms;
    gc->dp.buildTextureEnableDim = __glChipBuildTextureEnableDim;
    gc->dp.checkTextureConflict = __glChipCheckTextureConflict;

    /* Buffer object */
    gc->dp.bindBuffer = __glChipBindBufferObject;
    gc->dp.deleteBuffer = __glChipDeleteBufferObject;
    gc->dp.mapBuffer = __glChiptMapBufferObject;
    gc->dp.unmapBuffer = __glChipUnMapBufferObject;
    gc->dp.bufferData = __glChipBufferData;
    gc->dp.bufferSubData = __glChipBufferSubData;
    gc->dp.getBufferSubData = __glChipGetBufferSubData;

    /* Pixel pipeline */
    gc->dp.rasterBegin = __glChipRasterBegin;
    gc->dp.rasterEnd = __glChipRasterEnd;
    gc->dp.lockBuffer = __glChipLockBuffer;
    gc->dp.unlockBuffer = __glChipUnlockBuffer;
    gc->dp.ctx.drawPixels = __glChipDrawPixels;
    gc->dp.ctx.readPixels = __glChipReadPixels;
    gc->dp.ctx.copyPixels = __glChipCopyPixels;
    gc->dp.ctx.bitmaps = __glChipBitmaps;
    gc->dp.accum = __glChipAccum;

    /* GL_EXT_framebuffer_object*/
    gc->dp.bindDrawFramebuffer  = __glChipBindDrawFrameBuffer;
    gc->dp.bindReadFramebuffer  = __glChipBindReadFrameBuffer;
    gc->dp.bindRenderbuffer     = __glChipBindRenderBufferObject;
    gc->dp.deleteRenderbuffer   = __glChipDeleteRenderbufferObject;
    gc->dp.renderbufferStorage  = __glChipRenderbufferStorage;
    gc->dp.blitFramebuffer      = __glChipBlitFrameBuffer;
    gc->dp.frameBufferTexture   = __glChipFrameBufferTexture;
    gc->dp.framebufferRenderbuffer = __glChipFramebufferRenderbuffer;
    gc->dp.isFramebufferComplete = __glChipIsFramebufferComplete;

    gc->dp.updateShadingMode    = __glChipUpdateShadingMode;
}

GLvoid initExtension(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    /* Enable or disable GL extensions */
    __glExtension[INDEX_ARB_multitexture].bEnabled = GL_TRUE;

    __glExtension[INDEX_EXT_polygon_offset].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_subtexture].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_compression].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_multisample].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_env_combine].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture_env_combine].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_env_add].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture_env_add].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture_object].bEnabled = GL_TRUE;

    __glExtension[INDEX_EXT_texture_env_dot3].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_texture_env_crossbar].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_texture_env_dot3].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_texture_cube_map].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_texture_border_clamp].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_texture_mirrored_repeat].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_transpose_matrix].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_point_parameters].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_point_sprite].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_vertex_buffer_object].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_window_pos].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_non_power_of_two].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_rectangle].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_texture_float].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_half_float_pixel].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_packed_float].bEnabled = GL_FALSE;

    __glExtension[INDEX_EXT_abgr].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_bgra].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_blend_minmax].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_blend_subtract].bEnabled = GL_FALSE;

    __glExtension[INDEX_EXT_fog_coord].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_point_parameters].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_separate_specular_color].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_secondary_color].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_stencil_wrap].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_texture_lod_bias].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_vertex_array].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_stencil_two_side].bEnabled = GL_TRUE;
    __glExtension[INDEX_NV_blend_square].bEnabled = GL_FALSE;
    __glExtension[INDEX_SGIS_texture_edge_clamp].bEnabled = GL_FALSE;
    __glExtension[INDEX_WIN_swap_hint].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_draw_range_elements].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_rescale_normal].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_blend_color].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_color_table].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_convolution].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_color_matrix].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_histogram].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_blend_func_separate].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_blend_equation_separate].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture3D].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_texture_cube_map].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_vertex_program].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_fragment_program].bEnabled = GL_FALSE;
    __glExtension[INDEX_SGIS_generate_mipmap].bEnabled = GL_TRUE;
    __glExtension[INDEX_SGIS_texture_lod].bEnabled = GL_TRUE;
    __glExtension[INDEX_NV_texgen_reflection].bEnabled = GL_FALSE;

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_ANISOTROPIC_FILTERING)
        == gcvSTATUS_TRUE)
    {
        __glExtension[INDEX_EXT_texture_filter_anisotropic].bEnabled = GL_TRUE;
    }

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal,gcvFEATURE_DXT_TEXTURE_COMPRESSION)
        == gcvSTATUS_TRUE) {
        __glExtension[INDEX_EXT_texture_compression_s3tc].bEnabled = GL_TRUE;
    }

    __glExtension[INDEX_NV_texture_rectangle].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_packed_pixels].bEnabled = GL_FALSE;

    __glExtension[INDEX_ARB_occlusion_query].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_depth_texture].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_shadow].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_shadow_ambient ].bEnabled = GL_FALSE;
    __glExtension[INDEX_ATI_vertex_array_object].bEnabled = GL_TRUE;
    __glExtension[INDEX_ATI_element_array].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_depth_bounds_test].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_multi_draw_arrays].bEnabled = GL_FALSE;
    __glExtension[INDEX_SGIS_texture_border_clamp].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_texture_edge_clamp].bEnabled = GL_FALSE;
    __glExtension[INDEX_ARB_imaging].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_shadow_funcs].bEnabled = GL_FALSE;
    __glExtension[INDEX_ATI_texture_env_combine3].bEnabled = GL_TRUE;
    __glExtension[INDEX_ATI_separate_stencil].bEnabled = GL_TRUE;

    /* GLSL */
    __glExtension[INDEX_ARB_shader_objects].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_vertex_shader].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_fragment_shader].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_shading_language_100].bEnabled = GL_FALSE;

    __glExtension[INDEX_EXT_bindable_uniform].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_gpu_shader4].bEnabled = GL_FALSE;
    __glExtension[INDEX_EXT_geometry_shader4].bEnabled = GL_FALSE;

    /* MSAA */
    __glExtension[INDEX_ARB_multisample].bEnabled = GL_TRUE;

    /* FBO */
    __glExtension[INDEX_EXT_framebuffer_object].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_framebuffer_blit].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_framebuffer_multisample].bEnabled = GL_TRUE;

    /* MRT */
    __glExtension[INDEX_ATI_draw_buffers].bEnabled = GL_TRUE;
    __glExtension[INDEX_ARB_draw_buffers].bEnabled = GL_TRUE;

    __glExtension[INDEX_EXT_texture_array].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_draw_buffers2].bEnabled  = GL_TRUE;
    __glExtension[INDEX_EXT_texture_buffer_object].bEnabled  = GL_TRUE;
    __glExtension[INDEX_EXT_texture_shared_exponent].bEnabled  = GL_FALSE;
    __glExtension[INDEX_EXT_gpu_program_parameters].bEnabled  = GL_FALSE;
    __glExtension[INDEX_ARB_draw_buffers].bEnabled = GL_FALSE;

    /* Draw instanced */
    __glExtension[INDEX_EXT_draw_instanced].bEnabled = GL_TRUE;

    /* Color buffer float */
    __glExtension[INDEX_ARB_color_buffer_float].bEnabled = GL_FALSE;

    /* Timer Query */
    __glExtension[INDEX_EXT_timer_query].bEnabled = GL_TRUE;

    /* fragment program shadow */
    __glExtension[INDEX_ARB_fragment_program_shadow].bEnabled = GL_TRUE;

    /* pbo */
    __glExtension[INDEX_ARB_pixel_buffer_object].bEnabled = GL_TRUE;
    __glExtension[INDEX_EXT_pixel_buffer_object].bEnabled = GL_TRUE;
}


/**********************************************************************/
/* Implementation for device context APIs                               */
/**********************************************************************/
GLint __glChipDeviceConfigChangeEnter(GLvoid)
{
    return GL_TRUE;
}

GLint __glChipDeviceConfigChangeExit(__GLcontext *gc)
{
    return GL_TRUE;
}

GLvoid __glChipDeviceConfigurationChanged(__GLcontext * gc)
{
}

GLint __glChipUpdateDefaultVB(__GLcontext *gc)
{
    return GL_TRUE;
}

GLuint __glChipMakeCurrent(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    gcmVERIFY_OK(gco3D_Set3DEngine(chipCtx->hw));

    return GL_TRUE;
}

GLuint __glChipLoseCurrent(__GLcontext *gc, GLboolean bkickoffcmd)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    gco3D_UnSet3DEngine(chipCtx->hw);

    return GL_TRUE;
}


GLvoid __glChipGetDeviceConstants(__GLdeviceConstants *constants)
{
    gceSTATUS status;

    constants->vendor   = (GLbyte *)__GL_VENDOR;
    constants->version  = (GLbyte *)__GL_VERSION21;
    constants->GLSLVersion = (GLbyte *)__GL_GLSL_VERSION;

    constants->numberOfLights       = __GL_MAX_HW_LIGHTS;
    constants->maxTextureArraySize  = __GL_MAX_HW_ARRAY_SIZE;
    constants->maxTextureBufferSize = __GL_MAX_HW_TEXTURE_BUFFER_SIZE;
    constants->pointSizeMaximum     = __GL_MAX_POINTSPRITE_SIZE;
    constants->lineWidthMaximum     = __GL_MAX_HW_LINE_WIDTH;
#ifdef _LINUX_
    constants->maxStreamStride = 4096;
    constants->maxStreams = 16;
#else

#if defined(_MSC_VER)
    constants->maxStreamStride = 4096;
    constants->maxStreams = 16;
#else
    gcoHARDWARE_QueryStreamCaps(&constants->maxStreamStride, gcvNULL, &constants->maxStreams, gcvNULL, gcvNULL);
#endif

#endif

    constants->maxInstruction[__GL_VERTEX_PROGRAM_INDEX]        = __GL_MAX_VERTEX_PROGRAM_INSTS;
    constants->maxInstruction[__GL_FRAGMENT_PROGRAM_INDEX]      = __GL_MAX_FRAGMENT_PROGRAM_INSTS;
    constants->maxTemp[__GL_VERTEX_PROGRAM_INDEX]               = __GL_MAX_VERTEX_PROGRAM_TEMPREGS;
    constants->maxTemp[__GL_FRAGMENT_PROGRAM_INDEX]             = __GL_MAX_FRAGMENT_PROGRAM_TEMPREGS;
    constants->maxParameter[__GL_VERTEX_PROGRAM_INDEX]          = __GL_MAX_VERTEX_PROGRAM_CONSTANTS;
    constants->maxParameter[__GL_FRAGMENT_PROGRAM_INDEX]        = __GL_MAX_FRAGMENT_PROGRAM_CONSTANTS;
    constants->maxAttribute[__GL_VERTEX_PROGRAM_INDEX]          = __GL_MAX_VERTEX_PROGRAM_ATTRIBS;
    constants->maxAttribute[__GL_FRAGMENT_PROGRAM_INDEX]        = __GL_MAX_FRAGMENT_PROGRAM_ATTRIBS;
    constants->maxAddressRegister[__GL_VERTEX_PROGRAM_INDEX]    = __GL_MAX_VERTEX_PROGRAM_ADDRREGS;
    constants->maxAddressRegister[__GL_FRAGMENT_PROGRAM_INDEX]  = __GL_MAX_FRAGMENT_PROGRAM_ADDRREGS;
    constants->maxLocalParameter[__GL_VERTEX_PROGRAM_INDEX]     = __GL_MAX_VERTEX_PROGRAM_LOCALPARAMS;
    constants->maxLocalParameter[__GL_FRAGMENT_PROGRAM_INDEX]   = __GL_MAX_FRAGMENT_PROGRAM_LOCALPARAMS;
    constants->maxEnvParameter[__GL_VERTEX_PROGRAM_INDEX]       = __GL_MAX_VERTEX_PROGRAM_ENVPARAMS;
    constants->maxEnvParameter[__GL_FRAGMENT_PROGRAM_INDEX]     = __GL_MAX_FRAGMENT_PROGRAM_ENVPARAMS;

    constants->maxPSTexIndirectionInstr                         = __GL_MAX_PS_TEXREGS;
    constants->maxPSTexInstr                                    = __GL_MAX_PS_TEXREGS - 8;
    constants->maxPSALUInstr                                    = __GL_MAX_PS_INSTS;

    constants->numberofQueryCounterBits = __GL_QUERY_COUNTER_BITS;

    constants->pixelPipelineDrawSimulation  = GL_TRUE;
    constants->pixelPipelineCopySimulation  = GL_TRUE;

    constants->maxViewportWidth             = __GL_MAX_VIEWPORT_WIDTH ;
    constants->maxViewportHeight            = __GL_MAX_VIEWPORT_HEIGHT;

    constants->maxVSTextureImageUnits = __GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS;
    /* combined texture image units equals to VS + PS texture image units */
    constants->maxCombinedTextureImageUnits = constants->maxVSTextureImageUnits * 2;

    constants->maxVertexBindableUniform = __GL_MAX_VERTEX_BINDABLE_UNIFORMS;
    constants->maxFragmentBindableUniform = __GL_MAX_FRAGMENT_BINDABLE_UNIFORMS;
    constants->maxGeometryBindableUniform = __GL_MAX_GEOMETRY_BINDABLE_UNIFORMS;
    constants->maxBindableUniformSize = __GL_MAX_BINDABLE_UNIFORM_SIZE;
    constants->minProgramTexelOffset = __GL_MIN_PROGRAM_TEXEL_OFFSET;
    constants->maxProgramTexelOffset = __GL_MAX_PROGRAM_TEXEL_OFFSET;

    constants->maxGeometryTextureImageUnits = __GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS;
    constants->maxGeometryVaryingComponents = __GL_MAX_GEOMETRY_VARYING_COMPONENTS;
    constants->maxVertexVaryingComponents = __GL_MAX_VERTEX_VARYING_COMPONENTS;
    constants->maxVaryingComponents = __GL_MAX_VARYING_COMPONENTS;
    constants->maxGeometryUniformComponents = __GL_MAX_GEOMETRY_UNIFORM_COMPONENTS;
    constants->maxGeometryOutputVertices = __GL_MAX_GEOMETRY_OUTPUT_VERTICES;
    constants->maxGeometryTotalOuputComponents = __GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS;
    constants->maxVertexUniformComponents = __GL_MAX_GEOMETRY_UNIFORM_COMPONENTS;
    constants->maxFragmentUniformComponents = __GL_MAX_GEOMETRY_UNIFORM_COMPONENTS;

    do
    {
        /* Get the target maximum size. */
        gcmERR_BREAK(gcoHAL_QueryTargetCaps(
            gcvNULL, &constants->maxRenderBufferWidth,
            &constants->maxRenderBufferHeight, &constants->maxDrawBuffers, &constants->maxSamples
            ));

        /* Get the anisotropic texture max value. */
        gcmERR_BREAK(gcoHAL_QueryTextureMaxAniso(
            gcvNULL, &constants->maxTextureMaxAnisotropy));

        /* Get the texture max dimensions. */
        gcmERR_BREAK(gcoHAL_QueryTextureCaps(
            gcvNULL, &constants->maxTextureSize, gcvNULL,
            gcvNULL, gcvNULL, gcvNULL, gcvNULL, &constants->numberOfTextureUnits
            ));

        constants->numberOfTextureUnits = gcmMIN(constants->numberOfTextureUnits, 8);

        gcmERR_BREAK(gcoINDEX_QueryCaps(gcvNULL, gcvNULL, gcvNULL, &constants->maxVertexIndex));

        constants->maxVertexIndex -= 1;
    } while(GL_FALSE);

    constants->maxNumTextureLevels  = __glFloorLog2(constants->maxTextureSize) + 1;
}

GLuint __glChipQueryDeviceFormat(GLenum internalFormat,GLboolean generateMipmap,GLint samples)
{

    GLuint returnFormat;
    GLuint baseFmt = __GL_DEVFMT_RGBA8888;

    switch(internalFormat)
    {
        case 1:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE8;
            break;

        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE16;
            break;

        case GL_LUMINANCE32UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA32UI;
            break;
        case GL_LUMINANCE32I_EXT:
            returnFormat = __GL_DEVFMT_RGBA32I;
            break;
        case GL_LUMINANCE16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_LUMINANCE16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_LUMINANCE8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_LUMINANCE8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_LUMINANCE32F_ARB:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA32F;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE32F;
            break;

        case GL_LUMINANCE16F_ARB:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA16F;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE32F;
            break;
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_LUMINANCE_S3TC:
#endif
        case GL_COMPRESSED_LUMINANCE_ARB:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            if (generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            if (generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA8888_SIGNED;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1;
            break;
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_LUMINANCE_ALPHA_S3TC:
#endif
        case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            if (generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            if (generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA8888_SIGNED;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2;
            break;

        case GL_LUMINANCE4_ALPHA4:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE_ALPHA4;
            break;

        case 2:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE_ALPHA8;
            break;

        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_LUMINANCE_ALPHA16;
            break;

        case GL_LUMINANCE_ALPHA32UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA32UI;
            break;
        case GL_LUMINANCE_ALPHA32I_EXT:
            returnFormat = __GL_DEVFMT_RGBA32I;
            break;
        case GL_LUMINANCE_ALPHA16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_LUMINANCE_ALPHA16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_LUMINANCE_ALPHA8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_LUMINANCE_ALPHA8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_LUMINANCE_ALPHA32F_ARB:
            returnFormat = __GL_DEVFMT_RGBA32F;
            break;

        case GL_LUMINANCE_ALPHA16F_ARB:
            returnFormat = __GL_DEVFMT_RGBA16F;
            break;

        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
            returnFormat = __GL_DEVFMT_BGR565;
            break;

        case 3:
        case GL_RGB:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            returnFormat = baseFmt;
            break;

        case GL_RGB32F_ARB:
            if (samples > 1)
            {
                returnFormat = __GL_DEVFMT_RGBA16F;
            }
            else
            {
                returnFormat = __GL_DEVFMT_RGB32F;
            }
            break;

        case GL_BGRA8_VIVPRIV:
            returnFormat = __GL_DEVFMT_BGRA8888;
            break;

        case GL_RGB16F_ARB:
            returnFormat = __GL_DEVFMT_RGBA16F;
            break;

        case GL_RGB32UI_EXT:
            returnFormat = __GL_DEVFMT_RGB32UI;
            break;
        case GL_RGB32I_EXT:
            returnFormat = __GL_DEVFMT_RGB32I;
            break;
        case GL_RGB16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_RGB16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_RGB8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_RGB8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_R11F_G11F_B10F_EXT:
            returnFormat = __GL_DEVFMT_R11G11B10F;
            break;

        case GL_RGBA2:
        case GL_RGBA4:
            returnFormat = __GL_DEVFMT_BGRA4444;
            break;

        case 4:
        case GL_RGBA:
        case GL_RGBA8:
            returnFormat = baseFmt;
            break;

        case GL_RGBA12:
        case GL_RGBA16:
            returnFormat = __GL_DEVFMT_RGBA16;
            break;

        case GL_RGB5_A1:
            returnFormat = __GL_DEVFMT_BGRA5551;
            break;
        case GL_RGB10_A2:
            returnFormat = __GL_DEVFMT_BGRA1010102;
            break;

        case GL_RGBA32UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA32UI;
            break;
        case GL_RGBA32I_EXT:
            returnFormat = __GL_DEVFMT_RGBA32I;
            break;
        case GL_RGBA16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_RGBA16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_RGBA8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_RGBA8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_RGBA16F_ARB:
            returnFormat = __GL_DEVFMT_RGBA16F;
            break;

        case GL_RGBA32F_ARB:
            if (samples > 1)
            {
                returnFormat = __GL_DEVFMT_RGBA16F;
            }
            else
            {
                returnFormat = __GL_DEVFMT_RGBA32F;
            }
            break;

        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_COMPRESSED_ALPHA_ARB:
            returnFormat = __GL_DEVFMT_ALPHA8;
            break;

        case GL_ALPHA12:
        case GL_ALPHA16:
            returnFormat = __GL_DEVFMT_ALPHA16;
            break;

        case GL_ALPHA32UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA32UI;
            break;
        case GL_ALPHA32I_EXT:
            returnFormat = __GL_DEVFMT_RGBA32I;
            break;
        case GL_ALPHA16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_ALPHA16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_ALPHA8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_ALPHA8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_ALPHA32F_ARB:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA32F;
            else
                returnFormat = __GL_DEVFMT_ALPHA32F;
            break;

        case GL_ALPHA16F_ARB:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA16F;
            else
                returnFormat = __GL_DEVFMT_ALPHA32F;
            break;

        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA8888;
            else
                returnFormat = __GL_DEVFMT_INTENSITY8;
            break;

        case GL_INTENSITY12:
        case GL_INTENSITY16:
        case GL_COMPRESSED_INTENSITY_ARB:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA16;
            else
                returnFormat = __GL_DEVFMT_INTENSITY16;
            break;

        case GL_INTENSITY32UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA32UI;
            break;
        case GL_INTENSITY32I_EXT:
            returnFormat = __GL_DEVFMT_RGBA32I;
            break;
        case GL_INTENSITY16UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA16UI;
            break;
        case GL_INTENSITY16I_EXT:
            returnFormat = __GL_DEVFMT_RGBA16I;
            break;
        case GL_INTENSITY8UI_EXT:
            returnFormat = __GL_DEVFMT_RGBA8UI;
            break;
        case GL_INTENSITY8I_EXT:
            returnFormat = __GL_DEVFMT_RGBA8I;
            break;

        case GL_INTENSITY32F_ARB:
            if (generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA32F;
            else
                returnFormat = __GL_DEVFMT_INTENSITY32F;
            break;

        case GL_INTENSITY16F_ARB:
            if (generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA16F;
            else
                returnFormat = __GL_DEVFMT_INTENSITY32F;
            break;

        case GL_RGB_S3TC:
        case GL_RGB4_S3TC:
        case GL_COMPRESSED_RGB_ARB:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGRA5551;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RGB_DXT1;
            break;

        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGRA5551;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RGBA_DXT1;
            break;
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB4_ALPHA4_S3TC:
#endif
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGR565;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RGBA_DXT3;
            break;
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGBA_S3TC:
#endif
        case GL_COMPRESSED_RGBA_ARB:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC:
#endif
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RGBA_DXT5;
            break;

        case GL_COMPRESSED_RED_RGTC1_EXT:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RED_RGTC1;
            break;

        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            if (generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA8888_SIGNED;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1;
            break;

        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            if (generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2;
            break;

       case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_RGBA8888_SIGNED;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2;
            break;

        case GL_DEPTH_COMPONENT16:
            returnFormat = __GL_DEVFMT_Z16;
            break;

        case GL_DEPTH_COMPONENT:
            returnFormat = __GL_DEVFMT_Z24;
            break;

        case GL_DEPTH_COMPONENT24:
            returnFormat = __GL_DEVFMT_Z24;
            break;

        case GL_DEPTH_COMPONENT32:
            returnFormat = __GL_DEVFMT_Z32F;
            break;

        case GL_STENCIL_INDEX:
        case GL_STENCIL_INDEX1_EXT:
        case GL_STENCIL_INDEX4_EXT:
        case GL_STENCIL_INDEX8_EXT:
        case GL_STENCIL_INDEX16_EXT:
            returnFormat = __GL_DEVFMT_STENCIL;
            break;

        case GL_SRGB:
        case GL_SRGB8:
        case GL_SRGB_ALPHA:
        case GL_SRGB8_ALPHA8:
        case GL_SLUMINANCE_ALPHA:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_SLUMINANCE:
        case GL_SLUMINANCE8:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_SLUMINANCE:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_SRGB_ALPHA;
            break;
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGR565;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1;
            break;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGRA5551;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1;
            break;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            if(generateMipmap)
                returnFormat = __GL_DEVFMT_BGRA5551;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3;
            break;
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5;
            break;

        case GL_RGB9_E5_EXT:
            if(generateMipmap)
                returnFormat = baseFmt;
            else
                returnFormat = __GL_DEVFMT_RGB9_E5;
            break;
        case GL_DEPTH24_STENCIL8_EXT:
            returnFormat = __GL_DEVFMT_Z24_STENCIL;
            break;
        default:
#if defined(_MSC_VER)
            returnFormat = baseFmt;
#else
            GL_ASSERT(0);
            returnFormat = baseFmt;
#endif
    }
    return returnFormat;
}

extern gceSTATUS deinitializeSampler( __GLcontext* gc );
extern gceSTATUS deinitializeDraw( glsCHIPCONTEXT_PTR  chipCtx );
extern void glshReleaseCompiler(IN glsCHIPCONTEXT_PTR  chipCtx);
extern gceSTATUS deinitializeHashTable( glsCHIPCONTEXT_PTR chipCtx );
extern GLvoid freePolygonStipplePatch(__GLcontext *gc,  glsCHIPCONTEXT_PTR chipCtx);
extern GLvoid freeLineStipplePatch(__GLcontext *gc,  glsCHIPCONTEXT_PTR chipCtx);

static GLvoid freeHWEngine(glsCHIPCONTEXT_PTR chipCtx)
{

    if ( chipCtx == (glsCHIPCONTEXT_PTR)gcvNULL )
        return ;

    if (chipCtx->hw)
    {
        gcmVERIFY_OK(gco3D_UnSet3DEngine(chipCtx->hw));
        gcmVERIFY_OK(gco3D_Destroy(chipCtx->hw));
        chipCtx->hw = gcvNULL;
    }

    if (chipCtx->hal)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(chipCtx->hal));
        chipCtx->hal = gcvNULL;
    }

    if (chipCtx->os)
    {
        gcmVERIFY_OK(gcoOS_Destroy(chipCtx->os));
        chipCtx->os = gcvNULL;
    }

}

GLuint __glChipDestroyContext(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    dpGlobalInfo.numContext--;

#ifndef DISABLE_SWAP_THREAD
    if (dpGlobalInfo.numContext == 0) {
        destroyWorkThread(&dpGlobalInfo);
    }
#endif

    deinitializeSampler(gc);

    deinitializeDraw(chipCtx);

    glshReleaseCompiler(chipCtx);

    deinitializeHashTable(chipCtx);

    freePolygonStipplePatch(gc, chipCtx);
    freeLineStipplePatch(gc, chipCtx);
    freeHWEngine(chipCtx);

    (*gc->imports.free)(0, chipCtx);
    gc->dp.ctx.privateData = NULL;

    return GL_TRUE;
}

extern gceSTATUS initializeDraw( glsCHIPCONTEXT_PTR  chipCtx );
extern gceSTATUS initializeSampler( __GLcontext* gc, glsCHIPCONTEXT_PTR chipCtx );
extern GLboolean glshLoadCompiler(IN glsCHIPCONTEXT_PTR  chipCtx );
extern gceSTATUS initializeHashTable( glsCHIPCONTEXT_PTR chipCtx );
extern GLvoid initPolygonStipplePatch(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx);
extern GLvoid initLineStipplePatch(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx);
GLvoid __glChipCreateContext(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx;
    gceSTATUS status;
    GLenum result;
    /************************************************************************/
    /* Allocate dp context                                                  */
    /************************************************************************/
    chipCtx = (glsCHIPCONTEXT_PTR)(*gc->imports.calloc)(NULL, 1, sizeof(glsCHIPCONTEXT) );
    GL_ASSERT(chipCtx);

    memset((char *)chipCtx->builtinAttributeIndex, 0xFF, sizeof(chipCtx->builtinAttributeIndex));

    do
    {
        gcmONERROR(gcoOS_Construct(gcvNULL, &chipCtx->os));
        gcmONERROR(gcoHAL_Construct(gcvNULL, chipCtx->os, &chipCtx->hal));

        /* Query chip identity. */
        gcmERR_BREAK(gcoHAL_QueryChipIdentity(
            chipCtx->hal,
            &chipCtx->chipModel,
            &chipCtx->chipRevision,
            gcvNULL,
            gcvNULL
            ));

        /* Test chip ID. */
        if (chipCtx->chipModel == 0)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Construct chip name. */
        constructChipName(chipCtx);

        /* Initialize driver strings. */
        gc->constants.renderer   = (GLbyte *) chipCtx->chipName;

        /* Determine whether the fragment processor is available. */
        chipCtx->useFragmentProcessor = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_FRAGMENT_PROCESSOR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has correct stencil support in depth-only mode. */
        chipCtx->hasCorrectStencil = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_CORRECT_STENCIL
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has tile status support. */
        chipCtx->hasTileStatus = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_FAST_CLEAR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has logicOp support. */
        chipCtx->hwLogicOp = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_LOGIC_OP
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has render to texture support. */
        chipCtx->renderToTexture = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_TEXTURE_TILE_STATUS_READ
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has tx descriptor support. */
        chipCtx->hasTxDescriptor = gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_TX_DESCRIPTOR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has blit engine support. */
        chipCtx->hasBlitEngine= gcoHAL_IsFeatureAvailable(
            chipCtx->hal, gcvFEATURE_BLT_ENGINE
            ) == gcvSTATUS_TRUE;

        /* Get the 3D engine pointer. */
        gcmONERROR(gco3D_Construct(chipCtx->hal, &chipCtx->hw));

        gcmERR_BREAK(gcoHAL_QueryStreamCaps(chipCtx->hal,
                                        (gctUINT32 *)&chipCtx->maxAttributes,
                                        gcvNULL,
                                        gcvNULL,
                                        gcvNULL,
                                        gcvNULL));

        /* Set API type to OpenGL. */
        gcmERR_BREAK(gco3D_SetAPI(chipCtx->hw, gcvAPI_OPENGL));

        gcmERR_BREAK(gco3D_SetColorOutCount(chipCtx->hw, 1));

        gc->dp.ctx.privateData = chipCtx;

        initDPInterface(gc);

        /* Init extension */
        initExtension(gc);

        initializeDraw(chipCtx);

        initializeSampler(gc, chipCtx);

        glshLoadCompiler(chipCtx);

        initializeHashTable(chipCtx);

        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetAntiAliasLine(
            chipCtx->hw,
            gcvFALSE
            )));

        /* Set lastPixel Disable. */
        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetLastPixelEnable(
            chipCtx->hw, gcvFALSE
            )));

        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetDepthOnly(
            chipCtx->hw,
            gcvFALSE
            )));

        /* Turn on early-depth. */
        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetEarlyDepth(
            chipCtx->hw,
            gcvTRUE
            )));

        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetPointSizeEnable(
            chipCtx->hw,
            gcvFALSE)));

        glmERR_BREAK(glmTRANSLATEHALSTATUS(gco3D_SetPointSprite(
            chipCtx->hw,
            gcvFALSE)));

        initPolygonStipplePatch(gc, chipCtx);
        initLineStipplePatch(gc, chipCtx);

        chipCtx->samplerDirty = 0xFFFFFFFF;

        /* Free the context buffer if error. */
        gcmONERROR(status);

    }
    while (GL_FALSE);


#ifndef DISABLE_SWAP_THREAD
    if (dpGlobalInfo.numContext == 0) {
        createWorkThread(&dpGlobalInfo);
    }
#endif

    dpGlobalInfo.numContext++;

    return;

OnError:
    if (chipCtx->hw)
    {
        gcmVERIFY_OK(gco3D_Destroy(chipCtx->hw));
    }
    if (chipCtx->hal)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(chipCtx->hal));
    }

    if (chipCtx->os)
    {
        gcmVERIFY_OK(gcoOS_Destroy(chipCtx->os));
    }

    /* Free the GLContext structure. */
    if ( chipCtx) {
        (gc->imports.free)(NULL, chipCtx);
        chipCtx = gcvNULL;
    }
    return;

}

