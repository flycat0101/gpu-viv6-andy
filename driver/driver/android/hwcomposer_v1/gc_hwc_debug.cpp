/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_hwc.h"
#include "gc_hwc_debug.h"
#include "gc_gralloc_priv.h"

#include <math.h>
#include <sys/stat.h>

static int
_WriteRawData(
    const char * filename,
    void * data[3],
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    int format
    );

static int
_WriteBitmap(
    const char * filename,
    const void * data,
    unsigned int offset,
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    int format
    );

static void
_WriteSurface(
    const char *Prefix,
    buffer_handle_t Handle,
    gcoSURF Surface
    );

void
hwcDumpLayer(
    IN hwcContext * Context,
    IN hwc_display_contents_1_t * HwDisplay,
    IN hwcDisplay * Display
    )
{
    buffer_handle_t outbuf;
    size_t layerCount;

    if (uintptr_t(HwDisplay->outbuf) > 0x04)
    {
        /* Dump outbuf instead of framebuffer target. */
        outbuf     = HwDisplay->outbuf;
        layerCount = HwDisplay->numHwLayers - 1;
    }
    else
    {
        /* Dump framebuffer target. */
        outbuf     = NULL;
        layerCount = HwDisplay->numHwLayers;
    }

    LOGD("      type       handle  flag    usage   pid rot alph blend "
         "fmt tiling      size sourceCrop => displayFrame");

    for (size_t i = 0; i < layerCount; i++)
    {
        int usage  = 0;
        int pid    = 0;
        int alpha  = 255;
        int width  = 0;
        int height = 0;
        int format = 0;
        gceTILING tiling = gcvINVALIDTILED;
        const char * typeStr;
        const char * blendStr;
        const char * transStr;
        const char * tilingStr;
        hwc_layer_1_t * layer = &HwDisplay->hwLayers[i];
        hwc_rect_t * srcRect;

#if ANDROID_SDK_VERSION >= 19
        hwc_rect_t sourceCropi;

        if (Context->device.common.version >= HWC_DEVICE_API_VERSION_1_3)
        {
            sourceCropi.left   = int(ceilf(layer->sourceCropf.left));
            sourceCropi.top    = int(ceilf(layer->sourceCropf.top));
            sourceCropi.right  = int(floorf(layer->sourceCropf.right));
            sourceCropi.bottom = int(floorf(layer->sourceCropf.bottom));
            srcRect = &sourceCropi;
        }
        else
#endif
        {
            srcRect = &layer->sourceCrop;
        }

        hwc_rect_t *   dstRect = &layer->displayFrame;
        hwc_region_t * region  = &layer->visibleRegionScreen;

        if (layer->handle != NULL)
        {
            gc_native_handle_t * handle =
                gc_native_handle_get(layer->handle);

            usage = handle->allocUsage;
            pid = handle->clientPID;
            width  = handle->width;
            height = handle->height;
            format = handle->halFormat;
            gcoSURF_GetTiling(gcoSURF(handle->surface), &tiling);
        }

        switch (layer->compositionType)
        {
        case HWC_FRAMEBUFFER:
            typeStr = "  GLES";
            break;
        case HWC_FRAMEBUFFER_TARGET:
            typeStr = "FB-TRG";
            break;
        case HWC_BACKGROUND:
            typeStr = "BK-GND";
            break;
#if ANDROID_SDK_VERSION >= 21
        case HWC_SIDEBAND:
            typeStr = "S-BAND";
            break;
        case HWC_CURSOR_OVERLAY:
            typeStr = "CURSOR";
            break;
#endif
        case HWC_OVERLAY:
            switch (Display->layers[i].compositionType)
            {
            case HWC_BLITTER:
                typeStr = "2D-BLT";
                break;
            case HWC_DIM:
                typeStr = "2D-DIM";
                break;
            case HWC_CLEAR_HOLE:
                typeStr = "2D-HOL";
                break;
            default:
                typeStr = "OVRLAY";
                break;
            }
            break;

        default:
            typeStr = " ERROR";
            break;
        }

#if ANDROID_SDK_VERSION >= 18
        alpha = layer->planeAlpha;
#elif ENABLE_PLANE_ALPHA || ENABLE_DIM
        alpha = layer->alpha;
#endif

        switch (layer->blending & 0xFFFF)
        {
        case 0:
        case HWC_BLENDING_NONE:
            blendStr = "OFF";
            break;

        case HWC_BLENDING_PREMULT:
            blendStr = "   ON";
            break;

        case HWC_BLENDING_COVERAGE:
            blendStr = "COVER";
            break;

        case 0x0805:
            blendStr = "  DIM";
            break;

        default:
            blendStr = "ERROR";
            break;
        }

        switch (layer->transform)
        {
        case 0:
        default:
            transStr = "  0";
            break;
        case HWC_TRANSFORM_FLIP_H:
            transStr = "  H";
            break;
        case HWC_TRANSFORM_FLIP_V:
            transStr = "  V";
            break;
        case HWC_TRANSFORM_ROT_90:
            transStr = " 90";
            break;
        case HWC_TRANSFORM_ROT_180:
            transStr = "180";
            break;
        case HWC_TRANSFORM_ROT_270:
            transStr = "270";
            break;
        case (HWC_TRANSFORM_ROT_90 | HWC_TRANSFORM_FLIP_H):
            transStr = "90H";
            break;
        case (HWC_TRANSFORM_ROT_90 | HWC_TRANSFORM_FLIP_V):
            transStr = "90V";
            break;
        }

        switch (tiling)
        {
        case gcvLINEAR:
            tilingStr = "LINEAR";
            break;
        case gcvTILED:
            tilingStr = " TILED";
            break;
        case gcvSUPERTILED:
            tilingStr = "SUPERT";
            break;
        case gcvMULTI_TILED:
            tilingStr = "    MT";
            break;
        case gcvMULTI_SUPERTILED:
            tilingStr = "   MST";
            break;
        default:
            tilingStr = "      ";
            break;
        }

        LOGD("[%d] %6s %12p %5x %8x %5d %3s  %3d %5s "
             "%3d %6s %4dx%-4d [%d,%d,%d,%d] [%d,%d,%d,%d]",
             i, typeStr,
             layer->handle,
             layer->flags,
             usage, pid,
             transStr,
             alpha,
             blendStr,
             format,
             tilingStr,
             width,
             height,
             srcRect->left,
             srcRect->top,
             srcRect->right,
             srcRect->bottom,
             dstRect->left,
             dstRect->top,
             dstRect->right,
             dstRect->bottom);

        if ((region->numRects == 1)
        &&  (region->rects[0].left   == dstRect->left)
        &&  (region->rects[0].top    == dstRect->top)
        &&  (region->rects[0].right  == dstRect->right)
        &&  (region->rects[0].bottom == dstRect->bottom)
        )
        {
            /* Skip print if region equals displayFrame. */
            continue;
        }

        for (size_t j = 0; j < region->numRects; j++)
        {
            LOGD("    region %d: [%d,%d,%d,%d]",
                 j,
                 region->rects[j].left,
                 region->rects[j].top,
                 region->rects[j].right,
                 region->rects[j].bottom);
        }
    }

    if (outbuf)
    {
        int usage  = 0;
        int pid    = 0;
        int width  = 0;
        int height = 0;
        int format = 0;
        gceTILING tiling = gcvINVALIDTILED;
        const char * tilingStr;

        gc_native_handle_t * handle = gc_native_handle_get(outbuf);

        usage = handle->allocUsage;
        pid = handle->clientPID;
        width  = handle->width;
        height = handle->height;
        format = handle->halFormat;
        gcoSURF_GetTiling(gcoSURF(handle->surface), &tiling);

        switch (tiling)
        {
        case gcvLINEAR:
            tilingStr = "LINEAR";
            break;
        case gcvTILED:
            tilingStr = " TILED";
            break;
        case gcvSUPERTILED:
            tilingStr = "SUPERT";
            break;
        case gcvMULTI_TILED:
            tilingStr = "    MT";
            break;
        case gcvMULTI_SUPERTILED:
            tilingStr = "   MST";
            break;
        default:
            tilingStr = "      ";
            break;
        }

        LOGD("    outbuf %12p       %8x %5d                "
             "%3d %6s %4dx%-4d",
             outbuf,
             usage, pid,
             format,
             tilingStr,
             width,
             height);
    }
}


void
hwcDumpArea(
    IN hwcContext * Context,
    IN hwcArea * Area
    )
{
    hwcArea * area = Area;

    while (area != NULL)
    {
        char buf[128];
        char digit[8];
        bool first = true;

        snprintf(buf, 128,
                 " [%d,%d,%d,%d] %08x:",
                 area->rect.left,
                 area->rect.top,
                 area->rect.right,
                 area->rect.bottom,
                 area->owners);

        for (gctUINT32 i = 0; i < 32; i++)
        {
            /* Build decimal layer indices. */
            if (area->owners & (1U << i))
            {
                if (first)
                {
                    snprintf(digit, 8, " %d", i);
                    strcat(buf, digit);
                    first = false;
                }
                else
                {
                    snprintf(digit, 8, ",%d", i);
                    strcat(buf, digit);
                }
            }

            if (area->owners < (1U << i))
            {
                break;
            }
        }

        LOGD("%s", buf);

        /* Advance to next area. */
        area = area->next;
    }
}


void
hwcDumpBitmap(
    IN hwcContext * Context,
    IN hwc_display_contents_1_t* HwDisplay,
    IN hwcDisplay * Display
    )
{
    static int frame;
    struct stat st;
    buffer_handle_t outbuf;
    size_t layerCount;

    if (stat("/data/dump", &st) != 0)
    {
        /* Try create /data/dump directory. */
        int err = mkdir("/data/dump", 0777);

        if (err)
        {
            LOGE("Failed to create dir: /data/dump/");
            return;
        }
    }

    if (uintptr_t(HwDisplay->outbuf) > 0x04)
    {
        /* Dump outbuf instead of framebuffer target. */
        outbuf     = HwDisplay->outbuf;
        layerCount = HwDisplay->numHwLayers - 1;
    }
    else
    {
        /* Dump framebuffer target. */
        outbuf     = NULL;
        layerCount = HwDisplay->numHwLayers;
    }

    /* frame number. */
    frame++;
    LOGD("BITMAPS of dpy=%d:", Display->disp);

    for (gctUINT32 i = 0; i < layerCount; i++)
    {
        buffer_handle_t handle = HwDisplay->hwLayers[i].handle;
        gcoSURF surface;
        char prefix[32];

        if (handle == gcvNULL)
        {
            /* Skip dumpping no source layers. */
            continue;
        }

        /* Get source surface. */
        surface = (gcoSURF) gc_native_handle_get(handle)->surface;

        if (surface == gcvNULL)
        {
            /* Skip dumpping no source layers. */
            continue;
        }

        /* Write surface to bitmap. */
        if (i == HwDisplay->numHwLayers - 1)
        {
            snprintf(prefix, 32, "/data/dump/f%03d_fb", frame);
        }
        else
        {
            snprintf(prefix, 32, "/data/dump/f%03d_l%d", frame, i);
        }
        _WriteSurface(prefix, handle, surface);
    }

    if (outbuf)
    {
        gcoSURF surface;
        char prefix[32];

        /* Get source surface. */
        surface = (gcoSURF) gc_native_handle_get(outbuf)->surface;

        if (surface == gcvNULL)
        {
            /* Skip dumpping no source layers. */
            return;
        }

        /* Write surface to bitmap. */
        snprintf(prefix, 32, "/data/dump/f%03d_outbuf", frame);
        _WriteSurface(prefix, outbuf, surface);
    }
}

void
_WriteSurface(
    const char *Prefix,
    buffer_handle_t Handle,
    gcoSURF Surface
    )
{
    char filename[64];
    const char *suffix = NULL;
    gctUINT width, height;
    gctUINT alignedWidth, alignedHeight;
    gceSURF_FORMAT format;
    gctINT stride;
    gceTILING tiling;

    gctUINT32 address[3];
    gctPOINTER memory[3];
    unsigned int offset = 0;

    /* Surface format. */
    gcmVERIFY_OK(
        gcoSURF_GetFormat(Surface, gcvNULL, &format));

    /* Layer size. */
    gcmVERIFY_OK(
        gcoSURF_GetSize(Surface, &width, &height, gcvNULL));

    /* Surface aligned size. */
    gcmVERIFY_OK(
        gcoSURF_GetAlignedSize(Surface, &alignedWidth, &alignedHeight, &stride));

    /* Surface tiling. */
    gcmVERIFY_OK(
        gcoSURF_GetTiling(Surface, &tiling));

    /* Lock for memory. */
    gcmVERIFY_OK(
        gcoSURF_Lock(Surface, address, memory));

    if (tiling & gcvTILING_SPLIT_BUFFER)
    {
        gcmVERIFY_OK(
            gcoSURF_GetBottomBufferOffset(Surface, &offset));
    }

    switch (format)
    {
    case gcvSURF_YUY2:
        suffix = "yuy2";
        break;
    case gcvSURF_YVYU:
        suffix = "yvyu";
        break;
    case gcvSURF_NV16:
        suffix = "nv16";
        break;
    case gcvSURF_NV61:
        suffix = "nv61";
        break;
    case gcvSURF_NV12:
        suffix = "nv12";
        break;
    case gcvSURF_NV21:
        suffix = "nv21";
        break;
    case gcvSURF_YV12:
        suffix = "yv12";
        break;
    case gcvSURF_I420:
        suffix = "i420";
        break;
    default:
        break;
    }

    if (suffix)
    {
        /* Build file name. */
        snprintf(filename, 64,
                 "%s_h%p_s%dx%d_a%dx%d.%s",
                 Prefix, Handle,
                 width, height, alignedWidth, alignedHeight, suffix);

        /* Save as raw binary for yuv. */
        _WriteRawData(filename,
                      memory,
                      alignedWidth,
                      alignedHeight,
                      stride,
                      format);
    }
    else
    {
        /* Build bitmap file name. */
        snprintf(filename, 64,
                 "%s_h%p_s%dx%d_a%dx%d.bmp",
                 Prefix, Handle,
                 width, height, alignedWidth, alignedHeight);

        /* Write surface to bitmap. */
        _WriteBitmap(filename,
                     memory[0],
                     offset,
                     alignedWidth,
                     alignedHeight,
                     stride,
                     format);
    }

    /* Print more information. */
    LOGD("  %s: fromat=%d stride=%d physical=%x",
         filename, format, stride, address[0]);

    /* Unlock. */
    gcmVERIFY_OK(
        gcoSURF_Unlock(Surface, memory[0]));
}

/******************************************************************************/

int
_WriteRawData(
    const char * filename,
    void * data[3],
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    int format
    )
{
    FILE *fp;
    size_t length[3] = {0, 0, 0};

    /* Check filename. */
    if (filename == NULL)
    {
        LOGE("filename is NULL.");
        return -EINVAL;
    }

    /* Check data. */
    if (data == NULL)
    {
        LOGE("Pixels data is empty.");
        return -EINVAL;
    }

    /* Check width and height. */
    if (width == 0 || height == 0)
    {
        LOGE("Width: %d, Height: %d", width, height);
        return -EINVAL;
    }

    switch (format)
    {
    case gcvSURF_YUY2:
    case gcvSURF_YVYU:
        length[0] = stride * height;
        break;
    case gcvSURF_NV16:
    case gcvSURF_NV61:
        length[0] = stride * height;
        length[1] = stride * height;
        break;
    case gcvSURF_NV12:
    case gcvSURF_NV21:
        length[0] = stride * height;
        length[1] = stride * height / 2;
        break;
    case gcvSURF_YV12:
    case gcvSURF_I420:
        length[0] = stride * height;
        length[1] = ((stride / 2 + 0xf) & ~0xf) * height / 2;
        length[2] = ((stride / 2 + 0xf) & ~0xf) * height / 2;
        break;
    default:
        length[0] = stride * height;
        break;
    }

    /* Open file for write. */
    fp = fopen(filename, "wb");

    if (fp == NULL)
    {
        LOGE("Failed to open %s for write", filename);
        return -EINVAL;
    }

    for (int i = 0; i < 3; i++)
    {
        /* Write planes. */
        fwrite(data[i], 1, length[i], fp);
    }

    fflush(fp);
    fclose(fp);

    return 0;
}

/******************************************************************************/


#ifndef __pixel_format_
#define __pixel_format_

#include <gc_hal_enum.h>

enum
{
                                    /* Bytes: 0 1 2 3 (Low to high)*/
    RGBA_8888 = gcvSURF_A8B8G8R8,   /*        R G B A */
    BGRA_8888 = gcvSURF_A8R8G8B8,   /*        B G R A */
    RGBX_8888 = gcvSURF_X8B8G8R8,   /*        R G B - */
    BGRX_8888 = gcvSURF_X8R8G8B8,   /*        B G R - */

                                    /* Bytes: 0 1 2 (Low to high) */
    RGB_888   = gcvSURF_R8G8B8,     /*        R G B */
    BGR_888   = gcvSURF_B8G8R8,     /*        B G R */

                                    /* Bits: 15 11 5  0 (MSB to LSB) */
    RGB_565   = gcvSURF_R5G6B5,     /*         R  G  B */
    BGR_565   = gcvSURF_B5G6R5,     /*         B  G  R */

                                    /* Bits: 15 14 10 5  0 (MSB to LSB) */
    ARGB_1555 = gcvSURF_A1R5G5B5    /*        A   R  G  B */
};
#endif

#   include <stdint.h>
typedef uint8_t         BYTE;
typedef uint32_t        BOOL;
typedef uint32_t        DWORD;
typedef uint16_t        WORD;
typedef uint32_t        LONG;

#   define BI_RGB           0L
#   define BI_RLE8          1L
#   define BI_RLE4          2L
#   define BI_BITFIELDS     3L

typedef struct tagBITMAPINFOHEADER
{
        DWORD   biSize;
        LONG    biWidth;
        LONG    biHeight;
        WORD    biPlanes;
        WORD    biBitCount;
        DWORD   biCompression;
        DWORD   biSizeImage;
        LONG    biXPelsPerMeter;
        LONG    biYPelsPerMeter;
        DWORD   biClrUsed;
        DWORD   biClrImportant;
}
__attribute__((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
}
__attribute__((packed)) BITMAPFILEHEADER;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


static int _big_endian = -1;

static void _check_endian()
{
    unsigned long v = 0x01020304;
    _big_endian = ((unsigned char *) &v)[0] == 0x01;
}

static unsigned short _little16(unsigned short word)
{
    if (_big_endian)
    {
        word = ((word & 0x00ff) << 8) |
               ((word & 0xff00) >> 8);
    }

    return word;
}

static unsigned long _little32(unsigned long dword)
{
    if (_big_endian)
    {
        dword = ((dword & 0x000000ff) << 24) |
                ((dword & 0x0000ff00) << 8)  |
                ((dword & 0x00ff0000) >> 8)  |
                ((dword & 0xff000000) >> 24);
    }

    return dword;
}


static int _fconv(
    void * dest,
    int    destformat,
    const void * source,
    int    srcformat,
    int    count
    )
{
    int i;

    /* Get some shortcuts. */
    const uint8_t  * csource = (const uint8_t  *)  source;
    const uint16_t * ssource = (const uint16_t *) source;
    uint8_t  * cdest = (uint8_t  *)  dest;
    uint16_t * sdest = (uint16_t *) dest;

    /* Check source input. */
    if (source == NULL)
    {
        LOGE("Null source input");
        return -EINVAL;
    }

    switch (srcformat)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:

    case RGB_888:
    case BGR_888:

    case RGB_565:
    case BGR_565:

    case ARGB_1555:
        break;

    default:
        LOGE("Invalid source format");
        return -EINVAL;
    }

    /* Check dest input. */
    if (dest == NULL)
    {
        LOGE("Null dest input");
        return -EINVAL;
    }

    switch (srcformat)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:

    case RGB_888:
    case BGR_888:

    case RGB_565:
    case BGR_565:

    case ARGB_1555:
        break;

    default:
        LOGE("Invalid dest format");
        return -EINVAL;
    }

    /* Special case for src format == dest format. */
    if (srcformat == destformat)
    {
        int bypp;

        switch (srcformat)
        {
        case RGBA_8888:
        case BGRA_8888:
        case RGBX_8888:
        case BGRX_8888:
            bypp = 4;
            break;

        case RGB_888:
        case BGR_888:
            bypp = 3;
            break;

        case RGB_565:
        case BGR_565:
        case ARGB_1555:
        default:
            bypp = 2;
            break;
        }

        memcpy(dest, source, bypp * count);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        uint32_t rgba;

        /* Convert to rgba8888. */
        switch (srcformat)
        {
        case RGBA_8888:
        case RGBX_8888:
            rgba = ((uint32_t *) source)[i];
            break;

        case BGRA_8888:
        case BGRX_8888:
            ((uint8_t *) &rgba)[0] = csource[i * 4 + 2];
            ((uint8_t *) &rgba)[1] = csource[i * 4 + 1];
            ((uint8_t *) &rgba)[2] = csource[i * 4 + 0];
            ((uint8_t *) &rgba)[3] = csource[i * 4 + 3];
            break;

        case RGB_888:
            ((uint8_t *) &rgba)[0] = csource[i * 3 + 0];
            ((uint8_t *) &rgba)[1] = csource[i * 3 + 1];
            ((uint8_t *) &rgba)[2] = csource[i * 3 + 2];
            ((uint8_t *) &rgba)[3] = 0xFF;
            break;

        case BGR_888:
            ((uint8_t *) &rgba)[0] = csource[i * 3 + 2];
            ((uint8_t *) &rgba)[1] = csource[i * 3 + 1];
            ((uint8_t *) &rgba)[2] = csource[i * 3 + 0];
            ((uint8_t *) &rgba)[3] = 0xFF;
            break;

        case RGB_565:
            /* R: 11111 000000 00000
             * G: 00000 111111 00000
             * B: 00000 000000 11111 */
            ((uint8_t *) &rgba)[0] = (ssource[i] & 0xF800) >> 8;
            ((uint8_t *) &rgba)[1] = (ssource[i] & 0x07E0) >> 3;
            ((uint8_t *) &rgba)[2] = (ssource[i] & 0x001F) << 3;
            ((uint8_t *) &rgba)[3] = 0xFF;
            break;

        case BGR_565:
            /* R: 00000 000000 11111
             * G: 00000 111111 00000
             * B: 11111 000000 00000 */
            ((uint8_t *) &rgba)[0] = (ssource[i] & 0x001F) << 3;
            ((uint8_t *) &rgba)[1] = (ssource[i] & 0x07E0) >> 3;
            ((uint8_t *) &rgba)[2] = (ssource[i] & 0xF800) >> 8;
            ((uint8_t *) &rgba)[3] = 0xFF;
            break;

        case ARGB_1555:
        default:
            /* R: 0 11111 00000 00000
             * G: 0 00000 11111 00000
             * B: 0 00000 00000 11111
             * A: 1 00000 00000 00000 */
            ((uint8_t *) &rgba)[0] = (ssource[i] & 0x7C00) >> 7;
            ((uint8_t *) &rgba)[1] = (ssource[i] & 0x03E0) >> 2;
            ((uint8_t *) &rgba)[2] = (ssource[i] & 0x001F) << 3;
            ((uint8_t *) &rgba)[3] = (ssource[i] & 0x8000) >> 8;
            break;
        }

        /* Convert to dest format. */
        switch (destformat)
        {
        case RGBA_8888:
        case RGBX_8888:
            ((uint32_t *) dest)[i] = rgba;
            break;

        case BGRA_8888:
        case BGRX_8888:
            cdest[i * 4 + 2] = ((uint8_t *) &rgba)[0];
            cdest[i * 4 + 1] = ((uint8_t *) &rgba)[1];
            cdest[i * 4 + 0] = ((uint8_t *) &rgba)[2];
            cdest[i * 4 + 3] = ((uint8_t *) &rgba)[3];
            break;

        case RGB_888:
            cdest[i * 3 + 0] = ((uint8_t *) &rgba)[0];
            cdest[i * 3 + 1] = ((uint8_t *) &rgba)[1];
            cdest[i * 3 + 2] = ((uint8_t *) &rgba)[2];
            break;

        case BGR_888:
            cdest[i * 3 + 2] = ((uint8_t *) &rgba)[0];
            cdest[i * 3 + 1] = ((uint8_t *) &rgba)[1];
            cdest[i * 3 + 0] = ((uint8_t *) &rgba)[2];
            break;

        case RGB_565:
            /* R: 11111 000000 00000
             * G: 00000 111111 00000
             * B: 00000 000000 11111 */
            sdest[i] =
                ((((uint8_t *) &rgba)[0] & 0xF8) << 8) |
                ((((uint8_t *) &rgba)[1] & 0xFC) << 3) |
                ((((uint8_t *) &rgba)[2] & 0xF8) >> 3);
            break;

        case BGR_565:
            /* R: 00000 000000 11111
             * G: 00000 111111 00000
             * B: 11111 000000 00000 */
            sdest[i] =
                ((((uint8_t *) &rgba)[0] & 0xF8) >> 3) |
                ((((uint8_t *) &rgba)[1] & 0xFC) << 3) |
                ((((uint8_t *) &rgba)[2] & 0xF8) << 8);
            break;

        case ARGB_1555:
        default:
            /* R: 0 11111 00000 00000
             * G: 0 00000 11111 00000
             * B: 0 00000 00000 11111
             * A: 1 00000 00000 00000 */
            sdest[i] =
            ((((uint8_t *) &rgba)[0] & 0xF8) << 7) |
            ((((uint8_t *) &rgba)[1] & 0xF8) << 2) |
            ((((uint8_t *) &rgba)[2] & 0xF8) >> 3) |
            ((((uint8_t *) &rgba)[3] & 0x80) << 8);
            break;
        }
    }

    return 0;
}


int _WriteBitmap(
    const char * filename,
    const void * data,
    unsigned int offset,
    unsigned int width,
    unsigned int height,
    unsigned int stride,
    int format
    )
{
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;

    FILE * fp = NULL;
    unsigned int i;
    unsigned short  bypp;
    /* Buffer for dest data line. */
    unsigned char * buff = NULL;
    /* Source data line. */
    const unsigned char * line;
    /* Dest stride. */
    unsigned int    dstride;
    /* Rows in one write batch. */
    unsigned int batch;

    /* Check endian. */
    _check_endian();

    /* Check filename. */
    if (filename == NULL)
    {
        LOGE("filename is NULL.");
        return -EINVAL;
    }

    /* Check data. */
    if (data == NULL)
    {
        LOGE("Pixels data is empty.");
        return -EINVAL;
    }

    /* Check width and height. */
    if (width == 0 || height == 0)
    {
        LOGE("Width: %d, Height: %d", width, height);
        return -EINVAL;
    }

    /* Check source format and get bytes per pixel. */
    switch (format)
    {
    case RGBA_8888:
    case BGRA_8888:
    case RGBX_8888:
    case BGRX_8888:
        bypp = 4;
        break;

    case RGB_888:
    case BGR_888:
        bypp = 3;
        break;

    case RGB_565:
    case BGR_565:
    case ARGB_1555:
        bypp = 2;
        break;

    default:
        LOGE("Invalid format: %d", format);
        return -EINVAL;
    }

    /* Check source stride. */
    if (stride == 0)
    {
        /* Source is contiguous. */
        stride = width * bypp;
    }
    else
    /* Source has stride. */
    if (stride < width * bypp)
    {
        LOGE("Invalid stride: %d", stride);
        return -EINVAL;
    }

    /* Compute dest bypp. */
    switch (format)
    {
    case RGBA_8888:
    case BGRA_8888:
        bypp = 4;
        break;

    case ARGB_1555:
        bypp = 2;
        break;

    default:
        /* Convert to BGR 888 for other formats. */
        bypp = 3;
        break;
    }

    /* Compute number of bytes per dstride in bitmap file: align by 4. */
    dstride = ((width * bypp) + 3) & ~3;

    /* Build file header. */
    file_header.bfType        = _little16(0x4D42);
    file_header.bfSize        = _little32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER)
            + dstride * height);
    file_header.bfReserved1   = _little32(0);
    file_header.bfReserved2   = _little32(0);
    file_header.bfOffBits     = _little32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER));

    /* Build info_header. */
    memset(&info_header, 0, sizeof (BITMAPINFOHEADER));
    info_header.biSize        = _little32(sizeof (BITMAPINFOHEADER));
    info_header.biWidth       = _little32(width);
    info_header.biHeight      = _little32(height);
    info_header.biPlanes      = _little16(1);
    info_header.biBitCount    = _little16(bypp * 8);
    info_header.biCompression = _little32(BI_RGB);
    info_header.biSizeImage   = _little32(dstride * height);

    /* Open dest file. */
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        LOGE("Can not open %s for write.", filename);
        return -EACCES;
    }

    /* Write file header. */
    if (fwrite(&file_header, 1, sizeof (BITMAPFILEHEADER), fp)
        != sizeof (BITMAPFILEHEADER))
    {
        fclose(fp);
        LOGE("Can not write file header.");
        return -EACCES;
    }

    /* Write info header. */
    if (fwrite(&info_header, 1, sizeof (BITMAPINFOHEADER), fp)
        != sizeof (BITMAPINFOHEADER))
    {
        fclose(fp);
        LOGE("Can not write info header.");
        return -EACCES;
    }

    /* Malloc memory for single dstride. */
    buff = (unsigned char *) malloc(dstride);

    if (buff == NULL)
    {
        fclose(fp);
        LOGE("Out of memory.");
        return -ENOMEM;
    }

    /* Go to last line.
     * Bitmap is bottom-top, our date is top-bottom. */
    if (offset > 0)
    {
        batch = height / 2;
        line = (const unsigned char *) data + offset + stride * (batch - 1);
    }
    else
    {
        batch = height;
        line = (const unsigned char *) data + stride * (batch - 1);
    }

again:
    for (i = 0; i < batch; i++)
    {
        /* Process each line. */
        switch (format)
        {
        case RGBA_8888:
            /* Swap RB to BGRA 8888. */
            _fconv(buff, BGRA_8888, line, RGBA_8888, width);
            break;

        case BGRA_8888:
            /* No conversion BGRA 8888. */
            memcpy(buff, line, width * 4);
            break;

        case RGBX_8888:
            /* Swap RB and skip A to BGR 888. */
            _fconv(buff, BGR_888, line, RGBX_8888, width);
            break;

        case BGRX_8888:
            /* Skip A to BGR 888. */
            _fconv(buff, BGR_888, line, BGRX_8888, width);
            break;

        case RGB_888:
            /* Swap RB to BGR 888. */
            _fconv(buff, BGR_888, line, RGB_888, width);
            break;

        case BGR_888:
            /* No conversion: BGR 888. */
            _fconv(buff, BGR_888, line, BGR_888, width);
            break;

        case RGB_565:
            /* Convert to BGR 888. */
            _fconv(buff, BGR_888, line, RGB_565, width);
            break;

        case BGR_565:
            /* Convert to BGR 888. */
            _fconv(buff, BGR_888, line, BGR_565, width);
            break;

        case ARGB_1555:
            /* No conversion: ARGB 15555. */
            memcpy(buff, line, width * 2);
            break;
        }

        /* Write a line. */
        if (fwrite(buff, 1, dstride, fp) != dstride)
        {
            LOGE("Error: Can not write file");
            LOGE("Bitmap is incomplete.");

            /* Difficult to recover. */
            free(buff);
            fclose(fp);

            return -EACCES;
        }

        /* Go to next source line. */
        line -= stride;
    }

    if (offset > 0)
    {
        line = (const unsigned char *) data + stride * (batch - 1);
        offset = 0;
        goto again;
    }

    /* Free buffer. */
    free(buff);

    /* Close file. */
    fclose(fp);

    /* Change file mode. */
    chmod(filename, 0644);

    return 0;
}

