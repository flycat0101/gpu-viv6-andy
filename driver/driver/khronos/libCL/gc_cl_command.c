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
#include <stdio.h>

#define __NEXT_MSG_ID__     003013

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

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
        gctUINT i;

        if (Command->event)
        {
            clReleaseEvent(Command->event);
            Command->event = gcvNULL;
        }

        for(i = 0; i < Command->internalNumEventsInWaitList; i++)
        {
            clReleaseEvent(Command->internalEventWaitList[i]);
        }

        /* Free kernel arguments for NDRangeKernel and Task commands. */
        if (Command->type == clvCOMMAND_NDRANGE_KERNEL)
        {
            clsCommandNDRangeKernel_PTR NDRangeKernel = &Command->u.NDRangeKernel;
            clfFreeKernelArgs(NDRangeKernel->numArgs, NDRangeKernel->args, gcvFALSE);
            clReleaseKernel(NDRangeKernel->kernel);
        }
        else if (Command->type == clvCOMMAND_TASK)
        {
            clsCommandTask_PTR task = &Command->u.task;
            clfFreeKernelArgs(task->numArgs, task->args, gcvFALSE);
            clReleaseKernel(task->kernel);
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

static gctINT
clfAddCommandToCommandQueue(
    clsCommandQueue_PTR     CommandQueue,
    clsCommand_PTR          Command
    )
{
    gctINT                  status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x",
                  CommandQueue, Command);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND,
              CL_INVALID_VALUE);

    /* Lock the command list. */
    clfLockCommandList(CommandQueue);

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

    /* Unlock the command list. */
    clfUnlockCommandList(CommandQueue);

    /* Wake up the worker. */
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
    gcmONERROR(gcoCL_SubmitSignal(Command->releaseSignal,
                                  commandQueue->context->process));

    /* Wake up the queue worker to do the release after the release signal is set. */
    gcmONERROR(gcoCL_SubmitSignal(commandQueue->workerStartSignal,
                                  commandQueue->context->process));

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
#ifdef ANDROID
            /*Special handle, Under Android, print 64bit integer, %ld will print pointer value, %lld can print correct value*/
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
    gctPOINTER data)
{
    gctCHAR* bufPtr = formatString;

    while(*bufPtr) {
        gctCHAR c = *bufPtr++;

        if(c == '%') {
            if(*bufPtr == '%') {
                bufPtr++;
                continue;
            }else {
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
                            gctUINT value = *((gctINT*)data + 1);
                            if (value == 0xFFFFFFFF) /* handle printf("%s", 0);*/
                            {
                                printf(fmt, "(null)");
                            }else{
                                /* the string is stored in const buffer, offset is stored in printf buffer */
                                gctUINT offset = value;
                                printf(fmt, formatString + offset);
                            }
                            data = (gctINT*)data + 2;
                        }else{
                            clfPrintfFmt(fmt, c, &data, vectorSize, argType, flags, fieldWidth, precision);
                        }
                        break;
                    }
                }
            }
        }else {
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
    }
}

static gctINT
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
                clsCommandNDRangeKernel_PTR NDRangeKernel = &command->u.NDRangeKernel;
                fmt = ((gcSHADER)(NDRangeKernel->kernel->states.binary))->constantMemoryBuffer;
                for (i = 0; i < NDRangeKernel->numArgs; i++)
                {
                    if (NDRangeKernel->args[i].isMemAlloc)
                    {
                        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) NDRangeKernel->args[i].data;
                        if (strcmp(NDRangeKernel->args[i].uniform->name, "#printf_address") == 0)
                        {
                            {
                                void* dataAddress = (void*)((gctUINT*)(memAllocInfo->logical) + 1);
                                clfPrintParseData(fmt, dataAddress);
                            }
                        }
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

        if (clmCOMMAND_EXEC_HARDWARE(Command->type))
        {
            clfSubmitEventForRunning(Command);
        }
        else
        {
            clfSetEventExecutionStatus(Command->event, CL_RUNNING);

            clfScheduleEventCallback(Command->event, CL_RUNNING);
        }
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
    while (queue != gcvNULL)
    {
        clfWakeUpCommandQueueWorker(queue);
        queue = queue->next;
    }

    /* Unlock the command queue list in the context. */
    clfUnlockCommandQueueList(Context);
}

static gctINT
clfCreateCommitRequest(
    gctBOOL                 Stall,
    clsCommitRequest_PTR *  CommitRequest
    )
{
    gctINT                  status;
    clsCommitRequest_PTR    commitRequest = gcvNULL;
    gctPOINTER              pointer;

    gcmHEADER_ARG("Stall=%d" , Stall);

    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsCommitRequest), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    commitRequest                   = (clsCommitRequest_PTR) pointer;
    commitRequest->stall            = Stall;
    commitRequest->signal           = gcvNULL;
    commitRequest->previous         = gcvNULL;
    commitRequest->next             = gcvNULL;

    clmONERROR(gcoCL_CreateSignal(gcvFALSE,
                                  &commitRequest->signal),
               CL_OUT_OF_HOST_MEMORY);

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

    gcmHEADER_ARG("CommitRequest=0x%x", CommitRequest);

    clmASSERT(CommitRequest, CL_INVALID_VALUE);

    if (CommitRequest->signal != gcvNULL)
    {
        gcmVERIFY_OK(gcoCL_DestroySignal(CommitRequest->signal));
        CommitRequest->signal = gcvNULL;
    }

    gcmVERIFY_OK(gcoOS_Free(gcvNULL, CommitRequest));

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

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

    /* Acquire synchronization mutex. */
    clfLockCommandList(CommandQueue);

    CommitRequest->nextEnqueueNo = CommandQueue->nextEnqueueNo;

    /* Add the commit request into the list */
    CommitRequest->next = CommandQueue->commitRequestList;

    if (CommitRequest->next != gcvNULL)
    {
        CommitRequest->next->previous = CommitRequest;
    }

    CommandQueue->commitRequestList = CommitRequest;

    /* Release synchronization mutex. */
    clfUnlockCommandList(CommandQueue);

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

                    /* Submit an event command for the signal for clFinish */
                    gcmONERROR(gcoCL_SubmitSignal(commitRequest->signal,
                                                  CommandQueue->context->process));
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

                    /* Wake up the corresponding clFlush thead by signal directly */
                    gcmONERROR(gcoCL_SetSignal(commitRequest->signal));
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

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    /* Create a new commit request */
    gcmONERROR(clfCreateCommitRequest(Stall, &commitRequest));

    /* Add the commit request to the list */
    gcmONERROR(clfAddCommitRequestToList(CommandQueue, commitRequest));

    /* Wait for the commit is done */
    gcmVERIFY_OK(gcoCL_WaitSignal(commitRequest->signal, gcvINFINITE));

    /* Delete the used commit request */
    gcmVERIFY_OK(clfDeleteCommitRequest(commitRequest));

#if !cldSYNC_MEMORY
    if (Stall) gcmONERROR(gcoCL_Flush(gcvTRUE));
#endif

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (commitRequest)
    {
        gcmVERIFY_OK(clfDeleteCommitRequest(commitRequest));
    }

    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

#if cldFSL_OPT
gctINT
clfAddCommandDependency(
    clsCommandQueue_PTR         CommandQueue,
    clsCommand_PTR              Command
    )
{
    gctUINT          i;
    gctINT          status  = CL_SUCCESS;
    gctBOOL         acquire = gcvFALSE;

    gcmHEADER_ARG("Command=0x%x", Command);

    /* only process IN ORDER COMMAND_QUEUE */
    if (CommandQueue->properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
    {
        gcmFOOTER_NO();
        return CL_SUCCESS;
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                    CommandQueue->context->addDependencyMutex,
                                    gcvINFINITE));
    acquire = gcvTRUE;

    if (clmCOMMAND_EXEC_HARDWARE(Command->type))
    {
        clsArgument_PTR args    = gcvNULL;
        gctUINT         numArgs = 0;

        if (clvCOMMAND_NDRANGE_KERNEL == Command->type)
        {
            args    = Command->u.NDRangeKernel.args;
            numArgs = Command->u.NDRangeKernel.numArgs;
        }
        else if (clvCOMMAND_TASK == Command->type)
        {
            args    = Command->u.task.args;
            numArgs = Command->u.task.numArgs;
        }

        for(i=0; i < numArgs; i++)
        {
            if (args[i].uniform && !isUniformInactive(args[i].uniform) && isUniformKernelArg(args[i].uniform))
            {
                gctBOOL isPointer;

                clmONERROR(gcUNIFORM_GetFormat(args[i].uniform, gcvNULL, &isPointer),
                  CL_INVALID_VALUE);

                if (isPointer)
                {
                    clsMem_PTR memObj = *(clsMem_PTR *) args[i].data;

                    if (memObj == gcvNULL) continue;

                    /* header node */
                    if (!memObj->waitEvent.commandQueueID)
                    {
                        gcmASSERT(!memObj->waitEvent.event);
                        gcmASSERT(!memObj->waitEvent.next);

                        memObj->waitEvent.commandQueueID = CommandQueue;
                        memObj->waitEvent.event = Command->event;
                        if (memObj->waitEvent.event) clRetainEvent(memObj->waitEvent.event);
                    }
                    else
                    {
                        clsBufferWaitEvent_PTR  tempWaitEvent    = gcvNULL;
                        for(tempWaitEvent = &memObj->waitEvent; tempWaitEvent; tempWaitEvent = tempWaitEvent->next)
                        {
                            /* find the slot */
                            if (tempWaitEvent->commandQueueID == CommandQueue)
                            {
                                if (tempWaitEvent->event) clReleaseEvent(tempWaitEvent->event);
                                tempWaitEvent->event = Command->event;
                                if (tempWaitEvent->event) clRetainEvent(tempWaitEvent->event);
                                break;
                            }
                        }

                        /* can not find, create a new one */
                        if (!tempWaitEvent)
                        {
                            clsBufferWaitEvent_PTR pointer;
                            clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsBufferWaitEvent), (gctPOINTER*)&pointer), CL_OUT_OF_HOST_MEMORY);
                            gcoOS_ZeroMemory((clsBufferWaitEvent_PTR)pointer, sizeof(clsBufferWaitEvent));
                            pointer->next = memObj->waitEvent.next;
                            memObj->waitEvent.next = pointer;

                            pointer->commandQueueID = CommandQueue;
                            pointer->event = Command->event;
                            if (pointer->event) clRetainEvent(pointer->event);
                        }
                    }
                }
            }
        }
    }

    if (clmCOMMAND_BUFFER_OPERATION(Command->type))
    {
        clsMem_PTR memObj[2] = {gcvNULL};

        Command->internalNumEventsInWaitList = 0;

        switch (Command->type)
        {
        case clvCOMMAND_READ_BUFFER:
            memObj[0] = Command->u.readBuffer.buffer;
            break;
        case clvCOMMAND_READ_BUFFER_RECT:
            memObj[0] = Command->u.readBufferRect.buffer;
            break;
        case clvCOMMAND_WRITE_BUFFER:
            memObj[0] = Command->u.writeBuffer.buffer;
            break;
#if BUILD_OPENCL_12
        case clvCOMMAND_FILL_BUFFER:
            memObj[0] = Command->u.fillBuffer.buffer;
            break;
#endif
        case clvCOMMAND_WRITE_BUFFER_RECT:
            memObj[0] = Command->u.writeBufferRect.buffer;
            break;
        case clvCOMMAND_COPY_BUFFER:
            memObj[0] = Command->u.copyBuffer.srcBuffer;
            memObj[1] = Command->u.copyBuffer.dstBuffer;
            break;
        case clvCOMMAND_COPY_BUFFER_RECT:
            memObj[0] = Command->u.copyBufferRect.srcBuffer;
            memObj[1] = Command->u.copyBufferRect.dstBuffer;
            break;
        case clvCOMMAND_READ_IMAGE:
            memObj[0] = Command->u.readImage.image;
            break;
        case clvCOMMAND_WRITE_IMAGE:
            memObj[0] = Command->u.writeImage.image;
            break;
#if BUILD_OPENCL_12
        case clvCOMMAND_FILL_IMAGE:
            memObj[0] = Command->u.fillImage.image;
            break;
#endif
        case clvCOMMAND_COPY_IMAGE:
            memObj[0] = Command->u.copyImage.srcImage;
            memObj[1] = Command->u.copyImage.dstImage;
            break;
        case clvCOMMAND_COPY_IMAGE_TO_BUFFER:
            memObj[0] = Command->u.copyImageToBuffer.srcImage;
            memObj[1] = Command->u.copyImageToBuffer.dstBuffer;
            break;
        case clvCOMMAND_COPY_BUFFER_TO_IMAGE:
            memObj[0] = Command->u.copyBufferToImage.srcBuffer;
            memObj[1] = Command->u.copyBufferToImage.dstImage;
            break;
#if BUILD_OPENCL_12
        case clvCOMMAND_MIGRATE_MEM_OBJECTS:
            break;
#endif
        case clvCOMMAND_MAP_BUFFER:
            memObj[0] = Command->u.mapBuffer.buffer;
            break;
        case clvCOMMAND_MAP_IMAGE:
            memObj[0] = Command->u.mapImage.image;
            break;
        default:
            break;
        }

        for(i = 0; i < 2; i++)
        {
            clsBufferWaitEvent_PTR  tempWaitEvent = memObj[i] ? &memObj[i]->waitEvent : gcvNULL;
            for(; tempWaitEvent; tempWaitEvent = tempWaitEvent->next)
            {
                if (tempWaitEvent->commandQueueID == CommandQueue)
                {
                    Command->internalEventWaitList[Command->internalNumEventsInWaitList] = tempWaitEvent->event;
                    if(tempWaitEvent->event) clRetainEvent(tempWaitEvent->event);
                    Command->internalNumEventsInWaitList++;
                    break;
                }
            }
        }

        gcmASSERT(Command->internalNumEventsInWaitList <= 2);

    }

OnError:
    if (acquire)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                    CommandQueue->context->addDependencyMutex));
    }
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

gctINT
clfSubmitCommand(
    clsCommandQueue_PTR         CommandQueue,
    clsCommand_PTR              Command,
    gctBOOL                     Blocking
    )
{
    cl_event        commandEvent = gcvNULL;
    gctINT          status;

    gcmHEADER_ARG("CommandQueue=0x%x Command=0x%x Blocking=%d",
                  CommandQueue, Command, Blocking);

    clmASSERT(CommandQueue && CommandQueue->objectType == clvOBJECT_COMMAND_QUEUE,
              CL_INVALID_COMMAND_QUEUE);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND,
              CL_INVALID_VALUE);
#if cldFSL_OPT
    if (Command->outEvent ||
        Blocking ||
        (((CommandQueue->properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) == 0) &&
             clmCOMMAND_EXEC_HARDWARE(Command->type)))
#else
    if (Command->outEvent ||
        Blocking)
#endif
    {
        /* Create event to track status of this command */
        clmONERROR(clfAllocateEvent(CommandQueue->context, &commandEvent),
                   CL_OUT_OF_HOST_MEMORY);

        commandEvent->commandType = clmGET_COMMANDTYPE(Command->type);
        commandEvent->queue = CommandQueue;

#if cldSYNC_MEMORY
        clRetainCommandQueue(CommandQueue);
#endif

        if (Blocking) clRetainEvent(commandEvent);

        clfSetEventExecutionStatus(commandEvent, CL_QUEUED);

        Command->event = commandEvent;
    }

    if (Command->outEvent)
    {
        /* The host must release this event once */
        clRetainEvent(commandEvent);

        *Command->outEvent = Command->event;
    }
#if cldFSL_OPT
    /* establish the buffer dependency */
    clfAddCommandDependency(CommandQueue, Command);
#endif
    clfAddCommandToCommandQueue(CommandQueue, Command);

    if (Blocking)
    {
        /* Wait for the command to finish. */
        clmASSERT(commandEvent, CL_INVALID_VALUE);
        clfWaitForEvent(commandEvent);
        clReleaseEvent(commandEvent);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctTHREAD_RETURN CL_API_CALL
clfCommandQueueWorker(
    gctPOINTER Data
    )
{
    clsCommandQueue_PTR commandQueue = (clsCommandQueue_PTR) Data;
    clsCommand_PTR      command = gcvNULL;
    gceSTATUS           status;
    gctBOOL             acquired = gcvFALSE;

    /* Use the same hardware for command queue worker. */

    gcmONERROR(gcoCL_SelectDevice(commandQueue->device->gpuId));
    gcmONERROR(gcoCL_SetHardware());

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

        clfProcessDeferredReleaseCommandList(commandQueue);

        /* Lock the command list in this command queue. */
        clfLockCommandList(commandQueue);
        acquired = gcvTRUE;

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

OnError:
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

    status = clfRemoveSyncPoint(commandQueue, Command);

OnError:
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

    clRetainContext(Context);

    /* Create the mutex for the sync point list. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &queue->syncPointListMutex),
               CL_OUT_OF_HOST_MEMORY);

#if cldSEQUENTIAL_EXECUTION
    queue->workerStartSignal  = gcvNULL;
    queue->workerStopSignal   = gcvNULL;
    queue->workerThread = gcvNULL;
#else
    /* Create worker thread. */

    /* Create thread start signal. */
    clmONERROR(gcoCL_CreateSignal(gcvFALSE,
                                  &queue->workerStartSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Create thread stop signal. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &queue->workerStopSignal),
               CL_OUT_OF_HOST_MEMORY);

    /* Start the worker thread. */
    clmONERROR(gcoOS_CreateThread(gcvNULL,
                                  clfCommandQueueWorker,
                                  queue,
                                  &queue->workerThread),
               CL_OUT_OF_HOST_MEMORY);
#endif

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

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003003: (clRetainCommandQueue) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, CommandQueue->referenceCount, gcvNULL));

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
    gctINT32            oldReference;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-003004: (clReleaseCommandQueue) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, CommandQueue->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        /* Implicit flush. */
        gcmONERROR(clfFlushCommandQueue(CommandQueue, gcvFALSE));

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

#if !cldSEQUENTIAL_EXECUTION
        /* Send signal to stop worker thread. */
        gcmONERROR(gcoCL_SetSignal(CommandQueue->workerStopSignal));

        gcmONERROR(gcoCL_SetSignal(CommandQueue->workerStartSignal));

        gcmONERROR(gcoCL_Flush(gcvTRUE));

        /* Wait until the thread is closed. */
        gcoOS_CloseThread(gcvNULL, CommandQueue->workerThread);
        CommandQueue->workerThread = gcvNULL;

        /* Free signals and mutex. */
        gcmVERIFY_OK(gcoCL_DestroySignal(CommandQueue->workerStartSignal));
        CommandQueue->workerStartSignal = gcvNULL;

        gcmVERIFY_OK(gcoCL_DestroySignal(CommandQueue->workerStopSignal));
        CommandQueue->workerStopSignal = gcvNULL;
#endif

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, CommandQueue->syncPointListMutex));
        CommandQueue->syncPointListMutex = gcvNULL;

        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, CommandQueue->commandListMutex));
        CommandQueue->commandListMutex = gcvNULL;

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, CommandQueue->referenceCount));
        CommandQueue->referenceCount = gcvNULL;

        /* Release context. */
        clReleaseContext(CommandQueue->context);

        /* Free context. */
        gcoOS_Free(gcvNULL, CommandQueue);
    }

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

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
