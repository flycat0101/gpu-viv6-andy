/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/**
 * \file gc_dfb_utils.h
 */

#ifndef __gc_dfb_utils_h_
#define __gc_dfb_utils_h_

#include <directfb.h>
#include <gc_hal_enum.h>
#include <gc_hal_base.h>

#define GAL_RECTANGLE_VALS(r) (r)->left, (r)->top, (r)->right, (r)->bottom
#define GAL_COLOR_VALS(c)     (c)->a, (c)->r, (c)->g, (c)->b

#define RECT_MIN(a,b) ((a)<(b)?(a):(b))
#define RECT_MAX(a,b) ((a)>(b)?(a):(b))

typedef enum {
    TRIANGLE_UNIT_LINE,
    TRIANGLE_UNIT_RECT
}
GalTriangleUnitType;

int gal_mod(int x);

bool gal_is_source_format( void                  *dev,
                           DFBSurfacePixelFormat  color );

bool gal_is_dest_format( void                  *dev,
                         DFBSurfacePixelFormat  color );

bool gal_is_yuv_format( DFBSurfacePixelFormat color );

bool gal_get_native_format( DFBSurfacePixelFormat  format,
                            gceSURF_FORMAT        *native_format );

bool
gal_native_format2bpp( gceSURF_FORMAT  native_format,
                       unsigned int    *bitperpixel );

bool gal_rect_union( gcsRECT *rects,
                     int      num,
                     gcsRECT *rect );

void gal_pixel_to_color_wo_expantion( DFBSurfacePixelFormat  format,
                                      unsigned long          pixel,
                                      DFBColor              *ret_color );

void gal_get_kernel_size( gcsRECT* srect,
                          gcsRECT* drect,
                          unsigned int* horfactor,
                          unsigned int* verfactor );

/* Check and convert rectangle/line into valid region */
bool checkConvert_valid_rectangle( gctUINT32     RectCount,
                                   gcsRECT_PTR   SrcRect,
                                   gcsRECT_PTR   DestRect );

bool checkConvert_valid_line( gcsRECT_PTR DestRect );

/* Scan convert triangle into lines */
void
scanConvertTriangle( DFBTriangle         *tri,
                     gcsRECT             *drect,
                     int                 *numRect,
                     GalTriangleUnitType  type );

#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)
/* Scan convert trapezoid into lines */
void
scanConvertTrapezoid( DFBTrapezoid        *trap,
                      gcsRECT             *drect,
                      int                 *numRect,
                      GalTriangleUnitType  type );
#endif

gceSTATUS
gal_get_stretch_factors( gcsRECT_PTR  src_rect,
                         gcsRECT_PTR  dst_rect,
                         gctUINT32   *hor_factor,
                         gctUINT32   *ver_factor );

#endif /* __gc_dfb_utils_h_ */
