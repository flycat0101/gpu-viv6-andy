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


#ifndef _GBM_VIV_INTERNAL_H_
#define _GBM_VIV_INTERNAL_H_

#include <sys/mman.h>
#include <gc_hal.h>
#include "gbmint.h"
#include "common_drm.h"

#define GBM_MAX_BUFFER  4

struct gbm_viv_surface;
struct gbm_viv_buffer;
struct gbm_viv_bo;

struct gbm_viv_device
{
    struct gbm_device base;

    void *driver;
};

struct gbm_viv_bo
{
    struct gbm_bo base;

    uint32_t size;
    void *map;
    uint32_t type;
    uint64_t modifier;
    int32_t fd;
    int32_t ts_fd;

    gcoSURF render_surface;
};

struct gbm_viv_buffer
{
    struct gbm_bo         *bo;
    enum {
        LOCKED_BY_CLIENT, /* taken by gbm_surface_lock_front_buffer */
        USED_BY_EGL,      /* currently in use for EGL rendering */
        FRONT_BUFFER,     /* last buffer with completed rendering */
        FREE              /* free */
    } status;
    uint32_t               bpp;

    struct gbm_viv_surface *surf;
};

struct gbm_viv_surface
{
    struct gbm_surface base;

    struct gbm_viv_buffer buffers[GBM_MAX_BUFFER];
    gctINT buffer_count;
    uint32_t bpp;
    gctBOOL extResolve;
    gctBOOL sync_post;
};

struct
{
    int gbmFormat;
    int halFormat;
}
_gGBMFormatTable[] =
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

static inline struct gbm_viv_device *
gbm_viv_device(struct gbm_device *gbm)
{
    return (struct gbm_viv_device *) gbm;
}

static inline struct gbm_viv_bo *
gbm_viv_bo(struct gbm_bo *bo)
{
    return (struct gbm_viv_bo *) bo;
}

static inline struct gbm_viv_surface *
gbm_viv_surface(struct gbm_surface *surface)
{
    return (struct gbm_viv_surface *) surface;
}

#endif
