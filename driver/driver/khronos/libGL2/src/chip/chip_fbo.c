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


#include "gc_gl_context.h"
#include "chip_context.h"
#include "../glcore/gc_gl_names_inline.c"

#define _GC_OBJ_ZONE    gcvZONE_API_GL
extern GLvoid setReadBuffers(glsCHIPCONTEXT_PTR chipCtx,
                      GLboolean          invertY,
                      GLboolean          integerRT,
                      gcoSURF            rtSurf,
                      gcoSURF            dSurf,
                      gcoSURF            sSurf);
extern GLvoid setDrawBuffers(glsCHIPCONTEXT_PTR chipCtx,
                      GLboolean          invertY,
                      GLboolean          integerRT,
                      GLboolean          floatRT,
                      gcoSURF*           rtSurfs,
                      gcoSURF            dSurf,
                      gcoSURF            sSurf);
extern gceSURF_FORMAT getHWFormat(GLuint devFmt);
extern GLboolean __glIsIntegerInternalFormat(GLenum internalFormat);
extern GLboolean __glIsFloatInternalFormat(GLenum internalFormat);
extern GLvoid residentTextureLevel(__GLclientPixelState *ps, glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj, GLint face, GLint level, const GLvoid* buf);

gceSTATUS
glSURF_Blit(
    IN OPTIONAL gcoSURF SrcSurface,
    IN gcoSURF DstSurface,
    IN gctUINT32 RectCount,
    IN OPTIONAL gcsRECT_PTR SrcRect,
    IN gcsRECT_PTR DstRect,
    IN OPTIONAL gcoBRUSH Brush,
    IN gctUINT8 FgRop,
    IN gctUINT8 BgRop,
    IN OPTIONAL gceSURF_TRANSPARENCY Transparency,
    IN OPTIONAL gctUINT32 TransparencyColor,
    IN OPTIONAL gctPOINTER Mask,
    IN OPTIONAL gceSURF_MONOPACK MaskPack
    )
{

#if gcdENABLE_2D
    return gcoSURF_Blit(SrcSurface, DstSurface, RectCount, SrcRect, DstRect, Brush, FgRop, BgRop, Transparency, TransparencyColor, Mask, MaskPack);
#endif

#if gcdENABLE_3D
    {
        gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
        gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        rtView.surf = SrcSurface;
        tgtView.surf = DstSurface;

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.srcOrigin.x = SrcRect->left;
        rlvArgs.uArgs.v2.srcOrigin.y = SrcRect->top;
        rlvArgs.uArgs.v2.dstOrigin.x = DstRect->left;
        rlvArgs.uArgs.v2.dstOrigin.y = DstRect->top;
        rlvArgs.uArgs.v2.rectSize.x = SrcRect->right - SrcRect->left;
        rlvArgs.uArgs.v2.rectSize.y = SrcRect->bottom- SrcRect->top;
        rlvArgs.uArgs.v2.numSlices  = 1;
        return gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
    }
#endif

/* make the compiler happy */
    return gcvSTATUS_OK;

}

__GL_INLINE GLboolean IsAttachFormatRenderable(GLint attachpointIndex, GLenum internalformat)
{
    GL_ASSERT((attachpointIndex>=0) && (attachpointIndex < __GL_MAX_ATTACHMENTS));

    /* Attachment point of COLOR_ATTACHMENT0_EXT through  COLOR_ATTACHMENTn_EXT */
    if(attachpointIndex < __GL_MAX_COLOR_ATTACHMENTS){
        switch(internalformat){
            /* color-renderable format */
                case GL_RGB:
                case GL_R3_G3_B2:
                case GL_RGB4:
                case GL_RGB5:
                case GL_RGB8:
                case GL_RGB10:
                case GL_RGB12:
                case GL_RGB16:

                case GL_RGBA:
                case GL_RGBA2:
                case GL_RGBA4:
                case GL_RGB5_A1:
                case GL_RGBA8:
                case GL_RGB10_A2:
                case GL_RGBA12:
                case GL_RGBA16:

                case GL_FLOAT_R_NV:
                case GL_FLOAT_RG_NV:
                case GL_FLOAT_RGB_NV:
                case GL_FLOAT_RGBA_NV:

                    /*extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGB32I_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGB16I_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB8I_EXT:

                    /* extension GL_EXT_packed_float*/
                case GL_R11F_G11F_B10F_EXT:

                    /* Extension GL_ARB_texture_float */
                case GL_RGBA32F_ARB:
                case GL_RGB32F_ARB:
                case GL_ALPHA32F_ARB:
                case GL_INTENSITY32F_ARB:
                case GL_LUMINANCE32F_ARB:
                case GL_LUMINANCE_ALPHA32F_ARB:
                case GL_RGBA16F_ARB:
                case GL_RGB16F_ARB:
                case GL_ALPHA16F_ARB:
                case GL_INTENSITY16F_ARB:
                case GL_LUMINANCE16F_ARB:
                case GL_LUMINANCE_ALPHA16F_ARB:

                    return GL_TRUE;

                default:
                    return GL_FALSE;
        }
    }

    if(attachpointIndex == __GL_MAX_COLOR_ATTACHMENTS){
        switch(internalformat){
            /* depth-renderable */
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                return GL_TRUE;
            default:
                return GL_FALSE;
        }
    }

    switch(internalformat){
        /* stencil-renderable */
        case GL_STENCIL_INDEX:
        case GL_STENCIL_INDEX1_EXT:
        case GL_STENCIL_INDEX4_EXT:
        case GL_STENCIL_INDEX8_EXT:
        case GL_STENCIL_INDEX16_EXT:
            return GL_TRUE;
        default:
            return GL_FALSE;
    }

}

GLboolean IsIntegerColorBuffer(GLenum pixelFormat)
{
    switch(pixelFormat)
    {
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
        return GL_TRUE;
        break;
    default:
        break;
    }
    return GL_FALSE;
}

GLboolean IsFloatColorBuffer(GLenum pixelFormat)
{
    switch(pixelFormat)
    {
    case GL_RGBA32F_ARB:
    case GL_RGB32F_ARB:
    case GL_RGBA16F_ARB:
    case GL_RGB16F_ARB:
    case GL_R11F_G11F_B10F_EXT:
    case GL_RGB9_E5_EXT:
        return GL_TRUE;
    default:
        break;
    }

    return GL_FALSE;
}

GLvoid pickReadBufferForFBO(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject   *fbo = gc->frameBuffer.readFramebufObj;
    __GLfboAttachPoint      *attachState = NULL;
    GLuint                  attachIdx = 0;
    gcoSURF rtSurf = NULL;
    gcoSURF depthSurf = NULL;
    gcoSURF stencilSurf = NULL;
    GLboolean                invertY = GL_FALSE;

    /*
    **  Set correct render target views for reading
    */
    if (fbo->readBuffer != GL_NONE)
    {
        attachIdx = fbo->readBuffer - GL_COLOR_ATTACHMENT0_EXT;
        attachState = &fbo->attachPoint[attachIdx];

        switch(attachState->objectType)
        {
            case GL_NONE:
            break;

            case GL_RENDERBUFFER_EXT:
            {
                __GLrenderbufferObject      *rbo = NULL;
                glsRenderBufferObject   *chipRBO = NULL;

                rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

                GL_ASSERT(rbo);
                chipRBO = (glsRenderBufferObject*)(rbo->privateData);

                GL_ASSERT(chipRBO);
                rtSurf = chipRBO->rtBufferInfo->renderTarget;
            }
            break;

        case GL_TEXTURE:
            {
                __GLtextureObject   *tex = NULL;
                glsTEXTUREINFO      *texInfo = NULL;
                gcoSURF surf;

                tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachState->objName);

                /*The texture is not created successfully*/
                if (!tex->faceMipmap[attachState->face][attachState->level].deviceFormat)
                {
                    break;
                }

                GL_ASSERT(tex);
                texInfo = (glsTEXTUREINFO*)(tex->privateData);

                gcoTEXTURE_GetMipMap(texInfo->object,
                    attachState->level,
                    &surf);
                rtSurf = surf;
                if(!attachState->layered)
                {
                    /* Consider depth texture later if HW supports depth texture */
                }
           }
           break;

        default:
            GL_ASSERT(0);
            break;
        }
    }

    /*
    **  Set correct depth view for reading
    */
    attachState = &fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS];
    switch(attachState->objectType)
    {
    case GL_NONE:
        break;

    case GL_RENDERBUFFER_EXT:
        {
            __GLrenderbufferObject      *rbo = NULL;
            glsRenderBufferObject   *chipRBO = NULL;

            rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

            GL_ASSERT(rbo);
            chipRBO = (glsRenderBufferObject*)(rbo->privateData);

            GL_ASSERT(chipRBO);
            depthSurf = chipRBO->dBufferInfo->depthBuffer;
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject       *tex = NULL;
            glsTEXTUREINFO      *texInfo = NULL;
            gcoSURF surf;

            tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachState->objName);

            /*The texture is not created successfully*/
            if (!tex->faceMipmap[attachState->face][attachState->level].deviceFormat)
            {
                break;
            }

            GL_ASSERT(tex);
            texInfo = (glsTEXTUREINFO*)(tex->privateData);

            GL_ASSERT(texInfo);
                gcoTEXTURE_GetMipMap(texInfo->object,
                    attachState->level,
                    &surf);
            depthSurf = surf;
            if(!attachState->layered)
            {
                /* Consider depth texture later if HW supports depth texture */
            }
        }
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    /*
    **  Set correct stencil view for reading
    */
    attachState = &fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS + 1];
    switch(attachState->objectType)
    {
    case GL_NONE:
        {
            stencilSurf = NULL;
        }
        break;

    case GL_RENDERBUFFER_EXT:
        {
            __GLrenderbufferObject      *rbo = NULL;
            glsRenderBufferObject   *chipRBO = NULL;

            rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

            GL_ASSERT(rbo);
            chipRBO = (glsRenderBufferObject*)(rbo->privateData);

            GL_ASSERT(chipRBO);
            stencilSurf = chipRBO->sBufferInfo->stencilBuffer;
        }
        break;

    case GL_TEXTURE:
        /*SPEC not support stencil texture now */
    default:
        GL_ASSERT(0);
        break;
    }

    /*
    **  Set read buffer states
    */
    setReadBuffers(chipCtx, invertY,fbo->fbInteger, rtSurf, depthSurf,stencilSurf);
}

GLvoid pickDrawBufferForFBO(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject   *fbo = gc->frameBuffer.drawFramebufObj;
    __GLfboAttachPoint      *attachState = NULL;
    GLuint                  attachIdx = 0;
    GLuint                  i = 0;

    gcoSURF rtSurf[__GL_MAX_DRAW_BUFFERS] = {NULL};
    gcoSURF depthSurf = NULL;
    gcoSURF stencilSurf = NULL;
    GLboolean          invertY = GL_FALSE;

    /*
    **  Set correct render target views for drawing
    */
    for(i = 0; i < __GL_MAX_COLOR_ATTACHMENTS; i++)
    {
        if(fbo->drawBuffer[i] != GL_NONE)
        {
            attachIdx = fbo->drawBuffer[i] - GL_COLOR_ATTACHMENT0_EXT;
            attachState = &fbo->attachPoint[attachIdx];

            switch(attachState->objectType)
            {
            case GL_NONE:
                {
                    rtSurf[attachIdx] = NULL;
                }
                break;

            case GL_RENDERBUFFER_EXT:
                {
                    __GLrenderbufferObject      *rbo = NULL;
                    glsRenderBufferObject   *chipRBO = NULL;

                    rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

                    GL_ASSERT(rbo);
                    chipRBO = (glsRenderBufferObject*)(rbo->privateData);

                    GL_ASSERT(chipRBO);
                    rtSurf[attachIdx] = chipRBO->rtBufferInfo->renderTarget;
                }
                break;

            case GL_TEXTURE:
                {
                    __GLtextureObject       *tex = NULL;
                    glsTEXTUREINFO      *texInfo = NULL;
                    gcoSURF surf;

                    tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachState->objName);

                    /*The texture is not created successfully*/
                    if (!tex->faceMipmap[attachState->face][attachState->level].deviceFormat)
                    {
                        break;
                    }

                    GL_ASSERT(tex);
                    texInfo = (glsTEXTUREINFO*)(tex->privateData);

                    GL_ASSERT(texInfo);
                    gcoTEXTURE_GetMipMap(texInfo->object,
                        attachState->level,
                        &surf);

                    if (chipCtx->renderToTexture)
                    {
                        rtSurf[attachIdx] = surf;
                    }
                    else
                    {
                        rtSurf[attachIdx] = texInfo->texRenderTarget;
                        texInfo->renderDirty = gcvTRUE;
                    }

                    if(!attachState->layered)
                    {
                        /* Consider depth texture later if HW supports depth texture */
                    }
                }
                break;

            default:
                GL_ASSERT(0);
                break;
            }
        }
    }

    /*
    **  Set correct depth view for drawing
    */
    attachState = &fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS];
    switch(attachState->objectType)
    {
    case GL_NONE:
        break;

    case GL_RENDERBUFFER_EXT:
        {
            __GLrenderbufferObject      *rbo = NULL;
            glsRenderBufferObject   *chipRBO = NULL;

            rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

            GL_ASSERT(rbo);
            chipRBO = (glsRenderBufferObject*)(rbo->privateData);

            GL_ASSERT(chipRBO);
            depthSurf = chipRBO->dBufferInfo->depthBuffer;
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject   *tex = NULL;
            glsTEXTUREINFO      *texInfo = NULL;
            gcoSURF surf;

            tex = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachState->objName);

            /*The texture is not created successfully*/
            if (!tex->faceMipmap[attachState->face][attachState->level].deviceFormat)
            {
                break;
            }

            GL_ASSERT(tex);
            texInfo = (glsTEXTUREINFO*)(tex->privateData);

            GL_ASSERT(texInfo);
            gcoTEXTURE_GetMipMap(texInfo->object,
            attachState->level,
            &surf);
            depthSurf = surf;

            if (chipCtx->renderToTexture)
            {
                        rtSurf[attachIdx] = surf;
            }
            else
            {
                        rtSurf[attachIdx] = texInfo->texRenderTarget;
                        texInfo->renderDirty = gcvTRUE;
                        depthSurf = texInfo->texRenderTarget;
            }

            if(!attachState->layered)
            {
                /* Consider depth texture later if HW supports depth texture */
            }
        }
        break;

    default:
        GL_ASSERT(0);
        break;
    }


    /*
    **  Set correct stencil view for drawing
    */
    attachState = &fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS + 1];
    switch(attachState->objectType)
    {
    case GL_NONE:
        break;

    case GL_RENDERBUFFER_EXT:
        {
            __GLrenderbufferObject      *rbo = NULL;
            glsRenderBufferObject   *chipRBO = NULL;

            rbo = (__GLrenderbufferObject*)__glGetObject(gc, gc->frameBuffer.rboShared, attachState->objName);

            GL_ASSERT(rbo);
            chipRBO = (glsRenderBufferObject*)(rbo->privateData);

            GL_ASSERT(chipRBO);
            stencilSurf = chipRBO->sBufferInfo->stencilBuffer;
        }
        break;

    case GL_TEXTURE:
        /*SPEC not support stencil texture now */
    default:
        GL_ASSERT(0);
        break;
    }

    setDrawBuffers(chipCtx, invertY,fbo->fbInteger, fbo->fbFloat, rtSurf, depthSurf, stencilSurf);
}

GLvoid UpdateReadBuffer(__GLcontext *gc)
{
    __glChipReadBuffer(gc);
}

GLvoid UpdateDrawBuffers(__GLcontext *gc)
{
    __glChipDrawBuffers(gc);
}

GLboolean MakeFBOResident(__GLcontext *gc, glsCHIPCONTEXT_PTR chipCtx, __GLframebufferObject *fbo)
{
    GLuint attachIdx = 0;
    __GLfboAttachPoint *attachState = NULL;

    for(attachIdx = 0; attachIdx < __GL_MAX_ATTACHMENTS; attachIdx++)
    {
        attachState = &fbo->attachPoint[attachIdx];
        switch(attachState->objectType)
        {
        case GL_NONE:
            break;

        case GL_TEXTURE:
            {
                __GLtextureObject       *tex = NULL;
                GLuint                  face  = attachState->face;
                GLuint                  level = attachState->level;

                tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachState->objName);
                GL_ASSERT(tex);

                /*The texture is not created successfully*/
                if (!tex->faceMipmap[face][level].deviceFormat)
                {
                    break;
                }

                residentTextureLevel(&gc->clientState.pixel, chipCtx, tex, face, level, NULL);
            }
            break;

        case GL_RENDERBUFFER_EXT:
            {
                /* All things have been done when set storage, need do nothing here. */
            }
            break;

        default :
            GL_ASSERT(0);
            break;
        }
    }

    return GL_TRUE;
}

GLvoid setRenderTarget(glsCHIPCONTEXT_PTR chipCtx, gcoSURF  rtSurf, GLuint rtIndex)
{
    gcoSURF  oldRtSurf;

    oldRtSurf = chipCtx->drawRT[rtIndex];
    if (oldRtSurf != rtSurf) {
        chipCtx->drawRT[rtIndex] = rtSurf;
    }
}

GLvoid FramebufferResetAttachpoint(glsCHIPCONTEXT_PTR chipCtx,
    __GLframebufferObject *fbo,
    GLint attachIndex)
{
    if (attachIndex < __GL_MAX_COLOR_ATTACHMENTS)
    {
        GLuint rtIndex;
        /* Find the render target index */
        for (rtIndex = 0; rtIndex < (GLuint)fbo->drawBufferCount; rtIndex++)
        {
            if ((fbo->drawBuffer[rtIndex] - GL_COLOR_ATTACHMENT0_EXT)== attachIndex)
            {
                break;
            }
        }

        if (rtIndex < (GLuint)fbo->drawBufferCount)
        {
            setRenderTarget(chipCtx, NULL, rtIndex);
        }
    }
    else if (attachIndex == __GL_MAX_COLOR_ATTACHMENTS)
    {
        chipCtx->drawDepth = NULL;
    }
    else
    {
        chipCtx->drawStencil = NULL;
    }
}

GLboolean pickRenderBufferInfo(glsCHIPCONTEXT_PTR chipCtx, __GLrenderbufferObject * rbo)
{
    GLboolean ret = GL_TRUE;
    GLuint bufferType = __GL_BACK_BUFFER;
    GLenum internalFormat = rbo->internalFormat;

    rbo->samplesUsed = rbo->samples;

    switch(internalFormat)
    {
    /* Color buffer */
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:


    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:


    /* GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:

    /*GL_EXT_packed_float*/
    case GL_R11F_G11F_B10F_EXT:

    /*GL_ARB_texture_float */
    case GL_RGBA32F_ARB:
    case GL_RGBA16F_ARB:
    case GL_RGB32F_ARB:
        bufferType = __GL_BACK_BUFFER;
        break;

    /* Depth buffer */
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT:
        bufferType = __GL_DEPTH_BUFFER;
        break;


    /* Sencil buffer
    ** Note : currently mix stencil with depth, seperate depth and stencil
              will be implemented later
    */
    case GL_STENCIL_INDEX:
    case GL_STENCIL_INDEX1_EXT:
    case GL_STENCIL_INDEX4_EXT:
    case GL_STENCIL_INDEX8_EXT:
    case GL_STENCIL_INDEX16_EXT:
        bufferType = __GL_STENCIL_BUFFER;
        break;

    case GL_FLOAT_R_NV:
    case GL_FLOAT_RG_NV:
    case GL_FLOAT_RGB_NV:
    case GL_FLOAT_RGBA_NV:
    default:
        GL_ASSERT(0);
        ret = GL_FALSE;
    }
    rbo->bufferType = bufferType;
    return ret;
}

GLboolean createRenderBufferStorage(glsCHIPCONTEXT_PTR chipCtx,
    __GLrenderbufferObject *renderbuf)
{
    glsRenderBufferObject   * chipRBO = (glsRenderBufferObject*)(renderbuf->privateData);
    gceSTATUS status = gcvSTATUS_OK;
    GLvoid  * chipRenderBuffer = NULL;
    gceSURF_TYPE surfType = gcvSURF_RENDER_TARGET;
    GLuint bufSize = 0;
    gcoSURF renderTarget;
    gceSURF_FORMAT hwFormat;

    gcmHEADER_ARG("renderbuf=0x%x", renderbuf);

    switch(renderbuf->bufferType)
    {
        case __GL_BACK_BUFFER:
            bufSize = sizeof(glsCHIPRENDERBUFFER);
            surfType = gcvSURF_RENDER_TARGET;
            break;

        case __GL_DEPTH_BUFFER:
            bufSize = sizeof(glsCHIPDEPTHBUFFER);
            surfType = gcvSURF_DEPTH;
            break;

        case __GL_STENCIL_BUFFER:
            bufSize = sizeof(glsCHIPSTENCILBUFFER);
            surfType = gcvSURF_DEPTH;
            break;

        default:
            GL_ASSERT(0);
            break;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
               bufSize,
              (gctPOINTER *) &chipRenderBuffer));

    gcoOS_ZeroMemory(chipRenderBuffer, bufSize);

    hwFormat = getHWFormat(renderbuf->deviceFormatInfo->devfmt);

    /* create render target */
    status = gcoSURF_Construct(gcvNULL, renderbuf->width, renderbuf->height,
        1,   /* number of planes */
        surfType,
        hwFormat,
        gcvPOOL_DEFAULT,
        &renderTarget
        );

    if (!gcmIS_SUCCESS(status))
    {
        goto OnError;
    }

    do {
         /* Set multi-sampling size. */
       gcmERR_BREAK(gcoSURF_SetSamples(
                 renderTarget,
                 renderbuf->samples
                 ));
    } while (GL_FALSE);

    switch(renderbuf->bufferType) {
        case __GL_BACK_BUFFER:
             gcmERR_BREAK(gcoSURF_GetAlignedSize(
                    renderTarget,
                    &((glsCHIPRENDERBUFFER *)chipRenderBuffer)->alignedWidth,
                    &((glsCHIPRENDERBUFFER *)chipRenderBuffer)->alignedHeight,
                    gcvNULL
                    ));
            ((glsCHIPRENDERBUFFER *)chipRenderBuffer)->renderTarget = renderTarget;
            ((glsCHIPRENDERBUFFER *)chipRenderBuffer)->renderTargetFormat = hwFormat;
            chipRBO->rtBufferInfo = (glsCHIPRENDERBUFFER *)chipRenderBuffer;
            break;
        case __GL_DEPTH_BUFFER:
            ((glsCHIPDEPTHBUFFER *)chipRenderBuffer)->depthBuffer = renderTarget;
            ((glsCHIPDEPTHBUFFER *)chipRenderBuffer)->depthFormat = hwFormat;
            chipRBO->dBufferInfo = (glsCHIPDEPTHBUFFER *)chipRenderBuffer;
            break;
        case __GL_STENCIL_BUFFER:
            break;
    }

    gcmFOOTER_NO();
    return GL_TRUE;

OnError:
    if (chipRenderBuffer) {
        gcoOS_Free(gcvNULL, chipRenderBuffer);
    }

    gcmFOOTER_NO();
    return GL_FALSE;
}

GLvoid deleteRenderBuffer(glsCHIPCONTEXT_PTR chipCtx, glsRenderBufferObject *chipRBO)
{
    glsCHIPRENDERBUFFER * chipRenderBuffer = (glsCHIPRENDERBUFFER*)(chipRBO->rtBufferInfo);

    if (chipRenderBuffer) {
        gcoSURF_Destroy(chipRenderBuffer->renderTarget);

        gcoOS_Free(gcvNULL, chipRenderBuffer);
        chipRBO->rtBufferInfo = NULL;
    }
}


/************************************************************************/
/* Frame buffer object implementation for DP interface                  */
/************************************************************************/

/*************************************************************************/
/*  Attach a new render object to frame buffer object */
/************************************************************************/
GLvoid __glChipFramebufferRenderbuffer(__GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLrenderbufferObject *renderbufferObj)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    /* If the framebuffer is currently bound, we should set draw in chipCtx */
    if (framebufferObj == gc->frameBuffer.drawFramebufObj)
    {
        if (renderbufferObj == NULL)
        {
            FramebufferResetAttachpoint(chipCtx, framebufferObj, attachIndex);
        }
        else
        {
            /* TODO */
        }
    }

}

GLvoid __glChipFrameBufferTexture(__GLcontext *gc,
    __GLframebufferObject *framebufferObj,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLint zoffset,
    GLboolean layered)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    /* If the framebuffer is currently bound, we should set draw draw in chipCtx */
    if (framebufferObj == gc->frameBuffer.drawFramebufObj)
    {
        if (texObj == NULL)
        {
            FramebufferResetAttachpoint(chipCtx, framebufferObj, attachIndex);
        }
        else
        {
            /* TODO */
        }
    }
}

/********************************************************************
**
**  __glChipIsFramebufferComplete
**
**  Function to check the completeness of a Framebuffer obj.
**
**  Parameters:      Describe the calling sequence
**  Return Value:    Describe the returning sequence
**
********************************************************************/
GLboolean __glChipIsFramebufferComplete(__GLcontext *gc, __GLframebufferObject *framebufferObj)
{
    GLuint i, attachIdx;
    __GLrenderbufferObject *rbo = NULL;
    __GLtextureObject *tex = NULL;
    __GLmipMapLevel *mipmap = NULL;
    __GLfboAttachPoint *attachPoint = NULL;

    GLboolean noImageAttached = GL_TRUE;
    GLuint width = 0, height = 0, samples = 0;
    GLuint fbwidth = 0, fbheight = 0, fbsamples = 0;
    GLenum  internalFormat = 0;

    GLenum error = GL_FRAMEBUFFER_COMPLETE_EXT;
    GLboolean fbInteger = GL_FALSE;
    GLboolean fbFloat = GL_FALSE;

    GLint layerCount = -1;
    GLenum layerTargetIdx = 0;
    GLenum attachType = GL_NONE;
    GLint idx = -1;

    if(framebufferObj->flag & __GL_FRAMEBUFFER_IS_CHECKED){
        if(framebufferObj->flag & __GL_FRAMEBUFFER_IS_COMPLETENESS)
            return GL_TRUE;
        else
            return GL_FALSE;
    }

    for(i = 0; i < __GL_MAX_ATTACHMENTS; i++)
    {
        /*
        ** All framebuffer attachment points are "framebuffer attachment complete".
        */

        attachPoint = &framebufferObj->attachPoint[i];

        /* Attach point with type NONE is framebuffer attachment complete. */
        if(attachPoint->objectType == GL_NONE)
            continue;

        GL_ASSERT(attachPoint->objName != 0);

        switch(attachPoint->objectType)
        {
        case GL_RENDERBUFFER_EXT:

            /* The obj must exist */
            rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, attachPoint->objName);
            if(!rbo){
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT ;
                goto return_not_complete;
            }

            /* image dimension not zero */
            if((rbo->width == 0) || (rbo->height == 0)){
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
                goto return_not_complete;
            }

            width = rbo->width;
            height = rbo->height;
            internalFormat = rbo->internalFormat;
            samples = rbo->samples;
            fbInteger =  IsIntegerColorBuffer(internalFormat);
            fbFloat =  IsFloatColorBuffer(internalFormat);
            break;

        case GL_TEXTURE:
            tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
            mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];

            /* tex obj must exist */
            if(!tex){
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT ;
                goto return_not_complete;
            }

            /* 3d tex: ZOFFSET must be  smaller than the depth of the texture */
            if((tex->targetIndex == __GL_TEXTURE_3D_INDEX)
                && attachPoint->zoffset >= tex->faceMipmap[0][tex->params.baseLevel].depth){
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT ;
                    goto return_not_complete;
            }

            if(((tex->targetIndex == __GL_TEXTURE_1D_ARRAY_INDEX) ||
                (tex->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX))
                && attachPoint->layer >= tex->faceMipmap[0][tex->params.baseLevel].arrays){
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT ;
                    goto return_not_complete;
            }

            if(attachPoint->layered)
            {
                /* all populated attachments must be layered */
                if( !layerTargetIdx )
                {
                    if(attachType != GL_NONE)
                    {
                        error = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT;
                        goto return_not_complete;
                    }
                    layerTargetIdx = tex->targetIndex;
                    layerCount = tex->faceMipmap[0][tex->params.baseLevel].depth * tex->arrays;
                }
                else
                {
                    if(layerTargetIdx != tex->targetIndex)
                    {
                        error = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT;
                        goto return_not_complete;
                    }
                    else
                    {
                        if(layerCount != tex->faceMipmap[0][tex->params.baseLevel].depth * tex->arrays)
                        {
                            error = GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT;
                            goto return_not_complete;
                        }
                    }
                }
            }

            width = mipmap->width;
            height = mipmap->height;
            internalFormat = mipmap->requestedFormat;
            samples = 0;
            fbInteger =  __glIsIntegerInternalFormat(internalFormat);
            fbFloat =  __glIsFloatInternalFormat(internalFormat);
            break;
        }


        /*
        ** Check internalformat match with attachment point type
        */
        if(!IsAttachFormatRenderable(i, internalFormat)){
            error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT ;
            goto return_not_complete;
        }

        /*
        ** check if all images have same dimension and same samples
        */
        if(noImageAttached){
            fbwidth = width;
            fbheight = height;
            fbsamples = samples;
        }
        else if((fbwidth != width) || (fbheight != height))
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT ;
            goto return_not_complete;
        }
        else if(fbsamples != samples)
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT;
            goto return_not_complete;
        }

        noImageAttached = GL_FALSE;
        attachType = attachPoint->objectType;
    }

    /*
    ** There is at least one image attached to the framebuffer.
    */
    if( noImageAttached ){
        error = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT ;
        goto return_not_complete;
    }

    /*
    ** The value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be
    ** NONE for any color attachment point(s) named by DRAW_BUFFERi
    */
    if ( (internalFormat == GL_DEPTH_COMPONENT)
        || ( internalFormat == GL_DEPTH_COMPONENT16 )
        || ( internalFormat == GL_DEPTH_COMPONENT24 )
        || ( internalFormat == GL_DEPTH_COMPONENT32 ) )
    {
            if( framebufferObj->attachPoint[__GL_MAX_COLOR_ATTACHMENTS].objectType == GL_NONE ){
                error = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT ;
                goto return_not_complete;
            }
    } else {
    for(i = 0; i < __GL_MAX_COLOR_ATTACHMENTS; i++){

        if( framebufferObj->drawBuffer[i] !=  GL_NONE ){

            attachIdx = framebufferObj->drawBuffer[i] - GL_COLOR_ATTACHMENT0_EXT;
            if( framebufferObj->attachPoint[attachIdx].objectType == GL_NONE ){
                error = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT ;
                goto return_not_complete;
            }
            idx = i;
        }

    }
    }

    /*
    ** If READ_BUFFER is not NONE, then the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE
    */
    if ( (internalFormat == GL_DEPTH_COMPONENT)
        || ( internalFormat == GL_DEPTH_COMPONENT16 )
        || ( internalFormat == GL_DEPTH_COMPONENT24 )
        || ( internalFormat == GL_DEPTH_COMPONENT32 ) )
    {
            if( framebufferObj->attachPoint[__GL_MAX_COLOR_ATTACHMENTS].objectType == GL_NONE ){
                error = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT ;
                goto return_not_complete;
            }
    } else {
    if( (framebufferObj->readBuffer != GL_NONE) && ( idx != -1) ){
        attachIdx = framebufferObj->drawBuffer[idx] - GL_COLOR_ATTACHMENT0_EXT;
        if( framebufferObj->attachPoint[attachIdx].objectType == GL_NONE ){
            error = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT ;
            goto return_not_complete;
        }
    }
    }

    /* FRAMEBUFFER_UNSUPPORTED_EXT */

    /* Sucess */
    framebufferObj->flag |= (__GL_FRAMEBUFFER_IS_COMPLETENESS | __GL_FRAMEBUFFER_IS_CHECKED);
    framebufferObj->checkCode = GL_FRAMEBUFFER_COMPLETE_EXT;
    framebufferObj->fbWidth = fbwidth;
    framebufferObj->fbHeight = fbheight;
    framebufferObj->fbSamples = fbsamples;
    framebufferObj->fbInteger = fbInteger;
    framebufferObj->fbFloat = fbFloat;

    return GL_TRUE;

return_not_complete:

    framebufferObj->flag |=  __GL_FRAMEBUFFER_IS_CHECKED;
    framebufferObj->checkCode = error;
    framebufferObj->fbWidth = framebufferObj->fbHeight = 0;
    framebufferObj->fbSamples = 0;
    framebufferObj->fbInteger = GL_FALSE;
    framebufferObj->fbFloat = GL_FALSE;
    return GL_FALSE;
}

GLvoid __glChipBlitFrameBuffer(__GLcontext *gc,
    GLint srcX0,
    GLint srcY0,
    GLint srcX1,
    GLint srcY1,
    GLint dstX0,
    GLint dstY0,
    GLint dstX1,
    GLint dstY1,
    GLbitfield mask,
    GLenum filter)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcsRECT srcrect;
    gcsRECT dstrect;
    GLint  srcWidth    = 0;
    GLint  srcHeight   = 0;
    GLint  dstWidth    = 0;
    GLint  dstHeight   = 0;
    GLint i;

    if(mask & GL_COLOR_BUFFER_BIT)
    {
        gcoSURF_GetSize(chipCtx->readRT, (gctUINT *)&srcWidth, (gctUINT *)&srcHeight, NULL);
        if(chipCtx->readInvertY)
        {
            srcrect.left     = max(0, srcX0);
            srcrect.top      = max(0, srcHeight - srcY0);
            srcrect.right    = min(srcWidth, srcX1);
            srcrect.bottom   = min(srcHeight, srcHeight - srcY1);
        }
        else
        {
            srcrect.left     = max(0, srcX0);
            srcrect.top      = max(0, srcY0);
            srcrect.right    = min(srcWidth, srcX1);
            srcrect.bottom   = min(srcHeight, srcY1);
        }

        if (filter == GL_LINEAR) {
            /* Do we need to do resolve before BLT? */
        }

        for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
        {
            if(chipCtx->drawRT[i])
            {
                gcoSURF_GetSize(chipCtx->drawRT[i], (gctUINT *)&dstWidth, (gctUINT *)&dstHeight, NULL);
                if(chipCtx->drawInvertY)
                {
                    dstrect.left     = max(0, dstX0);
                    dstrect.top      = max(0, dstHeight - dstY0);
                    dstrect.right    = min(dstWidth, dstX1);
                    dstrect.bottom   = min(dstHeight, dstHeight - dstY1);
                }
                else
                {
                    dstrect.left     = max(0, dstX0);
                    dstrect.top      = max(0, dstY0);
                    dstrect.right    = min(dstWidth, dstX1);
                    dstrect.bottom   = min(dstHeight, dstY1);
                }

                if (filter == GL_LINEAR) {
                    /* Do we need to do resolve before BLT? */
                }

                /* Blt from read color buffer to draw color buffer */
                glSURF_Blit(
                    chipCtx->readRT,
                    chipCtx->drawRT[i],
                    1,
                    &srcrect,
                    &dstrect,
                    gcvNULL,
                    0xCC,
                    0xCC,
                    gcvSURF_OPAQUE,
                    0,
                    gcvNULL,
                    0);
            }
        }
    }

    if(mask & GL_DEPTH_BUFFER_BIT)
    {
         /* Blt from read depth buffer to draw depth buffer */
        /* Consider stencil and depth sharing same surface */
        if ((chipCtx->readDepth) && (chipCtx->drawDepth))
        {
            gcoSURF_GetSize(chipCtx->readDepth, (gctUINT *)&srcWidth, (gctUINT *)&srcHeight, NULL);
            if(chipCtx->readInvertY)
            {
                srcrect.left     = max(0, srcX0);
                srcrect.top      = max(0, srcHeight - srcY0);
                srcrect.right    = min(srcWidth, srcX1);
                srcrect.bottom   = min(srcHeight, srcHeight - srcY1);
            }
            else
            {
                srcrect.left     = max(0, srcX0);
                srcrect.top      = max(0, srcY0);
                srcrect.right    = min(srcWidth, srcX1);
                srcrect.bottom   = min(srcHeight, srcY1);
            }

            gcoSURF_GetSize(chipCtx->drawDepth, (gctUINT *)&dstWidth, (gctUINT *)&dstHeight, NULL);
            if(chipCtx->drawInvertY)
            {
                dstrect.left     = max(0, dstX0);
                dstrect.top      = max(0, dstHeight - dstY0);
                dstrect.right    = min(dstWidth, dstX1);
                dstrect.bottom   = min(dstHeight, dstHeight - dstY1);
            }
            else
            {
                dstrect.left     = max(0, dstX0);
                dstrect.top      = max(0, dstY0);
                dstrect.right    = min(dstWidth, dstX1);
                dstrect.bottom   = min(dstHeight, dstY1);
            }

            if (filter == GL_LINEAR) {
               /* Do we need to do resolve before BLT? */
            }

            glSURF_Blit(
                chipCtx->readDepth,
                chipCtx->drawDepth,
                1,
                &srcrect,
                &dstrect,
                gcvNULL,
                0xCC,
                0xCC,
                gcvSURF_OPAQUE,
                0,
                gcvNULL,
                0);
        }
    }

    if(mask & GL_STENCIL_BUFFER_BIT)
    {
        /* Blt from read stencil buffer to stencil depth buffer */
        /* Consider stencil and depth sharing same surface */
        if ((chipCtx->readStencil) && (chipCtx->drawStencil))
        {
            gcoSURF_GetSize(chipCtx->readStencil, (gctUINT *)&srcWidth, (gctUINT *)&srcHeight, NULL);
            if(chipCtx->readInvertY)
            {
                srcrect.left     = max(0, srcX0);
                srcrect.top      = max(0, srcHeight - srcY0);
                srcrect.right    = min(srcWidth, srcX1);
                srcrect.bottom   = min(srcHeight, srcHeight - srcY1);
            }
            else
            {
                srcrect.left     = max(0, srcX0);
                srcrect.top      = max(0, srcY0);
                srcrect.right    = min(srcWidth, srcX1);
                srcrect.bottom   = min(srcHeight, srcY1);
            }

            gcoSURF_GetSize(chipCtx->drawStencil, (gctUINT *)&dstWidth, (gctUINT *)&dstHeight, NULL);
            if(chipCtx->drawInvertY)
            {
                dstrect.left     = max(0, dstX0);
                dstrect.top      = max(0, dstHeight - dstY0);
                dstrect.right    = min(dstWidth, dstX1);
                dstrect.bottom   = min(dstHeight, dstHeight - dstY1);
            }
            else
            {
                dstrect.left     = max(0, dstX0);
                dstrect.top      = max(0, dstY0);
                dstrect.right    = min(dstWidth, dstX1);
                dstrect.bottom   = min(dstHeight, dstY1);
            }

            glSURF_Blit(
                chipCtx->readStencil,
                chipCtx->drawStencil,
                1,
                &srcrect,
                &dstrect,
                gcvNULL,
                0xCC,
                0xCC,
                gcvSURF_OPAQUE,
                0,
                gcvNULL,
                0);
        }
    }
}

GLvoid __glChipBindDrawFrameBuffer(__GLcontext *gc,
    __GLframebufferObject *prebuf,
    __GLframebufferObject *curbuf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if(curbuf->name != 0)
    {
        /* Make resident logic buffers */
        MakeFBOResident(gc, chipCtx, curbuf);
    }

    /* Update read buffer . */
    UpdateDrawBuffers(gc);
}


GLvoid __glChipBindReadFrameBuffer(__GLcontext *gc,
    __GLframebufferObject *prebuf,
    __GLframebufferObject *curbuf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if(curbuf->name != 0)
    {
        /* Make resident logic buffers */
        MakeFBOResident(gc, chipCtx, curbuf);
    }

    /* Update read buffer . */
    UpdateReadBuffer(gc);
}


GLvoid __glChipDeleteRenderbufferObject(__GLcontext *gc,
    __GLrenderbufferObject *renderbuf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if(renderbuf->privateData)
    {
        glsRenderBufferObject *chipRBO = (glsRenderBufferObject*)renderbuf->privateData;
        if(chipRBO->rtBufferInfo)
        {
            /* Free RT surface */
            deleteRenderBuffer(chipCtx, chipRBO);
        }

        (gc->imports.free)(NULL, chipRBO);
        renderbuf->privateData = NULL;
    }
}

GLboolean __glChipRenderbufferStorage(__GLcontext *gc,
    __GLrenderbufferObject *renderbuf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsRenderBufferObject   *chipRBO = (glsRenderBufferObject*)renderbuf->privateData;

    GL_ASSERT(chipRBO);

    /* Destroy previous resource if exist */
    if(chipRBO->rtBufferInfo)
    {
        /* Free RT surface */
        deleteRenderBuffer(chipCtx, chipRBO);
    }

    /* Pick buffer info according to format */
    if(!pickRenderBufferInfo(chipCtx, renderbuf) )
    {
        return GL_FALSE;
    }

    /* Create resource */
    if(!createRenderBufferStorage(chipCtx, renderbuf))
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLvoid __glChipBindRenderBufferObject(__GLcontext *gc, __GLrenderbufferObject *renderbuf)
{
    if(renderbuf->privateData == NULL)
    {
        renderbuf->privateData = gc->imports.calloc(gc, 1, sizeof(glsRenderBufferObject));
    }
}
