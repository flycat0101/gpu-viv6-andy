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


#include "gc_es_context.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    __GLES3_ZONE_PIXEL

#if (defined(DEBUG) || defined(_DEBUG) || gcdDUMP_FRAME_STATS)
extern GLboolean g_dbgPerDrawKickOff;
#endif

/**************************************************************************/
/* Implementation of internal functions                                   */
/**************************************************************************/

gceSTATUS
gcChipProcessPBO(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    const GLvoid **pBuf
    )
{
    __GLchipVertexBufferInfo *bufInfo = gcvNULL;
    gctPOINTER logical = gcvNULL;
    const GLbyte* buf = *pBuf;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x pBuf=%u", gc, bufObj, pBuf);

    if (bufObj == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);
    if (!bufInfo)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, &logical));
    *pBuf = (GLbyte *)logical + (gctSIZE_T)buf;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipPostProcessPBO(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLboolean isPacked
    )
{
    __GLchipVertexBufferInfo *bufInfo = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x isPacked=%u", gc, bufObj, isPacked);

    if (bufObj == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);
    if (!bufInfo)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));

    if (isPacked)
    {
        gcmONERROR(gcoBUFOBJ_CPUCacheOperation(bufInfo->bufObj, gcvCACHE_CLEAN));
    }

OnError:
    gcmFOOTER();
    return status;
}

GLvoid
gcChipProcessPixelStore(
    __GLcontext* gc,
    __GLpixelPackMode *packMode,
    gctSIZE_T width,
    gctSIZE_T height,
    GLenum format,
    GLenum type,
    gctSIZE_T skipImgs,
    gctSIZE_T *pRowStride,
    gctSIZE_T *pImgHeight,
    gctSIZE_T *pSkipBytes
    )
{
    gctSIZE_T bpp = 0;
    gctSIZE_T rowStride = 0;
    gctSIZE_T imgLength = packMode->lineLength  ? (gctSIZE_T)packMode->lineLength  : width;
    gctSIZE_T imgHeight = packMode->imageHeight ? (gctSIZE_T)packMode->imageHeight : height;

    gcmHEADER_ARG("gc=0x%x packMode=0x%x width=%u height=%u format=0x%04x type=0x%04x "
                  "skipImgs=%u pRowStride=0x%x pImgHeight=0x%x pSkipBytes=0x%x",
                  gc, packMode, width, height, format, type, skipImgs,
                  pRowStride, pImgHeight, pSkipBytes);

    /* pixel store unpack parameters */
    gcChipUtilGetImageFormat(format, type, gcvNULL, &bpp);

    rowStride = gcmALIGN(bpp * imgLength / 8, packMode->alignment);

    if (pRowStride)
    {
        *pRowStride = rowStride;
    }

    if (pImgHeight)
    {
        *pImgHeight = imgHeight;
    }

    if (pSkipBytes)
    {
        gctSIZE_T imgStride = rowStride * imgHeight;

        *pSkipBytes = skipImgs * imgStride                /* skip images */
                    + packMode->skipLines * rowStride     /* skip lines */
                    + packMode->skipPixels * bpp / 8;     /* skip pixels */
    }

    gcmFOOTER_NO();
}

__GL_INLINE GLboolean
gcChipUtilCalculateArea(
    GLint *pdx,
    GLint *pdy,
    GLint *psx,
    GLint *psy,
    GLint *pw,
    GLint *ph,
    GLint dstW,
    GLint dstH,
    GLint srcW,
    GLint srcH
    )
{
    gctINT32 srcsx, srcex, dstsx, dstex;
    gctINT32 srcsy, srcey, dstsy, dstey;

    gctINT32 dx = *pdx, dy = *pdy, sx = *psx, sy = *psy, w = *pw, h = *ph;

    gcmHEADER_ARG("pdx=0x%x pdy=0x%x psx=0x%x psy=0x%x pw=0x%x ph=0x%x"
                  "dstW=%d dstH=%d srcW=%d srcH=%d",
                   pdx, pdy, psx, psy, pw, ph, dstW, dstH, srcW, srcH);

    sx = gcmMIN(gcmMAX(sx, ((gctINT32)__glMinInt)>>2), ((gctINT32)__glMaxInt)>>2);
    sy = gcmMIN(gcmMAX(sy, ((gctINT32)__glMinInt)>>2), ((gctINT32)__glMaxInt)>>2);
    dx = gcmMIN(gcmMAX(dx, ((gctINT32)__glMinInt)>>2), ((gctINT32)__glMaxInt)>>2);
    dy = gcmMIN(gcmMAX(dy, ((gctINT32)__glMinInt)>>2), ((gctINT32)__glMaxInt)>>2);
    w = gcmMIN(w, ((gctINT32)__glMaxInt)>>2);
    h = gcmMIN(h, ((gctINT32)__glMaxInt)>>2);

    srcsx = sx;
    srcex = sx + w;
    dstsx = dx;
    dstex = dx + w;

    if (srcsx < 0)
    {
        dstsx -= srcsx;
        srcsx = 0;
    }
    if (srcex > srcW)
    {
        dstex -= srcex - srcW;
        srcex = srcW;
    }
    if (dstsx < 0)
    {
        srcsx -= dstsx;
        dstsx = 0;
    }
    if (dstex > dstW)
    {
        srcex -= dstex - dstW;
        dstex = dstW;
    }

    gcmASSERT(srcsx >= 0 && dstsx >= 0 && srcex <= srcW && dstex <= dstW);
    w = srcex - srcsx;
    gcmASSERT(w == dstex - dstsx);

    if (w <= 0)
    {
        gcmFOOTER_ARG("return=%d", gcvFALSE);
        return gcvFALSE;
    }

    srcsy = sy;
    srcey = sy + h;
    dstsy = dy;
    dstey = dy + h;

    if (srcsy < 0)
    {
        dstsy -= srcsy;
        srcsy = 0;
    }
    if (srcey > srcH)
    {
        dstey -= srcey - srcH;
        srcey = srcH;
    }
    if (dstsy < 0)
    {
        srcsy -= dstsy;
        dstsy = 0;
    }
    if (dstey > dstH)
    {
        srcey -= dstey - dstH;
        dstey = dstH;
    }
    gcmASSERT(srcsy >= 0 && dstsy >= 0 && srcey <= srcH && dstey <= dstH);
    h = srcey - srcsy;
    gcmASSERT(h == dstey - dstsy);

    if (h <= 0)
    {
        gcmFOOTER_ARG("return=%d", gcvFALSE);
        return gcvFALSE;
    }

    *pdx = dstsx;
    *pdy = dstsy;
    *psx = srcsx;
    *psy = srcsy;
    *pw  = w;
    *ph  = h;

    gcmFOOTER_ARG("return=%d", gcvTRUE);

    return gcvTRUE;
}



/**************************************************************************/
/* Implementation of EXPORTED FUNCTIONS                                   */
/**************************************************************************/
GLboolean
__glChipReadPixelsBegin(
    __GLcontext *gc
    )
{
    return GL_TRUE;
}

GLvoid
__glChipReadPixelsValidateState(
    __GLcontext *gc
    )
{
    return;
}

GLboolean
__glChipReadPixelsEnd(
    __GLcontext *gc
    )
{
#if (defined(DEBUG) || defined(_DEBUG) || gcdDUMP_FRAME_STATS)
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x", gc);
    if (g_dbgPerDrawKickOff)
    {
        /* Flush the cache, the surface parameter of gcoSURF_Flush() was only use for check */
        GL_ASSERT(chipCtx->readRtView.surf);
        gcmONERROR(gcoSURF_Flush(chipCtx->readRtView.surf));

        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

    }
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

GLboolean
__glChipReadPixels(
    __GLcontext *gc,
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    GLubyte *buf
    )
{
    __GLclientPixelState *ps = &gc->clientState.pixel;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsSURF_VIEW srcView = {gcvNULL, 0, 1};
    gcsSURF_VIEW dstView = {gcvNULL, 0, 1};
    gceSURF_FORMAT wrapformat = gcvSURF_UNKNOWN;
    GLint right, bottom;
    gctUINT w, h;
    GLint dx, dy, sx, sy, Width, Height;
    gctUINT dstWidth, dstHeight;
    __GLbufferObject *packBufObj = gcvNULL;
    __GLchipVertexBufferInfo *packBufInfo = gcvNULL;
    gctUINT32 physicalAddress = gcvINVALID_ADDRESS;
    gctPOINTER logicalAddress = buf;
    gctSIZE_T skipOffset = 0;
    GLuint lineLength = ps->packModes.lineLength ? ps->packModes.lineLength : (GLuint)width;
    GLuint imageHeight = ps->packModes.imageHeight ? ps->packModes.imageHeight : (GLuint)height;
    __GLformatInfo *formatInfo;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x x=%d y=%d width=%d height=%d format=0x%04x type=0x%04x buf=%x",
                   gc, x, y, width, height, format, type, buf);

    /* If chipCtx->readRT is a shadow surface, we should sync it to master resource
    ** And read pixels from master resource, as shadow resource may have different
    ** meaning (format) with master surface, such as SRGB encoding.
    ** Even for the same format between shadow and master, then we only sync once.
    */

    /* FIXME: Here we don't consider offset in surface for reading.
    ** If we read a face of a cube or one of array texture, we should set surface offset
    ** And all surface function should take offset into consideration
    */
    srcView = gcChipFboSyncFromShadowSurface(gc, &chipCtx->readRtView, GL_TRUE);

    /* When commands such as ReadPixels read from a layered framebuffer,
    ** the image at layer zero of the selected attachment is always used to obtain pixel values
    */
    if (srcView.numSlices > 1)
    {
        srcView.firstSlice = 0;
        srcView.numSlices = 1;
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A8B8G8R8;
        }
        else if (format == GL_BGRA_EXT)
        {
            wrapformat = gcvSURF_A8R8G8B8;
        }
        break;
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A2B10G10R10;
        }
        break;
    case GL_FLOAT:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A32B32G32R32F;
        }
        break;
    case GL_UNSIGNED_INT:
        if (format == GL_RGBA_INTEGER)
        {
            wrapformat = gcvSURF_A32B32G32R32UI;
        }
        break;
    case GL_INT:
        if (format == GL_RGBA_INTEGER)
        {
            wrapformat = gcvSURF_A32B32G32R32I;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
        {
            wrapformat = gcvSURF_A4R4G4B4;
        }
        break;
    case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
        {
            wrapformat = gcvSURF_A1R5G5B5;
        }
        break;
    default:
        break;
    }

    /* Check if framebuffer is complete */
    if (READ_FRAMEBUFFER_BINDING_NAME == 0)
    {
        formatInfo = gc->drawablePrivate->rtFormatInfo;
    }
    else
    {
        __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;
        formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
    }

    if (formatInfo == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (gcvSURF_UNKNOWN == wrapformat)
    {
        __GLchipFmtMapInfo *formatMapInfo = gcChipGetFormatMapInfo(gc, formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);
        wrapformat = formatMapInfo->requestFormat;
    }

    gcChipProcessPixelStore(gc, &ps->packModes, (gctSIZE_T)width, (gctSIZE_T)height,
                            format, type, 0, gcvNULL, gcvNULL, &skipOffset);

    /* The image is from pack buffer object? */
    packBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufObj;
    if (packBufObj)
    {
        packBufInfo = (__GLchipVertexBufferInfo *)(packBufObj->privateData);
        GL_ASSERT(packBufInfo);
        gcmONERROR(gcoBUFOBJ_Lock(packBufInfo->bufObj, &physicalAddress, &logicalAddress));
        gcmONERROR(gcoBUFOBJ_GetFence(packBufInfo->bufObj, gcvFENCE_TYPE_WRITE));

        skipOffset += __GL_PTR2SIZE(buf);
        physicalAddress += (gctUINT32)skipOffset;
    }
    logicalAddress = (gctPOINTER)((gctINT8_PTR)logicalAddress + skipOffset);

    /* Create the wrapper surface. */
    gcmONERROR(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_BITMAP,
                                 wrapformat, gcvPOOL_USER, &dstView.surf));
    gcmONERROR(gcoSURF_ResetSurWH(dstView.surf, width, height, lineLength, imageHeight, wrapformat));
    gcmONERROR(gcoSURF_WrapSurface(dstView.surf, ps->packModes.alignment, logicalAddress, physicalAddress));

    gcmONERROR(gcoSURF_GetSize(srcView.surf, &w, &h, gcvNULL));
    right  = gcmMIN(x + width,  (gctINT) w);
    bottom = gcmMIN(y + height, (gctINT) h);
    gcmONERROR(gcoSURF_GetSize(dstView.surf, &dstWidth, &dstHeight, gcvNULL));

    /*
    ** Set Non-Linear space for SRGB8_ALPHA8
    */
    if (formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8)
    {
        gcmONERROR(gcoSURF_SetColorSpace(dstView.surf, gcvSURF_COLOR_SPACE_NONLINEAR));
    }

    sx = x;
    sy = y;
    dx = 0;
    dy = 0;
    Width = right - x;
    Height = bottom - y;

    if (!gcChipUtilCalculateArea(&dx, &dy, &sx, &sy, &Width, &Height, dstWidth, dstHeight, w, h))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    do {
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted   = chipCtx->readYInverted;
        rlvArgs.uArgs.v2.srcOrigin.x = sx;
        rlvArgs.uArgs.v2.srcOrigin.y = chipCtx->readYInverted ? (GLint)(h - (sy + Height)) : sy;
        rlvArgs.uArgs.v2.dstOrigin.x = dx;
        rlvArgs.uArgs.v2.dstOrigin.y = dy;
        rlvArgs.uArgs.v2.rectSize.x  = Width;
        rlvArgs.uArgs.v2.rectSize.y  = Height;
        rlvArgs.uArgs.v2.numSlices   = 1;
        rlvArgs.uArgs.v2.dump        = gcvTRUE;

        if (packBufObj)
        {
            if (gcmIS_SUCCESS(gcoSURF_ResolveRect(&srcView, &dstView, &rlvArgs)))
            {
                break;
            }
        }
        gcmERR_BREAK(gcoSURF_CopyPixels(&srcView, &dstView, &rlvArgs));
    }
    while (gcvFALSE);

OnError:
    if (packBufInfo && gcvINVALID_ADDRESS != physicalAddress) /* The image is from pack buffer object */
    {
        /* CPU cache will not be flushed in HAL, bc HAL only see wrapped user pool surface.
        ** Instead it will be flushed when unlock the packed buffer as non-user pool node.
        */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(packBufInfo->bufObj));
        gcmVERIFY_OK(gcoBUFOBJ_CPUCacheOperation(packBufInfo->bufObj, gcvCACHE_CLEAN));
    }

    if (dstView.surf)
    {
        gcoSURF_Destroy(dstView.surf);
    }

    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

