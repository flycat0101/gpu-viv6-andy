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
#ifdef OPENGL40
#include "gc_chip_buffer.h"
#endif

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
#ifdef OPENGL40
GLvoid  __glChipCreateAccumBufferInfo(__GLcontext* gc,
                              gcoSURF accumSurf,
                              __GLdrawablePrivate *glDrawable)
{
    GLuint bufSize = 0;
    glsCHIPACCUMBUFFER   *chipAccumBuffer = NULL;

    bufSize = sizeof(glsCHIPACCUMBUFFER);
    gcoOS_Allocate(gcvNULL,
            bufSize,
            (gctPOINTER *) &chipAccumBuffer);

    gcoOS_ZeroMemory(chipAccumBuffer, bufSize);
    chipAccumBuffer->renderTarget = accumSurf;
    chipAccumBuffer->renderTargetFormat = accumSurf->format;
    gcChipclearAccumBuffer(gc, chipAccumBuffer);
    glDrawable->accumBuffer.privateData = chipAccumBuffer;
}

GLvoid initAccumOperationPatch(__GLcontext* gc)
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPACCUMBUFFER * accumBuffer = (glsCHIPACCUMBUFFER *)(draw->accumBuffer.privateData);
    glsTEXTUREINFO * textureInfo;
    glsTEXTURESAMPLER * sampler;
    gceSURF_FORMAT  format;
    gceSTATUS status = gcvSTATUS_OK;

    gcoTEXTURE_InitParams(chipCtx->hal, &accumBuffer->texture[1]);
    accumBuffer->texture[1].mipFilter = gcvTEXTURE_NONE;
    accumBuffer->texture[0] = accumBuffer->texture[1];

    textureInfo = &accumBuffer->textureInfo[1];
    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }

    gcoSURF_GetFormat(((gcoSURF)(draw->rtHandles[0])), NULL, &format);
    textureInfo->imageFormat = textureInfo->residentFormat = format;

    textureInfo->residentLevels = 1;
    textureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
    textureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
    textureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
    textureInfo->format = GL_LUMINANCE;

    status = gcoTEXTURE_AddMipMap(
           textureInfo->object,
           0,
           gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
           textureInfo->residentFormat,
           draw->width,
           draw->height,
           0,
           0,
           gcvPOOL_DEFAULT,
           gcvTRUE,
           gcvNULL
    );

    if (gcmIS_ERROR(status)) {
        return;
    }

    sampler = &accumBuffer->sampler[1];
    sampler->binding = textureInfo;
    sampler->genEnable = 0;
    sampler->coordType    = gcSHADER_FLOAT_X2;
    sampler->coordSwizzle = gcSL_SWIZZLE_XYYY;

    accumBuffer->sampler[0] = accumBuffer->sampler[1];
    sampler = &accumBuffer->sampler[0];
    sampler->binding = &accumBuffer->textureInfo[0];

    textureInfo = &accumBuffer->textureInfo[0];
    textureInfo->imageFormat = textureInfo->residentFormat = accumBuffer->renderTargetFormat;
    textureInfo->object = gcvNULL;

    status = gcoTEXTURE_ConstructEx(chipCtx->hal, gcvTEXTURE_2D, &textureInfo->object);
    if (gcmIS_ERROR(status)) {
        return;
    }
    status = gcoTEXTURE_AddMipMap(
           textureInfo->object,
           0,
           gcvUNKNOWN_MIPMAP_IMAGE_FORMAT,
           textureInfo->residentFormat,
           draw->width,
           draw->height,
           0,
           0,
           gcvPOOL_DEFAULT,
           gcvTRUE,
           gcvNULL
    );

    if (gcmIS_ERROR(status)) {
        return;
    }
}


/*******************************************************************************
**
**  initializeTempBitmap
**
**  Initialize the temporary bitmap image.
**
**  INPUT:
**
**      Context
**          Pointer to the context.
**
**      Format
**          Format of the image.
**
**      Width, Height
**          The size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

gceSTATUS initializeTempBitmap(
    IN __GLchipContext *chipCtx,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF bitmap = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Format=0x%04x Width=%u Height=%u",
                    chipCtx, Format, Width, Height);

    do
    {
        /* See if the existing surface can be reused. */
        if ((chipCtx->tempWidth  < Width)  ||
            (chipCtx->tempHeight < Height) ||
            (chipCtx->tempFormat != Format))
        {
            gctUINT width;
            gctUINT height;
            gctINT stride;
            gctPOINTER bits[3];
            gcsSURF_FORMAT_INFO_PTR info[2];

            /* Is there a surface allocated? */
            if (chipCtx->tempBitmap != gcvNULL)
            {
                /* Unlock the surface. */
                if (chipCtx->tempBits != gcvNULL)
                {
                    gcmERR_BREAK(gcoSURF_Unlock(
                        chipCtx->tempBitmap, chipCtx->tempBits
                        ));

                    chipCtx->tempBits = gcvNULL;
                }

                /* Destroy the surface. */
                gcmERR_BREAK(gcoSURF_Destroy(chipCtx->tempBitmap));

                /* Reset temporary surface. */
                chipCtx->tempBitmap       = gcvNULL;
                chipCtx->tempFormat       = gcvSURF_UNKNOWN;
                chipCtx->tempBitsPerPixel = 0;
                chipCtx->tempWidth        = 0;
                chipCtx->tempHeight       = 0;
                chipCtx->tempStride       = 0;
            }

            /* Valid surface requested? */
            if (Format != gcvSURF_UNKNOWN)
            {
                /* Round up the size. */
                width  = gcmALIGN(Width,  256);
                height = gcmALIGN(Height, 256);

                /* Allocate a new surface. */
                gcmONERROR(gcoSURF_Construct(
                    chipCtx->hal,
                    width, height, 1,
                    gcvSURF_BITMAP, Format,
                    gcvPOOL_UNIFIED,
                    &bitmap
                    ));

                /* Get the pointer to the bits. */
                gcmONERROR(gcoSURF_Lock(
                    bitmap, gcvNULL, bits
                    ));

                /* Query the parameters back. */
                gcmONERROR(gcoSURF_GetAlignedSize(
                    bitmap, &width, &height, &stride
                    ));

                /* Query format specifics. */
                gcmONERROR(gcoSURF_QueryFormat(
                    Format, info
                    ));

                /* Set information. */
                chipCtx->tempBitmap       = bitmap;
                chipCtx->tempBits         = bits[0];
                chipCtx->tempFormat       = Format;
                chipCtx->tempBitsPerPixel = info[0]->bitsPerPixel;
                chipCtx->tempWidth        = width;
                chipCtx->tempHeight       = height;
                chipCtx->tempStride       = stride;
            }
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;

OnError:
    if (bitmap != gcvNULL)
    {
        gcoSURF_Destroy(bitmap);
        bitmap = gcvNULL;
    }
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  resolveDrawToTempBitmap
**
**  Resolve specified area of the drawing surface to the temporary bitmap.
**
**  INPUT:
**
**      chipCtx
**          Pointer to the context.
**
**      SourceX, SourceY, Width, Height
**          The origin and the size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

gceSTATUS resolveDrawToTempBitmap(
    IN __GLchipContext *chipCtx,
    IN gcoSURF srcSurf,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status;
    gceSURF_FORMAT format;
    GLuint  drawRTWidth;
    GLuint  drawRTHeight;

    gcmHEADER_ARG("Context=0x%x SourceX=%d SourceY=%d Width=%d Height=%d",
                    chipCtx, SourceX, SourceY, Width, Height);

    do
    {
        gctUINT resX;
        gctUINT resY;
        gctUINT resW;
        gctUINT resH;

        gctUINT sourceX;
        gctUINT sourceY;

        gctINT left;
        gctINT top;
        gctINT right;
        gctINT bottom;

        gcsSURF_VIEW srcView = {srcSurf, 0, 1};
        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcoSURF_GetSize(srcSurf, &drawRTWidth, &drawRTHeight, gcvNULL);

        /* Clamp coordinates. */
        left   = gcmMAX(SourceX, 0);
        top    = gcmMAX(SourceY, 0);
        right  = gcmMIN(SourceX + Width,  (GLint) drawRTWidth);
        bottom = gcmMIN(SourceY + Height, (GLint) drawRTHeight);

        if ((right <= 0) || (bottom <= 0))
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        gcmERR_BREAK(gcoSURF_GetResolveAlignment(srcSurf,
                                                 &resX,
                                                 &resY,
                                                 &resW,
                                                 &resH));

        /* Convert GL coordinates. */
        sourceX = left;
        sourceY = top;

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.numSlices  = 1;

        /* Determine the aligned source origin. */
        rlvArgs.uArgs.v2.srcOrigin.x = sourceX & ~(resX - 1);
        rlvArgs.uArgs.v2.srcOrigin.y = sourceY & ~(resY - 1);
        if ((rlvArgs.uArgs.v2.srcOrigin.x + (gctINT) resW > (GLint) drawRTWidth)
        &&  (rlvArgs.uArgs.v2.srcOrigin.x > 0)
        )
        {
            rlvArgs.uArgs.v2.srcOrigin.x = (chipCtx->drawRTWidth - resW) & ~(resX - 1);
        }

        /* Determine the origin adjustment. */
        chipCtx->tempX = sourceX - rlvArgs.uArgs.v2.srcOrigin.x;
        chipCtx->tempY = sourceY - rlvArgs.uArgs.v2.srcOrigin.y;

        /* Determine the aligned area size. */
        rlvArgs.uArgs.v2.rectSize.x = (gctUINT) gcmALIGN(right  - left + chipCtx->tempX, resW);
        rlvArgs.uArgs.v2.rectSize.y = (gctUINT) gcmALIGN(bottom - top  + chipCtx->tempY, resH);

        gcmVERIFY_OK(gcoSURF_GetFormat(srcSurf, gcvNULL, &format));

        /* Initialize the temporary surface. */
        gcmERR_BREAK(initializeTempBitmap(
            chipCtx,
            format,
            rlvArgs.uArgs.v2.rectSize.x,
            rlvArgs.uArgs.v2.rectSize.y
            ));

        tmpView.surf = chipCtx->tempBitmap;
        gcmERR_BREAK(gcoSURF_ResolveRect(&srcView, &tmpView, &rlvArgs));

        /* Make sure the operation is complete. */
        gcmERR_BREAK(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

        /* Compute the pointer to the last line. */
        chipCtx->tempLastLine
            =  chipCtx->tempBits
            +  chipCtx->tempStride *  chipCtx->tempY
            + (chipCtx->tempX      *  chipCtx->tempBitsPerPixel) / 8;

    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}

/* Save current states */
GLvoid saveAttributes(__GLcontext* gc, __GLchipContext *chipCtx)
{
    __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;

    /* Save current states affected by push/pop */
    pDispatchTable->PushAttrib(gc, GL_CURRENT_BIT  | GL_POLYGON_BIT        | GL_POLYGON_STIPPLE_BIT |
                               GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT   | GL_DEPTH_BUFFER_BIT |
                               GL_FOG_BIT      | GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT |
                               GL_ALL_ATTRIB_BITS);

    /* Save current multi sample enable */
    chipCtx->multiSampleOn = gc->state.enables.multisample.multisampleOn;

    /* Save current error code */
    chipCtx->errorNo = gc->error;
}


/* Reset current states to default states for manual draw */
GLvoid resetAttributes(__GLcontext* gc, __GLchipContext *chipCtx)
{
    __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;
    __GLesDispatchTable *pESDispatchTable = &(gc->apiDispatchTable);

    /* Push Matrix*/
    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->PushMatrix(gc);
    pDispatchTable->MatrixMode(gc,GL_TEXTURE);
    pDispatchTable->PushMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->PushMatrix(gc);


    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->LoadIdentity(gc);
    pDispatchTable->MatrixMode(gc, GL_TEXTURE);
    pDispatchTable->LoadIdentity(gc);
    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->LoadIdentity(gc);

    /* Reset most of the states */
    pESDispatchTable->Disable(gc, GL_LIGHTING);
    pESDispatchTable->Disable(gc, GL_CULL_FACE);
    pESDispatchTable->Disable(gc, GL_POLYGON_STIPPLE);
    pESDispatchTable->Disable(gc, GL_POLYGON_OFFSET_FILL);
    pESDispatchTable->Disable(gc, GL_ALPHA_TEST);
    pESDispatchTable->Disable(gc, GL_BLEND);
    pESDispatchTable->Disable(gc, GL_LOGIC_OP);
    pESDispatchTable->Disable(gc, GL_STENCIL_TEST);
    pESDispatchTable->Disable(gc, GL_DEPTH_TEST);
    pESDispatchTable->Disable(gc, GL_SCISSOR_TEST);
    pESDispatchTable->DepthMask(gc, GL_FALSE);
    pESDispatchTable->Disable(gc, GL_VERTEX_PROGRAM_ARB);
    pESDispatchTable->Disable(gc, GL_FRAGMENT_PROGRAM_ARB);
    pESDispatchTable->Disable(gc, GL_FOG);
    pDispatchTable->PolygonMode(gc, GL_FRONT_AND_BACK, GL_FILL);
    pESDispatchTable->Viewport(gc, 0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);
    pESDispatchTable->DepthRangef(gc, 0, 1);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE0);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE1);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE2);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE3);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE4);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE5);
    pESDispatchTable->ActiveTexture(gc, GL_TEXTURE0);
    pESDispatchTable->Enable(gc, GL_TEXTURE_2D);
    pESDispatchTable->ActiveTexture(gc, GL_TEXTURE1);
    pESDispatchTable->Enable(gc, GL_TEXTURE_2D);

    /* Disable MSAA */
    pESDispatchTable->Disable(gc, GL_MULTISAMPLE);

    /*Disable glsl path*/
    pESDispatchTable->UseProgram(gc, 0);
    /* Clear error code */
    gc->error = 0;
}

/* Restore current states according to last save result */
GLvoid restoreAttributes(__GLcontext* gc, __GLchipContext *chipCtx)
{
    __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;
    __GLesDispatchTable *pESDispatchTable = &(gc->apiDispatchTable);

    /* Restore previous pushed states */
    pDispatchTable->PopAttrib(gc);

    /* Restore previous MSAA enable or disable */
    if(chipCtx->multiSampleOn)
    {
        pESDispatchTable->Enable(gc, GL_MULTISAMPLE);
    }
    else
    {
        pESDispatchTable->Disable(gc, GL_MULTISAMPLE);
    }

    /* Restore previous error code */
    gc->error = chipCtx->errorNo;
}


GLvoid __glChipAccum(__GLcontext* gc, GLenum op, GLfloat value)
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    glsTEXTURESAMPLER    oldSampler[2];
    gcsTEXTURE oldTexture[2];
    glsCHIPACCUMBUFFER * accumBuffer;
//    __GLchipRenderbufferObject *renderBuffer = gcvNULL;
//    glsCHIPRENDERBUFFER * renderBuffer = gcvNULL;
    GLint drawRTWidth, drawRTHeight;
    gceSTATUS status = gcvSTATUS_OK;
    __GLbitmask texmask;
    GLint drawtofront = 0;
    gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
    gcoSURF renderSurf = gcvNULL;

    __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;

    __GLcoord vertex[4] =
    {
        {{-1.0f, -1.0f, 1.0f, 1.0f}},
        {{ 1.0f, -1.0f, 1.0f, 1.0f}},
        {{ 1.0f,  1.0f, 1.0f, 1.0f}},
        {{-1.0f,  1.0f, 1.0f, 1.0f}},
    };

    /* We evaluateAttribute to update states before we change our special states */
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);

    /* Save current states */
    saveAttributes(gc, chipCtx);

    /* Reset current states to default states for manual draw */
    resetAttributes(gc, chipCtx);

    /* Disable color mask for accum op which target on accum buffer */
    switch (op)
    {
    case GL_ACCUM:
    case GL_LOAD:
    case GL_MULT:
    case GL_ADD:
        /* When accum on accum buffer, it should not affected by color mask */
        pDispatchTable->ColorMask(gc, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        pDispatchTable->ClampColorARB(gc, GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
        break;

    case GL_RETURN:
        /* When return to color buffer, it should be affected by color mask */
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    texmask = gc->texUnitAttrDirtyMask;
    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_FALSE);

    chipCtx->drawToAccumBuf = GL_TRUE;
    chipCtx->accumValue = value;

    oldSampler[0] = chipCtx->texture.sampler[0];
    oldSampler[1] = chipCtx->texture.sampler[1];
    oldTexture[0] = chipCtx->texture.halTexture[0];
    oldTexture[1] = chipCtx->texture.halTexture[1];

    chipCtx->hashKey.accumMode = gccACCUM_UNKNOWN;

    if (gc->drawablePrivate->width != 0 && gc->drawablePrivate->height != 0)
    {
        accumBuffer = (glsCHIPACCUMBUFFER*)(gc->drawablePrivate->accumBuffer.privateData);

        switch (op)
        {
        case GL_ACCUM:
        case GL_LOAD:
        case GL_MULT:
        case GL_ADD:
            switch (gc->state.pixel.readBuffer)
            {
            case GL_FRONT_LEFT:
            case GL_FRONT_RIGHT:
            case GL_FRONT:
                renderSurf = (gcoSURF)(gc->readablePrivate->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX]);
                break;

            case GL_BACK_LEFT:
            case GL_BACK_RIGHT:
            case GL_LEFT:
            case GL_RIGHT:
            case GL_BACK:
                renderSurf = (gcoSURF)(gc->readablePrivate->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX]);
                break;
            }
            break;

        case GL_RETURN:
            switch (gc->state.raster.drawBuffers[0])
            {
            case GL_FRONT_LEFT:
            case GL_FRONT_RIGHT:
            case GL_FRONT:
                renderSurf = (gcoSURF)(gc->readablePrivate->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX]);
                break;

            case GL_BACK_LEFT:
            case GL_BACK_RIGHT:
            case GL_LEFT:
            case GL_RIGHT:
            case GL_BACK:
                renderSurf = (gcoSURF)(gc->readablePrivate->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX]);
                break;
            }
            break;

        default:
            GL_ASSERT(0);
            break;
        }

        chipCtx->texture.sampler[0] = accumBuffer->sampler[0];
        chipCtx->texture.sampler[1] = accumBuffer->sampler[1];
        chipCtx->texture.halTexture[0] = accumBuffer->texture[0];
        chipCtx->texture.halTexture[1] = accumBuffer->texture[1];

        /* Blt rtResource to texResource if needed */

        chipCtx->hashKey.accumMode = op - GL_ACCUM + gccACCUM_ACCUM;
        /* Set new RT and textures */
        gcoSURF_GetSize(accumBuffer->renderTarget, (gctUINT*)&drawRTWidth, (gctUINT*)&drawRTHeight, gcvNULL);

        surfView.surf = accumBuffer->renderTarget;
        status = gco3D_SetTarget(chipCtx->engine, 0, &surfView, 0);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Resolve the rectangle to the temporary surface. */
        status = resolveDrawToTempBitmap(chipCtx,
            accumBuffer->renderTarget,
            0, 0,
            drawRTWidth,
            drawRTHeight);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Upload the texture. */
        status = gcoTEXTURE_Upload(accumBuffer->textureInfo[0].object,
                                   0,
                                   gcvFACE_NONE,
                                   drawRTWidth,
                                   drawRTHeight,
                                   0,
                                   chipCtx->tempLastLine,
                                   chipCtx->tempStride,
                                   chipCtx->tempFormat,
                                   gcvSURF_COLOR_SPACE_LINEAR);

        gcoSURF_GetSize(renderSurf, (gctUINT *)&drawRTWidth, (gctUINT *)&drawRTHeight, gcvNULL);

        surfView.surf = renderSurf;
        status = gco3D_SetTarget(chipCtx->engine, 0, &surfView, 0);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Resolve the rectangle to the temporary surface. */
        status = resolveDrawToTempBitmap(chipCtx,
            renderSurf,
            0, 0,
            drawRTWidth,
            drawRTHeight);

        if (gcmIS_ERROR(status))
        {
            return;
        }

        /* Upload the texture. */
        status = gcoTEXTURE_Upload(accumBuffer->textureInfo[1].object,
                                   0,
                                   gcvFACE_NONE,
                                   drawRTWidth,
                                   drawRTHeight,
                                   0,
                                   chipCtx->tempLastLine,
                                   chipCtx->tempStride,
                                   chipCtx->tempFormat,
                                   gcvSURF_COLOR_SPACE_LINEAR);

        surfView.surf = accumBuffer->renderTarget;
        if (op != GL_RETURN)
        {
            gco3D_SetTarget(chipCtx->engine, 0, &surfView, 0);
        }

        if ( gc->flags & __GL_DRAW_TO_FRONT)
        {
            gc->flags &= ~__GL_DRAW_TO_FRONT;
            drawtofront = 1;
        }

        /* Draw a quad */
        pDispatchTable->Begin(gc, GL_QUADS);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0, 0, 0);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE1, 0, 0);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex[0]);

        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0, 1, 0);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE1, 1, 0);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex[1]);

        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0, 1, 1);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE1, 1, 1);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex[2]);

        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0, 0, 1);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE1, 0, 1);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex[3]);
        pDispatchTable->End(gc);

        /* Flush the cache. */
        if (op != GL_RETURN)
        {
            gcoSURF_Flush(accumBuffer->renderTarget);
        }
        else
        {
            gcoSURF_Flush(renderSurf);
        }
        gcoHAL_Commit(chipCtx->hal, gcvTRUE);

        surfView.surf = renderSurf;

        if (op != GL_RETURN)
        {
            gco3D_SetTarget(chipCtx->engine, 0, &surfView, 0);
        }

        if ( drawtofront)
        {
            gc->flags |= __GL_DRAW_TO_FRONT;

            if (op == GL_RETURN)
            {
                pDispatchTable->Flush(gc);
            }
        }
    }

    chipCtx->texture.sampler[0] = oldSampler[0];
    chipCtx->texture.sampler[1] = oldSampler[1];
    chipCtx->texture.halTexture[0] = oldTexture[0];
    chipCtx->texture.halTexture[1] = oldTexture[1];

    chipCtx->hashKey.accumMode = gccACCUM_UNKNOWN;

    gc->texUnitAttrDirtyMask = texmask;

    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->PopMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_TEXTURE);
    pDispatchTable->PopMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->PopMatrix(gc);

    /* Restore saved current states */
    restoreAttributes(gc, chipCtx);
}

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/

void __glChipRasterBegin(__GLcontext *gc, GLenum rasterOp,GLenum format, GLint width, GLint height)
{

}

void __glChipRasterEnd(__GLcontext *gc, GLenum rasterOp)
{
    if (gc->drawablePrivate->pbufferTex &&
         (rasterOp == __GL_RASTERFUNC_BITMAP ||
          rasterOp == __GL_RASTERFUNC_COPYPIX ||
          rasterOp == __GL_RASTERFUNC_DRAWPIX))
    {
        gc->drawablePrivate->pbufferTex->needGenMipmap = GL_TRUE;
    }
}

GLboolean __glChipBitmaps(__GLcontext *gc, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap, __GLbufferObject* bufObj)
{
    GLubyte     *newbits = NULL;
    GLboolean   bFreeNewBits = GL_FALSE;
    GLint       w, h, shiftBit;
    GLubyte     *texImageData = NULL, *texImagePointer = NULL;
    GLint       align_bytes = width / 8;
    GLint       reminder_bits = width % 8;
    GLuint      index = 0;

    if (width == 0 || height == 0)
    {
        return GL_TRUE;
    }

    /* If any of the shader stage was enabled, */
    if (gc->state.enables.program.vertexProgram || gc->state.enables.program.fragmentProgram ||
        gc->shaderProgram.vertShaderEnable || gc->shaderProgram.fragShaderEnable || gc->shaderProgram.geomShaderEnable)
    {
        return GL_FALSE;
    }

    /*temporarily disable*/

    if (((((width + 7) / 8) % gc->clientState.pixel.unpackModes.alignment) == 0) &&
        (gc->clientState.pixel.unpackModes.lineLength == 0) &&
        (gc->clientState.pixel.unpackModes.skipLines  == 0) &&
        (gc->clientState.pixel.unpackModes.skipPixels == 0) &&
        (gc->clientState.pixel.unpackModes.lsbFirst == 0))
    {
        newbits = (GLubyte *)bitmap;
    }
    else
    {
        newbits = (GLubyte *) (*gc->imports.malloc)(gc, (GLsizei)__glImageSize(width, height, GL_COLOR_INDEX, GL_BITMAP));
        __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP, bitmap, newbits);
        bFreeNewBits = GL_TRUE;
    }

    texImageData =
    texImagePointer = (GLubyte *)(*gc->imports.malloc)(gc, 2*width * height);

    GL_ASSERT(texImagePointer);

    /* Convert bit mask to image */
    for (h = 0; h < height; h++)
    {
        for (w = 0; w < align_bytes; w++)
        {
            /* Revert bit */
            for (shiftBit = 7; shiftBit >= 0; shiftBit--)
            {
                if (newbits[index] & (1 << shiftBit))
                {
                    *texImagePointer++ = 0xff;
                    *texImagePointer++ = 0xff;
                }
                else
                {
                    *texImagePointer++ = 0x0;
                    *texImagePointer++ = 0x0;
                }
            }
            index++;
        }

        for (shiftBit = 0; shiftBit < reminder_bits; shiftBit++)
        {
            if (newbits[index] & (1 << (7 - shiftBit)))
            {
                *texImagePointer++ = 0xff;
                *texImagePointer++ = 0xff;
            }
            else
            {
                *texImagePointer++ = 0x0;
                *texImagePointer++ = 0x0;
            }
        }

        if (reminder_bits)
        {
            index ++;
        }
    }

    {
        GLuint texture;
        GLint oldError = gc->error;
        GLuint texStage = __GL_MAX_TEXTURE_COORDS - 1;
        __GLcoord vertex = gc->state.rasterPos.rPos.winPos;
        __GLcolor color = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
        __GLcolor secondaryColor = gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
        __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;
        __GLesDispatchTable *pESDispatchTable = &(gc->apiDispatchTable);

        /* The simulation need to use the last texture unit. But if it was already been used, return FALSE */
        if (gc->state.enables.texUnits[texStage].enabledDimension > 0 )
        {
            return GL_FALSE;
        }

        gc->error = 0;

        vertex.f.x = (GLfloat)((GLint) (vertex.f.x - xorig));
        /*temporarily disable*/
        vertex.f.y = (GLfloat)((GLint) (vertex.f.y - yorig));

        /* Push state */
        pDispatchTable->PushAttrib(gc, GL_ALL_ATTRIB_BITS | GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_POLYGON_STIPPLE_BIT |
                                   GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT);

        pESDispatchTable->ActiveTexture(gc, GL_TEXTURE0 + texStage);

        /* Push Matrix*/
        pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
        pDispatchTable->PushMatrix(gc);
        pDispatchTable->MatrixMode(gc, GL_TEXTURE);
        pDispatchTable->PushMatrix(gc);
        pDispatchTable->MatrixMode(gc, GL_PROJECTION);
        pDispatchTable->PushMatrix(gc);

        /* State setting */
        pESDispatchTable->Disable(gc, GL_LIGHTING);/*Disable lighting*/
        pESDispatchTable->Disable(gc, GL_CULL_FACE);
        pESDispatchTable->Disable(gc, GL_POLYGON_STIPPLE);
        pDispatchTable->PolygonMode(gc, GL_FRONT_AND_BACK, GL_FILL);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE0);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE1);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE2);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE3);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE4);
        pESDispatchTable->Disable(gc, GL_CLIP_PLANE5);
        pESDispatchTable->Enable(gc, GL_ALPHA_TEST);
        pDispatchTable->AlphaFunc(gc, GL_GREATER, 0);


        /* Texture initialization */
        pESDispatchTable->GenTextures(gc, 1, &texture);   /* Create a texture*/
        pESDispatchTable->BindTexture(gc, GL_TEXTURE_2D, texture);
        pESDispatchTable->Enable(gc, GL_TEXTURE_2D);      /* Enable texture */
        pESDispatchTable->TexParameteri(gc, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        pESDispatchTable->TexParameteri(gc, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, __GL_STIPPLE);
        pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
        pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
        pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_S);
        pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_T);
        pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_R);
        pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_Q);

        pESDispatchTable->TexImage2D(gc, GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
                                   width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texImageData);

        /* Transformation Matrix */
        pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
        pDispatchTable->LoadIdentity(gc);
        pDispatchTable->MatrixMode(gc, GL_PROJECTION);
        pDispatchTable->LoadIdentity(gc);
        // The raster pos are already in screen space, its depth are already in range [0, 1]
        pDispatchTable->Ortho(gc, 0,gc->drawablePrivate->width, 0, gc->drawablePrivate->height, 0, -1);
        pESDispatchTable->Viewport(gc, 0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);

        /* Depth Range */
        pESDispatchTable->DepthRangef(gc, 0, 1);

        /*==============Issue draw====================*/
        /* Draw a Quad*/
        /* Spec vertex with only xyz channel.*/

        pDispatchTable->Color4f(gc, color.r, color.g, color.b, color.a);
        pDispatchTable->SecondaryColor3f(gc, secondaryColor.r, secondaryColor.g, secondaryColor.b);

        pDispatchTable->Begin(gc, GL_QUADS);
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 0, 0);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex);

        vertex.f.y += height * gc->state.pixel.transferMode.zoomY;
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 0, 1);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex);

        vertex.f.x += width * gc->state.pixel.transferMode.zoomX;
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 1, 1);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex);

        vertex.f.y -= height * gc->state.pixel.transferMode.zoomY;
        pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 1, 0);
        pDispatchTable->Vertex4fv(gc, (GLfloat *)&vertex);
        pDispatchTable->End(gc);

        pESDispatchTable->Flush(gc);

        /*Delete texture*/
        pESDispatchTable->DeleteTextures(gc, 1, &texture);

        /*=============restore state===========================*/
        /* Pop Matrix*/
        pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
        pDispatchTable->PopMatrix(gc);
        pDispatchTable->MatrixMode(gc, GL_TEXTURE);
        pDispatchTable->PopMatrix(gc);
        pDispatchTable->MatrixMode(gc, GL_PROJECTION);
        pDispatchTable->PopMatrix(gc);
        /*Pop State*/
        pDispatchTable->PopAttrib(gc);

        /* Do we have error?*/
        if (gc->error)
        {
            gc->error = oldError;
            GL_ASSERT(0);
            goto __Exit;
        }

        /* Restore the error value */
        gc->error = oldError;
    }

__Exit:

    if (texImageData)
    {
        (*gc->imports.free)(gc, texImageData);
    }

    if (bFreeNewBits)
    {
        (*gc->imports.free)(gc, newbits);
    }

    /*temporarily disable*/
    return GL_TRUE;
}


static GLvoid checkPixelTransferAttrib(__GLcontext *gc, __GLchipContext *chipCtx)
{

    if ((gc->state.pixel.transferMode.r_scale != 1.0f) ||
        (gc->state.pixel.transferMode.g_scale != 1.0f) ||
        (gc->state.pixel.transferMode.b_scale != 1.0f) ||
        (gc->state.pixel.transferMode.a_scale != 1.0f))
    {
        /* pixel transfer scale enabled */
        glmSETHASH_1BIT(hashPixelTransfer, 1, gccPIXEL_TRANSFER_SCALE);
    }

    if ((gc->state.pixel.transferMode.r_bias != 0.0f) ||
        (gc->state.pixel.transferMode.g_bias != 0.0f) ||
        (gc->state.pixel.transferMode.b_bias != 0.0f) ||
        (gc->state.pixel.transferMode.a_bias != 0.0f))
    {
        /* pixel transfer scale enabled */
        glmSETHASH_1BIT(hashPixelTransfer, 1, gccPIXEL_TRANSFER_BIAS);
    }
}
GLboolean simulatePixelOperation(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type, const GLvoid *pixels, GLboolean bDrawPixels)
{
    GLuint texture;
    GLenum internalFormat;
    GLint  oldError = gc->error;
    GLuint texStage = __GL_MAX_TEXTURE_COORDS - 1;
    __GLcoord vertex = gc->state.rasterPos.rPos.winPos;
    __GLcolor color = gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
    __GLcolor secondaryColor = gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
    __GLesDispatchTable *pDispatchTable = gc->currentImmediateTable;
    __GLesDispatchTable *pESDispatchTable = &(gc->apiDispatchTable);
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    if (gc->renderMode != GL_RENDER  || format == GL_STENCIL_INDEX)
    {
        return GL_FALSE;
    }

    /*
    ** The spec says: when writing to the stencil buffer, "If a depth component is present,
    ** and the setting of DepthMask is not FALSE, is also written to the framebuffer; the
    ** setting of DepthTest is ignored."
    ** The simulation path cannot handle the case.
    */
    if (format == GL_DEPTH_COMPONENT)
    {
        return GL_FALSE;
    }

    /* The simulation need to use the last texture unit. But if it was already been used, return FALSE */
    if (gc->state.enables.texUnits[texStage].enabledDimension > 0 )
    {
        return GL_FALSE;
    }

    /* If any of the shader stage was enabled, */
    if (gc->state.enables.program.vertexProgram || gc->state.enables.program.fragmentProgram ||
        gc->shaderProgram.vertShaderEnable || gc->shaderProgram.fragShaderEnable || gc->shaderProgram.geomShaderEnable)
    {
        return GL_FALSE;
    }

    gc->error = 0;
    /*
    if(gc->drawablePrivate->yInverted)
    {
        vertex.f.y = gc->drawablePrivate->height - vertex.f.y;
    }
    */
    /* Push state */
    pDispatchTable->PushAttrib(gc, GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_POLYGON_STIPPLE_BIT |
                               GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT);

    pESDispatchTable->ActiveTexture(gc, GL_TEXTURE0 + texStage);

    /* Push Matrix*/
    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->PushMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_TEXTURE);
    pDispatchTable->PushMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->PushMatrix(gc);

    /* State setting */
    pESDispatchTable->Disable(gc, GL_LIGHTING);/*Disable lighting*/
    pESDispatchTable->Disable(gc, GL_CULL_FACE);
    pESDispatchTable->Disable(gc, GL_POLYGON_STIPPLE);
    pDispatchTable->PolygonMode(gc, GL_FRONT_AND_BACK, GL_FILL);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE0);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE1);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE2);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE3);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE4);
    pESDispatchTable->Disable(gc, GL_CLIP_PLANE5);

    /* Texture initialization */
    pESDispatchTable->GenTextures(gc, 1, &texture);   /* Create a texture*/
    pESDispatchTable->BindTexture(gc, GL_TEXTURE_2D, texture);
    pESDispatchTable->Enable(gc, GL_TEXTURE_2D);      /* Enable texture */
    pESDispatchTable->TexParameteri(gc, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    pESDispatchTable->TexParameteri(gc, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
    pDispatchTable->TexEnvi(gc, GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
    pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_S);
    pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_T);
    pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_R);
    pESDispatchTable->Disable(gc, GL_TEXTURE_GEN_Q);

    chipCtx->hashKey.hashPixelTransfer = 0;
    checkPixelTransferAttrib(gc, chipCtx);

    if (format == GL_DEPTH_COMPONENT)
    {
 //       gc->texture.drawDepthStage = texStage;

        /* For depth component, the current color should be set to the current raster color*/
        pDispatchTable->Color4f(gc, color.r, color.g, color.b, color.a);
        pDispatchTable->SecondaryColor3f(gc, secondaryColor.r, secondaryColor.g, secondaryColor.b);
    }
    else
    {
        /*
        ** For color component, the current color may be used in the texture blending:
        ** when format == GL_ALPHA( no RGB) or format = GL_RGB( no Alpha).  so the
        ** current color should be set to (0,0,0,1) in order to do correct simulation
        */
        pDispatchTable->Color4f(gc, 0.0, 0.0, 0.0, 1.0);
    }

    switch (format)
    {
    /*
    ** When format is one of color formats, we need the internalFormat include all
    ** color channels for DrawPixel simulation path. Because Simulation will load
    ** the source channel image to a texture and draw with texenv replace.
    ** the result will be Channel= texture.Channel,means the channels
    ** not included will not changed. But, with glDrawPixels pipe, the dest color
    ** should be set to default value(0,0,0,1).
    */
    case GL_STENCIL_INDEX:
    case GL_DEPTH_COMPONENT:
        internalFormat = format;
        break;
/*
    case GL_RGBA_INTEGER_EXT:
    case GL_BGRA_INTEGER_EXT:
        internalFormat = GL_RGBA32UI_EXT;
        break;
    case GL_BLUE_INTEGER_EXT:
    case GL_GREEN_INTEGER_EXT:
    case GL_ALPHA_INTEGER_EXT:
        internalFormat = GL_R32UI;
        break;
    case GL_RGB_INTEGER_EXT:
    case GL_BGR_INTEGER_EXT:
        internalFormat = GL_RGB32UI_EXT;
        break;
    case GL_LUMINANCE_INTEGER_EXT:
        internalFormat = GL_LUMINANCE32UI_EXT;
        break;
    case GL_LUMINANCE_ALPHA_INTEGER_EXT:
        internalFormat = GL_LUMINANCE_ALPHA32UI_EXT;
        break;
*/
    default:
        if(gc->modes.rgbFloatMode)
        {
            internalFormat = GL_RGBA32F_ARB;
        }
        else
        {
            if(gc->modes.alphaBits == 0)
            {
                internalFormat = GL_RGB;
            }
            else
            {
                internalFormat = GL_RGBA;
            }
        }
        break;
    }

    if (bDrawPixels)
    {
        gctUINT8* mappedPixels = gcvNULL;
        gctUINT8 index, clampIndex;
        GLsizei i;

        if ((format == GL_COLOR_INDEX) && (type == GL_UNSIGNED_BYTE))
        {
            /* map the color */
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                           width * height * 4,
                                           (gctPOINTER*)&mappedPixels)))
            {
                return GL_FALSE;
            }


            for (i = 0; i < width * height; i++)
            {
                index = ((gctUINT8*)pixels)[i];

                clampIndex =  (gctUINT8)gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_R - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[0 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_R - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex =  (gctUINT8)gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_G - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[1 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_G - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex =  (gctUINT8)gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_B - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[2 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_B - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);

                clampIndex =  (gctUINT8)gcmMIN(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_A - GL_PIXEL_MAP_I_TO_I].size, index);
                mappedPixels[3 + i * 4] = \
                    (gctUINT8)(gc->state.pixel.pixelMap[GL_PIXEL_MAP_I_TO_A - GL_PIXEL_MAP_I_TO_I].base.mapF[clampIndex] * 255);
            }

            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            pixels = mappedPixels;
        }

        pESDispatchTable->TexImage2D(gc, GL_TEXTURE_2D, 0, internalFormat,
                                   width, height, 0, format, type, pixels);

        if (mappedPixels != gcvNULL)
        {
            gcoOS_Free(gcvNULL, mappedPixels);
        }

    }
    else
    {
        pESDispatchTable->CopyTexImage2D(gc, GL_TEXTURE_2D, 0, internalFormat,
                                       x, y, width, height, 0);
    }

    /* Transformation Matrix */
    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->LoadIdentity(gc);
    pDispatchTable->MatrixMode(gc, GL_TEXTURE);
    pDispatchTable->LoadIdentity(gc);
    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->LoadIdentity(gc);
    // The raster pos are already in screen space, its depth are already in range [0, 1]
    pDispatchTable->Ortho(gc, 0,gc->drawablePrivate->width, 0, gc->drawablePrivate->height, 0, -1);
    pDispatchTable->Viewport(gc, 0, 0, gc->drawablePrivate->width, gc->drawablePrivate->height);

    /* Depth Range */
    pDispatchTable->DepthRange(gc, 0,1);

    /* Draw a Quad*/
    /* Spec vertex with only xyz channel.*/
    pDispatchTable->Begin(gc, GL_QUADS);
    pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 0, 0 );
    pDispatchTable->Vertex3fv(gc, (GLfloat *)&vertex);

    vertex.f.y += height * gc->state.pixel.transferMode.zoomY;
    pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 0, 1 );
    pDispatchTable->Vertex3fv(gc, (GLfloat *)&vertex);

    vertex.f.x += width * gc->state.pixel.transferMode.zoomX;
    pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 1, 1 );
    pDispatchTable->Vertex3fv(gc, (GLfloat *)&vertex);

    vertex.f.y -= height * gc->state.pixel.transferMode.zoomY;
    pDispatchTable->MultiTexCoord2f(gc, GL_TEXTURE0 + texStage, 1, 0 );
    pDispatchTable->Vertex3fv(gc, (GLfloat *)&vertex);
    pDispatchTable->End(gc);

    pESDispatchTable->Flush(gc);

    /*Delete texture*/
    pESDispatchTable->DeleteTextures(gc, 1, &texture);

    /* Pop Matrix*/
    pDispatchTable->MatrixMode(gc, GL_MODELVIEW);
    pDispatchTable->PopMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_TEXTURE);
    pDispatchTable->PopMatrix(gc);
    pDispatchTable->MatrixMode(gc, GL_PROJECTION);
    pDispatchTable->PopMatrix(gc);
    /*Pop State*/
    pDispatchTable->PopAttrib(gc);

//    gc->texture.drawDepthStage = -1;

    chipCtx->hashKey.hashPixelTransfer = 0;

    /* Do we have error?*/
    if (gc->error)
    {
        gc->error = oldError;
        GL_ASSERT(0);
        return GL_FALSE;
    }

    /* Restore the error value */
    gc->error = oldError;
    return GL_TRUE;
}

GLboolean
calculateArea(
    GLint *pdx, GLint *pdy,
    GLint *psx, GLint *psy,
    GLint *pw, GLint *ph,
    GLint dstW, GLint dstH,
    GLint srcW, GLint srcH
    )
{
    gctINT32 srcsx, srcex, dstsx, dstex;
    gctINT32 srcsy, srcey, dstsy, dstey;

    gctINT32 dx = *pdx, dy = *pdy, sx = *psx, sy = *psy, w = *pw, h = *ph;

    gcmHEADER_ARG("pdx=0x%x pdy=0x%x psx=0x%x psy=0x%x pw=0x%x ph=0x%x dstW=%d dstH=%d srcW=%d srcH=%d",
        pdx, pdy, psx, psy, pw, ph, dstW, dstH, srcW, srcH);

    sx = gcmMIN(gcmMAX(sx, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    sy = gcmMIN(gcmMAX(sy, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    dx = gcmMIN(gcmMAX(dx, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    dy = gcmMIN(gcmMAX(dy, (gctINT32)((gctINT32)__glMinInt>>2)), (gctINT32)((gctINT32)__glMaxInt>>2));
    w = gcmMIN(w, (gctINT32)((gctINT32)__glMaxInt>>2));
    h = gcmMIN(h, (gctINT32)((gctINT32)__glMaxInt>>2));

    srcsx = sx;
    srcex = sx + w;
    dstsx = dx;
    dstex = dx + w;

    if(srcsx < 0)
    {
        dstsx -= srcsx;
        srcsx = 0;
    }
    if(srcex > srcW)
    {
        dstex -= srcex - srcW;
        srcex = srcW;
    }
    if(dstsx < 0)
    {
        srcsx -= dstsx;
        dstsx = 0;
    }
    if(dstex > dstW)
    {
        srcex -= dstex - dstW;
        dstex = dstW;
    }

    gcmASSERT(srcsx >= 0 && dstsx >= 0 && srcex <= srcW && dstex <= dstW);
    w = srcex - srcsx;
    gcmASSERT(w == dstex - dstsx);

    if(w <= 0)
    {
        gcmFOOTER_ARG("return=%s", "FALSE");

        return gcvFALSE;
    }

    srcsy = sy;
    srcey = sy + h;
    dstsy = dy;
    dstey = dy + h;

    if(srcsy < 0)
    {
        dstsy -= srcsy;
        srcsy = 0;
    }
    if(srcey > srcH)
    {
        dstey -= srcey - srcH;
        srcey = srcH;
    }
    if(dstsy < 0)
    {
        srcsy -= dstsy;
        dstsy = 0;
    }
    if(dstey > dstH)
    {
        srcey -= dstey - dstH;
        dstey = dstH;
    }
    gcmASSERT(srcsy >= 0 && dstsy >= 0 && srcey <= srcH && dstey <= dstH);
    h = srcey - srcsy;
    gcmASSERT(h == dstey - dstsy);

    if(h <= 0)
    {
        gcmFOOTER_ARG("return=%s", "FALSE");

        return gcvFALSE;
    }

    *pdx = dstsx;
    *pdy = dstsy;
    *psx = srcsx;
    *psy = srcsy;
    *pw  = w;
    *ph  = h;

    gcmFOOTER_ARG("return=%s", "TRUE");

    return gcvTRUE;
}
GLboolean __glChipCopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format)
{
    /*
    ** Current Vivante chip doesn't support drawpixels directly. So we simulated using texture draw for some cases.
    */

    return simulatePixelOperation(gc, x, y, width, height, format, 0, NULL, GL_FALSE);
}

GLboolean __glChipDrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *pixels)
{
    /*
    ** Current Vivante chip doesn't support drawpixels directly. So we simulated using texture draw for some cases.
    */

    if (gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufName > 0)
    {
        return GL_FALSE;
    }

    return simulatePixelOperation(gc, 0, 0, width, height, format, type, pixels, GL_TRUE);
}

#define GET_SOURCE(s) \
    ((*s >> (srcShift)) & srcMax) \

#define CONVERT_FIXED_TO_FIXED(dstDataType, srcDataType) \
    *d = (dstDataType)__GL_FLOORF((( GET_SOURCE(s) / (gctFLOAT)(srcMax)) * (dstMax)) + 0.5) \

#define CONVERT_FIXED_TO_FLOAT(dstDataType, srcDataType) \
    *d = (dstDataType)(GET_SOURCE(s) / (gctFLOAT)(srcMax)) \

#define CONVERT_NONE(dstDataType, srcDataType) \
    *d = (dstDataType) GET_SOURCE(s) \

#define CONVERT_DEPTH_PIXELS(dstDataType, srcDataType, convertFunc) \
    do \
    { \
        srcDataType* s;\
        dstDataType* d;\
        for (j = 0; j < h; j++) \
        {\
            for (i = 0; i < w; i++) \
            {\
                s = ((srcDataType*)((gctUINT8*)srcData[0] + (j + sy) * srcStride)) + (i + sx);\
                d = ((dstDataType*)((gctUINT8*)dstData + (j + dy) * dstStride)) + (i + dx);\
                convertFunc(dstDataType, srcDataType);\
            }\
        }\
    } while(gcvFALSE) \


gceSTATUS
__glChipReadDepthStencilPixels(__GLcontext *gc,
                       GLint x, GLint y,
                       GLsizei width, GLsizei height,
                       GLenum format, GLenum type, GLubyte *buf)
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSURF_FORMAT srcFormat;
    gctUINT32 srcWidth, srcHeight;
    GLint dx, dy, sx, sy, w, h, i, j;
    gctINT srcStride, dstStride;
    gctUINT32 srcShift, srcMax, dstMax;
    gctPOINTER srcData[3] = {gcvNULL};
    gctPOINTER dstData;
    gcsSURF_VIEW srcView = chipCtx->readDepthView;
    gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};

    gceSTATUS status = gcvSTATUS_OK;

    if (srcView.surf == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    gcmONERROR(gcoSURF_GetSize(srcView.surf,
                               &srcWidth,
                               &srcHeight,
                               gcvNULL));

    sx = x; sy = y; dx = 0; dy = 0; w = width; h = height;

    if (!calculateArea(&dx, &dy, &sx, &sy,
                        &w, &h, width, height,
                        srcWidth, srcHeight))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    {
        /* limitation : */
        /* 1. Enable FC, can not use CopyPixels directly */
        /* 2. can not resolve a depth surface to a user surface */
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcmONERROR(gcoSURF_GetFormat(srcView.surf, gcvNULL, &srcFormat));

        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     w, h, 1,
                                     gcvSURF_BITMAP,
                                     srcFormat,
                                     gcvPOOL_DEFAULT,
                                     &tmpView.surf));

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.srcOrigin.x = sx;
        rlvArgs.uArgs.v2.srcOrigin.y = sy;
        rlvArgs.uArgs.v2.rectSize.x  = w;
        rlvArgs.uArgs.v2.rectSize.y  = h;
        rlvArgs.uArgs.v2.numSlices   = 1;
        gcmONERROR(gcoSURF_ResolveRect(&srcView, &tmpView, &rlvArgs));

        gcmONERROR(gcoSURF_Flush(tmpView.surf));

        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    gcmONERROR(gcoSURF_GetAlignedSize(
        tmpView.surf,
        gcvNULL,
        gcvNULL,
        &srcStride));

    gcmONERROR(gcoSURF_Lock(tmpView.surf, gcvNULL, srcData));

    dstData = buf; sx = 0; sy = 0;

    switch (srcFormat)
    {
    case gcvSURF_D16:
        srcShift = 0;
        srcMax = (1 << 16) - 1;

        switch(type)
        {
        case GL_UNSIGNED_SHORT:
            dstMax = 0;
            dstStride = width * 2;
            CONVERT_DEPTH_PIXELS(gctUINT16, gctUINT16, CONVERT_NONE);
            break;
        case GL_UNSIGNED_BYTE:
            dstMax = (1 << 8) - 1;
            dstStride = width;
            CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT16, CONVERT_FIXED_TO_FIXED);
            break;
        default : break;
        }
        break;

    case gcvSURF_D24S8:
        srcShift = (format == GL_DEPTH_COMPONENT) ? 8 : 0;
        srcMax = (format == GL_DEPTH_COMPONENT) ? ((1 << 24) - 1) : ((1 << 8) - 1);

        switch(type)
        {
        case GL_FLOAT:
            dstMax = 0;
            dstStride = width * 4;
            CONVERT_DEPTH_PIXELS(gctFLOAT, gctUINT32, CONVERT_FIXED_TO_FLOAT);
            break;
        case GL_UNSIGNED_SHORT:
            dstMax = (1 << 16) - 1;
            dstStride = width * 2;
            CONVERT_DEPTH_PIXELS(gctUINT16, gctUINT32, CONVERT_FIXED_TO_FIXED);
            break;
        case GL_UNSIGNED_INT:
            dstStride = width * 4;
            CONVERT_DEPTH_PIXELS(gctUINT32, gctUINT32, CONVERT_NONE);
            break;
        case GL_UNSIGNED_INT_24_8_EXT:
            srcShift = 0;
            srcMax = 0xFFFFFFFF;
            dstStride = width * 4;
            CONVERT_DEPTH_PIXELS(gctUINT32, gctUINT32, CONVERT_NONE);
            break;
        case GL_UNSIGNED_BYTE:
            dstStride = width;
            if (format == GL_STENCIL_INDEX)
            {
                dstMax = 0;
                CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT32, CONVERT_NONE);
            }
            else
            {
                dstMax = (1 << 8) - 1;
                CONVERT_DEPTH_PIXELS(gctUINT8, gctUINT32, CONVERT_FIXED_TO_FIXED);
            }
            break;
        default : break;
        }
        break;

    default :
        break;
    }

    gcoSURF_Unlock(tmpView.surf, gcvNULL);

OnError:

    if (tmpView.surf != gcvNULL)
    {
        gcoSURF_Destroy(tmpView.surf);
    }

    return status;

}
#endif

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
#ifdef OPENGL40
    GLboolean     RGBFloat = GL_FALSE;
    gceSURF_FORMAT      floatFmt;
    float  *bit = gcvNULL;
    float  *temp = gcvNULL;
    GLuint  i,j;
#endif
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
#ifdef OPENGL40
    switch(format)
    {
    case GL_DEPTH_COMPONENT:
    case GL_STENCIL_INDEX:
    case GL_DEPTH_STENCIL_EXT:
        status = __glChipReadDepthStencilPixels(gc, x, y,
                                        width, height,
                                        format, type, buf);
        gcmFOOTER_ARG("return=%d", status);
        return (status == gcvSTATUS_OK);
    default:
        break;
    }

#endif
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
        switch (format)
        {
        case GL_RGBA:
            wrapformat = gcvSURF_A8B8G8R8;
            break;
        case GL_BGRA_EXT:
            wrapformat = gcvSURF_A8R8G8B8;
            break;
#ifdef OPENGL40
        case GL_RED:
            wrapformat = gcvSURF_R8;
            break;
        case GL_ALPHA:
            wrapformat = gcvSURF_A8;
            break;
        case GL_LUMINANCE:
            wrapformat = gcvSURF_L8;
            break;
        case GL_LUMINANCE_ALPHA:
            wrapformat = gcvSURF_A8L8;
            break;
#endif
        default:
            break;
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
#ifdef OPENGL40
        else if (format == GL_RGB)
        {
            RGBFloat = GL_TRUE;
            floatFmt = gcvSURF_A32B32G32R32F;
        }
#endif
        break;

#ifdef OPENGL40
    case GL_UNSIGNED_SHORT:
        switch (format)
        {
        case GL_RGBA_INTEGER:
            wrapformat = gcvSURF_A16B16G16R16UI;
            break;
        case GL_RED_INTEGER:
            wrapformat = gcvSURF_R16UI;
            break;
        case GL_RED:
            wrapformat = gcvSURF_R16;
            break;
        case GL_ALPHA:
            wrapformat = gcvSURF_A16;
            break;
        case GL_LUMINANCE:
            wrapformat = gcvSURF_L16;
            break;
        case GL_LUMINANCE_ALPHA:
            wrapformat = gcvSURF_A16L16;
            break;
        default:
            break;
        }
        break;
#endif

    case GL_UNSIGNED_INT:
        switch (format)
        {
        case GL_RGBA_INTEGER:
            wrapformat = gcvSURF_A32B32G32R32UI;
            break;
#ifdef OPENGL40
        case GL_RED_INTEGER:
            wrapformat = gcvSURF_R32UI;
            break;
        case GL_RED:
            wrapformat = gcvSURF_R32;
            break;
        case GL_ALPHA:
            wrapformat = gcvSURF_A32;
            break;
        case GL_LUMINANCE:
            wrapformat = gcvSURF_L32;
            break;
#endif
        default:
            break;
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

#ifdef OPENGL40
    case GL_UNSIGNED_INT_8_8_8_8_REV:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A8B8G8R8;
        }
        else if (format == GL_BGRA)
        {
            wrapformat = gcvSURF_A8R8G8B8;
        }
        break;

    case GL_UNSIGNED_INT_8_8_8_8:
        if (format == GL_BGRA)
        {
            wrapformat = gcvSURF_B8G8R8A8;
        }
        else if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_R8G8B8A8;
        }
        break;

    case GL_UNSIGNED_SHORT_5_6_5:
        gcmASSERT(format == GL_RGB);
        wrapformat = gcvSURF_B5G6R5;
        break;
#endif

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

#ifdef OPENGL40
    if (RGBFloat)
    {
        /* Allocate a new surface. */
        gcmONERROR(gcoSURF_Construct(
            chipCtx->hal,
            width, height, 1,
            gcvSURF_BITMAP, floatFmt,
            gcvPOOL_UNIFIED,
            &dstView.surf
            ));

        /* Get the pointer to the bits. */
        gcmONERROR(gcoSURF_Lock(
            dstView.surf, gcvNULL, (gctPOINTER*)&bit
            ));
    }
    else
#endif
    {
        /* Create the wrapper surface. */
        gcmONERROR(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_BITMAP,
                                     wrapformat, gcvPOOL_USER, &dstView.surf));
        gcmONERROR(gcoSURF_ResetSurWH(dstView.surf, width, height, lineLength, imageHeight, wrapformat));
        gcmONERROR(gcoSURF_WrapSurface(dstView.surf, ps->packModes.alignment, logicalAddress, physicalAddress));
    }

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

#ifdef OPENGL40
    if (RGBFloat)
    {
        temp = logicalAddress;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                memcpy(temp, bit, 3 * sizeof(float));
                temp += 3;
                bit += 4;
            }
            temp += (lineLength - width) * 3;
        }
    }
#endif

OnError:
    if (packBufInfo && gcvINVALID_ADDRESS != physicalAddress) /* The image is from pack buffer object */
    {
        /* CPU cache will not be flushed in HAL, bc HAL only see wrapped user pool surface.
        ** Instead it will be flushed when unlock the packed buffer as non-user pool node.
        */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(packBufInfo->bufObj));
        gcmVERIFY_OK(gcoBUFOBJ_CPUCacheOperation(packBufInfo->bufObj, gcvCACHE_CLEAN));
    }

#ifdef OPENGL40
    if (bit)
    {
        gcoSURF_Unlock(dstView.surf, bit);
    }
#endif

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


