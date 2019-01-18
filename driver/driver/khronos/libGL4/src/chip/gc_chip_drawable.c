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
#include "gc_es_device.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    __GLES3_ZONE_TRACE

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
#include "viv_lock.h"
extern __GLformatInfo* __glGetFormatInfo(GLenum internalFormat);
extern GLboolean  createRenderBuffer(__GLcontext *gc, glsCHIPBUFFERCREATE * chipCreateInfo, gcoSURF *retSurf);
extern GLvoid initAccumOperationPatch(__GLcontext* gc);
extern gceSTATUS __glChipDestroyRenderBuffer(glCHIPBUFFERDESSTROY* chipDestroyInfo);
#endif

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/

__GL_INLINE gceSTATUS
gcChipPickDrawBuffersForDrawable(
    __GLcontext *gc
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    gcsSURF_VIEW         rtViews[__GL_MAX_DRAW_BUFFERS];
    gcsSURF_VIEW         dView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         sView = {gcvNULL, 0, 1};
    GLint                sBpp = 0;
    GLboolean            yInverted = GL_FALSE;
    GLuint               samples = 0;
    gceSTATUS            status = gcvSTATUS_OK;
    GLuint               i;
    GLuint rtNum = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        /* Init rtViews array to nullView first: dView now is nullView */
        rtViews[i] = dView;
    }

    if (drawable)
    {
        gcsRECT clearRect = {0};
        gctBOOL fullClear = gcvFALSE;

    if (!gc->imports.fromEGL )
    {
        gctBOOL bDrawToFront = gcvFALSE;
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            switch (gc->state.raster.drawBuffers[i])
            {
            case GL_FRONT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX];
                if (drawable->modes.stereoMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                }
                bDrawToFront  = GL_TRUE;
                break;

            case GL_FRONT_LEFT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX];
                bDrawToFront  = GL_TRUE;
                break;

            case GL_FRONT_RIGHT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                bDrawToFront  = GL_TRUE;
                break;

            case GL_BACK:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX];
                if (drawable->modes.stereoMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                }
                break;

            case GL_BACK_LEFT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX];
                break;

            case GL_BACK_RIGHT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                break;

            case GL_LEFT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX];
                if (gc->modes.doubleBufferMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX];
                }
                bDrawToFront  = GL_TRUE;
                break;

            case GL_RIGHT:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                if (gc->modes.doubleBufferMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                }
                bDrawToFront  = GL_TRUE;
                break;

            case GL_FRONT_AND_BACK:
                rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX];
                if (drawable->modes.stereoMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                }
                if (gc->modes.doubleBufferMode)
                {
                    rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX];
                    if (drawable->modes.stereoMode)
                    {
                        rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                    }
                }
                bDrawToFront  = GL_TRUE;
                break;

            }
        }

        if (bDrawToFront)
        {
            gc->flags |= __GL_CONTEXT_DRAW_TO_FRONT;
        }
        else
        {
            gc->flags &= ~__GL_CONTEXT_DRAW_TO_FRONT;
        }
    }
    else
    {
        rtViews[rtNum++].surf = (gcoSURF)drawable->rtHandles[0];
    }

        if (gc->flags & __GL_CONTEXT_SKIP_PRESERVE_CLEAR_RECT)
        {
            /* Assume all RT should have same dimension */
            if (gc->state.enables.scissorTest)
            {
                GLint width, height;
                __GLscissor *pScissor = &gc->state.scissor;
                gcmVERIFY_OK(gcoSURF_GetSize(rtViews[0].surf, (gctUINT*)&width, (gctUINT*)&height, gcvNULL));

                clearRect.left   = __GL_MIN(__GL_MAX(0, pScissor->scissorX), width);
                clearRect.top    = __GL_MIN(__GL_MAX(0, pScissor->scissorY), height);
                clearRect.right  = __GL_MIN(__GL_MAX(0, pScissor->scissorX + pScissor->scissorWidth), width);
                clearRect.bottom = __GL_MIN(__GL_MAX(0, pScissor->scissorY + pScissor->scissorHeight), height);

                if ((clearRect.left   == 0) &&
                    (clearRect.top    == 0) &&
                    (clearRect.right  == (gctINT) width) &&
                    (clearRect.bottom == (gctINT) height))
                {
                    fullClear = gcvTRUE;
                }
            }
            else
            {
                fullClear = gcvTRUE;
            }
        }

        for (i = 0; i < rtNum; ++i)
        {
            gcoSURF thisRT = rtViews[i].surf;
            gcoSURF prevRT = (gcoSURF)drawable->prevRtHandles[i];

            if (thisRT && prevRT &&
                !gcoSURF_QueryFlags(thisRT, gcvSURF_FLAG_CONTENT_UPDATED) &&
                gcoSURF_QueryFlags(thisRT, gcvSURF_FLAG_CONTENT_PRESERVED))
            {
                if (!fullClear)
                {
                    gcmVERIFY_OK(gcoSURF_Preserve(prevRT, thisRT, &clearRect));
                }

                gcmVERIFY_OK(gcoSURF_SetFlags(thisRT, gcvSURF_FLAG_CONTENT_PRESERVED, gcvFALSE));
            }
        }

        dView.surf      = (gcoSURF)drawable->depthHandle;
        sView.surf      = (gcoSURF)drawable->stencilHandle;

        if (rtViews[0].surf)
        {
            yInverted = (gcoSURF_QueryFlags(rtViews[0].surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(rtViews[0].surf, &samples));
        }
        else if (dView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(dView.surf, &samples));
        }
        else if (sView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(dView.surf, &samples));
        }

        if (sView.surf)
        {
            sBpp = drawable->dsFormatInfo->stencilSize;
        }
    }

    if (chipCtx->drawStencilMask != (GLuint) ((1 << sBpp) - 1))
    {
        chipCtx->drawStencilMask = (1 << sBpp) - 1;
        chipCtx->chipDirty.uDefer.sDefer.stencilRef = 1;
    }

    gc->state.raster.mrtEnable = (rtNum > 1);

    gcmONERROR(gcChipSetDrawBuffers(gc,
                                    0,
                                    (GLboolean)gc->modes.rgbFloatMode,
                                    rtViews,
                                    &dView,
                                    &sView,
                                    yInverted,
                                    samples,
                                    GL_FALSE,
                                    0,
                                    0,
                                    GL_FALSE,
                                    1));

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipPickReadBufferForDrawable(
    __GLcontext *gc
    )
{
    __GLdrawablePrivate *readable = gc->readablePrivate;
    gcsSURF_VIEW         rtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         dView  = {gcvNULL, 0, 1};
    gcsSURF_VIEW         sView  = {gcvNULL, 0, 1};
    GLboolean            yInverted = GL_FALSE;
    gceSTATUS            status = gcvSTATUS_OK;
    GLuint               readIdx = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (readable)
    {
#ifdef OPENGL40
        switch (gc->state.pixel.readBuffer)
        {
        case GL_FRONT:
        case GL_LEFT:
        case GL_FRONT_LEFT:
        case GL_FRONT_AND_BACK:
            readIdx = __GL_DRAWBUFFER_FRONTLEFT_INDEX;
            break;

        case GL_RIGHT:
        case GL_FRONT_RIGHT:
            readIdx = __GL_DRAWBUFFER_FRONTRIGHT_INDEX;
            break;

        case GL_BACK_LEFT:
        case GL_BACK:
            readIdx = __GL_DRAWBUFFER_BACKLEFT_INDEX;
            break;

        case GL_BACK_RIGHT:
            readIdx = __GL_DRAWBUFFER_BACKRIGHT_INDEX;
            break;

        default:
            break;
        }
#endif
        rtView.surf = (gcoSURF)readable->rtHandles[readIdx];
        dView.surf  = (gcoSURF)readable->depthHandle;
        sView.surf  = (gcoSURF)readable->stencilHandle;

#if gcdENABLE_BLIT_BUFFER_PRESERVE
        if (rtView.surf && readable->prevRtHandles[readIdx] &&
            !gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_UPDATED) &&
            gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_PRESERVED))
        {
            gcoSURF prev = (gcoSURF)readable->prevRtHandles[readIdx];
            gcmVERIFY_OK(gcoSURF_Preserve(prev, rtView.surf, gcvNULL));
            gcmVERIFY_OK(gcoSURF_SetFlags(rtView.surf, gcvSURF_FLAG_CONTENT_PRESERVED, gcvFALSE));
        }
#endif

        if (rtView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
        else if (dView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
        else if (sView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(sView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
    }

    /*
    **  Set read buffer states
    */
    gcmONERROR(gcChipSetReadBuffers(gc, 0, &rtView, &dView, &sView, yInverted, GL_FALSE));


OnError:
    gcmFOOTER();
    return status;
}

/***************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                   */
/***************************************************************************/

GLboolean
__glChipChangeDrawBuffers(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);
    if (!gc->frameBuffer.drawFramebufObj->name)
    {
        gcmONERROR(gcChipPickDrawBuffersForDrawable(gc));
    }
    else
    {
        gcmONERROR(gcChipPickDrawBufferForFBO(gc));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipChangeReadBuffers(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x", gc);
    if (!gc->frameBuffer.readFramebufObj->name)
    {
        gcmONERROR(gcChipPickReadBufferForDrawable(gc));
    }
    else
    {
        gcmONERROR(gcChipPickReadBufferForFBO(gc));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLvoid
__glChipDetachDrawable(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    __GLdrawablePrivate *readable = gc->readablePrivate;
    gcoSURF surfList[__GL_MAX_DRAW_BUFFERS * 2 + 4];
    GLuint surfCount = 0;
    GLuint i;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (drawable)
    {
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (drawable->rtHandles[i])
            {
                surfList[surfCount++] = (gcoSURF)drawable->rtHandles[i];
            }
        }
        if (drawable->depthHandle)
        {
            surfList[surfCount++] = (gcoSURF)drawable->depthHandle;
        }
        if (drawable->stencilHandle)
        {
            surfList[surfCount++] = (gcoSURF)drawable->stencilHandle;
        }
    }

    if (readable)
    {
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (readable->rtHandles[i])
            {
                surfList[surfCount++] = (gcoSURF)readable->rtHandles[i];
            }
        }
        if (readable->depthHandle)
        {
            surfList[surfCount++] = (gcoSURF)readable->depthHandle;
        }
        if (readable->stencilHandle)
        {
            surfList[surfCount++] = (gcoSURF)readable->stencilHandle;
        }
    }

    GL_ASSERT(surfCount <= __GL_CHIP_SURF_COUNT);

    if (surfCount)
    {
        gcChipDetachSurface(gc, chipCtx, surfList, surfCount);
    }

    gcmFOOTER_NO();
    return;
}


GLboolean
__glChipUpdateDrawable(
    __GLdrawablePrivate *drawable
    )
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    __GLchipDrawable *chipDrawable = (__GLchipDrawable*)drawable->privateData;
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean ret;

    gcmHEADER_ARG("drawable=%p", drawable);

    /* Get PatchID from HAL in the very beginning */
    gcmONERROR(gcoHAL_GetPatchID(gcvNULL, &patchId));

    if (!chipDrawable)
    {
        /* Allocate the array for the vertex attributes. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(__GLchipDrawable),
                                  (gctPOINTER*)&chipDrawable));
        gcoOS_ZeroMemory(chipDrawable, gcmSIZEOF(__GLchipDrawable));
        drawable->privateData = chipDrawable;
    }

    /* Only enable stencil opt for those conformance tests */
    if (patchId == gcvPATCH_GTFES30 || patchId == gcvPATCH_DEQP)
    {
        GLint stencilSize = drawable->dsFormatInfo ? (gctSIZE_T)drawable->dsFormatInfo->stencilSize : 0;

        if (stencilSize > 0)
        {
            if (!chipDrawable->stencilOpt)
            {
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          gcmSIZEOF(__GLchipStencilOpt),
                                          (gctPOINTER*)&chipDrawable->stencilOpt));
            }

            gcChipPatchStencilOptReset(chipDrawable->stencilOpt,
                                       (gctSIZE_T)drawable->width,
                                       (gctSIZE_T)drawable->height,
                                       (gctSIZE_T)stencilSize);
        }
        else
        {
            if (chipDrawable->stencilOpt)
            {
                gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable->stencilOpt));
                chipDrawable->stencilOpt = gcvNULL;
            }
        }
    }

OnError:
    ret = gcmIS_ERROR(status) ? GL_FALSE : GL_TRUE;
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLvoid
__glChipDestroyDrawable(
    __GLdrawablePrivate *drawable
    )
{
    __GLchipDrawable *chipDrawable = (__GLchipDrawable*)drawable->privateData;
    gcmHEADER_ARG("drawable=0x%x", drawable);

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
    if (drawable->dp.privateData) {
        (*drawable->free)(drawable->dp.privateData);
    }

    drawable->dp.privateData = NULL;
#endif

    if (chipDrawable)
    {
        if (chipDrawable->stencilOpt)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable->stencilOpt));
            chipDrawable->stencilOpt = gcvNULL;
        }

        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable));
        drawable->privateData = gcvNULL;
    }
    gcmFOOTER_NO();
}

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
GLvoid __glChipCreatePbuffer(__GLcontext *gc, __GLdrawablePrivate * draw)
{

}

GLvoid updateDrawableBufferInfo(__GLdrawablePrivate *draw, __GLdrawableBuffer *buf, GLuint depth)
{
    buf->width = draw->width;
    buf->height = draw->height;
    buf->depth = depth;
    buf->elementSize = depth / 8;
}

/* The code is still being modified and can't be compiled currectly */
GLboolean initDrawable(glsCHIPDRAWABLE *chipDrawable, __GLdrawablePrivate *draw)
{
    GLuint i;
    gceSTATUS status;

    gcmHEADER_ARG("chipDrawable=0x%x, draw=0x%x", chipDrawable, draw);

    gcmVERIFY_ARGUMENT(chipDrawable != gcvNULL);
    gcmVERIFY_ARGUMENT(draw != gcvNULL);

    __GL_MEMZERO(chipDrawable, sizeof(glsCHIPDRAWABLE));

    chipDrawable->width  = draw->width;
    chipDrawable->height = draw->height;
    chipDrawable->internalFormatColorBuffer = draw->internalFormatColorBuffer;

    chipDrawable->haveRenderBuffer = GL_TRUE;

    chipDrawable->originalFrontBuffer  = (glsCHIPRENDERBUFFER*)(draw->frontBuffer.privateData);
    chipDrawable->originalFrontBuffer1 = (glsCHIPRENDERBUFFER*)(draw->frontBuffer2.privateData);

    chipDrawable->drawBuffers[__GL_FRONT_BUFFER_INDEX] = (glsCHIPRENDERBUFFER**)(&draw->frontBuffer.privateData);

    for(i = 1; i <__GL_MAX_DRAW_BUFFERS; i++ )
    {
        chipDrawable->drawBuffers[i] = (glsCHIPRENDERBUFFER**)(&draw->drawBuffers[i].privateData);
    }

    chipDrawable->resolveBuffer = (glsCHIPRENDERBUFFER*)draw->backBuffer[__GL_RESOLVE_BUFFER].privateData;

    if(draw->modes.haveDepthBuffer)
    {
       chipDrawable->haveDepthBuffer = GL_TRUE;
       chipDrawable->depthBuffer = (glsCHIPDEPTHBUFFER*)(draw->depthBuffer.privateData);
    }

    if(draw->modes.haveStencilBuffer)
    {
       chipDrawable->haveStencilBuffer = GL_TRUE;
       chipDrawable->stencilBuffer = (glsCHIPSTENCILBUFFER*)(draw->stencilBuffer.privateData);
    }

    if(draw->modes.haveAccumBuffer)
    {
       chipDrawable->haveAccumBuffer = GL_TRUE;
       chipDrawable->accumBuffer = (glsCHIPACCUMBUFFER*)(draw->accumBuffer.privateData);
    }

    if (draw->backBuffer[__GL_RESOLVE_BUFFER].privateData) {
        do {
            gctUINT height, alignedHeight, stride;
            glsCHIPRENDERBUFFER * chipRenderBuffer = (glsCHIPRENDERBUFFER *)draw->backBuffer[__GL_RESOLVE_BUFFER].privateData;
            gctPOINTER memoryResolve[3] = {gcvNULL};


            gcmERR_BREAK(gcoSURF_GetSize(chipRenderBuffer->renderTarget, NULL, &height, NULL));
            gcmERR_BREAK(gcoSURF_GetAlignedSize(chipRenderBuffer->renderTarget, NULL, &alignedHeight, (gctINT *)&stride));
            /* Lock the surface. */
            gcmERR_BREAK(gcoSURF_Lock(chipRenderBuffer->renderTarget, gcvNULL, memoryResolve));
            chipRenderBuffer->resolveBits = memoryResolve[0];


            /* Set the swap surface. */
            chipDrawable->swapInfo.swapSurface = chipRenderBuffer->renderTarget;
            gcmERR_BREAK(gcoOS_CreateMutex(gcvNULL, &chipDrawable->workerMutex));
            chipDrawable->swapInfo.swapBits = (gctPOINTER)((gctUINT8_PTR)chipRenderBuffer->resolveBits + (alignedHeight - height) * stride);
            chipDrawable->swapInfo.bitsAlignedWidth = chipRenderBuffer->alignedWidth;
            chipDrawable->swapInfo.bitsAlignedHeight = chipRenderBuffer->alignedHeight;
            chipDrawable->swapInfo.swapBitsPerPixel = draw->modes.rgbaBits;
        } while (GL_FALSE);

    }
    gcmFOOTER_NO();
    return GL_TRUE;
}


GLvoid deInitDrawable(glsCHIPDRAWABLE* chipDrawable)
{
    GL_ASSERT(chipDrawable);
    __GL_MEMZERO(chipDrawable, sizeof(glsCHIPDRAWABLE));
}

void __glChipUpdateDrawableInfo(__GLdrawablePrivate* draw)
{
    GLuint i;

    /* Update all draw buffers */
    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++) {
        updateDrawableBufferInfo(draw, &draw->drawBuffers[i], draw->modes.rgbaBits);
    }

    updateDrawableBufferInfo(draw, &draw->frontBuffer2, draw->modes.rgbaBits);
    updateDrawableBufferInfo(draw, &draw->backBuffer[__GL_RESOLVE_BUFFER], draw->modes.rgbaBits);

    if(draw->modes.haveDepthBuffer)
    {
        updateDrawableBufferInfo(draw, &draw->depthBuffer, draw->modes.depthBits);
        switch (draw->modes.depthBits)
        {
            case 16:
                break;
            case 24:
                draw->depthBuffer.elementSize = 4;
                break;
            case 32:
                break;
        }
    }

    if(draw->modes.haveStencilBuffer)
    {
        updateDrawableBufferInfo(draw, &draw->stencilBuffer, draw->modes.stencilBits);
        switch (draw->modes.stencilBits)
        {
            case 8:
                break;
            default:
                GL_ASSERT(0);
                break;
        }
    }

    if(draw->modes.haveAccumBuffer)
    {
        updateDrawableBufferInfo(draw, &draw->accumBuffer, draw->modes.accumBits);
    }

}

GLvoid notifyChangeBufferSizePBuffer(__GLcontext * gc)
{
/*
    __GLchipContext *chipCtx = (__GLchipContext *)(gc->dp.ctx.privateData);
*/
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    /* If the drawable already have the required width and height, just return */
    if(draw->width != chipDraw->width || draw->height != chipDraw->height)
    {

/*
        detachDrawable(chipCtx, draw);
*/
        __glChipDetachDrawable(gc);

        /*Flush before destroy,since maybe command buffer still refer to these resources*/

        /* Free all video memory */
        if (draw->dp.freeBuffers)
            draw->dp.freeBuffers(draw, GL_TRUE);

        if(draw->width != 0 && draw->height != 0)
        {
            __glChipCreatePbuffer(gc, draw);
        }
    }
}

GLvoid notifyChangeBufferSizeDrawable(__GLcontext * gc)
{
    __GLchipContext *chipCtx = (__GLchipContext *)(gc->dp.ctx.privateData);
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
    glsCHIPBUFFERCREATE chipCreateInfo;
    __GLformatInfo *pformatInfo = gcvNULL;
    GLboolean retValue = GL_TRUE;

    gcoSURF stencilSurf = gcvNULL;
    gcoSURF accumSurf = gcvNULL;
    gcoSURF nullSurf = gcvNULL;


    gcmHEADER_ARG("gc=0x%x", gc);

    /* If the drawable already have the required width and height, just return */
    /* __GL_DRAWABLE_PENDING_PRIMARY_LOST means that the primary surface handle was invalidated because of a display mode change. If
       the OpenGL installable client driver (ICD) receives this error code, it should reopen or recreate the primary handle, replace
       all references in the command buffer to the old handle with the new handle, and then resubmit the buffer. */
    if(draw->width != chipDraw->width || draw->height != chipDraw->height || draw->internalFormatColorBuffer != chipDraw->internalFormatColorBuffer || (gc->changeMask & __GL_DRAWABLE_PENDING_PRIMARY_LOST))
    {
/*
        detachDrawable(chipCtx, draw);
*/
        __glChipDetachDrawable(gc);
        /* Flush the cache. */
        if (chipCtx->drawRT[0] != gcvNULL)
        {
            gcoSURF_Flush(chipCtx->drawRT[0]);
            /* Commit command buffer. */
            gcoHAL_Commit(chipCtx->hal, gcvTRUE);
        }

        /*Flush before destroy,since maybe command buffer still refer to these resources*/

        if(draw->dp.freeBuffers) {
            draw->dp.freeBuffers(draw, GL_TRUE);
        }

        if (draw->width != 0 && draw->height != 0)
        {
            __GL_MEMZERO(&chipCreateInfo, sizeof(glsCHIPBUFFERCREATE));

            /* set multisample parameters */
            pformatInfo = __glGetFormatInfo(draw->internalFormatDisplayBuffer);
            /*frontleftbuffer: create front buffer(default) */
            draw->frontBuffer.deviceFormatInfo = pformatInfo;
            chipCreateInfo.flags = __GL_FRONT_BUFFER;
            chipCreateInfo.bufInfo = (GLvoid *)&draw->frontBuffer;
            chipCreateInfo.subFlags = 0;
            chipCreateInfo.poolType = gcvPOOL_DEFAULT;
            chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;

            retValue = createRenderBuffer(gc, &chipCreateInfo, (gcoSURF*)&draw->rtHandles[__GL_DRAWBUFFER_FRONTLEFT_INDEX]);

            /* set multisample parameters */
            chipCreateInfo.sampleBuffers = draw->modes.sampleBuffers;
            chipCreateInfo.samples = draw->modes.samples;
            /*frontrightbuffer: create the frontright buffer*/
            if ((draw->modes.stereoMode) && (retValue))
            {
                draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX].deviceFormatInfo = pformatInfo;
                chipCreateInfo.flags = __GL_FRONT_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                retValue = createRenderBuffer(gc, &chipCreateInfo, (gcoSURF*)&draw->rtHandles[__GL_DRAWBUFFER_FRONTRIGHT_INDEX]);
            }

            if((draw->modes.doubleBufferMode) && (retValue))
            {
                /*backleftbuffer: create the first back buffer*/
                draw->backBuffer[__GL_BACK_BUFFER0].deviceFormatInfo = pformatInfo;
                chipCreateInfo.flags = __GL_BACK_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_BACK_BUFFER0];
                retValue = createRenderBuffer(gc, &chipCreateInfo, (gcoSURF*)&draw->rtHandles[__GL_DRAWBUFFER_BACKLEFT_INDEX]);
                /*backrightbuffer: create the backright buffer*/
                if ((draw->modes.stereoMode) && (retValue))
                {
                    draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX].deviceFormatInfo = pformatInfo;
                    chipCreateInfo.flags = __GL_BACK_BUFFER;
                    chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                    retValue = createRenderBuffer(gc, &chipCreateInfo, (gcoSURF*)&draw->rtHandles[__GL_DRAWBUFFER_BACKRIGHT_INDEX]);
                }
            }

            if ((draw->modes.tripleBufferMode)  && (retValue))
            {
                /*Now we should not come here,cause we have no triplebuffer flag*/
                GL_ASSERT(0);
                /*create the second back buffer*/
                draw->backBuffer[__GL_BACK_BUFFER1].deviceFormatInfo = pformatInfo;
                chipCreateInfo.flags = __GL_BACK_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_BACK_BUFFER1];
                retValue = createRenderBuffer(gc, &chipCreateInfo, &nullSurf);
            }

            /*AUXBuffers: create the AUX buffers*/
            GL_ASSERT(draw->modes.numAuxBuffers == 0);

            /* Create resolve buffer */
            if (retValue) {
                pformatInfo = __glGetFormatInfo(draw->internalFormatDisplayBuffer);
                draw->backBuffer[__GL_RESOLVE_BUFFER].deviceFormatInfo = pformatInfo;
                /*
                chipCreateInfo.subFlags = (GLuint)draw->fullScreenMode;
                */
                chipCreateInfo.subFlags = 0;
                chipCreateInfo.flags = __GL_RESOLVE_BUFFER_FLAG;
                chipCreateInfo.poolType = gcvPOOL_UNIFIED;
                chipCreateInfo.surfType = gcvSURF_BITMAP;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_RESOLVE_BUFFER];
                retValue = createRenderBuffer(gc, &chipCreateInfo,  &nullSurf);
            }

            /* depthbuffer: Create depth buffer */
            if ((draw->modes.haveDepthBuffer) && retValue)
            {
                chipCreateInfo.flags = __GL_DEPTH_BUFFER;
                pformatInfo = __glGetFormatInfo(draw->internalFormatDepthBuffer);
                draw->depthBuffer.deviceFormatInfo = pformatInfo;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_DEPTH;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->depthBuffer;
                retValue = createRenderBuffer(gc, &chipCreateInfo, (gcoSURF *)&draw->depthHandle);
            }
            /* stecncilbuffer: Create stencil buffer */
            if ((draw->modes.haveStencilBuffer) && retValue)
            {
                glsCHIPSTENCILBUFFER   *chipStencilBuffer;
                glsCHIPDEPTHBUFFER   *chipDepthBuffer;
                chipCreateInfo.flags = __GL_STENCIL_BUFFER;
                pformatInfo = __glGetFormatInfo(draw->internalFormatStencilBuffer);
                draw->stencilBuffer.deviceFormatInfo = pformatInfo;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->stencilBuffer;
                retValue = createRenderBuffer(gc, &chipCreateInfo,&stencilSurf);
                chipDepthBuffer = draw->depthBuffer.privateData;
                chipStencilBuffer = draw->stencilBuffer.privateData;
                chipStencilBuffer->stencilBuffer = chipDepthBuffer->depthBuffer;
                chipStencilBuffer->stencilFormat = chipDepthBuffer->depthFormat;
                draw->stencilHandle = draw->depthHandle;
            }

            /* accumbuffer: Create accumulator buffer */
            if ((draw->modes.haveAccumBuffer) && retValue)
            {
                pformatInfo = __glGetFormatInfo(draw->internalFormatDisplayBuffer);
                draw->accumBuffer.deviceFormatInfo = pformatInfo;
                chipCreateInfo.flags = __GL_ACCUM_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->accumBuffer;
                createRenderBuffer(gc, &chipCreateInfo, &accumSurf);
                (*gc->dp.createAccumBufferInfo)(gc, accumSurf,draw);
            }

            /* Initial Drawable */
            if (retValue) {
                retValue = initDrawable(chipDraw, draw);
                chipDraw->gc = gc;
            }
        }
    }

    if (retValue)
    {
        if (chipDraw->haveAccumBuffer) {
            /* FIX me ? */
            initAccumOperationPatch(gc);
        }
    }

    if (!retValue) {
        if(draw->dp.freeBuffers) {
            draw->dp.freeBuffers(draw, GL_FALSE);
        }
    }

    if (retValue)
    {
        __glChipUpdateDrawable(draw);
    }
    gcmFOOTER_NO();
    return;
}


void getDrawableBufInfo(__GLdrawablePrivate *draw, __GLdrawableInfo infoType, gcoSURF *pSurf)
{
    glsCHIPDRAWABLE     *chipDrawable = (glsCHIPDRAWABLE*)draw->dp.privateData;
    glsCHIPRENDERBUFFER *chipRenderBuffer = NULL;

    GL_ASSERT(draw);
    GL_ASSERT(pSurf);

    switch(infoType)
    {
    case __GL_DRAW_FRONTBUFFER_ALLOCATION:
        chipRenderBuffer = *(chipDrawable->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
        break;

    case __GL_DRAW_BACKBUFFER0_ALLOCATION:
        chipRenderBuffer = *(chipDrawable->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
        break;

    case __GL_DRAW_BACKBUFFER1_ALLOCATION:
        chipRenderBuffer = *(chipDrawable->drawBuffers[__GL_BACK_BUFFER1_INDEX]);
        break;

    case __GL_DRAW_PRIMARY_ALLOCATION:
        chipRenderBuffer = chipDrawable->originalFrontBuffer;
        break;
    }

    if(chipRenderBuffer)
    {
        *pSurf = chipRenderBuffer->renderTarget;
    }
    else
    {
        *pSurf = gcvNULL;
    }
}

GLvoid exchangeBufferHandles(__GLcontext *gc, __GLdrawablePrivate * draw, GLboolean exchange)
{
    GL_ASSERT(draw);
    /*always exchange the buffers*/
    if (exchange)
    {
        glsCHIPDRAWABLE     *chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
        glsCHIPRENDERBUFFER *pTemp = NULL;
        void *pRtSurf = gcvNULL;

        if(draw->modes.doubleBufferMode)
        {
            pTemp = *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
            pRtSurf = draw->rtHandles[__GL_BACK_BUFFER0_INDEX];

            *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]) = *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
            draw->rtHandles[__GL_BACK_BUFFER0_INDEX] = draw->rtHandles[__GL_FRONT_BUFFER_INDEX];

            *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]) = pTemp;
            draw->rtHandles[__GL_FRONT_BUFFER_INDEX] = pRtSurf;

        }
        else if(draw->modes.tripleBufferMode)
        {
            pTemp = *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
            pRtSurf = draw->rtHandles[__GL_BACK_BUFFER0_INDEX];

            *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]) = *(chipDraw->drawBuffers[__GL_BACK_BUFFER1_INDEX]);
            draw->rtHandles[__GL_BACK_BUFFER0_INDEX] = draw->rtHandles[__GL_BACK_BUFFER1_INDEX];

            *(chipDraw->drawBuffers[__GL_BACK_BUFFER1_INDEX]) = *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
            draw->rtHandles[__GL_BACK_BUFFER1_INDEX] = draw->rtHandles[__GL_FRONT_BUFFER_INDEX];

            *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]) = pTemp;
            draw->rtHandles[__GL_FRONT_BUFFER_INDEX] = pRtSurf;

        }
        __glChipChangeDrawBuffers(gc);
    }
}

GLvoid resolveBuffer(__GLcontext * gc,  GLboolean swapFront)
{

    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;
    __DRIdrawablePrivate *dPriv = pDriMirror->drawable;
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);
    gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);

    if (swapFront) {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]))->renderTarget;
    } else {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]))->renderTarget;
    }


    tgtView.surf = (gcoSURF)dPriv->wrapSurface;
    if ( tgtView.surf == gcvNULL )
        tgtView.surf = chipDraw->resolveBuffer->renderTarget;


    gcoSURF_Flush(rtView.surf);
    /* Commit command buffer. */
    gcoHAL_Commit(chipCtx->hal, gcvFALSE);
    /* If using cached video memory, we should clean cache before using GPU
    to do resolve rectangle. fix Bug3508*/
    gcoSURF_CPUCacheOperation(rtView.surf, gcvCACHE_CLEAN);

    if (tgtView.surf) {
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.rectSize.x = draw->width;
        rlvArgs.uArgs.v2.rectSize.y = draw->height;
        rlvArgs.uArgs.v2.numSlices  = 1;
        gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
    }
}

GLvoid resolveRenderTargetToScreen(__GLcontext * gc)
{
    __GLdrawablePrivate *draw = gc->drawablePrivate;

    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;
    __DRIdrawablePrivate *dPriv = pDriMirror->drawable;

    LINUX_LOCK_FRAMEBUFFER(gc);

    if (!(gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE)) {
        if ( (gcoSURF)dPriv->wrapSurface )
        {
            dPriv->doCPYToSCR(dPriv);
        }
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

/**********************************************************************/
/* Implementation for device drawable APIs                            */
/**********************************************************************/
GLboolean __glChipSwapBuffers(__GLcontext *gc,
    __GLdrawablePrivate *draw,
    GLboolean bSwapFront)
{
    GLboolean retValue = GL_TRUE;

    resolveBuffer(gc, bSwapFront);
    resolveRenderTargetToScreen(gc);
    exchangeBufferHandles(gc, draw, !bSwapFront);

    if (gc->profiler.enable)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_FRAME_END, 0);
    }

    /* Success. */
    return retValue;
}

GLvoid __glChipFreeDrawableBuffers(__GLdrawablePrivate *draw, GLboolean bWaitDrawDone)
{
    glsCHIPREADABLE * chipDrawable;
    glCHIPBUFFERDESSTROY chipDestroyInfo;
    GLuint i;

    gcmHEADER_ARG("draw=0x%x", draw);

    chipDrawable = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    if(draw->type == __GL_PBUFFER)
    {
        GL_ASSERT(draw->pbufferTex);
        if(draw->pbufferTex->renderTexture)
        {
            chipDestroyInfo.flags = __GL_PBUFFERTEX_BUFFER;
        }
        else
        {
            chipDestroyInfo.flags = __GL_BACK_BUFFER;
        }

        for( i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
        {
            chipDestroyInfo.bufInfo = (void*)(&(draw->drawBuffers[i]));
            __glChipDestroyRenderBuffer(&chipDestroyInfo);
            chipDrawable->drawBuffers[i] = NULL;
        }
    }
    else
    {
        for( i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
        {
            /* for 0,1 are frontbuffer;others are backbuffer*/
            chipDestroyInfo.flags = (i < 2) ? __GL_FRONT_BUFFER : __GL_BACK_BUFFER;
            chipDestroyInfo.bufInfo = (void*)(&(draw->drawBuffers[i]));
            __glChipDestroyRenderBuffer(&chipDestroyInfo);
            chipDrawable->drawBuffers[i] = NULL;
        }
    }

    /* Free desondary front for SAMM */
    chipDestroyInfo.bufInfo = (void*)(&(draw->frontBuffer2));
    chipDestroyInfo.flags = __GL_FRONT_BUFFER;
    __glChipDestroyRenderBuffer(&chipDestroyInfo);

    /* Free fake font buffer */
    chipDestroyInfo.bufInfo = (void*)(&(draw->backBuffer[__GL_RESOLVE_BUFFER]));
    chipDestroyInfo.flags = __GL_BACK_BUFFER;
    __glChipDestroyRenderBuffer(&chipDestroyInfo);

    /* Free depth buffer */
    if(draw->modes.haveDepthBuffer)
    {
        chipDestroyInfo.bufInfo = (void*)(&(draw->depthBuffer));
        chipDestroyInfo.flags = __GL_DEPTH_BUFFER;
        __glChipDestroyRenderBuffer(&chipDestroyInfo);
    }

    if(draw->modes.haveStencilBuffer)
    {
        chipDestroyInfo.bufInfo = (void*)(&(draw->stencilBuffer));
        chipDestroyInfo.flags = __GL_STENCIL_BUFFER;
        __glChipDestroyRenderBuffer(&chipDestroyInfo);
    }

    /* Free video memory for accumulator buffer */
    if(draw->modes.haveAccumBuffer)
    {
        chipDestroyInfo.bufInfo = (void*)(&(draw->accumBuffer));
        chipDestroyInfo.flags = __GL_ACCUM_BUFFER;
        __glChipDestroyRenderBuffer(&chipDestroyInfo);
    }

    chipDrawable = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    deInitDrawable(chipDrawable);
    if ( draw->gc != gcvNULL )
    __glChipDetachDrawable(draw->gc);
    gcmFOOTER_NO();
    return;
}


GLvoid __glChipNotifyChangeBufferSize(__GLcontext * gc)
{
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    GL_ASSERT(draw && chipDraw);

    if (draw->type == __GL_PBUFFER)
        notifyChangeBufferSizePBuffer(gc);
    else
        notifyChangeBufferSizeDrawable(gc);

    __glChipChangeDrawBuffers(gc);
    __glChipChangeReadBuffers(gc);


}


GLvoid __glChipNotifyDestroyBuffers(__GLcontext *gc)
{
    // Do nothing
}

GLvoid __glChipNotifySwapBuffers(__GLcontext *gc)
{
    if(gc->flags & __GL_FULL_SCREEN)
    {
        /* Update render target after flip */
    __glChipChangeDrawBuffers(gc);
    __glChipChangeReadBuffers(gc);
    }
}


GLvoid __glChipNotifyDrawableSwitch(__GLcontext *gc)
{
    __glChipChangeDrawBuffers(gc);
    __glChipChangeReadBuffers(gc);
}

void __glChipRestoreFrontBuffer(__GLdrawablePrivate *draw)
{
    if(draw->flipOn)
    {
    }
}

void __glChipClearShareData(__GLdrawablePrivate *draw)
{

}

void __glChipAddSwapHintRectWIN(__GLdrawablePrivate* draw, RECT* rect, GLuint count)
{

}

void __glChipClearSwapHintRectWIN(__GLdrawablePrivate* draw)
{

}

void __glChipBindRenderBuffer(__GLdrawablePrivate* draw, __GLdrawableBuffer* drawBuffer)
{
}

GLboolean __glChipSetDisplayMode(__GLcontext *gc)
{
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF     pSurf;

    if (draw == NULL )
    {
        GL_ASSERT(0);
        return GL_FALSE;
    }

    getDrawableBufInfo(draw, __GL_DRAW_FRONTBUFFER_ALLOCATION, &pSurf);

    if(!pSurf)
    {
        return GL_FALSE; /*Surface is not residented*/
    }

    //status = __glDpSetDisplayMode(chipCtx, pSurf);

    if(status != gcvSTATUS_OK)
    {
        GL_ASSERT(0);
        return GL_FALSE;
    }

    if(draw->modes.doubleBufferMode)
    {
        getDrawableBufInfo(draw, __GL_DRAW_BACKBUFFER0_ALLOCATION, &pSurf);

        if(!pSurf)
        {
            return GL_FALSE; /*Surface is not residented*/
        }

        //status = __glDpSetDisplayMode(chipCtx, pSurf);
    }
    else if(draw->modes.tripleBufferMode)
    {
        getDrawableBufInfo(draw, __GL_DRAW_BACKBUFFER0_ALLOCATION, &pSurf);

        if(!pSurf)
        {
            return GL_FALSE; /*Surface is not residented*/
        }

        //status = __glDpSetDisplayMode(chipCtx, pSurf);

        if(status != gcvSTATUS_OK)
        {
            GL_ASSERT(0);
            return GL_FALSE;
        }

        getDrawableBufInfo(draw, __GL_DRAW_BACKBUFFER1_ALLOCATION,&pSurf);
        if(!pSurf)
        {
            return GL_FALSE; /*Surface is not residented*/
        }
        //status = __glDpSetDisplayMode(chipCtx, pSurf);
    }

    if(status != gcvSTATUS_OK)
    {
        GL_ASSERT(0);
        return GL_FALSE;
    }

    /* Now flip is really enabled */
    gc->flags |= __GL_FULL_SCREEN;
    draw->flipOn = GL_TRUE;

    return GL_TRUE;
}

GLboolean __glChipSetExclusiveDisplay(GLboolean bSet)
{
    return GL_TRUE;
}

GLvoid __glChipNotifyChangeExclusiveMode(__GLcontext * gc)
{
    __GLdrawablePrivate *draw = gc->drawablePrivate;

    if(draw->fullScreenMode && (draw->type == __GL_WINDOW) && (draw->bFocus) && (!__glDevice->IsRotated))
    {
        __glChipSetExclusiveDisplay(GL_TRUE);
        __glChipSetDisplayMode(gc);
    }
    else
    {
        __glChipSetExclusiveDisplay(GL_FALSE);
        draw->flipOn = GL_FALSE;
    }
}

void __glChipDeleteRenderBuffer(__GLdrawablePrivate* draw, __GLdrawableBuffer* drawBuffer)
{
    /* draw buffer specific private data have been deleted in __glChipFreeDrawableBuffers */
}

void __glChipNotifyBuffersSwapable(__GLdrawablePrivate* draw)
{
    /* On vista, we don't need resource swap */
}

GLboolean __glChipPresentBuffers(__GLcontext* gc,
            __GLdrawablePrivate* draw,
            GLvoid* hSurface,
            GLboolean bSwapFront,
            GLboolean DWMEnabled,
            ULONGLONG presentToken)
{
    __glChipSwapBuffers(gc, draw, bSwapFront);
    return GL_TRUE;
}

void __glChipCreateDrawable(__GLdrawablePrivate *draw,void *window)
{
    glsCHIPREADABLE * chipDraw;

    chipDraw = (glsCHIPDRAWABLE*)(*draw->calloc)(1, sizeof(glsCHIPDRAWABLE));
    GL_ASSERT(chipDraw);

    draw->dp.privateData = chipDraw;

    /* Initialize  function pointers */
    draw->dp.destroyPrivateData = __glChipDestroyDrawable;
    draw->dp.updateDrawable = __glChipUpdateDrawableInfo;

    draw->dp.freeBuffers = __glChipFreeDrawableBuffers;
    draw->dp.restoreFrontBuffer = __glChipRestoreFrontBuffer;
    draw->dp.clearShareData = __glChipClearShareData;

    draw->dp.addSwapHintRectWIN = __glChipAddSwapHintRectWIN;
    draw->dp.clearSwapHintRectWIN = __glChipClearSwapHintRectWIN;
    draw->dp.bindRenderBuffer = __glChipBindRenderBuffer;
    draw->dp.deleteRenderBuffer = __glChipDeleteRenderBuffer;
    draw->dp.notifyBuffersSwapable = __glChipNotifyBuffersSwapable;

    /*
    **  Original Comment :
    **  Strictly it look not good to put lh specific code in chip specific part,
    **  but this seems make the change as little as possible.  Other solution such as
    **  adding it to glDevice is considered, but the code change is almost the same, but
    **  the drawable->dp call back structure seems better to hold these call backs,
    **  gc->exports also considered, but it's better if we can call it when no gc is valid
    **  (for the application closing up or lose-current case).
    */
    draw->dp.setDisplayMode = __glChipSetDisplayMode;
    draw->dp.setExclusiveDisplay = __glChipSetExclusiveDisplay;
    draw->dp.ExclusiveModeChange = __glChipNotifyChangeExclusiveMode;
    draw->dp.presentBuffers = __glChipPresentBuffers;
    draw->dp.swapBuffers = __glChipSwapBuffers;

}
#endif
