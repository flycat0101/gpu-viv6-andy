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

#include <fcntl.h>

#include <wayland-viv-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "wayland-egl-backend.h"

#include <gc_hal_user.h>

#include "gc_egl_platform.h"
#include <gc_egl_precomp.h>

#define _GC_OBJ_ZONE    gcvZONE_OS

#define GC_WL_MAX_SWAP_INTERVAL     1
#define GC_WL_MIN_SWAP_INTERVAL     0

#define WL_EGL_NUM_BACKBUFFERS          3
#define WL_EGL_MAX_NUM_BACKBUFFERS      8

typedef struct __WLEGLBufferRec *__WLEGLBuffer;

typedef struct __WLEGLDisplayRec
{
    struct wl_display * wl_dpy;
    struct wl_viv * wl_viv;
    struct wl_registry * registry;
    struct wl_event_queue * wl_queue;

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    struct wl_display * wrap_dpy;
#endif
} *__WLEGLDisplay;

typedef struct __WLEGLSurfaceRec
{
    uint32_t signature;

    __WLEGLDisplay display;
    struct wl_egl_window  * window;

    int nr_buffers;
    __WLEGLBuffer buffers;
    int next;

    int indequeue;

    int32_t width;
    int32_t height;
    int32_t swap_interval;

    gceSURF_FORMAT format;
    gceSURF_TYPE type;


    pthread_mutex_t commit_mutex;

    gctHANDLE tid;
    struct wl_event_queue * wl_queue;
    struct wl_event_queue * commit_queue;

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    struct wl_surface * wrap_surface;
#endif

    struct wl_callback * frame_callback;
    struct wl_list link;
} *__WLEGLSurface;

enum
{
    BUFFER_STATE_FREE = 0,
    BUFFER_STATE_DEQUEUED,
    BUFFER_STATE_QUEUED,
};

struct __WLEGLBufferRec
{
    __WLEGLSurface parent;
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
        gctUINT32 tsNode;
        gcePOOL tsPool;
        gctSIZE_T tsSize;
        gcoSURF surface;
        gceHARDWARE_TYPE hwType;
        gctINT32 fd;
        gctINT32 ts_fd;
    } info;

    EGLint age;
    volatile int state;
    struct wl_buffer * wl_buf;
};

static struct wl_list __wl_egl_surface_list;
static pthread_mutex_t __wl_egl_surface_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t __once_control = PTHREAD_ONCE_INIT;

static void __wl_egl_init(void)
{
    wl_list_init(&__wl_egl_surface_list);
}

static inline int
__wl_egl_poll_event(struct wl_display *wl_dpy, short int events, int timeout)
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
__wl_egl_dispatch_queue(struct wl_display *wl_dpy,
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

        if (__wl_egl_poll_event(wl_dpy, POLLOUT, -1) == -1)
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

    ret = __wl_egl_poll_event(wl_dpy, POLLIN, timeout);

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
__sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
    int *done = data;

    *done = 1;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener __sync_listener = {
    __sync_callback
};

/*
 * Our optimized roundtrip_queue. It's thread-safe.
 * Return -1 on error, 0 on success.
 */
static int
__wl_egl_roundtrip_queue(struct wl_display *wl_dpy, struct wl_event_queue *wl_queue)
{
    struct wl_callback *callback;
    int done, ret = 0;

    done = 0;

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    {
        struct wl_display *wrapped_display;
        wrapped_display = (struct wl_display *)wl_proxy_create_wrapper((void *)wl_dpy);
        wl_proxy_set_queue((struct wl_proxy *) wrapped_display, wl_queue);
        callback = wl_display_sync(wrapped_display);
        wl_proxy_wrapper_destroy((void *)wrapped_display);
        wl_callback_add_listener(callback, &__sync_listener, &done);
    }
#else
    /*
     * This is to block read & dispatch events in other threads, so that the
     * callback is with correct queue and listener when 'done' event.
     */
    while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1 && ret != -1)
        ret = wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

    if (ret < 0)
        return ret;

    callback = wl_display_sync(wl_dpy);

    if (callback == NULL)
    {
        wl_display_cancel_read(wl_dpy);
        return -1;
    }

    wl_proxy_set_queue((struct wl_proxy *) callback, wl_queue);
    wl_callback_add_listener(callback, &__sync_listener, &done);

    wl_display_cancel_read(wl_dpy);
#endif

    while (!done && ret >= 0)
        ret = __wl_egl_dispatch_queue(wl_dpy, wl_queue, 100);

    if (ret == -1 && !done)
        wl_callback_destroy(callback);

    return ret;
}

static void
__registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
    __WLEGLDisplay display = data;

    if (strcmp(interface, "wl_viv") == 0 && display)
    {
        display->wl_viv = wl_registry_bind(registry, name, &wl_viv_interface, 1);
        wl_proxy_set_queue((struct wl_proxy *)display->wl_viv, display->wl_queue);
    }
}

static void
__registry_handle_global_remove(void *data, struct wl_registry *wl_registry,
                              uint32_t name)
{
}

static const struct wl_registry_listener __registry_listener = {
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
    __registry_handle_global,

    /*
     * global_remove - announce removal of global object
     *
     * Notify the client of removed global objects.
     *
     * This event notifies the client that the global identified by
     * name is no longer available. If the client bound to the global
     * using the bind request, the client should now destroy that
     * object.
     *
     * The object remains valid and requests to the object will be
     * ignored until the client destroys it, to avoid races between the
     * global going away and a client sending a request to it.
     * @param name numeric name of the global object
     */
    __registry_handle_global_remove
};

static __WLEGLDisplay
__wl_egl_display_create(struct wl_display *wl_dpy)
{
    __WLEGLDisplay display = malloc(sizeof(*display));

    if (!display)
    {
        /* Out of memory. */
        return NULL;
    }

    memset(display, 0, sizeof(*display));

    display->wl_dpy = wl_dpy;
#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    display->wrap_dpy = (struct wl_display * )wl_proxy_create_wrapper((void *)wl_dpy);
#endif
    display->wl_queue = wl_display_create_queue(wl_dpy);

    display->registry = wl_display_get_registry(wl_dpy);

    wl_proxy_set_queue((struct wl_proxy *)display->registry, display->wl_queue);
    wl_registry_add_listener(display->registry, &__registry_listener, display);
    __wl_egl_roundtrip_queue(display->wl_dpy, display->wl_queue);

    return display;
}

static void
__wl_egl_display_destroy(__WLEGLDisplay display)
{
    __WLEGLSurface egl_surface;

    pthread_once(&__once_control, __wl_egl_init);

    /*
     * Need destroy all 'display' relevant window resources, window destroying
     * may be after this, without display resource.
     */
    pthread_mutex_lock(&__wl_egl_surface_mutex);

    /*
     * Display may be destroyed before window destroying.
     * Make sure window buffers are synced in this case.
     */
    wl_list_for_each(egl_surface, &__wl_egl_surface_list, link)
    {
        if (egl_surface->display != display)
            continue;


        {
            if (egl_surface->frame_callback && egl_surface->commit_queue)
            {
                int ret = 0;

                pthread_mutex_lock(&egl_surface->commit_mutex);
                while (egl_surface->frame_callback && ret != -1)
                {
                    ret = __wl_egl_dispatch_queue(display->wl_dpy, egl_surface->commit_queue, 100);
                }
                pthread_mutex_unlock(&egl_surface->commit_mutex);
            }
        }

        if (egl_surface->commit_queue)
            __wl_egl_roundtrip_queue(display->wl_dpy, egl_surface->commit_queue);

        if (egl_surface->wl_queue)
            __wl_egl_roundtrip_queue(display->wl_dpy, egl_surface->wl_queue);

        if (egl_surface->commit_queue)
            wl_event_queue_destroy(egl_surface->commit_queue);

        if (egl_surface->wl_queue)
            wl_event_queue_destroy(egl_surface->wl_queue);


#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
        if (egl_surface->wrap_surface)
        {
            wl_proxy_wrapper_destroy((void *)egl_surface->wrap_surface);
            egl_surface->wrap_surface = gcvNULL;
        }
#endif
        egl_surface->commit_queue = gcvNULL;
        egl_surface->wl_queue = gcvNULL;
        egl_surface->display = NULL;
    }

    pthread_mutex_unlock(&__wl_egl_surface_mutex);


    __wl_egl_roundtrip_queue(display->wl_dpy, display->wl_queue);

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    wl_proxy_wrapper_destroy(display->wrap_dpy);
#endif

    wl_registry_destroy(display->registry);
    wl_viv_destroy(display->wl_viv);
    wl_event_queue_destroy(display->wl_queue);
    free(display);
}

static int
__wl_egl_display_is_mthread(__WLEGLDisplay display)
{
    int mthread = 0;
    gctHANDLE tid = gcvNULL;
    __WLEGLSurface egl_surface;

    pthread_once(&__once_control, __wl_egl_init);

    pthread_mutex_lock(&__wl_egl_surface_mutex);
    wl_list_for_each(egl_surface, &__wl_egl_surface_list, link)
    {
        if (egl_surface->display != display)
        {
            continue;
        }

        if (tid == gcvNULL)
        {
            tid = egl_surface->tid;
        }
        else if (tid != egl_surface->tid)
        {
            mthread = 1;
            break;
        }
    }
    pthread_mutex_unlock(&__wl_egl_surface_mutex);

    return mthread;
}

static void
__wl_egl_surface_register(__WLEGLSurface egl_surface)
{
    pthread_once(&__once_control, __wl_egl_init);

    pthread_mutex_lock(&__wl_egl_surface_mutex);
    wl_list_insert(&__wl_egl_surface_list, &egl_surface->link);
    pthread_mutex_unlock(&__wl_egl_surface_mutex);
}

static void
__wl_egl_surface_unregister(__WLEGLSurface egl_surface)
{
    pthread_mutex_lock(&__wl_egl_surface_mutex);
    wl_list_remove(&egl_surface->link);
    pthread_mutex_unlock(&__wl_egl_surface_mutex);
}

static void
__buffer_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time)
{
    __WLEGLBuffer buffer = (__WLEGLBuffer)data;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

    wl_callback_destroy(callback);

    if (buffer->info.fd >= 0)
    {
        close(buffer->info.fd);
        buffer->info.fd = -1;
    }
    if (buffer->info.ts_fd >= 0)
    {
        close(buffer->info.ts_fd);
        buffer->info.ts_fd = -1;
    }

    /* Switch to hardware type when buffer allocation. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);
    gcoHAL_SetHardwareType(gcvNULL, buffer->info.hwType);

    gcoSURF_Unlock(buffer->info.surface, gcvNULL);
    gcoSURF_Destroy(buffer->info.surface);
    gcoHAL_Commit(gcvNULL, gcvFALSE);
    buffer->info.surface = gcvNULL;

    free(buffer);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);
}

static const struct wl_callback_listener __buffer_callback_listener =
{
    /**
     * done - done event
     * @callback_data: request-specific data for the wl_callback
     *
     * Notify the client when the related request is done.
     */
    __buffer_callback_handle_done
};

static void __buffer_handle_release(void *data, struct wl_buffer *wl_buf)
{
    __WLEGLBuffer buffer = data;

    if  (buffer->state != BUFFER_STATE_QUEUED)
    {
        fprintf(stderr, "%s: ERROR: invalid state=%d\n",
                __func__, buffer->state);
    }

    buffer->state = BUFFER_STATE_FREE;
}

static struct wl_buffer_listener __buffer_listener = {
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
    __buffer_handle_release
};

static void
__frame_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time)
{
    __WLEGLBuffer buffer = data;
    __WLEGLSurface egl_surface = buffer->parent;

    egl_surface->frame_callback = NULL;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener __frame_callback_listener = {
    /**
     * done - done event
     * @callback_data: request-specific data for the wl_callback
     *
     * Notify the client when the related request is done.
     */
    __frame_callback_handle_done
};

static int
__wl_egl_buffer_create(__WLEGLSurface egl_surface, __WLEGLBuffer buffer)
{
    gceSTATUS status;
    gctINT32 fd;
    gcoSURF surface = gcvNULL;

    /*
     * Current hardware type should be correctly set already.
     * Use that hardware type for buffer allocation.
     */
    gcoHAL_GetHardwareType(gcvNULL, &buffer->info.hwType);

    gcmONERROR(
        gcoSURF_Construct(gcvNULL,
                          egl_surface->width,
                          egl_surface->height,
                          1,
                          egl_surface->type,
                          egl_surface->format,
                          gcvPOOL_DEFAULT,
                          &surface));

    buffer->info.surface = surface;

    if ((egl_surface->type & 0xFF) != gcvSURF_BITMAP)
    {
        gcoSURF_SetFlags(surface,
                         gcvSURF_FLAG_CONTENT_YINVERTED,
                         gcvTRUE);
    }

    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    gcmONERROR(
        gcoSURF_GetAlignedSize(surface,
                               gcvNULL,
                               gcvNULL,
                               &buffer->info.stride));

    gcmONERROR(
        gcoSURF_QueryVidMemNode(surface,
                                &buffer->info.node,
                                &buffer->info.pool,
                                &buffer->info.size,
                                &buffer->info.tsNode,
                                &buffer->info.tsPool,
                                &buffer->info.tsSize
                                ));

    gcmONERROR(gcoHAL_ExportVideoMemory(buffer->info.node, O_RDWR, &fd));
    buffer->info.fd = fd;
    gcmONERROR(gcoHAL_NameVideoMemory(buffer->info.node, &buffer->info.node));

    if (buffer->info.tsNode)
    {
        gcmONERROR(gcoHAL_ExportVideoMemory(buffer->info.tsNode, O_RDWR, &fd));
        buffer->info.ts_fd = fd;

        gcmONERROR(
            gcoHAL_NameVideoMemory(buffer->info.tsNode,
                                   &buffer->info.tsNode));
    }

    buffer->info.width      = egl_surface->width;
    buffer->info.height     = egl_surface->height;
    buffer->info.format     = (gceSURF_FORMAT)egl_surface->format;
    buffer->info.type       = (gceSURF_TYPE)egl_surface->type;

    buffer->parent          = egl_surface;

    buffer->wl_buf =
        wl_viv_create_buffer(egl_surface->display->wl_viv,
                buffer->info.width,
                buffer->info.height,
                buffer->info.stride,
                buffer->info.format,
                buffer->info.type,
                buffer->info.node,
                buffer->info.pool,
                buffer->info.size,
                buffer->info.tsNode,
                buffer->info.tsPool,
                buffer->info.tsSize,
                buffer->info.fd);

    wl_buffer_add_listener(buffer->wl_buf, &__buffer_listener, buffer);

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
__wl_egl_buffer_destroy(__WLEGLSurface egl_surface, __WLEGLBuffer buffer)
{
    __WLEGLDisplay display = egl_surface->display;

    int done = 0;

    if (!buffer->info.surface)
    {
        return;
    }

    if (display)
    {
#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
        struct wl_callback *callback;
        __WLEGLBuffer wrapper;

        /* Use a wrapper struct to allow modifications in original one. */
        wrapper = malloc(sizeof(*buffer));
        memcpy(wrapper, buffer, sizeof(*buffer));

        callback = wl_display_sync(display->wrap_dpy);
        wl_proxy_set_queue((struct wl_proxy *)callback, egl_surface->wl_queue);
        wl_callback_add_listener(callback, &__buffer_callback_listener, wrapper);
        buffer->info.surface = gcvNULL;
        done = 1;
#else
        struct wl_callback *callback;
        struct wl_display *wl_dpy = display->wl_dpy;
        struct wl_event_queue *wl_queue = egl_surface->wl_queue;
        __WLEGLBuffer wrapper;
        int ret = 0;
        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1 && ret != -1)
            ret = wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

        if (ret >= 0)
        {
            /* Use a wrapper struct to allow modifications in original one. */
            wrapper = malloc(sizeof(*buffer));
            memcpy(wrapper, buffer, sizeof(*buffer));

            callback = wl_display_sync(display->wl_dpy);
            wl_proxy_set_queue((struct wl_proxy *)callback, egl_surface->wl_queue);
            wl_callback_add_listener(callback, &__buffer_callback_listener, wrapper);

            wl_display_cancel_read(wl_dpy);

            buffer->info.surface = gcvNULL;

            done = 1;
        }
#endif
    }

    if (!done)
    {
        gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

        if (buffer->info.fd >= 0)
        {
            close(buffer->info.fd);
            buffer->info.fd = -1;
        }
        if (buffer->info.ts_fd >= 0)
        {
            close(buffer->info.ts_fd);
            buffer->info.ts_fd = -1;
        }

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

    buffer->age = 0;
}


static void
__wl_egl_surface_destroy(__WLEGLSurface egl_surface)
{
    int i;
    __WLEGLDisplay display = egl_surface->display;

    __wl_egl_surface_unregister(egl_surface);

    for (i = 0; i < egl_surface->nr_buffers; i++)
    {
        __WLEGLBuffer buffer = &egl_surface->buffers[i];

        if (display && egl_surface->frame_callback)
        {
            int ret = 0;

            pthread_mutex_lock(&egl_surface->commit_mutex);
            while (egl_surface->frame_callback && ret != -1)
            {
                ret = __wl_egl_dispatch_queue(display->wl_dpy, egl_surface->commit_queue, 100);
            }
            pthread_mutex_unlock(&egl_surface->commit_mutex);
        }

        __wl_egl_buffer_destroy(egl_surface, buffer);
    }

    if (display)
    {
        __wl_egl_roundtrip_queue(display->wl_dpy, egl_surface->commit_queue);
        __wl_egl_roundtrip_queue(display->wl_dpy, egl_surface->wl_queue);
    }

    if (egl_surface->wl_queue)
    {
        wl_event_queue_destroy(egl_surface->wl_queue);
        egl_surface->wl_queue = gcvNULL;
    }

    if (egl_surface->commit_queue)
    {
        wl_event_queue_destroy(egl_surface->commit_queue);
        egl_surface->commit_queue = gcvNULL;
    }

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    if (egl_surface->wrap_surface)
    {
        wl_proxy_wrapper_destroy((void *)egl_surface->wrap_surface);
        egl_surface->wrap_surface = gcvNULL;
    }
#endif

    free(egl_surface->buffers);
    egl_surface->buffers   = NULL;
    egl_surface->signature = 0;
    egl_surface->frame_callback = gcvNULL;

    pthread_mutex_destroy(&egl_surface->commit_mutex);

    free(egl_surface);
}

static void
__resize_callback(struct wl_egl_window *window, void *data)
{
   __WLEGLSurface egl_surface = data;

   if (window && egl_surface &&
       (window->width != egl_surface->width || window->height != egl_surface->height)
      )
   {
        int i;
        VEGLDisplay dpy = NULL;
        VEGLSurface sur = NULL;

        if (egl_surface->display)
        {
            __wl_egl_roundtrip_queue(egl_surface->display->wl_dpy, egl_surface->wl_queue);
        }

        egl_surface->width  = window->width;
        egl_surface->height = window->height;
        egl_surface->frame_callback = gcvNULL;

        /* handle recursive call to dequeue issue. */
        if (egl_surface->indequeue)
        {
            return;
        }

        /* Reset age. */
        for (i = 0; i < egl_surface->nr_buffers; i++)
        {
            __WLEGLBuffer buffer = &egl_surface->buffers[i];
            buffer->age = 0;
        }

        /* Go through dpy list to find EGLSurface for window. */
        gcoOS_LockPLS();
        for (dpy = (VEGLDisplay)gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO); dpy != NULL; dpy = dpy->next)
        {
            for (sur = (VEGLSurface)dpy->surfaceStack; sur != NULL; sur = (VEGLSurface)sur->resObj.next)
            {
                if (sur->hwnd == window)
                {
                    break;
                }
            }

            if (sur)
            {
                break;
            }
        }
        gcoOS_UnLockPLS();

        if (dpy && sur)
        {
            VEGLThreadData thread = veglGetThreadData();

            if (thread && thread->context &&
                thread->context->context &&
                (thread->context->read == sur ||
                 thread->context->draw == sur))
            {
                veglResizeSurface(dpy, sur, egl_surface->width, egl_surface->height);
            }
        }
   }
}

static void
__destroy_window_callback(void *data)
{
    __WLEGLSurface egl_surface = data;
    if (egl_surface)
    {
        __wl_egl_surface_destroy(egl_surface);
    }
}

__WLEGLSurface
__wl_egl_surface_create(struct wl_egl_window *window)
{
    int i;
    char *p;
    __WLEGLSurface egl_surface = NULL;

    egl_surface = (__WLEGLSurface)malloc(sizeof(*egl_surface));
    if (!egl_surface)
    {
        return NULL;
    }

    memset(egl_surface, 0, sizeof(*egl_surface));
    egl_surface->window = window;
#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
    egl_surface->wrap_surface = (struct wl_surface *)wl_proxy_create_wrapper((void*)window->surface);
#endif

    egl_surface->nr_buffers  = WL_EGL_NUM_BACKBUFFERS;

    egl_surface->width  = window->width;
    egl_surface->height = window->height;
    egl_surface->format = gcvSURF_A8R8G8B8;
    egl_surface->type   = gcvSURF_BITMAP;
    egl_surface->swap_interval = 1;
    egl_surface->frame_callback = gcvNULL;

    pthread_mutex_init(&egl_surface->commit_mutex, NULL);

    p = getenv("WL_EGL_SWAP_INTERVAL");
    if (p)
    {
        int interval = atoi(p);
        egl_surface->swap_interval = gcmCLAMP(interval, GC_WL_MIN_SWAP_INTERVAL, GC_WL_MAX_SWAP_INTERVAL);
    }
    else
    {
        egl_surface->swap_interval = 1;
    }

    p = getenv("GPU_VIV_WL_MULTI_BUFFER");
    if (p)
    {
        int nr_buffers = atoi(p);

        if (nr_buffers > 0 && nr_buffers <= WL_EGL_MAX_NUM_BACKBUFFERS)
        {
            egl_surface->nr_buffers = nr_buffers;
        }
    }

    egl_surface->buffers = calloc(egl_surface->nr_buffers, sizeof(struct __WLEGLBufferRec));
    for (i = 0; i < egl_surface->nr_buffers; ++i)
    {
        egl_surface->buffers[i].info.fd = -1;
        egl_surface->buffers[i].info.ts_fd = -1;
    }

    __wl_egl_surface_register(egl_surface);

    window->driver_private = egl_surface;
    window->destroy_window_callback = __destroy_window_callback;
    window->resize_callback = __resize_callback;

    return egl_surface;
}

/* returns errno. */
static int
__wl_egl_surface_set_format(__WLEGLSurface egl_surface,
            gceSURF_TYPE Type, gceSURF_FORMAT Format)
{
    switch (Type)
    {
    case gcvSURF_BITMAP:
    case gcvSURF_RENDER_TARGET:
    case gcvSURF_RENDER_TARGET_NO_COMPRESSION:
    case gcvSURF_RENDER_TARGET_NO_TILE_STATUS:
        break;

    default:
        return -EINVAL;
    }

    egl_surface->type   = Type | gcvSURF_DMABUF_EXPORTABLE | gcvSURF_CACHE_MODE_128;
    egl_surface->format = Format;
    return 0;
}

static inline int
__wl_egl_window_validate(struct wl_egl_window *window)
{
    int ret = -EINVAL;
    __WLEGLSurface egl_surface;

    pthread_once(&__once_control, __wl_egl_init);

    pthread_mutex_lock(&__wl_egl_surface_mutex);
    wl_list_for_each(egl_surface, &__wl_egl_surface_list, link)
    {
        if (egl_surface->window == window)
        {
            ret = 0;
            break;
        }
    }
    pthread_mutex_unlock(&__wl_egl_surface_mutex);

    return ret;
}

static __WLEGLBuffer
__wl_egl_window_dequeue_buffer(struct wl_egl_window *window)
{
    __WLEGLBuffer buffer = NULL;
    __WLEGLSurface egl_surface = window->driver_private;
    struct wl_display *wl_dpy = egl_surface->display->wl_dpy;
    struct wl_event_queue *wl_queue = egl_surface->wl_queue;
    int ret = 0;

    if (egl_surface->indequeue)
    {
        fprintf(stderr, "ERROR: nested dequeue buffer\n");
        return NULL;
    }

    egl_surface->indequeue = 1;


    /* Try to read and dispatch some events. */
    __wl_egl_dispatch_queue(wl_dpy, wl_queue, 1);

    if (egl_surface->nr_buffers > 1)
    {

        for (;;)
        {
            int current = egl_surface->next;

            buffer = &egl_surface->buffers[current];

            if (buffer->state == BUFFER_STATE_FREE)
            {
                egl_surface->next = current + 1;

                if (egl_surface->next >= egl_surface->nr_buffers)
                    egl_surface->next -= egl_surface->nr_buffers;

                break;
            }

            ret = __wl_egl_dispatch_queue(wl_dpy, wl_queue, 5);

            if (ret == -1)
            {
                buffer = NULL;
                break;
            }
        }

    }
    else
    {
        buffer = &egl_surface->buffers[0];
    }

    if (!buffer)
    {
        egl_surface->indequeue = 0;
        return NULL;
    }

    if (buffer->info.surface)
    {
        /* check resize. */
        if (buffer->info.width  != egl_surface->width  ||
            buffer->info.height != egl_surface->height ||
            buffer->info.format != egl_surface->format)
        {
            /* The buffer must not be in compositor (ie, QUEUED state). */
            __wl_egl_buffer_destroy(egl_surface, buffer);
        }
    }

    if (!buffer->info.surface)
    {
        __wl_egl_buffer_create(egl_surface, buffer);

        window->attached_width  = egl_surface->width;
        window->attached_height = egl_surface->height;
    }

    egl_surface->indequeue = 0;
    buffer->state = BUFFER_STATE_DEQUEUED;
    return buffer;
}

static void
__make_bounding_box(__WLEGLBuffer buffer, const struct eglRegion *region,
        int *x, int *y, int *width, int *height)
{
    if (region)
    {
        int i;
        /* region is relative to bottom-left corner. */
        int left   = region->rects[0];
        int bottom = region->rects[1];
        int right  = region->rects[2] + left;
        int top    = region->rects[3] + bottom;

        for (i = 1; i < region->numRects; i++)
        {
            int l = region->rects[i * 4 + 0];
            int b = region->rects[i * 4 + 1];
            int r = region->rects[i * 4 + 2] + l;
            int t = region->rects[i * 4 + 3] + b;

            left   = gcmMIN(l, left);
            bottom = gcmMIN(b, bottom);
            right  = gcmMAX(r, right);
            top    = gcmMAX(t, top);
        }

        /* translate to upper-left corner relative. */
        *x = left;
        *y = buffer->info.height - top;
        *width  = right - left;
        *height = top - bottom;
    }
    else
    {
        *x = *y = 0;
        *width  = buffer->info.width;
        *height = buffer->info.height;
    }
}

#if (WAYLAND_VERSION_MAJOR >= 1) && (WAYLAND_VERSION_MINOR >= 13)
static int
__wl_egl_window_queue_buffer(struct wl_egl_window *window,
        __WLEGLBuffer buffer,
        struct eglRegion *damage)
{
    __WLEGLSurface egl_surface = window->driver_private;
    __WLEGLDisplay display = egl_surface->display;
    struct wl_display *wl_dpy = display->wl_dpy;
    struct wl_event_queue *commit_queue = egl_surface->commit_queue;
    int x, y, width, height;
    int ret = 0;

    __make_bounding_box(buffer, damage, &x, &y, &width, &height);

    pthread_mutex_lock(&egl_surface->commit_mutex);

    /* Make sure previous frame with this buffer is done. */
    while (egl_surface->frame_callback && ret != -1)
        ret = __wl_egl_dispatch_queue(wl_dpy, commit_queue, 100);

    if (ret == -1)
    {
        /* fatal error, can not recover. */
        goto out;
    }

    if (egl_surface->swap_interval > 0)
    {
        egl_surface->frame_callback = wl_surface_frame(egl_surface->wrap_surface);
        wl_proxy_set_queue((struct wl_proxy *)egl_surface->frame_callback, commit_queue);
        wl_callback_add_listener(egl_surface->frame_callback, &__frame_callback_listener, buffer);
    }

    /* buffer is queued to compositor. */
    buffer->state = BUFFER_STATE_QUEUED;
    window->dx = 0;
    window->dy = 0;

    if (buffer->info.surface)
    {
        gcoSURF surface = buffer->info.surface;

#if gcdENABLE_3D
        wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf,
            !surface->tileStatusDisabled[0], surface->compressed,
            surface->dirty[0], surface->fcValue[0], surface->fcValueUpper[0]);
#else
        wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf, 0, 0, 0, 0, 0);
#endif

        gcoSURF_UpdateMetadata(surface, buffer->info.ts_fd);
    }

    wl_surface_attach(egl_surface->wrap_surface, buffer->wl_buf, window->dx, window->dy);
    wl_surface_damage(egl_surface->wrap_surface, x, y, width, height);
    wl_surface_commit(egl_surface->wrap_surface);


    /*
     * If we're not waiting for a frame callback then we'll at least throttle
     * to a sync callback so that we always give a chance for the compositor to
     * handle the commit and send a release event before checking for a free
     * buffer
     */
    if (egl_surface->frame_callback == NULL)
    {
        egl_surface->frame_callback = wl_display_sync(display->wrap_dpy);
        wl_proxy_set_queue((struct wl_proxy *)egl_surface->frame_callback, commit_queue);
        wl_callback_add_listener(egl_surface->frame_callback, &__frame_callback_listener, buffer);
    }

    /* flush events. */
    wl_display_flush(wl_dpy);

out:
    pthread_mutex_unlock(&egl_surface->commit_mutex);

    return ret;
}
#else
static int
__wl_egl_window_queue_buffer(struct wl_egl_window *window,
        __WLEGLBuffer buffer,
        struct eglRegion *damage)
{
    __WLEGLSurface egl_surface = window->driver_private;
    __WLEGLDisplay display = egl_surface->display;
    struct wl_display *wl_dpy = display->wl_dpy;
    struct wl_event_queue *commit_queue = egl_surface->commit_queue;
    int x, y, width, height;
    int ret = 0;

    __make_bounding_box(buffer, damage, &x, &y, &width, &height);

    pthread_mutex_lock(&egl_surface->commit_mutex);

    /* Make sure previous frame with this buffer is done. */
    while (egl_surface->frame_callback && ret != -1)
        ret = __wl_egl_dispatch_queue(wl_dpy, commit_queue, 100);

    if (ret == -1)
    {
        /* fatal error, can not recover. */
        goto out;
    }

    if (egl_surface->swap_interval > 0)
    {
        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, commit_queue) == -1 && ret != -1)
            ret = wl_display_dispatch_queue_pending(wl_dpy, commit_queue);

        if (ret == -1)
        {
            /* fatal error, can not recover. */
            goto out;
        }

        egl_surface->frame_callback = wl_surface_frame(window->surface);
        wl_proxy_set_queue((struct wl_proxy *)egl_surface->frame_callback, commit_queue);
        wl_callback_add_listener(egl_surface->frame_callback, &__frame_callback_listener, buffer);

        wl_display_cancel_read(wl_dpy);
    }

    /* buffer is queued to compositor. */
    buffer->state = BUFFER_STATE_QUEUED;
    window->dx = 0;
    window->dy = 0;

    if (buffer->info.surface)
    {
        gcoSURF surface = buffer->info.surface;

#if gcdENABLE_3D
        wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf,
            !surface->tileStatusDisabled[0], surface->compressed,
            surface->dirty[0], surface->fcValue[0], surface->fcValueUpper[0]);
#else
        wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf, 0, 0, 0, 0, 0);
#endif

        gcoSURF_UpdateMetadata(surface, buffer->info.ts_fd);
    }

    wl_surface_attach(window->surface, buffer->wl_buf, window->dx, window->dy);
    wl_surface_damage(window->surface, x, y, width, height);
    wl_surface_commit(window->surface);

    /*
     * If we're not waiting for a frame callback then we'll at least throttle
     * to a sync callback so that we always give a chance for the compositor to
     * handle the commit and send a release event before checking for a free
     * buffer
     */
    if (egl_surface->frame_callback == NULL)
    {
        /*
         * This is to block read & dispatch events in other threads, so that the
         * callback is with correct queue and listener when 'done' event.
         */
        while (wl_display_prepare_read_queue(wl_dpy, commit_queue) == -1 && ret != -1)
            ret = wl_display_dispatch_queue_pending(wl_dpy, commit_queue);

        if (ret == -1)
        {
            goto out;
        }

        egl_surface->frame_callback = wl_display_sync(wl_dpy);
        wl_proxy_set_queue((struct wl_proxy *)egl_surface->frame_callback, commit_queue);
        wl_callback_add_listener(egl_surface->frame_callback, &__frame_callback_listener, buffer);

        wl_display_cancel_read(wl_dpy);
    }

    /* flush events. */
    wl_display_flush(wl_dpy);

out:
    pthread_mutex_unlock(&egl_surface->commit_mutex);

    return ret;
}
#endif

static int
__wl_egl_window_cancel_buffer(struct wl_egl_window *window, __WLEGLBuffer buffer)
{
    __WLEGLSurface egl_surface = window->driver_private;

    buffer->state = BUFFER_STATE_FREE;

    /* Roll back to make back buffers returned in order. */
    egl_surface->next = (egl_surface->next == 0)
                      ? egl_surface->nr_buffers - 1
                      : egl_surface->next - 1;

    /* flush events. */
    wl_display_flush(egl_surface->display->wl_dpy);

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
    __WLEGLDisplay display = __wl_egl_display_create(wl_dpy);

    if (!display)
    {
        return EGL_FALSE;
    }

    Display->localInfo = display;

    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    __WLEGLDisplay display = Display->localInfo;

    if (!display)
    {
        return EGL_FALSE;
    }

    __wl_egl_display_destroy(display);
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
        *Min = GC_WL_MIN_SWAP_INTERVAL;
    }

    if (Max != NULL)
    {
        *Max = GC_WL_MAX_SWAP_INTERVAL;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    __WLEGLSurface egl_surface;
    struct wl_egl_window *window = Surface->hwnd;

    if (!window)
    {
        return EGL_FALSE;
    }

    /* clamp to min and max */
    egl_surface = window->driver_private;
    egl_surface->swap_interval = gcmCLAMP(Interval, GC_WL_MIN_SWAP_INTERVAL, GC_WL_MAX_SWAP_INTERVAL);

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
    __WLEGLSurface egl_surface;
    struct wl_egl_window *window = Window;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(window != gcvNULL);

    egl_surface = window->driver_private;
    if (egl_surface == gcvNULL)
    {
        egl_surface = __wl_egl_surface_create(window);
    }
    /* Set display to window. */
    egl_surface->display = Display->localInfo;

    gcmASSERT(Surface->renderTargetFormat != gcvSURF_UNKNOWN);
    __wl_egl_surface_set_format(egl_surface, gcvSURF_BITMAP, Surface->renderTargetFormat);

    if (egl_surface->wl_queue == gcvNULL)
    {
        egl_surface->wl_queue = wl_display_create_queue(egl_surface->display->wl_dpy);
        wl_proxy_set_queue((struct wl_proxy *)egl_surface->display->wl_viv, egl_surface->wl_queue);
    }

    if (egl_surface->commit_queue == gcvNULL)
        egl_surface->commit_queue = wl_display_create_queue(egl_surface->display->wl_dpy);

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

    if (__wl_egl_window_validate(window))
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
#if gcdENABLE_3D
    gceSTATUS status;
#endif
    __WLEGLSurface egl_surface;
    struct wl_egl_window *window = Surface->hwnd;

    /* Indirect rendering by default. */
    EGLint renderMode = VEGL_INDIRECT_RENDERING;

    if (__wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    egl_surface = window->driver_private;

    if (Surface->openVG)
    {
        gceSURF_FORMAT format = egl_surface->format;

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
        __wl_egl_surface_set_format(egl_surface, gcvSURF_BITMAP, format);
    }
    else
    {
#if gcdENABLE_3D
        gceSURF_TYPE type;
        gceSURF_FORMAT format = egl_surface->format;

#if gcdENABLE_RENDER_INTO_WINDOW
        /* 3D pipe. */
        do
        {
            /* Check if direct rendering is available. */
            VEGLThreadData thread;
            EGLBoolean fcFill = EGL_FALSE;
            EGLBoolean formatSupported;
            EGLint texMode = 0;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

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

            thread = veglGetThreadData();

            if (thread && thread->api == EGL_OPENVG_API)
            {
                /* 3D VG, stop. */
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

        type = egl_surface->type;
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
        __wl_egl_surface_set_format(egl_surface, type, format);
#endif
    }

    egl_surface->tid = gcoOS_GetCurrentThreadID();

    *RenderMode = renderMode;
    return EGL_TRUE;
}

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    __WLEGLSurface egl_surface;
    struct wl_egl_window *window = Surface->hwnd;

    if (__wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    /* Do not throttle the swap thread. */
    egl_surface = window->driver_private;
    egl_surface->tid = gcvNULL;

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

    if (__wl_egl_window_validate(window))
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
    __WLEGLBuffer buffer;
    struct wl_egl_window *window = Surface->hwnd;

    if (__wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    buffer = __wl_egl_window_dequeue_buffer(window);

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
    IN struct eglRegion * Region,
    IN struct eglRegion * DamageHint
    )
{
    int err;
    struct wl_egl_window *window = Surface->hwnd;

    if (__wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    err = __wl_egl_window_queue_buffer(window, BackBuffer->context, DamageHint);

    if (err < 0)
    {
        fprintf(stderr, "EGL: errno=%d (%s)\n", errno, strerror(errno));
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

    if (__wl_egl_window_validate(window))
    {
        return EGL_FALSE;
    }

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    err = __wl_egl_window_cancel_buffer(window, BackBuffer->context);

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
    __WLEGLSurface egl_surface = window->driver_private;

    if (egl_surface->nr_buffers == 1)
    {
        return EGL_TRUE;
    }

    /* If there are mulitithreads and each thread has its own window,
     * for window with <swap_interval = 0>, current thread is used to swap frame into screen instead of swapthread.
     * When mulitithread, all rendering requests will be put into swapthread and will be processed one by one,
     * but the requests for <interval = 0> should be handled faster than requests for <interval = 1>, the processing rate is decided by the slow ones,
     * this means <interval = 0> will not work.
     */
    if (__wl_egl_display_is_mthread(egl_surface->display))
    {
        if (egl_surface->swap_interval == 0)
        {
            return EGL_TRUE;
        }
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
    int i;
    struct wl_egl_window *window = Surface->hwnd;
    __WLEGLSurface egl_surface = window->driver_private;
    __WLEGLBuffer buffer = BackBuffer->context;

    for (i = 0; i < egl_surface->nr_buffers; i++)
    {
        __WLEGLBuffer b = &egl_surface->buffers[i];

        if (b->age)
        {
            b->age++;
        }
    }

    buffer->age = 1;

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
    if (BackBuffer && BackBuffer->context)
    {
        __WLEGLBuffer buffer = BackBuffer->context;
        *BufferAge = buffer->age;
        return EGL_TRUE;
    }
    else if (!Surface->newSwapModel)
    {
        /*
         * In current wayland implementation, back buffers are returned in order.
         * It's safe to return age of buffer to dequeue next time --- except that
         * there're 0 aged buffers.
         */
        int i;
        struct wl_egl_window *window = Surface->hwnd;
        __WLEGLSurface egl_surface = window->driver_private;
        __WLEGLBuffer buffer = &egl_surface->buffers[egl_surface->next];

        *BufferAge = buffer->age;

        for (i = 0; i < egl_surface->nr_buffers; i++)
        {
            buffer = &egl_surface->buffers[i];

            if (buffer->age == 0)
            {
                *BufferAge = 0;
                break;
            }
        }

        return EGL_TRUE;
    }

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
    EGL_PLATFORM_WAYLAND_VIV,

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
                   : gcoHAL_QueryHybrid2D(gcvNULL) ? gcvHARDWARE_3D2D : gcvHARDWARE_3D;

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
    __WLEGLDisplay display;
    __WLEGLBuffer buffer;
    gctINT32 fd = -1;
    gcoSURF surface = NULL;

    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    display = (__WLEGLDisplay)Dpy->localInfo;

    buffer = malloc(sizeof(*buffer));
    memset(buffer, 0, sizeof(*buffer));

    /* wlbuffer's width and height and fd got from veglQueryWaylandBuffer */
    buffer->info.width  = Image->image.u.wlbuffer.width;
    buffer->info.height = Image->image.u.wlbuffer.height;
    fd = Image->image.u.wlbuffer.fd;
    surface = Image->image.surface;

    gcmASSERT((fd >= 0));

    gcmONERROR(
        gcoSURF_GetAlignedSize(surface,
                               gcvNULL,
                               gcvNULL,
                               &buffer->info.stride));

    gcmONERROR(
        gcoSURF_GetFormat(surface,
                          &buffer->info.type,
                          &buffer->info.format));
    /* Need original type ever pass when creating surface */
    buffer->info.type |= surface->hints;

    gcmONERROR(
        gcoSURF_QueryVidMemNode(surface,
                                &buffer->info.node,
                                &buffer->info.pool,
                                &buffer->info.size,
                                &buffer->info.tsNode,
                                &buffer->info.tsPool,
                                &buffer->info.tsSize
                                ));

    buffer->info.fd = fd;
    gcmONERROR(gcoHAL_NameVideoMemory(buffer->info.node, &buffer->info.node));

    if (buffer->info.tsNode)
    {
        gcmONERROR(gcoHAL_ExportVideoMemory(buffer->info.tsNode, O_RDWR, &fd));
        buffer->info.ts_fd = fd;
        gcmONERROR(gcoHAL_NameVideoMemory(buffer->info.tsNode, &buffer->info.tsNode));
    }

    wl_buf = buffer->wl_buf =
        wl_viv_create_buffer(display->wl_viv,
                buffer->info.width, buffer->info.height, buffer->info.stride,
                buffer->info.format, buffer->info.type,
                buffer->info.node, buffer->info.pool, buffer->info.size,
                buffer->info.tsNode, buffer->info.tsPool, buffer->info.tsSize, buffer->info.fd);

#if gcdENABLE_3D
    wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf,
        !surface->tileStatusDisabled[0], surface->compressed,
        surface->dirty[0], surface->fcValue[0], surface->fcValueUpper[0]);
#else
    wl_viv_enable_tile_status(display->wl_viv, buffer->wl_buf, 0, 0, 0, 0, 0);
#endif

    wl_proxy_set_queue((struct wl_proxy *)buffer->wl_buf, NULL);

    /*buffer is no longer required. wl_buffer will be destoryed by application, look weston nested.c*/
    free(buffer);
    buffer = NULL;

    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

OnError:
    return wl_buf;
}

