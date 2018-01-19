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


/* Gralloc bo definition, exposed interface for EGL, etc. */
#ifndef _GRALLOC_VIVANTE_BO_H_
#define _GRALLOC_VIVANTE_BO_H_

#include <gralloc_handle.h>

#include <drm.h>
#include <vivante_drm.h>
#include <vivante_bo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GRALLOC_VIVANTE_BO_MAGIC        ('g'<<24|'v'<<16|'b'<<8|'o')

struct drm_vivante_bo;

struct gralloc_vivante_bo_t {
    uint32_t magic;

    int fb_handle;

    int lock_count;
    int locked_for;

    unsigned int refcount;
    struct drm_vivante_bo *bo;
};

static inline struct gralloc_vivante_bo_t *
gralloc_vivante_bo_from_handle(buffer_handle_t handle)
{
    int err;
    struct gralloc_vivante_bo_t *bo;

    err = gralloc_handle_validate(handle);
    if (err)
        return NULL;

    bo = (struct gralloc_vivante_bo_t *)gralloc_handle_data(handle);
    if (!bo || bo->magic != GRALLOC_VIVANTE_BO_MAGIC)
        return NULL;

    return bo;
}

#ifdef __cplusplus
}
#endif
#endif
