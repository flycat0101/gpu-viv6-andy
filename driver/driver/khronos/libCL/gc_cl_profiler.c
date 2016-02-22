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

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
