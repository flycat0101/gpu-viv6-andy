/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/**
 * \file gc_dfb_raster.c
 */

#include <directfb.h>
#include <direct/messages.h>
#include <direct/types.h>
#include <gfx/convert.h>

#include "gc_dfb.h"
#include "gc_dfb_raster.h"
#include "gc_dfb_utils.h"

static gceSTATUS
_BlitROPBlend( void    *drv,
               void    *dev,
               gcsRECT *srect,
               gcsRECT *drect );

#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)
static gceSTATUS
_Blit2NoMultiSrcFeature( void    *drv,
                         void    *dev,
                         gcsRECT *srect,
                         gcsRECT *srect2,
                         gcsRECT *drect );
#endif

D_DEBUG_DOMAIN( Gal_2D, "Gal/2D", "2D functions" );


/**
 * Real blitting function.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 *
 * \return    gcvSTATUS_OK: accelerated by hardware
 * \return    otherwise: not accelerated by hardware
 */
gceSTATUS
_Blit( GalDriverData *vdrv,
       GalDeviceData *vdev )
{
    gceSTATUS status = gcvSTATUS_OK;

    D_ASSERT( vdrv != NULL );
    D_ASSERT( vdev != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "vdrv: %p, vdev: %p\n",
                   vdrv, vdev );

    do {
        if (!vdev->hw_2d_pe20 && vdrv->blit_blend && vdrv->dst_blend_no_alpha) {
            gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                   vdev,
                                                   vdrv->pending_src_rects,
                                                   vdrv->pending_num,
                                                   vdrv->pending_dst_rects,
                                                   vdrv->pending_num ));

        }
        else {
            /* Blit. */
            gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                          vdrv->pending_num,
                                          vdrv->pending_src_rects,
                                          vdrv->pending_dst_rects,
                                          vdrv->blit_fg_rop,
                                          vdrv->blit_bg_rop,
                                          vdrv->dst_native_format ));
        }
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to blit. status: %d\n", status );
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}


/**
 * Real draw line function.
 *
 * \param [in]    vdrv: driver specific data
 * \param [in]    vdev: device specific data
 *
 * \return    gcvSTATUS_OK: accelerated by hardware
 * \return    otherwise: not accelerated by hardware
 */
gceSTATUS
_Line( GalDriverData *vdrv,
       GalDeviceData *vdev )
{
    gceSTATUS status = gcvSTATUS_OK;

    D_ASSERT( vdrv  != NULL );
    D_ASSERT( vdev  != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "vdrv: %p, vdev: %p\n",
                   vdrv, vdev );

    do {
        gcmERR_BREAK(gco2D_ColorLine( vdrv->engine_2d,
                                      vdrv->pending_num,
                                      vdrv->pending_dst_rects,
                                      vdrv->pending_colors[0],
                                      vdrv->draw_fg_rop,
                                      vdrv->draw_bg_rop,
                                      vdrv->dst_native_format ));
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to draw line. status: %d\n", status );
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}

/**
 * Fill rectangles.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle
 * \param [in]    num: number of rectangles
 *
 * \return    gcvSTATUS_OK: accelerated by hardware
 * \return    otherwise: not accelerated by hardware
 */
gceSTATUS
_FillRectangles( GalDriverData *vdrv,
                 GalDeviceData *vdev )
{
    gceSTATUS status = gcvSTATUS_OK;

    D_ASSERT( vdrv != NULL );
    D_ASSERT( vdev != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "vdrv: %p, vdev: %p\n",
                   vdrv, vdev );

    do {
        if (!vdev->hw_2d_pe20) {
            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                D_DEBUG_AT( Gal_2D, "Using galStretchBlitDstNoAlpha work.\n" );
                gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                       vdev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       vdrv->pending_dst_rects,
                                                       vdrv->pending_num ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                D_DEBUG_AT( Gal_2D,
                            "vdrv->pool_src_rect (%u, %u, %u, %u)\n",
                            GAL_RECTANGLE_VALS(&vdrv->pool_src_rect) );

                /* Calculate the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      0, 0 ));

                D_DEBUG_AT( Gal_2D,
                            "gco2D_StretchBlit\n"
                            "vdrv->draw_fg_rop: 0x%08X\n"
                            "vdrv->draw_bg_rop: 0x%08X\n"
                            "vdrv->dst_native_format: %u\n",
                            vdrv->draw_fg_rop,
                            vdrv->draw_bg_rop,
                            vdrv->dst_native_format );

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                vdrv->pending_num,
                                                vdrv->pending_dst_rects,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
                ((vdrv->src_pmp_glb_mode == gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE &&
                 !vdrv->need_glb_alpha_multiply) ||
                 vdrv->src_format == DSPF_A8))
            {
                gcmERR_BREAK(gco2D_LoadSolidBrush(
                    vdrv->engine_2d,
                    vdrv->dst_native_format,
                    (vdrv->dst_native_format!=gcvSURF_A8R8G8B8),
                    vdrv->pending_colors[0],
                    0
                    ));

                gcmERR_BREAK(gco2D_Blit(
                    vdrv->engine_2d,
                    vdrv->pending_num,
                    vdrv->pending_dst_rects,
                    0xF0, 0xF0,
                    vdrv->dst_native_format
                    ));
            }
            else
            {
                gcmERR_BREAK(gco2D_Clear( vdrv->engine_2d,
                                          vdrv->pending_num,
                                          vdrv->pending_dst_rects,
                                          vdrv->pending_colors[0],
                                          vdrv->draw_fg_rop,
                                          vdrv->draw_bg_rop,
                                          vdrv->dst_native_format ));
            }
        }
    } while (0);

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}

/**
 * Flush the pending primitives.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 *
 * \return    gcvSTATUS_OK: accelerated by hardware
 * \return    otherwise: not accelerated by hardware
 */
gceSTATUS
_FlushPendingPrimitives( GalDriverData *vdrv,
                         GalDeviceData *vdev )
{
    gceSTATUS status = gcvSTATUS_OK;

    D_ASSERT( vdrv != NULL );
    D_ASSERT( vdev != NULL );

    do {
        if (vdrv->pending_num > 0) {
            if (vdrv->pending_type == GAL_PENDING_BLIT) {
                gcmERR_BREAK(_Blit( vdrv, vdev ));
            }
            else if (vdrv->pending_type == GAL_PENDING_LINE) {
                gcmERR_BREAK(_Line( vdrv, vdev ));
            }
            else if (vdrv->pending_type == GAL_PENDING_FILL) {
                gcmERR_BREAK(_FillRectangles( vdrv, vdev ));
            }

            vdrv->pending_num  = 0;
            vdrv->pending_type = GAL_PENDING_NONE;
        }
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to flush the pendings!!!!!!!!!! status: %d\n", status );
    }

    return status;
}

#if GAL_SURFACE_COMPRESSED
static gceSTATUS
_Compressed2Tmp2Compressed( GalDriverData *vdrv,
                            GalDeviceData *vdev,
                            gcsRECT_PTR src_rect,
                            gcsRECT_PTR dst_rect )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT srect, drect;
    unsigned int   hor_factor, ver_factor;

    D_ASSERT( vdrv != NULL );
    D_ASSERT( vdev != NULL );

    do {
        if (vdrv->tmp_dst_phys_addr == gcvINVALID_ADDRESS)
        {
            printf("No dst tmp surface.\n");
            break;
        }

        /* Src->Tmp */
        gcmERR_BREAK(gco2D_SetSourceTileStatus(
            vdrv->engine_2d,
            vdrv->src_tile_status,
            vdrv->src_native_format,
            0,
            vdrv->src_tile_status_addr
            ));

        gcmERR_BREAK(gco2D_SetGenericSource(vdrv->engine_2d,
                                            vdrv->src_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->src_pitch,
                                            1U,
                                            vdrv->src_tiling,
                                            vdrv->src_native_format,
                                            gcvSURF_0_DEGREE,
                                            vdrv->src_aligned_width,
                                            vdrv->src_aligned_height));

        gcmERR_BREAK(gco2D_SetTargetTileStatus(
            vdrv->engine_2d,
            gcv2D_TSC_DISABLE,
            gcvSURF_UNKNOWN,
            0,
            gcvINVALID_ADDRESS
            ));

        gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                            &vdrv->tmp_dst_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->tmp_dst_pitch,
                                            1U,
                                            vdrv->dst_tiling,
                                            vdrv->dst_native_format,
                                            gcvSURF_0_DEGREE,
                                            vdrv->tmp_dst_aligned_width,
                                            vdrv->tmp_dst_aligned_height));

        drect.left = drect.top = 0;
        drect.right = dst_rect->right - dst_rect->left;
        drect.bottom = dst_rect->bottom - dst_rect->top;

        gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                      src_rect ));

        gcmERR_BREAK(gco2D_SetClipping(vdrv->engine_2d, &drect));

        /* Calculate the stretch factors. */
        gcmERR_BREAK(gal_get_stretch_factors( src_rect,
                                              &drect,
                                              &hor_factor,
                                              &ver_factor ));

        /* Program the stretch factors. */
        gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                              hor_factor,
                                              ver_factor ));

        gcmERR_BREAK(gco2D_SetBitBlitMirror( vdrv->engine_2d,
                                             vdrv->backup_hor_mirror,
                                             vdrv->backup_ver_mirror ));

        gcmERR_BREAK(gco2D_DisableAlphaBlend(vdrv->engine_2d));

        gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE ));

        gcmERR_BREAK(gco2D_SetTransparencyAdvanced( vdrv->engine_2d,
                                                    gcv2D_OPAQUE,
                                                    gcv2D_OPAQUE,
                                                    gcv2D_OPAQUE ));

        gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                        1,
                                        &drect,
                                        0xCC,
                                        0xCC,
                                        vdrv->dst_native_format ));

        /* Tmp -> Dst */
        gcmERR_BREAK(gco2D_SetSourceTileStatus(
            vdrv->engine_2d,
            gcv2D_TSC_DISABLE,
            gcvSURF_UNKNOWN,
            0,
            gcvINVALID_ADDRESS
            ));

        gcmERR_BREAK(gco2D_SetGenericSource(vdrv->engine_2d,
                                            &vdrv->tmp_dst_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->tmp_dst_pitch,
                                            1U,
                                            vdrv->dst_tiling,
                                            vdrv->dst_native_format,
                                            gcvSURF_0_DEGREE,
                                            vdrv->tmp_dst_aligned_width,
                                            vdrv->tmp_dst_aligned_height));

        gcmERR_BREAK(gco2D_SetTargetTileStatus(
            vdrv->engine_2d,
            vdrv->dst_tile_status,
            vdrv->dst_native_format,
            0,
            vdrv->dst_tile_status_addr
            ));

        gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                            &vdrv->dst_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->dst_pitch,
                                            1U,
                                            vdrv->dst_tiling,
                                            vdrv->dst_native_format,
                                            gcvSURF_0_DEGREE,
                                            vdrv->dst_aligned_width,
                                            vdrv->dst_aligned_height));

        srect = drect;

        gcmERR_BREAK(gco2D_SetSource(vdrv->engine_2d, &srect));
        gcmERR_BREAK(gco2D_SetClipping(vdrv->engine_2d, dst_rect));

        if (vdrv->backup_blended)
        {
            gcmERR_BREAK(gco2D_EnableAlphaBlend(
                vdrv->engine_2d,
                vdrv->backup_src_global_alpha_value,
                vdrv->backup_dst_global_alpha_value,
                vdrv->backup_src_alpha_mode,
                vdrv->backup_dst_alpha_mode,
                vdrv->backup_src_global_alpha_mode,
                vdrv->backup_dst_global_alpha_mode,
                vdrv->backup_drc_factor_mode,
                vdrv->backup_dst_factor_mode,
                vdrv->backup_src_color_mode,
                vdrv->backup_dst_color_mode));
        }

        gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                         vdrv->backup_src_premultiply_src_alpha,
                                                         vdrv->backup_dst_premultiply_dst_alpha,
                                                         vdrv->backup_src_premultiply_global_mode,
                                                         vdrv->backup_dst_demultiply_dst_alpha ));

        gcmERR_BREAK(gco2D_SetTransparencyAdvanced( vdrv->engine_2d,
                                                    vdrv->src_trans_mode,
                                                    vdrv->dst_trans_mode,
                                                    vdrv->pat_trans_mode ));

        gcmERR_BREAK(gco2D_SetBitBlitMirror( vdrv->engine_2d,
                                             gcvFALSE,
                                             gcvFALSE ));

        gcmERR_BREAK(gco2D_Blit( vdrv->engine_2d,
                                 1,
                                 dst_rect,
                                 vdrv->blit_fg_rop,
                                 vdrv->blit_bg_rop,
                                 vdrv->dst_native_format ));

        vdrv->dst_tile_status = gcv2D_TSC_2D_COMPRESSED;

        /* Restore status. */
        gcmERR_BREAK(gco2D_SetBitBlitMirror( vdrv->engine_2d,
                                             vdrv->backup_hor_mirror,
                                             vdrv->backup_ver_mirror ));
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to clear dst and its tsb!!!!!!!!!! status: %d\n", status );
    }

    return status;
}

static gceSTATUS
_ClearDstAndTileStatusBuffer( GalDriverData *vdrv,
                              GalDeviceData *vdev,
                              bool compressed)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsRECT rect;

    D_ASSERT( vdrv != NULL );
    D_ASSERT( vdev != NULL );


    do {
        if (!compressed)
        {
            gcmERR_BREAK(gco2D_DisableAlphaBlend(vdrv->engine_2d));

            gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                             gcv2D_COLOR_MULTIPLY_DISABLE,
                                                             gcv2D_COLOR_MULTIPLY_DISABLE,
                                                             gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                             gcv2D_COLOR_MULTIPLY_DISABLE ));

            gcmERR_BREAK(gco2D_SetTransparencyAdvanced( vdrv->engine_2d,
                                                        gcv2D_OPAQUE,
                                                        gcv2D_OPAQUE,
                                                        gcv2D_OPAQUE ));

            gcmERR_BREAK(gco2D_SetBitBlitMirror( vdrv->engine_2d,
                                                 gcvFALSE,
                                                 gcvFALSE ));

            gcmERR_BREAK(gco2D_SetSourceTileStatus(
                vdrv->engine_2d,
                vdrv->dst_tile_status,
                vdrv->dst_native_format,
                0,
                vdrv->dst_tile_status_addr
                ));

            gcmERR_BREAK(gco2D_SetGenericSource(vdrv->engine_2d,
                                                &vdrv->dst_phys_addr,
                                                1U,
                                                (unsigned int*) &vdrv->dst_pitch,
                                                1U,
                                                vdrv->dst_tiling,
                                                vdrv->dst_native_format,
                                                gcvSURF_0_DEGREE,
                                                vdrv->dst_aligned_width,
                                                vdrv->dst_aligned_height));

            gcmERR_BREAK(gco2D_SetTargetTileStatus(
                vdrv->engine_2d,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                gcvINVALID_ADDRESS
                ));

            gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                                &vdrv->tmp_dst_phys_addr,
                                                1U,
                                                (unsigned int*) &vdrv->tmp_dst_pitch,
                                                1U,
                                                vdrv->dst_tiling,
                                                vdrv->dst_native_format,
                                                gcvSURF_0_DEGREE,
                                                vdrv->tmp_dst_aligned_width,
                                                vdrv->tmp_dst_aligned_height));

            rect.left = rect.top = 0;
            rect.right = vdrv->tmp_dst_aligned_width;
            rect.bottom = vdrv->tmp_dst_aligned_height;

            gcmERR_BREAK(gco2D_SetSource(vdrv->engine_2d, &rect));
            gcmERR_BREAK(gco2D_SetClipping(vdrv->engine_2d, &rect));

            gcmERR_BREAK(gco2D_Blit( vdrv->engine_2d,
                                     1,
                                     &rect,
                                     0xCC, 0xCC,
                                     vdrv->dst_native_format ));

            {
                gctUINT w, h, s;
                w = vdrv->dst_aligned_width / 8;
                h = vdrv->dst_aligned_height / 8;
                s = w * 4;

                gcmERR_BREAK(gco2D_SetTargetTileStatus(
                    vdrv->engine_2d,
                    gcv2D_TSC_DISABLE,
                    gcvSURF_UNKNOWN,
                    0,
                    gcvINVALID_ADDRESS
                    ));

                /* Clear tile status buffer. */
                gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                                    &vdrv->dst_tile_status_addr,
                                                    1U,
                                                    &s,
                                                    1U,
                                                    vdrv->dst_tiling,
                                                    vdrv->dst_native_format,
                                                    gcvSURF_0_DEGREE,
                                                    w, h));

                rect.left = rect.top = 0;
                rect.right = w;
                rect.bottom = h;

                gcmERR_BREAK(gco2D_SetClipping(vdrv->engine_2d, &rect));

                gcmERR_BREAK(gco2D_Clear( vdrv->engine_2d,
                                          1,
                                          &rect,
                                          vdrv->dst_native_format==gcvSURF_A8R8G8B8 ? 0x88888888:0x66666666,
                                          0xCC, 0xCC,
                                          vdrv->dst_native_format ));

                gcmERR_BREAK(gco2D_Commit( vdrv->engine_2d, false ));
            }

            gcmERR_BREAK(gco2D_SetSourceTileStatus(
                vdrv->engine_2d,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                gcvINVALID_ADDRESS
                ));

            gcmERR_BREAK(gco2D_SetGenericSource(vdrv->engine_2d,
                                                &vdrv->tmp_dst_phys_addr,
                                                1U,
                                                (unsigned int*) &vdrv->tmp_dst_pitch,
                                                1U,
                                                vdrv->dst_tiling,
                                                vdrv->dst_native_format,
                                                gcvSURF_0_DEGREE,
                                                vdrv->tmp_dst_aligned_width,
                                                vdrv->tmp_dst_aligned_height));

            gcmERR_BREAK(gco2D_SetTargetTileStatus(
                vdrv->engine_2d,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                gcvINVALID_ADDRESS
                ));

            gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                                &vdrv->dst_phys_addr,
                                                1U,
                                                (unsigned int*) &vdrv->dst_pitch,
                                                1U,
                                                vdrv->dst_tiling,
                                                vdrv->dst_native_format,
                                                gcvSURF_0_DEGREE,
                                                vdrv->dst_aligned_width,
                                                vdrv->dst_aligned_height));

            rect.left = rect.top = 0;
            rect.right = vdrv->dst_aligned_width;
            rect.bottom = vdrv->dst_aligned_height;

            gcmERR_BREAK(gco2D_SetSource(vdrv->engine_2d, &rect));
            gcmERR_BREAK(gco2D_SetClipping(vdrv->engine_2d, &rect));

            gcmERR_BREAK(gco2D_Blit( vdrv->engine_2d,
                                     1,
                                     &rect,
                                     0xCC, 0xCC,
                                     vdrv->dst_native_format ));

            if (vdrv->backup_blended)
            {
                gcmERR_BREAK(gco2D_EnableAlphaBlend(
                    vdrv->engine_2d,
                    vdrv->backup_src_global_alpha_value,
                    vdrv->backup_dst_global_alpha_value,
                    vdrv->backup_src_alpha_mode,
                    vdrv->backup_dst_alpha_mode,
                    vdrv->backup_src_global_alpha_mode,
                    vdrv->backup_dst_global_alpha_mode,
                    vdrv->backup_drc_factor_mode,
                    vdrv->backup_dst_factor_mode,
                    vdrv->backup_src_color_mode,
                    vdrv->backup_dst_color_mode));
            }

            gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                             vdrv->backup_src_premultiply_src_alpha,
                                                             vdrv->backup_dst_premultiply_dst_alpha,
                                                             vdrv->backup_src_premultiply_global_mode,
                                                             vdrv->backup_dst_demultiply_dst_alpha ));

            gcmERR_BREAK(gco2D_SetTransparencyAdvanced( vdrv->engine_2d,
                                                        vdrv->src_trans_mode,
                                                        vdrv->dst_trans_mode,
                                                        vdrv->pat_trans_mode ));

            gcmERR_BREAK(gco2D_SetBitBlitMirror( vdrv->engine_2d,
                                                 vdrv->backup_hor_mirror,
                                                 vdrv->backup_ver_mirror ));
        }


        gcmERR_BREAK(gco2D_SetSourceTileStatus(
            vdrv->engine_2d,
            vdrv->src_tile_status,
            vdrv->src_native_format,
            0,
            vdrv->src_tile_status_addr
            ));

        gcmERR_BREAK(gco2D_SetGenericSource(vdrv->engine_2d,
                                            vdrv->src_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->src_pitch,
                                            1U,
                                            vdrv->src_tiling,
                                            vdrv->src_native_format,
                                            vdrv->src_rotation,
                                            vdrv->src_aligned_width,
                                            vdrv->src_aligned_height));

        vdrv->dst_tile_status = compressed ? gcv2D_TSC_2D_COMPRESSED : gcv2D_TSC_DISABLE;

        if (compressed)
        {
            gcmERR_BREAK(gco2D_SetTargetTileStatus(
                vdrv->engine_2d,
                gcv2D_TSC_2D_COMPRESSED,
                vdrv->dst_native_format,
                0,
                vdrv->dst_tile_status_addr
                ));
        }
        else
        {
            gcmERR_BREAK(gco2D_SetTargetTileStatus(
                vdrv->engine_2d,
                gcv2D_TSC_DISABLE,
                gcvSURF_UNKNOWN,
                0,
                gcvINVALID_ADDRESS
                ));
        }

        gcmERR_BREAK(gco2D_SetGenericTarget(vdrv->engine_2d,
                                            &vdrv->dst_phys_addr,
                                            1U,
                                            (unsigned int*) &vdrv->dst_pitch,
                                            1U,
                                            vdrv->dst_tiling,
                                            vdrv->dst_native_format,
                                            vdrv->dst_rotation,
                                            vdrv->dst_aligned_width,
                                            vdrv->dst_aligned_height));
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to clear dst and its tsb!!!!!!!!!! status: %d\n", status );
    }

    return status;
}
#endif

/**
 * Stretch blit function.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    srect: the rectangle of the source surface
 * \param [in]    drect: the rectangle of the    target surface
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool galStretchBlit( void         *drv,
                     void         *dev,
                     DFBRectangle *srect,
                     DFBRectangle *drect )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;

    gceSTATUS      status = gcvSTATUS_OK;
    gcsRECT        src_rect, dst_rect, dst_sub_rect, clip_rect;
    unsigned int   hor_factor, ver_factor;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    unsigned int   horfactor, verfactor;


    D_ASSERT( drv   != NULL );
    D_ASSERT( dev   != NULL );
    D_ASSERT( srect != NULL );
    D_ASSERT( drect != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p\n"
                   "srect->x: %d, srect->y: %d, srect->w: %d, srect->h: %d\n"
                   "drect->x: %d, drect->y: %d, drect->w: %d, drect->h: %d\n",
                   drv, dev,
                   srect->x, srect->y, srect->w, srect->h,
                   drect->x, drect->y, drect->w, drect->h );

#if GAL_SURFACE_COMPRESSED
    if (srect->w == drect->w && srect->h == drect->h)
    {
        D_DEBUG_EXIT( Gal_2D, "\n" );
        return galBlit(drv, dev, srect, drect->x, drect->y);
    }
#endif

    do {
        /* Check the threshold for small object rendering. */
        if (vdev->turn_on_threshold &&
            !vdev->disable_threshold &&
            (srect->w == drect->w) && (srect->h == drect->h) &&
            (srect->w * srect->h < vdev->threshold_value)) {
            return false;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* No pending for galStretchBlit. */
        gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression &&
            vdrv->per_process_compression &&
            vdrv->dst_tile_status != gcv2D_TSC_DISABLE)
        {
            gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
            vdrv->flush = true;
        }
#endif

        if ((vdrv->src_is_yuv_format || vdrv->smooth_scale) &&
            !vdrv->ckey_w) {
            src_rect.left   = srect->x;
            src_rect.top     = srect->y;
            src_rect.right  = src_rect.left + srect->w;
            src_rect.bottom = src_rect.top + srect->h;

            dst_rect.left   = drect->x;
            dst_rect.top    = drect->y;
            dst_rect.right  = dst_rect.left + drect->w;
            dst_rect.bottom = dst_rect.top + drect->h;

#if GAL_SURFACE_COMPRESSED
            if (vdev->hw_2d_compression &&
                !vdrv->per_process_compression &&
                vdrv->dst_tile_status == gcv2D_TSC_2D_COMPRESSED)
            {
                gcmERR_BREAK(_Compressed2Tmp2Compressed(vdrv, vdev, &src_rect, &dst_rect));
            }
            else
#endif
            {
                /* Clip it with dest sub rectangle. */
                clip_rect.left   = MAX(vdrv->clip.left, dst_rect.left);
                clip_rect.right  = MIN(vdrv->clip.right, dst_rect.right);
                clip_rect.top    = MAX(vdrv->clip.top, dst_rect.top);
                clip_rect.bottom = MIN(vdrv->clip.bottom, dst_rect.bottom);

                if (dst_rect.left < 0)
                {
                    dst_sub_rect.left   = 0;
                    dst_sub_rect.right  = clip_rect.right - dst_rect.left;
                }
                else
                {
                    dst_sub_rect.left   = clip_rect.left - dst_rect.left;
                    dst_sub_rect.right  = dst_sub_rect.left + (clip_rect.right - clip_rect.left);
                }
                if (dst_rect.top < 0)
                {
                    dst_sub_rect.top    = 0;
                    dst_sub_rect.bottom = clip_rect.bottom - dst_rect.top;
                }
                else
                {
                    dst_sub_rect.top    = clip_rect.top  - dst_rect.top;
                    dst_sub_rect.bottom = dst_sub_rect.top + (clip_rect.bottom - clip_rect.top);
                }

                D_DEBUG_AT( Gal_2D, "src_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src_rect) );
                D_DEBUG_AT( Gal_2D, "dst_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_rect) );
                D_DEBUG_AT( Gal_2D, "dst_sub_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_sub_rect) );

                gal_get_kernel_size(&src_rect, &dst_rect, &horfactor, &verfactor);
                gcmERR_BREAK(gco2D_SetKernelSize( vdrv->engine_2d, horfactor, verfactor));

                gcmERR_BREAK(gco2D_FilterBlit( vdrv->engine_2d,
                                               vdrv->src_phys_addr[0],
                                               vdrv->src_pitch[0],
                                               vdrv->src_phys_addr[1],
                                               vdrv->src_pitch[1],
                                               vdrv->src_phys_addr[2],
                                               vdrv->src_pitch[2],
                                               vdrv->src_native_format,
                                               vdrv->src_rotation,
                                               vdrv->src_aligned_width,
                                               &src_rect,
                                               vdrv->dst_phys_addr,
                                               vdrv->dst_pitch,
                                               vdrv->dst_native_format,
                                               vdrv->dst_rotation,
                                               vdrv->dst_aligned_width,
                                               &dst_rect,
                                               &dst_sub_rect ));
            }
        }
        else {
            if (vdrv->src_rotation == gcvSURF_0_DEGREE) {
                src_rect.left   = srect->x;
                src_rect.top    = srect->y;
                src_rect.right  = src_rect.left + srect->w;
                src_rect.bottom = src_rect.top + srect->h;
            }
            else if (vdrv->src_rotation == gcvSURF_90_DEGREE) {
                src_rect.left   = srect->y;
                src_rect.top    = vdrv->src_width - srect->x - srect->w + 1;
                src_rect.right  = src_rect.left + srect->h;
                src_rect.bottom = src_rect.top + srect->w;
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported src rotation [%d].\n", __func__, vdrv->src_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }

            if (vdrv->dst_rotation == gcvSURF_0_DEGREE) {
                dst_rect.left   = drect->x;
                dst_rect.top    = drect->y;
                dst_rect.right  = dst_rect.left + drect->w;
                dst_rect.bottom = dst_rect.top + drect->h;
            }
            else if (vdrv->dst_rotation == gcvSURF_90_DEGREE) {
                dst_rect.left   = drect->y;
                dst_rect.top    = vdrv->dst_width - drect->x - drect->w + 1;
                dst_rect.right  = dst_rect.left + drect->h;
                dst_rect.bottom = dst_rect.top + drect->w;
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported dest rotation [%d].\n", __func__, vdrv->dst_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }

            D_DEBUG_AT( Gal_2D, "src_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src_rect) );
            D_DEBUG_AT( Gal_2D, "dst_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_rect) );

            checkConvert_valid_rectangle(1, &src_rect, &dst_rect);

#if GAL_SURFACE_COMPRESSED
            if (vdev->hw_2d_compression &&
                !vdrv->per_process_compression &&
                vdrv->dst_tile_status == gcv2D_TSC_2D_COMPRESSED)
            {
                gcmERR_BREAK(_Compressed2Tmp2Compressed(vdrv, vdev, &src_rect, &dst_rect));
            }
            else
#endif
            {
                if (!vdev->hw_2d_pe20 && vdrv->blit_blend && vdrv->dst_blend_no_alpha) {
                    D_DEBUG_AT( Gal_2D, "Work for destination blending no alpha.\n" );

                    gcmERR_BREAK(galStretchBlitDstNoAlpha( drv,
                                                           dev,
                                                           &src_rect,
                                                           1,
                                                           &dst_rect,
                                                           1 ));
                }
                else {
                    /* Set source rectangle. */
                    gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                                  &src_rect ));

                    /* Calculate the stretch factors. */
                    gcmERR_BREAK(gal_get_stretch_factors( &src_rect,
                                                          &dst_rect,
                                                          &hor_factor,
                                                          &ver_factor ));

                    /* Program the stretch factors. */
                    gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                          hor_factor,
                                                          ver_factor ));

                    D_DEBUG_AT( Gal_2D,
                                "gco2D_StretchBlit\n"
                                "dst rect (%u, %u, %u, %u)\n"
                                "vdrv->blit_fg_rop: 0x%08X\n"
                                "vdrv->blit_bg_rop: 0x%08X\n"
                                "vdrv->dst_native_format: %u\n",
                                dst_rect.left,
                                dst_rect.top,
                                dst_rect.right,
                                dst_rect.bottom,
                                vdrv->blit_fg_rop,
                                vdrv->blit_bg_rop,
                                vdrv->dst_native_format );

                    gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                    1,
                                                    &dst_rect,
                                                    vdrv->blit_fg_rop,
                                                    vdrv->blit_bg_rop,
                                                    vdrv->dst_native_format ));

                }
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "StretchBlit error. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

/**
 * Fill rectangle.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galFillRectangle( void            *drv,
                  void            *dev,
                  DFBRectangle *rect )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS        status = gcvSTATUS_OK;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv  != NULL );
    D_ASSERT( dev  != NULL );
    D_ASSERT( rect != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p, rect: %p (%u,%u - %ux%u)\n",
                   drv, dev, rect,
                   DFB_RECTANGLE_VALS(rect) );

    do {
        /* Check the threshold for small object rendering. */
        if (vdev->turn_on_threshold &&
            !vdev->disable_threshold &&
            (rect->w * rect->h < vdev->threshold_value)) {
            return false;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush the pendings if primitive type was changed. */
        if (vdrv->pending_type != GAL_PENDING_FILL) {
            gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
            vdrv->pending_type = GAL_PENDING_FILL;
        }

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression &&
            vdrv->per_process_compression &&
            vdrv->dst_tile_status != gcv2D_TSC_2D_COMPRESSED &&
            vdrv->dst_tile_status_addr != gcvINVALID_ADDRESS)
        {
            gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, true));
            vdrv->flush = true;
        }
#endif

        /* Append to the pending list. */
        vdrv->pending_dst_rects[vdrv->pending_num].left   = rect->x;
        vdrv->pending_dst_rects[vdrv->pending_num].top    = rect->y;
        vdrv->pending_dst_rects[vdrv->pending_num].right  = rect->x + rect->w;
        vdrv->pending_dst_rects[vdrv->pending_num].bottom = rect->y + rect->h;

        checkConvert_valid_rectangle( 1, &(vdrv->pending_dst_rects[vdrv->pending_num]), &(vdrv->pending_dst_rects[vdrv->pending_num]) );


        if (vdev->hw_2d_pe20) {
            vdrv->pending_colors[vdrv->pending_num] = vdrv->color;
        }

        if (++vdrv->pending_num >= vdev->max_pending_num || vdrv->flush) {
            gcmERR_BREAK(_FillRectangles( vdrv, vdev ));
            vdrv->pending_num = 0;
            vdrv->flush = 0;
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to fill rectangle. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

/**
 * Fill triangle.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galFillTriangle( void         *drv,
                 void         *dev,
                 DFBTriangle *tri )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gceSTATUS        status  = gcvSTATUS_OK;
    int                 numRect = 0;
    unsigned int   hor_factor, ver_factor;
    gctINT32 delta;
    gcsRECT* drect;
    GalTriangleUnitType type;

    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );
    D_ASSERT( tri != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "numRect:%d, draw_fg_rop: %d, draw_bg_rop: %d, dst_native_format: %d\n",
                   numRect, vdrv->draw_fg_rop, vdrv->draw_bg_rop, vdrv->dst_native_format );

    do {
        if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
            (vdrv->src_pmp_glb_mode != gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE ||
             vdrv->need_glb_alpha_multiply) &&
             vdrv->src_format != DSPF_A8)
        {
            return false;
        }

        /* Scan convert triangle into lines */
        if (!vdev->hw_2d_pe20) {
            /* Use FillRectangle to fill triangle. */
            type = TRIANGLE_UNIT_RECT;
        }
        else {
            /* Use DrawLine to fill triangle. */
            type = TRIANGLE_UNIT_LINE;
        }

        /* check if the area is too large and resize it. */
        delta = gal_mod(tri->y2 - tri->y1) + 1;
        if (delta > vdrv->cur_convert_rect_num) {
            D_FREE(vdrv->convert_rect);
            vdrv->convert_rect = D_CALLOC(1, sizeof(gcsRECT) * delta);
            gcmERR_BREAK(vdrv->convert_rect ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);
            vdrv->cur_convert_rect_num = delta;
        }
        drect = vdrv->convert_rect;

        scanConvertTriangle( tri, drect, &numRect, type );

        if (numRect < 1) {
            D_DEBUG_AT( Gal_2D, "numRect < 1\n" );

            return true;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* No pending for galFillTriangle. */
        gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression)
        {
            if (vdrv->per_process_compression &&
                vdrv->dst_tile_status != gcv2D_TSC_DISABLE)
            {
                gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
            }
            else if (!vdrv->per_process_compression &&
                     vdrv->dst_tile_status == gcv2D_TSC_2D_COMPRESSED)
            {
                vdrv->draw_bg_rop = 0xAA;
            }
        }
#endif

        if (!vdev->hw_2d_pe20) {
            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                       vdev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       drect,
                                                       numRect ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                /* Calculate the stretch factors. */
                gcmERR_BREAK(gal_get_stretch_factors( &vdrv->pool_src_rect,
                                                      &drect[0],
                                                      &hor_factor,
                                                      &ver_factor ));

                /* Program the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      hor_factor,
                                                      ver_factor ));

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                numRect,
                                                drect,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            gcmERR_BREAK(gco2D_ColorLine( vdrv->engine_2d,
                                          numRect,
                                          drect,
                                          vdrv->color,
                                          vdrv->draw_fg_rop,
                                          vdrv->draw_bg_rop,
                                          vdrv->dst_native_format ));
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while(0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to fill triangle. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)

/**
 * Fill trapezoid.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    trap: the trapezoid
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galFillTrapezoid( void         *drv,
                  void         *dev,
                  DFBTrapezoid *trap )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gceSTATUS        status  = gcvSTATUS_OK;
    int                 numRect = 0;
    unsigned int   hor_factor, ver_factor;
    gctINT32 delta;
    gcsRECT* drect;
    GalTriangleUnitType type;


    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );
    D_ASSERT( trap != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "draw_fg_rop: %d, draw_bg_rop: %d, dst_native_format: %d\n",
                   vdrv->draw_fg_rop, vdrv->draw_bg_rop, vdrv->dst_native_format );


    do {
        /* Scan convert triangle into rectangles */
        type = TRIANGLE_UNIT_RECT;

        /* check if the area is too large and resize it. */
        delta = gal_mod(trap->y2 - trap->y1) + 1;
        if (delta > vdrv->cur_convert_rect_num) {
            D_FREE(vdrv->convert_rect);
            vdrv->convert_rect = D_CALLOC(1, sizeof(gcsRECT) * delta);
            gcmERR_BREAK(vdrv->convert_rect ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);
            vdrv->cur_convert_rect_num = delta;
        }
        drect = vdrv->convert_rect;

        scanConvertTrapezoid( trap, drect, &numRect, type);

        if (numRect < 1) {
            D_DEBUG_AT( Gal_2D, "numRect < 1\n" );
            return true;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* No pending for galFillTrapezoid. */
        gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression &&
            vdrv->per_process_compression &&
            vdrv->dst_tile_status != gcv2D_TSC_DISABLE)
        {
            gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
            vdrv->flush = true;
        }
#endif

        if (!vdev->hw_2d_pe20) {
            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                       vdev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       drect,
                                                       numRect ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                /* Calculate the stretch factors. */
                gcmERR_BREAK(gal_get_stretch_factors( &vdrv->pool_src_rect,
                                                      &drect[0],
                                                      &hor_factor,
                                                      &ver_factor ));

                /* Program the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      hor_factor,
                                                      ver_factor ));

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                numRect,
                                                drect,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
                ((vdrv->src_pmp_glb_mode == gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE &&
                 !vdrv->need_glb_alpha_multiply) ||
                 vdrv->src_format == DSPF_A8))
            {
                gcmERR_BREAK(gco2D_LoadSolidBrush(
                    vdrv->engine_2d,
                    vdrv->dst_native_format,
                    (vdrv->dst_native_format!=gcvSURF_A8R8G8B8),
                    vdrv->color,
                    0
                    ));

                gcmERR_BREAK(gco2D_Blit(
                    vdrv->engine_2d,
                    numRect,
                    drect,
                    0xF0, 0xF0,
                    vdrv->dst_native_format
                    ));
            }
            else
            {
                gcmERR_BREAK(gco2D_Clear( vdrv->engine_2d,
                                          numRect,
                                          drect,
                                          vdrv->color,
                                          vdrv->draw_fg_rop,
                                          vdrv->draw_bg_rop,
                                          vdrv->dst_native_format ));
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while(0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to fill trapezoid. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;

}

/**
 * Batch fill.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rects: the rectangles
 * \param [in]    num: the number of rectangles
 * \param [out]    done_num: the number of successful batchfill rectangles
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galBatchFill( void                  *drv,
              void                  *dev,
              const DFBRectangle    *rects,
              unsigned int           num,
              unsigned int          *done_num )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gceSTATUS status  = gcvSTATUS_OK;
    int numRect = 0, noTileRect = 0;
    const DFBRectangle *rect;
    static gcsRECT drects[1024];
    bool Ret = true;


    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );
    D_ASSERT( rects != NULL );
    D_ASSERT( num != 0 );
    D_ASSERT( done_num != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "num: %d, draw_fg_rop: %d, draw_bg_rop: %d, dst_native_format: %d\n",
                    num, vdrv->draw_fg_rop, vdrv->draw_bg_rop, vdrv->dst_native_format );

    if ( num > 1024 ) {
        D_DEBUG_AT( Gal_2D, "BatchFill fail: too many rectangles, just return to use software\n");
        num = *done_num = 1024;
        Ret = false;
    }

    do {

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush the pendings if primitive type was changed. */
        if (vdrv->pending_type != GAL_PENDING_FILL) {
            gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
            vdrv->pending_type = GAL_PENDING_FILL;
        }

        rect = &rects[0];
        do {
            drects[numRect].left   = rect->x;
            drects[numRect].top    = rect->y;
            drects[numRect].right  = rect->x + rect->w;
            drects[numRect].bottom = rect->y + rect->h;
            if (rect->w < 256 || rect->h < 256)
                ++noTileRect;
            ++rect;
        } while(++numRect <= num);

        checkConvert_valid_rectangle( num, drects, drects );

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression &&
            vdrv->per_process_compression &&
            vdrv->dst_tile_status != gcv2D_TSC_DISABLE &&
            noTileRect >= numRect/2)
        {
            gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
            vdrv->flush = true;
        }
#endif

        if (!vdev->hw_2d_pe20) {
            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                D_DEBUG_AT( Gal_2D, "Using galStretchBlitDstNoAlpha work.\n" );
                gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                       vdev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       drects,
                                                       num ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                D_DEBUG_AT( Gal_2D,
                            "vdrv->pool_src_rect (%u, %u, %u, %u)\n",
                            GAL_RECTANGLE_VALS(&vdrv->pool_src_rect) );

                /* Calculate the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      0, 0 ));

                D_DEBUG_AT( Gal_2D,
                            "gco2D_StretchBlit\n"
                            "vdrv->draw_fg_rop: 0x%08X\n"
                            "vdrv->draw_bg_rop: 0x%08X\n"
                            "vdrv->dst_native_format: %u\n",
                            vdrv->draw_fg_rop,
                            vdrv->draw_bg_rop,
                            vdrv->dst_native_format );

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                num,
                                                drects,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
                ((vdrv->src_pmp_glb_mode == gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE &&
                 !vdrv->need_glb_alpha_multiply) ||
                 vdrv->src_format == DSPF_A8))
            {
                gcmERR_BREAK(gco2D_LoadSolidBrush(
                    vdrv->engine_2d,
                    vdrv->dst_native_format,
                    (vdrv->dst_native_format!=gcvSURF_A8R8G8B8),
                    vdrv->color,
                    0
                    ));

                gcmERR_BREAK(gco2D_Blit(
                    vdrv->engine_2d,
                    num,
                    drects,
                    0xF0, 0xF0,
                    vdrv->dst_native_format
                    ));
            }
            else
            {
                gcmERR_BREAK(gco2D_Clear( vdrv->engine_2d,
                                          num,
                                          drects,
                                          vdrv->color,
                                          vdrv->draw_fg_rop,
                                          vdrv->draw_bg_rop,
                                          vdrv->dst_native_format ));
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;

    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to batchfill. status: %d\n", status );
        *done_num = 0;
        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return Ret;
}

#endif

/**
 * Draw rectangle.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galDrawRectangle( void            *drv,
                  void            *dev,
                  DFBRectangle *rect )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;
    gcsRECT        line[4];
    int            num;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv  != NULL );
    D_ASSERT( dev  != NULL );
    D_ASSERT( rect != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p, rect: %p\n"
                   "x: %d, y: %d, w: %d, h: %d\n",
                   drv, dev, rect,
                   rect->x, rect->y, rect->w, rect->h );

    do {
        /* Check the threshold for small object rendering. */
        if (vdev->turn_on_threshold &&
            !vdev->disable_threshold &&
            (rect->w + rect->h) * 2 < vdev->threshold_value) {
            return false;
        }

        if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
            (vdrv->src_pmp_glb_mode != gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE ||
             vdrv->need_glb_alpha_multiply) &&
             vdrv->src_format != DSPF_A8)
        {
            return false;
        }

        if (rect->w < 1 || rect->h < 1)
        {
            return true;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush the pendings if primitive type was changed. */
        if (vdrv->pending_type != GAL_PENDING_LINE) {
            gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
            vdrv->pending_type = GAL_PENDING_LINE;
        }

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression)
        {
            if (vdrv->per_process_compression &&
                vdrv->dst_tile_status != gcv2D_TSC_DISABLE)
            {
                gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
                vdrv->flush = true;
            }

            if (!vdrv->per_process_compression &&
                vdrv->dst_tile_status == gcv2D_TSC_2D_COMPRESSED)
            {
                vdrv->draw_bg_rop = 0xAA;
            }
        }
#endif

        if (!vdev->hw_2d_pe20) {
            /* Decompose rect into 4 rectangles */
            if (rect->w == 1 || rect->h == 1) {
                num = 1;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + rect->w;
                line[0].bottom = rect->y + rect->h;
            }
            else if (rect->w == 2) {
                num = 2;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + 1;
                line[0].bottom = rect->y + rect->h;

                line[1].left   = rect->x + 1;
                line[1].top    = rect->y;
                line[1].right  = rect->x + 2;
                line[1].bottom = rect->y + rect->h;
            }
            else if (rect->h == 2) {
                num = 2;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + rect->w;
                line[0].bottom = rect->y + 1;

                line[1].left   = rect->x;
                line[1].top    = rect->y + 1;
                line[1].right  = rect->x + rect->w;
                line[1].bottom = rect->y + 2;
            }
            else {
                num = 4;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + rect->w - 1;
                line[0].bottom = rect->y + 1;

                line[1].left   = rect->x + rect->w - 1;
                line[1].top    = rect->y;
                line[1].right  = rect->x + rect->w;
                line[1].bottom = rect->y + rect->h - 1;

                line[2].left   = rect->x + 1;
                line[2].top    = rect->y + rect->h - 1;
                line[2].right  = rect->x + rect->w;
                line[2].bottom = rect->y + rect->h;

                line[3].left   = rect->x;
                line[3].top    = rect->y + 1;
                line[3].right  = rect->x + 1;
                line[3].bottom = rect->y + rect->h;
            }

            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                gcmERR_BREAK(galStretchBlitDstNoAlpha( drv,
                                                       dev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       line,
                                                       num ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                /* Let GPU calc the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      0, 0 ));

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                num,
                                                line,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            int i;

            /* Decompose rect into 4 rectangles */
            if (rect->w == 1 || rect->h == 1) {
                num = 1;

                line[0].left   = rect->x;
                line[0].top    = rect->y;

                if (rect->w == 1) {
                    /* Vertical line. */
                    line[0].right  = rect->x + rect->w - 1;
                    line[0].bottom = rect->y + rect->h;
                }
                else {
                    /* Horizontal line. */
                    line[0].right  = rect->x + rect->w;
                    line[0].bottom = rect->y + rect->h - 1;
                }
            }
            else if (rect->w == 2) {
                num = 2;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x;
                line[0].bottom = rect->y + rect->h;

                line[1].left   = rect->x + 1;
                line[1].top    = rect->y;
                line[1].right  = rect->x + 1;
                line[1].bottom = rect->y + rect->h;
            }
            else if (rect->h == 2) {
                num = 2;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + rect->w;
                line[0].bottom = rect->y;

                line[1].left   = rect->x;
                line[1].top    = rect->y + 1;
                line[1].right  = rect->x + rect->w;
                line[1].bottom = rect->y + 1;
            }
            else {
                num = 4;

                line[0].left   = rect->x;
                line[0].top    = rect->y;
                line[0].right  = rect->x + rect->w - 1;
                line[0].bottom = rect->y;

                line[1].left   = rect->x + rect->w - 1;
                line[1].top    = rect->y;
                line[1].right  = rect->x + rect->w - 1;
                line[1].bottom = rect->y + rect->h - 1;

                line[2].left   = rect->x + 1;
                line[2].top    = rect->y + rect->h - 1;
                line[2].right  = rect->x + rect->w;
                line[2].bottom = rect->y + rect->h - 1;

                line[3].left   = rect->x;
                line[3].top    = rect->y + 1;
                line[3].right  = rect->x;
                line[3].bottom = rect->y + rect->h;
            }

            for (i = 0; i < num; i++)
            {
                if (checkConvert_valid_line(&line[i]))
                {
                    /* Append to the pending list. */
                    vdrv->pending_dst_rects[vdrv->pending_num].left   = line[i].left;
                    vdrv->pending_dst_rects[vdrv->pending_num].top    = line[i].top;
                    vdrv->pending_dst_rects[vdrv->pending_num].right  = line[i].right;
                    vdrv->pending_dst_rects[vdrv->pending_num].bottom = line[i].bottom;

                    vdrv->pending_colors[vdrv->pending_num] = vdrv->color;

                    ++vdrv->pending_num;
                }
            }

            if (vdrv->pending_num >= vdev->max_pending_num || vdrv->flush ) {
                gcmERR_BREAK(_Line( vdrv, vdev ));
                vdrv->pending_num = 0;
                vdrv->flush = false;
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "%s: failed. status: %d\n", __func__, status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

/**
 * Draw line.
 * We only support horizontal and vertical lines.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    line: the line to be draw
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galDrawLine( void *drv,
             void *dev,
             DFBRegion *line )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS        status = gcvSTATUS_OK;
    gcsRECT        dst_rect;
    unsigned int   hor_factor, ver_factor;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv  != NULL );
    D_ASSERT( dev  != NULL );
    D_ASSERT( line != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p, line: %p.\n"
                   "x1: %d, y1: %d, x2: %d, y2: %d.\n",
                   drv, dev, line,
                   line->x1, line->y1, line->x2, line->y2 );

    do {
        /* Check the threshold for small object rendering. */
        if (vdev->turn_on_threshold &&
            !vdev->disable_threshold &&
            ((line->x2 - line->x1) + (line->y2 - line->y1) < vdev->threshold_value)) {
            return false;
        }

        if (vdev->chipModel == gcv320 && vdev->chipRevision == 0x5007 &&
            (vdrv->src_pmp_glb_mode != gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE ||
             vdrv->need_glb_alpha_multiply) &&
             vdrv->src_format != DSPF_A8)
        {
            return false;
        }

        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush the pendings if primitive type was changed. */
        if (vdrv->pending_type != GAL_PENDING_LINE) {
            gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
            vdrv->pending_type = GAL_PENDING_LINE;
        }

#if GAL_SURFACE_COMPRESSED
        if (vdev->hw_2d_compression)
        {
            if (vdrv->per_process_compression &&
                vdrv->dst_tile_status != gcv2D_TSC_DISABLE)
            {
                gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, false));
            }
            else if (!vdrv->per_process_compression &&
                     vdrv->dst_tile_status == gcv2D_TSC_2D_COMPRESSED)
            {
                vdrv->draw_bg_rop = 0xAA;
            }
        }
#endif

        if (!vdev->hw_2d_pe20) {
            /* Horizantal or vertical only. */
            if ((line->y1 != line->y2) &&
                (line->x1 != line->x2))
            {
                return false;
            }

            dst_rect.left   = line->x1;
            dst_rect.top    = line->y1;
            dst_rect.right  = line->x2 + 1;
            dst_rect.bottom = line->y2 + 1;

            if (vdrv->draw_blend && vdrv->dst_blend_no_alpha) {
                gcmERR_BREAK(galStretchBlitDstNoAlpha( vdrv,
                                                       vdev,
                                                       &vdrv->pool_src_rect,
                                                       1,
                                                       &dst_rect,
                                                       1 ));
            }
            else {
                /* Set source rectangle. */
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d,
                                              &vdrv->pool_src_rect ));

                /* Calculate the stretch factors. */
                gcmERR_BREAK(gal_get_stretch_factors( &vdrv->pool_src_rect,
                                                      &dst_rect,
                                                      &hor_factor,
                                                      &ver_factor ));

                /* Program the stretch factors. */
                gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                      hor_factor,
                                                      ver_factor ));

                gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                                1,
                                                &dst_rect,
                                                vdrv->draw_fg_rop,
                                                vdrv->draw_bg_rop,
                                                vdrv->dst_native_format ));
            }
        }
        else {
            dst_rect.left = line->x1;
            dst_rect.top  = line->y1;

            if (line->x1 == line->x2) {
                /* Vertical line. */
                dst_rect.right  = line->x2;
                dst_rect.bottom = line->y2 + 1;
            }
            else if (line->y1 == line->y2) {
                /* Horizontal line. */
                dst_rect.right  = line->x2 + 1;
                dst_rect.bottom = line->y2;
            }
            else {
                /* Any angle. */
                dst_rect.right  = line->x2 + 1;
                dst_rect.bottom = line->y2 + 1;
            }

            D_DEBUG_AT( Gal_2D,
                        "draw_fg_rop: 0x%08X, draw_bg_rop: 0x%08X\n",
                        vdrv->draw_fg_rop,
                        vdrv->draw_bg_rop );

            if (checkConvert_valid_line(&dst_rect))
            {
                vdrv->pending_dst_rects[vdrv->pending_num].left   = dst_rect.left;
                vdrv->pending_dst_rects[vdrv->pending_num].top    = dst_rect.top;
                vdrv->pending_dst_rects[vdrv->pending_num].right  = dst_rect.right;
                vdrv->pending_dst_rects[vdrv->pending_num].bottom = dst_rect.bottom;

                vdrv->pending_colors[vdrv->pending_num] = vdrv->color;

                if (++vdrv->pending_num >= vdev->max_pending_num || vdrv->flush) {
                    gcmERR_BREAK(_Line( vdrv, vdev ));
                    vdrv->pending_num = 0;
                    vdrv->flush = false;
                }
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }


    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to draw line. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT(Gal_2D, "\n" );

    return true;
}

/**
 * Blit function.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle area on
 *                    source surface
 * \param [in]    dx: x on the target surface
 * \param [in]    dy: y on the target surface
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
bool
galBlit( void         *drv,
         void         *dev,
         DFBRectangle *rect,
         int           dx,
         int           dy )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;
    gcsRECT        dst_rect, src_rect, dst_sub_rect, clip_rect;
    int            max_pending_num;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    unsigned int   horfactor, verfactor;

    D_ASSERT( drv  != NULL );
    D_ASSERT( dev  != NULL );
    D_ASSERT( rect != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p, rect(%p): (%d, %d, %d, %d).\n"
                   "dx: %d, dy: %d.\n",
                   drv, dev, rect,
                   DFB_RECTANGLE_VALS(rect),
                   dx, dy );

    do {
        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        if (vdrv->src_is_yuv_format) {
            src_rect.left       = rect->x;
            src_rect.top        = rect->y;
            src_rect.right      = rect->x + rect->w;
            src_rect.bottom     = rect->y + rect->h;

            dst_rect.left       = dx;
            dst_rect.top        = dy;
            dst_rect.right      = dst_rect.left + rect->w;
            dst_rect.bottom     = dst_rect.top + rect->h;

            clip_rect.left   = MAX(vdrv->clip.left, dst_rect.left);
            clip_rect.right  = MIN(vdrv->clip.right, dst_rect.right);
            clip_rect.top    = MAX(vdrv->clip.top, dst_rect.top);
            clip_rect.bottom = MIN(vdrv->clip.bottom, dst_rect.bottom);

            if (dst_rect.left < 0)
            {
                dst_sub_rect.left   = 0;
                dst_sub_rect.right  = clip_rect.right - dst_rect.left;
            }
            else
            {
                dst_sub_rect.left   = clip_rect.left - dst_rect.left;
                dst_sub_rect.right  = dst_sub_rect.left + (clip_rect.right - clip_rect.left);
            }
            if (dst_rect.top < 0)
            {
                dst_sub_rect.top    = 0;
                dst_sub_rect.bottom = clip_rect.bottom - dst_rect.top;
            }
            else
            {
                dst_sub_rect.top    = clip_rect.top  - dst_rect.top;
                dst_sub_rect.bottom = dst_sub_rect.top + (clip_rect.bottom - clip_rect.top);
            }

            D_DEBUG_AT( Gal_2D, "src_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src_rect) );
            D_DEBUG_AT( Gal_2D, "dst_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_rect) );
            D_DEBUG_AT( Gal_2D, "dst_sub_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_sub_rect) );

            /* No pending for filter blit. */
            gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

            gal_get_kernel_size(&src_rect, &dst_rect, &horfactor, &verfactor);
            gcmERR_BREAK(gco2D_SetKernelSize( vdrv->engine_2d, horfactor, verfactor));

            gcmERR_BREAK(gco2D_FilterBlit( vdrv->engine_2d,
                                           vdrv->src_phys_addr[0],
                                           vdrv->src_pitch[0],
                                           vdrv->src_phys_addr[1],
                                           vdrv->src_pitch[1],
                                           vdrv->src_phys_addr[2],
                                           vdrv->src_pitch[2],
                                           vdrv->src_native_format,
                                           vdrv->src_rotation,
                                           vdrv->src_aligned_width,
                                           &src_rect,
                                           vdrv->dst_phys_addr,
                                           vdrv->dst_pitch,
                                           vdrv->dst_native_format,
                                           vdrv->dst_rotation,
                                           vdrv->dst_aligned_width,
                                           &dst_rect,
                                           &dst_sub_rect ));
        }
        else {
            /* Check the threshold for small object rendering. */
            if (vdev->turn_on_threshold &&
                !vdev->disable_threshold &&
                (rect->w * rect->h < vdev->threshold_value)) {
                return false;
            }

            /* Flush the pendings if primitive type was changed. */
            if (vdrv->pending_type != GAL_PENDING_BLIT) {
                gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
                vdrv->pending_type = GAL_PENDING_BLIT;
            }

#if GAL_SURFACE_COMPRESSED
            if (vdev->hw_2d_compression &&
                vdrv->per_process_compression &&
                vdrv->dst_tile_status != gcv2D_TSC_2D_COMPRESSED &&
                vdrv->dst_tile_status_addr != gcvINVALID_ADDRESS)
            {
                gcmERR_BREAK(_ClearDstAndTileStatusBuffer(vdrv, vdev, true));
                vdrv->flush = true;
            }
#endif

            if (vdrv->src_rotation == gcvSURF_0_DEGREE) {
                src_rect.left   = rect->x;
                src_rect.top    = rect->y;
                src_rect.right  = rect->x + rect->w;
                src_rect.bottom = rect->y + rect->h;
            }
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
            else if (vdrv->src_rotation == gcvSURF_90_DEGREE) {
                src_rect.left   = rect->y;
                src_rect.top    = vdrv->src_width - rect->x - rect->w + 1;
                src_rect.right  = src_rect.left + rect->h;
                src_rect.bottom = src_rect.top + rect->w;

                D_UTIL_SWAP( rect->w, rect->h );
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported src rotation [%d].\n", __func__, vdrv->src_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
#endif

            if (vdrv->dst_rotation == gcvSURF_0_DEGREE) {
                dst_rect.left   = dx;
                dst_rect.top    = dy;
                dst_rect.right  = dst_rect.left + rect->w;
                dst_rect.bottom = dst_rect.top + rect->h;
            }
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
            else if (vdrv->dst_rotation == gcvSURF_90_DEGREE) {
                dst_rect.left   = dy;
                dst_rect.top    = vdrv->dst_width - dx - rect->h + 1;
                dst_rect.right  = dst_rect.left + rect->w;
                dst_rect.bottom = dst_rect.top + rect->h;
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported dest rotation [%d].\n", __func__, vdrv->dst_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
#endif

            D_DEBUG_AT( Gal_2D, "src_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src_rect) );
            D_DEBUG_AT( Gal_2D, "dst_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_rect) );

            checkConvert_valid_rectangle(1, &src_rect, &dst_rect);

            if (vdrv->need_rop_blend_w) {
                gcmERR_BREAK(_BlitROPBlend( vdrv, vdev, &src_rect, &dst_rect ));
            }
            else {
                vdrv->pending_src_rects[vdrv->pending_num].left   = src_rect.left;
                vdrv->pending_src_rects[vdrv->pending_num].top    = src_rect.top;
                vdrv->pending_src_rects[vdrv->pending_num].right  = src_rect.right;
                vdrv->pending_src_rects[vdrv->pending_num].bottom = src_rect.bottom;

                vdrv->pending_dst_rects[vdrv->pending_num].left   = dst_rect.left;
                vdrv->pending_dst_rects[vdrv->pending_num].top    = dst_rect.top;
                vdrv->pending_dst_rects[vdrv->pending_num].right  = dst_rect.right;
                vdrv->pending_dst_rects[vdrv->pending_num].bottom = dst_rect.bottom;

                if (!vdev->hw_2d_pe20 && vdrv->blit_blend && vdrv->dst_blend_no_alpha) {
                    max_pending_num = 1;
                }
                else {
                    max_pending_num = vdev->max_pending_num;
                }

                if (++vdrv->pending_num >= max_pending_num || vdrv->flush) {
                    gcmERR_BREAK(_Blit( vdrv, vdev ));
                    vdrv->pending_num = 0;
                    vdrv->flush =  false;
                }
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to do blit. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

/**
 * Blit2 function.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    rect: the rectangle area on source surface
 * \param [in]    dx: x on the target surface
 * \param [in]    dy: y on the target surface
 * \param [in]    sx2: x on the second source surface
 * \param [in]    sy2: y on the second source surface

 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)
bool
galBlit2( void         *drv,
          void         *dev,
          DFBRectangle *rect,
          int           dx,
          int           dy,
          int           sx2,
          int           sy2 )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;
    gcsRECT        dst_rect, src_rect, src2_rect;
    unsigned int   addr1, addr2, bpp, align;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv  != NULL );
    D_ASSERT( dev  != NULL );
    D_ASSERT( rect != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p, rect(%p): (%d, %d, %d, %d).\n"
                   "dx: %d, dy: %d.\n",
                   drv, dev, rect,
                   DFB_RECTANGLE_VALS(rect),
                   dx, dy );

    do {
        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        {
            /* Check the threshold for small object rendering. */
            if (vdev->turn_on_threshold &&
                !vdev->disable_threshold &&
                (rect->w * rect->h < vdev->threshold_value)) {
                return false;
            }

            /* Flush the pendings if primitive type was changed. */
            if (vdrv->pending_type != GAL_PENDING_BLIT) {
                gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));
                vdrv->pending_type = GAL_PENDING_BLIT;
            }

            if (vdrv->src_rotation == gcvSURF_0_DEGREE) {
                src_rect.left   = rect->x;
                src_rect.top    = rect->y;
                src_rect.right  = rect->x + rect->w;
                src_rect.bottom = rect->y + rect->h;

                src2_rect.left   = sx2;
                src2_rect.top    = sy2;
                src2_rect.right  = src2_rect.left + rect->w;
                src2_rect.bottom = src2_rect.top + rect->h;
            }
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
            else if (vdrv->src_rotation == gcvSURF_90_DEGREE) {
                src_rect.left   = rect->y;
                src_rect.top    = vdrv->src_width - rect->x - rect->w + 1;
                src_rect.right  = src_rect.left + rect->h;
                src_rect.bottom = src_rect.top + rect->w;

                src2_rect.left   = sy2;
                src2_rect.top    = sx2;
                src2_rect.right  = src2_rect.left + rect->h;
                src2_rect.bottom = src2_rect.top + rect->w;

                D_UTIL_SWAP( rect->w, rect->h );
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported src rotation [%d].\n", __func__, vdrv->src_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
#endif
            if (vdrv->dst_rotation == gcvSURF_0_DEGREE) {
                dst_rect.left   = dx;
                dst_rect.top    = dy;
                dst_rect.right  = dst_rect.left + rect->w;
                dst_rect.bottom = dst_rect.top + rect->h;
            }
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
            else if (vdrv->dst_rotation == gcvSURF_90_DEGREE) {
                dst_rect.left   = dy;
                dst_rect.top    = vdrv->dst_width - dx - rect->h + 1;
                dst_rect.right  = dst_rect.left + rect->w;
                dst_rect.bottom = dst_rect.top + rect->h;
            }
            else {
                D_DEBUG_AT( Gal_2D, "%s error: Unsupported dest rotation [%d].\n", __func__, vdrv->dst_rotation );
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
#endif

            D_DEBUG_AT( Gal_2D, "src_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src_rect) );
            D_DEBUG_AT( Gal_2D, "src2_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&src2_rect) );
            D_DEBUG_AT( Gal_2D, "dst_rect(%d, %d, %d, %d)\n", GAL_RECTANGLE_VALS(&dst_rect) );

            checkConvert_valid_rectangle(1, &src_rect, &dst_rect);
            checkConvert_valid_rectangle(1, &src2_rect, &dst_rect);


            /* Adjust address to the same dst rectangle */
            gcmERR_BREAK(gal_native_format2bpp(vdrv->src2_native_format, &bpp));
            addr2 = vdrv->src2_phys_addr[0] +
                    vdrv->src2_pitch[0] * (src2_rect.top - dst_rect.top) +
                    bpp / 8 * (src2_rect.left - dst_rect.left);

            gcmERR_BREAK(gal_native_format2bpp(vdrv->src_native_format, &bpp));
            addr1 = vdrv->src_phys_addr[0] +
                    vdrv->src_pitch[0] * (src_rect.top - dst_rect.top) +
                    bpp / 8 * (src_rect.left - dst_rect.left);

            gcmERR_BREAK(gco2D_QueryU32(vdrv->engine_2d, gcv2D_QUERY_RGB_ADDRESS_MIN_ALIGN, &align));

            if (!vdev->hw_2d_multi_src ||
                addr1 & align || addr2 & align) {
                gcmERR_BREAK(_Blit2NoMultiSrcFeature( vdrv, vdev, &src_rect, &src2_rect, &dst_rect ));
            }
            else {
                /* Blend first source to dest without alpha blending */
                gcmERR_BREAK(
                    gco2D_SetCurrentSourceIndex(vdrv->engine_2d, 0));

                gcmERR_BREAK(gco2D_DisableAlphaBlend(vdrv->engine_2d));

                gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                                 gcv2D_COLOR_MULTIPLY_DISABLE,
                                                                 gcv2D_COLOR_MULTIPLY_DISABLE,
                                                                 gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                                 gcv2D_COLOR_MULTIPLY_DISABLE ));

                gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                           addr2,
                                                           vdrv->src2_pitch[0],
                                                           vdrv->src2_native_format,
                                                           vdrv->src2_rotation,
                                                           vdrv->src2_aligned_width,
                                                           vdrv->src2_aligned_height,
                                                           gcvFALSE ));
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d, &dst_rect ));
                gcmERR_BREAK(gco2D_SetROP(vdrv->engine_2d, vdrv->blit_fg_rop, vdrv->blit_bg_rop));


                /* Blend second source to first surface with alpha blending */
                gcmERR_BREAK(
                    gco2D_SetCurrentSourceIndex(vdrv->engine_2d, 1));

                gcmERR_BREAK(gco2D_EnableAlphaBlendAdvanced( vdrv->engine_2d,
                                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                                             vdrv->blit_src_global_alpha_mode,
                                                             vdrv->blit_dst_global_alpha_mode,
                                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode ));

                gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                                 vdrv->need_src_alpha_multiply ?
                                                                    gcv2D_COLOR_MULTIPLY_ENABLE : vdrv->src_pmp_src_alpha,
                                                                 vdrv->dst_pmp_dst_alpha,
                                                                 vdrv->need_glb_alpha_multiply ?
                                                                    gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA : vdrv->src_pmp_glb_mode,
                                                                 vdrv->dst_dmp_dst_alpha ));

                gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                           addr1,
                                                           vdrv->src_pitch[0],
                                                           vdrv->src_native_format,
                                                           vdrv->src_rotation,
                                                           vdrv->src_aligned_width,
                                                           vdrv->src_aligned_height,
                                                           gcvFALSE ));
                gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d, &dst_rect ));
                gcmERR_BREAK(gco2D_SetROP(vdrv->engine_2d, vdrv->blit_fg_rop, vdrv->blit_bg_rop));

                /* Roll back */
                gcmERR_BREAK(
                    gco2D_SetCurrentSourceIndex(vdrv->engine_2d, 0));


                /* Multi-src blit */
                gcmERR_BREAK(
                    gco2D_SetClipping(vdrv->engine_2d, &dst_rect));
                gcmERR_BREAK(
                    gco2D_MultiSourceBlit(vdrv->engine_2d, 0x3U, &dst_rect, 1U));
            }
        }

        /* HW accelerated flag. */
        vdrv->dirty = true;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to do blit2. status: %d\n", status );

        return false;
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return true;
}

/**
 * _Blit2NoMultiSrcFeature function for no multi-src feature hardware.
 *
 * \param [in] drv: driver specific data
 * \param [in] dev: device specific data
 * \param [in] srect:  the rectangle of the source surface
 * \param [in] srect2: the rectangle of the second source surface
 * \param [in] drect:  the rectangle of the target surface
 *
 * \return true: accelerated by hardware
 * \return false: not accelerated by hardware
 */
static gceSTATUS
_Blit2NoMultiSrcFeature( void    *drv,
                      void    *dev,
                      gcsRECT *srect,
                      gcsRECT *srect2,
                      gcsRECT *drect )
{
    GalDriverData *vdrv   = drv;
    gceSTATUS      status = gcvSTATUS_OK;

    D_ASSERT( drv     != NULL );
    D_ASSERT( dev     != NULL );
    D_ASSERT( srect   != NULL );
    D_ASSERT( srect2  != NULL );
    D_ASSERT( drect   != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p.\n",
                   drv, dev );

    do {
        /* Blend src2 to dst. */
        gcmERR_BREAK(gco2D_DisableAlphaBlend(vdrv->engine_2d));

        gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                         gcv2D_COLOR_MULTIPLY_DISABLE ));

        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->src2_phys_addr[0],
                                                   vdrv->src2_pitch[0],
                                                   vdrv->src2_native_format,
                                                   vdrv->src2_rotation,
                                                   vdrv->src2_aligned_width,
                                                   vdrv->src2_aligned_height,
                                                   gcvFALSE ));

         gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                       vdrv->dst_phys_addr,
                                       vdrv->dst_pitch,
                                       vdrv->dst_rotation,
                                       vdrv->dst_aligned_width ));

        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                      1,
                                      srect2,
                                      drect,
                                      vdrv->blit_fg_rop,
                                      vdrv->blit_bg_rop,
                                      vdrv->dst_native_format ));

        /* Blend src1 to dest. */
        gcmERR_BREAK(gco2D_EnableAlphaBlendAdvanced( vdrv->engine_2d,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                                     vdrv->blit_src_global_alpha_mode,
                                                     vdrv->blit_dst_global_alpha_mode,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode ));

        gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced( vdrv->engine_2d,
                                                         vdrv->need_src_alpha_multiply ?
                                                            gcv2D_COLOR_MULTIPLY_ENABLE : vdrv->src_pmp_src_alpha,
                                                         vdrv->dst_pmp_dst_alpha,
                                                         vdrv->need_glb_alpha_multiply ?
                                                            gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA : vdrv->src_pmp_glb_mode,
                                                         vdrv->dst_dmp_dst_alpha ));

        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->src_phys_addr[0],
                                                   vdrv->src_pitch[0],
                                                   vdrv->src_native_format,
                                                   vdrv->src_rotation,
                                                   vdrv->src_aligned_width,
                                                   vdrv->src_aligned_height,
                                                   gcvFALSE ));

         gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                       vdrv->dst_phys_addr,
                                       vdrv->dst_pitch,
                                       vdrv->dst_rotation,
                                       vdrv->dst_aligned_width ));

        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                      1,
                                      srect,
                                      drect,
                                      vdrv->blit_fg_rop,
                                      vdrv->blit_bg_rop,
                                      vdrv->dst_native_format ));

    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to blit multi-source without hardware feature. status: %d\n", status );
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}
#endif

/**
 * Stretch blit function for dest surface without alpha channel.
 *
 * \param [in]    drv: driver specific data
 * \param [in]    dev: device specific data
 * \param [in]    srects: the rectangle of the source surface
 * \param [in]    num: rects number. It must be 1 if it's a stretch blit.
 * \param [in]    drects: the rectangle of the    target surface
 *
 * \return    true: accelerated by hardware
 * \return    false: not accelerated by hardware
 */
gceSTATUS
galStretchBlitDstNoAlpha( void    *drv,
                          void    *dev,
                          gcsRECT *srects,
                          int      src_num,
                          gcsRECT *drects,
                          int      dst_num )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;

    unsigned int         src_phys_addr;
    int                  src_pitch;
    gceSURF_FORMAT       src_format;
    unsigned char        fg_rop, bg_rop;
    gceSURF_TRANSPARENCY trans;
    unsigned int         hor_factor, ver_factor;

    gceSURF_GLOBAL_ALPHA_MODE src_global_alpha_mode;
    gceSURF_GLOBAL_ALPHA_MODE dst_global_alpha_mode;

    D_ASSERT( drv     != NULL );
    D_ASSERT( dev     != NULL );
    D_ASSERT( srects  != NULL );
    D_ASSERT( src_num > 0 );
    D_ASSERT( drects  != NULL );
    D_ASSERT( dst_num > 0 );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p. rect number is %d.\n",
                   drv, dev, dst_num );

    do {
        /* Copy dest to tmp surface. */
        D_DEBUG_AT( Gal_2D, "Copy dest to tmp surface.\n" );

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetColorSource\n"
                    "vdrv->dst_phys_addr: 0x%08X\n"
                    "vdrv->dst_pitch: %u\n"
                    "vdrv->dst_native_format: %u\n",
                    vdrv->dst_phys_addr,
                    vdrv->dst_pitch,
                    vdrv->dst_native_format );

        gcmERR_BREAK(gco2D_SetColorSource( vdrv->engine_2d,
                                           vdrv->dst_phys_addr,
                                           vdrv->dst_pitch,
                                           vdrv->dst_native_format,
                                           gcvSURF_0_DEGREE,
                                           0,                    /* no use of width */
                                           gcvFALSE,
                                           gcvSURF_OPAQUE,
                                           0 ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetTarget\n"
                    "vdrv->tmp_dst_phys_addr: 0x%08X\n"
                    "vdrv->tmp_dst_pitch: %u\n",
                    vdrv->tmp_dst_phys_addr,
                    vdrv->tmp_dst_pitch );

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->tmp_dst_phys_addr,
                                      vdrv->tmp_dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));

        /* Disable alpha blending. */
        gcmERR_BREAK(gco2D_DisableAlphaBlend( vdrv->engine_2d ));

        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                       dst_num,
                                       drects,
                                       drects,
                                       0xCC,
                                       0xCC,
                                      vdrv->tmp_dst_format ));

        /* Stretch blit src to tmp surface. */
        if (vdrv->use_pool_source) {
            D_DEBUG_AT( Gal_2D,    "Using pool src.\n" );

            src_phys_addr = vdrv->pool_src_phys_addr;
            src_pitch     = vdrv->pool_src_pitch;
            src_format    = vdrv->pool_src_native_format;
            if (!vdev->hw_2d_pe20) {
                bg_rop = fg_rop = 0x80;
            }
            else {
                fg_rop        = vdrv->draw_fg_rop;
                bg_rop        = vdrv->draw_bg_rop;
            }
            trans         = vdrv->draw_trans;
        }
        else {
            D_DEBUG_AT( Gal_2D,    "Not using pool src.\n" );

            src_phys_addr = vdrv->src_phys_addr[0];
            src_pitch     = vdrv->src_pitch[0];
            src_format    = vdrv->src_native_format;
            if (!vdev->hw_2d_pe20) {
                bg_rop = fg_rop = 0x80;
            }
            else {
                fg_rop        = vdrv->blit_fg_rop;
                bg_rop        = vdrv->blit_bg_rop;
            }
            trans         = vdrv->blit_trans;
        }

        D_DEBUG_AT( Gal_2D, "Stretch blit src to tmp surface.\n" );

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetColorSource\n"
                    "src_phys_addr: 0x%08X\n"
                    "src_pitch: %u\n"
                    "src_native_format: %u\n"
                    "trans: %u\n"
                    "vdrv->trans_color: %u\n",
                    src_phys_addr,
                    src_pitch,
                    src_format,
                    trans,
                    vdrv->trans_color );

        gcmERR_BREAK(gco2D_SetColorSource( vdrv->engine_2d,
                                           src_phys_addr,
                                           src_pitch,
                                           src_format,
                                           gcvSURF_0_DEGREE,
                                           0,                    /* no use of width */
                                           gcvFALSE,
                                           trans,
                                           vdrv->trans_color ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetTarget\n"
                    "vdrv->tmp_dst_phys_addr: 0x%08X\n"
                    "vdrv->tmp_dst_pitch: 0x%08X\n",
                    vdrv->tmp_dst_phys_addr,
                    vdrv->tmp_dst_pitch );

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->tmp_dst_phys_addr,
                                      vdrv->tmp_dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));

        D_DEBUG_AT( Gal_2D, "Enable blending.\n");

        if (DFB_DRAWING_FUNCTION( vdrv->accel )) {
            src_global_alpha_mode  = vdrv->draw_src_global_alpha_mode;
            dst_global_alpha_mode  = vdrv->draw_dst_global_alpha_mode;
        }
        else {
            src_global_alpha_mode  = vdrv->blit_src_global_alpha_mode;
            dst_global_alpha_mode  = vdrv->blit_dst_global_alpha_mode;
        }

        D_DEBUG_AT( Gal_2D,
                    "gco2D_EnableAlphaBlend\n"
                    "global_alpha_value: %u\n"
                    "vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode: %u\n"
                    "vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode: %u\n"
                    "src_global_alpha_mode: %u\n"
                    "dst_global_alpha_mode: %u\n"
                    "vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode: %u\n"
                    "vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode: %u\n"
                    "vdrv->src_blend_funcs[vdrv->src_blend - 1].color_mode: %u\n"
                    "vdrv->dst_blend_funcs[vdrv->dst_blend - 1].color_mode: %u\n",
                    vdrv->global_alpha_value,
                    vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                    vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                    src_global_alpha_mode,
                    dst_global_alpha_mode,
                    vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                    vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode,
                    vdrv->src_blend_funcs[vdrv->src_blend - 1].color_mode,
                    vdrv->dst_blend_funcs[vdrv->dst_blend - 1].color_mode );

        gcmERR_BREAK(gco2D_EnableAlphaBlend( vdrv->engine_2d,
                                             vdrv->global_alpha_value,
                                             vdrv->global_alpha_value,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                             src_global_alpha_mode,
                                             dst_global_alpha_mode,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].color_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].color_mode ));

        if (src_num == 1) {
            /* Calculate the stretch factors. */
            gcmERR_BREAK(gal_get_stretch_factors( &srects[0],
                                                  &drects[0],
                                                  &hor_factor,
                                                  &ver_factor ));

            /* Program the stretch factors. */
            gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                                  hor_factor,
                                                  ver_factor ));

            gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d, &srects[0] ));

            gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                            dst_num,
                                            drects,
                                            fg_rop,
                                            bg_rop,
                                            vdrv->tmp_dst_format ));
        }
        else {
            gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                             dst_num,
                                             srects,
                                             drects,
                                             fg_rop,
                                             bg_rop,
                                          vdrv->tmp_dst_format ));
        }

        /* Blit tmp to dest surface. */
        D_DEBUG_AT( Gal_2D, "Blit tmp to dest surface.\n" );

        D_DEBUG_AT( Gal_2D, "Disable blending.\n" );

        gcmERR_BREAK(gco2D_DisableAlphaBlend( vdrv->engine_2d ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetColorSource\n"
                    "vdrv->tmp_dst_phys_addr: 0x%08X\n"
                    "vdrv->tmp_dst_pitch: %u\n"
                    "vdrv->tmp_dst_format: %u\n",
                    vdrv->tmp_dst_phys_addr,
                    vdrv->tmp_dst_pitch,
                    vdrv->tmp_dst_format );

        gcmERR_BREAK(gco2D_SetColorSource( vdrv->engine_2d,
                                           vdrv->tmp_dst_phys_addr,
                                           vdrv->tmp_dst_pitch,
                                           vdrv->tmp_dst_format,
                                           gcvSURF_0_DEGREE,
                                           0,                        /* no use of width */
                                           gcvFALSE,
                                           gcvSURF_OPAQUE,
                                           0 ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetTarget\n"
                    "vdrv->dst_phys_addr: 0x%08X\n"
                    "vdrv->dst_pitch: 0x%08X\n",
                    vdrv->dst_phys_addr,
                    vdrv->dst_pitch );

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->dst_phys_addr,
                                      vdrv->dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));
        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                         dst_num,
                                         drects,
                                         drects,
                                         0xCC,
                                         0xCC,
                                      vdrv->dst_native_format ));

        D_DEBUG_AT( Gal_2D, "Recover the status after work.\n" );

        gcmERR_BREAK(gco2D_EnableAlphaBlend( vdrv->engine_2d,
                                             vdrv->global_alpha_value,
                                             vdrv->global_alpha_value,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                             src_global_alpha_mode,
                                             dst_global_alpha_mode,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode,
                                             vdrv->src_blend_funcs[vdrv->src_blend - 1].color_mode,
                                             vdrv->dst_blend_funcs[vdrv->dst_blend - 1].color_mode ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetColorSource\n"
                    "src_phys_addr: 0x%08X\n"
                    "src_pitch: %u\n"
                    "src_format: %u\n"
                    "vdrv->trans: %u\n"
                    "vdrv->trans_color: %u\n",
                    src_phys_addr,
                    src_pitch,
                    src_format,
                    trans,
                    vdrv->trans_color );

        gcmERR_BREAK(gco2D_SetColorSource( vdrv->engine_2d,
                                           src_phys_addr,
                                           src_pitch,
                                           src_format,
                                           gcvSURF_0_DEGREE,
                                           0,                    /* no use of width */
                                           gcvFALSE,
                                           trans,
                                           vdrv->trans_color ));

        D_DEBUG_AT( Gal_2D,
                    "gco2D_SetTarget\n"
                    "vdrv->dst_phys_addr: 0x%08X\n"
                    "vdrv->dst_pitch: 0x%08X\n",
                    vdrv->dst_phys_addr,
                    vdrv->dst_pitch );

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->dst_phys_addr,
                                      vdrv->dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to stretch blit without alpha. status: %d\n", status );
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}

/**
 * Blit function for DST_COLORKEY/XOR/BLENDING issue.
 *
 * \param [in] drv: driver specific data
 * \param [in] dev: device specific data
 * \param [in] srect: the rectangle of the source surface
 * \param [in] drect: the rectangle of the target surface
 *
 * \return true: accelerated by hardware
 * \return false: not accelerated by hardware
 */
static gceSTATUS
_BlitROPBlend( void    *drv,
               void    *dev,
               gcsRECT *srect,
               gcsRECT *drect )
{
    GalDriverData *vdrv   = drv;
    gceSTATUS      status = gcvSTATUS_OK;

    unsigned char  fg_rop, bg_rop;
    unsigned int   hor_factor, ver_factor;

    D_ASSERT( drv     != NULL );
    D_ASSERT( dev     != NULL );
    D_ASSERT( srect   != NULL );
    D_ASSERT( drect   != NULL );

    D_DEBUG_ENTER( Gal_2D,
                   "drv: %p, dev: %p.\n",
                   drv, dev );

    do {
        /* Copy dest to tmp surface. */
        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->dst_phys_addr,
                                                   vdrv->dst_pitch,
                                                   vdrv->dst_native_format,
                                                   gcvSURF_0_DEGREE,
                                                   0, 0,
                                                   gcvFALSE ));

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->tmp_dst_phys_addr,
                                      vdrv->tmp_dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));

        gcmERR_BREAK(gco2D_DisableAlphaBlend( vdrv->engine_2d ));

        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                      1,
                                      drect,
                                      drect,
                                      0xCC,
                                      0xCC,
                                      vdrv->tmp_dst_format ));

        /* Blit the src to tmp surface with blending and w/o ROP. */
        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->src_phys_addr[0],
                                                   vdrv->src_pitch[0],
                                                   vdrv->src_native_format,
                                                   vdrv->src_rotation,
                                                   vdrv->src_aligned_width,
                                                   vdrv->src_aligned_height,
                                                   gcvFALSE ));

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->tmp_dst_phys_addr,
                                      vdrv->tmp_dst_pitch,
                                      vdrv->dst_rotation,
                                      vdrv->dst_aligned_width ));

        gcmERR_BREAK(gco2D_EnableAlphaBlendAdvanced( vdrv->engine_2d,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                                     vdrv->blit_src_global_alpha_mode,
                                                     vdrv->blit_dst_global_alpha_mode,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode ));

        gcmERR_BREAK(gal_get_stretch_factors( srect,
                                              drect,
                                              &hor_factor,
                                              &ver_factor ));

        gcmERR_BREAK(gco2D_SetStretchFactors( vdrv->engine_2d,
                                              hor_factor,
                                              ver_factor ));

        gcmERR_BREAK(gco2D_SetSource( vdrv->engine_2d, srect ));

        if (!vdrv->vdev->hw_2d_pe20)
            fg_rop = bg_rop = 0x80;
        else
            fg_rop = bg_rop = 0xCC;
        gcmERR_BREAK(gco2D_StretchBlit( vdrv->engine_2d,
                                        1,
                                        drect,
                                        fg_rop,
                                        bg_rop,
                                        vdrv->tmp_dst_format ));

        /* Blit tmp to dest surface with ROP and w/o blend. */
        gcmERR_BREAK(gco2D_DisableAlphaBlend( vdrv->engine_2d ));

        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->tmp_dst_phys_addr,
                                                   vdrv->tmp_dst_pitch,
                                                   vdrv->tmp_dst_format,
                                                   gcvSURF_0_DEGREE,
                                                   0, 0,
                                                   gcvFALSE ));

        gcmERR_BREAK(gco2D_SetTarget( vdrv->engine_2d,
                                      vdrv->dst_phys_addr,
                                      vdrv->dst_pitch,
                                      gcvSURF_0_DEGREE,
                                      0 ));

        gcmERR_BREAK(gco2D_BatchBlit( vdrv->engine_2d,
                                      1,
                                      drect,
                                      drect,
                                      vdrv->blit_fg_rop,
                                      vdrv->blit_bg_rop,
                                      vdrv->dst_native_format ));

        D_DEBUG_AT( Gal_2D, "Recover the status after work.\n" );

        gcmERR_BREAK(gco2D_EnableAlphaBlendAdvanced( vdrv->engine_2d,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].alpha_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].alpha_mode,
                                                     vdrv->blit_src_global_alpha_mode,
                                                     vdrv->blit_dst_global_alpha_mode,
                                                     vdrv->src_blend_funcs[vdrv->src_blend - 1].factor_mode,
                                                     vdrv->dst_blend_funcs[vdrv->dst_blend - 1].factor_mode ));

        gcmERR_BREAK(gco2D_SetColorSourceAdvanced( vdrv->engine_2d,
                                                   vdrv->src_phys_addr[0],
                                                   vdrv->src_pitch[0],
                                                   vdrv->src_native_format,
                                                   gcvSURF_0_DEGREE,
                                                   0, 0,
                                                   gcvFALSE ));
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_WARN( "Failed to work the ROP and BLEND blitting. status: %d\n", status );
    }

    D_DEBUG_EXIT( Gal_2D, "\n" );

    return status;
}
