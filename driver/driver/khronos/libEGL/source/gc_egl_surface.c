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


#include "gc_egl_precomp.h"

#if defined(ANDROID)
#if ANDROID_SDK_VERSION >= 16
#      include <ui/ANativeObjectBase.h>
#   else
#      include <ui/android_native_buffer.h>
#      include <ui/egl/android_natives.h>
#   endif
#endif


/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_SURFACE

#if defined(ANDROID) && gcdENABLE_3D
static gctBOOL
_ForceRenderTarget(
    void
    )
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    gceSTATUS status = gcvTRUE;

    gcmONERROR(gcoHAL_GetPatchID(gcvNULL, &patchId));

    switch (patchId)
    {
    case gcvPATCH_MM06:
    case gcvPATCH_MM07:
    case gcvPATCH_GLBM11:
    case gcvPATCH_BM21:
    case gcvPATCH_ANTUTU:
    case gcvPATCH_ANTUTU4X:
    case gcvPATCH_ANTUTU5X:
    case gcvPATCH_GLBM21:
    case gcvPATCH_GLBM25:
    case gcvPATCH_GLBM27:
    case gcvPATCH_GFXBENCH:
    case gcvPATCH_QUADRANT:
    case gcvPATCH_NENAMARK:
    case gcvPATCH_NENAMARK2:
    case gcvPATCH_SMARTBENCH:
    case gcvPATCH_JPCT:
    case gcvPATCH_NEOCORE:
    case gcvPATCH_RTESTVA:
    case gcvPATCH_BMGUI:
    case gcvPATCH_BASEMARKX:
    case gcvPATCH_FISHNOODLE:
        status = gcvFALSE;
        break;
    default:
        status = gcvTRUE;
        break;
    }

OnError:
    return status;
}

static gctBOOL
_isDepth24App(
    void
    )
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    gceSTATUS status = gcvFALSE;

    gcmONERROR(gcoHAL_GetPatchID(gcvNULL, &patchId));

    switch (patchId)
    {
    case gcvPATCH_NBA2013:
    case gcvPATCH_BARDTALE:
    case gcvPATCH_BUSPARKING3D:
    case gcvPATCH_F18:
    case gcvPATCH_CARPARK:
    case gcvPATCH_FSBHAWAIIF:
        status = gcvTRUE;
        break;
    default:
        status = gcvFALSE;
        break;
    }
OnError:
    return status;
}
#endif

static gcmINLINE EGLBoolean
_AllocRegion(
    struct eglRegion * Region
    )
{
    gctPOINTER ptr;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(EGLint) * 4, &ptr)))
    {
        Region->rects = gcvNULL;
        return EGL_FALSE;
    }

    Region->rects    = ptr;
    Region->numRects = Region->maxNumRects = 1;

    return EGL_TRUE;
}

static gcmINLINE void
_FreeRegion(
    struct eglRegion * Region
    )
{
    if (Region->rects)
    {
        gcmOS_SAFE_FREE(gcvNULL, Region->rects);
    }

    Region->numRects = Region->maxNumRects = 0;
}

static void
_AddDumpSurface(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    )
{
    gctUINT32 address[3] = { 0 };
    gctPOINTER memory[3] = { gcvNULL };
    gctUINT width, height;
    gctINT stride;
    gceSTATUS status;
    gcoSURF surface = gcvNULL;

    if ( (Thread->dump != gcvNULL) && (Surface->renderTarget != gcvNULL) )
    {
        /* Lock the render target. */
        gcmONERROR(gcoSURF_Lock(Surface->renderTarget, address, memory));
        surface = Surface->renderTarget;

        /* Get the size of the render target. */
        gcmONERROR(gcoSURF_GetAlignedSize(Surface->renderTarget,
                                          &width,
                                          &height,
                                          &stride));

        /* Dump the allocation for the render target. */
        gcmONERROR(gcoDUMP_AddSurface(Thread->dump,
                                      width,
                                      height,
                                      Surface->renderTargetFormat,
                                      address[0],
                                      stride * height));

        /* Unlock the render target. */
        gcmONERROR(gcoSURF_Unlock(Surface->renderTarget, memory[0]));
        surface = gcvNULL;
    }

    if ( (Thread->dump != gcvNULL) && (Surface->depthBuffer != gcvNULL) )
    {
        /* Lock the depth buffer. */
        gcmONERROR(gcoSURF_Lock(Surface->depthBuffer, address, memory));
        surface = Surface->depthBuffer;

        /* Get the size of the depth buffer. */
        gcmONERROR(gcoSURF_GetAlignedSize(Surface->depthBuffer,
                                          &width,
                                          &height,
                                          &stride));

        /* Dump the allocation for the depth buffer. */
        gcmONERROR(gcoDUMP_AddSurface(Thread->dump,
                                      width,
                                      height,
                                      Surface->depthFormat,
                                      address[0],
                                      stride * height));

        /* Unlock the depth buffer. */
        gcmONERROR(gcoSURF_Unlock(Surface->depthBuffer, memory[0]));
        surface = gcvNULL;
    }

OnError:
    if (surface != gcvNULL)
    {
        /* Unlock the failed surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(surface, memory[0]));
    }

    return;
}

static gceSTATUS
_InitDrawable(
    IN VEGLSurface Surface
)
{
    /* Prepare the API drawable struct */
    Surface->drawable.config         = &Surface->config;
    Surface->drawable.width          = Surface->config.width;
    Surface->drawable.height         = Surface->config.height;
    Surface->drawable.rtHandles[0]       = (void*)Surface->renderTarget;
    Surface->drawable.prevRtHandles[0]   = (void*)Surface->prevRenderTarget;
    Surface->drawable.depthHandle    = (Surface->config.depthSize > 0) ?
                                       (void*)Surface->depthBuffer: gcvNULL;
    /* If config requires stencil buffer, EGL chooses to combine it with depth buffer */
    Surface->drawable.stencilHandle  = (Surface->config.stencilSize > 0) ?
                                       (void*)Surface->depthBuffer : gcvNULL;
    Surface->drawable.private        = gcvNULL;
    Surface->drawable.destroyPrivate = gcvNULL;

    /* Set preserved flag. */
    if (Surface->renderTarget != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_SetFlags(
            Surface->renderTarget,
            gcvSURF_FLAG_CONTENT_PRESERVED,
            (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
            ));
    }

    if (Surface->renderTarget)
    {
        gcmVERIFY_OK(gcoSURF_SetFlags(
            Surface->renderTarget,
            gcvSURF_FLAG_CONTENT_YINVERTED,
            !(Surface->type & EGL_PBUFFER_BIT)
            ));
    }

    if (Surface->depthBuffer)
    {
        gcmVERIFY_OK(gcoSURF_SetFlags(
            Surface->depthBuffer,
            gcvSURF_FLAG_CONTENT_YINVERTED,
            !(Surface->type & EGL_PBUFFER_BIT)
            ));
    }

    return gcvSTATUS_OK;
}

static EGLint
_CreateSurfaceObjects(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i, num = EGL_WORKER_COUNT;

    do
    {
        /* Verify the surface. */
        gcmASSERT(Surface->renderTarget     == gcvNULL);
        gcmASSERT(Surface->depthBuffer      == gcvNULL);

        /* OpenVG surface? */
        if (Surface->openVG)
        {
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): OPEN VG PIPE.",
                __FUNCTION__, __LINE__
                );

            if (!(Surface->type & EGL_WINDOW_BIT))
            {
                /* Create render target. */
                gcmERR_BREAK(gcoSURF_Construct(
                    gcvNULL,
                    Surface->config.width,
                    Surface->config.height, 1,
                    gcvSURF_BITMAP,
                    Surface->renderTargetFormat,
                    gcvPOOL_DEFAULT,
                    &Surface->renderTarget
                    ));

                /* Set render surface type. */
                gcmERR_BREAK(gcoSURF_SetColorType(
                    Surface->renderTarget, Surface->colorType
                    ));
            }
        }
        else
        {
#if gcdENABLE_3D
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): 3D PIPE.",
                __FUNCTION__, __LINE__
                );

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): render taget=%dx%d, format=%d, samples=%d",
                __FUNCTION__, __LINE__,
                Surface->config.width,
                Surface->config.height,
                Surface->renderTargetFormat,
                Surface->config.samples
                );

            /* Defere allocation render target for window surface. */
            if (!(Surface->type & EGL_WINDOW_BIT))
            {
                EGLBoolean success;

                /* Create render target. */
                success = veglCreateRenderTarget(Thread, Surface);

                if (!success)
                {
                    veglSetEGLerror(Thread, EGL_BAD_ALLOC);
                    break;
                }
            }

            /* Create depth buffer. */
            if (Surface->depthFormat != gcvSURF_UNKNOWN)
            {
                gcePATCH_ID patchId = gcvPATCH_INVALID;
                gctUINT samples;
                gceSURF_TYPE type;

                samples = Surface->config.samples;

                gcoHAL_GetPatchID(gcvNULL, &patchId);

                if ((patchId == gcvPATCH_ANTUTU)
                ||  (patchId == gcvPATCH_ANTUTU4X)
                ||  (patchId == gcvPATCH_ANTUTU5X))
                {
                    samples = 0;
                }

                type = (samples > 1) ? gcvSURF_DEPTH : (gcvSURF_DEPTH | gcvSURF_CREATE_AS_DISPLAYBUFFER);
                if (Surface->protectedContent)
                {
                    type = (gceSURF_TYPE) (type | gcvSURF_PROTECTED_CONTENT);
                }

                /* Create depth buffer. */
                gcmERR_BREAK(gcoSURF_Construct(
                    gcvNULL,
                    Surface->config.width,
                    Surface->config.height, 1,
                    type,
                    Surface->depthFormat,
                    gcvPOOL_DEFAULT,
                    &Surface->depthBuffer
                    ));

                /* Set multi-sampling size. */
                gcmERR_BREAK(gcoSURF_SetSamples(
                    Surface->depthBuffer,
                    samples
                    ));
            }
#else
            gcmERR_BREAK(gcvSTATUS_GENERIC_IO);
#endif
        }

        _InitDrawable(Surface);

        /* Initialize worker count. */
        Surface->totalWorkerCount = num;
        Surface->freeWorkerCount  = num;

        for (i = 0; i < num; i++)
        {
            VEGLWorkerInfo worker = &Surface->workers[i];

            worker->draw                   = gcvNULL;
            worker->backBuffer.context     = gcvNULL;
            worker->backBuffer.surface     = gcvNULL;
            worker->next  = Surface->availableWorkers;
            Surface->availableWorkers = worker;

            worker->region.numRects     = worker->region.maxNumRects     = 0;
            worker->damageHint.numRects = worker->damageHint.maxNumRects = 0;
        }

        if (gcmIS_ERROR(status))
        {
            break;
        }

        gcmERR_BREAK(gcoOS_CreateMutex(gcvNULL, &Surface->workerMutex));

        /* Create worker avaiable signal. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvTRUE,
            &Surface->workerAvaiableSignal
            ));

        gcmERR_BREAK(gcoOS_Signal(
            gcvNULL,
            Surface->workerAvaiableSignal,
            gcvTRUE
            ));

        /* Create workers done signal. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvTRUE,
            &Surface->workerDoneSignal
            ));

        gcmERR_BREAK(gcoOS_Signal(
            gcvNULL,
            Surface->workerDoneSignal,
            gcvTRUE
            ));

        if (!_AllocRegion(&Surface->clipRegion))
        {
            gcmERR_BREAK(gcvSTATUS_OUT_OF_MEMORY);
        }

        if (!_AllocRegion(&Surface->damageHint))
        {
            gcmERR_BREAK(gcvSTATUS_OUT_OF_MEMORY);
        }

        /* Success. */
        return EGL_SUCCESS;
    }
    while (gcvFALSE);

    /* Allocation failed. */
    gcmFATAL("Failed to allocate surface objects: %d", status);

    /* Roll back. */
    _FreeRegion(&Surface->clipRegion);

    if (Surface->workerMutex != gcvNULL)
    {
        gcmVERIFY_OK(
            gcoOS_DeleteMutex(gcvNULL, Surface->workerMutex));
    }

    if (Surface->workerAvaiableSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Surface->workerAvaiableSignal));
        Surface->workerAvaiableSignal = gcvNULL;
    }

    if (Surface->workerDoneSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Surface->workerDoneSignal));
        Surface->workerDoneSignal = gcvNULL;
    }

    if (Surface->depthBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Surface->depthBuffer));
        Surface->depthBuffer = gcvNULL;
    }

    if (Surface->renderTarget != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Surface->renderTarget));
        Surface->renderTarget = gcvNULL;
    }

    if (Surface->drawable.destroyPrivate)
    {
        Surface->drawable.destroyPrivate(&Surface->drawable);
    }

    gcoOS_ZeroMemory(&Surface->drawable, sizeof(EGLDrawable));

    /* Return error code. */
    return EGL_BAD_ALLOC;
}

static gceSTATUS
_DestroySurfaceObjects(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
#if gcdENABLE_3D
    gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
#endif

    do
    {
        /* Free the workers. */
        gctUINT32 i, num = EGL_WORKER_COUNT;

        if (Surface->workerDoneSignal != gcvNULL)
        {
            /* Wait for workers to become idle. */
            gcmVERIFY_OK(gcoOS_WaitSignal(
                gcvNULL, Surface->workerDoneSignal, gcvINFINITE));
        }

        _FreeRegion(&Surface->clipRegion);
        _FreeRegion(&Surface->damageHint);

        for (i = 0; i < num; i++)
        {
            /* Make sure all worker threads are finished. */
            if (Surface->workers[i].signal != gcvNULL)
            {
                gcmVERIFY_OK(
                    gcoOS_DestroySignal(gcvNULL, Surface->workers[i].signal));

                Surface->workers[i].signal = gcvNULL;
            }

            _FreeRegion(&Surface->workers[i].region);
            _FreeRegion(&Surface->workers[i].damageHint);
        }

        /* Destroy worker mutex. */
        if (Surface->workerMutex != gcvNULL)
        {
            gcmERR_BREAK(
                gcoOS_DeleteMutex(gcvNULL, Surface->workerMutex));
        }

        if (Surface->workerAvaiableSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Surface->workerAvaiableSignal));
            Surface->workerAvaiableSignal = gcvNULL;
        }

        if (Surface->workerDoneSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Surface->workerDoneSignal));
            Surface->workerDoneSignal = gcvNULL;
        }

        if (Surface->depthBuffer != gcvNULL)
        {
#if gcdENABLE_3D
            surfView.surf = Surface->depthBuffer;
            /* Flush pixels and disable the tile status. */
            gcmERR_BREAK(gcoSURF_DisableTileStatus(
                &surfView, gcvFALSE
                ));
#endif

            /* Destroy the depth buffer */
            gcmERR_BREAK(gcoSURF_Destroy(Surface->depthBuffer));
            Surface->depthBuffer = gcvNULL;
        }

        if (Surface->renderTarget != gcvNULL)
        {
#if gcdENABLE_3D
            surfView.surf = Surface->renderTarget;
            /* Flush pixels and disable the tile status. */
            gcmERR_BREAK(gcoSURF_DisableTileStatus(
                &surfView, gcvFALSE
                ));
#endif

            /* Destroy the render target. */
            gcmERR_BREAK(gcoSURF_Destroy(Surface->renderTarget));
            Surface->renderTarget = gcvNULL;
        }

        for (i = 0; i < EGL_WORKER_COUNT; i++)
        {
            if (Surface->damage[i].numRects != 0)
            {
                gcmOS_SAFE_FREE(gcvNULL, Surface->damage[i].rects);
                Surface->damage[i].numRects = 0;
            }
        }

        if (Surface->prevRenderTarget != gcvNULL)
        {
            /* Dereference previous render target. */
            gcmERR_BREAK(gcoSURF_Destroy(Surface->prevRenderTarget));
            Surface->prevRenderTarget = gcvNULL;
        }


        if (Surface->lockBuffer != gcvNULL)
        {
            /* Destroy the bitmap surface. */
            gcmERR_BREAK(gcoSURF_Destroy(Surface->lockBuffer));
            Surface->lockBuffer = gcvNULL;
            Surface->lockBits   = gcvNULL;
        }

        if (Surface->lockBufferMirror != gcvNULL)
        {
            /* Destroy the bitmap surface. */
            gcmERR_BREAK(gcoSURF_Destroy(Surface->lockBufferMirror));
        }

        if (Surface->drawable.destroyPrivate)
        {
            Surface->drawable.destroyPrivate(&Surface->drawable);
        }

        gcoOS_ZeroMemory(&Surface->drawable, sizeof(EGLDrawable));
        if (Surface->openVG)
        {
            /* Flush VG event queue */
            gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}

EGLint
veglResizeSurface(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    EGLint eglResult = EGL_SUCCESS;
    gcoSURF prevRenderTarget = gcvNULL;
    gcoSURF prevDepth        = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Surface=0x%x Width=%u Height=%u", Surface, Width, Height);

    do
    {
        VEGLThreadData thread;
        VEGLContext current;
        VEGLPlatform platform = Display->platform;
        gcmASSERT(Surface->type & EGL_WINDOW_BIT);

        /* Get thread data. */
        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(
                gcvLEVEL_ERROR,
                "%s(%d): veglGetThreadData failed.",
                __FUNCTION__, __LINE__
                );

            eglResult = EGL_BAD_SURFACE;
            break;
        }

        current = thread->context;
        if (current && (current->draw == Surface || current->read == Surface))
        {
            if (!_SetDrawable(thread, current, gcvNULL, gcvNULL))
            {
                /* Error freeing surface objects. */
                eglResult = EGL_BAD_ALLOC;
                break;
            }
        }

        if (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
        {
            /* Reference previous render target. */
            prevRenderTarget = (Surface->renderMode > 0)
                             ? Surface->prevRenderTarget
                             : Surface->renderTarget;

            if (prevRenderTarget)
            {
                gcoSURF_ReferenceSurface(prevRenderTarget);
            }

            /* Reference previous depth buffer. */
            prevDepth = Surface->depthBuffer;

            if (prevDepth)
            {
                gcoSURF_ReferenceSurface(prevDepth);
            }
        }

        /* Destroy existing objects. */
        status = _DestroySurfaceObjects(thread, Surface);

        if (gcmIS_ERROR(status))
        {
            /* Error feeing surface objects. */
            eglResult = EGL_BAD_ALLOC;
            break;
        }

        /* Set new parameters. */
        Surface->config.width       = Width;
        Surface->config.height      = Height;

        /* Create surface objects. */
        eglResult = _CreateSurfaceObjects(thread, Surface);

        if (eglResult != EGL_SUCCESS)
        {
            break;
        }

        /* Similar to eglMakeCurrent here. */
        /* Cancel window back buffer. */
        if (Surface->backBuffer.surface != gcvNULL)
        {
            platform->cancelWindowBackBuffer(Display, Surface, &Surface->backBuffer);

            Surface->backBuffer.context = gcvNULL;
            Surface->backBuffer.surface = gcvNULL;
        }

        gcmASSERT(Surface->bound);
        platform->unbindWindow(Display, Surface);
        Surface->bound = EGL_FALSE;

        /* Re-bind window for rendering. */
        if (!platform->bindWindow(Display, Surface, &Surface->renderMode))
        {
            eglResult = EGL_BAD_NATIVE_WINDOW;
            break;
        }

        Surface->bound = EGL_TRUE;

        /* Must use new swap model for direct rendering. */
        Surface->newSwapModel = (Surface->renderMode > 0);

#if defined(ANDROID)
        /* Always use new swap model for android. */
        Surface->newSwapModel = gcvTRUE;
#endif

        if (Surface->newSwapModel)
        {
            EGLBoolean result;

            /* Get window back buffer for new swap model. */
            result = platform->getWindowBackBuffer(Display, Surface, &Surface->backBuffer);

            if (!result)
            {
                /*
                 * Do not break here. Need update drawable to client in
                 * direct rendering mode. See below.
                 */
                eglResult = EGL_BAD_NATIVE_WINDOW;
            }
        }

        if (Surface->renderMode > 0)
        {
            /* Get render target from window back buffer. */
            Surface->renderTarget = Surface->backBuffer.surface;

            if (Surface->renderTarget)
            {
                /* Reference external surface. */
                gcoSURF_ReferenceSurface(Surface->renderTarget);

                /* Set preserve flag. */
                gcmVERIFY_OK(gcoSURF_SetFlags(
                    Surface->renderTarget,
                    gcvSURF_FLAG_CONTENT_PRESERVED,
                    (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
                    ));

                /* Reset content updated flag. */
                gcmVERIFY_OK(gcoSURF_SetFlags(
                    Surface->renderTarget,
                    gcvSURF_FLAG_CONTENT_UPDATED,
                    gcvFALSE
                    ));
            }

            /* Sync drawable with renderTarget. */
            Surface->drawable.rtHandles[0]     = Surface->renderTarget;
            Surface->drawable.prevRtHandles[0] = gcvNULL;
        }
        else if (Surface->renderTarget == gcvNULL)
        {
            EGLBoolean success;

            /* Create render target. */
            success = veglCreateRenderTarget(thread, Surface);

            if (success != EGL_TRUE)
            {
                /* Error creating render target. */
                eglResult = EGL_BAD_ALLOC;
                break;
            }
        }

        if (prevRenderTarget)
        {
#if gcdENABLE_VG
            if (Surface->openVG)
            {
                gcoSURF_Copy(Surface->renderTarget, prevRenderTarget);
            }
            else
#endif
            {
#if gcdENABLE_3D
                /* Copy pixels from previous render target. */
                gcsSURF_VIEW srcView = {prevRenderTarget, 0, 1};
                gcsSURF_VIEW trgView = {Surface->renderTarget, 0, 1};

                gcoSURF_ResolveRect(&srcView, &trgView, gcvNULL);
#endif
            }
        }

        if (prevDepth)
        {
#if gcdENABLE_VG
            if (Surface->openVG)
            {
                gcoSURF_Copy(Surface->depthBuffer, prevDepth);
            }
            else
#endif
            {
#if gcdENABLE_3D
                /* Copy pixels from previous render target. */
                gcsSURF_VIEW srcView = {prevDepth, 0, 1};
                gcsSURF_VIEW trgView = {Surface->depthBuffer, 0, 1};

                gcoSURF_ResolveRect(&srcView, &trgView, gcvNULL);
#endif
            }
        }

        if (current && (current->draw == Surface || current->read == Surface))
        {
            VEGLDrawable drawable = (current->draw == Surface)
                                 ? &Surface->drawable
                                 : &current->draw->drawable;
            VEGLDrawable readable = (current->read == Surface)
                                 ? &Surface->drawable
                                 : &current->read->drawable;

            if (!_SetDrawable(thread, current, drawable, readable))
            {
                /* Error feeing surface objects. */
                eglResult = EGL_BAD_ALLOC;
                break;
            }
        }

        /* eglResult could be failure here. */
    }
    while (gcvFALSE);

    if (prevRenderTarget)
    {
        /* Dereference previous render target. */
        gcmVERIFY_OK(gcoSURF_Destroy(prevRenderTarget));
    }

    if (prevDepth)
    {
        /* Dereference previous depth. */
        gcmVERIFY_OK(gcoSURF_Destroy(prevDepth));
    }

    /* Return error code. */
    gcmFOOTER_ARG("return=%d", eglResult);
    return eglResult;
}

void veglGetFormat(
    IN VEGLThreadData Thread,
    IN VEGLConfig Config,
    OUT gceSURF_FORMAT * RenderTarget,
    OUT gceSURF_FORMAT * DepthBuffer
    )
{
    gceSURF_FORMAT requestFormat = gcvSURF_UNKNOWN;
    gceHARDWARE_TYPE currentType  = gcvHARDWARE_INVALID;

    gcmTRACE_ZONE(
        gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
        "%s(%d): config %d RGBA sizes=%d, %d, %d, %d; depth size=%d",
        __FUNCTION__, __LINE__,
        Config->configId,
        Config->redSize,
        Config->greenSize,
        Config->blueSize,
        Config->alphaSize,
        Config->depthSize
        );

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    if (Thread->openVGpipe &&
        (Config->greenSize != 8) && (Config->greenSize != 6)
#if gcdENABLE_VG
        && (Config->greenSize != 0)
#endif
        )
    {
        gcmTRACE_ZONE(
            gcvLEVEL_WARNING, gcdZONE_EGL_SURFACE,
            "%s(%d): unsupported OpenVG target",
            __FUNCTION__, __LINE__
            );
    }
#endif

    /* Get current hardwaret type. */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

    switch (Config->greenSize)
    {
#if gcdENABLE_VG
    case 0:
        if (Thread->openVGpipe &&
            (Config->redSize == 0) && (Config->alphaSize == 8))
        {
            requestFormat = gcvSURF_A8;
        }
        else
        {
            gcmFATAL("Unsupported format (green size=%d)", Config->greenSize);
        }
        break;
#endif
    case 4:
        requestFormat = (Config->alphaSize == 0)
            ? gcvSURF_X4R4G4B4
            : gcvSURF_A4R4G4B4;
        break;

    case 5:
        requestFormat = (Config->alphaSize == 0)
            ? gcvSURF_X1R5G5B5
            : gcvSURF_A1R5G5B5;
        break;

    case 6:
        requestFormat = gcvSURF_R5G6B5;
        break;

    case 8:
        if (Config->bufferSize == 16)
        {
            gcmASSERT(Thread->openVGpipe == gcvFALSE);

            requestFormat = gcvSURF_YUY2;
        }
        else
        {
            requestFormat = (Config->alphaSize == 0)
                ? gcvSURF_X8R8G8B8
                : gcvSURF_A8R8G8B8;
        }
        break;

    default:
        gcmFATAL("Unsupported format (green size=%d)", Config->greenSize);
    }

    gcmASSERT(RenderTarget);

#if gcdENABLE_3D
    if ((currentType == gcvHARDWARE_3D) && (requestFormat != gcvSURF_UNKNOWN))
    {
        gcmVERIFY_OK(gco3D_GetClosestRenderFormat(gcvNULL, requestFormat, RenderTarget));
    }
    else
#endif
    {
        *RenderTarget = requestFormat;
    }

    if (DepthBuffer != gcvNULL)
    {
        switch (Config->depthSize)
        {
        case 0:
            if (Config->stencilSize > 0)
            {
                if (gcvSTATUS_TRUE == gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_S8_ONLY_RENDERING))
                {
                    requestFormat = gcvSURF_S8;
                }
                else
                {
                    requestFormat = gcvSURF_X24S8;
                }
            }
            else
            {
                requestFormat = gcvSURF_UNKNOWN;
            }
            break;


        case 16:
#if defined(ANDROID) && gcdENABLE_3D
            if (_isDepth24App())
            {
                requestFormat = gcvSURF_D24X8;
            }
            else
#endif
            {
                gcmASSERT(Config->stencilSize == 0);
                requestFormat = gcvSURF_D16;
            }
            break;

        case 24:
            if (Config->stencilSize > 0)
            {
                requestFormat = gcvSURF_D24S8;
            }
            else
            {
                requestFormat = gcvSURF_D24X8;
            }
            break;

        default:
            gcmFATAL("Unsupported format (depth size=%d)", Config->depthSize);
        }

        {
#if gcdENABLE_3D
            gcePATCH_ID patchId = gcvPATCH_INVALID;

            gcoHAL_GetPatchID(gcvNULL, &patchId);
            if ((requestFormat != gcvSURF_UNKNOWN) &&
                (requestFormat == gcvSURF_D16) &&
                (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COMPRESSION_V1) == gcvTRUE) &&
                (patchId == gcvPATCH_GTFES30)
                )
            {
                *DepthBuffer = gcvSURF_D24X8;
            }
            else
#endif
            {
                *DepthBuffer = requestFormat;
            }
        }
    }

    gcmTRACE_ZONE(
        gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
        "%s(%d): chosen render target=%d; chosen depth=%d",
        __FUNCTION__, __LINE__,
        *RenderTarget,
        DepthBuffer ? *DepthBuffer : 0
        );
}

static VEGLSurface
_InitializeSurface(
    IN VEGLThreadData Thread,
    IN VEGLConfig Config,
    IN EGLint Type
    )
{
    VEGLSurface surface;
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;

    /* Allocate the surface structure. */
    status = gcoOS_Allocate(gcvNULL,
                            sizeof(struct eglSurface),
                            &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Out of memory. */
        veglSetEGLerror(Thread, EGL_BAD_ALLOC);
        return gcvNULL;
    }

    gcoOS_ZeroMemory(pointer, gcmSIZEOF(struct eglSurface));

    surface = pointer;

    /* Initialize the surface object. */
    surface->resObj.signature = EGL_SURFACE_SIGNATURE;
    surface->type             = Type;
    surface->buffer           = EGL_BACK_BUFFER;
    surface->colorType        = gcvSURF_COLOR_UNKNOWN;
    surface->initialFrame     = EGL_TRUE;

    surface->vgColorSpace     = EGL_VG_COLORSPACE_sRGB;
    surface->vgAlphaFormat    = EGL_VG_ALPHA_FORMAT_NONPRE;

    if (Type & EGL_VG_COLORSPACE_LINEAR_BIT)
    {
        surface->colorType |= gcvSURF_COLOR_LINEAR;
    }

    if (Type & EGL_VG_ALPHA_FORMAT_PRE_BIT)
    {
        surface->colorType |= gcvSURF_COLOR_ALPHA_PRE;
    }

    surface->swapBehavior       = EGL_BUFFER_DESTROYED;
    surface->multisampleResolve = EGL_MULTISAMPLE_RESOLVE_DEFAULT;

    /* PBuffer attributes. */
    if (Type & EGL_PBUFFER_BIT)
    {
        surface->textureFormat  = EGL_NO_TEXTURE;
        surface->textureTarget  = EGL_NO_TEXTURE;
    }

    surface->rtFormatChanged  = gcvFALSE;
    surface->protectedContent = EGL_FALSE;

    /* Render mode and swap model. */
    surface->renderMode       = -1;
    surface->newSwapModel     = EGL_FALSE;

    if (Config != gcvNULL)
    {
#if gcdENABLE_3D
        gcePATCH_ID patchId = gcvPATCH_INVALID;
        gcoHAL_GetPatchID(gcvNULL, &patchId);
#endif

        /* Get configuration formats. */
        veglGetFormat(
            Thread,
            Config,
            &surface->renderTargetFormat,
            &surface->depthFormat
            );

#if gcdENABLE_3D
#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_FORCE_16BIT_RENDER_TARGET
        if (surface->renderTargetFormat == gcvSURF_R5G6B5 && patchId == gcvPATCH_DEBUG)
        {
            gcmPRINT(" The original format of render target is rgb565, transform is unnecessary!");
        }
#endif

        if ((Config->bufferSize < 32)
#if defined(ANDROID)
            && _ForceRenderTarget()
#endif
#if gcdENABLE_VG
            /* For 3DVG, dither is the same as es. */
            && !(Thread->openVGpipe && (Thread->api == EGL_OPENVG_API))
#endif
           )
        {
            gctBOOL force32bitsRT = gcvFALSE;

            if ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX) != gcvTRUE) ||
                (patchId == gcvPATCH_GTFES30) ||
                (patchId == gcvPATCH_DEQP)    ||
                (patchId == gcvPATCH_TRIAL)
               )
            {
                force32bitsRT = gcvTRUE;
            }
            else if ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PE_DITHER_FIX2) != gcvTRUE) &&
                     (Config->greenSize == 4 || Config->greenSize == 5)
                    )
            {
                force32bitsRT = gcvTRUE;
            }

            if (force32bitsRT)
            {
                gceSURF_FORMAT rtFormat = gcvSURF_UNKNOWN;

                switch (surface->renderTargetFormat)
                {
                case gcvSURF_X4R4G4B4:
                case gcvSURF_X1R5G5B5:
                case gcvSURF_R5G6B5:
                case gcvSURF_X8R8G8B8:
                    rtFormat = gcvSURF_X8R8G8B8;
                    break;

                case gcvSURF_A4R4G4B4:
                    rtFormat = gcvSURF_A8R8G8B8;
                    break;

                case gcvSURF_A1R5G5B5:
                    rtFormat = (patchId == gcvPATCH_DEQP) ? gcvSURF_A1R5G5B5 : gcvSURF_A8R8G8B8;
                    break;

                case gcvSURF_YUY2:
                    rtFormat = gcvSURF_YUY2;
                    break;

                default:
                    gcmFATAL("Error: Unknow 16bpp format: %d", surface->renderTargetFormat);
                    break;
                }

                if (surface->renderTargetFormat != rtFormat)
                {
                    surface->rtFormatChanged = gcvTRUE;
                    surface->renderTargetFormat = rtFormat;
                }
            }
        }

        if (patchId == gcvPATCH_ANTUTU || patchId == gcvPATCH_ANTUTU4X
#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_FORCE_16BIT_RENDER_TARGET
                || patchId == gcvPATCH_DEBUG
#endif
        )
        {
            gceCHIPMODEL chipModel;
            gctUINT32 chipRevision;

            gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL);

            if (!(chipModel == gcv860 && chipRevision == 0x4645)
                && !(chipModel == gcv1000 && chipRevision == 0x5036))
            {
                surface->renderTargetFormat = gcvSURF_R5G6B5;
            }
        }

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NONE_DEPTH
        if (patchId == gcvPATCH_DEBUG)
        {
            surface->depthFormat = gcvSURF_UNKNOWN;
        }
#endif
        if (patchId == gcvPATCH_TEMPLERUN)
        {
            surface->renderTargetFormat = gcvSURF_R5G6B5;
        }
#endif

        /* Copy config information */
        gcoOS_MemCopy(&surface->config, Config, sizeof(surface->config));
    }

    /* Success. */
    return surface;
}

static EGLint
_CreateSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    VEGLPlatform platform = Display->platform;
    EGLint eglResult = EGL_SUCCESS;

    do
    {
        EGLint surfaceType;
        EGLint width, height;

        /* Determine the surface type. */
        surfaceType
            = Surface->type
            & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT | EGL_PIXMAP_BIT);

        /***********************************************************************
        ** WINDOW SURFACE.
        */

        if (surfaceType == EGL_WINDOW_BIT)
        {
            EGLBoolean result;

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): EGL_WINDOW_BIT",
                __FUNCTION__, __LINE__
                );

            /* Query window size. */
            result = platform->getWindowSize(Display, Surface, &width, &height);

            if (!result)
            {
                eglResult = EGL_BAD_NATIVE_WINDOW;
                break;
            }

            /* Save width and height. */
            Surface->config.width  = width;
            Surface->config.height = height;
        }

        /***********************************************************************
        ** PBUFFER SURFACE.
        */

        else if (surfaceType == EGL_PBUFFER_BIT)
        {
            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): EGL_PBUFFER_BIT",
                __FUNCTION__, __LINE__
                );

            /* Set width and height from configuration. */
            width        = Surface->config.width;
            height       = Surface->config.height;
        }

        /***********************************************************************
        ** PIXMAP SURFACE.
        */

        else if (surfaceType == EGL_PIXMAP_BIT)
        {
            EGLBoolean result;

            gcmTRACE_ZONE(
                gcvLEVEL_VERBOSE, gcdZONE_EGL_SURFACE,
                "%s(%d): EGL_PIXMAP_BIT",
                __FUNCTION__, __LINE__
                );

            /* Get pixmap size. */
            result = platform->getPixmapSize(Display,
                                             Surface->pixmap,
                                             Surface->pixInfo,
                                             &width,
                                             &height);

            if (!result)
            {
                eglResult = EGL_BAD_NATIVE_PIXMAP;
                break;
            }

            /* Set configuration width and height. */
            Surface->config.width  = width;
            Surface->config.height = height;
        }

        /***********************************************************************
        ** UNKNOWN SURFACE.
        */

        else
        {
            gcmFATAL("ERROR: Unknown surface type: 0x%04X", surfaceType);
            eglResult = EGL_BAD_PARAMETER;
            break;
        }

        /* Get rid of compiler warnings. */
        (void) width;
        (void) height;

        /* Determine whether OpenVG pipe is present and OpenVG API is active. */
        Surface->openVG = Thread->openVGpipe && (Thread->api == EGL_OPENVG_API);

#if gcdENABLE_VG
        if (Surface->openVG)
        {
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG));
        }
#endif

        /* Create surface objects. */
        eglResult = _CreateSurfaceObjects(Thread, Surface);

        if (eglResult != EGL_SUCCESS)
        {
            break;
        }

        /* Dump the surface. */
        _AddDumpSurface(Thread, Surface);

        /* Success. */
        return EGL_SUCCESS;
    }
    while (gcvFALSE);

    /* Return the error code. */
    return eglResult;
}

static void
_DestroySurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    VEGLPlatform platform = Display->platform;
    gcmVERIFY_OK(_DestroySurfaceObjects(Thread, Surface));

    if (Surface->type & EGL_WINDOW_BIT)
    {
        if (Surface->hwnd != gcvNULL)
        {
            if (Surface->backBuffer.surface != gcvNULL)
            {
                platform->cancelWindowBackBuffer(Display, Surface,
                                                 &Surface->backBuffer);

                Surface->backBuffer.context = gcvNULL;
                Surface->backBuffer.surface = gcvNULL;
            }

            if (Surface->winInfo)
            {
                /* Disconnect window. */
                platform->disconnectWindow(Display, Surface);
            }

            Surface->hwnd = gcvNULL;
        }
    }
    else if (Surface->type & EGL_PIXMAP_BIT)
    {
        if (Surface->pixmap != gcvNULL)
        {
            if (Surface->pixmapSurface != gcvNULL)
            {
                /* Dereference surface. */
                gcoSURF_Destroy(Surface->pixmapSurface);
                Surface->pixmapSurface = gcvNULL;
            }

            if (Surface->pixInfo)
            {
                /* Disconnect pixmap. */
                platform->disconnectPixmap(Display,
                                           (void *) Surface->pixmap,
                                           Surface->pixInfo);
            }

            Surface->pixmap = gcvNULL;
        }
    }

    if (Surface->reference != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Surface->reference));
        Surface->reference = gcvNULL;
    }
}

static EGLBoolean
_MapLockedBuffer(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    )
{
    gceSTATUS status;
    gctPOINTER lockBits[3] = {gcvNULL};

    if (Surface->lockBufferMirror != gcvNULL)
    {
        /* Retrieve the buffer. */
        Surface->lockBuffer       = Surface->lockBufferMirror;
        Surface->lockBufferMirror = gcvNULL;
    }
    else
    {
        /* Create a bitmap surface. */
        status = gcoSURF_Construct(
            gcvNULL,
            Surface->config.width,
            Surface->config.height, 1,
            gcvSURF_BITMAP,
            Surface->renderTargetFormat,
            gcvPOOL_DEFAULT,
            &Surface->lockBuffer
            );

        /* Create fail. */
        if (gcmIS_ERROR(status))
        {
            veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
            return EGL_FALSE;
        }
    }

    /* Lock bitmap address. */
    status = gcoSURF_Lock(
        Surface->lockBuffer,
        gcvNULL,
        lockBits
        );
    Surface->lockBits = lockBits[0];

    if (gcmIS_ERROR(status))
    {
        /* Roll back and return false. */
        gcoSURF_Destroy(Surface->lockBuffer);

        veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
        return EGL_FALSE;
    }

    status = gcoSURF_GetAlignedSize(Surface->lockBuffer,
                                gcvNULL, gcvNULL, &Surface->lockBufferStride);

    if (gcmIS_ERROR(status))
    {
        /* Roll back and return false. */
        gcoSURF_Destroy(Surface->lockBuffer);

        veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
        return EGL_FALSE;
    }

    /* If Preserve bit is set. */
    if (Surface->lockPreserve && (Surface->renderTarget != gcvNULL))
    {
#if gcdENABLE_VG && gcdENABLE_3D
        if (Surface->openVG)
#endif
        {
#if gcdENABLE_VG
            status = gcoSURF_Copy(Surface->renderTarget, Surface->lockBuffer);
#endif
        }
#if gcdENABLE_VG && gcdENABLE_3D
        else
#endif
        {
#if gcdENABLE_3D
            gcsSURF_VIEW rtView = {Surface->renderTarget, 0, 1};
            gcsSURF_VIEW lockView = {Surface->lockBuffer, 0, 1};

            /* Resolve color buffer to bitmap. */
            status = gcoSURF_ResolveRect(&rtView, &lockView, gcvNULL);
#endif
        }

        if (gcmIS_ERROR(status))
        {
            veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
            return EGL_FALSE;
        }

        status = gcoHAL_Commit(gcvNULL, gcvTRUE);

        if (gcmIS_ERROR(status))
        {
            veglSetEGLerror(Thread,  EGL_BAD_ACCESS);
            return EGL_FALSE;
        }
    }

    veglSetEGLerror(Thread,  EGL_SUCCESS);
    return EGL_TRUE;
}

static EGLint
_GetLockedBufferPixelChannelOffset(
    IN VEGLSurface Surface,
    IN EGLint      Attribute
    )
{
    gceSURF_FORMAT format;
    EGLint         redOffset       = 0;
    EGLint         greenOffset     = 0;
    EGLint         blueOffset      = 0;
    EGLint         alphaOffset     = 0;
    EGLint         luminanceOffset = 0;

    if (!Surface->locked || !Surface->lockBuffer)
    {
        return 0;
    }

    format = Surface->lockBufferFormat;

    switch(format)
    {
    case gcvSURF_A8R8G8B8:
        alphaOffset = 24;
        redOffset   = 16;
        greenOffset = 8;
        break;

    case gcvSURF_X8R8G8B8:
        redOffset   = 16;
        greenOffset = 8;
        break;

    case gcvSURF_R5G6B5:
        redOffset   = 11;
        greenOffset = 6;
        break;

    case gcvSURF_A4R4G4B4:
        alphaOffset = 12;
        redOffset   = 8;
        greenOffset = 4;
        break;

    case gcvSURF_A1R5G5B5:
        alphaOffset = 15;
        redOffset   = 10;
        greenOffset = 5;
        break;

    default:
        break;
    }

    switch (Attribute)
    {
    case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
        return redOffset;

    case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
        return greenOffset;

    case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
        return blueOffset;

    case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
        return alphaOffset;

    case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
        return luminanceOffset;

    default:
        break;
    }

    return 0;
}

EGLBoolean
veglReferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    gctINT32 reference;

    gcmHEADER_ARG("Thread=0x%x Surface=0x%x", Thread, Surface);

    if (Surface->reference == gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_AtomConstruct(gcvNULL, &Surface->reference)))
        {
            /* Out of memory. */
            veglSetEGLerror(Thread, EGL_BAD_ALLOC);
            gcmFOOTER_ARG("return=%d", EGL_FALSE);
            return EGL_FALSE;
        }
    }

    /* Increment surface reference counter. */
    if (gcmIS_ERROR(gcoOS_AtomIncrement(gcvNULL,
                                        Surface->reference,
                                        &reference)))
    {
        /* Error. */
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;
}

void
veglDereferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN EGLBoolean Always
    )
{
    gctINT32 reference = 0;

    gcmHEADER_ARG("Thread=0x%x Surface=0x%x Always=%d", Thread, Surface, Always);

    if (Surface->reference != gcvNULL)
    {
        /* Decrement surface reference counter. */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL,
                                         Surface->reference,
                                         &reference));
    }

    if ((reference == 1) || Always)
    {
        _DestroySurface(Thread, Display, Surface);
    }

    gcmFOOTER_NO();
}

EGLBoolean
veglCreateRenderTarget(
    IN VEGLThreadData Thread,
    IN VEGLSurface Surface
    )
{
    gceSTATUS status;
#if gcdENABLE_3D
    gcePATCH_ID patchId = gcvPATCH_INVALID;
#endif
    gctUINT samples;
    gceSURF_TYPE type;

    samples = Surface->config.samples;

#if gcdENABLE_3D
    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if ((patchId == gcvPATCH_ANTUTU)
    ||  (patchId == gcvPATCH_ANTUTU4X)
    ||  (patchId == gcvPATCH_ANTUTU5X))
    {
        samples = 0;
    }
#endif

    if (Surface->openVG)
    {
        /* Bitmap for openvg render target. */
        type = gcvSURF_BITMAP;
    }
    else
    {
        type = (samples > 1) ? gcvSURF_RENDER_TARGET : (gcvSURF_RENDER_TARGET | gcvSURF_CREATE_AS_DISPLAYBUFFER);
    }

    if (Surface->protectedContent)
    {
        type = (gceSURF_TYPE) (type | gcvSURF_PROTECTED_CONTENT);
    }

    /* Create render target. */
    gcmONERROR(gcoSURF_Construct(
        gcvNULL,
        Surface->config.width,
        Surface->config.height, 1,
        type,
        Surface->renderTargetFormat,
        gcvPOOL_DEFAULT,
        &Surface->renderTarget
        ));

    /* Set multi-sampling size. */
    gcmONERROR(gcoSURF_SetSamples(
        Surface->renderTarget,
        samples
        ));

    /* Set render surface type. */
    gcmONERROR(gcoSURF_SetColorType(
        Surface->renderTarget, Surface->colorType
        ));

    {
        gctPOINTER memory[3] = {gcvNULL};
        gctUINT8_PTR bits;
        gctINT bitsStride;
        gctUINT alHeight;

        /* Get the stride of render target. */
        gcmONERROR(gcoSURF_GetAlignedSize(
            Surface->renderTarget,
            gcvNULL,
            &alHeight,
            &bitsStride
            ));

        /* Lock render target and clear it with zero. for fixing eglmage test bug. */
        gcmONERROR(gcoSURF_Lock(
            Surface->renderTarget,
            gcvNULL,
            memory
            ));

        bits = (gctUINT8_PTR) memory[0];
        gcoOS_ZeroMemory(
            bits,
            bitsStride * alHeight
            );

        gcmONERROR(gcoSURF_Unlock(
            Surface->renderTarget, memory[0]
            ));
    }

    /* Initialize preserved flag. */
    gcmVERIFY_OK(gcoSURF_SetFlags(
        Surface->renderTarget,
        gcvSURF_FLAG_CONTENT_PRESERVED,
        (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
        ));

    /* Set y-inverted rendering. */
    gcmVERIFY_OK(gcoSURF_SetFlags(
        Surface->renderTarget,
        gcvSURF_FLAG_CONTENT_YINVERTED,
        !(Surface->type & EGL_PBUFFER_BIT)
        ));

    /* Sync drawable with renderTarget. */
    Surface->drawable.rtHandles[0]     = Surface->renderTarget;
    Surface->drawable.prevRtHandles[0] = gcvNULL;

    return EGL_TRUE;

OnError:
    if (Surface->renderTarget != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Surface->renderTarget));
        Surface->renderTarget = gcvNULL;
    }

    return EGL_FALSE;
}

static gcmINLINE EGLAttrib
_AttribValue(
    const void *attrib_list,
    EGLBoolean intAttrib,
    EGLint index
    )
{
    const EGLint * intList       = (const EGLint *) attrib_list;
    const EGLAttrib * attribList = (const EGLAttrib *) attrib_list;

    return intAttrib ? (EGLAttrib) intList[index]
                     : attribList[index];
}

static EGLSurface
veglCreatePlatformWindowSurface(
    EGLDisplay Dpy,
    EGLConfig config,
    void * native_window,
    const void * attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    EGLenum renderBuffer = EGL_BACK_BUFFER;
    VEGLSurface surface;
    EGLBoolean protectedContent = EGL_FALSE;
    EGLint error;
    EGLBoolean result;
    EGLint type = EGL_WINDOW_BIT;
    VEGLConfig eglConfig;
    gceSTATUS status;
#if defined(__QNXNTO__)
    gctSTRING ignoreSubBuf = gcvNULL;
#endif

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_NO_SURFACE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        return EGL_NO_SURFACE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid config. */
    if (((EGLint)(intptr_t)config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid native window handle. */
    if (native_window == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_NATIVE_WINDOW);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)config - 1]);

    /* Parse attributes. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLAttrib attribute = _AttribValue(attrib_list, intAttrib, i);
            EGLAttrib value     = _AttribValue(attrib_list, intAttrib, i + 1);
            switch (attribute)
            {
            case EGL_RENDER_BUFFER:
                renderBuffer = (EGLenum) value;
                break;

            case EGL_VG_COLORSPACE:
                if (value == EGL_VG_COLORSPACE_LINEAR)
                {
                    type |= EGL_VG_COLORSPACE_LINEAR_BIT;
                }
                break;

            case EGL_VG_ALPHA_FORMAT:
                if (value == EGL_VG_ALPHA_FORMAT_PRE)
                {
                    type |= EGL_VG_ALPHA_FORMAT_PRE_BIT;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (value == EGL_TRUE )
                {
                    protectedContent = EGL_TRUE;
                }
                break;

#if defined(__QNXNTO__)
            case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
                if (gcmIS_SUCCESS(gcoOS_GetEnv(NULL, "IGNORE_EGL_POST_SUB_BUFFER_ATTRIB", &ignoreSubBuf)))
                {
                  if (ignoreSubBuf != gcvNULL)
                  {
                    break;
                  }
                }
                /* fall through to report error if EGL_POST_SUB_BUFFER_SUPPORTED_NV is not ignored */
#endif

            default:
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

#if gcdGC355_MEM_PRINT
    gcoOS_RecordAllocation();
#endif

    /* Create and initialize the surface. */
    surface = _InitializeSurface(thread, eglConfig, type);

    if (surface == EGL_NO_SURFACE)
    {
        /* Bad allocation. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Set Window attributes. */
    surface->hwnd   = native_window;
    surface->buffer = renderBuffer;
    surface->protectedContent = protectedContent;

    /* For easily debug */
    if (dpy->platform->setSwapInterval)
    {
        gctSTRING interval = gcvNULL;

        gcoOS_GetEnv(gcvNULL, "VIV_EGL_SWAP_INTERVAL", &interval);
        if (interval)
        {
            dpy->platform->setSwapInterval(surface, (EGLint)atoi(interval));
        }
    }

    /* Connect to window. */
    result = dpy->platform->connectWindow(dpy, surface, surface->hwnd);

    if (!result)
    {
        /* Rool back. */
        _DestroySurface(thread, dpy, surface);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create the surface. */
    error = _CreateSurface(thread, dpy, surface);

    if (error != EGL_SUCCESS)
    {
        /* Roll back. */
        _DestroySurface(thread, dpy, surface);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        veglSetEGLerror(thread, error);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Push surface into stack. */
    veglPushResObj(Dpy, (VEGLResObj *)&dpy->surfaceStack, (VEGLResObj) surface);

    /* Reference surface. Temp fix for lock surface. */
    veglReferenceSurface(thread, dpy, surface);

#if gcdGC355_MEM_PRINT
    thread->fbMemSize += gcoOS_EndRecordAllocation();
#endif

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    return surface;

OnError:
    return EGL_NO_SURFACE;
}

/* EGL 1.5 */
EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformWindowSurface(
    EGLDisplay dpy,
    EGLConfig config,
    void *native_window,
    const EGLAttrib *attrib_list
    )
{
    VEGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x native_window=%d attrib_list=0x%x",
                    dpy, config, native_window, attrib_list);

    VEGL_TRACE_API_PRE(CreatePlatformWindowSurface)(dpy, config, native_window, attrib_list);

    /* Call internal implmentation. */
    surface = (VEGLSurface) veglCreatePlatformWindowSurface(
                dpy, config, native_window, attrib_list, EGL_FALSE);

    VEGL_TRACE_API_POST(CreatePlatformWindowSurface)(dpy, config, native_window, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePlatformWindowSurface 0x%08X 0x%08X 0x%08X (0x%08X) := "
                "0x%08X (%dx%d)",
                dpy, config, native_window, attrib_list, surface,
                surface->config.width, surface->config.height);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

/* EGL_EXT_platform_base. */
EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformWindowSurfaceEXT(
    EGLDisplay dpy,
    EGLConfig config,
    void *native_window,
    const EGLint *attrib_list
    )
{
    VEGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x native_window=%d attrib_list=0x%x",
                    dpy, config, native_window, attrib_list);

    VEGL_TRACE_API_PRE(CreatePlatformWindowSurfaceEXT)(dpy, config, native_window, attrib_list);

    /* Call internal implmentation. */
    surface = (VEGLSurface) veglCreatePlatformWindowSurface(
                dpy, config, native_window, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreatePlatformWindowSurfaceEXT)(dpy, config, native_window, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePlatformWindowSurfaceEXT 0x%08X 0x%08X 0x%08X (0x%08X) := "
                "0x%08X (%dx%d)",
                dpy, config, native_window, attrib_list, surface,
                surface->config.width, surface->config.height);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreateWindowSurface(
    EGLDisplay dpy,
    EGLConfig config,
    NativeWindowType window,
    const EGLint* attrib_list
    )
{
    VEGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x window=%d attrib_list=0x%x",
                    dpy, config, window, attrib_list);

    VEGL_TRACE_API_PRE(CreateWindowSurface)(dpy, config, window, attrib_list);

    /* Alias to eglCreatePlatformWindowSurface. */
    surface = (VEGLSurface) veglCreatePlatformWindowSurface(
                dpy, config, (void *) window, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreateWindowSurface)(dpy, config, window, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreateWindowSurface 0x%08X 0x%08X 0x%08X (0x%08X) := "
                "0x%08X (%dx%d)",
                dpy, config, window, attrib_list, surface,
                surface->config.width, surface->config.height);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

EGLBoolean
veglDestroySurface(
    EGLDisplay Dpy,
    EGLSurface Surface
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Surface=0x%x", Dpy, Surface);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(
        dpy,
        (VEGLResObj*)&dpy->surfaceStack,
        (EGLResObj)Surface,
        EGL_SURFACE_SIGNATURE
        );

    if (surface == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if surface is locked. */
    if (surface->locked)
    {
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* We should deference the surface, not destroy, since others are maybe using it*/
    veglDereferenceSurface(thread, Dpy, surface, EGL_FALSE);

    /* Pop EGLSurface from the stack to mark it invalid. */
    veglPopResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (VEGLResObj)surface);

    /* If the surface has been destoried then free the struct .*/
    if (surface->reference == gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;
OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroySurface(
    EGLDisplay Dpy,
    EGLSurface Surface
    )
{
    EGLBoolean ret;

    VEGL_TRACE_API(DestroySurface)(Dpy, Surface);
    gcmDUMP_API("${EGL eglDestroySurface 0x%08X 0x%08X}", Dpy, Surface);

    ret = veglDestroySurface(Dpy, Surface);

    return ret;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePbufferSurface(
    IN EGLDisplay Dpy,
    IN EGLConfig Config,
    IN const EGLint * attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLConfig eglConfig;
    VEGLSurface surface;
    EGLint error;
    EGLint i;

    /* Default attributes. */
    EGLint width   = 0;
    EGLint height  = 0;
    EGLint largest = EGL_FALSE;
    EGLint format  = EGL_NO_TEXTURE;
    EGLint target  = EGL_NO_TEXTURE;
    EGLint mipmap  = EGL_FALSE;
    EGLint type = EGL_PBUFFER_BIT;
    EGLBoolean protectedContent = EGL_FALSE;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Config=0x%x attrib_list=0x%x",
                  Dpy, Config, attrib_list);

    VEGL_TRACE_API_PRE(CreatePbufferSurface)(Dpy, Config, attrib_list);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid config. */
    if (((EGLint)(intptr_t)Config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)Config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create shortcuts to objects. */
    eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)Config - 1]);

    /* Determine if the configuration is valid. */
    if (!(eglConfig->surfaceType & EGL_PBUFFER_BIT))
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Parse attributes. */
    if (attrib_list != gcvNULL)
    {
        for (i = 0; attrib_list[i] != EGL_NONE; i += 2)
        {
            switch (attrib_list[i])
            {
            case EGL_WIDTH:
                width = attrib_list[i + 1];
                if (width < 0)
                {
                    veglSetEGLerror(thread, EGL_BAD_PARAMETER);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_HEIGHT:
                height = attrib_list[i + 1];
                if (height < 0)
                {
                    veglSetEGLerror(thread, EGL_BAD_PARAMETER);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_LARGEST_PBUFFER:
                largest = attrib_list[i + 1];
                break;

            case EGL_TEXTURE_FORMAT:
                format = attrib_list[i + 1];
                break;

            case EGL_TEXTURE_TARGET:
                target = attrib_list[i + 1];
                break;

            case EGL_MIPMAP_TEXTURE:
                mipmap = attrib_list[i + 1];
                break;

            case EGL_VG_COLORSPACE:
                if (attrib_list[i + 1] == EGL_VG_COLORSPACE_LINEAR)
                {
                    if (!(eglConfig->surfaceType & EGL_VG_COLORSPACE_LINEAR_BIT))
                    {
                        veglSetEGLerror(thread, EGL_BAD_MATCH);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }
                    type |= EGL_VG_COLORSPACE_LINEAR_BIT;
                }
                break;

            case EGL_VG_ALPHA_FORMAT:
                if (attrib_list[i + 1] == EGL_VG_ALPHA_FORMAT_PRE)
                {
                    if (!(eglConfig->surfaceType & EGL_VG_ALPHA_FORMAT_PRE_BIT))
                    {
                        veglSetEGLerror(thread, EGL_BAD_MATCH);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }
                    type |= EGL_VG_ALPHA_FORMAT_PRE_BIT;
                }
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                if (attrib_list[i + 1] == EGL_TRUE)
                {
                    protectedContent = EGL_TRUE;
                }
                break;

            default:
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);

            }
        }

        if (target != EGL_TEXTURE_2D && target != EGL_NO_TEXTURE)
        {
            veglSetEGLerror(thread, EGL_BAD_PARAMETER);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if ((format == EGL_NO_TEXTURE && target != EGL_NO_TEXTURE) ||
            (format != EGL_NO_TEXTURE && target == EGL_NO_TEXTURE))
        {
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Get width from configuration is not specified. */
    if (width == 0)
    {
        width = eglConfig->width;
    }

    /* Get height from configuration is not specified. */
    if (height == 0)
    {
        height = eglConfig->height;
    }

    if ((width  < 1)
    ||  (width  > thread->maxWidth)
    ||  (height < 1)
    || (height > thread->maxHeight)
    )
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create surface object. */
    surface = _InitializeSurface(thread, eglConfig, type);

    if (surface == EGL_NO_SURFACE)
    {
        /* Bad allocation. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Set PBuffer attributes. */
    surface->config.width   = width;
    surface->config.height  = height;
    surface->largestPBuffer = largest;
    surface->mipmapTexture  = mipmap;
    surface->textureFormat  = format;
    surface->textureTarget  = target;
    surface->buffer         = EGL_BACK_BUFFER;
    surface->protectedContent = protectedContent;

    /* Create the surface. */
    error = _CreateSurface(thread, dpy, surface);

    if (error != EGL_SUCCESS)
    {
        /* Roll back. */
        _DestroySurface(thread, dpy, surface);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        /* Return error. */
        veglSetEGLerror(thread, error);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Push surface into stack. */
    veglPushResObj(Dpy, (VEGLResObj *)&dpy->surfaceStack, (VEGLResObj) surface);

    veglReferenceSurface(thread, dpy, surface);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(CreatePbufferSurface)(Dpy, Config, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePbufferSurface 0x%08X 0x%08X (0x%08X) := "
                "0x%08X (%dx%d)",
                Dpy, Config, attrib_list, surface, surface->config.width,
                surface->config.height);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;

OnError:
    gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
    return EGL_NO_SURFACE;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePbufferFromClientBuffer(
    EGLDisplay Dpy,
    EGLenum buftype,
    EGLClientBuffer buffer,
    EGLConfig Config,
    const EGLint *attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLConfig eglConfig;
    VEGLSurface surface = gcvNULL;
    gceSTATUS status;

    /* Default attributes. */
    EGLint largest = EGL_FALSE;
    EGLint format  = EGL_NO_TEXTURE;
    EGLint target  = EGL_NO_TEXTURE;
    EGLint mipmap  = EGL_FALSE;

    gcmHEADER_ARG("Dpy=0x%x buftype=%d buffer=0x%x Config=0x%x attrib_list=0x%x",
                    Dpy, buftype, buffer, Config, attrib_list);

    VEGL_TRACE_API_PRE(CreatePbufferFromClientBuffer)(Dpy, buftype, buffer, Config, attrib_list);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid config. */
    if (((EGLint)(intptr_t)Config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)Config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create shortcuts to objects. */
    eglConfig  = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)Config - 1]);

    if (buftype != EGL_OPENVG_IMAGE)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Determine if the configuration is valid. */
    if (!(eglConfig->surfaceType & EGL_PBUFFER_BIT))
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (thread->context == EGL_NO_CONTEXT)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Parse attributes. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; attrib_list[i] != EGL_NONE; i += 2)
        {
            switch (attrib_list[i])
            {
            case EGL_TEXTURE_FORMAT:
                format = attrib_list[i + 1];
                break;

            case EGL_TEXTURE_TARGET:
                target = attrib_list[i + 1];
                break;

            case EGL_MIPMAP_TEXTURE:
                mipmap = attrib_list[i + 1];
                break;

            default:
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_SURFACE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attrib_list[i], attrib_list[i + 1]
                    );

                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        if (target != EGL_TEXTURE_2D && target != EGL_NO_TEXTURE)
        {
            veglSetEGLerror(thread, EGL_BAD_PARAMETER);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Create surface object. */
    surface = _InitializeSurface(thread, eglConfig, EGL_PBUFFER_BIT);

    if (surface == EGL_NO_SURFACE)
    {
        /* Bad allocation. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    surface->renderTarget = _GetClientBuffer(
        thread, thread->context->context, buffer);

    if (!surface->renderTarget)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        /* Bad access. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Set PBuffer attributes. */
    status = gcoSURF_GetSize(surface->renderTarget,
                             (gctUINT*)&surface->config.width,
                             (gctUINT*)&surface->config.height,
                             gcvNULL);
    if (gcmIS_ERROR(status))
    {
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(status);
    }

    status = gcoSURF_GetSamples(surface->renderTarget,
                                 (gctUINT*)&surface->config.samples);
    if (gcmIS_ERROR(status))
    {
       veglSetEGLerror(thread, EGL_BAD_ALLOC);
       gcmONERROR(status);
    }

    if (surface->config.samples > 0)
    {
        surface->config.sampleBuffers = 1;
    }
    else
    {
        surface->config.sampleBuffers = 0;
    }

    surface->largestPBuffer = largest;
    surface->mipmapTexture  = mipmap;
    surface->textureFormat  = format;
    surface->textureTarget  = target;
    surface->buffer         = EGL_BACK_BUFFER;

#if gcdENABLE_VG
        /* Determine whether OpenVG pipe is present and OpenVG API is active. */
    surface->openVG = thread->openVGpipe && (thread->api == EGL_OPENVG_API);
    if (surface->depthFormat != gcvSURF_UNKNOWN && !surface->openVG)
#else
    /* Create depth buffer. */
    if (surface->depthFormat != gcvSURF_UNKNOWN)
#endif
    {
        gcmASSERT(surface->depthBuffer == gcvNULL);
        status = gcoSURF_Construct(gcvNULL,
                                     surface->config.width, surface->config.height, 1,
                                     gcvSURF_DEPTH,
                                     surface->depthFormat,
                                     gcvPOOL_DEFAULT,
                                     &surface->depthBuffer);
        if (gcmIS_ERROR(status))
        {
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            gcmONERROR(status);
        }

#if gcdENABLE_3D
        /* Set multi-sampling size. */
        status = gcoSURF_SetSamples(surface->depthBuffer,
                                        surface->config.samples);
        if (gcmIS_ERROR(status))
        {
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            gcmONERROR(status);
        }
#endif
    }

    _InitDrawable(surface);

    /* Push surface into stack. */
    veglPushResObj(Dpy, (VEGLResObj *)&dpy->surfaceStack, (VEGLResObj) surface);

    veglReferenceSurface(thread, dpy, surface);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(CreatePbufferFromClientBuffer)(Dpy, buftype, buffer, Config, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePbufferFromClientBuffer 0x%08X 0x%08X 0x%08X "
                "0x%08X (0x%08X) := 0x%08X (%dx%d)",
                Dpy, buftype, buffer, Config, attrib_list, surface,
                surface->config.width, surface->config.height);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;

OnError:
    if (surface && (surface->depthBuffer != gcvNULL))
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface->depthBuffer));
    }

    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
    }

    gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
    return EGL_NO_SURFACE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglSurfaceAttrib(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLint attribute,
    EGLint value
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;
    VEGLContext current;
    VEGLPlatform platform;

    gcmHEADER_ARG("Dpy=0x%x Surface=0x%x attribute=%d value=%d",
                    Dpy, Surface, attribute, value);
    gcmDUMP_API("${EGL eglSurfaceAttrib 0x%08X 0x%08X 0x%08X 0x%08X}",
                Dpy, Surface, attribute, value);
    VEGL_TRACE_API(SurfaceAttrib)(Dpy, Surface, attribute, value);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (EGLResObj)Surface, EGL_SURFACE_SIGNATURE);
    if (surface == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get shortcut. */
    platform = dpy->platform;

    switch (attribute)
    {
    case EGL_MIPMAP_LEVEL:
        /*
         * According to EGL Spec 1.4:
         * if surface is not pbuffer, EGL_MIPMAP_LEVEL may be set, but has no
         * effect.
         */
        if (surface->type & EGL_PBUFFER_BIT)
        {
            surface->mipmapLevel = value;
        }
        break;

    case EGL_SWAP_BEHAVIOR:
        if (surface->swapBehavior == value)
        {
            /* Nothing to do. */
            break;
        }

        surface->swapBehavior = value;

        if (!(surface->type & EGL_WINDOW_BIT))
        {
            /* Not window surface. */
            break;
        }

#if gcdENABLE_BLIT_BUFFER_PRESERVE
        if (surface->config.samples <= 1)
        {
            /* Swap behavior does not impact renderMode for MSAA. */
            break;
        }
#   endif

        if ((thread->context == gcvNULL) ||
            (thread->context->draw != surface))
        {
            /* Surface is not current. */
            surface->renderMode = -1;
            break;
        }

        /* Cancel window back buffer. */
        if (surface->backBuffer.surface != gcvNULL)
        {
            platform->cancelWindowBackBuffer(dpy, surface, &surface->backBuffer);

            surface->backBuffer.context = gcvNULL;
            surface->backBuffer.surface = gcvNULL;
        }

        /* Dereference render target. */
        if (surface->renderTarget != gcvNULL)
        {
            gcoSURF_Destroy(surface->renderTarget);
            surface->renderTarget = gcvNULL;
        }

        if (surface->prevRenderTarget != gcvNULL)
        {
            gcoSURF_Destroy(surface->prevRenderTarget);
            surface->prevRenderTarget = gcvNULL;
        }

        gcmASSERT(surface->bound);
        platform->unbindWindow(dpy, surface);
        surface->bound = EGL_FALSE;

        /* Re-bind window for rendering. */
        if (!platform->bindWindow(dpy, surface, &surface->renderMode))
        {
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        surface->bound = EGL_TRUE;

        /* Must use new swap model for direct rendering. */
        surface->newSwapModel = (surface->renderMode > 0);

#if defined(ANDROID)
        /* Always use new swap model for android. */
        surface->newSwapModel = gcvTRUE;
#endif

        if (surface->newSwapModel)
        {
            platform->getWindowBackBuffer(dpy, surface, &surface->backBuffer);
        }

        if (surface->renderMode > 0)
        {
            /* Get render target from window back buffer. */
            surface->renderTarget = surface->backBuffer.surface;

            /* Reference external surface. */
            gcoSURF_ReferenceSurface(surface->renderTarget);

            /* Set preserve flag. */
            gcmVERIFY_OK(gcoSURF_SetFlags(
                surface->renderTarget,
                gcvSURF_FLAG_CONTENT_PRESERVED,
                (value == EGL_BUFFER_PRESERVED)
                ));

            /* Reset content updated flag. */
            gcmVERIFY_OK(gcoSURF_SetFlags(
                surface->renderTarget,
                gcvSURF_FLAG_CONTENT_UPDATED,
                gcvFALSE
                ));

            /* Sync drawable with renderTarget. */
            surface->drawable.rtHandles[0]     = surface->renderTarget;
            surface->drawable.prevRtHandles[0] = gcvNULL;
        }
        else if (surface->renderTarget == gcvNULL)
        {
            EGLBoolean success;

            /* Create render target. */
            success = veglCreateRenderTarget(thread, surface);

            if (!success)
            {
                veglSetEGLerror(thread, EGL_BAD_ALLOC);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        current = thread->context;
        if (current && (current->draw == Surface || current->read == Surface))
        {
            VEGLDrawable drawable = (current->draw == Surface)
                                 ? &surface->drawable
                                 : &current->draw->drawable;

            VEGLDrawable readable = (current->read == Surface)
                                 ? &surface->drawable
                                 : &current->read->drawable;

            if (!_SetDrawable(thread, current, drawable, readable))
            {
                /* Error feeing surface objects. */
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
        break;

    case EGL_MULTISAMPLE_RESOLVE:
        if (value == EGL_MULTISAMPLE_RESOLVE_DEFAULT)
        {
            surface->multisampleResolve = value;
        }
        else if (value == EGL_MULTISAMPLE_RESOLVE_BOX)
        {
            if (surface->config.surfaceType & EGL_MULTISAMPLE_RESOLVE_BOX_BIT)
            {
                surface->multisampleResolve = value;
            }
            else
            {
                veglSetEGLerror(thread, EGL_BAD_MATCH);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
        else
        {
            /* Invalid attribute. */
            veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    default:
        /* Invalid attribute. */
        veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglQuerySurface(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLint attribute,
    EGLint *value
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Surface=0x%x attribute=%d value=0x%x",
                    Dpy, Surface, attribute, value);
    VEGL_TRACE_API_PRE(QuerySurface)(Dpy, Surface, attribute, value);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(
        dpy,
        (VEGLResObj*)&dpy->surfaceStack,
        (EGLResObj)Surface,
        EGL_SURFACE_SIGNATURE
        );

    if (surface == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (attribute)
    {
    case EGL_CONFIG_ID:
        *value = surface->config.configId;
        break;

    case EGL_WIDTH:
        *value = surface->config.width;
        break;

    case EGL_HEIGHT:
        *value = surface->config.height;
        break;

    /*
     * According to EGL Spec 1.4:
     * Query of EGL_LARGEST_PBUFFER, EGL_TEXTURE_FORMAT, EGL_TEXTURE_TARGET,
     * EGL_MIPMAP_TEXTURE, EGL_MIPMAP_LEVEL for a non-pbuffer surface is not an
     * error, but 'value' is not modified.
     */
    case EGL_MIPMAP_TEXTURE:
        if (surface->type & EGL_PBUFFER_BIT)
        {
            *value = surface->mipmapTexture;
        }
        break;

    case EGL_MIPMAP_LEVEL:
        if (surface->type & EGL_PBUFFER_BIT)
        {
            *value = surface->mipmapLevel;
        }
        break;

    case EGL_TEXTURE_FORMAT:
        if (surface->type & EGL_PBUFFER_BIT)
        {
            *value = surface->textureFormat;
        }
        break;

    case EGL_TEXTURE_TARGET:
        if (surface->type & EGL_PBUFFER_BIT)
        {
            *value = surface->textureTarget;
        }
        break;

    case EGL_LARGEST_PBUFFER:
        if (surface->type & EGL_PBUFFER_BIT)
        {
            *value = surface->largestPBuffer;
        }
        break;

    case EGL_HORIZONTAL_RESOLUTION:
    case EGL_VERTICAL_RESOLUTION:
    case EGL_PIXEL_ASPECT_RATIO:
        *value = EGL_UNKNOWN;
        break;

    case EGL_RENDER_BUFFER:
        *value = surface->buffer;
        break;

    case EGL_SWAP_BEHAVIOR:
        *value = surface->swapBehavior;
        break;

    case EGL_MULTISAMPLE_RESOLVE:
        *value = surface->multisampleResolve;
        break;

    case EGL_VG_ALPHA_FORMAT:
        *value = surface->vgAlphaFormat;
        break;
    case EGL_VG_COLORSPACE:
        *value = surface->vgColorSpace;
        break;

    case EGL_BITMAP_POINTER_KHR:
        *value = 0;
        if (surface->locked)
        {
            if (!surface->lockBuffer)
            {
                if (!_MapLockedBuffer(thread, surface))
                {
                    break;
                }
            }

            *value = gcmPTR2INT32(surface->lockBits);
        }
        else
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        }
        break;

    case EGL_BITMAP_PITCH_KHR:
        if (surface->locked)
        {
            if (!surface->lockBuffer)
            {
                if (!_MapLockedBuffer(thread, surface))
                {
                    break;
                }
            }

            *value = surface->lockBufferStride;
        }
        else
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        }

        break;

    case EGL_BITMAP_ORIGIN_KHR:
        *value = EGL_UPPER_LEFT_KHR;
        break;

    case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
    case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
    case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
    case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
    case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
        *value = _GetLockedBufferPixelChannelOffset(Surface, attribute);
        break;

#if defined(__linux__) || defined(ANDROID)
    case EGL_BUFFER_AGE_EXT:
        if (thread->context && thread->context->draw == surface)
        {
            if (surface->buffer == EGL_SINGLE_BUFFER)
            {
                *value = 0;
                surface->queriedAge = EGL_TRUE;
                break;
            }
            else if (surface->type & EGL_PBUFFER_BIT)
            {
                /* Pbuffer is not double buffered. */
                *value = 1;
                surface->queriedAge = EGL_TRUE;
                break;
            }

            if ((surface->swapBehavior == EGL_BUFFER_PRESERVED) ||
                /*
                 * Currently we will resolve the whole internal render target
                 * to window back buffer, hence age is 1, if not zero.
                 */
                (surface->renderMode == VEGL_INDIRECT_RENDERING))
            {
                *value = surface->initialFrame ? 0 : 1;
                surface->queriedAge = EGL_TRUE;
                break;
            }

            if (dpy->platform->queryBufferAge &&
                dpy->platform->queryBufferAge(dpy, surface,
                                              &surface->backBuffer, value))
            {
                surface->queriedAge = EGL_TRUE;
                break;
            }
        }

        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
#endif

    case EGL_PROTECTED_CONTENT_EXT:
        *value = surface->protectedContent;
        break;

    case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
        *value = 0;
        break;

    default:
        /* Bad attribute. */
        veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    veglSetEGLerror(thread, EGL_SUCCESS);
    VEGL_TRACE_API_POST(QuerySurface)(Dpy, Surface, attribute, value);
    gcmDUMP_API("${EGL eglQuerySurface 0x%08X 0x%08X 0x%08X := 0x%08X}",
                Dpy, Surface, attribute, *value);

    gcmFOOTER_ARG("*value=%d return=%d", *value, EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

static EGLSurface
veglCreatePlatformPixmapSurface(
    EGLDisplay Dpy,
    EGLConfig Config,
    void *native_pixmap,
    const void * attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLConfig eglConfig;
    VEGLSurface surface;
    VEGLPlatform platform;
    EGLint type = EGL_PIXMAP_BIT;
    EGLint error;
    EGLBoolean result;
    gceSTATUS status;

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_NO_SURFACE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        return EGL_NO_SURFACE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Get shortcut. */
    platform = dpy->platform;

    /* Test for valid config. */
    if (((EGLint)(intptr_t)Config <= __EGL_INVALID_CONFIG__)
    ||  ((EGLint)(intptr_t)Config > dpy->configCount)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid native pixmap handle. */
    if (native_pixmap == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_NATIVE_PIXMAP);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create shortcuts to objects. */
    eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)Config - 1]);

    /* Determine if the configuration is valid. */
    if (!(eglConfig->surfaceType & EGL_PIXMAP_BIT))
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Parse attributes. */
    if (attrib_list != gcvNULL)
    {
        EGLint i;

        for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
        {
            EGLAttrib attribute = _AttribValue(attrib_list, intAttrib, i);
            EGLAttrib value     = _AttribValue(attrib_list, intAttrib, i + 1);

            switch (attribute)
            {
            case EGL_VG_COLORSPACE:
                if (value == EGL_VG_COLORSPACE_LINEAR)
                {
                    type |= EGL_VG_COLORSPACE_LINEAR_BIT;
                }
                break;

            case EGL_VG_ALPHA_FORMAT:
                if (value == EGL_VG_ALPHA_FORMAT_PRE)
                {
                    type |= EGL_VG_ALPHA_FORMAT_PRE_BIT;
                }
                break;
            default:
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

    /* Create surface object. */
    surface = _InitializeSurface(thread, eglConfig, type);

    if (surface == EGL_NO_SURFACE)
    {
        /* Bad allocation. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Set Pixmap attributes. */
    surface->pixmap = native_pixmap;
    surface->buffer = EGL_SINGLE_BUFFER;

    /* Connect to pixmap. */
    result = platform->connectPixmap(dpy,
                                     native_pixmap,
                                     &surface->pixInfo,
                                     &surface->pixmapSurface);

    if (!result)
    {
        _DestroySurface(thread, dpy, surface);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        /* Return error. */
        veglSetEGLerror(thread, EGL_BAD_NATIVE_PIXMAP);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Sync pixels. */
    platform->syncFromPixmap(native_pixmap, surface->pixInfo);

    /* Reference the pixmap surface. */
    gcoSURF_ReferenceSurface(surface->pixmapSurface);

    /* Create the surface. */
    error = _CreateSurface(thread, dpy, surface);

    if (error != EGL_SUCCESS)
    {
        /* Roll back. */
        _DestroySurface(thread, dpy, surface);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));

        /* Return error. */
        veglSetEGLerror(thread, error);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Push surface into stack. */
    veglPushResObj(Dpy, (VEGLResObj *)&dpy->surfaceStack, (VEGLResObj) surface);

    veglReferenceSurface(thread, dpy, surface);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    return surface;

OnError:
    return EGL_NO_SURFACE;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformPixmapSurface(
    EGLDisplay dpy,
    EGLConfig config,
    void *native_pixmap,
    const EGLAttrib *attrib_list
    )
{
    EGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x native_pixmap=0x%x attrib_list=0x%x",
                    dpy, config, native_pixmap, attrib_list);

    VEGL_TRACE_API_PRE(CreatePlatformPixmapSurface)(dpy, config, native_pixmap, attrib_list);

    /* Call internal implementation. */
    surface = veglCreatePlatformPixmapSurface(
                dpy, config, native_pixmap, attrib_list, EGL_FALSE);

    VEGL_TRACE_API_POST(CreatePlatformPixmapSurface)(dpy, config, native_pixmap, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePlatformPixmapSurface 0x%08X 0x%08X 0x%08X (0x%08X) := 0x%08X",
                dpy, config, native_pixmap, attrib_list, surface);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePlatformPixmapSurfaceEXT(
    EGLDisplay dpy,
    EGLConfig config,
    void *native_pixmap,
    const EGLint *attrib_list
    )
{
    EGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x native_pixmap=0x%x attrib_list=0x%x",
                    dpy, config, native_pixmap, attrib_list);

    VEGL_TRACE_API_PRE(CreatePlatformPixmapSurfaceEXT)(dpy, config, native_pixmap, attrib_list);

    /* Call internal implementation. */
    surface = veglCreatePlatformPixmapSurface(
                dpy, config, native_pixmap, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreatePlatformPixmapSurfaceEXT)(dpy, config, native_pixmap, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePlatformPixmapSurfaceEXT 0x%08X 0x%08X 0x%08X (0x%08X) := 0x%08X",
                dpy, config, native_pixmap, attrib_list, surface);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

EGLAPI EGLSurface EGLAPIENTRY
eglCreatePixmapSurface(
    EGLDisplay dpy,
    EGLConfig config,
    NativePixmapType pixmap,
    const EGLint * attrib_list
    )
{
    EGLSurface surface;

    gcmHEADER_ARG("dpy=0x%x config=0x%x pixmap=0x%x attrib_list=0x%x",
                    dpy, config, pixmap, attrib_list);

    VEGL_TRACE_API_PRE(CreatePixmapSurface)(dpy, config, pixmap, attrib_list);

    /* Alias to eglCreatePlatformPixmapSurface. */
    surface = veglCreatePlatformPixmapSurface(
                dpy, config, (void *) pixmap, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreatePixmapSurface)(dpy, config, pixmap, attrib_list, surface);

    gcmDUMP_API("${EGL eglCreatePixmapSurface 0x%08X 0x%08X 0x%08X (0x%08X) := 0x%08X",
                dpy, config, pixmap, attrib_list, surface);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", surface);
    return surface;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglBindTexImage(
    EGLDisplay Display,
    EGLSurface Surface,
    EGLint Buffer
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;

    gcmHEADER_ARG("Display=0x%x Surface=0x%x Buffer=%d",
                  Display, Surface, Buffer);
    gcmDUMP_API("${EGL eglBindTexImage 0x%08X 0x%08X 0x%08X}",
                Display, Surface, Buffer);
    VEGL_TRACE_API(BindTexImage)(Display, Surface, Buffer);

    /* Get current thread. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Display);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid surface. */
    surface = (VEGLSurface)veglGetResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (EGLResObj)Surface, EGL_SURFACE_SIGNATURE);
    if ((surface == gcvNULL)
    ||  (surface->buffer != EGL_BACK_BUFFER)
    )
    {
        /* Bad surface. */
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if surface locked. */
    if (surface->locked)
    {
        /* Bad access. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test surface arguments. */
    if (surface->textureFormat == EGL_NO_TEXTURE)
    {
        /* Bad match. */
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid buffer. */
    if (Buffer != EGL_BACK_BUFFER)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if already bound. */
    if (surface->texBinder != gcvNULL)
    {
        /* Bad access. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if Display and Surface are attached to the current context. */
    if ((thread->context->display == Display)
    &&  (thread->context->draw == Surface)
    )
    {
        /* Call glFlush. */
        _Flush(thread);
    }
    else
    {
    }

    /* Call the GL to bind the texture. */
    thread->error = _BindTexImage(thread,
                                  surface->renderTarget,
                                  surface->textureFormat,
                                  surface->mipmapTexture,
                                  surface->mipmapLevel,
                                  surface->config.width,
                                  surface->config.height,
                                  &surface->texBinder);

    /* Return success or failure. */
    gcmFOOTER_ARG("return=%d", (thread->error == EGL_SUCCESS) ? EGL_TRUE : EGL_FALSE);
    return (thread->error == EGL_SUCCESS) ? EGL_TRUE : EGL_FALSE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglReleaseTexImage(
    EGLDisplay Display,
    EGLSurface Surface,
    EGLint Buffer
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;

    gcmHEADER_ARG("Display=0x%x Surface=0x%x Buffer=%d",
                  Display, Surface, Buffer);
    gcmDUMP_API("${EGL eglReleaseTexImage 0x%08X 0x%08X 0x%08X}",
                Display, Surface, Buffer);
    VEGL_TRACE_API(ReleaseTexImage)(Display, Surface, Buffer);

    /* Get current thread. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Display);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (EGLResObj)Surface, EGL_SURFACE_SIGNATURE);
    if ((surface == gcvNULL)
    ||  (surface->buffer != EGL_BACK_BUFFER)
    )
    {
        /* Bad surface. */
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if surface is locked. */
    if (surface->locked)
    {
        /* Bad access. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test surface arguments. */
    if (surface->textureFormat == EGL_NO_TEXTURE)
    {
        /* Bad match. */
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid buffer. */
    if (Buffer != EGL_BACK_BUFFER)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if bound. */
    if (surface->texBinder == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_SURFACE);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Call the GL to unbind the texture. */
    thread->error = _BindTexImage(thread,
                                  gcvNULL,
                                  surface->textureFormat,
                                  EGL_FALSE,
                                  0,
                                  0,
                                  0,
                                  &surface->texBinder);

    /* Return success or failure. */
    gcmFOOTER_ARG("return=%d", (thread->error == EGL_SUCCESS) ? EGL_TRUE : EGL_FALSE);
    return (thread->error == EGL_SUCCESS) ? EGL_TRUE : EGL_FALSE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglLockSurfaceKHR(
    EGLDisplay Display,
    EGLSurface Surface,
    const EGLint *Attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    VEGLContext ctx;
    EGLBoolean  preserve = EGL_FALSE;
    gceSTATUS status;

    gcmHEADER_ARG("Display=0x%x Surface=0x%x Attrib_list=0x%x",
                    Display, Surface, Attrib_list);
    gcmDUMP_API("${EGL eglLockSurfaceKHR 0x%08X 0x%08X (0x%08X)",
                Display, Surface, Attrib_list);
    gcmDUMP_API_ARRAY_TOKEN(Attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    /* Get current thread. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Display);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (EGLResObj)Surface, EGL_SURFACE_SIGNATURE);
    if (surface == gcvNULL)
    {
        /* Bad surface. */
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check EGL_LOCK_SURFACE_BIT_KHR bit in config. */
    if (!(surface->config.surfaceType & EGL_LOCK_SURFACE_BIT_KHR) || surface->locked)
    {
        /* Not a lockable surface, or surface has been locked. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    VEGL_LOCK_DISPLAY_RESOURCE(dpy);

    for (ctx = dpy->contextStack; ctx != gcvNULL; ctx = (VEGLContext)ctx->resObj.next)
    {
        if (ctx->draw == surface || ctx->read == surface)
        {
            /* Surface is current of some context. */
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);

    /* Parse the attribute list. */
    if (Attrib_list != gcvNULL)
    {
        while (*Attrib_list != EGL_NONE)
        {
            EGLint attribute = *Attrib_list++;
            EGLint value     = *Attrib_list++;

            switch(attribute)
            {
            case EGL_MAP_PRESERVE_PIXELS_KHR:
                preserve = value;
                break;

            case EGL_LOCK_USAGE_HINT_KHR:
                break;

            default:
                /* Ignore unacceptable attributes. */
                gcmTRACE_ZONE(
                    gcvLEVEL_ERROR, gcdZONE_EGL_SURFACE,
                    "%s(%d): Unknown attribute 0x%04X = 0x%04X.",
                    __FUNCTION__, __LINE__,
                    attribute, value
                    );

                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }


    /* Set surface as locked. */
    surface->locked           = EGL_TRUE;
    surface->lockBufferFormat = surface->renderTargetFormat;
    surface->lockBuffer       = gcvNULL;
    surface->lockBits         = gcvNULL;
    surface->lockPreserve     = preserve;

    veglSetEGLerror(thread,  EGL_SUCCESS);

    /* Return success or failure. */
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglUnlockSurfaceKHR(
    EGLDisplay Display,
    EGLSurface Surface
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS   status;

    gcmHEADER_ARG("Display=0x%x Surface=0x%x", Display, Surface);
    gcmDUMP_API("${EGL eglUnlockSurfaceKHR 0x%08X 0x%08X}", Display, Surface);

    /* Get current thread. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Display);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for valid EGLSurface structure */
    surface = (VEGLSurface)veglGetResObj(dpy, (VEGLResObj*)&dpy->surfaceStack, (EGLResObj)Surface, EGL_SURFACE_SIGNATURE);
    if (surface == gcvNULL)
    {
        /* Bad surface. */
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (!surface->locked)
    {
        /* Surface has been unlocked. */
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* If surface is mapped. */
    if (surface->lockBuffer)
    {
        if (surface->renderTarget != gcvNULL)
        {
#if gcdENABLE_VG && gcdENABLE_3D
            if (surface->openVG)
#endif
            {
#if gcdENABLE_VG
                status = gcoSURF_Copy(surface->renderTarget,
                                      surface->lockBuffer);
#endif
            }
#if gcdENABLE_VG && gcdENABLE_3D
            else
#endif
            {
#if gcdENABLE_3D
                gcsSURF_VIEW lockView = {surface->lockBuffer, 0, 1};
                gcsSURF_VIEW rtView = {surface->renderTarget, 0, 1};

                /* Reflect mapped buffer to color buffer. */
                status = gcoSURF_ResolveRect(&lockView, &rtView, gcvNULL);
#endif
            }

            if (gcmIS_ERROR(status))
            {
                veglSetEGLerror(thread,  EGL_BAD_ACCESS);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
        }
        else
        {
            /* Preserve the lockBuffer, until the renderTarget is determined. */
            surface->lockBufferMirror = surface->lockBuffer;
            gcoSURF_ReferenceSurface(surface->lockBufferMirror);
        }

        /* Unlock bitmap. */
        gcoSURF_Unlock(surface->lockBuffer, surface->lockBits);

        /* Destroy bitmap. */
        gcoSURF_Destroy(surface->lockBuffer);
    }

    surface->locked = EGL_FALSE;

    /* Should clean locked buffer? */
    surface->lockBufferFormat = gcvSURF_UNKNOWN;
    surface->lockBuffer       = gcvNULL;
    surface->lockBits         = gcvNULL;

    veglSetEGLerror(thread,  EGL_SUCCESS);

    /* Return success or failure. */
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;
OnError:

    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}
