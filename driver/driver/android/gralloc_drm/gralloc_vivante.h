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


/* Vivante drm based gralloc. */
#ifndef __GRALLOC_VIVANTE_H_
#define __GRALLOC_VIVANTE_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Perform operations. */
#define GRALLOC_VIVANTE_PERFORM_GET_DRM_FD  0x80000002

/*
 * Vivante gralloc driver.
 * NOTICE: Do not use below functions out side fo gralloc module.
 */
struct gralloc_vivante_t;

/* Create/destroy vivante gralloc driver. */
int gralloc_vivante_create(gralloc_module_t const *module,
            struct gralloc_vivante_t **pDrv);
void gralloc_vivante_destroy(struct gralloc_vivante_t *drv);

int gralloc_vivante_alloc(struct gralloc_vivante_t *drv, int w, int h,
            int format, int usage, buffer_handle_t* pHandle, int* pStride);
int gralloc_vivante_free(struct gralloc_vivante_t *drv, buffer_handle_t handle);

int gralloc_vivante_register_buffer(struct gralloc_vivante_t *drv,
            buffer_handle_t handle);
int gralloc_vivante_unregister_buffer(struct gralloc_vivante_t *drv,
            buffer_handle_t handle);
int gralloc_vivante_lock(struct gralloc_vivante_t *drv, buffer_handle_t handle,
            int usage, int l, int t, int w, int h, void** vaddr);
int gralloc_vivante_unlock(struct gralloc_vivante_t *drv,
            buffer_handle_t handle);
int gralloc_vivante_lock_ycbcr(struct gralloc_vivante_t *drv,
            buffer_handle_t handle, int usage, int l, int t, int w, int h,
            struct android_ycbcr *ycbcr);
int gralloc_vivante_perform(struct gralloc_vivante_t *drv,
            int operation, va_list args);

#ifdef __cplusplus
}
#endif
#endif
