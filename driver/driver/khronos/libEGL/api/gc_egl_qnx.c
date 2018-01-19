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


#define _QNX_SOURCE

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/queue.h>

#include <stdlib.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <screen/screen.h>
#include <KHR/khronos_utils.h>

#include "gc_egl_platform.h"

#define _GC_OBJ_ZONE    gcvZONE_OS

typedef int             PlatformDisplayType;
typedef screen_window_t PlatformWindowType;
typedef screen_pixmap_t PlatformPixmapType;


static gceSTATUS
qnx_GetScreenWindowBufferAddress(
    IN screen_buffer_t ScreenBuffer,
    OUT gctPOINTER * Logical,
    OUT gctUINT32 * Physical
    )
{
    int contiguous;
    gctINT64 paddr;

    /* The logical address of the window. */
    if (screen_get_buffer_property_pv(ScreenBuffer, SCREEN_PROPERTY_POINTER, Logical))
    {
        /* Failed to get logical address. */
        return  gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Whether or not it is physically contiguous. */
    if (screen_get_buffer_property_iv(ScreenBuffer, SCREEN_PROPERTY_PHYSICALLY_CONTIGUOUS, &contiguous))
    {
        return  gcvSTATUS_INVALID_ARGUMENT;
    }

    if (contiguous)
    {
        /* The physical address of the window. */
        if (screen_get_buffer_property_llv(ScreenBuffer, SCREEN_PROPERTY_PHYSICAL_ADDRESS, &paddr))
        {
            /* Failed to get physical address. */
            return  gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    else
    {
        /* Invalid address. */
        return  gcvSTATUS_INVALID_ARGUMENT;
    }

    *Physical = (gctUINT32) paddr;
    return gcvSTATUS_OK;
}


static gceSTATUS
_ConvertWindowFormat(
    IN gctINT ScreenFormat,
    OUT gctINT * BitsPerPixel,
    OUT gceSURF_FORMAT * Format
    )
{
    gctUINT bpp;
    gceSURF_FORMAT fmt;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("ScreenFormat=%d.", ScreenFormat);

    switch (ScreenFormat)
    {
        case SCREEN_FORMAT_BYTE:
            bpp = 8;
            fmt = gcvSURF_L8;
            break;

        case SCREEN_FORMAT_RGBA4444:
            bpp = 16;
            fmt = gcvSURF_A4R4G4B4;
            break;

        case SCREEN_FORMAT_RGBX4444:
            bpp = 16;
            fmt = gcvSURF_X4R4G4B4;
            break;

        case SCREEN_FORMAT_RGBA5551:
            bpp = 16;
            fmt = gcvSURF_A1R5G5B5;
            break;

        case SCREEN_FORMAT_RGBX5551:
            bpp = 16;
            fmt = gcvSURF_X1R5G5B5;
            break;

        case SCREEN_FORMAT_RGB565:
            bpp = 16;
            fmt = gcvSURF_R5G6B5;
            break;

        case SCREEN_FORMAT_RGB888:
            bpp = 24;
            fmt = gcvSURF_R8G8B8;
            break;

        case SCREEN_FORMAT_RGBA8888:
            bpp = 32;
            fmt = gcvSURF_A8R8G8B8;
            break;

        case SCREEN_FORMAT_RGBX8888:
            bpp = 32;
            fmt = gcvSURF_X8R8G8B8;
            break;

        default:
            goto OnError;
    }

    if (gcvNULL != BitsPerPixel)
    {
        *BitsPerPixel = bpp;
    }

    if (gcvNULL != Format)
    {
        *Format = fmt;
    }

    gcmFOOTER_ARG("*BitsPerPixel=%d, *Format=%d.", bpp, fmt);
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

/*******************************************************************************
** Display.
*/

gceSTATUS
qnx_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    *Display = (PlatformDisplayType)1;
    return gcvSTATUS_OK;
}

gceSTATUS
qnx_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return qnx_GetDisplay(Display, Context);
}

gceSTATUS
qnx_GetDisplayInfo(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctSIZE_T * Physical,
    OUT gctINT * Stride,
    OUT gctINT * BitsPerPixel
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

typedef struct _qnxDISPLAY_INFO
{
    /* The size of the display in pixels. */
    int                         width;
    int                         height;

    /* The stride of the dispay. -1 is returned if the stride is not known
    ** for the specified display.*/
    int                         stride;

    /* The color depth of the display in bits per pixel. */
    int                         bitsPerPixel;

    /* The logical pointer to the display memory buffer. NULL is returned
    ** if the pointer is not known for the specified display. */
    void *                      logical;

    /* The physical address of the display memory buffer. ~0 is returned
    ** if the address is not known for the specified display. */
    unsigned long               physical;

    /* Can be wraped as surface. */
    int                         wrapFB;

    /* FB_MULTI_BUFFER support */
    int                         multiBuffer;
    int                         backBufferY;

    /* Tiled buffer / tile status support. */
    int                         tiledBuffer;
    int                         tileStatus;
    int                         compression;

    /* The color info of the display. */
    unsigned int                alphaLength;
    unsigned int                alphaOffset;
    unsigned int                redLength;
    unsigned int                redOffset;
    unsigned int                greenLength;
    unsigned int                greenOffset;
    unsigned int                blueLength;
    unsigned int                blueOffset;

    /* Display flip support. */
    int                         flip;
}
qnxDISPLAY_INFO;


gceSTATUS
qnx_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT qnxDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    screen_buffer_t buf[gcdDISPLAY_BACK_BUFFERS];
    gctPOINTER pointer;
    gctINT contiguous = 0;
    gctINT64 paddr;
    gctINT rc, stride;
    gctINT size[2];

    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);

    /* Valid Window? and structure size? */
    if ((Window == gcvNULL) || (DisplayInfoSize != sizeof(qnxDISPLAY_INFO)))
    {
        goto OnError;
    }

    rc = screen_get_window_property_pv((screen_window_t)Window, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)buf);
    if (rc)
    {
        goto OnError;
    }

    /* For QNX, the Width and Height are size of the framebuffer. */
    rc = screen_get_buffer_property_iv(buf[0], SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc)
    {
        goto OnError;
    }

    DisplayInfo->width  = size[0];
    DisplayInfo->height = size[1];

    /* The stride of the window. */
    rc = screen_get_buffer_property_iv(buf[0], SCREEN_PROPERTY_STRIDE, &stride);
    if (rc)
    {
        goto OnError;
    }

    DisplayInfo->stride = stride;

    /* The logical address of the window. */
    rc = screen_get_buffer_property_pv(buf[0], SCREEN_PROPERTY_POINTER, &pointer);
    if (rc)
    {
        goto OnError;
    }

    DisplayInfo->logical = pointer;

    /* Whether or not it is physically contiguous. */
    rc = screen_get_buffer_property_iv(buf[0], SCREEN_PROPERTY_PHYSICALLY_CONTIGUOUS, &contiguous);
    if (rc)
    {
        goto OnError;
    }

    if (contiguous)
    {
        /* The physical address of the window. */
        rc = screen_get_buffer_property_llv(buf[0], SCREEN_PROPERTY_PHYSICAL_ADDRESS, &paddr);
        if (rc)
        {
            goto OnError;
        }
    }
    else
    {
        /* Invalid address. */
        paddr = gcvINVALID_ADDRESS;
    }

    DisplayInfo->physical = paddr;

    /* Flip. */
    DisplayInfo->flip = 1;

    DisplayInfo->wrapFB = gcvTRUE;

    /* Success. */
    gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
qnx_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    *Offset = 0;
    *X = 0;
    *Y = 0;

    return gcvSTATUS_OK;
}

gceSTATUS
qnx_SetDisplayVirtual(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    screen_buffer_t buf[gcdDISPLAY_BACK_BUFFERS];
    int rc;
    int rects[4];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    rects[0] = 0;
    rects[1] = 0;

    rc = screen_get_window_property_pv((screen_window_t)Window, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)buf);
    if (rc)
    {
        goto OnError;
    }

    rc = screen_get_window_property_iv((screen_window_t)Window, SCREEN_PROPERTY_BUFFER_SIZE, &rects[2]);
    if (rc)
    {
        goto OnError;
    }

    rc = screen_post_window((screen_window_t)Window, buf[0], 1, rects, 0);
    if (rc)
    {
        goto OnError;
    }

    gcmFOOTER_NO();
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
qnx_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return qnx_SetDisplayVirtual(Display, Window, Offset, X, Y);
}

gceSTATUS
qnx_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gcmPRINT("%s: TODO", __func__);
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_SetSwapInterval(
    IN PlatformWindowType Window,
    IN gctINT Interval
)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_SetSwapIntervalEx(
    IN PlatformDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_GetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_DisplayBufferRegions(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    screen_buffer_t buf,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{

    int rc;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Window=0x%x NumRects=%d", Display, Window, NumRects);

    rc = screen_post_window((screen_window_t)Window, buf, NumRects, Rects, 0);
    if (rc) {
        goto OnError;
    }

    gcmFOOTER_NO();
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
qnx_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    /*
     * nothing to do because GetDefaultDisplay simply returned an integer...
     */

    return gcvSTATUS_OK;
}

/*******************************************************************************
** Windows
*/

gceSTATUS
qnx_GetWindowInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset,
    OUT gceSURF_FORMAT * Format,
    OUT gceSURF_TYPE * Type
    )
{
    gctINT rc, size[2], format;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == gcvNULL)
    {
        /* Window is not a valid window data structure pointer. */
        goto OnError;
    }

    if (X != gcvNULL)
    {
        *X = 0;
    }

    if (Y != gcvNULL)
    {
        *Y = 0;
    }

    if ((Width != gcvNULL) || (Height != gcvNULL))
    {
        rc = screen_get_window_property_iv((screen_window_t)Window, SCREEN_PROPERTY_BUFFER_SIZE, size);
        if (rc)
        {
            goto OnError;
        }

        if (Width != gcvNULL)
        {
            *Width = size[0];
        }

        if (Height != gcvNULL)
        {
            *Height = size[1];
        }
    }

    if ((BitsPerPixel != gcvNULL) || (Format != gcvNULL))
    {
        rc = screen_get_window_property_iv(Window, SCREEN_PROPERTY_FORMAT, &format);
        if (rc)
        {
            goto OnError;
        }

        gcmONERROR(_ConvertWindowFormat(format, BitsPerPixel, Format));
    }

    if (Offset != gcvNULL)
    {
        *Offset = 0;
    }

    if (Type != gcvNULL)
    {
        *Type = gcvSURF_BITMAP;
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
qnx_DrawImage(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_GetImage(
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    OUT gctINT * BitsPerPixel,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

gceSTATUS
qnx_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    gctINT rc, size[2], format;
    screen_buffer_t buf[2];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    if (Pixmap == gcvNULL)
    {
        /* Pixmap is not a valid pixmap data structure pointer. */
        goto OnError;
    }

    if ((Width != gcvNULL) || (Height != gcvNULL))
    {
        rc = screen_get_pixmap_property_iv(Pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
        if (rc)
        {
            goto OnError;
        }

        if (Width != gcvNULL)
        {
            *Width = size[0];
        }

        if (Height != gcvNULL)
        {
            *Height = size[1];
        }
    }

    if (BitsPerPixel != gcvNULL)
    {
        rc = screen_get_pixmap_property_iv(Pixmap, SCREEN_PROPERTY_FORMAT, &format);
        if (rc)
        {
            goto OnError;
        }

        gcmONERROR(_ConvertWindowFormat(format, BitsPerPixel, gcvNULL));
    }

    rc = screen_get_pixmap_property_pv(Pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void **) &buf);
    if (rc)
    {
        goto OnError;
    }

    if (Stride != NULL)
    {
        rc = screen_get_buffer_property_iv(buf[0], SCREEN_PROPERTY_STRIDE, (int *) Stride);
        if (rc)
        {
            goto OnError;
        }
    }

    if (Bits != NULL)
    {
        rc = screen_get_buffer_property_pv(buf[0], SCREEN_PROPERTY_POINTER, (void **) Bits);
        if (rc)
        {
            goto OnError;
        }
    }

    gcmFOOTER_NO();
    return status;

OnError:
    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
qnx_DrawPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
qnx_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
qnx_GetDisplayInfoEx2(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT qnxDISPLAY_INFO * DisplayInfo
    )
{
    return qnx_GetDisplayInfoEx(Display, Window, DisplayInfoSize, DisplayInfo);
}

gceSTATUS
qnx_GetDisplayBackbufferEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    return qnx_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
}

gceSTATUS
qnx_IsValidDisplay(
    IN PlatformDisplayType Display
    )
{
    if (Display != (PlatformDisplayType)0)
        return gcvSTATUS_OK;

    return gcvSTATUS_INVALID_ARGUMENT;
}

gctBOOL
qnx_SynchronousFlip(
    IN PlatformDisplayType Display
    )
{
    return gcvFALSE;
}

gceSTATUS
qnx_GetNativeVisualId(
    IN PlatformDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
qnx_GetWindowInfo(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset
    )
{
    if (gcmIS_ERROR(qnx_GetWindowInfoEx(
                          Display,
                          Window,
                          X, Y,
                          (gctINT_PTR) Width, (gctINT_PTR) Height,
                          (gctINT_PTR) BitsPerPixel,
                          (gctUINT_PTR) Offset,
                          gcvNULL,
                          gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
qnx_DrawImageEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits,
    IN gceSURF_FORMAT Format
    )
{
    return qnx_DrawImage(Display,
                           Window,
                           Left,
                           Top,
                           Right,
                           Bottom,
                           Width,
                           Height,
                           BitsPerPixel,
                           Bits);
}

gceSTATUS
qnx_SetWindowFormat(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format
    )
{
    /*
     * Possiable types:
     *   gcvSURF_BITMAP
     *   gcvSURF_RENDER_TARGET
     *   gcvSURF_RENDER_TARGET_NO_COMPRESSION
     *   gcvSURF_RENDER_TARGET_NO_TILE_STATUS
     */
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_GetPixmapInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{
    gctINT rc, format;
    gceSTATUS status = gcvSTATUS_OK;

    if (Pixmap == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(qnx_GetPixmapInfo(Display,
                                   Pixmap,
                                   (gctINT_PTR) Width,
                                   (gctINT_PTR) Height,
                                   gcvNULL,
                                   gcvNULL,
                                   gcvNULL));

    if ((BitsPerPixel != gcvNULL) || (Format != gcvNULL))
    {
        rc = screen_get_pixmap_property_iv(Pixmap, SCREEN_PROPERTY_FORMAT, &format);
        if (rc)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmONERROR(_ConvertWindowFormat(format, BitsPerPixel, Format));
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
qnx_CopyPixmapBits(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    OUT gctPOINTER DstBits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_CreateContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_DestroyContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_MakeCurrent(
    IN gctPOINTER Display,
    IN PlatformWindowType DrawDrawable,
    IN PlatformWindowType ReadDrawable,
    IN gctPOINTER Context,
    IN gcoSURF ResolveTarget
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_CreateDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_DestroyDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_RSForSwap(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctPOINTER resolve
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_SwapBuffers(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
qnx_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#include <gc_egl_precomp.h>


/*
 * Number of temorary linear 'resolve' surfaces.
 * Need alloate those surfaces when can directly resolve into window back
 * buffers.
 */
#define NUM_TEMPORARY_RESOLVE_SURFACES      1

/*
 * Make sure GPU rendering and window back buffer displaying (may be by CPU)
 * are synchronized.
 * The idea is to wait until buffer is displayed before next time return back
 * to GPU rendering.
 */
#define SYNC_TEMPORARY_RESOLVE_SURFACES     0


/******************************************************************************/
/* Display. */

static void *
_GetDefaultDisplay(
    void
    )
{
    PlatformDisplayType display = (PlatformDisplayType)0;
    qnx_GetDisplay(&display, gcvNULL);

    return gcmINT2PTR(display);
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    qnx_DestroyDisplay((PlatformDisplayType)gcmPTR2INT32(Display));
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (gcmIS_ERROR(qnx_IsValidDisplay((PlatformDisplayType)gcmPTR2INT32(Display))))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = qnx_InitLocalDisplayInfo((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                        &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = qnx_DeinitLocalDisplayInfo((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                          &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint visualId = 0;

    qnx_GetNativeVisualId((PlatformDisplayType)gcmPTR2INT32(Display->hdc), &visualId);
    return visualId;
}

/* Query of swap interval range. */
static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Min,
    OUT EGLint * Max
    )
{
    gceSTATUS status;

    /* Get swap interval from HAL OS layer. */
    status = qnx_GetSwapInterval((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                   Min, Max);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    gceSTATUS status;

    status = qnx_SetSwapInterval((PlatformWindowType)Surface->hwnd, Interval);

    if (status == gcvSTATUS_NOT_SUPPORTED)
    {
        /*
         * return true to maintain legacy behavior. If the feature is not there
         * we were ignoring it. And now we are ignoring it too.
         */
        return EGL_TRUE;
    }
    else if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */

typedef struct eglNativeBuffer * VEGLNativeBuffer;

struct eglNativeBuffer
{
    gctPOINTER          context;
    gcsPOINT            origin;
    gcoSURF             surface;

    /* QNX only. */
    gctPOINTER          logical;

    /* Buffer lock. */
    gctSIGNAL           lock;

    VEGLNativeBuffer    prev;
    VEGLNativeBuffer    next;
};

struct eglWindowInfo
{
    /*
     * Can directly access window memory?
     * True  for FBDEV, DFB, QNX, DDraw, etc.
     * False for GDI, X11, DRI, etc.
     *
     * As descripted in comments in 'bindWindow' in header file, 4 conditions
     * for window back buffer:
     * If 'fbDirect' window memory: ('fbDirect' = 'True')
     *   1. Direct window back buffer surface ('wrapFB' = 'False')
     *   2. Wrapped surface ('wrapFB' = 'True')
     *   3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
     * Else:
     *   4. Temporary surface. ('fbDirect' = 'False')
     */
    EGLBoolean          fbDirect;

    /*
     * Wrap window back buffer as HAL surface object?
     * Invalid if 'fbDirect' is 'False'.
     */
    EGLBoolean          wrapFB;

    /* Window back buffer list, wrappers or temporary surface objects. */
    VEGLNativeBuffer    bufferList;
    gctPOINTER          bufferListMutex;

    /* Information obtained by qnx_GetDisplayInfoEx2. */
    gctPOINTER          logical;
    unsigned long       physical;
    gctINT              stride;
    gctUINT             width;
    gctUINT             height;
    gceSURF_FORMAT      format;
    gceSURF_TYPE        type;
    gctINT              bitsPerPixel;
    gctUINT             xresVirtual;
    gctUINT             yresVirtual;
    gctUINT             multiBuffer;
};



/*
 * Create wrappers or temporary surface object(s) for native window (conditions
 * 2, 3 and 4 mentioned above).
 *
 * o 2. Wrapped surface ('wrapFB' = 'True')
 * o 3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
 * o 4. Temporary surface. ('fbDirect' = 'False')
 */
static gceSTATUS
_CreateWindowBuffers(
    void * Window,
    VEGLWindowInfo Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VEGLNativeBuffer buffer;
    screen_buffer_t *bufs;
    int num_of_buffers = 0, i, rc;
    gctPOINTER pointer;

    /* Valid Window?  */
    if (Window == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    /* Get number of buffers assigned to the window */
    rc = screen_get_window_property_iv((screen_window_t)Window, SCREEN_PROPERTY_BUFFER_COUNT, &num_of_buffers);
    if (rc || (num_of_buffers < 1))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Allocate memory for all render buffers */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
            num_of_buffers * sizeof(screen_buffer_t),
            &pointer));

    gcoOS_ZeroMemory(pointer, num_of_buffers * sizeof(screen_buffer_t));
    bufs = pointer;

    rc = screen_get_window_property_pv((screen_window_t)Window, SCREEN_PROPERTY_BUFFERS, (void**)bufs);
    if (rc)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    for(i = 0; i < num_of_buffers; i++) {
        gctUINT32 physical;

        /* Allocate native buffer object. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                sizeof (struct eglNativeBuffer),
                &pointer));

        gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
        buffer = pointer;

        if(gcmIS_ERROR(qnx_GetScreenWindowBufferAddress(bufs[i], &pointer, &physical))) {
            /* Failed to get address of the physical buffer. */
            gcmOS_SAFE_FREE(gcvNULL, buffer);
            goto OnError;
        }

        /* Create wrapper or temporary surface object. */
        status = gcoSURF_Construct(gcvNULL,
                                   Info->width,
                                   Info->height, 1,
                                   gcvSURF_BITMAP,
                                   Info->format,
                                   gcvPOOL_USER,
                                   &buffer->surface);

        if (gcmIS_ERROR(status))
        {
            /* Failed to construct wrapper. */
            gcmOS_SAFE_FREE(gcvNULL, buffer);
            goto OnError;
        }

        /* Map the memory. */
        status = gcoSURF_MapUserSurface(buffer->surface, Info->stride, pointer, physical);

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(gcoSURF_Destroy(buffer->surface));
            gcmOS_SAFE_FREE(gcvNULL, buffer);
            goto OnError;
        }

        /* New buffer. */
        buffer->logical  = pointer;
        buffer->context  = bufs[i];
        buffer->origin.x = 0;
        buffer->origin.y = 0;

#if SYNC_TEMPORARY_RESOLVE_SURFACES
        /* Create the buffer lock. */
        gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &buffer->lock));

        /* Set initial 'unlocked' state. */
        gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
#endif

        /* Add into buffer list. */
        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);
        if (Info->bufferList)
        {
            VEGLNativeBuffer prev = Info->bufferList->prev;

            buffer->prev = prev;
            buffer->next = Info->bufferList;

            prev->next = buffer;
            Info->bufferList->prev = buffer;
        }
        else
        {
            buffer->prev = buffer->next = buffer;
            Info->bufferList = buffer;
        }
        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);

    }

    return status;

OnError:
    /* Error roll back. */
    if ((buffer = Info->bufferList) != gcvNULL)
    {
        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;
    }

    /* The buffer list mutex must be acquired. */
    gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return status;
}

static void
_FreeWindowBuffers(
    VEGLSurface Surface,
    void * Window,
    VEGLWindowInfo Info
    )
{
    if (Info->bufferList)
    {
        VEGLNativeBuffer buffer;

        /* Make sure all workers have been processed. */
        if (Surface->workerDoneSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                          Surface->workerDoneSignal,
                                          gcvINFINITE));
        }

        /* Lock buffers. */
        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        /* Go through all buffers. */
        buffer = Info->bufferList;

        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;

        /* Unlock. */
        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
}

static
EGLBoolean
_QueryWindowInfo(
    IN VEGLDisplay Display,
    IN void * Window,
    IN VEGLWindowInfo Info
    )
{
    gceSTATUS status;
    qnxDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = qnx_GetWindowInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                   (PlatformWindowType) Window,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   &bitsPerPixel,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Initialize window geometry info. */
    Info->width        = width;
    Info->height       = height;
    Info->format       = format;
    Info->type         = type;
    Info->bitsPerPixel = bitsPerPixel;

    /* Get display information. */
    gcoOS_ZeroMemory(&dInfo, sizeof (qnxDISPLAY_INFO));

    status = qnx_GetDisplayInfoEx2((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                     (PlatformWindowType) Window,
                                     Display->localInfo,
                                     sizeof (qnxDISPLAY_INFO),
                                     &dInfo);

    if (gcmIS_ERROR(status))
    {
        Info->fbDirect     = EGL_FALSE;
        Info->logical      = gcvNULL;
        Info->physical     = gcvINVALID_ADDRESS;
        Info->stride       = 0;
        Info->wrapFB       = gcvFALSE;
        Info->multiBuffer  = 1;
    }
    else
    {
        Info->fbDirect     = EGL_TRUE;
        Info->logical      = dInfo.logical;
        Info->physical     = dInfo.physical;
        Info->stride       = dInfo.stride;
        Info->wrapFB       = dInfo.wrapFB;
        Info->multiBuffer  = 1;
    }

    /* Get virtual size. */
    status = qnx_GetDisplayVirtual((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                     (gctINT_PTR) &Info->xresVirtual,
                                     (gctINT_PTR) &Info->yresVirtual);

    if (gcmIS_ERROR(status))
    {
        Info->xresVirtual = Info->width;
        Info->yresVirtual = Info->height;

        if (Info->multiBuffer > 1)
        {
            Info->multiBuffer = 1;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean
_ConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN void * Window
    )
{
    gceSTATUS status;
    VEGLWindowInfo info = gcvNULL;
    void * win = Window;
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != gcvNULL);
    gcmASSERT(Surface->winInfo == gcvNULL);

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglWindowInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglWindowInfo));
    info = pointer;

    /* Query window information. */
    if (_QueryWindowInfo(Display, Window, info) == EGL_FALSE)
    {
        /* Bad native window. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create buffer mutex. */
    gcmONERROR(gcoOS_CreateMutex(gcvNULL, &info->bufferListMutex));

    /* Create window drawable? */
    qnx_CreateDrawable(Display->localInfo, (PlatformWindowType)win);

    /* Create window buffers to represent window native bufers. */
    gcmONERROR(_CreateWindowBuffers(Window, info));

    /* Save window info structure. */
    Surface->winInfo = info;
    return EGL_TRUE;

OnError:
    if (info)
    {
        if (info->bufferListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
            info->bufferListMutex = gcvNULL;
        }

        gcmOS_SAFE_FREE(gcvNULL, info);
        Surface->winInfo = gcvNULL;
    }

    return EGL_FALSE;
}

static EGLBoolean
_DisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Free native window buffers. */
    _FreeWindowBuffers(Surface, win, info);

    /* Delete the mutex. */
    gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
    info->bufferListMutex = gcvNULL;

    qnx_DestroyDrawable(Display->localInfo, (PlatformWindowType)win);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, info);
    return EGL_TRUE;
}

static EGLBoolean
_BindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    )
{
    gceSTATUS status;

    /* Get shortcut. */
    void * win  = Surface->hwnd;
    VEGLWindowInfo info   = Surface->winInfo;
    /* Indirect rendering by default. */
    EGLint renderMode     = VEGL_INDIRECT_RENDERING;
    EGLBoolean winChanged = EGL_FALSE;
    gctINT width          = 0;
    gctINT height         = 0;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceSURF_TYPE type     = gcvSURF_UNKNOWN;

    status = qnx_GetWindowInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                   (PlatformWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Check window resize. */
    if ((info->width  != (gctUINT)width) ||
        (info->height != (gctUINT)height) ||
        (info->format != format))
    {
        /* Native window internally changed. */
        winChanged = EGL_TRUE;
    }

    if (Surface->openVG)
    {
        if ((type != gcvSURF_BITMAP) ||
            (info->type != gcvSURF_BITMAP))
        {
            /* Request linear buffer for hardware OpenVG. */
            status = qnx_SetWindowFormat((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            /* Recreate window buffers. */
            _FreeWindowBuffers(Surface, win, info);
            gcmONERROR(_CreateWindowBuffers(win, info));
        }
    }
    else
    {
#if gcdENABLE_3D
        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            ((type != gcvSURF_BITMAP) || (info->type != gcvSURF_BITMAP)))
        {
            /* Only linear supported in this case. */
            status = qnx_SetWindowFormat((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            if ((renderMode == VEGL_DIRECT_RENDERING_FCFILL) &&
                (info->type == gcvSURF_RENDER_TARGET_NO_TILE_STATUS))
            {
                /* Special for FC-FILL mode: tile status is required. */
                info->type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
            }

            /* Recreate window buffers. */
            _FreeWindowBuffers(Surface, win, info);
            gcmONERROR(_CreateWindowBuffers(win, info));
        }


#endif
    }

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): winChanged=%d format=%d type=%x EGLConfig=%d%d%d%d "
             " renderMode=%d",
             __FUNCTION__, __LINE__,
             winChanged,
             info->format,
             info->type,
             Surface->config.redSize,
             Surface->config.greenSize,
             Surface->config.blueSize,
             Surface->config.alphaSize,
             renderMode);

    *RenderMode = renderMode;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_GetWindowSize(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
    gceSTATUS status;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE   type;

    /* Get shortcut. */
    void * win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

    status = qnx_GetWindowInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                   (PlatformWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    (void) format;
    (void) type;

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    *Width  = width;
    *Height = height;

    return EGL_TRUE;
}

static EGLBoolean
_GetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    gceSTATUS status;
    VEGLWindowInfo info  = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Return the surface object. */
    VEGLNativeBuffer buffer;

    gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);


    buffer = info->bufferList;
    BackBuffer->surface  = buffer->surface;
    BackBuffer->context  = buffer;
    BackBuffer->origin.x = 0;
    BackBuffer->origin.y = 0;
    BackBuffer->flip     = gcvTRUE;

    info->bufferList = buffer->next;
    gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

    if (buffer->lock != gcvNULL)
    {

        /* Wait for buffer lock. */
        for (;;)
        {
            status = gcoOS_WaitSignal(gcvNULL, buffer->lock, 5000);

            if (status == gcvSTATUS_TIMEOUT)
            {
                gcmPRINT("Wait for buffer lock timeout");
                continue;
            }

            break;
        }

        /*
         * Set the buffer to 'locked' state.
         * It will be 'unlocked' when buffer posted to display.
         * This can make sure next time GetWindowBackBuffer, the buffer
         * is 'posted' before returns for GPU rendering.
         */
        gcoOS_Signal(gcvNULL, buffer->lock, gcvFALSE);
    }
    return EGL_TRUE;
}

static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN struct eglRegion * Region,
    IN struct eglRegion * DamageHint
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    (void) surface;

    if (info->fbDirect)
    {
        screen_buffer_t screen_buf;
        VEGLNativeBuffer buffer;

        buffer = (VEGLNativeBuffer) BackBuffer->context;
        screen_buf = (screen_buffer_t)buffer->context;
        status = qnx_DisplayBufferRegions((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                            (PlatformWindowType) win,
                                            screen_buf,
                                            Region->numRects,
                                            Region->rects);

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }

        if (buffer->lock != gcvNULL)
        {
            /* The buffer is now posted. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
        }
    }
    else
    {
        VEGLNativeBuffer buffer;
        gctINT alignedWidth, alignedHeight;
        gceORIENTATION orientation;
        gceSURF_FORMAT format = gcvSURF_UNKNOWN;
        gcsSURF_FORMAT_INFO_PTR formatInfo;
        gctPOINTER memory[3] = {gcvNULL};
        gctINT i;

        /* Cast type. */
        buffer = (VEGLNativeBuffer) BackBuffer->context;

        /* Get aligned size. */
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(BackBuffer->surface,
                                            (gctUINT_PTR) &alignedWidth,
                                            (gctUINT_PTR) &alignedHeight,
                                            gcvNULL));

        gcmVERIFY_OK(gcoSURF_QueryOrientation(BackBuffer->surface, &orientation));

        if (orientation == gcvORIENTATION_BOTTOM_TOP)
        {
            alignedHeight = -alignedHeight;
        }

        /* Gather source information. */
        gcmVERIFY_OK(gcoSURF_GetFormat(BackBuffer->surface,
                                       gcvNULL,
                                       &format));

        /* Query format. */
        if (gcoSURF_QueryFormat(format, &formatInfo))
        {
            return EGL_FALSE;
        }

        /* Lock surface for memory. */
        if (gcoSURF_Lock(BackBuffer->surface, gcvNULL, memory))
        {
            return EGL_FALSE;
        }

        for (i = 0; i < Region->numRects; i++)
        {
            EGLint left   = Region->rects[i * 4 + 0];
            EGLint top    = Region->rects[i * 4 + 1];
            EGLint width  = Region->rects[i * 4 + 2];
            EGLint height = Region->rects[i * 4 + 3];

            /* Draw image. */
            status = qnx_DrawImageEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                       (PlatformWindowType) win,
                                       left, top, left + width, top + height,
                                       alignedWidth, alignedHeight,
                                       formatInfo->bitsPerPixel,
                                       memory[0],
                                       format);

            if (gcmIS_ERROR(status))
            {
                break;
            }
        }

        /* Unlock the surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(BackBuffer->surface, memory[0]));

        if (buffer->lock != gcvNULL)
        {
            /* The buffer is now posted. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
        }

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

    status = qnx_CancelDisplayBackbuffer((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                           (PlatformWindowType) win,
                                           BackBuffer->context,
                                           surface,
                                           0,
                                           BackBuffer->origin.x,
                                           BackBuffer->origin.y);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return qnx_SynchronousFlip((PlatformDisplayType)gcmPTR2INT32(Display->hdc));
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_QueryBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    OUT EGLint *BufferAge
    )
{
    return EGL_FALSE;
}

/******************************************************************************/
/* Pixmap. */

struct eglPixmapInfo
{
    /* Native pixmap geometry info in Vivante HAL. */
    gctINT              width;
    gctINT              height;
    gceSURF_FORMAT      format;
    gctINT              stride;
    gctINT              bitsPerPixel;

    /* Pixmap memory. */
    gctUINT8_PTR        data;

    /* Reference native display. */
    void *              hdc;

    /* Pixmap wrapper. */
    gcoSURF             wrapper;

    /* Shadow surface, exists when the wrapper is not resovable. */
    gcoSURF             shadow;
};

static void
_DoSyncFromPixmap(
    void * Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(memory[0], Info->data, stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) Info->data;
            gctUINT8_PTR dest   = (gctUINT8_PTR) memory[0];
            gctINT shadowStride = stride;

            /* Get min stride. */
            stride = gcmMIN(shadowStride, Info->stride);

            for (y = 0; y < Info->height; y++)
            {
                /* Copy a scanline. */
                gcoOS_MemCopy(dest, source, stride);

                /* Advance to next line. */
                source += Info->stride;
                dest   += shadowStride;
            }
        }
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(qnx_CopyPixmapBits((PlatformDisplayType)gcmPTR2INT32(Info->hdc),
                                        (PlatformPixmapType) Pixmap,
                                        width, height,
                                        stride,
                                        Info->format,
                                        memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    /* Unlock. */
    if (memory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

static void
_DoSyncToPixmap(
    void * Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data != gcvNULL)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(Info->data, memory[0], stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) memory[0];
            gctUINT8_PTR dest   = (gctUINT8_PTR) Info->data;
            gctINT shadowStride = stride;

            /* Get min stride. */
            stride = gcmMIN(shadowStride, Info->stride);

            for (y = 0; y < Info->height; y++)
            {
                /* Copy a scanline. */
                gcoOS_MemCopy(dest, source, stride);

                /* Advance to next line. */
                source += shadowStride;
                dest   += Info->stride;
            }
        }
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(qnx_DrawPixmap((PlatformDisplayType)gcmPTR2INT32(Info->hdc),
                                    (PlatformPixmapType) Pixmap,
                                    0, 0,
                                    Info->width,
                                    Info->height,
                                    width,
                                    height,
                                    Info->bitsPerPixel,
                                    memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    if (memory[0] != gcvNULL)
    {
        /* Unlock. */
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

static EGLBoolean
_MatchPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN struct eglConfig * Config
    )
{
    gceSTATUS status;
    gctINT width, height, bitsPerPixel;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = qnx_GetPixmapInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                   (PlatformPixmapType) Pixmap,
                                   &width,
                                   &height,
                                   &bitsPerPixel,
                                   gcvNULL, gcvNULL, &pixmapFormat);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    /* Check if format is matched. */
    switch (pixmapFormat)
    {
    case gcvSURF_R5G6B5:
        if ((Config->redSize   != 5)
        ||  (Config->greenSize != 6)
        ||  (Config->blueSize  != 5))
        {
            match = EGL_FALSE;
        }
        break;

    case gcvSURF_X8R8G8B8:
        if ((Config->redSize   != 8)
        ||  (Config->greenSize != 8)
        ||  (Config->blueSize  != 8)
        ||  (Config->alphaSize != 0))
        {
            match = EGL_FALSE;
        }
        break;

    default:
        break;
    }

    return match;
}

static EGLBoolean
_ConnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL needShadow = gcvFALSE;
    gctINT pixmapWidth;
    gctINT pixmapHeight;
    gctINT pixmapStride = 0;
    gceSURF_FORMAT pixmapFormat;
    gctINT pixmapBpp;
    gctPOINTER pixmapBits = gcvNULL;
    gctUINT32 pixmapPhysical = gcvINVALID_ADDRESS;
    gcoSURF wrapper = gcvNULL;
    gcoSURF shadow = gcvNULL;
    gctPOINTER pointer;
    VEGLPixmapInfo info = gcvNULL;

    /* Query pixmap geometry info. */
    gcmONERROR(qnx_GetPixmapInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = qnx_GetPixmapInfo((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                                 (PlatformPixmapType) Pixmap,
                                 gcvNULL,
                                 gcvNULL,
                                 gcvNULL,
                                 &pixmapStride,
                                 &pixmapBits);

    do
    {
        if (gcmIS_ERROR(status) || !pixmapBits)
        {
            /* Can not wrap as surface object. */
            needShadow = gcvTRUE;
            break;
        }

        if (((gctUINTPTR_T) pixmapBits) & 0x3F)
        {
            needShadow = gcvTRUE;
            break;
        }

        if ((pixmapStride * 8 / pixmapBpp) < 16)
        {
            /* Too small in width. */
            needShadow = gcvTRUE;
            break;
        }


        /* Height needs to be 4 aligned or vstride is large enough. */
        if (pixmapHeight & 3)
        {
            /*
             * Not enough memory in height.
             * Resolve may exceeds the buffer and overwrite other memory.
             */
            needShadow = gcvTRUE;
            break;
        }

        /* Check pixmap format. */
        switch (pixmapFormat)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_X8R8G8B8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_R5G6B5:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4R4G4B4:
        case gcvSURF_X4B4G4R4:
            break;

        default:
            /* Resolve can not support such format. */
            return EGL_FALSE;
        }
    }
    while (gcvFALSE);

    do
    {
        if (needShadow)
        {
            /* No pixmap wrapper. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Construct pixmap wrapper. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmapWidth,
                              pixmapHeight,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_USER,
                              &wrapper));

        /* Set pixels. */
        status = gcoSURF_SetBuffer(wrapper,
                                   gcvSURF_BITMAP,
                                   pixmapFormat,
                                   pixmapStride,
                                   pixmapBits,
                                   pixmapPhysical);

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Do the wrap. */
        status = gcoSURF_SetWindow(wrapper,
                                   0, 0,
                                   pixmapWidth,
                                   pixmapHeight);
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status) && (wrapper != gcvNULL))
    {
        /* Failed to wrap as surface object. */
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
        wrapper = gcvFALSE;

        /* Shadow required and format must be supported. */
        needShadow = gcvTRUE;
    }

    if (needShadow)
    {
        /* Construct the shadow surface. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmapWidth,
                              pixmapHeight,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_DEFAULT,
                              &shadow));
    }

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglPixmapInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglPixmapInfo));
    info = pointer;

    /* Save pixmap info. */
    info->width        = pixmapWidth;
    info->height       = pixmapHeight;
    info->format       = pixmapFormat;
    info->stride       = pixmapStride;
    info->bitsPerPixel = pixmapBpp;
    info->data         = pixmapBits;
    info->hdc          = Display->hdc;
    info->wrapper      = wrapper;
    info->shadow       = shadow;

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p wrapper=%p shadow=%p",
             __FUNCTION__, __LINE__, Display, Pixmap, wrapper, shadow);

    /* Output. */
    *Info    = info;
    *Surface = (shadow != gcvNULL) ? shadow : wrapper;

    return EGL_TRUE;

OnError:
    if (wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
    }

    if (shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(shadow));
    }

    if (info != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, info);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return EGL_FALSE;
}

static EGLBoolean
_DisconnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    /* Free pixmap wrapper. */
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p",
             __FUNCTION__, __LINE__, Display, Pixmap);

    if (Info->wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->wrapper));
        Info->wrapper = gcvNULL;
    }

    if (Info->shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->shadow));
        Info->shadow = gcvNULL;
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, Info);
    return EGL_TRUE;
}

static EGLBoolean
_GetPixmapSize(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
    gceSTATUS status;
    gctINT bitsPerPixel;
    gceSURF_FORMAT format;
    gctINT width, height;

    /* Query pixmap info again. */
    gcmONERROR(
        qnx_GetPixmapInfoEx((PlatformDisplayType)gcmPTR2INT32(Display->hdc),
                              (PlatformPixmapType) Pixmap,
                              &width,
                              &height,
                              &bitsPerPixel,
                              gcvNULL,
                              gcvNULL,
                              &format));

    (void) bitsPerPixel;
    (void) format;

    gcmASSERT(width  == Info->width);
    gcmASSERT(height == Info->height);

    *Width  = width;
    *Height = height;

    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
_SyncFromPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncFromPixmap(Pixmap, Info);
    }
    else
    {
        gcmVERIFY_OK(
            gcoSURF_CPUCacheOperation(Info->wrapper, gcvCACHE_FLUSH));
    }

    return EGL_TRUE;
}

static EGLBoolean
_SyncToPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncToPixmap(Pixmap, Info);
    }

    return EGL_TRUE;
}


static struct eglPlatform qnxPlatform =
{
    EGL_PLATFORM_QNX_VIV,

    _GetDefaultDisplay,
    _ReleaseDefaultDisplay,
    _IsValidDisplay,
    _InitLocalDisplayInfo,
    _DeinitLocalDisplayInfo,
    _GetNativeVisualId,
    _GetSwapInterval,
    _SetSwapInterval,
    _ConnectWindow,
    _DisconnectWindow,
    _BindWindow,
    _UnbindWindow,
    _GetWindowSize,
    _GetWindowBackBuffer,
    _PostWindowBackBuffer,
    gcvNULL,
    _CancelWindowBackBuffer,
    _SynchronousPost,
    gcvNULL,
    _UpdateBufferAge,
    _QueryBufferAge,
    _MatchPixmap,
    _ConnectPixmap,
    _DisconnectPixmap,
    _GetPixmapSize,
    _SyncFromPixmap,
    _SyncToPixmap,
};

VEGLPlatform
veglGetQnxPlatform(
    void * NativeDisplay
    )
{
    return &qnxPlatform;
}


