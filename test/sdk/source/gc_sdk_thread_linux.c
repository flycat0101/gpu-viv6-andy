/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#if defined(LINUX)

#include <gc_sdk.h>
#include <pthread.h>
#include "gc_sdk_internal.h"


/******************************************************************************\
**************************** Internal Declarations ****************************
\******************************************************************************/

struct _gcsEVENT
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;

    gctBOOL manualReset;
    gctBOOL state;
};

typedef struct _gcsEVENT * gcsEVENT_PTR;


/******************************************************************************\
****************************** Support Functions ******************************
\******************************************************************************/

static gctPOINTER
_ThreadStarter(
    gctPOINTER Argument
    )
{
    /* Copy information. */
    gctTHREADROUTINE routine = ((gcsTHREADINFO_PTR) Argument)->routine;
    gctPOINTER argument = ((gcsTHREADINFO_PTR) Argument)->argument;

    /* Free the info structure. */
    free(Argument);

    /* Call the thread. */
    (*routine) (argument);
    return gcvNULL;
}


/******************************************************************************\
****************************** API Implementation *****************************
\******************************************************************************/

/*******************************************************************************
**
**    vdkSleep
**
**    Sleep for specified number of milliseconds.
**
**    INPUT:
**
**        gctUINT Milliseconds
**            Number of milliseconds to sleep for.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkSleep(
    gctUINT Milliseconds
    )
{
    struct timespec milliseconds;

    /* Init the wait structure. */
    milliseconds.tv_sec  = 0;
    milliseconds.tv_nsec = Milliseconds * 1000;

    /* Sleep for 1 millisecond. */
    nanosleep(&milliseconds, NULL);
}

/*******************************************************************************
**
**    vdkCreateThread
**
**    Create and start a new thread.
**
**    INPUT:
**
**        gctTHREADROUTINE ThreadRoutine
**            Pointer to the thread routine.
**
**        gctPOINTER Argument
**            Argument to be passed to the thread routine.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gctHANDLE
**            Handle of the new thread.
*/

gctHANDLE
vdkCreateThread(
    gctTHREADROUTINE ThreadRoutine,
    gctPOINTER Argument
    )
{
    int result;
    pthread_t thread;
    gcsTHREADINFO_PTR info;

    /* Allocate thread info structure. */
    info = (gcsTHREADINFO_PTR) malloc(sizeof(struct _gcsTHREADINFO));
    if (info == gcvNULL)
    {
        return gcvNULL;
    }

    /* Set the info. */
    info->routine = ThreadRoutine;
    info->argument = Argument;

    /* Start the thread. */
    result = pthread_create(
        &thread,
        NULL,                /* Use default attributes. */
        _ThreadStarter,
        info
        );

    /* Return result. */
    if (result == 0)
    {
        return (gctHANDLE) thread;
    }
    else
    {
        /* Cleanup. */
        free(info);

        /* Failed. */
        return gcvNULL;
    }
}

/*******************************************************************************
**
**    vdkCloseThread
**
**    Close existing thread.
**
**    INPUT:
**
**        gctHANDLE ThreadHandle
**            Handle of the existing thread to be closed.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkCloseThread(
    gctHANDLE ThreadHandle
    )
{
    /* Wait until the thread terminates. */
    pthread_join(
        (pthread_t) ThreadHandle,
        gcvNULL                /* Ignore return value. */
        );
}

/*******************************************************************************
**
**    vdkCreateEvent
**
**    Create an event.
**
**    INPUT:
**
**        gctBOOL ManualReset
**            If gcvTRUE, a manual reset is needed to reset the event to
**            non-signaled state.
**
**        gctBOOL InitialState
**            Initial state of the event.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gctHANDLE
**            Handle to the new event.
*/

gctHANDLE
vdkCreateEvent(
    gctBOOL ManualReset,
    gctBOOL InitialState
    )
{
    gcsEVENT_PTR info;

    /* Allocate event structure. */
    info = (gcsEVENT_PTR) malloc(sizeof(struct _gcsEVENT));
    if (info == gcvNULL)
    {
        return gcvNULL;
    }

    /* Initialize the event. */
    pthread_mutex_init(&info->mutex, NULL);        /* Default attributes. */
    pthread_cond_init(&info->condition, NULL);    /* Default attributes. */

    info->manualReset = ManualReset;
    info->state = InitialState;

    /* Return the handle. */
    return (gctHANDLE) info;
}

/*******************************************************************************
**
**    vdkCloseEvent
**
**    Destroy an event.
**
**    INPUT:
**
**        gctHANDLE EventHandle
**            Handle of the event.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkCloseEvent(
    gctHANDLE EventHandle
    )
{
    gcsEVENT_PTR info;

    /* Cast to the structure. */
    info = (gcsEVENT_PTR) EventHandle;

    /* Acquire the mutex. */
    if (pthread_mutex_lock(&info->mutex) != 0)
    {
        return;
    }

    /* Signal the condition before destroying it in case other threads
       are waiting for it. */
    pthread_cond_signal(&info->condition);
    pthread_cond_destroy(&info->condition);

    /* Release and destroy the mutex. */
    pthread_mutex_unlock(&info->mutex);
    pthread_mutex_destroy(&info->mutex);

    /* Free the structure. */
    free(info);
}

/*******************************************************************************
**
**    vdkSetEvent
**
**    Set event to signaled state.
**
**    INPUT:
**
**        gctHANDLE EventHandle
**            Handle of the event.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkSetEvent(
    gctHANDLE EventHandle
    )
{
    gcsEVENT_PTR info;

    /* Cast to the structure. */
    info = (gcsEVENT_PTR) EventHandle;

    /* Acquire the mutex. */
    if (pthread_mutex_lock(&info->mutex) == 0)
    {
        /* Set the event. */
        info->state = gcvTRUE;

        /* Signal the condition. */
        pthread_cond_signal(&info->condition);

        /* Release the mutex. */
        pthread_mutex_unlock(&info->mutex);
    }
}

/*******************************************************************************
**
**    vdkResetEvent
**
**    Set event to non-signaled state.
**
**    INPUT:
**
**        gctHANDLE EventHandle
**            Handle of the event.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkResetEvent(
    gctHANDLE EventHandle
    )
{
    gcsEVENT_PTR info;

    /* Cast to the structure. */
    info = (gcsEVENT_PTR) EventHandle;

    /* Acquire the mutex. */
    if (pthread_mutex_lock(&info->mutex) == 0)
    {
        /* Set the event. */
        info->state = gcvFALSE;

        /* Release the mutex. */
        pthread_mutex_unlock(&info->mutex);
    }
}

/*******************************************************************************
**
**    vdkWaitForEvent
**
**    Wait for the event to become signaled.
**
**    INPUT:
**
**        gctHANDLE EventHandle
**            Handle of the event.
**
**        gctINT Timeout
**            Timeout in milliseconds.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkWaitForEvent(
    gctHANDLE EventHandle,
    gctUINT Timeout
    )
{
    gcsEVENT_PTR info;

    /* Cast to the structure. */
    info = (gcsEVENT_PTR) EventHandle;

    /* Acquire the mutex. Functions pthread_cond_wait and pthread_cond_timedwait
       atomically unlock the mutex when called and automatically lock it back
       before returning. */
    pthread_mutex_lock(&info->mutex);

    do
    {
        /* Already signaled? */
        if (info->state)
        {
            break;
        }

        /* Infinite wait? */
        if (Timeout == gcvINFINITE)
        {
            pthread_cond_wait(&info->condition, &info->mutex);
            break;
        }

        /* Timed wait. */
        {
            struct timeval tv;
            struct timespec ts;

            gctUINT32 nowMicroseconds;
            gctUINT32 stopMicroseconds;

            /* Get current time. */
            gettimeofday(&tv, NULL);

            /* Compute the current time in microseconds. */
            nowMicroseconds
                = tv.tv_sec * 1000 * 1000
                + tv.tv_usec;

            /* Compute the stop time in microseconds. */
            stopMicroseconds
                = nowMicroseconds
                + Timeout * 1000;

            /* Split in seconds and nanoseconds. */
            ts.tv_sec  = stopMicroseconds / (1000 * 1000);
            ts.tv_nsec = stopMicroseconds % (1000 * 1000);

            /* Wait for the event to happen. */
            pthread_cond_timedwait(&info->condition, &info->mutex, &ts);
        }
    }
    while (gcvFALSE);

    /* Reset the state to nonsignaled if needed. */
    if (!info->manualReset)
    {
        info->state = gcvFALSE;
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&info->mutex);
}

/*******************************************************************************
**
**    vdkCreateMutex
**
**    Create a mutex.
**
**    INPUT:
**
**        Nothing.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gctHANDLE
**            Handle to the new mutex.
*/

gctHANDLE
vdkCreateMutex(
    void
    )
{
    pthread_mutex_t* mutex;

    /* Allocate mutex structure. */
    mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    if (mutex == gcvNULL)
    {
        return gcvNULL;
    }

    /* Initialize the mutex. */
    pthread_mutex_init(mutex, NULL);    /* Default attributes. */

    /* Return the handle. */
    return (gctHANDLE) mutex;
}

/*******************************************************************************
**
**    vdkCloseMutex
**
**    Destroy a mutex.
**
**    INPUT:
**
**        gctHANDLE MutexHandle
**            Handle of the mutex.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkCloseMutex(
    gctHANDLE MutexHandle
    )
{
    pthread_mutex_t* mutex;

    /* Cast to mutex. */
    mutex = (pthread_mutex_t*) MutexHandle;

    /* Release and destroy the mutex. */
    pthread_mutex_unlock(mutex);
    pthread_mutex_destroy(mutex);

    /* Free the mutex structure. */
    free(mutex);
}

/*******************************************************************************
**
**    vdkAcquireMutex
**
**    Acquire a mutex.
**
**    INPUT:
**
**        gctHANDLE MutexHandle
**            Handle of the event.
**
**        gctINT Timeout
**            Timeout in milliseconds.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkAcquireMutex(
    gctHANDLE MutexHandle,
    gctUINT Timeout
    )
{
    pthread_mutex_t* mutex;

    /* Cast to mutex. */
    mutex = (pthread_mutex_t*) MutexHandle;

    do
    {
        /* Infinite wait? */
        if (Timeout == gcvINFINITE)
        {
            pthread_mutex_lock(mutex);
            break;
        }

        /* Timed wait. */
        while (gcvTRUE)
        {
            /* Successfully locked? */
            if (pthread_mutex_trylock(mutex) == 0)
            {
                break;
            }

            /* Timed out? */
            if (Timeout == 0)
            {
                break;
            }

            /* Decrement the wait value. */
            Timeout--;

            /* Sleep for 1 millisecond. */
            vdkSleep(1);
        }
    }
    while (gcvFALSE);
}

/*******************************************************************************
**
**    vdkReleaseMutex
**
**    Release a mutex.
**
**    INPUT:
**
**        gctHANDLE MutexHandle
**            Handle of the mutex.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
*/

void
vdkReleaseMutex(
    gctHANDLE MutexHandle
    )
{
    pthread_mutex_unlock((pthread_mutex_t*) MutexHandle);
}

/* if defined(LINUX) */
#endif
