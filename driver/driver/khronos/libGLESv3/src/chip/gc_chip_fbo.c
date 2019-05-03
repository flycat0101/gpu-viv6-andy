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


#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_chip_misc.h"
#include "gc_hal_dump.h"

#if defined(ANDROID)
#if ANDROID_SDK_VERSION >= 16
#      include <ui/ANativeObjectBase.h>
#   else
#      include <private/ui/android_natives_priv.h>
#   endif

#if gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
#      include <gc_gralloc_priv.h>
#   endif
#endif

#define _GC_OBJ_ZONE    gcdZONE_ES30_FBO

extern __GLformatInfo __glFormatInfoTable[];

#if gcdFRAMEINFO_STATISTIC
extern GLboolean  g_dbgPerDrawKickOff;
extern GLbitfield g_dbgDumpImagePerDraw;
extern GLboolean  g_dbgSkipDraw;
#endif

/************************************************************************/
/* Implementation for INTERNAL FUNCTIONS                                */
/************************************************************************/

__GL_INLINE gceTEXTURE_FILTER
gcChipUtilConvertFilter(
    GLenum minFilter
    )
{
    gceTEXTURE_FILTER halFilter;
    gcmHEADER_ARG("minFilter=0x%04x", minFilter);

    switch (minFilter)
    {
    case GL_NEAREST:
        halFilter = gcvTEXTURE_POINT;
        break;
    case GL_LINEAR:
        halFilter= gcvTEXTURE_LINEAR;
        break;
    default:
        halFilter = gcvTEXTURE_NONE;
        break;
    }

    gcmFOOTER_NO();
    return halFilter;
}

/*
** As we only have sub pixel plane for msaa surface, there is no routine to get pixel plane in gcoSURF_ResolveRect,
** so we have to resolve it before read MSAA surface.
*/
__GL_INLINE gceSTATUS
gcChipUtilGetPixelPlane(
    __GLcontext *gc,
    gcsSURF_VIEW *ssbView,
    gcsSURF_VIEW *pixelPlaneView
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    /*
    **  If it's not a msaa surface, just return itself.
    */
    if (!ssbView->surf->isMsaa)
    {
        *pixelPlaneView = *ssbView;
    }
    else
    {
        GLuint width, height;
        gcsSURF_FORMAT_INFO_PTR fmtInfo;
        gceSURF_TYPE surfType;

        /* Collect current pixel plane information */
        gcmONERROR(gcoSURF_GetSize(ssbView->surf, &width, &height, gcvNULL));
        gcmONERROR(gcoSURF_GetFormatInfo(ssbView->surf, &fmtInfo));

        surfType = (fmtInfo->fmtClass == gcvFORMAT_CLASS_DEPTH)
                 ? gcvSURF_DEPTH_TS_DIRTY
                 : gcvSURF_RENDER_TARGET_TS_DIRTY;

        gcmONERROR(gcoSURF_Construct(chipCtx->hal, width, height, 1, surfType,
                                     fmtInfo->format, gcvPOOL_DEFAULT, &pixelPlaneView->surf));

        pixelPlaneView->firstSlice = 0;
        pixelPlaneView->numSlices   = 1;

        /* Downsample to pixel plane surface. */
        gcmONERROR(gcoSURF_ResolveRect(ssbView, pixelPlaneView, gcvNULL));
    }

OnError:
    return status;
}

__GL_INLINE gceSTATUS
gcChipUtilDestroyPixelPlane(
    __GLcontext *gc,
    gcsSURF_VIEW *ssbView,
    gcsSURF_VIEW *pixelPlaneView
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (ssbView->surf == pixelPlaneView->surf)
    {
        return status;
    }
    else
    {
        gcmONERROR(gcoSURF_Destroy(pixelPlaneView->surf));
    }

OnError:
    return status;
}

__GL_INLINE gceSTATUS
gcChipUtilGetShadowTexView(
    IN  __GLcontext *gc,
    IN  gcsSURF_VIEW *masterView,
    OUT gcsSURF_VIEW *shadowView
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_VIEW pixelPlaneView = {gcvNULL, 0, 1};

    /* Get pixel plane for msaa surface */
    gcmONERROR(gcChipUtilGetPixelPlane(gc, masterView, &pixelPlaneView));

    /* If HW only support tiled texture while source surface if supertiled,
    ** created a tiled shadow one instead.
    */
    if (!chipCtx->chipFeature.hwFeature.hasSupertiledTx &&
        pixelPlaneView.surf->tiling != gcvTILED)
    {
        GLuint width, height;
        gceSURF_FORMAT format;

        gcmONERROR(gcoSURF_GetSize(pixelPlaneView.surf, &width, &height, gcvNULL));
        gcmONERROR(gcoSURF_GetFormat(pixelPlaneView.surf, gcvNULL, &format));
        gcmONERROR(gcoSURF_Construct(chipCtx->hal, width, height, 1, gcvSURF_TEXTURE,
                                     format, gcvPOOL_DEFAULT, &shadowView->surf));
        shadowView->firstSlice = 0;
        shadowView->numSlices   = 1;

        gcmONERROR(gcoSURF_ResolveRect(&pixelPlaneView, shadowView, gcvNULL));
        gcmONERROR(gcChipUtilDestroyPixelPlane(gc, masterView, &pixelPlaneView));
    }
    else
    {
        *shadowView = pixelPlaneView;
    }

OnError:
    return status;
}

__GL_INLINE gceSTATUS
gcChipUtilReleaseShadowTex(
    IN __GLcontext *gc,
    IN gcsSURF_VIEW *masterView,
    IN gcsSURF_VIEW *shadowView
    )
{
    return gcChipUtilDestroyPixelPlane(gc, masterView, shadowView);
}



/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/
gcsSURF_VIEW
gcChipGetFramebufferAttachedSurfaceAndImage(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLenum attachment,
    GLvoid **image
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLfboAttachPoint *attachPoint;
    gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
    GLint attachIndex = __glMapAttachmentToIndex(attachment);

    gcmHEADER_ARG("gc=0x%x fbo=0x%x attachment=0x%04x", gc, fbo, attachment);

    if (!fbo || -1 == attachIndex)
    {
        goto OnError;
    }

    attachPoint = &fbo->attachPoint[attachIndex];
    switch (attachPoint->objType)
    {
    case GL_RENDERBUFFER:
        {
            __GLrenderbufferObject *rbo = NULL;
            __GLchipRenderbufferObject *chipRBO = NULL;

            rbo = (__GLrenderbufferObject*)attachPoint->object;
            GL_ASSERT(rbo);

            chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
            GL_ASSERT(chipRBO);

            if (chipRBO->shadow.surface != gcvNULL)
            {
                surfView.surf = chipRBO->shadow.surface;
            }
            else
            {
                surfView.surf = chipRBO->surface;
            }

            if (image)
            {
                *image = rbo->eglImage;
            }
        }
        break;

    case GL_TEXTURE:
        {
            __GLtextureObject *tex = (__GLtextureObject*)attachPoint->object;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(tex->privateData);
            __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
            __GLchipResourceShadow *shadow= &chipMipLevel->shadow[attachPoint->slice];

            /* If created extra RT surface, return that surface; otherwise return texture surface */
            if (shadow->surface)
            {
                surfView.surf = shadow->surface;
            }
            else if (texInfo->direct.directRender)
            {
                surfView.surf = texInfo->direct.source;
            }
            else
            {
                surfView = gcChipGetTextureSurface(chipCtx, tex, attachPoint->layered, attachPoint->level, attachPoint->slice);
            }

            if (image)
            {
                *image = texInfo->eglImage.image;
            }
        }
        break;

    default:
        break;
    }

OnError:
    gcmFOOTER_ARG("return={0x%x, %d, %d}", surfView.surf, surfView.firstSlice, surfView.numSlices);
    return surfView;
}

gceSTATUS
gcChipPickReadBufferForFBO(
    __GLcontext *gc
    )
{
    __GLframebufferObject *fbo = gc->frameBuffer.readFramebufObj;
    gcsSURF_VIEW rtView, dView, sView;
    gceSTATUS status;
    GLboolean readYInverted = GL_FALSE;

    gcmHEADER_ARG("gc=0x%x", gc);

    rtView = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, fbo->readBuffer, gcvNULL);
    dView  = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, GL_DEPTH_ATTACHMENT, gcvNULL);
    sView  = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, GL_STENCIL_ATTACHMENT, gcvNULL);

    if (rtView.surf)
    {
        readYInverted = (gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
    }
    else if (dView.surf)
    {
        readYInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
    }
    else if (sView.surf)
    {
        readYInverted = (gcoSURF_QueryFlags(sView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
    }

    if (fbo->fbLayered)
    {
        rtView.firstSlice = 0;
        dView.firstSlice  = 0;
        sView.firstSlice  = 0;
    }
    else
    {
        rtView.numSlices = 1;
        dView.numSlices = 1;
        sView.numSlices = 1;
    }

    gcmONERROR(gcChipSetReadBuffers(gc, fbo->fbIntMask, &rtView, &dView, &sView, readYInverted, fbo->fbLayered));

OnError:
    gcmFOOTER();
    return status;

}

gceSTATUS
gcChipPickDrawBufferForFBO(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;
    gcsSURF_VIEW         rtViews[__GL_MAX_DRAW_BUFFERS];
    gcsSURF_VIEW         dView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         sView = {gcvNULL, 0, 1};
    GLint sBpp = 0;
    GLvoid * image = gcvNULL;
    GLboolean drawYInverted = GL_FALSE;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint i = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    dView = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, GL_DEPTH_ATTACHMENT, &image);

    if (dView.surf)
    {
        drawYInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
    }

    if (image)
    {
        gcmONERROR(gcChipSetImageSrc(image, dView.surf));
    }

    sView = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, GL_STENCIL_ATTACHMENT, &image);

    if (sView.surf)
    {
        drawYInverted = (gcoSURF_QueryFlags(sView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
    }

    image = gcvNULL;

    if (image)
    {
        gcmONERROR(gcChipSetImageSrc(image, sView.surf));
    }

    if (sView.surf)
    {
        __GLformatInfo *formatInfo = __glGetFramebufferFormatInfo(gc, fbo, GL_STENCIL_ATTACHMENT);
        sBpp = formatInfo ? formatInfo->stencilSize : 0;
    }

    if (chipCtx->drawStencilMask != (GLuint) ((1 << sBpp) - 1))
    {
        chipCtx->drawStencilMask = (1 << sBpp) - 1;
        chipCtx->chipDirty.uDefer.sDefer.stencilRef = 1;
    }

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        image = gcvNULL;

        rtViews[i] = gcChipGetFramebufferAttachedSurfaceAndImage(gc, fbo, fbo->drawBuffers[i], &image);

        if (image)
        {
            gcmONERROR(gcChipSetImageSrc(image, rtViews[i].surf));
        }

        if (rtViews[i].surf)
        {
            drawYInverted = (gcoSURF_QueryFlags(rtViews[i].surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }

        if (fbo->fbLayered)
        {
            rtViews[i].firstSlice = 0;
        }
        else
        {
            rtViews[i].numSlices = 1;
        }
    }

    if (fbo->fbLayered)
    {
        dView.firstSlice = 0;
        sView.firstSlice = 0;
    }
    else
    {
        dView.numSlices = 1;
        sView.numSlices = 1;
    }

    gcmONERROR(gcChipSetDrawBuffers(gc,
                                    fbo->fbIntMask,
                                    fbo->fbFloatMask,
                                    rtViews,
                                    &dView,
                                    &sView,
                                    drawYInverted,
                                    fbo->fbSamples,
                                    fbo->useDefault,
                                    fbo->defaultWidth,
                                    fbo->defaultHeight,
                                    fbo->fbLayered,
                                    fbo->fbMaxLayers));

OnError:
    gcmFOOTER();
    return status;
}


/*
** For FBO rendering, this function is to sync shadow(RT) to master at some sync points.
*/
gceSTATUS
gcChipFboSyncFromShadow(
    __GLcontext *gc,
    __GLframebufferObject *fbo
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x fbo=0x%x", gc, fbo);

    /*
    ** Default fbo(window drawable) won't have indirectly rendering.
    */
    if (fbo->name)
    {
        GLuint attachIdx;
        __GLfboAttachPoint *attachPoint;

        /* Loop through all draw buffer, if it was from texture */
        for (attachIdx = 0; attachIdx < __GL_MAX_ATTACHMENTS; ++attachIdx)
        {
            attachPoint = &fbo->attachPoint[attachIdx];
            if (attachPoint->objType == GL_TEXTURE)
            {
                __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
                __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
                __GLchipFmtMapInfo *fmtMapInfo = texInfo->mipLevels[attachPoint->level].formatMapInfo;

                if ((texInfo->eglImage.image) ||
                    (texInfo->direct.source && !texInfo->direct.directRender)  ||
                    (gc->texture.shared->refcount > 1 &&
                     fmtMapInfo &&
                     (fmtMapInfo->flags & (__GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE | __GL_CHIP_FMTFLAGS_LAYOUT_DIFF_READ_WRITE))
                    )
                   )
                {
                    gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc,
                                                               texObj,
                                                               attachPoint->face,
                                                               attachPoint->level,
                                                               attachPoint->layer));
                }

                /* Sync Master to direct source. */
                if (texInfo->direct.source        &&
                    !texInfo->direct.directSample &&
                    attachPoint->level == 0
                    )
                {
                    gcmONERROR(gcChipTexDirectSourceSyncFromMipSlice(gc, texObj));
                }
            }
            else if (attachPoint->objType == GL_RENDERBUFFER)
            {
                __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
                if (rbo && rbo->eglImage)
                {
                    gcChipRboSyncFromShadow(gc, rbo);
                }
            }
        }
    }

 OnError:
    gcmFOOTER();
    return status;
}

/* Temporarily for chrome freon. */
gceSTATUS
gcChipFboSyncFromShadowFreon(
    __GLcontext *gc,
    __GLframebufferObject *fbo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLuint attachIdx;
    __GLfboAttachPoint *attachPoint;

    gcmHEADER_ARG("gc=0x%x fbo=0x%x", gc, fbo);

    /*
    ** Default fbo(window drawable) won't have indirectly rendering.
    */
    if (!fbo->name)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Loop through all draw buffer, if it was from texture */
    for (attachIdx = 0; attachIdx < __GL_MAX_ATTACHMENTS; ++attachIdx)
    {
        attachPoint = &fbo->attachPoint[attachIdx];

        if (attachPoint->objType == GL_TEXTURE)
        {
            khrEGL_IMAGE_PTR image;
            __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
            __GLchipFmtMapInfo *fmtMapInfo = texInfo->mipLevels[attachPoint->level].formatMapInfo;

            image = (khrEGL_IMAGE_PTR) texInfo->eglImage.image;

            if ((image && image->magic == KHR_EGL_IMAGE_MAGIC_NUM &&
                 image->type == KHR_IMAGE_LINUX_DMA_BUF) ||
                (fmtMapInfo &&
                 (fmtMapInfo->flags & (__GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE |
                                       __GL_CHIP_FMTFLAGS_LAYOUT_DIFF_READ_WRITE))))
            {
                gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc,
                                                           texObj,
                                                           attachPoint->face,
                                                           attachPoint->level,
                                                           attachPoint->layer));
            }
        }
    }

 OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS gcChipFBOSyncAttachment(__GLcontext *gc, __GLfboAttachPoint * attachPoint)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x  attachPoint=0x%x", gc, attachPoint);

    if (attachPoint->objType == GL_TEXTURE)
    {
        __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
        if(texObj)
        {
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
            if ((texInfo && texInfo->eglImage.image) ||
                (texInfo && texInfo->direct.source) )
            {
                gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc,
                    texObj,
                    attachPoint->face,
                    attachPoint->level,
                    attachPoint->layer));
            }
            /* Sync Master to direct source. */
            if(texInfo && texInfo->direct.source &&
                !texInfo->direct.directSample &&
                attachPoint->level == 0
                )
            {
                gcmONERROR(gcChipTexDirectSourceSyncFromMipSlice(gc,
                    texObj));
            }
        }
    }
    else if (attachPoint->objType == GL_RENDERBUFFER)
    {
        __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
        if (rbo && rbo->eglImage)
        {
            gcChipRboSyncFromShadow(gc, rbo);

        }
    }
OnError:
    gcmFOOTER();
    return status;

}


GLvoid
__glChipFramebufferRenderbuffer(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    __GLrenderbufferObject *rbo,
    __GLfboAttachPoint *preAttach
    )
{
   gcChipFBOSyncAttachment(gc, preAttach);
}

GLboolean
__glChipFramebufferTexture(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLsizei samples,
    GLint zoffset,
    GLboolean layered,
    __GLfboAttachPoint *preAttach
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLmipMapLevel *mipmap;

    gcmHEADER_ARG("gc=0x%x fbo=0x%x attachIndex=%d texObj=0x%x level=%d face=%d samples=%d zoffset=%d layered=%d preAttach=0x%x",
        gc, fbo, attachIndex, texObj, level, face, samples, zoffset, layered, preAttach);

    gcmONERROR(gcChipFBOSyncAttachment(gc, preAttach));
    mipmap = texObj ? &texObj->faceMipmap[face][level] : gcvNULL;

    if (mipmap && ((mipmap->width * mipmap->height * mipmap->depth) != 0))
    {
        __GLchipContext         *chipCtx        = CHIP_CTXINFO(gc);
        __GLchipTextureInfo     *texInfo        = (__GLchipTextureInfo*)texObj->privateData;
        __GLfboAttachPoint      *attachPoint    = &fbo->attachPoint[attachIndex];
        __GLchipMipmapInfo      *chipMipLevel   = &texInfo->mipLevels[level];
        __GLchipFmtMapInfo      *formatMapInfo  = chipMipLevel->formatMapInfo;
        GLint slice = attachPoint->slice;

        if (texInfo->eglImage.source)
        {
            /* For EGLImage target texture, base level may be not allocated yet. */
            gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
        }

        /* Sync direct source to mipmap surface.*/
        if (texInfo->direct.source &&
            level == 0             &&
            face == 0)
        {
            /* Make sure mipmap surface exist, before bind to fbo.*/
            texInfo->direct.dirty = gcvTRUE;
            gcmONERROR(gcChipTexSyncDirectVIV(gc, texObj));
        }

        /* If it's not renderable format, no need to create shadow surface. */
        if ((formatMapInfo == gcvNULL) || (formatMapInfo->writeFormat == gcvSURF_UNKNOWN))
        {
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }

        samples = gcmMAX((GLsizei)texObj->samplesUsed, samples);

        if (gcChipTexNeedShadow(gc, texObj, texInfo, formatMapInfo, samples, &attachPoint->samplesUsed))
        {
            __GLimageUser *fboList = texObj->fboList;
            gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, level, slice);

            if (texView.surf && chipMipLevel->shadow[slice].shadowDirty && chipMipLevel->shadow[slice].surface)
            {
                gcsSURF_VIEW shadowView = {chipMipLevel->shadow[slice].surface, 0, 1};

                gcmONERROR(gcoSURF_ResolveRect(&shadowView, &texView, gcvNULL));

                chipMipLevel->shadow[slice].shadowDirty = gcvFALSE;
                chipMipLevel->shadow[slice].masterDirty = GL_TRUE;
            }

            gcmONERROR(gcChipRellocShadowResource(gc,
                                                  texView.surf,
                                                  attachPoint->samplesUsed,
                                                  &chipMipLevel->shadow[slice],
                                                  formatMapInfo,
                                                  GL_TRUE));

            /* TO_DO, put at render real happens */
            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);

            /* If there is a shadow surface , the fbo which own this texture should check compelete again */
            while (fboList)
            {
                __GLframebufferObject *fbo = (__GLframebufferObject*)fboList->imageUser;
                if (fbo && fbo->name)
                {
                    __GL_FRAMEBUFFER_COMPLETE_DIRTY(fbo);
                }
                fboList = fboList->next;
            }
        }
        /* HAL texobj might not be created then, it will be created when TexImg* */
        else if (texInfo->object)
        {
            /* Set up direct render into mipmap surface */
            gcmONERROR(gcoTEXTURE_RenderIntoMipMap2(texInfo->object,
                                                    level,
                                                    chipMipLevel->shadow[slice].masterDirty));
            /* TO_DO, put at render real happens */
            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
        }
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

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
GLboolean
__glChipIsFramebufferComplete(
    __GLcontext *gc,
    __GLframebufferObject *framebufferObj
    )
{
    GLuint i;
    GLuint fbwidth = 0, fbheight = 0, fbsamples = 0;
    GLboolean fbFixedSampleLoc = GL_TRUE;
    GLboolean fbLayered = GL_FALSE;
    GLenum fbLayeredTarget = __GL_TEXTURE_2D_INDEX;
    GLenum depthObjType = GL_NONE;
    GLenum stencilObjType = GL_NONE;
    GLuint depthObjName = 0;
    GLuint stencilObjName = 0;
    GLboolean depthObjSet = GL_FALSE;
    GLboolean stencilObjSet = GL_FALSE;
    GLboolean noImageAttached = GL_TRUE;
    GLboolean shadowRender = GL_FALSE;
    GLboolean hasDefaultDimesion = GL_FALSE;
    GLboolean useDefault = GL_FALSE;
    GLbitfield attribFlag = 0;
    GLuint maxLayers = ~(GLuint)0;
    GLboolean ret = GL_TRUE;
    GLenum error = GL_FRAMEBUFFER_COMPLETE;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x framebufferObj=0x%x", gc, framebufferObj);

    /* Default FBO must already be checked when make current. */
    GL_ASSERT(framebufferObj->name || (framebufferObj->flag & __GL_FRAMEBUFFER_IS_CHECKED));

    if (framebufferObj->flag & __GL_FRAMEBUFFER_IS_CHECKED)
    {
        ret = (framebufferObj->flag & __GL_FRAMEBUFFER_IS_COMPLETE) ? GL_TRUE : GL_FALSE;
        goto OnExit;
    }

    /* Zero flag and masks before starting evaluation */
    framebufferObj->flag = 0;
    framebufferObj->fbIntMask = 0;
    framebufferObj->fbFloatMask = 0;
    framebufferObj->fbUIntMask = 0;
    framebufferObj->fbUnormMask = 0;

    for (i = 0; i < __GL_MAX_ATTACHMENTS; ++i)
    {
        GLuint width = 0, height = 0, samples = 0;
        GLenum layeredTarget = __GL_TEXTURE_2D_INDEX;
        GLboolean fixedSampleLoc = GL_TRUE;
        GLboolean layered = GL_FALSE;
        GLboolean renderable = GL_TRUE;

        GLuint maskInt = 0;
        GLuint maskFloat = 0;
        GLuint maskUint = 0;
        GLuint maskUnorm = 0;

        __GLformatInfo *formatInfo = NULL;
        __GLrenderbufferObject *rbo = gcvNULL;
        __GLtextureObject *tex = gcvNULL;
        __GLmipMapLevel *mipmap = gcvNULL;
        __GLchipRenderbufferObject *chipRBO = gcvNULL;
        __GLchipTextureInfo *texInfo = gcvNULL;
        __GLchipMipmapInfo *chipMipLevel = gcvNULL;

        __GLfboAttachPoint *attachPoint = &framebufferObj->attachPoint[i];

        if (attachPoint->objType == GL_NONE)
        {
            hasDefaultDimesion = (framebufferObj->defaultWidth > 0) &&
                                 (framebufferObj->defaultHeight > 0);
            continue;
        }

        GL_ASSERT(attachPoint->objName != 0);
        switch (attachPoint->objType)
        {
        case GL_RENDERBUFFER:
            rbo = (__GLrenderbufferObject*)attachPoint->object;

            if (!rbo)
            {
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                goto OnExit;
            }

            /* Image dimension not zero */
            if ((rbo->width == 0) || (rbo->height == 0) || (rbo->formatInfo == gcvNULL))
            {
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                goto OnExit;
            }

            chipRBO = (__GLchipRenderbufferObject *)rbo->privateData;

            GL_ASSERT(chipRBO);

            if (chipRBO->shadow.surface != gcvNULL)
            {
                shadowRender = GL_TRUE;
            }

            width = rbo->width;
            height = rbo->height;
            formatInfo = rbo->formatInfo;
            samples = rbo->samplesUsed;
            fixedSampleLoc = GL_TRUE;
            attribFlag |= __GL_FRAMEBUFFER_ATTACH_RBO;
            layered = attachPoint->layered;
            maxLayers = 1;
            GL_ASSERT(!layered);
            break;

        case GL_TEXTURE:
            tex = (__GLtextureObject*)attachPoint->object;
            if (!tex)
            {
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                goto OnExit;
            }
            mipmap = &tex->faceMipmap[attachPoint->face][attachPoint->level];

            if (mipmap->width == 0 || mipmap->height == 0 || mipmap->formatInfo == gcvNULL)
            {
                error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                goto OnExit;
            }

            fixedSampleLoc = GL_TRUE;
            samples = attachPoint->samplesUsed;
            layered = attachPoint->layered;
            layeredTarget = tex->targetIndex;

            switch (tex->targetIndex)
            {
            case __GL_TEXTURE_3D_INDEX:
                if ((attachPoint->layer >= mipmap->depth) ||
                    (layered && (mipmap->depth >= (GLint)gc->constants.maxTextureDepthSize)))
                {
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                    goto OnExit;
                }
                if ((GLuint)mipmap->depth < maxLayers)
                {
                    maxLayers = mipmap->depth;
                }
                break;

            case __GL_TEXTURE_2D_ARRAY_INDEX:
                if ((attachPoint->layer >= mipmap->arrays) ||
                    (layered && (mipmap->arrays >= (GLint)gc->constants.maxTextureDepthSize)))
                {
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                    goto OnExit;
                }
                if ((GLuint)mipmap->arrays < maxLayers)
                {
                    maxLayers = mipmap->arrays;
                }
                break;

            case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
                if ((attachPoint->layer >= mipmap->arrays) ||
                    (layered && (mipmap->arrays >= (GLint)gc->constants.maxTextureDepthSize)))
                {
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                    goto OnExit;
                }
                /* fall through */
            case __GL_TEXTURE_2D_MS_INDEX:
                /* overwrite with MSAA texture properties */
                samples = tex->samplesUsed;
                fixedSampleLoc = tex->fixedSampleLocations;
                if ((GLuint)mipmap->arrays < maxLayers)
                {
                    maxLayers = mipmap->arrays;
                }
                break;
            case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
                if ((attachPoint->layer >= mipmap->arrays) ||
                    (layered && (mipmap->arrays >= (GLint)gc->constants.maxTextureDepthSize)))
                {
                    error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
                    goto OnExit;
                }
                if ((GLuint)mipmap->arrays < maxLayers)
                {
                    maxLayers = mipmap->arrays;
                }
                break;
            case __GL_TEXTURE_CUBEMAP_INDEX:
                if (maxLayers > 6)
                {
                    maxLayers = 6;
                }
                break;
            default:
                maxLayers = 1;
                break;
            }

            renderable &= tex->canonicalFormat;

            texInfo = (__GLchipTextureInfo *)tex->privateData;
            chipMipLevel = &texInfo->mipLevels[attachPoint->level];
            if (chipMipLevel->shadow[attachPoint->slice].surface != gcvNULL)
            {
                shadowRender = GL_TRUE;
            }

            width = mipmap->width;
            height = mipmap->height;
            formatInfo = mipmap->formatInfo;
            attribFlag |= __GL_FRAMEBUFFER_ATTACH_TEX;
            break;

        default:
            GL_ASSERT(GL_FALSE);
            break;
        }

        maskUint = ((GL_UNSIGNED_INT == formatInfo->category)? 0x1: 0x0) << i;
        maskInt = ((maskUint || (GL_INT == formatInfo->category))? 0x1: 0x0) << i;
        maskUnorm = ((GL_UNSIGNED_NORMALIZED == formatInfo->category)? 0x1: 0x0) << i;
        maskFloat = ((GL_FLOAT == formatInfo->category)? 0x1: 0x0) << i;

        if (i < __GL_MAX_COLOR_ATTACHMENTS)
        {
            renderable = renderable && (formatInfo->renderable && (formatInfo->baseFormat != GL_DEPTH_COMPONENT &&
                                                                   formatInfo->baseFormat != GL_DEPTH_STENCIL &&
                                                                   formatInfo->baseFormat != GL_STENCIL));
        }
        else if (__GL_DEPTH_ATTACHMENT_POINT_INDEX == i)
        {
            renderable = renderable && (formatInfo->renderable && (formatInfo->baseFormat == GL_DEPTH_COMPONENT ||
                                                                   formatInfo->baseFormat == GL_DEPTH_STENCIL));
            depthObjType = attachPoint->objType;
            depthObjName = attachPoint->objName;
            depthObjSet = GL_TRUE;
        }
        else if (__GL_STENCIL_ATTACHMENT_POINT_INDEX == i)
        {
            renderable = renderable && (formatInfo->renderable && (formatInfo->baseFormat == GL_DEPTH_STENCIL ||
                                                                   formatInfo->baseFormat == GL_STENCIL));
            stencilObjType = attachPoint->objType;
            stencilObjName = attachPoint->objName;
            stencilObjSet = GL_TRUE;
        }

        if (!renderable)
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            goto OnExit;
        }

        /*
        ** Check if all images have same dimension and same samples
        */
        if (noImageAttached)
        {
            fbwidth = width;
            fbheight = height;
            fbsamples = samples;
            fbFixedSampleLoc = fixedSampleLoc;
            fbLayered = layered;
            fbLayeredTarget = layeredTarget;
        }
        /* Make the latest cts case follow the ES3.2 spec. */
        else if((gc->apiVersion == __GL_API_VERSION_ES20) &&
                (fbwidth != width || fbheight != height) &&
                ((chipCtx->patchId != gcvPATCH_GTFES30) || (gc->constants.majorVersion == 2)))
        {
            /* ES30 will not generate the error */
            error = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
            goto OnExit;
        }
        else if ((fbsamples != samples) || (fbFixedSampleLoc != fixedSampleLoc))
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
            goto OnExit;
        }
        else if ((fbLayered != layered) ||
                 (fbLayered && (fbLayeredTarget != layeredTarget)))
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT;
            goto OnExit;
        }

        /*
        ** As ES30 SPEC required, both depth and stencil attach point must refer to same image.
        ** For ES20, SPEC implicitly indicates implementation cannot support the combination,
        ** And our HW indeed didn't support it.
        */
        if (depthObjSet && stencilObjSet)
        {
            if ((depthObjType != stencilObjType) ||
                (depthObjName != stencilObjName))
            {
                error = GL_FRAMEBUFFER_UNSUPPORTED;
                goto OnExit;
            }
        }

        framebufferObj->fbIntMask |= maskInt;
        framebufferObj->fbUIntMask |= maskUint;
        framebufferObj->fbUnormMask |= maskUnorm;
        framebufferObj->fbFloatMask |= maskFloat;

        noImageAttached = GL_FALSE;
    }

    /*
    ** There is at least one image attached to the framebuffer.
    */
    if (noImageAttached)
    {
        if (hasDefaultDimesion)
        {
            fbwidth = framebufferObj->defaultWidth;
            fbheight = framebufferObj->defaultHeight;
            if (framebufferObj->defaultSamples > 0)
            {
                GLint i;
                for (i = 0; i < chipCtx->numSamples; i++)
                {
                    if (chipCtx->samples[i] >= framebufferObj->defaultSamples)
                    {
                        break;
                    }
                }

                framebufferObj->defaultSamplesUsed = chipCtx->samples[i];
            }
            else
            {
                framebufferObj->defaultSamplesUsed = framebufferObj->defaultSamples;
            }

            fbsamples = framebufferObj->defaultSamplesUsed;
            fbLayered = (framebufferObj->defaultLayers > 0);
            maxLayers = framebufferObj->defaultLayers;
            useDefault = GL_TRUE;
        }
        else
        {
            error = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
            goto OnExit;
        }
    }

    /* Success */
    framebufferObj->flag |= (__GL_FRAMEBUFFER_IS_COMPLETE | __GL_FRAMEBUFFER_IS_CHECKED | attribFlag);
    framebufferObj->checkCode = GL_FRAMEBUFFER_COMPLETE;
    framebufferObj->fbWidth = fbwidth;
    framebufferObj->fbHeight = fbheight;
    framebufferObj->fbSamples = fbsamples;
    framebufferObj->shadowRender = shadowRender;
    framebufferObj->useDefault = useDefault;
    framebufferObj->fbLayered = fbLayered;
    framebufferObj->fbMaxLayers = maxLayers;

OnExit:
    if (error != GL_FRAMEBUFFER_COMPLETE)
    {
        framebufferObj->flag |=  __GL_FRAMEBUFFER_IS_CHECKED;
        framebufferObj->checkCode = error;
        framebufferObj->fbWidth = framebufferObj->fbHeight = 0;
        framebufferObj->fbSamples = 0;
        framebufferObj->fbIntMask = 0;
        framebufferObj->fbFloatMask = 0;
        framebufferObj->fbLayered = 0;
        framebufferObj->fbMaxLayers = 0;

        ret = GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

/********************************************************************
**
**  _BlitFramebufferResolve
**
**  Function to use resolve to do the framebuffer blit.
**  This is the fast path and first choice when do blit.
**  But it will be limited by scaling or other features.
**
**  Parameters: The framebuffer blit arguments
**  Return Value: Blit successful
**
********************************************************************/
__GL_INLINE gceSTATUS
gcChipBlitFramebufferResolve(
    __GLcontext *gc,
    GLint srcX0,
    GLint srcY0,
    GLint srcX1,
    GLint srcY1,
    GLint dstX0,
    GLint dstY0,
    GLint dstX1,
    GLint dstY1,
    GLbitfield *mask,
    GLboolean  xReverse,
    GLboolean  yReverse,
    GLenum     filter
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);


    GLint srcWidth  = 0;
    GLint srcHeight = 0;
    GLint dstWidth  = 0;
    GLint dstHeight = 0;
    gcsSURF_VIEW pixelPlaneView = {gcvNULL, 0, 1};
    gcsSURF_RESOLVE_ARGS rlvArgs = {0};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x srcX0=%d srcY0=%d srcX1=%d srcY1=%d dstX0=%d dstY0=%d "
                  "dstX1=%d desY1=%d mask=0x%x xReverse=%d yReverse=%d filter=0x%04x",
                   gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstX1, dstY1,
                   mask, xReverse, yReverse, filter);

    /* Resolve cannot support scissor test with rect */
    if (gc->state.enables.scissorTest)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Resolve does not support x reverse */
    if (xReverse || yReverse)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Resolve cannot support scale */
    if (((srcX1 - srcX0) != (dstX1 - dstX0)) ||
        ((srcY1 - srcY0) != (dstY1 - dstY0)))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* webgl 2.0 spec: For any source pixel lying outside the read framebuffer, the corresponding pixel remains untouched */
    if (srcX0 < 0)
    {
        dstX0 = dstX0 - srcX0;
        srcX0 = 0;
    }

    if (srcY0 < 0)
    {
        dstY0 = dstY0 - srcY0;
        srcY0 = 0;
    }

    rlvArgs.version = gcvHAL_ARG_VERSION_V2;
    rlvArgs.uArgs.v2.srcOrigin.x = __GL_MAX(0, srcX0);
    rlvArgs.uArgs.v2.srcOrigin.y = __GL_MAX(0, srcY0);
    rlvArgs.uArgs.v2.dstOrigin.x = __GL_MAX(0, dstX0);
    rlvArgs.uArgs.v2.dstOrigin.y = __GL_MAX(0, dstY0);
    rlvArgs.uArgs.v2.numSlices = 1;

    if (*mask & GL_COLOR_BUFFER_BIT)
    {
        GLuint i;

        gcmONERROR(gcoSURF_GetSize(chipCtx->readRtView.surf, (gctUINT*)&srcWidth, (gctUINT*)&srcHeight, NULL));
        gcmONERROR(gcChipFboSyncFromMasterSurface(gc, &chipCtx->readRtView, GL_TRUE));
        /* Get pixel plan for msaa source */
        gcmONERROR(gcChipUtilGetPixelPlane(gc, &chipCtx->readRtView, &pixelPlaneView));

        rlvArgs.uArgs.v2.rectSize.x  = __GL_MIN(srcWidth,  (srcX1 - srcX0));
        rlvArgs.uArgs.v2.rectSize.y  = __GL_MIN(srcHeight, (srcY1 - srcY0));

        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            gcsSURF_VIEW rtView = {chipCtx->drawRtViews[i].surf, chipCtx->drawRtViews[i].firstSlice, chipCtx->drawRtViews[i].numSlices};
            if (rtView.surf)
            {
                /* Set up the dst surface and rect */
                gcmONERROR(gcoSURF_GetSize(rtView.surf, (gctUINT*)&dstWidth, (gctUINT*)&dstHeight, NULL));
                gcmONERROR(gcChipFboSyncFromMasterSurface(gc, &rtView, GL_FALSE));

                if (rlvArgs.uArgs.v2.rectSize.x != __GL_MIN(dstWidth,  (dstX1 - dstX0)) ||
                    rlvArgs.uArgs.v2.rectSize.y != __GL_MIN(dstHeight, (dstY1 - dstY0))
                   )
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                /* If the read framebuffer is layered (see section 9.8), pixel values are read from layer zero.
                ** If the draw framebuffer is layered, pixel values are written to layer zero.
                ** If both read and draw framebuffers are layered, the blit operation is still performed only on layer zero.
                */
                pixelPlaneView.numSlices = rtView.numSlices = 1;
                gcmONERROR(gcoSURF_ResolveRect(&pixelPlaneView, &rtView, &rlvArgs));
            }
        }
        /* Destroy pixel plane if needed */
        gcmONERROR(gcChipUtilDestroyPixelPlane(gc, &chipCtx->readRtView, &pixelPlaneView));

        *mask &= ~GL_COLOR_BUFFER_BIT;
    }

    if (*mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        gceSURF_FORMAT readFmt, drawFmt;
        gcsSURF_VIEW *readDsView = chipCtx->readDepthView.surf ? &chipCtx->readDepthView : &chipCtx->readStencilView;
        gcsSURF_VIEW *drawDsView = chipCtx->drawDepthView.surf ? &chipCtx->drawDepthView : &chipCtx->drawStencilView;
        gctUINT drawDsNumSlice = chipCtx->drawDepthView.surf ? chipCtx->drawDepthView.numSlices : chipCtx->drawStencilView.numSlices;

        /* If both read/draw are valid, they must be same. */
        GL_ASSERT(!(chipCtx->readDepthView.surf && chipCtx->readStencilView.surf) ||
                    chipCtx->readDepthView.surf == chipCtx->readStencilView.surf);
        GL_ASSERT(!(chipCtx->drawDepthView.surf && chipCtx->drawStencilView.surf) ||
                    chipCtx->drawDepthView.surf == chipCtx->drawStencilView.surf);

        /* Resolve in fact cannot do depth-only/stencil-only copy for packed ds format */
        gcmONERROR(gcoSURF_GetFormat(readDsView->surf, gcvNULL, &readFmt));
        gcmONERROR(gcoSURF_GetFormat(drawDsView->surf, gcvNULL, &drawFmt));

        if ((readFmt == gcvSURF_D24S8 || readFmt == gcvSURF_D24S8_1_A8R8G8B8) &&
            (drawFmt == gcvSURF_D24S8 || drawFmt == gcvSURF_D24S8_1_A8R8G8B8) &&
            (!(*mask & GL_DEPTH_BUFFER_BIT) || !(*mask & GL_STENCIL_BUFFER_BIT))
            )
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Blit from read depth buffer to draw depth buffer */
        /* Consider stencil and depth sharing same surface */
        if (readDsView->surf && drawDsView->surf)
        {
            gcmONERROR(gcoSURF_GetSize(readDsView->surf, (gctUINT*)&srcWidth, (gctUINT*)&srcHeight, NULL));
            gcmONERROR(gcChipFboSyncFromMasterSurface(gc, readDsView, GL_TRUE));
            /* Get Pixel Plane if source is msaa surface */
            gcmONERROR(gcChipUtilGetPixelPlane(gc, readDsView, &pixelPlaneView));

            rlvArgs.uArgs.v2.rectSize.x = __GL_MIN(srcWidth,  (srcX1 - srcX0));
            rlvArgs.uArgs.v2.rectSize.y = __GL_MIN(srcHeight, (srcY1 - srcY0));

            gcmONERROR(gcoSURF_GetSize(drawDsView->surf, (gctUINT*)&dstWidth, (gctUINT*)&dstHeight, NULL));
            gcmONERROR(gcChipFboSyncFromMasterSurface(gc, drawDsView, GL_FALSE));

            if (rlvArgs.uArgs.v2.rectSize.x != __GL_MIN(dstWidth,  (dstX1 - dstX0)) ||
                rlvArgs.uArgs.v2.rectSize.y != __GL_MIN(dstHeight, (dstY1 - dstY0))
               )
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            /* If the read framebuffer is layered (see section 9.8), pixel values are read from layer zero.
            ** If the draw framebuffer is layered, pixel values are written to layer zero.
            ** If both read and draw framebuffers are layered, the blit operation is still performed only on layer zero.
            */
            pixelPlaneView.numSlices = drawDsView->numSlices = 1;

            gcmONERROR(gcoSURF_ResolveRect(&pixelPlaneView, drawDsView, &rlvArgs));

            /* Destroy pixel plane if needed */
            gcmONERROR(gcChipUtilDestroyPixelPlane(gc, readDsView, &pixelPlaneView));

            /* restore the numSlice for drawDsView*/
            drawDsView->numSlices = drawDsNumSlice;
        }

        /*
        ** Clear stencil bit also, as it's always same buffer.
        ** We don't support blit depth or stencil only case.
        */
        *mask &= ~(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

OnError:
    gcmFOOTER();
    return status;
}

/********************************************************************
**
**  _BlitFramebuffer3Dblit
**
**  Function to use 3dblit to do the framebuffer blit.
**  Only called when resolve blit failed.
**  Support 3dblit should cover all the cases.
**
**  Parameters: The framebuffer blit arguments
**  Return Value: Blit successful
**
********************************************************************/
__GL_INLINE gceSTATUS
gcChipBlitFramebuffer3Dblit(
    __GLcontext *gc,
    GLint srcX0,
    GLint srcY0,
    GLint srcX1,
    GLint srcY1,
    GLint dstX0,
    GLint dstY0,
    GLint dstX1,
    GLint dstY1,
    GLbitfield *mask,
    GLboolean   xReverse,
    GLboolean   yReverse,
    GLenum      filter
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gscSURF_BLITDRAW_BLIT blitArgs;
    gcsSURF_VIEW masterView = {gcvNULL, 0, 1};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x srcX0=%d srcY0=%d srcX1=%d srcY1=%d dstX0=%d dstY0=%d "
                  "dstX1=%d desY1=%d mask=0x%x xReverse=%d yReverse=%d filter=0x%04x",
                   gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstX1, dstY1,
                   mask, xReverse, yReverse, filter);

    /* Do not clip the src/dst rect here, otherwise, scale factor will be changed.
    ** Draw simulation will let HW do the clipping.
    */
    __GL_MEMZERO(&blitArgs, sizeof(blitArgs));

    blitArgs.scissorEnabled = gc->state.enables.scissorTest;
    blitArgs.scissor.left   = gc->state.scissor.scissorX;
    blitArgs.scissor.right  = gc->state.scissor.scissorX + gc->state.scissor.scissorWidth;
    blitArgs.scissor.top    = gc->state.scissor.scissorY;
    blitArgs.scissor.bottom = gc->state.scissor.scissorY + gc->state.scissor.scissorHeight;

    if (chipCtx->drawYInverted)
    {
        GLint temp = blitArgs.scissor.top;
        blitArgs.scissor.top = (gctINT32)(chipCtx->drawRTHeight - blitArgs.scissor.bottom);
        blitArgs.scissor.bottom = (gctINT32)(chipCtx->drawRTHeight - temp);
    }

    blitArgs.srcRect.left   = srcX0;
    blitArgs.srcRect.top    = srcY0;
    blitArgs.srcRect.right  = srcX1;
    blitArgs.srcRect.bottom = srcY1;
    blitArgs.dstRect.left   = dstX0;
    blitArgs.dstRect.top    = dstY0;
    blitArgs.dstRect.right  = dstX1;
    blitArgs.dstRect.bottom = dstY1;
    blitArgs.filterMode     = gcChipUtilConvertFilter(filter);;
    blitArgs.xReverse       = xReverse;
    blitArgs.yReverse       = yReverse;

    if (*mask & GL_COLOR_BUFFER_BIT)
    {
        GLuint i;
        gcsSURF_VIEW shadowView = {gcvNULL, 0, 1};
        __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

        /* For INT/UINT src/dst, 3DBlit doesn't have shader to support the blit yet.
        ** No need to check dst, for GLcore guarantees them matched.
        */
        if (readFBO->name)
        {
            GLuint rAttachPointMask = 0x1 << __glMapAttachmentToIndex(readFBO->readBuffer);

            if ((rAttachPointMask & readFBO->fbIntMask) || (rAttachPointMask & readFBO->fbUIntMask))
            {
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }
        }

        masterView = gcChipFboSyncFromShadowSurface(gc, &chipCtx->readRtView, GL_TRUE);

        if (!masterView.surf || ((masterView.surf->tiling == gcvMULTI_SUPERTILED) && (chipCtx->chipFeature.hwFeature.indirectRTT)))
        {
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        /* Get usable texture if needed */
        gcmONERROR(gcChipUtilGetShadowTexView(gc, &masterView, &shadowView));

        GL_ASSERT((shadowView.surf != chipCtx->readRtView.surf) || (chipCtx->readRtView.firstSlice == 0));

        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (chipCtx->drawRtViews[i].surf)
            {
                GL_ASSERT(chipCtx->drawRtViews[i].firstSlice == 0);

                gcmONERROR(gcChipFboSyncFromMasterSurface(gc, &chipCtx->drawRtViews[i], GL_FALSE));
                gcmONERROR(gcoSURF_DrawBlit(&shadowView, &chipCtx->drawRtViews[i], &blitArgs));
            }
        }

        *mask &= ~GL_COLOR_BUFFER_BIT;

        /* Release shadow texture if needed */
        gcmONERROR(gcChipUtilReleaseShadowTex(gc, &masterView, &shadowView));
    }

    /*
    ** FIXME: Actually 3Dblit can't support depth/stencil blit for now.
    */
    if (*mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    gcmFOOTER();
    return status;
}

/********************************************************************
**
**  _BlitFramebufferSoftware
**
**  Function to use software path to do the framebuffer blit.
**  Only called when all hardware path failed, not implement yet.
**
**  Parameters: The framebuffer blit arguments
**  Return Value: Blit successful
**
********************************************************************/
__GL_INLINE gceSTATUS
gcChipBlitFramebufferSoftware(
    __GLcontext *gc,
    GLint srcX0,
    GLint srcY0,
    GLint srcX1,
    GLint srcY1,
    GLint dstX0,
    GLint dstY0,
    GLint dstX1,
    GLint dstY1,
    GLbitfield *mask,
    GLboolean  xReverse,
    GLboolean  yReverse,
    GLenum     filter
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsSURF_BLIT_ARGS blitArgs;
    gceSTATUS status = gcvSTATUS_OK;
    GLint scissorLeft;
    GLint scissorRight;
    GLint scissorTop;
    GLint scissorBottom;
    GLboolean scissorEnable;
    gcsSURF_VIEW pixelPlaneView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("gc=0x%x srcX0=%d srcY0=%d srcX1=%d srcY1=%d dstX0=%d dstY0=%d "
                  "dstX1=%d desY1=%d mask=0x%x xReverse=%d yReverse=%d filter=0x%04x",
                   gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstX1, dstY1,
                   mask, xReverse, yReverse, filter);

    /* SW blit current does not support linear filter */
    if (GL_LINEAR == filter)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    scissorEnable = gc->state.enables.scissorTest;
    scissorLeft  = gc->state.scissor.scissorX;
    scissorRight = gc->state.scissor.scissorX + gc->state.scissor.scissorWidth;
    scissorTop   = gc->state.scissor.scissorY;
    scissorBottom = gc->state.scissor.scissorY + gc->state.scissor.scissorHeight;

    if (chipCtx->drawYInverted)
    {
        GLint temp = scissorTop;
        scissorTop = (GLint)(chipCtx->drawRTHeight - scissorBottom);
        scissorBottom = (GLint)(chipCtx->drawRTHeight - temp);
    }

    gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
    blitArgs.srcX               = srcX0;
    blitArgs.srcY               = srcY0;
    blitArgs.srcZ               = 0;
    blitArgs.srcWidth           = srcX1 - srcX0;
    blitArgs.srcHeight          = srcY1 - srcY0;
    blitArgs.srcDepth           = 1;
    blitArgs.srcNumSlice        = 1;
    blitArgs.dstX               = dstX0;
    blitArgs.dstY               = dstY0;
    blitArgs.dstZ               = 0;
    blitArgs.dstWidth           = dstX1 - dstX0;
    blitArgs.dstHeight          = dstY1 - dstY0;
    blitArgs.dstDepth           = 1;
    blitArgs.dstNumSlice        = 1;
    blitArgs.xReverse           = xReverse;
    blitArgs.yReverse           = yReverse;
    blitArgs.scissorTest        = scissorEnable;
    blitArgs.scissor.left       = scissorLeft;
    blitArgs.scissor.right      = scissorRight;
    blitArgs.scissor.top        = scissorTop;
    blitArgs.scissor.bottom     = scissorBottom;

    if (*mask & GL_COLOR_BUFFER_BIT)
    {
        GLuint i;

        gcmONERROR(gcChipFboSyncFromMasterSurface(gc, &chipCtx->readRtView, GL_TRUE));
        gcmONERROR(gcChipUtilGetPixelPlane(gc, &chipCtx->readRtView, &pixelPlaneView));

        blitArgs.srcSurface = pixelPlaneView.surf;
        blitArgs.srcZ = pixelPlaneView.firstSlice;

        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            gcsSURF_VIEW *rtView = &chipCtx->drawRtViews[i];
            if (rtView->surf)
            {
                /* Set up the dst surface and rect */
                blitArgs.dstSurface = rtView->surf;
                blitArgs.dstZ = rtView->firstSlice;

                gcmONERROR(gcChipFboSyncFromMasterSurface(gc, rtView, GL_FALSE));
                gcmONERROR(gcoSURF_BlitCPU(&blitArgs));
            }
        }

        *mask &= ~GL_COLOR_BUFFER_BIT;

        /* Destroy pixel plane if needed */
        gcmONERROR(gcChipUtilDestroyPixelPlane(gc, &chipCtx->readRtView, &pixelPlaneView));
    }

    if (*mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
    {
        gcsSURF_VIEW *readDsView = chipCtx->readDepthView.surf ? &chipCtx->readDepthView : &chipCtx->readStencilView;
        gcsSURF_VIEW *drawDsView = chipCtx->drawDepthView.surf ? &chipCtx->drawDepthView : &chipCtx->drawStencilView;

        /* If both read/draw are valid, they must be same. */
        GL_ASSERT(!(chipCtx->readDepthView.surf && chipCtx->readStencilView.surf) ||
                    chipCtx->readDepthView.surf == chipCtx->readStencilView.surf);
        GL_ASSERT(!(chipCtx->drawDepthView.surf && chipCtx->drawStencilView.surf) ||
                    chipCtx->drawDepthView.surf == chipCtx->drawStencilView.surf);

        /* Blt from read depth buffer to draw depth buffer */
        /* Consider stencil and depth sharing same surface */
        if (readDsView->surf && drawDsView->surf)
        {
            gcmONERROR(gcChipFboSyncFromMasterSurface(gc, readDsView, GL_TRUE));
            gcmONERROR(gcChipFboSyncFromMasterSurface(gc, drawDsView, GL_FALSE));

            /* Get Pixel Plane if source is msaa surface */
            gcmONERROR(gcChipUtilGetPixelPlane(gc, readDsView, &pixelPlaneView));

            if (!(*mask & GL_DEPTH_BUFFER_BIT))
            {
                blitArgs.flags = gcvBLIT_FLAG_SKIP_DEPTH_WRITE;
            }
            else if (!(*mask & GL_STENCIL_BUFFER_BIT))
            {
                blitArgs.flags = gcvBLIT_FLAG_SKIP_STENCIL_WRITE;
            }

            blitArgs.srcSurface = pixelPlaneView.surf;
            blitArgs.srcZ = pixelPlaneView.firstSlice;
            blitArgs.dstSurface = drawDsView->surf;
            blitArgs.dstZ = drawDsView->firstSlice;

            gcmONERROR(gcoSURF_BlitCPU(&blitArgs));
            *mask &= ~(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            /* Destroy pixel plane if needed */
            gcmONERROR(gcChipUtilDestroyPixelPlane(gc, readDsView, &pixelPlaneView));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

GLboolean
__glChipBlitFramebufferBegin(
    __GLcontext *gc
    )
{
#if gcdFRAMEINFO_STATISTIC
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcoHAL_FrameInfoOps(chipCtx->hal,
                        gcvFRAMEINFO_DRAW_NUM,
                        gcvFRAMEINFO_OP_INC,
                        gcvNULL);
    if (g_dbgSkipDraw)
    {
        return GL_FALSE;
    }
#endif

    return GL_TRUE;
}

GLboolean
__glChipBlitFramebufferValidateState(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;

    gcmHEADER_ARG("gc=0x%x mask=%d", gc, mask);

    if (fbo && fbo->shadowRender)
    {
        gcmONERROR(gcChipFBOMarkShadowRendered(gc, fbo, mask));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipBlitFramebufferEnd(
    __GLcontext *gc
    )
{
#if gcdFRAMEINFO_STATISTIC || (gcdDUMP && gcdDUMP_VERIFY_PER_DRAW)
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);
#endif

#if gcdFRAMEINFO_STATISTIC
    if (g_dbgPerDrawKickOff)
    {
        /* Flush the cache. */
        gcmONERROR(gcoSURF_Flush(gcvNULL));

        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
    }

    if (g_dbgDumpImagePerDraw & (__GL_PERDRAW_DUMP_BLITFBO_RT | __GL_PERDRAW_DUMP_BLITFBO_DS))
    {
        gcmONERROR(gcChipUtilsDumpRT(gc, (__GL_PERDRAW_DUMP_BLITFBO_RT | __GL_PERDRAW_DUMP_BLITFBO_DS)));
    }
#endif

#if (gcdDUMP && gcdDUMP_VERIFY_PER_DRAW)
    gcmONERROR(gcChipUtilsVerifyRT(gc));
#endif

#if gcdFRAMEINFO_STATISTIC || (gcdDUMP && gcdDUMP_VERIFY_PER_DRAW)

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

#else
     return GL_TRUE;
#endif

}

GLvoid __glChipBlitFramebuffer(__GLcontext *gc,
                               GLint srcX0,
                               GLint srcY0,
                               GLint srcX1,
                               GLint srcY1,
                               GLint dstX0,
                               GLint dstY0,
                               GLint dstX1,
                               GLint dstY1,
                               GLbitfield mask,
                               GLboolean  xReverse,
                               GLboolean  yReverse,
                               GLenum     filter
                               )
{
    gceSTATUS status;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLboolean hasStencilBlit = (mask & GL_STENCIL_BUFFER_BIT) ? GL_TRUE : GL_FALSE;

    gcmHEADER_ARG("gc=0x%x srcX0=%d srcY0=%d srcX1=%d srcY1=%d dstX0=%d dstY0=%d "
                  "dstX1=%d desY1=%d mask=0x%x xReverse=%d yReverse=%d filter=0x%04x",
                   gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                   mask, xReverse, yReverse, filter);

    if (chipCtx->drawYInverted)
    {
        GLint temp = dstY1;
        dstY1 = (GLint)(chipCtx->drawRTHeight - dstY0);
        dstY0 = (GLint)(chipCtx->drawRTHeight - temp);
    }

    if (chipCtx->readYInverted)
    {
        GLint temp = srcY1;
        srcY1 = (GLint)(chipCtx->readRTHeight - srcY0);
        srcY0 = (GLint)(chipCtx->readRTHeight - temp);
    }

    if (chipCtx->drawYInverted ^ chipCtx->readYInverted)
    {
        yReverse = !yReverse;
    }

     /* Try use resolve surface do blit first */
    status = gcChipBlitFramebufferResolve(gc,
                                          srcX0,
                                          srcY0,
                                          srcX1,
                                          srcY1,
                                          dstX0,
                                          dstY0,
                                          dstX1,
                                          dstY1,
                                          &mask,
                                          xReverse,
                                          yReverse,
                                          filter);

    if (mask)
    {
        /* Use 3dblit to do blit */
        status = gcChipBlitFramebuffer3Dblit(gc,
                                             srcX0,
                                             srcY0,
                                             srcX1,
                                             srcY1,
                                             dstX0,
                                             dstY0,
                                             dstX1,
                                             dstY1,
                                             &mask,
                                             xReverse,
                                             yReverse,
                                             filter);
    }

    if (mask)
    {
        /* Roll back to software path to do blit */
        status = gcChipBlitFramebufferSoftware(gc,
                                               srcX0,
                                               srcY0,
                                               srcX1,
                                               srcY1,
                                               dstX0,
                                               dstY0,
                                               dstX1,
                                               dstY1,
                                               &mask,
                                               xReverse,
                                               yReverse,
                                               filter);
    }

    if (chipCtx->needStencilOpt &&
        gcmIS_SUCCESS(status) &&
        hasStencilBlit
       )
    {
        gcsRECT srcRect;
        gcsRECT dstRect;
        GLint scissorLeft;
        GLint scissorRight;
        GLint scissorTop;
        GLint scissorBottom;

        srcRect.left   = srcX0;
        srcRect.right  = srcX1 - 1;
        srcRect.top    = srcY0;
        srcRect.bottom = srcY1 - 1;
        dstRect.left   = dstX0;
        dstRect.right  = dstX1 - 1;
        dstRect.top    = dstY0;
        dstRect.bottom = dstY1 - 1;

        scissorLeft  = gc->state.scissor.scissorX;
        scissorRight = gc->state.scissor.scissorX + gc->state.scissor.scissorWidth;
        scissorTop   = gc->state.scissor.scissorY;
        scissorBottom = gc->state.scissor.scissorY + gc->state.scissor.scissorHeight;

        if (chipCtx->drawYInverted)
        {
            GLint temp = scissorTop;
            scissorTop = (GLint)(chipCtx->drawRTHeight - scissorBottom);
            scissorBottom = (GLint)(chipCtx->drawRTHeight - temp);
        }

        gcChipPatchStencilOptBlit(gc,
                                  &srcRect,
                                  &dstRect,
                                  scissorLeft,
                                  scissorRight,
                                  scissorTop,
                                  scissorBottom,
                                  xReverse,
                                  yReverse);
    }

    gcmFOOTER_NO();
}

GLboolean
__glChipBindDrawFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x prebuf=0x%x curbuf=0x%x", gc, preFBO, curFBO);

#if __GL_CHIP_PATCH_ENABLED
    if (chipCtx->patchInfo.patchFlags.isNavi && (chipCtx->patchInfo.clearCount > 2))
    {
        chipCtx->patchInfo.patchFlags.naviNormal = 0;
    }
#endif

    /* If EGLimage was used for indirect RTT, sync texture surface from corresponding RT surface when unbound.
    ** Ideally, for indirect RTT, texture surface will be synced from RT when next time it is used as texture.
    ** But EGLimage can be shared by different clients, it's not clean to propagate ES3 sync logic to other
    ** clients, like VG. Unbound time sync can make ES3 driver as good as legacy ES2 driver, it can pass all
    ** known tests. In fact, there is still hole when other client uses EGLimage w/o unbinding from ES3 client.
    ** For later chips support direct RTT, no need to sync any more.
    */
    gcmONERROR(gcChipFboSyncFromShadow(gc, preFBO));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipBindReadFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    )
{

}

GLvoid
__glChipDeleteRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    gcmHEADER_ARG("gc=0x%x rbo=0x%x", gc, rbo);
    if (rbo->privateData)
    {
        __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;

        if (chipRBO->stencilOpt)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)chipRBO->stencilOpt));
            chipRBO->stencilOpt = gcvNULL;
        }

        if (chipRBO->surface)
        {
            gcmVERIFY_OK(gcoSURF_Destroy(chipRBO->surface));
            chipRBO->surface = gcvNULL;
        }

        if (chipRBO->shadow.surface)
        {
            gcmVERIFY_OK(gcoSURF_Destroy(chipRBO->shadow.surface));
            chipRBO->shadow.surface  = gcvNULL;
        }

        (gc->imports.free)(NULL, chipRBO);
        rbo->privateData = NULL;
    }
    gcmFOOTER_NO();
}

/*
** Try to detach rbo chip/HAL object from this context.
** NOTE, this rbo probably still exist in other contexts.
*/
GLvoid
__glChipDetachRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcoSURF surfList[2] = {gcvNULL, gcvNULL};
    GLuint surfCount = 0;
    gcmHEADER_ARG("gc=0x%x rbo=0x%x", gc, rbo);

    if (rbo->privateData)
    {
        __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;
        if (chipRBO->surface)
        {
            surfList[surfCount++] = chipRBO->surface;
        }

        if (chipRBO->shadow.surface)
        {
            surfList[surfCount++] = chipRBO->shadow.surface;
        }

        GL_ASSERT(surfCount <= 2);

        if (surfCount)
        {
            gcChipDetachSurface(gc, chipCtx, surfList, surfCount);
        }
    }
    gcmFOOTER_NO();
    return;
}


GLboolean
__glChipRenderbufferStorage(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_TYPE surfType;
    __GLchipFmtMapInfo *formatMapInfo;
    __GLchipFmtPatch patchCase =  __GL_CHIP_FMT_PATCH_NONE;
    __GLformat drvFormat = rbo->formatInfo->drvFormat;

    gcmHEADER_ARG("gc=0x%x rbo=0x%x", gc, rbo);

    GL_ASSERT(chipRBO);

    if (chipRBO == gcvNULL)
    {
        return GL_FALSE;
    }

    /* Destroy previous resource if exist */
    if (chipRBO->surface)
    {
        gcmONERROR(gcoSURF_Destroy(chipRBO->surface));
        chipRBO->surface = gcvNULL;
    }

    /*
    ** - MSAA rendering formats that HW can't support directly as it always need be compressed.
    */
    if (rbo->samples > 0)
    {
        switch (drvFormat)
        {
        case __GL_FMT_R8:
        case __GL_FMT_RG8:
        case __GL_FMT_RGB10_A2:
            drvFormat = __GL_FMT_RGBA8;
            break;

        /* Disable MSAA for SRGBA rendering, as floating rendering can NOT support MSAA for now.
        */
        case __GL_FMT_SRGB8_ALPHA8:
            rbo->samples = 0;
            break;
        default:
            break;
        }

    }

    if (drvFormat == __GL_FMT_RGB565 &&
        (gcdPROC_IS_WEBGL(chipCtx->patchId)))
    {
        drvFormat = __GL_FMT_RGBX8;
    }

    if (drvFormat == __GL_FMT_RGBA4 &&
        (chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30) &&
        ! gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI5) &&
        rbo->width <= 0x80 && rbo->height <= 0x80)
    {
        drvFormat = __GL_FMT_RGBA8;
    }

    if ((rbo->formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8) &&
        (chipCtx->patchId == gcvPATCH_GTFES30) &&
        ((rbo->width == 0x1f4 && rbo->height == 0x1f4) ||
         (rbo->width == 2 && rbo->height == 2) ||
         (rbo->width == 128 && rbo->height == 64)
        )
       )
    {
        patchCase = __GL_CHIP_FMT_PATCH_CASE1;
    }
    else if ((__GL_API_VERSION_ES20 != gc->apiVersion) &&
             (__GL_FMT_Z16 == drvFormat) &&
             (chipCtx->chipFeature.hwFeature.hasCompressionV1)
            )
    {
        patchCase = __GL_CHIP_FMT_PATCH_CASE2;
    }
    else if (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_6 &&
             rbo->samples > 0 &&
             rbo->formatInfo->bitsPerPixel == 8)
    {
        patchCase = __GL_CHIP_FMT_PATCH_8BIT_MSAA;
    }

    formatMapInfo = gcChipGetFormatMapInfo(gc, drvFormat, patchCase);

    chipRBO->formatMapInfo = formatMapInfo;

    if (rbo->samples > 0)
    {
        GLint i;
        for (i = 0; i < formatMapInfo->numSamples; i++)
        {
            if (formatMapInfo->samples[i] >= rbo->samples)
            {
                break;
            }
        }

        rbo->samplesUsed = formatMapInfo->samples[i];
    }
    else
    {
        rbo->samplesUsed = rbo->samples;
    }

    GL_ASSERT((formatMapInfo->flags & __GL_CHIP_FMTFLAGS_CANT_FOUND_HAL_FORMAT) == GL_FALSE);
    GL_ASSERT(formatMapInfo->writeFormat != gcvSURF_UNKNOWN);

    switch (rbo->formatInfo->baseFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_STENCIL:
    case GL_STENCIL:
        surfType = gcvSURF_DEPTH;
        break;
    default:
        surfType = gcvSURF_RENDER_TARGET;
        break;
    }

    chipCtx->needRTRecompile = chipCtx->needRTRecompile
                             || gcChipCheckRecompileEnable(gc, formatMapInfo->writeFormat);

    /* Create render target. */
    gcmONERROR(gcoSURF_Construct(chipCtx->hal,
                                 rbo->width,
                                 rbo->height,
                                 1,
                                 surfType,
                                 formatMapInfo->writeFormat,
                                 gcvPOOL_DEFAULT,
                                 &chipRBO->surface));

    gcmONERROR(gcoSURF_SetSamples(chipRBO->surface, rbo->samplesUsed));

    if (chipCtx->needStencilOpt)
    {
        if (rbo->formatInfo->stencilSize > 0)
        {
            if (!chipRBO->stencilOpt)
            {
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          gcmSIZEOF(__GLchipStencilOpt),
                                          (gctPOINTER*)&chipRBO->stencilOpt));
            }

            gcChipPatchStencilOptReset(chipRBO->stencilOpt,
                                       (gctSIZE_T)rbo->width,
                                       (gctSIZE_T)rbo->height,
                                       (gctSIZE_T)rbo->formatInfo->stencilSize);
        }
        else
        {
            if (chipRBO->stencilOpt)
            {
                gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)chipRBO->stencilOpt));
                chipRBO->stencilOpt = gcvNULL;
            }
        }
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipCleanTextureShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    GLint level, slice;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    for (level = 0; level < (GLint)texObj->maxLevels; ++level)
    {
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
        GLint numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX)
                        ? texObj->faceMipmap[0][level].depth
                        : texObj->arrays;

        for (slice = 0; slice < numSlices; ++slice)
        {
            __GLchipResourceShadow *shadow = &chipMipLevel->shadow[slice];

            /* Only indirect RTT would mark create the surface and mark it dirty */
            if (shadow->surface)
            {
                if (shadow->shadowDirty)
                {
                    GLint face  = (texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX) ? slice : 0;
                    GLint depth = (texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX) ? 0 : slice;
                    gcChipTexMipSliceSyncFromShadow(gc, texObj, face, level, depth);
                }
                gcmONERROR(gcoSURF_Destroy(shadow->surface));
                shadow->surface = gcvNULL;
            }
        }
    }

OnError:
    gcmFOOTER();
}

GLvoid
__glChipCleanRenderbufferShadow(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;
    __GLchipResourceShadow *shadow = &chipRBO->shadow;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x rbo=0x%x", gc, rbo);

    if (shadow->surface)
    {
        if (shadow->shadowDirty)
        {
            gcsSURF_VIEW shadowView = {shadow->surface,  0, 1};
            gcsSURF_VIEW rboView    = {chipRBO->surface, 0, 1};

            gcmONERROR(gcoSURF_ResolveRect(&shadowView, &rboView, gcvNULL));
            gcmONERROR(gcChipSetImageSrc(rbo->eglImage, chipRBO->surface));
            shadow->shadowDirty = GL_FALSE;

            /* Commit commands. */
            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        gcmONERROR(gcoSURF_Destroy(shadow->surface));
        shadow->surface = gcvNULL;
    }

OnError:
    gcmFOOTER();
}

static gceSTATUS
gcChipGetRenderBufferAttribFromImage(
    khrEGL_IMAGE_PTR image,
    gctINT  *stride,
    gctUINT *width,
    gctUINT *height,
    gceSURF_FORMAT *format,
    gcoSURF *surface,
    gctPOINTER *address
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("image=0x%x width=0x%x height=0x%x format=0x%x surface=0x%x address=0x%x",
                   image, width, height, format, surface, address);

    switch (image->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
    case KHR_IMAGE_RENDER_BUFFER:
    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
    case KHR_IMAGE_WAYLAND_BUFFER:
    case KHR_IMAGE_PIXMAP:
    case KHR_IMAGE_LINUX_DMA_BUF:
        *surface = image->surface;

        gcmERR_BREAK(
            gcoSURF_GetSize(image->surface,
                            width,
                            height,
                            gcvNULL));

        gcmERR_BREAK(
            gcoSURF_GetFormat(image->surface,
                              gcvNULL,
                              format));

        gcmERR_BREAK(
            gcoSURF_GetAlignedSize(image->surface,
                                   gcvNULL, gcvNULL,
                                   stride));

        *address = gcvNULL;
        break;

    default:
        status = gcvSTATUS_INVALID_ARGUMENT;
        break;
    }

    gcmFOOTER();
    return status;
}

GLenum
__glChipCreateEglImageRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo, GLvoid *image
    )
{
    __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;
    gcoSURF surface = gcvNULL;
    khrEGL_IMAGE * eglImage = gcvNULL;
    gctINT32 referenceCount = 0;
    GLenum ret;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSURF_FORMAT format;
    gcmHEADER_ARG("gc=0x%x rbo=0x%x image=0x%x", gc, rbo, image);


    if (chipRBO == gcvNULL)
    {
        ret = EGL_BAD_PARAMETER;
        gcmFOOTER_ARG("return=0x%04x", ret);
        return ret;
    }

    /* Get render buffer surface. */
    surface = chipRBO->surface;
    if (!surface)
    {
        ret = EGL_BAD_ACCESS;
        gcmFOOTER_ARG("return=0x%04x", ret);
        return ret;
    }

    /* Get source surface reference count. */
    gcoSURF_QueryReferenceCount(surface, &referenceCount);

    /* Test if surface is a sibling of any eglImage. */
    if (referenceCount > 1)
    {
        ret = EGL_BAD_ACCESS;
        gcmFOOTER_ARG("return=0x%04x", ret);
        return ret;
    }

    eglImage = (khrEGL_IMAGE*)image;

    /* Set EGL Image info. */
    eglImage->magic   = KHR_EGL_IMAGE_MAGIC_NUM;
    eglImage->type    = KHR_IMAGE_RENDER_BUFFER;
    eglImage->surface = surface;
    eglImage->u.texture.internalFormat = rbo->internalFormat;
    eglImage->u.texture.format = rbo->internalFormat;
    eglImage->u.texture.width = rbo->width;
    eglImage->u.texture.height = rbo->height;
    eglImage->u.texture.level  = 0;
    /* Type is not yet set. Default to unknown/invalid. */
    eglImage->u.texture.type = 0xFFFFFFFFu;

    gcoSURF_GetFormat(surface, gcvNULL, &format);

    chipCtx->needRTRecompile = chipCtx->needRTRecompile || gcChipCheckRecompileEnable(gc, format);

    ret = EGL_SUCCESS;
    gcmFOOTER_ARG("return=0x%04x", ret);
    return ret;
}


GLboolean
__glChipEglImageTargetRenderbufferStorageOES(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo,
    GLenum target,
    GLvoid *eglImage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)rbo->privateData;
    gctINT stride;
    gctUINT width, height;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceSURF_TYPE type = gcvSURF_TYPE_UNKNOWN;
    gctPOINTER address = gcvNULL;
    gcoSURF surface = gcvNULL;
    gceSTATUS status;

    khrEGL_IMAGE *image = (khrEGL_IMAGE_PTR) eglImage;

    gcmHEADER_ARG("gc=0x%x rbo=0x%x target=0x%x eglImage=0x%x", gc, rbo, target, eglImage);

    GL_ASSERT(chipRBO);

    gcmONERROR(gcChipGetRenderBufferAttribFromImage(image,
                                                    &stride,
                                                    &width,
                                                    &height,
                                                    &format,
                                                    &surface,
                                                    &address));

    switch (format)
    {
    case gcvSURF_R5G6B5:
    case gcvSURF_B8G8R8:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_X1R5G5B5:
    case gcvSURF_R4G4B4A4:
    case gcvSURF_X4R4G4B4:
    case gcvSURF_A16B16G16R16F:
    case gcvSURF_A2B10G10R10:
        type = gcvSURF_RENDER_TARGET;
        break;

    case gcvSURF_D16:
    case gcvSURF_D24X8:
    case gcvSURF_D24S8:
    case gcvSURF_X24S8:
    case gcvSURF_S8:
        type = gcvSURF_DEPTH;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (surface)
    {
        gceSURF_FORMAT imgFormat;
        gctUINT imgSamples = 1;
        gctUINT tgtSamples = (rbo->samplesUsed == 0) ? 1 : rbo->samplesUsed;
        __GLchipFmtPatch patchCase = (image->type == KHR_IMAGE_TEXTURE_3D) ? __GL_CHIP_FMT_PATCH_CASE0 :
                                                                             __GL_CHIP_FMT_PATCH_NONE;

        chipRBO->formatMapInfo = gcChipGetFormatMapInfo(gc, rbo->formatInfo->drvFormat, patchCase);

        if (chipRBO->surface != gcvNULL)
        {
            gcmONERROR(gcoSURF_Destroy(chipRBO->surface));
            chipRBO->surface = gcvNULL;
        }

        if (image->type == KHR_IMAGE_TEXTURE_CUBE && (image->u.texture.face > 0 ))
        {
            if ((image->u.texture.shadowSurface != gcvNULL) && (image->u.texture.masterDirty == gcvTRUE))
            {
                gcsSURF_VIEW imageView  = {image->surface, image->u.texture.sliceIndex, 1};
                gcsSURF_VIEW shadowView = {image->u.texture.shadowSurface, 0, 1};

                gcmONERROR(gcoSURF_ResolveRect(&imageView, &shadowView, gcvNULL));
                image->u.texture.masterDirty = gcvFALSE;
            }
            chipRBO->surface = image->u.texture.shadowSurface;
            gcmONERROR(gcoSURF_ReferenceSurface(image->u.texture.shadowSurface));
        }
        else
        {
            chipRBO->surface = surface;
            gcmONERROR(gcoSURF_ReferenceSurface(surface));
        }

        gcmONERROR(gcoSURF_GetFormat(chipRBO->surface, gcvNULL, &imgFormat));
        gcmONERROR(gcoSURF_GetSamples(chipRBO->surface, &imgSamples));

        /* Check if we need shadow surface: has nothing to do with its readFormat */
        if (imgFormat != chipRBO->formatMapInfo->writeFormat ||
            imgSamples != tgtSamples ||
            gcoSURF_IsRenderable(chipRBO->surface) != gcvSTATUS_OK)
        {
            gcmONERROR(gcChipRellocShadowResource(gc,
                                                  surface,
                                                  tgtSamples,
                                                  &chipRBO->shadow,
                                                  chipRBO->formatMapInfo,
                                                  GL_FALSE));
            chipRBO->shadow.masterDirty = GL_TRUE;
        }
    }
    else
    {
        gcmONERROR(gcoSURF_Construct(chipCtx->hal,
                                     width, height, 1,
                                     type, format,
                                     gcvPOOL_USER,
                                     &chipRBO->surface));

        /* If user specified stride(alignment in fact), it must be no less than calculated one. */
        GL_ASSERT((gctUINT)stride >= chipRBO->surface->stride);
        gcmONERROR(gcoSURF_MapUserSurface(chipRBO->surface,
                                          (gctUINT)stride,
                                          address,
                                          gcvINVALID_ADDRESS));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipBindRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *renderbuf
    )
{
    __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)renderbuf->privateData;

    gcmHEADER_ARG("gc=0x%x renderbuf=0x%x", gc, renderbuf);
    if (chipRBO == gcvNULL)
    {
        chipRBO = (__GLchipRenderbufferObject*)gc->imports.calloc(gc, 1, sizeof(__GLchipRenderbufferObject));
        renderbuf->privateData = chipRBO;
    }
    gcmFOOTER_NO();
}


gceSTATUS
gcChipRellocShadowResource(
    __GLcontext *gc,
    gcoSURF master,
    GLuint samples,
    __GLchipResourceShadow *shadow,
    __GLchipFmtMapInfo *formatMapInfo,
    GLboolean isFromTexture
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLuint targetW, targetH, shadowW, shadowH;
    GLuint shadowSamples, targetSamples;
    gceSURF_FORMAT targetFormat, shadowFormat;
    GLboolean needAlloc = GL_TRUE;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32  masterSamples;

    gcmHEADER_ARG("gc=0x%x master=0x%x samples=%d shadow=0x%x formatMapInfo=0x%x",
                   gc, master, samples, shadow, formatMapInfo);

    GL_ASSERT(shadow);

    if (master == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gcoSURF_GetSamples(master, &masterSamples));

    targetSamples = (masterSamples > 1) ? masterSamples :
                                          ((samples == 0) ? 1 : samples);

    /* Collect master information */
    gcmONERROR(gcoSURF_GetSize(master, &targetW, &targetH, gcvNULL));
    targetFormat = formatMapInfo->writeFormat;

    /* Collect current shadow information */
    if (shadow->surface)
    {
        gcmONERROR(gcoSURF_GetSize(shadow->surface, &shadowW, &shadowH, gcvNULL));
        gcmONERROR(gcoSURF_GetFormat(shadow->surface, gcvNULL, &shadowFormat));
        gcmONERROR(gcoSURF_GetSamples(shadow->surface, &shadowSamples));

        if ((shadowW == targetW) &&
            (shadowH == targetH) &&
            (shadowFormat == targetFormat) &&
            (shadowSamples == targetSamples))
        {
            needAlloc = GL_FALSE;
        }
    }

    if (needAlloc)
    {
        gceSURF_TYPE surfType = gcvSURF_TYPE_UNKNOWN;

        if (master->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH)
        {
            if (shadow->masterDirty)
            {
                surfType = gcvSURF_DEPTH_TS_DIRTY;
            }
            else
            {
                surfType = gcvSURF_DEPTH;
            }
        }
        else
        {
            if (shadow->masterDirty)
            {
                surfType = gcvSURF_RENDER_TARGET_TS_DIRTY;
            }
            else
            {
                surfType = gcvSURF_RENDER_TARGET;
            }
        }

        if (shadow->surface)
        {
            gcmONERROR(gcoSURF_Destroy(shadow->surface));
            shadow->surface = gcvNULL;
        }

        /* nv12 output for AXIS. It needs linear output. */
        if (chipCtx->chipModel == gcv1000 &&
            chipCtx->chipRevision >= 0x5039 &&
            targetFormat == gcvSURF_G8R8_1_X8R8G8B8)
        {
            surfType |= gcvSURF_LINEAR;
        }

        if (gcoSURF_QueryHints(master, gcvSURF_PROTECTED_CONTENT))
        {
            surfType |= gcvSURF_PROTECTED_CONTENT;
        }

        if (isFromTexture && targetFormat == gcvSURF_D16 &&
            chipCtx->chipFeature.hwFeature.hasCompressionV1)
        {
            surfType |= gcvSURF_NO_TILE_STATUS;
        }

        if (targetSamples > 1 &&
            !chipCtx->chipFeature.hwFeature.hasRSBLTMsaaDecompression)
        {
            surfType |= gcvSURF_NO_COMPRESSION;
        }

        chipCtx->needRTRecompile = chipCtx->needRTRecompile
                                 || gcChipCheckRecompileEnable(gc, targetFormat);

        chipCtx->needTexRecompile = chipCtx->needTexRecompile
                                  || gcChipCheckRecompileEnable(gc, targetFormat);


        gcmONERROR(gcoSURF_Construct(chipCtx->hal, targetW, targetH, 1, surfType,
                                     targetFormat, gcvPOOL_DEFAULT, &shadow->surface));
        gcmONERROR(gcoSURF_SetSamples(shadow->surface, targetSamples));
    }


OnError:
    gcmFOOTER();
    return status;
}

/*
** Every time a texture/rbo was drawn or cleared,
** Sync shadow from texture master only, then mark shadow surface dirtied.
*/
gceSTATUS
gcChipFBOMarkShadowRendered(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLbitfield mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsSURF_VIEW texView    = {gcvNULL, 0, 1};
    gcsSURF_VIEW rboView    = {gcvNULL, 0, 1};
    gcsSURF_VIEW shadowView = {gcvNULL, 0, 1};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x fbo=0x%x mask=%d", gc, fbo, mask);

    GL_ASSERT(fbo);

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        GLuint rtIdx;

        /* Loop through all draw buffer, if it was from texture */
        for (rtIdx = 0; rtIdx < __GL_MAX_COLOR_ATTACHMENTS; ++rtIdx)
        {
            GLint attachIdx;
            __GLfboAttachPoint *attachPoint;

            if (!chipCtx->drawRtViews[rtIdx].surf)
            {
                continue;
            }

            /* If all RGBA write mask are FALSE, the RT will not really be written */
            if (!gc->state.raster.colorMask[rtIdx].redMask && !gc->state.raster.colorMask[rtIdx].greenMask &&
                !gc->state.raster.colorMask[rtIdx].blueMask && !gc->state.raster.colorMask[rtIdx].alphaMask)
            {
                continue;
            }

            attachIdx = __glMapAttachmentToIndex(fbo->drawBuffers[rtIdx]);
            GL_ASSERT(attachIdx < __GL_MAX_COLOR_ATTACHMENTS);
            /* This draw buffer was not selected */
            if (-1 == attachIdx)
            {
                continue;
            }

            attachPoint = &fbo->attachPoint[attachIdx];
            if (attachPoint->objType == GL_TEXTURE)
            {
                __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
                __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);
                __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
                __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

                if (shadow->surface == chipCtx->drawRtViews[rtIdx].surf)
                {
                    /* If the texSurface was uploaded before, sync it to rtSurface before draw/clear RT */
                    if (shadow->masterDirty)
                    {
                        gcmONERROR(gcoSURF_DisableTileStatus(&chipCtx->drawRtViews[rtIdx], gcvTRUE));
                        texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, attachPoint->level, attachPoint->slice);
                        if (texView.surf)
                        {
                            shadowView.surf = shadow->surface;
                            gcmONERROR(gcoSURF_ResolveRect(&texView, &shadowView, gcvNULL));
                            shadow->masterDirty = GL_FALSE;
                        }
                        else
                        {
                            gcmONERROR(gcvSTATUS_INVALID_OBJECT);
                        }
                    }
                    gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, shadow->surface));
                    shadow->shadowDirty = GL_TRUE;
                    texInfo->rendered  = GL_TRUE;
                }
            }
            else if (attachPoint->objType == GL_RENDERBUFFER)
            {
                __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
                __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
                 __GLchipResourceShadow *shadow = &chipRBO->shadow;
                if (shadow->masterDirty)
                {
                    shadowView.surf = shadow->surface;
                    rboView.surf = chipRBO->surface;
                    gcmONERROR(gcoSURF_ResolveRect(&rboView, &shadowView, gcvNULL));
                    shadow->masterDirty = GL_FALSE;
                }
                gcmONERROR(gcChipSetImageSrc(rbo->eglImage, shadow->surface));
                chipRBO->shadow.shadowDirty = GL_TRUE;
            }
        }
    }

    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        __GLfboAttachPoint *attachPoint = &fbo->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];

        if (attachPoint->objType == GL_TEXTURE && chipCtx->drawDepthView.surf && gc->state.depth.writeEnable)
        {
            __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);
            __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
            __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

            if (shadow->surface == chipCtx->drawDepthView.surf)
            {
                /* If the texSurface was uploaded before, sync it to rtSurface before draw/clear RT */
                if (shadow->masterDirty)
                {
                    gcmONERROR(gcoSURF_DisableTileStatus(&chipCtx->drawDepthView, gcvTRUE));
                    texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, attachPoint->level, attachPoint->slice);
                    if (texView.surf)
                    {
                        shadowView.surf = shadow->surface;
                        gcmONERROR(gcoSURF_ResolveRect(&texView, &shadowView, gcvNULL));
                        shadow->masterDirty = GL_FALSE;
                    }
                    else
                    {
                        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
                    }
                }
                shadow->shadowDirty = GL_TRUE;
                gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, shadow->surface));
                texInfo->rendered  = GL_TRUE;
            }
        }
        else if (attachPoint->objType == GL_RENDERBUFFER && chipCtx->drawDepthView.surf && gc->state.depth.writeEnable)
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
            __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
             __GLchipResourceShadow *shadow = &chipRBO->shadow;
            if (shadow->masterDirty)
            {
                shadowView.surf = shadow->surface;
                rboView.surf = chipRBO->surface;
                gcmONERROR(gcoSURF_ResolveRect(&rboView, &shadowView, gcvNULL));
                shadow->masterDirty = GL_FALSE;
            }
            gcmONERROR(gcChipSetImageSrc(rbo->eglImage, shadow->surface));
            chipRBO->shadow.shadowDirty = GL_TRUE;

        }
    }

    if (mask & GL_STENCIL_BUFFER_BIT)
    {
        __GLfboAttachPoint *attachPoint = &fbo->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];

        if (attachPoint->objType == GL_TEXTURE && chipCtx->drawStencilView.surf &&
            (gc->state.stencil.front.writeMask || gc->state.stencil.back.writeMask))
        {
            __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);
            __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
            __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

            if (shadow->surface == chipCtx->drawStencilView.surf)
            {
                /* If the texSurface was uploaded before, sync it to rtSurface before draw/clear RT */
                if (shadow->masterDirty)
                {
                    gcmONERROR(gcoSURF_DisableTileStatus(&chipCtx->drawStencilView, gcvTRUE));
                    texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, attachPoint->level, attachPoint->slice);
                    if (texView.surf)
                    {
                        shadowView.surf = shadow->surface;
                        gcmONERROR(gcoSURF_ResolveRect(&texView, &shadowView, gcvNULL));
                        shadow->masterDirty = GL_FALSE;
                    }
                    else
                    {
                        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
                    }
                }
                gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, shadow->surface));
                shadow->shadowDirty = GL_TRUE;
                texInfo->rendered  = GL_TRUE;
            }
        }
        else if (attachPoint->objType == GL_RENDERBUFFER && chipCtx->drawStencilView.surf &&
                 (gc->state.stencil.front.writeMask || gc->state.stencil.back.writeMask))
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)attachPoint->object;
            __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
            __GLchipResourceShadow *shadow = &chipRBO->shadow;
            if (shadow->masterDirty)
            {
                shadowView.surf = shadow->surface;
                rboView.surf = chipRBO->surface;
                gcmONERROR(gcoSURF_ResolveRect(&rboView, &shadowView, gcvNULL));
                shadow->masterDirty = GL_FALSE;
            }
            gcmONERROR(gcChipSetImageSrc(rbo->eglImage, shadow->surface));
            chipRBO->shadow.shadowDirty = GL_TRUE;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipTexMipSliceSyncFromShadow(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint depth
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
    GLint slice = face > 0 ? face : depth;
    __GLchipResourceShadow *shadow = &chipMipLevel->shadow[slice];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d depth=%d", gc, texObj, face, level, depth);

    /* Only indirect RTT would mark create the surface and mark it dirty */
    if (shadow->surface)
    {
        if (shadow->shadowDirty)
        {
            gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, level, slice);
            if (texView.surf)
            {
                gcsSURF_VIEW shadowView = {shadow->surface, 0 ,1};

                gcmONERROR(gcoSURF_ResolveRect(&shadowView, &texView, gcvNULL));
                gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, texView.surf));
                shadow->shadowDirty = GL_FALSE;

                /* Commit commands. */
                gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));

                /* Get fence for master surface if needed. */
                if(!(chipCtx->chipFeature.hwFeature.hasBlitEngine))
                {
                    gcmONERROR(gcoSURF_GetFence(texView.surf, gcvFENCE_TYPE_WRITE));
                }
            }
            else
            {
                gcmONERROR(gcvSTATUS_INVALID_OBJECT);
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipRboSyncFromShadow(
    __GLcontext* gc,
    __GLrenderbufferObject *rbo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipRenderbufferObject *chipRBO = NULL;
    __GLchipResourceShadow *shadow = NULL;

    gcmHEADER_ARG("gc=0x%x rbo=0x%x ", gc, rbo);

    chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
    GL_ASSERT(chipRBO);
    shadow = &chipRBO->shadow;

    if (shadow->surface && shadow->shadowDirty)
    {
        gcsSURF_VIEW shadowView = {shadow->surface,  0, 1};
        gcsSURF_VIEW rboView    = {chipRBO->surface, 0, 1};

        gcmONERROR(gcoSURF_ResolveRect(&shadowView, &rboView, gcvNULL));
        gcmONERROR(gcChipSetImageSrc(rbo->eglImage, rboView.surf));
        shadow->shadowDirty = GL_FALSE;

        /* Commit commands. */
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipTexDirectSourceSyncFromMipSlice(
    __GLcontext* gc,
    __GLtextureObject *texObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_VIEW texView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("gc=0x%x texObj=0x%x ", gc, texObj);

    /* Only Fbo been touched */
    texView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, 0, 0);
    if (texView.surf)
    {
        gcsSURF_VIEW directView = {texInfo->direct.source, 0, 1};

        gcmONERROR(gcoSURF_ResolveRect(&texView, &directView, gcvNULL));

        /* Commit commands. */
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));
    }
    else
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

OnError:
    gcmFOOTER();
    return status;
}

/*
** Every time a draw would sample a texture, it need to sync from the RT first
*/
gceSTATUS
gcChipTexSyncFromShadow(
    __GLcontext *gc,
    GLuint unit,
    __GLtextureObject *texObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    GLint level, slice;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x unit=%u texObj=0x%x", gc, unit, texObj);

    if (!texInfo->rendered)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    for (level = texObj->params.baseLevel; level <= (GLint)gc->texture.units[unit].maxLevelUsed; ++level)
    {
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
        GLint numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX)
                        ? texObj->faceMipmap[0][level].depth
                        : texObj->arrays;

        for (slice = 0; slice < numSlices; ++slice)
        {
            __GLchipResourceShadow *shadow = &chipMipLevel->shadow[slice];

            /* Only indirect RTT would mark create the surface and mark it dirty */
            if (shadow->surface)
            {
                if (shadow->shadowDirty)
                {
                    gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, level, slice);
                    if (texView.surf)
                    {
                        gcsSURF_VIEW shadowView = {shadow->surface, 0, 1};

                        gcmONERROR(gcoSURF_ResolveRect(&shadowView, &texView, gcvNULL));

                        gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, texView.surf));
                        shadow->shadowDirty = GL_FALSE;
                    }
                    else
                    {
                        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
                    }
                }
            }
        }
    }

    texInfo->rendered = GL_FALSE;

OnError:
    gcmFOOTER();
    return status;
}

/*
** Sync shadow from texture master.
** We don't need to consider sync shadow from RBO master, as there is no case the RBO master
** surface will be dirty for ES3.0 API (no drawpixels path).
*/
gceSTATUS
gcChipFboSyncFromMasterSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    GLboolean read
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject *fbo = read ? gc->frameBuffer.readFramebufObj : gc->frameBuffer.drawFramebufObj;
    __GLfboAttachPoint *attachPoint = NULL;
    GLint attachIdx = -1;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x surfView=0x%x read=%d", gc, surfView, read);

    /* If surface is NULL or no FBO was used */
    if (!surfView->surf || fbo->name == 0)
    {
        goto OnExit;
    }

    /* Get attachIdx for read case and draw case */
    if (read)
    {
        if (gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readRtView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __glMapAttachmentToIndex(fbo->readBuffer);
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readDepthView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readStencilView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }
    else
    {
        GLuint rtIdx;

        for (rtIdx = 0; rtIdx < gc->constants.shaderCaps.maxDrawBuffers; ++rtIdx)
        {
            if (gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawRtViews[rtIdx], surfView, sizeof(gcsSURF_VIEW))))
            {
                /* It is a draw surface */
                attachIdx = __glMapAttachmentToIndex(fbo->drawBuffers[rtIdx]);
                break;
            }
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawDepthView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawStencilView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }

    /* If the surface was not any of the current draw or read one */
    if (-1 == attachIdx)
    {
        goto OnExit;
    }

    attachPoint = &fbo->attachPoint[attachIdx];

    if (GL_TEXTURE == attachPoint->objType)
    {
        __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
        __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);
        gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, attachPoint->level, attachPoint->slice);
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
        __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

        if (texView.surf && shadow->surface && shadow->masterDirty)
        {
            GL_ASSERT(shadow->surface == surfView->surf && 0 == surfView->firstSlice);
            GL_ASSERT((shadow->surface->sampleInfo.x <= texView.surf->sampleInfo.x) &&
                      (shadow->surface->sampleInfo.y <= texView.surf->sampleInfo.y));

            gcmONERROR(gcoSURF_DisableTileStatus(surfView, gcvTRUE));
            gcmONERROR(gcoSURF_ResolveRect(&texView, surfView, gcvNULL));

            shadow->masterDirty= GL_FALSE;
        }
    }

OnExit:
OnError:
    gcmFOOTER();
    return status;
}

/*
** If 'surface' is a shadow surface, we will sync it to its master surface
** And return its master surface for future use.
** Currently we assume 'surface' is always the current read buffer
*/
gcsSURF_VIEW
gcChipFboSyncFromShadowSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    GLboolean read
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject *fbo = read ? gc->frameBuffer.readFramebufObj : gc->frameBuffer.drawFramebufObj;
    __GLfboAttachPoint *attachPoint = NULL;
    GLint attachIdx = -1;
    gcsSURF_VIEW masterView = {gcvNULL, 0, 1};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x surfView=0x%x read=%d", gc, surfView, read);

    if (!surfView)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* If surface is NULL or no FBO was used */
    if (!surfView->surf || fbo->name == 0)
    {
        masterView = *surfView;
        goto OnError;
    }

    /* Get attachIdx for read case and draw case */
    if (read)
    {
        if (gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readRtView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __glMapAttachmentToIndex(fbo->readBuffer);
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readDepthView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->readStencilView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }
    else
    {
        GLuint rtIdx;

        for (rtIdx = 0; rtIdx < gc->constants.shaderCaps.maxDrawBuffers; ++rtIdx)
        {
            if (gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawRtViews[rtIdx], surfView, sizeof(gcsSURF_VIEW))))
            {
                /* It is a draw surface */
                attachIdx = __glMapAttachmentToIndex(fbo->drawBuffers[rtIdx]);
                break;
            }
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawDepthView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && gcmIS_SUCCESS(gcoOS_MemCmp(&chipCtx->drawStencilView, surfView, sizeof(gcsSURF_VIEW))))
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }

    /* If the surface was not any of the current draw or read one */
    if (-1 == attachIdx)
    {
        masterView = *surfView;
        goto OnError;
    }

    attachPoint = &fbo->attachPoint[attachIdx];

    if (GL_TEXTURE == attachPoint->objType)
    {
        __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
        __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);
        gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, attachPoint->level, attachPoint->slice);
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[attachPoint->level];
        __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

        if (texView.surf && shadow->surface && shadow->shadowDirty)
        {
            GL_ASSERT(shadow->surface == surfView->surf && 0 == surfView->firstSlice);

            gcmONERROR(gcoSURF_ResolveRect(surfView, &texView, gcvNULL));
            gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, texView.surf));

            shadow->shadowDirty = GL_FALSE;
        }
        masterView = texView;
    }
    else if (GL_RENDERBUFFER == attachPoint->objType)
    {
        __GLrenderbufferObject *rbo = NULL;
        __GLchipRenderbufferObject *chipRBO = NULL;
        __GLchipResourceShadow *shadow;
        gcsSURF_VIEW rboView = {gcvNULL, 0, 1};

        rbo = (__GLrenderbufferObject*)attachPoint->object;
        GL_ASSERT(rbo);

        chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);
        GL_ASSERT(chipRBO);

        shadow = &chipRBO->shadow;

        if (shadow->surface && shadow->shadowDirty)
        {
            GL_ASSERT(chipRBO->surface == surfView->surf && 0 == surfView->firstSlice);

            rboView.surf = chipRBO->surface;
            gcmONERROR(gcoSURF_ResolveRect(surfView, &rboView, gcvNULL));

            gcmONERROR(gcChipSetImageSrc(rbo->eglImage, chipRBO->surface));

            shadow->shadowDirty = GL_FALSE;
        }

        masterView.surf = chipRBO->surface;
    }

OnError:
    gcmFOOTER_ARG("return={0x%x, %d, %d}", masterView.surf, masterView.firstSlice, masterView.numSlices);
    return masterView;
}

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
gceSTATUS
gcChipFBOSyncEGLImageNativeBuffer(
    __GLcontext *gc,
    gcoSURF surface,
    GLboolean read
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLframebufferObject *fbo = read ? gc->frameBuffer.readFramebufObj : gc->frameBuffer.drawFramebufObj;
    __GLfboAttachPoint *attachPoint = NULL;
    GLint attachIdx = -1;
    struct gc_native_handle_t * handle = gcvNULL;

    gcmHEADER_ARG("gc=0x%x surface=0x%x read=%d", gc, surface, read);

    /* If surface is NULL or no FBO was used */
    if (!surface || fbo->name == 0)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Get attachIdx for read case and draw case */
    if (read)
    {
        if (surface == chipCtx->readRtView.surf)
        {
            attachIdx = __glMapAttachmentToIndex(fbo->readBuffer);
        }

        if (-1 == attachIdx && surface == chipCtx->readDepthView.surf)
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && surface == chipCtx->readStencilView.surf)
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }
    else
    {
        GLuint rtIdx;

        for (rtIdx = 0; rtIdx < gc->constants.shaderCaps.maxDrawBuffers; ++rtIdx)
        {
            if (surface == chipCtx->drawRtViews[rtIdx].surf)
            {
                /* It is a draw surface */
                attachIdx = __glMapAttachmentToIndex(fbo->drawBuffers[rtIdx]);
                break;
            }
        }

        if (-1 == attachIdx && surface == chipCtx->drawDepthView.surf)
        {
            attachIdx = __GL_DEPTH_ATTACHMENT_POINT_INDEX;
        }

        if (-1 == attachIdx && surface == chipCtx->drawStencilView.surf)
        {
            attachIdx = __GL_STENCIL_ATTACHMENT_POINT_INDEX;
        }
    }

    /* If the surface was not any of the current draw or read one */
    if (-1 == attachIdx)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }

    attachPoint = &fbo->attachPoint[attachIdx];

    if (GL_TEXTURE == attachPoint->objType)
    {
        khrEGL_IMAGE_PTR image;
        __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
        __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)(texObj->privateData);

        image = (khrEGL_IMAGE_PTR) texInfo->eglImage.image;

        if ((image != gcvNULL) &&
            (image->magic == KHR_EGL_IMAGE_MAGIC_NUM) &&
            (image->type == KHR_IMAGE_ANDROID_NATIVE_BUFFER) &&
            (image->surface != gcvNULL) &&
            (texInfo->eglImage.directSample))
        {
            android_native_buffer_t * native;
            struct private_handle_t * hnd;

            /* Cast to android native buffer. */
            native = (android_native_buffer_t *) texInfo->eglImage.nativeBuffer;

            if (native != gcvNULL)
            {
                /* Get private handle. */
                hnd = (struct private_handle_t *) native->handle;
                handle = gc_native_handle_get(native->handle);
            }
        }
    }

    /*
    else if (GL_RENDERBUFFER == attachPoint->objType)
    {
        __GLrenderbufferObject *rbo = NULL;
        __GLchipRenderbufferObject *chipRBO = NULL;
    }
     */

    /* Check composition signal. */
    if (handle != gcvNULL && handle->hwDoneSignal != 0)
    {
        gcsHAL_INTERFACE iface;

        gcmVERIFY_OK(gcoOS_Signal(gcvNULL, (gctSIGNAL) (gctUINTPTR_T) handle->hwDoneSignal, gcvFALSE));

        /* Signal the signal, so CPU apps
         * can lock again once resolve is done. */
        iface.command            = gcvHAL_SIGNAL;
        iface.engine             = gcvENGINE_RENDER;
        iface.u.Signal.signal    = handle->hwDoneSignal;
        iface.u.Signal.auxSignal = 0;
        /* Stuff the client's PID. */
        iface.u.Signal.process   = handle->clientPID;
        iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

        /* Schedule the event. */
        gcmVERIFY_OK(gcoHAL_ScheduleEvent(gcvNULL, &iface));

        /* Commit commands. */
        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif
