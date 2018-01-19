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


#define _ISOC99_SOURCE
#define _XOPEN_SOURCE 501

#include "gc_hal_user_linux.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>

#include <sys/time.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/xshmfence.h>
#include <xcb/xcb.h>
#include <xcb/present.h>
#include <xcb/xcbext.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#include <xcb/sync.h>

#include <X11/Xlib-xcb.h>

#include <linux/dma-buf.h>
#include <xf86drm.h>
#include "vivante_bo.h"


#include <gc_hal_types.h>
#include <gc_hal_base.h>
#include "gc_egl_platform.h"

#define _GC_OBJ_ZONE    gcvZONE_OS

typedef Display *PlatformDisplayType;
typedef Window   PlatformWindowType;
typedef Pixmap   PlatformPixmapType;

typedef struct __DRIcontextRec  __DRIcontextPriv;
typedef struct __DRIdrawableRec __DRIdrawablePriv;
typedef struct __DRIDisplayRec  __DRIDisplay;

static xcb_connection_t *dri_GetXCB(void *dpy)
{
    return XGetXCBConnection(dpy);
}

static int
dri3_open(Display *dpy,
          Window root,
          CARD32 provider)
{
   xcb_dri3_open_cookie_t       cookie;
   xcb_dri3_open_reply_t        *reply;
   xcb_connection_t             *c = dri_GetXCB(dpy);
   int                          fd;

   cookie = xcb_dri3_open(c,
                          root,
                          provider);

   reply = xcb_dri3_open_reply(c, cookie, NULL);
   if (!reply)
      return -1;

   if (reply->nfd != 1) {
      free(reply);
      return -1;
   }

   fd = xcb_dri3_open_reply_fds(c, reply)[0];
   fcntl(fd, F_SETFD, FD_CLOEXEC);

   return fd;
}

static int create_fd_from_pixmap(xcb_connection_t *c, Pixmap pixmap, int *stride) {
    int *fd;
    xcb_dri3_buffer_from_pixmap_cookie_t cookie;
    xcb_dri3_buffer_from_pixmap_reply_t *reply;

    cookie = xcb_dri3_buffer_from_pixmap(c, pixmap);
    reply = xcb_dri3_buffer_from_pixmap_reply(c, cookie, NULL);
    if (!reply)
        return -1;

    if (reply->nfd != 1)
        return -1;

    *stride = reply->stride;
    fd = xcb_dri3_buffer_from_pixmap_reply_fds(c, reply);
    free(reply);
    return fd[0];
}

struct __DRIcontextRec {
    gctPOINTER eglContext;
    /*
    ** Pointer to drawable currently bound to this context.
    */
    __DRIdrawablePriv *drawablePriv;
    __DRIcontextPriv            *next;
};


#define _FRAME_BUSY     1
#define _FRAME_FREE     0
#define _NOT_SET        0
#define _TO_INITFRAME   1
#define _TO_UPDATEFRAME 2
#define NUM_ASYNCFRAME  4

typedef struct _asyncFrame {
    __DRIdrawablePriv *dridrawable;
    PlatformWindowType Drawable;
    Pixmap backPixmap;
    gcoSURF pixWrapSurf;
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctUINT32 backNode;
    int w;
    int h;
    xcb_sync_fence_t sync_fence;
    struct xshmfence *shm_fence;
    int fence_fd;
    int pixmapfd;
    EGLint age;
} asyncFrame;


typedef gceSTATUS (*RSFUNC)(gcoSURF, gcoSURF, gcsPOINT_PTR, gcsPOINT_PTR, gcsPOINT_PTR);

struct __DRIdrawableRec {
    /**
     * X's drawable ID associated with this private drawable.
     */
    PlatformWindowType drawable;

    /**
     * Reference count for number of context's currently bound to this
     * drawable.
     *
     * Once it reaches zero, the drawable can be destroyed.
     *
     * \note This behavior will change with GLX 1.3.
     */
    gctINT refcount;

    /**
     * Index of this drawable information in the SAREA.
     */
    gctUINT index;

    /**
     * Pointer to the "drawable has changed ID" stamp in the SAREA.
     */
    gctUINT Stamp;

    /**
     * Last value of the stamp.
     *
     * If this differs from the value stored at __DRIdrawablePrivate::pStamp,
     * then the drawable information has been modified by the X server, and the
     * drawable information (below) should be retrieved from the X server.
     */
    gctUINT lastStamp;

    gctINT x;
    gctINT y;
    gctINT w;
    gctINT h;

    gctINT backX;
    gctINT backY;
    gctINT backClipRectType;
    gctINT wWidth;
    gctINT wHeight;
    gctINT xWOrigin;
    gctINT yWOrigin;


    /* Back buffer information */
    gctUINT nodeName;
    gctUINT backNode;

    __DRIcontextPriv *contextPriv;

    __DRIDisplay * display;
    gctINT screen;

    gctBOOL fullScreenMode;
    __DRIdrawablePriv * next;
    int fullscreenCovered;


    /* the next two are initialized when creating drawable */
    uint32_t eid;
    void *cq;

    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;

    asyncFrame ascframe[NUM_ASYNCFRAME];
    int busyframe[NUM_ASYNCFRAME];

    gctINT oldx;
    gctINT oldy;
    gctUINT oldw;
    gctUINT oldh;
    PlatformWindowType olddrawable;
};


/* Structure that defines a display. */
struct __DRIDisplayRec
{
    gctINT                         drmFd;
    struct drm_vivante             *drmVIV;
    Display                        *dpy;
    gctSIZE_T                      physicalAddr;
    gctPOINTER                     linearAddr;
    gctINT                         fbSize;
    gctINT                         stride;
    gctINT                         width;
    gctINT                         height;
    gceSURF_FORMAT                 format;
    gctINT                         screen;
    gctINT                         bpp;

    gcoSURF                        renderTarget;

    __DRIcontextPriv * activeContext;
    __DRIdrawablePriv * activeDrawable;

    __DRIcontextPriv * contextStack;
    __DRIdrawablePriv * drawableStack;
};

/* Structure that defines a window. */
struct _DRIWindow
{
    __DRIDisplay*      display;
    gctUINT            offset;
    gctINT             x, y;
    gctINT             width;
    gctINT             height;
    /* Color format. */
    gceSURF_FORMAT     format;
};

static gceSTATUS _CreateOnScreenSurfaceWrapper(
    __DRIdrawablePriv * drawable,
      gceSURF_FORMAT Format
)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("drawable=0x%x", drawable);

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS _DestroyOnScreenSurfaceWrapper(
    __DRIDisplay * Display
)
{
    return gcvSTATUS_OK;
}

static void _cleanAsyncFrame(asyncFrame *frame)
{
    if (!frame || !frame->dridrawable)
        return;

    if (frame->pixmapfd)
    {
        close(frame->pixmapfd);
        frame->pixmapfd = -1;
    }

    if (frame->dridrawable->display->dpy)
        xcb_sync_destroy_fence(dri_GetXCB(frame->dridrawable->display->dpy), frame->sync_fence);

    if (frame->shm_fence)
        xshmfence_unmap_shm(frame->shm_fence);

    if (frame->fence_fd >= 0)
        close(frame->fence_fd);

    if (frame->pixWrapSurf)
        gcoSURF_Destroy(frame->pixWrapSurf);

    if (frame->backPixmap)
        XFreePixmap(frame->dridrawable->display->dpy, frame->backPixmap);

    frame->fence_fd = -1;
    frame->shm_fence = (struct xshmfence *)gcvNULL;
    frame->backPixmap = (Pixmap)0;
    frame->pixWrapSurf = gcvNULL;
}

static void _setupAsyncFrame(asyncFrame *frame)
{
    int xx, yy;
    unsigned int ww, hh, bb;
    int stride;
    unsigned int depth = 24;
    Window    root;
    gceSTATUS status = gcvSTATUS_OK;
    __DRIDisplay * display;


    gctUINT32 fdhandle = 0;
    gctUINT bufferoffset = 0;


    if(frame == NULL)
        return;
    display = frame->dridrawable->display;

    if ((frame->dridrawable->w == 0) || (frame->dridrawable->h == 0))
    {
        XGetGeometry(display->dpy, frame->Drawable, &root, &xx, &yy, &ww, &hh, &bb, &depth);
        frame->dridrawable->w = ww;
        frame->dridrawable->h = hh;
    } else {
        ww = frame->dridrawable->w;
        hh = frame->dridrawable->h;
    }

    frame->w = ww;
    frame->h = hh;

    frame->fence_fd = xshmfence_alloc_shm();
    if (frame->fence_fd < 0) {
        goto OnError;
    }

    frame->shm_fence = xshmfence_map_shm(frame->fence_fd);
    if (frame->shm_fence == NULL) {
        close(frame->fence_fd);
        goto OnError;
    }
    xcb_dri3_fence_from_fd(dri_GetXCB(display->dpy),
                          frame->Drawable,
                          (frame->sync_fence = xcb_generate_id(dri_GetXCB(display->dpy))),
                          0,
                          frame->fence_fd);

    ww = gcmALIGN(ww, 16);

    frame->backPixmap = XCreatePixmap(display->dpy, frame->Drawable, ww, hh, depth);
    frame->pixmapfd = create_fd_from_pixmap(dri_GetXCB(display->dpy), frame->backPixmap, &stride);
    if (frame->pixmapfd < 0) {
        goto OnError;
    }
    xcb_flush(dri_GetXCB(display->dpy));

    fdhandle = frame->pixmapfd;

    gcmONERROR(gcoSURF_WrapUserMultiBuffer(
        gcvNULL,
        ww,
        hh,
        frame->surftype,
        frame->surfformat,
        (gctUINT *)&stride,
        &fdhandle,
        &bufferoffset,
        gcvALLOC_FLAG_DMABUF,
        &frame->pixWrapSurf
    ));


    xshmfence_reset(frame->shm_fence);
    xshmfence_trigger(frame->shm_fence);

    return;

OnError:
    _cleanAsyncFrame(frame);

#if (defined(DEBUG) || defined(_DEBUG))
    fprintf(stderr, "Warning::Backpixmap can't be created for the current Drawable\n");
#endif
}

static void
_GetDisplayInfo(
    __DRIDisplay* display,
    int scrn)
{

    Display *dpy = display->dpy;

    display->width = DisplayWidth(dpy, scrn);
    display->height = DisplayHeight(dpy, scrn);
    display->bpp = DefaultDepth(dpy, scrn);

    if ( display->bpp == 24 ) /* not support 24-bpp */
    display->bpp = 32;

    switch(display->bpp)
    {
        case 16:
            display->format = gcvSURF_R5G6B5;
            break;
        case 32:
            display->format = gcvSURF_A8R8G8B8;
            break;
        default:
            display->format = gcvSURF_UNKNOWN;
            break;
    }

    display->drmFd = dri3_open(dpy, RootWindow(dpy, scrn), None);

    if (display->drmFd < 0)
        fprintf(stderr,"Fail to open vivante drm\n");

    if (drm_vivante_create(display->drmFd, &display->drmVIV))
        fprintf(stderr,"Fail to create drm_vivante\n");
}

static __DRIcontextPriv * _FindContext(__DRIDisplay* display,
    IN gctPOINTER Context)
{
    gctBOOL found = gcvFALSE;
    __DRIcontextPriv * cur;

    cur = display->contextStack;

    while ((cur != NULL) && (!found)) {
        if (cur->eglContext == Context) {
            found = gcvTRUE;
        } else {
            cur = cur->next;
        }
    }
    return found ? cur : NULL;
}

static __DRIdrawablePriv * _FindDrawable(__DRIDisplay* display,
    IN PlatformWindowType Drawable)
{
    gctBOOL found = gcvFALSE;
    __DRIdrawablePriv * cur;

    cur = display->drawableStack;

    while ((cur != NULL) && (!found)) {
        if (cur->drawable == Drawable) {
            found = gcvTRUE;
        } else {
            cur = cur->next;
        }
    }
    return found ? cur: NULL;
}

/*******************************************************************************
** Display.
*/

static void
_GetColorBitsInfoFromMask(
    gctSIZE_T Mask,
    gctUINT *Length,
    gctUINT *Offset
)
{
    gctINT start, end;
    size_t i;

    if (Length == NULL && Offset == NULL)
    {
        return;
    }

    if (Mask == 0)
    {
        start = 0;
        end   = 0;
    }
    else
    {
        start = end = -1;

        for (i = 0; i < (sizeof(Mask) * 8); i++)
        {
            if (start == -1)
            {
                if ((Mask & (1 << i)) > 0)
                {
                    start = i;
                }
            }
            else if ((Mask & (1 << i)) == 0)
            {
                end = i;
                break;
            }
        }

        if (end == -1)
        {
            end = i;
        }
    }

    if (Length != NULL)
    {
        *Length = end - start;
    }

    if (Offset != NULL)
    {
        *Offset = start;
    }
}

static gceSTATUS
dri_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    XImage *image;
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    do
    {

        *Display = XOpenDisplay(gcvNULL);
        if (*Display == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }

        image = XGetImage(*Display,
                          DefaultRootWindow(*Display),
                          0, 0, 1, 1, AllPlanes, ZPixmap);

        if (image == gcvNULL)
        {
            /* Error. */
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }
        XDestroyImage(image);

        gcmFOOTER_ARG("*Display=0x%x", *Display);
        return status;
    }
    while (0);

    if (*Display != gcvNULL)
    {
        XCloseDisplay(*Display);
    }

    gcmFOOTER();
    return status;
}

typedef struct _driDISPLAY_INFO
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
driDISPLAY_INFO;


static gceSTATUS
dri_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT DisplayInfoSize,
    OUT driDISPLAY_INFO * DisplayInfo
    )
{
    Screen * screen;
    XImage *image;

    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);
    /* Valid display? and structure size? */
    if ((Display == gcvNULL) || (DisplayInfoSize != sizeof(driDISPLAY_INFO)))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    image = XGetImage(Display,
        DefaultRootWindow(Display),
        0, 0, 1, 1, AllPlanes, ZPixmap);

    if (image == gcvNULL)
    {
        /* Error. */
        status = gcvSTATUS_NOT_SUPPORTED;
        gcmFOOTER();
        return status;
    }

    _GetColorBitsInfoFromMask(image->red_mask, &DisplayInfo->redLength, &DisplayInfo->redOffset);
    _GetColorBitsInfoFromMask(image->green_mask, &DisplayInfo->greenLength, &DisplayInfo->greenOffset);
    _GetColorBitsInfoFromMask(image->blue_mask, &DisplayInfo->blueLength, &DisplayInfo->blueOffset);

    XDestroyImage(image);

    screen = XScreenOfDisplay(Display, DefaultScreen(Display));

    /* Return the size of the display. */
    DisplayInfo->width  = XWidthOfScreen(screen);
    DisplayInfo->height = XHeightOfScreen(screen);

    /* The stride of the display is not known in the X environment. */
    DisplayInfo->stride = ~0;

    DisplayInfo->bitsPerPixel = image->bits_per_pixel;

    /* Get the color info. */
    DisplayInfo->alphaLength = image->bits_per_pixel - image->depth;
    DisplayInfo->alphaOffset = image->depth;

    /* The logical address of the display is not known in the X
    ** environment. */
    DisplayInfo->logical = NULL;

    /* The physical address of the display is not known in the X
    ** environment. */
    DisplayInfo->physical = ~0;

    /* No flip. */
    DisplayInfo->flip = 0;

    DisplayInfo->wrapFB = gcvTRUE;

    /* Success. */
    gcmFOOTER_ARG("*DisplayInfo=0x%x", *DisplayInfo);
    return status;
}

static gceSTATUS
dri_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_SetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT Interval
)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_GetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    if (Display != gcvNULL)
    {
        XCloseDisplay(Display);
    }
    return gcvSTATUS_OK;
}

/*******************************************************************************
** Context
*/
static gceSTATUS
dri_CreateContext(IN gctPOINTER localDisplay, IN gctPOINTER Context)
{
    __DRIcontextPriv *context;
    __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;


    gcmHEADER_ARG("localDisplay=0x%x, Context=0x%x\n", localDisplay, Context);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    display = (__DRIDisplay*)localDisplay;

    context = (__DRIcontextPriv *)malloc(sizeof(__DRIcontextPriv));
    if (!context) {
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    memset(context, 0, sizeof(__DRIcontextPriv));

    context->eglContext = Context;
    context->next = display->contextStack;
    display->contextStack = context;

OnError:
    /* Success. */
    gcmFOOTER_ARG("*context=0x%x", context);
    return status;
}

static gceSTATUS
dri_DestroyContext(IN gctPOINTER localDisplay, IN gctPOINTER Context)
{
    __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;
    __DRIcontextPriv            *cur;
    __DRIcontextPriv            *prev;
    gctBOOL        found = gcvFALSE;


    gcmHEADER_ARG("localDisplay=0x%x, Context=0x%x\n", localDisplay, Context);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    display = (__DRIDisplay*)localDisplay;
    prev = cur = display->contextStack;
    while ((cur != NULL) && (!found)) {
        if (cur->eglContext == Context) {
            found = gcvTRUE;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
    if (found) {
        if (display->contextStack == cur) {
            display->contextStack = cur->next;
        } else {
            prev->next = cur->next;
        }
        free(cur);
    }
    _DestroyOnScreenSurfaceWrapper(display);

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
dri_MakeCurrent(IN gctPOINTER localDisplay,
    IN PlatformWindowType DrawDrawable,
    IN PlatformWindowType ReadDrawable,
    IN gctPOINTER Context,
    IN gcoSURF ResolveTarget)
{
   __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;
    __DRIdrawablePriv *drawable;
    gceSURF_FORMAT format;

    gcmHEADER_ARG("localDisplay=0x%x, Drawable = 0x%x, Readable = 0x%x Context=0x%x\n", localDisplay, DrawDrawable, ReadDrawable, Context);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    display = (__DRIDisplay*)localDisplay;

    display->activeContext = _FindContext(display, Context);
    display->activeDrawable = _FindDrawable(display, DrawDrawable);
    if (display->activeContext && display->activeDrawable) {
         display->activeContext->drawablePriv = display->activeDrawable;
         display->activeDrawable->contextPriv = display->activeContext;
         drawable = display->activeDrawable;
    } else {
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    gcoSURF_GetFormat(ResolveTarget, gcvNULL,&format );

    _CreateOnScreenSurfaceWrapper(drawable, format);

OnError:
    gcmFOOTER();
    return status;
}


/*******************************************************************************
** Drawable
*/
static gceSTATUS
dri_CreateDrawable(IN gctPOINTER localDisplay, IN PlatformWindowType Drawable)
{
    __DRIdrawablePriv *drawable;
    gctUINT32 index = 0;

    __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;
    xcb_connection_t *con;

    gcmHEADER_ARG("localDisplay=0x%x, Drawable=0x%x\n", localDisplay, Drawable);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    display = (__DRIDisplay*)localDisplay;

    drawable = (__DRIdrawablePriv *)malloc(sizeof(__DRIdrawablePriv));
    if (!drawable) {
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    memset(drawable, 0, sizeof(__DRIdrawablePriv));

    drawable->drawable = Drawable;
    drawable->refcount = 0;
    drawable->Stamp = 0;
    drawable->lastStamp = 0;
    drawable->index = 0;
    drawable->x = 0;
    drawable->y = 0;
    drawable->w = 0;
    drawable->h = 0;

    drawable->display = display;

    drawable->screen = display->screen;
    drawable->backNode = 0;
    drawable->nodeName = 0;

    drawable->next = display->drawableStack;
    display->drawableStack = drawable;


    drawable->oldx = -1;
    drawable->oldy = -1;
    drawable->oldw = 0;
    drawable->oldh = 0;
    drawable->olddrawable = (PlatformWindowType)0;

    for (index = 0; index < NUM_ASYNCFRAME; index++)
    {
        drawable->ascframe[index].fence_fd = -1;
        drawable->ascframe[index].pixmapfd = -1;
    }

    con = dri_GetXCB(display->dpy);
    drawable->eid = xcb_generate_id(con);
    xcb_present_select_input(con, drawable->eid, drawable->drawable, XCB_PRESENT_EVENT_MASK_CONFIGURE_NOTIFY | XCB_PRESENT_EVENT_MASK_IDLE_NOTIFY | XCB_PRESENT_EVENT_MASK_COMPLETE_NOTIFY);
    drawable->cq = xcb_register_for_special_xge(con, &xcb_present_id, drawable->eid, &drawable->Stamp);
    drawable->lastStamp = drawable->Stamp;

OnError:
    /* Success. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
dri_DestroyDrawable(IN gctPOINTER localDisplay, IN PlatformWindowType Drawable)
{
    __DRIdrawablePriv *cur;
    __DRIdrawablePriv *prev;
    __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL found = gcvFALSE;
    int index = 0;

    gcmHEADER_ARG("localDisplay=0x%x, Drawable=0x%x\n", localDisplay, Drawable);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    display = (__DRIDisplay*)localDisplay;
    prev = cur = display->drawableStack;
    while ((cur != NULL) && (!found)) {
        if (cur->drawable == Drawable) {
            found = gcvTRUE;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
    if (found) {

        cur->backNode = 0;
        cur->nodeName = 0;

       for (index = 0; index < NUM_ASYNCFRAME; index++)
       {
           _cleanAsyncFrame(&cur->ascframe[index]);
       }

       if (cur->cq)
       {
           xcb_unregister_for_special_event(dri_GetXCB(display->dpy), cur->cq);
           cur->cq = NULL;
       }

        if (display->drawableStack == cur) {
            display->drawableStack = cur->next;
        } else {
            prev->next = cur->next;
        }
        free(cur);
    }

OnError:
    /* Success. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
** Windows
*/

static gceSTATUS
dri_GetWindowInfo(
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
    XWindowAttributes attr;
    XImage *image;
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);
    if (Window == 0)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    XGetWindowAttributes(Display, Window, &attr);

    if (X != NULL)
    {
        *X = attr.x;
    }

    if (Y != NULL)
    {
        *Y = attr.y;
    }

    if (Width != NULL)
    {
        *Width = attr.width;
    }

    if (Height != NULL)
    {
        *Height = attr.height;
    }

    if (BitsPerPixel != NULL)
    {
        image = XGetImage(Display,
            DefaultRootWindow(Display),
            0, 0, 1, 1, AllPlanes, ZPixmap);

        if (image != NULL)
        {
            *BitsPerPixel = image->bits_per_pixel;
            XDestroyImage(image);
        }
    }

    if (Offset != NULL)
    {
        *Offset = 0;
    }

    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/

static gceSTATUS
dri_DrawPixmap(
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
    Screen* xScreen;
    Visual* xVisual;
    gctINT     status;
    XImage* xImage = gcvNULL;
    GC gc = NULL;

    Drawable pixmap = Pixmap;
    gceSTATUS eStatus = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x Left=%d Top=%d Right=%d Bottom=%d Width=%d Height=%d BitsPerPixel=%d Bits=0x%x",
        Display, Pixmap, Left, Top, Right, Bottom, Width, Height, BitsPerPixel, Bits);

    do
    {
        /* Create graphics context. */
        gc = XCreateGC(Display, pixmap, 0, gcvNULL);

        /* Fetch defaults. */
        xScreen = DefaultScreenOfDisplay(Display);
        xVisual = DefaultVisualOfScreen(xScreen);

        /* Create image from the bits. */
        xImage = XCreateImage(Display,
            xVisual,
            BitsPerPixel,
            ZPixmap,
            0,
            (char*) Bits,
            Width,
            Height,
            8,
            Width * BitsPerPixel / 8);

        if (xImage == gcvNULL)
        {
            /* Error. */
            eStatus = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }

        /* Draw the image. */
        status = XPutImage(Display,
            Pixmap,
            gc,
            xImage,
            Left, Top,           /* Source origin. */
            Left, Top,           /* Destination origin. */
            Right - Left, Bottom - Top);

        if (status != Success)
        {
            /* Error. */
            eStatus = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Flush buffers. */
        status = XFlush(Display);

        if (status != Success)
        {
            /* Error. */
            eStatus = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }
    }
    while (0);

    /* Destroy the image. */
    if (xImage != gcvNULL)
    {
        xImage->data = gcvNULL;
        XDestroyImage(xImage);
    }

    /* Free graphics context. */
    XFreeGC(Display, gc);

    /* Return result. */
    gcmFOOTER();
    return eStatus;
}

static gceSTATUS
dri_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gctINT          screen;
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER();
    __DRIDisplay* display;

    do
    {
        display = (__DRIDisplay*) malloc(sizeof(__DRIDisplay));

        if (display == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }
        memset(display, 0, sizeof(__DRIDisplay));

        display->dpy = Display;
        display->contextStack = NULL;
        display->drawableStack = NULL;

        screen = DefaultScreen(Display);

        _GetDisplayInfo(display, screen);

        *localDisplay = (gctPOINTER)display;
        gcmFOOTER_ARG("*localDisplay=0x%x", *localDisplay);
        return status;
    }
    while (0);

    gcmFOOTER();
    return status;
}

static gceSTATUS
dri_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    __DRIDisplay* display;

    display = (__DRIDisplay*)*localDisplay;
    if (display != gcvNULL)
    {
        drm_vivante_close(display->drmVIV);
        free(display);
        *localDisplay = gcvNULL;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
dri_GetDisplayInfoEx2(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT driDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = dri_GetDisplayInfoEx(Display, Window, DisplayInfoSize, DisplayInfo);
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

static gceSTATUS
dri_IsValidDisplay(
    IN PlatformDisplayType Display
    )
{
    if(Display != gcvNULL)
        return gcvSTATUS_OK;
    return gcvSTATUS_INVALID_ARGUMENT;
}

static gctBOOL
dri_SynchronousFlip(
    IN PlatformDisplayType Display
    )
{
    return gcvFALSE;
}

static gceSTATUS
dri_GetNativeVisualId(
    IN PlatformDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = (gctINT)
        DefaultVisual(Display,DefaultScreen(Display))->visualid;
    return gcvSTATUS_OK;
}

static gceSTATUS
dri_GetWindowInfoEx(
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
    driDISPLAY_INFO info;

    if (gcmIS_ERROR(dri_GetWindowInfo(
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

    if (gcmIS_ERROR(dri_GetDisplayInfoEx(
        Display,
        Window,
        sizeof(info),
        &info)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

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

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
dri_SetWindowFormat(
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

static gceSTATUS
dri_GetPixmapInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctINT * FD,
    OUT gceSURF_FORMAT * Format
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    Window rootWindow = 0;
    gctINT x = 0, y = 0;
    gctUINT width = 0, height = 0, borderWidth = 0, bitsPerPixel = 0;
    int pixmapfd = -1, stride = 0;

    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    if (Pixmap == 0)
    {
        /* Pixmap is not a valid pixmap data structure pointer. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Query pixmap parameters. */
    if (XGetGeometry(Display, Pixmap, &rootWindow,
                     &x, &y, &width, &height,
                     &borderWidth, &bitsPerPixel) == False)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Stride || FD)
    {
        pixmapfd = create_fd_from_pixmap(dri_GetXCB(Display), Pixmap, &stride);
        xcb_flush(dri_GetXCB(Display));
        if (pixmapfd < 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Set back values. */
    if (Width)
    {
        *Width = width;
    }

    if (Height)
    {
        *Height = height;
    }

    if (BitsPerPixel)
    {
        *BitsPerPixel = bitsPerPixel;
    }

    if (Stride)
    {
        *Stride = stride;
    }

    if (FD)
    {
        *FD = pixmapfd;
    }

    if (Format)
    {
        /* Return format for pixmap depth. */
        switch (bitsPerPixel)
        {
        case 16:
            *Format = gcvSURF_R5G6B5;
            break;
        case 24:
            *Format = gcvSURF_X8R8G8B8;
            break;
        case 32:
            *Format = gcvSURF_A8R8G8B8;
            break;

        default:
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
dri_CopyPixmapBits(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    OUT gctPOINTER DstBits
    )
{
    gctINT status = gcvSTATUS_OK;
    Window rootWindow;
    XImage *img = 0;
    int x = 0, y = 0;
    unsigned int  w = 0, h = 0, bitsPerPixel = 0, borderWidth = 0;

    switch (DstFormat)
    {
    case gcvSURF_R5G6B5:
        break;

    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8R8G8B8:
        break;

    default:
       printf("dri_CopyPixmapBits error format\n");
        return gcvSTATUS_INVALID_ARGUMENT;
    }

   if (XGetGeometry(Display,
        Pixmap,
        &rootWindow,
        &x, &y, &w, &h,
        &borderWidth,
        &bitsPerPixel) == False)
    {
        printf("dri_CopyPixmapBits error\n");
        status = gcvSTATUS_INVALID_ARGUMENT;
        return status;
    }
    else
    {
        img = XGetImage(Display, Pixmap, x, y, w, h, AllPlanes, ZPixmap);
    }

    if (img && DstBits)
    {
         gctUINT8* src = (gctUINT8*)img->data + (img->xoffset * img->bits_per_pixel >> 3);
         gctUINT8* dst = (gctUINT8*)DstBits;
         gctINT SrcStride = img->bytes_per_line;
         gctINT len = gcmMIN(DstStride, SrcStride);
         gctINT height = gcmMIN(h, DstHeight);

         if (SrcStride == DstStride)
         {
             gcoOS_MemCopy(dst, src, len * height);
         }
         else
         {
             gctUINT n;
             for (n = 0; n < height; n++)
             {
                 gcoOS_MemCopy(dst, src, len);
                 src += SrcStride;
                 dst += DstStride;
             }
         }
    }

    if (img) XDestroyImage(img);

    return status;
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
    dri_GetDisplay(&display, gcvNULL);

    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    dri_DestroyDisplay((PlatformDisplayType) Display);
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (gcmIS_ERROR(dri_IsValidDisplay((PlatformDisplayType) Display)))
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

    status = dri_InitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    status = dri_DeinitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
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

    dri_GetNativeVisualId((PlatformDisplayType) Display->hdc, &visualId);
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
    status = dri_GetSwapInterval((PlatformDisplayType) Display->hdc,
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

    status = dri_SetSwapInterval((PlatformDisplayType) Display->hdc,
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

    /* Information obtained by dri_GetDisplayInfoEx2. */
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
    driDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = dri_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
    gcoOS_ZeroMemory(&dInfo, sizeof (driDISPLAY_INFO));

    status = dri_GetDisplayInfoEx2((PlatformDisplayType) Display->hdc,
                                     (PlatformWindowType) Window,
                                     Display->localInfo,
                                     sizeof (driDISPLAY_INFO),
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
    status = dri_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
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
    dri_CreateDrawable(Display->localInfo, (PlatformWindowType) win);

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

    dri_DestroyDrawable(Display->localInfo, (PlatformWindowType) win);

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

    status = dri_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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
            status = dri_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                    status = dri_SetWindowFormat((PlatformDisplayType) Display->hdc,
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

                    status = dri_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
                status = dri_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
            status = dri_SetWindowFormat((PlatformDisplayType) Display->hdc,
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
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;


    /* Get shortcut. */
    void * win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

    driDisplay = (__DRIDisplay*)Display->localInfo;
    drawable = _FindDrawable(driDisplay, (PlatformWindowType)win);

    if (drawable)
    {
        int i = 0;
        for(i = 0; i< NUM_ASYNCFRAME;i++)
        {
            if (drawable->ascframe[i].backPixmap != (Pixmap)0)
            {
                width  = drawable->w;
                height = drawable->h;
                goto ENDSIZE;
            }
        }
    }

    if (gcmIS_ERROR(dri_GetWindowInfo(
                          (PlatformDisplayType) Display->hdc,
                          (PlatformWindowType) win,
                          gcvNULL,
                          gcvNULL,
                          (gctINT_PTR) &width,
                          (gctINT_PTR) &height,
                          gcvNULL,
                          gcvNULL)))
    {
        return EGL_FALSE;
    }

    if ((Surface->config.width  == width) &&
    (Surface->config.height == height)
    )
    {
            goto ENDSIZE;
    }

    status = dri_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
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

ENDSIZE:

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
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    VEGLWindowInfo wininfo = gcvNULL;

    int index = 0;
    struct dma_buf_sync sync_args;

    xcb_connection_t *con;

    unsigned int schanged = 0;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    if (Display == gcvNULL || Display->localInfo == gcvNULL )
    {
        return EGL_FALSE;
    }

    wininfo = Surface->winInfo;

    driDisplay = (__DRIDisplay*)Display->localInfo;
    drawable = _FindDrawable(driDisplay, (PlatformWindowType)Surface->hwnd);

    con = dri_GetXCB((void *)driDisplay->dpy);

    while (1) {
        for (index = 0; index < NUM_ASYNCFRAME; index++)
        {
            if (drawable->busyframe[index] == _FRAME_FREE)
                break;
        }

        if (index == NUM_ASYNCFRAME)
        {
            xcb_present_generic_event_t *ev;
            ev = (xcb_present_generic_event_t *)xcb_wait_for_special_event(con, drawable->cq);
            if (ev == NULL)
            {
                fprintf(stderr,"Failed to wait for xcb event\n");
                abort();
            }

            do {
                switch (ev->evtype) {
                case XCB_PRESENT_CONFIGURE_NOTIFY:
                {
                 xcb_present_configure_notify_event_t *ce = (void *) ev;

                 if ( ce->width != drawable->w || ce->height != drawable->h)
                     schanged = 1;

                 drawable->w = ce->width;
                 drawable->h = ce->height;
                 }
                 break;
                case XCB_PRESENT_COMPLETE_NOTIFY:
                    break;
                case XCB_PRESENT_EVENT_IDLE_NOTIFY:
                    {
                        xcb_present_idle_notify_event_t *ie = (xcb_present_idle_notify_event_t *)ev;
                        for (index = 0; index < NUM_ASYNCFRAME; index++) {
                            if (drawable->ascframe[index].backPixmap == ie->pixmap) {
                                drawable->busyframe[index] = _FRAME_FREE;
                                break;
                            }
                        }
                        break;
                    }
                }

                free(ev);
            } while ((ev = (xcb_present_generic_event_t *)xcb_poll_for_special_event(con, drawable->cq)));

            continue;
        }


        break;
    }

    if (drawable->ascframe[index].backPixmap == (Pixmap)0)
    {
        drawable->ascframe[index].dridrawable = drawable;
        drawable->ascframe[index].surftype = gcvSURF_BITMAP;
        drawable->ascframe[index].surfformat = wininfo->format;
        drawable->ascframe[index].pixWrapSurf = gcvNULL;
        drawable->ascframe[index].Drawable = (PlatformWindowType)Surface->hwnd;
        drawable->ascframe[index].backNode = 0;
        _setupAsyncFrame(&drawable->ascframe[index]);
    }
    else
    {
       if (schanged)
       {
           _cleanAsyncFrame(&drawable->ascframe[index]);
            drawable->ascframe[index].dridrawable = drawable;
            drawable->ascframe[index].surftype = gcvSURF_BITMAP;
            drawable->ascframe[index].surfformat = wininfo->format;
            drawable->ascframe[index].pixWrapSurf = gcvNULL;
            drawable->ascframe[index].Drawable = (PlatformWindowType)Surface->hwnd;
            drawable->ascframe[index].backNode = 0;
            _setupAsyncFrame(&drawable->ascframe[index]);
            schanged = 0;
       }
    }

    drawable->busyframe[index] = _FRAME_BUSY;

    sync_args.flags = DMA_BUF_SYNC_START;
    ioctl(drawable->ascframe[index].pixmapfd, DMA_BUF_IOCTL_SYNC, sync_args);

    BackBuffer->surface  = drawable->ascframe[index].pixWrapSurf;
    BackBuffer->context  = (gctPOINTER)drawable;
    BackBuffer->origin.x = 0;
    BackBuffer->origin.y = 0;
    BackBuffer->flip     = gcvTRUE;
    return EGL_TRUE;
}

typedef Display *X11DISPLAY;
static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN struct eglRegion * Region,
    IN struct eglRegion * DamageHint
    )
{
    gcoSURF surface;
    __DRIdrawablePriv * drawable;
    int index = 0;
    unsigned int present_flags = XCB_PRESENT_OPTION_ASYNC;

    static X11DISPLAY    dpy = gcvNULL;
    static int firsttime = 1;

    asyncFrame *frame = NULL;
    xcb_xfixes_region_t update = 0;

    struct dma_buf_sync sync_args;

    static uint32_t serial = 0;


    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    (void) surface;


    drawable = (__DRIdrawablePriv *)BackBuffer->context;

    for (index = 0; index < NUM_ASYNCFRAME; index++)
    {
        if (drawable->ascframe[index].pixWrapSurf == BackBuffer->surface)
            break;
    }

    gcmASSERT(index < NUM_ASYNCFRAME);

    if (firsttime)
    {
        XInitThreads();
        dpy = XOpenDisplay(NULL);
    }

    firsttime = 0;

    frame = &drawable->ascframe[index];


    sync_args.flags = DMA_BUF_SYNC_END;

    ioctl(frame->pixmapfd, DMA_BUF_IOCTL_SYNC, sync_args);


    if (frame->fence_fd) {
    xshmfence_await(frame->shm_fence);
    xshmfence_reset(frame->shm_fence);
    }

    xcb_present_pixmap(dri_GetXCB(dpy), frame->Drawable, frame->backPixmap,
    serial++,
    0, /* valid */
    update, /* update */
    0, /* x_off */
    0, /* y_off */
    None,
    None, /* wait fence */
    frame->sync_fence,
    present_flags,
    0, /* target msc */
    0, /* divisor */
    0, /* remainder */
    0, (xcb_present_notify_t *)NULL);

    xcb_flush(dri_GetXCB(dpy));
    return EGL_TRUE;
}

static EGLBoolean
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    __DRIdrawablePriv * drawable;
    int index = 0;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);


    drawable = (__DRIdrawablePriv *)BackBuffer->context;

    for (index = 0; index < NUM_ASYNCFRAME; index++)
    {
        if (drawable->ascframe[index].pixWrapSurf == BackBuffer->surface)
            break;
    }

    gcmASSERT( index < NUM_ASYNCFRAME);

    drawable->busyframe[index] = _FRAME_FREE;

    return EGL_TRUE;
}

static EGLBoolean
_SynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return dri_SynchronousFlip((PlatformDisplayType)Display->hdc);
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    __DRIdrawablePriv * drawable;
    int index = 0;
    drawable = (__DRIdrawablePriv *)BackBuffer->context;

    for (index = 0; index < NUM_ASYNCFRAME; index++)
    {
        if (drawable->ascframe[index].age)
            drawable->ascframe[index].age++;
    }

    for (index = 0; index < NUM_ASYNCFRAME; index++)
    {
        if (drawable->ascframe[index].pixWrapSurf == BackBuffer->surface)
            break;
    }

    gcmASSERT( index < NUM_ASYNCFRAME);
    drawable->ascframe[index].age = 1;
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
    __DRIdrawablePriv * drawable;
    int index = 0;
    __DRIDisplay* driDisplay;

    if (BackBuffer && BackBuffer->context)
    {
        drawable = (__DRIdrawablePriv *)BackBuffer->context;

        for (index = 0; index < NUM_ASYNCFRAME; index++)
        {
            if (drawable->ascframe[index].pixWrapSurf == BackBuffer->surface)
                break;
        }
        *BufferAge = drawable->ascframe[index].age;
        return EGL_TRUE;
    }
    else if (!Surface->newSwapModel)
    {
        /*
         * back buffers are returned in order.
         * It's safe to return age of buffer to dequeue next time --- except that
         * there're 0 aged buffers.
         */
        driDisplay = (__DRIDisplay*)Display->localInfo;
        drawable = _FindDrawable(driDisplay, (PlatformWindowType)Surface->hwnd);

        for (index = 0; index < NUM_ASYNCFRAME; index++)
        {
            if (drawable->ascframe[index].age == 0)
            {
                *BufferAge = 0;
                break;
            }
        }

        return EGL_TRUE;
    }

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
        gcmONERROR(dri_CopyPixmapBits((PlatformDisplayType) Info->hdc,
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
        gcmONERROR(dri_DrawPixmap((PlatformDisplayType) Info->hdc,
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
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = dri_GetPixmapInfoEx((PlatformDisplayType)Display->hdc,
                                 (PlatformPixmapType)Pixmap,
                                 gcvNULL, gcvNULL, gcvNULL,
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
    gctBOOL needShadow = gcvTRUE;
    gctINT pixmapWidth;
    gctINT pixmapHeight;
    gctINT pixmapStride = 0;
    gceSURF_FORMAT pixmapFormat;
    gctINT pixmapBpp;
    gctINT pixmapFD = 0;
    gctUINT bufferoffset = 0;
    gctUINT32 physical[3] = {0};
    gctPOINTER logical[3] = {0};
    gcoSURF wrapper = gcvNULL;
    gcoSURF shadow = gcvNULL;
    VEGLPixmapInfo info = gcvNULL;

    /* Query pixmap geometry info. */
    gcmONERROR(dri_GetPixmapInfoEx((PlatformDisplayType)Display->hdc,
                                   (PlatformPixmapType)Pixmap,
                                   &pixmapWidth,
                                   &pixmapHeight,
                                   &pixmapBpp,
                                   &pixmapStride,
                                   &pixmapFD,
                                   &pixmapFormat));

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
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    gcmONERROR(gcoSURF_WrapUserMultiBuffer(
            gcvNULL,
            pixmapWidth,
            pixmapHeight,
            gcvSURF_BITMAP,
            pixmapFormat,
            (gctUINT*)&pixmapStride,
            (gctUINT32*)&pixmapFD,
            &bufferoffset,
            gcvALLOC_FLAG_DMABUF,
            &wrapper
        ));
    close(pixmapFD);
    gcmONERROR(gcoSURF_Lock(wrapper, physical, logical));

    do
    {

        if ((pixmapStride * 8 / pixmapBpp) < 16)
        {
            /* Too small in width. */
            break;
        }

        /* Height needs to be 4 aligned or vstride is large enough. */
        if (pixmapHeight & 3)
        {
            /* No enough memory in height.
            ** Resolve may exceeds the buffer and overwrite other memory.
            */
            break;
        }

        if (physical[0] & 0x3F)
        {
            /* Not 64 byte aligned. */
            break;
        }

        needShadow = gcvFALSE;
    }
    while (gcvFALSE);

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
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct eglPixmapInfo), (gctPOINTER*)&info));

    gcoOS_ZeroMemory(info, sizeof(struct eglPixmapInfo));
    /* Save pixmap info. */
    info->width        = pixmapWidth;
    info->height       = pixmapHeight;
    info->format       = pixmapFormat;
    info->stride       = pixmapStride;
    info->bitsPerPixel = pixmapBpp;
    info->data         = logical[0];
    info->hdc          = (PlatformDisplayType)Display->hdc;
    info->wrapper      = wrapper;
    info->shadow       = shadow;

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p wrapper=%p shadow=%p",
             __FUNCTION__, __LINE__, Display, Pixmap, wrapper, shadow);

    /* Output. */
    *Info    = info;
    *Surface = shadow ? shadow : wrapper;

    return EGL_TRUE;

OnError:
    if (wrapper)
    {
        if (logical[0])
        {
            gcmVERIFY_OK(gcoSURF_Unlock(wrapper, gcvNULL));
        }

        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
    }

    if (shadow)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(shadow));
    }

    if (info)
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
        gcmVERIFY_OK(gcoSURF_Unlock(Info->wrapper, gcvNULL));
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
    gctINT width, height;

    /* Query pixmap info again. */
    gcmONERROR(
        dri_GetPixmapInfoEx((PlatformDisplayType)Display->hdc,
                            (PlatformPixmapType)Pixmap,
                            &width,
                            &height,
                            gcvNULL,
                            gcvNULL,
                            gcvNULL,
                            gcvNULL));

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

static EGLBoolean
_CreateContext(
    void * LocalDisplay,
    void * Context
    )
{
    gceSTATUS status;

    status = dri_CreateContext(LocalDisplay, Context);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static void
_DestroyContext(
    void * Display,
    void * Context
    )
{
    dri_DestroyContext(Display, Context);
}

static EGLBoolean
_MakeCurrent(
    void * LocalDisplay,
    void * DrawDrawable,
    void * ReadDrawable,
    void * Context,
    gcoSURF ResolveTarget
    )
{
    gceSTATUS status;

    status = dri_MakeCurrent(LocalDisplay,
                             (PlatformWindowType) DrawDrawable,
                             (PlatformWindowType) ReadDrawable,
                             Context,
                             ResolveTarget);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static struct eglPlatform driPlatform =
{
    EGL_PLATFORM_DRI3_VIV,

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

    _CreateContext,
    _DestroyContext,
    _MakeCurrent,
};

extern VEGLPlatform
_veglGetDRIPlatform(
    void * NativeDisplay
    );

static int check_dri3(xcb_connection_t *con) {
    const xcb_query_extension_reply_t *ext;

    xcb_prefetch_extension_data (con, &xcb_dri3_id);
    xcb_prefetch_extension_data (con, &xcb_present_id);
    ext = xcb_get_extension_data(con, &xcb_dri3_id);
    if (!(ext && ext->present))
        return 0;
    ext = xcb_get_extension_data(con, &xcb_present_id);
    if (!(ext && ext->present))
        return 0;
    return 1;
}

VEGLPlatform
veglGetX11Platform(
    void * NativeDisplay
    )
{
    int dri3support = 0;
    if (NativeDisplay == NULL)
        dri3support = check_dri3(dri_GetXCB(XOpenDisplay(NULL)));
    else
        dri3support = check_dri3(dri_GetXCB(NativeDisplay));
    if (dri3support)
        return &driPlatform;

    return NULL;
}
