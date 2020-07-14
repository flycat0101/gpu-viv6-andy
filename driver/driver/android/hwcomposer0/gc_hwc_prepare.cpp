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


#include "gc_hwc.h"
#include "gc_hwc_debug.h"

#include <gc_gralloc_priv.h>

#if ENABLE_CLEAR_HOLE
static inline gctBOOL
_IsClearHole(
    hwc_layer_t * Layer
    )
{
    return (Layer->flags & HWC_SKIP_LAYER)
        && (Layer->flags & 0x02 /* CLEAR_HOLE flag. */)
        && (Layer->handle == NULL);
}
#endif


static inline gctBOOL
_IsSkip(
    hwc_layer_t * Layer
    )
{
    return (Layer->flags & HWC_SKIP_LAYER);
}


#if ENABLE_DIM
static inline gctBOOL
_IsDim(
    hwc_layer_t * Layer
    )
{
    return (Layer->blending & 0xFFFF) == 0x0805 /* HWC_BLENDING_DIM */;
}
#endif


static inline gctBOOL
_IsOverlay(
    hwc_layer_t * Layer
    )
{
    /* Cast handle. */
    gc_native_handle_t * handle = gc_native_handle_get(Layer->handle);

    if (Layer->handle == gcvNULL)
    {
        return gcvFALSE;
    }

    /* TODO (Soc-vendor): Check OVERLAY layer correctly.
     * Here we set overlay only when no surfaces in handle. This may be
     * because 'private_handle_t' is customized. */
    return (handle->surface == 0);
}


static  inline gctBOOL
_IsBlitter(
    hwcContext * Context,
    hwc_layer_t * Layer
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
**  hwcPrepare
**
**  Prepare for composition.
**  This function is only to set compositionType of each layer. Note that this
**  function could be called multiple times before 'hwcSet' is called.
**
**  INPUT:
**
**      hwcContext * Context
**          hwcomposer context pointer.
**
**      hwc_layer_list_t * List
**          All layers to be composed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
hwcPrepare(
    IN hwcContext * Context,
    IN hwc_layer_list_t * List
    )
{

    /* Reset geometry change flag. */
    Context->geometryChanged = gcvTRUE;

    /* Reset flags. */
    Context->hasGles         = gcvFALSE;
    Context->hasG2D          = gcvFALSE;
    Context->hasClearHole    = gcvFALSE;
    Context->hasDim          = gcvFALSE;
    Context->hasOverlay      = gcvFALSE;

    /* Go through all layer. */
    for (size_t i = 0; i < List->numHwLayers; i++)
    {
        hwc_layer_t * layer  = &List->hwLayers[i];

#if ENABLE_CLEAR_HOLE
        /* Check clear hole. */
        if (_IsClearHole(layer))
        {
            layer->flags          &= ~HWC_SKIP_LAYER;
            layer->compositionType =  HWC_CLEAR_HOLE;
            Context->hasClearHole  = gcvTRUE;

            continue;
        }
#endif

        /* Check skip. */
        if (_IsSkip(layer))
        {
            /* We are forbidden to touch this layer. */
            layer->compositionType  = HWC_FRAMEBUFFER;
            Context->hasGles        = gcvTRUE;

            continue;
        }

#if ENABLE_DIM

        /* Check dim. */
        if (_IsDim(layer))
        {
            /* Set dim. */
            layer->compositionType = HWC_DIM;
            Context->hasDim        = gcvTRUE;

            continue;
        }
#endif

        /* Check overlay. */
        if (_IsOverlay(layer))
        {
            /* Set overlay. */
            layer->compositionType = HWC_OVERLAY;
            Context->hasOverlay    = gcvTRUE;

            continue;
        }

        /* Check blitter. */
        if (_IsBlitter(Context, layer))
        {
            layer->compositionType = HWC_BLITTER;
            Context->hasG2D        = gcvTRUE;

            continue;
        }

        /* Fail back to 3D composition. */
        layer->compositionType  = HWC_FRAMEBUFFER;
        Context->hasGles = gcvTRUE;
    }

    if (Context->hasGles)
    {
        /* Reset flags. */
        Context->hasG2D       = gcvFALSE;
        Context->hasClearHole = gcvFALSE;
        Context->hasDim       = gcvFALSE;

        /* We need go through all layer again. */
        for (size_t i = 0; i < List->numHwLayers; i++)
        {
            hwc_layer_t * layer = &List->hwLayers[i];

            if (layer->compositionType == HWC_OVERLAY)
            {
                /* Set HWC_HINT_CLEAR_FB hint if overlay device needs it.
                 * Here we will do the composition with 3D, for some overlay
                 * devices, the overlay area should be cleared as transparet.
                 * Set HWC_HINT_CLEAR_FB to let surfaceflinger know this.
                 * See function 'setupHardwareComposer' in SurfaceFlinger.cpp */
#if CLEAR_FB_FOR_OVERLAY
                   layer->hints |= HWC_HINT_CLEAR_FB;
#endif
            }

            else if (layer->compositionType >= HWC_BLITTER)
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
        if (Context->hasClearHole || Context->hasDim)
        {
            /* Use G2D for clear hole and DIM. */
            Context->hasG2D = gcvTRUE;
        }

#if CLEAR_FB_FOR_OVERLAY
        /* Check overlay clearing. */
        if (!Context->hasG2D && Context->hasOverlay && !Context->pureOverlay)
        {
            /* All layers are overlay. Clear fb for overlay once. */
            Context->pureOverlay = gcvTRUE;
            Context->hasG2D = gcvTRUE;
        }
        else
        {
            /* Not all overlay layers. */
            Context->pureOverlay = gcvFALSE;
        }
#endif
    }

    return gcvSTATUS_OK;
}

