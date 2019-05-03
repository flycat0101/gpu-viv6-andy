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


#ifndef __gc_sdk_internal_h_
#define __gc_sdk_internal_h_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************\
*************************** Intrernal Object Pointers. *************************
\******************************************************************************/

typedef struct _gcsAPPLICATION *    gcsAPPLICATION_PTR;
typedef struct _gcsVISUAL *            gcsVISUAL_PTR;
typedef struct _gcsCOMMONVISUAL *    gcsCOMMONVISUAL_PTR;


/******************************************************************************\
**************************** Thread info structure. ****************************
\******************************************************************************/

typedef struct _gcsTHREADINFO * gcsTHREADINFO_PTR;

struct _gcsTHREADINFO
{
    gctTHREADROUTINE routine;
    gctPOINTER argument;
};


/******************************************************************************\
****************************** Common Structures. ******************************
\******************************************************************************/

struct _gcsCOMMONVISUAL
{
    /* Desktop size. */
    gctUINT desktopWidth;
    gctUINT desktopHeight;

    /* Message handlers. */
    gctPOINTER         eventArgument;
    gctKEYBOARDEVENT   keyboardEvent;
    gctMOUSEPRESSEVENT mousePressEvent;
    gctMOUSEMOVEEVENT  mouseMoveEvent;
    gctTIMERFUNC       timerEvent;
    gctUINT            timerDelay;

    /* Visual system mutex. */
    gctHANDLE visualMutex;

    /* Main window. */
    gctUINT mainWindowDepth;

    /* Main window image. */
    gctUINT mainWindowImageWidth;
    gctUINT mainWindowImageHeight;
};


/******************************************************************************\
***************************** Intrernal Functions. *****************************
\******************************************************************************/

gcsAPPLICATION_PTR
ivdkGetApplication(
    void
    );

gcsVISUAL_PTR
ivdkGetVisual(
    void
    );

void
ivdkLockVisual(
    void
    );

void
ivdkUnlockVisual(
    void
    );

gctBOOL
ivdkServeTimer(
    void
    );


#ifdef __cplusplus
}
#endif

#endif /* __gc_sdk_internal_h_ */
