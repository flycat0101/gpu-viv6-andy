/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/



#include "gc_egl_fb.h"

static struct eglFbPlatform* fbBackend = gcvNULL;
extern struct eglFbPlatform* getFbDevBackend();
#if gcdUSE_KMS
extern struct eglFbPlatform* getFbDrmBackend();
#endif

static void
fbGetBackends(
    OUT struct eglFbPlatform ** fbBackend
    )
{
#if gcdUSE_KMS
    if (*fbBackend == gcvNULL)
    {
        *fbBackend = getFbDrmBackend();
    }
#endif
    if (*fbBackend == gcvNULL)
    {
        *fbBackend = getFbDevBackend();
    }
}

void *
fbGetDisplay(
    gctPOINTER context
    )
{
    PlatformDisplayType display = gcvNULL;
    fbGetBackends(&fbBackend);
    fbBackend->GetDisplay(&display, context);
    return display;
}

void *
fbGetDisplayByIndex(
    IN gctINT DisplayIndex
    )
{
    PlatformDisplayType display = gcvNULL;
    fbGetBackends(&fbBackend);
    fbBackend->GetDisplayByIndex(DisplayIndex, &display, gcvNULL);
    return display;
}

void
fbGetDisplayGeometry(
    IN void * Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetDisplayInfo((gctPOINTER)Display, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetDisplayInfo(
    IN void * Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT unsigned long *Physical,
    OUT gctINT_PTR Stride,
    OUT gctINT_PTR BitsPerPixel
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetDisplayInfo((gctPOINTER)Display, Width, Height,(gctSIZE_T_PTR)Physical, Stride, BitsPerPixel);
}

void
fbDestroyDisplay(
    IN void * Display
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->DestroyDisplay((gctPOINTER)Display);
}

void *
fbCreateWindow(
    IN void * Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformWindowType window = gcvNULL;
    fbGetBackends(&fbBackend);
    fbBackend->CreateWindow((gctPOINTER)Display, X, Y, Width, Height, &window);
    return window;
}

void
fbGetWindowGeometry(
    IN void * Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetWindowInfo(gcvNULL, (gctPOINTER)Window, X, Y, Width, Height, gcvNULL, gcvNULL);
}

void
fbGetWindowInfo(
    IN void * Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctUINT_PTR Offset
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetWindowInfo(gcvNULL, (gctPOINTER)Window, X, Y, Width, Height, BitsPerPixel, Offset);
}

void
fbDestroyWindow(
    IN void * Window
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->DestroyWindow(gcvNULL, (gctPOINTER)Window);
}

void *
fbCreatePixmap(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    fbGetBackends(&fbBackend);
    fbBackend->CreatePixmap((gctPOINTER)Display, Width, Height, 32, &Pixmap);
    return Pixmap;
}

void *
fbCreatePixmapWithBpp(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    fbGetBackends(&fbBackend);
    fbBackend->CreatePixmap((gctPOINTER)Display, Width, Height, BitsPerPixel, &Pixmap);
    return Pixmap;
}

void
fbGetPixmapGeometry(
    IN void * Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetPixmapInfo(gcvNULL, (gctPOINTER)Pixmap, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetPixmapInfo(
    IN void * Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctINT_PTR Stride,
    OUT gctPOINTER * Bits
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->GetPixmapInfo(gcvNULL, (gctPOINTER)Pixmap, Width, Height, BitsPerPixel, Stride, Bits);
}

void
fbDestroyPixmap(
    IN void * Pixmap
    )
{
    fbGetBackends(&fbBackend);
    fbBackend->DestroyPixmap(gcvNULL, (gctPOINTER)Pixmap);
}

VEGLPlatform
veglGetFbdevPlatform(
    void * NativeDisplay
    )
{
    fbGetBackends(&fbBackend);
    return fbBackend->GetFbdevPlatform(NativeDisplay);
}
