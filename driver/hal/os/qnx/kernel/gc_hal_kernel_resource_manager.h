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


#ifndef __GC_HAL_KERNEL_RESOURCE_MANAGER_H
#define __GC_HAL_KERNEL_RESOURCE_MANAGER_H

#define MSG_SIZE        (4 * 1024)

#ifdef __QNX_RESOURCE_MANAGER_DEBUG
#define dprintf_rsmgr(fmt, args...) do {    \
    fprintf(stdout, fmt, ##args);       \
} while (0)
#else
#define dprintf_rsmgr(fmt, args...) do { } while (0)
#endif

/*
 * adding them all under the same prefix "/dev/gc" spares us of adding a dirent
 * entry
 */
#define R_INFO          "/dev/gc/info"
#define R_CLIENTS       "/dev/gc/clients"
#define R_MEMINFO       "/dev/gc/meminfo"
#define R_DATABASE      "/dev/gc/database"
#define R_VERSION       "/dev/gc/version"
#define R_VIDMEM        "/dev/gc/vidmem"
#define R_DUMP_TRIGGER      "/dev/gc/dump_trigger"

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))

enum resource_id {
    RESOURCE_NONE       = 0,
    RESOURCE_INFO       = 1,
    RESOURCE_CLIENTS    = 2,
    RESOURCE_MEMINFO    = 3,
    RESOURCE_DATABASE   = 4,
    RESOURCE_VERSION    = 5,
    RESOURCE_VIDMEM     = 6,
    RESOURCE_DUMP_TRIGGER   = 7
};


struct buf_msg;

struct thread_state {
    pthread_t tid;
    gckGALDEVICE device;

    struct buf_msg *buf_msg;
    volatile unsigned *refcnt;
};

#define THREAD_GET_BUF_MSG(thread)  (thread.buf_msg)
#define THREAD_GET_GALDEVICE(thread)    (thread.device)
#define THREAD_GET_REFCNT(thread)   (thread.refcnt)

typedef int (*read_cb)(void *data);
typedef int (*write_cb)(void *data);

struct rsmgr {
    int rsmgr_id;
    enum resource_id resource_id;
    const char *resource_name;

    read_cb read;
    write_cb write;
};

int
resource_manager_init(gckGALDEVICE device);

int
resource_manager_exit(void);

#endif
