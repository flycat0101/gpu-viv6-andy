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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     008019
#define _GC_OBJ_ZONE        gcdZONE_CL_EVENT

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
gctINT
clfRetainEvent(
    cl_event Event
    )
{
    gctINT status = CL_SUCCESS;

    gcmHEADER_ARG("Event=0x%x", Event);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008002: (clfRetainEvent) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Event->referenceCount, gcvNULL));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfReleaseEvent(
    cl_event Event
    )
{
    gctINT                  status;
    clsEventCallback_PTR    eventCallback, nextEventCallback;
    gctINT32                oldReference;

    gcmHEADER_ARG("Event=0x%x", Event);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008003: (clfReleaseEvent) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Event->referenceCount, &oldReference));

    gcmASSERT(oldReference > 0);

    if (oldReference == 1)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Event->callbackMutex, gcvINFINITE));
        /* Free signals. */
        gcmVERIFY_OK(gcoCL_DestroySignal(Event->finishSignal));
        Event->finishSignal = gcvNULL;

        gcmVERIFY_OK(gcoCL_DestroySignal(Event->runSignal));
        Event->runSignal = gcvNULL;

        gcmVERIFY_OK(gcoCL_DestroySignal(Event->completeSignal));
        Event->completeSignal = gcvNULL;

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Event->referenceCount));
        Event->referenceCount = gcvNULL;

        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Event->callbackMutex));
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, Event->callbackMutex));
        Event->callbackMutex = gcvNULL;

        /* Free callbacks */
        eventCallback = Event->callback;
        while (eventCallback != gcvNULL)
        {
            nextEventCallback = eventCallback->next;
            gcoOS_Free(gcvNULL, eventCallback);
            eventCallback = nextEventCallback;
        }

        gcmOS_SAFE_FREE(gcvNULL, Event);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfAllocateEvent(
    clsContext_PTR          Context,
    clsEvent_PTR *          Event
    )
{
    clsEvent_PTR            event = gcvNULL;
    gctPOINTER              pointer = gcvNULL;
    gctINT                  status;

    gcmHEADER_ARG("Context=0x%x Event=0x%x", Context, Event);

    clmCHECK_ERROR(Context == gcvNULL ||
                   Context->objectType != clvOBJECT_CONTEXT,
                   CL_INVALID_CONTEXT);

    clmCHECK_ERROR(Event == gcvNULL, CL_INVALID_VALUE);

    /* Allocate event */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsEvent), &pointer), CL_OUT_OF_HOST_MEMORY);

    event                       = (clsEvent_PTR) pointer;
    event->dispatch             = Context->dispatch;
    event->objectType           = clvOBJECT_EVENT;
    event->context              = Context;
    event->queue                = gcvNULL;
    event->executionStatus      = CL_QUEUED + 1;
    event->executionStatusSet   = gcvFALSE;
    event->commandType          = CL_COMMAND_USER;
    event->callback             = gcvNULL;
    event->next                 = gcvNULL;
    event->previous             = gcvNULL;
    event->userEvent            = gcvFALSE;
    event->dominateByUser       = gcvFALSE;

    event->profileInfo.queued   = 0;
    event->profileInfo.submit   = 0;
    event->profileInfo.start    = 0;
    event->profileInfo.end      = 0;

    event->finishSignal         = gcvNULL;
    event->runSignal            = gcvNULL;
    event->completeSignal       = gcvNULL;
    event->callbackMutex        = gcvNULL;

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&event->id), CL_INVALID_VALUE);

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &event->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, event->referenceCount, gcvNULL));

    /* Create event finish signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &event->finishSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event run signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &event->runSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create event complete signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &event->completeSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create thread lock mutex for callback list setting/using. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &event->callbackMutex),
               CL_OUT_OF_HOST_MEMORY);

    *Event = event;

    gcmFOOTER_ARG("%d *Event=0x%x",
                  CL_SUCCESS, gcmOPT_POINTER(Event));

    return CL_SUCCESS;

OnError:
    if (event != gcvNULL)
    {
        if (event->finishSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(event->finishSignal));
            event->finishSignal = gcvNULL;
        }

        if (event->runSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(event->runSignal));
            event->runSignal = gcvNULL;
        }

        if (event->completeSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(event->completeSignal));
            event->completeSignal = gcvNULL;
        }

        if (event->referenceCount)
        {
            gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, event->referenceCount));
            event->referenceCount = gcvNULL;
        }

        if (event->callbackMutex)
        {
            gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, event->callbackMutex));
            event->callbackMutex = gcvNULL;
        }

        gcoOS_Free(gcvNULL, event);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctBOOL
clfIsEventInEventList(
    cl_event                Event
    )
{
    gctBOOL                 result;
    clsContext_PTR          context;

    context = Event->context;

    if (Event == context->eventList)
    {
        result = gcvTRUE;
    }
    else
    {
        result = (Event->previous != gcvNULL || Event->next != gcvNULL);
    }

    return result;
}

gctINT
clfAddEventToEventList(
    cl_event                Event
    )
{
    gctINT                  status;
    clsContext_PTR          context;

    gcmHEADER_ARG("Event=0x%x", Event);

    clmASSERT(Event, CL_INVALID_VALUE);
    clmASSERT(!clfIsEventInEventList(Event), CL_INVALID_OPERATION);

    clfRetainEvent(Event);

    context = Event->context;

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    context->eventListMutex,
                                    gcvINFINITE));

    if (context->eventList == gcvNULL)
    {
        context->eventList = Event;
    }
    else
    {
        cl_event tail = context->eventList;
        while (tail->next != gcvNULL)
        {
            tail = tail->next;
        }
        tail->next = Event;
        Event->previous = tail;
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    context->eventListMutex));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfRemoveEventFromEventList(
    cl_event                Event
    )
{
    gctINT                  status;
    clsContext_PTR          context;

    gcmHEADER_ARG("Event=0x%x", Event);

    clmASSERT(Event, CL_INVALID_VALUE);
    clmASSERT(clfIsEventInEventList(Event), CL_INVALID_OPERATION);

    context = Event->context;

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    context->eventListMutex,
                                    gcvINFINITE));

    if (Event == context->eventList)
    {
        context->eventList = Event->next;
    }

    if (Event->previous)
    {
        Event->previous->next = Event->next;
    }

    if (Event->next)
    {
        Event->next->previous = Event->previous;
    }

    Event->next = Event->previous = gcvNULL;

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    context->eventListMutex));

    clfReleaseEvent(Event);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfAddEventCallback(
    clsEventCallback_PTR    EventCallback
    )
{
    gctINT                  status;
    clsContext_PTR          context = gcvNULL;
    clsEventCallback_PTR    eventCallback;

    gcmHEADER_ARG("EventCallback=0x%x", EventCallback);

    clmASSERT(EventCallback, CL_INVALID_VALUE);

    clfRetainEvent(EventCallback->event);

    context = EventCallback->event->context;

    eventCallback       = EventCallback;
    eventCallback->next = gcvNULL;

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    context->eventCallbackListMutex,
                                    gcvINFINITE));

    if (context->eventCallbackWorkerThread == gcvNULL)
    {
        /* Start the event callback worker thread. */
        clmONERROR(gcoOS_CreateThread(gcvNULL,
                                      clfEventCallbackWorker,
                                      context,
                                      &context->eventCallbackWorkerThread),
                                      CL_OUT_OF_HOST_MEMORY);
    }

    if (context->eventCallbackList == gcvNULL)
    {
        context->eventCallbackList = eventCallback;
    }
    else
    {
        clsEventCallback_PTR    lastEventCallback = context->eventCallbackList;

        while (lastEventCallback->next != gcvNULL)
        {
            lastEventCallback = lastEventCallback->next;
        }

        lastEventCallback->next = eventCallback;
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    context->eventCallbackListMutex));

    gcmVERIFY_OK(gcoCL_SetSignal(context->eventCallbackWorkerStartSignal));

    status = CL_SUCCESS;
    gcmFOOTER_ARG("%d", status);

    gcmFOOTER_NO();

    return status;

OnError:
    if (context)
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, context->eventCallbackListMutex));

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfGetEventCallback(
    clsContext_PTR          Context,
    clsEventCallback_PTR *  EventCallback
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Context=0x%x EventCallback=0x%x",
                  Context, EventCallback);

    clmCHECK_ERROR(Context == gcvNULL ||
                   Context->objectType != clvOBJECT_CONTEXT,
                   CL_INVALID_CONTEXT);

    clmCHECK_ERROR(EventCallback == gcvNULL, CL_INVALID_VALUE);

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    Context->eventCallbackListMutex,
                                    gcvINFINITE));

    *EventCallback = Context->eventCallbackList;

    if (Context->eventCallbackList != gcvNULL)
    {
        Context->eventCallbackList = Context->eventCallbackList->next;
        (*EventCallback)->next = gcvNULL;
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    Context->eventCallbackListMutex));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfSetEventExecutionStatus(
    cl_event        Event,
    gctINT          EventStatus
    )
{
    gcmHEADER_ARG("Event=0x%x EventStatus=%d", Event, EventStatus);

    if (Event->queue != gcvNULL &&
        (Event->queue->properties & CL_QUEUE_PROFILING_ENABLE) != 0)
    {
        /* Profiling is enabled, record timestamp */
        gctUINT64 time;

        gcoOS_GetTime(&time);  /* in microseconds */
        time *= 1000;          /* in nanoseconds  */

        switch (EventStatus)
        {
        case CL_SUBMITTED:
            Event->profileInfo.submit = time;
            break;
        case CL_QUEUED:
            Event->profileInfo.queued = time;
            break;
        case CL_RUNNING:
            Event->profileInfo.start = time;
            break;
        case CL_COMPLETE:
            Event->profileInfo.end = time;
            break;
        default:
            break;
        }
    }

    if (gcmNO_ERROR(Event->executionStatus))
    {
        Event->executionStatus = EventStatus;
    }

    if (Event->userEvent == gcvTRUE)
    {
        Event->executionStatusSet = gcvTRUE;
    }

    gcmFOOTER_NO();
    return CL_SUCCESS;
}

gctINT
clfGetEventExecutionStatus(
    cl_event        Event
    )
{
    gctINT executionStatus;

    gcmHEADER_ARG("Event=0x%x", Event);

    if (gcmIS_ERROR(Event->executionStatus) || (Event->executionStatus == CL_COMPLETE))
    {
        executionStatus = Event->executionStatus;
    }
    else if (gcmIS_SUCCESS(gcoCL_WaitSignal(Event->finishSignal, 0)))
    {
        executionStatus = CL_COMPLETE;
    }
    else if (gcmIS_SUCCESS(gcoCL_WaitSignal(Event->runSignal, 0)))
    {
        executionStatus = CL_RUNNING;
    }
    else
    {
        executionStatus = Event->executionStatus;
    }

    gcmFOOTER_ARG("%d", executionStatus);

    return executionStatus;
}

gctINT
clfFinishEvent(
    cl_event        Event,
    gctINT          EventStatus
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Event=0x%x EventStatus=%d", Event, EventStatus);

    clmASSERT(Event, CL_INVALID_VALUE);

    clfSetEventExecutionStatus(Event, EventStatus);

    /* Wake up all waiters */
    gcmONERROR(gcoCL_SetSignal(Event->completeSignal));

    /* Wake up all command queue workers */
    clfWakeUpAllCommandQueueWorkers(Event->context);

    /* Invoke the event callbacks. */
    status = clfScheduleEventCallback(Event, EventStatus);

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}

gctINT
clfScheduleEventCallback(
    cl_event        Event,
    gctINT          ExecutionStatus
    )
{
    gctINT                  status;
    clsEventCallback_PTR    eventCallback, prev, next;
    gctINT                  checkPoint = CL_SUBMITTED;

    gcmHEADER_ARG("Event=0x%x Status=%d", Event, ExecutionStatus);

    clmASSERT(Event, CL_INVALID_VALUE);

    if(!gcoOS_StrCmp(clgDefaultDevice->deviceVersion, cldVERSION11)) checkPoint = CL_COMPLETE;

    if (ExecutionStatus <= checkPoint)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Event->callbackMutex, gcvINFINITE));

        prev = eventCallback = Event->callback;

        /* Invoke event callbacks in separate threads */
        while (eventCallback != gcvNULL)
        {
            next = eventCallback->next;

            if (ExecutionStatus <= eventCallback->type)
            {
                if (eventCallback == Event->callback)
                {
                    prev = Event->callback = next;
                }
                else
                {
                    prev->next = next;
                }

                clfAddEventCallback(eventCallback);
            }
            else
            {
                prev = eventCallback;
            }

            eventCallback = next;
        }

        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Event->callbackMutex));
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}

gctINT
clfCheckPendingEventsList(
    clsCommand_PTR      Command,
    gctUINT             numEventsInWaitList,
    const clsEvent_PTR  *eventWaitList
    )
{
    gctBOOL             allEventsCompleted = gcvTRUE;
    gctUINT             i;

    gcmHEADER_ARG("Command=0x%x", Command);

    if (!Command)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_FALSE;
    }

    for (i = 0; i < numEventsInWaitList; i++)
    {
        gctINT executionStatus;

        executionStatus = clfGetEventExecutionStatus(eventWaitList[i]);

        if (executionStatus != CL_COMPLETE)
        {
            if (gcmNO_ERROR(executionStatus))
            {
                allEventsCompleted = gcvFALSE;
                break;
            }
            else
            {
                if (Command->event)
                {
                    clfFinishEvent(Command->event, executionStatus);
                }

                gcmFOOTER_ARG("status=%d", gcvSTATUS_TERMINATE);
                return gcvSTATUS_TERMINATE;
            }
        }
    }

    gcmFOOTER_ARG("allEventsCompleted=%d", allEventsCompleted);
    return (allEventsCompleted == gcvFALSE) ? gcvSTATUS_TRUE : gcvSTATUS_FALSE;
}

gctINT
clfCheckPendingEvents(
    clsCommand_PTR  Command
    )
{
    gctINT pendingEventStatus;

    gcmHEADER_ARG("Command=0x%x", Command);

    if (!Command)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_FALSE;
    }

    pendingEventStatus = clfCheckPendingEventsList(Command, Command->numEventsInWaitList, Command->eventWaitList);

    gcmFOOTER_ARG("pendingEventStatus=%d", pendingEventStatus);

    return pendingEventStatus;
}

gctINT
clfWaitForEvent(
    cl_event                Event
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Event=0x%x", Event);

    clmASSERT(Event, CL_INVALID_VALUE);

    clfRetainEvent(Event);

    /* wait until the event complete */
    gcmVERIFY_OK(gcoCL_WaitSignal(Event->completeSignal, gcvINFINITE));

    clfReleaseEvent(Event);

    status = Event->executionStatus;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfProcessEventList(
    clsContext_PTR          Context
    )
{
    gctINT                  status;
    cl_event                event, nextEvent;

    gcmHEADER_ARG("Context=0x%x", Context);

    clmASSERT(Context, CL_INVALID_VALUE);

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    Context->eventListMutex,
                                    gcvINFINITE));

    event = Context->eventList;

    /* do one pass over pending events in the context */
    while (event)
    {
        nextEvent = event->next;

        if ((event->executionStatus > CL_RUNNING) && gcmIS_SUCCESS(gcoCL_WaitSignal(event->runSignal, 0)))
        {
            clfSetEventExecutionStatus(event, CL_RUNNING);

            clfScheduleEventCallback(event, CL_RUNNING);
        }

        if (gcmIS_SUCCESS(gcoCL_WaitSignal(event->finishSignal, 0)))
        {
            gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                            Context->eventListMutex));

            clfFinishEvent(event, CL_COMPLETE);

            /* Remove the event from the list */
            clfRemoveEventFromEventList(event);

            gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                        Context->eventListMutex,
                                        gcvINFINITE));

            /* Restart the loop */
            nextEvent = Context->eventList;
        }

        event = nextEvent;
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    Context->eventListMutex));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfSubmitEventForFinish(
    clsCommand_PTR          Command
    )
{
    gctINT                  status;
    cl_event                event;
    clsContext_PTR          context;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command, CL_INVALID_VALUE);

    event = Command->event;
    context = event->context;

    /* Add the event to the event list */
    if (!clfIsEventInEventList(event)) clfAddEventToEventList(event);

    /* Submit the event's finish signal. */
    gcmONERROR(gcoCL_SubmitSignal(event->finishSignal,
                                  context->process,
                                  Command->submitEngine));

    /* Submit the signal to wake up the event list worker to handle the event. */
    gcmONERROR(gcoCL_SubmitSignal(context->eventListWorkerStartSignal,
                                  context->process,
                                  Command->submitEngine));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfSetEventForFinish(
    clsCommand_PTR          Command
    )
{
    gctINT                  status;
    cl_event                event;
    clsContext_PTR          context;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command, CL_INVALID_VALUE);

    event = Command->event;
    context = event->context;

    /* Add the event to the event list */
    if (!clfIsEventInEventList(event)) clfAddEventToEventList(event);

    /* Set the event's finish signal. */
    gcmONERROR(gcoCL_SetSignal(event->finishSignal));

    /* Set the signal to wake up the event list worker to handle the event. */
    gcmONERROR(gcoCL_SetSignal(context->eventListWorkerStartSignal));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfSubmitEventForRunning(
    clsCommand_PTR          Command
    )
{
    gctINT                  status;
    cl_event                event;
    clsContext_PTR          context;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command, CL_INVALID_VALUE);

    event = Command->event;
    context = event->context;

    /* Add the event to the event list */
    clfAddEventToEventList(event);

    /* Add event's running signal. */
    gcmONERROR(gcoCL_SubmitSignal(event->runSignal,
                                  context->process,
                                  Command->submitEngine));

    /* Add signal to wake up worker to handle the event. */
    gcmONERROR(gcoCL_SubmitSignal(context->eventListWorkerStartSignal,
                                  context->process,
                                  Command->submitEngine));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctTHREAD_RETURN CL_API_CALL
clfEventListWorker(
    gctPOINTER              Data
    )
{
    clsContext_PTR          context = (clsContext_PTR) Data;

    while (gcvTRUE)
    {
        /* Wait for the start signal. */
        gcmVERIFY_OK(gcoCL_WaitSignal(context->eventListWorkerStartSignal, gcvINFINITE));

        /* Check the thread's stop signal. */
        if (gcmIS_SUCCESS(gcoCL_WaitSignal(context->eventListWorkerStopSignal, 0)))
        {
            /* Stop had been signaled, exit. */
            break;
        }

        /* Process pending event signals. */
        clfProcessEventList(context);
    }

    return (gctTHREAD_RETURN) 0;
}

gctTHREAD_RETURN CL_API_CALL
clfEventCallbackWorker(
    gctPOINTER              Data
    )
{
    clsContext_PTR          context = (clsContext_PTR) Data;
    clsEventCallback_PTR    eventCallback = gcvNULL;

    /* Use the same hardware for event callback worker. */

    while (gcvTRUE)
    {
        /* Wait for the start signal. */
        gcmVERIFY_OK(gcoCL_WaitSignal(context->eventCallbackWorkerStartSignal, gcvINFINITE));

        /* Check the thread's stop signal. */
        if (gcmIS_SUCCESS(gcoCL_WaitSignal(context->eventCallbackWorkerStopSignal, 0)))
        {
            /* Stop had been signaled, exit. */
            break;
        }

        while (gcvTRUE)
        {
            /* Check event callback queue. */
            clfGetEventCallback(context, &eventCallback);

            if (eventCallback != gcvNULL)
            {
                eventCallback->pfnNotify(
                    eventCallback->event,
                    eventCallback->type,
                    eventCallback->userData);

                clfReleaseEvent(eventCallback->event);

                if(eventCallback) gcoOS_Free(gcvNULL, eventCallback);
            }
            else
            {
                break;
            }
        }
    }
    return (gctTHREAD_RETURN) 0;
}

/*****************************************************************************\
|*                        OpenCL Event Object API                            *|
\*****************************************************************************/
CL_API_ENTRY cl_event CL_API_CALL
clCreateUserEvent(
    cl_context    Context,
    cl_int *      ErrcodeRet
    )
{
    clsEvent_PTR            event;
    gctINT                  status;

    gcmHEADER_ARG("Context=0x%x", Context);
    gcmDUMP_API("${OCL clCreateUserEvent 0x%x}", Context);
    VCL_TRACE_API(CreateUserEvent_Pre)(Context, ErrcodeRet);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008000: (clCreateUserEvent) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    status = clfAllocateEvent(Context, &event);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008001: (clCreateUserEvent) cannot create user event.  Maybe run out of memory.\n");
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    event->userEvent = gcvTRUE;

    event->executionStatus = CL_SUBMITTED;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateUserEvent_Post)(Context, ErrcodeRet, event);
    gcmFOOTER_ARG("%d event=%lu",
                  CL_SUCCESS, event);
    return event;

OnError:
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(
    cl_event Event
    )
{
    gctINT          status;

    gcmHEADER_ARG("Event=0x%x", Event);
    gcmDUMP_API("${OCL clRetainEvent 0x%x}", Event);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008002: (clRetainEvent) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    clfONERROR(clfRetainEvent(Event));

    VCL_TRACE_API(RetainEvent)(Event);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(
    cl_event Event
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Event=0x%x", Event);
    gcmDUMP_API("${OCL clReleaseEvent 0x%x}", Event);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008003: (clReleaseEvent) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    clfONERROR(clfReleaseEvent(Event));

    VCL_TRACE_API(ReleaseEvent)(Event);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetUserEventStatus(
    cl_event   Event,
    cl_int     ExecutionStatus
    )
{
    gctINT           status;

    gcmHEADER_ARG("Event=0x%x ExecutionStatus=%d", Event, ExecutionStatus);
    gcmDUMP_API("${OCL clSetUserEventStatus 0x%x, 0x%x}", Event, ExecutionStatus);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008004: (clSetUserEventStatus) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    if (Event->executionStatusSet == gcvTRUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008005: (clSetUserEventStatus) Event's execution status has been set.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (ExecutionStatus != CL_COMPLETE && ExecutionStatus > 0 )
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008006: (clSetUserEventStatus) ExecutionStatus is invalid.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clfFinishEvent(Event, ExecutionStatus);

    VCL_TRACE_API(SetUserEventStatus)(Event, ExecutionStatus);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(
    cl_uint             NumEvents,
    const cl_event *    EventList
    )
{
    gctINT          status = CL_SUCCESS;
    gctUINT         i;

    gcmHEADER_ARG("EventList=0x%x NumEvents=%u", EventList, NumEvents);
    gcmDUMP_API("${OCL clWaitForEvents 0x%x}", NumEvents);
    VCL_TRACE_API(WaitForEvents)(NumEvents, EventList);

    if (EventList == gcvNULL || NumEvents == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008007: (clWaitForEvents) EventList is NULL, or NumEvents is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    for (i = 0; i < NumEvents; i++)
    {
        if (EventList[i] == gcvNULL || EventList[i]->objectType != clvOBJECT_EVENT)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-008008: (clWaitForEvents) EventList[%d] is invalid.\n",
                i);
            clmRETURN_ERROR(CL_INVALID_EVENT);
        }

        /* check if all events have the same context */
        if (EventList[0]->context != EventList[i]->context)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-008009: (clWaitForEvents) EventList[%d] has different context than EventList[0].\n",
                i);
            clmRETURN_ERROR(CL_INVALID_CONTEXT);
        }

        /* Wait for the event complete */
        {
            gctINT waitResult = clfWaitForEvent(EventList[i]);
            if (gcmIS_ERROR(waitResult))
            {
                status = CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
            }
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(
    cl_event         Event,
    cl_event_info    ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;
    gctINT32         referenceCount;
    gctINT           executionStatus;

    gcmHEADER_ARG("Event=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Event, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetEventInfo 0x%x, 0x%x}", Event, ParamName);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008011: (clGetEventInfo) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    switch (ParamName)
    {
    case CL_EVENT_COMMAND_QUEUE:
        retParamSize = gcmSIZEOF(Event->queue);
        retParamPtr = &Event->queue;
        break;

    case CL_EVENT_CONTEXT:
        retParamSize = gcmSIZEOF(Event->context);
        retParamPtr = &Event->context;
        break;

    case CL_EVENT_COMMAND_TYPE:
        retParamSize = gcmSIZEOF(Event->commandType);
        retParamPtr = &Event->commandType;
        break;

    case CL_EVENT_COMMAND_EXECUTION_STATUS:
        executionStatus = clfGetEventExecutionStatus(Event);

        retParamSize = gcmSIZEOF(executionStatus);
        retParamPtr = &executionStatus;
        break;

    case CL_EVENT_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, Event->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008012: (clGetEventInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-008013: (clGetEventInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetEventInfo)(Event, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetEventCallback(
    cl_event    Event,
    cl_int      CommandExecCallbackType,
    void        (CL_CALLBACK * PfnNotify)(cl_event, cl_int, void *),
    void *      UserData
    )
{
    clsEventCallback_PTR    eventCallback;
    gctPOINTER              pointer;
    gctINT                  status;
    gctINT                  checkPoint = CL_SUBMITTED | CL_RUNNING | CL_COMPLETE;

    gcmHEADER_ARG("Event=0x%x CallBackType=%d PfnNotify=0x%x UserData=0x%x",
                  Event, CommandExecCallbackType, PfnNotify, UserData);
    gcmDUMP_API("${OCL clSetEventCallback 0x%x, 0x%x}", Event, CommandExecCallbackType);

    if (Event == gcvNULL || Event->objectType != clvOBJECT_EVENT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008014: (clSetEventCallback) invalid Event.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT);
    }

    if(!gcoOS_StrCmp(clgDefaultDevice->deviceVersion, cldVERSION11))
        checkPoint = CL_COMPLETE;

    if ((CommandExecCallbackType != CL_COMPLETE) && ((CommandExecCallbackType & checkPoint) == 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008018: (clSetEventCallback) invalid CommandExecCallbackType.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (PfnNotify == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008015: (clSetEventCallback) PfnNotify is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clfRetainEvent(Event);
    status = gcoOS_Allocate(gcvNULL, sizeof(clsEventCallback), &pointer);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-008017: (clSetEventCallback) Run out of memory.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Event->callbackMutex, gcvINFINITE));

    eventCallback               = (clsEventCallback_PTR) pointer;
    eventCallback->pfnNotify    = PfnNotify;
    eventCallback->userData     = UserData;
    eventCallback->event        = Event;
    eventCallback->type         = CommandExecCallbackType;
    eventCallback->next         = gcvNULL;

    if (clfGetEventExecutionStatus(Event) <= eventCallback->type)
    {
        /* Invoke callback if the event execution status is equal to or past
         * the eventCallback->type.
         */
        clfAddEventCallback(eventCallback);
    }
    else
    {
        /* Add to the list and it will be invoked later. */
        eventCallback->next     = Event->callback;
        Event->callback         = eventCallback;
    }

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Event->callbackMutex));
    clfReleaseEvent(Event);

    VCL_TRACE_API(SetEventCallback)(Event, CommandExecCallbackType, PfnNotify, UserData);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
