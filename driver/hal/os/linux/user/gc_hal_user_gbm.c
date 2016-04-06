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


#if gcdENABLE_3D || gcdENABLE_VG
#ifdef __GBM__

#include "gc_hal_user_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <xf86drm.h>
#include <gbm.h>

#include "gc_hal_eglplatform.h"


#define DEFAULT_DRM_DEV_ID              (1)


#define _GC_OBJ_ZONE    gcvZONE_OS

struct
{
    int gbmFormat;
    int halFormat;
}
formatTable[] =
{
    {GBM_FORMAT_XRGB4444, gcvSURF_X4R4G4B4},
    {GBM_FORMAT_XBGR4444, gcvSURF_X4B4G4R4},
    /* {GBM_FORMAT_RGBX4444, gcvSURF_R4G4B4X4}, */
    /* {GBM_FORMAT_BGRX4444, gcvSURF_B4G4R4X4}, */

    {GBM_FORMAT_ARGB4444, gcvSURF_A4R4G4B4},
    {GBM_FORMAT_ABGR4444, gcvSURF_A4B4G4R4},
    /* {GBM_FORMAT_RGBA4444, gcvSURF_R4G4B4A4}, */
    /* {GBM_FORMAT_BGRA4444, gcvSURF_B4G4R4A4}, */

    {GBM_FORMAT_XRGB1555, gcvSURF_X1R5G5B5},
    {GBM_FORMAT_XBGR1555, gcvSURF_X1B5G5R5},
    /* {GBM_FORMAT_RGBX5551, gcvSURF_R5G5B5X1}, */
    /* {GBM_FORMAT_BGRX5551, gcvSURF_B5G5R5X1}, */

    {GBM_FORMAT_ARGB1555, gcvSURF_A1R5G5B5},
    {GBM_FORMAT_ABGR1555, gcvSURF_A1B5G5R5},
    /* {GBM_FORMAT_RGBA5551, gcvSURF_R5G5B5A1}, */
    /* {GBM_FORMAT_BGRA5551, gcvSURF_B5G5R5A1}, */

    {GBM_FORMAT_RGB565,   gcvSURF_R5G6B5  },
    {GBM_FORMAT_BGR565,   gcvSURF_B5G6R5  },

    /* {GBM_FORMAT_RGB888,   gcvSURF_R8G8B8  }, */
    /* {GBM_FORMAT_BGR888,   gcvSURF_B8G8R8  }, */

    {GBM_FORMAT_XRGB8888, gcvSURF_X8R8G8B8},
    {GBM_FORMAT_XBGR8888, gcvSURF_X8B8G8R8},
    /* {GBM_FORMAT_RGBX8888, gcvSURF_R8G8B8X8}, */
    /* {GBM_FORMAT_BGRX8888, gcvSURF_B8G8R8X8}, */

    {GBM_FORMAT_ARGB8888, gcvSURF_A8R8G8B8},
    {GBM_FORMAT_ABGR8888, gcvSURF_A8B8G8R8},
    /* {GBM_FORMAT_RGBA8888, gcvSURF_R8G8B8A8}, */
    /* {GBM_FORMAT_BGRA8888, gcvSURF_B8G8R8A8}, */

    {GBM_FORMAT_XRGB2101010, gcvSURF_X2R10G10B10},
    {GBM_FORMAT_XBGR2101010, gcvSURF_X2B10G10R10},
    /* {GBM_FORMAT_RGBX1010102, gcvSURF_R10G10B10X2}, */
    /* {GBM_FORMAT_BGRX1010102, gcvSURF_B10G10R10X2}, */

    {GBM_FORMAT_ARGB2101010, gcvSURF_A2R10G10B10},
    {GBM_FORMAT_ABGR2101010, gcvSURF_A2B10G10R10},
    /* {GBM_FORMAT_RGBA1010102, gcvSURF_R10G10B10A2}, */
    /* {GBM_FORMAT_BGRA1010102, gcvSURF_B10G10R10A2}, */

    {GBM_FORMAT_YUYV,     gcvSURF_YUY2},
    {GBM_FORMAT_YVYU,     gcvSURF_YVYU},
    {GBM_FORMAT_UYVY,     gcvSURF_UYVY},
    {GBM_FORMAT_VYUY,     gcvSURF_VYUY},

    /* {GBM_FORMAT_AYUV,     gcvSURF_AYUV}, */

    {GBM_FORMAT_NV12,     gcvSURF_NV12},
    {GBM_FORMAT_NV21,     gcvSURF_NV21},
    {GBM_FORMAT_NV16,     gcvSURF_NV16},
    {GBM_FORMAT_NV61,     gcvSURF_NV61},

    {GBM_FORMAT_YUV420,   gcvSURF_I420},
    {GBM_FORMAT_YVU420,   gcvSURF_YV12},
};

struct _GBMDisplay
{
    int index;
    int drmCard;
    struct gbm_device * gbmDev;
    gceTILING tiling;

    int swapInterval;

    /* Pointer to next. */
    struct _GBMDisplay * next;
};

static struct _GBMDisplay *displayStack = gcvNULL;
static pthread_mutex_t displayMutex;

static void
_FreeDisplays(
    void
    )
{
    pthread_mutex_lock(&displayMutex);

    while (displayStack != NULL)
    {
        struct _GBMDisplay* display = displayStack;
        displayStack = display->next;

        if (display->gbmDev != NULL)
        {
            gbm_device_destroy(display->gbmDev);
            display->gbmDev = NULL;
        }

        if (display->drmCard >= 0)
        {
            close(display->drmCard);
            display->drmCard = -1;
        }

        free(display);
    }

    pthread_mutex_unlock(&displayMutex);
}

static struct _GBMDisplay *
_FindDisplayLocked(
    HALNativeDisplayType Display
    )
{
    struct _GBMDisplay* display;

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display->gbmDev == Display)
        {
            /* Found display. */
            return display;
        }
    }

    return NULL;
}

static struct _GBMDisplay *
_FindDisplay(
    HALNativeDisplayType Display
    )
{
    struct _GBMDisplay* display;

    pthread_mutex_lock(&displayMutex);

    display = _FindDisplayLocked(Display);

    pthread_mutex_unlock(&displayMutex);

    return display;
}

static void
_SignalHandler(
    int signo
    )
{
    gcoOS_FreeEGLLibrary(gcvNULL);
    _FreeDisplays();

    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    raise(signo);
}

static void
_HookExit(
    void
    )
{
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    gcoOS_FreeEGLLibrary(gcvNULL);
    _FreeDisplays();
}

static pthread_once_t onceControl = PTHREAD_ONCE_INIT;

static void
_OnceInit(
    void
    )
{
    pthread_mutexattr_t mta;

    /* Register atexit callback. */
    atexit(_HookExit);

    /* Register signal handler. */
    signal(SIGINT,  _SignalHandler);
    signal(SIGQUIT, _SignalHandler);
    signal(SIGTERM, _SignalHandler);

    /* Init mutex attribute. */
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    /* Set display mutex as recursive. */
    pthread_mutex_init(&displayMutex, &mta);
}

/*******************************************************************************
** Display. ********************************************************************
*/

static
gceSTATUS
_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT HALNativeDisplayType * Display,
    IN gctPOINTER Context
    )
{
    char drmCardPath[256];
    gceSTATUS status = gcvSTATUS_OUT_OF_RESOURCES;
    struct _GBMDisplay * display = NULL;

    gcmHEADER_ARG("DisplayIndex=%d", DisplayIndex);

    /* Init global variables. */
    pthread_once(&onceControl, _OnceInit);

    /* Lock display stack. */
    pthread_mutex_lock(&displayMutex);

    do
    {
        if (DisplayIndex < 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        for (display = displayStack; display != NULL; display = display->next)
        {
            if (display->index == DisplayIndex)
            {
                /* Find display.*/
                break;
            }
        }

        if (display != NULL)
        {
            *Display = (HALNativeDisplayType)display;
            pthread_mutex_unlock(&displayMutex);

            gcmFOOTER_ARG("*Display=0x%x", *Display);
            return gcvSTATUS_OK;
        }

        display = (struct _GBMDisplay*) malloc(sizeof(struct _GBMDisplay));

        if (display == gcvNULL)
        {
            break;
        }

        display->index        = DisplayIndex;
        display->drmCard      = -1;
        display->gbmDev       = NULL;
        display->tiling       = gcvLINEAR;
        display->swapInterval = 1;
        display->next         = NULL;

        if (DisplayIndex == 0)
        {
            DisplayIndex = DEFAULT_DRM_DEV_ID;
        }

        snprintf(drmCardPath, 256, DRM_DEV_NAME, DRM_DIR_NAME, DisplayIndex);

        display->drmCard = open(drmCardPath, O_RDWR);

        if (display->drmCard < 0)
        {
            gcmPRINT("failed to open %s", drmCardPath);
            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        display->gbmDev = gbm_create_device(display->drmCard);

        if (!display->gbmDev)
        {
            gcmPRINT("failed to create gbm_device");
            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        display->next = displayStack;
        displayStack  = NULL;

        pthread_mutex_unlock(&displayMutex);

        *Display = display->gbmDev;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (0);

    pthread_mutex_unlock(&displayMutex);

    if (display != gcvNULL)
    {
        if (display->gbmDev != gcvNULL)
        {
            gbm_device_destroy(display->gbmDev);
        }

        if (display->drmCard >= 0)
        {
            close(display->drmCard);
        }

        free(display);
        display = gcvNULL;
    }

    *Display = gcvNULL;
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_GetDisplay(
    OUT HALNativeDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return _GetDisplayByIndex(0, Display, Context);
}

gceSTATUS
gcoOS_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT HALNativeDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return _GetDisplayByIndex(DisplayIndex, Display, Context);
}

gceSTATUS
gcoOS_GetDisplayInfo(
    IN HALNativeDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctSIZE_T * Physical,
    OUT gctINT * Stride,
    OUT gctINT * BitsPerPixel
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetDisplayInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT halDISPLAY_INFO * DisplayInfo
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetDisplayVirtual(
    IN HALNativeDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetDisplayBackbuffer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SetDisplayVirtual(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SetDisplayVirtualEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CancelDisplayBackbuffer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SetSwapInterval(
    IN HALNativeDisplayType Display,
    IN gctINT Interval
    )
{
    struct _GBMDisplay * display;

    /* Lock display stack. */
    pthread_mutex_lock(&displayMutex);

    display = _FindDisplayLocked(Display);

    if (display)
    {
        display->swapInterval = Interval;
    }

    /* Lock display stack. */
    pthread_mutex_unlock(&displayMutex);

    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_SetSwapIntervalEx(
    IN HALNativeDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay
    )
{
    return gcoOS_SetSwapInterval(Display, Interval);
}

gceSTATUS
gcoOS_GetSwapInterval(
    IN HALNativeDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
    )
{
    /* TODO */
    gcmHEADER_ARG("Display=0x%x Min=0x%x Max=0x%x", Display, Min, Max);

    if( Min != gcvNULL)
    {
        *Min = 1;
    }

    if (Max != gcvNULL)
    {
        *Max = 60;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

}

gceSTATUS
gcoOS_DestroyDisplay(
    IN HALNativeDisplayType Display
    )
{
    struct _GBMDisplay* display;

    pthread_mutex_lock(&displayMutex);

    display = _FindDisplayLocked(Display);

    if (display != NULL)
    {
        /* Unlink form display stack. */
        if (display == displayStack)
        {
            /* First one. */
            displayStack = display->next;
        }
        else
        {
            struct _GBMDisplay* prev = displayStack;

            while (prev->next != display)
            {
                prev = prev->next;
            }

            prev->next = display->next;
        }
    }

    pthread_mutex_unlock(&displayMutex);

    if (display != NULL)
    {
        if (display->gbmDev != NULL)
        {
            gbm_device_destroy(display->gbmDev);
            display->gbmDev = NULL;
        }

        if (display->drmCard >= 0)
        {
            close(display->drmCard);
            display->drmCard = -1;
        }

        free(display);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_DisplayBufferRegions(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Windows. ********************************************************************
*/

gceSTATUS
gcoOS_CreateWindow(
    IN HALNativeDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT HALNativeWindowType * Window
    )
{
    /* No window supported. */
    gcmPRINT("%s: X=%d Y=%d Width=%d Height=%d",
             __func__, X, Y, Width, Height);
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetWindowInfo(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DestroyWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DrawImage(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetImage(
    IN HALNativeWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    OUT gctINT * BitsPerPixel,
    OUT gctPOINTER * Bits
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}



/*******************************************************************************
** Pixmaps. ********************************************************************
*/

gceSTATUS
gcoOS_CreatePixmap(
    IN HALNativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    OUT HALNativePixmapType * Pixmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    int format;
    struct gbm_bo *bo;

    gcmHEADER_ARG("Display=%p Width=%d Height=%d BitsPerPixel=%d",
                  Display, Width, Height, BitsPerPixel);

    if (Display == gcvNULL || Pixmap == gcvNULL ||
        Width <= 0 || Height <= 0 || !Pixmap)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (BitsPerPixel)
    {
    case 16:
        format = GBM_FORMAT_RGB565;
        break;
    case 32:
        format = GBM_FORMAT_ARGB8888;
        break;
    default:
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    bo = gbm_bo_create(Display, Width, Height, format, 0);

    if (!bo)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_GetPixmapInfo(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (Display == gcvNULL || Pixmap == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Bits != NULL)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (Width != NULL)
    {
        *Width = (gctINT) gbm_bo_get_width(Pixmap);
    }

    if (Height != NULL)
    {
        *Height = (gctINT) gbm_bo_get_height(Pixmap);
    }

    if (BitsPerPixel != NULL)
    {
        gcsSURF_FORMAT_INFO_PTR formatInfo = gcvNULL;
        gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
        int format;
        int i;

        format = gbm_bo_get_format(Pixmap);

        for (i = 0; i < gcmSIZEOF(formatTable); i++)
        {
            if (format == formatTable[i].gbmFormat)
            {
                halFormat = formatTable[i].halFormat;
                break;
            }
        }

        if (halFormat == gcvSURF_UNKNOWN)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcoSURF_QueryFormat(halFormat, &formatInfo);
        *BitsPerPixel = formatInfo->bitsPerPixel;
    }

    if (Format != NULL)
    {
        gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
        int i;
        int format = gbm_bo_get_format(Pixmap);

        for (i = 0; i < gcmSIZEOF(formatTable); i++)
        {
            if (format == formatTable[i].gbmFormat)
            {
                halFormat = formatTable[i].halFormat;
                break;
            }
        }

        if (halFormat == gcvSURF_UNKNOWN)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        *Format = halFormat;
    }

    if (Stride != NULL)
    {
        *Stride = gbm_bo_get_stride(Pixmap);
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
gcoOS_GetPixmapInfo(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    status = _GetPixmapInfo(Display, Pixmap, Width, Height,
                            BitsPerPixel, Stride, Bits, gcvNULL);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_DestroyPixmap(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap
    )
{
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    if (Display == gcvNULL || Pixmap == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Destroy the bo. */
    gbm_bo_destroy(Pixmap);

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_DrawPixmap(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_LoadEGLLibrary(
    OUT gctHANDLE * Handle
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_FreeEGLLibrary(
    IN gctHANDLE Handle
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_ShowWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_HideWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SetWindowTitle(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctCONST_STRING Title
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CapturePointer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetEvent(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT halEvent * Event
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CreateClientBuffer(
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT Format,
    IN gctINT Type,
    OUT gctPOINTER * ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetClientBufferInfo(
    IN gctPOINTER ClientBuffer,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DestroyClientBuffer(
    IN gctPOINTER ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_InitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_DeinitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_GetDisplayInfoEx2(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT halDISPLAY_INFO * DisplayInfo
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetDisplayBackbufferEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_IsValidDisplay(
    IN HALNativeDisplayType Display
    )
{
    struct _GBMDisplay* display;

    if (Display == NULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    display = _FindDisplay(Display);

    if (display != NULL)
    {
        /* If created by HAL, it is valid. */
        return gcvSTATUS_OK;
    }
    else
    {
        /* Check backend name. */
        if (gbm_device_get_backend_name(Display) != NULL)
        {
            return gcvSTATUS_OK;
        }
    }

    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
gcoOS_GetNativeVisualId(
    IN HALNativeDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_GetWindowInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset,
    OUT gceSURF_FORMAT * Format,
    OUT gceSURF_TYPE * Type
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DrawImageEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits,
    IN gceSURF_FORMAT Format
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SetWindowFormat(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format
    )
{
    /*
     * Possiable types:
     *   gcvSURF_BITMAP
     *   gcvSURF_RENDER_TARGET
     *   gcvSURF_RENDER_TARGET_NO_COMPRESSION
     *   gcvSURF_RENDER_TARGET_NO_TILE_STATUS
     */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetPixmapInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    status = _GetPixmapInfo(Display, Pixmap, Width, Height,
                            BitsPerPixel, Stride, Bits, Format);

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_CopyPixmapBits(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    OUT gctPOINTER DstBits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gctBOOL
gcoOS_SynchronousFlip(
    IN HALNativeDisplayType Display
    )
{
    return gcvFALSE;
}
gceSTATUS
gcoOS_CreateContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}
gceSTATUS
gcoOS_DestroyContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_MakeCurrent(
    IN gctPOINTER Display,
    IN HALNativeWindowType DrawDrawable,
    IN HALNativeWindowType ReadDrawable,
    IN gctPOINTER Context,
    IN gcoSURF ResolveTarget
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CreateDrawable(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DestroyDrawable(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SwapBuffers(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN HALNativeWindowType Drawable,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#endif
#endif

