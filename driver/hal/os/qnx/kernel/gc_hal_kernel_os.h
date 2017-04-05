/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_kernel_os_h_
#define __gc_hal_kernel_os_h_

typedef struct
{
    io_msg_t iomsg;
    gcsHAL_INTERFACE iface;
} gcsDRIVER_ARGS;

struct _gckPAGE_USAGE
{
    gctUINT16 startPageIndex;
    gctUINT16 pageCount;
};

struct _gckSHM_POOL
{
    gctUINT32 pid;                      /* Pid of the owner of this pool. */
    gctUINT32 freePage;                 /* Index of first free page. */
    gctUINT32 freePageCount;            /* Count of free pages. */
    gctUINT32 pageCount;                /* Number of pages. */
    gctUINT32 pageSize;                 /* Size of each page. */
    gctUINT32 poolSize;                 /* Size of this pool. */
    pthread_mutex_t mutex;              /* Mutex. */
    gctUINT32 UserLogical;              /* Logical base address in user process. */
    gctUINT32 KernelLogical;            /* logical base address in galcore process. */
    gctUINT32 Physical;                 /* Physical base address. */
    gctUINT32 cacheFlag;                /* Flag with which the shmPool was created. */
    struct _gckPAGE_USAGE* pageUsage;   /* List of pageUsage arrays. */
    struct _gckSHM_POOL* nextPool;      /* Pointer to next pool. */
    struct _gckSHM_POOL* nextPoolPid;   /* Pointer to next pool for the same Pid. */
};

typedef struct _gckSHM_POOL* gckSHM_POOL;
typedef struct _gckPAGE_USAGE* gckPAGE_USAGE;

typedef enum _gcePHYSICAL_TYPE
{
    gcvPHYSICAL_TYPE_UNKNOWN        = 0,
    gcvPHYSICAL_TYPE_MEMPOOL        = 1,
    gcvPHYSICAL_TYPE_SHMPOOL        = 2,
    gcvPHYSICAL_TYPE_PAGED_MEMORY   = 3,
    gcvPHYSICAL_TYPE_WRAPPED_MEMORY = 4,
}
gcePHYSICAL_TYPE;

typedef struct _gcsPHYSICAL * gcsPHYSICAL_PTR;
typedef struct _gcsPHYSICAL
{
    gcePHYSICAL_TYPE            type;

    gctINT32                    fd;
    gctBOOL                     contiguous;
    gctSIZE_T                   bytes;

    gctUINT32                   pid;

    gctUINT32                   physicalAddress;
    gctPOINTER                  userLogical;
    gctPOINTER                  kernelLogical;
    gctUINT32                   pageCount;
    gctUINT32                   extraPage;

    gctINT32                    kernelMapCount;

    LIST_ENTRY(_gcsPHYSICAL)    node;
}
gcsPHYSICAL;

/*
 * gcsSignalHandle definitions.
 */
typedef struct _gcsSignalHandle * gcsSignalHandle_PTR;
typedef struct _gcsSignalHandle
{
    gctUINT32                       pid;
    gctINT32                        coid;
    gctINT32                        rcvid;
    gctBOOL                         alive;
    gctUINT64                       signal;
    LIST_ENTRY(_gcsSignalHandle)    node;
}
gcsSignalHandle;

void
drv_signal_mgr_init();

void
drv_signal_mgr_fini();

void
drv_signal_mgr_lock();

void
drv_signal_mgr_unlock();

gceSTATUS
drv_signal_mgr_add(
    IN gctUINT32 Pid,
    IN gctINT32 Coid,
    IN gctINT32 Rcvid,
    IN gctUINT64 Signal,
    OUT gctPOINTER *Handle
    );

void
drv_signal_mgr_del(
    IN gcsSignalHandle_PTR Signal
    );

gceSTATUS
drv_mempool_init();

void
drv_mempool_destroy();

void
drv_mempool_alloc_contiguous(
    IN gctUINT32 Bytes,
    OUT gctUINT32 * Physical,
    OUT gctPOINTER * Logical
    );

int
drv_mempool_free(
    IN gctPOINTER Logical
    );

gctUINT32
drv_mempool_get_baseAddress();

gctUINT32
drv_mempool_get_basePAddress();

gctUINT32
drv_mempool_get_page_size();

gctINT
drv_mempool_get_fileDescriptor();

gceSTATUS
drv_mempool_mem_offset(
    IN gctPOINTER Logical,
    OUT gctUINT32 * Address);

gceSTATUS
drv_mempool_get_kernel_logical(
    IN gctUINT32 Address,
    OUT gctPOINTER *Logical);

/* Shared memory pool functions. */
gckSHM_POOL
drv_shmpool_create(
    IN gctUINT32 Pid,
    IN gctUINT32 PoolSize,
    IN gctUINT32 PageSize,
    IN gctUINT32 CacheFlag);

void
drv_shmpool_destroy(
    IN gckSHM_POOL ShmPool
    );

gckSHM_POOL
drv_shm_acquire_pool(
    IN gctUINT32 Pid,
    IN gctUINT32 PoolSize,
    IN gctUINT32 CacheFlag
    );

gckSHM_POOL
drv_shm_acquire_pool_by_user_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical
    );

gckSHM_POOL
drv_shm_acquire_pool_by_kernel_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical
    );

gceSTATUS
drv_shm_remove_pool(
    IN gctUINT32 Pid
    );

gceSTATUS
drv_shmpool_mem_offset(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical,
    OUT gctUINT32 * Address);

gceSTATUS
drv_shmpool_mem_offset_by_user_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical,
    OUT gctUINT32 * Address);

gctPOINTER
drv_shmpool_alloc_contiguous(
    IN gctUINT32 Pid,
    IN gctUINT32 Bytes,
    IN gctUINT32 CacheFlag
    );

gctPOINTER
drv_shmpool_get_kernel_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical
    );

gctUINT32
drv_shmpool_free(
    IN gctPOINTER Logical
    );

int
drv_create_shm_object();

gceSTATUS
drv_physical_map_init();

gceSTATUS
drv_physical_allocate_node(
    OUT gcsPHYSICAL_PTR * Node
    );

gceSTATUS
drv_physical_free_node(
    IN gcsPHYSICAL_PTR Node
    );

void
drv_physical_map_add_node(
    IN gcsPHYSICAL_PTR Node
    );

void
drv_physical_map_delete_node(
    IN gcsPHYSICAL_PTR Node
    );

gctPOINTER
drv_physical_map_get_kernel_logical(
    IN gctUINT32 Pid,
    IN gctPOINTER Logical
    );

gctUINT32
drv_get_user_pid(void);

gctUINT32
drv_get_user_tid(void);

int drv_thread_specific_key_assign(
    gctUINT32 Pid,
    gctUINT32 Tid
    );

int
mem_offset64_peer(pid_t pid, const uintptr_t addr, size_t len,
                off64_t *offset, size_t *contig_len);

int
munmap_peer(pid_t pid, void *addr, size_t len);

void *
mmap64_peer(pid_t pid, void *addr, size_t len, int prot, int flags, int fd, off64_t off);

#endif /* __gc_hal_kernel_os_h_ */
