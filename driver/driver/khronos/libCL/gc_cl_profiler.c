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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     009006

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

/*****************************************************************************\
|*                         OpenCL Profiling API                              *|
\*****************************************************************************/

CL_API_ENTRY cl_int CL_API_CALL
clGetEventProfilingInfo(
    cl_event            Event,
    cl_profiling_info   ParamName,
    size_t              ParamValueSize,
    void *              ParamValue,
    size_t *            ParamValueSizeRet
    )
{
    gctINT              status;
    gctSIZE_T           retParamSize = 0;
    gctPOINTER          retParamPtr = NULL;

    gcmHEADER_ARG("Event=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Event, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetEventProfilingInfo 0x%x, 0x%x}", Event, ParamName);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-009000: (clGetEventProfilingInfo) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    if (Event->userEvent == gcvTRUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-009001: (clGetEventProfilingInfo) Event is not a user event.\n");
        clmRETURN_ERROR(CL_PROFILING_INFO_NOT_AVAILABLE);
    }

    if (clfGetEventExecutionStatus(Event) != CL_COMPLETE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-009002: (clGetEventProfilingInfo) Event's execution status is not CL_COMPLETE.\n");
        clmRETURN_ERROR(CL_PROFILING_INFO_NOT_AVAILABLE);
    }

    if ((Event->queue->properties & CL_QUEUE_PROFILING_ENABLE) == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-009003: (clGetEventProfilingInfo) Event's queue is not enabled for profiling.\n");
        clmRETURN_ERROR(CL_QUEUE_PROFILING_ENABLE);
    }

    switch (ParamName)
    {
    case CL_PROFILING_COMMAND_QUEUED:
        retParamSize = gcmSIZEOF(Event->profileInfo.queued);
        retParamPtr = &Event->profileInfo.queued;
        break;

    case CL_PROFILING_COMMAND_SUBMIT:
        retParamSize = gcmSIZEOF(Event->profileInfo.submit);
        retParamPtr = &Event->profileInfo.submit;
        break;

    case CL_PROFILING_COMMAND_START:
        retParamSize = gcmSIZEOF(Event->profileInfo.start);
        retParamPtr = &Event->profileInfo.start;
        break;

    case CL_PROFILING_COMMAND_END:
        retParamSize = gcmSIZEOF(Event->profileInfo.end);
        retParamPtr = &Event->profileInfo.end;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-009004: (clGetEventProfilingInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-009005: (clGetEventProfilingInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetEventProfilingInfo)(Event, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*                         Vivante Profile functions                          *|
\*****************************************************************************/
#if VIVANTE_PROFILER
#define gcmWRITE_CONST(ConstValue) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = ConstValue; \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profilerObj, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_VALUE(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profilerObj, gcmSIZEOF(value), &value)); \
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
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profilerObj, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profilerObj, length, String)); \
    } \
    while (gcvFALSE)

gctINT
clfInitializeProfiler(
    clsContext_PTR Context
    )
{
    gctINT status = gcvSTATUS_OK;
    gctINT profileMode = 0;
    gctCHAR *env = gcvNULL;
#ifdef ANDROID
    gctBOOL matchResult = gcvFALSE;
#endif

    gcmHEADER_ARG("Context=0x%x ", Context);

    clmCHECK_ERROR(Context == gcvNULL ||
                   Context->objectType != clvOBJECT_CONTEXT,
                   CL_INVALID_CONTEXT);

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_CL_PROFILE", &env)) && env)
    {
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "0"))
        {
            gcoPROFILER_NEW_Disable();
            Context->profiler.enable = gcvFALSE;
            Context->profiler.perClfinish = gcvFALSE;
            gcmFOOTER_ARG("%d", status);
            return status;
        }
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "1"))
        {
            profileMode = 1;
        }
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "2"))
        {
            profileMode = 2;
            Context->profiler.perClfinish = gcvTRUE;
        }
    }

    if (profileMode == 0)
    {
        Context->profiler.enable = gcvFALSE;
        Context->profiler.perClfinish = gcvFALSE;
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(Context->halProfile == gcvNULL)
    {
        status = gcoPROFILER_NEW_Construct(&Context->halProfile);
        if(status < 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL: (Vivante Profile) Unable to create profile object.\n");
            goto OnError;
        }
    }

    status = (gctINT)gcoPROFILER_NEW_Enable(Context->halProfile);
    if ( status < 0)
    {
        Context->profiler.enable = gcvFALSE;
        gcmUSER_DEBUG_ERROR_MSG(
        "OCL: (Vivante Profile) Unable to create profile object.\n");
        goto OnError;
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

    gcoOS_GetEnv(gcvNULL, "VP_OUTPUT", &env);
    if ((env != gcvNULL) && *env != '\0')
    {
        Context->halProfile->fileName = env;
    }

#ifdef ANDROID
    gcoOS_GetEnv(gcvNULL, "VP_PROCESS_NAME", &env);
    if ((env != gcvNULL) && (env[0] != 0)) matchResult = (gcoOS_DetectProcessByName(env) ? gcvTRUE : gcvFALSE);
    if (matchResult != gcvTRUE) {
        gcmFOOTER();
        return gcvSTATUS_MISMATCH;
    }
#endif

    {
        gcoPROFILER profilerObj = Context->halProfile;
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char  infoRenderer[255] = {'\0'};
        char* infoDriver = "OpenCL 1.2";
        gctUINT32 chipRevision;
        gctUINT offset = 0;
        gctSTRING productName = gcvNULL;

        chipRevision = Context->devices[0]->deviceInfo.chipRevision;

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
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->profiler.enable)
    {
        Context->profiler.enable = gcvFALSE;
        gcoPROFILER_NEW_Destroy(Context->halProfile);
    }

    gcmFOOTER_NO();
    return;
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
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
    gcoPROFILER_NEW_Begin(Context->halProfile, gcvCOUNTER_OP_DRAW);

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
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    {
        gcoPROFILER profilerObj = Context->halProfile;
        /* write frame number */
        gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);

        /* write gpu counters */
        gcoPROFILER_NEW_EndFrame(Context->halProfile, gcvCOUNTER_OP_NONE);

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

        gcoPROFILER_NEW_Flush(Context->halProfile);
        gcmPRINT("VPC_KERNELNAME: %s\n", Kernel->name);
        gcmPRINT("VPC_ELAPSETIME: %d\n", (gctINT32) (Context->profiler.frameEndTimeusec
                         - Context->profiler.frameStartTimeusec));
        gcmPRINT("*********\n");
        Context->profiler.frameNumber++;
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

