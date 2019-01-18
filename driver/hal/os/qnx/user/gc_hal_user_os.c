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


/**
**  @file
**  OS object for hal user layers.
**
*/

#include "gc_hal_user_qnx.h"
#include "gc_hal_user_os_atomic.h"
#include "gc_hal_dump.h"
#if gcdUSE_FAST_MEM_COPY
#include <fastmemcpy.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/procfs.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/neutrino.h>
#include <sys/cache.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <sys/resmgr.h>
#include <KHR/khronos_utils.h>
#include <atomic.h>

#include <signal.h>
#include <errno.h>
#include <libgen.h>

#define _GC_OBJ_ZONE    gcvZONE_OS

#ifndef GAL_DEV
#define GAL_DEV "/dev/galcore"
#endif

/*******************************************************************************
***** Version Signature *******************************************************/

const char * _GAL_PLATFORM = "\n\0$PLATFORM$QNX$\n";

/******************************************************************************\
***************************** gcoOS Object Structure ***************************
\******************************************************************************/

typedef struct _gcsSIGNAL *         gcsSIGNAL_PTR;
typedef struct _gcsSIGNAL_RECORD *  gcsSIGNAL_RECORD_PTR;

typedef struct _gcsSIGNAL_RECORD
{
    /* Pointer to next signal. */
    gcsSIGNAL_RECORD_PTR next;

    /* Signal. */
    gcsSIGNAL_PTR signal;
}
gcsSIGNAL_RECORD;

typedef struct _gcsDRIVER_ARGS
{
    io_msg_t iomsg;
    gcsHAL_INTERFACE iface;
}
gcsDRIVER_ARGS;

struct _gcoOS
{
    /* Object. */
    gcsOBJECT               object;

    /* Context. */
    gctPOINTER              context;

    /* Heap. */
    gcoHEAP                 heap;

#if gcdENABLE_PROFILING
    gctUINT64               startTick;
#endif

    /* Handle to the device. */
    int                     device;

    /* Shared memory. */
    gctUINT32               mempoolBaseAddress;
    gctUINT32               mempoolPageSize;

    /* Signal. */
    gctINT                  pulseChid;
    gctINT                  pulseCoid;
    pthread_t               pulseThread;
};

/******************************************************************************\
*********************************** Globals ************************************
\******************************************************************************/

static pthread_key_t gcProcessKey;

gcsPLS gcPLS = gcPLS_INITIALIZER;

/*
 * Definition of the linked list and lock for the shared memory management.
 */
typedef struct _gcsSharedMemoryNode * gcsSharedMemoryNode_PTR;
typedef struct _gcsSharedMemoryNode
{
    gctPOINTER                          Memory;
    gctSIZE_T                           Bytes;
    gctUINT32                           Physical;
    LIST_ENTRY(_gcsSharedMemoryNode)    node;
}
gcsSharedMemoryNode;

static LIST_HEAD(_gcsSharedMemoryList, _gcsSharedMemoryNode) \
    s_sharedMemoryList, s_sharedMemoryFreeList;

static gctPOINTER s_sharedMemoryListLock;

static gcsSharedMemoryNode s_sharedMemoryFreeListNodes[ \
   (16 << 10) / gcmSIZEOF(gcsSharedMemoryNode) \
   ];

static gceSTATUS
_AddSharedMemoryNode(
    /* [in] */ gcsSharedMemoryNode_PTR Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSharedMemoryNode_PTR pNode = gcvNULL;
    gctPOINTER pointer;
    gctBOOL acquired = gcvFALSE;

    if (Node == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the mutex. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, s_sharedMemoryListLock, gcvINFINITE));
    acquired = gcvTRUE;

    if (LIST_EMPTY(&s_sharedMemoryFreeList))
    {
        /* Allocate the node structure. */
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(*pNode), &pointer));

        pNode = pointer;
    }
    else
    {
        /* Get a node from the free list. */
        pNode = LIST_FIRST(&s_sharedMemoryFreeList);
        LIST_REMOVE(pNode, node);
    }

    /* Copy the data. */
    gcoOS_MemCopy(pNode, Node, gcmSIZEOF(*pNode));

    /* Add the new node to the list. */
    LIST_INSERT_HEAD(&s_sharedMemoryList, pNode, node);

    /* Release the mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, s_sharedMemoryListLock));
    acquired = gcvFALSE;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    if (acquired)
    {
        gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, s_sharedMemoryListLock));
    }

    return status;
}

static gceSTATUS
_DetachSharedMemoryNode(
    /* [in] */ gctPOINTER Memory,
    /* [out] */ gcsSharedMemoryNode_PTR Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL acquired = gcvFALSE;
    gcsSharedMemoryNode_PTR pNode;

    if ((Memory == gcvNULL) || (Node == gcvNULL))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the mutex. */
    gcmONERROR(gcoOS_AcquireMutex(gcvNULL, s_sharedMemoryListLock, gcvINFINITE));
    acquired = gcvTRUE;

    /* Iterate over the linked list. */
    LIST_FOREACH(pNode, &s_sharedMemoryList, node)
    {
        if (pNode->Memory == Memory)
        {
            goto Found;
        }
    }

    /* Not found. */
    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);

Found:
    /* Delete from the list. */
    LIST_REMOVE(pNode, node);

    /* Copy back the data. */
    gcoOS_MemCopy(Node, pNode, gcmSIZEOF(*pNode));

    /* Add to the free list. */
    if ((pNode >= s_sharedMemoryFreeListNodes) &&
        (pNode < &s_sharedMemoryFreeListNodes[gcmCOUNTOF(s_sharedMemoryFreeListNodes)]))
    {
        LIST_INSERT_HEAD(&s_sharedMemoryFreeList, pNode, node);
        pNode = gcvNULL;
    }

    /* Release the mutex. */
    gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, s_sharedMemoryListLock));
    acquired = gcvFALSE;

    /* Free the node if from user heap. */
    if (pNode != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, pNode));
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    if (acquired)
    {
        gcmONERROR(gcoOS_ReleaseMutex(gcvNULL, s_sharedMemoryListLock));
    }

    return status;
}

/*
 * User signal management.
 */

gceSTATUS
gcoOS_SignalPending(
    IN gcoOS Os,
    IN gcsSIGNAL_PTR Signal
    );

#define gcdQNX_SIGNAL_CHECK 0

#if gcdQNX_SIGNAL_CHECK
static pthread_mutex_t s_signalListMutex = PTHREAD_MUTEX_INITIALIZER;
static LIST_HEAD(_gcsSIGNALList, _gcsSIGNAL) s_signalList = LIST_HEAD_INITIALIZER(s_signalList);

static gctBOOL _CheckSignal(
    IN gcsSIGNAL_PTR Signal
    )
{
    gcsSIGNAL_PTR p;

    pthread_mutex_lock(&s_signalListMutex);

    LIST_FOREACH(p, &s_signalList, node)
    {
        if ((p == Signal) && (p->signature == gcdQNX_SIGNAL_SIGNATURE))
        {
            pthread_mutex_unlock(&s_signalListMutex);

            return gcvTRUE;
        }
    }

    pthread_mutex_unlock(&s_signalListMutex);

    return gcvFALSE;
}
#endif

static gceSTATUS
_DestroySignal(
    IN gcoOS Os,
    IN gcsSIGNAL_PTR Signal
    )
{
    gcmHEADER_ARG("Signal=0x%x", Signal);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Signal != gcvNULL);

#if gcdQNX_SIGNAL_CHECK
    /* Acquire the list mutex. */
    pthread_mutex_lock(&s_signalListMutex);

    /* Remove from the list. */
    LIST_REMOVE(Signal, node);

    /* Release the list mutex. */
    pthread_mutex_unlock(&s_signalListMutex);
#endif

    /* Destroy the condition variable. */
    pthread_cond_destroy(&Signal->condition);

    /* Destroy the mutex. */
    pthread_mutex_destroy(&Signal->mutex);

    /* Clear the signature. */
    Signal->signature = 0;

    /* Free the signal structure. */
    gcmVERIFY_OK(gcoOS_Free(Os, Signal));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Signal thread. */
static void *
_PulseThread(
    void * Argument
    )
{
    gcoOS os = (gcoOS) Argument;
    gctINT rc;
    struct _pulse pulse;
    gctBOOL destroy;

    pthread_setname_np(0, "gal-os_Signal-waiter");

    /* Loop forever. */
    while (1)
    {
        rc = MsgReceivePulse_r(os->pulseChid, &pulse, sizeof(pulse), gcvNULL);

        if (rc == (-1 * EINTR))
        {
            continue;
        }

        if (rc == (-1 * ESRCH))
        {
            break;
        }

        if (rc != EOK)
        {
            continue;
        }

        if (pulse.code == GC_HAL_QNX_PULSEVAL_SIGNAL)
        {
            gcsSIGNAL_PTR signal = (gcsSIGNAL_PTR)pulse.value.sival_ptr;

#if gcdQNX_SIGNAL_CHECK
            /* Validate the signal. */
            if (!_CheckSignal(signal))
            {
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "%s(%d): Invalid Signal=0x%x.\n",
                        __FUNCTION__, __LINE__,
                        (gctUINT32)signal);

                continue;
            }
#endif

            /* Check the signature. */
            if (signal->signature != gcdQNX_SIGNAL_SIGNATURE)
            {
                gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                        "%s(%d): Invalid Signal=0x%x signature=0x%x.\n",
                        __FUNCTION__, __LINE__,
                        (gctUINT32)signal,
                        signal->signature);

                continue;
            }

            gcmVERIFY_OK(
                gcoOS_Signal(os, signal, gcvTRUE));

            pthread_mutex_lock(&signal->mutex);

            signal->received++;

            destroy = (signal->destroyed && (signal->pending == signal->received));

            if (destroy)
            {
                _DestroySignal(os, signal);
            }
            else
            {
                pthread_mutex_unlock(&signal->mutex);
            }
        }
    }

    return gcvNULL;
}

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
static void _ReportDB(
    void
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcoOS_ZeroMemory(&iface, sizeof(iface));

    iface.command = gcvHAL_DATABASE;
    iface.u.Database.processID = (gctUINT32) gcoOS_GetCurrentProcessID();
    iface.u.Database.validProcessID = gcvTRUE;

    /* Call kernel service. */
    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        ));

    if ((iface.u.Database.vidMem.counters.bytes     != 0) ||
        (iface.u.Database.nonPaged.counters.bytes   != 0))
    {
        gcmTRACE(gcvLEVEL_ERROR, "\n");
        gcmTRACE(gcvLEVEL_ERROR, "******* MEMORY LEAKS DETECTED *******\n");
    }

    if (iface.u.Database.vidMem.counters.bytes != 0)
    {
        gcmTRACE(gcvLEVEL_ERROR, "\n");
        gcmTRACE(gcvLEVEL_ERROR, "vidMem.bytes      = %d\n", iface.u.Database.vidMem.counters.bytes);
        gcmTRACE(gcvLEVEL_ERROR, "vidMem.maxBytes   = %d\n", iface.u.Database.vidMem.counters.maxBytes);
        gcmTRACE(gcvLEVEL_ERROR, "vidMem.totalBytes = %d\n", iface.u.Database.vidMem.counters.totalBytes);
    }

    if (iface.u.Database.nonPaged.counters.bytes != 0)
    {
        gcmTRACE(gcvLEVEL_ERROR, "\n");
        gcmTRACE(gcvLEVEL_ERROR, "nonPaged.bytes      = %d\n", iface.u.Database.nonPaged.counters.bytes);
        gcmTRACE(gcvLEVEL_ERROR, "nonPaged.maxBytes   = %d\n", iface.u.Database.nonPaged.counters.maxBytes);
        gcmTRACE(gcvLEVEL_ERROR, "nonPaged.totalBytes = %d\n", iface.u.Database.nonPaged.counters.totalBytes);
    }

OnError:;
}
#endif

#if gcdDUMP || gcdDUMP_API || gcdDUMP_2D
static void
_SetDumpFileInfo(
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;

#if gcdDUMP || gcdDUMP_2D
    #define DUMP_FILE_PREFIX   "hal"
#else
    #define DUMP_FILE_PREFIX   "api"
#endif

/*
    #define DEFAULT_DUMP_KEY        "allprocesses"

    if (gcmIS_SUCCESS(gcoOS_StrCmp(gcdDUMP_KEY, DEFAULT_DUMP_KEY)))
        status = gcvSTATUS_TRUE;
    else
        status = gcoOS_DetectProcessByName(gcdDUMP_KEY);
*/

    if (status == gcvSTATUS_TRUE)
    {
#if !gcdDUMP_IN_KERNEL
        char dump_file[128];
        gctUINT offset = 0;

        /* Customize filename as needed. */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(dump_file,
                     gcmSIZEOF(dump_file),
                     &offset,
                     "%s%s_dump_pid-%d_tid-%d_%s.log",
                     gcdDUMP_PATH,
                     DUMP_FILE_PREFIX,
                     gcoOS_GetCurrentProcessID(),
                     (gctUINT32)gcoOS_GetCurrentThreadID(),
                     gcdDUMP_KEY));

        gcoOS_SetDebugFile(dump_file);
#endif
        gcoOS_SetDumpFlag(gcvTRUE);
    }
}
#endif

/******************************************************************************\
**************************** OS Construct/Destroy ******************************
\******************************************************************************/
static gceSTATUS
_DestroyOs(
    IN gcoOS Os
    )
{
    gceSTATUS status;

    gcmPROFILE_DECLARE_ONLY(gctUINT64 ticks);

    gcmHEADER();

    if (gcPLS.os != gcvNULL)
    {
        gcmPROFILE_QUERY(gcPLS.os->startTick, ticks);
        gcmPROFILE_ONLY(gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                                      "Total ticks during gcoOS life: %llu",
                                      ticks));

        /* Kill the signal thread. */
        if (gcPLS.os->pulseThread != 0)
        {
            ConnectDetach_r(gcPLS.os->pulseCoid);
            ChannelDestroy_r(gcPLS.os->pulseChid);

            gcPLS.os->pulseCoid = -1;
            gcPLS.os->pulseChid = -1;

            pthread_join(gcPLS.os->pulseThread, gcvNULL);
        }

        if (gcPLS.os->heap != gcvNULL)
        {
            gcoHEAP heap = gcPLS.os->heap;

#if VIVANTE_PROFILER_SYSTEM_MEMORY
            /* End profiler. */
            gcoHEAP_ProfileEnd(heap, "gcoOS_HEAP");
#endif

            /* Mark the heap as gone. */
            gcPLS.os->heap = gcvNULL;

            /* Destroy the heap. */
            gcmONERROR(gcoHEAP_Destroy(heap));
        }

        /* Close the handle to the kernel service. */
        if (gcPLS.os->device != -1)
        {
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            _ReportDB();
#endif

            close(gcPLS.os->device);
            gcPLS.os->device = -1;
        }

        /* Mark the gcoOS object as unknown. */
        gcPLS.os->object.type = gcvOBJ_UNKNOWN;

        /* Free the gcoOS structure. */
        free(gcPLS.os);

        /* Reset PLS object. */
        gcPLS.os = gcvNULL;
    }

    /* Success. */
    gcmFOOTER_KILL();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_ConstructOs(
    IN gctPOINTER Context,
    OUT gcoOS *Os
    )
{
    gcoOS os = gcPLS.os;
    gceSTATUS status;

    gcmPROFILE_DECLARE_ONLY(gctUINT64 freq);

    gcmHEADER_ARG("Context=0x%x", Context);

    if (os == gcvNULL)
    {
        /* Allocate the gcoOS structure. */
        os = malloc(gcmSIZEOF(struct _gcoOS));
        if (os == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        /* Initialize the gcoOS object. */
        os->object.type = gcvOBJ_OS;
        os->context     = Context;
        os->heap        = gcvNULL;
        os->device      = -1;

        /* Set the object pointer to PLS. */
        gcmASSERT(gcPLS.os == gcvNULL);
        gcPLS.os = os;

        /* Attempt to open the device. */
        os->device = open(GAL_DEV, O_RDWR);
        if (os->device < 0)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): open failed.",
                     __FUNCTION__, __LINE__);

            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        os->mempoolPageSize = 1 << 12;

        /* Construct heap. */
        status = gcoHEAP_Construct(gcvNULL, gcdHEAP_SIZE, &os->heap);

        if (gcmIS_ERROR(status))
        {
            gcmTRACE_ZONE(
                gcvLEVEL_WARNING, gcvZONE_OS,
                "%s(%d): Could not construct gcoHEAP (%d).",
                __FUNCTION__, __LINE__, status
                );

            os->heap = gcvNULL;
        }
#if VIVANTE_PROFILER_SYSTEM_MEMORY
        else
        {
            /* Start profiler. */
            gcoHEAP_ProfileStart(os->heap);
        }
#endif

        /* Create pulse side channel. */
        os->pulseChid = ChannelCreate(0);
        if (os->pulseChid == -1)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): Pulse channel create failed (%d).",
                 __FUNCTION__, __LINE__, errno);

            goto OnError;
        }

        /* Create connection to channel. */
        os->pulseCoid = ConnectAttach(0, 0, os->pulseChid, _NTO_SIDE_CHANNEL, 0);
        if (os->pulseCoid == -1)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                     "%s(%d): Pulse connection attach failed (%d).",
                     __FUNCTION__, __LINE__, errno);

            goto OnError;
        }

        /* Create pulse thread. */
        if (pthread_create(&os->pulseThread, gcvNULL, _PulseThread, os))
        {
            gcmTRACE(gcvLEVEL_ERROR,
                 "%s(%d): pthread_create failed.",
                     __FUNCTION__, __LINE__);

            goto OnError;
        }

        /* Get profiler start tick. */
        gcmPROFILE_INIT(freq, os->startTick);
    }

    /* Return pointer to the gcoOS object. */
    if (Os != gcvNULL)
    {
        *Os = os;
    }

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                  "Successfully opened %s devId->%d",
                  gcvHAL_CLASS,
                  os->device);

    /* Success. */
    gcmFOOTER_ARG("*Os=0x%x", os);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    gcmVERIFY_OK(_DestroyOs(gcvNULL));
    gcmFOOTER();
    return status;
}

/******************************************************************************\
************************* Process/Thread Local Storage *************************
\******************************************************************************/

static void __attribute__((constructor)) _ModuleConstructor(void);
#if (_NTO_VERSION >= 700)
static void _ModuleDestructor(void);
#else
static void _ModuleDestructor(void *);
#endif

static gceSTATUS
_CacheRangeFlush(
    IN gctUINT32 Node,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes,
    IN gceCACHEOPERATION Operation
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u Operation=%d",
                  Node, Logical, Bytes, Operation);

    if (!(Operation & gcvCACHE_FLUSH))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Operation & gcvCACHE_INVALIDATE)
    {
        msync(Logical, Bytes, MS_SYNC | MS_CACHE_ONLY | MS_INVALIDATE);
    }
    else if (Operation & gcvCACHE_CLEAN)
    {
        msync(Logical, Bytes, MS_SYNC | MS_CACHE_ONLY);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static void
_PLSDestructor(
    void
    )
{
    gcmHEADER();

    if (gcPLS.destructor != gcvNULL)
    {
        gcPLS.destructor(&gcPLS);
        gcPLS.destructor = gcvNULL;
    }

#if gcdDUMP_2D
    gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, dumpMemInfoListMutex));
    dumpMemInfoListMutex = gcvNULL;
#endif

    gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.accessLock));
    gcPLS.accessLock = gcvNULL;

    gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.glFECompilerAccessLock));
    gcPLS.glFECompilerAccessLock = gcvNULL;

    gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.clFECompilerAccessLock));
    gcPLS.clFECompilerAccessLock = gcvNULL;

    gcmVERIFY_OK(gcoOS_AtomDestroy(gcPLS.os, gcPLS.reference));
    gcPLS.reference = gcvNULL;

    if (gcPLS.hal != gcvNULL)
    {
        gcmVERIFY_OK(gcoHAL_DestroyEx(gcPLS.hal));
        gcPLS.hal = gcvNULL;
    }

    gcmVERIFY_OK(_DestroyOs(gcPLS.os));

    pthread_key_delete(gcProcessKey);

    gcmFOOTER_NO();
}

static void
_TLSDestructor(
    gctPOINTER TLS
    )
{
    gcsTLS_PTR tls;
    gctINT reference = 0;
    gctINT i;

    gcmHEADER_ARG("TLS=0x%x", TLS);

    tls = (gcsTLS_PTR) TLS;
    gcmASSERT(tls != gcvNULL);

    pthread_setspecific(gcProcessKey, tls);

    if (tls->copied)
    {
        /* Zero out all information if this TLS was copied. */
        gcoOS_ZeroMemory(tls, gcmSIZEOF(gcsTLS));
    }

    for (i = 0; i < gcvTLS_KEY_COUNT; i++)
    {
        gcsDRIVER_TLS_PTR drvTLS = tls->driverTLS[i];

        if (drvTLS && drvTLS->destructor != gcvNULL)
        {
            drvTLS->destructor(drvTLS);
        }

        tls->driverTLS[i] = gcvNULL;
    }

#if gcdENABLE_3D
    /* DON'T destroy tls->engine3D, which belongs to app context
    */
#endif

    if(tls->engineVX)
    {
        gcmVERIFY_OK(gcoVX_Destroy(tls->engineVX));
    }

    if (tls->defaultHardware != gcvNULL)
    {
        gceHARDWARE_TYPE type = tls->currentType;

        tls->currentType = gcvHARDWARE_3D;

        gcmTRACE_ZONE(
            gcvLEVEL_VERBOSE, gcvZONE_HARDWARE,
            "%s(%d): destroying default hardware object 0x%08X.",
            __FUNCTION__, __LINE__, tls->defaultHardware
            );

        gcmVERIFY_OK(gcoHARDWARE_Destroy(tls->defaultHardware, gcvTRUE));
        tls->defaultHardware = gcvNULL;
        tls->currentHardware = gcvNULL;
        tls->currentType = type;
    }

#if gcdENABLE_2D
    if (tls->engine2D != gcvNULL)
    {
        gcmVERIFY_OK(gco2D_Destroy(tls->engine2D));
        tls->engine2D = gcvNULL;
    }

    if (tls->hardware2D != gcvNULL)
    {
        gceHARDWARE_TYPE type = tls->currentType;
        tls->currentType = gcvHARDWARE_2D;

        gcmTRACE_ZONE(
            gcvLEVEL_VERBOSE, gcvZONE_HARDWARE,
            "%s(%d): destroying hardware object 0x%08X.",
            __FUNCTION__, __LINE__, tls->hardware2D
            );

        gcmVERIFY_OK(gcoHARDWARE_Destroy(tls->hardware2D, gcvTRUE));
        tls->hardware2D = gcvNULL;
        tls->currentType = type;
    }
#endif

#if gcdENABLE_VG
    if (tls->engineVG != gcvNULL)
    {
#if gcdGC355_PROFILER
        gcmVERIFY_OK(gcoVG_Destroy(tls->engineVG, 0, 0, 0));
#else
        gcmVERIFY_OK(gcoVG_Destroy(tls->engineVG));
#endif
        tls->engineVG = gcvNULL;
    }

    if (tls->vg != gcvNULL)
    {
        gceHARDWARE_TYPE type = tls->currentType;
        tls->currentType = gcvHARDWARE_VG;

        gcmTRACE_ZONE(
            gcvLEVEL_VERBOSE, gcvZONE_HARDWARE,
            "%s(%d): destroying hardware object 0x%08X.",
            __FUNCTION__, __LINE__, tls->vg
            );

        gcmVERIFY_OK(gcoVGHARDWARE_Destroy(tls->vg));
        tls->vg = gcvNULL;
        tls->currentType = type;
    }
#endif

    gcmVERIFY_OK(gcoOS_FreeMemory(gcvNULL, tls));

    pthread_setspecific(gcProcessKey, gcvNULL);

    if (gcPLS.reference != gcvNULL)
    {
        /* Decrement the reference. */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcPLS.os,
                                         gcPLS.reference,
                                         &reference));

        /* Check if there are still more references. */
        if (reference == 1)
        {
            /* If all threads exit, destruct PLS */
            _PLSDestructor();
        }
    }

    gcmFOOTER_NO();
}

static void
_InitializeProcess(
    void
    )
{
    /* Install thread destructor. */
    pthread_key_create(&gcProcessKey, _TLSDestructor);
}

static void _ModuleConstructor(
    void
    )
{
    gceSTATUS status;
    int i, result;
    static pthread_once_t onceControl = PTHREAD_ONCE_INIT;

    gcmHEADER();

    /* Each process gets its own objects. */
    gcmASSERT(gcPLS.os  == gcvNULL);
    gcmASSERT(gcPLS.hal == gcvNULL);

    gcmASSERT(gcPLS.internalLogical   == gcvNULL);
    gcmASSERT(gcPLS.externalLogical   == gcvNULL);
    gcmASSERT(gcPLS.contiguousLogical == gcvNULL);

    /* Call _InitializeProcess function only one time for the process. */
    result = pthread_once(&onceControl, _InitializeProcess);

    if (result != 0)
    {
        gcmTRACE(
            gcvLEVEL_ERROR,
            "%s(%d): pthread_once returned %d",
            __FUNCTION__, __LINE__, result
            );

        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* The library is loaded from dlopen().
       dlclose is called for the library so we have to use __cxa_atexit here. */

#if (_NTO_VERSION >= 700)
    atexit((void (*)(void)) _ModuleDestructor);
#else
    __cxa_atexit((void (*)(void*)) _ModuleDestructor, NULL, __dso_handle);
#endif
    /* Initialize the shared memory list. */
    LIST_INIT(&s_sharedMemoryList);
    LIST_INIT(&s_sharedMemoryFreeList);

    for (i = 0; i < gcmCOUNTOF(s_sharedMemoryFreeListNodes); i++)
    {
        LIST_INSERT_HEAD(&s_sharedMemoryFreeList, &s_sharedMemoryFreeListNodes[i], node);
    }

    gcmONERROR(gcoOS_CreateMutex(gcvNULL, &s_sharedMemoryListLock));

    /* Construct OS object. */
    gcmONERROR(_ConstructOs(gcvNULL, gcvNULL));

    /* Construct PLS reference atom. */
    gcmONERROR(gcoOS_AtomConstruct(gcPLS.os, &gcPLS.reference));

    /* Increment PLS reference for main thread. */
    gcmONERROR(gcoOS_AtomIncrement(gcPLS.os, gcPLS.reference, gcvNULL));

    /* Construct gcoHAL object. */
    gcmONERROR(gcoHAL_ConstructEx(gcvNULL, gcvNULL, &gcPLS.hal));

    /* Construct access lock */
    gcmONERROR(gcoOS_CreateMutex(gcPLS.os, &gcPLS.accessLock));

    /* Construct gl FE compiler access lock */
    gcmONERROR(gcoOS_CreateMutex(gcPLS.os, &gcPLS.glFECompilerAccessLock));

    /* Construct cl FE compiler access lock */
    gcmONERROR(gcoOS_CreateMutex(gcPLS.os, &gcPLS.clFECompilerAccessLock));

#if gcdDUMP_2D
    gcmONERROR(gcoOS_CreateMutex(gcPLS.os, &dumpMemInfoListMutex));
#endif

    gcmFOOTER_ARG(
        "gcPLS.os=0x%08X, gcPLS.hal=0x%08X"
        " internal=0x%08X external=0x%08X contiguous=0x%08X",
        gcPLS.os, gcPLS.hal,
        gcPLS.internalLogical, gcPLS.externalLogical, gcPLS.contiguousLogical
        );

    return;

OnError:
    if (gcPLS.accessLock != gcvNULL)
    {
        /* Destroy access lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.accessLock));
    }

    if (gcPLS.glFECompilerAccessLock != gcvNULL)
    {
        /* Destroy access lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.glFECompilerAccessLock));
    }

    if (gcPLS.clFECompilerAccessLock != gcvNULL)
    {
        /* Destroy access lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcPLS.os, gcPLS.clFECompilerAccessLock));
    }

    if (gcPLS.reference != gcvNULL)
    {
        /* Destroy the reference. */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcPLS.os, gcPLS.reference));
    }

    gcmFOOTER();
}

#if (_NTO_VERSION >= 700)
static void
_ModuleDestructor(
    void
    )
#else
static void
_ModuleDestructor(
    void * arg
    )
#endif
{
    gctINT reference = 0;

#if (_NTO_VERSION < 700)
    arg = arg;
#endif

    gcmHEADER();

    if (gcPLS.reference != gcvNULL)
    {
        /* Decrement the reference for main thread. */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcPLS.os,
                                         gcPLS.reference,
                                         &reference));

        if (reference == 1)
        {
            /* If all threads exit, destruct PLS. */
            _PLSDestructor();
        }
        else
        {
            gcoOS_FreeThreadData();
        }
    }

    gcmFOOTER_NO();
}

/******************************************************************************\
********************************* gcoOS API Code *******************************
\******************************************************************************/

/*******************************************************************************
 **
 ** gcoOS_GetPLSValue
 **
 ** Get value associated with the given key.
 **
 ** INPUT:
 **
 **     gcePLS_VALUE key
 **         key to look up.
 **
 ** OUTPUT:
 **
 **     None
 **
 ** RETURN:
 **
 **     gctPOINTER
 **         Pointer to object associated with key.
 */
gctPOINTER
gcoOS_GetPLSValue(
    IN gcePLS_VALUE key
    )
{
    switch (key)
    {
        case gcePLS_VALUE_EGL_DISPLAY_INFO :
            return gcPLS.eglDisplayInfo;

        case gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO :
            return (gctPOINTER) gcPLS.eglConfigFormat;

        case gcePLS_VALUE_EGL_DESTRUCTOR_INFO :
            return (gctPOINTER) gcPLS.destructor;
    }

    return gcvNULL;
}

/*******************************************************************************
 **
 ** gcoOS_SetPLSValue
 **
 ** Associated object represented by 'value' with the given key.
 **
 ** INPUT:
 **
 **     gcePLS_VALUE key
 **         key to associate.
 **
 **     gctPOINTER value
 **         value to associate with key.
 **
 ** OUTPUT:
 **
 **     None
 **
 */
void
gcoOS_SetPLSValue(
    IN gcePLS_VALUE key,
    IN gctPOINTER value
    )
{
    switch (key)
    {
        case gcePLS_VALUE_EGL_DISPLAY_INFO :
            gcPLS.eglDisplayInfo = value;
            return;

        case gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO :
            gcPLS.eglConfigFormat = (gceSURF_FORMAT) value;
            return;

        case gcePLS_VALUE_EGL_DESTRUCTOR_INFO :
            gcPLS.destructor = (gctPLS_DESTRUCTOR) value;
            return;
    }
}

/*******************************************************************************
 **
 ** gcoOS_GetTLS
 **
 ** Get access to the thread local storage.
 **
 ** INPUT:
 **
 **     Nothing.
 **
 ** OUTPUT:
 **
 **     gcsTLS_PTR * TLS
 **         Pointer to a variable that will hold the pointer to the TLS.
 */
gceSTATUS
gcoOS_GetTLS(
    OUT gcsTLS_PTR * TLS
    )
{
    gceSTATUS status;
    gcsTLS_PTR tls;
    int res;

    gcmHEADER_ARG("TLS=%p", TLS);

    tls = (gcsTLS_PTR) pthread_getspecific(gcProcessKey);

    if (tls == NULL)
    {
        gcmONERROR(gcoOS_AllocateMemory(
            gcvNULL, gcmSIZEOF(gcsTLS), (gctPOINTER *) &tls
            ));

        gcoOS_ZeroMemory(tls, gcmSIZEOF(gcsTLS));

        /* The default hardware type is 2D */
        tls->currentType = gcvHARDWARE_2D;

        res = pthread_setspecific(gcProcessKey, tls);

        if (res != 0)
        {
            gcmTRACE(
                gcvLEVEL_ERROR,
                "%s(%d): pthread_setspecific returned %d",
                __FUNCTION__, __LINE__, res
                );

            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }

        if (gcPLS.reference != gcvNULL)
        {
            /* Increment PLS reference. */
            gcmONERROR(gcoOS_AtomIncrement(gcPLS.os, gcPLS.reference, gcvNULL));
        }

#if gcdDUMP || gcdDUMP_API || gcdDUMP_2D
        _SetDumpFileInfo();
#endif
    }

    * TLS = tls;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (tls != gcvNULL)
    {
        gcmVERIFY_OK(gcoOS_FreeMemory(gcvNULL, (gctPOINTER) tls));
    }

    * TLS = gcvNULL;

    gcmFOOTER();
    return status;
}

/*
 *  gcoOS_CopyTLS
 *
 *  Copy the TLS from a source thread and mark this thread as a copied thread, so the destructor won't free the resources.
 *
 *  NOTE: Make sure the "source thread" doesn't get kiiled while this thread is running, since the objects will be taken away. This
 *  will be fixed in a future version of the HAL when reference counters will be used to keep track of object usage (automatic
 *  destruction).
 */
gceSTATUS gcoOS_CopyTLS(IN gcsTLS_PTR Source)
{
    gceSTATUS   status;
    gcsTLS_PTR  tls;

    gcmHEADER();

    /* Verify the arguyments. */
    gcmDEBUG_VERIFY_ARGUMENT(Source != gcvNULL);

    /* Get the thread specific data. */
    tls = pthread_getspecific(gcProcessKey);

    if (tls != gcvNULL)
    {
        /* We cannot copy if the TLS has already been initialized. */
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    /* Allocate memory for the TLS. */
    gcmONERROR(gcoOS_AllocateMemory(gcvNULL, gcmSIZEOF(gcsTLS), (gctPOINTER *) &tls));

    /* Set the thread specific data. */
    pthread_setspecific(gcProcessKey, tls);

    if (gcPLS.reference != gcvNULL)
    {
        /* Increment PLS reference. */
        gcmONERROR(gcoOS_AtomIncrement(gcPLS.os, gcPLS.reference, gcvNULL));
    }

    /* Copy the TLS information. */
    gcoOS_MemCopy(tls, Source, sizeof(gcsTLS));

    /* Mark this TLS as copied. */
    tls->copied = gcvTRUE;

    tls->currentHardware = gcvNULL;

#if gcdDUMP || gcdDUMP_API || gcdDUMP_2D
    _SetDumpFileInfo();
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
gcoOS_QueryTLS(
    OUT gcsTLS_PTR * TLS
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsTLS_PTR tls;

    gcmHEADER();

    /* Verify the arguyments. */
    gcmVERIFY_ARGUMENT(TLS != gcvNULL);

    /* Get the thread specific data. */
    tls = pthread_getspecific(gcProcessKey);

    /* Return pointer to user. */
    *TLS = tls;

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* Get access to driver tls. */
gceSTATUS
gcoOS_GetDriverTLS(
    IN gceTLS_KEY Key,
    OUT gcsDRIVER_TLS_PTR * TLS
    )
{
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER_ARG("Key=%d", Key);

    if ((Key < (gceTLS_KEY)0) || (Key >= gcvTLS_KEY_COUNT))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get generic tls. */
    gcmONERROR(gcoOS_GetTLS(&tls));

    *TLS = tls->driverTLS[Key];
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* Set driver tls. */
gceSTATUS
gcoOS_SetDriverTLS(
    IN gceTLS_KEY Key,
    IN gcsDRIVER_TLS * TLS
    )
{
    gceSTATUS status;
    gcsTLS_PTR tls;

    gcmHEADER_ARG("Key=%d", Key);

    if ((Key < (gceTLS_KEY)0) || (Key >= gcvTLS_KEY_COUNT))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get generic tls. */
    gcmONERROR(gcoOS_GetTLS(&tls));

    tls->driverTLS[Key] = TLS;
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoOS_LockPLS
**
**  Lock mutext before access PLS if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_LockPLS(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.accessLock)
    {
        status = gcoOS_AcquireMutex(gcPLS.os, gcPLS.accessLock, gcvINFINITE);
    }
    gcmFOOTER_ARG("Lock PLS ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_UnLockPLS
**
**  Release mutext after access PLS if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_UnLockPLS(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.accessLock)
    {
        status = gcoOS_ReleaseMutex(gcPLS.os, gcPLS.accessLock);
    }
    gcmFOOTER_ARG("Release PLS ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_LockGLFECompiler
**
**  Lock mutext before access GL FE compiler if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_LockGLFECompiler(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.glFECompilerAccessLock)
    {
        status = gcoOS_AcquireMutex(gcPLS.os, gcPLS.glFECompilerAccessLock, gcvINFINITE);
    }
    gcmFOOTER_ARG("Lock GL FE compiler ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_UnLockGLFECompiler
**
**  Release mutext after access GL FE compiler if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_UnLockGLFECompiler(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.glFECompilerAccessLock)
    {
        status = gcoOS_ReleaseMutex(gcPLS.os, gcPLS.glFECompilerAccessLock);
    }
    gcmFOOTER_ARG("Release GL FE compiler ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_LockCLFECompiler
**
**  Lock mutext before access CL FE compiler if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_LockCLFECompiler(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.clFECompilerAccessLock)
    {
        status = gcoOS_AcquireMutex(gcPLS.os, gcPLS.clFECompilerAccessLock, gcvINFINITE);
    }
    gcmFOOTER_ARG("Lock CL FE compiler ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_UnLockCLFECompiler
**
**  Release mutext after access CL FE compiler if needed
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_UnLockCLFECompiler(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    if (gcPLS.clFECompilerAccessLock)
    {
        status = gcoOS_ReleaseMutex(gcPLS.os, gcPLS.clFECompilerAccessLock);
    }
    gcmFOOTER_ARG("Release CL FE compiler ret=%d", status);

    return status;
}

/*******************************************************************************
**
**  gcoOS_FreeThreadData
**
**  Destroy the objects associated with the current thread.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
*/
void
gcoOS_FreeThreadData(
    void
    )
{
    gcsTLS_PTR tls;

    tls = (gcsTLS_PTR) pthread_getspecific(gcProcessKey);

    if (tls != NULL)
    {
        _TLSDestructor((gctPOINTER) tls);
    }
}

/*******************************************************************************
 **
 ** gcoOS_Construct
 **
 ** Construct a new gcoOS object. Empty function only for compatibility.
 **
 ** INPUT:
 **
 **     gctPOINTER Context
 **         Pointer to an OS specific context.
 **
 ** OUTPUT:
 **
 **     Nothing.
 **
 */
gceSTATUS
gcoOS_Construct(
    IN gctPOINTER Context,
    OUT gcoOS *Os
    )
{
    /* Return gcoOS object for compatibility to prevent any failure in applications. */
    *Os = gcPLS.os;

    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 ** gcoOS_Destroy
 **
 ** Destroys an gcoOS object. Empty function only for compatibility.
 **
 ** ARGUMENTS:
 **
 **     gcoOS Os
 **         Pointer to the gcoOS object that needs to be destroyed.
 **
 ** OUTPUT:
 **
 **     Nothing.
 **
 */
gceSTATUS
gcoOS_Destroy(
    IN gcoOS Os
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_GetPhysicalSystemMemorySize
**
**  Get the amount of system memory.
**
**  OUTPUT:
**
**      gctSIZE_T * PhysicalSystemMemorySize
**          Pointer to a variable that will hold the size of the physical system
**          memory.
*/
gceSTATUS
gcoOS_GetPhysicalSystemMemorySize(
    OUT gctSIZE_T * PhysicalSystemMemorySize
    )
{
    char *str = SYSPAGE_ENTRY(strings)->data;
    struct asinfo_entry *as = SYSPAGE_ENTRY(asinfo);
    off64_t total = 0;
    unsigned num;

    for (num = _syspage_ptr->asinfo.entry_size / sizeof(*as); num > 0; --num)
    {
        if (strcmp(&str[as->name], "ram") == 0)
        {
            total += as->end - as->start + 1;
        }

        ++as;
    }

    if (PhysicalSystemMemorySize != gcvNULL)
    {
        *PhysicalSystemMemorySize = (gctSIZE_T) total;
    }

    return  gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_QueryVideoMemory
**
**  Query the amount of video memory.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to a gcoOS object.
**
**  OUTPUT:
**
**      gctUINT32 * InternalPhysName
**          Pointer to a variable that will hold the physical memory name of the
**          internal memory.  If 'InternalPhysName' is gcvNULL, no information
**          about the internal memory will be returned.
**
**      gctSIZE_T * InternalSize
**          Pointer to a variable that will hold the size of the internal
**          memory.  'InternalSize' cannot be gcvNULL if 'InternalPhysName' is
**          not gcvNULL.
**
**      gctUINT32 * ExternalPhysName
**          Pointer to a variable that will hold the physical memory name of the
**          external memory.  If 'ExternalPhysName' is gcvNULL, no information
**          about the external memory will be returned.
**
**      gctSIZE_T * ExternalSize
**          Pointer to a variable that will hold the size of the external
**          memory.  'ExternalSize' cannot be gcvNULL if 'ExternalPhysName' is
**          not gcvNULL.
**
**      gctUINT32 * ContiguousPhysName
**          Pointer to a variable that will hold the physical memory name of the
**          contiguous memory.  If 'ContiguousPhysName' is gcvNULL, no
**          information about the contiguous memory will be returned.
**
**      gctSIZE_T * ContiguousSize
**          Pointer to a variable that will hold the size of the contiguous
**          memory.  'ContiguousSize' cannot be gcvNULL if 'ContiguousPhysName'
**          is not gcvNULL.
*/
gceSTATUS
gcoOS_QueryVideoMemory(
    IN gcoOS Os,
    OUT gctUINT32 * InternalPhysName,
    OUT gctSIZE_T * InternalSize,
    OUT gctUINT32 * ExternalPhysName,
    OUT gctSIZE_T * ExternalSize,
    OUT gctUINT32 * ContiguousPhysName,
    OUT gctSIZE_T * ContiguousSize
    )
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmHEADER();

    /* Call kernel HAL to query video memory. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_QUERY_VIDEO_MEMORY;

    /* Call kernel service. */
    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    if (InternalPhysName != gcvNULL)
    {
        /* Verify arguments. */
        gcmDEBUG_VERIFY_ARGUMENT(InternalSize != gcvNULL);

        /* Save internal memory size. */
        *InternalPhysName = iface.u.QueryVideoMemory.internalPhysName;
        *InternalSize    = iface.u.QueryVideoMemory.internalSize;
    }

    if (ExternalPhysName != gcvNULL)
    {
        /* Verify arguments. */
        gcmDEBUG_VERIFY_ARGUMENT(ExternalSize != gcvNULL);

        /* Save external memory size. */
        *ExternalPhysName = iface.u.QueryVideoMemory.externalPhysName;
        *ExternalSize    = iface.u.QueryVideoMemory.externalSize;
    }

    if (ContiguousPhysName != gcvNULL)
    {
        /* Verify arguments. */
        gcmDEBUG_VERIFY_ARGUMENT(ContiguousSize != gcvNULL);

        /* Save contiguous memory size. */
        *ContiguousPhysName = iface.u.QueryVideoMemory.contiguousPhysName;
        *ContiguousSize    = iface.u.QueryVideoMemory.contiguousSize;
    }

    /* Success. */
    gcmFOOTER_ARG("*InternalPhysName=0x%08x *InternalSize=%lu "
                  "*ExternalAddress=0x%08x *ExternalSize=%lu "
                  "*ContiguousPhysName=0x%08x *ContiguousSize=%lu",
                  gcmOPT_VALUE(InternalPhysName), gcmOPT_VALUE(InternalSize),
                  gcmOPT_VALUE(ExternalAddress), gcmOPT_VALUE(ExternalSize),
                  gcmOPT_VALUE(ContiguousPhysName),
                  gcmOPT_VALUE(ContiguousSize));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
** Deprecated API: please use gcoHAL_GetBaseAddr() instead.
**                 This API was kept only for legacy BSP usage.
**
**  gcoOS_GetBaseAddress
**
**  Get the base address for the physical memory.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to the gcoOS object.
**
**  OUTPUT:
**
**      gctUINT32_PTR BaseAddress
**          Pointer to a variable that will receive the base address.
*/
gceSTATUS
gcoOS_GetBaseAddress(
    IN gcoOS Os,
    OUT gctUINT32_PTR BaseAddress
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gceHARDWARE_TYPE type = gcvHARDWARE_INVALID;

    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(BaseAddress);

    gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &type));

    /* Return base address. */
    if (type == gcvHARDWARE_VG)
    {
        *BaseAddress = 0;
    }
    else
    {
        gcsHAL_INTERFACE iface;

        /* Query base address. */
        iface.ignoreTLS = gcvFALSE;
        iface.command = gcvHAL_GET_BASE_ADDRESS;

        /* Call kernel driver. */
        gcmONERROR(gcoOS_DeviceControl(
            gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(iface),
            &iface, gcmSIZEOF(iface)
            ));

        *BaseAddress = iface.u.GetBaseAddress.baseAddress;
    }

OnError:
    /* Success. */
    gcmFOOTER_ARG("*BaseAddress=0x%08x", *BaseAddress);
    return status;
}

/*******************************************************************************
**
**  gcoOS_Allocate
**
**  Allocate memory from the user heap.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the pointer to the memory
**          allocation.
*/
gceSTATUS
gcoOS_Allocate(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Bytes=%lu", Bytes);

    /* Special wrapper when gcPLS.os is gcvNULL. */
    if (gcPLS.os == gcvNULL)
    {
        if (Bytes == 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        *Memory = malloc(Bytes);
        if (*Memory == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    }
    else
    {
        /* Verify the arguments. */
        gcmDEBUG_VERIFY_ARGUMENT(Bytes > 0);
        gcmDEBUG_VERIFY_ARGUMENT(Memory != gcvNULL);

        if ((gcPLS.os != gcvNULL) && (gcPLS.os->heap != gcvNULL))
        {
            gcmONERROR(gcoHEAP_Allocate(gcPLS.os->heap, Bytes, Memory));
        }
        else
        {
            gcmONERROR(gcoOS_AllocateMemory(gcPLS.os, Bytes, Memory));
        }
    }

    /* Success. */
    gcmFOOTER_ARG("*Memory=0x%x", *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_GetMemorySize
**
**  Get allocated memory from the user heap.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER  Memory
**          Pointer to the memory
**          allocation.
**
**  OUTPUT:
**
**      gctPOINTER MemorySize
**          Pointer to a variable that will hold the pointer to the memory
**          size.
*/
gceSTATUS
gcoOS_GetMemorySize(
    IN gcoOS Os,
    IN gctPOINTER Memory,
    OUT gctSIZE_T_PTR MemorySize
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Memory=0x%x", Memory);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Memory != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(MemorySize != gcvNULL);

    /* Free the memory. */
    if ((gcPLS.os != gcvNULL) && (gcPLS.os->heap != gcvNULL))
    {
        gcmONERROR(gcoHEAP_GetMemorySize(gcPLS.os->heap, Memory, MemorySize));
    }
    else
    {
        *MemorySize = 0;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
 **
 ** gcoOS_Free
 **
 ** Free allocated memory from the user heap.
 **
 ** INPUT:
 **
 **     gcoOS Os
 **         Pointer to an gcoOS object.
 **
 **     gctPOINTER Memory
 **         Pointer to the memory allocation that needs to be freed.
 **
 ** OUTPUT:
 **
 **     Nothing.
 */
gceSTATUS
gcoOS_Free(
    IN gcoOS Os,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Memory=0x%x", Memory);

    /* Special wrapper when gcPLS.os is gcvNULL. */
    if (gcPLS.os == gcvNULL)
    {
        if (Memory == gcvNULL)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        free(Memory);
    }
    else
    {
        /* Verify the arguments. */
        gcmDEBUG_VERIFY_ARGUMENT(Memory != gcvNULL);

        /* Free the memory. */
        if ((gcPLS.os != gcvNULL) && (gcPLS.os->heap != gcvNULL))
        {
            gcmONERROR(gcoHEAP_Free(gcPLS.os->heap, Memory));
        }
        else
        {
            gcmONERROR(gcoOS_FreeMemory(gcPLS.os, Memory));
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_AllocateNonPagedMemory(
    IN gcoOS Os,
    IN gctBOOL InUserSpace,
    IN gctUINT32 Flags,
    IN OUT gctSIZE_T * Bytes,
    OUT gctUINT32 * PhysName,
    OUT gctPOINTER * Logical
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("InUserSpace=%d *Bytes=%lu",
                  InUserSpace, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(PhysName != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Logical != gcvNULL);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_ALLOCATE_NON_PAGED_MEMORY;
    iface.u.AllocateNonPagedMemory.flags = Flags;
    iface.u.AllocateNonPagedMemory.bytes = *Bytes;

    /* Call kernel driver. */
    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        ));

    /* Get status from kernel. */
    status = iface.status;

    if (status == gcvSTATUS_OK)
    {
        /* Return allocated number of bytes. */
        *Bytes = iface.u.AllocateNonPagedMemory.bytes;

        /* Return physical and logical address. */
        *PhysName = iface.u.AllocateNonPagedMemory.physName;
        *Logical = gcmUINT64_TO_PTR(iface.u.AllocateNonPagedMemory.logical);
    }
    else
    {
        gcmTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_DRIVER,
                    "_AllocateNonPagedMemory: failed to allocate memory status->%d",
                    status);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_FreeNonPagedMemory(
    IN gcoOS Os,
    IN gctUINT32 PhysName,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("Bytes=%lu PhysName=0x%x Logical=0x%x",
                  Bytes, PhysName, Logical);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_FREE_NON_PAGED_MEMORY;
    iface.u.FreeNonPagedMemory.bytes    = Bytes;
    iface.u.FreeNonPagedMemory.physName = PhysName;
    iface.u.FreeNonPagedMemory.logical  = gcmPTR_TO_UINT64(Logical);

    /* Call kernel driver. */
    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, sizeof(iface),
        &iface, sizeof(iface)
        ));

    /* Get status from kernel. */
    status = iface.status;

OnError:
    /* Return status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_AllocateSharedMemory
**
**  Allocate memory that can be used in both user and kernel.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the pointer to the memory
**          allocation.
*/
gceSTATUS
gcoOS_AllocateSharedMemory(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;
    gctUINT32 physical = 0;
    gctPOINTER pointer = gcvNULL;
    gcsSharedMemoryNode node;

    gcmHEADER_ARG("Bytes=%lu", Bytes);

    if (Memory == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Allocate the memory. */
    gcmONERROR(_AllocateNonPagedMemory(
        Os, gcvTRUE, gcvALLOC_FLAG_CONTIGUOUS, &Bytes, &physical, &pointer));

    /* Initialize the node. */
    node.Memory   = pointer;
    node.Bytes    = Bytes;
    node.Physical = physical;

    /* Add the node to the list. */
    gcmONERROR(_AddSharedMemoryNode(&node));

    /* Return the pointer to the memory. */
    *Memory = pointer;

    /* Success. */
    gcmFOOTER_ARG("*Memory=0x%x", *Memory);
    return gcvSTATUS_OK;

OnError:
    if (pointer != gcvNULL)
    {
        gcmVERIFY_OK(_FreeNonPagedMemory(Os, physical, pointer, Bytes));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
 **
 ** gcoOS_FreeSharedMemory
 **
 ** Free allocated memory.
 **
 ** INPUT:
 **
 **     gcoOS Os
 **         Pointer to an gcoOS object.
 **
 **     gctPOINTER Memory
 **         Pointer to the memory allocation that needs to be freed.
 **
 ** OUTPUT:
 **
 **     Nothing.
 */
gceSTATUS
gcoOS_FreeSharedMemory(
    IN gcoOS Os,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status;
    gcsSharedMemoryNode node;

    gcmHEADER_ARG("Memory=0x%x", Memory);

    gcmONERROR(_DetachSharedMemoryNode(Memory, &node));

    gcmVERIFY_OK(_FreeNonPagedMemory(
        Os, node.Physical, node.Memory, node.Bytes));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
 **
 ** gcoOS_AllocateMemory
 **
 ** Allocate memory from the user heap.
 **
 ** INPUT:
 **
 **     gcoOS Os
 **         Pointer to an gcoOS object.
 **
 **     gctSIZE_T Bytes
 **         Number of bytes to allocate.
 **
 ** OUTPUT:
 **
 **     gctPOINTER * Memory
 **         Pointer to a variable that will hold the pointer to the memory
 **         allocation.
 */
gceSTATUS
gcoOS_AllocateMemory(
    IN gcoOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;
    gctPOINTER memory;

    gcmHEADER_ARG("Bytes=%lu", Bytes);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Bytes > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Memory != gcvNULL);

    /* Allocate the memory. */
    memory = malloc(Bytes);

    if (memory == gcvNULL)
    {
        /* Out of memory. */
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Return pointer to the memory allocation. */
    *Memory = memory;

    gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
          "gcoOS_AllocateMemory: Memory allocated->0x%x:0x%x bytes",
          memory,
          (gctUINT32) Bytes);

    /* Success. */
    gcmFOOTER_ARG("*Memory=0x%x", *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
 **
 ** gcoOS_FreeMemory
 **
 ** Free allocated memory from the user heap.
 **
 ** INPUT:
 **
 **     gcoOS Os
 **         Pointer to an gcoOS object.
 **
 **     gctPOINTER Memory
 **         Pointer to the memory allocation that needs to be freed.
 **
 ** OUTPUT:
 **
 **     Nothing.
 */
gceSTATUS
gcoOS_FreeMemory(
    IN gcoOS Os,
    IN gctPOINTER Memory
    )
{
    gcmHEADER_ARG("Memory=0x%x", Memory);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Memory != gcvNULL);

    /* Free the memory allocation. */
    free(Memory);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 ** gcoOS_DeviceControl
 **
 ** Perform a device I/O control call to the kernel API.
 **
 ** INPUT:
 **
 **     gcoOS Os
 **         Pointer to an gcoOS object.
 **
 **     gctUINT32 IoControlCode
 **         I/O control code to execute.
 **
 **     gctPOINTER InputBuffer
 **         Pointer to the input buffer.
 **
 **     gctSIZE_T InputBufferSize
 **         Size of the input buffer in bytes.
 **
 **     gctSIZE_T outputBufferSize
 **         Size of the output buffer in bytes.
 **
 ** OUTPUT:
 **
 **     gctPOINTER OutputBuffer
 **         Output buffer is filled with the data returned from the kernel HAL
 **         layer.
 */
gceSTATUS
gcoOS_DeviceControl(
    IN gcoOS Os,
    IN gctUINT32 IoControlCode,
    IN gctPOINTER InputBuffer,
    IN gctSIZE_T InputBufferSize,
    OUT gctPOINTER OutputBuffer,
    IN gctSIZE_T OutputBufferSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsDRIVER_ARGS args;
    gcsHAL_INTERFACE_PTR iface = (gcsHAL_INTERFACE_PTR) InputBuffer;
    int rc;
    gcsTLS_PTR tls;
    gctPOINTER logical;
    gcsSIGNAL_PTR signal;

    gcsQUEUE_PTR queue;
#if gcdENABLE_VG
    gcsVGCONTEXT_PTR context;
#endif

    if (!iface->ignoreTLS)
    {
        /* Set current hardware type */
        status = gcoOS_GetTLS(&tls);
        if (gcmIS_ERROR(status))
        {
            return status;
        }

        iface->hardwareType = tls->currentType;
        iface->coreIndex = tls->currentType == gcvHARDWARE_2D ? 0: tls->currentCoreIndex;
    }

    switch (iface->command)
    {
    case gcvHAL_MAP_MEMORY:

        if (iface->u.MapMemory.bytes > 70 * 1024 * 1024)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

        /*iface->u.MapMemory.logical = mmap64(NULL,*/
        logical = mmap(NULL,
                       iface->u.MapMemory.bytes,
                       PROT_READ | PROT_WRITE | PROT_NOCACHE,
                       MAP_SHARED | MAP_PHYS,
                       NOFD,
                       (off_t) iface->u.MapMemory.physName);

        iface->u.MapMemory.logical = gcmPTR_TO_UINT64(logical);
        if (logical != MAP_FAILED)
        {
            return (iface->status = gcvSTATUS_OK);
        }
        break;

    case gcvHAL_UNMAP_MEMORY:
        munmap(gcmUINT64_TO_PTR(iface->u.UnmapMemory.logical),
               iface->u.UnmapMemory.bytes);

        return (iface->status = gcvSTATUS_OK);

    case gcvHAL_SIGNAL:
        signal = gcmUINT64_TO_TYPE(iface->u.Signal.signal, gcsSIGNAL_PTR);
        gcoOS_SignalPending(Os, signal);
        iface->u.Signal.coid = gcPLS.os->pulseCoid;
        break;

    case gcvHAL_EVENT_COMMIT:
        queue = gcmUINT64_TO_PTR(iface->u.Event.queue);
        /* Walk the event queue. */
        for (; queue != gcvNULL; )
        {
            /* Test for signal event. */
            if (queue->iface.command == gcvHAL_SIGNAL)
            {
                signal = gcmUINT64_TO_TYPE(queue->iface.u.Signal.signal, gcsSIGNAL_PTR);
                gcoOS_SignalPending(Os, signal);
                queue->iface.u.Signal.coid = gcPLS.os->pulseCoid;
            }
            queue = gcmUINT64_TO_PTR(queue->next);
        }
        break;

    case gcvHAL_COMMIT:
#if gcdENABLE_VG
        if (iface->hardwareType == gcvHARDWARE_VG)
        {
            context = gcmUINT64_TO_PTR(iface->u.VGCommit.context);
            context->coid = gcPLS.os->pulseCoid;
            gcoOS_SignalPending(Os, context->signal);
        }
        else
#endif
        {
            gcsHAL_SUBCOMMIT *subCommit = &iface->u.Commit.subCommit;

            do
            {
                queue = gcmUINT64_TO_PTR(subCommit->queue);

                /* Walk the event queue. */
                while (queue)
                {
                    /* Test for signal event. */
                    if (queue->iface.command == gcvHAL_SIGNAL)
                    {
                        signal = gcmUINT64_TO_TYPE(queue->iface.u.Signal.signal, gcsSIGNAL_PTR);
                        gcoOS_SignalPending(Os, signal);
                        queue->iface.u.Signal.coid = gcPLS.os->pulseCoid;
                    }
                    queue = gcmUINT64_TO_PTR(queue->next);
                }

                subCommit = gcmUINT64_TO_PTR(subCommit->next);
            }
            while (subCommit);
        }
        break;

    case gcvHAL_WRAP_USER_MEMORY:
        {
            gcsUSER_MEMORY_DESC_PTR desc = &iface->u.WrapUserMemory.desc;

            if (desc->logical != 0)
            {
                /* Flush the data cache. */
                _CacheRangeFlush(
                    0, gcmINT2PTR(desc->logical), desc->size, gcvCACHE_FLUSH);
            }
        }
        break;

    default:
        break;
    }

    /* Send interface data as value. */
    gcoOS_MemCopy(&args.iface, iface, gcmSIZEOF(args.iface));

    args.iomsg.i.type = _IO_MSG;
    args.iomsg.i.subtype = IoControlCode;
    args.iomsg.i.mgrid = _IOMGR_VIVANTE;
    args.iomsg.i.combine_len = sizeof(args);

    do {
        rc = MsgSend_r(gcPLS.os->device,
                &args,
                args.iomsg.i.combine_len,
                OutputBuffer,
                OutputBufferSize);
    } while ((rc * -1) == EINTR);

    if (rc < 0)
    {
        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                      "%s(%d): ioctl failed.",
                      __FUNCTION__, __LINE__);

        /* Generic I/O error. */
        return gcvSTATUS_GENERIC_IO;
    }

    /* Success. */
    return ((gcsHAL_INTERFACE_PTR) OutputBuffer)->status;
}


/*******************************************************************************
**
**  gcoOS_AllocateVideoMemory
**
**  Allocate contiguous video memory from the kernel.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE to map the memory into the user space.
**
**      gctBOOL InCacheable
**          gcvTRUE to allocate the cacheable memory.
**
**
**      gctSIZE_T * Bytes
**          Pointer to the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that will receive the aligned number of bytes
**          allocated.
**
**      gctUINT32 * Physical
**          Pointer to a variable that will receive the physical addresses of
**          the allocated memory.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will receive the logical address of the
**          allocation.
**
**      gctPOINTER * Handle
**          Pointer to a variable that will receive the node handle of the
**          allocation.
*/
gceSTATUS
gcoOS_AllocateVideoMemory(
    IN gcoOS Os,
    IN gctBOOL InUserSpace,
    IN gctBOOL InCacheable,
    IN OUT gctSIZE_T * Bytes,
    OUT gctUINT32 * Physical,
    OUT gctPOINTER * Logical,
    OUT gctPOINTER * Handle
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoOS_FreeVideoMemory
**
**  Free contiguous video memory from the kernel.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER  Handle
**          Pointer to a variable that indicate the node of the
**          allocation.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_FreeVideoMemory(
    IN gcoOS Os,
    IN gctPOINTER Handle
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoOS_Open
**
**  Open or create a file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctCONST_STRING FileName
**          File name of file to open or create.
**
**      gceFILE_MODE Mode
**          Mode to open file with:
**
**              gcvFILE_CREATE      - Overwite any existing file.
**              gcvFILE_APPEND      - Append to an exisiting file or create a
**                                    new file if there is no exisiting file.
**              gcvFILE_READ        - Open an existing file for read only.
**              gcvFILE_CREATETEXT  - Overwite any existing text file.
**              gcvFILE_APPENDTEXT  - Append to an exisiting text file or create
**                                    a new text file if there is no exisiting
**                                    file.
**              gcvFILE_READTEXT    - Open an existing text file fir read only.
**
**  OUTPUT:
**
**      gctFILE * File
**          Pointer to a variable receivig the handle to the opened file.
*/
gceSTATUS
gcoOS_Open(
    IN gcoOS Os,
    IN gctCONST_STRING FileName,
    IN gceFILE_MODE Mode,
    OUT gctFILE * File
    )
{
    static gctCONST_STRING modes[] =
    {
        "wb",
        "ab",
        "rb",
        "w",
        "a",
        "r",
    };
    FILE * file;

    gcmHEADER_ARG("FileName=%s Mode=%d",
                  gcmOPT_STRING(FileName), Mode);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);

    /* Open the file. */
    file = fopen(FileName, modes[Mode]);

    if (file == gcvNULL)
    {
        /* Error. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }

    /* Return handle to file. */
    *File = (gctFILE) file;

    /* Success. */
    gcmFOOTER_ARG("*File=0x%x", *File);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_Close
**
**  Close a file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Close(
    IN gcoOS Os,
    IN gctFILE File
    )
{
    gcmHEADER_ARG("File=0x%x", File);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);

    /* Close the file. */
    fclose((FILE *) File);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_Remove(
    IN gcoOS Os,
    IN gctCONST_STRING FileName
    )
{
    int ret;
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("FileName=%s", FileName);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(FileName != gcvNULL);

    ret = remove((const char *)FileName);

    if (ret != 0)
    {
        status = gcvSTATUS_GENERIC_IO;
    }

    /* Success. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_Read
**
**  Read data from an open file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctSIZE_T ByteCount
**          Number of bytes to read from the file.
**
**      gctCONST_POINTER Data
**          Pointer to the data to read from the file.
**
**  OUTPUT:
**
**      gctSIZE_T * ByteRead
**          Pointer to a variable receiving the number of bytes read from the
**          file.
*/
gceSTATUS
gcoOS_Read(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctSIZE_T ByteCount,
    IN gctPOINTER Data,
    OUT gctSIZE_T * ByteRead
    )
{
    size_t byteRead;

    gcmHEADER_ARG("File=0x%x ByteCount=%lu Data=0x%x",
                  File, ByteCount, Data);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(ByteCount > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Data != gcvNULL);

    /* Read the data from the file. */
    byteRead = fread(Data, 1, ByteCount, (FILE *) File);

    if (ByteRead != gcvNULL)
    {
        *ByteRead = (gctSIZE_T) byteRead;
    }

    /* Success. */
    gcmFOOTER_ARG("*ByteRead=%lu", gcmOPT_VALUE(ByteRead));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_Write
**
**  Write data to an open file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctSIZE_T ByteCount
**          Number of bytes to write to the file.
**
**      gctCONST_POINTER Data
**          Pointer to the data to write to the file.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Write(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctSIZE_T ByteCount,
    IN gctCONST_POINTER Data
    )
{
    size_t byteWritten;

    gcmHEADER_ARG("File=0x%x ByteCount=%lu Data=0x%x",
                  File, ByteCount, Data);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(ByteCount > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Data != gcvNULL);

    /* Write the data to the file. */
    byteWritten = fwrite(Data, 1, ByteCount, (FILE *) File);

    if (byteWritten == ByteCount)
    {
        /* Success. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        /* Error */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }
}

/* Flush data to a file. */
gceSTATUS
gcoOS_Flush(
    IN gcoOS Os,
    IN gctFILE File
    )
{
    gcmHEADER_ARG("File=0x%x", File);

    fflush((FILE *) File);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_LockFile
**
**  Apply an advisory lock on an open file
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctBOOL Shared
**          Place a shared lock if true. More than one process may hold a
**          shared lock for a given file at a given time.
**          Place an exclusive lock. Only one process may hold an exclusive
**          lock for a given file at a given time.
**
**      gctBOOL Block
**          Block if an incompatible lock is held by another process.
**          Otherwise return immediately with gcvSTATUS_LOCKED error.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_LockFile(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctBOOL Shared,
    IN gctBOOL Block
    )
{
    int flags;
    int err;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("File=%p Shared=%d Block=%d", File, Shared, Block);

    flags  = Shared ? LOCK_SH : LOCK_EX;
    flags |= Block ? 0 : LOCK_NB;

    err = flock(fileno((FILE *)File), flags);

    if (err)
    {
        if (errno == EWOULDBLOCK)
        {
            gcmONERROR(gcvSTATUS_LOCKED);
        }
        else if (errno == EINTR)
        {
            gcmONERROR(gcvSTATUS_INTERRUPTED);
        }
        else
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("status=%d", status);
    return status;
}

/*******************************************************************************
**
**  gcoOS_UnlockFile
**
**  Remove an advisory lock on an open file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_UnlockFile(
    IN gcoOS Os,
    IN gctFILE File
    )
{
    int err;
    gceSTATUS status;

    gcmHEADER_ARG("File=%p", File);

    err = flock(fileno((FILE *)File), LOCK_UN);

    if (err)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("status=%d", status);
    return status;
}

/* Create an endpoint for communication. */
gceSTATUS
gcoOS_Socket(
    IN gcoOS Os,
    IN gctINT Domain,
    IN gctINT Type,
    IN gctINT Protocol,
    OUT gctINT * SockFd
    )
{
    gctINT fd;

    gcmHEADER_ARG("Domain=%d Type=%d Protocol=%d", Domain, Type, Protocol);

    /* Create a socket. */
    fd = socket(Domain, Type, Protocol);

    if (fd >= 0)
    {
        /* Return socket descriptor. */
        *SockFd = fd;

        /* Success. */
        gcmFOOTER_ARG("*SockFd=%d", *SockFd);
        return gcvSTATUS_OK;
    }
    else
    {
        /* Error. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }
}

/* Close a socket. */
gceSTATUS
gcoOS_WaitForSend(
    IN gcoOS Os,
    IN gctINT SockFd,
    IN gctINT Seconds,
    IN gctINT MicroSeconds
    )
{
    gcmHEADER_ARG("SockFd=%d Seconds=%d MicroSeconds=%d",
                  SockFd, Seconds, MicroSeconds);

    struct timeval tv;
    fd_set writefds;
    int ret;

    /* Linux select() will overwrite the struct on return */
    tv.tv_sec  = Seconds;
    tv.tv_usec = MicroSeconds;

    FD_ZERO(&writefds);
    FD_SET(SockFd, &writefds);

    ret = select(SockFd + 1, NULL, &writefds, NULL, &tv);

    if (ret == 0)
    {
        /* Timeout. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_TIMEOUT);
        return gcvSTATUS_TIMEOUT;
    }
    else if (ret == -1)
    {
        /* Error. */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }
    else
    {
        int error = 0;
        socklen_t len = sizeof(error);

        /* Get error code. */
        getsockopt(SockFd, SOL_SOCKET, SO_ERROR, (char*) &error, &len);

        if (!error)
        {
            /* Success. */
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

    /* Error */
    gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
    return gcvSTATUS_GENERIC_IO;
}

/* Close a socket. */
gceSTATUS
gcoOS_CloseSocket(
    IN gcoOS Os,
    IN gctINT SockFd
    )
{
    gcmHEADER_ARG("SockFd=%d", SockFd);

    /* Close the socket file descriptor. */
    gcoOS_WaitForSend(gcvNULL, SockFd, 600, 0);
    close(SockFd);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Initiate a connection on a socket. */
gceSTATUS
gcoOS_Connect(
    IN gcoOS Os,
    IN gctINT SockFd,
    IN gctCONST_POINTER HostName,
    IN gctUINT Port
    )
{
    gctINT rc;
    gctINT addrLen;
    struct sockaddr sockAddr;
    struct sockaddr_in *sockAddrIn;
    struct in_addr *inAddr;

    gcmHEADER_ARG("SockFd=0x%x HostName=0x%x Port=%d", SockFd, HostName, Port);

    /* Get server address. */
    sockAddrIn = (struct sockaddr_in *) &sockAddr;
    sockAddrIn->sin_family = AF_INET;
    inAddr = &sockAddrIn->sin_addr;
    inAddr->s_addr = inet_addr(HostName);

    /* If it is a numeric host name, convert it now */
    if (inAddr->s_addr == INADDR_NONE)
    {
        struct hostent *hostEnt;
        struct in_addr *arrayAddr;

        /* It is a real name, we solve it */
        if ((hostEnt = gethostbyname(HostName)) == NULL)
        {
            /* Error */
            gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
            return gcvSTATUS_GENERIC_IO;
        }
        arrayAddr = (struct in_addr *) *(hostEnt->h_addr_list);
        inAddr->s_addr = arrayAddr[0].s_addr;
    }

    sockAddrIn->sin_port = htons((gctUINT16) Port);

    /* Currently, for INET only. */
    addrLen = sizeof(struct sockaddr);

    /*{
    gctINT arg = 1;
    ioctl(SockFd, FIONBIO, &arg);
    }*/

    /* Close the file descriptor. */
    rc = connect(SockFd, &sockAddr, addrLen);

    if (rc)
    {
        int err = errno;

        if (err == EINPROGRESS)
        {
            gceSTATUS status;

            /* Connect is not complete.  Wait for it. */
            status = gcoOS_WaitForSend(gcvNULL, SockFd, 600, 0);

            gcmFOOTER();
            return status;
        }

        /* Error */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Shut down part of connection on a socket. */
gceSTATUS
gcoOS_Shutdown(
    IN gcoOS Os,
    IN gctINT SockFd,
    IN gctINT How
    )
{
    gcmHEADER_ARG("SockFd=%d How=%d", SockFd, How);

    /* Shut down connection. */
    shutdown(SockFd, How);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Send a message on a socket. */
gceSTATUS
gcoOS_Send(
    IN gcoOS Os,
    IN gctINT SockFd,
    IN gctSIZE_T ByteCount,
    IN gctCONST_POINTER Data,
    IN gctINT Flags
    )
{
    gctINT byteSent;

    gcmHEADER_ARG("SockFd=0x%x ByteCount=%lu Data=0x%x Flags=%d",
                  SockFd, ByteCount, Data, Flags);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(ByteCount > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Data != gcvNULL);

    /* Write the data to the file. */
    /*gcoOS_WaitForSend(gcvNULL, SockFd, 0, 50000);*/
    byteSent = send(SockFd, Data, ByteCount, Flags);

    if (byteSent == (gctINT)ByteCount)
    {
        /* Success. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        /* Error */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }
}

/* Get current working directory. */
gceSTATUS
gcoOS_GetCwd(
    IN gcoOS Os,
    IN gctINT SizeInBytes,
    OUT gctSTRING Buffer
    )
{
    gcmHEADER_ARG("SizeInBytes=%d", SizeInBytes);

    if (getcwd(Buffer, SizeInBytes))
    {
        gcmFOOTER_ARG("Buffer=%s", Buffer);
        return gcvSTATUS_NOT_SUPPORTED;
    }
    else
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }
}

/* Get file status info. */
gceSTATUS
gcoOS_Stat(
    IN gcoOS Os,
    IN gctCONST_STRING FileName,
    OUT gctPOINTER Buffer
    )
{
    gcmHEADER_ARG("FileName=%s", gcmOPT_STRING(FileName));

    if (stat(FileName, Buffer) == 0)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_NOT_SUPPORTED);
        return gcvSTATUS_NOT_SUPPORTED;
    }
}

/*******************************************************************************
**
**  gcoOS_GetPos
**
**  Get the current position of a file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**  OUTPUT:
**
**      gctUINT32 * Position
**          Pointer to a variable receiving the current position of the file.
*/
gceSTATUS
gcoOS_GetPos(
    IN gcoOS Os,
    IN gctFILE File,
    OUT gctUINT32 * Position
    )
{
    gcmHEADER_ARG("File=0x%x", File);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Position != gcvNULL);

    /* Get the current file position. */
    *Position = ftell((FILE *) File);

    /* Success. */
    gcmFOOTER_ARG("*Position=%u", *Position);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_SetPos
**
**  Set position for a file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctUINT32 Position
**          Absolute position of the file to set.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_SetPos(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctUINT32 Position
    )
{
    gcmHEADER_ARG("File=0x%x Position=%u", File, Position);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);

    /* Set file position. */
    fseek((FILE *) File, Position, SEEK_SET);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_Seek
**
**  Set position for a file.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctFILE File
**          Pointer to an open file object.
**
**      gctUINT32 Offset
**          Offset added to the position specified by Whence.
**
**      gceFILE_WHENCE Whence
**          Mode that specify how to add the offset to the position:
**
**              gcvFILE_SEEK_SET    - Relative to the start of the file.
**              gcvFILE_SEEK_CUR    - Relative to the current position.
**              gcvFILE_SEEK_END    - Relative to the end of the file.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Seek(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctUINT32 Offset,
    IN gceFILE_WHENCE Whence
    )
{
    gctINT result = 0;

    gcmHEADER_ARG("File=0x%x Offset=%u Whence=%d",
                  File, Offset, Whence);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(File != gcvNULL);

    /* Set file position. */
    switch (Whence)
    {
    case gcvFILE_SEEK_SET:
        result = fseek((FILE *) File, Offset, SEEK_SET);
        break;

    case gcvFILE_SEEK_CUR:
        result = fseek((FILE *) File, Offset, SEEK_CUR);
        break;

    case gcvFILE_SEEK_END:
        result = fseek((FILE *) File, Offset, SEEK_END);
        break;
    }

    if (result == 0)
    {
        /* Success. */
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        /* Error */
        gcmFOOTER_ARG("status=%d", gcvSTATUS_GENERIC_IO);
        return gcvSTATUS_GENERIC_IO;
    }
}

/* Get environment variable value. */
gceSTATUS
gcoOS_GetEnv(
    IN gcoOS Os,
    IN gctCONST_STRING VarName,
    OUT gctSTRING * Value
    )
{
    gcmHEADER_ARG("VarName=%s", gcmOPT_STRING(VarName));

    *Value = getenv(VarName);

    /* Success. */
    gcmFOOTER_ARG("*Value=%s", gcmOPT_STRING(*Value));
    return gcvSTATUS_OK;
}

/* Set environment variable value. */
gceSTATUS gcoOS_SetEnv(
    IN gcoOS Os,
    IN gctCONST_STRING VarName,
    IN gctSTRING Value
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_CreateThread
**
**  Create a new thread.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Thread
**          Pointer to a variable that will hold a pointer to the thread.
*/
gceSTATUS
gcoOS_CreateThread(
    IN gcoOS Os,
    IN gcTHREAD_ROUTINE Worker,
    IN gctPOINTER Argument,
    OUT gctPOINTER * Thread
    )
{
    pthread_t thread;

    pthread_attr_t attr;

    pthread_attr_init(&attr );
    pthread_attr_setstacksize(&attr, 262144);

    gcmHEADER_ARG("Worker=0x%x Argument=0x%x", Worker, Argument);

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Thread != gcvNULL);

    if (pthread_create(&thread, &attr, Worker, Argument) != 0)
    {
        gcmFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    *Thread = gcmINT2PTR(thread);

    /* Success. */
    gcmFOOTER_ARG("*Thread=0x%x", *Thread);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_CloseThread
**
**  Close a thread.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER Thread
**          Pointer to the thread to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_CloseThread(
    IN gcoOS Os,
    IN gctPOINTER Thread
    )
{
    gcmHEADER_ARG("Thread=0x%x", Thread);

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Thread != gcvNULL);

    pthread_join((pthread_t)gcmPTR2INT32(Thread), gcvNULL);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_CreateMutex
**
**  Create a new mutex.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Mutex
**          Pointer to a variable that will hold a pointer to the mutex.
*/
gceSTATUS
gcoOS_CreateMutex(
    IN gcoOS Os,
    OUT gctPOINTER * Mutex
    )
{
    gceSTATUS status;
    pthread_mutex_t* mutex;
    pthread_mutexattr_t mattr;
    int rc;

    gcmHEADER();

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Allocate memory for the mutex. */
    gcmONERROR(gcoOS_Allocate(
        gcvNULL, gcmSIZEOF(pthread_mutex_t), (gctPOINTER *) &mutex
        ));

    /* Initialize the mutex. */
    rc = pthread_mutexattr_init(&mattr);
    if (rc != EOK)
    {
        gcoOS_Free(gcvNULL, mutex);
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    rc = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    if (rc != EOK)
    {
        pthread_mutexattr_destroy(&mattr);
        gcoOS_Free(gcvNULL, mutex);
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    rc = pthread_mutex_init(mutex, &mattr);
    if (rc != EOK)
    {
        pthread_mutexattr_destroy(&mattr);
        gcoOS_Free(gcvNULL, mutex);
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    /* We do not need the attribute any more. */
    pthread_mutexattr_destroy(&mattr);

    /* Return mutex to caller. */
    *Mutex = (gctPOINTER) mutex;

    /* Success. */
    gcmFOOTER_ARG("*Mutex = 0x%x", *Mutex);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_DeleteMutex
**
**  Delete a mutex.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_DeleteMutex(
    IN gcoOS Os,
    IN gctPOINTER Mutex
    )
{
    pthread_mutex_t *mutex;

    gcmHEADER_ARG("Mutex=0x%x", Mutex);

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Cast the pointer. */
    mutex = (pthread_mutex_t *) Mutex;

    /* Destroy the mutex. */
    pthread_mutex_destroy(mutex);

    /* Free the memory. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, mutex));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_AcquireMutex
**
**  Acquire a mutex.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be acquired.
**
**      gctUINT32 Timeout
**          Timeout value specified in milliseconds.
**          Specify the value of gcvINFINITE to keep the thread suspended
**          until the mutex has been acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_AcquireMutex(
    IN gcoOS Os,
    IN gctPOINTER Mutex,
    IN gctUINT32 Timeout
    )
{
    gceSTATUS status;
    struct timespec tv;
    gctUINT64 nanos;
    pthread_mutex_t *mutex;
    gctINT rc;

    gcmHEADER_ARG("Mutex=0x%x Timeout=%u", Mutex, Timeout);

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Cast the pointer. */
    mutex = (pthread_mutex_t *) Mutex;

    /* Test for infinite. */
    if (Timeout == gcvINFINITE)
    {
        /* Lock the mutex. */
        rc = pthread_mutex_lock(mutex);
    }
    else if (Timeout == 0)
    {
        /* Attempt to lock the mutex. */
        rc = pthread_mutex_trylock(mutex);

        /* Maybe the mutex was already locked. */
        if (rc == EBUSY)
        {
            rc = ETIMEDOUT;
        }
    }
    else
    {
        /* Get current time. */
        clock_gettime(CLOCK_MONOTONIC, &tv);

        /* Compute absolute time. */
        nanos  = Timeout;
        nanos *= 1000000ULL;
        nanos += timespec2nsec(&tv);
        nsec2timespec(&tv, nanos);

        /* Attempt to lock the mutex. */
        rc = pthread_mutex_timedlock_monotonic(mutex, &tv);
    }

    if (rc != EOK)
    {
        if (rc == ETIMEDOUT)
        {
            gcmONERROR(gcvSTATUS_TIMEOUT);
        }
        else
        {
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_ReleaseMutex
**
**  Release an acquired mutex.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be released.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_ReleaseMutex(
    IN gcoOS Os,
    IN gctPOINTER Mutex
    )
{
    pthread_mutex_t *mutex;

    gcmHEADER_ARG("Mutex=0x%x", Mutex);

    /* Validate the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Cast the pointer. */
    mutex = (pthread_mutex_t *) Mutex;

    /* Release the mutex. */
    pthread_mutex_unlock(mutex);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_Delay
**
**  Delay execution of the current thread for a number of milliseconds.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctUINT32 Delay
**          Delay to sleep, specified in milliseconds.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Delay(
    IN gcoOS Os,
    IN gctUINT32 Delay
    )
{
    gcmHEADER_ARG("Delay=%u", Delay);

    /* Sleep for a while. */
    usleep((Delay == 0) ? 1 : (1000 * Delay));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Same semantics as strstr. */
gceSTATUS
gcoOS_StrStr(
    IN gctCONST_STRING String,
    IN gctCONST_STRING SubString,
    OUT gctSTRING * Output
    )
{
    gctCHAR* pos;
    gceSTATUS status;

    gcmHEADER_ARG("String=0x%x SubString=0x%x", String, SubString);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(SubString != gcvNULL);

    /* Call C. */
    pos = strstr(String, SubString);
    if (Output)
    {
        *Output = pos;
    }
    status = pos ? gcvSTATUS_TRUE : gcvSTATUS_FALSE;

    /* Success. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_StrFindReverse(
    IN gctCONST_STRING String,
    IN gctINT8 Character,
    OUT gctSTRING * Output
    )
{
    gcmHEADER_ARG("String=0x%x Character=%d", String, Character);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Output != gcvNULL);

    /* Call C. */
    *Output = strrchr(String, Character);

    /* Success. */
    gcmFOOTER_ARG("*Output=0x%x", *Output);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_StrCopy
**
**  Copy a string.
**
**  INPUT:
**
**      gctSTRING Destination
**          Pointer to the destination string.
**
**      gctCONST_STRING Source
**          Pointer to the source string.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_StrCopySafe(
    IN gctSTRING Destination,
    IN gctSIZE_T DestinationSize,
    IN gctCONST_STRING Source
    )
{
    gcmHEADER_ARG("Destination=0x%x DestinationSize=%lu Source=0x%x",
                  Destination, DestinationSize, Source);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Destination != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Source != gcvNULL);

    /* Don't overflow the destination buffer. */
    strncpy(Destination, Source, DestinationSize - 1);

    /* Put this there in case the strncpy overflows. */
    Destination[DestinationSize - 1] = '\0';

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_StrCat
**
**  Append a string.
**
**  INPUT:
**
**      gctSTRING Destination
**          Pointer to the destination string.
**
**      gctCONST_STRING Source
**          Pointer to the source string.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_StrCatSafe(
    IN gctSTRING Destination,
    IN gctSIZE_T DestinationSize,
    IN gctCONST_STRING Source
    )
{
    gctSIZE_T n;

    gcmHEADER_ARG("Destination=0x%x DestinationSize=%lu Source=0x%x",
                  Destination, DestinationSize, Source);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Destination != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Source != gcvNULL);

    /* Find the end of the destination. */
    n = strlen(Destination);
    if (n + 1 < DestinationSize)
    {
        /* Append the string but don't overflow the destination buffer. */
        strncpy(Destination + n, Source, DestinationSize - n - 1);

        /* Put this there in case the strncpy overflows. */
        Destination[DestinationSize - 1] = '\0';
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_StrCmp
**
**  Compare two strings and return whether they match or not.
**
**  INPUT:
**
**      gctCONST_STRING String1
**          Pointer to the first string to compare.
**
**      gctCONST_STRING String2
**          Pointer to the second string to compare.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK if the strings match
**      gcvSTATUS_LARGER if String1 > String2
**      gcvSTATUS_SMALLER if String1 < String2
*/
gceSTATUS
gcoOS_StrCmp(
    IN gctCONST_STRING String1,
    IN gctCONST_STRING String2
    )
{
    int result;
    gceSTATUS status;

    gcmHEADER_ARG("String1=0x%x String2=0x%x", String1, String2);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(String1 != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(String2 != gcvNULL);

    /* Compare the strings and return proper status. */
    result = strcmp(String1, String2);

    status = (result == 0) ? gcvSTATUS_OK
            : (result > 0) ? gcvSTATUS_LARGER
                           : gcvSTATUS_SMALLER;

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_StrNCmp
**
**  Compare characters of two strings and return whether they match or not.
**
**  INPUT:
**
**      gctCONST_STRING String1
**          Pointer to the first string to compare.
**
**      gctCONST_STRING String2
**          Pointer to the second string to compare.
**
**      gctSIZE_T Count
**          Number of characters to compare.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK if the strings match
**      gcvSTATUS_LARGER if String1 > String2
**      gcvSTATUS_SMALLER if String1 < String2
*/
gceSTATUS
gcoOS_StrNCmp(
    IN gctCONST_STRING String1,
    IN gctCONST_STRING String2,
    IN gctSIZE_T Count
    )
{
    int result;
    gceSTATUS status;

    gcmHEADER_ARG("String1=0x%x String2=0x%x Count=%lu",
                  String1, String2, Count);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(String1 != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(String2 != gcvNULL);

    /* Compare the strings and return proper status. */
    result = strncmp(String1, String2, Count);

    status = (result == 0)
            ? gcvSTATUS_OK
            : ((result > 0) ? gcvSTATUS_LARGER : gcvSTATUS_SMALLER);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_StrToFloat
**
**  Convert string to float.
**
**  INPUT:
**
**      gctCONST_STRING String
**          Pointer to the string to be converted.
**
**
**  OUTPUT:
**
**      gctFLOAT * Float
**          Pointer to a variable that will receive the float.
**
*/
gceSTATUS
gcoOS_StrToFloat(
    IN gctCONST_STRING String,
    OUT gctFLOAT * Float
    )
{
    gcmHEADER_ARG("String=%s", gcmOPT_STRING(String));

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);

    *Float = (gctFLOAT)atof(String);

    gcmFOOTER_ARG("*Float=%f", *Float);
    return gcvSTATUS_OK;
}

/* Converts a hex string to 32-bit integer. */
gceSTATUS gcoOS_HexStrToInt(IN gctCONST_STRING String,
               OUT gctINT * Int)
{
    gcmHEADER_ARG("String=%s", gcmOPT_STRING(String));
    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Int != gcvNULL);

    sscanf(String, "%x", Int);

    gcmFOOTER_ARG("*Int=%d", *Int);
    return gcvSTATUS_OK;
}

/* Converts a hex string to float. */
gceSTATUS gcoOS_HexStrToFloat(IN gctCONST_STRING String,
               OUT gctFLOAT * Float)
{
    gctSTRING pch = gcvNULL;
    gctCONST_STRING delim = "x.p";
    gctFLOAT b=0.0, exp=0.0;
    gctINT s=0;

    gcmHEADER_ARG("String=%s", gcmOPT_STRING(String));
    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Float != gcvNULL);

    pch = strtok((gctSTRING)String, delim);
    if (pch == NULL) goto onError;

    pch = strtok(NULL, delim);
    if (pch == NULL) goto onError;
    gcmVERIFY_OK(gcoOS_StrToFloat(pch, &b));

    pch = strtok(NULL, delim);
    if (pch == NULL) goto onError;
    gcmVERIFY_OK(gcoOS_HexStrToInt(pch, &s));

    pch = strtok(NULL, delim);
    if (pch == NULL) goto onError;
    gcmVERIFY_OK(gcoOS_StrToFloat(pch, &exp));

    *Float = (float)(b + s / (float)(1 << 24)) * (float)pow(2.0, exp);

    gcmFOOTER_ARG("*Float=%d", *Float);
    return gcvSTATUS_OK;

onError:
    gcmFOOTER_NO();
    return gcvSTATUS_INVALID_ARGUMENT;
}

/*******************************************************************************
**
**  gcoOS_StrToInt
**
**  Convert string to integer.
**
**  INPUT:
**
**      gctCONST_STRING String
**          Pointer to the string to be converted.
**
**
**  OUTPUT:
**
**      gctINT * Int
**          Pointer to a variable that will receive the integer.
**
*/
gceSTATUS
gcoOS_StrToInt(
    IN gctCONST_STRING String,
    OUT gctINT * Int
    )
{
    gcmHEADER_ARG("String=%s", gcmOPT_STRING(String));

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);

    *Int = (gctINT)atoi(String);

    gcmFOOTER_ARG("*Int=%d", *Int);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_MemCmp
**
**  Compare two memory regions and return whether they match or not.
**
**  INPUT:
**
**      gctCONST_POINTER Memory1
**          Pointer to the first memory region to compare.
**
**      gctCONST_POINTER Memory2
**          Pointer to the second memory region to compare.
**
**      gctSIZE_T Bytes
**          Number of bytes to compare.
**
**  OUTPUT:
**
**      Nothing.
**
**  RETURNS:
**
**      gcvSTATUS_OK if the memory regions match or gcvSTATUS_MISMATCH if the
**      memory regions don't match.
*/
gceSTATUS
gcoOS_MemCmp(
    IN gctCONST_POINTER Memory1,
    IN gctCONST_POINTER Memory2,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Memory1=0x%x Memory2=0x%x Bytes=%lu",
                  Memory1, Memory2, Bytes);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Memory1 != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Memory2 != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Bytes > 0);

    /* Compare the memory rregions and return proper status. */
    status = (memcmp(Memory1, Memory2, Bytes) == 0)
               ? gcvSTATUS_OK
               : gcvSTATUS_MISMATCH;
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_PrintStr
**
**  Append a "printf" formatted string to a string buffer and adjust the offset
**  into the string buffer.  There is no checking for a buffer overflow, so make
**  sure the string buffer is large enough.
**
**  INPUT:
**
**      gctSTRING String
**          Pointer to the string buffer.
**
**      gctUINT_PTR Offset
**          Pointer to a variable that holds the current offset into the string
**          buffer.
**
**      gctCONST_STRING Format
**          Pointer to a "printf" style format to append to the string buffer
**          pointet to by <String> at the offset specified by <*Offset>.
**
**      ...
**          Variable number of arguments that will be used by <Format>.
**
**  OUTPUT:
**
**      gctUINT_PTR Offset
**          Pointer to a variable that receives the new offset into the string
**          buffer pointed to by <String> after the formatted string pointed to
**          by <Formnat> has been appended to it.
*/
gceSTATUS
gcoOS_PrintStrSafe(
    IN gctSTRING String,
    IN gctSIZE_T StringSize,
    IN OUT gctUINT_PTR Offset,
    IN gctCONST_STRING Format,
    ...
    )
{
    gctARGUMENTS arguments;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("String=0x%x StringSize=%lu *Offset=%u Format=0x%x",
                  String, StringSize, gcmOPT_VALUE(Offset), Format);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(StringSize > 0);
    gcmVERIFY_ARGUMENT(Format != gcvNULL);

    /* Route through gcoOS_PrintStrVSafe. */
    gcmARGUMENTS_START(arguments, Format);
    gcmONERROR(gcoOS_PrintStrVSafe(String, StringSize,
                                   Offset,
                                   Format, arguments));

OnError:
    gcmARGUMENTS_END(arguments);

    gcmFOOTER_ARG("*Offset=%u", gcmOPT_VALUE(Offset));
    return status;
}

/*******************************************************************************
**
**  gcoOS_PrintStrV
**
**  Append a "vprintf" formatted string to a string buffer and adjust the offset
**  into the string buffer.  There is no checking for a buffer overflow, so make
**  sure the string buffer is large enough.
**
**  INPUT:
**
**      gctSTRING String
**          Pointer to the string buffer.
**
**      gctUINT_PTR Offset
**          Pointer to a variable that holds the current offset into the string
**          buffer.
**
**      gctCONST_STRING Format
**          Pointer to a "printf" style format to append to the string buffer
**          pointet to by <String> at the offset specified by <*Offset>.
**
**      gctPOINTER ArgPtr
**          Pointer to list of arguments.
**
**  OUTPUT:
**
**      gctUINT_PTR Offset
**          Pointer to a variable that receives the new offset into the string
**          buffer pointed to by <String> after the formatted string pointed to
**          by <Formnat> has been appended to it.
*/
gceSTATUS
gcoOS_PrintStrVSafe(
    OUT gctSTRING String,
    IN gctSIZE_T StringSize,
    IN OUT gctUINT_PTR Offset,
    IN gctCONST_STRING Format,
    IN gctARGUMENTS Arguments
    )
{
    gctUINT offset = gcmOPT_VALUE(Offset);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("String=0x%x StringSize=%lu *Offset=%u Format=0x%x Arguments=0x%x",
                  String, StringSize, offset, Format, Arguments);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(String != gcvNULL);
    gcmVERIFY_ARGUMENT(StringSize > 0);
    gcmVERIFY_ARGUMENT(Format != gcvNULL);

    if (offset < StringSize - 1)
    {
        /* Print into the string. */
        gctINT n = vsnprintf(String + offset,
                             StringSize - offset,
                             Format,
                             Arguments);

        if (n < 0 || n >= (gctINT)(StringSize - offset))
        {
            status = gcvSTATUS_GENERIC_IO;
        }
        else if (Offset)
        {
            *Offset = offset + n;
        }
    }
    else
    {
        status = gcvSTATUS_BUFFER_TOO_SMALL;
    }

    /* Success. */
    gcmFOOTER_ARG("*Offset=%u", offset);
    return status;
}

/*******************************************************************************
**
**  gcoOS_MapUserMemory
**
**  This function is QNX only and for backwards compatibility, any new code must not use it.
**
**  Lock down a user buffer and return an DMA'able address to be used by the
**  hardware to access it.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory to lock down.
**
**      gctSIZE_T Size
**          Size in bytes of the memory to lock down.
**
**  OUTPUT:
**
**      gctPOINTER * Info
**          Pointer to variable receiving the information record required by
**          gcoOS_UnmapUserMemory.
**
**      gctUINT32_PTR Address
**          Pointer to a variable that will receive the address DMA'able by the
**          hardware.
*/
gceSTATUS
gcoOS_MapUserMemory(
    IN gcoOS Os,
    IN gctPOINTER Memory,
    IN gctSIZE_T Size,
    OUT gctPOINTER * Info,
    OUT gctUINT32_PTR Address
    )
{
    gceSTATUS status;
    gcsUSER_MEMORY_DESC desc;
    gctUINT32 node = 0;

    gcoOS_ZeroMemory(&desc, gcmSIZEOF(desc));

    desc.flag    = gcvALLOC_FLAG_USERMEMORY;
    desc.logical = gcmPTR_TO_UINT64(Memory);
    desc.size    = Size;
    desc.physical = ~0ULL;

    gcmONERROR(gcoHAL_WrapUserMemory(&desc, gcvVIDMEM_TYPE_BITMAP, &node));

    gcmONERROR(gcoHAL_LockVideoMemory(node, gcvFALSE, gcvENGINE_RENDER, Address, gcvNULL));

    *Info = (gctPOINTER)(gctUINTPTR_T)node;

    return gcvSTATUS_OK;

OnError:
    if (node)
    {
        gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));
    }

    return status;
}

/*******************************************************************************
**
**  gcoOS_UnmapUserMemory
**
**  This function is QNX only and for backwards compatibility, any new code must not use it.
**
**  Unlock a user buffer and that was previously locked down by
**  gcoOS_MapUserMemory.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory to unlock.
**
**      gctSIZE_T Size
**          Size in bytes of the memory to unlock.
**
**      gctPOINTER Info
**          Information record returned by gcoOS_MapUserMemory.
**
**      gctUINT32_PTR Address
**          The address returned by gcoOS_MapUserMemory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_UnmapUserMemory(
    IN gcoOS Os,
    IN gctPOINTER Memory,
    IN gctSIZE_T Size,
    IN gctPOINTER Info,
    IN gctUINT32 Address
    )
{
    gctUINT32 node;

    node = (gctUINT32)(gctUINTPTR_T) Info;

    gcmVERIFY_OK(gcoHAL_UnlockVideoMemory(node, gcvSURF_BITMAP, gcvENGINE_RENDER));

    gcmVERIFY_OK(gcoHAL_ReleaseVideoMemory(node));

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_StrDup
**
**  Duplicate the given string by copying it into newly allocated memory.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctCONST_STRING String
**          Pointer to string to duplicate.
**
**  OUTPUT:
**
**      gctSTRING * Target
**          Pointer to variable holding the duplicated string address.
*/
gceSTATUS
gcoOS_StrDup(
    IN gcoOS Os,
    IN gctCONST_STRING String,
    OUT gctSTRING * Target
    )
{
    gctSIZE_T bytes;
    gctSTRING string;
    gceSTATUS status;

    gcmHEADER_ARG("String=0x%x", String);

    gcmDEBUG_VERIFY_ARGUMENT(String != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Target != gcvNULL);

    bytes = gcoOS_StrLen(String, gcvNULL);

    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes + 1, (gctPOINTER *) &string));

    memcpy(string, String, bytes + 1);

    *Target = string;

    /* Success. */
    gcmFOOTER_ARG("*Target=0x%x", gcmOPT_VALUE(Target));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_LoadLibrary
**
**  Load a library.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctCONST_STRING Library
**          Name of library to load.
**
**  OUTPUT:
**
**      gctHANDLE * Handle
**          Pointer to variable receiving the library handle.
*/
gceSTATUS
gcoOS_LoadLibrary(
    IN gcoOS Os,
    IN gctCONST_STRING Library,
    OUT gctHANDLE * Handle
    )
{
#if gcdSTATIC_LINK
    return gcvSTATUS_NOT_SUPPORTED;
#else
    gctSIZE_T length;
    gctSTRING library = gcvNULL;
    gceSTATUS status = gcvSTATUS_NOT_FOUND;
    gctCHAR buf[512] = "";

    gcmHEADER_ARG("Library=%s", gcmOPT_STRING(Library));

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Handle != gcvNULL);

    /* Reset the handle. */
    *Handle = gcvNULL;

    if (Library == gcvNULL)
    {
        /* This function isn't very useful unless you give it a library name. */
        status = gcvSTATUS_NOT_FOUND;
        goto OnError;
    }

    if (*Handle == gcvNULL)
    {
        int rc = __khrGetDisplayConfigValue(1, Library, buf, sizeof(buf));
        if (rc == EOK)
        {
            *Handle = __khrLoadLibraryString(buf);
        }
    }

    if (*Handle == gcvNULL)
    {
        /* Try loading the name directly */
        /* Get the length of the library name. */
        length = strlen(Library);

        /* Allocate temporary string buffer. */
        gcmONERROR(gcoOS_Allocate(
            gcvNULL, length + 3 + 1, (gctPOINTER *) &library
            ));

        /* Copy the library name to the temporary string buffer. */
        strcpy(library, Library);

        /* Try to load it. */
        *Handle = __khrLoadLibraryString(library);

        if (*Handle == gcvNULL)
        {
            /* Test if the library has ".so" at the end. */
            if (strcmp(Library + length - 3, ".so") != 0)
            {
                /* Append the ".so" to the temporary string buffer. */
                strcat(library, ".so");

                /* And try again */
                *Handle = __khrLoadLibraryString(library);
            }
        }
    }

    /* Free the temporary string buffer. */
    if (library != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, library));
    }

    /* Return error if library could not be loaded. */
    status = (*Handle == gcvNULL) ? gcvSTATUS_NOT_FOUND : gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("*Handle=0x%x status=%d", *Handle, status);
    return status;
#endif
}

/*******************************************************************************
**
**  gcoOS_FreeLibrary
**
**  Unload a loaded library.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctHANDLE Handle
**          Handle of a loaded libarry.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_FreeLibrary(
    IN gcoOS Os,
    IN gctHANDLE Handle
    )
{
#if gcdSTATIC_LINK
    return gcvSTATUS_NOT_SUPPORTED;
#else
    gcmHEADER_ARG("Handle=0x%x", Handle);

    /* Free the library. */
    dlclose(Handle);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
#endif
}

/*******************************************************************************
**
**  gcoOS_GetProcAddress
**
**  Get the address of a function inside a loaded library.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctHANDLE Handle
**          Handle of a loaded libarry.
**
**      gctCONST_STRING Name
**          Name of function to get the address of.
**
**  OUTPUT:
**
**      gctPOINTER * Function
**          Pointer to variable receiving the function pointer.
*/
gceSTATUS
gcoOS_GetProcAddress(
    IN gcoOS Os,
    IN gctHANDLE Handle,
    IN gctCONST_STRING Name,
    OUT gctPOINTER * Function
    )
{
#if gcdSTATIC_LINK
    return gcvSTATUS_NOT_SUPPORTED;
#else
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Handle=0x%x Name=%s", Handle, gcmOPT_STRING(Name));

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Name != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(Function != gcvNULL);

    /* Get the address of the function. */
    *Function = dlsym(Handle, Name);

    if (*Function == gcvNULL)
    {
        gcmTRACE(
            gcvLEVEL_WARNING,
            "%s(%d): Function %s not found.",
            __FUNCTION__, __LINE__, Name
            );

        /* Function could not be found. */
        status = gcvSTATUS_NOT_FOUND;
    }

    /* Success. */
    gcmFOOTER_ARG("*Function=0x%x status=%d", *Function, status);
    return status;
#endif
}

#if VIVANTE_PROFILER_SYSTEM_MEMORY
gceSTATUS
gcoOS_ProfileStart(
    IN gcoOS Os
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_ProfileEnd(
    IN gcoOS Os,
    IN gctCONST_STRING Title
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_SetProfileSetting
**
**  Set Vivante profiler settings.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctBOOL Enable
**          Enable or Disable Vivante profiler.
**
**      gctCONST_STRING FileName
**          Specify FileName for storing profile data into.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_SetProfileSetting(
    IN gcoOS Os,
    IN gctBOOL Enable,
    IN gctCONST_STRING FileName
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

/*  if (strlen(FileName) >= gcmMAX_PROFILE_FILE_NAME)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }*/

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_SET_PROFILE_SETTING;
    iface.u.SetProfileSetting.enable = Enable;

        /* Call the kernel. */
    status = gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        );

    if (gcmIS_ERROR(status))
    {
        return status;
    }

    return iface.status;
}
#endif

gceSTATUS
gcoOS_Compact(
    IN gcoOS Os
    )
{
    return gcvSTATUS_OK;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------- Atoms ----------------------------------*/
/* Create an atom. */
gceSTATUS
gcoOS_AtomConstruct(
    IN gcoOS Os,
    OUT gcsATOM_PTR * Atom
    )
{
    gceSTATUS status;
    gcsATOM_PTR atom = gcvNULL;

    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Atom != gcvNULL);

    do
    {
        /* Allocate memory for the atom. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(struct gcsATOM),
                                    (gctPOINTER *) &atom));

        /* Initialize the atom to 0. */
        atom->counter = 0;

        /* Return pointer to atom. */
        *Atom = atom;

        /* Success. */
        gcmFOOTER_ARG("*Atom=%p", *Atom);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Free the atom. */
    if (atom != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, atom);
    }

    /* Return error status. */
    gcmFOOTER();
    return status;
}

/* Destroy an atom. */
gceSTATUS
gcoOS_AtomDestroy(
    IN gcoOS Os,
    IN gcsATOM_PTR Atom
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Atom=0x%x", Atom);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Atom != gcvNULL);

    /* Free the atom. */
    status = gcmOS_SAFE_FREE(gcvNULL, Atom);

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_AtomGet(
    IN gcoOS Os,
    IN gcsATOM_PTR Atom,
    OUT gctINT32_PTR Value
    )
{
    gcmHEADER_ARG("Atom=0x%0x", Atom);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Get the atom value. */
    *Value = Atom->counter;

    /* Success. */
    gcmFOOTER_ARG("*Value=%d", *Value);
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_AtomSet(
    IN gcoOS Os,
    IN gcsATOM_PTR Atom,
    IN gctINT32 Value
    )
{
    gcmHEADER_ARG("Atom=0x%0x Value=%d", Atom, Value);

    /* Verify the arguments. */
    gcmVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Set the atom value. */
    Atom->counter = Value;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Increment an atom. */
gceSTATUS
gcoOS_AtomIncrement(
    IN gcoOS Os,
    IN gcsATOM_PTR Atom,
    OUT gctINT32_PTR OldValue OPTIONAL
    )
{
    gctINT32 value;

    gcmHEADER_ARG("Atom=0x%x", Atom);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Atom != gcvNULL);

    /* Increment the atom's counter. */
    value = atomic_add_value((gctUINT32_PTR)&Atom->counter, 1);

    if (OldValue != gcvNULL)
    {
        /* Return the original value to the caller. */
        *OldValue = value;
    }

    /* Success. */
    gcmFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/* Decrement an atom. */
gceSTATUS
gcoOS_AtomDecrement(
    IN gcoOS Os,
    IN gcsATOM_PTR Atom,
    OUT gctINT32_PTR OldValue OPTIONAL
    )
{
    gctINT32 value;

    gcmHEADER_ARG("Atom=0x%x", Atom);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Atom != gcvNULL);

    /* Decrement the atom's counter. */
    value = atomic_sub_value((gctUINT32_PTR)&Atom->counter, 1);

    if (OldValue != gcvNULL)
    {
        /* Return the original value to the caller. */
        *OldValue = value;
    }

    /* Success. */
    gcmFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

gctHANDLE
gcoOS_GetCurrentProcessID(
    void
    )
{
    return (gctHANDLE)gcmINT2PTR(getpid());
}

gctHANDLE
gcoOS_GetCurrentThreadID(
    void
    )
{
    return (gctHANDLE)gcmINT2PTR( pthread_self());
}

/*----------------------------------------------------------------------------*/
/*----------------------------------- Time -----------------------------------*/

/*******************************************************************************
**
**  gcoOS_GetTicks
**
**  Get the number of milliseconds since the system started.
**
**  INPUT:
**
**  OUTPUT:
**
*/
gctUINT32
gcoOS_GetTicks(
    void
    )
{
    struct timeval tv;

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*******************************************************************************
**
**  gcoOS_GetTime
**
**  Get the number of microseconds since 1970/1/1.
**
**  INPUT:
**
**  OUTPUT:
**
**      gctUINT64_PTR Time
**          Pointer to a variable to get time.
**
*/
gceSTATUS
gcoOS_GetTime(
    OUT gctUINT64_PTR Time
    )
{
    struct timeval tv;

    /* Return the time of day in microseconds. */
    gettimeofday(&tv, 0);
    *Time = (tv.tv_sec * 1000000) + tv.tv_usec;
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_GetCPUTime
**
**  Get CPU time usage in microseconds.
**
**  INPUT:
**
**  OUTPUT:
**
**      gctUINT64_PTR CPUTime
**          Pointer to a variable to get CPU time usage.
**
*/
gceSTATUS
gcoOS_GetCPUTime(
    OUT gctUINT64_PTR CPUTime
    )
{
    /* Return CPU time in microseconds. */
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoOS_GetMemoryUsage
**
**  Get current processes resource usage.
**
**  INPUT:
**
**  OUTPUT:
**
**      gctUINT32_PTR MaxRSS
**          Total amount of resident set memory used.
**          The value will be in terms of memory pages used.
**
**      gctUINT32_PTR IxRSS
**          Total amount of memory used by the text segment
**          in kilobytes multiplied by the execution-ticks.
**
**      gctUINT32_PTR IdRSS
**          Total amount of private memory used by a process
**          in kilobytes multiplied by execution-ticks.
**
**      gctUINT32_PTR IsRSS
**          Total amount of memory used by the stack in
**          kilobytes multiplied by execution-ticks.
**
*/
gceSTATUS
gcoOS_GetMemoryUsage(
    OUT gctUINT32_PTR MaxRSS,
    OUT gctUINT32_PTR IxRSS,
    OUT gctUINT32_PTR IdRSS,
    OUT gctUINT32_PTR IsRSS
    )
{
    /* Return memory usage. */
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gcoOS_ReadRegister
**
**  Read data from a register.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctUINT32 Address
**          Address of register.
**
**  OUTPUT:
**
**      gctUINT32 * Data
**          Pointer to a variable that receives the data read from the register.
*/
gceSTATUS
gcoOS_ReadRegister(
    IN gcoOS Os,
    IN gctUINT32 Address,
    OUT gctUINT32 * Data
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("Address=0x%x", Address);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Data != gcvNULL);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_READ_REGISTER;
    iface.u.ReadRegisterData.address = Address;
    iface.u.ReadRegisterData.data    = 0xDEADDEAD;

    /* Call kernel driver. */
    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    /* Return the Data on success. */
    *Data = iface.u.ReadRegisterData.data;

    /* Success. */
    gcmFOOTER_ARG("*Data=0x%08x", *Data);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_WriteRegister
**
**  Write data to a register.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctUINT32 Address
**          Address of register.
**
**      gctUINT32 Data
**          Data for register.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_WriteRegister(
    IN gcoOS Os,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER_ARG("Address=0x%x Data=0x%08x", Address, Data);

    /* Initialize the gcsHAL_INTERFACE structure. */
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_WRITE_REGISTER;
    iface.u.WriteRegisterData.address = Address;
    iface.u.WriteRegisterData.data    = Data;

    /* Call kernel driver. */
    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheClean
**
**  Clean the cache for the specified addresses.  The GPU is going to need the
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctUINT32 Node
**          Pointer to the video memory node that needs to be flushed.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gcoOS_CacheClean(
    IN gcoOS Os,
    IN gctUINT32 Node,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u",
                  Node, Logical, Bytes);

    /* Call common code. */
    gcmONERROR(_CacheRangeFlush(Node, Logical, Bytes, gcvCACHE_CLEAN));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheFlush
**
**  Flush the cache for the specified addresses and invalidate the lines as
**  well.  The GPU is going to need and modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctUINT32 Node
**          Pointer to the video memory node that needs to be flushed.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gcoOS_CacheFlush(
    IN gcoOS Os,
    IN gctUINT32 Node,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u",
                  Node, Logical, Bytes);

    /* Call common code. */
    gcmONERROR(_CacheRangeFlush(Node, Logical, Bytes, gcvCACHE_FLUSH));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**  gcoOS_CacheInvalidate
**
**  Invalidate the lines. The GPU is going modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gcoOS Os
**          Pointer to gcoOS object.
**
**      gctUINT32 Node
**          Pointer to the video memory node that needs to be invalidated.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to invalidated.
*/
gceSTATUS
gcoOS_CacheInvalidate(
    IN gcoOS Os,
    IN gctUINT32 Node,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Node=0x%x Logical=0x%x Bytes=%u",
                  Node, Logical, Bytes);

    /* Call common code. */
    gcmONERROR(_CacheRangeFlush(Node, Logical, Bytes, gcvCACHE_INVALIDATE));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*----------------------------------------------------------------------------*/
/*----- Profiling ------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#if gcdENABLE_PROFILING
gceSTATUS
gcoOS_GetProfileTick(
    OUT gctUINT64_PTR Tick
    )
{
    struct timespec tv;

    clock_gettime(CLOCK_MONOTONIC, &tv);

    *Tick = timespec2nsec(&tv);

    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_QueryProfileTickRate(
    OUT gctUINT64_PTR TickRate
    )
{
    *TickRate = 0;

    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**  gcoOS_ProfileDB
**
**  Manage the profile database.
**
**  The database layout is very simple:
**
**      <RecordID> (1 byte) <record data>
**
**  The <RecordID> can be one of the following values:
**
**      1       Initialize a new function to be profiled. Followed by the NULL-
**              terminated name of the function, 4 bytes of the function ID and
**              8 bytes of the profile tick.
**      2       Enter a function to be profiled. Followed by 4 bytes of function
**              ID and 8 bytes of the profile tick.
**      3       Exit a function to be profiled. Followed by 8 bytes of the
**              profile tick.
**
**  There are three options to manage the profile database. One would be to
**  enter a function that needs to be profiled. This is specified with both
**  <Function> and <Initialized> pointers initialized to some value. Here
**  <Function> is pointing to a string with the function name and <Initialized>
**  is pointing to a boolean value that tells the profiler whether this function
**  has been initialized or not.
**
**  The second option would be to exit a function that was being profiled. This
**  is specified by <Function> pointing to a string with the function name and
**  <Initialized> set to gcvNULL.
**
**  The third and last option is to flush the profile database. This is
**  specified with <Function> set to gcvNULL.
**
***** PARAMETERS
**
**  Function
**
**      Pointer to a string with the function name being profiled or gcvNULL to
**      flush the profile database.
**
**  Initialized
**
**      Pointer to a boolean variable that informs the profiler if the entry of
**      a function has been initialized or not, or gcvNULL to mark the exit of a
**      function being profiled.
*/
void
gcoOS_ProfileDB(
    IN gctCONST_STRING Function,
    IN OUT gctBOOL_PTR Initialized
    )
{
    gctUINT64 nanos;
    static gctUINT8_PTR profileBuffer = gcvNULL;
    static gctSIZE_T profileSize, profileThreshold, totalBytes;
    static gctUINT32 profileIndex;
    static gctINT profileLevel;
    static FILE * profileDB = gcvNULL;
    int len, bytes;

    /* Check if we need to flush the profile database. */
    if (Function == gcvNULL)
    {
        if (profileBuffer != gcvNULL)
        {
            /* Check of the profile database exists. */
            if (profileIndex > 0)
            {
                if (profileDB == gcvNULL)
                {
                    /* Open the profile database file. */
                    profileDB = fopen("profile.database", "wb");
                }

                if (profileDB != gcvNULL)
                {
                    /* Write the profile database to the file. */
                    totalBytes += fwrite(profileBuffer,
                                         1, profileIndex,
                                         profileDB);
                }
            }

            if (profileDB != gcvNULL)
            {
                /* Convert the size of the profile database into a nice human
                ** readable format. */
                char buf[] = "#,###,###,###";
                int i;

                i = strlen(buf);
                while ((totalBytes != 0) && (i > 0))
                {
                    if (buf[--i] == ',') --i;

                    buf[i]      = '0' + (totalBytes % 10);
                    totalBytes /= 10;
                }

                /* Print the size of the profile database. */
                gcmPRINT("Closing the profile database: %s bytes.", &buf[i]);

                /* Close the profile database file. */
                fclose(profileDB);
                profileDB = gcvNULL;
            }

            /* Destroy the profile database. */
            free(profileBuffer);
            profileBuffer = gcvNULL;
        }
    }

    /* Check if we have to enter a function. */
    else if (Initialized != gcvNULL)
    {
        /* Check if the profile database exists or not. */
        if (profileBuffer == gcvNULL)
        {
            /* Allocate the profile database. */
            for (profileSize = 32 << 20;
                 profileSize > 0;
                 profileSize -= 1 << 20
            )
            {
                profileBuffer = malloc(profileSize);

                if (profileBuffer != gcvNULL)
                {
                    break;
                }
            }

            if (profileBuffer == gcvNULL)
            {
                /* Sorry - no memory. */
                gcmPRINT("Cannot create the profile buffer!");
                return;
            }

            /* Reset the profile database. */
            profileThreshold = gcmMIN(profileSize / 2, 4 << 20);
            totalBytes       = 0;
            profileIndex     = 0;
            profileLevel     = 0;
        }

        /* Increment the profile level. */
        ++profileLevel;

        /* Determine number of bytes to copy. */
        len   = strlen(Function) + 1;
        bytes = 1 + (*Initialized ? 0 : len) + 4 + 8;

        /* Check if the profile database has enough space. */
        if (profileIndex + bytes > profileSize)
        {
            gcmPRINT("PROFILE ENTRY: index=%lu size=%lu bytes=%d level=%d",
                     profileIndex, profileSize, bytes, profileLevel);

            if (profileDB == gcvNULL)
            {
                /* Open the profile database file. */
                profileDB = fopen("profile.database", "wb");
            }

            if (profileDB != gcvNULL)
            {
                /* Write the profile database to the file. */
                totalBytes += fwrite(profileBuffer, 1, profileIndex, profileDB);
            }

            /* Empty the profile databse. */
            profileIndex = 0;
        }

        /* Check whether this function is initialized or not. */
        if (*Initialized)
        {
            /* Already initialized - don't need to save name. */
            profileBuffer[profileIndex] = 2;
        }
        else
        {
            /* Not yet initialized, save name as well. */
            profileBuffer[profileIndex] = 1;
            gcoOS_MemCopy(profileBuffer + profileIndex + 1, Function, len);
            profileIndex += len;

            /* Mark function as initialized. */
            *Initialized = gcvTRUE;
        }

        /* Copy the function ID into the profile database. */
        gcoOS_MemCopy(profileBuffer + profileIndex + 1, &Initialized, 4);

        /* Get the profile tick. */
        gcoOS_GetProfileTick(&nanos);

        /* Copy the profile tick into the profile database. */
        gcoOS_MemCopy(profileBuffer + profileIndex + 5, &nanos, 8);
        profileIndex += 1 + 4 + 8;
    }

    /* Exit a function, check whether the profile database is around. */
    else if (profileBuffer != gcvNULL)
    {
        /* Get the profile tick. */
        gcoOS_GetProfileTick(&nanos);

        /* Check if the profile database has enough space. */
        if (profileIndex + 1 + 8 > profileSize)
        {
            gcmPRINT("PROFILE EXIT: index=%lu size=%lu bytes=%d level=%d",
                     profileIndex, profileSize, 1 + 8, profileLevel);

            if (profileDB == gcvNULL)
            {
                /* Open the profile database file. */
                profileDB = fopen("profile.database", "wb");
            }

            if (profileDB != gcvNULL)
            {
                /* Write the profile database to the file. */
                totalBytes += fwrite(profileBuffer, 1, profileIndex, profileDB);
            }

            /* Empty the profile databse. */
            profileIndex = 0;
        }

        /* Copy the profile tick into the profile database. */
        profileBuffer[profileIndex] = 3;
        gcoOS_MemCopy(profileBuffer + profileIndex + 1, &nanos, 8);
        profileIndex += 1 + 8;

        /* Decrease the profile level and check whether the profile database is
        ** getting too big if we exit a top-level function. */
        if ((--profileLevel == 0)
        &&  (profileSize - profileIndex < profileThreshold)
        )
        {
            if (profileDB == gcvNULL)
            {
                /* Open the profile database file. */
                profileDB = fopen("profile.database", "wb");
            }

            if (profileDB != gcvNULL)
            {
                /* Write the profile database to the file. */
                totalBytes += fwrite(profileBuffer, 1, profileIndex, profileDB);

                /* Flush the file now. */
                fflush(profileDB);
            }

            /* Empty the profile databse. */
            profileIndex = 0;
        }
    }
}
#endif

/******************************************************************************\
******************************* Signal Management ******************************
\******************************************************************************/

#undef _GC_OBJ_ZONE
#define _GC_OBJ_ZONE    gcvZONE_SIGNAL

/*******************************************************************************
**
**  gcoOS_CreateSignal
**
**  Create a new signal.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctBOOL ManualReset
**          If set to gcvTRUE, gcoOS_Signal with gcvFALSE must be called in
**          order to set the signal to nonsignaled state.
**          If set to gcvFALSE, the signal will automatically be set to
**          nonsignaled state by gcoOS_WaitSignal function.
**
**  OUTPUT:
**
**      gctSIGNAL * Signal
**          Pointer to a variable receiving the created gctSIGNAL.
*/
gceSTATUS
gcoOS_CreateSignal(
    IN gcoOS Os,
    IN gctBOOL ManualReset,
    OUT gctSIGNAL * Signal
    )
{
    gceSTATUS status;
    gcsSIGNAL_PTR signal = gcvNULL;
    gctINT mutexResult = -1;
    gctINT condResult = -1;
    gctINT condattrResult = -1;
    gctINT rc;
    pthread_condattr_t cattr;

    gcmHEADER_ARG("ManualReset=%d", ManualReset);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Signal != NULL);

    /* Allocate a signal structure. */
    gcmONERROR(
        gcoOS_Allocate(Os, gcmSIZEOF(gcsSIGNAL), (gctPOINTER *) &signal));

    /* Initialize mutex. */
    mutexResult = pthread_mutex_init(&signal->mutex, gcvNULL);
    if (mutexResult != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Initialize the condition. */
    condattrResult = pthread_condattr_init(&cattr);
    if (condattrResult != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    rc = pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (rc != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    condResult = pthread_cond_init(&signal->condition, &cattr);
    if (condResult != 0)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* We do not need the attribute any more. */
    pthread_condattr_destroy(&cattr);

    /* Initialize signal states. */
    signal->signature = gcdQNX_SIGNAL_SIGNATURE;
    signal->os        = Os;
    signal->state     = gcvFALSE;
    signal->manual    = ManualReset;
    signal->pending   = 0;
    signal->received  = 0;
    signal->destroyed = gcvFALSE;

#if gcdQNX_SIGNAL_CHECK
    /* Acquire the list mutex. */
    pthread_mutex_lock(&s_signalListMutex);

    /* Add the signal to the list. */
    LIST_INSERT_HEAD(&s_signalList, signal, node);

    /* Release the list mutex. */
    pthread_mutex_unlock(&s_signalListMutex);
#endif

    /* Set the signal handle. */
    *Signal = (gctSIGNAL) signal;

    /* Success. */
    gcmFOOTER_ARG("*Signal=0x%x", *Signal);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (signal != gcvNULL)
    {
        if (condResult == EOK)
        {
            /* Destroy the condition variable. */
            pthread_cond_destroy(&signal->condition);
        }

        if (condattrResult == EOK)
        {
            /* Destroy the attribute. */
            pthread_condattr_destroy(&cattr);
        }

        if (mutexResult == EOK)
        {
            /* Destroy the mutex. */
            pthread_mutex_destroy(&signal->mutex);
        }

        /* Free the signal structure */
        gcmVERIFY_OK(gcoOS_Free(Os, signal));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_DestroySignal
**
**  Destroy a signal.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_DestroySignal(
    IN gcoOS Os,
    IN gctSIGNAL Signal
    )
{
    gcsSIGNAL * signal;

    gcmHEADER_ARG("Signal=0x%x", Signal);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Signal != gcvNULL);

    /* Cast the handle to the signal structure. */
    signal = (gcsSIGNAL *) Signal;

    /* Acquire the mutex. */
    pthread_mutex_lock(&signal->mutex);

    if (signal->pending != signal->received)
    {
        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                      "%s(%d): Signal=0x%x Pending=%d Received=%d.",
                      __FUNCTION__, __LINE__,
                      signal,
                      signal->pending,
                      signal->received);

        /* Only mark the destroy flag if the signal is still pending. */
        signal->destroyed = gcvTRUE;

        /* Release the mutex. */
        pthread_mutex_unlock(&signal->mutex);
    }
    else
    {
        /* Free the signal structure. */
        _DestroySignal(Os, signal);
    }

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gcoOS_Signal
**
**  Set a state of the specified signal.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctBOOL State
**          If gcvTRUE, the signal will be set to signaled state.
**          If gcvFALSE, the signal will be set to nonsignaled state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_Signal(
    IN gcoOS Os,
    IN gctSIGNAL Signal,
    IN gctBOOL State
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSIGNAL * signal;
    gctINT rc;

    gcmHEADER_ARG("Signal=0x%x State=%d", Signal, State);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Signal != gcvNULL);

    /* Cast the handle to the signal structure. */
    signal = (gcsSIGNAL *) Signal;

    /* Acquire the mutex. */
    rc = pthread_mutex_lock(&signal->mutex);
    if (rc != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Set the state. */
    signal->state = State;

    /* If the state is signaled, notify the waiting threads. */
    if (State)
    {
        if (signal->manual)
        {
            rc = pthread_cond_broadcast(&signal->condition);
        }
        else
        {
            rc = pthread_cond_signal(&signal->condition);
        }

        if (rc != EOK)
        {
            /* Release the mutex. */
            pthread_mutex_unlock(&signal->mutex);

            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&signal->mutex);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_WaitSignal
**
**  Wait for a signal to become signaled.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctUINT32 Wait
**          Number of milliseconds to wait.
**          Pass the value of gcvINFINITE for an infinite wait.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_WaitSignal(
    IN gcoOS Os,
    IN gctSIGNAL Signal,
    IN gctUINT32 Wait
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSIGNAL * signal;
    gctINT result;

    gcmHEADER_ARG("Signal=0x%x Wait=%u", Signal, Wait);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Signal != NULL);

    /* Cast the handle to the signal structure. */
    signal = (gcsSIGNAL *) Signal;

    /* Acquire the mutex. */
    result = pthread_mutex_lock(&signal->mutex);
    if (result != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    while (!signal->state)
    {
        if (Wait == 0)
        {
            /* User just wants to check the signal state. */
            result = ETIMEDOUT;
        }
        else if (Wait == gcvINFINITE)
        {
            /* Wait forever. */
            result = pthread_cond_wait(&signal->condition, &signal->mutex);
        }
        else
        {
            struct timespec timeout;
            gctUINT64 nanos;

            /* Get current time. */
            clock_gettime(CLOCK_MONOTONIC, &timeout);

            /* Compute absolute time. */
            nanos  = Wait;
            nanos *= 1000000ULL;
            nanos += timespec2nsec(&timeout);
            nsec2timespec(&timeout, nanos);

            /* Wait until either the condition is set or time out. */
            result = pthread_cond_timedwait(&signal->condition,
                                            &signal->mutex,
                                            &timeout);
        }

        if (result != EOK)
        {
            /* Release the mutex. */
            pthread_mutex_unlock(&signal->mutex);

            if (result == ETIMEDOUT)
            {
                gcmONERROR(gcvSTATUS_TIMEOUT);
            }
            else
            {
                gcmONERROR(gcvSTATUS_GENERIC_IO);
            }
        }
    }

    /* Clear the state if not manual reset. */
    if ((result == EOK) && !signal->manual)
    {
        signal->state = gcvFALSE;
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&signal->mutex);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_SignalPending(
    IN gcoOS Os,
    IN gcsSIGNAL_PTR Signal
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT rc;

    gcmHEADER_ARG("Signal=0x%x", Signal);

    /* Verify the arguments. */
    if (Signal == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the mutex. */
    rc = pthread_mutex_lock(&Signal->mutex);
    if (rc != EOK)
    {
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }

    Signal->pending++;

    /* Release the mutex. */
    pthread_mutex_unlock(&Signal->mutex);

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_AddSignalHandler
**
**  Adds Signal handler depending on Signal Handler Type
**
**  INPUT:
**
**      gceSignalHandlerType SignalHandlerType
**          Type of handler to be added
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcoOS_AddSignalHandler(
    IN gceSignalHandlerType SignalHandlerType
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_QueryCurrentProcessName(
    OUT gctSTRING Name,
    IN gctSIZE_T Size
    )
{
    extern char * __progname;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Name=%p Size=%lu", Name, Size);

    if (!Name || Size <= 0)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gcoOS_StrCopySafe(Name, Size, __progname));

    /* Success. */
    gcmFOOTER_ARG("Name=%s", Name);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoOS_DetectProcessByName
**
**  Detect if the current process is the executable specified.
**
**  INPUT:
**
**      gctCONST_STRING Name
**          Name (full or partial) of executable.
**
**  OUTPUT:
**
**      Nothing.
**
**
**  RETURN:
**
**      gcvSTATUS_TRUE
**              if process is as specified by Name parameter.
**      gcvSTATUS_FALSE
**              Otherwise.
**
*/
gceSTATUS
gcoOS_DetectProcessByName(
    IN gctCONST_STRING Name
    )
{
    gctCHAR curProcessName[gcdMAX_PATH];
    gceSTATUS status = gcvSTATUS_FALSE;

    gcmHEADER_ARG("Name=%s", Name);

    if (gcmIS_SUCCESS(gcoOS_QueryCurrentProcessName(curProcessName, gcdMAX_PATH)) &&
        (gcoOS_StrStr(curProcessName, Name, gcvNULL) == gcvSTATUS_TRUE)
       )
    {
        status = gcvSTATUS_TRUE;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_DetectProcessByEncryptedName(
    IN gctCONST_STRING Name
    )
{
    gceSTATUS status = gcvSTATUS_FALSE;
    gctCHAR *p, buff[gcdMAX_PATH];
    p = buff;

    gcmONERROR(gcoOS_StrCopySafe(buff, gcdMAX_PATH, Name));

    while (*p)
    {
        *p = ~(*p);
        p++;
    }

    status = gcoOS_DetectProcessByName(buff);

OnError:
    return status;
}

gceSTATUS
gcoOS_CPUPhysicalToGPUPhysical(
    IN gctPHYS_ADDR_T CPUPhysical,
    OUT gctPHYS_ADDR_T * GPUPhysical
    )
{
    *GPUPhysical = CPUPhysical;
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_QuerySystemInfo(
    IN gcoOS Os,
    OUT gcsSystemInfo *Info
    )
{
    /* SH cycles = MC cycles * (SH clock/MC clock).
    ** Default value is 128 * 3 (cycles).
    */
    Info->memoryLatencySH = 128 * 3;

    return gcvSTATUS_OK;
}

#if VIVANTE_PROFILER_SYSTEM_MEMORY

gceSTATUS gcoOS_GetMemoryProfileInfo(size_t                      size,
                                     struct _memory_profile_info *info)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#endif
