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
gctINT
clfInitializeProfiler(
    clsCommandQueue_PTR CommandQueue
    )
{
    gctINT status = gcvSTATUS_OK;
    gctINT profileMode = 0;
    gctCHAR *env = gcvNULL;

    gcmHEADER_ARG("CommandQueue=0x%x ", CommandQueue);

    clmCHECK_ERROR(CommandQueue == gcvNULL ||
                   CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE,
                   CL_INVALID_COMMAND_QUEUE);

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_CL_PROFILE", &env)) && env)
    {
        if gcmIS_SUCCESS(gcoOS_StrCmp(env, "0"))
        {
            gcoPROFILER_Disable();
            CommandQueue->profiler.enable = gcvFALSE;
            CommandQueue->profiler.perClfinish = gcvFALSE;
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
            CommandQueue->profiler.perClfinish = gcvTRUE;
        }
    }

    if (profileMode == 0)
    {
        CommandQueue->profiler.enable = gcvFALSE;
        CommandQueue->profiler.perClfinish = gcvFALSE;
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(CommandQueue->halProfile == gcvNULL)
    {
        status = gcoPROFILER_Construct(&CommandQueue->halProfile);
        if(status < 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
            "OCL: (Vivante Profile) Unable to create profile object.\n");
            goto OnError;
        }
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&CommandQueue->profiler, gcmSIZEOF(CommandQueue->profiler));
    CommandQueue->profiler.enable = gcvTRUE;
    CommandQueue->halProfile->profilerClient = gcvCLIENT_OPENCL;

    status = (gctINT)gcoPROFILER_Enable(CommandQueue->halProfile);

    if (status < 0)
    {
        CommandQueue->profiler.enable = gcvFALSE;
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL: (Vivante Profile) Unable to create profile object.\n");
        goto OnError;
    }

    {
        gcoPROFILER Profiler = CommandQueue->halProfile;
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char  infoRenderer[255] = {'\0'};
        char* infoDriver = "OpenCL 1.2";
        gctUINT32 chipRevision;
        gctUINT offset = 0;
        gctSTRING productName = gcvNULL;

        chipRevision = CommandQueue->context->devices[0]->deviceInfo.chipRevision;

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
    clsCommandQueue_PTR CommandQueue
    )
{
    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    if (CommandQueue->profiler.enable)
    {
        CommandQueue->profiler.enable = gcvFALSE;
        gcoPROFILER_Destroy(CommandQueue->halProfile);
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

    gcmHEADER_ARG("CommandQueue=0x%x ", CommandQueue);

    if (!CommandQueue)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(!CommandQueue->profiler.enable)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    gcoOS_GetTime(&CommandQueue->profiler.frameStartTimeusec);
    gcoPROFILER_Begin(CommandQueue->halProfile, gcvCOUNTER_OP_FRAME);

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

    gcmHEADER_ARG("CommandQueue=0x%x ", CommandQueue);

    if (!CommandQueue)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if(!CommandQueue->profiler.enable)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    {
        gcoPROFILER Profiler = CommandQueue->halProfile;
        /* write frame number */
        gcmWRITE_COUNTER(VPG_FRAME, CommandQueue->profiler.frameNumber);

        /* write gpu counters */
        gcoPROFILER_End(CommandQueue->halProfile, gcvCOUNTER_OP_FRAME, CommandQueue->profiler.frameNumber);

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
        gcoOS_GetTime(&CommandQueue->profiler.frameEndTimeusec);
        gcmWRITE_CONST(VPG_TIME);
        gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32) (CommandQueue->profiler.frameEndTimeusec
                         - CommandQueue->profiler.frameStartTimeusec));
        gcmWRITE_CONST(VPG_END);

        gcmWRITE_CONST(VPG_END);

        gcoPROFILER_Flush(CommandQueue->halProfile);
        gcmPRINT("VPC_KERNELNAME: %s\n", Kernel->name);
        gcmPRINT("VPC_ELAPSETIME: %d\n", (gctINT32) (CommandQueue->profiler.frameEndTimeusec
                         - CommandQueue->profiler.frameStartTimeusec));
        gcmPRINT("*********\n");
        CommandQueue->profiler.frameNumber++;
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

