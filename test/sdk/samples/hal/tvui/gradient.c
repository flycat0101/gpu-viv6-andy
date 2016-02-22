/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#include "PreComp.h"
#include "gradient.h"

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**	_GetComponents
**
**	Extract color components for the gradirent and return them in 8.16 fixed
**	point format.
**
**	INPUT:
**
**		gctUINT32 color
**			For simplicity this sample code always assumes that all colors are
**			in A8R8G8B8 format.
**
**	OUTPUT:
**
**		gctUINT32 * a
**		gctUINT32 * r
**		gctUINT32 * g
**		gctUINT32 * b
**			Color components.
*/
static void _GetComponents(
	gctUINT32 color,
	gctUINT32 * a,
	gctUINT32 * r,
	gctUINT32 * g,
	gctUINT32 * b
	)
{
	*a = (color >> 8)  & 0x00FF0000;
	*r = (color)       & 0x00FF0000;
	*g = (color << 8)  & 0x00FF0000;
	*b = (color << 16) & 0x00FF0000;
}

/*******************************************************************************
**
**	_MakeColor
**
**	Create a color value from color components.
**
**	INPUT:
**
**		gctUINT32 a
**		gctUINT32 r
**		gctUINT32 g
**		gctUINT32 b
**			Color components.
**
**	RETURN:
**
**		gctUINT32
**			For simplicity this sample code always assumes that all colors are
**			in A8R8G8B8 format.
*/
static gctUINT32 _MakeColor(
	gctUINT32 a,
	gctUINT32 r,
	gctUINT32 g,
	gctUINT32 b
	)
{
	a = (a & 0x00FF0000) << 8;
	r = (r & 0x00FF0000);
	g = (g & 0x00FF0000) >> 8;
	b = (b & 0x00FF0000) >> 16;

	return a | r | g | b;
}

/*******************************************************************************
**
**	_ComputeStep
**
**	Compute color component step for the gradient.
**
**	INPUT:
**
**		gctUINT32 value0
**		gctUINT32 value1
**			Value range of the color component in 8.16 fixed point format.
**
**		gctUINT16 count
**			The number of stops within the range.
**
**	OUTPUT:
**
**		None.
*/
static gctINT32 _ComputeStep(
	gctUINT32 value0,
	gctUINT32 value1,
	gctUINT16 count
	)
{
	return (gctINT32)(value1 - value0) / (count - 1);
}


/******************************************************************************\
********************************* Ellipse API *********************************
\******************************************************************************/

/*******************************************************************************
**
**	DrawGradientRect
**
**	Draw a rectangle fill with gradient color.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoSURF DestSurface
**			Pointer to the destination surface.
**
**		gctINT Left
**		gctINT Top
**		gctINT Right
**		gctINT Bottom
**			Coordinates in pixels of the rectangle. Note that left top corner
**			coordinates are inclusive, but right bottom corner coordinates are
**			greater by 1 then the actual corner. This is done this way so that
**			the size can be easily computed as follows:
**				width  = right  - left
**				height = bottom - top
**
**		gctUINT32 color0
**		gctUINT32 color1
**			Gradient color range.
**			For simplicity this sample code always assumes that all colors are
**			in A8R8G8B8 format.
**
**		gctUINT horizontal
**			If not zero, the gradient is drawn in horizontal direction;
**			otherwise in vertical.
**
**	OUTPUT:
**
**		None.
*/
gceSTATUS DrawGradientRect(
	IN gco2D Engine,
	IN gcoSURF DestSurface,
	IN gcsRECT_PTR DestRect,
	IN gctUINT32 color0,
	IN gctUINT32 color1,
	IN gctBOOL horizontal
	)
{
	gceSTATUS status = gcvSTATUS_OK;
	gcoBRUSH brush = gcvNULL;
	gctUINT32 a0, r0, g0, b0;
	gctUINT32 a1, r1, g1, b1;
	gctUINT32 color;
	gctINT32 aStep, rStep, gStep, bStep;
	gctUINT16 count;
	gctUINT xDelta, yDelta;
	gcsRECT rect;

	/* Verify the arguments. */
    gcmHEADER_ARG("Engine=0x%x DestSurface=0x%x DestRect=0x%x color0=%d color1=%d horizontal=%d",
                Engine, DestSurface, DestRect, color0, color1, horizontal);
	gcmVERIFY_ARGUMENT(DestRect != gcvNULL);

	do
	{
		/* If both colors are the same, just draw a single rectangle. */
		if (color0 == color1)
		{
			/* Create brush object. */
			gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Engine,
													 gcvTRUE,
													 color0,
													 ~0,
													 &brush));

			/* Draw the rectangle. */
			gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
								  1, gcvNULL, DestRect,
								  brush,
								  0xF0, 0xF0,
								  gcvSURF_OPAQUE,
								  0,
								  gcvNULL, 0));

			/* Success. */
			break;
		}

		/* Determine direction parameters. */
		if (horizontal)
		{
			count = DestRect->right - DestRect->left;

			rect.left   = DestRect->left;
			rect.top    = DestRect->top;
			rect.right  = DestRect->left + 1;
			rect.bottom = DestRect->bottom;

			xDelta = 1;
			yDelta = 0;
		}
		else
		{
			count = DestRect->bottom - DestRect->top;

			rect.left   = DestRect->left;
			rect.top    = DestRect->top;
			rect.right  = DestRect->right;
			rect.bottom = DestRect->top + 1;

			xDelta = 0;
			yDelta = 1;
		}

		/* Extract components. */
		_GetComponents(color0, &a0, &r0, &g0, &b0);
		_GetComponents(color1, &a1, &r1, &g1, &b1);

		/* Compute component steps. */
		aStep = _ComputeStep(a0, a1, count);
		rStep = _ComputeStep(r0, r1, count);
		gStep = _ComputeStep(g0, g1, count);
		bStep = _ComputeStep(b0, b1, count);

		while (count--)
		{
			/* Construct the current color value. */
			color = count
				? _MakeColor(a0, r0, g0, b0)
				: _MakeColor(a1, r1, g1, b1);

			/* Create brush object. */
			gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Engine,
													 gcvTRUE,
													 color,
													 ~0,
													 &brush));

			/* Draw a line. */
			gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
								  1, gcvNULL, &rect,
								  brush,
								  0xF0, 0xF0,
								  gcvSURF_OPAQUE,
								  0,
								  gcvNULL, 0));

			/* Delete the brush. */
			gcmERR_BREAK(gcoBRUSH_Destroy(brush));
			brush = gcvNULL;

			/* Advance the coordinates for the next line. */
			rect.left   += xDelta;
			rect.top    += yDelta;
			rect.right  += xDelta;
			rect.bottom += yDelta;

			/* Advance the color for the next line. */
			a0 += aStep;
			r0 += rStep;
			g0 += gStep;
			b0 += bStep;
		}
	}
	while (gcvFALSE);

	/* Cleanup. */
	if (brush != gcvNULL)
	{
		/* Delete the brush. */
		gcmVERIFY_OK(gcoBRUSH_Destroy(brush));
	}

	return status;
}
