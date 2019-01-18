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


#ifndef __GC_HAL_KERNEL_RESOURCE_MANAGER_BUF_H
#define __GC_HAL_KERNEL_RESOURCE_MANAGER_BUF_H

struct thread_state;

struct buf_msg {
    char *buf;

    /* where we are in the buffer */
    size_t pos;

    /* how many bytes have been allocated, in case we need to realloc */
    size_t allocated;

    /* back-pointer to the thread that created us so we can access the device */
    struct thread_state *thread;
};

#define BUF_MSG_GET_BUF(buf_msg)    (buf_msg->buf)
#define BUF_MSG_GET_POS(buf)        (buf->pos)
#define BUF_MSG_GET_THREAD(buf)     (buf->thread)
#define BUF_MSG_GET_ALLOCATED(buf)  (buf->allocated)

struct write_msg {
    char *buf;
    size_t nbytes;
};

/**
 * creates the buf_msg with initially @allocated var set
 */
struct buf_msg
*buf_msg_create(size_t allocated, struct thread_state *thread);

/**
 *
 */
void
buf_msg_clean(struct buf_msg **buf_msg);

/**
 * @buf_msg_destroy: destroys/frees the entire buf_msg
 */
void
buf_msg_destroy(struct buf_msg **buf_msg);

/**
 * @msg_buf_add_msg: This adds fmt to the underlying buffer in buf_msg, takes
 * care of formating and allocating data
 */
void
msg_buf_add_msg(struct buf_msg *buf_msg, const char *fmt, ...);

#endif
