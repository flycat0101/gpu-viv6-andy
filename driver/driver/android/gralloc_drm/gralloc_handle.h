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


#ifndef GRALLOC_HANDLE_H_
#define GRALLOC_HANDLE_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <cutils/native_handle.h>
#include <cutils/log.h>

#include <gralloc_priv.h>
typedef struct private_handle_t gralloc_handle_t;


/*
 * Following functions are to hide platform specific gralloc handle
 * definition.
 *
 * Vivante drivers will only call these functions, instead of access
 * gralloc handle struct directly.
 */

/* handle creation. */
static inline buffer_handle_t gralloc_handle_create(int width, int height,
                                    int format, int usage)
{
    gralloc_handle_t *hnd;
    const int numFds = GRALLOC_PRIVATE_HANDLE_NUM_FDS;
    const int numInts = GRALLOC_PRIVATE_HANDLE_NUM_INTS;

    hnd = (gralloc_handle_t *)native_handle_create(numFds, numInts);
    if (!hnd)
        return NULL;

    memset(hnd->nativeHandle.data, 0, (numFds + numInts) * sizeof(int));
    hnd->fd = -1;
    hnd->magic = GRALLOC_PRIVATE_HANDLE_MAGIC;

    hnd->width = width;
    hnd->height = height;
    hnd->format = format;
    hnd->stride = 0;
    hnd->usage = usage;
    hnd->pid = 0;
    hnd->data = 0;

    return (buffer_handle_t)hnd;
}

/* handle destroy. */
static inline void gralloc_handle_free(buffer_handle_t handle)
{
    gralloc_handle_t *hnd = (gralloc_handle_t *)handle;
    hnd->magic = 0;
#ifdef __GNUC__
    asm volatile("":::"memory");
#endif
    native_handle_delete(&hnd->nativeHandle);
}

/* validate, returns error code. */
static inline int gralloc_handle_validate_taged(buffer_handle_t handle,
                        const char *func, int line)
{
    gralloc_handle_t *hnd = (gralloc_handle_t *)handle;

    if (hnd && (hnd->nativeHandle.version != sizeof(hnd->nativeHandle) ||
                   hnd->nativeHandle.numInts != GRALLOC_PRIVATE_HANDLE_NUM_INTS ||
                   hnd->nativeHandle.numFds != GRALLOC_PRIVATE_HANDLE_NUM_FDS ||
                   hnd->magic != GRALLOC_PRIVATE_HANDLE_MAGIC)) {
        ALOGE("%s(%d): invalid handle: version=%d, numInts=%d, numFds=%d, magic=%x",
            func, line,
            hnd->nativeHandle.version, hnd->nativeHandle.numInts,
            hnd->nativeHandle.numFds, hnd->magic);
        return -EINVAL;
    }
    return 0;
}

#define gralloc_handle_validate(handle) \
    gralloc_handle_validate_taged(handle, __FUNCTION__, __LINE__)

/* get prime fd. */
static inline int gralloc_handle_fd(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->fd;
}

/* set prime fd. */
static inline void gralloc_handle_set_fd(buffer_handle_t handle, int fd)
{
    ((gralloc_handle_t *)handle)->fd = fd;
}

/* get width, width is immutable. */
static inline int gralloc_handle_width(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->width;
}

/* get height, height is immutable. */
static inline int gralloc_handle_height(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->height;
}

/* get format, format is immutable. */
static inline int gralloc_handle_format(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->format;
}

/* get usage, usage is immutable. */
static inline int gralloc_handle_usage(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->usage;
}

/* get stride in pixels. */
static inline int gralloc_handle_stride(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->stride;
}

/* set stride in pixels. */
static inline void gralloc_handle_set_stride(buffer_handle_t handle, int stride)
{
    ((gralloc_handle_t *)handle)->stride = stride;
}

/* get data owner, ie the pid. */
static inline int gralloc_handle_data_owner(buffer_handle_t handle)
{
    return ((gralloc_handle_t *)handle)->pid;
}

/* get data, ie the bo. */
static inline void * gralloc_handle_data(buffer_handle_t handle)
{
    return (void *)(uintptr_t)((gralloc_handle_t *)handle)->data;
}

/* set data along with owner. */
static inline void gralloc_handle_set_data(buffer_handle_t handle,
                        void *data, int data_owner)
{
    gralloc_handle_t * hnd = (gralloc_handle_t *)handle;
    hnd->data = (uintptr_t)data;
    hnd->pid = data_owner;
}

#endif
