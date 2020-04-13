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


/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/mman.h>

#include <dlfcn.h>

#include <cutils/ashmem.h>
#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/atomic.h>

#if HAVE_ANDROID_OS
#include <linux/fb.h>
#endif

#include "gralloc_priv.h"
#include "gralloc_fb.h"

#include "gc_gralloc.h"


/*
     FRAMEBUFFER_PIXEL_FORMAT

         Explictly set framebuffer pixel format.
         If it is set to '0', format is set according to the current
         bytes-per-pixel:
             HAL_PIXEL_FORMAT_RGBA_8888 for 32 bpp
             HAL_PIXEL_FORMAT_RGB_565 for 16 bpp

         Available format values are:
             HAL_PIXEL_FORMAT_RGBA_8888 : 1
             HAL_PIXEL_FORMAT_RGBX_8888 : 2
             HAL_PIXEL_FORMAT_RGB_888   : 3
             HAL_PIXEL_FORMAT_RGB_565   : 4
             HAL_PIXEL_FORMAT_BGRA_8888 : 5
 */
#define FRAMEBUFFER_PIXEL_FORMAT  0

/*
     NUM_BUFFERS

         Numbers of buffers of framebuffer device for page flipping.

         Normally it can be '2' for double buffering, or '3' for tripple
         buffering.
         This value should equal to (yres_virtual / yres).
 */
#ifndef NUM_FRAMEBUFFER_SURFACE_BUFFERS
#define NUM_BUFFERS               2
#else
#define NUM_BUFFERS               NUM_FRAMEBUFFER_SURFACE_BUFFERS
#endif


/*
     NUM_PAGES_MMAP

         Numbers of pages to mmap framebuffer to userspace.

         Normally the total buffer size should be rounded up to page boundary
         and mapped to userspace. On some platform, maping the rounded size
         will fail. Set non-zero value to specify the page count to map.
 */
#define NUM_PAGES_MMAP            0


/* Framebuffer pixel format table. */
static const struct
{
    int      format;
    uint32_t bits_per_pixel;
    uint32_t red_offset;
    uint32_t red_length;
    uint32_t green_offset;
    uint32_t green_length;
    uint32_t blue_offset;
    uint32_t blue_length;
    uint32_t transp_offset;
    uint32_t transp_length;
}
formatTable[] =
{
    {HAL_PIXEL_FORMAT_RGBA_8888, 32,  0, 8,  8, 8,  16, 8, 24, 8},
    {HAL_PIXEL_FORMAT_RGBX_8888, 32,  0, 8,  8, 8,  16, 8,  0, 0},
    {HAL_PIXEL_FORMAT_RGB_888,   24,  0, 8,  8, 8,  16, 8,  0, 0},
    {HAL_PIXEL_FORMAT_RGB_565,   16, 11, 5,  5, 6,   0, 5,  0, 0},
    {HAL_PIXEL_FORMAT_BGRA_8888, 32, 16, 8,  8, 8,   0, 8, 24, 8}
};

/* Framebuffer pixel format. */
static int format = FRAMEBUFFER_PIXEL_FORMAT;

enum {
    PAGE_FLIP = 0x00000001,
    LOCKED = 0x00000002
};

struct fb_context_t {
    framebuffer_device_t  device;
};


/*
 * The following macro are for build compatibility
 * between different android versions.
 */
#ifndef LOGE
#   define LOGE(...) ALOGE(__VA_ARGS__)
#endif

#ifndef LOGW
#   define LOGW(...) ALOGW(__VA_ARGS__)
#endif

#ifndef LOGI
#   define LOGI(...) ALOGI(__VA_ARGS__)
#endif

/*****************************************************************************/

static int fb_setSwapInterval(struct framebuffer_device_t* dev,
            int interval)
{
    // fb_context_t* ctx = (fb_context_t*)dev;
    if (interval < dev->minSwapInterval || interval > dev->maxSwapInterval)
        return -EINVAL;
    // FIXME: implement fb_setSwapInterval
    return 0;
}

static int fb_setUpdateRect(struct framebuffer_device_t* dev,
        int l, int t, int w, int h)
{
    if (((w|h) <= 0) || ((l|t)<0))
        return -EINVAL;

    // fb_context_t* ctx = (fb_context_t*)dev;
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);
    m->info.reserved[0] = 0x54445055; // "UPDT";
    m->info.reserved[1] = (uint16_t)l | ((uint32_t)t << 16);
    m->info.reserved[2] = (uint16_t)(l+w) | ((uint32_t)(t+h) << 16);
    return 0;
}

static int fb_post(struct framebuffer_device_t* dev, buffer_handle_t buffer)
{
    if (private_handle_t::validate(buffer) < 0)
        return -EINVAL;

    // fb_context_t* ctx = (fb_context_t*)dev;

    private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(buffer);
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);

    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        const size_t offset = hnd->base - m->framebuffer->base;
        m->info.activate = FB_ACTIVATE_VBL;
        m->info.yoffset = offset / m->finfo.line_length;
#ifndef FRONT_BUFFER
        if (ioctl(m->framebuffer->fd, FBIOPAN_DISPLAY, &m->info) == -1) {
            LOGE("FBIOPAN_DISPLAY failed");
            m->base.unlock(&m->base, buffer);
            return -errno;
        }
#endif
        m->currentBuffer = buffer;

    } else {
        LOGE("Cannot post this buffer");
    }

    return 0;
}

/*****************************************************************************/

static int mapFrameBufferLocked(struct private_module_t* module)
{
    // already initialized...
    if (module->framebuffer) {
        return 0;
    }

    char const * const device_template[] = {
            "/dev/graphics/fb%u",
            "/dev/fb%u",
            0 };

    int fd = -1;
    unsigned int i=0;
    char name[64];

    while ((fd==-1) && device_template[i]) {
        snprintf(name, 64, device_template[i], 0);
        fd = open(name, O_RDWR, 0);
        i++;
    }
    if (fd < 0)
        return -errno;

    struct fb_fix_screeninfo finfo;
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        close(fd);
        return -errno;
    }

    struct fb_var_screeninfo info;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1)
    {
        close(fd);
        return -errno;
    }

    info.reserved[0] = 0;
    info.reserved[1] = 0;
    info.reserved[2] = 0;
    info.xoffset = 0;
    info.yoffset = 0;
    info.activate = FB_ACTIVATE_NOW;

    /*
     * Request pixel format
     * If pixel format is specified, we should update framebuffer info.
     */
    if (format == 0) {
        /* Use default pixel format id. */
        switch (info.bits_per_pixel) {
            case 32:
            default:
                format = HAL_PIXEL_FORMAT_RGBA_8888;
                break;
            case 16:
                format = HAL_PIXEL_FORMAT_RGB_565;
                break;
        }

        LOGI("Use default framebuffer pixel format=%d", format);
    }

    /* Find specified pixel format info in table. */
    for (i = 0; i < sizeof (formatTable) / sizeof (formatTable[0]); i++) {
         if (formatTable[i].format == format) {
             /* Set pixel format detail. */
             info.bits_per_pixel = formatTable[i].bits_per_pixel;
             info.red.offset     = formatTable[i].red_offset;
             info.red.length     = formatTable[i].red_length;
             info.green.offset   = formatTable[i].green_offset;
             info.green.length   = formatTable[i].green_length;
             info.blue.offset    = formatTable[i].blue_offset;
             info.blue.length    = formatTable[i].blue_length;
             info.transp.offset  = formatTable[i].transp_offset;
             info.transp.length  = formatTable[i].transp_length;

             break;
         }
    }

    if (i == sizeof (formatTable) / sizeof (formatTable[0])) {
        /* Can not find format info in table. */
        LOGW("Unkown format specified: %d", format);
        close(fd);
        return -EINVAL;
    }

    /*
     * Request NUM_BUFFERS screens (at lest 2 for page flipping)
     */
    info.yres_virtual = info.yres * NUM_BUFFERS;


    uint32_t flags = PAGE_FLIP;
    if (ioctl(fd, FBIOPUT_VSCREENINFO, &info) == -1) {
        info.yres_virtual = info.yres;
        flags &= ~PAGE_FLIP;
        LOGW("FBIOPUT_VSCREENINFO failed, page flipping not supported");
    }

#ifndef FRONT_BUFFER
    if (info.yres_virtual < info.yres * 2) {
        // we need at least 2 for page-flipping
        info.yres_virtual = info.yres;
        flags &= ~PAGE_FLIP;
        LOGW("page flipping not supported (yres_virtual=%d, requested=%d)",
                info.yres_virtual, info.yres*2);
    }
#endif

    if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1)
    {
        close(fd);
        return -errno;
    }

    uint64_t  refreshQuotient =
    (
            uint64_t( info.upper_margin + info.lower_margin + info.yres )
            * ( info.left_margin  + info.right_margin + info.xres )
            * info.pixclock
    );

    /* Beware, info.pixclock might be 0 under emulation, so avoid a
     * division-by-0 here (SIGFPE on ARM) */
    int refreshRate = refreshQuotient > 0 ? (int)(1000000000000000LLU / refreshQuotient) : 0;

    if (refreshRate == 0) {
        // bleagh, bad info from the driver
        refreshRate = 60*1000;  // 60 Hz
    }

    if (int(info.width) <= 0 || int(info.height) <= 0) {
        // the driver doesn't return that information
        // default to 160 dpi
        info.width  = ((info.xres * 25.4f)/160.0f + 0.5f);
        info.height = ((info.yres * 25.4f)/160.0f + 0.5f);
    }

    float xdpi = (info.xres * 25.4f) / info.width;
    float ydpi = (info.yres * 25.4f) / info.height;
    float fps  = refreshRate / 1000.0f;

    LOGI("using (fd=%d)\n"
         "id           = %s\n"
         "xres         = %d px\n"
         "yres         = %d px\n"
         "xres_virtual = %d px\n"
         "yres_virtual = %d px\n"
         "bpp          = %d\n"
         "r            = %2u:%u\n"
         "g            = %2u:%u\n"
         "b            = %2u:%u\n",
         fd,
         finfo.id,
         info.xres,
         info.yres,
         info.xres_virtual,
         info.yres_virtual,
         info.bits_per_pixel,
         info.red.offset, info.red.length,
         info.green.offset, info.green.length,
         info.blue.offset, info.blue.length
    );

    LOGI("width        = %d mm (%f dpi)\n"
         "height       = %d mm (%f dpi)\n"
         "refresh rate = %.2f Hz\n"
         "Framebuffer phys addr = %p\n",
         info.width,  xdpi,
         info.height, ydpi,
         fps,
         (void*) finfo.smem_start
    );


    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        close(fd);
        return -errno;
    }

    if (finfo.smem_len <= 0)
    {
        close(fd);
        return -errno;
    }


    module->flags = flags;
    module->info = info;
    module->finfo = finfo;
    module->xdpi = xdpi;
    module->ydpi = ydpi;
    module->fps = fps;

    /*
     * map the framebuffer
     */

    // int err;
    size_t fbSize = roundUpToPageSize(finfo.line_length * info.yres_virtual);

    /* Check pages for mapping. */
#if (NUM_PAGES_MMAP != 0)
    fbSize = NUM_PAGES_MMAP * PAGE_SIZE;
#endif

    module->framebuffer = new private_handle_t(dup(fd), fbSize, 0);

#ifndef FRONT_BUFFER
    module->numBuffers = info.yres_virtual / info.yres;
#else
    module->numBuffers = 1;
#endif
    module->bufferMask = 0;

    void* vaddr = mmap(0, fbSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (vaddr == MAP_FAILED) {
        LOGE("Error mapping the framebuffer (%s)", strerror(errno));
        close(fd);
        return -errno;
    }
    module->framebuffer->base = intptr_t(vaddr);
    memset(vaddr, 0, fbSize);
    close(fd);
    return 0;
}

static int mapFrameBuffer(struct private_module_t* module)
{
    pthread_mutex_lock(&module->lock);
    int err = mapFrameBufferLocked(module);
    pthread_mutex_unlock(&module->lock);
    return err;
}

/*****************************************************************************/

static int fb_close(struct hw_device_t *dev)
{
    fb_context_t* ctx = (fb_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

int fb_device_open(hw_module_t const* module, const char* name,
        hw_device_t** device)
{
    int status = -EINVAL;
    if (!strcmp(name, GRALLOC_HARDWARE_FB0)) {
        /* initialize our state here */
        fb_context_t *dev = (fb_context_t*)malloc(sizeof(*dev));
        if (dev == NULL)
            return status;
        else
            memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = 0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = fb_close;
        dev->device.setSwapInterval = fb_setSwapInterval;
        dev->device.post            = fb_post;
        dev->device.setUpdateRect = 0;
        (void) fb_setUpdateRect;

        private_module_t* m = (private_module_t*)module;
        status = mapFrameBuffer(m);
        if (status >= 0) {
            int stride = m->finfo.line_length / (m->info.bits_per_pixel >> 3);

            const_cast<uint32_t&>(dev->device.flags) = 0;
            const_cast<uint32_t&>(dev->device.width) = m->info.xres;
            const_cast<uint32_t&>(dev->device.height) = m->info.yres;
            const_cast<int&>(dev->device.stride) = stride;
            const_cast<int&>(dev->device.format) = format;
            const_cast<float&>(dev->device.xdpi) = m->xdpi;
            const_cast<float&>(dev->device.ydpi) = m->ydpi;
            const_cast<float&>(dev->device.fps) = m->fps;
            const_cast<int&>(dev->device.minSwapInterval) = 1;
            const_cast<int&>(dev->device.maxSwapInterval) = 1;
#if ANDROID_SDK_VERSION >= 17
            const_cast<int&>(dev->device.numFramebuffers) = NUM_BUFFERS;
#endif
            *device = &dev->device.common;
        }
    }
    return status;
}

/******************************************************************************/


static int gralloc_alloc_framebuffer_locked(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    /*
     * This function code is taken from 'gralloc.cpp' of Google's reference
     * gralloc module.
     * It is to allocate (or exactly, wrap) framebuffer buffer.
     */
    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);

    // allocate the framebuffer
    if (m->framebuffer == NULL) {
        // initialize the framebuffer, the framebuffer is mapped once
        // and forever.
        int err = mapFrameBufferLocked(m);
        if (err < 0) {
            return err;
        }
    }

    const uint32_t bufferMask = m->bufferMask;
    const uint32_t numBuffers = m->numBuffers;
    const size_t bufferSize = m->finfo.line_length * (m->info.yres_virtual / m->numBuffers);

    if (bufferMask >= ((1LU<<numBuffers)-1)) {
        // We ran out of buffers.
        return -ENOMEM;
    }

    // create a "fake" handles for it
    intptr_t vaddr = intptr_t(m->framebuffer->base);
    private_handle_t* hnd = new private_handle_t(dup(m->framebuffer->fd), size,
            private_handle_t::PRIV_FLAGS_FRAMEBUFFER);

    // find a free slot
    for (uint32_t i=0 ; i<numBuffers ; i++) {
        if ((bufferMask & (1LU<<i)) == 0) {
#ifndef FRONT_BUFFER
            m->bufferMask |= (1LU<<i);
#endif
            break;
        }
        vaddr += bufferSize;
    }

    hnd->base = vaddr;
    hnd->offset = vaddr - intptr_t(m->framebuffer->base);
    *pHandle = hnd;

    return 0;
}

int gralloc_alloc_framebuffer(alloc_device_t* dev,
        int w, int h, int fmt, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
    /*
     * The following code is from Google's reference gralloc.
     */
    if (!pHandle || !pStride)
        return -EINVAL;

    size_t size, stride;

    int align = 4;
    int bpp = 0;
    switch (fmt) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bpp = 3;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
#if ANDROID_SDK_VERSION < 19
        case HAL_PIXEL_FORMAT_RGBA_5551:
        case HAL_PIXEL_FORMAT_RGBA_4444:
#endif
            bpp = 2;
            break;
        default:
            return -EINVAL;
    }

    size_t bpr = (w*bpp + (align-1)) & ~(align-1);
    size = bpr * h;
    stride = bpr / bpp;

    private_module_t* m = reinterpret_cast<private_module_t*>(
            dev->common.module);
    pthread_mutex_lock(&m->lock);
    int err = gralloc_alloc_framebuffer_locked(dev, size, usage, pHandle);
    pthread_mutex_unlock(&m->lock);

    if (err < 0) {
        return err;
    }

    *pStride = stride;

    /* XXX: Exact stride should be from framebuffer info. */
    *pStride = m->info.xres_virtual;

    /*
     * XXX: Wrap it into Vivante driver so that Vivante HAL can
     * access the buffer.
     */
    {
        private_handle_t* hnd = (private_handle_t*) *pHandle;
        long phys = m->finfo.smem_start + hnd->offset;
        void* vaddr = (void*)hnd->base;

        err = gc_gralloc_wrap(hnd, w, h, format, *pStride, phys, vaddr);

        if (err < 0) {
            LOGE("%s: failed to wrap", __FUNCTION__);
        }
    }

    return 0;
}

int gralloc_free_framebuffer(alloc_device_t* dev, buffer_handle_t handle)
{
    /*
     * This function code is from Google's reference gralloc.
     * It is to clear bit mask of freed framebuffer buffer.
     */
    if (private_handle_t::validate(handle) < 0) {
        return -EINVAL;
    }

    /* Free framebuffer. */
    private_module_t* m =
        reinterpret_cast<private_module_t*>(dev->common.module);
    private_handle_t const* hnd =
        reinterpret_cast<private_handle_t const*>(handle);

    const size_t bufferSize = m->finfo.line_length * (m->info.yres_virtual / m->numBuffers);
    int index = (hnd->base - m->framebuffer->base) / bufferSize;

    /* Save unused buffer index. */
    m->bufferMask &= ~(1<<index);

    /* XXX: Un-wrap the buffer. */
    gc_gralloc_unwrap(const_cast<private_handle_t*>(hnd));
    close(hnd->fd);
    delete hnd;

    return 0;
}

