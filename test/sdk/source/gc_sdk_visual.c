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


#include <gc_sdk.h>
#include "gc_sdk_internal.h"


/******************************************************************************\
*************************** Common Internal Functions **************************
\******************************************************************************/

void
ivdkLockVisual(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsCOMMONVISUAL_PTR visual = (gcsCOMMONVISUAL_PTR) ivdkGetVisual();

    /* Acquire the mutex. */
    if (visual->visualMutex != gcvNULL)
    {
        vdkAcquireMutex(visual->visualMutex, gcvINFINITE);
    }
}

void
ivdkUnlockVisual(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsCOMMONVISUAL_PTR visual = (gcsCOMMONVISUAL_PTR) ivdkGetVisual();

    /* Release the mutex. */
    if (visual->visualMutex != gcvNULL)
    {
        vdkReleaseMutex(visual->visualMutex);
    }
}

gctBOOL
ivdkServeTimer(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsCOMMONVISUAL_PTR visual = (gcsCOMMONVISUAL_PTR) ivdkGetVisual();

    /* Serve the event. */
    return (visual->timerEvent == gcvNULL)
        ? gcvTRUE
        : (*visual->timerEvent) (visual->eventArgument);
}


/******************************************************************************\
****************************** API Implementation *****************************
\******************************************************************************/

/*******************************************************************************
**
**  vdkGetDesktopSize
**
**  Get the size of the desktop.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**        gctUINT* Width
**        gctUINT* Height
**            Pointers to the area size.
**
*/

void
vdkGetDesktopSize(
    IN OUT gctUINT* Width,
    IN OUT gctUINT* Height
    )
{
    /* Get a pointer to the visual object. */
    gcsCOMMONVISUAL_PTR visual = (gcsCOMMONVISUAL_PTR) ivdkGetVisual();

    /* Retrieve the size. */
    *Width  = visual->desktopWidth;
    *Height = visual->desktopHeight;
}
