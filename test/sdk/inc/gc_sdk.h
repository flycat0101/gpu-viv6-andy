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


#ifndef __gc_sdk_h_
#define __gc_sdk_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
***************************** Include all headers. ****************************
\******************************************************************************/

#if defined(LINUX) || defined(__QNXNTO__)
#    include <signal.h>
#    include <unistd.h>
#    include <string.h>
#    include <time.h>
#    include <sys/time.h>
#elif defined _WIN32
#    include <windows.h>
#    include <tchar.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gc_hal.h>
#include <gc_hal_raster.h>
#if VIVANTE_ENABLE_3D
#include <gc_hal_engine.h>
#endif

/******************************************************************************\
********************************* Definitions. ********************************
\******************************************************************************/

#define gcvLEFTBUTTON    (1 << 0)
#define gcvMIDDLEBUTTON    (1 << 1)
#define gcvRIGHTBUTTON    (1 << 2)
#define gcvCTRL            (1 << 3)
#define gcvALT            (1 << 4)
#define gcvSHIFT        (1 << 5)

/******************************************************************************\
****************************** Type declarations. *****************************
\******************************************************************************/

/*
    Window timer event function type.
*/
typedef gctBOOL (* gctTIMERFUNC) (
    gctPOINTER Argument
    );

/*
    Keyboard key press event function type.
*/
typedef gctBOOL (* gctKEYBOARDEVENT) (
    gctPOINTER Argument,
    gctUINT State,
    gctUINT Code
    );

/*
    Mouse button press event function type.
*/
typedef gctBOOL (* gctMOUSEPRESSEVENT) (
    gctPOINTER Argument,
    gctINT X,
    gctINT Y,
    gctUINT State,
    gctUINT Code
    );

/*
    Mouse move event function type.
*/
typedef gctBOOL (* gctMOUSEMOVEEVENT) (
    gctPOINTER Argument,
    gctINT X,
    gctINT Y,
    gctUINT State
    );

/*
    Thread routine.
*/
typedef void (* gctTHREADROUTINE) (
    gctPOINTER Argument
    );

/*
    Time structure.
*/
struct _gcsTIME
{
    gctINT year;
    gctINT month;
    gctINT day;
    gctINT weekday;
    gctINT hour;
    gctINT minute;
    gctINT second;
};

typedef struct _gcsTIME * gcsTIME_PTR;

/******************************************************************************\
******************************* API declarations. *****************************
\******************************************************************************/

int
vdkAppEntry(
    void
    );

void
vdkGetCurrentTime(
    gcsTIME_PTR CurrTime
    );

/******************************************************************************\
********************************** Visual API. ********************************
\******************************************************************************/

gctBOOL
vdkInitVisual(
    void
    );

void
vdkCleanupVisual(
    void
    );

void
vdkGetDesktopSize(
    IN OUT gctUINT* Width,
    IN OUT gctUINT* Height
    );

gctBOOL
vdkCreateMainWindow(
    IN gctINT X,
    IN gctINT Y,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctSTRING Title,
    IN gctPOINTER EventArgument,
    IN gctKEYBOARDEVENT KeyboardEvent,
    IN gctMOUSEPRESSEVENT MousePressEvent,
    IN gctMOUSEMOVEEVENT MouseMoveEvent,
    IN gctTIMERFUNC TimerEvent,
    IN gctUINT TimerDelay
    );

void
vdkDestroyMainWindow(
    void
    );

gctHANDLE
vdkGetDisplayContext(
    void
    );

void
vdkReleaseDisplayContext(
    gctHANDLE Context
    );

gctHANDLE
vdkGetMainWindowHandle(
    void
    );

void
vdkGetMainWindowSize(
    IN OUT gctUINT* Width,
    IN OUT gctUINT* Height
    );

void
vdkGetMainWindowColorBits(
    IN OUT gctUINT* RedCount,
    IN OUT gctUINT* GreenCount,
    IN OUT gctUINT* BlueCount
    );

void
vdkSetMainWindowPostTitle(
    gctSTRING PostTitle
    );

void
vdkSetMainWindowImage(
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT AlignedWidth,
    IN gctUINT AlignedHeight,
    IN gctPOINTER Image
    );

int
vdkEnterMainWindowLoop(
    void
    );

/******************************************************************************\
*********************** Threading and Synchronization API. ********************
\******************************************************************************/

void
vdkSleep(
    gctUINT Milliseconds
    );

gctHANDLE
vdkCreateThread(
    gctTHREADROUTINE ThreadRoutine,
    gctPOINTER Argument
    );

void
vdkCloseThread(
    gctHANDLE ThreadHandle
    );

gctHANDLE
vdkCreateEvent(
    gctBOOL ManualReset,
    gctBOOL InitialState
    );

void
vdkCloseEvent(
    gctHANDLE EventHandle
    );

void
vdkSetEvent(
    gctHANDLE EventHandle
    );

void
vdkResetEvent(
    gctHANDLE EventHandle
    );

void
vdkWaitForEvent(
    gctHANDLE EventHandle,
    gctUINT Timeout
    );

gctHANDLE
vdkCreateMutex(
    void
    );

void
vdkCloseMutex(
    gctHANDLE MutexHandle
    );

void
vdkAcquireMutex(
    gctHANDLE MutexHandle,
    gctUINT Timeout
    );

void
vdkReleaseMutex(
    gctHANDLE MutexHandle
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_sdk_h_ */
