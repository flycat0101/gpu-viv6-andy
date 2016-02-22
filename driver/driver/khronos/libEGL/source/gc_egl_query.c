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


#include "gc_egl_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_API

#if defined(WL_EGL_PLATFORM)
#include "wayland-server.h"
#endif

EGLAPI EGLBoolean EGLAPIENTRY
eglSwapInterval(
    EGLDisplay Dpy,
    EGLint Interval
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
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

    if (dpy->hdc == (NativeDisplayType)gcvNULL)
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

    if (thread->context->draw == EGL_NO_SURFACE)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (!veglSetSwapInterval(dpy, Interval))
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
        case EGL_CONFIG_ID:
            *value = context->config.configId;
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

EGLAPI EGLBoolean EGLAPIENTRY
eglPatchID(
    EGLenum *PatchID,
    EGLBoolean Set
    )
{
#if gcdENABLE_3D

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    if (Set)
    {
        gcoHAL_SetGlobalPatchID(gcvNULL, *PatchID);
    }
    else
    {
        gcoHAL_GetPatchID(gcvNULL, (gcePATCH_ID*)PatchID);
    }
#endif

    return EGL_TRUE;
}

/*
 * The following GL API functions are used for eglGetProcAddress() which is independent of the currently bound client API.
 * As different GLES client APIs have the same API entry functions, eglGetProcAddress() should always return the function
 * pointers here. And the following API functions will dispatch the API call to the currently bound client API
 */
#ifndef GL_APIENTRY
#define GL_APIENTRY KHRONOS_APIENTRY
#endif

/* GL_OES_EGL_image */

typedef void (GL_APIENTRY *ImageTargetTexture2DFunc)(EGLenum, void*);
typedef void (GL_APIENTRY *ImageTargetRenderbufferStorageFunc)(EGLenum, void*);

void GL_APIENTRY glEGLImageTargetTexture2DOES_Entry(EGLenum target, void* image)
{
    ImageTargetTexture2DFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->imageTargetTex2DFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->imageTargetTex2DFunc[index] =
                    dispatch->getProcAddr("glEGLImageTargetTexture2DOES");
            }
        }

        extfunc = (ImageTargetTexture2DFunc)thread->imageTargetTex2DFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, image);
        }
    }
}

void GL_APIENTRY glEGLImageTargetRenderbufferStorageOES_Entry(EGLenum target, void* image)
{
    ImageTargetRenderbufferStorageFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);

        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->imageTargetRBStorageFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->imageTargetRBStorageFunc[index] =
                    dispatch->getProcAddr("glEGLImageTargetRenderbufferStorageOES");
            }
        }

        extfunc = (ImageTargetRenderbufferStorageFunc)thread->imageTargetRBStorageFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, image);
        }
    }
}

/* GL_EXT_multi_draw_arrays */

typedef void (GL_APIENTRY *MultiDrawArraysFunc)(EGLenum mode, EGLint *first, EGLint *count, EGLint primcount);
typedef void (GL_APIENTRY *MultiDrawElementsFunc)(EGLenum mode, const EGLint *count, EGLenum type, const void* *indices, EGLint primcount);

void GL_APIENTRY glMultiDrawArraysEXT_Entry(EGLenum mode, EGLint *first, EGLint *count, EGLint primcount)
{
    MultiDrawArraysFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->multiDrawArraysFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->multiDrawArraysFunc[index] =
                    dispatch->getProcAddr("glMultiDrawArraysEXT");
            }
        }

        extfunc = (MultiDrawArraysFunc)thread->multiDrawArraysFunc[index];
        if (extfunc)
        {
            (*extfunc)(mode, first, count, primcount);
        }
    }
}

void GL_APIENTRY glMultiDrawElementsEXT_Entry(EGLenum mode, const EGLint *count, EGLenum type, const void* *indices, EGLint primcount)
{
    MultiDrawElementsFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->multiDrawElementsFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->multiDrawElementsFunc[index] =
                    dispatch->getProcAddr("glMultiDrawElementsEXT");
            }
        }

        extfunc = (MultiDrawElementsFunc)thread->multiDrawElementsFunc[index];
        if (extfunc)
        {
            (*extfunc)(mode, count, type, indices, primcount);
        }
    }
}

/* GL_OES_mapbuffer */

typedef void (GL_APIENTRY *GetBufferPointervFunc)(EGLenum target, EGLenum pname, void** params);
typedef void* (GL_APIENTRY *MapBufferFunc)(EGLenum target, EGLenum access);
typedef EGLBoolean (GL_APIENTRY *UnmapBufferFunc)(EGLenum target);

void GL_APIENTRY glGetBufferPointervOES_Entry(EGLenum target, EGLenum pname, void** params)
{
    GetBufferPointervFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->getBufferPointervFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->getBufferPointervFunc[index] =
                    dispatch->getProcAddr("glGetBufferPointervFuncOES");
            }
        }

        extfunc = (GetBufferPointervFunc)thread->getBufferPointervFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, pname, params);
        }
    }
}

void* GL_APIENTRY glMapBufferOES_Entry(EGLenum target, EGLenum access)
{
    MapBufferFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return gcvNULL;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->mapBufferFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->mapBufferFunc[index] =
                    dispatch->getProcAddr("glMapBufferOES");
            }
        }

        extfunc = (MapBufferFunc)thread->mapBufferFunc[index];
        if (extfunc)
        {
            return (*extfunc)(target, access);
        }
    }

    return gcvNULL;
}

EGLBoolean GL_APIENTRY glUnmapBufferOES_Entry(EGLenum target)
{
    UnmapBufferFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return gcvFALSE;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->unmapBufferFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->unmapBufferFunc[index] =
                    dispatch->getProcAddr("glUnmapBufferOES");
            }
        }

        extfunc = (UnmapBufferFunc)thread->unmapBufferFunc[index];
        if (extfunc)
        {
            return (*extfunc)(target);
        }
    }

    return gcvFALSE;
}

/* GL_EXT_discard_framebuffer */

typedef void (GL_APIENTRY *DiscardFramebufferFunc)(EGLenum target, EGLint numAttachments, const EGLenum *attachments);

void GL_APIENTRY glDiscardFramebufferEXT_Entry(EGLenum target, EGLint numAttachments, const EGLenum *attachments)
{
    DiscardFramebufferFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->discardFramebuffer[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->discardFramebuffer[index] =
                    dispatch->getProcAddr("glDiscardFramebufferEXT");
            }
        }

        extfunc = (DiscardFramebufferFunc)thread->discardFramebuffer[index];
        if (extfunc)
        {
            (*extfunc)(target, numAttachments, attachments);
        }
    }
}

/* GL_EXT_multisampled_render_to_texture */
typedef void (GL_APIENTRY *RenderbufferStorageMultisampleFunc)(EGLenum target, EGLint samples, EGLenum internalformat, EGLint width, EGLint height);
typedef void (GL_APIENTRY *FramebufferTexture2DMultisampleFunc)(EGLenum target, EGLenum attachment, EGLenum textarget, EGLint texture, EGLint level, EGLint samples);

void GL_APIENTRY glRenderbufferStorageMultisampleEXT_Entry(EGLenum target, EGLint samples, EGLenum internalformat, EGLint width, EGLint height)
{
    RenderbufferStorageMultisampleFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->renderbufferStorageMultisampleFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->renderbufferStorageMultisampleFunc[index] =
                    dispatch->getProcAddr("glRenderbufferStorageMultisampleEXT");
            }
        }

        extfunc = (RenderbufferStorageMultisampleFunc)thread->renderbufferStorageMultisampleFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, samples, internalformat, width, height);
        }
    }
}

void GL_APIENTRY glFramebufferTexture2DMultisampleEXT_Entry(EGLenum target, EGLenum attachment, EGLenum textarget, EGLint texture, EGLint level, EGLint samples)
{
    FramebufferTexture2DMultisampleFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->framebufferTexture2DMultisampleFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->framebufferTexture2DMultisampleFunc[index] =
                    dispatch->getProcAddr("glFramebufferTexture2DMultisampleEXT");
            }
        }

        extfunc = (FramebufferTexture2DMultisampleFunc)thread->framebufferTexture2DMultisampleFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, attachment, textarget, texture, level, samples);
        }
    }
}

/* GL_OES_get_program_binary */

typedef void (GL_APIENTRY *GetProgramBinaryFunc)(EGLint program, EGLint bufSize, EGLint* length, EGLenum* binaryFormat, void* binary);
typedef void (GL_APIENTRY *ProgramBinaryFunc)(EGLint program, EGLenum binaryFormat, const void* binary, EGLint length);

void GL_APIENTRY glGetProgramBinaryOES_Entry(EGLint program, EGLint bufSize, EGLint* length, EGLenum* binaryFormat, void* binary)
{
    GetProgramBinaryFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->getProgramBinaryFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->getProgramBinaryFunc[index] =
                    dispatch->getProcAddr("glGetProgramBinaryOES");
            }
        }

        extfunc = (GetProgramBinaryFunc)thread->getProgramBinaryFunc[index];
        if (extfunc)
        {
            (*extfunc)(program, bufSize, length, binaryFormat, binary);
        }
    }
}

void GL_APIENTRY glProgramBinaryOES_Entry(EGLint program, EGLenum binaryFormat, const void* binary, EGLint length)
{
    ProgramBinaryFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->programBinaryFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->programBinaryFunc[index] =
                    dispatch->getProcAddr("glProgramBinaryOES");
            }
        }

        extfunc = (ProgramBinaryFunc)thread->programBinaryFunc[index];
        if (extfunc)
        {
            (*extfunc)(program, binaryFormat, binary, length);
        }
    }
}

/* GL_VIV_direct_texture */

typedef void (GL_APIENTRY *TexDirectFunc)(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** pixels);
typedef void (GL_APIENTRY *TexDirectInvalidateFunc)(EGLenum target);
typedef void (GL_APIENTRY *TexDirectVIVMapFunc)(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** logical, const EGLint * physical);
typedef void (GL_APIENTRY *TexDirectTiledMapFunc)(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** logical, const EGLint * physical);

void GL_APIENTRY glTexDirectVIV_Entry(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** pixels)
{
    TexDirectFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->texDirectFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->texDirectFunc[index] =
                    dispatch->getProcAddr("glTexDirectVIV");
            }
        }

        extfunc = (TexDirectFunc)thread->texDirectFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, width, height, format, pixels);
        }
    }
}

void GL_APIENTRY glTexDirectInvalidateVIV_Entry(EGLenum target)
{
    TexDirectInvalidateFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->texDirectInvalidateFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->texDirectInvalidateFunc[index] =
                    dispatch->getProcAddr("glTexDirectInvalidateVIV");
            }
        }

        extfunc = (TexDirectInvalidateFunc)thread->texDirectInvalidateFunc[index];
        if (extfunc)
        {
            (*extfunc)(target);
        }
    }
}

void GL_APIENTRY glTexDirectVIVMap_Entry(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** logical, const EGLint * physical)
{
    TexDirectVIVMapFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->texDirectMapFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->texDirectMapFunc[index] =
                    dispatch->getProcAddr("glTexDirectVIVMap");
            }
        }

        extfunc = (TexDirectVIVMapFunc)thread->texDirectMapFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, width, height, format, logical, physical);
        }
    }
}

void GL_APIENTRY glTexDirectTiledMapVIV_Entry(EGLenum target, EGLint width, EGLint height, EGLenum format, void ** logical, const EGLint * physical)
{
    TexDirectTiledMapFunc extfunc;
    VEGLThreadData thread;
    veglDISPATCH *dispatch;
    EGLint index;

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.", __FUNCTION__, __LINE__);
        return;
    }

    if (thread->api == EGL_OPENGL_ES_API && thread->context)
    {
        index = MAJOR_API_VER(thread->context->client) - 1;

        if (thread->texDirectTiledMapFunc[index] == gcvNULL)
        {
            dispatch = _GetDispatch(thread, gcvNULL);

            if (dispatch && dispatch->getProcAddr)
            {
                thread->texDirectTiledMapFunc[index] =
                    dispatch->getProcAddr("glTexDirectTiledMapVIV");
            }
        }

        extfunc = (TexDirectTiledMapFunc)thread->texDirectTiledMapFunc[index];
        if (extfunc)
        {
            (*extfunc)(target, width, height, format, logical, physical);
        }
    }
}

#if defined(WL_EGL_PLATFORM)
EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
    gcsWL_LOCAL_DISPLAY* wl_localDisplay;
    gctPOINTER tmp = VEGL_DISPLAY(dpy)->localInfo;

    gcoOS_AllocateMemory(gcvNULL, sizeof *wl_localDisplay, (gctPOINTER) &wl_localDisplay );
    gcoOS_ZeroMemory( wl_localDisplay, sizeof *wl_localDisplay);

    wl_localDisplay->wl_signature = WL_LOCAL_DISPLAY_SIGNATURE;
    wl_localDisplay->localInfo = (gctPOINTER)display;

    VEGL_DISPLAY(dpy)->localInfo = wl_localDisplay;
    veglInitLocalDisplayInfo(VEGL_DISPLAY(dpy));

    VEGL_DISPLAY(dpy)->localInfo = tmp;

    gcoOS_FreeMemory(gcvNULL, wl_localDisplay);

    return EGL_TRUE;
}
EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
    return veglDeinitLocalDisplayInfo(VEGL_DISPLAY(dpy));
}
EGLAPI EGLBoolean EGLAPIENTRY eglQueryWaylandBufferWL(EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value)
{
    gcsWL_VIV_BUFFER *wl_viv_buffer;
    wl_viv_buffer = wl_resource_get_user_data(buffer);
    switch (attribute)
    {
        case EGL_TEXTURE_FORMAT:
            *value = EGL_TEXTURE_RGB;
            return EGL_TRUE;
        case EGL_WIDTH:
            *value = wl_viv_buffer->width;
            return EGL_TRUE;
        case EGL_HEIGHT:
            *value = wl_viv_buffer->height;
            return EGL_TRUE;
        case EGL_WAYLAND_Y_INVERTED_WL:
            *value = 1;
            return EGL_TRUE;
        default: ;
    }
    return EGL_FALSE;
}
#endif

static veglLOOKUP _veglLookup[] =
{
    eglMAKE_LOOKUP(eglGetError),
    eglMAKE_LOOKUP(eglGetDisplay),
    eglMAKE_LOOKUP(eglInitialize),
    eglMAKE_LOOKUP(eglTerminate),
    eglMAKE_LOOKUP(eglQueryString),
    eglMAKE_LOOKUP(eglGetConfigs),
    eglMAKE_LOOKUP(eglChooseConfig),
    eglMAKE_LOOKUP(eglGetConfigAttrib),
    eglMAKE_LOOKUP(eglCreateWindowSurface),
    eglMAKE_LOOKUP(eglCreatePbufferSurface),
    eglMAKE_LOOKUP(eglCreatePixmapSurface),
    eglMAKE_LOOKUP(eglDestroySurface),
    eglMAKE_LOOKUP(eglQuerySurface),
    eglMAKE_LOOKUP(eglBindAPI),
    eglMAKE_LOOKUP(eglQueryAPI),
    eglMAKE_LOOKUP(eglWaitClient),
    eglMAKE_LOOKUP(eglReleaseThread),
    eglMAKE_LOOKUP(eglCreatePbufferFromClientBuffer),
    eglMAKE_LOOKUP(eglSurfaceAttrib),
    eglMAKE_LOOKUP(eglBindTexImage),
    eglMAKE_LOOKUP(eglReleaseTexImage),
    eglMAKE_LOOKUP(eglSwapInterval),
    eglMAKE_LOOKUP(eglCreateContext),
    eglMAKE_LOOKUP(eglDestroyContext),
    eglMAKE_LOOKUP(eglMakeCurrent),
    eglMAKE_LOOKUP(eglGetCurrentContext),
    eglMAKE_LOOKUP(eglGetCurrentSurface),
    eglMAKE_LOOKUP(eglGetCurrentDisplay),
    eglMAKE_LOOKUP(eglQueryContext),
    eglMAKE_LOOKUP(eglWaitGL),
    eglMAKE_LOOKUP(eglWaitNative),
    eglMAKE_LOOKUP(eglSwapBuffers),
    eglMAKE_LOOKUP(eglCopyBuffers),
    eglMAKE_LOOKUP(eglGetProcAddress),
    /* EGL 1.5 */
    eglMAKE_LOOKUP(eglCreateSync),
    eglMAKE_LOOKUP(eglDestroySync),
    eglMAKE_LOOKUP(eglClientWaitSync),
    eglMAKE_LOOKUP(eglGetSyncAttrib),
    eglMAKE_LOOKUP(eglCreateImage),
    eglMAKE_LOOKUP(eglDestroyImage),
    eglMAKE_LOOKUP(eglGetPlatformDisplay),
    eglMAKE_LOOKUP(eglCreatePlatformWindowSurface),
    eglMAKE_LOOKUP(eglCreatePlatformPixmapSurface),
    eglMAKE_LOOKUP(eglWaitSync),
    /* EGL_KHR_lock_surface. */
    eglMAKE_LOOKUP(eglLockSurfaceKHR),
    eglMAKE_LOOKUP(eglUnlockSurfaceKHR),
    /* EGL_KHR_image. */
    eglMAKE_LOOKUP(eglCreateImageKHR),
    eglMAKE_LOOKUP(eglDestroyImageKHR),
    /* EGL_KHR_fence_sync. */
    eglMAKE_LOOKUP(eglCreateSyncKHR),
    eglMAKE_LOOKUP(eglDestroySyncKHR),
    eglMAKE_LOOKUP(eglClientWaitSyncKHR),
    eglMAKE_LOOKUP(eglGetSyncAttribKHR),
    /* EGL_KHR_reusable_sync. */
    eglMAKE_LOOKUP(eglSignalSyncKHR),
    /* EGL_KHR_wait_sync. */
    eglMAKE_LOOKUP(eglWaitSyncKHR),
#if defined(WL_EGL_PLATFORM)
    /* EGL_WL_bind_wayland_display. */
    eglMAKE_LOOKUP(eglBindWaylandDisplayWL),
    eglMAKE_LOOKUP(eglUnbindWaylandDisplayWL),
    eglMAKE_LOOKUP(eglQueryWaylandBufferWL),
#endif
#if defined(ANDROID) && gcdANDROID_NATIVE_FENCE_SYNC
    /* EGL_ANDROID_native_fence_sync. */
    eglMAKE_LOOKUP(eglDupNativeFenceFDANDROID),
#endif
    eglMAKE_LOOKUP(eglPatchID),
    /* GL_OES_EGL_image */
    eglMAKE_EXT_ENTRY(glEGLImageTargetTexture2DOES),
    eglMAKE_EXT_ENTRY(glEGLImageTargetRenderbufferStorageOES),
    /* GL_EXT_multi_draw_arrays */
    eglMAKE_EXT_ENTRY(glMultiDrawArraysEXT),
    eglMAKE_EXT_ENTRY(glMultiDrawElementsEXT),
    /* GL_OES_mapbuffer */
    eglMAKE_EXT_ENTRY(glGetBufferPointervOES),
    eglMAKE_EXT_ENTRY(glMapBufferOES),
    eglMAKE_EXT_ENTRY(glUnmapBufferOES),
    /* GL_EXT_discard_framebuffer */
    eglMAKE_EXT_ENTRY(glDiscardFramebufferEXT),
    /* GL_EXT_multisampled_render_to_texture */
    eglMAKE_EXT_ENTRY(glRenderbufferStorageMultisampleEXT),
    eglMAKE_EXT_ENTRY(glFramebufferTexture2DMultisampleEXT),
    /* GL_OES_get_program_binary */
    eglMAKE_EXT_ENTRY(glGetProgramBinaryOES),
    eglMAKE_EXT_ENTRY(glProgramBinaryOES),
    /* GL_VIV_direct_texture */
    eglMAKE_EXT_ENTRY(glTexDirectVIV),
    eglMAKE_EXT_ENTRY(glTexDirectInvalidateVIV),
    eglMAKE_EXT_ENTRY(glTexDirectVIVMap),
    eglMAKE_EXT_ENTRY(glTexDirectTiledMapVIV),

    { gcvNULL, gcvNULL }
};

static EGL_PROC
_Lookup(
    veglLOOKUP * Lookup,
    const char * Name,
    const char * Appendix
    )
{
    /* Test for lookup. */
    if (Lookup != gcvNULL)
    {
        /* Loop while there are entries in the lookup tabke. */
        while (Lookup->name != gcvNULL)
        {
            const char *p = Name;
            const char *q = Lookup->name;

            /* Compare the name and the lookup table. */
            while ((*p == *q) && (*p != '\0') && (*q != '\0'))
            {
                ++p;
                ++q;
            }

            /* No match yet, see if it matches if we append the appendix. */
            if ((*p != *q) && (*p == '\0') && (Appendix != gcvNULL))
            {
                p = Appendix;

                /* Compare the appendix and the lookup table. */
                while ((*p == *q) && (*p != '\0') && (*q != '\0'))
                {
                    ++p;
                    ++q;
                }
            }

            /* See if we have a match. */
            if (*p == *q)
            {
                /* Return the function pointer. */
                return Lookup->function;
            }

            /* Next lookup entry. */
            ++Lookup;
        }
    }

    /* No match found. */
    return gcvNULL;
}

#define gcmDEF2STRING(def) #def

EGLAPI EGL_PROC EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
    union gcuVARIANT
    {
        EGL_PROC   func;
        gctPOINTER ptr;
    } proc;
    VEGLThreadData thread;
    const char * appendix = gcvNULL;
    veglDISPATCH * dispatch;
#if !gcdSTATIC_LINK
    gctHANDLE library;
    veglAPIINDEX index;
#endif
    char * name;
    gctSIZE_T nameLen = 0, appendixLen = 0;
    gctSIZE_T len = 0;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("procname=%s", procname);

    VEGL_TRACE_API_PRE(GetProcAddress)(procname);

    if ((procname == gcvNULL) || (procname[0] == '\0'))
    {
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    /* Lookup in EGL API. */
#if defined _EGL_APPENDIX
    proc.func = _Lookup(_veglLookup, procname, gcmDEF2STRING(_EGL_APPENDIX));
#else
    proc.func = _Lookup(_veglLookup, procname, gcvNULL);
#endif
    if (proc.func != gcvNULL)
    {
        goto Exit;
    }

    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.",
                 __FUNCTION__, __LINE__);

        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    dispatch = _GetDispatch(thread, gcvNULL);

    switch (thread->api)
    {
    case EGL_OPENGL_ES_API:
#if gcdENABLE_3D
        if ((thread->context == gcvNULL)
        ||  (MAJOR_API_VER(thread->context->client) == 1)
        )
        {
            /* OpenGL ES 1.1 API. */
#if defined _GL_11_APPENDIX
            appendix = gcmDEF2STRING(_GL_11_APPENDIX);
#else
            appendix = gcvNULL;
#endif
        }
        else if (MAJOR_API_VER(thread->context->client) == 2)
        {
            /* OpenGL ES 2.0 API. */
#if defined _GL_2_APPENDIX
            appendix = gcmDEF2STRING(_GL_2_APPENDIX);
#else
            appendix = gcvNULL;
#endif
        }
        else
#endif
        {
            appendix = gcvNULL;
        }
        break;

    case EGL_OPENVG_API:
#if defined(VIVANTE_NO_VG)
        /* VG driver is not available. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmTRACE_ZONE(gcvLEVEL_WARNING, gcdZONE_EGL_API,
                      "%s(%d): %s",
                      __FUNCTION__, __LINE__,
                      "VG driver is not available.");
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
#else
#if defined _VG_APPENDIX
        appendix = gcmDEF2STRING(_VG_APPENDIX);
#else
        appendix = gcvNULL;
#endif
#endif
        break;

    default:
        appendix = gcvNULL;
        break;
    }

    if (dispatch && dispatch->getProcAddr)
    {
        if (appendix)
        {
            nameLen = gcoOS_StrLen(procname, gcvNULL);
            appendixLen = gcoOS_StrLen(appendix, gcvNULL);
            len = nameLen + appendixLen + 1;
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                           len,
                                           &pointer)))
            {
                veglSetEGLerror(thread,  EGL_BAD_ALLOC);
                gcmFOOTER_ARG("0x%x", gcvNULL);
                return gcvNULL;
            }

            name = pointer;

            gcmVERIFY_OK(gcoOS_StrCopySafe(name, len, procname));
            gcmVERIFY_OK(gcoOS_StrCatSafe(name, len, appendix));

            proc.func = dispatch->getProcAddr(name);

            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, name));
        }
        else
        {
            proc.func = dispatch->getProcAddr(procname);
        }
    }

    if (proc.func != gcvNULL)
    {
        goto Exit;
    }

#if !gcdSTATIC_LINK
    nameLen = gcoOS_StrLen(procname, gcvNULL);

#if defined _EGL_APPENDIX
    len = nameLen + gcmSIZEOF(gcmDEF2STRING(_EGL_APPENDIX)) + 1;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                   len,
                                   (gctPOINTER *) &name)))
    {
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    gcmVERIFY_OK(gcoOS_StrCopySafe(name, len, procname));
    gcmVERIFY_OK(gcoOS_StrCatSafe(name, len, gcmDEF2STRING(_EGL_APPENDIX)));
#else
    name = gcvNULL;
#endif

    /* Try loading from libEGL. */
    library = veglGetModule(gcvNULL, vegl_EGL, gcvNULL);

    if (library != gcvNULL)
    {
        if (gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                               library,
                                               procname,
                                               &proc.ptr)))
        {
            goto Done;
        }

        if ((name != gcvNULL)
        &&  gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                               library,
                                               name,
                                               &proc.ptr))
        )
        {
            goto Done;
        }
    }

    if (name != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, name));
    }

    if (appendix == gcvNULL)
    {
        name = gcvNULL;
    }
    else
    {
        appendixLen = gcoOS_StrLen(appendix, gcvNULL);
        len = nameLen + appendixLen + 1;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                       len,
                                       &pointer)))
        {
            veglSetEGLerror(thread,  EGL_BAD_ALLOC);
            gcmFOOTER_ARG("0x%x", gcvNULL);
            return gcvNULL;
        }

        name = pointer;

        gcmVERIFY_OK(gcoOS_StrCopySafe(name, len, procname));
        gcmVERIFY_OK(gcoOS_StrCatSafe(name, len, appendix));
    }

    /* Try iterate all client library. */
    for (index = 0; index < vegl_API_LAST; ++index)
    {
        dispatch = gcvNULL;
        library = veglGetModule(gcvNULL, index, &dispatch);

        if (library != gcvNULL)
        {
            if (gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                                   library,
                                                   procname,
                                                   &proc.ptr)))
            {
                goto Done;
            }

            if (dispatch && dispatch->getProcAddr)
            {
                proc.func = dispatch->getProcAddr(procname);
                if (proc.func)
                {
                    goto Done;
                }
            }

            if (name)
            {
                if (gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                                       library,
                                                       name,
                                                       &proc.ptr)))
                {
                    goto Done;
                }

                if (dispatch && dispatch->getProcAddr)
                {
                    proc.func = dispatch->getProcAddr(name);
                    if (proc.func)
                    {
                        goto Done;
                    }
                }
            }
        }
    }

    proc.func = gcvNULL;

Done:
    veglSetEGLerror(thread,  EGL_SUCCESS);

    if (name != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, name));
    }
#endif /* gcdSTATIC_LINK */

Exit:
    VEGL_TRACE_API_POST(GetProcAddress)(procname, proc.ptr);
    gcmDUMP_API("${EGL eglGetProcAddress (0x%08X) := 0x%08X",
                procname, proc.func);
    gcmDUMP_API_DATA(procname, 0);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("0x%x", proc.func);
    return proc.func;
}
