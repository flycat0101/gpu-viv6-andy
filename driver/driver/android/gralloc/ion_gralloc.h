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


#ifndef __ion_gralloc_h_
#define __ion_gralloc_h_

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

/*
 * ION reference allocator provided by Vivante Corporation
 */
int ion_gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t * pHandle, int * pStride);

int ion_gralloc_free(alloc_device_t* dev,
        buffer_handle_t handle);

int ion_gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

int ion_gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

int ion_gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int left, int top, int width, int height,
        void** vaddr);

int ion_gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle);

#endif

