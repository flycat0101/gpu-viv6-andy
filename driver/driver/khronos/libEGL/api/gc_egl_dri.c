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
#include <drm_sarea.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xdamage.h>
#include "drmgl.h"
#include <xf86dri.h>

#include <gc_hal_types.h>
#include <gc_hal_base.h>
#include "gc_egl_platform.h"

#define _GC_OBJ_ZONE    gcvZONE_OS
#if !defined(SAREA_MAX)
#define SAREA_MAX                       0x2000
#endif

typedef Display *PlatformDisplayType;
typedef Window   PlatformWindowType;
typedef Pixmap   PlatformPixmapType;

typedef struct __DRIcontextRec  __DRIcontextPriv;
typedef struct __DRIdrawableRec __DRIdrawablePriv;
typedef struct __DRIDisplayRec  __DRIDisplay;


static void _VivGetLock(__DRIdrawablePriv * drawable);
static void _UpdateDrawableInfoDrawableInfo(__DRIdrawablePriv * drawable);

static pthread_mutex_t drmMutex = PTHREAD_MUTEX_INITIALIZER;


#define LINUX_LOCK_FRAMEBUFFER( context )                       \
    do {                                                        \
        pthread_mutex_lock(&drmMutex);                          \
        if (!context->initialized) {                            \
            _VivGetLock( context->drawablePriv);                \
            context->initialized = 1;                           \
        } else {                                                \
               _VivGetLock( context->drawablePriv );            \
        }                                                       \
    } while (0)

#define LINUX_UNLOCK_FRAMEBUFFER( context )                     \
    do {                                                        \
        pthread_mutex_unlock(&drmMutex);                        \
    } while (0)


#define DRI_VALIDATE_EXTRADRAWABLE_INFO_ONCE(pDrawPriv)              \
    do {                                                        \
            _UpdateDrawableInfoDrawableInfo(pDrawPriv);             \
    } while (0)


#define DRI_VALIDATE_DRAWABLE_INFO(pdp)                                 \
 do {                                                                   \
        DRI_VALIDATE_EXTRADRAWABLE_INFO_ONCE(pdp);        \
    } while (0)


#define DRI_FULLSCREENCOVERED(pdp)            \
 do {                                                                   \
        _FullScreenCovered(pdp);                        \
} while (0)



struct __DRIcontextRec {
    gctPOINTER eglContext;
    /*
    ** Kernel context handle used to access the device lock.
    */
    gctINT fd;
    __DRIid contextID;

    /*
    ** Kernel context handle used to access the device lock.
    */
    drm_context_t hHWContext;
    drm_hw_lock_t *hwLock;
    gctUINT lockCnt;
    gctBOOL initialized;

    /*
    ** Pointer to drawable currently bound to this context.
    */
    __DRIdrawablePriv *drawablePriv;
    __DRIcontextPriv            *next;
};

#ifdef DRI_PIXMAPRENDER_ASYNC

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
    GC xgc;
    gcoSURF pixWrapSurf;
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctUINT32 backNode;
    int w;
    int h;
    gctSIGNAL signal;
    int busy;
} asyncFrame;
#endif

typedef gceSTATUS (*RSFUNC)(gcoSURF, gcoSURF, gcsPOINT_PTR, gcsPOINT_PTR, gcsPOINT_PTR);

struct __DRIdrawableRec {
    /**
     * X's drawable ID associated with this private drawable.
     */
    PlatformWindowType drawable;
    /**
     * Kernel drawable handle
     */
    drm_drawable_t hHWDrawable;

    gctINT fd;

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
    gctUINT *pStamp;

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
    gctINT numClipRects;
    drm_clip_rect_t *pClipRects;

    gctINT drawLockID;

    gctINT backX;
    gctINT backY;
    gctINT backClipRectType;
    gctINT numBackClipRects;
    gctINT wWidth;
    gctINT wHeight;
    gctINT xWOrigin;
    gctINT yWOrigin;
    drm_clip_rect_t *pBackClipRects;

    /* Back buffer information */
    gctUINT nodeName;
    gctUINT backNode;
    gctUINT backBufferPhysAddr;
    gctUINT * backBufferLogicalAddr;

    __DRIcontextPriv *contextPriv;

    __DRIDisplay * display;
    gctINT screen;
    gctBOOL drawableResize;
    gctBOOL fullScreenMode;
    __DRIdrawablePriv * next;
    int fullscreenCovered;

#ifdef DRI_PIXMAPRENDER
    Pixmap backPixmap;
    GC xgc;
    gcoSURF pixWrapSurf;
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctUINT32 backPixmapNode;
#else

#ifdef DRI_PIXMAPRENDER_ASYNC
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctPOINTER frameMutex;
    gctHANDLE workerThread;
    asyncFrame ascframe[NUM_ASYNCFRAME];
    int curIndex;
    int busyNum;
    gctSIGNAL busySIG;
    gctSIGNAL stopSIG;
    gctSIGNAL exitSIG;
#endif

/* only for VG 2D, otherwise should be NULL */
    RSFUNC rsFunc;
#endif

    gctINT oldx;
    gctINT oldy;
    gctUINT oldw;
    gctUINT oldh;
    PlatformWindowType olddrawable;
    gctBOOL vgapi;
};


/* Structure that defines a display. */
struct __DRIDisplayRec
{
    gctINT                         drmFd;
    Display                        *dpy;
    drm_sarea_t                    *pSAREA;
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


#define VIV_EXT
#if defined(VIV_EXT)

#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>

#define X_VIVEXTQueryVersion            0
#define X_VIVEXTPixmapPhysaddr          1
#define X_VIVEXTDrawableFlush           2
#define X_VIVEXTDrawableInfo            3
#define X_VIVEXTFULLScreenInfo          4


#define VIVEXTNumberEvents              0

#define VIVEXTClientNotLocal            0
#define VIVEXTOperationNotSupported     1
#define VIVEXTNumberErrors        (VIVEXTOperationNotSupported + 1)



#define VIVEXTNAME "vivext"


#define VIVEXT_MAJOR_VERSION    1
#define VIVEXT_MINOR_VERSION    0
#define VIVEXT_PATCH_VERSION    0

typedef struct _VIVEXTQueryVersion {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
} xVIVEXTQueryVersionReq;
#define sz_xVIVEXTQueryVersionReq    4


typedef struct {
    BYTE      type;/* X_Reply */
    BYTE      pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    CARD16    majorVersion B16;        /* major version of vivEXT protocol */
    CARD16    minorVersion B16;        /* minor version of vivEXT protocol */
    CARD32    patchVersion B32;        /* patch version of vivEXT protocol */
    CARD32    pad3 B32;
    CARD32    pad4 B32;
    CARD32    pad5 B32;
    CARD32    pad6 B32;
} xVIVEXTQueryVersionReply;
#define sz_xVIVEXTQueryVersionReply    32


typedef struct _VIVEXTDrawableFlush {
    CARD8    reqType;        /* always vivEXTReqCode */
    CARD8    vivEXTReqType;        /* always X_vivEXTDrawableFlush */
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTDrawableFlushReq;
#define sz_xVIVEXTDrawableFlushReq    12

typedef struct _VIVEXTDrawableInfo {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTDrawableInfoReq;
#define sz_xVIVEXTDrawableInfoReq    12

typedef struct {
    BYTE    type;/* X_Reply */
    BYTE    pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    INT16    drawableX B16;
    INT16    drawableY B16;
    INT16    drawableWidth B16;
    INT16    drawableHeight B16;
    CARD32    numClipRects B32;
    INT16       relX B16;
    INT16       relY B16;
    CARD32      alignedWidth B32;
    CARD32      alignedHeight B32;
    CARD32      stride B32;
    CARD32      nodeName B32;
    CARD32      phyAddress B32;
} xVIVEXTDrawableInfoReply;

#define sz_xVIVEXTDrawableInfoReply    44


typedef struct _VIVEXTFULLScreenInfo {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTFULLScreenInfoReq;
#define sz_xVIVEXTFULLScreenInfoReq    12

typedef struct {
    BYTE    type;            /* X_Reply */
    BYTE    pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    CARD32    fullscreenCovered B32;    /* if fullscreen is covered by windows, set to 1 otherwise 0 */
    CARD32    pad3 B32;
    CARD32    pad4 B32;
    CARD32    pad5 B32;
    CARD32    pad6 B32;
    CARD32    pad7 B32;        /* bytes 29-32 */
} xVIVEXTFULLScreenInfoReply;
#define    sz_xVIVEXTFULLScreenInfoReply 32


static XExtensionInfo _VIVEXT_info_data;
static XExtensionInfo *VIVEXT_info = &_VIVEXT_info_data;
static char *VIVEXT_extension_name = VIVEXTNAME;

#define VIVEXTCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, VIVEXT_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *                           private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display(Display *dpy, XExtCodes *extCodes);
static /* const */ XExtensionHooks VIVEXT_extension_hooks = {
    NULL,                                /* create_gc */
    NULL,                                /* copy_gc */
    NULL,                                /* flush_gc */
    NULL,                                /* free_gc */
    NULL,                                /* create_font */
    NULL,                                /* free_font */
    close_display,                        /* close_display */
    NULL,                                /* wire_to_event */
    NULL,                                /* event_to_wire */
    NULL,                                /* error */
    NULL,                                /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, VIVEXT_info,
                                   VIVEXT_extension_name,
                                   &VIVEXT_extension_hooks,
                                   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, VIVEXT_info)

static Bool VIVEXTDrawableFlush(Display *dpy, unsigned int screen, unsigned int drawable)
{
    XExtDisplayInfo *info = find_display (dpy);
    xVIVEXTDrawableFlushReq *req;

    VIVEXTCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VIVEXTDrawableFlush, req);
    req->reqType = info->codes->major_opcode;
    req->vivEXTReqType = X_VIVEXTDrawableFlush;
    req->screen = screen;
    req->drawable = drawable;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}
static Bool VIVEXTDrawableInfo(Display* dpy, int screen, Drawable drawable,
    int* X, int* Y, int* W, int* H,
    int* numClipRects, drm_clip_rect_t ** pClipRects,
    int* relX,
    int* relY,
    unsigned int * alignedWidth,
    unsigned int * alignedHeight,
    unsigned int * stride,
    unsigned int * nodeName,
    unsigned int * phyAddress
)
{

    XExtDisplayInfo *info = find_display (dpy);
    xVIVEXTDrawableInfoReply rep;
    xVIVEXTDrawableInfoReq *req;
    int extranums = 0;
#ifdef DRI_PIXMAPRENDER_ASYNC
    #define DRM_CLIPRECTS_CACHE_NUM 32
    static drm_clip_rect_t xcliprects[DRM_CLIPRECTS_CACHE_NUM];
#endif

    VIVEXTCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(VIVEXTDrawableInfo, req);
    req->reqType = info->codes->major_opcode;
    req->vivEXTReqType = X_VIVEXTDrawableInfo;
    req->screen = screen;
    req->drawable = drawable;

    extranums = ( sizeof(xVIVEXTDrawableInfoReply) - 32 ) / 4;

    if (!_XReply(dpy, (xReply *)&rep, extranums , xFalse))
    {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    *X = (int)rep.drawableX;
    *Y = (int)rep.drawableY;
    *W = (int)rep.drawableWidth;
    *H = (int)rep.drawableHeight;
    *numClipRects = rep.numClipRects;
    *alignedWidth = (unsigned int)rep.alignedWidth;
    *alignedHeight = (unsigned int)rep.alignedHeight;
    *stride = (unsigned int)rep.stride;
    *nodeName = (unsigned int)rep.nodeName;
    *phyAddress = (unsigned int)rep.phyAddress;

    *relX = rep.relX;
    *relY = rep.relY;

#ifdef DRI_PIXMAPRENDER_ASYNC

    *pClipRects = NULL;
     if (*numClipRects) {
        if (*numClipRects < DRM_CLIPRECTS_CACHE_NUM)
        {
           int len = sizeof(drm_clip_rect_t) * (*numClipRects);
           _XRead(dpy, (char*)xcliprects, len);
        } else {
           int len = sizeof(drm_clip_rect_t) * (*numClipRects);

           *pClipRects = (drm_clip_rect_t *)Xcalloc(len, 1);
           if (*pClipRects)
               _XRead(dpy, (char*)*pClipRects, len);
        }
     }

#else

    if (*numClipRects) {
       int len = sizeof(drm_clip_rect_t) * (*numClipRects);

       *pClipRects = (drm_clip_rect_t *)Xcalloc(len, 1);
       if (*pClipRects)
          _XRead(dpy, (char*)*pClipRects, len);
    } else {
        *pClipRects = NULL;
    }

#endif

    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}

static Bool VIVEXTFULLScreenInfo(Display* dpy, int screen, Drawable drawable)
{
    XExtDisplayInfo *info = find_display (dpy);
    xVIVEXTFULLScreenInfoReply rep;
    xVIVEXTFULLScreenInfoReq *req;
    int ret = 0;

    VIVEXTCheckExtension (dpy, info, False);


    LockDisplay(dpy);
    GetReq(VIVEXTFULLScreenInfo, req);
    req->reqType = info->codes->major_opcode;
    req->vivEXTReqType = X_VIVEXTFULLScreenInfo;
    req->screen = screen;
    req->drawable = drawable;

    if (!_XReply(dpy, (xReply *)&rep, 0 , xFalse))
    {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    ret = rep.fullscreenCovered;

    UnlockDisplay(dpy);
    SyncHandle();

    return (Bool)ret;
}


#endif

static gceSTATUS _CreateOnScreenSurfaceWrapper(
    __DRIdrawablePriv * drawable,
      gceSURF_FORMAT Format
)
{
    __DRIDisplay * display;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("drawable=0x%x", drawable);

    display = drawable->display;

    if ((display->width == drawable->w) && (display->height == drawable->h)) {
        drawable->fullScreenMode = gcvTRUE;
    }

    if ( display->format != gcvSURF_UNKNOWN )
        Format = display->format;

    if ((display->physicalAddr) && (drawable->fullScreenMode)) {
        do {
                gcmERR_BREAK(gcoSURF_ConstructWrapper(gcvNULL,
                    &display->renderTarget));

                gcmERR_BREAK(gcoSURF_SetBuffer(display->renderTarget,
                    gcvSURF_BITMAP,
                    Format,
                    display->width * display->bpp,
                    display->linearAddr,
                    display->physicalAddr
                    ));

                gcmERR_BREAK(gcoSURF_SetWindow(display->renderTarget,
                    0,
                    0,
                    display->width,
                    display->height));
        } while(gcvFALSE);
    }
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS _DestroyOnScreenSurfaceWrapper(
    __DRIDisplay * Display
)
{
    if (Display->renderTarget) {
        gcoSURF_Destroy(Display->renderTarget);
        Display->renderTarget = gcvNULL;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS _LockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node,
        OUT gctUINT64 *Address,
        OUT gctPOINTER *Memory)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmASSERT(Address != gcvNULL);
    gcmASSERT(Memory != gcvNULL);
    gcmASSERT(Node != 0);

    memset(&iface, 0, sizeof(gcsHAL_INTERFACE));

    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
    iface.u.LockVideoMemory.node = Node;
    iface.u.LockVideoMemory.cacheable = gcvFALSE;
    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));

    /* Get allocated node in video memory. */
    *Address = iface.u.LockVideoMemory.physicalAddress;
    *Memory = gcmUINT64_TO_PTR(iface.u.LockVideoMemory.memory);
OnError:
    return status;
}

static gceSTATUS _UnlockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node)
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmASSERT(Node != 0);

    memset(&iface, 0, sizeof(gcsHAL_INTERFACE));
    iface.ignoreTLS = gcvFALSE;
    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.u.UnlockVideoMemory.node = Node;
    iface.u.UnlockVideoMemory.type = gcvSURF_BITMAP;
    iface.u.UnlockVideoMemory.asynchroneous = gcvTRUE;

/*
    gcmONERROR(gcoHAL_Commit(Hal, gcvTRUE));
*/

    /* Call kernel API. */
    gcmONERROR(gcoOS_DeviceControl(
               gcvNULL,
               IOCTL_GCHAL_INTERFACE,
               &iface, gcmSIZEOF(iface),
               &iface, gcmSIZEOF(iface)));
    gcmONERROR(iface.status);

    gcmONERROR(iface.status);

    if (iface.u.UnlockVideoMemory.asynchroneous){
        iface.u.UnlockVideoMemory.asynchroneous = gcvFALSE;
        gcmONERROR(gcoHAL_ScheduleEvent(Hal,&iface));
    }

OnError:
    return status;
}

static gceSTATUS _FreeVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node) {
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmASSERT(Node != 0);

    iface.command = gcvHAL_RELEASE_VIDEO_MEMORY;
    iface.u.ReleaseVideoMemory.node = Node;

    status = gcoHAL_Call(Hal, &iface);

    gcoHAL_Commit(Hal, gcvFALSE);

    return status;
}

#if defined(DRI_PIXMAPRENDER) || defined(DRI_PIXMAPRENDER_ASYNC)
static void _createPixmapInfo(
        __DRIdrawablePriv *dridrawable,
        PlatformWindowType Drawable,
        Pixmap *backPixmap,
        GC *xgc,
        gcoSURF *pixWrapSurf,
        gctUINT32 *backNode,
        gceSURF_TYPE surftype,
        gceSURF_FORMAT surfformat
        )
{
    int x,y,w,h;
    int xx, yy;
    unsigned int ww, hh, bb;
    int xw,yw;
    int numRects = 0;
    drm_clip_rect_t *pClipRects;
    unsigned int alignedW;
    unsigned int alignedH;
    unsigned int stride;
    unsigned int nodeName;
    unsigned int physAddr;
    unsigned int depth = 24;
    Window    root;
    gctPOINTER  destLogicalAddr[3] = {0, 0, 0};
    gctUINT64   destPhys[3] = {0,0,0};
    gceSTATUS status = gcvSTATUS_OK;

    __DRIDisplay * display;
    __DRIdrawablePriv *pdp=dridrawable;


    display = dridrawable->display;
    XGetGeometry(display->dpy, Drawable, &root, &xx, &yy, &ww, &hh, &bb, &depth);

    *backPixmap = XCreatePixmap(display->dpy, Drawable, dridrawable->w, dridrawable->h, depth);
    *xgc = XCreateGC(display->dpy, Drawable, 0, NULL);
    XFlush(display->dpy);

    VIVEXTDrawableInfo(display->dpy, pdp->screen, *backPixmap,
                    &x, &y, &w, &h,
                    &numRects, &pClipRects,
                    &xw,
                    &yw,
                    (unsigned int *)&alignedW,
                    (unsigned int *)&alignedH,
                    &stride,
                    &nodeName,
                    (unsigned int *)&physAddr);

    if ( numRects >0 && pClipRects )
        Xfree((void *)pClipRects);

    if ( nodeName )
        gcoHAL_ImportVideoMemory((gctUINT32)nodeName, (gctUINT32 *)backNode);
    else
        *pixWrapSurf = gcvNULL;

    gcmONERROR(_LockVideoNode(0, *backNode, &destPhys[0], &destLogicalAddr[0]));

    do {
        /* Construct a wrapper around the pixmap.  */
        gcmERR_BREAK(gcoSURF_ConstructWrapper(
        gcvNULL,
        pixWrapSurf
        ));

        /* Set the underlying frame buffer surface. */
        gcmERR_BREAK(gcoSURF_SetBuffer(
        *pixWrapSurf,
        surftype,
        surfformat,
        stride,
        destLogicalAddr[0],
        (gctUINT32)destPhys[0]
        ));

        /* Set the window. */
        gcmERR_BREAK(gcoSURF_SetWindow(
        *pixWrapSurf,
        0,
        0,
        dridrawable->w,
        dridrawable->h
        ));
        return ;
    } while(0);


    gcmONERROR(_UnlockVideoNode(0, *backNode));
    gcmONERROR(_FreeVideoNode(0, *backNode));
    *backNode = 0;

OnError:

    if ( *backNode )
     _FreeVideoNode(0, *backNode);


    if ( *pixWrapSurf )
    gcoSURF_Destroy(*pixWrapSurf);

    if ( (*backPixmap) )
        XFreePixmap(display->dpy, *backPixmap);
    if ( (*xgc) )
        XFreeGC(display->dpy, *xgc);

    *backPixmap = (Pixmap)0;
    *xgc =(GC)0;
    *pixWrapSurf = (gcoSURF)0;
#if (defined(DEBUG) || defined(_DEBUG))
    fprintf(stderr, "Warning::Backpixmap can't be created for the current Drawable\n");
#endif
}

static void _destroyPixmapInfo(
    __DRIDisplay * display,
    Pixmap backPixmap,
    GC xgc,
    gcoSURF pixWrapSurf,
    gctUINT32 backNode)
{

    if ( backNode )
    _UnlockVideoNode(0, backNode);

    if ( backNode )
    _FreeVideoNode(0, backNode);

    if ( (gctUINT32)backPixmap )
        XFreePixmap(display->dpy, backPixmap);


    if ( (gctUINT32)xgc )
        XFreeGC(display->dpy, xgc);

    if ( pixWrapSurf )
        gcoSURF_Destroy(pixWrapSurf);

}
#endif

#ifdef DRI_PIXMAPRENDER_ASYNC
static void renderThread(void* refData)
{
    __DRIdrawablePriv * drawable = (__DRIdrawablePriv *)refData;

    int i = 0;
    Display                        *dpy = gcvNULL;
    #define T_OPEN 10
    int opentimes = T_OPEN;

    if ( drawable == gcvNULL )
        return ;

    XInitThreads();

    while( 1 && drawable->frameMutex ) {


        if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, drawable->busySIG, gcvINFINITE)))
        {
            break;
        }

        if (gcmIS_SUCCESS(gcoOS_WaitSignal(gcvNULL, drawable->stopSIG, 0)))
        {
            gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex,0);
            gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
            break;
        }

        gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);

        if ( drawable->busyNum > 0 )
        {
            for ( i = 0; i < NUM_ASYNCFRAME; i++)
            {
                if ( drawable->ascframe[i].busy )
                {
                    gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
                    if ( gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, drawable->ascframe[i].signal, gcvINFINITE)) )
                    {
                        gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);
                        continue;
                    }

/* We don't need lock any more cause backpixmap is used and 3D will not draw the window directly*/
/*
                    LINUX_LOCK_FRAMEBUFFER(context);
*/
                    if ( drawable->ascframe[i].backPixmap )
                    {
                        if ( opentimes == T_OPEN )
                        dpy = XOpenDisplay(NULL);

                        XCopyArea(dpy, drawable->ascframe[i].backPixmap, drawable->ascframe[i].Drawable, drawable->ascframe[i].xgc, 0, 0, drawable->ascframe[i].w, drawable->ascframe[i].h, 0, 0);
                        XFlush(dpy);
                        opentimes--;

                        if ( opentimes == 0 )
                        {
                           XCloseDisplay(dpy);
                           dpy = gcvNULL;
                           opentimes = T_OPEN;
                        }


                    }
/*
                    LINUX_UNLOCK_FRAMEBUFFER(context);
*/

                    gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);
                    drawable->ascframe[i].busy = _FRAME_FREE;
                    drawable->busyNum--;
                    if (drawable->busyNum<0) drawable->busyNum = 0;
                }
            }

        }
        gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);

    }

    /*gcoHAL_Commit(gcvNULL, gcvTRUE);*/

    if ( drawable->exitSIG)
    gcoOS_Signal(gcvNULL, drawable->exitSIG, gcvTRUE);

    if ( dpy != 0 )
        XCloseDisplay(dpy);

}


static void asyncRenderKill(__DRIdrawablePriv * drawable)
{
    if ( drawable == gcvNULL || drawable->busySIG == gcvNULL || drawable->stopSIG == gcvNULL || drawable->exitSIG == gcvNULL || drawable->frameMutex == gcvNULL )
    return ;

    XSync(drawable->display->dpy, False);

    while(1) {
        gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);
        if (drawable->busyNum == 0)
        {
            drawable->curIndex = -1;
            gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
            break;
        }
        gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
    }

    gcoOS_Signal(gcvNULL, drawable->busySIG, gcvTRUE);
    gcoOS_Signal(gcvNULL, drawable->stopSIG, gcvTRUE);

    while(gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, drawable->exitSIG, 0)))
    {
        gcoOS_Signal(gcvNULL, drawable->busySIG, gcvTRUE);
        gcoOS_Signal(gcvNULL, drawable->stopSIG, gcvTRUE);
    }

    gcoOS_DestroySignal(gcvNULL, drawable->stopSIG);
    gcoOS_DestroySignal(gcvNULL, drawable->exitSIG);
    gcoOS_DestroySignal(gcvNULL, drawable->busySIG);

    gcoOS_CloseThread(gcvNULL, drawable->workerThread);

    drawable->stopSIG = gcvNULL;
    drawable->busySIG = gcvNULL;
    drawable->exitSIG = gcvNULL;
}

static void asyncRenderDestroy(__DRIdrawablePriv * drawable)
{

    __DRIDisplay * display;
    int i = 0;

    if ( drawable == gcvNULL ) return;

    display = drawable->display;

    /*gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);*/
    for ( i = 0; i < NUM_ASYNCFRAME; i++)
    {
        if ( drawable->ascframe[i].backPixmap && drawable->ascframe[i].pixWrapSurf )
            _destroyPixmapInfo(display, drawable->ascframe[i].backPixmap, drawable->ascframe[i].xgc, drawable->ascframe[i].pixWrapSurf, drawable->ascframe[i].backNode);

        if ( drawable->ascframe[i].signal )
            gcoOS_DestroySignal(gcvNULL, drawable->ascframe[i].signal);

        drawable->ascframe[i].signal = gcvNULL;
        drawable->ascframe[i].w = 0;
        drawable->ascframe[i].h = 0;

        drawable->ascframe[i].backPixmap = 0;
        drawable->ascframe[i].pixWrapSurf = gcvNULL;
    }
    /*gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);*/
    if ( drawable->frameMutex )
    {
        gcoOS_DeleteMutex(gcvNULL, drawable->frameMutex);
        drawable->frameMutex = gcvNULL;
    }
}

static void asyncRenderSurf(__DRIdrawablePriv * drawable, int index, gcoSURF *renderSurf)
{

    if ( index < 0 || index >= NUM_ASYNCFRAME || drawable->frameMutex == gcvNULL )
        *renderSurf = gcvNULL;
    else {

        if (drawable->stopSIG == gcvNULL)
        *renderSurf = gcvNULL;
        else
        *renderSurf = drawable->ascframe[index].pixWrapSurf;
    }
}

static int pickFrameIndex(__DRIdrawablePriv * drawable, int *index)
{
    int crtframe = _NOT_SET;
    int i = 0;

    gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);


    if ( drawable->w != drawable->ascframe[0].w || drawable->h !=drawable->ascframe[0].h)
    {
        gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
        /* stop thread */
        /* destroy old frames */
        /* set stat to recreate frames */
        asyncRenderKill(drawable);
        asyncRenderDestroy(drawable);

        if ( drawable->frameMutex == gcvNULL )
            gcoOS_CreateMutex(gcvNULL, &drawable->frameMutex);

        crtframe = _TO_UPDATEFRAME;

    } else {

        for(i = ( drawable->curIndex + 1 )%NUM_ASYNCFRAME ; i < NUM_ASYNCFRAME; i++)
        {
            if ( drawable->ascframe[i].busy == _FRAME_FREE ) {
                *index = i;
                drawable->curIndex = i;
                break;
            }
        }

       if ( i >= NUM_ASYNCFRAME )
       {
            /* release mutex to give the renderthread chance to reset busynum */
            gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
            while(1) {
                gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);
                    if (drawable->busyNum == 0)
                    {
                        *index = 0;
                        drawable->curIndex = 0;
                        break;
                    }
                gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);
                }
       }

        gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);

    }
    return crtframe;
}

static void asyncRenderBegin(__DRIdrawablePriv * drawable, PlatformWindowType Drawable, int *index)
{
    gceSTATUS status = gcvSTATUS_OK;
    int crtframe = _NOT_SET;
    int i = 0;

    if ( drawable->frameMutex == gcvNULL ){
        gcoOS_CreateMutex(
            gcvNULL,
            &drawable->frameMutex
            );

        if ( drawable->frameMutex == gcvNULL ){
            *index = -1;
            return ;
        }
        crtframe = _TO_INITFRAME;
    }

    if ( crtframe == _NOT_SET) {

        crtframe = pickFrameIndex(drawable, index);

    }

    if ( crtframe == _TO_INITFRAME || crtframe == _TO_UPDATEFRAME ) {
        drawable->busyNum = 0;
        gcoOS_CreateSignal(gcvNULL, gcvFALSE, &drawable->busySIG);
        gcoOS_CreateSignal(gcvNULL, gcvFALSE, &drawable->stopSIG);
        gcoOS_CreateSignal(gcvNULL, gcvFALSE, &drawable->exitSIG);

        gcoOS_CreateThread(gcvNULL, (gcTHREAD_ROUTINE)renderThread, drawable, &drawable->workerThread);


        for ( i = 0; i < NUM_ASYNCFRAME; i++)
        {
            drawable->ascframe[i].w = drawable->w;
            drawable->ascframe[i].h = drawable->h;
            gcmERR_BREAK(gcoOS_CreateSignal(
            gcvNULL,
            gcvFALSE,
            &drawable->ascframe[i].signal
            ));

            drawable->ascframe[i].dridrawable = drawable;
            drawable->ascframe[i].surftype = drawable->surftype;
            drawable->ascframe[i].surfformat = drawable->surfformat;
            drawable->ascframe[i].pixWrapSurf = gcvNULL;
            drawable->ascframe[i].Drawable = Drawable;
            drawable->ascframe[i].backNode = 0;
            _createPixmapInfo(drawable, Drawable, &(drawable->ascframe[i].backPixmap), &(drawable->ascframe[i].xgc), &(drawable->ascframe[i].pixWrapSurf), &drawable->ascframe[i].backNode, drawable->surftype, drawable->surfformat);
        }

       if ( i < NUM_ASYNCFRAME )
        *index = -1;
       else {
        *index = 0;
        drawable->curIndex = 0;
       }
    }

    if ( drawable->frameMutex )
    gcoOS_AcquireMutex(gcvNULL, drawable->frameMutex, gcvINFINITE);
}


static void asyncRenderEnd(__DRIdrawablePriv * drawable, PlatformWindowType Drawable, int index)
{
    gcsHAL_INTERFACE iface;


    if (drawable == gcvNULL || drawable->frameMutex == gcvNULL)
        return;

    if (index < -1)
        goto ENDUNLOCK;

    if (drawable->ascframe[index].signal == gcvNULL
        || drawable->ascframe[index].backPixmap == 0
        || drawable->stopSIG == gcvNULL
        || drawable->exitSIG == gcvNULL
        || drawable->busySIG == gcvNULL )
        goto ENDUNLOCK;


    drawable->ascframe[index].busy = _FRAME_BUSY;
    drawable->ascframe[index].Drawable = Drawable;
    drawable->busyNum ++;

    iface.command            = gcvHAL_SIGNAL;
    iface.engine             = gcvENGINE_RENDER;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(drawable->ascframe[index].signal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
    iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

    /* Schedule the event. */
    gcoHAL_ScheduleEvent(gcvNULL, &iface);

    if ( drawable->busySIG)
    gcoOS_Signal(gcvNULL,drawable->busySIG, gcvTRUE);


ENDUNLOCK:

    gcoOS_ReleaseMutex(gcvNULL, drawable->frameMutex);

#if gcdENABLE_VG
/* drawable->rsFunc != NULL is VG path */
    if ( drawable->rsFunc )
    {
        gcoHAL_Commit(gcvNULL, gcvTRUE);
        if ( drawable->ascframe[index].busy ==  _FRAME_BUSY )
            gcoOS_Signal(gcvNULL, drawable->ascframe[index].signal, gcvTRUE);
    } else {
#endif
        gcoHAL_Commit(gcvNULL, gcvFALSE);
#if gcdENABLE_VG
    }
#endif
}

#endif


#ifdef DRI_PIXMAPRENDER
static void _CreateBackPixmapForDrawable(__DRIdrawablePriv * drawable)
{
    _createPixmapInfo(drawable, drawable->drawable, &drawable->backPixmap, &drawable->xgc, &drawable->pixWrapSurf, &drawable->backPixmapNode, drawable->surftype, drawable->surfformat);
}

static void _DestroyBackPixmapForDrawable(__DRIdrawablePriv * drawable)
{
    __DRIDisplay * display;
    display = drawable->display;

    if ( drawable->pixWrapSurf )
        _destroyPixmapInfo(display, drawable->backPixmap, drawable->xgc, drawable->pixWrapSurf, drawable->backPixmapNode);

    drawable->backPixmapNode = 0;
    drawable->backPixmap = (Pixmap)0;
    drawable->pixWrapSurf = gcvNULL;
    drawable->xgc = (GC) 0;

}
#endif

typedef struct _M_Pixmap
{
    Display *dpy;
    Drawable pixmap;
    gctINT mapped;
    gctPOINTER destLogicalAddr;
    gctPOINTER phyAddr;
    gctINT stride;
    gctPOINTER next;
}MPIXMAP,*PMPIXMAP;

static PMPIXMAP _vpixmaphead = gcvNULL;
static MPIXMAP _cachepixmap = {gcvNULL, (Drawable)0, 0, gcvNULL, gcvNULL, 0, gcvNULL};

static gctBOOL _pixmapMapped(Drawable pixmap, gctPOINTER *destLogicalAddr, gctPOINTER *phyAddr, Display **dpy, gctINT *stride)
{
    PMPIXMAP pixmapnode = gcvNULL;
    if ( _vpixmaphead == gcvNULL )
        return gcvFALSE;

    if ( _cachepixmap.pixmap == pixmap)
    {
        *destLogicalAddr = _cachepixmap.destLogicalAddr;
        *phyAddr = _cachepixmap.phyAddr;
        *dpy = _cachepixmap.dpy;
        *stride = _cachepixmap.stride;
        return gcvTRUE;
    }

    pixmapnode = _vpixmaphead;
    while( pixmapnode )
    {
        if ( pixmapnode->pixmap == pixmap )
        {
            if ( pixmapnode->mapped)
            {
                *destLogicalAddr = pixmapnode->destLogicalAddr;
                *phyAddr = pixmapnode->phyAddr;
                *dpy = pixmapnode->dpy;
                *stride = pixmapnode->stride;
                return gcvTRUE;
            }
            else
                return gcvFALSE;
        }
        pixmapnode = (PMPIXMAP)pixmapnode->next;
    }

    return gcvFALSE;

}

static gctBOOL _setPixmapMapped(Drawable pixmap, gctPOINTER destLogicalAddr, gctPOINTER phyAddr, Display *dpy, gctINT stride)
{
    PMPIXMAP pixmapnode = gcvNULL;
    PMPIXMAP prepixmapnode = gcvNULL;

    _cachepixmap.destLogicalAddr = destLogicalAddr;
    _cachepixmap.phyAddr = phyAddr;
    _cachepixmap.dpy = dpy;
    _cachepixmap.pixmap = pixmap;
    _cachepixmap.mapped = 1;
    _cachepixmap.stride = stride;

    if ( _vpixmaphead == gcvNULL )
    {
        _vpixmaphead = (PMPIXMAP)malloc(sizeof(MPIXMAP));
        _vpixmaphead->mapped = 1;
        _vpixmaphead->pixmap = pixmap;
        _vpixmaphead->dpy= dpy;
        _vpixmaphead->destLogicalAddr = destLogicalAddr;
        _vpixmaphead->phyAddr = phyAddr;
        _vpixmaphead->stride = stride;
        _vpixmaphead->next = gcvNULL;
        return gcvTRUE;
    }

    pixmapnode = _vpixmaphead;
    while( pixmapnode )
    {
        prepixmapnode = pixmapnode;
        if ( pixmapnode->pixmap == pixmap )
        {
            pixmapnode->mapped = 1;
            pixmapnode->destLogicalAddr = destLogicalAddr;
            pixmapnode->phyAddr = phyAddr;
            pixmapnode->stride = stride;
            return gcvTRUE;
        }
        pixmapnode = (PMPIXMAP)pixmapnode->next;
    }
    pixmapnode = (PMPIXMAP)malloc(sizeof(MPIXMAP));
    pixmapnode->mapped = 1;
    pixmapnode->pixmap = pixmap;
    pixmapnode->dpy= dpy;
    pixmapnode->destLogicalAddr = destLogicalAddr;
    pixmapnode->phyAddr = phyAddr;
    pixmapnode->stride = stride;
    pixmapnode->next = gcvNULL;
    prepixmapnode->next = pixmapnode;
    return gcvTRUE;
}

static gctBOOL _unSetPixmapMapped(Drawable pixmap)
{
    PMPIXMAP pixmapnode = gcvNULL;
    PMPIXMAP prepixmapnode = gcvNULL;

    _cachepixmap.pixmap = (Drawable)0;
    _cachepixmap.mapped = 0;

    if ( _vpixmaphead == gcvNULL )
        return gcvFALSE;

    pixmapnode = _vpixmaphead;
    while( pixmapnode )
    {
        if ( pixmapnode->pixmap == pixmap )
        {
            pixmapnode->mapped = 0;
            if ( pixmapnode == _vpixmaphead)
            _vpixmaphead = pixmapnode->next;
            else
            prepixmapnode->next = pixmapnode->next;
            free(pixmapnode);
            return gcvTRUE;
        }
        prepixmapnode = pixmapnode;
        pixmapnode = (PMPIXMAP)pixmapnode->next;
    }

    return gcvTRUE;
}

gceSTATUS _getPixmapDrawableInfo(Display *dpy, Drawable pixmap, gctUINT* videoNode, gctINT* s)
{
    int discarded;
    unsigned int uDiscarded;
    unsigned int nodeName;
    int numClipRects = 0;
    drm_clip_rect_t* pClipRects;
    int stride = 0;
    int x;
    int y;
    int w;
    int h;
    int alignedw;
    int alignedh;


    if ( !VIVEXTDrawableInfo(dpy, DefaultScreen(dpy), pixmap,
                    &x, &y, &w, &h,
                    &numClipRects, &pClipRects,
                    &discarded,
                    &discarded,
                    (unsigned int *)&alignedw,
                    (unsigned int *)&alignedh,
                    (unsigned int *)&stride,
                    (unsigned int *)&nodeName,
                    (unsigned int *)&uDiscarded) )
    {
        *videoNode = 0;
        *s = 0;
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ( numClipRects >0 && pClipRects )
        Xfree((void *)pClipRects);

    /* Extract the data needed for pixmaps from the variables set in dri.c */
    if ( nodeName )
    gcoHAL_ImportVideoMemory(nodeName, videoNode);
    else
    *videoNode = 0;

    *s = stride;
    return gcvSTATUS_OK;
}

static void _UpdateDrawableInfoDrawableInfo(__DRIdrawablePriv * drawable)
{

    __DRIdrawablePriv *pdp=drawable;
    __DRIDisplay * display;

    int x;
    int y;
    int w;
    int h;
    int numClipRects = 0;
    drm_clip_rect_t *pClipRects;
    unsigned int stride;
    unsigned int nodeName = 0;

    display = drawable->display;

    if ( pdp->numBackClipRects >0 && pdp->pBackClipRects )
    Xfree((void *)pdp->pBackClipRects);

    pdp->numBackClipRects = 0;

    /* Assign __DRIdrawablePrivate first */
    if (!XF86DRIGetDrawableInfo(display->dpy, pdp->screen, pdp->drawable,
                    &pdp->index, &pdp->lastStamp,
                    &x, &y, &w, &h,
                    &numClipRects, &pClipRects,
                    &pdp->backX,
                    &pdp->backY,
                    &pdp->numBackClipRects,
                    &pdp->pBackClipRects
                                    )) {
        /* Error -- eg the window may have been destroyed.  Keep going
         * with no cliprects.
         */
        pdp->pStamp = &pdp->lastStamp; /* prevent endless loop */
        pdp->numClipRects = 0;
        pdp->pClipRects = NULL;
        pdp->numBackClipRects = 0;
        pdp->pBackClipRects = NULL;
        goto ENDFLAG;
    }
    else
       pdp->pStamp = &(display->pSAREA->drawableTable[pdp->index].stamp);

    if ( numClipRects >0 && pClipRects )
        Xfree((void *)pClipRects);

    if (!drawable->vgapi) {
        if (pdp->oldx == (gctINT)x && pdp->oldy == (gctINT)y
              && pdp->oldw == (gctUINT)w && pdp->oldh == (gctUINT)h
              && pdp->olddrawable == pdp->drawable )
        goto ENDFLAG;

        pdp->oldx = (gctINT)x;
        pdp->oldy = (gctINT)y;
        pdp->oldw = (gctUINT)w;
        pdp->oldh = (gctUINT)h;
        pdp->olddrawable = pdp->drawable;
    }

    if (pdp->numClipRects >0 && pdp->pClipRects)
        Xfree((void *)pdp->pClipRects);
    pdp->numClipRects = 0;

    VIVEXTDrawableInfo(display->dpy, pdp->screen, pdp->drawable,
                    &pdp->x, &pdp->y, &pdp->w, &pdp->h,
                    &pdp->numClipRects, &pdp->pClipRects,
                    &pdp->xWOrigin,
                    &pdp->yWOrigin,
                    (unsigned int *)&pdp->wWidth,
                    (unsigned int *)&pdp->wHeight,
                    &stride,
                    &nodeName,
                    (unsigned int *)&pdp->backBufferPhysAddr);

    if (nodeName)
    {

        if (pdp->backNode)
            _FreeVideoNode(0, pdp->backNode);

        pdp->backNode = 0;
        pdp->nodeName = nodeName;

        gcoHAL_ImportVideoMemory((gctUINT32)nodeName, (gctUINT32 *)&pdp->backNode);
    }
    else
    pdp->backNode = 0;

ENDFLAG:
/* make compiler happy */
;
}

static void _FullScreenCovered(__DRIdrawablePriv * drawable)
{
    Bool ret;
    __DRIDisplay * display;

    display = drawable->display;
    drawable->fullscreenCovered = 0;
    ret = VIVEXTFULLScreenInfo(display->dpy, drawable->screen, drawable->drawable);
    drawable->fullscreenCovered = (int)ret;
}

/* Lock the hardware and validate drawable state.
 * LOCK_FRAMEBUFFER( gc ) and UNLOCK_FRAMEBUFFER( gc ) should be called whenever
 * OpenGL driver needs to access framebuffer hardware.
 */
static void _VivGetLock(__DRIdrawablePriv * drawable)
{
    if (drawable)
    {
        __DRIDisplay *display = drawable->display;
        gctINT width = drawable->w;
        gctINT height = drawable->h;

        DRI_VALIDATE_DRAWABLE_INFO(drawable);
        DRI_FULLSCREENCOVERED(drawable);

        if ((drawable->w != width) || (drawable->h != height))
        {

#ifdef DRI_PIXMAPRENDER
            _DestroyBackPixmapForDrawable(drawable);
            _CreateBackPixmapForDrawable(drawable);
#endif

            drawable->drawableResize = gcvTRUE;
            drawable->fullScreenMode = (display->width == width && display->height == height)
                                     ? gcvTRUE : gcvFALSE;
        }
    }
}

static void
_GetDisplayInfo(
    __DRIDisplay* display,
    int scrn)
{
    int   fd = -1;
    drm_handle_t hSAREA;
    drm_handle_t hFB;
    char *BusID;
/*
    const char * err_msg;
    const char * err_extra;
*/
    void * pSAREA = NULL;
    Display * dpy = display->dpy;
    int   status;
    void * priv;
    int priv_size;
    int junk;

    if (XF86DRIOpenConnection(dpy, scrn, &hSAREA, &BusID))
    {

        fd = drmOpen(NULL,BusID);

        XFree(BusID); /* No longer needed */
/*
        err_msg = "open DRM";
        err_extra = strerror( -fd );
*/
        if (fd >= 0)
        {

            drm_magic_t magic;
            display->drmFd = fd;
/*
            err_msg = "drmGetMagic";
            err_extra = NULL;
*/
            if (!drmGetMagic(fd, &magic))
            {
                /*err_msg = "XF86DRIAuthConnection";*/
                if (XF86DRIAuthConnection(dpy, scrn, magic))
                {
                    /*err_msg = "XF86DRIGetDeviceInfo";*/
                    if (XF86DRIGetDeviceInfo(dpy, scrn,
                         &hFB,
                         &junk,
                         &display->fbSize,
                         &display->stride,
                         &priv_size,
                         &priv))
                    {
                        /* Set on screen frame buffer physical address */
                        display->physicalAddr = hFB;
                        display->width = DisplayWidth(dpy, scrn);
                        display->height = DisplayHeight(dpy, scrn);
                        display->bpp = DefaultDepth(dpy, scrn);

                        if ( display->bpp == 24 ) /* not support 24-bpp */
                            display->bpp = 32;

                        switch( display->bpp )
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

                        status = drmMap(fd, hFB, display->fbSize, &display->linearAddr);
                        if (status == 0)
                        {
                         /*
                            * Map the SAREA region.  Further mmap regions may be setup in
                            * each DRI driver's "createScreen" function.
                         */
                            status = drmMap(fd, hSAREA, SAREA_MAX, &pSAREA);
                            display->pSAREA = pSAREA;
                        }
                    }
                }
            }
        }
    }
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
dri_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_SetDisplayVirtual(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return dri_SetDisplayVirtual(Display, Window, Offset, X, Y);
}

static gceSTATUS
dri_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
dri_SetSwapInterval(
    IN PlatformWindowType Window,
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

    if (!XF86DRICreateContextWithConfig(display->dpy, display->screen, 0/*modes->fbconfigID*/,
                                        &context->contextID, &context->hHWContext)) {
        free(context);
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    context->eglContext = Context;
    context->hwLock = (drm_hw_lock_t *)&display->pSAREA->lock;
    context->next = display->contextStack;
    context->fd = display->drmFd;
    display->contextStack = context;

#ifdef DRI_PIXMAPRENDER
    if ( context->drawablePriv )
    {
            _DestroyBackPixmapForDrawable(context->drawablePriv);
            _CreateBackPixmapForDrawable(context->drawablePriv);
    }
#endif


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
        XF86DRIDestroyContext(display->dpy, display->screen, cur->contextID);
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
    VEGLThreadData  thread;

    gcmHEADER_ARG("localDisplay=0x%x, Drawable = 0x%x, Readable = 0x%x Context=0x%x\n", localDisplay, DrawDrawable, ReadDrawable, Context);

    if (localDisplay == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    thread = veglGetThreadData();

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

    drawable->vgapi = (thread->api == EGL_OPENVG_API);

    if (!drawable->pStamp) {

       _UpdateDrawableInfoDrawableInfo(drawable);

       drawable->pStamp = &(display->pSAREA->drawableTable[drawable->index].stamp);
    } else {

            _UpdateDrawableInfoDrawableInfo(drawable);

    }

    gcoSURF_GetFormat(ResolveTarget, gcvNULL,&format );

#ifdef DRI_PIXMAPRENDER
    if ( drawable )
    {
        _DestroyBackPixmapForDrawable(drawable);
        gcoSURF_GetFormat(ResolveTarget, &drawable->surftype, &drawable->surfformat);
        _CreateBackPixmapForDrawable(drawable);
    }
#endif

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

    __DRIDisplay* display;
    gceSTATUS status = gcvSTATUS_OK;

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
    if (!XF86DRICreateDrawable(display->dpy, display->screen, Drawable, &drawable->hHWDrawable)) {
        free(drawable);
        gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

    drawable->drawable = Drawable;
    drawable->refcount = 0;
    drawable->pStamp = NULL;
    drawable->lastStamp = 0;
    drawable->index = 0;
    drawable->x = 0;
    drawable->y = 0;
    drawable->w = 0;
    drawable->h = 0;
    drawable->numClipRects = 0;
    drawable->numBackClipRects = 0;
    drawable->pClipRects = NULL;
    drawable->pBackClipRects = NULL;
    drawable->display = display;
    drawable->drawableResize = gcvFALSE;
    drawable->screen = display->screen;
    drawable->drawLockID = 1;
    drawable->fd = display->drmFd;
    drawable->backNode = 0;
    drawable->nodeName = 0;

    drawable->next = display->drawableStack;
    display->drawableStack = drawable;
#ifdef DRI_PIXMAPRENDER
    drawable->backPixmap = (Pixmap)0;
    drawable->xgc = (GC)0;
    drawable->pixWrapSurf = gcvNULL;
#endif

#ifdef DRI_PIXMAPRENDER_ASYNC
    drawable->frameMutex = gcvNULL;
    drawable->workerThread = gcvNULL;
    drawable->busyNum = 0;
    drawable->curIndex = -1;
    drawable->busySIG = gcvNULL;
    drawable->stopSIG = gcvNULL;
    drawable->exitSIG = gcvNULL;
#endif

    drawable->oldx = -1;
    drawable->oldy = -1;
    drawable->oldw = 0;
    drawable->oldh = 0;
    drawable->olddrawable = (PlatformWindowType)0;

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

        if ( cur->backNode && cur->nodeName )
            _FreeVideoNode(0, cur->backNode);

        cur->backNode = 0;
        cur->nodeName = 0;

        if ( cur->numClipRects >0 && cur->pClipRects )
            Xfree((void *)cur->pClipRects);

        cur->numClipRects = 0;

        if ( cur->numBackClipRects >0 && cur->pBackClipRects )
            Xfree((void *)cur->pBackClipRects);

        cur->numBackClipRects = 0;

#ifdef DRI_PIXMAPRENDER
        _DestroyBackPixmapForDrawable(cur);
#endif
#ifdef DRI_PIXMAPRENDER_ASYNC
        asyncRenderKill(cur);
        asyncRenderDestroy(cur);
#endif
        XF86DRIDestroyDrawable(display->dpy, display->screen, Drawable);
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

#ifdef DRI_PIXMAPRENDER
static gceSTATUS
dri_SwapBuffersXwinEx(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    IN gctBOOL yInverted,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);

    if (localDisplay == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;
    drawable = _FindDrawable(driDisplay, Drawable);

    if ( drawable )
    {
        context = drawable->contextPriv;
        gcoSURF_GetFormat(ResolveTarget, &drawable->surftype, &drawable->surfformat);
        LINUX_LOCK_FRAMEBUFFER(context);
    } else {
        goto ENDXWINEX;
    }

    ResolveTarget = drawable->pixWrapSurf;

    if (drawable) {
        if ( drawable->rsFunc == (RSFUNC)gcvNULL )
        {
            gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {ResolveTarget, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.rectSize.x = *Width;
            rlvArgs.uArgs.v2.rectSize.y = *Height;
            rlvArgs.uArgs.v2.numSlices  = 1;
            rlvArgs.uArgs.v2.yInverted = yInverted;

            status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
        } else {
            gcsPOINT srcOrigin, size;
            srcOrigin.x = 0;
            srcOrigin.y = 0;
            size.x = *Width;
            size.y = *Height;
            status = (drawable->rsFunc)(RenderTarget, ResolveTarget, &srcOrigin, &srcOrigin, &size);
        }
        gcoHAL_Commit(gcvNULL, gcvFALSE);
    }

    *Width = drawable->w;
    *Height = drawable->h;

#if defined(VIV_EXT)
        if ( drawable->backPixmap )
        VIVEXTDrawableFlush(driDisplay->dpy, (unsigned int)(driDisplay->screen),(unsigned int)drawable->backPixmap);
#endif
    XCopyArea(driDisplay->dpy, drawable->backPixmap, Drawable, drawable->xgc, 0, 0, *Width, *Height, 0, 0);
    XFlush(driDisplay->dpy);

ENDXWINEX:
    if ( drawable )
    {
        context = drawable->contextPriv;
        LINUX_UNLOCK_FRAMEBUFFER(context);
    }
    gcmFOOTER();
    return status;

}
#endif

#ifdef DRI_PIXMAPRENDER_ASYNC
static gceSTATUS
dri_SwapBuffersXwinAsync(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    IN gctBOOL yInverted,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gceSTATUS status = gcvSTATUS_OK;

    int index = -1;

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);
    if (localDisplay == gcvNULL )
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;
    drawable = _FindDrawable(driDisplay, Drawable);

    if ( drawable )
    {
        gcoSURF_GetFormat(ResolveTarget, &drawable->surftype, &drawable->surfformat);
        /* To check Win by DRI */
        /*LINUX_LOCK_FRAMEBUFFER(context);*/
        /*LINUX_UNLOCK_FRAMEBUFFER(context);*/
    } else {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }


    asyncRenderBegin(drawable, Drawable, &index);

    asyncRenderSurf(drawable, index, &ResolveTarget);

    if (drawable && ResolveTarget) {

        if ( drawable->rsFunc == (RSFUNC)gcvNULL )
        {
            gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {ResolveTarget, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.rectSize.x = *Width;
            rlvArgs.uArgs.v2.rectSize.y = *Height;
            rlvArgs.uArgs.v2.numSlices  = 1;
            rlvArgs.uArgs.v2.yInverted = yInverted;

            status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
        } else {
            gcsPOINT srcOrigin, size;
            srcOrigin.x = 0;
            srcOrigin.y = 0;
            size.x = *Width;
            size.y = *Height;
            status = (drawable->rsFunc)(RenderTarget, ResolveTarget, &srcOrigin, &srcOrigin, &size);
        }

        if (gcmIS_ERROR(status))
        {
            index = -1;
            goto ENDXWINEX;
        }

    }

ENDXWINEX:

    *Width = drawable->w;
    *Height = drawable->h;

    asyncRenderEnd(drawable, Drawable, index);

    gcmFOOTER();
    return status;
}
#endif

#if !defined(DRI_PIXMAPRENDER) && !defined(DRI_PIXMAPRENDER_ASYNC)

static gceSTATUS
dri_SwapBuffersXwin(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    IN gctBOOL yInverted,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    gctUINT8_PTR  destStart;
    gctUINT8_PTR  srcStart;
    gctUINT i,j;
    gctUINT alignedWidth, alignedHeight;
    gctUINT alignedDestWidth;
    drm_clip_rect_t *pClipRects;
    gctUINT xoffset, yoffset;
    gctUINT height, width;
    __DRIDisplay* driDisplay;
    gceSTATUS status = gcvSTATUS_OK;

    Window root_w;
    Window child_w;
    int root_x;
    int root_y;
    int win_x;
    int win_y;
    unsigned int mask_return;
    static int root_x_s = -1;
    static int root_y_s = -1;
    static int cursorindex = 6;

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);
    if (localDisplay == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;
    drawable = _FindDrawable(driDisplay, Drawable);

    if ( drawable )
    {
        context = drawable->contextPriv;
        LINUX_LOCK_FRAMEBUFFER(context);
    }


    if (drawable && drawable->fullScreenMode) {
        if ( drawable->fullscreenCovered == 0)
        {
                ResolveTarget = driDisplay->renderTarget;
        }
    }


    if (drawable) {
        if ( drawable->rsFunc == (RSFUNC)gcvNULL )
        {
            gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {ResolveTarget, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.rectSize.x = *Width;
            rlvArgs.uArgs.v2.rectSize.y = *Height;
            rlvArgs.uArgs.v2.numSlices  = 1;
            rlvArgs.uArgs.v2.yInverted = yInverted;

            status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
        } else {
            gcsPOINT srcOrigin, size;
            srcOrigin.x = 0;
            srcOrigin.y = 0;
            size.x = *Width;
            size.y = *Height;
            status = (drawable->rsFunc)(RenderTarget, ResolveTarget, &srcOrigin, &srcOrigin, &size);
        }
        if (gcmIS_ERROR(status))
        {
            goto ENDXWIN;
        }
        gcoHAL_Commit(gcvNULL, gcvFALSE);
    }

    if (drawable && drawable->fullScreenMode) {
        /* Full screen apps, image is directly resolved to on screen buffer */
        LINUX_UNLOCK_FRAMEBUFFER(context);
        XQueryPointer(driDisplay->dpy, DefaultRootWindow(driDisplay->dpy), &root_w, &child_w, &root_x, &root_y, &win_x, &win_y, &mask_return);
        if ( root_x != root_x_s || root_y != root_y_s )
        {
           root_x_s = root_x;
           root_y_s = root_y;
           cursorindex = 6;
        } else {
           cursorindex--;
        }

        if ( cursorindex > 0) {
           XFixesHideCursor(driDisplay->dpy,Drawable);
           XFlush(driDisplay->dpy);
           gcoHAL_Commit(gcvNULL, gcvFALSE);
           XFixesShowCursor(driDisplay->dpy,Drawable);
        }
        gcmFOOTER();
        return status;
    }

    if (drawable) {
        alignedDestWidth = drawable->wWidth;

        gcoSURF_GetAlignedSize(
                        ResolveTarget,
                        &alignedWidth,
                        &alignedHeight,
                        gcvNULL
        );

        if (!drawable->drawableResize) {

            srcStart = ResolveBits;
            pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
            if ( (drawable->backNode == 0) && (drawable->numClipRects > 0) )
            {
                pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
                for (i = 0; i < drawable->numClipRects; i++)
                {
                            destStart = driDisplay->linearAddr;
                            srcStart = ResolveBits;
                            xoffset = pClipRects->x1 - drawable->xWOrigin;
                            yoffset = pClipRects->y1 - drawable->yWOrigin;
                            width = pClipRects->x2 - pClipRects->x1;
                            height = pClipRects->y2 - pClipRects->y1;
                            srcStart += (alignedWidth*yoffset + xoffset ) * driDisplay->bpp;
                            xoffset = pClipRects->x1;
                            yoffset = pClipRects->y1;
                            destStart += (alignedDestWidth * yoffset + xoffset) * driDisplay->bpp;

                            for (j = 0; j < height; j++) {
                            memcpy((destStart + j * alignedDestWidth * driDisplay->bpp), (srcStart + j * alignedWidth * driDisplay->bpp), width * driDisplay->bpp);
                            }

                            pClipRects++;
                }
            }
        } else {
            drawable->drawableResize = gcvFALSE;
        }
        *Width = drawable->w;
        *Height = drawable->h;
    }

ENDXWIN:

    if ( drawable )
    {
        context = drawable->contextPriv;
        LINUX_UNLOCK_FRAMEBUFFER(context);
    }

    gcmFOOTER();
    return status;
}

#endif

#if !defined(DRI_PIXMAPRENDER_ASYNC)
static gcoSURF _GetWrapSurface(gcoSURF ResolveTarget, gctPOINTER phyaddr, gctPOINTER lineaddr, gctUINT width, gctUINT height, gctUINT stride)
{
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    static gcoSURF tarsurf = gcvNULL;

    gceSTATUS status = gcvSTATUS_OK;

    if ( tarsurf )
        gcoSURF_Destroy(tarsurf);

    gcoSURF_GetFormat(ResolveTarget, &surftype, &surfformat);

    do {
        /* Construct a wrapper around the pixmap.  */
        gcmERR_BREAK(gcoSURF_ConstructWrapper(
        gcvNULL,
        &tarsurf
        ));

        /* Set the underlying frame buffer surface. */
        gcmERR_BREAK(gcoSURF_SetBuffer(
        tarsurf,
        surftype,
        surfformat,
        stride,
        lineaddr,
        (gctUINT32)phyaddr
        ));

        /* Set the window. */
        gcmERR_BREAK(gcoSURF_SetWindow(
        tarsurf,
        0,
        0,
        width,
        height
        ));

        return tarsurf;
    } while(0);

    return gcvNULL;
}

static gceSTATUS _SwapBuffersWithDirectMode(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gctPOINTER  destLogicalAddr[3] = {0, 0, 0};
    gctUINT8_PTR  destStart;
    gctUINT8_PTR  destPhyStart;
    gctUINT64   destPhys[3] = {0,0,0};
    gctUINT alignedDestWidth;
    gctUINT height, width;
    gceSTATUS status = gcvSTATUS_OK;
    drm_clip_rect_t *pClipRects;
    gctUINT xoffset, yoffset;
    XserverRegion region;
    XRectangle damagedRect[1];
    gcoSURF wrapsurf = gcvNULL;


    driDisplay = (__DRIDisplay*)localDisplay;

    drawable = _FindDrawable( driDisplay, Drawable );
    context = drawable->contextPriv;

    if ( drawable )
    {
        if ( drawable->fullScreenMode)
        {
            if ( drawable->fullscreenCovered == 0 )
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        /* resolve function needs aligned position and size */
        /* so only handle one */
        if ( drawable->numClipRects < 1)
            return gcvSTATUS_INVALID_ARGUMENT;
        if ( drawable->numClipRects > 1)
            return gcvSTATUS_INVALID_ARGUMENT;

         pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
         xoffset = pClipRects->x1 - drawable->xWOrigin;
         yoffset = pClipRects->y1 - drawable->yWOrigin;

        if (xoffset != 0 || yoffset != 0)
            return gcvSTATUS_INVALID_ARGUMENT;

        width = pClipRects->x2 - pClipRects->x1;
        height = pClipRects->y2 - pClipRects->y1;

        if ( *Width != width || *Height != height )
            return gcvSTATUS_INVALID_ARGUMENT;

    }

    if (drawable) {
        status = gcvSTATUS_INVALID_ARGUMENT;
        context = drawable->contextPriv;
        LINUX_LOCK_FRAMEBUFFER(context);
        if (drawable->backNode && !drawable->drawableResize)
        {

            _LockVideoNode(0, drawable->backNode, &destPhys[0], &destLogicalAddr[0]);
            drawable->backBufferPhysAddr = (gctUINT)destPhys[0];
            drawable->backBufferLogicalAddr = destLogicalAddr[0];

            if ( drawable->numClipRects) {

                alignedDestWidth = drawable->wWidth;
                destStart = (gctUINT8_PTR)drawable->backBufferLogicalAddr;
                destPhyStart = (gctUINT8_PTR)drawable->backBufferPhysAddr;

                pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
                width = pClipRects->x2 - pClipRects->x1;
                height = pClipRects->y2 - pClipRects->y1;
                xoffset = pClipRects->x1;
                yoffset = pClipRects->y1;
                destStart += (alignedDestWidth * yoffset + xoffset) * driDisplay->bpp;
                destPhyStart += (alignedDestWidth * yoffset + xoffset) * driDisplay->bpp;

                wrapsurf = _GetWrapSurface(ResolveTarget, (gctPOINTER)destPhyStart, (gctPOINTER)destStart, width, height, alignedDestWidth*driDisplay->bpp);

                if ( wrapsurf )
                {
                    if ( drawable->rsFunc == (RSFUNC)gcvNULL )
                    {
                        gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
                        gcsSURF_VIEW wrapView = {wrapsurf, 0, 1};

                        status = gcoSURF_ResolveRect(&rtView, &wrapView, gcvNULL);
                    } else {
                        status = (drawable->rsFunc)(RenderTarget, wrapsurf, gcvNULL, gcvNULL, gcvNULL);
                    }
                    gcoHAL_Commit(gcvNULL, gcvFALSE);
                }

            }
            _UnlockVideoNode(0, drawable->backNode);
            drawable->drawableResize = gcvFALSE;
        }

        LINUX_UNLOCK_FRAMEBUFFER(context);

        if (gcmIS_ERROR(status))
        {
                return status;
        }

#if defined(VIV_EXT)
        if ( wrapsurf )
        VIVEXTDrawableFlush(driDisplay->dpy, (unsigned int)(driDisplay->screen),(unsigned int)Drawable);
#endif

        /* Report damage to Xserver */
        damagedRect[0].x = 0;
        damagedRect[0].y = 0;
        damagedRect[0].width = drawable->w;
        damagedRect[0].height = drawable->h;
        region = XFixesCreateRegion (driDisplay->dpy, &damagedRect[0], 1);
        XDamageAdd (driDisplay->dpy, Drawable, region);
        XFixesDestroyRegion(driDisplay->dpy, region);
        XFlush(driDisplay->dpy);

        *Width = drawable->w;
        *Height = drawable->h;
    }

    return gcvSTATUS_OK;
}
#endif

#ifdef DRI_PIXMAPRENDER
static gceSTATUS
dri_SwapBuffersUnityEx(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    IN gctBOOL yInverted,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{

    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gctPOINTER  destLogicalAddr[3] = {0, 0, 0};
    gctUINT8_PTR  destStart;
    gctUINT8_PTR  srcStart;
    gctUINT64   destPhys[3] = {0,0,0};
    gctUINT i, j;
    gctUINT alignedWidth, alignedHeight;
    gctUINT alignedDestWidth;
    gctUINT height, width;
    gceSTATUS status = gcvSTATUS_OK;
    drm_clip_rect_t *pClipRects;
    gctUINT xoffset, yoffset;
    XserverRegion region;
    XRectangle damagedRect[1];

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);
    if (localDisplay == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;

    drawable = _FindDrawable( driDisplay, Drawable );
    context = drawable->contextPriv;

    if (_SwapBuffersWithDirectMode(localDisplay, Drawable, RenderTarget, ResolveTarget, ResolveBits, Width, Height) == gcvSTATUS_OK)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (drawable) {
        if ( drawable->rsFunc == (RSFUNC)gcvNULL )
        {
            gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {ResolveTarget, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.rectSize.x = *Width;
            rlvArgs.uArgs.v2.rectSize.y = *Height;
            rlvArgs.uArgs.v2.numSlices  = 1;
            rlvArgs.uArgs.v2.yInverted = yInverted;

            status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
        } else {
            gcsPOINT srcOrigin, size;
            srcOrigin.x = 0;
            srcOrigin.y = 0;
            size.x = *Width;
            size.y = *Height;
            status = (drawable->rsFunc)(RenderTarget, ResolveTarget, &srcOrigin, &srcOrigin, &size);
        }
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    if (drawable) {
        gcoHAL_Commit(gcvNULL, gcvFALSE);
        context = drawable->contextPriv;
        LINUX_LOCK_FRAMEBUFFER(context);
        if (drawable->backNode && !drawable->drawableResize)
        {

            _LockVideoNode(0, drawable->backNode, &destPhys[0], &destLogicalAddr[0]);
            drawable->backBufferPhysAddr = (gctUINT)destPhys[0];
            drawable->backBufferLogicalAddr = destLogicalAddr[0];

            gcoSURF_GetAlignedSize(
                    ResolveTarget,
                    &alignedWidth,
                    &alignedHeight,
                    gcvNULL
                    );

            alignedDestWidth = drawable->wWidth;

            srcStart = ResolveBits;
            destStart = destLogicalAddr[0];

            if (drawable->numClipRects > 0)
            {
                    pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
                    for (i = 0; i < drawable->numClipRects; i++)
                    {
                        destStart = destLogicalAddr[0];
                        srcStart = ResolveBits;
                        xoffset = pClipRects->x1 - drawable->xWOrigin;
                        yoffset = pClipRects->y1 - drawable->yWOrigin;
                        width = pClipRects->x2 - pClipRects->x1;
                        height = pClipRects->y2 - pClipRects->y1;
                        srcStart += (alignedWidth*yoffset + xoffset ) * driDisplay->bpp;
                        xoffset = pClipRects->x1;
                        yoffset = pClipRects->y1;
                        destStart += (alignedDestWidth * yoffset + xoffset) * driDisplay->bpp;

                        for (j = 0; j < height; j++) {
                        memcpy((destStart + j * alignedDestWidth * driDisplay->bpp), (srcStart + j * alignedWidth * driDisplay->bpp), width * driDisplay->bpp);
                        }

                        pClipRects++;
                    }
            }

            _UnlockVideoNode(0, drawable->backNode);
        }
        LINUX_UNLOCK_FRAMEBUFFER(context);
        drawable->drawableResize = gcvFALSE;

        damagedRect[0].x = 0;
        damagedRect[0].y = 0;
        damagedRect[0].width = drawable->w;
        damagedRect[0].height = drawable->h;
        region = XFixesCreateRegion (driDisplay->dpy, &damagedRect[0], 1);
        XDamageAdd (driDisplay->dpy, Drawable, region);
        XFixesDestroyRegion(driDisplay->dpy, region);
        XFlush(driDisplay->dpy);

        *Width = drawable->w;
        *Height = drawable->h;
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}
#endif


#if !defined(DRI_PIXMAPRENDER) && !defined(DRI_PIXMAPRENDER_ASYNC)
static gceSTATUS
dri_SwapBuffersUnity(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    IN gctBOOL yInverted,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{

    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gctPOINTER  destLogicalAddr[3] = {0, 0, 0};
    gctUINT8_PTR  destStart;
    gctUINT8_PTR  srcStart;
    gctUINT64   destPhys[3] = {0,0,0};
    gctUINT i, j;
    gctUINT alignedWidth, alignedHeight;
    gctUINT alignedDestWidth;
    gctUINT height, width;
    gceSTATUS status = gcvSTATUS_OK;
    drm_clip_rect_t *pClipRects;
    gctUINT xoffset, yoffset;
    XserverRegion region;
    XRectangle damagedRect[1];

    Window root_w;
    Window child_w;
    int root_x;
    int root_y;
    int win_x;
    int win_y;
    unsigned int mask_return;
    static int root_x_s = -1;
    static int root_y_s = -1;
    static int cursorindex = 6;

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);
    if (localDisplay == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;

    drawable = _FindDrawable( driDisplay, Drawable );
    context = drawable->contextPriv;

    if (_SwapBuffersWithDirectMode(localDisplay, Drawable, RenderTarget, ResolveTarget, ResolveBits, Width, Height) == gcvSTATUS_OK)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (drawable && drawable->fullScreenMode) {
        /* If drawable is full and covered by another win, we can't resolve it to screen directly */
        /* If covered, go normal path */
        if ( drawable->fullscreenCovered == 0)
            ResolveTarget = driDisplay->renderTarget;
    }

    if (drawable) {
        if ( drawable->rsFunc == (RSFUNC)gcvNULL )
        {
            gcsSURF_VIEW rtView = {RenderTarget, 0, 1};
            gcsSURF_VIEW tgtView = {ResolveTarget, 0, 1};
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.rectSize.x = *Width;
            rlvArgs.uArgs.v2.rectSize.y = *Height;
            rlvArgs.uArgs.v2.numSlices  = 1;
            rlvArgs.uArgs.v2.yInverted = yInverted;

            status = gcoSURF_ResolveRect(&rtView, &tgtView, &rlvArgs);
        } else {
            gcsPOINT srcOrigin, size;
            srcOrigin.x = 0;
            srcOrigin.y = 0;
            size.x = *Width;
            size.y = *Height;
            status = (drawable->rsFunc)(RenderTarget, ResolveTarget, &srcOrigin, &srcOrigin, &size);
        }
        if (gcmIS_ERROR(status))
        {
            gcmFOOTER();
            return status;
        }
    }

    if (drawable && drawable->fullScreenMode && (drawable->fullscreenCovered == 0 ) ) {
       /* Full screen apps, image is directly resolved to on screen buffer
        * Force the xserver to update the area where the cursor is
       * Fix the garbage image issue when moving win with compiz
        */
       XQueryPointer(driDisplay->dpy, DefaultRootWindow(driDisplay->dpy), &root_w, &child_w, &root_x, &root_y, &win_x, &win_y, &mask_return);
       if ( root_x != root_x_s || root_y != root_y_s )
       {
           root_x_s = root_x;
           root_y_s = root_y;
           cursorindex = 6;
       } else {
           cursorindex--;
       }

       if ( cursorindex > 0) {
           XFixesHideCursor(driDisplay->dpy,Drawable);
           XFlush(driDisplay->dpy);
           gcoHAL_Commit(gcvNULL, gcvFALSE);
           XFixesShowCursor(driDisplay->dpy,Drawable);
       }
        gcmFOOTER();
        return gcvSTATUS_OK;
    }


    if (drawable) {
        gcoHAL_Commit(gcvNULL, gcvFALSE);
        context = drawable->contextPriv;
        LINUX_LOCK_FRAMEBUFFER(context);
        if (drawable->backNode && !drawable->drawableResize)
        {

            _LockVideoNode(0, drawable->backNode, &destPhys[0], &destLogicalAddr[0]);
            drawable->backBufferPhysAddr = (gctUINT)destPhys[0];
            drawable->backBufferLogicalAddr = destLogicalAddr[0];

            gcoSURF_GetAlignedSize(
                    ResolveTarget,
                    &alignedWidth,
                    &alignedHeight,
                    gcvNULL
                    );

            alignedDestWidth = drawable->wWidth;

            srcStart = ResolveBits;
            destStart = destLogicalAddr[0];

            if (drawable->numClipRects > 0)
            {
                    pClipRects = (drm_clip_rect_t *)drawable->pClipRects;
                    for (i = 0; i < drawable->numClipRects; i++)
                    {
                        destStart = destLogicalAddr[0];
                        srcStart = ResolveBits;
                        xoffset = pClipRects->x1 - drawable->xWOrigin;
                        yoffset = pClipRects->y1 - drawable->yWOrigin;
                        width = pClipRects->x2 - pClipRects->x1;
                        height = pClipRects->y2 - pClipRects->y1;
                        srcStart += (alignedWidth*yoffset + xoffset ) * driDisplay->bpp;
                        xoffset = pClipRects->x1;
                        yoffset = pClipRects->y1;
                        destStart += (alignedDestWidth * yoffset + xoffset) * driDisplay->bpp;

                        for (j = 0; j < height; j++) {
                        memcpy((destStart + j * alignedDestWidth * driDisplay->bpp), (srcStart + j * alignedWidth * driDisplay->bpp), width * driDisplay->bpp);
                        }

                        pClipRects++;
                    }
            }

            _UnlockVideoNode(0, drawable->backNode);
        }
        LINUX_UNLOCK_FRAMEBUFFER(context);
        drawable->drawableResize = gcvFALSE;
        if (!drawable->fullScreenMode) {
            /* Report damage to Xserver */
            damagedRect[0].x = 0;
            damagedRect[0].y = 0;
            damagedRect[0].width = drawable->w;
            damagedRect[0].height = drawable->h;
            region = XFixesCreateRegion (driDisplay->dpy, &damagedRect[0], 1);
            XDamageAdd (driDisplay->dpy, Drawable, region);
            XFixesDestroyRegion(driDisplay->dpy, region);
            XFlush(driDisplay->dpy);
        }
        *Width = drawable->w;
        *Height = drawable->h;
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}

#endif

static gctBOOL _legacyPath(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable)
{
    __DRIcontextPriv * context;
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gctBOOL ret = gcvFALSE;
    driDisplay = (__DRIDisplay*)localDisplay;

    drawable = _FindDrawable( driDisplay, Drawable );
    if ( drawable ) {
        context = drawable->contextPriv;
        if ( context )
        {
            LINUX_LOCK_FRAMEBUFFER(context);
            ret= (drawable->backNode == 0)? gcvTRUE : gcvFALSE;
            LINUX_UNLOCK_FRAMEBUFFER(context);
        }
    }
    return ret;
}


static gceSTATUS
dri_RSForSwap(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctPOINTER resolve
    )
{
    __DRIdrawablePriv * drawable;
    __DRIDisplay* driDisplay;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("localDisplay=0x%x", localDisplay);
    if (localDisplay == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    driDisplay = (__DRIDisplay*)localDisplay;
    drawable = _FindDrawable(driDisplay, Drawable);
    drawable->rsFunc = (RSFUNC)resolve;

    gcmFOOTER();
    return status;
}

static gceSTATUS
dri_SwapBuffers(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    gctBOOL yInverted = gcvFALSE;
#ifdef DRI_PIXMAPRENDER_ASYNC
#define dri_SwapBuffersUnityAsync dri_SwapBuffersXwinAsync
#endif
    /* Front buffer swap for NON-UNITY desktops */
    if ( _legacyPath(localDisplay, Drawable) )
    {
#ifdef DRI_PIXMAPRENDER
        return dri_SwapBuffersXwinEx(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);
#else

#ifdef DRI_PIXMAPRENDER_ASYNC

        return dri_SwapBuffersXwinAsync(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);

#else
        return dri_SwapBuffersXwin(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);
#endif

#endif
    }
    else
    {

#ifdef DRI_PIXMAPRENDER
        return dri_SwapBuffersUnityEx(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);
#else

#ifdef DRI_PIXMAPRENDER_ASYNC

        return dri_SwapBuffersUnityAsync(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);

#else
        return dri_SwapBuffersUnity(
                                localDisplay,
                                Drawable,
                                RenderTarget,
                                ResolveTarget,
                                ResolveBits,
                                yInverted,
                                Width,
                                Height);
#endif

#endif
    }
}

static gceSTATUS
dri_DrawImage(
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
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x Left=%d Top=%d Right=%d Bottom=%d Width=%d Height=%d BitsPerPixel=%d Bits=0x%x",
                  Display, Window, Left, Top, Right, Bottom, Width, Height, BitsPerPixel, Bits);

#if defined(VIV_EXT)
    if ( Left == 0
        && Top == 0
        && Right == 0
        && Bottom == 0
        && Width == 0
        && Height == 0
        && BitsPerPixel == 0
        && Bits == gcvNULL )
   {
        VIVEXTDrawableFlush(Display, (unsigned int)0,(unsigned int)Window);
   }
#endif

    gcmFOOTER();
    return status;
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/
static gceSTATUS
dri_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    Window rootWindow = 0;
    gctINT x = 0, y = 0;
    gctUINT width = 0, height = 0, borderWidth = 0, bitsPerPixel = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER  destLogicalAddr = gcvNULL;
    gctPOINTER  phyAddr = gcvNULL;
    PlatformDisplayType tmpDisplay;
    gctPOINTER tDestLogicalAddr[3] = {0, 0, 0};
    gctUINT64 destPhys[3] = {0,0,0};
    gctUINT videoNode = 0;
    gctBOOL mapped = gcvFALSE;
    int tStride = 0;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);

    if (Pixmap == 0)
    {
        /* Pixmap is not a valid pixmap data structure pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if ( Width == NULL && Height == NULL
          && BitsPerPixel == NULL && Stride == NULL
          && Bits == NULL )
    {
        /*UnLock lokced videonode */
        if (_pixmapMapped(Pixmap, &destLogicalAddr, &phyAddr, &tmpDisplay, (gctINT *)&tStride) == gcvFALSE)
        {
            gcmFOOTER();
            status = gcvSTATUS_OK;
            return status;
        }
        if (Display == (PlatformDisplayType)gcvNULL)
        {
            Display = tmpDisplay;
        }
        status = _getPixmapDrawableInfo(Display, Pixmap, &videoNode, &tStride);
        if ((status == gcvSTATUS_OK) && videoNode)
        {
            _UnlockVideoNode(0, videoNode);
            _unSetPixmapMapped(Pixmap);
        }
        gcmFOOTER();
        return status;
    }

    /* Query pixmap parameters. */
    if (XGetGeometry(Display,
        Pixmap,
        &rootWindow,
        &x, &y, &width, &height,
        &borderWidth,
        &bitsPerPixel) == False)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    /* Set back values. */
    if (Width != NULL)
    {
        *Width = width;
    }

    if (Height != NULL)
    {
        *Height = height;
    }

    mapped = _pixmapMapped(Pixmap, &tDestLogicalAddr[0], &phyAddr, &tmpDisplay, (gctINT *)&tStride);

    destLogicalAddr = tDestLogicalAddr[0];
    /* 1st pass: Get Logical & Physical address and stride through DRI */
    if ((Bits != NULL) && (Stride != NULL))
    {
        if ( mapped == gcvFALSE )
        {
                status = _getPixmapDrawableInfo(Display, Pixmap, &videoNode, Stride);
                if ((status == gcvSTATUS_OK) && videoNode)
                {
                    if (_LockVideoNode(0, videoNode, &destPhys[0], &tDestLogicalAddr[0]) == gcvSTATUS_MEMORY_LOCKED)
                    {
                        _UnlockVideoNode(0, videoNode);
                        _LockVideoNode(0, videoNode, &destPhys[0], &tDestLogicalAddr[0]);
                    }
                    _setPixmapMapped(Pixmap, tDestLogicalAddr[0], (gctPOINTER)(gctUINT32)destPhys[0], Display, *Stride);
                    phyAddr = (gctPOINTER)(gctUINT32)destPhys[0];
                    tStride = *Stride;
                }
        }
        *Bits = phyAddr;
        destLogicalAddr = tDestLogicalAddr[0];
    }

    /* 2nd pass: Get the locally stored Logical address */
    if ((BitsPerPixel != NULL) && (Bits != NULL))
    {
        *BitsPerPixel = bitsPerPixel;
        *Bits = destLogicalAddr;
    }

    if ( Stride != NULL )
        *Stride = tStride;

    gcmFOOTER_NO();
    return status;
}

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
        drmUnmap((drmAddress)display->pSAREA, SAREA_MAX);
        drmUnmap((drmAddress)display->linearAddr, display->fbSize);
        drmClose(display->drmFd);
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
dri_GetDisplayBackbufferEx(
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
    return dri_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
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
dri_DrawImageEx(
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
    return dri_DrawImage(Display,
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
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{

    gctPOINTER logicalAddress = 0;
    gctINT XStride;
    gctPOINTER XBits;

/* The first calling dri_GetPixmapInfo makes pixmap cache-pool work */
    if (gcmIS_ERROR(dri_GetPixmapInfo(
                        Display,
                        Pixmap,
                        gcvNULL,
                        gcvNULL,
                        gcvNULL,
                        &XStride,
                        &XBits)))
    {
        return gcvFALSE;
    }

    if (gcmIS_ERROR(dri_GetPixmapInfo(
                        Display,
                        Pixmap,
                        (gctINT_PTR) Width, (gctINT_PTR) Height,
                        (gctINT_PTR) BitsPerPixel,
                        gcvNULL,
                        &logicalAddress)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Return format for pixmap depth. */
    switch (*BitsPerPixel)
    {
    case 16:
        *Format = gcvSURF_R5G6B5;
        *BitsPerPixel = 16;
        break;

    case 24:
        *Format = gcvSURF_X8R8G8B8;
        *BitsPerPixel = 24;
        break;
    case 32:
        *Format = gcvSURF_A8R8G8B8;
        *BitsPerPixel = 32;
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ( Bits != gcvNULL )
        *Bits = logicalAddress;

    /* Success. */
    return gcvSTATUS_OK;
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
       printf("dri_GetPixmapInfo error format\n");
        return gcvSTATUS_INVALID_ARGUMENT;
    }

   if (XGetGeometry(Display,
        Pixmap,
        &rootWindow,
        &x, &y, &w, &h,
        &borderWidth,
        &bitsPerPixel) == False)
    {
        printf("dri_GetPixmapInfo error\n");
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
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    gceSTATUS status;

    status = dri_SetSwapInterval((PlatformWindowType)Surface->hwnd, Interval);

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

    /* Get shortcut. */
    void * win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

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
        status = dri_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
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
        surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

        status = dri_SetDisplayVirtualEx((PlatformDisplayType) Display->hdc,
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

        for (i = 0; i < Region->numRects; i++)
        {
            EGLint left   = Region->rects[i * 4 + 0];
            EGLint top    = Region->rects[i * 4 + 1];
            EGLint width  = Region->rects[i * 4 + 2];
            EGLint height = Region->rects[i * 4 + 3];

            /* Draw image. */
            status = dri_DrawImageEx((PlatformDisplayType) Display->hdc,
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

    status = dri_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
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
    return dri_SynchronousFlip((PlatformDisplayType)Display->hdc);
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
    gctINT width, height, bitsPerPixel;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = dri_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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
    gcmONERROR(dri_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = dri_GetPixmapInfo((PlatformDisplayType) Display->hdc,
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

        pixmapPhysical = (gctUINT32) pixmapBits;
        pixmapBits     = gcvNULL;

        /* Query pixmap bits. */
        status = dri_GetPixmapInfo((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapBpp,
                                     gcvNULL,
                                     &pixmapBits);

        if (gcmIS_ERROR(status))
        {
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
        dri_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
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

static EGLBoolean
_SwapBuffers(
    void * LocalDisplay,
    void * Drawable,
    gcoSURF RenderTarget,
    gcoSURF ResolveTarget,
    void * ResolveBits,
    gctUINT * Width,
    gctUINT * Height
    )
{
    gceSTATUS status;

    status = dri_SwapBuffers(LocalDisplay,
                             (PlatformWindowType) Drawable,
                             RenderTarget,
                             ResolveTarget,
                             ResolveBits,
                             Width,
                             Height);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_RSForSwap(
    void * localDisplay,
    void * Drawable,
    void * RsCallBack
    )
{
    gceSTATUS status;

    status = dri_RSForSwap(localDisplay,
                           (PlatformWindowType) Drawable,
                           RsCallBack);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static struct eglPlatform driPlatform =
{
    EGL_PLATFORM_DRI_VIV,

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
    _SwapBuffers,
    _RSForSwap,
};



#ifdef X11_DRI3
/* For DRI3 auto check if backend supports DRI3, otherwise DRI1 */
VEGLPlatform
_veglGetDRIPlatform(
    void * NativeDisplay
    )
{
    return &driPlatform;
}

#else

VEGLPlatform
veglGetX11Platform(
    void * NativeDisplay
    )
{
    return &driPlatform;
}

#endif
