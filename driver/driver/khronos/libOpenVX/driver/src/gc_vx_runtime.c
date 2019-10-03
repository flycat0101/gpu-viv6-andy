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


#if defined __QNXNTO__ || defined ANDROID
#include <ctype.h>
#endif
#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

#define VX_TRACE_BUFFER_COUNT      2048

static vx_bool vxTraceTargetStates[VX_TRACE_TARGET_COUNT];

static vx_bool vxRuntimeDebugMode = vx_true_e;

VX_INTERNAL_API void vxEnableRuntimeDebugMode(vx_bool enable)
{
    vxRuntimeDebugMode = enable;
}

VX_INTERNAL_API vx_bool vxInRuntimeDebugMode()
{
    return vxRuntimeDebugMode;
}

VX_INTERNAL_API void vxEnableAllTraceTargets(vx_bool enable)
{
    int i;

    for (i = 0; i < VX_TRACE_TARGET_COUNT; i++)
    {
        vxTraceTargetStates[i] = enable;
    }
}

VX_INTERNAL_API void vxEnableTraceTarget(vx_trace_target_e target, vx_bool enable)
{
    vxTraceTargetStates[target] = enable;
}

VX_INTERNAL_API void vxTrace(vx_trace_target_e target, char *message, ...)
{
    if (vxTraceTargetStates[target])
    {
        va_list argList;
        char outputBuffer[VX_TRACE_BUFFER_COUNT];

        va_start(argList, message);

        vsnprintf(outputBuffer, VX_TRACE_BUFFER_COUNT - 1, message, argList);
        outputBuffer[VX_TRACE_BUFFER_COUNT - 1] = 0;

        //gcmTRACE(gcvLEVEL_INFO, outputBuffer);
        vxInfo(outputBuffer);

        va_end(argList);
    }
}

VX_INTERNAL_API vx_ptr vxAllocate(vx_size size)
{
    gceSTATUS status;
    vx_ptr memory;

    gcmHEADER_ARG("size=0x%lx", size);

    status = gcoOS_Allocate(gcvNULL, size, &memory);

    if (gcmIS_ERROR(status))
    {
        vxError("Failed to allocate enough memory");
        gcmFOOTER_ARG("%d", status);
        return VX_NULL;
    }

    gcoOS_ZeroMemory(memory, size);
    gcmFOOTER_ARG("%d", status);
    return memory;
}

VX_INTERNAL_API vx_ptr vxAllocateAndZeroMemory(vx_size size)
{
    return vxAllocate(size);
}

VX_INTERNAL_API void vxFree(vx_ptr memory)
{
    gcmVERIFY_OK(gcoOS_Free(gcvNULL, memory));
}

VX_INTERNAL_API void vxZeroMemory(vx_ptr memory, vx_size size)
{
    gcoOS_ZeroMemory(memory, size);
}

VX_INTERNAL_API void vxMemCopy(vx_ptr dest, vx_const_ptr src, vx_size bytes)
{
    gcoOS_MemCopy(dest, src, bytes);
}

/* String APIs */
VX_INTERNAL_API void vxStrCopySafe(vx_string dest, vx_size size, vx_const_string src)
{
    gcmVERIFY_OK(gcoOS_StrCopySafe(dest, size, src));
}

VX_INTERNAL_API vx_bool vxIsSameString(vx_const_string string1, vx_const_string string2, vx_size count)
{
    gceSTATUS status = gcoOS_StrNCmp(string1, string2, count);

    return (vx_bool)(status == gcvSTATUS_OK);
}

VX_INTERNAL_API vx_string vxStrDup(vx_const_string string)
{
    vx_string target;

    gcoOS_StrDup(gcvNULL, string, &target);

    return target;
}

VX_INTERNAL_API vx_bool vxStrToLower(vx_const_string srcString, vx_string lowerString)
{
    gcmHEADER_ARG("srcString=%s, lowerString=%s", srcString, lowerString);

    if (srcString != VX_NULL && lowerString!= VX_NULL)
    {
        vx_uint32 i;

        /* to lower case */
        for (i = 0; srcString[i] != 0; i++)
        {
            lowerString[i] = (char)tolower(srcString[i]);
        }
        gcmFOOTER_NO();
        return vx_true_e;
    }

    gcmFOOTER_NO();
    return vx_false_e;
}

/* Mutex APIs */
VX_INTERNAL_API vx_bool vxCreateMutex(OUT vx_mutex *mutex)
{
    gceSTATUS status = gcoOS_CreateMutex(gcvNULL, mutex);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API void vxDestroyMutex(vx_mutex mutex)
{
    gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, mutex));
}

VX_INTERNAL_API vx_bool vxAcquireMutex(vx_mutex mutex)
{
    gceSTATUS status = gcoOS_AcquireMutex(gcvNULL, mutex, gcvINFINITE);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API vx_bool vxTryAcquireMutex(vx_mutex mutex, vx_uint32 timeout)
{
    gceSTATUS status = gcoOS_AcquireMutex(gcvNULL, mutex, timeout);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API vx_bool vxReleaseMutex(vx_mutex mutex)
{
    gceSTATUS status = gcoOS_ReleaseMutex(gcvNULL, mutex);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API vx_bool vxCreateEvent(vx_bool manualReset, OUT vx_event *event)
{
    gceSTATUS status = gcoOS_CreateSignal(gcvNULL, manualReset, event);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API void vxDestroyEvent(vx_event event)
{
    gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, event));
}

VX_INTERNAL_API vx_bool vxWaitEvent(vx_event event, vx_uint32 timeout)
{
    gceSTATUS status = gcoOS_WaitSignal(gcvNULL, event, timeout);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API vx_bool vxSetEvent(vx_event event)
{
    gceSTATUS status = gcoOS_Signal(gcvNULL, event, gcvTRUE);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API vx_bool vxResetEvent(vx_event event)
{
    gceSTATUS status = gcoOS_Signal(gcvNULL, event, gcvFALSE);

    return (vx_bool)gcmIS_SUCCESS(status);
}

VX_INTERNAL_API void vxSleep(vx_uint32 milliseconds)
{
    gcmVERIFY_OK(gcoOS_Delay(gcvNULL, milliseconds));
}

VX_INTERNAL_API vx_thread vxCreateThread(vx_thread_routine_f routine, void *arg)
{
    gctPOINTER thread;

    gceSTATUS status = gcoOS_CreateThread(gcvNULL, (gcTHREAD_ROUTINE)routine, arg, &thread);

    if (gcmIS_ERROR(status)) return gcvNULL;

    return thread;
}

VX_INTERNAL_API vx_bool vxCloseThread(vx_thread thread)
{
    gceSTATUS status = gcoOS_CloseThread(gcvNULL, thread);

    return (vx_bool)gcmIS_SUCCESS(status);
}


VX_INTERNAL_API vx_module_handle vxLoadModule(vx_const_string name)
{
    gceSTATUS           status;
    vx_module_handle    handle;
    gcmHEADER_ARG("name=%s", name);
    vxmASSERT(name);

    gcmONERROR(gcoOS_LoadLibrary(gcvNULL, name, &handle));

    gcmFOOTER_NO();
    return handle;

OnError:
    gcmFOOTER_NO();
    return VX_NULL_MODULE_HANDLE;
}

VX_INTERNAL_API void vxUnloadModule(vx_module_handle moduleHandle)
{
    gcmHEADER_ARG("moduleHandle=%p", moduleHandle);
    if (moduleHandle == (vx_module_handle)1)
    {
        gcmFOOTER_NO();
        return;
    }
    else
    {
        vxmASSERT(moduleHandle != VX_NULL_MODULE_HANDLE);

        gcoOS_FreeLibrary(gcvNULL, moduleHandle);
    }
    gcmFOOTER_NO();
}

vx_symbol_handle vxGetSymbol(vx_module_handle mod, vx_char *name)
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__) || defined(__QNX__) || defined(__CYGWIN__)

    return (vx_symbol_handle)dlsym(mod, name);

#elif defined(_WIN32)

    return (vx_symbol_handle)GetProcAddress(mod, name);

#endif
}

/* Perf APIs */
#define BILLION                                 1000000000

VX_PRIVATE_API vx_uint64 vxGetPerfCount()
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (vx_uint64)((vx_uint64)ts.tv_nsec + (vx_uint64)ts.tv_sec * BILLION);
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER ln;

    QueryPerformanceCounter(&ln);

    return (vx_uint64)ln.QuadPart;
#endif
}

/* unused code
VX_PRIVATE_API vx_uint64 vxGetPerfFreq()
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec ts;

    clock_getres(CLOCK_MONOTONIC, &ts);

    return (vx_uint64)(BILLION / ts.tv_nsec);
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER ln;

    QueryPerformanceFrequency(&ln);

    return (vx_uint64)ln.QuadPart;
#endif
}
*/
#define NS_PER_MSEC  (1000000.0f)

VX_INTERNAL_API vx_float64 vxConvertPerfCountToMS(vx_uint64 count)
{
    return (vx_float64)count / NS_PER_MSEC;
}

VX_INTERNAL_API void vxoPerf_Initialize(vx_perf perf)
{
    gcmHEADER_ARG("perf=%p", perf);
    vxmASSERT(perf);

    vxZeroMemory(perf, sizeof(vx_perf_t));

    perf->min = UINT64_MAX;
    gcmFOOTER_NO();
}

static vx_uint64 lastCount = 0;
VX_INTERNAL_API void vxoPerf_Begin(vx_perf perf)
{
    gcmHEADER_ARG("perf=%p", perf);
    vxmASSERT(perf);

    if (lastCount == 0)
    {
        lastCount = vxGetPerfCount();
        perf->beg = lastCount;
    }
    else
    {
        vx_uint64 cur = vxGetPerfCount();
        if (lastCount >= cur)
             perf->beg = lastCount + 1;
        else
             perf->beg = cur;
        lastCount = perf->beg;
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoPerf_End(vx_perf perf)
{
    vx_uint64 cur;
    gcmHEADER_ARG("perf=%p", perf);
    vxmASSERT(perf);

    cur = vxGetPerfCount();
    if (cur <= perf->beg)
        perf->end = perf->beg + 1;
    else
        perf->end = cur;

    perf->tmp = ((perf->end - perf->beg) < 1ull ? 1ull : (perf->end - perf->beg));
    perf->sum += perf->tmp;
    perf->num++;
    perf->avg = perf->sum / perf->num;
    perf->min = (perf->min < perf->tmp ? perf->min : perf->tmp);
    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoPerf_Dump(vx_perf perf)
{
    if (perf == VX_NULL)
    {
        vxTrace(VX_TRACE_PERF, "<perf>null</perf>\n");
    }
    else
    {
        vxTrace(VX_TRACE_PERF,
                "<perf>\n"
                "   <address>%p</address>\n"
                "   <tmp>"VX_FORMAT_MS"</tmp>\n"
                "   <beg>"VX_FORMAT_MS"</beg>\n"
                "   <end>"VX_FORMAT_MS"</end>\n"
                "   <sum>"VX_FORMAT_MS"</sum>\n"
                "   <avg>"VX_FORMAT_MS"</avg>\n"
                "   <min>"VX_FORMAT_MS"</min>\n"
                "   <num>%llu</num>\n"
                "</perf>",
                perf,
                vxConvertPerfCountToMS(perf->tmp),
                vxConvertPerfCountToMS(perf->beg),
                vxConvertPerfCountToMS(perf->end),
                vxConvertPerfCountToMS(perf->sum),
                vxConvertPerfCountToMS(perf->avg),
                vxConvertPerfCountToMS(perf->min),
                perf->num);
    }
}

VX_INTERNAL_API void vxoQueue_Initialize(vx_queue queue)
{
    gcmHEADER_ARG("queue=%p", queue);
    vxmASSERT(queue);

    if (queue == VX_NULL)
    {
        gcmFOOTER_NO();
        return;
    }
    vxZeroMemory(queue->data, sizeof(queue->data));

    vxCreateMutex(OUT &queue->lock);

    vxAcquireMutex(queue->lock);

    queue->stopped      = vx_false_e;

    queue->beginIndex   = 0;
    queue->endIndex     = -1;

    vxCreateEvent(vx_true_e, OUT &queue->readEvent);

    vxCreateEvent(vx_true_e, OUT &queue->writeEvent);

    vxSetEvent(queue->writeEvent);

    vxReleaseMutex(queue->lock);

    gcmFOOTER_NO();
}

VX_INTERNAL_API void vxoQueue_Deinitialize(vx_queue queue)
{
    gcmHEADER_ARG("queue=%p", queue);
    vxmASSERT(queue);

    if (queue == VX_NULL) return;

    vxAcquireMutex(queue->lock);
    queue->beginIndex   = 0;
    queue->endIndex     = -1;
    vxReleaseMutex(queue->lock);

    vxmASSERT(queue->readEvent);
    vxDestroyEvent(queue->readEvent);

    vxmASSERT(queue->writeEvent);
    vxDestroyEvent(queue->writeEvent);

    vxmASSERT(queue->lock);
    vxDestroyMutex(queue->lock);
    gcmFOOTER_NO();
}
/* unused code
VX_PRIVATE_API vx_queue vxoQueue_Create()
{
    vx_queue queue = (vx_queue)vxAllocateAndZeroMemory(sizeof(vx_queue_s));

    if (queue == VX_NULL) return VX_NULL;

    vxoQueue_Initialize(queue);

    return queue;
}

VX_PRIVATE_API void vxoQueue_Destroy(vx_queue_ptr queuePtr)
{
    vx_queue queue;

    vxmASSERT(queuePtr);

    queue = *queuePtr;

    *queuePtr = VX_NULL;

    if (queue != VX_NULL)
    {
        vxoQueue_Deinitialize(queue);

        vxFree(queue);
    }
}
*/
VX_INTERNAL_API void vxoQueue_Stop(vx_queue queue)
{
    gcmHEADER_ARG("queue=%p", queue);
    vxmASSERT(queue);

    if (queue == VX_NULL) return;

    vxAcquireMutex(queue->lock);

    queue->stopped = vx_true_e;

    vxSetEvent(queue->readEvent);

    vxSetEvent(queue->writeEvent);

    vxReleaseMutex(queue->lock);

    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoQueue_WriteData(vx_queue queue, vx_value_set data)
{
    gcmHEADER_ARG("queue=%p, data=%p", queue, data);
    vxmASSERT(queue);

    while (vxWaitEvent(queue->writeEvent, gcvINFINITE))
    {
        vx_bool wrote = vx_false_e;

        vxAcquireMutex(queue->lock);

        if (queue->stopped)
        {
            vxReleaseMutex(queue->lock);
            gcmFOOTER_NO();
            return vx_false_e;
        }

        /* Write data if queue is not full */
        if (queue->beginIndex != queue->endIndex)
        {
            if (queue->endIndex == -1) queue->endIndex = queue->beginIndex;

            queue->data[queue->endIndex]    = data;
            queue->endIndex                 = (queue->endIndex + 1) % VX_QUEUE_DEPTH;

            vxSetEvent(queue->readEvent);

            wrote = vx_true_e;
        }

        /* Pending all writers if queue is full */
        if (queue->beginIndex == queue->endIndex) vxResetEvent(queue->writeEvent);

        vxReleaseMutex(queue->lock);

        if (wrote) break;
    }

    gcmFOOTER_NO();
    return vx_true_e;
}

VX_INTERNAL_API vx_bool vxoQueue_ReadData(vx_queue queue, OUT vx_value_set_ptr dataPtr)
{
    vx_bool read = vx_false_e;

    gcmHEADER_ARG("queue=%p, dataPtr=%p", queue, dataPtr);
    vxmASSERT(queue);
    vxmASSERT(dataPtr);

    while (vxWaitEvent(queue->readEvent, gcvINFINITE))
    {
        vxAcquireMutex(queue->lock);

        if (queue->stopped)
        {
            vxReleaseMutex(queue->lock);
            gcmFOOTER_NO();
            return vx_false_e;
        }

        /* Read data if queue is not empty */
        if (queue->endIndex != -1)
        {
            *dataPtr = queue->data[queue->beginIndex];

            queue->data[queue->beginIndex]  = VX_NULL;
            queue->beginIndex               = (queue->beginIndex + 1) % VX_QUEUE_DEPTH;

            if (queue->beginIndex == queue->endIndex) queue->endIndex = -1;

            vxSetEvent(queue->writeEvent);

            read = vx_true_e;
        }

        /* Pending all readers if queue is empty */
        if (queue->endIndex == -1) vxResetEvent(queue->readEvent);

        vxReleaseMutex(queue->lock);

        if (read) break;
    }
    gcmFOOTER_NO();
    return read;
}

#if VX_USE_THREADPOOL
/* ThreadPool APIs */
struct _vx_threadpool_s
{
    vx_mutex                        lock;

    vx_uint32                       workerCount;

    vx_uint32                       maxWorkItemCount;

    vx_uint32                       workItemSize;

    vx_int32                        currentWorkItemCount;

    vx_threadpool_worker            workers;

    vx_uint32                       nextWorkerIndex;

    vx_event                        completeEvent;
};

VX_PRIVATE_API vx_return_value vxThreadRoutineForThreadPoolWorker(vx_ptr arg)
{
    vx_threadpool_worker worker = (vx_threadpool_worker)arg;
    vx_bool ret = vx_false_e;

    vxmASSERT(worker);

    /* Stop Perf and restart */
    vxoPerf_End(&worker->perf);

    vxoPerf_Initialize(&worker->perf);
    vxoPerf_Begin(&worker->perf);

    while (vxoQueue_ReadData(worker->queue, OUT &worker->data))
    {
        vx_threadpool_worker_callback_f callback = worker->callback;

        worker->active = vx_true_e;
        vxoPerf_End(&worker->perf);

        ret = callback(worker);

        vxAcquireMutex(worker->threadPool->lock);

        vxmASSERT(worker->threadPool->currentWorkItemCount > 0);
        worker->threadPool->currentWorkItemCount--;
        if (worker->threadPool->currentWorkItemCount == 0) vxSetEvent(worker->threadPool->completeEvent);

        vxReleaseMutex(worker->threadPool->lock);

        vxoPerf_Begin(&worker->perf);
        worker->active = vx_false_e;
    }

    return (vx_return_value)vx_true_e;
}

VX_INTERNAL_API vx_threadpool vxoThreadPool_Create(
        vx_uint32 workerCount, vx_uint32 maxWorkItemCount, vx_size workItemSize,
        vx_threadpool_worker_callback_f workerCallback, vx_ptr workerArg)
{
    vx_threadpool threadPool;
    vx_uint32 index;

    threadPool = (vx_threadpool)vxAllocateAndZeroMemory(sizeof(vx_threadpool_s));

    if (threadPool == VX_NULL) return VX_NULL;

    if (!vxCreateMutex(&threadPool->lock)) goto ErrorExit;

    threadPool->workerCount     = workerCount;

    threadPool->workers =
        (vx_threadpool_worker)vxAllocateAndZeroMemory(threadPool->workerCount * sizeof(vx_threadpool_worker_s));

    if (threadPool->workers == VX_NULL) goto ErrorExit;

    for (index = 0; index < threadPool->workerCount; index++)
    {
        vx_threadpool_worker worker = &threadPool->workers[index];

        worker->threadPool          = threadPool;

        worker->queue               = vxoQueue_Create();
        if (worker->queue == VX_NULL) goto ErrorExit;

        worker->index               = index;

        worker->callback            = workerCallback;
        worker->arg                 = workerArg;

        vxoPerf_Initialize(&worker->perf);
        vxoPerf_Begin(&worker->perf);

        worker->thread              = vxCreateThread((vx_thread_routine_f)&vxThreadRoutineForThreadPoolWorker, worker);
    }

    threadPool->maxWorkItemCount        = maxWorkItemCount;
    threadPool->workItemSize            = (vx_uint32)workItemSize;
    threadPool->currentWorkItemCount    = 0;

    if (!vxCreateEvent(vx_true_e, OUT &threadPool->completeEvent)) goto ErrorExit;

    return threadPool;

ErrorExit:
    if (threadPool != VX_NULL)
    {
        if (threadPool->workers != VX_NULL)
        {
            for (index = 0; index < threadPool->workerCount; index++)
            {
                vx_threadpool_worker worker = &threadPool->workers[index];

                if (worker->queue != VX_NULL) vxoQueue_Destroy(&worker->queue);

                if (worker->thread != VX_NULL) vxCloseThread(worker->thread);
            }

            vxFree(threadPool->workers);
        }

        if (threadPool->completeEvent != VX_NULL) vxDestroyEvent(threadPool->completeEvent);

        if (threadPool->lock != VX_NULL) vxDestroyMutex(threadPool->lock);

        vxFree(threadPool);
    }

    return VX_NULL;
}

VX_INTERNAL_API void vxoThreadPool_Destroy(vx_threadpool_ptr threadPoolPtr)
{
    vx_threadpool threadPool;

    vxmASSERT(threadPoolPtr);

    threadPool = *threadPoolPtr;

    *threadPoolPtr = VX_NULL;

    if (threadPool != VX_NULL)
    {
        vx_uint32 index;

        for (index = 0; index < threadPool->workerCount; index++)
        {
            vx_threadpool_worker worker = &threadPool->workers[index];

            vxoQueue_Stop(worker->queue);

            vxCloseThread(worker->thread);
            worker->thread = VX_NULL;

            vxoPerf_End(&threadPool->workers[index].perf);

            vxoQueue_Destroy(&worker->queue);
        }

        vxFree(threadPool->workers);
        threadPool->workers = VX_NULL;

        vxDestroyEvent(threadPool->completeEvent);

        vxDestroyMutex(threadPool->lock);

        vxFree(threadPool);
    }
}

VX_INTERNAL_API vx_bool vxoThreadPool_QueueWorkItems(
        vx_threadpool threadPool, vx_value_set_s workItems[], vx_uint32 workItemCount)
{
    vx_uint32 i, j;
    vx_bool queued = vx_false_e;

    vxAcquireMutex(threadPool->lock);

    for (i = 0; i < workItemCount; i++)
    {
        for (j = 0; j < threadPool->workerCount; j++)
        {
            queued = vxoQueue_WriteData(threadPool->workers[threadPool->nextWorkerIndex].queue, &workItems[i]);

            threadPool->nextWorkerIndex = (threadPool->nextWorkerIndex + 1) % threadPool->workerCount;

            if (queued)
            {
                threadPool->currentWorkItemCount++;
                break;
            }
        }

        if (!queued) break;
    }

    if (threadPool->currentWorkItemCount > 0)
    {
        vxResetEvent(threadPool->completeEvent);
    }

    vxReleaseMutex(threadPool->lock);

    return queued;
}

VX_INTERNAL_API vx_bool vxoThreadpool_IsCompleted(vx_threadpool threadPool, vx_bool wait)
{
    vxmASSERT(threadPool);

    if (wait)
    {
        return vxWaitEvent(threadPool->completeEvent, gcvINFINITE);
    }
    else
    {
        return (vx_bool)(threadPool->currentWorkItemCount == 0);
    }
}
#endif /* VX_USE_THREADPOOL */


