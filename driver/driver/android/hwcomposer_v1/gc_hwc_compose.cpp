/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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


/* Setup framebuffer source(for swap rectangle). */
static gceSTATUS
_CopySwapRect(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

/* Setup worm hole source. */
static gceSTATUS
_ClearWormHole(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

/* Clear full screen */
static gceSTATUS
_ClearFullScreen(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

/* Setup blit source. */
static gceSTATUS
_Blit(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Index,
    IN gctUINT32 Dim,
    IN gcsRECT * ClipRect
    );

/* Setup multi-source blit source. */
static gceSTATUS
_MultiSourceBlit(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Indices[8],
    IN gctUINT32 Dims[8],
    IN gctUINT32 Count,
    IN gcsRECT * EigenRect,
    IN gcsRECT * ClipRect
    );

/* Setup multi-source blit source with source offset. */
static gceSTATUS
_MultiSourceBlit2(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Indices[8],
    IN gctUINT32 Dims[8],
    IN gctUINT32 Count,
    IN gcsRECT * ClipRect
    );

static gctBOOL
_IsForceOpaque(
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 LayerIndex
    );


/*******************************************************************************
**
**  hwcComposeG2D
**
**  Do actual composition in hwcomposer.
**
**  1. Do swap rectangle optimization if enabled.
**  2. Clear overlay area.
**  3. Fill worm holes with opaque black.
**  4. Compose all layers which can be composed by hwcomposer: dim layer,
**     'clear hole' layer, normal layer and 'overlay clear'.
**
**  INPUT:
**
**      hwcContext * Context
**          hwcomposer context pointer.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
hwcComposeG2D(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    gceSTATUS status;
    hwcArea * area;
    /* Hold a mask for layers are composed. */
    gctUINT32 composed = 0U;
    gctBOOL   commited = gcvFALSE;

    hwcBuffer * target = Display->target;

    /* Setup clipping to full screen. */
    gcmONERROR(gco2D_SetClipping(Context->engine, &Display->res));


    if (Context->dumpCompose & DUMP_OPERATIONS)
    {
        LOGD("COMPOSE of dpy=%d: target=0x%08x tsConfig=%d",
             Display->disp, target->physical, target->tsConfig);
    }

    if (Display->layerCount == 0)
    {
        /* Do a full-screen 2D clear if no layers. */
        _ClearFullScreen(Context, Display);
        return gcvSTATUS_OK;
    }

#if ENABLE_SWAP_RECTANGLE

    /***************************************************************************
    ** Swap Rectangle Optimization.
    */

    /* Copy swap areas from front buffer to current back buffer.
     * This process is independent because the layer composition will never
     * touch swap areas.
     * So it is safe to put this process before everything. */
    if (Display->swapArea != NULL)
    {
        /* Do swap rectangle optimization. */
        gcmONERROR(_CopySwapRect(Context, Display));
    }

    /* Invalid swap rect means no change for current buffer. */
    if (target->swapRect.left >= target->swapRect.right ||
        target->swapRect.top  >= target->swapRect.bottom)
    {
        return gcvSTATUS_OK;
    }

#else

    /* Just to avoid warning. */
    (void) _CopySwapRect;
    (void) target;
#endif

#if ENABLE_DITHER

    /* Enable dithering. */
    gcmONERROR(gco2D_EnableDither(Context->engine, gcvTRUE));
#endif

#if CLEAR_FB_FOR_OVERLAY

    /***************************************************************************
    ** Clear framebuffer for overlay.
    */

    /* Clear framebuffer area to (0,0,0,0) for overlay .
     * See HWC_HINT_CLEAR_FB process in 'setupHardwareComposer' function in
     * SurfaceFlinger.cpp. 'setupHardwareCompose' is beofre 'composeSurfaces',
     * which means, this operation should be before composing surfaces. */
    if (Display->hasOverlay)
    {
        gctUINT32 overlayOwners = 0U;

        for (gctUINT i = 0; i < Display->layerCount; i++)
        {
            if (Display->layers[i].compositionType == HWC_OVERLAY)
            {
                overlayOwners |= (1U << i);
            }
        }

        /* Save this overlay layer to composed mask. */
        composed |= overlayOwners;

        for (gctUINT i = 0; i < Display->layerCount; i++)
        {
            if ((overlayOwners & (1U << i)) == 0U)
            {
                continue;
            }

            /* Go through all areas.. */
            area = Display->compositionArea;

            while (area != NULL)
            {
                if ((area->owners != overlayOwners))
                {
                    area = area->next;
                    continue;
                }

#if ENABLE_SWAP_RECTANGLE
                gcsRECT clipRect;

                /* Intersect with swap rectangle. */
                clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
                clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
                clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
                clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

                if ((clipRect.left >= clipRect.right)
                ||  (clipRect.top  >= clipRect.bottom)
                )
                {
                    area = area->next;
                    continue;
                }

                gcmONERROR(_Blit(Context, Display, area, i, 0xFF, &clipRect));
#   else
                gcmONERROR(_Blit(Context, Display, area, i, 0xFF, &area->rect));
#   endif

                /* Advance to next area. */
                area = area->next;
            }
        }
    }
#endif

    /***************************************************************************
    ** Clear Worm Hole.
    */

    /* Clear worm holes before composing layers.
     * See 'drawWormhole' part in 'composeSurfaces' in SurfaceFlinger.cpp.
     * For 3D path, 'drawWormhole' is beofre draw layers in 'composeSurfaces',
     * which means worm hole clearing should be before draw layers.
     */

    gcmONERROR(_ClearWormHole(Context, Display));

#if ENABLE_CLEAR_HOLE

    /***************************************************************************
    ** Draw CLEAR_HOLE.
    */

    /* For CLEAR_HOLE type, which must be on bottom of each area, needs to be
     * cleared by area. It is safe to make it done here because clear holes
     * will never cover others. */
    if (Display->hasClearHole)
    {
        for (gctUINT i = 0; i < Display->layerCount; i++)
        {
            gctUINT32 owner;

            if (Display->layers[i].compositionType != HWC_CLEAR_HOLE)
            {
                /* Not clear hole. */
                continue;
            }

            /* Compute owner. */
            owner = (1U << i);

            /* Save this clear hole layer to mask. */
            composed |= owner;

            /* Go through all areas. */
            area = Display->compositionArea;

            while (area != NULL)
            {
                if ((owner & area->owners) == 0)
                {
                    area = area->next;
                    continue;
                }

#if ENABLE_SWAP_RECTANGLE
                gcsRECT clipRect;

                /* Intersect with swap rectangle. */
                clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
                clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
                clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
                clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

                if ((clipRect.left >= clipRect.right)
                ||  (clipRect.top  >= clipRect.bottom)
                )
                {
                    area = area->next;
                    continue;
                }

                gcmONERROR(_Blit(Context, Display, area, i, 0xFF, &clipRect));
#   else
                gcmONERROR(_Blit(Context, Display, area, i, 0xFF, &area->rect));
#   endif
                /* Advance to next area. */
                area = area->next;
            }
        }
    }
#endif

    /***************************************************************************
    ** Compose layers (BLITTER, DIM).
    */

    /* Check multi-source blit feature. */
    if (Context->multiSourceBlt == gcvFALSE)
    {
        /* Singal source blit path.
         * Check compose type: by area or by layer.
         *   1) By default, we compose by layer.
         *   2) If hasDim, we compose by area for performance
         *   3) But if there's any YUV layer, we always compose by layer. */
        gctBOOL composeByArea = gcvFALSE;

        if (Display->hasDim)
        {
            /* Compose by area for dim optimization. */
            composeByArea = gcvTRUE;

            for (gctUINT i = 0; i < Display->layerCount; i++)
            {
                if ((Display->layers[i].source)
                &&  (Display->layers[i].yuv || Display->layers[i].stretch)
                &&  (Display->layers[i].compositionType == HWC_BLITTER)
                )
                {
                    /* But if there's any yuv layer, it is better to compose all
                     * by layer. Or when stretch blit is used. Clipping box
                     * performance is relatively low. Better to have less
                     * rectangles. */
                    composeByArea = gcvFALSE;
                    break;
                }
            }
        }

        if (Display->hasOverlay)
        {
            /* Must compose by area. */
            composeByArea = gcvTRUE;
        }

        if (composeByArea)
        {
            /* Compose by area.
             * We only need to go through all areas
             * BLITTER: blit area
             * DIM: only blit area if on bottom
             *      use alpha dim for non-bottom DIM areas
             * CLEAR_HOLE & OVERLAY: skip because already composed */
            area = Display->compositionArea;

            while (area != NULL)
            {
                gcsRECT clipRect;

                if ((area->owners & ~composed) == 0U)
                {
                    /* Reject this area because has no layers. */
                    area = area->next;
                    continue;
                }

#if ENABLE_SWAP_RECTANGLE
                /* Intersect with swap rectangle. */
                clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
                clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
                clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
                clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

                if ((clipRect.left >= clipRect.right)
                ||  (clipRect.top  >= clipRect.bottom)
                )
                {
                    /* Skip areas out of swap rectangle. */
                    area = area->next;
                    continue;
                }
#else
                clipRect = area->rect;
#endif

                /* Go through all areas. */
                for (gctUINT32 i = 0; i < Display->layerCount; i++)
                {
                    gctUINT32 dim;
                    hwcLayer * layer;
                    gctUINT32 owner;

                    /* Compute owner. */
                    owner = (1U << i);

                    if ((!(area->owners & owner))
                    ||  (composed & owner)
                    )
                    {
                        if (owner > area->owners)
                        {
                            /* No more layers. */
                            break;
                        }

                        /* No such layer or already composed. */
                        continue;
                    }

                    /* Get shortcut. */
                    layer = &Display->layers[i];

                    if ((layer->compositionType == HWC_DIM)
                    &&  (((owner - 1U) & area->owners))
                    )
                    {
                        /* Skip DIM layer if it is not on bottom.
                         * Non-bottom DIM layers are done with optimization. */
                        continue;
                    }

                    /* Compute dim. */
                    dim = 0xFF;

                    for (gctUINT32 j = (i + 1); j < Display->layerCount; j++)
                    {
                        if (((1U << j) & area->owners)
                        &&  (Display->layers[j].compositionType == HWC_DIM)
                        )
                        {
                            gctUINT32 d = 0xFF - (Display->layers[j].color32 >> 24);

                            /* Compute dim value. */
                            if (d < 0xFF)
                            {
                                dim = dim * d / 0xFF + (d < 127 ? 2 : 1);
                            }
                        }
                    }

                    /* Start single-source blit. */
                    gcmONERROR(
                        _Blit(Context, Display, area, i, dim, &clipRect));
                }

                /* Advance to next area. */
                area = area->next;

                /* Commit commands. */
                if (commited == gcvFALSE)
                {
                    gcmONERROR(gcoHAL_Commit(Context->hal, gcvFALSE));
                    commited = gcvTRUE;
                }
            }

            /* All areas done. */
            return gcvSTATUS_OK;
        }

        /* else compose by layer */

        /* Go through all layers.
         * BLITTER: blit layer
         * DIM: blit layer
         * CLEARHOLE & OVERLAY: skip because already composed */
        for (gctUINT32 i = 0; i < Display->layerCount; i++)
        {
            if ((1U << i) & composed)
            {
                /* Already composed: CLEAR_HOLE/OVERLAY. */
                continue;
            }

            /* Get shortcut. */
            hwcLayer * layer = &Display->layers[i];

            for (gctUINT32 j = 0; j < layer->clipCount; j++)
            {
                gcsRECT clipRect;
                gcsRECT_PTR dstRect = &layer->dstRect;

                /* Shortcut to dest clip rect. */
                gcsRECT_PTR rect = &layer->clipRects[j];

#if ENABLE_SWAP_RECTANGLE
                /* Intersect with swap rectangle. */
                clipRect.left   = gcmMAX(target->swapRect.left,   rect->left);
                clipRect.top    = gcmMAX(target->swapRect.top,    rect->top);
                clipRect.right  = gcmMIN(target->swapRect.right,  rect->right);
                clipRect.bottom = gcmMIN(target->swapRect.bottom, rect->bottom);
#else
                /* Intersect with display size. */
                clipRect.left   = gcmMAX(Display->res.left,   rect->left);
                clipRect.top    = gcmMAX(Display->res.top,    rect->top);
                clipRect.right  = gcmMIN(Display->res.right,  rect->right);
                clipRect.bottom = gcmMIN(Display->res.bottom, rect->bottom);
#endif

                /* Intersect with dest rectangle. */
                clipRect.left   = gcmMAX(clipRect.left,   dstRect->left);
                clipRect.top    = gcmMAX(clipRect.top,    dstRect->top);
                clipRect.right  = gcmMIN(clipRect.right,  dstRect->right);
                clipRect.bottom = gcmMIN(clipRect.bottom, dstRect->bottom);

                if ((clipRect.left >= clipRect.right)
                ||  (clipRect.top  >= clipRect.bottom)
                )
                {
                    /* Skip clipRect out of swap rectangle. */
                    continue;
                }

                /* Start single-source blit. */
                gcmONERROR(_Blit(Context, Display, NULL, i, 0xFF, &clipRect));
            }

            /* Commit commands. */
            if (commited == gcvFALSE)
            {
                gcmONERROR(gcoHAL_Commit(Context->hal, gcvFALSE));
                commited = gcvTRUE;
            }
        }

        /* Done. */
        return gcvSTATUS_OK;
    }

    /* Multi-source blit path.
     * Some layers can never be composed by multi-source blit. Let's call such
     * layers as 'barrier'. 'Barrier' layers are always composed by layer but
     * never by area.
     * Between 'barrier' layers, we try multi-source blit by area.
     *
     * BLITTER: barrier if YUV, stretch or mirror(no Source-Mirror feature)
     * DIM:  do DIM optimization if start is 0, and this is the first DIM layer.
     * CLEAR_HOLE & OVERLAY: skip because already composed */
    for (gctUINT start = 0; start < Display->layerCount; start++)
    {
        hwcLayer * layer;
        /* Record how many layers between start and barrier layer. */
        gctUINT between = 0;
        /* Record the barrier layer index. */
        gctUINT barrier;

        gctINT hMirror = -1;
        gctINT vMirror = -1;
        gctINT rotation = -1;

        if ((1U << start) & composed)
        {
            /* Already composed: CLEAR_HOLE/OVERLAY. */
            continue;
        }

        /* Try to find a barrier layer. */
        for (barrier = start; barrier < Display->layerCount; barrier++)
        {
            if ((1U << barrier) & composed)
            {
                /* Already composed: CLEAR_HOLE/OVERLAY. */
                continue;
            }

            /* Get shortcut. */
            layer = &Display->layers[barrier];

            if (layer->source != gcvNULL)
            {
                if (layer->stretch
                && (  !Context->multiSourceBltV2
                   || layer->xScale < 2 ||  layer->yScale < 2)
                )
                {
                    /*
                     * Multi-source stretch need version 2 feature. But even the
                     * feature is there, it still benifits if shrinks too much.
                     */
                    break;
                }

                /* support yuv when EX feature exists. */
                if (layer->yuv && !Context->multiSourceBltEx)
                {
                    /* Do not support yuv layer. */
                    break;
                }

                if (!Context->msSourceMirror)
                {
                    if (hMirror < 0)
                    {
                        hMirror = gctINT(layer->hMirror);
                        vMirror = gctINT(layer->vMirror);
                    }
                    else
                    if ((hMirror != layer->hMirror)
                    ||  (vMirror != layer->vMirror)
                    )
                    {
                        /* Only support same mirror for one batch. */
                        break;
                    }
                }

                if (!Context->msSourceRotation)
                {
                    /* Only accept same rotation, will tranfer to dest rotation. */
                    if (rotation < 0)
                    {
                        rotation = gctINT(layer->rotation);
                    }
                    else if (rotation != gctINT(layer->rotation))
                    {
                        /* Same rotation in one batch. */
                        break;
                    }
                }
            }

            if ((layer->compositionType == HWC_DIM)
            &&  ((start != 0) || (barrier == 0))
            )
            {
                /* Dim as barrier if start layer is not bottom.
                 * This is because, if start layer is not bottom, this DIM layer
                 * may cover layers under start.
                 * Another case, the layer on bottom is DIM.
                 * The bottom DIM layer can not use DIM optimization. */
                break;
            }

            if (layer->compositionType != HWC_DIM)
            {
                gctINT res     = Display->res.right * Display->res.bottom;
                gctINT acreage = (layer->dstRect.right  - layer->dstRect.left)
                               * (layer->dstRect.bottom - layer->dstRect.top);

                if ((acreage > 0) && (res / acreage > 10))
                {
                    /* Too small area. */
                    break;
                }
            }

            if ((barrier == Display->layerCount - 1)
            &&  layer->opaque
            )
            {
                /* The last layer is opaque.
                 * Do not try multi-source blit for it. */
                break;
            }

            /* Inc layer counter between start and barrier. */
            if (layer->source != gcvNULL)
            {
                between ++;
            }
        }

        /* Check layer between. */
        if (between <= 1)
        {

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD(" Single-source between [%d],[%d]", start, barrier);
            }

            /* Blit by layer from start and barrier(include) if,
             * 1. There's no layer between start and barrier, which means the
             * start layer is the barrier layer.
             * 2. there's only one layer between start and barrier, which
             * means the start layer is the only layer. */
            for (gctUINT i = start;
                 i <= barrier && i < Display->layerCount;
                 i++)
            {
                gctUINT32 owner = (1U << i);

                if (owner & composed)
                {
                    /* Already composed: CLEAR_HOLE/OVERLAY. */
                    continue;
                }

                /* Get shortcut. */
                area = Display->compositionArea;

                while (area != NULL)
                {
                    gcsRECT clipRect;

                    if ((area->owners & owner) == 0U)
                    {
                        area = area->next;
                        continue;
                    }

#if ENABLE_SWAP_RECTANGLE
                    /* Intersect with swap rectangle. */
                    clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
                    clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
                    clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
                    clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

                    if ((clipRect.left >= clipRect.right)
                    ||  (clipRect.top  >= clipRect.bottom)
                    )
                    {
                        /* Skip clipRect out of swap rectangle. */
                        area = area->next;
                        continue;
                    }
#else
                    clipRect = area->rect;
#endif

                    /* Start single-source blit. */
                    gcmONERROR(
                        _Blit(Context, Display, area, i, 0xFF, &clipRect));

                    /* Check next area. */
                    area = area->next;
                }
            }

            /* We have handled layers till barrier. */
            start = barrier;

            /* Commit commands. */
            if (commited == gcvFALSE)
            {
                gcmONERROR(gcoHAL_Commit(Context->hal, gcvFALSE));
                commited = gcvTRUE;
            }

            /* Go to next batch directly. */
            continue;
        }

        if (Context->dumpCompose & DUMP_OPERATIONS)
        {
            LOGD(" Try multi-source between [%d],[%d]", start, barrier - 1);
        }

        /* Try multi-source blit by area between start and barrier. */
        area = Display->compositionArea;

        while (area != NULL)
        {
            gcsRECT clipRect;

            if ((area->owners & ~composed) == 0U)
            {
                /* Reject this area because has no layers. */
                area = area->next;
                continue;
            }

#if ENABLE_SWAP_RECTANGLE
            /* Intersect with swap rectangle. */
            clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
            clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
            clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
            clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

            if ((clipRect.left >= clipRect.right)
            ||  (clipRect.top  >= clipRect.bottom)
            )
            {
                /* Skip areas out of swap rectangle. */
                area = area->next;
                continue;
            }
#else
            clipRect = area->rect;
#endif

            /* Multi-source blit locals. */
            gctUINT32 sourceIndices[8];
            gctUINT32 sourceDims[8];
            gctUINT32 sourceCount = 0;

            /* Eigen rect. */
            /* Value -1  means any value is OK.
             * Otherwise means specified values. */
            gcsRECT eigenRect;

            /* Correspond masks. Since different condition has different
             * constraints, we need remember the constraints as well. */
            gcsRECT maskRect = {0, 0, 0, 0};

            /* YUV input existed.
             * Multi-source blit can only support one YUV input. */
            gctUINT yuvSource   = 0;

            /* Multi-source layers initialized? */
            gctBOOL initialized = gcvFALSE;

            /* Get initial left eigenvalue. */
            {
                /* Framebuffer alignment constraint. */
                eigenRect.left  = clipRect.left & Display->alignMask;
                maskRect.left   = Display->alignMask;
            }

            /* Get other initial eigenvalues. */
            eigenRect.top    =
            eigenRect.right  =
            eigenRect.bottom = -1;

            /* Try multi-source blit between start and barrier. */
            for (gctUINT32 i = start; i < barrier; i++)
            {
                gctUINT32 owner = (1U << i);
                gctUINT32 dim;

                if (!(area->owners & owner)
                ||  (owner & composed)
                )
                {
                    if (owner > area->owners)
                    {
                        /* No more layers. */
                        break;
                    }

                    /* No such layer or already composed. */
                    continue;
                }

                /* Get short cut. */
                layer = &Display->layers[i];

                if (layer->compositionType == HWC_DIM)
                {
                    if (!initialized)
                    {
                        /* Detect on-bottom (in this batch) DIM layer. */
                        gctUINT32 under = area->owners & (owner - 1U)
                                        & ~composed & ~((1U << start) - 1U);

                        if (under == 0U)
                        {
                            /* Compose on bottom DIM layer. */
                            gcmONERROR(_Blit(Context, Display, NULL, i, 0xFF, &clipRect));
                        }
                    }

                    /* Skip DIM layer because DIM optimization is used for
                     * non-bottom layers, and done for bottom layers above. */
                    continue;
                }

                /* Compute dim. */
                dim = 0xFF;

                for (gctUINT32 j = (i + 1); j < barrier; j++)
                {
                    if (((1U << j) & area->owners)
                    &&  (Display->layers[j].compositionType == HWC_DIM)
                    )
                    {
                        gctUINT32 d = 0xFF - (Display->layers[j].color32 >> 24);

                        /* Compute dim value. */
                        if (d < 0xFF)
                        {
                            dim = dim * d / 0xFF + (d < 127 ? 2 : 1);
                        }
                    }
                }

                /* Handled by multi-source blit? */
                gctBOOL handled = gcvFALSE;

                do
                {
                    if (layer->source == gcvNULL)
                    {
                        /* This layer must be non-bottom DIM layer. */
                        handled = gcvTRUE;
                        break;
                    }

                    if (layer->yuv && yuvSource >= Context->msYuvSourceCount)
                    {
                        /* Can not support more yuv layers. */
                        break;
                    }

                    if (Context->msSourceOffset)
                    {
                        /* Do not need move source address. */
                        handled = gcvTRUE;
                        break;
                    }

                    gctINT dl, dt, dr, db;
                    gctINT ml = 0, mt = 0, mr = 0, mb = 0;
                    gctINT el = -1;
                    gctINT et = -1;
                    gctINT er = -1;
                    gctINT eb = -1;

                    /* Compute deltas. */
                    dl = layer->orgDest.left   - clipRect.left;
                    dt = layer->orgDest.top    - clipRect.top;
                    dr = layer->orgDest.right  - clipRect.right;
                    db = layer->orgDest.bottom - clipRect.bottom;

                    if (layer->hMirror)
                    {
                        gctINT tl = dl;

                        dl = -dr;
                        dr = -tl;
                    }

                    if (layer->vMirror)
                    {
                        gctINT tt = dt;

                        dt = -db;
                        db = -tt;
                    }

                    /* Check 'Multi-Source Blit Byte Alignment' feature. */
                    if (Context->msByteAlignment
                    &&  !layer->yuv
                    &&  layer->tiling == gcvLINEAR
                    )
                    {
                        /* If 'Byte-Alignment' feature is avaible, there's no buffer address
                         * alignment limitation for RGB source. */
                        if (Context->msOddCoord)
                        {
                            handled = gcvTRUE;
                            break;
                        }

                        gctINT dx = 0, dy = 0;

                        el = eigenRect.left   > 0 ? eigenRect.left   : 0;
                        et = eigenRect.top    > 0 ? eigenRect.top    : 0;
                        er = eigenRect.right  > 0 ? eigenRect.right  : 0;
                        eb = eigenRect.bottom > 0 ? eigenRect.bottom : 0;

                        switch (layer->rotation)
                        {
                        case gcvSURF_0_DEGREE:
                        default:
                            dx = layer->orgRect.left - dl - el;
                            dy = layer->orgRect.top  - dt - et;
                            break;

                        case gcvSURF_90_DEGREE:
                            dx = layer->orgRect.left + db - eb;
                            dy = layer->orgRect.top  - dl - el;
                            break;

                        case gcvSURF_180_DEGREE:
                            dx = layer->orgRect.left + dr - er;
                            dy = layer->orgRect.top  + db - eb;
                            break;

                        case gcvSURF_270_DEGREE:
                            dx = layer->orgRect.left - dt - et;
                            dy = layer->orgRect.top  + dr - er;
                            break;
                        }

                        if (((dx < 0) && (dx & 1))
                        ||  ((dy < 0) && (dy & 1))
                        )
                        {
                            handled = gcvFALSE;
                            break;
                        }
                    }

                    /* Check start address alignments.
                     * We need to move buffer address to an aligned one, so we
                     * need compute coorespoinding coordinate offsets.
                     * The 'eigenvalues' are actually left coodinate offsets in
                     * 'original coord sys'.
                     *
                     * Left coordinate becomes at left,bottom,right,top when 0,90,
                     * 180,270 degree rotated.
                     * So the first step is to compute needed coordinate (need left
                     * for 0 deg, bottom for 90 deg, etc) to 'original coord sys',
                     * and then move the left coordinate in 'original coord sys' to
                     * aligned one.
                     *
                     * Top coordinate becomes at top,left,bottom,right when 0,90,
                     * 180,270 degree rotated.
                     * So the first step is to compute needed coordinate (need top
                     * for 0 deg, left for 90 deg, etc) to 'original coord sys',
                     * and then move the top coordinate in 'original coord sys' to
                     * aligned one.
                     *
                     *  [+]------------------------+ [+]-------------------------+
                     *  |        ^                 |  |        ^                 |
                     *  |        T (0 deg)         |  |        L (270 deg)       |
                     *  |        v                 |  |        v                 |
                     *  |     +--------------+     |  |     +--------------+     |
                     *  |     | blit rect    |     |  |     | blit rect    |     |
                     *  |<-T->|              |<-T->|  |<-L->|              |<-L->|
                     *  |(90  |              |(270 |  |(0   |              |(180 |
                     *  | deg)|              | deg)|  | deg)|              | deg)|
                     *  |     +--------------+     |  |     +--------------+     |
                     *  |        ^                 |  |        ^                 |
                     *  |        T (180 deg)       |  |        L (90 deg)        |
                     *  |        v                 |  |        v                 |
                     *  +--------------------------+  +--------------------------+
                     */
                    switch (layer->rotation)
                    {
                    case gcvSURF_0_DEGREE:
                    default:
                        ml = layer->alignMaskX;
                        mt = layer->alignMaskY;
                        el = (layer->orgRect.left - dl) & ml;
                        et = (layer->orgRect.top  - dt) & mt;
                        break;

                    case gcvSURF_90_DEGREE:
                        mb = layer->alignMaskX;
                        ml = layer->alignMaskY;
                        eb = (layer->orgRect.left + db) & mb;
                        el = (layer->orgRect.top  - dl) & ml;
                        break;

                    case gcvSURF_180_DEGREE:
                        mr = layer->alignMaskX;
                        mb = layer->alignMaskY;
                        er = (layer->orgRect.left + dr) & mr;
                        eb = (layer->orgRect.top  + db) & mb;
                        break;

                    case gcvSURF_270_DEGREE:
                        mt = layer->alignMaskX;
                        mr = layer->alignMaskY;
                        et = (layer->orgRect.left - dt) & mt;
                        er = (layer->orgRect.top  + dr) & mr;
                        break;
                    }

                    if (el >= 0)
                    {
                        if (eigenRect.left < 0)
                        {
                            /* Get new left eigenvalue. */
                            eigenRect.left = el;
                            maskRect.left  = ml;
                            handled        = gcvTRUE;
                        }
                        else
                        {
                            if (maskRect.left == ml)
                            {
                                /* Check with last left eigenvalue. */
                                handled = (eigenRect.left == el);
                            }
                            else if (maskRect.left > ml)
                            {
                                /* Check with masked last left eigenvalue. */
                                handled = ((eigenRect.left & ml) == el);
                            }
                            else
                            {
                                /* Stronger constraint comes. */
                                if (eigenRect.left == (el & maskRect.left))
                                {
                                    handled = gcvTRUE;

                                    /* Update mask. */
                                    maskRect.left = ml;

                                    /* Update left eigenvalue. */
                                    eigenRect.left = el;
                                }
                                else
                                {
                                    handled = gcvFALSE;
                                }
                            }
                        }

                        if (handled == gcvFALSE)
                        {
                            break;
                        }
                    }

                    if (et >= 0)
                    {
                        if (eigenRect.top < 0)
                        {
                            /* Get new top eigenvalue. */
                            eigenRect.top = et;
                            maskRect.top  = mt;
                            handled       = gcvTRUE;
                        }
                        else
                        {
                            if (maskRect.top == mt)
                            {
                                /* Check with last top eigenvalue. */
                                handled = (eigenRect.top == et);
                            }
                            else if (maskRect.top > mt)
                            {
                                /* Check with masked last top eigenvalue. */
                                handled = ((eigenRect.top & mt) == et);
                            }
                            else
                            {
                                /* Stronger constraint comes. */
                                if (eigenRect.top == (et & maskRect.top))
                                {
                                    handled = gcvTRUE;

                                    /* Update mask. */
                                    maskRect.top = mt;

                                    /* Update top eigenvalue. */
                                    eigenRect.top = et;
                                }
                                else
                                {
                                    handled = gcvFALSE;
                                }
                            }
                        }

                        if (handled == gcvFALSE)
                        {
                            break;
                        }
                    }

                    if (er >= 0)
                    {
                        if (eigenRect.right < 0)
                        {
                            /* Get new right eigenvalue. */
                            eigenRect.right = er;
                            maskRect.right  = mr;
                            handled         = gcvTRUE;
                        }
                        else
                        {
                            if (maskRect.right == mr)
                            {
                                /* Check with last right eigenvalue. */
                                handled = (eigenRect.right == er);
                            }
                            else if (maskRect.right > mr)
                            {
                                /* Check with masked last right eigenvalue. */
                                handled = ((eigenRect.right & mr) == er);
                            }
                            else
                            {
                                /* Stronger constraint comes. */
                                if (eigenRect.right == (er & maskRect.right))
                                {
                                    handled = gcvTRUE;

                                    /* Update mask. */
                                    maskRect.right = mr;

                                    /* Update right eigenvalue. */
                                    eigenRect.right = er;
                                }
                                else
                                {
                                    handled = gcvFALSE;
                                }
                            }
                        }

                        if (handled == gcvFALSE)
                        {
                            break;
                        }
                    }

                    if (eb >= 0)
                    {
                        if (eigenRect.bottom < 0)
                        {
                            /* Get new bottom eigenvalue. */
                            eigenRect.bottom = eb;
                            maskRect.bottom  = mb;
                            handled          = gcvTRUE;
                        }
                        else
                        {
                            if (maskRect.bottom == mb)
                            {
                                /* Check with last bottom eigenvalue. */
                                handled = (eigenRect.bottom == eb);
                            }
                            else if (maskRect.bottom > mb)
                            {
                                /* Check with masked last bottom eigenvalue. */
                                handled = ((eigenRect.bottom & mb) == eb);
                            }
                            else
                            {
                                /* Stronger constraint comes. */
                                if (eigenRect.bottom == (eb & maskRect.bottom))
                                {
                                    handled = gcvTRUE;

                                    /* Update mask. */
                                    maskRect.bottom = mb;

                                    /* Update bottom eigenvalue. */
                                    eigenRect.bottom = eb;
                                }
                                else
                                {
                                    handled = gcvFALSE;
                                }
                            }
                        }

                        if (handled == gcvFALSE)
                        {
                            break;
                        }
                    }
                }
                while (gcvFALSE);

                if (initialized && handled)
                {
                    /* Has layers before and the new layer can be handled. */

                    /* Update source array. */
                    sourceDims[sourceCount] = dim;
                    sourceIndices[sourceCount++] = i;

                    /* Increase yuv source count. */
                    if (layer->yuv)
                    {
                        yuvSource++;
                    }

                    /* Check source count limitation. */
                    if (sourceCount >= Context->msMaxSource)
                    {
                        /* Trigger multi-source blit. */
                        if (Context->msSourceOffset)
                        {
                            gcmONERROR(
                                _MultiSourceBlit2(Context, Display,
                                                  area,
                                                  sourceIndices,
                                                  sourceDims,
                                                  sourceCount,
                                                  &clipRect));
                        }
                        else
                        {
                            gcmONERROR(
                                _MultiSourceBlit(Context, Display,
                                                 area,
                                                 sourceIndices,
                                                 sourceDims,
                                                 sourceCount,
                                                 &eigenRect,
                                                 &clipRect));
                        }

                        /* Reset flags. */
                        sourceCount = 0U;
                        yuvSource   = 0U;
                        initialized = gcvFALSE;

                        eigenRect.top    =
                        eigenRect.right  =
                        eigenRect.bottom = -1;
                    }
                }

                else if (initialized)
                {
                    /* Has layers before but the new layer can not be handled. */
                    if (sourceCount > 1)
                    {
                        /* Trigger multi-source blit. */
                        if (Context->msSourceOffset)
                        {
                            gcmONERROR(
                            _MultiSourceBlit2(Context, Display,
                                              area,
                                              sourceIndices,
                                              sourceDims,
                                              sourceCount,
                                              &clipRect));
                        }
                        else
                        {
                            gcmONERROR(
                            _MultiSourceBlit(Context, Display,
                                             area,
                                             sourceIndices,
                                             sourceDims,
                                             sourceCount,
                                             &eigenRect,
                                             &clipRect));
                        }
                    }
                    else
                    {
                        /* Use single-source blit instead. */
                        gcmONERROR(
                            _Blit(Context, Display,
                                  area,
                                  sourceIndices[0],
                                  sourceDims[0],
                                  &clipRect));
                    }

                    /* Reset flags. */
                    sourceCount = 0U;
                    yuvSource   = 0U;
                    initialized = gcvFALSE;

                    eigenRect.top    =
                    eigenRect.right  =
                    eigenRect.bottom = -1;

                    /* Need check this layer again. */
                    i--;
                }

                else if (handled)
                {
                    /* First layer comes. */
                    initialized = gcvTRUE;

                    /* Update source array. */
                    sourceDims[sourceCount] = dim;
                    sourceIndices[sourceCount++] = i;

                    /* Increase yuv source count. */
                    if (layer->yuv)
                    {
                        yuvSource++;
                    }
                }

                else
                {
                    /* A single single-source blit layer comes.
                     * Start single-source blit immediately. */
                    gcmONERROR(_Blit(Context, Display, area, i, dim, &clipRect));
                }
            }

            /* Start multi-source blit for accumulated layers if any. */
            if (sourceCount > 1)
            {
                /* Trigger multi-source blit. */
                if (Context->msSourceOffset)
                {
                    gcmONERROR(
                        _MultiSourceBlit2(Context, Display,
                                          area,
                                          sourceIndices,
                                          sourceDims,
                                          sourceCount,
                                          &clipRect));
                }
                else
                {
                    gcmONERROR(
                        _MultiSourceBlit(Context, Display,
                                         area,
                                         sourceIndices,
                                         sourceDims,
                                         sourceCount,
                                         &eigenRect,
                                         &clipRect));
                }
            }

            else if (sourceCount == 1)
            {
                /* Use single-source blit instead. */
                gcmONERROR(
                    _Blit(Context, Display,
                          area,
                          sourceIndices[0],
                          sourceDims[0],
                          &clipRect));
            }

            /* Advance to next area. */
            area = area->next;

            /* Commit commands. */
            if (commited == gcvFALSE)
            {
                gcmONERROR(gcoHAL_Commit(Context->hal, gcvFALSE));
                commited = gcvTRUE;
            }
        }

        /* Check whether we need blit the barrier layer. */
        if (barrier >= Display->layerCount)
        {
            /* All layers done. */
            break;;
        }

        if (Context->dumpCompose & DUMP_OPERATIONS)
        {
            LOGD(" Single-source for barrier=[%d]", barrier);
        }

        /* Now we need to compose the barrier layer by area. */
        area = Display->compositionArea;

        while (area != NULL)
        {
            gcsRECT clipRect;

            if ((area->owners & (1U << barrier)) == 0U)
            {
                area = area->next;
                continue;
            }

#if ENABLE_SWAP_RECTANGLE
            /* Intersect with swap rectangle. */
            clipRect.left   = gcmMAX(target->swapRect.left,   area->rect.left);
            clipRect.top    = gcmMAX(target->swapRect.top,    area->rect.top);
            clipRect.right  = gcmMIN(target->swapRect.right,  area->rect.right);
            clipRect.bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);
#else
            clipRect = area->rect;
#endif

            if ((clipRect.left >= clipRect.right)
            ||  (clipRect.top  >= clipRect.bottom)
            )
            {
                /* Skip clipRect out of swap rectangle. */
                area = area->next;
                continue;
            }

            /* Start single-source blit. */
            gcmONERROR(
                _Blit(Context, Display, area, barrier, 0xFF, &clipRect));

            /* Advance to next area. */
            area = area->next;
        }

        /* We have handled layers till barrier. */
        start = barrier;
    }

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}


gceSTATUS
_CopySwapRect(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    gceSTATUS status;
    gcsRECT swapRects[16];
    gctUINT rectCount  = 0U;
    hwcBuffer * target = Display->target;
    hwcArea * area     = Display->swapArea;


    /***************************************************************************
    ** Setup Source.
    */

    /* Setup source index. */
    gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, 0U));

    /* Setup source. */
    gcmONERROR(
        gco2D_SetGenericSource(Context->engine,
                               &target->prev->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Previous display buffer tile status. */
    gcmONERROR(
        gco2D_SetSourceTileStatus(Context->engine,
                                  target->prev->tsConfig,
                                  Display->format,
                                  0,
                                  target->prev->tsPhysical));

    /* Setup mirror. */
    gcmONERROR(gco2D_SetBitBlitMirror(Context->engine, gcvFALSE, gcvFALSE));

    /* Disable alhpa blending. */
    gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));

    /* Disable premultiply. */
    gcmONERROR(
        gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE));


    /***************************************************************************
    ** Setup Target.
    */

    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &target->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));

#if ENABLE_DITHER

    /* Disable dithering for swap rectangle. */
    gcmONERROR(gco2D_EnableDither(Context->engine, gcvFALSE));
#endif

    /***************************************************************************
    ** Copy Swap Areas.
    */

    while (area != NULL)
    {
        /* Blit only layers without owners. No owner means we need copy area
         * from front buffer to current. */
        if (area->owners == 0U)
        {
            swapRects[rectCount++] = area->rect;
        }

        /* Advance to next area. */
        area = area->next;

        if ((rectCount == 16) || ((area == NULL) && (rectCount > 0)))
        {
            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("  SWAP RECT: addr=0x%08x",
                     target->prev->physical);

                for (gctUINT i = 0; i < rectCount; i++)
                {
                    LOGD("    [%d,%d,%d,%d]",
                         swapRects[i].left,
                         swapRects[i].top,
                         swapRects[i].right,
                         swapRects[i].bottom);
                }
            }

            /* Do batchblit. */
            gcmONERROR(
                gco2D_BatchBlit(Context->engine,
                                rectCount,
                                swapRects,
                                swapRects,
                                0xCC,
                                0xCC,
                                Display->format));

            /* Reset rect count. */
            rectCount = 0U;
        }
    }

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}

static gceSTATUS
_ClearFullScreen(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    gceSTATUS status;
    hwcBuffer * target = Display->target;

    /* Disable alpha blending. */
    gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));

    /* No premultiply. */
    gcmONERROR(
        gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE));

    /* Setup Target. */
    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &target->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));

    /* Perform a full-screen Clear. */
    gcmONERROR(
        gco2D_Clear(Context->engine,
                    1U,
                    &Display->res,
                    0x00000000,
                    0xCC,
                    0xCC,
                    Display->format));

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}


gceSTATUS
_ClearWormHole(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    )
{
    gceSTATUS status;
    gcsRECT wormHoles[16];
    gctUINT rectCount = 0U;

    hwcArea * area     = Display->compositionArea;
    hwcBuffer * target = Display->target;

    /* Disable alpha blending. */
    gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));

    /* No premultiply. */
    gcmONERROR(
        gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE));

    /* Setup Target. */
    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &target->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));

    /* Go through all areas. */
    while (area != NULL)
    {
        gctUINT i;

        /* Find bottom layer. */
        for (i = 0; i < Display->layerCount; i++)
        {
            /* Do not need to skip composed layers. If the fist layer is
             * composed OVERLAY layer, this area is already cleared. */
            if ((1U << i) & area->owners)
            {
                break;
            }
        }

        /* Worm holes
         * type 1: area not covered by any layer.
         * type 2: (rare case) bottom layer has alpha blending. */
        if ((i == Display->layerCount)
        || (Display->layers[i].opaque == gcvFALSE)
        )
        {
#if ENABLE_SWAP_RECTANGLE
            gcsRECT_PTR wormHole = &wormHoles[rectCount];

            /* Intersect with swap rectangle. */
            wormHole->left   = gcmMAX(target->swapRect.left,   area->rect.left);
            wormHole->top    = gcmMAX(target->swapRect.top,    area->rect.top);
            wormHole->right  = gcmMIN(target->swapRect.right,  area->rect.right);
            wormHole->bottom = gcmMIN(target->swapRect.bottom, area->rect.bottom);

            if ((wormHole->left < wormHole->right)
            &&  (wormHole->top  < wormHole->bottom)
            )
            {
                /* Valid worm hole. */
                rectCount++;
            }
#else
            wormHoles[rectCount++] = area->rect;
#endif
        }

        /* Advance to next area. */
        area = area->next;

        if ((rectCount == 16) || ((area == NULL) && (rectCount > 0)))
        {
            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("  WORMHOLE:");

                for (gctUINT i = 0; i < rectCount; i++)
                {
                    LOGD("    [%d,%d,%d,%d]",
                         wormHoles[i].left,
                         wormHoles[i].top,
                         wormHoles[i].right,
                         wormHoles[i].bottom);
                }
            }

            /* Perform a Clear. */
            gcmONERROR(
                gco2D_Clear(Context->engine,
                            rectCount,
                            wormHoles,
                            0x00000000,
                            0xCC,
                            0xCC,
                            Display->format));

            /* Reset rect count. */
            rectCount = 0U;
        }
    }

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}


gctBOOL
_IsForceOpaque(
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 LayerIndex
    )
{
    gctBOOL forceOpaque = gcvFALSE;
    hwcLayer * layer = &Display->layers[LayerIndex];

    /* Check if need force opaque. */
    if (!layer->opaque && (Area != NULL))
    {
        gctINT32 k;

        for (k = LayerIndex - 1; k >= 0; k--)
        {
            if (Area->owners & (1U << k))
            {
                break;
            }
        }

        hwcLayer * under = &Display->layers[k];

        if (Area->owners & (1U << k))
        {
            if (under->compositionType == HWC_OVERLAY)
            {
                /* Force opaque if the layer under current layer is
                 * OVERLAY. */
                /* TODO: Same optimization can be applied for worm-hole
                 * and clear-hole. */
                forceOpaque = gcvTRUE;
            }
        }
    }

    return forceOpaque;
}


/* Single-source blit. */
gceSTATUS
_Blit(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Index,
    IN gctUINT32 Dim,
    IN gcsRECT * ClipRect
    )
{
    gceSTATUS status;

    hwcBuffer * target = Display->target;

    hwcLayer * layer = &Display->layers[Index];


    /***************************************************************************
    ** Dim detection.
    */

    /* We are handling DIM differently as last hwcomposer version.
     * DIM alpha 255 is special because it is actually a clear. This 'solid dim'
     * is handled in 'DIM Optimization' section in hwcSet.
     *
     * For normal DIM case, it is doing DIM lie,
     *
     *   CsAs = (0, 0, 0, alpha)
     *
     *   Cd' = Cs + Cd * (1 - As)
     *   Ad' = As + Ad * (1 - As)
     *
     * That is,
     *   Cd' = Cd * (1 - As) = Cd * (1 - alpha)
     *
     * So, can easily 'premultiply' a (1 - alpha) value for each layers before
     * this DIM layer and skip the DIM layer.
     * This optimization will reduce a layer!
     */

    /***************************************************************************
    ** Setup Source.
    */

    /* Setup source index. */
    gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, 0U));

    /* Setup alpha blending. */
    if (layer->opaque || _IsForceOpaque(Display, Area, Index))
    {
        /* The layer is at the very bottom in this area, and it needs
         * alpha blending. Where is the alpha blending target then?
         * This is actually another type of 'WormHole' and target area
         * should be cleared as [0,0,0,0] before blitting this layer.
         * But we can easily disable alpha blending to get the same
         * result. */
        gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));
    }

    else
    {
        gcmONERROR(
            gco2D_EnableAlphaBlendAdvanced(Context->engine,
                                           layer->srcAlphaMode,
                                           layer->dstAlphaMode,
                                           layer->srcGlobalAlphaMode,
                                           layer->dstGlobalAlphaMode,
                                           layer->srcFactorMode,
                                           layer->dstFactorMode));
    }

    /* Setup premultiply. */
    if (Dim < 0xFF)
    {
        gctUINT32 srcGlobal = (layer->srcGlobalAlpha * Dim) >> 8;

        /* DIM on color channel of src global. */
        srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
        srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

        /* Dim optimization. */
        gcmONERROR(
            gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                               layer->srcPremultSrcAlpha,
                                               layer->dstPremultDstAlpha,
                                               gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
                                               layer->dstDemultDstAlpha));

        gcmONERROR(
            gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                               srcGlobal));

        gcmONERROR(
            gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                               0xFFFFFFFF));
    }

    else
    {
        gctUINT32 srcGlobal = layer->srcGlobalAlpha;
        srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
        srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

        gcmONERROR(
            gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                               layer->srcPremultSrcAlpha,
                                               layer->dstPremultDstAlpha,
                                               layer->srcPremultGlobalMode,
                                               layer->dstDemultDstAlpha));

        gcmONERROR(
            gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                               srcGlobal));

        gcmONERROR(
            gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                               0xFFFFFFFF));
    }


    /***************************************************************************
    ** Setup Target.
    */

    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &target->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));


    /***************************************************************************
    ** Start Single-Source Blit(Clear).
    */

    if (layer->source != gcvNULL)
    {
        if (layer->filter)
        {
            gcsRECT srcRect;
            gcsRECT dstRect;
            gcsRECT subRect;
            gctINT  hKernel, vKernel;
            gctBOOL hMirror, vMirror;
            gceSURF_ROTATION srcFlip = gcvSURF_0_DEGREE;
            gceSURF_ROTATION rotation;

            /* Translate to dest rotation. */
            switch (layer->rotation)
            {
            case gcvSURF_0_DEGREE:
            default:
                hKernel = layer->hKernel;
                vKernel = layer->vKernel;
                hMirror = layer->hMirror;
                vMirror = layer->vMirror;
                rotation = gcvSURF_0_DEGREE;

                srcRect = layer->srcRect;
                dstRect = layer->dstRect;
                subRect = *ClipRect;
                break;

            case gcvSURF_90_DEGREE:
                hKernel = layer->vKernel;
                vKernel = layer->hKernel;
                hMirror = layer->vMirror;
                vMirror = layer->hMirror;
                rotation = gcvSURF_270_DEGREE;

                srcRect.left   = layer->width - layer->srcRect.bottom;
                srcRect.top    = layer->srcRect.left;
                srcRect.right  = layer->width - layer->srcRect.top;
                srcRect.bottom = layer->srcRect.right;

                dstRect.left   = Display->res.bottom - layer->dstRect.bottom;
                dstRect.top    = layer->dstRect.left;
                dstRect.right  = Display->res.bottom - layer->dstRect.top;
                dstRect.bottom = layer->dstRect.right;

                subRect.left   = Display->res.bottom - ClipRect->bottom;
                subRect.top    = ClipRect->left;
                subRect.right  = Display->res.bottom - ClipRect->top;
                subRect.bottom = ClipRect->right;
                break;

            case gcvSURF_180_DEGREE:
                hKernel = layer->hKernel;
                vKernel = layer->vKernel;
                hMirror = layer->hMirror;
                vMirror = layer->vMirror;
                rotation = gcvSURF_180_DEGREE;

                srcRect.left   = layer->width  - layer->srcRect.right;
                srcRect.top    = layer->height - layer->srcRect.bottom;
                srcRect.right  = layer->width  - layer->srcRect.left;
                srcRect.bottom = layer->height - layer->srcRect.top;

                dstRect.left   = Display->res.right  - layer->dstRect.right;
                dstRect.top    = Display->res.bottom - layer->dstRect.bottom;
                dstRect.right  = Display->res.right  - layer->dstRect.left;
                dstRect.bottom = Display->res.bottom - layer->dstRect.top;

                subRect.left   = Display->res.right  - ClipRect->right;
                subRect.top    = Display->res.bottom - ClipRect->bottom;
                subRect.right  = Display->res.right  - ClipRect->left;
                subRect.bottom = Display->res.bottom - ClipRect->top;
                break;

            case gcvSURF_270_DEGREE:
                hKernel = layer->vKernel;
                vKernel = layer->hKernel;
                hMirror = layer->vMirror;
                vMirror = layer->hMirror;
                rotation = gcvSURF_90_DEGREE;

                srcRect.left   = layer->srcRect.top;
                srcRect.top    = layer->height - layer->srcRect.right;
                srcRect.right  = layer->srcRect.bottom;
                srcRect.bottom = layer->height - layer->srcRect.left;

                dstRect.left   = layer->dstRect.top;
                dstRect.top    = Display->res.right - layer->dstRect.right;
                dstRect.right  = layer->dstRect.bottom;
                dstRect.bottom = Display->res.right - layer->dstRect.left;

                subRect.left   = ClipRect->top;
                subRect.top    = Display->res.right - ClipRect->right;
                subRect.right  = ClipRect->bottom;
                subRect.bottom = Display->res.right - ClipRect->left;

                break;
            }

            if (!Context->mirrorExt && (hMirror || vMirror))
            {
                gcsRECT tmpRect = srcRect;

                if (hMirror && vMirror)
                {
                    /* Use 180 degree rotation to simulate. */
                    srcFlip = gcvSURF_180_DEGREE;
                    srcRect.left   = layer->width  - tmpRect.right;
                    srcRect.top    = layer->height - tmpRect.bottom;
                    srcRect.right  = layer->width  - tmpRect.left;
                    srcRect.bottom = layer->height - tmpRect.top;
                }
                else if (hMirror)
                {
                    /* Use FLIP_X to simulate. */
                    srcFlip = gcvSURF_FLIP_X;
                    srcRect.left   = layer->width  - tmpRect.right;
                    srcRect.right  = layer->width  - tmpRect.left;
                }
                else /* if (vMirror) */
                {
                    /* Use FLIP_Y to simulate. */
                    srcFlip = gcvSURF_FLIP_Y;
                    srcRect.top    = layer->height - tmpRect.bottom;
                    srcRect.bottom = layer->height - tmpRect.top;
                }

                /* Disable source mirror. */
                hMirror = vMirror = 0;
            }

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("  FILTER: layer[%d] addr=0x%08x dim=%d,rot=%d,mirror=%d,%d,kernel=%d,%d:"
                     " [%d,%d,%d,%d] => [%d,%d,%d,%d] clip[%d,%d,%d,%d]",
                     Index,
                     layer->addresses[0],
                     Dim,
                     rotation,
                     layer->hMirror,
                     layer->vMirror,
                     hKernel,
                     vKernel,
                     srcRect.left,
                     srcRect.top,
                     srcRect.right,
                     srcRect.bottom,
                     dstRect.left,
                     dstRect.top,
                     dstRect.right,
                     dstRect.bottom,
                     subRect.left,
                     subRect.top,
                     subRect.right,
                     subRect.bottom);
            }

            /* Compute subRect again dstRect. */
            subRect.left   -= dstRect.left;
            subRect.top    -= dstRect.top;
            subRect.right  -= dstRect.left;
            subRect.bottom -= dstRect.top;

            /* Update mirror. */
            gcmONERROR(
                gco2D_SetBitBlitMirror(Context->engine, hMirror, vMirror));

            /* Use filterBlit to blit YUV source if YUV blit not supported. */
            /* Set kernel size. */
            gcmONERROR(gco2D_SetKernelSize(Context->engine, hKernel, vKernel));

            gcmONERROR(gco2D_SetFilterType(Context->engine, gcvFILTER_SYNC));

            /* Trigger filter blit. */
            gcmONERROR(
                gco2D_FilterBlitEx2(Context->engine,
                                    layer->addresses,
                                    layer->addressNum,
                                    layer->strides,
                                    layer->strideNum,
                                    layer->tiling,
                                    layer->format,
                                    srcFlip,
                                    layer->width,
                                    layer->height,
                                    &srcRect,
                                    &target->physical,
                                    1U,
                                    &Display->stride,
                                    1U,
                                    Display->tiling,
                                    Display->format,
                                    rotation,
                                    Display->res.right,
                                    Display->res.bottom,
                                    &dstRect,
                                    &subRect));
        }

        else if (layer->stretch)
        {
            gcsRECT srcRect;
            gcsRECT dstRect;

            /* Set source address. */
            gcmONERROR(
                gcoSURF_Set2DSource(layer->source, layer->rotation));

            /* Setup mirror. */
            gcmONERROR(
                gco2D_SetBitBlitMirror(Context->engine,
                                   layer->hMirror,
                                   layer->vMirror));

            if (layer->dstRect.left != ClipRect->left)
            {
                /* Adjust left coordinate. */
                gctINT multiple = (ClipRect->left - layer->dstRect.left)
                                / layer->dstXStep;

                srcRect.left = layer->srcRect.left + layer->srcXStep * multiple;
                dstRect.left = layer->dstRect.left + layer->dstXStep * multiple;
            }

            else
            {
                srcRect.left = layer->srcRect.left;
                dstRect.left = layer->dstRect.left;
            }

            if (layer->dstRect.right != ClipRect->right)
            {
                /* Adjust right coordinate. */
                gctINT multiple = (ClipRect->right -layer->dstRect.left + layer->dstXStep - 1)
                               / layer->dstXStep;

                srcRect.right = layer->srcRect.left + layer->srcXStep * multiple;
                dstRect.right = layer->dstRect.left + layer->dstXStep * multiple;
            }

            else
            {
                srcRect.right = layer->srcRect.right;
                dstRect.right = layer->dstRect.right;
            }

            if (layer->dstRect.top != ClipRect->top)
            {
                /* Adjust top coordinate. */
                gctINT multiple = (ClipRect->top - layer->dstRect.top)
                                / layer->dstYStep;

                srcRect.top = layer->srcRect.top + layer->srcYStep * multiple;
                dstRect.top = layer->dstRect.top + layer->dstYStep * multiple;
            }

            else
            {
                srcRect.top = layer->srcRect.top;
                dstRect.top = layer->dstRect.top;
            }

            if (layer->dstRect.bottom != ClipRect->bottom)
            {
                /* Adjust bottom coordinate. */
                gctINT multiple = (ClipRect->bottom - layer->dstRect.top + layer->dstYStep - 1)
                                / layer->dstYStep;

                srcRect.bottom = layer->srcRect.top + layer->srcYStep * multiple;
                dstRect.bottom = layer->dstRect.top + layer->dstYStep * multiple;
            }

            else
            {
                srcRect.bottom = layer->srcRect.bottom;
                dstRect.bottom = layer->dstRect.bottom;
            }

            /* Set source rect. */
            gcmONERROR(gco2D_SetSource(Context->engine, &srcRect));

            /* Update stretch factors. */
            gcmONERROR(
                gco2D_SetStretchFactors(Context->engine,
                                        layer->hFactor,
                                        layer->vFactor));

            /* Set clip rectangle. */
            gcmONERROR(gco2D_SetClipping(Context->engine, ClipRect));

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("  STRETCH: layer[%d] addr=0x%08x dim=%d,rot=%d,mirror=%d,%d:"
                     " [%d,%d,%d,%d] => [%d,%d,%d,%d] clip[%d,%d,%d,%d]",
                     Index,
                     layer->addresses[0],
                     Dim,
                     layer->rotation,
                     layer->hMirror,
                     layer->vMirror,
                     srcRect.left,
                     srcRect.top,
                     srcRect.right,
                     srcRect.bottom,
                     dstRect.left,
                     dstRect.top,
                     dstRect.right,
                     dstRect.bottom,
                     ClipRect->left,
                     ClipRect->top,
                     ClipRect->right,
                     ClipRect->bottom);
            }

            /* StretchBlit. */
            gcmONERROR(
                gco2D_StretchBlit(Context->engine,
                                  1U,
                                  &dstRect,
                                  0xCC,
                                  0xCC,
                                  Display->format));

            /* Reset clipping to full screen. */
            gcmONERROR(gco2D_SetClipping(Context->engine, &Display->res));
        }

        else
        {
            gcsRECT srcRect;

            /* Set source address. */
            gcmONERROR(
                gcoSURF_Set2DSource(layer->source, layer->rotation));

            /* Setup mirror. */
            gcmONERROR(
                gco2D_SetBitBlitMirror(Context->engine,
                                   layer->hMirror,
                                   layer->vMirror));

            /* Move source rect to corresponding rect. */
            gctINT dl = layer->dstRect.left   - ClipRect->left;
            gctINT dt = layer->dstRect.top    - ClipRect->top;
            gctINT dr = layer->dstRect.right  - ClipRect->right;
            gctINT db = layer->dstRect.bottom - ClipRect->bottom;

            if (layer->hMirror)
            {
                gctINT tl = dl;

                dl = -dr;
                dr = -tl;
            }

            if (layer->vMirror)
            {
                gctINT tt = dt;

                dt = -db;
                db = -tt;
            }

            /* Compute source rectangle. */
            srcRect.left   = layer->srcRect.left   - dl;
            srcRect.top    = layer->srcRect.top    - dt;
            srcRect.right  = layer->srcRect.right  - dr;
            srcRect.bottom = layer->srcRect.bottom - db;

            /* Set source rect. */
            gcmONERROR(gco2D_SetSource(Context->engine, &srcRect));

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("  BLIT: layer[%d] addr=0x%08x dim=%d,rot=%d,mirror=%d,%d:"
                     " [%d,%d,%d,%d] => [%d,%d,%d,%d]",
                     Index,
                     layer->addresses[0],
                     Dim,
                     layer->rotation,
                     layer->hMirror,
                     layer->vMirror,
                     srcRect.left,
                     srcRect.top,
                     srcRect.right,
                     srcRect.bottom,
                     ClipRect->left,
                     ClipRect->top,
                     ClipRect->right,
                     ClipRect->bottom);
            }

            /* Do bit blit. */
            gcmONERROR(
                gco2D_Blit(Context->engine,
                           1U,
                           ClipRect,
                           0xCC,
                           0xCC,
                           Display->format));
        }
    }

    else if (layer->opaque)
    {
        if (Context->dumpCompose & DUMP_OPERATIONS)
        {
            LOGD("  CLEAR: layer[%d]: color32=0x%08x => [%d,%d,%d,%d]",
                 Index,
                 layer->color32,
                 ClipRect->left,
                 ClipRect->top,
                 ClipRect->right,
                 ClipRect->bottom);
        }

        /* Do clear. */
        gcmONERROR(
            gco2D_Clear(Context->engine,
                        1U,
                        ClipRect,
                        layer->color32,
                        0xCC,
                        0xCC,
                        Display->format));
    }

    else
    {
        gcmONERROR(
            gco2D_LoadSolidBrush(Context->engine,
                                 /* This should not be taken. */
                                 gcvSURF_UNKNOWN,
                                 gcvTRUE,
                                 layer->color32,
                                 ~0ULL));

        if (Context->dumpCompose & DUMP_OPERATIONS)
        {
            LOGD("  BLIT: layer[%d]: color32=0x%08x => [%d,%d,%d,%d]",
                 Index,
                 layer->color32,
                 ClipRect->left,
                 ClipRect->top,
                 ClipRect->right,
                 ClipRect->bottom);
        }

        /* Do bit blit. */
        gcmONERROR(
            gco2D_Blit(Context->engine,
                       1U,
                       ClipRect,
                       0xF0,
                       0xF0,
                       Display->format));
    }

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}


/* Multi-source blit. */
gceSTATUS
_MultiSourceBlit(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Indices[8],
    IN gctUINT32 Dims[8],
    IN gctUINT32 Count,
    IN gcsRECT * EigenRect,
    IN gcsRECT * ClipRect
    )
{
    gceSTATUS status;

    hwcBuffer * target = Display->target;

    gctUINT32 clearMask  = 0U;
    gctUINT32 sourceMask = 0U;
    gctUINT32 sourceNum  = 0U;

    /* Blit rectangle, which is the same rectangle on source and dest.
     * This rectangle is actually clipRect against a new 'initial point' */
    gcsRECT blitRect;

    /* Surface size, which is the same size on source and dest. */
    gctUINT32 width;
    gctUINT32 height;

    /* Target physical address. */
    gctUINT32 dstAddress;
    gceSURF_ROTATION dstRotation;

    if (EigenRect->left   < 0)    { EigenRect->left    = 0; }
    if (EigenRect->top    < 0)    { EigenRect->top     = 0; }
    if (EigenRect->right  < 0)    { EigenRect->right   = 0; }
    if (EigenRect->bottom < 0)    { EigenRect->bottom  = 0; }

    if (!Context->msSourceMirror)
    {
        hwcLayer * layer = &Display->layers[Indices[0]];

        dstRotation = layer->source == gcvNULL ? gcvSURF_0_DEGREE
                    : layer->vMirror ? gcvSURF_FLIP_Y
                    : layer->hMirror ? gcvSURF_FLIP_X
                    :  gcvSURF_0_DEGREE;
    }

    else
    {
        /* Do not need flip if Source-Mirror feature exists. */
        dstRotation = gcvSURF_0_DEGREE;
    }

    if (dstRotation == gcvSURF_0_DEGREE && !Context->msSourceRotation)
    {
        gceSURF_ROTATION rot0 = Display->layers[Indices[0]].rotation;

        if (rot0 != gcvSURF_0_DEGREE)
        {
            /* Transfer to dest rotation. */
            switch (rot0)
            {
            case gcvSURF_90_DEGREE:
                dstRotation = gcvSURF_270_DEGREE;
                break;
            case gcvSURF_270_DEGREE:
                dstRotation = gcvSURF_90_DEGREE;
                break;
            default:
                dstRotation = gcvSURF_180_DEGREE;
                break;
            }
        }
    }

    /***************************************************************************
    ** Determine blit rectangle and surface size.
    */

    /*  Blit rectangle is the clipRect against a new 'initial point'
     *
     *    orig 'initial point'
     *   [+]--------------------------------------...
     *    | \
     *    |   \
     *    |     \  new 'initial point'
     *    |     [+]-------------------------+
     *    |      |        ^                 |
     *    |      |        T (270 deg)       |
     *    |      |        v                 |
     *    |      |     +--------------+     |
     *    |      |     | blit rect    |     |
     *    |      |<-L->|              |<-R->|
     *    |      |(0   |              |(180 |
     *    |      | deg)|              | deg)|
     *    |      |     +--------------+     |
     *    |      |        ^                 |
     *    |      |        B (90 deg)        |
     *    |      |        v                 |
     *    |      +--------------------------+
     *    .
     *    .
     *    .
     *
     * L, T, R, B: are eigenvalues represented by eigen rectangle.
     */

    /*
     * The blitRect is calculated for source rotation,
     * move it to dst rotation.
     */
    switch (dstRotation)
    {
    case gcvSURF_FLIP_X:
        blitRect.left   = EigenRect->right;
        blitRect.top    = EigenRect->top;
        blitRect.right  = EigenRect->right + (ClipRect->right  - ClipRect->left);
        blitRect.bottom = EigenRect->top   + (ClipRect->bottom - ClipRect->top);
        break;

    case gcvSURF_FLIP_Y:
        blitRect.left   = EigenRect->left;
        blitRect.top    = EigenRect->bottom;
        blitRect.right  = EigenRect->left   + (ClipRect->right  - ClipRect->left);
        blitRect.bottom = EigenRect->bottom + (ClipRect->bottom - ClipRect->top);
        break;

    case gcvSURF_90_DEGREE:
        blitRect.left   = EigenRect->top;
        blitRect.top    = EigenRect->right;
        blitRect.right  = EigenRect->top    + (ClipRect->bottom - ClipRect->top);
        blitRect.bottom = EigenRect->right  + (ClipRect->right  - ClipRect->left);
        break;

    case gcvSURF_270_DEGREE:
        blitRect.left   = EigenRect->bottom;
        blitRect.top    = EigenRect->left;
        blitRect.right  = EigenRect->bottom + (ClipRect->bottom - ClipRect->top);
        blitRect.bottom = EigenRect->left   + (ClipRect->right  - ClipRect->left);
        break;

    case gcvSURF_180_DEGREE:
        blitRect.left   = EigenRect->right;
        blitRect.top    = EigenRect->bottom;
        blitRect.right  = EigenRect->right  + (ClipRect->right  - ClipRect->left);
        blitRect.bottom = EigenRect->bottom + (ClipRect->bottom - ClipRect->top);
        break;

    default:
        blitRect.left   = EigenRect->left;
        blitRect.top    = EigenRect->top;
        blitRect.right  = EigenRect->left + (ClipRect->right  - ClipRect->left);
        blitRect.bottom = EigenRect->top  + (ClipRect->bottom - ClipRect->top);

        break;
    }

    /* Calculate surface size. */
    width  = ClipRect->right - ClipRect->left
           + EigenRect->left + EigenRect->right;

    height = ClipRect->bottom - ClipRect->top
           + EigenRect->top + EigenRect->bottom;

    /***************************************************************************
    ** Setup target.
    */

    /* Compute offset to dest 'initial point'. */
    gctINT dx = ClipRect->left - EigenRect->left;
    gctINT dy = ClipRect->top  - EigenRect->top;

    /* Move target physical to dest initial point. */
    dstAddress = target->physical
               + Display->stride * dy
               + Display->bytesPerPixel * dx;

    /* Disable dest mirror. */
    gcmONERROR(
        gco2D_SetBitBlitMirror(Context->engine,
                               gcvFALSE,
                               gcvFALSE));

    /* Setup target. */
    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &dstAddress,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               dstRotation,
                               width,
                               height));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));

    if (Context->dumpCompose & DUMP_OPERATIONS)
    {
        LOGD("  MULTI-SOURCE: eigenRect[%d,%d,%d,%d] blitRect[%d,%d,%d,%d]",
             EigenRect->left,
             EigenRect->top,
             EigenRect->right,
             EigenRect->bottom,
             blitRect.left,
             blitRect.top,
             blitRect.right,
             blitRect.bottom);

        LOGD("   ClipRect[%d,%d,%d,%d] rot=%d,dx=%d,dy=%d: target=0x%08x->0x%08x",
             ClipRect->left,
             ClipRect->top,
             ClipRect->right,
             ClipRect->bottom,
             dstRotation,
             dx, dy,
             Display->target->physical,
             dstAddress);
    }

    /***************************************************************************
    ** Setup color source(s) and Start Blit.
    */

    for (gctUINT32 j = 0; j < Count; j++)
    {
        /* Get shortcuts. */
        hwcLayer * layer = &Display->layers[Indices[j]];

        /* Setup source index. */
        gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, sourceNum));

        /* Setup source. */
        if (layer->source != gcvNULL)
        {
            /* Source physical address offset. */
            gctUINT32 addresses[3];

            /* Source size. */
            gctINT srcWidth;
            gctINT srcHeight;

            /* Source rotation. */
            gceSURF_ROTATION rotation;

            /* Deltas. */
            gctINT dl;
            gctINT dt;
            gctINT dr;
            gctINT db;

            /* Compute deltas. */
            dl = layer->orgDest.left   - ClipRect->left;
            dt = layer->orgDest.top    - ClipRect->top;
            dr = layer->orgDest.right  - ClipRect->right;
            db = layer->orgDest.bottom - ClipRect->bottom;

            if (layer->hMirror)
            {
                gctINT tl = dl;

                dl = -dr;
                dr = -tl;
            }

            if (layer->vMirror)
            {
                gctINT tt = dt;

                dt = -db;
                db = -tt;
            }

            /* Compute offset to source initial point. */
            switch (layer->rotation)
            {
            case gcvSURF_0_DEGREE:
            default:
                dx = layer->orgRect.left - dl - EigenRect->left;
                dy = layer->orgRect.top  - dt - EigenRect->top;

                srcWidth  = width;
                srcHeight = height;
                break;

            case gcvSURF_90_DEGREE:
                dx = layer->orgRect.left + db - EigenRect->bottom;
                dy = layer->orgRect.top  - dl - EigenRect->left;

                srcWidth  = height;
                srcHeight = width;
                break;

            case gcvSURF_180_DEGREE:
                dx = layer->orgRect.left + dr - EigenRect->right;
                dy = layer->orgRect.top  + db - EigenRect->bottom;

                srcWidth  = width;
                srcHeight = height;
                break;

            case gcvSURF_270_DEGREE:
                dx = layer->orgRect.left - dt - EigenRect->top;
                dy = layer->orgRect.top  + dr - EigenRect->right;

                srcWidth  = height;
                srcHeight = width;
                break;
            }

            /* Get original source physical addresses. */
            if (layer->yuv)
            {
                switch (layer->format)
                {
                case gcvSURF_YUY2:
                case gcvSURF_UYVY:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + 2 * dx;
                    break;

                case gcvSURF_NV12:
                case gcvSURF_NV21:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + 1 * dx;

                    addresses[1] = layer->addresses[1]
                                 + layer->strides[1] * dy / 2
                                 + 1 * dx;
                    break;

                case gcvSURF_NV16:
                case gcvSURF_NV61:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + 1 * dx;

                    addresses[1] = layer->addresses[1]
                                 + layer->strides[1] * dy
                                 + 1 * dx;
                    break;

                case gcvSURF_YV12:
                case gcvSURF_I420:
                default:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + 1 * dx;

                    addresses[1] = layer->addresses[1]
                                 + layer->strides[1] * dy / 2
                                 + 1 * dx / 2;

                    addresses[2] = layer->addresses[2]
                                 + layer->strides[2] * dy / 2
                                 + 1 * dx / 2;
                    break;
                }
            }

            else
            {
                switch (layer->tiling)
                {
                case gcvMULTI_SUPERTILED:
                    /* Multi-supertiled source has two addresses. */
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy / 2
                                 + layer->bytesPerPixel * (dx << 6);

                    addresses[1] = layer->addresses[1]
                                 + layer->strides[1] * dy / 2
                                 + layer->bytesPerPixel * (dx << 6);
                    break;

                case gcvSUPERTILED:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + layer->bytesPerPixel * (dx << 6);
                    break;

                case gcvMULTI_TILED:
                    /* Multi-tiled source has two addresses. */
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy / 2
                                 + layer->bytesPerPixel * (dx << 2);

                    addresses[1] = layer->addresses[1]
                                 + layer->strides[1] * dy / 2
                                 + layer->bytesPerPixel * (dx << 2);
                    break;

                case gcvTILED:
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + layer->bytesPerPixel * (dx << 2);
                    break;

                default:
                    /* RGB surfaces will have only one plane. */
                    addresses[0] = layer->addresses[0]
                                 + layer->strides[0] * dy
                                 + layer->bytesPerPixel * dx;
                    break;
                }
            }

            if ((dstRotation == gcvSURF_90_DEGREE)
            ||  (dstRotation == gcvSURF_180_DEGREE)
            ||  (dstRotation == gcvSURF_270_DEGREE)
            )
            {
                /* Transfer to dest rotation. */
                rotation = gcvSURF_0_DEGREE;
            }
            else
            {
                rotation = layer->rotation;
            }


            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("   layer[%d]: dim=%d,rot=%d,mirror=%d,%d dx=%d,dy=%d: addr=0x%08x->0x%08x",
                     Indices[j],
                     Dims[j],
                     rotation,
                     layer->hMirror,
                     layer->vMirror,
                     dx, dy,
                     layer->addresses[0],
                     addresses[0]);
            }

            if (Context->msSourceMirror)
            {
                /* Set mirror for current source. */
                gcmONERROR(
                    gco2D_SetBitBlitMirror(Context->engine,
                                           layer->hMirror,
                                           layer->vMirror));
            }

            /* Setup source. */
            gcmONERROR(
                gco2D_SetGenericSource(Context->engine,
                                       addresses,
                                       layer->addressNum,
                                       layer->strides,
                                       layer->strideNum,
                                       layer->tiling,
                                       layer->format,
                                       rotation,
                                       srcWidth,
                                       srcHeight));

            /* Can not support source tile status because address moved. */
            gcmONERROR(
                gco2D_SetSourceTileStatus(Context->engine,
                                          gcv2D_TSC_DISABLE,
                                          gcvSURF_UNKNOWN,
                                          0,
                                          ~0U));

            /* Set source rect (equal to what for dest). */
            gcmONERROR(gco2D_SetSource(Context->engine, &blitRect));

            /* Set ROP. */
            gcmONERROR(gco2D_SetROP(Context->engine, 0xCC, 0xCC));

            /* Try to set source address of previous no-source layers to
               current layer. */
            for (gctUINT i = 0; clearMask != 0U; i++)
            {
                if ((clearMask & (1 << i)) == 0) continue;

                /* Operate on previous layer. */
                gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, i));

                /* Update address to current. */
                gcmONERROR(
                    gco2D_SetGenericSource(Context->engine,
                                           addresses,
                                           layer->addressNum,
                                           layer->strides,
                                           layer->strideNum,
                                           layer->tiling,
                                           layer->format,
                                           rotation,
                                           srcWidth,
                                           srcHeight));

                /* Can not support source tile status because address moved. */
                gcmONERROR(
                    gco2D_SetSourceTileStatus(Context->engine,
                                              gcv2D_TSC_DISABLE,
                                              gcvSURF_UNKNOWN,
                                              0,
                                              ~0U));

                /* Remove mask. */
                clearMask &= ~(1 << i);

                /* Roll back. */
                gcmONERROR(
                    gco2D_SetCurrentSourceIndex(Context->engine, sourceNum));
            }
        }

        else
        {
            /* Color source is still needed if uses patthen only. We set dummy
             * color source to framebuffer target. */
            gcmONERROR(
                gco2D_SetGenericSource(Context->engine,
                                       &target->prev->physical,
                                       1U,
                                       &Display->stride,
                                       1U,
                                       Display->tiling,
                                       gcvSURF_A8,
                                       gcvSURF_0_DEGREE,
                                       width,
                                       height));

            /* Previous display buffer tile status. */
            gcmONERROR(
                gco2D_SetSourceTileStatus(Context->engine,
                                          target->prev->tsConfig,
                                          Display->format,
                                          0,
                                          target->prev->tsPhysical));

            /* Set source rect (equal to what for dest). */
            gcmONERROR(gco2D_SetSource(Context->engine, &blitRect));

            gcmONERROR(
                gco2D_LoadSolidBrush(Context->engine,
                                     /* This should not be taken. */
                                     gcvSURF_UNKNOWN,
                                     gcvTRUE,
                                     layer->color32,
                                     ~0ULL));

            /* Set ROP: use pattern only. */
            gcmONERROR(gco2D_SetROP(Context->engine, 0xF0, 0xCC));

            /* Record clearMask. */
            clearMask |= (1 << sourceNum);

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("   layer[%d]: color32=0x%08x",
                     Indices[j], layer->color32);
            }
        }

        /* Setup alpha blending. */
        if (layer->opaque ||
            ((j == 0) && _IsForceOpaque(Display, Area, Indices[0])))
        {
            /* Disable alpha blending. */
            gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));
        }

        else
        {
            gcmONERROR(
                gco2D_EnableAlphaBlendAdvanced(Context->engine,
                                               layer->srcAlphaMode,
                                               layer->dstAlphaMode,
                                               layer->srcGlobalAlphaMode,
                                               layer->dstGlobalAlphaMode,
                                               layer->srcFactorMode,
                                               layer->dstFactorMode));
        }

        /* Setup premultiply. */
        if (Dims [j] < 0xFF)
        {
            gctUINT32 srcGlobal = (layer->srcGlobalAlpha * Dims[j]) >> 8;

            /* DIM on color channel of src global. */
            srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
            srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

            /* Dim optimization. */
            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                                   layer->srcPremultSrcAlpha,
                                                   layer->dstPremultDstAlpha,
                                                   gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
                                                   layer->dstDemultDstAlpha));

            gcmONERROR(
                gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                                   srcGlobal));

            gcmONERROR(
                gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                                   0xFFFFFFFF));
        }

        else
        {
            gctUINT32 srcGlobal = layer->srcGlobalAlpha;
            srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
            srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                                   layer->srcPremultSrcAlpha,
                                                   layer->dstPremultDstAlpha,
                                                   layer->srcPremultGlobalMode,
                                                   layer->dstDemultDstAlpha));

            gcmONERROR(
                gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                                   srcGlobal));

            gcmONERROR(
                gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                                   0xFFFFFFFF));
        }

        if (Context->multiSourceBltV2)
        {
            /* Set target rectangle for each source. */
            gcmONERROR(gco2D_SetTargetRect(Context->engine, ClipRect));
        }

        /* Append mask to sourceMask. */
        sourceMask = ((sourceMask << 1U) | 1U);
        sourceNum++;
    }


    /***************************************************************************
    ** Trigger Multi-Source Blit.
    */

    /* Set clip rectangle. */
    gcmONERROR(gco2D_SetClipping(Context->engine, &blitRect));

    gcmONERROR(
        gco2D_MultiSourceBlit(Context->engine, sourceMask, &blitRect, 1U));

    /* Reset clipping to full screen. */
    gcmONERROR(gco2D_SetClipping(Context->engine, &Display->res));

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}

gceSTATUS
_MultiSourceBlit2(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwcArea    * Area,
    IN gctUINT32 Indices[8],
    IN gctUINT32 Dims[8],
    IN gctUINT32 Count,
    IN gcsRECT * ClipRect
    )
{
    gceSTATUS status;

    hwcBuffer * target = Display->target;

    gctUINT32 clearMask  = 0U;
    gctUINT32 sourceMask = 0U;
    gctUINT32 sourceNum  = 0U;

    gcmASSERT(Context->msSourceMirror);
    gcmASSERT(Context->msSourceRotation);

    /* Setup target. */
    gcmONERROR(
        gco2D_SetGenericTarget(Context->engine,
                               &target->physical,
                               1U,
                               &Display->stride,
                               1U,
                               Display->tiling,
                               Display->format,
                               gcvSURF_0_DEGREE,
                               Display->res.right,
                               Display->res.bottom));

    /* Target display buffer tile status. */
    gcmONERROR(
        gco2D_SetTargetTileStatus(Context->engine,
                                  target->tsConfig,
                                  Display->format,
                                  0,
                                  target->tsPhysical));

    if (Context->dumpCompose & DUMP_OPERATIONS)
    {
        LOGD("  MULTI-SOURCE: ClipRect[%d,%d,%d,%d]",
             ClipRect->left,
             ClipRect->top,
             ClipRect->right,
             ClipRect->bottom);
    }

    /***************************************************************************
    ** Setup color source(s) and Start Blit.
    */

    for (gctUINT32 j = 0; j < Count; j++)
    {
        /* Get shortcuts. */
        hwcLayer * layer = &Display->layers[Indices[j]];

        /* Setup source index. */
        gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, sourceNum));

        /* Setup source. */
        if (layer->source != gcvNULL)
        {
            /* Source clip rect. */
            if (Context->multiSourceBltV2)
            {
                /* Set source rect (equal to what for dest). */
                gcmONERROR(gco2D_SetSource(Context->engine, &layer->srcRect));

                /* Set target rectangle for each source. */
                gcmONERROR(gco2D_SetTargetRect(Context->engine, &layer->dstRect));

                /* Set clip rectangle. */
                gcmONERROR(gco2D_SetClipping(Context->engine, ClipRect));

                if (Context->dumpCompose & DUMP_OPERATIONS)
                {
                    LOGD("   layer[%d]: [%d,%d,%d,%d]->[%d,%d,%d,%d] dim=%d,rot=%d,mirror=%d,%d: addr=0x%08x",
                         Indices[j],
                         layer->srcRect.left,
                         layer->srcRect.top,
                         layer->srcRect.right,
                         layer->srcRect.bottom,
                         layer->dstRect.left,
                         layer->dstRect.top,
                         layer->dstRect.right,
                         layer->dstRect.bottom,
                         Dims[j],
                         layer->rotation,
                         layer->hMirror,
                         layer->vMirror,
                         layer->addresses[0]);
                }
            }
            else
            {
                gcsRECT srcRect;

                /* Deltas. */
                gctINT dl;
                gctINT dt;
                gctINT dr;
                gctINT db;

                /* Compute deltas. */
                dl = layer->dstRect.left   - ClipRect->left;
                dt = layer->dstRect.top    - ClipRect->top;
                dr = layer->dstRect.right  - ClipRect->right;
                db = layer->dstRect.bottom - ClipRect->bottom;

                if (layer->hMirror)
                {
                    gctINT tl = dl;

                    dl = -dr;
                    dr = -tl;
                }

                if (layer->vMirror)
                {
                    gctINT tt = dt;

                    dt = -db;
                    db = -tt;
                }

                if (layer->stretch)
                {
                    /* Must have multi-source blit v2. */
                    dl *= layer->xScale;
                    dt *= layer->yScale;
                    dr *= layer->xScale;
                    db *= layer->yScale;
                }

                /* Compute source rectangle. */
                srcRect.left   = layer->srcRect.left   - dl;
                srcRect.top    = layer->srcRect.top    - dt;
                srcRect.right  = layer->srcRect.right  - dr;
                srcRect.bottom = layer->srcRect.bottom - db;

                /* Set source rect (equal to what for dest). */
                gcmONERROR(gco2D_SetSource(Context->engine, &srcRect));

                if (Context->dumpCompose & DUMP_OPERATIONS)
                {
                    LOGD("   layer[%d]: rect[%d,%d,%d,%d] dim=%d,rot=%d,mirror=%d,%d: addr=0x%08x",
                         Indices[j],
                         srcRect.left,
                         srcRect.top,
                         srcRect.right,
                         srcRect.bottom,
                         Dims[j],
                         layer->rotation,
                         layer->hMirror,
                         layer->vMirror,
                         layer->addresses[0]);
                }
            }

            /* Set mirror for current source. */
            gcmONERROR(
                gco2D_SetBitBlitMirror(Context->engine,
                                       layer->hMirror,
                                       layer->vMirror));

            /* Setup source. */
            gcmONERROR(
                gcoSURF_Set2DSource(layer->source, layer->rotation));

            /* Set ROP. */
            gcmONERROR(gco2D_SetROP(Context->engine, 0xCC, 0xCC));

            /* Try to set source address of previous no-source layers to
               current layer. */
            for (gctUINT i = 0; clearMask != 0U; i++)
            {
                if ((clearMask & (1 << i)) == 0) continue;

                /* Operate on previous layer. */
                gcmONERROR(gco2D_SetCurrentSourceIndex(Context->engine, i));

                /* Update address to current. */
                gcmONERROR(
                    gcoSURF_Set2DSource(layer->source, layer->rotation));

                /* Remove mask. */
                clearMask &= ~(1 << i);

                /* Roll back. */
                gcmONERROR(
                    gco2D_SetCurrentSourceIndex(Context->engine, sourceNum));
            }
        }

        else
        {
            /* Color source is still needed if uses patthen only. We set dummy
             * color source to framebuffer target. */
            gcmONERROR(
                gco2D_SetGenericSource(Context->engine,
                                       &target->prev->physical,
                                       1U,
                                       &Display->stride,
                                       1U,
                                       Display->tiling,
                                       gcvSURF_A8,
                                       gcvSURF_0_DEGREE,
                                       layer->width,
                                       layer->height));

            /* Previous display buffer tile status. */
            gcmONERROR(
                gco2D_SetSourceTileStatus(Context->engine,
                                          target->prev->tsConfig,
                                          Display->format,
                                          0,
                                          target->prev->tsPhysical));

            /* Set source rect (equal to what for dest). */
            gcmONERROR(gco2D_SetSource(Context->engine, ClipRect));

            /* v2: Set target rectangle for each source. */
            gcmONERROR(gco2D_SetTargetRect(Context->engine, ClipRect));

            /* v2: Set clip rectangle. */
            gcmONERROR(gco2D_SetClipping(Context->engine, ClipRect));

            gcmONERROR(
                gco2D_LoadSolidBrush(Context->engine,
                                     /* This should not be taken. */
                                     gcvSURF_UNKNOWN,
                                     gcvTRUE,
                                     layer->color32,
                                     ~0ULL));

            /* Set ROP: use pattern only. */
            gcmONERROR(gco2D_SetROP(Context->engine, 0xF0, 0xCC));

            /* Record clearMask. */
            clearMask |= (1 << sourceNum);

            if (Context->dumpCompose & DUMP_OPERATIONS)
            {
                LOGD("   layer[%d]: color32=0x%08x",
                     Indices[j], layer->color32);
            }
        }

        /* Setup alpha blending. */
        if (layer->opaque ||
            ((j == 0) && _IsForceOpaque(Display, Area, Indices[0])))
        {
            /* Disable alpha blending. */
            gcmONERROR(gco2D_DisableAlphaBlend(Context->engine));
        }

        else
        {
            gcmONERROR(
                gco2D_EnableAlphaBlendAdvanced(Context->engine,
                                               layer->srcAlphaMode,
                                               layer->dstAlphaMode,
                                               layer->srcGlobalAlphaMode,
                                               layer->dstGlobalAlphaMode,
                                               layer->srcFactorMode,
                                               layer->dstFactorMode));
        }

        /* Setup premultiply. */
        if (Dims [j] < 0xFF)
        {
            gctUINT32 srcGlobal = (layer->srcGlobalAlpha * Dims[j]) >> 8;

            /* DIM on color channel of src global. */
            srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
            srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

            /* Dim optimization. */
            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                                   layer->srcPremultSrcAlpha,
                                                   layer->dstPremultDstAlpha,
                                                   gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
                                                   layer->dstDemultDstAlpha));

            gcmONERROR(
                gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                                   srcGlobal));

            gcmONERROR(
                gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                                   0xFFFFFFFF));
        }

        else
        {
            gctUINT32 srcGlobal = layer->srcGlobalAlpha;
            srcGlobal = srcGlobal | (srcGlobal << 8) | (srcGlobal << 16);
            srcGlobal = srcGlobal | (layer->srcGlobalAlpha << 24);

            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(Context->engine,
                                                   layer->srcPremultSrcAlpha,
                                                   layer->dstPremultDstAlpha,
                                                   layer->srcPremultGlobalMode,
                                                   layer->dstDemultDstAlpha));

            gcmONERROR(
                gco2D_SetSourceGlobalColorAdvanced(Context->engine,
                                                   srcGlobal));

            gcmONERROR(
                gco2D_SetTargetGlobalColorAdvanced(Context->engine,
                                                   0xFFFFFFFF));
        }

        /* Append mask to sourceMask. */
        sourceMask = ((sourceMask << 1U) | 1U);
        sourceNum++;
    }


    /***************************************************************************
    ** Trigger Multi-Source Blit.
    */

    if (Context->multiSourceBltV2)
    {
        /* Target rectangles are separated given. */
        gcmONERROR(
            gco2D_MultiSourceBlit(Context->engine, sourceMask, gcvNULL, 0U));
    }
    else
    {
        /* Set clip rectangle. */
        gcmONERROR(gco2D_SetClipping(Context->engine, ClipRect));

        gcmONERROR(
            gco2D_MultiSourceBlit(Context->engine, sourceMask, ClipRect, 1U));
    }

    /* Reset clipping to full screen. */
    gcmONERROR(gco2D_SetClipping(Context->engine, &Display->res));

    return gcvSTATUS_OK;

OnError:
    LOGE("Failed in %s: status=%d", __FUNCTION__, status);
    return status;
}

