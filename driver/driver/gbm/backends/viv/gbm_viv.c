/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <xf86drm.h>

#include "gbm_vivint.h"
#include "gbmint.h"

#include <gc_egl_common.h>

#define VIV_BACKEND_NAME "viv"

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

/**
 * For 2.4.66 DRM,there is no DRM_RDWR
 * but 2.4.88 DRM, there is DRM_RDWR
 * not sure which version toolchain will use
 * test if DRM_RDWR is defined
 */
#ifndef DRM_RDWR
#define DRM_RDWR O_RDWR
#endif

struct gbm_backend gbm_viv_backend;

gceSTATUS
gbm_viv_query_attrib_from_image(
    void *eglImage,
    gctINT *width,
    gctINT *height,
    gctINT *stride,
    gceSURF_FORMAT *format,
    void **pixel,
    uint32_t *handle
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF   surface = gcvNULL;
    khrEGL_IMAGE *image = (khrEGL_IMAGE_PTR)eglImage;

    if (handle)
        *handle = 0xFFFFFFFF;

    /* Get texture attributes. */
    switch (image->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
    case KHR_IMAGE_RENDER_BUFFER:
    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
    case KHR_IMAGE_WAYLAND_BUFFER:
    case KHR_IMAGE_VIV_DEC:
    case KHR_IMAGE_PIXMAP:
    case KHR_IMAGE_LINUX_DMA_BUF:
        {
            surface = image->surface;

            if (!surface)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if ((width != gcvNULL) && (height != gcvNULL))
            {
                /* Query width and height from source. */
                gcmONERROR(
                    gcoSURF_GetSize(surface,
                                    (gctUINT_PTR) width,
                                    (gctUINT_PTR) height,
                                    gcvNULL));
            }

            if (format != gcvNULL)
            {
                /* Query source surface format. */
                gcmONERROR(gcoSURF_GetFormat(surface, gcvNULL, format));
            }

            if (stride != gcvNULL)
            {
                /* Query srouce surface stride. */
                gcmONERROR(gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, stride));
            }

            if (pixel != gcvNULL)
            {
                if(image->type == KHR_IMAGE_PIXMAP)
                {
                    struct gbm_viv_bo *bo=(struct gbm_viv_bo *)image->u.pixmap.nativePixmap;
                    *pixel = bo->map;
                }
            }

            if (handle != gcvNULL && image->type == KHR_IMAGE_PIXMAP)
            {
                struct gbm_viv_bo *bo=(struct gbm_viv_bo *)image->u.pixmap.nativePixmap;
                *handle = bo->base.handle.u32;
            }

        }
        break;
    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}


#ifdef WL_EGL_PLATFORM
#include <wayland-server.h>
#include <wl-viv-buffer.h>
uint32_t gbm_viv_query_waylandbuffer(
    void *Buffer,
    int32_t * Width,
    int32_t * Height,
    int32_t *Stride,
    int32_t *Format,
    int * Fd
    )
{
    struct wl_viv_buffer *buffer;
    gceSTATUS status = gcvSTATUS_OK;
    buffer = wl_resource_get_user_data((struct wl_resource *)Buffer);

    if (Format)
    {
        gcmONERROR(gcoSURF_GetFormat(buffer->surface, gcvNULL, (gceSURF_FORMAT *)Format));
    }

    if (Width)
    {
        *Width = buffer->width;
    }

    if (Height)
    {
        *Height = buffer->height;
    }

    if (Stride)
    {
        gctUINT wx;
        gctUINT hx;
        gcmONERROR(gcoSURF_GetAlignedSize(buffer->surface, &wx, &hx, Stride));
    }

    if (Fd)
    {
        *Fd = buffer->fd;
        if (buffer->fd < 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    return 1;
OnError:
    return 0;
}
#else
uint32_t gbm_viv_query_waylandbuffer(
    void *Buffer,
    int32_t * Width,
    int32_t * Height,
    int32_t *Stride,
    int32_t *Format,
    int * Fd
    )
{
    return 0;
}
#endif

gceSTATUS
gbm_viv_get_gbm_format(
    gceSURF_FORMAT HalFormat,
    uint32_t *GbmFormat
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    uint32_t gbmFormat = 0;
    int i;

    if (GbmFormat)
    {
        for (i = 0; i < gcmCOUNTOF(_gGBMFormatTable); i++)
        {
            if (HalFormat == _gGBMFormatTable[i].halFormat)
            {
                gbmFormat = _gGBMFormatTable[i].gbmFormat;
                break;
            }
        }

        if (gbmFormat == 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        *GbmFormat = gbmFormat;
    }

OnError:
    return status;
}

static int
gbm_viv_is_format_supported(
    struct gbm_device *gbm,
    uint32_t format,
    uint32_t usage
    )
{
    /* Check format. */
    switch (format) {
    case GBM_FORMAT_RGB565:
    case GBM_FORMAT_XRGB8888:
    case GBM_FORMAT_XBGR8888:
    case GBM_FORMAT_ARGB8888:
    case GBM_FORMAT_ABGR8888:
        break;
    default:
        return 0;
    }

    return 1;
}

static int
gbm_viv_bo_write(
    struct gbm_bo *_bo,
    const void *buf,
    size_t count
    )
{
    struct gbm_viv_bo *bo = gbm_viv_bo(_bo);
    memcpy(bo->map, buf, count);
    return 0;
}

static int
gbm_viv_bo_get_fd(struct gbm_bo *_bo)
{
    struct gbm_viv_bo *bo = gbm_viv_bo(_bo);

    if (bo->fd < 0)
    {
        int32_t fd;

        if (drmPrimeHandleToFD(_bo->gbm->fd, _bo->handle.u32, DRM_RDWR | DRM_CLOEXEC, &fd) == 0)
        {
            bo->fd = fd;
        }
    }

    return bo->fd;
}

static void *
gbm_viv_bo_map_fd(struct gbm_viv_bo *bo)
{
   struct drm_mode_map_dumb map_arg;
   int ret;

   if (bo->map != NULL)
      return bo->map;

   memset(&map_arg, 0, sizeof(map_arg));
   map_arg.handle = bo->base.handle.u32;

   ret = drmIoctl(bo->base.gbm->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);
   if (ret)
      return NULL;

   bo->map = mmap(0, bo->size, PROT_WRITE,
                  MAP_SHARED, bo->base.gbm->fd, map_arg.offset);

   if (bo->map == MAP_FAILED)
   {
      bo->map = NULL;
      return NULL;
   }

   return bo->map;
}

static void
gbm_viv_bo_unmap_fd(struct gbm_viv_bo *bo)
{
   munmap(bo->map, bo->size);
   bo->map = NULL;
}

static struct gbm_bo *
gbm_viv_bo_import(
    struct gbm_device *gbm,
    uint32_t type,
    void *buffer,
    uint32_t usage
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct gbm_viv_device *dev = gbm_viv_device(gbm);
    struct gbm_import_fd_data *fd_data = buffer;
    struct gbm_viv_bo *bo = NULL;
    struct drm_prime_handle prime_handle;
    struct gbm_import_fd_data wl_fd_data={0};
    int ret;

    if (type == GBM_BO_IMPORT_EGL_IMAGE)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Wrap a import_fd_data for WL_BUFFER */
    if (type == GBM_BO_IMPORT_WL_BUFFER)
    {
        int32_t w,h,stride,fd,format;
        if (gbm_viv_query_waylandbuffer(buffer, &w, &h, &stride, &format, &fd))
        {
            wl_fd_data.fd = (int)fd;
            wl_fd_data.width = (uint32_t)w;
            wl_fd_data.height = (uint32_t)h;
            gbm_viv_get_gbm_format((gceSURF_FORMAT)format, &wl_fd_data.format);
            wl_fd_data.stride = (uint32_t)stride;
            fd_data = &wl_fd_data;
        } else {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    bo = calloc(1, sizeof *bo);
    if (bo == NULL)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    bo->base.gbm = gbm;
    bo->type = type;
    bo->fd = -1;

    if(type == GBM_BO_IMPORT_FD || type == GBM_BO_IMPORT_WL_BUFFER)
    {
        if (!gbm_viv_is_format_supported(gbm, fd_data->format, usage))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        prime_handle.fd = fd_data->fd;
        ret = drmIoctl(dev->base.fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &prime_handle);
        if (ret)
        {
            fprintf(stderr, "DRM_IOCTL_PRIME_FD_TO_HANDLE failed "
                    "(fd=%u)\n", prime_handle.fd);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        bo->base.handle.u32 = prime_handle.handle;
        bo->base.width = fd_data->width;
        bo->base.height = fd_data->height;
        bo->base.stride = fd_data->stride;
        bo->base.format = fd_data->format;
        bo->size = fd_data->height * fd_data->stride;

        if (gbm_viv_bo_map_fd(bo) == NULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        if (type == GBM_BO_IMPORT_EGL_IMAGE)
        {
            gctINT width;
            gctINT height;
            gctINT stride;
            gceSURF_FORMAT format;
            void *pixel;
            uint32_t handle;
            gcmONERROR(gbm_viv_query_attrib_from_image(buffer, &width, &height, &stride, &format, &pixel, &handle));
            bo->base.handle.u32 = handle;
            bo->base.width = width;
            bo->base.height = height;
            bo->base.stride = stride;
            bo->base.format = format;
            bo->size = height * stride;
            bo->map = pixel;
        } else {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    return &bo->base;

OnError:

    if (bo)
    {
        free(bo);
        bo = NULL;
    }

    return NULL;
}

static void *
gbm_viv_bo_map(
    struct gbm_bo *_bo,
    uint32_t x, uint32_t y,
    uint32_t width, uint32_t height,
    uint32_t flags, uint32_t *stride,
    void **map_data
    )
{
    struct gbm_viv_bo *bo = gbm_viv_bo(_bo);
    /* If it's a dumb buffer, we already have a mapping */
    if (bo->map)
    {
        *map_data = (char *)bo->map + (bo->base.stride * y) + (x * 4);
        *stride = bo->base.stride;
        return *map_data;
    }
    return NULL;
}

static void
gbm_viv_bo_unmap(
    struct gbm_bo *_bo,
    void *map_data
    )
{
    struct gbm_viv_bo *bo = gbm_viv_bo(_bo);
    /* Check if it's a dumb buffer and check the pointer is in range */
    if (bo->map)
    {
        gcmASSERT(map_data >= bo->map);
        gcmASSERT(map_data < (bo->map + bo->size));
    }
    return;
}

static uint32_t
_get_bpp_from_format(uint32_t format)
{
    uint32_t ret = 0;

    switch (format) {
    case GBM_FORMAT_RGB565:
        ret = 16;
        break;
    case GBM_FORMAT_XRGB8888:
    case GBM_FORMAT_XBGR8888:
    case GBM_FORMAT_ARGB8888:
    case GBM_FORMAT_ABGR8888:
        ret = 32;
        break;
    default:
        ret = 0;
        break;
    }

    return ret;
}

static struct gbm_bo *
create_dumb(
    struct gbm_device *gbm,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t usage
    )
{
    struct gbm_viv_device *dev = gbm_viv_device(gbm);
    struct drm_mode_create_dumb create_arg;
    struct gbm_viv_bo *bo;
    struct drm_mode_destroy_dumb destroy_arg;
    int ret;

    bo = calloc(1, sizeof *bo);
    if (bo == NULL)
        return NULL;

    memset(&create_arg, 0, sizeof(create_arg));
    create_arg.bpp = _get_bpp_from_format(format);
    create_arg.width = width;
    create_arg.height = height;

    ret = drmIoctl(dev->base.fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
    if (ret)
        goto free_bo;

    bo->base.gbm = gbm;
    bo->base.width = width;
    bo->base.height = height;
    bo->base.stride = create_arg.pitch;
    bo->base.format = format;
    bo->base.handle.u32 = create_arg.handle;
    bo->size = create_arg.size;
    bo->fd = -1;

    if (gbm_viv_bo_map_fd(bo) == NULL)
        goto destroy_dumb;

    return &bo->base;

destroy_dumb:
    memset(&destroy_arg, 0, sizeof destroy_arg);
    destroy_arg.handle = create_arg.handle;
    drmIoctl(dev->base.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_arg);
free_bo:
    free(bo);

    return NULL;
}

static struct gbm_bo *
gbm_viv_bo_create(
    struct gbm_device *gbm,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t usage,
    const uint64_t *modifiers,
    const unsigned int count
    )
{
    if (modifiers)
    {
        return NULL;
    }
    else
    {
        return create_dumb(gbm, width, height, format, usage);
    }
}

static int
gbm_viv_bo_get_planes(struct gbm_bo *_bo)
{
   /* Dumb buffers are single-plane only. */
    return 1;
}

static union gbm_bo_handle
gbm_viv_bo_get_handle_for_plane(
    struct gbm_bo *_bo,
    int plane
    )
{
    union gbm_bo_handle ret;
    ret.s32 = -1;

    errno = ENOSYS;
    return ret;
}

static uint32_t
gbm_viv_bo_get_stride(
    struct gbm_bo *_bo,
    int plane
    )
{
    /* Preserve legacy behavior if plane is 0 */
    if (plane == 0)
        return _bo->stride;

    errno = ENOSYS;
    return 0;

}

static uint32_t
gbm_viv_bo_get_offset(
    struct gbm_bo *_bo,
    int plane
    )
{
    return 0;
}

static uint64_t
gbm_viv_bo_get_modifier(struct gbm_bo *_bo)
{
    return DRM_FORMAT_MOD_LINEAR;
}

static void
gbm_viv_bo_destroy(struct gbm_bo *_bo)
{
    struct gbm_viv_device *dev = gbm_viv_device(_bo->gbm);
    struct gbm_viv_bo *bo = gbm_viv_bo(_bo);

    {
        struct drm_mode_destroy_dumb arg = {
            .handle = bo->base.handle.u32
        };

        gbm_viv_bo_unmap_fd(bo);

        drmIoctl(dev->base.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);

        if (bo->fd >= 0)
        {
            close(bo->fd);
            bo->fd = -1;
        }
    }

    free(bo);

    return;
}

gceSTATUS
gbm_viv_get_hal_format(
    uint32_t GbmFormat,
    gceSURF_FORMAT *HalFormat
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
    int i;

    if (HalFormat)
    {
        for (i = 0; i < gcmCOUNTOF(_gGBMFormatTable); i++)
        {
            if (GbmFormat == _gGBMFormatTable[i].gbmFormat)
            {
                halFormat = _gGBMFormatTable[i].halFormat;
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
gbm_viv_create_buffers(
    struct gbm_viv_surface *surf,
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

    gcmONERROR(gbm_viv_get_hal_format(format, &gc_format));
    gcmONERROR(gcoSURF_GetAlignment(gcvSURF_BITMAP, gc_format, NULL, &xalignment, NULL));
    alligned_width = gcmALIGN_NP2(width, xalignment);

    for (i = 0; i < surf->buffer_count; i++)
    {
        gcoSURF rtSurf = gcvNULL;
        struct gbm_bo *bo = gbm_viv_bo_create(surf->base.gbm, alligned_width, height, format, usage, 0, 0);

        if (!bo)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        surf->buffers[i].bo = bo;

        gcmONERROR(gcoSURF_WrapUserMemory(
            gcvNULL,
            width,
            height,
            bo->stride,
            1,
            gcvSURF_BITMAP,
            gc_format,
            gbm_viv_bo_get_fd(bo),
            gcvALLOC_FLAG_DMABUF,
            &rtSurf));

        surf->buffers[i].surf = surf;
        surf->buffers[i].render_surface = rtSurf;
        surf->buffers[i].status = FREE;
    }

    return gcvSTATUS_OK;

OnError:
    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].render_surface)
        {
            gcoSURF_Destroy(surf->buffers[i].render_surface);
        }

        if (surf->buffers[i].bo)
        {
            gbm_viv_bo_destroy(surf->buffers[i].bo);
        }

        surf->buffers[i].render_surface = NULL;
        surf->buffers[i].bo = NULL;
    }

    return status;
}

static struct gbm_surface *
gbm_viv_surface_create(
    struct gbm_device *gbm,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t flags,
    const uint64_t *modifiers,
    const unsigned count
    )
{
    gceSTATUS status;
    uint32_t usage = GBM_BO_USE_SCANOUT;
    struct gbm_viv_surface *surf = calloc(1, sizeof(*surf));
    if (!surf || modifiers)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    surf->buffer_count = GBM_MAX_BUFFER;
    surf->base.gbm = gbm;
    surf->base.width = width;
    surf->base.height = height;
    surf->base.format = format;
    surf->base.flags = flags;

    gcmONERROR(gbm_viv_create_buffers(surf, width, height,
        format, usage));

    return &surf->base;

OnError:
    /*Create gbm_bo failed */
    if (surf)
        free(surf);

    return NULL;
}

static struct gbm_bo *
gbm_viv_surface_lock_front_buffer(
    struct gbm_surface *surface
    )
{
    struct gbm_viv_surface *surf = (struct gbm_viv_surface *) surface;
    struct gbm_bo *bo = NULL;
    int i;

    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].status == FRONT_BUFFER)
        {
            surf->buffers[i].status = LOCKED_BY_CLIENT;
            bo = surf->buffers[i].bo;
        }
    }

    return bo;
}

static void
gbm_viv_surface_release_buffer(
    struct gbm_surface *surface,
    struct gbm_bo *bo
    )
{
    struct gbm_viv_surface *surf = (struct gbm_viv_surface *) surface;
    int i;

    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].bo == bo)
        {
            surf->buffers[i].status = FREE;
            break;
        }
    }

    return;
}

static int
gbm_viv_surface_has_free_buffers(
    struct gbm_surface *surface
    )
{
    struct gbm_viv_surface *surf = (struct gbm_viv_surface *) surface;
    int i;

    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].status == FREE)
        {
            return 1;
        }
    }

    return 0;
}

static void
gbm_viv_surface_destroy(
    struct gbm_surface *surface
    )
{
    struct gbm_viv_surface *surf = (struct gbm_viv_surface *) surface;
    int i;

    if (!surf)
        return;

    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].render_surface != NULL)
        {
            gcoSURF_Destroy(surf->buffers[i].render_surface);
            surf->buffers[i].render_surface = NULL;
        }
    }

    /* Flush hardware to scheduled surface free to takeplace */
    gcoHAL_Commit(gcvNULL, gcvTRUE);

    for (i = 0; i < surf->buffer_count; i++)
    {
        if (surf->buffers[i].bo != NULL)
        {
            gbm_viv_bo_destroy(surf->buffers[i].bo);
            surf->buffers[i].bo = NULL;
        }
    }

    free(surf);

    return;
}

static void
gbm_viv_destroy(struct gbm_device *gbm)
{
    struct gbm_viv_device *dev = gbm_viv_device(gbm);

    if (dev)
        free(dev);

    return;
}

static struct gbm_device *
viv_device_create(int fd)
{
    struct gbm_viv_device *dev;

    dev = calloc(1, sizeof *dev);
    if (!dev)
        return NULL;

    dev->base.fd = fd;
    dev->base.bo_create = gbm_viv_bo_create;
    dev->base.bo_import = gbm_viv_bo_import;
    dev->base.bo_map = gbm_viv_bo_map;
    dev->base.bo_unmap = gbm_viv_bo_unmap;
    dev->base.is_format_supported = gbm_viv_is_format_supported;
    dev->base.bo_write = gbm_viv_bo_write;
    dev->base.bo_get_fd = gbm_viv_bo_get_fd;
    dev->base.bo_get_planes = gbm_viv_bo_get_planes;
    dev->base.bo_get_handle = gbm_viv_bo_get_handle_for_plane;
    dev->base.bo_get_stride = gbm_viv_bo_get_stride;
    dev->base.bo_get_offset = gbm_viv_bo_get_offset;
    dev->base.bo_get_modifier = gbm_viv_bo_get_modifier;
    dev->base.bo_destroy = gbm_viv_bo_destroy;
    dev->base.destroy = gbm_viv_destroy;
    dev->base.surface_create = gbm_viv_surface_create;
    dev->base.surface_lock_front_buffer = gbm_viv_surface_lock_front_buffer;
    dev->base.surface_release_buffer = gbm_viv_surface_release_buffer;
    dev->base.surface_has_free_buffers = gbm_viv_surface_has_free_buffers;
    dev->base.surface_destroy = gbm_viv_surface_destroy;
    dev->base.name = gbm_viv_backend.backend_name;

    return &dev->base;
}

struct gbm_backend gbm_viv_backend = {
    .backend_name = VIV_BACKEND_NAME,
    .create_device = viv_device_create,
};
