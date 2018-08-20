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


#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <hardware/gralloc.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "gralloc_vivante.h"
#include "gralloc_util.h"

/* required by trace functions. */
uint32_t __gralloc_traceEnabled;
__thread int __gralloc_traceDepth;

struct vivante_module_t
{
    struct gralloc_module_t base;
    pthread_mutex_t mutex;
    struct gralloc_vivante_t *viv_gr;
};

static int gralloc_init(vivante_module_t *mod)
{
    int err = 0;

    pthread_mutex_lock(&mod->mutex);
    if (!mod->viv_gr)
        err = gralloc_vivante_create(&mod->base, &mod->viv_gr);
    pthread_mutex_unlock(&mod->mutex);

    return err;
}

static int gralloc_alloc(struct alloc_device_t* dev, int w, int h,
                int format, int usage, buffer_handle_t* pHandle, int* pStride)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)dev->common.module;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p w=%d h=%d format=%d usage=0x%x",
        dev, w, h, format, usage);

    err = gralloc_vivante_alloc(mod->viv_gr, w, h, format, usage, pHandle, pStride);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok: out handle=%p stride=%d", *pHandle, *pStride);
    return 0;
}

static int gralloc_free(struct alloc_device_t* dev, buffer_handle_t handle)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)dev->common.module;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p handle=%p", dev, handle);

    err = gralloc_vivante_free(mod->viv_gr, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_register_buffer(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = gralloc_vivante_register_buffer(mod->viv_gr, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_unregister_buffer(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = gralloc_vivante_unregister_buffer(mod->viv_gr, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_lock(gralloc_module_t const* module,
                buffer_handle_t handle, int usage,
                int l, int t, int w, int h, void** vaddr)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = gralloc_vivante_lock(mod->viv_gr, handle, usage, l, t, w, h, vaddr);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok: out vaddr=%p", *vaddr);
    return 0;
}

static int gralloc_unlock(gralloc_module_t const* module,
                buffer_handle_t handle)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p", module, handle);

    err = gralloc_vivante_unlock(mod->viv_gr, handle);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_perform(struct gralloc_module_t const* module,
        int operation, ... )
{
    int err;
    va_list args;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p operation=%d", module, operation);

    err = gralloc_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    va_start(args, operation);
    err = gralloc_vivante_perform(mod->viv_gr, operation, args);
    va_end(args);

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_lock_ycbcr(struct gralloc_module_t const* module,
                buffer_handle_t handle, int usage, int l, int t, int w, int h,
                struct android_ycbcr *ycbcr)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p handle=%p usage=0x%08X rect=[%d,%d,%d,%d]",
        module, handle, usage, l, t, w, h);

    err = gralloc_vivante_lock_ycbcr(mod->viv_gr,
                handle, usage, l, t, w, h, ycbcr);

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_close_gpu0(struct hw_device_t *dev)
{
    struct vivante_module_t *mod = (struct vivante_module_t *)dev->module;
    struct alloc_device_t *alloc = (struct alloc_device_t *)dev;

    gralloc_trace_init();
    gralloc_trace(0, "dev=%p mod=%p", dev, mod);

    if (mod->viv_gr) {
        gralloc_vivante_destroy(mod->viv_gr);
        mod->viv_gr = NULL;
    }

    free(alloc);
    gralloc_trace(1, "ok");
    return 0;
}

static int gralloc_open_gpu0(struct vivante_module_t *mod,
                struct hw_device_t **dev)
{
    int err;
    struct alloc_device_t *alloc;

    gralloc_trace(0, "mod=%p", mod);

    err = gralloc_init(mod);
    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    alloc = (struct alloc_device_t *)calloc(1, sizeof(*alloc));
    if (!alloc) {
        gralloc_vivante_destroy(mod->viv_gr);
        mod->viv_gr = NULL;
        gralloc_trace_error(1, "out of memory");
        return -ENOMEM;
    }

    alloc->common.tag = HARDWARE_DEVICE_TAG;
    alloc->common.version = 0;
    alloc->common.module = &mod->base.common;
    alloc->common.close = gralloc_close_gpu0;
    alloc->alloc = gralloc_alloc;
    alloc->free = gralloc_free;

    *dev = &alloc->common;
    gralloc_trace(1, "ok: out dev=%p", *dev);
    return 0;
}

static int gralloc_open_fb0(struct vivante_module_t *mod,
                struct hw_device_t **dev)
{
    gralloc_trace_error(2, "fb device not supported");
    return -EINVAL;
}

static int gralloc_open(const struct hw_module_t *module,
        const char *name, struct hw_device_t **dev)
{
    int err;
    struct vivante_module_t *mod = (struct vivante_module_t *)module;

    gralloc_trace_init();
    gralloc_trace(0, "module=%p name=%s", module, name);

    if (strcmp(name, GRALLOC_HARDWARE_GPU0) == 0)
        err = gralloc_open_gpu0(mod, dev);
    else if (strcmp(name, GRALLOC_HARDWARE_FB0) == 0)
        err = gralloc_open_fb0(mod, dev);
    else
        err = -EINVAL;

    if (err) {
        gralloc_trace_error(1, "err=%d", err);
        return err;
    }

    gralloc_trace(1, "ok: out dev=%p", *dev);
    return 0;
}

static struct hw_module_methods_t gralloc_methods = {
    .open = gralloc_open
};

#ifndef GRALLOC_VIV_HARDWARE_MODULE_ID
#  define GRALLOC_VIV_HARDWARE_MODULE_ID "gralloc_viv"
#endif

struct vivante_module_t HAL_MODULE_INFO_SYM = {
    .base = {
        .common = {
            .tag = HARDWARE_MODULE_TAG,
            .version_major = 1,
            .version_minor = 0,
            .id = GRALLOC_VIV_HARDWARE_MODULE_ID,
            .name = "Vivante DRM Memory Allocator",
            .author = "Zongzong Yan",
            .methods = &gralloc_methods
        },
        .registerBuffer = gralloc_register_buffer,
        .unregisterBuffer = gralloc_unregister_buffer,
        .lock = gralloc_lock,
        .unlock = gralloc_unlock,
        .perform = gralloc_perform,
        .lock_ycbcr = gralloc_lock_ycbcr,
    },
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .viv_gr = NULL,
};
