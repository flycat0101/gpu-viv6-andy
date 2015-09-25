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


#include "gc_wayland_protocol.h"
#include "wayland-viv-client-protocol.h"

#include <wayland-client.h>
#include <string.h>
#include <stdlib.h>
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

static int
roundtrip(gcsWL_EGL_DISPLAY *display)
{
   struct wl_callback *callback;
   int done = 0, ret = 0;

   callback = wl_display_sync(display->wl_display);
   wl_callback_add_listener(callback, &sync_listener, &done);
   wl_proxy_set_queue((struct wl_proxy *) callback, display->wl_queue);
   while (ret != -1 && !done)
   {
      ret = wl_display_dispatch_queue(display->wl_display, display->wl_queue);
   }

   return ret;
}

void
gcoWL_CreateGhostBuffer(gcsWL_EGL_DISPLAY* wl_egl, gcsWL_EGL_BUFFER* buffer)
{

    buffer->wl_buffer =
            wl_viv_create_buffer(wl_egl->wl_viv,
                                buffer->info.width,
                                buffer->info.height,
                                buffer->info.stride,
                                buffer->info.format,
                                buffer->info.type,
                                (int32_t) buffer->info.node,
                                buffer->info.pool,
                                buffer->info.bytes);
}

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
   gcsWL_EGL_DISPLAY* display = data;

   if (strcmp(interface, "wl_viv") == 0) {

   if (display)
   {
        display->wl_viv = wl_registry_bind(registry, name, &wl_viv_interface, 1);
   }
   }
}

static const struct wl_registry_listener registry_listener = {
       registry_handle_global
};

gcsWL_EGL_DISPLAY*
gcoWL_GetDisplay(struct wl_display *wl_dpy)
{
   gcsWL_EGL_DISPLAY* display = gcvNULL;
   gceSTATUS status = gcvSTATUS_OK;

   display = malloc(sizeof *display);
   if (display == gcvNULL)
   {
       /* Out of memory. */
       gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
   }
   memset(display, 0, sizeof *display);

   display->wl_display = wl_dpy;
   display->registry = wl_display_get_registry(wl_dpy);
   display->wl_queue = wl_display_create_queue(wl_dpy);
   wl_proxy_set_queue((struct wl_proxy *) display->registry,
                      display->wl_queue);

   display->wl_swap_queue = wl_display_create_queue(wl_dpy);

   wl_registry_add_listener(display->registry, &registry_listener, display);

   if (roundtrip(display) >= 0 && display->wl_viv != NULL)
   {
        return display;
   }

OnError:
    return 0;
}

void
gcoWL_ReleaseDisplay(gcsWL_EGL_DISPLAY* display)
{
    wl_registry_destroy(display->registry);
    wl_event_queue_destroy(display->wl_queue);
    wl_event_queue_destroy(display->wl_swap_queue);
    wl_viv_destroy(display->wl_viv);
    free((gctPOINTER)display);
    display = NULL;
}

