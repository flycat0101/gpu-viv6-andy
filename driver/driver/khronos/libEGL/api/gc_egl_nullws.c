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


#include "gc_egl_platform.h"

static void *
_GetDefaultDisplay(
    void
    )
{
    return (void *) 1;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    return 0;
}

static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Max,
    OUT EGLint * Min
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    )
{
    return EGL_TRUE;
}

static struct eglPlatform nullwsPlatform = {
    0,
    _GetDefaultDisplay,
    _ReleaseDefaultDisplay,
    _IsValidDisplay,
    _InitLocalDisplayInfo,
    _DeinitLocalDisplayInfo,
    _GetNativeVisualId,
    _GetSwapInterval,
    _SetSwapInterval,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
    gcvNULL,
};

VEGLPlatform
veglGetNullwsPlatform(
    void * NativeDisplay
    )
{
    return &nullwsPlatform;
}

