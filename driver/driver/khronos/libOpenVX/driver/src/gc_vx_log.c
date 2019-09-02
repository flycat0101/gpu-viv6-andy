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


#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

VX_API_ENTRY void  VX_API_CALL vxRegisterLogCallback(vx_context context, vx_log_callback_f callback, vx_bool reentrant)
{
    gcmHEADER_ARG("context=%p, callback=%p, reentrant=0x%x", context, callback, reentrant);
    gcmDUMP_API("$VX vxRegisterLogCallback: context=%p, callback=%p, reentrant=0x%x", context, callback, reentrant);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return;
    }
    vxAcquireMutex(context->base.lock);

    if (context->logCallback == VX_NULL && callback != VX_NULL)
    {
        context->logEnabled = vx_true_e;
        context->logCallbackReentrant = reentrant;
        if (!reentrant) vxCreateMutex(OUT &context->logLock);
    }

    if (context->logCallback != VX_NULL && callback == VX_NULL)
    {
        if (!context->logCallbackReentrant)
        {
            vxDestroyMutex(context->logLock);
            context->logLock = VX_NULL;
        }

        context->logEnabled = vx_false_e;
        context->logCallbackReentrant = vx_true_e;
    }

    if (context->logCallback != VX_NULL && callback != VX_NULL && context->logCallback != callback)
    {
        if (!context->logCallbackReentrant)
        {
            vxDestroyMutex(context->logLock);
            context->logLock = VX_NULL;
        }

        context->logEnabled = vx_true_e;
        context->logCallbackReentrant = reentrant;
        if (!reentrant) vxCreateMutex(OUT &context->logLock);
    }

    context->logCallback = callback;

    vxReleaseMutex(context->base.lock);
    gcmFOOTER_NO();
}

#if defined(_WIN32)
VX_API_ENTRY void  VX_API_CALL _vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    vx_context context;
    va_list argList;
    vx_char buffer[VX_MAX_LOG_MESSAGE_LEN];

    gcmHEADER_ARG("ref=%p, status=0x%x, message=%s", ref, status, message);
    gcmDUMP_API("$VX _vxAddLogEntry: ref=%p, status=0x%x, message=%s", ref, status, message);

    if (!vxoReference_IsValidAndNoncontext(ref) && !vxoContext_IsValid((vx_context)ref))
    {
        vxError("Invalid reference, %p, for vxAddLogEntry", ref);
        gcmFOOTER_NO();
        return;
    }

    if (status == VX_SUCCESS)
    {
        vxError("Invalid status, %d, for vxAddLogEntry", status);
        gcmFOOTER_NO();
        return;
    }

    if (message == VX_NULL)
    {
        vxError("The message is NULL, for vxAddLogEntry");
        gcmFOOTER_NO();
        return;
    }

    context = vxoContext_GetFromReference(ref);
    vxmASSERT(context);

    if (!context->logEnabled)
    {
        gcmFOOTER_NO();
        return;
    }

    if (context->logCallback == VX_NULL)
    {
        vxError("No registered log callback for vxAddLogEntry");
        gcmFOOTER_NO();
        return;
    }

    va_start(argList, message);

    vsnprintf(buffer, VX_MAX_LOG_MESSAGE_LEN, message, argList);

    buffer[VX_MAX_LOG_MESSAGE_LEN - 1] = 0;

    va_end(argList);

    if (!context->logCallbackReentrant) vxAcquireMutex(context->logLock);

    context->logCallback(context, ref, status, buffer);

    if (!context->logCallbackReentrant) vxReleaseMutex(context->logLock);

    gcmFOOTER_NO();
    return;
}
#endif

VX_API_ENTRY void  VX_API_CALL vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    vx_context context;
    va_list argList;
    vx_char buffer[VX_MAX_LOG_MESSAGE_LEN];

    gcmHEADER_ARG("ref=%p, status=0x%x, message=%s", ref, status, message);
    gcmDUMP_API("$VX vxAddLogEntry: ref=%p, status=0x%x, message=%s", ref, status, message);

    if (!vxoReference_IsValidAndNoncontext(ref) && !vxoContext_IsValid((vx_context)ref))
    {
        vxError("Invalid reference, %p, for vxAddLogEntry", ref);
        gcmFOOTER_NO();
        return;
    }

    if (status == VX_SUCCESS)
    {
        vxError("Invalid status, %d, for vxAddLogEntry", status);
        gcmFOOTER_NO();
        return;
    }

    if (message == VX_NULL)
    {
        vxError("The message is NULL, for vxAddLogEntry");
        gcmFOOTER_NO();
        return;
    }

    context = vxoContext_GetFromReference(ref);
    vxmASSERT(context);

    if (!context->logEnabled)
    {
        gcmFOOTER_NO();
        return;
    }

    if (context->logCallback == VX_NULL)
    {
        vxError("No registered log callback for vxAddLogEntry");
        gcmFOOTER_NO();
        return;
    }

    va_start(argList, message);

    vsnprintf(buffer, VX_MAX_LOG_MESSAGE_LEN, message, argList);

    buffer[VX_MAX_LOG_MESSAGE_LEN - 1] = 0;

    va_end(argList);

    if (!context->logCallbackReentrant) vxAcquireMutex(context->logLock);

    context->logCallback(context, ref, status, buffer);

    if (!context->logCallbackReentrant) vxReleaseMutex(context->logLock);

    gcmFOOTER_NO();
    return;
}


