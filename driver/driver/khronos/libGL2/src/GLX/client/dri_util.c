/* $XFree86: xc/lib/GL/dri/dri_util.c,v 1.6 2003/02/15 22:12:29 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian E. Paul <brian@precisioninsight.com>
 */

/*
 * This file gets compiled into each of the DRI 3D drivers.  This is
 * basically just a collection of utility functions that are useful
 * for most drivers.  A DRI driver doesn't have to use any of this,
 * but it's useful boilerplate.
 *
 *
 * Many of the functions defined here are called from the GL library
 * via function pointers in the __DRIdisplayRec, __DRIscreenRec,
 * __DRIcontextRec, __DRIdrawableRec structures defined in glxclient.h
 *
 * Those function pointers are initialized by code in this file.
 * The process starts when libGL calls the __driCreateScreen() function
 * at the end of this file.
 *
 * The above-mentioned DRI structures have no dependencies on Mesa.
 * Each structure instead has a generic (GLvoid *) private pointer that
 * points to a private structure.  For the current drivers, these private
 * structures are the __DRIdrawablePrivateRec, __DRIcontextPrivateRec,
 * __DRIscreenPrivateRec, and __DRIvisualPrivateRec structures defined
 * in dri_util.h.  We allocate and attach those structs here in
 * this file.
 */

#include "gc_hal.h"
#include <assert.h>
#include <unistd.h>
#include <X11/Xlibint.h>
#include <Xext.h>
#include <extutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include "glxclient.h"
#include "xf86dri.h"
#include "sarea.h"
#include "dri_util.h"

#define VIV_EXT
#if defined(VIV_EXT)

#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>


#define X_VIVEXTQueryVersion        0
#define X_VIVEXTPixmapPhysaddr      1
#define X_VIVEXTDrawableFlush       2
#define X_VIVEXTDrawableInfo        3
#define X_VIVEXTFULLScreenInfo      4


#define VIVEXTNumberEvents      0

#define VIVEXTClientNotLocal        0
#define VIVEXTOperationNotSupported 1
#define VIVEXTNumberErrors      (VIVEXTOperationNotSupported + 1)



#define VIVEXTNAME "vivext"


#define VIVEXT_MAJOR_VERSION    1
#define VIVEXT_MINOR_VERSION    0
#define VIVEXT_PATCH_VERSION    0

typedef struct _VIVEXTQueryVersion {
    CARD8   reqType;
    CARD8   vivEXTReqType;
    CARD16  length B16;
} xVIVEXTQueryVersionReq;
#define sz_xVIVEXTQueryVersionReq   4


typedef struct {
    BYTE    type;/* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD16  majorVersion B16;       /* major version of vivEXT protocol */
    CARD16  minorVersion B16;       /* minor version of vivEXT protocol */
    CARD32  patchVersion B32;       /* patch version of vivEXT protocol */
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
} xVIVEXTQueryVersionReply;
#define sz_xVIVEXTQueryVersionReply 32


typedef struct _VIVEXTDrawableFlush {
    CARD8   reqType;        /* always vivEXTReqCode */
    CARD8   vivEXTReqType;      /* always X_vivEXTDrawableFlush */
    CARD16  length B16;
    CARD32  screen B32;
    CARD32  drawable B32;
} xVIVEXTDrawableFlushReq;
#define sz_xVIVEXTDrawableFlushReq  12

typedef struct _VIVEXTDrawableInfo {
    CARD8   reqType;
    CARD8   vivEXTReqType;
    CARD16  length B16;
    CARD32  screen B32;
    CARD32  drawable B32;
} xVIVEXTDrawableInfoReq;
#define sz_xVIVEXTDrawableInfoReq   12

typedef struct {
    BYTE    type;/* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    INT16   drawableX B16;
    INT16   drawableY B16;
    INT16   drawableWidth B16;
    INT16   drawableHeight B16;
    CARD32  numClipRects B32;
    INT16       relX B16;
    INT16       relY B16;
    CARD32      alignedWidth B32;
    CARD32      alignedHeight B32;
    CARD32      stride B32;
    CARD32      nodeName B32;
    CARD32      phyAddress B32;
} xVIVEXTDrawableInfoReply;

#define sz_xVIVEXTDrawableInfoReply 44

typedef struct _VIVEXTFULLScreenInfo {
    CARD8   reqType;
    CARD8   vivEXTReqType;
    CARD16  length B16;
    CARD32  screen B32;
    CARD32  drawable B32;
} xVIVEXTFULLScreenInfoReq;
#define sz_xVIVEXTFULLScreenInfoReq 12

typedef struct {
    BYTE    type;           /* X_Reply */
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD32  fullscreenCovered B32;  /* if fullscreen is covered by windows, set to 1 otherwise 0 */
    CARD32  pad3 B32;
    CARD32  pad4 B32;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
    CARD32  pad7 B32;       /* bytes 29-32 */
} xVIVEXTFULLScreenInfoReply;
#define sz_xVIVEXTFULLScreenInfoReply 32


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

    if (*numClipRects) {
       int len = sizeof(drm_clip_rect_t) * (*numClipRects);

       *pClipRects = (drm_clip_rect_t *)Xcalloc(len, 1);
       if (*pClipRects)
          _XRead(dpy, (char*)*pClipRects, len);
    } else {
        *pClipRects = NULL;
    }

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


extern __GLdispatchTable *__glNopDispatchTab;
extern Bool __glXDisplayIsClosed;

extern GLvoid __glContextModesDestroy( __GLcontextModes * modes );


/**
 * This is used in a couple of places that call \c driCreateNewDrawable.
 */
static const int empty_attribute_list[1] = { None };


/* forward declarations */
static GLvoid *driCreateNewDrawable(Display *dpy, const __GLcontextModes *modes,
    __DRIid draw, __DRIdrawable *pdraw, int renderType, const int *attrs);

static GLvoid driDestroyDrawable(Display *dpy, GLvoid *drawablePrivate);




/*
** Print message to stderr if LIBGL_DEBUG env var is set.
*/
GLvoid
__driUtilMessage(const char *f, ...)
{
    va_list args;

    if (getenv("LIBGL_DEBUG")) {
        fprintf(stderr, "libGL error: \n");
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
}

/*****************************************************************/

/* Maintain a list of drawables */

static Bool __driAddDrawable(GLvoid *drawHash, __DRIdrawable *pdraw)
{
    __DRIdrawablePrivate *pdp = (__DRIdrawablePrivate *)pdraw->private;

    if (drmHashInsert(drawHash, pdp->draw, pdraw))
        return GL_FALSE;

    return GL_TRUE;
}

__DRIdrawable TEMPdraw;

static __DRIdrawable *__driFindDrawable(GLvoid *drawHash, GLXDrawable draw)
{
    int retcode;
    __DRIdrawable *pdraw = NULL;

    retcode = drmHashLookup(drawHash, draw, (GLvoid **)&pdraw);
    if (retcode)
        return NULL;

    return pdraw;
}

static GLvoid __driRemoveDrawable(GLvoid *drawHash, __DRIdrawable *pdraw)
{
    int retcode;
    __DRIdrawablePrivate *pdp = (__DRIdrawablePrivate *)pdraw->private;

    retcode = drmHashLookup(drawHash, pdp->draw, (GLvoid **)&pdraw);
    if (!retcode) { /* Found */
        drmHashDelete(drawHash, pdp->draw);
    }
}

static Bool __driWindowExistsFlag;

static int __driWindowExistsErrorHandler(Display *dpy, XErrorEvent *xerr)
{
    if (xerr->error_code == BadWindow) {
        __driWindowExistsFlag = GL_FALSE;
    }
    return 0;
}

static Bool __driWindowExists(Display *dpy, GLXDrawable draw)
{
    XWindowAttributes xwa;
    int (*oldXErrorHandler)(Display *, XErrorEvent *);

    __driWindowExistsFlag = GL_TRUE;
    oldXErrorHandler = XSetErrorHandler(__driWindowExistsErrorHandler);
    XGetWindowAttributes(dpy, draw, &xwa); /* dummy request */
    XSetErrorHandler(oldXErrorHandler);
    return __driWindowExistsFlag;
}

static GLvoid __driGarbageCollectDrawables(GLvoid *drawHash)
{
    GLXDrawable draw;
    __DRIdrawable *pdraw;
    Display *dpy;
    Bool forceClean = GL_TRUE;

    if (drmHashFirst(drawHash, &draw, (GLvoid **)&pdraw)) {
        do {
            __DRIdrawablePrivate *pdp = (__DRIdrawablePrivate *)pdraw->private;
            dpy = pdp->driScreenPriv->display;
            if (!__glXDisplayIsClosed) {
                XSync(dpy, GL_FALSE);
                forceClean = !(__driWindowExists(dpy, draw));
            }
            if (forceClean) {
                /* Destroy the local drawable data in the hash table, if the
                   drawable no longer exists in the Xserver */
                __driRemoveDrawable(drawHash, pdraw);
                (*pdraw->destroyDrawable)(dpy, pdraw->private);
                Xfree(pdraw);
            }
        } while (drmHashNext(drawHash, &draw, (GLvoid **)&pdraw));
    }
}

/*****************************************************************/

/*****************************************************************/
/** \name Context (un)binding functions                          */
/*****************************************************************/
/*@{*/

/**
 * Unbind context.
 *
 * \param dpy the display handle.
 * \param scrn the screen number.
 * \param draw drawable.
 * \param read Current reading drawable.
 * \param gc context.
 *
 * \return \c GL_TRUE on success, or \c GL_FALSE on failure.
 *
 * \internal
 * This function calls __DriverAPIRec::UnbindContext, and then decrements
 * __DRIdrawablePrivateRec::refcount which must be non-zero for a successful
 * return.
 *
 * While casting the opaque private pointers associated with the parameters
 * into their respective real types it also assures they are not \c NULL.
 */
static Bool driUnbindContext3(Display *dpy, int scrn,
                              __DRIid draw, __DRIid read, __DRIcontext *ctx)
{
    __DRIscreen *pDRIScreen;
    __DRIdrawable *pdraw;
    __DRIdrawable *pread;
    __DRIcontextPrivate *pcp;
    __DRIscreenPrivate *psp;
    __DRIdrawablePrivate *pdp;
    __DRIdrawablePrivate *prp;

    /*
    ** Assume error checking is done properly in glXMakeCurrent before
    ** calling driUnbindContext3.
    */

    if (ctx == NULL || draw == None || read == None) {
        /* ERROR!!! */
        return GL_FALSE;
    }

    pDRIScreen = __glXFindDRIScreen(dpy, scrn);
    if ( (pDRIScreen == NULL) || (pDRIScreen->private == NULL) ) {
        /* ERROR!!! */
        return GL_FALSE;
    }

    psp = (__DRIscreenPrivate *)pDRIScreen->private;
    pcp = (__DRIcontextPrivate *)ctx->private;

    pdraw = __driFindDrawable(psp->drawHash, draw);
    if (!pdraw) {
        /* ERROR!!! */
        return GL_FALSE;
    }
    pdp = (__DRIdrawablePrivate *)pdraw->private;

    pread = __driFindDrawable(psp->drawHash, read);
    if (!pread) {
        /* ERROR!!! */
        return GL_FALSE;
    }
    prp = (__DRIdrawablePrivate *)pread->private;

    /* Let driver unbind drawable from context */
    (*psp->DriverAPI.UnbindContext)(pcp);

    if (pdp->refcount == 0) {
        /* ERROR!!! */
        return GL_FALSE;
    }

    pdp->refcount--;

    if (prp != pdp) {
        if (prp->refcount == 0) {
            /* ERROR!!! */
            return GL_FALSE;
        }
        prp->refcount--;
    }

    /* XXX this is disabled so that if we call SwapBuffers on an unbound
     * window we can determine the last context bound to the window and
     * use that context's lock. (BrianP, 2-Dec-2000)
     */

    return GL_TRUE;
}


/**
 * This function takes both a read buffer and a draw buffer.  This is needed
 * for \c glXMakeCurrentReadSGI or GLX 1.3's \c glXMakeContextCurrent
 * function.
 *
 * \bug This function calls \c driCreateNewDrawable in two places with the
 *      \c renderType hard-coded to \c GLX_WINDOW_BIT.  Some checking might
 *      be needed in those places when support for pbuffers and / or pixmaps
 *      is added.  Is it safe to assume that the drawable is a window?
 */
static Bool DoBindContext(Display *dpy,
                          __DRIid draw, __DRIid read,
                          __DRIcontext *ctx, const __GLcontextModes * modes,
                          __DRIscreenPrivate *psp)
{
    __DRIdrawable *pdraw;
    __DRIdrawablePrivate *pdp;
    __DRIdrawable *pread;
    __DRIdrawablePrivate *prp;
    __DRIcontextPrivate * const pcp = ctx->private;

    /* Find the _DRIdrawable which corresponds to the writing drawable. */
    pdraw = __driFindDrawable(psp->drawHash, draw);
    if (!pdraw) {
        /* Allocate a new drawable */
        pdraw = (__DRIdrawable *)Xmalloc(sizeof(__DRIdrawable));
        if (!pdraw) {
            /* ERROR!!! */
            return GL_FALSE;
        }

        /* Create a new drawable */
        driCreateNewDrawable(dpy, modes, draw, pdraw, GLX_WINDOW_BIT,
                             empty_attribute_list);
        if (!pdraw->private) {
            /* ERROR!!! */
            Xfree(pdraw);
            return GL_FALSE;
        }
    }
    pdp = (__DRIdrawablePrivate *) pdraw->private;

    /* Find the _DRIdrawable which corresponds to the reading drawable. */
    if (read == draw) {
        /* read buffer == draw buffer */
        prp = pdp;
    }
    else {
        pread = __driFindDrawable(psp->drawHash, read);
        if (!pread) {
            /* Allocate a new drawable */
            pread = (__DRIdrawable *)Xmalloc(sizeof(__DRIdrawable));
            if (!pread) {
                /* ERROR!!! */
                return GL_FALSE;
            }

            /* Create a new drawable */
            driCreateNewDrawable(dpy, modes, read, pread, GLX_WINDOW_BIT,
                                    empty_attribute_list);
            if (!pread->private) {
                /* ERROR!!! */
                Xfree(pread);
                return GL_FALSE;
            }
        }
        prp = (__DRIdrawablePrivate *) pread->private;
    }

    /* Bind the drawable to the context */
    pcp->driDrawablePriv = pdp;
    pdp->driContextPriv = pcp;
    pdp->refcount++;
    if ( pdp != prp ) {
        prp->refcount++;
    }

    /*
    ** Now that we have a context associated with this drawable, we can
    ** initialize the drawable information if has not been done before.
    */
    if (!pdp->pStamp || *pdp->pStamp != pdp->lastStamp) {
        DRM_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
        __driUtilUpdateExtraDrawableInfo(pdp);
        DRM_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
    }

    /* Call device-specific MakeCurrent */
    (*psp->DriverAPI.MakeCurrent)(pcp, pdp, prp);

    return GL_TRUE;
}


/**
 * This function takes both a read buffer and a draw buffer.  This is needed
 * for GLX 1.3's \c glXMakeContextCurrent function.
 */
static Bool driBindContext3(Display *dpy, int scrn,
                            __DRIid draw, __DRIid read,
                            __DRIcontext * ctx)
{
    __DRIscreen *pDRIScreen;

    /*
    ** Assume error checking is done properly in glXMakeCurrent before
    ** calling driBindContext.
    */
    if (ctx == NULL || draw == None || read == None) {
        /* ERROR!!! */
        return GL_FALSE;
    }

    pDRIScreen = __glXFindDRIScreen(dpy, scrn);
    if ( (pDRIScreen == NULL) || (pDRIScreen->private == NULL) ) {
        /* ERROR!!! */
        return GL_FALSE;
    }

    return DoBindContext( dpy, draw, read, ctx, ctx->mode,
                            (__DRIscreenPrivate *)pDRIScreen->private );
}

#if defined(DRI_PIXMAPRENDER_GL)

typedef struct _wrapPixData{
    Pixmap backPixmap;
    GC xgc;
    gcoSURF pixWrapSurf;
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctUINT32 backPixmapNode;
    gctUINT32 directPix;
}wPixData;

static gceSTATUS _LockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node,
        OUT gctUINT64 *Address,
        OUT gctPOINTER *Memory) {
    gceSTATUS status;
    gcsHAL_INTERFACE iface;
    gctUINT8 **pMem = (gctUINT8 **)Memory;
    gcmASSERT(Address != gcvNULL);
    gcmASSERT(Memory != gcvNULL);
    gcmASSERT(Node != 0);

    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
    iface.u.LockVideoMemory.node = Node;
    iface.u.LockVideoMemory.cacheable = gcvFALSE;

    /* Call kernel API. */
    gcmONERROR(gcoHAL_Call(Hal, &iface));

    /* Get allocated node in video memory. */
    *Address = iface.u.LockVideoMemory.physicalAddress;
    *pMem = gcvNULL;
    *pMem += iface.u.LockVideoMemory.memory;

OnError:
    return status;
}

static gceSTATUS _UnlockVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node) {
    gcsHAL_INTERFACE iface;

    gceSTATUS status = gcvSTATUS_OK;
    gcmASSERT(Node != 0);

    iface.engine = gcvENGINE_RENDER;
    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.u.UnlockVideoMemory.node = Node;
    iface.u.UnlockVideoMemory.type = gcvSURF_BITMAP;
    iface.u.UnlockVideoMemory.asynchroneous = gcvTRUE;
/*
    gcmONERROR(gcoHAL_Commit(Hal,gcvTRUE));
*/
    gcmONERROR(gcoOS_DeviceControl(
                        gcvNULL,
                        IOCTL_GCHAL_INTERFACE,
                        &iface,gcmSIZEOF(iface),
                        &iface,gcmSIZEOF(iface)
    ));

    gcmONERROR(iface.status);

    if (iface.u.UnlockVideoMemory.asynchroneous){
        iface.u.UnlockVideoMemory.asynchroneous = gcvFALSE;
        gcmONERROR(gcoHAL_ScheduleEvent(Hal,&iface));
    }

OnError:
    /* Call kernel API. */
    return status;
}

static gceSTATUS _FreeVideoNode(
        IN gcoHAL Hal,
        IN gctUINT32 Node) {
    gcsHAL_INTERFACE iface;

    gcmASSERT(Node != 0);

    iface.command = gcvHAL_RELEASE_VIDEO_MEMORY;
    iface.u.ReleaseVideoMemory.node = Node;

    /* Call kernel API. */
    return gcoHAL_Call(Hal, &iface);
}

static void _createPixmapInfo(
        __DRIdrawablePrivate *dridrawable,
        __DRIid Drawable,
        Pixmap *backPixmap,
        gctUINT32 directPix,
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
    int numRects;
    drm_clip_rect_t *pClipRects;
    unsigned int alignedW;
    unsigned int alignedH;
    unsigned int stride = 0;
    unsigned int physAddr = 0;
    unsigned int depth = 24;
    unsigned int videoNode = 0;
    unsigned int nodeName = 0;
    Window  root;
    gctPOINTER  destLogicalAddr[3] = {0, 0, 0};
    gctUINT64   destPhys[3] = {0,0,0};
    gceSTATUS status = gcvSTATUS_OK;

    __DRInativeDisplay * display;
    __DRIdrawablePrivate *pdp=dridrawable;

    display = dridrawable->display;
    *xgc = XCreateGC(display, Drawable, 0, NULL);

    if ( directPix == 0 )
    {
        XGetGeometry(display, Drawable, &root, &xx, &yy, &ww, &hh, &bb, &depth);
        *backPixmap = XCreatePixmap(display, Drawable,ww, hh, depth);
        XFlush(display);
    }

    VIVEXTDrawableInfo(display, pdp->screen, *backPixmap,
                    &x, &y, &w, &h,
                    &numRects, &pClipRects,
                    &xw,
                    &yw,
                    (unsigned int *)&alignedW,
                    (unsigned int *)&alignedH,
                    &stride,
                    &nodeName,
                    (unsigned int *)&physAddr);

    if ( nodeName)
    gcoHAL_ImportVideoMemory((unsigned int)nodeName, (unsigned int *)&videoNode);
    else
    videoNode = 0;

       *backNode = (gctUINT32)(videoNode);

       if ( *backNode == 0)
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

    if (!__glXDisplayIsClosed) {
        if ( (*backPixmap) && (directPix == 0) )
            XFreePixmap(display, *backPixmap);
        if ( (*xgc) )
            XFreeGC(display, *xgc);
    }

    *backPixmap = (Pixmap)0;
    *xgc =(GC)0;
    *pixWrapSurf = (gcoSURF)0;

    fprintf(stderr, "Warning::Backpixmap can't be created for the current Drawable\n");

}

static void _destroyPixmapInfo(
    __DRInativeDisplay * display,
    Pixmap backPixmap,
    gctUINT32 directPix,
    GC xgc,
    gcoSURF pixWrapSurf,
    gctUINT32 backNode)
{

    if ( pixWrapSurf )
        gcoSURF_Destroy(pixWrapSurf);

    if ( backNode )
    _UnlockVideoNode(0, backNode);

    if ( backNode )
    _FreeVideoNode(0, backNode);

    if (!__glXDisplayIsClosed) {
        if ( (gctUINT32)backPixmap && (directPix == 0) )
            XFreePixmap(display, backPixmap);

        if ( (gctUINT32)xgc )
            XFreeGC(display, xgc);
    }
}
GLvoid _CopyToDrawable(__DRIdrawablePrivate * drawable)
{
    __DRInativeDisplay * display;
    display = drawable->display;
    wPixData *pPixdata = NULL;

    if ( drawable == gcvNULL || display == gcvNULL )
        return;
    gcoHAL_Commit(gcvNULL, gcvTRUE);

    pPixdata = (wPixData *)drawable->wrapPixData;
    if (!__glXDisplayIsClosed) {
#if defined(VIV_EXT)
        if ( __drawableIsPixmap(drawable->draw) && pPixdata->backPixmap )
        VIVEXTDrawableFlush(display, (unsigned int)(drawable->screen),(unsigned int)pPixdata->backPixmap);
#endif
        if ( pPixdata->backPixmap && (pPixdata->directPix == 0 ) )
        {
                XSetGraphicsExposures(display, pPixdata->xgc,0);
                XCopyArea(display, pPixdata->backPixmap, drawable->draw, pPixdata->xgc, 0, 0, drawable->w, drawable->h, 0, 0);
        }
    }
}

#endif


/*****************************************************************/

/*
 * This function basically updates the __DRIdrawablePrivate struct's
 * cliprect information by calling XF86DRIGetDrawableInfo().  This is
 * usually called by the DRI_VALIDATE_DRAWABLE_INFO macro which
 * compares the __DRIdrwablePrivate pStamp and lastStamp values.  If
 * the values are different that means we have to update the clipping
 * info.
 */
GLvoid
__driUtilUpdateDrawableInfo(__DRIdrawablePrivate *pdp)
{
    __DRIscreenPrivate *psp;
    __DRIcontextPrivate *pcp = pdp->driContextPriv;

    if (!pcp || (pdp != pcp->driDrawablePriv)) {
        /* ERROR!!! */
        return;
    }

    psp = pdp->driScreenPriv;
    if (!psp) {
        /* ERROR!!! */
        return;
    }

    if (pdp->pClipRects) {
        Xfree(pdp->pClipRects);
    }

    if (pdp->pBackClipRects) {
        Xfree(pdp->pBackClipRects);
    }

    DRM_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);

    if (!__driFindDrawable(psp->drawHash, pdp->draw) ||
        !XF86DRIGetDrawableInfo(pdp->display, pdp->screen, pdp->draw,
                    &pdp->index, &pdp->lastStamp,
                    &pdp->x, &pdp->y, &pdp->w, &pdp->h,
                    &pdp->numClipRects, &pdp->pClipRects,
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
    }
    else
       pdp->pStamp = &(psp->pSAREA->drawableTable[pdp->index].stamp);

    DRM_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
}

 /*
 * Same as __driUtilUpdateDrawableInfo and be implemented by viv-extension.
 */
GLvoid __driUtilUpdateExtraDrawableInfo(__DRIdrawablePrivate *pdp)
{
    __DRIscreenPrivate *psp;
    __DRIcontextPrivate *pcp = pdp->driContextPriv;
    unsigned int stride;
    int x,y,w,h;
    int numrects;
    drm_clip_rect_t *prects = NULL;
    int backx, backy;
    int numbackrects;
    drm_clip_rect_t *pbackrects = NULL;
    gctUINT32 directPix = 0;

#if defined(DRI_PIXMAPRENDER_GL)
    wPixData *pPixdata = NULL;
    gceSURF_FORMAT hwFormat;
    vvtDeviceInfo *pDevInfo;
#endif

    if (!pcp || (pdp != pcp->driDrawablePriv)) {
        /* ERROR!!! */
        return;
    }

    psp = pdp->driScreenPriv;
    if (!psp) {
        /* ERROR!!! */
        return;
    }

    DRM_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
    if ( __drawableIsPixmap(pdp->draw) == GL_FALSE )
    {
       /* Assign __DRIdrawablePrivate first */
       if (!__driFindDrawable(psp->drawHash, pdp->draw) ||
        !XF86DRIGetDrawableInfo(pdp->display, pdp->screen, pdp->draw,
                    &pdp->index, &pdp->lastStamp,
                    &x, &y, &w, &h,
                    &numrects, &prects,
                    &backx,
                    &backy,
                    &numbackrects,
                    &pbackrects
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
            pdp->pStamp = &(psp->pSAREA->drawableTable[pdp->index].stamp);
    } else {
       pdp->lastStamp = (GLuint) pdp->draw;
       pdp->pStamp = &pdp->lastStamp;
       directPix = 1;
    }

    VIVEXTDrawableInfo(pdp->display, pdp->screen, pdp->draw,
                    &pdp->x, &pdp->y, &pdp->w, &pdp->h,
                    &pdp->numClipRects, &pdp->pClipRects,
                    (int *)&pdp->xWOrigin,
                    (int *)&pdp->yWOrigin,
                    &pdp->wWidth,
                    &pdp->wHeight,
                    &stride,
                    &pdp->nodeName,
                    &pdp->phyAddress);

    if ( pdp->nodeName)
    {

        if ( pdp->backNode )
            _FreeVideoNode(0, pdp->backNode);
        pdp->backNode = 0;

        gcoHAL_ImportVideoMemory((unsigned int)pdp->nodeName, (unsigned int *)&pdp->backNode);
    }
    else
    pdp->backNode = 0;

#if defined(DRI_PIXMAPRENDER_GL)
    pPixdata= (wPixData *)pdp->wrapPixData;
    if (pPixdata && pPixdata->backPixmap )
    {
        _destroyPixmapInfo(pdp->display, pPixdata->backPixmap, pPixdata->directPix, pPixdata->xgc, pPixdata->pixWrapSurf, pPixdata->backPixmapNode);
    }

    if (pPixdata)
            Xfree(pPixdata);

    pdp->wrapPixData = NULL;
    pdp->wrapSurface = NULL;

    pdp->wrapPixData = (GLvoid *)Xmalloc(sizeof(wPixData));
    pPixdata = (wPixData *)pdp->wrapPixData;

    pPixdata->surftype = gcvSURF_BITMAP;
    pPixdata->directPix = directPix;

    if ( directPix )
        pPixdata->backPixmap = (Pixmap)pdp->draw;

    pDevInfo = (vvtDeviceInfo*)psp->pDevPriv;
    if (pDevInfo->bufBpp == 2 )
        hwFormat = gcvSURF_R5G6B5;
    else
        hwFormat = gcvSURF_A8R8G8B8;
    pPixdata->surfformat = hwFormat;
    _createPixmapInfo(pdp, pdp->draw, &pPixdata->backPixmap, directPix, &pPixdata->xgc, &pPixdata->pixWrapSurf, &pPixdata->backPixmapNode, pPixdata->surftype, pPixdata->surfformat);
    pdp->wrapSurface = pPixdata->pixWrapSurf;
#endif

ENDFLAG:
    DRM_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
}

extern GLboolean
__driUtilFullScreenCovered(__DRIdrawablePrivate *pdp)
{
    GLboolean ret;
    __DRIscreenPrivate *psp;

    psp = pdp->driScreenPriv;
    if (!psp) {
        /* ERROR!!! */
        return 0;
    }
    pdp->fullscreenCovered = 0;
    DRM_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
    ret = (GLboolean)VIVEXTFULLScreenInfo(pdp->display, pdp->screen, pdp->draw);
    pdp->fullscreenCovered = (int)ret;
    DRM_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);
    return ret;

}

/*****************************************************************/

/**
 * Swap buffers.
 *
 * \param dpy the display handle.
 * \param drawablePrivate opaque pointer to the per-drawable private info.
 *
 * \internal
 * This function calls __DRIdrawablePrivate::swapBuffers.
 *
 * Is called directly from glXSwapBuffers().
 */
static GLvoid driSwapBuffers(Display *dpy, GLvoid *drawablePrivate)
{
    __DRIdrawablePrivate *dPriv = (__DRIdrawablePrivate *) drawablePrivate;
    dPriv->swapBuffers(dPriv);
}

typedef  GLvoid (*TDESDRAWABLE)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);
typedef  GLvoid (*TSWAPBUFFERS)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);



/**
 * This is called via __DRIscreenRec's createNewDrawable pointer.
 */
static GLvoid *driCreateNewDrawable(Display *dpy,
                                  const __GLcontextModes *modes,
                                  __DRIid draw,
                                  __DRIdrawable *pdraw,
                                  int renderType,
                                  const int *attrs)
{
    __DRIscreen * const pDRIScreen = __glXFindDRIScreen(dpy, modes->screen);
    __DRIscreenPrivate *psp;
    __DRIdrawablePrivate *pdp;

#if defined(DRI_PIXMAPRENDER_GL)
    wPixData *pPixdata = NULL;
    gceSURF_FORMAT hwFormat;
    vvtDeviceInfo *pDevInfo;
#endif

    pdraw->private = NULL;

    /* Since pbuffers are not yet supported, no drawable attributes are
     * supported either.
     */
    (GLvoid) attrs;

    if ( (pDRIScreen == NULL) || (pDRIScreen->private == NULL) ) {
        return NULL;
    }

    pdp = (__DRIdrawablePrivate *)Xmalloc(sizeof(__DRIdrawablePrivate));
    if (!pdp) {
        return NULL;
    }

    if (!XF86DRICreateDrawable(dpy, modes->screen, draw, &pdp->hHWDrawable)) {
        Xfree(pdp);
        return NULL;
    }

    pdp->draw = draw;
    pdp->pdraw = pdraw;
    pdp->refcount = 0;
    pdp->pStamp = NULL;
    pdp->lastStamp = 0;
    pdp->index = 0;
    pdp->x = 0;
    pdp->y = 0;
    pdp->w = 0;
    pdp->h = 0;
    pdp->numClipRects = 0;
    pdp->numBackClipRects = 0;
    pdp->pClipRects = NULL;
    pdp->pBackClipRects = NULL;
    pdp->display = dpy;
    pdp->screen = modes->screen;
    pdp->nodeName = 0;
    pdp->backNode = 0;

    psp = (__DRIscreenPrivate *)pDRIScreen->private;
    pdp->driScreenPriv = psp;
    pdp->driContextPriv = &psp->dummyContextPriv;

    if (!(*psp->DriverAPI.CreateBuffer)(psp, pdp, modes,
                                        renderType == GLX_PIXMAP_BIT)) {
        (GLvoid)XF86DRIDestroyDrawable(dpy, modes->screen, pdp->draw);
         Xfree(pdp);
        return NULL;
    }

    pdraw->private = pdp;
    pdraw->destroyDrawable = (TDESDRAWABLE)driDestroyDrawable;
    pdraw->swapBuffers = (TSWAPBUFFERS)driSwapBuffers;  /* called by glXSwapBuffers() */

    pdp->swapBuffers = psp->DriverAPI.SwapBuffers;

    /* Add pdraw to drawable list */
    if (!__driAddDrawable(psp->drawHash, pdraw)) {
        /* ERROR!!! */
        (*pdraw->destroyDrawable)(dpy, pdp);
        Xfree(pdp);
        pdp = NULL;
        pdraw->private = NULL;
    }

#if defined(DRI_PIXMAPRENDER_GL)
    if (pdp)
    {
        pdp->wrapPixData = (GLvoid *)Xmalloc(sizeof(wPixData));
        pPixdata = (wPixData *)pdp->wrapPixData;
        pdp->doCPYToSCR = _CopyToDrawable;
        pPixdata->surftype = gcvSURF_BITMAP;
        pPixdata->directPix = 0;
        pDevInfo = (vvtDeviceInfo*)psp->pDevPriv;
        if (pDevInfo->bufBpp == 2 )
            hwFormat = gcvSURF_R5G6B5;
        else
            hwFormat = gcvSURF_A8R8G8B8;
        pPixdata->surfformat = hwFormat;
        _createPixmapInfo(pdp, pdp->draw, &pPixdata->backPixmap, 0, &pPixdata->xgc, &pPixdata->pixWrapSurf, &pPixdata->backPixmapNode, pPixdata->surftype, pPixdata->surfformat);
        pdp->wrapSurface = (GLvoid *)pPixdata->pixWrapSurf;
    }
#endif

   return (GLvoid *) pdp;
}

static __DRIdrawable *driGetDrawable(Display *dpy, __DRIid draw,
                                         GLvoid *screenPrivate)
{
    __DRIscreenPrivate *psp = (__DRIscreenPrivate *) screenPrivate;

    /*
    ** Make sure this routine returns NULL if the drawable is not bound
    ** to a direct rendering context!
    */
    return __driFindDrawable(psp->drawHash, draw);
}

static GLvoid driDestroyDrawable(Display *dpy, GLvoid *drawablePrivate)
{
    __DRIdrawablePrivate *pdp = (__DRIdrawablePrivate *) drawablePrivate;
    __DRIscreenPrivate *psp = pdp->driScreenPriv;
    int scrn = psp->myNum;

#if defined(DRI_PIXMAPRENDER_GL)
    wPixData *pPixdata = NULL;
#endif

    if (pdp) {

#if defined(DRI_PIXMAPRENDER_GL)

        pPixdata = (wPixData *)pdp->wrapPixData;

        if (pPixdata && pPixdata->backPixmap )
        _destroyPixmapInfo(dpy, pPixdata->backPixmap,pPixdata->directPix, pPixdata->xgc, pPixdata->pixWrapSurf, pPixdata->backPixmapNode);

        if (pPixdata)
            Xfree(pPixdata);

        pdp->wrapPixData = NULL;
        pdp->wrapSurface = NULL;

#endif

        if ( pdp->backNode && pdp->nodeName )
            _FreeVideoNode(0, pdp->backNode);

        pdp->backNode = 0;
        pdp->nodeName = 0;

        (*psp->DriverAPI.DestroyBuffer)(pdp);
        if (!__glXDisplayIsClosed && __driWindowExists(dpy, pdp->draw)) {
            (GLvoid)XF86DRIDestroyDrawable(dpy, scrn, pdp->draw);
        }
        if (pdp->pClipRects) {
            Xfree(pdp->pClipRects);
            pdp->pClipRects = NULL;
        }
        if (pdp->pBackClipRects) {
            Xfree(pdp->pBackClipRects);
            pdp->pBackClipRects = NULL;
        }
        Xfree(pdp);
    }
}

/*****************************************************************/

/**
 * Destroy the per-context private information.
 *
 * \param dpy the display handle.
 * \param scrn the screen number.
 * \param contextPrivate opaque pointer to the per-drawable private info.
 *
 * \internal
 * This function calls __DriverAPIRec::DestroyContext on \p contextPrivate, calls
 * drmDestroyContext(), and finally frees \p contextPrivate.
 */
static GLvoid driDestroyContext(Display *dpy, int scrn, GLvoid *contextPrivate)
{
    __DRIcontextPrivate  *pcp = (__DRIcontextPrivate *) contextPrivate;

    if (pcp) {
        (*pcp->driScreenPriv->DriverAPI.DestroyContext)(pcp);
        __driGarbageCollectDrawables(pcp->driScreenPriv->drawHash);
        if (!__glXDisplayIsClosed) {
            (GLvoid)XF86DRIDestroyContext(dpy, scrn, pcp->contextID);
        }
        Xfree(pcp);
    }
}


/**
 * Create the per-drawable private driver information.
 *
 * \param dpy           The display handle.
 * \param modes         Mode used to create the new context.
 * \param render_type   Type of rendering target.  \c GLX_RGBA is the only
 *                      type likely to ever be supported for direct-rendering.
 * \param sharedPrivate The shared context dependent methods or \c NULL if
 *                      non-existent.
 * \param pctx          DRI context to receive the context dependent methods.
 *
 * \returns An opaque pointer to the per-context private information on
 *          success, or \c NULL on failure.
 *
 * \internal
 * This function allocates and fills a __DRIcontextPrivateRec structure.  It
 * performs some device independent initialization and passes all the
 * relevent information to __DriverAPIRec::CreateContext to create the
 * context.
 *
 */
typedef   GLvoid (*TDESCXT)(__DRInativeDisplay *dpy, int scrn, GLvoid *contextPrivate);


typedef    GLboolean (*TBINDCXT)(__DRInativeDisplay *dpy, int scrn, __DRIid draw,
                __DRIid read, __DRIcontext *ctx);

typedef    GLboolean (*TUNBINDCXT)(__DRInativeDisplay *dpy, int scrn, __DRIid draw,
               __DRIid read, __DRIcontext *ctx);

static GLvoid *
driCreateNewContext(Display *dpy, const __GLcontextModes *modes,
                    int render_type, GLvoid *sharedPrivate, __DRIcontext *pctx)
{
    __DRIscreen *pDRIScreen;
    __DRIcontextPrivate *pcp;
    __DRIcontextPrivate *pshare = (__DRIcontextPrivate *) sharedPrivate;
    __DRIscreenPrivate *psp;
    GLvoid * const shareCtx = (pshare != NULL) ? pshare->driverPrivate : NULL;

    pDRIScreen = __glXFindDRIScreen(dpy, modes->screen);
    if ( (pDRIScreen == NULL) || (pDRIScreen->private == NULL) ) {
        /* ERROR!!! */
        return NULL;
    }

    psp = (__DRIscreenPrivate *)pDRIScreen->private;

    pcp = (__DRIcontextPrivate *)Xmalloc(sizeof(__DRIcontextPrivate));
    if (!pcp) {
        return NULL;
    }

    if (!XF86DRICreateContextWithConfig(dpy, modes->screen, modes->fbconfigID,
                                        &pcp->contextID, &pcp->hHWContext)) {
        Xfree(pcp);
        return NULL;
    }

    pcp->display = dpy;
    pcp->driScreenPriv = psp;
    pcp->driDrawablePriv = NULL;

    /* When the first context is created for a screen, initialize a "dummy"
     * context.
     */

    if (!psp->dummyContextPriv.driScreenPriv) {
        psp->dummyContextPriv.contextID = 0;
        psp->dummyContextPriv.hHWContext = psp->pSAREA->dummy_context;
        psp->dummyContextPriv.driScreenPriv = psp;
        psp->dummyContextPriv.driDrawablePriv = NULL;
        psp->dummyContextPriv.driverPrivate = NULL;
            /* No other fields should be used! */
    }

    pctx->destroyContext = (TDESCXT)driDestroyContext;
    pctx->bindContext   = (TBINDCXT)driBindContext3;
    pctx->unbindContext = (TUNBINDCXT)driUnbindContext3;

    if ( !(*psp->DriverAPI.CreateContext)(modes, pcp, shareCtx) ) {
        (GLvoid)XF86DRIDestroyContext(dpy, modes->screen, pcp->contextID);
        Xfree(pcp);
        return NULL;
    }

    __driGarbageCollectDrawables(pcp->driScreenPriv->drawHash);

    return pcp;
}

/**
 * Copy context.
 *
 * \internal
 * This function calls __DriverAPIRec::CopyContext on \p contextPrivate.
 */
static GLvoid driCopyContext(Display *dpy, __DRIcontext *src,
                           __DRIcontext *dst, GLuint mask)
{
    __DRIcontextPrivate *srcPri = (__DRIcontextPrivate *)(src->private);
    __DRIcontextPrivate *dstPri = (__DRIcontextPrivate *)(dst->private);

    (*srcPri->driScreenPriv->DriverAPI.CopyContext)(srcPri, dstPri, mask);
}

/*****************************************************************/
/** \name Screen handling functions                              */
/*****************************************************************/
/*@{*/

/**
 * Destroy the per-screen private information.
 *
 * \param dpy the display handle.
 * \param scrn the screen number.
 * \param screenPrivate opaque pointer to the per-screen private information.
 *
 * \internal
 * This function calls __DriverAPIRec::DestroyScreen on \p screenPrivate, calls
 * drmClose(), and finally frees \p screenPrivate.
 */
static GLvoid driDestroyScreen(Display *dpy, int scrn, GLvoid *screenPrivate)
{
    __DRIscreenPrivate *psp = (__DRIscreenPrivate *) screenPrivate;

    /* No interaction with the X-server is possible at this point.  This
     * routine is called after XCloseDisplay, so there is no protocol
     * stream open to the X-server anymore.
     */
    if (psp) {
        /* Destroy all drawbles on this screen */
        __driGarbageCollectDrawables(psp->drawHash);

        if (psp->DriverAPI.DestroyScreen)
            (*psp->DriverAPI.DestroyScreen)(psp);

#if USE_DRM_MAP
        (GLvoid)drmUnmap(psp->pLogicalAddr, psp->fbSize);
#else
        if (psp->pLogicalAddr) {
            munmap(psp->pLogicalAddr,   psp->fbSize);
        }
#endif

        (GLvoid)drmUnmap((drmAddress)psp->pSAREA, SAREA_MAX);
        (GLvoid)drmClose(psp->fd);

        Xfree(psp->pDevPriv);
        if ( psp->modes != NULL ) {
            __glContextModesDestroy( psp->modes );
        }
        Xfree(psp);
    }
}
typedef    GLvoid (*TDESSCN)(__DRInativeDisplay *dpy, int scrn, GLvoid *screenPrivate);
typedef    GLvoid *(*TNEWCXT)(__DRInativeDisplay *dpy, const __GLcontextModes *modes, int render_type, GLvoid *sharedPrivate, __DRIcontext *pctx);
typedef    GLvoid *(*TCPYCXT)(__DRInativeDisplay *dpy, __DRIcontext *src, __DRIcontext *dst, GLuint mask);
typedef  GLvoid *(*TNEWDRAWABLE)(__DRInativeDisplay *dpy, const __GLcontextModes *modes, __DRIid draw, __DRIdrawable *pdraw,int renderType, const int *attrs);
typedef  __DRIdrawable *(*TGETDRAWABLE)(__DRInativeDisplay *dpy, __DRIid draw, GLvoid *drawablePrivate);
/**
 * Utility function used to create a new driver-private screen structure.
 *
 * \param dpy   Display pointer
 * \param scrn  Index of the screen
 * \param psc   DRI screen data (not driver private)
 * \param modes Linked list of known display modes.  This list is, at a
 *              minimum, a list of modes based on the current display mode.
 *              These roughly match the set of available X11 visuals, but it
 *              need not be limited to X11!  The calling libGL should create
 *              a list that will inform the driver of the current display
 *              mode (i.e., color buffer depth, depth buffer depth, etc.).
 * \param ddx_version Version of the 2D DDX.  This may not be meaningful for
 *                    all drivers.
 * \param dri_version Version of the "server-side" DRI.
 * \param drm_version Version of the kernel DRM.
 * \param frame_buffer Data describing the location and layout of the
 *                     framebuffer.
 * \param pSAREA       Pointer the the SAREA.
 * \param fd           Device handle for the DRM.
 * \param internal_api_version  Version of the internal interface between the
 *                              driver and libGL.
 * \param driverAPI Driver API functions used by other routines in dri_util.c.
 */
__DRIscreenPrivate *
__driUtilCreateNewScreen(__DRInativeDisplay *dpy, int scrn, __DRIscreen *psc,
                         __GLcontextModes * modes,
                         const __DRIversion * ddx_version,
                         const __DRIversion * dri_version,
                         const __DRIversion * drm_version,
                         const __DRIframebuffer * frame_buffer,
                         GLvoid *pSAREA,
                         int fd,
                         int internal_api_version,
                         const struct __DriverAPIRec *driverAPI)
{
    __DRIscreenPrivate *psp;
    vvtDeviceInfo *pdi;

    psp = (__DRIscreenPrivate *)Xmalloc(sizeof(__DRIscreenPrivate));
    if (!psp) {
        return NULL;
    }

    /* Create the hash table */
    psp->drawHash = drmHashCreate();
    if ( psp->drawHash == NULL ) {
        Xfree( psp );
        return NULL;
    }

    psp->display = dpy;
    psp->myNum = scrn;
    psp->psc = psc;
    psp->modes = modes;
    psp->nopDispatchPtrAddr = &__glNopDispatchTab;

    /*
    ** NOT_DONE: This is used by the X server to detect when the client
    ** has died while holding the drawable lock.  The client sets the
    ** drawable lock to this value.
    */
    psp->drawLockID = 1;

    psp->drmMajor = drm_version->major;
    psp->drmMinor = drm_version->minor;
    psp->drmPatch = drm_version->patch;
    psp->ddxMajor = ddx_version->major;
    psp->ddxMinor = ddx_version->minor;
    psp->ddxPatch = ddx_version->patch;
    psp->driMajor = dri_version->major;
    psp->driMinor = dri_version->minor;
    psp->driPatch = dri_version->patch;

    /* install driver's callback functions */
    memcpy( &psp->DriverAPI, driverAPI, sizeof(struct __DriverAPIRec) );

    psp->pSAREA = pSAREA;

    psp->pFB = frame_buffer->base;
    psp->pLogicalAddr = frame_buffer->dev_priv;
    psp->fbSize = frame_buffer->size;
    psp->fbStride = frame_buffer->stride;
    psp->fbWidth = frame_buffer->width;
    psp->fbHeight = frame_buffer->height;
    psp->devPrivSize = sizeof(vvtDeviceInfo);

    psp->pDevPriv = (GLvoid *)Xmalloc(sizeof(vvtDeviceInfo));

    if (psp->pDevPriv == NULL) {
        Xfree( psp );
        return NULL;
    }

    pdi = (vvtDeviceInfo *)psp->pDevPriv;
    pdi->bufBpp = (DisplayPlanes(dpy, scrn) >> 3);
    pdi->sareaPrivOffset = sizeof(drm_sarea_frame_t);
    pdi->ScrnConf.virtualX = DisplayWidth(dpy, scrn);
    pdi->ScrnConf.virtualY =  DisplayHeight(dpy, scrn);
    psp->fd = fd;

    /*
    ** Do not init dummy context here; actual initialization will be
    ** done when the first DRI context is created.  Init screen priv ptr
    ** to NULL to let CreateContext routine that it needs to be inited.
    */
    psp->dummyContextPriv.driScreenPriv = NULL;

    psc->destroyScreen     = (TDESSCN)driDestroyScreen;
    psc->createNewDrawable = (TNEWDRAWABLE)driCreateNewDrawable;
    psc->getDrawable       = (TGETDRAWABLE)driGetDrawable;
    psc->createNewContext  = (TNEWCXT)driCreateNewContext;
    psc->copyContext       = (TCPYCXT)driCopyContext;

    if ( (psp->DriverAPI.InitDriver != NULL)
                && !(*psp->DriverAPI.InitDriver)(psp) ) {
        Xfree( psp->pDevPriv);
        Xfree( psp );
        return NULL;
    }

    return psp;
}

