/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/




#ifndef __ellipse_h_
#define __ellipse_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
************************** Ellipse quadrant values ****************************
\******************************************************************************/

typedef enum _ELLIPSE_QUADRANT
{
    QUADRANT1     = (1 << 0),    /* Top right. */
    QUADRANT2     = (1 << 1),    /* Top left. */
    QUADRANT3     = (1 << 2),    /* Bottom left. */
    QUADRANT4     = (1 << 3),    /* Bottom right. */
    ALL_QUADRANTS = (QUADRANT1 | QUADRANT2 | QUADRANT3 | QUADRANT4)
}
ELLIPSE_QUADRANT;


/******************************************************************************\
********************************* Ellipse API *********************************
\******************************************************************************/

gceSTATUS
DrawEllipse(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gctINT CenterX,
    IN gctINT CenterY,
    IN gctUINT XRadius,
    IN gctUINT YRadius,
    IN gctUINT32 Color,
    IN gctUINT BorderWidth,
    IN gctBOOL Filled,
    IN ELLIPSE_QUADRANT Quadrant
    );

#ifdef __cplusplus
}
#endif

#endif
