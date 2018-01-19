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
    const char *filename,
    const void *pixels,
    size_t offset,
    size_t width,
    size_t height,
    size_t stride,
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

        LOGD("[%zd] %6s %12p %5x %8x %5d %3s  %3d %5s "
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
            LOGD("    region %zd: [%d,%d,%d,%d]",
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
                     size_t(offset),
                     size_t(alignedWidth),
                     size_t(alignedHeight),
                     size_t(stride),
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

enum
{
    RGBA_8888 = gcvSURF_A8B8G8R8,
    BGRA_8888 = gcvSURF_A8R8G8B8,
    RGBX_8888 = gcvSURF_X8B8G8R8,
    BGRX_8888 = gcvSURF_X8R8G8B8,

    RGB_888   = gcvSURF_B8G8R8,
    BGR_888   = gcvSURF_R8G8B8,

    RGB_565   = gcvSURF_R5G6B5,
    BGR_565   = gcvSURF_B5G6R5,

    RGBA_4444 = gcvSURF_R4G4B4A4,
    BGRA_4444 = gcvSURF_B4G4R4A4,

    ARGB_1555 = gcvSURF_A1R5G5B5,
    RGBA_5551 = gcvSURF_R5G5B5A1,

    D16       = gcvSURF_D16,

    D24S8     = gcvSURF_D24S8,
    D32       = gcvSURF_D32,
    D24X8     = gcvSURF_D24X8,
    S8        = gcvSURF_S8,

    L8        = gcvSURF_L8,
};

#   include <stdint.h>
#   include <sys/stat.h>
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

static void check_endian()
{
    uint32_t v = 0x01020304;
    _big_endian = ((uint8_t *) &v)[0] == 0x01;
}

static uint16_t convert_le16(uint16_t word)
{
    if (_big_endian) {
        word = ((word & 0x00ff) << 8) |
               ((word & 0xff00) >> 8);
    }

    return word;
}

static uint32_t convert_le32(uint32_t dword)
{
    if (_big_endian) {
        dword = ((dword & 0x000000ff) << 24) |
                ((dword & 0x0000ff00) << 8)  |
                ((dword & 0x00ff0000) >> 8)  |
                ((dword & 0xff000000) >> 24);
    }

    return dword;
}

int
_WriteBitmap(
    const char *filename,
    const void *pixels,
    size_t offset,
    size_t width,
    size_t height,
    size_t stride,
    int format
    )
{
    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;

    FILE * fp = NULL;
    size_t i;
    int bytes_per_pixel;
    /* Buffer for dest data line. */
    unsigned char * buff = NULL;
    /* Source data line. */
    const unsigned char * line;
    /* Dest stride. */
    size_t dstride;
    size_t nmemb;

    /* Check endian. */
    check_endian();

    /* Check filename. */
    if (filename == NULL) {
        ALOGE("%s: filename is NULL.\n", __FUNCTION__);
        return -EINVAL;
    }

    /* Check data. */
    if (pixels == NULL) {
        ALOGE("%s: pixel data are empty.\n", __FUNCTION__);
        return -EINVAL;
    }

    /* Check width and height. */
    if (width <= 0 || height <= 0) {
        ALOGE("%s: invalid size: width=%zu, height=%zu\n",
            __FUNCTION__, width, height);
        return -EINVAL;
    }

    switch (format)
    {
    case D16:
        ALOGI("%s: fake D16 as RGBA_4444\n", __FUNCTION__);
        format = RGBA_4444;
        break;
    case D24S8:
        ALOGI("%s: fake D24S8 as RGBA_8888\n", __FUNCTION__);
        format = RGBA_8888;
        break;
    case D24X8:
        ALOGI("%s: fake D24X8 as RGBA_8888\n", __FUNCTION__);
        format = RGBA_8888;
        break;
    case D32:
        ALOGI("%s: fake D32 as RGBA_8888\n", __FUNCTION__);
        format = RGBA_8888;
        break;
    case S8:
        ALOGI("%s: fake S8 as L8\n", __FUNCTION__);
        format = L8;
        break;
    default:
        break;
    }

    /* Check source format and get bytes per pixel. */
    switch (format) {
        case RGBA_8888:
        case BGRA_8888:
        case RGBX_8888:
        case BGRX_8888:
            bytes_per_pixel = 4;
            break;

        case RGB_888:
        case BGR_888:
            bytes_per_pixel = 3;
            break;

        case RGB_565:
        case BGR_565:
        case RGBA_4444:
        case BGRA_4444:
        case ARGB_1555:
        case RGBA_5551:
            bytes_per_pixel = 2;
            break;

        case L8:
            bytes_per_pixel = 1;
            break;

        default:
            ALOGE("%s: unknown format=%d\n", __FUNCTION__, format);
            return -EINVAL;
    }

    /* Check source stride. */
    if (stride == 0) {
        /* Source is contiguous. */
        stride = (size_t) width * bytes_per_pixel;
    } else if (stride < (size_t) width * bytes_per_pixel) {
        /* Source has stride. */
        ALOGE("%s: invalid stride=%zu\n", __FUNCTION__, stride);
        return -EINVAL;
    }

    /* Compute dest bytes_per_pixel. */
    switch (format) {
        case RGBA_8888:
        case BGRA_8888:
            /* Save as BGRA_8888. */
            bytes_per_pixel = 4;
            break;

        case RGBA_4444:
        case BGRA_4444:
            /* Convert to BGRA 8888. */
            bytes_per_pixel = 4;
            break;

        case ARGB_1555:
        case RGBA_5551:
            /* Save as ARGB_1555. */
            bytes_per_pixel = 2;
            break;

        default:
            /* Convert to BGR 888 for other formats. */
            bytes_per_pixel = 3;
            break;
    }

    /* Compute number of bytes per dstride in bitmap file: align by 4. */
    dstride = ((width * bytes_per_pixel) + 3) & ~3;

    /* Build file header. */
    bmf.bfType        = convert_le16(0x4D42);
    bmf.bfSize        = convert_le32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER)
            + dstride * height);
    bmf.bfReserved1   = convert_le32(0);
    bmf.bfReserved2   = convert_le32(0);
    bmf.bfOffBits     = convert_le32(
        sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER));

    /* Build bmi. */
    memset(&bmi, 0, sizeof (BITMAPINFOHEADER));
    bmi.biSize        = convert_le32(sizeof (BITMAPINFOHEADER));
    bmi.biWidth       = convert_le32(width);
    bmi.biHeight      = convert_le32(height);
    bmi.biPlanes      = convert_le16(1);
    bmi.biBitCount    = convert_le16(bytes_per_pixel * 8);
    bmi.biCompression = convert_le32(BI_RGB);
    bmi.biSizeImage   = convert_le32(dstride * height);

    /* Open dest file. */
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        ALOGE("%s: failed to open %s for write -- %s.\n",
            __FUNCTION__, filename, strerror(errno));
        return -EACCES;
    }

    /* Write file header. */
    nmemb = fwrite(&bmf, 1, sizeof (BITMAPFILEHEADER), fp);

    if (nmemb != sizeof (BITMAPFILEHEADER)) {
        ALOGE("%s: failed to write file header.\n", __FUNCTION__);
        return -EACCES;
    }

    /* Write info header. */
    nmemb = fwrite(&bmi, 1, sizeof (BITMAPINFOHEADER), fp);

    if (nmemb != sizeof (BITMAPINFOHEADER)) {
        ALOGE("%s: failed to write info header.\n", __FUNCTION__);
        return -EACCES;
    }

    /* Malloc memory for single dstride. */
    buff = (unsigned char *) malloc(dstride);

    if (buff == NULL) {
        ALOGE("%s: out of memory.\n", __FUNCTION__);
        return -ENOMEM;
    }

    if (offset == 0) {
        /* No offset: calculate bottom part start offset. */
        offset = stride * height / 2;
    }

again:
    /* Go to last line.
     * Bitmap is bottom-top, our date is top-bottom.
     * bottom half part. */
    line = (const unsigned char *) pixels + offset + stride * (height / 2 - 1);

    for (i = 0; i < (size_t) height / 2; i++) {
        /* Process each line. */
        switch (format) {
            case RGBA_8888:
                /* Swap RB to BGRA 8888. */
                {
                    size_t j;
                    uint32_t *s = (uint32_t *) line;
                    uint32_t *p = (uint32_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        uint32_t pix = *s++;
                        *p++ =
                            ((pix & 0xff0000) >> 16) |
                            ((pix & 0xff) << 16) |
                            (pix & 0xff00ff00);
                    }
                }
                break;

            case BGRA_8888:
                /* No conversion BGRA 8888. */
                memcpy(buff, line, width * 4);
                break;

            case RGBX_8888:
                /* Swap RB and skip A to BGR 888. */
                {
                    size_t j;
                    uint32_t *s = (uint32_t *) line;
                    uint8_t  *p = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        uint32_t pix = *s++;
                        *p++ = (pix & 0xff0000) >> 16;
                        *p++ = (pix & 0xff00) >> 8;
                        *p++ = (pix & 0xff);
                    }
                }
                break;

            case BGRX_8888:
                /* Skip X to BGR 888. */
                {
                    size_t j;
                    uint32_t *s = (uint32_t *) line;
                    uint8_t  *p = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        uint32_t pix = *s++;
                        *p++ = (pix & 0xff);
                        *p++ = (pix & 0xff00) >> 8;
                        *p++ = (pix & 0xff0000) >> 16;
                    }
                }
                break;

            case RGB_888:
                /* Swap RB to BGR 888. */
                {
                    size_t j;
                    uint8_t *s = (uint8_t *) line;
                    uint8_t *p = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        *p++ = s[2];
                        *p++ = s[1];
                        *p++ = s[0];
                        s += 3;
                    }
                }
                break;

            case BGR_888:
                /* No conversion: BGR 888. */
                memcpy(buff, line, width * 3);
                break;

            case RGB_565:
                /* Convert to BGR 888. */
                {
                    size_t j;
                    uint16_t *s = (uint16_t *) line;
                    uint8_t  *p = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint16_t pix = *s++;
                        *p++ = (pix & 0x001F) << 3;
                        *p++ = (pix & 0x07E0) >> 3;
                        *p++ = (pix & 0xF800) >> 8;
                    }
                }
                break;

            case BGR_565:
                /* Convert to BGR 888. */
                {
                    size_t j;
                    uint16_t *s = (uint16_t *) line;
                    uint8_t *p  = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint16_t pix = *s++;
                        *p++ = (pix & 0xF800) >> 8;
                        *p++ = (pix & 0x07E0) >> 3;
                        *p++ = (pix & 0x001F) << 3;
                    }
                }
                break;

            case RGBA_4444:
                /* Convert to BGRA 8888. */
                {
                    size_t j;
                    uint16_t *s = (uint16_t *) line;
                    uint32_t *p = (uint32_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint16_t pix = *s++;
                        *p++ = ( pix & 0xF0) |
                               ((pix & 0x0F00) << 4) |
                               ((pix & 0xF000) << 8) |
                               ((pix & 0x0F) << 28);
                    }
                }
                break;

            case BGRA_4444:
                /* Convert to BGRA 8888. */
                {
                    size_t j;
                    uint16_t *s = (uint16_t *) line;
                    uint32_t *p = (uint32_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint16_t pix = *s++;
                        *p++ = ((pix & 0xF000) >> 8) |
                               ((pix & 0x0F00) << 4) |
                               ((pix & 0xF0) << 16) |
                               ((pix & 0x0F) << 28);
                    }
                }
                break;

            case L8:
                /* Convert to BGR 888. */
                {
                    size_t j;
                    uint8_t *s = (uint8_t *) line;
                    uint8_t *p = (uint8_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint8_t pix = *s++;
                        *p++ = pix;
                        *p++ = pix;
                        *p++ = pix;
                    }
                }
                break;

            case ARGB_1555:
                /* No conversion: ARGB 1555. */
                memcpy(buff, line, width * 2);
                break;

            case RGBA_5551:
                /* ARGB 1555 -> RGBA 5551. */
                {
                    size_t j;
                    uint16_t *s = (uint16_t *) line;
                    uint16_t *p = (uint16_t *) buff;

                    for (j = 0; j < (size_t) width; j++) {
                        /* low bits are '0' currently. */
                        uint16_t pix = *s++;
                        *p++ = ((pix & 0x7FFF) << 1) | ((pix & 0x8000) >> 15);
                    }
                }
                break;
        }

        /* Write a line. */
        nmemb = fwrite(buff, 1, dstride, fp);

        if (nmemb != dstride) {
            ALOGE("%s: failed to write more pixels\n", __FUNCTION__);
            ALOGE("%s: bitmap is incomplete.\n", __FUNCTION__);

            /* Difficult to recover. */
            free(buff);
            fclose(fp);

            return -EACCES;
        }

        /* Go to next source line. */
        line -= stride;
    }

    if (offset != 0) {
        /* finished bottom half part and work on top half part. */
        offset = 0;
        goto again;
    }

    /* Free buffer. */
    free(buff);

    /* Close file. */
    fclose(fp);

#ifdef __linux__
    /* Change file mode. */
    chmod(filename, 0644);
#endif

    return 0;
}

