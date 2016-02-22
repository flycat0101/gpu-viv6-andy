/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

#ifndef __gc_egl_graphics_h_
#define __gc_egl_graphics_h_

#include "gc_egl_os.h"
#include "gc_hal_eglplatform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Display. */
/*
 * Get default display.
 * Used for EGL_DEFAULT_DISPLAY.
 * By design, this function should return the same native display.
 */
NativeDisplayType
veglGetDefaultDisplay(
    void
    );

/*
 * Release default display.
 */
void
veglReleaseDefaultDisplay(
    IN NativeDisplayType Display
    );

/*
 * Check for valid native display.
 */
EGLBoolean
veglIsValidDisplay(
    IN NativeDisplayType Display
    );

EGLBoolean
veglInitLocalDisplayInfo(
    IN VEGLDisplay Display
    );

EGLBoolean
veglDeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    );

/*
 * Convert EGLConfig to native visual id.
 */
EGLint
veglGetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    );

/*
 * Query of swap interval range.
 */
EGLBoolean
veglGetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Min,
    OUT EGLint * Max
    );

/*
 * Set swap interval.
 */
EGLBoolean
veglSetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    );

/* Window. */
/*
 * Connect to native window.
 * Create private data for native window (wraps as a VEGLWindowInfo), and
 * other platform specific opeartions.
 * Automatically set VEGLWindowInfo to VEGLSurface::winInfo.
 *
 * Called once when eglCreateWindowSurface.
 */
EGLBoolean
veglConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN NativeWindowType Win
    );

/*
 * Disconnect from native window.
 * Free private data of native display and other platform specific operations.
 * Called once when destroying of window surface in eglDestroySurface.
 */
EGLBoolean
veglDisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    );

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
EGLBoolean
veglBindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    );

/*
 * Unbind native window.
 * Called when ends rendering in eglMakeCurrent.
 */
EGLBoolean
veglUnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    );

/*
 * Query window size.
 * This is required for EGL surface size and client viewport.
 */
EGLBoolean
veglGetWindowSize(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * Width,
    OUT EGLint * Height
    );

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
EGLBoolean
veglGetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    );

/*
 * Post window back buffer to display.
 * Wait until the post opeation is done.
 *
 * The window must be bound before call this function.
 */
EGLBoolean
veglPostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN EGLint NumRects,
    IN EGLint Rects[]
    );

/*
 * Android only.
 * Normal PostWindowBackBuffer plus native fence sync.
 */
EGLBoolean
veglPostWindowBackBufferFence(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    );

/*
 * Cancel window back buffer.
 * In direct rendering mode, the last window buffer (ie, render target) may be
 * not drawn, use this function to cancel it without post to display.
 *
 * The window must be bound before call this function.
 */
EGLBoolean
veglCancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    );

/*
 * Check if synchronous post is required for this window.
 *
 * Called every frame.
 */
EGLBoolean
veglSynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    );

/*
 * Check if the Handle belongs to specified native window.
 * Android only.
 */
EGLBoolean
veglHasWindowBuffer(
    IN VEGLSurface Surface,
    IN EGLClientBuffer Handle
    );

/* Pixmap. */
/*
 * Check if the EGLConfig matches native pixmap.
 * For EGL_MATCH_NATIVE_PIXMAP.
 */
EGLBoolean
veglMatchPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN struct eglConfig * Config
    );

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
EGLBoolean
veglConnectPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    );

/*
 * Disconnect from native pixmap.
 * Free private data of native pixmap and other platform specific operations.
 *
 * Called once when finished accessing to native pixmap.
 */
EGLBoolean
veglDisconnectPixmap(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info
    );

/*
 * Query pixmap size.
 * This is required for EGL surface size and client viewport.
 */
EGLBoolean
veglGetPixmapSize(
    IN VEGLDisplay Display,
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo Info,
    OUT EGLint * Width,
    OUT EGLint * Height
    );

/*
 * Update pixmap pixels.
 * Pixmap only has one buffer, use this function to sync pixels from native
 * pixmap to pixmap surface.
 * 1. For wrapped surface, cache operations might be required.
 * 2. For shadow surface, need copy pixels from native pixmap to surface.
 */
EGLBoolean
veglSyncFromPixmap(
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo PixInfo
    );

/*
 * Wait native function to sync pixels to native pixmap.
 *
 * Called at synchronization points, such as eglWaitCliet, glFinish, etc.
 */
EGLBoolean
veglSyncToPixmap(
    IN NativePixmapType Pixmap,
    IN VEGLPixmapInfo PixInfo
    );

/*
 * Update buffer age.
 */
EGLBoolean
veglUpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    );

/*
 * Query buffer age.
 */
EGLBoolean
veglQueryBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    OUT EGLint *BufferAge
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_graphics_h_ */
