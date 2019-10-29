/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hwc_display.h"
#include <gc_gralloc_priv.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>


/*
 * blank(..., blank)
 * Blanks or unblanks a display's screen.
 *
 * Turns the screen off when blank is nonzero, on when blank is zero.
 * Multiple sequential calls with the same blank value must be supported.
 * The screen state transition must be be complete when the function
 * returns.
 *
 * returns 0 on success, negative on error.
 */
int
hwc_blank(
    struct hwc_composer_device_1* dev,
    int disp,
    int blank
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device. */
    if (context == NULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (disp >= HWC_NUM_DISPLAY_TYPES)
    {
        /* Invalid parameters. */
        LOGE("%s(%d): Invalid parameters!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    /*
     * TODO (Soc-vendor): Turn on or turn off screen!.
     * Currently not supported.
     */

    hwcDisplay * dpy    = context->displays[disp];

    if (dpy != NULL)
    {
        dpy->device.actived = !blank;
        return 0;
    }

    return -EINVAL;
}

/*
 * For HWC 1.4 and above, setPowerMode() will be used in place of
 * blank().
 *
 * setPowerMode(..., mode)
 * Sets the display screen's power state.
 *
 * Refer to the documentation of the HWC_POWER_MODE_* constants
 * for information about each power mode.
 *
 * The functionality is similar to the blank() command in previous
 * versions of HWC, but with support for more power states.
 *
 * The display driver is expected to retain and restore the low power
 * state of the display while entering and exiting from suspend.
 *
 * Multiple sequential calls with the same mode value must be supported.
 *
 * The screen state transition must be be complete when the function
 * returns.
 *
 * returns 0 on success, negative on error.
 */
#if ANDROID_SDK_VERSION >= 21
int
hwc_setPowerMode(
    struct hwc_composer_device_1* dev,
    int disp,
    int mode
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device. */
    if (context == NULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (disp >= HWC_NUM_DISPLAY_TYPES)
    {
        /* Invalid parameters. */
        LOGE("%s(%d): Invalid parameters!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    /*
     * TODO (Soc-vendor): Power control!
     * Currently not supported.
     */

    hwcDisplay * dpy    = context->displays[disp];

    if (dpy != NULL)
    {
        dpy->device.actived   = (mode != HWC_POWER_MODE_OFF);
        dpy->device.powerMode = mode;
        return 0;
    }

    return -EINVAL;
}
#endif


/*
 * (*getDisplayConfigs)() returns handles for the configurations available
 * on the connected display. These handles must remain valid as long as the
 * display is connected.
 *
 * Configuration handles are written to configs. The number of entries
 * allocated by the caller is passed in *numConfigs; getDisplayConfigs must
 * not try to write more than this number of config handles. On return, the
 * total number of configurations available for the display is returned in
 * *numConfigs. If *numConfigs is zero on entry, then configs may be NULL.
 *
 * HWC_DEVICE_API_VERSION_1_1 does not provide a way to choose a config.
 * For displays that support multiple configurations, the h/w composer
 * implementation should choose one and report it as the first config in
 * the list. Reporting the not-chosen configs is not required.
 *
 * Returns 0 on success or -errno on error. If disp is a hotpluggable
 * display type and no display is connected, an error should be returned.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
 * It should be NULL for previous versions.
 */
int
hwc_getDisplayConfigs(
    struct hwc_composer_device_1* dev,
    int disp,
    uint32_t* configs,
    size_t* numConfigs
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device. */
    if (context == NULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (disp >= HWC_NUM_DISPLAY_TYPES ||
        configs == NULL || numConfigs == NULL || *numConfigs == 0)
    {
        /* Invalid parameters. */
        LOGE("%s(%d): Invalid parameters!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    hwcDisplay * dpy = context->displays[disp];

    if (dpy != NULL && dpy->device.connected)
    {
        /*
         * TODO (Soc-vendor): Support multiple configs.
         * Current only one config is supported.
         */
        configs[0]  = 0;
        *numConfigs = 1;

        return 0;
    }

    return -EINVAL;
}


/*
 * (*getDisplayAttributes)() returns attributes for a specific config of a
 * connected display. The config parameter is one of the config handles
 * returned by getDisplayConfigs.
 *
 * The list of attributes to return is provided in the attributes
 * parameter, terminated by HWC_DISPLAY_NO_ATTRIBUTE. The value for each
 * requested attribute is written in order to the values array. The
 * HWC_DISPLAY_NO_ATTRIBUTE attribute does not have a value, so the values
 * array will have one less value than the attributes array.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
 * It should be NULL for previous versions.
 *
 * If disp is a hotpluggable display type and no display is connected,
 * or if config is not a valid configuration for the display, a negative
 * value should be returned.
 */
int
hwc_getDisplayAttributes(
    struct hwc_composer_device_1* dev,
    int disp,
    uint32_t config,
    const uint32_t* attributes,
    int32_t* values
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device. */
    if (context == NULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (disp >= HWC_NUM_DISPLAY_TYPES || attributes == NULL || values == 0)
    {
        /* Invalid parameters. */
        LOGE("%s(%d): Invalid parameters!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    hwcDisplay * dpy = context->displays[disp];

    if (dpy == NULL)
    {
        LOGE("%s(%d): display %d not supported!", __FUNCTION__, __LINE__, disp);
        return -EINVAL;
    }

    /*
     * TODO (Soc-vendor): Support mutiple configs.
     * Currently only one config is supported.
     */
    if (config != 0)
    {
        LOGE("%s(%d): config %d not supported!",
             __FUNCTION__, __LINE__, config);
        return 0;
    }

    for (int i = 0; attributes[i] != HWC_DISPLAY_NO_ATTRIBUTE; i++)
    {
        switch (attributes[i])
        {
        case HWC_DISPLAY_VSYNC_PERIOD:
            values[i] = int32_t(1e9 / dpy->device.fps);
            break;

        case HWC_DISPLAY_WIDTH:
            values[i] = dpy->device.xres;
            break;

        case HWC_DISPLAY_HEIGHT:
            values[i] = dpy->device.yres;
            break;

        case HWC_DISPLAY_DPI_X:
            values[i] = dpy->device.xdpi;
            break;

        case HWC_DISPLAY_DPI_Y:
            values[i] = dpy->device.ydpi;
            break;

        default:
            return -EINVAL;
        }
    }

    return 0;
}


/*
 * eventControl(..., event, enabled)
 * Enables or disables h/w composer events for a display.
 *
 * eventControl can be called from any thread and takes effect
 * immediately.
 *
 *  Supported events are:
 *      HWC_EVENT_VSYNC
 *
 * returns -EINVAL if the "event" parameter is not one of the value above
 * or if the "enabled" parameter is not 0 or 1.
 */
int
hwc_eventControl(
    hwc_composer_device_1_t * dev,
    int disp,
    int event,
    int enabled
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    if (disp != HWC_DISPLAY_PRIMARY)
    {
        LOGE("%s(%d): disp %d not supported!",
             __FUNCTION__, __LINE__, disp);
        return -EINVAL;
    }

    hwcDisplay * dpy = context->displays[disp];

    switch (event)
    {
    case HWC_EVENT_VSYNC:
        return hwcVsyncControl(context, dpy, enabled);

    default:
        return -EINVAL;
    }
}


/*
 * Used to retrieve information about the h/w composer
 *
 * Returns 0 on success or -errno on error.
 */
int
hwc_query(
    hwc_composer_device_1_t * dev,
    int what,
    int* value
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    switch (what)
    {
    case HWC_BACKGROUND_LAYER_SUPPORTED:
        *value = 0;
        break;

    case HWC_VSYNC_PERIOD:
        {
            /* Get vsync period of primary display. */
            hwcDisplay * dpy = context->displays[HWC_DISPLAY_PRIMARY];

            if (dpy != NULL)
            {
                *value = int32_t(1e9 / dpy->device.fps);
            }
        }
        break;

    case HWC_DISPLAY_TYPES_SUPPORTED:
        *value = (context->displays[HWC_DISPLAY_EXTERNAL] != NULL)
               ? HWC_DISPLAY_PRIMARY_BIT | HWC_DISPLAY_EXTERNAL_BIT
               : HWC_DISPLAY_PRIMARY_BIT;
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

/*
 * (*getActiveConfig)() returns the index of the configuration that is
 * currently active on the connected display. The index is relative to
 * the list of configuration handles returned by getDisplayConfigs. If there
 * is no active configuration, -1 shall be returned.
 *
 * Returns the configuration index on success or -1 on error.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_4 and later.
 * It shall be NULL for previous versions.
 */
#if ANDROID_SDK_VERSION >= 21
int
hwc_getActiveConfig(
    struct hwc_composer_device_1* dev,
    int disp
    )
{
    /* TODO (Soc-vendor): display config management. */
    return 0;
}
#endif

/*
 * (*setActiveConfig)() instructs the hardware composer to switch to the
 * display configuration at the given index in the list of configuration
 * handles returned by getDisplayConfigs.
 *
 * If this function returns without error, any subsequent calls to
 * getActiveConfig shall return the index set by this function until one
 * of the following occurs:
 *   1) Another successful call of this function
 *   2) The display is disconnected
 *
 * Returns 0 on success or a negative error code on error. If disp is a
 * hotpluggable display type and no display is connected, or if index is
 * outside of the range of hardware configurations returned by
 * getDisplayConfigs, an error shall be returned.
 *
 * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_4 and later.
 * It shall be NULL for previous versions.
 */
#if ANDROID_SDK_VERSION >= 21
int
hwc_setActiveConfig(
    struct hwc_composer_device_1* dev,
    int disp,
    int index
    )
{
    /* TODO (Soc-vendor): display config management. */
    return 0;
}
#endif

/*
 * Asynchronously update the location of the cursor layer.
 *
 * Within the standard prepare()/set() composition loop, the client
 * (surfaceflinger) can request that a given layer uses dedicated cursor
 * composition hardware by specifiying the HWC_IS_CURSOR_LAYER flag. Only
 * one layer per display can have this flag set. If the layer is suitable
 * for the platform's cursor hardware, hwcomposer will return from prepare()
 * a composition type of HWC_CURSOR_OVERLAY for that layer. This indicates
 * not only that the client is not responsible for compositing that layer,
 * but also that the client can continue to update the position of that layer
 * after a call to set(). This can reduce the visible latency of mouse
 * movement to visible, on-screen cursor updates. Calls to
 * setCursorPositionAsync() may be made from a different thread doing the
 * prepare()/set() composition loop, but care must be taken to not interleave
 * calls of setCursorPositionAsync() between calls of set()/prepare().
 *
 * Notes:
 * - Only one layer per display can be specified as a cursor layer with
 *   HWC_IS_CURSOR_LAYER.
 * - hwcomposer will only return one layer per display as HWC_CURSOR_OVERLAY
 * - This returns 0 on success or -errno on error.
 * - This field is optional for HWC_DEVICE_API_VERSION_1_4 and later. It
 *   should be null for previous versions.
 */
#if ANDROID_SDK_VERSION >= 21
int
hwc_setCursorPositionAsync(
    struct hwc_composer_device_1 *dev,
    int disp,
    int x_pos,
    int y_pos
    )
{
    /* TODO (Soc-vendor): hw cursor. */
    return 0;
}
#endif


PFNEGLGETRENDERBUFFERVIVPROC  _eglGetRenderBufferVIV;
PFNEGLPOSTBUFFERVIVPROC _eglPostBufferVIV;

static int
_AddVirtualDisplay(
    hwcContext * Context,
    gctUINT DisplayId
    )
{
    /* Initialize virtual displays. */
    hwcDisplay * dpy = (hwcDisplay *) malloc(sizeof (hwcDisplay));
    if (dpy == NULL)
    {
        return -EINVAL;
    }

    memset(dpy, 0, sizeof (hwcDisplay));

    dpy->disp = DisplayId;

    dpy->device.name      = "virtual display";
    dpy->device.fd        = -1;

    dpy->device.connected = 1;
    dpy->device.actived   = 1;

    dpy->device.fps       = 60;

    Context->displays[DisplayId] = dpy;

    ALOGI("Virtual display @%d initialized.\n", DisplayId);
    return 0;
}

int
hwcInitDisplays(
    hwcContext * Context
    )
{
    _eglGetRenderBufferVIV = (PFNEGLGETRENDERBUFFERVIVPROC)
            eglGetProcAddress("eglGetRenderBufferVIV");

    if (_eglGetRenderBufferVIV == NULL)
    {
        ALOGE("eglGetRenderBufferVIV not found!");

        return -EINVAL;
    }

    _eglPostBufferVIV = (PFNEGLPOSTBUFFERVIVPROC)
            eglGetProcAddress("eglPostBufferVIV");

    if (_eglPostBufferVIV == NULL)
    {
        ALOGE("eglPostBufferVIV=%p!", _eglPostBufferVIV);

        return -EINVAL;
    }

#if ANDROID_SDK_VERSION >= 19
    Context->numPhysicalDisplays = HWC_NUM_PHYSICAL_DISPLAY_TYPES;
#else
    Context->numPhysicalDisplays = HWC_NUM_DISPLAY_TYPES;
#endif

    /* 1 virtual display by default. */
    Context->numDisplays = Context->numPhysicalDisplays + 1;

    /* Default primary display. */
    do
    {
        const hw_module_t * gralloc;
        framebuffer_device_t * dev;

        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &gralloc))
        {
            return -EINVAL;
        }

        if (framebuffer_open(gralloc, &dev))
        {
            return -EINVAL;
        }

        hwcDisplay * dpy = (hwcDisplay *) malloc(sizeof (hwcDisplay));
        if (dpy == NULL)
        {
            return -EINVAL;
        }

        memset(dpy, 0, sizeof (hwcDisplay));

        dpy->disp = 0;

        dpy->device.name      = "dpy0";
        dpy->device.fd        = -1;

        dpy->device.connected = 1;
        dpy->device.actived   = 1;

        dpy->device.xres      = dev->width;
        dpy->device.yres      = dev->height;
        dpy->device.xdpi      = 1000 * dev->xdpi;
        dpy->device.ydpi      = 1000 * dev->ydpi;
        dpy->device.format    = dev->format;
        dpy->device.fps       = dev->fps;

        dpy->device.framebuffer = dev;

        ALOGI("Default primary display\n"
              "xres         = %d px\n"
              "yres         = %d px\n"
              "xdpi         = %.1f dpi\n"
              "ydpi         = %.1f dpi\n"
              "refresh rate = %.1f Hz\n",
               dpy->device.xres,
               dpy->device.yres,
               dpy->device.xdpi / 1000.0f,
               dpy->device.ydpi / 1000.0f,
               dpy->device.fps);

        Context->displays[0] = dpy;
    }
    while (0);

    /* External display. */
    do
    {
        static const char * externalDpy = "/dev/graphics/fb2";

        int fd = open(externalDpy, O_RDWR);

        if (fd < 0)
        {
            ALOGE("open %s failed", externalDpy);
            break;
        }

        struct fb_var_screeninfo info;

        if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1)
        {
            ALOGE("FBIOGET_VSCREENINFO ioctl failed: %s", strerror(errno));
            close(fd);
            break;
        }

        int refreshRate = 1000000000000LLU /
            (
             uint64_t( info.upper_margin + info.lower_margin + info.yres + info.vsync_len)
             * ( info.left_margin  + info.right_margin + info.xres + info.hsync_len)
             * info.pixclock
            );

        if (refreshRate == 0)
        {
            ALOGW("invalid refresh rate, assuming 60 Hz");
            refreshRate = 60;
        }

        hwcDisplay * dpy = (hwcDisplay *) malloc(sizeof (hwcDisplay));
        memset(dpy, 0, sizeof (hwcDisplay));

        dpy->disp = 1;

        dpy->device.name      = externalDpy;
        dpy->device.fd        = fd;

        dpy->device.connected = 1;
        dpy->device.actived   = 1;

        if (int(info.width) <= 0 || int(info.height) <= 0) {
            // the driver doesn't return that information
            // default to 160 dpi
            info.width  = ((info.xres * 25.4f)/160.0f + 0.5f);
            info.height = ((info.yres * 25.4f)/160.0f + 0.5f);
        }

        dpy->device.xres      = info.xres;
        dpy->device.yres      = info.yres;
        dpy->device.xdpi      = 1000 * (info.xres * 25.4f) / info.width;
        dpy->device.ydpi      = 1000 * (info.yres * 25.4f) / info.height;
        dpy->device.fps       = refreshRate;

        ALOGI("External display\n"
              "xres         = %d px\n"
              "yres         = %d px\n"
              "xdpi         = %.1f dpi\n"
              "ydpi         = %.1f dpi\n"
              "refresh rate = %.1f Hz\n",
               dpy->device.xres,
               dpy->device.yres,
               dpy->device.xdpi / 1000.0f,
               dpy->device.ydpi / 1000.0f,
               dpy->device.fps);

        Context->displays[1] = dpy;
    }
    while (0);

    for (size_t i = Context->numPhysicalDisplays; i < Context->numDisplays; i++)
    {
        _AddVirtualDisplay(Context, i);
    }


    /* Setup vsync. */
    hwcInitVsync(Context, Context->displays[0]);

    return 0;
}

int
hwcFinishDisplays(
    hwcContext * Context
    )
{
    /* Finish vsync. */
    hwcFinishVsync(Context, Context->displays[0]);

    for (size_t i = 0; i < Context->numDisplays; i++)
    {
        hwcDisplay * dpy = Context->displays[i];

        if (dpy == NULL) continue;

        if (dpy->device.framebuffer)
        {
            framebuffer_close(dpy->device.framebuffer);
        }

        /* Finish overlay. */
        hwcFinishOverlay(Context, dpy);

        /* Finish cursor. */
        hwcFinishCursor(Context, dpy);

        /* Free allocate memory. */
        hwcAreaPool * pool = dpy->areaPool;

        while (pool != NULL)
        {
            hwcAreaPool * next = pool->next;

            free(pool->areas);
            free(pool);

            pool = next;
        }

        free(dpy);
        Context->displays[i] = NULL;
    }

    return 0;
}

int
hwcDetectVirtualDisplays(
    hwcContext * Context,
    size_t NumDisplays
    )
{
    if (!(Context->numDisplays < NumDisplays))
    {
        return -EINVAL;
    }

    for (size_t i = Context->numDisplays; i < NumDisplays; i++)
    {
        /* Add specific amount of virtual displays. */
        _AddVirtualDisplay(Context, i);
    }

    /* Update number of total displays. */
    Context->numDisplays = NumDisplays;

    return 0;
}

int
hwcDisplayFrame(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    )
{
    hwc_layer_1_t * fbTarget;

    if (!Display || !HwDisplay)
    {
        return 0;
    }

    /* Get framebuffer-target shortcut. */
    fbTarget = &HwDisplay->hwLayers[HwDisplay->numHwLayers - 1];

    /*
     * Wait framebuffer-target until it can be read.
     * TODO (Soc-vendor): some DC may receive the fd and wait it automatically.
     */
    if (fbTarget->acquireFenceFd > 0)
    {
        sync_wait(fbTarget->acquireFenceFd, -1);
        close(fbTarget->acquireFenceFd);
        fbTarget->acquireFenceFd = -1;
    }

    /*
     * Wait 2D blitter until it can be read.
     * TODO (Soc-vendor): some DC may receive the fd and wait it automatically.
     */
    if (Display->fenceFd2D > 0)
    {
        sync_wait(Display->fenceFd2D, -1);
        close(Display->fenceFd2D);
        Display->fenceFd2D = -1;
    }

    /* overlay. */
    hwcOverlayFrame(Context, Display, HwDisplay);

    /* cursor. */
    hwcCursorFrame(Context, Display, HwDisplay);

#if 0
    /* Already done in fsl hwcomposer wrapper. */
    /*
     * Replace on-screen for physical displays for fbdev device.
     */
    if (Display->disp < int(Context->numPhysicalDisplays))
    {
        /* 2D blitter also accesses framebuffer-target directly like 3D. */
        if ((Display->hasGles || Display->hasG2D) && Display->device.framebuffer)
        {
            framebuffer_device_t * dev = Display->device.framebuffer;

            /* TODO (Soc-vendor): Tell framebuffer wheter the buffer is compressed. */

            /* Display framebuffer target. */
            dev->post(dev, fbTarget->handle);
        }
    }
#endif

    /*
     * TODO (Soc-vendor): If DC (Overlay, Cursor, etc) is used for composition, please
     * correct these fenceFds.
     */
    HwDisplay->retireFenceFd = -1;
    fbTarget->releaseFenceFd = -1;

    return 0;
}

