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


/*
 * Vivante Gralloc Helper header file.
 * Exported for gralloc module.
 *
 * Please DO NOT EDIT this file.
 */

#ifndef __gc_gralloc_helper_h_
#define __gc_gralloc_helper_h_

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

struct private_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Wrap a buffer to Vivante private handle.
 * If a buffer is to be accessed by Vivante HAL(and driver such as EGL, HWC..),
 * but it is allocated by non-vivante allocator. So 'wrap' operation is needed
 * to make it work.
 * This function should be called when buffer allocation(graloc_alloc).
 *
 * NOTICE that Vivante hardware needs:
 *  1. 64 byte alignment in start address
 *  2. 16 pixel alignment in width
 *  3. 4 pixel alignment in height
 * Please make sure the passed in buffer (memory refered by 'vaddr' and 'phys')
 * is correctly aligned.
 *
 * 'format' means Android pixel format. 'stride' is buffer stride in pixels.
 * Physical address('phys') can be ~0U which means invalid address. Virtual
 * address can not be NULL.
 *
 * returns 0 on success or -errno on error.
 */
int
gc_gralloc_wrap(
    struct private_handle_t* hnd,
    int width,
    int height,
    int format,
    int stride,
    unsigned long phys,
    void* vaddr
    );

/*
 * Register buffer wrap to Vivante private handle in client process.
 * This function should be called in buffer registery(gralloc_registerBuffer)
 *
 * Physical address('phys') can be ~0U which means invalid address. Virtual
 * address can not be NULL.
 *
 * returns 0 on success or -errno on error.
 */
int
gc_gralloc_register_wrap(
    struct private_handle_t* hnd,
    unsigned long phys,
    void* vaddr
    );

/*
 * Un-wrap. Cancel the wrap either by wrap or reigster_wrap.
 * Should be called when gralloc_free and gralloc_unregisterBuffer.
 */
int
gc_gralloc_unwrap(
    struct private_handle_t * hnd
    );
/*
 * Notify Vivante HAL that the buffer pixels are changed.
 * Should be called when gralloc_unlock in case buffer content is updated.
 */
int
gc_gralloc_notify_change(
    struct private_handle_t * hnd
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_gralloc_helper_h_ */

