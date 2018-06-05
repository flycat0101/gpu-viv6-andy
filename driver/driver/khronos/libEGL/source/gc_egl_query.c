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
 * XXX: Following typedefs and definitions are copied from GLES header file.
 * Maybe include GLES header file directly is better.
 *
 * Define data types here because 'gc_egl.h' is incorrectly included by GLES
 * driver. Duplicated typedefs may cause problems on some compilers.
 */
typedef void GLvoid;
typedef unsigned int GLenum;
typedef khronos_float_t GLfloat;
typedef khronos_int32_t GLfixed;
typedef unsigned int GLuint;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef khronos_uint8_t GLubyte;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef khronos_int32_t GLclampx;
typedef unsigned short   GLushort;

typedef void *GLeglImageOES;

#ifndef GL_API
#define GL_API      KHRONOS_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY KHRONOS_APIENTRY
#endif

/*
 * OpenGL/OpenGL ES Client API entries for eglGetProcAddress.
 * This table only includes APIs with same name in es11 and es3x.
 */
#define GL_API_ENTRIES(GL_ENTRY) \
    GL_ENTRY(void, glActiveTexture, GLenum texture) \
    GL_ENTRY(void, glBindBuffer, GLenum target, GLuint buffer) \
    GL_ENTRY(void, glBindTexture, GLenum target, GLuint texture) \
    GL_ENTRY(void, glBlendFunc, GLenum sfactor, GLenum dfactor) \
    GL_ENTRY(void, glBufferData, GLenum target, GLsizeiptr size, const void * data, GLenum usage) \
    GL_ENTRY(void, glBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void * data) \
    GL_ENTRY(void, glClear, GLbitfield mask) \
    GL_ENTRY(void, glClearColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
    GL_ENTRY(void, glClearDepthf, GLfloat d) \
    GL_ENTRY(void, glClearStencil, GLint s) \
    GL_ENTRY(void, glColorMask, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) \
    GL_ENTRY(void, glCompressedTexImage2D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data) \
    GL_ENTRY(void, glCompressedTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data) \
    GL_ENTRY(void, glCopyTexImage2D, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) \
    GL_ENTRY(void, glCopyTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glCullFace, GLenum mode) \
    GL_ENTRY(void, glDeleteBuffers, GLsizei n, const GLuint * buffers) \
    GL_ENTRY(void, glDeleteTextures, GLsizei n, const GLuint * textures) \
    GL_ENTRY(void, glDepthFunc, GLenum func) \
    GL_ENTRY(void, glDepthMask, GLboolean flag) \
    GL_ENTRY(void, glDepthRangef, GLfloat n, GLfloat f) \
    GL_ENTRY(void, glDisable, GLenum cap) \
    GL_ENTRY(void, glDrawArrays, GLenum mode, GLint first, GLsizei count) \
    GL_ENTRY(void, glDrawElements, GLenum mode, GLsizei count, GLenum type, const void * indices) \
    GL_ENTRY(void, glEnable, GLenum cap) \
    GL_ENTRY(void, glFinish, void) \
    GL_ENTRY(void, glFlush, void) \
    GL_ENTRY(void, glFrontFace, GLenum mode) \
    GL_ENTRY(void, glGenBuffers, GLsizei n, GLuint * buffers) \
    GL_ENTRY(void, glGenTextures, GLsizei n, GLuint * textures) \
    GL_ENTRY(void, glGetBooleanv, GLenum pname, GLboolean * data) \
    GL_ENTRY(void, glGetBufferParameteriv, GLenum target, GLenum pname, GLint * params) \
    GL_ENTRY(GLenum, glGetError, void) \
    GL_ENTRY(void, glGetFloatv, GLenum pname, GLfloat * data) \
    GL_ENTRY(void, glGetIntegerv, GLenum pname, GLint * data) \
    GL_ENTRY(void, glGetPointerv, GLenum pname, void ** params) \
    GL_ENTRY(const GLubyte *, glGetString, GLenum name) \
    GL_ENTRY(void, glGetTexParameterfv, GLenum target, GLenum pname, GLfloat * params) \
    GL_ENTRY(void, glGetTexParameteriv, GLenum target, GLenum pname, GLint * params) \
    GL_ENTRY(void, glHint, GLenum target, GLenum mode) \
    GL_ENTRY(GLboolean, glIsBuffer, GLuint buffer) \
    GL_ENTRY(GLboolean, glIsEnabled, GLenum cap) \
    GL_ENTRY(GLboolean, glIsTexture, GLuint texture) \
    GL_ENTRY(void, glLineWidth, GLfloat width) \
    GL_ENTRY(void, glPixelStorei, GLenum pname, GLint param) \
    GL_ENTRY(void, glPolygonOffset, GLfloat factor, GLfloat units) \
    GL_ENTRY(void, glReadPixels, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels) \
    GL_ENTRY(void, glSampleCoverage, GLfloat value, GLboolean invert) \
    GL_ENTRY(void, glScissor, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glStencilFunc, GLenum func, GLint ref, GLuint mask) \
    GL_ENTRY(void, glStencilMask, GLuint mask) \
    GL_ENTRY(void, glStencilOp, GLenum fail, GLenum zfail, GLenum zpass) \
    GL_ENTRY(void, glTexImage2D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels) \
    GL_ENTRY(void, glTexParameterf, GLenum target, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glTexParameterfv, GLenum target, GLenum pname, const GLfloat * params) \
    GL_ENTRY(void, glTexParameteri, GLenum target, GLenum pname, GLint param) \
    GL_ENTRY(void, glTexParameteriv, GLenum target, GLenum pname, const GLint * params) \
    GL_ENTRY(void, glTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels) \
    GL_ENTRY(void, glViewport, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glEGLImageTargetRenderbufferStorageOES, GLenum target, GLeglImageOES image) \
    GL_ENTRY(void, glEGLImageTargetTexture2DOES, GLenum target, GLeglImageOES image) \
    GL_ENTRY(void, glGetBufferPointervOES, GLenum target, GLenum pname, void ** params) \
    GL_ENTRY(void *, glMapBufferOES, GLenum target, GLenum access) \
    GL_ENTRY(GLboolean, glUnmapBufferOES, GLenum target) \
    GL_ENTRY(void, glMultiDrawArraysEXT, GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount) \
    GL_ENTRY(void, glMultiDrawElementsEXT, GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei primcount) \
    GL_ENTRY(void, glDiscardFramebufferEXT, GLenum target, GLsizei numAttachments, const GLenum *attachments) \
    GL_ENTRY(void, glRenderbufferStorageMultisampleEXT, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glFramebufferTexture2DMultisampleEXT, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) \
    GL_ENTRY(void, glGetProgramBinaryOES, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) \
    GL_ENTRY(void, glProgramBinaryOES, GLuint program, GLenum binaryFormat, const void *binary, GLint length) \
    GL_ENTRY(void, glTexDirectVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels) \
    GL_ENTRY(void, glTexDirectVIVMap, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectMapVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectTiledMapVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectInvalidateVIV, GLenum Target)


#define apiNameEntry(__ret, __api, ...) #__api,

const char * glApiNames[] =
{
    GL_API_ENTRIES(apiNameEntry)
};

#define apiHookEntry(__ret, __api, ...) __ret (GL_APIENTRY * __api)(__VA_ARGS__);

typedef struct _veglGLAPIHook
{
    GL_API_ENTRIES(apiHookEntry)
}
veglGLAPIHook;

/* Global gl hooks. Do not need to be thread specific. */
static veglGLAPIHook glHooks[3];
static EGLBoolean    glHoolInitialized[3];

veglGLAPIHook *
_GetGLAPIHook(
    void
    )
{
    gctSIZE_T i;
    EGLint index = 0;
    gctHANDLE library;
    veglDISPATCH *dispatch = gcvNULL;
    __eglMustCastToProperFunctionPointerType *api;

    VEGLThreadData thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        return gcvNULL;
    }

    if (thread->esContext)
    {
        index = MAJOR_API_VER(thread->esContext->client) - 1;
    }

    if (glHoolInitialized[index])
    {
        return &glHooks[index];
    }

    /* To avoid multiple initializations. */
    gcoOS_LockPLS();

    /* Check again. */
    if (glHoolInitialized[index])
    {
        gcoOS_UnLockPLS();
        return &glHooks[index];
    }

    /* Get client driver. */
    library  = thread->clientHandles[vegl_OPENGL_ES11 + index];
    dispatch = thread->dispatchTables[vegl_OPENGL_ES11 + index];

    if (dispatch == gcvNULL && index == 0)
    {
        /* Try commit-line profile. */
        library  = thread->clientHandles[vegl_OPENGL_ES11_CL];
        dispatch = thread->dispatchTables[vegl_OPENGL_ES11_CL];
    }

    /* Dispatch should not be null. */
    gcmASSERT(dispatch != gcvNULL);

    /* Set as initialized. */
    glHoolInitialized[index] = EGL_TRUE;

    /* Cast glHook as function pointer array. */
    api = (__eglMustCastToProperFunctionPointerType *) &glHooks[index];

    /* Now load the symbols. */
    for (i = 0; i < gcmCOUNTOF(glApiNames); i++)
    {
        __eglMustCastToProperFunctionPointerType func = gcvNULL;

        if (dispatch->getProcAddr)
        {
            func = dispatch->getProcAddr(glApiNames[i]);
        }

        if (!func && library)
        {
            gcoOS_GetProcAddress(gcvNULL, library, glApiNames[i], (gctPOINTER *) &func);
        }

        *api++ = func;
    }

    gcoOS_UnLockPLS();
    return &glHooks[index];
}


/*
 * define forward functions, like
 * GLvoid forward_##glXXX(...)
 */
#define API_ENTRY(__api) forward_##__api

#define CALL_GL_API(__api, ...) \
    veglGLAPIHook *hook = _GetGLAPIHook(); \
    if (hook && hook->__api) \
    { \
        hook->__api(__VA_ARGS__); \
    }

#define CALL_GL_API_RETURN(__api, ...) \
    veglGLAPIHook *hook = _GetGLAPIHook(); \
    if (hook && hook->__api) \
    { \
        return hook->__api(__VA_ARGS__); \
    } \
    return 0;

static void GL_APIENTRY API_ENTRY(glActiveTexture)(GLenum texture)
{
    CALL_GL_API(glActiveTexture, texture);
}

static void GL_APIENTRY API_ENTRY(glBindBuffer)(GLenum target, GLuint buffer)
{
    CALL_GL_API(glBindBuffer, target, buffer);
}

static void GL_APIENTRY API_ENTRY(glBindTexture)(GLenum target, GLuint texture)
{
    CALL_GL_API(glBindTexture, target, texture);
}

static void GL_APIENTRY API_ENTRY(glBlendFunc)(GLenum sfactor, GLenum dfactor)
{
    CALL_GL_API(glBlendFunc, sfactor, dfactor);
}

static void GL_APIENTRY API_ENTRY(glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    CALL_GL_API(glBufferData, target, size, data, usage);
}

static void GL_APIENTRY API_ENTRY(glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    CALL_GL_API(glBufferSubData, target, offset, size, data);
}

static void GL_APIENTRY API_ENTRY(glClear)(GLbitfield mask)
{
    CALL_GL_API(glClear, mask);
}

static void GL_APIENTRY API_ENTRY(glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    CALL_GL_API(glClearColor, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glClearDepthf)(GLfloat d)
{
    CALL_GL_API(glClearDepthf, d);
}

static void GL_APIENTRY API_ENTRY(glClearStencil)(GLint s)
{
    CALL_GL_API(glClearStencil, s);
}

static void GL_APIENTRY API_ENTRY(glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    CALL_GL_API(glColorMask, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data)
{
    CALL_GL_API(glCompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data)
{
    CALL_GL_API(glCompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    CALL_GL_API(glCopyTexImage2D, target, level, internalformat, x, y, width, height, border);
}

static void GL_APIENTRY API_ENTRY(glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glCopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glCullFace)(GLenum mode)
{
    CALL_GL_API(glCullFace, mode);
}

static void GL_APIENTRY API_ENTRY(glDeleteBuffers)(GLsizei n, const GLuint * buffers)
{
    CALL_GL_API(glDeleteBuffers, n, buffers);
}

static void GL_APIENTRY API_ENTRY(glDeleteTextures)(GLsizei n, const GLuint * textures)
{
    CALL_GL_API(glDeleteTextures, n, textures);
}

static void GL_APIENTRY API_ENTRY(glDepthFunc)(GLenum func)
{
    CALL_GL_API(glDepthFunc, func);
}

static void GL_APIENTRY API_ENTRY(glDepthMask)(GLboolean flag)
{
    CALL_GL_API(glDepthMask, flag);
}

static void GL_APIENTRY API_ENTRY(glDepthRangef)(GLfloat n, GLfloat f)
{
    CALL_GL_API(glDepthRangef, n, f);
}

static void GL_APIENTRY API_ENTRY(glDisable)(GLenum cap)
{
    CALL_GL_API(glDisable, cap);
}

static void GL_APIENTRY API_ENTRY(glDrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    CALL_GL_API(glDrawArrays, mode, first, count);
}

static void GL_APIENTRY API_ENTRY(glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    CALL_GL_API(glDrawElements, mode, count, type, indices);
}

static void GL_APIENTRY API_ENTRY(glEnable)(GLenum cap)
{
    CALL_GL_API(glEnable, cap);
}

static void GL_APIENTRY API_ENTRY(glFinish)(void)
{
    CALL_GL_API(glFinish);
}

static void GL_APIENTRY API_ENTRY(glFlush)(void)
{
    CALL_GL_API(glFlush);
}

static void GL_APIENTRY API_ENTRY(glFrontFace)(GLenum mode)
{
    CALL_GL_API(glFrontFace, mode);
}

static void GL_APIENTRY API_ENTRY(glGenBuffers)(GLsizei n, GLuint * buffers)
{
    CALL_GL_API(glGenBuffers, n, buffers);
}

static void GL_APIENTRY API_ENTRY(glGenTextures)(GLsizei n, GLuint * textures)
{
    CALL_GL_API(glGenTextures, n, textures);
}

static void GL_APIENTRY API_ENTRY(glGetBooleanv)(GLenum pname, GLboolean * data)
{
    CALL_GL_API(glGetBooleanv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetBufferParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_GL_API(glGetBufferParameteriv, target, pname, params);
}

static GLenum GL_APIENTRY API_ENTRY(glGetError)(void)
{
    CALL_GL_API_RETURN(glGetError);
}

static void GL_APIENTRY API_ENTRY(glGetFloatv)(GLenum pname, GLfloat * data)
{
    CALL_GL_API(glGetFloatv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetIntegerv)(GLenum pname, GLint * data)
{
    CALL_GL_API(glGetIntegerv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetPointerv)(GLenum pname, void ** params)
{
    CALL_GL_API(glGetPointerv, pname, params);
}

const GLubyte * GL_APIENTRY API_ENTRY(glGetString)(GLenum name)
{
    CALL_GL_API_RETURN(glGetString, name);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    CALL_GL_API(glGetTexParameterfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_GL_API(glGetTexParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glHint)(GLenum target, GLenum mode)
{
    CALL_GL_API(glHint, target, mode);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsBuffer)(GLuint buffer)
{
    CALL_GL_API_RETURN(glIsBuffer, buffer);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsEnabled)(GLenum cap)
{
    CALL_GL_API_RETURN(glIsEnabled, cap);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsTexture)(GLuint texture)
{
    CALL_GL_API_RETURN(glIsTexture, texture);
}

static void GL_APIENTRY API_ENTRY(glLineWidth)(GLfloat width)
{
    CALL_GL_API(glLineWidth, width);
}

static void GL_APIENTRY API_ENTRY(glPixelStorei)(GLenum pname, GLint param)
{
    CALL_GL_API(glPixelStorei, pname, param);
}

static void GL_APIENTRY API_ENTRY(glPolygonOffset)(GLfloat factor, GLfloat units)
{
    CALL_GL_API(glPolygonOffset, factor, units);
}

static void GL_APIENTRY API_ENTRY(glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)
{
    CALL_GL_API(glReadPixels, x, y, width, height, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glSampleCoverage)(GLfloat value, GLboolean invert)
{
    CALL_GL_API(glSampleCoverage, value, invert);
}

static void GL_APIENTRY API_ENTRY(glScissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glScissor, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glStencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    CALL_GL_API(glStencilFunc, func, ref, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilMask)(GLuint mask)
{
    CALL_GL_API(glStencilMask, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    CALL_GL_API(glStencilOp, fail, zfail, zpass);
}

static void GL_APIENTRY API_ENTRY(glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    CALL_GL_API(glTexImage2D, target, level, internalformat, width, height, border, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glTexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    CALL_GL_API(glTexParameterf, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    CALL_GL_API(glTexParameterfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexParameteri)(GLenum target, GLenum pname, GLint param)
{
    CALL_GL_API(glTexParameteri, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    CALL_GL_API(glTexParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    CALL_GL_API(glTexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glViewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glViewport, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    CALL_GL_API(glEGLImageTargetRenderbufferStorageOES, target, image);
}

static void GL_APIENTRY API_ENTRY(glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    CALL_GL_API(glEGLImageTargetTexture2DOES, target, image);
}

static void GL_APIENTRY API_ENTRY(glGetBufferPointervOES)(GLenum target, GLenum pname, void ** params)
{
    CALL_GL_API(glGetBufferPointervOES, target, pname, params);
}

static void * GL_APIENTRY API_ENTRY(glMapBufferOES)(GLenum target, GLenum access)
{
    CALL_GL_API_RETURN(glMapBufferOES, target, access);
}

static GLboolean GL_APIENTRY API_ENTRY(glUnmapBufferOES)(GLenum target)
{
    CALL_GL_API_RETURN(glUnmapBufferOES, target);
}

static void GL_APIENTRY API_ENTRY(glMultiDrawArraysEXT)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount)
{
    CALL_GL_API(glMultiDrawArraysEXT, mode, first, count, primcount);
}

static void GL_APIENTRY API_ENTRY(glMultiDrawElementsEXT)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei primcount)
{
    CALL_GL_API(glMultiDrawElementsEXT, mode, count, type, indices, primcount);
}

static void GL_APIENTRY API_ENTRY(glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    CALL_GL_API(glDiscardFramebufferEXT, target, numAttachments, attachments);
}

static void GL_APIENTRY API_ENTRY(glRenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    CALL_GL_API(glRenderbufferStorageMultisampleEXT, target, samples, internalformat, width, height);
}

static void GL_APIENTRY API_ENTRY(glFramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    CALL_GL_API(glFramebufferTexture2DMultisampleEXT, target, attachment, textarget, texture, level, samples);
}

static void GL_APIENTRY API_ENTRY(glGetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    CALL_GL_API(glGetProgramBinaryOES, program, bufSize, length, binaryFormat, binary);
}

static void GL_APIENTRY API_ENTRY(glProgramBinaryOES)(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
    CALL_GL_API(glProgramBinaryOES, program, binaryFormat, binary, length);
}

static void GL_APIENTRY API_ENTRY(glTexDirectVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels)
{
    CALL_GL_API(glTexDirectVIV, Target, Width, Height, Format, Pixels);
}

static void GL_APIENTRY API_ENTRY(glTexDirectVIVMap)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectVIVMap, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectMapVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectMapVIV, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectTiledMapVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectTiledMapVIV, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectInvalidateVIV)(GLenum Target)
{
    CALL_GL_API(glTexDirectInvalidateVIV, Target);
}

#undef API_ENTRY
#undef CALL_GL_API
#undef CALL_GL_API_RETURN

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

typedef struct _veglLOOKUP
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglLOOKUP;

#define eglMAKE_LOOKUP(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function }

#define forwardGLFunction(__ret, __api, ...)   \
    {#__api, (__eglMustCastToProperFunctionPointerType) forward_##__api},

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
    /* EGL_EXT_platform_base. */
    eglMAKE_LOOKUP(eglGetPlatformDisplayEXT),
    eglMAKE_LOOKUP(eglCreatePlatformWindowSurfaceEXT),
    eglMAKE_LOOKUP(eglCreatePlatformPixmapSurfaceEXT),
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
    eglMAKE_LOOKUP(eglSetDamageRegionKHR),
    eglMAKE_LOOKUP(eglSwapBuffersWithDamageKHR),
    eglMAKE_LOOKUP(eglSwapBuffersWithDamageEXT),
    eglMAKE_LOOKUP(eglQueryDmaBufFormatsEXT),
    eglMAKE_LOOKUP(eglQueryDmaBufModifiersEXT),
    eglMAKE_LOOKUP(eglPatchID),

    GL_API_ENTRIES(forwardGLFunction)

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

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
    __eglMustCastToProperFunctionPointerType func = gcvNULL;
    VEGLThreadData thread;

    static const char * appendix[] =
    {
#ifdef _EGL_APPENDIX
        gcmDEF2STRING(_EGL_APPENDIX),
#else
        gcvNULL,
#endif

        /* ES11_CL and ES11. */
#ifdef _GL_11_APPENDIX
        gcmDEF2STRING(_GL_11_APPENDIX),
        gcmDEF2STRING(_GL_11_APPENDIX),
#else
        gcvNULL,
        gcvNULL,
#endif

#ifdef _GL_2_APPENDIX
        gcmDEF2STRING(_GL_2_APPENDIX),
#else
        gcvNULL,
#endif

#ifdef _GL_3_APPENDIX
        gcmDEF2STRING(_GL_3_APPENDIX),
#else
        gcvNULL,
#endif

#ifdef _VG_APPENDIX
        gcmDEF2STRING(_VG_APPENDIX),
#else
        gcvNULL,
#endif
    };

    gcmHEADER_ARG("procname=%s", procname);
    VEGL_TRACE_API_PRE(GetProcAddress)(procname);

    do
    {
        veglAPIINDEX index;
        gctHANDLE library;
        veglDISPATCH * dispatch;

        /* Lookup in EGL API. */
        func = _Lookup(_veglLookup, procname, appendix[vegl_EGL]);

        if (func != gcvNULL)
        {
            break;
        }

        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.",
                    __FUNCTION__, __LINE__);

            break;
        }

        /* Go through drivers. */
        for (index = vegl_EGL; index < vegl_API_LAST; index++)
        {
            char rename[128];
            const char *name;

            if (appendix[index])
            {
                gcoOS_StrCopySafe(rename, 128, procname);
                gcoOS_StrCatSafe(rename, 128, appendix[index]);
                name = rename;
            }
            else
            {
                name = procname;
            }

            library  = thread->clientHandles[index];
            dispatch = thread->dispatchTables[index];

            if (dispatch && dispatch->getProcAddr)
            {
                func = dispatch->getProcAddr(name);

                if (func != gcvNULL)
                {
                    break;
                }
            }

            if (library)
            {
                if (gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                                       library,
                                                       name,
                                                       (gctPOINTER *) &func)))
                {
                    break;
                }
            }
        }
    }
    while (gcvFALSE);

    VEGL_TRACE_API_POST(GetProcAddress)(procname, func);
    gcmDUMP_API("${EGL eglGetProcAddress (0x%08X) := 0x%08X",
                procname, func);
    gcmDUMP_API_DATA(procname, 0);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("0x%x", func);
    return func;
}

