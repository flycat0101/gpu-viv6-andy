/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
 * 'Display module' includes multiple parts,
 * 1. display discovery and configuration (gc_hwc_display.cpp)
 * 2. overlay control (gc_hwc_overlay.cpp)
 * 3. vsync control (gc_hwc_vsync.cpp)
 * 4. hardware cursor (gc_hwc_cursor.cpp)
 *
 * TODO (Soc-vendor): Implement all of this.
 */
#include "gc_hwc.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>


/*
 * Get/post render buffer for physical display.
 * 2D blitters opeartions in Vivante hwcomposer replace 3D functionanities. 3D
 * composotion is done on framebuffer target but 2D does not have such context.
 * These functions are to get/post correct framebuffer target.
 *
 * Why not use HWC_FRAMEBUFFER_TARGET?
 * Google's design for HWC composition is as follows:
 *  * 3D compose layers and writes to back buffer. FRAMEBUFFER_TARGET becomes
 *  front and is switched when eglSwapBuffers.
 *  * HWC can use OVERLAY(DC) to mix FRAMEBUFFER_TARGET and OVERLAY layers
 *    together and display the result.
 *  * If only OVERLAY layers change, FRAMEBUFFER_TARGET is not changed since
 *  3D render is not needed.
 *
 * Vivante HWC can use 2D blitters to replace 3D composition:
 *  * 2D compose layers and writes to back buffer obtained by
 *  eglGetRenderBufferVIV. FRAMEBUFFER_TARGET becomes front and is switched
 *  when eglPostBufferVIV.
 *
 * So HWC_FRAMEBUFFER_TARGET in layer list is *front buffer* for DC composition.
 * 2D blitters need get back buffer for write and post after. That's why the
 * following two functions are needed.
 *
 * TODO (Soc-vendor): Implement the functions if non-Vivante EGL.
 */
typedef EGLClientBuffer (EGLAPIENTRYP PFNEGLGETRENDERBUFFERVIVPROC) (EGLClientBuffer Handle);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLPOSTBUFFERVIVPROC) (EGLClientBuffer Buffer);

extern PFNEGLGETRENDERBUFFERVIVPROC  _eglGetRenderBufferVIV;
extern PFNEGLPOSTBUFFERVIVPROC _eglPostBufferVIV;

/*
 * 'Display module' includes multiple parts,
 * 1. display discovery and configuration (gc_hwc_display.cpp)
 * 2. overlay control (gc_hwc_overlay.cpp)
 * 3. vsync control (gc_hwc_vsync.cpp)
 * 4. hardware cursor (no prototypes currently)
 *
 * See gc_hwc_display.h
 */
/*
 * First time init displays.
 * Call when module opened.
 */
int
hwcInitDisplays(
    hwcContext * Context
    );

/*
 * Finish displays.
 * Call when module closing.
 */
int
hwcFinishDisplays(
    hwcContext * Context
    );

/*
 * Detect virtual displays.
 */
int
hwcDetectVirtualDisplays(
    hwcContext * Context,
    size_t NumDisplays
    );

/*
 * This function is called every frame.
 * TODO (Soc-vendor): should complete works in it:
 * 1. display discovery (hotplug)
 * 2. overlay control
 * 3. hardware cursor control
 * etc.
 */
int
hwcDisplayFrame(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    );

/* vsync. */
int
hwcInitVsync(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcFinishVsync(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcVsyncControl(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN int Enable
    );

/* overlay. */
int
hwcInitOverlay(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcFinishOverlay(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcOverlayFrame(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    );

/* cursor. */
int
hwcInitCursor(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcFinishCursor(
    IN hwcContext * Context,
    IN hwcDisplay * Display
    );

int
hwcCursorFrame(
    IN hwcContext * Context,
    IN hwcDisplay * Display,
    IN hwc_display_contents_1_t * HwDisplay
    );

/* etc, ... */

