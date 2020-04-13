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




#ifndef __buttons_h_
#define __buttons_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
******************************* Button parameters *****************************
\******************************************************************************/

typedef enum _BUTTON_CONSTANTS
{
    AQBUTTON_PLAY,
    AQBUTTON_PAUSE,
    AQBUTTON_STOP,
    AQBUTTON_RECORD,
    AQBUTTON_REWIND,
    AQBUTTON_FAST_FORWARD,
    AQBUTTON_SKIP_BACK,
    AQBUTTON_SKIP_FORWARD,
    AQBUTTON_UP,
    AQBUTTON_DOWN,
}
BUTTON_CONSTANTS;


/******************************************************************************\
****************************** gcoBUTTONS Object *******************************
\******************************************************************************/

typedef struct _gcoBUTTONS * gcoBUTTONS;

/* Construct buttons object. */
gceSTATUS
gcoBUTTONS_Construct(
    IN gcoHAL Hal,
    IN gcoOS Os,
    IN OUT gcoBUTTONS * Buttons
    );

/* Destroy buttons object. */
gceSTATUS
gcoBUTTONS_Destroy(
    IN gcoBUTTONS Buttons
    );

/* Draw a button with an icon. */
gceSTATUS
gcoBUTTONS_DrawButton(
    IN gco2D Engine,
    IN gcoBUTTONS Buttons,
    IN gcoSURF DestSurface,
    IN gcsRECT_PTR DestRect,
    IN gctUINT CornerRadius,
    IN gctUINT32 TopColor,
    IN gctUINT32 BottomColor,
    IN gctUINT32 BorderColor,
    IN BUTTON_CONSTANTS Button
    );

#ifdef __cplusplus
}
#endif

#endif
