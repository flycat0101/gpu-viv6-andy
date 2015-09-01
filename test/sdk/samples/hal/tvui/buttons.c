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


#include "PreComp.h"
#include "buttons.h"
#include "bitmap.h"
#include "window.h"

#ifndef IMAGE_PREFIX
#ifdef LINUX
#	define IMAGE_PREFIX "images/"
#else
#	define IMAGE_PREFIX "images\\"
#endif
#endif


/******************************************************************************\
************************* gcoBUTTONS Object definitions ************************
\******************************************************************************/

enum BUTTONS_TYPE
{
	gcvOBJ_BUTTONS = gcmCC('B','T','N','S')
};

gcsRECT ButtonRects[] =
{
	{0,  0,  8,  9},	/* AQBUTTON_PLAY */
	{9,  0,  17, 9},	/* AQBUTTON_PAUSE */
	{18, 0,  27, 9},	/* AQBUTTON_STOP */
	{28, 0,  37, 9},	/* AQBUTTON_RECORD */
	{0,  10, 14, 19},	/* AQBUTTON_REWIND */
	{15, 10, 29, 19},	/* AQBUTTON_FAST_FORWARD */
	{0,  20, 11, 29},	/* AQBUTTON_SKIP_BACK */
	{12, 20, 23, 29},	/* AQBUTTON_SKIP_FORWARD */
	{30, 10, 39, 17},	/* AQBUTTON_UP */
	{30, 18, 39, 25},	/* AQBUTTON_DOWN */
};


/******************************************************************************\
******************************* gcoBUTTONS Object *******************************
\******************************************************************************/

struct _gcoBUTTONS
{
	/* Object. */
	gcsOBJECT object;

	/* Pointer to an gcoOS object. */
	gcoOS os;

	/* Button parameters. */
	gcoSURF surf;
};


/******************************************************************************\
********************************* Buttons API *********************************
\******************************************************************************/

/*******************************************************************************
**
**  gcoBUTTONS_Construct
**
**  Construct buttons object.
**
**  INPUT:
**
**		gcoHAL Hal
**			Pointer to an gcoHAL object.
**
**		gcoOS Os
**			Pointer to an gcoOS object.
**
**  OUTPUT:
**
**		gcoBUTTONS * Buttons
**			Pointer to a new gcoBUTTONS object.
*/
gceSTATUS gcoBUTTONS_Construct(
	IN gcoHAL Hal,
	IN gcoOS Os,
	IN OUT gcoBUTTONS * Buttons
	)
{
	gceSTATUS status;
	gcoBUTTONS buttons = gcvNULL;
	gcoSURF surf = gcvNULL;

	/* Verify arguments. */
    gcmHEADER_ARG("Hal=0x%x Os=0x%x Buttons=0x%x", Hal, Os, Buttons);
	gcmVERIFY_OBJECT(Os, gcvOBJ_OS);
	gcmVERIFY_ARGUMENT(Buttons != gcvNULL);

	do
	{
		/* Load buttons into a surface. */
		gcmERR_BREAK(gcoBITMAP_LoadSurface(Hal, Os, IMAGE_PREFIX "Buttons.bmp", &surf));

		/* Allocate the gcoBRUSH object. */
		gcmERR_BREAK(gcoOS_Allocate(Os, sizeof(struct _gcoBUTTONS), (gctPOINTER *) &buttons));

		/* Initialize the gcoBRUSH object.*/
		buttons->object.type = gcvOBJ_BUTTONS;
		buttons->os = Os;

		/* Set members. */
		buttons->surf = surf;

		/* Set the result. */
		*Buttons = buttons;

		/* Success. */
		return gcvSTATUS_OK;
	}
	while (gcvFALSE);

	/* Cleanup. */
	if (buttons)
	{
		gcmVERIFY_OK(gcoOS_Free(Os, buttons));
	}

	if (surf)
	{
		gcmVERIFY_OK(gcoSURF_Destroy(surf));
	}

	/* Return status. */
	return status;
}

/*******************************************************************************
**
**  gcoBUTTONS_Destroy
**
**  Destroy buttons object.
**
**  INPUT:
**
**		gcoBUTTONS Buttons
**			Pointer to a gcoBUTTONS object.
**
**  OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBUTTONS_Destroy(
	IN gcoBUTTONS Buttons
	)
{
    gcmHEADER_ARG("Buttons=0x%x", Buttons);

	/* Verify arguments. */
	gcmVERIFY_OBJECT(Buttons, gcvOBJ_BUTTONS);

	/* Free the surface. */
	gcmVERIFY_OK(gcoSURF_Destroy(Buttons->surf));

	/* Free the object. */
	gcmVERIFY_OK(gcoOS_Free(Buttons->os, Buttons));

	return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoBUTTONS_DrawButton
**
**  Draw a button with an icon.
**
**  INPUT:
**
**		gco2D Engine
**			Pointer to an gco2D object.
**
**		gcoBUTTONS Buttons
**			Pointer to a gcoBUTTONS object.
**
**		gcoSURF DestSurface
**			Pointer to the destination surface.
**
**		gcsRECT_PTR DestRect
**			Destination rectangle.
**
**		gctUINT CornerRadius
**			If zero, a rectangular button will be drawn.
**			If not zero, the corners will be dircular with the specified radius.
**
**		gctUINT32 TopColor
**		gctUINT32 BottomColor
**			Top of the button and bottom of the button colors for the gradient.
**
**		gctUINT32 BorderColor
**			The color of the border around the button.
**
**		BUTTON_CONSTANTS Button
**			Button type to draw.
**
**  OUTPUT:
**
**		Nothing.
*/
gceSTATUS gcoBUTTONS_DrawButton(
	IN gco2D Engine,
	IN gcoBUTTONS Buttons,
	IN gcoSURF DestSurface,
	IN gcsRECT_PTR DestRect,
	IN gctUINT CornerRadius,
	IN gctUINT32 TopColor,
	IN gctUINT32 BottomColor,
	IN gctUINT32 BorderColor,
	IN BUTTON_CONSTANTS Button
	)
{
	gceSTATUS status;
	gcsRECT_PTR srcRect;
	gcsRECT dstRect;
	gctINT32 srcWidth, srcHeight;
	gctINT32 dstWidth, dstHeight;

	gcmHEADER_ARG("Engine=0x%x Buttons=0x%x DestSurface=0x%x \
                  DestRect=0x%x CornerRadius=%d TopColor=%d \
                  BottomColor=%d BorderColor=%d Button=0x%x",
		Engine, Buttons, DestSurface, DestRect, CornerRadius, TopColor,
        BottomColor, BorderColor, Button);

	/* Verify arguments. */
	gcmVERIFY_OBJECT(Buttons, gcvOBJ_BUTTONS);
	gcmVERIFY_ARGUMENT((Button >= 0) && (Button < gcmCOUNTOF(ButtonRects)));

	do
	{
		/* Draw the button's base. */
		gcmERR_BREAK(DrawGradientBase(Engine,
								   DestSurface,
								   DestRect,
								   CornerRadius,
								   1,
								   TopColor,
								   BottomColor,
								   BorderColor));

		/* Get a pointer to the source rectangle. */
		srcRect = &ButtonRects[Button];

		/* Determine the size of the source rectangle. */
		gcmERR_BREAK(gcsRECT_Width(srcRect, &srcWidth));
		gcmERR_BREAK(gcsRECT_Height(srcRect, &srcHeight));

		/* Determine the size of the destination rectangle. */
		gcmERR_BREAK(gcsRECT_Width(DestRect, &dstWidth));
		gcmERR_BREAK(gcsRECT_Height(DestRect, &dstHeight));

		/* Init the destination rectngle. */
		dstRect.left   = DestRect->left + (dstWidth  - srcWidth)  / 2;
		dstRect.top    = DestRect->top  + (dstHeight - srcHeight) / 2;
		dstRect.right  = dstRect.left + srcWidth;
		dstRect.bottom = dstRect.top  + srcHeight;

		/* Blit the button image. */
		gcmERR_BREAK(gcoSURF_Blit(Buttons->surf, DestSurface,
							  1, srcRect, &dstRect,
							  gcvNULL,
							  0xCC, 0xAA,
							  gcvSURF_SOURCE_MATCH,
							  0x0000FF00,
							  gcvNULL, 0));
	}
	while (gcvFALSE);

	/* Return status. */
	return status;
}
