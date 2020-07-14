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


#ifndef __wl_viv_buffer_
#define __wl_viv_buffer_

#include <wayland-egl.h>
#include <gc_hal.h>

/*
 * Obsolete: Will be updated in later release.
 */

/* Server side wl buffer implemtation. */
struct wl_viv_buffer
{
    struct wl_resource *resource;
    gcoSURF  surface;
    gctINT32 width;
    gctINT32 height;

    gctINT32 format;
    gctUINT alignedWidth;
    gctUINT alignedHeight;
    gctUINT32 physical[3];
    gctUINT32 gpuBaseAddr;
    gceTILING tiling;
    gctINT32 fd;
};

/* Legacy naming. */
typedef struct wl_viv_buffer gcsWL_VIV_BUFFER;

#endif

