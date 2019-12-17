/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE    gcdZONE_ES11_CONTEXT

#define ES11_MAGIC      gcmCC('e', 's', '1', '1')


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  _ConstructChipName
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

static void _ConstructChipName(
    glsCONTEXT_PTR Context
    )
{
    char* ChipName = Context->chipName;
    gctSTRING productName = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

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

    gcmONERROR(gcoHAL_GetProductName(Context->hal, &productName, gcvNULL));

    gcoOS_StrCatSafe(Context->chipName, gldCHIP_NAME_LEN , productName);

    gcmOS_SAFE_FREE(Context->os, productName);

OnError:

    gcmFOOTER_NO();
    return;
}

/* Clear Texture if the pointer of pixels is null if glTexImage2D is called */
static gctBOOL _IsNeedClearTexture()
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if (patchId == gcvPATCH_PREMIUM || patchId == gcvPATCH_RACEILLEGAL)
        return gcvTRUE;
    else
        return gcvFALSE;
}

/******************************************************************************\
********************** OpenGL ES 1.1 Context API Functions *********************
\******************************************************************************/

/*******************************************************************************
**
**  glfCreateContext
**
**  Create and initialize a context object.
**
**  INPUT:
**
**      Os
**          Pointer to an gcoOS object that needs to be destroyed.
**
**      Hal
**          Pointer to an gcoHAL object.
**
**      SharedContext
**          TBD.
**
**      SharedContextClient
**          shared context's client, such as es11, es20, es3
**
**  OUTPUT:
**
**      Pointer to the new context object.
*/

void *
glfCreateContext(
    void * Thread,
    gctINT ClientVersion,
    VEGLimports *Imports,
    void * SharedContext,
    GLint SharedContextClient
    )
{
    gceSTATUS status;
    gctUINT offset;
    gcoOS Os = NULL;
    gcoHAL Hal = NULL;
    gco3D Engine = NULL;
    glsCONTEXT_PTR context = gcvNULL;
    gctSIZE_T extensionStringLength;
    gctSTRING extensions = gcvNULL;
    gctSTRING defaultExtensions   = "GL_OES_blend_equation_separate"
                                    " "
                                    "GL_OES_blend_func_separate"
                                    " "
                                    "GL_OES_blend_subtract"
                                    " "
                                    "GL_OES_byte_coordinates"
                                    " "
                                    "GL_OES_compressed_ETC1_RGB8_texture"
                                    " "
                                    "GL_OES_compressed_paletted_texture"
                                    " "
                                    "GL_OES_draw_texture"
                                    " "
                                    "GL_OES_extended_matrix_palette"
                                    " "
                                    "GL_OES_fixed_point"
                                    " "
                                    "GL_OES_framebuffer_object"
                                    " "
                                    "GL_OES_matrix_get"
                                    " "
                                    "GL_OES_matrix_palette"
                                    " "
                                    "GL_OES_point_size_array"
                                    " "
                                    "GL_OES_point_sprite"
                                    " "
                                    "GL_OES_query_matrix"
                                    " "
                                    "GL_OES_read_format"
                                    " "
                                    "GL_OES_single_precision"
                                    " "
                                    "GL_OES_stencil_wrap"
                                    " "
                                    "GL_OES_texture_cube_map"
                                    " "
                                    /*"GL_OES_texture_env_crossbar"
                                    " "*/
                                    "GL_OES_texture_mirrored_repeat"
                                    " "
                                    "GL_OES_EGL_image"
                                    " "
                                    "GL_OES_depth24"
                                    " "
                                    "GL_OES_element_index_uint"
                                    " "
                                    "GL_OES_fbo_render_mipmap"
                                    " "
                                    "GL_OES_mapbuffer"
                                    " "
                                    "GL_OES_rgb8_rgba8"
                                    " "
                                    "GL_OES_stencil1"
                                    " "
                                    "GL_OES_stencil4"
                                    " "
                                    "GL_OES_stencil8"
                                    " "
                                    "GL_OES_texture_npot"
                                    " "
                                    /*"GL_OES_texture_3D"
                                    " "*/
                                    "GL_OES_vertex_half_float"
                                    " "
                                    "GL_OES_packed_depth_stencil"
                                    " "
                                    "GL_OES_surfaceless_context"
                                    " "
                                    "GL_EXT_texture_compression_dxt1"
                                    " "
                                    "GL_EXT_texture_format_BGRA8888"
                                    " "
                                    "GL_IMG_read_format"
                                    " "
                                    "GL_IMG_user_clip_plane"
                                    " "
                                    "GL_APPLE_texture_2D_limited_npot"
                                    " "
                                    "GL_EXT_texture_lod_bias"
                                    " "
                                    "GL_EXT_blend_minmax"
                                    " "
                                    "GL_EXT_read_format_bgra"
                                    " "
                                    "GL_EXT_multi_draw_arrays"
                                    " "
                                    "GL_OES_EGL_sync"
                                    " "
                                    "GL_APPLE_texture_format_BGRA8888"
                                    " "
                                    "GL_APPLE_texture_max_level"
                                    " "
                                    "GL_ARM_rgba8"
                                    " "
                                    "GL_OES_EGL_image_external"
                                    " "
                                    "GL_VIV_direct_texture"
                                    " "
                                    "GL_EXT_texture_compression_s3tc"
                                    " "
                                    /* MM06 depends on this. */
                                    "GL_ARB_vertex_buffer_object"
                                    " "
                                    /* MM06 depends on this. */
                                    "GL_ARB_multitexture"
                                    ;

 gctSTRING specialExtensions1   = "GL_OES_blend_equation_separate"
                                    " "
                                    /* Mega run should not support this extension.*/
                                    /*"GL_OES_blend_func_separate"
                                    " "*/
                                    "GL_OES_blend_subtract"
                                    " "
                                    "GL_OES_byte_coordinates"
                                    " "
                                    "GL_OES_compressed_ETC1_RGB8_texture"
                                    " "
                                    "GL_OES_compressed_paletted_texture"
                                    " "
                                    "GL_OES_draw_texture"
                                    " "
                                    "GL_OES_extended_matrix_palette"
                                    " "
                                    "GL_OES_fixed_point"
                                    " "
                                    "GL_OES_framebuffer_object"
                                    " "
                                    "GL_OES_matrix_get"
                                    " "
                                    "GL_OES_matrix_palette"
                                    " "
                                    "GL_OES_point_size_array"
                                    " "
                                    "GL_OES_point_sprite"
                                    " "
                                    "GL_OES_query_matrix"
                                    " "
                                    "GL_OES_read_format"
                                    " "
                                    "GL_OES_single_precision"
                                    " "
                                    "GL_OES_stencil_wrap"
                                    " "
                                    "GL_OES_texture_cube_map"
                                    " "
                                    /*"GL_OES_texture_env_crossbar"
                                    " "*/
                                    "GL_OES_texture_mirrored_repeat"
                                    " "
                                    "GL_OES_EGL_image"
                                    " "
                                    "GL_OES_depth24"
                                    " "
                                    "GL_OES_element_index_uint"
                                    " "
                                    "GL_OES_fbo_render_mipmap"
                                    " "
                                    "GL_OES_mapbuffer"
                                    " "
                                    "GL_OES_rgb8_rgba8"
                                    " "
                                    "GL_OES_stencil1"
                                    " "
                                    "GL_OES_stencil4"
                                    " "
                                    "GL_OES_stencil8"
                                    " "
                                    "GL_OES_texture_npot"
                                    " "
                                    /*"GL_OES_texture_3D"
                                    " "*/
                                    "GL_OES_vertex_half_float"
                                    " "
                                    "GL_OES_packed_depth_stencil"
                                    " "
                                    "GL_EXT_texture_compression_dxt1"
                                    " "
                                    "GL_EXT_texture_format_BGRA8888"
                                    " "
                                    "GL_IMG_read_format"
                                    " "
                                    "GL_IMG_user_clip_plane"
                                    " "
                                    "GL_APPLE_texture_2D_limited_npot"
                                    " "
                                    "GL_EXT_texture_lod_bias"
                                    " "
                                    "GL_EXT_blend_minmax"
                                    " "
                                    "GL_EXT_read_format_bgra"
                                    " "
                                    "GL_EXT_multi_draw_arrays"
                                    " "
                                    "GL_OES_EGL_sync"
                                    " "
                                    "GL_APPLE_texture_format_BGRA8888"
                                    " "
                                    "GL_APPLE_texture_max_level"
                                    " "
                                    "GL_ARM_rgba8"
                                    " "
                                    "GL_OES_EGL_image_external"
                                    " "
                                    "GL_VIV_direct_texture"
                                    " "
                                    "GL_EXT_texture_compression_s3tc"
                                    " "
                                    /* MM06 depends on this. */
                                    "GL_ARB_vertex_buffer_object"
                                    " "
                                    /* MM06 depends on this. */
                                    "GL_ARB_multitexture"
                                    ;
    gcmHEADER_ARG("Os=0x%x Hal=0x%x SharedContext=0x%x",
                  Os, Hal, SharedContext);

    do
    {
        gctPOINTER pointer = gcvNULL;
        gctBOOL    isMegaRun = gcvFALSE;
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        gcmERR_BREAK(gcoOS_Construct(gcvNULL, &Os));

        gcmERR_BREAK(gcoHAL_Construct(gcvNULL, Os, &Hal));

        gcmERR_BREAK(gco3D_Construct(Hal, gcvFALSE, &Engine));

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

        /* Allocate the context structure. */
        gcmERR_BREAK(gcoOS_Allocate(
            Os,
            gcmSIZEOF(glsCONTEXT),
            &pointer
            ));

        context = pointer;

        /* Init context structure. */
        gcoOS_ZeroMemory(context, gcmSIZEOF(glsCONTEXT));

        /* Set API pointers. */
        context->hal = Hal;

        /* Set OS object. */
        context->os  = Os;

        /* Set patch ID. */
        context->patchId = patchId;

        /* Set 3D object */
        context->hw = Engine;

        /* Save shared context pointer. */
        /*
            VIV: We don't support shared context between ES11 and ES20 above.
        */
        if (ClientVersion == SharedContextClient)
        {
            context->shared = (struct _glsCONTEXT*)SharedContext;
        }

        /* Query chip identity. */
        gcmERR_BREAK(gcoHAL_QueryChipIdentity(
            Hal,
            &context->chipModel,
            &context->chipRevision,
            gcvNULL,
            gcvNULL
            ));

        /* Test chip ID. */
        if (context->chipModel == 0)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Patch strip if the bug fix is not available. */
        context->patchStrip = (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_BUG_FIXED_INDEXED_TRIANGLE_STRIP)
                                != gcvSTATUS_TRUE);

        /* Set wLimit patch if the bug fix is not available. */
        context->wLimitPatch = (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_FRUSTUM_CLIP_FIX)
                                != gcvSTATUS_TRUE);

        context->wLimitComputeLimit         = 0x7FFFFFFF;
        context->wLimitSampleCount          = 16;
        context->zNear                      = 0.0;

        if (gcvPATCH_SF4 == patchId)
        {
            context->bComputeWlimitByVertex = gcvTRUE;
        }
        else if(gcvPATCH_SPEEDRACE == patchId)
        {
            context->bComputeWlimitByVertex = gcvTRUE;
            context->wLimitComputeLimit     = 90;
        }
        else
        {
            context->bComputeWlimitByVertex = gcvFALSE;
        }

        if(patchId == gcvPATCH_INVALID)
        {
            context->bComputeWlimitByVertex = gcvTRUE;
        }

        /* Set Sub VBO sync cap to false by default. */
        context->bSyncSubVBO = gcvFALSE;

        context->drawYInverted = gcvFALSE;

        /* Construct chip name. */
        _ConstructChipName(context);

        /* Initialize driver strings. */
        context->chipInfo.vendor     = (GLubyte*) "Vivante Corporation";
        context->chipInfo.renderer   = (GLubyte*) context->chipName;
#if defined(COMMON_LITE)
        context->chipInfo.version    = (GLubyte*) "OpenGL ES-CL 1.1";
#else
        context->chipInfo.version    = (GLubyte*) "OpenGL ES-CM 1.1";
#endif

        /* Save EGL callback functions. */
        context->imports             = *Imports;

        isMegaRun = (patchId == gcvPATCH_MEGARUN);

        if (isMegaRun)
        {
            extensions = specialExtensions1;
        }
        else
        {
            extensions = defaultExtensions;
        }

        /* Set Extension string. */
        extensionStringLength = gcoOS_StrLen(extensions, gcvNULL);

        /* Allocate more to make space for non-default extensions. */
        extensionStringLength += 100;

        gcmONERROR(
            gcoOS_Allocate(context->os,
                           extensionStringLength,
                           (gctPOINTER*)&context->chipInfo.extensions));

        offset = 0;
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(context->chipInfo.extensions,
                               extensionStringLength,
                               &offset,
                               extensions));

        if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_TEXTURE_ANISOTROPIC_FILTERING)
            == gcvSTATUS_TRUE)
        {
            gcmVERIFY_OK(
                gcoOS_StrCatSafe(context->chipInfo.extensions,
                                 extensionStringLength,
                                 " GL_EXT_texture_filter_anisotropic"));
        }

        /* Clear Texture if the pointer of pixels is null if glTexImage2D is called */
        context->clearTexturePatch = _IsNeedClearTexture();

#if gldSUPPORT_SHARED_CONTEXT
        if (context->shared != gcvNULL)
        {
            gcmERR_BREAK(glfPointNamedObjectList(
                &context->bufferList,
                context->shared->bufferList
                ));

            gcmERR_BREAK(glfPointNamedObjectList(
                &context->renderBufferList,
                context->shared->renderBufferList
                ));

            gcmERR_BREAK(glfPointNamedObjectList(
                &context->frameBufferList,
                context->shared->frameBufferList
                ));

            gcmERR_BREAK(glfPointTexture(
                &context->texture.textureList,
                context->shared->texture.textureList
                ));
        }
        else
        {
            /* Initialize buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->bufferList,
                sizeof(glsBUFFER)
                ));

            /* Initialize render buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->renderBufferList,
                sizeof(glsRENDER_BUFFER)
                ));

            /* Initialize frame buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->frameBufferList,
                sizeof(glsFRAME_BUFFER)
                ));

            if (context->texture.textureList == gcvNULL)
            {
                gcmERR_BREAK(gcoOS_Allocate(
                    gcvNULL,
                    sizeof(glsTEXTURELIST),
                    (gctPOINTER *)&context->texture.textureList
                    ));

                /* Zero buffers. */
                gcoOS_ZeroMemory(context->texture.textureList,
                    sizeof(glsTEXTURELIST));
            }

            context->texture.textureList->reference = 1;
        }
#endif

        /* Determine whether the frament processor is available. */
        context->useFragmentProcessor = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_FRAGMENT_PROCESSOR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has correct stencil support in depth-only mode. */
        context->hasCorrectStencil = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_CORRECT_STENCIL
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has tile status support. */
        context->hasTileStatus = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_FAST_CLEAR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has logicOp support. */
        context->hwLogicOp = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_LOGIC_OP
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has yuv assembler. */
        context->hasYuvAssembler = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TEXTURE_YUV_ASSEMBLER
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has linear texture. */
        context->hasLinearTx = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TEXTURE_LINEAR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has supertiled texture. */
        context->hasTxSwizzle = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TEXTURE_SWIZZLE
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has supertiled texture. */
        context->hasSupertiledTx = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_SUPERTILED_TEXTURE
            ) == gcvSTATUS_TRUE;

        /* Check whether texture unit can support tile status. */
        context->hasTxTileStatus = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TEXTURE_TILE_STATUS_READ
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has texture decompressor. */
        context->hasTxDecompressor = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TX_DECOMPRESSOR
            ) == gcvSTATUS_TRUE;

        /* Check whether IP has texture descriptor. */
        context->hasTxDescriptor = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TX_DESCRIPTOR
            ) == gcvSTATUS_TRUE;

        /* No SH alpha and PE alpha test, we need do ourself */
        context->hashAlphaTest =
            (gcoHAL_IsFeatureAvailable (Hal, gcvFEATURE_PE_NO_ALPHA_TEST) == gcvSTATUS_TRUE) &&
            (gcoHAL_IsFeatureAvailable (Hal, gcvFEATURE_SH_SUPPORT_ALPHA_KILL) != gcvSTATUS_TRUE);

        /* Check whether IP has yuv assembler. */
        context->hasYuvAssembler10bit = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_TX_YUV_ASSEMBLER_10BIT
            ) == gcvSTATUS_TRUE;

        /* Get the target maximum size. */
        gcmERR_BREAK(gcoHAL_QueryTargetCaps(
            Hal, &context->maxWidth, &context->maxHeight, gcvNULL, gcvNULL
            ));

        /* Get the anisotropic texture max value. */
        gcmERR_BREAK(gcoHAL_QueryTextureMaxAniso(
            Hal, &context->maxAniso
            ));

        /* Get the texture max dimensions. */
        gcmERR_BREAK(gcoHAL_QueryTextureCaps(
            Hal, &context->maxTextureWidth, &context->maxTextureHeight,
            gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL
            ));

        /* Set API type to OpenGL. */
        gcmERR_BREAK(gco3D_SetAPI(context->hw, gcvAPI_OPENGL_ES11));

        gcmERR_BREAK(gco3D_SetColorOutCount(context->hw, 1));
        /* Initialize the drawing system. */
        gcmERR_BREAK(glfInitializeDraw(context));

        /* Store a magic number to indicate it is a ES11 context */
        context->magic = ES11_MAGIC;

        /* Do not need destructor. */
        context->base.destructor = gcvNULL;

        context->isQuadrant = (patchId == gcvPATCH_QUADRANT);
        context->varrayDirty = gcvFALSE;
        context->curFrameBufferID = 0;
#if gcdDUMP || defined(__APPLE__) || defined(_WIN32) || defined(__QNXNTO__) || defined(EMULATOR)
        context->useInternalMem = gcvTRUE;
#else
        context->useInternalMem = gcvFALSE;
#endif

        /* Load compiler. */
        gcmERR_BREAK(glfLoadCompiler(context));
    }
    while (GL_FALSE);

    /* Free the context buffer if error. */
    if (gcmIS_ERROR(status) && (context != gcvNULL))
    {
#if gldSUPPORT_SHARED_CONTEXT
        if (context->bufferList)
        {
            glfDestroyNamedObjectList(context, context->bufferList);
            context->bufferList = gcvNULL;
        }

        if (context->renderBufferList)
        {
            glfDestroyNamedObjectList(context, context->renderBufferList);
            context->renderBufferList = gcvNULL;
        }

        if (context->frameBufferList)
        {
            glfDestroyNamedObjectList(context, context->frameBufferList);
            context->frameBufferList = gcvNULL;
        }
#endif
        /* Free the context. */
        gcmOS_SAFE_FREE(Os, context);

        /* Reset the pointer. */
        context = gcvNULL;
    }

#if VIVANTE_PROFILER
    if (context != gcvNULL)
    {
        _glffProfilerInitialize(context);
    }
#endif


    /* Return GLContext structure. */
    gcmFOOTER_ARG("context=0x%x", context);
    return context;

OnError:
    if (Engine)
    {
        gcmVERIFY_OK(gco3D_Destroy(Engine));
    }

    if (Hal)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(Hal));
    }

    if (Os)
    {
        gcmVERIFY_OK(gcoOS_Destroy(Os));
    }

    /* Free the GLContext structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(context->os, context));

    gcmFOOTER_ARG("context=0x%x", context);
    return gcvNULL;
}


/*******************************************************************************
**
**  glfDestroyContext
**
**  Destroy a context object.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Not zero if successfully destroyed.
*/

static EGLBoolean
glfDestroyContext(
    void * Thread,
    void * Context
    )
{
    gceSTATUS status = gcvSTATUS_OK, last;
    glsCONTEXT_PTR context = (glsCONTEXT_PTR) Context;
    gcmHEADER_ARG("Context=0x%x ", Context);

    /* Free the temporary bitmap. */
    gcmCHECK_STATUS(glfInitializeTempBitmap(context, gcvSURF_UNKNOWN, 0, 0));

    /* Destroy hash tables. */
    gcmCHECK_STATUS(glfFreeHashTable(context));

    /* Destroy texture management object. */
    gcmCHECK_STATUS(glfDestroyTexture(context));

    /* Free matrix stacks. */
    gcmCHECK_STATUS(glfFreeMatrixStack(context));

    if (context->bufferList != gcvNULL)
    {
        /* Destroy buffer list. */
        gcmCHECK_STATUS(glfDestroyNamedObjectList(context, context->bufferList));

        context->bufferList = gcvNULL;
    }

    if (context->renderBufferList != gcvNULL)
    {
        /* Destroy render buffer list. */
        gcmCHECK_STATUS(glfDestroyNamedObjectList(context, context->renderBufferList));

        context->renderBufferList = gcvNULL;
    }

    if (context->frameBufferList != gcvNULL)
    {
        /* Destroy frame buffer list. */
        gcmCHECK_STATUS(glfDestroyNamedObjectList(context, context->frameBufferList));

        context->frameBufferList = gcvNULL;
    }

    /* Shut down the drawing system. */
    gcmCHECK_STATUS(glfDeinitializeDraw(context));

    /* Free up the render target and depth surface. */
    gcmCHECK_STATUS(gco3D_SetTarget(context->hw, 0, gcvNULL, 0));
    gcmCHECK_STATUS(gco3D_SetDepth(context->hw, gcvNULL));

#if VIVANTE_PROFILER
    _glffProfilerDestroy(context);
#endif

    if (context->chipInfo.extensions != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(context->os, context->chipInfo.extensions));
    }

    /* Release compiler */
    if (context->dll)
    {
        gcmVERIFY_OK(glfReleaseCompiler(context));
    }

    /* Destroy 3D object */
    gcmVERIFY_OK(gco3D_Destroy(context->hw));

    /* Destroy HAL object */
    gcmVERIFY_OK(gcoHAL_Destroy(context->hal));

    /* Destroy OS object */
    gcmVERIFY_OK(gcoOS_Destroy(context->os));

    /* Destroy the context object. */
    gcmCHECK_STATUS(gcmOS_SAFE_FREE(gcvNULL, context));
    gcoOS_SetDriverTLS(gcvTLS_KEY_OPENGL_ES, (gcsDRIVER_TLS *)gcvNULL);

    /* Assert if error. */
    if (gcmIS_ERROR(status))
    {
        gcmFATAL("glfDestroyContext failed.");
        gcmFOOTER_ARG("result=%d", EGL_FALSE);
        return EGL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("result=%d", EGL_TRUE);
        return EGL_TRUE;
    }
}


/*******************************************************************************
**
**  glfSetContext
**
**  Set current context to the specified one, or update drawable/readable for
**  current context.
**
**  INPUT:
**
**      Context
**          Pointer to the context to be set as current.
**
**      Drawable,
**          Pointer to the drawable surfaces.
**
**      Readable
**          Pointer to the readable surfaces.
**
**  OUTPUT:
**
**      Nothing.
*/

EGLBoolean
glfSetContext(
    void       * Thread,
    void       * Context,
    VEGLDrawable Drawable,
    VEGLDrawable Readable
    )
{
    gceSTATUS status;
    glsCONTEXT_PTR context;
    gcoSURF draw;
    gcoSURF read;
    gcoSURF depth;
    gcsSURF_VIEW drawView = {gcvNULL, 0, 1};
    gcsSURF_VIEW dsView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("Context=0x%x Drawable=0x%x Readable=0x%x",
                  Context, Drawable, Readable);


    context = (glsCONTEXT_PTR) Context;
    draw    = Drawable ? (gcoSURF) Drawable->rtHandles[0] : gcvNULL;
    read    = Readable ? (gcoSURF) Readable->rtHandles[0] : gcvNULL;
    depth   = Drawable ? ((gcoSURF) (Drawable->depthHandle
                       ?  Drawable->depthHandle : Drawable->stencilHandle))
                       : gcvNULL;

    do
    {
        gceSURF_FORMAT drawFormat = gcvSURF_UNKNOWN;
        gctUINT drawWidth = 0, drawHeight = 0;

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

        gcmASSERT(context);

        /* Start to set context */
        gcmERR_BREAK(gco3D_Set3DEngine(context->hw));

        if (draw != gcvNULL)
        {
            /* Get surface information. */
            gcmERR_BREAK(gcoSURF_GetSize(draw, &drawWidth, &drawHeight, gcvNULL));
            gcmERR_BREAK(gcoSURF_GetFormat(draw, gcvNULL, &drawFormat));
            gcmERR_BREAK(gcoSURF_QueryFormat(drawFormat, context->drawFormatInfo));
            gcmERR_BREAK(gcoSURF_GetSamples(draw, &context->drawSamples));
        }

        if (draw != gcvNULL)
        {
            context->drawYInverted =
                (gcoSURF_QueryFlags(draw, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
        else if (depth)
        {
            context->drawYInverted =
                (gcoSURF_QueryFlags(depth, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }

        /* Store the draw size. */
        context->drawWidth  = context->effectiveWidth  = drawWidth;
        context->drawHeight = context->effectiveHeight = drawHeight;

        /* Get previous surfaces. */
        context->prevRead = Readable ? (gcoSURF) Readable->prevRtHandles[0] : gcvNULL;
        context->prevDraw = Drawable ? (gcoSURF) Drawable->prevRtHandles[0] : gcvNULL;

        /* Copy surfaces into context. */
        context->draw  = draw;
        context->read  = read;
        context->depth = depth;

        drawView.surf = draw;

        /* Set surfaces into hardware context. */
        gcmERR_BREAK(gco3D_SetTarget(context->hw, 0, &drawView, 0));

        if (depth != gcvNULL)
        {
            gcmERR_BREAK(gco3D_SetDepthMode(
                    context->hw,
                    context->depthStates.depthMode));

            gcmERR_BREAK(gco3D_EnableDepthWrite(
                    context->hw,
                    context->depthStates.testEnabled ?
                        context->depthStates.depthMask : gcvFALSE));
        }

        dsView.surf = depth;

        gcmERR_BREAK(gco3D_SetDepth(context->hw, &dsView));

        gco3D_SetColorCacheMode(context->hw);

        /* Set defaults if this is a new context. */
        if (!context->initialized)
        {
            /* fsRoundingEnabled enables a patch for conformance test where it
               fails in RGB8 mode because of absence of the proper conversion
               from 32-bit floating point PS output to 8-bit RGB frame buffer.
               At the time the problem was identified, GC500 was already taped
               out, so we fix the problem by rouding in the shader. */
            context->hashKey.hashFSRoundingEnabled =
            context->fsRoundingEnabled =
                (
                    (context->chipModel == gcv500) &&
                    (drawWidth  == 48) &&
                    (drawHeight == 48) &&
                    (!context->drawFormatInfo[0]->interleaved) &&
                    ( context->drawFormatInfo[0]->fmtClass == gcvFORMAT_CLASS_RGBA) &&
                    ( context->drawFormatInfo[0]->u.rgba.red.width   == 8) &&
                    ( context->drawFormatInfo[0]->u.rgba.green.width == 8) &&
                    ( context->drawFormatInfo[0]->u.rgba.blue.width  == 8)
                );

#if !gldSUPPORT_SHARED_CONTEXT
            /* Initialize buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->bufferList,
                sizeof(glsBUFFER)
                ));

            /* Initialize render buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->renderBufferList,
                sizeof(glsRENDER_BUFFER)
                ));

            /* Initialize frame buffer list. */
            gcmERR_BREAK(glfConstructNamedObjectList(
                context,
                &context->frameBufferList,
                sizeof(glsFRAME_BUFFER)
                ));
#endif

            /* Construct objects. */
            gcmERR_BREAK(glfInitializeHashTable(context));
            gcmERR_BREAK(glfInitializeTexture(context));
            gcmERR_BREAK(glfInitializeStreams(context));
            gcmERR_BREAK(glfInitializeMatrixStack(context));

            /* Initialize default states. */
            gcmERR_BREAK(glfSetDefaultMiscStates(context));
            gcmERR_BREAK(glfSetDefaultCullingStates(context));
            gcmERR_BREAK(glfSetDefaultAlphaStates(context));
            gcmERR_BREAK(glfSetDefaultDepthStates(context));
            gcmERR_BREAK(glfSetDefaultLightingStates(context));
            gcmERR_BREAK(glfSetDefaultPointStates(context));
            gcmERR_BREAK(glfSetDefaultFogStates(context));
            gcmERR_BREAK(glfSetDefaultLineStates(context));
            gcmERR_BREAK(glfSetDefaultPixelStates(context));
            gcmERR_BREAK(glfSetDefaultClipPlaneStates(context));
            gcmERR_BREAK(glfSetDefaultViewportStates(context));

            if (draw != gcvNULL)
            {
                context->recomputeViewport = GL_TRUE;
                context->reprogramCulling = GL_TRUE;
            }

            /* Mark as initialized. */
            context->initialized = gcvTRUE;
        }
        else
        {
            /* Reset current program so that it is loaded. */
            context->currProgram = gcvNULL;

            /* Flush internal states: texture, stream, matrix. */
            gcmERR_BREAK(glfFlushTexture(context));
            gcmERR_BREAK(glfFlushStreams(context));
            gcmERR_BREAK(glfFlushMatrixStack(context));

            /* Flush GL states. */
            gcmERR_BREAK(glfFlushMiscStates(context));
            gcmERR_BREAK(glfFlushCullingStates(context));
            gcmERR_BREAK(glfFlushAlphaStates(context));
            gcmERR_BREAK(glfFlushDepthStates(context));
            gcmERR_BREAK(glfFlushLightingStates(context));
            gcmERR_BREAK(glfFlushPointStates(context));
            gcmERR_BREAK(glfFlushFogStates(context));
            gcmERR_BREAK(glfFlushLineStates(context));
            gcmERR_BREAK(glfFlushPixelStates(context));
            gcmERR_BREAK(glfFlushClipPlaneStates(context));

            context->recomputeViewport = GL_TRUE;
            context->reprogramCulling = GL_TRUE;
        }

        /* Readjust viewport and scissor. */
        glfSetViewport(context,
                       context->viewportStates.viewportBox[0],
                       context->viewportStates.viewportBox[1],
                       context->viewportStates.viewportBox[2],
                       context->viewportStates.viewportBox[3]);

        glfSetScissor(context,
                      context->viewportStates.scissorBox[0],
                      context->viewportStates.scissorBox[1],
                      context->viewportStates.scissorBox[2],
                      context->viewportStates.scissorBox[3]);

        SetCurrentContext(context);
    }
    while (GL_FALSE);

    /* Return result. */
    gcmFOOTER();
    return gcmIS_SUCCESS(status);
}

/*******************************************************************************
**
**  glfFlushContext/glfFlush/glfFinish
**
**  Context flushing functions.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/

static EGLBoolean
glfFlushContext(
    void * Context
    )
{
    glmENTER()
    {
        /* Flush the frame buffer. */
        if (gcmIS_ERROR(gcoSURF_Flush(context->draw)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(context->hal, gcvFALSE)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }
    }
    glmLEAVE();
    return EGL_TRUE;
}

static void
glfFlush(
    void
    )
{
    glmENTER()
    {
        glfUpdateFrameBuffer(context);

        /* Flush the frame buffer. */
        if (gcmIS_ERROR(gcoSURF_Flush(context->draw)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(context->hal, gcvFALSE)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }
    }
    glmLEAVE();
}

static void
glfFinish(
    void
    )
{
    glmENTER()
    {
        /* Flush the frame buffer. */
        if (gcmIS_ERROR(gcoSURF_Flush(context->draw)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(context->hal, gcvTRUE)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }
    }
    glmLEAVE();
}

/*******************************************************************************
**
**  glfUnsetContext
**
**  Unset current context.
**
**  INPUT:
**
**      Context
**          Pointer to the context to be unset from current.
**
**  OUTPUT:
**
**      Nothing.
*/

EGLBoolean
glfUnsetContext(
    void * Thread,
    void * Context
    )
{
    gceSTATUS status;
    glsCONTEXT_PTR context = (glsCONTEXT_PTR) Context;

    gcmHEADER_ARG("Context=0x%x", Context);

    /* To maintain the previous EGL flushContext;loseCurrent; call sequence */
    glfFlushContext(Context);

    do
    {
        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        gcmASSERT(context);

        /* Release context */
        gcmERR_BREAK(gco3D_SetTarget(context->hw, 0, gcvNULL, 0));

        gcmERR_BREAK(gco3D_SetDepthMode(context->hw, gcvDEPTH_NONE));

        gcmERR_BREAK(gco3D_EnableDepthWrite(context->hw, gcvFALSE));

        gcmERR_BREAK(gco3D_SetDepth(context->hw, gcvNULL));

        gcmERR_BREAK(gco3D_UnSet3DEngine(context->hw));

        /* No surfaces in context. */
        context->draw  = gcvNULL;
        context->read  = gcvNULL;
        context->depth = gcvNULL;

        /* No previous surfaces. */
        context->prevRead = gcvNULL;
        context->prevDraw = gcvNULL;

        SetCurrentContext(gcvNULL);
    }
    while (GL_FALSE);

    /* Return result. */
    gcmFOOTER();
    return gcmIS_SUCCESS(status);
}

#if gcdENABLE_BLIT_BUFFER_PRESERVE
gceSTATUS
glfUpdateBufferPreserve(
    IN glsCONTEXT * Context
    )
{
    if ((Context->frameBuffer == gcvNULL) &&
        (Context->prevDraw != gcvNULL) &&
        !gcoSURF_QueryFlags(Context->draw, gcvSURF_FLAG_CONTENT_UPDATED) &&
        gcoSURF_QueryFlags(Context->draw, gcvSURF_FLAG_CONTENT_PRESERVED))
    {
        /* Preserve when this is first operation. */
        gcmVERIFY_OK(gcoSURF_Preserve(
            Context->prevDraw,
            Context->draw,
            gcvNULL
            ));

        /* Finish preserve. */
        gcmVERIFY_OK(gcoSURF_SetFlags(
            Context->draw,
            gcvSURF_FLAG_CONTENT_PRESERVED,
            gcvFALSE
            ));
    }

    if ((Context->frameBuffer == gcvNULL) &&
        (Context->prevRead != gcvNULL) &&
        !gcoSURF_QueryFlags(Context->read, gcvSURF_FLAG_CONTENT_UPDATED) &&
        gcoSURF_QueryFlags(Context->read, gcvSURF_FLAG_CONTENT_PRESERVED))
    {
        /* Preserve when this is first operation. */
        gcmVERIFY_OK(gcoSURF_Preserve(
            Context->prevRead,
            Context->read,
            gcvNULL
            ));

        /* Finish preserve. */
        gcmVERIFY_OK(gcoSURF_SetFlags(
            Context->read,
            gcvSURF_FLAG_CONTENT_PRESERVED,
            gcvFALSE
            ));
    }

    return gcvSTATUS_OK;
}
#endif

static gctBOOL
glfProfiler(
    void * Profiler,
    gctUINT32 Enum,
    gctHANDLE Value
    )
{
#if VIVANTE_PROFILER
    glsCONTEXT_PTR context;
    context = GetCurrentContext();
    return _glffProfilerSet(context, Enum, Value);
#else
    return gcvFALSE;
#endif
}


/*******************************************************************************
**
**  glFlush
**
**  Different GL implementations buffer commands in several different locations,
**  including network buffers and the graphics accelerator itself. glFlush
**  empties all of these buffers, causing all issued commands to be executed
**  as quickly as they are accepted by the actual rendering engine. Though this
**  execution may not be completed in any particular time period, it does
**  complete in finite time.
**
**  Because any GL program might be executed over a network, or on an
**  accelerator that buffers commands, all programs should call glFlush
**  whenever they count on having all of their previously issued commands
**  completed. For example, call glFlush before waiting for user input that
**  depends on the generated image.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glFlush(
    void
    )
{
    glmENTER()
    {
        gcmDUMP_API("${ES11 glFlush}");

        glmPROFILE(context, GLES1_FLUSH, 0);

        if ((context != gcvNULL) && context->profiler.useGlfinish)
        {
            glmPROFILE(context, GL1_PROFILER_FINISH_BEGIN, 0);
        }

        /* Flush the frame buffer. */
        if (gcmIS_ERROR(gcoSURF_Flush(context->draw)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(context->hal, gcvFALSE)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        if ((context != gcvNULL) && context->profiler.useGlfinish)
        {
            glmPROFILE(context, GL1_PROFILER_FINISH_END, 0);
        }
    }
    glmLEAVE();
}


/*******************************************************************************
**
**  glFinish
**
**  glFinish does not return until the effects of all previously called GL
**  commands are complete. Such effects include all changes to GL state,
**  all changes to connection state, and all changes to the frame buffer
**  contents.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/

GL_API void GL_APIENTRY glFinish(
    void
    )
{
    glmENTER()
    {
        gcmDUMP_API("${ES11 glFinish}");

        glmPROFILE(context, GLES1_FINISH, 0);

        if ((context != gcvNULL) && context->profiler.useGlfinish)
        {
            glmPROFILE(context, GL1_PROFILER_FINISH_BEGIN, 0);
        }

        /* Flush the cache. */
        if (gcmIS_ERROR(gcoSURF_Flush(context->draw)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        context->imports.syncNative();

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(context->hal, gcvTRUE)))
        {
            glmERROR(GL_INVALID_OPERATION);
            break;
        }

        if ((context != gcvNULL) && context->profiler.useGlfinish)
        {
            glmPROFILE(context, GL1_PROFILER_FINISH_END, 0);
        }

    }
    glmLEAVE();
}

glsCONTEXT_PTR
GetCurrentContext(void)
{
    gceSTATUS status;
    glsCONTEXT_PTR context = gcvNULL;

    gcmHEADER();

    status = gcoOS_GetDriverTLS(gcvTLS_KEY_OPENGL_ES,
                                (gcsDRIVER_TLS_PTR *) &context);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return gcvNULL;
    }

    if ((context != gcvNULL) && (context->magic != ES11_MAGIC))
    {
        gcmPRINT("%s: Invalid API call", __FUNCTION__);
        return gcvNULL;
    }

    gcmFOOTER_ARG("result=0x%x", context);

    return context;
}

void
SetCurrentContext(
    void *context
    )
{
    gcmHEADER_ARG("context=0x%x", context);

    gcoOS_SetDriverTLS(gcvTLS_KEY_OPENGL_ES, (gcsDRIVER_TLS_PTR) context);

    gcmFOOTER_NO();
}

gceSTATUS
glfLoadCompiler(
    glsCONTEXT_PTR      Context
    )
{
    gceSTATUS status;
    VSC_HW_CONFIG hwCfg;

    gcmHEADER();

    gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg);

    do
    {
        union __GLinitializerUnion
        {
            gctGLSLInitCompiler initGLSL;
            gctPOINTER          ptr;
        } intializer;

        union __GLfinalizerUnion
        {
            gctGLSLFinalizeCompiler finalizeGLSL;
            gctPOINTER ptr;
        } finalizer;

#if !defined(__VXWORKS__)
        status = gcoOS_LoadLibrary(gcvNULL,
#if defined(__APPLE__)
                                   "libGLSLC.dylib",
#elif defined(LINUXEMULATOR)
                                   "libGLSLC.so",
#elif defined(__QNXNTO__)
                                   "glesv2-sc-dlls",
#else
                                   "libGLSLC",
#endif
                                   &Context->dll);

        if (gcmIS_ERROR(status))
        {
            break;
        }
#endif

        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, Context->dll, "gcInitializeCompiler", &intializer.ptr));
        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, Context->dll, "gcFinalizeCompiler", &finalizer.ptr));

        Context->pfInitCompiler = intializer.initGLSL;
        Context->pfFinalizeCompiler = finalizer.finalizeGLSL;

        gcmERR_BREAK(Context->pfInitCompiler(Context->patchId, &hwCfg, gcvNULL));
    } while (gcvFALSE);

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
glfReleaseCompiler(
    glsCONTEXT_PTR      Context
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmONERROR(Context->pfFinalizeCompiler());

OnError:
    gcmFOOTER_NO();
    return status;
}

typedef struct _veglLOOKUP
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglLOOKUP;

#define eglMAKE_LOOKUP(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function }

static veglLOOKUP _glaLookup[] =
{
    eglMAKE_LOOKUP(glActiveTexture),
    eglMAKE_LOOKUP(glAlphaFuncx),
    eglMAKE_LOOKUP(glAlphaFuncxOES),
    eglMAKE_LOOKUP(glBindBuffer),
    eglMAKE_LOOKUP(glBindFramebufferOES),
    eglMAKE_LOOKUP(glBindRenderbufferOES),
    eglMAKE_LOOKUP(glBindTexture),
    eglMAKE_LOOKUP(glBlendFunc),
    eglMAKE_LOOKUP(glBlendFuncSeparateOES),
    eglMAKE_LOOKUP(glBlendEquationOES),
    eglMAKE_LOOKUP(glBlendEquationSeparateOES),
    eglMAKE_LOOKUP(glBufferData),
    eglMAKE_LOOKUP(glBufferSubData),
    eglMAKE_LOOKUP(glCheckFramebufferStatusOES),
    eglMAKE_LOOKUP(glClear),
    eglMAKE_LOOKUP(glClearColorx),
    eglMAKE_LOOKUP(glClearColorxOES),
    eglMAKE_LOOKUP(glClearDepthx),
    eglMAKE_LOOKUP(glClearDepthxOES),
    eglMAKE_LOOKUP(glClearStencil),
    eglMAKE_LOOKUP(glClientActiveTexture),
    eglMAKE_LOOKUP(glClipPlanex),
    eglMAKE_LOOKUP(glClipPlanexOES),
    eglMAKE_LOOKUP(glClipPlanexIMG),
    eglMAKE_LOOKUP(glColor4ub),
    eglMAKE_LOOKUP(glColor4x),
    eglMAKE_LOOKUP(glColor4xOES),
    eglMAKE_LOOKUP(glColorMask),
    eglMAKE_LOOKUP(glColorPointer),
    eglMAKE_LOOKUP(glCompressedTexImage2D),
    eglMAKE_LOOKUP(glCompressedTexSubImage2D),
    eglMAKE_LOOKUP(glCopyTexImage2D),
    eglMAKE_LOOKUP(glCopyTexSubImage2D),
    eglMAKE_LOOKUP(glCullFace),
    eglMAKE_LOOKUP(glCurrentPaletteMatrixOES),
    eglMAKE_LOOKUP(glDeleteBuffers),
    eglMAKE_LOOKUP(glDeleteFramebuffersOES),
    eglMAKE_LOOKUP(glDeleteRenderbuffersOES),
    eglMAKE_LOOKUP(glDeleteTextures),
    eglMAKE_LOOKUP(glDepthFunc),
    eglMAKE_LOOKUP(glDepthMask),
    eglMAKE_LOOKUP(glDepthRangex),
    eglMAKE_LOOKUP(glDepthRangexOES),
    eglMAKE_LOOKUP(glDisable),
    eglMAKE_LOOKUP(glDisableClientState),
    eglMAKE_LOOKUP(glDrawArrays),
    eglMAKE_LOOKUP(glDrawElements),
    eglMAKE_LOOKUP(glDrawTexiOES),
    eglMAKE_LOOKUP(glDrawTexivOES),
    eglMAKE_LOOKUP(glDrawTexsOES),
    eglMAKE_LOOKUP(glDrawTexsvOES),
    eglMAKE_LOOKUP(glDrawTexxOES),
    eglMAKE_LOOKUP(glDrawTexxvOES),
    eglMAKE_LOOKUP(glEGLImageTargetRenderbufferStorageOES),
    eglMAKE_LOOKUP(glEGLImageTargetTexture2DOES),
    eglMAKE_LOOKUP(glEnable),
    eglMAKE_LOOKUP(glEnableClientState),
    eglMAKE_LOOKUP(glFinish),
    eglMAKE_LOOKUP(glFlush),
    eglMAKE_LOOKUP(glFogx),
    eglMAKE_LOOKUP(glFogxOES),
    eglMAKE_LOOKUP(glFogxv),
    eglMAKE_LOOKUP(glFogxvOES),
    eglMAKE_LOOKUP(glFramebufferRenderbufferOES),
    eglMAKE_LOOKUP(glFramebufferTexture2DOES),
    eglMAKE_LOOKUP(glFrontFace),
    eglMAKE_LOOKUP(glFrustumx),
    eglMAKE_LOOKUP(glFrustumxOES),
    eglMAKE_LOOKUP(glGenBuffers),
    eglMAKE_LOOKUP(glGenerateMipmapOES),
    eglMAKE_LOOKUP(glGenFramebuffersOES),
    eglMAKE_LOOKUP(glGenRenderbuffersOES),
    eglMAKE_LOOKUP(glGenTextures),
    eglMAKE_LOOKUP(glGetBooleanv),
    eglMAKE_LOOKUP(glGetBufferParameteriv),
    eglMAKE_LOOKUP(glGetClipPlanex),
    eglMAKE_LOOKUP(glGetClipPlanexOES),
    eglMAKE_LOOKUP(glGetError),
    eglMAKE_LOOKUP(glGetFixedv),
    eglMAKE_LOOKUP(glGetFixedvOES),
    eglMAKE_LOOKUP(glGetFramebufferAttachmentParameterivOES),
    eglMAKE_LOOKUP(glGetIntegerv),
    eglMAKE_LOOKUP(glGetLightxv),
    eglMAKE_LOOKUP(glGetLightxvOES),
    eglMAKE_LOOKUP(glGetMaterialxv),
    eglMAKE_LOOKUP(glGetMaterialxvOES),
    eglMAKE_LOOKUP(glGetPointerv),
    eglMAKE_LOOKUP(glGetRenderbufferParameterivOES),
    eglMAKE_LOOKUP(glGetString),
    eglMAKE_LOOKUP(glGetTexEnviv),
    eglMAKE_LOOKUP(glGetTexEnvxv),
    eglMAKE_LOOKUP(glGetTexEnvxvOES),
    eglMAKE_LOOKUP(glGetTexGenivOES),
    eglMAKE_LOOKUP(glGetTexGenxvOES),
    eglMAKE_LOOKUP(glGetTexParameteriv),
    eglMAKE_LOOKUP(glGetTexParameterxv),
    eglMAKE_LOOKUP(glGetTexParameterxvOES),
    eglMAKE_LOOKUP(glHint),
    eglMAKE_LOOKUP(glIsBuffer),
    eglMAKE_LOOKUP(glIsEnabled),
    eglMAKE_LOOKUP(glIsFramebufferOES),
    eglMAKE_LOOKUP(glIsRenderbufferOES),
    eglMAKE_LOOKUP(glIsTexture),
    eglMAKE_LOOKUP(glLightModelx),
    eglMAKE_LOOKUP(glLightModelxOES),
    eglMAKE_LOOKUP(glLightModelxv),
    eglMAKE_LOOKUP(glLightModelxvOES),
    eglMAKE_LOOKUP(glLightx),
    eglMAKE_LOOKUP(glLightxOES),
    eglMAKE_LOOKUP(glLightxv),
    eglMAKE_LOOKUP(glLightxvOES),
    eglMAKE_LOOKUP(glLineWidthx),
    eglMAKE_LOOKUP(glLineWidthxOES),
    eglMAKE_LOOKUP(glLoadIdentity),
    eglMAKE_LOOKUP(glLoadMatrixx),
    eglMAKE_LOOKUP(glLoadMatrixxOES),
    eglMAKE_LOOKUP(glLoadPaletteFromModelViewMatrixOES),
    eglMAKE_LOOKUP(glLogicOp),
    eglMAKE_LOOKUP(glMaterialx),
    eglMAKE_LOOKUP(glMaterialxOES),
    eglMAKE_LOOKUP(glMaterialxv),
    eglMAKE_LOOKUP(glMaterialxvOES),
    eglMAKE_LOOKUP(glMatrixIndexPointerOES),
    eglMAKE_LOOKUP(glMatrixMode),
    eglMAKE_LOOKUP(glMultMatrixx),
    eglMAKE_LOOKUP(glMultMatrixxOES),
    eglMAKE_LOOKUP(glMultiDrawArraysEXT),
    eglMAKE_LOOKUP(glMultiDrawElementsEXT),
    eglMAKE_LOOKUP(glMultiTexCoord4x),
    eglMAKE_LOOKUP(glMultiTexCoord4xOES),
    eglMAKE_LOOKUP(glNormal3x),
    eglMAKE_LOOKUP(glNormal3xOES),
    eglMAKE_LOOKUP(glNormalPointer),
    eglMAKE_LOOKUP(glOrthox),
    eglMAKE_LOOKUP(glOrthoxOES),
    eglMAKE_LOOKUP(glPixelStorei),
    eglMAKE_LOOKUP(glPointParameterx),
    eglMAKE_LOOKUP(glPointParameterxOES),
    eglMAKE_LOOKUP(glPointParameterxv),
    eglMAKE_LOOKUP(glPointParameterxvOES),
    eglMAKE_LOOKUP(glPointSizePointerOES),
    eglMAKE_LOOKUP(glPointSizex),
    eglMAKE_LOOKUP(glPointSizexOES),
    eglMAKE_LOOKUP(glPolygonOffsetx),
    eglMAKE_LOOKUP(glPolygonOffsetxOES),
    eglMAKE_LOOKUP(glPopMatrix),
    eglMAKE_LOOKUP(glPushMatrix),
    eglMAKE_LOOKUP(glQueryMatrixxOES),
    eglMAKE_LOOKUP(glReadPixels),
    eglMAKE_LOOKUP(glRenderbufferStorageOES),
    eglMAKE_LOOKUP(glRotatex),
    eglMAKE_LOOKUP(glRotatexOES),
    eglMAKE_LOOKUP(glSampleCoveragex),
    eglMAKE_LOOKUP(glSampleCoveragexOES),
    eglMAKE_LOOKUP(glScalex),
    eglMAKE_LOOKUP(glScalexOES),
    eglMAKE_LOOKUP(glScissor),
    eglMAKE_LOOKUP(glShadeModel),
    eglMAKE_LOOKUP(glStencilFunc),
    eglMAKE_LOOKUP(glStencilMask),
    eglMAKE_LOOKUP(glStencilOp),
    eglMAKE_LOOKUP(glTexCoordPointer),
    eglMAKE_LOOKUP(glTexDirectInvalidateVIV),
    eglMAKE_LOOKUP(glTexDirectVIV),
    eglMAKE_LOOKUP(glTexDirectVIVMap),
    eglMAKE_LOOKUP(glTexDirectMapVIV),
    eglMAKE_LOOKUP(glTexDirectTiledMapVIV),
    eglMAKE_LOOKUP(glTexEnvi),
    eglMAKE_LOOKUP(glTexEnviv),
    eglMAKE_LOOKUP(glTexEnvx),
    eglMAKE_LOOKUP(glTexEnvxOES),
    eglMAKE_LOOKUP(glTexEnvxv),
    eglMAKE_LOOKUP(glTexEnvxvOES),
    eglMAKE_LOOKUP(glTexGeniOES),
    eglMAKE_LOOKUP(glTexGenivOES),
    eglMAKE_LOOKUP(glTexGenxOES),
    eglMAKE_LOOKUP(glTexGenxvOES),
    eglMAKE_LOOKUP(glTexImage2D),
    eglMAKE_LOOKUP(glTexParameteri),
    eglMAKE_LOOKUP(glTexParameteriv),
    eglMAKE_LOOKUP(glTexParameterx),
    eglMAKE_LOOKUP(glTexParameterxOES),
    eglMAKE_LOOKUP(glTexParameterxv),
    eglMAKE_LOOKUP(glTexParameterxvOES),
    eglMAKE_LOOKUP(glTexSubImage2D),
    eglMAKE_LOOKUP(glTranslatex),
    eglMAKE_LOOKUP(glTranslatexOES),
    eglMAKE_LOOKUP(glVertexPointer),
    eglMAKE_LOOKUP(glViewport),
    eglMAKE_LOOKUP(glWeightPointerOES),
    eglMAKE_LOOKUP(glBindBufferARB),
    eglMAKE_LOOKUP(glBufferDataARB),
    eglMAKE_LOOKUP(glBufferSubDataARB),
    eglMAKE_LOOKUP(glDeleteBuffersARB),
    eglMAKE_LOOKUP(glGenBuffersARB),
    eglMAKE_LOOKUP(glGetBufferParameterivARB),
    eglMAKE_LOOKUP(glAlphaFunc),
    eglMAKE_LOOKUP(glClearColor),
    eglMAKE_LOOKUP(glClearDepthf),
    eglMAKE_LOOKUP(glClearDepthfOES),
    eglMAKE_LOOKUP(glClipPlanef),
    eglMAKE_LOOKUP(glClipPlanefOES),
    eglMAKE_LOOKUP(glClipPlanefIMG),
    eglMAKE_LOOKUP(glColor4f),
    eglMAKE_LOOKUP(glDepthRangef),
    eglMAKE_LOOKUP(glDepthRangefOES),
    eglMAKE_LOOKUP(glDrawTexfOES),
    eglMAKE_LOOKUP(glDrawTexfvOES),
    eglMAKE_LOOKUP(glFogf),
    eglMAKE_LOOKUP(glFogfv),
    eglMAKE_LOOKUP(glFrustumf),
    eglMAKE_LOOKUP(glFrustumfOES),
    eglMAKE_LOOKUP(glGetClipPlanef),
    eglMAKE_LOOKUP(glGetClipPlanefOES),
    eglMAKE_LOOKUP(glGetFloatv),
    eglMAKE_LOOKUP(glGetLightfv),
    eglMAKE_LOOKUP(glGetMaterialfv),
    eglMAKE_LOOKUP(glGetTexEnvfv),
    eglMAKE_LOOKUP(glGetTexGenfvOES),
    eglMAKE_LOOKUP(glGetTexParameterfv),
    eglMAKE_LOOKUP(glLightModelf),
    eglMAKE_LOOKUP(glLightModelfv),
    eglMAKE_LOOKUP(glLightf),
    eglMAKE_LOOKUP(glLightfv),
    eglMAKE_LOOKUP(glLineWidth),
    eglMAKE_LOOKUP(glLoadMatrixf),
    eglMAKE_LOOKUP(glMaterialf),
    eglMAKE_LOOKUP(glMaterialfv),
    eglMAKE_LOOKUP(glMultMatrixf),
    eglMAKE_LOOKUP(glMultiTexCoord4f),
    eglMAKE_LOOKUP(glNormal3f),
    eglMAKE_LOOKUP(glOrthof),
    eglMAKE_LOOKUP(glOrthofOES),
    eglMAKE_LOOKUP(glPointParameterf),
    eglMAKE_LOOKUP(glPointParameterfv),
    eglMAKE_LOOKUP(glPointSize),
    eglMAKE_LOOKUP(glPolygonOffset),
    eglMAKE_LOOKUP(glRotatef),
    eglMAKE_LOOKUP(glSampleCoverage),
    eglMAKE_LOOKUP(glScalef),
    eglMAKE_LOOKUP(glTexEnvf),
    eglMAKE_LOOKUP(glTexEnvfv),
    eglMAKE_LOOKUP(glTexGenfOES),
    eglMAKE_LOOKUP(glTexGenfvOES),
    eglMAKE_LOOKUP(glTexParameterf),
    eglMAKE_LOOKUP(glTexParameterfv),
    eglMAKE_LOOKUP(glTranslatef),
    eglMAKE_LOOKUP(glMapBufferOES),
    eglMAKE_LOOKUP(glUnmapBufferOES),
    eglMAKE_LOOKUP(glGetBufferPointervOES),
    { gcvNULL, gcvNULL }
};

static EGL_PROC
glfGetProcAddr(
    const GLchar *procName
    )
{
    veglLOOKUP *lookup = _glaLookup;

    /* Loop while there are entries in the lookup tabke. */
    while (lookup->name != gcvNULL)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(lookup->name, procName)))
        {
            return lookup->function;
        }

        /* Next lookup entry. */
        ++lookup;
    }

    return gcvNULL;
}


GL_API veglDISPATCH
#ifdef COMMON_LITE
GLES_CL_DISPATCH_TABLE =
#else
GLES_CM_DISPATCH_TABLE =
#endif
{
    /* createContext            */  glfCreateContext,
    /* destroyContext           */  glfDestroyContext,
    /* makeCurrent              */  glfSetContext,
    /* loseCurrent              */  glfUnsetContext,
    /* setDrawable              */  glfSetContext,
    /* flush                    */  glfFlush,
    /* finish                   */  glfFinish,
    /* getClientBuffer          */  gcvNULL,
    /* createImageTexture       */  glfCreateImageTexture,
    /* createImageRenderbuffer  */  glfCreateImageRenderBuffer,
    /* createImageParentImage   */  gcvNULL,
    /* bindTexImage             */  glfBindTexImage,
    /* profiler                 */  glfProfiler,
    /* getProcAddr              */  glfGetProcAddr,
    /* swapBuffers              */  gcvNULL,
    /* queryHWVG                */  gcvNULL,
    /* resolveVG                */  gcvNULL,
};
