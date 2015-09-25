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


#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "gc_hwc.h"
#include "gc_hwc_debug.h"

#include <gc_hal.h>
#include <gc_hal_driver.h>
#include <gc_gralloc_priv.h>

#include <linux/fb.h>

#include <stdlib.h>
#include <errno.h>

static void
_DetectSource(
    IN hwcContext * Context,
    IN hwc_layer_list_t * List,
    IN gctINT Index
    );

static hwcArea *
_AllocateArea(
    IN hwcContext * Context,
    IN hwcArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    );

static void
_FreeArea(
    IN hwcContext * Context,
    IN hwcArea *    Head
    );

static void
_SplitArea(
    IN hwcContext * Context,
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
**  hwcSet
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
**      android_native_buffer_t * BackBuffer
**          The display back buffer to render to.
**
**      hwc_layer_list_t * List
**          All layers to be composed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
hwcSet(
    IN hwcContext * Context,
    IN android_native_buffer_t * BackBuffer,
    IN hwc_layer_list_t * List
    )
{
    gceSTATUS status = gcvSTATUS_OK;


    /***************************************************************************
    ** Framebuffer Detection.
    */

    if (Context->hasG2D)
    {
        /* Get GC handle. */
        gc_native_handle_t * handle = gc_native_handle_get(BackBuffer->handle);

        /* Get shortcuts. */
        hwcFramebuffer * framebuffer = Context->framebuffer;

        /* Get surface wrapper. */
        gcoSURF surface = (gcoSURF) intptr_t(handle->surface);

        hwcBuffer * target;
        gcsSURF_FORMAT_INFO_PTR info[2];
        gctUINT physical[3];

        /* Lock for GPU address. */
        gcmVERIFY_OK(gcoSURF_Lock(surface, physical, gcvNULL));

        /* Detect framebuffer information. */
        if (framebuffer == NULL)
        {
            /* Framebuffer resolution. */
            gctUINT32 xres;
            gctUINT32 yres;

            /* Allocate memory for framebuffer struct. */
            framebuffer = (hwcFramebuffer *) malloc( sizeof (hwcFramebuffer));

            /* Get framebuffer size. */
            gcmVERIFY_OK(
                gcoSURF_GetSize(surface,
                                (gctUINT32 *) &xres,
                                (gctUINT32 *) &yres,
                                gcvNULL));

            /* Save framebuffer resolution. */
            framebuffer->res.left   = 0;
            framebuffer->res.top    = 0;
            framebuffer->res.right  = xres;
            framebuffer->res.bottom = yres;

            /* Get stride. */
            gcmVERIFY_OK(
                gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL,
                                       (gctINT_PTR) &framebuffer->stride));

            /* TODO: Get tiling. */
            framebuffer->tiling = gcvLINEAR;

            /* Get format. */
            gcmVERIFY_OK(
                gcoSURF_GetFormat(surface, gcvNULL, &framebuffer->format));

            /* Query format info. */
            gcmVERIFY_OK(gcoSURF_QueryFormat(framebuffer->format, info));

            /* Get bytes per pixel. */
            framebuffer->bytesPerPixel = info[0]->bitsPerPixel / 8;

            /* Get multi-source blit alignment mask. */
            if ((framebuffer->format >= gcvSURF_YUY2)
            &&  (framebuffer->format <= gcvSURF_NV16)
            )
            {
                /* YUV framebuffer? */
                framebuffer->alignMask =
                    YUV_BUFFER_ALIGN / framebuffer->bytesPerPixel - 1;
            }
            else
            {
                /* RGB framebuffer. */
                framebuffer->alignMask =
                    RGB_BUFFER_ALIGN / framebuffer->bytesPerPixel - 1;
            }

            /* Clear valid flag. */
            framebuffer->valid = gcvFALSE;

            /* Allocate target framebuffer buffer. */
            target = (hwcBuffer *) malloc(sizeof (hwcBuffer));

            /* Save handle. */
            target->handle = BackBuffer->handle;

            /* Save physical address. */
            target->physical = physical[0];

            /* Point prev/next buffer to self. */
            target->prev = target->next = target;

            /* Save target handle to framebuffer. */
            framebuffer->head = framebuffer->target = target;

            /* Save framebuffer handle to context. */
            Context->framebuffer = framebuffer;

            /* Logs. */
            LOGI("Framebuffer(%p) detected: format=%d %dx%d",
                 framebuffer, framebuffer->format, xres, yres);

            LOGI("Framebuffer buffer(%p) detected: physical=0x%08x",
                 target, target->physical);
        }

        /* Get target framebuffer buffer. */
        target = framebuffer->target = framebuffer->target->next;

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
            while (target != framebuffer->target);

            /* Check physical address. */
            if (target->physical != physical[0])
            {
                /* Allocate new buffer when not found. */
                target = (hwcBuffer *) malloc(sizeof (hwcBuffer));

                /* Save buffer handle. */
                target->handle = BackBuffer->handle;

                /* Get buffer address. */
                target->physical = physical[0];

                /* Insert target before framebuffer->target. */
                hwcBuffer * t = framebuffer->target;

                t->prev->next = target;
                target->prev  = t->prev;

                target->next = t;
                t->prev      = target;

                /* Logs. */
                LOGI("Framebuffer buffer(%p) detected: physical=0x%08x",
                     target, target->physical);
            }

            else if (framebuffer->valid)
            {
                /* Adust buffer sequence:
                 * isolate target and put it before framebuffer->target. */
                target->prev->next = target->next;
                target->next->prev = target->prev;

                /* Insert target before framebuffer->target. */
                hwcBuffer * t = framebuffer->target;

                t->prev->next = target;
                target->prev  = t->prev;

                target->next = t;
                t->prev      = target;
            }

            /* Update target buffer. */
            framebuffer->target  = target;
        }

#if ENABLE_SWAP_RECTANGLE

        /* Get swap rectangle from android_native_buffer_t. */
        gctUINT32 origin = (gctUINT32) BackBuffer->common.reserved[0];
        gctUINT32 size   = (gctUINT32) BackBuffer->common.reserved[1];

        /* Update swap rectangle. */
        if (size == 0)
        {
            /* This means full screen swap rectangle. */
            target->swapRect = framebuffer->res;
        }

        else
        {
            target->swapRect.left   = (origin >> 16);
            target->swapRect.top    = (origin &  0xFFFF);
            target->swapRect.right  = (origin >> 16) + (size >> 16);
            target->swapRect.bottom = (origin &  0xFFFF) + (size & 0xFFFF);
        }
#else

        /* Always set to full screen swap rectangle. */
        target->swapRect = framebuffer->res;
#endif

        /* Update valid flag. */
        if (framebuffer->valid == gcvFALSE)
        {
            /* Reset swap rectangles of other buffers. */
            hwcBuffer * buffer = target->next;

            while (buffer != target)
            {
                /* Reset size to full screen. */
                buffer->swapRect = framebuffer->res;

                buffer = buffer->next;
            }

            /* Set valid falg. */
            framebuffer->valid = gcvTRUE;
        }
    }

    else if (Context->framebuffer != NULL)
    {
        /* 3D composition path, remove the valid flag. */
        Context->framebuffer->valid = gcvFALSE;
    }


    /***************************************************************************
    ** Update Layer Information and Geometry.
    */

    if (Context->geometryChanged && Context->hasG2D)
    {
        /* Geometry changed and has composition. */
        hwcFramebuffer * framebuffer = Context->framebuffer;

        /* Update layer count. */
        Context->layerCount = List->numHwLayers;

        /* Generate new layers. */
        for (gctUINT32 i = 0; i < Context->layerCount; i++)
        {
            /* Get shortcuts. */
            hwc_layer_t *  hwLayer = &List->hwLayers[i];
            hwcLayer *     layer   = &Context->layers[i];
            hwc_region_t * region  = &hwLayer->visibleRegionScreen;

            gctBOOL perpixelAlpha;
            gctUINT32 planeAlpha;

            /* Pass composition type. */
            layer->compositionType = hwLayer->compositionType;

            /* Dest rectangle. */
            layer->dstRect         = *(gcsRECT *) &hwLayer->displayFrame;

            /* Dest clips. */
            layer->clipRects       = (gcsRECT_PTR) region->rects;
            layer->clipCount       = region->numRects;

            switch (hwLayer->compositionType)
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
                _DetectSource(Context, List, i);

                /* Get alpha blending/premultiply states. */
                switch (hwLayer->blending & 0xFFFF)
                {
                case HWC_BLENDING_PREMULT:

                    /* Get perpixelAlpha enabling state. */
                    perpixelAlpha = _HasAlpha(layer->format);

                    /* Get plane alpha value. */
#if ENABLE_PLANE_ALPHA
                    planeAlpha    = hwLayer->blending >> 16;
#else
                    planeAlpha    = 0xFF;
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
#if ENABLE_PLANE_ALPHA
                    planeAlpha    = hwLayer->blending >> 16;
#else
                    planeAlpha    = 0xFF;
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

            case HWC_DIM:

                /* Set no source surface. Color should be from solid color32. */
                layer->source  = gcvNULL;

                /* Get alpha channel. */
                layer->color32 = (hwLayer->blending & 0xFF0000) << 8;

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
            }
        }

        /* Reset allocated areas. */
        if (Context->compositionArea != NULL)
        {
            _FreeArea(Context, Context->compositionArea);

            Context->compositionArea = NULL;
#if ENABLE_SWAP_RECTANGLE
            Context->swapArea        = NULL;
#endif
        }

        /* Generate new areas. */
        /* Put a no-owner area with screen size, this is for worm hole,
         * and is needed for clipping. */
        Context->compositionArea = _AllocateArea(Context,
                                                 NULL,
                                                 &framebuffer->res,
                                                 0U);

        /* Split areas: go through all regions. */
        for (gctUINT32 i = 0; i < List->numHwLayers; i++)
        {
            gctUINT32 owner = 1U << i;
            hwc_layer_t *  hwLayer = &List->hwLayers[i];
            hwc_region_t * region  = &hwLayer->visibleRegionScreen;

            /* Now go through all rectangles to split areas. */
            for (gctUINT32 j = 0; j < region->numRects; j++)
            {
                gcsRECT rect = {
                    region->rects[j].left,
                    region->rects[j].top,
                    region->rects[j].right,
                    region->rects[j].bottom,
                };

                if ((rect.left >= framebuffer->res.right)
                ||  (rect.top  >= framebuffer->res.bottom)
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
                if (rect.right  > framebuffer->res.right)  { rect.right  = framebuffer->res.right;  }
                if (rect.bottom > framebuffer->res.bottom) { rect.bottom = framebuffer->res.bottom; }

                _SplitArea(Context,
                           Context->compositionArea,
                           &rect,
                           owner);
            }
        }
    }

#if ENABLE_CLEAR_HOLE

    /***************************************************************************
    ** 'CLEAR_HOLE' Corretion.
    */

    if (Context->hasG2D && Context->geometryChanged
    &&  Context->hasClearHole
    )
    {
        /* If some layer has compositionType 'CLEAR_HOLE', but it has one or
         * more layers under it, skip 'CLEAR_HOLE' operation.
         * See 'no texture' section in drawWithOpenGL in LayerBase.cpp. */
        for (gctUINT32 i = 0; i < Context->layerCount; i++)
        {
            if (Context->layers[i].compositionType != HWC_CLEAR_HOLE)
            {
                /* Likely, not clear hole. */
                continue;
            }

            /* Find a layer with clear hole. */
            gctUINT32 owner = (1U << i);

            hwcArea * area = Context->compositionArea;

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

    if (Context->hasG2D && Context->geometryChanged
    &&  Context->hasDim
    )
    {
        /* DIM value 255 is very special. It does not do any dim effection but
         * only make a clear to (0,0,0,1). So we do not need to draw any layers
         * under it.  See LayerDim::onDraw.
         * Same for other opaque layer. */
        for (gctUINT32 i = 1; i < Context->layerCount; i++)
        {
            if (Context->layers[i].opaque == gcvFALSE)
            {
                /* Likely, not opaque layer. */
                continue;
            }

            /* Find a layer with solid dim. */
            gctUINT32 owner = (1U << i);

            hwcArea * area = Context->compositionArea;

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

    if (Context->hasG2D)
    {
        /* Geometry is not changed and we are doing 2D composition. So we need
         * to update source for each layer. */
        for (gctUINT32 i = 0; i < List->numHwLayers; i++)
        {
            /* Get shortcuts. */
            hwcLayer * layer = &Context->layers[i];

            /* Update layer source (for blitter layer). */
            if (layer->source != gcvNULL)
            {
                gceSURF_TYPE   type;
                gceSURF_FORMAT format;
                gceTILING      tiling;
                gctUINT        stride;

                gc_native_handle_t * handle
                    = gc_native_handle_get(List->hwLayers[i].handle);

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
                    _DetectSource(Context, List, i);
                }

#if gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
                /* Pop up shared states. */
                gcmVERIFY_OK(gcoSURF_PopSharedInfo(layer->source));
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

        /* Clear geometry change flag. */
        Context->geometryChanged = gcvFALSE;
    }

#if ENABLE_SWAP_RECTANGLE

    /***************************************************************************
    ** Swap Rectangle Optimization.
    */

    if (Context->hasG2D)
    {
        /* Get short cuts. */
        hwcFramebuffer * framebuffer = Context->framebuffer;
        hwcBuffer * buffer           = framebuffer->target;

        /* Free all allocated swap areas. */
        if (Context->swapArea != NULL)
        {
            _FreeArea(Context, Context->swapArea);
            Context->swapArea = NULL;
        }

        /* Optimize: do not need split area for full screen composition. */
        if (
            (   (buffer->swapRect.left > 0)
            ||  (buffer->swapRect.top  > 0)
            ||  (buffer->swapRect.right  < framebuffer->res.right)
            ||  (buffer->swapRect.bottom < framebuffer->res.bottom)
            )
        &&  (buffer != buffer->next)
        )
        {
            /* Put target swap rectangle. */
            Context->swapArea = _AllocateArea(Context,
                                              NULL,
                                              &buffer->swapRect,
                                              1U);

            /* Point to earlier buffer. */
            buffer = buffer->next;

            /* Split area with rectangles of earlier buffer(with no-owner).
             * Now owner 0 means we need copy area from front buffer.
             * 1 means target swap rectangle. */
            while (buffer != framebuffer->target)
            {
                _SplitArea(Context, Context->swapArea, &buffer->swapRect, 0U);

                /* Advance to next framebuffer buffer. */
                buffer = buffer->next;
            }
        }
    }
#endif


    /***************************************************************************
    ** Debug and Dumps.
    */

    if (Context->dumpCompose)
    {

        LOGD("LAYERS:");
        hwcDumpLayer(List);
    }

    if (Context->hasG2D && Context->dumpSplitArea)
    {
        LOGD("SPLITED AREA:");
        hwcDumpArea(Context->compositionArea);

        LOGD("SWAP AREA:");
        hwcDumpArea(Context->swapArea);
    }


    /***************************************************************************
    ** Do Compose.
    */

    if (Context->hasG2D)
    {
        /* Start composition if we have hwc composition. */
        status = hwcComposeG2D(Context);

        if (gcmIS_ERROR(status))
        {
            LOGE("Failed in %s: status=%d", __FUNCTION__, status);
        }
    }


    /***************************************************************************
    ** Do Overlay.
    */

    /* Compose overlay layers. */
    if (Context->hasOverlay)
    {
        if (hwcOverlay(Context, List) != 0)
        {
            LOGE("%s(%d): failed in setting overlay", __FUNCTION__, __LINE__);
        }
    }

    /***********************************************************************
    ** Unlock layer sources.
    */

    if (Context->hasG2D)
    {
        /* Get target surface. */
        gc_native_handle_t * handle = gc_native_handle_get(BackBuffer->handle);

        gcoSURF surface = (gcoSURF) intptr_t(handle->surface);

        /* Unlock target address. */
        gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));

        for (gctUINT i = 0; i < Context->layerCount; i++)
        {
            /* Get shortcut. */
            hwcLayer * layer = &Context->layers[i];

            if (layer->source != gcvNULL)
            {
                /* Unlock the surface. */
                gcmVERIFY_OK(gcoSURF_Unlock(layer->source, gcvNULL));
            }
        }
    }

    return status;
}


void
_DetectSource(
    IN hwcContext * Context,
    IN hwc_layer_list_t * List,
    IN gctINT Index
    )
{
    hwc_layer_t *  hwLayer = &List->hwLayers[Index];
    hwcLayer *     layer   = &Context->layers[Index];

    gcoSURF surface;
    gceSURF_TYPE type;

    gcsSURF_FORMAT_INFO_PTR info[2];
    gc_native_handle_t  * handle;

    gctINT  top;
    gctINT  bottom;
    gctBOOL yflip;

    /* Get source handle. */
    handle = gc_native_handle_get(hwLayer->handle);

    /* Get source surface. */
    surface = (gcoSURF) handle->surface;

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
    gcsRECT_PTR sourceCrop = (gcsRECT *) &hwLayer->sourceCrop;

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
    gcsRECT_PTR res = &Context->framebuffer->res;

    /* Clip dest into framebuffer area. */
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
    IN hwcContext * Context,
    IN hwcArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    hwcArea * area;
    hwcAreaPool * pool  = Context->areaPool;

    if (pool == NULL)
    {
        /* First pool. */
        pool = (hwcAreaPool *) malloc(sizeof (hwcAreaPool));
        Context->areaPool = pool;

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
    IN hwcContext * Context,
    IN hwcArea* Head
    )
{
    /* Free the first node is enough. */
    hwcAreaPool * pool  = Context->areaPool;

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
    IN hwcContext * Context,
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
            _AllocateArea(Context, Area, Rect, Owner);
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
                _AllocateArea(Context, Area, &r1[i], Owner);
            }
        }

        else
        {
            /* Rects outside area. */
            for (gctUINT32 i = 0; i < c1; i++)
            {
                _SplitArea(Context, Area, &r1[i], Owner);
            }
        }
    }

    if (c0 > 0)
    {
        /* Save rects inside area but not overlapped. */
        for (gctUINT32 i = 0; i < c0; i++)
        {
            _AllocateArea(Context, Area, &r0[i], Area->owners);
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

