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


#ifndef __gc_cl_context_h_
#define __gc_cl_context_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
***************************** Miscellaneous Types ******************************
\******************************************************************************/

typedef gceSTATUS (* clfCOMPILER) (
    IN gcoHAL Hal,
    IN gctINT ShaderType,
    IN gctSIZE_T SourceSize,
    IN gctCONST_STRING Source,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    );

/* Profile information. */
typedef struct _cl_profiler
{
    gctBOOL         enable;
    gctBOOL         perClfinish;
    gctBOOL         enableProbe;

    gctUINT32       frameNumber;
    gctUINT32       frameMaxNum;
    gctUINT64       frameStartTimeusec;
    gctUINT64       frameEndTimeusec;
    gctUINT64       frameStartCPUTimeusec;
    gctUINT64       frameEndCPUTimeusec;
}
clsProfiler;

/******************************************************************************\
************************* Context Object Definition *************************
\******************************************************************************/

typedef struct _cl_context
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsPlatformId_PTR       platform;
    gctUINT                 numDevices;
    clsDeviceId_PTR *       devices;
    clsProgram_PTR          programs;
    clsKernel_PTR           kernels;
    clsMem_PTR              mems;
    clsCommandQueue_PTR     queueList;
    gctPOINTER              queueListMutex;

    clsEvent_PTR            eventList;
    gctPOINTER              eventListMutex;

    gctPOINTER              eventListWorkerThread;
    gctSIGNAL               eventListWorkerStartSignal;
    gctSIGNAL               eventListWorkerStopSignal;

    clsEventCallback_PTR    eventCallbackList;
    gctPOINTER              eventCallbackListMutex;
    gctPOINTER              addDependencyMutex;

    clsSampler_PTR          samplers;

    cl_context_properties   properties[3];
    void                    (CL_CALLBACK * pfnNotify)(const char *, const void *, size_t, void *);

    /* Process handle. */
    gctHANDLE               process;

    gctPOINTER              eventCallbackWorkerThread;
    gctSIGNAL               eventCallbackWorkerStartSignal;
    gctSIGNAL               eventCallbackWorkerStopSignal;

    /* Profiler */
#if VIVANTE_PROFILER
    clsProfiler             profiler;
    gcoHAL                  phal;
#endif

#if cldTUNING
    gctBOOL                 sortRects;
#endif
}
clsContext;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
gctINT
clfInitializeProfiler(
    clsContext_PTR Context
    );

void
clfDestroyProfiler(
    clsContext_PTR Context
    );

gctINT
clfBeginProfiler(
    cl_command_queue CommandQueue
    );

gctINT
clfEndProfiler(
    cl_command_queue CommandQueue,
    clsKernel_PTR Kerenl
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_context_h_ */
