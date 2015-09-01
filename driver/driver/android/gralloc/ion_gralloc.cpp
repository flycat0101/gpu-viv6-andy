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


#include "ion_gralloc.h"

#include <sys/mman.h>
#include <ion/ion.h>
#include <linux/ion.h>

#include "gralloc_priv.h"

#include "gc_gralloc_helper.h"


/* Global ion fd. */
static int ionfd = -1;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void ion_gralloc_init(void)
{
    /* Open ion. */
    ionfd = ion_open();
}

/*****************************************************************************/

static int ion_gralloc_map(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    private_handle_t* hnd = (private_handle_t*)handle;

    void* address = mmap(0, hnd->size, PROT_READ | PROT_WRITE,
            MAP_SHARED, hnd->fd, 0);

    if (address == MAP_FAILED) {
        ALOGE("%s: could not mmap - %s", __FUNCTION__, strerror(errno));
        return -errno;
    }

    hnd->base = (int) address;
    return 0;
}

static int ion_gralloc_unmap( gralloc_module_t const* module,
        buffer_handle_t handle)
{
    private_handle_t* hnd = (private_handle_t*) handle;

    if (!hnd->base) {
        return 0;
    }

    if (munmap((void*) hnd->base, hnd->size) < 0) {
        ALOGE("%s :could not unmap - %s %x %d", __FUNCTION__, strerror(errno),
                hnd->base, hnd->size);
    }

    hnd->base = 0;
    return 0;
}

static int ion_gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    int err = 0;
    int fd = -1;
    struct ion_handle* ion_hnd;
    unsigned long phys;
    unsigned int ion_flags = 0;

    err = ion_alloc(ionfd, size, 4096, ion_flags, &ion_hnd);

    if (err) {
        ALOGE("%s: ion_alloc failed - %s", __FUNCTION__, strerror(-err));
        return err;
    }

    err = ion_share(ionfd, ion_hnd, &fd);

    if (err) {
        ion_free(ionfd, ion_hnd);
        ALOGE("%s: ion_share failed - %s", __FUNCTION__, strerror(-err));
        return err;
    }

    private_handle_t* hnd = new private_handle_t(fd, size, 0);

    gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
            dev->common.module);

    err = ion_gralloc_map(module, hnd);

    if (err) {
        ion_free(ionfd, ion_hnd);
        close(fd);
        delete hnd;
        return err;
    }

    /* TODO: Get physical address. */
    /*
    err = ion_phys(ionfd, ion_hnd, &phys);
    if (err) {
        munmap((void*) hnd->base, hnd->size);
        ion_free(ionfd, ion_hnd);
        close(fd);
        delete hnd;
        ALOGE("%s: ion_phys failed - %s", __FUNCTION__, strerror(errno));

        return err;
    }
     */

    ion_free(ionfd, ion_hnd);

    *pHandle = hnd;
    return 0;
}

int ion_gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
    size_t size, stride;

    if (!pHandle || !pStride) {
        return -EINVAL;
    }

    /* Init ion gralloc allocator. */
    pthread_once(&once_control, ion_gralloc_init);

    if (ionfd < 0) {
        ALOGE("%s: ion_open failed", __FUNCTION__);
        return -EINVAL;
    }

    int align = 4;
    int bpp = 0;
    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bpp = 3;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
        case HAL_PIXEL_FORMAT_RAW_SENSOR:
            bpp = 2;
            break;
#if ANDROID_SDK_VERSION >= 17
        case HAL_PIXEL_FORMAT_BLOB:
            bpp = 1;
            break;
#endif
        default:
            ALOGE("%s: format %d is not supported", __FUNCTION__, format);
            return -EINVAL;
    }

    size_t bpr = (w*bpp + (align-1)) & ~(align-1);
    size = bpr * h;
    stride = bpr / bpp;

    /*
     * XXX: Vivante HAL needs 16 pixel alignment in width and 4 pixel
     * alignment in height.
     *
     * Here we assume the buffer will be used by Vivante HAL...
     */
    bpr = ((w + 15) & ~15) * bpp;
    size = bpr * ((h + 3) & ~3);
    stride = bpr / bpp;

    int err;
    err = ion_gralloc_alloc_buffer(dev, size, usage, pHandle);

    if (err < 0) {
        return err;
    }

    *pStride = stride;

    /*
     * XXX: Wrap it into Vivante driver so that Vivante HAL can
     * access the buffer.
     */
    {
        private_handle_t* hnd = (private_handle_t*) *pHandle;
        /* TODO: physical address. */
        unsigned long phys = ~0U;
        void* vaddr = (void*)hnd->base;

        err = gc_gralloc_wrap(hnd, w, h, format, stride, phys, vaddr);
        if (err) {
            LOGE("%s: failed to wrap", __FUNCTION__);
        }
    }
    return 0;
}

int ion_gralloc_free(alloc_device_t* dev, buffer_handle_t handle)
{
    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    private_handle_t const* hnd =
            reinterpret_cast<private_handle_t const*>(handle);

    gralloc_module_t* module =
            reinterpret_cast<gralloc_module_t*>(dev->common.module);

    /* XXX: Un-wrap the buffer. */
    gc_gralloc_unwrap(const_cast<private_handle_t*>(hnd));

    if (hnd->base) {
        ion_gralloc_unmap(module, const_cast<private_handle_t*>(hnd));
    }

    close(hnd->fd);

    delete hnd;
    return 0;
}

int ion_gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    int err;

    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    /* Init ion gralloc allocator. */
    pthread_once(&once_control, ion_gralloc_init);

    if (ionfd < 0) {
        ALOGE("%s: ion_open failed", __FUNCTION__);
        return -EINVAL;
    }

    err = ion_gralloc_map(module, handle);
    if (err) {
        return err;
    }

    /*
     * XXX: Wrap it into Vivante driver so that Vivante HAL can
     * access the buffer in client side.
     */
    {
        private_handle_t* hnd = (private_handle_t*)handle;
        /* TODO: physical address. */
        unsigned long phys = ~0U;
        void* vaddr = (void*)hnd->base;

        err = gc_gralloc_register_wrap(hnd, phys, vaddr);
        if (err) {
            LOGE("%s: failed to register wrap", __FUNCTION__);
        }
    }

    return err;
}

int ion_gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    /* XXX: Un-wrap. */
    gc_gralloc_unwrap((private_handle_t*) handle);

    ion_gralloc_unmap(module, handle);

    return 0;
}

int ion_gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int left, int top, int width, int height,
        void** vaddr)
{
    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    private_handle_t* hnd = (private_handle_t*) handle;

    if (!hnd->base) {
        ion_gralloc_map(module, handle);
    }

    *vaddr = (void*) hnd->base;

    return 0;
}

int ion_gralloc_unlock(gralloc_module_t const* module, buffer_handle_t handle)
{
    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    /* TODO: Flush cache if allocated for SW read/write. */
    /*
    private_handle_t* hnd = (private_handle_t*) handle;

    ion_sync_fd(ionfd, hnd->fd);
     */

    /* inform buffer update. */
    gc_gralloc_notify_change((private_handle_t*) handle);

    return 0;
}

