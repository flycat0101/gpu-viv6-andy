/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_egl_precomp.h>
#include <gc_hal_eglplatform.h>


/*
 * Number of temorary linear 'resolve' surfaces.
 * Need alloate those surfaces when can directly resolve into window back
 * buffers.
 */
#define NUM_TEMPORARY_RESOLVE_SURFACES      1

/*
 * Make sure GPU rendering and window back buffer displaying (may be by CPU)
 * are synchronized.
 * The idea is to wait until buffer is displayed before next time return back
 * to GPU rendering.
 *
 * TODO: But this will break frame skipping because skipped back buffer post
 * will cause infinite wait in veglGetWindowBackBuffer.
 */
#define SYNC_TEMPORARY_RESOLVE_SURFACES     0


/******************************************************************************/
/* Display. */

NativeDisplayType
veglGetDefaultDisplay(
    void
    )
{
    NativeDisplayType display = (NativeDisplayType) gcvNULL;

    gcoOS_GetDisplay((HALNativeDisplayType*) &display, gcvNULL);

    return display;
}

void
veglReleaseDefaultDisplay(
    IN NativeDisplayType Display
    )
{
    gcoOS_DestroyDisplay((HALNativeDisplayType) Display);
}

EGLBoolean
veglIsValidDisplay(
    IN NativeDisplayType Display
    )
{
    if (gcmIS_ERROR(gcoOS_IsValidDisplay((HALNativeDisplayType) Display)))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
veglInitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = gcoOS_InitLocalDisplayInfo((HALNativeDisplayType) Display->hdc,
                                        &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
veglDeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = gcoOS_DeinitLocalDisplayInfo((HALNativeDisplayType) Display->hdc,
                                          &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLint
veglGetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint visualId = 0;

    gcoOS_GetNativeVisualId((HALNativeDisplayType) Display->hdc, &visualId);
    return visualId;
}

/* Query of swap interval range. */
EGLBoolean
veglGetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Max,
    OUT EGLint * Min
    )
{
    gceSTATUS status;

    /* Get swap interval from HAL OS layer. */
    status = gcoOS_GetSwapInterval((HALNativeDisplayType) Display->hdc,
                                   Min, Max);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
veglSetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    )
{
    gceSTATUS status;

#ifdef EGL_API_WL
    /* Wayland specific. */
    status = gcoOS_SetSwapIntervalEx((HALNativeDisplayType) Display->hdc,
                                      Interval,
                                      Display->localInfo);
#else
    status = gcoOS_SetSwapInterval((HALNativeDisplayType) Display->hdc,
                                   Interval);
#endif

    if (status == gcvSTATUS_NOT_SUPPORTED)
    {
        /*
         * return true to maintain legacy behavior. If the feature is not there
         * we were ignoring it. And now we are ignoring it too.
         */
        return EGL_TRUE;
    }
    else if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */

typedef struct eglNativeBuffer * VEGLNativeBuffer;

struct eglNativeBuffer
{
    gctPOINTER          context;
    gcsPOINT            origin;
    gcoSURF             surface;

    /* QNX only. */
    gctPOINTER          logical;

    /* Buffer lock. */
    gctSIGNAL           lock;

    VEGLNativeBuffer    prev;
    VEGLNativeBuffer    next;
};

struct eglWindowInfo
{
    /*
     * Can directly access window memory?
     * True  for FBDEV, DFB, QNX, DDraw, etc.
     * False for GDI, X11, DRI, etc.
     *
     * As descripted in comments in veglBindWindow in header file, 4 conditions
     * for window back buffer:
     * If 'fbDirect' window memory: ('fbDirect' = 'True')
     *   1. Direct window back buffer surface ('wrapFB' = 'False')
     *   2. Wrapped surface ('wrapFB' = 'True')
     *   3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
     * Else:
     *   4. Temporary surface. ('fbDirect' = 'False')
     */
    EGLBoolean          fbDirect;

    /*
     * Wrap window back buffer as HAL surface object?
     * Invalid if 'fbDirect' is 'False'.
     */
    EGLBoolean          wrapFB;

    /* Window back buffer list, wrappers or temporary surface objects. */
    VEGLNativeBuffer    bufferList;
    gctPOINTER          bufferListMutex;

    /* Information obtained by gcoOS_GetDisplayInfoEx2. */
    gctPOINTER          logical;
    unsigned long       physical;
    gctINT              stride;
    gctUINT             width;
    gctUINT             height;
    gceSURF_FORMAT      format;
    gceSURF_TYPE        type;
    gctINT              bitsPerPixel;
    gctUINT             xresVirtual;
    gctUINT             yresVirtual;
    gctUINT             multiBuffer;
};



/*
 * Create wrappers or temporary surface object(s) for native window (conditions
 * 2, 3 and 4 mentioned above).
 *
 * o 2. Wrapped surface ('wrapFB' = 'True')
 * o 3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
 * o 4. Temporary surface. ('fbDirect' = 'False')
 */
static gceSTATUS
_CreateWindowBuffers(
    NativeWindowType Window,
    VEGLWindowInfo Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VEGLNativeBuffer buffer;

    if (Info->fbDirect)
    {
#ifdef __QNXNTO__
        /*
         * Special for QNX.
         * It will wrap surface objects when GetWindowBackBuffer.
         */
        if (0)
#else
        if (Info->wrapFB)
#endif
        {
            gctPOINTER pointer;
            gctUINT alignedHeight;
            gceSURF_TYPE baseType;
            gctUINT i;

            baseType = (gceSURF_TYPE) ((gctUINT32) Info->type & 0xFF);

            /* Lock. */
            gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

            alignedHeight = Info->yresVirtual / Info->multiBuffer;

            for (i = 0; i < Info->multiBuffer; i++)
            {
                /*
                 * TODO: Check wrapper limitations.
                 * Allocate temporary surface objects if can not wrap.
                 *
                 * Current logic follows former code without changes.
                 */
                gctUINT    offset;
                gctPOINTER logical;
                gctUINT    physical;

                /* Allocate native buffer object. */
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          sizeof (struct eglNativeBuffer),
                                          &pointer));

                gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
                buffer = pointer;

                /* Bytes offset to the buffer. */
                offset = (gctUINT) (Info->stride * alignedHeight * i);

                /* Calculate buffer addresses. */
                logical  = (gctUINT8_PTR) Info->logical + offset;
                physical = Info->physical + offset;

                /* Construct the wrapper. */
                gcmONERROR(gcoSURF_Construct(gcvNULL,
                                             Info->width,
                                             Info->height, 1,
                                             Info->type,
                                             Info->format,
                                             gcvPOOL_USER,
                                             &buffer->surface));

                /* Set the underlying framebuffer surface. */
                gcmONERROR(gcoSURF_SetBuffer(buffer->surface,
                                             Info->type,
                                             Info->format,
                                             Info->stride,
                                             logical,
                                             physical));

                /* For a new surface, clear it to get rid of noises. */
                gcoOS_ZeroMemory(logical, alignedHeight * Info->stride);

                gcmONERROR(gcoSURF_SetWindow(buffer->surface,
                                             0, 0,
                                             Info->width, Info->height));

                /* Initial lock for user-pool surface. */
                gcmONERROR(gcoSURF_Lock(buffer->surface, gcvNULL, gcvNULL));

                (void) baseType;

#if gcdENABLE_3D
                if (baseType == gcvSURF_RENDER_TARGET)
                {
                    /* Render target surface orientation is different. */
                    gcmVERIFY_OK(
                        gcoSURF_SetFlags(buffer->surface,
                                         gcvSURF_FLAG_CONTENT_YINVERTED,
                                         gcvTRUE));

                    if (!(Info->type & gcvSURF_NO_TILE_STATUS))
                    {
                        /* Append tile status to this user-pool rt. */
                        gcmVERIFY_OK(gcoSURF_AppendTileStatus(buffer->surface));
                    }
                }
#endif

                buffer->context  = gcvNULL;
                buffer->origin.x = 0;
                buffer->origin.y = alignedHeight * i;

                /* Add into buffer list. */
                if (Info->bufferList != gcvNULL)
                {
                    VEGLNativeBuffer prev = Info->bufferList->prev;

                    buffer->prev = prev;
                    buffer->next = Info->bufferList;

                    prev->next = buffer;
                    Info->bufferList->prev = buffer;
                }
                else
                {
                    buffer->prev = buffer->next = buffer;
                    Info->bufferList = buffer;
                }

                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): buffer[%d]: yoffset=%-4d physical=%x",
                         __FUNCTION__, __LINE__,
                         i, alignedHeight * i, physical);
            }

            gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
        }
        else
        {
            /*
             * gcoOS_GetDisplayBackbuffer must return gcoSURF objects directly
             * or it will wrap surface in GetWindowBackBuffer for QNX.
             */
            return gcvSTATUS_OK;
        }
    }
    else
    {
        /* Create temporary surface objects */
        gctINT i;
        gctPOINTER pointer;

        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        for (i = 0; i < NUM_TEMPORARY_RESOLVE_SURFACES; i++)
        {
            /* Allocate native buffer object. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      sizeof (struct eglNativeBuffer),
                                      &pointer));

            gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
            buffer = pointer;

            /* Construct temporary resolve surfaces. */
            gcmONERROR(gcoSURF_Construct(gcvNULL,
                                         Info->width,
                                         Info->height, 1,
                                         gcvSURF_BITMAP,
                                         Info->format,
                                         gcvPOOL_DEFAULT,
                                         &buffer->surface));

#if SYNC_TEMPORARY_RESOLVE_SURFACES
            /* Create the buffer lock. */
            gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &buffer->lock));

            /* Set initial 'unlocked' state. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
#endif

            /* Add into buffer list. */
            if (Info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer prev = Info->bufferList->prev;

                buffer->prev = prev;
                buffer->next = Info->bufferList;

                prev->next = buffer;
                Info->bufferList->prev = buffer;
            }
            else
            {
                buffer->prev = buffer->next = buffer;
                Info->bufferList = buffer;
            }

            gcmTRACE(gcvLEVEL_INFO,
                     "%s(%d): buffer[%d]: surface=%p",
                     __FUNCTION__, __LINE__,
                     i, buffer->surface);
        }

        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    return EGL_TRUE;

OnError:
    /* Error roll back. */
    if ((buffer = Info->bufferList) != gcvNULL)
    {
        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;
    }

    /* The buffer list mutex must be acquired. */
    gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return status;
}

static void
_FreeWindowBuffers(
    NativeWindowType Window,
    VEGLWindowInfo Info
    )
{
    if (Info->bufferList)
    {
        VEGLNativeBuffer buffer;

        /* Lock buffers. */
        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        /* Go through all buffers. */
        buffer = Info->bufferList;

        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;

        /* Unlock. */
        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
}

static
EGLBoolean
_QueryWindowInfo(
    IN VEGLDisplay Display,
    IN NativeWindowType Window,
    IN VEGLWindowInfo Info
    )
{
    gceSTATUS status;
    halDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = gcoOS_GetWindowInfoEx((HALNativeDisplayType) Display->hdc,
                                   (HALNativeWindowType) Window,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   &bitsPerPixel,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Initialize window geometry info. */
    Info->width        = width;
    Info->height       = height;
    Info->format       = format;
    Info->type         = type;
    Info->bitsPerPixel = bitsPerPixel;

    /* Get display information. */
    gcoOS_ZeroMemory(&dInfo, sizeof (halDISPLAY_INFO));

    status = gcoOS_GetDisplayInfoEx2((HALNativeDisplayType) Display->hdc,
                                     (HALNativeWindowType) Window,
                                     Display->localInfo,
                                     sizeof (halDISPLAY_INFO),
                                     &dInfo);

    if (gcmIS_ERROR(status))
    {
        Info->fbDirect     = EGL_FALSE;
        Info->logical      = gcvNULL;
        Info->physical     = gcvINVALID_ADDRESS;
        Info->stride       = 0;
        Info->wrapFB       = gcvFALSE;
        Info->multiBuffer  = 1;
    }
    else
    {
        Info->fbDirect     = EGL_TRUE;
        Info->logical      = dInfo.logical;
        Info->physical     = dInfo.physical;
        Info->stride       = dInfo.stride;
        Info->wrapFB       = dInfo.wrapFB;
#ifdef __QNXNTO__
        Info->multiBuffer  = 1;
#else
        Info->multiBuffer  = dInfo.multiBuffer > 1 ? dInfo.multiBuffer : 1;
#endif
    }

    /* Get virtual size. */
    status = gcoOS_GetDisplayVirtual((HALNativeDisplayType) Display->hdc,
                                     (gctINT_PTR) &Info->xresVirtual,
                                     (gctINT_PTR) &Info->yresVirtual);

    if (gcmIS_ERROR(status))
    {
        Info->xresVirtual = Info->width;
        Info->yresVirtual = Info->height;

        if (Info->multiBuffer > 1)
        {
            Info->multiBuffer = 1;
        }
    }

    return EGL_TRUE;
}

EGLBoolean
veglConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN NativeWindowType Window
    )
{
    gceSTATUS status;
    VEGLWindowInfo info = gcvNULL;
    NativeWindowType win = Window;
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != (NativeWindowType) gcvNULL);
    gcmASSERT(Surface->winInfo == gcvNULL);

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglWindowInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglWindowInfo));
    info = pointer;

    /* Query window information. */
    if (_QueryWindowInfo(Display, Window, info) == EGL_FALSE)
    {
        /* Bad native window. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create buffer mutex. */
    gcmONERROR(gcoOS_CreateMutex(gcvNULL, &info->bufferListMutex));

    /* Create window drawable? */
    gcoOS_CreateDrawable(Display->localInfo, (HALNativeWindowType) win);

    /* Create window buffers to represent window native bufers. */
    gcmONERROR(_CreateWindowBuffers(win, info));

    /* Save window info structure. */
    Surface->winInfo = info;
    return EGL_TRUE;

OnError:
    if (info)
    {
        if (info->bufferListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
            info->bufferListMutex = gcvNULL;
        }

        gcmOS_SAFE_FREE(gcvNULL, info);
        Surface->winInfo = gcvNULL;
    }

    return EGL_FALSE;
}

EGLBoolean
veglDisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    NativeWindowType win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Free native window buffers. */
    _FreeWindowBuffers(win, info);

    /* Delete the mutex. */
    gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
    info->bufferListMutex = gcvNULL;

    gcoOS_DestroyDrawable(Display->localInfo, (HALNativeWindowType) win);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, info);
    return EGL_TRUE;
}

#if gcdENABLE_RENDER_INTO_WINDOW && gcdENABLE_3D
/*
 * For apps in this list, EGL will use indirect rendering,
 * ie, disable no-resolve.
 */
static gcePATCH_ID indirectList[] =
{
    -1,
};

#endif


EGLBoolean
veglBindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    )
{
    gceSTATUS status;

    /* Get shortcut. */
    NativeWindowType win  = Surface->hwnd;
    VEGLWindowInfo info   = Surface->winInfo;
    /* Indirect rendering by default. */
    EGLint renderMode     = VEGL_INDIRECT_RENDERING;
    EGLBoolean winChanged = EGL_FALSE;
    gctINT width          = 0;
    gctINT height         = 0;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceSURF_TYPE type     = gcvSURF_UNKNOWN;

    status = gcoOS_GetWindowInfoEx((HALNativeDisplayType) Display->hdc,
                                   (HALNativeWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    (void) type;

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Check window resize. */
    if ((info->width  != (gctUINT) width) ||
        (info->height != (gctUINT) height) ||
        (info->format != format))
    {
        /* Window changed. */
        winChanged = EGL_TRUE;
    }

    if (Surface->openVG)
    {
        /* Check direct rendering support for 2D VG. */
        do
        {
            EGLBoolean formatSupported = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

#ifdef __QNXNTO__
            /* Can not support for QNX. */
            break;
#endif

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            switch (info->format)
            {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_A8B8G8R8:
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8R8G8B8:
            case gcvSURF_X8B8G8R8:
                if (Surface->config.alphaSize == 0)
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_R5G6B5:
                if ((Surface->config.redSize == 5) &&
                    (Surface->config.greenSize == 6) &&
                    (Surface->config.blueSize == 5) &&
                    (Surface->config.alphaSize == 0))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_A4R4G4B4:
            case gcvSURF_A4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4) &&
                    (Surface->config.alphaSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_X4R4G4B4:
            case gcvSURF_X4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            /* Should use direct rendering. */
            renderMode = VEGL_DIRECT_RENDERING;
        }
        while (gcvFALSE);

        if (info->type != gcvSURF_BITMAP)
        {
            /* Request linear buffer for hardware OpenVG. */
            status = gcoOS_SetWindowFormat(Display->hdc,
                                           win,
                                           gcvSURF_BITMAP,
                                           info->format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Update window buffer type. */
            info->type = gcvSURF_BITMAP;
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            /* Recreate window buffers. */
            _FreeWindowBuffers(win, info);
            gcmONERROR(_CreateWindowBuffers(win, info));
        }
    }
    else
    {
#if gcdENABLE_3D
#if gcdENABLE_RENDER_INTO_WINDOW
        /* 3D pipe. */
        do
        {
            /* Check if direct rendering is available. */
            EGLBoolean fcFill = EGL_FALSE;
            EGLBoolean formatSupported;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

#ifdef __QNXNTO__
            /* Can not support for QNX. */
            break;
#endif

            /* Get patch id. */
            gcoHAL_GetPatchID(gcvNULL, &patchId);

            for (i = 0; i < gcmCOUNTOF(indirectList); i++)
            {
                if (patchId == indirectList[i])
                {
                    indirect = EGL_TRUE;
                    break;
                }
            }

            if (indirect)
            {
                /* Forced indirect rendering. */
                break;
            }

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            /* Require fc-fill feature in hardware. */
            status = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER);

            if (status == gcvSTATUS_TRUE)
            {
                /* Has fc-fill feature. */
                fcFill = EGL_TRUE;
            }

            /* Check window format. */
            switch (info->format)
            {
            case gcvSURF_A8B8G8R8:
                format = gcvSURF_A8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8B8G8R8:
                format = gcvSURF_X8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A4R4G4B4:
            case gcvSURF_X4R4G4B4:
            case gcvSURF_R5G6B5:
                format = info->format;
                formatSupported = EGL_TRUE;
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            if (info->wrapFB)
            {
                if (!fcFill)
                {
                    break;
                }

                type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;

                if (gcmIS_ERROR(gcoOS_SetWindowFormat(Display->hdc,
                                                      win,
                                                      type, format)))
                {
                    /* Format or type not supported by window, stop. */
                    break;
                }

                /* Update window buffer format. */
                info->format = format;

                /*
                 * Will attach tile status for performance, and fc fill still
                 * can not support compression.
                 */
                info->type = type;

                /* Should use direct rendering and fc fill. */
                renderMode = VEGL_DIRECT_RENDERING_FCFILL;
                winChanged = EGL_TRUE;
                break;
            }
            else
            {
                /* Try many direct rendering levels here. */
                /* 1. The best, direct rendering with compression. */
                type = gcvSURF_RENDER_TARGET;

                if (gcmIS_SUCCESS(gcoOS_SetWindowFormat(Display->hdc,
                                                        win,
                                                        type,
                                                        format)))
                {
                    /* Update window buffer format,type. */
                    info->format = format;
                    info->type   = type;

                    /* Should use direct rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING;
                    winChanged = EGL_TRUE;
                    break;
                }

                /* 2. Second, with tile status, no compression. */
                type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;

                if (gcmIS_SUCCESS(gcoOS_SetWindowFormat(Display->hdc,
                                                        win,
                                                        type,
                                                        format)))
                {
                    /* Update window buffer format,type. */
                    info->format = format;
                    info->type   = type;

                    /* Should use direct rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
                    winChanged = EGL_TRUE;
                    break;
                }

                if (!fcFill)
                {
                    break;
                }

                /* 3. Third, with tile status, no compression, fc-filled. */
                type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;

                if (gcmIS_SUCCESS(gcoOS_SetWindowFormat(Display->hdc,
                                                        win,
                                                        type,
                                                        format)))
                {
                    /* Update window buffer format,type. */
                    info->format = format;
                    info->type   = gcvSURF_RENDER_TARGET_NO_COMPRESSION;

                    /* Should use direct rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING_FCFILL;
                    winChanged = EGL_TRUE;
                    break;
                }
            }
        }
        while (gcvFALSE);
#   endif

        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            (info->type != gcvSURF_BITMAP))
        {
            /* Use bitmap for indirect rendering. */
            if (gcmIS_ERROR(gcoOS_SetWindowFormat(Display->hdc,
                                                  win,
                                                  gcvSURF_BITMAP,
                                                  info->format)))
            {
                /* Can not support this window. */
                return EGL_FALSE;
            }

            /* Update window buffer type. */
            info->type = gcvSURF_BITMAP;
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            if ((renderMode == VEGL_DIRECT_RENDERING_FCFILL) &&
                (info->type == gcvSURF_RENDER_TARGET_NO_TILE_STATUS))
            {
                /* Special for FC-FILL mode: tile status is required. */
                info->type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
            }

            /* Recreate window buffers. */
            _FreeWindowBuffers(win, info);
            gcmONERROR(_CreateWindowBuffers(win, info));
        }


#endif
    }

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): winChanged=%d format=%d type=%x EGLConfig=%d%d%d%d "
             " renderMode=%d",
             __FUNCTION__, __LINE__,
             winChanged,
             info->format,
             info->type,
             Surface->config.redSize,
             Surface->config.greenSize,
             Surface->config.blueSize,
             Surface->config.alphaSize,
             renderMode);

    *RenderMode = renderMode;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

EGLBoolean
veglUnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return EGL_TRUE;
}

EGLBoolean
veglGetWindowSize(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
    gceSTATUS status;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE   type;

    /* Get shortcut. */
    NativeWindowType win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

    status = gcoOS_GetWindowInfoEx((HALNativeDisplayType) Display->hdc,
                                   (HALNativeWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    (void) format;
    (void) type;

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    *Width  = width;
    *Height = height;

    return EGL_TRUE;
}

EGLBoolean
veglGetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    gceSTATUS status;
    NativeWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    if (info->fbDirect)
    {
        gctUINT offset;

        BackBuffer->surface  = gcvNULL;
        BackBuffer->context  = gcvNULL;
        BackBuffer->origin.x = 0;
        BackBuffer->origin.y = 0;
        BackBuffer->flip     = gcvTRUE;

        /* Formerly veglGetDisplayBackBuffer. */
        status = gcoOS_GetDisplayBackbufferEx((HALNativeDisplayType)Display->hdc,
                                              (HALNativeWindowType)win,
                                               Display->localInfo,
                                               &BackBuffer->context,
                                               &BackBuffer->surface,
                                               &offset,
                                               &BackBuffer->origin.x,
                                               &BackBuffer->origin.y);

        if (gcmIS_ERROR(status))
        {
            /*
             * Fomerly, returns flip=false, then it will use first wrapper.
             */
            VEGLNativeBuffer buffer = info->bufferList;

            if (!buffer)
            {
                /* No wrappers? Bad native window. */
                return EGL_FALSE;
            }

            /* Copy out back buffer. */
            BackBuffer->context = buffer->context;
            BackBuffer->origin  = buffer->origin;
            BackBuffer->surface = buffer->surface;

#ifdef EGL_API_FB
            BackBuffer->flip    = gcvFALSE;
#endif

            /* Increase reference count. */
            /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

            return EGL_TRUE;
        }

        if (BackBuffer->surface)
        {
            /* Returned the surface directly. */
            return EGL_TRUE;
        }
        else
        {
#ifdef __QNXNTO__
            VEGLNativeBuffer buffer = gcvNULL;

            /*
             * QNX specific:
             * Find buffer with the same logical address.
             */
            halDISPLAY_INFO dInfo;
            gctUINT offset;
            gctPOINTER logical;
            gctUINT32 physical;

            gcmASSERT(info->wrapFB);

            gcoOS_ZeroMemory(&dInfo, sizeof (halDISPLAY_INFO));

            /* Get display information. */
            status = gcoOS_GetDisplayInfoEx((HALNativeDisplayType) Display->hdc,
                                            (HALNativeWindowType) win,
                                            sizeof (dInfo),
                                            &dInfo);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                return EGL_FALSE;
            }

            /* Get window information. */
            status = gcoOS_GetWindowInfo((HALNativeDisplayType) Display,
                                         (HALNativeWindowType) win,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         &offset);
            if (gcmIS_ERROR(status))
            {
                /* Error. */
                return EGL_FALSE;
            }

            if ((offset == ~0U) ||
                (dInfo.logical == gcvNULL) ||
                (dInfo.physical == gcvINVALID_ADDRESS))
            {
                /* No offset. */
                return EGL_FALSE;
            }

            /* Compute window addresses. */
            logical  = (gctUINT8_PTR) dInfo.logical + offset;
            physical = dInfo.physical + offset;

            gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

            if (info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer buf;

                buf = info->bufferList;

                do
                {
                    /* Loop the list. */
                    if (buf->logical == logical)
                    {
                        /* We found it. */
                        buffer = buf;
                        break;
                    }

                    buf = buf->next;
                }
                while (buf != info->bufferList);
            }

            gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

            if (buffer == gcvNULL)
            {
                /* Buffer is not saved. */
                gctPOINTER pointer;

                /* Allocate native buffer object. */
                if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                               sizeof (struct eglNativeBuffer),
                                               &pointer)))
                {
                    return EGL_FALSE;
                }

                gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
                buffer = pointer;

                /* Create wrapper or temporary surface object. */
                status = gcoSURF_Construct(gcvNULL,
                                           info->width,
                                           info->height, 1,
                                           gcvSURF_BITMAP,
                                           info->format,
                                           gcvPOOL_USER,
                                           &buffer->surface);

                if (gcmIS_ERROR(status))
                {
                    /* Failed to construct wrapper. */
                    gcmOS_SAFE_FREE(gcvNULL, buffer);
                    return EGL_FALSE;
                }

                /* Map the memory. */
                status = gcoSURF_MapUserSurface(buffer->surface,
                                                dInfo.stride,
                                                logical,
                                                physical);

                if (gcmIS_ERROR(status))
                {
                    gcmVERIFY_OK(gcoSURF_Destroy(buffer->surface));
                    gcmOS_SAFE_FREE(gcvNULL, buffer);
                    return EGL_FALSE;
                }

                /* New buffer. */
                buffer->logical  = logical;
                buffer->context  = gcvNULL;
                buffer->origin.x = 0;
                buffer->origin.y = 0;

                /* Add into buffer list. */
                gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);
                if (info->bufferList)
                {
                    VEGLNativeBuffer prev = info->bufferList->prev;

                    buffer->prev = prev;
                    buffer->next = info->bufferList;

                    prev->next = buffer;
                    info->bufferList->prev = buffer;
                }
                else
                {
                    buffer->prev = buffer->next = buffer;
                    info->bufferList = buffer;
                }

                gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);
            }

            /* Return the found surface. */
            BackBuffer->surface  = buffer->surface;
            BackBuffer->context  = gcvNULL;
            BackBuffer->origin.x = 0;
            BackBuffer->origin.y = 0;

            /* Increase reference count. */
            /* gcoSURF_ReferenceSurface(surface); */
            return EGL_TRUE;

#else /* !__QNXNTO__ */

            VEGLNativeBuffer buffer = gcvNULL;

            /* Non-QNX. */
            /* WrapFB or temporary surface, go through bufferList to find */
            gcmASSERT(info->wrapFB);

            if (info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer buf = info->bufferList;

                gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

                do
                {
                    if ((buf->context  == BackBuffer->context)  &&
                        (buf->origin.x == BackBuffer->origin.x) &&
                        (buf->origin.y == BackBuffer->origin.y))
                    {
                        /* Found. */
                        buffer = buf;
                        break;
                    }

                    buf = buf->next;
                }
                while (buffer != info->bufferList);

                gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);
            }

            if (buffer != gcvNULL)
            {
                /* Return the found surface. */
                BackBuffer->surface  = buffer->surface;
                BackBuffer->context  = buffer->context;
                BackBuffer->origin.x = buffer->origin.x;
                BackBuffer->origin.y = buffer->origin.y;

                /* Increase reference count. */
                /* gcoSURF_ReferenceSurface(BackBuffer->surface); */
                return EGL_TRUE;
            }
            else
            {
                /* Bad native window. */
                return EGL_FALSE;
            }
#endif
        }
    }
    else
    {
        /* Return the temorary surface object. */
        VEGLNativeBuffer buffer;

        gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

        buffer = info->bufferList;

        BackBuffer->surface  = buffer->surface;
        BackBuffer->context  = buffer;
        BackBuffer->origin.x = 0;
        BackBuffer->origin.y = 0;
        BackBuffer->flip     = gcvTRUE;

        info->bufferList = buffer->next;

        gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

        if (buffer->lock != gcvNULL)
        {
            /* Wait for buffer lock. */
            for (;;)
            {
                status = gcoOS_WaitSignal(gcvNULL, buffer->lock, 5000);

                if (status == gcvSTATUS_TIMEOUT)
                {
                    gcmPRINT("Wait for buffer lock timeout");
                    continue;
                }

                break;
            }

            /*
             * Set the buffer to 'locked' state.
             * It will be 'unlocked' when buffer posted to display.
             * This can make sure next time GetWindowBackBuffer, the buffer
             * is 'posted' before returns for GPU rendering.
             */
            gcoOS_Signal(gcvNULL, buffer->lock, gcvFALSE);
        }

        /* Increase reference count. */
        /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

        return EGL_TRUE;
    }
}

EGLBoolean
veglPostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN EGLint NumRects,
    IN EGLint Rects[]
    )
{
    NativeWindowType win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    (void) surface;

    if (info->fbDirect)
    {
#ifdef __QNXNTO__
        /* QNX. */
        status = gcoOS_DisplayBufferRegions((HALNativeDisplayType) Display->hdc,
                                            (HALNativeWindowType) win,
                                            NumRects,
                                            Rects);

#else
        /* Non-QNX. */
        surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

        status = gcoOS_SetDisplayVirtualEx((HALNativeDisplayType) Display->hdc,
                                           (HALNativeWindowType) win,
                                           BackBuffer->context,
                                           surface,
                                           0,
                                           BackBuffer->origin.x,
                                           BackBuffer->origin.y);
#endif

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }
    }
    else
    {
        VEGLNativeBuffer buffer;
        gctINT alignedWidth, alignedHeight;
        gceORIENTATION orientation;
        gceSURF_FORMAT format = gcvSURF_UNKNOWN;
        gcsSURF_FORMAT_INFO_PTR formatInfo;
        gctPOINTER memory[3] = {gcvNULL};
        gctINT i;

        /* Cast type. */
        buffer = (VEGLNativeBuffer) BackBuffer->context;

        /* Get aligned size. */
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(BackBuffer->surface,
                                            (gctUINT_PTR) &alignedWidth,
                                            (gctUINT_PTR) &alignedHeight,
                                            gcvNULL));

        gcmVERIFY_OK(gcoSURF_QueryOrientation(BackBuffer->surface, &orientation));

        if (orientation == gcvORIENTATION_BOTTOM_TOP)
        {
            alignedHeight = -alignedHeight;
        }

        /* Gather source information. */
        gcmVERIFY_OK(gcoSURF_GetFormat(BackBuffer->surface,
                                       gcvNULL,
                                       &format));

        /* Query format. */
        if (gcoSURF_QueryFormat(format, &formatInfo))
        {
            return EGL_FALSE;
        }

        /* Lock surface for memory. */
        if (gcoSURF_Lock(BackBuffer->surface, gcvNULL, memory))
        {
            return EGL_FALSE;
        }

        for (i = 0; i < NumRects; i++)
        {
            EGLint left   = Rects[i * 4 + 0];
            EGLint top    = Rects[i * 4 + 1];
            EGLint width  = Rects[i * 4 + 2];
            EGLint height = Rects[i * 4 + 3];

            /* Draw image. */
            status = gcoOS_DrawImageEx((HALNativeDisplayType) Display->hdc,
                                       (HALNativeWindowType) win,
                                       left, top, left + width, top + height,
                                       alignedWidth, alignedHeight,
                                       formatInfo->bitsPerPixel,
                                       memory[0],
                                       format);

            if (gcmIS_ERROR(status))
            {
                break;
            }
        }

        /* Unlock the surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(BackBuffer->surface, memory[0]));

        if (buffer->lock != gcvNULL)
        {
            /* The buffer is now posted. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
        }

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}

EGLBoolean
veglCancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    /* TODO: Not used currently because no direct rendering mode. */
    EGLint rect[4] =
    {
        0, 0,
        Surface->config.width,
        Surface->config.height
    };

    return veglPostWindowBackBuffer(Display, Surface, BackBuffer, 1, rect);
}

EGLBoolean
veglSynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return gcoOS_SynchronousFlip((HALNativeDisplayType)Display->hdc);
}

/******************************************************************************/
/* Pixmap. */

struct eglPixmapInfo
{
    /* Native pixmap geometry info in Vivante HAL. */
    gctINT              width;
    gctINT              height;
    gceSURF_FORMAT      format;
    gctINT              stride;
    gctINT              bitsPerPixel;

    /* Pixmap memory. */
    gctUINT8_PTR        data;

    /* Reference native display. */
    NativeDisplayType   hdc;

    /* Pixmap wrapper. */
    gcoSURF             wrapper;

    /* Shadow surface, exists when the wrapper is not resovable. */
    gcoSURF             shadow;
};

static void
_DoSyncFromPixmap(
    NativePixmapType Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(memory[0], Info->data, stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) Info->data;
            gctUINT8_PTR dest   = (gctUINT8_PTR) memory[0];
            gctINT shadowStride = stride;

            /* Get min stride. */
            stride = gcmMIN(shadowStride, Info->stride);

            for (y = 0; y < Info->height; y++)
            {
                /* Copy a scanline. */
                gcoOS_MemCopy(dest, source, stride);

                /* Advance to next line. */
                source += Info->stride;
                dest   += shadowStride;
            }
        }
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(gcoOS_CopyPixmapBits((HALNativeDisplayType) Info->hdc,
                                        (HALNativePixmapType) Pixmap,
                                        width, height,
                                        stride,
                                        Info->format,
                                        memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    /* Unlock. */
    if (memory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

static void
_DoSyncToPixmap(
    NativePixmapType Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data != gcvNULL)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(Info->data, memory[0], stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) memory[0];
            gctUINT8_PTR dest   = (gctUINT8_PTR) Info->data;
            gctINT shadowStride = stride;

            /* Get min stride. */
            stride = gcmMIN(shadowStride, Info->stride);

            for (y = 0; y < Info->height; y++)
            {
                /* Copy a scanline. */
                gcoOS_MemCopy(dest, source, stride);

                /* Advance to next line. */
                source += shadowStride;
                dest   += Info->stride;
            }
        }
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(gcoOS_DrawPixmap((HALNativeDisplayType) Info->hdc,
                                    (HALNativePixmapType) Pixmap,
                                    0, 0,
                                    Info->width,
                                    Info->height,
                                    width,
                                    height,
                                    Info->bitsPerPixel,
                                    memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    if (memory[0] != gcvNULL)
    {
        /* Unlock. */
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

EGLBoolean
veglMatchPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN struct eglConfig * Config
    )
{
    gceSTATUS status;
    gctINT width, height, bitsPerPixel;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = gcoOS_GetPixmapInfoEx((HALNativeDisplayType) Display->hdc,
                                   (HALNativePixmapType) Pixmap,
                                   &width,
                                   &height,
                                   &bitsPerPixel,
                                   gcvNULL, gcvNULL, &pixmapFormat);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    /* Check if format is matched. */
    switch (pixmapFormat)
    {
    case gcvSURF_R5G6B5:
        if ((Config->redSize   != 5)
        ||  (Config->greenSize != 6)
        ||  (Config->blueSize  != 5))
        {
            match = EGL_FALSE;
        }
        break;

    case gcvSURF_X8R8G8B8:
        if ((Config->redSize   != 8)
        ||  (Config->greenSize != 8)
        ||  (Config->blueSize  != 8)
        ||  (Config->alphaSize != 0))
        {
            match = EGL_FALSE;
        }
        break;

    default:
        break;
    }

    return match;
}

EGLBoolean
veglConnectPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL needShadow = gcvFALSE;
    gctINT pixmapWidth;
    gctINT pixmapHeight;
    gctINT pixmapStride = 0;
    gceSURF_FORMAT pixmapFormat;
    gctINT pixmapBpp;
    gctPOINTER pixmapBits = gcvNULL;
    gctUINT32 pixmapPhysical = gcvINVALID_ADDRESS;
    gcoSURF wrapper = gcvNULL;
    gcoSURF shadow = gcvNULL;
    gctPOINTER pointer;
    VEGLPixmapInfo info = gcvNULL;

    /* Query pixmap geometry info. */
    gcmONERROR(gcoOS_GetPixmapInfoEx((HALNativeDisplayType) Display->hdc,
                                     (HALNativePixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = gcoOS_GetPixmapInfo((HALNativeDisplayType) Display->hdc,
                                 (HALNativePixmapType) Pixmap,
                                 gcvNULL,
                                 gcvNULL,
                                 gcvNULL,
                                 &pixmapStride,
                                 &pixmapBits);

    do
    {
        if (gcmIS_ERROR(status) || !pixmapBits)
        {
            /* Can not wrap as surface object. */
            needShadow = gcvTRUE;
            break;
        }

#ifdef EGL_API_DRI
        pixmapPhysical = (gctUINT32) pixmapBits;
        pixmapBits     = gcvNULL;

        /* Query pixmap bits. */
        status = gcoOS_GetPixmapInfo((HALNativeDisplayType) Display->hdc,
                                     (HALNativePixmapType) Pixmap,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapBpp,
                                     gcvNULL,
                                     &pixmapBits);

        if (gcmIS_ERROR(status))
        {
            needShadow = gcvTRUE;
            break;
        }
#endif

        if (((gctUINTPTR_T) pixmapBits) & 0x3F)
        {
            needShadow = gcvTRUE;
            break;
        }

        if ((pixmapStride * 8 / pixmapBpp) < 16)
        {
            /* Too small in width. */
            needShadow = gcvTRUE;
            break;
        }


        /* Height needs to be 4 aligned or vstride is large enough. */
        if (pixmapHeight & 3)
        {
            /*
             * Not enough memory in height.
             * Resolve may exceeds the buffer and overwrite other memory.
             */
            needShadow = gcvTRUE;
            break;
        }
    }
    while (gcvFALSE);

    do
    {
        if (!pixmapBits)
        {
            /* No pixmap wrapper. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Construct pixmap wrapper. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmapWidth,
                              pixmapHeight,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_USER,
                              &wrapper));

        /* Set pixels. */
        status = gcoSURF_SetBuffer(wrapper,
                                   gcvSURF_BITMAP,
                                   pixmapFormat,
                                   pixmapStride,
                                   pixmapBits,
                                   pixmapPhysical);

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Do the wrap. */
        status = gcoSURF_SetWindow(wrapper,
                                   0, 0,
                                   pixmapWidth,
                                   pixmapHeight);

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Initial lock for user-pool surface. */
        status = gcoSURF_Lock(wrapper, gcvNULL, gcvNULL);
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status) && (wrapper != gcvNULL))
    {
        /* Failed to wrap as surface object. */
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
        wrapper = gcvFALSE;

        /* Shadow required and format must be supported. */
        needShadow = gcvTRUE;
    }

    if (needShadow)
    {
        /* Construct the shadow surface. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmapWidth,
                              pixmapHeight,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_DEFAULT,
                              &shadow));
    }

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglPixmapInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglPixmapInfo));
    info = pointer;

    /* Save pixmap info. */
    info->width        = pixmapWidth;
    info->height       = pixmapHeight;
    info->format       = pixmapFormat;
    info->stride       = pixmapStride;
    info->bitsPerPixel = pixmapBpp;
    info->data         = pixmapBits;
    info->hdc          = Display->hdc;
    info->wrapper      = wrapper;
    info->shadow       = shadow;

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p wrapper=%p shadow=%p",
             __FUNCTION__, __LINE__, Display, Pixmap, wrapper, shadow);

    /* Output. */
    *Info    = info;
    *Surface = (shadow != gcvNULL) ? shadow : wrapper;

    return EGL_TRUE;

OnError:
    if (wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
    }

    if (shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(shadow));
    }

    if (info != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, info);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return EGL_FALSE;
}

EGLBoolean
veglDisconnectPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    /* Free pixmap wrapper. */
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p",
             __FUNCTION__, __LINE__, Display, Pixmap);

    if (Info->wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->wrapper));
        Info->wrapper = gcvNULL;
    }

    if (Info->shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->shadow));
        Info->shadow = gcvNULL;
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, Info);
    return EGL_TRUE;
}

EGLBoolean
veglGetPixmapSize(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
    gceSTATUS status;
    gctINT bitsPerPixel;
    gceSURF_FORMAT format;
    gctINT width, height;

    /* Query pixmap info again. */
    gcmONERROR(
        gcoOS_GetPixmapInfoEx((HALNativeDisplayType) Display->hdc,
                              (HALNativePixmapType) Pixmap,
                              &width,
                              &height,
                              &bitsPerPixel,
                              gcvNULL,
                              gcvNULL,
                              &format));

    (void) bitsPerPixel;
    (void) format;

    gcmASSERT(width  == Info->width);
    gcmASSERT(height == Info->height);

    *Width  = width;
    *Height = height;

    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

EGLBoolean
veglSyncFromPixmap(
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncFromPixmap(Pixmap, Info);
    }
    else
    {
        gcmVERIFY_OK(
            gcoSURF_SetWindow(Info->wrapper,
                              0, 0,
                              Info->width,
                              Info->height));

        /* Initial lock for user-pool surface. */
        gcmVERIFY_OK(gcoSURF_Lock(Info->wrapper, gcvNULL, gcvNULL));
    }

    return EGL_TRUE;
}

EGLBoolean
veglSyncToPixmap(
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncToPixmap(Pixmap, Info);
    }

    return EGL_TRUE;
}

/******************************************************************************/

#if defined(EGL_API_WL)
struct wl_egl_window *
wl_egl_window_create(struct wl_surface *surface, int width, int height)
{
    struct wl_egl_window* window = gcvNULL;
    gctUINT i;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(surface);

    do
    {
        gcsSURF_FORMAT_INFO_PTR renderTargetInfo[2];
        int typeChanged = 0;
        gceSURF_FORMAT resolveFormat = gcvSURF_UNKNOWN;
        gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
        gcmONERROR(
            gcoOS_AllocateMemory(
                gcvNULL,
                sizeof *window,
                (gctPOINTER) &window
            ));

        gcoOS_ZeroMemory(
                window,
                sizeof *window
             );

        window->surface     = surface;
        window->info.dx     = 0;
        window->info.dy     = 0;
        window->info.width  = width;
        window->info.height = height;
        /* Set the window surface format save as the config requested */
        /* window->info.format = (gceSURF_FORMAT) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO); */
        window->info.format = gcvSURF_A8R8G8B8;

        gcmONERROR(
            gcoSURF_QueryFormat(
                window->info.format,
                renderTargetInfo
            ));

        window->info.bpp = renderTargetInfo[0]->bitsPerPixel;
        /* Query current hardware type. */
        gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        if(currentType == gcvHARDWARE_VG)
        {
            /* VG355 cannot support gcoTEXTURE_GetClosestFormat */
            gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
            typeChanged = 1;
        }
        gcmONERROR(
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        window->info.format,
                                        &resolveFormat));
        window->info.format = resolveFormat;

        if(typeChanged)
        {
            gcoHAL_SetHardwareType(gcvNULL, currentType);
        }

        for (i=0; i<WL_EGL_NUM_BACKBUFFERS ; i++)
        {
            gcmONERROR(
                gcoSURF_Construct(
                                gcvNULL,
                                width,
                                height,
                                1,
                                /*gcvSURF_TEXTURE*/gcvSURF_BITMAP,
                                resolveFormat,
                                gcvPOOL_DEFAULT,
                                &window->backbuffers[i].info.surface
                                ));
/*
            gcmONERROR(
                gcoSURF_SetOrientation(
                                    window->backbuffers[i].info.surface,
                                    gcvORIENTATION_BOTTOM_TOP));
*/
            gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i].info.surface,
                    gcvNULL,
                    gcvNULL
                    ));

            gcmONERROR(
                gcoSURF_GetAlignedSize(
                    window->backbuffers[i].info.surface,
                    gcvNULL,
                    gcvNULL,
                    &window->backbuffers[i].info.stride
                    ));

            gcmONERROR(
               gcoSURF_QueryVidMemNode(
                    window->backbuffers[i].info.surface,
                    (gctUINT32 *)&window->backbuffers[i].info.node,
                    &window->backbuffers[i].info.pool,
                    &window->backbuffers[i].info.bytes
                    ));

            gcmONERROR(
                gcoHAL_NameVideoMemory((gctUINT32)window->backbuffers[i].info.node,
                                       (gctUINT32 *)&window->backbuffers[i].info.node));

            window->backbuffers[i].info.width = window->info.width;
            window->backbuffers[i].info.height = window->info.height;
            window->backbuffers[i].info.format = resolveFormat;
            window->backbuffers[i].info.invalidate = gcvTRUE;
            window->backbuffers[i].info.locked = gcvFALSE;
            window->frame_callback = gcvNULL;
            gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p): width=%d, height=%d, format=%d, stride=%d, node=%p, pool=%d, bytes=%d, calc=%d",
                        i, window->backbuffers[i].info.surface,
                        width, height,
                        window->backbuffers[i].info.format,
                        window->backbuffers[i].info.stride,
                        window->backbuffers[i].info.node,
                        window->backbuffers[i].info.pool,
                        window->backbuffers[i].info.bytes,
                        window->backbuffers[i].info.stride*width
                        );
        }
    }
    while (gcvFALSE);

    return window;

OnError:
    wl_egl_window_destroy(window);
    return gcvNULL;
}

void
wl_egl_window_destroy(struct wl_egl_window *window)
{
    gctUINT i;

    if (window != gcvNULL)
    {
        /* Make sure the last rendering completed */
        gcoHAL_Commit(gcvNULL, gcvTRUE);
        for (i=0; i<WL_EGL_NUM_BACKBUFFERS ; i++)
        {
            if (window->backbuffers[i].info.surface != gcvNULL)
            {
                gcoSURF_Unlock(
                    window->backbuffers[i].info.surface,
                    gcvNULL
                    );

                gcoSURF_Destroy(
                    window->backbuffers[i].info.surface
                    );

                gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p) destroyed", i, window->backbuffers[i].info.surface);
            }
            if (window->backbuffers[i].wl_buffer != gcvNULL)
            {
                wl_buffer_destroy(window->backbuffers[i].wl_buffer);
            }
        }

        gcoOS_FreeMemory(gcvNULL, window);
    }
}

void
wl_egl_window_resize(struct wl_egl_window *window, int width, int height, int dx, int dy)
{
    gctUINT i;
    gceSTATUS status = gcvSTATUS_OK;
    gcmASSERT(window);

    window->info.dx = dx;
    window->info.dy = dy;

    /* Nothing to do if window size if same. */
    if(window->info.width == width && window->info.height == height)
    {
        return;
    }

    window->info.width  = width;
    window->info.height = height;

    for (i=0; i<WL_EGL_NUM_BACKBUFFERS ; i++)
    {
        gceSURF_FORMAT resolveFormat = window->info.format;

        if (window->backbuffers[i].info.surface != gcvNULL)
        {
            gcoSURF_Unlock(
                window->backbuffers[i].info.surface,
                gcvNULL
            );

            gcoSURF_Destroy(
                window->backbuffers[i].info.surface
            );

            gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p) destroyed", i, window->backbuffers[i].info.surface);
        }

        gcmONERROR(
                gcoSURF_Construct(
                gcvNULL,
                width,
                height,
                1,
                /*gcvSURF_TEXTURE*/gcvSURF_BITMAP,
                resolveFormat,
                gcvPOOL_DEFAULT,
                &window->backbuffers[i].info.surface
                ));
/*
        gcmONERROR(
                gcoSURF_SetOrientation(
                window->backbuffers[i].info.surface,
                gcvORIENTATION_BOTTOM_TOP));
*/
        gcmONERROR(
                gcoSURF_Lock(
                window->backbuffers[i].info.surface,
                gcvNULL,
                gcvNULL
                ));

        gcmONERROR(
                gcoSURF_GetAlignedSize(
                window->backbuffers[i].info.surface,
                gcvNULL,
                gcvNULL,
                &window->backbuffers[i].info.stride
                ));

        gcmONERROR(
                gcoSURF_QueryVidMemNode(
                window->backbuffers[i].info.surface,
                (gctUINT32*) &window->backbuffers[i].info.node,
                &window->backbuffers[i].info.pool,
                &window->backbuffers[i].info.bytes
                ));

        gcmONERROR(gcoHAL_NameVideoMemory((gctUINT32)window->backbuffers[i].info.node, (gctUINT32 *)&window->backbuffers[i].info.node));

        window->backbuffers[i].info.width = window->info.width;
        window->backbuffers[i].info.height = window->info.height;
        window->backbuffers[i].info.format = resolveFormat;
        window->backbuffers[i].info.invalidate = gcvTRUE;

        gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p): x=%d, y=%d, width=%d, height=%d, format=%d, stride=%d, node=%p, pool=%d, bytes=%d, calc=%d",
                i, window->backbuffers[i].info.surface,
                dx, dy, width, height,
                window->backbuffers[i].info.format,
                window->backbuffers[i].info.stride,
                window->backbuffers[i].info.node,
                window->backbuffers[i].info.pool,
                window->backbuffers[i].info.bytes,
                window->backbuffers[i].info.stride*width
                );
    }
OnError:
    /* Nothing to do; */
    return;
}
void wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,int *width, int *height)
{
    gcmASSERT(egl_window);

    if (width)
            *width = egl_window->info.attached_width;
    if (height)
            *height = egl_window->info.attached_height;
}

#endif

/******************************************************************************/

#if defined(LINUX) && defined(EGL_API_FB) && !defined(__APPLE__)

NativeDisplayType
fbGetDisplay(
    gctPOINTER context
    )
{
    NativeDisplayType display = gcvNULL;
    gcoOS_GetDisplay((HALNativeDisplayType*)(&display), context);
    return display;
}

EGLNativeDisplayType
fbGetDisplayByIndex(
    IN gctINT DisplayIndex
    )
{
    NativeDisplayType display = gcvNULL;
    gcoOS_GetDisplayByIndex(DisplayIndex, (HALNativeDisplayType*)(&display), gcvNULL);
    return display;
}

void
fbGetDisplayGeometry(
    IN NativeDisplayType Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    gcoOS_GetDisplayInfo((HALNativeDisplayType)Display, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetDisplayInfo(
    IN NativeDisplayType Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT unsigned long *Physical,
    OUT gctINT_PTR Stride,
    OUT gctINT_PTR BitsPerPixel
    )
{
    gcoOS_GetDisplayInfo((HALNativeDisplayType)Display, Width, Height,(gctSIZE_T_PTR)Physical, Stride, BitsPerPixel);
}

void
fbDestroyDisplay(
    IN NativeDisplayType Display
    )
{
    gcoOS_DestroyDisplay((HALNativeDisplayType)Display);
}

NativeWindowType
fbCreateWindow(
    IN NativeDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height
    )
{
    NativeWindowType Window = gcvNULL;
    gcoOS_CreateWindow((HALNativeDisplayType)Display, X, Y, Width, Height, (HALNativeWindowType*)(&Window));
    return Window;
}

void
fbGetWindowGeometry(
    IN NativeWindowType Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    gcoOS_GetWindowInfo(gcvNULL, (HALNativeWindowType)Window, X, Y, Width, Height, gcvNULL, gcvNULL);
}

void
fbGetWindowInfo(
    IN NativeWindowType Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctUINT_PTR Offset
    )
{
    gcoOS_GetWindowInfo(gcvNULL, (HALNativeWindowType)Window, X, Y, Width, Height, BitsPerPixel, Offset);
}

void
fbDestroyWindow(
    IN NativeWindowType Window
    )
{
    gcoOS_DestroyWindow(gcvNULL, (HALNativeWindowType)Window);
}

NativePixmapType
fbCreatePixmap(
    IN NativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height
    )
{
    NativePixmapType Pixmap = gcvNULL;
    gcoOS_CreatePixmap((HALNativeDisplayType)Display, Width, Height, 32, (HALNativePixmapType*)(&Pixmap));
    return Pixmap;
}

NativePixmapType
fbCreatePixmapWithBpp(
    IN NativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel
    )
{
    NativePixmapType Pixmap = gcvNULL;
    gcoOS_CreatePixmap((HALNativeDisplayType)Display, Width, Height, BitsPerPixel, (HALNativePixmapType*)(&Pixmap));
    return Pixmap;
}

void
fbGetPixmapGeometry(
    IN NativePixmapType Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    gcoOS_GetPixmapInfo(gcvNULL, (HALNativePixmapType)Pixmap, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetPixmapInfo(
    IN NativePixmapType Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctINT_PTR Stride,
    OUT gctPOINTER * Bits
    )
{
    gcoOS_GetPixmapInfo(gcvNULL, (HALNativePixmapType)Pixmap, Width, Height, BitsPerPixel, Stride, Bits);
}

void
fbDestroyPixmap(
    IN NativePixmapType Pixmap
    )
{
    gcoOS_DestroyPixmap(gcvNULL, (HALNativePixmapType)Pixmap);
}

#endif

/******************************************************************************/

#if defined(LINUX) && defined(EGL_API_DFB) && !defined(__APPLE__)
NativeDisplayType
dfbGetDisplay(
    gctPOINTER context
    )
{
    NativeDisplayType display = gcvNULL;
    gcoOS_GetDisplay((HALNativeDisplayType*)(&display), context);
    return display;
}

void
dfbDestroyDisplay(
    IN NativeDisplayType Display
    )
{
    gcoOS_DestroyDisplay((HALNativeDisplayType)Display);
}

NativeWindowType
dfbCreateWindow(
    IN NativeDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height
    )
{
    NativeWindowType Window = gcvNULL;
    gcoOS_CreateWindow((HALNativeDisplayType)Display, X, Y, Width, Height, (HALNativeWindowType*)(&Window));
    return Window;
}

void
dfbDestroyWindow(
    IN NativeWindowType Window
    )
{
    gcoOS_DestroyWindow(gcvNULL, (HALNativeWindowType)Window);
}

NativePixmapType
dfbCreatePixmap(
    IN NativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height
    )
{
    NativePixmapType Pixmap = gcvNULL;
    gcoOS_CreatePixmap((HALNativeDisplayType)Display, Width, Height, 32, (HALNativePixmapType*)(&Pixmap));
    return Pixmap;
}

NativePixmapType
dfbCreatePixmapWithBpp(
    IN NativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel
    )
{
    NativePixmapType Pixmap = gcvNULL;
    gcoOS_CreatePixmap((HALNativeDisplayType)Display, Width, Height, BitsPerPixel, (HALNativePixmapType*)(&Pixmap));
    return Pixmap;
}

void
dfbGetPixmapInfo(
    IN NativePixmapType Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctINT_PTR Stride,
    OUT gctPOINTER * Bits
    )
{
    gcoOS_GetPixmapInfo(gcvNULL, (HALNativePixmapType)Pixmap, Width, Height, BitsPerPixel, Stride, Bits);
}

void
dfbDestroyPixmap(
    IN NativePixmapType Pixmap
    )
{
    gcoOS_DestroyPixmap(gcvNULL, (HALNativePixmapType)Pixmap);
}
#endif

