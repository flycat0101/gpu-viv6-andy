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


#include "gc_egl_platform.h"
#include <directfb.h>

#define _GC_OBJ_ZONE    gcvZONE_OS

#ifndef ABS
#   define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

#define DFB_EGL_NUM_BACKBUFFERS      3
#define DFB_DUMMY                    (31415926)

typedef struct _DFBDisplay * PlatformDisplayType;
typedef struct _DFBWindow  * PlatformWindowType;
typedef struct _DFBPixmap  * PlatformPixmapType;

typedef struct _DFBEGLBuffer
{
    gctINT32 width;
    gctINT32 height;
    gceSURF_FORMAT format;
    gcoSURF surface;
    gctBOOL locked;
    gctPOINTER resolveBits;
} DFBEGLBuffer;

typedef struct _DFBEGlWindowInfo
{
    gctINT32 dx;
    gctINT32 dy;
    gctUINT width;
    gctUINT height;
    gceSURF_FORMAT format;
    gctUINT bpp;
} DFBEGlWindowInfo;

/* Structure that defines a pixmap. */
struct _DFBDisplay
{
    IDirectFB*             pDirectFB;
    IDirectFBDisplayLayer* pLayer;
    IDirectFBEventBuffer*  pEventBuffer;
    gctINT                 winWidth;
    gctINT                 winHeight;
    pthread_cond_t         cond;
    pthread_mutex_t        condMutex;
};

/* Structure that defines a pixmap. */
struct _DFBPixmap
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
};

struct _DFBWindow
{
    IDirectFBWindow*    pWindow;
    IDirectFBSurface*   pDFBSurf;
    DFBEGLBuffer        backbuffers[DFB_EGL_NUM_BACKBUFFERS];
    DFBEGlWindowInfo    info;
    gctINT              current;
};

static gctBOOL
GetDFBSurfFormat(IDirectFBSurface* pDFBSurface, gceSURF_FORMAT *Format, gctUINT32 *Bpp)
{
    gceSURF_FORMAT format;
    DFBSurfacePixelFormat dfb_format;
    gctUINT32 bpp;

    if(pDFBSurface == gcvNULL)
        return gcvFALSE;

    if (pDFBSurface->GetPixelFormat(pDFBSurface, &dfb_format) != DFB_OK)
    {
        return gcvFALSE;
    }

    switch (dfb_format)
    {
    /* A1R5G5B5 */
    case DSPF_ARGB1555:
        format = gcvSURF_A1R5G5B5;
        bpp = 16;
        break;
    /* A4R4G4B4 */
    case DSPF_ARGB4444:
        format = gcvSURF_A4R4G4B4;
        bpp = 16;
        break;
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
    /* R4G4B4A4 */
    case DSPF_RGBA4444:
        format = gcvSURF_R4G4B4A4;
        bpp = 16;
        break;
#endif
    /* A8R8G8B8 */
    case DSPF_ARGB:
        format = gcvSURF_A8R8G8B8;
        bpp = 32;
        break;
    /* R5G6B5 */
    case DSPF_RGB16:
        format = gcvSURF_R5G6B5;
        bpp = 16;
        break;
    /* X1R5G5B5 */
    case DSPF_RGB555:
        format = gcvSURF_X1R5G5B5;
        bpp = 16;
        break;
    /* X4R4G4B4 */
    case DSPF_RGB444:
        format = gcvSURF_X4R4G4B4;
        bpp = 16;
        break;
    /* X8R8G8B8 */
    case DSPF_RGB32:
        format = gcvSURF_X8R8G8B8;
        bpp = 24;
        break;
    /* INDEX8 */
    case DSPF_LUT8:
        format = gcvSURF_INDEX8;
        bpp = 8;
        break;
    /* YV12 */
    case DSPF_YV12:
        format = gcvSURF_YV12;
        bpp = 12;
        break;
    /* I420 */
    case DSPF_I420:
        format = gcvSURF_I420;
        bpp = 12;
        break;
    /* NV12 */
    case DSPF_NV12:
        format = gcvSURF_NV12;
        bpp = 12;
        break;
    /* NV21 */
    case DSPF_NV21:
        format = gcvSURF_NV21;
        bpp = 12;
        break;
    /* NV16 */
    case DSPF_NV16:
        format = gcvSURF_NV16;
        bpp = 16;
        break;
    /* YUY2 */
    case DSPF_YUY2:
        format = gcvSURF_YUY2;
        bpp = 16;
        break;
    /* UYVY */
    case DSPF_UYVY:
        format = gcvSURF_UYVY;
        bpp = 16;
        break;
    case DSPF_A8:
        format = gcvSURF_A8;
        bpp = 8;
        break;
    /* UNKNOWN */
    default:
        format = gcvSURF_UNKNOWN;
        return gcvFALSE;
    }

    if (Format)
    {
        *Format = format;
    }

    if (Bpp)
    {
        *Bpp = bpp;
    }

    return gcvTRUE;
}

/* Structure that defined keyboard mapping. */
typedef struct _keyMap
{
    /* Normal key. */
    platKeyCode normal;

    /* Extended key. */
    platKeyCode extended;
}
keyMap;

static keyMap keys[] =
{   /*  directFB identifiers for basic mapping    */
    /* 00 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 01 */ { KEYCODE_A,               KEYCODE_UNKNOWN     },
    /* 02 */ { KEYCODE_B,               KEYCODE_UNKNOWN     },
    /* 03 */ { KEYCODE_C,               KEYCODE_UNKNOWN     },
    /* 04 */ { KEYCODE_D,               KEYCODE_UNKNOWN     },
    /* 05 */ { KEYCODE_E,               KEYCODE_UNKNOWN     },
    /* 06 */ { KEYCODE_F,               KEYCODE_UNKNOWN     },
    /* 07 */ { KEYCODE_G,               KEYCODE_UNKNOWN     },
    /* 08 */ { KEYCODE_H,               KEYCODE_UNKNOWN     },
    /* 09 */ { KEYCODE_I,               KEYCODE_UNKNOWN     },
    /* 0A */ { KEYCODE_J,               KEYCODE_UNKNOWN     },
    /* 0B */ { KEYCODE_K,               KEYCODE_UNKNOWN     },
    /* 0C */ { KEYCODE_L,               KEYCODE_UNKNOWN     },
    /* 0D */ { KEYCODE_M,               KEYCODE_UNKNOWN     },
    /* 0E */ { KEYCODE_N,               KEYCODE_UNKNOWN     },
    /* 0F */ { KEYCODE_O,               KEYCODE_UNKNOWN     },
    /* 10 */ { KEYCODE_P,               KEYCODE_UNKNOWN     },
    /* 11 */ { KEYCODE_Q,               KEYCODE_UNKNOWN     },
    /* 12 */ { KEYCODE_R,               KEYCODE_UNKNOWN     },
    /* 13 */ { KEYCODE_S,               KEYCODE_UNKNOWN     },
    /* 14 */ { KEYCODE_T,               KEYCODE_UNKNOWN     },
    /* 15 */ { KEYCODE_U,               KEYCODE_UNKNOWN     },
    /* 16 */ { KEYCODE_V,               KEYCODE_UNKNOWN     },
    /* 17 */ { KEYCODE_W,               KEYCODE_UNKNOWN     },
    /* 18 */ { KEYCODE_X,               KEYCODE_UNKNOWN     },
    /* 19 */ { KEYCODE_Y,               KEYCODE_UNKNOWN     },
    /* 1A */ { KEYCODE_Z,               KEYCODE_UNKNOWN     },
    /* 1B */ { KEYCODE_0,               KEYCODE_UNKNOWN     },
    /* 1C */ { KEYCODE_1,               KEYCODE_UNKNOWN     },
    /* 1D */ { KEYCODE_2,               KEYCODE_UNKNOWN     },
    /* 1E */ { KEYCODE_3,               KEYCODE_UNKNOWN     },
    /* 1F */ { KEYCODE_4,               KEYCODE_UNKNOWN     },
    /* 20 */ { KEYCODE_5,               KEYCODE_UNKNOWN     },
    /* 21 */ { KEYCODE_6,               KEYCODE_UNKNOWN     },
    /* 22 */ { KEYCODE_7,               KEYCODE_UNKNOWN     },
    /* 23 */ { KEYCODE_8,               KEYCODE_UNKNOWN     },
    /* 24 */ { KEYCODE_9,               KEYCODE_UNKNOWN     },
    /* 25 */ { KEYCODE_F1,              KEYCODE_UNKNOWN     },
    /* 26 */ { KEYCODE_F2,              KEYCODE_UNKNOWN     },
    /* 27 */ { KEYCODE_F3,              KEYCODE_UNKNOWN     },
    /* 28 */ { KEYCODE_F4,              KEYCODE_UNKNOWN     },
    /* 29 */ { KEYCODE_F5,              KEYCODE_UNKNOWN     },
    /* 2A */ { KEYCODE_F6,              KEYCODE_UNKNOWN     },
    /* 2B */ { KEYCODE_F7,              KEYCODE_UNKNOWN     },
    /* 2C */ { KEYCODE_F8,              KEYCODE_UNKNOWN     },
    /* 2D */ { KEYCODE_F9,              KEYCODE_UNKNOWN     },
    /* 2E */ { KEYCODE_F10,             KEYCODE_UNKNOWN     },
    /* 2F */ { KEYCODE_F11,             KEYCODE_UNKNOWN     },
    /* 30 */ { KEYCODE_F12,             KEYCODE_UNKNOWN     },
    /* 31 */ { KEYCODE_LSHIFT,          KEYCODE_UNKNOWN     },
    /* 32 */ { KEYCODE_RSHIFT,          KEYCODE_UNKNOWN     },
    /* 33 */ { KEYCODE_LCTRL,           KEYCODE_UNKNOWN     },
    /* 34 */ { KEYCODE_RCTRL,           KEYCODE_UNKNOWN     },
    /* 35 */ { KEYCODE_LALT,            KEYCODE_UNKNOWN     },
    /* 36 */ { KEYCODE_RALT,            KEYCODE_UNKNOWN     },
    /* 37 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 38 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 39 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 3A */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 3B */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 3C */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 3D */ { KEYCODE_CAPSLOCK,        KEYCODE_UNKNOWN     },
    /* 3E */ { KEYCODE_NUMLOCK,         KEYCODE_UNKNOWN     },
    /* 3F */ { KEYCODE_SCROLLLOCK,      KEYCODE_UNKNOWN     },
    /* 40 */ { KEYCODE_ESCAPE,          KEYCODE_UNKNOWN     },
    /* 41 */ { KEYCODE_LEFT,            KEYCODE_UNKNOWN     },
    /* 42 */ { KEYCODE_RIGHT,           KEYCODE_UNKNOWN     },
    /* 43 */ { KEYCODE_UP,              KEYCODE_UNKNOWN     },
    /* 44 */ { KEYCODE_DOWN,            KEYCODE_UNKNOWN     },
    /* 45 */ { KEYCODE_TAB,             KEYCODE_UNKNOWN     },
    /* 46 */ { KEYCODE_ENTER,           KEYCODE_UNKNOWN     },
    /* 47 */ { KEYCODE_SPACE,           KEYCODE_UNKNOWN     },
    /* 48 */ { KEYCODE_BACKSPACE,       KEYCODE_UNKNOWN     },
    /* 49 */ { KEYCODE_INSERT,          KEYCODE_UNKNOWN     },
    /* 4A */ { KEYCODE_DELETE,          KEYCODE_UNKNOWN     },
    /* 4B */ { KEYCODE_HOME,            KEYCODE_UNKNOWN     },
    /* 4C */ { KEYCODE_END,             KEYCODE_UNKNOWN     },
    /* 4D */ { KEYCODE_PGUP,            KEYCODE_UNKNOWN     },
    /* 4E */ { KEYCODE_PGDN,            KEYCODE_UNKNOWN     },
    /* 4F */ { KEYCODE_PRNTSCRN,        KEYCODE_UNKNOWN     },
    /* 50 */ { KEYCODE_BREAK,           KEYCODE_UNKNOWN     },
    /* 51 */ { KEYCODE_SINGLEQUOTE,     KEYCODE_UNKNOWN     },
    /* 52 */ { KEYCODE_HYPHEN,          KEYCODE_UNKNOWN     },
    /* 53 */ { KEYCODE_EQUAL,           KEYCODE_UNKNOWN     },
    /* 54 */ { KEYCODE_LBRACKET,        KEYCODE_UNKNOWN     },
    /* 55 */ { KEYCODE_RBRACKET,        KEYCODE_UNKNOWN     },
    /* 56 */ { KEYCODE_BACKSLASH,       KEYCODE_UNKNOWN     },
    /* 57 */ { KEYCODE_SEMICOLON,       KEYCODE_UNKNOWN     },
    /* 58 */ { KEYCODE_BACKQUOTE,       KEYCODE_UNKNOWN     },
    /* 59 */ { KEYCODE_COMMA,           KEYCODE_UNKNOWN     },
    /* 5A */ { KEYCODE_PERIOD,          KEYCODE_UNKNOWN     },
    /* 5B */ { KEYCODE_SLASH,           KEYCODE_UNKNOWN     },
    /* 5C */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 5D */ { KEYCODE_PAD_SLASH,       KEYCODE_UNKNOWN     },
    /* 5E */ { KEYCODE_PAD_ASTERISK,    KEYCODE_UNKNOWN     },
    /* 5F */ { KEYCODE_PAD_HYPHEN,      KEYCODE_UNKNOWN     },
    /* 60 */ { KEYCODE_PAD_PLUS,        KEYCODE_UNKNOWN     },
    /* 61 */ { KEYCODE_PAD_ENTER,       KEYCODE_UNKNOWN     },
    /* 62 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 63 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 64 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 65 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 66 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 67 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 68 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 69 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6A */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6B */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6C */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6D */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6E */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 6F */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 70 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 71 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 72 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 73 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 74 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 75 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 76 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 77 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 78 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 79 */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7A */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7B */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7C */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7D */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7E */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
    /* 7F */ { KEYCODE_UNKNOWN,         KEYCODE_UNKNOWN     },
};

/*******************************************************************************
** Display.
*/

static gceSTATUS
dfb_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _DFBDisplay* display = gcvNULL;
    DFBDisplayLayerConfig config;
    gcmHEADER();
    *Display = (PlatformDisplayType)gcvNULL;
    do
    {
        if(DirectFBInit(gcvNULL, gcvNULL) != DFB_OK)
        {
            gcmFATAL("%s: failed to init directfb.", __FUNCTION__);
            status = gcvSTATUS_OUT_OF_RESOURCES;
            gcmFOOTER();
            return status;
        }

        DirectFBSetOption ("no-sighandler", NULL);

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct _DFBDisplay),
                              (gctPOINTER *)&display))
        {
            gcmFATAL("%s: failed to malloc memory.", __FUNCTION__);
            status = gcvSTATUS_OUT_OF_RESOURCES;
            gcmFOOTER();
            return status;
        }

        gcoOS_ZeroMemory(display, sizeof(struct _DFBDisplay));

        if (DirectFBCreate(&(display->pDirectFB)) != DFB_OK)
        {
            gcmFATAL("%s: failed to create directfb.", __FUNCTION__);
            status = gcvSTATUS_OUT_OF_RESOURCES;
            gcmFOOTER();
            return status;
        }
        if(display->pDirectFB->CreateInputEventBuffer(display->pDirectFB, DICAPS_KEYS, DFB_FALSE, &(display->pEventBuffer)) != DFB_OK)
        {
            gcmFATAL("%s: failed to create directfb input event.", __FUNCTION__);
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }
        if(display->pDirectFB->GetDisplayLayer( display->pDirectFB, DLID_PRIMARY, &(display->pLayer) ) != DFB_OK)
            break;
        display->pLayer->GetConfiguration(display->pLayer, &config);
        display->winWidth = config.width;
        display->winHeight = config.height;
        pthread_cond_init(&(display->cond), gcvNULL);
        pthread_mutex_init(&(display->condMutex), gcvNULL);
        *Display = (PlatformDisplayType)display;
        gcmFOOTER();
        return status;
    }
    while(0);
    if(display != gcvNULL)
    {
        if(display->pDirectFB != gcvNULL)
        {
            if(display->pEventBuffer != gcvNULL)
            {
                display->pEventBuffer->Release(display->pEventBuffer);
            }
            if(display->pLayer != gcvNULL)
            {
                display->pLayer->Release(display->pLayer);
            }
            display->pDirectFB->Release(display->pDirectFB);
        }
        gcmOS_SAFE_FREE(gcvNULL, display);
        display = gcvNULL;
    }
    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

typedef struct _dfbDISPLAY_INFO
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
dfbDISPLAY_INFO;


static gceSTATUS
dfb_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT dfbDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _DFBWindow* window = Window;

    DisplayInfo->width = window->info.width;
    DisplayInfo->height = window->info.height;
    DisplayInfo->stride = DFB_DUMMY;
    DisplayInfo->bitsPerPixel = window->info.bpp;
    DisplayInfo->logical = (gctPOINTER) DFB_DUMMY;
    DisplayInfo->physical = DFB_DUMMY;
    DisplayInfo->multiBuffer = DFB_EGL_NUM_BACKBUFFERS;
    DisplayInfo->backBufferY = 0;
    DisplayInfo->flip = gcvTRUE;
    DisplayInfo->wrapFB = gcvFALSE;

    return status;
}

static gceSTATUS
dfb_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dfb_GetDisplayBackbuffer(
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
    struct _DFBWindow  *window = Window;

    /*Wait the back buffer to be free*/
    pthread_mutex_lock(&(Display->condMutex));
    if (window->backbuffers[window->current].locked)
    {
        pthread_cond_wait(&(Display->cond), &(Display->condMutex));
    }

    window->backbuffers[window->current].locked = gcvTRUE;
    pthread_mutex_unlock(&(Display->condMutex));

    *context = &window->backbuffers[window->current];
    *surface = window->backbuffers[window->current].surface;
    *Offset  = 0;
    *X       = 0;
    *Y       = 0;
    window->current = (window->current + 1)%DFB_EGL_NUM_BACKBUFFERS;

    return status;
}

static gceSTATUS
dfb_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gctPOINTER logical;
    gctINT pitch, BitsPerPixel;
    IDirectFBSurface *pDFBSurf;
    DFBEGLBuffer *buffer = (DFBEGLBuffer*)Context;

    if((Display == gcvNULL) || (Window == 0) || Window->pWindow == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    BitsPerPixel = Window->info.bpp;
    pDFBSurf = Window->pDFBSurf;
    if (pDFBSurf == gcvNULL)
    {
        if(Window->pWindow->GetSurface( Window->pWindow, &pDFBSurf ) != DFB_OK)
            return gcvSTATUS_INVALID_ARGUMENT;
        Window->pDFBSurf = pDFBSurf;
    }

    if (pDFBSurf->Lock(pDFBSurf,
        DSLF_WRITE,
        &logical,
        &pitch) != DFB_OK)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }
    memcpy(logical, buffer->resolveBits, ((buffer->width * BitsPerPixel / 8 + 3) & ~3) * gcmABS(buffer->height));
    pDFBSurf->Unlock(pDFBSurf);
    pDFBSurf->Flip( pDFBSurf, NULL, DSFLIP_WAITFORSYNC );
    Display->pDirectFB->WaitIdle( Display->pDirectFB );

    pthread_mutex_lock(&(Display->condMutex));
    buffer->locked = gcvFALSE;
    pthread_cond_broadcast(&(Display->cond));
    pthread_mutex_unlock(&(Display->condMutex));

    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_CancelDisplayBackbuffer(
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

static gceSTATUS
dfb_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    if(Display)
    {
        if(Display->pDirectFB)
        {
            if(Display->pEventBuffer != gcvNULL)
            {
                Display->pEventBuffer->Release(Display->pEventBuffer);
            }
            if(Display->pLayer != gcvNULL)
            {
                Display->pLayer->Release(Display->pLayer);
            }
            Display->pDirectFB->Release(Display->pDirectFB);
        }
        pthread_mutex_destroy(&(Display->condMutex));
        pthread_cond_destroy(&(Display->cond));
        gcmOS_SAFE_FREE(gcvNULL, Display);
    }
    return gcvSTATUS_OK;
}

/*******************************************************************************
** Windows
*/
static void
_DestoryBackBuffers(
    IN PlatformWindowType Window
    )
{
    gctUINT i;

    if (Window != gcvNULL)
    {
        for (i = 0; i < DFB_EGL_NUM_BACKBUFFERS; i++)
        {
            if (Window->backbuffers[i].surface != gcvNULL)
            {
                gcoSURF_Unlock(
                    Window->backbuffers[i].surface,
                    gcvNULL
                    );

                gcoSURF_Destroy(
                    Window->backbuffers[i].surface
                    );
            }
        }
    }
}

static gceSTATUS
_CreateBackBuffers(
    IN PlatformDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    IN PlatformWindowType window
    )
{
    gctUINT i;
    gceSTATUS status = gcvSTATUS_OK;
    do
    {
        gceSURF_FORMAT resolveFormat = gcvSURF_UNKNOWN;
        gctUINT32 BitsPerPixel = 0;
        IDirectFBWindow  *pWindow = window->pWindow;
        IDirectFBSurface *pDFBSurf = window->pDFBSurf;

        window->info.dx     = 0;
        window->info.dy     = 0;
        window->info.width  = Width;
        window->info.height = Height;

        if (pDFBSurf == gcvNULL)
        {
            if(pWindow->GetSurface( pWindow, &pDFBSurf ) != DFB_OK)
                break;
            window->pDFBSurf = pDFBSurf;
        }

        if (!GetDFBSurfFormat(pDFBSurf, &resolveFormat, &BitsPerPixel))
        {
            break;
        }

        window->info.bpp = BitsPerPixel;
        window->info.format = resolveFormat;

        for (i = 0; i < DFB_EGL_NUM_BACKBUFFERS; i++)
        {
            gctPOINTER memoryResolve[3] = {gcvNULL};
            gcmONERROR(
                gcoSURF_Construct(
                    gcvNULL,
                    Width,
                    Height,
                    1,
                    gcvSURF_BITMAP,
                    resolveFormat,
                    gcvPOOL_DEFAULT,
                    &window->backbuffers[i].surface
                    ));

            gcmONERROR(
                gcoSURF_Lock(
                    window->backbuffers[i].surface,
                    gcvNULL,
                    memoryResolve
                    ));

            window->backbuffers[i].resolveBits = memoryResolve[0];
            window->backbuffers[i].width = window->info.width;
            window->backbuffers[i].height = window->info.height;
            window->backbuffers[i].format = resolveFormat;
            window->backbuffers[i].locked = gcvFALSE;
        }
    }
    while (gcvFALSE);

    return status;

OnError:
    _DestoryBackBuffers(window);
    status = gcvSTATUS_OUT_OF_RESOURCES;
    return status;
}

static gceSTATUS
dfb_CreateWindow(
    IN PlatformDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT PlatformWindowType * Window
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    struct _DFBWindow  *DFBWindow = gcvNULL;
    IDirectFBWindow    *window = gcvNULL;

    gcmHEADER_ARG("Display=0x%x X=%d Y=%d Width=%d Height=%d", Display, X, Y, Width, Height);

    do
    {
        DFBWindowDescription  desc;
        if(Display == gcvNULL)
            break;
        if(Display->pLayer == gcvNULL)
            break;
        Display->pLayer->SetCooperativeLevel( Display->pLayer, DLSCL_ADMINISTRATIVE );

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct _DFBWindow),
                              (gctPOINTER *)&DFBWindow))
        {
            break;
        }
        gcoOS_ZeroMemory(DFBWindow, sizeof(struct _DFBWindow));

        desc.flags = ( DWDESC_POSX | DWDESC_POSY);
        if(X < 0)
            X = 0;
        if(Y < 0)
            Y = 0;
        desc.posx   = X;
        desc.posy   = Y;
        /* if no width or height setting, dfb system will use the default layer config for the window creation*/
        if((Width <= 0) || (Height <= 0))
        {
            Width = Display->winWidth;
            Height = Display->winHeight;
        }
        else
        {
            Display->winWidth = Width;
            Display->winHeight = Height;
        }
        desc.flags = desc.flags | DWDESC_WIDTH | DWDESC_HEIGHT;
        desc.width  = Width;
        desc.height = Height;
        if(Display->pLayer->CreateWindow( Display->pLayer, &desc, &window) != DFB_OK)
            break;
        DFBWindow->pWindow = window;

        window->SetOpacity(window, 0xFF);
        window->RaiseToTop(window);
        window->AttachEventBuffer(window, Display->pEventBuffer);
        Display->pLayer->EnableCursor(Display->pLayer, 0);

        status = _CreateBackBuffers(Display, X, Y, Width, Height, DFBWindow);
        if(status != gcvSTATUS_OK)
            break;
        *Window = DFBWindow;
        gcmFOOTER_ARG("*Window=0x%x", *Window);
        return status;
    }
    while (0);

    if(window)
        window->Release(window);

    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

static gceSTATUS
dfb_GetWindowInfo(
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
    gctINT width, height;
    gceSURF_FORMAT format;

    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);

    if (Window == 0 || Window->pWindow == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    do
    {
        IDirectFBWindow  *pWindow = Window->pWindow;
        IDirectFBSurface *pDFBSurf = Window->pDFBSurf;

        if (pDFBSurf == gcvNULL)
        {
            if(pWindow->GetSurface( pWindow, &pDFBSurf ) != DFB_OK)
                break;
            Window->pDFBSurf = pDFBSurf;
        }

        if (!GetDFBSurfFormat(pDFBSurf, &format, (gctUINT32*)BitsPerPixel))
        {
            break;
        }

        if(X != gcvNULL)
            * X = 0;
        if(Y != gcvNULL)
            * Y = 0;
        pDFBSurf->GetSize(pDFBSurf, &width, &height);

        if(Width != gcvNULL)
            * Width = width;
        if(Height != gcvNULL)
            * Height = height;

        if (Offset != gcvNULL)
            *Offset = 0;

        gcmFOOTER_NO();
        return status;
    }
    while(gcvFALSE);

    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
dfb_DestroyWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    if((Display == gcvNULL) || (Window == 0) || Window->pWindow == NULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    if (Window->pDFBSurf)
        Window->pDFBSurf->Release(Window->pDFBSurf);

    Window->pWindow->Release(Window->pWindow);

    _DestoryBackBuffers(Window);

    gcmOS_SAFE_FREE(gcvNULL, Window);
    return gcvSTATUS_OK;
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

static gceSTATUS
dfb_CreatePixmap(
    IN PlatformDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    OUT PlatformPixmapType * Pixmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT alignedWidth, alignedHeight;
    struct _DFBPixmap* pixmap;
    gcmHEADER_ARG("Display=0x%x Width=%d Height=%d BitsPerPixel=%d", Display, Width, Height, BitsPerPixel);

    if ((Width <= 0) || (Height <= 0) || (BitsPerPixel <= 0))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    do
    {
        alignedWidth   = (Width + 0x0F) & (~0x0F);
        alignedHeight  = (Height + 0x3) & (~0x03);
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct _DFBPixmap),
                              (gctPOINTER *)&pixmap))
        {
            break;
        }

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                              alignedWidth * alignedHeight * (BitsPerPixel + 7) / 8 + 64,
                              (gctPOINTER *)&pixmap->original))
        {
            gcmOS_SAFE_FREE(gcvNULL, pixmap);
            pixmap = gcvNULL;
            break;
        }
        pixmap->bits = (gctPOINTER)(((gctUINT)(gctCHAR*)pixmap->original + 0x3F) & (~0x3F));

        pixmap->width = Width;
        pixmap->height = Height;
        pixmap->alignedWidth    = alignedWidth;
        pixmap->alignedHeight   = alignedHeight;
        pixmap->bpp     = (BitsPerPixel + 0x07) & (~0x07);
        pixmap->stride  = Width * (pixmap->bpp) / 8;
        *Pixmap = (PlatformPixmapType)pixmap;
        gcmFOOTER_ARG("*Pixmap=0x%x", *Pixmap);
        return status;
    }
    while (0);

    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

static gceSTATUS
dfb_GetPixmapInfo(
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
    struct _DFBPixmap* pixmap;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);
    pixmap = (struct _DFBPixmap*)Pixmap;
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
        *Bits = pixmap->bits;
    }

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
dfb_DestroyPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap
    )
{
    struct _DFBPixmap* pixmap;
    pixmap = (struct _DFBPixmap*)Pixmap;
    if (pixmap != gcvNULL)
    {
        if (pixmap->original != NULL)
        {
            gcmOS_SAFE_FREE(gcvNULL, pixmap->original);
        }
        gcmOS_SAFE_FREE(gcvNULL, pixmap);
        pixmap = gcvNULL;
        Pixmap = gcvNULL;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_DrawPixmap(
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

static gceSTATUS
dfb_ShowWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{

    if(((Window == 0) || Window->pWindow == gcvNULL))
        return gcvSTATUS_INVALID_ARGUMENT;

    Window->pWindow->RaiseToTop(Window->pWindow);

    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_HideWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{

    if((Display == gcvNULL) || (Window == 0) || Window->pWindow == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    Window->pWindow->LowerToBottom(Window->pWindow);

    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_GetEvent(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT platEvent * Event
    )
{
    /* event buffer. */
    DFBWindowEvent evt;
    /* Translated scancode. */
    platKeyCode scancode;

    /* Test for valid Window and Event pointers. */
    if ((Display == gcvNULL) ||(Window == gcvNULL) || (Event == gcvNULL))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(Display->pEventBuffer == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if(Display->pEventBuffer->GetEvent( Display->pEventBuffer, DFB_EVENT(&evt) ) == DFB_OK)
    {
        switch (evt.type)
        {
        case DWET_KEYDOWN:
        case DWET_KEYUP:
            /* Keyboard event. */
            Event->type = EVENT_KEYBOARD;

            /* Translate the scancode. */
            scancode = (evt.key_id & 0x80)
                ? keys[evt.key_id & 0x7F].extended
                : keys[evt.key_id & 0x7F].normal;
            /* Set scancode. */
            Event->data.keyboard.scancode = scancode;

            /* Set ASCII key. */
            Event->data.keyboard.key = (  (scancode < KEYCODE_SPACE)
                || (scancode >= KEYCODE_F1)
                )
                ? 0
                : (char) scancode;

            /* Set up or down flag. */
            Event->data.keyboard.pressed = (evt.type == DWET_KEYDOWN);
            /* Valid event. */
            return gcvSTATUS_OK;
        case DWET_BUTTONDOWN:
        case DWET_BUTTONUP:
            /* Button event. */
            Event->type = EVENT_BUTTON;

            /* Set button states. */
            Event->data.button.left   = (evt.button == DIBI_LEFT) ? 1 : 0;
            Event->data.button.middle = (evt.button & DIBI_MIDDLE) ? 1 : 0;
            Event->data.button.right  = (evt.button & DIBI_RIGHT) ? 1 : 0;
            Event->data.button.x      = evt.cx;
            Event->data.button.y      = evt.cy;
            return gcvSTATUS_OK;
        case DWET_MOTION:
            /* Pointer event.*/
            Event->type = EVENT_POINTER;

            /* Set pointer location. */
            Event->data.pointer.x = evt.cx;
            Event->data.pointer.y = evt.cy;
            /* Valid event. */
            return gcvSTATUS_OK;
        case DWET_CLOSE:
        case DWET_DESTROYED:
            /* Application should close. */
            Event->type = EVENT_CLOSE;
            /* Valid event. */
            return gcvSTATUS_OK;
        default:
            return gcvSTATUS_NOT_FOUND;
        }
    }
    return gcvSTATUS_NOT_FOUND;
}

static gceSTATUS
dfb_GetDisplayInfoEx2(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT dfbDISPLAY_INFO * DisplayInfo
    )
{
    return dfb_GetDisplayInfoEx(Display,Window,DisplayInfoSize,DisplayInfo);
}

static gceSTATUS
dfb_GetDisplayBackbufferEx(
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
    return dfb_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
}

static gceSTATUS
dfb_GetWindowInfoEx(
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
    gctINT width, height;
    gceSURF_FORMAT format;
    IDirectFBSurface *pDFBSurf;

    if((Display == gcvNULL) || (Window == 0) || Window->pWindow == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    do
    {
        pDFBSurf = Window->pDFBSurf;
        if (pDFBSurf == gcvNULL)
        {
            if(Window->pWindow->GetSurface( Window->pWindow, &pDFBSurf ) != DFB_OK)
                break;
            Window->pDFBSurf = pDFBSurf;
        }

        if (!GetDFBSurfFormat(pDFBSurf, &format, (gctUINT32*)BitsPerPixel))
        {
            break;
        }

        if (Format != gcvNULL)
        {
            *Format = format;
        }

        if (X != gcvNULL)
        {
            *X = 0;
        }

        if (Y != gcvNULL)
        {
            *Y = 0;
        }

        pDFBSurf->GetSize(pDFBSurf, &width, &height);

        if (Width != gcvNULL)
        {
            *Width = width;
        }

        if (Height != gcvNULL)
        {
            *Height = height;
        }

        if (Type != gcvNULL)
        {
            *Type = gcvSURF_BITMAP;
        }

        return gcvSTATUS_OK;
    }
    while(gcvFALSE);
    return gcvSTATUS_INVALID_ARGUMENT;
}

static gceSTATUS
dfb_SetWindowFormat(
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
    IDirectFBSurface *pDFBSurf;
    gceSURF_FORMAT format;

    /* Only BITMAP. */
    if (Type != gcvSURF_BITMAP)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    pDFBSurf = Window->pDFBSurf;

    /* Can not change format. */
    if (!GetDFBSurfFormat(pDFBSurf, &format, gcvNULL) ||
        Format != format)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_GetPixmapInfoEx(
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
    if (gcmIS_ERROR(dfb_GetPixmapInfo(
                        Display,
                        Pixmap,
                      (gctINT_PTR) Width, (gctINT_PTR) Height,
                      (gctINT_PTR) BitsPerPixel,
                      Stride,
                      Bits)))
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
        *Format = gcvSURF_X8R8G8B8;
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
dfb_CopyPixmapBits(
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
    PlatformDisplayType display = gcvNULL;
    dfb_GetDisplay(&display, gcvNULL);

    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    dfb_DestroyDisplay((PlatformDisplayType) Display);
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (Display != NULL)
    {
        return EGL_TRUE;
    }

    return EGL_FALSE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint visualId = 0;
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
    /* Not supported. */
    return EGL_FALSE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    /*
     * return true to maintain legacy behavior. If the feature is not there
     * we were ignoring it. And now we are ignoring it too.
     */
    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */

struct eglWindowInfo
{
    /* Information obtained by dfb_GetDisplayInfoEx2. */
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

static
EGLBoolean
_QueryWindowInfo(
    IN VEGLDisplay Display,
    IN void * Window,
    IN VEGLWindowInfo Info
    )
{
    gceSTATUS status;
    dfbDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = dfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
    gcoOS_ZeroMemory(&dInfo, sizeof (dfbDISPLAY_INFO));

    status = dfb_GetDisplayInfoEx2((PlatformDisplayType) Display->hdc,
                                     (PlatformWindowType) Window,
                                     Display->localInfo,
                                     sizeof (dfbDISPLAY_INFO),
                                     &dInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    Info->logical      = dInfo.logical;
    Info->physical     = dInfo.physical;
    Info->stride       = dInfo.stride;
    Info->multiBuffer  = dInfo.multiBuffer > 1 ? dInfo.multiBuffer : 1;

    /* Get virtual size. */
    status = dfb_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
                                     (gctINT_PTR) &Info->xresVirtual,
                                     (gctINT_PTR) &Info->yresVirtual);

    if (gcmIS_ERROR(status))
    {
        Info->xresVirtual = Info->width;
        Info->yresVirtual = Info->height;
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
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Window != gcvNULL);
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

    /* Save window info structure. */
    Surface->winInfo = info;
    return EGL_TRUE;

OnError:
    if (info)
    {
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
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

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

    status = dfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
            status = dfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                reqFormat = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_PE_A8B8G8R8) ? gcvSURF_A8B8G8R8 : gcvSURF_A8R8G8B8;
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

            /* Try many direct rendering levels here. */
            /* 1. The best, direct rendering with compression. */
            if ((type != gcvSURF_RENDER_TARGET) ||
                (info->type != gcvSURF_RENDER_TARGET)  ||
                (info->format != reqFormat))
            {
                status = dfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

                status = dfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                status = dfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
            status = dfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

    status = dfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    gctUINT offset;

    BackBuffer->surface  = gcvNULL;
    BackBuffer->context  = gcvNULL;
    BackBuffer->origin.x = 0;
    BackBuffer->origin.y = 0;
    BackBuffer->flip     = gcvTRUE;

    /* Formerly veglGetDisplayBackBuffer. */
    status = dfb_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
                                          (PlatformWindowType) win,
                                           Display->localInfo,
                                           &BackBuffer->context,
                                           &BackBuffer->surface,
                                           &offset,
                                           &BackBuffer->origin.x,
                                           &BackBuffer->origin.y);

    /* Should return the surface directly. */
    if (gcmIS_ERROR(status) ||
        !BackBuffer->surface)
    {
        return EGL_FALSE;
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
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    status = dfb_SetDisplayVirtualEx((PlatformDisplayType) Display->hdc,
                                       (PlatformWindowType) win,
                                       BackBuffer->context,
                                       BackBuffer->surface,
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
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    void * win = Surface->hwnd;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    status = dfb_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           BackBuffer->context,
                                           BackBuffer->surface,
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
    return EGL_FALSE;
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
        gcmONERROR(dfb_CopyPixmapBits((PlatformDisplayType) Info->hdc,
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
        gcmONERROR(dfb_DrawPixmap((PlatformDisplayType) Info->hdc,
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

    status = dfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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
    gctPHYS_ADDR_T pixmapPhysical = gcvINVALID_PHYSICAL_ADDRESS;
    gcoSURF wrapper = gcvNULL;
    gcoSURF shadow = gcvNULL;
    gctPOINTER pointer;
    VEGLPixmapInfo info = gcvNULL;

    /* Query pixmap geometry info. */
    gcmONERROR(dfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = dfb_GetPixmapInfo((PlatformDisplayType) Display->hdc,
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
        dfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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


static struct eglPlatform dfbPlatform =
{
    EGL_PLATFORM_DFB_VIV,
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
veglGetDfbPlatform(
    void * NativeDisplay
    )
{
    return &dfbPlatform;
}

/******************************************************************************/

void *
dfbGetDisplay(
    gctPOINTER context
    )
{
    PlatformDisplayType display = gcvNULL;
    dfb_GetDisplay(&display, context);
    return display;
}

void
dfbDestroyDisplay(
    IN void * Display
    )
{
    dfb_DestroyDisplay((PlatformDisplayType)Display);
}

void *
dfbCreateWindow(
    IN void * Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformWindowType Window = gcvNULL;
    dfb_CreateWindow((PlatformDisplayType)Display, X, Y, Width, Height, &Window);
    return Window;
}

void
dfbShowWindow(
    void * Display,
    void * Window
    )
{
    dfb_ShowWindow(Display, Window);
}

void
dfbHideWindow(
    void * Display,
    void * Window
    )
{
    dfb_HideWindow(Display, Window);
}

int
dfbGetEvent(
    void * Display,
    void * Window,
    platEvent * Event
    )
{
    if (gcmIS_SUCCESS(dfb_GetEvent(Display, Window, Event)))
    {
        return 1;
    }

    return 0;
}

int
dfbGetWindowInfo(
    void * Window,
    int * X,
    int * Y,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    unsigned int * Offset
    )
{
    if (gcmIS_SUCCESS(dfb_GetWindowInfo(gcvNULL,
                                        Window,
                                        X, Y, Width, Height,
                                        BitsPerPixel, Offset)))
    {
        return 1;
    }

    return 0;
}

void
dfbDestroyWindow(
    void * Window
    );

void
dfbDestroyWindow(
    void * Window
    )
{
    dfb_DestroyWindow(gcvNULL, (PlatformWindowType)Window);
}

void *
dfbCreatePixmap(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    dfb_CreatePixmap((PlatformDisplayType)Display, Width, Height, 32, &Pixmap);
    return Pixmap;
}

void *
dfbCreatePixmapWithBpp(
    IN void * Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel
    )
{
    PlatformPixmapType Pixmap = gcvNULL;
    dfb_CreatePixmap((PlatformDisplayType)Display, Width, Height, BitsPerPixel, &Pixmap);
    return Pixmap;
}

void
dfbGetPixmapInfo(
    IN void * Pixmap,
    OUT gctINT_PTR Width,
    OUT gctINT_PTR Height,
    OUT gctINT_PTR BitsPerPixel,
    OUT gctINT_PTR Stride,
    OUT gctPOINTER * Bits
    )
{
    dfb_GetPixmapInfo(gcvNULL, (PlatformPixmapType)Pixmap, Width, Height, BitsPerPixel, Stride, Bits);
}

void
dfbDestroyPixmap(
    IN void * Pixmap
    )
{
    dfb_DestroyPixmap(gcvNULL, (PlatformPixmapType)Pixmap);
}
