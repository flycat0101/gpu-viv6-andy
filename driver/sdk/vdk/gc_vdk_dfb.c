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


#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dlfcn.h>

#include <sys/time.h>


/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$DFB\n";

struct _vdkPrivate
{
    vdkDisplay  display;
    void *      egl;
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

    vdk = (vdkPrivate) malloc(sizeof(struct _vdkPrivate));

    if (vdk == NULL)
    {
        return NULL;
    }

#if gcdSTATIC_LINK
    vdk->egl = NULL;
#else
    vdk->egl = dlopen("libEGL.so", RTLD_LAZY);
#endif

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

        if (Private->egl != NULL)
        {
            dlclose(Private->egl);
            Private->egl = NULL;
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

    if (Private->display != (EGLNativeDisplayType) NULL)
    {
        return Private->display;
    }

    Private->display = dfbGetDisplay(NULL);
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

    fprintf(stderr, "%s not implemented\n", __func__);
    return 0;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    dfbDestroyDisplay(Display);

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
    return dfbCreateWindow(Display, X, Y, Width, Height);
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
    return dfbGetWindowInfo(Window, X, Y, Width, Height, BitsPerPixel, Offset);
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    dfbDestroyWindow(Window);
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    dfbShowWindow(_vdk->display, Window);
    return 1;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    dfbHideWindow(_vdk->display, Window);
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
    platEvent event;
    int result = 0;

    if (_vdk == NULL)
    {
        return result;
    }

    if (dfbGetEvent(_vdk->display, Window, &event))
    {
        result = 1;
        switch (event.type)
        {
        case EVENT_KEYBOARD:
            Event->type = VDK_KEYBOARD;
            Event->data.keyboard.scancode = (vdkKeys)event.data.keyboard.scancode;
            Event->data.keyboard.pressed  = event.data.keyboard.pressed;
            Event->data.keyboard.key      = event.data.keyboard.key;
            break;
        case EVENT_BUTTON:
            Event->type = VDK_BUTTON;
            Event->data.button.left     = event.data.button.left;
            Event->data.button.right    = event.data.button.right;
            Event->data.button.middle   = event.data.button.middle;
            Event->data.button.x        = event.data.button.x;
            Event->data.button.y        = event.data.button.y;
            break;
        case EVENT_POINTER:
            Event->type = VDK_POINTER;
            Event->data.pointer.x = event.data.pointer.x;
            Event->data.pointer.y = event.data.pointer.y;
            break;
        case EVENT_CLOSE:
            Event->type = VDK_CLOSE;
            break;
        case EVENT_WINDOW_UPDATE:
            Event->type = VDK_WINDOW_UPDATE;
            break;
        default:
            return 0;
        }
    }
    return result;
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
#if gcdSTATIC_LINK
    return (EGL_ADDRESS) eglGetProcAddress(Function);
#else
    return (EGL_ADDRESS) dlsym(Private->egl, Function);
 #endif
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
    struct timeval tv;

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
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
    return dfbCreatePixmapWithBpp(Display,
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
    dfbGetPixmapInfo(Pixmap,
                     Width, Height, BitsPerPixel, Stride, Bits);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    dfbDestroyPixmap(Pixmap);
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
