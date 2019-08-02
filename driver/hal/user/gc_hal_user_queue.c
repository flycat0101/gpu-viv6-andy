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


/**
**  @file
**  Architecture independent event queue management.
**
*/

#include "gc_hal_user_precomp.h"
#include "gc_hal_user_buffer.h"

#ifdef __QNXNTO__

#include <pthread.h>
#include <sys/queue.h>
#include "../os/qnx/inc/gc_hal_common_qnx.h"
gceSTATUS
gcoOS_SignalPending(
    IN gcoOS Os,
    IN gcsSIGNAL_PTR Signal
    );

#endif


#if (gcdENABLE_3D || gcdENABLE_2D)
/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_BUFFER

#define gcoQUEUE_LIST_SIZE 16

gceSTATUS
gcoQUEUE_Construct(
    IN gcoOS Os,
    IN gceENGINE Engine,
    OUT gcoQUEUE * Queue
    )
{
    gcoQUEUE queue = gcvNULL;
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Queue != gcvNULL);

    /* Create the queue. */
    gcmONERROR(
        gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoQUEUE),
                       &pointer));
    queue = pointer;

    /* Initialize the object. */
    queue->object.type = gcvOBJ_QUEUE;

    /* Nothing in the queue yet. */
    queue->head = queue->tail = gcvNULL;

    queue->recordCount = 0;
    queue->maxUnlockBytes= 0;

    queue->chunks = gcvNULL;

    queue->freeList = gcvNULL;

    queue->engine = Engine;

    /* Return gcoQUEUE pointer. */
    *Queue = queue;

    /* Success. */
    gcmFOOTER_ARG("*Queue=0x%x", *Queue);
    return gcvSTATUS_OK;

OnError:
    if (queue != gcvNULL)
    {
        /* Roll back. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, queue));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoQUEUE_Destroy(
    IN gcoBUFFER Buffer,
    IN gcoQUEUE Queue
    )
{
    gceSTATUS status;
    gcsChunkHead_PTR p;

    gcmHEADER_ARG("Queue=0x%x", Queue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

    while (Queue->chunks != gcvNULL)
    {
        /* Unlink the first chunk. */
        p = Queue->chunks;
        Queue->chunks = p->next;

        /* Free the memory. */
        gcmONERROR(gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, p));
    }

    /* Free the queue. */
    gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, Queue));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoQUEUE_AppendEvent(
    IN gcoQUEUE Queue,
    IN gcsHAL_INTERFACE * Interface
    )
{
    gceSTATUS status;
    gcsQUEUE_PTR record = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gcsChunkHead_PTR p;
    gctSIZE_T i, count = gcoQUEUE_LIST_SIZE;

    gcmHEADER_ARG("Queue=0x%x Interface=0x%x", Queue, Interface);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);
    gcmVERIFY_ARGUMENT(Interface != gcvNULL);

    /* Check if we have records on the free list. */
    if (Queue->freeList == gcvNULL)
    {
        gcmONERROR(gcoOS_AllocateSharedMemory(
                                    gcvNULL,
                                    gcmSIZEOF(*p) + gcmSIZEOF(*record) * count,
                                    &pointer));

        p = pointer;

        /* Put it on the chunk list. */
        p->next       = Queue->chunks;
        Queue->chunks = p;

        /* Put the records on free list. */
        for (i = 0, record = (gcsQUEUE_PTR)(p + 1); i < count; i++, record++)
        {
            record->next    = gcmPTR_TO_UINT64(Queue->freeList);
            Queue->freeList = record;
        }
    }

    /* Allocate from the free list. */
    record          = Queue->freeList;
    Queue->freeList = gcmUINT64_TO_PTR(record->next);

    /* Initialize record. */
    record->next  = 0;
    gcoOS_MemCopy(&record->iface, Interface, gcmSIZEOF(record->iface));

    if (Queue->head == gcvNULL)
    {
        /* Initialize queue. */
        Queue->head = record;
    }
    else
    {
        /* Append record to end of queue. */
        Queue->tail->next = gcmPTR_TO_UINT64(record);
    }

    /* Mark end of queue. */
    Queue->tail = record;

    /* update count */
    Queue->recordCount++;

    if (Interface->command == gcvHAL_UNLOCK_VIDEO_MEMORY &&
        Interface->u.UnlockVideoMemory.asynchroneous &&
        Interface->u.UnlockVideoMemory.pool > gcvPOOL_UNKNOWN &&
        Interface->u.UnlockVideoMemory.pool <= gcvPOOL_SYSTEM
       )
    {
        if (Queue->maxUnlockBytes < Interface->u.UnlockVideoMemory.bytes)
        {
            Queue->maxUnlockBytes = Interface->u.UnlockVideoMemory.bytes;
        }
    }
#if defined(__QNXNTO__)
    else if (Interface->command == gcvHAL_SIGNAL)
    {
        gcsSIGNAL_PTR signal = gcmUINT64_TO_TYPE(Interface->u.Signal.signal, gcsSIGNAL_PTR);
        gcoOS_SignalPending(gcvNULL, signal);
    }
#endif
    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoQUEUE_Free(
    IN gcoQUEUE Queue
    )
{
    gctSIZE_T i;
    gcsChunkHead_PTR p;
    gcsQUEUE_PTR record = gcvNULL;

    gcmHEADER_ARG("Queue=0x%x", Queue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

    p = Queue->chunks;
    Queue->freeList = gcvNULL;

    while (p != gcvNULL)
    {
        /* Put the records on free list. */
        for (i = 0, record = (gcsQUEUE_PTR)(p + 1); i < gcoQUEUE_LIST_SIZE; i++, record++)
        {
            record->next    = gcmPTR_TO_UINT64(Queue->freeList);
            Queue->freeList = record;
        }

        p = p->next;
    }

    /* clean up the queue now. */
    Queue->head = Queue->tail = gcvNULL;
    Queue->recordCount = 0;
    Queue->maxUnlockBytes = 0;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoQUEUE_Commit(
    IN gcoBUFFER Buffer,
    IN gcoQUEUE Queue,
    IN gctBOOL Stall
    )
{
    gcmHEADER_ARG("Queue=0x%x", Queue);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Queue, gcvOBJ_QUEUE);

    if (Queue->head != gcvNULL && Buffer)
    {
        gcoWorkerInfo* worker;

        gcoSuspendWorker(Buffer);

        /* Find an available worker. */
        worker = gcoGetWorker(Queue, Buffer, gcvTRUE);

        gcoResumeWorker(Buffer);

        if (worker == gcvNULL)
        {
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        /* Submit the worker. */
        gcoSubmitWorker(Buffer, worker, Stall);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}
#endif  /*  (gcdENABLE_3D || gcdENABLE_2D) */
