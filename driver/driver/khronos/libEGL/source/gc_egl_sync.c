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

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_SYNC

static gcmINLINE EGLAttrib
_AttribValue(
    const void *attrib_list,
    EGLBoolean intAttrib,
    EGLint index
    )
{
    const EGLint * intList       = (const EGLint *) attrib_list;
    const EGLAttrib * attribList = (const EGLAttrib *) attrib_list;

    return intAttrib ? (EGLAttrib) intList[index]
                     : attribList[index];
}

static EGLSync
veglCreateSync(
    EGLDisplay Dpy,
    EGLenum type,
    const void *attrib_list,
    EGLBoolean intAttrib
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;
    gcsHAL_INTERFACE iface;
#if defined(__linux__)
    EGLint fenceFD = EGL_NO_NATIVE_FENCE_FD_ANDROID;
#endif

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        return EGL_NO_SYNC;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        return EGL_NO_SYNC;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Test type is a supported type of sync object. */
    if (type != EGL_SYNC_REUSABLE_KHR
#if defined(__linux__)
        && type != EGL_SYNC_NATIVE_FENCE_ANDROID
#endif
        && type != EGL_SYNC_FENCE)
    {
        thread->error = EGL_BAD_ATTRIBUTE;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (EGL_SYNC_REUSABLE_KHR != type && thread->context == EGL_NO_CONTEXT)
    {
        thread->error = EGL_BAD_MATCH;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (type)
    {
    case EGL_SYNC_REUSABLE_KHR:
    case EGL_SYNC_FENCE:
        if ((attrib_list != gcvNULL) &&
            (_AttribValue(attrib_list, intAttrib, 0) != EGL_NONE))
        {
            thread->error = EGL_BAD_ATTRIBUTE;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

#if defined(__linux__)
    case EGL_SYNC_NATIVE_FENCE_ANDROID:
        if (attrib_list != gcvNULL)
        {
            gctUINT i;

            for (i = 0; _AttribValue(attrib_list, intAttrib, i) != EGL_NONE; i += 2)
            {
                switch (_AttribValue(attrib_list, intAttrib, i))
                {
                case EGL_SYNC_NATIVE_FENCE_FD_ANDROID:
                    fenceFD = _AttribValue(attrib_list, intAttrib, i + 1);
                    break;

                default:
                    thread->error = EGL_BAD_ATTRIBUTE;
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }
        break;
#endif

    default:
        thread->error = EGL_BAD_ATTRIBUTE;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create new sync. */
    status = gcoOS_Allocate(gcvNULL,
                           sizeof(struct eglSync),
                           &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        thread->error = EGL_BAD_ALLOC;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    sync = pointer;

    /* Initialize context. */
    sync->resObj.signature  = EGL_SYNC_SIGNATURE;
    sync->type              = type;
    sync->condition         = EGL_SYNC_PRIOR_COMMANDS_COMPLETE;
    sync->signal            = gcvNULL;
#if defined(__linux__)
    sync->fenceFD           = EGL_NO_NATIVE_FENCE_FD_ANDROID;
#endif

    switch (type)
    {
    case EGL_SYNC_REUSABLE_KHR:
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &sync->signal);

        if (gcmIS_ERROR(status))
        {
            /* Roll back. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));

            thread->error = EGL_BAD_ALLOC;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

#if defined(__linux__)
    case EGL_SYNC_NATIVE_FENCE_ANDROID:
        sync->fenceFD = fenceFD;

        if (fenceFD != EGL_NO_NATIVE_FENCE_FD_ANDROID)
        {
            /* Fence is from external. */
            sync->condition = EGL_SYNC_NATIVE_FENCE_SIGNALED_ANDROID;
            break;
        }

        /* Create native fence sync. */
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &sync->signal);

        if (gcmIS_ERROR(status))
        {
            /* Roll back. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));
            thread->error = EGL_BAD_ALLOC;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        /* Create native fence. */
        status = gcoOS_CreateNativeFence(gcvNULL, sync->signal, &fenceFD);

        if (gcmIS_ERROR(status))
        {
            /* Roll back. */
            gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, sync->signal));
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));

            thread->error = EGL_BAD_ALLOC;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        sync->fenceFD = fenceFD;

        veglSyncNative(thread, dpy);

        if (dpy->platform->platform == EGL_PLATFORM_GBM_VIV)
        {
            thread->pendingSignal = sync->signal;
        }
        else
        {
            /* Submit the sync point. */
            iface.command            = gcvHAL_SIGNAL;
            iface.engine             = gcvENGINE_RENDER;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(sync->signal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = gcmPTR_TO_UINT64(dpy->process);
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Send event. */
            gcoHAL_ScheduleEvent(gcvNULL, &iface);
            gcoHAL_Commit(gcvNULL, gcvFALSE);
        }

        break;

#endif

    case EGL_SYNC_FENCE:
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &sync->signal);

        if (gcmIS_ERROR(status))
        {
            /* Roll back. */
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));

            thread->error = EGL_BAD_ALLOC;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        veglSyncNative(thread, dpy);

        /* Submit the signal. */
        iface.command            = gcvHAL_SIGNAL;
        iface.engine             = gcvENGINE_RENDER;
        iface.u.Signal.signal    = gcmPTR_TO_UINT64(sync->signal);
        iface.u.Signal.auxSignal = 0;
        iface.u.Signal.process   = gcmPTR_TO_UINT64(dpy->process);
        iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;
        /* Send event. */
        gcoHAL_ScheduleEvent(gcvNULL, &iface);
        gcoHAL_Commit(gcvNULL, gcvFALSE);
        break;

    default:
        thread->error = EGL_BAD_ATTRIBUTE;
        /* Roll back. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_SIGNAL,
        "%s(%d): sync signal created 0x%08X",
        __FUNCTION__, __LINE__,
        sync->signal
        );

    /* push to stack */
    veglPushResObj(dpy, (VEGLResObj *)&dpy->syncStack, (VEGLResObj)sync);

    /* Success. */
    thread->error = EGL_SUCCESS;
    return sync;

OnError:
    return EGL_NO_SYNC;
}

EGLBoolean
veglDestroySync(
    EGLDisplay Dpy,
    EGLSync Sync
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;

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

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if (sync == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (sync->signal)
    {
        if (sync->type == EGL_SYNC_REUSABLE_KHR)
        {
            /* Signal before destroying. */
            status = gcoOS_Signal(gcvNULL, sync->signal, gcvTRUE);

            if (gcmIS_ERROR(status))
            {
                /* Error. */
                thread->error = EGL_BAD_ACCESS;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
        }

        /* Destroy the signal. */
        status = gcoOS_DestroySignal(gcvNULL, sync->signal);

        if (gcmIS_ERROR(status))
        {
            /* Error. */
            thread->error = EGL_BAD_ACCESS;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

#if defined(__linux__)
    if (sync->fenceFD != EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        /* Close file descriptor. */
        status = gcoOS_CloseFD(gcvNULL, sync->fenceFD);

        if (gcmIS_ERROR(status))
        {
            /* Error. */
            thread->error = EGL_BAD_ACCESS;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
#endif

    /* pop out from stack */
    veglPopResObj(dpy, (VEGLResObj *)&dpy->syncStack,(VEGLResObj)sync);

    /* Free the eglSync structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, sync));
    /* Success. */
    thread->error = EGL_SUCCESS;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLint
veglClientWaitSync(
    EGLDisplay Dpy,
    EGLSync Sync,
    EGLint flags,
    EGLTime timeout
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;
    gctUINT32 wait;
    EGLint result = EGL_TIMEOUT_EXPIRED;

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

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if (sync == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check flags */
    if (flags & EGL_SYNC_FLUSH_COMMANDS_BIT)
    {
        /*
         * This is optimization:
         * Check if the sync is already signaled. If it is, there's no
         * need to flush again.
         */
#if defined(__linux__)
        if (sync->fenceFD != EGL_NO_NATIVE_FENCE_FD_ANDROID)
        {
            /* ANDROID_native_fence_sync or KHR_fence_sync */
            status = gcoOS_ClientWaitNativeFence(gcvNULL, sync->fenceFD, 0);
        }
        else
#endif
        {
            /* Other sync types. */
            status = gcoOS_WaitSignal(gcvNULL, sync->signal, 0);
        }

        if (gcmIS_SUCCESS(status))
        {
            /* Already signaled. */
            result = EGL_CONDITION_SATISFIED;
        }
        else if (status == gcvSTATUS_TIMEOUT)
        {
            /* Flush as parameter required if not signaled. */
            _Flush(thread);
        }
        else
        {
            /* Error. */
            thread->error = EGL_BAD_ACCESS;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    if (result != EGL_CONDITION_SATISFIED)
    {
        /* Wait the signal */
        wait = (timeout == (EGLTime) EGL_FOREVER)
             ? gcvINFINITE
             : (gctUINT32) gcoMATH_DivideUInt64(timeout, 1000000ull);

#if defined(__linux__)
        if (sync->fenceFD != EGL_NO_NATIVE_FENCE_FD_ANDROID)
        {
            /* Wait external fence fd. */
            status = gcoOS_ClientWaitNativeFence(gcvNULL, sync->fenceFD, wait);
        }
        else
#endif
        {
            /* Wait on signal. */
            status = gcoOS_WaitSignal(gcvNULL, sync->signal, wait);
        }

        if (gcmIS_SUCCESS(status))
        {
            /* Signaled. */
            result = EGL_CONDITION_SATISFIED;
        }
        else if (status == gcvSTATUS_TIMEOUT)
        {
            /* Timeout. */
            result = EGL_TIMEOUT_EXPIRED;
        }
        else
        {
            /* Error. */
            result = EGL_TIMEOUT_EXPIRED;
            thread->error = EGL_BAD_ACCESS;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Success. */
    thread->error = EGL_SUCCESS;
    return result;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
veglGetSyncAttrib(
    EGLDisplay Dpy,
    EGLSync Sync,
    EGLint attribute,
    EGLAttrib *value
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;

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

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if (sync == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (value == gcvNULL)
    {
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (attribute)
    {
    case EGL_SYNC_TYPE:
        *value = sync->type;
        break;

    case EGL_SYNC_STATUS:
#if defined(__linux__)
        if (sync->fenceFD != EGL_NO_NATIVE_FENCE_FD_ANDROID)
        {
            status = gcoOS_ClientWaitNativeFence(gcvNULL, sync->fenceFD, 0);

            if (gcmIS_SUCCESS(status))
            {
                /* Wait till sync signal. */
                *value = EGL_SIGNALED;
            }
            else if (status == gcvSTATUS_TIMEOUT)
            {
                /* Timeout. */
                *value = EGL_UNSIGNALED;
            }
            else
            {
                /* Error. */
                thread->error = EGL_BAD_ACCESS;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            break;
        }
#endif

        status = gcoOS_WaitSignal(gcvNULL, sync->signal, 0);

        if (gcmIS_SUCCESS(status))
        {
            /* Wait till sync signal. */
            *value = EGL_SIGNALED;
        }
        else if (status == gcvSTATUS_TIMEOUT)
        {
            /* Timeout. */
            *value = EGL_UNSIGNALED;
        }
        else
        {
            /* Error. */
            thread->error = EGL_BAD_ACCESS;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        break;

    case EGL_SYNC_CONDITION:
        switch (sync->type)
        {
        case EGL_SYNC_FENCE:
        case EGL_SYNC_NATIVE_FENCE_ANDROID:
            *value = sync->condition;
            break;

        default:
            thread->error = EGL_BAD_ATTRIBUTE;
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
        }
        break;

    default:
        /* Bad attribute. */
        thread->error = EGL_BAD_ATTRIBUTE;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    thread->error = EGL_SUCCESS;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
veglWaitSync(
    EGLDisplay Dpy,
    EGLSync Sync,
    EGLint flags
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;

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

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Dpy);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if (sync == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (flags != 0)
    {
        /* Bad parameter. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

#if defined(WL_EGL_PLATFORM) || defined(EGL_API_WL)
    status = gcoOS_WaitNativeFence(gcvNULL, sync->fenceFD, 2000);

    if (status == gcvSTATUS_TIMEOUT)
    {
        /* Print a warning. */
        gcmPRINT("%s: Warning: wait for fence fd=%d", __func__, sync->fenceFD);

        /* Wait for ever. */
        status = gcoOS_WaitNativeFence(gcvNULL, sync->fenceFD, gcvINFINITE);
    }
#endif

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Success. */
    thread->error = EGL_SUCCESS;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;

}

/* EGL 1.5. */
EGLAPI EGLSync EGLAPIENTRY
eglCreateSync(
    EGLDisplay dpy,
    EGLenum type,
    const EGLAttrib *attrib_list
    )
{
    VEGLSync sync;

    gcmHEADER_ARG("dpy=0x%x type=%d attrib_list=0x%x", dpy, type, attrib_list);
    VEGL_TRACE_API_PRE(CreateSync)(dpy, type, attrib_list);

    /* Call internal function. */
    sync = veglCreateSync(dpy, type, attrib_list, EGL_FALSE);

    VEGL_TRACE_API_POST(CreateSync)(dpy, type, attrib_list, sync);
    gcmDUMP_API("${EGL eglCreateSync 0x%08X 0x%08X (0x%08X) := 0x%08X",
                dpy, type, attrib_list, sync);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", sync);
    return sync;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroySync(
    EGLDisplay dpy,
    EGLSync sync
    )
{
    EGLBoolean ret;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x", dpy, sync);
    gcmDUMP_API("${EGL eglDestroySync 0x%08X 0x%08X}", dpy, sync);
    VEGL_TRACE_API(DestroySync)(dpy, (VEGLSync) sync);

    /* Call internal function. */
    ret = veglDestroySync(dpy, sync);

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

EGLAPI EGLint EGLAPIENTRY
eglClientWaitSync(
    EGLDisplay dpy,
    EGLSync sync,
    EGLint flags,
    EGLTime timeout
    )
{
    EGLint result;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x flags=%d timeout=%lld",
                  dpy, sync, flags, timeout);
    VEGL_TRACE_API(ClientWaitSync)(dpy, sync, flags, timeout);

    /* Call internal function. */
    result = veglClientWaitSync(dpy, sync, flags, timeout);

    gcmDUMP_API("${EGL eglClientWaitSync 0x%08X 0x%08X 0x%08X 0x%016llX := "
                "0x%08X}",
                dpy, sync, flags, timeout, result);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglGetSyncAttrib(
    EGLDisplay dpy,
    EGLSync sync,
    EGLint attribute,
    EGLAttrib *value
    )
{
    EGLBoolean result;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x attribute=%d", dpy, sync, attribute);
    VEGL_TRACE_API_PRE(GetSyncAttrib)(dpy, sync, attribute, value);

    /* Call internal function. */
    result = veglGetSyncAttrib(dpy, sync, attribute, value);

    gcmFOOTER_ARG("return=%d", result);
    VEGL_TRACE_API_POST(GetSyncAttrib)(dpy, sync, attribute, value, (value ? *value : 0));
    gcmDUMP_API("${EGL eglGetSyncAttrib 0x%08X 0x%08X 0x%08X := 0x%08X}",
                dpy, sync, attribute, (value ? *value : 0));

    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglWaitSync(
    EGLDisplay dpy,
    EGLSync sync,
    EGLint flags
    )
{
    EGLBoolean result;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x flags=%d", dpy, sync, flags);
    VEGL_TRACE_API(WaitSync)(dpy, sync, flags);

    /* Call internal function. */
    result = veglWaitSync(dpy, (EGLSync) sync, flags);

    gcmDUMP_API("${EGL eglWaitSync 0x%08X 0x%08X 0x%08X := 0x%08X}",
                dpy, sync, flags, result);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/* EGL_KHR_fence_sync. */
EGLAPI EGLSyncKHR EGLAPIENTRY
eglCreateSyncKHR(
    EGLDisplay dpy,
    EGLenum type,
    const EGLint *attrib_list
    )
{
    EGLSyncKHR sync;

    gcmHEADER_ARG("dpy=0x%x type=%d attrib_list=0x%x", dpy, type, attrib_list);
    VEGL_TRACE_API_PRE(CreateSyncKHR)(dpy, type, attrib_list);

    /* Alias to eglCreateSync. */
    sync = (EGLSyncKHR) veglCreateSync(dpy, type, attrib_list, EGL_TRUE);

    VEGL_TRACE_API_POST(CreateSyncKHR)(dpy, type, attrib_list, sync);
    gcmDUMP_API("${EGL eglCreateSyncKHR 0x%08X 0x%08X (0x%08X) := 0x%08X",
                dpy, type, attrib_list, sync);
    gcmDUMP_API_ARRAY_TOKEN(attrib_list, EGL_NONE);
    gcmDUMP_API("$}");

    gcmFOOTER_ARG("return=0x%x", sync);
    return sync;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglDestroySyncKHR(
    EGLDisplay dpy,
    EGLSyncKHR sync
    )
{
    EGLBoolean ret;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x", dpy, sync);
    gcmDUMP_API("${EGL eglDestroySyncKHR 0x%08X 0x%08X}", dpy, sync);
    VEGL_TRACE_API(DestroySyncKHR)(dpy, sync);

    /* Alias to eglDestroySync. */
    ret = veglDestroySync(dpy, (EGLSync) sync);

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

EGLAPI EGLint EGLAPIENTRY
eglClientWaitSyncKHR(
    EGLDisplay dpy,
    EGLSyncKHR sync,
    EGLint flags,
    EGLTimeKHR timeout
    )
{
    EGLint result;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x flags=%d timeout=%lld",
                  dpy, sync, flags, timeout);
    VEGL_TRACE_API(ClientWaitSyncKHR)(dpy, sync, flags, timeout);

    /* Alias to eglClientWaitSync. */
    result = veglClientWaitSync(dpy, (EGLSync) sync, flags, (EGLTime) timeout);

    gcmDUMP_API("${EGL eglClientWaitSyncKHR 0x%08X 0x%08X 0x%08X 0x%016llX := "
                "0x%08X}",
                dpy, sync, flags, timeout, result);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

EGLAPI EGLBoolean EGLAPIENTRY
eglGetSyncAttribKHR(
    EGLDisplay dpy,
    EGLSyncKHR sync,
    EGLint attribute,
    EGLint *value
    )
{
    EGLBoolean result;
    EGLAttrib value0 = 0;
    EGLAttrib *pointer;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x attribute=%d", dpy, sync, attribute);
    VEGL_TRACE_API_PRE(GetSyncAttribKHR)(dpy, sync, attribute, value);

    pointer = value ? &value0 : gcvNULL;

    /* Call internal function. */
    result = veglGetSyncAttrib(dpy, (EGLSync) sync, attribute, pointer);

    if (result == EGL_TRUE)
    {
        *value = (EGLint) value0;
    }

    gcmFOOTER_ARG("return=%d", result);
    VEGL_TRACE_API_POST(GetSyncAttribKHR)(dpy, (EGLSync) sync, attribute, value, (value ? *value : 0));
    gcmDUMP_API("${EGL eglGetSyncAttribKHR 0x%08X 0x%08X 0x%08X := 0x%08X}",
                dpy, sync, attribute, (value ? *value : 0));

    return result;
}

/* EGL_KHR_wait_sync. */
EGLAPI EGLint EGLAPIENTRY
eglWaitSyncKHR(
    EGLDisplay dpy,
    EGLSyncKHR sync,
    EGLint flags
    )
{
    EGLint result;

    gcmHEADER_ARG("dpy=0x%x sync=0x%x flags=%d", dpy, sync, flags);
    VEGL_TRACE_API(WaitSyncKHR)(dpy, sync, flags);

    /* Alias to eglWaitSync. */
    result = veglWaitSync(dpy, (EGLSync) sync, flags);

    gcmDUMP_API("${EGL eglWaitSyncKHR 0x%08X 0x%08X 0x%08X := 0x%08X}",
                dpy, sync, flags, result);

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

/* EGL_KHR_reusable_sync */
EGLAPI EGLBoolean EGLAPIENTRY
eglSignalSyncKHR(
    EGLDisplay Dpy,
    EGLSyncKHR Sync,
    EGLenum mode
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    gceSTATUS status;

    gcmHEADER_ARG("Dpy=0x%x Sync=0x%x mode=0x%04x", Dpy, Sync, mode);
    gcmDUMP_API("${EGL eglSignalSyncKHR 0x%08X 0x%08X 0x%08X}",
                Dpy, Sync, mode);

    VEGL_TRACE_API(SignalSyncKHR)(Dpy, Sync, mode);
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
        thread->error = EGL_BAD_DISPLAY;
        gcmFOOTER_ARG("return=%d", EGL_FALSE);
        return EGL_FALSE;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if (sync == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test for type. */
    if (sync->type != EGL_SYNC_REUSABLE_KHR)
    {
        /* Bad match. */
        thread->error = EGL_BAD_MATCH;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check mode */
    switch (mode)
    {
    case EGL_SIGNALED_KHR:
    case EGL_UNSIGNALED_KHR:
        break;

    default:
        /* Bad attribute. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Signal. */
    status = gcoOS_Signal(gcvNULL,
                            sync->signal,
                            (mode == EGL_SIGNALED_KHR));

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        thread->error = EGL_BAD_ACCESS;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
    /* Success. */
    thread->error = EGL_SUCCESS;
    gcmFOOTER_ARG("return=%d", EGL_TRUE);
    return EGL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", EGL_FALSE);
    return EGL_FALSE;
}

#if defined(__linux__)
/* EGL_ANDROID_native_fence_sync. */
EGLAPI EGLint EGLAPIENTRY
eglDupNativeFenceFDANDROID(
    EGLDisplay Display,
    EGLSyncKHR Sync
    )
{
    VEGLThreadData thread;
    VEGLDisplay dpy;
    VEGLSync sync;
    EGLint fenceFD;
    gceSTATUS status;

    gcmHEADER_ARG("Display=0x%x Sync=0x%x", Display, Sync);
    VEGL_TRACE_API_PRE(DupNativeFenceFDANDROID)(Display, Sync);

    /* Get thread data. */
    thread = veglGetThreadData();
    if (thread == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): veglGetThreadData failed.",
            __FUNCTION__, __LINE__
            );

        gcmFOOTER_ARG("return=%d", EGL_NO_NATIVE_FENCE_FD_ANDROID);
        return EGL_NO_NATIVE_FENCE_FD_ANDROID;
    }

    /* Test for valid EGLDisplay structure. */
    dpy = veglGetDisplay(Display);
    if (dpy == gcvNULL)
    {
        /* Bad display. */
        thread->error = EGL_BAD_DISPLAY;
        gcmFOOTER_ARG("%d", EGL_NO_NATIVE_FENCE_FD_ANDROID);
        return EGL_NO_NATIVE_FENCE_FD_ANDROID;
    }

    /* Test if EGLDisplay structure has been initialized. */
    if (!dpy->initialized)
    {
        /* Not initialized. */
        thread->error = EGL_NOT_INITIALIZED;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Hardware relevant thread data initialization. */
    veglInitDeviceThreadData(thread);

    /* Create a shortcut to the sync. */
    sync = (VEGLSync)veglGetResObj(dpy, (VEGLResObj*)&dpy->syncStack, (EGLResObj)Sync, EGL_SYNC_SIGNATURE);

    /* Test for valid EGLSync structure. */
    if ((sync == gcvNULL) || sync->type != EGL_SYNC_NATIVE_FENCE_ANDROID)
    {
        /* Bad parameter. */
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (sync->fenceFD == EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        thread->error = EGL_BAD_PARAMETER;
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (gcmIS_ERROR(gcoOS_DupFD(gcvNULL, sync->fenceFD, &fenceFD)))
    {
        fenceFD = EGL_NO_NATIVE_FENCE_FD_ANDROID;
        thread->error = EGL_BAD_ALLOC;
    }
    gcmDUMP_API("${EGL eglDupNativeFenceFDANDROID 0x%08X 0x%08X := 0x%08X",
                Display, Sync, fenceFD);
    gcmFOOTER_ARG("%d", fenceFD);
    VEGL_TRACE_API_POST(DupNativeFenceFDANDROID)(Display, Sync, fenceFD);
    return fenceFD;

OnError:
    gcmFOOTER_ARG("%d", EGL_NO_NATIVE_FENCE_FD_ANDROID);
    VEGL_TRACE_API_POST(DupNativeFenceFDANDROID)(
                        Display, Sync, EGL_NO_NATIVE_FENCE_FD_ANDROID);
    return EGL_NO_NATIVE_FENCE_FD_ANDROID;

}
#endif

