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
#include "gl/gl_device.h"
#include "chip_context.h"

#define _GC_OBJ_ZONE    gcvZONE_API_GL

extern glsDEVICEPIPELINEGLOBAL dpGlobalInfo;
extern const __GLdeviceFormatInfo __glDevfmtInfo[];
extern glsWORKINFO_PTR getWorker(glsCHIPDRAWABLE_PTR chipDrawable);
extern gceSTATUS releaseWorker(glsWORKINFO_PTR Worker);
extern gctBOOL submitWorker(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal,
    glsWORKINFO_PTR Worker,
    gctBOOL ScheduleSignals
    );
extern GLvoid resolveBuffer(__GLcontext * gc, GLboolean swapFront);
#ifdef CPUBLT_TO_FB
extern GLvoid CPUBltBufferToScreen(__GLcontext * gc, GLbyte * resolveLogicalAddress);
#endif

extern GLvoid setDrawBuffers(glsCHIPCONTEXT_PTR chipCtx, GLboolean invertY, GLboolean integerRT, GLboolean floatRT, gcoSURF *rtSurfs, gcoSURF dSurf, gcoSURF sSurf);
extern GLvoid setReadBuffers(glsCHIPCONTEXT_PTR chipCtx, GLboolean invertY, GLboolean integerRT, gcoSURF rtSurf, gcoSURF dSurf, gcoSURF sSurf);
extern GLboolean  createRenderBuffer(__GLcontext *gc, glsCHIPBUFFERCREATE * chipCreateInfo);
extern GLvoid initAccumOperationPatch(__GLcontext* gc, glsCHIPDRAWABLE * chipDraw);
extern GLvoid resolveRenderTargetToScreen(__GLcontext * gc);
extern GLvoid validateViewport(__GLcontext *gc);
extern GLvoid validateScissor(__GLcontext *gc);
extern gceSTATUS __glChipDestroyRenderBuffer(glCHIPBUFFERDESSTROY* chipDestroyInfo);

GLboolean __glChipSetDisplayMode(__GLcontext *gc);
GLboolean __glChipSetExclusiveDisplay(GLboolean bSet);

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/

GLvoid pickDrawBuffersForDrawable(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR      chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate     *drawable = gc->drawablePrivate;
    glsCHIPDRAWABLE         *chipDrawable = (glsCHIPDRAWABLE*)(drawable->dp.privateData);
    gcoSURF                 rtSurfs[__GL_MAX_DRAW_BUFFERS];
    gcoSURF                 dSurf = NULL;
    gcoSURF                 sSurf = NULL;
    __GLpBufferTexture      *pbufferTex = drawable->pbufferTex;
    glsCHIPRENDERTEXTURE    *chipRenderTexture;
    GLboolean               invertY, bDrawToFront = GL_FALSE;
    GLuint                  i,rtIndex,NumRtSurf = 0;

    if(!drawable)
        return;

    invertY = drawable->yInverted;

    /*
    **  Pick correct draw buffers according to gc state
    */
    __GL_MEMZERO(rtSurfs, sizeof(rtSurfs));
    if (chipDrawable->haveRenderBuffer)
    {
        for (i = 0 ; i < __GL_MAX_DRAW_BUFFERS; i++)
        {
            switch (gc->state.raster.drawBuffer[i])
            {
            case GL_FRONT_LEFT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTLEFT_INDEX);
                bDrawToFront  = GL_TRUE;
                break;
                /*Here cause the default drawBuffer is GL_FRONT or GL_BACK,so must add estimation about stereoMode*/
            case GL_FRONT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTLEFT_INDEX);
                if (drawable->modes.stereoMode)
                {
                    __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTRIGHT_INDEX);
                }
                bDrawToFront  = GL_TRUE;
                break;

            case GL_FRONT_RIGHT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTRIGHT_INDEX);
                bDrawToFront  = GL_TRUE;
                break;

            case GL_BACK_LEFT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKLEFT_INDEX);
                break;

            case GL_BACK:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKLEFT_INDEX);
                if (drawable->modes.stereoMode)
                {
                    __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKRIGHT_INDEX);
                }
                break;

            case GL_BACK_RIGHT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKRIGHT_INDEX);
                break;

            case GL_FRONT_AND_BACK:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTLEFT_INDEX);
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTRIGHT_INDEX);
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKLEFT_INDEX);
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKRIGHT_INDEX);
                bDrawToFront  = GL_TRUE;
                break;

            case GL_LEFT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTLEFT_INDEX);
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKLEFT_INDEX);
                bDrawToFront  = GL_TRUE;
                break;

            case GL_RIGHT:
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_FRONTRIGHT_INDEX);
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(__GL_DRAWBUFFER_BACKRIGHT_INDEX);
                bDrawToFront  = GL_TRUE;
                break;

            case GL_AUX0:
            case GL_AUX1:
            case GL_AUX2:
            case GL_AUX3:
                rtIndex = gc->state.raster.drawBuffer[i] - GL_AUX0 + __GL_DRAWBUFFER_AUX0_INDEX;
                __GL_SET_RENDERTARGET_FROM_DRAWBUFFER(rtIndex);
                break;

            case GL_NONE:
                break;

            default:
                GL_ASSERT(0);
                break;
            }
        }
        GL_ASSERT(NumRtSurf <= __GL_MAX_DRAW_BUFFERS);

        gc->state.raster.mrtEnable = (NumRtSurf > 1);

        if (bDrawToFront)
        {
            gc->flags |= __GL_DRAW_TO_FRONT;
        }
        else
            gc->flags &= ~__GL_DRAW_TO_FRONT;
        if(drawable->fullScreenMode)
            gc->flags |= __GL_FULL_SCREEN;
        else
            gc->flags &= ~__GL_FULL_SCREEN;

    }

    if(chipDrawable->haveDepthBuffer)
    {
        dSurf = ((glsCHIPDEPTHBUFFER*)(chipDrawable->depthBuffer))->depthBuffer;
    }

    if(chipDrawable->haveStencilBuffer)
    {
        sSurf = ((glsCHIPSTENCILBUFFER*)(chipDrawable->stencilBuffer))->stencilBuffer;
    }

    /*
    **  Set draw buffer states
    */
    setDrawBuffers(chipCtx, invertY, GL_FALSE, (GLboolean)gc->modes.rgbFloatMode, rtSurfs, dSurf, sSurf);
}

GLvoid pickReadBufferForDrawable(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR      chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate     *readable = gc->readablePrivate;
    glsCHIPDRAWABLE         *chipReadable;
    gcoSURF                  rtSurf = NULL;
    gcoSURF                  dSurf  = NULL;
    gcoSURF                  sSurf  = NULL;
    __GLpBufferTexture      *pbufferTex = readable->pbufferTex;
    glsCHIPRENDERTEXTURE    *chipRenderTexture;

    GLboolean               invertY;
    GLuint                  rtIndex = 0;

    if (!readable) {
        return;
    }

    chipReadable = (glsCHIPREADABLE*)(readable->dp.privateData);
    invertY = readable->yInverted;

    /*
    **  Pick correct read buffers according to gc state
    */
    if (chipReadable->haveRenderBuffer)
    {
        switch (gc->state.pixel.readBuffer)
        {
            case GL_FRONT_LEFT:
            case GL_LEFT:
            case GL_FRONT:
                __GL_SET_RENDERTARGET_FROM_READBUFFER(__GL_DRAWBUFFER_FRONTLEFT_INDEX);
                break;

            case GL_BACK_LEFT:
            case GL_BACK:
                __GL_SET_RENDERTARGET_FROM_READBUFFER(__GL_DRAWBUFFER_BACKLEFT_INDEX);
             break;

            case GL_RIGHT:
            case GL_FRONT_RIGHT:
                __GL_SET_RENDERTARGET_FROM_READBUFFER(__GL_DRAWBUFFER_FRONTRIGHT_INDEX);
           break;

            case GL_BACK_RIGHT:
                __GL_SET_RENDERTARGET_FROM_READBUFFER(__GL_DRAWBUFFER_BACKRIGHT_INDEX);
            break;

            case GL_AUX0:
            case GL_AUX1:
            case GL_AUX2:
            case GL_AUX3:
                rtIndex = gc->state.pixel.readBuffer - GL_AUX0 + __GL_DRAWBUFFER_AUX0_INDEX;
                __GL_SET_RENDERTARGET_FROM_READBUFFER(rtIndex);
              break;

            default:
                break;
        }
    }

    if (chipReadable->haveDepthBuffer)
    {
        dSurf = ((glsCHIPDEPTHBUFFER*)(chipReadable->depthBuffer))->depthBuffer;
    }

    if (chipReadable->haveStencilBuffer)
    {
        sSurf = ((glsCHIPSTENCILBUFFER*)(chipReadable->stencilBuffer))->stencilBuffer;
    }

    /*
    **  Set read buffer states
    */
    setReadBuffers(chipCtx, invertY, GL_FALSE, rtSurf, dSurf, sSurf);
}

GLvoid __glChipDrawBuffers(__GLcontext * gc)
{
    if (!gc->frameBuffer.drawFramebufObj->name)
    {
        pickDrawBuffersForDrawable(gc);
    }
    else
    {
        pickDrawBufferForFBO(gc);
    }
}

GLvoid __glChipReadBuffer(__GLcontext * gc)
{
    if (gc->frameBuffer.readFramebufObj->name)
    {
        pickReadBufferForFBO(gc);
    }
    else
    {
        pickReadBufferForDrawable(gc);
    }
}

GLvoid updateDrawableBuffer(__GLdrawablePrivate *draw, __GLdrawableBuffer *buf, GLuint depth)
{
    buf->width = draw->width;
    buf->height = draw->height;
    buf->depth = depth;
    buf->elementSize = depth / 8;
}

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

#if DIRECT_TO_FB
            if (1 || !draw->fullScreenMode) {
#endif
                gcmERR_BREAK(gcoSURF_GetSize(chipRenderBuffer->renderTarget, NULL, &height, NULL));
                gcmERR_BREAK(gcoSURF_GetAlignedSize(chipRenderBuffer->renderTarget, NULL, &alignedHeight, (gctINT *)&stride));
                /* Lock the surface. */
                gcmERR_BREAK(gcoSURF_Lock(chipRenderBuffer->renderTarget, gcvNULL, memoryResolve));
                chipRenderBuffer->resolveBits = memoryResolve[0];
#if DIRECT_TO_FB
            }
#endif

            /* Set the swap surface. */
            chipDrawable->swapInfo.swapSurface = chipRenderBuffer->renderTarget;
            gcmERR_BREAK(gcoOS_CreateMutex(gcvNULL, &chipDrawable->workerMutex));
            chipDrawable->swapInfo.swapBits = (gctPOINTER)((gctUINT8_PTR)chipRenderBuffer->resolveBits + (alignedHeight - height) * stride);
            chipDrawable->swapInfo.bitsAlignedWidth = chipRenderBuffer->alignedWidth;
            chipDrawable->swapInfo.bitsAlignedHeight = chipRenderBuffer->alignedHeight;
            chipDrawable->swapInfo.swapBitsPerPixel = draw->modes.rgbaBits;
        } while (GL_FALSE);

        for (i = 0; i < WORKER_COUNT; i++)
        {
            chipDrawable->workers[i].draw                   = gcvNULL;
            chipDrawable->workers[i].backBuffer.origin.x    = -1;
            chipDrawable->workers[i].backBuffer.origin.y    = -1;
            chipDrawable->workers[i].backBuffer.size.width  = -1;
            chipDrawable->workers[i].backBuffer.size.height = -1;

            chipDrawable->workers[i].next  = chipDrawable->availableWorkers;
            chipDrawable->availableWorkers = &chipDrawable->workers[i];
        }
    }
    gcmFOOTER_NO();
    return GL_TRUE;
}


GLvoid deInitDrawable(glsCHIPDRAWABLE* chipDrawable)
{
    GL_ASSERT(chipDrawable);
    __GL_MEMZERO(chipDrawable, sizeof(glsCHIPDRAWABLE));
}

/********************************************************************
**
**  __glChipDetachDrawable
**
**  Collect all views of a drawable and try to detach them if they are bound to this gc
**  We have a hole if drawable is shared by other gc(e.g. its views are bound to others
**  chipCtx, and we haven't detach them**
**
**  Parameters:      Describe the calling sequence
**  Return Value:    Describe the returning sequence
**
********************************************************************/
GLvoid detachDrawable(glsCHIPCONTEXT_PTR chipCtx, __GLdrawablePrivate* draw)
{
}

GLvoid imageTransfer(glsCHIPCONTEXT_PTR chipCtx,
    gcsRECT_PTR srcRect,
    gcsRECT_PTR dstRect,
    gcoSURF srcSurf,
    gcoSURF dstSurf)
{
}

GLvoid syncFrontBufferToFakeFront(__GLcontext* gc)
{
}

/* Set current RT view for specific RT */
GLvoid setCurrentSurf(__GLpBufferTexture* pbufferTex, glsCHIPRENDERTEXTURE* chipRenderTexture)
{
    GLuint face = 0;
    GLuint level = 0;
    GLuint rtIndex = 0;

    if (pbufferTex->target == GL_TEXTURE_CUBE_MAP)
    {
        face = pbufferTex->face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    }

    if (pbufferTex->mipmap)
    {
        level = pbufferTex->level;
    }

    rtIndex = face * chipRenderTexture->levels + level;
    chipRenderTexture->renderTarget = chipRenderTexture->renderTargets[rtIndex];
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

void resolveResource(__GLcontext *gc, gcoSURF pSurf)
{
}


void __glChipResolveRenderBuffer(__GLcontext * gc, GLboolean bSwapFront)
{
}


GLvoid exchangeBufferHandles(__GLcontext *gc, __GLdrawablePrivate * draw, GLboolean exchange)
{
    GL_ASSERT(draw);
    /*always exchange the buffers*/
    if (exchange)
    {
        glsCHIPDRAWABLE     *chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
        glsCHIPRENDERBUFFER *pTemp = NULL;

        if(draw->modes.doubleBufferMode)
        {
            pTemp                       = *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
            *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX])    = *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
            *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX])    = pTemp;
        }
        else if(draw->modes.tripleBufferMode)
        {
            pTemp                       = *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]);
            *(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX])    = *(chipDraw->drawBuffers[__GL_BACK_BUFFER1_INDEX]);
            *(chipDraw->drawBuffers[__GL_BACK_BUFFER1_INDEX])    = *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]);
            *(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX])    = pTemp;
        }
        __glChipDrawBuffers(gc);
    }
}


GLboolean bltRmToLinearRm(glsCHIPCONTEXT_PTR chipCtx, gcoSURF rmResource, gcoSURF *dstRmResource, __GLdrawablePrivate* draw)
{
    return GL_TRUE;
}


GLvoid notifyChangeBufferSizePBuffer(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR chipCtx = (glsCHIPCONTEXT_PTR)(gc->dp.ctx.privateData);
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    /* If the drawable already have the required width and height, just return */
    if(draw->width != chipDraw->width || draw->height != chipDraw->height)
    {
        detachDrawable(chipCtx, draw);

        /*Flush before destroy,since maybe command buffer still refer to these resources*/

        /* Free all video memory */
        if(draw->dp.freeBuffers)
            draw->dp.freeBuffers(draw, GL_TRUE);

        if(draw->width != 0 && draw->height != 0)
        {
            __glChipCreatePbuffer(gc, draw);
        }
    }
}

GLvoid notifyChangeBufferSizeDrawable(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR chipCtx = (glsCHIPCONTEXT_PTR)(gc->dp.ctx.privateData);
    __GLdrawablePrivate* draw = gc->drawablePrivate;
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
    glsCHIPBUFFERCREATE chipCreateInfo;
    __GLdeviceFormat chosenFormat;
    GLint i;
    GLboolean retValue = GL_TRUE;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* If the drawable already have the required width and height, just return */
    /* __GL_DRAWABLE_PENDING_PRIMARY_LOST means that the primary surface handle was invalidated because of a display mode change. If
       the OpenGL installable client driver (ICD) receives this error code, it should reopen or recreate the primary handle, replace
       all references in the command buffer to the old handle with the new handle, and then resubmit the buffer. */
    if(draw->width != chipDraw->width || draw->height != chipDraw->height || draw->internalFormatColorBuffer != chipDraw->internalFormatColorBuffer || (gc->changeMask & __GL_DRAWABLE_PENDING_PRIMARY_LOST))
    {
        detachDrawable(chipCtx, draw);

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
            chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatColorBuffer, GL_FALSE,chipCreateInfo.samples);

            /*frontleftbuffer: create front buffer(default) */
            draw->frontBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
            chipCreateInfo.flags = __GL_FRONT_BUFFER;
            chipCreateInfo.bufInfo = (GLvoid *)&draw->frontBuffer;
            chipCreateInfo.subFlags = 0;
            chipCreateInfo.poolType = gcvPOOL_DEFAULT;
            chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
            retValue = createRenderBuffer(gc, &chipCreateInfo);

            /* set multisample parameters */
            chipCreateInfo.sampleBuffers = draw->modes.sampleBuffers;
            chipCreateInfo.samples = draw->modes.samples;
            /*frontrightbuffer: create the frontright buffer*/
            if ((draw->modes.stereoMode) && (retValue))
            {
                draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.flags = __GL_FRONT_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
                retValue = createRenderBuffer(gc, &chipCreateInfo);
            }

            if((draw->modes.doubleBufferMode) && (retValue))
            {
                /*backleftbuffer: create the first back buffer*/
                draw->backBuffer[__GL_BACK_BUFFER0].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.flags = __GL_BACK_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_BACK_BUFFER0];
                retValue = createRenderBuffer(gc, &chipCreateInfo);

                /*backrightbuffer: create the backright buffer*/
                if ((draw->modes.stereoMode) && (retValue))
                {
                    draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                    chipCreateInfo.flags = __GL_BACK_BUFFER;
                    chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
                    retValue = createRenderBuffer(gc, &chipCreateInfo);
                }
            }

            if ((draw->modes.tripleBufferMode)  && (retValue))
            {
                /*Now we should not come here,cause we have no triplebuffer flag*/
                GL_ASSERT(0);
                /*create the second back buffer*/
                draw->backBuffer[__GL_BACK_BUFFER1].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.flags = __GL_BACK_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_BACK_BUFFER1];
                retValue = createRenderBuffer(gc, &chipCreateInfo);
            }

            /*AUXBuffers: create the AUX buffers*/
            GL_ASSERT(draw->modes.numAuxBuffers >=0 && draw->modes.numAuxBuffers <=4);
            for(i = 0; (i < draw->modes.numAuxBuffers) && retValue; i++)
            {
                draw->drawBuffers[__GL_DRAWBUFFER_AUX0_INDEX + i].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.flags = __GL_BACK_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_AUX0_INDEX + i];
                retValue = createRenderBuffer(gc, &chipCreateInfo);
            }

            /* Create resolve buffer */
            if (retValue) {
                chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatDisplayBuffer, GL_FALSE, 0);
                draw->backBuffer[__GL_RESOLVE_BUFFER].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                /*
                chipCreateInfo.subFlags = (GLuint)draw->fullScreenMode;
                */
                chipCreateInfo.subFlags = 0;
                chipCreateInfo.flags = __GL_RESOLVE_BUFFER_FLAG;
                chipCreateInfo.poolType = gcvPOOL_UNIFIED;
                chipCreateInfo.surfType = gcvSURF_BITMAP;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_RESOLVE_BUFFER];
                retValue = createRenderBuffer(gc, &chipCreateInfo);
            }

            /* depthbuffer: Create depth buffer */
            if ((draw->modes.haveDepthBuffer) && retValue)
            {
                chipCreateInfo.flags = __GL_DEPTH_BUFFER;
                chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatDepthBuffer,GL_FALSE,chipCreateInfo.samples);
                draw->depthBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_DEPTH;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->depthBuffer;
                retValue = createRenderBuffer(gc, &chipCreateInfo);
            }
            /* stecncilbuffer: Create stencil buffer */
            if ((draw->modes.haveStencilBuffer) && retValue)
            {
                glsCHIPSTENCILBUFFER   *chipStencilBuffer;
                glsCHIPDEPTHBUFFER   *chipDepthBuffer;
                chipCreateInfo.flags = __GL_STENCIL_BUFFER;
                chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatStencilBuffer,GL_FALSE,chipCreateInfo.samples);
                draw->stencilBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.bufInfo = (GLvoid *)&draw->stencilBuffer;
                retValue = createRenderBuffer(gc, &chipCreateInfo);
                chipDepthBuffer = draw->depthBuffer.privateData;
                chipStencilBuffer = draw->stencilBuffer.privateData;
                chipStencilBuffer->stencilBuffer = chipDepthBuffer->depthBuffer;
                chipStencilBuffer->stencilFormat = chipDepthBuffer->depthFormat;
            }

            /* accumbuffer: Create accumulator buffer */
            if ((draw->modes.haveAccumBuffer) && retValue)
            {
                chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatAccumBuffer, GL_FALSE,chipCreateInfo.samples);
                draw->accumBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
                chipCreateInfo.flags = __GL_ACCUM_BUFFER;
                chipCreateInfo.poolType = gcvPOOL_DEFAULT;
                chipCreateInfo.surfType = gcvSURF_RENDER_TARGET;
                chipCreateInfo.bufInfo = (GLvoid *)&draw->accumBuffer;
                createRenderBuffer(gc, &chipCreateInfo);
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
            initAccumOperationPatch(gc, chipDraw);
        }
    }

    if (!retValue) {
        if(draw->dp.freeBuffers) {
            draw->dp.freeBuffers(draw, GL_FALSE);
        }
    }
    gcmFOOTER_NO();
    return;
}

/**********************************************************************/
/* Implementation for device drawable APIs                            */
/**********************************************************************/
GLboolean __glChipSwapBuffers(__GLcontext *gc,
    __GLdrawablePrivate *draw,
    GLboolean bSwapFront)
{
    GLboolean retValue = GL_TRUE;
#ifndef DISABLE_SWAP_THREAD
    glsWORKINFO_PTR worker;
#endif

#if DISABLE_SWAP_THREAD
#ifndef DIRECT_TO_FB
    glsCHIPDRAWABLE_PTR chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
#endif
#else
    glsCHIPDRAWABLE_PTR chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
    GLvoid *bits;
#endif

#ifndef USE_RESOLVE
    resolveBuffer(gc, bSwapFront);
#endif

#if DISABLE_SWAP_THREAD
    gcoHAL_Commit(gcvNULL, gcvTRUE);
#ifdef CPUBLT_TO_FB
    CPUBltBufferToScreen(chipDraw->gc, chipDraw->swapInfo.swapBits);
#else
#if DIRECT_TO_FB
    resolveRenderTargetToScreen(gc);
#else
    (*chipDraw->gc->imports.bltImageToScreen)(
               chipDraw->gc,
               chipDraw->swapInfo.bitsAlignedWidth,
               chipDraw->swapInfo.bitsAlignedHeight,
               chipDraw->swapInfo.swapBitsPerPixel,
               chipDraw->swapInfo.swapBits,
               0,
               0,
               draw->width,
               draw->height
               );
#endif
#endif
#endif

#if DISABLE_SWAP_THREAD
#else
    /* Set resolve surface bits. */
    bits = chipDraw->swapInfo.swapBits;

    if (dpGlobalInfo.workerThread != gcvNULL) {
        /* Suspend the worker thread. */
        suspendSwapWorker(&dpGlobalInfo);

        /* Find an available worker. */
        worker = getWorker(chipDraw);
        if (worker == gcvNULL)
        {
            /* Something horrible has happened. */
            retValue = GL_FALSE;
            return retValue;
        }

        /* Multi-buffered target enabled? */
        if (chipDraw->renderListEnable)
        {
        }
        else
        {
            /* Bits must be set. */
            gcmASSERT(bits != gcvNULL);

            /* Set surface attributes. */
            worker->targetSignal = gcvNULL;
            worker->bits         = bits;
        }

        worker->draw                   = chipDraw;
#if DIRECT_TO_FB
        worker->backBuffer.origin.x    = draw->xOrigin;
        worker->backBuffer.origin.y    = draw->yOrigin;
#else
        worker->backBuffer.origin.x    = 0;
        worker->backBuffer.origin.y    = 0;
#endif
        worker->backBuffer.size.width  = draw->width;
        worker->backBuffer.size.height = draw->height;
        worker->next                   = gcvNULL;

        /* Submit worker. */
        if (!submitWorker(&dpGlobalInfo, worker, gcvTRUE))
        {
            retValue = GL_FALSE;
        }

        /* Resume the thread. */
        resumeSwapWorker(&dpGlobalInfo);
    }

    /* Commit the command buffer. */
    if (gcmIS_ERROR(gcoHAL_Commit(gcvNULL, gcvFALSE)))
    {
        /* Bad surface. */
        retValue = GL_FALSE;
    }
#endif
    exchangeBufferHandles(gc, draw, !bSwapFront);

    /* Success. */
    return retValue;
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

    __glChipDrawBuffers(gc);
    __glChipReadBuffer(gc);

    /* When Draw Buffers change, update viewport and scissor based on new drawRTwidth and drawRTheight*/
    /* The next lines are useful only when window size can't be got in time through DRI interface */
    validateViewport(gc);
    validateScissor(gc);

    /* If one of needed buffer failed to create,we will skip following draws until renderbuffer is ready */
    if (!chipDraw->haveRenderBuffer)
    {
        gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_NULL_RENDERBUFFER;
    }
    else
    {
        gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_NULL_RENDERBUFFER;
    }
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
        __glChipDrawBuffers(gc);
        __glChipReadBuffer(gc);
    }
}


GLvoid __glChipNotifyDrawableSwitch(__GLcontext *gc)
{
    __glChipDrawBuffers(gc);
    __glChipReadBuffer(gc);
}

GLvoid __glChipNotifyWinBuffersResident(__GLcontext *gc, __GLdrawablePrivate* draw)
{
   /* For vista, we don't need consider RT make resident now */
}

GLvoid __glChipCreatePbuffer(__GLcontext *gc, __GLdrawablePrivate * draw)
{
    glsCHIPDRAWABLE* chipDraw = (glsCHIPDRAWABLE*)(draw->dp.privateData);
    glsCHIPBUFFERCREATE chipCreateInfo;
    __GLdeviceFormat chosenFormat;
    GLint i;

    __GL_MEMZERO(&chipCreateInfo, sizeof(glsCHIPBUFFERCREATE));

    /* set multisample parameters */
    chipCreateInfo.sampleBuffers = draw->modes.sampleBuffers;
    chipCreateInfo.samples = draw->modes.samples;

    GL_ASSERT(draw->pbufferTex);
    if (draw->pbufferTex->renderTexture)
    {
        /* This pbuffer is a texture. */
        GLuint targetIndex = 0;
        GLuint levels = 0;

        chipCreateInfo.flags = __GL_PBUFFERTEX_BUFFER;

        switch(draw->pbufferTex->target)
        {
        case GL_TEXTURE_1D:
            targetIndex = __GL_TEXTURE_1D_INDEX;
            break;
        case GL_TEXTURE_2D:
            targetIndex = __GL_TEXTURE_2D_INDEX;
            break;
        case GL_TEXTURE_RECTANGLE_ARB:
            targetIndex = __GL_TEXTURE_RECTANGLE_INDEX;
            break;
        case GL_TEXTURE_3D:
            targetIndex = __GL_TEXTURE_3D_INDEX;
            break;
        case GL_TEXTURE_CUBE_MAP:
            targetIndex = __GL_TEXTURE_CUBEMAP_INDEX;
            break;
        default:
            GL_ASSERT(0);
            break;
        }
        chipCreateInfo.targetIndex = targetIndex;

        if(draw->pbufferTex->mipmap)
        {
            levels = __glComputeNumLevels(draw->width, draw->height, 1, 0);
        }
        else
        {
            levels = 1;
        }
        chipCreateInfo.levels = levels;

        /* Set the chosen format for render texture. */
        draw->pbufferTex->chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatColorBuffer,GL_FALSE,chipCreateInfo.samples);
    }
    else
    {
        /* non-texture pbuffer is "back buffer". */
        chipCreateInfo.flags = __GL_BACK_BUFFER;
    }
    /* Allocate video memory for frontleft buffer*/
    chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatColorBuffer,GL_FALSE,chipCreateInfo.samples);
    draw->frontBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
    chipCreateInfo.bufInfo = (GLvoid *)&(draw->frontBuffer);
    createRenderBuffer(gc, &chipCreateInfo);

    /* Allocate video memory for frontright buffer*/
    if (draw->modes.stereoMode)
    {
        draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_FRONTRIGHT_INDEX];
        createRenderBuffer(gc, &chipCreateInfo);
    }

    /* Allocate video memory for backleft& backright buffer*/
    if(draw->modes.doubleBufferMode)
    {
        draw->drawBuffers[__GL_DRAWBUFFER_BACKLEFT_INDEX].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_BACKLEFT_INDEX];
        createRenderBuffer(gc, &chipCreateInfo);

        if (draw->modes.stereoMode)
        {
            draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
            chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_BACKRIGHT_INDEX];
            createRenderBuffer(gc, &chipCreateInfo);
        }
    }

    /* Allocate video memory for AUX buffer*/
    for(i = 0; i < draw->modes.numAuxBuffers; i++)
    {
        draw->drawBuffers[__GL_DRAWBUFFER_AUX0_INDEX + i].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->drawBuffers[__GL_DRAWBUFFER_AUX0_INDEX + i];
        createRenderBuffer(gc, &chipCreateInfo);
    }

    /* Allocate video memory for second back buffer*/
    if( draw->modes.tripleBufferMode )
    {
        /* Note : We should not come here now*/
        GL_ASSERT(0);
        draw->backBuffer[__GL_BACK_BUFFER1].deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->backBuffer[__GL_BACK_BUFFER1];
        createRenderBuffer(gc, &chipCreateInfo);
    }
    /* Allocate video memory for depth buffer */
    if(draw->modes.haveDepthBuffer)
    {
        chipCreateInfo.flags = __GL_DEPTH_BUFFER;
        chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatDepthBuffer,GL_FALSE,chipCreateInfo.samples);
        draw->depthBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->depthBuffer;
        createRenderBuffer(gc, &chipCreateInfo);
    }

    if(draw->modes.haveStencilBuffer)
    {
        chipCreateInfo.flags = __GL_STENCIL_BUFFER;
        chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatStencilBuffer,GL_FALSE,chipCreateInfo.samples);
        draw->stencilBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.bufInfo = (GLvoid *)&draw->stencilBuffer;
        createRenderBuffer(gc, &chipCreateInfo);
    }

    /* Allocate video memory for multiple sampling buffer */
    if(draw->modes.haveMultiSampleBuffer)
    {
    }

    /* Allocate video memory for accumulator buffer */
    if(draw->modes.haveAccumBuffer)
    {
        chosenFormat = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatAccumBuffer, GL_FALSE,chipCreateInfo.samples);
        draw->accumBuffer.deviceFormatInfo = &__glDevfmtInfo[chosenFormat];
        chipCreateInfo.flags = __GL_ACCUM_BUFFER;
        chipCreateInfo.bufInfo = (GLvoid *)&draw->accumBuffer;
        createRenderBuffer(gc, &chipCreateInfo);
    }

    /* Initial Drawable */
    initDrawable(chipDraw, draw);
}

GLboolean __glChipSwapInterval( __GLcontext *gc, GLint interval, GLuint fSet)
{
    return  GL_FALSE;
}

void __glChipDestroyDrawable(__GLdrawablePrivate *draw)
{
    if (draw->dp.privateData) {
        (*draw->free)(draw->dp.privateData);
    }

    draw->dp.privateData = NULL;
}

void __glChipUpdateDrawable(__GLdrawablePrivate* draw)
{
    GLuint i;

    /* Update all draw buffers */
    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++) {
        updateDrawableBuffer(draw, &draw->drawBuffers[i], draw->modes.rgbaBits);
    }

    updateDrawableBuffer(draw, &draw->frontBuffer2, draw->modes.rgbaBits);
    updateDrawableBuffer(draw, &draw->backBuffer[__GL_RESOLVE_BUFFER], draw->modes.rgbaBits);

    if(draw->modes.haveDepthBuffer)
    {
        updateDrawableBuffer(draw, &draw->depthBuffer, draw->modes.depthBits);
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
        updateDrawableBuffer(draw, &draw->stencilBuffer, draw->modes.stencilBits);
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
        updateDrawableBuffer(draw, &draw->accumBuffer, draw->modes.accumBits);
    }

    if(draw->modes.haveMultiSampleBuffer) {
        updateDrawableBuffer(draw, &draw->multisampleBuffer, draw->modes.rgbaBits);
    }
}

GLvoid __glChipFreeDrawableBuffers(__GLdrawablePrivate *draw, GLboolean bWaitDrawDone)
{
    glsCHIPREADABLE * chipDrawable;
    glCHIPBUFFERDESSTROY chipDestroyInfo;
    GLuint i;
    gceSTATUS status;

    gcmHEADER_ARG("draw=0x%x", draw);

    chipDrawable = (glsCHIPDRAWABLE*)(draw->dp.privateData);

    do
    {
        /* Free the workers. */
        gctUINT32 i;

        /* Wait for workers to become idle. */
        for (i = 0; i < WORKER_COUNT; i++)
        {
            gcmASSERT(chipDrawable->workers != gcvNULL);
            while(bWaitDrawDone && (chipDrawable->workers[i].draw != gcvNULL))
            {
                /* Sleep for a while */
                gcmVERIFY_OK(gcoOS_Delay(gcvNULL, 10));
            }
        }

        for (i = 0; i < WORKER_COUNT; i++)
        {
            /* Make sure all worker threads are finished. */
            if (chipDrawable->workers[i].signal != gcvNULL)
            {
                gcmVERIFY_OK(
                    gcoOS_DestroySignal(gcvNULL, chipDrawable->workers[i].signal));
                chipDrawable->workers[i].signal = gcvNULL;
            }
        }

        /* Destroy worker mutex. */
        if (chipDrawable->workerMutex != gcvNULL)
        {
            gcmERR_BREAK(
                gcoOS_DeleteMutex(gcvNULL, chipDrawable->workerMutex));
        }
    } while (GL_FALSE);

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

    /* Free multiple sampling buffer */
    if(draw->modes.haveMultiSampleBuffer)
    {

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
    gcmFOOTER_NO();
    return;
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
    draw->dp.updateDrawable = __glChipUpdateDrawable;

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


