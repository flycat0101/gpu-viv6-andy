/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
 * Graphics API specific functions.
 */

#ifndef __gc_egl_platform_h_
#define __gc_egl_platform_h_

#include "gc_egl_os.h"

#ifdef __cplusplus
extern "C" {
#endif
enum
{
    EGL_PLATFORM_FLAG_GBM_ASYNC     = 1 << 0,
};

struct eglPlatform
{
    /* Platform type, '0' for default (ie, by eglGetDisplay). */
    EGLenum platform;

    /* platform flags */
    gctUINT32   flags;
    /* Display. */
    /*
     * Get default display.
     * Used for EGL_DEFAULT_DISPLAY.
     * By design, this function should return the same native display.
     */
    void * (* getDefaultDisplay)(void);

    /*
     * Release default display.
     */
    void (* releaseDefaultDisplay)(void * Display);

    /*
     * Check for valid native display.
     */
    EGLBoolean (* isValidDisplay)(void * Display);

    EGLBoolean (* initLocalDisplayInfo)(VEGLDisplay Display);
    EGLBoolean (* deinitLocalDisplayInfo)(VEGLDisplay Display);

    /*
     * Convert EGLConfig to native visual id.
     */
    EGLint (* getNativeVisualId)(VEGLDisplay Display,
                                 struct eglConfig * Config);

    /*
     * Query of swap interval range.
     */
    EGLBoolean (* getSwapInterval)(VEGLDisplay Display,
                                   EGLint * Min,
                                   EGLint * Max);

    /*
     * Set swap interval.
     */
    EGLBoolean (* setSwapInterval)(VEGLSurface Surface,
                                   EGLint Interval);

    /* Window. */
    /*
     * Connect to native window.
     * Create private data for native window (wraps as a VEGLWindowInfo), and
     * other platform specific opeartions.
     * Automatically set VEGLWindowInfo to VEGLSurface::winInfo.
     *
     * Called once when eglCreateWindowSurface.
     */
    EGLBoolean (* connectWindow)(VEGLDisplay Display,
                                 VEGLSurface Surface,
                                 void * Win);

    /*
     * Disconnect from native window.
     * Free private data of native display and other platform specific operations.
     * Called once when destroying of window surface in eglDestroySurface.
     */
    EGLBoolean (* disconnectWindow)(VEGLDisplay Display,
                                    VEGLSurface Surface);

    /*
     * Bind native window for rendering.
     * Called when start rendering in eglMakeCurrent.
     *
     * Implementation (the integration layer) should decide the render mode for
     * given native window.
     *
     * Here're all conditions for native window rendering:
     *
     * 1. If can access window memory directly ('directAccess' = 'True').
     *
     *    If we can dynamically decide native window buffer types (such as ANDROID),
     *    this function should calculate the best way for window rendering.
     *
     *    1) Window back buffers are already HAL surface objects (such as ANDROID,
     *       DFB, etc).
     *       => Check if can use direct rendering
     *       => Return the HAL surface object in GetWindowBackBuffer
     *       => gcoOS_SetDisplayVirtualEx to PostWindowBackBuffer
     *
     *    2) Can be wrapped as HAL surface object (such as FBDEV, QNX, DFB, DDraw,
     *       wayland-compositor).
     *       => Check if can use direct rendering (tiled FB can still use direct
     *          rendering)
     *       => Wrap as HAL surface object in GetWindowBackBuffer
     *       => gcoOS_SetDisplayVirtual to PostWindowBackBuffer
     *
     *    3) Can not wrap as HAL surface object
     *       => Normally, can not use direct rendering in this case.
     *       => Allocate temporary surface object in GetWindowBackBuffer.
     *       => Copy to window back buffer memory and gcoOS_SetDisplayVirtual to
     *          PostWindowBackBuffer
     *
     * 2. Else can not access window memory directly ('directAccess' = 'False',
     *    such as GDI).
     *    => Normally, Can not use direct rendering.
     *    => Allocate temporary surface object in GetWindowBackBuffer.
     *    => gcoOS_DrawImage to PostWindowBackBuffer.
     *
     * Render mode may also depends on EGLConfig and preserved flag.
     */
    EGLBoolean (* bindWindow)(VEGLDisplay Display,
                              VEGLSurface Surface,
                              EGLint * RenderMode);

    /*
     * Unbind native window.
     * Called when ends rendering in eglMakeCurrent.
     */
    EGLBoolean (* unbindWindow)(VEGLDisplay Display,
                                VEGLSurface Surface);

    /*
     * Query window size.
     * This is required for EGL surface size and client viewport.
     */
    EGLBoolean (* getWindowSize)(VEGLDisplay Display,
                                 VEGLSurface Surface,
                                 EGLint * Width,
                                 EGLint * Height);

    /*
     * Get window back buffer.
     *
     * There're mainly 4 conditions mentioned in BindWindow above:
     *
     * If 'directAccess' window memory:
     *   1. Direct window back buffer surface.
     *   2. Wrapped surface.
     *   3. Temporary surface.
     * Else:
     *   4. Temporary surface.
     *
     * The window must be bound before call this function.
     */
    EGLBoolean (* getWindowBackBuffer)(VEGLDisplay Display,
                                       VEGLSurface Surface,
                                       struct eglBackBuffer * BackBuffer);

    /*
     * Post window back buffer to display.
     * Wait until the post opeation is done.
     *
     * The window must be bound before call this function.
     */
    EGLBoolean (* postWindowBackBuffer)(VEGLDisplay Display,
                                        VEGLSurface Surface,
                                        struct eglBackBuffer * BackBuffer,
                                        struct eglRegion * Region,
                                        struct eglRegion * DamageHint);

    /*
     * Android only.
     * Normal PostWindowBackBuffer plus native fence sync.
     */
    EGLBoolean (* postWindowBackBufferFence)(VEGLDisplay Display,
                                             VEGLSurface Surface,
                                             struct eglBackBuffer * BackBuffer,
                                             struct eglRegion * DamageHint);

    /*
     * Cancel window back buffer.
     * In direct rendering mode, the last window buffer (ie, render target) may be
     * not drawn, use this function to cancel it without post to display.
     *
     * The window must be bound before call this function.
     */
    EGLBoolean (* cancelWindowBackBuffer)(VEGLDisplay Display,
                                          VEGLSurface Surface,
                                          struct eglBackBuffer * BackBuffer);

    /*
     * Check if synchronous post is required for this window.
     *
     * Called every frame.
     */
    EGLBoolean (* synchronousPost)(VEGLDisplay Display,
                                   VEGLSurface Surface);

    /*
     * Check if the Handle belongs to specified native window.
     * Android only.
     */
    EGLBoolean (* hasWindowBuffer)(VEGLSurface Surface,
                                   EGLClientBuffer Handle);

    /*
     * Update buffer age.
     */
    EGLBoolean (* updateBufferAge)(VEGLDisplay Display,
                                   VEGLSurface Surface,
                                   struct eglBackBuffer * BackBuffer);

    /*
     * Query buffer age.
     */
    EGLBoolean (* queryBufferAge)(VEGLDisplay Display,
                                  VEGLSurface Surface,
                                  struct eglBackBuffer * BackBuffer,
                                  EGLint *BufferAge);

    /* Pixmap. */
    /*
     * Check if the EGLConfig matches native pixmap.
     * For EGL_MATCH_NATIVE_PIXMAP.
     */
    EGLBoolean (* matchPixmap)(VEGLDisplay Display,
                               void * Pixmap,
                               struct eglConfig * Config);

    /*
     * Connect to native pixmap.
     * Create private data for native pixmap (wraps as a VEGLPixmapInfo), and
     * other platform specific operations.
     * Different from native window, pixmap may be used in other purpose rather
     * than pixmap surface, so it will output the VEGLPixmapInfo structure here.
     *
     * Called once when accessing to native pixmap such as eglCreatePixmapSurface,
     * eglCreateImageKHR with EGL_NATIVE_PIXMAP_KHR as 'target', eglCopyBuffers,
     * etc.
     */
    EGLBoolean (* connectPixmap)(VEGLDisplay Display,
                                 void * Pixmap,
                                 VEGLPixmapInfo * Info,
                                 gcoSURF * Surface);

    /*
     * Disconnect from native pixmap.
     * Free private data of native pixmap and other platform specific operations.
     *
     * Called once when finished accessing to native pixmap.
     */
    EGLBoolean (* disconnectPixmap)(VEGLDisplay Display,
                                    void * Pixmap,
                                    VEGLPixmapInfo Info);

    /*
     * Query pixmap size.
     * This is required for EGL surface size and client viewport.
     */
    EGLBoolean (* getPixmapSize)(VEGLDisplay Display,
                                 void * Pixmap,
                                 VEGLPixmapInfo Info,
                                 EGLint * Width,
                                 EGLint * Height);

    /*
     * Update pixmap pixels.
     * Pixmap only has one buffer, use this function to sync pixels from native
     * pixmap to pixmap surface.
     * 1. For wrapped surface, cache operations might be required.
     * 2. For shadow surface, need copy pixels from native pixmap to surface.
     */
    EGLBoolean (* syncFromPixmap)(void * Pixmap,
                                  VEGLPixmapInfo PixInfo);

    /*
     * Wait native function to sync pixels to native pixmap.
     *
     * Called at synchronization points, such as eglWaitCliet, glFinish, etc.
     */
    EGLBoolean (* syncToPixmap)(void * Pixmap,
                                VEGLPixmapInfo PixInfo);

    /* DRI specific. */
    EGLBoolean (* createContext)(void * LocalDisplay,
                                 void * Context);

    void (* destroyContext)(void * Display,
                            void * Context);

    EGLBoolean (* makeCurrent)(void * LocalDisplay,
                               void * DrawDrawable,
                               void * ReadDrawable,
                               void * Context,
                               gcoSURF ResolveTarget);

    EGLBoolean (* swapBuffers)(void * LocalDisplay,
                               void * Drawable,
                               gcoSURF RenderTarget,
                               gcoSURF ResolveTarget,
                               void * ResolveBits,
                               gctUINT * Width,
                               gctUINT * Height);

    EGLBoolean (* rsForSwap)(void * localDisplay,
                             void * Drawable,
                             void * RsCallBack);
};

/* EGL_KHR_platform_android */
VEGLPlatform
veglGetAndroidPlatform(
    void * NativeDisplay
    );

/* EGL_KHR_platform_gbm */
VEGLPlatform
veglGetGbmPlatform(
    void * NativeDisplay
    );

/* EGL_KHR_platform_wayland */
VEGLPlatform
veglGetWaylandPlatform(
    void * NativeDisplay
    );

/*
 * EGL_KHR_platform_x11
 * selects either X11 or DRI implementation.
 */
VEGLPlatform
veglGetX11Platform(
    void * NativeDisplay
    );

/*
 * Valid on windows, selects one of win32, wince or wince ddraw.
 * Only support as default platform.
 */
VEGLPlatform
veglGetWin32Platform(
    void * NativeDisplay
    );

/* Null window system, only support as default platform. */
VEGLPlatform
veglGetNullwsPlatform(
    void * NativeDisplay
    );

/* Valid on linux, only support as default platform. */
VEGLPlatform
veglGetFbdevPlatform(
    void * NativeDisplay
    );

/* Valid on linux, only support as default platform. */
VEGLPlatform
veglGetDfbPlatform(
    void * NativeDisplay
    );

/* Valid on QNX, only support as default platform. */
VEGLPlatform
veglGetQnxPlatform(
    void * NativeDisplay
    );

#if defined(WL_EGL_PLATFORM)
struct wl_display;

EGLBoolean
veglBindWaylandDisplay(
    VEGLDisplay Display,
    struct wl_display * Dpy
    );

EGLBoolean
veglUnbindWaylandDisplay(
    VEGLDisplay Display,
    struct wl_display * Dpy
    );

EGLBoolean
veglQueryWaylandBuffer(
    VEGLDisplay Display,
    struct wl_resource *Buffer,
    EGLint * Width,
    EGLint * Height,
    EGLint * Fd,
    gcoSURF * Surface
    );

struct wl_buffer *
veglCreateWaylandBufferFromImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy,
    VEGLImage Image
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_platform_h_ */
