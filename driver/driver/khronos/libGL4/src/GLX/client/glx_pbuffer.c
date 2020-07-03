/* $XFree86: xc/lib/GL/glx/glx_pbuffer.c,v 1.3 2004/12/17 16:38:03 tsi Exp $ */
/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glx_pbuffer.c
 * Implementation of pbuffer related functions.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "glxclient.h"
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <string.h>
#include <stdio.h>
#include "gl_thread.h"



static GLvoid ChangeDrawableAttribute( Display * dpy, GLXDrawable drawable,
    const CARD32 * attribs, size_t num_attribs );

static GLvoid DestroyPbuffer( Display * dpy, GLXDrawable drawable );

static GLXDrawable CreatePbuffer( Display *dpy,
    const __GLcontextModes * fbconfig, unsigned int width, unsigned int height,
    const int *attrib_list, GLboolean size_in_attribs );

static int GetDrawableAttribute( Display *dpy, GLXDrawable drawable,
    int attribute, unsigned int *value );


typedef struct _pixmappool {
GLXDrawable drawable;
void *pnext;
}PixPool, *PPixPool;

static PPixPool _glxpixpool = NULL;

static _glthread_Mutex __glPixMutex = PTHREAD_MUTEX_INITIALIZER;

static void __putPixIntoPool(GLXDrawable drawable)
{
    PPixPool pel;
    PPixPool pelx;
    pel = (PPixPool)malloc(sizeof(PixPool));
    pel->pnext = NULL;
    pel->drawable = drawable;
    _glthread_LOCK_MUTEX(__glPixMutex);
    if ( _glxpixpool == NULL )
    {
        _glxpixpool = pel;
        _glthread_UNLOCK_MUTEX(__glPixMutex);
        return;
    }
    pelx = _glxpixpool;
    while( pelx->pnext != NULL )
        pelx = (PPixPool)pelx->pnext;
    pelx->pnext = (void *)pel;
    _glthread_UNLOCK_MUTEX(__glPixMutex);
}

static void __delPixFromPool(GLXDrawable drawable)
{
    PPixPool pelx;
    PPixPool pely;
    if ( _glxpixpool == NULL )
    {
        return;
    }
    _glthread_LOCK_MUTEX(__glPixMutex);
    pelx = _glxpixpool;
    pely = pelx;
    while( pelx ) {
        if ( pelx->drawable == drawable )
        {
            pely->pnext = pelx->pnext;
            free(pelx);
            _glthread_UNLOCK_MUTEX(__glPixMutex);
            return;
        }
        pely = pelx;
        pelx = (PPixPool)pelx->pnext;
    }
    _glthread_UNLOCK_MUTEX(__glPixMutex);
}

GLboolean __drawableIsPixmap(GLXDrawable drawable)
{
    PPixPool pelx;
    if ( _glxpixpool == NULL )
    {
        return GL_FALSE;
    }
    _glthread_LOCK_MUTEX(__glPixMutex);
    pelx = _glxpixpool;
    while ( pelx )
    {
        if ( drawable == pelx->drawable )
        {
            _glthread_UNLOCK_MUTEX(__glPixMutex);
            return GL_TRUE;
        }
        pelx = (PPixPool)pelx->pnext;
    }
    _glthread_UNLOCK_MUTEX(__glPixMutex);
    return GL_FALSE;
}
/**
 * Change a drawable's attribute.
 *
 * This function is used to implement \c glXSelectEvent.
 */
static GLvoid
ChangeDrawableAttribute( Display * dpy, GLXDrawable drawable,
             const CARD32 * attribs, size_t num_attribs )
{
    CARD32 * output;

    if ( (dpy == NULL) || (drawable == 0) ) {
        return;
    }

    LockDisplay(dpy);

    {
        xGLXChangeDrawableAttributesReq *req;

        GetReqExtra( GLXChangeDrawableAttributes, 8 + (8 * num_attribs), req );
        output = (CARD32 *) (req + 1);

        req->reqType = __glXSetupForCommand(dpy);
        req->glxCode = X_GLXChangeDrawableAttributes;
        req->drawable = drawable;
        req->numAttribs = (CARD32) num_attribs;
    }

    (GLvoid) memcpy( output, attribs, sizeof( CARD32 ) * 2 * num_attribs );

    UnlockDisplay(dpy);
    SyncHandle();

    return;
}


/**
 * Destroy a pbuffer.
 *
 * This function is used to implement \c glXDestroyPbuffer.
 */
static GLvoid
DestroyPbuffer( Display * dpy, GLXDrawable drawable )
{
    if ( (dpy == NULL) || (drawable == 0) ) {
        return;
    }

    LockDisplay(dpy);

    {
        xGLXDestroyPbufferReq * req;

        GetReqExtra( GLXDestroyPbuffer, 4, req );
        req->reqType = __glXSetupForCommand(dpy);
        req->glxCode = X_GLXDestroyPbuffer;
        req->pbuffer = (GLXPbuffer) drawable;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return;
}


/**
 * Get a drawable's attribute.
 *
 * This function is used to implement \c glXGetSelectedEvent.
 */
static int
GetDrawableAttribute( Display *dpy, GLXDrawable drawable,
              int attribute, unsigned int *value )
{
    xGLXGetDrawableAttributesReply reply;
    CARD32 * data;
    unsigned int length;
    unsigned int i;
    unsigned int num_attributes;

    if ( (dpy == NULL) || (drawable == 0) ) {
        return 0;
    }

    LockDisplay(dpy);

    {
        xGLXGetDrawableAttributesReq *req;

        GetReqExtra( GLXGetDrawableAttributes, 4, req );
        req->reqType = __glXSetupForCommand(dpy);
        req->glxCode = X_GLXGetDrawableAttributes;
        req->drawable = drawable;
    }

    _XReply(dpy, (xReply*) &reply, 0, False);

    length = reply.length;
    num_attributes = reply.numAttribs;
    data = (CARD32 *) Xmalloc( length * sizeof(CARD32) );
    if ( data == NULL ) {
        /* Throw data on the floor */
        _XEatData(dpy, length);
    } else {
        _XRead(dpy, (char *)data, length * sizeof(CARD32) );
    }

    UnlockDisplay(dpy);
    SyncHandle();

    /* Search the set of returned attributes for the attribute requested by
     * the caller.
     */
    for ( i = 0 ; i < num_attributes ; i++ ) {
        if ( data[i*2] == attribute ) {
            *value = data[ (i*2) + 1 ];
            break;
        }
    }

    Xfree( data );

    return 0;
}


/**
 * Create a non-pbuffer GLX drawable.
 */
static GLXDrawable
CreateDrawable( Display *dpy, const __GLcontextModes * fbconfig,
        Drawable drawable, const int *attrib_list, CARD8 glxCode )
{
   xGLXCreateWindowReq *req;
   CARD32 *data;
   unsigned int i;
   CARD8 opcode;
   GLXDrawable xid;

   i = 0;
   if (attrib_list) {
      while (attrib_list[i * 2] != None)
         i++;
   }

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return None;

   LockDisplay(dpy);
   GetReqExtra(GLXCreateWindow, 8 * i, req);
   data = (CARD32 *) (req + 1);

   req->reqType = opcode;
   req->glxCode = glxCode;
   req->screen = (CARD32) fbconfig->screen;
   req->fbconfig = fbconfig->fbconfigID;
   req->window = drawable;
   req->glxwindow = xid = XAllocID(dpy);
   req->numAttribs = i;

   if (attrib_list)
      memcpy(data, attrib_list, 8 * i);

   UnlockDisplay(dpy);
   SyncHandle();

    return drawable;
}


/**
 * Destroy a non-pbuffer GLX drawable.
 */
static GLvoid
DestroyDrawable( Display * dpy, GLXDrawable drawable, CARD32 glxCode )
{
   xGLXDestroyPbufferReq *req;
   CARD8 opcode;

   opcode = __glXSetupForCommand(dpy);
   if (!opcode)
      return;

   LockDisplay(dpy);

   GetReq(GLXDestroyPbuffer, req);
   req->reqType = opcode;
   req->glxCode = glxCode;
   req->pbuffer = (GLXPbuffer) drawable;

   UnlockDisplay(dpy);
   SyncHandle();

    return;
}


/**
 * Create a pbuffer.
 *
 * This function is used to implement \c glXCreatePbuffer.
 */
static GLXDrawable
CreatePbuffer( Display *dpy, const __GLcontextModes * fbconfig,
           unsigned int width, unsigned int height,
           const int *attrib_list, GLboolean size_in_attribs )
{
    GLXDrawable id = 0;
    CARD32 * data;
    unsigned int  i;

    for ( i = 0 ; attrib_list[i * 2] != None ; i++ )
        /* empty */ ;

    LockDisplay(dpy);
    id = XAllocID(dpy);

    {
        xGLXCreatePbufferReq * req;
        unsigned int extra = (size_in_attribs) ? 0 : 2;

        GetReqExtra( GLXCreatePbuffer, (8 * (i + extra)), req );
        data = (CARD32 *) (req + 1);

        req->reqType = __glXSetupForCommand(dpy);
        req->glxCode = X_GLXCreatePbuffer;
        req->screen = (CARD32) fbconfig->screen;
        req->fbconfig = fbconfig->fbconfigID;
        req->pbuffer = (GLXPbuffer) id;
        req->numAttribs = (CARD32) (i + extra);

        if ( ! size_in_attribs ) {
            data[(2 * i) + 0] = GLX_PBUFFER_WIDTH;
            data[(2 * i) + 1] = width;
            data[(2 * i) + 2] = GLX_PBUFFER_HEIGHT;
            data[(2 * i) + 3] = height;
            data += 4;
        }
    }

    (GLvoid) memcpy( data, attrib_list, sizeof(CARD32) * 2 * i );

    UnlockDisplay(dpy);
    SyncHandle();

    return id;
}

/**
 * Create a new pbuffer.
 */
GLXPbuffer
GLX_PREFIX(glXCreatePbuffer)(Display *dpy, GLXFBConfig config,
                 const int *attrib_list)
{
    return (GLXPbuffer) CreatePbuffer( dpy, (__GLcontextModes *) config,
                        0, 0, attrib_list, GL_TRUE );
}

/**
 * Create a new pbuffer.
 */
GLXPbufferSGIX
GLX_PREFIX(glXCreateGLXPbufferSGIX)(Display *dpy, GLXFBConfigSGIX config,
                                    unsigned int width, unsigned int height,
                                    int *attrib_list)
{
    return (GLXPbufferSGIX) CreatePbuffer( dpy, (__GLcontextModes *) config,
                        width, height, attrib_list, GL_FALSE );
}

/**
 * Destroy an existing pbuffer.
 */
GLvoid
GLX_PREFIX(glXDestroyPbuffer)(Display *dpy, GLXPbuffer pbuf)
{
    DestroyPbuffer( dpy, pbuf );
}


/**
 * Query an attribute of a drawable.
 */
GLvoid
GLX_PREFIX(glXQueryDrawable)(Display *dpy, GLXDrawable drawable,
                   int attribute, unsigned int *value)
{
    GetDrawableAttribute( dpy, drawable, attribute, value );
}


/**
 * Query an attribute of a pbuffer.
 */
int
GLX_PREFIX(glXQueryGLXPbufferSGIX)(Display *dpy, GLXPbufferSGIX drawable,
                                   int attribute, unsigned int *value)
{
    return GetDrawableAttribute( dpy, drawable, attribute, value );
}


/**
 * Select the event mask for a drawable.
 */
GLvoid
GLX_PREFIX(glXSelectEvent)(Display *dpy, GLXDrawable drawable,
               unsigned long mask)
{
    CARD32 attribs[2];

    attribs[0] = (CARD32) GLX_EVENT_MASK;
    attribs[1] = (CARD32) mask;

    ChangeDrawableAttribute( dpy, drawable, attribs, 1 );
}


/**
 * Get the selected event mask for a drawable.
 */
GLvoid
GLX_PREFIX(glXGetSelectedEvent)(Display *dpy, GLXDrawable drawable,
                unsigned long *mask)
{
    unsigned int value;

    /* The non-sense with value is required because on LP64 platforms
     * sizeof(unsigned int) != sizeof(unsigned long).  On little-endian
     * we could just type-cast the pointer, but why?
     */

    GetDrawableAttribute( dpy, drawable, GLX_EVENT_MASK_SGIX, & value );
    *mask = value;
}


GLXPixmap
GLX_PREFIX(glXCreatePixmap)( Display *dpy, GLXFBConfig config, Pixmap pixmap,
                 const int *attrib_list )
{
    GLXPixmap temPix = CreateDrawable( dpy, (__GLcontextModes *) config, (Drawable) pixmap, attrib_list, X_GLXCreatePixmap );
    if ( temPix )
        __putPixIntoPool((GLXDrawable) temPix);
    return temPix;
}


GLXWindow
GLX_PREFIX(glXCreateWindow)( Display *dpy, GLXFBConfig config, Window win,
                 const int *attrib_list )
{
    return CreateDrawable( dpy, (__GLcontextModes *) config,
              (Drawable) win, attrib_list, X_GLXCreateWindow );
}


GLvoid
GLX_PREFIX(glXDestroyPixmap)(Display *dpy, GLXPixmap pixmap)
{
    __delPixFromPool((GLXDrawable) pixmap);
    DestroyDrawable( dpy, (GLXDrawable) pixmap, X_GLXDestroyPixmap );
}


GLvoid
GLX_PREFIX(glXDestroyWindow)(Display *dpy, GLXWindow win)
{
    DestroyDrawable( dpy, (GLXDrawable) win, X_GLXDestroyWindow );
}


GLX_ALIAS_VOID(glXDestroyGLXPbufferSGIX,
          (Display *dpy, GLXPbufferSGIX pbuf),
          (dpy, pbuf),
          glXDestroyPbuffer)

GLX_ALIAS_VOID(glXSelectEventSGIX,
          (Display *dpy, GLXDrawable drawable, unsigned long mask),
          (dpy, drawable, mask),
          glXSelectEvent)

GLX_ALIAS_VOID(glXGetSelectedEventSGIX,
          (Display *dpy, GLXDrawable drawable, unsigned long *mask),
          (dpy, drawable, mask),
          glXGetSelectedEvent)
