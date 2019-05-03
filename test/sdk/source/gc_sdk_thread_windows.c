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


#include <gc_sdk.h>
#include "gc_sdk_internal.h"


/******************************************************************************\
****************************** Support Functions ******************************
\******************************************************************************/

static DWORD WINAPI
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
    return 0;
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
    Sleep(Milliseconds);
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
    gctHANDLE result;
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
    result = (gctHANDLE) CreateThread(
        gcvNULL,            /* Default security. */
        0,                    /* Default stack. */
        _ThreadStarter,
        info,
        0,                    /* Default creation flags. */
        gcvNULL                /* Don't care about the ID. */
        );

    /* Return result. */
    if (result != gcvNULL)
    {
        return result;
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
    /* Wait for the thread to terminate. */
    WaitForSingleObject((HANDLE) ThreadHandle, INFINITE);

    /* Close the handle. */
    CloseHandle((HANDLE) ThreadHandle);
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
    return (gctHANDLE) CreateEvent(NULL, ManualReset, InitialState, NULL);
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
    /* Set the event before destroying it in case other threads
       are waiting for it. */
    SetEvent((HANDLE) EventHandle);
    CloseHandle((HANDLE) EventHandle);
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
    SetEvent((HANDLE) EventHandle);
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
    ResetEvent((HANDLE) EventHandle);
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
    if (Timeout == gcvINFINITE)
    {
        Timeout = INFINITE;
    }

    WaitForSingleObject((HANDLE) EventHandle, Timeout);
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
    return (gctHANDLE) CreateMutex(NULL, FALSE, NULL);
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
    CloseHandle((HANDLE) MutexHandle);
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
    if (Timeout == gcvINFINITE)
    {
        Timeout = INFINITE;
    }

    WaitForSingleObject((HANDLE) MutexHandle, Timeout);
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
    ReleaseMutex((HANDLE) MutexHandle);
}
