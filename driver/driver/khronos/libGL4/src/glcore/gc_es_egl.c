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


#include "EGL/egl.h"
#include "gc_es_context.h"
#include "gc_es_device.h"

__GLdeviceStruct __glDevicePipe;

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
extern GLvoid __glFreeImmedVertexCacheBuffer( __GLcontext *gc );
extern void __glInitImmedNoVertInfoEntries(__GLdispatchTable *dispatch);

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

static void __eglDestroyThreadArea(void *thrArea)
{
    __GLthreadPriv *esThrArea = thrArea;

    /* Destroy 3d blit state */
    if (esThrArea->p3DBlitState)
    {
        esThrArea->destroy3DBlitState(esThrArea->p3DBlitState);
        esThrArea->p3DBlitState = gcvNULL;
    }

    __eglFree(esThrArea);

    return;
}


__GLthreadPriv* __eglGetThreadEsPrivData(void* thrData)
{
    VEGLThreadData eglThreadData = (VEGLThreadData)thrData;
    __GLthreadPriv *esThreadData = gcvNULL;

    /* Return if the esPrivate was already allocated
    */
    if (eglThreadData->esPrivate)
    {
        esThreadData = (__GLthreadPriv *)eglThreadData->esPrivate;
        return esThreadData;
    }

    esThreadData = (__GLthreadPriv*)__eglCalloc(1, sizeof(__GLthreadPriv));
    if (!esThreadData)
    {
        return gcvNULL;
    }

    eglThreadData->esPrivate = esThreadData;
    eglThreadData->destroyESPrivate = __eglDestroyThreadArea;

    return esThreadData;
}


#if defined(_WINDOWS)


#elif (defined(_LINUX_) || defined(__QNXNTO__)) || defined(__APPLE__) || defined(__VXWORKS__)

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
#if defined(OPENGL40) && defined(GL4_DRI_BUILD)
        __glDevicePipe.devDeinitialize(gcvNULL);
#else
        __glDevicePipe.devDeinitialize();
#endif
    }
}

#else
#error "Unsupported compiler version for TLS"
#endif

static void* veglCreateContext_es3(void *thrData, gctINT ClientVersion, VEGLimports *Imports,
                                   gctPOINTER SharedContext, gctINT SharedContextClient)
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
    GLuint i;

    __glSetDrawable(gc, glDrawable, glReadable);


    /*
    ** Application is using immediate mode path.
    ** This function is called by EGL in eglSwapBuffers so sue this to determine cache hit.
    ** will move following code to veglSwapBuffers_es3 after this call is enabled in EGL
    */
    if (gc->input.vtxCacheDrawIndex)
    {
        /*
        ** Stop vertex caching if there is no cache hit at all in 4 consecutive frames.
        */
        if ((gc->input.enableVertexCaching == GL_TRUE) &&
            (gc->input.currentFrameIndex - gc->input.cacheHitFrameIndex) > 3 &&
            !(gc->input.vertexCacheHistory & __GL_IMMED_VERTEX_CACHE_HIT))
        {
             /* Overwrite immediate vertex entries with no-vertex-info entries */
             __glInitImmedNoVertInfoEntries(&gc->immedModeDispatch);
             gc->pModeDispatch = &gc->immedModeOutsideDispatch; /* immediate mode by default. */
             gc->currentImmediateDispatch = gc->pModeDispatch;
             gc->currentImmediateDispatch->Begin = __glim_Begin;

             __glFreeImmedVertexCacheBuffer(gc);
             gc->input.enableVertexCaching = gc->input.origVertexCacheFlag = GL_FALSE;

#if __GL_VERTEX_CACHE_STATISTIC
            fprintf(gc->input.vtxCacheStatFile, "Frame:%3d, Disable vertex cache for continuous cache miss!\n",
                gc->input.currentFrameIndex);
#endif
             /* Force re-validation of DrawArrays/DrawElements dispatch functions */
             gc->currentImmediateDispatch->DrawArrays = __glim_DrawArrays_Validate;
             gc->currentImmediateDispatch->DrawElements = __glim_DrawElements_Validate;
             __GL_SET_VARRAY_STOP_CACHE_BIT(gc);
        }
    }
    else
    {
        gc->input.cacheHitFrameIndex = gc->input.currentFrameIndex;

        /*
        ** Free vertex cache buffers if the application stop using immediate mode path.
        */
        if (gc->input.totalCacheMemSize > 0) {
            __glFreeImmedVertexCacheBlocks(gc);
        }
    }

#if __GL_VERTEX_CACHE_STATISTIC
    if (gc->input.canCacheDraws != 0)
    {
        fprintf(gc->input.vtxCacheStatFile, "Frame:%3d, canCacheDraws:%5d  cacheHitDraw:%5d  cacheMissDraws:%5d  disabledDraws:%5d  cacheHitRatio:%3d%%;\n\n",
            gc->input.currentFrameIndex, gc->input.canCacheDraws, gc->input.cacheHitDraws, gc->input.cacheMissDraws, gc->input.disabledDraws,
            100*gc->input.cacheHitDraws/gc->input.canCacheDraws);
    }
    else
    {
        fprintf(gc->input.vtxCacheStatFile, "Frame:%3d, all the draws of this frame are too samll to do cache!\n\n", gc->input.currentFrameIndex);
    }

    gc->input.canCacheDraws = 0;
    gc->input.cacheHitDraws = 0;
    gc->input.cacheMissDraws = 0;
    gc->input.disabledDraws = 0;
#endif

    /*
    ** Reset vtxCacheDrawIndex so we can compare drawPrimitives
    ** in this frame with the previous frame's drawPrimitives.
    */
    gc->input.vertexCacheHistory |= gc->input.vertexCacheStatus;
    gc->input.vertexCacheStatus = 0;
    gc->input.vtxCacheDrawIndex = 0;

    /* Reset currentDrawIndex and increment currentFrameIndex */
    gc->input.currentDrawIndex = 0;
    gc->input.currentFrameIndex += 1;

    /* Handle gc->input.currentFrameIndex UINT wrap around case */
    if ((gc->input.currentFrameIndex ^ 0xffffffff) == 0)
    {
        __GLvertexCacheBlock *cacheBlock = gc->input.vertexCacheBlock;
        while (cacheBlock)
        {
            for (i = 0; i < __GL_VERTEX_CACHE_BLOCK_SIZE; i++) {
                cacheBlock->cache[i].frameIndex = 0;
            }
            cacheBlock = cacheBlock->next;
        }
        gc->input.cacheHitFrameIndex = gc->input.currentFrameIndex = 1;
    }

    /* Reset currentCacheBlock and currentVertexCache pointers */
    gc->input.currentCacheBlock = gc->input.vertexCacheBlock;
    gc->input.currentVertexCache = &gc->input.vertexCacheBlock->cache[0];

    /* Reset immediate mode vertex buffer */
    __glResetImmedVertexBuffer(gc, GL_FALSE);


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

static void veglFlush_es3(void)
{
    __GLcontext *gc = __glGetGLcontext();

    if (gcvNULL == gc)
    {
        gcmFATAL("Get context failed");
        return;
    }

    gc->immedModeDispatch.Flush(gc);
}

static void veglFinish_es3(void)
{
    __GLcontext *gc = __glGetGLcontext();

    if (gcvNULL == gc)
    {
        gcmFATAL("Get context failed");
        return;
    }

    gc->immedModeDispatch.Finish(gc);
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

static EGLenum
veglBindTexImage_es3(
    void * Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    void ** Binder
    )
{
    EGLenum error = EGL_BAD_ACCESS;

    do
    {
        GLenum glFormat = GL_NONE;
        __GLcontext *gc = __glGetGLcontext();

        if (gcvNULL == gc)
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

/* OpenGL Dispatch table. */
veglDISPATCH GL_DISPATCH_TABLE =
{
    /* createContext            */  veglCreateContext_es3,
    /* destroyContext           */  veglDestroyContext_es3,
    /* makeCurrent              */  veglMakeCurrent_es3,
    /* loseCurrent              */  veglLoseCurrent_es3,
    /* setDrawable              */  veglSetDrawable_es3,
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

/* OpenGL ES Dispatch table for ES Compatibility */
veglDISPATCH GLESv2_DISPATCH_TABLE =
{
    /* createContext            */  veglCreateContext_es3,
    /* destroyContext           */  veglDestroyContext_es3,
    /* makeCurrent              */  veglMakeCurrent_es3,
    /* loseCurrent              */  veglLoseCurrent_es3,
    /* setDrawable              */  veglSetDrawable_es3,
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
