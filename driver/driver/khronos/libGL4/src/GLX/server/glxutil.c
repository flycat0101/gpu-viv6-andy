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


/* $XFree86: xc/programs/Xserver/GL/glx/glxutil.c,v 1.5 2001/03/21 16:29:37 dawes Exp $ */
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

#define NEED_REPLIES
#define FONT_PCF
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "gl.h"
#include "glxserver.h"
#include "GL/glxtokens.h"
#include <unpack.h>
#include <pixmapstr.h>
#include <windowstr.h>
#include "glxutil.h"
#include "glxbuf.h"
#include "GL/glx_ansic.h"
#include "glcore.h"
#include "GL/glxint.h"
#include "glcontextmodes.h"
#include "context.h"


/************************************************************************/

GLvoid __glXNop(GLvoid) {}

/************************************************************************/

/* Memory Allocation for GLX */

GLvoid *
__glXMalloc(size_t size)
{
    GLvoid *addr;

    if (size == 0) {
    return NULL;
    }
    addr = (GLvoid *) xalloc(size);
    if (addr == NULL) {
    /* XXX: handle out of memory error */
    return NULL;
    }
    return addr;
}

GLvoid *
__glXCalloc(size_t numElements, size_t elementSize)
{
    GLvoid *addr;
    size_t size;

    if ((numElements == 0) || (elementSize == 0)) {
    return NULL;
    }
    size = numElements * elementSize;
    addr = (GLvoid *) xalloc(size);
    if (addr == NULL) {
    /* XXX: handle out of memory error */
    return NULL;
    }
    __glXMemset(addr, 0, size);
    return addr;
}

GLvoid *
__glXRealloc(GLvoid *addr, size_t newSize)
{
    GLvoid *newAddr;

    if (addr) {
    if (newSize == 0) {
        xfree(addr);
        return NULL;
    } else {
        newAddr = xrealloc(addr, newSize);
    }
    } else {
    if (newSize == 0) {
        return NULL;
    } else {
        newAddr = xalloc(newSize);
    }
    }
    if (newAddr == NULL) {
    return NULL;    /* XXX: out of memory */
    }

    return newAddr;
}

GLvoid
__glXFree(GLvoid *addr)
{
    if (addr) {
    xfree(addr);
    }
}

/************************************************************************/
/* Context stuff */


/*
** associate a context with a drawable
*/
GLvoid
__glXAssociateContext(__GLXcontext *glxc)
{
    glxc->nextDrawPriv = glxc->drawPriv->drawGlxc;
    glxc->drawPriv->drawGlxc = glxc;

    __glXRefDrawablePrivate(glxc->drawPriv);


    glxc->nextReadPriv = glxc->readPriv->readGlxc;
    glxc->readPriv->readGlxc = glxc;

    __glXRefDrawablePrivate(glxc->readPriv);
}

/*
** Deassociate a context from a drawable
*/
GLvoid
__glXDeassociateContext(__GLXcontext *glxc)
{
    __GLXcontext *curr, *prev;

    prev = NULL;
    for ( curr = glxc->drawPriv->drawGlxc
      ; curr != NULL
      ; prev = curr, curr = curr->nextDrawPriv ) {
    if (curr == glxc) {
        /* found context.  Deassociate. */
        if (prev == NULL) {
        glxc->drawPriv->drawGlxc = curr->nextDrawPriv;
        } else {
        prev->nextDrawPriv = curr->nextDrawPriv;
        }
        curr->nextDrawPriv = NULL;
        __glXUnrefDrawablePrivate(glxc->drawPriv);
        break;
    }
    }


    prev = NULL;
    for ( curr = glxc->readPriv->readGlxc
      ; curr != NULL
      ; prev = curr, curr = curr->nextReadPriv ) {
    if (curr == glxc) {
        /* found context.  Deassociate. */
        if (prev == NULL) {
        glxc->readPriv->readGlxc = curr->nextReadPriv;
        } else {
        prev->nextReadPriv = curr->nextReadPriv;
        }
        curr->nextReadPriv = NULL;
        __glXUnrefDrawablePrivate(glxc->readPriv);
        break;
    }
    }
}

/************************************************************************/

GLvoid
__glXGetDrawableSize(__GLdrawablePrivate *glPriv,
             GLint *x, GLint *y, GLuint *width, GLuint *height)
{
    __GLXdrawablePrivate *glxPriv = (__GLXdrawablePrivate *)glPriv->other;

    if (glxPriv) {
    *x = glxPriv->xorigin;
    *y = glxPriv->yorigin;
    *width = glxPriv->width;
    *height = glxPriv->height;
    } else {
    *x = *y = *width = *height = 0;
    }
}

GLboolean
__glXResizeDrawable(__GLdrawablePrivate *glPriv)
{
    /* nothing to be done here */
    return GL_TRUE;
}


/*****************************************************************************/
/* accessing the drawable private */

static GLvoid
LockDP(__GLdrawablePrivate *glPriv, __GLcontext *gc)
{
    GL_ASSERT(0);
}

static GLvoid
UnlockDP(__GLdrawablePrivate *glPriv)
{
}

/*****************************************************************************/
/* Drawable private stuff */

GLvoid
__glXRefDrawablePrivate(__GLXdrawablePrivate *glxPriv)
{
    glxPriv->refCount++;
}

GLvoid
__glXUnrefDrawablePrivate(__GLXdrawablePrivate *glxPriv)
{
    glxPriv->refCount--;
    if (glxPriv->refCount == 0) {
    __glXDestroyDrawablePrivate(glxPriv);
    }
}


static GLvoid
__glXGenerateDrawableInternalFormat(__GLdrawablePrivate * drawable)
{
    __GLcontextModes * modes = &drawable->modes;

    switch(modes->rgbaBits)
    {
    case 8:
        drawable->internalFormatColorBuffer = GL_R3_G3_B2;
        break;
    case 16:
        drawable->internalFormatColorBuffer = GL_RGB5;
        break;
    case 32:
        drawable->internalFormatColorBuffer = GL_RGBA;
        break;
    default:
        GL_ASSERT(0);
        break;
    }

    switch(modes->depthBits)
    {
    case 16:
        drawable->internalFormatDepthStencil = GL_DEPTH_COMPONENT16;
        break;
    case 24:
        drawable->internalFormatDepthStencil = GL_DEPTH_COMPONENT;
        break;
    case 32:
        drawable->internalFormatDepthStencil = GL_DEPTH_COMPONENT32;
        break;
    default:
        GL_ASSERT(0);
        break;
    }

    switch(modes->accumBits)
    {
    case 0:
        break;
    case 64:
        drawable->internalFormatAccumBuffer = GL_RGBA16F_ARB;
        break;
    case 128:
        drawable->internalFormatAccumBuffer = GL_RGBA32F_ARB;
        break;
    default:
        GL_ASSERT(0);
        break;
    }

    return;
}


__GLXdrawablePrivate *
__glXCreateDrawablePrivate(DrawablePtr pDraw, XID drawId,
               __GLcontextModes *modes)
{
    __GLXdrawablePrivate *glxPriv;
    __GLdrawablePrivate *glPriv;
    __GLXscreenInfo *pGlxScreen;

    glxPriv = (__GLXdrawablePrivate *) __glXMalloc(sizeof(*glxPriv));
    __glXMemset(glxPriv, 0, sizeof(__GLXdrawablePrivate));

    glxPriv->type = pDraw->type;
    glxPriv->pDraw = pDraw;
    glxPriv->drawId = drawId;

    /* if not a pixmap, lookup will fail, so pGlxPixmap will be NULL */
    glxPriv->pGlxPixmap = (__GLXpixmap *)
    LookupIDByType(drawId, __glXPixmapRes);
    /* since we are creating the drawablePrivate, drawId should be new */
    if (!AddResource(drawId, __glXDrawableRes, glxPriv)) {
    /* oops! */
    __glXFree(glxPriv);
    return NULL;
    }

    /* fill up glPriv */
    glPriv = &glxPriv->glPriv;
    glPriv->modes = *modes;
    __glXGenerateDrawableInternalFormat( glPriv );
    glPriv->malloc = __glXMalloc;
    glPriv->calloc = __glXCalloc;
    glPriv->realloc = __glXRealloc;
    glPriv->free = __glXFree;
    glPriv->other = glxPriv;

    glxPriv->freeBuffers = __glXFreeBuffers;
    glxPriv->updatePalette = (GLvoid (*)(__GLXdrawablePrivate *)) __glXNop;

    pGlxScreen = &__glXActiveScreens[pDraw->pScreen->myNum];

    if (glxPriv->type == DRAWABLE_WINDOW) {
    VisualID vid = wVisual((WindowPtr)pDraw);

    glxPriv->modes = _gl_context_modes_find_visual( pGlxScreen->modes, vid );
    __glXFBInitDrawable(glxPriv, modes);
    } else {
    glxPriv->modes = glxPriv->pGlxPixmap->modes;
    __glXPixInitDrawable(glxPriv, modes);
    }

    /* initialize the core's private buffer information */
/*    (*pGlxScreen->createBuffer)(glxPriv);*/

    GL_ASSERT(0);/* TODO: call the device specific createBuffer function*/

    return glxPriv;
}

GLboolean
__glXDestroyDrawablePrivate(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;

    /* remove the drawable from the drawable list */
    FreeResourceByType(glxPriv->drawId, __glXDrawableRes, FALSE);


    /* Free any framebuffer memory attached to the drawable */
    if (glxPriv->freeBuffers) {
    (*glxPriv->freeBuffers)(glxPriv);
    }

    /* Free the drawable Private */
    __glXFree(glxPriv);

    return GL_TRUE;
}

__GLXdrawablePrivate *
__glXFindDrawablePrivate(XID drawId)
{
    __GLXdrawablePrivate *glxPriv;

    glxPriv = (__GLXdrawablePrivate *)LookupIDByType(drawId, __glXDrawableRes);

    return glxPriv;
}

__GLXdrawablePrivate *
__glXGetDrawablePrivate(DrawablePtr pDraw, XID drawId,
            __GLcontextModes *modes)
{
    __GLXdrawablePrivate *glxPriv;

    glxPriv = __glXFindDrawablePrivate(drawId);

    if (glxPriv == NULL) {
    glxPriv = __glXCreateDrawablePrivate(pDraw, drawId, modes);
    if (glxPriv) {
        __glXRefDrawablePrivate(glxPriv);
    }
    }

    return glxPriv;
}

GLvoid
__glXCacheDrawableSize(__GLXdrawablePrivate *glxPriv)
{
    if (glxPriv) {
    if (glxPriv->pDraw) {
        glxPriv->xorigin = glxPriv->pDraw->x;
        glxPriv->yorigin = glxPriv->pDraw->y;
        glxPriv->width = glxPriv->pDraw->width;
        glxPriv->height = glxPriv->pDraw->height;
    }
    }
}

/*
** resize/move the drawable.  Called during the actual resize callback
** to update the drawable side of the buffers
*/
GLboolean
__glXResizeDrawableBuffers(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
    GLint x, y;
    GLuint w, h;
#if defined(__GL_ALIGNED_BUFFERS)
    GLint xAlignment, yAlignment;
    GLint xOffset, yOffset;
    GLint xStart, xEnd;
    GLint yStart, yEnd;
    GLuint xAlignedMask, yAlignedMask;
#endif
    GLboolean status = GL_TRUE;

    __glXCacheDrawableSize(glxPriv);

    w = glxPriv->width;
    h = glxPriv->height;
    x = glxPriv->xorigin;
    y = glxPriv->yorigin;

#if defined(__GL_ALIGNED_BUFFERS)
    xAlignment = glPriv->xAlignment;
    yAlignment = glPriv->yAlignment;

    xOffset = x & (xAlignment-1);
    yOffset = y & (yAlignment-1);

    xAlignedMask = ~(xAlignment-1);
    yAlignedMask = ~(yAlignment-1);

    xStart = x; xEnd = x+w;
    yStart = y; yEnd = y+h;

    xStart &= xAlignedMask;
    if (xEnd & ~xAlignedMask) {
    xEnd = (xEnd&xAlignedMask) + xAlignment;
    }
    yStart &= yAlignedMask;
    if (yEnd & ~yAlignedMask) {
    yEnd = (yEnd&yAlignedMask) + yAlignment;
    }

    x = xStart; y = yStart;
    w = xEnd-xStart; h = yEnd-yStart;
#endif

    if ((x != glPriv->xOrigin) ||
    (y != glPriv->yOrigin) ||
#if defined(__GL_ALIGNED_BUFFERS)
    (xOffset != glPriv->xOffset) ||
    (yOffset != glPriv->yOffset) ||
#endif
    (w != glPriv->width) ||
    (h != glPriv->height) ||
    (!w && !h)) {
    /* set up the glPriv info */
    glPriv->width = w;
    glPriv->height = h;
    glPriv->xOrigin = x;
    glPriv->yOrigin = y;
#if defined(__GL_ALIGNED_BUFFERS)
    glPriv->xOffset = xOffset;
    glPriv->yOffset = yOffset;
#endif

    /* notify the buffers */
    status = __glXResizeBuffers(glPriv, x, y, w, h);
    }

    return status;
}

/************************************************************************/

