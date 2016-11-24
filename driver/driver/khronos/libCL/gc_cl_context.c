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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     002016

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

#define gcmWRITE_CONST(ConstValue) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = ConstValue; \
        gcmERR_BREAK(gcoPROFILER_Write(Context->phal, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_VALUE(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_Write(Context->phal, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_COUNTER(Counter, Value) \
    gcmWRITE_CONST(Counter); \
    gcmWRITE_VALUE(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING(String) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 length; \
        length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
        gcmERR_BREAK(gcoPROFILER_Write(Context->phal, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_Write(Context->phal, length, String)); \
    } \
    while (gcvFALSE)

#if VIVANTE_PROFILER_PROBE
#define gcmWRITE_XML_STRING(String) \
    do \
    { \
    gceSTATUS status; \
    gctINT32 length; \
    length = (gctINT32) gcoOS_StrLen((gctSTRING) String, gcvNULL); \
    gcmERR_BREAK(gcoPROFILER_Write(Context->phal, length, String)); \
    } \
    while (gcvFALSE)

#endif

gctINT
clfInitializeProfiler(
    clsContext_PTR Context
    )
{
    gctINT status = gcvSTATUS_OK;
    gctINT profileMode = 0;
    char *env = gcvNULL;

    gcmHEADER_ARG("Context=0x%x ", Context);

    clmCHECK_ERROR(Context == gcvNULL ||
                   Context->objectType != clvOBJECT_CONTEXT,
                   CL_INVALID_CONTEXT);

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_CL_PROFILE", &env)) && env)
    {
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "1"))
        {
            profileMode = 1;
        }
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "2"))
        {
            profileMode = 2;
        }
    }

    if (profileMode == 0)
    {
        Context->profiler.enable = gcvFALSE;
        Context->profiler.perClfinish = gcvFALSE;
#if VIVANTE_PROFILER_PROBE
        if(Context->phal == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                           gcmSIZEOF(struct _gcoHAL),
                           &pointer)))
            {
                gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", status);
                return status;
            }
            gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
            Context->phal = (gcoHAL) pointer;
            Context->profiler.enableProbe = gcvTRUE;
            if (gcoPROFILER_Initialize(Context->phal, gcvNULL, gcvFALSE) != gcvSTATUS_OK)
                Context->profiler.enableProbe = gcvFALSE;
            if (Context->profiler.enableProbe)
                gcmWRITE_XML_STRING("<DrawCounter>\n");
        }
#endif
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(Context->phal == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoHAL),
                       &pointer)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            gcmFOOTER_ARG("%d", status);
            return status;
        }
        gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
        Context->phal = (gcoHAL) pointer;
    }

    status = gcoPROFILER_Initialize(Context->phal, gcvNULL, gcvTRUE);
    switch (status)
    {
        case gcvSTATUS_OK:
            break;
        case gcvSTATUS_MISMATCH: /*fall through*/
        case gcvSTATUS_NOT_SUPPORTED:
        default:
            Context->profiler.enable = gcvFALSE;
            if(Context->phal != gcvNULL)
                gcoOS_Free(gcvNULL, Context->phal);
            gcmFOOTER_ARG("%d", status);
            return status;
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&Context->profiler, gcmSIZEOF(Context->profiler));
    Context->profiler.enable = gcvTRUE;

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VP_FRAME_NUM", &env)))
    {
        if ((env != gcvNULL) && (env[0] !=0))
        {
            int frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                Context->profiler.frameMaxNum = (gctUINT32)frameNum;
        }
    }

    if (profileMode == 2)
    {
        Context->profiler.perClfinish = gcvTRUE;
    }

    {
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char  infoRenderer[255] = {'\0'};
        char* infoDriver = "OpenCL 1.2";
        gceCHIPMODEL chipModel;
        gctUINT32 chipRevision;
        gctUINT offset = 0;
        gctSTRING productName = gcvNULL;

        gcoHAL_QueryChipIdentity(gcvNULL,&chipModel, &chipRevision,gcvNULL,gcvNULL);

#define BCD(digit)      ((chipRevision >> (digit * 4)) & 0xF)
        gcoOS_MemFill(infoRevision, 0, gcmSIZEOF(infoRevision));
        if (BCD(3) == 0)
        {
            /* Old format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d\" ", BCD(1), BCD(0));
        }
        else
        {
            /* New format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d.%d_rc%d\" ",
                BCD(3), BCD(2), BCD(1), BCD(0));
        }

        gcoHAL_GetProductName(gcvNULL, &productName);
        gcoOS_StrCatSafe(infoRenderer, 9, "Vivante ");
        gcoOS_StrCatSafe(infoRenderer, 23, productName);
        gcmOS_SAFE_FREE(gcvNULL, productName);

        gcmWRITE_CONST(VPG_INFO);

        gcmWRITE_CONST(VPC_INFOCOMPANY);
        gcmWRITE_STRING(infoCompany);
        gcmWRITE_CONST(VPC_INFOVERSION);
        gcmWRITE_STRING(infoVersion);
        gcmWRITE_CONST(VPC_INFORENDERER);
        gcmWRITE_STRING(infoRenderer);
        gcmWRITE_CONST(VPC_INFOREVISION);
        gcmWRITE_STRING(infoRevision);
        gcmWRITE_CONST(VPC_INFODRIVER);
        gcmWRITE_STRING(infoDriver);

        gcmWRITE_CONST(VPG_END);
    }

    gcoOS_GetTime(&Context->profiler.frameStartTimeusec);

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

void
clfDestroyProfiler(
    clsContext_PTR Context
    )
{
    gcsHAL_INTERFACE iface;
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->profiler.enable)
    {
        /* disable profiler in kernel. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_SET_PROFILE_SETTING;
        iface.u.SetProfileSetting.enable = gcvFALSE;

        /* Call the kernel. */
        gcoOS_DeviceControl(gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface));

        Context->profiler.enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(Context->phal));
        if(Context->phal != gcvNULL)
            gcoOS_Free(gcvNULL, Context->phal);
    }
    else
    {
#if VIVANTE_PROFILER_PROBE
        if (Context->profiler.enableProbe)
            gcmVERIFY_OK(gcoPROFILER_Destroy(Context->phal));
        if (Context->phal != gcvNULL)
            gcoOS_Free(gcvNULL, Context->phal);
#endif
    }
    gcmFOOTER_NO();
}

gctINT
clfBeginProfiler(
    cl_command_queue CommandQueue
    )
{
    gctINT status = gcvSTATUS_OK;
    clsContext_PTR      Context;

    gcmHEADER_ARG("CommandQueue=0x%x ", CommandQueue);

    if (!CommandQueue)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    Context = CommandQueue->context;
    if (!Context)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if(!Context->profiler.enable)
    {
#if VIVANTE_PROFILER_PROBE
        if (Context->profiler.enableProbe)
            gcoPROFILER_Begin(Context->phal, gcvCOUNTER_OP_NONE);
#endif
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
    gcoPROFILER_Begin(Context->phal, 0);

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfEndProfiler(
    cl_command_queue CommandQueue,
    clsKernel_PTR Kernel
    )
{
    gctINT status = gcvSTATUS_OK;
    clsContext_PTR      Context;

    gcmHEADER_ARG("CommandQueue=0x%x ", CommandQueue);

    if (!CommandQueue)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    Context = CommandQueue->context;
    if (!Context)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if(!Context->profiler.enable)
    {
#if VIVANTE_PROFILER_PROBE
        if (Context->profiler.enableProbe)
        {
            gcoPROFILER_EndFrame(Context->phal);
            gcoPROFILER_Flush(Context->phal);
            Context->profiler.frameNumber++;
        }
#endif
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    /* write frame number */
    gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);

    /* write gpu counters */
    gcoPROFILER_EndFrame(Context->phal);

    /* write kernel info */
    gcmWRITE_CONST(VPG_PROG);
    gcmWRITE_COUNTER(VPC_PROGRAMHANDLE, gcmPTR2INT32(Kernel));
    gcmWRITE_CONST(VPG_PVS);
    gcmWRITE_CONST(VPC_PVSSOURCE);
    gcmWRITE_STRING(Kernel->name);
    gcmWRITE_CONST(VPG_END);
    gcmWRITE_CONST(VPG_PPS);
    /* TODO: need to find these data
    gcmWRITE_COUNTER(VPC_PPSINSTRCOUNT, (tex + alu));
    gcmWRITE_COUNTER(VPC_PPSALUINSTRCOUNT, alu);
    gcmWRITE_COUNTER(VPC_PPSTEXINSTRCOUNT, tex);
    gcmWRITE_COUNTER(VPC_PPSATTRIBCOUNT, (GetShaderAttributeCount(Shader)));
    gcmWRITE_COUNTER(VPC_PPSUNIFORMCOUNT, (GetShaderUniformCount(Shader)));
    gcmWRITE_COUNTER(VPC_PPSFUNCTIONCOUNT, (GetShaderFunctionCount(Shader))); */
    if (Kernel->program && Kernel->program->source)
    {
        gcmWRITE_CONST(VPC_PPSSOURCE);
        gcmWRITE_STRING(Kernel->program->source);
    }
    gcmWRITE_CONST(VPG_END);
    gcmWRITE_CONST(VPG_END);

    /* write frame time */
    gcoOS_GetTime(&Context->profiler.frameEndTimeusec);
    gcmWRITE_CONST(VPG_TIME);
    gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32) (Context->profiler.frameEndTimeusec
                     - Context->profiler.frameStartTimeusec));
    gcmWRITE_CONST(VPG_END);

    gcmWRITE_CONST(VPG_END);

    gcoPROFILER_Flush(Context->phal);
    gcmPRINT("VPC_KERNELNAME: %s\n", Kernel->name);
    gcmPRINT("VPC_ELAPSETIME: %d\n", (gctINT32) (Context->profiler.frameEndTimeusec
                     - Context->profiler.frameStartTimeusec));
    gcmPRINT("*********\n");
    Context->profiler.frameNumber++;

    /* Return the status. */
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
#if !gcdFPGA_BUILD
    gceCHIPMODEL  chipModel;
    gctUINT32 chipRevision;
#endif

    gcmHEADER_ARG("Properties=0x%x NumDevices=%u Devices=0x%x",
                  Properties, NumDevices, Devices);

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
    gcoCL_SetHardware();
    /* Allocate context. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsContext),
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    context                  = (clsContext_PTR) pointer;
    context->objectType      = clvOBJECT_CONTEXT;
    context->dispatch        = Devices[0]->dispatch;
    context->programs        = gcvNULL;
    context->kernels         = gcvNULL;
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
#if VIVANTE_PROFILER
    context->phal            = gcvNULL;
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
        context->platform       = (clsPlatformId_PTR) Properties[1];
        context->properties[0]  = Properties[0];    /* CL_CONTEXT_PLATFORM */
        context->properties[1]  = Properties[1];    /* clsPlatformId_PTR */
        context->properties[2]  = Properties[2];    /* 0 to terminate the list */
    }
    else
    {
        context->platform       = Devices[0]->platform;
        context->properties[0]  = 0;                /* CL_CONTEXT_PLATFORM */
        context->properties[1]  = 0;                /* clsPlatformId_PTR */
        context->properties[2]  = 0;                /* 0 to terminate the list */
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

#if VIVANTE_PROFILER
    clfInitializeProfiler(context);
#endif

#if !gcdFPGA_BUILD
    gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);
    if((chipModel == gcv3000 && chipRevision == 0x5435) || (chipModel == gcv7000 && chipRevision == 0x6008))
    {
        gcoHAL_SetTimeOut(gcvNULL, 1200*gcdGPU_TIMEOUT);
    }
#endif

    return context;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002004: (clCreateContext) cannot create context.  Maybe run out of memory.\n");
    }

    if(context != gcvNULL && context->devices != gcvNULL)
    {
        gcoOS_Free(gcvNULL, context->devices);
    }
    if(context != gcvNULL)
    {
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

    gcmHEADER_ARG("Properties=0x%x DeviceType=0x%x", Properties, DeviceType);

    /* We support only GPU */
    if ((DeviceType & CL_DEVICE_TYPE_GPU) == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002005: (clCreateContextFromType) DeviceType (0x%x) is not CL_DEVICE_TYPE_GPU (0x%x).\n",
            DeviceType, CL_DEVICE_TYPE_GPU);
        clmRETURN_ERROR(CL_DEVICE_NOT_FOUND);
    }

    if (Properties != gcvNULL &&
        (Properties[0] != CL_CONTEXT_PLATFORM ||
         Properties[1] == 0 ||
         ((clsPlatformId_PTR)Properties[1])->objectType != clvOBJECT_PLATFORM ||
         Properties[2] != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002006: (clCreateContextFromType) argument Properties provides invalid platform.\n");
        clmRETURN_ERROR(CL_INVALID_PLATFORM);
    }

    if (Properties != gcvNULL)
    {
        platform    = (clsPlatformId_PTR) Properties[1];
    }
    else
    {
        clfGetDefaultPlatformID(&platform);
    }

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, gcvNULL, gcvNULL);

    if (platform->numDevices == 1)
    {
        context = clCreateContext(Properties, platform->numDevices, &platform->devices, PfnNotify, UserData, &status);
    }
    else
    {
        gctPOINTER      pointer = gcvNULL;
        cl_device_id*           devices = gcvNULL;
        int i;
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

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002007: (clRetainContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Context->referenceCount, gcvNULL));

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
    gctINT32        oldReference;
    gceCHIPMODEL  chipModel;
    gctUINT32 chipRevision;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-002008: (clReleaseContext) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }
    gcoCL_SetHardware();
    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Context->referenceCount, &oldReference));

    if (oldReference == 1)
    {
#if VIVANTE_PROFILER
        /* Destroy the profiler. */
        clfDestroyProfiler(Context);
#endif

        /* Send signal to stop event list worker thread. */
        gcmONERROR(gcoCL_SetSignal(Context->eventListWorkerStopSignal));

        gcmONERROR(gcoCL_SetSignal(Context->eventListWorkerStartSignal));

        /* Send signal to stop event callback worker thread. */
        gcmONERROR(gcoCL_SetSignal(Context->eventCallbackWorkerStopSignal));

        gcmONERROR(gcoCL_SetSignal(Context->eventCallbackWorkerStartSignal));

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

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Context->referenceCount));
        Context->referenceCount = gcvNULL;

        /* Free context. */
        gcoOS_Free(gcvNULL, Context);
    }

    gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);
    if((chipModel == gcv3000 && chipRevision == 0x5435) || (chipModel == gcv7000 && chipRevision == 0x6008))
    {
        gcoHAL_SetTimeOut(gcvNULL, gcdGPU_TIMEOUT);
    }

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
            retParamSize = gcmSIZEOF(Context->properties);
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

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
