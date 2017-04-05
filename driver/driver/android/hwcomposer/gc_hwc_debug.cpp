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


#include "gc_hwc.h"
#include "gc_hwc_debug.h"
#include "gc_gralloc_priv.h"

#include <sys/stat.h>

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
    const char * filename,
    gcoSURF surface
    );

void
hwcDumpLayer(
    IN hwc_layer_list_t * List
    )
{
    LOGD("      type      handle    flag     usage   pid   tr  alpha  blend  format     size    sourceCrop => displayFrame");

    for (size_t i = 0; i < List->numHwLayers; i++)
    {
        int usage = 0;
        int pid = 0;
        int alpha = 255;
        int width  = 0;
        int height = 0;
        int format = 0;
        const char * type;
        const char * blend;
        hwc_layer_t *  layer   = &List->hwLayers[i];
        hwc_rect_t *   srcRect = &layer->sourceCrop;
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
        }

        switch (layer->compositionType)
        {
        case HWC_FRAMEBUFFER:
            type = "  FB   ";
            break;

        case HWC_OVERLAY:
            type = "OVERLAY";
            break;

        case HWC_BLITTER:
            type = "BLITTER";
            break;

        case HWC_DIM:
            type = "  DIM  ";
            break;

        case HWC_CLEAR_HOLE:
            type = "CLRHOLE";
            break;

        default:
            type = "UNKNOWN";
            break;
        }

        alpha = (layer->blending >> 16);

        switch (layer->blending & 0xFFFF)
        {
        case 0:
        case HWC_BLENDING_NONE:
            blend = "OFF";
            break;

        case HWC_BLENDING_PREMULT:
            blend = " ON";
            break;

        case HWC_BLENDING_COVERAGE:
            blend = "COV";
            break;

        case 0x0805:
            blend = "DIM";
            break;

        default:
            blend = "ERR";
            break;
        }

        LOGD("[%d]: %s  %10p  %4x  %8x  %5d   %d   %3d    %s     %3d   %4dx%4d  [%d,%d,%d,%d] [%d,%d,%d,%d]",
             i, type,
             layer->handle,
             layer->flags,
             usage, pid,
             layer->transform,
             alpha,
             blend,
             format,
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
}


void
hwcDumpArea(
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
    IN hwc_layer_list_t * List,
    IN android_native_buffer_t * BackBuffer
    )
{
    static int frame;
    char fname[64];
    struct stat st;

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

    /* frame number. */
    frame++;
    LOGD("BITMAPS:");

    for (gctUINT32 i = 0; i < List->numHwLayers; i++)
    {
        buffer_handle_t handle = List->hwLayers[i].handle;
        gcoSURF surface;
        gctUINT width, height;

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

        /* Layer size. */
        gcmVERIFY_OK(
            gcoSURF_GetSize(surface, &width, &height, gcvNULL));

        /* Build bitmap file name. */
        snprintf(fname,
                 64,
                 "/data/dump/f%d_l%d_h%p_s%dx%d.bmp",
                 frame,
                 i,
                 handle,
                 width,
                 height);

        /* Write surface to bitmap. */
        _WriteSurface(fname, surface);
    }

    if (BackBuffer != NULL)
    {
        buffer_handle_t handle = BackBuffer->handle;
        gcoSURF surface;
        gctUINT width, height;

        if (handle== gcvNULL)
        {
            /* Error? */
            return;
        }

        /* Get source surface. */
        surface = (gcoSURF) gc_native_handle_get(handle)->surface;

        if (surface == gcvNULL)
        {
            /* Error? */
            return;
        }

        /* Layer size. */
        gcmVERIFY_OK(
            gcoSURF_GetSize(surface, &width, &height, gcvNULL));

        /* Build bitmap file name. */
        snprintf(fname,
                 64,
                 "/data/dump/f%d_fb_h%p_s%dx%d.bmp",
                 frame,
                 handle,
                 width,
                 height);

        /* Write surface to bitmap. */
        _WriteSurface(fname, surface);
    }
}

void
_WriteSurface(
    const char * filename,
    gcoSURF surface
    )
{
    gctUINT width, height;
    gceSURF_FORMAT format;
    gctINT stride;
    gceTILING tiling;

    gctUINT32 address[3];
    gctPOINTER memory[3];
    unsigned int offset = 0;

    /* Surface aligned size. */
    gcmVERIFY_OK(
        gcoSURF_GetAlignedSize(surface, &width, &height, &stride));

    /* Surface format. */
    gcmVERIFY_OK(
        gcoSURF_GetFormat(surface, gcvNULL, &format));

    /* Surface tiling. */
    gcmVERIFY_OK(
        gcoSURF_GetTiling(surface, &tiling));

    /* Lock for memory. */
    gcmVERIFY_OK(
        gcoSURF_Lock(surface, address, memory));

    if (tiling & gcvTILING_SPLIT_BUFFER)
    {
        gcmVERIFY_OK(
            gcoSURF_GetBottomBufferOffset(surface, &offset));
    }

    /* Print more information. */
    LOGD("  %s: fromat=%d stride=%d alignedSize=%dx%d physical=%x",
         filename, format, stride, width, height, address[0]);

    /* Write surface to bitmap. */
    _WriteBitmap(filename,
                 memory[0],
                 offset,
                 width,
                 height,
                 stride,
                 format);

    /* Unlock. */
    gcmVERIFY_OK(
        gcoSURF_Unlock(surface, memory[0]));
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
        LOGE("Can not write file header.");
        return -EACCES;
    }

    /* Write info header. */
    if (fwrite(&info_header, 1, sizeof (BITMAPINFOHEADER), fp)
        != sizeof (BITMAPINFOHEADER))
    {
        LOGE("Can not write info header.");
        return -EACCES;
    }

    /* Malloc memory for single dstride. */
    buff = (unsigned char *) malloc(dstride);

    if (buff == NULL)
    {
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

