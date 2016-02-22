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


#ifndef __GC_VX_RUNTIME_H__
#define __GC_VX_RUNTIME_H__

EXTERN_C_BEGIN

/* Debug APIs */
typedef enum _vx_trace_target_e
{
    VX_TRACE_PLATFORM   = 0,
    VX_TRACE_PERF,
    VX_TRACE_REF,
    VX_TRACE_CONTEXT,
    VX_TRACE_GRAPH,
    VX_TRACE_TARGET,
    VX_TRACE_NODE,
    VX_TRACE_KERNEL,
    VX_TRACE_SCALAR,
    VX_TRACE_MEMORY,
    VX_TRACE_ARRAY,
    VX_TRACE_IMAGE,

    VX_TRACE_TARGET_COUNT
}
vx_trace_target_e;

VX_INTERNAL_API void vxEnableRuntimeDebugMode(vx_bool enable);

VX_INTERNAL_API vx_bool vxInRuntimeDebugMode();

VX_INTERNAL_API void vxEnableAllTraceTargets(vx_bool enable);

VX_INTERNAL_API void vxEnableTraceTarget(vx_trace_target_e target, vx_bool enable);

VX_INTERNAL_API void vxTrace(vx_trace_target_e target,  char *message, ...);

VX_INTERNAL_API void vxWarning(char *message, ...);

VX_INTERNAL_API void vxError(char *message, ...);

/* Heap & Memory APIs */
VX_INTERNAL_API vx_ptr vxAllocate(vx_size size);

VX_INTERNAL_API void vxFree(vx_ptr memory);

VX_INTERNAL_API vx_ptr vxAllocateAndZeroMemory(vx_size size);

VX_INTERNAL_API void vxZeroMemory(vx_ptr memory, vx_size size);

VX_INTERNAL_API void vxMemCopy(vx_ptr dest, vx_const_ptr src, vx_size bytes);

/* String APIs */
VX_INTERNAL_API void vxStrCopySafe(vx_string dest, vx_size size, vx_const_string src);

VX_INTERNAL_API vx_bool vxIsSameString(vx_const_string string1, vx_const_string string2, vx_size count);

VX_INTERNAL_API vx_string vxStrDup(vx_const_string string);

/* Thread APIs */
VX_INTERNAL_API vx_thread vxCreateThread(vx_thread_routine_f func, void *arg);

VX_INTERNAL_API vx_bool vxCloseThread(vx_thread thread);

VX_INTERNAL_API void vxSleep(vx_uint32 milliseconds);

/* Mutex APIs */
VX_INTERNAL_API vx_bool vxCreateMutex(OUT vx_mutex *mutex);

VX_INTERNAL_API void vxDestroyMutex(vx_mutex mutex);

VX_INTERNAL_API vx_bool vxAcquireMutex(vx_mutex mutex);

VX_INTERNAL_API vx_bool vxTryAcquireMutex(vx_mutex mutex, vx_uint32 timeout);

VX_INTERNAL_API vx_bool vxReleaseMutex(vx_mutex mutex);

/* Event APIs */
VX_INTERNAL_API vx_bool vxCreateEvent(vx_bool manualReset, OUT vx_event *event);

VX_INTERNAL_API void vxDestroyEvent(vx_event event);

VX_INTERNAL_API vx_bool vxWaitEvent(vx_event event, vx_uint32 timeout);

VX_INTERNAL_API vx_bool vxSetEvent(vx_event event);

VX_INTERNAL_API vx_bool vxResetEvent(vx_event event);

/* Module APIs */
VX_INTERNAL_API vx_module_handle vxLoadModule(vx_const_string name);

VX_INTERNAL_API void vxUnloadModule(vx_module_handle moduleHandle);

vx_symbol_handle vxGetSymbol(vx_module_handle mod, vx_char *name);

/* Perf APIs */
VX_INTERNAL_API vx_float64 vxConvertPerfCountToMS(vx_uint64 count);

VX_INTERNAL_API void vxoPerf_Initialize(vx_perf perf);

VX_INTERNAL_API void vxoPerf_Begin(vx_perf perf);

VX_INTERNAL_API void vxoPerf_End(vx_perf perf);

VX_INTERNAL_API void vxoPerf_Dump(vx_perf perf);

/* Queue APIs */
VX_INTERNAL_API void vxoQueue_Initialize(vx_queue queue);

VX_INTERNAL_API void vxoQueue_Deinitialize(vx_queue queue);

VX_INTERNAL_API void vxoQueue_Stop(vx_queue queue);

VX_INTERNAL_API vx_bool vxoQueue_WriteData(vx_queue queue, vx_value_set data);

VX_INTERNAL_API vx_bool vxoQueue_ReadData(vx_queue queue, OUT vx_value_set_ptr dataPtr);

#if VX_USE_THREADPOOL
/* ThreadPool APIs */
VX_INTERNAL_API vx_threadpool vxoThreadPool_Create(
        vx_uint32 workerCount, vx_uint32 maxWorkItemCount, vx_size workItemSize,
        vx_threadpool_worker_callback_f workerCallback, vx_ptr workerArg);

VX_INTERNAL_API void vxoThreadPool_Destroy(vx_threadpool_ptr threadPoolPtr);

VX_INTERNAL_API vx_bool vxoThreadPool_QueueWorkItems(
        vx_threadpool threadPool, vx_value_set_s workItems[], vx_uint32 workItemCount);

VX_INTERNAL_API vx_bool vxoThreadpool_IsCompleted(vx_threadpool threadPool, vx_bool wait);
#endif

EXTERN_C_END

#endif /* __GC_VX_RUNTIME_H__ */
