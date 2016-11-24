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

#include <wayland-viv-server-protocol.h>
#include <wayland-server.h>

#include <gc_hal_user.h>

#include "gc_egl_platform.h"

/* Extended wl buffer. */
struct wl_viv_buffer
{
    struct wl_resource *resource;
    gcoSURF  surface;
    gctINT32 width;
    gctINT32 height;
};

static void
destroy_buffer(struct wl_resource *resource)
{
    struct wl_viv_buffer *buffer = resource->data;

    if (buffer != NULL)
    {
        gcoSURF surface = buffer->surface;

        if (surface)
        {
            gcoSURF_Unlock(surface, gcvNULL);
            gcoSURF_Destroy(surface);
        }

        free(buffer);
    }
}

static void
handle_destroy_buffer(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface wl_viv_buffer_implementation = {
    handle_destroy_buffer
};

static void
viv_handle_create_buffer(struct wl_client *client,
                  struct wl_resource *resource,
                  uint32_t id,
                  uint32_t width,
                  uint32_t height,
                  uint32_t stride,
                  int32_t format,
                  int32_t type,
                  uint32_t node,
                  int32_t pool,
                  uint32_t size)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface = gcvNULL;
    struct wl_viv_buffer * buffer = NULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    buffer = malloc(sizeof(*buffer));

    if (!buffer)
    {
        return;
    }

    memset(buffer, 0, sizeof(*buffer));

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
    surface->node.size          = (gctSIZE_T) size;

    /*
    gcmONERROR(
        gcoSURF_SetOrientation(surface,
                               gcvORIENTATION_BOTTOM_TOP));
     */

    gcmONERROR(gcoHAL_ImportVideoMemory(
        (gctUINT32)node, (gctUINT32 *)&surface->node.u.normal.node));

    /* Lock once as it's done in gcoSURF_Construct with vidmem. */
    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    buffer->surface = surface;
    buffer->width   = width;
    buffer->height  = height;

    buffer->resource = wl_resource_create(client, &wl_buffer_interface, 1, id);

    if (!buffer->resource)
    {
        wl_resource_post_no_memory(resource);

        goto OnError;
    }

    wl_resource_set_implementation(buffer->resource,
                       (void (**)(void)) &wl_viv_buffer_implementation,
                       buffer, destroy_buffer);

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

    if (buffer)
    {
        free(buffer);
    }

    return;
}

struct wl_viv_interface wl_viv_implementation = {
    viv_handle_create_buffer
};

static void
bind_wl_viv(struct wl_client *client,
         void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_viv_interface, 1, id);

    if (!resource)
    {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &wl_viv_implementation, data, NULL);
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
        wl_global =
            wl_global_create(Dpy, &wl_viv_interface, 1, NULL, bind_wl_viv);

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
    struct wl_viv_buffer *buffer;
    buffer = wl_resource_get_user_data(Buffer);

    if (Width)
    {
        *Width = buffer->width;
    }

    if (Height)
    {
        *Height = buffer->height;
    }

    if (Surface)
    {
        *Surface = buffer->surface;
    }

    return EGL_TRUE;
}

