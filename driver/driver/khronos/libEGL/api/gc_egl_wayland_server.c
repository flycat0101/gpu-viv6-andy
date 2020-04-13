/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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
#include <wl-viv-buffer.h>

#include "gc_egl_platform.h"

/* Private data of struct wl_viv_buffer. */
struct wl_viv_buffer_private
{
    gceHARDWARE_TYPE hwType;
};

static void
destroy_buffer(struct wl_resource *resource)
{
    struct wl_viv_buffer *buffer = wl_resource_get_user_data(resource);
    struct wl_viv_buffer_private *priv = (struct wl_viv_buffer_private*)&buffer[1];

    if (buffer != NULL)
    {
        gcoSURF surface = buffer->surface;

        if (buffer->fd >= 0)
        {
            close(buffer->fd);
            buffer->fd = -1;
        }

        if (surface)
        {
            gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

            /* Switch to hardware type when allocation. */
            gcoHAL_GetHardwareType(gcvNULL, &hwType);
            gcoHAL_SetHardwareType(gcvNULL, priv->hwType);

            gcoSURF_Destroy(surface);

            gcoHAL_Commit(gcvNULL, gcvFALSE);

            /* Restore hardware type. */
            gcoHAL_SetHardwareType(gcvNULL, hwType);
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
                  uint32_t size,
                  uint32_t tsNode,
                  int32_t tsPool,
                  uint32_t tsSize,
                  int32_t fd)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface = gcvNULL;
    struct wl_viv_buffer * buffer = NULL;
    struct wl_viv_buffer_private * priv = NULL;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_INVALID;

    buffer = malloc(sizeof(*buffer) + sizeof(struct wl_viv_buffer_private));

    if (!buffer)
    {
        return;
    }

    memset(buffer, 0, sizeof(*buffer));
    priv = (struct wl_viv_buffer_private *)&buffer[1];

    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to an available hardware type. */
    priv->hwType = !gcoHAL_Is3DAvailable(gcvNULL) ? gcvHARDWARE_VG
                   : gcoHAL_QueryHybrid2D(gcvNULL) ? gcvHARDWARE_3D2D : gcvHARDWARE_3D;

    gcoHAL_SetHardwareType(gcvNULL, priv->hwType);

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

#if gcdENABLE_3D
    /* Import tile status video memory node. */
    if (tsNode != 0)
    {
        gcmVERIFY_OK(gcoHAL_ImportVideoMemory(tsNode, &tsNode));
    }

    surface->tileStatusNode.u.normal.node = tsNode;
    surface->tileStatusNode.pool          = (gcePOOL  ) tsPool;
    surface->tileStatusNode.size          = (gctSIZE_T) tsSize;

    /* Set tile status disabled by default for compositor. */
    surface->tileStatusDisabled[0] = gcvTRUE;
#endif

    if ((type & 0xFF) != gcvSURF_BITMAP)
    {
        gcmONERROR(gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE));
    }

    gcmONERROR(gcoHAL_ImportVideoMemory(
        (gctUINT32)node, (gctUINT32 *)&surface->node.u.normal.node));


    /* Lock once as it's done in gcoSURF_Construct with vidmem. */
    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    buffer->surface = surface;
    buffer->width   = width;
    buffer->height  = height;

    buffer->fd = (gctINT32)fd;

    buffer->resource = wl_resource_create(client, &wl_buffer_interface, 1, id);

    if (!buffer->resource)
    {
        wl_resource_post_no_memory(resource);

        goto OnError;
    }

    wl_resource_set_implementation(buffer->resource,
                       (void (**)(void)) &wl_viv_buffer_implementation,
                       buffer, destroy_buffer);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);
    return;

OnError:
    wl_resource_post_no_memory(resource);

    if (surface)
    {
        gcoSURF_Destroy(surface);
        surface = gcvNULL;
    }

    if (buffer)
    {
        free(buffer);
    }

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);
    return;
}

static void
enable_tile_status(struct wl_client *client,
                   struct wl_resource *resource,
                   struct wl_resource *id,
                   uint32_t enabled,
                   uint32_t compressed,
                   uint32_t dirty,
                   uint32_t fc_value,
                   uint32_t fc_value_upper)
{
#if gcdENABLE_3D
    struct wl_viv_buffer * buffer;
    gcoSURF surface;

    buffer = wl_resource_get_user_data(id);
    surface = buffer->surface;

    surface->tileStatusDisabled[0] = !enabled;
    surface->dirty[0] = dirty;
    surface->fcValue[0] = fc_value;
    surface->fcValueUpper[0] = fc_value_upper;
    surface->compressed = compressed;
#endif
}

static struct wl_viv_interface wl_viv_implementation =
{
    viv_handle_create_buffer,
    enable_tile_status,
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

#if defined(__linux__) && defined(EGL_API_FB) && !defined(__APPLE__)

extern void fbdev_SetServerTag(VEGLDisplay Display);
extern void fbdev_UnSetServerTag(VEGLDisplay Display);

#else
void fbdev_SetServerTag(VEGLDisplay Display)
{
}
void fbdev_UnSetServerTag(VEGLDisplay Display)
{
}
#endif

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

    /* If not fbdev backend, ignore this */
    /* Tag backend fbdev which is used at wayland server side*/
    fbdev_SetServerTag(Display);

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

    /* If not fbdev backend, ignore this */
    fbdev_UnSetServerTag(Display);

    return EGL_TRUE;
}

EGLBoolean
veglQueryWaylandBuffer(
    VEGLDisplay Display,
    struct wl_resource *Buffer,
    EGLint * Width,
    EGLint * Height,
    EGLint * Fd,
    gcoSURF * Surface
    )
{
    struct wl_viv_buffer *buffer;

    if (!wl_resource_instance_of(Buffer,
                                  &wl_buffer_interface,
                                  &wl_viv_buffer_implementation))
    {
        /* Not a wl_viv_buffer. */
        return EGL_FALSE;
    }

    buffer = wl_resource_get_user_data(Buffer);

    if (Width)
    {
        *Width = buffer->width;
    }

    if (Height)
    {
        *Height = buffer->height;
    }

    if (Fd)
    {
        *Fd = buffer->fd;
    }

    if (Surface)
    {
        *Surface = buffer->surface;
    }

    return EGL_TRUE;
}