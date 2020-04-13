/*
 *  Copyright (C) 2014 Freescale Semiconductor, Inc.
 *  Copyright 2018 NXP.c
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

/*
 *  drm_hwc_helper.c
 *  This helper file implements the required HWComposer helper function for Android HWComposer based on drm gralloc.
 *  History :
 *  Date(y.m.d)        Author            Version        Description
 *  2018-06-01         Liu Xuegang         1.0            Created
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
#include <gralloc_helper.h>
#include <gralloc_priv.h>
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
    gceTILING tiling = gcvINVALIDTILED;
    static const int sMagic = 0x3141592;

    const struct private_handle_t* handle = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t)
                || handle->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    surface = (gcoSURF) (handle->surface);
    if (surface != NULL)
        gcmVERIFY_OK(gcoSURF_GetTiling(surface, &tiling));

    if(tiling > gcvSUPERTILED || tiling < gcvLINEAR)
    {
        ALOGV("tiling %d not support", tiling);
        return -1;
    }

    if (tile != NULL) {
        *tile = (enum g2d_tiling)tiling;
    }

    return 0;
}

enum g2d_format hwc_alterFormat(buffer_handle_t hnd, enum g2d_format format)
{
    gcoSURF surface;
    enum g2d_format halFormat;
    static const int sMagic = 0x3141592;
    gceSURF_TYPE type = gcvSURF_TYPE_UNKNOWN;
    const struct private_handle_t* handle = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t)
            || handle->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return (g2d_format)-1;
    }

    surface = (gcoSURF) (handle->surface);
    if (surface != NULL)
        gcmVERIFY_OK(gcoSURF_GetFormat(surface, &type, NULL));

    halFormat = format;
    if ((type & 0xFF) == gcvSURF_RENDER_TARGET)
    {
        switch (format)
        {
        case G2D_RGBA8888:
            /*VIV: Check PE_A8B8G8R8 feature. */
            halFormat = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_PE_A8B8G8R8) ? G2D_RGBA8888 : G2D_BGRA8888;
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

int hwc_getAlignedSize(buffer_handle_t hnd, int *width, int *height)
{
    gcoSURF surface;
    gctUINT alignedWidth=0;
    gctUINT alignedHeight=0;
    static const int sMagic = 0x3141592;

    const struct private_handle_t* handle = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            handle->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    surface = (gcoSURF) (handle->surface);
    if (surface != NULL) {
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(
            surface, &alignedWidth, &alignedHeight, NULL));
    }

    if (alignedWidth == 0 || alignedHeight ==0)
        return -1;

    if(width) *width = (int)alignedWidth;
    if(height) *height = (int)alignedHeight;
    return 0;
}

int hwc_getFlipOffset(buffer_handle_t hnd, int *offset)
{
    gctSIZE_T flipOffset=0;

    if(offset) *offset = (int)flipOffset;
    return 0;
}

int hwc_lockSurface(buffer_handle_t hnd)
{
    gcoSURF surface;
    static const int sMagic = 0x3141592;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;
    gctUINT32 address[3];
    gctUINT32 baseAddress = 0;
    int ret;

    struct private_handle_t* handle = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            handle->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    ret = gralloc_buffer_create_surface(hnd, &surface);

    if (ret) {
        g2d_printf("create surface from handle fail (at %p)", hnd);
        return -1;
    }

    handle->surface = (uint64_t) surface;

    /* Query current hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);

    gcoSURF_Lock(surface, address, gcvNULL);
    gcoOS_GetBaseAddress(gcvNULL, &baseAddress);
    address[0] += baseAddress;

    if (address[0] != (gctUINT32)handle->phys) {
               handle->phys = address[0];
    }

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

int hwc_unlockSurface(buffer_handle_t hnd)
{
    gcoSURF surface;
    static const int sMagic = 0x3141592;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    struct private_handle_t* handle = (struct private_handle_t*)hnd;

    if (!hnd || hnd->version != sizeof(native_handle_t) ||
            handle->magic != sMagic)
    {
        g2d_printf("invalid gralloc handle (at %p)", hnd);
        return -1;
    }

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);

    surface = (gcoSURF) (handle->surface);

    if (surface != NULL) {
        gcoSURF_Unlock(surface, gcvNULL);
        gcoSURF_Destroy(surface);
        handle->surface = (uint64_t) NULL;
    }

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

int hwc_align_tile(int *width, int *height, int format, int usage)
{
    gceSURF_TYPE type;
    gceSURF_FORMAT dstFormat = gcvSURF_UNKNOWN;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    /* Query current hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    if (usage & GRALLOC_USAGE_TS_VIV) {
        type = gcvSURF_RENDER_TARGET;
    }
    else if (usage & GRALLOC_USAGE_TILED_VIV) {
        type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
    }
    else {
        type = gcvSURF_BITMAP;
    }

    switch (format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            dstFormat = gcvSURF_A8B8G8R8;
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            dstFormat = gcvSURF_X8B8G8R8;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            dstFormat = gcvSURF_R5G6B5;
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            dstFormat = gcvSURF_A8R8G8B8;
            break;
        default:
            ALOGE("unsupported format:0x%x", format);
            dstFormat = gcvSURF_A8B8G8R8;
            break;
    }

    gcoHAL_AlignToTile((gctUINT32 *)width, (gctUINT32 *)height, type, dstFormat);
    gcoHAL_SetHardwareType(gcvNULL, currentType);
    return 0;
}

