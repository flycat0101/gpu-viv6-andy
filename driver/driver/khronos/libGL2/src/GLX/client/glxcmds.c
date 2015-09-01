/* $XFree86: xc/lib/GL/glx/glxcmds.c,v 1.19 2003/01/20 21:37:18 tsi Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#include "packsingle.h"
#include "glxclient.h"
#include <extutil.h>
#include <Xext.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <string.h>
#include <assert.h>
#include "glcore/g_disp.h"
#include "glcore/gc_gl_extensions.h"
#include "glcore/glapioffsets.h"
#include "dri_util.h"
#include <pthread.h>

GLuint fakedXID = 1;
static const char __glXGLClientExtensions[] = " ";

static const char __glXGLXClientVendorName[] = "Vivante Corp";
static const char __glXGLXClientVersion[] = "1.4";
static const char __glXGLXDefaultClientExtensions[] =
    "GLX_SGI_make_current_read  GLX_SGIX_fbconfig GLX_SGIX_pbuffer GLX_ARB_get_proc_address";

static const char *__glXGLXClientExtensions = __glXGLXDefaultClientExtensions;

extern int currentDesktop;

/**
 * Get the GLX per-screen data structure associated with a GLX context.
 *
 * \param dpy   Display for which the GLX per-screen information is to be
 *              retrieved.
 * \param scrn  Screen on \c dpy for which the GLX per-screen information is
 *              to be retrieved.
 * \returns A pointer to the GLX per-screen data if \c dpy and \c scrn
 *          specify a valid GLX screen, or NULL otherwise.
 *
 * \todo Should this function validate that \c scrn is within the screen
 *       number range for \c dpy?
 */

static __GLXscreenConfigs *
GetGLXScreenConfigs(Display *dpy, int scrn)
{
    __GLXdisplayPrivate * const priv = __glXInitialize(dpy);

    return (priv->screenConfigs != NULL) ? &priv->screenConfigs[scrn] : NULL;
}

static int
GetGLXPrivScreenConfig( Display *dpy, int scrn, __GLXdisplayPrivate ** ppriv,
            __GLXscreenConfigs ** ppsc )
{
    /* Initialize the extension, if needed.  This has the added value
     * of initializing/allocating the display private
     */
    if ( dpy == NULL ) {
        return GLX_NO_EXTENSION;
    }

    *ppriv = __glXInitialize(dpy);
    if ( *ppriv == NULL ) {
        return GLX_NO_EXTENSION;
    }

    /* Check screen number to see if its valid */
    if ((scrn < 0) || (scrn >= ScreenCount(dpy))) {
        return GLX_BAD_SCREEN;
    }

    /* Check to see if the GL is supported on this screen */
    *ppsc = &((*ppriv)->screenConfigs[scrn]);
    if ( (*ppsc)->configs == NULL ) {
        /* No support for GL on this screen regardless of visual */
        return GLX_BAD_VISUAL;
    }

    return Success;
}

/**
 * Determine if a \c GLXFBConfig supplied by the application is valid.
 *
 * \param dpy     Application supplied \c Display pointer.
 * \param config  Application supplied \c GLXFBConfig.
 *
 * \returns If the \c GLXFBConfig is valid, the a pointer to the matching
 *          \c __GLcontextModes structure is returned.  Otherwise, \c NULL
 *          is returned.
 */
static __GLcontextModes *
ValidateGLXFBConfig( Display * dpy, GLXFBConfig config )
{
    __GLXdisplayPrivate * const priv = __glXInitialize(dpy);
    const unsigned num_screens = ScreenCount(dpy);
    const __GLcontextModes * modes;
    unsigned i;

    if ( priv != NULL ) {
        for ( i = 0 ; i < num_screens ; i++ ) {
            for ( modes = priv->screenConfigs[i].configs
              ; modes != NULL
              ; modes = modes->next ) {
                if ( modes == (__GLcontextModes *) config ) {
                    return (__GLcontextModes *) config;
                }
            }
        }
    }

    return NULL;
}

/**
 * \todo It should be possible to move the allocate of \c client_state_private
 * later in the function for direct-rendering contexts.  Direct-rendering
 * contexts don't need to track client state, so they don't need that memory
 * at all.
 *
 * \todo Eliminate \c __glXInitVertexArrayState.  Replace it with a new
 * function called \c __glXAllocateClientState that allocates the memory and
 * does all the initialization (including the pixel pack / unpack).
 */
static
GLXContext AllocateGLXContext( Display *dpy )
{
    __GLXdisplayPrivate *priv = __glXInitialize(dpy);
    GLXContext gc;
    int bufSize = XMaxRequestSize(dpy) * 4;
    CARD8 opcode;

    if (!dpy)
       return NULL;

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return NULL;
    }

    /* Allocate our context record */
    gc = (GLXContext) Xmalloc(sizeof(struct __GLXcontextRec));
    if (!gc) {
        /* Out of memory */
        return NULL;
    }
    memset(gc, 0, sizeof(struct __GLXcontextRec));

    /* Allocate transport buffer */
    gc->buf = (GLubyte *) Xmalloc(bufSize);
    if (!gc->buf) {
        Xfree(gc);
        return NULL;
    }
    gc->bufSize = bufSize;

    /* Fill in the new context */
    gc->renderMode = GL_RENDER;

    gc->state.storePack.alignment = 4;
    gc->state.storeUnpack.alignment = 4;

    __glXInitVertexArrayState(gc);

    gc->attributes.stackPointer = &gc->attributes.stack[0];

    /*
    ** PERFORMANCE NOTE: A mode dependent fill image can speed things up.
    ** Other code uses the fastImageUnpack bit, but it is never set
    ** to GL_TRUE.
    */
    gc->fastImageUnpack = GL_FALSE;
    gc->fillImage = __glXFillImage;
    gc->isDirect = GL_FALSE;
    gc->pc = gc->buf;
    gc->bufEnd = gc->buf + bufSize;
    if (__glXDebug) {
        /*
        ** Set limit register so that there will be one command per packet
        */
        gc->limit = gc->buf;
    } else {
        gc->limit = gc->buf + bufSize - __GLX_BUFFER_LIMIT_SIZE;
    }
    gc->createDpy = dpy;
    gc->majorOpcode = opcode;

    /*
    ** Constrain the maximum drawing command size allowed to be
    ** transfered using the X_GLXRender protocol request.  First
    ** constrain by a software limit, then constrain by the protocl
    ** limit.
    */
    if (bufSize > __GLX_RENDER_CMD_SIZE_LIMIT) {
        bufSize = __GLX_RENDER_CMD_SIZE_LIMIT;
    }
    if (bufSize > __GLX_MAX_RENDER_CMD_SIZE) {
        bufSize = __GLX_MAX_RENDER_CMD_SIZE;
    }
    gc->maxSmallRenderCommandSize = bufSize;

    /*
    ** Insert gc into priv->contextList.
    */
    gc->next = priv->contextList;
    priv->contextList = gc;

    return gc;
}

/*
** Create a new context.
*/
static
GLXContext CreateContext(Display *dpy, XVisualInfo *vis,
          const __GLcontextModes *fbconfig,
          GLXContext shareList,
          Bool allowDirect, int renderType)
{
    GLXContext gc = NULL;

    if ( dpy == NULL )
        return NULL;

    if ( (vis == NULL) && (fbconfig == NULL) )
        return NULL;

    __glXMutexLock();

    gc = AllocateGLXContext(dpy);
    if (!gc) {
        goto CreateContext_exit;
    }

    if (allowDirect) {
        int screen = (fbconfig == NULL) ? vis->screen : fbconfig->screen;
        __GLXscreenConfigs * const psc = GetGLXScreenConfigs(dpy, screen);
        const __GLcontextModes * mode;

        /* The value of fbconfig cannot change because it is tested
         * later in the function.
         */
        if ( fbconfig == NULL ) {
            mode = psc->configs;
            while ( mode != NULL ) {
                    if ( mode->visualID == vis->visualid ) {
                        break;
                    }
                    mode = mode->next;
            }
            assert( mode != NULL );
            assert( mode->screen == screen );
        }
        else {
            mode = fbconfig;
        }

        if (psc && psc->driScreen.private) {
            GLvoid * const shared = (shareList != NULL)
                ? shareList->driContext.private : NULL;
            gc->driContext.private =
                (*psc->driScreen.createNewContext)(dpy, mode, renderType,
                          shared, &gc->driContext);
            if (gc->driContext.private) {
                gc->isDirect = GL_TRUE;
                gc->screen = mode->screen;
                gc->vid = mode->visualID;
                gc->fbconfigID = mode->fbconfigID;
                gc->driContext.mode = mode;
                /* Fake xid for direct context */
                gc->xid = fakedXID++;
                /* Fake xid for direct context, if XAllocID is called, application will be SG fault */
                //gc->xid = XAllocID(dpy);
            }
        }
    }

    if (!gc->isDirect) {
        /* Send the glXCreateContext request */
        LockDisplay(dpy);
        if ( fbconfig == NULL ) { /* Use glx_1_2 protocol */
            xGLXCreateContextReq *req;

            /* Send the glXCreateContext request */
            GetReq(GLXCreateContext,req);
            req->reqType = gc->majorOpcode;
            req->glxCode = X_GLXCreateContext;
            req->context = gc->xid = XAllocID(dpy);
            req->visual = vis->visualid;
            req->screen = vis->screen;
            req->shareList = shareList ? shareList->xid : None;
            req->isDirect = gc->isDirect;
        }
        else { /* Use glx_1_3 protocol */
            xGLXCreateNewContextReq *req;

            /* Send the glXCreateNewContext request */
            GetReq(GLXCreateNewContext,req);
            req->reqType = gc->majorOpcode;
            req->glxCode = X_GLXCreateNewContext;
            req->context = gc->xid = XAllocID(dpy);
            req->fbconfig = fbconfig->fbconfigID;
            req->screen = fbconfig->screen;
            req->renderType = renderType;
            req->shareList = shareList ? shareList->xid : None;
            req->isDirect = gc->isDirect;
        }

        UnlockDisplay(dpy);
        SyncHandle();
    }

CreateContext_exit:

    __glXMutexUnlock();

    return gc;
}

GLXContext GLX_PREFIX(glXCreateContext)(Display *dpy, XVisualInfo *vis,
                GLXContext shareList, Bool allowDirect)
{
   return CreateContext(dpy, vis, NULL, shareList, allowDirect, 0);
}

GLvoid __glXFreeContext(__GLXdisplayPrivate *priv, __GLXcontext *gc)
{
    __GLXcontext *prevGc;

    /* Remove gc from priv->contextList.
    */
    prevGc = priv->contextList;
    if (prevGc == gc) {
        priv->contextList = gc->next;
    }
    else {
        while (prevGc->next) {
            if (prevGc->next == gc) {
                prevGc->next = gc->next;
                gc->next = NULL;
                break;
            }
            prevGc = prevGc->next;
        }
    }

    if (gc->vendor) XFree((char *) gc->vendor);
    if (gc->renderer) XFree((char *) gc->renderer);
    if (gc->version) XFree((char *) gc->version);
    if (gc->extensions) XFree((char *) gc->extensions);
    __glFreeAttributeState(gc);
    XFree((char *) gc->buf);
    XFree((char *) gc);
}

/*
** Destroy the named context
*/
static GLvoid
DestroyContext(Display *dpy, GLXContext gc)
{
    __GLXdisplayPrivate *priv;
    xGLXDestroyContextReq *req;
    CARD8 opcode;

    if (gc == NULL) {
        return;
    }

    /* Send the glXDestroyContext request for indirect context */
    if (!gc->isDirect) {
        opcode = __glXSetupForCommand(dpy);
        if (!opcode) {
            return;
        }

        /*
        ** This dpy also created the server side part of the context.
        ** Send the glXDestroyContext request.
        */
        LockDisplay(dpy);
        GetReq(GLXDestroyContext,req);
        req->reqType = opcode;
        req->glxCode = X_GLXDestroyContext;
        req->context = gc->xid;
        UnlockDisplay(dpy);
        SyncHandle();
    }

    __glXMutexLock();

    gc->xid = None;

    /* Destroy the direct rendering context */
    if (gc->isDirect) {
        if (gc->driContext.private) {
            (*gc->driContext.destroyContext)(dpy, gc->screen,
                             gc->driContext.private);
            gc->driContext.private = NULL;
        }
    }

    if (gc->currentDpy) {
        /* Have to free later cuz it's in use now */
    } else {
        /* Destroy the handle if not current to anybody */
        priv = __glXInitialize(dpy);
        __glXFreeContext(priv, gc);
    }

    __glXMutexUnlock();

}
GLvoid GLX_PREFIX(glXDestroyContext)(Display *dpy, GLXContext gc)
{
    DestroyContext(dpy, gc);
}

/*
** Return the major and minor version #s for the GLX extension
*/
Bool GLX_PREFIX(glXQueryVersion)(Display *dpy, int *major, int *minor)
{
    __GLXdisplayPrivate *priv;

    /* Init the extension.  This fetches the major and minor version. */
    priv = __glXInitialize(dpy);
    if (!priv) return GL_FALSE;

    if (major) *major = priv->majorVersion;
    if (minor) *minor = priv->minorVersion;
    return GL_TRUE;
}

/*
** Query the existance of the GLX extension
*/
Bool GLX_PREFIX(glXQueryExtension)(Display *dpy, int *errorBase, int *eventBase)
{
    int major_op, erb, evb;
    Bool rv;

    rv = XQueryExtension(dpy, GLX_EXTENSION_NAME, &major_op, &evb, &erb);
    if (rv) {
        if (errorBase) *errorBase = erb;
        if (eventBase) *eventBase = evb;
    }
    return rv;
}

/*
** Put a barrier in the token stream that forces the GL to finish its
** work before X can proceed.
*/
GLvoid GLX_PREFIX(glXWaitGL)(GLvoid)
{
    xGLXWaitGLReq *req;
    GLXContext gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    /* Flush any pending commands out */
    __glXFlushRenderBuffer(gc, gc->pc);

    if (gc->isDirect) {
        glFinish();
        return;
    }

    /* Send the glXWaitGL request */
    LockDisplay(dpy);
    GetReq(GLXWaitGL,req);
    req->reqType = gc->majorOpcode;
    req->glxCode = X_GLXWaitGL;
    req->contextTag = gc->currentContextTag;
    UnlockDisplay(dpy);
    SyncHandle();
}

/*
** Put a barrier in the token stream that forces X to finish its
** work before GL can proceed.
*/
GLvoid GLX_PREFIX(glXWaitX)(GLvoid)
{
    xGLXWaitXReq *req;
    GLXContext gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    /* Flush any pending commands out */
    __glXFlushRenderBuffer(gc, gc->pc);

    if (gc->isDirect) {
        XSync(dpy, False);
        return;
    }

    /*
    ** Send the glXWaitX request.
    */
    LockDisplay(dpy);
    GetReq(GLXWaitX,req);
    req->reqType = gc->majorOpcode;
    req->glxCode = X_GLXWaitX;
    req->contextTag = gc->currentContextTag;
    UnlockDisplay(dpy);
    SyncHandle();
}

GLvoid GLX_PREFIX(glXUseXFont)(Font font, int first, int count, int listBase)
{
    xGLXUseXFontReq *req;
    GLXContext gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    /* Flush any pending commands out */
    (GLvoid) __glXFlushRenderBuffer(gc, gc->pc);

    if (gc->isDirect) {
        DRI_glXUseXFont(font, first, count, listBase);
        return;
    }

    /* Send the glXUseFont request */
    LockDisplay(dpy);
    GetReq(GLXUseXFont,req);
    req->reqType = gc->majorOpcode;
    req->glxCode = X_GLXUseXFont;
    req->contextTag = gc->currentContextTag;
    req->font = font;
    req->first = first;
    req->count = count;
    req->listBase = listBase;
    UnlockDisplay(dpy);
    SyncHandle();
}

/************************************************************************/

/*
** Copy the source context to the destination context using the
** attribute "mask".
*/
GLvoid GLX_PREFIX(glXCopyContext)(Display *dpy, GLXContext source, GLXContext dest,
            unsigned long mask)
{
    __GLXscreenConfigs *psc;
    xGLXCopyContextReq *req;
    GLXContext gc;
    GLXContextTag tag;
    CARD8 opcode;

    if ((!source) || (!dest))
        return; /* BadContext error */

    if (source->isDirect && dest->isDirect) {
        if (dest->currentDpy)
            return; /* BadAccess error */

        if (source->screen != dest->screen)
            return; /* BadMatch error */

        /* Call the copyContext routine */
        psc = GetGLXScreenConfigs(dpy, source->screen);
        (*psc->driScreen.copyContext)(dpy, &source->driContext, &dest->driContext, mask);

        return;
    }

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return;
    }

    /*
    ** If the source is the current context, send its tag so that the context
    ** can be flushed before the copy.
    */
    gc = __glXGetCurrentContext();
    if (source == gc && dpy == gc->currentDpy) {
        tag = gc->currentContextTag;
    } else {
        tag = 0;
    }

    /* Send the glXCopyContext request */
    LockDisplay(dpy);
    GetReq(GLXCopyContext,req);
    req->reqType = opcode;
    req->glxCode = X_GLXCopyContext;
    req->source = source ? source->xid : None;
    req->dest = dest ? dest->xid : None;
    req->mask = mask;
    req->contextTag = tag;
    UnlockDisplay(dpy);
    SyncHandle();
}


/*
** Return GL_TRUE if the context is direct rendering or not.
*/
static Bool __glXIsDirect(Display *dpy, GLXContextID contextID)
{
    xGLXIsDirectReq *req;
    xGLXIsDirectReply reply;
    CARD8 opcode;

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return GL_FALSE;
    }

    /* Send the glXIsDirect request */
    LockDisplay(dpy);
    GetReq(GLXIsDirect,req);
    req->reqType = opcode;
    req->glxCode = X_GLXIsDirect;
    req->context = contextID;
    _XReply(dpy, (xReply*) &reply, 0, False);
    UnlockDisplay(dpy);
    SyncHandle();

    return reply.isDirect;
}

Bool GLX_PREFIX(glXIsDirect)(Display *dpy, GLXContext gc)
{
    if (!gc) {
        return GL_FALSE;
    } else if (gc->isDirect) {
        return GL_TRUE;
    }
    return __glXIsDirect(dpy, gc->xid);
}

GLXPixmap GLX_PREFIX(glXCreateGLXPixmap)(Display *dpy, XVisualInfo *vis, Pixmap pixmap)
{
    xGLXCreateGLXPixmapReq *req;
    GLXPixmap xid;
    CARD8 opcode;

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return None;
    }

    /* Send the glXCreateGLXPixmap request */
    LockDisplay(dpy);
    GetReq(GLXCreateGLXPixmap,req);
    req->reqType = opcode;
    req->glxCode = X_GLXCreateGLXPixmap;
    req->screen = vis->screen;
    req->visual = vis->visualid;
    req->pixmap = pixmap;
    req->glxpixmap = xid = XAllocID(dpy);
    UnlockDisplay(dpy);
    SyncHandle();
    return xid;
}

/*
** Destroy the named pixmap
*/
GLvoid GLX_PREFIX(glXDestroyGLXPixmap)(Display *dpy, GLXPixmap glxpixmap)
{
    xGLXDestroyGLXPixmapReq *req;
    CARD8 opcode;

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return;
    }

    /* Send the glXDestroyGLXPixmap request */
    LockDisplay(dpy);
    GetReq(GLXDestroyGLXPixmap,req);
    req->reqType = opcode;
    req->glxCode = X_GLXDestroyGLXPixmap;
    req->glxpixmap = glxpixmap;
    UnlockDisplay(dpy);
    SyncHandle();
}

GLvoid GLX_PREFIX(glXSwapBuffers)(Display *dpy, GLXDrawable drawable)
{
    xGLXSwapBuffersReq *req;
    GLXContext gc = __glXGetCurrentContext();
    GLXContextTag tag;
    CARD8 opcode;
    __GLXdisplayPrivate *priv;
    __DRIdrawable *pdraw;
    __DRIdrawablePrivate *dPriv;
    XserverRegion region;
    XRectangle damagedRect[1];
    __DRIscreenPrivate *psp;
    vvtDeviceInfo *pDevInfo;

    priv = __glXInitialize(dpy);
    if (priv->driDisplay.private) {
        __GLXscreenConfigs *psc = &priv->screenConfigs[gc->screen];
        if (psc && psc->driScreen.private) {
            /*
            ** getDrawable returning NULL implies that the drawable is
            ** not bound to a direct rendering context.
            */
            pdraw = (*psc->driScreen.getDrawable)(dpy, drawable,
                              psc->driScreen.private);
            if (pdraw) {
                (*pdraw->swapBuffers)(dpy, pdraw->private);

                dPriv = (__DRIdrawablePrivate *)pdraw->private;
                psp = (__DRIscreenPrivate *)(psc->driScreen.private);
                pDevInfo = (vvtDeviceInfo *)psp->pDevPriv;

                if (currentDesktop != 0)
                {
                if ((dPriv->x != 0) || (dPriv->y != 0) ||
                    (dPriv->w != pDevInfo->ScrnConf.virtualX ) ||
                    (dPriv->h != pDevInfo->ScrnConf.virtualY) || dPriv->fullscreenCovered) {
                    /*
                    ** Report damage to Xserver.
                    */
                    dPriv = (__DRIdrawablePrivate *)pdraw->private;
                    damagedRect[0].x = 0;
                    damagedRect[0].y = 0;
                    damagedRect[0].width = dPriv->w;
                    damagedRect[0].height = dPriv->h;
                    region = XFixesCreateRegion(dpy, &damagedRect[0], 1);
                    XDamageAdd(dpy, drawable, region);
                    XFixesDestroyRegion(dpy, region);
                    XFlush(dpy);
                }
                }

                return;
            }
        }
    }

    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return;
    }

    /*
    ** The calling thread may or may not have a current context.  If it
    ** does, send the context tag so the server can do a flush.
    */
    if ((dpy == gc->currentDpy) && (drawable == gc->currentDrawable)) {
        tag = gc->currentContextTag;
    } else {
        tag = 0;
    }

    /* Send the glXSwapBuffers request */
    LockDisplay(dpy);
    GetReq(GLXSwapBuffers,req);
    req->reqType = opcode;
    req->glxCode = X_GLXSwapBuffers;
    req->drawable = drawable;
    req->contextTag = tag;
    UnlockDisplay(dpy);
    SyncHandle();
    XFlush(dpy);
}

/*
** Return configuration information for the given display, screen and
** visual combination.
*/
int GLX_PREFIX(glXGetConfig)(Display *dpy, XVisualInfo *vis, int attribute,
         int *value_return)
{
    __GLcontextModes *pConfig;
    __GLXscreenConfigs *psc;
    __GLXdisplayPrivate *priv;

    /* Initialize the extension, if needed */
    priv = __glXInitialize(dpy);
    if (!priv) {
        /* No extension */
        return GLX_NO_EXTENSION;
    }

    /* Check screen number to see if its valid */
    if ((vis->screen < 0) || (vis->screen >= ScreenCount(dpy))) {
        return GLX_BAD_SCREEN;
    }

    /* Check to see if the GL is supported on this screen */
    psc = &priv->screenConfigs[vis->screen];
    pConfig = psc->configs;
    if (!pConfig) {
        /* No support for GL on this screen regardless of visual */
        if (attribute == GLX_USE_GL) {
            *value_return = GL_FALSE;
            return Success;
        }
        return GLX_BAD_VISUAL;
    }

    /* Lookup attribute after first finding a match on the visual */
    while (pConfig) {
        if (pConfig->visualID == vis->visualid) {
            break;
        }
        pConfig = pConfig->next;
    }

    if (pConfig) {
        switch (attribute) {
          case GLX_USE_GL:
            *value_return = GL_TRUE;
            return Success;
          case GLX_BUFFER_SIZE:
            *value_return =  pConfig->rgbaBits;
            return Success;
          case GLX_RGBA:
            *value_return = pConfig->rgbMode;
            return Success;
          case GLX_RED_SIZE:
            *value_return =  pConfig->redBits;
            return Success;
          case GLX_GREEN_SIZE:
            *value_return =  pConfig->greenBits;
            return Success;
          case GLX_BLUE_SIZE:
            *value_return =  pConfig->blueBits;
            return Success;
          case GLX_ALPHA_SIZE:
            *value_return =  pConfig->alphaBits;
            return Success;
          case GLX_DOUBLEBUFFER:
            *value_return =  pConfig->doubleBufferMode;
            return Success;
          case GLX_STEREO:
            *value_return =  pConfig->stereoMode;
            return Success;
          case GLX_AUX_BUFFERS:
            *value_return =  pConfig->numAuxBuffers;
            return Success;
          case GLX_DEPTH_SIZE:
            *value_return =  pConfig->depthBits;
            return Success;
          case GLX_STENCIL_SIZE:
            *value_return =  pConfig->stencilBits;
            return Success;
          case GLX_ACCUM_RED_SIZE:
            *value_return =  pConfig->accumRedBits;
            return Success;
          case GLX_ACCUM_GREEN_SIZE:
            *value_return =  pConfig->accumGreenBits;
            return Success;
          case GLX_ACCUM_BLUE_SIZE:
            *value_return =  pConfig->accumBlueBits;
            return Success;
          case GLX_ACCUM_ALPHA_SIZE:
            *value_return =  pConfig->accumAlphaBits;
            return Success;
          case GLX_LEVEL:
            *value_return =  pConfig->level;
            return Success;
          case GLX_TRANSPARENT_TYPE_EXT:
            *value_return = pConfig->transparentPixel;
            return Success;
          case GLX_TRANSPARENT_RED_VALUE_EXT:
            *value_return = pConfig->transparentRed;
            return Success;
          case GLX_TRANSPARENT_GREEN_VALUE_EXT:
            *value_return = pConfig->transparentGreen;
            return Success;
          case GLX_TRANSPARENT_BLUE_VALUE_EXT:
            *value_return = pConfig->transparentBlue;
            return Success;
          case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
            *value_return = pConfig->transparentAlpha;
            return Success;
          case GLX_TRANSPARENT_INDEX_VALUE_EXT:
            *value_return = pConfig->transparentIndex;
            return Success;
          case GLX_X_VISUAL_TYPE_EXT:
            switch(pConfig->visualType) {
            case TrueColor:
              *value_return = GLX_TRUE_COLOR_EXT;   break;
            case DirectColor:
              *value_return = GLX_DIRECT_COLOR_EXT; break;
            case PseudoColor:
              *value_return = GLX_PSEUDO_COLOR_EXT; break;
            case StaticColor:
              *value_return = GLX_STATIC_COLOR_EXT; break;
            case GrayScale:
              *value_return = GLX_GRAY_SCALE_EXT;   break;
            case StaticGray:
              *value_return = GLX_STATIC_GRAY_EXT;  break;
            }
            return Success;
          case GLX_VISUAL_CAVEAT_EXT:
            *value_return = pConfig->visualRating;
            return Success;
          default:
            return GLX_BAD_ATTRIBUTE;
        }
    }

    /*
    ** If we can't find the config for this visual, this visual is not
    ** supported by the OpenGL implementation on the server.
    */
    if (attribute == GLX_USE_GL) {
        *value_return = GL_FALSE;
        return Success;
    }
    return GLX_BAD_VISUAL;
}

/************************************************************************/

/*
** Penalize for more auxiliary buffers than requested
*/
static int AuxScore(int minAux, int aux)
{
    return minAux - aux;
}

/*
** If color is desired, give increasing score for amount available.
** Scale this score by a multiplier to make color differences more
** important than other differences.  Otherwise give decreasing score for
** amount available.
*/
static int ColorScore(int minColor, int color)
{
    if (minColor)
        return 4 * (color - minColor);
    else
        return -color;
}

/*
** If accum buffer is desired, give increasing score for amount
** available.  Otherwise give decreasing score for amount available.
*/
static int AccumScore(int minAccum, int accum)
{
    if (minAccum)
        return accum - minAccum;
    else
        return -accum;
}

/*
** Penalize for indexes larger than requested
*/
static int IndexScore(int minIndex, int ix)
{
    return minIndex - ix;
}

/*
** If depth buffer is desired, give increasing score for amount
** available.  Scale this score by a multiplier to make depth differences
** more important than other non-color differences.  Otherwise give
** decreasing score for amount available.
*/
static int DepthScore(int minDepth, int depth)
{
    if (minDepth)
    return 2 * (depth - minDepth);
    else
    return -depth;
}

/*
** Penalize for stencil buffer larger than requested
*/
static int StencilScore(int minStencil, int stencil)
{
    return minStencil - stencil;
}

/* "Logical" xor - like && or ||; would be ^^ */
#define __GLX_XOR(a,b) (((a) && !(b)) || (!(a) && (b)))

/* Fetch a configuration value */
#define __GLX_GCONF(attrib)                                     \
    if (GLX_PREFIX(glXGetConfig)(dpy, thisVis, attrib, &val)) { \
        XFree((char *)visualList);                              \
        return NULL;                                            \
    }


/*
** Return the visual that best matches the template.  Return None if no
** visual matches the template.
*/
XVisualInfo *GLX_PREFIX(glXChooseVisual)(Display *dpy, int screen, int *attribList)
{
    XVisualInfo visualTemplate;
    XVisualInfo *visualList;
    XVisualInfo *thisVis;
    int count, i, maxscore = 0, maxi, score, val, thisVisRating, maxRating = 0;

    /*
    ** Declare and initialize template variables
    */
    int bufferSize = 0;
    int level = 0;
    int rgba = 0;
    int doublebuffer = 0;
    int stereo = 0;
    int auxBuffers = 0;
    int redSize = 0;
    int greenSize = 0;
    int blueSize = 0;
    int alphaSize = 0;
    int depthSize = 0;
    int stencilSize = 0;
    int accumRedSize = 0;
    int accumGreenSize = 0;
    int accumBlueSize = 0;
    int accumAlphaSize = 0;
    /* for visual_info extension */
    int visualType = 0;
    int visualTypeValue = 0;
    int transparentPixel = 0;
    int transparentPixelValue = GLX_NONE_EXT;
    int transparentIndex = 0;
    int transparentIndexValue = 0;
    int transparentRed = 0;
    int transparentRedValue = 0;
    int transparentGreen = 0;
    int transparentGreenValue = 0;
    int transparentBlue = 0;
    int transparentBlueValue = 0;
    /* for visual_rating extension */
    int visualRating = 0;
    int visualRatingValue = GLX_NONE_EXT;

    /*
    ** Get a list of all visuals, return if list is empty
    */
    visualTemplate.screen = screen;
    visualList = XGetVisualInfo(dpy,VisualScreenMask,&visualTemplate,&count);
    if (visualList == NULL)
        return None;

    /*
    ** Build a template from the defaults and the attribute list
    ** Free visual list and return if an unexpected token is encountered
    */
    while (*attribList != None) {
        switch (*attribList++) {
          case GLX_USE_GL:
            break;
          case GLX_BUFFER_SIZE:
            bufferSize = *attribList++;
            break;
          case GLX_LEVEL:
            level = *attribList++;
            break;
          case GLX_RGBA:
            rgba = 1;
            break;
          case GLX_DOUBLEBUFFER:
            doublebuffer = 1;
            break;
          case GLX_STEREO:
            stereo = 1;
            break;
          case GLX_AUX_BUFFERS:
            auxBuffers = *attribList++;
            break;
          case GLX_RED_SIZE:
            redSize = *attribList++;
            break;
          case GLX_GREEN_SIZE:
            greenSize = *attribList++;
            break;
          case GLX_BLUE_SIZE:
            blueSize = *attribList++;
            break;
          case GLX_ALPHA_SIZE:
            alphaSize = *attribList++;
            break;
          case GLX_DEPTH_SIZE:
            depthSize = *attribList++;
            break;
          case GLX_STENCIL_SIZE:
            stencilSize = *attribList++;
            break;
          case GLX_ACCUM_RED_SIZE:
            accumRedSize = *attribList++;
            break;
          case GLX_ACCUM_GREEN_SIZE:
            accumGreenSize = *attribList++;
            break;
          case GLX_ACCUM_BLUE_SIZE:
            accumBlueSize = *attribList++;
            break;
          case GLX_ACCUM_ALPHA_SIZE:
            accumAlphaSize = *attribList++;
            break;
          case GLX_X_VISUAL_TYPE_EXT:
            visualType = 1;
            visualTypeValue = *attribList++;
            break;
          case GLX_TRANSPARENT_TYPE_EXT:
            transparentPixel = 1;
            transparentPixelValue = *attribList++;
            break;
          case GLX_TRANSPARENT_INDEX_VALUE_EXT:
            transparentIndex= 1;
            transparentIndexValue = *attribList++;
            break;
          case GLX_TRANSPARENT_RED_VALUE_EXT:
            transparentRed = 1;
            transparentRedValue = *attribList++;
            break;
          case GLX_TRANSPARENT_GREEN_VALUE_EXT:
            transparentGreen = 1;
            transparentGreenValue = *attribList++;
            break;
          case GLX_TRANSPARENT_BLUE_VALUE_EXT:
            transparentBlue = 1;
            transparentBlueValue = *attribList++;
            break;
          case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
            break;
          case GLX_VISUAL_CAVEAT_EXT:
            visualRating = 1;
            visualRatingValue = *attribList++;
            break;
          default:
            XFree((char *)visualList);
            return None;
        }
    }

    /*
    ** Eliminate visuals that don't meet minimum requirements
    ** Compute a score for those that do
    ** Remember which visual, if any, got the highest score
    */
    maxi = -1;
    for (i = 0; i < count; i++) {
        score = 0;
        thisVis = &visualList[i];    /* NOTE: used by __GLX_GCONF */

        if (thisVis->class == TrueColor || thisVis->class == PseudoColor) {
            /* Bump score by one for TrueColor and PseudoColor visuals. */
            score++;
        }

        __GLX_GCONF(GLX_USE_GL);
        if (! val)
            continue;
        __GLX_GCONF(GLX_LEVEL);
        if (level != val)
            continue;
        __GLX_GCONF(GLX_RGBA);
        if (__GLX_XOR(rgba, val))
            continue;
        __GLX_GCONF(GLX_DOUBLEBUFFER);
        if (__GLX_XOR(doublebuffer, val))
            continue;
        __GLX_GCONF(GLX_STEREO);
        if (__GLX_XOR(stereo, val))
            continue;
        __GLX_GCONF(GLX_AUX_BUFFERS);
        if (auxBuffers > val)
            continue;
        else
            score += AuxScore(auxBuffers, val);
        if (transparentPixel) {
            __GLX_GCONF(GLX_TRANSPARENT_TYPE_EXT);
            if (transparentPixelValue != val)
                continue;
            if (transparentPixelValue == GLX_TRANSPARENT_TYPE_EXT) {
                if (rgba) {
                    __GLX_GCONF(GLX_TRANSPARENT_RGB_EXT);
                    if (transparentRed) {
                        __GLX_GCONF(GLX_TRANSPARENT_RED_VALUE_EXT);
                        if (transparentRedValue != val)
                            continue;
                    }
                    if (transparentGreen) {
                        __GLX_GCONF(GLX_TRANSPARENT_GREEN_VALUE_EXT);
                        if (transparentGreenValue != val)
                            continue;
                    }
                    if (transparentBlue) {
                        __GLX_GCONF(GLX_TRANSPARENT_BLUE_VALUE_EXT);
                        if (transparentBlueValue != val)
                            continue;
                    }
                    /* Transparent Alpha ignored for now */
                } else {
                    __GLX_GCONF(GLX_TRANSPARENT_INDEX_EXT);
                    if (transparentIndex) {
                        __GLX_GCONF(GLX_TRANSPARENT_INDEX_VALUE_EXT);
                        if (transparentIndexValue != val)
                            continue;
                    }
                }
            }
        }
        if (visualType) {
            __GLX_GCONF(GLX_X_VISUAL_TYPE_EXT);
            if (visualTypeValue != val)
                continue;
        } else if (rgba) {
            /* If the extension isn't specified then insure that rgba
            ** and ci return the usual visual types.
            */
            if (!(thisVis->class == TrueColor || thisVis->class == DirectColor))
                continue;
        } else {
            if (!(thisVis->class == PseudoColor
                || thisVis->class == StaticColor))
                continue;
        }

        __GLX_GCONF(GLX_VISUAL_CAVEAT_EXT);
        /**
        ** Unrated visuals are given rating GLX_NONE.
        */
        thisVisRating = val ? val : GLX_NONE_EXT;
        if (visualRating && (visualRatingValue != val))
            continue;
        if (rgba) {
            __GLX_GCONF(GLX_RED_SIZE);
            if (redSize > val)
                continue;
            else
                score += ColorScore(redSize,val);
            __GLX_GCONF(GLX_GREEN_SIZE);
            if (greenSize > val)
                continue;
            else
                score += ColorScore(greenSize, val);
            __GLX_GCONF(GLX_BLUE_SIZE);
            if (blueSize > val)
                continue;
            else
                score += ColorScore(blueSize, val);
            __GLX_GCONF(GLX_ALPHA_SIZE);
            if (alphaSize > val)
                continue;
            else
                score += ColorScore(alphaSize, val);
            __GLX_GCONF(GLX_ACCUM_RED_SIZE);
            if (accumRedSize > val)
                continue;
            else
                score += AccumScore(accumRedSize, val);
            __GLX_GCONF(GLX_ACCUM_GREEN_SIZE);
            if (accumGreenSize > val)
                continue;
            else
                score += AccumScore(accumGreenSize, val);
            __GLX_GCONF(GLX_ACCUM_BLUE_SIZE);
            if (accumBlueSize > val)
                continue;
            else
                score += AccumScore(accumBlueSize, val);
            __GLX_GCONF(GLX_ACCUM_ALPHA_SIZE);
            if (accumAlphaSize > val)
                continue;
            else
                score += AccumScore(accumAlphaSize, val);
        } else {
            __GLX_GCONF(GLX_BUFFER_SIZE);
            if (bufferSize > val)
                continue;
            else
                score += IndexScore(bufferSize, val);
        }
        __GLX_GCONF(GLX_DEPTH_SIZE);
        if (depthSize > val)
            continue;
        else
            score += DepthScore(depthSize, val);
        __GLX_GCONF(GLX_STENCIL_SIZE);
        if (stencilSize > val)
            continue;
        else
            score += StencilScore(stencilSize, val);

        /*
        ** The visual_rating extension indicates that a NONE visual
        ** is always returned in preference to a SLOW one.
        ** Note that enum values are in increasing order (NONE < SLOW).
        */
        if (maxi < 0 || maxRating > thisVisRating) {
            maxi = i;
            maxscore = score;
            maxRating = thisVisRating;
        } else {
            if (score > maxscore) {
                maxi = i;
                maxscore = score;
            }
        }
    }

    /*
    ** If no visual is acceptable, return None
    ** Otherwise, create an XVisualInfo list with just the selected X visual
    **   and return this after freeing the original list
    */
    if (maxi < 0) {
        XFree((char *)visualList);
        return None;
    } else {
        visualTemplate.visualid = visualList[maxi].visualid;
        XFree((char *)visualList);
        visualList = XGetVisualInfo(dpy,VisualScreenMask|VisualIDMask,&visualTemplate,&count);
        return visualList;
    }
}

/*
** Query the Server GLX string and cache it in the display private.
** This routine will allocate the necessay space for the string.
*/
static char *QueryServerString( Display *dpy, int opcode,
                                        int screen, int name )
{
    xGLXQueryServerStringReq *req;
    xGLXQueryServerStringReply reply;
    int length, numbytes, slop;
    char *buf;

    /* Send the glXQueryServerString request */
    LockDisplay(dpy);
    GetReq(GLXQueryServerString,req);
    req->reqType = opcode;
    req->glxCode = X_GLXQueryServerString;
    req->screen = screen;
    req->name = name;
    _XReply(dpy, (xReply*) &reply, 0, False);

    length = reply.length;
    numbytes = reply.n;
    slop = numbytes * __GLX_SIZE_INT8 & 3;
    buf = (char *)Xmalloc(numbytes);
    if (!buf) {
        /* Throw data on the floor */
        _XEatData(dpy, length);
    } else {
        _XRead(dpy, (char *)buf, numbytes);
        if (slop) _XEatData(dpy,4-slop);
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return buf;
}

#define SEPARATOR " "

static char *combine_strings( const char *cext_string, const char *sext_string )
{
    int clen, slen;
    char *combo_string, *token, *s1;
     const char *s2, *end;

    /*
    ** String can't be longer than min(cstring, sstring)
    ** pull tokens out of shortest string
    ** include space in combo_string for final separator and null terminator
    */
    if ( (clen = strlen( cext_string)) > (slen = strlen( sext_string)) ) {
        combo_string = (char *) Xmalloc( slen + 2 );
        s1 = (char *) Xmalloc( slen + 2 ); strcpy( s1, sext_string );
        s2 = cext_string;
    } else {
        combo_string = (char *) Xmalloc( clen + 2 );
        s1 = (char *) Xmalloc( clen + 2 ); strcpy( s1, cext_string);
        s2 = sext_string;
    }
    if (!combo_string || !s1) {
        if (combo_string) Xfree(combo_string);
        if (s1) Xfree(s1);
            return NULL;
    }
    combo_string[0] = '\0';

    /* Get first extension token */
    token = strtok( s1, SEPARATOR);
    while ( token != NULL ) {
        /*
        ** if token in second string then save it
        ** beware of extension names which are prefixes of other extension names
        */
        const char *p = s2;
        end = p + strlen(p);
        while (p < end) {
            int n = strcspn(p, SEPARATOR);
            if ((strlen(token) == n) && (strncmp(token, p, n) == 0)) {
                combo_string = strcat( combo_string, token);
                combo_string = strcat( combo_string, SEPARATOR);
            }
            p += (n + 1);
        }

        /* Get next extension token */
        token = strtok( NULL, SEPARATOR);
   }
   Xfree(s1);
   return combo_string;
}

const char *GLX_PREFIX(glXQueryExtensionsString)( Display *dpy, int screen )
{
    __GLcontextModes *pConfig;
    __GLXscreenConfigs *psc;
    __GLXdisplayPrivate *priv;

    /* Initialize the extension, if needed .  This has the added value
       of initializing/allocating the display private */
    priv = __glXInitialize(dpy);
    if (!priv) {
        return NULL;
    }

    /* Check screen number to see if its valid */
    if ((screen < 0) || (screen >= ScreenCount(dpy))) {
        return NULL;
    }

    /* Check to see if the GL is supported on this screen */
    psc = &priv->screenConfigs[screen];
    pConfig = psc->configs;
    if (!pConfig) {
        /* No support for GL on this screen regardless of visual */
        return NULL;
    }

    if (!psc->effectiveGLXexts) {
        if (!psc->serverGLXexts) {
            psc->serverGLXexts = QueryServerString(dpy, priv->majorOpcode,
                             screen, GLX_EXTENSIONS);
        }
        psc->effectiveGLXexts = combine_strings(__glXGLXClientExtensions,
                            psc->serverGLXexts);
    }

    return psc->effectiveGLXexts;
}

const char *GLX_PREFIX(glXGetClientString)( Display *dpy, int name )
{
    switch(name) {
    case GLX_VENDOR:
        return (__glXGLXClientVendorName);
    case GLX_VERSION:
        return (__glXGLXClientVersion);
    case GLX_EXTENSIONS:
        return (__glXGLXClientExtensions);
    default:
        return NULL;
    }
}

const char *GLX_PREFIX(glXQueryServerString)( Display *dpy, int screen, int name )
{
    __GLcontextModes *pConfig;
    __GLXscreenConfigs *psc;
    __GLXdisplayPrivate *priv;

    /* Initialize the extension, if needed .  This has the added value
       of initializing/allocating the display private */
    priv = __glXInitialize(dpy);
    if (!priv) {
        /* No extension */
        return NULL;
    }

    /* Check screen number to see if its valid */
    if ((screen < 0) || (screen >= ScreenCount(dpy))) {
        return NULL;
    }

    /* Check to see if the GL is supported on this screen */
    psc = &priv->screenConfigs[screen];
    pConfig = psc->configs;
    if (!pConfig) {
        /* No support for GL on this screen regardless of visual */
        return NULL;
    }

    switch(name) {
    case GLX_VENDOR:
        if (!priv->serverGLXvendor) {
            priv->serverGLXvendor =
                QueryServerString(dpy, priv->majorOpcode,
                                    screen, GLX_VENDOR);
        }
        return(priv->serverGLXvendor);
    case GLX_VERSION:
        if (!priv->serverGLXversion) {
            priv->serverGLXversion =
                QueryServerString(dpy, priv->majorOpcode,
                                    screen, GLX_VERSION);
        }
        return(priv->serverGLXversion);
    case GLX_EXTENSIONS:
        if (!psc->serverGLXexts) {
            psc->serverGLXexts =
                QueryServerString(dpy, priv->majorOpcode,
                                    screen, GLX_EXTENSIONS);
        }
        return(psc->serverGLXexts);
    default:
        return NULL;
    }
}

GLvoid __glXClientInfo (  Display *dpy, int opcode  )
{
    xGLXClientInfoReq *req;
    int size;

    /* Send the glXClientInfo request */
    LockDisplay(dpy);
    GetReq(GLXClientInfo,req);
    req->reqType = opcode;
    req->glxCode = X_GLXClientInfo;
    req->major = GLX_MAJOR_VERSION;
    req->minor = GLX_MINOR_VERSION;

    size = strlen(__glXGLClientExtensions) + 1;
    req->length += (size + 3) >> 2;
    req->numbytes = size;
    Data(dpy, __glXGLClientExtensions, size);

    UnlockDisplay(dpy);
    SyncHandle();
}


Display *GLX_PREFIX(glXGetCurrentDisplay)(GLvoid)
{
    GLXContext gc = __glXGetCurrentContext();
    if (NULL == gc) return NULL;
    return gc->currentDpy;
}


/************************************************************************/

static GLvoid
init_fbconfig_for_chooser( __GLcontextModes * config,
                           GLboolean fbconfig_style_tags )
{
    memset( config, 0, sizeof( __GLcontextModes ) );
    config->visualID = (XID) GLX_DONT_CARE;
    config->visualType = GLX_DONT_CARE;

    /* glXChooseFBConfig specifies different defaults for these two than
     * glXChooseVisual.
     */
    if ( fbconfig_style_tags ) {
        config->rgbMode = GL_TRUE;
        config->doubleBufferMode = GLX_DONT_CARE;
    }

    config->visualRating = GLX_DONT_CARE;
    config->transparentPixel = GLX_NONE;
    config->transparentRed = GLX_DONT_CARE;
    config->transparentGreen = GLX_DONT_CARE;
    config->transparentBlue = GLX_DONT_CARE;
    config->transparentAlpha = GLX_DONT_CARE;
    config->transparentIndex = GLX_DONT_CARE;

    config->drawableType = GLX_WINDOW_BIT;
    config->renderType = (config->rgbMode) ? GLX_RGBA_BIT : GLX_COLOR_INDEX_BIT;
    config->xRenderable = GLX_DONT_CARE;
    config->fbconfigID = (GLXFBConfigID)(GLX_DONT_CARE);
}

#define MATCH_DONT_CARE( param ) \
        do { \
            if ( (a-> param != GLX_DONT_CARE) \
                 && (a-> param != b-> param) ) { \
                return False; \
            } \
        } while ( 0 )

#define MATCH_MINIMUM( param ) \
        do { \
            if ( (a-> param != GLX_DONT_CARE) \
                 && (a-> param > b-> param) ) { \
                return False; \
            } \
        } while ( 0 )

#define MATCH_EXACT( param ) \
        do { \
            if ( a-> param != b-> param) { \
                return False; \
            } \
        } while ( 0 )

/**
 * Determine if two GLXFBConfigs are compatible.
 *
 * \param a  Application specified config to test.
 * \param b  Server specified config to test against \c a.
 */
static Bool
fbconfigs_compatible( const __GLcontextModes * const a,
                      const __GLcontextModes * const b )
{
    MATCH_DONT_CARE( doubleBufferMode );
    MATCH_DONT_CARE( visualType );
    MATCH_DONT_CARE( visualRating );
    MATCH_DONT_CARE( xRenderable );
    MATCH_DONT_CARE( fbconfigID );

    MATCH_MINIMUM( rgbaBits );
    MATCH_MINIMUM( numAuxBuffers );
    MATCH_MINIMUM( redBits );
    MATCH_MINIMUM( greenBits );
    MATCH_MINIMUM( blueBits );
    if ( b->rgbaBits != 16 || b->alphaBits != 0 )
    {
    MATCH_MINIMUM( alphaBits );
    }
    MATCH_MINIMUM( depthBits );
    MATCH_MINIMUM( stencilBits );
    MATCH_MINIMUM( accumRedBits );
    MATCH_MINIMUM( accumGreenBits );
    MATCH_MINIMUM( accumBlueBits );
    MATCH_MINIMUM( accumAlphaBits );
    MATCH_MINIMUM( sampleBuffers );
    MATCH_MINIMUM( maxPbufferWidth );
    MATCH_MINIMUM( maxPbufferHeight );
    MATCH_MINIMUM( maxPbufferPixels );
    MATCH_MINIMUM( samples );

    MATCH_DONT_CARE( stereoMode );
    MATCH_EXACT( level );

    if ( ((a->drawableType & b->drawableType) == 0)
         || ((a->renderType & b->renderType) == 0) ) {
        return False;
    }


    if ( a->transparentPixel != GLX_DONT_CARE
         && a->transparentPixel != 0 ) {
        if ( a->transparentPixel == GLX_NONE ) {
            if ( b->transparentPixel != GLX_NONE && b->transparentPixel != 0 )
                return False;
        } else {
            MATCH_EXACT( transparentPixel );
        }

        switch ( a->transparentPixel ) {
          case GLX_TRANSPARENT_RGB:
            MATCH_DONT_CARE( transparentRed );
            MATCH_DONT_CARE( transparentGreen );
            MATCH_DONT_CARE( transparentBlue );
            MATCH_DONT_CARE( transparentAlpha );
            break;

          case GLX_TRANSPARENT_INDEX:
            MATCH_DONT_CARE( transparentIndex );
            break;

          default:
            break;
        }
    }

    return True;
}


/* There's some trickly language in the GLX spec about how this is supposed
 * to work.  Basically, if a given component size is either not specified
 * or the requested size is zero, it is supposed to act like PERFER_SMALLER.
 * Well, that's really hard to do with the code as-is.  This behavior is
 * closer to correct, but still not technically right.
 */
#define PREFER_LARGER_OR_ZERO(comp) \
    do { \
        if ( ((*a)-> comp) != ((*b)-> comp) ) { \
            if ( ((*a)-> comp) == 0 ) { \
                return -1; \
            } \
            else if ( ((*b)-> comp) == 0 ) { \
                return 1; \
            } \
            else { \
                return ((*b)-> comp) - ((*a)-> comp) ; \
            } \
        } \
    } while( 0 )

#define PREFER_LARGER(comp) \
    do { \
        if ( ((*a)-> comp) != ((*b)-> comp) ) { \
            return ((*b)-> comp) - ((*a)-> comp) ; \
        } \
    } while( 0 )

#define PREFER_SMALLER(comp) \
    do { \
        if ( ((*a)-> comp) != ((*b)-> comp) ) { \
            return ((*a)-> comp) - ((*b)-> comp) ; \
        } \
    } while( 0 )

/**
 * Compare two GLXFBConfigs.  This function is intended to be used as the
 * compare function passed in to qsort.
 *
 * \returns If \c a is a "better" config, according to the specification of
 *          SGIX_fbconfig, a number less than zero is returned.  If \c b is
 *          better, then a number greater than zero is return.  If both are
 *          equal, zero is returned.
 * \sa qsort, glXChooseVisual, glXChooseFBConfig, glXChooseFBConfigSGIX
 */
static int
fbconfig_compare( const __GLcontextModes * const * const a,
                  const __GLcontextModes * const * const b )
{
    /* The order of these comparisons must NOT change.  It is defined by
     * the GLX 1.3 spec and ARB_multisample.
     */

    /* The sort order for the visualRating is GLX_NONE, GLX_SLOW, and
     * GLX_NON_CONFORMANT_CONFIG.  It just so happens that this is the
     * numerical sort order of the enums (0x8000, 0x8001, and 0x800D).
     */
    PREFER_SMALLER( visualRating );

    /* This isn't quite right.  It is supposed to compare the sum of the
     * components the user specifically set minimums for.
     */
    PREFER_LARGER_OR_ZERO( redBits );
    PREFER_LARGER_OR_ZERO( greenBits );
    PREFER_LARGER_OR_ZERO( blueBits );
    PREFER_LARGER_OR_ZERO( alphaBits );

    PREFER_SMALLER( rgbaBits );

    if ( ((*a)->doubleBufferMode != (*b)->doubleBufferMode) ) {
        /* Prefer single-buffer.
         */
        return ( !(*a)->doubleBufferMode ) ? -1 : 1;
    }

    PREFER_SMALLER( numAuxBuffers );

    PREFER_LARGER_OR_ZERO( depthBits );
    PREFER_SMALLER( stencilBits );

    /* This isn't quite right.  It is supposed to compare the sum of the
     * components the user specifically set minimums for.
     */
    PREFER_LARGER_OR_ZERO( accumRedBits );
    PREFER_LARGER_OR_ZERO( accumGreenBits );
    PREFER_LARGER_OR_ZERO( accumBlueBits );
    PREFER_LARGER_OR_ZERO( accumAlphaBits );

    PREFER_SMALLER( visualType );

    /* None of the multisample specs say where this comparison should happen,
     * so I put it near the end.
     */
    PREFER_SMALLER( sampleBuffers );
    PREFER_SMALLER( samples );

    /* None of the pbuffer or fbconfig specs say that this comparison needs
     * to happen at all, but it seems like it should.
     */
    PREFER_LARGER( maxPbufferWidth );
    PREFER_LARGER( maxPbufferHeight );
    PREFER_LARGER( maxPbufferPixels );

    return 0;
}


/**
 * Selects and sorts a subset of the supplied configs based on the attributes.
 * This function forms to basis of \c glXChooseVisual, \c glXChooseFBConfig,
 * and \c glXChooseFBConfigSGIX.
 *
 * \param configs   Array of pointers to possible configs.  The elements of
 *                  this array that do not meet the criteria will be set to
 *                  NULL.  The remaining elements will be sorted according to
 *                  the various visual / FBConfig selection rules.
 * \param num_configs  Number of elements in the \c configs array.
 * \param attribList   Attributes used select from \c configs.  This array is
 *                     terminated by a \c None tag.  The array can either take
 *                     the form expected by \c glXChooseVisual (where boolean
 *                     tags do not have a value) or by \c glXChooseFBConfig
 *                     (where every tag has a value).
 * \param fbconfig_style_tags  Selects whether \c attribList is in
 *                             \c glXChooseVisual style or
 *                             \c glXChooseFBConfig style.
 * \returns The number of valid elements left in \c configs.
 *
 * \sa glXChooseVisual, glXChooseFBConfig, glXChooseFBConfigSGIX
 */
static int
ChooseFBConfig( __GLcontextModes ** configs, int num_configs,
               const int *attribList, GLboolean fbconfig_style_tags )
{
    __GLcontextModes    test_config;
    int   base;
    int   i;

    /* This is a fairly direct implementation of the selection method
     * described by GLX_SGIX_fbconfig.  Start by culling out all the
     * configs that are not compatible with the selected parameter
     * list.
     */

    if (NULL==attribList) {
        base = num_configs;
    } else if (None==*attribList)
    {
        base = num_configs;
    } else {

        init_fbconfig_for_chooser( & test_config, fbconfig_style_tags );
        __glXInitializeVisualConfigFromTags( & test_config, 512,
                                         (const INT32 *) attribList,
                                         GL_TRUE, fbconfig_style_tags );

        base = 0;
        for ( i = 0 ; i < num_configs ; i++ ) {
            if ( fbconfigs_compatible( & test_config, configs[i] ) ) {
                configs[ base ] = configs[ i ];
                base++;
            }
        }

    }

    if ( base == 0 ) {
        return 0;
    }

    if ( base < num_configs ) {
        (GLvoid) memset( & configs[ base ], 0,
                       sizeof( GLvoid * ) * (num_configs - base) );
    }

    /* After the incompatible configs are removed, the resulting
     * list is sorted according to the rules set out in the various
     * specifications.
     */

    qsort( configs, base, sizeof( __GLcontextModes * ),
           (int (*)(const GLvoid*, const GLvoid*)) fbconfig_compare );
    return base;
}

/************************************************************************/

/*
 * GLX 1.3 functions
 */

GLXFBConfig *GLX_PREFIX(glXChooseFBConfig)(Display *dpy, int screen, const int *attribList, int *nitems)
{
    __GLcontextModes **config_list;
    int list_size;

    config_list = (__GLcontextModes **)
    GLX_PREFIX(glXGetFBConfigs)( dpy, screen, & list_size );

    if ( (config_list != NULL) && (list_size > 0) ) {
        list_size = ChooseFBConfig( config_list, list_size, attribList, GL_TRUE );
        if ( list_size == 0 ) {
            XFree( config_list );
            config_list = NULL;
        }
    }

    *nitems = list_size;
    return (GLXFBConfig *) config_list;
}


GLXContext GLX_PREFIX(glXCreateNewContext)(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct)
{
    return CreateContext( dpy, NULL, (__GLcontextModes *)config, shareList,
                        direct, renderType );
}


GLXDrawable glXGetCurrentReadDrawable(GLvoid)
{
    GLXContext gc = __glXGetCurrentContext();
    return gc->currentDrawable;
}

/*
** GLX_SGI_make_current_read
*/
GLX_ALIAS(GLXDrawable, glXGetCurrentReadDrawableSGI, (GLvoid), (), glXGetCurrentReadDrawable)


GLXFBConfig *GLX_PREFIX(glXGetFBConfigs)(Display *dpy, int screen, int *nelements)
{
    __GLXdisplayPrivate *priv = __glXInitialize(dpy);
    __GLcontextModes **config = NULL;
    int   i;

    if ( (priv->screenConfigs != NULL)
     && (screen >= 0) && (screen <= ScreenCount(dpy))
     && (priv->screenConfigs[screen].configs != NULL)
     && (priv->screenConfigs[screen].configs->fbconfigID != GLX_DONT_CARE) ) {
        unsigned num_configs = 0;
        __GLcontextModes * modes;

        for ( modes = priv->screenConfigs[screen].configs
              ; modes != NULL
              ; modes = modes->next ) {
            if ( modes->fbconfigID != GLX_DONT_CARE ) {
                num_configs++;
            }
        }

        config = (__GLcontextModes **) Xmalloc( sizeof(__GLcontextModes *) * num_configs );
        if ( config != NULL ) {
            *nelements = num_configs;
            i = 0;
            for ( modes = priv->screenConfigs[screen].configs
              ; modes != NULL
              ; modes = modes->next ) {
                config[i] = modes;
                i++;
            }
        }
    }

    return (GLXFBConfig *) config;
}


int GLX_PREFIX(glXGetFBConfigAttrib)(Display *dpy, GLXFBConfig config, int attribute, int *value_return)
{
    __GLcontextModes * const mode = ValidateGLXFBConfig( dpy, config );

    if (mode == NULL)
        return GLXBadFBConfig;

    switch (attribute) {
      case GLX_USE_GL:
            *value_return = GL_TRUE;
            return 0;
      case GLX_BUFFER_SIZE:
            *value_return = mode->rgbaBits;
            return 0;
      case GLX_RGBA:
            *value_return = mode->rgbMode;
            return 0;
      case GLX_RED_SIZE:
            *value_return = mode->redBits;
            return 0;
      case GLX_GREEN_SIZE:
            *value_return = mode->greenBits;
            return 0;
      case GLX_BLUE_SIZE:
            *value_return = mode->blueBits;
            return 0;
      case GLX_ALPHA_SIZE:
            *value_return = mode->alphaBits;
            return 0;
      case GLX_DOUBLEBUFFER:
            *value_return = mode->doubleBufferMode;
            return 0;
      case GLX_STEREO:
            *value_return = mode->stereoMode;
            return 0;
      case GLX_AUX_BUFFERS:
            *value_return = mode->numAuxBuffers;
            return 0;
      case GLX_DEPTH_SIZE:
            *value_return = mode->depthBits;
            return 0;
      case GLX_STENCIL_SIZE:
            *value_return = mode->stencilBits;
            return 0;
      case GLX_ACCUM_RED_SIZE:
            *value_return = mode->accumRedBits;
            return 0;
      case GLX_ACCUM_GREEN_SIZE:
            *value_return = mode->accumGreenBits;
            return 0;
      case GLX_ACCUM_BLUE_SIZE:
            *value_return = mode->accumBlueBits;
            return 0;
      case GLX_ACCUM_ALPHA_SIZE:
            *value_return = mode->accumAlphaBits;
            return 0;
      case GLX_LEVEL:
            *value_return = mode->level;
            return 0;
      case GLX_TRANSPARENT_TYPE_EXT:
            *value_return = mode->transparentPixel;
            return 0;
      case GLX_TRANSPARENT_RED_VALUE:
            *value_return = mode->transparentRed;
            return 0;
      case GLX_TRANSPARENT_GREEN_VALUE:
            *value_return = mode->transparentGreen;
            return 0;
      case GLX_TRANSPARENT_BLUE_VALUE:
            *value_return = mode->transparentBlue;
            return 0;
      case GLX_TRANSPARENT_ALPHA_VALUE:
            *value_return = mode->transparentAlpha;
            return 0;
      case GLX_TRANSPARENT_INDEX_VALUE:
            *value_return = mode->transparentIndex;
            return 0;
      case GLX_X_VISUAL_TYPE:
            *value_return = mode->visualType;
            return 0;
      case GLX_CONFIG_CAVEAT:
            *value_return = mode->visualRating;
            return 0;
      case GLX_VISUAL_ID:
            *value_return = mode->visualID;
            return 0;
      case GLX_DRAWABLE_TYPE:
            *value_return = mode->drawableType;
            return 0;
      case GLX_RENDER_TYPE:
            *value_return = mode->renderType;
            return 0;
      case GLX_X_RENDERABLE:
            *value_return = mode->xRenderable;
            return 0;
      case GLX_FBCONFIG_ID:
            *value_return = mode->fbconfigID;
            return 0;
      case GLX_MAX_PBUFFER_WIDTH:
            *value_return = mode->maxPbufferWidth;
            return 0;
      case GLX_MAX_PBUFFER_HEIGHT:
            *value_return = mode->maxPbufferHeight;
            return 0;
      case GLX_MAX_PBUFFER_PIXELS:
            *value_return = mode->maxPbufferPixels;
            return 0;
      case GLX_SAMPLE_BUFFERS_SGIS:
            *value_return = mode->sampleBuffers;
            return 0;
      case GLX_SAMPLES_SGIS:
            *value_return = mode->samples;
            return 0;

      default:
            return GLX_BAD_ATTRIBUTE;
    }
}

XVisualInfo *GLX_PREFIX(glXGetVisualFromFBConfig)(Display *dpy, GLXFBConfig config)
{
    XVisualInfo visualTemplate;
    __GLcontextModes * fbconfig = (__GLcontextModes *) config;
    int  count;

    /*
    ** Get a list of all visuals, return if list is empty
    */
    visualTemplate.visualid = fbconfig->visualID;
    return XGetVisualInfo(dpy, VisualIDMask, &visualTemplate, &count);
}


/**
 * \param dpy  Display where \c ctx was created.
 * \param ctx  Context to query.
 * \returns  \c Success on success.  \c GLX_BAD_CONTEXT if \c ctx is invalid,
 *           or zero if the request failed due to internal problems (i.e.,
 *           unable to allocate temporary memory, etc.)
 */
static int __glXQueryContextInfo(Display *dpy, GLXContext ctx)
{
    xGLXQueryContextReq * req;
    xGLXQueryContextReply reply;
    CARD8 opcode;
    GLuint numValues;
    int retval;

    if (ctx == NULL) {
        return GLX_BAD_CONTEXT;
    }
    opcode = __glXSetupForCommand(dpy);
    if (!opcode) {
        return 0;
    }

    /* Send the glXQueryContextInfoEXT request */
    LockDisplay(dpy);
    GetReq(GLXQueryContext, req);
    req->reqType = opcode;
    req->glxCode = X_GLXQueryContext;
    req->context = (unsigned int)(ctx->xid);
    _XReply(dpy, (xReply*) &reply, 0, False);

    numValues = reply.n;
    if (numValues == 0)
        retval = Success;
    else if (numValues > __GLX_MAX_CONTEXT_PROPS)
        retval = 0;
    else
    {
        int *propList, *pProp;
        int nPropListBytes;
        int i;

        nPropListBytes = numValues << 3;
        propList = (int *) Xmalloc(nPropListBytes);
        if (NULL == propList) {
            retval = 0;
        } else {
            _XRead(dpy, (char *)propList, nPropListBytes);
            pProp = propList;
            for (i=0; i < numValues; i++) {
                switch (*pProp++) {
                case GLX_SHARE_CONTEXT_EXT:
                    ctx->share_xid = *pProp++;
                    break;
                case GLX_VISUAL_ID_EXT:
                    ctx->vid = *pProp++;
                    break;
                case GLX_SCREEN:
                    ctx->screen = *pProp++;
                    break;
                case GLX_FBCONFIG_ID:
                    ctx->fbconfigID = *pProp++;
                    break;
                case GLX_RENDER_TYPE:
                    ctx->renderType = *pProp++;
                    break;
                default:
                    pProp++;
                    continue;
                }
            }
            Xfree((char *)propList);
            retval = Success;
        }
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return retval;
}

int GLX_PREFIX(glXQueryContext)(Display *dpy, GLXContext ctx, int attribute, int *value)
{
    int retVal;

    /* get the information from the server if we don't have it already */
    if (!ctx->isDirect && (ctx->vid == None)) {
        retVal = __glXQueryContextInfo(dpy, ctx);
        if (Success != retVal) return retVal;
    }
    switch (attribute) {
    case GLX_SHARE_CONTEXT_EXT:
        *value = (int)(ctx->share_xid);
        break;
    case GLX_VISUAL_ID_EXT:
        *value = (int)(ctx->vid);
        break;
    case GLX_SCREEN:
        *value = (int)(ctx->screen);
        break;
    case GLX_FBCONFIG_ID:
        *value = (int)(ctx->fbconfigID);
        break;
    case GLX_RENDER_TYPE:
        *value = (int)(ctx->renderType);
        break;
    default:
        return GLX_BAD_ATTRIBUTE;
    }
    return Success;
}


/* GLX 1.4 function */
GLvoid (*glXGetProcAddress(const GLubyte *procName))( GLvoid )
{
    return glXGetProcAddressARB(procName);
}


/*
** GLX_SGIX_fbconfig
** Many of these functions are aliased to GLX 1.3 entry points in the
** GLX_functions table.
*/
GLX_ALIAS(int, glXGetFBConfigAttribSGIX,
          (Display *dpy, GLXFBConfigSGIX config, int attribute, int *value),
          (dpy, config, attribute, value),
          glXGetFBConfigAttrib)

GLX_ALIAS(GLXFBConfigSGIX *, glXChooseFBConfigSGIX,
          (Display *dpy, int screen, int *attrib_list, int *nelements),
          (dpy, screen, attrib_list, nelements),
          glXChooseFBConfig)

GLX_ALIAS(XVisualInfo *, glXGetVisualFromFBConfigSGIX,
          (Display * dpy, GLXFBConfigSGIX config),
          (dpy, config),
          glXGetVisualFromFBConfig)

GLXPixmap GLX_PREFIX(glXCreateGLXPixmapWithConfigSGIX)(Display *dpy, GLXFBConfigSGIX config, Pixmap pixmap)
{
    xGLXVendorPrivateWithReplyReq *vpreq;
    xGLXCreateGLXPixmapWithConfigSGIXReq *req;
    GLXPixmap xid = None;
    CARD8 opcode;
    const __GLcontextModes * const fbconfig = (__GLcontextModes *) config;
    __GLXscreenConfigs * psc;

    if ( (dpy == NULL) || (config == NULL) ) {
            return None;
    }

    psc = GetGLXScreenConfigs( dpy, fbconfig->screen );
    if ( (psc != NULL) /* && __glXExtensionBitIsEnabled( psc, SGIX_fbconfig_bit ) */) {
        opcode = __glXSetupForCommand(dpy);
        if (!opcode) {
            return None;
        }

        /* Send the glXCreateGLXPixmapWithConfigSGIX request */
        LockDisplay(dpy);
        GetReqExtra(GLXVendorPrivateWithReply,
                    sz_xGLXCreateGLXPixmapWithConfigSGIXReq-sz_xGLXVendorPrivateWithReplyReq,vpreq);
        req = (xGLXCreateGLXPixmapWithConfigSGIXReq *)vpreq;
        req->reqType = opcode;
        req->glxCode = X_GLXVendorPrivateWithReply;
        req->vendorCode = X_GLXvop_CreateGLXPixmapWithConfigSGIX;
        req->screen = fbconfig->screen;
        req->fbconfig = fbconfig->fbconfigID;
        req->pixmap = pixmap;
        req->glxpixmap = xid = XAllocID(dpy);
        UnlockDisplay(dpy);
        SyncHandle();
    }

    return xid;
}

GLXContext GLX_PREFIX(glXCreateContextWithConfigSGIX)(Display *dpy, GLXFBConfigSGIX config, int renderType, GLXContext shareList, Bool allowDirect)
{
    GLXContext gc = NULL;
    const __GLcontextModes * const fbconfig = (__GLcontextModes *) config;
    __GLXscreenConfigs * psc;


    if ( (dpy == NULL) || (config == NULL) ) {
        return None;
    }

    psc = GetGLXScreenConfigs( dpy, fbconfig->screen );
    if ( (psc != NULL)
         /* && __glXExtensionBitIsEnabled( psc, SGIX_fbconfig_bit ) */ ) {
        gc = CreateContext( dpy, NULL, (__GLcontextModes *) config, shareList,
                            allowDirect, renderType );
    }

    return gc;
}


GLXFBConfigSGIX GLX_PREFIX(glXGetFBConfigFromVisualSGIX)(Display *dpy, XVisualInfo *vis)
{
    __GLXdisplayPrivate *priv;
    __GLXscreenConfigs *psc = NULL;
    __GLcontextModes *mode;

    if ( (GetGLXPrivScreenConfig( dpy, vis->screen, & priv, & psc ) != Success)
         /* && __glXExtensionBitIsEnabled( psc, SGIX_fbconfig_bit ) */
         && (psc->configs->fbconfigID != GLX_DONT_CARE) ) {

        mode = psc->configs;
        while ( mode != NULL ) {
            if ( mode->visualID == vis->visualid ) {
                break;
            }
            mode = mode->next;
        }

        return (GLXFBConfigSGIX) mode;
    }

    return NULL;
}

/*
** GLX_SGIX_video_resize
*/
int GLX_PREFIX(glXBindChannelToWindowSGIX)(Display *dpy, int screen, int channel , Window window)
{
   (GLvoid) dpy;
   (GLvoid) screen;
   (GLvoid) channel;
   (GLvoid) window;
   return 0;
}

int GLX_PREFIX(glXChannelRectSGIX)(Display *dpy, int screen, int channel, int x, int y, int w, int h)
{
   (GLvoid) dpy;
   (GLvoid) screen;
   (GLvoid) channel;
   (GLvoid) x;
   (GLvoid) y;
   (GLvoid) w;
   (GLvoid) h;
   return 0;
}

int GLX_PREFIX(glXQueryChannelRectSGIX)(Display *dpy, int screen, int channel, int *x, int *y, int *w, int *h)
{
   (GLvoid) dpy;
   (GLvoid) screen;
   (GLvoid) channel;
   (GLvoid) x;
   (GLvoid) y;
   (GLvoid) w;
   (GLvoid) h;
   return 0;
}

int GLX_PREFIX(glXQueryChannelDeltasSGIX)(Display *dpy, int screen, int channel, int *dx, int *dy, int *dw, int *dh)
{
   (GLvoid) dpy;
   (GLvoid) screen;
   (GLvoid) channel;
   (GLvoid) dx;
   (GLvoid) dy;
   (GLvoid) dw;
   (GLvoid) dh;
   return 0;
}

int GLX_PREFIX(glXChannelRectSyncSGIX)(Display *dpy, int screen, int channel, GLenum synctype)
{
   (GLvoid) dpy;
   (GLvoid) screen;
   (GLvoid) channel;
   (GLvoid) synctype;
   return 0;
}

/* strdup() is actually not a standard ANSI C or POSIX routine.
 * Irix will not define it if ANSI mode is in effect.
 */
char *
__glXstrdup(const char *str)
{
   char *copy;
   copy = (char *) Xmalloc(strlen(str) + 1);
   if (!copy)
      return NULL;
   strcpy(copy, str);
   return copy;
}

/*
** glXGetProcAddress support
*/

struct name_address_pair {
   const char *Name;
   GLvoid *Address;
};

#define GLX_FUNCTION(f) { # f, (GLvoid *) f }
#define GLX_FUNCTION2(n,f) { # n, (GLvoid *) f }

static struct name_address_pair GLX_functions[] = {
    /*** GLX_VERSION_1_0 ***/
    GLX_FUNCTION( glXChooseVisual ),
    GLX_FUNCTION( glXCopyContext ),
    GLX_FUNCTION( glXCreateContext ),
    GLX_FUNCTION( glXCreateGLXPixmap ),
    GLX_FUNCTION( glXDestroyContext ),
    GLX_FUNCTION( glXDestroyGLXPixmap ),
    GLX_FUNCTION( glXGetConfig ),
    GLX_FUNCTION( glXGetCurrentContext ),
    GLX_FUNCTION( glXGetCurrentDrawable ),
    GLX_FUNCTION( glXIsDirect ),
    GLX_FUNCTION( glXMakeCurrent ),
    GLX_FUNCTION( glXQueryExtension ),
    GLX_FUNCTION( glXQueryVersion ),
    GLX_FUNCTION( glXSwapBuffers ),
    GLX_FUNCTION( glXUseXFont ),
    GLX_FUNCTION( glXWaitGL ),
    GLX_FUNCTION( glXWaitX ),

    /*** GLX_VERSION_1_1 ***/
    GLX_FUNCTION( glXGetClientString ),
    GLX_FUNCTION( glXQueryExtensionsString ),
    GLX_FUNCTION( glXQueryServerString ),

    /*** GLX_VERSION_1_2 ***/
    GLX_FUNCTION( glXGetCurrentDisplay ),

    /*** GLX_VERSION_1_3 ***/
    GLX_FUNCTION( glXChooseFBConfig ),
    GLX_FUNCTION( glXCreateNewContext ),
    GLX_FUNCTION( glXCreatePbuffer ),
    GLX_FUNCTION( glXCreatePixmap ),
    GLX_FUNCTION( glXCreateWindow ),
    GLX_FUNCTION( glXDestroyPbuffer ),
    GLX_FUNCTION( glXDestroyPixmap ),
    GLX_FUNCTION( glXDestroyWindow ),
    GLX_FUNCTION( glXGetCurrentReadDrawable ),
    GLX_FUNCTION( glXGetFBConfigAttrib ),
    GLX_FUNCTION( glXGetFBConfigs ),
    GLX_FUNCTION( glXGetSelectedEvent ),
    GLX_FUNCTION( glXGetVisualFromFBConfig ),
    GLX_FUNCTION( glXMakeContextCurrent ),
    GLX_FUNCTION( glXQueryContext ),
    GLX_FUNCTION( glXQueryDrawable ),
    GLX_FUNCTION( glXSelectEvent ),

    /*** GLX 1.4 ***/
    GLX_FUNCTION2( glXGetProcAddress, glXGetProcAddressARB ),

    /*** GLX_SGI_make_current_read ***/
    GLX_FUNCTION2( glXMakeCurrentReadSGI, glXMakeContextCurrent ),
    GLX_FUNCTION2( glXGetCurrentReadDrawableSGI, glXGetCurrentReadDrawable ),

    /*** GLX_SGIX_fbconfig ***/
    GLX_FUNCTION2( glXGetFBConfigAttribSGIX, glXGetFBConfigAttrib ),
    GLX_FUNCTION2( glXChooseFBConfigSGIX, glXChooseFBConfig ),
    GLX_FUNCTION( glXCreateGLXPixmapWithConfigSGIX ),
    GLX_FUNCTION( glXCreateContextWithConfigSGIX ),
    GLX_FUNCTION2( glXGetVisualFromFBConfigSGIX, glXGetVisualFromFBConfig ),
    GLX_FUNCTION( glXGetFBConfigFromVisualSGIX ),

    /*** GLX_SGIX_pbuffer ***/
    GLX_FUNCTION( glXCreateGLXPbufferSGIX ),
    GLX_FUNCTION( glXDestroyGLXPbufferSGIX ),
    GLX_FUNCTION( glXQueryGLXPbufferSGIX ),
    GLX_FUNCTION( glXSelectEventSGIX ),
    GLX_FUNCTION( glXGetSelectedEventSGIX ),

    /*** GLX_ARB_get_proc_address ***/
    GLX_FUNCTION( glXGetProcAddressARB ),

    /*** GLX_SGIX_video_resize ***/
    GLX_FUNCTION( glXBindChannelToWindowSGIX ),
    GLX_FUNCTION( glXChannelRectSGIX ),
    GLX_FUNCTION( glXQueryChannelRectSGIX ),
    GLX_FUNCTION( glXQueryChannelDeltasSGIX ),
    GLX_FUNCTION( glXChannelRectSyncSGIX ),

    { NULL, NULL }   /* end of list */
};


#if defined(PTHREADS)
#  define X86_DISPATCH_FUNCTION_SIZE  32
# else
#  define X86_DISPATCH_FUNCTION_SIZE  16
# endif

static __GLextFuncAlias __glExtFuncAlias[] =
{
    __GL_EXT_ALIAS_TABLE
};

static const char *__glApiNameString[] = {
    __GL_API_ENTRIES(glStr)
};

static GLint __glGetDispatchOffset(const char *procName)
{
    __GLextFuncAlias *extAlias;
    const char *apiName;
    GLint offset, index;

    /* Skip the first two characters "gl" of procName */
    apiName = procName + 2;

    /* Find extension function alias name in __glExtFuncAlias[] table */
    for (extAlias = __glExtFuncAlias; extAlias->index < INDEX_EXT_LAST; extAlias++) {
        if (strcmp((const char *)extAlias->procName, (const char *)apiName) == 0) {
            apiName = extAlias->aliasName;
        }
    }

    /* Find extension function's offset in __glApiNameString[] table */
    offset = -1;
    for (index = 0; index < _gloffset_LAST; index++) {
        if (strcmp((const char *)__glApiNameString[index], (const char *)apiName) == 0) {
            offset = index;
            break;
        }
    }

    return offset;
}

extern __GLdispatchTable glVVT_DispatchTable;
/**
 * Get the address of a named GL function.  This is the pre-GLX 1.4 name for
 * \c glXGetProcAddress.
 *
 * \param procName  Name of a GL or GLX function.
 * \returns         A pointer to the named function
 *
 * \sa glXGetProcAddress
 */
GLvoid (*glXGetProcAddressARB(const GLubyte *procName))( GLvoid )
{
#ifdef USE_X86_ASM
    extern const GLubyte gl_dispatch_functions_start[];
#endif
    typedef GLvoid (*gl_function)( GLvoid );
    gl_function f = NULL;
    GLuint i;

    /* Search the table of GLX and internal functions first.  If that
    * fails and the supplied name could be a valid core GL name, try
    * searching the core GL function table.  This check is done to prevent
    * DRI based drivers from searching the core GL function table for
    * internal API functions.
    */
    for (i = 0; GLX_functions[i].Name; i++) {
        if (strcmp((const char *)GLX_functions[i].Name, (const char *)procName) == 0) {
            f = (gl_function)GLX_functions[i].Address;
        }
    }

    /* Search GL API function table to find the function with "procName".
    */
    if ((f == NULL) && (procName[0] == 'g') && (procName[1] == 'l') && (procName[2] != 'X'))
    {
        GLint offset = __glGetDispatchOffset((const char *) procName);
        if (offset >= 0) {
#ifdef USE_X86_ASM
            f = (gl_function)(gl_dispatch_functions_start
                                    + (X86_DISPATCH_FUNCTION_SIZE * offset));
#else
            f = (gl_function)*((((unsigned long*)&glVVT_DispatchTable)+offset));

#endif
        }
        else {
            f = NULL;
        }
    }

    return f;
}
