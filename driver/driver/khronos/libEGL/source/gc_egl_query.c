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


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include "gc_egl_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_API

#if defined(WL_EGL_PLATFORM)
#  include <wayland-server.h>
#endif

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapInterval(
    EGLDisplay Dpy,
    EGLint Interval
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface surface;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Interval=%d", Dpy, Interval);
    gcmDUMP_API("${EGL eglSwapInterval 0x%08X 0x%08X}", Dpy, Interval);
    VEGL_TRACE_API(SwapInterval)(Dpy, Interval);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);

    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (dpy->hdc == (void *) gcvNULL)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (thread->context == EGL_NO_CONTEXT)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    surface = thread->context->draw;
    if (surface == EGL_NO_SURFACE)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (!dpy->platform->setSwapInterval(surface, Interval))
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLContext EGLAPIENTRY
eglGetCurrentContext(
    void
    )
{
    VEGLThreadData thread;
    VEGLContext context = gcvNULL;

    gcmHEADER();

    VEGL_TRACE_API_PRE(GetCurrentContext)();

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        /* Fatal error. */
        gcmFOOTER_ARG("return=0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    if (thread->api == EGL_NONE)
    {
        /* Fatal error. */
        gcmFOOTER_ARG("return=0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    context = thread->context;
    VEGL_TRACE_API_POST(GetCurrentContext)(context);
    gcmDUMP_API("${EGL eglGetCurrentContext := 0x%08X}", context);
    gcmFOOTER_ARG("return=0x%x", context);
    return context;
}

EGLAPI EGLSurface EGLAPIENTRY
eglGetCurrentSurface(
    EGLint readdraw
    )
{
    VEGLThreadData thread;
    EGLSurface result;

    gcmHEADER_ARG("readdraw=0x%x", readdraw);

    VEGL_TRACE_API_PRE(GetCurrentSurface)(readdraw);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    if ( (thread->context          == gcvNULL)           ||
         (thread->context->context == EGL_NO_CONTEXT) )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmFOOTER_ARG("return=0x%x", EGL_NO_SURFACE);
        return EGL_NO_SURFACE;
    }

    switch (readdraw)
    {
    case EGL_READ:
        result = thread->context->read;
        break;

    case EGL_DRAW:
        result = thread->context->draw;
        break;

    default:
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        result = EGL_NO_SURFACE;
        break;
    }

    VEGL_TRACE_API_POST(GetCurrentSurface)(readdraw, result);
    gcmDUMP_API("${EGL eglGetCurrentSurface 0x%08X := 0x%08X}",
                readdraw, result);
    gcmFOOTER_ARG("return=0x%x", result);
    return result;
}

EGLAPI EGLDisplay EGLAPIENTRY
eglGetCurrentDisplay(
    void
    )
{
    VEGLThreadData thread;

    gcmHEADER();
    VEGL_TRACE_API_PRE(GetCurrentDisplay)();
    gcmDUMP_API("${EGL %s}", __FUNCTION__);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=0x%x", EGL_NO_DISPLAY);
        return EGL_NO_DISPLAY;
    }

    if (thread->context == EGL_NO_CONTEXT)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);

        /* Fatal error. */
        gcmFOOTER_ARG("return=0x%x", EGL_NO_DISPLAY);
        return EGL_NO_DISPLAY;
    }

    veglSetEGLerror(thread,  EGL_SUCCESS);
    VEGL_TRACE_API_POST(GetCurrentDisplay)(thread->context->display);
    gcmDUMP_API("${EGL eglGetCurrentDisplay := 0x%08X}",
                thread->context->display);
    gcmFOOTER_ARG("return=0x%x", thread->context->display);
    return thread->context->display;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglQueryContext(
    EGLDisplay dpy,
    EGLContext ctx,
    EGLint attribute,
    EGLint *value
    )
{
    VEGLDisplay display;
    VEGLContext context;
    VEGLThreadData thread;
    gceSTATUS status;

    gcmHEADER_ARG("dpy=0x%x ctx=0x%x attribute=%d value=0x%x",
                    dpy, ctx, attribute, value);
    VEGL_TRACE_API_PRE(QueryContext)(dpy, ctx, attribute, value);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    display = veglGetDisplay(dpy);
    if (display == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test for a valid context. */
    context = (VEGLContext)veglGetResObj(display,
                                         (VEGLResObj*)&display->contextStack,
                                         (EGLResObj)ctx,
                                         EGL_CONTEXT_SIGNATURE);

    if (context == gcvNULL)
    {
        /* Invalid context. */
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (value != gcvNULL)
    {
        switch (attribute)
        {
       /* EGL_KHR_no_config_context
       ** "Querying EGL_CONFIG_ID returns the ID of the EGLConfig with respect
       **  to which the context was created, or zero if created without
       **  respect to an EGLConfig."
       */
        case EGL_CONFIG_ID:
            *value = context->config.configId ? context->config.configId : 0;
            break;

        case EGL_CONTEXT_CLIENT_TYPE:
            *value = context->api;
            break;

        case EGL_CONTEXT_CLIENT_VERSION:
            *value = MAJOR_API_VER(context->client);
            break;

        case EGL_RENDER_BUFFER:
            if (context->draw == gcvNULL)
            {
                /* Returns EGL_NONE if context is not bound to a surface. */
                *value = EGL_NONE;
            }
            else if (context->draw->type & EGL_PBUFFER_BIT)
            {
                *value = EGL_BACK_BUFFER;
            }
            else if (context->draw->type & EGL_PIXMAP_BIT)
            {
                *value = EGL_SINGLE_BUFFER;
            }
            else if (context->draw->type & EGL_WINDOW_BIT)
            {
                *value = context->draw->buffer;
            }
            break;

        case EGL_PROTECTED_CONTENT_EXT:
            *value = context->protectedContent;
            break;

        default:
            veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmDUMP_API("${EGL eglQueryContext 0x%08X 0x%08X 0x%08X := 0x%08X}",
                    dpy, ctx, attribute, *value);
    }

    veglSetEGLerror(thread, EGL_SUCCESS);
    VEGL_TRACE_API_POST(QueryContext)(dpy, ctx, attribute, value);
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitNative(
    EGLint engine
    )
{
    VEGLThreadData thread;

    gcmHEADER_ARG("engine=0x%x", engine);
    gcmDUMP_API("${EGL eglWaitNative 0x%08X}", engine);
    VEGL_TRACE_API(WaitNative)(engine);

    thread = veglGetThreadData();

    if (gcvNULL == thread)
    {
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    if (engine != EGL_CORE_NATIVE_ENGINE)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Sync native. */
    if (thread->context)
    {
        veglSyncNative(thread, thread->context->display);
    }


    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;
}

#if defined(WL_EGL_PLATFORM)
EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
    VEGLDisplay d = VEGL_DISPLAY(dpy);
    return veglBindWaylandDisplay(d, display);
}

EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
    VEGLDisplay d = VEGL_DISPLAY(dpy);
    return veglUnbindWaylandDisplay(d, display);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryWaylandBufferWL(EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value)
{
    VEGLDisplay d = VEGL_DISPLAY(dpy);

    switch (attribute)
    {
    case EGL_TEXTURE_FORMAT:
        *value = EGL_TEXTURE_RGB;
        /* Not a query, just check if buffer is valid. */
        return veglQueryWaylandBuffer(d, buffer, gcvNULL, gcvNULL, gcvNULL, gcvNULL);
    case EGL_WIDTH:
        return veglQueryWaylandBuffer(d, buffer, value, gcvNULL, gcvNULL, gcvNULL);
    case EGL_HEIGHT:
        return veglQueryWaylandBuffer(d, buffer, gcvNULL, value, gcvNULL, gcvNULL);
    case EGL_WAYLAND_Y_INVERTED_WL:
        *value = 1;
        /* Not a query, just check if buffer is valid. */
        return veglQueryWaylandBuffer(d, buffer, gcvNULL, gcvNULL, gcvNULL, gcvNULL);
    default:
        return EGL_FALSE;
    }

    return EGL_TRUE;
}
#endif


