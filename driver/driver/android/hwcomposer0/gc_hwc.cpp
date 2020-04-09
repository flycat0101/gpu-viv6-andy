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


#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gc_hal.h>
#include <gc_hal_driver.h>

#include "gc_hwc.h"
#include "gc_hwc_debug.h"

#include <hardware/hardware.h>
#include <cutils/properties.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/time.h>

static int
hwc_prepare(
    hwc_composer_device_t * dev,
    hwc_layer_list_t * list
    );

static int
hwc_set(
    hwc_composer_device_t * dev,
    hwc_display_t dpy,
    hwc_surface_t surf,
    hwc_layer_list_t * list
    );

#if ANDROID_SDK_VERSION >= 14
/* ICS and later. */
static void
hwc_registerProcs(
    hwc_composer_device_t * dev,
    hwc_procs_t const * procs
    );
#endif

#if ANDROID_SDK_VERSION >= 16
/* Jellybean and later. */
static int
hwc_query(
    hwc_composer_device_t * dev,
    int what,
    int* value
    );

static int
hwc_eventControl(
    hwc_composer_device_t * dev,
    int event,
    int enabled
    );
#endif

static int
hwc_device_close(
    struct hw_device_t * dev
    );

static int
hwc_device_open(
    const struct hw_module_t * module,
    const char * name,
    struct hw_device_t ** device
    );


/******************************************************************************/

#ifndef HWC_VIV_HARDWARE_MODULE_ID
#define HWC_VIV_HARDWARE_MODULE_ID "hwcomposer_viv"
#endif
static struct hw_module_methods_t hwc_module_methods =
{
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM =
{
    common:
    {
        tag:           HARDWARE_MODULE_TAG,
        version_major: 2,
        version_minor: 4,
        id:            HWC_VIV_HARDWARE_MODULE_ID,
        name:          "Hardware Composer Module",
        author:        "Vivante Corporation",
        methods:       &hwc_module_methods,
        dso:           NULL,
        reserved:      {0, }
    }
};

#if ANDROID_SDK_VERSION >= 16
/* Jellybean and later. */
static hwc_methods_t hwc_methods =
{
    eventControl: hwc_eventControl
};
#endif

typedef EGLClientBuffer (EGLAPIENTRYP PFNEGLGETRENDERBUFFERV0VIVPROC) (EGLDisplay dpy, EGLSurface draw);

static PFNEGLGETRENDERBUFFERV0VIVPROC _eglGetRenderBufferv0VIV;


/******************************************************************************/

int
hwc_prepare(
    hwc_composer_device_t * dev,
    hwc_layer_list_t * list
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    /* Check layer count. */
    if ((list == NULL)
    ||  (list->numHwLayers == 0)
    )
    {
        return 0;
    }

    if (gcmIS_ERROR(hwcPrepare(context, list)))
    {
        LOGE("%s(%d): Failed in prepare", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    return 0;
}


int
hwc_set(
    hwc_composer_device_t * dev,
    hwc_display_t dpy,
    hwc_surface_t surf,
    hwc_layer_list_t * list
    )
{
    gceSTATUS status;
    struct timeval last;
    hwcContext * context = (hwcContext *) dev;
    android_native_buffer_t * backBuffer = NULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    /* Check layer count. */
    if ((list == NULL)
    ||  (list->numHwLayers == 0)
    )
    {
        /* No layer to set. */
        return 0;
    }

    if (context->hasG2D)
    {
        /* Get back buffer for hwc composition. */
        backBuffer = (android_native_buffer_t *)
           _eglGetRenderBufferv0VIV((EGLDisplay) dpy, (EGLSurface) surf);

        if (backBuffer == NULL)
        {
            LOGE("%s(%d): Failed to get back buffer", __FUNCTION__, __LINE__);
            return -EINVAL;
        }

        /* Set cureent hardware type to 2D. */
        if (context->separated2D)
        {
            /* Query current hardware type. */
            gcoHAL_GetHardwareType(context->hal, &currentType);

            gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
        }
    }

    {
        char property[PROPERTY_VALUE_MAX] = "0";
        const char * defaultValue = "0";

        property_get("hwc.debug.dump_compose", property, defaultValue);
        context->dumpCompose = property[0] - '0';

        property_get("hwc.debug.dump_bitmap", property, defaultValue);
        context->dumpBitmap = property[0] - '0';

        property_get("hwc.debug.dump_split_area", property, defaultValue);
        context->dumpSplitArea = property[0] - '0';
    }

    if (context->dumpCompose)
    {
        gettimeofday(&last, NULL);
    }

    /* Start hwc set. */
    gcmONERROR(
        hwcSet(context, backBuffer, list));

    if (context->hasG2D)
    {
        /* Commit and stall before swap for 2D composition. */
        gcmVERIFY_OK(
            gcoHAL_Commit(context->hal, gcvTRUE));
    }

    /* Swap buffers. */
    eglSwapBuffers((EGLDisplay) dpy, (EGLSurface) surf);

    if (context->dumpCompose)
    {
        struct timeval curr;
        long   expired;

        gettimeofday(&curr, NULL);

        expired = (curr.tv_sec - last.tv_sec) * 1000
                + (curr.tv_usec - last.tv_usec) / 1000;

        LOGD("Set %d layer(s): %ld ms", list->numHwLayers, expired);
    }

    if (context->dumpBitmap)
    {
        hwcDumpBitmap(list, backBuffer);
    }

    /* Reset hardware type. */
    if (context->separated2D && backBuffer != NULL)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }

    return 0;

OnError:
    LOGE("%s(%d): Failed in set", __FUNCTION__, __LINE__);

    if (backBuffer != NULL)
    {
        /* Must call eglSwapBuffers to queue buffer. */
        eglSwapBuffers((EGLDisplay) dpy, (EGLSurface) surf);

        /* Reset hardware type. */
        if (context->separated2D)
        {
            gcoHAL_SetHardwareType(context->hal, currentType);
        }
    }

    return -EINVAL;
}

#if ANDROID_SDK_VERSION >= 14
/* ICS and later. */
static void
hwc_registerProcs(
    hwc_composer_device_t * dev,
    hwc_procs_t const * procs
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return;
    }

    /* Retrieve procs. */
    context->callback = procs;
}
#endif

#if ANDROID_SDK_VERSION >= 16
/* Jellybean and later. */
int
hwc_query(
    hwc_composer_device_t * dev,
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
        /* Get vsync period. */
        *value = hwcVsyncPeriod(context);
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

int
hwc_eventControl(
    hwc_composer_device_t * dev,
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

    switch (event)
    {
    case HWC_EVENT_VSYNC:
        return hwcVsyncControl(context, enabled);

    default:
        return -EINVAL;
    }
}
#endif


int
hwc_device_close(
    struct hw_device_t *dev
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device. */
    if (context == NULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

#if ANDROID_SDK_VERSION >= 16
    /* Jellybean and later: Stop vsync. */
    hwcVsyncStop(context);
#endif

    /* Free filter buffer. */
    gcmVERIFY_OK(
        gco2D_FreeFilterBuffer(context->engine));

    /* Destroy hal object. */
    gcmVERIFY_OK(
        gcoHAL_Destroy(context->hal));

    /* Destroy os object. */
    gcmVERIFY_OK(
        gcoOS_Destroy(context->os));

    /* Clean context. */
    free(context);

    return 0;
}


int
hwc_device_open(
    const struct hw_module_t * module,
    const char * name,
    struct hw_device_t ** device
    )
{
    gceSTATUS  status    = gcvSTATUS_OK;
    hwcContext * context = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;
    gcsHAL_INTERFACE iface;
    gctBOOL has2DPipe = gcvFALSE;
    gctBOOL blitAllQuad = gcvFALSE;

    if (strcmp(name, HWC_HARDWARE_COMPOSER) != 0)
    {
        LOGE("%s(%d): Invalid device name!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    _eglGetRenderBufferv0VIV = (PFNEGLGETRENDERBUFFERV0VIVPROC)
                                 eglGetProcAddress("eglGetRenderBufferv0VIV");

    if (_eglGetRenderBufferv0VIV == NULL)
    {
        LOGE("eglGetRenderBufferv0VIV function "
             "not found for hwcomposer");

        return HWC_EGL_ERROR;
    }

    /* Allocate memory for context. */
    context = (hwcContext *) malloc(sizeof (hwcContext));
    memset(context, 0, sizeof (hwcContext));

    /* Initialize variables. */
    context->device.common.tag     = HARDWARE_DEVICE_TAG;
#if ANDROID_SDK_VERSION >= 16
    /* Jellybean and later. */
    context->device.common.version = 0; /* HWC_DEVICE_API_VERSION_0_3; */
#else
    context->device.common.version = 0;
#endif
    context->device.common.module  = (hw_module_t *) module;

    /* initialize the procs */
    context->device.common.close   = hwc_device_close;
    context->device.prepare        = hwc_prepare;
    context->device.set            = hwc_set;
    context->device.dump           = NULL;

#if ANDROID_SDK_VERSION >= 14
    /* ICS and later. */
    context->device.registerProcs  = hwc_registerProcs;
#endif

#if ANDROID_SDK_VERSION >= 16
    /* Jellybean and later. */
    context->device.query          = hwc_query;
    context->device.methods        = &hwc_methods;

    /* Setup vsync. */
    hwcVsyncSetup(context);
#endif

    /* Initialize GC stuff. */
    /* Construct os object. */
    gcmONERROR(
        gcoOS_Construct(gcvNULL, &context->os));

    /* Construct hal object. */
    gcmONERROR(
        gcoHAL_Construct(gcvNULL, context->os, &context->hal));

    /* Query chip info */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_CHIP_INFO;

    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    /* Determine hardware features. */
    for (gctINT i = 0; i < iface.u.ChipInfo.count; i++)
    {
        switch (iface.u.ChipInfo.types[i])
        {
        case gcvHARDWARE_3D2D:
            has2DPipe = gcvTRUE;
            break;

        case gcvHARDWARE_3D:
            break;

        case gcvHARDWARE_2D:
            has2DPipe = gcvTRUE;
            context->separated2D = gcvTRUE;
            break;

        default:
            break;
        }
    }

    /* Check 2D pipe existance. */
    if (has2DPipe == gcvFALSE)
    {
        LOGE("%s(%d): 2D PIPE not found", __FUNCTION__, __LINE__);
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Switch to 2D core if has separated 2D core. */
    if (context->separated2D)
    {
        /* Query current hardware type. */
        gcmONERROR(
            gcoHAL_GetHardwareType(context->hal, &currentType));

        gcmONERROR(
            gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D));
    }

    /* Get gco2D object pointer. */
    gcmONERROR(
        gcoHAL_Get2DEngine(context->hal, &context->engine));

    /* Check GPU PE 2.0 feature. */
    status = gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2DPE20);

    if (status != gcvSTATUS_TRUE)
    {
        LOGE("%s(%d): Requires PE20, but not supported", __FUNCTION__, __LINE__);
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Check YUV separarted stride feature. */
    context->yuvSeparateStride =
        gcoHAL_IsFeatureAvailable(context->hal,
                                  gcvFEATURE_2D_YUV_SEPARATE_STRIDE);

    /* Check multi-source blit feature. */
    context->multiSourceBlt =
        gcoHAL_IsFeatureAvailable(context->hal,
                                  gcvFEATURE_2D_MULTI_SOURCE_BLT);

    /* Check multi-source blit Ex feature. */
    if (gcoHAL_IsFeatureAvailable(context->hal,
                                  gcvFEATURE_2D_MULTI_SOURCE_BLT_EX))
    {
        context->msSourceMirror  = gcvTRUE;
        context->msMaxSource     = 8;
        context->msByteAlignment = gcvTRUE;
    }
    else
    {
        context->msMaxSource = 4;
    }

    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_ALL_QUAD))
    {
        blitAllQuad = gcvTRUE;
    }

    context->msOddCoord = !blitAllQuad;

    /* Check One patch filter blt/YUV blit feature. */
    context->opf =
        gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_ONE_PASS_FILTER);

    /* Check tiled input feature. */
    context->tiledInput =
        gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_TILING);

    /* Check compression/multi-source blit with different offsets support. */
    context->compression =
        gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_COMPRESSION);

    if (context->compression || blitAllQuad)
    {
        context->msSourceRotation = gcvTRUE;
    }

    if (context->compression)
    {
        context->msSourceOffset = gcvTRUE;
    }

    /* Set 2D parameters. */
    /* Mirror in dstRect instead of dstSubRect. */
    gcmVERIFY_OK(
        gco2D_SetStateU32(context->engine,
                          gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE, 1));

    gcmVERIFY_OK(
        gco2D_SetStateU32(context->engine,
                          gcv2D_STATE_XRGB_ENABLE, 1));

    /* Switch back to current type. */
    if (context->separated2D)
    {
        gcmONERROR(
            gcoHAL_SetHardwareType(context->hal, currentType));
    }

    /* Get filter stretch config from property. */
    char property[PROPERTY_VALUE_MAX];
    if (property_get("hwc.stretch.filter", property, NULL) > 0)
    {
        context->filterStretch = property[0] - '0';
    }
    else
    {
        /* Default filter stretch config. */
#if FILTER_STRETCH_BY_DEFAULT
        context->filterStretch = 1;
#else
        context->filterStretch = 0;
#endif
    }

    /* Return device handle. */
    *device = &context->device.common;

    /* Print infomation. */
    LOGI("Vivante HWComposer v%d.%d\n"
         "Device:               %p\n"
         "Separated 2D:         %s\n"
         "YUV separate stride : %s\n"
         "Multi-source blit   : %s\n"
         "  Source mirror     : %s\n"
         "  Max source count  : %d\n"
         "  Byte alignment    : %s\n"
         "  Source rotation   : %s\n"
         "  Source offsets:   : %s\n"
         "OPF/YUV blit        : %s\n"
         "Tiled input         : %s\n"
         "Compression         : %s\n"
         "Filter stretch      : %s\n",
         HMI.common.version_major,
         HMI.common.version_minor,
         (void *) context,
         (context->separated2D      ? "YES" : "NO"),
         (context->yuvSeparateStride? "YES" : "NO"),
         (context->multiSourceBlt   ? "YES" : "NO"),
         (context->msSourceMirror   ? "YES" : "NO"),
         (context->msMaxSource),
         (context->msByteAlignment  ? "YES" : "NO"),
         (context->msSourceRotation ? "YES" : "NO"),
         (context->msSourceOffset   ? "YES" : "NO"),
         (context->opf              ? "YES" : "NO"),
         (context->tiledInput       ? "YES" : "NO"),
         (context->compression      ? "YES" : "NO"),
         (context->filterStretch    ? "YES" : "NO"));

    return 0;

OnError:

    /* Error roll back. */
    if (context != gcvNULL)
    {
        if (context->separated2D)
        {
            gcmVERIFY_OK(
                gcoHAL_SetHardwareType(gcvNULL, currentType));
        }

        if (context->hal != gcvNULL)
        {
            gcmVERIFY_OK(
                gcoHAL_Destroy(context->hal));

            gcmVERIFY_OK(
                gcoOS_Destroy(context->os));

            free(context);
        }
    }

    *device = NULL;

    LOGE("%s(%d): Failed to initialize hwcomposer!", __FUNCTION__, __LINE__);

    return -EINVAL;
}

