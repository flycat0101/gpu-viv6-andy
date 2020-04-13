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


#include <stdio.h>
#include <stdlib.h>
#include "gc_hal_kernel_qnx.h"
#include "shared/gc_hal_driver.h"
#include "gc_hal_base.h"

#include "gc_hal_kernel_resource_manager.h"
#include "gc_hal_kernel_resource_manager_buf.h"
#include "gc_hal_kernel_resource_manager_info.h"

#define DEBUG_FILE          "galcore_trace"
#define gcdDEBUG_FS_WARN    "Experimental debug entry, may be removed in future release, do NOT rely on it!\n"

static unsigned int dumpCore = 0;
static unsigned int dumpProcess = 0;

extern void
_DumpState(
    IN gckKERNEL Kernel
    );

static gcmINLINE gckKERNEL
_GetValidKernel(
    gckGALDEVICE Device
    )
{
    if (Device->kernels[gcvCORE_MAJOR])
    {
        return Device->kernels[gcvCORE_MAJOR];
    }
    else
    if (Device->kernels[gcvCORE_3D1])
    {
        return Device->kernels[gcvCORE_3D1];
    }
    else
    if (Device->kernels[gcvCORE_3D2])
    {
        return Device->kernels[gcvCORE_3D2];
    }
    else
    if (Device->kernels[gcvCORE_3D3])
    {
        return Device->kernels[gcvCORE_3D3];
    }
    else
    if (Device->kernels[gcvCORE_2D])
    {
        return Device->kernels[gcvCORE_2D];
    }
    else
    if (Device->kernels[gcvCORE_VG])
    {
        return Device->kernels[gcvCORE_VG];
    }
    else
    {
        gcmkASSERT(gcvFALSE);
        return gcvNULL;
    }
}

int
gc_info_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
    gckGALDEVICE device = m->thread->device;
    int i = 0;
    gceCHIPMODEL chipModel = 0;
    gctUINT32 chipRevision = 0;
    gctUINT32 productID = 0;
    gctUINT32 ecoID = 0;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i])
        {
#if gcdENABLE_VG
            if (i == gcvCORE_VG)
            {
                chipModel = device->kernels[i]->vg->hardware->chipModel;
                chipRevision = device->kernels[i]->vg->hardware->chipRevision;
            }
            else
#endif
            {
                chipModel = device->kernels[i]->hardware->identity.chipModel;
                chipRevision = device->kernels[i]->hardware->identity.chipRevision;
                productID = device->kernels[i]->hardware->identity.productID;
                ecoID = device->kernels[i]->hardware->identity.ecoID;
            }

            msg_buf_add_msg(m, "gpu      : %d\n", i);
            msg_buf_add_msg(m, "model    : %4x\n", chipModel);
            msg_buf_add_msg(m, "revision : %4x\n", chipRevision);
            msg_buf_add_msg(m, "product  : %4x\n", productID);
            msg_buf_add_msg(m, "eco      : %4x\n", ecoID);
            msg_buf_add_msg(m, "\n");
        }
    }

    return 0;
}

int
gc_clients_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
    gckKERNEL kernel = _GetValidKernel(m->thread->device);

    gcsDATABASE_PTR database;
    gctINT i, pid;
    uint8_t name[24];

    msg_buf_add_msg(m, "%-8s%s\n", "PID", "NAME");
    msg_buf_add_msg(m, "------------------------\n");

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i)
    {
        for (database = kernel->db->db[i];
                database != gcvNULL;
                database = database->next)
        {
            pid = database->processID;

            gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name);

            msg_buf_add_msg(m, "%-16d%s\n", pid, name);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    /* Success. */
    return 0;
}

static void
_CounterAdd(
        gcsDATABASE_COUNTERS * Dest,
        gcsDATABASE_COUNTERS * Src
       )
{
    Dest->bytes += Src->bytes;
    Dest->maxBytes += Src->maxBytes;
    Dest->totalBytes += Src->totalBytes;
}

static void
_CounterPrint(
        gcsDATABASE_COUNTERS * Counter,
        gctCONST_STRING Name,
        struct buf_msg* m
         )
{
    msg_buf_add_msg(m, "    %s:\n", Name);
    msg_buf_add_msg(m, "        Used  : %10llu B\n", Counter->bytes);
}

int
gc_meminfo_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
    gckKERNEL kernel = _GetValidKernel(m->thread->device);
    gckVIDMEM memory;
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gctUINT32 i;

    gctUINT32 free = 0, used = 0, total = 0, minFree = 0, maxUsed = 0;

    gcsDATABASE_COUNTERS virtualCounter = {0, 0, 0};
    gcsDATABASE_COUNTERS nonPagedCounter = {0, 0, 0};

    status = gckKERNEL_GetVideoMemoryPool(kernel, gcvPOOL_SYSTEM, &memory);

    if (gcmIS_SUCCESS(status))
    {
        gcmkVERIFY_OK(
                gckOS_AcquireMutex(memory->os, memory->mutex, gcvINFINITE));

        free    = memory->freeBytes;
        minFree = memory->minFreeBytes;
        used    = memory->bytes - memory->freeBytes;
        maxUsed = memory->bytes - memory->minFreeBytes;
        total   = memory->bytes;

        gcmkVERIFY_OK(gckOS_ReleaseMutex(memory->os, memory->mutex));
    }

    msg_buf_add_msg(m, "VIDEO MEMORY:\n");
    msg_buf_add_msg(m, "    gcvPOOL_SYSTEM:\n");
    msg_buf_add_msg(m, "        Free  : %10u B\n", free);
    msg_buf_add_msg(m, "        Used  : %10u B\n", used);
    msg_buf_add_msg(m, "        MinFree  : %10u B\n", minFree);
    msg_buf_add_msg(m, "        MaxUsed  : %10u B\n", maxUsed);
    msg_buf_add_msg(m, "        Total : %10u B\n", total);

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i)
    {
        for (database = kernel->db->db[i];
                database != gcvNULL;
                database = database->next)
        {
            gcsDATABASE_COUNTERS * counter;

            counter = &database->vidMemPool[gcvPOOL_VIRTUAL];
            _CounterAdd(&virtualCounter, counter);


            counter = &database->nonPaged;
            _CounterAdd(&nonPagedCounter, counter);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    _CounterPrint(&virtualCounter, "gcvPOOL_VIRTUAL", m);

    msg_buf_add_msg(m, "\n");

    msg_buf_add_msg(m, "NON PAGED MEMORY:\n");
    msg_buf_add_msg(m, "    Used  : %10llu B\n", nonPagedCounter.bytes);

    return 0;
}

static int
_ShowRecord(
        IN struct buf_msg *File,
        IN gcsDATABASE_RECORD_PTR Record
       )
{
    static const char * recordTypes[gcvDB_NUM_TYPES] = {
        "Unknown",
        "VideoMemory",
        "NonPaged",
        "Signal",
        "VidMemLock",
        "Context",
        "Idel",
        "MapMemory",
        "ShBuf",
    };

    msg_buf_add_msg(File, "%-14s %3d %16p %16zu %16zu\n",
            recordTypes[Record->type],
            Record->kernel->core,
            Record->data,
            (size_t) Record->physical,
            Record->bytes
          );

    return 0;
}

static int
_ShowRecords(
        IN struct buf_msg *File,
        IN gcsDATABASE_PTR Database
        )
{
    gctUINT i;

    msg_buf_add_msg(File, "Records:\n");

    msg_buf_add_msg(File, "%14s %3s %16s %16s %16s\n",
            "Type", "GPU", "Data/Node", "Physical/Node", "Bytes");

    for (i = 0; i < gcmCOUNTOF(Database->list); i++)
    {
        gcsDATABASE_RECORD_PTR record = Database->list[i];

        while (record != NULL)
        {
            _ShowRecord(File, record);
            record = record->next;
        }
    }

    return 0;
}

static void
_ShowCounters(
        struct buf_msg *File,
        gcsDATABASE_PTR Database
         )
{
    gctUINT i = 0;

    static const char * surfaceTypes[gcvSURF_NUM_TYPES] = {
        "Unknown",
        "Index",
        "Vertex",
        "Texture",
        "RenderTarget",
        "Depth",
        "Bitmap",
        "TileStatus",
        "Image",
        "Mask",
        "Scissor",
        "HZ",
        "ICache",
        "TxDesc",
        "Fence",
        "TFBHeader",
    };

    static const char * poolTypes[gcvPOOL_NUMBER_OF_POOLS] = {
        "Unknown",
        "Default",
        "Local",
        "Internal",
        "External",
        "Unified",
        "System",
        "Virtual",
        "User",
    };

    static const char * otherCounterNames[] = {
        "AllocNonPaged",
        "MapMemory",
    };

    gcsDATABASE_COUNTERS * otherCounters[] = {
        &Database->nonPaged,
        &Database->mapMemory,
    };

    msg_buf_add_msg(File, "%-16s %16s %16s %16s\n", "", "Current", "Maximum", "Total");

    /* Print surface type counters. */
    msg_buf_add_msg(File, "%-16s %16lld %16lld %16lld\n",
            "All-Types",
            Database->vidMem.bytes,
            Database->vidMem.maxBytes,
            Database->vidMem.totalBytes);

    for (i = 1; i < gcvSURF_NUM_TYPES; i++)
    {
        msg_buf_add_msg(File, "%-16s %16lld %16lld %16lld\n",
                surfaceTypes[i],
                Database->vidMemType[i].bytes,
                Database->vidMemType[i].maxBytes,
                Database->vidMemType[i].totalBytes);
    }
    msg_buf_add_msg(File, "\n");

    /* Print surface pool counters. */
    msg_buf_add_msg(File, "%-16s %16lld %16lld %16lld\n",
            "All-Pools",
            Database->vidMem.bytes,
            Database->vidMem.maxBytes,
            Database->vidMem.totalBytes);

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++)
    {
        msg_buf_add_msg(File, "%-16s %16lld %16lld %16lld\n",
                poolTypes[i],
                Database->vidMemPool[i].bytes,
                Database->vidMemPool[i].maxBytes,
                Database->vidMemPool[i].totalBytes);
    }
    msg_buf_add_msg(File, "\n");

    /* Print other counters. */
    for (i = 0; i < gcmCOUNTOF(otherCounterNames); i++)
    {
        msg_buf_add_msg(File, "%-16s %16lld %16lld %16lld\n",
                otherCounterNames[i],
                otherCounters[i]->bytes,
                otherCounters[i]->maxBytes,
                otherCounters[i]->totalBytes);
    }
    msg_buf_add_msg(File, "\n");
}

static void
_ShowProcess(
        IN struct buf_msg *File,
        IN gcsDATABASE_PTR Database
        )
{
    gctINT pid;
    uint8_t name[24];

    /* Process ID and name */
    pid = Database->processID;
    gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

    msg_buf_add_msg(File, "--------------------------------------------------------------------------------\n");
    msg_buf_add_msg(File, "Process: %-8d %s\n", pid, name);

    /* Detailed records */
    _ShowRecords(File, Database);

    msg_buf_add_msg(File, "Counters:\n");

    _ShowCounters(File, Database);
}

static void
_ShowProcesses(
        IN struct buf_msg * File,
        IN gckKERNEL Kernel
          )
{
    gcsDATABASE_PTR database;
    gctINT i;
    static gctUINT64 idleTime = 0;

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));

    if (Kernel->db->idleTime)
    {
        /* Record idle time if DB upated. */
        idleTime = Kernel->db->idleTime;
        Kernel->db->idleTime = 0;
    }

    /* Idle time since last call */
    msg_buf_add_msg(File, "GPU Idle: %llu ns\n",  idleTime);

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(Kernel->db->db); ++i)
    {
        for (database = Kernel->db->db[i];
                database != gcvNULL;
                database = database->next)
        {
            _ShowProcess(File, database);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
}

int
gc_db_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
    gckKERNEL kernel = _GetValidKernel(m->thread->device);
    _ShowProcesses(m, kernel);
    return 0 ;
}

int
gc_version_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;

#ifndef HOST
#define HOST "QNX"
#endif

    msg_buf_add_msg(m, "%s built at %s\n",  gcvVERSION_STRING, HOST);
    msg_buf_add_msg(m, "Code path: %s\n", __FILE__);

    return 0 ;
}

/*******************************************************************************
 **
 ** Show PM state timer.
 **
 ** Entry is called as 'idle' for compatible reason, it shows more information
 ** than idle actually.
 **
 **  Start: Start time of this counting period.
 **  End: End time of this counting peroid.
 **  On: Time GPU stays in gcvPOWER_0N.
 **  Off: Time GPU stays in gcvPOWER_0FF.
 **  Idle: Time GPU stays in gcvPOWER_IDLE.
 **  Suspend: Time GPU stays in gcvPOWER_SUSPEND.
 */
int
gc_idle_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
    gckKERNEL kernel = _GetValidKernel(m->thread->device);

    gctUINT64 on;
    gctUINT64 off;
    gctUINT64 idle;
    gctUINT64 suspend;

    gckHARDWARE_QueryStateTimer(kernel->hardware, &on, &off, &idle, &suspend);

    /* Idle time since last call */
    msg_buf_add_msg(m, "On:      %llu ns\n",  on);
    msg_buf_add_msg(m, "Off:     %llu ns\n",  off);
    msg_buf_add_msg(m, "Idle:    %llu ns\n",  idle);
    msg_buf_add_msg(m, "Suspend: %llu ns\n",  suspend);

    return 0 ;
}

int
gc_dump_trigger_show(void *data)
{
    struct buf_msg *m = (struct buf_msg *) data;
#if gcdENABLE_3D || gcdENABLE_2D
    gckGALDEVICE device = m->thread->device;
    gckKERNEL kernel = gcvNULL;

    if (dumpCore >= gcvCORE_MAJOR && dumpCore < gcvCORE_COUNT)
    {
        kernel = device->kernels[dumpCore];
    }
#endif

    msg_buf_add_msg(m, gcdDEBUG_FS_WARN);

#if gcdENABLE_3D || gcdENABLE_2D
    msg_buf_add_msg(m, "Get dump from /sys/gc/galcore_trace\n");

    if (kernel && kernel->hardware->options.powerManagement == gcvFALSE)
    {
        _DumpState(kernel);
    }
#endif

    return 0;
}

int
gc_vidmem_show(void *data)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    struct buf_msg *m = (struct buf_msg *) data;
    gckGALDEVICE device = m->thread->device;
    uint8_t name[64];
    int i;

    gckKERNEL kernel = _GetValidKernel(device);

    if (dumpProcess == 0)
    {
        /* Acquire the database mutex. */
        gcmkVERIFY_OK(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

        for (i = 0; i < gcmCOUNTOF(kernel->db->db); i++)
        {
            for (database = kernel->db->db[i];
                    database != gcvNULL;
                    database = database->next)
            {
                gckOS_GetProcessNameByPid(database->processID, gcmSIZEOF(name), name);
                msg_buf_add_msg(m, "VidMem Usage (Process %d: %s):\n", database->processID, name);
                _ShowCounters(m, database);
                msg_buf_add_msg(m, "\n");
            }
        }

        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));
    }
    else
    {
        /* Find the database. */
        status = gckKERNEL_FindDatabase(kernel, dumpProcess, gcvFALSE, &database);

        if (gcmIS_ERROR(status))
        {
            msg_buf_add_msg(m, "ERROR: process %d not found\n", dumpProcess);
            return 0;
        }

        gckOS_GetProcessNameByPid(dumpProcess, gcmSIZEOF(name), name);
        msg_buf_add_msg(m, "VidMem Usage (Process %d: %s):\n", dumpProcess, name);
        _ShowCounters(m, database);
    }

    return 0;
}

int
gc_vidmem_write(void *data)
{
    struct write_msg *wmsg = (struct write_msg *) data;
    dumpProcess = strtoul(wmsg->buf, NULL, 10);
    return 0;
}

int
gc_dump_trigger_write(void *data)
{
    struct write_msg *wmsg = (struct write_msg *) data;
    dumpCore = strtoul(wmsg->buf, NULL, 10);
    return 0;
}
