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


#ifndef _GBM_VIV_INTERNAL_H_
#define _GBM_VIV_INTERNAL_H_

#include <sys/mman.h>
#include "gbmint.h"

#include "common_drm.h"

struct gbm_viv_surface;
struct gbm_viv_bo;

struct gbm_viv_device {
    struct gbm_device base;

    void *driver;
};

struct gbm_viv_bo {
    struct gbm_bo base;

    uint32_t size;
    void *map;
};

struct gbm_viv_surface {
   struct gbm_surface base;

   uint32_t buffer_count;
   struct gbm_viv_bo bos[4];

   void *viv_private;
};

static inline struct gbm_viv_device *
gbm_viv_device(struct gbm_device *gbm)
{
   return (struct gbm_viv_device *) gbm;
}

static inline struct gbm_viv_bo *
gbm_viv_bo(struct gbm_bo *bo)
{
   return (struct gbm_viv_bo *) bo;
}

static inline struct gbm_viv_surface *
gbm_viv_surface(struct gbm_surface *surface)
{
   return (struct gbm_viv_surface *) surface;
}

#endif
