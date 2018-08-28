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


#include "gc_hal_kernel_qnx.h"
#if gcdUSE_FAST_MEM_COPY
#include <fastmemcpy.h>
#endif
#include <sys/procfs.h>
#include <sys/time.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <libgen.h>
#include <semaphore.h>
#include <screen/gpu.h>
#include <errno.h>

/* for ARM_PTE_... */
#if (defined(IMX) && defined(AARCH64))
#include <aarch64/mmu.h>
#else
#include <arm/mmu.h>
#endif

#define GC_HAL_QNX_PULSEVAL_SIGNAL  (_PULSE_CODE_MINAVAIL+1)

#define USE_VMALLOC 0

#define _GC_OBJ_ZONE gcvZONE_OS

/*******************************************************************************
***** Version Signature *******************************************************/

const char * _PLATFORM = "\n\0$PLATFORM$QNX$\n";

#define MEMORY_LOCK(os) \
    gcmkVERIFY_OK(gckOS_AcquireMutex( \
                                (os), \
                                (os)->memoryLock, \
                                gcvINFINITE))

#define MEMORY_UNLOCK(os) \
    gcmkVERIFY_OK(gckOS_ReleaseMutex((os), (os)->memoryLock))

#define MEMORY_MAP_LOCK(os) \
    gcmkVERIFY_OK(gckOS_AcquireMutex( \
                                (os), \
                                (os)->memoryMapLock, \
                                gcvINFINITE))

#define MEMORY_MAP_UNLOCK(os) \
    gcmkVERIFY_OK(gckOS_ReleaseMutex((os), (os)->memoryMapLock))

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gckOS
{
    /* Object. */
    gcsOBJECT                   object;

    /* Heap. */
    gckHEAP                     heap;

    /* Pointer to device */
    gckGALDEVICE                device;

    /* Memory management */
    gctPOINTER                  memoryLock;
    gctPOINTER                  memoryMapLock;
    gctPOINTER                  mempoolBaseAddress;
    gctPHYS_ADDR_T              mempoolBasePAddress;
    gctUINT32                   mempoolPageSize;

    gctUINT32                   baseAddress;

    /* Debug lock. */
    gctPOINTER                  debugLock;

    /* Allocate extra page to avoid cache overflow. */
    gctINT32                    extraPageFd;
    gctPOINTER                  extraPageAddress;
    off64_t                     extraPagePAddress;
    size_t                      extraPageSize;
};

typedef struct _gcskSIGNAL
{
    /* Signaled state. */
    gctBOOL         state;

    /* Manual reset flag. */
    gctBOOL         manual;

    /* Mutex. */
    pthread_mutex_t mutex;

    /* Condition. */
    pthread_cond_t  condition;

    /* Number of signals pending in the command queue. */
    gctINT          pending;

    /* Number of signals received. */
    gctINT          received;

    /* Destroy flag. */
    gctBOOL         destroyed;
}
gcskSIGNAL;

typedef struct _gcskSIGNAL *    gcskSIGNAL_PTR;

typedef struct _gcsOSTIMER
{
    gckOS               os;
    gctPOINTER          mutex;
    gctPOINTER          sema;
    gctTIMERFUNCTION    func;
    gctPOINTER          data;
    gctUINT64           dueTime;
    pthread_t           tid;
    gctBOOL             quit;
}
gcsOSTIMER;

typedef struct _gcsOSTIMER * gcsOSTIMER_PTR;

typedef struct _gcskATOM
{
    gctINT32            counter;
} gcskATOM;

typedef struct _gcskATOM * gcskATOM_PTR;

/******************************************************************************\
******************************* Private Functions ******************************
\******************************************************************************/
static void *_KernelTimerThread(void *data)
{
    gcsOSTIMER_PTR timer = (gcsOSTIMER_PTR)data;
    gctUINT64 current = 0;
    gctBOOL trigger = gcvFALSE;
    gctBOOL stopped = gcvFALSE;

    while (1)
    {
Continue:
        gckOS_AcquireSemaphore(timer->os, timer->sema);

        while (1)
        {
            if (timer->quit)
            {
                goto Exit;
            }

            gckOS_GetTime(&current);

            gckOS_AcquireMutex(timer->os, timer->mutex, gcvINFINITE);

            if (timer->dueTime == -1)
            {
                stopped = gcvTRUE;
            }
            else if (current >= timer->dueTime)
            {
                timer->dueTime = -1;
                trigger = gcvTRUE;
            }

            gckOS_ReleaseMutex(timer->os, timer->mutex);

            if (stopped)
            {
                stopped = gcvFALSE;
                goto Continue;
            }

            if (trigger)
            {
                (timer->func)(timer->data);
                trigger = gcvFALSE;
                goto Continue;
            }

            gckOS_Delay(timer->os, 10);
        }
    }

Exit:
    return 0;
}

static gctSIZE_T
_GetPageCount(
    IN gctPOINTER Logical,
    IN gctSIZE_T Size
    )
{
    gctUINTPTR_T logical = gcmPTR2INT(Logical);

    return ((logical + Size + __PAGESIZE - 1) >> 12) - (logical >> 12);
}

static gctPOINTER
_GetPageAlignedAddress(
    IN gctPOINTER Logical,
    OUT gctUINT32 *Offset OPTIONAL
    )
{
    gctUINTPTR_T logical = gcmPTR2INT(Logical);

    if (Offset != gcvNULL)
    {
        *Offset = logical & (__PAGESIZE - 1);
    }

    return gcmINT2PTR(logical & ~(__PAGESIZE - 1));
}

static gceSTATUS
_IsMemoryContiguous(
    IN gckOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Size,
    OUT gctBOOL *Contiguous,
    OUT gctUINT32 *Physical
    )
{
    gceSTATUS status;
    gctUINT32 pid;
    gctUINT32 offset;
    gctPOINTER logical;
    off64_t paddr;
    size_t size, len = 0;

    gcmkHEADER_ARG("Os=0x%X Logical=0x%X Size=%d", Os, Logical, Size);

    gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

    logical = _GetPageAlignedAddress(Logical, &offset);
    size    = _GetPageCount(Logical, Size) * __PAGESIZE;

    if (pid == getpid())
    {
        if (mem_offset64(logical, NOFD, size, &paddr, &len) != 0)
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
    }
    else
    {
        if (mlock_peer(pid, (uintptr_t)logical, size) != 0)           /* Lock the user memory, so mem_offset64_peer can be used */
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
        if (mem_offset64_peer(pid, (uintptr_t)logical, size, &paddr, &len) != 0)
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
    }

    /* Return values. */
    if (Contiguous != gcvNULL)
    {
        *Contiguous = (len == size);
    }

    if (Physical != gcvNULL)
    {
        *Physical = (gctUINT32)paddr + offset;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Contiguous=%d, *Physical=0x%08x", \
        gcmOPT_VALUE(Contiguous), gcmOPT_VALUE(Physical));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_MapPages(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctBOOL InUserSpace,
    IN gctPOINTER Logical,
    IN gctSIZE_T PageCount,
    IN gctSIZE_T ExtraPage,
    IN gctPOINTER PageTable,
    IN gctBOOL Writable
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32* table;
    gctUINT32 addr;
    size_t contigLen = 0;
    off64_t offset;
    gctINT32 bytes;
    gctUINT32 pid;
    int rc;
    gctSIZE_T i;

    gcmkHEADER_ARG("Os=0x%X Core=%d InUserSpace=%d Logical=0x%X PageCount=%u ExtraPage=%u PageTable=0x%X Writable=%d",
                   Os, Core, InUserSpace, Logical, PageCount, ExtraPage, PageTable, Writable);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(PageCount > 0);
    gcmkVERIFY_ARGUMENT(PageTable != gcvNULL);

    addr  = gcmPTR2INT32(Logical);
    table = (gctUINT32 *)PageTable;
    bytes = PageCount * __PAGESIZE;

    if (InUserSpace)
    {
        /* Get current process id. */
        gcmkVERIFY_OK(gckOS_GetProcessID(&pid));
    }

    /* Try to get the user pages so DMA can happen. */
    while (PageCount > 0)
    {
        /* fd should be NOFD here, to get physical address. */
        if (InUserSpace)
        {
            if (mlock_peer((pid_t)pid, (uintptr_t)addr, (size_t)bytes) != 0) /* Lock the user memory, so mem_offset64_peer can be used */
            {
                gcmkONERROR(gcvSTATUS_GENERIC_IO);
            }
            rc = mem_offset64_peer(pid, (uintptr_t)addr, bytes, &offset, &contigLen);
        }
        else
        {
            rc = mem_offset64(gcmINT2PTR(addr), NOFD, bytes, &offset, &contigLen);
        }

        if (rc == -1)
        {
            gcmkPRINT("[VIV]: %s: mem_offset failed: %s\n",
                __FUNCTION__, strerror(errno));
            gcmkPRINT("[VIV]: %s: address: %x, bytes: %d\n",
                __FUNCTION__, (gctUINT32)addr, bytes);

            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        gcmkASSERT(contigLen > 0);

        while (contigLen > 0)
        {
#if gcdENABLE_VG
            if (Core == gcvCORE_VG)
            {
                gcmkONERROR(
                    gckVGMMU_SetPage(Os->device->kernels[Core]->vg->mmu,
                                     (gctUINT32)offset, table++));
            }
            else
#endif
            {
                gcmkONERROR(
                    gckMMU_SetPage(Os->device->kernels[Core]->mmu,
                                   offset, Writable, table++));
            }

            offset += 4096;
            addr += 4096;
            contigLen = contigLen > 4096 ? contigLen - 4096 : 0;
            bytes -= 4096;
            PageCount--;
        }
    }

    for (i = 0; i < ExtraPage; i++)
    {
        offset = Os->extraPagePAddress;

#if gcdENABLE_VG
        if (Core == gcvCORE_VG)
        {
            gcmkONERROR(
                gckVGMMU_SetPage(Os->device->kernels[Core]->vg->mmu,
                                 (gctUINT32)offset, table++));
        }
        else
#endif
        {
            gcmkONERROR(
                gckMMU_SetPage(Os->device->kernels[Core]->mmu,
                               offset, Writable, table++));
        }
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_Construct
**
**  Construct a new gckOS object.
**
**  INPUT:
**
**      gctPOINTER Context
**          Pointer to the gckGALDEVICE class.
**
**  OUTPUT:
**
**      gckOS * Os
**          Pointer to a variable that will hold the pointer to the gckOS object.
*/
gceSTATUS
gckOS_Construct(
    IN gctPOINTER Context,
    OUT gckOS * Os
    )
{
    gckOS os;
    gceSTATUS status;
    size_t pconfig;

    gcmkHEADER_ARG("Context=0x%X", Context);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Os != gcvNULL);

    /* Allocate the gckOS object. */
    os = (gckOS) malloc(gcmSIZEOF(struct _gckOS));

    if (os == gcvNULL)
    {
        /* Out of memory. */
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_MEMORY);
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    /* Zero the memory. */
    gckOS_ZeroMemory(os, gcmSIZEOF(struct _gckOS));

    /* Initialize the gckOS object. */
    os->object.type = gcvOBJ_OS;

    /* Set device device. */
    os->device = Context;

    /* IMPORTANT! No heap yet. */
    os->heap = gcvNULL;

    /* Initialize the memory lock. */
    gcmkONERROR(gckOS_CreateMutex(os, &os->memoryLock));
    gcmkONERROR(gckOS_CreateMutex(os, &os->memoryMapLock));

    /* Create debug lock mutex. */
    gcmkONERROR(gckOS_CreateMutex(os, &os->debugLock));


    /* Find the base address of the physical memory. */
    os->baseAddress = os->device->baseAddress;

    /* Allocate extra page to avoid cache overflow. */
    os->extraPageSize = __PAGESIZE * 2;

    os->extraPageFd = drv_create_shm_object();
    if (os->extraPageFd == -1)
    {
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    if (shm_ctl(
            os->extraPageFd, SHMCTL_ANON | SHMCTL_PHYS,
            0, os->extraPageSize) == -1)
    {
        close(os->extraPageFd);
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    os->extraPageAddress = mmap64(
            0, os->extraPageSize, PROT_READ | PROT_WRITE | PROT_NOCACHE,
            MAP_SHARED, os->extraPageFd, 0);
    if (os->extraPageAddress == MAP_FAILED)
    {
        close(os->extraPageFd);
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    if (mem_offset64(os->extraPageAddress, NOFD, os->extraPageSize,
            &os->extraPagePAddress, &pconfig) == -1)
    {
        munmap(os->extraPageAddress, os->extraPageSize);
        close(os->extraPageFd);
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    os->extraPagePAddress = gcmALIGN(os->extraPagePAddress, __PAGESIZE);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                  "Physical base address set to 0x%08X.\n",
                  os->baseAddress);

    os->mempoolBaseAddress = drv_mempool_get_baseAddress();
    os->mempoolBasePAddress = drv_mempool_get_basePAddress();
    os->mempoolPageSize = drv_mempool_get_page_size();

    /* Return pointer to the gckOS object. */
    *Os = os;

    /* Success. */
    gcmkFOOTER_ARG("*Os=0x%X", *Os);
    return gcvSTATUS_OK;

OnError:
    if (os->heap != gcvNULL)
    {
        gcmkVERIFY_OK(
            gckHEAP_Destroy(os->heap));
    }

    if (os->memoryMapLock != gcvNULL)
    {
        gcmkVERIFY_OK(
            gckOS_DeleteMutex(os, os->memoryMapLock));
    }

    if (os->memoryLock != gcvNULL)
    {
        gcmkVERIFY_OK(
            gckOS_DeleteMutex(os, os->memoryLock));
    }

    if (os->debugLock != gcvNULL)
    {
        gcmkVERIFY_OK(
            gckOS_DeleteMutex(os, os->debugLock));
    }

    free(os);

    /* Return the error. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_Destroy
**
**  Destroy an gckOS object.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object that needs to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_Destroy(
    IN gckOS Os
    )
{
    gckHEAP heap;

    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    /* Destroy the extra page. */
    if (Os->extraPageAddress != 0)
    {
        munmap(Os->extraPageAddress, Os->extraPageSize);
        close(Os->extraPageFd);
    }

    if (Os->heap != gcvNULL)
    {
        /* Mark gckHEAP as gone. */
        heap     = Os->heap;
        Os->heap = gcvNULL;

        /* Destroy the gckHEAP object. */
        gcmkVERIFY_OK(gckHEAP_Destroy(heap));
    }

    /* Destroy the memory lock. */
    gcmkVERIFY_OK(gckOS_DeleteMutex(Os, Os->memoryMapLock));
    gcmkVERIFY_OK(gckOS_DeleteMutex(Os, Os->memoryLock));

    /* Destroy debug lock mutex. */
    gcmkVERIFY_OK(gckOS_DeleteMutex(Os, Os->debugLock));

    /* Mark the gckOS object as unknown. */
    Os->object.type = gcvOBJ_UNKNOWN;

    /* Free the gckOS object. */
    free(Os);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Allocate
**
**  Allocate memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the allocated memory location.
*/
gceSTATUS
gckOS_Allocate(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Bytes=%lu", Os, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    /* Do we have a heap? */
    if (Os->heap != gcvNULL)
    {
        /* Allocate from the heap. */
        gcmkONERROR(gckHEAP_Allocate(Os->heap, Bytes, Memory));
    }
    else
    {
        gcmkONERROR(gckOS_AllocateMemory(Os, Bytes, Memory));
    }

    /* Success. */
    gcmkFOOTER_ARG("*Memory=0x%X", *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_Free
**
**  Free allocated memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Memory
**          Pointer to memory allocation to free.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_Free(
    IN gckOS Os,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Memory=0x%X", Os, Memory);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    /* Do we have a heap? */
    if (Os->heap != gcvNULL)
    {
        /* Free from the heap. */
        gcmkONERROR(gckHEAP_Free(Os->heap, Memory));
    }
    else
    {
        gcmkONERROR(gckOS_FreeMemory(Os, Memory));
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}
/*******************************************************************************
**
**  gckOS_AllocateMemory
**
**  Allocate memory wrapper.
**
**  INPUT:
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the allocated memory location.
*/
gceSTATUS
gckOS_AllocateMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gctPOINTER memory;
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Bytes=%lu", Os, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    memory = (gctPOINTER) calloc(1, Bytes);

    if (memory == gcvNULL)
    {
        /* Out of memory. */
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Return pointer to the memory allocation. */
    *Memory = memory;

    /* Success. */
    gcmkFOOTER_ARG("*Memory=0x%X", *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeMemory
**
**  Free allocated memory wrapper.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory allocation to free.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_FreeMemory(
    IN gckOS Os,
    IN gctPOINTER Memory
    )
{
    gcmkHEADER_ARG("Memory=0x%X", Memory);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    /* Free the memory from the OS pool. */
    free(Memory);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_MapMemory
**
**  Map physical memory into the current process.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to map.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the logical address of the
**          mapped memory.
*/
gceSTATUS
gckOS_MapMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR node;
    gctUINT32 pid = 0;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != 0);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node->type == gcvPHYSICAL_TYPE_MEMPOOL);
    gcmkASSERT(node->fd == 0);
    gcmkASSERT(node->contiguous);
    gcmkASSERT(node->physicalAddress != gcvINVALID_ADDRESS);

    /* Get current process id. */
    gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

    do
    {
        /* Maybe it is already mapped into the user process. */
        if ((node->pid == pid) && (node->userLogical != gcvNULL))
        {
            *Logical = node->userLogical;
            break;
        }

        /* Acquire the lock. */
        MEMORY_LOCK(Os);
        acquired = gcvTRUE;

        /* Map physical address. */
        *Logical = mmap64_peer(pid,
                               0,
                               Bytes,
                               PROT_READ | PROT_WRITE | PROT_NOCACHE,
                               MAP_PHYS | MAP_SHARED,
                               NOFD,
                               (off_t)node->physicalAddress);

        /* Map failed. */
        if (*Logical == MAP_FAILED)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        /* Update the node structure. */
        node->pid         = pid;
        node->userLogical = *Logical;

        /* Release the lock. */
        MEMORY_UNLOCK(Os);
        acquired = gcvFALSE;
    }
    while (gcvFALSE);

    gcmkFOOTER_ARG("*Logical=0x%X", *Logical);
    return gcvSTATUS_OK;

OnError:
    if (acquired)
    {
        MEMORY_UNLOCK(Os);
    }

    gcmkPRINT("[VIV]: %s:%d: %s: Out of memory.\n", __FUNCTION__, __LINE__);

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_UnmapMemory
**
**  Unmap physical memory out of the current process.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**      gctPOINTER Memory
**          Pointer to a previously mapped memory region.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gcsPHYSICAL_PTR node;
    gctUINT32 res;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu Logical=0x%X",
                   Os, Physical, Bytes, Logical);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != 0);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node->type == gcvPHYSICAL_TYPE_MEMPOOL);
    gcmkASSERT(node->fd == 0);
    gcmkASSERT(node->contiguous);
    gcmkASSERT(node->physicalAddress != gcvINVALID_ADDRESS);
    gcmkASSERT(node->pid != 0);
    gcmkASSERT(node->userLogical != gcvNULL);
    gcmkASSERT(node->userLogical == Logical);

    /* Acquire the lock. */
    MEMORY_LOCK(Os);

    /* Unmap the memory. */
    res = munmap_peer(node->pid, Logical, Bytes);

    /* Update the node. */
    node->pid         = 0;
    node->userLogical = gcvNULL;

    /* Release the lock. */
    MEMORY_UNLOCK(Os);

    if (res == -1)
    {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_OS,
            "%s: munmap_peer error: %s\n",
            __FUNCTION__, strerror(errno));

        gcmkFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_UnmapUserLogical
**
**  Unmap user logical memory out of physical memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**      gctPOINTER Memory
**          Pointer to a previously mapped memory region.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapUserLogical(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocateNonPagedMemory
**
**  Allocate a number of pages from non-paged memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE if the pages need to be mapped into user space.
**
**      gctUINT32 Flag
**          Allocation attribute.
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that holds the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that hold the number of bytes allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will hold the physical address of the
**          allocation.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will hold the logical address of the
**          allocation.
*/
gceSTATUS
gckOS_AllocateNonPagedMemory(
    IN gckOS Os,
    IN gctBOOL InUserSpace,
    IN gctUINT32 Flag,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPHYS_ADDR_T physical;
    gcsPHYSICAL_PTR node;
    gctBOOL nodeAllocated = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X InUserSpace=%d Bytes=%d Physical=0x%X Logical=0x%X Memory=0x%X", Os, InUserSpace, *Bytes, Physical, Logical);

    MEMORY_LOCK(Os);

    gcmkONERROR(drv_physical_allocate_node(&node));
    nodeAllocated = gcvTRUE;

    if (InUserSpace)
    {
        gctUINT32 pid;

        gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

        *Logical = drv_shmpool_alloc_contiguous((gctUINT32)pid, *Bytes, 0x1);

        if (*Logical == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        node->type        = gcvPHYSICAL_TYPE_SHMPOOL;
        node->contiguous  = gcvTRUE;
        node->bytes       = *Bytes;
        node->pid         = pid;
        node->userLogical = *Logical;
        node->pageCount   = _GetPageCount(node->userLogical, node->bytes);
    }
    else
    {
        drv_mempool_alloc_contiguous(*Bytes, &physical, Logical);

        if (*Logical == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        node->type            = gcvPHYSICAL_TYPE_MEMPOOL;
        node->contiguous      = gcvTRUE;
        node->bytes           = *Bytes;
        node->physicalAddress = physical;
        node->kernelLogical   = *Logical;
        node->pageCount       = _GetPageCount(node->kernelLogical, node->bytes);
    }

    *Physical = (gctPHYS_ADDR)node;

    MEMORY_UNLOCK(Os);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
        "%s: Bytes->0x%x, Logical->0x%x Physical->0x%x\n",
        __FUNCTION__, (gctUINT32)*Bytes, *Logical, *Physical);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (nodeAllocated)
    {
        drv_physical_free_node(node);
    }

    MEMORY_UNLOCK(Os);

    *Bytes = 0;

    gcmkPRINT("[VIV]: %s:%d: %s: Out of memory.\n", __FUNCTION__, __LINE__);

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeNonPagedMemory
**
**  Free previously allocated and mapped pages from non-paged memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes allocated.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocated memory.
**
**      gctPOINTER Logical
**          Logical address of the allocated memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_FreeNonPagedMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical
    )
{
    gcsPHYSICAL_PTR node;
    int rc = 0;

    gcmkHEADER_ARG("Os=0x%X Bytes=%lu Physical=0x%X Logical=0x%X",
                   Os, Bytes, Physical, Logical);

    gcmkASSERT(Logical != gcvNULL);

    node = (gcsPHYSICAL_PTR)Physical;

    MEMORY_LOCK(Os);

    if (node->type == gcvPHYSICAL_TYPE_SHMPOOL)
    {
        gcmkASSERT(node->userLogical == Logical);

        rc = drv_shmpool_free(node->userLogical);
    }
    else if (node->type == gcvPHYSICAL_TYPE_MEMPOOL)
    {
        gcmkASSERT(node->kernelLogical == Logical);

        rc = drv_mempool_free(node->kernelLogical);
    }
    else
    {
        gcmkASSERT(0);
    }

    drv_physical_free_node(node);

    MEMORY_UNLOCK(Os);

    if (rc == -1)
    {
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
            "%s: Unmap Failed Logical->0x%x, Bytes->%d, Physical->0x%x\n",
            __FUNCTION__, (gctUINT32)Logical, Bytes, (gctUINT32)Physical);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
        "%s: Logical->0x%x Physical->0x%x\n",
        __FUNCTION__, (gctUINT32)Logical, (gctUINT32)Physical);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ReadRegister
**
**  Read data from a register.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
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
gckOS_ReadRegister(
    IN gckOS Os,
    IN gctUINT32 Address,
    OUT gctUINT32 * Data
    )
{
    return gckOS_ReadRegisterEx(Os, gcvCORE_MAJOR, Address, Data);
}

gceSTATUS
gckOS_ReadRegisterEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    OUT gctUINT32 * Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%X", Os, Core, Address);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Data != gcvNULL);

    *Data = (gctUINT32)in32((uintptr_t) ((gctUINT8 *)Os->device->registerBases[Core] + Address));

    /* Success. */
    gcmkFOOTER_ARG("*Data=0x%08x", *Data);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_WriteRegister
**
**  Write data to a register.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
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
gckOS_WriteRegister(
    IN gckOS Os,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    return gckOS_WriteRegisterEx(Os, gcvCORE_MAJOR, Address, Data);
}

gceSTATUS
gckOS_WriteRegisterEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%X Data=0x%08x", Os, Core, Address, Data);

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                gcvZONE_OS,
                "gckOS_WriteRegister: "
                "Writing to physical address [%x] = %x\n",
                (gctUINT8 *)Os->device->registerBases[Core],
                Data);

    out32((uintptr_t) ((gctUINT8 *)Os->device->registerBases[Core] + Address), (uint32_t)Data);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetPageSize
**
**  Get the system's page size.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**  OUTPUT:
**
**      gctSIZE_T * PageSize
**          Pointer to a variable that will receive the system's page size.
*/
gceSTATUS gckOS_GetPageSize(
    IN gckOS Os,
    OUT gctSIZE_T * PageSize
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(PageSize != gcvNULL);

    /* Return the page size. */
    *PageSize = (gctSIZE_T) __PAGESIZE;

    /* Success. */
    gcmkFOOTER_ARG("*PageSize=%u", *PageSize);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetPhysicalAddress
**
**  Get the physical system address of a corresponding virtual address.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Logical address.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Pointer to a variable that receives the 32-bit physical adress.
*/
gceSTATUS
gckOS_GetPhysicalAddress(
    IN gckOS Os,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 res;
    gctPHYS_ADDR_T address;
    off64_t offset;
    gctUINT32 pid;

    gcmkHEADER_ARG("Os=0x%X Logical=0x%X", Os, Logical);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    do
    {
        /* Check the mempool. */
        if (drv_mempool_mem_offset(Logical, &address) == gcvSTATUS_OK)
        {
            *Address = (gctPHYS_ADDR_T)address;

            /* Got it! */
            break;
        }

        /* Get current process id. */
        gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

        /* Check the shmpool. */
        if (drv_shmpool_mem_offset(pid, Logical, &address) == gcvSTATUS_OK)
        {
            *Address = (gctPHYS_ADDR_T)address;

            /* Got it! */
            break;
        }

        MEMORY_LOCK(Os);

        /* Get the physical address. */
        res = mem_offset64(Logical, NOFD, 1, &offset, gcvNULL);

        MEMORY_UNLOCK(Os);

        if (res == EOK)
        {
            gcmkASSERT((offset & 0xFFFFFFFF00000000ull) == 0);

            *Address = (gctPHYS_ADDR_T)offset;

            /* Got it! */
            break;
        }

        /* Not found. */
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }
    while (gcvFALSE);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                   "%s: Logical->0x%x Physical->0x%x\n",
                    __FUNCTION__, gcmPTR2INT32(Logical), (gctUINT32)*Address);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkPRINT("[VIV]: %s: Unable to get physical address for 0x%x\n",
        __FUNCTION__, gcmPTR2INT32(Logical));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_UserLogicalToPhysical
**
**  Get the physical system address of a corresponding user virtual address.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Logical address.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Pointer to a variable that receives the 32-bit physical address.
*/
gceSTATUS gckOS_UserLogicalToPhysical(
    IN gckOS Os,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 pid;
    gctPHYS_ADDR_T address;
    off64_t offset;
    size_t length;
    int rc;

    gcmkHEADER_ARG("Os=0x%X Logical=0x%X", Os, Logical);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

    do
    {
        /* Check the shmpool. */
        if (drv_shmpool_mem_offset_by_user_logical(pid, Logical, &address) == gcvSTATUS_OK)
        {
            *Address = (gctPHYS_ADDR_T)address;

            /* Got it! */
            break;
        }

        if (mlock_peer((pid_t)pid, (uintptr_t)Logical, 1) != 0)       /* Lock the user memory, so mem_offset64_peer can be used */
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
        /* Get the physical address. */
        rc = mem_offset64_peer(pid, (const uintptr_t)Logical, 1, &offset, &length);

        if (rc == EOK)
        {
            *Address = (gctPHYS_ADDR_T)offset;

            /* Got it! */
            break;
        }

        /* Not found. */
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }
    while (gcvFALSE);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                   "%s: Logical->0x%x Physical->0x%x\n",
                    __FUNCTION__, gcmPTR2INT32(Logical), (gctUINT32)*Address);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                   "%s: Unable to get physical address for 0x%x\n",
                   __FUNCTION__, gcmPTR2INT32(Logical));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_MapPhysical
**
**  Map a physical address into kernel space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Physical
**          Physical address of the memory to map.
**
**      gctSIZE_T Bytes
**          Number of bytes to map.
**
**  OUTPUT:
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the base address of the mapped
**          memory.
*/
gceSTATUS
gckOS_MapPhysical(
    IN gckOS Os,
    IN gctUINT32 Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical
    )
{

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    MEMORY_LOCK(Os);

    do
    {
        if (gcmIS_SUCCESS(drv_mempool_get_kernel_logical(Physical, Logical)))
        {
            /* So it is already mapped. */
            break;
        }

        /* Map physical address. */
        *Logical = mmap64(0,
            Bytes,
            PROT_READ | PROT_WRITE | PROT_NOCACHE,
            MAP_PHYS | MAP_SHARED | MAP_NOINIT,
            NOFD,
            (off_t)Physical);

        if (*Logical == MAP_FAILED)
        {
            MEMORY_UNLOCK(Os);

            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_OS,
                "gckOS_MapMemory: mmap error: %s\n",
                strerror(errno));

            gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_MEMORY);
            return gcvSTATUS_OUT_OF_MEMORY;
        }
    }
    while (gcvFALSE);

    MEMORY_UNLOCK(Os);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                  "gckOS_MapPhysical: "
                  "Physical->0x%X Bytes->0x%X Logical->0x%X\n",
                  (gctUINT32) Physical,
                  (gctUINT32) Bytes,
                  (gctUINT32) *Logical);

    /* Success. */
    gcmkFOOTER_ARG("*Logical=0x%X", *Logical);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_UnmapPhysical
**
**  Unmap a previously mapped memory region from kernel memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Pointer to the base address of the memory to unmap.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapPhysical(
    IN gckOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gctUINT32 res;
    gctPHYS_ADDR_T address;

    gcmkHEADER_ARG("Os=0x%X Logical=0x%X Bytes=%lu", Os, Logical, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    MEMORY_LOCK(Os);

    do
    {
        if (gcmIS_SUCCESS(drv_mempool_mem_offset(Logical, &address)))
        {
            /* No need to unmap it. */
            break;
        }

        res = munmap(Logical, Bytes);

        if (res == -1)
        {
            MEMORY_UNLOCK(Os);

            gcmkTRACE_ZONE(gcvLEVEL_ERROR,
                gcvZONE_OS,
                "gckOS_UnmapMemory: munmap error: %s\n",
                strerror(errno));

            gcmkFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    while (gcvFALSE);

    MEMORY_UNLOCK(Os);

    gcmkTRACE_ZONE(gcvLEVEL_INFO,
                    gcvZONE_OS,
                    "gckOS_UnmapPhysical: "
                    "Logical->0x%x Bytes->0x%x\n",
                    (gctUINT32)Logical,
                    (gctUINT32)Bytes);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CreateMutex
**
**  Create a new mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Mutex
**          Pointer to a variable that will hold a pointer to the mutex.
*/
gceSTATUS
gckOS_CreateMutex(
    IN gckOS Os,
    OUT gctPOINTER * Mutex
    )
{
    pthread_mutexattr_t mattr;
    int rc;

    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Validate the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Allocate a FAST_MUTEX structure. */
    *Mutex = (gctPOINTER) malloc(sizeof(pthread_mutex_t));
    if (*Mutex == gcvNULL)
    {
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_MEMORY);
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    /* Initialize the mutex. */
    rc = pthread_mutexattr_init(&mattr);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);

        free(*Mutex);
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    rc = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);

        pthread_mutexattr_destroy(&mattr);
        free(*Mutex);
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    rc = pthread_mutex_init((pthread_mutex_t *)(*Mutex), &mattr);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);

        pthread_mutexattr_destroy(&mattr);
        free(*Mutex);
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    /* We do not need the attribute any more. */
    pthread_mutexattr_destroy(&mattr);

    /* Return status. */
    gcmkFOOTER_ARG("*Mutex=0x%X", *Mutex);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_DeleteMutex
**
**  Delete a mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mute to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DeleteMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex
    )
{
    gctUINT32 res;

    gcmkHEADER_ARG("Os=0x%X Mutex=0x%X", Os, Mutex);

    /* Validate the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    res = pthread_mutex_destroy((pthread_mutex_t *)(Mutex));

    if (res != EOK)
    {
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Delete the fast mutex. */
    free(Mutex);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AcquireMutex
**
**  Acquire a mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
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
gckOS_AcquireMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex,
    IN gctUINT32 Timeout
    )
{
    gceSTATUS status;
    struct timespec tv;
    gctUINT64 nanos;
    gctINT rc;

    gcmkHEADER_ARG("Os=0x%X Mutex=0x%0x Timeout=%u", Os, Mutex, Timeout);

    /* Validate the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    if (Timeout == gcvINFINITE)
    {
        /* Lock the mutex. */
        rc = pthread_mutex_lock((pthread_mutex_t *)Mutex);
    }
    else if (Timeout == 0)
    {
        /* Attempt to lock the mutex. */
        rc = pthread_mutex_trylock((pthread_mutex_t *)Mutex);

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
        rc = pthread_mutex_timedlock_monotonic((pthread_mutex_t *)Mutex, &tv);
    }

    if (rc != EOK)
    {
        if (rc == ETIMEDOUT)
        {
            gcmkONERROR(gcvSTATUS_TIMEOUT);
        }
        else
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_ReleaseMutex
**
**  Release an acquired mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be released.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_ReleaseMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex
    )
{
    gcmkHEADER_ARG("Os=%p Mutex=%p", Os, Mutex);

    /* Validate the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    /* Release the fast mutex. */
    pthread_mutex_unlock((pthread_mutex_t *) Mutex);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicExchange
**
**  Atomically exchange a pair of 32-bit values.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      IN OUT gctINT32_PTR Target
**          Pointer to the 32-bit value to exchange.
**
**      IN gctINT32 NewValue
**          Specifies a new value for the 32-bit value pointed to by Target.
**
**      OUT gctINT32_PTR OldValue
**          The old value of the 32-bit value pointed to by Target.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomicExchange(
    IN gckOS Os,
    IN OUT gctUINT32_PTR Target,
    IN gctUINT32 NewValue,
    OUT gctUINT32_PTR OldValue
    )
{
    gcskATOM_PTR atom;
    gctUINT32 oldValue;

    gcmkHEADER_ARG("Os=0x%X Target=0x%X NewValue=%u", Os, Target, NewValue);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Target != gcvNULL);

    atom = (gcskATOM_PTR)Target;

    oldValue = _smp_xchg((volatile unsigned *)&atom->counter, NewValue);

    if (OldValue != gcvNULL)
    {
        *OldValue = oldValue;
    }

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=%u", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicExchangePtr
**
**  Atomically exchange a pair of pointers.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      IN OUT gctPOINTER * Target
**          Pointer to the 32-bit value to exchange.
**
**      IN gctPOINTER NewValue
**          Specifies a new value for the pointer pointed to by Target.
**
**      OUT gctPOINTER * OldValue
**          The old value of the pointer pointed to by Target.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomicExchangePtr(
    IN gckOS Os,
    IN OUT gctPOINTER * Target,
    IN gctPOINTER NewValue,
    OUT gctPOINTER * OldValue
    )
{
    gcskATOM_PTR atom;
    gctUINT32 oldValue;

    gcmkHEADER_ARG("Os=0x%X Target=0x%X NewValue=0x%X", Os, Target, NewValue);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Target != gcvNULL);

    atom = (gcskATOM_PTR)Target;

    oldValue = _smp_xchg((volatile unsigned *)&atom->counter, gcmPTR2INT(NewValue));

    if (OldValue != gcvNULL)
    {
        *OldValue = gcmINT2PTR(oldValue);
    }

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=0x%p", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicSetMask
**
**  Atomically set mask to Atom, can't be used in interrupt
**
**  INPUT:
**      IN OUT gctPOINTER Atom
**          Pointer to the atom to set.
**
**      IN gctUINT32 Mask
**          Mask to set.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomSetMask(
    IN gctPOINTER Atom,
    IN gctUINT32 Mask
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Atom=0x%X Mask=0x%0x", Atom, Mask);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    atomic_set((volatile unsigned *)&atom->counter, Mask);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomClearMask
**
**  Atomically clear mask from Atom, can't be used in interrupt
**
**  INPUT:
**      IN OUT gctPOINTER Atom
**          Pointer to the atom to clear.
**
**      IN gctUINT32 Mask
**          Mask to clear.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomClearMask(
    IN gctPOINTER Atom,
    IN gctUINT32 Mask
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Atom=0x%X Mask=0x%0x", Atom, Mask);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    atomic_clr((volatile unsigned *)&atom->counter, Mask);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomConstruct
**
**  Create an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Atom
**          Pointer to a variable receiving the constructed atom.
*/
gceSTATUS
gckOS_AtomConstruct(
    IN gckOS Os,
    OUT gctPOINTER* Atom
    )
{
    gceSTATUS status;
    gcskATOM_PTR atom = gcvNULL;

    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Allocate memory for the atom. */
    gcmkONERROR(gckOS_Allocate(Os, gcmSIZEOF(*atom), (gctPOINTER *) &atom));

    /* Initialize the atom. */
    gcmkONERROR(gckOS_ZeroMemory(atom, gcmSIZEOF(*atom)));

    /* Return pointer to atom. */
    *Atom = atom;

    /* Success. */
    gcmkFOOTER_ARG("*Atom=0x%0x Value=0x%0x", *Atom, atom->counter);
    return gcvSTATUS_OK;

OnError:
    if (atom != gcvNULL)
    {
        gcmkOS_SAFE_FREE(Os, atom);
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_AtomDestroy
**
**  Destroy an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom to destroy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomDestroy(
    IN gckOS Os,
    IN gctPOINTER Atom
    )
{
    gcmkHEADER_ARG("Os=0x%X Atom=0x%0x", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Free the atom. */
    gcmkOS_SAFE_FREE(Os, Atom);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomGet
**
**  Get the 32-bit value protected by an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**  OUTPUT:
**
**      gctINT32_PTR Value
**          Pointer to a variable the receives the value of the atom.
*/
gceSTATUS
gckOS_AtomGet(
    IN gckOS Os,
    IN gctPOINTER Atom,
    OUT gctINT32_PTR Value
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Os=0x%X Atom=0x%0x Value=0x%0x", Os, Atom, Value);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);
    gcmkVERIFY_ARGUMENT(Value != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    *Value = atomic_add_value((volatile unsigned *)&atom->counter, 0);

    gcmkFOOTER_ARG("Value=0x%0x", gcmOPT_VALUE(Value));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomSet
**
**  Set the 32-bit value protected by an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**      gctINT32 Value
**          The value of the atom.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomSet(
    IN gckOS Os,
    IN gctPOINTER Atom,
    IN gctINT32 Value
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Os=0x%X Atom=0x%0x Value=%d", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    *(volatile unsigned *)&atom->counter = Value;

    /* Success. */
    gcmkFOOTER_ARG("Value=0x%0x", Value);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomIncrement
**
**  Atomically increment the 32-bit integer value inside an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**  OUTPUT:
**
**      gctINT32_PTR Value
**          Pointer to a variable that receives the original value of the atom.
*/
gceSTATUS
gckOS_AtomIncrement(
    IN gckOS Os,
    IN gctPOINTER Atom,
    OUT gctINT32_PTR OldValue
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Os=%p Atom=%p", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    if (OldValue != gcvNULL)
    {
        *OldValue = atomic_add_value((volatile unsigned *)&atom->counter, 1);
    }
    else
    {
        atomic_add((volatile unsigned *)&atom->counter, 1);
    }

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/* Decrement an atom. */
gceSTATUS
gckOS_AtomDecrement(
    gckOS Os,
    gctPOINTER Atom,
    gctINT32_PTR OldValue
    )
{
    gcskATOM_PTR atom;

    gcmkHEADER_ARG("Os=%p Atom=%p", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    atom = (gcskATOM_PTR)Atom;

    if (OldValue != gcvNULL)
    {
        *OldValue = atomic_sub_value((volatile unsigned *)&atom->counter, 1);
    }
    else
    {
        atomic_sub((volatile unsigned *)&atom->counter, 1);
    }

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=%d", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Delay
**
**  Delay execution of the current thread for a number of milliseconds.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Delay
**          Delay to sleep, specified in milliseconds.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_Delay(
    IN gckOS Os,
    IN gctUINT32 Delay
    )
{
    if (Delay != 0)
    {
        /* Schedule delay. */
        delay(Delay);
    }

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetTicks
**
**  Get the number of milliseconds since the system started.
**
**  INPUT:
**
**  OUTPUT:
**
**      gctUINT32_PTR Time
**          Pointer to a variable to get time.
**
*/
gceSTATUS
gckOS_GetTicks(
    OUT gctUINT32_PTR Time
    )
{
    struct timeval tv;
    gcmkHEADER();

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    *Time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    gcmkFOOTER_NO();

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_TicksAfter
**
**  Compare time values got from gckOS_GetTicks.
**
**  INPUT:
**      gctUINT32 Time1
**          First time value to be compared.
**
**      gctUINT32 Time2
**          Second time value to be compared.
**
**  OUTPUT:
**
**      gctBOOL_PTR IsAfter
**          Pointer to a variable to result.
**
*/
gceSTATUS
gckOS_TicksAfter(
    IN gctUINT32 Time1,
    IN gctUINT32 Time2,
    OUT gctBOOL_PTR IsAfter
    )
{
    gcmkHEADER();

    *IsAfter = (Time2 < Time1);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetTime
**
**  Get the number of microseconds since the system started.
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
gckOS_GetTime(
    OUT gctUINT64_PTR Time
    )
{
    struct timespec tv;

    gcmkHEADER();

    gcmkASSERT(Time);

    clock_gettime(CLOCK_MONOTONIC, &tv);

    *Time  = tv.tv_sec;
    *Time *= 1000000;
    *Time += (tv.tv_nsec / 1000);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CreateTimer
**
**  Create a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctTIMERFUNCTION Function.
**          Pointer to a call back function which will be called when timer is
**          expired.
**
**      gctPOINTER Data.
**          Private data which will be passed to call back function.
**
**  OUTPUT:
**
**      gctPOINTER * Timer
**          Pointer to a variable receiving the created timer.
*/
gceSTATUS
gckOS_CreateTimer(
    IN gckOS Os,
    IN gctTIMERFUNCTION Function,
    IN gctPOINTER Data,
    OUT gctPOINTER * Timer
    )
{
    gceSTATUS status;
    gcsOSTIMER_PTR pointer;
    gctINT ret;

    gcmkHEADER_ARG("Os=0x%X Function=0x%X Data=0x%X", Os, Function, Data);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    /* Allocate the structure. */
    gcmkONERROR(gckOS_Allocate(Os, sizeof(gcsOSTIMER), (gctPOINTER)&pointer));

    pointer->quit    = gcvFALSE;
    pointer->dueTime = -1;
    pointer->func    = Function;
    pointer->data    = Data;
    pointer->os      = Os;

    /* Create the mutex. */
    status = gckOS_CreateMutex(Os, &pointer->mutex);
    if (gcmIS_ERROR(status))
    {
        gckOS_Free(Os, (gctPOINTER)pointer);
        goto OnError;
    }

    /* Create the semaphore. */
    status = gckOS_CreateSemaphore(Os, &pointer->sema);
    if (gcmIS_ERROR(status))
    {
        gckOS_DeleteMutex(Os, pointer->mutex);
        gckOS_Free(Os, (gctPOINTER)pointer);
        goto OnError;
    }

    gckOS_AcquireSemaphore(Os, pointer->sema);

    /* Start the timer thread. */
    ret = pthread_create(&pointer->tid, gcvNULL, _KernelTimerThread, pointer);
    if (EOK != ret)
    {
        gcmkPRINT("[VIV]: %s:%d: ret=%d.\n", __FUNCTION__, __LINE__, ret);

        gckOS_DestroySemaphore(Os, pointer->sema);
        gckOS_DeleteMutex(Os, pointer->mutex);
        gckOS_Free(Os, (gctPOINTER)pointer);
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    pthread_setname_np(pointer->tid, "Timer Thread");

    *Timer = (gctPOINTER)pointer;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_DestroyTimer
**
**  Destory a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be destoryed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroyTimer(
    IN gckOS Os,
    IN gctPOINTER Timer
    )
{
    gcsOSTIMER_PTR timer;

    gcmkHEADER_ARG("Os=0x%X Timer=0x%X", Os, Timer);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    timer = (gcsOSTIMER_PTR)Timer;

    timer->quit = gcvTRUE;

    gckOS_ReleaseSemaphore(Os, timer->sema);

    pthread_join(timer->tid, gcvNULL);

    gckOS_DestroySemaphore(Os, timer->sema);
    gckOS_DeleteMutex(Os, timer->mutex);
    gckOS_Free(Os, (gctPOINTER)timer);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_StartTimer
**
**  Schedule a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be scheduled.
**
**      gctUINT32 Delay
**          Delay in milliseconds.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_StartTimer(
    IN gckOS Os,
    IN gctPOINTER Timer,
    IN gctUINT32 Delay
    )
{
    gcsOSTIMER_PTR timer;
    gctUINT64 current;

    gcmkHEADER_ARG("Os=0x%X Timer=0x%X Delay=%u", Os, Timer, Delay);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);
    gcmkVERIFY_ARGUMENT(Delay != 0);

    timer = (gcsOSTIMER_PTR)Timer;

    gckOS_GetTime(&current);

    gckOS_AcquireMutex(Os, timer->mutex, gcvINFINITE);

    timer->dueTime = current + (Delay * 1000);

    gckOS_ReleaseMutex(Os, timer->mutex);

    gckOS_ReleaseSemaphore(Os, timer->sema);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_StopTimer
**
**  Cancel a unscheduled timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be cancel.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_StopTimer(
    IN gckOS Os,
    IN gctPOINTER Timer
    )
{
    gcsOSTIMER_PTR timer;

    gcmkHEADER_ARG("Os=0x%X Timer=0x%X", Os, Timer);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    timer = (gcsOSTIMER_PTR)Timer;

    gckOS_AcquireMutex(Os, timer->mutex, gcvINFINITE);

    timer->dueTime = -1;

    gckOS_ReleaseMutex(Os, timer->mutex);

    gckOS_ReleaseSemaphore(Os, timer->sema);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_MemoryBarrier
**
**  Make sure the CPU has executed everything up to this point and the data got
**  written to the specified pointer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Address
**          Address of memory that needs to be barriered.
**
**  OUTPUT:
**
**      Nothing.
*/
#if defined(IMX6X) || defined(IMX)
extern uintptr_t m_pl310Base;
#endif
gceSTATUS
gckOS_MemoryBarrier(
    IN gckOS Os,
    IN gctPOINTER Address
    )
{
    gcmkHEADER_ARG("Os=0x%X Address=0x%X", Os, Address);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
#ifdef IMX6X
    /*
     * for write-combine memory access
     */
    __asm__ __volatile__ ("dsb" : : : "memory");

    /*
     * Linux also calles: outer_sync()
     *
     */
    if (m_pl310Base == MAP_DEVICE_FAILED) {
        /* This should unlikely happen */
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, " [%s] device io map for PL310 L2 Cache controller is Failed or not-done (abort ... )", __FUNCTION__);
        abort();
    } else {
        out32((uintptr_t)m_pl310Base + 0x730, 1);
    }
#elif defined(IMX8X)
    __asm__ __volatile__ ("dsb" : : : "memory");
#elif defined(IMX) && !defined(AARCH64)
    __asm__ __volatile__ ("dsb" : : : "memory");
    if (m_pl310Base != MAP_DEVICE_FAILED) { /* non iMX6 platform keep m_pl310Base invalid */
        out32((uintptr_t)m_pl310Base + 0x730, 1);
    }
#elif defined(IMX) && defined(AARCH64)
    __asm__ __volatile__ ("dsb sy" : : : "memory");
#else /* fallback: it is for OMAP4/5, LAZYWRITE causes lock-ups. */
    __asm__ __volatile__ ("dsb" : : : "memory");
    /*
     * As recommended by TI, in order to push the stale data out
     * we need to have strongly ordered access between dsb and isb
     */
    *(volatile unsigned *)Address = *(volatile unsigned *)Address;
    __asm__ __volatile__ ("isb" : : : "memory");
#endif

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocatePagedMemory
**
**  Allocate memory from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
*/
gceSTATUS
gckOS_AllocatePagedMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPHYS_ADDR * Physical
    )
{
    return gckOS_AllocatePagedMemoryEx(Os, gcvFALSE, Bytes, gcvNULL, Physical);
}

/*******************************************************************************
**
**  gckOS_AllocatePagedMemoryEx
**
**  Allocate memory from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Flag
**          Allocation attribute.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctUINT32 * Gid
**          Save the global ID for the piece of allocated memory.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
*/
gceSTATUS
gckOS_AllocatePagedMemoryEx(
    IN gckOS Os,
    IN gctUINT32 Flag,
    IN gctSIZE_T Bytes,
    OUT gctUINT32 * Gid,
    OUT gctPHYS_ADDR * Physical
    )
{
#if defined(IMX6X) || defined(IMX8X) || defined(IMX)
    int rc, fd, shm_ctl_flags = SHMCTL_ANON | SHMCTL_LAZYWRITE;   /* use SHMCTL_LAZYWRITE to get write-combine memory access */
#else
    int rc, fd, shm_ctl_flags = SHMCTL_ANON /* | SHMCTL_LAZYWRITE - don't do it.  Bad. */;
#endif
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gcsPHYSICAL_PTR node;
    gctBOOL nodeAllocated = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X Flag=%x Bytes=%lu", Os, Flag, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);

    if (Physical == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Flag & gcvALLOC_FLAG_CONTIGUOUS)
    {
        shm_ctl_flags |= SHMCTL_PHYS;
    }

    /* Lock down, to avoid opening same shm file twice. */
    MEMORY_LOCK(Os);
    acquired = gcvTRUE;

    /* Allocate a physical node. */
    gcmkONERROR(drv_physical_allocate_node(&node));
    nodeAllocated = gcvTRUE;

    fd = drv_create_shm_object();
    if (fd == -1) {
        gcmkPRINT("[VIV]: %s:%d, shm_open failed. error %s\n",
            __FUNCTION__, __LINE__, strerror(errno));
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "shm_open failed. error %s", strerror( errno ) );
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Virtual memory doesn't need to be physically contiguous. */
    /* Allocations would be page aligned. */
    rc = shm_ctl(fd,
                 shm_ctl_flags,
                  0,
                 Bytes);
    if (rc == -1) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "shm_ctl failed. error %s", strerror( errno ) );
        close(fd);
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Setup the node structure. */
    node->type       = gcvPHYSICAL_TYPE_PAGED_MEMORY;
    node->fd         = fd;
    node->contiguous = Flag & gcvALLOC_FLAG_CONTIGUOUS;
    node->bytes      = Bytes;
    node->pageCount  = gcmALIGN(Bytes, __PAGESIZE) / __PAGESIZE;

    /* Use the node as the handle for the physical memory just allocated. */
    *Physical = (gctPHYS_ADDR)node;

    /* Gid is not in use at the moment. */
    if (Gid != gcvNULL)
    {
        *Gid = 0;
    }

    /* Release the lock. */
    MEMORY_UNLOCK(Os);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
        "%s: Bytes->0x%x, Physical->0x%x\n",
        __FUNCTION__, (gctUINT32)Bytes, (gctUINT32)*Physical);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (nodeAllocated)
    {
        drv_physical_free_node(node);
    }

    if (acquired)
    {
        MEMORY_UNLOCK(Os);
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreePagedMemory
**
**  Free memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_FreePagedMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;
    gcsPHYSICAL_PTR node;
    int rc;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    if (node->type == gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        /* Close the memory fd. */
        rc = close(node->fd);
        if (rc == -1)
        {
            gcmkPRINT("[VIV]: %s:%d: failed. error: %s\n",
                __FUNCTION__, __LINE__, strerror(errno));

            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
    }
    else if (node->type == gcvPHYSICAL_TYPE_WRAPPED_MEMORY)
    {
        gcmkASSERT(node->kernelMapCount == 0);
    }
    else
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Free the node structure. */
    drv_physical_free_node(node);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_LockPages
**
**  Lock memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**      gctBOOL Cacheable
**          Cache mode of mapping.
**
**  OUTPUT:
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the user address of the mapped
**          memory.
**
**      gctSIZE_T * PageCount
**          Pointer to a variable that receives the number of pages required for
**          the page table according to the GPU page size.
*/
gceSTATUS
gckOS_LockPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctBOOL Cacheable,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    gceSTATUS status;
    gcsPHYSICAL_PTR node;
    gctPOINTER addr = gcvNULL;
    gctUINT32 pid = 0;
    gctUINT32 prot = PROT_READ | PROT_WRITE;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Logical);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(PageCount != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Get current process id. */
    gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

    /* Configure the cache flag. */
    if (!Cacheable)
    {
        prot |= PROT_NOCACHE;
    }

    if (node->type == gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        gcmkASSERT(node->pid == 0);
        gcmkASSERT(node->userLogical == gcvNULL);
        gcmkASSERT(node->fd != 0);

        /* Map the memory to the user process. */
        addr = mmap64_peer(pid,
                           0,
                           Bytes,
                           prot,
                           MAP_SHARED,
                           node->fd,
                           0);
    }
    else if (node->type == gcvPHYSICAL_TYPE_WRAPPED_MEMORY)
    {
        gcmkASSERT(node->pid == 0);
        gcmkASSERT(node->fd == 0);

        if (node->userLogical == gcvNULL)
        {
            gcmkASSERT(node->physicalAddress != gcvINVALID_ADDRESS);

            /* Map the memory to the user process. */
            addr = mmap64_peer(pid,
                               0,
                               Bytes,
                               prot,
                               MAP_SHARED,
                               NOFD,
                               (off64_t)node->physicalAddress);

            if (addr != MAP_FAILED)
            {
                node->kernelMapCount++;
            }
        }
        else
        {
            /* Already mapped. */
            addr = node->userLogical;
        }
    }
    else
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    if (addr == MAP_FAILED)
    {
        gcmkPRINT("[VIV]: %s: couldn't map memory of size %d, pid: %d [errno %s]\n",
            __FUNCTION__, (gctUINT32)Bytes, pid, strerror(errno));

        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Update the node structure. */
    node->pid         = pid;
    node->userLogical = addr;

    /* Save the info. */
    *Logical   = addr;
    *PageCount = node->pageCount + node->extraPage;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
        "%s: gctPHYS_ADDR->0x%x Bytes->0x%x Logical->0x%x pid->%d\n",
        __FUNCTION__, (gctUINT32)Physical, (gctUINT32)Bytes,
        (gctUINT32)*Logical, pid);

    /* Success. */
    gcmkFOOTER_ARG("*Logical=0x%X *PageCount=%lu", *Logical, *PageCount);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_MapPages
**
**  Map paged memory into a page table.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T PageCount
**          Number of pages required for the physical address.
**
**      gctPOINTER PageTable
**          Pointer to the page table to fill in.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_MapPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T PageCount,
    IN gctPOINTER PageTable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_MapPagesEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T PageCount,
    IN gctUINT32 Address,
    IN gctPOINTER PageTable,
    IN gctBOOL Writable,
    IN gceSURF_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR node;
    gctPOINTER logical = gcvNULL;
    gctBOOL user;

    gcmkHEADER_ARG("Os=0x%X Core=%d Physical=0x%X PageCount=%u PageTable=0x%X Writable=%d",
                   Os, Core, Physical, PageCount, PageTable, Writable);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(PageCount > 0);
    gcmkVERIFY_ARGUMENT(PageTable != gcvNULL);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node != gcvNULL);

    if ((node->type != gcvPHYSICAL_TYPE_PAGED_MEMORY) &&
        (node->type != gcvPHYSICAL_TYPE_WRAPPED_MEMORY))
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n",
            __FUNCTION__, __LINE__, node->type);

        gcmkASSERT(0);
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (node->pageCount + node->extraPage != PageCount)
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d+%d!=%d\n",
            __FUNCTION__, __LINE__,
            node->pageCount, node->extraPage, PageCount);

        gcmkASSERT(0);
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (node->kernelLogical != 0)
    {
        user    = gcvFALSE;
        logical = node->kernelLogical;
    }
    else if (node->userLogical != 0)
    {
        user    = gcvTRUE;
        logical = node->userLogical;
    }
    else
    {
        gcmkASSERT(0);
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    logical = _GetPageAlignedAddress(logical, gcvNULL);

    /* Map the pages into the page talbe. */
    gcmkONERROR(_MapPages(
        Os, Core, user, logical, node->pageCount, node->extraPage, PageTable, Writable));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_UnmapPages(
    IN gckOS Os,
    IN gctSIZE_T PageCount,
    IN gctUINT32 Address
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_UnlockPages
**
**  Unlock memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**      gctPOINTER Logical
**          Address of the mapped memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnlockPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gcsPHYSICAL_PTR node;
    int rc;
    gctBOOL unmap = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%u Logical=0x%X",
                   Os, Physical, Bytes, Logical);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node->userLogical == Logical);
    gcmkASSERT(node->pid != 0);

    /* Check if need to unmap the memory. */
    if (node->type == gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        unmap = gcvTRUE;
    }
    else if (node->type == gcvPHYSICAL_TYPE_WRAPPED_MEMORY)
    {
        if (node->kernelMapCount > 0)
        {
            node->kernelMapCount = 0;
            unmap = gcvTRUE;
        }
    }
    else
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);
    }

    /* Unmap the memory. */
    if (unmap)
    {
        rc = munmap_peer(node->pid, Logical, Bytes);

        if (rc == -1)
        {
            gcmkPRINT("[VIV]: %s(%d): rc=%d.", __FUNCTION__, __LINE__, rc);
        }

        /* Update the node. */
        node->pid         = 0;
        node->userLogical = gcvNULL;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocateContiguous
**
**  Allocate memory from the contiguous pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE if the pages need to be mapped into user space.
**
**      gctSIZE_T * Bytes
**          Pointer to the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that receives the number of bytes allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the logical address of the
**          memory allocation.
*/
gceSTATUS
gckOS_AllocateContiguous(
    IN gckOS Os,
    IN gctBOOL InUserSpace,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPHYS_ADDR_T physical;
    gcsPHYSICAL_PTR node;
    gctBOOL nodeAllocated = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X InUserSpace=%d Bytes=%d Physical=0x%X Logical=0x%X Memory=0x%X", Os, InUserSpace, *Bytes, Physical, Logical);

    MEMORY_LOCK(Os);

    gcmkONERROR(drv_physical_allocate_node(&node));
    nodeAllocated = gcvTRUE;

    if (InUserSpace)
    {
        gctUINT32 pid;

        gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

        *Logical = drv_shmpool_alloc_contiguous((gctUINT32)pid, *Bytes, 0x0);

        if (*Logical == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        node->type        = gcvPHYSICAL_TYPE_SHMPOOL;
        node->contiguous  = gcvTRUE;
        node->bytes       = *Bytes;
        node->pid         = pid;
        node->userLogical = *Logical;
        node->pageCount   = _GetPageCount(node->userLogical, node->bytes);
    }
    else
    {
        drv_mempool_alloc_contiguous(*Bytes, &physical, Logical);

        if (*Logical == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        node->type            = gcvPHYSICAL_TYPE_MEMPOOL;
        node->contiguous      = gcvTRUE;
        node->bytes           = *Bytes;
        node->physicalAddress = physical;
        node->kernelLogical   = *Logical;
        node->pageCount       = _GetPageCount(node->kernelLogical, node->bytes);
    }

    *Physical = (gctPHYS_ADDR)node;

    MEMORY_UNLOCK(Os);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
        "%s: Bytes->0x%x, Logical->0x%x Physical->0x%x\n",
        __FUNCTION__, (gctUINT32)*Bytes, *Logical, *Physical);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (nodeAllocated)
    {
        drv_physical_free_node(node);
    }

    MEMORY_UNLOCK(Os);

    *Bytes = 0;

    gcmkPRINT("[VIV]: %s: Out of memory.\n", __FUNCTION__);

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeContiguous
**
**  Free memory allocated from the contiguous pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctPOINTER Logical
**          Logicval address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_FreeContiguous(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    /* Same of non-paged memory for now. */
    return gckOS_FreeNonPagedMemory(Os, Bytes, Physical, Logical);
}

#if gcdENABLE_VG
/******************************************************************************
**
**  gckOS_GetKernelLogical
**
**  Return the kernel logical pointer that corresponds to the specified
**  hardware address.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Address
**          Hardware physical address.
**
**  OUTPUT:
**
**      gctPOINTER * KernelPointer
**          Pointer to a variable receiving the pointer in kernel address space.
*/
gceSTATUS
gckOS_GetKernelLogical(
    IN gckOS Os,
    IN gctUINT32 Address,
    OUT gctPOINTER * KernelPointer
    )
{
    return gckOS_GetKernelLogicalEx(Os, gcvCORE_MAJOR, Address, KernelPointer);
}

gceSTATUS
gckOS_GetKernelLogicalEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    OUT gctPOINTER * KernelPointer
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%08x", Os, Core, Address);

    do
    {
        gckGALDEVICE device;
        gckKERNEL kernel;
        gcePOOL pool;
        gctUINT32 offset;
        gctPOINTER logical;

        /* Extract the pointer to the gckGALDEVICE class. */
        device = (gckGALDEVICE) Os->device;

        /* Kernel shortcut. */
        kernel = device->kernels[Core];

        /* Split the memory address into a pool type and offset. */
#if gcdENABLE_VG
       if (Core == gcvCORE_VG)
       {
           gcmkERR_BREAK(gckVGHARDWARE_SplitMemory(
                kernel->vg->hardware, Address, &pool, &offset
                ));
       }
       else
#endif
       {
            gcmkERR_BREAK(gckHARDWARE_SplitMemory(
                kernel->hardware, Address, &pool, &offset
                ));
       }

        /* Dispatch on pool. */
        switch (pool)
        {
        case gcvPOOL_LOCAL_INTERNAL:
            /* Internal memory. */
            logical = device->internalLogical;
            break;

        case gcvPOOL_LOCAL_EXTERNAL:
            /* External memory. */
            logical = device->externalLogical;
            break;

        case gcvPOOL_SYSTEM:
            /* System memory. */
            logical = device->contiguousLogical;
            break;

        default:
            /* Invalid memory pool. */
            gcmkFOOTER();
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* Build logical address of specified address. */
        * KernelPointer = ((gctUINT8_PTR) logical) + offset;

        /* Success. */
        gcmkFOOTER_ARG("*KernelPointer=0x%X", *KernelPointer);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return status. */
    gcmkFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gckOS_MapUserPointer
**
**  Map a pointer from the user process into the kernel address space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Pointer
**          Pointer in user process space that needs to be mapped.
**
**      gctSIZE_T Size
**          Number of bytes that need to be mapped.
**
**  OUTPUT:
**
**      gctPOINTER * KernelPointer
**          Pointer to a variable receiving the mapped pointer in kernel address
**          space.
*/
gceSTATUS
gckOS_MapUserPointer(
    IN gckOS Os,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size,
    OUT gctPOINTER * KernelPointer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 pid;
    off64_t offset = 0;
    size_t bytes;

    gcmkHEADER_ARG("Os=0x%X Pointer=0x%X Size=%d", Os, Pointer, Size);

    do
    {
        if (KernelPointer == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        /* Get current process id. */
        gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

        /* Check the shmpool. */
        *KernelPointer = drv_shmpool_get_kernel_logical(pid, Pointer);

        if (*KernelPointer != gcvNULL)
        {
            /* Got it! */
            break;
        }

        /* Check the user virtual mapping. */
        *KernelPointer = drv_physical_map_get_kernel_logical(pid, Pointer);

        if (*KernelPointer != gcvNULL)
        {
            /* Got it! */
            break;
        }
        if (mlock_peer((pid_t)pid, (uintptr_t) Pointer, Size) != 0)   /* Lock the user memory, so mem_offset64_peer can be used */
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        /* Check the mempool. */
        if (mem_offset64_peer(pid, (uintptr_t) Pointer, Size, &offset, &bytes) == 0)
        {
            status = drv_mempool_get_kernel_logical(offset, KernelPointer);

            if (gcmIS_SUCCESS(status))
            {
                /* Got it! */
                break;
            }
        }

        gcmkTRACE(gcvLEVEL_ERROR,
                "%s:%d: error: Pid=%d, Pointer=0x%x, paddr=0x%x.\n",
                __FUNCTION__, __LINE__, pid, Pointer, offset);

        gcmkONERROR(gcvSTATUS_INVALID_DATA);
    }
    while (gcvFALSE);

    /* Success. */
    gcmkFOOTER_ARG("*KernelPointer=0x%X", *KernelPointer);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_UnmapUserPointer
**
**  Unmap a user process pointer from the kernel address space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Pointer
**          Pointer in user process space that needs to be unmapped.
**
**      gctSIZE_T Size
**          Number of bytes that need to be unmapped.
**
**      gctPOINTER KernelPointer
**          Pointer in kernel address space that needs to be unmapped.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapUserPointer(
    IN gckOS Os,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size,
    IN gctPOINTER KernelPointer
    )
{
    /* Nothing to unmap. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_QueryNeedCopy
**
**  Query whether the memory can be accessed or mapped directly or it has to be
**  copied.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID of the current process.
**
**  OUTPUT:
**
**      gctBOOL_PTR NeedCopy
**          Pointer to a boolean receiving gcvTRUE if the memory needs a copy or
**          gcvFALSE if the memory can be accessed or mapped dircetly.
*/
gceSTATUS
gckOS_QueryNeedCopy(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    OUT gctBOOL_PTR NeedCopy
    )
{
    gcmkHEADER_ARG("Os=0x%X ProcessID=%d", Os, ProcessID);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(NeedCopy != gcvNULL);

    /* No need to copy data. */
    *NeedCopy = gcvFALSE;

    /* Success. */
    gcmkFOOTER_ARG("*NeedCopy=%d", *NeedCopy);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CopyFromUserData
**
**  Copy data from user to kernel memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER KernelPointer
**          Pointer to kernel memory.
**
**      gctPOINTER Pointer
**          Pointer to user memory.
**
**      gctSIZE_T Size
**          Number of bytes to copy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_CopyFromUserData(
    IN gckOS Os,
    IN gctPOINTER KernelPointer,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size
    )
{
    gcmkHEADER_ARG("Os=0x%X KernelPointer=0x%X Pointer=0x%X Size=%lu",
                   Os, KernelPointer, Pointer, Size);

    gcmkASSERT(gcvFALSE);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CopyToUserData
**
**  Copy data from kernel to user memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER KernelPointer
**          Pointer to kernel memory.
**
**      gctPOINTER Pointer
**          Pointer to user memory.
**
**      gctSIZE_T Size
**          Number of bytes to copy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_CopyToUserData(
    IN gckOS Os,
    IN gctPOINTER KernelPointer,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size
    )
{
    gcmkHEADER_ARG("Os=0x%X KernelPointer=0x%X Pointer=0x%X Size=%lu",
                   Os, KernelPointer, Pointer, Size);

    gcmkASSERT(gcvFALSE);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_ReadMappedPointer(
    IN gckOS Os,
    IN gctPOINTER Address,
    IN gctUINT32_PTR Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Address=0x%X Data=%u", Os, Address, Data);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    /* Read memory . */
    *Data = *(gctUINT32_PTR)Address;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_MapUserMemory
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
**          gckOS_UnmapUserMemory.
**
**      gctUINT32_PTR Address
**          Pointer to a variable that will receive the address DMA'able by the
**          hardware.
*/
gceSTATUS
gckOS_MapUserMemory(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPOINTER Memory,
    IN gctUINT32 Physical,
    IN gctSIZE_T Size,
    OUT gctPOINTER * Info,
    OUT gctUINT32_PTR Address
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_UnmapUserMemory
**
**  Unlock a user buffer and that was previously locked down by
**  gckOS_MapUserMemory.
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
**          Information record returned by gckOS_MapUserMemory.
**
**      gctUINT32_PTR Address
**          The address returned by gckOS_MapUserMemory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapUserMemory(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPOINTER Memory,
    IN gctSIZE_T Size,
    IN gctPOINTER Info,
    IN gctUINT32 Address
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_GetBaseAddress
**
**  Get the base address for the physical memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**  OUTPUT:
**
**      gctUINT32_PTR BaseAddress
**          Pointer to a variable that will receive the base address.
*/
gceSTATUS
gckOS_GetBaseAddress(
    IN gckOS Os,
    OUT gctUINT32_PTR BaseAddress
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(BaseAddress != gcvNULL);

    /* Return base address. */
    *BaseAddress = Os->baseAddress;

    /* Success. */
    gcmkFOOTER_ARG("*BaseAddress=0x%08x", *BaseAddress);
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_SuspendInterrupt(
    IN gckOS Os
    )
{
    return gckOS_SuspendInterruptEx(Os, gcvCORE_MAJOR);
}

gceSTATUS
gckOS_SuspendInterruptEx(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    InterruptLock(&Os->device->isrLock);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_ResumeInterrupt(
    IN gckOS Os
    )
{
    return gckOS_ResumeInterruptEx(Os, gcvCORE_MAJOR);
}

gceSTATUS
gckOS_ResumeInterruptEx(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    InterruptUnlock(&Os->device->isrLock);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Perform a memory copy. */
gceSTATUS
gckOS_MemCopy(
        IN gctPOINTER Destination,
        IN gctCONST_POINTER Source,
        IN gctSIZE_T Bytes
        )
{
    gcmkHEADER_ARG("Destination=0x%X Source=0x%X Bytes=%lu",
                   Destination, Source, Bytes);

    gcmkVERIFY_ARGUMENT(Destination != gcvNULL);
    gcmkVERIFY_ARGUMENT(Source != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

#if gcdUSE_FAST_MEM_COPY
    fast_mem_cpy(Destination, Source, Bytes);
#else
    memcpy(Destination, Source, Bytes);
#endif

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_ZeroMemory(
    IN gctPOINTER Memory,
    IN gctSIZE_T Bytes
    )
{
    gcmkHEADER_ARG("Memory=0x%X Bytes=%lu", Memory, Bytes);

    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    memset(Memory, 0, Bytes);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

int
memmgr_peer_sendnc(pid_t pid, int coid, void *smsg, size_t sbytes, void *rmsg, size_t rbytes )
{
    mem_peer_t  peer;
    iov_t       siov[2];
    int         rc;

    peer.i.type = _MEM_PEER;
    peer.i.peer_msg_len = sizeof(peer);
    peer.i.pid = pid;
    peer.i.reserved1 = 0;

    SETIOV(siov + 0, &peer, sizeof peer);
    SETIOV(siov + 1, smsg, sbytes);

    do {
        rc = MsgSendvsnc(coid, siov, 2, rmsg, rbytes);
    } while (rc == -1 && errno == EINTR);

    return rc;
}

void *
_mmap2_peer(pid_t pid, void *addr, size_t len, int prot, int flags, int fd, off64_t off,
        unsigned align, unsigned pre_load, void **base, size_t *size) {
    mem_map_t msg;

    msg.i.type = _MEM_MAP;
    msg.i.zero = 0;
    msg.i.addr = (uintptr_t)addr;
    msg.i.len = len;
    msg.i.prot = prot;
    msg.i.flags = flags;
    msg.i.fd = fd;
    msg.i.offset = off;
    msg.i.align = align;
    msg.i.preload = pre_load;
    msg.i.reserved1 = 0;
    if (memmgr_peer_sendnc(pid, MEMMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
        return MAP_FAILED;
    }
    if (base) {
        *base = (void *)(uintptr_t)msg.o.real_addr;
    }
    if (size) {
        *size = msg.o.real_size;
    }
    return (void *)(uintptr_t)msg.o.addr;
}

void *
mmap64_peer(pid_t pid, void *addr, size_t len, int prot, int flags, int fd, off64_t off) {
    return _mmap2_peer(pid, addr, len, prot, flags, fd, off, 0, 0, 0, 0);
}

static int
mctrl_peer(pid_t pid, const uintptr_t addr, size_t len, unsigned ctrl_type, unsigned flags)
{
    mem_ctrl_t msg;

    msg.i.type = _MEM_CTRL;
    msg.i.subtype = ctrl_type;
    msg.i.addr = addr;
    msg.i.len = len;
    msg.i.flags = flags;
    return memmgr_peer_sendnc(pid, MEMMGR_COID, &msg.i, sizeof msg.i, 0, 0);
}

int
munmap_flags_peer(pid_t pid, void *addr, size_t len, unsigned flags) {
    return mctrl_peer(pid, (uintptr_t) addr, len, _MEM_CTRL_UNMAP, flags);
}


int
munmap_peer(pid_t pid, void *addr, size_t len) {
    int rc;

    rc = munmap_flags_peer(pid, addr, len, 0);
    if ((rc == -1) && (errno == ESRCH)) {
        /* In case the process is terminating then all its mappings are undone by OS. The return value -1 is then expected. */
        rc = 0;
    }
    return rc;
}

int
mlock_peer(pid_t pid, const uintptr_t addr, size_t len)
{
    return mctrl_peer(pid, addr, len, _MEM_CTRL_LOCK, 0);
}

int
munlock_peer(pid_t pid, const uintptr_t addr, size_t len)
{
    return mctrl_peer(pid, addr, len, _MEM_CTRL_UNLOCK, 0);
}

int
mem_offset64_peer(pid_t pid, const uintptr_t addr, size_t len,
                off64_t *offset, size_t *contig_len) {
    int rc;

    struct _peer_mem_off {
        struct _mem_peer peer;
        struct _mem_offset msg;
    };
    typedef union {
        struct _peer_mem_off i;
        struct _mem_offset_reply o;
    } memoffset_peer_t;
    memoffset_peer_t msg;

    msg.i.peer.type = _MEM_PEER;
    msg.i.peer.peer_msg_len = sizeof(msg.i.peer);
    msg.i.peer.pid = pid;
    msg.i.peer.reserved1 = 0;

    msg.i.msg.type = _MEM_OFFSET;
    msg.i.msg.subtype = _MEM_OFFSET_PHYS;
    msg.i.msg.addr = addr;
    msg.i.msg.reserved = -1;
    msg.i.msg.len = len;

    do {
        rc = MsgSendnc(MEMMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o);
    } while (rc == -1 && errno == EINTR);

    if (rc == -1) {
        return -1;
    }

    *offset = msg.o.offset;
    *contig_len = msg.o.size;

    return 0;
}

/*******************************************************************************
**  gckOS_CacheClean
**
**  Clean the cache for the specified addresses.  The GPU is going to need the
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctPOINTER Physical
**          Physical address to flush.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheClean(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctPHYS_ADDR_T Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
#if defined(IMX) && defined(AARCH64)
    __asm__ __volatile__ ("dsb sy");
#else
    __asm__ __volatile__ ("dsb");
#endif

    return gcvSTATUS_OK;
}

/*******************************************************************************
**  gckOS_CacheFlush
**
**  Flush the cache for the specified addresses.  The GPU is going to need the
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheFlush(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctPHYS_ADDR_T Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
#if defined(IMX) && defined(AARCH64)
  __asm__ __volatile__ ("dsb sy");
#else
  __asm__ __volatile__ ("dsb");
#endif

  return gcvSTATUS_OK;
}

/*******************************************************************************
**  gckOS_CacheInvalidate
**
**  Flush the cache for the specified addresses and invalidate the lines as
**  well.  The GPU is going to need and modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheInvalidate(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctPHYS_ADDR_T Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
#if defined(IMX) && defined(AARCH64)
  __asm__ __volatile__ ("dsb sy");
#else
  __asm__ __volatile__ ("dsb");
#endif
     return gcvSTATUS_OK;
}

/*******************************************************************************
********************************* Broadcasting *********************************
*******************************************************************************/

/*******************************************************************************
**
**  gckOS_Broadcast
**
**  System hook for broadcast events from the kernel driver.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gckHARDWARE Hardware
**          Pointer to the gckHARDWARE object.
**
**      gceBROADCAST Reason
**          Reason for the broadcast.  Can be one of the following values:
**
**              gcvBROADCAST_GPU_IDLE
**                  Broadcasted when the kernel driver thinks the GPU might be
**                  idle.  This can be used to handle power management.
**
**              gcvBROADCAST_GPU_COMMIT
**                  Broadcasted when any client process commits a command
**                  buffer.  This can be used to handle power management.
**
**              gcvBROADCAST_GPU_STUCK
**                  Broadcasted when the kernel driver hits the timeout waiting
**                  for the GPU.
**
**              gcvBROADCAST_FIRST_PROCESS
**                  First process is trying to connect to the kernel.
**
**              gcvBROADCAST_LAST_PROCESS
**                  Last process has detached from the kernel.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_Broadcast(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gceBROADCAST Reason
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Hardware=0x%X Reason=%d", Os, Hardware, Reason);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    switch (Reason)
    {
    case gcvBROADCAST_FIRST_PROCESS:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "First process has attached");
        break;

    case gcvBROADCAST_LAST_PROCESS:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "Last process has detached");

        /* Put GPU OFF. */
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware,
                                                gcvPOWER_OFF_BROADCAST));
        break;

    case gcvBROADCAST_GPU_IDLE:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "GPU idle.");

        /*
        * VIV: See bug #10104. Since atomic operation can be used in interrupt
        * VIV: in QNX, so have to disable gcdPOWER_SUSPEND_WHEN_IDLE for QNX.
        */
        /* Put GPU IDLE. */
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware,
                                                gcvPOWER_IDLE_BROADCAST));

        /* Add idle process DB. */
        gcmkONERROR(gckKERNEL_AddProcessDB(Hardware->kernel,
                                           1,
                                           gcvDB_IDLE,
                                           gcvNULL, gcvNULL, 0));
        break;

    case gcvBROADCAST_GPU_COMMIT:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "COMMIT has arrived.");

        /* Add busy process DB. */
        gcmkONERROR(gckKERNEL_AddProcessDB(Hardware->kernel,
                                           0,
                                           gcvDB_IDLE,
                                           gcvNULL, gcvNULL, 0));

        /* Put GPU ON. */
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware, gcvPOWER_ON_AUTO));
        break;

    case gcvBROADCAST_GPU_STUCK:
        gcmkTRACE_N(gcvLEVEL_ERROR, 0, "gcvBROADCAST_GPU_STUCK\n");
        gcmkONERROR(gckKERNEL_Recovery(Hardware->kernel));
        break;

    case gcvBROADCAST_AXI_BUS_ERROR:
        gcmkTRACE_N(gcvLEVEL_ERROR, 0, "gcvBROADCAST_AXI_BUS_ERROR\n");
        gcmkONERROR(gckHARDWARE_DumpGPUState(Hardware));
        gcmkONERROR(gckKERNEL_Recovery(Hardware->kernel));
        break;

    case gcvBROADCAST_OUT_OF_MEMORY:
        status = gcvSTATUS_NOT_SUPPORTED;
        goto OnError;
        break;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_BroadcastHurry
**
**  The GPU is running too slow.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gckHARDWARE Hardware
**          Pointer to the gckHARDWARE object.
**
**      gctUINT Urgency
**          The higher the number, the higher the urgency to speed up the GPU.
**          The maximum value is defined by the gcdDYNAMIC_EVENT_THRESHOLD.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_BroadcastHurry(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gctUINT Urgency
    )
{
    gcmkHEADER_ARG("Os=0x%x Hardware=0x%x Urgency=%u", Os, Hardware, Urgency);

    /* Do whatever you need to do to speed up the GPU now. */

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_BroadcastCalibrateSpeed
**
**  Calibrate the speed of the GPU.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gckHARDWARE Hardware
**          Pointer to the gckHARDWARE object.
**
**      gctUINT Idle, Time
**          Idle/Time will give the percentage the GPU is idle, so you can use
**          this to calibrate the working point of the GPU.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_BroadcastCalibrateSpeed(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gctUINT Idle,
    IN gctUINT Time
    )
{
    gcmkHEADER_ARG("Os=0x%x Hardware=0x%x Idle=%u Time=%u",
                   Os, Hardware, Idle, Time);

    /* Do whatever you need to do to callibrate the GPU speed. */

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
********************************** Semaphores **********************************
*******************************************************************************/

/*******************************************************************************
**
**  gckOS_CreateSemaphore
**
**  Create a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Semaphore
**          Pointer to the variable that will receive the created semaphore.
*/
gceSTATUS
gckOS_CreateSemaphore(
    IN gckOS Os,
    OUT gctPOINTER * Semaphore
    )
{
    gceSTATUS status;
    sem_t *sem = gcvNULL;

    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Allocate the semaphore structure. */
    gcmkONERROR(
        gckOS_Allocate(Os, gcmSIZEOF(*sem), (gctPOINTER *) &sem));

    /* Initialize the semaphore. */
    if (sem_init(sem, 0, 1) != 0)
    {
        gcmkPRINT("[VIV]: %s:%d: errno=%d.\n",
            __FUNCTION__, __LINE__, (int)errno);

        gckOS_Free(Os, sem);
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    /* Return to caller. */
    *Semaphore = (gctPOINTER) sem;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_AcquireSemaphore
**
**  Acquire a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AcquireSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%08X Semaphore=0x%08X", Os, Semaphore);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Acquire the semaphore. */
    if (sem_wait((sem_t *)Semaphore))
    {
        gcmkONERROR(gcvSTATUS_TIMEOUT);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_TryAcquireSemaphore
**
**  Try to acquire a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_TryAcquireSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;
    int rc;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Acquire the semaphore. */
    rc = sem_trywait((sem_t *)Semaphore);
    if ((0 != rc) && (EAGAIN != rc))
    {
        /* Timeout. */
        status = gcvSTATUS_TIMEOUT;
        gcmkFOOTER();
        return status;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ReleaseSemaphore
**
**  Release a previously acquired semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be released.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_ReleaseSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gcmkHEADER_ARG("Os=0x%X Semaphore=0x%X", Os, Semaphore);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Release the semaphore. */
    sem_post((sem_t *)Semaphore);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_DestroySemaphore
**
**  Destroy a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroySemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Semaphore=0x%X", Os, Semaphore);

     /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    sem_destroy((sem_t *)Semaphore);

    /* Free the sempahore structure. */
    gcmkONERROR(gckOS_Free(Os, Semaphore));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_GetProcessID
**
**  Get current process ID.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      gctUINT32_PTR ProcessID
**          Pointer to the variable that receives the process ID.
*/
gceSTATUS
gckOS_GetProcessID(
    OUT gctUINT32_PTR ProcessID
    )
{
    /* Get process ID. */
    *ProcessID = drv_get_user_pid();

    if (0 == *ProcessID)
    {
        /* Return Kernel PID. */
        *ProcessID = getpid();
    }

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetThreadID
**
**  Get current thread ID.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      gctUINT32_PTR ThreadID
**          Pointer to the variable that receives the thread ID.
*/
gceSTATUS
gckOS_GetThreadID(
    OUT gctUINT32_PTR ThreadID
    )
{
    /* Get thread ID. */
    if (ThreadID != gcvNULL)
    {
        *ThreadID = drv_get_user_tid();
    }

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_SetGPUPower
**
**  Set the power of the GPU on or off.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gceCORE Core
**          GPU whose power is set.
**
**      gctBOOL Clock
**          gcvTRUE to turn on the clock, or gcvFALSE to turn off the clock.
**
**      gctBOOL Power
**          gcvTRUE to turn on the power, or gcvFALSE to turn off the power.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_SetGPUPower(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctBOOL Clock,
    IN gctBOOL Power
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Clock=%d Power=%d", Os, Core, Clock, Power);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ResetGPU
**
**  Reset the GPU.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gckCORE Core
**          GPU whose power is set.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_ResetGPU(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    gcmkFOOTER_NO();
    return gcvSTATUS_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------------*/
/*----- Profile --------------------------------------------------------------*/

gceSTATUS
gckOS_GetProfileTick(
    OUT gctUINT64_PTR Tick
    )
{
    struct timespec ts;

    gcmkHEADER();

    clock_gettime(CLOCK_MONOTONIC, &ts);

    *Tick = ts.tv_nsec + ts.tv_sec * 1000000000ULL;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_QueryProfileTickRate(
    OUT gctUINT64_PTR TickRate
    )
{
    *TickRate = 0;

    return gcvSTATUS_NOT_SUPPORTED;
}

gctUINT32
gckOS_ProfileToMS(
    IN gctUINT64 Ticks
    )
{
    gctUINT64 rem = Ticks;
    gctUINT64 b = 1000000;
    gctUINT64 res, d = 1;
    gctUINT32 high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= 1000000)
    {
        high /= 1000000;
        res   = (gctUINT64) high << 32;
        rem  -= (gctUINT64) (high * 1000000) << 32;
    }

    while (((gctINT64) b > 0) && (b < rem))
    {
        b <<= 1;
        d <<= 1;
    }

    do
    {
        if (rem >= b)
        {
            rem -= b;
            res += d;
        }

        b >>= 1;
        d >>= 1;
    }
    while (d);

    return (gctUINT32) res;
}

/******************************************************************************\
******************************* Signal Management ******************************
\******************************************************************************/

#undef _GC_OBJ_ZONE
#define _GC_OBJ_ZONE    gcvZONE_SIGNAL

static gceSTATUS
_DestroySignal(
    IN gcskSIGNAL_PTR Signal
    )
{
    gcmkHEADER_ARG("Signal=0x%X", Signal);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Destroy the mutex. */
    pthread_mutex_destroy(&Signal->mutex);

    /* Destroy the condition variable. */
    pthread_cond_destroy(&Signal->condition);

    /* Free the signal structure. */
    free(Signal);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CreateSignal
**
**  Create a new signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL ManualReset
**          If set to gcvTRUE, gckOS_Signal with gcvFALSE must be called in
**          order to set the signal to nonsignaled state.
**          If set to gcvFALSE, the signal will automatically be set to
**          nonsignaled state by gckOS_WaitSignal function.
**
**  OUTPUT:
**
**      gctSIGNAL * Signal
**          Pointer to a variable receiving the created gctSIGNAL.
*/
gceSTATUS
gckOS_CreateSignal(
    IN gckOS Os,
    IN gctBOOL ManualReset,
    OUT gctSIGNAL * Signal
    )
{
    gcskSIGNAL_PTR signal;
    pthread_condattr_t cattr;
    int rc;

    gcmkHEADER_ARG("Os=0x%X ManualReset=%d", Os, ManualReset);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Create an event structure. */
    signal = (gcskSIGNAL_PTR) calloc(1, sizeof(gcskSIGNAL));
    if (signal == gcvNULL)
    {
        gcmkPRINT("[VIV]: %s:%d: calloc failed.\n", __FUNCTION__, __LINE__);
        goto fail_01;
    }

    signal->manual    = ManualReset;
    signal->pending   = 0;
    signal->received  = 0;
    signal->destroyed = gcvFALSE;

    /* Initialize the mutex. */
    rc = pthread_mutex_init(&signal->mutex, gcvNULL);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);
        goto fail_02;
    }

    /* Initialize the condition. */
    rc = pthread_condattr_init(&cattr);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);
        goto fail_03;
    }

    rc = pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);
        goto fail_04;
    }

    rc = pthread_cond_init(&signal->condition, &cattr);
    if (rc != EOK)
    {
        gcmkPRINT("[VIV]: %s:%d: rc=%d.\n", __FUNCTION__, __LINE__, rc);
        goto fail_04;
    }

    /* We do not need the attribute any more. */
    pthread_condattr_destroy(&cattr);

    *Signal = (gctSIGNAL) signal;

    gcmkFOOTER_ARG("*Signal=0x%X", *Signal);
    return gcvSTATUS_OK;

fail_04:
    pthread_condattr_destroy(&cattr);
fail_03:
    pthread_mutex_destroy(&signal->mutex);
fail_02:
    free(signal);
fail_01:
    gcmkFOOTER_NO();
    return gcvSTATUS_OUT_OF_RESOURCES;
}

/*******************************************************************************
**
**  gckOS_DestroySignal
**
**  Destroy a signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroySignal(
    IN gckOS Os,
    IN gctSIGNAL Signal
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcskSIGNAL_PTR signal;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X", Os, Signal);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    signal = (gcskSIGNAL_PTR) Signal;

    /* Acquire the mutex. */
    pthread_mutex_lock(&signal->mutex);

    if (signal->pending != signal->received)
    {
        signal->destroyed = gcvTRUE;

        /* Release the mutex. */
        pthread_mutex_unlock(&signal->mutex);
    }
    else
    {
        /* Destroy the signal structure. */
        _DestroySignal(signal);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gckOS_Signal
**
**  Set a state of the specified signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
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
gckOS_Signal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctBOOL State
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcskSIGNAL_PTR signal;
    gctINT rc;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X State=%d", Os, Signal, State);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Cast the handle to the signal structure. */
    signal = (gcskSIGNAL_PTR) Signal;

    /* Acquire the mutex. */
    rc = pthread_mutex_lock(&signal->mutex);
    if (rc != EOK)
    {
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
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

            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&signal->mutex);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_SignalPulse(
    IN gckOS Os,
    IN gctSIGNAL Signal
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcskSIGNAL_PTR signal;
    gctBOOL destroy;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X", Os, Signal);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    signal = (gcskSIGNAL_PTR) Signal;

    gcmkONERROR(gckOS_Signal(Os, Signal, gcvTRUE));

    /* Acquire the mutex. */
    pthread_mutex_lock(&signal->mutex);

    signal->received++;

    destroy = (signal->destroyed && (signal->pending == signal->received));

    if (destroy)
    {
        /* Destroy the signal structure. */
        _DestroySignal(signal);
    }
    else
    {
        /* Release the mutex. */
        pthread_mutex_unlock(&signal->mutex);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_SignalPending(
    IN gckOS Os,
    IN gctSIGNAL Signal
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcskSIGNAL_PTR signal;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X", Os, Signal);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    signal = (gcskSIGNAL_PTR) Signal;

    /* Acquire the mutex. */
    pthread_mutex_lock(&signal->mutex);

    /* Update the pending number. */
    signal->pending++;

    /* Release the mutex. */
    pthread_mutex_unlock(&signal->mutex);

    /* Success. */
    gcmkFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gckOS_UserSignal
**
**  Set the specified signal which is owned by a process to signaled state.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctHANDLE Process
**          Handle of process owning the signal.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UserSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctINT Rcvid,
    IN gctINT Coid
    )
{
    gctINT rc;
    gcsSignalHandle_PTR signal;
    struct sigevent event;

    gcmkHEADER_ARG("Os=0x%x Signal=0x%x Rcvid=%d Coid=%d", Os, Signal, Rcvid, Coid);

    signal = (gcsSignalHandle_PTR) Signal;

    drv_signal_mgr_lock();

    gcmkASSERT(signal->rcvid == Rcvid);
    gcmkASSERT(signal->coid  == Coid);

    if (signal->alive)
    {
        SIGEV_PULSE_INIT(&event, Coid, 21, GC_HAL_QNX_PULSEVAL_SIGNAL, (gctUINT32)signal->signal);

        rc = MsgDeliverEvent_r(Rcvid, &event);
        if (rc != EOK)
        {
            gcmkTRACE(gcvLEVEL_ERROR,
                     "%s(%d): MsgDeliverEvent failed (%d) error:%s.",
                     __FUNCTION__, __LINE__, rc, strerror(rc));
        }
    }

    drv_signal_mgr_del(signal);

    drv_signal_mgr_unlock();

    gcmkTRACE(gcvLEVEL_INFO,
             "%s(%d): Sent signal to (receive ID = %d, connect ID = %d).",
             __FUNCTION__, __LINE__, Rcvid, Coid);

    /* Return status. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_WaitSignal
**
**  Wait for a signal to become signaled.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
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
gckOS_WaitSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctBOOL Interruptable,
    IN gctUINT32 Wait
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcskSIGNAL_PTR signal;
    gctINT result;

    gcmkHEADER_ARG("Os=0x%x Signal=0x%x Wait=%d", Os, Signal, Wait);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Cast the handle to the signal structure. */
    signal = (gcskSIGNAL_PTR) Signal;

    /* Acquire the mutex. */
    result = pthread_mutex_lock(&signal->mutex);
    if (result != EOK)
    {
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
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
                gcmkONERROR(gcvSTATUS_TIMEOUT);
            }
            else
            {
                gcmkONERROR(gcvSTATUS_GENERIC_IO);
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
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_MapSignal
**
**  Map a signal in to the current process space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to tha gctSIGNAL to map.
**
**      gctHANDLE Process
**          Handle of process owning the signal.
**
**  OUTPUT:
**
**      gctSIGNAL * MappedSignal
**          Pointer to a variable receiving the mapped gctSIGNAL.
*/
gceSTATUS
gckOS_MapSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctHANDLE Process,
    OUT gctSIGNAL * MappedSignal
    )
{
    gcmkPRINT("[VIV]: ERROR: %s Not supported.\n", __FUNCTION__);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_CreateUserSignal
**
**  Create a new signal to be used in the user space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL ManualReset
**          If set to gcvTRUE, gckOS_Signal with gcvFALSE must be called in
**          order to set the signal to nonsignaled state.
**          If set to gcvFALSE, the signal will automatically be set to
**          nonsignaled state by gckOS_WaitSignal function.
**
**  OUTPUT:
**
**      gctINT * SignalID
**          Pointer to a variable receiving the created signal's ID.
*/
gceSTATUS
gckOS_CreateUserSignal(
    IN gckOS Os,
    IN gctBOOL ManualReset,
    OUT gctINT * SignalID
    )
{
    gcmkASSERT(0);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_DestroyUserSignal
**
**  Destroy a signal to be used in the user space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctINT SignalID
**          The signal's ID.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroyUserSignal(
    IN gckOS Os,
    IN gctINT SignalID
    )
{
    gcmkASSERT(0);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_WaitUserSignal
**
**  Wait for a signal used in the user mode to become signaled.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctINT SignalID
**          Signal ID.
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
gckOS_WaitUserSignal(
    IN gckOS Os,
    IN gctINT SignalID,
    IN gctUINT32 Wait
    )
{
    gcmkASSERT(0);
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_SignalUserSignal
**
**  Set a state of the specified signal to be used in the user space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctINT SignalID
**          SignalID.
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
gckOS_SignalUserSignal(
    IN gckOS Os,
    IN gctINT SignalID,
    IN gctBOOL State
    )
{
    gcmkASSERT(0);
    return gcvSTATUS_NOT_SUPPORTED;
}
#if gcdENABLE_VG

gceSTATUS
gckOS_CreateSemaphoreVG(
    IN gckOS Os,
    OUT gctSEMAPHORE * Semaphore
    )
{
    gceSTATUS status;
    sem_t *sem = gcvNULL;

    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Allocate the semaphore structure. */
    gcmkONERROR(
        gckOS_Allocate(Os, gcmSIZEOF(*sem), (gctPOINTER *) &sem));

    /* Initialize the semaphore. */
    if (sem_init(sem, 0, 0) != 0)
    {
        gcmkPRINT("[VIV]: %s:%d: errno=%d.\n",
              __FUNCTION__, __LINE__, (int)errno);

        gckOS_Free(Os, sem);
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    /* Return to caller. */
    *Semaphore = (gctPOINTER) sem;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_IncrementSemaphore(
    IN gckOS Os,
    IN gctSEMAPHORE Semaphore
    )
{
    gcmkHEADER_ARG("Os=0x%X Semaphore=0x%x", Os, Semaphore);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Release the semaphore. */
    sem_post((sem_t *)Semaphore);

    gcmkFOOTER_NO();
    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_DecrementSemaphore(
    IN gckOS Os,
    IN gctSEMAPHORE Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Semaphore=0x%x", Os, Semaphore);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Acquire the semaphore. */
    if (sem_wait((sem_t *)Semaphore))
    {
        gcmkONERROR(gcvSTATUS_TIMEOUT);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}
/******************************************************************************\
******************************** Thread Object *********************************
\******************************************************************************/

gceSTATUS
gckOS_StartThread(
    IN gckOS Os,
    IN gctTHREADFUNC ThreadFunction,
    IN gctPOINTER ThreadParameter,
    OUT gctTHREAD * Thread
    )
{
    gceSTATUS status;
    pthread_t tid;
    gctINT ret;

    gcmkHEADER_ARG("Os=0x%X ", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(ThreadFunction != gcvNULL);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    ret = pthread_create(&tid, gcvNULL, ThreadFunction, ThreadParameter);
    if (EOK != ret)
    {
        gcmkPRINT("[VIV]: %s:%d: ret=%d.\n", __FUNCTION__, __LINE__, ret);

        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    pthread_setname_np(tid, "Vivante Kernel Thread");

    *Thread = (gctTHREAD)gcmINT2PTR(tid);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_StopThread(
    IN gckOS Os,
    IN gctTHREAD Thread
    )
{
    gcmkHEADER_ARG("Os=0x%X Thread=0x%x", Os, Thread);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    /* Thread should have already been enabled to terminate. */
    pthread_join((pthread_t)gcmPTR2INT(Thread), gcvNULL);

    gcmkFOOTER_NO();
    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_VerifyThread(
    IN gckOS Os,
    IN gctTHREAD Thread
    )
{
    gcmkHEADER_ARG("Os=0x%X Thread=0x%x", Os, Thread);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    gcmkFOOTER_NO();
    /* Success. */
    return gcvSTATUS_OK;
}
#endif

gceSTATUS
gckOS_GetProcessNameByPid(
    IN gctINT Pid,
    IN gctSIZE_T Length,
    OUT gctUINT8_PTR String
    )
{
    gceSTATUS status;
    char buffer[128];
    int fd = -1;

    struct
    {
        procfs_debuginfo    info;
        char                path[PATH_MAX];
    } dinfo;

    extern char * __progname;

    gcmkHEADER_ARG("Pid=%d Length=%u", Pid, Length);

    gcmkVERIFY_ARGUMENT(String != gcvNULL);

    if ((gctINT) getpid() != Pid)
    {
        /* Construct the proc entry. */
        sprintf(buffer, "/proc/%d/as", (gctINT32)Pid);

        /* Open the proc entry. */
        fd = open(buffer, O_RDONLY);
        if (fd == -1)
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        /* Query the path. */
        if (devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &dinfo, gcmSIZEOF(dinfo), gcvNULL) != EOK)
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        /* Close the fd. */
        close(fd);
        fd = -1;

        /* Copy the string. */
        strncpy((char *)String, basename(dinfo.info.path), Length - 1);
        String[Length - 1] = 0;
    }
    else
    {
        /* Copy the string. */
        strncpy((char *)String, __progname, Length - 1);
        String[Length - 1] = 0;
    }

    gcmkFOOTER_ARG("String=%s", String);

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    if (fd >= 0)
    {
        close(fd);
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_DumpCallStack(
    IN gckOS Os
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_PrepareGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_FinishGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_QueryGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core,
    OUT gctUINT32 * Frequency,
    OUT gctUINT8 * Scale
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_SetGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT8 Scale
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_CreateKernelVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR node;
    gctPOINTER logical;

    gcmkHEADER_ARG("Physical=0x%x Bytes=%u.", Physical, Bytes);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node != gcvNULL);
    gcmkASSERT(node->fd != 0);

    if (node->type != gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (node->kernelLogical == gcvNULL)
    {
        gcmkASSERT(node->kernelMapCount == 0);

        /* Map the memory into the kernel space. */
        logical = mmap64(gcvNULL, Bytes, PROT_READ | PROT_WRITE | PROT_NOCACHE,
                         MAP_SHARED, node->fd, 0);

        /* Map failed. */
        if (logical == MAP_FAILED)
        {
            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        /* Update the node. */
        node->kernelLogical = logical;
    }
    else
    {
        /* It's already mapped. */
        logical = node->kernelLogical;
    }

    node->kernelMapCount++;

    if (Logical != gcvNULL)
    {
        *Logical = logical;
    }

    if (PageCount != gcvNULL)
    {
        *PageCount = node->pageCount;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_DestroyKernelVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR node;

    gcmkHEADER_ARG("Logical=0x%x Bytes=%u.", gcmPTR2INT32(Logical), Bytes);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node != gcvNULL);
    gcmkASSERT(node->fd != 0);
    gcmkASSERT(node->kernelLogical != gcvNULL);
    gcmkASSERT(node->kernelLogical == Logical);
    gcmkASSERT(node->kernelMapCount > 0);

    if (node->type != gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (--node->kernelMapCount == 0)
    {
        /* Unmap the memory. */
        if (munmap(Logical, Bytes) == -1)
        {
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        /* Update the node. */
        node->kernelLogical = gcvNULL;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_CreateUserVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR node;
    gctUINT32 pid = 0;
    gctPOINTER userLogical, kernelLogical;

    gcmkHEADER_ARG("Physical=0x%x Bytes=%u.", Physical, Bytes);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node != gcvNULL);
    gcmkASSERT(node->userLogical == gcvNULL);
    gcmkASSERT(node->kernelLogical == gcvNULL);
    gcmkASSERT(node->kernelMapCount == 0);
    gcmkASSERT(node->fd != 0);

    if (node->type != gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Get current process id. */
    gcmkVERIFY_OK(gckOS_GetProcessID(&pid));

    /* Map the memory into the kernel space. */
    kernelLogical = mmap64(gcvNULL, Bytes, PROT_READ | PROT_WRITE | PROT_NOCACHE,
                     MAP_SHARED, node->fd, 0);

    /* Map failed. */
    if (kernelLogical == MAP_FAILED)
    {
        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Map the memory into the user space. */
    userLogical = mmap64_peer(pid,
                              0,
                              Bytes,
                              PROT_READ | PROT_WRITE | PROT_NOCACHE,
                              MAP_SHARED,
                              node->fd,
                              0);

    /* Map failed. */
    if (userLogical == MAP_FAILED)
    {
        munmap(kernelLogical, Bytes);

        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Update the node. */
    node->userLogical   = userLogical;
    node->kernelLogical = kernelLogical;
    node->kernelMapCount++;
    node->pid           = pid;

    drv_physical_map_add_node(node);

    if (Logical != gcvNULL)
    {
        *Logical = userLogical;
    }

    if (PageCount != gcvNULL)
    {
        *PageCount = node->pageCount;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_DestroyUserVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gceSTATUS status;
    gcsPHYSICAL_PTR node;

    gcmkHEADER_ARG("Logical=0x%x Bytes=%u.", gcmPTR2INT32(Logical), Bytes);

    /* Cast the handle to the node structure. */
    node = (gcsPHYSICAL_PTR)Physical;

    /* Verify the node. */
    gcmkASSERT(node != gcvNULL);
    gcmkASSERT(node->pid != 0);
    gcmkASSERT(node->userLogical != gcvNULL);
    gcmkASSERT(node->userLogical == Logical);
    gcmkASSERT(node->kernelLogical != gcvNULL);
    gcmkASSERT(node->kernelMapCount == 1);
    gcmkASSERT(node->fd != 0);

    if (node->type != gcvPHYSICAL_TYPE_PAGED_MEMORY)
    {
        gcmkPRINT("[VIV]: %s:%d: invalid type: %d.\n", __FUNCTION__, __LINE__, node->type);
        gcmkASSERT(0);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Unmap the memory. */
    munmap(node->kernelLogical, Bytes);
    munmap_peer(node->pid, Logical, Bytes);

    /* Update the node. */
    node->pid            = 0;
    node->userLogical    = gcvNULL;
    node->kernelLogical  = gcvNULL;
    node->kernelMapCount = 0;

    drv_physical_map_delete_node(node);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_CPUPhysicalToGPUPhysical(
    IN gckOS Os,
    IN gctPHYS_ADDR_T CPUPhysical,
    IN gctPHYS_ADDR_T * GPUPhysical
    )
{
    gcmkHEADER_ARG("CPUPhysical=0x%X", CPUPhysical);

    *GPUPhysical = CPUPhysical;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_GPUPhysicalToCPUPhysical(
    IN gckOS Os,
    IN gctUINT32 GPUPhysical,
    IN gctPHYS_ADDR_T * CPUPhysical
    )
{
    gcmkHEADER_ARG("GPUPhysical=0x%X", GPUPhysical);

    *CPUPhysical = GPUPhysical;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_GetFd(
    IN gctSTRING Name,
    IN gcsFDPRIVATE_PTR Private,
    OUT gctINT * Fd
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_QueryOption(
    IN gckOS Os,
    IN gctCONST_STRING Option,
    OUT gctUINT32 * Value
    )
{
    gckGALDEVICE device = Os->device;

    if (!strcmp(Option, "physBase"))
    {
        *Value = device->physBase;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "physSize"))
    {
        *Value = device->physSize;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "mmu"))
    {
        *Value = device->args.mmu;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "contiguousSize"))
    {
        *Value = device->contiguousSize;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "contiguousBase"))
    {
        *Value = (gctUINT32)device->contiguousBase;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "recovery"))
    {
        *Value = device->args.recovery;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "stuckDump"))
    {
        *Value = device->args.stuckDump;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "powerManagement"))
    {
        *Value = device->args.powerManagement;
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "gpuProfiler"))
    {
        *Value = device->args.gpuProfiler;
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_WrapMemory(
    IN gckOS Os,
    IN gcsUSER_MEMORY_DESC_PTR Desc,
    OUT gctSIZE_T *Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctBOOL *Contiguous
    )
{
    gceSTATUS status;
    gcsPHYSICAL_PTR node;
    gctUINT32 address;
    gctBOOL acquired = gcvFALSE;
    gctBOOL nodeAllocated = gcvFALSE;

    gcmkHEADER_ARG("Os=0x%X Desc=0x%X logical=0x%X, physical=0x%x, size=%d", \
        Os, Desc, (gctUINT32)Desc->logical, Desc->physical, Desc->size);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Desc != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Contiguous != gcvNULL);

    /* Verify the user memory info. */
    if ((Desc->physical == gcvINVALID_ADDRESS) && (Desc->logical == 0))
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Desc->size == 0)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the lock. */
    MEMORY_LOCK(Os);
    acquired = gcvTRUE;

    /* Allocate a physical node. */
    gcmkONERROR(drv_physical_allocate_node(&node));
    nodeAllocated = gcvTRUE;

    /* Setup the node structure. */
    node->type            = gcvPHYSICAL_TYPE_WRAPPED_MEMORY;
    node->physicalAddress = Desc->physical;
    node->userLogical     = gcmINT2PTR(Desc->logical);
    node->bytes           = Desc->size;

    if (Desc->physical != gcvINVALID_ADDRESS)
    {
        address = Desc->physical;

        node->contiguous = gcvTRUE;
    }
    else
    {
        address = gcmPTR2INT32(node->userLogical);

        gcmkONERROR(_IsMemoryContiguous(
            Os,
            node->userLogical,
            node->bytes,
            &node->contiguous,
            gcvNULL));
    }

    node->pageCount = _GetPageCount(gcmINT2PTR(address), Desc->size);

#if gcdENABLE_2D
    node->extraPage = 2;
#else
    node->extraPage = (
            ((address + gcmALIGN(node->bytes + 64, 64) + __PAGESIZE - 1) >> 12) >
            ((address + node->bytes + __PAGESIZE - 1) >> 12)
            ) ? 1 : 0;
#endif

    /* Return values. */
    *Bytes      = Desc->size;
    *Physical   = (gctPHYS_ADDR)node;
    *Contiguous = node->contiguous;

    /* Release the lock. */
    MEMORY_UNLOCK(Os);

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=%d, *Physical=0x%08x, *Contiguous=%d", \
        gcmOPT_VALUE(Bytes), gcmOPT_VALUE(Physical), gcmOPT_VALUE(Contiguous));
    return gcvSTATUS_OK;

OnError:
    if (nodeAllocated)
    {
        drv_physical_free_node(node);
    }

    if (acquired)
    {
        MEMORY_UNLOCK(Os);
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_PhysicalToPhysicalAddress(
    IN gckOS Os,
    IN gctPOINTER Physical,
    IN gctUINT32 Offset,
    OUT gctPHYS_ADDR_T * PhysicalAddress
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_GetPolicyID(
    IN gckOS Os,
    IN gceSURF_TYPE Type,
    OUT gctUINT32_PTR PolicyID,
    OUT gctUINT32_PTR AXIConfig
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

