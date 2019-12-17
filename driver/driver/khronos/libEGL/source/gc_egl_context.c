/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_egl_precomp.h"

#if defined(ANDROID) && !defined(GPU_VENDOR)
#  define GPU_VENDOR "VIVANTE"
#endif

#if defined(ANDROID)
/* Android wrapper expect EGL 1.4 */
#  define EGL_MINOR_VERSION 4
#else
#  define EGL_MINOR_VERSION 5
#endif

#define _GC_OBJ_ZONE    gcdZONE_EGL_CONTEXT

#if !gcdSTATIC_LINK

static const char * _driverDlls[] =
{
#if defined(ANDROID)
    "libEGL_" GPU_VENDOR ".so",         /* EGL */
    "libGLESv1_CL_" GPU_VENDOR ".so",   /* OpenGL ES 1.1 Common Lite */
    "libGLESv1_CM_" GPU_VENDOR ".so",   /* OpenGL ES 1.1 Common */
    "libGLESv2_" GPU_VENDOR ".so",      /* OpenGL ES 2.0/3.x */
    "libGL.so",                         /* OpenGL */
    "libOpenVG.so",                     /* OpenVG 1.0 */
#elif defined(__QNXNTO__)
    "egl-dlls",                         /* EGL */
    "glesv1-dlls",                      /* OpenGL ES 1.1 Common Lite */
    "glesv1-dlls",                      /* OpenGL ES 1.1 Common */
    "glesv2-dlls",                      /* OpenGL ES 2.0/3.x */
    gcvNULL,                            /* OpenGL */
    "vg-dlls",                          /* OpenVG 1.0 */
#elif defined(__APPLE__)
    "libEGL.dylib",                     /* EGL */
    "libGLESv1_CL.dylib",               /* OpenGL ES 1.1 Common Lite */
    "libGLESv1_CM.dylib",               /* OpenGL ES 1.1 Common */
    "libGLESv2.dylib",                  /* OpenGL ES 2.0/3.x */
    "libOpenGL.dylib",                  /* OpenGL */
    "libOpenVG.dylib",                  /* OpenVG 1.0 */
#else
    "libEGL",                           /* EGL */
    "libGLESv1_CL",                     /* OpenGL ES 1.1 Common Lite */
    "libGLESv1_CM",                     /* OpenGL ES 1.1 Common */
    "libGLESv2",                        /* OpenGL ES 2.0/3.x */
    "libGL",                            /* OpenGL */
    "libOpenVG",                        /* OpenVG 1.0 */
#endif
};

#if !defined(__linux__) && !defined(__ANDROID__) && !defined(__QNX__)
static const char * _dispatchNames[] =
{
    "",                                 /* EGL */
    "GLES_CL_DISPATCH_TABLE",           /* OpenGL ES 1.1 Common Lite */
    "GLES_CM_DISPATCH_TABLE",           /* OpenGL ES 1.1 Common */
    "GLESv2_DISPATCH_TABLE",            /* OpenGL ES 2.0 */
    "GL_DISPATCH_TABLE",                /* OpenGL 4.x */
    "OpenVG_DISPATCH_TABLE",            /* OpenVG 1.0 */
};
#endif

gctHANDLE
veglGetModule(
    IN gcoOS Os,
    IN veglAPIINDEX Index,
    IN gctCONST_STRING Name,
    IN veglDISPATCH **Dispatch
    )
{
    gctHANDLE library = gcvNULL;
    gctSTRING libEnvStr = gcvNULL;

    if (Index < vegl_API_LAST)
    {
#if defined(ANDROID)
        gctCHAR path[64];
        gctCONST_STRING subdir;
        gctUINT offset;

#ifdef __LP64__
#    define LIBRARY_PATH1 "/vendor/lib64/"
#    define LIBRARY_PATH2 "/system/lib64/"
#  else
#    define LIBRARY_PATH1 "/vendor/lib/"
#    define LIBRARY_PATH2 "/system/lib/"
#  endif
        /*
         * Put OpenVG library to /system/lib or /vendor/lib because there's no
         * OpenVG wrapper library in android framework.
         */
        subdir = (Index == vegl_OPENVG) ? "" : "egl/";

        /* Try load library. */
        offset = 0;
        gcoOS_PrintStrSafe(path, 64, &offset, "%s%s%s", LIBRARY_PATH1, subdir, _driverDlls[Index]);
        gcoOS_LoadLibrary(Os, path, &library);

        /* Try 2nd path for Android */
        if (!library)
        {
            /* Try load library in 2d path. */
            offset = 0;
            gcoOS_PrintStrSafe(path, 64, &offset, "%s%s%s", LIBRARY_PATH2, subdir, _driverDlls[Index]);
            gcoOS_LoadLibrary(Os, path, &library);
        }
#else
        if (Index == vegl_OPENGL_ES20)
        {
            if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_GL_FOR_GLES", &libEnvStr)) &&
                libEnvStr && gcmIS_SUCCESS(gcoOS_StrCmp(libEnvStr, "1")))
            {
                gcoOS_Print("Use OpenGL library libGL.so.x for GLES application!\n");
                gcoOS_LoadLibrary(Os, _driverDlls[vegl_OPENGL], &library);
            }
            else
            {
                gcoOS_LoadLibrary(Os, _driverDlls[vegl_OPENGL_ES20], &library);
            }
        }
        else
        {
            gcoOS_LoadLibrary(Os, _driverDlls[Index], &library);
        }

        /* Query the CL handle if CM not available. */
        if (!library && Index == vegl_OPENGL_ES11)
        {
            Index = vegl_OPENGL_ES11_CL;
            gcoOS_LoadLibrary(Os, _driverDlls[vegl_OPENGL_ES11_CL], &library);
        }
#endif

        if (Dispatch && library)
        {
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
            gcoOS_GetProcAddress(Os, library, Name, (gctPOINTER*)Dispatch);
#else
            gcoOS_GetProcAddress(Os, library, _dispatchNames[Index], (gctPOINTER*)Dispatch);
#endif
        }
    }

    /* Return result. */
    return library;
}
#endif /* gcdSTATIC_LINK */

static veglAPIINDEX
_GetAPIIndex(
    VEGLContext Context
    )
{
    veglAPIINDEX index = vegl_API_LAST;

    do
    {
        VEGLThreadData thread;
        VEGLContext context;
        EGLenum api;
        EGLint major;

        /* Get thread data. */
        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(
                gcvLEVEL_ERROR,
                "%s(%d): veglGetThreadData failed.",
                __FUNCTION__, __LINE__
                );

            break;
        }

        /* Get current context. */
        context = (Context != gcvNULL) ? Context
                : (thread->api == EGL_NONE)
                ? gcvNULL
                : thread->context;

        /* Is there a current context? */
        if (context == gcvNULL)
        {
            /* No current context, use thread data. API default is ES1 */
            api    = thread->api;
            major = 1;
        }
        else
        {
            /* Have current context set. */
            api    = context->api;
            major = MAJOR_API_VER(context->client);
        }

        /* Dispatch base on the API. */
        switch (api)
        {
        case EGL_OPENGL_ES_API:
            index = (major == 1)? vegl_OPENGL_ES11 : vegl_OPENGL_ES20;
            break;

        case EGL_OPENVG_API:
            index = vegl_OPENVG;
            break;

        case EGL_OPENGL_API:
            index = vegl_OPENGL;
            break;
        }
    }
    while (EGL_FALSE);

    /* Return result. */
    return index;
}


veglDISPATCH *
_GetDispatch(
    VEGLThreadData Thread,
    VEGLContext Context
    )
{
    struct eglContext context;
    veglAPIINDEX index;

    if (Thread == gcvNULL)
    {
        return gcvNULL;
    }

    if (Context == gcvNULL)
    {
        Context = Thread->context;

        if (Context == gcvNULL)
        {
            /* Use default context. */
            context.thread        = Thread;
            context.api           = Thread->api;
            context.client        = 1;
            context.display       = EGL_NO_DISPLAY;
            context.sharedContext = EGL_NO_CONTEXT;
            context.draw          = EGL_NO_SURFACE;
            context.read          = EGL_NO_SURFACE;

            Context = &context;
        }
    }

    index = _GetAPIIndex(Context);

    if (index >= vegl_API_LAST)
    {
        return gcvNULL;
    }

    /* Return dispatch table. */
    return Thread->dispatchTables[index];
}


/*
** Imported function for client API driver use
*/
static void *
_GetCurrentAPIContext(
    EGLenum Api
    );

static void
_SetCurrentAPIContext(
    EGLenum Api,
    void* context
    );

static void
_SyncNative(
    void
    );

static void
_ReferenceImage(
    khrEGL_IMAGE * Image
    )
{
    VEGLImage image;
    VEGLThreadData thread;

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
        return;
    }

    /* Cast EGL image. */
    image = (VEGLImage) Image;

    /* Call reference func. */
    veglReferenceImage(thread, image->display, image);
}

static void
_DereferenceImage(
    khrEGL_IMAGE * Image
    )
{
    VEGLImage image;
    VEGLThreadData thread;

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
        return;
    }

    /* Cast EGL image. */
    image = (VEGLImage) Image;

    /* Call dereference func. */
    veglDereferenceImage(thread, image->display, image);
}

static gctPOINTER
_Malloc(
    void *ctx,
    gctSIZE_T size
    )
{
    gctPOINTER data = gcvNULL;

    if (0x0 == size)
    {
        return gcvNULL;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
    }

    gcoOS_ZeroMemory(data, size);
    return data;
}

static gctPOINTER
_Calloc(
    void *ctx,
    gctSIZE_T numElements,
    gctSIZE_T elementSize
    )
{
    gctPOINTER data = gcvNULL;
    gctSIZE_T  size = numElements * elementSize;
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, size, &data)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
        return data;
    }
    gcoOS_ZeroMemory(data, size);
    return data;
}

static gctPOINTER
_Realloc(
    void *ctx,
    gctPOINTER oldPtr,
    gctSIZE_T newSize
    )
{
    gctSIZE_T  oldSize = 0;
    gctPOINTER newPtr  = gcvNULL;

    gcoOS_GetMemorySize(gcvNULL, oldPtr, &oldSize);

    if (newSize <= oldSize)
    {
        if (0 == newSize)
        {
            gcoOS_Free(gcvNULL, oldPtr);
            return gcvNULL;
        }
        return oldPtr;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, newSize, &newPtr)))
    {
        return gcvNULL;
    }

    if (oldPtr)
    {
        gcoOS_MemCopy(newPtr, oldPtr, oldSize);
        gcoOS_Free(gcvNULL, oldPtr);
    }

    return newPtr;
}

static void
_Free(
    void *ctx,
    gctPOINTER ptr
    )
{
    gcoOS_Free(gcvNULL, ptr);
}

static void
_CreateUserMutex(
    VEGLLock *mp
    )
{
    mp->usageCount++;
    if (mp->usageCount == 1)
    {
        gcoOS_CreateMutex(gcvNULL, &mp->lock);
    }
}

static void
_DestroyUserMutex(
    VEGLLock *mp
    )
{
    mp->usageCount--;
    if (mp->usageCount == 0)
    {
        gcoOS_DeleteMutex(gcvNULL, mp->lock);
    }
}

static void
_LockUserMutex(
    VEGLLock *mp
    )
{
    gcoOS_AcquireMutex(gcvNULL, mp->lock, gcvINFINITE);
}

static void
_UnlockUserMutex(
    VEGLLock *mp
    )
{
    gcoOS_ReleaseMutex(gcvNULL, mp->lock);
}

void *
_CreateApiContext(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLConfig     Config,
    void         * SharedContext,
    EGLint         SharedContextClient
    )
{
    VEGLimports imports =
    {
        _GetCurrentAPIContext,   /* getCurContext */
        _SetCurrentAPIContext,   /* setCurContext */
        _SyncNative,             /* syncNative */

        _ReferenceImage,         /* referenceImage */
        _DereferenceImage,       /* dereferenceImage */

        _Malloc,                 /* malloc */
        _Calloc,                 /* calloc */
        _Realloc,                /* realloc */
        _Free,                   /* free */

        _CreateUserMutex,        /* createMutex */
        _DestroyUserMutex,       /* destroyMutex */
        _LockUserMutex,          /* lockMutex */
        _UnlockUserMutex,        /* unlockMutex */

        gcvNULL,                 /* config */
        gcvFALSE,                /* robustAccess */
        0,                       /* resetNotification */
        gcvFALSE,                /* debuggable */
        0,                       /* contextFlags.*/

        gcvFALSE,                /* protected content. */

#if gcdGC355_PROFILER
        Context->appStartTime,   /* appStartTime */
        Context->apiTimeFile,    /* appTileFile */
#endif
    };

    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, Context, SharedContext, Config);

    if ((dispatch == gcvNULL) || (dispatch->createContext == gcvNULL))
    {
        return gcvNULL;
    }

    if (!_IsExtSuppored(VEGL_EXTID_KHR_no_config_context))
    {
        gcmASSERT(Config);
    }

    imports.config = Config;
    imports.robustAccess = Context->robustAccess;
    imports.resetNotification = Context->resetNotification;
    imports.contextFlags = Context->flags;
    imports.debuggable = (Context->flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR) ? gcvTRUE : gcvFALSE;
    imports.version = Context->client;
    imports.conformGLSpec = (Context->api == EGL_OPENGL_API) ? gcvTRUE : gcvFALSE;
    imports.coreProfile = Context->coreProfile;
    imports.fromEGL = gcvTRUE;

    /*
     * EGL_EXT_protected_context.
     * Notice, should still behave as protected context if this flag is 'false',
     * because EGL_EXT_protected_surface is supported.
     * SPEC:
     * ... However, if EGL_EXT_protected_surface is also supported, a regular
     * (not protected) context will execute stages where one or more protected
     * resources is accessed as if it were a protected context.
     */
    imports.protectedContext = Context->protectedContent;

    return (*dispatch->createContext)(Thread, Context->client, &imports, SharedContext, SharedContextClient);
}


EGLBoolean
_DestroyApiContext(
    VEGLThreadData Thread,
    VEGLContext Context,
    void * ApiContext
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext);

    if ((dispatch == gcvNULL)
    ||  (dispatch->destroyContext == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    return (*dispatch->destroyContext)(Thread, ApiContext);
}

static EGLBoolean
_ApiMakeCurrent(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLDrawable   Draw,
    VEGLDrawable   Read
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext, Draw, Read);

    if ((dispatch == gcvNULL) || (dispatch->makeCurrent == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->makeCurrent)(Thread, ApiContext, Draw, Read);
}

static EGLBoolean
_ApiLoseCurrent(
    VEGLThreadData Thread,
    VEGLContext    Context
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x", __FUNCTION__, __LINE__,
                  Thread, ApiContext);

    if ((dispatch == gcvNULL) || (dispatch->loseCurrent == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->loseCurrent)(Thread, ApiContext);
}

EGLBoolean
_SetDrawable(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLDrawable   Draw,
    VEGLDrawable   Read
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, ApiContext, Draw, Read);

    if ((dispatch == gcvNULL) || (dispatch->setDrawable == gcvNULL))
    {
        return (ApiContext == gcvNULL) ? EGL_TRUE : EGL_FALSE;
    }

    return (*dispatch->setDrawable)(Thread, ApiContext, Draw, Read);
}

EGLBoolean
_Flush(
    VEGLThreadData Thread
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x",
                  __FUNCTION__, __LINE__,
                  Thread);

    if ((dispatch == gcvNULL)
    ||  (dispatch->flush == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    if (Thread->context != gcvNULL)
    {
        (*dispatch->flush)();
    }

    return EGL_TRUE;
}

EGLBoolean
_Finish(
    VEGLThreadData Thread
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x",
                  __FUNCTION__, __LINE__,
                  Thread);

    if ((dispatch == gcvNULL)
    ||  (dispatch->finish == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    if (Thread->context != gcvNULL)
    {
        (*dispatch->finish)();
    }

    return EGL_TRUE;
}

gcoSURF
_GetClientBuffer(
    VEGLThreadData Thread,
    void * Context,
    EGLClientBuffer Buffer
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,0x%x,0x%x",
                  __FUNCTION__, __LINE__,
                  Thread, Context, Buffer);

    if ((dispatch == gcvNULL)
    ||  (dispatch->getClientBuffer == gcvNULL)
    )
    {
        return gcvNULL;
    }

    return (*dispatch->getClientBuffer)(Context, Buffer);
}

EGLBoolean
_ProfilerCallback(
    VEGLThreadData Thread,
    gctUINT32 Enum,
    gctHANDLE Value
    )
{
#if VIVANTE_PROFILER
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): 0x%x,%u,%lu",
                  __FUNCTION__, __LINE__,
                  Thread, Enum, Value);

    if ((dispatch == gcvNULL)
    ||  (dispatch->profiler == gcvNULL)
    )
    {
        return EGL_FALSE;
    }

    return (*dispatch->profiler)(gcvNULL, Enum, Value);
#else

    return EGL_TRUE;
#endif
}


EGLBoolean
veglBindAPI(
    VEGLThreadData thread,
    EGLenum api
    )
{
    gcmHEADER_ARG("api=0x%04x", api);

    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        /* Fatal error. */
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    switch (api)
    {
    case EGL_OPENGL_ES_API:
        if (!thread->dispatchTables[vegl_OPENGL_ES11_CL] &&
            !thread->dispatchTables[vegl_OPENGL_ES11] &&
            !thread->dispatchTables[vegl_OPENGL_ES20])
        {
            /* OpenGL ES API not supported. */
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
            gcmFOOTER_ARG("%d", EGL_FALSE);
            return EGL_FALSE;
        }

        /* Bind OpenGL ES API. */
        thread->api = api;
        thread->context = thread->esContext;

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        break;

    case EGL_OPENGL_API:
        if (!thread->dispatchTables[vegl_OPENGL])
        {
            /* OpenGL API not supported. */
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
            gcmFOOTER_ARG("%d", EGL_FALSE);
            return EGL_FALSE;
        }

        /* Bind OpenGL API. */
        thread->api = api;
        thread->context = thread->glContext;

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        break;

    case EGL_OPENVG_API:
        if (!thread->dispatchTables[vegl_OPENVG])
        {
            /* OpenVG API not supported. */
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
            gcmFOOTER_ARG("%d", EGL_FALSE);
            return EGL_FALSE;
        }

        /* Bind OpenVG API. */
        thread->api = api;
        thread->context = thread->vgContext;

#if gcdENABLE_VG
        if (thread->openVGpipe)
        {
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG));
        }
        else
        {
            gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
        }
#endif
        break;

    default:
        /* Bad parameter. */
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglBindAPI(
    EGLenum api
    )
{
    EGLenum ret;
    VEGLThreadData thread;

    VEGL_TRACE_API(BindAPI)(api);

    gcmDUMP_API("${EGL eglBindAPI 0x%08X}", api);

    /* Get thread data. */
    thread = veglGetThreadData();

    ret = veglBindAPI(thread, api);

    return ret;
}

EGLenum
veglQueryAPI(
    void
    )
{
    VEGLThreadData thread;

    gcmHEADER();

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%04x", EGL_NONE);
        return EGL_NONE;
    }

    /* Return current API. */
    gcmFOOTER_ARG("%04x", thread->api);
    return thread->api;
}

EGLAPI EGLenum EGLAPIENTRY
eglQueryAPI(
    void
    )
{
    EGLenum api;

    VEGL_TRACE_API_PRE(QueryAPI)();
    api = veglQueryAPI();
    VEGL_TRACE_API_POST(QueryAPI)(api);

    gcmDUMP_API("${EGL eglQueryAPI := 0x%08X}", api);

    return api;
}

EGLAPI EGLContext EGLAPIENTRY
eglCreateContext(
    EGLDisplay Dpy,
    EGLConfig config,
    EGLContext SharedContext,
    const EGLint *attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLContext sharedContext;
    gceSTATUS status;
    VEGLContext context;
    EGLint major = 1;
    EGLint minor = 0;
    gctPOINTER pointer = gcvNULL;
    VEGLConfig eglConfig = gcvNULL;
    EGLint flags = 0;
    gctBOOL robustAccess = EGL_FALSE;
    gctINT resetNotification = EGL_NO_RESET_NOTIFICATION_EXT;
    gctBOOL robustAttribSet = gcvFALSE;
    EGLBoolean protectedContent = gcvFALSE;

    gcmHEADER_ARG("Dpy=0x%x config=0x%x SharedContext=0x%x attrib_list=0x%x",
                  Dpy, config, SharedContext, attrib_list);

    VEGL_TRACE_API_PRE(CreateContext)(Dpy, config, SharedContext, attrib_list);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);

        return EGL_NO_CONTEXT;
    }

    /* Test for valid bounded API. */
    if (thread->api == EGL_NONE)
    {
        /* Bad match. */
        veglSetEGLerror(thread, EGL_BAD_MATCH);

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    /* Create shortcuts to objects. */
    sharedContext = VEGL_CONTEXT(SharedContext);

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
        return EGL_NO_CONTEXT;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    if (config != EGL_NO_CONFIG_KHR)
    {
        /* Test for valid config. */
        if (((EGLint)(intptr_t)config <= __EGL_INVALID_CONFIG__)
        ||  ((EGLint)(intptr_t)config > dpy->configCount)
        )
        {
            veglSetEGLerror(thread,  EGL_BAD_CONFIG);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        eglConfig = VEGL_CONFIG(&dpy->config[(EGLint)(intptr_t)config - 1]);
    }
    else
    {
        if (!_IsExtSuppored(VEGL_EXTID_KHR_no_config_context))
        {
            veglSetEGLerror(thread,  EGL_BAD_CONFIG);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Get attribute. */
    if (attrib_list != gcvNULL)
    {
        EGLint i = 0;
        EGLBoolean needCheckRobustAndVersion = EGL_FALSE;
        EGLBoolean setRobustAccess = EGL_FALSE;

        for (i = 0; attrib_list[i] != EGL_NONE; i += 2)
        {
            switch (attrib_list[i])
            {
            case EGL_CONTEXT_PRIORITY_LEVEL_IMG:
            case 0x4098:
                /* Ignored EGL attributes here.
                 * 0x4098 is for bug 7297. The app sends an unknown attribute 0x4098.
                 */
                break;

            case EGL_CONTEXT_CLIENT_VERSION:
                switch (thread->api)
                {
                case EGL_OPENGL_API:
                case EGL_OPENGL_ES_API:
                    major = attrib_list[i + 1];
                    break;
                default:
                    if (eglConfig->renderableType & EGL_OPENVG_BIT)
                    {
                        veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    }
                    else
                    {
                        veglSetEGLerror(thread, EGL_BAD_CONFIG);
                    }
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_MINOR_VERSION_KHR:
                switch (thread->api)
                {
                case EGL_OPENGL_API:
                case EGL_OPENGL_ES_API:
                    minor = attrib_list[i + 1];
                    break;
                default:
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_FLAGS_KHR:
                {
                    flags = attrib_list[i + 1];

                    if (flags & ~(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR))
                    {
                        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }

                    if (flags & (EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                 EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR))
                    {
                        switch (thread->api)
                        {
                        case EGL_OPENGL_API:
                        case EGL_OPENGL_ES_API:
                            if (flags & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR)
                            {
                                needCheckRobustAndVersion = EGL_TRUE;
                                setRobustAccess = EGL_TRUE;
                            }
                            break;
                        default:
                            veglSetEGLerror(thread,  EGL_BAD_ATTRIBUTE);
                            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                        }
                    }

                    if (flags & EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR)
                    {
                        if (thread->api != EGL_OPENGL_API)
                        {
                            veglSetEGLerror(thread,  EGL_BAD_ATTRIBUTE);
                            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                        }
                    }
                }
                break;

            case EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR:
                if (thread->api != EGL_OPENGL_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                break;

            case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR:
                /* According to EGL Spec:
                ** This attribute is supported only for OpenGL and OpenGL ES contexts of EGL 1.5 version.
                */
                if ((thread->api != EGL_OPENGL_API &&
                    thread->api != EGL_OPENGL_ES_API) ||
                    (thread->api == EGL_OPENGL_ES_API &&
                    EGL_MINOR_VERSION == 4))
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                resetNotification = (gctINT)attrib_list[i + 1];
                if (resetNotification != EGL_NO_RESET_NOTIFICATION_EXT &&
                    resetNotification != EGL_LOSE_CONTEXT_ON_RESET_EXT)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                else
                {
                    needCheckRobustAndVersion = EGL_TRUE;
                }
                robustAttribSet = gcvTRUE;
                break;

            case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
                if (thread->api != EGL_OPENGL_ES_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAccess = (gctBOOL)attrib_list[i + 1];
                if (robustAccess != EGL_FALSE && robustAccess != EGL_TRUE)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAttribSet = robustAttribSet || (robustAccess == EGL_TRUE);
                break;

            case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
                if (thread->api != EGL_OPENGL_ES_API)
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                resetNotification = (gctINT)attrib_list[i + 1];
                /* According to EGL Spec:
                ** An EGL_BAD_ATTRIBUTE error is generated if an attribute is
                ** specified that is not supported for the client API type.
                */
                if ((resetNotification != EGL_NO_RESET_NOTIFICATION_EXT &&
                    resetNotification != EGL_LOSE_CONTEXT_ON_RESET_EXT) ||
                    !_IsExtSuppored(VEGL_EXTID_EXT_create_context_robustness))
                {
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
                robustAttribSet = gcvTRUE;
                break;

            case EGL_PROTECTED_CONTENT_EXT:
                protectedContent = attrib_list[i + 1];
                break;

            default:
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        /* check robust access.*/
        if (needCheckRobustAndVersion)
        {
            /* for opengl es context, es3.0 support it.*/
            if (major < 3)
            {
                veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if (setRobustAccess)
            {
                robustAccess = EGL_TRUE;
            }
        }
    }

    if (eglConfig && thread->api == EGL_OPENGL_ES_API)
    {
        EGLBoolean valid = EGL_FALSE;
        EGLBoolean match = EGL_FALSE;
        gceCHIPMODEL chipModel;
        gctUINT32 chipRevision;
        EGLenum renderableType = eglConfig->renderableType;

#if defined(ANDROID)
        gctSTRING esVersion = gcvNULL;
        gcePATCH_ID patchId = gcvPATCH_INVALID;
        gcmVERIFY_OK(gcoHAL_GetPatchID(gcvNULL, &patchId));
#endif

        switch (major)
        {
        case 1:
            match = (minor == 0 || minor == 1) ? EGL_TRUE : EGL_FALSE;
            valid = (renderableType & EGL_OPENGL_ES_BIT ) ? EGL_TRUE : EGL_FALSE;
            valid = valid && (!robustAttribSet);
            break;

        case 2:
            match = (minor == 0) ? EGL_TRUE : EGL_FALSE;
            valid = (renderableType & EGL_OPENGL_ES2_BIT) ? EGL_TRUE : EGL_FALSE;
            break;

        case 3:
            gcmONERROR(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));

            /* Halti5 HW with TS/GS supports ES3.2 */
            if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI5) &&
                gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_GEOMETRY_SHADER) &&
                gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_TESSELLATION))
            {
                match = (minor >= 0 && minor <= 2) ? EGL_TRUE : EGL_FALSE;
            }
            /* Halti2 HW support ES30 and ES31 */
            else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI2) ||
                (chipModel == gcv900 && chipRevision == 0x5250))
            {
                match = (minor >= 0 && minor <= 1) ? EGL_TRUE : EGL_FALSE;
            }
            /* Halti0 HW support ES30 only */
#if defined(ANDROID)
            else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0) &&
                    !(((patchId == gcvPATCH_ANTUTU6X) || (patchId == gcvPATCH_ANTUTU3DBench))
                    && (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "ro.opengles.version", &esVersion)) &&
                    esVersion && gcmIS_SUCCESS(gcoOS_StrCmp(esVersion, "131072")))))
#else
            else if (gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0))
#endif
            {
                match = (minor == 0) ? EGL_TRUE : EGL_FALSE;
            }
            else
            {
                match = EGL_FALSE;
            }
            valid = (renderableType & EGL_OPENGL_ES3_BIT_KHR) ? EGL_TRUE : EGL_FALSE;
            break;
        }

        if (!match)
        {
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (!valid)
        {
            veglSetEGLerror(thread,  EGL_BAD_CONFIG);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else if (eglConfig && (thread->api == EGL_OPENVG_API)
    &&  !(eglConfig->renderableType & EGL_OPENVG_BIT)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    else if (eglConfig && (thread->api == EGL_OPENGL_API)
    &&  !(eglConfig->renderableType & EGL_OPENGL_BIT)
    )
    {
        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }


    /* Test if shared context is compatible. */
    if (sharedContext != gcvNULL)
    {
        /* Test for a valid context. */
        VEGLContext p_ctx = (VEGLContext)veglGetResObj(dpy,
                                                       (VEGLResObj*)&dpy->contextStack,
                                                       (EGLResObj)SharedContext ,
                                                       EGL_CONTEXT_SIGNATURE);
        if (p_ctx == gcvNULL)
        {
            veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (sharedContext->api != thread->api)
        {
            /* Bad match. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (sharedContext->resetNotification != resetNotification)
        {
            /* Bad match. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Create new context. */
    status = gcoOS_Allocate(gcvNULL,
                            gcmSIZEOF(struct eglContext),
                            &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        veglSetEGLerror(thread, EGL_BAD_ALLOC);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    context = pointer;

    /* Zero the context memory. */
    gcoOS_ZeroMemory(context, gcmSIZEOF(struct eglContext));

    /* Initialize context. */
    context->thread        = gcvNULL;
    context->api           = thread->api;
    context->client        = (major << 4) | minor;
    context->display       = dpy;
    context->sharedContext = sharedContext;
    context->draw          = EGL_NO_SURFACE;
    context->read          = EGL_NO_SURFACE;
    context->resObj.signature = EGL_CONTEXT_SIGNATURE;
    context->flags         = flags;
    context->robustAccess  = robustAccess;
    context->resetNotification = resetNotification;
    context->protectedContent  = protectedContent;

    if (thread->api == EGL_OPENGL_API)
    {
        context->coreProfile = gcvFALSE;
    }

#if gcdGC355_PROFILER
    {
        extern gctUINT64 _AppstartTimeusec;
        gctCHAR fileName[256] = {'\0'};

        gctUINT offset = 0;
        gctHANDLE pid = gcoOS_GetCurrentProcessID();
        gctHANDLE tid = gcoOS_GetCurrentThreadID();

        context->appStartTime = _AppstartTimeusec;

        gcoOS_PrintStrSafe(
            fileName,
            gcmSIZEOF(fileName),
            &offset,
#if defined(ANDROID)
            "/data/data/APITimes_pid%d_tid%d.log",
#   else
            "APITimes_pid%d_tid%d.log",
#   endif
            (gctUINTPTR_T)pid,
            (gctUINTPTR_T)tid
            );

        gcoOS_Open(
            gcvNULL,
            fileName,
            gcvFILE_CREATE,
            &context->apiTimeFile
            );
    }
#endif

    /* Copy config information */
    if (eglConfig)
        gcoOS_MemCopy(&context->config, eglConfig, sizeof(context->config));

    /* Push context into stack. */
    veglPushResObj(dpy, (VEGLResObj *)&dpy->contextStack, (VEGLResObj)context);

    /* Create context for API. */
    context->context = _CreateApiContext(
        thread, context,
        eglConfig,
        (sharedContext != gcvNULL) ? sharedContext->context : gcvNULL,
        (sharedContext != gcvNULL) ? sharedContext->client : 0);

    if (context->context == gcvNULL)
    {
        /* Roll back. */
        veglPopResObj(dpy, (VEGLResObj *)&dpy->contextStack, (VEGLResObj)context);

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, context));

        veglSetEGLerror(thread,  EGL_BAD_CONFIG);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (dpy->platform && dpy->platform->platform == EGL_PLATFORM_DRI_VIV)
    {
        dpy->platform->createContext(dpy->localInfo, context);
    }

    /* Useful for debugging */
    if (eglConfig)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, _GC_OBJ_ZONE,
                "a,b,g,r=%d,%d,%d,%d, d,s=%d,%d, id=%d, AA=%d, t=0x%08X",
                eglConfig->alphaSize,
                eglConfig->blueSize,
                eglConfig->greenSize,
                eglConfig->redSize,
                eglConfig->depthSize,
                eglConfig->stencilSize,
                eglConfig->configId,
                eglConfig->samples,
                eglConfig->surfaceType
            );
    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    VEGL_TRACE_API_POST(CreateContext)(Dpy, config, SharedContext, attrib_list, context);
    gcmDUMP_API("${EGL eglCreateContext 0x%08X 0x%08X 0x%08X (0x%08X) := "
                "0x%08X",
                Dpy, config, SharedContext, attrib_list, context);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("0x%x", context);
    return context;

OnError:

    gcmFOOTER_ARG("0x%x", EGL_NO_CONTEXT);
    return EGL_NO_CONTEXT;

}

static EGLBoolean
_DestroyContext(
    IN VEGLThreadData Thread,
    VEGLDisplay Display,
    VEGLContext Context
    )
{
    VEGLContext current;
    gceHARDWARE_TYPE currentType  = gcvHARDWARE_INVALID;
    gceHARDWARE_TYPE requiredType;

    gcmHEADER_ARG("Thread=0x%x Display=0x%x Context=0x%x",
                  Thread, Display, Context);

    /* Get current hardwaret type. */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

    switch (Context->api)
    {
    case EGL_OPENGL_API:
        current = Thread->glContext;
        requiredType = currentType;
        break;
    case EGL_OPENGL_ES_API:
        current = Thread->esContext;
        requiredType = gcvHARDWARE_3D;
        break;
    case EGL_OPENVG_API:
        current = Thread->vgContext;
        requiredType = Thread->openVGpipe ?  gcvHARDWARE_VG : gcvHARDWARE_3D;
        break;
    default:
        current = gcvNULL;
        requiredType = currentType;
        gcmTRACE(gcvLEVEL_ERROR,
                 "%s(%d): _DestroyContext bad current rendering API.",
                 __FUNCTION__, __LINE__);
        break;
    }

    if (Context->thread && (Context->thread != Thread))
    {
        Context->deleteReq = gcvTRUE;

        veglSetEGLerror(Thread,  EGL_SUCCESS);

        gcmTRACE(gcvLEVEL_INFO,
                 "%s(%d): _DestroyContext:context is still in current of other thread",
                 __FUNCTION__, __LINE__);

        gcmFOOTER_ARG("%d", EGL_TRUE);
        return EGL_TRUE;
    }

    if (requiredType != currentType)
    {
        /* Switch hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, requiredType);
    }

    if (current == Context)
    {
        gcmTRACE(gcvLEVEL_INFO,
                 "%s(%d): _DestroyContext: context is still in current of current thread.",
                 __FUNCTION__, __LINE__);

        /*  Make sure all swap workers have been processed before detaching
         *  window drawable from the current context.
         */
        if (current->draw && (current->draw->type & EGL_WINDOW_BIT))
        {
            if (Display->workerThread != gcvNULL)
            {
                gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                current->draw->workerDoneSignal, gcvINFINITE));
            }
        }

        /* Remove context. */
        gcmVERIFY(_ApiLoseCurrent(Thread, current));
    }

    /* Destroy client API context. */
    if (Context->context != gcvNULL)
    {
        _DestroyApiContext(Thread, Context, Context->context);
        Context->context = gcvNULL;
    }

    if (requiredType != currentType)
    {
        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, currentType);
    }

    /* Deference any surfaces. */
    if (Context->draw != EGL_NO_SURFACE)
    {
        VEGLSurface surface = Context->draw;

        veglDereferenceSurface(Thread, Display, surface, EGL_FALSE);

        /* If the surface has been destoried then free the struct .*/
        if (surface->reference == gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
        }
    }

    if (Context->read != EGL_NO_SURFACE)
    {
        VEGLSurface surface = Context->read;

        veglDereferenceSurface(Thread, Display, surface, EGL_FALSE);

        /* If the surface has been destoried then free the struct .*/
        if (surface->reference == gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, surface));
        }
    }

    if (Thread->context == Context)
    {
        /* Current context has been destroyed. */
        Thread->context  = gcvNULL;
    }

    if (Thread->esContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->esContext  = gcvNULL;
    }

    if (Thread->vgContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->vgContext  = gcvNULL;
    }

    if (Thread->glContext == Context)
    {
        /* Current context has been destroyed. */
        Thread->glContext  = gcvNULL;
    }

    /* Pop EGLContext from the stack. */
    veglPopResObj(Display, (VEGLResObj *)&Display->contextStack, (VEGLResObj)Context);

    /* Reset the context. */
    Context->thread = gcvNULL;
    Context->api    = EGL_NONE;

    if (Display->platform && Display->platform->platform == EGL_PLATFORM_DRI_VIV)
    {
        Display->platform->destroyContext(Display->localInfo, Context);
    }

    /* Execute events accumulated in the buffer if any. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Free the eglContext structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Context));

    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLBoolean
veglDestroyContext(
    EGLDisplay Dpy,
    EGLContext Ctx
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLContext ctx;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Ctx=0x%x", Dpy, Ctx);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test for valid bounded API. */
    if (thread->api == EGL_NONE)
    {
        /* Bad match. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
        gcmFOOTER_ARG("%d (line %d)", EGL_FALSE, __LINE__);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Find the context object on the stack. */
    ctx = (VEGLContext)veglGetResObj(dpy,
                                     (VEGLResObj*)&dpy->contextStack,
                                     (EGLResObj)Ctx ,
                                     EGL_CONTEXT_SIGNATURE);

    /* Test if ctx is valid. */
    if (ctx == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#if gcdGC355_PROFILER
    if (ctx->apiTimeFile != gcvNULL)
    {
        gcoOS_Flush(gcvNULL,ctx->apiTimeFile);
        gcoOS_Close(gcvNULL, ctx->apiTimeFile);
        ctx->apiTimeFile = gcvNULL;
    }
#endif

    /* Do actual destroy. */
    _DestroyContext(thread, dpy, ctx);

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("%d", EGL_FALSE);
    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroyContext(
    EGLDisplay Dpy,
    EGLContext Ctx
    )
{
    EGLBoolean ret;
    VEGL_TRACE_API(DestroyContext)(Dpy, Ctx);

    gcmDUMP_API("${EGL eglDestroyContext 0x%08X 0x%08X}", Dpy, Ctx);

    ret = veglDestroyContext(Dpy, Ctx);

    return ret;
}

EGLBoolean
veglMakeCurrent(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLSurface Read,
    EGLContext Ctx
    )
{
    EGLBoolean result;
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSurface draw;
    VEGLSurface read;
    VEGLContext ctx, current;
    VEGLPlatform platform;
    EGLint width = 0, height = 0;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Draw=0x%x Read=0x%x Ctx=0x%x",
                  Dpy, Draw, Read, Ctx);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);

    if (dpy == gcvNULL)
    {
        /* Bad display. */
        veglSetEGLerror(thread,  EGL_BAD_DISPLAY);

        gcmFOOTER_ARG("%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Get shortcut. */
    platform = dpy->platform;

    /* Test for valid EGLContext structure */
    ctx = (VEGLContext)veglGetResObj(dpy,
                                     (VEGLResObj*)&dpy->contextStack,
                                     (EGLResObj)Ctx ,
                                     EGL_CONTEXT_SIGNATURE);

    if (Ctx != EGL_NO_CONTEXT && ctx == gcvNULL)
    {
        veglSetEGLerror(thread,  EGL_BAD_CONTEXT);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for valid EGLSurface structure */
    draw = (VEGLSurface)veglGetResObj(dpy,
                                      (VEGLResObj*)&dpy->surfaceStack,
                                      (EGLResObj)Draw,
                                      EGL_SURFACE_SIGNATURE);

    if (Draw != EGL_NO_SURFACE && draw == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    read = (VEGLSurface)veglGetResObj(dpy,
                                      (VEGLResObj*)&dpy->surfaceStack,
                                      (EGLResObj)Read,
                                      EGL_SURFACE_SIGNATURE);

    if (Read != EGL_NO_SURFACE && read == gcvNULL)
    {
        veglSetEGLerror(thread, EGL_BAD_SURFACE);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for content protection */
    if (draw && (draw->protectedContent == EGL_FALSE) &&
        read && (read->protectedContent == EGL_TRUE))

    {
        veglSetEGLerror(thread, EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (draw && (draw->type & EGL_WINDOW_BIT))
    {
        /*  Make sure all swap workers have been processed before attaching
         *  window drawable to a new context.
         */
        if (dpy->workerThread != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                            draw->workerDoneSignal, gcvINFINITE));
        }

        /* Validate native window. */
        result = platform->getWindowSize(dpy, draw, &width, &height);

        if (!result)
        {
            /* Window is gone. */
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    else if (draw && (draw->type & EGL_PIXMAP_BIT))
    {
        /* Validate native pixmap. */
        result = platform->getPixmapSize(dpy, draw->pixmap,
                                         draw->pixInfo,
                                         &width, &height);

        if (!result)
        {
            /* Pixmap is gone. */
            veglSetEGLerror(thread, EGL_BAD_NATIVE_PIXMAP);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Test for release of context. */
    if (!ctx && !draw && !read)
    {
        if (thread->context != EGL_NO_CONTEXT)
        {
#if VIVANTE_PROFILER
            if (thread->context->draw != gcvNULL && thread->context->draw != draw && _ProfilerCallback(thread, 0, 0))
            {
#if defined(ANDROID)
                veglSuspendSwapWorker(dpy);
#endif
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));

                if (thread->api == EGL_OPENVG_API)
                {
                    _ProfilerCallback(thread, 10, 0);
                }
#if defined(ANDROID)
                veglResumeSwapWorker(dpy);
#endif
            }
#endif
            current = thread->context;

            /* Remove context. */
            gcmVERIFY(_ApiLoseCurrent(thread, current));

            if (current->draw != gcvNULL)
            {
                VEGLSurface sur = current->draw;

                if (sur->type & EGL_WINDOW_BIT)
                {
                    if (sur->newSwapModel && sur->backBuffer.surface)
                    {
                        /*
                         * A back buffer should be acquired if new swap model.
                         * No if failed to get window back buffer previously.
                         */
                        platform->cancelWindowBackBuffer(dpy, sur, &sur->backBuffer);

                        /* Clear back buffer. */
                        sur->backBuffer.context = gcvNULL;
                        sur->backBuffer.surface = gcvNULL;
                    }

                    /* Unbind this window. */
                    platform->unbindWindow(dpy, sur);
                    sur->bound = EGL_FALSE;
                }

                if (sur->renderMode > 0)
                {
                    /* Track correct previous render target before release. */
                    if (sur->renderTarget &&
                        gcoSURF_QueryFlags(sur->renderTarget,
                                           gcvSURF_FLAG_CONTENT_UPDATED))
                    {
                        /*
                         * Release current after render target is ever rendered.
                         * Take current render target as reference.
                         * Always need to track render target, even in DESTROYED
                         * mode, because we should not drop pixels
                         * when EGL current switches.
                         */
                        if (sur->prevRenderTarget != gcvNULL)
                        {
                            /* Dereference previous render target. */
                            gcoSURF_Destroy(sur->prevRenderTarget);
                        }

                        /* The current render target becomes previous. */
                        sur->prevRenderTarget = sur->renderTarget;

                        /* Dereference external window back buffer. */
                        sur->renderTarget = gcvNULL;
                    }
                    else
                    {
                        /* Previous render target not change. */
                        if (sur->renderTarget != gcvNULL)
                        {
                            /* Dereference external window back buffer. */
                            gcoSURF_Destroy(sur->renderTarget);
                            sur->renderTarget = gcvNULL;
                        }
                    }

                    /* Sync drawable with renderTarget. */
                    sur->drawable.rtHandles[0] = gcvNULL;
                    sur->drawable.prevRtHandles[0] = gcvNULL;
                }

                /* Dereference the current draw surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                current->draw = gcvNULL;
            }

            if (current->read != gcvNULL)
            {
                VEGLSurface sur = current->read;

                /* Dereference the current read surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                current->read = gcvNULL;
            }

            /* Unbind thread from context. */
            current->thread = gcvNULL;

            /* Set context to EGL_NO_CONTEXT. */
            thread->context = EGL_NO_CONTEXT;

            switch(thread->api)
            {
            case EGL_OPENGL_ES_API:
                thread->esContext = EGL_NO_CONTEXT;
                break;
            case EGL_OPENVG_API:
                thread->vgContext = EGL_NO_CONTEXT;
                break;
            case EGL_OPENGL_API:
                thread->glContext = EGL_NO_CONTEXT;
                break;
            }

            /*
             * If this context was requested to be deleted,
             * now it can be deleted.
             */
            if (current->deleteReq)
            {
                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): veglMakeCurren delete defer freed context.",
                         __FUNCTION__, __LINE__);

                _DestroyContext(thread, dpy, current);
            }
        }

        /* Uninstall the owner thread */
        dpy->ownerThread = gcvNULL;

        /* Success. */
        veglSetEGLerror(thread,  EGL_SUCCESS);
        gcmFOOTER_ARG("%d", EGL_TRUE);
        return EGL_TRUE;
    }

    /* Test for EGL_KHR_surfaceless_context */
    if (ctx && !draw && !read)
    {
        switch (ctx->api)
        {
        case EGL_OPENGL_ES_API:
            break;

        default:
            /* Do not support surfaceless context for other API. */
            veglSetEGLerror(thread, EGL_BAD_MATCH);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* OpenGL ES didn't support make current w/o default framebuffer */
    else if (!(ctx && draw && read))
    {
        veglSetEGLerror(thread, EGL_BAD_MATCH);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if surface is locked. */
    if ((draw && draw->locked) || (read && read->locked))
    {
        veglSetEGLerror(thread,  EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check if context is current to other thread. */
    if ((ctx->thread != gcvNULL) && (ctx->thread != thread))
    {
        /* Bad access. */
        veglSetEGLerror(thread, EGL_BAD_ACCESS);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (draw != gcvNULL)
    {
        VEGLContext context = (VEGLContext) dpy->contextStack;

        while (context != gcvNULL)
        {
            if (context != ctx)
            {
                if ((context->draw == draw) || (context->read == draw) ||
                    (context->draw == read) || (context->read == read))
                {
                    if ((context->thread != gcvNULL) &&
                        (context->thread != thread))
                    {
                        /* Surface is current to other thread. */
                        veglSetEGLerror(thread, EGL_BAD_ACCESS);
                        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }
                }
            }

            context = (VEGLContext) context->resObj.next;
        }
    }

    /*
     * Step 1: unlink old draw and read for new context.
     * link with new read and draw
     */
    do
    {
        if (ctx->draw != draw)
        {
#if gcdENABLE_3D
            if (ctx->draw != gcvNULL &&
                ctx->draw->texBinder != gcvNULL &&
                ctx->draw->renderTarget != gcvNULL)
            {
                gcsSURF_VIEW rtView = {ctx->draw->renderTarget, 0, 1};
                gcsSURF_VIEW texView = {ctx->draw->texBinder, 0, 1};
                /*
                 * Resolve surface to texture when surface becomes not current.
                 * During resolve, we do not need flip.
                 */

                /* Sync the render target content to texture. */
                if (gcmIS_ERROR(gcoSURF_ResolveRect(&rtView, &texView, gcvNULL)))
                {
                    /* Resolve failed, set error enum. */
                    veglSetEGLerror(thread, EGL_BAD_SURFACE);
                }

                if (thread->error != EGL_SUCCESS)
                {
                    gcmONERROR(gcvSTATUS_INVALID_REQUEST);
                }
            }
#endif

            if (ctx->draw != gcvNULL)
            {
                VEGLSurface sur = ctx->draw;

                if (sur->type & EGL_WINDOW_BIT)
                {
                    if (sur->newSwapModel && sur->backBuffer.surface)
                    {
                        /*
                         * A back buffer should be acquired if new swap model.
                         * No if failed to get window back buffer previously.
                         */
                        platform->cancelWindowBackBuffer(dpy, sur, &sur->backBuffer);

                        /* Clear back buffer. */
                        sur->backBuffer.context = gcvNULL;
                        sur->backBuffer.surface = gcvNULL;
                    }

                    /* Unbind this window. */
                    platform->unbindWindow(dpy, sur);
                    sur->bound = EGL_FALSE;
                }

                if (sur->renderMode > 0)
                {
                    /* Track correct previous render target before release. */
                    if (sur->renderTarget &&
                        gcoSURF_QueryFlags(sur->renderTarget,
                                           gcvSURF_FLAG_CONTENT_UPDATED))
                    {
                        /*
                         * Switch current after render target is ever rendered.
                         * Take current render target as reference.
                         * Always need to track render target, even in DESTROYED
                         * mode, because we should not drop pixels
                         * when EGL current switches.
                         */
                        if (sur->prevRenderTarget != gcvNULL)
                        {
                            /* Dereference previous render target. */
                            gcoSURF_Destroy(sur->prevRenderTarget);
                        }

                        /* The current render target becomes previous. */
                        sur->prevRenderTarget = sur->renderTarget;

                        /* Dereference external window back buffer. */
                        sur->renderTarget = gcvNULL;
                    }
                    else
                    {
                        /* Previous render target not change. */
                        if (sur->renderTarget != gcvNULL)
                        {
                            /* Dereference external window back buffer. */
                            gcoSURF_Destroy(sur->renderTarget);
                            sur->renderTarget = gcvNULL;
                        }
                    }

                    /* Sync drawable with renderTarget. */
                    sur->drawable.rtHandles[0]     = gcvNULL;
                    sur->drawable.prevRtHandles[0] = gcvNULL;
                }

                /* Dereference the current draw surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                ctx->draw = EGL_NO_SURFACE;
            }

            /* Reference the new draw surface. */
            if (draw)
            {
                if (!veglReferenceSurface(thread, dpy, draw))
                {
                    /* Error. */
                    result = EGL_FALSE;
                    break;
                }
            }

            /* Set the new draw surface. */
            ctx->draw = draw;
        }

        if (ctx->read != read)
        {
            if (ctx->read != gcvNULL)
            {
                VEGLSurface sur = ctx->read;

                /* Dereference the current read surface. */
                veglDereferenceSurface(thread, dpy, sur, EGL_FALSE);

                /* If the surface has been destoried then free the struct .*/
                if (sur->reference == gcvNULL)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sur));
                }

                ctx->read = EGL_NO_SURFACE;
            }

            /* Reference the new read surface. */
            if (read)
            {
                if (!veglReferenceSurface(thread, dpy, read))
                {
                    /* Error. */
                    result = EGL_FALSE;
                    break;
                }
            }

            /* Set the new read surface. */
            ctx->read = read;
        }

        /* Success. */
        result = EGL_TRUE;
    }
    while (gcvFALSE);

    if (!result)
    {
        veglSetEGLerror(thread,  EGL_BAD_PARAMETER);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch(ctx->api)
    {
    case EGL_OPENGL_API:
        current = thread->glContext;
        break;
    case EGL_OPENGL_ES_API:
        current = thread->esContext;
        break;
    case EGL_OPENVG_API:
        current = thread->vgContext;
        break;
    default:
        current = gcvNULL;
        gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglMakeCurrent bad current rendering API.",__FUNCTION__, __LINE__);
        break;
    }

    /* Step 2: Unlink current context from thread. */
    if (current)
    {
        /* Remove context. */
        gcmVERIFY(_ApiLoseCurrent(thread, current));

        current->thread = gcvNULL;

        if (current->deleteReq && current != ctx)
        {
            gcmTRACE(gcvLEVEL_INFO,
                     "%s(%d): veglMakeCurrent delete defer freed context.",
                     __FUNCTION__, __LINE__);

            _DestroyContext(thread, dpy, current);
        }
    }

    /* Step 3: bind native window. */
    if (draw && (draw->type & EGL_WINDOW_BIT) && !draw->bound)
    {
        /* Bind native window for rendering. */
        result = platform->bindWindow(dpy, draw, &draw->renderMode);

        if (!result)
        {
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        draw->bound = EGL_TRUE;

        /* Must use new swap model for direct rendering. */
        draw->newSwapModel = (draw->renderMode > 0);

#if defined(ANDROID)
        /* Force new swap model on android platform. */
        draw->newSwapModel = gcvTRUE;
#endif

        if (draw->newSwapModel)
        {
            /*
             * Get window buffer if not acquired.
             * If context is not switched, window is already connected.
             * Or if hwc obtains buffers when no current surface, the window
             * buffer should be also acquired already.
             */
            if (draw->backBuffer.surface == gcvNULL)
            {
                result = platform->getWindowBackBuffer(dpy, draw, &draw->backBuffer);

                if (!result)
                {
                    /* FIXME: Unbind window here?? */
                    veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }

        if (draw->renderMode > 0)
        {
            /* Switch render target to external surface. */
            if (draw->renderTarget != gcvNULL)
            {
                /* Destroy internal render target. */
                gcmVERIFY_OK(gcoSURF_Destroy(draw->renderTarget));
            }

            /* Reference external surface. */
            draw->renderTarget = draw->backBuffer.surface;
            gcoSURF_SetColorType(draw->renderTarget, draw->colorType);

            if (draw->renderTarget != gcvNULL)
            {
                /* Inc reference count. */
                gcmVERIFY_OK(gcoSURF_ReferenceSurface(draw->renderTarget));
            }

            if ((draw->prevRenderTarget == draw->renderTarget) &&
                (draw->prevRenderTarget != gcvNULL))
            {
                /* Do not need to keep previous if it is the same. */
                gcoSURF_Destroy(draw->prevRenderTarget);
                draw->prevRenderTarget = gcvNULL;
            }

            /*
             * In DESTROYED mode, we still need to recover buffer pixels from
             * EGLSurface when it is switched out. Need copy the buffers right
             * here in this case.
             *
             * In PRESERVED mode, set previous render target and client driver
             * will preserve pixels there.
             */
            if ((draw->swapBehavior == EGL_BUFFER_DESTROYED) &&
                (draw->renderTarget != gcvNULL) &&
                (draw->prevRenderTarget != gcvNULL))
            {
#if gcdENABLE_VG
                if (draw->openVG)
                {
                    gcoSURF_Copy(draw->renderTarget, draw->prevRenderTarget);
                }
                else
#endif
                {
#if gcdENABLE_3D
                    gcsSURF_VIEW srcView = {draw->prevRenderTarget, 0, 1};
                    gcsSURF_VIEW trgView  = {draw->renderTarget,    0, 1};

                    gcoSURF_ResolveRect(&srcView, &trgView, gcvNULL);
#endif
                }

                /* Now drop previous render target. */
                gcoSURF_Destroy(draw->prevRenderTarget);
                draw->prevRenderTarget = gcvNULL;
            }

            /* Sync drawable with renderTarget. */
            draw->drawable.rtHandles[0]     = draw->renderTarget;
            draw->drawable.prevRtHandles[0] = draw->prevRenderTarget;
        }
        else if (draw->renderTarget == gcvNULL)
        {
            /* Defer allocate render target Create render target. */
            result = veglCreateRenderTarget(thread, draw);

            if (!result)
            {
                veglSetEGLerror(thread, EGL_BAD_ALLOC);
                gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
        }

        /* The renderTarget is not determined. Check lockBuffer. */
        if (draw->lockBufferMirror != gcvNULL)
        {
#if gcdENABLE_3D
            gcsSURF_VIEW mirrorView = {draw->lockBufferMirror, 0, 1};
            gcsSURF_VIEW rtView = {draw->renderTarget, 0, 1};

            /* Resolve to renderTarget. */
            status = gcoSURF_ResolveRect(&mirrorView, &rtView, gcvNULL);

            if (gcmIS_ERROR(status))
            {
                veglSetEGLerror(thread,  EGL_BAD_ACCESS);
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
#endif

            /* Destroy the lockBuffer mirror. */
            gcmVERIFY_OK(gcoSURF_Destroy(draw->lockBufferMirror));
            draw->lockBufferMirror = gcvNULL;

            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }
    }

    if (draw && (draw->type & EGL_WINDOW_BIT))
    {
        /* Detect native window resize. */
        result = platform->getWindowSize(dpy, draw, &width, &height);

        if (result == EGL_FALSE)
        {
            veglSetEGLerror(thread, EGL_BAD_NATIVE_WINDOW);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if ((width  != draw->config.width) ||
            (height != draw->config.height))
        {
            /* Native window resized. */
            if (EGL_SUCCESS != veglResizeSurface(dpy, draw,
                                                 (gctUINT) width,
                                                 (gctUINT) height))
            {
                veglSetEGLerror(thread, EGL_BAD_SURFACE);
                result = EGL_FALSE;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }
    }

    /* Step 4: flush new context with new read and draw. */
    if (draw && read)
    {
        /* Normal make current. */
        if (!_ApiMakeCurrent(thread, ctx, &draw->drawable, &read->drawable))
        {
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        /* Make current for EGL_KHR_surfaceless_context. */
        if (!_ApiMakeCurrent(thread, ctx, gcvNULL, gcvNULL))
        {
            veglSetEGLerror(thread, EGL_BAD_SURFACE);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* step 5. link new context. */
    switch(ctx->api)
    {
    case EGL_OPENGL_ES_API:
        thread->esContext = ctx;
        break;
    case EGL_OPENVG_API:
        thread->vgContext = ctx;
        break;
    case EGL_OPENGL_API:
        thread->glContext = ctx;
        break;
    }

    /* if ctx which is being made current is current rendering API,
    ** update shortcut of current rendering API's current context.
    */
    if (ctx->api == thread->api)
    {
        thread->context = ctx;
    }

    /* Set the new current thread. */
    ctx->thread = thread;

    if (platform && platform->platform == EGL_PLATFORM_DRI_VIV)
    {
        if (draw && (draw->type & EGL_WINDOW_BIT))
        {
            struct eglBackBuffer backBuffer;

            /* Borrow it from window back buffer. */
            platform->getWindowBackBuffer(dpy, draw, &backBuffer);

            platform->makeCurrent(
                dpy->localInfo,
                draw->hwnd,
                read->hwnd,
                ctx,
                backBuffer.surface
                );

            /* Cancel the back buffer, actually does nothing for DRI. */
            platform->cancelWindowBackBuffer(dpy, draw, &backBuffer);
        }
    }

    /* Install the owner thread */
    dpy->ownerThread = gcoOS_GetCurrentThreadID();

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    gcmFOOTER_ARG("%d", EGL_TRUE);

    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("%d", EGL_FALSE);

    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglMakeCurrent(
    EGLDisplay Dpy,
    EGLSurface Draw,
    EGLSurface Read,
    EGLContext Ctx
    )
{
    EGLBoolean ret;
    VEGL_TRACE_API(MakeCurrent)(Dpy, Draw, Read, Ctx);

    gcmDUMP_API("${EGL eglMakeCurrent 0x%08X 0x%08X 0x%08X 0x%08X}",
                Dpy, Draw, Read, Ctx);

    ret = veglMakeCurrent(Dpy, Draw, Read, Ctx);
    return ret;
}

EGLBoolean
veglReleaseThread(
    VEGLThreadData Thread
    )
{
    gcmHEADER_ARG("Thread=%p", Thread);

    if (Thread->esContext != gcvNULL)
    {
        veglBindAPI(Thread, EGL_OPENGL_ES_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->esContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->esContext = gcvNULL;

    }

    if (Thread->vgContext != gcvNULL)
    {
        veglBindAPI(Thread, EGL_OPENVG_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->vgContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->vgContext = gcvNULL;

    }

    if (Thread->glContext != gcvNULL)
    {
        veglBindAPI(Thread, EGL_OPENGL_API);

        /* Unbind the context. */
        veglMakeCurrent(Thread->glContext->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);

        Thread->glContext = gcvNULL;

    }

    Thread->context = gcvNULL;

    /* Set API to EGL_OPENGL_ES_API */
    veglBindAPI(Thread, EGL_OPENGL_ES_API);

    /* Success. */
    gcmFOOTER_ARG("%d", EGL_TRUE);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglReleaseThread(
    void
    )
{
    EGLBoolean ret;
    VEGLThreadData thread;

    VEGL_TRACE_API(ReleaseThread)();

    gcmDUMP_API("${EGL eglReleaseThread}");

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        return EGL_FALSE;
    }

    ret = veglReleaseThread(thread);

    /* Destroy thread data. */
    gcoOS_FreeThreadData();

    return ret;
}

void *
_GetCurrentAPIContext(
    EGLenum Api
    )
{
    VEGLThreadData thread;
    void * context;
    VEGLContext veglContext;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        context = gcvNULL;
    }
    else
    {
        switch (Api)
        {
        case EGL_OPENGL_ES_API:
            veglContext = thread->esContext;
            break;
        case EGL_OPENVG_API:
            veglContext = thread->vgContext;
            break;
        case EGL_OPENGL_API:
            veglContext = thread->glContext;
            break;
        default:
            veglContext = gcvNULL;
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): bad rendering API parameters",
                     __FUNCTION__, __LINE__);
            break;
        }

        if (veglContext)
        {
            context = veglContext->context;
        }
        else
        {
            context = gcvNULL;
        }
    }

    gcmFOOTER_ARG("0x%x", context);
    return context;
}

void
_SetCurrentAPIContext(
    EGLenum Api,
    void* context
    )
{
    VEGLThreadData thread;
    VEGLContext veglContext;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread)
    {
        switch (Api)
        {
        case EGL_OPENGL_ES_API:
            veglContext = thread->esContext;
            break;
        case EGL_OPENVG_API:
            veglContext = thread->vgContext;
            break;
        case EGL_OPENGL_API:
            veglContext = thread->glContext;
            break;
        default:
            veglContext = gcvNULL;
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): bad rendering API parameters",
                     __FUNCTION__, __LINE__);
            break;
        }

        if (veglContext)
        {
            veglContext->context = context;
        }
    }

    gcmFOOTER_ARG("0x%x", context);
}

static void
_SyncPixmap(
    VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
#if gcdENABLE_3D
    VEGLSurface surface = gcvNULL;

    do
    {
        gcsSURF_VIEW rtView = {gcvNULL, 0, 1};
        gcsSURF_VIEW pixmapView = {gcvNULL, 0, 1};

        if ((Thread->context == gcvNULL)
        ||  (Thread->context->draw == gcvNULL)
        ||  (Thread->context->draw->renderTarget == gcvNULL)
        ||  !(Thread->context->draw->type & EGL_PIXMAP_BIT)
        )
        {
            /* No pixmap. */
            return;
        }

        /* Get current surface. */
        surface = Thread->context->draw;
        rtView.surf = surface->renderTarget;
        pixmapView.surf = surface->pixmapSurface;

        /* Resolve from RT to pixmap (temp or wrapper). */
        if (gcmIS_ERROR(gcoSURF_ResolveRect(&rtView, &pixmapView, gcvNULL)))
        {
            /* Resolve failed? */
            break;
        }

        /*
         * Do a stalled commit here as the
         * next operation could be a CPU operation.
         */
        gcoHAL_Commit(gcvNULL, gcvTRUE);

        /* Wait native pixmap. */
        Dpy->platform->syncToPixmap(surface->pixmap, surface->pixInfo);
    }
    while (gcvFALSE);
#endif  /* gcdENABLE_3D */
}

static void
_SyncImage(
    VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
#if gcdENABLE_3D
    VEGLImage       image;
    khrEGL_IMAGE_PTR   khrImage;

    gcmHEADER_ARG("Dpy=0x%08x",Dpy);

    if (!Dpy)
    {
        gcmFOOTER_NO();
        return;
    }

    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    image = Dpy->imageStack;

    while (image)
    {
        if (image->signature != EGL_IMAGE_SIGNATURE)
        {
            image = image->next;
            continue;
        }

        khrImage = (khrEGL_IMAGE_PTR)&(image->image);

        /* Lock the image mutex. */
        gcoOS_AcquireMutex(gcvNULL, khrImage->mutex, gcvINFINITE);

        /* Flush the tile status and decompress the buffers,
        since pixmap buffer could be get by QNX system calls.*/
        if ((khrImage->surface != gcvNULL) &&
            (khrImage->type == KHR_IMAGE_PIXMAP))
        {
            gcsSURF_VIEW surfView = {khrImage->surface, 0, 1};
            gcmVERIFY_OK(gcoSURF_DisableTileStatus(&surfView, gcvTRUE));
        }

        if ((khrImage->surface != gcvNULL) &&
            (khrImage->srcSurface != gcvNULL) &&
            (khrImage->surface != khrImage->srcSurface))
        {
            gcsSURF_VIEW srcView = {khrImage->srcSurface, 0, 1};
            gcsSURF_VIEW surfView = {khrImage->surface, 0, 1};

            gcoSURF_ResolveRect(&srcView, &surfView, gcvNULL);

            gcoSURF_Destroy(khrImage->srcSurface);
            khrImage->srcSurface = gcvNULL;
        }

        image = image->next;

        /* Release the image mutex. */
        gcoOS_ReleaseMutex(gcvNULL, khrImage->mutex);
    }
    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

    gcmFOOTER_NO();
#endif
}

static void
_SyncSwapWorker(
    VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
    if ((Dpy->workerThread != gcvNULL)
    &&  (Thread->context != gcvNULL)
    &&  (Thread->context->draw != gcvNULL)
    &&  (Thread->context->draw->type & EGL_WINDOW_BIT)
    )
    {
        VEGLSurface surface = Thread->context->draw;

        /* Make sure all workers have been processed. */
        gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                      surface->workerDoneSignal,
                                      gcvINFINITE));
    }
}

/*
 * Sync EGL objects to native objects, includes,
 * 1. native pixmap of PixmapSurface
 * 2. native object of EGLImage source sibling.
 * 3. the swap worker thread.
 *
 * Notice:
 * This function is asyncrhonized.
 */
void
veglSyncNative(
    IN VEGLThreadData Thread,
    VEGLDisplay Dpy
    )
{
    if ((Thread == gcvNULL) ||
        (Thread->context == gcvNULL) ||
        (Dpy == gcvNULL))
    {
        /* No context. */
        return;
    }

    /* Sync pixmap-surface to native pixmap. */
    _SyncPixmap(Thread, Dpy);

    /* Sync EGLImage source to native objects. */
    _SyncImage(Thread, Dpy);

    /* Sync swap worker thread. */
    _SyncSwapWorker(Thread, Dpy);
}

/* Callback function exported for client. */
void
_SyncNative(
    void
    )
{
    VEGLThreadData thread       = gcvNULL;
    VEGLDisplay dpy             = gcvNULL;

    thread = veglGetThreadData();

    if ((thread == gcvNULL) ||
        (thread->context == gcvNULL))
    {
        /* No context. */
        return;
    }

    /* Get current display. */
    /* dpy = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO); */
    dpy = (VEGLDisplay) thread->context->display;

    /* Call actual function. */
    veglSyncNative(thread, dpy);
}

EGLBoolean veglWaitClient(
    void
    )
{
    VEGLThreadData thread;
    EGLBoolean result = EGL_TRUE;

    gcmHEADER();

    thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        result = EGL_FALSE;
    }
    else
    {
        /* Hardware relevant thread data initialization. */
        veglInitDeviceThreadData(thread);

        result = _Flush(thread);

        if (result)
        {
            _SyncNative();
        }

        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitClient(
    void
    )
{
    EGLBoolean ret;

    VEGL_TRACE_API(WaitClient)();

    gcmDUMP_API("${EGL eglWaitClient}");

    ret = veglWaitClient();

    return ret;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitGL(
    void
    )
{
    EGLenum api;
    EGLBoolean result;
    VEGLThreadData thread;

    gcmHEADER();

    gcmDUMP_API("${EGL eglWaitGL}");
    VEGL_TRACE_API(WaitGL)();
    thread = veglGetThreadData();

    /* Backwards compatibility. */
    api = veglQueryAPI();
    veglBindAPI(thread, EGL_OPENGL_ES_API);

    result = veglWaitClient();

    veglBindAPI(thread, api);

    gcmFOOTER_ARG("%d", result);
    return result;
}

EGLenum
_BindTexImage(
    VEGLThreadData Thread,
    gcoSURF Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    gcoSURF *BindTo
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, gcvNULL);

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Surface=0x%08x,Format=0x%04x "
                  "Mipmap=%d,Level=0x%04x Width=%d Height=%d",
                  __FUNCTION__, __LINE__,
                  Thread, Surface, Format, Mipmap, Level, Width, Height);

    if ((dispatch == gcvNULL)
    ||  (dispatch->bindTexImage == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->bindTexImage)((void*)Surface, Format, Mipmap, Level, Width, Height, (void**)BindTo);
}

EGLenum
_CreateImageTexture(
    VEGLThreadData Thread,
    VEGLContext Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Target=0x%04x,Texture=0x%08x",
                  __FUNCTION__, __LINE__, Thread, Target, Texture);
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "Level=%d,Depth=%d,Image=0x%08x", Level, Depth, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageTexture == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->createImageTexture)(ApiContext,
                                           Target,
                                           Texture,
                                           Level,
                                           Depth,
                                           Image);
}

EGLenum
_CreateImageFromRenderBuffer(
    VEGLThreadData Thread,
    VEGLContext Context,
    gctUINT Renderbuffer,
    gctPOINTER Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    void * ApiContext = Context->context;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x, RenderBuffer=%u,Image=0x%08x",
                  __FUNCTION__, __LINE__,
                  Thread, Renderbuffer, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageRenderbuffer == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    return (*dispatch->createImageRenderbuffer)(ApiContext, Renderbuffer, Image);
}

EGLenum
_CreateImageFromVGParentImage(
    VEGLThreadData  Thread,
    VEGLContext Context,
    unsigned int Parent,
    VEGLImage Image
    )
{
    veglDISPATCH * dispatch = _GetDispatch(Thread, Context);
    EGLenum status;
    gctPOINTER images = gcvNULL;
    int count = 0;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcdZONE_EGL_CONTEXT,
                  "%s(%d): Thread=0x%08x,Parent=%u,Image=0x%08x",
                  __FUNCTION__, __LINE__,
                  Thread, Parent, Image);

    if ((dispatch == gcvNULL)
    ||  (dispatch->createImageParentImage == gcvNULL)
    )
    {
        return EGL_BAD_ACCESS;
    }

    status = (*dispatch->createImageParentImage)(Context->context,
                                                 Parent,
                                                 &images,
                                                 &count);

    if (count == 0)
    {
        return status;
    }

    /* Fill the VEGLImage list. */
    /* TOOD: Only copied the fist image from returned list?? */
    if ((Image != gcvNULL) && (images != gcvNULL))
    {
        khrEGL_IMAGE * khrImage = (khrEGL_IMAGE *) images;
        Image->image.magic      = khrImage->magic;
        Image->image.type       = khrImage->type;
        Image->image.surface    = khrImage->surface;
        Image->image.srcSurface = gcvNULL;
        Image->image.u.vgimage  = khrImage->u.vgimage;
    }

    if (images != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, images));
    }

    return EGL_SUCCESS;
}
