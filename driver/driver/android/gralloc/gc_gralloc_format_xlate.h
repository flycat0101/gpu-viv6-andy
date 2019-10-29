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


/* Please add your formats in formatXlate table below. */

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <gc_hal_base.h>
#include <gc_hal_enum.h>

#ifdef FSL_YUV_EXT
#  include <graphics_ext.h>
#endif


static const int formatXlate[] =
{
    /* ANDROID HAL format      ---  VIVANTE HAL format. */
    HAL_PIXEL_FORMAT_RGBA_8888,     gcvSURF_A8B8G8R8,
    HAL_PIXEL_FORMAT_RGBX_8888,     gcvSURF_X8B8G8R8,
    HAL_PIXEL_FORMAT_RGB_888,       gcvSURF_UNKNOWN, /* Not support 24bpp. */
    HAL_PIXEL_FORMAT_RGB_565,       gcvSURF_R5G6B5,
    HAL_PIXEL_FORMAT_BGRA_8888,     gcvSURF_A8R8G8B8,
#if ANDROID_SDK_VERSION < 19
    HAL_PIXEL_FORMAT_RGBA_5551,     gcvSURF_R5G5B5A1,
    HAL_PIXEL_FORMAT_RGBA_4444,     gcvSURF_R4G4B4A4,
#endif

#if ANDROID_SDK_VERSION >= 18
    HAL_PIXEL_FORMAT_YCbCr_420_888, gcvSURF_NV12,
#endif

#if (ANDROID_SDK_VERSION >= 5) && (ANDROID_SDK_VERSION <= 8)
    /* Eclair and Froyo. */
    HAL_PIXEL_FORMAT_YCbCr_422_SP,  gcvSURF_NV16,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,  gcvSURF_NV21,
    HAL_PIXEL_FORMAT_YCbCr_422_P,   gcvSURF_UNKNOWN, /* Not support. */
    HAL_PIXEL_FORMAT_YCbCr_420_P,   gcvSURF_I420,
    HAL_PIXEL_FORMAT_YCbCr_422_I,   gcvSURF_YUY2,
    HAL_PIXEL_FORMAT_YCbCr_420_I,   gcvSURF_UNKNOWN, /* Not support. */
    HAL_PIXEL_FORMAT_CbYCrY_422_I,  gcvSURF_UYVY,
    HAL_PIXEL_FORMAT_CbYCrY_420_I,  gcvSURF_UNKNOWN,
    HAL_PIXEL_FORMAT_YCbCr_420_SP_TILED, gcvSURF_UNKNOWN, /* Not support. */
    HAL_PIXEL_FORMAT_YCbCr_420_SP,  gcvSURF_NV12,
    HAL_PIXEL_FORMAT_YCrCb_420_SP_TILED, gcvSURF_UNKNOWN, /* Not support. */
    HAL_PIXEL_FORMAT_YCrCb_422_SP,  gcvSURF_NV61,

#elif (ANDROID_SDK_VERSION >= 9) && (ANDROID_SDK_VERSION <= 15)
    /* Gingerbread, honeycomb and ICS. */
    HAL_PIXEL_FORMAT_YV12,          gcvSURF_YV12,
    /* Legacy formats (deprecated), used by ImageFormat.java */
    HAL_PIXEL_FORMAT_YCbCr_422_SP,  gcvSURF_NV16,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,  gcvSURF_NV21,
    HAL_PIXEL_FORMAT_YCbCr_422_I,   gcvSURF_YUY2,

    HAL_PIXEL_FORMAT_YCbCr_420_P,   gcvSURF_I420,
    HAL_PIXEL_FORMAT_YCbCr_420_I,   gcvSURF_I420,
    HAL_PIXEL_FORMAT_CbYCrY_422_I,  gcvSURF_UYVY,
    HAL_PIXEL_FORMAT_YCbCr_420_SP,  gcvSURF_NV12,

#elif (ANDROID_SDK_VERSION >= 16) && (ANDROID_SDK_VERSION <= 25)
    /* JellyBean and later */
    HAL_PIXEL_FORMAT_YV12,          gcvSURF_YV12,
    /* Legacy formats (deprecated), used by ImageFormat.java */
    HAL_PIXEL_FORMAT_YCbCr_422_SP,  gcvSURF_NV16,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,  gcvSURF_NV21,
    HAL_PIXEL_FORMAT_YCbCr_422_I,   gcvSURF_YUY2,
#if ANDROID_SDK_VERSION <= 23
    HAL_PIXEL_FORMAT_YCbCr_422_P,   gcvSURF_UNKNOWN, /* Not support. */
    HAL_PIXEL_FORMAT_YCbCr_420_SP,  gcvSURF_NV12,
#endif

    HAL_PIXEL_FORMAT_YCbCr_420_P,   gcvSURF_I420,
    HAL_PIXEL_FORMAT_CbYCrY_422_I,  gcvSURF_UYVY,

#elif (ANDROID_SDK_VERSION >= 26)
    HAL_PIXEL_FORMAT_YV12,          gcvSURF_YV12,
    /* Legacy formats (deprecated), used by ImageFormat.java */
    HAL_PIXEL_FORMAT_YCbCr_422_SP,  gcvSURF_NV16,
    HAL_PIXEL_FORMAT_YCrCb_420_SP,  gcvSURF_NV21,
    HAL_PIXEL_FORMAT_YCbCr_422_I,   gcvSURF_YUY2,
    HAL_PIXEL_FORMAT_YCbCr_420_SP,  gcvSURF_NV12,

    HAL_PIXEL_FORMAT_YCbCr_420_P,   gcvSURF_I420,
#endif

    0 /* Terminator. */
};


#ifndef LOGE
#   define LOGE(...) ALOGE(__VA_ARGS__)
#endif

static inline gceSURF_FORMAT
gc_gralloc_translate_format(
    int format
    )
{
    for (int i = 0; formatXlate[i] != 0; i += 2)
    {
        if (format == formatXlate[i])
        {
            /* Found. */
            gceSURF_FORMAT halFormat = (gceSURF_FORMAT) formatXlate[i+1];

            if (halFormat == gcvSURF_UNKNOWN)
            {
                LOGE("Not supported ANDROID format: %d", format);
            }

            return halFormat;
        }
    }

#if ANDROID_SDK_VERSION >= 18
    if (format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        /* NV12 for Android flexible YUV 420 format. */
        return gcvSURF_NV12;
    }

    if (format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED)
    {
        /* TODO: Soc-vendor should defined IMPLEMENTATION_DEFINED. */
        ALOGW("HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED not defined");
        ALOGW("Use RGBA_8888");
        return gcvSURF_A8B8G8R8;
    }
#endif

    LOGE("Unknown ANDROID format: %d", format);
    return gcvSURF_UNKNOWN;
}

