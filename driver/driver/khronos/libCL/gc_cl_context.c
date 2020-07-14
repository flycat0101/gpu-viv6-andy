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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     002016
#define _GC_OBJ_ZONE        gcdZONE_CL_CONTEXT


gctINT clfRetainContext(
    cl_context Context
    )
{
    gctINT status = CL_SUCCESS;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002007: (clfRetainContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }


    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Context->referenceCount, gcvNULL));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT clfReleaseContext(
    cl_context Context
    )
{
    gctINT          status;
    gctINT32        oldReference;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002007: (clfReleaseContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Context->referenceCount, &oldReference));
    if (oldReference == 1)
    {
#if !gcdFPGA_BUILD
        gcoHAL_SetTimeOut(gcvNULL, gcdGPU_TIMEOUT);
#endif

        /* Send signal to stop event list worker thread. */
        gcoCL_SetSignal(Context->eventListWorkerStopSignal);

        gcoCL_SetSignal(Context->eventListWorkerStartSignal);

        /* Send signal to stop event callback worker thread. */
        gcoCL_SetSignal(Context->eventCallbackWorkerStopSignal);

        gcoCL_SetSignal(Context->eventCallbackWorkerStartSignal);

        /* Wait until the event list thread is closed. */
        gcoOS_CloseThread(gcvNULL, Context->eventListWorkerThread);
        Context->eventListWorkerThread = gcvNULL;

        /* Free signals and mutex. */
        gcmVERIFY_OK(gcoCL_DestroySignal(Context->eventListWorkerStartSignal));
        Context->eventListWorkerStartSignal = gcvNULL;

        gcmVERIFY_OK(gcoCL_DestroySignal(Context->eventListWorkerStopSignal));
        Context->eventListWorkerStopSignal = gcvNULL;

        /* Wait until the event callback thread is closed. */
        if (Context->eventCallbackWorkerThread != gcvNULL)
        {
            gcoOS_CloseThread(gcvNULL, Context->eventCallbackWorkerThread);
            Context->eventCallbackWorkerThread = gcvNULL;
        }

        /* Free signals and mutex. */
        gcmVERIFY_OK(gcoCL_DestroySignal(Context->eventCallbackWorkerStartSignal));
        Context->eventCallbackWorkerStartSignal = gcvNULL;

        gcmVERIFY_OK(gcoCL_DestroySignal(Context->eventCallbackWorkerStopSignal));
        Context->eventCallbackWorkerStopSignal = gcvNULL;

        /* Delete event list mutex. */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL,
                                       Context->eventListMutex));
        Context->eventListMutex = gcvNULL;

        /* Delete queue list mutex. */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL,
                                       Context->queueListMutex));
        Context->queueListMutex = gcvNULL;

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL,
                                       Context->addDependencyMutex));
        Context->addDependencyMutex = gcvNULL;

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL,
                                       Context->eventCallbackListMutex));
        Context->eventCallbackListMutex = gcvNULL;

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Context->referenceCount));
        Context->referenceCount = gcvNULL;

        if (Context->devices)
        {
            gcoOS_Free(gcvNULL, Context->devices);
        }

        /* Free context. */
        gcoOS_Free(gcvNULL, Context);
    }
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return gcvSTATUS_OK;

OnError:
    if (status != CL_INVALID_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002009: (clfReleaseContext) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*                          OpenCL Context API                               *|
\*****************************************************************************/
CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(
    const cl_context_properties * Properties,
    cl_uint                       NumDevices,
    const cl_device_id *          Devices,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet
    )
{
    clsContext_PTR  context = gcvNULL;
    gctPOINTER      pointer = gcvNULL;
    gctINT          status;
    gctUINT         i;

    /*cl_context_properties prop[CONTEXT_PROPERTIES]={0};*/
    cl_context_properties  propPlatform = 0;

    gcmHEADER_ARG("Properties=0x%x NumDevices=%u Devices=0x%x",
                  Properties, NumDevices, Devices);
    gcmDUMP_API("${OCL clCreateContext 0x%x}", Properties);
    VCL_TRACE_API(CreateContext_Pre)(Properties, NumDevices, Devices, PfnNotify, UserData, ErrcodeRet);

    if (Devices == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002000: (clCreateContext) argument Devices is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (NumDevices == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002001: (clCreateContext) argument NumDevices is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (PfnNotify == gcvNULL && UserData != gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002002: (clCreateContext) argument PfnNotify is NULL but UserData is not.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Properties)
    {
        for (i = 0; Properties[i]; i++)
        {
            switch (Properties[i])
            {
            case CL_CONTEXT_PLATFORM:
                i++;
                if (Properties[i] == 0 ||
                     ((clsPlatformId_PTR)Properties[i])->objectType != clvOBJECT_PLATFORM)
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-002003: (clCreateContext) Properties[%d] not valid platform.\n", i);
                    clmRETURN_ERROR(CL_INVALID_PLATFORM);
                }
                propPlatform = Properties[i];
                break;

            case CL_GL_CONTEXT_KHR:
                i++;
                /*prop[GL_CONTEXT_KHR] = Properties[i];*/
                break;

            case CL_EGL_DISPLAY_KHR:
                i++;
                /*prop[EGL_DISPLAY_KHR] = Properties[i];*/
                break;

            case CL_GLX_DISPLAY_KHR:
            case CL_WGL_HDC_KHR:
            case CL_CGL_SHAREGROUP_KHR:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-002013: (clCreateContext) Properties[%d] (0x%x) not supported.\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);

            default:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-002014: (clCreateContext) invalid Properties[%d] (0x%x).\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);
            }
        }
    }
#if VIVANTE_PROFILER
    gcoCL_SetHardwareType(gcvHARDWARE_3D);
#endif
    /* Allocate context. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsContext),
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    gcoOS_ZeroMemory(pointer, sizeof(clsContext));

    context                  = (clsContext_PTR) pointer;
    context->objectType      = clvOBJECT_CONTEXT;
    context->dispatch        = Devices[0]->dispatch;
    context->programs        = gcvNULL;
    context->mems            = gcvNULL;
    context->queueList       = gcvNULL;
    context->eventList       = gcvNULL;
    context->eventCallbackList = gcvNULL;
    context->addDependencyMutex = gcvNULL;
    context->samplers        = gcvNULL;
    context->pfnNotify       = PfnNotify;
#if cldTUNING
    context->sortRects       = gcvFALSE;
#endif

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &context->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, context->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&context->id), CL_INVALID_VALUE);

    /* Allocate device pointers. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsDeviceId_PTR) * NumDevices,
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    context->numDevices      = NumDevices;
    context->devices         = (clsDeviceId_PTR *) pointer;

    for (i = 0; i < NumDevices; i++) context->devices[i] = Devices[i];

    if (Properties)
    {
        context->platform       = (clsPlatformId_PTR) propPlatform;
        i = 0;
        do{
            context->properties[i] = Properties[i];
            i++;
        }while(Properties[i]!=0);
        context->properties[i] = 0;
    }
    else
    {
        context->platform       = Devices[0]->platform;
        memset(context->properties, 0, sizeof(cl_context_properties)*CONTEXT_PROPERTIES_SIZE);
    }

    context->process         = gcoOS_GetCurrentProcessID();

    /* Create queue list mutex. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &context->queueListMutex),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event list mutex */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &context->eventListMutex),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event list mutex */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &context->addDependencyMutex),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event list worker thread. */

    /* Create thread start signal. */
    clmONERROR(gcoCL_CreateSignal(gcvFALSE,
                                  &context->eventListWorkerStartSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create thread stop signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &context->eventListWorkerStopSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Start the event list worker thread. */
    clmONERROR(gcoOS_CreateThread(gcvNULL,
                                  clfEventListWorker,
                                  context,
                                  &context->eventListWorkerThread),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event callback list mutex */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &context->eventCallbackListMutex),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event callback worker thread. */

    /* Create thread start signal. */
    clmONERROR(gcoCL_CreateSignal(gcvFALSE,
                                  &context->eventCallbackWorkerStartSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create thread stop signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &context->eventCallbackWorkerStopSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Delay the creation of the event callback worker thread. */
    context->eventCallbackWorkerThread = gcvNULL;


    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    gcmFOOTER_ARG("0x%x *ErrcodeRet=%d",
                  context, gcmOPT_VALUE(ErrcodeRet));

#if !gcdFPGA_BUILD
    gcoHAL_SetTimeOut(gcvNULL, 1200*gcdGPU_TIMEOUT);
#endif

    VCL_TRACE_API(CreateContext_Post)(Properties, NumDevices, Devices, PfnNotify, UserData, ErrcodeRet, context);
    return context;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002004: (clCreateContext) cannot create context.  Maybe run out of memory.\n");
    }
    if (context != gcvNULL)
    {
        if (context->referenceCount)
        {
            gcoOS_AtomDestroy(gcvNULL, context->referenceCount);
        }

        if (context->queueListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, context->queueListMutex);
        }

        if (context->eventListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, context->eventListMutex);
        }

        if (context->addDependencyMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, context->addDependencyMutex);
        }

        if (context->eventListWorkerStartSignal)
        {
            gcoCL_DestroySignal(context->eventListWorkerStartSignal);
        }

        if (context->eventListWorkerStopSignal)
        {
            gcoCL_DestroySignal(context->eventListWorkerStopSignal);
        }

        if (context->eventListWorkerThread)
        {
            gcoOS_CloseThread(gcvNULL, context->eventListWorkerThread);
        }

        if (context->eventCallbackListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL,
                              context->eventCallbackListMutex);
        }

        if (context->eventCallbackWorkerStartSignal)
        {
            gcoCL_DestroySignal(context->eventCallbackWorkerStartSignal);
        }

        if (context->eventCallbackWorkerStopSignal)
        {
            gcoCL_DestroySignal(context->eventCallbackWorkerStopSignal);
        }

        if(context->devices != gcvNULL)
        {
            gcoOS_Free(gcvNULL, context->devices);
        }

        gcoOS_Free(gcvNULL, context);
    }
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    gcmFOOTER_ARG("0x%x *ErrcodeRet=%d",
                  gcvNULL, gcmOPT_VALUE(ErrcodeRet));
    return gcvNULL;
}

CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(
    const cl_context_properties * Properties,
    cl_device_type                DeviceType,
    void                          (CL_CALLBACK * PfnNotify)(const char *, const void *, size_t, void *),
    void *                        UserData,
    cl_int *                      ErrcodeRet
    )
{
    gctINT                  status;
    clsPlatformId_PTR       platform = gcvNULL;
    clsContext_PTR          context = gcvNULL;
    int i;

    gcmHEADER_ARG("Properties=0x%x DeviceType=0x%x", Properties, DeviceType);
    gcmDUMP_API("${OCL clCreateContextFromType 0x%x, 0x%x}", Properties, DeviceType);
    VCL_TRACE_API(CreateContextFromType_Pre)(Properties, DeviceType, PfnNotify, UserData, ErrcodeRet);

    /* We support only GPU */
    if ((DeviceType & (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT)) == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002005: (clCreateContextFromType) DeviceType (0x%llx) is not CL_DEVICE_TYPE_GPU or CL_DEVICE_TYPE_DEFAULT \n",
            DeviceType);
        clmRETURN_ERROR(CL_DEVICE_NOT_FOUND);
    }

    if (Properties)
    {
        for (i = 0; Properties[i]; i++)
        {
            switch (Properties[i])
            {
            case CL_CONTEXT_PLATFORM:
                i++;
                if (Properties[i] == 0 ||
                     ((clsPlatformId_PTR)Properties[i])->objectType != clvOBJECT_PLATFORM)
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-002003: (clCreateContext) Properties[%d] not valid platform.\n", i);
                    clmRETURN_ERROR(CL_INVALID_PLATFORM);
                }
                platform    = (clsPlatformId_PTR) Properties[i];
                break;

            case CL_GL_CONTEXT_KHR:
                i++;
                break;

            case CL_EGL_DISPLAY_KHR:
                i++;
                break;

            case CL_GLX_DISPLAY_KHR:
            case CL_WGL_HDC_KHR:
            case CL_CGL_SHAREGROUP_KHR:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-002013: (clCreateContext) Properties[%d] (0x%x) not supported.\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);

            default:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-002014: (clCreateContext) invalid Properties[%d] (0x%x).\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);
            }
        }
    }
    if (platform == gcvNULL)
    {
        clfGetDefaultPlatformID(&platform);
    }

    clmASSERT(platform, CL_INVALID_PLATFORM);

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, gcvNULL, gcvNULL);

    if (platform->numDevices == 1)
    {
        context = clCreateContext(Properties, platform->numDevices, &platform->devices, PfnNotify, UserData, &status);
    }
    else
    {
        gctPOINTER      pointer = gcvNULL;
        cl_device_id*           devices = gcvNULL;
        /* Allocate device array. */
        status = gcoOS_Allocate(gcvNULL, sizeof(cl_device_id*) * platform->numDevices, &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-002016: (clCreateContextFromType) cannot allocate memory for devices.\n");
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        devices = (cl_device_id*) pointer;
        for( i = 0; i < (gctINT)platform->numDevices; i++)
        {
            devices[i] = &platform->devices[i];
        }

        context = clCreateContext(Properties, platform->numDevices, devices, PfnNotify, UserData, &status);

        if(pointer)
        {
            gcoOS_Free(gcvNULL, pointer);
            pointer = gcvNULL;
        }
    }

OnError:
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    VCL_TRACE_API(CreateContextFromType_Post)(Properties, DeviceType, PfnNotify, UserData, ErrcodeRet, context);
    gcmFOOTER_ARG("0x%x *ErrcodeRet=%d",
                  context, gcmOPT_VALUE(ErrcodeRet));
    return context;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(
    cl_context Context
    )
{
    gctINT          status;

    gcmHEADER_ARG("Context=0x%x", Context);
    gcmDUMP_API("${OCL clRetainContext 0x%x}", Context);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002007: (clRetainContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    clfONERROR(clfRetainContext(Context));

    VCL_TRACE_API(RetainContext)(Context);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(
    cl_context Context
    )
{
    gctINT          status;

    gcmHEADER_ARG("Context=0x%x", Context);
    gcmDUMP_API("${OCL clReleaseContext 0x%x}", Context);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002008: (clReleaseContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    clfONERROR(clfReleaseContext(Context));
    VCL_TRACE_API(ReleaseContext)(Context);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (status != CL_INVALID_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002009: (clReleaseContext) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(
    cl_context         Context,
    cl_context_info    ParamName,
    size_t             ParamValueSize,
    void *             ParamValue,
    size_t *           ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;
    gctINT32         referenceCount;

    gcmHEADER_ARG("Context=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Context, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetContextInfo 0x%x, 0x%x}", Context, ParamName);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002010: (clGetContextInfo) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    switch (ParamName)
    {
    case CL_CONTEXT_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, Context->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    case CL_CONTEXT_NUM_DEVICES:
        retParamSize = gcmSIZEOF(Context->numDevices);
        retParamPtr = &Context->numDevices;
        break;

    case CL_CONTEXT_DEVICES:
        retParamSize = Context->numDevices * gcmSIZEOF(*Context->devices);
        retParamPtr = Context->devices;
        break;

    case CL_CONTEXT_PROPERTIES:
        if (Context->properties[0] == 0) {   /* empty properties */
            retParamSize = gcmSIZEOF(Context->properties[0]);
            retParamPtr = &Context->properties[0];
        } else {
            /* to support no cl_gl_sharing extension condition of user case*/
            int icount = 0;
            while(Context->properties[icount] != 0)
            {
                icount++;
            }
            retParamSize = (icount+1)*sizeof(cl_context_properties);
            retParamPtr = &Context->properties;
        }
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002011: (clGetContextInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-002012: (clGetContextInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    VCL_TRACE_API(GetContextInfo)(Context, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
