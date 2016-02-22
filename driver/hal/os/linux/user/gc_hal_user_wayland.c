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


#if gcdENABLE_3D || gcdENABLE_VG
#if (defined EGL_API_FB && defined EGL_API_WL)
/* Enable sigaction declarations. */
#if defined(LINUX)
#if !defined _XOPEN_SOURCE
#   define _XOPEN_SOURCE 501
#endif
#endif
#include "gc_hal_user_linux.h"
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
#include "gc_wayland_protocol.h"
#include "wayland-viv-server-protocol.h"
#include "wayland-server.h"
#include "gc_hal_eglplatform.h"
/*From include/linux/mxcfb.h*/
#define MXCFB_WAIT_FOR_VSYNC _IOW('F',0x20, u_int32_t)

#define MXCFB_SET_PREFETCH  _IOW('F', 0x30, int)
#define MXCFB_GET_PREFETCH  _IOR('F', 0x31, int)
#ifndef gcdENABLE_FB_PREFETCH
#   define gcdENABLE_FB_PREFETCH       0
#endif

/*From include/linux/ipu.h*/
#define fourcc(a, b, c, d)\
     (((__u32)(a)<<0)|((__u32)(b)<<8)|((__u32)(c)<<16)|((__u32)(d)<<24))

#define IPU_PIX_FMT_GPU32_SB_ST  fourcc('5', 'P', '4', 'S') /*!< 32bit split buf 4x4 standard */
#define IPU_PIX_FMT_GPU32_SB_SRT fourcc('5', 'P', '4', 'R') /*!< 32bit split buf 4x4 super */
#define IPU_PIX_FMT_GPU32_ST     fourcc('5', 'I', '4', 'S') /*!< 32bit single buf 4x4 standard */
#define IPU_PIX_FMT_GPU32_SRT    fourcc('5', 'I', '4', 'R') /*!< 32bit single buf 4x4 super */
#define IPU_PIX_FMT_GPU16_SB_ST  fourcc('4', 'P', '8', 'S') /*!< 16bit split buf 8x4 standard */
#define IPU_PIX_FMT_GPU16_SB_SRT fourcc('4', 'P', '8', 'R') /*!< 16bit split buf 8x4 super */
#define IPU_PIX_FMT_GPU16_ST     fourcc('4', 'I', '8', 'S') /*!< 16bit single buf 8x4 standard */
#define IPU_PIX_FMT_GPU16_SRT    fourcc('4', 'I', '8', 'R') /*!< 16bit single buf 8x4 super */


#define gcdUSE_PIXMAP_SURFACE 1
#define _GC_OBJ_ZONE    gcvZONE_OS

#define GC_FB_MAX_SWAP_INTERVAL     10
#define GC_FB_MIN_SWAP_INTERVAL     0

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif

#if defined (WAYLAND_VERSION_MAJOR) &&      \
    WAYLAND_VERSION_MAJOR == 1 &&       \
    WAYLAND_VERSION_MINOR < 6
#define WAYLAND_LEGACY_SUPPORT
#endif

#define WL_EGL_NUM_BACKBUFFERS     3
#define WL_EGL_MAX_NUM_BACKBUFFERS 3

/* Structure that defines a display. */
struct _FBDisplay
{
    gctUINT                 wl_signature;
    /* Note that struct wl_display is not the same object for client and server. */
    struct wl_display *     wl_display;
    gctINT                  index;
    gctINT                  file;
    gctSIZE_T               physical;
    gctINT                  stride;
    gctINT                  width;
    gctINT                  height;
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
    gctINT                  swapInterval;

    gctBOOL                 fbPrefetch;

    struct _FBDisplay *     next;
};

/* Structure that defines a window. */
struct _FBWindow
{
    gctUINT                 wl_signature;
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
    struct wl_buffer wl_buffer;
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

static halKeyMap keys[] =
{
    /* 00 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 01 */ { HAL_ESCAPE,          HAL_UNKNOWN     },
    /* 02 */ { HAL_1,               HAL_UNKNOWN     },
    /* 03 */ { HAL_2,               HAL_UNKNOWN     },
    /* 04 */ { HAL_3,               HAL_UNKNOWN     },
    /* 05 */ { HAL_4,               HAL_UNKNOWN     },
    /* 06 */ { HAL_5,               HAL_UNKNOWN     },
    /* 07 */ { HAL_6,               HAL_UNKNOWN     },
    /* 08 */ { HAL_7,               HAL_UNKNOWN     },
    /* 09 */ { HAL_8,               HAL_UNKNOWN     },
    /* 0A */ { HAL_9,               HAL_UNKNOWN     },
    /* 0B */ { HAL_0,               HAL_UNKNOWN     },
    /* 0C */ { HAL_HYPHEN,          HAL_UNKNOWN     },
    /* 0D */ { HAL_EQUAL,           HAL_UNKNOWN     },
    /* 0E */ { HAL_BACKSPACE,       HAL_UNKNOWN     },
    /* 0F */ { HAL_TAB,             HAL_UNKNOWN     },
    /* 10 */ { HAL_Q,               HAL_UNKNOWN     },
    /* 11 */ { HAL_W,               HAL_UNKNOWN     },
    /* 12 */ { HAL_E,               HAL_UNKNOWN     },
    /* 13 */ { HAL_R,               HAL_UNKNOWN     },
    /* 14 */ { HAL_T,               HAL_UNKNOWN     },
    /* 15 */ { HAL_Y,               HAL_UNKNOWN     },
    /* 16 */ { HAL_U,               HAL_UNKNOWN     },
    /* 17 */ { HAL_I,               HAL_UNKNOWN     },
    /* 18 */ { HAL_O,               HAL_UNKNOWN     },
    /* 19 */ { HAL_P,               HAL_UNKNOWN     },
    /* 1A */ { HAL_LBRACKET,        HAL_UNKNOWN     },
    /* 1B */ { HAL_RBRACKET,        HAL_UNKNOWN     },
    /* 1C */ { HAL_ENTER,           HAL_PAD_ENTER   },
    /* 1D */ { HAL_LCTRL,           HAL_RCTRL       },
    /* 1E */ { HAL_A,               HAL_UNKNOWN     },
    /* 1F */ { HAL_S,               HAL_UNKNOWN     },
    /* 20 */ { HAL_D,               HAL_UNKNOWN     },
    /* 21 */ { HAL_F,               HAL_UNKNOWN     },
    /* 22 */ { HAL_G,               HAL_UNKNOWN     },
    /* 23 */ { HAL_H,               HAL_UNKNOWN     },
    /* 24 */ { HAL_J,               HAL_UNKNOWN     },
    /* 25 */ { HAL_K,               HAL_UNKNOWN     },
    /* 26 */ { HAL_L,               HAL_UNKNOWN     },
    /* 27 */ { HAL_SEMICOLON,       HAL_UNKNOWN     },
    /* 28 */ { HAL_SINGLEQUOTE,     HAL_UNKNOWN     },
    /* 29 */ { HAL_BACKQUOTE,       HAL_UNKNOWN     },
    /* 2A */ { HAL_LSHIFT,          HAL_UNKNOWN     },
    /* 2B */ { HAL_BACKSLASH,       HAL_UNKNOWN     },
    /* 2C */ { HAL_Z,               HAL_UNKNOWN     },
    /* 2D */ { HAL_X,               HAL_UNKNOWN     },
    /* 2E */ { HAL_C,               HAL_UNKNOWN     },
    /* 2F */ { HAL_V,               HAL_UNKNOWN     },
    /* 30 */ { HAL_B,               HAL_UNKNOWN     },
    /* 31 */ { HAL_N,               HAL_UNKNOWN     },
    /* 32 */ { HAL_M,               HAL_UNKNOWN     },
    /* 33 */ { HAL_COMMA,           HAL_UNKNOWN     },
    /* 34 */ { HAL_PERIOD,          HAL_UNKNOWN     },
    /* 35 */ { HAL_SLASH,           HAL_PAD_SLASH   },
    /* 36 */ { HAL_RSHIFT,          HAL_UNKNOWN     },
    /* 37 */ { HAL_PAD_ASTERISK,    HAL_PRNTSCRN    },
    /* 38 */ { HAL_LALT,            HAL_RALT        },
    /* 39 */ { HAL_SPACE,           HAL_UNKNOWN     },
    /* 3A */ { HAL_CAPSLOCK,        HAL_UNKNOWN     },
    /* 3B */ { HAL_F1,              HAL_UNKNOWN     },
    /* 3C */ { HAL_F2,              HAL_UNKNOWN     },
    /* 3D */ { HAL_F3,              HAL_UNKNOWN     },
    /* 3E */ { HAL_F4,              HAL_UNKNOWN     },
    /* 3F */ { HAL_F5,              HAL_UNKNOWN     },
    /* 40 */ { HAL_F6,              HAL_UNKNOWN     },
    /* 41 */ { HAL_F7,              HAL_UNKNOWN     },
    /* 42 */ { HAL_F8,              HAL_UNKNOWN     },
    /* 43 */ { HAL_F9,              HAL_UNKNOWN     },
    /* 44 */ { HAL_F10,             HAL_UNKNOWN     },
    /* 45 */ { HAL_NUMLOCK,         HAL_UNKNOWN     },
    /* 46 */ { HAL_SCROLLLOCK,      HAL_BREAK       },
    /* 47 */ { HAL_PAD_7,           HAL_HOME        },
    /* 48 */ { HAL_PAD_8,           HAL_UP          },
    /* 49 */ { HAL_PAD_9,           HAL_PGUP        },
    /* 4A */ { HAL_PAD_HYPHEN,      HAL_UNKNOWN     },
    /* 4B */ { HAL_PAD_4,           HAL_LEFT        },
    /* 4C */ { HAL_PAD_5,           HAL_UNKNOWN     },
    /* 4D */ { HAL_PAD_6,           HAL_RIGHT       },
    /* 4E */ { HAL_PAD_PLUS,        HAL_UNKNOWN     },
    /* 4F */ { HAL_PAD_1,           HAL_END         },
    /* 50 */ { HAL_PAD_2,           HAL_DOWN        },
    /* 51 */ { HAL_PAD_3,           HAL_PGDN        },
    /* 52 */ { HAL_PAD_0,           HAL_INSERT      },
    /* 53 */ { HAL_PAD_PERIOD,      HAL_DELETE      },
    /* 54 */ { HAL_SYSRQ,           HAL_UNKNOWN     },
    /* 55 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 56 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 57 */ { HAL_F11,             HAL_UNKNOWN     },
    /* 58 */ { HAL_F12,             HAL_UNKNOWN     },
    /* 59 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 5A */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 5B */ { HAL_UNKNOWN,         HAL_LWINDOW     },
    /* 5C */ { HAL_UNKNOWN,         HAL_RWINDOW     },
    /* 5D */ { HAL_UNKNOWN,         HAL_MENU        },
    /* 5E */ { HAL_UNKNOWN,         HAL_POWER       },
    /* 5F */ { HAL_UNKNOWN,         HAL_SLEEP       },
    /* 60 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 61 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 62 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 63 */ { HAL_UNKNOWN,         HAL_WAKE        },
    /* 64 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 65 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* For TTC Board only */
    /* 66 */ { HAL_HOME,            HAL_UNKNOWN     },
    /* 67 */ { HAL_UP,              HAL_UNKNOWN     },
    /* 68 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 69 */ { HAL_LEFT,            HAL_UNKNOWN     },
    /* 6A */ { HAL_RIGHT,           HAL_UNKNOWN     },
    /* 6B */ { HAL_ESCAPE,          HAL_UNKNOWN     },
    /* 6C */ { HAL_DOWN,            HAL_UNKNOWN     },
    /* 6D */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 6E */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 6F */ { HAL_BACKSPACE,       HAL_UNKNOWN     },
    /* End */
    /* 70 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 71 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 72 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 73 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 74 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 75 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 76 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 77 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 78 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 79 */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7A */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7B */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7C */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7D */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7E */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
    /* 7F */ { HAL_UNKNOWN,         HAL_UNKNOWN     },
};

gctCHAR            name[11];
gctINT             uid, gid;
gctINT             active;
gctINT             file = -1;
gctINT             mice = -1;
gctINT             mode;
struct termios  tty;
gctINT ignore;
gctINT hookSEGV = 0;

static  struct wl_list WLEGLWindowList = {0};
static  struct wl_list WLEGLBufferList = {0};
static  pthread_mutex_t registerMutex = PTHREAD_MUTEX_INITIALIZER;

static gctBOOL inline
_IsCompositorDisplay(struct _FBDisplay *Display)
{
    return ((Display->wl_signature == WL_COMPOSITOR_SIGNATURE) ? gcvTRUE : gcvFALSE);
}

static gctBOOL inline
_IsWaylandLocalDisplay(gcsWL_LOCAL_DISPLAY* Display)
{
    return ((Display->wl_signature == WL_LOCAL_DISPLAY_SIGNATURE) ? gcvTRUE : gcvFALSE);
}

static gctBOOL inline
_IsCompositorWindow(struct _FBWindow *Window)
{
    return ((Window->wl_signature == WL_COMPOSITOR_SIGNATURE) ? gcvTRUE : gcvFALSE);
}

static gctBOOL inline
_IsClientWindow(struct wl_egl_window *Window)
{
    return ((Window->wl_signature == WL_CLIENT_SIGNATURE) ? gcvTRUE : gcvFALSE);
}

static gctBOOL inline
_IsClientEGLBuffer(gcsWL_EGL_BUFFER *egl_buffer)
{
    return ((egl_buffer->wl_signature == WL_CLIENT_SIGNATURE) ? gcvTRUE : gcvFALSE);
}

static gctBOOL inline
_isValidWindow(struct wl_egl_window *window)
{
    struct wl_egl_window* win;

    wl_list_for_each(win, &WLEGLWindowList, link)
    {
        if(win == window && _IsClientWindow(window))
            return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL inline
_isValidEGLBuffer(gcsWL_EGL_BUFFER *buffer)
{
    gcsWL_EGL_BUFFER* egl_buffer;
    wl_list_for_each(egl_buffer, &WLEGLBufferList, link)
    {
        if(egl_buffer == buffer && _IsClientEGLBuffer(buffer))
            return gcvTRUE;
    }
    return gcvFALSE;
}

static void registerWindow(struct wl_egl_window *window)
{
    int i;
    struct wl_egl_window* wl_window = window;
    pthread_mutex_lock(&registerMutex);
    if(WLEGLWindowList.next == gcvNULL)
    {
        wl_list_init(&WLEGLWindowList);
        wl_list_init(&WLEGLBufferList);
    }
    wl_list_insert(&WLEGLWindowList, &wl_window->link);
    for(i = 0; i < wl_window->info->bufferCount; i++)
    {
        wl_list_insert(&WLEGLBufferList, &wl_window->backbuffers[i]->link);
    }
    pthread_mutex_unlock(&registerMutex);
}

static void unRegisterWindow(struct wl_egl_window *window)
{
    int i;
    struct wl_egl_window* wl_window = window;
    if(WLEGLWindowList.next == gcvNULL)
    {
        gcmPRINT("The WLEGLWindowList was not initialized \n");
        return;
    }
    pthread_mutex_lock(&registerMutex);
    wl_list_remove(&wl_window->link);
    for(i = 0; i < wl_window->info->bufferCount; i++)
    {
        wl_list_remove(&wl_window->backbuffers[i]->link);
    }
    pthread_mutex_unlock(&registerMutex);
}

static struct _FBDisplay *displayStack = gcvNULL;
static pthread_mutex_t displayMutex = PTHREAD_MUTEX_INITIALIZER;
#if defined(LINUX)
static void sig_handler(gctINT signo)
{
    if(hookSEGV == 0)
    {
        signal(SIGSEGV, sig_handler);
        hookSEGV = 1;
    }
    gcoOS_FreeEGLLibrary(gcvNULL);
    signal(signo, SIG_DFL);
    raise(signo);
}
#endif

static void
halOnExit(
    void
    )
{
#if defined(LINUX)
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
#endif
    gcoOS_FreeEGLLibrary(gcvNULL);
}


/*******************************************************************************
** Display. ********************************************************************
*/

gceSTATUS
gcoOS_GetDisplay(
    OUT HALNativeDisplayType * Display,
    IN gctPOINTER Context
    )
{

    gctINT index = 0;
    char* fbenv = getenv("FB_FRAMEBUFFER_0");
    if(fbenv != gcvNULL)
    {
        gctINT count = strlen(fbenv);
        if(count > 1)
        {
            index = atoi(&fbenv[count-1]);
            if(index < 0)
            {
                return  gcvSTATUS_INVALID_ARGUMENT;
            }
        }
    }

    return gcoOS_GetDisplayByIndex(index, Display, Context);
}

gceSTATUS
gcoOS_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT HALNativeDisplayType * Display,
    IN gctPOINTER Context
    )
{
    char *dev, *p;
    int i;
    char fbDevName[256];
    struct _FBDisplay* display = NULL;
    gceSTATUS status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmHEADER();

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
            *Display = (HALNativeDisplayType)display;
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

        p = getenv("FB_MULTI_BUFFER");
        if (p == gcvNULL)
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
            else if(display->multiBuffer > 3)
            {
                display->multiBuffer = 3;
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
            char const * const GALDeviceName[] =
            {
                "/dev/fb%d",
                "/dev/graphics/fb%d",
                gcvNULL
            };

            /* Create a handle to the device. */
            while ((display->file == -1) && GALDeviceName[i])
            {
                sprintf(fbDevName, GALDeviceName[i],DisplayIndex);
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
            gcmTRACE(gcvLEVEL_ERROR,
                "FBEGL Error reading variable framebuffer information.");

            break;
        }
        display->orgVarInfo = display->varInfo;

#if gcdENABLE_FB_PREFETCH
        /* Check if prefetch feature exists. */
        do
        {
            int prefetch = 0;
            display->fbPrefetch = gcvFALSE;

            /* Try to enable FSL fbdev tiled input. */
            p = getenv("GPU_VIV_EXT_RESOLVE");
            if ((p != gcvNULL) && (p[0] == '0'))
            {
                /* Disable resolve requested. */
                break;
            }

            if (ioctl(display->file, MXCFB_GET_PREFETCH, &prefetch) < 0)
            {
                break;
            }

            if (prefetch > 0)
            {
                /* Feature exists. */
                display->fbPrefetch = gcvTRUE;
                break;
            }

            /* Try to enable prefetch. */
            prefetch = 1;

            if (ioctl(display->file, MXCFB_SET_PREFETCH, &prefetch) < 0)
            {
                break;
            }

            if (ioctl(display->file, MXCFB_GET_PREFETCH, &prefetch) < 0)
            {
                break;
            }

            if(prefetch)
            {
                /* Feature exists. */
                display->fbPrefetch = gcvTRUE;
            }
        }
        while (0);
#else
        display->fbPrefetch = gcvFALSE;
#endif

        display->width  = display->varInfo.xres;
        display->height = display->varInfo.yres;

        if (display->fbPrefetch)
        {
            display->varInfo.xres = (display->varInfo.xres + 0x3F) & ~0x3F;
            display->varInfo.yres = (display->varInfo.yres + 0x3F) & ~0x3F;
        }
        else
        {
            display->varInfo.nonstd = 0;
        }
        for (i = display->multiBuffer; i >= 1; --i)
        {
            /* Try setting up multi buffering. */
            display->varInfo.yres_virtual = display->varInfo.yres * i;

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
        display->bpp    = display->varInfo.bits_per_pixel;

        if (display->multiBuffer > 1)
        {
            /* Calculate actual buffer count. */
            display->multiBuffer = display->varInfo.yres_virtual
                                 / display->varInfo.yres;
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

        /* Mark this as the compositor display */
        display->wl_signature = WL_COMPOSITOR_SIGNATURE;
        /* For the server, Context carries the server-side wl_display object */
        display->wl_display = Context;

        *Display = (HALNativeDisplayType)display;

        /* Add display into stack. */
        display->next = displayStack;
        displayStack = display;
        pthread_mutex_unlock(&displayMutex);

        gcmFOOTER_ARG("*Display=0x%x", *Display);
        return status;
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
        *Display = gcvNULL;
    }
    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_GetDisplayInfo(
    IN HALNativeDisplayType Display,
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

    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        display = (struct _FBDisplay*)Display;
    }
    else
    {
        display = gcvNULL;
    }

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
#if USE_SW_FB
        *Physical = ~0;
#else
        *Physical = display->physical;
#endif
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

gceSTATUS
gcoOS_SetWindowFormat(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
gcoOS_GetDisplayInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT halDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);

    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        display = (struct _FBDisplay*)Display;
    }
    else
    {
        struct wl_egl_window* wl_window = Window;

        DisplayInfo->width = wl_window->info->width;
        DisplayInfo->height = wl_window->info->height;
        DisplayInfo->stride = WL_DUMMY;
        DisplayInfo->bitsPerPixel = wl_window->info->bpp;
        DisplayInfo->logical = (gctPOINTER) WL_DUMMY;
        DisplayInfo->physical = WL_DUMMY;
        DisplayInfo->multiBuffer = wl_window->info->bufferCount;
        DisplayInfo->backBufferY = 0;
        DisplayInfo->flip = gcvTRUE;
        DisplayInfo->wrapFB = gcvFALSE;

        gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);

        return status;
    }

    /* Valid display? and structure size? */
    if ((display == gcvNULL) || (DisplayInfoSize != sizeof(halDISPLAY_INFO)))
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
    DisplayInfo->multiBuffer = display->multiBuffer;
    DisplayInfo->backBufferY = display->backBufferY;

    DisplayInfo->wrapFB = gcvTRUE;

    /* Success. */
    gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);
    return status;
}

static void wayland_release_pending_resource(void *data, struct wl_callback *callback, uint32_t time)
{
   gcoSURF surface = data;
   wl_callback_destroy(callback);
   gcoSURF_Unlock(surface, gcvNULL);
   gcoSURF_Destroy(surface);
}

static const struct wl_callback_listener release_buffer_listener = { wayland_release_pending_resource};

 gctBOOL
isRenderFinished(struct wl_egl_window *window)
{
    gctUINT i;
    if(window->display == gcvNULL)
    {
        return gcvTRUE;
    }
    for (i=0; i< window->info->bufferCount ; i++)
    {
        if (window->backbuffers[i]->frame_callback != gcvNULL)
        {
            wl_display_dispatch_queue(window->display->wl_display, window->display->wl_swap_queue);
            return gcvFALSE;
        }
    }

   return gcvTRUE;
}

void wait_for_the_frame_finish(struct wl_egl_window *window)
{
    gctINT32 counter = 0;
    if(window->reference != gcvNULL)
    {
        do
        {
            gcoOS_AtomIncrement(gcvNULL, window->reference, gcvNULL);
            gcoOS_AtomDecrement(gcvNULL, window->reference, &counter);
            /* Sleep for a while */
            gcoOS_Delay(gcvNULL, 10);
        }

        while(counter > 1 || isRenderFinished(window) != gcvTRUE);
    }
}

static void
gcoWL_DestroryBO(struct wl_egl_window *window)
{
     gctUINT i;

    wait_for_the_frame_finish(window);
    if (window != gcvNULL)
    {
        for (i=0; i< window->info->bufferCount ; i++)
        {
            if (window->backbuffers[i]->info.surface != gcvNULL)
            {

               if(window->display)
               {
                   struct wl_callback *callback;
                   callback = wl_display_sync(window->display->wl_display);
                   wl_callback_add_listener(callback, &release_buffer_listener,
                       window->backbuffers[i]->info.surface);
                   wl_proxy_set_queue((struct wl_proxy *) callback, window->display->wl_swap_queue);
                   window->backbuffers[i]->info.surface = gcvNULL;
               }
               else
               {
                   gcoSURF_Unlock(window->backbuffers[i]->info.surface, gcvNULL);
                   gcoSURF_Destroy(window->backbuffers[i]->info.surface);
               }

            }
        }
    }
}

static gceSTATUS
gcoWL_CreateBO(struct wl_egl_window* window, int dx, int dy, int width, int height)
{
     gceSTATUS status = gcvSTATUS_OK;
     gctUINT i;

    gcmASSERT(window);
    do
    {
        gcsSURF_FORMAT_INFO_PTR renderTargetInfo[2];
        gceSURF_TYPE surfaceType = gcvSURF_BITMAP;
        gceSURF_FORMAT resolveFormat = gcvSURF_UNKNOWN;
        gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
        window->info->dx     = dx;
        window->info->dy     = dy;
        window->info->width  = width;
        window->info->height = height;
        /* Get hardware type. */
        gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &currentType));
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

        if (window->noResolve)
        {
            surfaceType = gcvSURF_TEXTURE;
        }
        /* Set the window surface format save as the config requested */
        window->info->format = (gceSURF_FORMAT) gcoOS_GetPLSValue(gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO);

        gcmONERROR(
            gcoSURF_QueryFormat(
                window->info->format,
                renderTargetInfo
            ));

        window->info->bpp = renderTargetInfo[0]->bitsPerPixel;

        gcmONERROR(
            gcoTEXTURE_GetClosestFormat(gcvNULL,
                                        window->info->format,
                                        &resolveFormat));
        window->info->format = resolveFormat;

        for (i=0; i< window->info->bufferCount ; i++)
        {
            gcmONERROR(
                gcoSURF_Construct(
                                gcvNULL,
                                width,
                                height,
                                1,
                                surfaceType,
                                resolveFormat,
                                gcvPOOL_DEFAULT,
                                &window->backbuffers[i]->info.surface
                                ));
            if(surfaceType != gcvSURF_BITMAP)
            {
                gcmONERROR(
                    gcoSURF_SetFlags(
                        window->backbuffers[i]->info.surface,
                        gcvSURF_FLAG_CONTENT_YINVERTED,
                        gcvTRUE));
            }
/*
            gcmONERROR(
                gcoSURF_SetOrientation(
                                    window->backbuffers[i]->info.surface,
                                    gcvORIENTATION_BOTTOM_TOP));
*/
            gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL
                    ));

            gcmONERROR(
                gcoSURF_GetAlignedSize(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL,
                    &window->backbuffers[i]->info.stride
                    ));

            gcmONERROR(
               gcoSURF_QueryVidMemNode(
                    window->backbuffers[i]->info.surface,
                    (gctUINT32 *)&window->backbuffers[i]->info.node,
                    &window->backbuffers[i]->info.pool,
                    &window->backbuffers[i]->info.bytes
                    ));

            gcmONERROR(
                gcoHAL_NameVideoMemory((gctUINT32)window->backbuffers[i]->info.node,
                                       (gctUINT32 *)&window->backbuffers[i]->info.node));

            window->backbuffers[i]->info.width = window->info->width;
            window->backbuffers[i]->info.height = window->info->height;
            window->backbuffers[i]->info.format = resolveFormat;
            window->backbuffers[i]->info.type = surfaceType;
            window->backbuffers[i]->info.invalidate = gcvTRUE;
            window->backbuffers[i]->frame_callback = gcvNULL;
            window->backbuffers[i]->info.locked = gcvFALSE;
            gcmTRACE(gcvLEVEL_VERBOSE, "Surface %d (%p): width=%d, height=%d, format=%d, stride=%d, node=%p, pool=%d, bytes=%d, calc=%d",
                        i, window->backbuffers[i]->info.surface,
                        window->info->width, window->info->height,
                        window->backbuffers[i]->info.format,
                        window->backbuffers[i]->info.stride,
                        window->backbuffers[i]->info.node,
                        window->backbuffers[i]->info.pool,
                        window->backbuffers[i]->info.bytes,
                        window->backbuffers[i]->info.stride*window->info->width
                        );
        }
        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, currentType);
    }
    while (gcvFALSE);
    return status;
OnError:
    gcoWL_DestroryBO(window);
    gcoOS_FreeMemory(gcvNULL, window);
    window = gcvNULL;
    return gcvSTATUS_INVALID_ARGUMENT;
}


static gceSTATUS
gcoWL_ResizeBO(struct wl_egl_window* window)
{
     gceSTATUS status = gcvSTATUS_OK;
     gctUINT i;

    gcmASSERT(window);
    do
    {
        gceSURF_TYPE surfaceType = gcvSURF_BITMAP;
        gceSURF_FORMAT resolveFormat = window->info->format;
        gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
        struct wl_callback *callback;
        i = window->info->current;

        callback = wl_display_sync(window->display->wl_display);
        wl_callback_add_listener(callback, &release_buffer_listener,
                window->backbuffers[i]->info.surface);
        wl_proxy_set_queue((struct wl_proxy *) callback, window->display->wl_swap_queue);
        window->backbuffers[i]->info.surface = gcvNULL;

        /* Get hardware type. */
        gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &currentType));
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

        if (window->noResolve)
        {
            surfaceType = gcvSURF_TEXTURE;
        }

        /* Query current hardware type. */
        gcmONERROR(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmONERROR(
            gcoSURF_Construct(
                      gcvNULL,
                      window->info->width,
                      window->info->height,
                      1,
                      surfaceType,
                      resolveFormat,
                      gcvPOOL_DEFAULT,
                      &window->backbuffers[i]->info.surface
                      ));

        if(surfaceType != gcvSURF_BITMAP)
        {
            gcmONERROR(
                    gcoSURF_SetFlags(
                    window->backbuffers[i]->info.surface,
                    gcvSURF_FLAG_CONTENT_YINVERTED,
                    gcvTRUE));
        }

        gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL
                    ));

        gcmONERROR(
                gcoSURF_GetAlignedSize(
                    window->backbuffers[i]->info.surface,
                    gcvNULL,
                    gcvNULL,
                    &window->backbuffers[i]->info.stride
                    ));

        gcmONERROR(
               gcoSURF_QueryVidMemNode(
                    window->backbuffers[i]->info.surface,
                    (gctUINT32 *)&window->backbuffers[i]->info.node,
                    &window->backbuffers[i]->info.pool,
                    &window->backbuffers[i]->info.bytes
                    ));

        gcmONERROR(
                gcoHAL_NameVideoMemory((gctUINT32)window->backbuffers[i]->info.node,
                                       (gctUINT32 *)&window->backbuffers[i]->info.node));

        window->backbuffers[i]->info.width = window->info->width;
        window->backbuffers[i]->info.height = window->info->height;
        window->backbuffers[i]->info.invalidate = gcvTRUE;
        window->backbuffers[i]->frame_callback = gcvNULL;

        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, currentType);
    }
    while (gcvFALSE);
    return status;
OnError:
    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
gcoOS_GetDisplayVirtual(
    IN HALNativeDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x", Display);

    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        display = (struct _FBDisplay*)Display;
    }
    else
    {
        display = gcvNULL;
    }

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

    /* Compute number of buffers. */
    display->multiBuffer = display->varInfo.yres_virtual / display->varInfo.yres;

    /* Move to off-screen memory. */
    display->backBufferY = display->varInfo.yoffset + display->varInfo.yres;
    if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
    {
        /* Warp around. */
        display->backBufferY = 0;
    }

    /* Save size of virtual buffer. */
    *Width  = display->varInfo.xres_virtual;
    *Height = display->varInfo.yres_virtual;

    /* Success. */
    gcmFOOTER_ARG("*Width=%d *Height=%d", *Width, *Height);
    return status;
}

gceSTATUS
gcoOS_GetDisplayBackbuffer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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

    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        display = (struct _FBDisplay*)Display;
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
        display->backBufferY += display->varInfo.yres;
        if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
        {
            /* Wrap around. */
            display->backBufferY = 0;
        }

        pthread_mutex_unlock(&(display->condMutex));
    }

    /* Success. */
    gcmFOOTER_ARG("*Offset=%u *X=%d *Y=%d", *Offset, *X, *Y);
    return status;
}

static void
wl_buffer_release(void *data, struct wl_buffer *buffer)
{
    gcsWL_EGL_BUFFER* egl_buffer = data;
    pthread_mutex_lock(&registerMutex);
    if (_isValidEGLBuffer(egl_buffer))
    {
        egl_buffer->info.locked = gcvFALSE;
    }
    pthread_mutex_unlock(&registerMutex);
}

static struct wl_buffer_listener wl_buffer_listener = {
   wl_buffer_release
};

static void
gcoWL_FrameCallback(void *data, struct wl_callback *callback, uint32_t time)
{
    gcsWL_EGL_BUFFER* egl_buffer = data;
    pthread_mutex_lock(&registerMutex);
    if (_isValidEGLBuffer(egl_buffer))
    {
        egl_buffer->frame_callback = NULL;
        wl_callback_destroy(callback);
    }
    pthread_mutex_unlock(&registerMutex);
}

static const struct wl_callback_listener gcsWL_FRAME_LISTENER = {
    gcoWL_FrameCallback
};


gceSTATUS
gcoOS_SetDisplayVirtual(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gctINT swapInterval;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        display = (struct _FBDisplay*)Display;
    }
    else
    {
        display = gcvNULL;

        gcmFOOTER();
        return status;
    }

    if (display == gcvNULL)
    {
        /* Invalid display pointer. */
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
                ioctl(display->file, FBIO_WAITFORVSYNC, (void *)0);
            }

            /* Set display offset. */
            display->varInfo.xoffset  = X;
            display->varInfo.yoffset  = Y;
            display->varInfo.activate = FB_ACTIVATE_VBL;
            ioctl(display->file, FBIOPAN_DISPLAY, &(display->varInfo));
            ioctl(display->file, MXCFB_WAIT_FOR_VSYNC, NULL);

            pthread_cond_broadcast(&(display->cond));
            pthread_mutex_unlock(&(display->condMutex));
        }
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoOS_SetDisplayVirtualEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    if (Display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvFALSE)
    {
        /* Client */
        struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
        gcsWL_EGL_BUFFER* egl_buffer = (gcsWL_EGL_BUFFER*)Context;

        struct wl_buffer* wl_buffer = gcvNULL;
        gcsWL_EGL_DISPLAY* display = gcvNULL;

        int ret = 0;
        int i   = 0;

        if(_isValidWindow(Window) == gcvFALSE)
        {
            return gcvSTATUS_FALSE;
        }
        wl_buffer = egl_buffer->wl_buffer;
        display = wl_window->display;
        wl_display_dispatch_queue_pending(display->wl_display, display->wl_queue);

        /* wait for frame callback */
        for(i = 0; i < wl_window->info->bufferCount; i++)
        {
            while (wl_window->backbuffers[i]->frame_callback && ret >= 0)
            {
                ret = wl_display_dispatch_queue(display->wl_display, display->wl_swap_queue);
            }
        }
        if(ret < 0)
        {
            gcoOS_AtomDecrement(gcvNULL, wl_window->reference, gcvNULL);
            return gcvSTATUS_FALSE;
        }

        if(wl_window->reference == gcvNULL)
        {
             return status;
         }

        if(display->swapInterval > 0)
        {
            gctINT swapInterval = display->swapInterval;
            swapInterval--;
            /* wait for swap interval  * vsync */
            while(swapInterval--)
            {
                ioctl(display->file, FBIO_WAITFORVSYNC, (void *)0);
            }
            egl_buffer->frame_callback = wl_surface_frame(wl_window->surface);
            wl_callback_add_listener(egl_buffer->frame_callback, &gcsWL_FRAME_LISTENER, egl_buffer);
            wl_proxy_set_queue((struct wl_proxy *) egl_buffer->frame_callback, display->wl_swap_queue);
        }
        wl_surface_attach(wl_window->surface, wl_buffer, wl_window->info->dx, wl_window->info->dy);
        wl_window->attached_width  = wl_window->info->width;
        wl_window->attached_height = wl_window->info->height;
        wl_window->info->dx = 0;
        wl_window->info->dy = 0;
        wl_surface_damage(wl_window->surface, 0, 0,
                            wl_window->info->width, wl_window->info->height);
        wl_surface_commit(wl_window->surface);
        /* If we're not waiting for a frame callback then we'll at least throttle
         * to a sync callback so that we always give a chance for the compositor to
         * handle the commit and send a release event before checking for a free
         * buffer */
        if(egl_buffer->frame_callback == gcvNULL)
        {
            egl_buffer->frame_callback = wl_display_sync(display->wl_display);
            wl_callback_add_listener(egl_buffer->frame_callback, &gcsWL_FRAME_LISTENER, egl_buffer);
            wl_proxy_set_queue((struct wl_proxy *) egl_buffer->frame_callback, display->wl_swap_queue);
        }
        wl_display_flush(display->wl_display);
        if(wl_window->reference != gcvNULL )
        {
            gcoOS_AtomDecrement(gcvNULL, wl_window->reference, gcvNULL);
        }
        return status;
    }

    return gcoOS_SetDisplayVirtual(Display, Window, Offset, X, Y);
}

gceSTATUS
gcoOS_SetSwapInterval(
    IN HALNativeDisplayType Display,
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
gcoOS_SetSwapIntervalEx(
    IN HALNativeDisplayType Display,
    IN gctINT Interval,
    IN gctPOINTER localDisplay
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT interval;
    if (Display == gcvNULL)
    {
        /* Invalid display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
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

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvFALSE)
    {
        gcsWL_EGL_DISPLAY* egl_display = ((gcsWL_EGL_DISPLAY*) localDisplay);
        egl_display->swapInterval = Interval;
        return status;
    }

    return gcoOS_SetSwapInterval(Display, Interval);
}

gceSTATUS
gcoOS_GetSwapInterval(
    IN HALNativeDisplayType Display,
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
gcoOS_DestroyDisplay(
    IN HALNativeDisplayType Display
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

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
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
    else
    {
        display = gcvNULL;
    }

    pthread_mutex_unlock(&displayMutex);

    if (display != NULL)
    {
        if (display->memory != NULL)
        {
            munmap(display->memory, display->size);
            display->memory = NULL;
        }

        ioctl(display->file, FBIOPUT_VSCREENINFO, &(display->orgVarInfo));

        if (display->file >= 0)
        {
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
gcoOS_DisplayBufferRegions(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
gcoOS_CreateWindow(
    IN HALNativeDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT HALNativeWindowType * Window
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    struct _FBDisplay * display;
    struct _FBWindow *  window;
    gctINT              ignoreDisplaySize = 0;
    gctCHAR *           p;

    gcmHEADER_ARG("Display=%p X=%d Y=%d Width=%d Height=%d",
                  Display, X, Y, Width, Height);

    if (Display != gcvNULL && _IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        if (Display == gcvNULL)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }
        display = (struct _FBDisplay*)Display;
    }
    else
    {
        gctUINT i;
        struct wl_egl_window* wl_window = (struct wl_egl_window*)(*Window);

        gcoOS_AllocateMemory(gcvNULL, sizeof(struct _gcsWL_EGL_WINDOW_INFO),
                            (gctPOINTER) &wl_window->info);
        gcoOS_ZeroMemory( wl_window->info, sizeof(struct _gcsWL_EGL_WINDOW_INFO));

        wl_window->info->bufferCount =  WL_EGL_NUM_BACKBUFFERS;

        gcoOS_AtomConstruct(gcvNULL, &wl_window->reference);
        wl_window->wl_signature = WL_CLIENT_SIGNATURE;
        pthread_mutex_init(&(wl_window->window_mutex), gcvNULL);

        p = getenv("GPU_VIV_WL_MULTI_BUFFER");
        if (p != gcvNULL)
        {
            int bufferCount = atoi(p);
            if (bufferCount > 0 && bufferCount <= WL_EGL_MAX_NUM_BACKBUFFERS)
            {
                wl_window->info->bufferCount = bufferCount;
            }
        }
        gcoOS_AllocateMemory(gcvNULL, wl_window->info->bufferCount * sizeof(struct _gcsWL_EGL_BUFFER *),
                    (gctPOINTER) &wl_window->backbuffers);
        for(i=0; i< wl_window->info->bufferCount; i++)
        {
            gcoOS_AllocateMemory(gcvNULL, sizeof(struct _gcsWL_EGL_BUFFER),
                    (gctPOINTER) &wl_window->backbuffers[i]);
            gcoOS_ZeroMemory( wl_window->backbuffers[i], sizeof(struct _gcsWL_EGL_BUFFER));
            wl_window->backbuffers[i]->wl_signature = WL_CLIENT_SIGNATURE;
        }
        registerWindow(wl_window);
        status = gcoWL_CreateBO((struct wl_egl_window*)(*Window), 0, 0, Width, Height);
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
        int err;
        int nonstd = 0;
        int extResolve = 1;
        struct fb_var_screeninfo varInfo;

        if (!display->fbPrefetch)
        {
            /* No prefetch in hardware. */
            break;
        }

        if ((X != 0) || (Y != 0) ||
            (Width != display->width) || (Height != display->height))
        {
            /* Not full screen, can not enable. */
            extResolve = 0;
        }

        if (extResolve)
        {
            switch (display->bpp)
            {
            case 16:
                nonstd = IPU_PIX_FMT_GPU16_SRT;
                break;
            case 32:
                nonstd = IPU_PIX_FMT_GPU32_SRT;
                break;
            default:
                /* Unknown pixel format. */
                extResolve = 0;
                break;
            }
        }

        if (!extResolve)
        {
            display->varInfo.xres = display->width;
            display->varInfo.yres = display->height;
        }

        varInfo        = display->varInfo;
        varInfo.nonstd = nonstd;

        /* Disable split. */
        err = ioctl(display->file, FBIOPUT_VSCREENINFO, &varInfo);

        if (err < 0)
        {
            /* Not changed. */
            break;
        }

        /* Prefetch mode changed. */
        display->varInfo.nonstd = nonstd;
    }
    while (0);

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

        /* Mark this as the compositor window */
        window->wl_signature = WL_COMPOSITOR_SIGNATURE;

        *Window = (HALNativeWindowType) window;
    }
    while (0);

    gcmFOOTER_ARG("*Window=0x%x", *Window);
    return status;
}

gceSTATUS
gcoOS_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN HALNativeWindowType Window,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcmHEADER_ARG("localDisplay=%p Width=%d Height=%d",
                  localDisplay, Width, Height);
    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if(_IsCompositorWindow((struct _FBWindow*)Window))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }
    else
    {
        struct wl_egl_window* window = (struct wl_egl_window*)Window;
        /* Nothing to do if window size if same. */
        if(window->info->width != Width || window->info->height != Height)
        {

            wait_for_the_frame_finish(window);
            window->info->width = Width;
            window->info->height = Height;
        }
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_GetWindowInfo(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBWindow* window = gcvNULL;
    struct wl_egl_window* wl_window = gcvNULL;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorWindow((struct _FBWindow*)Window) == gcvTRUE)
    {
        window = (struct _FBWindow*)Window;
        wl_window = gcvNULL;
    }
    else
    {
        window = gcvNULL;
        wl_window = (struct wl_egl_window*) Window;
    }

    gcmASSERT((window != gcvNULL) || (wl_window != gcvNULL));

    if (window != gcvNULL)
    { /* Compositor window */
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
    }
    else
    { /* Client window */
        if (X != gcvNULL)
        {
            *X = 0;
        }

        if (Y != gcvNULL)
        {
            *Y = 0;
        }

        if (Width != gcvNULL)
        {
            *Width = wl_window->info->width;
        }

        if (Height != gcvNULL)
        {
            *Height = wl_window->info->height;
        }

        if (BitsPerPixel != gcvNULL)
        {
            *BitsPerPixel = wl_window->info->bpp;
        }

        if (Offset != gcvNULL)
        {
            *Offset = 0;
        }
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcoOS_DestroyWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorWindow((struct _FBWindow*)Window) == gcvTRUE)
    {
        struct _FBWindow* window;
        window = (struct _FBWindow*)Window;
        if (window != gcvNULL)
        {
            free(window);
        }
    }
    else
    {
        struct wl_egl_window* window = (struct wl_egl_window*)Window;
        gctUINT i;
        gcoWL_DestroryBO(window);
        unRegisterWindow(window);
        pthread_mutex_lock(&(window->window_mutex));
        for(i=0; i< window->info->bufferCount; i++)
        {
            if (window->backbuffers[i]->wl_buffer != gcvNULL)
            {
                wl_buffer_destroy(window->backbuffers[i]->wl_buffer);
                window->backbuffers[i]->wl_buffer = gcvNULL;
            }



            gcoOS_FreeMemory(gcvNULL, window->backbuffers[i]);
            window->backbuffers[i] = gcvNULL;
        }

        pthread_mutex_unlock(&(window->window_mutex));
        pthread_mutex_destroy(&(window->window_mutex));
        gcoOS_FreeMemory(gcvNULL, window->backbuffers);
        gcoOS_FreeMemory(gcvNULL, window->info);
        window->info = gcvNULL;
        window->backbuffers = gcvNULL;
        window->wl_signature = 0;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_DrawImage(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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


    if (Window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (_IsCompositorWindow((struct _FBWindow*)Window) == gcvTRUE)
    {
        window = (struct _FBWindow*)Window;
    }
    else
    {
        window = gcvNULL;
    }

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
gcoOS_GetImage(
    IN HALNativeWindowType Window,
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
gcoOS_CreatePixmap(
    IN HALNativeDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    OUT HALNativePixmapType * Pixmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
#if !gcdUSE_PIXMAP_SURFACE
    gctINT alignedWidth, alignedHeight;
#endif
    struct _FBPixmap* pixmap;
#if gcdUSE_PIXMAP_SURFACE
    gceSURF_FORMAT format;
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
            &pixmap->bits
        ));

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

        *Pixmap = (HALNativePixmapType)pixmap;
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
gcoOS_GetPixmapInfo(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
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
gcoOS_DestroyPixmap(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap
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
gcoOS_DrawPixmap(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
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
gcoOS_LoadEGLLibrary(
    OUT gctHANDLE * Handle
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    atexit(halOnExit);

    /* Initialize the global data structure. */
    file    = -1;
    mice    = -1;

#if defined(ANDROID)
    status = gcoOS_LoadLibrary(gcvNULL, "libEGL_VIVANTE.so", Handle);
#else
    status = gcoOS_LoadLibrary(gcvNULL, "libEGL.so", Handle);
#endif

#if defined(LINUX)
    signal(SIGINT,  sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
#endif

    return status;
}

gceSTATUS
gcoOS_FreeEGLLibrary(
    IN gctHANDLE Handle
    )
{
#if gcdSTATIC_LINK
    return gcvSTATUS_OK;
#else
    if (Handle != gcvNULL)
    {
        void (*fini)(void);
        int *fptr;
        fptr = (int *)dlsym(Handle, "__fini");
        fini = *((void (**)(void))&fptr);

        if (fini != gcvNULL)
        {
            fini();
        }

        gcoOS_FreeLibrary(gcvNULL, Handle);
    }
    if (file > 0)
    {
        ioctl(file, KDSKBMODE, mode);
        tcsetattr(file, TCSANOW, &tty);

        ioctl(file, KDSETMODE, KD_TEXT);

        if (active != -1)
        {
            ioctl(file, VT_ACTIVATE, active);
            ioctl(file, VT_WAITACTIVE, active);
        }

        close(file);

        if (uid != -1)
        {
            ignore = chown(name, uid, gid);
        }
    }

    if (mice > 0)
    {
        close(mice);
    }
    file    = -1;
    mice    = -1;
    return gcvSTATUS_OK;
#endif
}

gceSTATUS
gcoOS_ShowWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
#if gcdUSE_INPUT_DEVICE
    int i, fd=0, term = -1;
    struct stat st;
    struct vt_stat v;
    struct termios t;

    if (file != -1)
    {
        return gcvSTATUS_OK;
    }

    do
    {
        /* Try opening the TTY device. */
        for (i = 0; i < 2; ++i)
        {
            static const char * dev[] = { "/dev/tty0", "/dev/vc/0" };

            /* Open the TTY device. */
            fd = open(dev[i], O_RDWR, 0);

            if (fd >= 0)
            {
                /* Break on success. */
                break;
            }
        }

        if (fd < 0)
        {
            /* Break when TTY device cannot be opened. */
            break;
        }

        if ((ioctl(fd, VT_OPENQRY, &term) < 0) || (term == -1))
        {
            break;
        }

        close(fd);
        fd = -1;

        for (i = 0; i < 2; ++i)
        {
            static const char * dev[] = { "/dev/tty%d", "/dev/vc/%d" };

            sprintf(name, dev[i], term);
            file = open(name, O_RDWR | O_NONBLOCK, 0);

            if (file >= 0)
            {
                break;
            }
        }

        if (file < 0)
        {
            break;
        }

        if (stat(name, &st) == 0)
        {
            uid = st.st_uid;
            gid = st.st_gid;

            ignore = chown(name, getuid(), getgid());
        }
        else
        {
            uid = -1;
        }

        if (ioctl(file, VT_GETSTATE, &v) >= 0)
        {
            active = v.v_active;
        }

        ioctl(file, VT_ACTIVATE, term);
        ioctl(file, VT_WAITACTIVE, term);
        ioctl(file, KDSETMODE, KD_GRAPHICS);

        /* Check FB config since it may be affected by VT_ACTIVATE. */
        {
            struct _FBDisplay* display;

            if (Display == gcvNULL)
            {
                break;
            }

            if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
            {
                display = (struct _FBDisplay*)Display;
            }
            else
            {
                display = gcvNULL;
                break;
            }

            if (ioctl(display->file, FBIOPUT_VSCREENINFO, &display->varInfo) >= 0)
            {
                ioctl(display->file, FBIOGET_VSCREENINFO, &(display->varInfo));
                if (display->varInfo.yres_virtual < display->varInfo.yres * display->multiBuffer)
                {   /* Fallback to single buffer. */
                    display->multiBuffer = 1;
                }
            }
            else
            {   /* Fallback to single buffer. */
                display->multiBuffer = 1;
            }
        }

        ioctl(file, KDGKBMODE, &mode);
        tcgetattr(file, &tty);

        ioctl(file, KDSKBMODE, K_RAW);

        t = tty;
        t.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
        t.c_oflag = 0;
        t.c_cflag = CREAD | CS8;
        t.c_lflag = 0;
        t.c_cc[VTIME] = 0;
        t.c_cc[VMIN] = 1;
        tcsetattr(file, TCSANOW, &t);

        if (mice == -1)
        {
            mice = open("/dev/input/mice", O_RDONLY | O_NONBLOCK, 0);
        }

        return gcvSTATUS_OK;
    }
    while (0);

    if (fd >= 0)
    {
        close(fd);
    }

    return gcvSTATUS_NOT_SUPPORTED;
#else
    return gcvSTATUS_OK;
#endif
}

gceSTATUS
gcoOS_HideWindow(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    if (file < 0)
    {
        return gcvSTATUS_OK;
    }

    ioctl(file, KDSKBMODE, mode);
    tcsetattr(file, TCSANOW, &tty);

    ioctl(file, KDSETMODE, KD_TEXT);

    if (active != -1)
    {
        ioctl(file, VT_ACTIVATE,   active);
        ioctl(file, VT_WAITACTIVE, active);
    }

    close(file);
    file = -1;

    if (uid != -1)
    {
        ignore = chown(name, uid, gid);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_SetWindowTitle(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctCONST_STRING Title
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CapturePointer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_GetEvent(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    OUT halEvent * Event
    )
{
    static int prefix = 0;
    unsigned char buffer;

    signed char mouse[3];
    static int x, y;
    static char left, right, middle;

    if ((Window == gcvNULL) || (Event == gcvNULL))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (file >= 0)
    {
        while (read(file, &buffer, 1) == 1)
        {
            halKeys scancode;

            if ((buffer == 0xE0) || (buffer == 0xE1))
            {
                prefix = buffer;
                continue;
            }

            if (prefix)
            {
                scancode = keys[buffer & 0x7F].extended;
                prefix = 0;
            }
            else
            {
                scancode = keys[buffer & 0x7F].normal;
            }

            if (scancode == HAL_UNKNOWN)
            {
                continue;
            }

            Event->type              = HAL_KEYBOARD;
            Event->data.keyboard.scancode = scancode;
            Event->data.keyboard.pressed  = (buffer < 0x80);
            Event->data.keyboard.key      = (  (scancode < HAL_SPACE)
                || (scancode >= HAL_F1)
                )
                ? 0
                : (char) scancode;
            return gcvSTATUS_OK;
        }
    }

    if (mice >= 0)
    {
        if (read(mice, mouse, 3) == 3)
        {
            char l, m, r;

            x += mouse[1];
            y -= mouse[2];

            /*
            x = (x < 0) ? 0 : x;
            x = (x > Window->display->width) ? Window->display->width : x;

            y = (y < 0) ? 0 : y;
            y = (y > Window->display->height) ? Window->display->height : y;
            */

            l = mouse[0] & 0x01;
            r = mouse[0] & 0x02;
            m = mouse[0] & 0x04;

            if ((l ^ left) || (r ^ right) || (m ^ middle))
            {
                Event->type                 = HAL_BUTTON;
                Event->data.button.left     = left      = l;
                Event->data.button.right    = right     = r;
                Event->data.button.middle   = middle    = m;
                Event->data.button.x        = x;
                Event->data.button.y        = y;
            }
            else
            {
                Event->type                 = HAL_POINTER;
                Event->data.pointer.x       = x;
                Event->data.pointer.y       = y;
            }

            return gcvSTATUS_OK;
        }
    }

    return gcvSTATUS_NOT_FOUND;
}

gceSTATUS
gcoOS_CreateClientBuffer(
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
gcoOS_GetClientBufferInfo(
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
gcoOS_DestroyClientBuffer(
    IN gctPOINTER ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static void
gcoWL_DestroyBuffer(struct wl_resource *resource)
{
    gcsWL_VIV_BUFFER *buffer = resource->data;

    if (buffer != gcvNULL)
    {
        gcoSURF surface = buffer->surface;

        if (surface)
        {
            gcoSURF_Unlock(surface, gcvNULL);
            gcoSURF_Destroy(surface);
        }

        gcoOS_FreeMemory(gcvNULL, buffer);
    }
}

static void
gcoWL_BufferDestroy(struct wl_client *client, struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct wl_buffer_interface gcsWL_BUFFER_IMPLEMENTATION = {
    gcoWL_BufferDestroy
};

static void
gcoWL_ImportBuffer(struct wl_client *client,
                      struct wl_resource *resource,
                      uint32_t id,
                      uint32_t width,
                      uint32_t height,
                      uint32_t stride,
                      int32_t format,
                      int32_t type,
                      int32_t node,
                      int32_t pool,
                      uint32_t bytes
                      )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF surface = gcvNULL;
    gcsWL_VIV_BUFFER * wl_viv_buffer = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gcmTRACE(gcvLEVEL_VERBOSE, "Ghost buffer width=%d, height=%d, stride=%d, format=%d, node=%p, pool=%d, bytes=%d",
                    width, height, stride, format, node, pool, bytes);
    gcoHAL_GetHardwareType(gcvNULL, &currentType);
    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
    gcmONERROR(
        gcoSURF_Construct(gcvNULL,
                          (gctUINT) width,
                          (gctUINT) height,
                          1,
                          type | gcvSURF_NO_VIDMEM,
                          (gceSURF_FORMAT) format,
                          (gcePOOL) pool,
                          &surface));

    /* Save surface info. */
    surface->node.u.normal.node = (gctUINT32) node;
    surface->node.pool          = (gcePOOL) pool;
    surface->node.size          = (gctSIZE_T) bytes;
/*
    gcmONERROR(
        gcoSURF_SetOrientation(surface,
                               gcvORIENTATION_BOTTOM_TOP));
*/

    gcmONERROR(gcoHAL_ImportVideoMemory(
        (gctUINT32)node, (gctUINT32 *)&surface->node.u.normal.node));

    /* Lock once as it's done in gcoSURF_Construct with vidmem. */
    gcmONERROR(
        gcoSURF_Lock(surface,
                     gcvNULL,
                     gcvNULL));

    gcmONERROR(
        gcoOS_AllocateMemory(gcvNULL,
                             gcmSIZEOF(*wl_viv_buffer),
                             (gctPOINTER *) &wl_viv_buffer));

    gcoOS_ZeroMemory(wl_viv_buffer,
                         gcmSIZEOF(*wl_viv_buffer));

    wl_viv_buffer->surface = surface;
    wl_viv_buffer->width = width;
    wl_viv_buffer->height = height;

    wl_viv_buffer->wl_buffer = wl_resource_create(client, &wl_buffer_interface, 1, id);
    if (!wl_viv_buffer->wl_buffer) {
        wl_resource_post_no_memory(resource);
        free(wl_viv_buffer);
        return;
    }
    wl_resource_set_implementation(wl_viv_buffer->wl_buffer,
                       &gcsWL_BUFFER_IMPLEMENTATION,
                       wl_viv_buffer, gcoWL_DestroyBuffer);
    gcoHAL_SetHardwareType(gcvNULL, currentType);
    return;

OnError:
    wl_resource_post_no_memory(resource);
    if (surface)
    {
        gcoSURF_Unlock(surface, gcvNULL);
        gcoSURF_Destroy(surface);
        surface = gcvNULL;
    }

    if (wl_viv_buffer != gcvNULL)
    {
        gcoOS_FreeMemory(gcvNULL, wl_viv_buffer);
        wl_viv_buffer = gcvNULL;
    }

    return;
}

struct wl_viv_interface gcsWL_VIV_IMPLEMENTATION = {
    (void *) gcoWL_ImportBuffer
};

static void
gcoWL_BindWLViv(struct wl_client *client,
         void *data, uint32_t version, uint32_t id)
{
    struct wl_resource *resource;
    resource = wl_resource_create(client, &wl_viv_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &gcsWL_VIV_IMPLEMENTATION, data, NULL);
}

gceSTATUS
gcoOS_InitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER Context = *localDisplay;
    gcsWL_LOCAL_DISPLAY* wl_localDisplay = (gcsWL_LOCAL_DISPLAY*)Context;

    if ((_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
         || (Context && _IsWaylandLocalDisplay(wl_localDisplay)))
    {
        struct wl_display *display = gcvNULL;
        if(wl_localDisplay != gcvNULL && Context && _IsWaylandLocalDisplay(wl_localDisplay))
        {
            display = (struct wl_display*) wl_localDisplay->localInfo;
        }
        else /*Deprecated path */
        {
            display = ((struct _FBDisplay*) Display)->wl_display;
        }
        if(display != NULL)
        {
            *localDisplay = wl_global_create(display,
                             &wl_viv_interface, 1, gcvNULL, gcoWL_BindWLViv);
        }
        else
        {
            *localDisplay = gcvNULL;
        }
    }
    else
    {
        struct wl_display *display = Display;
        gcsWL_EGL_DISPLAY *egl_display = gcvNULL;
        char *dev;
        char fbDevName[256];
        int  DisplayIndex = 0;

        egl_display = gcoWL_GetDisplay(display);
        egl_display->swapInterval = 1;
        egl_display->file = -1;
        sprintf(fbDevName,"FB_FRAMEBUFFER_%d",DisplayIndex);
        dev = getenv(fbDevName);
        if (dev != NULL)
        {
            egl_display->file = open(dev, O_RDWR);
        }
        if (egl_display->file < 0)
        {
            unsigned char i = 0;
            char const * const GALDeviceName[] =
            {
                "/dev/fb%d",
                "/dev/graphics/fb%d",
                gcvNULL
            };

            /* Create a handle to the device. */
            while ((egl_display->file == -1) && GALDeviceName[i])
            {
                sprintf(fbDevName, GALDeviceName[i],DisplayIndex);
                egl_display->file = open(fbDevName, O_RDWR);
                i++;
            }
        }

        if (egl_display->file < 0)
        {
            return gcvSTATUS_NOT_FOUND;
        }

        *localDisplay = egl_display;

        if (egl_display == gcvNULL)
        {
            return gcvSTATUS_NOT_FOUND;
        }
    }

    return status;
}

gceSTATUS
gcoOS_DeinitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;


    if(localDisplay && (*localDisplay == gcvNULL))
    {
        /*The localDisplay was not initialized in FB app */
        return status;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        if ((((struct _FBDisplay*) Display)->wl_display) != gcvNULL)
        {
            wl_global_destroy(*localDisplay);
        }
    }
    else
    {
        gcsWL_EGL_DISPLAY *egl_display = *localDisplay;
        struct wl_egl_window* wl_window;
        if (egl_display->file >= 0)
        {
            close(egl_display->file);
            egl_display->file = -1;
        }
        if(WLEGLWindowList.next != gcvNULL)
        {
            pthread_mutex_lock(&registerMutex);
            wl_list_for_each(wl_window, &WLEGLWindowList, link)
            {
                if(_IsClientWindow(wl_window))
                {
                    if(wl_window->display == egl_display)
                         wl_window->display = gcvNULL;
                }
            }
            pthread_mutex_unlock(&registerMutex);
        }
        gcoWL_ReleaseDisplay(*localDisplay);
    }

    return status;
}


gceSTATUS
gcoOS_GetDisplayInfoEx2(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT halDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcoOS_GetDisplayInfoEx(Display, Window, DisplayInfoSize, DisplayInfo);
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

static gctBOOL
gcoWL_GetBackbuffer(
    IN HALNativeWindowType Window
    )
{
    int i = 0;
    struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
    for(i = 0; i < wl_window->info->bufferCount; i++)
    {
       if(wl_window->backbuffers[i]->info.locked == gcvFALSE)
       {
           wl_window->info->current = i;
           if(wl_window->info->bufferCount > 1)
           {
               wl_window->backbuffers[i]->info.locked = gcvTRUE;
           }
           return gcvTRUE;
       }
    }
    return gcvFALSE;
}


static void
gcoWL_CreateWLBuffer(
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay
    )
{
    unsigned int i =0;
    struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
    i = wl_window->info->current;

    gcsWL_EGL_DISPLAY* display = ((gcsWL_EGL_DISPLAY*) localDisplay);
    pthread_mutex_lock(&(wl_window->window_mutex));
    struct wl_buffer* wl_buffer = wl_window->backbuffers[i]->wl_buffer;

    if(wl_buffer)
    {
        wl_buffer_destroy(wl_buffer);
    }

    gcoWL_CreateGhostBuffer(display, wl_window->backbuffers[i]);
    wl_buffer = wl_window->backbuffers[i]->wl_buffer;
    wl_proxy_set_queue((struct wl_proxy *) wl_buffer, display->wl_queue);
    wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, wl_window->backbuffers[i]);
    wl_window->backbuffers[i]->info.invalidate = gcvFALSE;
    pthread_mutex_unlock(&(wl_window->window_mutex));
}

gceSTATUS
gcoOS_GetDisplayBackbufferEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvFALSE)
    { /* Client */
        int ret = 0;
        struct wl_egl_window* wl_window = (struct wl_egl_window*) Window;
        gcsWL_EGL_DISPLAY* display = ((gcsWL_EGL_DISPLAY*) localDisplay);
        wl_window->display         = display;

        while ( !gcoWL_GetBackbuffer(Window) && ret != -1)
        {
            ret = wl_display_dispatch_queue(display->wl_display, display->wl_queue);
        }
        if (ret < 0)
            return gcvSTATUS_INVALID_ARGUMENT;

        if( wl_window->backbuffers[wl_window->info->current]->info.width != wl_window->info->width
            || wl_window->backbuffers[wl_window->info->current]->info.height != wl_window->info->height)
        {
            gcoWL_ResizeBO(wl_window);
        }

        if (wl_window->backbuffers[wl_window->info->current]->info.invalidate == gcvTRUE)
        {
           gcoWL_CreateWLBuffer(Window, localDisplay);
        }

        *context = wl_window->backbuffers[wl_window->info->current];
        *surface = wl_window->backbuffers[wl_window->info->current]->info.surface;
        *Offset  = 0;
        *X       = 0;
        *Y       = 0;

        if(wl_window->reference != gcvNULL )
        {
            gcoOS_AtomIncrement(gcvNULL, wl_window->reference, gcvNULL);
        }
    }
    else
    {
        status = gcoOS_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
    }

    return status;
}

gceSTATUS
gcoOS_IsValidDisplay(
    IN HALNativeDisplayType Display
    )
{
    if(Display != gcvNULL)
        return gcvSTATUS_OK;
    return gcvSTATUS_INVALID_ARGUMENT;
}

gctBOOL
gcoOS_SynchronousFlip(
    IN HALNativeDisplayType Display
    )
{
    gctBOOL syncFlip = gcvFALSE;
    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvFALSE)
    {
        gctCHAR *           p;
        p = getenv("GPU_VIV_WL_MULTI_BUFFER");
        if (p != gcvNULL)
        {
            gctINT bufferCount = atoi(p);
            if (bufferCount == 1)
            {
                syncFlip = gcvTRUE;
            }
        }
    }
    else
    {
        struct _FBDisplay* display = (struct _FBDisplay*)Display;
        if(display->multiBuffer <= 3 && display->multiBuffer > 1)
            syncFlip = gcvTRUE;
    }
    return syncFlip;
}

gceSTATUS
gcoOS_GetNativeVisualId(
    IN HALNativeDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_GetWindowInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
    halDISPLAY_INFO info;

    if (gcmIS_ERROR(gcoOS_GetWindowInfo(
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

    if (gcmIS_ERROR(gcoOS_GetDisplayInfoEx(
        Display,
        Window,
        sizeof(info),
        &info)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (_IsCompositorDisplay((struct _FBDisplay*)Display) == gcvTRUE)
    {
        /* Get the color format. */
        switch (info.greenLength)
        {
        case 4:
            if (info.blueOffset == 0)
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
            }
            else
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X4B4G4R4 : gcvSURF_A4B4G4R4;
            }
            break;

        case 5:
            if (info.blueOffset == 0)
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
            }
            else
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X1B5G5R5 : gcvSURF_A1B5G5R5;
            }
            break;

        case 6:
            *Format = gcvSURF_R5G6B5;
            break;

        case 8:
            if (info.blueOffset == 0)
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
            }
            else
            {
                *Format = (info.alphaLength == 0) ? gcvSURF_X8B8G8R8 : gcvSURF_A8B8G8R8;
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

    }
    else
    {
        struct wl_egl_window* wl_window = Window;
        *Format = wl_window->info->format;
        if (Type != gcvNULL)
        {
            *Type = gcvSURF_BITMAP;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_DrawImageEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
    return gcoOS_DrawImage(Display,
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
gcoOS_GetPixmapInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{
    if (gcmIS_ERROR(gcoOS_GetPixmapInfo(
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
gcoOS_CopyPixmapBits(
    IN HALNativeDisplayType Display,
    IN HALNativePixmapType Pixmap,
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
gcoOS_CreateContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DestroyContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_MakeCurrent(
    IN gctPOINTER Display,
    IN HALNativeWindowType DrawDrawable,
    IN HALNativeWindowType ReadDrawable,
    IN gctPOINTER Context,
    IN gcoSURF ResolveTarget
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_CreateDrawable(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DestroyDrawable(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_SwapBuffers(
    IN gctPOINTER Display,
    IN HALNativeWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#endif
#endif

