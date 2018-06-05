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
#include <stdio.h>
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
#include <pthread.h>
#endif

#define _GC_OBJ_ZONE            gcdZONE_EGL_API

#if !gcdSTATIC_LINK
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
static pthread_mutex_t client_handles_lock = PTHREAD_MUTEX_INITIALIZER;

static gctHANDLE client_handles[vegl_API_LAST] = {NULL};

static gctCONST_STRING _dispatchNames[] =
{
    "",                                 /* EGL */
    "GLES_CL_DISPATCH_TABLE",           /* OpenGL ES 1.1 Common Lite */
    "GLES_CM_DISPATCH_TABLE",           /* OpenGL ES 1.1 Common */
    "GLESv2_DISPATCH_TABLE",            /* OpenGL ES 2.0 */
    "GLESv2_DISPATCH_TABLE",            /* OpenGL ES 3.0 */
    "OpenVG_DISPATCH_TABLE",            /* OpenVG 1.0 */
};
#endif
#endif
/*******************************************************************************
***** Version Signature *******************************************************/

#define _gcmTXT2STR(t) #t
#define  gcmTXT2STR(t) _gcmTXT2STR(t)

const char * _EGL_VERSION = "\n\0$VERSION$"
                            gcmTXT2STR(gcvVERSION_MAJOR) "."
                            gcmTXT2STR(gcvVERSION_MINOR) "."
                            gcmTXT2STR(gcvVERSION_PATCH) ":"
                            gcmTXT2STR(gcvVERSION_BUILD) "$\n";

static void
_DestroyThreadData(
    gcsDRIVER_TLS_PTR TLS
    )
{
    gctBOOL releaseThread = gcvFALSE;
    VEGLThreadData thread;
    gcmHEADER_ARG("TLS=0x%x", TLS);

    thread = (VEGLThreadData) TLS;
    if (thread != gcvNULL)
    {
        VEGLDisplay head;

        gcoOS_LockPLS();

        head = (VEGLDisplay) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_DISPLAY_INFO);

        while (head != EGL_NO_DISPLAY)
        {
            VEGLDisplay display = head;
            head = display->next;

            if (display->ownerThread == gcvNULL || display->ownerThread == gcoOS_GetCurrentThreadID())
            {
                releaseThread = gcvTRUE;

                break;
            }
        }

        gcoOS_UnLockPLS();

        if (releaseThread)
        {
            /* Release current thread data. */
            veglReleaseThread(thread);

            /* Clean up the ES private data */
            if (thread->esPrivate && thread->destroyESPrivate)
            {
                thread->destroyESPrivate(thread->esPrivate);
                thread->esPrivate = gcvNULL;
                thread->destroyESPrivate = gcvNULL;
            }
        }

        if (thread->clientExtString)
        {
            gcoOS_Free(gcvNULL, thread->clientExtString);
            thread->clientExtString = gcvNULL;
        }

        gcoOS_Free(gcvNULL, thread);
    }

    gcmFOOTER_NO();
}

VEGLResObj veglGetResObj(
    IN VEGLDisplay Dpy,
    IN VEGLResObj *pResHead,
    IN EGLResObj   ResObj,
    IN gctUINT     ResSig
    )
{
    VEGLResObj resHead;
    VEGLResObj resObj = VEGL_RESOBJ(ResObj);
    VEGLResObj iterObj;

    if (resObj == gcvNULL)
    {
        return gcvNULL;
    }

    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    resHead = *pResHead;

    /* Test for valid EGLResObj structure */
    for (iterObj = resHead; iterObj != gcvNULL; iterObj = iterObj->next)
    {
        if (resObj == iterObj)
        {
            break;
        }
    }

    if (iterObj == gcvNULL || iterObj->signature != ResSig)
    {
        iterObj = gcvNULL;
    }

    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

    return iterObj;
}


void veglPushResObj(
    IN VEGLDisplay Dpy,
    INOUT VEGLResObj *pResHead,
    IN VEGLResObj ResObj
    )
{
    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);
    ResObj->next = *pResHead;
    *pResHead = ResObj;
    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

    return;
}


void veglPopResObj(
    IN VEGLDisplay Dpy,
    INOUT VEGLResObj *pResHead,
    IN VEGLResObj ResObj
    )
{
    VEGL_LOCK_DISPLAY_RESOURCE(Dpy);

    if (ResObj == *pResHead)
    {
        /* Simple - it is the top of the stack. */
        *pResHead = ResObj->next;
    }
    else
    {
        VEGLResObj stack;
        /* Walk the stack. */
        for (stack = *pResHead; stack != gcvNULL; stack = stack->next)
        {
            /* Check if the next context on the stack is ours. */
            if (stack->next == ResObj)
            {
                /* Pop the context from the stack. */
                stack->next = ResObj->next;
                break;
            }
        }
    }

    VEGL_UNLOCK_DISPLAY_RESOURCE(Dpy);

    return;
}

static void
_InitDispatchTables(
    VEGLThreadData Thread
    )
{
#if gcdSTATIC_LINK
#if gcdENABLE_3D
    extern veglDISPATCH GLES_CM_DISPATCH_TABLE;
    extern veglDISPATCH GLESv2_DISPATCH_TABLE;
#  endif
#ifndef VIVANTE_NO_VG
    extern veglDISPATCH OpenVG_DISPATCH_TABLE;
#  endif

    Thread->dispatchTables[vegl_EGL]            = gcvNULL;
#if gcdENABLE_3D
    Thread->dispatchTables[vegl_OPENGL_ES11_CL] = gcvNULL;
    Thread->dispatchTables[vegl_OPENGL_ES11]    = &GLES_CM_DISPATCH_TABLE;
    Thread->dispatchTables[vegl_OPENGL_ES20]    = &GLESv2_DISPATCH_TABLE;
    Thread->dispatchTables[vegl_OPENGL_ES30]    = &GLESv2_DISPATCH_TABLE;
#  endif
#ifndef VIVANTE_NO_VG
    Thread->dispatchTables[vegl_OPENVG]         = &OpenVG_DISPATCH_TABLE;
#  endif
#else
    gctSIZE_T i;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
    gceSTATUS status;
#endif

    for (i = 0; i < vegl_API_LAST; i++)
    {
        veglAPIINDEX index = (veglAPIINDEX) i;
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
        gcmONERROR(gcoOS_AcquireMutex(gcvNULL, &client_handles_lock, gcvINFINITE));

        if (client_handles[i] == NULL)
        {
            client_handles[i] = Thread->clientHandles[i] =
                    veglGetModule(gcvNULL, index, _dispatchNames[i], &Thread->dispatchTables[i]);
        }
        else
        {
            Thread->clientHandles[i] = client_handles[i];
            gcoOS_GetProcAddress(NULL, client_handles[i], _dispatchNames[i],
                    (gctPOINTER*) &Thread->dispatchTables[i]);
        }

        gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, &client_handles_lock));
#else
        Thread->clientHandles[i] =
            veglGetModule(gcvNULL, index, &Thread->dispatchTables[i]);
#endif
        gcmTRACE_ZONE(
            gcvLEVEL_VERBOSE, gcdZONE_EGL_API,
            "%s(%d): APIIndex=%d library=%p dispatch=%p",
            __FUNCTION__, __LINE__,
            index, Thread->clientHandles[i], Thread->dispatchTables[i]
            );
    }
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__)
OnError:
    return;
#endif
#endif
}


VEGLThreadData
veglGetThreadData(
    void
    )
{
    VEGLThreadData thread = gcvNULL;
    gceSTATUS status;
    gcmHEADER();

    gcmONERROR(
        gcoOS_GetDriverTLS(gcvTLS_KEY_EGL, (gcsDRIVER_TLS_PTR *) &thread));

    if (thread == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;

        gcmONERROR(gcoOS_Allocate(
            gcvNULL, gcmSIZEOF(struct eglThreadData), &pointer
            ));

        gcoOS_ZeroMemory(
            pointer, gcmSIZEOF(struct eglThreadData)
            );

        /* Cast the pointer. */
        thread = (VEGLThreadData) pointer;

        /* Initialize TLS record. */
        thread->base.destructor   = _DestroyThreadData;

        /* Initialize the thread data. */
        thread->error             = EGL_SUCCESS;
        thread->api               = EGL_OPENGL_ES_API;
        thread->worker            = gcvNULL;

#if gcdGC355_MEM_PRINT
        thread->fbMemSize         = 0;
#endif

#if veglUSE_HAL_DUMP
        /* Get the gcoDUMP object. */
        gcmONERROR(gcoHAL_GetDump(thread->hal, &thread->dump));
#endif

        /* Initialize client dispatch tables. */
        _InitDispatchTables(thread);

        /* Set driver tls. */
        gcoOS_SetDriverTLS(gcvTLS_KEY_EGL, &thread->base);
    }

    /* Return pointer to thread data. */
    gcmFOOTER_ARG("0x%x", thread);
    return thread;

OnError:
    /* Roll back. */
    if (thread != gcvNULL)
    {
        _DestroyThreadData(&thread->base);
    }

    /* Return error. */
    gcmFOOTER_ARG("0x%x", gcvNULL);
    return gcvNULL;
}

EGLBoolean
veglInitDeviceThreadData(
    VEGLThreadData Thread
    )
{
    gctINT32 i;

    if (Thread->chipCount > 0)
    {
        return EGL_TRUE;
    }

    /* Set to 3D hardware by default. */
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    if (gcmIS_ERROR(gcoHAL_QueryChipCount(gcvNULL, &Thread->chipCount)))
    {
        return EGL_FALSE;
    }

    for (i = 0; i < Thread->chipCount; i++)
    {
        if (gcvSTATUS_OK != gcoHAL_QueryChipLimits(gcvNULL, i, &Thread->chipLimits[i]))
        {
            Thread->chipLimits[i].maxWidth = 0;
            Thread->chipLimits[i].maxHeight = 0;
            Thread->chipLimits[i].maxSamples = 0;
        }
    }

    for (i = 0; i < Thread->chipCount; i++)
    {
        Thread->maxWidth =
            gcmMAX(Thread->maxWidth, (gctINT32)Thread->chipLimits[i].maxWidth);

        Thread->maxHeight =
            gcmMAX(Thread->maxHeight, (gctINT32)Thread->chipLimits[i].maxHeight);

        Thread->maxSamples =
            gcmMAX(Thread->maxSamples, (gctINT32)Thread->chipLimits[i].maxSamples);
    }

#if gcdENABLE_VG
    for (i = 0; i < Thread->chipCount; i++)
    {
        if (gcoHAL_QueryChipFeature(gcvNULL, i, gcvFEATURE_PIPE_VG))
        {
            veglDISPATCH * dispatch = Thread->dispatchTables[vegl_OPENVG];

            if (dispatch && dispatch->queryHWVG)
            {
                Thread->openVGpipe = (*dispatch->queryHWVG)();
            }
            break;
        }
    }
#endif

    /* Query fastMSAA support. */
    for (i = 0; i < Thread->chipCount; i++)
    {
        if ((gcoHAL_QueryChipFeature(gcvNULL, i, gcvFEATURE_FAST_MSAA)) ||
            (gcoHAL_QueryChipFeature(gcvNULL, i, gcvFEATURE_SMALL_MSAA)))
        {
            Thread->fastMSAA = gcvTRUE;
            break;
        }
    }

    for (i = 0; i < Thread->chipCount; i++)
    {
        if (gcoHAL_QueryChipFeature(gcvNULL, i, gcvFEATURE_SECURITY))
        {
            Thread->security = gcvTRUE;
            break;
        }
    }

    gcmTRACE_ZONE(
        gcvLEVEL_VERBOSE, gcdZONE_EGL_API,
        "%s(%d): maxWidth=%d maxHeight=%d maxSamples=%d fastmsaa=%d openVG=%d",
        __FUNCTION__, __LINE__,
        Thread->maxWidth, Thread->maxHeight, Thread->maxSamples,
        Thread->fastMSAA, Thread->openVGpipe
        );

    return EGL_TRUE;
}

#define EGL_LOG_API(...)  gcmPRINT(__VA_ARGS__)

EGLint LOG_eglGetError_pre(void)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetError_pre\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());
    return 0;
}

EGLint LOG_eglGetError_post(EGLint err)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetError_post() = 0x%04x\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), err);
    return 0;
}

EGLDisplay LOG_eglGetDisplay_pre(EGLNativeDisplayType display_id)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetDisplay_pre 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)display_id);

    return gcvNULL;
}

EGLDisplay LOG_eglGetDisplay_post(EGLNativeDisplayType display_id, EGLDisplay ret_disp)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetDisplay_post 0x%08X => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)display_id, (EGLint)(gctUINTPTR_T)ret_disp);

    return gcvNULL;
}

EGLBoolean LOG_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    EGL_LOG_API("EGL(tid=0x%x): eglInitialize 0x%08X (0x%08X) (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)major, (EGLint)(gctUINTPTR_T)minor);

    return EGL_TRUE;
}

EGLBoolean LOG_eglTerminate(EGLDisplay dpy)
{
    EGL_LOG_API("EGL(tid=0x%x): eglTerminate 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy);
    return EGL_TRUE;
}

const char * LOG_eglQueryString_pre(EGLDisplay dpy, EGLint name)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryString_pre 0x%08X 0x%08X\n",
                 (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, name);
    return gcvNULL;
}

const char * LOG_eglQueryString_post(EGLDisplay dpy, EGLint name, const char* str)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryString_post 0x%08X 0x%08X = \n%s\n",
                 (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, name, str);
    return gcvNULL;
}

EGLBoolean LOG_eglGetConfigs_pre(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetConfigs_pre 0x%08X (0x%08X) 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)configs,
                config_size, (EGLint)(gctUINTPTR_T)num_config);

    return EGL_TRUE;
}

EGLBoolean LOG_eglGetConfigs_post(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetConfigs_post 0x%08X (0x%08X) 0x%08X %d\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)configs,
                config_size, *num_config);

    return EGL_TRUE;
}

EGLBoolean LOG_eglChooseConfig_pre(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EGL_LOG_API("EGL(tid=0x%x): eglChooseConfig_pre 0x%08X (0x%08X) (0x%08X) 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)configs, config_size, (EGLint)(gctUINTPTR_T)num_config);

    return EGL_TRUE;
}

EGLBoolean LOG_eglChooseConfig_post(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EGL_LOG_API("EGL(tid=0x%x): eglChooseConfig_post 0x%08X (0x%08X) (0x%08X) 0x%08X %d\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)configs, config_size, *num_config);

    return EGL_TRUE;
}

EGLBoolean LOG_eglGetConfigAttrib_pre(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetConfigAttrib_pre 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config, attribute, *value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglGetConfigAttrib_post(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetConfigAttrib_post 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config, attribute, *value);

    return EGL_TRUE;
}


EGLSurface LOG_eglCreateWindowSurface_pre(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateWindowSurface_pre 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)win, (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLSurface LOG_eglCreateWindowSurface_post(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGLint width  = ret_surface ? ((VEGLSurface)ret_surface)->config.width  : 0;
    EGLint height = ret_surface ? ((VEGLSurface)ret_surface)->config.height : 0;

    EGL_LOG_API("EGL(tid=0x%x): eglCreateWindowSurface_post 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X (%d x %d)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)win, (EGLint)(gctUINTPTR_T)attrib_list, (EGLint)(gctUINTPTR_T)ret_surface,
                width, height);

    return gcvNULL;
}

EGLSurface LOG_eglCreatePbufferSurface_pre(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePbufferSurface_pre 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return NULL;
}

EGLSurface LOG_eglCreatePbufferSurface_post(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGLint width  = ret_surface ? ((VEGLSurface)ret_surface)->config.width  : 0;
    EGLint height = ret_surface ? ((VEGLSurface)ret_surface)->config.height : 0;

    EGL_LOG_API("EGL(tid=0x%x): eglCreatePbufferSurface_post 0x%08X 0x%08X (0x%08X) => 0x%08X (%d x %d)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)attrib_list, (EGLint)(gctUINTPTR_T)ret_surface,
                width, height);

    return NULL;
}

EGLSurface LOG_eglCreatePixmapSurface_pre(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePixmapSurface_pre 0x%08X 0x%08X 0x%08X (0x%08X)\n",
               (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config, (EGLint)(gctUINTPTR_T)pixmap,
               (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLSurface LOG_eglCreatePixmapSurface_post(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGLint width  = ret_surface ? ((VEGLSurface)ret_surface)->config.width  : 0;
    EGLint height = ret_surface ? ((VEGLSurface)ret_surface)->config.height : 0;

    EGL_LOG_API("EGL(tid=0x%x): eglCreatePixmapSurface_post 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X (%d x %d)\n",
               (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config, (EGLint)(gctUINTPTR_T)pixmap,
               (EGLint)(gctUINTPTR_T)attrib_list, (EGLint)(gctUINTPTR_T)ret_surface,
               width, height);

    return gcvNULL;
}

EGLBoolean LOG_eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroySurface 0x%08X 0x%08X\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface);

    return EGL_TRUE;
}

EGLBoolean LOG_eglQuerySurface_pre(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQuerySurface_pre 0x%08X 0x%08X 0x%08X 0x%08X\n",
               (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
               (EGLint)(gctUINTPTR_T)surface, attribute, *value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglQuerySurface_post(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQuerySurface_post 0x%08X 0x%08X 0x%08X 0x%08X\n",
               (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
               (EGLint)(gctUINTPTR_T)surface, attribute, *value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglBindAPI(EGLenum api)
{
    EGL_LOG_API("EGL(tid=0x%x): eglBindAPI 0x%08X\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), api);
    return EGL_TRUE;
}

EGLenum LOG_eglQueryAPI_pre(void)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryAPI_pre\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());
    return EGL_TRUE;
}

EGLenum LOG_eglQueryAPI_post(EGLenum api)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryAPI_post = 0x%04x\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), api);
    return EGL_TRUE;
}

EGLBoolean LOG_eglWaitClient(void)
{
    EGL_LOG_API("EGL(tid=0x%x): eglWaitClient\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());
    return EGL_TRUE;
}

EGLBoolean LOG_eglReleaseThread(void)
{
    EGL_LOG_API("EGL(tid=0x%x): eglReleaseThread\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());
    return EGL_TRUE;
}

EGLSurface LOG_eglCreatePbufferFromClientBuffer_pre(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePbufferFromClientBuffer_pre 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, buftype,
                (EGLint)(gctUINTPTR_T)buffer, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLSurface LOG_eglCreatePbufferFromClientBuffer_post(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGLint width  = ret_surface ? ((VEGLSurface)ret_surface)->config.width  : 0;
    EGLint height = ret_surface ? ((VEGLSurface)ret_surface)->config.height : 0;

    EGL_LOG_API("EGL(tid=0x%x): eglCreatePbufferFromClientBuffer_post 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X (%d x %d)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, buftype,
                (EGLint)(gctUINTPTR_T)buffer, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)attrib_list, (EGLint)(gctUINTPTR_T)ret_surface,
                width, height);

    return gcvNULL;
}

EGLBoolean LOG_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglSurfaceAttrib 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface, attribute, value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EGL_LOG_API("EGL(tid=0x%x): eglBindTexImage 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface, buffer);

    return EGL_TRUE;
}

EGLBoolean LOG_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EGL_LOG_API("EGL(tid=0x%x): eglReleaseTexImage 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface, buffer);

    return EGL_TRUE;
}

EGLBoolean LOG_eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    EGL_LOG_API("EGL(tid=0x%x): eglSwapInterval 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, interval);

    return EGL_TRUE;
}

EGLContext LOG_eglCreateContext_pre(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateContext_pre 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)share_context,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLContext LOG_eglCreateContext_post(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list, EGLContext ret_context)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateContext_post 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)share_context,
                (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_context);

    return gcvNULL;
}


EGLBoolean LOG_eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroyContext 0x%08X 0x%08X\n",
               (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)ctx);

    return EGL_TRUE;
}

EGLBoolean LOG_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    EGL_LOG_API("EGL(tid=0x%x): eglMakeCurrent 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)draw,
                (EGLint)(gctUINTPTR_T)read, (EGLint)(gctUINTPTR_T)ctx);

    return EGL_TRUE;
}

EGLContext LOG_eglGetCurrentContext_pre()
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentContext_pre\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());

    return gcvNULL;
}

EGLContext LOG_eglGetCurrentContext_post(EGLContext ret_ctx)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentContext_post => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)ret_ctx);

    return gcvNULL;
}

EGLSurface LOG_eglGetCurrentSurface_pre(EGLint readdraw)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentSurface_pre 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), readdraw);

    return gcvNULL;
}

EGLSurface LOG_eglGetCurrentSurface_post(EGLint readdraw, EGLSurface ret_surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentSurface_post 0x%08X => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), readdraw, (EGLint)(gctUINTPTR_T)ret_surface);

    return gcvNULL;
}

EGLDisplay LOG_eglGetCurrentDisplay_pre()
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentDisplay_pre\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());

    return gcvNULL;
}


EGLDisplay LOG_eglGetCurrentDisplay_post(EGLDisplay ret_dpy)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetCurrentDisplay_post => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)ret_dpy) ;

    return gcvNULL;
}

EGLBoolean LOG_eglQueryContext_pre(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryContext_pre 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, attribute, *value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglQueryContext_post(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglQueryContext_post 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, attribute, *value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglWaitGL(void)
{
    EGL_LOG_API("EGL(0x%x: eglWaitGL\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());

    return EGL_TRUE;
}

EGLBoolean LOG_eglWaitNative(EGLint engine)
{
    EGL_LOG_API("EGL(tid=0x%x): eglWaitNative 0x%08X\n", engine, (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID());

    return EGL_TRUE;
}

static EGLint frameNum = 0;

EGLBoolean LOG_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglSwapBuffers 0x%08X 0x%08X, Frame Number = %d\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface, frameNum++);

    return EGL_TRUE;
}

EGLBoolean LOG_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCopyBuffers 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)surface,
                (EGLint)(gctUINTPTR_T)target);

    return EGL_TRUE;
}

void LOG_eglGetProcAddress_pre(const char *procname)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetProcAddress_pre %s\n", (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), procname);
}

void LOG_eglGetProcAddress_post(const char *procname, __eglMustCastToProperFunctionPointerType func)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetProcAddress_post %s\n => (0x%08X)",
                 (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), procname, (EGLint)(gctUINTPTR_T)func);
}

/* EGL 1.5. */
EGLSync LOG_eglCreateSync_pre(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateSync_pre 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                type, (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLSync LOG_eglCreateSync_post(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list, EGLSync ret_sync)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateSync_post 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                type, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_sync);

    return gcvNULL;

}

EGLBoolean LOG_eglDestroySync(EGLDisplay dpy, EGLSync sync)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroySync 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync);

    return EGL_TRUE;

}

EGLint LOG_eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    EGL_LOG_API("EGL(tid=0x%x): eglClientWaitSync 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, flags, (EGLint)(gctUINTPTR_T)timeout);

    return EGL_TRUE;

}

EGLBoolean LOG_eglGetSyncAttrib_pre(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetSyncAttrib_pre 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, attribute, (EGLint)(gctUINTPTR_T)value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglGetSyncAttrib_post(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value, EGLAttrib ret_value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetSyncAttrib_post 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, attribute, (EGLint)(gctUINTPTR_T)value, ret_value);

    return EGL_TRUE;
}

EGLImage LOG_eglCreateImage_pre(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateImage_pre 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, target,
                (EGLint)(gctUINTPTR_T)buffer,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLImage LOG_eglCreateImage_post(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list, EGLImage ret_image)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateImage_post 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, target,
                (EGLint)(gctUINTPTR_T)buffer,
                (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_image);

    return gcvNULL;
}

EGLBoolean LOG_eglDestroyImage(EGLDisplay dpy, EGLImage image)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroyImage 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)image);

    return EGL_TRUE;
}

EGLDisplay LOG_eglGetPlatformDisplay_pre(EGLenum platform, void *native_display, const EGLAttrib *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetPlatformDisplay 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)platform, (EGLint)(gctUINTPTR_T)native_display, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_DISPLAY;
}

EGLDisplay LOG_eglGetPlatformDisplay_post(EGLenum platform, void *native_display, const EGLAttrib *attrib_list, EGLDisplay ret_dpy)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetPlatformDisplay 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)platform, (EGLint)(gctUINTPTR_T)native_display, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_dpy);

    return EGL_NO_DISPLAY;
}

EGLSurface LOG_eglCreatePlatformWindowSurface_pre(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformWindowSurface 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_window, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformWindowSurface_post(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list, EGLSurface ret_surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformWindowSurface 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_window, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_surface);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformPixmapSurface_pre(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformPixmapSurface 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_pixmap, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformPixmapSurface_post(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list, EGLSurface ret_surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformPixmapSurface 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_pixmap, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_surface);

    return EGL_NO_SURFACE;
}

EGLBoolean LOG_eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    EGL_LOG_API("EGL(tid=0x%x): eglWaitSync 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)sync, flags);

    return EGL_TRUE;
}

/* EGL_KHR_lock_surface. */
EGLBoolean LOG_eglLockSurfaceKHR(EGLDisplay dpy, EGLSurface surface, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglLockSurfaceKHR 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_TRUE;
}

EGLBoolean LOG_eglUnlockSurfaceKHR(EGLDisplay dpy, EGLSurface surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglUnlockSurfaceKHR 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)surface);

    return EGL_TRUE;
}

/* EGL_KHR_image. */
EGLImageKHR LOG_eglCreateImageKHR_pre(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateImageKHR_pre 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, target,
                (EGLint)(gctUINTPTR_T)buffer,
                (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLImageKHR LOG_eglCreateImageKHR_post(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list, EGLImageKHR ret_image)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateImageKHR_post 0x%08X 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)ctx, target,
                (EGLint)(gctUINTPTR_T)buffer,
                (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_image);

    return gcvNULL;
}


EGLBoolean LOG_eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroyImageKHR 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)image);

    return EGL_TRUE;
}

/* EGL_KHR_fence_sync. */
EGLSyncKHR LOG_eglCreateSyncKHR_pre(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateSyncKHR_pre 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                type, (EGLint)(gctUINTPTR_T)attrib_list);

    return gcvNULL;
}

EGLSyncKHR LOG_eglCreateSyncKHR_post(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list, EGLSyncKHR ret_sync)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreateSyncKHR_post 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                type, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_sync);

    return gcvNULL;

}

EGLBoolean LOG_eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync)
{
    EGL_LOG_API("EGL(tid=0x%x): eglDestroySyncKHR 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync);

    return EGL_TRUE;

}

EGLint LOG_eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
    EGL_LOG_API("EGL(tid=0x%x): eglClientWaitSyncKHR 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, flags, (EGLint)(gctUINTPTR_T)timeout);

    return EGL_TRUE;

}

EGLBoolean LOG_eglGetSyncAttribKHR_pre(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetSyncAttribKHR_pre 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, attribute, (EGLint)(gctUINTPTR_T)value);

    return EGL_TRUE;
}

EGLBoolean LOG_eglGetSyncAttribKHR_post(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value, EGLint ret_value)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetSyncAttribKHR_post 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, attribute, (EGLint)(gctUINTPTR_T)value, ret_value);

    return EGL_TRUE;
}

/* EGL_KHR_wait_sync. */
EGLint LOG_eglWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags)
{
    EGL_LOG_API("EGL(tid=0x%x): eglWaitSyncKHR 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)sync, flags);

    return EGL_TRUE;
}

/* EGL_KHR_reusable_sync. */
EGLBoolean LOG_eglSignalSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode)
{
    EGL_LOG_API("EGL(tid=0x%x): eglSignalSyncKHR 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(), (EGLint)(gctUINTPTR_T)dpy,
                (EGLint)(gctUINTPTR_T)sync, mode);

    return EGL_TRUE;

}

/* EGL_EXT_platform_base. */
EGLDisplay LOG_eglGetPlatformDisplayEXT_pre(EGLenum platform, void *native_display, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetPlatformDisplayEXT 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)platform, (EGLint)(gctUINTPTR_T)native_display, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_DISPLAY;
}

EGLDisplay LOG_eglGetPlatformDisplayEXT_post(EGLenum platform, void *native_display, const EGLint *attrib_list, EGLDisplay ret_dpy)
{
    EGL_LOG_API("EGL(tid=0x%x): eglGetPlatformDisplayEXT 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)platform, (EGLint)(gctUINTPTR_T)native_display, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_dpy);

    return EGL_NO_DISPLAY;
}

EGLSurface LOG_eglCreatePlatformWindowSurfaceEXT_pre(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformWindowSurfaceEXT 0x%08X 0x%08X 0x%08X (0x%08X)\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_window, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformWindowSurfaceEXT_post(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformWindowSurfaceEXT 0x%08X 0x%08X 0x%08X (0x%08X) => 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_window, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_surface);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformPixmapSurfaceEXT_pre(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformPixmapSurfaceEXT 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_pixmap, (EGLint)(gctUINTPTR_T)attrib_list);

    return EGL_NO_SURFACE;
}

EGLSurface LOG_eglCreatePlatformPixmapSurfaceEXT_post(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list, EGLSurface ret_surface)
{
    EGL_LOG_API("EGL(tid=0x%x): eglCreatePlatformPixmapSurfaceEXT 0x%08X 0x%08X 0x%08X 0x%08X\n",
                (EGLint)(gctUINTPTR_T)gcoOS_GetCurrentThreadID(),
                (EGLint)(gctUINTPTR_T)dpy, (EGLint)(gctUINTPTR_T)config,
                (EGLint)(gctUINTPTR_T)native_pixmap, (EGLint)(gctUINTPTR_T)attrib_list,
                (EGLint)(gctUINTPTR_T)ret_surface);

    return EGL_NO_SURFACE;
}

/* EGL_KHR_partial_update. */
EGLBoolean LOG_eglSetDamageRegionKHR (EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects)
{
    EGL_LOG_API("EGL(tid=0x%lx): eglSetDamageRegionKHR 0x%08lX 0x%08lX 0x%08lx 0x%08x\n",
                (long)(uintptr_t)gcoOS_GetCurrentThreadID(),
                (long)(uintptr_t)dpy, (long)(uintptr_t)surface,
                (long)(uintptr_t)rects, n_rects);

    return EGL_TRUE;
}

/* EGL_KHR_swap_buffers_with_damage. */
EGLBoolean LOG_eglSwapBuffersWithDamageKHR(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects)
{
    EGL_LOG_API("EGL(tid=0x%lx): eglSwapBuffersWithDamageKHR 0x%08lX 0x%08lX 0x%08lx 0x%08x\n",
                (long)(uintptr_t)gcoOS_GetCurrentThreadID(),
                (long)(uintptr_t)dpy, (long)(uintptr_t)surface,
                (long)(uintptr_t)rects, n_rects);

    return EGL_TRUE;
}

/* EGL_EXT_swap_buffers_with_damage. */
EGLBoolean LOG_eglSwapBuffersWithDamageEXT(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects)
{
    EGL_LOG_API("EGL(tid=0x%lx): eglSwapBuffersWithDamageEXT 0x%08lX 0x%08lX 0x%08lx 0x%08x\n",
                (long)(uintptr_t)gcoOS_GetCurrentThreadID(),
                (long)(uintptr_t)dpy, (long)(uintptr_t)surface,
                (long)(uintptr_t)rects, n_rects);

    return EGL_TRUE;
}

/* EGL_EXT_image_dma_buf_import_modifiers */
EGLBoolean LOG_eglQueryDmaBufFormatsEXT(EGLDisplay dpy, EGLint max_formats, EGLint *formats, EGLint *num_formats)
{
     EGL_LOG_API("EGL(tid=0x%lx): LOG_eglQueryDmaBufFormatsEXT 0x%08lX 0x%08lX 0x%08x 0x%08lX 0x%08lx\n",
                (long)(uintptr_t)gcoOS_GetCurrentThreadID(),
                (long)(uintptr_t)dpy, max_formats,
                (long)(uintptr_t)formats, (long)(uintptr_t)num_formats);

    return EGL_TRUE;
}

EGLBoolean LOG_eglQueryDmaBufModifiersEXT(EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR *modifiers, EGLBoolean *external_only, EGLint *num_modifiers)
{
     EGL_LOG_API("EGL(tid=0x%lx): eglQueryDmaBufModifiersEXT 0x%08lX 0x%08lX 0x%08x 0x%08x 0x%08lx 0x%08lx\n",
                (long)(uintptr_t)gcoOS_GetCurrentThreadID(),
                (long)(uintptr_t)dpy, format,
                 max_modifiers, (long)(uintptr_t)modifiers,
                (long)(uintptr_t)external_only, (long)(uintptr_t)num_modifiers);

    return EGL_TRUE;
}

eglTracerDispatchTableStruct veglLogFunctionTable = {
    LOG_eglGetError_pre,
    LOG_eglGetDisplay_post,
    LOG_eglInitialize,
    LOG_eglTerminate,
    LOG_eglQueryString_post,
    LOG_eglGetConfigs_pre,
    LOG_eglChooseConfig_pre,
    LOG_eglGetConfigAttrib_post,
    LOG_eglCreateWindowSurface_post,
    LOG_eglCreatePbufferSurface_post,
    LOG_eglCreatePixmapSurface_post,
    LOG_eglDestroySurface,
    LOG_eglQuerySurface_pre,
    LOG_eglBindAPI,
    LOG_eglQueryAPI_pre,
    LOG_eglWaitClient,
    LOG_eglReleaseThread,
    LOG_eglCreatePbufferFromClientBuffer_post,
    LOG_eglSurfaceAttrib,
    LOG_eglBindTexImage,
    LOG_eglReleaseTexImage,
    LOG_eglSwapInterval,
    LOG_eglCreateContext_post,
    LOG_eglDestroyContext,
    LOG_eglMakeCurrent,
    LOG_eglGetCurrentContext_post,
    LOG_eglGetCurrentSurface_post,
    LOG_eglGetCurrentDisplay_post,
    LOG_eglQueryContext_pre,
    LOG_eglWaitGL,
    LOG_eglWaitNative,
    LOG_eglSwapBuffers,
    LOG_eglCopyBuffers,
    LOG_eglGetProcAddress_pre,
    /* EGL 1.5 */
    LOG_eglCreateSync_post,
    LOG_eglDestroySync,
    LOG_eglClientWaitSync,
    LOG_eglGetSyncAttrib_post,
    LOG_eglCreateImage_post,
    LOG_eglDestroyImage,
    LOG_eglGetPlatformDisplay_post,
    LOG_eglCreatePlatformWindowSurface_post,
    LOG_eglCreatePlatformPixmapSurface_post,
    LOG_eglWaitSync,
    /* EGL_KHR_lock_surface. */
    LOG_eglLockSurfaceKHR,
    LOG_eglUnlockSurfaceKHR,
    /* EGL_KHR_image. */
    LOG_eglCreateImageKHR_post,
    LOG_eglDestroyImageKHR,
    /* EGL_KHR_fence_sync. */
    LOG_eglCreateSyncKHR_post,
    LOG_eglDestroySyncKHR,
    LOG_eglClientWaitSyncKHR,
    LOG_eglGetSyncAttribKHR_pre,
    /* EGL_KHR_wait_sync. */
    LOG_eglWaitSyncKHR,
    /* EGL_KHR_reusable_sync. */
    LOG_eglSignalSyncKHR,
    /* EGL_EXT_platform_base. */
    LOG_eglGetPlatformDisplayEXT_post,
    LOG_eglCreatePlatformWindowSurfaceEXT_post,
    LOG_eglCreatePlatformPixmapSurfaceEXT_post,
    /* EGL_KHR_partial_update */
    LOG_eglSetDamageRegionKHR,
    LOG_eglSwapBuffersWithDamageKHR,
    LOG_eglSwapBuffersWithDamageEXT,
    /* EGL_EXT_image_dma_buf_import_modifiers */
    LOG_eglQueryDmaBufFormatsEXT,
    LOG_eglQueryDmaBufModifiersEXT,

    /******  The above interfaces are used to link with external vTracer library libGLES_vlogger.so ******/

    LOG_eglGetDisplay_pre,
    LOG_eglGetConfigAttrib_pre,
    LOG_eglCreateWindowSurface_pre,
    LOG_eglCreatePbufferSurface_pre,
    LOG_eglCreatePixmapSurface_pre,
    LOG_eglCreatePbufferFromClientBuffer_pre,
    LOG_eglCreateContext_pre,
    LOG_eglGetCurrentContext_pre,
    LOG_eglGetCurrentSurface_pre,
    LOG_eglGetCurrentDisplay_pre,
    LOG_eglGetProcAddress_post,

    LOG_eglGetError_post,
    LOG_eglQueryString_pre,
    LOG_eglGetConfigs_post,
    LOG_eglChooseConfig_post,
    LOG_eglQuerySurface_post,
    LOG_eglQueryAPI_post,
    LOG_eglQueryContext_post,
    /* EGL 1.5 */
    LOG_eglCreateSync_pre,
    LOG_eglGetSyncAttrib_pre,
    LOG_eglCreateImage_pre,
    LOG_eglGetPlatformDisplay_pre,
    LOG_eglCreatePlatformWindowSurface_pre,
    LOG_eglCreatePlatformPixmapSurface_pre,
    /* EGL_KHR_image. */
    LOG_eglCreateImageKHR_pre,
    /* EGL_KHR_fence_sync. */
    LOG_eglCreateSyncKHR_pre,
    LOG_eglGetSyncAttribKHR_post,
    /* EGL_EXT_platform_base. */
    LOG_eglGetPlatformDisplayEXT_pre,
    LOG_eglCreatePlatformWindowSurfaceEXT_pre,
    LOG_eglCreatePlatformPixmapSurfaceEXT_pre,
};

char *veglTraceFuncNames[] = {
    /* EGL 1.4 */
    "eglGetError",
    "eglGetDisplay",
    "eglInitialize",
    "eglTerminate",
    "eglQueryString",
    "eglGetConfigs",
    "eglChooseConfig",
    "eglGetConfigAttrib",
    "eglCreateWindowSurface",
    "eglCreatePbufferSurface",
    "eglCreatePixmapSurface",
    "eglDestroySurface",
    "eglQuerySurface",
    "eglBindAPI",
    "eglQueryAPI",
    "eglWaitClient",
    "eglReleaseThread",
    "eglCreatePbufferFromClientBuffer",
    "eglSurfaceAttrib",
    "eglBindTexImage",
    "eglReleaseTexImage",
    "eglSwapInterval",
    "eglCreateContext",
    "eglDestroyContext",
    "eglMakeCurrent",
    "eglGetCurrentContext",
    "eglGetCurrentSurface",
    "eglGetCurrentDisplay",
    "eglQueryContext",
    "eglWaitGL",
    "eglWaitNative",
    "eglSwapBuffers",
    "eglCopyBuffers",
    "eglGetProcAddress",
    /* EGL 1.5 */
    "eglCreateSync",
    "eglDestroySync",
    "eglClientWaitSync",
    "eglGetSyncAttrib",
    "eglCreateImage",
    "eglDestroyImage",
    "eglGetPlatformDisplay",
    "eglCreatePlatformWindowSurface",
    "eglCreatePlatformPixmapSurface",
    "eglWaitSync",
    /* EGL_KHR_lock_surface. */
    "eglLockSurfaceKHR",
    "eglUnlockSurfaceKHR",
    /* EGL_KHR_image. */
    "eglCreateImageKHR",
    "eglDestroyImageKHR",
    /* EGL_KHR_fence_sync. */
    "eglCreateSyncKHR",
    "eglDestroySyncKHR",
    "eglClientWaitSyncKHR",
    "eglGetSyncAttribKHR",
    /* EGL_KHR_wait_sync. */
    "eglWaitSyncKHR",
    /* EGL_KHR_resuable_sync. */
    "eglSignalSyncKHR",
    /* EGL_EXT_platform_base. */
    "eglGetPlatformDisplayEXT",
    "eglCreatePlatformWindowSurfaceEXT",
    "eglCreatePlatformPixmapSurfaceEXT",
    /* EGL_KHR_partial_update */
    "eglSetDamageRegionKHR"
    "eglSwapBuffersWithDamageKHR",
    "eglSwapBuffersWithDamageEXT",
    /* EGL_EXT_image_dma_buf_import_modifiers */
    "eglQueryDmaBufFormatsEXT",
    "eglQueryDmaBufModifiersEXT"
};


/* EGL Tracer Dispatch Function Table */
eglTracerDispatchTableStruct veglTracerDispatchTable = {0};

const int veglTracerDispatchTableSize = sizeof(veglTraceFuncNames) / sizeof(char *);

EGLBoolean veglInitTracerDispatchTable()
{
    gctHANDLE trlib = gcvNULL;
    gctPOINTER funcPtr = gcvNULL;
    gceSTATUS status;
    char trApiName[80];
    int i;
    EGLBoolean ret = EGL_TRUE;

    switch (veglTraceMode)
    {
    case gcvTRACEMODE_LOGGER:
        {
            /* Set veglTracerDispatchTable[] to the external tracer functions */

            /* Clear veglTracerDispatchTable[] */
            memset(&veglTracerDispatchTable, 0, sizeof(eglTracerDispatchTableStruct));

#if defined(_WIN32) || defined(_WIN32_WCE)
            gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.dll", &trlib);
#else
            gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.so", &trlib);
#endif

            if (trlib  == gcvNULL)
            {
#if defined(_WIN32) || defined(_WIN32_WCE)
                gcoOS_Print("Failed to open libGLES_vlogger.dll!\n");
#else
                gcoOS_Print("Failed to open libGLES_vlogger.so!\n");
#endif
                return EGL_FALSE;
            }

            for (i = 0; i < veglTracerDispatchTableSize; i++)
            {
                trApiName[0] = '\0';
                gcoOS_StrCatSafe(trApiName, 80, "TR_");
                gcoOS_StrCatSafe(trApiName, 80, veglTraceFuncNames[i]);
                status =  gcoOS_GetProcAddress(gcvNULL, trlib, trApiName, &funcPtr);

                if (status == gcvSTATUS_OK)
                {
                    ((void *(*))(&veglTracerDispatchTable))[i] = funcPtr;
                }
                else
                {
                    gcoOS_Print("Failed to initialize veglTracerDispatchTable: %s!\n", veglTraceFuncNames[i]);
                    ((void *(*))(&veglTracerDispatchTable))[i] = gcvNULL;
                    ret = EGL_FALSE;
                }
            }
        }
        break;

    case gcvTRACEMODE_FULL:
        {
            /* Set veglTracerDispatchTable[] to simple log functions */
            veglTracerDispatchTable = veglLogFunctionTable;
        }
        break;

     case gcvTRACEMODE_NONE:
     default:
         {
             /* Clear veglTracerDispatchTable[] */
             memset(&veglTracerDispatchTable, 0, sizeof(eglTracerDispatchTableStruct));
             break;
         }
    }

    return ret;
}

