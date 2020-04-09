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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include "glxclient.h"

#include "dri_util.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/Xdamage.h>

#ifdef X11_DRI3
#include <X11/Xlib-xcb.h>
#include <X11/xshmfence.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#endif

#include <gc_hal_types.h>
#include <gc_hal_base.h>


extern Bool __glXDisplayIsClosed;

#if defined(X11_DRI3) && defined(DRI_PIXMAPRENDER_GL)

extern GLvoid __glContextModesDestroy( __GLcontextModes * modes );

/**
 * This is used in a couple of places that call \c driCreateNewDrawable.
 */
static const int empty_attribute_list[1] = { None };


/* forward declarations */
GLvoid *dri3CreateNewDrawable(Display *dpy, const __GLcontextModes *modes,
    __DRIid draw, __DRIdrawable *pdraw, int renderType, const int *attrs);

static GLvoid dri3DestroyDrawable(Display *dpy, GLvoid *drawablePrivate);

static int create_fd_from_pixmap(xcb_connection_t *c, Pixmap pixmap, int *stride) {
    xcb_dri3_buffer_from_pixmap_cookie_t cookie;
    xcb_dri3_buffer_from_pixmap_reply_t *reply;
    int fd = 0;

    cookie = xcb_dri3_buffer_from_pixmap(c, pixmap);
    reply = xcb_dri3_buffer_from_pixmap_reply(c, cookie, NULL);
    if (!reply)
        return -1;

    if (reply->nfd != 1)
        return -1;

    *stride = reply->stride;
    fd = xcb_dri3_buffer_from_pixmap_reply_fds(c, reply)[0];
    free(reply);
    return fd;
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
static Bool dri3UnbindContext3(Display *dpy, int scrn,
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
        dri3CreateNewDrawable(dpy, modes, draw, pdraw, GLX_WINDOW_BIT,
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
            dri3CreateNewDrawable(dpy, modes, read, pread, GLX_WINDOW_BIT,
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
    __dri3UtilUpdateExtraDrawableInfo(pdp);

    /* Call device-specific MakeCurrent */
    (*psp->DriverAPI.MakeCurrent)(pcp, pdp, prp);

    return GL_TRUE;
}


/**
 * This function takes both a read buffer and a draw buffer.  This is needed
 * for GLX 1.3's \c glXMakeContextCurrent function.
 */
static Bool dri3BindContext3(Display *dpy, int scrn,
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

typedef struct _wrapPixData{
    Pixmap backPixmap;
    GC xgc;
    gcoSURF pixWrapSurf;
    gceSURF_TYPE surftype;
    gceSURF_FORMAT surfformat;
    gctUINT32 directPix;
} wPixData;

static void _createPixmapInfo(
        __DRIdrawablePrivate *dridrawable,
        __DRIid Drawable,
        Pixmap *backPixmap,
        gctUINT32 directPix,
        GC *xgc,
        gcoSURF *pixWrapSurf,
        gceSURF_TYPE surftype,
        gceSURF_FORMAT surfformat
        )
{
    int xx, yy;
    unsigned int ww, hh, bb;
    int pixmapStride = 0;
    unsigned int depth = 24;
    Window  root;
    int pixmapfd = -1;
    Pixmap pixmap = 0;
    gcoSURF surface;
    __DRInativeDisplay * display;
    gceSTATUS status = gcvSTATUS_OK;

    display = dridrawable->display;

    *xgc = XCreateGC(display, Drawable, 0, NULL);

    if (directPix)
        XGetGeometry(display, *backPixmap, &root, &xx, &yy, &ww, &hh, &bb, &depth);
    else
        XGetGeometry(display, Drawable, &root, &xx, &yy, &ww, &hh, &bb, &depth);
    ww = gcmALIGN(ww, 16);

    if (directPix == 0) {
        pixmap = XCreatePixmap(display, Drawable, ww, hh, depth);
        XFlush(display);
    } else {
        pixmap = *backPixmap;
    }

    pixmapfd = create_fd_from_pixmap(XGetXCBConnection(display), pixmap, &pixmapStride);
    if (pixmapfd < 0) {
        goto OnError;
    }

    gcmONERROR(gcoSURF_WrapUserMemory(
        gcvNULL,
        ww,
        hh,
        (gctUINT)pixmapStride,
        1,
        surftype,
        surfformat,
        (gctUINT32)pixmapfd,
        gcvALLOC_FLAG_DMABUF,
        &surface
    ));
    close(pixmapfd);

    *pixWrapSurf = surface;
    *backPixmap = pixmap;
    return;

OnError:
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
    gcoSURF pixWrapSurf
    )
{
    if ( pixWrapSurf )
        gcoSURF_Destroy(pixWrapSurf);

    if ( (gctUINT32)backPixmap && (directPix == 0) )
        XFreePixmap(display, backPixmap);

    if ( (gctUINT32)xgc )
        XFreeGC(display, xgc);

}
static GLvoid _CopyToDrawable(__DRIdrawablePrivate * drawable)
{
    __DRInativeDisplay * display;
    display = drawable->display;
    wPixData *pPixdata = NULL;

    if ( drawable == gcvNULL || display == gcvNULL )
        return;
    gcoHAL_Commit(gcvNULL, gcvTRUE);

    pPixdata = (wPixData *)drawable->wrapPixData;
    if (!__glXDisplayIsClosed) {
        if ( pPixdata->backPixmap && (pPixdata->directPix == 0 ) )
        {
                XSetGraphicsExposures(display, pPixdata->xgc,0);
                XCopyArea(display, pPixdata->backPixmap, drawable->draw, pPixdata->xgc, 0, 0, drawable->w, drawable->h, 0, 0);
        }
    }
}

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
__dri3UtilUpdateDrawableInfo(__DRIdrawablePrivate *pdp)
{
    __DRIscreenPrivate *psp;
    __DRIcontextPrivate *pcp = pdp->driContextPriv;
    unsigned int depth = 24;
    Window    root;
    int xx, yy;
    unsigned int ww, hh, bb;

    if (!pcp || (pdp != pcp->driDrawablePriv)) {
        /* ERROR!!! */
        return;
    }

    psp = pdp->driScreenPriv;
    if (!psp) {
        /* ERROR!!! */
        return;
    }

    XGetGeometry(pdp->display, pdp->draw, &root, &xx, &yy, &ww, &hh, &bb, &depth);

    if ( pdp->oldx == (gctINT)xx && pdp->oldy == (gctINT)yy
          && pdp->oldw == (gctUINT)ww && pdp->oldh == (gctUINT)hh
          && pdp->olddrawable == pdp->draw)
        return;


    pdp->x = xx;
    pdp->y = yy;
    pdp->oldx = (gctINT)xx;
    pdp->oldy = (gctINT)yy;
    pdp->oldw = (gctUINT)ww;
    pdp->oldh = (gctUINT)hh;
    pdp->olddrawable = pdp->draw;
    pdp->wWidth = ww;
    pdp->wHeight = hh;

    pdp->w = ww;
    pdp->h = hh;
}

 /*
 * Same as __driUtilUpdateDrawableInfo and be implemented by viv-extension.
 */
GLvoid __dri3UtilUpdateExtraDrawableInfo(__DRIdrawablePrivate *pdp)
{
    __DRIscreenPrivate *psp;
    __DRIcontextPrivate *pcp = pdp->driContextPriv;
    gctUINT32 directPix = 0;

    unsigned int depth = 24;
    Window    root;
    int xx, yy;
    unsigned int ww, hh, bb;

    wPixData *pPixdata = NULL;
    gceSURF_FORMAT hwFormat;
    vvtDeviceInfo *pDevInfo;


    if (!pcp || (pdp != pcp->driDrawablePriv)) {
        /* ERROR!!! */
        return;
    }

    psp = pdp->driScreenPriv;
    if (!psp) {
        /* ERROR!!! */
        return;
    }

    if ( __drawableIsPixmap(pdp->draw))
    {
       directPix = 1;
    }

    XGetGeometry(pdp->display, pdp->draw, &root, &xx, &yy, &ww, &hh, &bb, &depth);

    if ( pdp->oldx == (gctINT)xx && pdp->oldy == (gctINT)yy
          && pdp->oldw == (gctUINT)ww && pdp->oldh == (gctUINT)hh
          && pdp->olddrawable == pdp->draw)
        return;

    pdp->x = xx;
    pdp->y = yy;

    pdp->oldx = (gctINT)xx;
    pdp->oldy = (gctINT)yy;
    pdp->oldw = (gctUINT)ww;
    pdp->oldh = (gctUINT)hh;
    pdp->olddrawable = pdp->draw;
    pdp->wWidth = ww;
    pdp->wHeight = hh;

    pdp->numClipRects = 0;

    pdp->w = ww;
    pdp->h = hh;



    pPixdata= (wPixData *)pdp->wrapPixData;
    if (pPixdata && pPixdata->backPixmap )
    {
        _destroyPixmapInfo(pdp->display, pPixdata->backPixmap, pPixdata->directPix,
            pPixdata->xgc, pPixdata->pixWrapSurf);
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
    _createPixmapInfo(pdp, pdp->draw, &pPixdata->backPixmap, directPix,
        &pPixdata->xgc, &pPixdata->pixWrapSurf, pPixdata->surftype, pPixdata->surfformat);
    pdp->wrapSurface = pPixdata->pixWrapSurf;

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
static GLvoid dri3SwapBuffers(Display *dpy, GLvoid *drawablePrivate)
{
    __DRIdrawablePrivate *dPriv = (__DRIdrawablePrivate *) drawablePrivate;
    dPriv->swapBuffers(dPriv);
}

typedef  GLvoid (*TDESDRAWABLE)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);
typedef  GLvoid (*TSWAPBUFFERS)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);

/**
 * This is called via __DRIscreenRec's createNewDrawable pointer.
 */
GLvoid *dri3CreateNewDrawable(Display *dpy,
                                  const __GLcontextModes *modes,
                                  __DRIid draw,
                                  __DRIdrawable *pdraw,
                                  int renderType,
                                  const int *attrs)
{
    __DRIscreen * const pDRIScreen = __glXFindDRIScreen(dpy, modes->screen);
    __DRIscreenPrivate *psp;
    __DRIdrawablePrivate *pdp;
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
    pdp->display = dpy;
    pdp->screen = modes->screen;
    pdp->pixtag = __drawableIsPixmap(pdp->draw);

    psp = (__DRIscreenPrivate *)pDRIScreen->private;
    pdp->driScreenPriv = psp;
    pdp->driContextPriv = &psp->dummyContextPriv;

    if (!(*psp->DriverAPI.CreateBuffer)(psp, pdp, modes,
                                        renderType == GLX_PIXMAP_BIT)) {
         Xfree(pdp);
        return NULL;
    }

    pdraw->private = pdp;
    pdraw->destroyDrawable = (TDESDRAWABLE)dri3DestroyDrawable;
    pdraw->swapBuffers = (TSWAPBUFFERS)dri3SwapBuffers;  /* called by glXSwapBuffers() */

    pdp->swapBuffers = psp->DriverAPI.SwapBuffers;

    /* Add pdraw to drawable list */
    if (!__driAddDrawable(psp->drawHash, pdraw)) {
        /* ERROR!!! */
        (*pdraw->destroyDrawable)(dpy, pdp);
        Xfree(pdp);
        pdp = NULL;
        pdraw->private = NULL;
    }

    if (pdp)
    {
        pdp->wrapPixData = NULL;
        pdp->doCPYToSCR = _CopyToDrawable;
        pdp->wrapSurface = NULL;
    }

   return (GLvoid *) pdp;
}

__DRIdrawable *dri3GetDrawable(Display *dpy, __DRIid draw,
                                         GLvoid *screenPrivate)
{
    __DRIscreenPrivate *psp = (__DRIscreenPrivate *) screenPrivate;

    /*
    ** Make sure this routine returns NULL if the drawable is not bound
    ** to a direct rendering context!
    */
    return __driFindDrawable(psp->drawHash, draw);
}

static GLvoid dri3DestroyDrawable(Display *dpy, GLvoid *drawablePrivate)
{
    __DRIdrawablePrivate *pdp = (__DRIdrawablePrivate *) drawablePrivate;
    __DRIscreenPrivate *psp = pdp->driScreenPriv;

    wPixData *pPixdata = NULL;


    if (pdp) {

        pPixdata = (wPixData *)pdp->wrapPixData;

        if (pPixdata && pPixdata->backPixmap )
        _destroyPixmapInfo(dpy, pPixdata->backPixmap,pPixdata->directPix, pPixdata->xgc, pPixdata->pixWrapSurf);

        if (pPixdata)
            Xfree(pPixdata);

        pdp->wrapPixData = NULL;
        pdp->wrapSurface = NULL;

        (*psp->DriverAPI.DestroyBuffer)(pdp);
        if (!__glXDisplayIsClosed && __driWindowExists(dpy, pdp->draw)) {
            ;
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
static GLvoid dri3DestroyContext(Display *dpy, int scrn, GLvoid *contextPrivate)
{
    __DRIcontextPrivate  *pcp = (__DRIcontextPrivate *) contextPrivate;

    if (pcp) {
        (*pcp->driScreenPriv->DriverAPI.DestroyContext)(pcp);
        __driGarbageCollectDrawables(pcp->driScreenPriv->drawHash);
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

GLvoid *
dri3CreateNewContext(Display *dpy, const __GLcontextModes *modes,
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

    pcp->display = dpy;
    pcp->driScreenPriv = psp;
    pcp->driDrawablePriv = NULL;

    /* When the first context is created for a screen, initialize a "dummy"
     * context.
     */

    if (!psp->dummyContextPriv.driScreenPriv) {
        psp->dummyContextPriv.contextID = 0;
        psp->dummyContextPriv.driScreenPriv = psp;
        psp->dummyContextPriv.driDrawablePriv = NULL;
        psp->dummyContextPriv.driverPrivate = NULL;
            /* No other fields should be used! */
    }

    pctx->destroyContext = (TDESCXT)dri3DestroyContext;
    pctx->bindContext   = (TBINDCXT)dri3BindContext3;
    pctx->unbindContext = (TUNBINDCXT)dri3UnbindContext3;

    if ( !(*psp->DriverAPI.CreateContext)(modes, pcp, shareCtx) ) {
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
GLvoid dri3CopyContext(Display *dpy, __DRIcontext *src,
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
GLvoid dri3DestroyScreen(Display *dpy, int scrn, GLvoid *screenPrivate)
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

        Xfree(psp->pDevPriv);
        if ( psp->modes != NULL ) {
            __glContextModesDestroy( psp->modes );
        }
        Xfree(psp);
    }
}
#else

GLvoid __dri3UtilUpdateDrawableInfo(__DRIdrawablePrivate *pdp)
{

}

GLvoid __dri3UtilUpdateExtraDrawableInfo(__DRIdrawablePrivate *pdp)
{

}

GLboolean __dri3UtilFullScreenCovered(__DRIdrawablePrivate *pdp)
{
    return GL_TRUE;
}


GLvoid dri3DestroyScreen(Display *dpy, int scrn, GLvoid *screenPrivate)
{
}

GLvoid *dri3CreateNewDrawable(Display *dpy,
                                  const __GLcontextModes *modes,
                                  __DRIid draw,
                                  __DRIdrawable *pdraw,
                                  int renderType,
                                  const int *attrs)
{
    return NULL;
}

__DRIdrawable *dri3GetDrawable(Display *dpy, __DRIid draw,
                                         GLvoid *screenPrivate)
{
    return NULL;
}

GLvoid *dri3CreateNewContext(Display *dpy, const __GLcontextModes *modes,
                    int render_type, GLvoid *sharedPrivate, __DRIcontext *pctx)
{
    return NULL;
}

GLvoid dri3CopyContext(Display *dpy, __DRIcontext *src,
                           __DRIcontext *dst, GLuint mask)
{

}
#endif
