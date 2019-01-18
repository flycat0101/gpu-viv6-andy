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


#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <gc_hal.h>
#include <gc_hal_driver.h>

#include "gc_hwc.h"
#include "gc_hwc_display.h"
#include "gc_hwc_debug.h"

#include <hardware/hardware.h>
#include <cutils/properties.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/time.h>


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

/*
 * (*registerProcs)() registers callbacks that the h/w composer HAL can
 * later use. It will be called immediately after the composer device is
 * opened with non-NULL procs. It is FORBIDDEN to call any of the callbacks
 * from within registerProcs(). registerProcs() must save the hwc_procs_t
 * pointer which is needed when calling a registered callback.
 */
static void
hwc_registerProcs(
    hwc_composer_device_1_t * dev,
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


/******************************************************************************/

static struct hw_module_methods_t hwc_module_methods =
{
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM =
{
    common:
    {
        tag:           HARDWARE_MODULE_TAG,
        module_api_version: 0,
        hal_api_version:    0,
        id:            HWC_HARDWARE_MODULE_ID,
        name:          "Hardware Composer Module",
        author:        "Vivante Corporation",
        methods:       &hwc_module_methods,
        dso:           NULL,
        reserved:      {0, }
    }
};

/******************************************************************************/

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

    /* Finish displays. */
    hwcFinishDisplays(context);

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
    gctBOOL has3DPipe = gcvFALSE;
    gctBOOL blitAllQuad = gcvFALSE;
    gctUINT superTileMode = 0;

    if (strcmp(name, HWC_HARDWARE_COMPOSER) != 0)
    {
        LOGE("%s(%d): Invalid device name!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    /* Allocate memory for context. */
    context = (hwcContext *) malloc(sizeof (hwcContext));

    if (context == NULL)
    {
        LOGE("%s(%d): Out of memory!", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    memset(context, 0, sizeof (hwcContext));

    /* Initialize variables. */
    context->device.common.tag     = HARDWARE_DEVICE_TAG;
#if ANDROID_SDK_VERSION >= 23
    /* HWC version 1.5 for Marshmallow. */
    context->device.common.version = HWC_DEVICE_API_VERSION_1_5;
#elif ANDROID_SDK_VERSION >= 21
    /* HWC version 1.4 for Lollipop. */
    context->device.common.version = HWC_DEVICE_API_VERSION_1_4;
#elif ANDROID_SDK_VERSION >= 19
    /* HWC version 1.3 for KitKat and later. */
    context->device.common.version = HWC_DEVICE_API_VERSION_1_3;
#else
    context->device.common.version = HWC_DEVICE_API_VERSION_1_2;
#endif
    context->device.common.module  = (hw_module_t *) module;
    context->device.common.close   = hwc_device_close;

    /* initialize the procs */
    context->device.prepare        = hwc_prepare;
    context->device.set            = hwc_set;
    context->device.eventControl   = hwc_eventControl;
    context->device.blank          = hwc_blank;
    context->device.query          = hwc_query;
    context->device.registerProcs  = hwc_registerProcs;
    context->device.dump           = NULL;
    context->device.getDisplayConfigs    = hwc_getDisplayConfigs;
    context->device.getDisplayAttributes = hwc_getDisplayAttributes;

#if ANDROID_SDK_VERSION >= 21
    if (context->device.common.version >= HWC_DEVICE_API_VERSION_1_4)
    {
        context->device.setPowerMode            = hwc_setPowerMode;
        context->device.getActiveConfig         = hwc_getActiveConfig;
        context->device.setActiveConfig         = hwc_setActiveConfig;
        context->device.setCursorPositionAsync  = hwc_setCursorPositionAsync;
    }
#endif

    /* Initialize displays. */
    if (hwcInitDisplays(context) != 0)
    {
        free(context);
        return -EINVAL;
    }

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
            has3DPipe = gcvTRUE;
            break;

        case gcvHARDWARE_3D:
            has3DPipe = gcvTRUE;
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

    if (context->separated2D)
    {
        /* Query current hardware type. */
        gcmONERROR(
            gcoHAL_GetHardwareType(context->hal, &currentType));
    }

    /* Switch to 3D core to check 3D features. */
    if (has3DPipe)
    {
        if (context->separated2D)
        {
            gcmONERROR(
                gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_3D));
        }

        gcmONERROR(gcoHAL_QuerySuperTileMode(&superTileMode));
    }

    /* Switch to 2D core if has separated 2D core. */
    if (context->separated2D)
    {
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
        LOGE("%s(%d): Requires PE20, but not supported ", __FUNCTION__, __LINE__);
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
        context->multiSourceBltEx = gcvTRUE;
        context->msSourceMirror   = gcvTRUE;
        context->msByteAlignment  = gcvTRUE;
        context->msYuvSourceCount = 1;
    }

    /* Check multi-source blit Ex feature. */
    if (gcoHAL_IsFeatureAvailable(context->hal,
                                  gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2))
    {
        context->multiSourceBlt   = gcvTRUE;
        context->multiSourceBltV2 = gcvTRUE;
        context->msYuvSourceCount = 8;
    }

    context->msMaxSource = context->multiSourceBltEx ? 8
                         : context->multiSourceBlt ? 4 : 0;

    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_ALL_QUAD))
    {
        blitAllQuad = gcvTRUE;
    }

    context->msOddCoord = !blitAllQuad;

    /* Check tiling input feature. */
    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_TILING))
    {
        context->tiledInput = gcvTRUE;
    }

    /* Check One patch filter blt/YUV blit feature. */
    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_ONE_PASS_FILTER))
    {
        context->opf = gcvTRUE;
    }

    /* Check 2D output compression. */
    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_COMPRESSION))
    {
        gceCHIPMODEL chipModel;
        context->compression = gcvTRUE;

        gcmVERIFY_OK(gcoHAL_QueryChipIdentity(context->hal,
                                              &chipModel,
                                              gcvNULL,
                                              gcvNULL,
                                              gcvNULL));

        if (chipModel == gcv320)
        {
            /* Drops performance on GC320C for compression. */
            context->tiledCompressQuirks = gcvTRUE;
        }
    }

    /* Check mirror externsion. */
    if (gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_MIRROR_EXTENSION))
    {
        context->mirrorExt = gcvTRUE;
    }

    if (context->compression || blitAllQuad)
    {
        context->msSourceRotation = gcvTRUE;
    }

    /* Check multi-source blit with different offsets support. */
    if (gcoHAL_IsFeatureAvailable(context->hal,
                                  gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT))
    {
        context->msUnifiedDestRect = gcvTRUE;

        /* Multi-source blit v1.5. */
        context->multiSourceBlt    = gcvTRUE;
        context->multiSourceBltV15 = gcvTRUE;
        context->msYuvSourceCount  = 8;
    }

    if (context->msUnifiedDestRect || context->multiSourceBltV2)
    {
        /* Support source offsets when has unified or separated dest rectangle. */
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

    if (context->msUnifiedDestRect && !context->multiSourceBltV2)
    {
        /* Use unified dest rectangle to support source offsets. */
        gcmVERIFY_OK(
            gco2D_SetStateU32(context->engine,
                              gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT, 1));
    }

    /* Supertile version. */
    gcmVERIFY_OK(
        gco2D_SetStateU32(context->engine,
                          gcv2D_STATE_SUPER_TILE_VERSION, (superTileMode + 1)));

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
    LOGI("Vivante HWComposer v2.4\n"
         "Device:               %p\n"
         "Separated 2D:         %s\n"
         "YUV separate stride : %s\n"
         "Multi-source blit   : %s\n",
         (void *) context,
         (context->separated2D      ? "YES" : "NO"),
         (context->yuvSeparateStride? "YES" : "NO"),
         (context->multiSourceBlt   ? "YES" : "NO"));

    if (context->multiSourceBlt)
    {
    LOGI("  Source mirror     : %s\n"
         "  Max source count  : %d\n"
         "  Byte alignment    : %s\n"
         "  Source rotation   : %s\n"
         "  Source offsets:   : %s\n"
         "  Scaling:          : %s\n",
         (context->msSourceMirror   ? "YES" : "NO"),
         (context->msMaxSource),
         (context->msByteAlignment  ? "YES" : "NO"),
         (context->msSourceRotation ? "YES" : "NO"),
         (context->msSourceOffset   ? "YES" : "NO"),
         (context->multiSourceBltV2 ? "YES" : "NO"));
    }

    LOGI("OPF/YUV blit        : %s\n"
         "Tiled input         : %s\n"
         "Compression         : %s\n"
         "Filter stretch      : %s\n",
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

