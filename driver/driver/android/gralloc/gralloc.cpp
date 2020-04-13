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


#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include "gralloc_priv.h"

/* Vivante gralloc buffer allocator */
#include "gc_gralloc.h"
/* Framebuffer (buffer allocator) */
#include "gralloc_fb.h"


/*****************************************************************************/

struct gralloc_context_t
{
    alloc_device_t device;
};


/*****************************************************************************/

static int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

static int gralloc_alloc(struct alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride);

static int gralloc_free(struct alloc_device_t* dev,
        buffer_handle_t handle);

static int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

static int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

static int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr);

#if ANDROID_SDK_VERSION >= 18
static int gralloc_lock_ycbcr(struct gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        struct android_ycbcr *ycbcr);
#endif

static int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle);

static int gralloc_perform(struct gralloc_module_t const* module,
        int operation, ... );


/*****************************************************************************/

#ifndef GRALLOC_VIV_HARDWARE_MODULE_ID
#define GRALLOC_VIV_HARDWARE_MODULE_ID "gralloc_viv"
#endif
static struct hw_module_methods_t gralloc_module_methods =
{
    open: gralloc_device_open
};

struct private_module_t HAL_MODULE_INFO_SYM =
{
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            version_major: 1,
            version_minor: 0,
            id: GRALLOC_VIV_HARDWARE_MODULE_ID,
            name: "Graphics Memory Allocator Module",
            author: "Vivante Corporation",
            methods: &gralloc_module_methods,
        },

        registerBuffer: gralloc_register_buffer,
        unregisterBuffer: gralloc_unregister_buffer,
        lock: gralloc_lock,
        unlock: gralloc_unlock,
        perform: gralloc_perform,
#if ANDROID_SDK_VERSION >= 18
        lock_ycbcr: gralloc_lock_ycbcr,
#endif
    },

    framebuffer: 0,
    numBuffers: 0,
    bufferMask: 0,
    lock: PTHREAD_MUTEX_INITIALIZER,
    currentBuffer: 0,
};


/*****************************************************************************/

static int gralloc_close(struct hw_device_t* dev)
{
    gralloc_context_t* ctx =
        reinterpret_cast<gralloc_context_t*>(dev);

    if (ctx != NULL) {
        free(ctx);
    }

    return 0;
}

int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    int rel = -EINVAL;

    if (!strcmp(name, GRALLOC_HARDWARE_GPU0)) {
        /* Open alloc device. */
        gralloc_context_t* dev;
        dev = (gralloc_context_t*) malloc(sizeof (*dev));

        if (!dev) {
            return -ENOMEM;
        }

        /* Initialize our state here */
        memset(dev, 0, sizeof (*dev));

        /* initialize the procs */
        dev->device.common.tag     = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module  = const_cast<hw_module_t*>(module);
        dev->device.common.close   = gralloc_close;
        dev->device.alloc          = gralloc_alloc;
        dev->device.free           = gralloc_free;

        *device = &dev->device.common;
        rel = 0;
    } else if (!strncmp(name, GRALLOC_HARDWARE_FB0, 2)) {
        /* Open framebuffer device. */
        rel = fb_device_open(module, name, device);
    } else {
        LOGE("Invalid device name: %s", name);
    }

    return rel;
}


/*****************************************************************************/

/*
 * gralloc functions below.
 * Please dispatch calls to specific allocators according usage.
 */

int gralloc_alloc(struct alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
    int rel;

    /* TODO (Soc-vendor): Redirect to specific allocator according to usage. */
    if (usage & GRALLOC_USAGE_HW_FB) {
        /* Dispatch to framebuffer allocator. */
        rel = gralloc_alloc_framebuffer(dev,
                w, h, format, usage, pHandle, pStride);
    } else {
        rel = gc_gralloc_alloc(dev, w, h, format, usage, pHandle, pStride);
    }

    return rel;
}

int gralloc_free(struct alloc_device_t* dev,
        buffer_handle_t handle)

{
    int rel;
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        rel = gralloc_free_framebuffer(dev, handle);
    } else {
        rel = gc_gralloc_free(dev, handle);
    }

    return rel;
}

int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    int rel;
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        /* Do not need to register for framebuffer. */
        return -EINVAL;
    }

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    rel = gc_gralloc_register_buffer(module, handle);

    return rel;
}

int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    int rel;
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        /* Do not need to unregister for framebuffer. */
        return -EINVAL;
    }

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    rel = gc_gralloc_unregister_buffer(module, handle);

    return rel;
}

int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{
    int rel;
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        if (vaddr) {
            *vaddr = (void*) hnd->base;
        }

        rel = 0;
    } else {
        rel = gc_gralloc_lock(module, handle, usage, l, t, w, h, vaddr);
    }

    return rel;
}

#if ANDROID_SDK_VERSION >= 18
int gralloc_lock_ycbcr(struct gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        struct android_ycbcr *ycbcr)
{
    int rel;

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    {
        rel = gc_gralloc_lock_ycbcr(module, handle, usage, l, t, w, h, ycbcr);
    }

    return rel;
}
#endif

int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    int rel;
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    /* TODO (Soc-vendor): Redirect to specific allocator. */
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        /* Do not need to unregister for framebuffer. */
        rel = 0;
    } else {
        rel = gc_gralloc_unlock(module, handle);
    }

    return rel;
}

int gralloc_perform(struct gralloc_module_t const* module,
        int operation, ... )
{
    /* TODO (Soc-vendor): Redirect to specific allocator. */
    return 0;
}

