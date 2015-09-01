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


#include "gc_hwc.h"
#include "gc_hwc_display.h"
#include "gc_hwc_debug.h"

#include <gc_gralloc_priv.h>

#if ENABLE_CLEAR_HOLE
static inline gctBOOL
_IsClearHole(
    hwc_layer_1_t * Layer
    )
{
    return (Layer->flags & HWC_SKIP_LAYER)
        && (Layer->flags & 0x02 /* CLEAR_HOLE flag. */)
        && (Layer->handle == NULL);
}
#endif


static inline gctBOOL
_IsSkip(
    hwc_layer_1_t * Layer
    )
{
    return (Layer->flags & HWC_SKIP_LAYER);
}


#if ENABLE_DIM
static inline gctBOOL
_IsDim(
    hwc_layer_1_t * Layer
    )
{
    return (Layer->blending & 0xFFFF) == 0x0805 /* HWC_BLENDING_DIM */;
}
#endif


static inline gctBOOL
_IsOverlay(
    hwc_layer_1_t * Layer
    )
{
    /* Cast handle. */
    gc_native_handle_t * handle = gc_native_handle_get(Layer->handle);

    if (Layer->handle == gcvNULL)
    {
        return gcvFALSE;
    }

    /* TODO: Check OVERLAY layer correctly.
     * Here we set overlay only when no surfaces in handle. This may be
     * because 'private_handle_t' is customized. */
    return (handle->surface == 0);
}


static  inline gctBOOL
_IsBlitter(
    hwcContext * Context,
    hwc_layer_1_t * Layer
    )
{
    /* Cast handle. */
    gc_native_handle_t * handle = gc_native_handle_get(Layer->handle);

    if (Layer->handle == gcvNULL)
    {
        return gcvFALSE;
    }

    /* Check magic. */
    if (private_handle_t::validate(Layer->handle))
    {
        return gcvFALSE;
    }

    /* Check source rectangle. */
#if ANDROID_SDK_VERSION >= 19
    /* HWC 1.3 uses sourceCrop in float. */
    if (Context->device.common.version >= HWC_DEVICE_API_VERSION_1_3)
    {
        if ((Layer->sourceCropf.left   < 0)
        ||  (Layer->sourceCropf.top    < 0)
        ||  (Layer->sourceCropf.right  > handle->width)
        ||  (Layer->sourceCropf.bottom > handle->height)
        ||  (Layer->sourceCropf.left   >= Layer->sourceCropf.right)
        ||  (Layer->sourceCropf.top    >= Layer->sourceCropf.bottom)
        )
        {
            /* Source area out of source surface, do not handle it. */
            return gcvFALSE;
        }
    }
    else
#endif
    {
        if ((Layer->sourceCrop.left   < 0)
        ||  (Layer->sourceCrop.top    < 0)
        ||  (Layer->sourceCrop.right  > handle->width)
        ||  (Layer->sourceCrop.bottom > handle->height)
        ||  (Layer->sourceCrop.left   >= Layer->sourceCrop.right)
        ||  (Layer->sourceCrop.top    >= Layer->sourceCrop.bottom)
        )
        {
            /* Source area out of source surface, do not handle it. */
            return gcvFALSE;
        }
    }

    /* Check yuv source. */
    if ((handle->surface != 0)
    &&  (!Context->yuvSeparateStride)
    &&  (  (handle->halFormat == gcvSURF_YV12)
        || (handle->halFormat == gcvSURF_I420))
    )
    {
        gctINT stride;

        gcmVERIFY_OK(
            gcoSURF_GetAlignedSize((gcoSURF) handle->surface,
                                   gcvNULL, gcvNULL, &stride));

        if (stride & 0x1F)
        {
            /* Not 32 aligned, fail back to 3D composition. */
            return gcvFALSE;
        }
    }


    /* If tiling input is supported, we can always handle this layer.
     * Otherwise we need linear 'surface'. */
    if (Context->tiledInput)
    {
        return gcvTRUE;
    }
    else
    {
        gceTILING tiling;
        gcmVERIFY_OK(gcoSURF_GetTiling((gcoSURF) handle->surface, &tiling));

        return tiling == gcvLINEAR;
    }
}


/*******************************************************************************
**
**  _Prepare
**
**  Prepare for composition.
**  This function is only to set compositionType of each layer. Note that this
**  function could be called multiple times before 'hwc_set' is called.
**
**  INPUT:
**
**      hwcContext * Context
**          hwcomposer context pointer.
**
**      hwcDisplay * Display,
**          Target display.
**
**      hwc_display_contents_1_t * HwDisplay
**          Display contents(layers) to be composed.
**
**  OUTPUT:
**
**      Nothing.
*/
static void
_Prepare(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    )
{
    /* Reset flags. */
    Display->hasGles         = gcvFALSE;
    Display->hasG2D          = gcvFALSE;
    Display->hasClearHole    = gcvFALSE;
    Display->hasDim          = gcvFALSE;
    Display->hasOverlay      = gcvFALSE;

    /* Get layer count. */
    Display->layerCount      = HwDisplay->numHwLayers - 1;

    /* Go through all layer. */
    for (size_t i = 0; i < Display->layerCount; i++)
    {
        hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
        hwcLayer * layer = &Display->layers[i];

#if ENABLE_CLEAR_HOLE
        /* Check clear hole. */
        if (_IsClearHole(hwLayer))
        {
            hwLayer->flags        &= ~HWC_SKIP_LAYER;
            layer->compositionType =  HWC_CLEAR_HOLE;
            Display->hasClearHole  = gcvTRUE;

            continue;
        }
#endif

        /* Check skip. */
        if (_IsSkip(hwLayer))
        {
            /* We are forbidden to touch this layer. */
            layer->compositionType  = HWC_FRAMEBUFFER;
            Display->hasGles        = gcvTRUE;

            continue;
        }

#if ENABLE_DIM

        /* Check dim. */
        if (_IsDim(hwLayer))
        {
            /* Set dim. */
            layer->compositionType = HWC_DIM;
            Display->hasDim        = gcvTRUE;

            continue;
        }
#endif

        /* Check overlay. */
        if (_IsOverlay(hwLayer))
        {
            /* Set overlay. */
            layer->compositionType = HWC_OVERLAY;
            Display->hasOverlay    = gcvTRUE;

            continue;
        }

        /* Check blitter. */
        if (_IsBlitter(Context, hwLayer))
        {
            layer->compositionType = HWC_BLITTER;
            Display->hasG2D        = gcvTRUE;

            continue;
        }

        /* Fail back to 3D composition. */
        layer->compositionType  = HWC_FRAMEBUFFER;
        Display->hasGles        = gcvTRUE;
    }

    /* Layer count check. */
    if (Display->layerCount > 32)
    {
        /* Use OpenGL ES for all non-overlay layers. */
        Display->hasGles = gcvTRUE;
    }

    /* Check for display target. */
    if (!Display->hasGles)
    {
        hwc_layer_1_t * displayTarget;
        displayTarget = &HwDisplay->hwLayers[HwDisplay->numHwLayers - 1];

        if (displayTarget->handle == NULL)
        {
            /* Can not do hwcomposer composition without display target. */
            Display->hasGles = gcvTRUE;
        }

        else
        {
            /* Check tiling of display target. */
            gceTILING tiling;
            gc_native_handle_t * handle;

            /* Cast handle. */
            handle = gc_native_handle_get(displayTarget->handle);

            gcmVERIFY_OK(
                gcoSURF_GetTiling((gcoSURF) handle->surface, &tiling));

            switch (tiling)
            {
            case gcvLINEAR:
                break;
            case gcvTILED:
                if (!Context->tiledInput)
                {
                    /* 2D does not support tiled target. */
                    Display->hasGles = gcvTRUE;
                }
                break;
            default:
                /* 2D does not support other tiled target. */
                Display->hasGles = gcvTRUE;
                break;
            }
        }

        if (!Display->hasGles &&
            Display->disp >= int(Context->numPhysicalDisplays))
        {
            /* Check outbuf for virtual displays. */
            /* Check tiling of outbuf. */
            gceTILING tiling;
            gc_native_handle_t * handle;

            /* Cast handle. */
            handle = gc_native_handle_get(HwDisplay->outbuf);

            gcmVERIFY_OK(
                gcoSURF_GetTiling((gcoSURF) handle->surface, &tiling));

            switch (tiling)
            {
            case gcvLINEAR:
                break;
            case gcvTILED:
                if (!Context->tiledInput)
                {
                    /* 2D does not support tiled target. */
                    Display->hasGles = gcvTRUE;
                }
                break;
            default:
                /* 2D does not support other tiled target. */
                Display->hasGles = gcvTRUE;
                break;
            }
        }
    }

    if (Display->hasGles)
    {
        /* Reset flags. */
        Display->hasG2D       = gcvFALSE;
        Display->hasClearHole = gcvFALSE;
        Display->hasDim       = gcvFALSE;

        /* We need go through all layer again. */
        for (size_t i = 0; i < Display->layerCount; i++)
        {
            hwc_layer_1_t * hwLayer  = &HwDisplay->hwLayers[i];
            hwcLayer * layer = &Display->layers[i];

            if (layer->compositionType == HWC_OVERLAY)
            {
                /* Set HWC_HINT_CLEAR_FB hint if overlay device needs it.
                 * Here we will do the composition with 3D, for some overlay
                 * devices, the overlay area should be cleared as transparet.
                 * Set HWC_HINT_CLEAR_FB to let surfaceflinger know this.
                 * See function 'setupHardwareComposer' in SurfaceFlinger.cpp */
#if CLEAR_FB_FOR_OVERLAY
                hwLayer->hints |= HWC_HINT_CLEAR_FB;
#endif
            }

            else
            {
                /* Roll back all hwc layers. */
                layer->compositionType  = HWC_FRAMEBUFFER;
            }
        }

        /* Print a log indicating 3D composition is used. */
        /* LOGI("hwc prepare: 3D composition"); */
    }

    else
    {
        if (Display->hasClearHole || Display->hasDim)
        {
            /* Use G2D for clear hole and DIM. */
            Display->hasG2D = gcvTRUE;
        }

#if CLEAR_FB_FOR_OVERLAY
        /* Check overlay clearing. */
        if (!Display->hasG2D && Display->hasOverlay && !Display->pureOverlay)
        {
            /* All layers are overlay. Clear fb for overlay once. */
            Display->pureOverlay = gcvTRUE;
            Display->hasG2D = gcvTRUE;
        }
        else
        {
            /* Not all overlay layers. */
            Display->pureOverlay = gcvFALSE;
        }
#endif
    }

    /*
     * Check if this 2D composition is exactly the same as previous.
     * If it is, skip all 2D operations includes buffer flip.
     */
    gctBOOL identical = gcvTRUE;

    if (!Display->hasG2D ||
        (HwDisplay->flags & HWC_GEOMETRY_CHANGED) ||
        (Display->layerCount != Display->identityCount))
    {
        identical = gcvFALSE;
    }
    else
    {
        for (size_t i = 0; i < Display->layerCount; i++)
        {
            hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
            hwcLayer * layer = &Display->layers[i];
            hwcLayerIdentity   current;
            hwcLayerIdentity * identity = &Display->identities[i];

            if ((identity->type       != layer->compositionType) ||
                (identity->hints      != hwLayer->hints) ||
                (identity->flags      != hwLayer->flags) ||
                (identity->transform  != hwLayer->transform) ||
                (identity->blending   != hwLayer->blending) ||
#if ANDROID_SDK_VERSION >= 18
                (identity->planeAlpha != hwLayer->planeAlpha)
#elif ENABLE_PLANE_ALPHA || ENABLE_DIM
                (identity->planeAlpha != hwLayer->alpha)
#endif
                )
            {
                identical = gcvFALSE;
                break;
            }

            if (layer->compositionType == HWC_BLITTER)
            {
                if ((identity->handle != hwLayer->handle) ||
                    (identity->node   != gc_native_handle_get(hwLayer->handle)->node))
                {
                    identical = gcvFALSE;
                    break;
                }
            }
        }
    }

    if (identical)
    {
        /* Skip all 2D operations. */
        Display->hasG2D = gcvFALSE;
    }
    else if (Display->hasG2D)
    {
        Display->identityCount = Display->layerCount;

        /* Copy current identities. */
        for (size_t i = 0; i < Display->layerCount; i++)
        {
            hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
            hwcLayer * layer = &Display->layers[i];
            hwcLayerIdentity * identity = &Display->identities[i];

            identity->type       = layer->compositionType;
            identity->hints      = hwLayer->hints;
            identity->flags      = hwLayer->flags;
            identity->transform  = hwLayer->transform;
            identity->blending   = hwLayer->blending;
#if ANDROID_SDK_VERSION >= 18
            identity->planeAlpha = hwLayer->planeAlpha;
#elif ENABLE_PLANE_ALPHA || ENABLE_DIM
            identity->planeAlpha = hwLayer->alpha;
#endif

            if (layer->compositionType == HWC_BLITTER)
            {
                identity->handle = hwLayer->handle;
                identity->node   = gc_native_handle_get(hwLayer->handle)->node;
            }
        }
    }

    /* Copy out 'compositionType' for all layers. */
    for (size_t i = 0; i < Display->layerCount; i++)
    {
        hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
        hwcLayer * layer = &Display->layers[i];

        if (layer->compositionType == HWC_FRAMEBUFFER)
        {
            hwLayer->compositionType = HWC_FRAMEBUFFER;
        }
        else
        {
            /*
             * Set 'layer_1_t::compositionType' to 'HWC_OVERLAY'.
             * NOTICE: this means the layer is handled by HWC. Vivante HWC
             * will composose depends on extended compositionType saved
             * in 'hwcLayer::compositionType'.
             */
            hwLayer->compositionType = HWC_OVERLAY;
        }
    }
}


/*
 * (*prepare)() is called for each frame before composition and is used by
 * SurfaceFlinger to determine what composition steps the HWC can handle.
 *
 * (*prepare)() can be called more than once, the last call prevails.
 *
 * The HWC responds by setting the compositionType field in each layer to
 * either HWC_FRAMEBUFFER or HWC_OVERLAY. In the former case, the
 * composition for the layer is handled by SurfaceFlinger with OpenGL ES,
 * in the later case, the HWC will have to handle the layer's composition.
 *
 * (*prepare)() is called with HWC_GEOMETRY_CHANGED to indicate that the
 * list's geometry has changed, that is, when more than just the buffer's
 * handles have been updated. Typically this happens (but is not limited to)
 * when a window is added, removed, resized or moved.
 *
 * For HWC 1.0, numDisplays will always be one, and displays[0] will be
 * non-NULL.
 *
 * For HWC 1.1, numDisplays will always be HWC_NUM_DISPLAY_TYPES. Entries
 * for unsupported or disabled/disconnected display types will be NULL.
 *
 * For HWC 1.2 and later, numDisplays will be HWC_NUM_DISPLAY_TYPES or more.
 * The extra entries correspond to enabled virtual displays, and will be
 * non-NULL. In HWC 1.2, support for one virtual display is required, and
 * no more than one will be used. Future HWC versions might require more.
 *
 * returns: 0 on success. An negative error code on error. If an error is
 * returned, SurfaceFlinger will assume that none of the layer will be
 * handled by the HWC.
 */
int
hwc_prepare(
    hwc_composer_device_1_t * dev,
    size_t NumDisplays,
    hwc_display_contents_1_t** HwDisplays
    )
{
    hwcContext * context = (hwcContext *) dev;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    if (NumDisplays > context->numDisplays)
    {
        /* Add more virtual displays. */
        hwcDetectVirtualDisplays(context, NumDisplays);
    }

    for (size_t i = 0; i < NumDisplays; i++)
    {
        hwcDisplay * dpy;
        hwc_display_contents_1_t * hwDisplay;

        dpy = context->displays[i];
        hwDisplay = HwDisplays[i];

        if ((hwDisplay == NULL) || (hwDisplay->numHwLayers <= 1))
        {
            /* Bail out for no display. */
            continue;
        }

        /* Prepare for this display. */
        _Prepare(context, dpy, hwDisplay);
    }

    return 0;
}

