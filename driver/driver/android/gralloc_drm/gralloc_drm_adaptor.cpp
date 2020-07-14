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

/* from libdrm. */
#include <vivante_drm.h>
#include <vivante_bo.h>

/* from drm_gralloc. */
#include <gralloc_drm.h>
#include <gralloc_drm_handle.h>
#include <gralloc_drm_vivante.h>

#include "gralloc_util.h"

/* to implement. */
#include <gralloc_handle.h>
#include <gralloc_vivante_bo.h>

/******************************************************************************/

#if defined(__LP64__)
#  define HAL_LIBRARY_PATH1 "/system/lib64/hw"
#  define HAL_LIBRARY_PATH2 "/vendor/lib64/hw"
#  define HAL_LIBRARY_PATH3 "/odm/lib64/hw"
#else
#  define HAL_LIBRARY_PATH1 "/system/lib/hw"
#  define HAL_LIBRARY_PATH2 "/vendor/lib/hw"
#  define HAL_LIBRARY_PATH3 "/odm/lib/hw"
#endif

#define DRM_GRALLOC_NAME "gralloc.drm.so"

struct drm_adaptor_t {
    gralloc_module_t base;
    pthread_mutex_t mutex;
    struct gralloc_module_t *drmGralloc;
    struct alloc_device_t *drmAlloc;
};

struct drm_adaptor_bo_t {
    struct gralloc_vivante_bo_t base;
    struct gralloc_drm_handle_t *drmHandle;
};

uint32_t __gralloc_traceEnabled;
__thread int __gralloc_traceDepth;

static int drm_adaptor_getpid(void)
{
    static int32_t drm_adaptor_pid = 0;

    if (unlikely(!drm_adaptor_pid))
        android_atomic_write((int32_t) getpid(), &drm_adaptor_pid);

    return (int)drm_adaptor_pid;
}

static int load(struct gralloc_module_t **pHmi)
{
    void *handle;
    struct hw_module_t *hmi = NULL;
    char path[256];
    int found = 0;
    const char *hal_library_path[] = {
        HAL_LIBRARY_PATH1,
        HAL_LIBRARY_PATH2,
        HAL_LIBRARY_PATH3
    };

    gralloc_trace(0, "");

    for (int i = 0; i < 3; i++) {
        snprintf(path, sizeof(path), "%s/" DRM_GRALLOC_NAME, hal_library_path[i]);

        if (access(path, R_OK) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        gralloc_trace(1, "ERROR: gralloc.drm.so not found");
        return -EINVAL;
    }

    handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        char const *err_str = dlerror();
        gralloc_trace(1, "ERROR: module=%s err=%s", path, err_str ? err_str : "unknown");
        return -EINVAL;
    }

    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
    hmi = (struct hw_module_t *)dlsym(handle, sym);
    if (!hmi) {
        dlclose(handle);
        gralloc_trace(1, "ERROR: couldn't find symbol %s", sym);
        return -EINVAL;
    }

    hmi->dso = handle;

    *pHmi = (struct gralloc_module_t *)hmi;
    gralloc_trace(1, "ok: out hmi=%p", hmi);
    return 0;
}

static int drm_adaptor_init(struct drm_adaptor_t *mod)
{
    int err = 0;

    pthread_mutex_lock(&mod->mutex);
    if (!mod->drmGralloc)
        err = load(&mod->drmGralloc);
    pthread_mutex_unlock(&mod->mutex);

    return err;
}

static drm_adaptor_bo_t *to_adaptor_bo(buffer_handle_t handle)
{
    struct drm_adaptor_bo_t *abo;

    abo = (struct drm_adaptor_bo_t *)gralloc_handle_data(handle);
    if (!abo || abo->base.magic != GRALLOC_VIVANTE_BO_MAGIC) {
        gralloc_trace_string("ERROR: invalid drm_adaptor_bo!");
        return NULL;
    }
    return abo;
}

static buffer_handle_t to_drm_handle(buffer_handle_t handle)
{
    drm_adaptor_bo_t *abo = to_adaptor_bo(handle);
    if (!abo)
        return NULL;
    return (buffer_handle_t)abo->drmHandle;
}

static struct drm_adaptor_bo_t *alloc_adaptor_bo(buffer_handle_t handle,
                                    buffer_handle_t drmHandle)
{
    drm_adaptor_bo_t *abo;
    struct gralloc_drm_vivante_bo_t *drmBo;

    gralloc_trace(0, "drmHandle=%p", drmHandle);

    drmBo = (struct gralloc_drm_vivante_bo_t *)
                    gralloc_drm_bo_from_handle(drmHandle);
    if (!drmBo) {
        gralloc_trace(1, "ERROR: invalid bo");
        return NULL;
    }

    abo = new drm_adaptor_bo_t;
    if (!abo) {
        gralloc_trace(1, "ERROR: out of memory");
        return NULL;
    }

    abo->base.magic = GRALLOC_VIVANTE_BO_MAGIC;
    abo->base.handle = handle;

    abo->base.imported = 0;
    abo->base.fb_handle = 0;

    abo->base.lock_count = 0;
    abo->base.locked_for = 0;

    abo->base.refcount = 1;
    abo->base.bo = drmBo->bo;

    abo->drmHandle = gralloc_drm_handle(drmHandle);

    gralloc_trace(1, "ok: return abo=%p", abo);
    return abo;
}

static void destroy_buffer(struct drm_adaptor_t *mod,
                struct drm_adaptor_bo_t *abo)
{
    int err;
    buffer_handle_t handle = abo->base.handle;
    buffer_handle_t drmHandle = (buffer_handle_t)abo->drmHandle;

    if (abo->base.imported) {
        err = mod->drmGralloc->unregisterBuffer(mod->drmGralloc, drmHandle);
        if (err) {
            gralloc_trace_error(2, "err=%d", err);
        }

        /* this drmHandle is malloc'ed in this module. */
        delete abo->drmHandle;
    } else {
        err = mod->drmAlloc->free(mod->drmAlloc, drmHandle);
        if (err) {
            gralloc_trace_error(2, "err=%d", err);
        }
    }

    gralloc_handle_set_data(handle, NULL, 0);
    delete abo;
}

static int decref_handle(struct drm_adaptor_t *mod,
                buffer_handle_t handle)
{
    int err;
    gralloc_trace(0, "mod=%p handle=%p", mod, handle);

    err = gralloc_handle_validate(handle);
    if (err) {
        gralloc_trace_error(1, "invalid handle");
        return -EINVAL;
    }

    drm_adaptor_bo_t *abo = to_adaptor_bo(handle);
    if (!abo) {
        gralloc_trace_error(1, "invalid bo");
        return -EINVAL;
    }

    if (--abo->base.refcount == 0)
        destroy_buffer(mod, abo);

    gralloc_trace(1, "ok");
    return 0;
}

/* gralloc_drm_handle_t to new buffer_handle_t. */
static int alloc_handle(buffer_handle_t drmHandle, buffer_handle_t *pHandle)
{
    int err;
    gralloc_drm_handle_t *_drmHandle;
    buffer_handle_t handle;

    gralloc_trace(0, "drmHandle=%p", drmHandle);

    _drmHandle = gralloc_drm_handle(drmHandle);
    if (!_drmHandle) {
        gralloc_trace(1, "ERROR: invalid drmHandle=%p", drmHandle);
        return -EINVAL;
    }


    handle = gralloc_handle_create(_drmHandle->width, _drmHandle->height,
                    _drmHandle->format, _drmHandle->usage);
    if (!handle) {
        gralloc_trace_error(1, "failed to create handle");
        return -ENOMEM;
    }

    gralloc_trace(1, "ok: out handle=%p", handle);
    *pHandle = handle;
    return 0;
}

/* buffer_handle_t to new gralloc_drm_handle_t. */
static int alloc_drm_handle(buffer_handle_t handle, buffer_handle_t *pDrmHandle)
{
    gralloc_drm_handle_t *drmHandle;
    int err;

    gralloc_trace(0, "handle=%p fd=%d", handle, gralloc_handle_fd(handle));

    drmHandle = new gralloc_drm_handle_t;
    if (!drmHandle) {
        gralloc_trace(1, "ERROR: out of memory");
        return -ENOMEM;
    }

    drmHandle->base.version = sizeof(drmHandle->base);
    drmHandle->base.numInts = GRALLOC_DRM_HANDLE_NUM_INTS;
    drmHandle->base.numFds = GRALLOC_DRM_HANDLE_NUM_FDS;

    drmHandle->magic = GRALLOC_DRM_HANDLE_MAGIC;
    drmHandle->width = gralloc_handle_width(handle);
    drmHandle->height = gralloc_handle_height(handle);
    drmHandle->format = gralloc_handle_format(handle);
    drmHandle->usage = gralloc_handle_usage(handle);
    drmHandle->prime_fd = gralloc_handle_fd(handle);
    drmHandle->data = (struct gralloc_drm_bo_t *)(uintptr_t)0x1234dead;
    drmHandle->data_owner = gralloc_handle_data_owner(handle);

    *pDrmHandle = (buffer_handle_t)drmHandle;
    gralloc_trace(1, "ok: out drmHandle=%p", drmHandle);
    return 0;
}


static int drm_adaptor_alloc(struct alloc_device_t* dev, int w, int h,
                int format, int usage, buffer_handle_t* pHandle, int* pStride)
{
    int err;
    buffer_handle_t drmHandle;
    buffer_handle_t handle;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)dev->common.module;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p mod=%p w=%d h=%d format=%d usage=0x%08X",
            dev, mod, w, h, format, usage);

    err = mod->drmAlloc->alloc(
            mod->drmAlloc, w, h, format, usage, &drmHandle, pStride);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    /* drmHandle -> buffer_handle_t + adaptor_bo. */
    err = alloc_handle(drmHandle, &handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    drm_adaptor_bo_t *abo = alloc_adaptor_bo(handle, drmHandle);
    if (!abo) {
        gralloc_trace_error(1, "out of memory");
        gralloc_handle_free(handle);
        return -ENOMEM;
    }

    gralloc_handle_set_stride(handle, *pStride);
    /* set bo of this process. */
    gralloc_handle_set_data(handle, abo, drm_adaptor_getpid());

    *pHandle = handle;
    gralloc_trace(1, "ok: out handle=%p stride=%d", *pHandle, *pStride);
    return 0;
}

static int drm_adaptor_free(struct alloc_device_t* dev, buffer_handle_t handle)
{
    int err;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)dev->common.module;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p handle=%p", dev, handle);

    err = decref_handle(mod, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    /* free buffer_handle_t. */
    gralloc_handle_free(handle);

    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_register_buffer(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    buffer_handle_t drmHandle;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;
    struct drm_adaptor_bo_t *abo;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = drm_adaptor_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    err = gralloc_handle_validate(handle);
    if (err) {
        gralloc_trace_error(1, "invalid handle");
        return err;
    }

    if (gralloc_handle_data_owner(handle) != drm_adaptor_getpid()) {
        /* the buffer handle is passed to a new process */
        gralloc_trace(2, "pass from other process: pid=%d",
                gralloc_handle_data_owner(handle));

        /* buffer_handle_t -> drmHandle + adaptor_bo. */
        err = alloc_drm_handle(handle, &drmHandle);
        if (err) {
            gralloc_trace_error(1, "err=%d", err);
            return err;
        }
        err = mod->drmGralloc->registerBuffer(mod->drmGralloc, drmHandle);
        if (err) {
            gralloc_trace_error(1, "err=%d", err);
            return err;
        }

        abo = alloc_adaptor_bo(handle, drmHandle);
        if (!abo) {
            gralloc_trace_error(1, "out of memory");
            return -ENOMEM;
        }
        abo->base.imported = 1;

        /* set bo of this process. */
        gralloc_handle_set_data(handle, abo, drm_adaptor_getpid());
    } else {
        gralloc_trace(2, "register in same process");

        abo = to_adaptor_bo(handle);
        if (!abo) {
            gralloc_trace_error(1, "invalid bo data=%p", gralloc_handle_data(handle));
            return -EINVAL;
        }
        abo->base.refcount++;
    }
    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_unregister_buffer(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = decref_handle(mod, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_lock(gralloc_module_t const* module,
                buffer_handle_t handle, int usage,
                int l, int t, int w, int h, void** vaddr)
{
    int err;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;
    buffer_handle_t drmHandle = to_drm_handle(handle);

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p drmHandle=%p usage=0x%08X [%d,%d,%d,%d]",
            module, handle, drmHandle, usage, l, t, w, h);

    err = drm_adaptor_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    err = mod->drmGralloc->lock(mod->drmGralloc,
            drmHandle, usage, l, t, w, h, vaddr);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok: out vaddr=%p", *vaddr);
    return 0;
}

static int drm_adaptor_lock_ycbcr(struct gralloc_module_t const* module,
                buffer_handle_t handle, int usage, int l, int t, int w, int h,
                struct android_ycbcr *ycbcr)
{
    int err;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;
    buffer_handle_t drmHandle = to_drm_handle(handle);

    gralloc_trace(0, "module=%p handle=%p drmHandle=%p usage=0x%08X [%d,%d,%d,%d]",
            module, handle, drmHandle, usage, l, t, w, h);

    gralloc_trace_init();
    err = drm_adaptor_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    err = mod->drmGralloc->lock_ycbcr(mod->drmGralloc,
            drmHandle, usage, l, t, w, h, ycbcr);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_unlock(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;
    buffer_handle_t drmHandle = to_drm_handle(handle);

    gralloc_trace(0, "module=%p handle=%p drmHandle=%p", module, handle, drmHandle);

    err = mod->drmGralloc->unlock(mod->drmGralloc, drmHandle);

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_perform(struct gralloc_module_t const* module,
        int operation, ... )
{
    gralloc_trace_init();
    gralloc_trace_string("ERROR: module=%p operation=%d", module, operation);
    return -EINVAL;
}

static int drm_adaptor_close_gpu0(struct hw_device_t *dev)
{
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)dev->module;
    struct alloc_device_t *alloc = (struct alloc_device_t *)dev;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p", dev);

    mod->drmAlloc->common.close(&mod->drmAlloc->common);

    delete alloc;
    gralloc_trace(1, "ok");
    return 0;
}

static int drm_adaptor_open_gpu0(struct drm_adaptor_t *mod, hw_device_t **dev)
{
    struct alloc_device_t *alloc;
    int err;

    gralloc_trace(0, "mod=%p", mod);

    err = drm_adaptor_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    err = gralloc_open(&mod->drmGralloc->common,
                (struct alloc_device_t **)&mod->drmAlloc);

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    alloc = new alloc_device_t;
    if (!alloc) {
        gralloc_trace_error(1, "out of memory");
        return -ENOMEM;
    }

    alloc->common.tag = HARDWARE_DEVICE_TAG;
    alloc->common.version = 0;
    alloc->common.module = &mod->base.common;
    alloc->common.close = drm_adaptor_close_gpu0;
    alloc->alloc = drm_adaptor_alloc;
    alloc->free = drm_adaptor_free;

    *dev = &alloc->common;
    gralloc_trace(1, "ok: out dev=%p", *dev);
    return 0;
}

static int drm_adaptor_open_fb0(struct drm_adaptor_t *mod, hw_device_t **dev)
{
    ALOGI("open fb0");
    *dev = NULL;
    return 0;
}

static int drm_adaptor_open(const struct hw_module_t *module,
                const char *name, struct hw_device_t **dev)
{
    struct drm_adaptor_t *mod = (struct drm_adaptor_t *)module;
    int err;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p name=%p", module, name);

    if (strcmp(name, GRALLOC_HARDWARE_GPU0) == 0)
        err = drm_adaptor_open_gpu0(mod, dev);
    else if (strcmp(name, GRALLOC_HARDWARE_FB0) == 0)
        err = drm_adaptor_open_fb0(mod, dev);
    else
        err = -EINVAL;

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok: out dev=%p", *dev);
    return 0;
}

static struct hw_module_methods_t drm_adaptor_methods = {
    .open = drm_adaptor_open
};

#ifndef GRALLOC_VIV_HARDWARE_MODULE_ID
#  define GRALLOC_VIV_HARDWARE_MODULE_ID "gralloc_viv"
#endif

struct drm_adaptor_t HAL_MODULE_INFO_SYM = {
    .base = {
        .common = {
            .tag = HARDWARE_MODULE_TAG,
            .version_major = 1,
            .version_minor = 0,
            .id = GRALLOC_VIV_HARDWARE_MODULE_ID,
            .name = "DRM Memory Allocator Adaptor",
            .author = "Zongzong Yan",
            .methods = &drm_adaptor_methods
        },
        .registerBuffer = drm_adaptor_register_buffer,
        .unregisterBuffer = drm_adaptor_unregister_buffer,
        .lock = drm_adaptor_lock,
        .unlock = drm_adaptor_unlock,
        .perform = drm_adaptor_perform,
        .lock_ycbcr = drm_adaptor_lock_ycbcr,
    },

    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .drmGralloc = NULL,
    .drmAlloc = NULL,
};

