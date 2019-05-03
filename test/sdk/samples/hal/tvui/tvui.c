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




#include "PreComp.h"
#include "bitmap.h"
#include "window.h"
#include "buttons.h"
#include "gradient.h"
#include "text.h"

#ifndef IMAGE_PREFIX
#    define IMAGE_PREFIX "images/"
#endif

/******************************************************************************\
******************************* Test structures. ******************************
\******************************************************************************/

typedef struct _AQTEST * AQTEST;
typedef struct _AQPROGRAMINFO * AQPROGRAMINFO;
typedef struct _AQCCINFO * AQCCINFO;

struct _AQPROGRAMINFO
{
    char * name;
    gctUINT channel;
    struct _gcsTIME startTime;
    struct _gcsTIME endTime;
    struct _gcsTIME curPosition;
};

struct _AQTEST
{
    gctHANDLE workerThread;
    gctHANDLE updateEvent;
    gctBOOL terminateApp;

    gcoOS os;
    gcoHAL hal;
    gco2D engine;
    gcoSURF resolveBuffer;
    gcoSURF frameBuffer;
    gcoSURF progInfoBg;
    gcoSURF posInfoBg;
    gcoSURF captionBg;
    gcoBUTTONS buttons;

    gctPHYS_ADDR internalPhysical;
    gctPHYS_ADDR externalPhysical;
    gctPHYS_ADDR contiguousPhysical;
    gctSIZE_T internalSize;
    gctSIZE_T externalSize;
    gctSIZE_T contiguousSize;
    gctPOINTER internalMemory;
    gctPOINTER externalMemory;
    gctPOINTER contiguousMemory;

    gctBOOL alphaSupport;

    /* Support PE 2.0. */
    gctBOOL hw2DPE20;

    /* Frame buffer size. */
    gctUINT frameWidth;
    gctUINT frameHeight;

    /* Distance from the edge of the frame buffer to the info window. */
    gctUINT frameSpacing;

    /* Width of the information window border. */
    gctUINT wndBorderWidth;

    /* Position info window: offset of the divider line. */
    gctUINT dividerYoffset;

    /* Position info window: height of the button bar at the bottom. */
    gctUINT buttonBarHeight;

    /* Position info window: offset of the live position bar. */
    gctUINT posBarXoffset;
    gctUINT posBarYoffset;

    /* Position info window: height of the live position bar. */
    gctUINT posBarHeight;

    struct _AQPROGRAMINFO program;

    gctUINT frameNumber;
};

struct _AQCCINFO
{
    int hour;
    int min;
    int sec;
    char * text;
};

/******************************************************************************\
***************************** Global definitions. *****************************
\******************************************************************************/

char AppName[] = "TV User Interface Sample";

char * WeekDays[] =
{
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

char * Months[] =
{
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

struct _AQCCINFO CCInfo[] =
{
    {0, 20, 13, "Are you with the angel?"},
    {0, 20, 50, "Do you see an angel?"},
    {0, 22, 13, "We don't know how it works with angles..."},
    {0, 31, 11, "How do you do?"},
    {0, 55, 40, "Battle!"},
    {0, 57, 33, "Michael!?"},
    {0, 57, 50, "I am completely happy."},
    {1, 12, 42, "I'll be with you in a minute."},
    {1, 20, 33, "All you need is love..."},
    {1, 40, 21, "...five hippopotamus..."},
};

struct _AQTEST UiTest;

const gctUINT g_ProgInfoWidth  = 480;
const gctUINT g_ProgInfoHeight = 80;

const gctUINT g_PosInfoWidth  = 550;
const gctUINT g_PosInfoHeight = 120;

const gctUINT g_PIPWidth  = 240;
const gctUINT g_PIPHeight = 135;

const gctUINT g_CCHeight = 48;


/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/

/*******************************************************************************
**
**    _GetTimeDifference
**
**  Calculate the time difference.
**
**  INPUT:
**
**        gcsTIME_PTR StartTime
**          Start time.
**
**        gcsTIME_PTR EndTime
**          End time.
**
**  OUTPUT:
**
**        gcsTIME_PTR DiffTime
**          The time difference.
*/
static void _GetTimeDifference(
    IN gcsTIME_PTR StartTime,
    IN gcsTIME_PTR EndTime,
    IN OUT gcsTIME_PTR DiffTime
    )
{
    gctINT carryOver;

    DiffTime->second = EndTime->second - StartTime->second;

    if (DiffTime->second < 0)
    {
        DiffTime->second += 60;
        carryOver = 1;
    }
    else
    {
        carryOver = 0;
    }

    DiffTime->minute = EndTime->minute - StartTime->minute - carryOver;

    if (DiffTime->minute < 0)
    {
        DiffTime->minute += 60;
        carryOver = 1;
    }
    else
    {
        carryOver = 0;
    }

    DiffTime->hour = EndTime->hour - StartTime->hour - carryOver;
}

/*******************************************************************************
**
**    _GetViewOffset
**
**  Calculate the offset of the current viewing position in pixels.
**
**  INPUT:
**
**        gcsTIME_PTR TimeLength
**          The length of the segment in time units.
**
**        gcsTIME_PTR TimeOffset
**          Current viewing offset in time units.
**
**        gctINT PixelWidth
**            The width of the bar in pixels.
**
**  RETURN:
**
**        gctINT
**          The offset of the current viewing position in pixels.
*/
static gctINT _GetViewOffset(
    IN gcsTIME_PTR TimeLength,
    IN gcsTIME_PTR TimeOffset,
    IN gctINT PixelWidth
    )
{
    gctINT totalSeconds;
    gctINT currSeconds;
    gctINT pixelOffset;

    /* Compute the number of seconds in the segment. */
    totalSeconds
        = TimeLength->hour * 60 * 60
        + TimeLength->minute * 60
        + TimeLength->second;

    /* Compute the number of seconds watched. */
    currSeconds
        = TimeOffset->hour * 60 * 60
        + TimeOffset->minute * 60
        + TimeOffset->second;

    /* Compute the offset in pixels. */
    pixelOffset = PixelWidth * currSeconds / totalSeconds;

    /* Return the result. */
    return pixelOffset;
}

/*******************************************************************************
**
**    _EnableSrcOver
**
**  Enables alpha blending in source over mode with global alpha value.
**
**  INPUT:
**
**        gcoSURF Surface
**            Pointer to an gcoSURF object.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS _EnableSrcOver(
    IN AQTEST Test,
    IN gcoSURF Surface
    )
{
    if (!Test->hw2DPE20)
    {
        return gcoSURF_EnableAlphaBlend
        (
            /*
                Surface pointer.
            */
            Surface,

            /*
                Global alpha values - not used.
            */
            0, 0,

            /*
                Per-pixel alpha value modes.
            */
            gcvSURF_PIXEL_ALPHA_STRAIGHT, gcvSURF_PIXEL_ALPHA_STRAIGHT,

            /*
                Global alpha value modes.
            */
            gcvSURF_GLOBAL_ALPHA_OFF, gcvSURF_GLOBAL_ALPHA_OFF,

            /*
                Blending factor mode; source over defines source as 1 and
                destination as inversed.
            */
            gcvSURF_BLEND_ONE, gcvSURF_BLEND_INVERSED,

            /*
                Per-pixel color component mode. Treat source pixels as
                non-premultiplied.
            */
            gcvSURF_COLOR_MULTIPLY, gcvSURF_COLOR_STRAIGHT
        );
    }
    else {
        gceSTATUS status;

        do
        {
            gcmERR_BREAK(gco2D_EnableAlphaBlendAdvanced(Test->engine,
                            gcvSURF_PIXEL_ALPHA_STRAIGHT,
                            gcvSURF_PIXEL_ALPHA_STRAIGHT,
                            gcvSURF_GLOBAL_ALPHA_OFF,
                            gcvSURF_GLOBAL_ALPHA_OFF,
                            gcvSURF_BLEND_ONE,
                            gcvSURF_BLEND_INVERSED
                            ));

            gcmERR_BREAK(gco2D_SetPixelMultiplyModeAdvanced(Test->engine,
                            gcv2D_COLOR_MULTIPLY_ENABLE,
                            gcv2D_COLOR_MULTIPLY_DISABLE,
                            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                            gcv2D_COLOR_MULTIPLY_DISABLE
                            ));
        } while (gcvFALSE);

        return status;
    }
}

/******************************************************************************\
********************************* AQTEST code *********************************
\******************************************************************************/

/*******************************************************************************
**
**    FilterBlit
**
**  Blit an image from the specified file to the specified surface rectangle.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**        gcsRECT_PTR DestRect
**            Destination rectangle.
**
**        char * FileName
**            The name of the image file.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS FilterBlit(
    IN AQTEST Test,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN char * FileName
    )
{
    gceSTATUS status;
    gcoSURF frame = gcvNULL;
    char fullName[256];

    /* Add the path to the file name. */
    sprintf(fullName, IMAGE_PREFIX "%s", FileName);

    do
    {
        /* Load the bitmap into a surface. */
        gcmERR_BREAK(gcoBITMAP_LoadSurface(Test->hal, Test->os, fullName, &frame));

        /* Set kernel size. */
        gcmERR_BREAK(gco2D_SetKernelSize(Test->engine, 9, 9));

        /* Blit the frame to the destination. */
        gcmERR_BREAK(gcoSURF_FilterBlit(frame, DestSurface,
                                    gcvNULL, DestRect, gcvNULL));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (frame != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(frame));
    }

    return status;
}

/*******************************************************************************
**
**    DrawClosedCaptionBackground
**
**  Draws the background for CC test.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawClosedCaptionBackground(
    IN AQTEST Test
    )
{
    gceSTATUS status;
    gcoBRUSH bgBrush = gcvNULL;

    const gctUINT32 alpha = 0x80;

    do
    {
        /* Create background brush. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 (alpha << 24) | 0x000000,
                                                 ~0,
                                                 &bgBrush));

        /* Draw the box. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->captionBg,
                              1, gcvNULL, gcvNULL,
                              bgBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (bgBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(bgBrush));
    }

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**    DrawClosedCaption
**
**  Draws a box with closed caption text in it.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**        char * Text
**            The text to draw.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawClosedCaption(
    IN AQTEST Test,
    IN gcoSURF DestSurface,
    IN char * Text
    )
{
    gceSTATUS status;
    gctINT txWidth, txHeight;
    gcsRECT srcRect;
    gcsRECT boxRect;

    const gctUINT horSpacing = 15;
    const gctUINT verSpacing = 4;

    const gctUINT ccFont = 4;

    do
    {
        gctUINT posInfoWidth;
        gctUINT posInfoHeight;

        /* Get the size of the position info surface. */
        gcmERR_BREAK(gcoSURF_GetSize(Test->posInfoBg,
                                     &posInfoWidth,
                                     &posInfoHeight,
                                     gcvNULL));

        /* Get the size of the text. */
        gcmERR_BREAK(GetTextSize(ccFont, Text, &txWidth, &txHeight, gcvNULL));

        /* Init the source rect for the background surface. */
        srcRect.left   = 0;
        srcRect.top    = 0;
        srcRect.right  = horSpacing * 2 + txWidth;
        srcRect.bottom = verSpacing * 2 + txHeight;

        /* Init the black box rectangle. */
        boxRect.left
            = (Test->frameWidth - posInfoWidth) / 2;
        boxRect.top
            = Test->frameHeight
            - Test->frameSpacing * 2
            - posInfoHeight
            - srcRect.bottom;

        boxRect.right  = boxRect.left + srcRect.right;
        boxRect.bottom = boxRect.top  + srcRect.bottom;

        /* Enable a source over blending. */
        _EnableSrcOver(Test, DestSurface);

        /* Draw the box. */
        gcmERR_BREAK(gcoSURF_Blit(Test->captionBg, DestSurface,
                              1, &srcRect, &boxRect,
                              gcvNULL,
                              0xCC, 0xAA,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Disable blending. */
        gcoSURF_DisableAlphaBlend(DestSurface);
        if (Test->hw2DPE20)
        {
            gco2D_SetPixelMultiplyModeAdvanced(Test->engine,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE
                    );
        }

        /* Draw the text. */
        gcmERR_BREAK(DrawString(Test->engine,
                             DestSurface,
                             &boxRect,
                             ccFont,
                             Text,
                             HCENTER_TEXT | VCENTER_TEXT,
                             0x00E7E7E7,
                             ~0,
                             gcvNULL,
                             0xCC, 0xAA,
                             gcvSURF_SOURCE_MATCH));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**    DrawPictureInPicture
**
**  Draws the PIP window with live video.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**        char * FileName
**            The name of the image file.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawPictureInPicture(
    IN AQTEST Test,
    IN char * FileName
    )
{
    gceSTATUS status;
    gcsRECT winRect;

    const gctUINT outerBorderWidth = 3;
    const gctUINT innerBorderWidth = 2;

    const gctUINT32 outerBorderColor = 0x0000FF00;
    const gctUINT32 innerBorderColor = 0x00000000;

    do
    {
        gctUINT progInfoHeight;

        /* Get the size of the program info. */
        gcmERR_BREAK(gcoSURF_GetSize(Test->progInfoBg,
                                     gcvNULL,
                                     &progInfoHeight,
                                     gcvNULL));

        /* Determine the rectangle. */
        winRect.left   = Test->frameWidth - Test->frameSpacing * 2 - g_PIPWidth;
        winRect.top    = Test->frameSpacing * 3 + progInfoHeight;
        winRect.right  = winRect.left + g_PIPWidth;
        winRect.bottom = winRect.top  + g_PIPHeight;

        /* Draw the outer border. */
        gcmERR_BREAK(DrawBorder(Test->engine,
                             Test->frameBuffer,
                             &winRect,
                             outerBorderWidth,
                             0,
                             outerBorderColor));

        /* Adjust the rectangle. */
        winRect.left   += outerBorderWidth;
        winRect.top    += outerBorderWidth;
        winRect.right  -= outerBorderWidth;
        winRect.bottom -= outerBorderWidth;

        /* Draw the outer border. */
        gcmERR_BREAK(DrawBorder(Test->engine,
                             Test->frameBuffer,
                             &winRect,
                             innerBorderWidth,
                             0,
                             innerBorderColor));

        /* Adjust the rectangle. */
        winRect.left   += innerBorderWidth;
        winRect.top    += innerBorderWidth;
        winRect.right  -= innerBorderWidth;
        winRect.bottom -= innerBorderWidth;

        /* Blit the frame. */
        gcmERR_BREAK(FilterBlit(Test, Test->frameBuffer, &winRect, FileName));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    return status;
}

/*******************************************************************************
**
**    DrawProgramInfoBackground
**
**  Draws the background for program information window.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawProgramInfoBackground(
    IN AQTEST Test
    )
{
    gceSTATUS status;
    gcoBRUSH clearBrush = gcvNULL;

    const gctUINT32 alpha = 0x80;

    do
    {
        /* Create brush to clear the surface. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 Test->alphaSupport
                                                    ? 0x00000000
                                                    : 0x0000FF00,
                                                 ~0,
                                                 &clearBrush));

        /* Clear the surface. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->progInfoBg,
                              1, gcvNULL, gcvNULL,
                              clearBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Draw the window base. */
        gcmERR_BREAK(DrawGradientBase(Test->engine,
                                   Test->progInfoBg,
                                   gcvNULL,            /* Rect = whole surface. */
                                   8,                /* Corner radius. */
                                   Test->wndBorderWidth,
                                   (alpha << 24) | 0x112769,    /* Top color. */
                                   (alpha << 24) | 0x111D44,    /* Botom color. */
                                   (alpha << 24) | 0xCCCCCC));    /* Border color. */

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (clearBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(clearBrush));
    }

    return status;
}

/*******************************************************************************
**
**    DrawProgramInfo
**
**  Draws a box with information about the program currently being viewed.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawProgramInfo(
    IN AQTEST Test,
    IN gcoSURF DestSurface
    )
{
    gceSTATUS status;
    gcsRECT destRect;
    gcsRECT logoRect;
    gcsRECT nameRect;
    gcsRECT airedRect;
    char logoFileName[256];
    char aired[256];
    gctUINT wndWidth, wndHeight;
    gctINT txWidth, txHeight;
    gctINT logoWidth, logoHeight;

    const gctUINT logoSpacing = 7;

    const gctUINT textTopSpacing    = 13;
    const gctUINT textSideSpacing   = 13;
    const gctUINT textBottomSpacing = 13;

    const gctUINT nameFont  = 2;
    const gctUINT airedFont = 1;

    const gctUINT textColor = 0x00F0F0F0;

    do
    {
        /* Get the size of the window. */
        gcmERR_BREAK(gcoSURF_GetSize(Test->progInfoBg,
                                 &wndWidth,
                                 &wndHeight,
                                 gcvNULL));

        /* Init the destination rectangle. */
        destRect.left   = (Test->frameWidth - wndWidth) / 2;
        destRect.top    =  Test->frameSpacing;
        destRect.right  = destRect.left + wndWidth;
        destRect.bottom = destRect.top  + wndHeight;

        /* Enable a source over blending. */
        _EnableSrcOver(Test, DestSurface);

        /* Draw the background.
           If alpha blending is supported, the transparency setup will
           be ignored and the blit will be blended with the destination.
           If alpha blending is not supported, the transparency setup
           will come into play making sure the ellipse corners are
           properly blitted. */
        gcmERR_BREAK(gcoSURF_Blit(Test->progInfoBg, DestSurface,
                              1, gcvNULL, &destRect,
                              gcvNULL,
                              0xCC, 0xAA,
                              gcvSURF_SOURCE_MATCH,
                              0x0000FF00,
                              gcvNULL, 0));

        /* Disable blending. */
        gcoSURF_DisableAlphaBlend(DestSurface);
        if (Test->hw2DPE20)
        {
            gco2D_SetPixelMultiplyModeAdvanced(Test->engine,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE
                    );
        }

        /* Init the logo rectangle. */
        logoHeight = wndHeight - 2 * logoSpacing;
        logoWidth  = (gctINT)(((float)logoHeight * 4.0) / 3.0 + 0.5);

        logoRect.left   = destRect.left + logoSpacing;
        logoRect.top    = destRect.top  + logoSpacing;
        logoRect.right  = logoRect.left + logoWidth;
        logoRect.bottom = logoRect.top  + logoHeight;

        /* Construct logo file name. */
        sprintf(logoFileName, "%03d.bmp", Test->program.channel);

        /* Blit the logo. */
        gcmERR_BREAK(FilterBlit(Test,
                             DestSurface,
                             &logoRect,
                             logoFileName));

        /* Get the size of the text. */
        gcmERR_BREAK(GetTextSize(nameFont, Test->program.name,
                              &txWidth, &txHeight, gcvNULL));

        /* Init the text rectangle. */
        nameRect.left   = logoRect.right + textSideSpacing;
        nameRect.top    = destRect.top   + textTopSpacing;
        nameRect.right  = destRect.right - textSideSpacing;
        nameRect.bottom = nameRect.top   + txHeight;

        /* Draw the text. */
        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &nameRect,
                                     nameFont,
                                     Test->program.name,
                                     LEFT_TEXT | TOP_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Format the aired string. */
        sprintf(aired, "Aired: %s, %s %d %d:%02d - %d:%02d",
            WeekDays[Test->program.startTime.weekday],
            Months[Test->program.startTime.month],
            Test->program.startTime.day,
            Test->program.startTime.hour,
            Test->program.startTime.minute,
            Test->program.endTime.hour,
            Test->program.endTime.minute);

        /* Get the size of the text. */
        gcmERR_BREAK(GetTextSize(airedFont, aired, &txWidth, &txHeight, gcvNULL));

        /* Format the aired string rectangle. */
        airedRect.left   = nameRect.left;
        airedRect.right  = nameRect.right;
        airedRect.bottom = destRect.bottom - textBottomSpacing;
        airedRect.top    = airedRect.bottom - txHeight;

        /* Draw the text. */
        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &airedRect,
                                     airedFont,
                                     aired,
                                     LEFT_TEXT | TOP_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    return status;
}

/*******************************************************************************
**
**    DrawPositionInfoBackground
**
**  Draws the background for position information window.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawPositionInfoBackground(
    IN AQTEST Test
    )
{
    gceSTATUS status;

    gcoBRUSH clearBrush = gcvNULL;
    gcoBRUSH dividerBrush = gcvNULL;
    gcoBRUSH posBarBrush  = gcvNULL;

    gcsRECT destRect;
    gcsRECT dividerRect;
    gcsRECT posBarTopRect;
    gcsRECT posBarBottomRect;

    const gctUINT32 alpha = 0x80;

    do
    {
        /* Create brush to clear the surface. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 Test->alphaSupport
                                                    ? 0x00000000
                                                    : 0x0000FF00,
                                                 ~0,
                                                 &clearBrush));

        /* Init the destination rectangle. */
        destRect.left = 0;
        destRect.top  = 0;

        gcmERR_BREAK(gcoSURF_GetSize(Test->posInfoBg,
                                 (gctUINT *) &destRect.right,
                                 (gctUINT *) &destRect.bottom,
                                 gcvNULL));

        /* Clear the surface. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->posInfoBg,
                              1, gcvNULL, gcvNULL,
                              clearBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Draw the window base. */
        gcmERR_BREAK(DrawGradientBase(Test->engine,
                                   Test->posInfoBg,
                                   &destRect,
                                   10,                /* Corner radius. */
                                   Test->wndBorderWidth,
                                   (alpha << 24) | 0x112667,    /* Top color. */
                                   (alpha << 24) | 0x111D44,    /* Botom color. */
                                   (alpha << 24) | 0xCCCCCC));    /* Border color. */

        /* Create the divider brush. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 (alpha << 24) | 0xCCCCCC,
                                                 ~0,
                                                 &dividerBrush));

        /* Init the divider line rectangle. */
        dividerRect.left   = destRect.left   + Test->wndBorderWidth;
        dividerRect.top    = destRect.top    + Test->dividerYoffset;
        dividerRect.right  = destRect.right  - Test->wndBorderWidth;
        dividerRect.bottom = dividerRect.top + Test->wndBorderWidth;

        /* Draw the divider. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->posInfoBg,
                              1, gcvNULL, &dividerRect,
                              dividerBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Create the position bar background brush. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 (alpha << 24) | 0x66616B,
                                                 ~0,
                                                 &posBarBrush));

        /* Init the position bar background top rectangle. */
        posBarTopRect.left   = dividerRect.left;
        posBarTopRect.top    = dividerRect.bottom;
        posBarTopRect.right  = dividerRect.right;
        posBarTopRect.bottom = posBarTopRect.top + Test->posBarYoffset;

        /* Draw the bar background. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->posInfoBg,
                              1, gcvNULL, &posBarTopRect,
                              posBarBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Init the position bar background top rectangle. */
        posBarBottomRect.left   = posBarTopRect.left;
        posBarBottomRect.top    = posBarTopRect.bottom + Test->posBarHeight;
        posBarBottomRect.right  = posBarTopRect.right;
        posBarBottomRect.bottom = destRect.bottom - Test->buttonBarHeight;

        /* Draw the bar background. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, Test->posInfoBg,
                              1, gcvNULL, &posBarBottomRect,
                              posBarBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (dividerBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(dividerBrush));
    }

    if (posBarBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(posBarBrush));
    }

    if (clearBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(clearBrush));
    }

    /* Return status. */
    return status;
}

/*******************************************************************************
**
**    DrawPositionInfo
**
**  Draws a box with current position information within the currently viewed
**    program.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**        gcoSURF DestSurface
**            Pointer to the destination surface.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DrawPositionInfo(
    IN AQTEST Test,
    IN gcoSURF DestSurface
    )
{
    gceSTATUS status;
    gcoBRUSH currPosBrush = gcvNULL;

    gcsRECT destRect;
    gcsRECT segmentStartTimeRect;
    gcsRECT segmentEndTimeRect;
    gcsRECT watchedTopRect;
    gcsRECT watchedBottomRect;
    gcsRECT toBeWatchedTopRect;
    gcsRECT toBeWatchedBottomRect;
    gcsRECT rect;

    struct _gcsTIME currTime;
    struct _gcsTIME timeDiff;
    char strBuffer[256];
    gctUINT wndWidth, wndHeight;
    gctUINT topPosBarY1, topPosBarY2;
    gctUINT bottomPosBarY1, bottomPosBarY2;
    gctINT posBarWidth, viewOffset;

    const gctUINT nameFont = 2;
    const gctUINT wndFont  = 1;

    const gctUINT buttonSpacing = 5;
    const gctUINT buttonWidth   = 23;

    const gctUINT textColor = 0x00F0F0F0;

    do
    {
/*----------------------------------------------------------------------------*/
/*----------------------------- Draw the bar base ----------------------------*/

        /* Get the size of the window. */
        gcmERR_BREAK(gcoSURF_GetSize(Test->posInfoBg,
                                 &wndWidth,
                                 &wndHeight,
                                 gcvNULL));

        /* Init the destination rectangle. */
        destRect.left   = (Test->frameWidth - wndWidth) / 2;
        destRect.bottom =  Test->frameHeight - Test->frameSpacing;
        destRect.right  = destRect.left   + wndWidth;
        destRect.top    = destRect.bottom - wndHeight;

        /* Compute gray position bar locations. */
        topPosBarY1 = destRect.top + Test->dividerYoffset + Test->wndBorderWidth;
        topPosBarY2 = topPosBarY1 + Test->posBarYoffset;

        bottomPosBarY1 = topPosBarY2 + Test->posBarHeight;
        bottomPosBarY2 = destRect.bottom - Test->buttonBarHeight;

        /* Enable a source over blending. */
        _EnableSrcOver(Test, DestSurface);

        /* Draw the background.
           If alpha blending is supported, the transparency setup will
           be ignored and the blit will be blended with the destination.
           If alpha blending is not supported, the transparency setup
           will come into play making sure the ellipse corners are
           properly blitted. */
        gcmERR_BREAK(gcoSURF_Blit(Test->posInfoBg, DestSurface,
                              1, gcvNULL, &destRect,
                              gcvNULL,
                              0xCC, 0xAA,
                              gcvSURF_SOURCE_MATCH,
                              0x0000FF00,
                              gcvNULL, 0));

        /* Disable blending. */
        gcoSURF_DisableAlphaBlend(DestSurface);
        if (Test->hw2DPE20)
        {
            gco2D_SetPixelMultiplyModeAdvanced(Test->engine,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE,
                    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                    gcv2D_COLOR_MULTIPLY_DISABLE
                    );
        }

/*----------------------------------------------------------------------------*/
/*--------------------- Draw segment start and end times ---------------------*/

        /* Init the segment start time rectangle. */
        segmentStartTimeRect.left   = destRect.left + Test->wndBorderWidth;
        segmentStartTimeRect.top    = topPosBarY2;
        segmentStartTimeRect.right  = segmentStartTimeRect.left
                                    + Test->posBarXoffset;
        segmentStartTimeRect.bottom = bottomPosBarY1;

        /* Draw the text. */
        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &segmentStartTimeRect,
                                     wndFont,
                                     "0:00:00",
                                     HCENTER_TEXT | VCENTER_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Determine the length of the program. */
        _GetTimeDifference(&Test->program.startTime,
                           &Test->program.endTime,
                           &timeDiff);

        /* Format segment end time string. */
        sprintf(strBuffer, "%d:%02d:%02d",
                timeDiff.hour,
                timeDiff.minute,
                timeDiff.second);

        /* Init the segment end time rectangle. */
        segmentEndTimeRect.right  = destRect.right - Test->wndBorderWidth;
        segmentEndTimeRect.top    = topPosBarY2;
        segmentEndTimeRect.left   = segmentEndTimeRect.right
                                  - Test->posBarXoffset;
        segmentEndTimeRect.bottom = bottomPosBarY1;

        /* Draw the text. */
        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &segmentEndTimeRect,
                                     wndFont,
                                     strBuffer,
                                     HCENTER_TEXT | VCENTER_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

/*----------------------------------------------------------------------------*/
/*--------------------------- Draw the timeline bar --------------------------*/

        /* Find the pixel coordinate of the current viewing position. */
        posBarWidth = segmentEndTimeRect.left - segmentStartTimeRect.right;
        viewOffset = _GetViewOffset(&timeDiff,
                                    &Test->program.curPosition,
                                    posBarWidth);

        /* Init the watched portion top and bottom rectangles. */
        watchedTopRect.left   = segmentStartTimeRect.right;
        watchedTopRect.top    = topPosBarY2;
        watchedTopRect.right  = watchedTopRect.left + viewOffset;
        watchedTopRect.bottom = watchedTopRect.top + Test->posBarHeight / 2;

        watchedBottomRect.left   = watchedTopRect.left;
        watchedBottomRect.top    = watchedTopRect.bottom;
        watchedBottomRect.right  = watchedTopRect.right;
        watchedBottomRect.bottom = bottomPosBarY1;

        /* Draw the watched portion as a gradient. */
        gcmERR_BREAK(DrawGradientRect(Test->engine,
                                   DestSurface,
                                   &watchedTopRect,
                                   0x00159D15,
                                   0x008DFD8D,
                                   gcvFALSE));

        gcmERR_BREAK(DrawGradientRect(Test->engine,
                                   DestSurface,
                                   &watchedBottomRect,
                                   0x008DFD8D,
                                   0x00159D15,
                                   gcvFALSE));

        /* Init the to be watched portion top and bottom rectangles. */
        toBeWatchedTopRect.left   = watchedTopRect.right;
        toBeWatchedTopRect.top    = topPosBarY2;
        toBeWatchedTopRect.right  = segmentEndTimeRect.left;
        toBeWatchedTopRect.bottom = watchedTopRect.bottom;

        toBeWatchedBottomRect.left   = toBeWatchedTopRect.left;
        toBeWatchedBottomRect.top    = toBeWatchedTopRect.bottom;
        toBeWatchedBottomRect.right  = toBeWatchedTopRect.right;
        toBeWatchedBottomRect.bottom = watchedBottomRect.bottom;

        /* Draw the to be watched portion as a gradient. */
        gcmERR_BREAK(DrawGradientRect(Test->engine,
                                   DestSurface,
                                   &toBeWatchedTopRect,
                                   0x008A8A8A,
                                   0x00FDFDFD,
                                   gcvFALSE));

        gcmERR_BREAK(DrawGradientRect(Test->engine,
                                   DestSurface,
                                   &toBeWatchedBottomRect,
                                   0x00FDFDFD,
                                   0x008A8A8A,
                                   gcvFALSE));

/*----------------------------------------------------------------------------*/
/*-------------------- Draw the current position indicator -------------------*/

        /* Init the currently watched position rectangle. */
        rect.left   = watchedTopRect.right - 1;
        rect.top    = watchedTopRect.top;
        rect.right  = watchedTopRect.right + 1;
        rect.bottom = watchedBottomRect.bottom;

        /* Create the position bar background brush. */
        gcmERR_BREAK(gco2D_ConstructSingleColorBrush(Test->engine,
                                                 gcvTRUE,
                                                 0x00FF0000,
                                                 ~0,
                                                 &currPosBrush));

        /* Draw the current position indicator. */
        gcmERR_BREAK(gcoSURF_Blit(gcvNULL, DestSurface,
                              1, gcvNULL, &rect,
                              currPosBrush,
                              0xF0, 0xF0,
                              gcvSURF_OPAQUE,
                              ~0,
                              gcvNULL, 0));

        /* Init the current position time rectangle. */
        rect.left   = watchedTopRect.right - 50;
        rect.right  = watchedTopRect.right + 50;
        rect.top    = topPosBarY1;
        rect.bottom = watchedTopRect.top;

        /* Format the time position string. */
        if (Test->program.curPosition.hour == 0)
        {
            sprintf(strBuffer, "%02d:%02d",
                    Test->program.curPosition.minute,
                    Test->program.curPosition.second);
        }
        else
        {
            sprintf(strBuffer, "%d:%02d:%02d",
                    Test->program.curPosition.hour,
                    Test->program.curPosition.minute,
                    Test->program.curPosition.second);
        }

        /* Draw the current time position. */
        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &rect,
                                     wndFont,
                                     strBuffer,
                                     HCENTER_TEXT | VCENTER_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

/*----------------------------------------------------------------------------*/
/*------------------------- Draw the channel control -------------------------*/

        /* Draw the up button. */
        rect.left   = destRect.left + Test->wndBorderWidth + buttonSpacing;
        rect.top    = destRect.top  + Test->wndBorderWidth + buttonSpacing;
        rect.right  = rect.left + buttonWidth;
        rect.bottom = destRect.top  + Test->dividerYoffset - buttonSpacing;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_UP));

        /* Draw the channel number indicator. */
        sprintf(strBuffer, "%d", Test->program.channel);

        rect.left   = rect.right + buttonSpacing;
        rect.top    = destRect.top + Test->wndBorderWidth + 2;
        rect.right  = rect.left + 50;
        rect.bottom = destRect.top + Test->dividerYoffset - 2;

        gcmERR_BREAK(DrawGradientBase(Test->engine,
                                   DestSurface,
                                   &rect,
                                   2,                /* Corner radius. */
                                   1,
                                   0x00F0F0F0,        /* Top color. */
                                   0x00F0F0F0,        /* Botom color. */
                                   0x00CCCCCC));    /* Border color. */

        gcmERR_BREAK(DrawString(Test->engine,
                             DestSurface,
                             &rect,
                             wndFont,
                             strBuffer,
                             HCENTER_TEXT | VCENTER_TEXT,
                             0x00000000,
                             ~0,
                             gcvNULL,
                             0xCC, 0xAA,
                             gcvSURF_SOURCE_MATCH));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

        /* Draw the up button. */
        rect.left   = rect.right + buttonSpacing;
        rect.top    = destRect.top + Test->wndBorderWidth + buttonSpacing;
        rect.right  = rect.left + buttonWidth;
        rect.bottom = destRect.top + Test->dividerYoffset - buttonSpacing;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_DOWN));

/*----------------------------------------------------------------------------*/
/*-------------------------- Draw the program name ---------------------------*/

        /* Draw the program name. */
        rect.left   = rect.right + 10;
        rect.top    = destRect.top   + Test->wndBorderWidth;
        rect.right  = destRect.right - Test->wndBorderWidth - 10;
        rect.bottom = destRect.top   + Test->dividerYoffset;

        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &rect,
                                     nameFont,
                                     Test->program.name,
                                     LEFT_TEXT | VCENTER_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

/*----------------------------------------------------------------------------*/
/*------------------------------ Draw the buttons ----------------------------*/

        rect.left   = destRect.left + Test->wndBorderWidth + buttonSpacing;
        rect.top    = bottomPosBarY2 + buttonSpacing;
        rect.right  = rect.left + buttonWidth;
        rect.bottom = destRect.bottom - Test->wndBorderWidth - buttonSpacing;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_SKIP_BACK));

        rect.left  = rect.right + buttonSpacing;
        rect.right = rect.left  + buttonWidth;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_REWIND));

        rect.left  = rect.right + buttonSpacing;
        rect.right = rect.left  + buttonWidth;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_STOP));

        rect.left  = rect.right + buttonSpacing;
        rect.right = rect.left  + buttonWidth;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_PAUSE));

        rect.left  = rect.right + buttonSpacing;
        rect.right = rect.left  + buttonWidth;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_FAST_FORWARD));

        rect.left  = rect.right + buttonSpacing;
        rect.right = rect.left  + buttonWidth;

        gcmERR_BREAK(gcoBUTTONS_DrawButton(Test->engine,
                                       Test->buttons,
                                       DestSurface,
                                       &rect,
                                       4,
                                       0x00858CAE,
                                       0x00484276,
                                       0x00B5B5C0,
                                       AQBUTTON_SKIP_FORWARD));

/*----------------------------------------------------------------------------*/
/*----------------------- Draw the current time string. ----------------------*/

        /* Draw the current time string. */
        vdkGetCurrentTime(&currTime);
        sprintf(strBuffer, "%s, %s %d %d:%02d",
            WeekDays[currTime.weekday],
            Months[currTime.month],
            currTime.day,
            currTime.hour,
            currTime.minute);

        rect.right  = destRect.right - Test->wndBorderWidth - 10;
        rect.left   = rect.right - 200;
        rect.top    = bottomPosBarY2;
        rect.bottom = destRect.bottom - Test->wndBorderWidth;

        gcmERR_BREAK(DrawShadowedString(Test->engine,
                                     DestSurface,
                                     &rect,
                                     wndFont,
                                     strBuffer,
                                     RIGHT_TEXT | VCENTER_TEXT,
                                     textColor,
                                     0x00000000,
                                     gcvNULL,
                                     0xCC, 0xAA));

        /* Text drawing changes the clipping, reset to default. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

        /* Flush. */
        gcmERR_BREAK(gcoHAL_Commit(Test->hal, gcvFALSE));
    }
    while (gcvFALSE);

    /* Cleanup. */
    if (currPosBrush != gcvNULL)
    {
        gcmVERIFY_OK(gcoBRUSH_Destroy(currPosBrush));
    }

    /* Return status. */
    return status;
}

gceSTATUS Resolve(
    AQTEST Test
    )
{
    gceSTATUS status;
    gcoSURF surface;
    gctPOINTER bits;
    gctUINT32 address;
    gctUINT width, height;
    gctUINT alignedWidth, alignedHeight;

    do
    {
        /* Copy the frame buffer to the resolve surface if needed. */
        if (Test->resolveBuffer == gcvNULL)
        {
            surface = Test->frameBuffer;
        }
        else
        {
            gcmERR_BREAK(gcoSURF_FilterBlit(
                Test->frameBuffer, Test->resolveBuffer,
                gcvNULL, gcvNULL, gcvNULL
                ));

            surface = Test->resolveBuffer;
        }

        /* Flush the frame buffer. */
        gcmERR_BREAK(gcoSURF_Flush(surface));

        /* Lock the frame buffer. */
        gcmVERIFY_OK(gcoSURF_Lock(surface, &address, &bits));

        /* Wait for hardware idle. */
        gcmVERIFY_OK(gcoHAL_Commit(Test->hal, gcvTRUE));

        /* Get the size of the surface. */
        gcmVERIFY_OK(gcoSURF_GetSize(surface, &width, &height, gcvNULL));
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(surface,
                                            &alignedWidth,
                                            &alignedHeight,
                                            gcvNULL));

        /* Display the image. */
        vdkSetMainWindowImage(width, height,
                              alignedWidth, alignedHeight,
                              bits);

        /* Unlock the frame buffer. */
        gcmVERIFY_OK(gcoSURF_Unlock(surface, bits));
    }
    while (gcvFALSE);

    return status;
}

/*******************************************************************************
**
**  DoTestStep
**
**  Draws the next frame.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS DoTestStep(
    AQTEST Test
    )
{
    gceSTATUS status;
    AQCCINFO curCC;
    char frameName[256];
    char pipName[256];

    do
    {
/*----------------------------------------------------------------------------*/
/*------------------------------ Draw the frame. -----------------------------*/

        /* No more frames to do? */
        if (Test->frameNumber >= gcmCOUNTOF(CCInfo))
        {
            Test->frameNumber = 0;
        }

        /* Shortcut to the current CC. */
        curCC = &CCInfo[Test->frameNumber];

        /* Update current position. */
        Test->program.curPosition.hour = curCC->hour;
        Test->program.curPosition.minute  = curCC->min;
        Test->program.curPosition.second  = curCC->sec;

        /* Form the current frame and pip names. */
        sprintf(frameName, "frame%02d.bmp", Test->frameNumber);
        sprintf(pipName,   "pip%02d.bmp",   Test->frameNumber);

        /* Reset clipping. */
        gcmERR_BREAK(gco2D_SetClipping(Test->engine, gcvNULL));

        /* Blit the frame. */
        gcmERR_BREAK(FilterBlit(Test, Test->frameBuffer, gcvNULL, frameName));

        /* Blit the CC. */
        gcmERR_BREAK(DrawClosedCaption(Test, Test->frameBuffer, curCC->text));

        /* Draw PIP. */
        gcmERR_BREAK(DrawPictureInPicture(Test, pipName));

        /* Blit the program info. */
        gcmERR_BREAK(DrawProgramInfo(Test, Test->frameBuffer));

        /* Blit the position info. */
        gcmERR_BREAK(DrawPositionInfo(Test, Test->frameBuffer));

        /* Advance to the next frame. */
        Test->frameNumber++;

/*----------------------------------------------------------------------------*/
/*--------------------------------- Resolve. ---------------------------------*/

        gcmERR_BREAK(Resolve(Test));

    }
    while (gcvFALSE);

    return status;
}

/*******************************************************************************
**
**  InitializeTest
**
**  Initialize the main test structure.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
**
*/
gctBOOL InitializeTest(
    AQTEST Test
    )
{
    /* Reset the test structure. */
    memset(Test, 0, sizeof(struct _AQTEST));

    /* Init test defaults. */
    Test->frameWidth      = 640;
    Test->frameHeight     = 480;
    Test->frameSpacing    = 10;
    Test->wndBorderWidth  = 2;
    Test->dividerYoffset  = 33;
    Test->buttonBarHeight = 34;
    Test->posBarXoffset   = 80;
    Test->posBarYoffset   = 20;
    Test->posBarHeight    = 19;

    /* Init program info. */
    Test->program.name = "Michael";
    Test->program.channel = 105;

    Test->program.startTime.year = 2007;
    Test->program.startTime.month = 1;
    Test->program.startTime.day = 8;
    Test->program.startTime.weekday = 1;
    Test->program.startTime.hour = 20;
    Test->program.startTime.minute = 0;
    Test->program.startTime.second = 0;

    Test->program.endTime.year = 2007;
    Test->program.endTime.month = 1;
    Test->program.endTime.day = 8;
    Test->program.endTime.weekday = 1;
    Test->program.endTime.hour = 22;
    Test->program.endTime.minute = 0;
    Test->program.endTime.second = 0;

    Test->frameNumber = 0;


    /* Create update event. */
    Test->updateEvent = vdkCreateEvent(gcvFALSE, gcvFALSE);
    if (Test->updateEvent == gcvNULL)
    {
        return gcvFALSE;
    }

    /* Success. */
    return gcvTRUE;
}

/*******************************************************************************
**
**  CleanupTest
**
**  Cleanup the main test structure.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
**
*/
void CleanupTest(
    AQTEST Test
    )
{
    if (Test->updateEvent != gcvNULL)
    {
        vdkCloseEvent(Test->updateEvent);
        Test->updateEvent = gcvNULL;
    }
}

/*******************************************************************************
**
**  InitializeSystem
**
**  Initialize the system objects.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
**
*/
gctBOOL InitializeSystem(
    AQTEST Test
    )
{
    gceSTATUS status;
    gctUINT greenBits;
    gceSURF_FORMAT format;
    gctUINT rlvWidth, rlvHeight;

    do
    {
        /* Query window size. */
        vdkGetMainWindowSize(&rlvWidth, &rlvHeight);

        /* Determine the resolve surface format. */
        vdkGetMainWindowColorBits(gcvNULL, &greenBits, gcvNULL);

        if (greenBits == 5)
        {
            format = gcvSURF_A1R5G5B5;
        }
        else if (greenBits == 6)
        {
            format = gcvSURF_R5G6B5;
        }
        else if (greenBits == 8)
        {
            format = gcvSURF_A8R8G8B8;
        }
        else
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Make sure the frame size is big enough. */
        {
            gctUINT minFrameWidth
                = gcmMAX(
                    gcmMAX(g_ProgInfoWidth, g_PosInfoWidth),
                    g_PIPWidth
                    );

            gctUINT minFrameHeight
                = Test->frameSpacing
                + g_ProgInfoHeight
                + Test->frameSpacing * 2
                + g_PIPHeight
                + Test->frameSpacing
                + g_PosInfoHeight
                + Test->frameSpacing;

            if ((Test->frameWidth  < minFrameWidth) ||
                (Test->frameHeight < minFrameHeight))
            {
                status = gcvSTATUS_NOT_SUPPORTED;
                break;
            }
        }

        /* Create OS object. */
        gcmERR_BREAK(gcoOS_Construct(gcvNULL, &Test->os));

        /* Create HAL object. */
        gcmERR_BREAK(gcoHAL_Construct(gcvNULL, Test->os, &Test->hal));

                {
                    gceCHIPMODEL chipModel;
                    gctUINT32 chipRev;

                    gcmERR_BREAK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRev, gcvNULL, gcvNULL));

                    if (chipModel == gcv320 && chipRev == 0x5007)
                    {
                        printf("Hardware gc320 5007 does not support monoblit.\n");
                        status = gcvSTATUS_NOT_SUPPORTED;
                        break;
                    }
                }

        /* Check for the PE 2.0 feature. */
        Test->hw2DPE20 = gcoHAL_IsFeatureAvailable(Test->hal, gcvFEATURE_2DPE20);

        /* Query the size and location of video memory. */
        gcmERR_BREAK(gcoHAL_QueryVideoMemory(
            Test->hal,
            &Test->internalPhysical,
            &Test->internalSize,
            &Test->externalPhysical,
            &Test->externalSize,
            &Test->contiguousPhysical,
            &Test->contiguousSize
            ));

        /* Map internal video memory. */
        if (Test->internalSize > 0)
        {
            gcmERR_BREAK(gcoHAL_MapMemory(
                Test->hal,
                Test->internalPhysical,
                Test->internalSize,
                &Test->internalMemory
                ));
        }

        /* Map external video memory. */
        if (Test->externalSize > 0)
        {
            gcmERR_BREAK(gcoHAL_MapMemory(
                Test->hal,
                Test->externalPhysical,
                Test->externalSize,
                &Test->externalMemory
                ));
        }

        /* Map contiguous video memory. */
        if (Test->contiguousSize > 0)
        {
            gcmERR_BREAK(gcoHAL_MapMemory(
                Test->hal,
                Test->contiguousPhysical,
                Test->contiguousSize,
                &Test->contiguousMemory
                ));
        }

        /* Create the frame buffer surface. */
        gcmERR_BREAK(gcoSURF_Construct(
            Test->hal,
            Test->frameWidth,
            Test->frameHeight,
            1,
            gcvSURF_BITMAP,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &Test->frameBuffer
            ));

        /* Create the resolve surface if needed. */
        if (format != gcvSURF_A8R8G8B8 ||
            Test->frameWidth != rlvWidth ||
            Test->frameHeight != rlvHeight)
        {
            gcmERR_BREAK(gcoSURF_Construct(
                Test->hal,
                rlvWidth,
                rlvHeight,
                1,
                gcvSURF_BITMAP,
                format,
                gcvPOOL_DEFAULT,
                &Test->resolveBuffer
                ));
        }

        /* Create the program info surface. */
        gcmERR_BREAK(gcoSURF_Construct(
            Test->hal,
            g_ProgInfoWidth,
            g_ProgInfoHeight,
            1,
            gcvSURF_BITMAP,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &Test->progInfoBg
            ));

        /* Create the position info surface. */
        gcmERR_BREAK(gcoSURF_Construct(
            Test->hal,
            g_PosInfoWidth,
            g_PosInfoHeight,
            1,
            gcvSURF_BITMAP,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &Test->posInfoBg
            ));

        /* Create the position info surface. */
        gcmERR_BREAK(gcoSURF_Construct(
            Test->hal,
            Test->frameWidth,    /* Maximum CC width. */
            g_CCHeight,            /* Maximum CC height. */
            1,
            gcvSURF_BITMAP,
            gcvSURF_A8R8G8B8,
            gcvPOOL_DEFAULT,
            &Test->captionBg
            ));

        /* Create buttons object. */
        gcmERR_BREAK(gcoBUTTONS_Construct(Test->hal, Test->os, &Test->buttons));

        /* Determine whether alpha blending is supported. */
        Test->alphaSupport =
            (gcoSURF_DisableAlphaBlend(Test->frameBuffer) == gcvSTATUS_OK);
    }
    while (gcvFALSE);

    return gcmIS_SUCCESS(status);
}

/*******************************************************************************
**
**  CleanupSystem
**
**  Cleanup the system objects.
**
**  INPUT:
**
**        AQTEST Test
**          Pointer to an AQTEST structure.
**
**  OUTPUT:
**
**      Nothing.
**
*/
void CleanupSystem(
    AQTEST Test
    )
{
    if (Test->hal != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_Commit(Test->hal, gcvTRUE));
    }

    if (Test->contiguousMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_UnmapMemory(
            Test->hal,
            Test->contiguousPhysical,
            Test->contiguousSize,
            Test->contiguousMemory
            ));

        Test->contiguousMemory = gcvNULL;
    }

    if (Test->externalMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_UnmapMemory(
            Test->hal,
            Test->externalPhysical,
            Test->externalSize,
            Test->externalMemory
            ));

        Test->externalMemory = gcvNULL;
    }

    if (Test->internalMemory != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_UnmapMemory(
            Test->hal,
            Test->internalPhysical,
            Test->internalSize,
            Test->internalMemory
            ));

        Test->internalMemory = gcvNULL;
    }

    if (Test->hal != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(Test->hal));
        Test->hal = gcvNULL;
    }

    if (Test->os != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_Destroy(Test->os));
        Test->os = gcvNULL;
    }
}

/*******************************************************************************
**
**  Worker thread definition.
**
*/

void
TVUIWorker(
    gctPOINTER Argument
    )
{
    /* Get a pointer to the test info. */
    AQTEST test = (AQTEST) Argument;

    /* Extract the gco2D object pointer. */
    if (gcmIS_ERROR(gcoHAL_Get2DEngine(test->hal, &test->engine)))
    {
        return;
    }

    /* Set clipping. */
    if (gcmIS_ERROR(gco2D_SetClipping(test->engine, gcvNULL)))
    {
        return;
    }

    /* Perdraw constant images. */
    if (gcmIS_ERROR(DrawClosedCaptionBackground(test))
        || gcmIS_ERROR(DrawProgramInfoBackground(test))
        || gcmIS_ERROR(DrawPositionInfoBackground(test)))
    {
        return;
    }

    /* Enter the loop. */
    while (gcvTRUE)
    {
        /* Wait for the update event. */
        vdkWaitForEvent(test->updateEvent, gcvINFINITE);

        /* Termination requested? */
        if (test->terminateApp)
        {
            break;
        }

        /* Update the frame. */
        gcmVERIFY_OK(DoTestStep(test));
    }

    if (test->engine != gcvNULL)
    {
        gcmVERIFY_OK(gco2D_FreeFilterBuffer(test->engine));
    }

    if (test->buttons != gcvNULL)
    {
        gcmVERIFY_OK(gcoBUTTONS_Destroy(test->buttons));
        test->buttons = gcvNULL;
    }

    if (test->captionBg != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(test->captionBg));
        test->captionBg = gcvNULL;
    }

    if (test->posInfoBg != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(test->posInfoBg));
        test->posInfoBg = gcvNULL;
    }

    if (test->progInfoBg != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(test->progInfoBg));
        test->progInfoBg = gcvNULL;
    }

    if (test->resolveBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(test->resolveBuffer));
        test->resolveBuffer = gcvNULL;
    }

    if (test->frameBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(test->frameBuffer));
        test->frameBuffer = gcvNULL;
    }
}

gctBOOL
StartWorker(
    AQTEST Test
    )
{
    /* Start the thread. */
    Test->workerThread = vdkCreateThread(TVUIWorker, Test);

    /* Return result. */
    return (Test->workerThread != gcvNULL);
}

void
StopWorker(
    AQTEST Test
    )
{
    if (Test->workerThread != gcvNULL)
    {
        /* Schedule the thread to stop. */
        Test->terminateApp = gcvTRUE;
        vdkSetEvent(Test->updateEvent);

        /* Start the thread. */
        vdkCloseThread(Test->workerThread);
        Test->workerThread = gcvNULL;
    }
}

/*******************************************************************************
**
**  handleXXX
**
**  Main window message handlers.
**
*/

gctBOOL
HandleKeyboard(
    gctPOINTER EventArgument,
    gctUINT State,
    gctUINT Code
    )
{
    /* Terminate if Esc is pressed. */
    return (Code != 27);
}

gctBOOL
HandleButtons(
    gctPOINTER EventArgument,
    gctINT X,
    gctINT Y,
    gctUINT State,
    gctUINT Code
    )
{
    /* Terminate if left button is pressed. */
    return (Code != gcvLEFTBUTTON);
}

gctBOOL
HandleTimer(
    gctPOINTER EventArgument
    )
{
    /* Get a pointer to the test info. */
    AQTEST test = (AQTEST) EventArgument;

    /* Release the worker thread. */
    vdkSetEvent(test->updateEvent);
    return gcvTRUE;
}

/*******************************************************************************
**
**  vdkAppEntry
**
**  Test entry point.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURN:
**
**      int
**            Exit value.
*/
int vdkAppEntry(
    void
    )
{
    /* Assume failure. */
    int result = -1;

    do
    {
        /* Initialize visual system. */
        if (!vdkInitVisual())
        {
            break;
        }

        /* Initialize the test. */
        if (!InitializeTest(&UiTest))
        {
            break;
        }

        /* Create the main window. */
        if (!vdkCreateMainWindow(-1, -1,
                                 UiTest.frameWidth, UiTest.frameHeight,
                                 AppName,
                                 &UiTest,
                                 HandleKeyboard,
                                 HandleButtons,
                                 gcvNULL,
                                 HandleTimer, 3))
        {
            break;
        }

        /* Inform the user that the initialization is in progress. */
        vdkSetMainWindowPostTitle(": initializing the application...");

        /* Initialize the test. */
        if (!InitializeSystem(&UiTest))
        {
            gcmFATAL("Failed to initialize the graphics system.");
            vdkDestroyMainWindow();
            break;
        }

        /* Reset the title. */
        vdkSetMainWindowPostTitle(gcvNULL);

        /* Start the worker thread. */
        if (StartWorker(&UiTest))
        {
            /* Enter the message loop. */
            result = vdkEnterMainWindowLoop();

            /* Stop worker thread. */
            StopWorker(&UiTest);
        }
    }
    while (gcvFALSE);

    /* Cleanup the system objects. */
    CleanupSystem(&UiTest);

    /* Cleanup the test environment. */
    CleanupTest(&UiTest);

    /* Cleanup the visual system. */
    vdkCleanupVisual();

    return result;
}
