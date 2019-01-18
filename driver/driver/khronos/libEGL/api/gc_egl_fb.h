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


#ifndef __gc_backends_h_
#define __fb_backends_h_

#include "gc_egl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FBDisplay * PlatformDisplayType;
typedef struct _FBWindow *  PlatformWindowType;
typedef struct _FBPixmap *  PlatformPixmapType;

struct eglFbPlatform
{
    EGLenum platform;

    gceSTATUS (*GetDisplay)(
                            OUT PlatformDisplayType * Display,
                            IN gctPOINTER Context
                            );

    gceSTATUS (*GetDisplayByIndex)(
                                    IN gctINT DisplayIndex,
                                    OUT PlatformDisplayType * Display,
                                    IN gctPOINTER Context
                                    );

    gceSTATUS (*GetDisplayInfo)(
                                IN PlatformDisplayType Display,
                                OUT gctINT * Width,
                                OUT gctINT * Height,
                                OUT gctSIZE_T * Physical,
                                OUT gctINT * Stride,
                                OUT gctINT * BitsPerPixel
                                );

    gceSTATUS (*DestroyDisplay)(
                                IN PlatformDisplayType Display
                                );

    gceSTATUS (*CreateWindow)(
                                IN PlatformDisplayType Display,
                                IN gctINT X,
                                IN gctINT Y,
                                IN gctINT Width,
                                IN gctINT Height,
                                PlatformWindowType * Window
                                );

    gceSTATUS (*GetWindowInfo)(
                                IN PlatformDisplayType Display,
                                IN PlatformWindowType Window,
                                OUT gctINT * X,
                                OUT gctINT * Y,
                                OUT gctINT * Width,
                                OUT gctINT * Height,
                                OUT gctINT * BitsPerPixel,
                                OUT gctUINT * Offset
                                );

    gceSTATUS (*DestroyWindow)(
                            IN PlatformDisplayType Display,
                            IN PlatformWindowType Window
                            );

    gceSTATUS (*CreatePixmap)(
                            IN PlatformDisplayType Display,
                            IN gctINT Width,
                            IN gctINT Height,
                            IN gctINT BitsPerPixel,
                            OUT PlatformPixmapType * Pixmap
                            );

    gceSTATUS (*GetPixmapInfo)(
                        IN PlatformDisplayType Display,
                        IN PlatformPixmapType Pixmap,
                        OUT gctINT * Width,
                        OUT gctINT * Height,
                        OUT gctINT * BitsPerPixel,
                        OUT gctINT * Stride,
                        OUT gctPOINTER * Bits
                        );

    gceSTATUS (*DestroyPixmap)(
                        IN PlatformDisplayType Display,
                        IN PlatformPixmapType Pixmap
                        );

    VEGLPlatform (*GetFbdevPlatform) (
                        void * NativeDisplay
                        );

};


#ifdef __cplusplus
}
#endif

#endif
