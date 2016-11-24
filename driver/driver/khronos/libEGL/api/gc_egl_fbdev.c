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


/* Enable sigaction declarations. */
#if !defined _XOPEN_SOURCE
#   define _XOPEN_SOURCE 501
#endif

#include "gc_egl_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define gcdUSE_PIXMAP_SURFACE 1


#define _GC_OBJ_ZONE    gcvZONE_OS

#define GC_FB_MAX_SWAP_INTERVAL     10
#define GC_FB_MIN_SWAP_INTERVAL     0


typedef struct _FBDisplay * PlatformDisplayType;
typedef struct _FBWindow *  PlatformWindowType;
typedef struct _FBPixmap *  PlatformPixmapType;

/* Structure that defines a display. */
struct _FBDisplay
{
    gctINT                  index;
    gctINT                  file;
    gctSIZE_T               physical;
    gctINT                  stride;
    gctINT                  width;
    gctINT                  height;
    gctINT                  alignedHeight;
    gctINT                  bpp;
    gctINT                  size;
    gctPOINTER              memory;
    struct fb_fix_screeninfo    fixInfo;
    struct fb_var_screeninfo    varInfo;
    struct fb_var_screeninfo    orgVarInfo;
    gctINT                  backBufferY;
    gctINT                  multiBuffer;
    pthread_cond_t          cond;
    pthread_mutex_t         condMutex;
    gctUINT                 alphaLength;
    gctUINT                 alphaOffset;
    gctUINT                 redLength;
    gctUINT                 redOffset;
    gctUINT                 greenLength;
    gctUINT                 greenOffset;
    gctUINT                 blueLength;
    gctUINT                 blueOffset;
    gceSURF_FORMAT          format;
    gceTILING               tiling;
    gctINT                  swapInterval;

    struct _FBDisplay *     next;
};

/* Structure that defines a window. */
struct _FBWindow
{
    struct _FBDisplay*      display;
    gctUINT    offset;
    gctINT             x, y;
    gctINT             width;
    gctINT             height;
    /* Color format. */
    gceSURF_FORMAT      format;
};

/* Structure that defines a pixmap. */
struct _FBPixmap
{
    /* Pointer to memory bits. */
    gctPOINTER       original;
    gctPOINTER       bits;

    /* Bits per pixel. */
    gctINT         bpp;

    /* Size. */
    gctINT         width, height;
    gctINT         alignedWidth, alignedHeight;
    gctINT         stride;

#if gcdUSE_PIXMAP_SURFACE
    gcoSURF         surface;
    gctUINT32       gpu;
#endif
};

static struct _FBDisplay *displayStack = gcvNULL;
static pthread_mutex_t displayMutex;

static void
destroyDisplays(
    void
    )
{
    pthread_mutex_lock(&displayMutex);

    while (displayStack != NULL)
    {
        struct _FBDisplay* display = displayStack;
        displayStack = display->next;

        if (display->memory != NULL)
        {
            munmap(display->memory, display->size);
            display->memory = NULL;
        }

        if (display->file >= 0)
        {
            ioctl(display->file, FBIOPUT_VSCREENINFO, &display->orgVarInfo);

            close(display->file);
            display->file = -1;
        }

        /*
        pthread_mutex_destroy(&(display->condMutex));
        pthread_cond_destroy(&(display->cond));
         */
        free(display);
    }

    pthread_mutex_unlock(&displayMutex);
}

static void sig_handler(gctINT signo)
{
    static int hookSEGV = 0;

    if (hookSEGV == 0)
    {
        signal(SIGSEGV, sig_handler);
        hookSEGV = 1;
    }

    destroyDisplays();

    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    raise(signo);
}

static void
halOnExit(
    void
    )
{
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    destroyDisplays();
}

static pthread_once_t onceControl = PTHREAD_ONCE_INIT;

static void
onceInit(
    void
    )
{
    pthread_mutexattr_t mta;

    /* Register atexit callback. */
    atexit(halOnExit);

    /* Register signal handler. */
    signal(SIGINT,  sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Init mutex attribute. */
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    /* Set display mutex as recursive. */
    pthread_mutex_init(&displayMutex, &mta);
}

/*******************************************************************************
** Display. ********************************************************************
*/

gceSTATUS
fbdev_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    char *dev, *p;
    int i;
    char fbDevName[256];
    struct _FBDisplay* display = NULL;
    gceSTATUS status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmHEADER();

    /* Init global variables. */
    pthread_once(&onceControl, onceInit);

    /* Lock display stack. */
    pthread_mutex_lock(&displayMutex);

    do
    {
        if (DisplayIndex < 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        for (display = displayStack; display != NULL; display = display->next)
        {
            if (display->index == DisplayIndex)
            {
                /* Find display.*/
                break;
            }
        }

        if (display != NULL)
        {
            *Display = (PlatformDisplayType)display;
            pthread_mutex_unlock(&displayMutex);

            gcmFOOTER_ARG("*Display=0x%x", *Display);
            return gcvSTATUS_OK;
        }

        display = (struct _FBDisplay*) malloc(sizeof(struct _FBDisplay));

        if (display == gcvNULL)
        {
            break;
        }

        display->index    = DisplayIndex;
        display->memory   = gcvNULL;
        display->file     = -1;
        display->tiling   = gcvLINEAR;

        p = getenv("FB_MULTI_BUFFER");
        if (p == NULL)
        {
            display->multiBuffer = 1;
        }
        else
        {
            display->multiBuffer = atoi(p);
            if (display->multiBuffer < 1)
            {
                display->multiBuffer = 1;
            }
        }

        sprintf(fbDevName,"FB_FRAMEBUFFER_%d",DisplayIndex);
        dev = getenv(fbDevName);

        if (dev != gcvNULL)
        {
            display->file = open(dev, O_RDWR);
        }

        if (display->file < 0)
        {
            unsigned char i = 0;
            char const * const fbDevicePath[] =
            {
                "/dev/fb%d",
                "/dev/graphics/fb%d",
                gcvNULL
            };

            /* Create a handle to the device. */
            while ((display->file == -1) && fbDevicePath[i])
            {
                sprintf(fbDevName, fbDevicePath[i], DisplayIndex);
                display->file = open(fbDevName, O_RDWR);
                i++;
            }
        }

        if (display->file < 0)
        {
            break;
        }

        /* Get variable framebuffer information. */
        if (ioctl(display->file, FBIOGET_VSCREENINFO, &display->varInfo) < 0)
        {
            break;
        }

        display->orgVarInfo = display->varInfo;

        /* Default aligned height, equals to y resolution. */
        display->alignedHeight = display->varInfo.yres;

        for (i = display->multiBuffer; i >= 1; --i)
        {
            /* Try setting up multi buffering. */
            display->varInfo.yres_virtual = display->alignedHeight * i;

            if (ioctl(display->file, FBIOPUT_VSCREENINFO, &display->varInfo) >= 0)
            {
                break;
            }
        }

        /* Get current virtual screen size. */
        if (ioctl(display->file, FBIOGET_VSCREENINFO, &(display->varInfo)) < 0)
        {
            break;
        }

        if (ioctl(display->file, FBIOGET_FSCREENINFO, &display->fixInfo) < 0)
        {
            break;
        }

        display->physical = display->fixInfo.smem_start;
        display->stride   = display->fixInfo.line_length;
        display->size     = display->fixInfo.smem_len;
        display->width  = display->varInfo.xres;
        display->height = display->varInfo.yres;
        display->bpp    = display->varInfo.bits_per_pixel;

        if (display->multiBuffer > 1)
        {
            /* Calculate actual buffer count. */
            display->multiBuffer = display->varInfo.yres_virtual
                                 / display->alignedHeight;
        }

        /* Compute aligned height of one backBuffer. */
        display->alignedHeight = display->varInfo.yres_virtual
                               / display->multiBuffer;

        /* Move to off-screen memory. */
        if (display->varInfo.yoffset % display->alignedHeight == 0)
        {
            display->backBufferY = display->varInfo.yoffset
                                 + display->alignedHeight;
        }
        else
        {
            display->varInfo.yoffset = 0;
            display->backBufferY = display->alignedHeight;
        }

        if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
        {
            /* Warp around. */
            display->backBufferY = 0;
        }

        /* Get the color format. */
        switch (display->varInfo.green.length)
        {
        case 4:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X4B4G4R4 : gcvSURF_A4B4G4R4;
            }
            break;

        case 5:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X1B5G5R5 : gcvSURF_A1B5G5R5;
            }
            break;

        case 6:
            display->format = gcvSURF_R5G6B5;
            break;

        case 8:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X8B8G8R8 : gcvSURF_A8B8G8R8;
            }
            break;

        default:
            display->format = gcvSURF_UNKNOWN;
            break;
        }

        if (display->format == gcvSURF_UNKNOWN)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Get the color info. */
        display->alphaLength = display->varInfo.transp.length;
        display->alphaOffset = display->varInfo.transp.offset;

        display->redLength   = display->varInfo.red.length;
        display->redOffset   = display->varInfo.red.offset;

        display->greenLength = display->varInfo.green.length;
        display->greenOffset = display->varInfo.green.offset;

        display->blueLength  = display->varInfo.blue.length;
        display->blueOffset  = display->varInfo.blue.offset;

        /* initialize swap interval, default value is 1 */
        display->swapInterval = 1;

        display->memory = mmap(0,
                               display->size,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               display->file,
                               0);

        if (display->memory == MAP_FAILED)
        {
            break;
        }

        pthread_cond_init(&(display->cond), gcvNULL);
        pthread_mutex_init(&(display->condMutex), gcvNULL);
        *Display = (PlatformDisplayType)display;

        /* Add display into stack. */
        display->next = displayStack;
        displayStack = display;
        pthread_mutex_unlock(&displayMutex);

        gcmFOOTER_ARG("*Display=0x%x", *Display);
        return gcvSTATUS_OK;
    }
    while (0);

    pthread_mutex_unlock(&displayMutex);

    if (display != gcvNULL)
    {
        if (display->memory != gcvNULL)
        {
            munmap(display->memory, display->size);
        }

        if (display->file >= 0)
        {
            ioctl(display->file, FBIOPUT_VSCREENINFO, &(display->orgVarInfo));
            close(display->file);
        }

        free(display);
        display = gcvNULL;
    }

    *Display = gcvNULL;
    gcmFOOTER();
    return status;
}

gceSTATUS
fbdev_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return fbdev_GetDisplayByIndex(0, Display, Context);
}

gceSTATUS
fbdev_GetDisplayInfo(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctSIZE_T * Physical,
    OUT gctINT * Stride,
    OUT gctINT * BitsPerPixel
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x", Display);
    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Width != gcvNULL)
    {
        *Width = display->width;
    }

    if (Height != gcvNULL)
    {
        *Height = display->height;
    }

    if (Physical != gcvNULL)
    {
        *Physical = display->physical;
    }

    if (Stride != gcvNULL)
    {
        *Stride = display->stride;
    }

    if (BitsPerPixel != gcvNULL)
    {
        *BitsPerPixel = display->bpp;
    }

    gcmFOOTER_NO();
    return status;
}

typedef struct _fbdevDISPLAY_INFO
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
fbdevDISPLAY_INFO;

gceSTATUS
fbdev_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT fbdevDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);
    display = (struct _FBDisplay*)Display;
    /* Valid display? and structure size? */
    if ((display == gcvNULL) || (DisplayInfoSize != sizeof(fbdevDISPLAY_INFO)))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    /* Return the size of the display. */
    DisplayInfo->width  = display->width;
    DisplayInfo->height = display->height;

    /* Return the stride of the display. */
    DisplayInfo->stride = display->stride;

    /* Return the color depth of the display. */
    DisplayInfo->bitsPerPixel = display->bpp;

    /* Return the logical pointer to the display. */
    DisplayInfo->logical = display->memory;

    /* Return the physical address of the display. */
    DisplayInfo->physical = display->physical;

    /* Return the color info. */
    DisplayInfo->alphaLength = display->alphaLength;
    DisplayInfo->alphaOffset = display->alphaOffset;
    DisplayInfo->redLength   = display->redLength;
    DisplayInfo->redOffset   = display->redOffset;
    DisplayInfo->greenLength = display->greenLength;
    DisplayInfo->greenOffset = display->greenOffset;
    DisplayInfo->blueLength  = display->blueLength;
    DisplayInfo->blueOffset  = display->blueOffset;

    /* Determine flip support. */
    DisplayInfo->flip = (display->multiBuffer > 1);

    /* 355_FB_MULTI_BUFFER */
    DisplayInfo->multiBuffer = Display->multiBuffer;
    DisplayInfo->backBufferY = Display->backBufferY;

    DisplayInfo->wrapFB = gcvTRUE;

    /* Success. */
    gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);
    return status;
}

gceSTATUS
fbdev_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x", Display);

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        /* Bad display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer < 1)
    {
        /* Turned off. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER_NO();
        return status;
    }

    /* Save size of virtual buffer. */
    *Width  = display->varInfo.xres_virtual;
    *Height = display->varInfo.yres_virtual;

    /* Success. */
    gcmFOOTER_ARG("*Width=%d *Height=%d", *Width, *Height);
    return status;
}

gceSTATUS
fbdev_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x", Display);
    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        /* Invalid display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer <= 1)
    {
        /* Turned off. */
        status = gcvSTATUS_NOT_SUPPORTED;
        gcmFOOTER_NO();
        return status;
    }

    pthread_mutex_lock(&(display->condMutex));

    /* Return current back buffer. */
    *X = 0;
    *Y = display->backBufferY;

    /* if swap interval is zero do not wait for the back buffer to change */
    if (display->swapInterval != 0)
    {
        while (display->backBufferY == (volatile int) (display->varInfo.yoffset))
        {
            pthread_cond_wait(&(display->cond), &(display->condMutex));
        }
    }

    /* Move to next back buffer. */
    display->backBufferY += display->alignedHeight;

    if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
    {
        /* Wrap around. */
        display->backBufferY = 0;
    }

    pthread_mutex_unlock(&(display->condMutex));

    /* Success. */
    gcmFOOTER_ARG("*Offset=%u *X=%d *Y=%d", *Offset, *X, *Y);
    return status;
}

gceSTATUS
fbdev_SetDisplayVirtual(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gctINT swapInterval;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    /*
     * This section may be called in a different thread, while main thread is
     * trying to free the display struct.
     * We must protect the display struct when the thread is running inside.
     */
    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    if (display == NULL)
    {
        pthread_mutex_unlock(&displayMutex);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer > 1)
    {
        /* clamp swap interval to be safe */
        swapInterval = display->swapInterval;
        if (swapInterval > GC_FB_MAX_SWAP_INTERVAL)
        {
            swapInterval = GC_FB_MAX_SWAP_INTERVAL;
        }
        else if (swapInterval < GC_FB_MIN_SWAP_INTERVAL)
        {
            swapInterval = GC_FB_MIN_SWAP_INTERVAL;
        }

        /* if swap interval is 0 skip this step */
        if (swapInterval != 0)
        {
            pthread_mutex_lock(&(display->condMutex));
            /* Panning will wait for the vsync. */
            swapInterval--;
            /* wait for swap interval  * vsync */
            while(swapInterval--)
            {
#ifdef FBIO_WAITFORVSYNC
                ioctl(display->file, FBIO_WAITFORVSYNC, (void *)0);
#endif
            }

            /* Set display offset. */
            display->varInfo.xoffset  = X;
            display->varInfo.yoffset  = Y;
            display->varInfo.activate = FB_ACTIVATE_VBL;
            ioctl(display->file, FBIOPAN_DISPLAY, &(display->varInfo));

            pthread_cond_broadcast(&(display->cond));
            pthread_mutex_unlock(&(display->condMutex));
        }
    }

    pthread_mutex_unlock(&displayMutex);

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return fbdev_SetDisplayVirtual(Display, Window, Offset, X, Y);
}

gceSTATUS
fbdev_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    /*
     * This section may be called in a different thread, while main thread is
     * trying to free the display struct.
     * We must protect the display struct when the thread is running inside.
     */
    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    if (display == NULL)
    {
        pthread_mutex_unlock(&displayMutex);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer > 1)
    {
        gctINT next;
        pthread_mutex_lock(&display->condMutex);

        next = (Y + display->height);
        if (next >= (int) display->varInfo.yres_virtual)
        {
            next = 0;
        }

        if (next != display->backBufferY)
        {
            gcmPRINT("%s: Canceling non-last buffer", __func__);
        }

        /* Roll back the buffer. */
        display->backBufferY = Y;

        pthread_cond_broadcast(&display->cond);
        pthread_mutex_unlock(&display->condMutex);
    }

    pthread_mutex_unlock(&displayMutex);

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_SetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT Interval
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gctINT interval;

    gcmHEADER_ARG("Display=0x%x Interval=%d", Display, Interval);

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        /* Invalid display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    /* clamp to min and max */
    interval = Interval;
    if (interval > GC_FB_MAX_SWAP_INTERVAL)
    {
        interval = GC_FB_MAX_SWAP_INTERVAL;
    }
    else if (interval < GC_FB_MIN_SWAP_INTERVAL)
    {
        interval = GC_FB_MIN_SWAP_INTERVAL;
    }
    display->swapInterval = interval;

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_SetSwapIntervalEx(
    IN PlatformDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay
    )
{
    return fbdev_SetSwapInterval(Display, Interval);
}

gceSTATUS
fbdev_GetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x Min=0x%x Max=0x%x", Display, Min, Max);

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
       /* Invalid display pointer. */
       status = gcvSTATUS_INVALID_ARGUMENT;
       gcmFOOTER();
       return status;
    }

    if( Min != gcvNULL)
    {
        *Min = GC_FB_MIN_SWAP_INTERVAL;
    }

    if (Max != gcvNULL)
    {
        *Max = GC_FB_MAX_SWAP_INTERVAL;
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;

}

gceSTATUS
fbdev_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    struct _FBDisplay* display;

    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    if (display != NULL)
    {
        /* Unlink form display stack. */
        if (display == displayStack)
        {
            /* First one. */
            displayStack = display->next;
        }
        else
        {
            struct _FBDisplay* prev = displayStack;

            while (prev->next != display)
            {
                prev = prev->next;
            }

            prev->next = display->next;
        }
    }

    pthread_mutex_unlock(&displayMutex);

    if (display != NULL)
    {
        if (display->memory != NULL)
        {
            munmap(display->memory, display->size);
            display->memory = NULL;
        }

        if (display->file >= 0)
        {
            ioctl(display->file, FBIOPUT_VSCREENINFO, &(display->orgVarInfo));

            close(display->file);
            display->file = -1;
        }

        pthread_mutex_destroy(&(display->condMutex));
        pthread_cond_destroy(&(display->cond));

        free(display);
        display = gcvNULL;
        Display = gcvNULL;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_DisplayBufferRegions(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Windows. ********************************************************************
*/

gceSTATUS
fbdev_CreateWindow(
    IN PlatformDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT PlatformWindowType * Window
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    struct _FBDisplay * display;
    struct _FBWindow *  window;
    gctINT              ignoreDisplaySize = 0;
    gctCHAR *           p;

    gcmHEADER_ARG("Display=%p X=%d Y=%d Width=%d Height=%d",
                  Display, X, Y, Width, Height);

    display = (struct _FBDisplay*) Display;
    if (display == NULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    p = getenv("FB_IGNORE_DISPLAY_SIZE");
    if (p != gcvNULL)
    {
        ignoreDisplaySize = atoi(p);
    }

    if (Width == 0)
    {
        Width = display->width;
    }

    if (Height == 0)
    {
        Height = display->height;
    }

    if (X == -1)
    {
        X = ((display->width - Width) / 2) & ~15;
    }

    if (Y == -1)
    {
        Y = ((display->height - Height) / 2) & ~7;
    }

    if (X < 0) X = 0;
    if (Y < 0) Y = 0;
    if (!ignoreDisplaySize)
    {
        if (X + Width  > display->width)  Width  = display->width  - X;
        if (Y + Height > display->height) Height = display->height - Y;
    }

    do
    {
        window = (struct _FBWindow *) malloc(gcmSIZEOF(struct _FBWindow));
        if (window == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }

        window->offset = Y * display->stride + X * ((display->bpp + 7) / 8);

        window->display = display;
        window->format = display->format;
        window->x = X;
        window->y = Y;

        window->width  = Width;
        window->height = Height;
        *Window = (PlatformWindowType) window;
    }
    while (0);

    gcmFOOTER_ARG("*Window=0x%x", *Window);
    return status;
}

gceSTATUS
fbdev_GetWindowInfo(
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
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBWindow* window;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);
    window = (struct _FBWindow*)Window;
    if (window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (X != gcvNULL)
    {
            *X = window->x;
    }

    if (Y != gcvNULL)
    {
            *Y = window->y;
    }

    if (Width != gcvNULL)
    {
        *Width = window->width;
    }

    if (Height != gcvNULL)
    {
        *Height = window->height;
    }

    if (BitsPerPixel != gcvNULL)
    {
        *BitsPerPixel = window->display->bpp;
    }

    if (Offset != gcvNULL)
    {
        *Offset = window->offset;
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_DestroyWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    if (Window != gcvNULL)
    {
        free(Window);
    }
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_DrawImage(
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
    unsigned char * ptr;
    int y;
    unsigned int bytes     = (Right - Left) * BitsPerPixel / 8;
    unsigned char * source = (unsigned char *) Bits;
    struct _FBDisplay* display;
    struct _FBWindow* window;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Left=%d Top=%d Right=%d Bottom=%d Width=%d Height=%d BitsPerPixel=%d Bits=0x%x",
                  Display, Window, Left, Top, Right, Bottom, Width, Height, BitsPerPixel, Bits);

    window = (struct _FBWindow*)Window;

    if ((window == gcvNULL) || (Bits == gcvNULL))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }
    else
    {
        display = window->display;
    }

    ptr = (unsigned char *) display->memory + (Left * display->bpp / 8);

    if (BitsPerPixel != display->bpp)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Height < 0)
    {
        for (y = Bottom - 1; y >= Top; --y)
        {
            unsigned char * line = ptr + y * display->stride;
            memcpy(line, source, bytes);

            source += Width * BitsPerPixel / 8;
        }
    }
    else
    {
        for (y = Top; y < Bottom; ++y)
        {
            unsigned char * line = ptr + y * display->stride;
            memcpy(line, source, bytes);

            source += Width * BitsPerPixel / 8;
        }
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_GetImage(
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
fbdev_CreatePixmap(
    IN PlatformDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    OUT PlatformPixmapType * Pixmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
#if !gcdUSE_PIXMAP_SURFACE
    gctINT alignedWidth, alignedHeight;
#endif
    struct _FBPixmap* pixmap;
#if gcdUSE_PIXMAP_SURFACE
    gceSURF_FORMAT format;
    gctPOINTER memAddr[3] = {gcvNULL};
#endif
    gcmHEADER_ARG("Display=0x%x Width=%d Height=%d BitsPerPixel=%d", Display, Width, Height, BitsPerPixel);

    if ((Width <= 0) || (Height <= 0) || (BitsPerPixel <= 0))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    do
    {
        pixmap = (struct _FBPixmap*) malloc(sizeof (struct _FBPixmap));
        memset(pixmap, 0, sizeof (struct _FBPixmap));

        if (pixmap == gcvNULL)
        {
            break;
        }
#if gcdUSE_PIXMAP_SURFACE
        if (BitsPerPixel <= 16)
        {
            format = gcvSURF_R5G6B5;
        }
        else if(BitsPerPixel == 24)
        {
            format = gcvSURF_X8R8G8B8;
        }
        else
        {
            format = gcvSURF_A8R8G8B8;
        }

        gcmERR_BREAK(gcoSURF_Construct(
            gcvNULL,
            Width, Height, 1,
            gcvSURF_BITMAP,
            format,
            gcvPOOL_DEFAULT,
            &pixmap->surface
        ));

        gcmERR_BREAK(gcoSURF_Lock(
            pixmap->surface,
                    &pixmap->gpu,
            memAddr
        ));
        pixmap->bits = memAddr[0];

        gcmERR_BREAK(gcoSURF_GetSize(
            pixmap->surface,
            (gctUINT *) &pixmap->width,
            (gctUINT *) &pixmap->height,
            gcvNULL
        ));

        gcmERR_BREAK(gcoSURF_GetAlignedSize(
            pixmap->surface,
            gcvNULL,
            gcvNULL,
            &pixmap->stride
        ));

        pixmap->bpp = (BitsPerPixel <= 16) ? 16 : 32;
#else
        alignedWidth   = (Width + 0x0F) & (~0x0F);
        alignedHeight  = (Height + 0x3) & (~0x03);
        pixmap->original = malloc(alignedWidth * alignedHeight * (BitsPerPixel + 7) / 8 + 64);
        if (pixmap->original == gcvNULL)
        {
            free(pixmap);
            pixmap = gcvNULL;

            break;
        }
        pixmap->bits = (gctPOINTER)(((gctUINTPTR_T)(gctCHAR*)pixmap->original + 0x3F) & (~0x3F));

        pixmap->width = Width;
        pixmap->height = Height;
        pixmap->alignedWidth    = alignedWidth;
        pixmap->alignedHeight   = alignedHeight;
        pixmap->bpp     = (BitsPerPixel + 0x07) & (~0x07);
        pixmap->stride  = Width * (pixmap->bpp) / 8;
#endif

        *Pixmap = (PlatformPixmapType)pixmap;
        gcmFOOTER_ARG("*Pixmap=0x%x", *Pixmap);
        return status;
    }
    while (0);

#if gcdUSE_PIXMAP_SURFACE
    if (pixmap!= gcvNULL)
    {
        if (pixmap->bits != gcvNULL)
        {
            gcoSURF_Unlock(pixmap->surface, pixmap->bits);
        }
        if (pixmap->surface != gcvNULL)
        {
            gcoSURF_Destroy(pixmap->surface);
        }
        free(pixmap);
    }
#endif

    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

gceSTATUS
fbdev_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBPixmap* pixmap;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);
    pixmap = (struct _FBPixmap*)Pixmap;
    if (pixmap == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Width != NULL)
    {
        *Width = pixmap->width;
    }

    if (Height != NULL)
    {
        *Height = pixmap->height;
    }

    if (BitsPerPixel != NULL)
    {
        *BitsPerPixel = pixmap->bpp;
    }

    if (Stride != NULL)
    {
        *Stride = pixmap->stride;
    }

    if (Bits != NULL)
    {
        *Bits = (pixmap->bits ? pixmap->bits : pixmap->original);
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
fbdev_DestroyPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap
    )
{
    struct _FBPixmap* pixmap;
    pixmap = (struct _FBPixmap*)Pixmap;
    if (pixmap != gcvNULL)
    {
#if gcdUSE_PIXMAP_SURFACE
        if (pixmap->bits != gcvNULL)
        {
            gcoSURF_Unlock(pixmap->surface, pixmap->bits);
        }
        if (pixmap->surface != gcvNULL)
        {
            gcoSURF_Destroy(pixmap->surface);
        }
#else
        if (pixmap->original != NULL)
        {
            free(pixmap->original);
        }
#endif
        free(pixmap);
        pixmap = gcvNULL;
        Pixmap = gcvNULL;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_DrawPixmap(
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
fbdev_SetWindowTitle(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctCONST_STRING Title
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_CapturePointer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_CreateClientBuffer(
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT Format,
    IN gctINT Type,
    OUT gctPOINTER * ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_GetClientBufferInfo(
    IN gctPOINTER ClientBuffer,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_DestroyClientBuffer(
    IN gctPOINTER ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_GetDisplayInfoEx2(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT fbdevDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = fbdev_GetDisplayInfoEx(Display, Window, DisplayInfoSize, DisplayInfo);
    if(gcmIS_SUCCESS(status))
    {
        if ((DisplayInfo->logical == gcvNULL) || (DisplayInfo->physical == ~0U))
        {
            /* No offset. */
            status = gcvSTATUS_NOT_SUPPORTED;
        }
        else
            status = gcvSTATUS_OK;
    }
    return status;
}

gceSTATUS
fbdev_GetDisplayBackbufferEx(
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
    return fbdev_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
}

gceSTATUS
fbdev_IsValidDisplay(
    IN PlatformDisplayType Display
    )
{
    struct _FBDisplay* display;

    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    pthread_mutex_unlock(&displayMutex);

    if (display != NULL)
    {
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
fbdev_GetNativeVisualId(
    IN PlatformDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_GetWindowInfoEx(
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
    struct _FBDisplay* display;

    display = (struct _FBDisplay*)Display;

    /* Valid display? and structure size? */
    if (display == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcmIS_ERROR(fbdev_GetWindowInfo(
                          Display,
                          Window,
                          X,
                          Y,
                          (gctINT_PTR) Width,
                          (gctINT_PTR) Height,
                          (gctINT_PTR) BitsPerPixel,
                          gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Get the color format. */
    switch (display->greenLength)
    {
    case 4:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X4B4G4R4 : gcvSURF_A4B4G4R4;
        }
        break;

    case 5:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X1B5G5R5 : gcvSURF_A1B5G5R5;
        }
        break;

    case 6:
        *Format = gcvSURF_R5G6B5;
        break;

    case 8:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X8B8G8R8 : gcvSURF_A8B8G8R8;
        }
        break;

    default:
        /* Unsupported colot depth. */
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (Type != gcvNULL)
    {
        *Type = gcvSURF_BITMAP;
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_DrawImageEx(
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
    return fbdev_DrawImage(Display,
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
fbdev_SetWindowFormat(
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
fbdev_GetPixmapInfoEx(
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
    if (gcmIS_ERROR(fbdev_GetPixmapInfo(
                        Display,
                        Pixmap,
                      (gctINT_PTR) Width, (gctINT_PTR) Height,
                      (gctINT_PTR) BitsPerPixel,
                      gcvNULL,
                      gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Return format for pixmap depth. */
    switch (*BitsPerPixel)
    {
    case 16:
        *Format = gcvSURF_R5G6B5;
        break;

    case 32:
        *Format = gcvSURF_A8R8G8B8;
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
fbdev_CopyPixmapBits(
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

gctBOOL
fbdev_SynchronousFlip(
    IN PlatformDisplayType Display
    )
{
    return gcvFALSE;
}
gceSTATUS
fbdev_CreateContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}
gceSTATUS
fbdev_DestroyContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_MakeCurrent(
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
fbdev_CreateDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_DestroyDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_RSForSwap(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctPOINTER resolve
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
fbdev_SwapBuffers(
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
fbdev_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/******************************************************************************/
/* TODO: merge functions. */

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
 *
 * TODO: But this will break frame skipping because skipped back buffer post
 * will cause infinite wait in getWindowBackBuffer.
 */
#define SYNC_TEMPORARY_RESOLVE_SURFACES     0


/******************************************************************************/
/* Display. */

static void *
_GetDefaultDisplay(
    void
    )
{
    PlatformDisplayType display = gcvNULL;
    fbdev_GetDisplay(&display, gcvNULL);

    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    fbdev_DestroyDisplay((PlatformDisplayType) Display);
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (gcmIS_ERROR(fbdev_IsValidDisplay((PlatformDisplayType) Display)))
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

    status = fbdev_InitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    status = fbdev_DeinitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    fbdev_GetNativeVisualId((PlatformDisplayType) Display->hdc, &visualId);
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
    status = fbdev_GetSwapInterval((PlatformDisplayType) Display->hdc,
                                   Min, Max);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLDisplay Display,
    IN EGLint Interval
    )
{
    gceSTATUS status;

    status = fbdev_SetSwapInterval((PlatformDisplayType) Display->hdc,
                                   Interval);

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

    /* Information obtained by fbdev_GetDisplayInfoEx2. */
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

    if (Info->fbDirect)
    {
        if (Info->wrapFB)
        {
            gctPOINTER pointer;
            gctUINT alignedHeight;
            gceSURF_TYPE baseType;
            gctUINT i;

            baseType = (gceSURF_TYPE) ((gctUINT32) Info->type & 0xFF);

            /* Lock. */
            gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

            alignedHeight = Info->yresVirtual / Info->multiBuffer;

            for (i = 0; i < Info->multiBuffer; i++)
            {
                /*
                 * TODO: Check wrapper limitations.
                 * Allocate temporary surface objects if can not wrap.
                 *
                 * Current logic follows former code without changes.
                 */
                gctUINT    offset;
                gctPOINTER logical;
                gctUINT    physical;

                /* Allocate native buffer object. */
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          sizeof (struct eglNativeBuffer),
                                          &pointer));

                gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
                buffer = pointer;

                /* Add into buffer list. */
                if (Info->bufferList != gcvNULL)
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

                /* Bytes offset to the buffer. */
                offset = (gctUINT) (Info->stride * alignedHeight * i);

                /* Calculate buffer addresses. */
                logical  = (gctUINT8_PTR) Info->logical + offset;
                physical = Info->physical + offset;

                /* Construct the wrapper. */
                gcmONERROR(gcoSURF_Construct(gcvNULL,
                                             Info->width,
                                             Info->height, 1,
                                             Info->type,
                                             Info->format,
                                             gcvPOOL_USER,
                                             &buffer->surface));

                /* Set the underlying framebuffer surface. */
                gcmONERROR(gcoSURF_SetBuffer(buffer->surface,
                                             Info->type,
                                             Info->format,
                                             Info->stride,
                                             logical,
                                             physical));

                /* For a new surface, clear it to get rid of noises. */
                gcoOS_ZeroMemory(logical, alignedHeight * Info->stride);

                gcmONERROR(gcoSURF_SetWindow(buffer->surface,
                                             0, 0,
                                             Info->width, Info->height));

                /* Initial lock for user-pool surface. */
                gcmONERROR(gcoSURF_Lock(buffer->surface, gcvNULL, gcvNULL));

                (void) baseType;

#if gcdENABLE_3D
                if (baseType == gcvSURF_RENDER_TARGET)
                {
                    /* Render target surface orientation is different. */
                    gcmVERIFY_OK(
                        gcoSURF_SetFlags(buffer->surface,
                                         gcvSURF_FLAG_CONTENT_YINVERTED,
                                         gcvTRUE));

                    if (!(Info->type & gcvSURF_NO_TILE_STATUS))
                    {
                        /* Append tile status to this user-pool rt. */
                        gcmVERIFY_OK(gcoSURF_AppendTileStatus(buffer->surface));
                    }
                }
#endif

                buffer->context  = gcvNULL;
                buffer->origin.x = 0;
                buffer->origin.y = alignedHeight * i;

                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): buffer[%d]: yoffset=%-4d physical=%x",
                         __FUNCTION__, __LINE__,
                         i, alignedHeight * i, physical);
            }

            gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
        }
        else
        {
            gcmPRINT("%s(%d): Invalid integration!", __FUNCTION__, __LINE__);
            return gcvSTATUS_OK;
        }
    }
    else
    {
        /* Create temporary surface objects */
        gctINT i;
        gctPOINTER pointer;

        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        for (i = 0; i < NUM_TEMPORARY_RESOLVE_SURFACES; i++)
        {
            /* Allocate native buffer object. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      sizeof (struct eglNativeBuffer),
                                      &pointer));

            gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
            buffer = pointer;

            /* Add into buffer list. */
            if (Info->bufferList != gcvNULL)
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

            /* Construct temporary resolve surfaces. */
            gcmONERROR(gcoSURF_Construct(gcvNULL,
                                         Info->width,
                                         Info->height, 1,
                                         gcvSURF_BITMAP,
                                         Info->format,
                                         gcvPOOL_DEFAULT,
                                         &buffer->surface));

#if SYNC_TEMPORARY_RESOLVE_SURFACES
            /* Create the buffer lock. */
            gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &buffer->lock));

            /* Set initial 'unlocked' state. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
#endif

            gcmTRACE(gcvLEVEL_INFO,
                     "%s(%d): buffer[%d]: surface=%p",
                     __FUNCTION__, __LINE__,
                     i, buffer->surface);
        }

        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    return EGL_TRUE;

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
    fbdevDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = fbdev_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
    gcoOS_ZeroMemory(&dInfo, sizeof (fbdevDISPLAY_INFO));

    status = fbdev_GetDisplayInfoEx2((PlatformDisplayType) Display->hdc,
                                     (PlatformWindowType) Window,
                                     Display->localInfo,
                                     sizeof (fbdevDISPLAY_INFO),
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
        Info->multiBuffer  = dInfo.multiBuffer > 1 ? dInfo.multiBuffer : 1;
    }

    /* Get virtual size. */
    status = fbdev_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
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
    fbdev_CreateDrawable(Display->localInfo, (PlatformWindowType) win);

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

    fbdev_DestroyDrawable(Display->localInfo, (PlatformWindowType) win);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, info);
    return EGL_TRUE;
}

#if gcdENABLE_RENDER_INTO_WINDOW && gcdENABLE_3D
/*
 * For apps in this list, EGL will use indirect rendering,
 * ie, disable no-resolve.
 */
static gcePATCH_ID indirectList[] =
{
    -1,
};

#endif


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

    status = fbdev_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
        /* Check direct rendering support for 2D VG. */
        do
        {
            EGLBoolean formatSupported = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            switch (format)
            {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_A8B8G8R8:
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8R8G8B8:
            case gcvSURF_X8B8G8R8:
                if (Surface->config.alphaSize == 0)
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_R5G6B5:
                if ((Surface->config.redSize == 5) &&
                    (Surface->config.greenSize == 6) &&
                    (Surface->config.blueSize == 5) &&
                    (Surface->config.alphaSize == 0))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_A4R4G4B4:
            case gcvSURF_A4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4) &&
                    (Surface->config.alphaSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_X4R4G4B4:
            case gcvSURF_X4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            /* Should use direct rendering. */
            renderMode = VEGL_DIRECT_RENDERING;
        }
        while (gcvFALSE);

        if ((type != gcvSURF_BITMAP) ||
            (info->type != gcvSURF_BITMAP))
        {
            /* Request linear buffer for hardware OpenVG. */
            status = fbdev_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
#if gcdENABLE_RENDER_INTO_WINDOW
        /* 3D pipe. */
        do
        {
            /* Check if direct rendering is available. */
            EGLBoolean fcFill = EGL_FALSE;
            EGLBoolean formatSupported;
            gceSURF_FORMAT reqFormat = format;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

            /* Get patch id. */
            gcoHAL_GetPatchID(gcvNULL, &patchId);

            for (i = 0; i < gcmCOUNTOF(indirectList); i++)
            {
                if (patchId == indirectList[i])
                {
                    indirect = EGL_TRUE;
                    break;
                }
            }

            if (indirect)
            {
                /* Forced indirect rendering. */
                break;
            }

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            /* Require fc-fill feature in hardware. */
            status = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER);

            if (status == gcvSTATUS_TRUE)
            {
                /* Has fc-fill feature. */
                fcFill = EGL_TRUE;
            }

            /* Check window format. */
            switch (format)
            {
            case gcvSURF_A8B8G8R8:
                reqFormat = gcvSURF_A8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8B8G8R8:
                reqFormat = gcvSURF_X8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A4R4G4B4:
            case gcvSURF_X4R4G4B4:
            case gcvSURF_R5G6B5:
                formatSupported = EGL_TRUE;
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            if (!info->wrapFB)
            {
                /* Try many direct rendering levels here. */
                /* 1. The best, direct rendering with compression. */
                if ((type != gcvSURF_RENDER_TARGET) ||
                    (info->type != gcvSURF_RENDER_TARGET)  ||
                    (info->format != reqFormat))
                {
                    status = fbdev_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering with compression. */
                        renderMode = VEGL_DIRECT_RENDERING;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING;
                }

                /* 2. Second, with tile status, no compression. */
                if ((type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->format != reqFormat))
                {

                    status = fbdev_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET_NO_COMPRESSION,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering without compression. */
                        renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already direct rendering without compression. */
                    renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
                }
            }

            if (!fcFill)
            {
                /* Do not need check the next mode. */
                break;
            }

            /*
             * Special for FC-FILL mode: tile status is required.
             * Final internal render type should be RENDER_TARGET_NO_COMPRESSION.
             */
            if ((type != gcvSURF_RENDER_TARGET_NO_TILE_STATUS) ||
                (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                (info->format != reqFormat))
            {
                /* Try FC fill. */
                status = fbdev_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                               (PlatformWindowType) win,
                                               gcvSURF_RENDER_TARGET_NO_TILE_STATUS,
                                               reqFormat);

                if (gcmIS_SUCCESS(status))
                {
                    /* Should use direct rendering with fc-fill. */
                    renderMode = VEGL_DIRECT_RENDERING_FCFILL;

                    /* Window is changed. */
                    winChanged = EGL_TRUE;
                    break;
                }

                /* Not an error. */
                status = gcvSTATUS_OK;
            }
            else
            {
                /* Already direct rendering with fc-fill. */
                renderMode = VEGL_DIRECT_RENDERING_FCFILL;
            }
        }
        while (gcvFALSE);
#   endif

        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            ((type != gcvSURF_BITMAP) || (info->type != gcvSURF_BITMAP)))
        {
            /* Only linear supported in this case. */
            status = fbdev_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

    status = fbdev_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
    void * win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    if (info->fbDirect)
    {
        gctUINT offset;

        BackBuffer->surface  = gcvNULL;
        BackBuffer->context  = gcvNULL;
        BackBuffer->origin.x = 0;
        BackBuffer->origin.y = 0;
        BackBuffer->flip     = gcvTRUE;

        /* Formerly veglGetDisplayBackBuffer. */
        status = fbdev_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
                                              (PlatformWindowType) win,
                                               Display->localInfo,
                                               &BackBuffer->context,
                                               &BackBuffer->surface,
                                               &offset,
                                               &BackBuffer->origin.x,
                                               &BackBuffer->origin.y);

        if (gcmIS_ERROR(status))
        {
            /*
             * Fomerly, returns flip=false, then it will use first wrapper.
             */
            VEGLNativeBuffer buffer = info->bufferList;

            if (!buffer)
            {
                /* No wrappers? Bad native window. */
                return EGL_FALSE;
            }

            /* Copy out back buffer. */
            BackBuffer->context = buffer->context;
            BackBuffer->origin  = buffer->origin;
            BackBuffer->surface = buffer->surface;

            BackBuffer->flip    = gcvFALSE;

            /* Increase reference count. */
            /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

            return EGL_TRUE;
        }

        if (BackBuffer->surface)
        {
            /* Returned the surface directly. */
            return EGL_TRUE;
        }
        else
        {
            VEGLNativeBuffer buffer = gcvNULL;

            /* WrapFB or temporary surface, go through bufferList to find */
            gcmASSERT(info->wrapFB);

            if (info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer buf = info->bufferList;

                gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

                do
                {
                    if ((buf->context  == BackBuffer->context)  &&
                        (buf->origin.x == BackBuffer->origin.x) &&
                        (buf->origin.y == BackBuffer->origin.y))
                    {
                        /* Found. */
                        buffer = buf;
                        break;
                    }

                    buf = buf->next;
                }
                while (buffer != info->bufferList);

                gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);
            }

            if (buffer != gcvNULL)
            {
                /* Return the found surface. */
                BackBuffer->surface  = buffer->surface;
                BackBuffer->context  = buffer->context;
                BackBuffer->origin.x = buffer->origin.x;
                BackBuffer->origin.y = buffer->origin.y;

                /* Increase reference count. */
                /* gcoSURF_ReferenceSurface(BackBuffer->surface); */
                return EGL_TRUE;
            }
            else
            {
                /* Bad native window. */
                return EGL_FALSE;
            }
        }
    }
    else
    {
        /* Return the temorary surface object. */
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

        /* Increase reference count. */
        /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

        return EGL_TRUE;
    }
}

static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN EGLint NumRects,
    IN EGLint Rects[]
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
        surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

        status = fbdev_SetDisplayVirtualEx((PlatformDisplayType) Display->hdc,
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

        for (i = 0; i < NumRects; i++)
        {
            EGLint left   = Rects[i * 4 + 0];
            EGLint top    = Rects[i * 4 + 1];
            EGLint width  = Rects[i * 4 + 2];
            EGLint height = Rects[i * 4 + 3];

            /* Draw image. */
            status = fbdev_DrawImageEx((PlatformDisplayType) Display->hdc,
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

    status = fbdev_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
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
    return fbdev_SynchronousFlip((PlatformDisplayType)Display->hdc);
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    /* TODO */
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
    /* TODO */
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
        gcmONERROR(fbdev_CopyPixmapBits((PlatformDisplayType) Info->hdc,
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
        gcmONERROR(fbdev_DrawPixmap((PlatformDisplayType) Info->hdc,
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

    status = fbdev_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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
    gcmONERROR(fbdev_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = fbdev_GetPixmapInfo((PlatformDisplayType) Display->hdc,
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

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Initial lock for user-pool surface. */
        status = gcoSURF_Lock(wrapper, gcvNULL, gcvNULL);
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
    info->hdc          = (PlatformDisplayType) Display->hdc;
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
        fbdev_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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


static struct eglPlatform fbdevPlatform =
{
    0,

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
veglGetFbdevPlatform(
    void * NativeDisplay
    )
{
    return &fbdevPlatform;
}

/******************************************************************************/

void *
fbGetDisplay(
    gctPOINTER context
    )
{
    PlatformDisplayType display = gcvNULL;
    fbdev_GetDisplay(&display, context);
    return display;
}

void *
fbGetDisplayByIndex(
    IN gctINT DisplayIndex
    )
{
    PlatformDisplayType display = gcvNULL;
    fbdev_GetDisplayByIndex(DisplayIndex, &display, gcvNULL);
    return display;
}

void
fbGetDisplayGeometry(
    IN void * Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbdev_GetDisplayInfo((PlatformDisplayType)Display, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetDisplayInfo(
    IN void * Display,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT unsigned long *Physical,
    OUT gctINT_PTR Stride,
    OUT gctINT_PTR BitsPerPixel
    )
{
    fbdev_GetDisplayInfo((PlatformDisplayType)Display, Width, Height,(gctSIZE_T_PTR)Physical, Stride, BitsPerPixel);
}

void
fbDestroyDisplay(
    IN void * Display
    )
{
    fbdev_DestroyDisplay((PlatformDisplayType)Display);
}

void *
fbCreateWindow(
    IN void * Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformWindowType window = gcvNULL;
    fbdev_CreateWindow((PlatformDisplayType)Display, X, Y, Width, Height, &window);
    return window;
}

void
fbGetWindowGeometry(
    IN void * Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbdev_GetWindowInfo(gcvNULL, (PlatformWindowType)Window, X, Y, Width, Height, gcvNULL, gcvNULL);
}

void
fbGetWindowInfo(
    IN void * Window,
    OUT gctINT_PTR X,
    OUT gctINT_PTR Y,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctUINT_PTR Offset
    )
{
    fbdev_GetWindowInfo(gcvNULL, (PlatformWindowType)Window, X, Y, Width, Height, BitsPerPixel, Offset);
}

void
fbDestroyWindow(
    IN void * Window
    )
{
    fbdev_DestroyWindow(gcvNULL, (PlatformWindowType)Window);
}

void *
fbCreatePixmap(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    fbdev_CreatePixmap((PlatformDisplayType)Display, Width, Height, 32, &Pixmap);
    return Pixmap;
}

void *
fbCreatePixmapWithBpp(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    fbdev_CreatePixmap((PlatformDisplayType)Display, Width, Height, BitsPerPixel, &Pixmap);
    return Pixmap;
}

void
fbGetPixmapGeometry(
    IN void * Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height
    )
{
    fbdev_GetPixmapInfo(gcvNULL, (PlatformPixmapType)Pixmap, Width, Height, gcvNULL, gcvNULL, gcvNULL);
}

void
fbGetPixmapInfo(
    IN void * Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctINT_PTR Stride,
    OUT gctPOINTER * Bits
    )
{
    fbdev_GetPixmapInfo(gcvNULL, (PlatformPixmapType)Pixmap, Width, Height, BitsPerPixel, Stride, Bits);
}

void
fbDestroyPixmap(
    IN void * Pixmap
    )
{
    fbdev_DestroyPixmap(gcvNULL, (PlatformPixmapType)Pixmap);
}

