/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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


#ifndef __text_h_
#define __text_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
**************************** Text drawing options *****************************
\******************************************************************************/

typedef enum _TEXT_OPTIONS
{
	LEFT_TEXT    = (0 << 0),
	HCENTER_TEXT = (1 << 0),
	RIGHT_TEXT   = (2 << 0),
	HORIZONTAL_TEXT_ALIGNMENT = (3 << 0),

	TOP_TEXT     = (0 << 4),
	VCENTER_TEXT = (1 << 4),
	BOTTOM_TEXT  = (2 << 4),
	VERTICAL_TEXT_ALIGNMENT = (3 << 4),

	SHADOW_TEXT  = (1 << 8),
}
TEXT_OPTIONS;

typedef enum _TEXT_CONSTANTS
{
	SHADOW_OFFSET = 1
}
TEXT_CONSTANTS;


/******************************************************************************\
*********************************** Text API **********************************
\******************************************************************************/

/* Calculates the width and the height of the text. */
gceSTATUS
GetTextSize(
	IN gctUINT FontSelect,
	IN char * String,
	IN OUT gctINT * TextWidth,
	IN OUT gctINT * TextHeight,
	IN OUT gctINT * VerOffset
	);

/* Draws the specified text. */
gceSTATUS
DrawString(
	IN gco2D Engine,
	IN gcoSURF DestSurface,
	IN gcsRECT_PTR DestRect,
	IN gctUINT FontSelect,
	IN char * String,
	IN TEXT_OPTIONS Options,
	IN gctUINT32 FgColor,
	IN gctUINT32 BgColor,
	IN gcoBRUSH Brush,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop,
	IN gceSURF_TRANSPARENCY Transparency
	);

/* Draws the specified text with a shadow. */
gceSTATUS
DrawShadowedString(
	IN gco2D Engine,
	IN gcoSURF DestSurface,
	IN gcsRECT_PTR DestRect,
	IN gctUINT FontSelect,
	IN char * String,
	IN TEXT_OPTIONS Options,
	IN gctUINT32 TextColor,
	IN gctUINT32 ShadowColor,
	IN gcoBRUSH Brush,
	IN gctUINT8 FgRop,
	IN gctUINT8 BgRop
	);

#ifdef __cplusplus
}
#endif

#endif
