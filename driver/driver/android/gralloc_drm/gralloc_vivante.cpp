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


#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>

#include <log/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 29)
#include <hardware/gralloc1.h>
#endif
#include <graphics_ext.h>

/* from libdrm. */
#include <vivante_drm.h>
#include <vivante_bo.h>

#include "gralloc_util.h"

/* to implement. */
#include <gralloc_handle.h>
#include <gralloc_vivante_bo.h>

#include "gralloc_vivante.h"

#define ALIGN(val, align) (((val) + (align) - 1) & ~((align) - 1))

static int Skip_lastpixel = -1;

struct gralloc_vivante_t {
    gralloc_module_t *module;

    /* drm driver fd. */
    int fd;
    struct drm_vivante *drm;
};

static int gralloc_vivante_getpid(void)
{
    static int32_t gralloc_vivante_pid = 0;

    if (unlikely(!gralloc_vivante_pid))
        android_atomic_write((int32_t) getpid(), &gralloc_vivante_pid);

    return gralloc_vivante_pid;
}

static int gralloc_vivante_get_bpp(int format)
{
    int bpp;

    switch (format) {
    case HAL_PIXEL_FORMAT_RGBA_1010102:
        bpp = 4;
        break;
    case HAL_PIXEL_FORMAT_RGBA_FP16:
        bpp = 8;
        break;
    case HAL_PIXEL_FORMAT_Y16:
    case HAL_PIXEL_FORMAT_RAW16:
        bpp = 2;
        break;
    case HAL_PIXEL_FORMAT_Y8:
    case HAL_PIXEL_FORMAT_BLOB:
        bpp = 1;
        break;
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        bpp = 4;
        break;
    case HAL_PIXEL_FORMAT_RGB_888:
        bpp = 3;
        break;
    case HAL_PIXEL_FORMAT_RGB_565:
    case HAL_PIXEL_FORMAT_YCbCr_422_888: /* YUY2 */
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        bpp = 2;
        break;
    /* planar; only Y is considered */
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
    // case HAL_PIXEL_FORMAT_DRM_NV12:
        bpp = 1;
        break;
    case HAL_PIXEL_FORMAT_CbYCrY_422_I: /* UYVY, graphics_ext */
        bpp = 2;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_P: /* I420, graphics_ext */
    case HAL_PIXEL_FORMAT_YCbCr_420_SP: /* NV12, graphics_ext */
        bpp = 1;
        break;
    default:
        bpp = 0;
        break;
    }

    return bpp;
}

static void buffer_size_quirk(buffer_handle_t handle, size_t size)
{
    gralloc_handle_t *hnd = (gralloc_handle_t *)handle;
    hnd->size = (int)size;
}

static void baseaddr_quirk(buffer_handle_t handle,
                struct gralloc_vivante_bo_t *bo)
{
    const int sw_usage = GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK;
    if (gralloc_handle_usage(handle) & sw_usage) {
        if (!bo->vaddr) {
            gralloc_trace(2, "do mmap");
            /* the driver is supposed to wait for the bo */
            drm_vivante_bo_mmap(bo->bo, &bo->vaddr);
        }

        gralloc_handle_t *hnd = (gralloc_handle_t *)handle;
        hnd->base = (uintptr_t)bo->vaddr;
    }
}

int gralloc_vivante_validate_buffer_size(buffer_handle_t handle,uint32_t *v_size,
                    uint32_t *v_tiling,int *v_create_ts,int *v_stride)
{
    uint32_t size;
    int align_w, align_h, bpp, stride;
    uint32_t tiling = 0;
    int create_ts = 0;

    gralloc_trace(0, "handle=%p usage=0x%x", handle, gralloc_handle_usage(handle));

    /* tiling & alignment. */
    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TILED_VIV) {
        tiling = DRM_VIV_GEM_TILING_SUPERTILED;
        align_w = align_h = 64;

        if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TS_VIV)
            create_ts = 1;
    }
    else {
        tiling = DRM_VIV_GEM_TILING_LINEAR;
        align_w = 16;
        align_h = 4;
    }

    /* format & bpp. */
    bpp = gralloc_vivante_get_bpp(gralloc_handle_format(handle));
    if (!bpp) {
        /* not supported. */
        gralloc_trace_error(1, "unknown format=%x",
            gralloc_handle_format(handle));
        return -1;
    }

    if (bpp == 3)
    {
        bpp = 4;
        gralloc_trace(2, "Be careful!!! format=%x bpp=%d", gralloc_handle_format(handle), bpp);
    }

    stride = ALIGN(gralloc_handle_width(handle), align_w) * bpp;
    size = stride * ALIGN(gralloc_handle_height(handle), align_h);

    /* u,v planes for planar formats. */
    switch (gralloc_handle_format(handle)) {
    case HAL_PIXEL_FORMAT_YV12:
        size += ALIGN(stride / 2, 16) *
                ALIGN(gralloc_handle_height(handle), align_h);
        /* HW need read 64bytes at least for YUV420 tex. */
        size += 64;
        break;
    // case HAL_PIXEL_FORMAT_DRM_NV12:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        size += size / 2;
        size += 64;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        size <<= 1;
        size += 64;
        break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        size += size / 2;
        size += 64;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_P: /* I420, graphics_ext */
        size += ALIGN(stride / 2, 16) *
                ALIGN(gralloc_handle_height(handle), align_h);
        size += 64;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_SP: /* NV12, grapphics_ext */
        size += size / 2;
        size += 64;
        break;
    default:
        /* In case there is NO SH_IMAGE_LD_LAST_PIXEL_FIX. */
        /*On chipModel=gc3000 && ChipRevision=0x5450 SOC, don't need allocate more 64bytes,
        * otherwise, there is little square on the upper left corner.
        */
        if(Skip_lastpixel < 0)
            size += 64;

        break;
    }

    gralloc_handle_set_stride(handle, stride / bpp);

    if (v_size != NULL) {
        *v_size = size;
    }
    if (v_tiling != NULL) {
        *v_tiling = tiling;
    }
    if (v_create_ts != NULL) {
        *v_create_ts = create_ts;
    }
    if (v_stride != NULL) {
        *v_stride = stride;
    }

    return 0;
}

static struct gralloc_vivante_bo_t *
gralloc_vivante_alloc_bo(struct gralloc_vivante_t *drv, buffer_handle_t handle)
{
    struct gralloc_vivante_bo_t *bo;
    int stride;
    uint32_t size;
    uint32_t flags = 0;
    uint32_t tiling = 0;
    uint32_t gem_handle;
    int create_ts = 0;
    int err;

    gralloc_trace(0, "handle=%p usage=0x%x", handle, gralloc_handle_usage(handle));

    err = gralloc_vivante_validate_buffer_size(handle,&size,&tiling,&create_ts,&stride);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return NULL;
    }

    /* flags. */
    if ((gralloc_handle_usage(handle) & GRALLOC_USAGE_SW_WRITE_OFTEN) ||
            (gralloc_handle_usage(handle) & GRALLOC_USAGE_SW_READ_OFTEN))
        flags |= DRM_VIV_GEM_CACHED;

    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_PROTECTED)
        flags |= DRM_VIV_GEM_SECURE;

    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_FB)
        flags |= DRM_VIV_GEM_CONTIGUOUS;

    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_COMPOSER) {
        /* Ignore cache for layer buffer */
        flags &= ~DRM_VIV_GEM_CACHED;
        flags |= DRM_VIV_GEM_CMA_LIMIT;
    }

    bo = (struct gralloc_vivante_bo_t *)calloc(1, sizeof(*bo));
    if (!bo) {
        gralloc_trace_error(1, "out of memory");
        return NULL;
    }

    gralloc_trace(2, "pid=%d usage=0x%x size=%u(0x%x) "
        "w=%d h=%d format=%x stride=%d tiling=0x%x flags=%x fd=%d",
        gralloc_handle_data_owner(handle), gralloc_handle_usage(handle),
        size, size, gralloc_handle_width(handle), gralloc_handle_height(handle),
        gralloc_handle_format(handle), stride, tiling, flags,
        gralloc_handle_fd(handle));

    if (gralloc_handle_fd(handle) >= 0) {
        err = drm_vivante_bo_import_from_fd(drv->drm,
                gralloc_handle_fd(handle), &bo->bo);
        if (err) {
            gralloc_trace_error(2, "failed to wrap bo: fd=%d err=%d(%s)",
                gralloc_handle_fd(handle), err, strerror(-err));
            goto err;
        }
    } else {
        uint32_t ts_mode;
        int fd = -1;

        if (create_ts) {
            ts_mode = DRM_VIV_GEM_TS_DISABLED;
            err = drm_vivante_bo_create_with_ts(drv->drm, flags, size, &bo->bo);
        } else {
            ts_mode = DRM_VIV_GEM_TS_NONE;
            err = drm_vivante_bo_create(drv->drm, flags, size, &bo->bo);
        }

        if (err) {
            gralloc_trace_error(2, "failed to create bo: err=%d(%s)", err, strerror(-err));
            goto err;
        }

        err = drm_vivante_bo_export_to_fd(bo->bo, &fd);
        if (err) {
            gralloc_trace_error(2, "failed to export prime fd: err=%d(%s)", err, strerror(-err));
            goto err_unref;
        }
        gralloc_handle_set_fd(handle, fd);

        struct drm_vivante_bo_tiling tiling_args = {
            .tiling_mode = tiling,
            .ts_mode = ts_mode,
            .clear_value = 0,
        };
        drm_vivante_bo_set_tiling(bo->bo, &tiling_args);

        if (!(gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_RENDER)
#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 29)
            /* also zero memory for GPU linear buffers */
            || ((gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_TEXTURE) &&
            (gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_RENDER) &&
            tiling == DRM_VIV_GEM_TILING_LINEAR)
            /* clear some virtual display related buffers to avoid blur issue*/
            || ((gralloc_handle_usage(handle) & GRALLOC1_PRODUCER_USAGE_CPU_READ_OFTEN) &&
            tiling == DRM_VIV_GEM_TILING_LINEAR)
#endif
            ) {
            /* zero memory for non GPU buffers. */
            void *vaddr = NULL;

            err = drm_vivante_bo_mmap(bo->bo, &vaddr);
            if (!err) {
                memset(vaddr, 0, size);
                drm_vivante_bo_munmap(bo->bo);
            }
        }
    }

    drm_vivante_bo_get_handle(bo->bo, &gem_handle);

    gralloc_trace(2, "fd=%d drm-bo=%p gem_handle=%d",
        gralloc_handle_fd(handle), bo->bo, gem_handle);

    buffer_size_quirk(handle, size);

    bo->magic = GRALLOC_VIVANTE_BO_MAGIC;
    bo->fb_handle = gem_handle;
    bo->refcount = 1;

    gralloc_trace(1, "ok: return bo=%p", bo);
    return bo;

err_unref:
    drm_vivante_bo_destroy(bo->bo);
err:
    free(bo);
    gralloc_trace_error(1, "no bo");
    return NULL;
}

static void gralloc_vivante_free_bo(struct gralloc_vivante_t *drv,
                    struct gralloc_vivante_bo_t *bo)
{
    gralloc_trace(0, "bo=%p", bo);

    drm_vivante_bo_destroy(bo->bo);
    free(bo);

    gralloc_trace(1, "done");
}

static void gralloc_vivante_decref_bo(struct gralloc_vivante_t *drv,
                    struct gralloc_vivante_bo_t *bo)
{
    gralloc_trace(0, "bo=%p refcount=%d", bo, bo->refcount);

    if (!--bo->refcount)
        gralloc_vivante_free_bo(drv, bo);

    gralloc_trace(1, "done");
}

static int gralloc_vivante_open_drm(const char *path,
                int *pFd, struct drm_vivante **pDrm)
{
    int fd;
    int err;

    gralloc_trace(0, "try open %s", path);

    fd = open(path, O_RDWR);
    if (fd < 0) {
        gralloc_trace(1, "failed to open file: err=%d(%s)",
                -errno, strerror(errno));
        return -errno;
    }

    err = drm_vivante_create(fd, pDrm);
    if (err) {
        gralloc_trace(1, "failed to create drm on fd=%d: err=%d(%s)",
                fd, -errno, strerror(errno));
        close(fd);
        return err;
    }

    *pFd = fd;
    gralloc_trace(1, "ok: out fd=%d drm=%p", fd, *pDrm);
    return 0;
}

int gralloc_vivante_create(gralloc_module_t const *module,
            struct gralloc_vivante_t **pDrv)
{
    char path[PROPERTY_VALUE_MAX];
    struct gralloc_vivante_t *drv = NULL;
    int fd = -1;
    int err;

    gralloc_trace(0, "module=%p", module);

    drv = (struct gralloc_vivante_t *)calloc(1, sizeof(*drv));
    if (!drv) {
        gralloc_trace_error(1, "out of memory");
        return -ENOMEM;
    }

    property_get("gralloc.drm.device", path, "");

    if (path[0] != '\0') {
        err = gralloc_vivante_open_drm(path, &fd, &drv->drm);
    } else {
        int d;
        err = -EINVAL;
        for (d = 128; err && (d < 128 + 16); d++) {
            snprintf(path, PROPERTY_VALUE_MAX, "/dev/dri/renderD%u", d);
            err = gralloc_vivante_open_drm(path, &fd, &drv->drm);
        }
    }

    memset(path, 0, sizeof path);
    property_get("ro.boot.soc_type", path, "");

    if(path[0] != '\0')
    {
        if(strcmp(path, "imx6qp") == 0)
            Skip_lastpixel = 1;
    }

    if (err) {
        gralloc_trace_error(2, "failed to create vivante drm");
        goto error;
    }

    drv->module = const_cast<gralloc_module_t *>(module);
    drv->fd = fd;
    *pDrv = drv;
    gralloc_trace(1, "ok: fd=%d drm=%p drv=%p", fd, drv->drm, drv);
    return 0;

error:
    if (fd > 0)
        close(fd);
    if (drv)
        free(drv);
    gralloc_trace_error(1, "err=%d(%s)", err, strerror(-err));
    return err;
}

void gralloc_vivante_destroy(struct gralloc_vivante_t *drv)
{
    gralloc_trace(0, "drv=%p", drv);

    drm_vivante_close(drv->drm);
    close(drv->fd);
    free(drv);

    gralloc_trace(1, "done");
}

int gralloc_vivante_alloc(struct gralloc_vivante_t *drv, int w, int h,
            int format, int usage, buffer_handle_t* pHandle, int* pStride)
{
    buffer_handle_t handle;
    struct gralloc_vivante_bo_t *bo;

    gralloc_trace(0, "drv=%p w=%d h=%d format=%d usage=0x%x",
            drv, w, h, format, usage);

    handle = gralloc_handle_create(w, h, format, usage);
    if (!handle) {
        gralloc_trace_error(1, "out of memory");
        *pHandle = NULL;
        return -ENOMEM;
    }

    bo = gralloc_vivante_alloc_bo(drv, handle);
    if (!bo) {
        gralloc_trace_error(1, "failed to allocate bo");
        gralloc_handle_free(handle);
        return -ENOMEM;
    }

    /* update data. */
    gralloc_handle_set_data(handle, bo, gralloc_vivante_getpid());

    *pHandle = handle;
    *pStride = gralloc_handle_stride(handle);

    baseaddr_quirk(handle, bo);

    gralloc_trace(1, "ok: out handle=%p stride=%d", *pHandle, *pStride);
    return 0;
}

int gralloc_vivante_free(struct gralloc_vivante_t *drv, buffer_handle_t handle)
{
    int err;
    int fd;
    struct gralloc_vivante_bo_t *bo;
    gralloc_trace(0, "drv=%p handle=%p", drv, handle);

    err = gralloc_handle_validate(handle);
    if (err) {
        gralloc_trace_error(1, "invalid handle");
        return -EINVAL;
    }

    bo = gralloc_vivante_bo_from_handle(handle);
    if (!bo) {
        gralloc_trace_error(1, "invalid bo=%p", gralloc_handle_data(handle));
        return -EINVAL;
    }
    gralloc_vivante_decref_bo(drv, bo);

    fd = gralloc_handle_fd(handle);
    if (fd >= 0)
        close(fd);

    gralloc_handle_free(handle);
    gralloc_trace(1, "ok");
    return 0;
}

int gralloc_vivante_register_buffer(struct gralloc_vivante_t *drv,
            buffer_handle_t handle)
{
    int err;
    struct gralloc_vivante_bo_t *bo;

    gralloc_trace(0, "drv=%p handle=%p", drv, handle);

    err = gralloc_handle_validate(handle);
    if (err) {
        gralloc_trace_error(1, "invalid handle");
        return err;
    }

    if (gralloc_handle_data_owner(handle) != gralloc_vivante_getpid()) {
        bo = gralloc_vivante_alloc_bo(drv, handle);
        if (!bo) {
            gralloc_trace_error(1, "failed to register bo");
            return -EINVAL;
        }
        /* update data. */
        gralloc_handle_set_data(handle, bo, gralloc_vivante_getpid());
    } else {
        bo = gralloc_vivante_bo_from_handle(handle);
        if (!bo) {
            gralloc_trace_error(1, "invalid handle");
            return -EINVAL;
        }
        bo->refcount++;
        gralloc_trace(2, "bo=%p refcount=%d", bo, bo->refcount);
    }

    baseaddr_quirk(handle, bo);

    gralloc_trace(1, "ok");
    return 0;
}

int gralloc_vivante_unregister_buffer(struct gralloc_vivante_t *drv,
            buffer_handle_t handle)
{
    int err;
    struct gralloc_vivante_bo_t *bo;
    gralloc_trace(0, "drv=%p handle=%p", drv, handle);

    err = gralloc_handle_validate(handle);
    if (err) {
        gralloc_trace_error(1, "invalid handle");
        return err;
    }

    bo = gralloc_vivante_bo_from_handle(handle);
    if (!bo) {
        gralloc_trace_error(1, "invalid bo=%p", gralloc_handle_data(handle));
        return -EINVAL;
    }
    gralloc_vivante_decref_bo(drv, bo);

    gralloc_trace(1, "ok");
    return 0;
}

int gralloc_vivante_lock(struct gralloc_vivante_t *drv, buffer_handle_t handle,
            int usage, int l, int t, int w, int h, void** vaddr)
{
    int err;
    struct gralloc_vivante_bo_t *bo;

    gralloc_trace(0, "drv=%p handle=%p usage=0x%x rect=[%d,%d,%d,%d]",
        drv, handle, usage, l, t, w, h);

    bo = gralloc_vivante_bo_from_handle(handle);
    if (!bo) {
        gralloc_trace_error(1, "invalid bo");
        return -EINVAL;
    }

    gralloc_trace(2, "bo=%p lock_count=%d locked_for=0x%x",
            bo, bo->lock_count, bo->locked_for);

    if ((gralloc_handle_usage(handle) & usage) != usage) {
        /* make FB special for testing software renderer with */

        if (!(gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_FB) &&
            !(gralloc_handle_usage(handle) & GRALLOC_USAGE_HW_TEXTURE)) {
            gralloc_trace_error(1, "invalid bo usage=0x%x/0x%x",
                    gralloc_handle_usage(handle), usage);
            return -EINVAL;
        }
    }


    usage |= bo->locked_for;

    if (usage & (GRALLOC_USAGE_SW_WRITE_MASK |
             GRALLOC_USAGE_SW_READ_MASK)) {
        if (!bo->vaddr) {
            gralloc_trace(2, "do mmap");
            /* the driver is supposed to wait for the bo */
            err = drm_vivante_bo_mmap(bo->bo, &bo->vaddr);
            if (err) {
                gralloc_trace_error(1, "failed to mmap");
                return err;
            }
        }
        *vaddr = bo->vaddr;

        baseaddr_quirk(handle, bo);
    } else {
        /* kernel handles the synchronization here */
        *vaddr = NULL;
    }

    bo->lock_count++;
    bo->locked_for = usage;

    gralloc_trace(1, "ok: out vaddr=%p", *vaddr);
    return 0;
}

int gralloc_vivante_unlock(struct gralloc_vivante_t *drv,
            buffer_handle_t handle)
{
    struct gralloc_vivante_bo_t *bo;

    gralloc_trace(0, "drv=%p handle=%p", drv, handle);

    bo = gralloc_vivante_bo_from_handle(handle);
    if (!bo) {
        gralloc_trace_error(1, "invalid bo");
        return -EINVAL;
    }

    gralloc_trace(2, "bo=%p lock_count=%d locked_for=0x%x",
            bo, bo->lock_count, bo->locked_for);

    if (!bo->lock_count) {
        gralloc_trace(1, "still locked, skip");
        return 0;
    }

    drm_vivante_bo_inc_timestamp(bo->bo, NULL);

    if (!--bo->lock_count) {
        if (bo->vaddr) {
            /*
            gralloc_trace(2, "do munmap");
            drm_vivante_bo_munmap(bo->bo);
            bo->vaddr = NULL;
             */
        }
        bo->locked_for = 0;
    }

    gralloc_trace(1, "ok");
    return 0;
}

int gralloc_vivante_lock_ycbcr(struct gralloc_vivante_t *drv,
            buffer_handle_t handle, int usage, int l, int t, int w, int h,
            struct android_ycbcr *ycbcr)
{
    int err;
    struct gralloc_vivante_bo_t *bo;
    int stride, vstride;
    void *ptr;

    gralloc_trace(0, "drv=%p handle=%p usage=0x%x rect=[%d,%d,%d,%d]",
        drv, handle, usage, l, t, w, h);

    bo = gralloc_vivante_bo_from_handle(handle);
    if (!bo) {
        gralloc_trace_error(1, "invalid bo");
        return -EINVAL;
    }

    gralloc_trace(2, "bo=%p lock_count=%d locked_for=0x%x",
            bo, bo->lock_count, bo->locked_for);

    switch (gralloc_handle_format(handle)) {
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        break;
    /* graphics_ext. */
    case HAL_PIXEL_FORMAT_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        break;
    default:
        gralloc_trace_error(1, "not supported format=%x",
                gralloc_handle_format(handle));
        return -EINVAL;
    }

    err = gralloc_vivante_lock(drv, handle, usage, l, t, w, h, &ptr);
    if (err) {
        gralloc_trace_error(1, "err=%d(%s)", err, strerror(-err));
        return err;
    }

    stride = gralloc_handle_stride(handle);
    vstride = ALIGN(gralloc_handle_height(handle), 4);

    switch (gralloc_handle_format(handle)) {
    case HAL_PIXEL_FORMAT_YV12:
        ycbcr->y = ptr;
        ycbcr->cr = (uint8_t *)ptr + stride * vstride;
        ycbcr->cb = (uint8_t *)ycbcr->cr + ALIGN(stride / 2, 16) * vstride / 2;
        ycbcr->ystride = stride;
        ycbcr->cstride = ALIGN(stride / 2, 16);
        ycbcr->chroma_step = 1;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        ycbcr->y = ptr;
        ycbcr->cb = (uint8_t *)ptr + stride * vstride;
        ycbcr->cr = (uint8_t *)ycbcr->cb + 1;
        ycbcr->ystride = stride;
        ycbcr->cstride = stride;
        ycbcr->chroma_step = 2;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        ycbcr->y = ptr;
        ycbcr->cb = (uint8_t *)ptr + stride * vstride;
        ycbcr->cr = (uint8_t *)ycbcr->cb + 1;
        ycbcr->ystride = stride;
        ycbcr->cstride = stride;
        ycbcr->chroma_step = 2;
        break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        ycbcr->y = ptr;
        ycbcr->cr = (uint8_t *)ptr + stride * vstride;
        ycbcr->cb = (uint8_t *)ycbcr->cb + 1;
        ycbcr->ystride = stride;
        ycbcr->cstride = stride;
        ycbcr->chroma_step = 2;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_P: /* graphics_ext. */
        ycbcr->y = ptr;
        ycbcr->cb = (uint8_t *)ptr + stride * vstride;
        ycbcr->cr = (uint8_t *)ycbcr->cb + ALIGN(stride / 2, 16) * vstride / 2;
        ycbcr->ystride = stride;
        ycbcr->cstride = ALIGN(stride / 2, 16);
        ycbcr->chroma_step = 1;
        break;
    case HAL_PIXEL_FORMAT_YCbCr_420_SP: /* graphics_ext. */
        ycbcr->y = ptr;
        ycbcr->cb = (uint8_t *)ptr + stride * vstride;
        ycbcr->cr = (uint8_t *)ycbcr->cb + 1;
        ycbcr->ystride = stride;
        ycbcr->cstride = stride;
        ycbcr->chroma_step = 2;
        break;
    default:
        break;
    }

    gralloc_trace(1, "ok");
    return 0;
}

int gralloc_vivante_perform(struct gralloc_vivante_t *drv,
            int operation, va_list args)
{
    int err;

    gralloc_trace(0, "drv=%p operation=%d", drv, operation);

    switch (operation) {
        case (int)GRALLOC_VIVANTE_PERFORM_GET_DRM_FD:
        {
            int *fd = va_arg(args, int *);
            *fd = drv->fd;
            err = 0;
        }
        break;
    default:
        gralloc_trace_error(2, "unknown operation=%d", operation);
        err = -EINVAL;
        break;
    }

    gralloc_trace(1, "done: err=%d(%s)", err, strerror(-err));
    return err;
}

