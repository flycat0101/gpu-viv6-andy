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


#ifndef __gc_glff_precomp_h_
#define __gc_glff_precomp_h_

#include "gc_hal_types.h"
#include "gc_hal_base.h"
#include "gc_hal_eglplatform_type.h"
#include "gc_hal_user_math.h"
#include "gc_hal_options.h"

/*******************************************************************************
** Include standard libraries.
*/
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#if !defined(GL_API)
#if defined(_WIN32)
# define GL_API __declspec(dllexport)
#else
# define GL_API
#endif
#endif

#if defined(gcdES11_CORE_WITH_EGL) && !defined(EGLAPI)
#if defined(_WIN32) && !defined(__SCITECH_SNAP__)
#    define EGLAPI    __declspec(dllexport)
#  else
#    define EGLAPI
#  endif
#endif

/* Use renaming trick to add the dispatching to all APIs. */
#if gcdRENDER_THREADS
# define _GL_11_APPENDIX _THREAD
#endif

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(_X11_XLIB_H_)
/* Rename some badly named X defines. */
#ifdef Status
#   define XStatus      int
#   undef Status
#endif
#ifdef Always
#   define XAlways      2
#   undef Always
#endif

#undef CurrentTime
#endif

#include "gc_egl_common.h"

/*******************************************************************************
** Declare the context pointer.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _glsCONTEXT * glsCONTEXT_PTR;

#ifdef __cplusplus
}
#endif

/*******************************************************************************
** Include the rest of the driver declarations.
*/

#include "gc_hal.h"
#include "gc_hal_raster.h"
#include "gc_hal_engine.h"
#include "gc_vsc_drvi_interface.h"
#include "gc_hal_driver.h"
#include "gc_hal_user.h"
#include "gc_hal_user_os_memory.h"
#include "gc_glff_basic_defines.h"
#include "gc_glff_basic_types.h"
#include "gc_glff_named_object.h"
#include "gc_glff_buffer.h"
#include "gc_glff_renderbuffer.h"
#include "gc_glff_framebuffer.h"
#include "gc_glff_fixed_func.h"
#include "gc_glff_frag_proc.h"
#include "gc_glff_matrix.h"
#include "gc_glff_texture.h"
#include "gc_glff_stream.h"
#include "gc_glff_hash.h"
#include "gc_glff_profiler.h"
#include "gc_glff_draw.h"
#include "gc_glff.h"
#include "gc_glff_point.h"
#include "gc_glff_line.h"
#include "gc_glff_pixel.h"
#include "gc_glff_alpha.h"
#include "gc_glff_fog.h"
#include "gc_glff_lighting.h"
#include "gc_glff_cull.h"
#include "gc_glff_depth.h"
#include "gc_glff_viewport.h"
#include "gc_glff_multisample.h"
#include "gc_glff_states.h"
#include "gc_glff_clip_plane.h"

void glfRenderThread(
    IN gcsRENDER_THREAD* ThreadInfo
    );

#endif /* __gc_glff_precomp_h_ */
