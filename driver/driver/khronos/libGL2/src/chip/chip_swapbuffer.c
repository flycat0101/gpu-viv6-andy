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
#include "gl/gl_device.h"
#include "viv_lock.h"

extern const __GLdeviceFormatInfo __glDevfmtInfo[];
extern glsDEVICEPIPELINEGLOBAL dpGlobalInfo;

extern int currentDesktop;

#define _GC_OBJ_ZONE    gcvZONE_API_GL

#if DIRECT_TO_FB
extern GLboolean createDisplayBuffer(__GLcontext * gc, GLboolean bOnScreen,  gcoSURF *renderTarget);
#endif

#ifdef CPUBLT_TO_FB
GLvoid CPUBltBufferToScreen(__GLcontext * gc, GLbyte * resolveLogicalAddress)
{
    GLuint offset;
    GLuint stride;
    gcoSURF     resolve;
    GLvoid          *addr[3];
    unsigned int     phys[3];
    GLuint alignedWidth, alignedHeight;
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);
    GLuint i,j;
    RECT *pClipRects;
    GLuint xoffset, yoffset;
    GLuint height, width;
    GLbyte * src;
    GLbyte * dest;

    LINUX_LOCK_FRAMEBUFFER(gc);

    if (!(gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE)) {

        resolve = chipDraw->resolveBuffer->renderTarget;

        stride = dpGlobalInfo.stride;
        offset = draw->xOrigin * dpGlobalInfo.bpp + draw->yOrigin * stride;

        gcoSURF_GetAlignedSize(
                    resolve,
                    &alignedWidth,
                    &alignedHeight,
                    gcvNULL
                    );

        pClipRects = (RECT *)draw->clipRects;
        if (draw->numClipRects > 0) {
            for (j = 0; j < draw->numClipRects; j++) {
                xoffset = pClipRects->left - draw->xOrigin;
                yoffset = pClipRects->top - draw->yOrigin;
                width = pClipRects->right - pClipRects->left;
                height = pClipRects->bottom - pClipRects->top;
                src = (GLbyte *)resolveLogicalAddress + (yoffset * alignedWidth + xoffset) * dpGlobalInfo.bpp;
                dest = (GLbyte *)dpGlobalInfo.logicalAddress + pClipRects->top * stride + pClipRects->left * dpGlobalInfo.bpp;
                for (i = 0; i < height; i++) {
                    memcpy((dest + i * stride), (src + i * alignedWidth * dpGlobalInfo.bpp), width * dpGlobalInfo.bpp);
                }
                pClipRects++;
            }
        }
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}
#endif

#if DIRECT_TO_FB
static gceSTATUS LockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node,
        OUT gctUINT64 *Address,
        OUT gctPOINTER *Memory) {
    gceSTATUS status;
    gcsHAL_INTERFACE iface;
    gctUINT8 **pMem = (gctUINT8 **)Memory;

    gcmASSERT(Address != gcvNULL);
    gcmASSERT(Memory != gcvNULL);
    gcmASSERT(Node != 0);

    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
    iface.u.LockVideoMemory.node = Node;
    iface.u.LockVideoMemory.cacheable = gcvFALSE;

    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));

    /* Get allocated node in video memory. */
    *Address = iface.u.LockVideoMemory.physicalAddress;
    *pMem = gcvNULL;
    *pMem += iface.u.LockVideoMemory.memory;

OnError:
    return status;
}

static gceSTATUS UnlockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node) {
    gcsHAL_INTERFACE iface;

    gceSTATUS status = gcvSTATUS_OK;
    gcmASSERT(Node != 0);

    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.u.UnlockVideoMemory.node = Node;
    iface.u.UnlockVideoMemory.type = gcvSURF_BITMAP;
    iface.u.UnlockVideoMemory.asynchroneous = gcvTRUE;

    gcmONERROR(gcoHAL_Commit(Hal,gcvTRUE));

    gcmONERROR(gcoOS_DeviceControl(
                        gcvNULL,
                        IOCTL_GCHAL_INTERFACE,
                        &iface,gcmSIZEOF(iface),
                        &iface,gcmSIZEOF(iface)
    ));

    gcmONERROR(iface.status);

    /*
    if (iface.u.UnlockVideoMemory.asynchroneous){
        iface.u.UnlockVideoMemory.asynchroneous = gcvFALSE;
        gcmONERROR(gcoHAL_ScheduleEvent(Hal,&iface));
    }
    */

OnError:
    /* Call kernel API. */
    return status;
}

static gcoSURF _GetWrapSurface(gcoSURF ResolveTarget, gctPOINTER phyaddr, gctPOINTER lineaddr, gctUINT width, gctUINT height, gctUINT stride)
{
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    static gcoSURF tarsurf = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;

    if ( tarsurf )
        gcoSURF_Destroy(tarsurf);

    gcoSURF_GetFormat(ResolveTarget, &surftype, &surfformat);

    do {
        /* Construct a wrapper around the pixmap.  */
        gcmERR_BREAK(gcoSURF_ConstructWrapper(
        gcvNULL,
        &tarsurf
        ));

        /* Set the underlying frame buffer surface. */
        gcmERR_BREAK(gcoSURF_SetBuffer(
        tarsurf,
        surftype,
        surfformat,
        stride,
        lineaddr,
        gcmALL_TO_UINT32(phyaddr)
        ));

        /* Set the window. */
        gcmERR_BREAK(gcoSURF_SetWindow(
        tarsurf,
        0,
        0,
        width,
        height
        ));
    } while(0);

    return tarsurf;
}

static gceSTATUS resolveRenderTargetWithDirectMode(
    gcoSURF renderTarget,
    gcoSURF resolve,
    __GLdrawablePrivate *draw,
    __DRIdrawablePrivate *dPriv,
    gctPOINTER phyaddr,
    gctPOINTER lineaddr,
    gctUINT alignedwidth)
{

    RECT *pClipRects;
    GLuint xoffset, yoffset;
    GLuint height, width;
    GLuint srcalignedwidth,srcalignedheight;
    GLbyte *destStart = (GLbyte *)lineaddr;
    GLbyte *destPhyStart = (GLbyte *)phyaddr;
    gcsSURF_VIEW rtView = {renderTarget, 0, 1};
    gcsSURF_VIEW wrapView = {gcvNULL, 0, 1};
    gceSTATUS status;

    if ( draw->numClipRects != 1 )
        return gcvSTATUS_INVALID_ARGUMENT;

    pClipRects = (RECT *)(RECT *)draw->clipRects;

    xoffset = pClipRects->left - draw->xWOrigin;
    yoffset = pClipRects->top - draw->yWOrigin;

    if ( xoffset != 0 || yoffset != 0 )
        return gcvSTATUS_INVALID_ARGUMENT;

    gcoSURF_GetAlignedSize(resolve, &srcalignedwidth, &srcalignedheight, gcvNULL);

    width = pClipRects->right - pClipRects->left;
    height = pClipRects->bottom - pClipRects->top;

    if ( draw->width != width || draw->height != height )
        return gcvSTATUS_INVALID_ARGUMENT;

    if ( alignedwidth < srcalignedwidth )
        return gcvSTATUS_INVALID_ARGUMENT;

    xoffset = pClipRects->left;
    yoffset = pClipRects->top -(srcalignedheight - height);
    destStart += (alignedwidth * yoffset + xoffset) * dpGlobalInfo.bpp;
    destPhyStart += (alignedwidth * yoffset + xoffset) * dpGlobalInfo.bpp;

    wrapView.surf = _GetWrapSurface(resolve, (gctPOINTER)destPhyStart, (gctPOINTER)destStart, width, height, alignedwidth * dpGlobalInfo.bpp);

    status = gcoSURF_ResolveRect(&rtView, &wrapView, gcvNULL);

    if (gcmIS_ERROR(status))
    {
        return status;
    }
    return gcvSTATUS_OK;
}
GLvoid resolveRenderTargetToScreen(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);

    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;
    __DRIdrawablePrivate *dPriv = pDriMirror->drawable;

    gcoSURF     renderTarget;
    gcoSURF     resolve;
    GLvoid      *destLogicalAddr[3] = {NULL, NULL, NULL};
    GLbyte      *destStart;
    GLbyte      *srcStart;
    GLbyte      *src;
    gctUINT64      destPhys[3] = {0, 0, 0};
    GLint i,j;
    GLuint alignedWidth, alignedHeight;
    GLuint alignedDestWidth;
    RECT *pClipRects;
    GLuint xoffset, yoffset;
    GLuint height, width;
    GLuint stride;
    gceSTATUS status;
#if USE_BLT
    gcoSURF     displaySurf;
#else
#if USE_CPU
#else
    gcoSURF displaySurf;
#endif
#endif

#if !defined(DRI_PIXMAPRENDER_GL)
    if (draw->fullScreenMode && (dPriv->fullscreenCovered == 0))
    {
        return;
    }
#endif

    if (gc->flags & __GL_DRAW_TO_FRONT) {
        renderTarget = (*(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]))->renderTarget;
    } else {
        renderTarget = (*(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]))->renderTarget;
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    if (!(gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE)) {

#if defined(DRI_PIXMAPRENDER_GL)
        if ( (gcoSURF)dPriv->wrapSurface )
        {
            dPriv->doCPYToSCR(dPriv);
            goto END_FLAGX;
        }
#endif

        if ( draw->backNode )
            LockVideoNode(chipCtx->hal, draw->backNode, &destPhys[0], &destLogicalAddr[0]);
        draw->backBufferPhysAddr = (GLuint)destPhys[0];
        draw->backBufferLogicalAddr = destLogicalAddr[0];
#if USE_BLT
        if (createDisplayBuffer(gc, GL_FALSE, NULL)) {
            displaySurf = chipDraw->displayBuffer.renderTarget;
            if (displaySurf) {
                gcsRECT srcrect= {0,0,draw->width,draw->height};
                gcsRECT dstrect= {draw->xWOrigin,draw->yWOrigin,draw->xWOrigin+ draw->width,draw->yWOrigin+draw->height};
                resolve = chipDraw->resolveBuffer->renderTarget;
                gcoSURF_Blit(
                    resolve,
                    displaySurf,
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
                gcoSURF_CPUCacheOperation(resolve, gcvCACHE_CLEAN);
                gcoHAL_Commit(gcvNULL, gcvTRUE);
                gcoSURF_Destroy(displaySurf);
                chipDraw->displayBuffer.renderTarget = gcvNULL;
            }
        }
#else
#if USE_CPU
        resolve = chipDraw->resolveBuffer->renderTarget;

        gcoSURF_GetAlignedSize(
                resolve,
                &alignedWidth,
                &alignedHeight,
                gcvNULL
        );

        if (draw->backNode) {

                alignedDestWidth = draw->wWidth;

                destStart = destLogicalAddr[0];
                srcStart = chipDraw->swapInfo.swapBits;


                if (draw->numClipRects > 0 )
                {
                    if (( status =resolveRenderTargetWithDirectMode(renderTarget, resolve, draw, dPriv, (gctPOINTER)draw->backBufferPhysAddr, (gctPOINTER)destStart, alignedDestWidth)) == gcvSTATUS_OK )
                        goto END_FLAG;

                    pClipRects = (RECT *)draw->clipRects;
                    for (i = 0; i < draw->numClipRects; i++)
                    {
                        destStart = destLogicalAddr[0];
                        srcStart = chipDraw->swapInfo.swapBits;
                        xoffset = pClipRects->left - draw->xWOrigin;
                        yoffset = pClipRects->top - draw->yWOrigin;
                        width = pClipRects->right - pClipRects->left;
                        height = pClipRects->bottom - pClipRects->top;
                        srcStart += (alignedWidth*yoffset + xoffset ) * dpGlobalInfo.bpp;
                        xoffset = pClipRects->left;
                        yoffset = pClipRects->top;
                        destStart += (alignedDestWidth * yoffset + xoffset) * dpGlobalInfo.bpp;
                        for (j = 0; j < height; j++) {
                        memcpy((destStart + j * alignedDestWidth * dpGlobalInfo.bpp), (srcStart + j * alignedWidth * dpGlobalInfo.bpp), width * dpGlobalInfo.bpp);
                        }
                        pClipRects++;
                    }
                }

        }

        /* Front buffer swap for legacy X window desktops */
        if ( draw->backNode == 0 ) {
            stride = dpGlobalInfo.stride;
            srcStart = chipDraw->swapInfo.swapBits;
            pClipRects = (RECT *)draw->clipRects;
            if ( (draw->backNode == 0) && draw->numClipRects > 0) {
                for (i = 0; i < draw->numClipRects; i++) {
                    xoffset = pClipRects->left - draw->xWOrigin;
                    yoffset = pClipRects->top - draw->yWOrigin;
                    width = pClipRects->right - pClipRects->left;
                    height = pClipRects->bottom - pClipRects->top;
                    src = (GLbyte *)srcStart + (yoffset * alignedWidth + xoffset) * dpGlobalInfo.bpp;
                    destStart = (GLbyte *)dpGlobalInfo.logicalAddress + pClipRects->top * stride + pClipRects->left * dpGlobalInfo.bpp;
                    for (j = 0; j < height; j++) {
                        memcpy((destStart + j * stride), (src + j * alignedWidth * dpGlobalInfo.bpp), width * dpGlobalInfo.bpp);
                    }
                    pClipRects++;
                }
            }
        }
#else
        if (createDisplayBuffer(gc, GL_FALSE, NULL)) {
            displaySurf = chipDraw->displayBuffer.renderTarget;
            if (displaySurf) {
                gcsSURF_VIEW rtView = {renderTarget, 0, 1};
                gcsSURF_VIEW displayView = {displaySurf, 0, 1};
                gcsSURF_RESOLVE_ARGS rlvArgs = {0};

                rlvArgs.version = gcvHAL_ARG_VERSION_V2;
                rlvArgs.uArgs.v2.dstOrigin.x = draw->xWOrigin;
                rlvArgs.uArgs.v2.dstOrigin.y = draw->yWOrigin;
                rlvArgs.uArgs.v2.rectSize.x  = draw->width;
                rlvArgs.uArgs.v2.rectSize.y  = draw->height;
                rlvArgs.uArgs.v2.numSlices   = 1;

                gcoSURF_ResolveRect(&rtView, &displayView, &rlvArgs);
                gcoSURF_CPUCacheOperation(resolve, gcvCACHE_CLEAN);
                gcoHAL_Commit(gcvNULL, gcvTRUE);
                gcoSURF_Destroy(displaySurf);
                chipDraw->displayBuffer.renderTarget = gcvNULL;
            }
        }
#endif
END_FLAG:
#endif
        if ( draw->backNode )
            UnlockVideoNode(chipCtx->hal, draw->backNode);
    }
#if defined(DRI_PIXMAPRENDER_GL)
END_FLAGX:
#endif
    LINUX_UNLOCK_FRAMEBUFFER(gc);
}

GLvoid resolveBufferToOnScreen(__GLcontext * gc)
{
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);
    gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
    GLvoid      *addr[3];
    unsigned int phys[3];
    GLint i;
    GLuint offset;
    GLuint stride;
    GLuint alignedWidth, alignedHeight;

    if (gc->flags & __GL_DRAW_TO_FRONT) {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]))->renderTarget;
    } else {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]))->renderTarget;
    }

    if (!chipDraw->displayBuffer.renderTarget) {
      createDisplayBuffer(gc, GL_FALSE, NULL);
    }

    tgtView.surf = chipDraw->resolveBuffer->renderTarget;

    /* If using cached video memory, we should clean cache before using GPU
    *  to do resolve rectangle. fix Bug3508
    */
    gcoSURF_CPUCacheOperation(rtView.surf, gcvCACHE_CLEAN);

    if (tgtView.surf) {
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.rectSize.x  = draw->width;
        rlvArgs.uArgs.v2.rectSize.y  = draw->height;
        rlvArgs.uArgs.v2.numSlices   = 1;

        gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
    }

    gcoSURF_CPUCacheOperation(tgtView.surf, gcvCACHE_CLEAN);

    stride = dpGlobalInfo.stride;
    offset = draw->xOrigin * dpGlobalInfo.bpp + draw->yOrigin * stride;

    gcoSURF_GetAlignedSize(
                    tgtView.surf,
                    &alignedWidth,
                    &alignedHeight,
                    gcvNULL
                    );
    gcoSURF_Lock(tgtView.surf, phys, addr );
    for (i = 0; i < draw->height; i++) {
        memcpy(((GLbyte *)dpGlobalInfo.logicalAddress + offset + i * stride), ((GLbyte *)addr[0] + i * alignedWidth * dpGlobalInfo.bpp)  , draw->width * dpGlobalInfo.bpp);
    }
    gcoSURF_Unlock(tgtView.surf, addr );

}
#endif

GLvoid resolveBuffer(__GLcontext * gc,  GLboolean swapFront)
{
#if DIRECT_TO_FB
    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;
    __DRIdrawablePrivate *dPriv = pDriMirror->drawable;
#endif
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);
    gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
#if DIRECT_TO_FB
#if !defined(DRI_PIXMAPRENDER_GL)
    GLboolean fulldisplay = GL_TRUE;
#endif
#endif

    if (swapFront) {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_FRONT_BUFFER_INDEX]))->renderTarget;
    } else {
        rtView.surf = (*(chipDraw->drawBuffers[__GL_BACK_BUFFER0_INDEX]))->renderTarget;
    }

#if DIRECT_TO_FB
#if !defined(DRI_PIXMAPRENDER_GL)
    if (draw->fullScreenMode && (dPriv->fullscreenCovered == 0))
    {

        if (createDisplayBuffer(gc, GL_TRUE, &tgtView.surf) == GL_FALSE )
        {
            fulldisplay = GL_FALSE;
            tgtView.surf = chipDraw->resolveBuffer->renderTarget;
        }

    }
    else
#endif
#endif
    {
#if defined(DRI_PIXMAPRENDER_GL) && defined(DIRECT_TO_FB)
        tgtView.surf = (gcoSURF)dPriv->wrapSurface;
        if ( tgtView.surf == gcvNULL )
        tgtView.surf = chipDraw->resolveBuffer->renderTarget;
#else
        tgtView.surf = chipDraw->resolveBuffer->renderTarget;
#endif
    }

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

#if DIRECT_TO_FB
#if !defined(DRI_PIXMAPRENDER_GL)
    if (draw->fullScreenMode && (dPriv->fullscreenCovered == 0) && fulldisplay) {
        gcoHAL_Commit(gcvNULL, gcvTRUE);
        gcoSURF_Destroy(tgtView.surf);
    }
#endif
#else
    if (draw->fullScreenMode) {
        gcoHAL_Commit(gcvNULL, gcvTRUE);
    }
#endif

}

glsWORKINFO_PTR freeWorker(glsWORKINFO_PTR Worker)
{
    gceSTATUS status;
    glsCHIPREADABLE_PTR ownerDraw;
    gctBOOL acquired = gcvFALSE;
    glsWORKINFO_PTR nextWorker;

    /* Get a shortcut to the surface. */
    ownerDraw = (glsCHIPDRAWABLE_PTR)Worker->draw;

    /* Get the next worker. */
    nextWorker = Worker->next;

    /* Remove worker from display worker list. */
    Worker->prev->next = Worker->next;
    Worker->next->prev = Worker->prev;

    /* Release the surface. */
    if (Worker->targetSignal != gcvNULL)
    {
        gcmONERROR(gcoOS_Signal(gcvNULL, Worker->targetSignal, gcvTRUE));
    }

    /* Get access to avaliable worker list. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, ownerDraw->workerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Add it back to the thread available worker list. */
    Worker->next = ownerDraw->availableWorkers;
    ownerDraw->availableWorkers = Worker;

    /* Set Worker as available. */
    Worker->draw = gcvNULL;

    /* Release access mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, ownerDraw->workerMutex));
    acquired = gcvFALSE;

    return nextWorker;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, ownerDraw->workerMutex));
    }

    return gcvNULL;
}

void suspendSwapWorker(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("deviceGlobal=0x%x", deviceGlobal);

    gcmASSERT(deviceGlobal != gcvNULL);
    if (deviceGlobal->suspendMutex != gcvNULL)
    {
        gcmONERROR(gcoOS_AcquireMutex(gcvNULL,
                                        deviceGlobal->suspendMutex,
                                        gcvINFINITE));
    }
    gcmFOOTER_NO();
    return;

OnError:
    gcmFOOTER();
    return;
}


void resumeSwapWorker(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("deviceGlobal=0x%x", deviceGlobal);

    gcmASSERT(deviceGlobal != gcvNULL);
    if (deviceGlobal->suspendMutex != gcvNULL)
    {
        gcmONERROR(gcoOS_ReleaseMutex(gcvNULL,
                                      deviceGlobal->suspendMutex));
    }
    gcmFOOTER_NO();
    return;

OnError:
    gcmFOOTER();
    return;
}

/* Worker thread to copy resolve buffer to display. */
DWORD WINAPI swapWorker(void* refData)
{
    glsWORKINFO_PTR displayWorker;
    glsWORKINFO_PTR currWorker;
    glsWORKINFO_PTR nextWorker;
    glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal;
    glsCHIPDRAWABLE_PTR chipDrawable;

    gcmHEADER_ARG("refData=0x%x", refData);

    /* Cast the device global object. */
    deviceGlobal = (glsDEVICEPIPELINEGLOBAL_PTR)refData;

    while (gcvTRUE)
    {
        /* Wait for the start signal. */
        if gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, deviceGlobal->startSignal, gcvINFINITE))
        {
            break;
        }

        /* Check the thread's stop signal. */
        if (gcmIS_SUCCESS(gcoOS_WaitSignal(gcvNULL, deviceGlobal->stopSignal, 0)))
        {
            /* Stop had been signaled, exit. */
            break;
        }

        /* Acquire synchronization mutex. */
        suspendSwapWorker(deviceGlobal);

        for (
            currWorker = deviceGlobal->workerSentinel.next;
            (currWorker != gcvNULL) && (currWorker->draw != gcvNULL);
        )
        {
            /* Is the worker's surface still being processed? */
            if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, currWorker->signal, 0)))
            {
                /* Yes, skip it. */
                currWorker = currWorker->next;
                continue;
            }

            /* Assume the current worker is the one to be displayed. */
            displayWorker = currWorker;

            /* Check if next frames are available. */
            for (
                nextWorker = currWorker->next;
                nextWorker->draw != gcvNULL;
                nextWorker = nextWorker->next
            )
            {
                /* Skip workers from other surfaces. */
                if (nextWorker->draw != currWorker->draw)
                {
                    continue;
                }

                /* Is the next worker ready? */
                if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, nextWorker->signal, 0)))
                {
                    /* Not yet. */
                    break;
                }

                /* Have we previously found a ready worker? */
                if (displayWorker != currWorker)
                {
                    /* Yes, ignore it. */
                    freeWorker(displayWorker);
                }

                /* Store the current worker. */
                displayWorker = nextWorker;
            }

            chipDrawable = (glsCHIPDRAWABLE_PTR)displayWorker->draw;
            if (chipDrawable->fbDirect)
            {
                ;
            }
            else
            {
#if DIRECT_TO_FB
                resolveBufferToOnScreen(chipDrawable->gc);
#else
#ifdef CPUBLT_TO_FB
        CPUBltBufferToScreen(chipDrawable->gc, displayWorker->bits);
#else
        (*chipDrawable->gc->imports.bltImageToScreen)(
               chipDrawable->gc,
               chipDrawable->swapInfo.bitsAlignedWidth,
               chipDrawable->swapInfo.bitsAlignedHeight,
               chipDrawable->swapInfo.swapBitsPerPixel,
               displayWorker->bits,
               displayWorker->backBuffer.origin.x,
               displayWorker->backBuffer.origin.y,
               displayWorker->backBuffer.size.width,
               displayWorker->backBuffer.size.height
               );
#endif
#endif
            }

            /* Free the more recent worker. */
            if (displayWorker != currWorker)
            {
                freeWorker(displayWorker);
            }

            /* Free the current worker. */
            currWorker = freeWorker(currWorker);
        }

        /* Release synchronization mutex. */
        resumeSwapWorker(deviceGlobal);
    }

    gcmFOOTER_ARG("return=%ld", (DWORD) 0);
    /* Success. */
    return (DWORD) 0;
}

GLboolean createWorkThread(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal)
{
    gceSTATUS status;

    /* Create worker thread. */
    do
    {
        gcmASSERT(deviceGlobal->startSignal == gcvNULL);
        /* Create thread start signal. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvFALSE,
            &deviceGlobal->startSignal
            ));

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_SIGNAL,
            "%s(%d): start signal created 0x%08X",
            __FUNCTION__, __LINE__,
            deviceGlobal->startSignal
            );

        gcmASSERT(deviceGlobal->stopSignal == gcvNULL);
        /* Create thread stop signal. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvTRUE,
            &deviceGlobal->stopSignal
            ));

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_SIGNAL,
            "%s(%d): stop signal created 0x%08X",
            __FUNCTION__, __LINE__,
            deviceGlobal->stopSignal
            );

        gcmASSERT(deviceGlobal->doneSignal == gcvNULL);
        /* Create thread done signal. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvTRUE,
            &deviceGlobal->doneSignal
            ));

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_SIGNAL,
           "%s(%d): done signal created 0x%08X",
            __FUNCTION__, __LINE__,
            deviceGlobal->doneSignal
            );

        gcmASSERT(deviceGlobal->suspendMutex == gcvNULL);
        /* Create thread lock signal. */
        gcmERR_BREAK(gcoOS_CreateMutex(
            gcvNULL,
            &deviceGlobal->suspendMutex
            ));

        /* No workers yet. */
        deviceGlobal->workerSentinel.draw = gcvNULL;
        deviceGlobal->workerSentinel.prev   = &deviceGlobal->workerSentinel;
        deviceGlobal->workerSentinel.next   = &deviceGlobal->workerSentinel;

        /* Set complete signal. */
        gcmERR_BREAK(gcoOS_Signal(
            gcvNULL,
            deviceGlobal->doneSignal,
            gcvTRUE
            ));

        gcmASSERT(deviceGlobal->workerThread == gcvNULL);
        /* Start the thread. */
        gcmERR_BREAK(gcoOS_CreateThread(
            gcvNULL, (gcTHREAD_ROUTINE)swapWorker, deviceGlobal, &deviceGlobal->workerThread
                ));

#if gcdENABLE_TIMEOUT_DETECTION
        /* Setup the swap thread signal timeout handler. */
        gcmVERIFY_OK(gcoOS_InstallTimeoutCallback(
            _SwapSignalTimeoutCallback, Display
            ));
#endif
    } while (gcvFALSE);

    return GL_TRUE;
}

GLvoid destroyWorkThread(glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal)
{
    /* Stop the thread. */
    if (deviceGlobal->workerThread != gcvNULL)
    {
        /* Set thread's stop signal. */
        gcmASSERT(deviceGlobal->stopSignal != gcvNULL);
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL,
            deviceGlobal->stopSignal,
            gcvTRUE
            ));

        /* Set thread's start signal. */
        gcmASSERT(deviceGlobal->startSignal != gcvNULL);
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL,
            deviceGlobal->startSignal,
            gcvTRUE
            ));

        /* Wait until the thread is closed. */
        gcmVERIFY_OK(gcoOS_CloseThread(
            gcvNULL, deviceGlobal->workerThread
            ));
        deviceGlobal->workerThread = gcvNULL;
    }

#if gcdENABLE_TIMEOUT_DETECTION
    /* Remove the default timeout handler. */
    gcmVERIFY_OK(gcoOS_RemoveTimeoutCallback(
        _SwapSignalTimeoutCallback
        ));
#endif

    /* Delete the start signal. */
    if (deviceGlobal->startSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, deviceGlobal->startSignal));
        deviceGlobal->startSignal = gcvNULL;
    }

    /* Delete the stop signal. */
    if (deviceGlobal->stopSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, deviceGlobal->stopSignal));
        deviceGlobal->stopSignal = gcvNULL;
    }

    /* Delete the done signal. */
    if (deviceGlobal->doneSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, deviceGlobal->doneSignal));
        deviceGlobal->doneSignal = gcvNULL;
    }

    /* Delete the mutex. */
    if (deviceGlobal->suspendMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, deviceGlobal->suspendMutex));
        deviceGlobal->suspendMutex = gcvNULL;
    }
}

glsWORKINFO_PTR getWorker(glsCHIPDRAWABLE_PTR chipDrawable)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    glsWORKINFO_PTR worker;

    /* Get access to avaliable worker list. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, chipDrawable->workerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Surface we have available workers? */
    if (chipDrawable->availableWorkers != gcvNULL)
    {
        /* Yes, pick the first one and use it. */
        worker = chipDrawable->availableWorkers;
        chipDrawable->availableWorkers = worker->next;
    }
    else
    {
        /* No, get the last submitted worker. */
        worker = chipDrawable->lastSubmittedWorker;

        /* Remove the worker from display worker list. */
        worker->prev->next = worker->next;
        worker->next->prev = worker->prev;
    }

    /* Set Worker as busy. */
    worker->draw = chipDrawable;

    /* Release access mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, chipDrawable->workerMutex));
    acquired = gcvFALSE;

    /* Create worker's signal. */
    if (worker->signal == gcvNULL)
    {
        gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvFALSE, &worker->signal));

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_SIGNAL,
            "%s(%d): worker thread signal created 0x%08X",
            __FUNCTION__, __LINE__,
            worker->signal
            );
    }

    /* Return the worker. */
    return worker;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, chipDrawable->workerMutex));
    }

    return gcvNULL;
}

gceSTATUS
releaseWorker(glsWORKINFO_PTR Worker)
{
    gceSTATUS status;
    glsCHIPDRAWABLE_PTR ownerSurface;
    gctBOOL acquired = gcvFALSE;

    /* Get a shortcut to the thread. */
    ownerSurface = Worker->draw;

    /* Get access to avaliable worker list. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, ownerSurface->workerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Add it back to the thread available worker list. */
    Worker->next = ownerSurface->availableWorkers;
    ownerSurface->availableWorkers = Worker;

    /* Set Worker as available. */
    Worker->draw = gcvNULL;

    /* Release access mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, ownerSurface->workerMutex));
    acquired = gcvFALSE;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, ownerSurface->workerMutex));
    }

    return status;
}

gctBOOL submitWorker(
    glsDEVICEPIPELINEGLOBAL_PTR deviceGlobal,
    glsWORKINFO_PTR Worker,
    gctBOOL ScheduleSignals
    )
{
    Worker->prev =  deviceGlobal->workerSentinel.prev;
    Worker->next = &deviceGlobal->workerSentinel;

    deviceGlobal->workerSentinel.prev->next = Worker;
    deviceGlobal->workerSentinel.prev       = Worker;

    ((glsCHIPDRAWABLE_PTR)Worker->draw)->lastSubmittedWorker = Worker;

    if (ScheduleSignals)
    {
        {
#if COMMAND_PROCESSOR_VERSION > 1
            gcsTASK_SIGNAL_PTR workerSignal;
            gcsTASK_SIGNAL_PTR startSignal;

            /* Allocate a cluster event. */
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
                                               gcvBLOCK_PIXEL,
                                               2,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 2,
                                               (gctPOINTER *) &workerSignal)))
            {
                /* Bad surface. */
                Thread->error = EGL_BAD_SURFACE;
                return gcvFALSE;
            }

            /* Determine the start signal set task pointer. */
            startSignal = (gcsTASK_SIGNAL_PTR) (workerSignal + 1);

            /* Fill in event info. */
            workerSignal->id      = gcvTASK_SIGNAL;
            workerSignal->process = deviceGlobal->process;
            workerSignal->signal  = Worker->signal;

            startSignal->id       = gcvTASK_SIGNAL;
            startSignal->process  = deviceGlobal->process;
            startSignal->signal   = Display->startSignal;
#else
            gcsHAL_INTERFACE iface;

            iface.command            = gcvHAL_SIGNAL;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(Worker->signal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = deviceGlobal->processID;
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Schedule the event. */
            if (gcmIS_ERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface)))
            {
                return gcvFALSE;
            }

            iface.command            = gcvHAL_SIGNAL;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(deviceGlobal->startSignal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = deviceGlobal->processID;
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Schedule the event. */
            if (gcmIS_ERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface)))
            {
                /* Bad surface. */
                return gcvFALSE;
            }
#endif
        }
    }

    return gcvTRUE;
}
