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
#include "font.h"
#include "text.h"

/******************************************************************************\
*********************************** Text API **********************************
\******************************************************************************/

/*******************************************************************************
**
**    GetTextSize
**
**    Calculates the width and the height of the text.
**
**    INPUT:
**
**        gctUINT FontSelect
**            Integer index in the font array.
**
**        char * String
**            A pointer to the text to draw.
**
**    OUTPUT:
**
**        gctINT * TextWidth
**        gctINT * TextHeight
**            Points to the width and the height of the text in pixels.
**
**        gctINT * VerOffset
**            Points to the vertical offset.
*/
gceSTATUS GetTextSize(
    IN gctUINT FontSelect,
    IN char * String,
    IN OUT gctINT * TextWidth,
    IN OUT gctINT * TextHeight,
    IN OUT gctINT * VerOffset
    )
{
    gcsFONTTABLE_PTR font;
    gcsGLYPH_PTR curGlyph;
    gctINT textWidth;
    gctINT verOffset;
    gctINT i;

    gcmHEADER_ARG("FontSelect=%d String=0x%x TextWidth=0x%x TextHeight=0x%x VerOffset=0x%x",
                FontSelect, String, TextWidth, TextHeight, VerOffset);
    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(FontSelect < gcmCOUNTOF(fontTableIndex));

    font = fontTableIndex[FontSelect];

    /* Compute combined width of the text. */
    textWidth = 0;
    verOffset = 0;
    for (i = 0; String[i] != '\0'; i++)
    {
        curGlyph = &font->glyph[(gctUINT8) String[i]];

        textWidth += curGlyph->cellIncX;

        if (curGlyph->glyphOriginY > verOffset)
        {
            verOffset = curGlyph->glyphOriginY;
        }
    }

    /* Set the result. */
    if (TextWidth != gcvNULL)
    {
        *TextWidth = textWidth;
    }

    if (TextHeight != gcvNULL)
    {
        *TextHeight = font->height;
    }

    if (VerOffset != gcvNULL)
    {
        *VerOffset = verOffset;
    }

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**    DrawString
**
**    Draws the specified text.
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
**        gctUINT FontSelect
**            Integer index in the font array.
**
**        char * String
**            A pointer to the text to draw.
**
**        TEXT_OPTIONS Options
*            A combination of TEXT_OPTIONS flags.
**
**        gctUINT32 FgColor
**            Foreground color for the text.
**
**        gctUINT32 BgColor
**            Background color for the text.
**
**        gcoBRUSH Brush
**            Brush to use with the text.
**
**        gctUINT8 FgRop
**            Foreground ROP.
**
**        gctUINT8 BgRop
**            Background ROP.
**
**        gceSURF_TRANSPARENCY Transparency
**            Text transparency.
**
**    OUTPUT:
**
**        Nothing.
*/

static gctUINT32 ReorderTo32(gctUINT8 *buf)
{
        return ((gctUINT32)buf[3] << 24) + ((gctUINT32)buf[2] << 16)
                + ((gctUINT32)buf[1] << 8) + (gctUINT32)buf[0];
}

gceSTATUS DrawString(
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
    )
{
    gceSTATUS status;
    gcsFONTTABLE_PTR font;
    gcsGLYPH_PTR curGlyph;
    gcsRECT clipRect;
    gcsRECT destRect;
    gctUINT8 * glyphData;
    gcsPOINT glyphSize;
    gctINT textWidth, textHeight;
    gctINT verOffset;
    gctINT x, y;
    gctINT i;

    /* Verify the arguments. */
    gcmHEADER_ARG("String=0x%x, DestRect=0x%x", String, DestRect);
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(DestRect != gcvNULL);
    gcmVERIFY_ARGUMENT(FontSelect < gcmCOUNTOF(fontTableIndex));

    do
    {
        /* Get the size of the text. */
        gcmERR_BREAK(GetTextSize(FontSelect, String,
                              &textWidth, &textHeight, &verOffset));

        /* Shortcut to the font. */
        font = fontTableIndex[FontSelect];

        /* Determine horizontal position. */
        switch (Options & HORIZONTAL_TEXT_ALIGNMENT)
        {
        case LEFT_TEXT:
            x = DestRect->left;
            break;

        case HCENTER_TEXT:
            x = DestRect->left
              + (DestRect->right - DestRect->left - textWidth) / 2;
            break;

        case RIGHT_TEXT:
            x = DestRect->right - textWidth;
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* Determine vertical position. */
        switch (Options & VERTICAL_TEXT_ALIGNMENT)
        {
        case TOP_TEXT:
            y = DestRect->top;
            break;

        case VCENTER_TEXT:
            y = DestRect->top
              + (DestRect->bottom - DestRect->top - textHeight) / 2;
            break;

        case BOTTOM_TEXT:
            y = DestRect->bottom - textHeight;
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        y += verOffset;

        /* Set clipping rectangle to the destination. */
        clipRect.left   = DestRect->left;
        clipRect.top    = DestRect->top;
        clipRect.right  = DestRect->right;
        clipRect.bottom = DestRect->bottom;

        /* Adjust for shadows. */
        if (Options & SHADOW_TEXT)
        {
            /* Adjust the origin.  */
            x += SHADOW_OFFSET;
            y += SHADOW_OFFSET;

            /* Adjust clipping. */
            clipRect.left   += SHADOW_OFFSET;
            clipRect.top    += SHADOW_OFFSET;
            clipRect.right  += SHADOW_OFFSET;
            clipRect.bottom += SHADOW_OFFSET;
        }

        /* Set clipping. */
        gcmERR_BREAK(gco2D_SetClipping(Engine, &clipRect));

        /* Assume success. */
        status = gcvSTATUS_OK;

        /* Go through the string character by character. */
        for (i = 0; String[i] != '\0'; i++)
        {
            curGlyph = &font->glyph[(gctUINT8) String[i]];

            if (curGlyph->size)
            {
                gctUINT j;
                gctUINT glyphDataSize = curGlyph->size;

                /* Compute the glyph location. */
                glyphData = (gctUINT8 *)malloc(glyphDataSize);
                memcpy(glyphData, font->data + curGlyph->offset, glyphDataSize);

                for (j = 0; j < glyphDataSize / sizeof(gctUINT32); j++)
                {
                        gctUINT32 *p = (gctUINT32 *)glyphData;
                        p[j] = (gctUINT32)ReorderTo32((gctUINT8 *)&p[j]);
                }



                /* Init destination rectangle. */
                destRect.left   = x + curGlyph->glyphOriginX;
                destRect.top    = y - curGlyph->glyphOriginY;
                destRect.right  = destRect.left + curGlyph->blackBoxX;
                destRect.bottom = destRect.top  + curGlyph->blackBoxY;

                /* Init the size of the glyph bitmap. */
                glyphSize.x = curGlyph->blackBoxX;
                glyphSize.y = curGlyph->blackBoxY;

                /* Execute the blit. */
                gcmERR_BREAK(gcoSURF_MonoBlit(
                    DestSurface,
                    glyphData,
                    gcvSURF_PACKED32,
                    &glyphSize,
                    gcvNULL,
                    &destRect,
                    Brush,
                    FgRop, BgRop,
                    gcvTRUE,
                    0, Transparency,
                    FgColor, BgColor
                    ));

                x += curGlyph->cellIncX;
                y += curGlyph->cellIncY;

                free(glyphData);
            }
            else
            {
                x += curGlyph->cellIncX;
                y += curGlyph->cellIncY;
            }
        }
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**    DrawShadowedString
**
**    Draws the specified text with a shadow.
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
**        gctUINT FontSelect
**            Integer index in the font array.
**
**        char * String
**            A pointer to the text to draw.
**
**        TEXT_OPTIONS Options
*            A combination of TEXT_OPTIONS flags.
**
**        gctUINT32 TextColor
**            Color of the text.
**
**        gctUINT32 ShadowColor
**            Shadow color of the text.
**
**        gcoBRUSH Brush
**            Brush to use with the text.
**
**        gctUINT8 FgRop
**            Foreground ROP.
**
**        gctUINT8 BgRop
**            Background ROP.
**
**    OUTPUT:
**
**        Nothing.
*/
gceSTATUS DrawShadowedString(
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
    )
{
    gceSTATUS status;

    do
    {
        /* Draw the shadow first. */
        gcmERR_BREAK(DrawString(Engine,
                             DestSurface,
                             DestRect,
                             FontSelect,
                             String,
                             Options | SHADOW_TEXT,
                             ShadowColor,
                             ~0,                    /* Not using bg color. */
                             gcvNULL,
                             0xCC, 0xAA,
                             gcvSURF_SOURCE_MATCH));

        /* Now draw the text in foreground color. */
        gcmERR_BREAK(DrawString(Engine,
                             DestSurface,
                             DestRect,
                             FontSelect,
                             String,
                             Options & ~SHADOW_TEXT,
                             TextColor,
                             ~0,                    /* Not using bg color. */
                             Brush,
                             FgRop, BgRop,
                             gcvSURF_SOURCE_MATCH));
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}
