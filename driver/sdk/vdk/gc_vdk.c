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


#include <gc_vdk_hal.h>
#include <stdlib.h>

/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$GAL\n";

struct _vdkPrivate
{
    vdkDisplay  display;
    void *      egl;
};


/*******************************************************************************
** Private functions.
*/
static vdkPrivate _vdk = galNULL;
int HAL_Constructor();
void HAL_Destructor();
extern GAL_API * GAL;

/*******************************************************************************
** Initialization.
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    vdkPrivate  vdk = galNULL;

    if(HAL_Constructor())
        return vdk;

    vdk = (vdkPrivate) malloc(sizeof(struct _vdkPrivate));
    if (vdk == NULL)
    {
        return galNULL;
    }

#if gcdSTATIC_LINK
    vdk->egl = galNULL;
#else
    if(GAL->GAL_LoadEGLLibrary(&vdk->egl))
        {
            free(vdk);
            return galNULL;
        }
#endif

    vdk->display = (EGLNativeDisplayType) galNULL;

    _vdk = vdk;
    return vdk;


}

VDKAPI void VDKLANG
vdkExit(
    vdkPrivate Private
    )
{
    if (Private != galNULL)
    {
        if (_vdk == Private)
        {
            _vdk = galNULL;
        }

        if (Private->egl != galNULL)
        {
            GAL->GAL_FreeEGLLibrary(Private->egl);
        }

        free(Private);
    }
    HAL_Destructor();
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
    vdkDisplay  display = (EGLNativeDisplayType) galNULL;

    if ((Private != galNULL) && (Private->display != (EGLNativeDisplayType) galNULL))
    {
        return Private->display;
    }

    if (IS_SUCCESS(GAL->GAL_GetDisplayByIndex(DisplayIndex, &display, galNULL)))
    {
     if(Private != galNULL)
     {
        Private->display = display;
     }
    }

    return display;
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
    return IS_SUCCESS(GAL->GAL_GetDisplayInfo(Display,
                                              Width, Height,
                                              Physical,
                                              Stride,
                                              BitsPerPixel)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkGetDisplayInfoEx(
    vdkDisplay Display,
    unsigned int DisplayInfoSize,
    vdkDISPLAY_INFO * DisplayInfo
    )
{
    return IS_SUCCESS(GAL->GAL_GetDisplayInfoEx(Display,
                                                0,
                                                DisplayInfoSize,
                                                DisplayInfo)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkGetDisplayVirtual(
    vdkDisplay Display,
    int * Width,
    int * Height
    )
{
    return IS_SUCCESS(GAL->GAL_GetDisplayVirtual(Display,
                                                 Width, Height)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkGetDisplayBackbuffer(
    vdkDisplay Display,
    unsigned int * Offset,
    int * X,
    int * Y
    )
{
    void *  context;
    SURFACE     surface;

    return IS_SUCCESS(GAL->GAL_GetDisplayBackbuffer(Display,
                                                    0, &context, &surface,
                                                    Offset, X, Y)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkSetDisplayVirtual(
    vdkDisplay Display,
    unsigned int Offset,
    int X,
    int Y
    )
{
    return IS_SUCCESS(GAL->GAL_SetDisplayVirtual(Display,
                                                 0,
                                                 Offset, X, Y)) ? 1 : 0;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    GAL->GAL_DestroyDisplay(Display);
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
    vdkWindow   window;

    return IS_SUCCESS(GAL->GAL_CreateWindow(Display,
                                            X, Y,
                                            Width, Height,
                                            (EGLNativeWindowType *) &window)) ? window : 0;
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
    SURF_FORMAT  format;
    SURF_TYPE type;

    if (_vdk == galNULL)
    {
        return 0;
    }

    return IS_SUCCESS(GAL->GAL_GetWindowInfoEx(_vdk->display,
                                               Window,
                                               X, Y,
                                               Width, Height,
                                               BitsPerPixel,
                                               Offset,
                                               &format,
                                               &type)) ? 1 : 0;
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    if (_vdk != galNULL)
    {
        GAL->GAL_DestroyWindow(_vdk->display, Window);
    }
}

/*
    vdkDrawImage

    Draw a rectangle from a bitmap to the window client area.

    PARAMETERS:

        vdkWindow Window
            Pointer to the window data structure returned by vdkCreateWindow.

        int Left
            Left coordinate of rectangle.

        int Top
            Top coordinate of rectangle.

        int Right
            Right coordinate of rectangle.

        int Bottom
            Bottom coordinate of rectangle.

        int Width
            Width of the specified bitmap.

        int Height
            Height of the specified bitmap.  If height is negative, the bitmap
            is bottom-to-top.

        int BitsPerPixel
            Color depth of the bitmap.

        void * Bits
            Pointer to the bits of the bitmap.

    RETURNS:

        1 if the rectangle has been copied to the window, or 0 if there is an
        error.

*/
VDKAPI int VDKLANG
vdkDrawImage(
    vdkWindow Window,
    int Left,
    int Top,
    int Right,
    int Bottom,
    int Width,
    int Height,
    int BitsPerPixel,
    void * Bits
    )
{
    if (_vdk == galNULL)
    {
        return 0;
    }

    return IS_SUCCESS(GAL->GAL_DrawImage(_vdk->display,
                                         Window,
                                         Left, Top, Right, Bottom,
                                         Width, Height,
                                         BitsPerPixel,
                                         Bits)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    if (_vdk == galNULL)
    {
        return 0;
    }

    return IS_SUCCESS(GAL->GAL_ShowWindow(_vdk->display, Window)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    if (_vdk == galNULL)
    {
        return 0;
    }

    return IS_SUCCESS(GAL->GAL_HideWindow(_vdk->display, Window)) ? 1 : 0;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
    if (_vdk != galNULL)
    {
        GAL->GAL_SetWindowTitle(_vdk->display, Window, Title);
    }
}

VDKAPI void VDKLANG
vdkCapturePointer(
    vdkWindow Window
    )
{
    if (_vdk != galNULL)
    {
        GAL->GAL_CapturePointer(_vdk->display, Window);
    }
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
    halEvent halEvent;
    int result = 0;
    if (_vdk == galNULL)
    {
        return result;
    }

    if(IS_SUCCESS(GAL->GAL_GetEvent(_vdk->display, Window, &halEvent)))
    {
        result = 1;
        switch(halEvent.type)
        {
        case HAL_KEYBOARD:
            Event->type = VDK_KEYBOARD;
            Event->data.keyboard.scancode = (vdkKeys)halEvent.data.keyboard.scancode;
            Event->data.keyboard.pressed  = halEvent.data.keyboard.pressed;
            Event->data.keyboard.key      = halEvent.data.keyboard.key;
            break;
        case HAL_BUTTON:
            Event->type = VDK_BUTTON;
            Event->data.button.left     = halEvent.data.button.left;
            Event->data.button.right    = halEvent.data.button.right;
            Event->data.button.middle   = halEvent.data.button.middle;
            Event->data.button.x        = halEvent.data.button.x;
            Event->data.button.y        = halEvent.data.button.y;
            break;
        case HAL_POINTER:
            Event->type = VDK_POINTER;
            Event->data.pointer.x = halEvent.data.pointer.x;
            Event->data.pointer.y = halEvent.data.pointer.y;
            break;
        case HAL_CLOSE:
            Event->type = VDK_CLOSE;
            break;
        case HAL_WINDOW_UPDATE:
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
    union gcsVARIANT
    {
        void *      ptr;
        EGL_ADDRESS func;
    }
    address;

    if ((Private != galNULL)
        &&
        IS_SUCCESS(GAL->GAL_GetProcAddress(galNULL,
                                           Private->egl,
                                           Function,
                                           &address.ptr))
        )
    {
        return address.func;
    }

    return galNULL;
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
    return GAL->GAL_GetTicks();
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
    vdkPixmap   pixmap;

    if (IS_SUCCESS(GAL->GAL_CreatePixmap(Display,
                                         Width, Height,
                                         BitsPerPixel,
                                         (EGLNativePixmapType *) &pixmap)))
    {
        return pixmap;
    }

    return 0;
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
    int bpp, stride;

    if (_vdk == galNULL)
    {
        return 0;
    }

    if (IS_SUCCESS(GAL->GAL_GetPixmapInfo(_vdk->display,
                                          Pixmap,
                                          Width, Height,
                                          &bpp,
                                          &stride,
                                          Bits)))
    {
        if (BitsPerPixel != NULL)
            *BitsPerPixel = bpp;
        if (Stride != NULL)
            *Stride = stride;

        return 1;
    }
    else
    {
        return 0;
    }
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    if (_vdk != galNULL)
    {
        GAL->GAL_DestroyPixmap(_vdk->display, Pixmap);
    }
}

/*
    vdkDrawPixmap

    Draw a rectangle from a bitmap to the pixmap client area.

    PARAMETERS:

        vdkPixmap Pixmap
            Pointer to the pixmap data structure returned by vdkCreatePixmap.

        int Left
            Left coordinate of rectangle.

        int Top
            Top coordinate of rectangle.

        int Right
            Right coordinate of rectangle.

        int Bottom
            Bottom coordinate of rectangle.

        int Width
            Width of the specified bitmap.

        int Height
            Height of the specified bitmap.  If height is negative, the bitmap
            is bottom-to-top.

        int BitsPerPixel
            Color depth of the bitmap.

        void * Bits
            Pointer to the bits of the bitmap.

    RETURNS:

        1 if the rectangle has been copied to the pixmap, or 0 if there is an
        error.
*/
VDKAPI int VDKLANG
vdkDrawPixmap(
    vdkPixmap Pixmap,
    int Left,
    int Top,
    int Right,
    int Bottom,
    int Width,
    int Height,
    int BitsPerPixel,
    void * Bits
    )
{
    if (_vdk == galNULL)
    {
        return 0;
    }

    return IS_SUCCESS(GAL->GAL_DrawPixmap(_vdk->display,
                                          Pixmap,
                                          Left, Top, Right, Bottom,
                                          Width, Height,
                                          BitsPerPixel,
                                          Bits)) ? 1 : 0;
}

/*******************************************************************************
** ClientBuffers. **************************************************************
*/

/* GL_VIV_direct_texture */
#ifndef GL_VIV_direct_texture
#define GL_VIV_YV12                        0x8FC0
#define GL_VIV_NV12                        0x8FC1
#define GL_VIV_YUY2                        0x8FC2
#define GL_VIV_UYVY                        0x8FC3
#define GL_VIV_NV21                        0x8FC4
#endif

VDKAPI vdkClientBuffer VDKLANG
vdkCreateClientBuffer(
    int Width,
    int Height,
    int Format,
    int Type
    )
{
    vdkClientBuffer clientBuffer;

    if (IS_SUCCESS(GAL->GAL_CreateClientBuffer(Width, Height,
                                               Format, Type,
                                               (void * *) &clientBuffer)))
    {
        return clientBuffer;
    }

    return galNULL;
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
    return IS_SUCCESS(GAL->GAL_GetClientBufferInfo(ClientBuffer,
                                                   Width, Height,
                                                   Stride, Bits)) ? 1 : 0;
}

VDKAPI int VDKLANG
vdkDestroyClientBuffer(
    vdkClientBuffer ClientBuffer
    )
{
    return IS_SUCCESS(GAL->GAL_DestroyClientBuffer(ClientBuffer)) ? 1 : 0;
}
