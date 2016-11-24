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


#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 501
#endif

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

#include <gbmint.h>
#include <gbm_vivint.h>

#include "gc_egl_platform.h"

#ifndef DRM_DIR_NAME
#  define DRM_DIR_NAME  "/dev/dri"
#  define DRM_DEV_NAME  "%s/card%d"
#  define DRM_CONTROL_DEV_NAME  "%s/controlD%d"
#  define DRM_RENDER_DEV_NAME  "%s/renderD%d"
#endif

#define DEFAULT_DRM_DEV_ID              (0)

typedef struct gbm_device *  PlatformDisplayType;
typedef struct gbm_surface * PlatformWindowType;
typedef struct gbm_bo *      PlatformPixmapType;

#define _GC_OBJ_ZONE    gcvZONE_OS

#define GBM_MAX_BUFFER  4

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
    struct gbm_device * gbm;
    gceTILING tiling;

    int multiBuffer;
    int swapInterval;

    /* Pointer to next. */
    struct _GBMDisplay * next;
};

struct _GBMBuffer {
   struct gbm_bo       *bo;
    enum {
        LOCKED_BY_CLIENT, /* taken by gbm_surface_lock_front_buffer */
        USED_BY_EGL, /* currently in use for EGL rendering */
        FRONT_BUFFER, /* last buffer with completed rendering */
        FREE /* free */
    } status;
    uint32_t bpp;
    gcoSURF         render_surface;
    struct _GBMWindow *win;
};

struct _GBMWindow
{
    struct gbm_surface base;
    struct _GBMBuffer buffers[GBM_MAX_BUFFER];
    gctINT buffer_count;
    uint32_t bpp;

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
        int fd = -1;
        displayStack = display->next;

        if (display->gbm != NULL)
        {
            fd = gbm_device_get_fd(display->gbm);
            gbm_device_destroy(display->gbm);
            display->gbm = NULL;
        }

        if (fd >= 0)
        {
            close(fd);
        }

        free(display);
    }

    pthread_mutex_unlock(&displayMutex);
}

static struct _GBMDisplay *
_FindDisplayLocked(
    PlatformDisplayType Display
    )
{
    struct _GBMDisplay* display;

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display->gbm == Display)
        {
            /* Found display. */
            return display;
        }
    }

    return NULL;
}

static struct _GBMDisplay *
_FindDisplay(
    PlatformDisplayType Display
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

gceSTATUS
gbm_GetHALFormat(
    IN uint32_t GbmFormat,
    OUT gceSURF_FORMAT *HalFormat
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
    int i;

    if (HalFormat)
    {
        for (i = 0; i < gcmSIZEOF(formatTable) / gcmSIZEOF(formatTable[0]); i++)
        {
            if (GbmFormat == formatTable[i].gbmFormat)
            {
                halFormat = formatTable[i].halFormat;
                break;
            }
        }

        if (halFormat == gcvSURF_UNKNOWN)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        *HalFormat = halFormat;
    }

OnError:
    return status;
}

gceSTATUS
_create_gbm_buffers(
    struct _GBMWindow *win,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t usage
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT xalignment = 0;
    int i;
    uint32_t alligned_width = 0;
    gceSURF_FORMAT gc_format;

    gcmONERROR(gbm_GetHALFormat(format, &gc_format));
    gcmONERROR(gcoSURF_GetAlignment(gcvSURF_BITMAP, gc_format, NULL, &xalignment, NULL));
    alligned_width = gcmALIGN_NP2(width, xalignment);

    for (i=0; i<win->buffer_count; i++)
    {
        win->buffers[i].bo = gbm_bo_create(win->base.gbm, alligned_width, height,
                                           format, usage);
        if (!win->buffers[i].bo)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        win->buffers[i].win = win;
        win->buffers[i].status = FREE;
        gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        gcmONERROR(gcoSURF_ConstructWrapper(NULL, &win->buffers[i].render_surface));
        gcmONERROR(gcoSURF_SetBuffer(win->buffers[i].render_surface, gcvSURF_BITMAP, gc_format,
                                    (gctUINT)win->buffers[i].bo->stride,
                                    gbm_viv_bo(win->buffers[i].bo)->map, ~0U));
        gcmONERROR(gcoSURF_SetWindow(win->buffers[i].render_surface,
                                   0,
                                   0,
                                   width,
                                   height));
    }

    return gcvSTATUS_OK;

OnError:
    for (i=0; i<win->buffer_count; i++)
    {
        if (win->buffers[i].render_surface)
        {
            gcoSURF_Destroy(win->buffers[i].render_surface);
        }

        if (win->buffers[i].bo)
        {
            gbm_bo_destroy(win->buffers[i].bo);
        }

        win->buffers[i].render_surface = NULL;
        win->buffers[i].bo = NULL;
    }

    return status;
}

static struct gbm_surface *
gbm_SurfaceCreate(
    struct gbm_device *gbm,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t flags
    )
{
    gceSTATUS status;
    uint32_t usage = GBM_BO_USE_SCANOUT;
    struct _GBMWindow *win = calloc(1, sizeof(*win));
    if (!win)
        return NULL;

    win->buffer_count = GBM_MAX_BUFFER;
    win->base.gbm = gbm;
    win->base.width = width;
    win->base.height = height;
    win->base.format = format;
    win->base.flags = flags;

    gcmONERROR(_create_gbm_buffers(win, width, height,
                                 format, usage));

    return &win->base;

OnError:
    /*Create gbm_bo failed */
    free(win);

    return NULL;
}

static struct gbm_bo *
gbm_SurfaceLockFrontBuffer(
    struct gbm_surface *surface
    )
{
    struct _GBMWindow *win = (struct _GBMWindow *) surface;
    struct gbm_bo *bo = NULL;
    int i;

    for (i = 0; i < win->buffer_count; i++) {
       if (win->buffers[i].status == FRONT_BUFFER) {
           win->buffers[i].status = LOCKED_BY_CLIENT;
           bo = win->buffers[i].bo;
       }
    }

    return bo;
}

static void
gbm_SurfaceReleaseBuffer(
    struct gbm_surface *surface,
    struct gbm_bo *bo
    )
{
    struct _GBMWindow *win = (struct _GBMWindow *) surface;
    int i;

    for (i = 0; i < win->buffer_count; i++) {
       if (win->buffers[i].bo == bo) {
           win->buffers[i].status = FREE;
           break;
       }
    }

    return;
}

static int
gbm_SurfaceHasFreeBuffers(
    struct gbm_surface *surface
    )
{
    struct _GBMWindow *win = (struct _GBMWindow *) surface;
    int i;

    for (i = 0; i < win->buffer_count; i++) {
       if (win->buffers[i].status == FREE) {
           return 1;
       }
    }

    return 0;
}

static void
gbm_SurfaceDestroy(
    struct gbm_surface *surface
    )
{
    struct _GBMWindow *win = (struct _GBMWindow *) surface;
    int i;

    if (!win)
        return;

    for (i = 0; i < win->buffer_count; i++) {
        if (win->buffers[i].render_surface != NULL)
        {
           gcoSURF_Destroy(win->buffers[i].render_surface);
           win->buffers[i].render_surface = NULL;
        }

        if (win->buffers[i].bo != NULL)
        {
           gbm_bo_destroy(win->buffers[i].bo);
           win->buffers[i].bo = NULL;
        }
    }

    free(win);

    return;
}

static
gceSTATUS
_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    int fd = -1;
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
            *Display = display->gbm;
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
        display->gbm          = NULL;
        display->tiling       = gcvLINEAR;
        display->swapInterval = 1;
        display->next         = NULL;

        if (DisplayIndex == 0)
        {
            DisplayIndex = DEFAULT_DRM_DEV_ID;
        }

        snprintf(drmCardPath, 256, DRM_DEV_NAME, DRM_DIR_NAME, DisplayIndex);

        fd = open(drmCardPath, O_RDWR);

        if (fd < 0)
        {
            gcmPRINT("failed to open %s", drmCardPath);
            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        display->gbm = gbm_create_device(fd);

        if (!display->gbm)
        {
            gcmPRINT("failed to create gbm_gbm");
            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        display->gbm->surface_create = gbm_SurfaceCreate;
        display->gbm->surface_lock_front_buffer = gbm_SurfaceLockFrontBuffer;
        display->gbm->surface_release_buffer = gbm_SurfaceReleaseBuffer;
        display->gbm->surface_has_free_buffers = gbm_SurfaceHasFreeBuffers;
        display->gbm->surface_destroy = gbm_SurfaceDestroy;

        display->next = displayStack;
        displayStack  = display;

        pthread_mutex_unlock(&displayMutex);

        *Display = display->gbm;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (0);

    pthread_mutex_unlock(&displayMutex);

    if (display != gcvNULL)
    {
        if (display->gbm != gcvNULL)
        {
            gbm_device_destroy(display->gbm);
        }

        if (fd >= 0)
        {
            close(fd);
        }

        free(display);
        display = gcvNULL;
    }

    *Display = gcvNULL;
    gcmFOOTER();
    return status;
}

static
gceSTATUS
_GetDisplayByDevice(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _GBMDisplay * display = NULL;

    gcmHEADER_ARG("Context=%d", Context);

    /* Init global variables. */
    pthread_once(&onceControl, _OnceInit);

    /* Lock display stack. */
    pthread_mutex_lock(&displayMutex);

    do
    {
        if (!Context)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        for (display = displayStack; display != NULL; display = display->next)
        {
            if (display->gbm == Context)
            {
                /* Find display.*/
                break;
            }
        }

        if (display != NULL)
        {
            *Display = display->gbm;
            pthread_mutex_unlock(&displayMutex);

            gcmFOOTER_ARG("*Display=0x%x", *Display);
            return gcvSTATUS_OK;
        }

        display = (struct _GBMDisplay*) malloc(sizeof(struct _GBMDisplay));

        if (display == gcvNULL)
        {
            break;
        }

        display->gbm          = Context;
        display->tiling       = gcvLINEAR;
        display->swapInterval = 1;
        display->next         = NULL;


        display->gbm->surface_create = gbm_SurfaceCreate;
        display->gbm->surface_lock_front_buffer = gbm_SurfaceLockFrontBuffer;
        display->gbm->surface_release_buffer = gbm_SurfaceReleaseBuffer;
        display->gbm->surface_has_free_buffers = gbm_SurfaceHasFreeBuffers;
        display->gbm->surface_destroy = gbm_SurfaceDestroy;

        display->next = displayStack;
        displayStack  = display;

        pthread_mutex_unlock(&displayMutex);

        *Display = display->gbm;

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    while (0);

    pthread_mutex_unlock(&displayMutex);

    if (display != gcvNULL)
    {
        if (display->gbm != gcvNULL)
        {
            gbm_device_destroy(display->gbm);
        }


        free(display);
        display = gcvNULL;
    }

    *Display = gcvNULL;
    gcmFOOTER();
    return status;
}

gceSTATUS
gbm_GetDisplayByDevice(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return _GetDisplayByDevice(Display, Context);
}

gceSTATUS
gbm_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return _GetDisplayByIndex(0, Display, Context);
}

gceSTATUS
gbm_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return _GetDisplayByIndex(DisplayIndex, Display, Context);
}

gceSTATUS
gbm_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    /* TODO */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gbm_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
gbm_SetDisplayVirtual(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    int i;
    struct _GBMWindow* win = ((struct _GBMWindow*) Window);

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    for (i = 0; i < win->buffer_count; i++) {
        if (win->buffers[i].status == USED_BY_EGL) {
            win->buffers[i].status = FRONT_BUFFER;
            break;
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gbm_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
gbm_SetSwapInterval(
    IN PlatformDisplayType Display,
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
gbm_SetSwapIntervalEx(
    IN PlatformDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay
    )
{
    return gbm_SetSwapInterval(Display, Interval);
}

gceSTATUS
gbm_GetSwapInterval(
    IN PlatformDisplayType Display,
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
gbm_DestroyDisplay(
    IN PlatformDisplayType Display
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
        int fd = -1;

        if (display->gbm != NULL)
        {
            fd = gbm_device_get_fd(display->gbm);
            gbm_device_destroy(display->gbm);
            display->gbm = NULL;
        }

        if (fd >= 0)
        {
            close(fd);
        }

        free(display);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gbm_DisplayBufferRegions(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

static gceSTATUS
_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
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

        for (i = 0; i < gcmSIZEOF(formatTable) / gcmSIZEOF(formatTable[0]); i++)
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

        for (i = 0; i < gcmSIZEOF(formatTable) / gcmSIZEOF(formatTable[0]); i++)
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
gbm_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
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
gbm_DestroyPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap
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
gbm_DrawPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
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
gbm_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gbm_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gbm_GetDisplayBackbufferEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    int i;
    struct _GBMWindow * win = (struct _GBMWindow *)Window;

    for (i = 0; i < win->buffer_count; i++) {
       if (win->buffers[i].status == FREE) {
           *surface = win->buffers[i].render_surface;
           win->buffers[i].status = USED_BY_EGL;
           *Offset  = 0;
           *X       = 0;
           *Y       = 0;
           break;
       }
    }

    if(!surface)
        return gcvSTATUS_OUT_OF_RESOURCES;

    return gcvSTATUS_OK;
}

gceSTATUS
gbm_IsValidDisplay(
    IN PlatformDisplayType Display
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
gbm_GetNativeVisualId(
    IN PlatformDisplayType Display,
    IN struct eglConfig * Config,
    OUT gctINT* nativeVisualId
    )
{
    gcmASSERT(Config != gcvNULL);
    gcmASSERT(nativeVisualId != gcvNULL);

    /* GBM_FORMAT_RGB565 */
    if ((Config->redSize == 5) &&
        (Config->greenSize == 6) &&
        (Config->blueSize == 5) &&
        (Config->alphaSize == 0))
    {
        *nativeVisualId = (gctINT)GBM_FORMAT_RGB565;
    }
    /* GBM_FORMAT_XRGB8888 */
    else if ((Config->redSize == 8) &&
             (Config->greenSize == 8) &&
             (Config->blueSize == 8) &&
             (Config->alphaSize == 0))
    {
        *nativeVisualId = (gctINT)GBM_FORMAT_XRGB8888;
    }
    /* GBM_FORMAT_ARGB8888 */
    else if ((Config->redSize == 8) &&
             (Config->greenSize == 8) &&
             (Config->blueSize == 8) &&
             (Config->alphaSize == 8))
    {
        *nativeVisualId = (gctINT)GBM_FORMAT_ARGB8888;
    }
    /* Unsupported*/
    else
    {
        *nativeVisualId = 0;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gbm_GetWindowInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
    gceSTATUS status = gcvSTATUS_OK;
    struct gbm_surface *surf = (struct gbm_surface *)Window;

    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Display == gcvNULL || Window == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (X != NULL)
    {
        *X = 0;
    }

    if (Y != NULL)
    {
        *Y = 0;
    }

    if (Offset != NULL)
    {
        *Offset = 0;
    }

    if (Width != NULL)
    {
        *Width = (gctINT) surf->width;
    }

    if (Height != NULL)
    {
        *Height = (gctINT) surf->height;
    }

    if ((BitsPerPixel != NULL) ||
        (Format != NULL))
    {
        gcsSURF_FORMAT_INFO_PTR formatInfo = gcvNULL;
        gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
        int format;
        int i;

        format = surf->format;

        for (i = 0; i < gcmSIZEOF(formatTable) / gcmSIZEOF(formatTable[0]); i++)
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

        if (BitsPerPixel != NULL)
        {
            *BitsPerPixel = formatInfo->bitsPerPixel;
        }

        if (Format != NULL)
        {
            *Format = halFormat;
        }
    }

    if (Type != NULL)
    {
        *Type = gcvSURF_BITMAP;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gbm_DrawImageEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
gbm_SetWindowFormat(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
gbm_GetPixmapInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
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
gbm_CopyPixmapBits(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    OUT gctPOINTER DstBits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gbm_CreateDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gbm_DestroyDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/******************************************************************************/
/* TODO: merge functions. */

#include <gc_egl_precomp.h>


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
 * will cause infinite wait in getWindowBackBuffer.
 */
#define SYNC_TEMPORARY_RESOLVE_SURFACES     0


/******************************************************************************/
/* Display. */

static void *
_GetDefaultDisplay(
    void
    )
{
    PlatformDisplayType display = gcvNULL;
    gbm_GetDisplay(&display, gcvNULL);

    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    gbm_DestroyDisplay((PlatformDisplayType) Display);
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (gcmIS_ERROR(gbm_IsValidDisplay((PlatformDisplayType) Display)))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = gbm_InitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
                                        &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = gbm_DeinitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
                                          &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint visualId = 0;

    gbm_GetNativeVisualId((PlatformDisplayType) Display->hdc, Config, &visualId);
    return visualId;
}

/* Query of swap interval range. */
static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Min,
    OUT EGLint * Max
    )
{
    gceSTATUS status;

    /* Get swap interval from HAL OS layer. */
    status = gbm_GetSwapInterval((PlatformDisplayType) Display->hdc,
                                   Min, Max);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    )
{
    gceSTATUS status;

    status = gbm_SetSwapInterval((PlatformDisplayType) Display->hdc,
                                   Interval);

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
     * As descripted in comments in 'bindWindow' in header file, 4 conditions
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

    /* Front and back buffer pointers. */
    VEGLNativeBuffer    front;
    VEGLNativeBuffer    back;

    /* Information obtained by gbm_GetDisplayInfoEx2. */
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
    void * Window,
    VEGLWindowInfo Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VEGLNativeBuffer buffer = gcvNULL;

    if (Info->fbDirect)
    {
        if (Info->wrapFB)
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

                buffer = gcvNULL;

                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): buffer[%d]: yoffset=%-4d physical=%x",
                         __FUNCTION__, __LINE__,
                         i, alignedHeight * i, physical);
            }

            gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
        }
        else
        {
            gcmPRINT("%s(%d): Invalid integration!", __FUNCTION__, __LINE__);
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
    /* Must ensure that it will not happen any ONERROR after add into buffer list */
    if (buffer != gcvNULL)
    {
        if (buffer->surface != gcvNULL)
        {
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;
        }

        if (buffer->lock != gcvNULL)
        {
            gcoOS_DestroySignal(gcvNULL, buffer->lock);
            buffer->lock = gcvNULL;
        }
        gcmOS_SAFE_FREE(gcvNULL, buffer);
    }

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
    VEGLSurface Surface,
    void * Window,
    VEGLWindowInfo Info
    )
{
    if (Info->bufferList)
    {
        VEGLNativeBuffer buffer;

        /* Make sure all workers have been processed. */
        if (Surface->workerDoneSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                          Surface->workerDoneSignal,
                                          gcvINFINITE));
        }

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
    IN void * Window,
    IN VEGLWindowInfo Info
    )
{
    gceSTATUS status;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = gbm_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) Window,
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

    {
        Info->fbDirect     = EGL_TRUE;
        Info->logical      = gcvNULL;
        Info->physical     = gcvINVALID_ADDRESS;
        Info->stride       = 0;
        Info->wrapFB       = gcvFALSE;
        Info->multiBuffer  = 1;
    }

    /* Get virtual size. */
    status = gbm_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
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

static EGLBoolean
_ConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN void * Window
    )
{
    gceSTATUS status;
    VEGLWindowInfo info = gcvNULL;
    void * win = Window;
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != gcvNULL);
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
    gbm_CreateDrawable(Display->localInfo, (PlatformWindowType) win);

    /* Create window buffers to represent window native bufers. */
    gcmONERROR(_CreateWindowBuffers(Window, info));

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

static EGLBoolean
_DisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Free native window buffers. */
    _FreeWindowBuffers(Surface, win, info);

    /* Delete the mutex. */
    gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
    info->bufferListMutex = gcvNULL;

    gbm_DestroyDrawable(Display->localInfo, (PlatformWindowType) win);

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


static EGLBoolean
_BindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    )
{
    gceSTATUS status;

    /* Get shortcut. */
    void * win  = Surface->hwnd;
    VEGLWindowInfo info   = Surface->winInfo;
    /* Indirect rendering by default. */
    EGLint renderMode     = VEGL_INDIRECT_RENDERING;
    EGLBoolean winChanged = EGL_FALSE;
    gctINT width          = 0;
    gctINT height         = 0;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceSURF_TYPE type     = gcvSURF_UNKNOWN;

    status = gbm_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Check window resize. */
    if ((info->width  != (gctUINT)width) ||
        (info->height != (gctUINT)height) ||
        (info->format != format))
    {
        /* Native window internally changed. */
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

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            switch (format)
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

        if ((type != gcvSURF_BITMAP) ||
            (info->type != gcvSURF_BITMAP))
        {
            /* Request linear buffer for hardware OpenVG. */
            status = gbm_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            /* Recreate window buffers. */
            _FreeWindowBuffers(Surface, win, info);
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
            gceSURF_FORMAT reqFormat = format;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

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
            switch (format)
            {
            case gcvSURF_A8B8G8R8:
                reqFormat = gcvSURF_A8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8B8G8R8:
                reqFormat = gcvSURF_X8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A4R4G4B4:
            case gcvSURF_X4R4G4B4:
            case gcvSURF_R5G6B5:
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

            if (!info->wrapFB)
            {
                /* Try many direct rendering levels here. */
                /* 1. The best, direct rendering with compression. */
                if ((type != gcvSURF_RENDER_TARGET) ||
                    (info->type != gcvSURF_RENDER_TARGET)  ||
                    (info->format != reqFormat))
                {
                    status = gbm_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering with compression. */
                        renderMode = VEGL_DIRECT_RENDERING;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING;
                }

                /* 2. Second, with tile status, no compression. */
                if ((type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->format != reqFormat))
                {

                    status = gbm_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET_NO_COMPRESSION,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering without compression. */
                        renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already direct rendering without compression. */
                    renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
                }
            }

            if (!fcFill)
            {
                /* Do not need check the next mode. */
                break;
            }

            /*
             * Special for FC-FILL mode: tile status is required.
             * Final internal render type should be RENDER_TARGET_NO_COMPRESSION.
             */
            if ((type != gcvSURF_RENDER_TARGET_NO_TILE_STATUS) ||
                (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                (info->format != reqFormat))
            {
                /* Try FC fill. */
                status = gbm_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                               (PlatformWindowType) win,
                                               gcvSURF_RENDER_TARGET_NO_TILE_STATUS,
                                               reqFormat);

                if (gcmIS_SUCCESS(status))
                {
                    /* Should use direct rendering with fc-fill. */
                    renderMode = VEGL_DIRECT_RENDERING_FCFILL;

                    /* Window is changed. */
                    winChanged = EGL_TRUE;
                    break;
                }

                /* Not an error. */
                status = gcvSTATUS_OK;
            }
            else
            {
                /* Already direct rendering with fc-fill. */
                renderMode = VEGL_DIRECT_RENDERING_FCFILL;
            }
        }
        while (gcvFALSE);
#   endif

        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            ((type != gcvSURF_BITMAP) || (info->type != gcvSURF_BITMAP)))
        {
            /* Only linear supported in this case. */
            status = gbm_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
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
            _FreeWindowBuffers(Surface, win, info);
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

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_GetWindowSize(
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
    void * win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

    status = gbm_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) win,
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

static EGLBoolean
_GetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    gceSTATUS status;
    void * win = Surface->hwnd;
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
        status = gbm_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
                                              (PlatformWindowType) win,
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
            VEGLNativeBuffer buffer = gcvNULL;

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

static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN EGLint NumRects,
    IN EGLint Rects[]
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    (void) surface;

    if (info->fbDirect)
    {
        surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

        status = gbm_SetDisplayVirtual((PlatformDisplayType) Display->hdc,
                                       (PlatformWindowType) win,
                                       0,
                                       BackBuffer->origin.x,
                                       BackBuffer->origin.y);

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
            status = gbm_DrawImageEx((PlatformDisplayType) Display->hdc,
                                       (PlatformWindowType) win,
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

static EGLBoolean
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

    status = gbm_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           BackBuffer->context,
                                           surface,
                                           0,
                                           BackBuffer->origin.x,
                                           BackBuffer->origin.y);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return EGL_FALSE;
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    /* TODO */
    return EGL_TRUE;
}

static EGLBoolean
_QueryBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    OUT EGLint *BufferAge
    )
{
    /* TODO */
    return EGL_FALSE;
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
    void *              hdc;

    /* Pixmap wrapper. */
    gcoSURF             wrapper;

    /* Shadow surface, exists when the wrapper is not resovable. */
    gcoSURF             shadow;
};

static void
_DoSyncFromPixmap(
    void * Pixmap,
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
        gcmONERROR(gbm_CopyPixmapBits((PlatformDisplayType) Info->hdc,
                                        (PlatformPixmapType) Pixmap,
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
    void * Pixmap,
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
        gcmONERROR(gbm_DrawPixmap((PlatformDisplayType) Info->hdc,
                                    (PlatformPixmapType) Pixmap,
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

static EGLBoolean
_MatchPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN struct eglConfig * Config
    )
{
    gceSTATUS status;
    gctINT width, height, bitsPerPixel;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = gbm_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformPixmapType) Pixmap,
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

static EGLBoolean
_ConnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
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
    gcmONERROR(gbm_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = gbm_GetPixmapInfo((PlatformDisplayType) Display->hdc,
                                 (PlatformPixmapType) Pixmap,
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

        /* Check pixmap format. */
        switch (pixmapFormat)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_X8R8G8B8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_R5G6B5:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4R4G4B4:
        case gcvSURF_X4B4G4R4:
            break;

        default:
            /* Resolve can not support such format. */
            return EGL_FALSE;
        }
    }
    while (gcvFALSE);

    do
    {
        if (needShadow)
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
    info->hdc          = (PlatformDisplayType) Display->hdc;
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

static EGLBoolean
_DisconnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
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

static EGLBoolean
_GetPixmapSize(
    IN VEGLDisplay Display,
    IN void * Pixmap,
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
        gbm_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                              (PlatformPixmapType) Pixmap,
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

static EGLBoolean
_SyncFromPixmap(
    IN void * Pixmap,
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
            gcoSURF_CPUCacheOperation(Info->wrapper, gcvCACHE_FLUSH));
    }

    return EGL_TRUE;
}

static EGLBoolean
_SyncToPixmap(
    IN void * Pixmap,
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


static struct eglPlatform gbmPlatform =
{
    EGL_PLATFORM_GBM_KHR,

    _GetDefaultDisplay,
    _ReleaseDefaultDisplay,
    _IsValidDisplay,
    _InitLocalDisplayInfo,
    _DeinitLocalDisplayInfo,
    _GetNativeVisualId,
    _GetSwapInterval,
    _SetSwapInterval,
    _ConnectWindow,
    _DisconnectWindow,
    _BindWindow,
    _UnbindWindow,
    _GetWindowSize,
    _GetWindowBackBuffer,
    _PostWindowBackBuffer,
    gcvNULL,
    _CancelWindowBackBuffer,
    _SynchronousPost,
    gcvNULL,
    _UpdateBufferAge,
    _QueryBufferAge,
    _MatchPixmap,
    _ConnectPixmap,
    _DisconnectPixmap,
    _GetPixmapSize,
    _SyncFromPixmap,
    _SyncToPixmap,
};

VEGLPlatform
veglGetGbmPlatform(
    void * NativeDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    PlatformDisplayType dpy;

    gcmONERROR(gbm_GetDisplayByDevice(&dpy, NativeDisplay));

    return &gbmPlatform;

OnError:
    return NULL;
}
