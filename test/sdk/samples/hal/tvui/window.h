/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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




#ifndef __window_h_
#define __window_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
********************************** Window API *********************************
\******************************************************************************/

/* Draws a rectangluar border. */
gceSTATUS
DrawBorder(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT BorderWidth,
    IN gctUINT CornerOffset,
    IN gctUINT32 BorderColor
    );

/* Draws a gradient base for a button or a window. */
gceSTATUS
DrawGradientBase(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT CornerRadius,
    IN gctUINT BorderWidth,
    IN gctUINT32 TopColor,
    IN gctUINT32 BottomColor,
    IN gctUINT32 BorderColor
    );

/* Draws a gradient button. */
gceSTATUS
DrawButton(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT CornerRadius,
    IN gctUINT BorderWidth,
    IN gctUINT32 TopColor,
    IN gctUINT32 BottomColor,
    IN gctUINT32 BorderColor,
    IN gctUINT32 TextColor,
    IN gctUINT FontSelect,
    IN char * String
    );

#ifdef __cplusplus
}
#endif

#endif
