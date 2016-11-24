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
#include <poll.h>
#include <assert.h>

#include <pthread.h>

#include <wayland-viv-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h>

#include <gc_hal_user.h>

#include "gc_egl_platform.h"
#include <gc_egl_precomp.h>

#define _GC_OBJ_ZONE    gcvZONE_OS

#define WL_EGL_NUM_BACKBUFFERS          3
#define WL_EGL_MAX_NUM_BACKBUFFERS      8

struct wl_egl_display
{
    struct wl_display * wl_dpy;
    struct wl_viv * wl_viv;
    struct wl_registry * registry;
    struct wl_event_queue * wl_queue;
    int swap_interval;
};

struct wl_egl_window
{
    uint32_t signature;

    struct wl_egl_display * display;

    struct wl_egl_buffer * buffers;
    int nr_buffers;

    int32_t dx;
    int32_t dy;
    int32_t width;
    int32_t height;

    gceSURF_FORMAT format;
    gceSURF_TYPE type;

    int32_t attached_width;
    int32_t attached_height;

    struct wl_surface * surface;
    struct wl_list link;
};

enum wl_egl_buffer_state
{
    BUFFER_STATE_FREE = 0,
    BUFFER_STATE_DEQUEUED,
    BUFFER_STATE_QUEUED,
};

struct wl_egl_buffer
{
    struct
    {
        gctINT32 width;
        gctINT32 height;
        gctINT32 stride;
        gceSURF_FORMAT format;
        gceSURF_TYPE   type;
        gctUINT32 node;
        gcePOOL pool;
        gctSIZE_T size;
        gcoSURF surface;
        gceHARDWARE_TYPE hwType;
    } info;

    volatile int state;
    struct wl_callback * frame_callback;
    struct wl_buffer * wl_buf;
    struct wl_list link;
};

static struct wl_list egl_window_list;
static pthread_mutex_t egl_window_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void once_init(void)
{
    wl_list_init(&egl_window_list);
}

static inline int
poll_event(struct wl_display *wl_dpy, short int events, int timeout)
{
    int ret;
    struct pollfd pfd[1];

    pfd[0].fd = wl_display_get_fd(wl_dpy);
    pfd[0].events = events;

    do
    {
        ret = poll(pfd, 1, timeout);
    }
    while (ret == -1 && errno == EINTR);

    return ret;
}

/*
 * Our optimized dispatch_queue, with timeout.
 * Use timeount to avoid deadlock in multi-threaded environment.
 * Returns the number of dispatched events on success or -1 on failure.
 */
static int
dispatch_queue(struct wl_display *wl_dpy,
                struct wl_event_queue *wl_queue, int timeout)
{
    int ret;

    if (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
        return wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

    for (;;)
    {
        ret = wl_display_flush(wl_dpy);

        if (ret != -1 || errno != EAGAIN)
            break;

        if (poll_event(wl_dpy, POLLOUT, -1) == -1)
        {
            wl_display_cancel_read(wl_dpy);
            return -1;
        }
    }

    /* Don't stop if flushing hits an EPIPE; continue so we can read any
     * protocol error that may have triggered it. */
    if (ret < 0 && errno != EPIPE)
    {
        wl_display_cancel_read(wl_dpy);
        return -1;
    }

    ret = poll_event(wl_dpy, POLLIN, timeout);

    /* cancel read when on error or timeout. */
    if (ret == -1 || ret == 0)
    {
        wl_display_cancel_read(wl_dpy);
        return ret;
    }

    if (wl_display_read_events(wl_dpy) == -1)
        return -1;

    return wl_display_dispatch_queue_pending(wl_dpy, wl_queue);
}

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
    int *done = data;

    *done = 1;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
    sync_callback
};

/*
 * Our optimized roundtrip_queue. It's thread-safe.
 * Return -1 on error, 0 on success.
 */
static int
roundtrip_queue(struct wl_display *wl_dpy, struct wl_event_queue *wl_queue)
{
    struct wl_callback *callback;
    int done, ret = 0;

    done = 0;

    /*
     * This is to block read & dispatch events in other threads, so that the
     * callback is with correct queue and listener when 'done' event.
     */
    while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
        wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

    callback = wl_display_sync(wl_dpy);

    if (callback == NULL)
    {
        wl_display_cancel_read(wl_dpy);
        return -1;
    }

    wl_proxy_set_queue((struct wl_proxy *) callback, wl_queue);
    wl_callback_add_listener(callback, &sync_listener, &done);

    wl_display_cancel_read(wl_dpy);

    while (!done && ret >= 0)
        ret = dispatch_queue(wl_dpy, wl_queue, 5);

    if (ret == -1 && !done)
        wl_callback_destroy(callback);

    return ret;
}

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
    struct wl_egl_display *display = data;

    if (strcmp(interface, "wl_viv") == 0 && display)
    {
        display->wl_viv = wl_registry_bind(registry, name, &wl_viv_interface, 1);
        wl_proxy_set_queue((struct wl_proxy *)display->wl_viv, display->wl_queue);
    }
}

static const struct wl_registry_listener registry_listener = {
    /**
     * global - announce global object
     * @name: (none)
     * @interface: (none)
     * @version: (none)
     *
     * Notify the client of global objects.
     *
     * The event notifies the client that a global object with the
     * given name is now available, and it implements the given version
     * of the given interface.
     */
    registry_handle_global
};

static struct wl_egl_display *
wl_egl_display_create(struct wl_display *wl_dpy)
{
    struct wl_egl_display *display;

    display = malloc(sizeof(*display));

    if (!display)
    {
        /* Out of memory. */
        return NULL;
    }

    memset(display, 0, sizeof(*display));

    display->wl_dpy   = wl_dpy;
    display->wl_queue = wl_display_create_queue(wl_dpy);
    display->registry = wl_display_get_registry(wl_dpy);

    wl_proxy_set_queue((struct wl_proxy *)display->registry, display->wl_queue);
    wl_registry_add_listener(display->registry, &registry_listener, display);

    roundtrip_queue(display->wl_dpy, display->wl_queue);

    return display;
}

static void
wl_egl_display_destroy(struct wl_egl_display *display)
{
    struct wl_egl_window *window;

    pthread_once(&once_control, once_init);

    /*
     * Need destroy all 'display' relevant window resources, window destroying
     * may be after this, without display resource.
     */
    pthread_mutex_lock(&egl_window_mutex);

    /*
     * Display may be destroyed before window destroying.
     * Make sure window buffers are synced in this case.
     */
    wl_list_for_each(window, &egl_window_list, link)
    {
        int i;

        if (window->display != display)
            continue;


        for (i = 0; i < window->nr_buffers; i++)
        {
            int ret = 0;
            struct wl_egl_buffer *buffer = &window->buffers[i];

            while (buffer->frame_callback && ret != -1)
                ret = dispatch_queue(display->wl_dpy, display->wl_queue, 5);
        }

        window->display = NULL;
    }

    pthread_mutex_unlock(&egl_window_mutex);

    roundtrip_queue(display->wl_dpy, display->wl_queue);

    wl_registry_destroy(display->registry);
    wl_viv_destroy(display->wl_viv);
    wl_event_queue_destroy(display->wl_queue);

    free(display);
}

static inline int
wl_egl_window_validate(struct wl_egl_window *window)
{
    struct wl_egl_window *win;

    pthread_once(&once_control, once_init);

    pthread_mutex_lock(&egl_window_mutex);

    wl_list_for_each(win, &egl_window_list, link)
    {
        if (win == window)
        {
            pthread_mutex_unlock(&egl_window_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&egl_window_mutex);

    return -EINVAL;
}

static void
wl_egl_window_register(struct wl_egl_window *window)
{
    pthread_once(&once_control, once_init);

    pthread_mutex_lock(&egl_window_mutex);

    wl_list_insert(&egl_window_list, &window->link);

    pthread_mutex_unlock(&egl_window_mutex);
}

static void
wl_egl_window_unregister(struct wl_egl_window *window)
{
    pthread_mutex_lock(&egl_window_mutex);

    wl_list_remove(&window->link);

    pthread_mutex_unlock(&egl_window_mutex);
}

static void
buffer_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time)
{
    struct wl_egl_buffer *buffer = (struct wl_egl_buffer *)data;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

    wl_callback_destroy(callback);

    /* Switch to hardware type when buffer allocation. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);
    gcoHAL_SetHardwareType(gcvNULL, buffer->info.hwType);

    gcoSURF_Unlock(buffer->info.surface, gcvNULL);
    gcoSURF_Destroy(buffer->info.surface);
    gcoHAL_Commit(gcvNULL, gcvFALSE);

    free(buffer);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);
}

static const struct wl_callback_listener buffer_callback_listener =
{
    /**
     * done - done event
     * @callback_data: request-specific data for the wl_callback
     *
     * Notify the client when the related request is done.
     */
    buffer_callback_handle_done
};

static void buffer_handle_release(void *data, struct wl_buffer *wl_buf)
{
    struct wl_egl_buffer *buffer = data;

    if  (buffer->state != BUFFER_STATE_QUEUED)
    {
        fprintf(stderr, "%s: ERROR: invalid state=%d\n",
                __func__, buffer->state);
    }

    buffer->state = BUFFER_STATE_FREE;
}

static struct wl_buffer_listener buffer_listener = {
    /**
     * release - compositor releases buffer
     *
     * Sent when this wl_buffer is no longer used by the compositor.
     * The client is now free to re-use or destroy this buffer and its
     * backing storage.
     *
     * If a client receives a release event before the frame callback
     * requested in the same wl_surface.commit that attaches this
     * wl_buffer to a surface, then the client is immediately free to
     * re-use the buffer and its backing storage, and does not need a
     * second buffer for the next surface content update. Typically
     * this is possible, when the compositor maintains a copy of the
     * wl_surface contents, e.g. as a GL texture. This is an important
     * optimization for GL(ES) compositors with wl_shm clients.
     */
    buffer_handle_release
};

static void
frame_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time)
{
    struct wl_egl_buffer *buffer = data;

    buffer->frame_callback = NULL;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener frame_callback_listener = {
    /**
     * done - done event
     * @callback_data: request-specific data for the wl_callback
     *
     * Notify the client when the related request is done.
     */
    frame_callback_handle_done
};

static int
wl_egl_buffer_create(struct wl_egl_window *window,
        struct wl_egl_buffer *buffer,
        int width, int height, int type, int format)
{
    gceSTATUS status;

    /*
     * Current hardware type should be correctly set already.
     * Use that hardware type for buffer allocation.
     */
    gcoHAL_GetHardwareType(gcvNULL, &buffer->info.hwType);

    gcmONERROR(
        gcoSURF_Construct(gcvNULL,
                          width,
                          height,
                          1,
                          type,
                          format,
                          gcvPOOL_DEFAULT,
                          &buffer->info.surface));

    if (type != gcvSURF_BITMAP)
    {
        gcoSURF_SetFlags(buffer->info.surface,
                         gcvSURF_FLAG_CONTENT_YINVERTED,
                         gcvTRUE);
    }

    gcmONERROR(gcoSURF_Lock(buffer->info.surface, gcvNULL, gcvNULL));

    gcmONERROR(
        gcoSURF_GetAlignedSize(buffer->info.surface,
                               gcvNULL,
                               gcvNULL,
                               &buffer->info.stride));

    gcmONERROR(
        gcoSURF_QueryVidMemNode(buffer->info.surface,
                                &buffer->info.node,
                                &buffer->info.pool,
                                &buffer->info.size));

    gcmONERROR(
        gcoHAL_NameVideoMemory(buffer->info.node,
                               &buffer->info.node));

    buffer->info.width      = width;
    buffer->info.height     = height;
    buffer->info.format     = (gceSURF_FORMAT) format;
    buffer->info.type       = (gceSURF_TYPE) type;

    buffer->frame_callback  = NULL;

    buffer->wl_buf =
        wl_viv_create_buffer(window->display->wl_viv,
                buffer->info.width,
                buffer->info.height,
                buffer->info.stride,
                buffer->info.format,
                buffer->info.type,
                buffer->info.node,
                buffer->info.pool,
                buffer->info.size);

    wl_buffer_add_listener(buffer->wl_buf, &buffer_listener, buffer);

    return 0;

OnError:
    return -EINVAL;
}

/*
 * display resource may be destroyed before window destroying. Meanwhile,
 * need take care of window resource when display destroying, see
 * wl_egl_display_destroy.
 */
static void
wl_egl_buffer_destroy(struct wl_egl_window *window,
        struct wl_egl_buffer *buffer)
{
    struct wl_egl_display *display = window->display;

    if (!buffer->info.surface)
    {
        return;
    }

    if (display)
    {
        struct wl_callback *callback;
        struct wl_display *wl_dpy = display->wl_dpy;
        struct wl_event_queue *wl_queue = display->wl_queue;
        struct wl_egl_buffer * wrapper;

        /* Use a wrapper struct to allow modifications in original one. */
        wrapper = malloc(sizeof(*buffer));
        memcpy(wrapper, buffer, sizeof(*buffer));

        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
            wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

        callback = wl_display_sync(display->wl_dpy);
        wl_proxy_set_queue((struct wl_proxy *)callback, display->wl_queue);
        wl_callback_add_listener(callback, &buffer_callback_listener, wrapper);

        wl_display_cancel_read(wl_dpy);

        buffer->info.surface = gcvNULL;
    }
    else
    {
        gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

        /* Switch to hardware type when buffer allocation. */
        gcoHAL_GetHardwareType(gcvNULL, &hwType);
        gcoHAL_SetHardwareType(gcvNULL, buffer->info.hwType);

        gcoSURF_Unlock(buffer->info.surface, gcvNULL);
        gcoSURF_Destroy(buffer->info.surface);
        gcoHAL_Commit(gcvNULL, gcvFALSE);

        buffer->info.surface = gcvNULL;

        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, hwType);
    }

    if (buffer->wl_buf)
    {
        wl_buffer_destroy(buffer->wl_buf);
        buffer->wl_buf = NULL;
    }
}

/* returns errno. */
static int wl_egl_window_set_format(struct wl_egl_window *window,
            gceSURF_TYPE Type, gceSURF_FORMAT Format)
{
    /*
     * Possiable types:
     *   gcvSURF_BITMAP
     *   gcvSURF_RENDER_TARGET
     *   gcvSURF_RENDER_TARGET_NO_COMPRESSION
     *   gcvSURF_RENDER_TARGET_NO_TILE_STATUS
     */
    if (Type != gcvSURF_BITMAP)
    {
        return -EINVAL;
    }

    window->format = Format;
    return 0;
}

static struct wl_egl_buffer *
wl_egl_window_dequeue_buffer(struct wl_egl_window *window)
{
    struct wl_egl_buffer *buffer = NULL;
    struct wl_display *wl_dpy = window->display->wl_dpy;
    struct wl_event_queue *wl_queue = window->display->wl_queue;
    int ret = 0;

    /* Try to read and dispatch some events. */
    dispatch_queue(wl_dpy, wl_queue, 0);

    /* Dispatch the default queue. */
    if (wl_display_prepare_read(wl_dpy) == -1)
        wl_display_dispatch_pending(wl_dpy);
    else
        wl_display_cancel_read(wl_dpy);

    if (window->nr_buffers > 1)
    {
        for (;;)
        {
            int i;

            for (i = 0; i < window->nr_buffers; i++)
            {
                /*
                 * Need check 'state', but not 'frame_callback' here. Our driver
                 * may use another thread for 'queue_buffer' where
                 * 'frame_callback' pointer is assigned. That means, null-
                 * 'frame_callback' does not mean the buffer is avaiable for
                 * rendering.
                 */
                if (window->buffers[i].state == BUFFER_STATE_FREE)
                {
                    buffer = &window->buffers[i];
                    break;
                }
            }

            if (buffer)
            {
                break;
            }

            dispatch_queue(wl_dpy, wl_queue, 1);

            /* Dispatch the default queue. */
            if (wl_display_prepare_read(wl_dpy) == -1)
                wl_display_dispatch_pending(wl_dpy);
            else
                wl_display_cancel_read(wl_dpy);
        }
    }
    else
    {
        buffer = &window->buffers[0];
    }

    if (!buffer)
    {
        return NULL;
    }

    /* Should not run here because we checked buffer state already. */
    while (buffer->frame_callback && ret != -1)
        ret = dispatch_queue(wl_dpy, wl_queue, 5);

    if (buffer->info.surface)
    {
        /* check resize. */
        if (buffer->info.width  != window->width ||
            buffer->info.height != window->height)
        {
            /* The buffer must not be in compositor (ie, QUEUED state). */
            wl_egl_buffer_destroy(window, buffer);
        }
    }

    if (!buffer->info.surface)
    {
        wl_egl_buffer_create(window, buffer,
                window->width, window->height,
                window->type,  window->format);
    }

    buffer->state = BUFFER_STATE_DEQUEUED;
    return buffer;
}

static int
wl_egl_window_queue_buffer(struct wl_egl_window *window,
        struct wl_egl_buffer *buffer)
{
    struct wl_egl_display *display = window->display;
    struct wl_display *wl_dpy = display->wl_dpy;
    struct wl_event_queue *wl_queue = display->wl_queue;

    if (display->swap_interval > 0)
    {
        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
            wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

        buffer->frame_callback = wl_surface_frame(window->surface);
        wl_proxy_set_queue((struct wl_proxy *)buffer->frame_callback, wl_queue);
        wl_callback_add_listener(buffer->frame_callback, &frame_callback_listener, buffer);

        wl_display_cancel_read(wl_dpy);
    }

    /* buffer is queued to compositor. */
    buffer->state = BUFFER_STATE_QUEUED;

    window->attached_width  = window->width;
    window->attached_height = window->height;
    window->dx = 0;
    window->dy = 0;

    wl_surface_attach(window->surface, buffer->wl_buf, window->dx, window->dy);
    wl_surface_damage(window->surface, 0, 0, window->width, window->height);
    wl_surface_commit(window->surface);

    /*
     * If we're not waiting for a frame callback then we'll at least throttle
     * to a sync callback so that we always give a chance for the compositor to
     * handle the commit and send a release event before checking for a free
     * buffer
     */
    if (buffer->frame_callback == NULL)
    {
        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
            wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

        buffer->frame_callback = wl_display_sync(wl_dpy);
        wl_proxy_set_queue((struct wl_proxy *)buffer->frame_callback, wl_queue);
        wl_callback_add_listener(buffer->frame_callback, &frame_callback_listener, buffer);

        wl_display_cancel_read(wl_dpy);
    }

    /* flush events. */
    wl_display_flush(wl_dpy);

    return 0;
}

static int
wl_egl_window_cancel_buffer(struct wl_egl_window *window,
        struct wl_egl_buffer *buffer)
{
    struct wl_egl_display *display = window->display;

    buffer->state = BUFFER_STATE_FREE;

    /* flush events. */
    wl_display_flush(display->wl_dpy);

    return 0;
}

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
    struct wl_display *dpy = Display;

    if (!dpy)
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
    struct wl_display *wl_dpy = Display->hdc;
    struct wl_egl_display *display;

    display = wl_egl_display_create(wl_dpy);

    if (!display)
    {
        return EGL_FALSE;
    }

    display->swap_interval = 1;
    Display->localInfo     = display;

    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    struct wl_egl_display *display = Display->localInfo;

    if (!display)
    {
        return EGL_FALSE;
    }

    wl_egl_display_destroy(display);
    Display->localInfo = gcvNULL;

    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    /* Not supported. */
    return 0;
}

/* Query of swap interval range. */
static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Min,
    OUT EGLint * Max
    )
{
    if (Min != NULL)
    {
        *Min = 0;
    }

    if (Max != NULL)
    {
        *Max = 10;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    )
{
    struct wl_egl_display *display = Display->localInfo;

    if (!display)
    {
        return EGL_FALSE;
    }

    /* clamp to min and max */
    display->swap_interval = Interval > 10 ? 10
                           : Interval < 0 ? 0 : Interval;
    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */

static EGLBoolean
_ConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN void * Window
    )
{
    struct wl_egl_window *window = Window;

    if (wl_egl_window_validate(window))
    {
        fprintf(stderr, "%s: invalid window=%p\n", __func__, Window);
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(window != gcvNULL);

    /* Set display to window. */
    window->display = Display->localInfo;

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

        wl_egl_window_set_format(window, gcvSURF_BITMAP, configFormat);
    }

    /* Save window info structure. */
    Surface->winInfo = (void *) 1;
    return EGL_TRUE;
}

static EGLBoolean
_DisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    struct wl_egl_window *window = Surface->hwnd;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    Surface->winInfo = gcvNULL;
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
    struct wl_egl_window *window = Surface->hwnd;
    /* Indirect rendering by default. */
    EGLint renderMode = VEGL_INDIRECT_RENDERING;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    format = window->format;
    type   = window->type;

    if (Surface->openVG)
    {
        /* Check direct rendering support for 2D VG. */
        do
        {
            EGLBoolean formatSupported = EGL_FALSE;

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

        /* Request linear buffer for hardware OpenVG. */
        wl_egl_window_set_format(window, gcvSURF_BITMAP, format);
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
            EGLint texMode = 0;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

            /* TODO: direct rendering is not ready yet. */
            break;

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

            /* Check window format. */
            switch (format)
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

            /* Require fc-fill feature in hardware. */
            status = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER);

            if (status == gcvSTATUS_TRUE)
            {
                /* Has fc-fill feature. */
                fcFill = EGL_TRUE;
            }

            /* Query super tiled TX. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_SUPERTILED_TEXTURE
                );

            if (status == gcvSTATUS_TRUE)
            {
                texMode = VEGL_DIRECT_RENDERING_NOFC;
            }

            /* Query TX with FC. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_TEXTURE_TILE_STATUS_READ
                );

            if (status == gcvSTATUS_TRUE)
            {
                texMode = VEGL_DIRECT_RENDERING_FC_NOCC;
            }

            /* Query TX with CC. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_TX_DECOMPRESSOR
                );

            if (status == gcvSTATUS_TRUE)
            {
                texMode = VEGL_DIRECT_RENDERING;
            }

            if ((texMode >= VEGL_DIRECT_RENDERING_NOFC) && fcFill)
            {
                renderMode = VEGL_DIRECT_RENDERING_FCFILL;
            }
#if gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
            if (texMode >= VEGL_DIRECT_RENDERING_FC_NOCC)
            {
                renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
            }

            if (texMode == VEGL_DIRECT_RENDERING)
            {
                renderMode = VEGL_DIRECT_RENDERING;
            }
#    endif
        }
        while (gcvFALSE);
#  endif

        switch (renderMode)
        {
        case VEGL_INDIRECT_RENDERING:
        default:
            type = gcvSURF_BITMAP;
            break;

        case VEGL_DIRECT_RENDERING_NOFC:
            type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
            break;

        case VEGL_DIRECT_RENDERING_FCFILL:
        case VEGL_DIRECT_RENDERING_FC_NOCC:
            type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
            break;

        case VEGL_DIRECT_RENDERING:
            type = gcvSURF_RENDER_TARGET;
            break;
        }

        /* Set required type and format. */
        wl_egl_window_set_format(window, type, format);
#endif
    }

    *RenderMode = renderMode;
    return EGL_TRUE;
}

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    struct wl_egl_window *window = Surface->hwnd;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

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
    struct wl_egl_window *window = Surface->hwnd;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    *Width  = window->width;
    *Height = window->height;

    return EGL_TRUE;
}

static EGLBoolean
_GetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    struct wl_egl_window *window = Surface->hwnd;
    struct wl_egl_buffer *buffer;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    buffer = wl_egl_window_dequeue_buffer(window);

    if (!buffer)
    {
        return EGL_FALSE;
    }

    BackBuffer->surface  = buffer->info.surface;
    BackBuffer->context  = buffer;
    BackBuffer->origin.x = 0;
    BackBuffer->origin.y = 0;
    BackBuffer->flip     = gcvTRUE;

    return EGL_TRUE;
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
    int err;
    struct wl_egl_window *window = Surface->hwnd;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    err = wl_egl_window_queue_buffer(window, BackBuffer->context);

    if (err)
    {
        return EGL_FALSE;
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
    int err;
    struct wl_egl_window *window = Surface->hwnd;

    if (wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    err = wl_egl_window_cancel_buffer(window, BackBuffer->context);

    if (err)
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
    struct wl_egl_window *window = Surface->hwnd;

    if (window->nr_buffers == 1)
    {
        return EGL_TRUE;
    }

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

    /* Hardware type when allocation. */
    gceHARDWARE_TYPE hwType;
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
    NULL,
    _CancelWindowBackBuffer,
    _SynchronousPost,
    NULL,
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
#ifdef EGL_API_FB
    if ((NativeDisplay != NULL) &&
        (*(uint32_t *) NativeDisplay == gcmCC('F', 'B', 'D', 'V')))
    {
        return veglGetFbdevPlatform(NativeDisplay);
    }
#endif

    return &waylandPlatform;
}

/******************************************************************************/

struct wl_egl_window *wl_egl_window_create(struct wl_surface *surface,
                            int width, int height)
{
    struct wl_egl_window *window = NULL;
    char *p;

    window = (struct wl_egl_window *) malloc(sizeof (struct wl_egl_window));

    if (!window)
    {
        return NULL;
    }

    memset(window, 0, sizeof (struct wl_egl_window));
    window->surface = surface;

    window->nr_buffers  = WL_EGL_NUM_BACKBUFFERS;

    window->width  = width;
    window->height = height;
    window->format = gcvSURF_A8R8G8B8;
    window->type   = gcvSURF_BITMAP;

    p = getenv("GPU_VIV_WL_MULTI_BUFFER");

    if (p != NULL)
    {
        int nr_buffers = atoi(p);

        if (nr_buffers > 0 && nr_buffers <= WL_EGL_MAX_NUM_BACKBUFFERS)
        {
            window->nr_buffers = nr_buffers;
        }
    }

    window->buffers = malloc(window->nr_buffers * sizeof(struct wl_egl_buffer));
    memset(window->buffers, 0, window->nr_buffers * sizeof(struct wl_egl_buffer));

    wl_egl_window_register(window);

    return window;
}

void wl_egl_window_destroy(struct wl_egl_window *window)
{
    int i;
    struct wl_egl_display *display = window->display;

    if (display)
    {
        /*
         * Better to make buffers FREE. But we meet crash in weston, seems
         * in weston desktop-shell, surface is gone in before destroy egl
         * window structure.
         */
        /*
        wl_surface_attach(window->surface, NULL, 0, 0);
        wl_surface_damage(window->surface, 0, 0, window->width, window->height);
        wl_surface_commit(window->surface);
         */
    }

    wl_egl_window_unregister(window);

    for (i = 0; i < window->nr_buffers; i++)
    {
        int ret = 0;
        struct wl_egl_buffer *buffer = &window->buffers[i];

        assert(!(buffer->frame_callback && !display));

        while (buffer->frame_callback && ret != -1)
            ret = dispatch_queue(display->wl_dpy, display->wl_queue, 5);

        wl_egl_buffer_destroy(window, buffer);
    }

    if (display)
    {
        roundtrip_queue(display->wl_dpy, display->wl_queue);
    }

    free(window->buffers);
    window->buffers   = NULL;
    window->signature = 0;

    free(window);
}

void wl_egl_window_resize(struct wl_egl_window *window,
        int width, int height, int dx, int dy)
{
    if (window->width != width || window->height != height)
    {
        struct wl_egl_display *display = window->display;

        if (display)
        {
            roundtrip_queue(display->wl_dpy, display->wl_queue);
        }

        window->width  = width;
        window->height = height;
    }
}

void wl_egl_window_get_attached_size(struct wl_egl_window *window,
        int *width, int *height)
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
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;
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

    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to an available hardware type. */
    pixmap->hwType = !gcoHAL_Is3DAvailable(gcvNULL) ? gcvHARDWARE_VG
                   : gcoHAL_QuerySeparated2D(gcvNULL) ? gcvHARDWARE_3D : gcvHARDWARE_3D2D;

    gcoHAL_SetHardwareType(gcvNULL, pixmap->hwType);

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

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

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

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

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
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

    if ((pixmap == NULL) ||
        (pixmap->signature != WL_EGL_PIXMAP_SIGNATURE))
    {
        /* Bad pixmap. */
        return;
    }

    /* Switch to hardware type when buffer allocation. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);
    gcoHAL_SetHardwareType(gcvNULL, pixmap->hwType);

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(pixmap->surface, gcvNULL));

    /* Destroy the surface. */
    gcmVERIFY_OK(gcoSURF_Destroy(pixmap->surface));

    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

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


struct wl_buffer *
veglCreateWaylandBufferFromImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLImage Image
    )
{
    gceSTATUS status;
    struct wl_buffer *wl_buf = NULL;
    struct wl_egl_display *display;
    struct wl_egl_buffer *buffer;

    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    display = (struct wl_egl_display *) Dpy->localInfo;

    buffer = malloc(sizeof (struct wl_egl_buffer));
    memset(buffer, 0, sizeof (struct wl_egl_buffer));

    buffer->info.width  = Image->image.u.wlbuffer.width;
    buffer->info.height = Image->image.u.wlbuffer.height;

    gcmONERROR(
        gcoSURF_GetAlignedSize(Image->image.surface,
                               gcvNULL,
                               gcvNULL,
                               &buffer->info.stride));

    gcmONERROR(
       gcoSURF_QueryVidMemNode(Image->image.surface,
                               &buffer->info.node,
                               &buffer->info.pool,
                               &buffer->info.size));
    gcmONERROR(
        gcoSURF_GetFormat(Image->image.surface,
                          &buffer->info.type,
                          &buffer->info.format));

    gcmONERROR(
        gcoHAL_NameVideoMemory(buffer->info.node, &buffer->info.node));


    wl_buf = buffer->wl_buf =
        wl_viv_create_buffer(display->wl_viv,
                buffer->info.width, buffer->info.height, buffer->info.stride,
                buffer->info.format, buffer->info.type,
                buffer->info.node, buffer->info.pool,
                buffer->info.size);

    wl_proxy_set_queue((struct wl_proxy *)buffer->wl_buf, NULL);

    /* TODO: what does this mean??? */
    /*buffer is no longer required. wl_buffer will be destoryed by application, look weston nested.c*/

    free(buffer);
    buffer = NULL;

    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

OnError:
    return wl_buf;
}

