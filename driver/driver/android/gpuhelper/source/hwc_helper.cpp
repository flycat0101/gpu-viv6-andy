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


/*
 *  Copyright (C) 2014 Freescale Semiconductor, Inc.
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

/*
 *  hwc_helper.c
 *  This helper file implements the required HWComposer helper function for Android HWComposer.
 *  History :
 *  Date(y.m.d)        Author            Version        Description
 *  2016-07-18         Liu Xiaowen         0.1            Created
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_driver.h>
#include <gc_hal_raster.h>
#include <gc_hal_engine_vg.h>

#include <gc_gralloc_priv.h>
#include "g2dExt.h"
#include "gpuhelper.h"

#include <cutils/log.h>

#if defined(LOGE)
#define g2d_printf LOGE
#elif defined(ALOGE)
#define g2d_printf ALOGE
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef unsigned int EGLBoolean;
typedef void *EGLClientBuffer;

typedef EGLClientBuffer (EGLAPIENTRYP PFNEGLGETRENDERBUFFERVIVPROC) (EGLClientBuffer Handle);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLPOSTBUFFERVIVPROC) (EGLClientBuffer Buffer);

PFNEGLGETRENDERBUFFERVIVPROC  hwc_eglGetRenderBufferVIV;
PFNEGLPOSTBUFFERVIVPROC hwc_eglPostBufferVIV;

void* hwc_getRenderBuffer(void *BufferHandle)
{
    void* result = gcvNULL;

    if(hwc_eglGetRenderBufferVIV == gcvNULL)
    {
        hwc_eglGetRenderBufferVIV = (PFNEGLGETRENDERBUFFERVIVPROC)
            eglGetProcAddress("eglGetRenderBufferVIV");
        if(hwc_eglGetRenderBufferVIV == gcvNULL)
        {
            /* handle error */
            g2d_printf("%s: fail with get _eglGetRenderBufferVIV\n", __FUNCTION__ );
            return gcvNULL;
        }
    } /* eglGetRenderBufferVIV() success */

    /* call _eglGetRenderBufferVIV() */
    result = hwc_eglGetRenderBufferVIV(BufferHandle);

    return result;
}

unsigned int hwc_postBuffer(void* postBuffer)
{
    if(hwc_eglPostBufferVIV == gcvNULL)
    {
        hwc_eglPostBufferVIV = (PFNEGLPOSTBUFFERVIVPROC)
            eglGetProcAddress("eglPostBufferVIV");
        if (hwc_eglPostBufferVIV == gcvNULL)
        {
            /* handle error */
            g2d_printf("%s: fail with get _eglPostBufferVIV\n", __FUNCTION__ );
            return gcvFALSE;
        }
    } /* _eglPostBufferVIV() success */

    /* call _eglPostBufferVIV() */
    hwc_eglPostBufferVIV(postBuffer);

    return gcvTRUE;
}

int hwc_getTiling(buffer_handle_t hnd, enum g2d_tiling* tile)
{
    gcoSURF surface;
    gceTILING tiling;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;

    const struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t)
                || priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    handle = gc_native_handle_get(hnd);

    surface = (gcoSURF) gcmINT2PTR(handle->surface);

    gcmVERIFY_OK(gcoSURF_GetTiling(surface, &tiling));
    if(tiling > gcvSUPERTILED || tiling < gcvLINEAR)
    {
        g2d_printf("tiling %d not support", tiling);
        return -1;
    }

    if (tile != NULL) {
        *tile = (enum g2d_tiling)tiling;
    }

    return 0;
}

enum g2d_format hwc_alterFormat(buffer_handle_t hnd, enum g2d_format format)
{
    enum g2d_format halFormat;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;

    const struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t)
            || priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return (g2d_format)-1;
    }

    handle = gc_native_handle_get(hnd);

    halFormat = format;
    if ((handle->type & 0xFF) == gcvSURF_RENDER_TARGET)
    {
        switch (format)
        {
        case G2D_RGBA8888:
            halFormat = G2D_BGRA8888;
            break;
        case G2D_RGBX8888:
            halFormat = G2D_BGRX8888;
            break;
        case G2D_BGRA8888:
        case G2D_RGB565:
            break;
        case G2D_NV12:
        case G2D_NV21:
        case G2D_NV16:
        case G2D_NV61:
        case G2D_I420:
        case G2D_YV12:
        case G2D_UYVY:
        case G2D_YUYV:
            halFormat = G2D_YUYV;
            break;
        default:
            halFormat = G2D_BGRA8888;
            break;
        }
    }

    return halFormat;
}

//#include <gc_gralloc_priv.h>
//this is private API and not exported in head file
//fsl hwcomposer need get alignment information for gralloc buffer
int hwc_getAlignedSize(buffer_handle_t hnd, int *width, int *height)
{
    gcoSURF surface;
    gctUINT alignedWidth=0;
    gctUINT alignedHeight=0;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;

    const struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    handle = gc_native_handle_get(hnd);

    surface = (gcoSURF) gcmINT2PTR(handle->surface);

    gcmVERIFY_OK(
       gcoSURF_GetAlignedSize(surface, &alignedWidth, &alignedHeight, gcvNULL));

    if(width) *width = (int) alignedWidth;
    if(height) *height = (int) alignedHeight;

    return 0;
}

int hwc_getFlipOffset(buffer_handle_t hnd, int *offset)
{
    gcoSURF surface;
    gctSIZE_T flipOffset=0;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;

#if gcdANDROID_UNALIGNED_LINEAR_COMPOSITION_ADJUST
    const struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    handle = gc_native_handle_get(hnd);

    if(handle->allocUsage & (GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE))
    {
        surface = (gcoSURF) handle->surface;

        gcmVERIFY_OK(
            gcoSURF_GetFlipBitmapOffset(surface, &flipOffset));
    }
#endif

    if(offset) *offset = (int) flipOffset;

    return 0;
}

int hwc_lockSurface(buffer_handle_t hnd)
{
    gcoSURF surface;
    gctSIZE_T flipOffset=0;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;
    gctUINT32 address[3];
    gctUINT32 baseAddress = 0;

    struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    handle = gc_native_handle_get(hnd);

    surface = (gcoSURF) handle->surface;
    /* Query current hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);

    gcoSURF_Lock(surface, address, gcvNULL);
    gcoOS_GetBaseAddress(gcvNULL, &baseAddress);
    address[0] += baseAddress;

    if (address[0] != (gctUINT32)priv->phys) {
    priv->phys = address[0];
    }

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

int hwc_unlockSurface(buffer_handle_t hnd)
{
    gcoSURF surface;
    gctSIZE_T flipOffset=0;
    gc_native_handle_t * handle;
    static const int sMagic = 0x3141592;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    const struct private_handle_t* priv = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            priv->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    handle = gc_native_handle_get(hnd);

    surface = (gcoSURF) handle->surface;
    /* Query current hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);

    gcoSURF_Unlock(surface, gcvNULL);

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

