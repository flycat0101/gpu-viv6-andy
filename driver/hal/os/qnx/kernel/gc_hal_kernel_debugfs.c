/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hal_kernel_qnx.h"
#include "shared/gc_hal_driver.h"
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <KHR/khronos_utils.h>


#define BUFSIZE 512

extern void
_DumpState(
    IN gckKERNEL Kernel
    );

static struct
{
    pthread_t       thd;
    gctBOOL         thdStarted;
    gctBOOL         quitFlag;
    gckGALDEVICE    device;
    int             out;

    union
    {
        struct
        {
            char   *pathname;
        } fifo;

        struct
        {
            int     fd;
        } net;
    } u;
}
s_debugfs_state;

static char *str_trim(char *str)
{
    char *end;

    /* Trim leading space. */
    while (isspace(*str))
    {
        str++;
    }

    /* All spaces? */
    if (*str == 0)
    {
        return str;
    }

    /* Trim trailing space. */
    end = str + strlen(str) - 1;
    while ((end > str) && isspace(*end))
    {
        end--;
    }

    /* Write new num terminator. */
    *(end + 1) = 0;

    return str;
}

static void debugfs_print(const char *format, ...)
{
    static char buffer[1024];
    int fd;
    va_list args;
    va_start(args, format);

    vsnprintf(buffer, gcmCOUNTOF(buffer) - 1, format, args);
    buffer[gcmCOUNTOF(buffer) - 1] = 0;

    fd = s_debugfs_state.out;

    if (fd > 0)
    {
        write(fd, buffer, strlen(buffer));
    }
    else
    {
        gcmkPRINT("%s", buffer);
    }
}

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
    gctCONST_STRING Name
    )
{
    debugfs_print("    %s:\n", Name);
    debugfs_print("        Used  : %10llu B\n", Counter->bytes);
}

static void
_PrintCounter(
    gcsDATABASE_COUNTERS * counter,
    gctCONST_STRING Name
    )
{
    debugfs_print("Counter: %s\n", Name);

    debugfs_print("%-9s%10s","", "All");

    debugfs_print("\n");

    debugfs_print("%-9s","Current");

    debugfs_print("%10lld", counter->bytes);

    debugfs_print("\n");

    debugfs_print("%-9s","Maximum");

    debugfs_print("%10lld", counter->maxBytes);

    debugfs_print("\n");

    debugfs_print("%-9s","Total");

    debugfs_print("%10lld", counter->totalBytes);

    debugfs_print("\n");
}

static void
_ShowCounters(
    gcsDATABASE_PTR database
    )
{
    gctUINT i = 0;
    gcsDATABASE_COUNTERS * counter;
    gcsDATABASE_COUNTERS * nonPaged;

    static gctCONST_STRING surfaceTypes[gcvVIDMEM_TYPE_COUNT] = {
        "Generic",
        "Index",
        "Vertex",
        "Texture",
        "RT",
        "Depth",
        "Bitmap",
        "TS",
        "Image",
        "Mask",
        "Scissor",
        "HZDepth",
        "ICache",
        "TXDesc",
        "Fence",
        "TFBHeader",
        "Command",
    };

    /* Get pointer to counters. */
    counter = &database->vidMem;

    nonPaged = &database->nonPaged;

    debugfs_print("Counter: vidMem (for each surface type)\n");

    debugfs_print("%-9s%10s","", "All");

    for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++)
    {
        counter = &database->vidMemType[i];

        debugfs_print("%10s",surfaceTypes[i]);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Current");

    debugfs_print("%10lld", database->vidMem.bytes);

    for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++)
    {
        counter = &database->vidMemType[i];

        debugfs_print("%10lld", counter->bytes);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Maximum");

    debugfs_print("%10lld", database->vidMem.maxBytes);

    for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++)
    {
        counter = &database->vidMemType[i];

        debugfs_print("%10lld", counter->maxBytes);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Total");

    debugfs_print("%10lld", database->vidMem.totalBytes);

    for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++)
    {
        counter = &database->vidMemType[i];

        debugfs_print("%10lld", counter->totalBytes);
    }

    debugfs_print("\n");

    debugfs_print("Counter: vidMem (for each pool)\n");

    debugfs_print("%-9s%10s","", "All");

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++)
    {
        debugfs_print("%10d", i);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Current");

    debugfs_print("%10lld", database->vidMem.bytes);

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++)
    {
        counter = &database->vidMemPool[i];

        debugfs_print("%10lld", counter->bytes);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Maximum");

    debugfs_print("%10lld", database->vidMem.maxBytes);

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++)
    {
        counter = &database->vidMemPool[i];

        debugfs_print("%10lld", counter->maxBytes);
    }

    debugfs_print("\n");

    debugfs_print("%-9s","Total");

    debugfs_print("%10lld", database->vidMem.totalBytes);

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++)
    {
        counter = &database->vidMemPool[i];

        debugfs_print("%10lld", counter->totalBytes);
    }

    debugfs_print("\n");

    /* Print nonPaged. */
    _PrintCounter(&database->nonPaged, "nonPaged");
    _PrintCounter(&database->mapMemory, "mapMemory");
}

static int
_ShowRecord(
    IN gcsDATABASE_RECORD_PTR record
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

    debugfs_print("%-14s %8d %16p %16p %16zu\n",
        recordTypes[record->type],
        record->kernel->core,
        record->data,
        record->physical,
        record->bytes
        );

    return 0;
}

static int
_ShowRecords(
    IN gcsDATABASE_PTR Database
    )
{
    gctUINT i;

    debugfs_print("Records:\n");

    debugfs_print("%14s %8s %16s %16s %16s\n",
               "Type", "GPU", "Data", "Physical", "Bytes");

    for (i = 0; i < gcmCOUNTOF(Database->list); i++)
    {
        gcsDATABASE_RECORD_PTR record = Database->list[i];

        while (record != NULL)
        {
            _ShowRecord(record);
            record = record->next;
        }
    }

    return 0;
}

static void
_ShowProcess(
    IN gcsDATABASE_PTR Database
    )
{
    gctINT pid;
    gctUINT8 name[24];

    /* Process ID and name */
    pid = Database->processID;

    gcmkVERIFY_OK(gckOS_ZeroMemory(name, gcmSIZEOF(name)));
    gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

    debugfs_print("--------------------------------------------------------------------------------\n");
    debugfs_print("Process: %-8d %s\n", pid, name);

    /* Detailed records */
    _ShowRecords(Database);

    debugfs_print("Counters:\n");

    _ShowCounters(Database);
}

static void
_ShowProcesses(
    IN gckKERNEL Kernel
    )
{
    gcsDATABASE_PTR database;
    gctINT i;
    static gctUINT64 idleTime = 0;

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(
        gckOS_AcquireMutex(Kernel->os, Kernel->db->dbMutex, gcvINFINITE));

    if (Kernel->db->idleTime)
    {
        /* Record idle time if DB upated. */
        idleTime = Kernel->db->idleTime;
        Kernel->db->idleTime = 0;
    }

    /* Idle time since last call */
    debugfs_print("GPU Idle: %llu ns\n", idleTime);

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(Kernel->db->db); ++i)
    {
        for (database = Kernel->db->db[i];
             database != gcvNULL;
             database = database->next)
        {
            _ShowProcess(database);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->db->dbMutex));
}

static int debugfs_read_help();

static int debugfs_read_info()
{
    gckGALDEVICE device = s_debugfs_state.device;
    gceCHIPMODEL chipModel;
    gctUINT32 chipRevision;
    int i;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i] != gcvNULL)
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
            }

            debugfs_print("gpu      : %4d\n", i);
            debugfs_print("model    : %4x\n", chipModel);
            debugfs_print("revision : %4x\n", chipRevision);
            debugfs_print("\n");
        }
    }

    return 0;
}

static int debugfs_read_clients()
{
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel = _GetValidKernel(device);

    gcsDATABASE_PTR database;
    gctINT i, pid;
    gctUINT8 name[24];

    debugfs_print("%-8s%s\n", "PID", "NAME");
    debugfs_print("------------------------\n");

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(
        gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i)
    {
        for (database = kernel->db->db[i];
             database != gcvNULL;
             database = database->next)
        {
            pid = database->processID;

            gcmkVERIFY_OK(gckOS_ZeroMemory(name, gcmSIZEOF(name)));

            gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

            debugfs_print("%-8d%s\n", pid, name);
        }
    }

    debugfs_print("\n");

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    return 0;
}

static int debugfs_read_meminfo()
{
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel = _GetValidKernel(device);
    gckVIDMEM memory;
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gctUINT32 i;

    gctUINT32 free = 0, used = 0, total = 0;

    gcsDATABASE_COUNTERS virtualCounter = {0, 0, 0};
    gcsDATABASE_COUNTERS nonPagedCounter = {0, 0, 0};

    status = gckKERNEL_GetVideoMemoryPool(kernel, gcvPOOL_SYSTEM, &memory);

    if (gcmIS_SUCCESS(status))
    {
        gcmkVERIFY_OK(
            gckOS_AcquireMutex(memory->os, memory->mutex, gcvINFINITE));

        free  = memory->freeBytes;
        used  = memory->bytes - memory->freeBytes;
        total = memory->bytes;

        gcmkVERIFY_OK(gckOS_ReleaseMutex(memory->os, memory->mutex));
    }

    debugfs_print("VIDEO MEMORY:\n");
    debugfs_print("    gcvPOOL_SYSTEM:\n");
    debugfs_print("        Free  : %10u B\n", free);
    debugfs_print("        Used  : %10u B\n", used);
    debugfs_print("        Total : %10u B\n", total);

    /* Acquire the database mutex. */
    gcmkVERIFY_OK(
        gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

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

    _CounterPrint(&virtualCounter, "gcvPOOL_VIRTUAL");

    debugfs_print("\n");

    debugfs_print("NON PAGED MEMORY:\n");
    debugfs_print("    Used  : %10llu B\n", nonPagedCounter.bytes);

    debugfs_print("\n");

    return 0;
}

static int debugfs_read_idle()
{
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel = _GetValidKernel(device);

    gctUINT64 on;
    gctUINT64 off;
    gctUINT64 idle;
    gctUINT64 suspend;

    gckHARDWARE_QueryStateTimer(kernel->hardware, &on, &off, &idle, &suspend);

    /* Idle time since last call */
    debugfs_print("On:      %llu ns\n", on);
    debugfs_print("Off:     %llu ns\n", off);
    debugfs_print("Idle:    %llu ns\n", idle);
    debugfs_print("Suspend: %llu ns\n", suspend);
    debugfs_print("\n");

    return 0;
}

static int debugfs_read_version()
{
    debugfs_print("Version:    %s\n"
                  "Code path:  %s\n"
                  "Build time: %s  %s\n\n",
                  gcvVERSION_STRING, __FILE__, __DATE__, __TIME__);

    return 0;
}

static int debugfs_write_vidmem(void *cmd)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel = _GetValidKernel(device);
    int pid;
    gctUINT8 name[24];

    pid = atoi(str_trim(cmd));

    /* Find the database. */
    gcmkONERROR(gckKERNEL_FindDatabase(kernel, pid, gcvFALSE, &database));

    /* Get the process name. */
    gcmkVERIFY_OK(gckOS_ZeroMemory(name, gcmSIZEOF(name)));
    gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

    debugfs_print("VidMem Usage (pid: %d, name: %s):\n", pid, name);

    _ShowCounters(database);

    debugfs_print("\n");

    return 0;

OnError:
    return 0;
}

static int debugfs_read_database()
{
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel = _GetValidKernel(device);

    _ShowProcesses(kernel);

    debugfs_print("\n");

    return 0;
}

static int debugfs_write_dump_trigger(void *cmd)
{
    gckGALDEVICE device = s_debugfs_state.device;
    gckKERNEL kernel;
    int core;

    core = atoi(str_trim(cmd));

    if ((core >= gcvCORE_MAJOR) && (core < gcvCORE_COUNT))
    {
        kernel = device->kernels[core];

#if gcdENABLE_3D || gcdENABLE_2D
        if (kernel && !kernel->hardware->options.powerManagement)
        {
            debugfs_print("Get dump from the stdout...\n");

            _DumpState(kernel);
        }
#endif
    }
    else
    {
        debugfs_print("*ERROR*: Invalid core: %d.\n", core);
    }

    debugfs_print("\n");

    return 0;
}

static int debugfs_read_interrupts()
{
    gctUINT i;
    gckGALDEVICE device = s_debugfs_state.device;

    debugfs_print("gpu    interrupts\n");

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (device->kernels[i] != gcvNULL)
        {
            debugfs_print("%3d      %8u\n", i, device->interrupts[i]);
        }
    }

    debugfs_print("\n");

    return 0;
}

static int debugfs_quit()
{
    return 1;
}

static gcsINFO s_info_list[] =
{
    { "m",              debugfs_read_help },
    { "info",           debugfs_read_info },
    { "clients",        debugfs_read_clients },
    { "meminfo",        debugfs_read_meminfo },
    { "idle",           debugfs_read_idle },
    { "database",       debugfs_read_database },
    { "version",        debugfs_read_version },
    { "vidmem",         gcvNULL, debugfs_write_vidmem,       "vidmem <pid>" },
    { "dump_trigger",   gcvNULL, debugfs_write_dump_trigger, "dump_trigger <core>" },
    { "interrupts",     debugfs_read_interrupts },
    { "quit",           debugfs_quit },
};

static int debugfs_read_help()
{
    gctUINT i;
    gcsINFO *p;

    debugfs_print("Command action\n");

    for (i = 0; i < gcmCOUNTOF(s_info_list); i++)
    {
        p = &s_info_list[i];

        if (p->help_string != gcvNULL)
        {
            debugfs_print("    %s\n", p->help_string);
        }
        else
        {
            debugfs_print("    %s\n", p->name);
        }
    }

    debugfs_print("\n");

    return 0;
}

static int debugfs_command(char *cmd)
{
    gctUINT i;
    gcsINFO *p;
    int ret = 0;

    if (strlen(cmd) == 0)
    {
        goto Exit;
    }

    for (i = 0; i < gcmCOUNTOF(s_info_list); i++)
    {
        p = &s_info_list[i];

        if (strcmp(cmd, p->name) == 0)
        {
            ret = p->read();
            goto Exit;
        }

        if ((p->write != gcvNULL) &&
            (strstr(cmd, p->name) == cmd))
        {
            ret = p->write(cmd + strlen(p->name));
            goto Exit;
        }
    }

    debugfs_print("*ERROR*: Invalid input: '%s'.\n\n", cmd);

Exit:
    /* Delay. */
    gcmkVERIFY_OK(gckOS_Delay(gcvNULL, 100));

    return ret;
}


static void *debugfs_thread(void *data)
{
    char *pathname;
    int fd = -1;
    struct stat stat;
    int n;
    char buffer[BUFSIZE] = { 0 };
    int rc;
    int flags;
    int dumpFd = 0;

    pathname = s_debugfs_state.u.fifo.pathname;

    while (!s_debugfs_state.quitFlag)
    {
        /* Open the file. */
        rc = __khrGetDeviceConfigValue(1, "gpu-debugfs-dump-to-fd", buffer, sizeof(buffer));
        if (rc == EOK)
        {
            dumpFd = (atoi(buffer) == 1);
        }

        flags = dumpFd ? O_RDWR : O_RDONLY;

        fd = open(pathname, flags);

        if (fd == -1)
        {
            gcmkPRINT("[VIV]: %s:%d: file open error: '%s'.\n", __FUNCTION__, __LINE__, strerror(errno));
            goto OnError;
        }

        if (s_debugfs_state.quitFlag)
        {
            goto OnError;
        }

        /* Get the file information. */
        if (fstat(fd, &stat) == -1)
        {
            gcmkPRINT("[VIV]: %s:%d: file stat error: '%s'.\n", __FUNCTION__, __LINE__, strerror(errno));
            goto OnError;
        }

        if (!S_ISFIFO(stat.st_mode))
        {
            gcmkPRINT("[VIV]: %s:%d: file not fifo: '%s'.\n", __FUNCTION__, __LINE__);
            goto OnError;
        }

        /* Read the data. */
        n = read(fd, buffer, gcmSIZEOF(buffer));
        if (n == -1)
        {
            gcmkPRINT("[VIV]: %s:%d: file read error: '%s'.\n", __FUNCTION__, __LINE__, strerror(errno));
            goto OnError;
        }

        if (dumpFd)
        {
            /*
             * Otherwise the data would be dumped with gcmkPRINT() as fd is
             * zero'ed at the beginning. This has the side effect that from
             * the shell we won't be able to retrieve the data.
             */
            s_debugfs_state.out = fd;
        }

        /* Handle the command. */
        debugfs_command(str_trim(buffer));

        /* Reset the data. */
        gcmkVERIFY_OK(gckOS_ZeroMemory(buffer, gcmSIZEOF(buffer)));
        close(fd);
    }

OnError:
    if (fd != -1)
    {
        close(fd);
    }

    return NULL;
}

static int debugfs_init()
{
    char *pathname;
    int ret;

    /* Get an environment variable. */
    pathname = getenv("QNX_VIV_DEBUGFS");
    if (pathname == gcvNULL)
    {
        goto OnError;
    }

    /* Duplicate the string. */
    s_debugfs_state.u.fifo.pathname = strdup(pathname);
    if (s_debugfs_state.u.fifo.pathname == gcvNULL)
    {
        gcmkPRINT("[VIV]: %s:%d: strdup failed.\n", __FUNCTION__, __LINE__);
        goto OnError;
    }

    /* Create the worker thread. */
    ret = pthread_create(&s_debugfs_state.thd, NULL, debugfs_thread, NULL);
    if (ret != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: create thread failed, ret=%d.\n", __FUNCTION__, __LINE__, ret);
        goto OnError;
    }

    pthread_setname_np(s_debugfs_state.thd, "viv_debugfs_fifo");

    /* Set the flag. */
    s_debugfs_state.thdStarted = gcvTRUE;

    /* Success. */
    return 0;

OnError:
    if (s_debugfs_state.u.fifo.pathname != gcvNULL)
    {
        free(s_debugfs_state.u.fifo.pathname);
        s_debugfs_state.u.fifo.pathname = gcvNULL;
    }

    return 1;
}

static int debugfs_exit()
{
    if (s_debugfs_state.thdStarted)
    {
        int fd = open(s_debugfs_state.u.fifo.pathname, O_WRONLY);

        if (fd != -1)
        {
            close(fd);
        }

        pthread_join(s_debugfs_state.thd, gcvNULL);

        free(s_debugfs_state.u.fifo.pathname);
    }

    return 0;
}


gctINT gckDEBUGFS_Initialize(void *data)
{
    gcmkVERIFY_OK(gckOS_ZeroMemory(
        &s_debugfs_state, gcmSIZEOF(s_debugfs_state)));

    s_debugfs_state.device = (gckGALDEVICE) data;

    return debugfs_init();
}

gctINT gckDEBUGFS_Terminate(void)
{
    int ret;

    s_debugfs_state.quitFlag = gcvTRUE;

    ret = debugfs_exit();

    gcmkVERIFY_OK(gckOS_ZeroMemory(
        &s_debugfs_state, gcmSIZEOF(s_debugfs_state)));

    return ret;
}
