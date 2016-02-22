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


/*
 * Vivante specific definitions and declarations for EGL library.
 */

#ifndef __eglvivante_h_
#define __eglvivante_h_

#ifdef __cplusplus
extern "C" {
#endif


#if defined(LINUX) && defined(EGL_API_FB) && !defined(__APPLE__)

EGLNativeDisplayType
fbGetDisplay(
    void *context
    );

EGLNativeDisplayType
fbGetDisplayByIndex(
    int DisplayIndex
    );

void
fbGetDisplayGeometry(
    EGLNativeDisplayType Display,
    int * Width,
    int * Height
    );

void
fbGetDisplayInfo(
    EGLNativeDisplayType Display,
    int * Width,
    int * Height,
    unsigned long * Physical,
    int * Stride,
    int * BitsPerPixel
    );

void
fbDestroyDisplay(
    EGLNativeDisplayType Display
    );

EGLNativeWindowType
fbCreateWindow(
    EGLNativeDisplayType Display,
    int X,
    int Y,
    int Width,
    int Height
    );

void
fbGetWindowGeometry(
    EGLNativeWindowType Window,
    int * X,
    int * Y,
    int * Width,
    int * Height
    );

void
fbGetWindowInfo(
    EGLNativeWindowType Window,
    int * X,
    int * Y,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    unsigned int * Offset
    );

void
fbDestroyWindow(
    EGLNativeWindowType Window
    );

EGLNativePixmapType
fbCreatePixmap(
    EGLNativeDisplayType Display,
    int Width,
    int Height
    );

EGLNativePixmapType
fbCreatePixmapWithBpp(
    EGLNativeDisplayType Display,
    int Width,
    int Height,
    int BitsPerPixel
    );

void
fbGetPixmapGeometry(
    EGLNativePixmapType Pixmap,
    int * Width,
    int * Height
    );

void
fbGetPixmapInfo(
    EGLNativePixmapType Pixmap,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    int * Stride,
    void ** Bits
    );

void
fbDestroyPixmap(
    EGLNativePixmapType Pixmap
    );

#endif

#if defined(LINUX) && defined(EGL_API_DFB) && !defined(__APPLE__)

EGLNativeDisplayType
dfbGetDisplay(
    void *context
    );

void
dfbDestroyDisplay(
    EGLNativeDisplayType Display
    );

EGLNativeWindowType
dfbCreateWindow(
    EGLNativeDisplayType Display,
    int X,
    int Y,
    int Width,
    int Height
    );

void
dfbDestroyWindow(
    EGLNativeWindowType Window
    );

EGLNativePixmapType
dfbCreatePixmap(
    EGLNativeDisplayType Display,
    int Width,
    int Height
    );

EGLNativePixmapType
dfbCreatePixmapWithBpp(
    EGLNativeDisplayType Display,
    int Width,
    int Height,
    int BitsPerPixel
    );

void
dfbGetPixmapInfo(
    EGLNativePixmapType Pixmap,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    int * Stride,
    void* * Bits
    );

void
dfbDestroyPixmap(
    EGLNativePixmapType Pixmap
    );

#endif


#ifdef __cplusplus
}
#endif

#endif /* __eglvivante_h_ */
