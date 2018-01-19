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
#include <stdio.h>

#define __NEXT_MSG_ID__     003013

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
gctBOOL
clfChooseThreadMode(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    );

static gctINT
clfProcessCommitInThread(
    clsCommandQueue_PTR     CommandQueue,
    clsCommitRequest_PTR    CommitRequest
    );

static gctINT
clfProcessCommandInThread(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    );

gctINT
clfAllocateCommand(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR *        Command
    )
{
    clsCommand_PTR          command;
    gctPOINTER              pointer = gcvNULL;
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x",
                  CommandQueue, Command);

    clmCHECK_ERROR(CommandQueue == gcvNULL ||
                   CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE,
                   CL_INVALID_COMMAND_QUEUE);

    clmCHECK_ERROR(Command == gcvNULL, CL_INVALID_VALUE);

    /* Allocate command. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsCommand), &pointer), CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, sizeof(clsCommand));

    command                         = (clsCommand_PTR) pointer;
    command->objectType             = clvOBJECT_COMMAND;
    command->commandQueue           = CommandQueue;
    command->type                   = clvCOMMAND_UNKNOWN;
    command->handler                = gcvNULL;
    command->releaseSignal          = gcvNULL;
    command->numEventsInWaitList    = 0;
    command->eventWaitList          = gcvNULL;
    command->event                  = gcvNULL;
    command->next                   = gcvNULL;
    command->previous               = gcvNULL;
    command->submitEngine           = gcvENGINE_RENDER;

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&command->id), CL_INVALID_VALUE);

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &command->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, command->referenceCount, gcvNULL));

    *Command = command;

    gcmFOOTER_ARG("%d *Command=0x%x",
                  CL_SUCCESS, gcmOPT_VALUE(Command));
    return CL_SUCCESS;

OnError:
    /* Cleanup. */
    if (pointer != gcvNULL)
        gcoOS_Free(gcvNULL, pointer);

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfRetainCommand(
    clsCommand_PTR Command
    )
{
    gctINT status;

    gcmHEADER_ARG("Command=0x%x", Command);

    if ((Command == gcvNULL) ||
        (Command->objectType != clvOBJECT_COMMAND))
    {
        status = CL_INVALID_VALUE;
        goto OnError;
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Command->referenceCount, gcvNULL));

    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfReleaseCommand(
    clsCommand_PTR Command
    )
{
    gctINT      status;
    gctINT32    oldReference;

    gcmHEADER_ARG("Command=0x%x", Command);

    if ((Command == gcvNULL) ||
        (Command->objectType != clvOBJECT_COMMAND))
    {
        status = CL_INVALID_VALUE;
        goto OnError;
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Command->referenceCount, &oldReference));

    /* Verify the reference. */
    if (oldReference <= 0)
    {
        status = CL_INVALID_VALUE;
        goto OnError;
    }

    /* Ready to be destroyed? */
    if (oldReference == 1)
    {
        if (Command->event)
        {
            clfReleaseEvent(Command->event);
            Command->event = gcvNULL;
        }

        if (Command->eventWaitList)
        {
            gcmASSERT(Command->numEventsInWaitList);
            gcoOS_Free(gcvNULL, (gctPOINTER)Command->eventWaitList);
        }

        /* Free kernel arguments for NDRangeKernel and Task commands. */
        if (Command->type == clvCOMMAND_NDRANGE_KERNEL)
        {
            clsCommandNDRangeKernel_PTR NDRangeKernel = &Command->u.NDRangeKernel;
            clfFreeKernelArgs(NDRangeKernel->numArgs, NDRangeKernel->args, gcvFALSE);
            clfReleaseKernel(NDRangeKernel->kernel);
        }
        else if (Command->type == clvCOMMAND_TASK)
        {
            clsCommandTask_PTR task = &Command->u.task;
            clfFreeKernelArgs(task->numArgs, task->args, gcvFALSE);
            clfReleaseKernel(task->kernel);
        }

        if (Command->releaseSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(Command->releaseSignal));
            Command->releaseSignal = gcvNULL;
        }

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Command->referenceCount));
        Command->referenceCount = gcvNULL;

        /* Free the command object. */
        gcoOS_Free(gcvNULL, Command);
    }

    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

static void
clfLockSyncPointList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gcmASSERT(CommandQueue != gcvNULL);

    if (CommandQueue->syncPointListMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                        CommandQueue->syncPointListMutex,
                                        gcvINFINITE));
    }
}

static void
clfUnlockSyncPointList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gcmASSERT(CommandQueue != gcvNULL);
    if (CommandQueue->syncPointListMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                        CommandQueue->syncPointListMutex));
    }
}

static void
clfLockCommandList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gcmASSERT(CommandQueue != gcvNULL);
    gcmASSERT(CommandQueue->commandListMutex != gcvNULL);

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    CommandQueue->commandListMutex,
                                    gcvINFINITE));
}

static void
clfUnlockCommandList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gcmASSERT(CommandQueue != gcvNULL);
    gcmASSERT(CommandQueue->commandListMutex != gcvNULL);

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    CommandQueue->commandListMutex));
}

/* This function should be inside mutex session. */
static gctBOOL
clfCheckPendingCommands(
    clsCommand_PTR  Command
    )
{
    clsCommandQueue_PTR     commandQueue;
    clsCommand_PTR          command;
    gctBOOL                 cmdsPending = gcvFALSE;
    clsSyncPoint_PTR        syncPoint;
    /*gctINT                  status;*/

    gcmHEADER_ARG("Command=0x%x", Command);

    if (!(Command && Command->objectType == clvOBJECT_COMMAND))
    {
        gcmFOOTER_NO();
        return cmdsPending;
    }

    commandQueue    = Command->commandQueue;

    /* Acquire synchronization mutex. */
    clfLockSyncPointList(commandQueue);

    syncPoint = commandQueue->syncPointList;
    while (syncPoint)
    {
        if (Command->enqueueNo > syncPoint->enqueueNo)
        {
            /* This command must wait, it has been enqueued after sync point */
            cmdsPending = gcvTRUE;
            break;
        }
        else if (Command->enqueueNo == syncPoint->enqueueNo)
        {
            /* This is the sync point command */

            /* Check if there are pending commands wrt to this sync point */
            command = commandQueue->commandHead;

            while (command)
            {
                if (command->enqueueNo < syncPoint->enqueueNo)
                {
                    cmdsPending = gcvTRUE;
                    break;
                }

                command = command->next;
            }

            if (!cmdsPending)
            {
                /* Check if there are pending deferred release commands to this sync point */
                command = commandQueue->deferredReleaseCommandHead;

                while (command)
                {
                    if (command->enqueueNo < syncPoint->enqueueNo)
                    {
                        cmdsPending = gcvTRUE;
                        break;
                    }

                    command = command->next;
                }
            }
        }

        if (cmdsPending) break;

        syncPoint = syncPoint->next;
    }

    /* Release synchronization mutex. */
    clfUnlockSyncPointList(commandQueue);

/*OnError:*/
    gcmFOOTER_ARG("cmdsPending=%d", cmdsPending);
    return cmdsPending;
}

static void
clfWakeUpCommandQueueWorker(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    gcmASSERT(CommandQueue != gcvNULL);

    gcmVERIFY_OK(gcoCL_SetSignal(CommandQueue->workerStartSignal));

    gcmFOOTER_NO();
}

static gctINT
clfAddSyncPoint(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    );

/* Function must in the guard of command list mutex */
static gctINT
clfAddCommandToCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gctINT                  status;
    gctUINT                 i;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x",
                  CommandQueue, Command);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND,
              CL_INVALID_VALUE);

    /* If in thread, We don't need check event, as they all from same commandQueue,
       Out thread need check these event in worker thread, so, retain it to make sure
       it not free.  See clfProcessCommand()
    */
    for (i = 0; i < Command->numEventsInWaitList; ++i)
    {
        clfRetainEvent(Command->eventWaitList[i]);
    }

    Command->enqueueNo = CommandQueue->nextEnqueueNo;
    CommandQueue->nextEnqueueNo++;

    if (CommandQueue->commandTail)
    {
        clmASSERT(CommandQueue->numCommands > 0, CL_INVALID_VALUE);
        CommandQueue->numCommands++;
        Command->previous = CommandQueue->commandTail;
        CommandQueue->commandTail->next = Command;
        CommandQueue->commandTail = Command;
        Command->next = gcvNULL;
    }
    else
    {
        clmASSERT(CommandQueue->numCommands == 0, CL_INVALID_VALUE);
        CommandQueue->numCommands = 1;
        CommandQueue->commandHead = CommandQueue->commandTail = Command;
        Command->next = Command->previous = gcvNULL;
    }


    if (clmCOMMAND_SYNC_POINT(Command->type))
    {
        clfAddSyncPoint(CommandQueue, Command);
    }

    clfWakeUpCommandQueueWorker(CommandQueue);

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* This function should be inside mutex session. */
static gctINT
clfRemoveCommandFromCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gctINT                  status = CL_SUCCESS;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command != gcvNULL, CL_INVALID_VALUE);
    clmASSERT(Command->commandQueue == CommandQueue, CL_INVALID_VALUE);

    CommandQueue->numCommands--;

    if (Command->previous) {
        Command->previous->next = Command->next;
    }
    if (Command->next) {
        Command->next->previous = Command->previous;
    }
    if (Command == CommandQueue->commandHead) {
        CommandQueue->commandHead = Command->next;
    }
    if (Command == CommandQueue->commandTail) {
        CommandQueue->commandTail = Command->previous;
    }

    Command->previous = gcvNULL;
    Command->next = gcvNULL;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfGetCommandFromCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR *        Command
    )
{
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x",
                  CommandQueue, Command);

    clmCHECK_ERROR(CommandQueue == gcvNULL ||
                   CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE,
                   CL_INVALID_COMMAND_QUEUE);

    clmCHECK_ERROR(Command == gcvNULL, CL_INVALID_VALUE);

    if (CommandQueue->numCommands > 0)
    {
        if (CommandQueue->properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
        {
            /* Find a command that is ready to execute */
            gctBOOL eventsPending, cmdsPending, cmdsTerminate;
            clsCommand_PTR command = gcvNULL;

            command = CommandQueue->commandHead;
            while (command)
            {
                gctINT pendingEventStatus = clfCheckPendingEvents(command);
                eventsPending   = (pendingEventStatus == gcvSTATUS_TRUE);
                cmdsTerminate   = (pendingEventStatus == gcvSTATUS_TERMINATE);
                cmdsPending     = clfCheckPendingCommands(command);

                if (cmdsTerminate || (!eventsPending && !cmdsPending)) break;
                command = command->next;
            }

            if (command)
            {
                clfRemoveCommandFromCommandQueue(CommandQueue, command);
                *Command = command;
            }
            else
            {
                /* Command queue is not empty but there are no commands
                 * ready to execute so make sure worker does not fall asleep
                 */
                *Command = gcvNULL;
            }
        }
        else
        {
            gctBOOL eventsPending, cmdsPending, cmdsTerminate;
            clsCommand_PTR command = CommandQueue->commandHead;
            gctINT pendingEventStatus   = clfCheckPendingEvents(command);
            eventsPending   = (pendingEventStatus == gcvSTATUS_TRUE);
            cmdsTerminate   = (pendingEventStatus == gcvSTATUS_TERMINATE);
            cmdsPending     = clfCheckPendingCommands(command);

            if (cmdsTerminate || (!eventsPending && !cmdsPending))
            {
                /* Just get the head of the command queue */
                CommandQueue->numCommands--;
                *Command = command;
                CommandQueue->commandHead = command->next;

                if (CommandQueue->commandHead == gcvNULL)
                {
                    clmASSERT(CommandQueue->numCommands == 0, CL_INVALID_VALUE);
                    CommandQueue->commandTail = gcvNULL;
                }
                else
                {
                    CommandQueue->commandHead->previous = gcvNULL;
                }

                (*Command)->next = (*Command)->previous = gcvNULL;
            }
            else
            {
                *Command = gcvNULL;
            }
        }
    }
    else
    {
        *Command = gcvNULL;
    }

    gcmFOOTER_ARG("%d *Command=0x%x",
                  CL_SUCCESS, gcmOPT_POINTER(Command));
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* In in-thread mode, we call this inside the command queue list mutex.
   In out-thread mode, we call inside same thread of clfProcessReleaseCommand, don't need lock command queue list
*/
static gctINT
clfDeferredReleaseCommand(
    clsCommand_PTR          Command
    )
{
    clsCommandQueue_PTR     commandQueue;
    gctINT                  status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND,
              CL_INVALID_VALUE);

    commandQueue = Command->commandQueue;

    clmASSERT(commandQueue && commandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    if (commandQueue->deferredReleaseCommandTail)
    {
        Command->previous = commandQueue->deferredReleaseCommandTail;
        commandQueue->deferredReleaseCommandTail->next = Command;
        commandQueue->deferredReleaseCommandTail = Command;
        Command->next = gcvNULL;
    }
    else
    {
        commandQueue->deferredReleaseCommandHead = commandQueue->deferredReleaseCommandTail = Command;
        Command->next = Command->previous = gcvNULL;
    }

    /* Submmit command's release signal. */
    gcmONERROR((gctINT)gcoCL_SubmitSignal(Command->releaseSignal,
                                  commandQueue->context->process,
                                  Command->submitEngine));


    /* Wake up the queue worker to do the release after the release signal is set. */
    gcmONERROR((gctINT)gcoCL_SubmitSignal(commandQueue->workerStartSignal,
                                  commandQueue->context->process,
                                  Command->submitEngine));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfRemoveDeferredReleaseCommandFromCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gctINT                  status = CL_SUCCESS;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command != gcvNULL, CL_INVALID_VALUE);
    clmASSERT(Command->commandQueue == CommandQueue, CL_INVALID_VALUE);

    if (Command->previous)
    {
        Command->previous->next = Command->next;
    }

    if (Command->next)
    {
        Command->next->previous = Command->previous;
    }

    if (Command == CommandQueue->deferredReleaseCommandHead)
    {
        CommandQueue->deferredReleaseCommandHead = Command->next;
    }

    if (Command == CommandQueue->deferredReleaseCommandTail)
    {
        CommandQueue->deferredReleaseCommandTail = Command->previous;
    }

    Command->previous = gcvNULL;
    Command->next = gcvNULL;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

#define _cldPrintfConvSpecifiers "diouxXfFeEgGaAcsp"
#define _cldDigits               "0123456789"
#define _cldFlags                "-+ #0"
#define _cldLowercase "0123456789abcdefghijklmnopqrstuvwxyz"
#define _cldUppercase "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define LEFT 0x01
#define PLUS 0x02
#define SPACE 0x04
#define SPECIAL 0x08
#define ZEROPAD 0x10
#define SMALL 0x20
#define LARGE 0x40
#define SIGN 0x80

typedef enum _cleARGTYPE
{
    cleARGTYPE_CHAR = 1,
    cleARGTYPE_UCHAR,
    cleARGTYPE_SHORT,
    cleARGTYPE_USHORT,
    cleARGTYPE_LONG,
    cleARGTYPE_ULONG,
    cleARGTYPE_HALF,
    cleARGTYPE_INT,
    cleARGTYPE_UINT,
    cleARGTYPE_FLOAT,
    cleARGTYPE_DOUBLE

}cleARGTYPE;

gctBOOL clfIsNan(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) > (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL clfIsInf(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) == (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL clfIsInString(gctCHAR s, gctCHAR* string)
{
    while (*string)
    {
        if (s != *string++)
            continue;
        return gcvTRUE;
    }
    return gcvFALSE;
}

void
clfGetSingleFormat(
    gctCHAR* StartPtr,
    gctCHAR* EndPtr,
    gctCHAR* Format,
    gctINT*  VectorSize,
    gctINT*  ArgType,
    gctINT*  Flags,
    gctINT*  FieldWidth,
    gctINT*  Precision
    )
{
    gctCHAR chr;
    gctCHAR followChr;
    gctBOOL isVectorSpecifier = gcvFALSE;

    gctINT vectorSize = 0;
    *ArgType = 0;
    *VectorSize = 0;
    *Flags = 0;
    *FieldWidth = 0;
    *Precision = 0;

    while (StartPtr <= EndPtr) {
         chr = *StartPtr++;

         switch(chr) {
         case 'v':
             if (StartPtr < EndPtr)
             {
                 chr = *StartPtr++;
                 switch(chr) {
                 case '1':
                     chr = *StartPtr++;
                     if(StartPtr < EndPtr) {
                         if(chr != '6') return;
                     }
                     else return;
                     vectorSize = 16;
                     break;
                 case '2':
                     vectorSize = 2;
                     break;

                 case '4':
                     vectorSize = 4;
                     break;

                 case '8':
                     vectorSize = 8;
                     break;

                 default:
                     return;
                 }
             }
             else return;
             isVectorSpecifier = gcvTRUE;
             break;

         case 'h':
             *Format++ = chr;
             followChr = *StartPtr;
             if ((followChr == 'd') || (followChr == 'i'))
             {
                 *ArgType = cleARGTYPE_SHORT;
             }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
             {
                 *ArgType = cleARGTYPE_USHORT;
             }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                       (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
             {
                 *ArgType = cleARGTYPE_HALF;
             }
             switch (*StartPtr) {
             case 'h':
                 *Format++ = 'h';
                 StartPtr++;
                 followChr = *StartPtr;
                 if ((followChr == 'd') || (followChr == 'o'))
                 {
                     *ArgType = cleARGTYPE_CHAR;
                 }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                 {
                     *ArgType = cleARGTYPE_UCHAR;
                 }
                 break;

             case 'l':
                 if(isVectorSpecifier) {  /* hl applies to vector specifier appeared only */
                     Format--;
                     StartPtr++;
                     followChr = *StartPtr;
                     if ((followChr == 'd') || (followChr == 'o'))
                     {
                         *ArgType = cleARGTYPE_INT;
                     }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                     {
                         *ArgType = cleARGTYPE_UINT;
                     }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                               (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
                     {
                         *ArgType = cleARGTYPE_FLOAT;
                     }
                 }
                 else return;
                 break;

             default:
                 break;
             }

             break;

         case 'l':
             *Format++ = chr;
             followChr = *StartPtr;
              if (followChr == 'd' || followChr == 'o')
              {
                  *ArgType = cleARGTYPE_LONG;
              }else if (followChr == 'o' || followChr == 'u' || followChr == 'x' || followChr == 'X')
              {
                  *ArgType = cleARGTYPE_ULONG;
              }else if (followChr == 'a' || followChr == 'A' || followChr == 'e' || followChr == 'E' ||
                        followChr == 'f' || followChr == 'F' || followChr == 'g' || followChr == 'G')
              {
                  *ArgType = cleARGTYPE_DOUBLE;
              }
             break;

         default:
             {
                 /* process flags */
                 if(clfIsInString(chr, (gctCHAR*)_cldFlags))
                 {
                     *Format++ = chr;
                     switch(chr)
                     {
                     case '-':
                         *Flags |= LEFT;
                         break;
                     case '+':
                         *Flags |= PLUS;
                         break;
                     case ' ':
                         *Flags |= SPACE;
                         break;
                     case '#':
                         *Flags |= SPECIAL;
                         break;
                     case '0':
                         *Flags |= ZEROPAD;
                         break;
                     }
                 }else if (clfIsInString(chr, (gctCHAR*)_cldDigits)) /* get field width */
                 {
                     gctINT i = 0;
                     do
                     {
                         *Format++ = chr;
                         i = i*10 + chr - '0';
                         chr = *StartPtr++;
                     }while (clfIsInString(chr, (gctCHAR*)_cldDigits));
                     *FieldWidth = i;
                     StartPtr--;
                 }else if (chr == '.') /* get the precision */
                 {
                     *Format++ = chr;
                     chr = *StartPtr++;
                     if (clfIsInString(chr, (gctCHAR*)_cldDigits))
                     {
                         gctINT i = 0;
                         do
                         {
                             *Format++ = chr;
                             i = i*10 + chr - '0';
                             chr = *StartPtr++;
                         }while (clfIsInString(chr, (gctCHAR*)_cldDigits));
                         *Precision = i;
                         StartPtr--;
                     }
                     if (*Precision < 0)
                     {
                         *Precision = 0;
                     }
                 }else{
                     *Format++ = chr;
                 }
                 break;
             }
        }
    }

    *VectorSize = vectorSize;
    return;
}

void floatToaHex(
    double Num,
    gctINT Precision,
    gctBOOL IsUpperCase,
    gctCHAR* Buff,
    gctINT* DecimalPos,
    gctINT* Exp)
{
    gctINT intData;
    gctINT i = 0;
    gctINT j = 0;
    double decimalData;
    gctBOOL binaryData[255] = {0};
    gctBOOL binaryInt[255] = {0};
    gctCHAR* dig = (char*)_cldLowercase;
    gctINT len;

    if (IsUpperCase)
    {
        dig = (char*)_cldUppercase;
    }
    /* Process value sign */
    if (Num < 0.0)
    {
        Num = -Num;
        *Buff++ = '-';
    }
    else
    {
        *Buff++ = '+';
    }

    if (Num > -0.000005 && Num < 0.000005)
    {
        for (i = 0; i < Precision + 1; i++)
        {
            *Buff++ = '0';
        }
        *Buff++ = '\0';
        *DecimalPos = 1;
        *Exp = 0;
        return;
    }

    intData = (gctINT)Num;
    decimalData = Num - intData;
    *Exp = 0;

    if (intData == 0)
    {
        j = 0;
        while(decimalData < 1.0)
        {
            decimalData *= 2;
            j++;
        }
        *Exp = -j;
        intData = (int)decimalData;
        decimalData -= intData;
    }
    /* handle integer part */
    while(intData)
    {
        binaryInt[i++] = (intData % 2 == 1) ? gcvTRUE : gcvFALSE;
        intData /= 2;
    }
    *Exp = ((*Exp) == 0) ? i - 1 : *Exp;
    j = 0;
    while(i)
    {
        binaryData[j++] = binaryInt[--i];
    }

    if (decimalData > -0.000005 && decimalData < 0.000005)
    {
        for(i = 0; i < 4 * Precision; i++)
        {
            binaryData[j++] = gcvFALSE;
        }
    }else
    {
        /* handle decimal part */
        while(decimalData != 0.0)
        {
            gctINT t = (gctINT)(decimalData * 2);
            binaryData[j++] = (t > 0) ? gcvTRUE : gcvFALSE;
            decimalData = decimalData * 2 - t;
        }
    }

    len = j - 1;

    if (len % 4)
    {
        memset(binaryData + len + 1, 0, 4 - len % 4);
        len += 4 - len % 4;
    }

    *Buff++ = binaryData[0] ? '1':'0';
    *DecimalPos = 1;

    i = 0;
    j = 0;
    while((i < len))
    {
        *Buff++ = dig[((binaryData[i+1] ? 8 : 0) + (binaryData[i+2] ? 4 : 0) + (binaryData[i+3] ? 2 : 0) + (binaryData[i+4] ? 1 : 0)) % 16];
        i += 4;
        j++;
    }
    *Buff++ = '\0';
}

void fltRound(
    gctCHAR* NumBuf,
    gctINT* DecimalPos,
    gctINT Precision,
    gctBOOL IsUppderCase)
{
    gctCHAR* last_digit = NumBuf + 1 +Precision;
    gctCHAR* after_last = last_digit + 1;

    if (*after_last > '4')
    {
        gctCHAR *p = last_digit;
        gctINT carry = 1;

        do
        {
            gctINT sum;
            if (*p == '.')
            {
                p--;
            }
            if (IsUppderCase)
            {
                if (*p == '9')
                {
                    *p-- = 'A';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'F';
                    *p-- = (gctCHAR)(sum - carry * 23);
                }
            }else
            {
                if (*p == '9')
                {
                    *p-- = 'a';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'f';
                    *p-- = (gctCHAR)(sum - carry * 55);
                }
            }
        } while (carry && p >= NumBuf);

        /* We have fffff... which needs to be rounded to 100000.. */
        if (carry && p == NumBuf)
        {
            *p = '1';
            *DecimalPos += 1;
        }
    }
}

void floatToText(
    double Value,
    gctINT Precision,
    gctCHAR *Buf,
    gctBOOL IsUpperCase)
{
    char chBuffer[255] = {'\0'};

    char *cvtBuf = chBuffer;
    int decimalPos;
    int pos;
    int exp;

    floatToaHex(Value, Precision, IsUpperCase, cvtBuf, &decimalPos, &exp);
    fltRound(cvtBuf, &decimalPos, Precision, IsUpperCase);

    if ('-' == *cvtBuf++)
    {
        *Buf++ = '-';
    }

    if (*cvtBuf)
    {
        *Buf++ = '0';
        *Buf++ = IsUpperCase ? 'X' : 'x';
        *Buf++ = *cvtBuf;
        if (Precision > 0)
        {
            *Buf++ = '.';
        }
        gcoOS_MemCopy(Buf, cvtBuf + 1, Precision);
        Buf += Precision;
        {
            *Buf++ = IsUpperCase ? 'P' : 'p';
        }

        if (exp < 0)
        {
            *Buf++ = '-';
            exp = -exp;
        }else{
            *Buf++ = '+';
        }

        do
        {
            if ((exp / 10) == 0)
            {
                *Buf++ = (gctCHAR)(exp % 10 + '0');
                break;
            }
            else
            {
                *Buf++ = (gctCHAR)(exp / 10 + '0');
                exp = exp % 10;
            }
        }while(exp);
    }
    else
    {
        *Buf++ = '0';
        if (Precision > 0)
        {
            *Buf++ = '.';
            for (pos = 0; pos < Precision; pos++)
            {
                *Buf++ = '0';
            }
        }
    }
}

void printf_aA(
    gctCHAR *Str,
    double Num,
    gctINT Size,
    gctINT Precision,
    gctCHAR Fmt,
    gctINT Flags)
{
    gctCHAR tmp[255] = {'\0'};
    gctCHAR c, sign;
    gctINT n, i;
    gctBOOL isUpperCase = gcvFALSE;

    if (Flags & LEFT) Flags &= ~ZEROPAD;

    c = (Flags & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (Flags & SIGN)
    {
        if (Num < 0.0)
        {
            sign = '-';
            Num = -Num;
            Size--;
        }
        else if (Flags & PLUS)
        {
            sign = '+';
            Size--;
        }
        else if (Flags & SPACE)
        {
            sign = ' ';
            Size--;
        }
    }

    if (Precision < 0)
    {
        Precision = 6; /* Default precision: 6 */
    }

    isUpperCase = (Fmt == 'A') ? gcvTRUE : gcvFALSE;

    floatToText(Num, Precision, tmp, isUpperCase);

    n = strlen(tmp);

    Size -= n;
    if (!(Flags & (ZEROPAD | LEFT)))
    {
        while (Size-- > 0)
        {
            *Str++ = ' ';
        }
    }
    if (sign)
    {
        *Str++ = sign;
    }
    if (!(Flags & LEFT))
    {
        while (Size-- > 0)
        {
            *Str++ = c;
        }
    }
    for (i = 0; i < n; i++)
    {
        *Str++ = tmp[i];
    }
    while (Size-- > 0)
    {
        *Str++ = ' ';
    }
}

void
clfPrintData(
    gctPOINTER** Data,
    gctCHAR* Format,
    gctINT ArgType,
    gctCHAR FollowType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision,
    gctBOOL IsDoublePrecision)
{
    if (ArgType)
    {
        switch (ArgType)
        {
        case cleARGTYPE_CHAR:
            printf(Format, *(gctINT8*)(**Data));
            **Data = (gctCHAR*)(**Data) + 1;
            break;
        case cleARGTYPE_UCHAR:
            printf(Format, *(gctUINT8*)(**Data));
            **Data = (gctUINT8*)(**Data) + 1;
            break;
        case cleARGTYPE_SHORT:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_USHORT:
            printf(Format, *(gctUINT16*)(**Data));
            **Data = (gctUINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_LONG:
#if (defined(ANDROID) || gcdFPGA_BUILD || defined(__QNXNTO__))
            /*Special handle, Under the platforms above, print 64bit integer, %ld will print pointer value, %lld can print correct value*/
            if (!gcoOS_StrCmp(Format,"%ld"))
            {
                char* NewFormat = "%lld";
                printf(NewFormat, *(cl_long*)(**Data));
            }
            else
            {
                printf(Format, *(cl_long*)(**Data));
            }
#else
            printf(Format, *(cl_long*)(**Data));
#endif
            **Data = (cl_long*)(**Data) + 1;
            break;
        case cleARGTYPE_ULONG:
            printf(Format, *(cl_ulong*)(**Data));
            **Data = (cl_ulong*)(**Data) + 1;
            break;
        case cleARGTYPE_HALF:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_INT:
            printf(Format, *(gctINT32*)(**Data));
            **Data = (gctINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_UINT:
            printf(Format, *(gctUINT32*)(**Data));
            **Data = (gctUINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_FLOAT:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (clfIsNan(value))
                {
                    printf("%s", "nan");
                }else if (clfIsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(gctFLOAT*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, value);
                    }
                }

                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        case cleARGTYPE_DOUBLE:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (clfIsNan(value))
                {
                    printf("%s", "nan");
                }else if (clfIsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(double*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, *(double*)(**Data));
                    }
                }
                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch(FollowType)
        {
         case 'd':
         case 'i':
         case 'o':
         case 'u':
         case 'x':
         case 'X':
             printf(Format, *(gctINT32*)(**Data));
             **Data = (gctINT32*)(**Data) + 1;
             break;
         case 'a':
         case 'A':
         case 'e':
         case 'E':
         case 'f':
         case 'F':
         case 'g':
         case 'G':
             {
                 gctFLOAT value = *(gctFLOAT*)(**Data);
                 if (clfIsNan(value))
                 {
                     printf("%s", "nan");
                 }else if (clfIsInf(value))
                 {
                     printf("%s", "inf");
                 }else if (FollowType == 'a' || FollowType == 'A')
                 {
                     gctCHAR tmpBuf[512] = {'\0'};
                     printf_aA(tmpBuf, value, FieldWidth, Precision, FollowType, Flags);
                     printf("%s", tmpBuf);
                 }else
                 {
                     printf(Format, value);
                 }
                 **Data = (gctFLOAT*)(**Data) + 1;
             }
             break;
         case 'c':
             printf(Format, *(gctINT8*)(**Data));
             **Data = (gctINT8*)(**Data) + 1;
             break;
         default:
            break;
        }
    }
}

void
clfPrintfFmt(
    gctCHAR* FmtString,
    gctCHAR SpecificChar,
    gctPOINTER* Data,
    gctINT VectorSize,
    gctINT ArgType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision
    )
{
    gctINT i = 0;
    gctINT numVector = (VectorSize == 0) ? 1 : VectorSize;
    gctINT isDoublePrecision = 0;
    /* Get precision info */
    isDoublePrecision = *(gctINT*)(*Data);
    /* Get Data */
    *Data = (gctINT*)(*Data) + 1;
    switch(SpecificChar) {
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
        case 'c':
            while (i < numVector)
            {
                if (i > 0 && i < numVector)
                {
                    printf(",");
                }
                clfPrintData(&Data, FmtString, ArgType, SpecificChar, Flags, FieldWidth, Precision, isDoublePrecision);
                i++;
            }
            break;

        case 'p':
            printf("%016x", *(gctUINT*)(*Data));
            *Data = (gctUINT*)(*Data) + 1;
            break;

        default:
            break;
    }
}

void clfPrintParseData(
    gctCHAR *formatString,
    gctPOINTER *pData)
{
    gctCHAR* bufPtr = formatString;
    gctPOINTER data = *pData;
    gctINT tmpData = 0;

    while(*bufPtr || tmpData != 0xFFFFFFFF)
    {
        gctCHAR c = *bufPtr++;
        gctUINT value = 0;

        if(c == '%')
        {
            if(*bufPtr == '%')
            {
                bufPtr++;
                continue;
            }
            else
            {
                /*indentify the conversion specification */
                gctCHAR* startPtr = bufPtr - 1;
                while (*bufPtr)
                {
                    c = *bufPtr++;
                    if (clfIsInString(c, (gctCHAR*)_cldPrintfConvSpecifiers))
                    {
                        gctINT vectorSize = 0;
                        gctINT argType = 0;
                        gctINT flags = 0;
                        gctINT fieldWidth = 0;
                        gctINT precision = 0;
                        gctCHAR* endPtr = bufPtr - 1;
                        gctCHAR fmt[255] = {'\0'};
                        /* get vector size and length and new format */
                        clfGetSingleFormat(startPtr, endPtr, fmt, &vectorSize, &argType, &flags, &fieldWidth, &precision);
                        if (c == 's')
                        {
                            value = *((gctINT*)data + 1);
                            if (value == 0xFFFFFFFF) /* handle printf("%s", 0);*/
                            {
                                printf(fmt, "(null)");
                            }
                            else
                            {
                                /* the string is stored in const buffer, offset is stored in printf buffer */
                                gctUINT offset = value;
                                printf(fmt, formatString + offset);
                            }
                            data = (gctINT*)data + 2;
                        }
                        else
                        {
                            clfPrintfFmt(fmt, c, &data, vectorSize, argType, flags, fieldWidth, precision);
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            gctINT constStrLen = 0;
            gctCHAR* constString;
            gctCHAR* startConstStrPtr;
            bufPtr--;
            startConstStrPtr = bufPtr;

            while((*bufPtr) && (*bufPtr != '%'))
            {
                constStrLen++;
                bufPtr++;
            }
            gcoOS_Allocate(gcvNULL, (constStrLen+1)*sizeof(gctCHAR), (gctPOINTER *)&constString);
            gcoOS_StrCopySafe(constString, constStrLen + 1, startConstStrPtr);
            *(constString + constStrLen) = '\0';
            printf("%s", constString);
            gcoOS_Free(gcvNULL, constString);
        }

        tmpData = *((gctINT *)data+1);

        if((*bufPtr == 0) && (tmpData != 0xFFFFFFFF))
        {
            bufPtr = &formatString[tmpData];
            data = (gctINT*)data + 2;
        }
    }

    *pData = data;
}

/* Now , put this inside commandQueue Worker, make sure it's inside command list mutex to protect HW context acces to in-thread mode */
gctINT
clfProcessDeferredReleaseCommandList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    clsCommand_PTR          command, nextCommand;
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    clmCHECK_ERROR(CommandQueue == gcvNULL ||
                   CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE,
                   CL_INVALID_COMMAND_QUEUE);

    command = CommandQueue->deferredReleaseCommandHead;

    while (command)
    {
        nextCommand = command->next;

        if (gcmIS_SUCCESS(gcoCL_WaitSignal(command->releaseSignal, 0)))
        {
            clfRemoveDeferredReleaseCommandFromCommandQueue(CommandQueue, command);

            if (command->type == clvCOMMAND_NDRANGE_KERNEL)
            {
                gctUINT i;
                char *fmt;
                gctINT printBufferSizePerThread = 0;
                gctPOINTER printBufferAddress = gcvNULL;
                gctPOINTER curPrintBufferAddress = gcvNULL;
                gctUINT printThreadNum = 0;
                clsCommandNDRangeKernel_PTR NDRangeKernel = &command->u.NDRangeKernel;
                fmt = ((gcSHADER)(NDRangeKernel->states->binary))->constantMemoryBuffer;

                for (i = 0; i < NDRangeKernel->numArgs; i++)
                {
                    if (NDRangeKernel->args[i].uniform && isUniformWorkItemPrintfBufferSize(NDRangeKernel->args[i].uniform))
                    {
                        printBufferSizePerThread = NDRangeKernel->args[i].printBufferSizePerThread;
                    }
                    else if (NDRangeKernel->args[i].isMemAlloc)
                    {
                        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) NDRangeKernel->args[i].data;
                        if (isUniformPrintfAddress(NDRangeKernel->args[i].uniform))
                        {
                            printBufferAddress = (void*)((gctUINT*)(memAllocInfo->logical));
                            printThreadNum = NDRangeKernel->args[i].printThreadNum;
                        }
                    }
                }

                if (printBufferAddress && printBufferSizePerThread > 0)
                {
                    for (i = 0; i < printThreadNum; i++)
                    {
                        curPrintBufferAddress = printBufferAddress;
                        do
                        {
                            gctINT writeMask = *(gctINT*)curPrintBufferAddress;
                            /* Still printf left in this thread. */
                            if (writeMask == __OCL_PRINTF_WRITE_MASK__)
                            {
                                /* Skip writeMask. */
                                curPrintBufferAddress = (gctINT*)curPrintBufferAddress + 1;
                                /* Skip offset. */
                                curPrintBufferAddress = (gctINT*)curPrintBufferAddress + 1;
                                clfPrintParseData(fmt, &curPrintBufferAddress);
                            }
                            /* No printf left in this thread, switch to the next thread. */
                            else
                            {
                                break;
                            }
                        } while (gcmPTR_TO_UINT64(curPrintBufferAddress) < ((gcmPTR_TO_UINT64(printBufferAddress) + printBufferSizePerThread)));

                        printBufferAddress = gcmUINT64_TO_PTR(gcmPTR_TO_UINT64(printBufferAddress) + printBufferSizePerThread);
                    }
                }
            }

            if (command->event)
            {
                /* Let the event list worker to finish the event */
                clfSetEventForFinish(command);
            }

            clfReleaseCommand(command);
        }

        command = nextCommand;
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfAddSyncPoint(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    clsSyncPoint_PTR        syncPoint;
    gctPOINTER              pointer;
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x", CommandQueue, Command);

    clmASSERT(Command, CL_INVALID_VALUE);
    clmASSERT(CommandQueue, CL_INVALID_VALUE);

    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsSyncPoint), &pointer), CL_OUT_OF_HOST_MEMORY);

    syncPoint                       = (clsSyncPoint_PTR) pointer;
    syncPoint->enqueueNo            = Command->enqueueNo;
    syncPoint->previous             = gcvNULL;

    /* Acquire synchronization mutex. */
    clfLockSyncPointList(CommandQueue);

    syncPoint->next = CommandQueue->syncPointList;

    if (syncPoint->next)
    {
        syncPoint->next->previous = syncPoint;
    }

    CommandQueue->syncPointList = syncPoint;

    /* Release synchronization mutex. */
    clfUnlockSyncPointList(CommandQueue);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfRemoveSyncPoint(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    clsSyncPoint_PTR        syncPoint;
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x", CommandQueue, Command);

    clmASSERT(Command, CL_INVALID_VALUE);
    clmASSERT(CommandQueue, CL_INVALID_VALUE);

    /* Acquire synchronization mutex. */
    clfLockSyncPointList(CommandQueue);

    /* Find the sync point associated with the command */
    syncPoint = CommandQueue->syncPointList;

    if (syncPoint == gcvNULL)
    {
        /* No sync point */
        clfUnlockSyncPointList(CommandQueue);
        return CL_SUCCESS;
    }

    while (syncPoint)
    {
        if (syncPoint->enqueueNo == Command->enqueueNo) break;

        syncPoint = syncPoint->next;
    }

    clmASSERT(syncPoint, CL_INVALID_VALUE);

    /* Remove from the sync point list */
    if (syncPoint == CommandQueue->syncPointList)
    {
        CommandQueue->syncPointList = syncPoint->next;
    }
    if (syncPoint->previous)
    {
        syncPoint->previous->next = syncPoint->next;
    }
    if (syncPoint->next)
    {
        syncPoint->next->previous = syncPoint->previous;
    }

    /* Release synchronization mutex. */
    clfUnlockSyncPointList(CommandQueue);

    gcoOS_Free(gcvNULL, syncPoint);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfStartCommand(
    clsCommand_PTR          Command
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command, CL_INVALID_VALUE);

    if (Command->event)
    {
        clfSetEventExecutionStatus(Command->event, CL_SUBMITTED);

        clfScheduleEventCallback(Command->event, CL_SUBMITTED);
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);

    /* Return the status. */
    return status;
}

static gctINT
clfFinishCommand(
    clsCommand_PTR          Command,
    gctINT                  CommandStatus
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Command=0x%x CommandStatus=%d", Command, CommandStatus);

    clmASSERT(Command, CL_INVALID_VALUE);

    if (Command->event)
    {
        if (gcmIS_ERROR(CommandStatus))
        {
            clfFinishEvent(Command->event, CommandStatus);
        }
        else if (Command->releaseSignal == gcvNULL)
        {
            clfSubmitEventForFinish(Command);
        }
    }

    if (Command->eventWaitList)
    {
        gcmASSERT(Command->numEventsInWaitList);
        gcoOS_Free(gcvNULL, (gctPOINTER)Command->eventWaitList);
        Command->eventWaitList = gcvNULL;
    }

    if (Command->releaseSignal == gcvNULL)
    {
        clfReleaseCommand(Command);
    }
    else
    {
        clfDeferredReleaseCommand(Command);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Free the command object. */
    clfReleaseCommand(Command);

    gcmFOOTER_ARG("%d", status);

    /* Return the status. */
    return status;
}

static gctINT
clfProcessCommand(
    clsCommand_PTR     Command
    )
{
    clsCommandQueue_PTR     commandQueue;
    /*clsContext_PTR          context;*/
    gctINT                  status;
    gctUINT                 i;
    gctINT                  pendingEventStatus;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command, CL_INVALID_VALUE);
    commandQueue = Command->commandQueue;

    clmASSERT(commandQueue, CL_INVALID_VALUE);
    /*context = commandQueue->context;*/

    pendingEventStatus = clfCheckPendingEvents(Command);
    gcmASSERT(pendingEventStatus != gcvSTATUS_TRUE);

    if (pendingEventStatus == gcvSTATUS_TERMINATE)
    {
        clfWakeUpCommandQueueWorker(commandQueue);
    }
    else
    {
        /*At this point, we Can sure the eventListWaitlist will be no used*/
        for (i = 0; i < Command->numEventsInWaitList; ++i)
        {
            clfReleaseEvent(Command->eventWaitList[i]);
        }
        /* Handle commands. */
        clfStartCommand(Command);
        status = Command->handler(Command);
        clfFinishCommand(Command, status);

        /* Add signals to wake up worker when the command is finished. */
        clfWakeUpCommandQueueWorker(commandQueue);

    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

static void
clfLockCommandQueueList(
    clsContext_PTR      Context
    )
{
    gcmASSERT(Context != gcvNULL);

    if (Context->queueListMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                        Context->queueListMutex,
                                        gcvINFINITE));
    }
}

static void
clfUnlockCommandQueueList(
    clsContext_PTR      Context
    )
{
    gcmASSERT(Context != gcvNULL);

    if (Context->queueListMutex != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                        Context->queueListMutex));
    }
}

void
clfWakeUpAllCommandQueueWorkers(
    clsContext_PTR      Context
    )
{
    clsCommandQueue_PTR     queue;

    /* Lock the command queue list in the context. */
    clfLockCommandQueueList(Context);

    queue = Context->queueList;

    /* Wake up all queue worker threads */
    while (queue != gcvNULL &&
        !queue->inThread)
    {
        clfWakeUpCommandQueueWorker(queue);
        queue = queue->next;
    }

    /* Unlock the command queue list in the context. */
    clfUnlockCommandQueueList(Context);
}

static gctINT
clfCreateCommitRequest(
    clsCommandQueue_PTR     CommandQueue,
    gctBOOL                 Stall,
    clsCommitRequest_PTR *  CommitRequest
    )
{
    gctINT                  status;
    clsCommitRequest_PTR    commitRequest = gcvNULL;
    gctPOINTER              pointer;
    gctUINT                 i;
    gctBOOL                 enableAsyc = gcvFALSE;

    gcmHEADER_ARG("Stall=%d" , Stall);

    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsCommitRequest), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    enableAsyc = gcoHAL_GetOption(gcvNULL, gcvOPTION_OCL_ASYNC_BLT) &&
                 CommandQueue->device->deviceInfo.asyncBLT;

    commitRequest                   = (clsCommitRequest_PTR) pointer;
    commitRequest->stall            = Stall;
    commitRequest->previous         = gcvNULL;
    commitRequest->next             = gcvNULL;

    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
    {
        commitRequest->event[i]    = gcvNULL;

        if (i == gcvENGINE_BLT && enableAsyc)
        {
            clmONERROR(clfAllocateEvent(CommandQueue->context,
                                        &commitRequest->event[i]),
                      CL_OUT_OF_HOST_MEMORY);

            clfSetEventExecutionStatus(commitRequest->event[i], CL_RUNNING);
        }

        if (i == gcvENGINE_RENDER)
        {
            clmONERROR(clfAllocateEvent(CommandQueue->context,
                                        &commitRequest->event[i]),
                      CL_OUT_OF_HOST_MEMORY);

            clfSetEventExecutionStatus(commitRequest->event[i], CL_RUNNING);
        }
    }

    *CommitRequest = commitRequest;

    gcmFOOTER_ARG("%d commitRequest=0x%x",
                  CL_SUCCESS, commitRequest);

    return CL_SUCCESS;

OnError:
    if (commitRequest != gcvNULL)
    {
        gcoOS_Free(gcvNULL, commitRequest);
    }

    *CommitRequest = gcvNULL;

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfDeleteCommitRequest(
    clsCommitRequest_PTR     CommitRequest
    )
{
    gctINT                  status;
    gctUINT                 i;

    gcmHEADER_ARG("CommitRequest=0x%x", CommitRequest);

    clmASSERT(CommitRequest, CL_INVALID_VALUE);

    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
    {
        if (CommitRequest->event[i] != gcvNULL)
        {
            gcmVERIFY_OK(clfReleaseEvent(CommitRequest->event[i]));
            CommitRequest->event[i] = gcvNULL;
        }
    }

    gcmVERIFY_OK(gcoOS_Free(gcvNULL, CommitRequest));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* must in command list mutex */
static gctINT
clfAddCommitRequestToList(
    clsCommandQueue_PTR     CommandQueue,
    clsCommitRequest_PTR    CommitRequest
    )
{
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x CommitRequest=%d" , CommandQueue, CommitRequest);

    clmASSERT(CommandQueue, CL_INVALID_VALUE);
    clmASSERT(CommitRequest, CL_INVALID_VALUE);

    CommitRequest->nextEnqueueNo = CommandQueue->nextEnqueueNo;

    /* Add the commit request into the list */
    CommitRequest->next = CommandQueue->commitRequestList;

    if (CommitRequest->next != gcvNULL)
    {
        CommitRequest->next->previous = CommitRequest;
    }

    CommandQueue->commitRequestList = CommitRequest;

    /* Wake up the worker */
    clfWakeUpCommandQueueWorker(CommandQueue);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* This function should be inside the command list mutex session. */
static gctINT
clfProcessCommitRequestList(
    clsCommandQueue_PTR     CommandQueue
    )
{
    gctINT                  status;
    clsCommitRequest_PTR    commitRequest, nextCommitRequest;
    gctBOOL                 doneStall = gcvFALSE;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    clmASSERT(CommandQueue, CL_INVALID_VALUE);

    if (CommandQueue->commitRequestList != gcvNULL)
    {
        gctBOOL             checkEnqueueNoForFlush = gcvFALSE;
        gctUINT64           minExistingEnqueueNoForFlush = CommandQueue->nextEnqueueNo;
        gctBOOL             checkEnqueueNoForFinish;
        gctUINT64           minExistingEnqueueNoForFinish;
        clsCommand_PTR      command;
        gctUINT32           i;
        gctSIGNAL           signals[gcvENGINE_GPU_ENGINE_COUNT];

        /* Get the min existing enqueue no for clFlush */
        if (CommandQueue->numCommands > 0)
        {
            checkEnqueueNoForFlush = gcvTRUE;

            minExistingEnqueueNoForFlush = CommandQueue->commandHead->enqueueNo;
        }

        /* Get the min existing enqueue no for clFinish */
        checkEnqueueNoForFinish = checkEnqueueNoForFlush;
        minExistingEnqueueNoForFinish = minExistingEnqueueNoForFlush;

        command = CommandQueue->deferredReleaseCommandHead;

        while (command)
        {
            if (minExistingEnqueueNoForFinish > command->enqueueNo)
            {
                checkEnqueueNoForFinish = gcvTRUE;

                minExistingEnqueueNoForFinish = command->enqueueNo;
            }

            command = command->next;
        }

        /* Go through all commit requests */
        commitRequest = CommandQueue->commitRequestList;

        do
        {
            /* Get the next commit request */
            nextCommitRequest = commitRequest->next;

            /* Handle the commit request */
            if (commitRequest->stall)
            {
                if (!checkEnqueueNoForFinish
                    || commitRequest->nextEnqueueNo <= minExistingEnqueueNoForFinish)
                {
                    /* Remove the commit request from the list */
                    if (commitRequest == CommandQueue->commitRequestList)
                    {
                        CommandQueue->commitRequestList = commitRequest->next;
                    }

                    if (commitRequest->previous != gcvNULL)
                    {
                        commitRequest->previous->next = commitRequest->next;
                    }

                    if (commitRequest->next != gcvNULL)
                    {
                        commitRequest->next->previous = commitRequest->previous;
                    }

                    doneStall = gcvTRUE;

                    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
                    {
                        if (commitRequest->event[i])
                        {
                            signals[i] = commitRequest->event[i]->finishSignal;
                        }
                        else
                        {
                            signals[i] = gcvNULL;
                        }
                    }

                    /* copy the signal to local var to keep thread safe */
                    for(i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
                    {
                        if (signals[i])
                        {
                            clfAddEventToEventList(commitRequest->event[i]);

                            gcmONERROR(gcoCL_SubmitSignal(signals[i],
                                                          CommandQueue->context->process,
                                                          i));

                            /* Submit the signal to wake up the event list worker to handle the event. */
                            gcmONERROR(gcoCL_SubmitSignal(CommandQueue->context->eventListWorkerStartSignal,
                                                          CommandQueue->context->process,
                                                          i));
                        }
                    }
                }
            }
            else
            {
                if (!checkEnqueueNoForFlush
                    || commitRequest->nextEnqueueNo <= minExistingEnqueueNoForFlush)
                {
                    /* Remove the commit request from the list */
                    if (commitRequest == CommandQueue->commitRequestList)
                    {
                        CommandQueue->commitRequestList = commitRequest->next;
                    }

                    if (commitRequest->previous != gcvNULL)
                    {
                        commitRequest->previous->next = commitRequest->next;
                    }

                    if (commitRequest->next != gcvNULL)
                    {
                        commitRequest->next->previous = commitRequest->previous;
                    }

                    /* Commit all previously queued commands/events to GPU */
                    gcmONERROR(gcoCL_Commit(gcvFALSE));

                    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
                    {
                        if (commitRequest->event[i])
                        {
                            signals[i] = commitRequest->event[i]->completeSignal;
                        }
                        else
                        {
                            signals[i] = gcvNULL;
                        }
                    }

                    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
                    {
                        /* Wake up the corresponding clFlush thead by signal directly */
                        if(signals[i])
                        {
                            gcmONERROR(gcoCL_SetSignal(signals[i]));
                        }
                    }
                }
            }

            /* Goto the next commit request */
            commitRequest = nextCommitRequest;
        }
        while (commitRequest != gcvNULL);

        if (doneStall)
        {
            /* Commit all stall event commands to GPU */
            gcmONERROR(gcoCL_Commit(gcvFALSE));
        }
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctINT
clfFlushCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    IN gctBOOL              Stall
    )
{
    gctINT                  status;
    clsCommitRequest_PTR    commitRequest = gcvNULL;
    gctUINT32 i = 0;
    gctBOOL locked = gcvFALSE;
    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    /* Create a new commit request */
    gcmONERROR(clfCreateCommitRequest(CommandQueue, Stall, &commitRequest));

    clfLockCommandList(CommandQueue);
    locked = gcvTRUE;

    if (CommandQueue->inThread)
    {
        gcmONERROR(clfProcessCommitInThread(CommandQueue,commitRequest));
    }
    else
    {
        gcmONERROR(clfAddCommitRequestToList(CommandQueue, commitRequest));
    }

    clfUnlockCommandList(CommandQueue);
    locked = gcvFALSE;

    /* Wait for the commit is done */
    for(i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i ++)
    {
        if(commitRequest->event[i])
        {
            gcmVERIFY_OK(gcoCL_WaitSignal(commitRequest->event[i]->completeSignal, gcvINFINITE));
        }
    }

    /* Delete the used commit request */
    gcmVERIFY_OK(clfDeleteCommitRequest(commitRequest));

     gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

     /* we need a function to flush main thread event for multi-thread mode.*/
     gcmONERROR(gcoCL_Flush(Stall));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (locked)
    {
        clfUnlockCommandList(CommandQueue);
    }

    if (commitRequest)
    {
        gcmVERIFY_OK(clfDeleteCommitRequest(commitRequest));
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfSubmitCommand(
    clsCommandQueue_PTR         CommandQueue,
    clsCommand_PTR              Command,
    gctBOOL                     Blocking
    )
{
    cl_event        commandEvent = gcvNULL;
    gctINT          status;
    gctUINT         i;
    gctBOOL         locked = gcvFALSE;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x Blocking=%d",
                  CommandQueue, Command, Blocking);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND,
              CL_INVALID_VALUE);

    if (Command->outEvent ||
        Blocking)
    {
        /* Create event to track status of this command */
        clmONERROR(clfAllocateEvent(CommandQueue->context, &commandEvent),
                   CL_OUT_OF_HOST_MEMORY);

        commandEvent->commandType = clmGET_COMMANDTYPE(Command->type);
        commandEvent->queue = CommandQueue;

        if(Blocking) clfRetainEvent(commandEvent);

        clfSetEventExecutionStatus(commandEvent, CL_QUEUED);

        if (Command->outEvent)
        {
            clfRetainEvent(commandEvent);

            *Command->outEvent = commandEvent;

            /* check if outEvent is from user Command */
            for (i = 0; i < Command->numEventsInWaitList; i++)
            {
                if (Command->eventWaitList[i]->userEvent ||
                    Command->eventWaitList[i]->dominateByUser)
                {
                    commandEvent->dominateByUser = gcvTRUE;
                    break;
                }
            }
        }

        Command->event = commandEvent;
    }

    clfLockCommandList(CommandQueue);
    locked = gcvTRUE;
    if (CommandQueue->inThread)
    {
        /* Check if we need switch to thread worker */
        CommandQueue->inThread = clfChooseThreadMode(CommandQueue, Command);
    }

    if (CommandQueue->inThread)
    {
        /* Still in thread mode */
        clfProcessCommandInThread(CommandQueue, Command);
    }
    else
    {
        /* Direct switch to worker, don't need flush command,
           as command already been handled (finish or in command buffer already) */
        clfAddCommandToCommandQueue(CommandQueue, Command);
    }
    clfUnlockCommandList(CommandQueue);
    locked = gcvFALSE;

    if (Blocking)
    {
        /* Wait for the command to finish. */
        clmASSERT(commandEvent, CL_INVALID_VALUE);
        clfWaitForEvent(commandEvent);
        clfReleaseEvent(commandEvent);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (locked)
    {
        clfUnlockCommandList(CommandQueue);
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* Function must inside command list mutex */
static gctINT
clfProcessCommitInThread(
    clsCommandQueue_PTR     CommandQueue,
    clsCommitRequest_PTR    CommitRequest
    )
{
    gceSTATUS           status;
    gctUINT32           i;
    gcmDECLARE_SWITCHVARS;

    gcmSWITCH_TO_HW(CommandQueue->hardware);

    /* Handle the commit request */
    if (CommitRequest->stall)
    {
        for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
        {
            /* Submit an event command for the signal for clFinish */
            if(CommitRequest->event[i] &&
               CommitRequest->event[i]->finishSignal)
            {
                clfAddEventToEventList(CommitRequest->event[i]);

                gcmONERROR(gcoCL_SubmitSignal(CommitRequest->event[i]->finishSignal,
                                              CommandQueue->context->process,
                                              i));

                 /* Submit the signal to wake up the event list worker to handle the event. */
                 gcmONERROR(gcoCL_SubmitSignal(CommandQueue->context->eventListWorkerStartSignal,
                                               CommandQueue->context->process,
                                               i));
            }
        }

        /* Commit all stall event commands to GPU */
        gcmONERROR(gcoCL_Commit(gcvFALSE));
    }
    else
    {
        /* Commit all previously queued commands/events to GPU */
        gcmONERROR(gcoCL_Commit(gcvFALSE));

        for (i  = 0 ; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
        {
            /* Wake up the corresponding clFlush thead by signal directly */
            if(CommitRequest->event[i] &&
                CommitRequest->event[i]->completeSignal)
            {
                gcmONERROR(gcoCL_SetSignal(CommitRequest->event[i]->completeSignal));
            }
        }
    }
OnError:
    gcmRESTORE_HW();

    return (gctINT)status;
}

static gceSTATUS
clfCheckPendingEventsListInThread(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR      Command,
    gctUINT             numEventsInWaitList,
    const clsEvent_PTR  *eventWaitList
    )
{
    gctUINT             i;
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("Command=0x%x", Command);

    if (!Command)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    for (i = 0; i < numEventsInWaitList; i++)
    {
        gctINT execution = clfGetEventExecutionStatus(eventWaitList[i]);

        if (!clmNO_ERROR(execution))
        {
            if (Command->event)
            {
                clfFinishEvent(Command->event, execution);
            }
            gcmFOOTER_ARG("status=%d", gcvSTATUS_TERMINATE);
            return gcvSTATUS_TERMINATE;
        }
    }

    gcmFOOTER_NO();
    return status;
}

/* This function must inside command queue list lock */
static gctINT
clfProcessCommandInThread(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcmDECLARE_SWITCHVARS;

    gcmSWITCH_TO_HW(CommandQueue->hardware);

    if (Command)
    {
        gceSTATUS pendingStatus = clfCheckPendingEventsListInThread(CommandQueue, Command, Command->numEventsInWaitList, Command->eventWaitList);

        if (clmIS_ERROR(pendingStatus))
        {
            gcmASSERT(gcvFALSE);
        }

        if ((pendingStatus != gcvSTATUS_TERMINATE))
        {
            clfStartCommand(Command);
            status = Command->handler(Command);

            if (clmIS_ERROR(clfFinishCommand(Command, status)))
                goto OnError;
        }
    }

OnError:
    gcmRESTORE_HW();

    return (gctINT)status;
}


gctTHREAD_RETURN CL_API_CALL
clfCommandQueueWorker(
    gctPOINTER Data
    )
{
    clsCommandQueue_PTR commandQueue = (clsCommandQueue_PTR) Data;
    clsCommand_PTR      command = gcvNULL;
    gctBOOL             acquired = gcvFALSE;
    gcmDECLARE_SWITCHVARS;

    /* Use the same hardware for command queue worker. */
    gcmSWITCH_TO_HW(commandQueue->hardware);

    while (gcvTRUE)
    {
        /* Wait for the start signal. */
        gcmVERIFY_OK(gcoCL_WaitSignal(commandQueue->workerStartSignal, gcvINFINITE));

        /* Check the thread's stop signal. */
        if (gcmIS_SUCCESS(gcoCL_WaitSignal(commandQueue->workerStopSignal, 0)))
        {
            /* Stop had been signaled, exit. */
            break;
        }

        /* Lock the command list in this command queue. */
        clfLockCommandList(commandQueue);
        acquired = gcvTRUE;

        /* Both in-thread and out-thread mode, we put this inside command list lock,
           If we put this in none current commandqueue hw context(some other thread), we should protect deferred list only,
           If put in this commandqueue hw context(here), we need put inside this protect to avoid hw context been access at same time
           And in thread mode, we don't need protect deferred list, as insert to deferred list is wrapped by command list lock.
           If this is triggered by in thread mode, the commitRequest and commandlist would be empty.
           If this is triggered by out thread mode, we process as usual.
        */
        clfProcessDeferredReleaseCommandList(commandQueue);

        clfProcessCommitRequestList(commandQueue);

        while (gcvTRUE)
        {
            clfGetCommandFromCommandQueue(commandQueue, &command);

            if (command == gcvNULL) break;

            /* Unlock the command list in the command queue. */
            clfUnlockCommandList(commandQueue);
            acquired = gcvFALSE;

            if (clmIS_ERROR(clfProcessCommand(command))) goto OnError;

            /* Lock the command list in this command queue. */
            clfLockCommandList(commandQueue);
            acquired = gcvTRUE;

            clfProcessCommitRequestList(commandQueue);
        }

        /* Unlock the command list in the command queue. */
        clfUnlockCommandList(commandQueue);
        acquired = gcvFALSE;
    }
    gcmRESTORE_HW();
OnError:

    gcmRESTORE_HW();
    if (acquired)
    {
        /* Unlock the command list in the command queue. */
        clfUnlockCommandList(commandQueue);
    }
    return (gctTHREAD_RETURN) 0;
}

gctINT
clfExecuteCommandMarker(
    clsCommand_PTR          Command
    )
{
    clsCommandQueue_PTR     commandQueue = Command->commandQueue;
    gctINT                  status = CL_SUCCESS;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);
    clmASSERT(Command->type == clvCOMMAND_MARKER, CL_INVALID_VALUE);

    EVENT_SET_CPU_RUNNING(Command);

    status = clfRemoveSyncPoint(commandQueue, Command);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandBarrier(
    clsCommand_PTR          Command
    )
{
    clsCommandQueue_PTR     commandQueue = Command->commandQueue;
    gctINT                  status = CL_SUCCESS;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);
    clmASSERT(Command->type == clvCOMMAND_BARRIER, CL_INVALID_VALUE);

    EVENT_SET_CPU_RUNNING(Command);

    status = clfRemoveSyncPoint(commandQueue, Command);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandWaitForEvents(
    clsCommand_PTR  Command
    )
{
    clsCommandQueue_PTR     commandQueue = Command->commandQueue;
    gctINT                  status = CL_SUCCESS;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);
    clmASSERT(Command->type == clvCOMMAND_WAIT_FOR_EVENTS, CL_INVALID_VALUE);

    EVENT_SET_CPU_RUNNING(Command);

    status = clfRemoveSyncPoint(commandQueue, Command);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS clfConstructWorkerThread(
    clsCommandQueue_PTR CommandQueue
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (CommandQueue)
    {
        if (CommandQueue->workerStartSignal == gcvNULL)
        {
            /* Create thread start signal. */
            clmONERROR(gcoCL_CreateSignal(gcvFALSE,
                                          &CommandQueue->workerStartSignal),
                       CL_OUT_OF_HOST_MEMORY);
        }

        if (CommandQueue->workerStopSignal == gcvNULL)
        {
            /* Create thread stop signal. */
            clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                          &CommandQueue->workerStopSignal),
                       CL_OUT_OF_HOST_MEMORY);
        }

        if (CommandQueue->workerThread == gcvNULL)
        {
            /* Start the worker thread. */
            clmONERROR(gcoOS_CreateThread(gcvNULL,
                                          clfCommandQueueWorker,
                                          CommandQueue,
                                          &CommandQueue->workerThread),
                       CL_OUT_OF_HOST_MEMORY);
        }
    }

OnError:
    return status;
}

gceSTATUS clfDestroyWorkerThread(
    clsCommandQueue_PTR CommandQueue
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (CommandQueue)
    {
        /* Send signal to stop worker thread. */
        if (CommandQueue->workerStopSignal)
        {
            gcmONERROR(gcoCL_SetSignal(CommandQueue->workerStopSignal));
        }

        if (CommandQueue->workerStartSignal)
        {
            gcmONERROR(gcoCL_SetSignal(CommandQueue->workerStartSignal));
        }

        gcmONERROR(gcoCL_Flush(gcvTRUE));

        /* Wait until the thread is closed. */
        if (CommandQueue->workerThread)
        {
            gcoOS_CloseThread(gcvNULL, CommandQueue->workerThread);
            CommandQueue->workerThread = gcvNULL;
        }

        /* Free signals and mutex. */
        if (CommandQueue->workerStartSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(CommandQueue->workerStartSignal));
            CommandQueue->workerStartSignal = gcvNULL;
        }

        if (CommandQueue->workerStopSignal)
        {
            gcmVERIFY_OK(gcoCL_DestroySignal(CommandQueue->workerStopSignal));
            CommandQueue->workerStopSignal = gcvNULL;
        }
    }

OnError:
    return status;
}

gctBOOL
clfChooseThreadMode(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gctUINT             i;
    gctBOOL             inThread = gcvTRUE;

    gcmHEADER_ARG("Command=0x%x", Command);

    if ((CommandQueue->properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ||
        (gcoHAL_GetOption(gcvNULL, gcvOPTION_OCL_IN_THREAD) == gcvFALSE))
    {
        inThread = gcvFALSE;
    }

    for (i = 0; i < Command->numEventsInWaitList; i++)
    {
        if (Command->eventWaitList[i]->userEvent ||
            Command->eventWaitList[i]->dominateByUser ||
            Command->eventWaitList[i]->queue != CommandQueue)
        {
            inThread = gcvFALSE;
        }
    }

    gcmFOOTER_NO();
    return inThread;
}


gceSTATUS clfRetainCommandQueue(
    cl_command_queue CommandQueue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, CommandQueue->referenceCount, gcvNULL));

    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS clfReleaseCommandQueue(
    cl_command_queue    CommandQueue
    )
{
    gceSTATUS              status;
    gctINT32            oldReference;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, CommandQueue->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        /* Implicit flush. */
        gcmONERROR(clfFlushCommandQueue(CommandQueue, gcvFALSE));

#if VIVANTE_PROFILER
        /* Destroy the profiler. */
        clfDestroyProfiler(CommandQueue);
#endif

        /* Clean up the private buffer list. */
        if (CommandQueue->privateBufList)
        {
             clsPrivateBuffer_PTR privateBuf = gcvNULL;
             clsPrivateBuffer_PTR tmpPrivateBuf = gcvNULL;

             for (privateBuf = CommandQueue->privateBufList; privateBuf != gcvNULL; )
             {
                 tmpPrivateBuf = privateBuf->next;

                 if (privateBuf->buffer)
                 {
                     gcoCL_FreeMemory(privateBuf->buffer->physical,
                                      privateBuf->buffer->logical,
                                      privateBuf->buffer->allocatedSize,
                                      privateBuf->buffer->node);
                     gcoOS_Free(gcvNULL, privateBuf->buffer);
                 }

                 gcoOS_Free(gcvNULL, privateBuf);

                 privateBuf = tmpPrivateBuf;
             }

             CommandQueue->privateBufList = gcvNULL;
        }

        /* Lock the command queue list in the context. */
        clfLockCommandQueueList(CommandQueue->context);

        /* Remove this from context's queue list. */
        if (CommandQueue->previous)
        {
            CommandQueue->previous->next = CommandQueue->next;
        }
        if (CommandQueue->next)
        {
            CommandQueue->next->previous = CommandQueue->previous;
        }
        if (CommandQueue == CommandQueue->context->queueList)
        {
            CommandQueue->context->queueList = CommandQueue->next;
        }

        /* Unlock the command queue list in the context. */
        clfUnlockCommandQueueList(CommandQueue->context);

        gcmVERIFY_OK(clfDestroyWorkerThread(CommandQueue));

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, CommandQueue->syncPointListMutex));
        CommandQueue->syncPointListMutex = gcvNULL;

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, CommandQueue->commandListMutex));
        CommandQueue->commandListMutex = gcvNULL;

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, CommandQueue->referenceCount));
        CommandQueue->referenceCount = gcvNULL;
        gcoCL_DestroyHW(CommandQueue->hardware);
        /* Release context. */
        clfReleaseContext(CommandQueue->context);

        /* Free context. */
        gcoOS_Free(gcvNULL, CommandQueue);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return gcvSTATUS_OK;

OnError:
    if (status != CL_INVALID_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003005: (clReleaseCommandQueue) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*                       OpenCL Command Queue API                            *|
\*****************************************************************************/
CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(
    cl_context                     Context,
    cl_device_id                   Device,
    cl_command_queue_properties    Properties,
    cl_int *                       ErrcodeRet
    )
{
    clsCommandQueue_PTR            queue=gcvNULL;
    gctPOINTER                     pointer=gcvNULL;
    gctUINT                        i;
    gctINT                         status;

    gcmHEADER_ARG("Context=0x%x Device=0x%x",
                  Context, Device);
    gcmDUMP_API("${OCL clCreateCommandQueue 0x%x, 0x%x}", Context, Device);
    VCL_TRACE_API(CreateCommandQueue_Pre)(Context, Device, Properties, ErrcodeRet);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003000: (clCreateCommandQueue) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Device == gcvNULL || Device->objectType != clvOBJECT_DEVICE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003001: (clCreateCommandQueue) invalid Device.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }

    for (i = 0; i < Context->numDevices; i++)
    {
        if (Context->devices[i] == Device)
        {
            break;
        }
    }

    if (i == Context->numDevices)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003001: (clCreateCommandQueue) invalid Device.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }
    /* Allocate command queue. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsCommandQueue), &pointer), CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, sizeof(clsCommandQueue));

    queue                       = (clsCommandQueue_PTR) pointer;
    queue->dispatch             = Context->dispatch;
    queue->objectType           = clvOBJECT_COMMAND_QUEUE;
    queue->context              = Context;
    queue->device               = Device;
    queue->properties           = Properties;
    queue->numCommands          = 0;
    queue->syncPointList        = gcvNULL;
    queue->commandHead          = gcvNULL;
    queue->commandTail          = gcvNULL;
    queue->deferredReleaseCommandHead   = gcvNULL;
    queue->deferredReleaseCommandTail   = gcvNULL;
    queue->next                 = gcvNULL;
    queue->previous             = gcvNULL;
    queue->nextEnqueueNo        = 0;
    queue->commitRequestList    = gcvNULL;
    queue->privateBufList       = gcvNULL;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &queue->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, queue->referenceCount, gcvNULL));

    /* Create the mutex for the command list. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &queue->commandListMutex),
               CL_OUT_OF_HOST_MEMORY);

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&queue->id), CL_INVALID_VALUE);

    clfRetainContext(Context);

    /* Create the mutex for the sync point list. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &queue->syncPointListMutex),
               CL_OUT_OF_HOST_MEMORY);
    gcmONERROR(gcoCL_CreateHW(queue->device->gpuId, &(queue->hardware)));

    queue->workerStartSignal  = gcvNULL;
    queue->workerStopSignal   = gcvNULL;
    queue->workerThread = gcvNULL;
    queue->inThread = gcvFALSE;

#if VIVANTE_PROFILER
    queue->halProfile            = gcvNULL;

    clfInitializeProfiler(queue);
#endif

    if (!(queue->properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) &&
        (gcoHAL_GetOption(gcvNULL, gcvOPTION_OCL_IN_THREAD)))
    {
        queue->inThread = gcvTRUE;
    }

    /* Create worker thread. */
    clmONERROR(clfConstructWorkerThread(queue), CL_OUT_OF_HOST_MEMORY);

    /* Lock the command queue list in the context. */
    clfLockCommandQueueList(Context);

    /* Insert to context's queue list. */
    queue->next            = Context->queueList;
    Context->queueList     = queue;

    if (queue->next)
    {
        queue->next->previous = queue;
    }

    /* Unlock the command queue list in the context. */
    clfUnlockCommandQueueList(Context);

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateCommandQueue_Post)(Context, Device, Properties, ErrcodeRet, queue);
    gcmFOOTER_ARG("%d queue=0x%x",
                  CL_SUCCESS, queue);

    return queue;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003002: (clCreateCommandQueue) cannot create command queue.  Maybe run out of memory.\n");
    }

    if (queue != gcvNULL)
    {
        if (queue->commandListMutex != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, queue->commandListMutex));
            queue->commandListMutex = gcvNULL;
        }

        gcoOS_Free(gcvNULL, queue);
    }

    gcmFOOTER_ARG("%d", status);

    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(
    cl_command_queue CommandQueue
    )
{
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    gcmDUMP_API("${OCL clRetainCommandQueue 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003003: (clRetainCommandQueue) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    gcmVERIFY_OK(clfRetainCommandQueue(CommandQueue));

    VCL_TRACE_API(RetainCommandQueue)(CommandQueue);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(
    cl_command_queue    CommandQueue
    )
{
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    gcmDUMP_API("${OCL clReleaseCommandQueue 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003004: (clReleaseCommandQueue) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    gcmVERIFY_OK(clfReleaseCommandQueue(CommandQueue));

    VCL_TRACE_API(ReleaseCommandQueue)(CommandQueue);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (status != CL_INVALID_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003005: (clReleaseCommandQueue) internal error.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(
    cl_command_queue      CommandQueue,
    cl_command_queue_info ParamName,
    size_t                ParamValueSize,
    void *                ParamValue,
    size_t *              ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;
    gctINT32         referenceCount;

    gcmHEADER_ARG("CommandQueue=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  CommandQueue, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetCommandQueueInfo 0x%x, 0x%x}", CommandQueue, ParamName);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003006: (clGetCommandQueueInfo) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    switch (ParamName)
    {
    case CL_QUEUE_CONTEXT:
        retParamSize = gcmSIZEOF(CommandQueue->context);
        retParamPtr = &CommandQueue->context;
        break;

    case CL_QUEUE_DEVICE:
        retParamSize = gcmSIZEOF(CommandQueue->device);
        retParamPtr = &CommandQueue->device;
        break;

    case CL_QUEUE_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, CommandQueue->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    case CL_QUEUE_PROPERTIES:
        retParamSize = gcmSIZEOF(CommandQueue->properties);
        retParamPtr = &CommandQueue->properties;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003007: (clGetCommandQueueInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-003008: (clGetCommandQueueInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetCommandQueueInfo)(CommandQueue, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS*/
/*#warning CL_USE_DEPRECATED_OPENCL_1_0_APIS is defined. These APIs are unsupported and untested in OpenCL 1.1!*/
/*
 *  WARNING:
 *     This API introduces mutable state into the OpenCL implementation. It has been REMOVED
 *  to better facilitate thread safety.  The 1.0 API is not thread safe. It is not tested by the
 *  OpenCL 1.1 conformance test, and consequently may not work or may not work dependably.
 *  It is likely to be non-performant. Use of this API is not advised. Use at your own risk.
 *
 *  Software developers previously relying on this API are instructed to set the command queue
 *  properties when creating the queue, instead.
 */
CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(
    cl_command_queue              CommandQueue,
    cl_command_queue_properties   Properties,
    cl_bool                       Enable,
    cl_command_queue_properties * OldProperties
    ) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED
{
    return CL_INVALID_VALUE;
}
/*#endif  CL_USE_DEPRECATED_OPENCL_1_0_APIS */


/* Flush and Finish APIs */
CL_API_ENTRY cl_int CL_API_CALL
clFlush(
    cl_command_queue CommandQueue
    )
{
    gctINT          status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    gcmDUMP_API("${OCL clFlush 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003009: (clFlush) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }
    status = clfFlushCommandQueue(CommandQueue, gcvFALSE);
    if (status != CL_SUCCESS)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003010: (clFlush) internal error.\n");
        /* No error code for internal error. */
        clmRETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    VCL_TRACE_API(Flush)(CommandQueue);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(
    cl_command_queue CommandQueue
    )
{
    gctINT          status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    gcmDUMP_API("${OCL clFinish 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003011: (clFinish) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }
    status = clfFlushCommandQueue(CommandQueue, gcvTRUE);
    if (status != CL_SUCCESS)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003012: (clFinish) internal error.\n");
        /* No error code for internal error. */
        clmRETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

    VCL_TRACE_API(Finish)(CommandQueue);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
