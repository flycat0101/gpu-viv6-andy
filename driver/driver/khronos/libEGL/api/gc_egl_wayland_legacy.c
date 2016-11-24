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


#if !defined _XOPEN_SOURCE
#   define _XOPEN_SOURCE 501
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "gc_wayland_protocol.h"
#include "wayland-viv-server-protocol.h"
#include "wayland-server.h"

#include "gc_egl_platform.h"

#include <gc_hal_user.h>

#define WL_DUMMY (31415926)

typedef struct wl_display *    PlatformDisplayType;
typedef struct wl_egl_window * PlatformWindowType;
typedef struct wl_egl_pixmap * PlatformPixmapType;

typedef struct _gcsWL_EGL_WINDOW_INFO
{
   gctINT32 dx;
   gctINT32 dy;
   gctUINT width;
   gctUINT height;
   gceSURF_FORMAT format;
   gctUINT bpp;
   gctINT  bufferCount;
   gctUINT current;
}
gcsWL_EGL_WINDOW_INFO;

struct wl_egl_window
{
   gctUINT wl_signature;
   gcsWL_EGL_DISPLAY* display;
   gcsWL_EGL_BUFFER **backbuffers;
   gcsWL_EGL_WINDOW_INFO* info;
   gctINT  noResolve;
   gctINT32 attached_width;
   gctINT32 attached_height;
   gcsATOM_PTR reference;
   pthread_mutex_t window_mutex;
   struct wl_surface* surface;
   struct wl_list link;
};

#define _GC_OBJ_ZONE    gcvZONE_OS

#if defined(WAYLAND_VERSION_MAJOR) && \
    WAYLAND_VERSION_MAJOR == 1 && \
    WAYLAND_VERSION_MINOR < 6

#  define WAYLAND_LEGACY_SUPPORT
#endif

#define WL_EGL_NUM_BACKBUFFERS     3
#define WL_EGL_MAX_NUM_BACKBUFFERS 3

static struct wl_list WLEGLWindowList = {0};

static struct wl_list WLEGLBufferList = {0};

static pthread_mutex_t registerMutex = PTHREAD_MUTEX_INITIALIZER;

static inline gctBOOL
_IsWaylandLocalDisplay(gcsWL_LOCAL_DISPLAY* Display)
{
    return (Display->wl_signature == WL_LOCAL_DISPLAY_SIGNATURE);
}

static inline gctBOOL
_isValidWindow(struct wl_egl_window *window)
{
    struct wl_egl_window* win;

    wl_list_for_each(win, &WLEGLWindowList, link)
    {
        if (win == window)
            return gcvTRUE;
    }

    return gcvFALSE;
}

static inline gctBOOL
_isValidEGLBuffer(gcsWL_EGL_BUFFER *buffer)
{
    gcsWL_EGL_BUFFER* buf;

    wl_list_for_each(buf, &WLEGLBufferList, link)
    {
        if (buf == buffer)
            return gcvTRUE;
    }

    return gcvFALSE;
}

static void
registerWindow(struct wl_egl_window *window)
{
    int i;
    pthread_mutex_lock(&registerMutex);

    if (WLEGLWindowList.next == gcvNULL)
    {
        wl_list_init(&WLEGLWindowList);
        wl_list_init(&WLEGLBufferList);
    }

    wl_list_insert(&WLEGLWindowList, &window->link);

    for (i = 0; i < window->info->bufferCount; i++)
    {
        wl_list_insert(&WLEGLBufferList, &window->backbuffers[i]->link);
    }

    pthread_mutex_unlock(&registerMutex);
}

static void
unRegisterWindow(struct wl_egl_window *window)
{
    int i;

    if (WLEGLWindowList.next == gcvNULL)
    {
        gcmPRINT("The WLEGLWindowList was not initialized \n");
        return;
    }

    pthread_mutex_lock(&registerMutex);

    wl_list_remove(&window->link);

    for (i = 0; i < window->info->bufferCount; i++)
    {
        wl_list_remove(&window->backbuffers[i]->link);
    }

    pthread_mutex_unlock(&registerMutex);
}

gceSTATUS
wayland_SetWindowFormat(
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
    struct wl_egl_window *wl_win;

    if (Type != gcvSURF_BITMAP)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    wl_win = (struct wl_egl_window *) Window;
    wl_win->info->format = Format;
    return gcvSTATUS_OK;
}

typedef struct _waylandDISPLAY_INFO
{
    /* The size of the display in pixels. */
    int                         width;
    int                         height;

    /* The stride of the dispay. -1 is returned if the stride is not known
    ** for the specified display.*/
    int                         stride;

    /* The color depth of the display in bits per pixel. */
    int                         bitsPerPixel;

    /* The logical pointer to the display memory buffer. NULL is returned
    ** if the pointer is not known for the specified display. */
    void *                      logical;

    /* The physical address of the display memory buffer. ~0 is returned
    ** if the address is not known for the specified display. */
    unsigned long               physical;

    /* Can be wraped as surface. */
    int                         wrapFB;

    /* FB_MULTI_BUFFER support */
    int                         multiBuffer;
    int                         backBufferY;

    /* Tiled buffer / tile status support. */
    int                         tiledBuffer;
    int                         tileStatus;
    int                         compression;

    /* The color info of the display. */
    unsigned int                alphaLength;
    unsigned int                alphaOffset;
    unsigned int                redLength;
    unsigned int                redOffset;
    unsigned int                greenLength;
    unsigned int                greenOffset;
    unsigned int                blueLength;
    unsigned int                blueOffset;

    /* Display flip support. */
    int                         flip;
}
waylandDISPLAY_INFO;


gceSTATUS
wayland_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT waylandDISPLAY_INFO * DisplayInfo
    )
{
    struct wl_egl_window* wl_window = Window;
    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);

    if (Display == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    DisplayInfo->width = wl_window->info->width;
    DisplayInfo->height = wl_window->info->height;
    DisplayInfo->stride = WL_DUMMY;
    DisplayInfo->bitsPerPixel = wl_window->info->bpp;
    DisplayInfo->logical = (gctPOINTER) WL_DUMMY;
    DisplayInfo->physical = WL_DUMMY;
    DisplayInfo->multiBuffer = wl_window->info->bufferCount;
    DisplayInfo->backBufferY = 0;
    DisplayInfo->flip = gcvTRUE;
    DisplayInfo->wrapFB = gcvFALSE;

    gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);
    return gcvSTATUS_OK;
}

static void wayland_release_pending_resource(void *data, struct wl_callback *callback, uint32_t time)
{
   gcoSURF surface = (gcoSURF) data;

   wl_callback_destroy(callback);

   gcoSURF_Unlock(surface, gcvNULL);
   gcoSURF_Destroy(surface);
}

static const struct wl_callback_listener release_buffer_listener =
{
    wayland_release_pending_resource
};

gctBOOL
isRenderFinished(struct wl_egl_window *window)
{
    int i;

    if (window->display == gcvNULL)
    {
        return gcvTRUE;
    }

    for (i=0; i< window->info->bufferCount ; i++)
    {
        if (window->backbuffers[i]->frame_callback != NULL)
        {
            wl_display_dispatch_queue(window->display->wl_display, window->display->wl_swap_queue);
            return gcvFALSE;
        }
    }

   return gcvTRUE;
}

void wait_for_the_frame_finish(struct wl_egl_window *window)
{
    gctINT32 counter = 0;

    if (window->reference != gcvNULL)
    {
        do
        {
            gcoOS_AtomIncrement(gcvNULL, window->reference, gcvNULL);
            gcoOS_AtomDecrement(gcvNULL, window->reference, &counter);
            /* Sleep for a while */
            gcoOS_Delay(gcvNULL, 10);
        }

        while (counter > 1 || isRenderFinished(window) != gcvTRUE);
    }
}

static void
gcoWL_DestroryBO(struct wl_egl_window *window)
{
     gctUINT i;

    wait_for_the_frame_finish(window);

    if (window == NULL)
    {
        return;
    }

    for (i=0; i< window->info->bufferCount ; i++)
    {
        if (window->backbuffers[i]->info.surface != gcvNULL)
        {

           if(window->display)
           {
               struct wl_callback *callback;
               callback = wl_display_sync(window->display->wl_display);
               wl_callback_add_listener(callback, &release_buffer_listener,
                   window->backbuffers[i]->info.surface);
               wl_proxy_set_queue((struct wl_proxy *) callback, window->display->wl_swap_queue);
               window->backbuffers[i]->info.surface = gcvNULL;
           }
           else
           {
               gcoSURF_Unlock(window->backbuffers[i]->info.surface, gcvNULL);
               gcoSURF_Destroy(window->backbuffers[i]->info.surface);
           }
        }
    }
}

static gceSTATUS
gcoWL_CreateBO(struct wl_egl_window* window)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    do
    {
        gcsSURF_FORMAT_INFO_PTR renderTargetInfo[2];
        gceSURF_TYPE surfaceType = gcvSURF_BITMAP;

        gceSURF_FORMAT resolveFormat = window->info->format;
        int width = window->info->width;
        int height = window->info->height;

        if (window->noResolve)
        {
            /* TODO: no resolve ?? */
            surfaceType = gcvSURF_TEXTURE;
        }

        gcmONERROR(
            gcoSURF_QueryFormat(
                window->info->format,
                renderTargetInfo));

        window->info->bpp = renderTargetInfo[0]->bitsPerPixel;

        for (i=0; i< window->info->bufferCount ; i++)
        {
            gcmONERROR(
                gcoSURF_Construct(
                                gcvNULL,
                                width,
                                height,
                                1,
                                surfaceType,
                                resolveFormat,
                                gcvPOOL_DEFAULT,
                                &window->backbuffers[i]->info.surface
                                ));
            if(surfaceType != gcvSURF_BITMAP)
            {
                gcmONERROR(
                    gcoSURF_SetFlags(
                        window->backbuffers[i]->info.surface,
                        gcvSURF_FLAG_CONTENT_YINVERTED,
                        gcvTRUE));
            }

            gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL
                    ));

            gcmONERROR(
                gcoSURF_GetAlignedSize(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL,
                    &window->backbuffers[i]->info.stride
                    ));

            gcmONERROR(
               gcoSURF_QueryVidMemNode(
                    window->backbuffers[i]->info.surface,
                    (gctUINT32 *)&window->backbuffers[i]->info.node,
                    &window->backbuffers[i]->info.pool,
                    &window->backbuffers[i]->info.bytes
                    ));

            gcmONERROR(
                gcoHAL_NameVideoMemory(gcmPTR2INT(window->backbuffers[i]->info.node),
                                       gcmINT2PTR(&window->backbuffers[i]->info.node)));

            window->backbuffers[i]->info.width = window->info->width;
            window->backbuffers[i]->info.height = window->info->height;
            window->backbuffers[i]->info.format = resolveFormat;
            window->backbuffers[i]->info.type = surfaceType;
            window->backbuffers[i]->info.invalidate = gcvTRUE;
            window->backbuffers[i]->frame_callback = gcvNULL;
            window->backbuffers[i]->info.locked = gcvFALSE;
            gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p): width=%d, height=%d, format=%d, stride=%d, node=%p, pool=%d, bytes=%d, calc=%d",
                        i, window->backbuffers[i]->info.surface,
                        window->info->width, window->info->height,
                        window->backbuffers[i]->info.format,
                        window->backbuffers[i]->info.stride,
                        window->backbuffers[i]->info.node,
                        window->backbuffers[i]->info.pool,
                        window->backbuffers[i]->info.bytes,
                        window->backbuffers[i]->info.stride*window->info->width
                        );
        }
    }
    while (gcvFALSE);
    return status;
OnError:
    gcoWL_DestroryBO(window);
    gcoOS_FreeMemory(gcvNULL, window);
    window = gcvNULL;
    return gcvSTATUS_INVALID_ARGUMENT;
}


static gceSTATUS
gcoWL_ResizeBO(struct wl_egl_window* window)
{
     gceSTATUS status = gcvSTATUS_OK;
     gctUINT i;

    gcmASSERT(window);

    do
    {
        gceSURF_TYPE surfaceType = gcvSURF_BITMAP;
        gceSURF_FORMAT resolveFormat = window->info->format;
        struct wl_callback *callback;
        i = window->info->current;

        callback = wl_display_sync(window->display->wl_display);
        wl_callback_add_listener(callback, &release_buffer_listener,
                window->backbuffers[i]->info.surface);
        wl_proxy_set_queue((struct wl_proxy *) callback, window->display->wl_swap_queue);
        window->backbuffers[i]->info.surface = gcvNULL;

        if (window->noResolve)
        {
            /* TODO: no resolve ?? */
            surfaceType = gcvSURF_TEXTURE;
        }

        gcmONERROR(
            gcoSURF_Construct(
                      gcvNULL,
                      window->info->width,
                      window->info->height,
                      1,
                      surfaceType,
                      resolveFormat,
                      gcvPOOL_DEFAULT,
                      &window->backbuffers[i]->info.surface
                      ));

        if(surfaceType != gcvSURF_BITMAP)
        {
            gcmONERROR(
                    gcoSURF_SetFlags(
                    window->backbuffers[i]->info.surface,
                    gcvSURF_FLAG_CONTENT_YINVERTED,
                    gcvTRUE));
        }

        gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL
                    ));

        gcmONERROR(
                gcoSURF_GetAlignedSize(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL,
                    &window->backbuffers[i]->info.stride
                    ));

        gcmONERROR(
               gcoSURF_QueryVidMemNode(
                    window->backbuffers[i]->info.surface,
                    (gctUINT32 *)&window->backbuffers[i]->info.node,
                    &window->backbuffers[i]->info.pool,
                    &window->backbuffers[i]->info.bytes
                    ));

        gcmONERROR(
                gcoHAL_NameVideoMemory(gcmPTR2INT(window->backbuffers[i]->info.node),
                                       gcmINT2PTR(&window->backbuffers[i]->info.node)));

        window->backbuffers[i]->info.width = window->info->width;
        window->backbuffers[i]->info.height = window->info->height;
        window->backbuffers[i]->info.invalidate = gcvTRUE;
        window->backbuffers[i]->frame_callback = gcvNULL;
    }
    while (gcvFALSE);
    return status;
OnError:
    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
wayland_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
wayland_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static void
wl_buffer_release(void *data, struct wl_buffer *buffer)
{
    gcsWL_EGL_BUFFER* egl_buffer = data;
    pthread_mutex_lock(&registerMutex);
    if (_isValidEGLBuffer(egl_buffer))
    {
        egl_buffer->info.locked = gcvFALSE;
    }
    pthread_mutex_unlock(&registerMutex);
}

static struct wl_buffer_listener wl_buffer_listener = {
   wl_buffer_release
};

static void
gcoWL_FrameCallback(void *data, struct wl_callback *callback, uint32_t time)
{
    gcsWL_EGL_BUFFER* egl_buffer = data;
    pthread_mutex_lock(&registerMutex);
    if (_isValidEGLBuffer(egl_buffer))
    {
        egl_buffer->frame_callback = NULL;
        wl_callback_destroy(callback);
    }
    pthread_mutex_unlock(&registerMutex);
}

static const struct wl_callback_listener gcsWL_FRAME_LISTENER = {
    gcoWL_FrameCallback
};


gceSTATUS
wayland_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (Display == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    {
        /* Client */
        struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
        gcsWL_EGL_BUFFER* egl_buffer = (gcsWL_EGL_BUFFER*)Context;

        struct wl_buffer* wl_buffer = gcvNULL;
        gcsWL_EGL_DISPLAY* display = gcvNULL;

        int ret = 0;
        int i   = 0;

        if(_isValidWindow(Window) == gcvFALSE)
        {
            return gcvSTATUS_FALSE;
        }
        wl_buffer = egl_buffer->wl_buffer;
        display = wl_window->display;
        wl_display_dispatch_queue_pending(display->wl_display, display->wl_queue);

        /* wait for frame callback */
        for(i = 0; i < wl_window->info->bufferCount; i++)
        {
            while (wl_window->backbuffers[i]->frame_callback && ret >= 0)
            {
                ret = wl_display_dispatch_queue(display->wl_display, display->wl_swap_queue);
            }
        }
        if(ret < 0)
        {
            gcoOS_AtomDecrement(gcvNULL, wl_window->reference, gcvNULL);
            return gcvSTATUS_FALSE;
        }

        if(wl_window->reference == gcvNULL)
        {
             return status;
         }

        if (display->swapInterval > 0)
        {
            egl_buffer->frame_callback = wl_surface_frame(wl_window->surface);
            wl_callback_add_listener(egl_buffer->frame_callback, &gcsWL_FRAME_LISTENER, egl_buffer);
            wl_proxy_set_queue((struct wl_proxy *) egl_buffer->frame_callback, display->wl_swap_queue);
        }

        wl_surface_attach(wl_window->surface, wl_buffer, wl_window->info->dx, wl_window->info->dy);
        wl_window->attached_width  = wl_window->info->width;
        wl_window->attached_height = wl_window->info->height;
        wl_window->info->dx = 0;
        wl_window->info->dy = 0;
        wl_surface_damage(wl_window->surface, 0, 0,
                            wl_window->info->width, wl_window->info->height);
        wl_surface_commit(wl_window->surface);
        /* If we're not waiting for a frame callback then we'll at least throttle
         * to a sync callback so that we always give a chance for the compositor to
         * handle the commit and send a release event before checking for a free
         * buffer */
        if(egl_buffer->frame_callback == gcvNULL)
        {
            egl_buffer->frame_callback = wl_display_sync(display->wl_display);
            wl_callback_add_listener(egl_buffer->frame_callback, &gcsWL_FRAME_LISTENER, egl_buffer);
            wl_proxy_set_queue((struct wl_proxy *) egl_buffer->frame_callback, display->wl_swap_queue);
        }
        wl_display_flush(display->wl_display);
        if(wl_window->reference != gcvNULL )
        {
            gcoOS_AtomDecrement(gcvNULL, wl_window->reference, gcvNULL);
        }

        return status;
    }
}

gceSTATUS
wayland_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    /* VIV: Quick fix. */
    return wayland_SetDisplayVirtualEx(Display, Window, Context, Surface, Offset, X, Y);
}

gceSTATUS
wayland_SetSwapIntervalEx(
    IN PlatformDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay
    )
{
    gcsWL_EGL_DISPLAY* egl_display;

    if (Display == gcvNULL)
    {
        /* Invalid display pointer. */
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* clamp to min and max */
    if (Interval > 10)
    {
        Interval = 10;
    }
    else if (Interval < 0)
    {
        Interval = 0;
    }

    egl_display = ((gcsWL_EGL_DISPLAY*) localDisplay);
    egl_display->swapInterval = Interval;
    return gcvSTATUS_OK;
}

gceSTATUS
wayland_GetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Min=0x%x Max=0x%x", Display, Min, Max);

    if (Display == gcvNULL)
    {
        /* Invalid display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if( Min != gcvNULL)
    {
        *Min = 0;
    }

    if (Max != gcvNULL)
    {
        *Max = 10;
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;

}

gceSTATUS
wayland_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
wayland_DisplayBufferRegions(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Windows. ********************************************************************
*/

gceSTATUS
wayland_CreateWindow(
    IN PlatformDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT PlatformWindowType * Window
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctCHAR *           p;

    gcmHEADER_ARG("Display=%p X=%d Y=%d Width=%d Height=%d",
                  Display, X, Y, Width, Height);

    {
        gctUINT i;
        struct wl_egl_window* wl_window = (struct wl_egl_window*)(*Window);

        gcoOS_AllocateMemory(gcvNULL, sizeof(struct _gcsWL_EGL_WINDOW_INFO),
                            (gctPOINTER) &wl_window->info);
        gcoOS_ZeroMemory( wl_window->info, sizeof(struct _gcsWL_EGL_WINDOW_INFO));

        wl_window->info->bufferCount =  WL_EGL_NUM_BACKBUFFERS;
        wl_window->info->width  = Width;
        wl_window->info->height = Height;
        wl_window->info->format = gcvSURF_A8R8G8B8;

        gcoOS_AtomConstruct(gcvNULL, &wl_window->reference);
        wl_window->wl_signature = WL_CLIENT_SIGNATURE;
        pthread_mutex_init(&(wl_window->window_mutex), gcvNULL);

        p = getenv("GPU_VIV_WL_MULTI_BUFFER");
        if (p != gcvNULL)
        {
            int bufferCount = atoi(p);
            if (bufferCount > 0 && bufferCount <= WL_EGL_MAX_NUM_BACKBUFFERS)
            {
                wl_window->info->bufferCount = bufferCount;
            }
        }
        gcoOS_AllocateMemory(gcvNULL, wl_window->info->bufferCount * sizeof(struct _gcsWL_EGL_BUFFER *),
                    (gctPOINTER) &wl_window->backbuffers);
        for(i=0; i< wl_window->info->bufferCount; i++)
        {
            gcoOS_AllocateMemory(gcvNULL, sizeof(struct _gcsWL_EGL_BUFFER),
                    (gctPOINTER) &wl_window->backbuffers[i]);
            gcoOS_ZeroMemory( wl_window->backbuffers[i], sizeof(struct _gcsWL_EGL_BUFFER));
            wl_window->backbuffers[i]->wl_signature = WL_CLIENT_SIGNATURE;
        }
        registerWindow(wl_window);
        gcmFOOTER();
        return status;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
wayland_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Window,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcmHEADER_ARG("localDisplay=%p Width=%d Height=%d",
                  localDisplay, Width, Height);

    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    {
        struct wl_egl_window* window = (struct wl_egl_window*)Window;
        /* Nothing to do if window size if same. */
        if(window->info->width != Width || window->info->height != Height)
        {

            wait_for_the_frame_finish(window);
            window->info->width = Width;
            window->info->height = Height;
        }
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
wayland_GetWindowInfo(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct wl_egl_window* wl_window = gcvNULL;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    wl_window = (struct wl_egl_window*) Window;

    {
        /* Client window */
        if (X != gcvNULL)
        {
            *X = 0;
        }

        if (Y != gcvNULL)
        {
            *Y = 0;
        }

        if (Width != gcvNULL)
        {
            *Width = wl_window->info->width;
        }

        if (Height != gcvNULL)
        {
            *Height = wl_window->info->height;
        }

        if (BitsPerPixel != gcvNULL)
        {
            *BitsPerPixel = wl_window->info->bpp;
        }

        if (Offset != gcvNULL)
        {
            *Offset = 0;
        }
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
wayland_DestroyWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    {
        struct wl_egl_window* window = (struct wl_egl_window*)Window;
        gctUINT i;
        gcoWL_DestroryBO(window);
        unRegisterWindow(window);
        pthread_mutex_lock(&(window->window_mutex));
        for(i=0; i< window->info->bufferCount; i++)
        {
            if (window->backbuffers[i]->wl_buffer != gcvNULL)
            {
                wl_buffer_destroy(window->backbuffers[i]->wl_buffer);
                window->backbuffers[i]->wl_buffer = gcvNULL;
            }



            gcoOS_FreeMemory(gcvNULL, window->backbuffers[i]);
            window->backbuffers[i] = gcvNULL;
        }

        pthread_mutex_unlock(&(window->window_mutex));
        pthread_mutex_destroy(&(window->window_mutex));
        gcoOS_FreeMemory(gcvNULL, window->backbuffers);
        gcoOS_FreeMemory(gcvNULL, window->info);
        window->info = gcvNULL;
        window->backbuffers = gcvNULL;
        window->wl_signature = 0;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
wayland_DrawImage(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
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
wayland_GetImage(
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    OUT gctINT * BitsPerPixel,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static void
gcoWL_DestroyBuffer(struct wl_resource *resource)
{
    gcsWL_VIV_BUFFER *buffer = resource->data;

    if (buffer != gcvNULL)
    {
        gcoSURF surface = buffer->surface;

        if (surface)
        {
            gcoSURF_Unlock(surface, gcvNULL);
            gcoSURF_Destroy(surface);
        }

        gcoOS_FreeMemory(gcvNULL, buffer);
    }
}

static void
gcoWL_BufferDestroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface gcsWL_BUFFER_IMPLEMENTATION = {
    gcoWL_BufferDestroy
};

static void
gcoWL_ImportBuffer(struct wl_client *client,
                      struct wl_resource *resource,
                      uint32_t id,
                      uint32_t width,
                      uint32_t height,
                      uint32_t stride,
                      int32_t format,
                      int32_t type,
                      int32_t node,
                      int32_t pool,
                      uint32_t bytes
                      )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface = gcvNULL;
    gcsWL_VIV_BUFFER * wl_viv_buffer = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gcmTRACE(gcvLEVEL_VERBOSE, "Ghost buffer width=%d, height=%d, stride=%d, format=%d, node=%p, pool=%d, bytes=%d",
                    width, height, stride, format, node, pool, bytes);
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    gcmONERROR(
        gcoSURF_Construct(gcvNULL,
                          (gctUINT) width,
                          (gctUINT) height,
                          1,
                          type | gcvSURF_NO_VIDMEM,
                          (gceSURF_FORMAT) format,
                          (gcePOOL) pool,
                          &surface));

    /* Save surface info. */
    surface->node.u.normal.node = (gctUINT32) node;
    surface->node.pool          = (gcePOOL) pool;
    surface->node.size          = (gctSIZE_T) bytes;
/*
    gcmONERROR(
        gcoSURF_SetOrientation(surface,
                               gcvORIENTATION_BOTTOM_TOP));
*/

    gcmONERROR(gcoHAL_ImportVideoMemory(
        (gctUINT32)node, (gctUINT32 *)&surface->node.u.normal.node));

    /* Lock once as it's done in gcoSURF_Construct with vidmem. */
    gcmONERROR(
        gcoSURF_Lock(surface,
                     gcvNULL,
                     gcvNULL));

    gcmONERROR(
        gcoOS_AllocateMemory(gcvNULL,
                             gcmSIZEOF(*wl_viv_buffer),
                             (gctPOINTER *) &wl_viv_buffer));

    gcoOS_ZeroMemory(wl_viv_buffer,
                         gcmSIZEOF(*wl_viv_buffer));

    wl_viv_buffer->surface = surface;
    wl_viv_buffer->width = width;
    wl_viv_buffer->height = height;

    wl_viv_buffer->wl_buffer = wl_resource_create(client, &wl_buffer_interface, 1, id);
    if (!wl_viv_buffer->wl_buffer) {
        wl_resource_post_no_memory(resource);
        free(wl_viv_buffer);
        return;
    }
    wl_resource_set_implementation(wl_viv_buffer->wl_buffer,
                       &gcsWL_BUFFER_IMPLEMENTATION,
                       wl_viv_buffer, gcoWL_DestroyBuffer);
    gcoHAL_SetHardwareType(gcvNULL, currentType);
    return;

OnError:
    wl_resource_post_no_memory(resource);
    if (surface)
    {
        gcoSURF_Unlock(surface, gcvNULL);
        gcoSURF_Destroy(surface);
        surface = gcvNULL;
    }

    if (wl_viv_buffer != gcvNULL)
    {
        gcoOS_FreeMemory(gcvNULL, wl_viv_buffer);
        wl_viv_buffer = gcvNULL;
    }

    return;
}

struct wl_viv_interface gcsWL_VIV_IMPLEMENTATION = {
    (void *) gcoWL_ImportBuffer
};

static void
gcoWL_BindWLViv(struct wl_client *client,
         void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_viv_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &gcsWL_VIV_IMPLEMENTATION, data, NULL);
}

gceSTATUS
wayland_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct wl_display *display = Display;
    gcsWL_EGL_DISPLAY *egl_display = gcvNULL;

    egl_display = gcoWL_GetDisplay(display);

    if (egl_display == gcvNULL)
    {
        return gcvSTATUS_NOT_FOUND;
    }

    egl_display->swapInterval = 1;
    *localDisplay = egl_display;

    return status;
}

gceSTATUS
wayland_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (localDisplay && (*localDisplay == gcvNULL))
    {
        /*The localDisplay was not initialized in FB app */
        return status;
    }

    {
        gcsWL_EGL_DISPLAY *egl_display = *localDisplay;
        struct wl_egl_window* wl_window;

        if (WLEGLWindowList.next != gcvNULL)
        {
            pthread_mutex_lock(&registerMutex);
            wl_list_for_each(wl_window, &WLEGLWindowList, link)
            {
                if (wl_window->display == egl_display)
                     wl_window->display = gcvNULL;
            }

            pthread_mutex_unlock(&registerMutex);
        }

        gcoWL_ReleaseDisplay(*localDisplay);
    }

    return status;
}


gceSTATUS
wayland_GetDisplayInfoEx2(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT waylandDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status;

    status = wayland_GetDisplayInfoEx(Display, Window, DisplayInfoSize, DisplayInfo);

    if (gcmIS_SUCCESS(status))
    {
        if ((DisplayInfo->logical == gcvNULL) || (DisplayInfo->physical == ~0U))
        {
            /* No offset. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
    }

    return status;
}

static gctBOOL
gcoWL_GetBackbuffer(
    IN PlatformWindowType Window
    )
{
    int i = 0;
    struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
    for(i = 0; i < wl_window->info->bufferCount; i++)
    {
       if(wl_window->backbuffers[i]->info.locked == gcvFALSE)
       {
           wl_window->info->current = i;
           if(wl_window->info->bufferCount > 1)
           {
               wl_window->backbuffers[i]->info.locked = gcvTRUE;
           }
           return gcvTRUE;
       }
    }
    return gcvFALSE;
}


static void
gcoWL_CreateWLBuffer(
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay
    )
{
    unsigned int i =0;
    struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
    i = wl_window->info->current;

    gcsWL_EGL_DISPLAY* display = ((gcsWL_EGL_DISPLAY*) localDisplay);
    pthread_mutex_lock(&(wl_window->window_mutex));
    struct wl_buffer* wl_buffer = wl_window->backbuffers[i]->wl_buffer;

    if(wl_buffer)
    {
        wl_buffer_destroy(wl_buffer);
    }

    gcoWL_CreateGhostBuffer(display, wl_window->backbuffers[i]);
    wl_buffer = wl_window->backbuffers[i]->wl_buffer;
    wl_proxy_set_queue((struct wl_proxy *) wl_buffer, display->wl_queue);
    wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, wl_window->backbuffers[i]);
    wl_window->backbuffers[i]->info.invalidate = gcvFALSE;
    pthread_mutex_unlock(&(wl_window->window_mutex));
}

gceSTATUS
wayland_GetDisplayBackbufferEx(
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
    gceSTATUS status = gcvSTATUS_OK;

    {
        /* Client */
        int ret = 0;
        struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
        gcsWL_EGL_DISPLAY* display = ((gcsWL_EGL_DISPLAY*) localDisplay);
        wl_window->display         = display;

        if(wl_window->info->bpp == 0)
        {
            status = gcoWL_CreateBO(wl_window);
        }

        while ( !gcoWL_GetBackbuffer(Window) && ret != -1)
        {
            ret = wl_display_dispatch_queue(display->wl_display, display->wl_queue);
        }
        if (ret < 0)
            return gcvSTATUS_INVALID_ARGUMENT;

        if( wl_window->backbuffers[wl_window->info->current]->info.width != wl_window->info->width
            || wl_window->backbuffers[wl_window->info->current]->info.height != wl_window->info->height)
        {
            gcoWL_ResizeBO(wl_window);
        }

        if (wl_window->backbuffers[wl_window->info->current]->info.invalidate == gcvTRUE)
        {
           gcoWL_CreateWLBuffer(Window, localDisplay);
        }

        *context = wl_window->backbuffers[wl_window->info->current];
        *surface = wl_window->backbuffers[wl_window->info->current]->info.surface;
        *Offset  = 0;
        *X       = 0;
        *Y       = 0;

        if(wl_window->reference != gcvNULL )
        {
            gcoOS_AtomIncrement(gcvNULL, wl_window->reference, gcvNULL);
        }
    }

    return status;
}

gctBOOL
wayland_SynchronousFlip(
    IN PlatformDisplayType Display
    )
{
    gctBOOL syncFlip = gcvFALSE;

    {
        gctCHAR *           p;
        p = getenv("GPU_VIV_WL_MULTI_BUFFER");

        if (p != gcvNULL)
        {
            gctINT bufferCount = atoi(p);
            if (bufferCount == 1)
            {
                syncFlip = gcvTRUE;
            }
        }
    }

    return syncFlip;
}

gceSTATUS
wayland_GetNativeVisualId(
    IN PlatformDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
wayland_GetWindowInfoEx(
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
    waylandDISPLAY_INFO info;

    if (gcmIS_ERROR(wayland_GetWindowInfo(
                          Display,
                          Window,
                          X,
                          Y,
                          (gctINT_PTR) Width,
                          (gctINT_PTR) Height,
                          (gctINT_PTR) BitsPerPixel,
                          gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcmIS_ERROR(wayland_GetDisplayInfoEx(
        Display,
        Window,
        sizeof(info),
        &info)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    {
        struct wl_egl_window* wl_window = Window;
        *Format = wl_window->info->format;
        if (Type != gcvNULL)
        {
            *Type = gcvSURF_BITMAP;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
wayland_DrawImageEx(
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
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
wayland_CreateDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
wayland_DestroyDrawable(
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
    /* No default display support on wayland. */
    fprintf(stderr, "EGL: Warning: No default display support on wayland\n");
    return NULL;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (!Display)
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

    status = wayland_InitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    status = wayland_DeinitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    wayland_GetNativeVisualId((PlatformDisplayType) Display->hdc, &visualId);
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
    status = wayland_GetSwapInterval((PlatformDisplayType) Display->hdc,
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

    /* Wayland specific. */
    status = wayland_SetSwapIntervalEx((PlatformDisplayType) Display->hdc,
                                      Interval,
                                      Display->localInfo);

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

    /* Information obtained by wayland_GetDisplayInfoEx2. */
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
    VEGLNativeBuffer buffer;

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

                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): buffer[%d]: yoffset=%-4d physical=%x",
                         __FUNCTION__, __LINE__,
                         i, alignedHeight * i, physical);
            }

            gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
        }
        else
        {
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
    waylandDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = wayland_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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

    /* Get display information. */
    gcoOS_ZeroMemory(&dInfo, sizeof (waylandDISPLAY_INFO));

    status = wayland_GetDisplayInfoEx2((PlatformDisplayType) Display->hdc,
                                     (PlatformWindowType) Window,
                                     Display->localInfo,
                                     sizeof (waylandDISPLAY_INFO),
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
        Info->multiBuffer  = dInfo.multiBuffer > 1 ? dInfo.multiBuffer : 1;
    }

    /* Get virtual size. */
    status = wayland_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
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

    {
        gceSURF_FORMAT configFormat = gcvSURF_UNKNOWN;

        switch (Surface->config.greenSize)
        {
        case 4:
            configFormat = (Surface->config.alphaSize == 0)
                ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
            break;
        case 5:
            configFormat = (Surface->config.alphaSize == 0)
                ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
            break;
        case 6:
            configFormat = gcvSURF_R5G6B5;
            break;
        case 8:
            configFormat = (Surface->config.alphaSize == 0)
                ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
            break;
        default:
            return EGL_FALSE;
        }

        wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
                            (PlatformWindowType) win,
                            gcvSURF_BITMAP,
                            configFormat);
    }

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
    wayland_CreateDrawable(Display->localInfo, (PlatformWindowType) win);

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

    wayland_DestroyDrawable(Display->localInfo, (PlatformWindowType) win);

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

    status = wayland_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
            status = wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                    status = wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

                    status = wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                status = wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
            status = wayland_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

    status = wayland_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
        status = wayland_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
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

        status = wayland_SetDisplayVirtualEx((PlatformDisplayType) Display->hdc,
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
            status = wayland_DrawImageEx((PlatformDisplayType) Display->hdc,
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

    status = wayland_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
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
    return wayland_SynchronousFlip((PlatformDisplayType)Display->hdc);
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

#define WL_EGL_PIXMAP_SIGNATURE     (('w'<<24) | ('l'<<16) | ('p'<<8) | ('x'))

struct wl_egl_pixmap
{
    uint32_t    signature;

    /* Parameters. */
    int         width;
    int         height;
    int         format;

    /* Underlying information. */
    int         stride;
    void        *data;

    gcoSURF     surface;
};


struct eglPixmapInfo
{
    /* Keep a reference of the surface. */
    gcoSURF             surface;
};

static EGLBoolean
_MatchPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN struct eglConfig * Config
    )
{
    struct wl_egl_pixmap *pixmap;

    pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    /* Check if format is matched. */
    switch (pixmap->format)
    {
    case WL_SHM_FORMAT_RGB565:
        if ((Config->redSize   == 5) &&
            (Config->greenSize == 6) &&
            (Config->blueSize  == 5))
        {
            return EGL_TRUE;
        }
        break;

    case WL_SHM_FORMAT_XRGB8888:
    case WL_SHM_FORMAT_XBGR8888:
        if ((Config->redSize   == 8) &&
            (Config->greenSize == 8) &&
            (Config->blueSize  == 8) &&
            (Config->alphaSize == 0))
        {
            return EGL_TRUE;
        }
        break;
    case WL_SHM_FORMAT_ARGB8888:
    case WL_SHM_FORMAT_ABGR8888:
        if ((Config->redSize   == 8) &&
            (Config->greenSize == 8) &&
            (Config->blueSize  == 8) &&
            (Config->alphaSize == 8))
        {
            return EGL_TRUE;
        }
        break;

    default:
        break;
    }

    return EGL_FALSE;
}

static EGLBoolean
_ConnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    )
{
    struct wl_egl_pixmap *pixmap;
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;
    VEGLPixmapInfo info;

    pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    /* Allocate memory. */
    status = gcoOS_Allocate(gcvNULL,
                            sizeof (struct eglPixmapInfo),
                            &pointer);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    gcoOS_ZeroMemory(pointer, sizeof (struct eglPixmapInfo));
    info = pointer;

    /* Save pixmap info. */
    info->surface = pixmap->surface;

    /* Reference surface. */
    gcoSURF_ReferenceSurface(pixmap->surface);

    /* Output. */
    *Info    = info;
    *Surface = pixmap->surface;

    return EGL_TRUE;
}

static EGLBoolean
_DisconnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    struct wl_egl_pixmap *pixmap;

    /* Free pixmap wrapper. */
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p",
             __FUNCTION__, __LINE__, Display, Pixmap);

    pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE) ||
        (pixmap->surface != Info->surface))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    if (Info->surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->surface));
        Info->surface = gcvNULL;
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
    struct wl_egl_pixmap * pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE) ||
        (pixmap->surface != Info->surface))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    if (Width)
    {
        *Width = pixmap->width;
    }

    if (Height)
    {
        *Height = pixmap->height;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SyncFromPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    struct wl_egl_pixmap *pixmap;

    pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE) ||
        (pixmap->surface != Info->surface))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    /* Just need to flush CPU cache. */
    gcmVERIFY_OK(
        gcoSURF_CPUCacheOperation(pixmap->surface, gcvCACHE_FLUSH));

    return EGL_TRUE;
}

static EGLBoolean
_SyncToPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    struct wl_egl_pixmap *pixmap;

    pixmap = (struct wl_egl_pixmap *) Pixmap;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE) ||
        (pixmap->surface != Info->surface))
    {
        fprintf(stderr, "%s: invalid pixmap=%p\n", __func__, pixmap);
        return EGL_FALSE;
    }

    /* Nothing to do. */
    return EGL_TRUE;
}


static struct eglPlatform waylandPlatform =
{
    EGL_PLATFORM_WAYLAND_KHR,

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
veglGetWaylandPlatform(
    void * NativeDisplay
    )
{
    return &waylandPlatform;
}

EGLBoolean
veglBindWaylandDisplay(
    VEGLDisplay Display,
    struct wl_display * Dpy
    )
{
    struct wl_global * wl_global;

    if (Dpy != NULL)
    {
        wl_global = wl_global_create(Dpy,
                                     &wl_viv_interface,
                                     1,
                                     NULL,
                                     gcoWL_BindWLViv);

        Display->wl_global = wl_global;
    }

    return EGL_TRUE;
}

EGLBoolean
veglUnbindWaylandDisplay(
    VEGLDisplay Display,
    struct wl_display * Dpy
    )
{
    if (Display->wl_global)
    {
        wl_global_destroy((struct wl_global *) Display->wl_global);
        Display->wl_global = NULL;
    }

    return EGL_TRUE;
}

EGLBoolean
veglQueryWaylandBuffer(
    VEGLDisplay Display,
    struct wl_resource *Buffer,
    EGLint * Width,
    EGLint * Height,
    gcoSURF * Surface
    )
{
    gcsWL_VIV_BUFFER *buf;
    buf = wl_resource_get_user_data(Buffer);

    if (Width) {
        *Width = buf->width;
    }

    if (Height) {
        *Height = buf->height;
    }

    if (Surface) {
        *Surface = buf->surface;
    }

    return EGL_TRUE;
}



/******************************************************************************/

static void
gcoWL_QueryResolveFeature(int* noResolve)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR * p         = gcvNULL;

    VEGLThreadData thread;
    do
    {
        if(noResolve == NULL)
        {
            break;
        }
        *noResolve = 1;

        p = getenv("GPU_VIV_EXT_RESOLVE");
        if ((p != gcvNULL) && (p[0] == '0'))
        {
            /* Disable no-resolve requested. */
            *noResolve = 0;
            break;
        }

        p = getenv("GPU_VIV_DISABLE_SUPERTILED_TEXTURE");
        if ((p != gcvNULL) && (p[0] == '1'))
        {
            /* Disable no-resolve while supertiled texture was disabled. */
            *noResolve = 0;
            break;
        }

        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_SUPERTILED_TEXTURE
            );

        if (status != gcvSTATUS_TRUE)
        {
            *noResolve = 0;
            break;
        }
        thread = veglGetThreadData();
        if(thread->api == EGL_OPENVG_API)
        {
            /* OpenVG cannot support direct rendering.*/
            *noResolve = 0;
        }
    }
    while (gcvFALSE);
    return;
}

struct wl_egl_window * wl_egl_window_create(struct wl_surface *surface, int width, int height)
{
    struct wl_egl_window * window = gcvNULL;

    window = (struct wl_egl_window *) malloc(sizeof (struct wl_egl_window));
    if (!window)
    {
        return NULL;
    }

    memset(window, 0, sizeof (struct wl_egl_window));
    window->surface = surface;
    gcoWL_QueryResolveFeature(&window->noResolve);

    if (gcmIS_ERROR(wayland_CreateWindow(NULL, 0, 0, width, height, &window)))
    {
        free(window);
        return NULL;
    }

    return window;
}

void wl_egl_window_destroy(struct wl_egl_window *window)
{
    wayland_DestroyWindow(gcvNULL, window);
    free(window);
}

void wl_egl_window_resize(struct wl_egl_window *window, int width, int height, int dx, int dy)
{
   if (window->info->width != width || window->info->height != height)
   {
       VEGLThreadData thread;
       wayland_ResizeWindow(gcvNULL, window, width, height);

       /* Get thread data. */
       thread = veglGetThreadData();

       if (thread != gcvNULL && thread->context != gcvNULL)
       {
           veglResizeSurface(thread->context->display,
                             thread->context->draw,
                             width, height);
       }
   }
}

void wl_egl_window_get_attached_size(struct wl_egl_window *window,int *width, int *height)
{
    if (width)
    {
        *width = window->attached_width;
    }

    if (height)
    {
        *height = window->attached_height;
    }
}

static int create_pixmap_content(struct wl_egl_pixmap *pixmap)
{
    gceSTATUS status;
    gceSURF_FORMAT format;
    gcoSURF surface = gcvNULL;
    gctINT stride = 0;
    gctPOINTER memory[3] = {gcvNULL, gcvNULL, gcvNULL};

    switch (pixmap->format)
    {
    case WL_SHM_FORMAT_XRGB4444:
        format = gcvSURF_X4R4G4B4;
        break;
    case WL_SHM_FORMAT_XBGR4444:
        format = gcvSURF_X4B4G4R4;
        break;
    case WL_SHM_FORMAT_ARGB4444:
        format = gcvSURF_A4R4G4B4;
        break;
    case WL_SHM_FORMAT_ABGR4444:
        format = gcvSURF_A4B4G4R4;
        break;
    case WL_SHM_FORMAT_XRGB1555:
        format = gcvSURF_X1R5G5B5;
        break;
    case WL_SHM_FORMAT_XBGR1555:
        format = gcvSURF_X1B5G5R5;
        break;
    case WL_SHM_FORMAT_ARGB1555:
        format = gcvSURF_A1R5G5B5;
        break;
    case WL_SHM_FORMAT_ABGR1555:
        format = gcvSURF_A1B5G5R5;
        break;
    case WL_SHM_FORMAT_RGB565:
        format = gcvSURF_R5G6B5;
        break;
    case WL_SHM_FORMAT_BGR565:
        format = gcvSURF_B5G6R5;
        break;
    case WL_SHM_FORMAT_XRGB8888:
        format = gcvSURF_X8R8G8B8;
        break;
    case WL_SHM_FORMAT_XBGR8888:
        format = gcvSURF_X8B8G8R8;
        break;
    case WL_SHM_FORMAT_ARGB8888:
        format = gcvSURF_A8R8G8B8;
        break;
    case WL_SHM_FORMAT_ABGR8888:
        format = gcvSURF_A8B8G8R8;
        break;

    case WL_SHM_FORMAT_YUYV:
        format = gcvSURF_YUY2;
        break;
    case WL_SHM_FORMAT_YVYU:
        format = gcvSURF_YVYU;
        break;
    case WL_SHM_FORMAT_UYVY:
        format = gcvSURF_UYVY;
        break;
    case WL_SHM_FORMAT_VYUY:
        format = gcvSURF_VYUY;
        break;

    default:
        fprintf(stderr, "%s: format not supported - 0x%x\n",
                __func__, pixmap->format);
        return -EINVAL;
    }

    gcmONERROR(gcoSURF_Construct(gcvNULL,
                                 (gctUINT) pixmap->width,
                                 (gctUINT) pixmap->height,
                                 1,
                                 gcvSURF_CACHEABLE_BITMAP,
                                 format,
                                 gcvPOOL_DEFAULT,
                                 &surface));

    gcmONERROR(gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, &stride));

    /* Lock the surface. */
    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, memory));

    pixmap->stride  = stride;
    pixmap->data    = memory[0];
    pixmap->surface = surface;
    return 0;

OnError:
    if (memory[0] != gcvNULL)
    {
        gcoSURF_Unlock(surface, gcvNULL);
    }

    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
        gcoHAL_Commit(gcvNULL, gcvFALSE);
    }

    return -EINVAL;
}

struct wl_egl_pixmap *wl_egl_pixmap_create(int width, int height, int format)
{
    struct wl_egl_pixmap *pixmap;

    if (width <= 0 || height <= 0)
        return NULL;

    pixmap = malloc(sizeof *pixmap);
    if (!pixmap)
        return NULL;

    pixmap->signature = WL_EGL_PIXMAP_SIGNATURE;
    pixmap->width     = width;
    pixmap->height    = height;
    pixmap->format    = format;

    pixmap->stride   = 0;
    pixmap->data     = 0;
    pixmap->surface  = NULL;

    if (create_pixmap_content(pixmap) != 0)
    {
        free(pixmap);
        pixmap = 0;
    }

    return pixmap;
}

void wl_egl_pixmap_destroy(struct wl_egl_pixmap *pixmap)
{
    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE))
    {
        /* Bad pixmap. */
        return;
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(pixmap->surface, gcvNULL));

    /* Destroy the surface. */
    gcmVERIFY_OK(gcoSURF_Destroy(pixmap->surface));

    pixmap->signature = 0;
    free(pixmap);
}

void wl_egl_pixmap_get_stride(struct wl_egl_pixmap *pixmap, int *stride)
{
    if (stride)
        *stride = pixmap->stride;
}

void wl_egl_pixmap_lock(struct wl_egl_pixmap *pixmap, void **pixels)
{
    if (pixels)
        *pixels = pixmap->data;
}

void wl_egl_pixmap_unlock(struct wl_egl_pixmap *pixmap)
{
}

