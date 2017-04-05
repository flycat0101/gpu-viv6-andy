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

#include <xf86drm.h>

#include "gbm_vivint.h"

#include "gbmint.h"

#define VIV_BACKEND_NAME "viv"

struct gbm_backend gbm_viv_backend;

static int
gbm_viv_is_format_supported(struct gbm_device *gbm,
                            uint32_t format,
                            uint32_t usage)
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
gbm_viv_bo_write(struct gbm_bo *_bo, const void *buf, size_t count)
{
   struct gbm_viv_bo *bo = gbm_viv_bo(_bo);

   memcpy(bo->map, buf, count);

   return 0;
}

static int
gbm_viv_bo_get_fd(struct gbm_bo *_bo)
{
    return 0;
}

static struct gbm_bo *
gbm_viv_bo_import(struct gbm_device *gbm,
                  uint32_t type, void *buffer, uint32_t usage)
{
    return NULL;
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

static void *
gbm_viv_bo_map(struct gbm_viv_bo *bo)
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
   if (bo->map == MAP_FAILED) {
      bo->map = NULL;
      return NULL;
   }

   return bo->map;
}

static void
gbm_viv_bo_unmap(struct gbm_viv_bo *bo)
{
   munmap(bo->map, bo->size);
   bo->map = NULL;
}

static struct gbm_bo *
create_dumb(struct gbm_device *gbm,
            uint32_t width, uint32_t height,
            uint32_t format, uint32_t usage)
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

   if (gbm_viv_bo_map(bo) == NULL)
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
gbm_viv_bo_create(struct gbm_device *gbm,
                  uint32_t width, uint32_t height,
                  uint32_t format, uint32_t usage)
{
   struct gbm_viv_bo *bo = NULL;

   {
      return create_dumb(gbm, width, height, format, usage);
   }


   if (bo)
      free(bo);

   return NULL;
}

static void
gbm_viv_bo_destroy(struct gbm_bo *_bo)
{
   struct gbm_viv_device *dev = gbm_viv_device(_bo->gbm);
   struct gbm_viv_bo *bo = gbm_viv_bo(_bo);
   struct drm_mode_destroy_dumb arg;

   {
      gbm_viv_bo_unmap(bo);
      memset(&arg, 0, sizeof(arg));
      arg.handle = bo->base.handle.u32;
      drmIoctl(dev->base.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
   }

   free(bo);

   return;
}


static void
viv_destroy(struct gbm_device *gbm)
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
   dev->base.is_format_supported = gbm_viv_is_format_supported;
   dev->base.bo_write = gbm_viv_bo_write;
   dev->base.bo_get_fd = gbm_viv_bo_get_fd;
   dev->base.bo_destroy = gbm_viv_bo_destroy;
   dev->base.destroy = viv_destroy;
   dev->base.name = gbm_viv_backend.backend_name;

   return &dev->base;
}

struct gbm_backend gbm_viv_backend = {
   .backend_name = VIV_BACKEND_NAME,
   .create_device = viv_device_create,
};
