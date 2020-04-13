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


#include "gc_hal_kernel_qnx.h"
#include "shared/gc_hal_driver.h"

#include <screen/gpu.h>

#ifndef IMX
#include <platform_config.h>
#endif

/* for ARM_PTE_... */
#include <arm/mmu.h>

/* for __khrGetDeviceConfigValue */
#include <KHR/khronos_utils.h>

/* for slogf */
#include <sys/slogcodes.h>

/* for timing */
#include <time.h>

#define MAX_EEXIST_RETRIES 5

static gckGALDEVICE galDevice;

#if defined(AARCH64)
static int posix_mem_fd = -1;
#endif

/*----------------------------------------------------------------------------
 * Device configuration -
 *      defines set in build_qnx/platform_config/<platform>/platform_config.h
 *----------------------------------------------------------------------------
 */
#if defined(IMX)

  int hasPL310 = 0;

int             irqLine             = -1;
long            registerMemBase     = -1;
unsigned long   registerMemSize     = 0;

#ifdef gcdDUAL_CORE
  int             irqLine3D1          = -1;
  long            registerMemBase3D1  = -1;
  unsigned long   registerMemSize3D1  = 0;
#endif

unsigned long   baseAddress         = 0;

int             irqLine2D           = -1;
long            registerMemBase2D   = -1;
unsigned long   registerMemSize2D   = 0;

int             irqLineVG           = -1;
long            registerMemBaseVG   = -1;
unsigned long   registerMemSizeVG   = 0;

static int      powerManagement     = -1;
static int      gpuProfiler         = -1;
static unsigned long   physBase     = 0;
static unsigned int    physSize     = 0;

#else
int             irqLine             = gcd3D_IRQ;
long            registerMemBase     = gcd3D_REG_BASE;
unsigned long   registerMemSize     = gcd3D_REG_SIZE;

#ifdef gcdDUAL_CORE
int             irqLine3D1          = gcd3D1_IRQ;
long            registerMemBase3D1  = gcd3D1_REG_BASE;
unsigned long   registerMemSize3D1  = gcd3D1_REG_SIZE;
#endif

unsigned long   baseAddress         = gcdDEVICE_BASE_ADDRESS;

int             irqLine2D           = gcd2D_IRQ;
long            registerMemBase2D   = gcd2D_REG_BASE;
unsigned long   registerMemSize2D   = gcd2D_REG_SIZE;

int             irqLineVG           = gcdVG_IRQ;
long            registerMemBaseVG   = gcdVG_REG_BASE;
unsigned long   registerMemSizeVG   = gcdVG_REG_SIZE;

static int      powerManagement     = gcdPOWER_MANAGEMENT;
static int      gpuProfiler         = gcdGPUPROFILER;

static unsigned long   physBase     = gcdMMU_PhysicalMemoryBase;
static unsigned int    physSize     = gcdMMU_PhysicalMemorySize;
#endif

/* Configurable Memory sizes. */
unsigned long contiguousSize        = (248 << 20);      /* Video memory pool. */

unsigned int internalPoolSize       = (  6 << 20);      /* Kernel local memory pool. */
unsigned int sharedPoolSize         = (  2 << 20);      /* Shared per-client memory pool initial size. */
unsigned int sharedPoolPageSize     = (  1 << 12);      /* Shared per-client memory pool page size. */
unsigned int internalPoolPageSize   = (  1 << 12);      /* Kernel local memory pool page size. */

unsigned int slogUsageInterval = 0;                     /* slog video memory pool usage every N seconds, 0 disables. */
unsigned int contiguousAbort = 0;                       /* abort() if contiguous pool exhausted. */

/* ContiguousBase should be 0,
 * for video memory to be allocated from the memory pool. */
static unsigned long contiguousBase = 0;

static long bankSize                = 0;
static int fastClear                = -1;
static int compression              = -1;

static unsigned int recovery        = 1;    /* Recover GPU from stuck (1: Enable, 0: Disable) */
static unsigned int stuckDump       = 1;    /* Level of stuck dump content (1: Minimal, 2: Middle, 3: Maximal) */

static int mmu                      = 1;    /* Enable mmu or not (only for new mmu). */

/*----------------------------------------------------------------------------*/

/*----------------------------- Timing helper --------------------------------*/
#define DEFINE_CLOCK(clock) struct timespec clock

#define START_CLOCK(clock) clock_gettime(CLOCK_REALTIME, &clock)

#define STOP_CLOCK(clock, message, ...) \
    do { \
          double start_s = (clock).tv_sec + ((clock).tv_nsec / 1000000000.0); \
          clock_gettime(CLOCK_REALTIME, &(clock)); \
          double stop_s = (clock).tv_sec + ((clock).tv_nsec / 1000000000.0); \
          slogf(_SLOGC_GRAPHICS_GL, _SLOG_INFO, "CLOCK: %gs " message, stop_s - start_s, ##__VA_ARGS__); \
    } while (0)


/*----------------------------------------------------------------------------*/

/*---------------------------- User Signal Management ------------------------*/

static pthread_mutex_t signalListLock = PTHREAD_MUTEX_INITIALIZER;
static LIST_HEAD(_gcsSignalList, _gcsSignalHandle) signalList, signalFreeList;

void
drv_signal_mgr_init()
{
    LIST_INIT(&signalList);
    LIST_INIT(&signalFreeList);
}

void
drv_signal_mgr_fini()
{
    gcsSignalHandle_PTR p;

    pthread_mutex_lock(&signalListLock);

    if (!LIST_EMPTY(&signalFreeList))
    {
        p = LIST_FIRST(&signalFreeList);
        LIST_REMOVE(p, node);

        free(p);
    }

    if (!LIST_EMPTY(&signalList))
    {
        p = LIST_FIRST(&signalList);
        LIST_REMOVE(p, node);

        free(p);
    }

    pthread_mutex_unlock(&signalListLock);
}

void
drv_signal_mgr_lock()
{
    pthread_mutex_lock(&signalListLock);
}

void
drv_signal_mgr_unlock()
{
    pthread_mutex_unlock(&signalListLock);
}

gceSTATUS
drv_signal_mgr_add(
    IN gctUINT32 Pid,
    IN gctINT32 Coid,
    IN gctINT32 Rcvid,
    IN gctUINT64 Signal,
    OUT gctPOINTER *Handle
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSignalHandle_PTR p = gcvNULL;

    /* Acquire the lock. */
    drv_signal_mgr_lock();

    if (!LIST_EMPTY(&signalFreeList))
    {
        /* Get a handle from the free list. */
        p = LIST_FIRST(&signalFreeList);
        LIST_REMOVE(p, node);
    }
    else
    {
        /* Get a handle from the heap. */
        p = (gcsSignalHandle_PTR)malloc(gcmSIZEOF(*p));
        if (p == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    }

    /* Init the handle structure. */
    p->pid    = Pid;
    p->coid   = Coid;
    p->rcvid  = Rcvid;
    p->alive  = gcvTRUE;
    p->signal = Signal;

    /* Add into the signal list. */
    LIST_INSERT_HEAD(&signalList, p, node);

    /* Save the handle. */
    *Handle = p;

OnError:
    /* Release the lock. */
    drv_signal_mgr_unlock();

    return status;
}

/*
 * @NOTE: The caller must acquire the lock first.
 */
void
drv_signal_mgr_del(
    IN gcsSignalHandle_PTR Signal
    )
{
    gcmkASSERT(Signal != gcvNULL);

    /* Detach the handle from the signal list. */
    LIST_REMOVE(Signal, node);

    /* Reset the handle. */
    memset(Signal, 0, gcmSIZEOF(*Signal));

    /* Add into the free list. */
    LIST_INSERT_HEAD(&signalFreeList, Signal, node);
}

gceSTATUS
drv_signal_mgr_update_iface(
    IN gctUINT32 Pid,
    IN gctINT32 Rcvid,
    IN gcsHAL_INTERFACE *iface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER signal;

    switch (iface->command)
    {
    case gcvHAL_SIGNAL:
        {
            gcmkONERROR(drv_signal_mgr_add(
                    Pid,
                    iface->u.Signal.coid,
                    Rcvid,
                    iface->u.Signal.signal,
                    &signal));

            iface->u.Signal.rcvid  = Rcvid;
            iface->u.Signal.signal = gcmPTR_TO_UINT64(signal);
        }
        break;

    default:
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

/*---------------------------- Global Memory Pool ----------------------------*/
/* Global video memory pool. */
typedef struct _gcsMEM_POOL
{
    gctINT32 freePage;
    gctSIZE_T pageCount;
    gctUINT32 pageSize;
    gctUINT32 poolSize;
    pthread_mutex_t mutex;
    gctPOINTER addr;
    gctPHYS_ADDR_T paddr;
    gckPAGE_USAGE pageUsage;
    gctINT fd;
} gcsMEM_POOL;

gcsMEM_POOL memPool;

/* Reference count handling per pid.
 * To invoke startup/cleanup on first and last connection respectively.
 */
struct pid_hash
{
    pid_t pid;
    int refCount;
    struct pid_hash* next;
};
struct pid_hash* pid_hash_head;

void
gckOS_DumpParam(
    void
    )
{
#define printk(format...) \
    slogf(_SLOGC_GRAPHICS_DISPLAY, _SLOG_INFO, "" format)

    printk("Galcore options:");

    printk("  irqLine           = %d",      irqLine);
    printk("  registerMemBase   = 0x%08lX", registerMemBase);
    printk("  registerMemSize   = 0x%08lX", registerMemSize);

#ifdef gcdDUAL_CORE
    if (irqLine3D1 != -1)
    {
      printk("  irqLine3D1           = %d",      irqLine3D1);
      printk("  registerMemBase3D1   = 0x%08lX", registerMemBase3D1);
      printk("  registerMemSize3D1   = 0x%08lX", registerMemSize3D1);
    }
#endif

    if (irqLine2D != -1)
    {
        printk("  irqLine2D         = %d",      irqLine2D);
        printk("  registerMemBase2D = 0x%08lX", registerMemBase2D);
        printk("  registerMemSize2D = 0x%08lX", registerMemSize2D);
    }

    if (irqLineVG != -1)
    {
        printk("  irqLineVG         = %d",      irqLineVG);
        printk("  registerMemBaseVG = 0x%08lX", registerMemBaseVG);
        printk("  registerMemSizeVG = 0x%08lX", registerMemSizeVG);
    }

    printk("  contiguousBase    = 0x%08lX", contiguousBase);
    printk("  contiguousSize    = %lu MB",  contiguousSize >> 20);
    printk("  slogUsageInterval = %d sec",  slogUsageInterval);
    printk("  internalPoolMB    = %u MB",   internalPoolSize >> 20);
    printk("  sharedPoolMB      = %u MB",   sharedPoolSize   >> 20);
    printk("  bankSize          = 0x%08lX", bankSize);
    printk("  fastClear         = %d",      fastClear);
    printk("  compression       = %d",      compression);
    printk("  powerManagement   = %d",      powerManagement);
    printk("  gpuProfiler       = %d",      gpuProfiler);
    printk("  baseAddress       = 0x%08lX", baseAddress);
    printk("  physBase          = 0x%08lX", (unsigned long)physBase);
    printk("  physSize          = 0x%08X",  physSize);
    printk("  recovery          = %d",      recovery);
    printk("  stuckDump         = %d",      stuckDump);
    printk("  mmu               = %d",      mmu);
}

gceSTATUS
drv_mempool_init()
{
    off64_t paddr;
    void* addr;
    int rc, err;
#if defined(AARCH64)
    size_t pt_contig_len = 0;
    int pt_fildes = NOFD;
    off64_t pt_offset;
#else
    size_t pcontig;
#endif

    DEFINE_CLOCK(clock);
    DEFINE_CLOCK(total);

#if defined(AARCH64)
    posix_mem_fd = posix_typed_mem_open("/memory/below4G/ram/sysram", O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG);
    if (posix_mem_fd < 0) {
        fprintf(stderr, "galcore:%s[%d]: posix_typed_mem_open failed\n", __FUNCTION__, __LINE__);
        return gcvSTATUS_GENERIC_IO;
    }
#endif

    memPool.pageSize = internalPoolPageSize;

    /* Compute number of pages. */
    memPool.pageCount = (contiguousSize + internalPoolSize) / memPool.pageSize;
    gcmkASSERT(memPool.pageCount <= 65536);

    /* Align memPoolSize to page size. */
    memPool.poolSize = memPool.pageCount * memPool.pageSize;
    /*fprintf(stderr, "memPoolSize: %d\n", memPool.poolSize);*/

    START_CLOCK(total);
    /* Allocate a single chunk of physical memory.
     * Zero memory with MAP_ANON so we don't leak any sensitive information by chance. */
    START_CLOCK(clock);
    memPool.fd = drv_create_shm_object();
    STOP_CLOCK(clock, "drv_create_shm_object");
    if (memPool.fd == -1) {
        fprintf(stderr, "galcore:%s[%d]: shm_open failed\n", __FUNCTION__, __LINE__);
        return gcvSTATUS_GENERIC_IO;
    }

#if defined(AARCH64)
    /* Map this memory inside user and galcore. */
    unsigned prot_flags = PROT_READ|PROT_WRITE;
    unsigned map_flags = 0;

    START_CLOCK(clock);
    addr = mmap64(0, memPool.poolSize, prot_flags, MAP_SHARED|MAP_NOINIT, posix_mem_fd, 0);
    err = errno;
    STOP_CLOCK(clock, "mmap64 prot_flags = 0x%x", prot_flags);
    if (addr == MAP_FAILED) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "galcore:%s[%d]: mmap64 failed: %s", __FUNCTION__, __LINE__, strerror(err));
        close(memPool.fd);
        memPool.fd = -1;
        return gcvSTATUS_GENERIC_IO;
    }

    START_CLOCK(clock);
    rc = posix_mem_offset64(addr, memPool.poolSize, &pt_offset, &pt_contig_len, &pt_fildes);
    err = errno;
    STOP_CLOCK(clock, "posix_mem_offset64");
    if (rc != 0) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "galcore:%s[%d]: posix_mem_offset64 failed: %s", __FUNCTION__, __LINE__, strerror(err));
        munmap(addr, memPool.poolSize);
        close(memPool.fd);
        memPool.fd = -1;
        return gcvSTATUS_GENERIC_IO;
    }
    paddr = pt_offset;
    /* Overlay the shared memory object over the posix memory. Set as shareable and write through. */
    /* cacheable, write back == lazy write */
    START_CLOCK(clock);
    rc = shm_ctl_special(memPool.fd, SHMCTL_PHYS, pt_offset, memPool.poolSize, ARM_SHMCTL_WT | ARM_SHMCTL_SH);
    err = errno;
    STOP_CLOCK(clock, "shm_ctl_special poolSize = %u (%uMB)", memPool.poolSize, memPool.poolSize / 1048576);
    if (rc == -1) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "galcore:%s[%d]: shm_ctl_special failed: %s", __FUNCTION__, __LINE__, strerror(err));
        munmap(addr, memPool.poolSize);
        close(memPool.fd);
        memPool.fd = -1;
        return gcvSTATUS_GENERIC_IO;
    }

    {
        void *taddr = mmap64( 0, memPool.poolSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NOINIT, memPool.fd, 0);
        if( taddr == MAP_FAILED ) {
            munmap(addr,memPool.poolSize);
            close(memPool.fd);
            memPool.fd = -1;
            return gcvSTATUS_GENERIC_IO;
        }
        // TODO: Check for errors.
        mprotect( addr, memPool.poolSize, PROT_NONE );
        addr = taddr;
    }

#else
#if defined(IMX6X) || defined(IMX8X) || defined(IMX)
     START_CLOCK(clock);

    /*
     * use SHMCTL_LAZYWRITE to get write-combine memory access
      */
    rc = shm_ctl(memPool.fd, SHMCTL_ANON|SHMCTL_PHYS|SHMCTL_LAZYWRITE, 0, memPool.poolSize);
    err = errno;
    STOP_CLOCK(clock, "shm_ctl poolSize = %u (%uMB)", memPool.poolSize, memPool.poolSize / 1048576);
    if (rc == -1) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "galcore:%s[%d]: shm_ctl failed: %s", __FUNCTION__, __LINE__, strerror(err));
        close(memPool.fd);
        memPool.fd = -1;
        return gcvSTATUS_GENERIC_IO;
    }
    unsigned prot_flags = PROT_READ | PROT_WRITE;
#else /* fallback: it is for OMAP4/5, LAZYWRITE causes lock-ups. */
    START_CLOCK(clock);
       rc = shm_ctl(memPool.fd, SHMCTL_ANON|SHMCTL_PHYS, 0, memPool.poolSize);
    err = errno;
    STOP_CLOCK(clock, "shm_ctl_special poolSize = %u (%uMB)", memPool.poolSize, memPool.poolSize / 1048576);
    if (rc == -1) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "galcore:%s[%d]: shm_ctl_special failed: %s", __FUNCTION__, __LINE__, strerror(err));
        close(memPool.fd);
        memPool.fd = -1;
        return gcvSTATUS_GENERIC_IO;
    }

    unsigned prot_flags = PROT_READ | PROT_WRITE | PROT_NOCACHE;
#endif

    unsigned map_flags = MAP_NOINIT;
    while (1) {
      START_CLOCK(clock);
      addr = mmap64(0, memPool.poolSize, prot_flags, map_flags | MAP_SHARED, memPool.fd, 0);
      err = errno;
      STOP_CLOCK(clock, "mmap64 prot_flags = 0x%x map_flags = 0x%x", prot_flags, map_flags);
      if (addr == MAP_FAILED) {
        fprintf(stderr, "galcore:%s[%d]: mmap64 failed, errno=%d (%s)\n", __FUNCTION__, __LINE__, err, strerror(err));
        if (err != EINTR && err != EAGAIN) {
          close(memPool.fd);
          memPool.fd = -1;
          return gcvSTATUS_GENERIC_IO;
        }
      } else {
          break;
      }
    }
#endif
    memPool.addr = addr;

#if defined(AARCH64)
    paddr = pt_offset;
#else
    while (1) {
      START_CLOCK(clock);
      if (mem_offset64(addr, NOFD, memPool.poolSize, &paddr, &pcontig) == -1) {
        fprintf(stderr, "galcore:%s[%d]: mem_offset64 failed, errno=%d (%s)\n", __FUNCTION__, __LINE__, errno, strerror(errno));
        if (errno != EINTR && errno != EAGAIN) {
          munmap(addr, memPool.poolSize);
          close(memPool.fd);
          memPool.fd = -1;
          memPool.addr = NULL;
          return gcvSTATUS_GENERIC_IO;
        }
      } else {
        STOP_CLOCK(clock, "mem_offset64");
        break;
      }
    }
#endif

#if defined(IMX6X) || defined(IMX8X) || defined(IMX)
    STOP_CLOCK(total, "total poolSize = %uMB%s%s", memPool.poolSize / 1048576, prot_flags & PROT_NOCACHE ? " PROT_NOCACHE" : "", map_flags & MAP_NOINIT ? " MAP_NOINIT" : "");
#else
    STOP_CLOCK(total, "total poolSize = %uMB%s%s", memPool.poolSize / 1048576, prot_flags & PROT_NOCACHE ? " PROT_NOCACHE" : "",
       map_flags & MAP_NOINIT ? " MAP_NOINIT" : "");
#endif

    memPool.paddr = (gctPHYS_ADDR_T)paddr;

    fprintf(stderr, "Mempool Map addr range[%p-%p]\n", memPool.addr, memPool.addr +  memPool.poolSize);
    fprintf(stderr, "Mempool Map paddr range[%lx-%lx]\n", (long)memPool.paddr, (long)memPool.paddr +  memPool.poolSize );

    /* Allocate the page usage array and Initialize all pages to free. */
    memPool.pageUsage = (gckPAGE_USAGE)calloc(
            memPool.pageCount,
            sizeof(struct _gckPAGE_USAGE));

    if (memPool.pageUsage == gcvNULL)
    {
        fprintf( stderr, "malloc failed: %s\n", strerror( errno ) );
        munmap(addr, memPool.poolSize);
        close(memPool.fd);
        memPool.fd = -1;
        memPool.addr = NULL;
        memPool.paddr = 0;
        return gcvSTATUS_GENERIC_IO;
    }

    /* The first page is free.*/
    memPool.freePage = 0;

    /* Initialize the semaphore. */
    if (pthread_mutex_init(&memPool.mutex, gcvNULL) != EOK)
    {
        free(memPool.pageUsage);
        munmap(addr, memPool.poolSize);
        close(memPool.fd);
        memPool.fd = -1;
        memPool.addr = NULL;
        memPool.paddr = 0;
        return gcvSTATUS_GENERIC_IO;
    }

    return gcvSTATUS_OK;
}

void
drv_mempool_destroy()
{
    pthread_mutex_destroy(&memPool.mutex);
    free(memPool.pageUsage);
    memPool.pageUsage = gcvNULL;
    munmap((void*)memPool.addr, memPool.poolSize);
    close(memPool.fd);
    memPool.fd = -1;
    memPool.addr = NULL;
    memPool.paddr = 0;
#if defined(AARCH64)
    if (posix_mem_fd != -1) {
        close(posix_mem_fd);
    }
#endif
}

gctINT
drv_mempool_get_fileDescriptor()
{
    return memPool.fd;
}

gctPHYS_ADDR_T
drv_mempool_get_basePAddress()
{
    return memPool.paddr;
}

gctPOINTER
drv_mempool_get_baseAddress()
{
    return memPool.addr;
}

gctUINT32
drv_mempool_get_page_size()
{
    return memPool.pageSize;
}

gceSTATUS
drv_mempool_mem_offset(
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address)
{
    if (Address == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ((Logical < memPool.addr) ||
        (Logical >= (memPool.addr + memPool.poolSize)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    *Address = (gctPHYS_ADDR_T)(Logical - memPool.addr) + memPool.paddr;

    return gcvSTATUS_OK;
}

gceSTATUS
drv_mempool_get_kernel_logical(
    IN gctPHYS_ADDR_T Address,
    OUT gctPOINTER *Logical)
{
    if (Logical == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ((Address < memPool.paddr) ||
        (Address >= (memPool.paddr + memPool.poolSize)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    *Logical = (Address - memPool.paddr) + memPool.addr;

    return gcvSTATUS_OK;
}

/* Allocate pages from mapped shared memory.
   Return Physical and Logical addresses.
*/
void
drv_mempool_alloc_contiguous(
    IN gctUINT32 Bytes,
    OUT gctPHYS_ADDR_T * Physical,
    OUT gctPOINTER * Logical
    )
{
    gctSIZE_T i, j;
    gctSIZE_T pageCount;

    pthread_mutex_lock(&memPool.mutex);

    /* Compute the number of required pages. */
    pageCount = gcmALIGN(Bytes, drv_mempool_get_page_size()) / drv_mempool_get_page_size();

    if ( (pageCount <= 0) || (memPool.freePage < 0) )
    {
        /*fprintf(stderr, "%s[%d] No free pages left!\n", __FUNCTION__, __LINE__);*/
        *Physical = 0;
        *Logical  = gcvNULL;
        pthread_mutex_unlock(&memPool.mutex);
        /* No free pages left. */
        return;
    }

    /* Try finding enough contiguous free pages. */
    for (i = memPool.freePage; i+pageCount <= memPool.pageCount;)
    {
        /* All pages behind this free page should be free. */
        gctSIZE_T j;
        for (j = 1; j < pageCount; ++j)
        {
            if (memPool.pageUsage[i + j].pageCount != 0)
            {
                /* Bail out if page is allocated. */
                break;
            }
        }

        if (j == pageCount)
        {
            /* We found a spot that has enough free pages. */
            break;
        }

        /* Move to the page after the allocated page. */
        i += j + 1;

        /* Find the next free page. */
        while ((i+pageCount <= memPool.pageCount) && (memPool.pageUsage[i].pageCount != 0))
        {
            ++i;
        }
    }

    if (i+pageCount > memPool.pageCount)
    {
        /*fprintf(stderr, "Not enough contiguous pages. pageCount:%d\n", (int)memPool.pageCount);*/
        *Physical = 0;
        *Logical  = gcvNULL;
        pthread_mutex_unlock(&memPool.mutex);
        /* Not enough contiguous pages. */
        return;
    }

    /* Check if we allocate from the first free page. */
    if (i == memPool.freePage)
    {
        /* Move first free page to beyond the contiguous request. */
        memPool.freePage = i + pageCount;

        /* Find first free page. */
        while ( (memPool.freePage < memPool.pageCount) &&
                (memPool.pageUsage[memPool.freePage].pageCount != 0) )
        {
            ++memPool.freePage;
        }

        if (memPool.freePage >= memPool.pageCount)
        {
            /* No more free pages. */
            memPool.freePage = -1;
        }
    }

    /* Walk all pages. */
    for (j = 0; j < pageCount; ++j)
    {
        /* Store page count in each pageUsage to mark page is allocated. */
        memPool.pageUsage[i+j].pageCount = pageCount;
    }

    gcmkTRACE(gcvLEVEL_INFO, "Allocated %u contiguous pages from 0x%X\n",
        pageCount, i);

    /*fprintf(stderr, "%s[%d] Allocated %u contiguous pages from 0x%X\n", __FUNCTION__, __LINE__, (gctUINT32)pageCount, (gctUINT32)i);*/

    /* Success. */
    *Physical = i * memPool.pageSize + memPool.paddr;
    *Logical  = (gctPOINTER)(i * memPool.pageSize + memPool.addr);

    pthread_mutex_unlock(&memPool.mutex);
}

int drv_mempool_free(gctPOINTER Logical)
{
    gctUINT16 pageCount;
    gctSIZE_T i;
    gctINT32 pageIndex;

    gcmkTRACE(gcvLEVEL_INFO, "Freeing pages @ %x\n", Logical);

    pthread_mutex_lock(&memPool.mutex);

    pageIndex = (gcmPTR2INT32(Logical - memPool.addr)) / memPool.pageSize;

    /* Verify the memory is valid and unlocked. */
    if ( (pageIndex < 0) || (pageIndex >= memPool.pageCount) )
    {
        pthread_mutex_unlock(&memPool.mutex);
        gcmkTRACE(gcvLEVEL_ERROR, "%s: Invalid page index @ %d\n", __FUNCTION__, pageIndex);
        return -1;
    }

    pageCount = memPool.pageUsage[pageIndex].pageCount;

    /* Mark all used pages as free. */
    for (i = 0; i < pageCount; ++i)
    {
        gcmkASSERT(memPool.pageUsage[i + pageIndex].pageCount == pageCount);

        memPool.pageUsage[i + pageIndex].pageCount = 0;
    }

    /* Update first free page. */
    if ( (memPool.freePage < 0) || (pageIndex < memPool.freePage) )
    {
        memPool.freePage = pageIndex;
    }

    pthread_mutex_unlock(&memPool.mutex);

    gcmkTRACE(gcvLEVEL_INFO, "Free'd %u contiguous pages from 0x%X @ 0x%x\n",
        pageCount, pageIndex);

    return 1;
}

/*----------------------------------------------------------------------------*/
/*---------------------------- Shared Memory Pool ----------------------------*/

/* Per process shared memory pool. */

/* Pointer to list of shared memory pools. */
gckSHM_POOL shmPoolList;
pthread_mutex_t shmPoolListMutex;

/*
 * Initialize shm pool list and mutex.
 */
gceSTATUS
drv_shm_init()
{
    shmPoolList = gcvNULL;
    pthread_mutex_init(&shmPoolListMutex, 0);
    return gcvSTATUS_OK;
}

gceSTATUS
drv_shm_destroy()
{
    gckSHM_POOL shmPool, nextPool;

    shmPool = shmPoolList;
    while (shmPool != gcvNULL)
    {
        nextPool = shmPool->nextPool;
        /* Remove this pool from the list. */
        drv_shmpool_destroy(shmPool);

        shmPool = nextPool;
    }
    pthread_mutex_destroy(&shmPoolListMutex);

    return gcvSTATUS_OK;
}

/*
 * Get the shm pool associated with this PID and lock it.
 * Create one, if not present.
 */
gckSHM_POOL
drv_shm_acquire_pool(
        IN gctUINT32 Pid,
        IN gctUINT32 PoolSize,
        IN gctUINT32 CacheFlag
        )
{
    gckSHM_POOL shmPool, tail = gcvNULL;

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = shmPoolList;
    while (shmPool != gcvNULL)
    {
        if ((shmPool->pid == Pid) && (shmPool->cacheFlag == CacheFlag))
        {
            pthread_mutex_unlock(&shmPoolListMutex);
            return shmPool;
        }

        tail = shmPool;
        shmPool = shmPool->nextPool;
    }

    shmPool = drv_shmpool_create(Pid, PoolSize, sharedPoolPageSize, CacheFlag);

    /* Add this pool to tail. */
    if ( shmPool != gcvNULL )
    {
        if (tail != gcvNULL )
        {
            tail->nextPool = shmPool;
        }
        else
        {
            /* Set this pool as head. */
            shmPoolList = shmPool;
        }

        shmPool->nextPool = gcvNULL;
    }
    else
    {
        fprintf(stderr, "%s:%d: Failed to create new shmPool.\n", __FUNCTION__, __LINE__);
    }

    pthread_mutex_unlock(&shmPoolListMutex);
    return shmPool;
}

/*
 * Get the shm pool associated with both the pid and user logical pointer.
 *
 * @note: the caller must acquire the mutex first.
 */
gckSHM_POOL
drv_shm_acquire_pool_by_user_logical(
        IN gctUINT32 Pid,
        IN gctPOINTER Logical)
{
    gckSHM_POOL shmPool, shmPoolPid = gcvNULL;

    shmPool = shmPoolList;

    while (1)
    {
        while (shmPool != gcvNULL)
        {
            if (shmPool->pid == Pid)
            {
                shmPoolPid = shmPool;
                break;
            }

            shmPool = shmPool->nextPool;
        }

        if (shmPool == gcvNULL)
        {
            break;
        }

        while (shmPoolPid != gcvNULL)
        {
            /* Check if this address is in range of this shmPool. */
            if ((shmPoolPid->UserLogical <= Logical) &&
                ((shmPoolPid->UserLogical + shmPoolPid->poolSize) > Logical)
               )
            {
                return shmPoolPid;
            }

            shmPoolPid = shmPoolPid->nextPoolPid;
        }

        shmPool = shmPool->nextPool;
    }

    return gcvNULL;
}

/*
 * Get the shm pool associated with both the user pid and kernel logical pointer.
 *
 * @note: the caller must acquire the mutex first, and release it after.
 */
gckSHM_POOL
drv_shm_acquire_pool_by_kernel_logical(
        IN gctUINT32 Pid,
        IN gctPOINTER Logical)
{
    gckSHM_POOL shmPool, shmPoolPid = gcvNULL;

    shmPool = shmPoolList;

    while (1)
    {
        while (shmPool != gcvNULL)
        {
            if (shmPool->pid == Pid)
            {
                shmPoolPid = shmPool;
                break;
            }

            shmPool = shmPool->nextPool;
        }

        if (shmPool == gcvNULL)
        {
            break;
        }

        while (shmPoolPid != gcvNULL)
        {
            /* Check if this address is in range of this shmPool. */
            if ((shmPoolPid->KernelLogical <= Logical) &&
                ((shmPoolPid->KernelLogical + shmPoolPid->poolSize) > Logical)
               )
            {
                return shmPoolPid;
            }

            shmPoolPid = shmPoolPid->nextPoolPid;
        }

        shmPool = shmPool->nextPool;
    }

    return gcvNULL;
}

/*
 * Remove the shm pools associated with this Pid.
 */
gceSTATUS
drv_shm_remove_pool(
        IN gctUINT32 Pid
        )
{
    gckSHM_POOL shmPool, prev = gcvNULL, next;

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = shmPoolList;

    while (shmPool != gcvNULL)
    {
        /* Remove this pool from the list. */
        if (shmPool->pid == Pid)
        {
            if (prev == gcvNULL)
            {
                shmPoolList = shmPool->nextPool;
            }
            else
            {
                prev->nextPool = shmPool->nextPool;
            }
            next = shmPool->nextPool;

            drv_shmpool_destroy(shmPool);

            shmPool = next;
        }
        else
        {
            prev = shmPool;
            shmPool = shmPool->nextPool;
        }
    }

    pthread_mutex_unlock(&shmPoolListMutex);

    return gcvSTATUS_OK;
}

gceSTATUS
drv_shmpool_mem_offset(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address)
{
    gckSHM_POOL shmPool;

    if (Address == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = drv_shm_acquire_pool_by_kernel_logical(Pid, Logical);

    if (shmPool != gcvNULL)
    {
        *Address = (gctPHYS_ADDR_T)(Logical - shmPool->KernelLogical) + shmPool->Physical;

        pthread_mutex_unlock(&shmPoolListMutex);

        return gcvSTATUS_OK;
    }

    pthread_mutex_unlock(&shmPoolListMutex);

    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
drv_shmpool_mem_offset_by_user_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address)
{
    gckSHM_POOL shmPool;

    if (Address == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = drv_shm_acquire_pool_by_user_logical(Pid, Logical);

    if (shmPool != gcvNULL)
    {
        *Address = (gctPHYS_ADDR_T)(Logical - shmPool->UserLogical) + shmPool->Physical;

        pthread_mutex_unlock(&shmPoolListMutex);

        return gcvSTATUS_OK;
    }

    pthread_mutex_unlock(&shmPoolListMutex);

    return gcvSTATUS_INVALID_ARGUMENT;
}

/* Initialize a shm pool for this Pid. */
gckSHM_POOL drv_shmpool_create(
        IN gctUINT32 Pid,
        IN gctUINT32 PoolSize,
        IN gctUINT32 PageSize,
        IN gctUINT32 CacheFlag)
{
    int rc;
    void *caddr=gcvNULL, *saddr=gcvNULL;
    off64_t paddr;
    gctUINT32 fd;
#if defined(AARCH64)
    size_t pt_contig_len = 0;
    int pt_fildes = NOFD;
    off64_t pt_offset;
#endif

    gckSHM_POOL shm = (gckSHM_POOL) calloc(1, sizeof(struct _gckSHM_POOL));
    if (shm == gcvNULL)
    {
        fprintf(stderr, "vivante: Failed to allocate shared mem structure for user mem");
        return gcvNULL;
    }

    /* Compute number of pages. */
    shm->pageSize    = PageSize;
    shm->pageCount   = gcmALIGN(PoolSize, shm->pageSize) / shm->pageSize;
    shm->poolSize    = shm->pageCount * shm->pageSize;
    shm->pid         = Pid;
    shm->nextPool    = gcvNULL;
    shm->nextPoolPid = gcvNULL;

    gcmkASSERT(shm->pageCount <= 65536);

    /* The first page is free. */
    shm->freePage = 0;
    shm->freePageCount = shm->pageCount;

    /* Initialize the semaphore. */
    if (pthread_mutex_init(&shm->mutex, NULL) != EOK)
    {
        fprintf(stderr, "%s:%d: pthread_mutex_init failed: %s\n",
                __FILE__, __LINE__, strerror(errno));
        free(shm);
        return gcvNULL;
    }
    fd = drv_create_shm_object();
    if (fd == -1) {
        fprintf(stderr, "%s: couldn't create shmem: %s\n", __FUNCTION__, strerror( errno ) );
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }

#if defined(IMX6X) || defined(IMX8X) || defined(IMX)
    /*
     * use SHMCTL_LAZYWRITE to get write-combine memory access
     */
#if defined(AARCH64)
    /* Map this memory inside user and galcore. */
    saddr = mmap64(0, shm->poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, posix_mem_fd, 0);
    if (saddr == MAP_FAILED) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "[%s %d] drv_shmpool_create: couldn't mmap memory of size: %u, Pid: %u [errno: %s]",
                __FUNCTION__, __LINE__, shm->poolSize, Pid, strerror( errno ));
        close(fd);
        pthread_mutex_destroy(&shm->mutex);
        if (shm->poolSize > 512*1024*1024) {
            slogf(_SLOGC_GRAPHICS_GL, _SLOG_CRITICAL, "shm->poolSize (%u) is suspiciously large -- forcing core dump!", shm->poolSize);
            *((int*)1) = 0;
        }
        free(shm);
        return gcvNULL;
    }

    if (posix_mem_offset64(saddr, shm->poolSize, &pt_offset, &pt_contig_len, &pt_fildes) == -1) {
        fprintf(stderr, "%s: posix_mem_offset64 failed: %s\n", __FUNCTION__, strerror( errno ) );
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "[%s %d] drv_shmpool_create: couldn't allocate memory of size %d, Pid: %d [errno %s]",
                __FUNCTION__, __LINE__, shm->poolSize, Pid, strerror( errno ) );
        close(fd);
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }

    /* Overlay the shared memory object over the posix memory. Set as shareable and write through. */
    if (CacheFlag == 0x1) {
        rc = shm_ctl_special(fd, SHMCTL_PHYS, pt_offset, shm->poolSize, ARM_SHMCTL_WB | ARM_SHMCTL_SH);
    } else {
        rc = shm_ctl_special(fd, SHMCTL_PHYS, pt_offset, shm->poolSize, ARM_SHMCTL_WT | ARM_SHMCTL_SH);
    }
    if (rc) {
        fprintf(stderr, "%s: shm_ctl_special failed: %s\n", __FUNCTION__, strerror( errno ) );
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "[%s %d] drv_shmpool_create: couldn't allocate memory of size %d, Pid: %d [errno %s]",
                __FUNCTION__, __LINE__, shm->poolSize, Pid, strerror( errno ) );
        close(fd);
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }
    if( !rc && saddr != MAP_FAILED && Pid == getpid()  ) {
        caddr = saddr;
    } else {
        caddr = mmap64_peer(Pid, 0, shm->poolSize, PROT_READ | PROT_WRITE,  MAP_SHARED, fd, 0);
    }
#else
    if (CacheFlag == 0x1) {
        rc = shm_ctl(fd, SHMCTL_ANON|SHMCTL_PHYS, 0, shm->poolSize);
    } else {
        rc = shm_ctl(fd, SHMCTL_ANON|SHMCTL_PHYS|SHMCTL_LAZYWRITE, 0, shm->poolSize);
    }

    if (rc) {
        fprintf(stderr, "%s: shm_ctl failed: %s\n", __FUNCTION__, strerror( errno ) );
        close(fd);
        pthread_mutex_destroy(&shm->mutex);
        free(shm);

        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "[%s %d] drv_shmpool_create: couldn't allocate memory of size %d, Pid: %d [errno %s]",
                                                __FUNCTION__, __LINE__, shm->poolSize, Pid, strerror( errno ) );

        return gcvNULL;
    }

    /* Map this memory inside user and galcore. */
    saddr = mmap64(0, shm->poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


    if( !rc && saddr != MAP_FAILED && Pid == getpid()  ) {
        caddr = saddr;
    } else {
        caddr = mmap64_peer(Pid, 0, shm->poolSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }
#endif

#else /* fallback: it is for OMAP4/5, LAZYWRITE causes lock-ups. */

    /* Allocation performance is not an issue for omap5 -- the amount of memory is small */
    /* TODO: we do not need/use the fd in this code! */
    rc = 0;

    saddr = mmap64(0, shm->poolSize, PROT_READ | PROT_WRITE | (CacheFlag?0:PROT_NOCACHE),
        MAP_SHARED | MAP_ANON | MAP_NOINIT | MAP_PHYS, NOFD, 0);

    if( saddr == MAP_FAILED ) {
        slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "drv_shmpool_create: mmap64() failed, size %d, Pid: %d [errno %s]",
            shm->poolSize, Pid, strerror( errno ) );
        rc = 1;

    } else if( Pid == getpid() ) {
        /* Same process mapping -- do not mmap_peer() into itself */
        caddr = saddr;

    } else {
        caddr = mmap64_peer(Pid, 0, shm->poolSize, PROT_READ | PROT_WRITE | (CacheFlag?0:PROT_NOCACHE), MAP_SHARED, NOFD, 0);
        if( caddr == MAP_FAILED ) {
            slogf(_SLOGC_GRAPHICS_GL, _SLOG_ERROR, "drv_shmpool_create: mmap64_peer() failed, size %d, Pid: %d [errno %s]",
                shm->poolSize, Pid, strerror( errno ) );
            rc = 1;
        }
    }

#endif

    if (rc || (caddr == MAP_FAILED) || (saddr == MAP_FAILED))
    {
        if ((caddr != MAP_FAILED) && (caddr != saddr))
        {
            munmap_peer(Pid, caddr, shm->poolSize);
        }

        if (saddr != MAP_FAILED)
        {
            munmap(saddr, shm->poolSize);
        }

        fprintf(stderr, "%s: couldn't map memory of size %d, Pid: %d [errno %s]",
            __FUNCTION__, shm->poolSize, Pid, strerror( errno ) );

        close(fd);
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }

    shm->cacheFlag = CacheFlag;

    rc = close(fd);
    if (rc == -1) {
        fprintf(stderr, "%s: close failed: %s\n", __FUNCTION__, strerror( errno ) );
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }

    shm->UserLogical   = caddr;
    shm->KernelLogical = saddr;

    /* fd should be NOFD here, to get physical address. */
    rc = mem_offset64(saddr, NOFD, 1, (off64_t *)&paddr, NULL);
    if (rc == -1) {
        fprintf(stderr, "%s: mem_offset failed (saddr:%p): %s\n", __FUNCTION__, saddr, strerror( errno ) );
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }

    shm->Physical = (gctPHYS_ADDR_T)paddr;

    mlock(shm->KernelLogical, shm->poolSize);

    /* Allocate the page usage array and Initialize all pages to free. */
    shm->pageUsage = (gckPAGE_USAGE)calloc(shm->pageCount, sizeof(struct _gckPAGE_USAGE));
    if (shm->pageUsage == gcvNULL)
    {
        fprintf( stderr, "%s: malloc failed: %s\n", __FUNCTION__, strerror(errno) );
        munmap(shm->KernelLogical, shm->poolSize);
        if (getpid() != Pid)
        {
            munmap_peer(Pid, shm->UserLogical, shm->poolSize);
        }
        pthread_mutex_destroy(&shm->mutex);
        free(shm);
        return gcvNULL;
    }


    return shm;
}

void
drv_shmpool_destroy(
        IN gckSHM_POOL ShmPool)
{
    if (ShmPool)
    {
        if (ShmPool->nextPoolPid != gcvNULL)
        {
            drv_shmpool_destroy(ShmPool->nextPoolPid);
        }

        pthread_mutex_destroy(&ShmPool->mutex);

        /*fprintf(stderr, "Destroying shmPool %p for pid %d. PageCount:%d freePageCount:%d\n", ShmPool, ShmPool->pid, ShmPool->pageCount, ShmPool->freePageCount);*/

        if (ShmPool->pageUsage)
        {
            free(ShmPool->pageUsage);
        }

        if (ShmPool->UserLogical)
        {
            munmap(ShmPool->KernelLogical, ShmPool->poolSize);
            if (getpid() != ShmPool->pid)
            {
                munmap_peer(ShmPool->pid, ShmPool->UserLogical, ShmPool->poolSize);
            }
        }

        free(ShmPool);
    }
    else
    {
        gcmkASSERT(0);
    }
}

/* Allocate pages from mapped shared memory.
   Return Logical user address.
*/
gctPOINTER
drv_shmpool_alloc_contiguous(
        IN gctUINT32 Pid,
        IN gctUINT32 Bytes,
        IN gctUINT32 CacheFlag
        )
{
    gctSIZE_T i = 0, j;
    gctSIZE_T pageCount;
    gctUINT32 poolSize;
    gckSHM_POOL shmPool;

    pageCount = gcmALIGN(Bytes, 4096) / 4096;
    gcmkASSERT(pageCount != 0);

    poolSize = pageCount * 4096;
    /* Align it wrt sharedPoolSize. */
    poolSize = gcmALIGN(poolSize, sharedPoolSize);

    shmPool = drv_shm_acquire_pool(Pid, poolSize, CacheFlag);

    if (shmPool == gcvNULL || Bytes == 0)
    {
        return gcvNULL;
    }

    pthread_mutex_lock(&shmPoolListMutex);

    /* Compute the number of required pages. */
    do
    {
        if (shmPool->cacheFlag != CacheFlag)
        {
            fprintf(stderr, "Assert!!! shm pool obtained is not matching!\n");
        }

        if (pageCount <= shmPool->freePageCount)
        {
            /* Try finding enough contiguous free pages. */
            for (i = shmPool->freePage; i+pageCount <= shmPool->pageCount;)
            {
                /* All pages after this free page should be free. */
                for (j = 1; j < pageCount; ++j)
                {
                    if (shmPool->pageUsage[i + j].pageCount != 0)
                    {
                        /* Bail out if page is allocated. */
                        break;
                    }
                }

                if (j == pageCount)
                {
                    /* We found a spot that has enough free pages. */
                    break;
                }

                /* Move to the page after the allocated page. */
                i += j + 1;

                /* Find the next free page. */
                while ((i+pageCount <= shmPool->pageCount) && (shmPool->pageUsage[i].pageCount != 0))
                {
                    /*i += shmPool->pageUsage[i].pageCount;*/
                    ++i;
                }
            }

            if (i+pageCount <= shmPool->pageCount)
            {
                /* Found free pages. */
                break;
            }
        }

        if (shmPool->nextPoolPid == gcvNULL)
        {
            /* Allocate a new shmPool. */
            shmPool->nextPoolPid = drv_shmpool_create(Pid, poolSize, shmPool->pageSize, CacheFlag);

            if (shmPool->nextPoolPid == gcvNULL)
            {
                pthread_mutex_unlock(&shmPoolListMutex);

                fprintf(stderr, "%s OOM!!!!\n", __FUNCTION__);
                /* Out of memory. */
                return gcvNULL;
            }
        }

        shmPool = shmPool->nextPoolPid;

    } while(shmPool != gcvNULL);

    /* Check if we allocate from the first free page. */
    if (i == shmPool->freePage)
    {
        /* Move first free page to beyond the contiguous request. */
        shmPool->freePage = i + pageCount;

        /* Find first free page. */
        while ( (shmPool->freePage < shmPool->pageCount) &&
                (shmPool->pageUsage[shmPool->freePage].pageCount != 0) )
        {
            /*shmPool->freePage += shmPool->pageUsage[shmPool->freePage].pageCount;*/
            shmPool->freePage++;
        }

        if (shmPool->freePage >= shmPool->pageCount)
        {
            /* No more free pages. */
            shmPool->freePage = -1;
        }
    }
    /*fprintf(stderr, "Allocating from shmpool: %p. freePage %2d, Bytes:%d\n", shmPool, shmPool->freePage, Bytes);*/

    /* Walk all pages. */
    shmPool->pageUsage[i].pageCount = pageCount;
    for (j = 0; j < pageCount; ++j)
    {
        /* Store page count in each pageUsage to mark page is allocated. */
        shmPool->pageUsage[i+j].pageCount = pageCount;

        /* Store the starting page for this block for checking partial frees */
        shmPool->pageUsage[i+j].startPageIndex = i;
    }

    shmPool->freePageCount -= pageCount;

    gcmkTRACE(gcvLEVEL_INFO, "Allocated %u contiguous pages from 0x%X\n",
        pageCount, i);

    pthread_mutex_unlock(&shmPoolListMutex);


    /* Success. */
    return (gctPOINTER)(i * shmPool->pageSize + shmPool->UserLogical);
}

gctPOINTER
drv_shmpool_get_kernel_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical
    )
{
    gckSHM_POOL shmPool;
    gctPOINTER svaddr = gcvNULL;

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = drv_shm_acquire_pool_by_user_logical(Pid, Logical);

    if (shmPool != gcvNULL)
    {
        svaddr = (Logical - shmPool->UserLogical) + shmPool->KernelLogical;
    }

    pthread_mutex_unlock(&shmPoolListMutex);

    return svaddr;
}

gctUINT32
drv_shmpool_free(
        IN gctPOINTER Logical
        )
{
    gctUINT16 pageCount;
    gctSIZE_T i;
    gctINT32 pageIndex;
    gckSHM_POOL shmPool;
    gctUINT32 pid;

    pid = drv_get_user_pid();

    pthread_mutex_lock(&shmPoolListMutex);

    shmPool = drv_shm_acquire_pool_by_user_logical(pid, Logical);
/*
    fprintf(stderr, "ShmPool %p Stats:\n", shmPool);
    fprintf(stderr, "pageCount %d:\n", shmPool->pageCount);
    fprintf(stderr, "pageSize %x:\n", shmPool->pageSize);
*/

    if (shmPool == gcvNULL)
    {
        pthread_mutex_unlock(&shmPoolListMutex);
        gcmkTRACE(gcvLEVEL_ERROR, "%s: Invalid Logical addr: %x.\n", __FUNCTION__, Logical);
        return 0;
    }

    pageIndex = (gcmPTR2INT32(Logical - shmPool->UserLogical)) / shmPool->pageSize;

    gcmkTRACE(gcvLEVEL_INFO, "Freeing pages @ %d\n", pageIndex);

    /* Verify the memory is valid and unlocked. */
    if ( (pageIndex < 0) || (pageIndex >= shmPool->pageCount) )
    {
        pthread_mutex_unlock(&shmPoolListMutex);
        gcmkTRACE(gcvLEVEL_ERROR, "%s: Invalid page index @ %d\n", __FUNCTION__, pageIndex);
        return 0;
    }

    pageCount = shmPool->pageUsage[pageIndex].pageCount;
    gcmkASSERT(shmPool->pageUsage[pageIndex].pageCount == pageCount);

    if (pageCount == 0) {
        gcmkPRINT("[VIV]: %s: CRITICAL: double free of a shmpool block, aborting!\n", __FUNCTION__);
        abort();
    }

    if (pageIndex != shmPool->pageUsage[pageIndex].startPageIndex) {
        gcmkPRINT("[VIV]: %s: CRITICAL: pageIndex doesn't match allocated pageIndex in a shmpool, aborting!\n", __FUNCTION__);
        abort();
    }

    /* Mark all used pages as free. */
    for (i = 0; i < pageCount; ++i)
    {
        shmPool->pageUsage[i + pageIndex].pageCount = 0;
    }

    /* Update first free page. */
    if ( (shmPool->freePage < 0) || (pageIndex < shmPool->freePage) )
    {
        shmPool->freePage = pageIndex;
    }
    /*fprintf(stderr, "Freeing    from shmpool: %p. freePage %2d, Bytes:%d\n", shmPool, shmPool->freePage, pageCount*shmPool->pageSize);*/

    shmPool->freePageCount += pageCount;

    pthread_mutex_unlock(&shmPoolListMutex);

    gcmkTRACE(gcvLEVEL_INFO, "Free'd %u contiguous pages from 0x%X @ 0x%x\n",
        pageCount, pageIndex);

    return 1;
}

int
drv_create_shm_object()
{
    char name[128];
    int fd, retries = MAX_EEXIST_RETRIES;
    static volatile unsigned seed = 0;

    do {
        /* create a random name to prevent namespace squatting attacks */
        /* FIXME:  does not prevent namespace squatting attacks */
        /* also add PID and TID so we don't collide with other processes started in the same second */
        snprintf(name, sizeof(name), "/gpu:%d:%d:%ld",
                 atomic_add_value(&seed, 1),
                 getpid(),
                 (long)pthread_self());
        fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd != -1) {
            shm_unlink(name);
            return fd;
        }
        fprintf(stderr, "Failed to create %s: %s\n", name, strerror(errno));
    } while (errno == EEXIST && retries--);

    return -1;
}

/*----------------------------------------------------------------------------*/

static LIST_HEAD(_gcsPHYSICALList, _gcsPHYSICAL) s_physicalMapList, s_physicalFreeList;
static gcsPHYSICAL s_physicalFreeListNodes[(1 << 20) / gcmSIZEOF(gcsPHYSICAL)];
static pthread_mutex_t s_physicalListLock = PTHREAD_RMUTEX_INITIALIZER;

gceSTATUS
drv_physical_map_init()
{
    gctINT i;

    /* Init the head node. */
    LIST_INIT(&s_physicalMapList);

    /* Init the free list. */
    LIST_INIT(&s_physicalFreeList);

    for (i = 0; i < gcmCOUNTOF(s_physicalFreeListNodes); i++)
    {
        LIST_INSERT_HEAD(&s_physicalFreeList, &s_physicalFreeListNodes[i], node);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
drv_physical_allocate_node(
    OUT gcsPHYSICAL_PTR * Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL_PTR pNode;
    gctBOOL acquired = gcvFALSE;

    if (Node == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the mutex. */
    pthread_mutex_lock(&s_physicalListLock);
    acquired = gcvTRUE;

    /* If the free list is empty. */
    if (LIST_EMPTY(&s_physicalFreeList))
    {
        /* We have to allocate the node structure from the heap. */
        pNode = (gcsPHYSICAL_PTR)malloc(gcmSIZEOF(*pNode));
        if (pNode == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    }
    else
    {
        /* Get a node from the free list. */
        pNode = LIST_FIRST(&s_physicalFreeList);
        LIST_REMOVE(pNode, node);
    }

    /* Reset the node. */
    gckOS_ZeroMemory(pNode, gcmSIZEOF(*pNode));
    pNode->physicalAddress = gcvINVALID_PHYSICAL_ADDRESS;

    /* Save pointer to the node. */
    *Node = pNode;

    /* Release the mutex. */
    pthread_mutex_unlock(&s_physicalListLock);
    acquired = gcvFALSE;

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    if (acquired)
    {
        pthread_mutex_unlock(&s_physicalListLock);
    }

    return status;
}

gceSTATUS
drv_physical_free_node(
    IN gcsPHYSICAL_PTR Node
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (Node == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Acquire the mutex. */
    pthread_mutex_lock(&s_physicalListLock);

    /* Add to the free list. */
    if ((Node >= s_physicalFreeListNodes) &&
        (Node < &s_physicalFreeListNodes[gcmCOUNTOF(s_physicalFreeListNodes)])
       )
    {
        LIST_INSERT_HEAD(&s_physicalFreeList, Node, node);

        Node = gcvNULL;
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&s_physicalListLock);

    /* Free the node if from heap. */
    if (Node != gcvNULL)
    {
        free(Node);
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    return status;
}

void
drv_physical_map_add_node(
    IN gcsPHYSICAL_PTR Node
    )
{
    if (Node == gcvNULL)
    {
        return;
    }

    /* Acquire the mutex. */
    pthread_mutex_lock(&s_physicalListLock);

    /* Add to the global list. */
    LIST_INSERT_HEAD(&s_physicalMapList, Node, node);

    /* Release the mutex. */
    pthread_mutex_unlock(&s_physicalListLock);
}

void
drv_physical_map_delete_node(
    IN gcsPHYSICAL_PTR Node
    )
{
    if (Node == gcvNULL)
    {
        return;
    }

    /* Acquire the mutex. */
    pthread_mutex_lock(&s_physicalListLock);

    /* Detach from the global list. */
    LIST_REMOVE(Node, node);

    /* Release the mutex. */
    pthread_mutex_unlock(&s_physicalListLock);
}

void
drv_physical_map_get_node(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical,
    OUT gcsPHYSICAL_PTR *Node
    )
{
    gcsPHYSICAL_PTR p;
    gctUINT32 logical, userLogical;

    logical = gcmPTR2INT32(Logical);

    /* Acquire the mutex. */
    pthread_mutex_lock(&s_physicalListLock);

    /* Iterate over the linked list. */
    LIST_FOREACH(p, &s_physicalMapList, node)
    {
        userLogical = gcmPTR2INT32(p->userLogical);

        if ((p->pid == Pid) &&
            (logical >= userLogical) &&
            (logical < (userLogical + p->bytes))
           )
        {
            /* Found it! */
            *Node = p;
            break;
        }
    }

    /* Release the mutex. */
    pthread_mutex_unlock(&s_physicalListLock);
}

/*----------------------------------------------------------------------------*/
/*----------------------------- Resource Manager -----------------------------*/

/* Resource Manager Globals. */
struct _gcskRESMGR_GLOBALS
{
    dispatch_t *dpp;
    dispatch_context_t *ctp;
    int id;
    thread_pool_attr_t pool_attr;
    thread_pool_t *tpp;
    pthread_t root;
} resmgr_globals;

win_gpu_2_cm_iface_t *g_qnx_gpu_2_cm_iface = 0;
static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

pthread_key_t thread_specific_key;                      /* To store calling process attributes. */

void thread_specific_key_destruct( void *value )
{
    /*fprintf(stderr, "Destroying thread: myTid:%u, value:%d\n", pthread_self(), (gctUINT32)value);*/
    if (value != gcvNULL)
    {
        free( value );
    }
    pthread_setspecific( thread_specific_key, NULL );
}

int drv_thread_specific_key_assign(
    gctUINT32 Pid,
    gctUINT32 Tid
    )
{
    gcskTHREAD_USER_DATA *data;

    data = (gcskTHREAD_USER_DATA *)pthread_getspecific( thread_specific_key );

    if( data == NULL ) {
        data = (gcskTHREAD_USER_DATA *) malloc( sizeof(gcskTHREAD_USER_DATA) );

        /*fprintf(stderr, "First time. Pid:%u, Tid:%u, myTid:%u\n", Pid, Tid, pthread_self());*/

        if( data == NULL )
        {
            return 0;
        }
    }

    /*fprintf(stderr, "Pid:%u, Tid:%u, myTid:%u\n", Pid, Tid, pthread_self());*/
    data->Pid = Pid;
    data->Tid = Tid;

    if ( pthread_setspecific( thread_specific_key, (void *)data ) != EOK)
    {
        return 0;
    }

    return 1;
}

gctUINT32 drv_get_user_pid( void )
{
    gcskTHREAD_USER_DATA *data;

    data = (gcskTHREAD_USER_DATA *)pthread_getspecific( thread_specific_key );

    if (data == gcvNULL)
    {
        /*fprintf(stderr, "%s[%d] pid = 0\n", __FUNCTION__, __LINE__);*/
        /* Galcore process. */
        return 0;
    }
    /*fprintf(stderr, "%s[%d] pid = %u\n", __FUNCTION__, __LINE__, data->Pid);*/
    return data->Pid;
}

gctUINT32 drv_get_user_tid( void )
{
    gcskTHREAD_USER_DATA *data;

    data = (gcskTHREAD_USER_DATA *)pthread_getspecific( thread_specific_key );

    if (data == gcvNULL)
    {
        return (gctUINT32)-1;
    }

    /*fprintf(stderr, "%s[%d] Tid = %u\n", __FUNCTION__, __LINE__, data->Tid);*/
    return data->Tid;
}

int drv_msg(resmgr_context_t *ctp,
            io_msg_t *msg,
            RESMGR_OCB_T *ocb)
{
    gcsDRIVER_ARGS *drvArgs = (gcsDRIVER_ARGS *)msg;
    int rc;
    gceSTATUS status;
    gcsQUEUE_PTR queue, userQueue;
    gctPOINTER pointer;
#if gcdENABLE_VG
    gcsVGCONTEXT_PTR context, userContext;
    gctPOINTER signal;
#endif
    gctUINT32 pid;

/*#define UNLOCK_RESMGR*/
#ifdef UNLOCK_RESMGR
    iofunc_attr_unlock(&attr);
#endif


    if ((drvArgs->iomsg.i.type != _IO_MSG)
    || (drvArgs->iomsg.i.mgrid != _IOMGR_VIVANTE)
    || (drvArgs->iomsg.i.subtype != IOCTL_GCHAL_INTERFACE
        && drvArgs->iomsg.i.subtype != IOCTL_GCHAL_KERNEL_INTERFACE
        && drvArgs->iomsg.i.subtype != IOCTL_GCHAL_TERMINATE))
    {
        /* Unknown command. Fail the I/O. */
#ifdef UNLOCK_RESMGR
        iofunc_attr_lock(&attr);
#endif
        rc = ENOSYS;
        if (ctp->info.scoid != -1)
            return _RESMGR_STATUS(ctp, rc);
        return _RESMGR_NOREPLY;
    }

    if (drvArgs->iomsg.i.subtype == IOCTL_GCHAL_TERMINATE)
    {
        /* terminate the resource manager */
        pthread_kill(resmgr_globals.root, SIGTERM);
#ifdef UNLOCK_RESMGR
        iofunc_attr_lock(&attr);
#endif
        return _RESMGR_NOREPLY;
    }

    pid  = (gctUINT32)ctp->info.pid;

    if (drv_thread_specific_key_assign(pid, (gctUINT32)ctp->info.tid) == 0)
    {
        drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
        goto OnError;
    }


    /* Store receive ID with signal event so that we can later respond via pulse. */
    switch (drvArgs->iface.command)
    {
    case gcvHAL_SIGNAL:
        status = drv_signal_mgr_update_iface(pid, ctp->rcvid, &drvArgs->iface);
        if (gcmIS_ERROR(status))
        {
            drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
            goto OnError;
        }
        break;

    case gcvHAL_EVENT_COMMIT:
        userQueue = gcmUINT64_TO_PTR(drvArgs->iface.u.Event.queue);

        for (; userQueue != gcvNULL; )
        {
            status = gckOS_MapUserPointer(
                        gcvNULL, userQueue, gcmSIZEOF(*userQueue), &pointer);

            if (gcmIS_ERROR(status))
            {
                drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                goto OnError;
            }

            queue = pointer;

            status = drv_signal_mgr_update_iface(pid, ctp->rcvid, &queue->iface);
            if (gcmIS_ERROR(status))
            {
                drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                goto OnError;
            }

            queue = gcmUINT64_TO_PTR(queue->next);

            gckOS_UnmapUserPointer(
                gcvNULL, userQueue, gcmSIZEOF(*userQueue), pointer);

            userQueue = queue;
        }
        break;

    case gcvHAL_COMMIT:
#if gcdENABLE_VG
        if (drvArgs->iface.hardwareType == gcvHARDWARE_VG)
        {
            userContext = gcmUINT64_TO_PTR(drvArgs->iface.u.VGCommit.context);

            status = gckOS_MapUserPointer(
                        gcvNULL, userContext, gcmSIZEOF(*userContext), &pointer);

            if (gcmIS_ERROR(status))
            {
                drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                goto OnError;
            }

            context = pointer;

            status = drv_signal_mgr_add(
                        pid,
                        context->coid,
                        ctp->rcvid,
                        gcmPTR_TO_UINT64(context->signal),
                        &signal);
            if (gcmIS_ERROR(status))
            {
                drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                goto OnError;
            }

            context->rcvid      = ctp->rcvid;
            context->userSignal = signal;

            gckOS_UnmapUserPointer(
                gcvNULL, userContext, gcmSIZEOF(*userContext), pointer);
        }
        else
#endif
        {
            gcsHAL_SUBCOMMIT *subCommit = &drvArgs->iface.u.Commit.subCommit;
            gctPOINTER userPtr = gcvNULL;
            gctUINT64 next;

            do
            {
                status = gcvSTATUS_OK;

                if (userPtr)
                {
                    status = gckOS_MapUserPointer(
                        gcvNULL,
                        userPtr,
                        gcmSIZEOF(gcsHAL_SUBCOMMIT),
                        (gctPOINTER *)&subCommit
                        );

                    if (gcmIS_ERROR(status))
                    {
                        drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                        goto OnError;
                    }
                }

                userQueue = gcmUINT64_TO_PTR(subCommit->queue);

                while (userQueue)
                {
                    status = gckOS_MapUserPointer(
                        gcvNULL,
                        userQueue,
                        gcmSIZEOF(gcsQUEUE_PTR),
                        &pointer
                        );

                    if (gcmIS_ERROR(status))
                    {
                        drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                        break;
                    }

                    queue = pointer;
                    status = drv_signal_mgr_update_iface(pid, ctp->rcvid, &queue->iface);

                    if (gcmIS_ERROR(status))
                    {
                        drvArgs->iface.status = gcvSTATUS_GENERIC_IO;
                        break;
                    }

                    queue = gcmUINT64_TO_PTR(queue->next);

                    gckOS_UnmapUserPointer(
                        gcvNULL,
                        userQueue,
                        gcmSIZEOF(gcsQUEUE_PTR),
                        pointer);

                    userQueue = queue;
                }

                next = subCommit->next;

                if (userPtr)
                {
                    gcmkVERIFY_OK(gckOS_UnmapUserPointer(
                        gcvNULL,
                        userPtr,
                        gcmSIZEOF(gcsHAL_SUBCOMMIT),
                        subCommit
                        ));
                }

                if (gcmIS_ERROR(status))
                {
                    /* Error in signal update loop. */
                    goto OnError;
                }

                /* Advance to next sub-commit from user. */
                userPtr = gcmUINT64_TO_PTR(next);
            }
            while (userPtr);
        }
        break;

    default:
        break;
    }

    status = gckDEVICE_Dispatch(galDevice->device, &drvArgs->iface);

#if gcdDEBUG
    if (gcmIS_ERROR(status))
    {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DRIVER,
                  "[galcore] gckDEVICE_Dispatch returned %d.\n",
              status);
    }
    else if (gcmIS_ERROR(drvArgs->iface.status))
    {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_DRIVER,
                  "[galcore] IOCTL %d returned %d.\n",
              drvArgs->iface.command,
              drvArgs->iface.status);
    }
#endif


OnError:
    /* Reply data back to the user. */
    MsgReply(ctp->rcvid, EOK, (gctPOINTER) &drvArgs->iface, sizeof(gcsHAL_INTERFACE));

#ifdef UNLOCK_RESMGR
    iofunc_attr_lock(&attr);
#endif

    gcmkTRACE(gcvLEVEL_INFO, "[galcore] Replied message with command %d, status %d\n",
        drvArgs->iface.command,
        drvArgs->iface.status);

    return (_RESMGR_NOREPLY);
}

/*----------------------------------------------------------------------------*/
/*-------------------------- DRIVER INITIALIZATIONS --------------------------*/
#if defined(IMX6X) || defined(IMX)
uintptr_t m_pl310Base = MAP_DEVICE_FAILED;
#endif

static int drv_init(void)
{
    gcsDEVICE_CONSTRUCT_ARGS args;

    int i;

    memset(&args, 0, sizeof(args));

    for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++) {
        args.irqs[i] = -1;
    }

    args.recovery        = recovery;
    args.stuckDump       = stuckDump;
    args.powerManagement = powerManagement;
    args.mmu             = mmu;
    args.gpuProfiler     = gpuProfiler;

    args.irqs[gcvCORE_MAJOR] = irqLine;
    args.registerBases[gcvCORE_MAJOR] = registerMemBase;
    args.registerSizes[gcvCORE_MAJOR] = registerMemSize;
    args.chipIDs[gcvCORE_MAJOR] = gcvCORE_MAJOR;

#if defined(gcdDUAL_CORE)
    if (irqLine3D1 != -1) {
      args.irqs[gcvCORE_3D1] = irqLine3D1;
      args.registerBases[gcvCORE_3D1] = registerMemBase3D1;
      args.registerSizes[gcvCORE_3D1] = registerMemSize3D1;
      args.chipIDs[gcvCORE_3D1] = gcvCORE_3D1;
    }
#endif

    args.irqs[gcvCORE_2D] = irqLine2D;
    args.registerBases[gcvCORE_2D] = registerMemBase2D;
    args.registerSizes[gcvCORE_2D] = registerMemSize2D;
    args.chipIDs[gcvCORE_2D] = gcvCORE_2D;

    args.irqs[gcvCORE_VG] = irqLine2D;
    args.registerBases[gcvCORE_VG] = registerMemBaseVG;
    args.registerSizes[gcvCORE_VG] = registerMemSizeVG;
    args.chipIDs[gcvCORE_VG] = gcvCORE_VG;

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_DRIVER,
                  "[galcore] Entering drv_init\n");

#if defined(IMX6X) || defined(IMX)
    /*
     * This is for L2-cache-sync
     * since we use write-combine memory access
     */
#if defined(IMX)
    if (hasPL310 == 1)
#endif
    {
      if (m_pl310Base == MAP_DEVICE_FAILED) {
          m_pl310Base = mmap_device_io(0x1000, 0x00A02000);
          if (m_pl310Base == MAP_DEVICE_FAILED) {
              gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                            "[galcore] [%s] Can't mmap device io for PL310 L2 Cache controller (errno: %s).\n", __FUNCTION__, strerror(errno));

              return -1;
          }
      }
    }
#endif

    gcmkPRINT("Galcore version %s\n", gcvVERSION_STRING);

    if (physSize == 0)
    {
        physBase = memPool.paddr;
        physSize = memPool.poolSize;
    }

    /* Create the GAL device. */
    if (gcmIS_ERROR(gckGALDEVICE_Construct(
                    irqLine, registerMemBase, registerMemSize,
                    irqLine2D, registerMemBase2D, registerMemSize2D,
                    irqLineVG, registerMemBaseVG, registerMemSizeVG,
                    contiguousBase,
                    contiguousSize,
                    bankSize,
                    fastClear,
                    compression,
                    baseAddress,
                    physBase,
                    physSize,
                    powerManagement,
                    gpuProfiler,
                    &args,
                    &galDevice)))
    {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                      "[galcore] Can't create the gal device.\n");

        return -1;
    }

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_DRIVER,
                  "[galcore] Device constructed.\n");

    /* Start the GAL device. */
    if (gcmIS_ERROR(gckGALDEVICE_Start(galDevice)))
    {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                      "[galcore] Can't start the gal device.\n");

        /* Roll back. */
        gckGALDEVICE_Stop(galDevice);
        gckGALDEVICE_Destroy(galDevice);

        return -1;
    }

    if ((galDevice->kernels[gcvCORE_MAJOR] != gcvNULL) &&
        (galDevice->kernels[gcvCORE_MAJOR]->hardware->mmuVersion != 0))
    {
        /* Reset the base address */
        baseAddress = 0;
        galDevice->baseAddress = 0;
    }

    if ((galDevice->kernels[gcvCORE_2D] != gcvNULL)
        && (galDevice->kernels[gcvCORE_2D]->hardware->mmuVersion != 0))
    {
        /* Reset the base address */
        baseAddress = 0;
        galDevice->baseAddress = 0;
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                  "[galcore] irqLine->%ld, contiguousSize->%lu, memBase->0x%lX\n",
          irqLine,
          contiguousSize,
          registerMemBase);

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_DRIVER,
                  "[galcore] driver registered successfully.\n");

    return 0;
}

static void drv_exit(void)
{
    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_DRIVER,
                  "[galcore] Entering drv_exit\n");

    gckGALDEVICE_Stop(galDevice);
    gckGALDEVICE_Destroy(galDevice);
}

/* Invoked by OS, when a new connection is created. */
int drv_open_connection(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
    struct pid_hash* node;
    gctINT i;

    if (msg->connect.subtype == _IO_CONNECT_OPEN)
    {
        /* A process gets attached. */
        for (i = 0; i < gcdMAX_GPU_COUNT; i++)
        {
            if (galDevice->kernels[i] != gcvNULL)
            {
                /* Assign pid and tid, before calling gckKERNEL_AttachProcess. */
                if (drv_thread_specific_key_assign(ctp->info.pid, ctp->info.tid))
                {
                    if (gcmIS_ERROR(gckKERNEL_AttachProcess(galDevice->kernels[i], gcvTRUE)))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        /* error handling */
        if (i != gcdMAX_GPU_COUNT)
        {
            for (; i >= 0; i--)
            {
                if (galDevice->kernels[i] != gcvNULL)
                {
                    if (drv_thread_specific_key_assign(ctp->info.pid, ctp->info.tid))
                    {
                        gckKERNEL_AttachProcess(galDevice->kernels[i], gcvFALSE);
                    }
                }
            }
            return EINVAL;
        }
    }

    /* Update reference count for this pid. */
    node = pid_hash_head;
    while(node != gcvNULL)
    {
        if(node->pid == ctp->info.pid)
        {
            break;
        }
        node = node->next;
    }

    if (node == gcvNULL)
    {
        /* New connection. */
        node = (struct pid_hash*) malloc(sizeof(struct pid_hash));
        if (node == gcvNULL) return ENOMEM;

        node->pid = ctp->info.pid;
        node->refCount = 0;

        /* Append node at head. */
        node->next = pid_hash_head;
        pid_hash_head = node;
    }

    node->refCount++;

    return iofunc_open_default(ctp, msg, handle, extra);
}

/* Invoked by OS, when a connection is closed or dies. */
int
drv_close_connection(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
    struct pid_hash *node, *prev_node;
    gctINT i;
    gckKERNEL kernel;
    gcsSignalHandle_PTR signal;

    /* Update reference count for this pid. */
    prev_node = pid_hash_head;

    if ( pid_hash_head != gcvNULL )
    {
        if ( prev_node->pid == ctp->info.pid )
        {
            /* Hit on first node. */
            node = prev_node;
        }
        else
        {
            node = prev_node->next;
            while(node != gcvNULL)
            {
                if(node->pid == ctp->info.pid)
                {
                    break;
                }
                prev_node = node;
                node = node->next;
            }
        }

        if ( node != gcvNULL )
        {
            node->refCount--;

            /* Last close. Close connection. */
            if ( node->refCount == 0)
            {
                if (node == prev_node)
                {
                    /* Head node. */
                    pid_hash_head = node->next;
                }
                else
                {
                    /* Delete from middle. */
                    prev_node->next = node->next;
                }

                free(node);

                /* A process gets detached. */
                for (i = 0; i < gcdMAX_GPU_COUNT; i++)
                {
                    kernel = galDevice->kernels[i];

                    if (kernel != gcvNULL)
                    {
                        if (drv_thread_specific_key_assign(
                                ctp->info.pid, ctp->info.tid)
                            )
                        {
                            gckKERNEL_AttachProcessEx(
                                kernel, gcvFALSE, ctp->info.pid);

#if gcdENABLE_VG
                            if (kernel->vg == gcvNULL)
#endif
                            {
                                gckCOMMAND_Stall(kernel->command, gcvFALSE);
                            }
                        }
                    }
                }

                /* Cleanup all the pending user signals. */
                drv_signal_mgr_lock();

                LIST_FOREACH(signal, &signalList, node)
                {
                    if (signal->pid == ctp->info.pid)
                    {
                        signal->alive = gcvFALSE;
                    }
                }

                drv_signal_mgr_unlock();

                /* Free shared memory and its mapping. */
                drv_shm_remove_pool(ctp->info.pid);
            }
        }
    }

    return iofunc_close_ocb_default(ctp, reserved, ocb);
}

static int drv_load_values_from_config_file()
{
    int rc;
    char val[128];

    /* Load values from configuration file */
    rc = __khrGetDeviceConfigValue(1, "gpu-contiguousMB", val, sizeof(val));
    if (rc == EOK)
    {
        contiguousSize = atoi(val) << 20;
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-internalPoolMB", val, sizeof(val));
    if (rc == EOK)
    {
        internalPoolSize = atoi(val) << 20;
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-sharedPoolMB", val, sizeof(val));
    if (rc == EOK)
    {
        sharedPoolSize = atoi(val) << 20;
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-physBase", val, sizeof(val));
    if (rc == EOK)
    {
        physBase = atol(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-physSize", val, sizeof(val));
    if (rc == EOK)
    {
        physSize = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-powerManagement", val, sizeof(val));
    if (rc == EOK)
    {
        powerManagement = (atoi(val) != 0);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-mmu", val, sizeof(val));
    if (rc == EOK)
    {
        mmu = (atoi(val) != 0);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-gpuProfiler", val, sizeof(val));
    if (rc == EOK)
    {
        gpuProfiler = (atoi(val) != 0);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-recovery", val, sizeof(val));
    if (rc == EOK)
    {
        recovery = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-stuckDump", val, sizeof(val));
    if (rc == EOK)
    {
        stuckDump = atoi(val);
    }

    /* When enable gpu profiler, we need to turn off the gpu powerMangement. */
    if (gpuProfiler)
    {
        powerManagement = 0;
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-slogUsageInterval", val, sizeof(val));
    if (rc == EOK) {
        slogUsageInterval = atoi(val);
    }

#if defined(IMX)
    rc = __khrGetDeviceConfigValue(1, "device-base-address", val, sizeof(val));
    if (rc == EOK)
    {
      baseAddress = atol(val);
    } else {
      fprintf(stderr, "Unable to read mandatory parameter device-base-address from graphics.conf!\n");
      return -1;
    }

    rc = __khrGetDeviceConfigValue(1, "hasPL310", val, sizeof(val));
    if (rc == EOK)
    {
      hasPL310 = atoi(val);
    }

    //GPU 3D 0
    rc = __khrGetDeviceConfigValue(1, "gpu-irq-3D", val, sizeof(val));
    if (rc == EOK)
    {
        irqLine = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-base-3D", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemBase = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-size-3D", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemSize = atoi(val);
    }

#ifdef gcdDUAL_CORE
    //GPU 3D 1
    rc = __khrGetDeviceConfigValue(1, "gpu-irq-3D1", val, sizeof(val));
    if (rc == EOK)
    {
        irqLine3D1 = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-base-3D1", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemBase3D1 = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-size-3D1", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemSize3D1 = atoi(val);
    }
#endif

    //2D core

    rc = __khrGetDeviceConfigValue(1, "gpu-irq-2D", val, sizeof(val));
    if (rc == EOK)
    {
        irqLine2D = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-base-2D", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemBase2D = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-size-2D", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemSize2D = atoi(val);
    }

    //VG core

    rc = __khrGetDeviceConfigValue(1, "gpu-irq-VG", val, sizeof(val));
    if (rc == EOK)
    {
        irqLineVG = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-base-VG", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemBaseVG = atoi(val);
    }

    rc = __khrGetDeviceConfigValue(1, "gpu-reg-size-VG", val, sizeof(val));
    if (rc == EOK)
    {
        registerMemSizeVG = atoi(val);
    }
#endif

    if(compression == -1)
    {
        compression = gcvCOMPRESSION_OPTION_DEFAULT;
        /* Depth compression generates 16bit bursts to reduce data traffic.
           Due to a LPDDR4 limitation, 16bit bursts causes high latency in LPDDR4. This latency outweighs benefit of the compression.
           This option can disable the depth compression for boards with LPDDR4. */
        rc = __khrGetDeviceConfigValue(1, "gpu-depth-compression", val, sizeof(val));
        if (rc == EOK)
        {
            if (atoi(val) == 0) {
                compression &= ~gcvCOMPRESSION_OPTION_DEPTH;
            }
        }
    }

    return 0;
}

int gpu_init()
{
    /* Declare variables we'll be using. */
    resmgr_attr_t resmgr_attr;
    sigset_t  sigset;
    int rc;

    if (drv_load_values_from_config_file() != 0) {
        goto fail_001;
    }

    gckOS_DumpParam();

    drv_signal_mgr_init();

    if (drv_physical_map_init() != gcvSTATUS_OK)
    {
        gcmkASSERT(0);
        goto fail_001;
    }

    if (drv_mempool_init() != gcvSTATUS_OK)
    {
        fprintf(stderr, "drv_mempool_init failed.");
        goto fail_001;
    }

    if (drv_shm_init() != gcvSTATUS_OK)
    {
        fprintf(stderr, "drv_shm_init failed.");
        goto fail_002;
    }

    if (drv_init() != 0)
    {
        fprintf(stderr, "drv_init failed.");
        goto fail_003;
    }

    /* Initialize thread local storage. */
    if((pthread_key_create( &thread_specific_key, &thread_specific_key_destruct)) != EOK)
    {
        fprintf(stderr, "Unable to create thread_specific_key.\n");
        goto fail_004;
    }

    /* initialize dispatch interface */
    if((resmgr_globals.dpp = dispatch_create()) == NULL)
    {
        fprintf(stderr, "Unable to allocate dispatch handle.\n");
        goto fail_004;
    }

    /* Initialize per pid reference count list. */
    pid_hash_head = gcvNULL;

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
            _RESMGR_IO_NFUNCS, &io_funcs);

    /* Register io handling functions. */
    io_funcs.msg       = drv_msg;
    io_funcs.close_ocb = drv_close_connection;
    connect_funcs.open = drv_open_connection;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    /* attach our device name */
    resmgr_globals.id = resmgr_attach(
            resmgr_globals.dpp,/* dispatch handle */
            &resmgr_attr,           /* resource manager attrs */
            GAL_DEV,                /* device name */
            _FTYPE_ANY,             /* open type */
            _RESMGR_FLAG_SELF,      /* flags */
            &connect_funcs,         /* connect routines */
            &io_funcs,              /* I/O routines */
            &attr);                 /* handle */
    if (resmgr_globals.id == -1) {
        fprintf(stderr, "Unable to attach name.\n");
        goto fail_005;
    }

    fprintf(stderr, "Attached resmgr to " GAL_DEV " with id:%d.\n", resmgr_globals.id);

    /* Prevent signals from affecting resmgr threads. */
    sigfillset(&sigset);
    sigdelset(&sigset, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    /* initialize thread pool attributes */
    memset(&resmgr_globals.pool_attr, 0, sizeof(resmgr_globals.pool_attr));
    resmgr_globals.pool_attr.handle = resmgr_globals.dpp;
    resmgr_globals.pool_attr.context_alloc = resmgr_context_alloc;
    resmgr_globals.pool_attr.block_func   = resmgr_block;
    resmgr_globals.pool_attr.unblock_func = resmgr_unblock;
    resmgr_globals.pool_attr.handler_func = resmgr_handler;
    resmgr_globals.pool_attr.context_free = resmgr_context_free;
    resmgr_globals.pool_attr.lo_water = 1;
    resmgr_globals.pool_attr.hi_water = 1;
    resmgr_globals.pool_attr.increment = 1;
    resmgr_globals.pool_attr.maximum = 1;
#if (defined(_NTO_VERSION) && (_NTO_VERSION >= 650))
    resmgr_globals.pool_attr.tid_name = "galcore-msg";
#endif

    /* allocate a thread pool handle */
    resmgr_globals.tpp = thread_pool_create(&resmgr_globals.pool_attr, POOL_FLAG_EXIT_SELF);
    if (resmgr_globals.tpp == NULL)
    {
        goto fail_006;
    }

    rc = pthread_create(NULL, resmgr_globals.pool_attr.attr, (void * (*)(void *))thread_pool_start, resmgr_globals.tpp);
    if (rc != 0)
    {
        goto fail_007;
    }

    return EXIT_SUCCESS;

fail_007:
    thread_pool_destroy(resmgr_globals.tpp);
fail_006:
    resmgr_detach(resmgr_globals.dpp, resmgr_globals.id, 0);
fail_005:
    dispatch_destroy(resmgr_globals.dpp);
fail_004:
    drv_exit();
fail_003:
    drv_shm_destroy();
fail_002:
    drv_mempool_destroy();
fail_001:
    return EXIT_FAILURE;
}

int gpu_fini()
{
    fprintf(stderr, "Exiting galcore.\n");
    thread_pool_destroy(resmgr_globals.tpp);
    resmgr_detach(resmgr_globals.dpp, resmgr_globals.id, 0);
    dispatch_destroy(resmgr_globals.dpp);
    drv_exit();
    drv_shm_destroy();
    drv_mempool_destroy();
    return EXIT_SUCCESS;
}

#ifndef QNX_USE_OLD_FRAMEWORK

int GPU_Startup(win_gpu_2_cm_iface_t *iface)
{
    g_qnx_gpu_2_cm_iface = iface;
    return gpu_init();
}

int GPU_Shutdown()
{
    g_qnx_gpu_2_cm_iface = NULL;
    return gpu_fini();
}

void win_gpu_module_getfuncs(win_cm_2_gpu_iface_t *iface)
{
    iface->init = GPU_Startup;
    iface->fini = GPU_Shutdown;
}

#else /* QNX_USE_OLD_FRAMEWORK */

int drv_resmgr_loop()
{
    sigset_t  sigset;
    siginfo_t info;

    resmgr_globals.root = pthread_self();

    /* Background ourselves */
    procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NODEVNULL |
                                 PROCMGR_DAEMON_NOCHDIR |
                                 PROCMGR_DAEMON_NOCLOSE);

    /*
     * This thread ignores all signals except SIGTERM. On receipt of
     * a SIGTERM, we shut everything down and exit.
     */
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTERM);

    while (1)
    {
        if (SignalWaitinfo(&sigset, &info) == -1)
            continue;
        if (info.si_signo == SIGTERM)
        {
            fprintf(stderr, "%s: received term signal.\n", __FUNCTION__);
            break;
        }
    }

    return EXIT_SUCCESS;
}

int drv_start_cmd()
{
    int rc;

    setlinebuf(stderr);
    fprintf(stderr, "Starting up...\n");

    pthread_setname_np(pthread_self(), "vivante-monitor");

    if ((rc = gpu_init()) != EXIT_SUCCESS)
    {
        fprintf(stderr, "Initialization failed!, Exiting.");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Running galcore...\n");
    rc = drv_resmgr_loop();
    fprintf(stderr, "Shutting down galcore...\n");

    gpu_fini();

    return EXIT_SUCCESS;
}

int drv_stop_cmd()
{
    gcsDRIVER_ARGS args;
    int fd, rc;

    /* Open the gpu device. */
    fd = open(GAL_DEV, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Could not connect to " GAL_DEV);
        return EXIT_FAILURE;
    }

    /* Send the term message. */
    args.iomsg.i.type    = _IO_MSG;
    args.iomsg.i.subtype = IOCTL_GCHAL_TERMINATE;
    args.iomsg.i.mgrid   = _IOMGR_VIVANTE;
    args.iomsg.i.combine_len = sizeof(io_msg_t);

    do {
        rc = MsgSend_r(fd, &args, args.iomsg.i.combine_len, NULL, 0);
    } while ((rc * -1) == EINTR);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    enum { start, stop } cmd = start;
    int i;
    int rc = EXIT_FAILURE;

    /* Process command lines -start, -stop, -c (file), -d [file]. */
    for (i = 1; i < argc; i++)
    {
        if (stricmp(argv[i], "-start") == 0)
        {
            cmd = start;
        }
        else if (strcmp(argv[i], "-stop") == 0)
        {
            cmd = stop;
        }
        else if (strncmp(argv[i], "-poolsize=", strlen("-poolsize=")) == 0)
        {
            /* The syntax of the poolsize option is -poolsize=(number).
             * All we need is to convert the number that starts after the '='.*/
            contiguousSize = atoi(argv[i] + strlen("-poolsize="));
            if (contiguousSize <= 0)
            {
                fprintf(stderr, "%s: poolsize needs to be a positive number\n", strerror(errno));
                return rc;
            }
        }
        else
        {
            fprintf(stderr, "%s: bad command line\n", argv[0]);
            return rc;
        }
    }

    switch (cmd)
    {
        case start:
            /* Elevate thread priority to do IO. */
#ifndef _NTO_TCTL_IO_PRIV
#define _NTO_TCTL_IO_PRIV _NTO_TCTL_IO
#endif
            ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
            rc = drv_start_cmd();
            break;

        case stop:
            rc = drv_stop_cmd();
            break;
    }

    return rc;
}

#endif /* QNX_USE_OLD_FRAMEWORK */
