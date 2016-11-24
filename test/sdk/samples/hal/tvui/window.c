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
#include "window.h"
#include "ellipse.h"
#include "gradient.h"
#include "text.h"


/******************************************************************************\
******************************** Global Defines *******************************
\******************************************************************************/

/* If USE_LINES is 1, the horizontal and vertical borders of a window will */
/* be drawn using LINE command in hardware. Otherwise the borders will be  */
/* drawn via BLIT command. Normally, the horizontal and vertical lines     */
/* should be drawn with via BLIT, because it's faster. This define here is */
/* merely for testing the LINE command API call.                           */
#define USE_LINES 1


/******************************************************************************\
********************************** Window API *********************************
\******************************************************************************/

/*******************************************************************************
**
**    DrawBorder
**
**    Draws a rectangluar border.
**
**    INPUT:
**
**        gco2D Engine
**            Pointer to an gco2D object.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**        gcsRECT_PTR DestRect
**            Destination rectangle.
**
**        gctUINT BorderWidth
**            The width of the border in pixels.
**
**        gctUINT CornerOffset
**            If zero, a connected border will be drawn.
**            If not zero, the border lines will be drawn with the specified
**            offset from the corners.
**
**        gctUINT32 BorderColor
**            The color of the border around the button.
**
**    OUTPUT:
**
**        Nothing.
*/
gceSTATUS DrawBorder(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT BorderWidth,
    IN gctUINT CornerOffset,
    IN gctUINT32 BorderColor
    )
{
    gceSTATUS status;
    gcoBRUSH borderBrush = gcvNULL;
    gcsRECT rect;
    gcsRECT destRect;
#if USE_LINES
    gctUINT i;
#endif

    do
    {
        /* Init destination rect if not provided. */
        if (DestRect == gcvNULL)
        {
            /* Init the destination origin. */
            destRect.left = 0;
            destRect.top  = 0;

            /* Query the size of the background surface. */
            gcmERR_BREAK(gcoSURF_GetSize(DestSurface,
                                     (gctUINT *)&destRect.right,
                                     (gctUINT *)&destRect.bottom,
                                     gcvNULL));

            /* Set the address of the rect. */
            DestRect = &destRect;
        }

        /* Construct border color brushe. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Engine,
                                                 gcvTRUE,
                                                 BorderColor,
                                                 ~0,
                                                 &borderBrush));

/*----------------------------------------------------------------------------*/
/*----------------------------- Draw top border. -----------------------------*/

#if USE_LINES
        rect.left   = DestRect->left  + CornerOffset;
        rect.top    = DestRect->top;
        rect.right  = DestRect->right - CornerOffset;
        rect.bottom = DestRect->top;

        for (i = 0; i < BorderWidth; i++)
        {
            /* Draw one line. */
            gcmERR_BREAK(gcoSURF_Line(DestSurface,
                                  1, &rect,
                                  borderBrush,
                                  0xF0, 0xF0));

            /* Advance the rectangle. */
            rect.top    += 1;
            rect.bottom += 1;
        }
#else
        rect.left   = DestRect->left  + CornerOffset;
        rect.top    = DestRect->top;
        rect.right  = DestRect->right - CornerOffset;
        rect.bottom = DestRect->top   + BorderWidth;

        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                              1, gcvNULL, &rect,
                              borderBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));
#endif

/*----------------------------------------------------------------------------*/
/*--------------------------- Draw bottom border. ----------------------------*/

#if USE_LINES
        rect.top = rect.bottom = DestRect->bottom - 1;

        for (i = 0; i < BorderWidth; i++)
        {
            /* Draw one line. */
            gcmERR_BREAK(gcoSURF_Line(DestSurface,
                                  1, &rect,
                                  borderBrush,
                                  0xF0, 0xF0));

            /* Advance the rectangle. */
            rect.top    -= 1;
            rect.bottom -= 1;
        }
#else
        rect.top    = DestRect->bottom - BorderWidth;
        rect.bottom = DestRect->bottom;

        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                              1, gcvNULL, &rect,
                              borderBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));
#endif

/*----------------------------------------------------------------------------*/
/*---------------------------- Draw left border. -----------------------------*/

#if USE_LINES
        rect.left   = DestRect->left;
        rect.top    = DestRect->top    + CornerOffset;
        rect.right  = DestRect->left;
        rect.bottom = DestRect->bottom - CornerOffset;

        for (i = 0; i < BorderWidth; i++)
        {
            /* Draw one line. */
            gcmERR_BREAK(gcoSURF_Line(DestSurface,
                                  1, &rect,
                                  borderBrush,
                                  0xF0, 0xF0));

            /* Advance the rectangle. */
            rect.left  += 1;
            rect.right += 1;
        }
#else
        rect.left   = DestRect->left;
        rect.top    = DestRect->top    + CornerOffset;
        rect.right  = DestRect->left   + BorderWidth;
        rect.bottom = DestRect->bottom - CornerOffset;

        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                              1, gcvNULL, &rect,
                              borderBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));
#endif

/*----------------------------------------------------------------------------*/
/*---------------------------- Draw right border. ----------------------------*/

#if USE_LINES
        rect.left = rect.right = DestRect->right - 1;

        for (i = 0; i < BorderWidth; i++)
        {
            /* Draw one line. */
            gcmERR_BREAK(gcoSURF_Line(DestSurface,
                                  1, &rect,
                                  borderBrush,
                                  0xF0, 0xF0));

            /* Advance the rectangle. */
            rect.left  -= 1;
            rect.right -= 1;
        }
#else
        rect.left  = DestRect->right - BorderWidth;
        rect.right = DestRect->right;

        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                              1, gcvNULL, &rect,
                              borderBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));
#endif
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (borderBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(borderBrush));
    }

    return status;
}

/*******************************************************************************
**
**    DrawGradientBase
**
**    Draws a gradient base for a button or a window.
**
**    INPUT:
**
**        gco2D Engine
**            Pointer to an gco2D object.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**        gcsRECT_PTR DestRect
**            Destination rectangle.
**
**        gctUINT CornerRadius
**            If zero, a rectangular button will be drawn.
**            If not zero, the corners will be dircular with the specified radius.
**
**        gctUINT BorderWidth
**            The width of the border in pixels.
**
**        gctUINT32 TopColor
**        gctUINT32 BottomColor
**            Top of the button and bottom of the button colors for the gradient.
**
**        gctUINT32 BorderColor
**            The color of the border around the button.
**
**    OUTPUT:
**
**        Nothing.
*/
gceSTATUS DrawGradientBase(
    IN gco2D Engine,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT CornerRadius,
    IN gctUINT BorderWidth,
    IN gctUINT32 TopColor,
    IN gctUINT32 BottomColor,
    IN gctUINT32 BorderColor
    )
{
    gceSTATUS status;
    gcoBRUSH topBrush = gcvNULL;
    gcoBRUSH bottomBrush = gcvNULL;
    gcsRECT rect;
    gcsRECT destRect;

    do
    {
        /* Init destination rect if not provided. */
        if (DestRect == gcvNULL)
        {
            /* Init the destination origin. */
            destRect.left = 0;
            destRect.top  = 0;

            /* Query the size of the background surface. */
            gcmERR_BREAK(gcoSURF_GetSize(DestSurface,
                                     (gctUINT *)&destRect.right,
                                     (gctUINT *)&destRect.bottom,
                                     gcvNULL));

            /* Set the address of the rect. */
            DestRect = &destRect;
        }

        if (CornerRadius != 0)
        {
            /*
                Draw circled corners.
            */

            /* Construct top and bottom color brushes. */
            gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Engine,
                                                     gcvTRUE,
                                                     TopColor,
                                                     ~0,
                                                     &topBrush));

            gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Engine,
                                                     gcvTRUE,
                                                     BottomColor,
                                                     ~0,
                                                     &bottomBrush));

            /* Draw four filled circular corners in background color. */
            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->left + CornerRadius,
                                  DestRect->top + CornerRadius,
                                  CornerRadius, CornerRadius,
                                  TopColor,
                                  1,
                                  gcvTRUE,
                                  QUADRANT2));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->right - CornerRadius,
                                  DestRect->top + CornerRadius,
                                  CornerRadius, CornerRadius,
                                  TopColor,
                                  1,
                                  gcvTRUE,
                                  QUADRANT1));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->left + CornerRadius,
                                  DestRect->bottom - CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BottomColor,
                                  1,
                                  gcvTRUE,
                                  QUADRANT3));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->right - CornerRadius,
                                  DestRect->bottom - CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BottomColor,
                                  1,
                                  gcvTRUE,
                                  QUADRANT4));

            /* Draw top and bottom bars to match the corners. */
            rect.left   = DestRect->left  + CornerRadius;
            rect.top    = DestRect->top;
            rect.right  = DestRect->right - CornerRadius;
            rect.bottom = DestRect->top   + CornerRadius;

            gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                                  1, gcvNULL, &rect,
                                  topBrush,
                                  0xF0, 0xF0,
                                  gcvSURF_OPAQUE,
                                  ~0,
                                  gcvNULL, 0));

            rect.top    = DestRect->bottom - CornerRadius;
            rect.bottom = DestRect->bottom;

            gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                                  1, gcvNULL, &rect,
                                  bottomBrush,
                                  0xF0, 0xF0,
                                  gcvSURF_OPAQUE,
                                  ~0,
                                  gcvNULL, 0));

            /* Draw gradient background. */
            rect.left   = DestRect->left;
            rect.top    = DestRect->top    + CornerRadius;
            rect.right  = DestRect->right;
            rect.bottom = DestRect->bottom - CornerRadius;

            gcmERR_BREAK(DrawGradientRect(Engine, DestSurface,
                                       &rect,
                                       TopColor, BottomColor,
                                       gcvFALSE));

            /* Draw four circular corners in border color. */
            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->left + CornerRadius,
                                  DestRect->top + CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BorderColor,
                                  BorderWidth,
                                  gcvFALSE,
                                  QUADRANT2));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->right - CornerRadius,
                                  DestRect->top + CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BorderColor,
                                  BorderWidth,
                                  gcvFALSE,
                                  QUADRANT1));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->left + CornerRadius,
                                  DestRect->bottom - CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BorderColor,
                                  BorderWidth,
                                  gcvFALSE,
                                  QUADRANT3));

            gcmERR_BREAK(DrawEllipse(Engine,
                                  DestSurface,
                                  DestRect->right - CornerRadius,
                                  DestRect->bottom - CornerRadius,
                                  CornerRadius, CornerRadius,
                                  BorderColor,
                                  BorderWidth,
                                  gcvFALSE,
                                  QUADRANT4));
        }
        else
        {
            /*
                Draw standard rectangular corners.
            */

            /* Draw gradient background. */
            gcmERR_BREAK(DrawGradientRect(Engine, DestSurface,
                                       DestRect,
                                       TopColor, BottomColor,
                                       gcvFALSE));
        }

        /* Draw the border. */
        gcmERR_BREAK(DrawBorder(Engine, DestSurface,
                             DestRect,
                             BorderWidth, CornerRadius,
                             BorderColor));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (topBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(topBrush));
    }

    if (bottomBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(bottomBrush));
    }

    return status;
}

/*******************************************************************************
**
**    DrawButton
**
**    Draws a gradient button.
**
**    INPUT:
**
**        gco2D Engine
**            Pointer to an gco2D object.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**        gcsRECT_PTR DestRect
**            Destination rectangle.
**
**        gctUINT CornerRadius
**            If zero, a rectangular button will be drawn.
**            If not zero, the corners will be dircular with the specified radius.
**
**        gctUINT BorderWidth
**            The width of the border in pixels.
**
**        gctUINT32 TopColor
**        gctUINT32 BottomColor
**            Top of the button and bottom of the button colors for the gradient.
**
**        gctUINT32 BorderColor
**            The color of the border around the button.
**
**        gctUINT32 TextColor
**            The color of the button text.
**
**        gctUINT FontSelect
**            Integer index in the font array.
**
**        char * String
**            A pointer to the text to draw.
**
**    OUTPUT:
**
**        Nothing.
*/
gceSTATUS DrawButton(
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
    )
{
    gceSTATUS status;

    do
    {
        /* Draw the base of the button. */
        gcmERR_BREAK(DrawGradientBase(Engine,
                                   DestSurface,
                                   DestRect,
                                   CornerRadius,
                                   BorderWidth,
                                   TopColor,
                                   BottomColor,
                                   BorderColor));

        /* Draw the shadow of the text. */
        gcmERR_BREAK(DrawString(Engine,
                             DestSurface,
                             DestRect,
                             FontSelect,
                             String,
                             HCENTER_TEXT | VCENTER_TEXT | SHADOW_TEXT,
                             0, TextColor,
                             gcvNULL,
                             0xCC, 0xAA,
                             gcvSURF_SOURCE_MATCH));

        /* Draw the text. */
        gcmERR_BREAK(DrawString(Engine,
                             DestSurface,
                             DestRect,
                             FontSelect,
                             String,
                             HCENTER_TEXT | VCENTER_TEXT,
                             0, 0,
                             gcvNULL,
                             0xCC, 0xAA,
                             gcvSURF_SOURCE_MATCH));
    }
    while (gcvFALSE);

    return status;
}
