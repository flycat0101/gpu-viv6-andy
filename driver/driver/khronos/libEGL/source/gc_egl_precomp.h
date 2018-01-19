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


#ifndef __gc_egl_precomp_h_
#define __gc_egl_precomp_h_

#ifndef EGL_EGLEXT_PROTOTYPES
#  define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef EGLAPI
#if defined(_WIN32) && !defined(__SCITECH_SNAP__)
#    define EGLAPI    __declspec(dllexport)
#  else
#    define EGLAPI
#  endif
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gc_egl_os.h>
#include "gc_egl.h"
#include "gc_egl_platform.h"
#include "gc_hal_user_math.h"
#include "gc_hal_user_os_memory.h"
#include "gc_hal_statistics.h"


#endif /* __gc_egl_precomp_h_ */
