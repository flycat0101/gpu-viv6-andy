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


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include "hwc2_common.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <log/log.h>
#include <sync/sync.h>

typedef struct fbdev_device fbdev_device_t;

struct fbdev_device
{
    __hwc2_device_t base;
};

typedef struct sw_vsync sw_vsync_t;

struct sw_vsync
{
    pthread_t pid;
    long refreshPeriod;

    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    volatile int enabled;
    volatile int done;
};

typedef struct fbdev_display fbdev_display_t;

struct fbdev_display
{
    __hwc2_display_t base;
    framebuffer_device_t * framebuffer;
    struct sw_vsync vsync;
};


static void * sw_vsync_thread(void *param)
{
    fbdev_display_t *display = (fbdev_display_t *)param;
    sw_vsync_t *vsync = &display->vsync;
    __hwc2_device_t *device = display->base.device;

    /* Set priority. */
    /*
    int prio = ANDROID_PRIORITY_URGENT_DISPLAY
             + ANDROID_PRIORITY_MORE_FAVORABLE;

    setpriority(PRIO_PROCESS, 0, prio);
    set_sched_policy(gettid(), SP_FOREGROUND);
     */

    vsync->refreshPeriod = display->base.activeConfig->vsyncPeriod;

    int64_t nextFakeVsync = 0;

    for (;;) {
        int err;

        /* Lock. */
        pthread_mutex_lock(&vsync->mutex);

        while (!vsync->enabled) {
            /* Wait for condition. */
            pthread_cond_wait(&vsync->cond, &vsync->mutex);
        }

        /* Release lock. */
        pthread_mutex_unlock(&vsync->mutex);

        if (vsync->done) {
            /* Check thread exiting flag. */
            break;
        }

        struct timespec spec;
        err = clock_gettime(CLOCK_MONOTONIC, &spec);
        (void) err;

        int64_t now = ((int64_t) spec.tv_sec) * 1000000000 + spec.tv_nsec;
        int64_t nextVsync = nextFakeVsync;
        int64_t sleep      = nextVsync - now;

        if (sleep < 0) {
            /* We missed. find where the next vsync should be */
            sleep = (vsync->refreshPeriod - ((now - nextVsync) % vsync->refreshPeriod));
            nextVsync = now + sleep;
        }

        nextFakeVsync = nextVsync + vsync->refreshPeriod;

        spec.tv_sec  = (long) (nextVsync / 1000000000);
        spec.tv_nsec = (long) (nextVsync % 1000000000);

        do {
            err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
        }
        while (err<0 && errno == EINTR);

        /* Vsync comes. */
        if (err == 0) {
            device->vsync(device->vsyncData,
                (hwc2_display_t)(uintptr_t)display, nextVsync);
        }
    }

    return NULL;
}

static int setup_sw_vsync(__hwc2_device_t *device, fbdev_display_t *display)
{
    sw_vsync_t *vsync = &display->vsync;
    int err;

    pthread_mutex_init(&vsync->mutex, NULL);
    pthread_cond_init(&vsync->cond, NULL);

    vsync->enabled = 0;
    vsync->done = 0;

    err = pthread_create(&vsync->pid, NULL, &sw_vsync_thread, display);

    if (err) {
        return HWC2_ERROR_BAD_PARAMETER;
    }

    return HWC2_ERROR_NONE;
}

static void close_sw_vsync(__hwc2_device_t *device, fbdev_display_t *display)
{
    sw_vsync_t *vsync = &display->vsync;

    if (!vsync) {
        return;
    }

    pthread_mutex_lock(&vsync->mutex);

    vsync->done = 1;
    vsync->enabled = 1;

    pthread_cond_broadcast(&vsync->cond);
    pthread_mutex_unlock(&vsync->mutex);

    pthread_join(vsync->pid, NULL);
}

static int32_t /*hwc2_error_t*/ set_sw_vsync_enabled(
        hwc2_device_t *device, hwc2_display_t display,
        int32_t /*hwc2_vsync_t*/ enabled)
{
    fbdev_display_t *dpy = (fbdev_display_t *)(uintptr_t)display;
    sw_vsync_t *vsync = &dpy->vsync;

    if (unlikely(enabled != HWC2_VSYNC_ENABLE &&
        enabled != HWC2_VSYNC_DISABLE)) {
        return HWC2_ERROR_BAD_PARAMETER;
    }

    pthread_mutex_lock(&vsync->mutex);

    vsync->enabled = dpy->base.vsyncEnabled =
        (enabled == HWC2_VSYNC_ENABLE);

    /* Change refresh period? */
    vsync->refreshPeriod = dpy->base.activeConfig->vsyncPeriod;

    pthread_cond_broadcast(&vsync->cond);
    pthread_mutex_unlock(&vsync->mutex);

    return HWC2_ERROR_NONE;
}

/******************************************************************************/

int fbdev_open_primary_display(fbdev_device_t *dev)
{
    const hw_module_t * gralloc = NULL;
    framebuffer_device_t *fb = NULL;
    fbdev_display_t *dpy;
    __hwc2_config_t *config;
    int err;

    /* Open framebuffer device by gralloc module. */
    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &gralloc))
    {
        ALOGE("%s: framebuffer_open failed", __FUNCTION__);
        return -1;
    }

    err = framebuffer_open(gralloc, &fb);
    if (err ) {
        ALOGE("%s: framebuffer_open failed, err=%d", __FUNCTION__, err);
        return err;
    }

    /* Allocate display. */
    dpy = (fbdev_display_t *)__hwc2_alloc_display(
            &dev->base, fb->width, fb->height, fb->format,
            HWC2_DISPLAY_TYPE_PHYSICAL, sizeof(fbdev_display_t));

    if (!unlikely(!dpy)) {
        framebuffer_close(fb);
        return -1;
    }

    /* Allocate a config. */
    config = __hwc2_alloc_config(&dev->base, &dpy->base, sizeof(__hwc2_config_t));

    if (unlikely(!config)) {
        __hwc2_free_display(&dev->base, &dpy->base);
        framebuffer_close(fb);
        return -1;
    }

    config->width       = fb->width;
    config->height      = fb->height;
    /* fb->fps is in float precision. */
    config->fps         = (int32_t)fb->fps;
    config->vsyncPeriod = (int32_t)(1000000000.0f / fb->fps);
    /* fb->xdpi, fb->ydpi are in float precision. */
    config->xdpi        = (int32_t)(fb->xdpi * 1000);
    config->ydpi        = (int32_t)(fb->ydpi * 1000);

    /* It's the current active config. */
    dpy->base.activeConfig = config;

    dpy->framebuffer = fb;

    /* Setup vsync. */
    setup_sw_vsync(&dev->base, dpy);

    /* Hotplug the display. */
    dev->base.hotplug(dev->base.hotplugData, (hwc2_display_t)(uintptr_t)dpy, 1);

    return err;
}

static int32_t /*hwc2_error_t*/ fbdev_present_display(
        hwc2_device_t* device, hwc2_display_t display, int32_t* outRetireFence)
{
    fbdev_display_t *dpy = (fbdev_display_t *)display;
    int err = 0;
    int32_t fence;

    /* Wait for acquireFence. */
    fence = dpy->base.clientAcquireFence;

    if (fence != -1) {
        sync_wait(fence, -1);
        close(fence);
        dpy->base.clientAcquireFence = -1;
    }

    /* TODO: Blitter composition. */

    /* Post client target. */
    if (dpy->base.type == HWC2_DISPLAY_TYPE_PHYSICAL) {
        err = dpy->framebuffer->post(dpy->framebuffer, dpy->base.clientTarget);
    } else {
        /* Virtual display. */
    }

    *outRetireFence = -1;
    return err;
}

/******************************************************************************/

static int32_t fbdev_register_callback(
        __hwc2_device_t *device,
        int32_t /*hwc2_callback_descriptor_t*/ descriptor,
        hwc2_callback_data_t callbackData, hwc2_function_pointer_t pointer)
{
    fbdev_device_t *dev = (fbdev_device_t *)device;
    int err;

    err = __hwc2_register_callback(
            &dev->base, descriptor, callbackData, pointer);

    if (err) {
        return err;
    }

    if (descriptor != HWC2_CALLBACK_HOTPLUG) {
        /* Nothing to do except hotplug. */
        return 0;
    }

    err = fbdev_open_primary_display(dev);

    return err;
}

/******************************************************************************/
/* Define the module and device. */

static int fbdev_device_open(const struct hw_module_t * module,
                const char * name, struct hw_device_t ** device);

static int fbdev_device_close(struct hw_device_t *device);

static hwc2_function_pointer_t fbdev_get_function(
        struct hwc2_device* device,
        int32_t /*hwc2_function_descriptor_t*/ descriptor);

static struct hw_module_methods_t fbdev_module_methods =
{
    .open = fbdev_device_open
};

/* hwc2 module. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
__attribute__((weak, visibility("default"))) hw_module_t HAL_MODULE_INFO_SYM =
{
    .tag                = HARDWARE_MODULE_TAG,
    .module_api_version = HWC_MODULE_API_VERSION_0_1,
    .hal_api_version    = HARDWARE_HAL_API_VERSION,
    .id                 = HWC_HARDWARE_MODULE_ID,
    .name               = "Vivante HWComposer HAL Reference",
    .author             = "Vivante Corporation",
    .methods            = &fbdev_module_methods,
};
#pragma clang diagnostic pop

/* hwc2 device. */
static fbdev_device_t fbdev_device =
{
    .base = {
        .device = {
            .common = {
                .tag     = HARDWARE_DEVICE_TAG,
                .version = HWC_DEVICE_API_VERSION_2_0,
                .module  = &HAL_MODULE_INFO_SYM,
                .close   = fbdev_device_close,
            },

            .getCapabilities = __hwc2_get_capabilities,
            .getFunction     = fbdev_get_function,
        },
    },
};

hwc2_function_pointer_t fbdev_get_function(
        struct hwc2_device* device,
        int32_t /*hwc2_function_descriptor_t*/ descriptor)
{
    switch (descriptor) {
        case HWC2_FUNCTION_PRESENT_DISPLAY:
            return (hwc2_function_pointer_t)fbdev_present_display;

        case HWC2_FUNCTION_SET_VSYNC_ENABLED:
            return (hwc2_function_pointer_t)set_sw_vsync_enabled;

        case HWC2_FUNCTION_REGISTER_CALLBACK:
            return (hwc2_function_pointer_t)fbdev_register_callback;

        default:
            return __hwc2_get_function(device, descriptor);
    }
}

int fbdev_device_open(const struct hw_module_t * module,
                const char * name, struct hw_device_t ** device)
{
    int err;

    if (strcmp(name, HWC_HARDWARE_COMPOSER)) {
        /* Not a hwc2 device. */
        return -ENOENT;
    }

    err = __hwc2_init_device((__hwc2_device_t *)&fbdev_device);

    if (err) {
        return err;
    }

    /* Initialize features. */
    fbdev_device.base.numCapabilities        = 0;
    fbdev_device.base.maxVirtualDisplayCount = 1;

    *device = &fbdev_device.base.device.common;
    return err;
}

int fbdev_device_close(struct hw_device_t *device)
{
    __hwc2_close_device((__hwc2_device_t *)device);
    return 0;
}

