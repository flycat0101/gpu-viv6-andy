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
#include "ellipse.h"


/******************************************************************************\
***************************** Internal draw flags *****************************
\******************************************************************************/

typedef enum _DRAW_FLAGS
{
	DRAW_LEFT   = (1 << 0),
	DRAW_TOP    = (1 << 1),
	DRAW_RIGHT  = (1 << 2),
	DRAW_BOTTOM = (1 << 3),
	DRAW_FILLED = (1 << 4)
}
DRAW_FLAGS;


/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**	_RectFill
**
**	Draw a rectangle.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoSURF DestSurface
**			Pointer to the destination surface.
**
**		gcoBRUSH Brush
**			Brush to use for drawing.
**
**		gctINT Left
**		gctINT Top
**		gctINT Right
**		gctINT Bottom
**			Coordinates of the part of the ellipse to draw.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _RectFill(
	IN gco2D Engine,
	IN gcoSURF DestSurface,
	IN gcoBRUSH Brush,
	IN gctINT Left,
	IN gctINT Top,
	IN gctINT Right,
	IN gctINT Bottom
	)
{
	gcsRECT destRect;

	/* Init destination rectangle. */
	destRect.left   = Left;
	destRect.top    = Top;
	destRect.right  = Right;
	destRect.bottom = Bottom;

    if ((Left == Right) || (Top == Bottom))
    {
        /* Do the line. */
        return gcoSURF_Line(DestSurface, 1,
                            &destRect, Brush,
                            0xF0, 0xF0);
    }
    else
    {
	    /* Do the blit. */
	    return gcoSURF_Blit(gcvNULL, DestSurface,
					       1, gcvNULL, &destRect,
					       Brush,
					       0xF0, 0xF0,
					       gcvSURF_OPAQUE,
					       0,
					       gcvNULL, 0);
    }
}

/*******************************************************************************
**
**	_DrawEllipsePart
**
**	Ellipse is a symmetric shape, where one can simply derive three other
**	points if one of the four is known. This function does exactly that.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoSURF DestSurface
**			Pointer to the destination surface.
**
**		gcoBRUSH Brush
**			Brush to use for drawing.
**
**		gctINT16 CenterX
**		gctINT16 CenterY
**			Coordinates of the center of the ellipse in pixels.
**
**		gctINT Left
**		gctINT Top
**		gctINT Right
**		gctINT Bottom
**			Coordinates of the part of the ellipse to draw.
**
**		gctUINT BorderWidth
**			The thinkess of the border line.
**
**		DRAW_FLAGS Flags
**			A combination of DRAW_FLAGS flags.
**
**		ELLIPSE_QUADRANT Quadrant
**			Ellipse quadrant configuration.
**
**	OUTPUT:
**
**		Nothing.
*/
static gceSTATUS _DrawEllipsePart(
	IN gco2D Engine,
	IN gcoSURF DestSurface,
	IN gcoBRUSH Brush,
	IN gctINT CenterX,
	IN gctINT CenterY,
	IN gctINT Left,
	IN gctINT Top,
	IN gctINT Right,
	IN gctINT Bottom,
	IN gctUINT BorderWidth,
	IN DRAW_FLAGS Flags,
	IN ELLIPSE_QUADRANT Quadrant
	)
{
	gceSTATUS status;
	gctINT HalfBorder1, Complement1;
	gctINT HalfBorder2, Complement2;
	gctBOOL pairedBlit;
	gctINT left, top, right, bottom;
	DRAW_FLAGS flags;

	/* Assume success. */
	status = gcvSTATUS_OK;

	/* Adjust the part accorgingly to the quadrant configuration. */
	if ((Quadrant & ALL_QUADRANTS) != ALL_QUADRANTS)
	{
		/*
			First look for paired quadrants.
		*/

		left   = Left;
		top    = Top;
		right  = Right;
		bottom = Bottom;

		flags = Flags;

		pairedBlit = gcvFALSE;

		if (((Quadrant & QUADRANT1) != 0) &&
			((Quadrant & QUADRANT2) != 0))
		{
			if (Top <= CenterY)
			{
				if (Bottom > CenterY)
				{
					bottom = CenterY;
					flags &= ~DRAW_BOTTOM;
				}

				pairedBlit = gcvTRUE;
			}

			/* Quadrants 1 and 2 are now drawn. */
			Quadrant &= ~(QUADRANT1 | QUADRANT2);
		}
		else if (((Quadrant & QUADRANT3) != 0) &&
				 ((Quadrant & QUADRANT4) != 0))
		{
			if (Bottom >= CenterY)
			{
				if (Top < CenterY)
				{
					top = CenterY;
					flags &= ~DRAW_TOP;
				}

				pairedBlit = gcvTRUE;
			}

			/* Quadrants 3 and 4 are now drawn. */
			Quadrant &= ~(QUADRANT3 | QUADRANT4);
		}
		else if (((Quadrant & QUADRANT2) != 0) &&
				 ((Quadrant & QUADRANT3) != 0))
		{
			if (Left <= CenterX)
			{
				if (Right > CenterX)
				{
					right = CenterX;
					flags &= ~DRAW_RIGHT;
				}

				pairedBlit = gcvTRUE;
			}

			/* Quadrants 2 and 3 are now drawn. */
			Quadrant &= ~(QUADRANT2 | QUADRANT3);
		}
		else if (((Quadrant & QUADRANT1) != 0) &&
				 ((Quadrant & QUADRANT4) != 0))
		{
			if (Right >= CenterX)
			{
				if (Left < CenterX)
				{
					left = CenterX;
					flags &= ~DRAW_LEFT;
				}

				pairedBlit = gcvTRUE;
			}

			/* Quadrants 1 and 4 are now drawn. */
			Quadrant &= ~(QUADRANT1 | QUADRANT4);
		}

		/* Do the paired blit if needed. */
		if (pairedBlit)
		{
			status = _DrawEllipsePart(Engine,
									  DestSurface,
									  Brush,
									  CenterX,
									  CenterY,
									  left,
									  top,
									  right,
									  bottom,
									  BorderWidth,
									  flags,
									  ALL_QUADRANTS);

			if (status != gcvSTATUS_OK)
			{
				return status;
			}
		}

		/*
			Now go through each quadrant.
		*/

		if ((Quadrant & QUADRANT1) != 0)
		{
			if ((Right >= CenterX) && (Top <= CenterY))
			{
				left   = Left;
				top    = Top;
				right  = Right;
				bottom = Bottom;

				flags = Flags;

				if (Left < CenterX)
				{
					left = CenterX;
					flags &= ~DRAW_LEFT;
				}

				if (Bottom > CenterY)
				{
					bottom = CenterY;
					flags &= ~DRAW_BOTTOM;
				}

				status = _DrawEllipsePart(Engine,
										  DestSurface,
										  Brush,
										  CenterX,
										  CenterY,
										  left,
										  top,
										  right,
										  bottom,
										  BorderWidth,
										  flags,
										  ALL_QUADRANTS);

				if (status != gcvSTATUS_OK)
				{
					return status;
				}
			}
		}

		if ((Quadrant & QUADRANT2) != 0)
		{
			if ((Left <= CenterX) && (Top <= CenterY))
			{
				left   = Left;
				top    = Top;
				right  = Right;
				bottom = Bottom;

				flags = Flags;

				if (Right > CenterX)
				{
					right = CenterX;
					flags &= ~DRAW_RIGHT;
				}

				if (Bottom > CenterY)
				{
					bottom = CenterY;
					flags &= ~DRAW_BOTTOM;
				}

				status = _DrawEllipsePart(Engine,
										  DestSurface,
										  Brush,
										  CenterX,
										  CenterY,
										  left,
										  top,
										  right,
										  bottom,
										  BorderWidth,
										  flags,
										  ALL_QUADRANTS);

				if (status != gcvSTATUS_OK)
				{
					return status;
				}
			}
		}

		if ((Quadrant & QUADRANT3) != 0)
		{
			if ((Left <= CenterX) && (Bottom >= CenterY))
			{
				left   = Left;
				top    = Top;
				right  = Right;
				bottom = Bottom;

				flags = Flags;

				if (Right > CenterX)
				{
					right = CenterX;
					flags &= ~DRAW_RIGHT;
				}

				if (Top < CenterY)
				{
					top = CenterY;
					flags &= ~DRAW_TOP;
				}

				status = _DrawEllipsePart(Engine,
										  DestSurface,
										  Brush,
										  CenterX,
										  CenterY,
										  left,
										  top,
										  right,
										  bottom,
										  BorderWidth,
										  flags,
										  ALL_QUADRANTS);

				if (status != gcvSTATUS_OK)
				{
					return status;
				}
			}
		}

		if ((Quadrant & QUADRANT4) != 0)
		{
			if ((Right >= CenterX) && (Bottom >= CenterY))
			{
				left   = Left;
				top    = Top;
				right  = Right;
				bottom = Bottom;

				flags = Flags;

				if (Left < CenterX)
				{
					left = CenterX;
					flags &= ~DRAW_LEFT;
				}

				if (Top < CenterY)
				{
					top = CenterY;
					flags &= ~DRAW_TOP;
				}

				status = _DrawEllipsePart(Engine,
										  DestSurface,
										  Brush,
										  CenterX,
										  CenterY,
										  left,
										  top,
										  right,
										  bottom,
										  BorderWidth,
										  flags,
										  ALL_QUADRANTS);

				if (status != gcvSTATUS_OK)
				{
					return status;
				}
			}
		}

		return status;
	}

	do
	{
		/* Draw filled part. */
		if (Flags & DRAW_FILLED)
		{
			status = _RectFill(Engine,
							   DestSurface,
							   Brush,
							   Left,
							   Top,
							   Right + 1,
							   Bottom + 1);

			break;
		}

		/* Compute the half of the border thinkness. */
		HalfBorder1 = BorderWidth >> 1;
		HalfBorder2 = HalfBorder1 | (~BorderWidth & 1);

		Complement1 = BorderWidth - HalfBorder1;
		Complement2 = BorderWidth - HalfBorder2;

		/* Draw left side. */
		if (Flags & DRAW_LEFT)
		{
			status = _RectFill(Engine,
							   DestSurface,
							   Brush,
							   Left - HalfBorder1,
							   Top,
							   Left + Complement1,
							   Bottom + 1);

			if (status != gcvSTATUS_OK)
			{
				break;
			}
		}

		/* Draw right side. */
		if (Flags & DRAW_RIGHT)
		{
			status = _RectFill(Engine,
							   DestSurface,
							   Brush,
							   Right - HalfBorder2,
							   Top,
							   Right + Complement2,
							   Bottom + 1);

			if (status != gcvSTATUS_OK)
			{
				break;
			}
		}

		/* Draw top side. */
		if (Flags & DRAW_TOP)
		{
			status = _RectFill(Engine,
							   DestSurface,
							   Brush,
							   Left,
							   Top - HalfBorder1,
							   Right + 1,
							   Top + Complement1);

			if (status != gcvSTATUS_OK)
			{
				break;
			}
		}

		/* Draw bottom side. */
		if (Flags & DRAW_BOTTOM)
		{
			status = _RectFill(Engine,
							   DestSurface,
							   Brush,
							   Left,
							   Bottom - HalfBorder2,
							   Right + 1,
							   Bottom + Complement2);

			if (status != gcvSTATUS_OK)
			{
				break;
			}
		}
	}
	while (gcvFALSE);

	return status;
}


/******************************************************************************\
********************************* Ellipse API *********************************
\******************************************************************************/

/*******************************************************************************
**
**	DrawEllipse
**
**	Draw an ellipse.
**
**	INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoSURF DestSurface
**			Pointer to the destination surface.
**
**		gctINT16 CenterX
**		gctINT16 CenterY
**			Coordinates of the center of the ellipse in pixels.
**
**		UINT16 XRadius
**		UINT16 YRadius
**			Horizontal and vertical radiuses of the ellipse.
**
**		gctUINT32 Color
**			For simplicity this sample code always assumes that all colors are
**			in A8R8G8B8 format.
**
**		gctUINT BorderWidth
**			The thinkess of the border line.
**
**		gctUINT Filled
**			If true, the ellipse is drawn filled.
**
**		ELLIPSE_QUADRANT Quadrant
**			Ellipse quadrant configuration.
**
**	OUTPUT:
**
**		None.
*/
gceSTATUS DrawEllipse(
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
	)
{
	gceSTATUS status;
	gcoBRUSH brush = gcvNULL;
	gcsRECT rect1;
	gcsRECT rect2;
	gctINT topBreak, bottomBreak;
	gctINT minorChange;
	gctUINT32 twoXsquare, twoYsquare;
	gctINT32 xChange, yChange;
	gctINT32 ellipseError;
	gctUINT32 stoppingX, stoppingY;
	gctBOOL stopDrawing;
	DRAW_FLAGS flags;

	/* Correct the radius. */
	XRadius--;
	YRadius--;

	do
	{
		/* Create the brush. */
		status = gco2D_ConstructSingleColorBrush(Engine,
												gcvTRUE,
												Color,
												~0,
												&brush);
		if (status != gcvSTATUS_OK)
		{
			/* Error. */
			break;
		}

		/* Init the drawing flags. */
		flags = Filled ? DRAW_FILLED : 0;

		/* Compute double square of X and Y radiuses. */
		twoXsquare = 2 * XRadius * XRadius;
		twoYsquare = 2 * YRadius * YRadius;

		/*
			Init for the first set of points.
		*/

		rect1.left   = rect2.left   = CenterX - XRadius;
		rect1.right  = rect2.right  = CenterX + XRadius;
		rect1.top    = rect2.top    = CenterY;
		rect1.bottom = rect2.bottom = CenterY;

		xChange = YRadius * YRadius * (1 - 2 * XRadius);
		yChange = XRadius * XRadius;

		ellipseError = 0;

		stoppingX = twoYsquare * XRadius;
		stoppingY = 0;

		/* 1st set of points. Draw from the center of the ellipse toward */
		/* the top and the bottom until the major changes. */
		do
		{
			/* Advance the major coordinate. */
			stoppingY += twoXsquare;
			ellipseError += yChange;
			yChange += twoXsquare;

			/* Determine whether we need to advance the minor. */
			if (((2 * ellipseError + xChange) > 0))
			{
				/* Advance minor coordinate. */
				stoppingX -= twoYsquare;
				ellipseError += xChange;
				xChange += twoYsquare;

				/* Set minor coordinate delta. */
				minorChange = 1;
			}
			else
			{
				/* No change in the minor. */
				minorChange = 0;
			}

			/* Determine whether it is time to break the loop. */
			stopDrawing = (stoppingX < stoppingY);

			/* Draw the points. */
			if (minorChange || stopDrawing)
			{
				if (rect1.bottom == rect2.top)
				{
					/* Draw a single rectangle. */
					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  rect1.top,
											  rect2.right,
											  rect2.bottom,
											  BorderWidth,
											  flags | DRAW_LEFT | DRAW_RIGHT,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}
				}
				else
				{
					/* Draw two rectangles. */
					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  rect1.top,
											  rect1.right,
											  rect1.bottom,
											  BorderWidth,
											  flags | DRAW_LEFT | DRAW_RIGHT,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}

					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect2.left,
											  rect2.top,
											  rect2.right,
											  rect2.bottom,
											  BorderWidth,
											  flags | DRAW_LEFT | DRAW_RIGHT,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}
				}

				/* Update rectangles. */
				rect1.bottom = rect1.top - 1;
				rect2.top = rect2.bottom + 1;
			}

			/* Advance coordinates. */
			rect1.left  += minorChange;
			rect1.right -= minorChange;
			rect1.top   -= 1;

			rect2.left   += minorChange;
			rect2.right  -= minorChange;
			rect2.bottom += 1;
		}
		while (!stopDrawing);

		/* Check the status. */
		if (status != gcvSTATUS_OK)
		{
			/* Error. */
			break;
		}

		/*
			Init for the second set of points.
		*/

		topBreak    = rect1.bottom;
		bottomBreak = rect2.top;

		rect1.left   = rect2.left   = CenterX;
		rect1.right  = rect2.right  = CenterX;
		rect1.top    = rect2.top    = CenterY - YRadius;
		rect1.bottom = rect2.bottom = CenterY + YRadius;

		xChange = YRadius * YRadius;
		yChange = XRadius * XRadius * (1 - 2 * YRadius);

		ellipseError = 0;

		stoppingX = 0;
		stoppingY = twoXsquare * YRadius;

		/* 2nd set of points. Draw from the top and the bottom of the ellipse */
		/* toward the center until the major changes. */
		do
		{
			/* Advance the major coordinate. */
			stoppingX += twoYsquare;
			ellipseError += xChange;
			xChange += twoYsquare;

			if ((2 * ellipseError + yChange) > 0)
			{
				stoppingY -= twoXsquare;
				ellipseError += yChange;
				yChange += twoXsquare;

				/* Set minor coordinate delta. */
				minorChange = 1;
			}
			else
			{
				/* No change in the minor. */
				minorChange = 0;
			}

			/* Determine whether it is time to break the loop. */
			stopDrawing = (stoppingX > stoppingY);

			/* Draw the points. */
			if (minorChange || stopDrawing)
			{
				if (rect1.right == rect2.left)
				{
					/* Draw two rectangles. */
					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  rect1.top,
											  rect2.right,
											  topBreak,
											  BorderWidth,
											  flags | DRAW_TOP,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}

					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  bottomBreak,
											  rect2.right,
											  rect2.bottom,
											  BorderWidth,
											  flags | DRAW_BOTTOM,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}
				}
				else
				{
					/* Draw four rectangles. */
					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  rect1.top,
											  rect1.right,
											  topBreak,
											  BorderWidth,
											  flags | DRAW_TOP,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}

					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect2.left,
											  rect2.top,
											  rect2.right,
											  topBreak,
											  BorderWidth,
											  flags | DRAW_TOP,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}

					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect1.left,
											  bottomBreak,
											  rect1.right,
											  rect1.bottom,
											  BorderWidth,
											  flags | DRAW_BOTTOM,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}

					status = _DrawEllipsePart(Engine,
											  DestSurface,
											  brush,
											  CenterX,
											  CenterY,
											  rect2.left,
											  bottomBreak,
											  rect2.right,
											  rect2.bottom,
											  BorderWidth,
											  flags | DRAW_BOTTOM,
											  Quadrant);
					if (status != gcvSTATUS_OK)
					{
						/* Error. */
						break;
					}
				}

				/* Update rectangles. */
				rect1.right = rect1.left  - 1;
				rect2.left  = rect2.right + 1;
			}

			/* Advance coordinates. */
			rect1.top    += minorChange;
			rect1.bottom -= minorChange;
			rect1.left   -= 1;

			rect2.top    += minorChange;
			rect2.bottom -= minorChange;
			rect2.right  += 1;
		}
		while (!stopDrawing);
	}
	while (gcvFALSE);

	/* Cleanup. */
	if (brush != gcvNULL)
	{
		gcoBRUSH_Destroy(brush);
	}

	return status;
}
