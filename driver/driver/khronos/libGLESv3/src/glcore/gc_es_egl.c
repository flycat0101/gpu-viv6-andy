/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "EGL/egl.h"
#include "gc_es_context.h"
#include "gc_es_device.h"


__GLdeviceStruct __glDevicePipe;

extern __GLesDispatchTable __glesApiFuncDispatchTable;

extern GLint __glGetDispatchOffset(const char *procName);
extern __GLprocAddr __glGetProcAddr(const GLchar *procName);

extern GLvoid* __glCreateContext(GLint, VEGLimports*, GLvoid*);
extern GLuint __glDestroyContext(GLvoid *gc);
extern __GLdrawablePrivate* __glGetDrawable(VEGLDrawable eglDrawable);
extern GLvoid __glSetDrawable(__GLcontext* gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
extern GLboolean __glMakeCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable, GLboolean flushDrawableChange);
extern GLboolean __glLoseCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
extern EGLenum __glCreateEglImageTexture(__GLcontext* gc, EGLenum target, GLint texture, GLint level, GLint depth, GLvoid* image);
extern EGLenum __glCreateEglImageRenderbuffer(__GLcontext* gc, GLuint renderbuffer, GLvoid* image);

extern GLboolean
__glBindTexImage(
    __GLcontext *gc,
    GLenum format,
    GLboolean mipmap,
    GLint level,
    GLint width,
    GLint height,
    void * surface,
    void ** pBinder
    );


GLvoid *__eglMalloc(size_t size)
{
    GLvoid *data = gcvNULL;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, (gctSIZE_T)size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
    }
    return data;
}

GLvoid *__eglCalloc(size_t numElements, size_t elementSize)
{
    GLvoid *data = gcvNULL;
    gctSIZE_T size = (gctSIZE_T)(numElements * elementSize);
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
        return data;
    }
    gcoOS_ZeroMemory(data, size);
    return data;
}

GLvoid __eglFree(GLvoid *ptr)
{
    gcoOS_Free(gcvNULL, ptr);
}



#if defined(_WINDOWS)

BOOL WINAPI attachProcess(HINSTANCE hInst)
{
    return TRUE;
}

BOOL WINAPI detachProcess(HINSTANCE hInst)
{
    /* Perform any device-dependent de-initialization */
    if (__glDevicePipe.devDeinitialize)
    {
        __glDevicePipe.devDeinitialize();
    }

    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
          return attachProcess(hInst);

      case DLL_PROCESS_DETACH:
          {
              gcsDRIVER_TLS_PTR tls = gcvNULL;
              gcoOS_GetDriverTLS(gcvTLS_KEY_EGL, &tls);
              if (tls)
              {
                  VEGLThreadData thread = (VEGLThreadData)tls;
                  thread->destroyESPrivate = gcvNULL;
              }
          }
          return detachProcess(hInst);

      case DLL_THREAD_DETACH:
          {
              gcsDRIVER_TLS_PTR tls = gcvNULL;
              gcoOS_GetDriverTLS(gcvTLS_KEY_EGL, &tls);
              if (tls)
              {
                  VEGLThreadData thread = (VEGLThreadData)tls;
                  thread->destroyESPrivate = gcvNULL;
              }
          }
          break;
      default:
          break;
    }
    return TRUE;
}

#elif (defined(_LINUX_) || defined(__QNXNTO__)) || defined(__APPLE__)

static void __attribute__((constructor)) __eglConstruct(void);
static void __attribute__((destructor)) __eglDestruct(void);

void __eglConstruct(void)
{
}

void __eglDestruct(void)
{
    /* Perform any device-dependent de-initialization */
    /* Send OPENGL_DEINITIALIZATION escape call */
    if (__glDevicePipe.devDeinitialize)
    {
        __glDevicePipe.devDeinitialize();
    }
}

#else
#error "Unsupported compiler version for TLS"
#endif

static void* veglCreateContext_es3(void *thrData, gctINT ClientVersion, VEGLimports *Imports, gctPOINTER SharedContext)
{
    __GLcontext *gc = __glCreateContext((GLint)ClientVersion, Imports, SharedContext);

    return gc;
}

static EGLBoolean veglDestroyContext_es3(void *thrData, GLvoid *pCtxPriv)
{
    __GLcontext *gc = (__GLcontext*)pCtxPriv;

    if (gc)
    {
        if (__glGetGLcontext() == gc)
        {
            /* Cache ES3 context's commitState if the context is current */
            if (__glLoseCurrent(gc, gcvNULL, gcvNULL))
            {
                __glSetGLcontext(gcvNULL);
            }
            else
            {
                return EGL_FALSE;
            }
        }

        __glDestroyContext(gc);
    }

    return EGL_TRUE;
}

static EGLBoolean veglSetDrawable_es3(void *thrData, void *pCtxPriv, VEGLDrawable drawable, VEGLDrawable readable)
{
    __GLcontext* gc = (__GLcontext*)pCtxPriv;
    __GLdrawablePrivate* glDrawable = __glGetDrawable(drawable);
    __GLdrawablePrivate* glReadable = __glGetDrawable(readable);

    __glSetDrawable(gc, glDrawable, glReadable);

    return EGL_TRUE;
}

static EGLBoolean veglMakeCurrent_es3(void *thrData, void *pCtxPriv, VEGLDrawable drawable, VEGLDrawable readable)
{
    __GLcontext *gc = (__GLcontext*)pCtxPriv;
    __GLdrawablePrivate* glDrawable = __glGetDrawable(drawable);
    __GLdrawablePrivate* glReadable = __glGetDrawable(readable);

    /* MakeCurrent for the new GL context, null drawables/readables means GL_OES_surfaceless_context. */
    if (!__glMakeCurrent(gc, glDrawable, glReadable, GL_FALSE))
    {
        return EGL_FALSE;
    }

    __glSetGLcontext(gc);
    return EGL_TRUE;
}

static EGLBoolean veglLoseCurrent_es3(void *thrData, void *pCtxPriv)
{
    __GLcontext *gc = (__GLcontext*)pCtxPriv;

    /* LoseCurrent for the current GL context. */
    if (!__glLoseCurrent(gc, gcvNULL, gcvNULL))
    {
        return EGL_FALSE;
    }

    __glSetGLcontext(gcvNULL);
    return EGL_TRUE;
}


static EGLBoolean veglFlushContext_es3(void * Context)
{
    return EGL_TRUE;
}

static void veglFlush_es3(void)
{
    __GLcontext *gc = __glGetGLcontext();
    gctBOOL temp = gc->profiler.useGlfinish;
    gc->profiler.useGlfinish = gcvFALSE;

    if (gcvNULL == gc)
    {
        gcmFATAL("Get context failed");
        return;
    }

    __gles_Flush(gc);

    gc->profiler.useGlfinish = temp;
}

static void veglFinish_es3(void)
{
    __GLcontext *gc = __glGetGLcontext();
    gctBOOL temp = gc->profiler.useGlfinish;
    gc->profiler.useGlfinish = gcvFALSE;

    if (gcvNULL == gc)
    {
        gcmFATAL("Get context failed");
        return;
    }

    __gles_Finish(gc);

    gc->profiler.useGlfinish = temp;
}

static EGLenum veglCreateImageTexture_es3(void * Context,
                                          EGLenum target,
                                          gctINT texture,
                                          gctINT level,
                                          gctINT depth,
                                          gctPOINTER image)
{
    EGLenum result;
    __GLcontext* gc = (__GLcontext*)Context;
    /* Create image from default texture isn't allowed. */
    if (texture == 0)
    {
        result = EGL_BAD_PARAMETER;
    }
    else
    {
        result = __glCreateEglImageTexture(gc, target, texture, level, depth, image);
    }

    return result;
}

static EGLenum
veglCreateImageRenderbuffer_es3(void * Context, gctUINT renderbuffer,
                                gctPOINTER image)
{
    EGLenum result;
    __GLcontext* gc = (__GLcontext*)Context;

    if (renderbuffer == 0)
    {
        result = EGL_BAD_PARAMETER;
    }
    else
    {
        result = __glCreateEglImageRenderbuffer(gc, renderbuffer, image);
    }

    return result;
}


static EGLBoolean veglSwapBuffers_es3(EGLDisplay Dpy,
                                     EGLSurface Draw,
                                     gcfEGL_DO_SWAPBUFFER Callback)
{
    gcmASSERT(Callback);
    (*Callback)(Dpy, Draw);

    return EGL_TRUE;
}

static EGLenum veglBindTexImage_es3(void * Surface,
                                    EGLenum Format,
                                    EGLBoolean Mipmap,
                                    EGLint Level,
                                    EGLint Width,
                                    EGLint Height,
                                    void ** Binder)
{
    EGLenum error = EGL_BAD_ACCESS;

    do
    {
        GLenum glFormat = GL_NONE;
        __GLcontext *gc = __glGetGLcontext();

        if(gcvNULL == gc)
        {
            error = EGL_BAD_CONTEXT;
            break;
        }

        if (Format == EGL_TEXTURE_RGB)
        {
            glFormat = GL_RGB;
        }
        else if  (Format == EGL_TEXTURE_RGBA)
        {
            glFormat = GL_RGBA;
        }
        else
        {
            error = EGL_BAD_PARAMETER;
            break;
        }

        if (__glBindTexImage(gc, glFormat, (Mipmap ? GL_TRUE : GL_FALSE),
                             (GLint)Level, Width, Height, Surface, Binder))
        {
            error = EGL_SUCCESS;
        }
    } while (0);

    return error;
}

static gctBOOL veglProfiler_es3(void * Profiler, gctUINT32 Enum, gctHANDLE Value)
{
#if VIVANTE_PROFILER
    return __glProfiler(Profiler, Enum, Value);
#else
    return gcvFALSE;
#endif
}

static EGL_PROC veglGetProcAddr_es3(const char *procname)
{
    return (EGL_PROC)__glGetProcAddr(procname);
}


/* Dispatch table. */
veglDISPATCH GLESv2_DISPATCH_TABLE =
{
    /* createContext            */  veglCreateContext_es3,
    /* destroyContext           */  veglDestroyContext_es3,
    /* makeCurrent              */  veglMakeCurrent_es3,
    /* loseCurrent              */  veglLoseCurrent_es3,
    /* setDrawable              */  veglSetDrawable_es3,
    /* flushContext             */  veglFlushContext_es3,
    /* flush                    */  veglFlush_es3,
    /* finish                   */  veglFinish_es3,
    /* getClientBuffer          */  gcvNULL,
    /* createImageTexture       */  veglCreateImageTexture_es3,
    /* createImageRenderbuffer  */  veglCreateImageRenderbuffer_es3,
    /* createImageParentImage   */  gcvNULL,
    /* bindTexImage             */  veglBindTexImage_es3,
    /* profiler                 */  veglProfiler_es3,
    /* getProcAddr              */  veglGetProcAddr_es3,
    /* swapBuffers              */  veglSwapBuffers_es3,
    /* queryHWVG                */  gcvNULL,
    /* resolveVG                */  gcvNULL,
};
