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


#ifdef __VXWORKS__

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$FBDEV\n";

struct _vdkPrivate
{
    vdkDisplay  display;
    int xres;
    int yres;
};

static vdkPrivate _vdk = NULL;

/*******************************************************************************
** Initialization.
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    vdkPrivate  vdk = NULL;

    vdk = (vdkPrivate) malloc(sizeof (struct _vdkPrivate));

    if (vdk == NULL)
    {
        return NULL;
    }

    vdk->display = (EGLNativeDisplayType) NULL;

    _vdk = vdk;
    return vdk;


}

VDKAPI void VDKLANG
vdkExit(
    vdkPrivate Private
    )
{
    if (Private != NULL)
    {
        if (_vdk == Private)
        {
            _vdk = NULL;
        }

        free(Private);
    }
}

/*******************************************************************************
** Display.
*/

VDKAPI vdkDisplay VDKLANG
vdkGetDisplayByIndex(
    vdkPrivate Private,
    int DisplayIndex
    )
{
    if (!Private)
    {
        return NULL;
    }

    /* Support only one display currently. */
    if (Private->display != (EGLNativeDisplayType) NULL)
    {
        return Private->display;
    }

    Private->display = fbGetDisplayByIndex(DisplayIndex);

    if (Private->display)
    {
        fbGetDisplayInfo(Private->display,
            &Private->xres, &Private->yres, NULL, NULL, NULL);
    }

    return Private->display;
}

VDKAPI vdkDisplay VDKLANG
vdkGetDisplay(
    vdkPrivate Private
    )
{
    return vdkGetDisplayByIndex(Private, 0);
}

VDKAPI int VDKLANG
vdkGetDisplayInfo(
    vdkDisplay Display,
    int * Width,
    int * Height,
    unsigned long * Physical,
    int * Stride,
    int * BitsPerPixel
    )
{
    if (!Display)
    {
        return 0;
    }

    fbGetDisplayInfo(Display, Width, Height, Physical, Stride, BitsPerPixel);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    fbDestroyDisplay(Display);

    if (_vdk->display == Display)
    {
        _vdk->display = NULL;
    }
}

/*******************************************************************************
** Windows
*/

vdkWindow
vdkCreateWindow(
    vdkDisplay Display,
    int X,
    int Y,
    int Width,
    int Height
    )
{
    NativeWindowType win;
    win = fbCreateWindow(Display, X, Y, Width, Height);

    return win;
}

VDKAPI int VDKLANG
vdkGetWindowInfo(
    vdkWindow Window,
    int * X,
    int * Y,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    unsigned int * Offset
    )
{
    fbGetWindowInfo(Window, X, Y, Width, Height, BitsPerPixel, Offset);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    fbDestroyWindow(Window);
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    return 1;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    return 1;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
}

VDKAPI void VDKLANG
vdkCapturePointer(
    vdkWindow Window
    )
{
}

/*******************************************************************************
** Events.
*/
VDKAPI int VDKLANG
vdkGetEvent(
    vdkWindow Window,
    vdkEvent * Event
    )
{
    return 0;
}

/*******************************************************************************
** EGL Support. ****************************************************************
*/

EGL_ADDRESS
vdkGetAddress(
    vdkPrivate Private,
    const char * Function
    )
{
    return (EGL_ADDRESS) eglGetProcAddress(Function);
}

/*******************************************************************************
** Time. ***********************************************************************
*/

/*
    vdkGetTicks

    Get the number of milliseconds since the system started.

    PARAMETERS:

        None.

    RETURNS:

        unsigned int
            The number of milliseconds the system has been running.
*/
VDKAPI unsigned int VDKLANG
vdkGetTicks(
    void
    )
{
    struct timespec tv;

    clock_gettime(CLOCK_REALTIME, &tv);

    return (tv.tv_sec * 1000000) + (tv.tv_nsec + 500 / 1000);
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    return fbCreatePixmapWithBpp(Display,
                                 Width, Height, BitsPerPixel);
}

VDKAPI int VDKLANG
vdkGetPixmapInfo(
    vdkPixmap Pixmap,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    int * Stride,
    void ** Bits
    )
{
    fbGetPixmapInfo(Pixmap,
                    Width, Height, BitsPerPixel, Stride, Bits);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    fbDestroyPixmap(Pixmap);
}

/*******************************************************************************
** ClientBuffers. **************************************************************
*/

VDKAPI vdkClientBuffer VDKLANG
vdkCreateClientBuffer(
    int Width,
    int Height,
    int Format,
    int Type
    )
{
    return NULL;
}

VDKAPI int VDKLANG
vdkGetClientBufferInfo(
    vdkClientBuffer ClientBuffer,
    int * Width,
    int * Height,
    int * Stride,
    void ** Bits
    )
{
    return 0;
}

VDKAPI int VDKLANG
vdkDestroyClientBuffer(
    vdkClientBuffer ClientBuffer
    )
{
    return 0;
}

#endif

