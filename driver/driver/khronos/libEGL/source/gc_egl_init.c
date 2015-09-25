/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_egl_precomp.h"
#include <gc_hal_eglplatform.h>

#define NO_STENCIL_VG   1
#define _GC_OBJ_ZONE    gcdZONE_EGL_INIT

gceTRACEMODE veglTraceMode = gcvTRACEMODE_NONE;

static gctBOOL enableSwapWorker = gcvTRUE;

#if gcdGC355_PROFILER
gctUINT64 AppstartTimeusec = 0;
#endif

static struct EGL_CONFIG_COLOR eglConfigColor[] =
{
#if VEGL_ENABLE_4444_5551_EGL_CONFIGS
    { 16, 12, 4, 4, 4, 0, VEGL_444  },  /* X4R4G4B4 */
    { 16, 16, 4, 4, 4, 4, VEGL_4444 },  /* A4R4G4B4 */
    { 16, 15, 5, 5, 5, 0, VEGL_555  },  /* X1R5G5B5 */
    { 16, 16, 5, 5, 5, 1, VEGL_5551 },  /* A1R5G5B5 */
#endif
    { 16, 16, 5, 6, 5, 0, VEGL_565  },  /* R5G6B5   */
    { 32, 24, 8, 8, 8, 0, VEGL_888  },  /* X8R8G8B8 */
#if !gcdENABLE_3D && gcdENABLE_VG
    { 8,  8,  0, 0, 0, 8, VEGL_ALPHA},  /* A8       */
#endif
    { 32, 32, 8, 8, 8, 8, (VEGL_COLOR_FORMAT) (VEGL_8888 | VEGL_DEFAULT) }, /* A8R8G8B8 */
};

static struct EGL_CONFIG_DEPTH eglConfigDepth[] =
{
    {  0, 0 },  /*       */
    { 16, 0 },  /* D16   */
    { 24, 0 },  /* D24X8 */
    { 24, 8 },  /* D24S8 */
    {  0, 8 },  /* S8 || X24S8*/
};

static const char extension[] =
    "EGL_KHR_fence_sync"
    " "
    "EGL_KHR_reusable_sync"
    " "
    "EGL_KHR_wait_sync"
    " "
    "EGL_KHR_image"
    " "
    "EGL_KHR_image_base"
    " "
    "EGL_KHR_image_pixmap"
    " "
    "EGL_KHR_gl_texture_2D_image"
    " "
    "EGL_KHR_gl_texture_cubemap_image"
    " "
    "EGL_KHR_gl_renderbuffer_image"
#if defined(LINUX)
    " "
    "EGL_EXT_image_dma_buf_import"
#endif
    " "
    "EGL_KHR_lock_surface"
    " "
    "EGL_KHR_create_context"
    " "
    "EGL_KHR_surfaceless_context"
    " "
    "EGL_EXT_create_context_robustness"
    " "
    "EGL_EXT_buffer_age"
    " "
    "EGL_EXT_protected_surface"
#if defined(ANDROID)
    " "
    "EGL_ANDROID_image_native_buffer"
    " "
    "EGL_ANDROID_swap_rectangle"
    " "
    "EGL_ANDROID_blob_cache"
    " "
    "EGL_ANDROID_recordable"
#if gcdANDROID_NATIVE_FENCE_SYNC
    " "
    "EGL_ANDROID_native_fence_sync"
#   endif
#endif
#if defined(WL_EGL_PLATFORM)
    " "
    "EGL_WL_bind_wayland_display"
#endif
    ;

/* See EGL SPEC 1.5 or EGL_EXT_client_extensions. */
static const char clientExtension[] =
#if defined(ANDROID)
    "EGL_KHR_platform_android"
    /* Not ready.
    " "
    "EGL_KHR_platform_gbm"
     */
#elif defined(X11)
    "EGL_KHR_platform_x11"
    /* Not ready.
    " "
    "EGL_KHR_platform_gbm"
     */
#elif defined(WL_EGL_PLATFORM)
    /* Not ready.
    "EGL_KHR_platform_wayland"
    " "
    "EGL_KHR_platform_gbm"
     */
#elif defined(LINUX)
    /* Not ready.
    "EGL_KHR_platform_gbm"
     */
#endif
    ""
    ;

#if defined(_WINDOWS)

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        {
            gcsTLS_PTR tls = gcvNULL;
            gcoOS_QueryTLS(&tls);
            if (tls)
            {
                tls->destructor = gcvNULL;
            }
        }
        break;

    default:
        break;

    }
    return TRUE;
}
#endif

#if gcdENABLE_TIMEOUT_DETECTION
static gceSTATUS
_SwapSignalTimeoutCallback(
    IN gcoOS Os,
    IN gctPOINTER Context,
    IN gctSIGNAL Signal,
    IN gctBOOL Reset
    )
{
    VEGLDisplay display;

    /* Cast display pointer. */
    display = (VEGLDisplay) Context;

    /* Ignore swap thread start signal. */
    if (Signal == display->startSignal)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }
    else
    {
        return gcvSTATUS_OK;
    }
}
#endif

static void
_DestroyProcessData(
    gcsPLS_PTR PLS
    )
{
    VEGLDisplay head;

    gcmHEADER_ARG("PLS=0x%x", PLS);

    head = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);

    while (head != gcvNULL)
    {
        VEGLDisplay display = head;

        head = display->next;
        /* Delete mutex */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, display->accessMutex));
        /* Delete mutex */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, display->resourceMutex));
        /* free memory of this display */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, display));
    }

    gcmFOOTER_NO();
}

static EGLBoolean
_ValidateMode(
    NativeDisplayType DeviceContext,
    VEGLConfigColor Color,
    VEGLConfigDepth Depth,
    EGLint * MaxSamples,
    EGLint * FastMSAA
    )
{
    /* Get thread data. */
    VEGLThreadData thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_FALSE;
    }

    /* Get sample count from thread data. */
    *MaxSamples = (thread->chipModel == gcv500) ? 0 : thread->maxSamples;

    /* Get fastmsaa feature from thread data. */
    *FastMSAA = thread->fastMSAA;

    /* Success. */
    return EGL_TRUE;
}

static void
_FillIn(
    VEGLConfig Config,
    EGLint * Index,
    VEGLConfigColor Color,
    VEGLConfigDepth Depth,
    EGLint Samples
    )
{
    VEGLConfig config;
    VEGLThreadData thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return;
    }

    /* Get a shortcut to the current configuration. */
    config = &Config[*Index];

    config->bufferSize  = Color->bufferSize;
    config->configBufferSize = Color->configBufferSize;
    config->alphaSize   = Color->alphaSize;
    config->blueSize    = Color->blueSize;
    config->greenSize   = Color->greenSize;
    config->redSize     = Color->redSize;
    config->depthSize   = Depth->depthSize;
    config->stencilSize = Depth->stencilSize;

    config->configCaveat = EGL_NONE;

    config->level                 = 0;
    config->maxSwapInterval       = 10;
    config->transparentType       = EGL_NONE;
    config->transparentRedValue   = EGL_DONT_CARE;
    config->transparentGreenValue = EGL_DONT_CARE;
    config->transparentBlueValue  = EGL_DONT_CARE;

    config->configId      = 1 + *Index;
    config->defaultConfig = (Color->formatFlags & VEGL_DEFAULT) != 0;

#ifdef _WIN32
    config->nativeRenderable
        = ((Color->formatFlags & VEGL_888) != 0);
#else
    config->nativeRenderable
        =  ((Color->formatFlags & VEGL_565) != 0)
        || ((Color->formatFlags & VEGL_888) != 0);
#endif
    config->nativeVisualType = config->nativeRenderable
        ? (Color->redSize == 8) ? 32 : 16
        : EGL_NONE;

    config->samples       = Samples;
    config->sampleBuffers = (Samples > 0) ? 1 : 0;

    config->surfaceType = EGL_WINDOW_BIT
                        | EGL_PBUFFER_BIT
                        | EGL_LOCK_SURFACE_BIT_KHR
                        | EGL_OPTIMAL_FORMAT_BIT_KHR
                        | EGL_SWAP_BEHAVIOR_PRESERVED_BIT;

    if (config->nativeRenderable)
    {
        config->surfaceType |= EGL_PIXMAP_BIT;
    }

    config->bindToTetxureRGB  = (Color->alphaSize == 0);
    config->bindToTetxureRGBA = EGL_TRUE;

    config->luminanceSize = 0;
    config->alphaMaskSize = 0;

    config->colorBufferType = EGL_RGB_BUFFER;

    config->renderableType = EGL_OPENGL_ES_BIT
                           | EGL_OPENGL_ES2_BIT
                           | EGL_OPENGL_ES3_BIT_KHR;

    config->conformant     = EGL_OPENGL_ES_BIT
                           | EGL_OPENGL_ES2_BIT
                           | EGL_OPENGL_ES3_BIT_KHR;

    if (Samples == 16)
    {
        /* Remove VAA configuration for OpenGL ES 1.1. */
        config->renderableType &= ~EGL_OPENGL_ES_BIT;
        config->conformant     &= ~EGL_OPENGL_ES_BIT;
    }

#if gcdENABLE_3D
    {
    gcePATCH_ID patchId   = gcvPATCH_INVALID;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if (patchId == gcvPATCH_GTFES30)
    {
        /* Only enable RGBA8888/RGB565 + D24S8 for OpenGL ES3 context to reduce ES3.x CTS running time */
        if (!((Color->formatFlags & VEGL_8888) == VEGL_8888 ||
              (Color->formatFlags & VEGL_565) == VEGL_565
             ) ||
            !(config->depthSize == 24 && config->stencilSize == 8)
           )
        {
            config->renderableType &= ~EGL_OPENGL_ES3_BIT_KHR;
            config->conformant     &= ~EGL_OPENGL_ES3_BIT_KHR;
        }
    }
    }
#endif

    if (!(gcoHAL_IsFeatureAvailable(NULL, gcvFEATURE_HALTI0)))
    {
        config->renderableType &= ~EGL_OPENGL_ES3_BIT_KHR;
        config->conformant     &= ~EGL_OPENGL_ES3_BIT_KHR;
    }

    config->alphaMaskSize = 8;

    config->matchFormat = (config->greenSize == 6)
                        ? EGL_FORMAT_RGB_565_EXACT_KHR
                        :   ( (config->greenSize == 8)
                            ? EGL_FORMAT_RGBA_8888_EXACT_KHR
                            : EGL_DONT_CARE
                            );

    config->matchNativePixmap = EGL_NONE;

    config->recordableConfig = EGL_TRUE;

    /* Is hardware VG pipe present? */
    if (thread->openVGpipe)
    {
        if ((Samples == 0)
        &&  (config->stencilSize ==  0)
        &&  (config->depthSize   == 16)
        &&  (  ((Color->formatFlags & VEGL_4444)  == VEGL_4444)
            || ((Color->formatFlags & VEGL_5551)  == VEGL_5551)
            || ((Color->formatFlags & VEGL_565)   == VEGL_565)
            || ((Color->formatFlags & VEGL_8888)  == VEGL_8888)
            || ((Color->formatFlags & VEGL_888)   == VEGL_888)
#if gcdENABLE_VG
            ||  ((Color->formatFlags & VEGL_ALPHA)   == VEGL_ALPHA)
#endif
            )
        )
        {
            config->conformant      |= EGL_OPENVG_BIT;

            config->renderableType  |= EGL_OPENVG_BIT;

            if (    ((Color->formatFlags & VEGL_565)   == VEGL_565)
                ||  ((Color->formatFlags & VEGL_8888)  == VEGL_8888)
                ||  ((Color->formatFlags & VEGL_888)  == VEGL_888))
            {
                config->surfaceType     |= EGL_VG_ALPHA_FORMAT_PRE_BIT
                                        |  EGL_VG_COLORSPACE_LINEAR_BIT;
            }
        }
    }

    /* No, 2D/3D implementation only. */
    else
    {
        /* Determing the number of samples required for OpenVG. */

        /* TODO: Need to get rid of depth for OpenVG. */
        if (
#if NO_STENCIL_VG
            (Depth->stencilSize == 0 && Depth->depthSize == 16) &&
#else
            (Depth->stencilSize > 0)  &&
#endif
            (((Color->formatFlags & VEGL_565)  == VEGL_565) ||
            ((Color->formatFlags & VEGL_8888) == VEGL_8888) ||
            ((Color->formatFlags & VEGL_888) == VEGL_888)))
        {
            config->renderableType |= EGL_OPENVG_BIT;

            /* Why? 3D OpenVG does not support color format with alpha channel. */
            if (((Color->formatFlags & VEGL_ALPHA) == 0) && (Samples >= thread->maxSamples))
            {
                config->conformant  |= EGL_OPENVG_BIT;
                config->surfaceType |= EGL_VG_ALPHA_FORMAT_PRE_BIT
                                    |  EGL_VG_COLORSPACE_LINEAR_BIT;
            }
        }
    }

    /* Advance to the next entry. */
    (*Index) ++;
}

static void
_SetTraceMode(
    void
    )
{
    static gctBOOL Once = gcvFALSE;

    if (!Once)
    {
        gctSTRING tracemode = gcvNULL;
        gctSTRING veglNoMtEnvVar = gcvNULL;

        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_TRACE", &tracemode)) && tracemode)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "0")))
            {
                veglTraceMode = gcvTRACEMODE_NONE;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "1")))
            {
                veglTraceMode = gcvTRACEMODE_FULL;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "2")))
            {
                veglTraceMode = gcvTRACEMODE_LOGGER;
            }
            else
            {
                gcmPRINT("EGL: unsupported trace mode");
            }

            veglInitTracerDispatchTable();
        }

        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_NO_MT", &veglNoMtEnvVar)) && veglNoMtEnvVar)
        {
            enableSwapWorker = gcvFALSE;
        }

        Once = gcvTRUE;
    }

#if VIVANTE_PROFILER
    /* single thread mode has to be used when running in profile mode */
    if (enableSwapWorker)
    {
        char *env = gcvNULL;
        gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &env);

        if ((env != gcvNULL) && (!gcmIS_SUCCESS(gcoOS_StrCmp(env, "0"))))
        {
            enableSwapWorker = gcvFALSE;

            gcoOS_GetEnv(gcvNULL, "VP_SYNC_MODE", &env);

            if ((env != gcvNULL) && (gcmIS_SUCCESS(gcoOS_StrCmp(env, "0"))))
            {
                enableSwapWorker = gcvTRUE;
            }
        }
    }
#endif

    return;
}


EGLAPI EGLint EGLAPIENTRY
eglGetError(
    void
    )
{
    VEGLThreadData thread;
    EGLint error;

    gcmHEADER();
    VEGL_TRACE_API_PRE(GetError)();

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        error = EGL_BAD_ALLOC;
    }
    else
    {
        error = thread->error;
        veglSetEGLerror(thread,  EGL_SUCCESS);
    }

    VEGL_TRACE_API_POST(GetError)(error);
    gcmDUMP_API("${EGL eglGetError := 0x%08X}", error);
    gcmFOOTER_ARG("0x%04x", error);
    return error;
}


void veglSetEGLerror(VEGLThreadData thread, EGLint error)
{
    thread->error = error;
}

static EGLDisplay
veglGetPlatformDisplay(
    EGLenum platform,
    void *native_display,
    const EGLAttrib *attrib_list
    )
{
    VEGLThreadData thread;
    VEGLDisplay display = gcvNULL;
    gctBOOL releaseDpy = gcvFALSE;
    void * nativeScreen = gcvNULL;

    /* Get shortcut. Only default platform display type supported for now. */
    EGLNativeDisplayType display_id = (EGLNativeDisplayType) native_display;

#if gcdGC355_PROFILER
    gcoOS_GetTime(&AppstartTimeusec);
    gcoOS_Print("App start at %llu(microsec)", AppstartTimeusec);
#endif

    gcoOS_LockPLS();
    _SetTraceMode();

    /* Add Necessary signal handlers */
    /* Add signal handler for SIGFPE when error no is zero. */
    gcoOS_AddSignalHandler(gcvHANDLE_SIGFPE_WHEN_SIGNAL_CODE_IS_0);
#ifdef WIN32
#pragma warning( disable: 4054)
#endif
    /* set PLS destructor to clean up display */
    gcoOS_SetPLSValue(gcePLS_VALUE_EGL_DESTRUCTOR_INFO, (gctPOINTER)&_DestroyProcessData);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        gcoOS_UnLockPLS();
        return EGL_NO_DISPLAY;
    }

    switch (platform)
    {
#if defined(ANDROID)
    case EGL_PLATFORM_ANDROID_KHR:
        /* EGL_KHR_platform_android. */
        if (display_id != EGL_DEFAULT_DISPLAY)
        {
            /* Requires EGL_DEFAULT_DISPLAY for native_display. */
            veglSetEGLerror(thread, EGL_BAD_PARAMETER);
            gcoOS_UnLockPLS();
            return EGL_NO_DISPLAY;
        }

        if ((attrib_list != gcvNULL) && (attrib_list[0] != EGL_NONE))
        {
            /* No attribute required. */
            veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
            gcoOS_UnLockPLS();
            return EGL_NO_DISPLAY;
        }
        break;
#endif

#if defined(X11)
    case EGL_PLATFORM_X11_KHR:
        /* Native display is checked later. */
        if (attrib_list != gcvNULL)
        {
            /* Check attributes. */
            EGLint i;

            for (i = 0; attrib_list[i] != EGL_NONE; i += 2)
            {
                switch (attrib_list[i])
                {
                case EGL_PLATFORM_X11_SCREEN_KHR:
                    nativeScreen = (void *) attrib_list[i+1];
                    break;
                default:
                    veglSetEGLerror(thread, EGL_BAD_ATTRIBUTE);
                    gcoOS_UnLockPLS();
                    return EGL_NO_DISPLAY;
                }
            }
        }
        break;
#endif

    case 0:
        /*
         * Default platform called by eglGetDisplay.
         * Don't pass '0' from eglGetPlatformDisplay API.
         */
        break;

    case EGL_PLATFORM_GBM_KHR:
    case EGL_PLATFORM_WAYLAND_KHR:
    default:
        /* Not supported, ie invalid. */
        veglSetEGLerror(thread, EGL_BAD_PARAMETER);
        gcoOS_UnLockPLS();
        return EGL_NO_DISPLAY;
    }

    if (display_id == EGL_DEFAULT_DISPLAY)
    {
        /* Try finding the default display inside the EGLDisplay stack. */
        for (display = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);
             display != gcvNULL;
             display = display->next)
        {
            if (display->releaseDpy)
            {
                /* Find it! */
                break;
            }
        }

        if (display == gcvNULL)
        {
            /* Get default device context for desktop window. */
            display_id = veglGetDefaultDisplay();
            releaseDpy = gcvTRUE;
        }
    }
    else
    {
        if (!veglIsValidDisplay(display_id))
        {
            gcoOS_UnLockPLS();
            return EGL_NO_DISPLAY;
        }
    }

    if (display == gcvNULL)
    {
        /* Try finding the display_id inside the EGLDisplay stack. */
        for (display = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);
             display != gcvNULL;
             display = display->next)
        {
            if (display->hdc == display_id)
            {
                /* Release DC if necessary. */
                if (releaseDpy)
                {
                    veglReleaseDefaultDisplay(display_id);
                }

                /* Got it! */
                break;
            }
        }
    }

    if (display == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;

        /* Allocate memory for eglDisplay structure. */
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                       sizeof(struct eglDisplay),
                                       &pointer)))
        {
            /* Allocation error. */
            veglSetEGLerror(thread, EGL_BAD_ALLOC);
            gcoOS_UnLockPLS();
            return EGL_NO_DISPLAY;
        }

        /* Zero memory. */
        gcoOS_ZeroMemory(pointer, sizeof (struct eglDisplay));

        display = pointer;

        /* Initialize EGLDisplay. */
        display->platform      = platform;
        display->nativeScreen  = nativeScreen;
        display->nativeDisplay = (void *) display_id;
        display->hdc           = display_id;
        display->releaseDpy    = releaseDpy;
        display->initialized   = gcvFALSE;
        display->configCount   = 0;
        display->config        = gcvNULL;
        display->surfaceStack  = gcvNULL;
        display->imageStack    = gcvNULL;
        display->imageRefStack = gcvNULL;
        display->syncStack     = gcvNULL;
        display->contextStack  = gcvNULL;
        display->process       = gcoOS_GetCurrentProcessID();
        display->ownerThread   = gcvNULL;
        display->workerThread  = gcvNULL;
        display->startSignal   = gcvNULL;
        display->stopSignal    = gcvNULL;
        display->suspendMutex  = gcvNULL;
        display->blobCacheGet  = gcvNULL;
        display->blobCacheSet  = gcvNULL;

        /* create access mutext to lock display */
        gcmVERIFY_OK(gcoOS_CreateMutex(gcvNULL, &display->accessMutex));
        /* create access mutext to lock resouce stack */
        gcmVERIFY_OK(gcoOS_CreateMutex(gcvNULL, &display->resourceMutex));
        /* Push EGLDisplay onto stack. */
        display->next = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);
        gcoOS_SetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO, (gctPOINTER) display);

    }

    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);

    gcoOS_UnLockPLS();
    return display;
}

/* EGL 1.5 */
EGLAPI EGLDisplay EGLAPIENTRY
eglGetPlatformDisplay(
    EGLenum platform,
    void *native_display,
    const EGLAttrib *attrib_list
    )
{
    EGLDisplay dpy;

    gcmHEADER_ARG("platform=0x%x native_display=0x%x attrib_list=0x%x",
                  platform, native_display, attrib_list);

    VEGL_TRACE_API_PRE(GetPlatformDisplay)(platform, native_display, attrib_list);

    if (platform == 0)
    {
        /*
         * platform '0' is used as default platform in internal
         * implementation, but it is error condition for this API.
         * Set it to another invalid value.
         */
        platform = EGL_BAD_PARAMETER;
    }

    /* Call internal function. */
    dpy = veglGetPlatformDisplay(platform, native_display, attrib_list);

    VEGL_TRACE_API_POST(GetPlatformDisplay)(platform, native_display, attrib_list, dpy);
    gcmDUMP_API("${EGL eglGetPlatformDisplay 0x%08X 0x%08X 0x%08X := 0x%08X}",
                platform, native_display, attrib_list, dpy);

    gcmFOOTER_ARG("return=0x%x", dpy);
    return dpy;
}

EGLAPI EGLDisplay EGLAPIENTRY
eglGetDisplay(
    NativeDisplayType display_id
    )
{
    EGLDisplay dpy;

    gcmHEADER_ARG("display_id=0x%x", display_id);
    VEGL_TRACE_API_PRE(GetDisplay)(display_id);

    /* Call GetPlatformDisplay with default platform. */
    dpy = veglGetPlatformDisplay(0, (void *) display_id, gcvNULL);

    VEGL_TRACE_API_POST(GetDisplay)(display_id, dpy);
    gcmDUMP_API("${EGL eglGetDisplay 0x%08X := 0x%08X}", display_id, dpy);

    gcmFOOTER_ARG("return=0x%x", dpy);
    return dpy;
}


VEGLDisplay
veglGetDisplay(
    IN EGLDisplay Display
    )
{
    VEGLDisplay dpy = (VEGLDisplay)(Display);
    VEGLDisplay stack;

    if (dpy == gcvNULL)
        return gcvNULL;

    gcoOS_LockPLS();

    /* Test for valid EGLDisplay structure. */
    for (stack = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);
         stack != gcvNULL;
         stack = stack->next)
    {
        if (stack == dpy)
        {
            break;
        }
    }

    gcoOS_UnLockPLS();

    if (stack == gcvNULL)
    {
        return gcvNULL;
    }

    return dpy;
}

static EGLBoolean
_SortAfter(
    struct eglConfig Config1,
    struct eglConfig Config2
    )
{
    EGLint bits1, bits2;

    /* Priority 1. */
    if (Config1.configCaveat < Config2.configCaveat)
    {
        return EGL_FALSE;
    }
    else if (Config1.configCaveat > Config2.configCaveat)
    {
        return EGL_TRUE;
    }

    /* Priority 2. */
    if (Config1.colorBufferType < Config2.colorBufferType)
    {
        return EGL_FALSE;
    }
    else if (Config1.colorBufferType > Config2.colorBufferType)
    {
        return EGL_TRUE;
    }

    /* Priority 3 is not applied. */

    /* Priority 4. */
    if (Config1.configBufferSize < Config2.configBufferSize)
    {
        return EGL_FALSE;
    }
    else if (Config1.configBufferSize > Config2.configBufferSize)
    {
        return EGL_TRUE;
    }

    /* Priority 5. */
    if (Config1.sampleBuffers < Config2.sampleBuffers)
    {
        return EGL_FALSE;
    }
    else if (Config1.sampleBuffers > Config2.sampleBuffers)
    {
        return EGL_TRUE;
    }

    /* Priority 6. */
    if (Config1.samples < Config2.samples)
    {
        return EGL_FALSE;
    }
    else if (Config1.samples > Config2.samples)
    {
        return EGL_TRUE;
    }

    /* Priority 7. */
    if (Config1.depthSize < Config2.depthSize)
    {
        return EGL_FALSE;
    }
    else if (Config1.depthSize > Config2.depthSize)
    {
        return EGL_TRUE;
    }

    /* Priority 8. */
    if (Config1.stencilSize < Config2.stencilSize)
    {
        return EGL_FALSE;
    }
    else if (Config1.stencilSize > Config2.stencilSize)
    {
        return EGL_TRUE;
    }

    /* Priority 9. */
    if (Config1.alphaMaskSize < Config2.alphaMaskSize)
    {
        return EGL_FALSE;
    }
    else if (Config1.alphaMaskSize > Config2.alphaMaskSize)
    {
        return EGL_TRUE;
    }

    /* Compute native visual type sorting. */
    bits1 = (Config1.nativeVisualType == EGL_NONE)
        ? 0
        : Config1.nativeVisualType;
    bits2 = (Config2.nativeVisualType == EGL_NONE)
        ? 0
        : Config2.nativeVisualType;

    /* Priority 10. */
    if (bits1 < bits2)
    {
        return EGL_FALSE;
    }
    else if (bits1 > bits2)
    {
        return EGL_TRUE;
    }

    /* Priority 11. */
    if (Config1.configId < Config2.configId)
    {
        return EGL_FALSE;
    }
    else if (Config1.configId > Config2.configId)
    {
        return EGL_TRUE;
    }

    /* Nothing to sort. */
    return EGL_FALSE;
}

static void
_Sort(
    VEGLConfig Config,
    EGLint ConfigCount
    )
{
    EGLBoolean swapped;
    EGLint i;

    do
    {
        /* Assume no sorting has happened. */
        swapped = EGL_FALSE;

        /* Loop through all configurations. */
        for (i = 0; i < ConfigCount - 1; i++)
        {
            /* Do we need to swap the current and next configuration? */
            if (_SortAfter(Config[i], Config[i+1]))
            {
                /* Swap configurations. */
                struct eglConfig temp = Config[i];
                Config[i] = Config[i+1];
                Config[i+1] = temp;

                /* We need another pass. */
                swapped = EGL_TRUE;
            }
        }
    }
    while (swapped);
}

static EGLBoolean
veglInitilizeDisplay(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display
    )
{
    gctUINT color, depth;
    gctINT index;
    EGLint i;
    gceSTATUS status;

    if (Display->initialized)
        return EGL_TRUE;

    do
    {
        gctPOINTER pointer = gcvNULL;

        /* Count the number of valid configurations. */
        gcmASSERT(Display->configCount == 0);
        for (color = 0; color < gcmCOUNTOF(eglConfigColor); color++)
        {
            for (depth = 0; depth < gcmCOUNTOF(eglConfigDepth); depth++)
            {
                EGLBoolean validMode;
                EGLint maxMultiSample = 0;
                EGLint fastMSAA = 0;

                validMode = _ValidateMode(
                    Display->hdc,
                    &eglConfigColor[color],
                    &eglConfigDepth[depth],
                    &maxMultiSample,
                    &fastMSAA
                    );

                if (!validMode)
                {
                    continue;
                }

#if VEGL_DUPLICATE_ABGR_EGL_CONFIGS
                /* Duplicate config to return BGRA visual. */
                if ((eglConfigColor[color].formatFlags & VEGL_8888) == VEGL_8888)
                {
                    Display->configCount ++;
                }
#endif

                switch (maxMultiSample)
                {
                case 4:
                    Display->configCount ++;

                    /* Fall through. */
#if VEGL_ENABLE_2xAA_CONFIG
                case 2:
                    if (!fastMSAA)
                    {
                        Display->configCount ++;
                    }
                    /* Fall through. */
#endif

                default:
                    Display->configCount ++;
                }
            }
        }

        gcmASSERT(Display->config == gcvNULL);
        /* Allocate configuration buffer. */
        gcmERR_BREAK(gcoOS_Allocate(
            gcvNULL,
            (gctSIZE_T) Display->configCount * sizeof(struct eglConfig),
            &pointer
            ));

        Display->config = pointer;

        /* Start at beginning of configuration buffer. */
        index = 0;

        /* Fill in the configuration table. */
        for (color = 0; color < gcmCOUNTOF(eglConfigColor); color++)
        {
            for (depth = 0; depth < gcmCOUNTOF(eglConfigDepth); depth++)
            {
                EGLBoolean validMode;
                EGLint maxMultiSample = 0;
                EGLint fastMSAA = 0;

                validMode = _ValidateMode(
                    Display->hdc,
                    &eglConfigColor[color],
                    &eglConfigDepth[depth],
                    &maxMultiSample,
                    &fastMSAA
                    );

                if (!validMode)
                {
                    continue;
                }

                _FillIn(
                    Display->config,
                    &index,
                    &eglConfigColor[color],
                    &eglConfigDepth[depth],
                    0);

#if VEGL_DUPLICATE_ABGR_EGL_CONFIGS
                /* Duplicate config to return BGRA visual. */
                if ((eglConfigColor[color].formatFlags & VEGL_8888) == VEGL_8888)
                {
                    Display->config[index] = Display->config[index - 1];
                    Display->config[index].swizzleRB = EGL_TRUE;

                    index++;
                }
#endif

#if VEGL_ENABLE_2xAA_CONFIG
                if (!fastMSAA && maxMultiSample >= 2)
                {
                    _FillIn(
                        Display->config,
                        &index,
                        &eglConfigColor[color],
                        &eglConfigDepth[depth],
                        2);
                }
#endif

                if (maxMultiSample >= 4)
                {
                    _FillIn(
                        Display->config,
                        &index,
                        &eglConfigColor[color],
                        &eglConfigDepth[depth],
                        4);
                }
            }
        }

        if ((Display->configCount > 1) && (Display->config != gcvNULL))
        {
            /* Sort the configurations. */
            _Sort(Display->config, Display->configCount);
            for (i = 0; i < Display->configCount; i++)
            {
                (Display->config + i)->configId = i + 1;

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "EGL CONFIGURATION:"
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  config index=%d; config ID=%d; caviat=0x%X",
                    i, (Display->config + i)->configId, (Display->config + i)->configCaveat
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  surface type=0x%X; color buffer type=0x%X; renderable type=0x%X",
                    (Display->config + i)->surfaceType, (Display->config + i)->colorBufferType,
                    (Display->config + i)->renderableType
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  buffer size=%d",
                    (Display->config + i)->bufferSize
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  RGBA sizes=%d, %d, %d, %d; depth=%d; stencil=%d",
                    (Display->config + i)->redSize, (Display->config + i)->greenSize,
                    (Display->config + i)->blueSize, (Display->config + i)->alphaSize,
                    (Display->config + i)->depthSize, (Display->config + i)->stencilSize
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  luminance size=%d; alpha mask size=%d",
                    (Display->config + i)->luminanceSize, (Display->config + i)->alphaMaskSize
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  native renderable=%d; native visual type=%d",
                    (Display->config + i)->nativeRenderable, (Display->config + i)->nativeVisualType
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  samples=%d; sample buffers=%d",
                    (Display->config + i)->samples, (Display->config + i)->sampleBuffers
                    );

                gcmTRACE_ZONE(
                    gcvLEVEL_VERBOSE, gcdZONE_EGL_INIT,
                    "  bind to tetxure RGB=%d; bind to tetxure RGBA=%d",
                    (Display->config + i)->bindToTetxureRGB, (Display->config + i)->bindToTetxureRGBA
                    );
            }
        }

        /* Create worker thread. */
        do
        {
            gcmASSERT(Display->startSignal == gcvNULL);
            /* Create thread start signal. */
            gcmERR_BREAK(gcoOS_CreateSignal(
                gcvNULL,
                gcvFALSE,
                &Display->startSignal
                ));

            gcmTRACE_ZONE(
                gcvLEVEL_INFO, gcvZONE_SIGNAL,
                "%s(%d): start signal created 0x%08X",
                __FUNCTION__, __LINE__,
                Display->startSignal
                );

            gcmASSERT(Display->stopSignal == gcvNULL);
            /* Create thread stop signal. */
            gcmERR_BREAK(gcoOS_CreateSignal(
                gcvNULL,
                gcvTRUE,
                &Display->stopSignal
                ));

            gcmTRACE_ZONE(
                gcvLEVEL_INFO, gcvZONE_SIGNAL,
                "%s(%d): stop signal created 0x%08X",
                __FUNCTION__, __LINE__,
                Display->stopSignal
                );

            gcmASSERT(Display->suspendMutex == gcvNULL);
            /* Create thread lock signal. */
            gcmERR_BREAK(gcoOS_CreateMutex(
                gcvNULL,
                &Display->suspendMutex
                ));

            /* No workers yet. */
            Display->workerSentinel.draw = gcvNULL;
            Display->workerSentinel.prev   = &Display->workerSentinel;
            Display->workerSentinel.next   = &Display->workerSentinel;

            gcmASSERT(Display->workerThread == gcvNULL);

#if defined(X11) || (defined(X_PROTOCOL) && X_PROTOCOL)
            Display->workerThread = gcvNULL;
#elif gcdANDROID_NATIVE_FENCE_SYNC >= 2
            /*
             * Swap worker is not required when android native fence sync
             * is enabled in app side.
             */
            Display->workerThread = gcvNULL;
#else
            /* Start the thread. */
            if (enableSwapWorker)
            {
                gcmERR_BREAK(gcoOS_CreateThread(
                    gcvNULL, veglSwapWorker, Display, &Display->workerThread
                    ));
            }
#endif

#if gcdENABLE_TIMEOUT_DETECTION
            /* Setup the swap thread signal timeout handler. */
            gcmVERIFY_OK(gcoOS_InstallTimeoutCallback(
                _SwapSignalTimeoutCallback, Display
                ));
#endif
        }
        while (gcvFALSE);

        if (!veglInitLocalDisplayInfo(Display))
        {
            /* Failed. */
            break;
        }

        Display->initialized = gcvTRUE;

        /* Success. */
        return EGL_TRUE;
    }
    while (gcvFALSE);

    if (Display->config != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Display->config));
        Display->configCount = 0;
    }

    /* Error. */
    return EGL_FALSE;
}

static void
veglTerminateDisplay(
    IN VEGLThreadData Thread,
    IN VEGLDisplay Display
    )
{
    /* Stop the thread. */
    if (Display->workerThread != gcvNULL)
    {
        /* Set thread's stop signal. */
        gcmASSERT(Display->stopSignal != gcvNULL);
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL,
            Display->stopSignal,
            gcvTRUE
            ));

        /* Set thread's start signal. */
        gcmASSERT(Display->startSignal != gcvNULL);
        gcmVERIFY_OK(gcoOS_Signal(
            gcvNULL,
            Display->startSignal,
            gcvTRUE
            ));

        /* Wait until the thread is closed. */
        gcmVERIFY_OK(gcoOS_CloseThread(
            gcvNULL, Display->workerThread
            ));
        Display->workerThread = gcvNULL;
    }

#if gcdENABLE_TIMEOUT_DETECTION
    /* Remove the default timeout handler. */
    gcmVERIFY_OK(gcoOS_RemoveTimeoutCallback(
        _SwapSignalTimeoutCallback
        ));
#endif

    /* Delete the start signal. */
    if (Display->startSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Display->startSignal));
        Display->startSignal = gcvNULL;
    }

    /* Delete the stop signal. */
    if (Display->stopSignal != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, Display->stopSignal));
        Display->stopSignal = gcvNULL;
    }

    /* Delete the mutex. */
    if (Display->suspendMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, Display->suspendMutex));
        Display->suspendMutex = gcvNULL;
    }

    VEGL_LOCK_DISPLAY_RESOURCE(Display);

    while (Display->contextStack != gcvNULL)
    {
        VEGLContext ctx = (VEGLContext) Display->contextStack->resObj.next;
        veglDestroyContext(Display, Display->contextStack);

        /* Remove context from stack to mark it as invalid. */
        Display->contextStack = ctx;
    }

    /* Dereference all surfaces. */
    while (Display->surfaceStack != gcvNULL)
    {
        VEGLSurface surface = (VEGLSurface) Display->surfaceStack->resObj.next;
        veglDestroySurface(Display, Display->surfaceStack);

        /* Remove context from stack to mark it as invalid. */
        Display->surfaceStack = surface;
    }

    /* Destroy the imageStack. */
    while (Display->imageStack != gcvNULL)
    {
        VEGLImage image     = Display->imageStack;
        Display->imageStack = image->next;

        veglDestroyImage(Thread, Display, image);
    }

    /* Destroy the imageRefStack. */
    while (Display->imageRefStack != gcvNULL)
    {
        VEGLImageRef ref       = Display->imageRefStack;
        Display->imageRefStack = ref->next;

        if (ref->surface != gcvNULL)
        {
            gcmVERIFY_OK(gcoSURF_Destroy(ref->surface));
            ref->surface = gcvNULL;
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, ref));
    }

    while (Display->syncStack != gcvNULL)
    {
        VEGLSync sync = (VEGLSync)Display->syncStack->resObj.next;
        veglDestroySync(Display, Display->syncStack);
        Display->syncStack = sync;
    }

    VEGL_UNLOCK_DISPLAY_RESOURCE(Display);

    /* Free the configuration buffer. */
    if (Display->config != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Display->config));
        Display->configCount = 0;
    }

    veglDeinitLocalDisplayInfo(Display);

    Display->initialized = gcvFALSE;

    return;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglInitialize(
    EGLDisplay Dpy,
    EGLint *major,
    EGLint *minor
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    gcmHEADER_ARG("Dpy=0x%x", Dpy);

    gcmDUMP_API("${EGL eglInitialize 0x%08X}", Dpy);
    VEGL_TRACE_API(Initialize)(Dpy, major, minor);

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

    VEGL_LOCK_DISPLAY(dpy);

    do
    {
        if (!veglInitilizeDisplay(thread, dpy))
        {
            /* Not initialized. */
            veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
            break;
        }

        /* App can call eglTerminate then call eglInitialize so dpy->hdc needs to be initialized */
        if (dpy->hdc == EGL_DEFAULT_DISPLAY)
        {
            /* Get default device context for desktop window. */
            dpy->hdc = veglGetDefaultDisplay();
            dpy->releaseDpy = gcvTRUE;
        }

        if (major != gcvNULL)
        {
            /* EGL 1.5 specification. */
            *major = 1;
        }

        if (minor != gcvNULL)
        {
            /* EGL 1.5 specification. */
            *minor = 5;
        }

        /* Success. */
        veglSetEGLerror(thread,  EGL_SUCCESS);

        VEGL_UNLOCK_DISPLAY(dpy);
        gcmFOOTER_ARG("*major=%d *minor=%d",
                      gcmOPT_VALUE(major), gcmOPT_VALUE(minor));

        return EGL_TRUE;

    }
    while(gcvFALSE);

    VEGL_UNLOCK_DISPLAY(dpy);
    gcmFOOTER_ARG("%d", EGL_FALSE);

    return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglTerminate(
    EGLDisplay Dpy
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x", Dpy);
    gcmDUMP_API("${EGL eglTerminate 0x%08X}", Dpy);
    VEGL_TRACE_API(Terminate)(Dpy);

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

    VEGL_LOCK_DISPLAY(dpy);

    /* Execute any left-overs in the command buffer. */
    gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));

    /* Release DC if necessary. */
    if (dpy->releaseDpy)
    {
        veglReleaseDefaultDisplay(dpy->hdc);

        /* This display should never be found again.*/
        dpy->hdc = EGL_DEFAULT_DISPLAY;

        dpy->releaseDpy = gcvFALSE;
    }

    /* Reset blob cache call back functions. */
    dpy->blobCacheGet = gcvNULL;
    dpy->blobCacheSet = gcvNULL;

#if defined(LINUX) && gcdGC355_MEM_PRINT
    gcmPRINT("03) Frame buffer  : %d \n", thread->fbMemSize);
#endif

    if (dpy->initialized)
    {
        veglTerminateDisplay(thread, dpy);

        /* Execute events accumulated in the buffer if any. */
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvTRUE));
    }

    /* uninstall the owner thread */
    dpy->ownerThread = gcvNULL;

    VEGL_UNLOCK_DISPLAY(dpy);
    /* Success. */
    veglSetEGLerror(thread,  EGL_SUCCESS);
    gcmFOOTER_NO();
    return EGL_TRUE;

OnError:
    VEGL_UNLOCK_DISPLAY(dpy);
    veglSetEGLerror(thread, EGL_FALSE);
    gcmFOOTER();
    return EGL_FALSE;
}

EGLAPI const char * EGLAPIENTRY
eglQueryString(
    EGLDisplay Dpy,
    EGLint name
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    const char * ptr;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x name=0x%04x", Dpy, name);

    VEGL_TRACE_API_PRE(QueryString)(Dpy, name);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        veglSetEGLerror(thread,  EGL_BAD_ALLOC);
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    if (Dpy == EGL_NO_DISPLAY)
    {
        switch (name)
        {
        case EGL_EXTENSIONS:
            /* client extensions. */
            ptr = clientExtension;
            break;

        case EGL_VERSION:
            ptr = "1.5";
            break;

        default:
            /* Bad display. */
            ptr = gcvNULL;
            veglSetEGLerror(thread,  EGL_BAD_DISPLAY);;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
    else
    {
        /* Test for valid EGLDisplay structure. */
        dpy = veglGetDisplay(Dpy);
        if (dpy == gcvNULL)
        {
            /* Bad display. */
            veglSetEGLerror(thread,  EGL_BAD_DISPLAY);
            gcmFOOTER_ARG("0x%x", gcvNULL);
            return gcvNULL;
        }

        /* Check for reference. */
        if (!dpy->initialized)
        {
            veglSetEGLerror(thread,  EGL_NOT_INITIALIZED);;
            gcmONERROR(gcvSTATUS_INVALID_DATA);
        }

        switch (name)
        {
        case EGL_CLIENT_APIS:
            ptr = "OpenGL_ES OpenVG";
            break;

        case EGL_EXTENSIONS:
            ptr = extension;
            break;

        case EGL_VENDOR:
            ptr = "Vivante Corporation";
            break;

        case EGL_VERSION:
            ptr = "1.5";
            break;

        default:
            /* Bad parameter. */
            ptr = gcvNULL;
            veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    veglSetEGLerror(thread,  EGL_SUCCESS);

    VEGL_TRACE_API_POST(QueryString)(Dpy, name, ptr);
    gcmDUMP_API("${EGL eglQueryString 0x%08X 0x%08X :=", Dpy, name);
    gcmDUMP_API_DATA(ptr, 0);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("%s", ptr);
    return ptr;

OnError:
    gcmFOOTER_ARG("0x%x", gcvNULL);
    return gcvNULL;
}

/*
    If eglSetBlobCacheFuncsANDROID generates an error then all client APIs must
    behave as though eglSetBlobCacheFuncsANDROID was not called for the display
    <dpy>.  If <set> or <get> is NULL then an EGL_BAD_PARAMETER error is
    generated.  If a successful eglSetBlobCacheFuncsANDROID call was already
    made for <dpy> and the display has not since been terminated then an
    EGL_BAD_PARAMETER error is generated.
*/
EGLAPI void EGLAPIENTRY
eglSetBlobCacheFuncsANDROID(
    EGLDisplay Dpy,
    EGLSetBlobFuncANDROID Set,
    EGLGetBlobFuncANDROID Get
    )
{

    VEGLThreadData thread;
    VEGLDisplay dpy;

    gcmHEADER_ARG("Dpy=0x%x Set=0x%x Get=0x%x", Dpy, Set, Get);
    gcmDUMP_API("${EGL eglSetBlobCacheFuncsANDROID 0x%08X 0x%08X 0x%08X}", Dpy, Set, Get);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_NO();
        return;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): VEGL_DISPLAY failed.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_NO();
        return;
    }

    do
    {
        /* Test for input functions.*/
        if (Set == gcvNULL || Get == gcvNULL)
        {
            break;
        }

        /* Test if set/get functions are already called for the display.*/
        if ((dpy->blobCacheGet != gcvNULL) || (dpy->blobCacheSet != gcvNULL))
        {
            break;
        }
        dpy->blobCacheGet = Get;
        dpy->blobCacheSet = Set;

        veglSetEGLerror(thread,  EGL_SUCCESS);
        gcmFOOTER_NO();
        return;

    }while(gcvFALSE);

    /* set/get functions are already called. */
    veglSetEGLerror(thread,  EGL_BAD_PARAMETER);;
    gcmFOOTER_NO();
    return;
}

EGLsizeiANDROID veglGetBlobCache(
    const void* Key,
    EGLsizeiANDROID KeySize,
    void* Value,
    EGLsizeiANDROID ValueSize
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    EGLGetBlobFuncANDROID get;
    EGLsizeiANDROID result;
    gcmHEADER_ARG("Key=0x%x KeySize=0x%x Value=0x%x ValueSize=0x%x", Key, KeySize, Value, ValueSize);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return 0;
    }

    if (thread->context == EGL_NO_CONTEXT)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): thread->context is EGL_NO_CONTEXT.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return 0;
    }


    dpy = thread->context->display;
    if (dpy == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): thread->context->display is gcvNULL.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return 0;
    }

    get = dpy->blobCacheGet;
    if (get == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): dpy->blobCacheGet is gcvNULL.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return 0;
    }

    result = get(Key, KeySize, Value, ValueSize);

    gcmDUMP_API("${EGL veglGetBlobCache 0x%x 0x%x 0x%x 0x%x 0x%x :=", result, ValueSize, Value, KeySize, Key);
    gcmDUMP_API_DATA(Key, KeySize);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", result);
    return result;
}


void veglSetBlobCache(
    const void* Key,
    EGLsizeiANDROID KeySize,
    const void* Value,
    EGLsizeiANDROID ValueSize
    )
{

    VEGLThreadData thread;
    VEGLDisplay dpy;
    EGLSetBlobFuncANDROID set;

    gcmHEADER_ARG("Key=0x%x KeySize=0x%x Value=0x%x ValueSize=0x%x", Key, KeySize, Value, ValueSize);
    gcmDUMP_API("${EGL veglSetBlobCache 0x%x 0x%x 0x%x 0x%x :=", ValueSize, Value, KeySize, Key);
    gcmDUMP_API_DATA(Key, KeySize);
    gcmDUMP_API("$}");

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return;
    }

    if (thread->context == EGL_NO_CONTEXT)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): thread->context is EGL_NO_CONTEXT.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return;
    }


    dpy = thread->context->display;
    if (dpy == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): thread->context->display is gcvNULL.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return;
    }

    set = dpy->blobCacheSet;
    if (set == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): dpy->blobCacheGet is gcvNULL.",
            __FUNCTION__, __LINE__
            );
        gcmFOOTER_ARG("return=0x%x", 0);
        return;
    }

    set(Key, KeySize, Value, ValueSize);

    gcmFOOTER_NO();
}

