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
#    include <ui/ANativeObjectBase.h>
#  else
#    include <ui/android_native_buffer.h>
#  endif
#endif


#include "gc_hal_user.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_SWAP

#if gcdDUMP_FRAME_TGA || defined(EMULATOR) || gcdFPGA_BUILD

#ifdef ANDROID
#  include <cutils/properties.h>
#endif

#ifdef UNDER_CE
#  include <winbase.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONF_MAX_LEN 256
#define STRING_LEN   1024

static void
_SaveFrameTGA(
    VEGLThreadData thread,
    VEGLSurface draw,
    gcsPOINT_PTR RectOrigin,
    gcsPOINT_PTR RectSize
    )
{
    do
    {
        gceSTATUS status;

        gcoSURF    resolve;
        gctPOINTER logical[3];
        gctINT32   resolveStride;

        gctUINT8_PTR frameBGR;
        gctUINT8_PTR frameARGB;
        gctUINT8 tgaHeader[18];
        gctPOINTER frameMemory;

        gctINT x;
        gctINT y;

        gctFILE file = gcvNULL;

        gctSTRING fileName;
        gctSTRING outDir;

        gctUINT a;
        gctUINT o;

        gctUINT  fileNameOffset = 0;
        gctINT32 frameStart    = 0;
        gctINT32 frameEnd      = 0;

        gctCHAR arrayString[STRING_LEN] = "\0";

#ifdef UNDER_CE
        TCHAR moduleName[CONF_MAX_LEN];
        gctCHAR cur[CONF_MAX_LEN];
        gctCHAR conf[CONF_MAX_LEN];
        gctCHAR *p1;
#endif

        static gctUINT32 frameCount = 0;
        static gctINT32  frameArray[STRING_LEN] = {0};

        /* Increase frame number. */
        frameCount++;

        /* Allocate memory for outDir string. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL, CONF_MAX_LEN, (gctPOINTER *) &outDir));

#ifdef UNDER_CE
        /* Query module path. */
        GetModuleFileName(NULL, moduleName, CONF_MAX_LEN);
        wcstombs(cur, moduleName, CONF_MAX_LEN);

        /* Build config file. */
        strcpy(conf, cur);
        p1 = strrchr(conf, '\\');
        strcpy(p1 + 1, "drv_config.ini");

        /* Open config file. */
        gcmERR_BREAK(gcoOS_Open(gcvNULL, conf, gcvFILE_READ, &file));

#elif defined(ANDROID)
        gctSTRING conf;

        /* Allocate config file name string. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL, PROPERTY_VALUE_MAX, (gctPOINTER *) &conf));

        /* Query config file name. */
        if (0 == property_get("drv.config", conf, NULL))
        {
            /* Default config file. */
            gcoOS_Open(gcvNULL, "/sdcard/drv_config.ini", gcvFILE_READ, &file);
        }
        else
        {
            /* Config file from property. */
            gcoOS_Open(gcvNULL, conf, gcvFILE_READ, &file);
        }

        /* Free config file name string. */
        gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, conf));

#else
        {
            gctSTRING conf;

            /* Query config file name. */
            gcoOS_GetEnv(gcvNULL, "DRV_CONFIG", &conf);

            if (conf == gcvNULL)
            {
                /* Config file from environment variable. */
                gcoOS_Open(gcvNULL, "./drv_config.ini", gcvFILE_READ, &file);
            }
            else
            {
                /* Default config file. */
                gcoOS_Open(gcvNULL, conf, gcvFILE_READ, &file);
            }
        }
#endif

        if (file)
        {
            /* Read configurations from config file. */
#ifdef WIN32
            a = fscanf_s(file, "FRAME_ARRAY = %s\n", arrayString);
            o = fscanf_s(file, "OUT_DIR = %s\n", outDir);

            if (fscanf_s(file, "FRAME_START = %d\n", &frameStart)) {};
            if (fscanf_s(file, "FRAME_END = %d\n", &frameEnd)) {};
#else
            a = fscanf(file, "FRAME_ARRAY = %s\n", arrayString);
            o = fscanf(file, "OUT_DIR = %s\n", outDir);

            if (fscanf(file, "FRAME_START = %d\n", &frameStart)) {};
            if (fscanf(file, "FRAME_END = %d\n", &frameEnd)) {};
#endif
            /* Close config file. */
            gcmERR_BREAK(gcoOS_Close(gcvNULL, file));
        }
        else
        {
            /* Default configurations. */
            a = 0;
            o = 0;

            frameStart = 0;
            frameEnd   = 99999;
        }

        if (a != 0)
        {
            /* FrameArray config available. */
            gctUINT k = 0;
            gctUINT i = 0;
            gctSTRING l;
            gctSTRING p;

            p = arrayString;

            do
            {
                /* Build frameArray: ascii to integer. */
                frameArray[i++] = atoi(p);

                l = strstr(p, ",");
                p = l + 1;
            }
            while (l != gcvNULL);

            /* Check frame numbers. */
            for (k = 0; k < i; k++)
            {
                if ((frameArray[k] >= 0) &&
                    (frameCount == (gctUINT32) frameArray[k]))
                {
                    /* Need dump current frame. */
                    frameArray[k] = -1;
                    break;
                }
            }

            if (k >= i)
            {
                /* Do not need to dump current frame, done. */
                break;
            }
        }
        else
        {
            /* Frame array from frameStart till frameEnd. */
            if ((frameCount < (gctUINT32) frameStart) ||
                (frameCount > (gctUINT32) frameEnd))
            {
                /* Do not dump frame out of frameStart-frameEnd area. */
                break;
            }
        }

        if (o == 0)
        {
            /* Build output directory string. */
#ifdef UNDER_CE
            gcmVERIFY_OK(gcoOS_PrintStrSafe(outDir, 10, &fileNameOffset, cur));

#elif defined(ANDROID)
            gcmVERIFY_OK(gcoOS_PrintStrSafe(outDir, 10, &fileNameOffset, "/sdcard/"));
#else
            gcmVERIFY_OK(gcoOS_PrintStrSafe(outDir, 10, &fileNameOffset, "./"));
#endif
        }

        /* Construct temp linear surface. */
        gcmERR_BREAK(gcoSURF_Construct(
            gcvNULL,
            RectSize->x,
            RectSize->y,
            1,
            gcvSURF_BITMAP,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &resolve
            ));

#if gcdENABLE_VG
        if (thread->openVGpipe && thread->api == EGL_OPENVG_API)
        {
             veglDISPATCH * dispatch = _GetDispatch(thread, gcvNULL);

             (*dispatch->resolveVG)(thread->context->context, resolve);
        }
        else
#endif
        {
#if gcdENABLE_3D
            gcsSURF_VIEW rtView = {draw->renderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {resolve, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.srcOrigin  = *RectOrigin;
            rlvArgs.uArgs.v2.dstOrigin  = *RectOrigin;
            rlvArgs.uArgs.v2.rectSize   = *RectSize;
            rlvArgs.uArgs.v2.numSlices  = 1;

            /* Resolve render target to linear surface. */
            gcmERR_BREAK(gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs));
#endif
        }

        /* Commit resolve command and stall hardware. */
        gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));

        /* Lock for linear surface memory. */
        gcmERR_BREAK(gcoSURF_Lock(resolve, gcvNULL, logical));

        /* Query linear surface stride. */
        gcmERR_BREAK(gcoSURF_GetAlignedSize(resolve, gcvNULL, gcvNULL, &resolveStride));

        /* Allocate frame memory. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL, RectSize->x * RectSize->y * 3, &frameMemory));

        /* Color format conversion: ARGB to RGB. */
        frameBGR = (gctUINT8_PTR) frameMemory;

        /* Go through all pixels. */
        for (y = 0; y < RectSize->y; ++y)
        {
            frameARGB = (gctUINT8_PTR) logical[0] + y * resolveStride;

            for (x = 0; x < RectSize->x; ++x)
            {
                frameBGR[0] = frameARGB[0];
                frameBGR[1] = frameARGB[1];
                frameBGR[2] = frameARGB[2];
                frameARGB += 4;
                frameBGR  += 3;
            }
        }

        /* Unlock and destroy linear surface. */
        gcmERR_BREAK(gcoSURF_Unlock(resolve, logical[0]));
        gcmERR_BREAK(gcoSURF_Destroy(resolve));

        /* Prepare tga file header. */
        tgaHeader[ 0] = 0;
        tgaHeader[ 1] = 0;
        tgaHeader[ 2] = 2;
        tgaHeader[ 3] = 0;
        tgaHeader[ 4] = 0;
        tgaHeader[ 5] = 0;
        tgaHeader[ 6] = 0;
        tgaHeader[ 7] = 0;
        tgaHeader[ 8] = 0;
        tgaHeader[ 9] = 0;
        tgaHeader[10] = 0;
        tgaHeader[11] = 0;
        tgaHeader[12] = (RectSize->x & 0x00ff);
        tgaHeader[13] = (RectSize->x & 0xff00) >> 8;
        tgaHeader[14] = (RectSize->y & 0x00ff);
        tgaHeader[15] = (RectSize->y & 0xff00) >> 8;
        tgaHeader[16] = 24;
        tgaHeader[17] = (thread->api == EGL_OPENGL_ES_API) ? (0x01 << 5) : 0;

        /* Allocate memory for output file name string. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL, CONF_MAX_LEN, (gctPOINTER *) &fileName));

        /* Build file name.*/
        fileNameOffset = 0;

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(fileName, CONF_MAX_LEN, &fileNameOffset, outDir));

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(fileName, CONF_MAX_LEN, &fileNameOffset,
                               "frame_%05d.tga", frameCount));

        /* Open tga file for write. */
        gcmERR_BREAK(gcoOS_Open(gcvNULL, fileName, gcvFILE_CREATE, &file));

        /* Write tga file header. */
        gcmERR_BREAK(gcoOS_Write(gcvNULL, file, 18, tgaHeader));

        /* Write pixel data. */
        gcmERR_BREAK(gcoOS_Write(gcvNULL, file, RectSize->x * RectSize->y * 3, frameMemory));

        /* Close tga file. */
        gcmERR_BREAK(gcoOS_Close(gcvNULL, file));

        /* Clean resources. */
        gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, frameMemory));
        gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, fileName));
        gcmERR_BREAK(gcmOS_SAFE_FREE(gcvNULL, outDir));
    }
    while(gcvFALSE);
}

static void
_DumpTGA(
    VEGLThreadData Thread,
    VEGLSurface Draw
    )
{
    gcsPOINT origin = {0, 0};
    gcsPOINT size   = {Draw->config.width, Draw->config.height};

#if gcdDUMP_FRAME_TGA
    _SaveFrameTGA(Thread, Draw, &origin, &size);
#  elif (defined(EMULATOR) || gcdFPGA_BUILD)
    static EGLint checkStatus = -1;

    if (checkStatus < 0)
    {
        gctSTRING dumpFrameTGA = gcvNULL;
        checkStatus = 0;

        if (gcmIS_ERROR(gcoOS_GetEnv(gcvNULL,
                                       "VIV_DUMP_FRAME_TGA",
                                       &dumpFrameTGA)))
        {
            /* No dump. */
            return;
        }

        if (dumpFrameTGA &&
            gcoOS_StrCmp(dumpFrameTGA, "1") == gcvSTATUS_OK)
        {
            checkStatus = 1;
        }
    }

    if (checkStatus)
    {
        _SaveFrameTGA(Thread, Draw, &origin, &size);
    }
#  endif
}

#  define VEGL_DUMP_TGA(thread, draw) \
    _DumpTGA(thread, draw)
#else

/* Nop when gcdDUMP_FRAME_TGA etc disabled. */
#  define VEGL_DUMP_TGA(thread, draw) \
    do {} while (0)
#endif

#if gcdDUMP
static void
_DumpSwapTag(
    gcoSURF Surface,
    EGLint width,
    EGLint height
    )
{
    gctUINT32 address[3] = {0};
    gctPOINTER logical[3] = {gcvNULL};
    gctINT stride;

    gcoSURF_Lock(Surface, address, logical);
    gcoSURF_Unlock(Surface, logical[0]);
    gcoSURF_GetAlignedSize(Surface, gcvNULL, gcvNULL, &stride);

    gcmDUMP(gcvNULL,
            "@[swap 0x%08X %dx%d +%u]",
            address[0],
            width,
            height,
            stride);
}
#  define VEGL_DUMP_SWAP_TAG(draw, sur) \
    _DumpSwapTag(sur, (draw)->config.width, (draw)->config.height)

#else
/* Nop. */
#  define VEGL_DUMP_SWAP_TAG(draw, sur) \
    do {} while (0)
#endif

#if gcdDUMP && gcdENABLE_3D

static void
_GetSurfaceHwFormat(
    gcoSURF Surface,
    gctUINT_PTR HwFormat,
    gctUINT_PTR Swizzle,
    gctUINT_PTR HwTiling
    )
{
    gceSURF_FORMAT format;
    gceTILING tiling;
    gctUINT hwFormat;
    gctUINT swizzle;
    gctUINT hwTiling;

    gcoSURF_GetFormat(Surface, gcvNULL, &format);
    gcoSURF_GetTiling(Surface, &tiling);

    switch (format)
    {
    case gcvSURF_A8B8G8R8:
        hwFormat = 0x06;
        swizzle  = 1;
        break;
    case gcvSURF_X8B8G8R8:
        hwFormat = 0x05;
        swizzle  = 1;
        break;
        case gcvSURF_X8R8G8B8:
        hwFormat = 0x05;
        swizzle  = 0;
        break;
    case gcvSURF_R5G6B5:
        hwFormat = 0x04;
        swizzle  = 0;
        break;
    case gcvSURF_A8R8G8B8:
        hwFormat = 0x06;
        swizzle  = 0;
        break;
    case gcvSURF_A1R5G5B5:
        hwFormat = 0x03;
        swizzle  = 0;
        break;
    case gcvSURF_A4R4G4B4:
        hwFormat = 0x01;
        swizzle  = 0;
        break;
    case gcvSURF_YUY2:
        hwFormat = 0x07;
        swizzle  = 0;
        break;
    default:
        hwFormat = 0xFF;
        swizzle  = 0;
        break;
    }

    switch (tiling)
    {
    case gcvLINEAR:
        hwTiling = 0x0;
        break;
    case gcvTILED:
        hwTiling = 0x1;
        break;
    case gcvSUPERTILED:
        hwTiling = 0x2;
        break;
    case gcvMULTI_TILED:
        hwTiling = 0x3;
        break;
    case gcvMULTI_SUPERTILED:
        hwTiling = 0x4;
        break;
    case gcvYMAJOR_SUPERTILED:
        hwTiling = 0x5;
        break;
    default:
        hwTiling = 0xF;
        break;
    }

    *HwFormat = hwFormat;
    *Swizzle  = swizzle;
    *HwTiling = hwTiling;
}

static void
_DumpFrameTag(
    gcoSURF Surface,
    gctCONST_STRING Tag,
    EGLint Width,
    EGLint Height
    )
{
    gctUINT32 address[3] = {0};
    gctPOINTER logical[3] = {gcvNULL};
    gctUINT bottomBufferOffset = 0;
    gctUINT hwFormat;
    gctUINT swizzle;
    gctUINT hwTiling;

    gcoSURF_Lock(Surface, address, logical);
    gcoSURF_Unlock(Surface, logical[0]);

    gcoSURF_GetBottomBufferOffset(Surface,
                                  &bottomBufferOffset);

    _GetSurfaceHwFormat(Surface,
                        &hwFormat,
                        &swizzle,
                        &hwTiling);

    gcmDUMP(gcvNULL,
            "@[%s 0x%08X:0x%08X %dx%d %d:%d %d]",
            Tag,
            address[0],
            address[0] + bottomBufferOffset,
            Width,
            Height,
            hwFormat,
            swizzle,
            hwTiling);
}

#  define VEGL_DUMP_FRAME_TAG(draw, sur) \
    _DumpFrameTag(sur, "frame", (draw)->config.width, (draw)->config.height)

#  define VEGL_DUMP_FCFILL_TAG(draw, sur) \
    _DumpFrameTag(sur, "fcfill", (draw)->config.width, (draw)->config.height)
#else

/* Nop when gcdDUMP disabled. */
#  define VEGL_DUMP_FRAME_TAG(draw, sur) \
    do {} while (0)

#  define VEGL_DUMP_FCFILL_TAG(draw, sur) \
    do {} while (0)
#endif

/* Realloc region with give capacity. */
static gcmINLINE EGLBoolean
_ReallocRegion(
    IN struct eglRegion * Region,
    IN EGLint             NumRects
    )
{
    gctPOINTER ptr;

    if (Region->maxNumRects >= NumRects)
    {
        Region->numRects = 0;
        return EGL_TRUE;
    }

    if (Region->rects)
    {
        gcmOS_SAFE_FREE(gcvNULL, Region->rects);
        Region->rects = gcvNULL;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, 4 * gcmSIZEOF(EGLint) * NumRects, &ptr)))
    {
        /* Out of memory. */
        return EGL_FALSE;
    }

    Region->numRects    = 0;
    Region->maxNumRects = NumRects;
    Region->rects       = ptr;

    return EGL_TRUE;
}

static gcmINLINE EGLBoolean
_CopyRegion(
    IN struct eglRegion * Dest,
    IN struct eglRegion * Source
    )
{
    if (!_ReallocRegion(Dest, Source->numRects))
    {
        return EGL_FALSE;
    }

    gcoOS_MemCopy(Dest->rects, Source->rects,
                  sizeof (EGLint) * 4 * Source->numRects);

    Dest->numRects = Source->numRects;

    return EGL_TRUE;
}

static gcmINLINE EGLBoolean
_ValidateRegion(
    IN struct eglRegion * Region
    )
{
    if (Region)
    {
        if ((Region->numRects < 0) ||
            (Region->numRects > 0 && !Region->rects))
        {
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}

VEGLWorkerInfo
veglGetWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    VEGLWorkerInfo worker;

    /* Avaiable workers required. */
    gcmVERIFY_OK(gcoOS_WaitSignal(
        gcvNULL, Surface->workerAvaiableSignal, gcvINFINITE));

    /* Get access to avaliable worker list. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, Surface->workerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Surface we have available workers? */
    if (Surface->availableWorkers != gcvNULL)
    {
        /* Yes, pick the first one and use it. */
        worker = Surface->availableWorkers;
        Surface->availableWorkers = worker->next;
    }
    else
    {
        /* Should never run here because 'workerAvaiableSignal' is used. */
        gcmASSERT(0);

        /* No, get the last submitted worker. */
        worker = Surface->lastSubmittedWorker;

        /* Remove the worker from display worker list. */
        worker->prev->next = worker->next;
        worker->next->prev = worker->prev;
    }

    /* Set Worker as busy. */
    worker->draw = Surface;

    /* Dec free worker count. */
    Surface->freeWorkerCount--;

    gcmASSERT(Surface->freeWorkerCount < Surface->totalWorkerCount);

    if (Surface->freeWorkerCount == (Surface->totalWorkerCount - 1))
    {
        /*
         * It's the first time to return out a worker.
         * Not all workers are free.
         */
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL, Surface->workerDoneSignal, gcvFALSE));
    }

    if (Surface->freeWorkerCount == 0)
    {
        /* No more free workers. */
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL, Surface->workerAvaiableSignal, gcvFALSE));

    }

    /* Release access mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, Surface->workerMutex));
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

        if (!_ReallocRegion(&worker->region, 1))
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        if (!_ReallocRegion(&worker->damageHint, 1))
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    }

    /* Return the worker. */
    return worker;

OnError:
    if (acquired)
    {
        /* Roll back. */
        Surface->freeWorkerCount++;

        if (Surface->freeWorkerCount == Surface->totalWorkerCount)
        {
            gcmVERIFY_OK(gcoOS_Signal(
                gcvNULL, Surface->workerDoneSignal, gcvTRUE));
        }

        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL, Surface->workerAvaiableSignal, gcvTRUE));

        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Surface->workerMutex));
    }

    return gcvNULL;
}

VEGLWorkerInfo
veglFreeWorker(
    VEGLWorkerInfo Worker
    )
{
    gceSTATUS status;
    VEGLSurface ownerSurface;
    gctBOOL acquired = gcvFALSE;
    VEGLWorkerInfo nextWorker;

    /* Get a shortcut to the surface. */
    ownerSurface = Worker->draw;

    /* Get the next worker. */
    nextWorker = Worker->next;

    /* Remove worker from display worker list. */
    Worker->prev->next = Worker->next;
    Worker->next->prev = Worker->prev;

    /* Get access to avaliable worker list. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, ownerSurface->workerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Add it back to the thread available worker list. */
    Worker->next = ownerSurface->availableWorkers;
    ownerSurface->availableWorkers = Worker;

    /* Set Worker as available. */
    Worker->draw = gcvNULL;

    /* Inc free worker count. */
    ownerSurface->freeWorkerCount++;

    if (ownerSurface->freeWorkerCount == ownerSurface->totalWorkerCount)
    {
        /* All workers are processed. */
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL, ownerSurface->workerDoneSignal, gcvTRUE));
    }

    if (ownerSurface->freeWorkerCount == 1)
    {
        /* The first free worker returned back. */
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL, ownerSurface->workerAvaiableSignal, gcvTRUE));
    }

    /* Release access mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, ownerSurface->workerMutex));
    acquired = gcvFALSE;

    return nextWorker;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, ownerSurface->workerMutex));
    }

    return gcvNULL;
}


gctBOOL
veglSubmitWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display,
    IN VEGLWorkerInfo Worker,
    IN gctBOOL ScheduleSignals
    )
{
    Worker->prev =  Display->workerSentinel.prev;
    Worker->next = &Display->workerSentinel;

    Display->workerSentinel.prev->next = Worker;
    Display->workerSentinel.prev       = Worker;

    Worker->draw->lastSubmittedWorker = Worker;

    if (ScheduleSignals)
    {
#if gcdENABLE_VG
        if (Thread->openVGpipe && Thread->api == EGL_OPENVG_API)
        {
            gcsTASK_SIGNAL_PTR workerSignal;
            gcsTASK_SIGNAL_PTR startSignal;

#ifdef __QNXNTO__
            /* Allocate a cluster event. */
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
#if gcdGC355_PROFILER
                                               gcvNULL,
                                               0,0,0,
#   endif
                                               gcvBLOCK_PIXEL,
                                               1,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 1,
                                               (gctPOINTER *) &workerSignal)))
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }

            /* Allocate a cluster event. */
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
#if gcdGC355_PROFILER
                                               gcvNULL,
                                               0,0,0,
#   endif
                                               gcvBLOCK_PIXEL,
                                               1,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 1,
                                               (gctPOINTER *) &startSignal)))
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }
#else /* !__QNXNTO__ */

#if gcdGC355_PROFILER
            /* Allocate a cluster event. */
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
                                               gcvNULL,
                                               0,0,0,
                                               gcvBLOCK_PIXEL,
                                               2,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 2,
                                               (gctPOINTER *) &workerSignal)))
#   else
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
                                               gcvBLOCK_PIXEL,
                                               2,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 2,
                                               (gctPOINTER *) &workerSignal)))
#   endif
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }

            /* Determine the start signal set task pointer. */
            startSignal = (gcsTASK_SIGNAL_PTR) (workerSignal + 1);
#endif /* __QNXNTO__ */

            /* Fill in event info. */
            workerSignal->id      = gcvTASK_SIGNAL;
            workerSignal->process = Display->process;
            workerSignal->signal  = Worker->signal;

            startSignal->id       = gcvTASK_SIGNAL;
            startSignal->process  = Display->process;
            startSignal->signal   = Display->startSignal;
        }
        else
#endif /* gcdENABLE_VG */

        {
#if COMMAND_PROCESSOR_VERSION > 1
            gcsTASK_SIGNAL_PTR workerSignal;
            gcsTASK_SIGNAL_PTR startSignal;
#if gcdGC355_PROFILER
            /* Allocate a cluster event. */
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
                                               gcvNULL,
                                               0,0,0,
                                               gcvBLOCK_PIXEL,
                                               2,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 2,
                                               (gctPOINTER *) &workerSignal)))
#else
             if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
                                               gcvBLOCK_PIXEL,
                                               2,
                                               gcmSIZEOF(gcsTASK_SIGNAL) * 2,
                                               (gctPOINTER *) &workerSignal)))
#endif
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }

            /* Determine the start signal set task pointer. */
            startSignal = (gcsTASK_SIGNAL_PTR) (workerSignal + 1);

            /* Fill in event info. */
            workerSignal->id      = gcvTASK_SIGNAL;
            workerSignal->process = Display->process;
            workerSignal->signal  = Worker->signal;

            startSignal->id       = gcvTASK_SIGNAL;
            startSignal->process  = Display->process;
            startSignal->signal   = Display->startSignal;
#else
            gcsHAL_INTERFACE iface;

            iface.command            = gcvHAL_SIGNAL;
            iface.engine             = gcvENGINE_RENDER;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(Worker->signal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = gcmPTR_TO_UINT64(Display->process);
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Schedule the event. */
            if (gcmIS_ERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface)))
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }

            iface.command            = gcvHAL_SIGNAL;
            iface.engine             = gcvENGINE_RENDER;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(Display->startSignal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = gcmPTR_TO_UINT64(Display->process);
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Schedule the event. */
            if (gcmIS_ERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface)))
            {
                /* Bad surface. */
                veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                return gcvFALSE;
            }
#endif
        }
    }

    return gcvTRUE;
}

/* Worker thread to copy resolve buffer to display. */
veglTHREAD_RETURN WINAPI
veglSwapWorker(
    void* Display
    )
{
    VEGLDisplay display;
    VEGLWorkerInfo displayWorker;
    VEGLWorkerInfo currWorker;
    gctBOOL bStop = gcvFALSE;

    gcmHEADER_ARG("Display=0x%x", Display);

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    /* Cast the display object. */
    display = VEGL_DISPLAY(Display);

    while (gcvTRUE)
    {
        /* Wait for the start signal. */
        if gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, display->startSignal, gcvINFINITE))
        {
            break;
        }

        /* Check the thread's stop signal. */
        if (gcmIS_SUCCESS(gcoOS_WaitSignal(gcvNULL, display->stopSignal, 0)))
        {
            /* Stop had been signaled, exit. */
            bStop = gcvTRUE;
        }

#if !defined(UNDER_CE)
        /* Acquire synchronization mutex. */
        veglSuspendSwapWorker(display);
#endif

        currWorker = display->workerSentinel.next;

#if !defined(UNDER_CE)
        /* Release synchronization mutex. */
        veglResumeSwapWorker(display);
#endif

        for (;;)
        {
            if  ((currWorker == gcvNULL) ||
                 (currWorker->draw == gcvNULL))
            {
                break;
            }

            /* Wait for the worker's surface. */
            if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, currWorker->signal, gcvINFINITE)))
            {
                break;
            }

            /* The current worker is the one to be displayed. */
            displayWorker = currWorker;

            gcmDUMP_FRAMERATE();

            /* Post window back buffer. */
            display->platform->postWindowBackBuffer(display,
                                                    displayWorker->draw,
                                                    &displayWorker->backBuffer,
                                                    &displayWorker->region,
                                                    &displayWorker->damageHint);

            /* Acquire synchronization mutex. */
            veglSuspendSwapWorker(display);

            /* Free the more recent worker. */
            if (displayWorker != currWorker)
            {
                veglFreeWorker(displayWorker);
            }

            /* Free the current worker. */
            currWorker = veglFreeWorker(currWorker);

            /* Release synchronization mutex. */
            veglResumeSwapWorker(display);
        }

        if (bStop)
        {
            break;
        }
    }

    gcmFOOTER_ARG("return=%ld", (veglTHREAD_RETURN) 0);
    /* Success. */
    return (veglTHREAD_RETURN) 0;
}

void
veglSuspendSwapWorker(
    VEGLDisplay Display
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x", Display);

    gcmASSERT(Display != gcvNULL);
    if (Display->suspendMutex != gcvNULL)
    {
        gcmONERROR(gcoOS_AcquireMutex(gcvNULL,
                                        Display->suspendMutex,
                                        gcvINFINITE));
    }
    gcmFOOTER_NO();
    return;

OnError:
    gcmFOOTER();
    return;
}

void
veglResumeSwapWorker(
    VEGLDisplay Display
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x", Display);

    gcmASSERT(Display != gcvNULL);
    if (Display->suspendMutex != gcvNULL)
    {
        gcmONERROR(gcoOS_ReleaseMutex(gcvNULL,
                                      Display->suspendMutex));
    }
    gcmFOOTER_NO();
    return;

OnError:
    gcmFOOTER();
    return;
}

#if gcdENABLE_VG
static gceSTATUS
_ResolveRectVG(
    gcoSURF SrcSurface,
    gcoSURF DestSurface,
    gcsPOINT_PTR SrcOrigin,
    gcsPOINT_PTR DestOrigin,
    gcsPOINT_PTR RectSize
    )
{
    VEGLThreadData  Thread = veglGetThreadData();
    veglDISPATCH   *dispatch;

    if (Thread == gcvNULL)
    {
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    dispatch = _GetDispatch(Thread, gcvNULL);

    if ((dispatch != gcvNULL) &&  (dispatch->resolveVG != gcvNULL))
    {
        (*dispatch->resolveVG)(Thread->context->context, DestSurface);
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_GENERIC_IO;
}
#endif

/*
 * Local function to combine common code between eglSwapBuffers and
 * eglSwapBuffersRegionEXT.
 * DRI verision.
 */
static EGLBoolean
_eglSwapBuffersRegionDRI(
    EGLDisplay Dpy,
    EGLSurface Draw,
    struct eglRegion * SwapRegion,
    struct eglRegion * DamageHint
    )
{
    EGLBoolean result = EGL_TRUE;
    VEGLThreadData  thread;
    VEGLDisplay dpy;
    VEGLSurface draw = gcvNULL;
    VEGLPlatform platform;
    gctUINT width, height;
    gcoSURF resolveTarget = gcvNULL;
    gctPOINTER resolveBits[3] = {gcvNULL};
    struct eglBackBuffer backBuffer;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x", Dpy, Draw);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);

    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    platform = dpy->platform;

    do
    {
        /* Test if EGLDisplay structure has been initialized. */
        if (!dpy->initialized)
        {
            /* Not initialized. */
            veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
            result = EGL_FALSE;
            break;
        }

        /* Test for valid EGLSurface structure */
        draw = (VEGLSurface)veglGetResObj(dpy,
                                          (VEGLResObj*)&dpy->surfaceStack,
                                          (EGLResObj)Draw,
                                          EGL_SURFACE_SIGNATURE);

        if (draw == gcvNULL)
        {
            /* Bad surface*/
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /* Per spec EGL_BAD_SURFACE if surface is not bound to thread's current context */
        if ((thread->context == EGL_NO_CONTEXT) ||
            (   (thread->context != EGL_NO_CONTEXT) &&
                (thread->context->draw != Draw)))
        {
            /* Bad surface. */
            veglSetEGLerror(thread,  EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /* Test if surface is locked. */
        if (draw->locked)
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        /* Only window surfaces need to be swapped. */
        if (!(draw->type & EGL_WINDOW_BIT))
        {
            /* Not a window surface. */
            veglSetEGLerror(thread,  EGL_SUCCESS);
            break;
        }

        /* Only back buffers need to be posted. */
        if (draw->buffer != EGL_BACK_BUFFER)
        {
            /* Not a back buffer. */
            veglSetEGLerror(thread,  EGL_SUCCESS);
            break;
        }

        /* Check native window. */
        if (draw->winInfo == gcvNULL)
        {
            /* Invalid native window. */
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            break;
        }

        /* Borrow window back buffer. */
        if (!platform->getWindowBackBuffer(dpy, draw, &backBuffer))
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        /* Get shortcut. */
        resolveTarget = backBuffer.surface;

        /* Lock for memory. */
        if (gcmIS_ERROR(gcoSURF_Lock(resolveTarget, gcvNULL, resolveBits)))
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        _Flush(thread);

        width  = draw->config.width;
        height = draw->config.height;

        VEGL_DUMP_SWAP_TAG(draw, resolveTarget);
#if gcdDUMP_2DVG
        {
            gctUINT32 physical[3] = {0};
            gctPOINTER logical[3] = {gcvNULL};
            gctINT stride;
            gctUINT width, height;

            gcoHAL_Flush(gcvNULL);
            gcoSURF_Lock(draw->renderTarget, physical, logical);
            gcoSURF_GetAlignedSize(draw->renderTarget, &width, &height, &stride);

            gcoHAL_Commit(gcvNULL,gcvTRUE);
            gcmDUMP(gcvNULL,
                "@[swap 0x%08X %dx%d +%u]",
                physical[0],
                width, height,
                stride);
            gcmDUMP_BUFFER(gcvNULL,
                "framebuffer",
                physical[0],logical[0],
                0,
                stride*height);
            gcoSURF_Unlock(draw->renderTarget, logical[0]);
        }
#endif
#if gcdENABLE_VG
        if ( thread->openVGpipe && (thread->api == EGL_OPENVG_API) )
        {
            if (!dpy->platform->rsForSwap(dpy->localInfo, draw->hwnd, (gctPOINTER)_ResolveRectVG))
            {
                veglSetEGLerror(thread, EGL_BAD_PARAMETER);
                break;
            }
        } else {
            dpy->platform->rsForSwap(dpy->localInfo, draw->hwnd, (gctPOINTER)gcvNULL);
        }
#else
        dpy->platform->rsForSwap(dpy->localInfo, draw->hwnd, (gctPOINTER)gcvNULL);
#endif

        if (!dpy->platform->swapBuffers(dpy->localInfo,
                                        draw->hwnd,
                                        draw->renderTarget,
                                        resolveTarget,
                                        resolveBits[0],
                                        &width,
                                        &height))
        {
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            break;
        }

        VEGL_DUMP_TGA(thread, draw);

        /* Check for window resize. */
        if ((draw->config.width != (EGLint)width) ||
            (draw->config.height != (EGLint)height))
        {
            if (gcmIS_ERROR(veglResizeSurface(dpy, draw, width, height)))
            {
                result = gcvFALSE;
                break;
            }
        }
    }
    while(gcvFALSE);

    if (resolveTarget != gcvNULL)
    {
        if (resolveBits[0] != gcvNULL)
        {
            /* Unlock. */
            gcmVERIFY_OK(gcoSURF_Unlock(resolveTarget, gcvNULL));
        }

        /* Cancel the back buffer, actually does nothing for DRI. */
        platform->cancelWindowBackBuffer(dpy, draw, &backBuffer);
    }

#if VIVANTE_PROFILER
    if (_ProfilerCallback(thread, 0, 0))
    {
        if (thread->api == EGL_OPENGL_ES_API)
        {
            if (MAJOR_API_VER(thread->context->client) == 1)
            {
                /* GL_PROFILER_FRAME_END */
                _ProfilerCallback(thread, 10, 0);
            }
            else if (MAJOR_API_VER(thread->context->client) == 2 || MAJOR_API_VER(thread->context->client) == 3)
            {
                /* GL3_PROFILER_FRAME_END */
                _ProfilerCallback(thread, 10, 0);
            }
        }
        else if (thread->api == EGL_OPENVG_API)
        {
            _ProfilerCallback(thread, 10, 0);
        }
    }
#endif

    gcmFOOTER_ARG("%d", result);

    return result;
}


/*
 * Compute valid swap region.
 * Clip swap region with surface size.
 * Rects form: {x, y, width, height}, relative to the bottom-left of the surface.
 */
static gcmINLINE EGLBoolean
_ComputeSwapRegion(
    VEGLDisplay Dpy,
    VEGLSurface Draw,
    struct eglRegion * SwapRegion
    )
{
    EGLint i;

    if (!SwapRegion || SwapRegion->numRects == 0)
    {
        /* Full surface. */
        Draw->clipRegion.rects[0] = 0;
        Draw->clipRegion.rects[1] = 0;
        Draw->clipRegion.rects[2] = Draw->config.width;
        Draw->clipRegion.rects[3] = Draw->config.height;
        Draw->clipRegion.numRects = 1;
    }
    else
    {
        EGLint numRects = 0;

        if (!_ReallocRegion(&Draw->clipRegion, SwapRegion->numRects))
        {
            return EGL_FALSE;
        }

        for (i = 0; i < SwapRegion->numRects; ++i)
        {
            EGLint left   = SwapRegion->rects[i * 4 + 0];
            EGLint bottom = SwapRegion->rects[i * 4 + 1];
            EGLint right  = left + SwapRegion->rects[i * 4 + 2];
            EGLint top    = bottom + SwapRegion->rects[i * 4 + 3];

            /* Clip with surface size. */
            left   = gcmMAX(left, 0);
            bottom = gcmMAX(bottom, 0);
            right  = gcmMIN(right,  Draw->config.width);
            top    = gcmMIN(top, Draw->config.height);

            if (left >= right || bottom >= top)
            {
                /* Invalid rect. */
                continue;
            }

            Draw->clipRegion.rects[numRects * 4 + 0] = left;
            Draw->clipRegion.rects[numRects * 4 + 1] = bottom;
            Draw->clipRegion.rects[numRects * 4 + 2] = right - left;
            Draw->clipRegion.rects[numRects * 4 + 3] = top - bottom;
            numRects++;
        }

        Draw->clipRegion.numRects = numRects;
    }

    return EGL_TRUE;
}

static gcmINLINE EGLBoolean
_ComputeDamageHint(
    VEGLDisplay Dpy,
    VEGLSurface Draw,
    struct eglRegion * DamageHint
    )
{
    if (!DamageHint || DamageHint->numRects == 0)
    {
        /* Full surface. */
        Draw->damageHint.rects[0] = 0;
        Draw->damageHint.rects[1] = 0;
        Draw->damageHint.rects[2] = Draw->config.width;
        Draw->damageHint.rects[3] = Draw->config.height;
        Draw->damageHint.numRects = 1;

        return EGL_TRUE;
    }
    else
    {
        return _CopyRegion(&Draw->damageHint, DamageHint);
    }
}

/*
 * EGL_KHR_partial_update.
 * Clip swap region with damage region.
 * Must work together with buffer age.
 */
static void
_ClipDamageRegion(
    VEGLDisplay Dpy,
    VEGLSurface Draw,
    struct eglBackBuffer * BackBuffer
    )
{
    EGLint age = 0;

    if (!Draw->damageValid)
    {
        return;
    }

    /* Reset damage set flag. */
    Draw->damageValid = EGL_FALSE;

    if (Draw->renderMode != VEGL_INDIRECT_RENDERING)
    {
        /*
         * EGL_KHR_partial_update is not useful for direct rendering, skip
         * direct rendering case.
         */
        return;
    }

    /* Get Surface age. No need to clip for fully new buffer (age == 0). */
    if (Dpy->platform->queryBufferAge &&
        Dpy->platform->queryBufferAge(Dpy, Draw, BackBuffer, &age) &&
        age > 0)
    {
        EGLint i, j;
        EGLint index;
        EGLint left, top, right, bottom;
        EGLint numRects;

        /* Use bounding box for damage currently. */
        gcsRECT bounding = {0, 0, 0, 0};
        EGLBoolean boundingValid = EGL_FALSE;

        index = (Draw->curDamage == 0) ? (EGL_WORKER_COUNT - 1)
              : (Draw->curDamage - 1);

        for (i = 0; i < age; i++)
        {
            for (j = 0; j < Draw->damage[index].numRects; j = j + 4)
            {
                left   = Draw->damage[index].rects[j + 0];
                bottom = Draw->damage[index].rects[j + 1];
                right  = left + Draw->damage[index].rects[j + 2];
                top    = bottom + Draw->damage[index].rects[j + 3];

                if (left >= right || bottom >= top)
                {
                    /* Skip invalid rectangle. */
                    continue;
                }

                if (boundingValid)
                {
                    /* Merge rects */
                    bounding.left   = gcmMIN(left,   bounding.left);
                    bounding.bottom = gcmMIN(bottom, bounding.bottom);
                    bounding.right  = gcmMAX(right,  bounding.right);
                    bounding.top    = gcmMAX(top,    bounding.top);
                }
                else
                {
                    bounding.left   = left;
                    bounding.bottom = bottom;
                    bounding.right  = right;
                    bounding.top    = top;
                    boundingValid = EGL_TRUE;
                }
            }

            index = (index == 0) ? (EGL_WORKER_COUNT - 1)
                  : (index - 1);
        }

        if (!boundingValid)
        {
            /* Empty damage region, no rectangle to resolve out. */
            Draw->clipRegion.numRects = 0;
            return;
        }

        numRects = 0;

        /* Do clip with the damage bounding box. */
        for (i = 0; i < Draw->clipRegion.numRects; i++)
        {
            left   = Draw->clipRegion.rects[i * 4 + 0];
            bottom = Draw->clipRegion.rects[i * 4 + 1];
            right  = left + Draw->clipRegion.rects[i * 4 + 2];
            top    = bottom + Draw->clipRegion.rects[i * 4 + 3];

            /* clip with bouding damage. */
            left   = gcmMAX(left,   bounding.left);
            bottom = gcmMAX(bottom, bounding.bottom);
            right  = gcmMIN(right,  bounding.right);
            top    = gcmMIN(top,    bounding.top);

            if (left >= right || bottom >= top)
            {
                /* Skip invalid rectangle. */
                continue;
            }

            Draw->clipRegion.rects[numRects * 4 + 0] = left;
            Draw->clipRegion.rects[numRects * 4 + 1] = bottom;
            Draw->clipRegion.rects[numRects * 4 + 2] = right - left;
            Draw->clipRegion.rects[numRects * 4 + 3] = top - bottom;
            numRects++;
        }

        Draw->clipRegion.numRects = numRects;
    }
}

#if gcdENABLE_3D
/*
 * Resolve internal render target to back buffer.
 * Returns EGL_TRUE on success.
 */
static EGLBoolean
_ResolveRects(
    gcoSURF Source,
    gcoSURF Dest,
    EGLint Width,
    EGLint Height,
    EGLBoolean YInverted,
    struct eglRegion * Region
    )
{
    EGLint i;
    gcsSURF_RESOLVE_ARGS rlvArgs = {0};
    gceSTATUS status = gcvSTATUS_OK;

    gcsSURF_VIEW rtView =  {Source,  0, 1};
    gcsSURF_VIEW tgtView = {Dest, 0, 1};

    /* Resolve engine alignment limitation. */
    gctUINT xAlignment = 0;
    gctUINT yAlignment = 0;
    gctUINT widthAlignment  = 0;
    gctUINT heightAlignment = 0;

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_BLT_ENGINE) != gcvTRUE)
    {
        /* Unlikey to fail. */
        gcmVERIFY_OK(
            gcoSURF_GetResolveAlignment(Source,
                                        &xAlignment,
                                        &yAlignment,
                                        &widthAlignment,
                                        &heightAlignment));
    }

    if (xAlignment < widthAlignment)
    {
        xAlignment = widthAlignment;
    }

    rlvArgs.version = gcvHAL_ARG_VERSION_V2;

    rlvArgs.uArgs.v2.yInverted = !YInverted;
    rlvArgs.uArgs.v2.numSlices = 1;

    for (i = 0; i < Region->numRects; i++)
    {
        /* Relative to upper-left of display. */
        EGLint left = Region->rects[i * 4 + 0];
        EGLint top  = Region->rects[i * 4 + 1];
        EGLint right  = left + Region->rects[i * 4 + 2];
        EGLint bottom = top  + Region->rects[i * 4 + 3];

        if (YInverted)
        {
            /* Flip Y for Y-inverted rendering. */
            EGLint t = Height - bottom;
            bottom = Height - top;
            top = t;
        }

        if (xAlignment != 0)
        {
            left   = gcmALIGN_BASE(left, xAlignment);
            top    = gcmALIGN_BASE(top,  yAlignment);
            right  = left + gcmALIGN(right  - left, widthAlignment);
            bottom = top  + gcmALIGN(bottom - top,  heightAlignment);
        }

        rlvArgs.uArgs.v2.srcOrigin.x = rlvArgs.uArgs.v2.dstOrigin.x = left;
        rlvArgs.uArgs.v2.srcOrigin.y = rlvArgs.uArgs.v2.dstOrigin.y = top;
        rlvArgs.uArgs.v2.rectSize.x  = right  - left;
        rlvArgs.uArgs.v2.rectSize.y  = bottom - top;

        status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);

        if (gcmIS_ERROR(status))
        {
            /* Bad surface. */
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}
#endif

/*
 * Local function to combine common code between eglSwapBuffers and
 * eglSwapBuffersRegionEXT.
 * Non-DRI verision.
 */
static EGLBoolean
_SwapBuffersRegion(
    VEGLThreadData Thread,
    EGLDisplay Dpy,
    EGLSurface Draw
    )
{
    EGLBoolean result;
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLPlatform platform;

    do
    {
        VEGLSurface draw;
        struct eglBackBuffer backBuffer;
        EGLBoolean synchronous;

        /* Create shortcuts to objects. */
        thread   = Thread;
        dpy      = VEGL_DISPLAY(Dpy);
        draw     = VEGL_SURFACE(Draw);
        platform = dpy->platform;

#if defined(ANDROID) && (ANDROID_SDK_VERSION < 17)
        if (draw->skipResolve)
        {
            /* EGL_ANDROID_get_render_buffer. */
            if (!platform->postWindowBackBuffer(dpy, draw,
                                                &draw->backBuffer,
                                                &draw->clipRegion,
                                                &draw->damageHint))
            {
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }

            /* Clear back buffer flag. */
            draw->backBuffer.context = gcvNULL;
            draw->backBuffer.surface = gcvNULL;
            draw->skipResolve = EGL_FALSE;

            /* Get native window back buffer for next frame. */
            if (!platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer))
            {
                /* Cannot get window back buffer. */
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }

            /* Success. */
            veglSetEGLerror(thread, EGL_SUCCESS);
            break;
        }
#endif

        if (draw->newSwapModel)
        {
            /*
             * In newSwapModel, window back buffer is dequeued in previous
             * eglSwapBuffers or eglMakeCurrent.
             */
            if (draw->backBuffer.surface == gcvNULL)
            {
                /* Requires back buffer surface in this swap mode. */
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }

            /* Use direct rendered backBuffer. */
            backBuffer = draw->backBuffer;
        }
        else
        {
            /* Get window back buffer. */
            result = platform->getWindowBackBuffer(dpy, draw, &backBuffer);

            if (!result)
            {
                gcmTRACE(
                    gcvLEVEL_ERROR,
                    "%s(%d): Get back buffer failed",
                    __FUNCTION__, __LINE__
                    );
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }
        }

        /* EGL_KHR_partial_update: Clip with damage region. */
        _ClipDamageRegion(dpy, draw, &backBuffer);

        /* Update buffer age. */
        platform->updateBufferAge(dpy, draw, &backBuffer);

#if gcdENABLE_3D
        if (!draw->openVG)
        {
            /* 3D pipe. */
            /* In no-resolve mode, check if render target pixels are updated. */
            if ((draw->renderMode > VEGL_INDIRECT_RENDERING) &&
                !gcoSURF_QueryFlags(draw->renderTarget, gcvSURF_FLAG_CONTENT_UPDATED) &&
                (draw->swapBehavior == EGL_BUFFER_PRESERVED))
            {
                /*
                 * Has drawn nothing in direct rendering mode, copy from
                 * previous render target to preserve pixels
                 */
                gcsSURF_VIEW prevView = {draw->prevRenderTarget, 0, 1};
                gcsSURF_VIEW rtView   = {draw->renderTarget,     0, 1};

                if (draw->prevRenderTarget != gcvNULL)
                {
                    /* Do the copy. */
                    if (gcmIS_ERROR(gcoSURF_ResolveRect(&prevView, &rtView, gcvNULL)))
                    {
                        veglSetEGLerror(thread, EGL_BAD_SURFACE);
                        break;
                    }
                }

                VEGL_DUMP_SWAP_TAG(draw, draw->renderTarget);
            }
            else if (draw->renderMode == VEGL_DIRECT_RENDERING_FCFILL)
            {
                gcsSURF_VIEW rtView = {draw->renderTarget, 0, 1};

                /* Basic No-resolve path. */
                if (gcmIS_ERROR(gcoSURF_FillFromTile(&rtView)))
                {
                    veglSetEGLerror(thread, EGL_BAD_SURFACE);
                    break;
                }

                /* Flush the pipe. */
                _Flush(thread);

                VEGL_DUMP_FCFILL_TAG(draw, draw->renderTarget);
            }
            else if (draw->renderMode > VEGL_INDIRECT_RENDERING)
            {
                /* direct rendering without fcfill . */

                /* Flush render target. */
                gcmVERIFY_OK(gcoSURF_Flush(draw->renderTarget));

                if (draw->renderMode >= VEGL_DIRECT_RENDERING_FC_NOCC)
                {
                    /* direct rendering with tile status / compression. */
                    /*
                     * Flush tile status and later compositor can read the tile
                     * status buffer correctly.
                     * Do not turn off hardware tile status, this buffer can still
                     * be access when buffer preserve.
                     */
                    gcsSURF_VIEW rtView = {draw->renderTarget, 0, 1};
                    gcmVERIFY_OK(gcoSURF_FlushTileStatus(&rtView, gcvFALSE));
                }

                /* Flush the pipe. */
                _Flush(thread);

                VEGL_DUMP_FRAME_TAG(draw, draw->renderTarget);
            }
            else
            {
                /* Flush the pipe. */
                _Flush(thread);

                /* Resolve path. */
                if (!_ResolveRects(draw->renderTarget,
                                   backBuffer.surface,
                                   draw->config.width,
                                   draw->config.height,
                                   1,
                                   &draw->clipRegion))
                {
                    /* Error in resolve out. */
                    veglSetEGLerror(Thread, EGL_BAD_SURFACE);
                    break;
                }

                VEGL_DUMP_SWAP_TAG(draw, backBuffer.surface);
            }
        }
        else
#endif
        {
            /* VG pipe. */
#if gcdENABLE_VG
            /* Flush the pipe. */
            _Flush(thread);

            if (draw->renderMode == VEGL_INDIRECT_RENDERING)
            {
                veglDISPATCH * dispatch = gcvNULL;
                dispatch = _GetDispatch(Thread, gcvNULL);

                if ((dispatch != gcvNULL) && (dispatch->resolveVG != gcvNULL))
                {
                    (*dispatch->resolveVG)(Thread->context->context,
                                           backBuffer.surface);
                }
            }

            VEGL_DUMP_SWAP_TAG(draw, backBuffer.surface);
#if gcdDUMP_2DVG
            {
                gctUINT32 physical[3] = {0};
                gctPOINTER logical[3] = {gcvNULL};
                gctINT stride;
                gctUINT width, height;

                gcoSURF_Lock(backBuffer.surface, physical, logical);
                gcoSURF_GetAlignedSize(backBuffer.surface, &width, &height, &stride);

                gcoHAL_Flush(gcvNULL);
                gcoHAL_Commit(gcvNULL,gcvTRUE);
                gcmDUMP(gcvNULL,
                    "@[swap 0x%08X %dx%d +%u]",
                    physical[0],
                    width, height,
                    stride);
                gcmDUMP_BUFFER(gcvNULL,
                    "framebuffer",
                    physical[0],logical[0],
                    0,
                    stride*height);
                gcoSURF_Unlock(backBuffer.surface, logical[0]);
            }
#endif
#endif
        }

        VEGL_DUMP_TGA(thread, draw);

        synchronous = platform->synchronousPost(dpy, draw);

        if (!backBuffer.flip)
        {
            /* No flip, commit accumulated commands. */
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, synchronous));

            /* Success. */
            veglSetEGLerror(thread, EGL_SUCCESS);

            /* No need to post or switch back buffer. */
            break;
        }

#if defined(ANDROID) && gcdANDROID_NATIVE_FENCE_SYNC >= 2
        /* Sumit swap worker / Wait until done. */
        if (synchronous)
        {
            /* Commit-stall. */
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));

            /* Post back buffer. */
            if (!platform->postWindowBackBuffer(dpy, draw,
                                                &backBuffer,
                                                &draw->clipRegion,
                                                &draw->damageHint))
            {
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }
        }
        else
        {
            /* Using android native fence sync. */
            if (!platform->postWindowBackBufferFence(dpy, draw,
                                                     &backBuffer,
                                                     &draw->damageHint))
            {
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }
        }
#else
        if(platform->platform == EGL_PLATFORM_GBM_VIV && (!synchronous))
        {
            /* Using android native fence sync. */
            if (!platform->postWindowBackBufferFence(dpy, draw,
                                                &backBuffer,
                                                &draw->damageHint))
            {
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }
        }
        else if ((dpy->workerThread == gcvNULL) || synchronous)
        {
            /* Commit-stall. */
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));

            /* Post back buffer. */
            if (!platform->postWindowBackBuffer(dpy, draw,
                                                &backBuffer,
                                                &draw->clipRegion,
                                                &draw->damageHint))
            {
                veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                break;
            }
        }
        else
        {
            /* Using swap worker. */
            VEGLWorkerInfo worker;

            /* Find an available worker. */
            worker = veglGetWorker(thread, dpy, draw);

            if (worker == gcvNULL)
            {
                /* Something horrible has happened. */
                veglSetEGLerror(thread,  EGL_BAD_ACCESS);
                break;
            }

            /* Fill in the worker information. */
            worker->draw         = draw;
            worker->backBuffer   = backBuffer;
            worker->targetSignal = gcvNULL;
            _CopyRegion(&worker->region, &draw->clipRegion);
            _CopyRegion(&worker->damageHint, &draw->damageHint);

            /* Suspend the worker thread. */
            veglSuspendSwapWorker(dpy);

            /* Submit the worker. */
            veglSubmitWorker(thread, dpy, worker, gcvTRUE);

            /* Resume the swap thread. */
            veglResumeSwapWorker(dpy);

            /* Commit the command buffer. */
            if (gcmIS_ERROR(gcoHAL_Commit(gcvNULL, gcvFALSE)))
            {
                /* Something horrible has happened. */
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                break;
            }
        }
#endif

        if (draw->newSwapModel)
        {
            /* Get next back buffer for rendering in newSwapModel. */
            /* Reset back buffer. */
            draw->backBuffer.context = gcvNULL;
            draw->backBuffer.surface = gcvNULL;

            result = EGL_TRUE;

            /* Get native window back buffer for next frame. */
            if (!platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer))
            {
                draw->backBuffer.surface = gcvNULL;
                draw->backBuffer.context = gcvNULL;

                /* Cannot get window back buffer. */
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                result = EGL_FALSE;

                /*
                 * Do not break here, need update client drawable if
                 * direct rendering mode is enabled. See below.
                 */
            }

            /* Switch render target in no-resolve mode. */
            if (draw->renderMode > 0)
            {
                if (draw->prevRenderTarget)
                {
                    /* Dereference previous back buffer. */
                    gcoSURF_Destroy(draw->prevRenderTarget);
                    draw->prevRenderTarget = gcvNULL;
                }

                if (draw->swapBehavior == EGL_BUFFER_PRESERVED)
                {
                    /* Becomes previous back buffer. */
                    draw->prevRenderTarget = draw->renderTarget;
                    draw->renderTarget = gcvNULL;
                }
                else
                {
                    /* Dereference last window back buffer. */
                    gcoSURF_Destroy(draw->renderTarget);
                }

                /* Get renderTarget from new window back buffer. */
                draw->renderTarget = draw->backBuffer.surface;

                if (draw->renderTarget)
                {
                    /* Reference new window back buffer. */
                    gcoSURF_ReferenceSurface(draw->renderTarget);

                    /* Update preserved flag for next renderTarget. */
                    gcmVERIFY_OK(gcoSURF_SetFlags(
                        draw->renderTarget,
                        gcvSURF_FLAG_CONTENT_PRESERVED,
                        (draw->swapBehavior == EGL_BUFFER_PRESERVED)
                        ));
                }

                /* Sync drawable with renderTarget. */
                draw->drawable.rtHandles[0] = draw->renderTarget;
                draw->drawable.prevRtHandles[0] = draw->prevRenderTarget;

                /* Update drawable to api. */
                if (!_SetDrawable(thread,
                                  thread->context,
                                  &draw->drawable,
                                  &draw->drawable))
                {
                    veglSetEGLerror(thread, EGL_BAD_CONTEXT);
                    break;
                }
            }

            if (result == EGL_FALSE)
            {
                /* Error. */
                break;
            }

            /* reset the flags.*/
            if (draw->renderTarget != gcvNULL)
            {
                gcoSURF_SetFlags(draw->renderTarget,
                                 gcvSURF_FLAG_CONTENT_UPDATED,
                                 gcvFALSE);
            }

            if (draw->depthBuffer != gcvNULL)
            {
                gcoSURF_SetFlags(draw->depthBuffer,
                                 gcvSURF_FLAG_CONTENT_UPDATED,
                                 gcvFALSE);
            }
        }

        /* Success. */
        veglSetEGLerror(thread, EGL_SUCCESS);
    }
    while (EGL_FALSE);

    /* Determine result. */
    result = (thread->error == EGL_SUCCESS)
        ? EGL_TRUE
        : EGL_FALSE;

#if gcdENABLE_3D
    /* Statistics Frame End */
    gcfSTATISTICS_MarkFrameEnd();
#endif

    /* Return result. */
    return result;
}

static EGLBoolean
_eglSwapBuffersRegion(
    EGLDisplay Dpy,
    EGLSurface Draw,
    struct eglRegion * SwapRegion,
    struct eglRegion * DamageHint
    )
{
    EGLBoolean result = EGL_TRUE;
    VEGLThreadData  thread;
    VEGLDisplay dpy;
    VEGLSurface draw;
    EGLint width, height;

    gcmHEADER_ARG("Dpy=0x%p Draw=0x%p SwapRegion=0x%p",
                  Dpy, Draw, (void *)SwapRegion);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;

    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread, EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;

    }

    do
    {
        /* Test if EGLDisplay structure has been initialized. */
        if (!dpy->initialized)
        {
            /* Not initialized. */
            veglSetEGLerror(thread, EGL_NOT_INITIALIZED);;
            result = EGL_FALSE;
            break;
        }

        /* Test for valid EGLSurface structure */
        draw = (VEGLSurface) veglGetResObj(dpy,
                                           (VEGLResObj*)&dpy->surfaceStack,
                                           (EGLResObj)Draw,
                                           EGL_SURFACE_SIGNATURE);
        if (draw == gcvNULL)
        {
            /* Bad surface*/
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /*
         * Per Spec:
         * EGL_BAD_SURFACE if surface is not bound to thread's current context
         */
        if ((thread->context == EGL_NO_CONTEXT) ||
            (thread->context->draw != draw))
        {
            /* Bad surface. */
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /* Test if surface is locked. */
        if (draw->locked)
        {
            veglSetEGLerror(thread, EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        /* Only window surfaces need to be swapped. */
        if (!(draw->type & EGL_WINDOW_BIT))
        {
            /* Not a window surface. */
            veglSetEGLerror(thread, EGL_SUCCESS);
            break;
        }

        /* Only back buffers need to be posted. */
        if (draw->buffer != EGL_BACK_BUFFER)
        {
            /* Not a back buffer. */
            veglSetEGLerror(thread, EGL_SUCCESS);
            break;
        }

        /* Check native window. */
        if (draw->winInfo == gcvNULL)
        {
            /* Invalid native window. */
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            break;
        }

        /* Test region parameters. */
        if (!_ValidateRegion(SwapRegion) ||
            !_ValidateRegion(DamageHint))
        {
            /* Invalid parameters. */
            veglSetEGLerror(thread, EGL_BAD_PARAMETER);
            break;
        }

#if gcdFRAME_DB
        /* Add (previous) frame to the database. */
        gcoHAL_AddFrameDB();
#endif

        /* Clip swap region with surface size. */
        if (!_ComputeSwapRegion(dpy, draw, SwapRegion))
        {
            /* Out of memory. */
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            break;
        }

        /* Compute damage hints. */
        if (!_ComputeDamageHint(dpy, draw, DamageHint))
        {
            /* Out of memory. */
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            break;
        }

        /* Call generic function. */
        result = _SwapBuffersRegion(thread, Dpy, Draw);

        /* Buffer swapped. */
        draw->initialFrame = EGL_FALSE;

        if (draw->winInfo == gcvNULL)
        {
            break;
        }

        /* Query window parameters. */
        result = dpy->platform->getWindowSize(dpy, draw, &width, &height);

        if (!result)
        {
            /* Window is gone. */
            break;
        }

        if ((draw->config.width  == width) &&
            (draw->config.height == height)
            )
        {
            break;
        }

        /* Make sure all workers have been processed. */
        if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL,
                                         draw->workerDoneSignal,
                                         gcvINFINITE)))
        {
            result = gcvFALSE;
            break;
        }

        /* Native window resized. */
        if (gcmIS_ERROR(veglResizeSurface(dpy, draw,
                                          (gctUINT) width,
                                          (gctUINT) height)))
        {
            result = gcvFALSE;
            break;
        }

        if (gcmIS_ERROR(gcoHAL_Commit(gcvNULL, gcvTRUE)))
        {
            /* Bad surface. */
            veglSetEGLerror(thread, EGL_BAD_SURFACE);

            result = gcvFALSE;
            break;
        }

        result = gcvTRUE;
    }
    while (gcvFALSE);

#if VIVANTE_PROFILER
    if (_ProfilerCallback(thread, 0, 0))
    {
        if (thread->api == EGL_OPENGL_ES_API)
        {
            if (MAJOR_API_VER(thread->context->client) == 1)
            {
                /* GL_PROFILER_FRAME_END */
                _ProfilerCallback(thread, 10, 0);
            }
            else if (MAJOR_API_VER(thread->context->client) == 2 || MAJOR_API_VER(thread->context->client) == 3)
            {
                /* GL3_PROFILER_FRAME_END */
                _ProfilerCallback(thread, 10, 0);
            }
        }
        else if (thread->api == EGL_OPENVG_API)
        {
            _ProfilerCallback(thread, 10, 0);
        }
    }
#endif

    gcmFOOTER_ARG("%d", result);
    return result;
}

static EGLBoolean
veglSwapBuffers(
    EGLDisplay Dpy,
    EGLSurface Draw,
    struct eglRegion * SwapRegion,
    struct eglRegion * DamageHint
    )
{
    EGLBoolean result = EGL_FALSE;

    VEGLDisplay dpy = veglGetDisplay(Dpy);



#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_SKIP_SWAP && gcdDEBUG_OPTION_SKIP_FRAMES
    {
        static gctUINT count = 0;
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            if (count++ % gcdDEBUG_OPTION_SKIP_FRAMES != 0)
            {
                return EGL_TRUE;
            }
        }
    }
#endif


    if(dpy && dpy->platform && dpy->platform->platform == EGL_PLATFORM_DRI_VIV)
    {
        result = _eglSwapBuffersRegionDRI(Dpy, Draw, gcvNULL, gcvNULL);
    }
    else
    {
        result = _eglSwapBuffersRegion(Dpy, Draw, SwapRegion, DamageHint);
    }


#if gcdGC355_PROFILER
    {
        static int frameCount = 0;
        static gctUINT64 startTimeusec;

        gctUINT64 endTimeusec = 0;
        gctUINT64 deltaValue = 0;
        gctSIZE_T length;
        gctCHAR savedValue[256] = {'\0'};
        gctUINT offset = 0;

        VEGLThreadData thread = veglGetThreadData();
        VEGLContext context = thread->context;

        gcoOS_GetTime(&endTimeusec);

        frameCount++;
        if (frameCount == 1)
        {
            startTimeusec = context->appStartTime;
        }

        deltaValue = endTimeusec - startTimeusec;
        startTimeusec = endTimeusec;
        gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "The elapsed time for Frame #%d = %llu(microsec) \n", frameCount, deltaValue);
        length = gcoOS_StrLen((gctSTRING)savedValue, gcvNULL);
        gcoOS_Write(gcvNULL, context->apiTimeFile, length, savedValue);
    }
#endif

#if gcdFRAMEINFO_STATISTIC
    /* Clear info for current frame */
    gcoHAL_FrameInfoOps(gcvNULL,
                        gcvFRAMEINFO_COMPUTE_NUM,
                        gcvFRAMEINFO_OP_ZERO,
                        gcvNULL);
    gcoHAL_FrameInfoOps(gcvNULL,
                        gcvFRAMEINFO_DRAW_NUM,
                        gcvFRAMEINFO_OP_ZERO,
                        gcvNULL);
    gcoHAL_FrameInfoOps(gcvNULL,
                        gcvFRAMEINFO_DUAL16_NUM,
                        gcvFRAMEINFO_OP_ZERO,
                        gcvNULL);
    /* Update frame count */
    gcoHAL_FrameInfoOps(gcvNULL,
                        gcvFRAMEINFO_FRAME_NUM,
                        gcvFRAMEINFO_OP_INC,
                        gcvNULL);

#endif

    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapBuffers(
    EGLDisplay Dpy,
    EGLSurface Draw
    )
{
    EGLBoolean result;
    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x", Dpy, Draw);

    gcmDUMP_API("${EGL eglSwapBuffers 0x%08X 0x%08X}", Dpy, Draw);
    VEGL_TRACE_API(SwapBuffers)(Dpy, Draw);

    result = veglSwapBuffers(Dpy, Draw, gcvNULL, gcvNULL);

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapBuffersRegionEXT(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLint NumRects,
    const EGLint* Rects
    )
{
    EGLBoolean result;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x NumRects=0x%x Rects=0x%x", Dpy, Draw, NumRects, Rects);
    gcmDUMP_API("${EGL eglSwapBuffersRegionEXT 0x%08X 0x%08X %d 0x%x}", Dpy, Draw, NumRects, Rects);

    result = veglSwapBuffers(Dpy, Draw, gcvNULL, gcvNULL);

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapBuffersWithDamageKHR(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLint *Rects,
    EGLint NumRects
    )
{
    EGLBoolean result;
    struct eglRegion region =
    {
        NumRects,
        0,
        Rects,
    };

    gcmHEADER_ARG("Dpy=0x%p Surface=0x%p rects=0x%p n_rects=%d",
                  Dpy, Surface, (void *)Rects, NumRects);
    gcmDUMP_API("${EGL eglSwapBuffersWithDamageKHR 0x%p 0x%p 0x%p 0x%X}",
                Dpy, Surface, (void *)Rects, NumRects);
    VEGL_TRACE_API(SwapBuffersWithDamageKHR)(Dpy, Surface, Rects, NumRects);

    result = veglSwapBuffers(Dpy, Surface, gcvNULL, &region);

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapBuffersWithDamageEXT(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLint *Rects,
    EGLint NumRects
    )
{
    EGLBoolean result;
    struct eglRegion region =
    {
        NumRects,
        0,
        Rects,
    };

    gcmHEADER_ARG("Dpy=0x%p Surface=0x%p rects=0x%p n_rects=%d",
                  Dpy, Surface, (void *)Rects, NumRects);
    gcmDUMP_API("${EGL eglSwapBuffersWithDamageEXT 0x%p 0x%p 0x%p 0x%X}",
                Dpy, Surface, (void *)Rects, NumRects);
    VEGL_TRACE_API(SwapBuffersWithDamageKHR)(Dpy, Surface, Rects, NumRects);

    result = veglSwapBuffers(Dpy, Surface, gcvNULL, &region);

    gcmFOOTER_ARG("%d", result);
    return result;
}


#if defined(ANDROID)
EGLAPI EGLBoolean EGLAPIENTRY
eglSetSwapRectangleVIV(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLint Left,
    EGLint Top,
    EGLint Width,
    EGLint Height
    )
{
    EGLBoolean result = EGL_FALSE;
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface draw;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x Left=%d Top=%d Width=%d Height=%d",
                  Dpy, Draw, Left, Top, Width, Height);
    gcmDUMP_API("${EGL eglSetSwapRectangleVIV 0x%08X 0x%08X 0x%08X 0x%08X "
                "0x%08X 0x%08X}",
                Dpy, Draw, Left, Top, Width, Height);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): eglSetSwapRectangleVIV failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    do
    {
        if (((gctUINT) (gctUINTPTR_T) Dpy) < 0x04)
        {
            /*
             * If bypass Android EGL wrapper (which means it does not
             * support EGL_ANDROID_get_render_buffer extension), EGLDisplay is
             * then an index integer value (normally 1).
             */
            if (thread->context == gcvNULL)
            {
                /* No current dpy/surface. */
                break;
            }

            /* Take dpy from current display. */
            dpy  = thread->context->display;

            if (dpy == gcvNULL)
            {
                thread->error = EGL_BAD_DISPLAY;
                break;
            }

            /* Take draw from current draw surface. */
            draw = thread->context->draw;

            if (draw == gcvNULL)
            {
                thread->error = EGL_BAD_SURFACE;
                break;
            }
        }
        else
        {
            /* Test for valid EGLDisplay structure. */
            dpy = veglGetDisplay(Dpy);
            if (dpy == gcvNULL)
            {
                /* Bad display. */
                veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
                gcmFOOTER_ARG("%d", EGL_FALSE);
                return EGL_FALSE;
            }

            /* Test if EGLDisplay structure has been initialized. */
            if (!dpy->initialized)
            {
                /* Not initialized. */
                thread->error = EGL_NOT_INITIALIZED;
                break;
            }

            /* Test for valid EGLSurface structure */
            draw = (VEGLSurface)veglGetResObj(dpy,
                                              (VEGLResObj*)&dpy->surfaceStack,
                                              (EGLResObj)Draw,
                                              EGL_SURFACE_SIGNATURE);

            if (draw == gcvNULL)
            {
                thread->error = EGL_BAD_SURFACE;
                break;
            }
        }

        draw->swapRect.x = Left;
        draw->swapRect.y = Top;
        draw->swapRect.width  = Width;
        draw->swapRect.height = Height;

        result = EGL_TRUE;
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("%d", result);
    return result;
}
#endif

/* EGL_KHR_partial_update */
EGLAPI EGLBoolean EGLAPIENTRY
eglSetDamageRegionKHR(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLint *Rects,
    EGLint NumRects
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    struct eglRegion *region;
    EGLint numRects;

    gcmHEADER_ARG("Dpy=0x%x Surface=0x%x rects=0x%x n_rects=%d", Dpy, Surface, Rects, NumRects);
    gcmDUMP_API("${EGL eglSetDamageRegionKHR 0x%08X 0x%08X 0x%08X 0x%08X}",
                Dpy, Surface, Rects, NumRects);

    VEGL_TRACE_API(SetDamageRegionKHR)(Dpy, Surface, Rects, NumRects);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    surface = VEGL_SURFACE(Surface);

    /*
     * * If <surface> is not a postable surface, an EGL_BAD_MATCH error is
     *   generated
     * * If <surface> is not the current draw surface for the calling thread, an
     *   EGL_BAD_MATCH error is generated
     * * If the value of EGL_SWAP_BEHAVIOR for <surface> is not
     *   EGL_BUFFER_DESTROYED, an EGL_BAD_MATCH error is generated
     */
    if ((surface->type != EGL_WINDOW_BIT) ||
        (thread->context == gcvNULL || thread->context->draw != surface) ||
        (surface->swapBehavior != EGL_BUFFER_DESTROYED))
    {
        veglSetEGLerror(thread,  EGL_BAD_MATCH);
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /*
     * * If eglSetDamageRegionKHR has already been called on <surface> since the
     *   most recent frame boundary, an EGL_BAD_ACCESS error is generated
     * * If the EGL_BUFFER_AGE_KHR attribute of <surface> has not been queried
     *   since the most recent frame boundary, an EGL_BAD_ACCESS error is generated
     */
    if (surface->damageValid || !surface->queriedAge)
    {
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    region = &surface->damage[surface->curDamage];

    numRects = (NumRects == 0) ? 1 : NumRects;
    _ReallocRegion(region, numRects);

    /*
     * If <n_rects> is zero, <rects> is ignored and the damage region is set to the
     * full dimensions of the surface.
     */
    if (NumRects == 0)
    {
        region->numRects = 1;

        region->rects[0] = 0;
        region->rects[1] = 0;
        region->rects[2] = (EGLint)surface->config.width;
        region->rects[3] = (EGLint)surface->config.height;
    }
    else
    {
        region->numRects = NumRects;
        gcoOS_MemCopy(region->rects, Rects, 4 * gcmSIZEOF(EGLint) * NumRects);
    }

    surface->curDamage   = (surface->curDamage + 1) % EGL_WORKER_COUNT;
    surface->damageValid = EGL_TRUE;

    /* Safe to reset queriedAge flag because damageValid flag is set. */
    surface->queriedAge  = EGL_FALSE;

    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglCopyBuffers(
    EGLDisplay Dpy,
    EGLSurface Surface,
    EGLNativePixmapType target
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLPlatform platform;
    EGLBoolean result = EGL_FALSE;
    VEGLPixmapInfo info = gcvNULL;
    gcoSURF pixmapSurface = gcvNULL;

    gcmHEADER_ARG("Dpy=0x%x Surface=0x%x target=%d", Dpy, Surface, target);
    gcmDUMP_API("${EGL eglCopyBuffers 0x%08X 0x%08X 0x%08X}",
                Dpy, Surface, target);
    VEGL_TRACE_API(CopyBuffers)(Dpy, Surface, target);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    platform = dpy->platform;

    do
    {
        gceSTATUS status      = gcvSTATUS_OK;
        VEGLSurface surface   = gcvNULL;
        gceHARDWARE_TYPE type = gcvHARDWARE_INVALID;

        /* Test if EGLDisplay structure has been initialized. */
        if (!dpy->initialized)
        {
            /* Not initialized. */
            veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
            result = EGL_FALSE;
            break;
        }

        /* Test for valid EGLSurface structure. */
        surface = (VEGLSurface)veglGetResObj(dpy,
                                             (VEGLResObj*)&dpy->surfaceStack,
                                             (EGLResObj)Surface,
                                             EGL_SURFACE_SIGNATURE);

        if (surface == gcvNULL)
        {
            /* Bad surface*/
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /* Test if surface is protected. */
        if (surface->protectedContent == EGL_TRUE)
        {
            veglSetEGLerror(thread, EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        /* Test if surface is locked. */
        if (surface->locked)
        {
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            break;
        }

        /* Flush the API. */
        _Flush(thread);

        if (!platform->connectPixmap(dpy, (void *) target, &info, &pixmapSurface))
        {
            veglSetEGLerror(thread, EGL_BAD_NATIVE_PIXMAP);
            result = EGL_FALSE;
            break;
        }

        /* Resolve the render target. */
        gcoHAL_GetHardwareType(gcvNULL, &type);
        (void) type;

#if gcdENABLE_VG
        if (type == gcvHARDWARE_VG)
        {
            status = gcoSURF_Copy(pixmapSurface, surface->renderTarget);
        }
        else
#endif
        {
#if gcdENABLE_3D
            gcsSURF_VIEW rtView = {surface->renderTarget, 0, 1};
            gcsSURF_VIEW pixmapView = {pixmapSurface, 0, 1};
            status = gcoSURF_ResolveRect(&rtView, &pixmapView, gcvNULL);
#endif
        }

        if (gcmIS_ERROR(status))
        {
            /* Bad surface. */
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            result = EGL_FALSE;
            break;
        }

        /* Stall the hardware. */
        status = gcoHAL_Commit(gcvNULL, gcvTRUE);

        if (gcmIS_ERROR(status))
        {
            /* Bad surface. */
            veglSetEGLerror(thread,  EGL_BAD_ACCESS);
            result = EGL_FALSE;
            break;
        }

        /* Wait native operations to write pixmap. */
        platform->syncToPixmap((void *) target, info);

        /* Success. */
        result = EGL_TRUE;
    }
    while (gcvFALSE);

    if (info != gcvNULL)
    {
        platform->disconnectPixmap(dpy, (void *) target, info);
        info = gcvNULL;
    }

    /* Success. */
    gcmFOOTER_ARG("%d", result);
    return result;
}

#if defined(ANDROID)

#if (ANDROID_SDK_VERSION < 17)

EGLAPI EGLClientBuffer EGLAPIENTRY
eglGetRenderBufferv0VIV(
    EGLDisplay Dpy,
    EGLSurface Draw
    )
{
    VEGLSurface draw;
    VEGLThreadData thread;
    VEGLDisplay dpy;
    EGLClientBuffer buffer = gcvNULL;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x", Dpy, Draw);
    gcmDUMP_API("${EGL eglGetRenderBufferv0VIV 0x%08X 0x%08X}", Dpy, Draw);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("0x%x08X", gcvNULL);
        return gcvNULL;
    }

    do
    {
        if (((gctUINT) (gctUINTPTR_T) Dpy) < 0x04)
        {
            /*
             * If bypass Android EGL wrapper (which means it does not
             * support EGL_ANDROID_get_render_buffer extension), EGLDisplay is
             * then an index integer value (normally 1).
             */
            if (thread->context == gcvNULL)
            {
                /* No current dpy/surface. */
                break;
            }

            /* Take dpy from current display. */
            dpy  = thread->context->display;

            if (dpy == gcvNULL)
            {
                veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
                break;
            }

            /* Take draw from current draw surface. */
            draw = thread->context->draw;

            if (draw == gcvNULL)
            {
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                break;
            }
        }
        else
        {

            /* Test for valid EGLDisplay structure. */
            dpy = veglGetDisplay(Dpy);
            if (dpy == gcvNULL)
            {
                /* Bad display. */
                veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
                gcmFOOTER_ARG("0x%x08X", gcvNULL);
                return gcvNULL;
            }


            /* Test if EGLDisplay structure has been initialized. */
            if (!dpy->initialized)
            {
                /* Not initialized. */
                veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
                break;
            }

            /* Test for valid EGLSurface structure */
            draw = (VEGLSurface)veglGetResObj(dpy,
                                              (VEGLResObj*)&dpy->surfaceStack,
                                              (EGLResObj)Draw,
                                              EGL_SURFACE_SIGNATURE);

            if (draw == gcvNULL)
            {
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                break;
            }
        }

        if (draw->backBuffer.context == gcvNULL)
        {
            /*
             *VIV: [TODO] Get window back buffer without bound.
             * This case is when 'draw' is not made current.
             */
            EGLBoolean result;

            result = dpy->platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer);

            if (!result)
            {
                gcmTRACE(
                    gcvLEVEL_ERROR,
                    "%s(%d): connect window failed",
                    __FUNCTION__, __LINE__
                    );
                break;
            }
        }

        if (draw->backBuffer.context != gcvNULL)
        {
            buffer = (EGLClientBuffer) draw->backBuffer.context;

            /* Set skipResolve flag. */
            draw->skipResolve = EGL_TRUE;

            /* Return back swap rectangle. */
            android_native_buffer_t * nativeBuffer =
                (android_native_buffer_t *) buffer;

            *((gctINT_PTR) &nativeBuffer->common.reserved[0]) =
                (draw->swapRect.x << 16) | draw->swapRect.y;

            *((gctINT_PTR) &nativeBuffer->common.reserved[1]) =
                (draw->swapRect.width << 16) | draw->swapRect.height;
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("0x%.08X", buffer);
    return buffer;
}

#   else

EGLAPI EGLClientBuffer EGLAPIENTRY
eglGetRenderBufferVIV(
    EGLClientBuffer Handle
    )
{
    VEGLDisplay dpy;
    VEGLSurface surface;
    VEGLSurface draw = gcvNULL;
    EGLClientBuffer buffer = gcvNULL;

    gcmHEADER_ARG("Handle=0x%x", Handle);
    gcmDUMP_API("${EGL eglGetRenderBuferVIV 0x%08X}", Handle);

    do
    {
        dpy = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);

        if (dpy == gcvNULL)
        {
            break;
        }

        VEGL_LOCK_DISPLAY_RESOURCE(dpy);

        /* Find EGLSurface which Handle belongs to. */
        surface = dpy->surfaceStack;

        if (surface->resObj.next == gcvNULL)
        {
            /* Only one surface. */
            draw = surface;
        }
        else
        {
            while (surface != gcvNULL)
            {
                gctINT i;

                if (!(surface->type & EGL_WINDOW_BIT) ||
                    (surface->winInfo == gcvNULL))
                {
                    surface = (VEGLSurface) surface->resObj.next;
                    continue;
                }

                if (dpy->platform->hasWindowBuffer(surface, Handle))
                {
                    /* Found. */
                    draw = surface;
                    break;
                }

                if (draw != gcvNULL)
                {
                    /* Found. */
                    break;
                }

                surface = (VEGLSurface) surface->resObj.next;
            }
        }

        VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);

        if (draw == gcvNULL)
        {
            /* EGLSurface not found. */
            break;
        }

        if (draw->backBuffer.context == gcvNULL)
        {
            /*
             *VIV: [TODO] Get window back buffer without bound.
             * This case is when 'draw' is not made current.
             */
            EGLBoolean result;

            result = dpy->platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer);

            if (!result)
            {
                gcmTRACE(
                    gcvLEVEL_ERROR,
                    "%s(%d): connect window failed",
                    __FUNCTION__, __LINE__
                    );
                break;
            }
        }

        buffer = (EGLClientBuffer) draw->backBuffer.context;

        if (buffer != gcvNULL)
        {
            /* Return back swap rectangle. */
            android_native_buffer_t * nativeBuffer =
                (android_native_buffer_t *) buffer;

            *((gctINT_PTR) &nativeBuffer->common.reserved[0]) =
                (draw->swapRect.x << 16) | draw->swapRect.y;

            *((gctINT_PTR) &nativeBuffer->common.reserved[1]) =
                (draw->swapRect.width << 16) | draw->swapRect.height;
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("0x%.08X", buffer);
    return buffer;
}


EGLAPI EGLBoolean EGLAPIENTRY
eglPostBufferVIV(
    EGLClientBuffer Buffer
    )
{
    VEGLDisplay dpy;
    VEGLSurface draw;
    EGLBoolean result = EGL_FALSE;

    gcmHEADER_ARG("Buffer=0x%x", Buffer);
    gcmDUMP_API("${EGL eglPostBufferVIV 0x%08X}", Buffer);

    do
    {
        gctBOOL flip;

        dpy = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);

        if (dpy == gcvNULL)
        {
            break;
        }

        VEGL_LOCK_DISPLAY_RESOURCE(dpy);

        /* Find the EGLSurface which Buffer belongs to. */
        draw = dpy->surfaceStack;

        while (draw != gcvNULL)
        {
            if (draw->backBuffer.context == (gctPOINTER) Buffer)
            {
                break;
            }

            draw = (VEGLSurface) draw->resObj.next;
        }
        VEGL_UNLOCK_DISPLAY_RESOURCE(dpy);

        if (draw != gcvNULL)
        {
            /*
             * Do not set queueBuffer fenceFd for framebuffer target here, to
             * reduce some resources.
             * HWComposer HAL has that fenceFd and can handle it there.
             */
            dpy->platform->postWindowBackBuffer(dpy, draw,
                                                &draw->backBuffer,
                                                &draw->clipRegion,
                                                &draw->damageHint);

            draw->backBuffer.context = gcvNULL;
            draw->backBuffer.surface = gcvNULL;

            if (draw->newSwapModel)
            {
                /* Get native window back buffer for next frame. */
                result = dpy->platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer);

                if (!result)
                {
                    gcmTRACE(
                        gcvLEVEL_ERROR,
                        "%s(%d): Get back buffer failed",
                        __FUNCTION__, __LINE__
                        );

                    break;
                }
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("0x%.08X", result);
    return result;
}

#   endif

#endif
