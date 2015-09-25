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

#include <gc_hal.h>
#include <gc_hal_driver.h>
#include <gc_gralloc_priv.h>

#include <cutils/properties.h>

#include <linux/fb.h>

#include <stdlib.h>
#include <errno.h>
#include <math.h>

static void
_DetectSource(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay,
    IN gctINT Index
    );

static hwcArea *
_AllocateArea(
    IN hwcDisplay * Display,
    IN hwcArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    );

static void
_FreeArea(
    IN hwcDisplay * Display,
    IN hwcArea *    Head
    );

static void
_SplitArea(
    IN hwcDisplay * Display,
    IN hwcArea * Area,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    );

static gctBOOL
_HasAlpha(
    IN gceSURF_FORMAT Format
    );


/*******************************************************************************
**
**  _Set
**
**  Translate android composition parameters to hardware specific.
**
**  For 2D compsition, it will translate android parameters to GC parameters
**  and start GC 2D operations.
**
**  For 3D compsition when failed to use hwc composition, this function will
*   start overlay only.
**
**  Swap rectangle change is not indicated by geometry chagne, so swap rect
**  clamp is not done in parameter generation.
**
**
**  INPUT:
**
**      hwcContext * Context
**          hwcomposer context pointer.
**
**      buffer_handle_t Handle
**          Pointer to target handle.
**
**      gcsREcT_PTR SwapRect
**          Swap rectangle area.
**
**      hwcDisplay * Display,
**          The target display to render onto.
**
**      hwc_display_contents_1_t * HwDisplay,
**          Display contents to be composed.
**
**  OUTPUT:
**
**      Nothing.
*/
void
_Set(
    IN hwcContext * Context,
    IN buffer_handle_t Handle,
    IN gcsRECT_PTR SwapRect,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    )
{

    /***************************************************************************
    ** Display Detection.
    */

    if (Display->hasG2D)
    {
        /* Get GC handle. */
        gc_native_handle_t * handle;
        hwcBuffer * target;

        gceSURF_FORMAT format;
        gceTILING tiling;
        gctINT32 xres;
        gctINT32 yres;
        gctUINT32 stride;

        gctUINT physical[3];

        /* Get private handle. */
        handle = gc_native_handle_get(Handle);

        /* Get surface wrapper. */
        gcoSURF surface = (gcoSURF) intptr_t(handle->surface);

        /* Lock target for GPU address. */
        gcmVERIFY_OK(gcoSURF_Lock(surface, physical, gcvNULL));

        /* Get format. */
        gcmVERIFY_OK(gcoSURF_GetFormat(surface, gcvNULL, &format));

        /* Get display size. */
        gcmVERIFY_OK(
            gcoSURF_GetSize(surface, (gctUINT32 *) &xres, (gctUINT32 *) &yres, gcvNULL));

        /* Get stride. */
        gcmVERIFY_OK(
            gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, (gctINT32_PTR) &stride));

        /* Get tiling. */
        gcoSURF_GetTiling(surface, &tiling);

        if ((Display->format != gcvSURF_UNKNOWN)
        &&  (   (Display->format     != format)
            ||  (Display->res.right  != xres)
            ||  (Display->res.bottom != yres)
            ||  (Display->stride     != stride))
        )
        {
            /* Display configuration has changed. */
            target = Display->head;

            do
            {
                hwcBuffer * next = target->next;

                free(target);
                target = next;
            }
            while (target != Display->head);

            /* Reset display configuration. */
            Display->target = NULL;
            Display->format = gcvSURF_UNKNOWN;

            LOGI("Display(%p) changed: format=%d tiling=%x %dx%d handle=%p usage=%x",
                 Display, format, tiling, xres, yres, Handle, handle->allocUsage);
        }

        if (Display->format == gcvSURF_UNKNOWN)
        {
            gcsSURF_FORMAT_INFO_PTR info[2];

            /* Save display format. */
            Display->format     = format;

            /* Save display resolution. */
            Display->res.left   = 0;
            Display->res.top    = 0;
            Display->res.right  = xres;
            Display->res.bottom = yres;

            /* Stride. */
            Display->stride     = stride;

            /* Get tiling. */
            Display->tiling     = tiling;

            /* Query format info. */
            gcmVERIFY_OK(gcoSURF_QueryFormat(Display->format, info));

            /* Get bytes per pixel. */
            Display->bytesPerPixel = info[0]->bitsPerPixel / 8;

            /* Get multi-source blit alignment mask. */
            if ((Display->format >= gcvSURF_YUY2)
            &&  (Display->format <= gcvSURF_NV16)
            )
            {
                /* YUV display? */
                Display->alignMask =
                    YUV_BUFFER_ALIGN / Display->bytesPerPixel - 1;
            }
            else
            {
                /* RGB display. */
                Display->alignMask =
                    RGB_BUFFER_ALIGN / Display->bytesPerPixel - 1;
            }

            /* Clear valid flag. */
            Display->valid = gcvFALSE;

            /* Set output compression. */
            switch (format)
            {
#if ENABLE_FRAMEBUFFER_COMPRESS
            /* Only following formats can support compression. */
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A8B8G8R8:
            case gcvSURF_X8B8G8R8:
                Display->compression = gcvTRUE;
                break;
#endif

            default:
                Display->compression = gcvFALSE;
                break;
            }

            /* Allocate target display buffer. */
            target = (hwcBuffer *) malloc(sizeof (hwcBuffer));

            if (!target)
            {
                return;
            }

            /* Save handle. */
            target->handle = Handle;

            /* Save physical address. */
            target->physical = physical[0];

            /* Calculate framebuffer tile status address. */
            if (Display->compression)
            {
                /* TODO: Assume tile status address is next to fb address. */
                target->tsPhysical = target->physical + stride * yres;
            }
            else
            {
                target->tsPhysical = ~0U;
            }

            /* Initially disable 2D compression. */
            target->tsConfig = gcv2D_TSC_DISABLE;

            /* Point prev/next buffer to self. */
            target->prev = target->next = target;

            /* Save target handle to display struct. */
            Display->head = Display->target = target;

            /* Logs. */
            LOGI("Display(%p) detected: format=%d %dx%d",
                 Display, Display->format, xres, yres);

            LOGI("Display buffer(%p) detected: physical=0x%08x",
                 target, target->physical);
        }

        /* Get target display buffer. */
        target = Display->target = Display->target->next;

        if (Display->disp >= int(Context->numPhysicalDisplays))
        {
            /* Use only one target for virtual displays. */
            /* Save buffer handle. */
            target->handle   = Handle;

            /* Get buffer address. */
            target->physical = physical[0];
        }

        /* Check physical address. */
        if (target->physical != physical[0])
        {
            /* Buffer physical is not the same as address pointed by the,
             * handle, we need to check again.
             * This case may be because some frames composed by 3D or a new
             * buffer detected. */
            do
            {
                if (target->physical == physical[0])
                {
                    /* Found. */
                    break;
                }

                target = target->next;
            }
            while (target != Display->target);

            /* Check physical address. */
            if (target->physical != physical[0])
            {
                /* Allocate new buffer when not found. */
                target = (hwcBuffer *) malloc(sizeof (hwcBuffer));

                if (!target)
                {
                    return;
                }

                /* Save buffer handle. */
                target->handle = Handle;

                /* Get buffer address. */
                target->physical = physical[0];

                /* Calculate framebuffer tile status address. */
                if (Display->compression)
                {
                    /* TODO: Assume tile status address is next to fb address. */
                    target->tsPhysical = target->physical
                                       + Display->stride * Display->res.bottom;
                }
                else
                {
                    target->tsPhysical = ~0U;
                }

                /* Initially disable 2D compression. */
                target->tsConfig = gcv2D_TSC_DISABLE;

                /* Insert target before Display->target. */
                hwcBuffer * t = Display->target;

                t->prev->next = target;
                target->prev  = t->prev;

                target->next = t;
                t->prev      = target;

                /* Logs. */
                LOGI("Display buffer(%p) detected: physical=0x%08x",
                     target, target->physical);
            }

            else if (Display->valid)
            {
                /* Adust buffer sequence:
                 * isolate target and put it before Display->target. */
                target->prev->next = target->next;
                target->next->prev = target->prev;

                /* Insert target before Display->target. */
                hwcBuffer * t = Display->target;

                t->prev->next = target;
                target->prev  = t->prev;

                target->next = t;
                t->prev      = target;
            }

            /* Update target buffer. */
            Display->target  = target;
        }

        if (Display->compression && !Context->disableCompression)
        {
            /* Enable 2D compression for this frame if available. */
            target->tsConfig = gcv2D_TSC_2D_COMPRESSED;
        }

#if ENABLE_SWAP_RECTANGLE
        /* Save swap rectangle. */
        if (SwapRect)
        {
            target->swapRect = *SwapRect;
        }
        else
        {
            target->swapRect = Display->res;
        }
#endif

        /* Update valid flag. */
        if (Display->valid == gcvFALSE)
        {
            /* Reset swap rectangles of other buffers. */
            hwcBuffer * buffer = target->next;

            while (buffer != target)
            {
#if ENABLE_SWAP_RECTANGLE
                /* Reset size to full screen. */
                buffer->swapRect = Display->res;
#endif

                if (Display->compression)
                {
                    /* No 2D compression when 3D composition. */
                    buffer->tsConfig = gcv2D_TSC_DISABLE;
                }

                buffer = buffer->next;
            }

            /* Set valid falg. */
            Display->valid = gcvTRUE;
        }
    }

    else
    {
        /* 3D composition path, remove the valid flag. */
        Display->valid = gcvFALSE;
    }


    /***************************************************************************
    ** Update Layer Information and Geometry.
    */

    if (Display->hasG2D)
    {
        /* Generate new layers. */
        for (gctUINT32 i = 0; i < Display->layerCount; i++)
        {
            /* Get shortcuts. */
            hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
            hwcLayer *      layer   = &Display->layers[i];
            hwc_region_t *  region  = &hwLayer->visibleRegionScreen;

            gctBOOL perpixelAlpha;
            gctUINT32 planeAlpha;

            /* Dest rectangle. */
            layer->dstRect         = *(gcsRECT *) &hwLayer->displayFrame;

            /* Dest clips. */
            layer->clipRects       = (gcsRECT_PTR) region->rects;
            layer->clipCount       = region->numRects;

            switch (layer->compositionType)
            {
            case HWC_OVERLAY:
            default:

                /* Set no source surface. Color should be from solid color32. */
                layer->source = gcvNULL;

                /* Normally transparent black for overlay area. */
                layer->color32 = 0U;

                /* Disable alpha blending. */
                layer->opaque  = gcvTRUE;

                break;

            case HWC_BLITTER:

                /* Detect source buffer. */
                _DetectSource(Context, Display, HwDisplay, i);

                /* Get alpha blending/premultiply states. */
                switch (hwLayer->blending & 0xFFFF)
                {
                case HWC_BLENDING_PREMULT:
                    /* Get perpixelAlpha enabling state. */
                    perpixelAlpha = _HasAlpha(layer->format);

                    /* Get plane alpha value. */
#if ANDROID_SDK_VERSION >= 18
                    planeAlpha    = hwLayer->planeAlpha;
#else
#if ENABLE_PLANE_ALPHA
                    planeAlpha    = hwLayer->alpha;
#   else
                    planeAlpha    = 0xFF;
#   endif
#endif

                    /* Alpha blending is not needed for layer 0. */
                    layer->opaque = (i == 0U) ? gcvTRUE : gcvFALSE;

                    if (perpixelAlpha && planeAlpha < 0xFF)
                    {
                        /*
                         * Cs' = Cs * Ags
                         * As' = As * Ags
                         *
                         * C  = Cs' + Cd * (1 - As') = Cs * Ags + Cd * (1 - As * Ags)
                         * A  = As' + Ad * (1 - As') = As * Ags + Ad * (1 - As * Ags)
                         */
                        /* Perpixel alpha and plane alpha. */
                        layer->srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_SCALE;
                        layer->dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                        layer->srcFactorMode        = gcvSURF_BLEND_ONE;
                        layer->dstFactorMode        = gcvSURF_BLEND_INVERSED;

                        /* Premultiply with src global alpha. */
                        layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR;
                        layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                        /* Global alpha values. */
                        layer->srcGlobalAlpha       = planeAlpha;
                    }

                    else if (perpixelAlpha)
                    {
                        /*
                         * Cs' = Cs
                         * As' = As
                         *
                         * C  = Cs' + Cd * (1 - As') = Cs + Cd * (1 - As)
                         * A  = As' + Ad * (1 - As') = As + Ad * (1 - As)
                         */
                        /* Perpixel alpha only. */
                        layer->srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                        layer->dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                        layer->srcFactorMode        = gcvSURF_BLEND_ONE;
                        layer->dstFactorMode        = gcvSURF_BLEND_INVERSED;

                        /* Disable premultiply. */
                        layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
                        layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                        /* Global alpha values. */
                        layer->srcGlobalAlpha       = 0xFF;
                    }

                    else
                    {
                        /*
                         * Cs' = Cs * Ags
                         * As' = 1  * Ags
                         *
                         * C  = Cs' + Cd * (1 - As') = Cs * Ags + Cd * (1 - Ags)
                         * A  = As' + Ad * (1 - As') = 1  * Ags + Ad * (1 - Ags)
                         */
                        /* Plane alpha only. */
                        layer->srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                        layer->srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_ON;
                        layer->dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                        layer->srcFactorMode        = gcvSURF_BLEND_ONE;
                        layer->dstFactorMode        = gcvSURF_BLEND_INVERSED;

                        /* Premultiply with src global alpha. */
                        layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                        layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR;
                        layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                        /* Global alpha values. */
                        layer->srcGlobalAlpha       = planeAlpha;
                    }

                    break;

                case HWC_BLENDING_COVERAGE:

                    LOGW("COVERAGE alpha blending...");
                    /* Get plane alpha value. */
#if ANDROID_SDK_VERSION >= 18
                    planeAlpha    = hwLayer->planeAlpha;
#else
#if ENABLE_PLANE_ALPHA
                    planeAlpha    = hwLayer->alpha;
#   else
                    planeAlpha    = 0xFF;
#   endif
#endif

                    /* Alpha blending is needed. */
                    layer->opaque = gcvFALSE;

                    /* Alpha blending parameters.
                     * TODO: this may be incorrect. */
                    layer->srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                    layer->dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                    layer->srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_ON;
                    layer->dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                    layer->srcFactorMode        = gcvSURF_BLEND_ONE;
                    layer->dstFactorMode        = gcvSURF_BLEND_INVERSED;

                    /* Premultiply with src global alpha. */
                    layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR;
                    layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                    /* Global alpha values. */
                    layer->srcGlobalAlpha       = planeAlpha;

                    break;

                case HWC_BLENDING_NONE:
                default:

                    /* Alpha blending is not needed. */
                    layer->opaque = gcvTRUE;

                    /* Disable premultiply. */
                    layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
                    layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                    /* Global alpha values. */
                    layer->srcGlobalAlpha       = 0xFF;

                    break;
                }

                break;

#if ENABLE_CLEAR_HOLE

            case HWC_CLEAR_HOLE:

                /* Set no source surface. Color should be from solid color32. */
                layer->source     = gcvNULL;

                /* Opaque black for clear hole. */
                layer->color32    = 0xFF000000;

                /* Disable alpha blending. */
                layer->opaque     = gcvTRUE;

                /* Disable premutiply. */
                layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
                layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;

                break;
#endif

#if ENABLE_DIM
            case HWC_DIM:

                /* Set no source surface. Color should be from solid color32. */
                layer->source  = gcvNULL;

                /* Get alpha channel. */
#if ANDROID_SDK_VERSION >= 18
                layer->color32 = hwLayer->planeAlpha << 24;
#   else
                layer->color32 = hwLayer->alpha << 24;
#   endif

                if (layer->color32 == 0xFF000000)
                {
                    /* Make a clear without alpha blending when alpha is 0xFF. */
                    layer->opaque  = gcvTRUE;

                    /* Disable premutiply. */
                    layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
                    layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;
                }

                else
                {
                    /* Do a blit with alpha blending. */
                    layer->opaque = gcvFALSE;

                    /* Alpha blending parameters. */
                    layer->srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                    layer->dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                    layer->srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                    layer->dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_OFF;
                    layer->srcFactorMode        = gcvSURF_BLEND_ONE;
                    layer->dstFactorMode        = gcvSURF_BLEND_INVERSED;

                    /* Disable premutiply. */
                    layer->srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_DISABLE;
                    layer->srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
                    layer->dstDemultDstAlpha    = gcv2D_COLOR_MULTIPLY_DISABLE;
                }

                /* Global alpha values. */
                layer->srcGlobalAlpha       = 0xFF;

                break;
#endif
            }
        }

        /* Reset allocated areas. */
        if (Display->compositionArea != NULL)
        {
            _FreeArea(Display, Display->compositionArea);

            Display->compositionArea = NULL;
#if ENABLE_SWAP_RECTANGLE
            Display->swapArea        = NULL;
#endif
        }

        /* Generate new areas. */
        /* Put a no-owner area with screen size, this is for worm hole,
         * and is needed for clipping. */
        Display->compositionArea = _AllocateArea(Display,
                                                 NULL,
                                                 &Display->res,
                                                 0U);

        /* Split areas: go through all regions. */
        for (gctUINT32 i = 0; i < Display->layerCount && i < 32; i++)
        {
            gctUINT32 owner = 1U << i;
            hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
            hwc_region_t *  region  = &hwLayer->visibleRegionScreen;
            hwc_rect_t * dstRect = &hwLayer->displayFrame;

            /* Now go through all rectangles to split areas. */
            for (gctUINT32 j = 0; j < region->numRects; j++)
            {
                gcsRECT rect = {
                    region->rects[j].left,
                    region->rects[j].top,
                    region->rects[j].right,
                    region->rects[j].bottom,
                };

                /* Clip into display frame. */
                if (rect.left   < dstRect->left)   { rect.left   = dstRect->left;   }
                if (rect.top    < dstRect->top)    { rect.top    = dstRect->top;    }
                if (rect.right  > dstRect->right)  { rect.right  = dstRect->right;  }
                if (rect.bottom > dstRect->bottom) { rect.bottom = dstRect->bottom; }

                if ((rect.left >= Display->res.right)
                ||  (rect.top  >= Display->res.bottom)
                ||  (rect.right  <= 0)
                ||  (rect.bottom <= 0)
                )
                {
                    /* Skip rectangle out of display. */
                    continue;
                }

                /* Clip into display size. */
                if (rect.left < 0) { rect.left = 0; }
                if (rect.top  < 0) { rect.top  = 0; }
                if (rect.right  > Display->res.right)  { rect.right  = Display->res.right;  }
                if (rect.bottom > Display->res.bottom) { rect.bottom = Display->res.bottom; }

                _SplitArea(Display,
                           Display->compositionArea,
                           &rect,
                           owner);
            }
        }
    }

#if ENABLE_CLEAR_HOLE

    /***************************************************************************
    ** 'CLEAR_HOLE' Corretion.
    */

    if (Display->hasG2D && Display->hasClearHole)
    {
        /* If some layer has compositionType 'CLEAR_HOLE', but it has one or
         * more layers under it, skip 'CLEAR_HOLE' operation.
         * See 'no texture' section in drawWithOpenGL in LayerBase.cpp. */
        for (gctUINT32 i = 0; i < Display->layerCount; i++)
        {
            if (Display->layers[i].compositionType != HWC_CLEAR_HOLE)
            {
                /* Likely, not clear hole. */
                continue;
            }

            /* Find a layer with clear hole. */
            gctUINT32 owner = (1U << i);

            hwcArea * area = Display->compositionArea;

            while (area != NULL)
            {
                if ((area->owners & owner) && (area->owners & (owner - 1U)))
                {
                    /* This area has clear hole layer, but it also has
                     * other layers under it. So it should NOT be
                     * cleared */
                    area->owners &= ~owner;
                }

                /* Advance to next area. */
                area = area->next;
            }
        }
    }
#endif


    /***************************************************************************
    ** Opaque Optimization.
    */

    if (Display->hasG2D && Display->hasDim)
    {
        /* DIM value 255 is very special. It does not do any dim effection but
         * only make a clear to (0,0,0,1). So we do not need to draw any layers
         * under it.  See LayerDim::onDraw.
         * Same for other opaque layer. */
        for (gctUINT32 i = 1; i < Display->layerCount; i++)
        {
            if (Display->layers[i].opaque == gcvFALSE)
            {
                /* Likely, not opaque layer. */
                continue;
            }

            /* Find a layer with solid dim. */
            gctUINT32 owner = (1U << i);

            hwcArea * area = Display->compositionArea;

            while (area != NULL)
            {
                if ((area->owners & owner) && (area->owners & (owner - 1U)))
                {
                    /* This area has solid dim, but it also has
                     * other layers under it. We do not need to blit
                     * layers under it. */
                    area->owners &= ~(owner - 1U);
                }

                /* Advance to next area. */
                area = area->next;
            }
        }
    }

    /***************************************************************************
    ** Source Buffer Detection.
    */

    if (Display->hasG2D)
    {
        /* Geometry is not changed and we are doing 2D composition. So we need
         * to update source for each layer. */
        for (gctUINT32 i = 0; i < Display->layerCount; i++)
        {
            /* Get shortcuts. */
            hwcLayer * layer = &Display->layers[i];

            /* Update layer source (for blitter layer). */
            if (layer->source != gcvNULL)
            {
                gceSURF_TYPE   type;
                gceSURF_FORMAT format;
                gceTILING      tiling;
                gctUINT        stride;

                gc_native_handle_t * handle
                    = gc_native_handle_get(HwDisplay->hwLayers[i].handle);

                /* Update source. */
                layer->source = (gcoSURF) intptr_t(handle->surface);

                /* Get source surface stride. */
                gcmVERIFY_OK(
                    gcoSURF_GetAlignedSize(layer->source,
                                           gcvNULL, gcvNULL,
                                           (gctINT *) &stride));

                /* Get source surface format. */
                gcmVERIFY_OK(gcoSURF_GetFormat(layer->source, &type, &format));

                /* Get source surface tiling. */
                gcmVERIFY_OK(gcoSURF_GetTiling(layer->source, &tiling));

                /* Check source buffer voilation. */
                if ((format != layer->format)
                ||  (stride != layer->strides[0])
                ||  (tiling != layer->tiling)
                )
                {
                    /* Source buffer voilation detected.
                     * This is a very rare case: geometry is not changed but
                     * source contents are change to buffers belongs another
                     * window. */
                    _DetectSource(Context, Display, HwDisplay, i);
                }

#if gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
                /* Pop up shared states. */
                gcoSURF_PopSharedInfo(layer->source);
#endif

                /* Get source surface base address. */
                gcmVERIFY_OK(
                    gcoSURF_Lock(layer->source, layer->addresses, gcvNULL));

                /* Assume the two(or more) buffers are same in size,
                 * format, tiling etc. */
                if (layer->yuv)
                {
                    switch (layer->format)
                    {
                    case gcvSURF_NV12:
                    case gcvSURF_NV21:
                    case gcvSURF_NV16:
                    case gcvSURF_NV61:
                        layer->strides[2] = layer->strides[1]
                                          = layer->strides[0];
                        layer->addressNum = layer->strideNum = 2U;
                        break;

                    case gcvSURF_YV12:
                    case gcvSURF_I420:
                        /* This alignment limitation is from
                         * frameworks/base/libs/gui/tests/SurfaceTexture_test.cpp. */
                        layer->strides[2] = layer->strides[1]
                                          = (layer->strides[0] / 2 + 0xf) & ~0xf;
                        layer->addressNum = layer->strideNum = 3U;
                        break;

                    case gcvSURF_YUY2:
                    case gcvSURF_UYVY:
                    default:
                        layer->strides[2] = layer->strides[1]
                                          = layer->strides[0];
                        layer->addressNum = layer->strideNum = 1U;
                        break;
                    }
                }

                else if (layer->tiling & gcvTILING_SPLIT_BUFFER)
                {
                    gctUINT offset = 0;

                    gcmVERIFY_OK(
                        gcoSURF_GetBottomBufferOffset(layer->source,
                                                      &offset));

                    layer->addresses[1] = layer->addresses[0] + offset;
                    layer->addressNum   = 2;

                    layer->strides[1]   = layer->strides[0];
                    layer->strideNum    = 2;
                }

                else
                {
                    layer->addressNum   = 1;
                    layer->strideNum    = 1;
                }
            }
        }

        if (Context->tiledCompressQuirks &&
            (Display->target->tsConfig == gcv2D_TSC_2D_COMPRESSED))
        {
            gctUINT32 tiledPixels = 0;
            gctUINT32 screenPixels;

            screenPixels = Display->res.right * Display->res.bottom;

            for (gctUINT32 i = 0; i < Display->layerCount; i++)
            {
                /* Get shortcuts. */
                hwcLayer * layer = &Display->layers[i];
                hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[i];
                hwc_region_t *  region  = &hwLayer->visibleRegionScreen;

                if (layer->source != gcvNULL && layer->tiling != gcvLINEAR)
                {
                    for (gctUINT32 j = 0; j < region->numRects; j++)
                    {
                        const hwc_rect_t * rect = &region->rects[j];
                        tiledPixels += (rect->right  - rect->left)
                                     * (rect->bottom - rect->top);
                    }
                }
            }

            if (tiledPixels >= screenPixels * 3 / 2)
            {
                /* Too many pixels in tiled input, disable compression. */
                Display->target->tsConfig = gcv2D_TSC_DISABLE;
            }
        }
    }

#if ENABLE_SWAP_RECTANGLE

    /***************************************************************************
    ** Swap Rectangle Optimization.
    */

    if (Display->hasG2D)
    {
        /* Get short cuts. */
        hwcBuffer * buffer           = Display->target;

        /* Free all allocated swap areas. */
        if (Display->swapArea != NULL)
        {
            _FreeArea(Display, Display->swapArea);
            Display->swapArea = NULL;
        }

        /* Optimize: do not need split area for full screen composition. */
        if (
            (   (buffer->swapRect.left > 0)
            ||  (buffer->swapRect.top  > 0)
            ||  (buffer->swapRect.right  < Display->res.right)
            ||  (buffer->swapRect.bottom < Display->res.bottom)
            )
        &&  (buffer != buffer->next)
        )
        {
            /* Put target swap rectangle. */
            Display->swapArea = _AllocateArea(Display,
                                              NULL,
                                              &buffer->swapRect,
                                              1U);

            /* Point to earlier buffer. */
            buffer = buffer->next;

            /* Split area with rectangles of earlier buffer(with no-owner).
             * Now owner 0 means we need copy area from front buffer.
             * 1 means target swap rectangle. */
            while (buffer != Display->target)
            {
                _SplitArea(Display, Display->swapArea, &buffer->swapRect, 0U);

                /* Advance to next buffer. */
                buffer = buffer->next;
            }
        }
    }
#endif


    /***************************************************************************
    ** Debug and Dumps.
    */

    if (Context->dumpCompose & DUMP_LAYERS)
    {
        LOGD("LAYERS of dpy=%d:", Display->disp);
        hwcDumpLayer(Context, HwDisplay, Display);
    }

    if (Display->hasG2D && Context->dumpSplitArea)
    {
        LOGD("SPLITED AREA of dpy=%d:", Display->disp);
        hwcDumpArea(Context, Display->compositionArea);

        LOGD("SWAP AREA of dpy=%d:", Display->disp);
        hwcDumpArea(Context, Display->swapArea);
    }

    /***************************************************************************
    ** Do Compose.
    */

    if (Display->hasG2D)
    {
        gceSTATUS status;

        /* Start composition if we have hwc composition. */
        status = hwcComposeG2D(Context, Display);

        if (gcmIS_ERROR(status))
        {
            LOGE("%s(%d): Compose failed", __FUNCTION__, __LINE__);
        }
    }
}


void
_DetectSource(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay,
    IN gctINT Index
    )
{
    hwc_layer_1_t * hwLayer = &HwDisplay->hwLayers[Index];
    hwcLayer *      layer   = &Display->layers[Index];

    gcoSURF surface;
    gceSURF_TYPE type;

    gcsSURF_FORMAT_INFO_PTR info[2];
    gc_native_handle_t * handle;

    gctINT  top;
    gctINT  bottom;
    gctBOOL yflip;

    /* Get source handle. */
    handle = gc_native_handle_get(hwLayer->handle);

    /* Get source surface. */
    surface = (gcoSURF) intptr_t(handle->surface);

    /* Set source color from surface. */
    layer->source = surface;

    /* Get source surface stride. */
    gcmVERIFY_OK(
        gcoSURF_GetAlignedSize(surface,
                               gcvNULL, gcvNULL,
                               (gctINT32 *) layer->strides));

    /* Get source surface format. */
    gcmVERIFY_OK(gcoSURF_GetFormat(surface, &type, &layer->format));

    /* Query format info. */
    gcmVERIFY_OK(gcoSURF_QueryFormat(layer->format, info));

    /* Get bytes per pixel. */
    layer->bytesPerPixel = info[0]->bitsPerPixel / 8;

    /* Check YV12 format. */
    layer->yuv  = (layer->format >= gcvSURF_YUY2)
               && (layer->format <= gcvSURF_NV61);

    /* Get source surface tiling. */
    gcmVERIFY_OK(gcoSURF_GetTiling(surface, &layer->tiling));

    switch (layer->tiling)
    {
    case gcvTILED:
        layer->alignMaskX = 0x03;
        layer->alignMaskY = 0x03;
        break;

    case gcvMULTI_TILED:
        layer->alignMaskX = 0x03;
        layer->alignMaskY = 0x07;
        break;

    case gcvSUPERTILED:
        layer->alignMaskX = 0x3F;
        layer->alignMaskY = 0x3F;
        break;

    case gcvMULTI_SUPERTILED:
        layer->alignMaskX = 0x3F;
        layer->alignMaskY = 0x7F;
        break;

    default:
        if (layer->yuv)
        {
            layer->alignMaskX = (YUV_BUFFER_ALIGN / layer->bytesPerPixel) - 1;
            layer->alignMaskY = 0x01;
        }
        else
        {
            layer->alignMaskX = (RGB_BUFFER_ALIGN / layer->bytesPerPixel) - 1;
            layer->alignMaskY = 0x00;
        }
    }

    /* Get source surface size. */
    gcmVERIFY_OK(
        gcoSURF_GetSize(surface, &layer->width, &layer->height, gcvNULL));

    /* Get shortcuts. */
    gcsRECT_PTR srcRect    = &layer->srcRect;
    gcsRECT_PTR dstRect    = &layer->dstRect;
    gcsRECT_PTR sourceCrop;

#if ANDROID_SDK_VERSION >= 19
    gcsRECT sourceCropi;

    if (Context->device.common.version >= HWC_DEVICE_API_VERSION_1_3)
    {
        sourceCropi.left   = gctINT(ceilf(hwLayer->sourceCropf.left));
        sourceCropi.top    = gctINT(ceilf(hwLayer->sourceCropf.top));
        sourceCropi.right  = gctINT(floorf(hwLayer->sourceCropf.right));
        sourceCropi.bottom = gctINT(floorf(hwLayer->sourceCropf.bottom));
        sourceCrop = &sourceCropi;
    }
    else
#endif
    {
        sourceCrop = (gcsRECT *) &hwLayer->sourceCrop;
    }

    /* Get source surface orientation. */
    gcmVERIFY_OK(gcoSURF_QueryOrientation(surface, &layer->orientation));

    /* Support bottom-top orientation. */
    if (layer->orientation == gcvORIENTATION_TOP_BOTTOM)
    {
        top    = sourceCrop->top;
        bottom = sourceCrop->bottom;
        yflip  = gcvFALSE;
    }

    else
    {
        top    = layer->height - sourceCrop->bottom;
        bottom = layer->height - sourceCrop->top;
        yflip  = gcvTRUE;
    }

    /* Get mirror/rotation states and srcRect.
     * Android is given src rectangle in original coord system
     * before trasformation. But G2D needs src rectangle given in
     * transformed coord system (source coord system). G2D also
     * needs dest rectangle given in transformed coord system.
     * But we never make dest rotated. Dest rectangle given by
     * android is directly suitable for G2D (dest coord system).
     * Between source coord system and dest coord system, there's
     * only stretching or traslating.
     * Here, it is to calculation src rectangle from original coord
     * system to source coord system. */
    switch (hwLayer->transform)
    {
    case 0:
    default:
        layer->hMirror  = gcvFALSE;
        layer->vMirror  = yflip;
        layer->rotation = gcvSURF_0_DEGREE;
        break;

    case HWC_TRANSFORM_FLIP_H:
        layer->hMirror  = gcvTRUE;
        layer->vMirror  = yflip;
        layer->rotation = gcvSURF_0_DEGREE;
        break;

    case HWC_TRANSFORM_FLIP_V:
        layer->hMirror  = gcvFALSE;
        layer->vMirror  = !yflip;
        layer->rotation = gcvSURF_0_DEGREE;
        break;

    case HWC_TRANSFORM_ROT_90:
        if (yflip)
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvTRUE;
            layer->rotation = gcvSURF_90_DEGREE;
        }
        else
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvFALSE;
            layer->rotation = gcvSURF_270_DEGREE;
        }
        break;

    case HWC_TRANSFORM_ROT_180:
        layer->hMirror  = gcvFALSE;
        layer->vMirror  = yflip;
        layer->rotation = gcvSURF_180_DEGREE;
        break;

    case HWC_TRANSFORM_ROT_270:
        layer->hMirror  = yflip;
        layer->vMirror  = gcvFALSE;
        layer->rotation = gcvSURF_90_DEGREE;
        break;

    case (HWC_TRANSFORM_ROT_90 | HWC_TRANSFORM_FLIP_H):
        if (yflip)
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvFALSE;
            layer->rotation = gcvSURF_90_DEGREE;
        }
        else
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvTRUE;
            layer->rotation = gcvSURF_270_DEGREE;
        }
        break;

    case (HWC_TRANSFORM_ROT_90 | HWC_TRANSFORM_FLIP_V):
        if (yflip)
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvFALSE;
            layer->rotation = gcvSURF_270_DEGREE;
        }
        else
        {
            layer->hMirror  = gcvFALSE;
            layer->vMirror  = gcvTRUE;
            layer->rotation = gcvSURF_90_DEGREE;
        }
        break;
    }

    switch (layer->rotation)
    {
    case gcvSURF_0_DEGREE:
        srcRect->left   = sourceCrop->left;
        srcRect->top    = top;
        srcRect->right  = sourceCrop->right;
        srcRect->bottom = bottom;
        break;

    case gcvSURF_90_DEGREE:
        srcRect->left   = top;
        srcRect->top    = layer->width  - sourceCrop->right;
        srcRect->right  = bottom;
        srcRect->bottom = layer->width  - sourceCrop->left;
        break;

    case gcvSURF_180_DEGREE:
        srcRect->left   = layer->width  - sourceCrop->right;
        srcRect->top    = layer->height - bottom;
        srcRect->right  = layer->width  - sourceCrop->left;
        srcRect->bottom = layer->height - top;
        break;

    case gcvSURF_270_DEGREE:
        srcRect->left   = layer->height - bottom;
        srcRect->top    = sourceCrop->left,
        srcRect->right  = layer->height - top;
        srcRect->bottom = sourceCrop->right;
        break;

    default:
        break;
    }

    /* Save original rectangles. */
    layer->orgRect.left   = sourceCrop->left;
    layer->orgRect.top    = top;
    layer->orgRect.right  = sourceCrop->right;
    layer->orgRect.bottom = bottom;

    layer->orgDest = *dstRect;

    /* Compute stretch factor. */
    gctFLOAT hFactor = gctFLOAT(srcRect->right  - srcRect->left)
                     / (dstRect->right  - dstRect->left);

    gctFLOAT vFactor = gctFLOAT(srcRect->bottom - srcRect->top)
                     / (dstRect->bottom - dstRect->top);

    gctFLOAT dl = 0, dt = 0, dr = 0, db = 0;
    gcsRECT_PTR res = &Display->res;

    /* Clip dest into display area. */
    if (dstRect->left < 0)
    {
        dl = dstRect->left * hFactor;
        dstRect->left = 0;
    }

    if (dstRect->top < 0)
    {
        dt = dstRect->top * vFactor;
        dstRect->top = 0;
    }

    if (dstRect->right > res->right)
    {
        dr = (dstRect->right - res->right) * hFactor;
        dstRect->right = res->right;
    }

    if (dstRect->bottom > res->bottom)
    {
        db = (dstRect->bottom - res->bottom) * vFactor;
        dstRect->bottom = res->bottom;
    }

    if (layer->hMirror)
    {
        gctFLOAT tl = dl;

        dl = -dr;
        dr = -tl;
    }

    if (layer->vMirror)
    {
        gctFLOAT tt = dt;

        dt = -db;
        db = -tt;
    }

    /* Compute corresponding clipped source rectangle. */
    srcRect->left   = gctINT(srcRect->left   - dl + 0.5f);
    srcRect->top    = gctINT(srcRect->top    - dt + 0.5f);
    srcRect->right  = gctINT(srcRect->right  - dr + 0.5f);
    srcRect->bottom = gctINT(srcRect->bottom - db + 0.5f);

    /* Recompute rectangle size. */
    gctINT srcWidth  = srcRect->right  - srcRect->left;
    gctINT srcHeight = srcRect->bottom - srcRect->top;
    gctINT dstWidth  = dstRect->right  - dstRect->left;
    gctINT dstHeight = dstRect->bottom - dstRect->top;

    /* Determine blit type: bit, stretch or filter? */
    if ((hFactor != 1.0f) || (vFactor != 1.0f))
    {
        /* Yes, stretch. */
        layer->stretch = gcvTRUE;

        /* Need filter blit if filterStretch. */
        layer->filter  = Context->filterStretch
                      || (layer->yuv && (Context->opf == gcvFALSE));

        /* Record scale factors. */
        layer->xScale = 1.0f / hFactor;
        layer->yScale = 1.0f / vFactor;
    }

    else
    {
        /* No stretch. */
        layer->stretch = gcvFALSE;

        /* Need filter blit if
         * 1. YUV
         * 2. No opf (includes YUV blit) feature
         */
        layer->filter  = layer->yuv && (Context->opf == gcvFALSE);
    }

    if (layer->filter)
    {
        /* Compute filter blit parameters. */
        /* Determine kernel size for filter blit. */
        if (Context->opf)
        {
            /* Shrink - Scale 2: Kernel size = 3
             * Scale >2: Kernel size = 5.
             * Notice that, kernel size 3 will have better performance for
             * one pass filter bit. And kernel size 1 is not supported by opf.
             */
            layer->hKernel = (hFactor >= 1.0f / 2) ? 3U : 5U;
            layer->vKernel = (vFactor >= 1.0f / 2) ? 3U : 5U;
        }

        else
        {
            /* No Scale: Kernel size = 1
             * Shrink - Scale 2: Kernel size = 3
             * Scale >2: Kernel size = 9.
             * For shrink case, big kernel size will cause incorrect output.
             * So kernel size 3 is the best choice. If not scaled, kernel size
             * may have better performance because one of two passes may be
             * skipped. Otherwise, kernel size 9 is simply used because any
             * kernel will have the same performance. */
            layer->hKernel = (hFactor == 1.0f / 1) ? 1U
                           : (hFactor >= 1.0f / 2) ? 3U : 9U;

            layer->vKernel = (vFactor == 1.0f / 1) ? 1U
                           : (vFactor >= 1.0f / 2) ? 3U : 9U;
        }
    }

    else if (layer->stretch)
    {
        /* Compute stretch blit parameters. */
        /* Determine stretch factors. */
        gcmVERIFY_OK(
            gco2D_CalcStretchFactor(Context->engine,
                                    srcWidth,
                                    dstWidth,
                                    &layer->hFactor));

        gcmVERIFY_OK(
            gco2D_CalcStretchFactor(Context->engine,
                                    srcHeight,
                                    dstHeight,
                                    &layer->vFactor));

        /* Compute greatest common divisor, X coordinate. */
        gctINT gcd = srcWidth;
        gctINT b   = dstWidth;

        while (b != 0)
        {
            gctINT r = b;
            b    = gcd % b;
            gcd  = r;
        }

        /* Compute X steps. */
        layer->srcXStep = srcWidth / gcd;
        layer->dstXStep = dstWidth / gcd;

        /* Compute greatest common divisor, X coordinate. */
        gcd = srcHeight;
        b   = dstHeight;

        while (b != 0)
        {
            gctINT r = b;
            b    = gcd % b;
            gcd  = r;
        }

        /* Compute Y steps. */
        layer->srcYStep = srcHeight / gcd;
        layer->dstYStep = dstHeight / gcd;
    }

    if (Display->target->tsConfig == gcv2D_TSC_2D_COMPRESSED)
    {
        if (layer->filter || layer->stretch)
        {
            /*
             * Filter/stretch blit does not support 2D compression.
             * Disable 2D compression if any layer needs filter/stretch blit.
             */
            Display->target->tsConfig = gcv2D_TSC_DISABLE;
        }
    }
}


static gctBOOL
_HasAlpha(
    IN gceSURF_FORMAT Format
    )
{
    return (Format == gcvSURF_R5G6B5) ? gcvFALSE
         : (
               (Format == gcvSURF_A8R8G8B8)
            || (Format == gcvSURF_A8B8G8R8)
            || (Format == gcvSURF_B8G8R8A8)
            || (Format == gcvSURF_R8G8B8A8)
           );
}


/*
 * Area spliting feature depends on the following 3 functions:
 * '_AllocateArea', '_FreeArea' and '_SplitArea'.
 */
#define POOL_SIZE 512

hwcArea *
_AllocateArea(
    IN hwcDisplay * Display,
    IN hwcArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    hwcArea * area;
    hwcAreaPool * pool  = Display->areaPool;

    if (pool == NULL)
    {
        /* First pool. */
        pool = (hwcAreaPool *) malloc(sizeof (hwcAreaPool));
        Display->areaPool = pool;

        /* Clear fields. */
        pool->areas     = NULL;
        pool->freeNodes = NULL;
        pool->next      = NULL;
    }

    for (;;)
    {
        if (pool->areas == NULL)
        {
            /* No areas allocated, allocate now. */
            pool->areas = (hwcArea *) malloc(sizeof (hwcArea) * POOL_SIZE);

            /* Get area. */
            area = pool->areas;

            /* Update freeNodes. */
            pool->freeNodes = area + 1;

            break;
        }

        else if (pool->freeNodes - pool->areas >= POOL_SIZE)
        {
            /* This pool is full. */
            if (pool->next == NULL)
            {
                /* No more pools, allocate one. */
                pool->next = (hwcAreaPool *) malloc(sizeof (hwcAreaPool));

                /* Point to the new pool. */
                pool = pool->next;

                /* Clear fields. */
                pool->areas     = NULL;
                pool->freeNodes = NULL;
                pool->next      = NULL;
            }

            else
            {
                /* Advance to next pool. */
                pool = pool->next;
            }
        }

        else
        {
            /* Get area and update freeNodes. */
            area = pool->freeNodes++;

            break;
        }
    }

    /* Update area fields. */
    area->rect   = *Rect;
    area->owners = Owner;

    if (Slibing == NULL)
    {
        area->next = NULL;
    }

    else
    {
        area->next = Slibing->next;
        Slibing->next = area;
    }

    return area;
}


void
_FreeArea(
    IN hwcDisplay * Display,
    IN hwcArea* Head
    )
{
    /* Free the first node is enough. */
    hwcAreaPool * pool  = Display->areaPool;

    while (pool != NULL)
    {
        if (Head >= pool->areas && Head < pool->areas + POOL_SIZE)
        {
            /* Belongs to this pool. */
            if (Head < pool->freeNodes)
            {
                /* Update freeNodes if the 'Head' is older. */
                pool->freeNodes = Head;

                /* Reset all later pools. */
                while (pool->next != NULL)
                {
                    /* Advance to next pool. */
                    pool = pool->next;

                    /* Reset freeNodes. */
                    pool->freeNodes = pool->areas;
                }
            }

            /* Done. */
            break;
        }

        else if (pool->freeNodes < pool->areas + POOL_SIZE)
        {
            /* Already tagged as freed. */
            break;
        }

        else
        {
            /* Advance to next pool. */
            pool = pool->next;
        }
    }
}


void
_SplitArea(
    IN hwcDisplay * Display,
    IN hwcArea * Area,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    gcsRECT r0[4];
    gcsRECT r1[4];
    gctUINT32 c0 = 0;
    gctUINT32 c1 = 0;

    gcsRECT * rect;

    for (;;)
    {
        rect = &Area->rect;

        if ((Rect->left   < rect->right)
        &&  (Rect->top    < rect->bottom)
        &&  (Rect->right  > rect->left)
        &&  (Rect->bottom > rect->top)
        )
        {
            /* Overlapped. */
            break;
        }

        if (Area->next == NULL)
        {
            /* This rectangle is not overlapped with any area. */
            _AllocateArea(Display, Area, Rect, Owner);
            return;
        }

        Area = Area->next;
    }

    /* OK, the rectangle is overlapped with 'rect' area. */
    if ((Rect->left <= rect->left)
    &&  (Rect->right >= rect->right)
    )
    {
        /* |-><-| */
        /* +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->left <= rect->left)
    {
        /* |-> */
        /* +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->right >= rect->right)
    {
        /*    <-| */
        /* +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else
    {
        /* | */
        /* +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    if (c1 > 0)
    {
        /* Process rects outside area. */
        if (Area->next == NULL)
        {
            /* Save rects outside area. */
            for (gctUINT32 i = 0; i < c1; i++)
            {
                _AllocateArea(Display, Area, &r1[i], Owner);
            }
        }

        else
        {
            /* Rects outside area. */
            for (gctUINT32 i = 0; i < c1; i++)
            {
                _SplitArea(Display, Area, &r1[i], Owner);
            }
        }
    }

    if (c0 > 0)
    {
        /* Save rects inside area but not overlapped. */
        for (gctUINT32 i = 0; i < c0; i++)
        {
            _AllocateArea(Display, Area, &r0[i], Area->owners);
        }

        /* Update overlapped area. */
        if (rect->left   < Rect->left)   { rect->left   = Rect->left;   }
        if (rect->top    < Rect->top)    { rect->top    = Rect->top;    }
        if (rect->right  > Rect->right)  { rect->right  = Rect->right;  }
        if (rect->bottom > Rect->bottom) { rect->bottom = Rect->bottom; }
    }

    /* The area is owned by the new owner as well. */
    Area->owners |= Owner;
}


/*
 * (*set)() is used in place of eglSwapBuffers(), and assumes the same
 * functionality, except it also commits the work list atomically with
 * the actual eglSwapBuffers().
 *
 * The layer lists are guaranteed to be the same as the ones returned from
 * the last call to (*prepare)().
 *
 * When this call returns the caller assumes that the displays will be
 * updated in the near future with the content of their work lists, without
 * artifacts during the transition from the previous frame.
 *
 * A display with zero layers indicates that the entire composition has
 * been handled by SurfaceFlinger with OpenGL ES. In this case, (*set)()
 * behaves just like eglSwapBuffers().
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
 * IMPORTANT NOTE: There is an implicit layer containing opaque black
 * pixels behind all the layers in the list. It is the responsibility of
 * the hwcomposer module to make sure black pixels are output (or blended
 * from).
 *
 * IMPORTANT NOTE: In the event of an error this call *MUST* still cause
 * any fences returned in the previous call to set to eventually become
 * signaled.  The caller may have already issued wait commands on these
 * fences, and having set return without causing those fences to signal
 * will likely result in a deadlock.
 *
 * returns: 0 on success. A negative error code on error:
 *    HWC_EGL_ERROR: eglGetError() will provide the proper error code (only
 *        allowed prior to HWComposer 1.1)
 *    Another code for non EGL errors.
 */
int
hwc_set(
    hwc_composer_device_1_t * dev,
    size_t NumDisplays,
    hwc_display_contents_1_t** HwDisplays
    )
{
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;
    hwcContext * context = (hwcContext *) dev;
    gctBOOL needCommit = gcvFALSE;

    /* Check device handle. */
    if (context == gcvNULL)
    {
        LOGE("%s(%d): Invalid device!", __FUNCTION__, __LINE__);
        return HWC_EGL_ERROR;
    }

    /***************************************************************************
    ** Update debug dump options
    */
    {
        char property[PROPERTY_VALUE_MAX] = "0";

        property_get("hwc.debug.dump_compose", property, "0");
        gctUINT32 value = property[0] - '0';
        context->dumpCompose = (value == 0x01) ? 0xFF : value;

        property_get("hwc.debug.dump_bitmap", property, "0");
        context->dumpBitmap = property[0] - '0';

        property_get("hwc.debug.disable_compression", property, "0");
        context->disableCompression = property[0] - '0';

        /* property_get("hwc.debug.dump_split_area", property, defaultValue); */
        /* context->dumpSplitArea = property[0] - '0'; */
    }

    /***************************************************************************
    ** Check for 2D Composition
    */

    if (context->separated2D)
    {
        /* Query current hardawre type. */
        gcoHAL_GetHardwareType(context->hal, &currentType);
    }

    for (size_t i = 0; i < NumDisplays; i++)
    {
        hwcDisplay * dpy;
        hwc_display_contents_1_t * hwDisplay;

        ANativeWindowBuffer * backBuffer = NULL;

        hwc_layer_1_t * displayTarget;
        private_handle_t * handle;

        private_handle_t * target = NULL;
        gcsRECT_PTR swapRect = NULL;

        dpy       = context->displays[i];
        hwDisplay = HwDisplays[i];

        if ((dpy == NULL)
        ||  !dpy->device.connected
        ||  !dpy->device.actived
        ||  (hwDisplay == NULL)
        )
        {
            /* Bail out for no display. */
            continue;
        }

        /* Get framebuffer target. */
        displayTarget = &hwDisplay->hwLayers[hwDisplay->numHwLayers - 1];
        handle = (private_handle_t *) displayTarget->handle;

        /***********************************************************************
        ** Get current window back buffer for 2D composition.
        */

        if (dpy->hasG2D)
        {
            if (i < context->numPhysicalDisplays)
            {
                /* Get back buffer for physical display. */
                backBuffer = (ANativeWindowBuffer *) _eglGetRenderBufferVIV(handle);
            }

            if (context->separated2D)
            {
                /* Set hardware type to 2D. */
                gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
            }

            /* Need commit-stall after 2D operations. */
            needCommit = gcvTRUE;
        }

        /***********************************************************************
        ** Wait upon producer (application) for HWC composition.
        */

        {
            /* Wait on producer, ie, application. */
            for (gctUINT i = 0; i < dpy->layerCount; i++)
            {
                /* Get shortcut. */
                hwcLayer * layer = &dpy->layers[i];

                if (layer->compositionType == HWC_FRAMEBUFFER)
                {
                    /* Skip wait on 3D layer. */
                    continue;
                }

                hwc_layer_1_t * hwLayer = &hwDisplay->hwLayers[i];
                int fenceFd = hwLayer->acquireFenceFd;

                if (fenceFd != -1)
                {
                    sync_wait(fenceFd, -1);
                    close(fenceFd);
                    hwLayer->acquireFenceFd = -1;
                }
            }
        }

        /***********************************************************************
        ** Set window back buffer / framebuffer target.
        */

        /* Set for this display. */
        if (backBuffer != NULL)
        {
#if ENABLE_SWAP_RECTANGLE
            gcsRECT rect;

            /* Get swap rectangle from android_native_buffer_t. */
            gctUINT32 origin = (gctUINT32) backBuffer->common.reserved[0];
            gctUINT32 size   = (gctUINT32) backBuffer->common.reserved[1];

            /* Update swap rectangle. */
            if (size == 0)
            {
                /* This means full screen swap rectangle. */
                swapRect = &dpy->res;
            }

            else
            {
                rect.left   = (origin >> 16);
                rect.top    = (origin &  0xFFFF);
                rect.right  = (origin >> 16) + (size >> 16);
                rect.bottom = (origin &  0xFFFF) + (size & 0xFFFF);

                swapRect = &rect;
            }
#endif

            target = (private_handle_t *) backBuffer->handle;
        }

        else if (hwDisplay->outbuf)
        {
            /* Virtual display. */
            int fenceFd = hwDisplay->outbufAcquireFenceFd;

            /* Wait upon acquire fence fd. */
            if (fenceFd != -1)
            {
                sync_wait(fenceFd, -1);
                close(fenceFd);
                hwDisplay->outbufAcquireFenceFd = -1;
            }

            swapRect = NULL;
            target   = (private_handle_t *) hwDisplay->outbuf;
        }

        else
        {
            /* No 2D composition. */
            swapRect = NULL;
            target   = (private_handle_t *) handle;
        }

        /* Do set. */
        _Set(context, target, swapRect, dpy, hwDisplay);

        /***********************************************************************
        ** Unlock layer sources.
        */

        if (dpy->hasG2D)
        {
            /* Cast to GC handle. */
            gc_native_handle_t * handle = gc_native_handle_get(target);

            /* Get surface wrapper. */
            gcoSURF surface = (gcoSURF) intptr_t(handle->surface);

            /* Unlock the target. */
            gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));

            for (gctUINT i = 0; i < dpy->layerCount; i++)
            {
                /* Get shortcut. */
                hwcLayer * layer = &dpy->layers[i];

                if (layer->source != gcvNULL)
                {
                    /* Unlock the surface. */
                    gcmVERIFY_OK(gcoSURF_Unlock(layer->source, gcvNULL));
                }
            }

            /* Commit accumulated commands. */
            gcoHAL_Commit(context->hal, gcvFALSE);
        }

#if gcdANDROID_NATIVE_FENCE_SYNC >= 3

        /***********************************************************************
        ** Create 2D blitter fenceFd.
        */

        do
        {
            int fenceFd = -1;
            gctSYNC_POINT syncPoint;
            gcsHAL_INTERFACE iface;
            gceSTATUS status;

            if (!dpy->hasG2D)
            {
                /* No layers to set releaseFenceFd. */
                break;
            }

            /* Create sync point. */
            status = gcoOS_CreateSyncPoint(gcvNULL, &syncPoint);

            if (gcmIS_ERROR(status))
            {
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
                break;
            }

            /* Create native fence. */
            status = gcoOS_CreateNativeFence(gcvNULL, syncPoint, &fenceFd);

            if (gcmIS_ERROR(status) || fenceFd < 0)
            {
                gcmVERIFY_OK(gcoOS_DestroySyncPoint(gcvNULL, syncPoint));
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
                break;
            }

            /* Submit the sync point. */
            iface.command               = gcvHAL_SYNC_POINT;
            iface.u.SyncPoint.command   = gcvSYNC_POINT_SIGNAL;
            iface.u.SyncPoint.syncPoint = gcmPTR_TO_UINT64(syncPoint);
            iface.u.Signal.fromWhere    = gcvKERNEL_PIXEL;

            /* Send event. */
            gcoHAL_ScheduleEvent(gcvNULL, &iface);

            /* Now destroy the sync point. */
            gcmVERIFY_OK(gcoOS_DestroySyncPoint(gcvNULL, syncPoint));

            for (gctUINT i = 0; i < dpy->layerCount; i++)
            {
                /* Get shortcut. */
                hwcLayer * layer = &dpy->layers[i];

                /* G2D only touches BLITTER layer. */
                if (layer->compositionType == HWC_BLITTER)
                {
                    /* Set release fence fd. */
                    hwDisplay->hwLayers[i].releaseFenceFd = dup(fenceFd);
                }
            }

            /* Record 2D blitter fenceFd. */
            dpy->fenceFd2D = fenceFd;

            /* Commit accumulated commands. */
            gcoHAL_Commit(context->hal, gcvFALSE);
        }
        while (0);
#endif

        /***********************************************************************
        ** Post window back buffer for 2D composition.
        */

        if (backBuffer)
        {
            /* Post framebuffer target. */
            _eglPostBufferVIV(backBuffer);
        }
    }

#if gcdANDROID_NATIVE_FENCE_SYNC < 3
    /* Check if a commit-stall is needed. */
    if (needCommit)
    {
        /* Wait all 2D operations done. */
        gcoHAL_Commit(context->hal, gcvTRUE);
    }
#endif

    if (context->separated2D)
    {
        /* Restore hardware type. */
        gcoHAL_SetHardwareType(context->hal, currentType);
    }


    /***************************************************************************
    ** Do Display Composition.
    */

    for (size_t i = 0; i < NumDisplays; i++)
    {
        /* Do Display Control. */
        hwcDisplayFrame(context, context->displays[i], HwDisplays[i]);
    }

    if (context->dumpCompose & DUMP_AVERAGE_FPS)
    {
        static struct timeval start;
        static int frames;

        if (++frames >= 10)
        {
            struct timeval curr;
            gettimeofday(&curr, NULL);

            long  expired = (curr.tv_sec  - start.tv_sec)  * 1000
                          + (curr.tv_usec - start.tv_usec) / 1000;

            float fps = frames * 1000.0f / expired;

            /* Skip unreasonable numbers. */
            if (fps >= 2.0f)
            {
                LOGD("Average FPS: %.1f", fps);
            }

            frames = 0;
            start  = curr;
        }
    }

    if (context->dumpBitmap)
    {
        for (size_t i = 0; i < NumDisplays; i++)
        {
            hwcDisplay * dpy;
            hwc_display_contents_1_t * hwDisplay;

            dpy       = context->displays[i];
            hwDisplay = HwDisplays[i];

            if ((dpy == NULL)
            ||  !dpy->device.connected
            ||  !dpy->device.actived
            ||  (hwDisplay == NULL)
            ||  (hwDisplay->numHwLayers <= 1)
            )
            {
                /* Bail out for no display. */
                continue;
            }

            hwcDumpBitmap(context, hwDisplay, dpy);
        }
    }

    return 0;
}

